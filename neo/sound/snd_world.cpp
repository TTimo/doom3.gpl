/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "snd_local.h"

/*
==================
idSoundWorldLocal::Init
==================
*/
void idSoundWorldLocal::Init( idRenderWorld *renderWorld ) {
	rw = renderWorld;
	writeDemo = NULL;

	listenerAxis.Identity();
	listenerPos.Zero();
	listenerPrivateId = 0;
	listenerQU.Zero();
	listenerArea = 0;
	listenerAreaName = "Undefined";
	listenerEnvironmentID = -2;

	gameMsec = 0;
	game44kHz = 0;
	pause44kHz = -1;
	lastAVI44kHz = 0;

	for ( int i = 0 ; i < SOUND_MAX_CLASSES ; i++ ) {
		soundClassFade[i].Clear();
	}

	// fill in the 0 index spot
	idSoundEmitterLocal	*placeHolder = new idSoundEmitterLocal;
	emitters.Append( placeHolder );

	fpa[0] = fpa[1] = fpa[2] = fpa[3] = fpa[4] = fpa[5] = NULL;

	aviDemoPath = "";
	aviDemoName = "";

	localSound = NULL;

	slowmoActive		= false;
	slowmoSpeed			= 0;
	enviroSuitActive	= false;
}

/*
===============
idSoundWorldLocal::idSoundWorldLocal
===============
*/
idSoundWorldLocal::idSoundWorldLocal() {
}

/*
===============
idSoundWorldLocal::~idSoundWorldLocal
===============
*/
idSoundWorldLocal::~idSoundWorldLocal() {
	Shutdown();
}

/*
===============
idSoundWorldLocal::Shutdown

  this is called from the main thread
===============
*/
void idSoundWorldLocal::Shutdown() {
	int i;

	if ( soundSystemLocal.currentSoundWorld == this ) {
		soundSystemLocal.currentSoundWorld = NULL;
	}

	AVIClose();

	for ( i = 0; i < emitters.Num(); i++ ) {
		if ( emitters[i] ) {
			delete emitters[i];
			emitters[i] = NULL;
		}
	}
	localSound = NULL;
}

/*
===================
idSoundWorldLocal::ClearAllSoundEmitters
===================
*/
void idSoundWorldLocal::ClearAllSoundEmitters() {
	int i;

	Sys_EnterCriticalSection();

	AVIClose();

	for ( i = 0; i < emitters.Num(); i++ ) {
		idSoundEmitterLocal *sound = emitters[i];
		sound->Clear();
	}
	localSound = NULL;

	Sys_LeaveCriticalSection();
}

/*
===================
idSoundWorldLocal::AllocLocalSoundEmitter
===================
*/
idSoundEmitterLocal *idSoundWorldLocal::AllocLocalSoundEmitter() {
	int i, index;
	idSoundEmitterLocal *def = NULL;

	index = -1;

	// never use the 0 index spot

	for ( i = 1 ; i < emitters.Num() ; i++ ) {
		def = emitters[i];

		// check for a completed and freed spot
		if ( def->removeStatus >= REMOVE_STATUS_SAMPLEFINISHED ) {
			index = i;
			if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
				common->Printf( "sound: recycling sound def %d\n", i );
			}
			break;
		}
	}

	if ( index == -1 ) {
		// append a brand new one
		def = new idSoundEmitterLocal;

		// we need to protect this from the async thread
		Sys_EnterCriticalSection();
		index = emitters.Append( def );
		Sys_LeaveCriticalSection();

		if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
			common->Printf( "sound: appended new sound def %d\n", index );
		}
	}

	def->Clear();
	def->index = index;
	def->removeStatus = REMOVE_STATUS_ALIVE;
	def->soundWorld = this;

	return def;
}

/*
===================
idSoundWorldLocal::AllocSoundEmitter

  this is called from the main thread
===================
*/
idSoundEmitter *idSoundWorldLocal::AllocSoundEmitter() {
	idSoundEmitterLocal *emitter = AllocLocalSoundEmitter();

	if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
		common->Printf( "AllocSoundEmitter = %i\n",  emitter->index );
	}
	if ( writeDemo ) {
		writeDemo->WriteInt( DS_SOUND );
		writeDemo->WriteInt( SCMD_ALLOC_EMITTER );
		writeDemo->WriteInt( emitter->index );
	}

	return emitter;
}

/*
===================
idSoundWorldLocal::StartWritingDemo

  this is called from the main thread
===================
*/
void idSoundWorldLocal::StartWritingDemo( idDemoFile *demo ) {
	writeDemo = demo;

	writeDemo->WriteInt( DS_SOUND );
	writeDemo->WriteInt( SCMD_STATE );

	// use the normal save game code to archive all the emitters
	WriteToSaveGame( writeDemo );
}

/*
===================
idSoundWorldLocal::StopWritingDemo

  this is called from the main thread
===================
*/
void idSoundWorldLocal::StopWritingDemo() {
	writeDemo = NULL;
}

/*
===================
idSoundWorldLocal::ProcessDemoCommand

  this is called from the main thread
===================
*/
void idSoundWorldLocal::ProcessDemoCommand( idDemoFile *readDemo ) {
	int	index;
	idSoundEmitterLocal	*def;

	if ( !readDemo ) {
		return;
	}

	soundDemoCommand_t	dc;

	if ( !readDemo->ReadInt( (int&)dc ) ) {
		return;
	}

	switch( dc ) {
	case SCMD_STATE:
		// we need to protect this from the async thread
		// other instances of calling idSoundWorldLocal::ReadFromSaveGame do this while the sound code is muted
		// setting muted and going right in may not be good enough here, as we async thread may already be in an async tick (in which case we could still race to it)
		Sys_EnterCriticalSection();
		ReadFromSaveGame( readDemo );
		Sys_LeaveCriticalSection();
		UnPause();
		break;
	case SCMD_PLACE_LISTENER:
		{
			idVec3	origin;
			idMat3	axis;
			int		listenerId;
			int		gameTime;

			readDemo->ReadVec3( origin );
			readDemo->ReadMat3( axis );
			readDemo->ReadInt( listenerId );
			readDemo->ReadInt( gameTime );
			
			PlaceListener( origin, axis, listenerId, gameTime, "" );
		};
		break;
	case SCMD_ALLOC_EMITTER:
		readDemo->ReadInt( index );
		if ( index < 1 || index > emitters.Num() ) {
			common->Error( "idSoundWorldLocal::ProcessDemoCommand: bad emitter number" );
		}
		if ( index == emitters.Num() ) {
			// append a brand new one
			def = new idSoundEmitterLocal;
			emitters.Append( def );
		}
		def = emitters[ index ];
		def->Clear();
		def->index = index;
		def->removeStatus = REMOVE_STATUS_ALIVE;
		def->soundWorld = this;
		break;
	case SCMD_FREE:
		{
			int	immediate;

			readDemo->ReadInt( index );
			readDemo->ReadInt( immediate );
			EmitterForIndex( index )->Free( immediate != 0 );
		}
		break;
	case SCMD_UPDATE:
		{
			idVec3 origin;
			int listenerId;
			soundShaderParms_t parms;

			readDemo->ReadInt( index );
			readDemo->ReadVec3( origin );
			readDemo->ReadInt( listenerId );
			readDemo->ReadFloat( parms.minDistance );
			readDemo->ReadFloat( parms.maxDistance );
			readDemo->ReadFloat( parms.volume );
			readDemo->ReadFloat( parms.shakes );
			readDemo->ReadInt( parms.soundShaderFlags );
			readDemo->ReadInt( parms.soundClass );
			EmitterForIndex( index )->UpdateEmitter( origin, listenerId, &parms );
		}
		break;
	case SCMD_START:
		{
			const idSoundShader *shader;
			int			channel;
			float		diversity;
			int			shaderFlags;

			readDemo->ReadInt( index );
			shader = declManager->FindSound( readDemo->ReadHashString() );
			readDemo->ReadInt( channel );
			readDemo->ReadFloat( diversity );
			readDemo->ReadInt( shaderFlags );
			EmitterForIndex( index )->StartSound( shader, (s_channelType)channel, diversity, shaderFlags );
		}
		break;
	case SCMD_MODIFY:
		{
			int		channel;
			soundShaderParms_t parms;

			readDemo->ReadInt( index );
			readDemo->ReadInt( channel );
			readDemo->ReadFloat( parms.minDistance );
			readDemo->ReadFloat( parms.maxDistance );
			readDemo->ReadFloat( parms.volume );
			readDemo->ReadFloat( parms.shakes );
			readDemo->ReadInt( parms.soundShaderFlags );
			readDemo->ReadInt( parms.soundClass );
			EmitterForIndex( index )->ModifySound( (s_channelType)channel, &parms );
		}
		break;
	case SCMD_STOP:
		{
			int		channel;

			readDemo->ReadInt( index );
			readDemo->ReadInt( channel );
			EmitterForIndex( index )->StopSound( (s_channelType)channel );
		}
		break;
	case SCMD_FADE:
		{
			int		channel;
			float	to, over;

			readDemo->ReadInt( index );
			readDemo->ReadInt( channel );
			readDemo->ReadFloat( to );
			readDemo->ReadFloat( over );
			EmitterForIndex( index )->FadeSound((s_channelType)channel, to, over );
		}
		break;
	}
}

/*
===================
idSoundWorldLocal::CurrentShakeAmplitudeForPosition

  this is called from the main thread
===================
*/
float idSoundWorldLocal::CurrentShakeAmplitudeForPosition( const int time, const idVec3 &listererPosition ) {
	float amp = 0.0f;
	int localTime;

	if ( idSoundSystemLocal::s_constantAmplitude.GetFloat() >= 0.0f ) {
		return 0.0f;
	}

	localTime = soundSystemLocal.GetCurrent44kHzTime();

	for ( int i = 1; i < emitters.Num(); i++ ) {
		idSoundEmitterLocal *sound = emitters[i];
		if ( !sound->hasShakes ) {
			continue;
		}
		amp += FindAmplitude( sound, localTime, &listererPosition, SCHANNEL_ANY, true );
	}
	return amp;
}

/*
===================
idSoundWorldLocal::MixLoop

Sum all sound contributions into finalMixBuffer, an unclamped float buffer holding
all output channels.  MIXBUFFER_SAMPLES samples will be created, with each sample consisting
of 2 or 6 floats depending on numSpeakers.

this is normally called from the sound thread, but also from the main thread
for AVIdemo writing
===================
*/
void idSoundWorldLocal::MixLoop( int current44kHz, int numSpeakers, float *finalMixBuffer ) {
	int i, j;
	idSoundEmitterLocal *sound;

	// if noclip flying outside the world, leave silence
	if ( listenerArea == -1 ) {
		if ( idSoundSystemLocal::useOpenAL )
			alListenerf( AL_GAIN, 0.0f );
		return;
	} 

	// update the listener position and orientation
	if ( idSoundSystemLocal::useOpenAL ) {
		ALfloat listenerPosition[3];

		listenerPosition[0] = -listenerPos.y;
		listenerPosition[1] =  listenerPos.z;
		listenerPosition[2] = -listenerPos.x;

		ALfloat listenerOrientation[6];

		listenerOrientation[0] = -listenerAxis[0].y;
		listenerOrientation[1] =  listenerAxis[0].z;
		listenerOrientation[2] = -listenerAxis[0].x;

		listenerOrientation[3] = -listenerAxis[2].y;
		listenerOrientation[4] =  listenerAxis[2].z;
		listenerOrientation[5] = -listenerAxis[2].x;

		alListenerf( AL_GAIN, 1.0f );
		alListenerfv( AL_POSITION, listenerPosition );
		alListenerfv( AL_ORIENTATION, listenerOrientation );

#if ID_OPENAL
		if ( soundSystemLocal.s_useEAXReverb.GetBool() ) {
			if ( soundSystemLocal.efxloaded ) {
				idSoundEffect *effect = NULL;
				int EnvironmentID = -1;
				idStr defaultStr( "default" );
				idStr listenerAreaStr( listenerArea );
				
				soundSystemLocal.EFXDatabase.FindEffect( listenerAreaStr, &effect, &EnvironmentID );
				if (!effect)
					soundSystemLocal.EFXDatabase.FindEffect( listenerAreaName, &effect, &EnvironmentID );
				if (!effect)
					soundSystemLocal.EFXDatabase.FindEffect( defaultStr, &effect, &EnvironmentID );
				
				// only update if change in settings 
				if ( soundSystemLocal.s_muteEAXReverb.GetBool() || ( listenerEnvironmentID != EnvironmentID ) ) {
					EAXREVERBPROPERTIES EnvironmentParameters;
					
					// get area reverb setting from EAX Manager
					if ( ( effect ) && ( effect->data) && ( memcpy( &EnvironmentParameters, effect->data, effect->datasize ) ) ) {
						if ( soundSystemLocal.s_muteEAXReverb.GetBool() ) {
							EnvironmentParameters.lRoom = -10000;
							EnvironmentID = -2;
						}
						if ( soundSystemLocal.alEAXSet ) {
							soundSystemLocal.alEAXSet( &EAXPROPERTYID_EAX_FXSlot0, EAXREVERB_ALLPARAMETERS, 0, &EnvironmentParameters, sizeof( EnvironmentParameters ) );
						}
					}
					listenerEnvironmentID = EnvironmentID;
				}
			}
		}
#endif
	}

	// debugging option to mute all but a single soundEmitter
	if ( idSoundSystemLocal::s_singleEmitter.GetInteger() > 0 && idSoundSystemLocal::s_singleEmitter.GetInteger() < emitters.Num() ) {
		sound = emitters[idSoundSystemLocal::s_singleEmitter.GetInteger()];

		if ( sound && sound->playing ) {
			// run through all the channels
			for ( j = 0; j < SOUND_MAX_CHANNELS ; j++ ) {
				idSoundChannel	*chan = &sound->channels[j];

				// see if we have a sound triggered on this channel
				if ( !chan->triggerState ) {
					chan->ALStop();
					continue;
				}

				AddChannelContribution( sound, chan, current44kHz, numSpeakers, finalMixBuffer );
			}
		}
		return;
	}

	for ( i = 1; i < emitters.Num(); i++ ) {
		sound = emitters[i];

		if ( !sound ) {
			continue;
		}
		// if no channels are active, do nothing
		if ( !sound->playing ) {
			continue;
		}
		// run through all the channels
		for ( j = 0; j < SOUND_MAX_CHANNELS ; j++ ) {
			idSoundChannel	*chan = &sound->channels[j];

			// see if we have a sound triggered on this channel
			if ( !chan->triggerState ) {
				chan->ALStop();
				continue;
			}

			AddChannelContribution( sound, chan, current44kHz, numSpeakers, finalMixBuffer );
		}
	}

	if ( !idSoundSystemLocal::useOpenAL && enviroSuitActive ) {
		soundSystemLocal.DoEnviroSuit( finalMixBuffer, MIXBUFFER_SAMPLES, numSpeakers );
	}
}

//==============================================================================

/*
===================
idSoundWorldLocal::AVIOpen

	this is called by the main thread
===================
*/
void idSoundWorldLocal::AVIOpen( const char *path, const char *name ) {
	aviDemoPath = path;
	aviDemoName = name;

	lastAVI44kHz = game44kHz - game44kHz % MIXBUFFER_SAMPLES;

	if ( soundSystemLocal.snd_audio_hw->GetNumberOfSpeakers() == 6 ) {
		fpa[0] = fileSystem->OpenFileWrite( aviDemoPath + "channel_51_left.raw" );
		fpa[1] = fileSystem->OpenFileWrite( aviDemoPath + "channel_51_right.raw" );
		fpa[2] = fileSystem->OpenFileWrite( aviDemoPath + "channel_51_center.raw" );
		fpa[3] = fileSystem->OpenFileWrite( aviDemoPath + "channel_51_lfe.raw" );
		fpa[4] = fileSystem->OpenFileWrite( aviDemoPath + "channel_51_backleft.raw" );
		fpa[5] = fileSystem->OpenFileWrite( aviDemoPath + "channel_51_backright.raw" );
	} else {
		fpa[0] = fileSystem->OpenFileWrite( aviDemoPath + "channel_left.raw" );
		fpa[1] = fileSystem->OpenFileWrite( aviDemoPath + "channel_right.raw" );
	}

	soundSystemLocal.SetMute( true );
}

/*
===================
idSoundWorldLocal::AVIUpdate

this is called by the main thread
writes one block of sound samples if enough time has passed
This can be used to write wave files even if no sound hardware exists
===================
*/
void idSoundWorldLocal::AVIUpdate() {
	int		numSpeakers;
	
	if ( game44kHz - lastAVI44kHz < MIXBUFFER_SAMPLES ) {
		return;
	}

	if ( !soundSystemLocal.snd_audio_hw ) {
		numSpeakers = 2;
	} else {
		numSpeakers = soundSystemLocal.snd_audio_hw->GetNumberOfSpeakers();
	}

	float	mix[MIXBUFFER_SAMPLES*6+16];
	float	*mix_p = (float *)((( int)mix + 15 ) & ~15);	// SIMD align

	SIMDProcessor->Memset( mix_p, 0, MIXBUFFER_SAMPLES*sizeof(float)*numSpeakers );

	MixLoop( lastAVI44kHz, numSpeakers, mix_p );

	for ( int i = 0; i < numSpeakers; i++ ) {
		short outD[MIXBUFFER_SAMPLES];

		for( int j = 0; j < MIXBUFFER_SAMPLES; j++ ) {
			float s = mix_p[ j*numSpeakers + i];
			if ( s < -32768.0f ) {
				outD[j] = -32768;
			} else if ( s > 32767.0f ) {
				outD[j] = 32767;
			} else {
				outD[j] = idMath::FtoiFast( s );
			}
		}
		// write to file
		fpa[i]->Write( outD, MIXBUFFER_SAMPLES*sizeof(short) );
	}

	lastAVI44kHz += MIXBUFFER_SAMPLES;

	return;
}

/*
===================
idSoundWorldLocal::AVIClose
===================
*/
void idSoundWorldLocal::AVIClose( void ) {
	int i;

	if ( !fpa[0] ) {
		return;
	}

	// make sure the final block is written
	game44kHz += MIXBUFFER_SAMPLES;
	AVIUpdate();
	game44kHz -= MIXBUFFER_SAMPLES;

	for ( i = 0; i < 6; i++ ) {
		if ( fpa[i] != NULL ) {
			fileSystem->CloseFile( fpa[i] );
			fpa[i] = NULL;
		}
	}
	if ( soundSystemLocal.snd_audio_hw->GetNumberOfSpeakers() == 2 ) {
		// convert it to a wave file
		idFile *rL, *lL, *wO;
		idStr	name;

		name = aviDemoPath + aviDemoName + ".wav";
		wO = fileSystem->OpenFileWrite( name );
		if ( !wO ) {
			common->Error( "Couldn't write %s", name.c_str() );
		}

		name = aviDemoPath + "channel_right.raw";
		rL = fileSystem->OpenFileRead( name );
		if ( !rL ) {
			common->Error( "Couldn't open %s", name.c_str() );
		}

		name = aviDemoPath + "channel_left.raw";
		lL = fileSystem->OpenFileRead( name );
		if ( !lL ) {
			common->Error( "Couldn't open %s", name.c_str() );
		}

		int numSamples = rL->Length()/2;
		mminfo_t	info;
		pcmwaveformat_t format;
		
		info.ckid = fourcc_riff;
		info.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' );
		info.cksize = (rL->Length()*2) - 8 + 4 + 16 + 8 + 8;
		info.dwDataOffset = 12;
		
		wO->Write( &info, 12 );

		info.ckid = mmioFOURCC( 'f', 'm', 't', ' ' );
		info.cksize = 16;

		wO->Write( &info, 8 );

		format.wBitsPerSample = 16;
		format.wf.nAvgBytesPerSec = 44100*4;		// sample rate * block align
		format.wf.nChannels = 2;
		format.wf.nSamplesPerSec = 44100;
		format.wf.wFormatTag = WAVE_FORMAT_TAG_PCM;
		format.wf.nBlockAlign = 4;					// channels * bits/sample / 8

		wO->Write( &format, 16 );

		info.ckid = mmioFOURCC( 'd', 'a', 't', 'a' );
		info.cksize = rL->Length() * 2;

		wO->Write( &info, 8 );

		short s0, s1;
		for( i = 0; i < numSamples; i++ ) {
			lL->Read( &s0, 2 );
			rL->Read( &s1, 2 );
			wO->Write( &s0, 2 );
			wO->Write( &s1, 2 );
		}

		fileSystem->CloseFile( wO );
		fileSystem->CloseFile( lL );
		fileSystem->CloseFile( rL );

		fileSystem->RemoveFile( aviDemoPath + "channel_right.raw" );
		fileSystem->RemoveFile( aviDemoPath + "channel_left.raw" );
	}

	soundSystemLocal.SetMute( false );
}

//==============================================================================


/*
===================
idSoundWorldLocal::ResolveOrigin

Find out of the sound is completely occluded by a closed door portal, or
the virtual sound origin position at the portal closest to the listener.
  this is called by the main thread

dist is the distance from the orignial sound origin to the current portal that enters soundArea
def->distance is the distance we are trying to reduce.

If there is no path through open portals from the sound to the listener, def->distance will remain
set at maxDistance
===================
*/
static const int MAX_PORTAL_TRACE_DEPTH = 10;

void idSoundWorldLocal::ResolveOrigin( const int stackDepth, const soundPortalTrace_t *prevStack, const int soundArea, const float dist, const idVec3& soundOrigin, idSoundEmitterLocal *def ) {

	if ( dist >= def->distance ) {
		// we can't possibly hear the sound through this chain of portals
		return;
	}

	if ( soundArea == listenerArea ) {
		float	fullDist = dist + (soundOrigin - listenerQU).LengthFast();
		if ( fullDist < def->distance ) {
			def->distance = fullDist;
			def->spatializedOrigin = soundOrigin;
		}
		return;
	}

	if ( stackDepth == MAX_PORTAL_TRACE_DEPTH ) {
		// don't spend too much time doing these calculations in big maps
		return;
	}

	soundPortalTrace_t newStack;
	newStack.portalArea = soundArea;
	newStack.prevStack = prevStack;

	int numPortals = rw->NumPortalsInArea( soundArea );
	for( int p = 0; p < numPortals; p++ ) {
		exitPortal_t re = rw->GetPortal( soundArea, p );

		float	occlusionDistance = 0;

		// air blocking windows will block sound like closed doors
		if ( (re.blockingBits & ( PS_BLOCK_VIEW | PS_BLOCK_AIR ) ) ) {
			// we could just completely cut sound off, but reducing the volume works better
			// continue;
			occlusionDistance = idSoundSystemLocal::s_doorDistanceAdd.GetFloat();
		}

		// what area are we about to go look at
		int otherArea = re.areas[0];
		if ( re.areas[0] == soundArea ) {
			otherArea = re.areas[1];
		}

		// if this area is already in our portal chain, don't bother looking into it
		const soundPortalTrace_t *prev;
		for ( prev = prevStack ; prev ; prev = prev->prevStack ) {
			if ( prev->portalArea == otherArea ) {
				break;
			}
		}
		if ( prev ) {
			continue;
		}

		// pick a point on the portal to serve as our virtual sound origin
#if 1
		idVec3	source;

		idPlane	pl;
		re.w->GetPlane( pl );

		float	scale;
		idVec3	dir = listenerQU - soundOrigin;
		if ( !pl.RayIntersection( soundOrigin, dir, scale ) ) {
			source = re.w->GetCenter();
		} else {
			source = soundOrigin + scale * dir;

			// if this point isn't inside the portal edges, slide it in
			for ( int i = 0 ; i < re.w->GetNumPoints() ; i++ ) {
				int j = ( i + 1 ) % re.w->GetNumPoints();
				idVec3	edgeDir = (*(re.w))[j].ToVec3() - (*(re.w))[i].ToVec3();
				idVec3	edgeNormal;

				edgeNormal.Cross( pl.Normal(), edgeDir );

				idVec3	fromVert = source - (*(re.w))[j].ToVec3();

				float	d = edgeNormal * fromVert;
				if ( d > 0 ) {
					// move it in
					float div = edgeNormal.Normalize();
					d /= div;

					source -= d * edgeNormal;
				}
			}
		}
#else
		// clip the ray from the listener to the center of the portal by
		// all the portal edge planes, then project that point (or the original if not clipped)
		// onto the portal plane to get the spatialized origin

		idVec3	start = listenerQU;
		idVec3	mid = re.w->GetCenter();
		bool	wasClipped = false;

		for ( int i = 0 ; i < re.w->GetNumPoints() ; i++ ) {
			int j = ( i + 1 ) % re.w->GetNumPoints();
			idVec3	v1 = (*(re.w))[j].ToVec3() - soundOrigin;
			idVec3	v2 = (*(re.w))[i].ToVec3() - soundOrigin;

			v1.Normalize();
			v2.Normalize();

			idVec3	edgeNormal;

			edgeNormal.Cross( v1, v2 );

			idVec3	fromVert = start - soundOrigin;
			float	d1 = edgeNormal * fromVert;

			if ( d1 > 0.0f ) {
				fromVert = mid - (*(re.w))[j].ToVec3();
				float d2 = edgeNormal * fromVert;

				// move it in
				float	f = d1 / ( d1 - d2 );

				idVec3	clipped = start * ( 1.0f - f ) + mid * f;
				start = clipped;
				wasClipped = true;
			}
		}

		idVec3	source;
		if ( wasClipped ) {
			// now project it onto the portal plane
			idPlane	pl;
			re.w->GetPlane( pl );

			float	f1 = pl.Distance( start );
			float	f2 = pl.Distance( soundOrigin );

			float	f = f1 / ( f1 - f2 );
			source = start * ( 1.0f - f ) + soundOrigin * f;
		} else {
			source = soundOrigin;
		}
#endif

		idVec3 tlen = source - soundOrigin;
		float tlenLength = tlen.LengthFast();

		ResolveOrigin( stackDepth+1, &newStack, otherArea, dist+tlenLength+occlusionDistance, source, def );
	}
}


/*
===================
idSoundWorldLocal::PlaceListener

  this is called by the main thread
===================
*/
void idSoundWorldLocal::PlaceListener( const idVec3& origin, const idMat3& axis, 
									const int listenerId, const int gameTime, const idStr& areaName  ) {

	int current44kHzTime;

	if ( !soundSystemLocal.isInitialized ) {
		return;
	}

	if ( pause44kHz >= 0 ){
		return;
	}

	if ( writeDemo ) {
		writeDemo->WriteInt( DS_SOUND );
		writeDemo->WriteInt( SCMD_PLACE_LISTENER );
		writeDemo->WriteVec3( origin );
		writeDemo->WriteMat3( axis );
		writeDemo->WriteInt( listenerId );
		writeDemo->WriteInt( gameTime );
	}

	current44kHzTime = soundSystemLocal.GetCurrent44kHzTime();

	// we usually expect gameTime to be increasing by 16 or 32 msec, but when
	// a cinematic is fast-forward skipped through, it can jump by a significant
	// amount, while the hardware 44kHz position will not have changed accordingly,
	// which would make sounds (like long character speaches) continue from the
	// old time.  Fix this by killing all non-looping sounds
	if ( gameTime > gameMsec + 500 ) {
		OffsetSoundTime( - ( gameTime - gameMsec ) * 0.001f * 44100.0f );
	}

	gameMsec = gameTime;
	if ( fpa[0] ) {
		// exactly 30 fps so the wave file can be used for exact video frames
		game44kHz = idMath::FtoiFast( gameMsec * ( ( 1000.0f / 60.0f ) / 16.0f ) * 0.001f * 44100.0f );
	} else {
		// the normal 16 msec / frame
		game44kHz = idMath::FtoiFast( gameMsec * 0.001f * 44100.0f );
	}

	listenerPrivateId = listenerId;

	listenerQU = origin;							// Doom units
	listenerPos = origin * DOOM_TO_METERS;			// meters
	listenerAxis = axis;
	listenerAreaName = areaName;
	listenerAreaName.ToLower();

	if ( rw ) {
		listenerArea = rw->PointInArea( listenerQU );	// where are we?
	} else {
		listenerArea = 0;
	}

	if ( listenerArea < 0 ) {
		return;
	}

	ForegroundUpdate( current44kHzTime );
}

/*
==================
idSoundWorldLocal::ForegroundUpdate
==================
*/
void idSoundWorldLocal::ForegroundUpdate( int current44kHzTime ) {
	int j, k;
	idSoundEmitterLocal	*def;

	if ( !soundSystemLocal.isInitialized ) {
		return;
	}

	Sys_EnterCriticalSection();

	// if we are recording an AVI demo, don't use hardware time
	if ( fpa[0] ) {
		current44kHzTime = lastAVI44kHz;
	}

	//
	// check to see if each sound is visible or not
	// speed up by checking maxdistance to origin
	// although the sound may still need to play if it has
	// just become occluded so it can ramp down to 0
	//
	for ( j = 1; j < emitters.Num(); j++ ) {
		def = emitters[j];

		if ( def->removeStatus >= REMOVE_STATUS_SAMPLEFINISHED ) {
			continue;
		}

		// see if our last channel just finished
		def->CheckForCompletion( current44kHzTime );

		if ( !def->playing ) {
			continue;
		}

		// update virtual origin / distance, etc
		def->Spatialize( listenerPos, listenerArea, rw );

		// per-sound debug options
		if ( idSoundSystemLocal::s_drawSounds.GetInteger() && rw ) {
			if ( def->distance < def->maxDistance || idSoundSystemLocal::s_drawSounds.GetInteger() > 1 ) {
				idBounds ref;
				ref.Clear();
				ref.AddPoint( idVec3( -10, -10, -10 ) );
				ref.AddPoint( idVec3(  10,  10,  10 ) );
				float vis = (1.0f - (def->distance / def->maxDistance));

				// draw a box
				rw->DebugBounds( idVec4( vis, 0.25f, vis, vis ), ref, def->origin );

				// draw an arrow to the audible position, possible a portal center
				if ( def->origin != def->spatializedOrigin ) {
					rw->DebugArrow( colorRed, def->origin, def->spatializedOrigin, 4 );
				}

				// draw the index
				idVec3	textPos = def->origin;
				textPos[2] -= 8;
				rw->DrawText( va("%i", def->index), textPos, 0.1f, idVec4(1,0,0,1), listenerAxis );
				textPos[2] += 8;

				// run through all the channels
				for ( k = 0; k < SOUND_MAX_CHANNELS ; k++ ) {
					idSoundChannel	*chan = &def->channels[k];

					// see if we have a sound triggered on this channel
					if ( !chan->triggerState ) {
						continue;
					}

					char	text[1024];
					float	min = chan->parms.minDistance;
					float	max = chan->parms.maxDistance;
					const char	*defaulted = chan->leadinSample->defaultSound ? "(DEFAULTED)" : "";
					sprintf( text, "%s (%i/%i %i/%i)%s", chan->soundShader->GetName(), (int)def->distance,
						(int)def->realDistance, (int)min, (int)max, defaulted );
					rw->DrawText( text, textPos, 0.1f, idVec4(1,0,0,1), listenerAxis );
					textPos[2] += 8;
				}
			}
		}
	}

	Sys_LeaveCriticalSection();

	//
	// the sound meter
	//
	if ( idSoundSystemLocal::s_showLevelMeter.GetInteger() ) {
		const idMaterial *gui = declManager->FindMaterial( "guis/assets/soundmeter/audiobg", false );
		if ( gui ) {
			const shaderStage_t *foo = gui->GetStage(0);
			if ( !foo->texture.cinematic ) {
				((shaderStage_t *)foo)->texture.cinematic = new idSndWindow;
			}
		}
	}

	//
	// optionally dump out the generated sound
	//
	if ( fpa[0] ) {
		AVIUpdate();
	}
}

/*
===================
idSoundWorldLocal::OffsetSoundTime
===================
*/
void idSoundWorldLocal::OffsetSoundTime( int offset44kHz ) {
	int i, j;

	for ( i = 0; i < emitters.Num(); i++ ) {
		if ( emitters[i] == NULL ) {
			continue;
		}
		for ( j = 0; j < SOUND_MAX_CHANNELS; j++ ) {
			idSoundChannel *chan = &emitters[i]->channels[ j ];

			if ( !chan->triggerState ) {
				continue;
			}

			chan->trigger44kHzTime += offset44kHz;
		}
	}
}

/*
===================
idSoundWorldLocal::WriteToSaveGame
===================
*/
void idSoundWorldLocal::WriteToSaveGame( idFile *savefile ) {
	int i, j, num, currentSoundTime;
	const char *name;

	// the game soundworld is always paused at this point, save that time down
	if ( pause44kHz > 0 ) {
		currentSoundTime = pause44kHz;
	} else {
		currentSoundTime = soundSystemLocal.GetCurrent44kHzTime();
	}

	// write listener data
	savefile->WriteVec3(listenerQU);
	savefile->WriteMat3(listenerAxis);
	savefile->WriteInt(listenerPrivateId);
	savefile->WriteInt(gameMsec);
	savefile->WriteInt(game44kHz);
	savefile->WriteInt(currentSoundTime);

	num = emitters.Num();
	savefile->WriteInt(num);

	for ( i = 1; i < emitters.Num(); i++ ) {
		idSoundEmitterLocal *def = emitters[i];

		if ( def->removeStatus != REMOVE_STATUS_ALIVE ) {
			int skip = -1;
			savefile->Write( &skip, sizeof( skip ) );
			continue;
		}

		savefile->WriteInt(i);

		// Write the emitter data
		savefile->WriteVec3( def->origin );
		savefile->WriteInt( def->listenerId );
		WriteToSaveGameSoundShaderParams( savefile, &def->parms );
		savefile->WriteFloat( def->amplitude );
		savefile->WriteInt( def->ampTime );
		for (int k = 0; k < SOUND_MAX_CHANNELS; k++) 
			WriteToSaveGameSoundChannel( savefile, &def->channels[k] );
		savefile->WriteFloat( def->distance );
		savefile->WriteBool( def->hasShakes );
		savefile->WriteInt( def->lastValidPortalArea );
		savefile->WriteFloat( def->maxDistance );
		savefile->WriteBool( def->playing );
		savefile->WriteFloat( def->realDistance );
		savefile->WriteInt( def->removeStatus );
		savefile->WriteVec3( def->spatializedOrigin );

		// write the channel data
		for( j = 0; j < SOUND_MAX_CHANNELS; j++ ) {
			idSoundChannel *chan = &def->channels[ j ];

			// Write out any sound commands for this def
			if ( chan->triggerState && chan->soundShader && chan->leadinSample ) {

				savefile->WriteInt( j );

				// write the pointers out separately
				name = chan->soundShader->GetName();
				savefile->WriteString( name );
				
				name = chan->leadinSample->name;
				savefile->WriteString( name );
			}
		}

		// End active channels with -1
		int end = -1;
		savefile->WriteInt( end );
	}

	// new in Doom3 v1.2
	savefile->Write( &slowmoActive, sizeof( slowmoActive ) );
	savefile->Write( &slowmoSpeed, sizeof( slowmoSpeed ) );
	savefile->Write( &enviroSuitActive, sizeof( enviroSuitActive ) );
}

/*
 ===================
 idSoundWorldLocal::WriteToSaveGameSoundShaderParams
 ===================
 */
void idSoundWorldLocal::WriteToSaveGameSoundShaderParams( idFile *saveGame, soundShaderParms_t *params ) {
	saveGame->WriteFloat(params->minDistance);
	saveGame->WriteFloat(params->maxDistance);
	saveGame->WriteFloat(params->volume);
	saveGame->WriteFloat(params->shakes);
	saveGame->WriteInt(params->soundShaderFlags);
	saveGame->WriteInt(params->soundClass);
}

/*
 ===================
 idSoundWorldLocal::WriteToSaveGameSoundChannel
 ===================
 */
void idSoundWorldLocal::WriteToSaveGameSoundChannel( idFile *saveGame, idSoundChannel *ch ) {
	saveGame->WriteBool( ch->triggerState );
	saveGame->WriteUnsignedChar( 0 );
	saveGame->WriteUnsignedChar( 0 );
	saveGame->WriteUnsignedChar( 0 );
	saveGame->WriteInt( ch->trigger44kHzTime );
	saveGame->WriteInt( ch->triggerGame44kHzTime );
	WriteToSaveGameSoundShaderParams( saveGame, &ch->parms );
	saveGame->WriteInt( (int)ch->leadinSample );
	saveGame->WriteInt( ch->triggerChannel );
	saveGame->WriteInt( (int)ch->soundShader );
	saveGame->WriteInt( (int)ch->decoder );
	saveGame->WriteFloat(ch->diversity );
	saveGame->WriteFloat(ch->lastVolume );
	for (int m = 0; m < 6; m++)
		saveGame->WriteFloat( ch->lastV[m] );
	saveGame->WriteInt( ch->channelFade.fadeStart44kHz );
	saveGame->WriteInt( ch->channelFade.fadeEnd44kHz );
	saveGame->WriteFloat( ch->channelFade.fadeStartVolume );
	saveGame->WriteFloat( ch->channelFade.fadeEndVolume );
}

/*
===================
idSoundWorldLocal::ReadFromSaveGame
===================
*/
void idSoundWorldLocal::ReadFromSaveGame( idFile *savefile ) {
	int i, num, handle, listenerId, gameTime, channel;
	int savedSoundTime, currentSoundTime, soundTimeOffset;
	idSoundEmitterLocal *def;
	idVec3 origin;
	idMat3 axis;
	idStr soundShader;

	ClearAllSoundEmitters();

	savefile->ReadVec3( origin );
	savefile->ReadMat3( axis );
	savefile->ReadInt( listenerId );
	savefile->ReadInt( gameTime );
	savefile->ReadInt( game44kHz );
	savefile->ReadInt( savedSoundTime );

	// we will adjust the sound starting times from those saved with the demo
	currentSoundTime = soundSystemLocal.GetCurrent44kHzTime();
	soundTimeOffset = currentSoundTime - savedSoundTime;

	// at the end of the level load we unpause the sound world and adjust the sound starting times once more
	pause44kHz = currentSoundTime;

	// place listener
	PlaceListener( origin, axis, listenerId, gameTime, "Undefined" );

	// make sure there are enough
	// slots to read the saveGame in.  We don't shrink the list
	// if there are extras.
	savefile->ReadInt( num );

	while( emitters.Num() < num ) {
		def = new idSoundEmitterLocal;
		def->index = emitters.Append( def );
		def->soundWorld = this;
	}

	// read in the state
	for ( i = 1; i < num; i++ ) {

		savefile->ReadInt( handle );
		if ( handle < 0 ) {
			continue;
		}
		if ( handle != i ) {
			common->Error( "idSoundWorldLocal::ReadFromSaveGame: index mismatch" );
		}
		def = emitters[i];

		def->removeStatus = REMOVE_STATUS_ALIVE;
		def->playing = true;		// may be reset by the first UpdateListener

		savefile->ReadVec3( def->origin );
		savefile->ReadInt( def->listenerId );
		ReadFromSaveGameSoundShaderParams( savefile, &def->parms );
		savefile->ReadFloat( def->amplitude );
		savefile->ReadInt( def->ampTime );
		for (int k = 0; k < SOUND_MAX_CHANNELS; k++) 
			ReadFromSaveGameSoundChannel( savefile, &def->channels[k] );
		savefile->ReadFloat( def->distance );
		savefile->ReadBool( def->hasShakes );
		savefile->ReadInt( def->lastValidPortalArea );
		savefile->ReadFloat( def->maxDistance );
		savefile->ReadBool( def->playing );
		savefile->ReadFloat( def->realDistance );
		savefile->ReadInt( (int&)def->removeStatus );
		savefile->ReadVec3( def->spatializedOrigin );

		// read the individual channels
		savefile->ReadInt( channel );

		while ( channel >= 0 ) {
			if ( channel > SOUND_MAX_CHANNELS ) {
				common->Error( "idSoundWorldLocal::ReadFromSaveGame: channel > SOUND_MAX_CHANNELS" );
			}

			idSoundChannel *chan = &def->channels[channel];

			if ( chan->decoder != NULL ) {
				// The pointer in the save file is not valid, so we grab a new one
				chan->decoder = idSampleDecoder::Alloc();
			}

			savefile->ReadString( soundShader );
			chan->soundShader = declManager->FindSound( soundShader );

			savefile->ReadString( soundShader );
			// load savegames with s_noSound 1
			if ( soundSystemLocal.soundCache ) {
				chan->leadinSample = soundSystemLocal.soundCache->FindSound( soundShader, false );
			} else {
				chan->leadinSample = NULL;
			}

			// adjust the hardware start time
			chan->trigger44kHzTime += soundTimeOffset;

			// make sure we start up the hardware voice if needed
			chan->triggered = chan->triggerState;
			chan->openalStreamingOffset = currentSoundTime - chan->trigger44kHzTime;

			// adjust the hardware fade time
			if ( chan->channelFade.fadeStart44kHz != 0 ) {
				chan->channelFade.fadeStart44kHz += soundTimeOffset;
				chan->channelFade.fadeEnd44kHz += soundTimeOffset;
			}

			// next command
			savefile->ReadInt( channel );
		}
	}

	if ( session->GetSaveGameVersion() >= 17 ) {
		savefile->Read( &slowmoActive, sizeof( slowmoActive ) );
		savefile->Read( &slowmoSpeed, sizeof( slowmoSpeed ) );
		savefile->Read( &enviroSuitActive, sizeof( enviroSuitActive ) );
	} else {
		slowmoActive		= false;
		slowmoSpeed			= 0;
		enviroSuitActive	= false;
	}
}

/*
 ===================
 idSoundWorldLocal::ReadFromSaveGameSoundShaderParams
 ===================
 */
void idSoundWorldLocal::ReadFromSaveGameSoundShaderParams( idFile *saveGame, soundShaderParms_t *params ) {
	saveGame->ReadFloat(params->minDistance);
	saveGame->ReadFloat(params->maxDistance);
	saveGame->ReadFloat(params->volume);
	saveGame->ReadFloat(params->shakes);
	saveGame->ReadInt(params->soundShaderFlags);
	saveGame->ReadInt(params->soundClass);
}

/*
 ===================
 idSoundWorldLocal::ReadFromSaveGameSoundChannel
 ===================
 */
void idSoundWorldLocal::ReadFromSaveGameSoundChannel( idFile *saveGame, idSoundChannel *ch ) {
	saveGame->ReadBool( ch->triggerState );
	char tmp;
	saveGame->ReadChar( tmp );
	saveGame->ReadChar( tmp );
	saveGame->ReadChar( tmp );
	saveGame->ReadInt( ch->trigger44kHzTime );
	saveGame->ReadInt( ch->triggerGame44kHzTime );
	ReadFromSaveGameSoundShaderParams( saveGame, &ch->parms );
	saveGame->ReadInt( (int&)ch->leadinSample );
	saveGame->ReadInt( ch->triggerChannel );
	saveGame->ReadInt( (int&)ch->soundShader );
	saveGame->ReadInt( (int&)ch->decoder );
	saveGame->ReadFloat(ch->diversity );
	saveGame->ReadFloat(ch->lastVolume );
	for (int m = 0; m < 6; m++)
		saveGame->ReadFloat( ch->lastV[m] );
	saveGame->ReadInt( ch->channelFade.fadeStart44kHz );
	saveGame->ReadInt( ch->channelFade.fadeEnd44kHz );
	saveGame->ReadFloat( ch->channelFade.fadeStartVolume );
	saveGame->ReadFloat( ch->channelFade.fadeEndVolume );
}

/*
===================
idSoundWorldLocal::EmitterForIndex
===================
*/
idSoundEmitter	*idSoundWorldLocal::EmitterForIndex( int index ) {
	if ( index == 0 ) {
		return NULL;
	}
	if ( index >= emitters.Num() ) {
		common->Error( "idSoundWorldLocal::EmitterForIndex: %i > %i", index, emitters.Num() );
	}
	return emitters[index];
}

/*
===============
idSoundWorldLocal::StopAllSounds

  this is called from the main thread
===============
*/
void idSoundWorldLocal::StopAllSounds() {

	for ( int i = 0; i < emitters.Num(); i++ ) {
		idSoundEmitterLocal * def = emitters[i];
		def->StopSound( SCHANNEL_ANY );
	}
}

/*
===============
idSoundWorldLocal::Pause
===============
*/
void idSoundWorldLocal::Pause( void ) {
	if ( pause44kHz >= 0 ) {
		common->Warning( "idSoundWorldLocal::Pause: already paused" );
		return;
	}

	pause44kHz = soundSystemLocal.GetCurrent44kHzTime();
}

/*
===============
idSoundWorldLocal::UnPause
===============
*/
void idSoundWorldLocal::UnPause( void ) {
	int offset44kHz;

	if ( pause44kHz < 0 ) {
		common->Warning( "idSoundWorldLocal::UnPause: not paused" );
		return;
	}

	offset44kHz = soundSystemLocal.GetCurrent44kHzTime() - pause44kHz;
	OffsetSoundTime( offset44kHz );

	pause44kHz = -1;
}

/*
===============
idSoundWorldLocal::IsPaused
===============
*/
bool idSoundWorldLocal::IsPaused( void ) {
	return ( pause44kHz >= 0 );
}

/*
===============
idSoundWorldLocal::PlayShaderDirectly

start a music track

  this is called from the main thread
===============
*/
void idSoundWorldLocal::PlayShaderDirectly( const char *shaderName, int channel ) {

	if ( localSound && channel == -1 ) {
		localSound->StopSound( SCHANNEL_ANY );
	} else if ( localSound ) {
		localSound->StopSound( channel );
	}

	if ( !shaderName || !shaderName[0] ) {
		return;
	}

	const idSoundShader *shader = declManager->FindSound( shaderName );
	if ( !shader ) {
		return;
	}

	if ( !localSound ) {
		localSound = AllocLocalSoundEmitter();
	}

	static idRandom	rnd;
	float	diversity = rnd.RandomFloat();

	localSound->StartSound( shader, ( channel == -1 ) ? SCHANNEL_ONE : channel , diversity, SSF_GLOBAL );

	// in case we are at the console without a game doing updates, force an update
	ForegroundUpdate( soundSystemLocal.GetCurrent44kHzTime() );
}

/*
===============
idSoundWorldLocal::CalcEars

Determine the volumes from each speaker for a given sound emitter
===============
*/
void idSoundWorldLocal::CalcEars( int numSpeakers, idVec3 spatializedOrigin, idVec3 listenerPos,
								 idMat3 listenerAxis, float ears[6], float spatialize ) {
	idVec3 svec = spatializedOrigin - listenerPos;
	idVec3 ovec;

	ovec[0] = svec * listenerAxis[0];
	ovec[1] = svec * listenerAxis[1];
	ovec[2] = svec * listenerAxis[2];

	ovec.Normalize();

	if ( numSpeakers == 6 ) {
		static idVec3	speakerVector[6] = {
			idVec3(  0.707f,  0.707f, 0.0f ),	// front left
			idVec3(  0.707f, -0.707f, 0.0f ),	// front right
			idVec3(  0.707f,  0.0f,   0.0f ),	// front center
			idVec3(  0.0f,    0.0f,   0.0f ),	// sub
			idVec3( -0.707f,  0.707f, 0.0f ),	// rear left
			idVec3( -0.707f, -0.707f, 0.0f )	// rear right
		};
		for ( int i = 0 ; i < 6 ; i++ ) {
			if ( i == 3 ) {
				ears[i] = idSoundSystemLocal::s_subFraction.GetFloat();		// subwoofer
				continue;
			}
			float dot = ovec * speakerVector[i];
			ears[i] = (idSoundSystemLocal::s_dotbias6.GetFloat() + dot) / ( 1.0f + idSoundSystemLocal::s_dotbias6.GetFloat() );
			if ( ears[i] < idSoundSystemLocal::s_minVolume6.GetFloat() ) {
				ears[i] = idSoundSystemLocal::s_minVolume6.GetFloat();
			}
		}
	} else {
		float dot = ovec.y;
		float dotBias = idSoundSystemLocal::s_dotbias2.GetFloat();

		// when we are inside the minDistance, start reducing the amount of spatialization
		// so NPC voices right in front of us aren't quieter that off to the side
		dotBias += ( idSoundSystemLocal::s_spatializationDecay.GetFloat() - dotBias ) * ( 1.0f - spatialize );

		ears[0] = (idSoundSystemLocal::s_dotbias2.GetFloat() + dot) / ( 1.0f + dotBias );
		ears[1] = (idSoundSystemLocal::s_dotbias2.GetFloat() - dot) / ( 1.0f + dotBias );

		if ( ears[0] < idSoundSystemLocal::s_minVolume2.GetFloat() ) {
			ears[0] = idSoundSystemLocal::s_minVolume2.GetFloat();
		}
		if ( ears[1] < idSoundSystemLocal::s_minVolume2.GetFloat() ) {
			ears[1] = idSoundSystemLocal::s_minVolume2.GetFloat();
		}

		ears[2] = 
		ears[3] = 
		ears[4] = 
		ears[5] = 0.0f;
	}
}

/*
===============
idSoundWorldLocal::AddChannelContribution

Adds the contribution of a single sound channel to finalMixBuffer
this is called from the async thread

Mixes MIXBUFFER_SAMPLES samples starting at current44kHz sample time into
finalMixBuffer
===============
*/
void idSoundWorldLocal::AddChannelContribution( idSoundEmitterLocal *sound, idSoundChannel *chan,
				   int current44kHz, int numSpeakers, float *finalMixBuffer ) {
	int j;
	float volume;

	//
	// get the sound definition and parameters from the entity
	//
	soundShaderParms_t *parms = &chan->parms;

	// assume we have a sound triggered on this channel
	assert( chan->triggerState );

	// fetch the actual wave file and see if it's valid
	idSoundSample *sample = chan->leadinSample;
	if ( sample == NULL ) {
		return;
	}

	// if you don't want to hear all the beeps from missing sounds
	if ( sample->defaultSound && !idSoundSystemLocal::s_playDefaultSound.GetBool() ) {
		return;
	}

	// get the actual shader
	const idSoundShader *shader = chan->soundShader;

	// this might happen if the foreground thread just deleted the sound emitter
	if ( !shader ) {
		return;
	}

	float maxd = parms->maxDistance;
	float mind = parms->minDistance;
	
	int  mask = shader->speakerMask;
	bool omni = ( parms->soundShaderFlags & SSF_OMNIDIRECTIONAL) != 0;
	bool looping = ( parms->soundShaderFlags & SSF_LOOPING ) != 0;
	bool global = ( parms->soundShaderFlags & SSF_GLOBAL ) != 0;
	bool noOcclusion = ( parms->soundShaderFlags & SSF_NO_OCCLUSION ) || !idSoundSystemLocal::s_useOcclusion.GetBool();

	// speed goes from 1 to 0.2
	if ( idSoundSystemLocal::s_slowAttenuate.GetBool() && slowmoActive && !chan->disallowSlow ) {
		maxd *= slowmoSpeed;
	}

	// stereo samples are always omni
	if ( sample->objectInfo.nChannels == 2 ) {
		omni = true;
	}

	// if the sound is playing from the current listener, it will not be spatialized at all
	if ( sound->listenerId == listenerPrivateId ) {
		global = true;
	}

	//
	// see if it's in range
	//
	
	// convert volumes from decibels to float scale

	// leadin volume scale for shattering lights
	// this isn't exactly correct, because the modified volume will get applied to
	// some initial chunk of the loop as well, because the volume is scaled for the
	// entire mix buffer
	if ( shader->leadinVolume && current44kHz - chan->trigger44kHzTime < sample->LengthIn44kHzSamples() ) {
		volume = soundSystemLocal.dB2Scale( shader->leadinVolume );
	} else {
		volume = soundSystemLocal.dB2Scale( parms->volume );
	}

	// global volume scale
	volume *= soundSystemLocal.dB2Scale( idSoundSystemLocal::s_volume.GetFloat() );


	// volume fading
	float	fadeDb = chan->channelFade.FadeDbAt44kHz( current44kHz );
	volume *= soundSystemLocal.dB2Scale( fadeDb );

	fadeDb = soundClassFade[parms->soundClass].FadeDbAt44kHz( current44kHz );
	volume *= soundSystemLocal.dB2Scale( fadeDb );
	

	//
	// if it's a global sound then
	// it's not affected by distance or occlusion
	//
	float	spatialize = 1;
	idVec3 spatializedOriginInMeters;
	if ( !global ) {
		float	dlen;

		if ( noOcclusion ) {
			// use the real origin and distance
			spatializedOriginInMeters = sound->origin * DOOM_TO_METERS;
			dlen = sound->realDistance;
		} else {
			// use the possibly portal-occluded origin and distance
			spatializedOriginInMeters = sound->spatializedOrigin * DOOM_TO_METERS;
			dlen = sound->distance;
		}

		// reduce volume based on distance
		if ( dlen >= maxd ) {
			volume = 0.0f;
		} else if ( dlen > mind ) {
			float frac = idMath::ClampFloat( 0.0f, 1.0f, 1.0f - ((dlen - mind) / (maxd - mind)));
			if ( idSoundSystemLocal::s_quadraticFalloff.GetBool() ) {
				frac *= frac;
			}
			volume *= frac;
		} else if ( mind > 0.0f ) {
			// we tweak the spatialization bias when you are inside the minDistance
			spatialize = dlen / mind;
		}
	}

	//
	// if it is a private sound, set the volume to zero
	// unless we match the listenerId
	//
	if ( parms->soundShaderFlags & SSF_PRIVATE_SOUND ) {
		if ( sound->listenerId != listenerPrivateId ) {
			volume = 0;
		}
	}
	if ( parms->soundShaderFlags & SSF_ANTI_PRIVATE_SOUND ) {
		if ( sound->listenerId == listenerPrivateId ) {
			volume = 0;
		}
	}

	//
	// do we have anything to add?
	//
	if ( volume < SND_EPSILON && chan->lastVolume < SND_EPSILON ) {
		return;
	}
	chan->lastVolume = volume;

	//
	// fetch the sound from the cache as 44kHz, 16 bit samples
	//
	int offset = current44kHz - chan->trigger44kHzTime;
	float inputSamples[MIXBUFFER_SAMPLES*2+16];
	float *alignedInputSamples = (float *) ( ( ( (int)inputSamples ) + 15 ) & ~15 );

	//
	// allocate and initialize hardware source
	// 
	if ( idSoundSystemLocal::useOpenAL && sound->removeStatus < REMOVE_STATUS_SAMPLEFINISHED ) {
		if ( !alIsSource( chan->openalSource ) ) {
			chan->openalSource = soundSystemLocal.AllocOpenALSource( chan, !chan->leadinSample->hardwareBuffer || !chan->soundShader->entries[0]->hardwareBuffer || looping, chan->leadinSample->objectInfo.nChannels == 2 );
		}

		if ( alIsSource( chan->openalSource ) ) {
			
			// stop source if needed..
			if ( chan->triggered ) {
				alSourceStop( chan->openalSource );
			}

			// update source parameters
			if ( global || omni ) {
				alSourcei( chan->openalSource, AL_SOURCE_RELATIVE, AL_TRUE);
				alSource3f( chan->openalSource, AL_POSITION, 0.0f, 0.0f, 0.0f );
				alSourcef( chan->openalSource, AL_GAIN, ( volume ) < ( 1.0f ) ? ( volume ) : ( 1.0f ) );
			} else {
				alSourcei( chan->openalSource, AL_SOURCE_RELATIVE, AL_FALSE);
				alSource3f( chan->openalSource, AL_POSITION, -spatializedOriginInMeters.y, spatializedOriginInMeters.z, -spatializedOriginInMeters.x );
				alSourcef( chan->openalSource, AL_GAIN, ( volume ) < ( 1.0f ) ? ( volume ) : ( 1.0f ) );
			}
			alSourcei( chan->openalSource, AL_LOOPING, ( looping && chan->soundShader->entries[0]->hardwareBuffer ) ? AL_TRUE : AL_FALSE );
#if !defined(MACOS_X)		
			alSourcef( chan->openalSource, AL_REFERENCE_DISTANCE, mind );
			alSourcef( chan->openalSource, AL_MAX_DISTANCE, maxd );
#endif
			alSourcef( chan->openalSource, AL_PITCH, ( slowmoActive && !chan->disallowSlow ) ? ( slowmoSpeed ) : ( 1.0f ) );
#if ID_OPENAL
			long lOcclusion = ( enviroSuitActive ? -1150 : 0);
			if ( soundSystemLocal.alEAXSet ) {
				soundSystemLocal.alEAXSet( &EAXPROPERTYID_EAX_Source, EAXSOURCE_OCCLUSION, chan->openalSource, &lOcclusion, sizeof(lOcclusion) );
			}
#endif
			if ( ( !looping && chan->leadinSample->hardwareBuffer ) || ( looping && chan->soundShader->entries[0]->hardwareBuffer ) ) {
				// handle uncompressed (non streaming) single shot and looping sounds
				if ( chan->triggered ) {
					alSourcei( chan->openalSource, AL_BUFFER, looping ? chan->soundShader->entries[0]->openalBuffer : chan->leadinSample->openalBuffer );
				}
			} else {
				ALint finishedbuffers;
				ALuint buffers[3];

				// handle streaming sounds (decode on the fly) both single shot AND looping
				if ( chan->triggered ) {
					alSourcei( chan->openalSource, AL_BUFFER, NULL );
					alDeleteBuffers( 3, &chan->lastopenalStreamingBuffer[0] );
					chan->lastopenalStreamingBuffer[0] = chan->openalStreamingBuffer[0];
					chan->lastopenalStreamingBuffer[1] = chan->openalStreamingBuffer[1];
					chan->lastopenalStreamingBuffer[2] = chan->openalStreamingBuffer[2];
					alGenBuffers( 3, &chan->openalStreamingBuffer[0] );
					if ( soundSystemLocal.alEAXSetBufferMode ) {
						soundSystemLocal.alEAXSetBufferMode( 3, &chan->openalStreamingBuffer[0], alGetEnumValue( ID_ALCHAR "AL_STORAGE_ACCESSIBLE" ) );
					}
					buffers[0] = chan->openalStreamingBuffer[0];
					buffers[1] = chan->openalStreamingBuffer[1];
					buffers[2] = chan->openalStreamingBuffer[2];
					finishedbuffers = 3;
				} else {
					alGetSourcei( chan->openalSource, AL_BUFFERS_PROCESSED, &finishedbuffers );
					alSourceUnqueueBuffers( chan->openalSource, finishedbuffers, &buffers[0] );
					if ( finishedbuffers == 3 ) {
						chan->triggered = true;
					}
				}

				for ( j = 0; j < finishedbuffers; j++ ) {
					chan->GatherChannelSamples( chan->openalStreamingOffset * sample->objectInfo.nChannels, MIXBUFFER_SAMPLES * sample->objectInfo.nChannels, alignedInputSamples );
					for ( int i = 0; i < ( MIXBUFFER_SAMPLES * sample->objectInfo.nChannels ); i++ ) {
						if ( alignedInputSamples[i] < -32768.0f )
							((short *)alignedInputSamples)[i] = -32768;
						else if ( alignedInputSamples[i] > 32767.0f )
							((short *)alignedInputSamples)[i] = 32767;
						else
							((short *)alignedInputSamples)[i] = idMath::FtoiFast( alignedInputSamples[i] );
					}
					alBufferData( buffers[j], chan->leadinSample->objectInfo.nChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, alignedInputSamples, MIXBUFFER_SAMPLES * sample->objectInfo.nChannels * sizeof( short ), 44100 );
					chan->openalStreamingOffset += MIXBUFFER_SAMPLES;
				}

				if ( finishedbuffers ) {
					alSourceQueueBuffers( chan->openalSource, finishedbuffers, &buffers[0] );
				}
			}
			
			// (re)start if needed..
			if ( chan->triggered ) {
				alSourcePlay( chan->openalSource );
				chan->triggered = false;
			}
		}
	} else {

		if ( slowmoActive && !chan->disallowSlow ) {
			idSlowChannel slow = sound->GetSlowChannel( chan );

			slow.AttachSoundChannel( chan );

				if ( sample->objectInfo.nChannels == 2 ) {
					// need to add a stereo path, but very few samples go through this
					memset( alignedInputSamples, 0, sizeof( alignedInputSamples[0] ) * MIXBUFFER_SAMPLES * 2 );
				} else {
					slow.GatherChannelSamples( offset, MIXBUFFER_SAMPLES, alignedInputSamples );
				}

			sound->SetSlowChannel( chan, slow );
		} else {
			sound->ResetSlowChannel( chan );

			// if we are getting a stereo sample adjust accordingly
			if ( sample->objectInfo.nChannels == 2 ) {
				// we should probably check to make sure any looping is also to a stereo sample...
				chan->GatherChannelSamples( offset*2, MIXBUFFER_SAMPLES*2, alignedInputSamples );
			} else {
				chan->GatherChannelSamples( offset, MIXBUFFER_SAMPLES, alignedInputSamples );
			}
		}

		//
		// work out the left / right ear values
		//
		float	ears[6];
		if ( global || omni ) {
			// same for all speakers
			for ( int i = 0 ; i < 6 ; i++ ) {
				ears[i] = idSoundSystemLocal::s_globalFraction.GetFloat() * volume;
			}
			ears[3] = idSoundSystemLocal::s_subFraction.GetFloat() * volume;		// subwoofer

		} else {
			CalcEars( numSpeakers, spatializedOriginInMeters, listenerPos, listenerAxis, ears, spatialize );

			for ( int i = 0 ; i < 6 ; i++ ) {
				ears[i] *= volume;
			}
		}

		// if the mask is 0, it really means do every channel
		if ( !mask ) {
			mask = 255;
		}
		// cleared mask bits set the mix volume to zero
		for ( int i = 0 ; i < 6 ; i++ ) {
			if ( !(mask & ( 1 << i ) ) ) {
				ears[i] = 0;
			}
		}

		// if sounds are generally normalized, using a mixing volume over 1.0 will
		// almost always cause clipping noise.  If samples aren't normalized, there
		// is a good call to allow overvolumes
		if ( idSoundSystemLocal::s_clipVolumes.GetBool() && !( parms->soundShaderFlags & SSF_UNCLAMPED )  ) {
			for ( int i = 0 ; i < 6 ; i++ ) {
				if ( ears[i] > 1.0f ) {
					ears[i] = 1.0f;
				}
			}
		}

		// if this is the very first mixing block, set the lastV
		// to the current volume
		if ( current44kHz == chan->trigger44kHzTime ) {
			for ( j = 0 ; j < 6 ; j++ ) {
				chan->lastV[j] = ears[j];
			}
		}

		if ( numSpeakers == 6 ) {
			if ( sample->objectInfo.nChannels == 1 ) {
				SIMDProcessor->MixSoundSixSpeakerMono( finalMixBuffer, alignedInputSamples, MIXBUFFER_SAMPLES, chan->lastV, ears );
			} else {
				SIMDProcessor->MixSoundSixSpeakerStereo( finalMixBuffer, alignedInputSamples, MIXBUFFER_SAMPLES, chan->lastV, ears );
			}
		} else {
			if ( sample->objectInfo.nChannels == 1 ) {
				SIMDProcessor->MixSoundTwoSpeakerMono( finalMixBuffer, alignedInputSamples, MIXBUFFER_SAMPLES, chan->lastV, ears );
			} else {
				SIMDProcessor->MixSoundTwoSpeakerStereo( finalMixBuffer, alignedInputSamples, MIXBUFFER_SAMPLES, chan->lastV, ears );
			}
		}

		for ( j = 0 ; j < 6 ; j++ ) {
			chan->lastV[j] = ears[j];
		}

	}

	soundSystemLocal.soundStats.activeSounds++;

}

/*
===============
idSoundWorldLocal::FindAmplitude

  this is called from the main thread

  if listenerPosition is NULL, this is being used for shader parameters,
  like flashing lights and glows based on sound level.  Otherwise, it is being used for
  the screen-shake on a player.

  This doesn't do the portal-occlusion currently, because it would have to reset all the defs
  which would be problematic in multiplayer
===============
*/
float idSoundWorldLocal::FindAmplitude( idSoundEmitterLocal *sound, const int localTime, const idVec3 *listenerPosition, 
									   const s_channelType channel, bool shakesOnly ) {
	int		i, j;
	soundShaderParms_t *parms;
	float	volume;
	int		activeChannelCount;
	static const int AMPLITUDE_SAMPLES = MIXBUFFER_SAMPLES/8;
	float	sourceBuffer[AMPLITUDE_SAMPLES];
	float	sumBuffer[AMPLITUDE_SAMPLES];
	// work out the distance from the listener to the emitter
	float	dlen;

	if ( !sound->playing ) {
		return 0;
	}

	if ( listenerPosition ) {
		// this doesn't do the portal spatialization
		idVec3 dist = sound->origin - *listenerPosition;
		dlen = dist.Length();
		dlen *= DOOM_TO_METERS;
	} else {
		dlen = 1;
	}

	activeChannelCount = 0;

	for ( i = 0; i < SOUND_MAX_CHANNELS ; i++ ) {
		idSoundChannel	*chan = &sound->channels[ i ];

		if ( !chan->triggerState ) {
			continue;
		}

		if ( channel != SCHANNEL_ANY && chan->triggerChannel != channel) {
			continue;
		}

		parms = &chan->parms;

		int	localTriggerTimes = chan->trigger44kHzTime;

		bool looping = ( parms->soundShaderFlags & SSF_LOOPING ) != 0;

		// check for screen shakes
		float shakes = parms->shakes;
		if ( shakesOnly && shakes <= 0.0f ) {
			continue;
		}

		//
		// calculate volume
		//
		if ( !listenerPosition ) {
			// just look at the raw wav data for light shader evaluation
			volume = 1.0;
		} else {
			volume = parms->volume;
			volume = soundSystemLocal.dB2Scale( volume );
			if ( shakesOnly ) {
				volume *= shakes;
			}

			if ( listenerPosition && !( parms->soundShaderFlags & SSF_GLOBAL )  ) {			
				// check for overrides
				float maxd = parms->maxDistance;
				float mind = parms->minDistance;

				if ( dlen >= maxd ) {
					volume = 0.0f;
				} else if ( dlen > mind ) {
					float frac = idMath::ClampFloat( 0, 1, 1.0f - ((dlen - mind) / (maxd - mind)));
					if ( idSoundSystemLocal::s_quadraticFalloff.GetBool() ) {
						frac *= frac;
					}
					volume *= frac;
				}
			}
		}

		if ( volume <= 0 ) {
			continue;
		}

		//
		// fetch the sound from the cache
		// this doesn't handle stereo samples correctly...
		//
		if ( !listenerPosition && chan->parms.soundShaderFlags & SSF_NO_FLICKER ) {
			// the NO_FLICKER option is to allow a light to still play a sound, but
			// not have it effect the intensity
			for ( j = 0 ; j < (AMPLITUDE_SAMPLES); j++ ) {
				sourceBuffer[j] = j & 1 ? 32767.0f : -32767.0f;
			}
		} else {
			int offset = (localTime - localTriggerTimes);	// offset in samples
			int size = ( looping ? chan->soundShader->entries[0]->LengthIn44kHzSamples() : chan->leadinSample->LengthIn44kHzSamples() );
			short *amplitudeData = (short *)( looping ? chan->soundShader->entries[0]->amplitudeData : chan->leadinSample->amplitudeData );
	
			if ( amplitudeData ) {
				// when the amplitudeData is present use that fill a dummy sourceBuffer
				// this is to allow for amplitude based effect on hardware audio solutions
				if ( looping ) offset %= size;
				if ( offset < size ) {
					for ( j = 0 ; j < (AMPLITUDE_SAMPLES); j++ ) {
						sourceBuffer[j] = j & 1 ? amplitudeData[ ( offset / 512 ) * 2 ] : amplitudeData[ ( offset / 512 ) * 2 + 1 ];
					}
				}
			} else {
				// get actual sample data
				chan->GatherChannelSamples( offset, AMPLITUDE_SAMPLES, sourceBuffer );
			}
		}
		activeChannelCount++;
		if ( activeChannelCount == 1 ) {
			// store to the buffer
			for( j = 0; j < AMPLITUDE_SAMPLES; j++ ) {
				sumBuffer[ j ] = volume * sourceBuffer[ j ];
			}
		} else {
			// add to the buffer
			for( j = 0; j < AMPLITUDE_SAMPLES; j++ ) {
				sumBuffer[ j ] += volume * sourceBuffer[ j ];
			}
		}
	}

	if ( activeChannelCount == 0 ) {
		return 0.0;
	}

	float high = -32767.0f;
	float low = 32767.0f;

	// use a 20th of a second
	for( i = 0; i < (AMPLITUDE_SAMPLES); i++ ) {
		float fabval = sumBuffer[i];
		if ( high < fabval ) {
			high = fabval;
		}
		if ( low > fabval ) {
			low = fabval;
		}
	}

	float sout;
	sout = atan( (high - low) / 32767.0f) / DEG2RAD(45);

	return sout;
}

/*
=================
idSoundWorldLocal::FadeSoundClasses

fade all sounds in the world with a given shader soundClass
to is in Db (sigh), over is in seconds
=================
*/
void	idSoundWorldLocal::FadeSoundClasses( const int soundClass, const float to, const float over ) {
	if ( soundClass < 0 || soundClass >= SOUND_MAX_CLASSES ) {
		common->Error( "idSoundWorldLocal::FadeSoundClasses: bad soundClass %i", soundClass );
	}

	idSoundFade	*fade = &soundClassFade[ soundClass ];

	int	length44kHz = soundSystemLocal.MillisecondsToSamples( over * 1000 );

	// if it is already fading to this volume at this rate, don't change it
	if ( fade->fadeEndVolume == to && 
		fade->fadeEnd44kHz - fade->fadeStart44kHz == length44kHz ) {
		return;
	}

	int	start44kHz;

	if ( fpa[0] ) {
		// if we are recording an AVI demo, don't use hardware time
		start44kHz = lastAVI44kHz + MIXBUFFER_SAMPLES;
	} else {
		start44kHz = soundSystemLocal.GetCurrent44kHzTime() + MIXBUFFER_SAMPLES;
	}

	// fade it
	fade->fadeStartVolume = fade->FadeDbAt44kHz( start44kHz );
	fade->fadeStart44kHz = start44kHz;
	fade->fadeEnd44kHz = start44kHz + length44kHz;
	fade->fadeEndVolume = to;
}

/*
=================
idSoundWorldLocal::SetSlowmo
=================
*/
void idSoundWorldLocal::SetSlowmo( bool active ) {
	slowmoActive = active;
}

/*
=================
idSoundWorldLocal::SetSlowmoSpeed
=================
*/
void idSoundWorldLocal::SetSlowmoSpeed( float speed ) {
	slowmoSpeed = speed;
}

/*
=================
idSoundWorldLocal::SetEnviroSuit
=================
*/
void idSoundWorldLocal::SetEnviroSuit( bool active ) {
	enviroSuitActive = active;
}

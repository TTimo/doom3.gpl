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
===================
idSoundFade::Clear
===================
*/
void idSoundFade::Clear() {
	fadeStart44kHz = 0;
	fadeEnd44kHz = 0;
	fadeStartVolume = 0;
	fadeEndVolume = 0;
}

/*
===================
idSoundFade::FadeDbAt44kHz
===================
*/
float idSoundFade::FadeDbAt44kHz( int current44kHz ) {
	float	fadeDb;

	if ( current44kHz >= fadeEnd44kHz ) {
		fadeDb = fadeEndVolume;
	} else if ( current44kHz > fadeStart44kHz ) {
		float fraction = ( fadeEnd44kHz - fadeStart44kHz );
		float over = ( current44kHz - fadeStart44kHz );
		fadeDb = fadeStartVolume + ( fadeEndVolume - fadeStartVolume ) * over / fraction;
	} else {
		fadeDb = fadeStartVolume;
	}
	return fadeDb;
}

//========================================================================


/*
=======================
GeneratePermutedList

Fills in elements[0] .. elements[numElements-1] with a permutation of
0 .. numElements-1 based on the permute parameter

numElements == 3
maxPermute = 6
permute 0 = 012
permute 1 = 021
permute 2 = 102
permute 3 = 120
permute 4 = 201
permute 5 = 210
=======================
*/
void PermuteList_r( int *list, int listLength, int permute, int maxPermute ) {
	if ( listLength < 2 ) {
		return;
	}
	permute %= maxPermute;
	int	swap = permute * listLength / maxPermute;
	int	old = list[swap];
	list[swap] = list[0];
	list[0] = old;

	maxPermute /= listLength;
	PermuteList_r( list + 1, listLength - 1, permute, maxPermute );
}

int	Factorial( int val ) {
	int	fact = val;
	while ( val > 1 ) {
		val--;
		fact *= val;
	}
	return fact;
}

void GeneratePermutedList( int *list, int listLength, int permute ) {	
	for ( int i = 0 ; i < listLength ; i++ ) {
		list[i] = i;
	}

	// we can't calculate > 12 factorial, so we can't easily build a permuted list
	if ( listLength > 12 ) {
		return;
	}

	// calculate listLength factorial
	int		maxPermute = Factorial( listLength );

	// recursively permute
	PermuteList_r( list, listLength, permute, maxPermute );
}

void TestPermutations( void ) {
	int	list[SOUND_MAX_LIST_WAVS];

	for ( int len = 1 ; len < 5 ; len++ ) {
		common->Printf( "list length: %i\n", len );

		int	max = Factorial( len );
		for ( int j = 0 ; j < max * 2 ; j++ ) {
			GeneratePermutedList( list, len, j );
			common->Printf( "%4i : ", j );
			for ( int k = 0 ; k < len ; k++ ) {
				common->Printf( "%i", list[k] );
			}
			common->Printf( "\n" );
		}
	}
}

//=====================================================================================

/*
===================
idSoundChannel::idSoundChannel
===================
*/
idSoundChannel::idSoundChannel( void ) {
	decoder = NULL;
	Clear();
}

/*
===================
idSoundChannel::~idSoundChannel
===================
*/
idSoundChannel::~idSoundChannel( void ) {
	Clear();
}

/*
===================
idSoundChannel::Clear
===================
*/
void idSoundChannel::Clear( void ) {
	int j;

	Stop();
	soundShader = NULL;
	lastVolume = 0.0f;
	triggerChannel = SCHANNEL_ANY;
	channelFade.Clear();
	diversity = 0.0f;
	leadinSample = NULL;
	trigger44kHzTime = 0;
	for( j = 0; j < 6; j++ ) {
		lastV[j] = 0.0f;
	}
	memset( &parms, 0, sizeof(parms) );

	triggered = false;
	openalSource = NULL;
	openalStreamingOffset = 0;
	openalStreamingBuffer[0] = openalStreamingBuffer[1] = openalStreamingBuffer[2] = 0;
	lastopenalStreamingBuffer[0] = lastopenalStreamingBuffer[1] = lastopenalStreamingBuffer[2] = 0;
}

/*
===================
idSoundChannel::Start
===================
*/
void idSoundChannel::Start( void ) {
	triggerState = true;
	if ( decoder == NULL ) {
		decoder = idSampleDecoder::Alloc();
	}
}

/*
===================
idSoundChannel::Stop
===================
*/
void idSoundChannel::Stop( void ) {
	triggerState = false;
	if ( decoder != NULL ) {
		idSampleDecoder::Free( decoder );
		decoder = NULL;
	}
}

/*
===================
idSoundChannel::ALStop
===================
*/
void idSoundChannel::ALStop( void ) {
	if ( idSoundSystemLocal::useOpenAL ) {

		if ( alIsSource( openalSource ) ) {
			alSourceStop( openalSource );
			alSourcei( openalSource, AL_BUFFER, 0 );
			soundSystemLocal.FreeOpenALSource( openalSource );
		}

		if ( openalStreamingBuffer[0] && openalStreamingBuffer[1] && openalStreamingBuffer[2] ) {
			alGetError();
			alDeleteBuffers( 3, &openalStreamingBuffer[0] );
			if ( alGetError() == AL_NO_ERROR ) {
				openalStreamingBuffer[0] = openalStreamingBuffer[1] = openalStreamingBuffer[2] = 0;
			}
		}

		if ( lastopenalStreamingBuffer[0] && lastopenalStreamingBuffer[1] && lastopenalStreamingBuffer[2] ) {
			alGetError();
			alDeleteBuffers( 3, &lastopenalStreamingBuffer[0] );
			if ( alGetError() == AL_NO_ERROR ) {
				lastopenalStreamingBuffer[0] = lastopenalStreamingBuffer[1] = lastopenalStreamingBuffer[2] = 0;
			}
		}
	}
}

/*
===================
idSoundChannel::GatherChannelSamples

Will always return 44kHz samples for the given range, even if it deeply looped or
out of the range of the unlooped samples.  Handles looping between multiple different
samples and leadins
===================
*/
void idSoundChannel::GatherChannelSamples( int sampleOffset44k, int sampleCount44k, float *dest ) const {
	float	*dest_p = dest;
	int		len;

//Sys_DebugPrintf( "msec:%i sample:%i : %i : %i\n", Sys_Milliseconds(), soundSystemLocal.GetCurrent44kHzTime(), sampleOffset44k, sampleCount44k );	//!@#

	// negative offset times will just zero fill
	if ( sampleOffset44k < 0 ) {
		len = -sampleOffset44k;
		if ( len > sampleCount44k ) {
			len = sampleCount44k;
		}
		memset( dest_p, 0, len * sizeof( dest_p[0] ) );
		dest_p += len;
		sampleCount44k -= len;
		sampleOffset44k += len;
	}
	
	// grab part of the leadin sample
	idSoundSample *leadin = leadinSample;
	if ( !leadin || sampleOffset44k < 0 || sampleCount44k <= 0 ) {
		memset( dest_p, 0, sampleCount44k * sizeof( dest_p[0] ) );
		return;
	}

	if ( sampleOffset44k < leadin->LengthIn44kHzSamples() ) {
		len = leadin->LengthIn44kHzSamples() - sampleOffset44k;
		if ( len > sampleCount44k ) {
			len = sampleCount44k;
		}

		// decode the sample
		decoder->Decode( leadin, sampleOffset44k, len, dest_p );

		dest_p += len;
		sampleCount44k -= len;
		sampleOffset44k += len;
	}

	// if not looping, zero fill any remaining spots
	if ( !soundShader || !( parms.soundShaderFlags & SSF_LOOPING ) ) {
		memset( dest_p, 0, sampleCount44k * sizeof( dest_p[0] ) );
		return;
	}

	// fill the remainder with looped samples
	idSoundSample *loop = soundShader->entries[0];

	if ( !loop ) {
		memset( dest_p, 0, sampleCount44k * sizeof( dest_p[0] ) );
		return;
	}

	sampleOffset44k -= leadin->LengthIn44kHzSamples();

	while( sampleCount44k > 0 ) {
		int totalLen = loop->LengthIn44kHzSamples();

		sampleOffset44k %= totalLen;

		len = totalLen - sampleOffset44k;
		if ( len > sampleCount44k ) {
			len = sampleCount44k;
		}

		// decode the sample
		decoder->Decode( loop, sampleOffset44k, len, dest_p );

		dest_p += len;
		sampleCount44k -= len;
		sampleOffset44k += len;
	}
}


//=====================================================================================

/*
===============
idSoundEmitterLocal::idSoundEmitterLocal
  
===============
*/
idSoundEmitterLocal::idSoundEmitterLocal( void ) {	
	soundWorld = NULL;
	Clear();
}

/*
===============
idSoundEmitterLocal::~idSoundEmitterLocal
===============
*/
idSoundEmitterLocal::~idSoundEmitterLocal( void ) {
	Clear();
}

/*
===============
idSoundEmitterLocal::Clear
===============
*/
void idSoundEmitterLocal::Clear( void ) {
	int i;

	for( i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
		channels[i].ALStop();
		channels[i].Clear();
	}

	removeStatus = REMOVE_STATUS_SAMPLEFINISHED;
	distance = 0.0f;

	lastValidPortalArea = -1;

	playing = false;
	hasShakes = false;
	ampTime = 0;								// last time someone queried
	amplitude = 0;
	maxDistance = 10.0f;						// meters
	spatializedOrigin.Zero();

	memset( &parms, 0, sizeof( parms ) );
}

/*
==================
idSoundEmitterLocal::OverrideParms
==================
*/
void idSoundEmitterLocal::OverrideParms( const soundShaderParms_t *base, 
									  const soundShaderParms_t *over, soundShaderParms_t *out ) {
	if ( !over ) {
		*out = *base;
		return;
	}
	if ( over->minDistance ) {
		out->minDistance = over->minDistance;
	} else {
		out->minDistance = base->minDistance;
	}
	if ( over->maxDistance ) {
		out->maxDistance = over->maxDistance;
	} else {
		out->maxDistance = base->maxDistance;
	}
	if ( over->shakes ) {
		out->shakes = over->shakes;
	} else {
		out->shakes = base->shakes;
	}
	if ( over->volume ) {
		out->volume = over->volume;
	} else {
		out->volume = base->volume;
	}
	if ( over->soundClass ) {
		out->soundClass = over->soundClass;
	} else {
		out->soundClass = base->soundClass;
	}
	out->soundShaderFlags = base->soundShaderFlags | over->soundShaderFlags;
}

/*
==================
idSoundEmitterLocal::CheckForCompletion

Checks to see if all the channels have completed, clearing the playing flag if necessary.
Sets the playing and shakes bools.
==================
*/
void idSoundEmitterLocal::CheckForCompletion( int current44kHzTime ) {
	bool hasActive;
	int i;

	hasActive = false;
	hasShakes = false;

	if ( playing ) {
		for ( i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
			idSoundChannel	*chan = &channels[i];

			if ( !chan->triggerState ) {
				continue;
			}
			const idSoundShader *shader = chan->soundShader;
			if ( !shader ) {
				continue;
			}

			// see if this channel has completed
			if ( !( chan->parms.soundShaderFlags & SSF_LOOPING ) ) {
				ALint state = AL_PLAYING;

				if ( idSoundSystemLocal::useOpenAL && alIsSource( chan->openalSource ) ) {
					alGetSourcei( chan->openalSource, AL_SOURCE_STATE, &state );
				}
				idSlowChannel slow = GetSlowChannel( chan );

				if ( soundWorld->slowmoActive && slow.IsActive() ) {
					if ( slow.GetCurrentPosition().time >= chan->leadinSample->LengthIn44kHzSamples() / 2 ) {
						chan->Stop();
						// if this was an onDemand sound, purge the sample now
						if ( chan->leadinSample->onDemand ) {
							chan->leadinSample->PurgeSoundSample();
						}
						continue;
					}
				} else if ( ( chan->trigger44kHzTime + chan->leadinSample->LengthIn44kHzSamples() < current44kHzTime ) || ( state == AL_STOPPED ) ) {
					chan->Stop();

					// free hardware resources
					chan->ALStop();
					
					// if this was an onDemand sound, purge the sample now
					if ( chan->leadinSample->onDemand ) {
						chan->leadinSample->PurgeSoundSample();
					}
					continue;
				}
			}

			// free decoder memory if no sound was decoded for a while
			if ( chan->decoder != NULL && chan->decoder->GetLastDecodeTime() < current44kHzTime - SOUND_DECODER_FREE_DELAY ) {
				chan->decoder->ClearDecoder();
			}

			hasActive = true;

			if ( chan->parms.shakes > 0.0f ) {
				hasShakes = true;
			}
		}
	}

	// mark the entire sound emitter as non-playing if there aren't any active channels
	if ( !hasActive ) {
		playing = false;
		if ( removeStatus == REMOVE_STATUS_WAITSAMPLEFINISHED ) {
			// this can now be reused by the next request for a new soundEmitter
			removeStatus = REMOVE_STATUS_SAMPLEFINISHED;
		}
	}
}

/*
===================
idSoundEmitterLocal::Spatialize

Called once each sound frame by the main thread from idSoundWorldLocal::PlaceOrigin
===================
*/
void idSoundEmitterLocal::Spatialize( idVec3 listenerPos, int listenerArea, idRenderWorld *rw ) {
	int			i;
	bool		hasActive = false;

	//
	// work out the maximum distance of all the playing channels
	//
	maxDistance = 0;

	for ( i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
		idSoundChannel	*chan = &channels[i];

		if ( !chan->triggerState ) {
			continue;
		}
		if ( chan->parms.maxDistance > maxDistance ) {
			maxDistance = chan->parms.maxDistance;
		}
	}

	//
	// work out where the sound comes from
	//
	idVec3 realOrigin = origin * DOOM_TO_METERS;
	idVec3 len = listenerPos - realOrigin;
	realDistance = len.LengthFast();

	if ( realDistance >= maxDistance ) {
		// no way to possibly hear it
		distance = realDistance;
		return;
	}

	//
	// work out virtual origin and distance, which may be from a portal instead of the actual origin
	//
	distance = maxDistance * METERS_TO_DOOM;
	if ( listenerArea == -1 ) {		// listener is outside the world
		return;
	}
	if ( rw ) {
		// we have a valid renderWorld
		int soundInArea = rw->PointInArea( origin );
		if ( soundInArea == -1 ) {
			if ( lastValidPortalArea == -1 ) {		// sound is outside the world
				distance = realDistance;
				spatializedOrigin = origin;			// sound is in our area
				return;
			}
			soundInArea = lastValidPortalArea;
		}
		lastValidPortalArea = soundInArea;
		if ( soundInArea == listenerArea ) {
			distance = realDistance;
			spatializedOrigin = origin;			// sound is in our area
			return;
		}

		soundWorld->ResolveOrigin( 0, NULL, soundInArea, 0.0f, origin, this );
		distance /= METERS_TO_DOOM;
	} else {
		// no portals available
		distance = realDistance;
		spatializedOrigin = origin;			// sound is in our area
	}
}

/*
===========================================================================================

PUBLIC FUNCTIONS

===========================================================================================
*/

/*
=====================
idSoundEmitterLocal::UpdateEmitter
=====================
*/
void idSoundEmitterLocal::UpdateEmitter( const idVec3 &origin, int listenerId, const soundShaderParms_t *parms ) {
	if ( !parms ) {
		common->Error( "idSoundEmitterLocal::UpdateEmitter: NULL parms" );
	}
	if ( soundWorld && soundWorld->writeDemo ) {
		soundWorld->writeDemo->WriteInt( DS_SOUND );
		soundWorld->writeDemo->WriteInt( SCMD_UPDATE );
		soundWorld->writeDemo->WriteInt( index );
		soundWorld->writeDemo->WriteVec3( origin );
		soundWorld->writeDemo->WriteInt( listenerId );
		soundWorld->writeDemo->WriteFloat( parms->minDistance );
		soundWorld->writeDemo->WriteFloat( parms->maxDistance );
		soundWorld->writeDemo->WriteFloat( parms->volume );
		soundWorld->writeDemo->WriteFloat( parms->shakes );
		soundWorld->writeDemo->WriteInt( parms->soundShaderFlags );
		soundWorld->writeDemo->WriteInt( parms->soundClass );
	}

	this->origin = origin;
	this->listenerId = listenerId;
	this->parms = *parms;

	// FIXME: change values on all channels?
}

/*
=====================
idSoundEmitterLocal::Free

They are never truly freed, just marked so they can be reused by the soundWorld
=====================
*/
void idSoundEmitterLocal::Free( bool immediate ) {
	if ( removeStatus != REMOVE_STATUS_ALIVE ) {
		return;
	}

	if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
		common->Printf( "FreeSound (%i,%i)\n",  index, (int)immediate );
	}
	if ( soundWorld && soundWorld->writeDemo ) {
		soundWorld->writeDemo->WriteInt( DS_SOUND );
		soundWorld->writeDemo->WriteInt( SCMD_FREE );
		soundWorld->writeDemo->WriteInt( index );
		soundWorld->writeDemo->WriteInt( immediate );
	}

	if ( !immediate ) {
		removeStatus = REMOVE_STATUS_WAITSAMPLEFINISHED;
	} else {
		Clear();
	}
}

/*
=====================
idSoundEmitterLocal::StartSound

returns the length of the started sound in msec
=====================
*/
int idSoundEmitterLocal::StartSound( const idSoundShader *shader, const s_channelType channel, float diversity, int soundShaderFlags, bool allowSlow ) {
	int i;

	if ( !shader ) {
		return 0;
	}

	if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
		common->Printf( "StartSound %ims (%i,%i,%s) = ", soundWorld->gameMsec, index, (int)channel, shader->GetName() );
	}

	if ( soundWorld && soundWorld->writeDemo ) {
		soundWorld->writeDemo->WriteInt( DS_SOUND );
		soundWorld->writeDemo->WriteInt( SCMD_START );
		soundWorld->writeDemo->WriteInt( index );

		soundWorld->writeDemo->WriteHashString( shader->GetName() );

		soundWorld->writeDemo->WriteInt( channel );
		soundWorld->writeDemo->WriteFloat( diversity );
		soundWorld->writeDemo->WriteInt( soundShaderFlags );
	}

	// build the channel parameters by taking the shader parms and optionally overriding
	soundShaderParms_t	chanParms;

	chanParms = shader->parms;
	OverrideParms( &chanParms, &this->parms, &chanParms );
	chanParms.soundShaderFlags |= soundShaderFlags;

	if ( chanParms.shakes > 0.0f ) {
		shader->CheckShakesAndOgg();
	}

	// this is the sample time it will be first mixed
	int start44kHz;
	
	if ( soundWorld->fpa[0] ) {
		// if we are recording an AVI demo, don't use hardware time
		start44kHz = soundWorld->lastAVI44kHz + MIXBUFFER_SAMPLES;
	} else {
		start44kHz = soundSystemLocal.GetCurrent44kHzTime() + MIXBUFFER_SAMPLES;
	}

	//
	// pick which sound to play from the shader
	//
	if ( !shader->numEntries ) {
		if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
			common->Printf( "no samples in sound shader\n" );
		}
		return 0;				// no sounds
	}
	int choice;

	// pick a sound from the list based on the passed diversity
	choice = (int)(diversity * shader->numEntries);
	if ( choice < 0 || choice >= shader->numEntries ) {
		choice = 0;
	}

	// bump the choice if the exact sound was just played and we are NO_DUPS
	if ( chanParms.soundShaderFlags & SSF_NO_DUPS ) {
		idSoundSample	*sample;
		if ( shader->leadins[ choice ] ) {
			sample = shader->leadins[ choice ];
		} else {
			sample = shader->entries[ choice ];
		}
		for( i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
			idSoundChannel	*chan = &channels[i];
			if ( chan->leadinSample == sample ) {
				choice = ( choice + 1 ) % shader->numEntries;
				break;
			}
		}
	}

	// PLAY_ONCE sounds will never be restarted while they are running
	if ( chanParms.soundShaderFlags & SSF_PLAY_ONCE ) {
		for( i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
			idSoundChannel	*chan = &channels[i];
			if ( chan->triggerState && chan->soundShader == shader ) {
				if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
					common->Printf( "PLAY_ONCE not restarting\n" );
				}
				return 0;
			}
		}
	}

	// never play the same sound twice with the same starting time, even
	// if they are on different channels
	for( i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
		idSoundChannel	*chan = &channels[i];
		if ( chan->triggerState && chan->soundShader == shader && chan->trigger44kHzTime == start44kHz ) {
			if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
				common->Printf( "already started this frame\n" );
			}
			return 0;
		}
	}

	Sys_EnterCriticalSection();

	// kill any sound that is currently playing on this channel
	if ( channel != SCHANNEL_ANY ) {
		for( i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
			idSoundChannel	*chan = &channels[i];
			if ( chan->triggerState && chan->soundShader && chan->triggerChannel == channel ) {
				if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
					common->Printf( "(override %s)", chan->soundShader->base->GetName() );
				}
				
				chan->Stop();

				// if this was an onDemand sound, purge the sample now
				if ( chan->leadinSample->onDemand ) {
					chan->ALStop();
					chan->leadinSample->PurgeSoundSample();
				}
				break;
			}
		}
	}

	// find a free channel to play the sound on
	idSoundChannel	*chan;
	for( i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
		chan = &channels[i];
		if ( !chan->triggerState ) {
			break;
		}
	}

	if ( i == SOUND_MAX_CHANNELS ) {
		// we couldn't find a channel for it
		Sys_LeaveCriticalSection();
		if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
			common->Printf( "no channels available\n" );
		}
		return 0;
	}

	chan = &channels[i];

	if ( shader->leadins[ choice ] ) {
		chan->leadinSample = shader->leadins[ choice ];
	} else {
		chan->leadinSample = shader->entries[ choice ];
	}

	// if the sample is onDemand (voice mails, etc), load it now
	if ( chan->leadinSample->purged ) {
		int		start = Sys_Milliseconds();
		chan->leadinSample->Load();
		int		end = Sys_Milliseconds();
		session->TimeHitch( end - start );
		// recalculate start44kHz, because loading may have taken a fair amount of time
		if ( !soundWorld->fpa[0] ) {
			start44kHz = soundSystemLocal.GetCurrent44kHzTime() + MIXBUFFER_SAMPLES;
		}
	}

	if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
		common->Printf( "'%s'\n", chan->leadinSample->name.c_str() );
	}

	if ( idSoundSystemLocal::s_skipHelltimeFX.GetBool() ) {
		chan->disallowSlow = true;
	} else {
		chan->disallowSlow = !allowSlow;
	}

	ResetSlowChannel( chan );

	// the sound will start mixing in the next async mix block
	chan->triggered = true;
	chan->openalStreamingOffset = 0;
	chan->trigger44kHzTime = start44kHz;
	chan->parms = chanParms;
	chan->triggerGame44kHzTime = soundWorld->game44kHz;
	chan->soundShader = shader;
	chan->triggerChannel = channel;
	chan->Start();

	// we need to start updating the def and mixing it in
	playing = true;

	// spatialize it immediately, so it will start the next mix block
	// even if that happens before the next PlaceOrigin()
	Spatialize( soundWorld->listenerPos, soundWorld->listenerArea, soundWorld->rw );

	// return length of sound in milliseconds
	int length = chan->leadinSample->LengthIn44kHzSamples();

	if ( chan->leadinSample->objectInfo.nChannels == 2 ) {
		length /= 2;	// stereo samples
	}

	// adjust the start time based on diversity for looping sounds, so they don't all start
	// at the same point
	if ( chan->parms.soundShaderFlags & SSF_LOOPING && !chan->leadinSample->LengthIn44kHzSamples() ) {
		chan->trigger44kHzTime -= diversity * length;
		chan->trigger44kHzTime &= ~7;		// so we don't have to worry about the 22kHz and 11kHz expansions
											// starting in fractional samples
		chan->triggerGame44kHzTime -= diversity * length;
		chan->triggerGame44kHzTime &= ~7;
	}
	
	length *= 1000 / (float)PRIMARYFREQ;

	Sys_LeaveCriticalSection();

	return length;
}

/*
===================
idSoundEmitterLocal::ModifySound
===================
*/
void idSoundEmitterLocal::ModifySound( const s_channelType channel, const soundShaderParms_t *parms ) {
	if ( !parms ) {
		common->Error( "idSoundEmitterLocal::ModifySound: NULL parms" );
	}
	if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
		common->Printf( "ModifySound(%i,%i)\n", index, channel );
	}
	if ( soundWorld && soundWorld->writeDemo ) {
		soundWorld->writeDemo->WriteInt( DS_SOUND );
		soundWorld->writeDemo->WriteInt( SCMD_MODIFY );
		soundWorld->writeDemo->WriteInt( index );
		soundWorld->writeDemo->WriteInt( channel );
		soundWorld->writeDemo->WriteFloat( parms->minDistance );
		soundWorld->writeDemo->WriteFloat( parms->maxDistance );
		soundWorld->writeDemo->WriteFloat( parms->volume );
		soundWorld->writeDemo->WriteFloat( parms->shakes );
		soundWorld->writeDemo->WriteInt( parms->soundShaderFlags );
		soundWorld->writeDemo->WriteInt( parms->soundClass );
	}

	for ( int i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
		idSoundChannel	*chan = &channels[i];

		if ( !chan->triggerState ) {
			continue;
		}
		if ( channel != SCHANNEL_ANY && chan->triggerChannel != channel ) {
			continue;
		}

		OverrideParms( &chan->parms, parms, &chan->parms );

		if ( chan->parms.shakes > 0.0f && chan->soundShader != NULL ) {
			chan->soundShader->CheckShakesAndOgg();
		}
	}
}

/*
===================
idSoundEmitterLocal::StopSound

can pass SCHANNEL_ANY
===================
*/
void idSoundEmitterLocal::StopSound( const s_channelType channel ) {
	int i;

	if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
		common->Printf( "StopSound(%i,%i)\n", index, channel );
	}

	if ( soundWorld && soundWorld->writeDemo ) {
		soundWorld->writeDemo->WriteInt( DS_SOUND );
		soundWorld->writeDemo->WriteInt( SCMD_STOP );
		soundWorld->writeDemo->WriteInt( index );
		soundWorld->writeDemo->WriteInt( channel );
	}

	Sys_EnterCriticalSection();

	for( i = 0; i < SOUND_MAX_CHANNELS; i++ ) {
		idSoundChannel	*chan = &channels[i];

		if ( !chan->triggerState ) {
			continue;
		}
		if ( channel != SCHANNEL_ANY && chan->triggerChannel != channel ) {
			continue;
		}

		// stop it
		chan->Stop();

		// free hardware resources
		chan->ALStop();

		// if this was an onDemand sound, purge the sample now
		if ( chan->leadinSample->onDemand ) {
			chan->leadinSample->PurgeSoundSample();
		}

		chan->leadinSample = NULL;
		chan->soundShader = NULL;
	}

	Sys_LeaveCriticalSection();
}

/*
===================
idSoundEmitterLocal::FadeSound

to is in Db (sigh), over is in seconds
===================
*/
void idSoundEmitterLocal::FadeSound( const s_channelType channel, float to, float over ) {
	if ( idSoundSystemLocal::s_showStartSound.GetInteger() ) {
		common->Printf( "FadeSound(%i,%i,%f,%f )\n", index, channel, to, over );
	}
	if ( !soundWorld ) {
		return;
	}
	if ( soundWorld->writeDemo ) {
		soundWorld->writeDemo->WriteInt( DS_SOUND );
		soundWorld->writeDemo->WriteInt( SCMD_FADE );
		soundWorld->writeDemo->WriteInt( index );
		soundWorld->writeDemo->WriteInt( channel );
		soundWorld->writeDemo->WriteFloat( to );
		soundWorld->writeDemo->WriteFloat( over );
	}

	int	start44kHz;

	if ( soundWorld->fpa[0] ) {
		// if we are recording an AVI demo, don't use hardware time
		start44kHz = soundWorld->lastAVI44kHz + MIXBUFFER_SAMPLES;
	} else {
		start44kHz = soundSystemLocal.GetCurrent44kHzTime() + MIXBUFFER_SAMPLES;
	}

	int	length44kHz = soundSystemLocal.MillisecondsToSamples( over * 1000 );

	for( int i = 0; i < SOUND_MAX_CHANNELS ; i++ ) {
		idSoundChannel	*chan = &channels[i];

		if ( !chan->triggerState ) {
			continue;
		}
		if ( channel != SCHANNEL_ANY && chan->triggerChannel != channel ) {
			continue;
		}

		// if it is already fading to this volume at this rate, don't change it
		if ( chan->channelFade.fadeEndVolume == to && 
			chan->channelFade.fadeEnd44kHz - chan->channelFade.fadeStart44kHz == length44kHz ) {
			continue;
		}

		// fade it
		chan->channelFade.fadeStartVolume = chan->channelFade.FadeDbAt44kHz( start44kHz );
		chan->channelFade.fadeStart44kHz = start44kHz;
		chan->channelFade.fadeEnd44kHz = start44kHz + length44kHz;
		chan->channelFade.fadeEndVolume = to;
	}
}

/*
===================
idSoundEmitterLocal::CurrentlyPlaying
===================
*/
bool idSoundEmitterLocal::CurrentlyPlaying( void ) const {
	return playing;
}

/*
===================
idSoundEmitterLocal::Index
===================
*/
int	idSoundEmitterLocal::Index( void ) const {
	return index;
}

/*
===================
idSoundEmitterLocal::CurrentAmplitude

this is called from the main thread by the material shader system
to allow lights and surface flares to vary with the sound amplitude
===================
*/
float idSoundEmitterLocal::CurrentAmplitude( void ) {
	if ( idSoundSystemLocal::s_constantAmplitude.GetFloat() >= 0.0f ) {
		return idSoundSystemLocal::s_constantAmplitude.GetFloat();
	}

	if ( removeStatus > REMOVE_STATUS_WAITSAMPLEFINISHED ) {
		return 0.0;
	}

	int localTime = soundSystemLocal.GetCurrent44kHzTime();

	// see if we can use our cached value
	if ( ampTime == localTime ) {
		return amplitude;
	}

	// calculate a new value
	ampTime = localTime;
	amplitude = soundWorld->FindAmplitude( this, localTime, NULL, SCHANNEL_ANY, false );

	return amplitude;
}

/*
===================
idSoundEmitterLocal::GetSlowChannel
===================
*/
idSlowChannel idSoundEmitterLocal::GetSlowChannel( const idSoundChannel *chan ) {
	return slowChannels[chan - channels];
}

/*
===================
idSoundEmitterLocal::SetSlowChannel
===================
*/
void idSoundEmitterLocal::SetSlowChannel( const idSoundChannel *chan, idSlowChannel slow ) {
	slowChannels[chan - channels] = slow;
}

/*
===================
idSoundEmitterLocal::ResetSlowChannel
===================
*/
void idSoundEmitterLocal::ResetSlowChannel( const idSoundChannel *chan ) {
	int index = chan - channels;
	slowChannels[index].Reset();
}

/*
===================
idSlowChannel::Reset
===================
*/
void idSlowChannel::Reset() {
	memset( this, 0, sizeof( *this ) );

	this->chan = chan;

	curPosition.Set( 0 );
	newPosition.Set( 0 );

	curSampleOffset = -10000;
	newSampleOffset = -10000;

	triggerOffset = 0;
}

/*
===================
idSlowChannel::AttachSoundChannel
===================
*/
void idSlowChannel::AttachSoundChannel( const idSoundChannel *chan ) {
	this->chan = chan;
}

/*
===================
idSlowChannel::GetSlowmoSpeed
===================
*/
float idSlowChannel::GetSlowmoSpeed() {
	idSoundWorldLocal *sw = static_cast<idSoundWorldLocal*>( soundSystemLocal.GetPlayingSoundWorld() );

	if ( sw ) {
		return sw->slowmoSpeed;
	} else {
		return 0;
	}
}

/*
===================
idSlowChannel::GenerateSlowChannel
===================
*/
void idSlowChannel::GenerateSlowChannel( FracTime& playPos, int sampleCount44k, float* finalBuffer ) {
	idSoundWorldLocal *sw = static_cast<idSoundWorldLocal*>( soundSystemLocal.GetPlayingSoundWorld() );
	float in[MIXBUFFER_SAMPLES+3], out[MIXBUFFER_SAMPLES+3], *src, *spline, slowmoSpeed;
	int i, neededSamples, orgTime, zeroedPos, count = 0;

	src = in + 2;
	spline = out + 2;

	if ( sw ) {
		slowmoSpeed = sw->slowmoSpeed;
	}
	else {
		slowmoSpeed = 1;
	}

	neededSamples = sampleCount44k * slowmoSpeed + 4;
	orgTime = playPos.time;

	// get the channel's samples
	chan->GatherChannelSamples( playPos.time * 2, neededSamples, src );
	for ( i = 0; i < neededSamples >> 1; i++ ) {
		spline[i] = src[i*2];
	}

	// interpolate channel
	zeroedPos = playPos.time;
	playPos.time = 0;

	for ( i = 0; i < sampleCount44k >> 1; i++, count += 2 ) {
		float val;
		val = spline[playPos.time];
		src[i] = val;
		playPos.Increment( slowmoSpeed );
	}

	// lowpass filter
	float *in_p = in + 2, *out_p = out + 2;
	int numSamples = sampleCount44k >> 1;

	lowpass.GetContinuitySamples( in_p[-1], in_p[-2], out_p[-1], out_p[-2] );
	lowpass.SetParms( slowmoSpeed * 15000, 1.2f );

	for ( int i = 0, count = 0; i < numSamples; i++, count += 2 ) {
		lowpass.ProcessSample( in_p + i, out_p + i );
		finalBuffer[count] = finalBuffer[count+1] = out[i];
	}

	lowpass.SetContinuitySamples( in_p[numSamples-2], in_p[numSamples-3], out_p[numSamples-2], out_p[numSamples-3] );

	playPos.time += zeroedPos;
}

/*
===================
idSlowChannel::GatherChannelSamples
===================
*/
void idSlowChannel::GatherChannelSamples( int sampleOffset44k, int sampleCount44k, float *dest ) {
	int state = 0;

	// setup chan
	active = true;
	newSampleOffset = sampleOffset44k >> 1;

	// set state
	if ( newSampleOffset < curSampleOffset ) {
		state = PLAYBACK_RESET;
	} else if ( newSampleOffset > curSampleOffset ) {
		state = PLAYBACK_ADVANCING;
	}

	if ( state == PLAYBACK_RESET ) {
		curPosition.Set( newSampleOffset );
	}

	// set current vars
	curSampleOffset = newSampleOffset;
	newPosition = curPosition;

	// do the slow processing
	GenerateSlowChannel( newPosition, sampleCount44k, dest );

	// finish off
	if ( state == PLAYBACK_ADVANCING )
		curPosition = newPosition;
}

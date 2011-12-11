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

#ifdef ID_DEDICATED
idCVar idSoundSystemLocal::s_noSound( "s_noSound", "1", CVAR_SOUND | CVAR_BOOL | CVAR_ROM, "" );
#else
idCVar idSoundSystemLocal::s_noSound( "s_noSound", "0", CVAR_SOUND | CVAR_BOOL | CVAR_NOCHEAT, "" );
#endif
idCVar idSoundSystemLocal::s_quadraticFalloff( "s_quadraticFalloff", "1", CVAR_SOUND | CVAR_BOOL, "" );
idCVar idSoundSystemLocal::s_drawSounds( "s_drawSounds", "0", CVAR_SOUND | CVAR_INTEGER, "", 0, 2, idCmdSystem::ArgCompletion_Integer<0,2> );
idCVar idSoundSystemLocal::s_showStartSound( "s_showStartSound", "0", CVAR_SOUND | CVAR_BOOL, "" );
idCVar idSoundSystemLocal::s_useOcclusion( "s_useOcclusion", "1", CVAR_SOUND | CVAR_BOOL, "" );
idCVar idSoundSystemLocal::s_maxSoundsPerShader( "s_maxSoundsPerShader", "0", CVAR_SOUND | CVAR_ARCHIVE, "", 0, 10, idCmdSystem::ArgCompletion_Integer<0,10> );
idCVar idSoundSystemLocal::s_showLevelMeter( "s_showLevelMeter", "0", CVAR_SOUND | CVAR_BOOL, "" );
idCVar idSoundSystemLocal::s_constantAmplitude( "s_constantAmplitude", "-1", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_minVolume6( "s_minVolume6", "0", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_dotbias6( "s_dotbias6", "0.8", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_minVolume2( "s_minVolume2", "0.25", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_dotbias2( "s_dotbias2", "1.1", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_spatializationDecay( "s_spatializationDecay", "2", CVAR_SOUND | CVAR_ARCHIVE | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_reverse( "s_reverse", "0", CVAR_SOUND | CVAR_ARCHIVE | CVAR_BOOL, "" );
idCVar idSoundSystemLocal::s_meterTopTime( "s_meterTopTime", "2000", CVAR_SOUND | CVAR_ARCHIVE | CVAR_INTEGER, "" );
idCVar idSoundSystemLocal::s_volume( "s_volume_dB", "0", CVAR_SOUND | CVAR_ARCHIVE | CVAR_FLOAT, "volume in dB" );
idCVar idSoundSystemLocal::s_playDefaultSound( "s_playDefaultSound", "1", CVAR_SOUND | CVAR_ARCHIVE | CVAR_BOOL, "play a beep for missing sounds" );
idCVar idSoundSystemLocal::s_subFraction( "s_subFraction", "0.75", CVAR_SOUND | CVAR_ARCHIVE | CVAR_FLOAT, "volume to subwoofer in 5.1" );
idCVar idSoundSystemLocal::s_globalFraction( "s_globalFraction", "0.8", CVAR_SOUND | CVAR_ARCHIVE | CVAR_FLOAT, "volume to all speakers when not spatialized" );
idCVar idSoundSystemLocal::s_doorDistanceAdd( "s_doorDistanceAdd", "150", CVAR_SOUND | CVAR_ARCHIVE | CVAR_FLOAT, "reduce sound volume with this distance when going through a door" );
idCVar idSoundSystemLocal::s_singleEmitter( "s_singleEmitter", "0", CVAR_SOUND | CVAR_INTEGER, "mute all sounds but this emitter" );
idCVar idSoundSystemLocal::s_numberOfSpeakers( "s_numberOfSpeakers", "2", CVAR_SOUND | CVAR_ARCHIVE, "number of speakers" );
idCVar idSoundSystemLocal::s_force22kHz( "s_force22kHz", "0", CVAR_SOUND | CVAR_BOOL, ""  );
idCVar idSoundSystemLocal::s_clipVolumes( "s_clipVolumes", "1", CVAR_SOUND | CVAR_BOOL, ""  );
idCVar idSoundSystemLocal::s_realTimeDecoding( "s_realTimeDecoding", "1", CVAR_SOUND | CVAR_BOOL | CVAR_INIT, "" );

idCVar idSoundSystemLocal::s_slowAttenuate( "s_slowAttenuate", "1", CVAR_SOUND | CVAR_BOOL, "slowmo sounds attenuate over shorted distance" );
idCVar idSoundSystemLocal::s_enviroSuitCutoffFreq( "s_enviroSuitCutoffFreq", "2000", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_enviroSuitCutoffQ( "s_enviroSuitCutoffQ", "2", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_reverbTime( "s_reverbTime", "1000", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_reverbFeedback( "s_reverbFeedback", "0.333", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_enviroSuitVolumeScale( "s_enviroSuitVolumeScale", "0.9", CVAR_SOUND | CVAR_FLOAT, "" );
idCVar idSoundSystemLocal::s_skipHelltimeFX( "s_skipHelltimeFX", "0", CVAR_SOUND | CVAR_BOOL, "" );

#if ID_OPENAL
// off by default. OpenAL DLL gets loaded on-demand
idCVar idSoundSystemLocal::s_libOpenAL( "s_libOpenAL", "openal32.dll", CVAR_SOUND | CVAR_ARCHIVE, "OpenAL DLL name/path" );
idCVar idSoundSystemLocal::s_useOpenAL( "s_useOpenAL", "0", CVAR_SOUND | CVAR_BOOL | CVAR_ARCHIVE, "use OpenAL" );
idCVar idSoundSystemLocal::s_useEAXReverb( "s_useEAXReverb", "0", CVAR_SOUND | CVAR_BOOL | CVAR_ARCHIVE, "use EAX reverb" );
idCVar idSoundSystemLocal::s_muteEAXReverb( "s_muteEAXReverb", "0", CVAR_SOUND | CVAR_BOOL, "mute eax reverb" );
idCVar idSoundSystemLocal::s_decompressionLimit( "s_decompressionLimit", "6", CVAR_SOUND | CVAR_INTEGER | CVAR_ARCHIVE, "specifies maximum uncompressed sample length in seconds" );
#else
idCVar idSoundSystemLocal::s_libOpenAL( "s_libOpenAL", "openal32.dll", CVAR_SOUND | CVAR_ARCHIVE, "OpenAL is not supported in this build" );
idCVar idSoundSystemLocal::s_useOpenAL( "s_useOpenAL", "0", CVAR_SOUND | CVAR_BOOL | CVAR_ROM, "OpenAL is not supported in this build" );
idCVar idSoundSystemLocal::s_useEAXReverb( "s_useEAXReverb", "0", CVAR_SOUND | CVAR_BOOL | CVAR_ROM, "EAX not available in this build" );
idCVar idSoundSystemLocal::s_muteEAXReverb( "s_muteEAXReverb", "0", CVAR_SOUND | CVAR_BOOL | CVAR_ROM, "mute eax reverb" );
idCVar idSoundSystemLocal::s_decompressionLimit( "s_decompressionLimit", "6", CVAR_SOUND | CVAR_INTEGER | CVAR_ROM, "specifies maximum uncompressed sample length in seconds" );
#endif

bool idSoundSystemLocal::useOpenAL = false;
bool idSoundSystemLocal::useEAXReverb = false;
int idSoundSystemLocal::EAXAvailable = -1;

idSoundSystemLocal	soundSystemLocal;
idSoundSystem	*soundSystem  = &soundSystemLocal;

/*
===============
SoundReloadSounds_f

  this is called from the main thread
===============
*/
void SoundReloadSounds_f( const idCmdArgs &args ) {
	if ( !soundSystemLocal.soundCache ) {
		return;
	}
	bool force = false;
	if ( args.Argc() == 2 ) {
		force = true;
	}
	soundSystem->SetMute( true );
	soundSystemLocal.soundCache->ReloadSounds( force );
	soundSystem->SetMute( false );
	common->Printf( "sound: changed sounds reloaded\n" );
}

/*
===============
ListSounds_f

Optional parameter to only list sounds containing that string
===============
*/
void ListSounds_f( const idCmdArgs &args ) {
	int i;
	const char	*snd = args.Argv( 1 );

	if ( !soundSystemLocal.soundCache ) {
		common->Printf( "No sound.\n" );
		return;
	}

	int	totalSounds = 0;
	int totalSamples = 0;
	int totalMemory = 0;
	int totalPCMMemory = 0;
	for( i = 0; i < soundSystemLocal.soundCache->GetNumObjects(); i++ ) {
		const idSoundSample *sample = soundSystemLocal.soundCache->GetObject(i);
		if ( !sample ) {
			continue;
		}
		if ( snd && sample->name.Find( snd, false ) < 0 ) {
			continue;
		}

		const waveformatex_t &info = sample->objectInfo;

		const char *stereo = ( info.nChannels == 2 ? "ST" : "  " );
		const char *format = ( info.wFormatTag == WAVE_FORMAT_TAG_OGG ) ? "OGG" : "WAV";
		const char *defaulted = ( sample->defaultSound ? "(DEFAULTED)" : sample->purged ? "(PURGED)" : "" );

		common->Printf( "%s %dkHz %6dms %5dkB %4s %s%s\n", stereo, sample->objectInfo.nSamplesPerSec / 1000,
					soundSystemLocal.SamplesToMilliseconds( sample->LengthIn44kHzSamples() ),
					sample->objectMemSize >> 10, format, sample->name.c_str(), defaulted );

		if ( !sample->purged ) {
			totalSamples += sample->objectSize;
			if ( info.wFormatTag != WAVE_FORMAT_TAG_OGG )
				totalPCMMemory += sample->objectMemSize;
			if ( !sample->hardwareBuffer )
				totalMemory += sample->objectMemSize;
		}
		totalSounds++;
	}
	common->Printf( "%8d total sounds\n", totalSounds );
	common->Printf( "%8d total samples loaded\n", totalSamples );
	common->Printf( "%8d kB total system memory used\n", totalMemory >> 10 );
#if ID_OPENAL
	common->Printf( "%8d kB total OpenAL audio memory used\n", ( alGetInteger( alGetEnumValue( "AL_EAX_RAM_SIZE" ) ) - alGetInteger( alGetEnumValue( "AL_EAX_RAM_FREE" ) ) ) >> 10 );
#endif
}

/*
===============
ListSoundDecoders_f
===============
*/
void ListSoundDecoders_f( const idCmdArgs &args ) {
	int i, j, numActiveDecoders, numWaitingDecoders;
	idSoundWorldLocal *sw = soundSystemLocal.currentSoundWorld;

	numActiveDecoders = numWaitingDecoders = 0;

	for ( i = 0; i < sw->emitters.Num(); i++ ) {
		idSoundEmitterLocal *sound = sw->emitters[i];

		if ( !sound ) {
			continue;
		}

		// run through all the channels
		for ( j = 0; j < SOUND_MAX_CHANNELS; j++ ) {
			idSoundChannel	*chan = &sound->channels[j];

			if ( chan->decoder == NULL ) {
				continue;
			}

			idSoundSample *sample = chan->decoder->GetSample();

			if ( sample != NULL ) {
				continue;
			}

			const char *format = ( chan->leadinSample->objectInfo.wFormatTag == WAVE_FORMAT_TAG_OGG ) ? "OGG" : "WAV";
			common->Printf( "%3d waiting %s: %s\n", numWaitingDecoders, format, chan->leadinSample->name.c_str() );

			numWaitingDecoders++;
		}
	}

	for ( i = 0; i < sw->emitters.Num(); i++ ) {
		idSoundEmitterLocal *sound = sw->emitters[i];

		if ( !sound ) {
			continue;
		}

		// run through all the channels
		for ( j = 0; j < SOUND_MAX_CHANNELS; j++ ) {
			idSoundChannel	*chan = &sound->channels[j];

			if ( chan->decoder == NULL ) {
				continue;
			}

			idSoundSample *sample = chan->decoder->GetSample();

			if ( sample == NULL ) {
				continue;
			}

			const char *format = ( sample->objectInfo.wFormatTag == WAVE_FORMAT_TAG_OGG ) ? "OGG" : "WAV";

			int localTime = soundSystemLocal.GetCurrent44kHzTime() - chan->trigger44kHzTime;
			int sampleTime = sample->LengthIn44kHzSamples() * sample->objectInfo.nChannels;
			int percent;
			if ( localTime > sampleTime ) {
				if ( chan->parms.soundShaderFlags & SSF_LOOPING ) {
					percent = ( localTime % sampleTime ) * 100 / sampleTime;
				} else {
					percent = 100;
				}
			} else {
				percent = localTime * 100 / sampleTime;
			}

			common->Printf( "%3d decoding %3d%% %s: %s\n", numActiveDecoders, percent, format, sample->name.c_str() );

			numActiveDecoders++;
		}
	}

	common->Printf( "%d decoders\n", numWaitingDecoders + numActiveDecoders );
	common->Printf( "%d waiting decoders\n", numWaitingDecoders );
	common->Printf( "%d active decoders\n", numActiveDecoders );
	common->Printf( "%d kB decoder memory in %d blocks\n", idSampleDecoder::GetUsedBlockMemory() >> 10, idSampleDecoder::GetNumUsedBlocks() );
}

/*
===============
TestSound_f

  this is called from the main thread
===============
*/
void TestSound_f( const idCmdArgs &args ) {
	if ( args.Argc() != 2 ) {
		common->Printf( "Usage: testSound <file>\n" );
		return;
	}
	if ( soundSystemLocal.currentSoundWorld ) {
		soundSystemLocal.currentSoundWorld->PlayShaderDirectly( args.Argv( 1 ) );
	}
}

/*
===============
SoundSystemRestart_f

restart the sound thread

  this is called from the main thread
===============
*/
void SoundSystemRestart_f( const idCmdArgs &args ) {
	soundSystem->SetMute( true );
	soundSystemLocal.ShutdownHW();
	soundSystemLocal.InitHW();
	soundSystem->SetMute( false );
}

/*
===============
idSoundSystemLocal::Init

initialize the sound system
===============
*/
void idSoundSystemLocal::Init() {

	common->Printf( "----- Initializing Sound System ------\n" );

	isInitialized = false;
	muted = false;
	shutdown = false;

	currentSoundWorld = NULL;
	soundCache = NULL;

	olddwCurrentWritePos = 0;
	buffers = 0;
	CurrentSoundTime = 0;

	nextWriteBlock = 0xffffffff;

	memset( meterTops, 0, sizeof( meterTops ) );
	memset( meterTopsTime, 0, sizeof( meterTopsTime ) );

	for( int i = -600; i < 600; i++ ) {
		float pt = i * 0.1f;
		volumesDB[i+600] = pow( 2.0f,( pt * ( 1.0f / 6.0f ) ) );
	}

	// make a 16 byte aligned finalMixBuffer
	finalMixBuffer = (float *) ( ( ( (int)realAccum ) + 15 ) & ~15 );

	graph = NULL;

	if ( !s_noSound.GetBool() ) {
		idSampleDecoder::Init();
		soundCache = new idSoundCache();
	}

	// set up openal device and context
	common->StartupVariable( "s_useOpenAL", true );
	common->StartupVariable( "s_useEAXReverb", true );

	if ( idSoundSystemLocal::s_useOpenAL.GetBool() || idSoundSystemLocal::s_useEAXReverb.GetBool() ) {
		if ( !Sys_LoadOpenAL() ) {
			idSoundSystemLocal::s_useOpenAL.SetBool( false );
		} else {
			common->Printf( "Setup OpenAL device and context... " );
			openalDevice = alcOpenDevice( NULL );
			openalContext = alcCreateContext( openalDevice, NULL );
			alcMakeContextCurrent( openalContext );
			common->Printf( "Done.\n" );

			// try to obtain EAX extensions
			if ( idSoundSystemLocal::s_useEAXReverb.GetBool() && alIsExtensionPresent( ID_ALCHAR "EAX4.0" ) ) {
				idSoundSystemLocal::s_useOpenAL.SetBool( true );	// EAX presence causes AL enable
				alEAXSet = (EAXSet)alGetProcAddress( ID_ALCHAR "EAXSet" );
				alEAXGet = (EAXGet)alGetProcAddress( ID_ALCHAR "EAXGet" );
				common->Printf( "OpenAL: found EAX 4.0 extension\n" );
			} else {
				common->Printf( "OpenAL: EAX 4.0 extension not found\n" );
				idSoundSystemLocal::s_useEAXReverb.SetBool( false );
				alEAXSet = (EAXSet)NULL;
				alEAXGet = (EAXGet)NULL;
			}

			// try to obtain EAX-RAM extension - not required for operation
			if ( alIsExtensionPresent( ID_ALCHAR "EAX-RAM" ) == AL_TRUE ) {
				alEAXSetBufferMode = (EAXSetBufferMode)alGetProcAddress( ID_ALCHAR "EAXSetBufferMode" );
				alEAXGetBufferMode = (EAXGetBufferMode)alGetProcAddress( ID_ALCHAR "EAXGetBufferMode" );
				common->Printf( "OpenAL: found EAX-RAM extension, %dkB\\%dkB\n", alGetInteger( alGetEnumValue( ID_ALCHAR "AL_EAX_RAM_FREE" ) ) / 1024, alGetInteger( alGetEnumValue( ID_ALCHAR "AL_EAX_RAM_SIZE" ) ) / 1024 );
			} else {
				alEAXSetBufferMode = (EAXSetBufferMode)NULL;
				alEAXGetBufferMode = (EAXGetBufferMode)NULL;
				common->Printf( "OpenAL: no EAX-RAM extension\n" );
			}

			if ( !idSoundSystemLocal::s_useOpenAL.GetBool() ) {
				common->Printf( "OpenAL: disabling ( no EAX ). Using legacy mixer.\n" );

				alcMakeContextCurrent( NULL );
		
				alcDestroyContext( openalContext );
				openalContext = NULL;
		
				alcCloseDevice( openalDevice );
				openalDevice = NULL;
			} else {

				ALuint handle;		
				openalSourceCount = 0;
				
				while ( openalSourceCount < 256 ) {
					alGetError();
					alGenSources( 1, &handle );
					if ( alGetError() != AL_NO_ERROR ) {
						break;
					} else {
						// store in source array
						openalSources[openalSourceCount].handle = handle;
						openalSources[openalSourceCount].startTime = 0;
						openalSources[openalSourceCount].chan = NULL;
						openalSources[openalSourceCount].inUse = false;
						openalSources[openalSourceCount].looping = false;

						// initialise sources
						alSourcef( handle, AL_ROLLOFF_FACTOR, 0.0f );

						// found one source
						openalSourceCount++;
					}
				}

				common->Printf( "OpenAL: found %s\n", alcGetString( openalDevice, ALC_DEVICE_SPECIFIER ) );
				common->Printf( "OpenAL: found %d hardware voices\n", openalSourceCount );

				// adjust source count to allow for at least eight stereo sounds to play
				openalSourceCount -= 8;

				EAXAvailable = 1;
			}
		}
	}

	useOpenAL = idSoundSystemLocal::s_useOpenAL.GetBool();
	useEAXReverb = idSoundSystemLocal::s_useEAXReverb.GetBool();

	cmdSystem->AddCommand( "listSounds", ListSounds_f, CMD_FL_SOUND, "lists all sounds" );
	cmdSystem->AddCommand( "listSoundDecoders", ListSoundDecoders_f, CMD_FL_SOUND, "list active sound decoders" );
	cmdSystem->AddCommand( "reloadSounds", SoundReloadSounds_f, CMD_FL_SOUND|CMD_FL_CHEAT, "reloads all sounds" );
	cmdSystem->AddCommand( "testSound", TestSound_f, CMD_FL_SOUND | CMD_FL_CHEAT, "tests a sound", idCmdSystem::ArgCompletion_SoundName );
	cmdSystem->AddCommand( "s_restart", SoundSystemRestart_f, CMD_FL_SOUND, "restarts the sound system" );

	common->Printf( "sound system initialized.\n" );
	common->Printf( "--------------------------------------\n" );
}

/*
===============
idSoundSystemLocal::Shutdown
===============
*/
void idSoundSystemLocal::Shutdown() {
	ShutdownHW();

	// EAX or not, the list needs to be cleared
	EFXDatabase.Clear();

	// destroy openal sources
	if ( useOpenAL ) {
		
		efxloaded = false;

		// adjust source count back up to allow for freeing of all resources
		openalSourceCount += 8;

		for ( ALsizei i = 0; i < openalSourceCount; i++ ) {
			// stop source
			alSourceStop( openalSources[i].handle );
			alSourcei( openalSources[i].handle, AL_BUFFER, 0 );
			
			// delete source
			alDeleteSources( 1, &openalSources[i].handle );

			// clear entry in source array
			openalSources[i].handle = NULL;
			openalSources[i].startTime = 0;
			openalSources[i].chan = NULL;
			openalSources[i].inUse = false;
			openalSources[i].looping = false;

		}
	}

	// destroy all the sounds (hardware buffers as well)
	delete soundCache;
	soundCache = NULL;

	// destroy openal device and context
	if ( useOpenAL ) {
		alcMakeContextCurrent( NULL );
		
		alcDestroyContext( openalContext );
		openalContext = NULL;
		
		alcCloseDevice( openalDevice );
		openalDevice = NULL;
	}

	Sys_FreeOpenAL();

	idSampleDecoder::Shutdown();
}

/*
===============
idSoundSystemLocal::InitHW
===============
*/
bool idSoundSystemLocal::InitHW() {

	if ( s_noSound.GetBool() ) {
		return false;
	}

	delete snd_audio_hw;
	snd_audio_hw = idAudioHardware::Alloc();

	if ( snd_audio_hw == NULL ) {
		return false;
	}

	if ( !useOpenAL ) {
		if ( !snd_audio_hw->Initialize() ) {
			delete snd_audio_hw;
			snd_audio_hw = NULL;
			return false;
		}

		if ( snd_audio_hw->GetNumberOfSpeakers() == 0 ) {
			return false;
		}
		// put the real number in there
		s_numberOfSpeakers.SetInteger( snd_audio_hw->GetNumberOfSpeakers() );
	}

	isInitialized = true;
	shutdown = false;

	return true;
}

/*
===============
idSoundSystemLocal::ShutdownHW
===============
*/
bool idSoundSystemLocal::ShutdownHW() {
	if ( !isInitialized ) {
		return false;
	}

	shutdown = true;		// don't do anything at AsyncUpdate() time
	Sys_Sleep( 100 );		// sleep long enough to make sure any async sound talking to hardware has returned

	common->Printf( "Shutting down sound hardware\n" );

	delete snd_audio_hw;
	snd_audio_hw = NULL;

	isInitialized = false;

	if ( graph ) {
		Mem_Free( graph );
		graph = NULL;
	}

	return true;
}

/*
===============
idSoundSystemLocal::GetCurrent44kHzTime
===============
*/
int idSoundSystemLocal::GetCurrent44kHzTime( void ) const {
	if ( snd_audio_hw ) {
		return CurrentSoundTime;
	} else {
		// NOTE: this would overflow 31bits within about 1h20 ( not that important since we get a snd_audio_hw right away pbly )
		//return ( ( Sys_Milliseconds()*441 ) / 10 ) * 4; 
		return idMath::FtoiFast( (float)Sys_Milliseconds() * 176.4f );
	}
}

/*
===================
idSoundSystemLocal::ClearBuffer
===================
*/
void idSoundSystemLocal::ClearBuffer( void ) {

	// check to make sure hardware actually exists
	if ( !snd_audio_hw ) {
		return;
	}

	short *fBlock;
	ulong fBlockLen;

	if ( !snd_audio_hw->Lock( (void **)&fBlock, &fBlockLen ) ) {
		return;
	}

	if ( fBlock ) {
		SIMDProcessor->Memset( fBlock, 0, fBlockLen );
		snd_audio_hw->Unlock( fBlock, fBlockLen );
	}
}

/*
===================
idSoundSystemLocal::AsyncMix
Mac OSX version. The system uses it's own thread and an IOProc callback
===================
*/
int idSoundSystemLocal::AsyncMix( int soundTime, float *mixBuffer ) {
	int	inTime, numSpeakers;

	if ( !isInitialized || shutdown || !snd_audio_hw ) {
		return 0;
	}

	inTime = Sys_Milliseconds();
	numSpeakers = snd_audio_hw->GetNumberOfSpeakers();
	
	// let the active sound world mix all the channels in unless muted or avi demo recording
	if ( !muted && currentSoundWorld && !currentSoundWorld->fpa[0] ) {
		currentSoundWorld->MixLoop( soundTime, numSpeakers, mixBuffer );
	}

	CurrentSoundTime = soundTime;
	
	return Sys_Milliseconds() - inTime;
}

/*
===================
idSoundSystemLocal::AsyncUpdate
called from async sound thread when com_asyncSound == 1 ( Windows )
===================
*/
int idSoundSystemLocal::AsyncUpdate( int inTime ) {

	if ( !isInitialized || shutdown || !snd_audio_hw ) {
		return 0;
	}

	ulong dwCurrentWritePos;
	dword dwCurrentBlock;

	// If not using openal, get actual playback position from sound hardware
	if ( useOpenAL ) {
		// here we do it in samples ( overflows in 27 hours or so )
		dwCurrentWritePos = idMath::Ftol( (float)Sys_Milliseconds() * 44.1f ) % ( MIXBUFFER_SAMPLES * ROOM_SLICES_IN_BUFFER );
		dwCurrentBlock = dwCurrentWritePos / MIXBUFFER_SAMPLES;
	} else {
		// and here in bytes
		// get the current byte position in the buffer where the sound hardware is currently reading
		if ( !snd_audio_hw->GetCurrentPosition( &dwCurrentWritePos ) ) {
			return 0;
		}
		// mixBufferSize is in bytes
		dwCurrentBlock = dwCurrentWritePos / snd_audio_hw->GetMixBufferSize();
	}

	if ( nextWriteBlock == 0xffffffff ) {
		nextWriteBlock = dwCurrentBlock;
	}

	if ( dwCurrentBlock != nextWriteBlock ) {
		return 0;
	}

	// lock the buffer so we can actually write to it
	short *fBlock = NULL;
	ulong fBlockLen = 0;
	if ( !useOpenAL ) {
		snd_audio_hw->Lock( (void **)&fBlock, &fBlockLen );
		if ( !fBlock ) {
			return 0;
		}
	}

	int j;
	soundStats.runs++;
	soundStats.activeSounds = 0;

	int	numSpeakers = snd_audio_hw->GetNumberOfSpeakers();

	nextWriteBlock++;
	nextWriteBlock %= ROOM_SLICES_IN_BUFFER;

	int newPosition = nextWriteBlock * MIXBUFFER_SAMPLES;

	if ( newPosition < olddwCurrentWritePos ) {
		buffers++;					// buffer wrapped
	}

	// nextWriteSample is in multi-channel samples inside the buffer
	int	nextWriteSamples = nextWriteBlock * MIXBUFFER_SAMPLES;

	olddwCurrentWritePos = newPosition;

	// newSoundTime is in multi-channel samples since the sound system was started
	int newSoundTime = ( buffers * MIXBUFFER_SAMPLES * ROOM_SLICES_IN_BUFFER ) + nextWriteSamples;
	
	// check for impending overflow
	// FIXME: we don't handle sound wrap-around correctly yet
	if ( newSoundTime > 0x6fffffff ) {
		buffers = 0;
	}

	if ( (newSoundTime - CurrentSoundTime) > (int)MIXBUFFER_SAMPLES ) {
		soundStats.missedWindow++;
	}

	if ( useOpenAL ) {
		// enable audio hardware caching
		alcSuspendContext( openalContext );
	} else {
		// clear the buffer for all the mixing output
		SIMDProcessor->Memset( finalMixBuffer, 0, MIXBUFFER_SAMPLES * sizeof(float) * numSpeakers );
	}

	// let the active sound world mix all the channels in unless muted or avi demo recording
	if ( !muted && currentSoundWorld && !currentSoundWorld->fpa[0] ) {
		currentSoundWorld->MixLoop( newSoundTime, numSpeakers, finalMixBuffer );
	}

	if ( useOpenAL ) {
		// disable audio hardware caching (this updates ALL settings since last alcSuspendContext)
		alcProcessContext( openalContext );
	} else {
		short *dest = fBlock + nextWriteSamples * numSpeakers;

		SIMDProcessor->MixedSoundToSamples( dest, finalMixBuffer, MIXBUFFER_SAMPLES * numSpeakers );

		// allow swapping the left / right speaker channels for people with miswired systems
		if ( numSpeakers == 2 && s_reverse.GetBool() ) {
			for( j = 0; j < MIXBUFFER_SAMPLES; j++ ) {
				short temp = dest[j*2];
				dest[j*2] = dest[j*2+1];
				dest[j*2+1] = temp;
			}
		}
		snd_audio_hw->Unlock( fBlock, fBlockLen );
	}

	CurrentSoundTime = newSoundTime;

	soundStats.timeinprocess = Sys_Milliseconds() - inTime;

	return soundStats.timeinprocess;
}

/*
===================
idSoundSystemLocal::AsyncUpdateWrite
sound output using a write API. all the scheduling based on time
we mix MIXBUFFER_SAMPLES at a time, but we feed the audio device with smaller chunks (and more often)
called by the sound thread when com_asyncSound is 3 ( Linux )
===================
*/
int idSoundSystemLocal::AsyncUpdateWrite( int inTime ) {

	if ( !isInitialized || shutdown || !snd_audio_hw ) {
		return 0;
	}

	if ( !useOpenAL ) {
		snd_audio_hw->Flush();
	}

	unsigned int dwCurrentBlock = (unsigned int)( inTime * 44.1f / MIXBUFFER_SAMPLES );

	if ( nextWriteBlock == 0xffffffff ) {
		nextWriteBlock = dwCurrentBlock;
	}

	if ( dwCurrentBlock < nextWriteBlock ) {
		return 0;
	}

	if ( nextWriteBlock != dwCurrentBlock ) {
		Sys_Printf( "missed %d sound updates\n", dwCurrentBlock - nextWriteBlock );
	}

	int sampleTime = dwCurrentBlock * MIXBUFFER_SAMPLES;	
	int numSpeakers = snd_audio_hw->GetNumberOfSpeakers();

	if ( useOpenAL ) {
		// enable audio hardware caching
		alcSuspendContext( openalContext );
	} else {
		// clear the buffer for all the mixing output
		SIMDProcessor->Memset( finalMixBuffer, 0, MIXBUFFER_SAMPLES * sizeof(float) * numSpeakers );
	}

	// let the active sound world mix all the channels in unless muted or avi demo recording
	if ( !muted && currentSoundWorld && !currentSoundWorld->fpa[0] ) {
		currentSoundWorld->MixLoop( sampleTime, numSpeakers, finalMixBuffer );
	}

	if ( useOpenAL ) {
		// disable audio hardware caching (this updates ALL settings since last alcSuspendContext)
		alcProcessContext( openalContext );
	} else {
		short *dest = snd_audio_hw->GetMixBuffer();

		SIMDProcessor->MixedSoundToSamples( dest, finalMixBuffer, MIXBUFFER_SAMPLES * numSpeakers );

		// allow swapping the left / right speaker channels for people with miswired systems
		if ( numSpeakers == 2 && s_reverse.GetBool() ) {
			int j;
			for( j = 0; j < MIXBUFFER_SAMPLES; j++ ) {
				short temp = dest[j*2];
				dest[j*2] = dest[j*2+1];
				dest[j*2+1] = temp;
			}
		}
		snd_audio_hw->Write( false );
	}

	// only move to the next block if the write was successful
	nextWriteBlock = dwCurrentBlock + 1;
	CurrentSoundTime = sampleTime;

	return Sys_Milliseconds() - inTime;
}

/*
===================
idSoundSystemLocal::dB2Scale
===================
*/
float idSoundSystemLocal::dB2Scale( const float val ) const {
	if ( val == 0.0f ) {
		return 1.0f;				// most common
	} else if ( val <= -60.0f ) {
		return 0.0f;
	} else if ( val >= 60.0f ) {
		return powf( 2.0f, val * ( 1.0f / 6.0f ) ); 
	}
	int ival = (int)( ( val + 60.0f ) * 10.0f );
	return volumesDB[ival];
}

/*
===================
idSoundSystemLocal::ImageForTime
===================
*/
cinData_t idSoundSystemLocal::ImageForTime( const int milliseconds, const bool waveform ) {
	cinData_t ret;
	int i, j;

	if ( !isInitialized || !snd_audio_hw ) {
		memset( &ret, 0, sizeof( ret ) );
		return ret;
	}

	Sys_EnterCriticalSection();

	if ( !graph ) {
		graph = (dword *)Mem_Alloc( 256*128 * 4);
	}
	memset( graph, 0, 256*128 * 4 );
	float *accum = finalMixBuffer;	// unfortunately, these are already clamped
	int time = Sys_Milliseconds();

	int numSpeakers = snd_audio_hw->GetNumberOfSpeakers();

	if ( !waveform ) {
		for( j = 0; j < numSpeakers; j++ ) {
			int meter = 0;
			for( i = 0; i < MIXBUFFER_SAMPLES; i++ ) {
				float result = idMath::Fabs(accum[i*numSpeakers+j]);
				if ( result > meter ) {
					meter = result;
				}
			}

			meter /= 256;		// 32768 becomes 128
			if ( meter > 128 ) {
				meter = 128;
			}
			int offset;
			int xsize;
			if ( numSpeakers == 6 ) {
				offset = j * 40;
				xsize = 20;
			} else {
				offset = j * 128;
				xsize = 63;
			}
			int x,y;
			dword color = 0xff00ff00;
			for ( y = 0; y < 128; y++ ) {
				for ( x = 0; x < xsize; x++ ) {
					graph[(127-y)*256 + offset + x ] = color;
				}
#if 0
				if ( y == 80 ) {
					color = 0xff00ffff;
				} else if ( y == 112 ) {
					color = 0xff0000ff;
				}
#endif
				if ( y > meter ) {
					break;
				}
			}

			if ( meter > meterTops[j] ) {
				meterTops[j] = meter;
				meterTopsTime[j] = time + s_meterTopTime.GetInteger();
			} else if ( time > meterTopsTime[j] && meterTops[j] > 0 ) {
				meterTops[j]--;
				if (meterTops[j]) {
					meterTops[j]--;
				}
			}
		}

		for( j = 0; j < numSpeakers; j++ ) {
			int meter = meterTops[j];

			int offset;
			int xsize;
			if ( numSpeakers == 6 ) {
				offset = j*40;
				xsize = 20;
			} else {
				offset = j*128;
				xsize = 63;
			}
			int x,y;
			dword color;
			if ( meter <= 80 ) {
				color = 0xff007f00;
			} else if ( meter <= 112 ) {
				color = 0xff007f7f;
			} else {
				color = 0xff00007f;
			}
			for ( y = meter; y < 128 && y < meter + 4; y++ ) {
				for ( x = 0; x < xsize; x++ ) {
					graph[(127-y)*256 + offset + x ] = color;
				}
			}
		}
	} else {
		dword colors[] = { 0xff007f00, 0xff007f7f, 0xff00007f, 0xff00ff00, 0xff00ffff, 0xff0000ff };

		for( j = 0; j < numSpeakers; j++ ) {
			int xx = 0;
			float fmeter;
			int step = MIXBUFFER_SAMPLES / 256;
			for( i = 0; i < MIXBUFFER_SAMPLES; i += step ) {
				fmeter = 0.0f;
				for( int x = 0; x < step; x++ ) {
					float result = accum[(i+x)*numSpeakers+j];
					result = result / 32768.0f;
					fmeter += result;
				}
				fmeter /= 4.0f;
				if ( fmeter < -1.0f ) {
					fmeter = -1.0f;
				} else if ( fmeter > 1.0f ) {
					fmeter = 1.0f;
				}
				int meter = (fmeter * 63.0f);
				graph[ (meter + 64) * 256 + xx ] = colors[j];

				if ( meter < 0 ) {
					meter = -meter;
				}
				if ( meter > meterTops[xx] ) {
					meterTops[xx] = meter;
					meterTopsTime[xx] = time + 100;
				} else if ( time>meterTopsTime[xx] && meterTops[xx] > 0 ) {
					meterTops[xx]--;
					if ( meterTops[xx] ) {
						meterTops[xx]--;
					}
				}
				xx++;
			}
		}
		for( i = 0; i < 256; i++ ) {
			int meter = meterTops[i];
			for ( int y = -meter; y < meter; y++ ) {
				graph[ (y+64)*256 + i ] = colors[j];
			}
		}
	}
	ret.imageHeight = 128;
	ret.imageWidth = 256;
	ret.image = (unsigned char *)graph;

	Sys_LeaveCriticalSection();

	return ret;
}

/*
===================
idSoundSystemLocal::GetSoundDecoderInfo
===================
*/
int idSoundSystemLocal::GetSoundDecoderInfo( int index, soundDecoderInfo_t &decoderInfo ) {
	int i, j, firstEmitter, firstChannel;
	idSoundWorldLocal *sw = soundSystemLocal.currentSoundWorld;

	if ( index < 0 ) {
		firstEmitter = 0;
		firstChannel = 0;
	} else {
		firstEmitter = index / SOUND_MAX_CHANNELS;
		firstChannel = index - firstEmitter * SOUND_MAX_CHANNELS + 1;
	}

	for ( i = firstEmitter; i < sw->emitters.Num(); i++ ) {
		idSoundEmitterLocal *sound = sw->emitters[i];

		if ( !sound ) {
			continue;
		}

		// run through all the channels
		for ( j = firstChannel; j < SOUND_MAX_CHANNELS; j++ ) {
			idSoundChannel	*chan = &sound->channels[j];

			if ( chan->decoder == NULL ) {
				continue;
			}

			idSoundSample *sample = chan->decoder->GetSample();

			if ( sample == NULL ) {
				continue;
			}

			decoderInfo.name = sample->name;
			decoderInfo.format = ( sample->objectInfo.wFormatTag == WAVE_FORMAT_TAG_OGG ) ? "OGG" : "WAV";
			decoderInfo.numChannels = sample->objectInfo.nChannels;
			decoderInfo.numSamplesPerSecond = sample->objectInfo.nSamplesPerSec;
			decoderInfo.num44kHzSamples = sample->LengthIn44kHzSamples();
			decoderInfo.numBytes = sample->objectMemSize;
			decoderInfo.looping = ( chan->parms.soundShaderFlags & SSF_LOOPING ) != 0;
			decoderInfo.lastVolume = chan->lastVolume;
			decoderInfo.start44kHzTime = chan->trigger44kHzTime;
			decoderInfo.current44kHzTime = soundSystemLocal.GetCurrent44kHzTime();

			return ( i * SOUND_MAX_CHANNELS + j );
		}

		firstChannel = 0;
	}
	return -1;
}

/*
===================
idSoundSystemLocal::AllocSoundWorld
===================
*/
idSoundWorld *idSoundSystemLocal::AllocSoundWorld( idRenderWorld *rw ) {
	idSoundWorldLocal	*local = new idSoundWorldLocal;

	local->Init( rw );

	return local;
}

/*
===================
idSoundSystemLocal::SetMute
===================
*/
void idSoundSystemLocal::SetMute( bool muteOn ) {
	muted = muteOn;
}

/*
===================
idSoundSystemLocal::SamplesToMilliseconds
===================
*/
int idSoundSystemLocal::SamplesToMilliseconds( int samples ) const {
	return ( samples / (PRIMARYFREQ/1000) );
}

/*
===================
idSoundSystemLocal::SamplesToMilliseconds
===================
*/
int idSoundSystemLocal::MillisecondsToSamples( int ms ) const {
	return ( ms * (PRIMARYFREQ/1000) );
}

/*
===================
idSoundSystemLocal::SetPlayingSoundWorld

specifying NULL will cause silence to be played
===================
*/
void idSoundSystemLocal::SetPlayingSoundWorld( idSoundWorld *soundWorld ) {
	currentSoundWorld = static_cast<idSoundWorldLocal *>(soundWorld);
}

/*
===================
idSoundSystemLocal::GetPlayingSoundWorld
===================
*/
idSoundWorld *idSoundSystemLocal::GetPlayingSoundWorld( void ) {
	return currentSoundWorld;
}

/*
===================
idSoundSystemLocal::BeginLevelLoad
===================
*/

void idSoundSystemLocal::BeginLevelLoad() {
	if ( !isInitialized ) {
		return;
	}
	soundCache->BeginLevelLoad();
	
	if ( efxloaded ) {
		EFXDatabase.UnloadFile();
		efxloaded = false;
	}
}

/*
===================
idSoundSystemLocal::EndLevelLoad
===================
*/
void idSoundSystemLocal::EndLevelLoad( const char *mapstring ) {
	if ( !isInitialized ) {
		return;
	}
	soundCache->EndLevelLoad();

	idStr efxname( "efxs/" );
	idStr mapname( mapstring );

	mapname.SetFileExtension( ".efx" );
	mapname.StripPath();
	efxname += mapname;

	efxloaded = EFXDatabase.LoadFile( efxname );

	if ( efxloaded ) {
		common->Printf("sound: found %s\n", efxname.c_str() );
	} else {
		common->Printf("sound: missing %s\n", efxname.c_str() );
	}
}

/*
===================
idSoundSystemLocal::AllocOpenALSource
===================
*/
ALuint idSoundSystemLocal::AllocOpenALSource( idSoundChannel *chan, bool looping, bool stereo ) {
	int timeOldestZeroVolSingleShot = Sys_Milliseconds();
	int timeOldestZeroVolLooping = Sys_Milliseconds();
	int timeOldestSingle = Sys_Milliseconds();
	int iOldestZeroVolSingleShot = -1;
	int iOldestZeroVolLooping = -1;
	int iOldestSingle = -1;
	int iUnused = -1;
	int index = -1;
	ALsizei i;

	// Grab current msec time
	int time = Sys_Milliseconds();

	// Cycle through all sources
	for ( i = 0; i < openalSourceCount; i++ ) {
		// Use any unused source first,
		// Then find oldest single shot quiet source,
		// Then find oldest looping quiet source and
		// Lastly find oldest single shot non quiet source..
		if ( !openalSources[i].inUse ) {
			iUnused = i;
			break;
		}  else if ( !openalSources[i].looping && openalSources[i].chan->lastVolume < SND_EPSILON ) {
			if ( openalSources[i].startTime < timeOldestZeroVolSingleShot ) {
				timeOldestZeroVolSingleShot = openalSources[i].startTime;
				iOldestZeroVolSingleShot = i;
			}
		} else if ( openalSources[i].looping && openalSources[i].chan->lastVolume < SND_EPSILON ) {
			if ( openalSources[i].startTime < timeOldestZeroVolLooping ) {
				timeOldestZeroVolLooping = openalSources[i].startTime;
				iOldestZeroVolLooping = i;
			}
		} else if ( !openalSources[i].looping ) {
			if ( openalSources[i].startTime < timeOldestSingle ) {
				timeOldestSingle = openalSources[i].startTime;
				iOldestSingle = i;
			}
		}
	}

	if ( iUnused != -1 ) {
		index = iUnused;
	} else if ( iOldestZeroVolSingleShot != - 1 ) {
		index = iOldestZeroVolSingleShot;
	} else if ( iOldestZeroVolLooping != -1 ) {
		index = iOldestZeroVolLooping;
	} else if ( iOldestSingle != -1 ) {
		index = iOldestSingle;
	}

	if ( index != -1 ) {
		// stop the channel that is being ripped off
		if ( openalSources[index].chan ) {
			// stop the channel only when not looping
			if ( !openalSources[index].looping ) {
				openalSources[index].chan->Stop();
			} else {
				openalSources[index].chan->triggered = true;
			}

			// Free hardware resources
			openalSources[index].chan->ALStop();
		}

		// Initialize structure
		openalSources[index].startTime = time;
		openalSources[index].chan = chan;
		openalSources[index].inUse = true;
		openalSources[index].looping = looping;
		openalSources[index].stereo = stereo;

		return openalSources[index].handle;
	} else {
		return NULL;
	}
}

/*
===================
idSoundSystemLocal::FreeOpenALSource
===================
*/
void idSoundSystemLocal::FreeOpenALSource( ALuint handle ) {
	ALsizei i;
	for ( i = 0; i < openalSourceCount; i++ ) {
		if ( openalSources[i].handle == handle ) {
			if ( openalSources[i].chan ) {
				openalSources[i].chan->openalSource = NULL;
			}
#if ID_OPENAL
			// Reset source EAX ROOM level when freeing stereo source
			if ( openalSources[i].stereo && alEAXSet ) {
				long Room = EAXSOURCE_DEFAULTROOM;
				alEAXSet( &EAXPROPERTYID_EAX_Source, EAXSOURCE_ROOM, openalSources[i].handle, &Room, sizeof(Room));
			}
#endif
			// Initialize structure
			openalSources[i].startTime = 0;
			openalSources[i].chan = NULL;
			openalSources[i].inUse = false;
			openalSources[i].looping = false;
			openalSources[i].stereo = false;
		}
	}
}

/*
============================================================
SoundFX and misc effects
============================================================
*/

/*
===================
idSoundSystemLocal::ProcessSample
===================
*/
void SoundFX_Lowpass::ProcessSample( float* in, float* out ) {
	float c, a1, a2, a3, b1, b2;
	float resonance = idSoundSystemLocal::s_enviroSuitCutoffQ.GetFloat();
	float cutoffFrequency = idSoundSystemLocal::s_enviroSuitCutoffFreq.GetFloat();

	Initialize();

	c = 1.0 / idMath::Tan16( idMath::PI * cutoffFrequency / 44100 );

	// compute coefs
	a1 = 1.0 / ( 1.0 + resonance * c + c * c );
	a2 = 2* a1;
	a3 = a1;
	b1 = 2.0 * ( 1.0 - c * c) * a1;
	b2 = ( 1.0 - resonance * c + c * c ) * a1;

	// compute output value
	out[0] = a1 * in[0] + a2 * in[-1] + a3 * in[-2] - b1 * out[-1] - b2 * out[-2];
}

void SoundFX_LowpassFast::ProcessSample( float* in, float* out ) {
	// compute output value
	out[0] = a1 * in[0] + a2 * in[-1] + a3 * in[-2] - b1 * out[-1] - b2 * out[-2];
}

void SoundFX_LowpassFast::SetParms( float p1, float p2, float p3 ) {
	float c;

	// set the vars
	freq = p1;
	res = p2;

	// precompute the coefs
	c = 1.0 / idMath::Tan( idMath::PI * freq / 44100 );

	// compute coefs
	a1 = 1.0 / ( 1.0 + res * c + c * c );
	a2 = 2* a1;
	a3 = a1;

	b1 = 2.0 * ( 1.0 - c * c) * a1;
	b2 = ( 1.0 - res * c + c * c ) * a1;
}

void SoundFX_Comb::Initialize() {
	if ( initialized )
		return;

	initialized = true;
	maxlen = 50000;
	buffer = new float[maxlen];
	currentTime = 0;
}

void SoundFX_Comb::ProcessSample( float* in, float* out ) {
	float gain = idSoundSystemLocal::s_reverbFeedback.GetFloat();
	int len = idSoundSystemLocal::s_reverbTime.GetFloat() + param;

	Initialize();

	// sum up and output
	out[0] = buffer[currentTime];
	buffer[currentTime] = buffer[currentTime] * gain + in[0];

	// increment current time
	currentTime++;
	if ( currentTime >= len )
		currentTime -= len;
}

/*
===================
idSoundSystemLocal::DoEnviroSuit
===================
*/
void idSoundSystemLocal::DoEnviroSuit( float* samples, int numSamples, int numSpeakers ) {
	float out[10000], *out_p = out + 2;
	float in[10000], *in_p = in + 2;

	assert( !idSoundSystemLocal::useOpenAL );

	if ( !fxList.Num() ) {
		for ( int i = 0; i < 6; i++ ) {
			SoundFX* fx;

			// lowpass filter
			fx = new SoundFX_Lowpass();
			fx->SetChannel( i );
			fxList.Append( fx );

			// comb
			fx = new SoundFX_Comb();
			fx->SetChannel( i );
			fx->SetParameter( i * 100 );
			fxList.Append( fx );

			// comb
			fx = new SoundFX_Comb();
			fx->SetChannel( i );
			fx->SetParameter( i * 100 + 5 );
			fxList.Append( fx );
		}
	}

	for ( int i = 0; i < numSpeakers; i++ ) {
		int j;

		// restore previous samples
		memset( in, 0, 10000 * sizeof( float ) );
		memset( out, 0, 10000 * sizeof( float ) );

		// fx loop
		for ( int k = 0; k < fxList.Num(); k++ ) {
			SoundFX* fx = fxList[k];

			// skip if we're not the right channel
			if ( fx->GetChannel() != i )
				continue;

			// get samples and continuity
			fx->GetContinuitySamples( in_p[-1], in_p[-2], out_p[-1], out_p[-2] );
			for ( j = 0; j < numSamples; j++ ) {
				in_p[j] = samples[j * numSpeakers + i] * s_enviroSuitVolumeScale.GetFloat();
			}

			// process fx loop
			for ( j = 0; j < numSamples; j++ ) {
				fx->ProcessSample( in_p + j, out_p + j );
			}

			// store samples and continuity
			fx->SetContinuitySamples( in_p[numSamples-2], in_p[numSamples-3], out_p[numSamples-2], out_p[numSamples-3] );

			for ( j = 0; j < numSamples; j++ ) {
				samples[j * numSpeakers + i] = out_p[j];
			}
		}
	}
}

/*
=================
idSoundSystemLocal::PrintMemInfo
=================
*/
void idSoundSystemLocal::PrintMemInfo( MemInfo_t *mi ) {
	soundCache->PrintMemInfo( mi );
}

/*
===============
idSoundSystemLocal::EAXAvailable
===============
*/
int idSoundSystemLocal::IsEAXAvailable( void ) {
#if !ID_OPENAL
	return -1;
#else
	ALCdevice	*device;
	ALCcontext	*context;

	if ( EAXAvailable != -1 ) {
		return EAXAvailable;
	}

	if ( !Sys_LoadOpenAL() ) {
		EAXAvailable = 2;
		return 2;
	}
	// when dynamically loading the OpenAL subsystem, we need to get a context before alIsExtensionPresent would work
	device = alcOpenDevice( NULL );
	context = alcCreateContext( device, NULL );
	alcMakeContextCurrent( context );
	if ( alIsExtensionPresent( ID_ALCHAR "EAX4.0" ) ) {
		alcMakeContextCurrent( NULL );
		alcDestroyContext( context );
		alcCloseDevice( device );
		EAXAvailable = 1;
		return 1;
	}
	alcMakeContextCurrent( NULL );
	alcDestroyContext( context );
	alcCloseDevice( device );
	EAXAvailable = 0;
	return 0;
#endif
}

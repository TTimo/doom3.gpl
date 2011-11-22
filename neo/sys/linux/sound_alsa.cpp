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
#include "../../idlib/precompiled.h"
#include "../../sound/snd_local.h"
#include "../posix/posix_public.h"
#include "sound.h"

#include <dlfcn.h>

static idCVar s_alsa_pcm( "s_alsa_pcm", "default", CVAR_SYSTEM | CVAR_ARCHIVE, "which alsa pcm device to use. default, hwplug, hw.. see alsa docs" );
static idCVar s_alsa_lib( "s_alsa_lib", "libasound.so.2", CVAR_SYSTEM | CVAR_ARCHIVE, "alsa client sound library" );

/*
===============
idAudioHardwareALSA::DLOpen
===============
*/
bool idAudioHardwareALSA::DLOpen( void ) {
	const char *version;
	
	if ( m_handle ) {
		return true;
	}
	common->Printf( "dlopen(%s)\n", s_alsa_lib.GetString() );
	if ( !( m_handle = dlopen( s_alsa_lib.GetString(), RTLD_NOW | RTLD_GLOBAL ) ) ) {
		common->Printf( "dlopen(%s) failed: %s\n", s_alsa_lib.GetString(), dlerror() );
		return false;
	}
	// print the version if available
	id_snd_asoundlib_version = ( pfn_snd_asoundlib_version )dlsym( m_handle, "snd_asoundlib_version" );
	if ( !id_snd_asoundlib_version ) {
		common->Printf( "dlsym(\"snd_asoundlib_version\") failed: %s\n", dlerror() );
		common->Warning( "please consider upgrading alsa to a more recent version." );
	} else {
		version = id_snd_asoundlib_version();	
		common->Printf( "asoundlib version: %s\n", version );
	}
	// dlsym the symbols
	ALSA_DLSYM(snd_pcm_avail_update);
	ALSA_DLSYM(snd_pcm_close);
	ALSA_DLSYM(snd_pcm_hw_params);
	ALSA_DLSYM(snd_pcm_hw_params_any);
	ALSA_DLSYM(snd_pcm_hw_params_get_buffer_size);
	ALSA_DLSYM(snd_pcm_hw_params_set_access);
	ALSA_DLSYM(snd_pcm_hw_params_set_buffer_size_min);
	ALSA_DLSYM(snd_pcm_hw_params_set_channels);
	ALSA_DLSYM(snd_pcm_hw_params_set_format);
	ALSA_DLSYM(snd_pcm_hw_params_set_rate);
	ALSA_DLSYM(snd_pcm_hw_params_sizeof);
	ALSA_DLSYM(snd_pcm_open);
	ALSA_DLSYM(snd_pcm_prepare);
	ALSA_DLSYM(snd_pcm_state);
	ALSA_DLSYM(snd_pcm_writei);
	ALSA_DLSYM(snd_strerror);
	return true;
}

/*
===============
idAudioHardwareALSA::Release
===============
*/
void idAudioHardwareALSA::Release() {
	if ( m_pcm_handle ) {
		common->Printf( "close pcm\n" );
		id_snd_pcm_close( m_pcm_handle );
		m_pcm_handle = NULL;
	}
	if ( m_buffer ) {
		free( m_buffer );
		m_buffer = NULL;
	}
	if ( m_handle ) {
		common->Printf( "dlclose\n" );
		dlclose( m_handle );
		m_handle = NULL;
	}
}

/*
=================
idAudioHardwareALSA::InitFailed
=================	
*/	
void idAudioHardwareALSA::InitFailed() {
	Release();
	cvarSystem->SetCVarBool( "s_noSound", true );
	common->Warning( "sound subsystem disabled\n" );
	common->Printf( "--------------------------------------\n" );
}

/*
=====================
idAudioHardwareALSA::Initialize
=====================
*/
bool idAudioHardwareALSA::Initialize( void ) {
	int err;
	
	common->Printf( "------ Alsa Sound Initialization -----\n" );
	if ( !DLOpen() ) {
		InitFailed();
		return false;
	}
	if ( ( err = id_snd_pcm_open( &m_pcm_handle, s_alsa_pcm.GetString(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK ) ) < 0 ) {
		common->Printf( "snd_pcm_open SND_PCM_STREAM_PLAYBACK '%s' failed: %s\n", s_alsa_pcm.GetString(), id_snd_strerror( err ) );
		InitFailed();
		return false;
	}
	common->Printf( "opened Alsa PCM device %s for playback\n", s_alsa_pcm.GetString() );

	// set hardware parameters ----------------------------------------------------------------------

	// init hwparams with the full configuration space
	snd_pcm_hw_params_t *hwparams;
	// this one is a define
	id_snd_pcm_hw_params_alloca( &hwparams );
	if ( ( err = id_snd_pcm_hw_params_any( m_pcm_handle, hwparams ) ) < 0 ) {
		common->Printf( "cannot configure the PCM device: %s\n", id_snd_strerror( err ) );
		InitFailed();
		return false;
	}

	if ( ( err = id_snd_pcm_hw_params_set_access( m_pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED ) ) < 0 ) {
		common->Printf( "SND_PCM_ACCESS_RW_INTERLEAVED failed: %s\n", id_snd_strerror( err ) );
		InitFailed();
		return false;
	}

	if ( ( err = id_snd_pcm_hw_params_set_format( m_pcm_handle, hwparams, SND_PCM_FORMAT_S16_LE ) ) < 0 ) {
		common->Printf( "SND_PCM_FORMAT_S16_LE failed: %s\n", id_snd_strerror( err ) );
		InitFailed();
		return false;
	}

	// channels

	// sanity over number of speakers
	if ( idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 6 && idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 2 ) {
		common->Warning( "invalid value for s_numberOfSpeakers. Use either 2 or 6" );
		idSoundSystemLocal::s_numberOfSpeakers.SetInteger( 2 );
	}

	m_channels = idSoundSystemLocal::s_numberOfSpeakers.GetInteger();
	if ( ( err = id_snd_pcm_hw_params_set_channels( m_pcm_handle, hwparams, m_channels ) ) < 0 ) {
		common->Printf( "error setting %d channels: %s\n", m_channels, id_snd_strerror( err ) );
		if ( idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 2 ) {
			// fallback to stereo if that works
			m_channels = 2;
			if ( ( err = id_snd_pcm_hw_params_set_channels( m_pcm_handle, hwparams, m_channels ) ) < 0 ) {
				common->Printf( "fallback to stereo failed: %s\n", id_snd_strerror( err ) );
				InitFailed();
				return false;
			} else {
				common->Printf( "fallback to stereo\n" );
				idSoundSystemLocal::s_numberOfSpeakers.SetInteger( 2 );
			}
		} else {
			InitFailed();
			return false;
		}
	}

	// set sample rate (frequency)
	if ( ( err = id_snd_pcm_hw_params_set_rate( m_pcm_handle, hwparams, PRIMARYFREQ, 0 ) ) < 0 ) {
		common->Printf( "failed to set 44.1KHz rate: %s - try ( +set s_alsa_pcm plughw:0 ? )\n", id_snd_strerror( err ) );
		InitFailed();
		return false;
	}

	// have enough space in the input buffer for our MIXBUFFER_SAMPLE feedings and async ticks
	snd_pcm_uframes_t frames;
	frames = MIXBUFFER_SAMPLES + MIXBUFFER_SAMPLES / 3;
	if ( ( err = id_snd_pcm_hw_params_set_buffer_size_min( m_pcm_handle, hwparams, &frames ) ) < 0 ) {
		common->Printf( "buffer size select failed: %s\n", id_snd_strerror( err ) );
		InitFailed();
		return false;
	}

	// apply parameters
	if ( ( err = id_snd_pcm_hw_params( m_pcm_handle, hwparams ) ) < 0 ) {
		common->Printf( "snd_pcm_hw_params failed: %s\n", id_snd_strerror( err ) );
		InitFailed();
		return false;
	}

	// check the buffer size
	if ( ( err = id_snd_pcm_hw_params_get_buffer_size( hwparams, &frames ) ) < 0 ) {
		common->Printf( "snd_pcm_hw_params_get_buffer_size failed: %s\n", id_snd_strerror( err ) );
	} else {
		common->Printf( "device buffer size: %lu frames ( %lu bytes )\n", ( long unsigned int )frames, frames * m_channels * 2 );
	}

	// TODO: can use swparams to setup the device so it doesn't underrun but rather loops over
	// snd_pcm_sw_params_set_stop_threshold
	// To get alsa to just loop on underruns. set the swparam stop_threshold to equal buffer size. The sound buffer will just loop and never throw an xrun.

	// allocate the final mix buffer
	m_buffer_size = MIXBUFFER_SAMPLES * m_channels * 2;
	m_buffer = malloc( m_buffer_size );
	common->Printf( "allocated a mix buffer of %d bytes\n", m_buffer_size );

#ifdef _DEBUG
	// verbose the state
	snd_pcm_state_t curstate = id_snd_pcm_state( m_pcm_handle );
	assert( curstate == SND_PCM_STATE_PREPARED );
#endif
	
	common->Printf( "--------------------------------------\n" );
	return true;
}

/*
===============
idAudioHardwareALSA::~idAudioHardwareALSA
===============
*/
idAudioHardwareALSA::~idAudioHardwareALSA() {
	common->Printf( "----------- Alsa Shutdown ------------\n" );
	Release();
	common->Printf( "--------------------------------------\n" );
}

/*
=================
idAudioHardwareALSA::GetMixBufferSize
=================
*/	
int idAudioHardwareALSA::GetMixBufferSize() {
	return m_buffer_size;
}

/*
=================
idAudioHardwareALSA::GetMixBuffer
=================
*/	
short* idAudioHardwareALSA::GetMixBuffer() {
	return (short *)m_buffer;
}

/*
===============
idAudioHardwareALSA::Flush
===============
*/
bool idAudioHardwareALSA::Flush( void ) {
	int ret;
	snd_pcm_state_t state;
	state = id_snd_pcm_state( m_pcm_handle );
	if ( state != SND_PCM_STATE_RUNNING && state != SND_PCM_STATE_PREPARED ) {
		if ( ( ret = id_snd_pcm_prepare( m_pcm_handle ) ) < 0 ) {
			Sys_Printf( "failed to recover from SND_PCM_STATE_XRUN: %s\n", id_snd_strerror( ret ) );
			cvarSystem->SetCVarBool( "s_noSound", true );
			return false;
		}
		Sys_Printf( "preparing audio device for output\n" );
	}
	Write( true );
}

/*
===============
idAudioHardwareALSA::Write
rely on m_freeWriteChunks which has been set in Flush() before engine did the mixing for this MIXBUFFER_SAMPLE
===============
*/
void idAudioHardwareALSA::Write( bool flushing ) {
	if ( !flushing && m_remainingFrames ) {
		// if we write after a new mixing loop, we should have m_writeChunk == 0
		// otherwise that last remaining chunk that was never flushed out to the audio device has just been overwritten
		Sys_Printf( "idAudioHardwareALSA::Write: %d frames overflowed and dropped\n", m_remainingFrames );
	}
	if ( !flushing ) {
		// if running after the mix loop, then we have a full buffer to write out
		m_remainingFrames = MIXBUFFER_SAMPLES;
	}
	if ( m_remainingFrames == 0 ) {
		return;
	}
	// write the max frames you can in one shot - we need to write it all out in Flush() calls before the next Write() happens
	int pos = (int)m_buffer + ( MIXBUFFER_SAMPLES - m_remainingFrames ) * m_channels * 2;
	snd_pcm_sframes_t frames = id_snd_pcm_writei( m_pcm_handle, (void*)pos, m_remainingFrames );
	if ( frames < 0 ) {
		if ( frames != -EAGAIN ) {
			Sys_Printf( "snd_pcm_writei %d frames failed: %s\n", m_remainingFrames, id_snd_strerror( frames ) );
		}
		return;
	}
	m_remainingFrames -= frames;
}

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
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
// OSS sound interface
// http://www.opensound.com/
#include <sys/soundcard.h>

#include "../../idlib/precompiled.h"
#include "../../sound/snd_local.h"
#include "../posix/posix_public.h"
#include "sound.h"

const char	*s_driverArgs[]	= { "best", "oss", "alsa", NULL };

#ifndef NO_ALSA
static idCVar s_driver( "s_driver", s_driverArgs[0], CVAR_SYSTEM | CVAR_ARCHIVE, "sound driver. 'best' will attempt to use alsa and fallback to OSS if not available", s_driverArgs, idCmdSystem::ArgCompletion_String<s_driverArgs> );
#else
static idCVar s_driver( "s_driver", "oss", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_ROM, "sound driver. only OSS is supported in this build" );
#endif

idAudioHardware *idAudioHardware::Alloc() {
#ifndef NO_ALSA
	if ( !strcmp( s_driver.GetString(), "best" ) ) {
		idAudioHardwareALSA *test = new idAudioHardwareALSA;
		if ( test->DLOpen() ) {
			common->Printf( "Alsa is available\n" );
			return test;
		}
		common->Printf( "Alsa is not available\n" );
		delete test;
		return new idAudioHardwareOSS;
	}
	if ( !strcmp( s_driver.GetString(), "alsa" ) ) {
		return new idAudioHardwareALSA;
	}
#endif
	return new idAudioHardwareOSS;
}

// OSS sound ----------------------------------------------------

/*
===============
idAudioHardware::~idAudioHardware
===============
*/
idAudioHardware::~idAudioHardware() { }
	
/*
=================
idAudioHardwareOSS::~idAudioHardwareOSS
=================	
*/	
idAudioHardwareOSS::~idAudioHardwareOSS() {
	Release();
}

/*
=================
idAudioHardwareOSS::Release
=================	
*/	
void idAudioHardwareOSS::Release( bool bSilent ) {
	if (m_audio_fd) {
		if (!bSilent) {
			common->Printf("------ OSS Sound Shutdown ------\n");
		}
		if (m_buffer) {
			free( m_buffer );
			m_buffer = NULL;
			m_buffer_size = 0;
		}
		common->Printf("close sound device\n");	
		if (close(m_audio_fd) == -1) {
			common->Warning( "failed to close sound device: %s", strerror(errno) );
		}
		m_audio_fd = 0;
		if (!bSilent) {
			common->Printf("--------------------------------\n");
		}
	}
}	

/*
=================
idAudioHardwareOSS::InitFailed
=================	
*/	
void idAudioHardwareOSS::InitFailed() {
	Release( true );
	cvarSystem->SetCVarBool( "s_noSound", true );
	common->Warning( "sound subsystem disabled" );
	common->Printf( "--------------------------------------\n" );
}

/*
=================
idAudioHardwareOSS::ExtractOSSVersion
=================	
*/	
void idAudioHardwareOSS::ExtractOSSVersion( int version, idStr &str ) const {
	sprintf( str, "%d.%d.%d", ( version & 0xFF0000 ) >> 16, ( version & 0xFF00 ) >> 8, version & 0xFF );
}

/*
=================
idAudioHardwareOSS::Initialize

http://www.4front-tech.com/pguide/index.html
though OSS API docs (1.1) advertise AFMT_S32_LE, AFMT_S16_LE is the only output format I've found in kernel emu10k1 headers

BSD NOTE: With the GNU library, you can use free to free the blocks that memalign, posix_memalign, and valloc return.
That does not work in BSD, however--BSD does not provide any way to free such blocks.
=================	
*/
idCVar s_device( "s_dsp", "/dev/dsp", CVAR_SYSTEM | CVAR_ARCHIVE, "" );

bool idAudioHardwareOSS::Initialize( ) {
	common->Printf("------ OSS Sound Initialization ------\n");

	int requested_sample_format, caps, oss_version;
	idStr s_compiled_oss_version, s_oss_version;
	struct audio_buf_info info;

	memset( &info, 0, sizeof( info ) );
	
	if (m_audio_fd) {
		Release();
	}
	
	// open device ------------------------------------------------
	if ((m_audio_fd = open( s_device.GetString(), O_WRONLY | O_NONBLOCK, 0)) == -1) {
		m_audio_fd = 0;
		common->Warning( "failed to open sound device '%s': %s", s_device.GetString(), strerror(errno) );
		InitFailed();
		return false;
	}
	// make it blocking - so write overruns don't fail with 'Resource temporarily unavailable'
	int flags;
	if ( ( flags = fcntl( m_audio_fd, F_GETFL ) ) == -1 ) {
		common->Warning( "failed fcntl F_GETFL on sound device '%s': %s", s_device.GetString(), strerror( errno ) );
		InitFailed();
		return false;
	}
	flags &= ~O_NONBLOCK;
	if ( fcntl( m_audio_fd, F_SETFL, flags ) == -1 ) {
		common->Warning( "failed to clear O_NONBLOCK on sound device '%s': %s", s_device.GetString(), strerror( errno ) );
		InitFailed();
		return false;
	}
	
	common->Printf("opened sound device '%s'\n", s_device.GetString());
	
	// verify capabilities -----------------------------------------

	// may only be available starting with OSS API v4.0
	// http://www.fi.opensound.com/developer/SNDCTL_SYSINFO.html
	// NOTE: at OSS API 4.0 headers, replace OSS_SYSINFO with SNDCTL_SYSINFO
	oss_sysinfo si;
	if ( ioctl( m_audio_fd, OSS_SYSINFO, &si ) == -1 ) {
		common->Printf( "ioctl SNDCTL_SYSINFO failed: %s\nthis ioctl is only available in OSS/Linux implementation. If you run OSS/Free, don't bother.", strerror( errno ) );
	} else {
		common->Printf( "%s: %s %s\n", s_device.GetString(), si.product, si.version );
	}

	if ( ioctl( m_audio_fd, SNDCTL_DSP_GETCAPS, &caps ) == -1 ) {
		common->Warning( "ioctl SNDCTL_DSP_GETCAPS failed - driver too old?" );
		InitFailed();
		return false;
	}
	common->DPrintf("driver rev %d - capabilities %d\n", caps & DSP_CAP_REVISION, caps);
	if (ioctl( m_audio_fd, OSS_GETVERSION, &oss_version ) == -1) {
		common->Warning( "ioctl OSS_GETVERSION failed" );
		InitFailed();
		return false;
	}
	ExtractOSSVersion( oss_version, s_oss_version );
	ExtractOSSVersion( SOUND_VERSION, s_compiled_oss_version );
	common->DPrintf( "OSS interface version %s - compile time %s\n", s_oss_version.c_str(), s_compiled_oss_version.c_str() );
	if (!(caps & DSP_CAP_MMAP)) {
		common->Warning( "driver doesn't have DSP_CAP_MMAP capability" );
		InitFailed();
		return false;
	}
	if (!(caps & DSP_CAP_TRIGGER)) {
		common->Warning( "driver doesn't have DSP_CAP_TRIGGER capability" );
		InitFailed();
		return false;
	}
	
	// sample format -----------------------------------------------
	requested_sample_format = AFMT_S16_LE;
	m_sample_format = requested_sample_format;
	if (ioctl(m_audio_fd, SNDCTL_DSP_SETFMT, &m_sample_format) == -1) {
		common->Warning( "ioctl SNDCTL_DSP_SETFMT %d failed: %s", requested_sample_format, strerror(errno) );
		InitFailed();
		return false;
	}
	if ( m_sample_format != requested_sample_format ) {
		common->Warning( "ioctl SNDCTL_DSP_SETFMT failed to get the requested sample format %d, got %d", requested_sample_format, m_sample_format );
		InitFailed();
		return false;
	}
	
	// channels ----------------------------------------------------

	// sanity over number of speakers
	if ( idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 6 && idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 2 ) {
		common->Warning( "invalid value for s_numberOfSpeakers. Use either 2 or 6" );
		idSoundSystemLocal::s_numberOfSpeakers.SetInteger( 2 );
	}

	m_channels = idSoundSystemLocal::s_numberOfSpeakers.GetInteger();
	if ( ioctl( m_audio_fd, SNDCTL_DSP_CHANNELS, &m_channels ) == -1 ) {
		common->Warning( "ioctl SNDCTL_DSP_CHANNELS %d failed: %s", idSoundSystemLocal::s_numberOfSpeakers.GetInteger(), strerror(errno) );
		InitFailed();
		return false;
	}
	if ( m_channels != (unsigned int)idSoundSystemLocal::s_numberOfSpeakers.GetInteger() ) {
		common->Warning( "ioctl SNDCTL_DSP_CHANNELS failed to get the %d requested channels, got %d", idSoundSystemLocal::s_numberOfSpeakers.GetInteger(), m_channels );
		if ( m_channels != 2 && idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 2 ) {
			// we didn't request 2 channels, some drivers reply 1 channel on error but may still let us still get 2 if properly asked
			m_channels = 2;
			if ( ioctl( m_audio_fd, SNDCTL_DSP_CHANNELS, &m_channels ) == -1 ) {
				common->Warning( "ioctl SNDCTL_DSP_CHANNELS fallback to 2 failed: %s", strerror(errno) );
				InitFailed();
				return false;
			}
		}
		if ( m_channels == 2 ) {
			// tell the system to mix 2 channels
			common->Warning( "falling back to stereo" );
			idSoundSystemLocal::s_numberOfSpeakers.SetInteger( 2 );
		} else {
			// disable sound
			InitFailed();
			return false;
		}
	}
	assert( (int)m_channels == idSoundSystemLocal::s_numberOfSpeakers.GetInteger() );
	
	// sampling rate ------------------------------------------------
	m_speed = PRIMARYFREQ;
	if ( ioctl( m_audio_fd, SNDCTL_DSP_SPEED, &m_speed ) == -1 ) {
		common->Warning( "ioctl SNDCTL_DSP_SPEED %d failed: %s", PRIMARYFREQ, strerror(errno) );
		InitFailed();
		return false;
	}
	// instead of an exact match, do a very close to
	// there is some horrible Ensonic ES1371 which replies 44101 for a 44100 request
	if ( abs( m_speed - PRIMARYFREQ ) > 5 ) {
		common->Warning( "ioctl SNDCTL_DSP_SPEED failed to get the requested frequency %d, got %d", PRIMARYFREQ, m_speed );
		InitFailed();
		return false;
	}
	common->Printf("%s - bit rate: %d, channels: %d, frequency: %d\n", s_device.GetString(), m_sample_format, m_channels, m_speed);
	
	// output buffer ------------------------------------------------
	// allocate a final buffer target, the sound engine locks, writes, and we write back to the device
	// we want m_buffer_size ( will have to rename those )
	// ROOM_SLICES_IN_BUFFER is fixed ( system default, 10 )
	// MIXBUFFER_SAMPLES is the number of samples found in a slice
	// each sample is m_channels * sizeof( float ) bytes
	// in AsyncUpdate we only write one block at a time, so we'd only need to have a final mix buffer sized of a single block
	m_buffer_size = MIXBUFFER_SAMPLES * m_channels * 2;
	m_buffer = malloc( m_buffer_size );
	common->Printf( "allocated a mix buffer of %d bytes\n", m_buffer_size );

	// toggle sound -------------------------------------------------
	
	// toggle off before toggling on. that's what OSS source code samples recommends
	int flag = 0;
	if (ioctl(m_audio_fd, SNDCTL_DSP_SETTRIGGER, &flag) == -1) {
		common->Warning( "ioctl SNDCTL_DSP_SETTRIGGER 0 failed: %s", strerror(errno) );
	}
	flag = PCM_ENABLE_OUTPUT;
	if (ioctl(m_audio_fd, SNDCTL_DSP_SETTRIGGER, &flag) == -1) {
		common->Warning( "ioctl SNDCTL_DSP_SETTRIGGER PCM_ENABLE_OUTPUT failed: %s", strerror(errno) );
	}

	common->Printf("--------------------------------------\n");
	return true;
}

/*
===============
idAudioHardwareOSS::Flush
===============
*/
bool idAudioHardwareOSS::Flush( void ) {
	audio_buf_info ospace;
	if ( ioctl( m_audio_fd, SNDCTL_DSP_GETOSPACE, &ospace ) == -1 ) {
		Sys_Printf( "ioctl SNDCTL_DSP_GETOSPACE failed: %s\n", strerror( errno ) );
		return false;
	}
	// how many chunks can we write to the audio device right now
	m_freeWriteChunks = ( ospace.bytes * MIXBUFFER_CHUNKS ) / ( MIXBUFFER_SAMPLES * m_channels * 2 );
	if ( m_writeChunks ) {
		// flush out any remaining chunks we could now
		Write( true );
	}
	return ( m_freeWriteChunks > 0 );
}

/*
=================
idAudioHardwareOSS::GetMixBufferSize
=================
*/	
int idAudioHardwareOSS::GetMixBufferSize() {
	//	return MIXBUFFER_SAMPLES * 2 * m_channels;
	return m_buffer_size;
}

/*
=================
idAudioHardwareOSS::GetMixBuffer
=================
*/	
short* idAudioHardwareOSS::GetMixBuffer() {
	return (short *)m_buffer;
}

/*
===============
idAudioHardwareOSS::Write
rely on m_freeWriteChunks which has been set in Flush() before engine did the mixing for this MIXBUFFER_SAMPLE
===============
*/
void idAudioHardwareOSS::Write( bool flushing ) {
	assert( m_audio_fd );
	int ret;
	if ( !flushing && m_writeChunks ) {
		// if we write after a new mixing loop, we should have m_writeChunk == 0
		// otherwise that last remaining chunk that was never flushed out to the audio device has just been overwritten
		Sys_Printf( "idAudioHardwareOSS::Write: %d samples were overflowed and dropped\n", m_writeChunks * MIXBUFFER_SAMPLES / MIXBUFFER_CHUNKS );
	}
	if ( !flushing ) {
		// if running after the mix loop, then we have a full buffer to write out
		m_writeChunks = MIXBUFFER_CHUNKS;
	}
	if ( m_freeWriteChunks == 0 ) {
		return;
	}
	// what to write and how much
	int pos = (int)m_buffer + ( MIXBUFFER_CHUNKS - m_writeChunks ) * m_channels * 2 * MIXBUFFER_SAMPLES / MIXBUFFER_CHUNKS;
	int len = Min( m_writeChunks, m_freeWriteChunks ) * m_channels * 2 * MIXBUFFER_SAMPLES / MIXBUFFER_CHUNKS;
	assert( len > 0 );
	if ( ( ret = write( m_audio_fd, (void*)pos, len ) ) == -1 ) {
		Sys_Printf( "write to audio fd failed: %s\n", strerror( errno ) );
		return;
	}
	if ( len != ret ) {
		Sys_Printf( "short write to audio fd: wrote %d out of %d\n", ret, m_buffer_size );
		return;
	}
	m_writeChunks -= Min( m_writeChunks, m_freeWriteChunks );
}

/*
 ===============
 Sys_LoadOpenAL
 -===============
 */
bool Sys_LoadOpenAL( void ) {
	return false;
}


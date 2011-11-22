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
#ifndef ID_SND_BACKENDS
#define ID_SND_BACKENDS

class idAudioHardwareOSS : public idAudioHardware {
	// if you can't write MIXBUFFER_SAMPLES all at once to the audio device, split in MIXBUFFER_CHUNKS
	static const int MIXBUFFER_CHUNKS = 4;

	int				m_audio_fd;
	int				m_sample_format;
	unsigned int	m_channels;
	unsigned int	m_speed;
	void			*m_buffer;
	int				m_buffer_size;
	
					// counting the loops through the dma buffer
	int				m_loops;
	
					// how many chunks we have left to write in cases where we need to split
	int				m_writeChunks;
					// how many chunks we can write to the audio device without blocking
	int				m_freeWriteChunks;
	
public:
	idAudioHardwareOSS() { 
		m_audio_fd = 0;
		m_sample_format = 0;
		m_channels = 0;
		m_speed = 0;
		m_buffer = NULL;
		m_buffer_size = 0;
		m_loops = 0;
		m_writeChunks		= 0;
		m_freeWriteChunks	= 0;
	}
	virtual		~idAudioHardwareOSS();

	bool		Initialize( void );

	// Linux driver doesn't support memory map API
	bool		Lock( void **pDSLockedBuffer, ulong *dwDSLockedBufferSize ) { return false; }
	bool		Unlock( void *pDSLockedBuffer, dword dwDSLockedBufferSize ) { return false; }
	bool		GetCurrentPosition( ulong *pdwCurrentWriteCursor ) { return false; }
	
	bool		Flush();
	void		Write( bool flushing );

	int			GetNumberOfSpeakers() { return m_channels; }
	int			GetMixBufferSize();
	short*		GetMixBuffer();
		
private:
	void		Release( bool bSilent = false );
	void		InitFailed();
	void		ExtractOSSVersion( int version, idStr &str ) const;
};

#ifndef NO_ALSA

// libasound2-dev
// the new/old API may be a problem if we are going to dynamically load the asound lib?
#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>

#define id_snd_pcm_hw_params_alloca(ptr) do { assert(ptr); *ptr = (snd_pcm_hw_params_t *) alloca(id_snd_pcm_hw_params_sizeof()); memset(*ptr, 0, id_snd_pcm_hw_params_sizeof()); } while (0)

typedef const char * ( *pfn_snd_asoundlib_version )( void );
typedef snd_pcm_sframes_t ( *pfn_snd_pcm_avail_update )( snd_pcm_t *pcm );
typedef int ( *pfn_snd_pcm_close )( snd_pcm_t *pcm );
typedef const char * ( *pfn_snd_strerror )( int errnum );
typedef int ( *pfn_snd_pcm_hw_params )( snd_pcm_t *pcm, snd_pcm_hw_params_t *params );
typedef int ( *pfn_snd_pcm_hw_params_any )( snd_pcm_t *pcm, snd_pcm_hw_params_t *params );
typedef int ( *pfn_snd_pcm_hw_params_get_buffer_size )( const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val );
typedef int ( *pfn_snd_pcm_hw_params_set_access )( snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t access );
typedef int ( *pfn_snd_pcm_hw_params_set_buffer_size_min )( snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val );
typedef	int ( *pfn_snd_pcm_hw_params_set_channels )( snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val );
typedef int ( *pfn_snd_pcm_hw_params_set_format )( snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t format );
typedef int ( *pfn_snd_pcm_hw_params_set_rate )( snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir );
typedef size_t ( *pfn_snd_pcm_hw_params_sizeof )( void );
typedef int ( *pfn_snd_pcm_open )( snd_pcm_t **pcmp, const char *name, snd_pcm_stream_t stream, int mode );
typedef int ( *pfn_snd_pcm_prepare )( snd_pcm_t *pcm );
typedef snd_pcm_state_t ( *pfn_snd_pcm_state )( snd_pcm_t *pcm );
typedef snd_pcm_sframes_t ( *pfn_snd_pcm_writei )( snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size );

#define ALSA_DLSYM(SYM)	id_##SYM = ( pfn_##SYM )dlvsym( m_handle, #SYM, "ALSA_0.9" ); if ( !id_##SYM ) { common->Printf( "dlsym "#SYM" failed: %s\n", dlerror() ); Release(); return false; }

class idAudioHardwareALSA : public idAudioHardware {
private:
	// if you can't write MIXBUFFER_SAMPLES all at once to the audio device, split in MIXBUFFER_CHUNKS
	static const int	MIXBUFFER_CHUNKS = 4;

	snd_pcm_t			*m_pcm_handle;
	unsigned int		m_channels;
	void				*m_buffer;
	int					m_buffer_size;

	// how many frames remaining to be written to the device
	int					m_remainingFrames;

	void				*m_handle;
	
public:
						idAudioHardwareALSA() {
							m_pcm_handle		= NULL;
							m_channels			= 0;
							m_buffer			= NULL;
							m_buffer_size		= 0;
							m_remainingFrames	= 0;
							m_handle			= NULL;
						}
						virtual				~idAudioHardwareALSA();

						// dlopen the lib ( check minimum version )
	bool				DLOpen();

    bool				Initialize( void );


	// Linux driver doesn't support memory map API
	bool				Lock( void **pDSLockedBuffer, ulong *dwDSLockedBufferSize ) { return false; }
	bool				Unlock( void *pDSLockedBuffer, dword dwDSLockedBufferSize ) { return false; }
	bool				GetCurrentPosition( ulong *pdwCurrentWriteCursor ) { return false; }
	
	bool				Flush();
	void				Write( bool flushing );

	int					GetNumberOfSpeakers( void ) { return m_channels; }
	int					GetMixBufferSize( void );
	short*				GetMixBuffer( void );

private:
	void				Release();
	void				InitFailed();
	void				PlayTestPattern();

	// may be NULL, outdated alsa versions are missing it and we just ignore
	pfn_snd_asoundlib_version id_snd_asoundlib_version;

	pfn_snd_pcm_avail_update id_snd_pcm_avail_update;
	pfn_snd_pcm_close id_snd_pcm_close;
	pfn_snd_strerror id_snd_strerror;
	pfn_snd_pcm_hw_params id_snd_pcm_hw_params;
	pfn_snd_pcm_hw_params_any id_snd_pcm_hw_params_any;
	pfn_snd_pcm_hw_params_get_buffer_size id_snd_pcm_hw_params_get_buffer_size;
	pfn_snd_pcm_hw_params_set_access id_snd_pcm_hw_params_set_access;
	pfn_snd_pcm_hw_params_set_buffer_size_min id_snd_pcm_hw_params_set_buffer_size_min;
	pfn_snd_pcm_hw_params_set_channels id_snd_pcm_hw_params_set_channels;
	pfn_snd_pcm_hw_params_set_format id_snd_pcm_hw_params_set_format;
	pfn_snd_pcm_hw_params_set_rate id_snd_pcm_hw_params_set_rate;
	pfn_snd_pcm_hw_params_sizeof id_snd_pcm_hw_params_sizeof;
	pfn_snd_pcm_open id_snd_pcm_open;
	pfn_snd_pcm_prepare id_snd_pcm_prepare;
	pfn_snd_pcm_state id_snd_pcm_state;
	pfn_snd_pcm_writei id_snd_pcm_writei;

};

#endif // NO_ALSA

#endif

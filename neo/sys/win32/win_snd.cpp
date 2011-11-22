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
#pragma hdrstop

// DirectX SDK
#include <DxErr.h>

#include <ks.h>
#include <ksmedia.h>
#include "../../sound/snd_local.h"
#include "win_local.h"

#include "../../openal/idal.cpp"

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

class idAudioBufferWIN32 : public idAudioBuffer {
public:
    idAudioBufferWIN32( LPDIRECTSOUNDBUFFER apDSBuffer, dword dwDSBufferSize, idWaveFile* pWaveFile=NULL );
    ~idAudioBufferWIN32();

    int FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, bool bRepeatWavIfBufferLarger );

	bool Lock( void **pDSLockedBuffer, ulong *dwDSLockedBufferSize );
	bool Unlock(void *pDSLockedBuffer, dword dwDSLockedBufferSize );
	bool GetCurrentPosition( ulong *pdwCurrentWriteCursor );

    int Play( dword dwPriority=0, dword dwFlags=0 );
    int Stop( void );
    int Reset( void );
    bool    IsSoundPlaying( void );
	void SetVolume( float x);

    idWaveFile*			m_pWaveFile;
private:
    LPDIRECTSOUNDBUFFER		m_apDSBuffer;
    dword					m_dwDSBufferSize;

    int RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, bool* pbWasRestored );
};

class idAudioHardwareWIN32 : public idAudioHardware {

public:
    idAudioHardwareWIN32();
    ~idAudioHardwareWIN32();

    bool Initialize( );
	bool InitializeSpeakers( byte *buffer, int bufferSize, dword dwPrimaryFreq, dword dwPrimaryBitRate, dword dwSpeakers );

	void SetPrimaryBufferFormat( dword dwPrimaryFreq, dword dwPrimaryBitRate, dword dwSpeakers );

    int Create( idWaveFile* pWaveFile, idAudioBuffer** ppiab );
    int Create( idAudioBuffer** ppSound, const char* strWaveFileName, dword dwCreationFlags = 0 );
    int CreateFromMemory( idAudioBufferWIN32** ppSound, byte* pbData, ulong ulDataSize, waveformatextensible_t *pwfx, dword dwCreationFlags = 0 );

	bool Lock( void **pDSLockedBuffer, ulong *dwDSLockedBufferSize );
	bool Unlock( void *pDSLockedBuffer, dword dwDSLockedBufferSize );
	bool GetCurrentPosition( ulong *pdwCurrentWriteCursor );
	
	int				GetNumberOfSpeakers()				{ return numSpeakers; }
	int				GetMixBufferSize()					{ return MIXBUFFER_SAMPLES * blockAlign; }

	// WIN32 driver doesn't support write API
	bool Flush( void ) { return true; }
	void Write( bool ) { }
	short* GetMixBuffer( void ) { return NULL; }

private:
    LPDIRECTSOUND			m_pDS;
    LPDIRECTSOUNDBUFFER		pDSBPrimary;
	idAudioBufferWIN32		*speakers;

	int						numSpeakers;
	int						bitsPerSample;
	int						bufferSize;		// allocate buffer handed over to DirectSound
	int						blockAlign;		// channels * bits per sample / 8: sound frame size
};

idAudioHardware *idAudioHardware::Alloc() { return new idAudioHardwareWIN32; }
idAudioHardware::~idAudioHardware() {}

/*
================
idAudioHardwareWIN32::idAudioHardware
================
*/
idAudioHardwareWIN32::idAudioHardwareWIN32() {
    m_pDS = NULL;
	pDSBPrimary = NULL;
	speakers = NULL;
}

/*
================
idAudioHardwareWIN32::~idAudioHardware
================
*/
idAudioHardwareWIN32::~idAudioHardwareWIN32() {
	SAFE_DELETE( speakers );
	SAFE_RELEASE( pDSBPrimary );
    SAFE_RELEASE( m_pDS ); 
}

/*
===============
idAudioHardwareWIN32::Initialize
===============
*/
bool idAudioHardwareWIN32::Initialize( void ) {
    int             hr;

	bufferSize = 0;
	numSpeakers = 0;
	blockAlign = 0;

    SAFE_RELEASE( m_pDS );

    // Create IDirectSound using the primary sound device
    if( FAILED( hr = DirectSoundCreate( NULL, &m_pDS, NULL ) )) {
        return false;
	}

    // Set primary buffer format
	SetPrimaryBufferFormat( PRIMARYFREQ, 16, idSoundSystemLocal::s_numberOfSpeakers.GetInteger() );
	return true;
}

/*
===============
idAudioHardwareWIN32::InitializeSpeakers
===============
*/
bool idAudioHardwareWIN32::InitializeSpeakers( byte *speakerData, int bufferSize, dword dwPrimaryFreq, dword dwPrimaryBitRate, dword dwSpeakers ) {
	if ( dwSpeakers == 2 ) {
		WAVEFORMATEXTENSIBLE wfx;
		ZeroMemory( &wfx, sizeof(WAVEFORMATEXTENSIBLE) ); 
		wfx.Format.wFormatTag      = WAVE_FORMAT_PCM; 
		wfx.Format.nChannels       = 2; 
		wfx.Format.nSamplesPerSec  = dwPrimaryFreq;
		wfx.Format.wBitsPerSample  = dwPrimaryBitRate; 
		wfx.Format.nBlockAlign     = wfx.Format.wBitsPerSample / 8 * wfx.Format.nChannels;
		wfx.Format.nAvgBytesPerSec = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;
		wfx.Format.cbSize = sizeof(WAVEFORMATEX);

		CreateFromMemory( &speakers, speakerData, bufferSize, (waveformatextensible_t *)&wfx );

		common->Printf("sound: STEREO\n");
	} else {
		WAVEFORMATEXTENSIBLE 	waveFormatPCMEx;
		ZeroMemory( &waveFormatPCMEx, sizeof(WAVEFORMATEXTENSIBLE) ); 

 		waveFormatPCMEx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		waveFormatPCMEx.Format.nChannels = 6;
		waveFormatPCMEx.Format.nSamplesPerSec = dwPrimaryFreq;
		waveFormatPCMEx.Format.wBitsPerSample  = dwPrimaryBitRate;
		waveFormatPCMEx.Format.nBlockAlign = waveFormatPCMEx.Format.wBitsPerSample / 8 * waveFormatPCMEx.Format.nChannels;
		waveFormatPCMEx.Format.nAvgBytesPerSec = waveFormatPCMEx.Format.nSamplesPerSec * waveFormatPCMEx.Format.nBlockAlign;
		waveFormatPCMEx.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
									 // SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
									 // SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
									 // SPEAKER_BACK_LEFT  | SPEAKER_BACK_RIGHT
		waveFormatPCMEx.SubFormat =  KSDATAFORMAT_SUBTYPE_PCM;  // Specify PCM
		waveFormatPCMEx.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE);
		waveFormatPCMEx.Samples.wValidBitsPerSample = 16;

		CreateFromMemory( &speakers, speakerData, bufferSize, (waveformatextensible_t *)&waveFormatPCMEx );

		common->Printf("sound: MULTICHANNEL\n");
	}

	if (!speakers) {
		return false;
	}

	speakers->Play(0,DSBPLAY_LOOPING);

	return true;
}

/*
===============
idAudioHardwareWIN32::SetPrimaryBufferFormat
Set primary buffer to a specified format 
For example, to set the primary buffer format to 22kHz stereo, 16-bit
then:   dwPrimaryChannels = 2
        dwPrimaryFreq     = 22050, 
        dwPrimaryBitRate  = 16
===============
*/
void idAudioHardwareWIN32::SetPrimaryBufferFormat( dword dwPrimaryFreq, dword dwPrimaryBitRate, dword dwSpeakers ) {
    HRESULT             hr;

    if( m_pDS == NULL ) {
        return;
	}

	ulong cfgSpeakers;
	m_pDS->GetSpeakerConfig( &cfgSpeakers );

	DSCAPS dscaps; 
	dscaps.dwSize = sizeof(DSCAPS); 
    m_pDS->GetCaps(&dscaps); 

	if (dscaps.dwFlags & DSCAPS_EMULDRIVER) {
		return;
	}

	// Get the primary buffer 
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat   = NULL;
       
	// Obtain write-primary cooperative level. 
	if( FAILED( hr = m_pDS->SetCooperativeLevel(win32.hWnd, DSSCL_PRIORITY ) ) ) {
        DXTRACE_ERR( TEXT("SetPrimaryBufferFormat"), hr );
		return;
	}

	if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL ) ) ) {
		return;
	}

	if ( dwSpeakers == 6 && (cfgSpeakers == DSSPEAKER_5POINT1 || cfgSpeakers == DSSPEAKER_SURROUND) ) {
		WAVEFORMATEXTENSIBLE 	waveFormatPCMEx;
		ZeroMemory( &waveFormatPCMEx, sizeof(WAVEFORMATEXTENSIBLE) ); 

 		waveFormatPCMEx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		waveFormatPCMEx.Format.nChannels = 6;
		waveFormatPCMEx.Format.nSamplesPerSec = dwPrimaryFreq;
		waveFormatPCMEx.Format.wBitsPerSample  = (WORD) dwPrimaryBitRate; 
		waveFormatPCMEx.Format.nBlockAlign = waveFormatPCMEx.Format.wBitsPerSample / 8 * waveFormatPCMEx.Format.nChannels;
		waveFormatPCMEx.Format.nAvgBytesPerSec = waveFormatPCMEx.Format.nSamplesPerSec * waveFormatPCMEx.Format.nBlockAlign;
		waveFormatPCMEx.dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
									 // SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
									 // SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
									 // SPEAKER_BACK_LEFT  | SPEAKER_BACK_RIGHT
		waveFormatPCMEx.SubFormat =  KSDATAFORMAT_SUBTYPE_PCM;  // Specify PCM
		waveFormatPCMEx.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE);
		waveFormatPCMEx.Samples.wValidBitsPerSample = 16;

		if( FAILED( hr = pDSBPrimary->SetFormat((WAVEFORMATEX*)&waveFormatPCMEx) ) ) {
	        DXTRACE_ERR( TEXT("SetPrimaryBufferFormat"), hr );
			return;
		}
		numSpeakers = 6;		// force it to think 5.1
		blockAlign = waveFormatPCMEx.Format.nBlockAlign;
	} else {
		if (dwSpeakers == 6) {
			common->Printf("sound: hardware reported unable to use multisound, defaulted to stereo\n");
		}
		WAVEFORMATEX wfx;
		ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
		wfx.wFormatTag      = WAVE_FORMAT_PCM; 
		wfx.nChannels       = 2; 
		wfx.nSamplesPerSec  = dwPrimaryFreq; 
		wfx.wBitsPerSample  = (WORD) dwPrimaryBitRate; 
		wfx.nBlockAlign     = wfx.wBitsPerSample / 8 * wfx.nChannels;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize = sizeof(WAVEFORMATEX);

		if( FAILED( hr = pDSBPrimary->SetFormat(&wfx) ) ) {
			return;
		}
		numSpeakers = 2;		// force it to think stereo
		blockAlign = wfx.nBlockAlign;
	}

	byte *speakerData;
	bufferSize = MIXBUFFER_SAMPLES * sizeof(word) * numSpeakers * ROOM_SLICES_IN_BUFFER;
	speakerData = (byte *)Mem_Alloc( bufferSize );
	memset( speakerData, 0, bufferSize );

	InitializeSpeakers( speakerData, bufferSize, dwPrimaryFreq, dwPrimaryBitRate, numSpeakers );
}

/*
===============
idAudioHardwareWIN32::Create
===============
*/
int idAudioHardwareWIN32::Create( idAudioBuffer** ppSound, 
                               const char* strWaveFileName, 
							   dword dwCreationFlags ) {
    int hr;
    LPDIRECTSOUNDBUFFER   apDSBuffer     = NULL;
    dword                 dwDSBufferSize = NULL;
    idWaveFile*          pWaveFile      = NULL;

    if( m_pDS == NULL )
        return -1;
    if( strWaveFileName == NULL || ppSound == NULL )
        return -1;

	pWaveFile = new idWaveFile();

    pWaveFile->Open( strWaveFileName, NULL );

    if( pWaveFile->GetOutputSize() == 0 )     {
        // Wave is blank, so don't create it.
        hr = E_FAIL;
        goto LFail;
    }

    // Make the DirectSound buffer the same size as the wav file
    dwDSBufferSize = pWaveFile->GetOutputSize();

    // Create the direct sound buffer, and only request the flags needed
    // since each requires some overhead and limits if the buffer can 
    // be hardware accelerated
    DSBUFFERDESC dsbd;
    memset( &dsbd, 0, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = dwCreationFlags;
    dsbd.dwBufferBytes   = dwDSBufferSize;
    dsbd.guid3DAlgorithm = GUID_NULL;
    dsbd.lpwfxFormat     = (WAVEFORMATEX*)&pWaveFile->mpwfx;

    // DirectSound is only guarenteed to play PCM data.  Other
    // formats may or may not work depending the sound card driver.
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &apDSBuffer, NULL ) ) )
		return -1;

    // Create the sound
    *ppSound = new idAudioBufferWIN32( apDSBuffer, dwDSBufferSize, pWaveFile );
    
    pWaveFile->Close();

    return 0;

LFail:
    // Cleanup
    SAFE_DELETE( pWaveFile );
    return -1;
}

/*
===============
idAudioHardwareWIN32::Create
===============
*/
int idAudioHardwareWIN32::Create( idWaveFile* pWaveFile, idAudioBuffer** ppiab ) {
    int hr;
    LPDIRECTSOUNDBUFFER   apDSBuffer     = NULL;
    dword                dwDSBufferSize = NULL;

    if( m_pDS == NULL )
        return -1;

    if( pWaveFile == NULL )
        return -1;

	*ppiab = NULL;

    if( pWaveFile->GetOutputSize() == 0 )     {
        // Wave is blank, so don't create it.
        hr = E_FAIL;
        goto LFail;
    }

    // Make the DirectSound buffer the same size as the wav file
    dwDSBufferSize = pWaveFile->GetOutputSize();

    // Create the direct sound buffer, and only request the flags needed
    // since each requires some overhead and limits if the buffer can 
    // be hardware accelerated
    DSBUFFERDESC dsbd;
    memset( &dsbd, 0, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = 0;
    dsbd.dwBufferBytes   = dwDSBufferSize;
    dsbd.guid3DAlgorithm = GUID_NULL;
    dsbd.lpwfxFormat     = (WAVEFORMATEX*)&pWaveFile->mpwfx;

    // DirectSound is only guarenteed to play PCM data.  Other
    // formats may or may not work depending the sound card driver.
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &apDSBuffer, NULL ) ) )
		return -1;

    // Create the sound
    *ppiab = new idAudioBufferWIN32( apDSBuffer, dwDSBufferSize, pWaveFile );
    
    return 0;

LFail:
    // Cleanup
    SAFE_DELETE( pWaveFile );
    return -1;
}

//-----------------------------------------------------------------------------
// Name: idAudioHardwareWIN32::CreateFromMemory()
// Desc: 
//-----------------------------------------------------------------------------
int idAudioHardwareWIN32::CreateFromMemory( idAudioBufferWIN32** ppSound, 
                                        byte* pbData,
                                        ulong  ulDataSize,
                                        waveformatextensible_t* pwfx,
										dword dwCreationFlags ) {
    int hr;
    LPDIRECTSOUNDBUFFER		apDSBuffer     = NULL;
    dword					dwDSBufferSize = NULL;
    idWaveFile*			pWaveFile      = NULL;

    if( m_pDS == NULL )
        return -1;
    if( pbData == NULL || ppSound == NULL )
        return -1;

    pWaveFile = new idWaveFile();

    pWaveFile->OpenFromMemory( (short *)pbData, ulDataSize, (waveformatextensible_t *)pwfx);


    // Make the DirectSound buffer the same size as the wav file
    dwDSBufferSize = ulDataSize;

    // Create the direct sound buffer, and only request the flags needed
    // since each requires some overhead and limits if the buffer can 
    // be hardware accelerated
    DSBUFFERDESC dsbd;
    memset( &dsbd, 0, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = dwCreationFlags | DSBCAPS_GETCURRENTPOSITION2;
    dsbd.dwBufferBytes   = dwDSBufferSize;
    dsbd.guid3DAlgorithm = GUID_NULL;
    dsbd.lpwfxFormat     = (WAVEFORMATEX *)pwfx;

    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &apDSBuffer, NULL ) ) )
		return -1;

    // Create the sound
    *ppSound = new idAudioBufferWIN32( apDSBuffer, dwDSBufferSize, pWaveFile );

    return S_OK;
}

/*
===============
idAudioHardwareWIN32::Lock
===============
*/
bool idAudioHardwareWIN32::Lock( void **pDSLockedBuffer, ulong *dwDSLockedBufferSize ) {
	if (speakers) {
		return speakers->Lock( pDSLockedBuffer, dwDSLockedBufferSize );
	}
	return false;
}

/*
===============
idAudioHardwareWIN32::Unlock
===============
*/
bool idAudioHardwareWIN32::Unlock(void *pDSLockedBuffer, dword dwDSLockedBufferSize ) {
	if (speakers) {
		return speakers->Unlock( pDSLockedBuffer, dwDSLockedBufferSize );
	}
	return false;
}

/*
===============
idAudioHardwareWIN32::GetCurrentPosition
===============
*/
bool idAudioHardwareWIN32::GetCurrentPosition( ulong *pdwCurrentWriteCursor ) {
	if (speakers) {
		return speakers->GetCurrentPosition( pdwCurrentWriteCursor );
	}
	return false;
}

static HMODULE hOpenAL = NULL;

/*
===============
Sys_LoadOpenAL
===============
*/
bool Sys_LoadOpenAL( void ) {
#if ID_OPENAL
	const char *sym;

	if ( hOpenAL ) {
		return true;
	}

	hOpenAL = LoadLibrary( idSoundSystemLocal::s_libOpenAL.GetString() );
	if ( !hOpenAL ) {
		common->Warning( "LoadLibrary %s failed.", idSoundSystemLocal::s_libOpenAL.GetString() );
		return false;
	}
	if ( ( sym = InitializeIDAL( hOpenAL ) ) ) {
		common->Warning( "GetProcAddress %s failed.", sym );
		FreeLibrary( hOpenAL );
		hOpenAL = NULL;
		return false;
	}
	return true;
#else
	return false;
#endif
}

/*
===============
Sys_FreeOpenAL
===============
*/
void Sys_FreeOpenAL( void ) {
	if ( hOpenAL ) {
		FreeLibrary( hOpenAL );
		hOpenAL = NULL;
	}
}

/*
===============
idAudioBufferWIN32::idAudioBuffer
===============
*/
idAudioBufferWIN32::idAudioBufferWIN32( LPDIRECTSOUNDBUFFER apDSBuffer, dword dwDSBufferSize, idWaveFile* pWaveFile ) {

    m_apDSBuffer = apDSBuffer;

    m_dwDSBufferSize = dwDSBufferSize;
    m_pWaveFile      = pWaveFile;

	if (pWaveFile) {
	    FillBufferWithSound( m_apDSBuffer, false );

		m_apDSBuffer->SetCurrentPosition(0);
	}
}

/*
===============
idAudioBufferWIN32::~idAudioBuffer
===============
*/
idAudioBufferWIN32::~idAudioBufferWIN32() {
	SAFE_DELETE(m_pWaveFile);
	SAFE_RELEASE( m_apDSBuffer ); 
	m_pWaveFile = NULL;
	m_apDSBuffer = NULL;
}

/*
===============
idAudioBufferWIN32::FillBufferWithSound
===============
*/
int idAudioBufferWIN32::FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, bool bRepeatWavIfBufferLarger ) {
    int hr; 
    void*   pDSLockedBuffer      = NULL; // Pointer to locked buffer memory
    ulong   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
    int		dwWavDataRead        = 0;    // Amount of data read from the wav file 

    if( pDSB == NULL )
        return -1;

	// we may not even have a wavefile
	if (m_pWaveFile==NULL) {
		return -1;
	}

    // Make sure we have focus, and we didn't just switch in from
    // an app which had a DirectSound device
    if( FAILED( hr = RestoreBuffer( pDSB, NULL ) ) ) {
        DXTRACE_ERR( TEXT("RestoreBuffer"), hr );
		return -1;
	}

    // Lock the buffer down
    if( FAILED( hr = pDSB->Lock( 0, m_dwDSBufferSize, &pDSLockedBuffer, &dwDSLockedBufferSize, NULL, NULL, 0L ) ) ) {
        DXTRACE_ERR( TEXT("Lock"), hr );
		return -1;
	}

    // Reset the wave file to the beginning 
    m_pWaveFile->ResetFile();

    if( FAILED( hr = m_pWaveFile->Read( (byte*) pDSLockedBuffer, dwDSLockedBufferSize, &dwWavDataRead ) ) ) {
        return DXTRACE_ERR( TEXT("Read"), hr );
	}

    if( dwWavDataRead == 0 ) {
        // Wav is blank, so just fill with silence
        memset( pDSLockedBuffer, (byte)(m_pWaveFile->mpwfx.Format.wBitsPerSample == 8 ? 128 : 0 ), dwDSLockedBufferSize );
    }  else if( dwWavDataRead < (int)dwDSLockedBufferSize ) {
        // If the wav file was smaller than the DirectSound buffer, 
        // we need to fill the remainder of the buffer with data 
        if( bRepeatWavIfBufferLarger ) {       
            // Reset the file and fill the buffer with wav data
            int dwReadSoFar = dwWavDataRead;    // From previous call above.
            while( dwReadSoFar < (int)dwDSLockedBufferSize ) {  
                // This will keep reading in until the buffer is full 
                // for very short files
                if( FAILED( hr = m_pWaveFile->ResetFile() ) ) {
                    return DXTRACE_ERR( TEXT("ResetFile"), hr );
				}

                hr = m_pWaveFile->Read( (byte*)pDSLockedBuffer + dwReadSoFar, dwDSLockedBufferSize - dwReadSoFar, &dwWavDataRead );
                if( FAILED(hr) ) {
                    return DXTRACE_ERR( TEXT("Read"), hr );
				}

                dwReadSoFar += dwWavDataRead;
            } 
        } else {
            // Don't repeat the wav file, just fill in silence 
            memset( (byte*) pDSLockedBuffer + dwWavDataRead, (byte)(m_pWaveFile->mpwfx.Format.wBitsPerSample == 8 ? 128 : 0 ), dwDSLockedBufferSize - dwWavDataRead);
        }
    }

    // Unlock the buffer, we don't need it anymore.
    pDSB->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    return S_OK;
}

/*
===============
idAudioBufferWIN32::RestoreBuffer
Desc: Restores the lost buffer. *pbWasRestored returns true if the buffer was 
      restored.  It can also NULL if the information is not needed.
===============
*/
int idAudioBufferWIN32::RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, bool* pbWasRestored ) {
    int hr;

    if( pDSB == NULL ) {
        return -1;
	}
    if( pbWasRestored ) {
        *pbWasRestored = false;
	}

    ulong dwStatus;
    if( FAILED( hr = pDSB->GetStatus( &dwStatus ) ) ) {
        return DXTRACE_ERR( TEXT("GetStatus"), hr );
	}

    if( dwStatus & DSBSTATUS_BUFFERLOST ) {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
        do {
            hr = pDSB->Restore();
            if( hr == DSERR_BUFFERLOST ) {
                Sleep( 10 );
			}
			hr = pDSB->Restore();
        } while( hr );

        if( pbWasRestored != NULL ) {
            *pbWasRestored = true;
		}

        return S_OK;
    } else {
        return S_FALSE;
    }
}

/*
===============
idAudioBufferWIN32::Play
Desc: Plays the sound using voice management flags.  Pass in DSBPLAY_LOOPING
      in the dwFlags to loop the sound
===============
*/
int idAudioBufferWIN32::Play( dword dwPriority, dword dwFlags ) {
    int hr;
    bool    bRestored;

    if( m_apDSBuffer == NULL ) {
        return -1;
	}

    // Restore the buffer if it was lost
    if( FAILED( hr = RestoreBuffer( m_apDSBuffer, &bRestored ) ) ) {
        common->Error( TEXT("RestoreBuffer"), hr );
	}

    if( bRestored ) {
        // The buffer was restored, so we need to fill it with new data
        if( FAILED( hr = FillBufferWithSound( m_apDSBuffer, false ) ) ) {
            common->Error( TEXT("FillBufferWithSound"), hr );
		}

        // Make DirectSound do pre-processing on sound effects
        Reset();
    }

    m_apDSBuffer->Play( 0, dwPriority, dwFlags );
	return 0;
}

/*
===============
idAudioBufferWIN32::Stop
Desc: Stops the sound from playing
===============
*/
int idAudioBufferWIN32::Stop() {
    if( this == NULL || m_apDSBuffer == NULL ) {
        return -1;
	}

	m_apDSBuffer->Stop();

    return 0;
}

/*
===============
idAudioBufferWIN32::Reset
Desc: Reset all of the sound buffers
===============
*/
int idAudioBufferWIN32::Reset() {
    if( m_apDSBuffer == NULL ) {
        return -1;
	}

    m_apDSBuffer->SetCurrentPosition( 0 );

    return 0;
}

/*
===============
idAudioBufferWIN32::IsSoundPlaying
Desc: Checks to see if a buffer is playing and returns true if it
===============
*/
bool idAudioBufferWIN32::IsSoundPlaying( ) {
    if( m_apDSBuffer == NULL ) {
        return false;
	}

	if( m_apDSBuffer ) {  
		ulong dwStatus = 0;
        m_apDSBuffer->GetStatus( &dwStatus );
        if ( dwStatus & DSBSTATUS_PLAYING ) {
			return true;
		}
	}
	return false;
}

/*
===============
idAudioBufferWIN32::Lock
===============
*/
bool idAudioBufferWIN32::Lock( void **pDSLockedBuffer, ulong *dwDSLockedBufferSize ) {
	int hr;
    // Restore the buffer if it was lost
    bool bRestored;
    if( FAILED( hr = RestoreBuffer( m_apDSBuffer, &bRestored ) ) ) {
        return false;
	}

    // Lock the DirectSound buffer
    if( FAILED( hr = m_apDSBuffer->Lock( 0, m_dwDSBufferSize, pDSLockedBuffer, dwDSLockedBufferSize, NULL, NULL, 0 ) ) ) {
        return false;
	}
	return true;
}

/*
===============
idAudioBufferWIN32::Unlock
===============
*/
bool idAudioBufferWIN32::Unlock(void *pDSLockedBuffer, dword dwDSLockedBufferSize ) {
    // Unlock the DirectSound buffer
    m_apDSBuffer->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );
	return true;
}

/*
===============
idAudioBufferWIN32::GetCurrentPosition
===============
*/
bool idAudioBufferWIN32::GetCurrentPosition( ulong *pdwCurrentWriteCursor ) {
	int hr;

    // Make sure we have focus, and we didn't just switch in from
    // an app which had a DirectSound device
    if( FAILED( hr = RestoreBuffer( m_apDSBuffer, NULL ) ) ) {
        DXTRACE_ERR( TEXT("RestoreBuffer"), hr );
		return false;
	}

    if( FAILED( hr = m_apDSBuffer->GetCurrentPosition( NULL, pdwCurrentWriteCursor ) ) ) {
        return false;
	}
	return true;
}

/*
===============
idAudioBufferWIN32::SetVolume
===============
*/
void idAudioBufferWIN32::SetVolume( float x) {
	if (m_apDSBuffer) {
		m_apDSBuffer->SetVolume(x);
	}
}

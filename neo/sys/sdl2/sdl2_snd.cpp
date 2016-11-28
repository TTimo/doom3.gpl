#include "../../idlib/precompiled.h"
#pragma hdrstop

#define INITGUID
#include "../../sound/snd_local.h"
#include "SDL2/SDL.h"

idCVar s_device( "s_device", "-1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "Sound device to use. -1 for default device" );

bool Sys_LoadOpenAL( void )
{
	return false;
}

void Sys_FreeOpenAL( void )
{
}

class idAudioHardwareSDL2 : public idAudioHardware
{
	SDL_AudioSpec m_format;
public:
    idAudioHardwareSDL2() {}
    ~idAudioHardwareSDL2() {}

    bool Initialize( )
	{
		SDL_AudioSpec want;
		
		SDL_zero(want);
		want.freq = PRIMARYFREQ;
		want.format = AUDIO_F32;
		want.channels = 2;
		want.samples = 4096;
		want.callback = SDLAudioCallback;
		want.userdata = this;
		
		SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &want, &m_format, 0);
		if (dev == 0)
		{
			common->DPrintf("Failed to open audio: %s\n", SDL_GetError());
			return false;
		}
		else
		{
			if (m_format.format != want.format)  // we let this one thing change.
				common->DPrintf("We didn't get Float32 audio format.\n");
		}
		SDL_PauseAudioDevice(dev, 0);
		return true;
	}
	int	GetNumberOfSpeakers()
	{
		return m_format.channels;
	}

	// SDL does not support memory map API
	bool Lock( void **pDSLockedBuffer, ulong *dwDSLockedBufferSize )
	{ return false; }
	bool Unlock( void *pDSLockedBuffer, dword dwDSLockedBufferSize )
	{ return false; }
	bool GetCurrentPosition( ulong *pdwCurrentWriteCursor )
	{ return false; }
	int	GetMixBufferSize() { return 0; }
	bool Flush( void )
	{ return true; }
	void Write( bool ) {}
	short* GetMixBuffer( void )
	{ return NULL; }
private:
	static void SDLCALL SDLAudioCallback(void *userdata, Uint8 *stream, int len)
	{
		idAudioHardwareSDL2 *pThis = (idAudioHardwareSDL2 *)userdata;
				
		// I think we can calculate time delta as sizeof buffer / samples per second ?
		static float time = 0;
		float deltatime = (pThis->m_format.samples);
		time += deltatime;
		
		if( soundSystem->GetPlayingSoundWorld() )
		{
			// setup similar to async thread
			Sys_EnterCriticalSection();
			soundSystem->AsyncMix( (int)time, (float*)stream );
			Sys_LeaveCriticalSection();

			// doom mixes sound to -32768.0f 32768.0f range, scale down to -1.0f 1.0f
			SIMDProcessor->Mul( (float*)stream, 1.0f / 32768.0f, (float*)stream, MIXBUFFER_SAMPLES * 2 );
		}
		else
		{
			// set to silence
			memset( stream, pThis->m_format.silence, len );
		}
	}
};

idAudioHardware *idAudioHardware::Alloc() { return new idAudioHardwareSDL2; }
idAudioHardware::~idAudioHardware() {}

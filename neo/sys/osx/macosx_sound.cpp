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

#include <Carbon/Carbon.h>
#include <CoreAudio/CoreAudio.h>
#include <sys/sysctl.h>
   
idCVar s_device( "s_device", "-1", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "Sound device to use. -1 for default device" );

class idAudioHardwareOSX : public idAudioHardware {
public:
	idAudioHardwareOSX();
    ~idAudioHardwareOSX();

    bool	Initialize( );

	// OSX driver doesn't support memory map API
	bool	Lock( void **pDSLockedBuffer, ulong *dwDSLockedBufferSize ) { return false; }
	bool	Unlock( void *pDSLockedBuffer, dword dwDSLockedBufferSize ) { return false; }
	bool	GetCurrentPosition( ulong *pdwCurrentWriteCursor ) { return false; }
	int		GetMixBufferSize( void )  { return 0; }
	
	int		GetNumberOfSpeakers( void );

	// OSX driver doesn't support write API
	bool	Flush( void ) { return false; }
	void	Write( bool ) { }
	short*	GetMixBuffer( void ) { return NULL; }
	
private:
	AudioDeviceID		selectedDevice;
	bool				activeIOProc;
	AudioDeviceIOProcID activeIOProcID;

	void				Reset( void );
	void				InitFailed( void );
	const char*			ExtractStatus( OSStatus status );
	void				GetAvailableNominalSampleRates( void );

	// AudioDevicePropertyListenerProc
	static OSStatus		DeviceListener(	AudioDeviceID			inDevice,
	   									UInt32					inChannel,
										Boolean					isInput,
										AudioDevicePropertyID	inPropertyID,
										void*					inClientData );

	// AudioDeviceIOProc
	static OSStatus		DeviceIOProc( AudioDeviceID				inDevice,
									  const AudioTimeStamp*		inNow,
									  const AudioBufferList*	inInputData,
									  const AudioTimeStamp*		inInputTime,
									  AudioBufferList*			outOutputData, 
									  const AudioTimeStamp*		inOutputTime,
									  void*						inClientData );
};

/*
==========
iAudioHardware::Alloc
==========
*/
idAudioHardware *idAudioHardware::Alloc() { return new idAudioHardwareOSX; }

/*
==========
idAudioHardware::~idAudioHardware
==========
*/
idAudioHardware::~idAudioHardware() { }

/*
==========
idAudioHardwareOSX::idAudioHardwareOSX
==========
*/
idAudioHardwareOSX::idAudioHardwareOSX() {
	selectedDevice = kAudioDeviceUnknown;
	activeIOProc = false;
}

/*
==========
idAudioHardwareOSX::~idAudioHardwareOSX
==========
*/
idAudioHardwareOSX::~idAudioHardwareOSX() {
	Reset();
}

/*
==========
idAudioHardwareOSX::Reset
==========
*/
void idAudioHardwareOSX::Reset() {
	OSStatus status;

	if ( activeIOProc ) {
		status = AudioDeviceStop( selectedDevice, DeviceIOProc );
		if ( status != kAudioHardwareNoError ) {
			common->Warning( "idAudioHardwareOSX::Reset: AudioDeviceStop failed. status: %s", ExtractStatus( status ) );
		}
		status = AudioDeviceDestroyIOProcID( selectedDevice, activeIOProcID );
		if ( status != kAudioHardwareNoError ) {
			common->Warning( "idAudioHardwareOSX::Reset: AudioDeviceRemoveIOProc failed. status %s\n", ExtractStatus( status ) );
		}
		activeIOProc = false;
	}
	selectedDevice = kAudioDeviceUnknown;
	AudioHardwareUnload();
}

/*
=================
idAudioHardwareOSX::InitFailed
=================	
*/	
void idAudioHardwareOSX::InitFailed() {
	Reset();
	cvarSystem->SetCVarBool( "s_noSound", true );
	common->Warning( "sound subsystem disabled" );
	common->Printf( "------------------------------------------------\n" );
}

/*
==========
idAudioHardwareOSX::DeviceListener
==========
*/
OSStatus idAudioHardwareOSX::DeviceListener(	AudioDeviceID			inDevice,
												UInt32					inChannel,
												Boolean					isInput,
												AudioDevicePropertyID	inPropertyID,
												void*					inClientData) {
	common->Printf( "DeviceListener\n" );
	return kAudioHardwareNoError;
}

/*
==========
idAudioHardwareOSX::DeviceIOProc
==========
*/
OSStatus idAudioHardwareOSX::DeviceIOProc( AudioDeviceID			inDevice,
										   const AudioTimeStamp*	inNow,
										   const AudioBufferList*	inInputData,
										   const AudioTimeStamp*	inInputTime,
										   AudioBufferList*			outOutputData, 
										   const AudioTimeStamp*	inOutputTime,
										   void*					inClientData ) {

	// setup similar to async thread
	Sys_EnterCriticalSection();
	soundSystem->AsyncMix( (int)inOutputTime->mSampleTime, (float*)outOutputData->mBuffers[ 0 ].mData );
	Sys_LeaveCriticalSection();

	// doom mixes sound to -32768.0f 32768.0f range, scale down to -1.0f 1.0f
	SIMDProcessor->Mul( (Float32*)outOutputData->mBuffers[ 0 ].mData, 1.0f / 32768.0f, (Float32*)outOutputData->mBuffers[ 0 ].mData, MIXBUFFER_SAMPLES * 2 );

	return kAudioHardwareNoError;
}

/*
==========
idAudioHardwareOSX::ExtractStatus
==========
*/
union USwapBytes
{
	OSStatus m_status;
	char     m_char[sizeof(OSStatus)];
};

const char*	idAudioHardwareOSX::ExtractStatus( OSStatus status ) {
	static char buf[ sizeof( OSStatus ) + 1 ];
	USwapBytes sb;
	sb.m_status = status;
	for( int i = 0; i < sizeof(OSStatus); ++i )
		buf[i] = sb.m_char[sizeof(OSStatus)-1-i];
	buf[ sizeof( OSStatus ) ] = '\0';
	return buf;
}

/*
==========
idAudioHardwareOSX::Initialize
==========
*/
bool idAudioHardwareOSX::Initialize( ) {
	AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
	UInt32			size;
	OSStatus		status;
	int				i, deviceCount;
	AudioDeviceID	*deviceList;
	char			buf[ 1024 ];

	status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &size);
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioHardwareGetPropertyInfo kAudioHardwarePropertyDevices failed. status: %s", ExtractStatus( status ) );
		InitFailed();
		return false;
	}

	deviceCount = size / sizeof( AudioDeviceID );
	if ( !deviceCount ) {
		common->Printf( "No sound device found\n" );
		InitFailed();
		return false;
	}

	deviceList = (AudioDeviceID*)malloc( size );
	status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &size, deviceList);
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioHardwareGetProperty kAudioHardwarePropertyDevices failed. status: %s", ExtractStatus( status ) );
		free( deviceList );
		InitFailed();
		return false;
	}

	common->Printf( "%d sound device(s)\n", deviceCount );
	for( i = 0; i < deviceCount; i++ ) {
		size = sizeof(buf);
        propertyAddress.mSelector = kAudioDevicePropertyDeviceName;
        status = AudioObjectGetPropertyData(deviceList[i], &propertyAddress, 0, NULL, &size, &buf);
		if ( status != kAudioHardwareNoError ) {
			common->Warning( "AudioDeviceGetProperty kAudioDevicePropertyDeviceName %d failed. status: %s", i, ExtractStatus( status ) );
			free( deviceList );
			InitFailed();
			return false;
		}
		common->Printf( "  %d: ID %d, %s - ", i, deviceList[ i ], buf );
		size = sizeof(buf);
        propertyAddress.mSelector = kAudioDevicePropertyDeviceManufacturer;
        status = AudioObjectGetPropertyData(deviceList[i], &propertyAddress, 0, NULL, &size, &buf);
		if ( status != kAudioHardwareNoError ) {
			common->Warning( "AudioDeviceGetProperty kAudioDevicePropertyDeviceName %d failed. status: %s", i, ExtractStatus( status ) );
			free( deviceList );
			InitFailed();
			return false;
		}
		common->Printf( "%s\n", buf );
	}

	if ( s_device.GetInteger() != -1 && s_device.GetInteger() < deviceCount ) {
		selectedDevice = deviceList[ s_device.GetInteger() ];
		common->Printf( "s_device: device ID %d\n", selectedDevice );
	} else {
		size = sizeof( selectedDevice );
		propertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
        status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &size, &selectedDevice);
		if ( status != kAudioHardwareNoError ) {
			common->Warning( "AudioHardwareGetProperty kAudioHardwarePropertyDefaultOutputDevice failed. status: %s", ExtractStatus( status ) );
			
			free( deviceList );
			InitFailed();
			return false;
		}
		common->Printf( "select default device, ID %d\n", selectedDevice );
	}

	free( deviceList );
	deviceList = NULL;

	/*
	 // setup a listener to watch for changes to properties
	 status = AudioDeviceAddPropertyListener( selectedDevice, 0, false, kAudioDeviceProcessorOverload, DeviceListener, this );
	 if ( status != kAudioHardwareNoError ) {
	 common->Warning( "AudioDeviceAddPropertyListener kAudioDeviceProcessorOverload failed. status: %s", ExtractStatus( status ) );
	 InitFailed();
	 return;
	 }
	 */

	Float64 sampleRate;
	size = sizeof( sampleRate );
	propertyAddress.mSelector = kAudioDevicePropertyNominalSampleRate;
	status = AudioObjectGetPropertyData(selectedDevice, &propertyAddress, 0, NULL, &size, &sampleRate);
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioDeviceGetProperty %d kAudioDevicePropertyNominalSampleRate failed. status: %s", selectedDevice, ExtractStatus( status ) );
		InitFailed();
		return false;
	}
	common->Printf( "current nominal rate: %g\n", sampleRate );

	if ( sampleRate != PRIMARYFREQ ) {
		GetAvailableNominalSampleRates();

		sampleRate = PRIMARYFREQ;
		common->Printf( "setting rate to: %g\n", sampleRate );
		propertyAddress.mSelector = kAudioDevicePropertyNominalSampleRate;
		status = AudioObjectSetPropertyData( selectedDevice, &propertyAddress, 0, NULL, size, &sampleRate );
		if ( status != kAudioHardwareNoError ) {
			common->Warning( "AudioDeviceSetProperty %d kAudioDevicePropertyNominalSampleRate %g failed. status: %s", selectedDevice, sampleRate, ExtractStatus( status ) );
			InitFailed();
			return false;
		}
	}

	UInt32 frameSize;
	size = sizeof( UInt32 );
	propertyAddress.mSelector = kAudioDevicePropertyBufferFrameSize;
	status = AudioObjectGetPropertyData(selectedDevice, &propertyAddress, 0, NULL, &size, &frameSize);
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioDeviceGetProperty %d kAudioDevicePropertyBufferFrameSize failed.status: %s", selectedDevice, ExtractStatus( status ) );
		InitFailed();
		return false;
	}
	common->Printf( "current frame size: %d\n", frameSize );

	// get the allowed frame size range
	AudioValueRange frameSizeRange;
	size = sizeof( AudioValueRange );
	propertyAddress.mSelector = kAudioDevicePropertyBufferFrameSizeRange;
	status = AudioObjectGetPropertyData(selectedDevice, &propertyAddress, 0, NULL, &size, &frameSizeRange);
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioDeviceGetProperty %d kAudioDevicePropertyBufferFrameSizeRange failed. status: %s", selectedDevice, ExtractStatus( status ) );
		InitFailed();
		return false;
	}
	common->Printf( "frame size allowed range: %g %g\n", frameSizeRange.mMinimum, frameSizeRange.mMaximum );

	if ( frameSizeRange.mMaximum < MIXBUFFER_SAMPLES ) {
		common->Warning( "can't obtain the required frame size of %d bits", MIXBUFFER_SAMPLES );
		InitFailed();
		return false;
	}

	if ( frameSize != (unsigned int)MIXBUFFER_SAMPLES ) {
		frameSize = MIXBUFFER_SAMPLES;
		common->Printf( "setting frame size to: %d\n", frameSize );
		size = sizeof( frameSize );
		propertyAddress.mSelector = kAudioDevicePropertyBufferFrameSize;
		status = AudioObjectSetPropertyData( selectedDevice, &propertyAddress, 0, NULL, size, &frameSize );
		if ( status != kAudioHardwareNoError ) {
			common->Warning( "AudioDeviceSetProperty %d kAudioDevicePropertyBufferFrameSize failed. status: %s", selectedDevice, ExtractStatus( status ) );
			InitFailed();
			return false;
		}
	}

	if ( idSoundSystemLocal::s_numberOfSpeakers.GetInteger() != 2 ) {
		common->Warning( "only stereo sound currently supported" );
		idSoundSystemLocal::s_numberOfSpeakers.SetInteger( 2 );
	}
	UInt32 channels[ 2 ];
	size = 2 * sizeof( UInt32 );

	// I set the scope to output here, be careful if reusing
	propertyAddress.mSelector = kAudioDevicePropertyPreferredChannelsForStereo;
	propertyAddress.mScope = kAudioDevicePropertyScopeOutput;
	status = AudioObjectGetPropertyData(selectedDevice, &propertyAddress, 0, NULL, &size, &channels);
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioDeviceGetProperty %d kAudioDevicePropertyPreferredChannelsForStereo failed. status: %s", selectedDevice, ExtractStatus( status ) );
		InitFailed();
		return false;
	}
	common->Printf( "using stereo channel IDs %d %d\n", channels[ 0 ], channels[ 1 ] );

	status = AudioDeviceCreateIOProcID( selectedDevice, DeviceIOProc, NULL, &activeIOProcID );
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioDeviceAddIOProc failed. status: %s", ExtractStatus( status ) );
		InitFailed();
		return false;
	}
	activeIOProc = true;

	status = AudioDeviceStart( selectedDevice, DeviceIOProc );
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioDeviceStart failed. status: %s", ExtractStatus( status ) );
		InitFailed();
		return false;
	}

	/*
	 // allocate the mix buffer
	 // it has the space for ROOM_SLICES_IN_BUFFER DeviceIOProc loops
	 mixBufferSize =  dwSpeakers * dwSampleSize * dwPrimaryBitRate * ROOM_SLICES_IN_BUFFER / 8;
	 mixBuffer = malloc( mixBufferSize );
	 memset( mixBuffer, 0, mixBufferSize );
	 */

	return true;
}

/*
==========
idAudioHardwareOSX::GetAvailableNominalSampleRates
==========
*/
void idAudioHardwareOSX::GetAvailableNominalSampleRates( void ) {
	AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };
	UInt32				size;
	OSStatus			status;
	int			   		i, rangeCount;
	AudioValueRange		*rangeArray;

	propertyAddress.mSelector = kAudioDevicePropertyAvailableNominalSampleRates;
	status = AudioObjectGetPropertyData(selectedDevice, &propertyAddress, 0, NULL, &size, NULL);
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioDeviceGetPropertyInfo %d kAudioDevicePropertyAvailableNominalSampleRates failed. status: %s", selectedDevice, ExtractStatus( status ) );
		return;
	}
	rangeCount = size / sizeof( AudioValueRange );
	rangeArray = (AudioValueRange *)malloc( size );

	common->Printf( "%d possible rate(s)\n", rangeCount );

	propertyAddress.mSelector = kAudioDevicePropertyAvailableNominalSampleRates;
	status = AudioObjectGetPropertyData(selectedDevice, &propertyAddress, 0, NULL, &size, rangeArray);
	if ( status != kAudioHardwareNoError ) {
		common->Warning( "AudioDeviceGetProperty %d kAudioDevicePropertyAvailableNominalSampleRates failed. status: %s", selectedDevice, ExtractStatus( status ) );
		free( rangeArray );
		return;
	}

	for( i = 0; i < rangeCount; i++ ) {
		common->Printf( "  %d: min %g max %g\n", i, rangeArray[ i ].mMinimum, rangeArray[ i ].mMaximum );
	}

	free( rangeArray );
}

/*
==========
idAudioHardwareOSX::GetNumberOfSpeakers
==========
*/
int	idAudioHardwareOSX::GetNumberOfSpeakers() {
	return idSoundSystemLocal::s_numberOfSpeakers.GetInteger();
}

/*
 ===============
 Sys_LoadOpenAL
 ===============
 */
bool Sys_LoadOpenAL( void ) {
	static const int kMountainLionVersionMajor = 12;
    size_t len;
	int mib[] = {CTL_KERN, KERN_OSRELEASE};
    sysctl(mib, sizeof mib / sizeof(int), NULL, &len, NULL, 0);
	
    char* kernelVersion = (char *)_alloca(len);
    sysctl(mib, sizeof mib / sizeof(int), kernelVersion, &len, NULL, 0);
	
	// to translate OSRELEASE into OSX version number, subtract 4 from the major
	// version number and then prepend a 10.  So Mountain Lion which returns 12.3.0 becomes
	// 10.8.3.0
	int version = 0;
	char *dot = strtok( kernelVersion, ".");
	if( dot )
		version = atoi( kernelVersion );

	if( version < kMountainLionVersionMajor )
		return false;
	return true;
}

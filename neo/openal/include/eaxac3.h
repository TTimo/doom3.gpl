/************************************************************************************************
/
/ EAX-AC3 Open AL Extension Header file
/
/ Description : The EAX-AC3 extension to Open AL provides a way to playback Dolby Digital AC3
/				files on systems equipped with a SB Live! card.  The packaged AC3 data is output
/				via the MMSYSTEM Wave device.
/				If a SB Live! 5.1 card is installed then the AC3 data will be decoded by the
/				audio card.
/				If a legacy SB Live! card is installed then the AC3 data will be sent directly
/				to the S/PDIF Out.
/				The API supports multiple EAX-AC3 devices, and multiple AC3 streams. However
/				the current implementation provides one EAX-AC3 device capable of playing
/				one AC3 Stream at a time.
/
/ Programmer  : Daniel Peacock		Creative Labs, Inc	February 2001
/
/************************************************************************************************/

#ifndef _EAXAC3_H_
#define _EAXAC3_H_

// Do not define the symbol EAXAC3_EXPORTS in any projects that use the EAX-AC3 Open AL Extension
#ifdef EAXAC3_EXPORTS
#define EAXAC3_API __declspec(dllexport)
#else
#define EAXAC3_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef signed long HRESULT;
#endif

enum POSFORMAT { MILLISECONDS, BYTES, AC3FRAMES };

enum SOURCE { AC3FILE, MEMORY };

// Success Codes
#define EAXAC3_OK										0
#define EAXAC3_ALREADYPLAYING							1
#define EAXAC3_EOF										2

// Error Codes
#define EAXAC3ERR_UNABLETOOPENEAXAC3DEVICE				 -1
#define EAXAC3ERR_WAVEOUTPREPAREHEADERFAILURE			 -2
#define EAXAC3ERR_OUTOFMEMORY							 -3
#define	EAXAC3ERR_FILENOTFOUND							 -4
#define EAXAC3ERR_AC3FILETOBIG							 -5
#define EAXAC3ERR_AC3FRAMENOTFOUND						 -6
#define EAXAC3ERR_AC3NOTAT48KHZ							 -7
#define EAXAC3ERR_INVALIDAC3FRAME						 -8
#define EAXAC3ERR_AC3FILENOTOPEN						 -9
#define EAXAC3ERR_BUFFERNOTMULTIPLEOFAC3FRAMESIZE		-10
#define EAXAC3ERR_WAVEOUTERROR							-11
#define EAXAC3ERR_FAILEDTOCREATEEVENT					-12
#define EAXAC3ERR_EAXAC3DEVICENOTOPEN					-13
#define EAXAC3ERR_AC3STREAMALREADYOPEN					-14
#define EAXAC3ERR_POSITIONOUTOFRANGE					-15
#define EAXAC3ERR_NOTATSTARTOFAC3FRAME					-16
#define EAXAC3ERR_AC3STREAMNOTOPEN						-17
#define EAXAC3ERR_SETPOSITIONONLYWORKSONAC3FILES		-18
#define EAXAC3ERR_WRITEDATAONLYWORKSWITHMEMORYSTREAMS	-19
#define EAXAC3ERR_INVALIDPARAMETER						-20
#define EAXAC3ERR_NOTENOUGHAC3DATAINAC3DATABUFFER		-21
#define EAXAC3ERR_NOTENOUGHDATA							-22
#define EAXAC3ERR_EAXAC3DEVICEALREADYOPEN				-23
#define EAXAC3ERR_EAXAC3DEVICENOTFOUND					-24
#define EAXAC3ERR_UNSUPPORTED							-25
#define EAXAC3ERR_FAILEDTOCREATEFNTABLE					-26

#define DEFAULTEAXAC3DEVICE	0

#define ENTIREBUFFER	0
#define FROMWRITECURSOR	1

#define LOOPING		1

#define ENDOFDATA	1

typedef unsigned int EAXAC3HANDLE;

typedef unsigned int AC3STREAM;

// Callback function
typedef void (__stdcall *LPAC3CALLBACK)(AC3STREAM AC3Stream, int msg);

// Callback messages
#define EAXAC3NEEDMOREDATA	0
#define EAXAC3REACHEDEND	1

typedef struct
{
	unsigned int nNumOfAC3Frames;
	unsigned int nAC3FrameSize;
	unsigned int nSizeOfFile;
	unsigned int nDuration;
	unsigned int nFrequency;
} AC3FILEINFO, *LPAC3FILEINFO;

#define UNKNOWN			1
#define SPDIFPASSTHRU	2
#define FULLDECODE		4

typedef struct
{
	char szDeviceName[256];
	unsigned int uFlags;
	unsigned int uStreams;
	unsigned int uReserved;
} EAXAC3DEVICEINFO, *LPEAXAC3DEVICEINFO;

// Function typedefs

typedef int		(*LPEAXAC3QUERYNUMBEROFDEVICES)		(void);
typedef HRESULT (*LPEAXAC3QUERYFILE)				(char *, LPAC3FILEINFO, int);
typedef HRESULT (*LPEAXAC3QUERYMEMORY)				(char *, int, LPAC3FILEINFO, int);
typedef int		(*LPEAXAC3QUERYNOOFFRAMESREQFORPLAYBACK)	(AC3STREAM);
typedef HRESULT	(*LPEAXAC3OPENPLAYBACKDEVICE)		(EAXAC3HANDLE);
typedef HRESULT	(*LPEAXAC3CLOSEPLAYBACKDEVICE)		(EAXAC3HANDLE);
typedef HRESULT	(*LPEAXAC3QUERYDEVICECAPS)			(EAXAC3HANDLE, LPEAXAC3DEVICEINFO, int);
typedef HRESULT	(*LPEAXAC3GETPOSITION)				(AC3STREAM, enum POSFORMAT, int *);
typedef HRESULT (*LPEAXAC3SETFILEPOSITION)			(AC3STREAM, enum POSFORMAT, int);
typedef HRESULT (*LPEAXAC3OPENSTREAM)				(EAXAC3HANDLE, AC3STREAM *, LPAC3CALLBACK, char *, enum SOURCE);
typedef HRESULT (*LPEAXAC3CLOSESTREAM)				(AC3STREAM);
typedef HRESULT (*LPEAXAC3PREPLAYSTREAM)			(AC3STREAM);
typedef HRESULT (*LPEAXAC3PLAYSTREAM)				(AC3STREAM, int);
typedef HRESULT (*LPEAXAC3STOPSTREAM)				(AC3STREAM);
typedef HRESULT	(*LPEAXAC3PAUSESTREAM)				(AC3STREAM);
typedef HRESULT	(*LPEAXAC3RESUMESTREAM)				(AC3STREAM);
typedef HRESULT (*LPEAXAC3LOCKBUFFER)				(AC3STREAM, unsigned long, void **, unsigned long *, void **,
														unsigned long *, unsigned long);
typedef HRESULT (*LPEAXAC3UNLOCKBUFFER)				(AC3STREAM, void *, unsigned long, void *, unsigned long, int);
typedef HRESULT	(*LPEAXAC3SETPLAYBACKMODE)			(EAXAC3HANDLE, unsigned int);
typedef char *	(*LPEAXAC3GETERRORSTRING)			(HRESULT, char *, int);
typedef HRESULT (*LPEAXAC3GETLASTERROR)				(HRESULT *);

// Function table declaration
typedef struct 
{
	LPEAXAC3QUERYNUMBEROFDEVICES					EAXAC3QueryNumberOfDevices;
	LPEAXAC3QUERYFILE								EAXAC3QueryFile;
	LPEAXAC3QUERYMEMORY								EAXAC3QueryMemory;
	LPEAXAC3QUERYNOOFFRAMESREQFORPLAYBACK			EAXAC3QueryNoOfFramesReqForPlayback;
	LPEAXAC3OPENPLAYBACKDEVICE						EAXAC3OpenPlaybackDevice;
	LPEAXAC3CLOSEPLAYBACKDEVICE						EAXAC3ClosePlaybackDevice;
	LPEAXAC3QUERYDEVICECAPS							EAXAC3QueryDeviceCaps;
	LPEAXAC3GETPOSITION								EAXAC3GetPosition;
	LPEAXAC3SETFILEPOSITION							EAXAC3SetFilePosition;
	LPEAXAC3OPENSTREAM								EAXAC3OpenStream;
	LPEAXAC3CLOSESTREAM								EAXAC3CloseStream;
	LPEAXAC3PREPLAYSTREAM							EAXAC3PrePlayStream;
	LPEAXAC3PLAYSTREAM								EAXAC3PlayStream;
	LPEAXAC3STOPSTREAM								EAXAC3StopStream;
	LPEAXAC3PAUSESTREAM								EAXAC3PauseStream;
	LPEAXAC3RESUMESTREAM							EAXAC3ResumeStream;
	LPEAXAC3LOCKBUFFER								EAXAC3LockBuffer;
	LPEAXAC3UNLOCKBUFFER							EAXAC3UnLockBuffer;
	LPEAXAC3SETPLAYBACKMODE							EAXAC3SetPlaybackMode;
	LPEAXAC3GETERRORSTRING							EAXAC3GetErrorString;
	LPEAXAC3GETLASTERROR							EAXAC3GetLastError;
} EAXAC3FNTABLE, *LPEAXAC3FNTABLE;


#ifndef OPENAL
typedef EAXAC3_API HRESULT (*LPEAXAC3GETFUNCTIONTABLE)		(LPEAXAC3FNTABLE);
#else
typedef ALboolean (*LPALEAXAC3GETFUNCTIONTABLE) (LPEAXAC3FNTABLE);
#endif

// Functions exposed in the DLL

EAXAC3_API HRESULT EAXAC3GetFunctionTable(LPEAXAC3FNTABLE lpEAXAC3FnTable);

EAXAC3_API int	   EAXAC3QueryNumberOfDevices();

EAXAC3_API HRESULT EAXAC3QueryFile(char *szAC3Filename, LPAC3FILEINFO lpAC3Caps, int nSizeOfAC3FileInfoStruct);

EAXAC3_API HRESULT EAXAC3QueryMemory(char *lpBuffer, int nSizeOfBuffer, LPAC3FILEINFO lpAC3FileInfo,
									 int nSizeOfAC3FileInfoStruct);

EAXAC3_API int     EAXAC3QueryNoOfFramesReqForPlayback(AC3STREAM AC3Stream);

EAXAC3_API HRESULT EAXAC3OpenPlaybackDevice(EAXAC3HANDLE EAXAC3Handle);

EAXAC3_API HRESULT EAXAC3ClosePlaybackDevice(EAXAC3HANDLE EAXAC3Handle);

EAXAC3_API HRESULT EAXAC3QueryDeviceCaps(EAXAC3HANDLE EAXAC3Handle, LPEAXAC3DEVICEINFO lpEAXAC3DeviceInfo,
										 int nSizeOfAC3DeviceInfoStruct);

EAXAC3_API HRESULT EAXAC3GetPosition(AC3STREAM AC3Stream, enum POSFORMAT posFormat, int *lpAmount);

EAXAC3_API HRESULT EAXAC3SetFilePosition(AC3STREAM AC3Stream, enum POSFORMAT posFormat, int nAmount);

EAXAC3_API HRESULT EAXAC3OpenStream(EAXAC3HANDLE EAXAC3Handle, AC3STREAM *lpAC3Stream,
									LPAC3CALLBACK pAC3CallbackFn, char *szAC3Filename, enum SOURCE src);

EAXAC3_API HRESULT EAXAC3CloseStream(AC3STREAM AC3Stream);

EAXAC3_API HRESULT EAXAC3PrePlayStream(AC3STREAM AC3Stream);

EAXAC3_API HRESULT EAXAC3PlayStream(AC3STREAM AC3Stream, int nLooping);

EAXAC3_API HRESULT EAXAC3StopStream(AC3STREAM AC3Stream);

EAXAC3_API HRESULT EAXAC3PauseStream(AC3STREAM AC3Stream);

EAXAC3_API HRESULT EAXAC3ResumeStream(AC3STREAM AC3Stream);

EAXAC3_API HRESULT EAXAC3LockBuffer(AC3STREAM AC3Stream, unsigned long ulBytes, void **ppvPointer1,
									unsigned long *pdwBytes1, void **ppvPointer2, unsigned long *pdwBytes2,
									unsigned long ulFlags);

EAXAC3_API HRESULT EAXAC3UnLockBuffer(AC3STREAM AC3Stream, void *pvPointer1, unsigned long ulSize1,
									  void *pvPointer2, unsigned long ulSize2, int nFinished);

EAXAC3_API HRESULT EAXAC3SetPlaybackMode(EAXAC3HANDLE EAXAC3Handle, unsigned int ulPlayMode);

EAXAC3_API char *  EAXAC3GetErrorString(HRESULT hr, char *szErrorString, int nSizeOfErrorString);

EAXAC3_API HRESULT EAXAC3GetLastError(HRESULT *hr);

#ifdef __cplusplus
}
#endif

#endif


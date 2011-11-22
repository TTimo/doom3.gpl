#ifndef _ALC_H_
#define _ALC_H_

#include "altypes.h"
#include "alctypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
 #ifdef _OPENAL32LIB
  #define ALCAPI __declspec(dllexport)
 #else
  #define ALCAPI __declspec(dllimport)
 #endif

 typedef struct ALCdevice_struct ALCdevice;
 typedef struct ALCcontext_struct ALCcontext;

 #define ALCAPIENTRY __cdecl
#else
 #ifdef TARGET_OS_MAC
  #if TARGET_OS_MAC
   #pragma export on
  #endif
 #endif
 #define ALCAPI
 #define ALCAPIENTRY __cdecl
#endif



#ifndef ALC_NO_PROTOTYPES

ALCAPI ALCubyte*  ALCAPIENTRY alcGetString(ALCdevice *device,ALCenum param);
ALCAPI ALCvoid    ALCAPIENTRY alcGetIntegerv(ALCdevice *device,ALCenum param,ALCsizei size,ALCint *data);

ALCAPI ALCdevice* ALCAPIENTRY alcOpenDevice(ALCubyte *deviceName);
ALCAPI ALCvoid    ALCAPIENTRY alcCloseDevice(ALCdevice *device);

ALCAPI ALCcontext*ALCAPIENTRY alcCreateContext(ALCdevice *device,ALCint *attrList);
ALCAPI ALCboolean ALCAPIENTRY alcMakeContextCurrent(ALCcontext *context);
ALCAPI ALCvoid	  ALCAPIENTRY alcProcessContext(ALCcontext *context);
ALCAPI ALCcontext*ALCAPIENTRY alcGetCurrentContext(ALCvoid);
ALCAPI ALCdevice* ALCAPIENTRY alcGetContextsDevice(ALCcontext *context);
ALCAPI ALCvoid	  ALCAPIENTRY alcSuspendContext(ALCcontext *context);
ALCAPI ALCvoid    ALCAPIENTRY alcDestroyContext(ALCcontext *context);

ALCAPI ALCenum	  ALCAPIENTRY alcGetError(ALCdevice *device);

ALCAPI ALCboolean ALCAPIENTRY alcIsExtensionPresent(ALCdevice *device,ALCubyte *extName);
ALCAPI ALCvoid *  ALCAPIENTRY alcGetProcAddress(ALCdevice *device,ALCubyte *funcName);
ALCAPI ALCenum	  ALCAPIENTRY alcGetEnumValue(ALCdevice *device,ALCubyte *enumName);
				
#else /* ALC_NO_PROTOTYPES */

ALCAPI ALCubyte*  ALCAPIENTRY (*alcGetString)(ALCdevice *device,ALCenum param);
ALCAPI ALCvoid    ALCAPIENTRY (*alcGetIntegerv)(ALCdevice * device,ALCenum param,ALCsizei size,ALCint *data);

ALCAPI ALCdevice* ALCAPIENTRY (*alcOpenDevice)(ALubyte *deviceName);
ALCAPI ALCvoid    ALCAPIENTRY (*alcCloseDevice)(ALCdevice *device);

ALCAPI ALCcontext*ALCAPIENTRY (*alcCreateContext)(ALCdevice *device,ALCint *attrList);
ALCAPI ALCboolean ALCAPIENTRY (*alcMakeContextCurrent)(ALCcontext *context);
ALCAPI ALCvoid	  ALCAPIENTRY (*alcProcessContext)(ALCcontext *context);
ALCAPI ALCcontext*ALCAPIENTRY (*alcGetCurrentContext)(ALCvoid);
ALCAPI ALCdevice* ALCAPIENTRY (*alcGetContextsDevice)(ALCcontext *context);
ALCAPI ALCvoid	  ALCAPIENTRY (*alcSuspendContext)(ALCcontext *context);
ALCAPI ALCvoid    ALCAPIENTRY (*alcDestroyContext)(ALCcontext *context);

ALCAPI ALCenum	  ALCAPIENTRY (*alcGetError)(ALCdevice *device);

ALCAPI ALCboolean ALCAPIENTRY (*alcIsExtensionPresent)(ALCdevice *device,ALCubyte *extName);
ALCAPI ALCvoid *  ALCAPIENTRY (*alcGetProcAddress)(ALCdevice *device,ALCubyte *funcName);
ALCAPI ALCenum	  ALCAPIENTRY (*alcGetEnumValue)(ALCdevice *device,ALCubyte *enumName);

#endif /* AL_NO_PROTOTYPES */

#ifdef TARGET_OS_MAC
 #if TARGET_OS_MAC
  #pragma export off
 #endif
#endif

#ifdef __cplusplus
}
#endif

#endif


// generated header. do not edit
// C:\Python23\Lib\idlelib\idle.pyw
// Mon Mar 28 12:31:26 2005

ALenum ( ALAPIENTRY * idalGetError )( ALvoid ) = NULL;
ALvoid ( ALAPIENTRY * idalGenBuffers )( ALsizei, ALuint * ) = NULL;
ALboolean ( ALAPIENTRY * idalIsSource )( ALuint ) = NULL;
ALvoid ( ALAPIENTRY * idalSourceStop )( ALuint ) = NULL;
ALvoid ( ALAPIENTRY * idalGetSourcei )( ALuint, ALenum, ALint * ) = NULL;
ALint ( ALAPIENTRY * idalGetInteger )( ALenum ) = NULL;
ALCvoid ( ALAPIENTRY * idalcSuspendContext )( ALCcontext * ) = NULL;
ALCboolean ( ALAPIENTRY * idalcMakeContextCurrent )( ALCcontext * ) = NULL;
ALCvoid ( ALAPIENTRY * idalcProcessContext )( ALCcontext * ) = NULL;
ALCvoid ( ALAPIENTRY * idalcDestroyContext )( ALCcontext * ) = NULL;
ALCubyte * ( ALAPIENTRY * idalcGetString )( ALCdevice *, ALCenum ) = NULL;
ALvoid ( ALAPIENTRY * idalBufferData )( ALuint, ALenum, ALvoid *, ALsizei, ALsizei ) = NULL;
ALvoid ( ALAPIENTRY * idalDeleteBuffers )( ALsizei, ALuint * ) = NULL;
ALboolean ( ALAPIENTRY * idalIsExtensionPresent )( ALubyte * ) = NULL;
ALvoid ( ALAPIENTRY * idalDeleteSources )( ALsizei, ALuint * ) = NULL;
ALenum ( ALAPIENTRY * idalGetEnumValue )( ALubyte * ) = NULL;
ALvoid * ( ALAPIENTRY * idalGetProcAddress )( ALubyte * ) = NULL;
ALCcontext * ( ALAPIENTRY * idalcCreateContext )( ALCdevice *, ALCint * ) = NULL;
ALCdevice * ( ALAPIENTRY * idalcOpenDevice )( ALubyte * ) = NULL;
ALvoid ( ALAPIENTRY * idalListenerfv )( ALenum, ALfloat* ) = NULL;
ALvoid ( ALAPIENTRY * idalSourceQueueBuffers )( ALuint, ALsizei, ALuint * ) = NULL;
ALvoid ( ALAPIENTRY * idalSourcei )( ALuint, ALenum, ALint ) = NULL;
ALvoid ( ALAPIENTRY * idalListenerf )( ALenum, ALfloat ) = NULL;
ALCvoid ( ALAPIENTRY * idalcCloseDevice )( ALCdevice * ) = NULL;
ALboolean ( ALAPIENTRY * idalIsBuffer )( ALuint ) = NULL;
ALvoid ( ALAPIENTRY * idalSource3f )( ALuint, ALenum, ALfloat, ALfloat, ALfloat ) = NULL;
ALvoid ( ALAPIENTRY * idalGenSources )( ALsizei, ALuint * ) = NULL;
ALvoid ( ALAPIENTRY * idalSourcef )( ALuint, ALenum, ALfloat ) = NULL;
ALvoid ( ALAPIENTRY * idalSourceUnqueueBuffers )( ALuint, ALsizei, ALuint * ) = NULL;
ALvoid ( ALAPIENTRY * idalSourcePlay )( ALuint ) = NULL;

const char* InitializeIDAL( HMODULE h ) {
idalGetError = ( ALenum ( ALAPIENTRY * ) ( ALvoid ) )GetProcAddress( h, "alGetError" );
if ( !idalGetError) {
  return "alGetError";
}
idalGenBuffers = ( ALvoid ( ALAPIENTRY * ) ( ALsizei, ALuint * ) )GetProcAddress( h, "alGenBuffers" );
if ( !idalGenBuffers) {
  return "alGenBuffers";
}
idalIsSource = ( ALboolean ( ALAPIENTRY * ) ( ALuint ) )GetProcAddress( h, "alIsSource" );
if ( !idalIsSource) {
  return "alIsSource";
}
idalSourceStop = ( ALvoid ( ALAPIENTRY * ) ( ALuint ) )GetProcAddress( h, "alSourceStop" );
if ( !idalSourceStop) {
  return "alSourceStop";
}
idalGetSourcei = ( ALvoid ( ALAPIENTRY * ) ( ALuint, ALenum, ALint * ) )GetProcAddress( h, "alGetSourcei" );
if ( !idalGetSourcei) {
  return "alGetSourcei";
}
idalGetInteger = ( ALint ( ALAPIENTRY * ) ( ALenum ) )GetProcAddress( h, "alGetInteger" );
if ( !idalGetInteger) {
  return "alGetInteger";
}
idalcSuspendContext = ( ALCvoid ( ALAPIENTRY * ) ( ALCcontext * ) )GetProcAddress( h, "alcSuspendContext" );
if ( !idalcSuspendContext) {
  return "alcSuspendContext";
}
idalcMakeContextCurrent = ( ALCboolean ( ALAPIENTRY * ) ( ALCcontext * ) )GetProcAddress( h, "alcMakeContextCurrent" );
if ( !idalcMakeContextCurrent) {
  return "alcMakeContextCurrent";
}
idalcProcessContext = ( ALCvoid ( ALAPIENTRY * ) ( ALCcontext * ) )GetProcAddress( h, "alcProcessContext" );
if ( !idalcProcessContext) {
  return "alcProcessContext";
}
idalcDestroyContext = ( ALCvoid ( ALAPIENTRY * ) ( ALCcontext * ) )GetProcAddress( h, "alcDestroyContext" );
if ( !idalcDestroyContext) {
  return "alcDestroyContext";
}
idalcGetString = ( ALCubyte * ( ALAPIENTRY * ) ( ALCdevice *, ALCenum ) )GetProcAddress( h, "alcGetString" );
if ( !idalcGetString) {
  return "alcGetString";
}
idalBufferData = ( ALvoid ( ALAPIENTRY * ) ( ALuint, ALenum, ALvoid *, ALsizei, ALsizei ) )GetProcAddress( h, "alBufferData" );
if ( !idalBufferData) {
  return "alBufferData";
}
idalDeleteBuffers = ( ALvoid ( ALAPIENTRY * ) ( ALsizei, ALuint * ) )GetProcAddress( h, "alDeleteBuffers" );
if ( !idalDeleteBuffers) {
  return "alDeleteBuffers";
}
idalIsExtensionPresent = ( ALboolean ( ALAPIENTRY * ) ( ALubyte * ) )GetProcAddress( h, "alIsExtensionPresent" );
if ( !idalIsExtensionPresent) {
  return "alIsExtensionPresent";
}
idalDeleteSources = ( ALvoid ( ALAPIENTRY * ) ( ALsizei, ALuint * ) )GetProcAddress( h, "alDeleteSources" );
if ( !idalDeleteSources) {
  return "alDeleteSources";
}
idalGetEnumValue = ( ALenum ( ALAPIENTRY * ) ( ALubyte * ) )GetProcAddress( h, "alGetEnumValue" );
if ( !idalGetEnumValue) {
  return "alGetEnumValue";
}
idalGetProcAddress = ( ALvoid * ( ALAPIENTRY * ) ( ALubyte * ) )GetProcAddress( h, "alGetProcAddress" );
if ( !idalGetProcAddress) {
  return "alGetProcAddress";
}
idalcCreateContext = ( ALCcontext * ( ALAPIENTRY * ) ( ALCdevice *, ALCint * ) )GetProcAddress( h, "alcCreateContext" );
if ( !idalcCreateContext) {
  return "alcCreateContext";
}
idalcOpenDevice = ( ALCdevice * ( ALAPIENTRY * ) ( ALubyte * ) )GetProcAddress( h, "alcOpenDevice" );
if ( !idalcOpenDevice) {
  return "alcOpenDevice";
}
idalListenerfv = ( ALvoid ( ALAPIENTRY * ) ( ALenum, ALfloat* ) )GetProcAddress( h, "alListenerfv" );
if ( !idalListenerfv) {
  return "alListenerfv";
}
idalSourceQueueBuffers = ( ALvoid ( ALAPIENTRY * ) ( ALuint, ALsizei, ALuint * ) )GetProcAddress( h, "alSourceQueueBuffers" );
if ( !idalSourceQueueBuffers) {
  return "alSourceQueueBuffers";
}
idalSourcei = ( ALvoid ( ALAPIENTRY * ) ( ALuint, ALenum, ALint ) )GetProcAddress( h, "alSourcei" );
if ( !idalSourcei) {
  return "alSourcei";
}
idalListenerf = ( ALvoid ( ALAPIENTRY * ) ( ALenum, ALfloat ) )GetProcAddress( h, "alListenerf" );
if ( !idalListenerf) {
  return "alListenerf";
}
idalcCloseDevice = ( ALCvoid ( ALAPIENTRY * ) ( ALCdevice * ) )GetProcAddress( h, "alcCloseDevice" );
if ( !idalcCloseDevice) {
  return "alcCloseDevice";
}
idalIsBuffer = ( ALboolean ( ALAPIENTRY * ) ( ALuint ) )GetProcAddress( h, "alIsBuffer" );
if ( !idalIsBuffer) {
  return "alIsBuffer";
}
idalSource3f = ( ALvoid ( ALAPIENTRY * ) ( ALuint, ALenum, ALfloat, ALfloat, ALfloat ) )GetProcAddress( h, "alSource3f" );
if ( !idalSource3f) {
  return "alSource3f";
}
idalGenSources = ( ALvoid ( ALAPIENTRY * ) ( ALsizei, ALuint * ) )GetProcAddress( h, "alGenSources" );
if ( !idalGenSources) {
  return "alGenSources";
}
idalSourcef = ( ALvoid ( ALAPIENTRY * ) ( ALuint, ALenum, ALfloat ) )GetProcAddress( h, "alSourcef" );
if ( !idalSourcef) {
  return "alSourcef";
}
idalSourceUnqueueBuffers = ( ALvoid ( ALAPIENTRY * ) ( ALuint, ALsizei, ALuint * ) )GetProcAddress( h, "alSourceUnqueueBuffers" );
if ( !idalSourceUnqueueBuffers) {
  return "alSourceUnqueueBuffers";
}
idalSourcePlay = ( ALvoid ( ALAPIENTRY * ) ( ALuint ) )GetProcAddress( h, "alSourcePlay" );
if ( !idalSourcePlay) {
  return "alSourcePlay";
}
return NULL;
};

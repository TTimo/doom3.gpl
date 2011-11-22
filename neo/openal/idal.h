// generated header. do not edit
// C:\Python23\Lib\idlelib\idle.pyw
// Mon Mar 28 12:31:26 2005

extern ALenum ( ALAPIENTRY * idalGetError )( ALvoid );
extern ALvoid ( ALAPIENTRY * idalGenBuffers )( ALsizei, ALuint * );
extern ALboolean ( ALAPIENTRY * idalIsSource )( ALuint );
extern ALvoid ( ALAPIENTRY * idalSourceStop )( ALuint );
extern ALvoid ( ALAPIENTRY * idalGetSourcei )( ALuint, ALenum, ALint * );
extern ALint ( ALAPIENTRY * idalGetInteger )( ALenum );
extern ALCvoid ( ALAPIENTRY * idalcSuspendContext )( ALCcontext * );
extern ALCboolean ( ALAPIENTRY * idalcMakeContextCurrent )( ALCcontext * );
extern ALCvoid ( ALAPIENTRY * idalcProcessContext )( ALCcontext * );
extern ALCvoid ( ALAPIENTRY * idalcDestroyContext )( ALCcontext * );
extern ALCubyte * ( ALAPIENTRY * idalcGetString )( ALCdevice *, ALCenum );
extern ALvoid ( ALAPIENTRY * idalBufferData )( ALuint, ALenum, ALvoid *, ALsizei, ALsizei );
extern ALvoid ( ALAPIENTRY * idalDeleteBuffers )( ALsizei, ALuint * );
extern ALboolean ( ALAPIENTRY * idalIsExtensionPresent )( ALubyte * );
extern ALvoid ( ALAPIENTRY * idalDeleteSources )( ALsizei, ALuint * );
extern ALenum ( ALAPIENTRY * idalGetEnumValue )( ALubyte * );
extern ALvoid * ( ALAPIENTRY * idalGetProcAddress )( ALubyte * );
extern ALCcontext * ( ALAPIENTRY * idalcCreateContext )( ALCdevice *, ALCint * );
extern ALCdevice * ( ALAPIENTRY * idalcOpenDevice )( ALubyte * );
extern ALvoid ( ALAPIENTRY * idalListenerfv )( ALenum, ALfloat* );
extern ALvoid ( ALAPIENTRY * idalSourceQueueBuffers )( ALuint, ALsizei, ALuint * );
extern ALvoid ( ALAPIENTRY * idalSourcei )( ALuint, ALenum, ALint );
extern ALvoid ( ALAPIENTRY * idalListenerf )( ALenum, ALfloat );
extern ALCvoid ( ALAPIENTRY * idalcCloseDevice )( ALCdevice * );
extern ALboolean ( ALAPIENTRY * idalIsBuffer )( ALuint );
extern ALvoid ( ALAPIENTRY * idalSource3f )( ALuint, ALenum, ALfloat, ALfloat, ALfloat );
extern ALvoid ( ALAPIENTRY * idalGenSources )( ALsizei, ALuint * );
extern ALvoid ( ALAPIENTRY * idalSourcef )( ALuint, ALenum, ALfloat );
extern ALvoid ( ALAPIENTRY * idalSourceUnqueueBuffers )( ALuint, ALsizei, ALuint * );
extern ALvoid ( ALAPIENTRY * idalSourcePlay )( ALuint );

#define alGetError idalGetError
#define alGenBuffers idalGenBuffers
#define alIsSource idalIsSource
#define alSourceStop idalSourceStop
#define alGetSourcei idalGetSourcei
#define alGetInteger idalGetInteger
#define alcSuspendContext idalcSuspendContext
#define alcMakeContextCurrent idalcMakeContextCurrent
#define alcProcessContext idalcProcessContext
#define alcDestroyContext idalcDestroyContext
#define alcGetString idalcGetString
#define alBufferData idalBufferData
#define alDeleteBuffers idalDeleteBuffers
#define alIsExtensionPresent idalIsExtensionPresent
#define alDeleteSources idalDeleteSources
#define alGetEnumValue idalGetEnumValue
#define alGetProcAddress idalGetProcAddress
#define alcCreateContext idalcCreateContext
#define alcOpenDevice idalcOpenDevice
#define alListenerfv idalListenerfv
#define alSourceQueueBuffers idalSourceQueueBuffers
#define alSourcei idalSourcei
#define alListenerf idalListenerf
#define alcCloseDevice idalcCloseDevice
#define alIsBuffer idalIsBuffer
#define alSource3f idalSource3f
#define alGenSources idalGenSources
#define alSourcef idalSourcef
#define alSourceUnqueueBuffers idalSourceUnqueueBuffers
#define alSourcePlay idalSourcePlay

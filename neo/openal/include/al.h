#ifndef _AL_H_
#define _AL_H_

/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA  02111-1307, USA.
 * Or go to http://www.gnu.org/copyleft/lgpl.html
 */
#include "altypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
 #ifdef _OPENAL32LIB
  #define ALAPI __declspec(dllexport)
 #else
  #define ALAPI __declspec(dllimport)
 #endif
 #define ALAPIENTRY __cdecl
 #define AL_CALLBACK
#else
 #ifdef TARGET_OS_MAC
  #if TARGET_OS_MAC
   #pragma export on
  #endif
 #endif
 #define ALAPI
 #define ALAPIENTRY __cdecl
 #define AL_CALLBACK
#endif

#define OPENAL

#ifndef AL_NO_PROTOTYPES

/**
 * OpenAL Maintenance Functions
 * Initialization and exiting.
 * State Management and Query.
 * Error Handling.
 * Extension Support.
 */

/** State management. */
ALAPI ALvoid	ALAPIENTRY alEnable( ALenum capability );
ALAPI ALvoid	ALAPIENTRY alDisable( ALenum capability ); 
ALAPI ALboolean ALAPIENTRY alIsEnabled( ALenum capability ); 

/** Application preferences for driver performance choices. */
ALAPI ALvoid	ALAPIENTRY alHint( ALenum target, ALenum mode );

/** State retrieval. */
ALAPI ALboolean ALAPIENTRY alGetBoolean( ALenum param );
ALAPI ALint		ALAPIENTRY alGetInteger( ALenum param );
ALAPI ALfloat	ALAPIENTRY alGetFloat( ALenum param );
ALAPI ALdouble	ALAPIENTRY alGetDouble( ALenum param );
ALAPI ALvoid	ALAPIENTRY alGetBooleanv( ALenum param, ALboolean* data );
ALAPI ALvoid	ALAPIENTRY alGetIntegerv( ALenum param, ALint* data );
ALAPI ALvoid	ALAPIENTRY alGetFloatv( ALenum param, ALfloat* data );
ALAPI ALvoid	ALAPIENTRY alGetDoublev( ALenum param, ALdouble* data );
ALAPI ALubyte*	ALAPIENTRY alGetString( ALenum param );

/**
 * Error support.
 * Obtain the most recent error generated in the AL state machine.
 */
ALAPI ALenum	ALAPIENTRY alGetError( ALvoid );


/** 
 * Extension support.
 * Obtain the address of a function (usually an extension)
 *  with the name fname. All addresses are context-independent. 
 */
ALAPI ALboolean ALAPIENTRY alIsExtensionPresent( ALubyte* fname );


/** 
 * Extension support.
 * Obtain the address of a function (usually an extension)
 *  with the name fname. All addresses are context-independent. 
 */
ALAPI ALvoid*	ALAPIENTRY alGetProcAddress( ALubyte* fname );


/**
 * Extension support.
 * Obtain the integer value of an enumeration (usually an extension) with the name ename. 
 */
ALAPI ALenum	ALAPIENTRY alGetEnumValue( ALubyte* ename );




/**
 * LISTENER
 * Listener is the sample position for a given context.
 * The multi-channel (usually stereo) output stream generated
 *  by the mixer is parametrized by this Listener object:
 *  its position and velocity relative to Sources, within
 *  occluder and reflector geometry.
 */



/**
 *
 * Listener Environment:  default 0.
 */
ALAPI ALvoid	ALAPIENTRY alListeneri( ALenum param, ALint value );


/**
 *
 * Listener Gain:  default 1.0f.
 */
ALAPI ALvoid	ALAPIENTRY alListenerf( ALenum param, ALfloat value );


/**  
 *
 * Listener Position.
 * Listener Velocity.
 */
ALAPI ALvoid	ALAPIENTRY alListener3f( ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 ); 


/**
 *
 * Listener Position:        ALfloat[3]
 * Listener Velocity:        ALfloat[3]
 * Listener Orientation:     ALfloat[6]  (forward and up vector).
 */
ALAPI ALvoid	ALAPIENTRY alListenerfv( ALenum param, ALfloat* values ); 

ALAPI ALvoid	ALAPIENTRY alGetListeneri( ALenum param, ALint* value );
ALAPI ALvoid	ALAPIENTRY alGetListenerf( ALenum param, ALfloat* value );
ALAPI ALvoid	ALAPIENTRY alGetListener3f( ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3 ); 
ALAPI ALvoid	ALAPIENTRY alGetListenerfv( ALenum param, ALfloat* values ); 


/**
 * SOURCE
 * Source objects are by default localized. Sources
 *  take the PCM data provided in the specified Buffer,
 *  apply Source-specific modifications, and then
 *  submit them to be mixed according to spatial 
 *  arrangement etc.
 */



/** Create Source objects. */
ALAPI ALvoid	ALAPIENTRY alGenSources( ALsizei n, ALuint* sources ); 

/** Delete Source objects. */
ALAPI ALvoid	ALAPIENTRY alDeleteSources( ALsizei n, ALuint* sources );

/** Verify a handle is a valid Source. */ 
ALAPI ALboolean ALAPIENTRY alIsSource( ALuint id ); 

/** Set an integer parameter for a Source object. */
ALAPI ALvoid	ALAPIENTRY alSourcei( ALuint source, ALenum param, ALint value ); 
ALAPI ALvoid	ALAPIENTRY alSourcef( ALuint source, ALenum param, ALfloat value ); 
ALAPI ALvoid	ALAPIENTRY alSource3f( ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 );
ALAPI ALvoid	ALAPIENTRY alSourcefv( ALuint source, ALenum param, ALfloat* values ); 

/** Get an integer parameter for a Source object. */
ALAPI ALvoid	ALAPIENTRY alGetSourcei( ALuint source,  ALenum param, ALint* value );
ALAPI ALvoid	ALAPIENTRY alGetSourcef( ALuint source,  ALenum param, ALfloat* value );
ALAPI ALvoid	ALAPIENTRY alGetSource3f( ALuint source,  ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3 );
ALAPI ALvoid	ALAPIENTRY alGetSourcefv( ALuint source, ALenum param, ALfloat* values );

ALAPI ALvoid	ALAPIENTRY alSourcePlayv( ALsizei n, ALuint *sources );
ALAPI ALvoid	ALAPIENTRY alSourcePausev( ALsizei n, ALuint *sources );
ALAPI ALvoid	ALAPIENTRY alSourceStopv( ALsizei n, ALuint *sources );
ALAPI ALvoid	ALAPIENTRY alSourceRewindv(ALsizei n,ALuint *sources);

/** Activate a source, start replay. */
ALAPI ALvoid	ALAPIENTRY alSourcePlay( ALuint source );

/**
 * Pause a source, 
 *  temporarily remove it from the mixer list.
 */
ALAPI ALvoid	ALAPIENTRY alSourcePause( ALuint source );

/**
 * Stop a source,
 *  temporarily remove it from the mixer list,
 *  and reset its internal state to pre-Play.
 * To remove a Source completely, it has to be
 *  deleted following Stop, or before Play.
 */
ALAPI ALvoid	ALAPIENTRY alSourceStop( ALuint source );

/**
 * Rewinds a source, 
 *  temporarily remove it from the mixer list,
 *  and reset its internal state to pre-Play.
 */
ALAPI ALvoid	ALAPIENTRY alSourceRewind( ALuint source );



/**
 * BUFFER
 * Buffer objects are storage space for sample data.
 * Buffers are referred to by Sources. There can be more than
 *  one Source using the same Buffer data. If Buffers have
 *  to be duplicated on a per-Source basis, the driver has to
 *  take care of allocation, copying, and deallocation as well
 *  as propagating buffer data changes.
 */




/** Buffer object generation. */
ALAPI ALvoid 	ALAPIENTRY alGenBuffers( ALsizei n, ALuint* buffers );
ALAPI ALvoid	ALAPIENTRY alDeleteBuffers( ALsizei n, ALuint* buffers );
ALAPI ALboolean ALAPIENTRY alIsBuffer( ALuint buffer );

/**
 * Specify the data to be filled into a buffer.
 */
ALAPI ALvoid	ALAPIENTRY alBufferData( ALuint   buffer,
										 ALenum   format,
										 ALvoid*  data,
										 ALsizei  size,
										 ALsizei  freq );


ALAPI ALvoid	ALAPIENTRY alGetBufferi( ALuint buffer, ALenum param, ALint*   value );
ALAPI ALvoid	ALAPIENTRY alGetBufferf( ALuint buffer, ALenum param, ALfloat* value );




/**
 * Queue stuff
 */

ALAPI ALvoid	ALAPIENTRY alSourceQueueBuffers( ALuint source, ALsizei n, ALuint* buffers );
ALAPI ALvoid	ALAPIENTRY alSourceUnqueueBuffers( ALuint source, ALsizei n, ALuint* buffers );

/**
 * Knobs and dials
 */
ALAPI ALvoid	ALAPIENTRY alDistanceModel( ALenum value );
ALAPI ALvoid	ALAPIENTRY alDopplerFactor( ALfloat value );
ALAPI ALvoid	ALAPIENTRY alDopplerVelocity( ALfloat value );

#else /* AL_NO_PROTOTYPES */

/**
 * OpenAL Maintenance Functions
 * Initialization and exiting.
 * State Management and Query.
 * Error Handling.
 * Extension Support.
 */

/** State management. */
ALAPI ALvoid	ALAPIENTRY (*alEnable)( ALenum capability );
ALAPI ALvoid	ALAPIENTRY (*alDisable)( ALenum capability ); 
ALAPI ALboolean ALAPIENTRY (*alIsEnabled)( ALenum capability ); 

/** Application preferences for driver performance choices. */
ALAPI ALvoid	ALAPIENTRY (*alHint)( ALenum target, ALenum mode );

/** State retrieval. */
ALAPI ALboolean ALAPIENTRY (*alGetBoolean)( ALenum param );
ALAPI ALint		ALAPIENTRY (*alGetInteger)( ALenum param );
ALAPI ALfloat	ALAPIENTRY (*alGetFloat)( ALenum param );
ALAPI ALdouble	ALAPIENTRY (*alGetDouble)( ALenum param );
ALAPI ALvoid	ALAPIENTRY (*alGetBooleanv)( ALenum param, ALboolean* data );
ALAPI ALvoid	ALAPIENTRY (*alGetIntegerv)( ALenum param, ALint* data );
ALAPI ALvoid	ALAPIENTRY (*alGetFloatv)( ALenum param, ALfloat* data );
ALAPI ALvoid	ALAPIENTRY (*alGetDoublev)( ALenum param, ALdouble* data );
ALAPI ALubyte*	ALAPIENTRY (*alGetString)( ALenum param );

/**
 * Error support.
 * Obtain the most recent error generated in the AL state machine.
 */
ALAPI ALenum	ALAPIENTRY (*alGetError)( ALvoid );


/** 
 * Extension support.
 * Obtain the address of a function (usually an extension)
 *  with the name fname. All addresses are context-independent. 
 */
ALAPI ALboolean ALAPIENTRY (*alIsExtensionPresent)( ALubyte* fname );


/** 
 * Extension support.
 * Obtain the address of a function (usually an extension)
 *  with the name fname. All addresses are context-independent. 
 */
ALAPI ALvoid*	ALAPIENTRY (*alGetProcAddress)( ALubyte* fname );


/**
 * Extension support.
 * Obtain the integer value of an enumeration (usually an extension) with the name ename. 
 */
ALAPI ALenum	ALAPIENTRY (*alGetEnumValue)( ALubyte* ename );




/**
 * LISTENER
 * Listener is the sample position for a given context.
 * The multi-channel (usually stereo) output stream generated
 *  by the mixer is parametrized by this Listener object:
 *  its position and velocity relative to Sources, within
 *  occluder and reflector geometry.
 */



/**
 *
 * Listener Environment:  default 0.
 */
ALAPI ALvoid	ALAPIENTRY (*alListeneri)( ALenum param, ALint value );


/**
 *
 * Listener Gain:  default 1.0f.
 */
ALAPI ALvoid	ALAPIENTRY (*alListenerf)( ALenum param, ALfloat value );


/**  
 *
 * Listener Position.
 * Listener Velocity.
 */
ALAPI ALvoid	ALAPIENTRY (*alListener3f)( ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 ); 


/**
 *
 * Listener Position:        ALfloat[3]
 * Listener Velocity:        ALfloat[3]
 * Listener Orientation:     ALfloat[6]  (forward and up vector).
 */
ALAPI ALvoid	ALAPIENTRY (*alListenerfv)( ALenum param, ALfloat* values ); 

ALAPI ALvoid	ALAPIENTRY (*alGetListeneri)( ALenum param, ALint* value );
ALAPI ALvoid	ALAPIENTRY (*alGetListenerf)( ALenum param, ALfloat* value );
ALAPI ALvoid	ALAPIENTRY (*alGetListener3f)( ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3 ); 
ALAPI ALvoid	ALAPIENTRY (*alGetListenerfv)( ALenum param, ALfloat* values ); 


/**
 * SOURCE
 * Source objects are by default localized. Sources
 *  take the PCM data provided in the specified Buffer,
 *  apply Source-specific modifications, and then
 *  submit them to be mixed according to spatial 
 *  arrangement etc.
 */



/** Create Source objects. */
ALAPI ALvoid	ALAPIENTRY (*alGenSources)( ALsizei n, ALuint* sources ); 

/** Delete Source objects. */
ALAPI ALvoid	ALAPIENTRY (*alDeleteSources)( ALsizei n, ALuint* sources );

/** Verify a handle is a valid Source. */ 
ALAPI ALboolean ALAPIENTRY (*alIsSource)( ALuint id ); 

/** Set an integer parameter for a Source object. */
ALAPI ALvoid	ALAPIENTRY (*alSourcei)( ALuint source, ALenum param, ALint value ); 
ALAPI ALvoid	ALAPIENTRY (*alSourcef)( ALuint source, ALenum param, ALfloat value ); 
ALAPI ALvoid	ALAPIENTRY (*alSource3f)( ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3 );
ALAPI ALvoid	ALAPIENTRY (*alSourcefv)( ALuint source, ALenum param, ALfloat* values ); 

/** Get an integer parameter for a Source object. */
ALAPI ALvoid	ALAPIENTRY (*alGetSourcei)( ALuint source,  ALenum param, ALint* value );
ALAPI ALvoid	ALAPIENTRY (*alGetSourcef)( ALuint source,  ALenum param, ALfloat* value );
ALAPI ALvoid	ALAPIENTRY (*alGetSourcefv)( ALuint source, ALenum param, ALfloat* values );

ALAPI ALvoid	ALAPIENTRY (*alSourcePlayv)( ALsizei n, ALuint *sources );
ALAPI ALvoid	ALAPIENTRY (*alSourceStopv)( ALsizei n, ALuint *sources );

/** Activate a source, start replay. */
ALAPI ALvoid	ALAPIENTRY (*alSourcePlay)( ALuint source );

/**
 * Pause a source, 
 *  temporarily remove it from the mixer list.
 */
ALAPI ALvoid	ALAPIENTRY (*alSourcePause)( ALuint source );

/**
 * Stop a source,
 *  temporarily remove it from the mixer list,
 *  and reset its internal state to pre-Play.
 * To remove a Source completely, it has to be
 *  deleted following Stop, or before Play.
 */
ALAPI ALvoid	ALAPIENTRY (*alSourceStop)( ALuint source );



/**
 * BUFFER
 * Buffer objects are storage space for sample data.
 * Buffers are referred to by Sources. There can be more than
 *  one Source using the same Buffer data. If Buffers have
 *  to be duplicated on a per-Source basis, the driver has to
 *  take care of allocation, copying, and deallocation as well
 *  as propagating buffer data changes.
 */




/** Buffer object generation. */
ALAPI ALvoid 	ALAPIENTRY (*alGenBuffers)( ALsizei n, ALuint* buffers );
ALAPI ALvoid	ALAPIENTRY (*alDeleteBuffers)( ALsizei n, ALuint* buffers );
ALAPI ALboolean ALAPIENTRY (*alIsBuffer)( ALuint buffer );

/**
 * Specify the data to be filled into a buffer.
 */
ALAPI ALvoid	ALAPIENTRY (*alBufferData)( ALuint   buffer,
											ALenum   format,
											ALvoid*  data,
											ALsizei  size,
											ALsizei  freq );

ALAPI ALvoid	ALAPIENTRY (*alGetBufferi)( ALuint buffer, ALenum param, ALint*   value );
ALAPI ALvoid	ALAPIENTRY (*alGetBufferf)( ALuint buffer, ALenum param, ALfloat* value );




/**
 * Queue stuff
 */
ALAPI ALvoid	ALAPIENTRY (*alSourceQueueBuffers)( ALuint source, ALsizei n, ALuint* buffers );
ALAPI ALvoid	ALAPIENTRY (*alSourceUnqueueBuffers)( ALuint source, ALsizei n, ALuint* buffers );

/**
 * Knobs and dials
 */
ALAPI ALvoid	ALAPIENTRY (*alDistanceModel)( ALenum value );
ALAPI ALvoid	ALAPIENTRY (*alDopplerFactor)( ALfloat value );
ALAPI ALvoid	ALAPIENTRY (*alDopplerVelocity)( ALfloat value );

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


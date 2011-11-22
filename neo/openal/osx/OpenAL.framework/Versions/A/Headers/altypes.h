#ifndef _ALTYPES_H_
#define _ALTYPES_H_

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


#ifdef __cplusplus
extern "C" {
#endif

/** OpenAL boolean type. */
typedef char ALboolean;

/** OpenAL 8bit signed byte. */
typedef char ALbyte;

/** OpenAL 8bit unsigned byte. */
typedef unsigned char ALubyte;

/** OpenAL 16bit signed short integer type. */
typedef short ALshort;

/** OpenAL 16bit unsigned short integer type. */
typedef unsigned short ALushort;

/** OpenAL 32bit unsigned integer type. */
typedef unsigned ALuint;

/** OpenAL 32bit signed integer type. */
typedef int ALint;

/** OpenAL 32bit floating point type. */
typedef float ALfloat;

/** OpenAL 64bit double point type. */
typedef double ALdouble;

/** OpenAL 32bit type. */
typedef unsigned int ALsizei;

/** OpenAL void type */
typedef void ALvoid;

/** OpenAL enumerations. */
typedef int ALenum;

/* Bad value. */
#define AL_INVALID                               (-1)

/* Disable value. */
#define AL_NONE									 0

/* Boolean False. */
#define AL_FALSE                                 0

/* Boolean True. */
#define AL_TRUE                                  1

/**
  * Indicate the type of AL_SOURCE.
  * Sources can be spatialized 
  */
#define AL_SOURCE_TYPE                           0x200

/** Indicate source has absolute coordinates. */
#define AL_SOURCE_ABSOLUTE                       0x201

/** Indicate Source has listener relative coordinates. */
#define AL_SOURCE_RELATIVE                       0x202

/**
 * Directional source, inner cone angle, in degrees.
 * Range:    [0-360] 
 * Default:  360
 */
#define AL_CONE_INNER_ANGLE                      0x1001

/**
 * Directional source, outer cone angle, in degrees.
 * Range:    [0-360] 
 * Default:  360
 */
#define AL_CONE_OUTER_ANGLE                      0x1002

/**
 * Specify the pitch to be applied, either at source,
 *  or on mixer results, at listener.
 * Range:	 [0.5-2.0]
 * Default:  1.0
 */
#define AL_PITCH                                 0x1003

/** 
 * Specify the current location in three dimensional space.
 * OpenAL, like OpenGL, uses a right handed coordinate system,
 *  where in a frontal default view X (thumb) points right, 
 *  Y points up (index finger), and Z points towards the
 *  viewer/camera (middle finger). 
 * To switch from a left handed coordinate system, flip the
 *  sign on the Z coordinate.
 * Listener position is always in the world coordinate system.
 */ 
#define AL_POSITION                              0x1004
  
/** Specify the current direction as forward vector. */
#define AL_DIRECTION                             0x1005
  
/** Specify the current velocity in three dimensional space. */
#define AL_VELOCITY                              0x1006

/**
 * Indicate whether source has to loop infinite.
 * Type: ALboolean
 * Range:    [AL_TRUE, AL_FALSE]
 * Default:  AL_FALSE
 */
#define AL_LOOPING                               0x1007

/**
 * Indicate the buffer to provide sound samples. 
 * Type: ALuint.
 * Range: any valid Buffer id.
 */
#define AL_BUFFER                                0x1009

/**
 * Indicate the gain (volume amplification) applied. 
 * Type:     ALfloat.
 * Range:    ]0.0-  ]
 * A value of 1.0 means un-attenuated/unchanged.
 * Each division by 2 equals an attenuation of -6dB.
 * Each multiplicaton with 2 equals an amplification of +6dB.
 * A value of 0.0 is meaningless with respect to a logarithmic
 *  scale; it is interpreted as zero volume - the channel
 *  is effectively disabled.
 */
#define AL_GAIN                                  0x100A

/**
 * Indicate minimum source attenuation.
 * Type:     ALfloat
 * Range:	 [0.0 - 1.0]
 */
#define AL_MIN_GAIN                              0x100D

/**
 * Indicate maximum source attenuation.
 * Type:	 ALfloat
 * Range:	 [0.0 - 1.0]
 */
#define AL_MAX_GAIN                              0x100E

/** 
 * Specify the current orientation.
 * Type:	 ALfv6 (at/up)
 * Range:	 N/A
 */
#define AL_ORIENTATION                           0x100F

/* byte offset into source (in canon format).  -1 if source
 * is not playing.  Don't set this, get this.
 *
 * Type:     ALfloat
 * Range:    [0.0 - ]
 * Default:  1.0
 */
#define AL_REFERENCE_DISTANCE                    0x1020

 /**
 * Indicate the rolloff factor for the source.
 * Type: ALfloat
 * Range:    [0.0 - ]
 * Default:  1.0
 */
#define AL_ROLLOFF_FACTOR                        0x1021

/**
 * Indicate the gain (volume amplification) applied. 
 * Type:     ALfloat.
 * Range:    ]0.0-  ]
 * A value of 1.0 means un-attenuated/unchanged.
 * Each division by 2 equals an attenuation of -6dB.
 * Each multiplicaton with 2 equals an amplification of +6dB.
 * A value of 0.0 is meaningless with respect to a logarithmic
 *  scale; it is interpreted as zero volume - the channel
 *  is effectively disabled.
 */
#define AL_CONE_OUTER_GAIN                       0x1022

/** 
 * Specify the maximum distance.
 * Type:	 ALfloat
 * Range:	 [0.0 - ]
 */
#define AL_MAX_DISTANCE                          0x1023

/**
 * Source state information
 */
#define AL_SOURCE_STATE                          0x1010
#define AL_INITIAL                               0x1011
#define AL_PLAYING                               0x1012
#define AL_PAUSED                                0x1013
#define AL_STOPPED                               0x1014

/**
 * Buffer Queue params
 */
#define AL_BUFFERS_QUEUED                        0x1015
#define AL_BUFFERS_PROCESSED                     0x1016

/** Sound buffers: format specifier. */
#define AL_FORMAT_MONO8                          0x1100
#define AL_FORMAT_MONO16                         0x1101
#define AL_FORMAT_STEREO8                        0x1102
#define AL_FORMAT_STEREO16                       0x1103

/** 
 * Sound buffers: frequency, in units of Hertz [Hz].
 * This is the number of samples per second. Half of the
 *  sample frequency marks the maximum significant
 *  frequency component.
 */
#define AL_FREQUENCY                             0x2001
#define AL_BITS                                  0x2002
#define AL_CHANNELS                              0x2003
#define AL_SIZE                                  0x2004
#define AL_DATA                                  0x2005

/**
 * Buffer state.
 *
 * Not supported for public use (yet).
 */
#define AL_UNUSED                                0x2010
#define AL_PENDING                               0x2011
#define AL_PROCESSED                             0x2012

/** Errors: No Error. */
#define AL_NO_ERROR                              AL_FALSE

/** 
 * Illegal name passed as an argument to an AL call.
 */
#define AL_INVALID_NAME                          0xA001

/** 
 * Illegal enum passed as an argument to an AL call.
 */
#define AL_INVALID_ENUM                          0xA002  
/** 
 * Illegal value passed as an argument to an AL call.
 * Applies to parameter values, but not to enumerations.
 */
#define AL_INVALID_VALUE                         0xA003
  
/**
 * A function was called at inappropriate time,
 *  or in an inappropriate way, causing an illegal state.
 * This can be an incompatible ALenum, object ID,
 *  and/or function.
 */
#define AL_INVALID_OPERATION                     0xA004

/**
 * A function could not be completed,
 * because there is not enough memory available.
 */
#define AL_OUT_OF_MEMORY                         0xA005

/** Context strings: Vendor Name. */
#define AL_VENDOR                                0xB001
#define AL_VERSION                               0xB002
#define AL_RENDERER                              0xB003
#define AL_EXTENSIONS                            0xB004

/** Global tweakage. */

/**
 * Doppler scale.  Default 1.0
 */
#define AL_DOPPLER_FACTOR                        0xC000
 
/**
 * Doppler velocity.  Default 1.0
 */
#define AL_DOPPLER_VELOCITY                      0xC001

/**
 * Distance model.  Default AL_INVERSE_DISTANCE_CLAMPED
 */
#define AL_DISTANCE_MODEL                        0xD000

/** Distance models. */

#define AL_INVERSE_DISTANCE                      0xD001
#define AL_INVERSE_DISTANCE_CLAMPED              0xD002
 
 /**
 * enables
 */

#ifdef __cplusplus
}
#endif

#endif


#ifndef _ALCTYPES_H_
#define _ALCTYPES_H_

/**
 * OpenAL cross platform audio library
 * Copyright (C) 1999-2000 by authors.
 * Portions Copyright (C) 2004 by Apple Computer Inc.
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
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

/** ALC boolean type. */
typedef char ALCboolean;

/** ALC 8bit signed byte. */
typedef char ALCbyte;

/** ALC 8bit unsigned byte. */
typedef unsigned char ALCubyte;

/** ALC 16bit signed short integer type. */
typedef short ALCshort;

/** ALC 16bit unsigned short integer type. */
typedef unsigned short ALCushort;

/** ALC 32bit unsigned integer type. */
typedef unsigned ALCuint;

/** ALC 32bit signed integer type. */
typedef int ALCint;

/** ALC 32bit floating point type. */
typedef float ALCfloat;

/** ALC 64bit double point type. */
typedef double ALCdouble;

/** ALC 32bit type. */
typedef unsigned int ALCsizei;

/** ALC void type */
typedef void ALCvoid;

/** ALC enumerations. */
typedef int ALCenum;

/* Bad value. */
#define ALC_INVALID                              (-1)

/* Boolean False. */
#define ALC_FALSE                                0

/* Boolean True. */
#define ALC_TRUE                                 1

/** Errors: No Error. */
#define ALC_NO_ERROR                             ALC_FALSE

#define ALC_MAJOR_VERSION                        0x1000
#define ALC_MINOR_VERSION                        0x1001
#define ALC_ATTRIBUTES_SIZE                      0x1002
#define ALC_ALL_ATTRIBUTES                       0x1003

#define ALC_DEFAULT_DEVICE_SPECIFIER             0x1004
#define ALC_DEVICE_SPECIFIER                     0x1005
#define ALC_EXTENSIONS                           0x1006

#define ALC_FREQUENCY							 0x1007
#define	ALC_REFRESH								 0x1008
#define ALC_SYNC								 0x1009

/** 
 * The device argument does not name a valid dvice.
 */
#define ALC_INVALID_DEVICE                       0xA001

/** 
 * The context argument does not name a valid context.
 */
#define ALC_INVALID_CONTEXT                      0xA002  

/**
 * A function was called at inappropriate time,
 *  or in an inappropriate way, causing an illegal state.
 * This can be an incompatible ALenum, object ID,
 *  and/or function.
 */
#define ALC_INVALID_ENUM						 0xA003

/** 
 * Illegal value passed as an argument to an AL call.
 * Applies to parameter values, but not to enumerations.
 */
#define ALC_INVALID_VALUE                        0xA004

/**
 * A function could not be completed,
 * because there is not enough memory available.
 */
#define ALC_OUT_OF_MEMORY                        0xA005


//*********************************************************************************
// OSX Specific Properties
//*********************************************************************************

/**
 * Convert Data When Loading.  Default false, currently applies only to monophonic sounds
 */
#define ALC_CONVERT_DATA_UPON_LOADING         		0xF001

/**
 * Render Quality.  
 */
#define ALC_SPATIAL_RENDERING_QUALITY               0xF002
	#define ALC_SPATIAL_RENDERING_QUALITY_HIGH      'rqhi'
	#define ALC_SPATIAL_RENDERING_QUALITY_LOW       'rdlo'

/**
 * Mixer Output Rate.
 */
#define ALC_MIXER_OUTPUT_RATE		         		0xF003

/**
 *  Maximum Mixer Busses.
 *  Set this before opening a new OAL device to indicate how many busses on the mixer
 *  are desired. Get returns either the current devices bus count value, or the value
 *  that will be used to open a device
 */
#define ALC_MIXER_MAXIMUM_BUSSES                    0xF004

/**
 * Render Channels.  
 * Allows a user to force OpenAL to render to stereo, regardless of the audio hardware being used
 */
#define ALC_RENDER_CHANNEL_COUNT                    0xF005
	#define ALC_RENDER_CHANNEL_COUNT_STEREO         'rcst'
	#define ALC_RENDER_CHANNEL_COUNT_MULTICHANNEL   'rcmc'


#ifdef __cplusplus
}
#endif

#endif


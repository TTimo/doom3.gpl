/******************************************************************
*
*  EAX.H - DirectSound3D Environmental Audio Extensions version 2.0
*  Updated July 8, 1999
*
*******************************************************************
*/

#ifndef EAX20_H_INCLUDED
#define EAX20_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#pragma pack(push, 4)

/*
* EAX 2.0 listener property set {0306A6A8-B224-11d2-99E5-0000E8D8C722}
*/
DEFINE_GUID(DSPROPSETID_EAX20_ListenerProperties, 
    0x306a6a8, 
    0xb224, 
    0x11d2, 
    0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22);

typedef enum
{
    DSPROPERTY_EAX20LISTENER_NONE,
    DSPROPERTY_EAX20LISTENER_ALLPARAMETERS,
    DSPROPERTY_EAX20LISTENER_ROOM,
    DSPROPERTY_EAX20LISTENER_ROOMHF,
    DSPROPERTY_EAX20LISTENER_ROOMROLLOFFFACTOR,
    DSPROPERTY_EAX20LISTENER_DECAYTIME,
    DSPROPERTY_EAX20LISTENER_DECAYHFRATIO,
    DSPROPERTY_EAX20LISTENER_REFLECTIONS,
    DSPROPERTY_EAX20LISTENER_REFLECTIONSDELAY,
    DSPROPERTY_EAX20LISTENER_REVERB,
    DSPROPERTY_EAX20LISTENER_REVERBDELAY,
    DSPROPERTY_EAX20LISTENER_ENVIRONMENT,
    DSPROPERTY_EAX20LISTENER_ENVIRONMENTSIZE,
    DSPROPERTY_EAX20LISTENER_ENVIRONMENTDIFFUSION,
    DSPROPERTY_EAX20LISTENER_AIRABSORPTIONHF,
    DSPROPERTY_EAX20LISTENER_FLAGS
} DSPROPERTY_EAX20_LISTENERPROPERTY;
	
// OR these flags with property id
#define DSPROPERTY_EAX20LISTENER_IMMEDIATE 0x00000000 // changes take effect immediately
#define DSPROPERTY_EAX20LISTENER_DEFERRED  0x80000000 // changes take effect later
#define DSPROPERTY_EAX20LISTENER_COMMITDEFERREDSETTINGS (DSPROPERTY_EAX20LISTENER_NONE | \
                                                       DSPROPERTY_EAX20LISTENER_IMMEDIATE)

// Use this structure for DSPROPERTY_EAX20LISTENER_ALLPARAMETERS
// - all levels are hundredths of decibels
// - all times are in seconds
// - the reference for high frequency controls is 5 kHz
//
// NOTE: This structure may change in future EAX versions.
//       It is recommended to initialize fields by name:
//              myListener.lRoom = -1000;
//              myListener.lRoomHF = -100;
//              ...
//              myListener.dwFlags = myFlags /* see EAXLISTENERFLAGS below */ ;
//       instead of:
//              myListener = { -1000, -100, ... , 0x00000009 };
//       If you want to save and load presets in binary form, you 
//       should define your own structure to insure future compatibility.
//
typedef struct _EAX20LISTENERPROPERTIES
{
    long lRoom;                    // room effect level at low frequencies
    long lRoomHF;                  // room effect high-frequency level re. low frequency level
    float flRoomRolloffFactor;     // like DS3D flRolloffFactor but for room effect
    float flDecayTime;             // reverberation decay time at low frequencies
    float flDecayHFRatio;          // high-frequency to low-frequency decay time ratio
    long lReflections;             // early reflections level relative to room effect
    float flReflectionsDelay;      // initial reflection delay time
    long lReverb;                  // late reverberation level relative to room effect
    float flReverbDelay;           // late reverberation delay time relative to initial reflection
    unsigned long dwEnvironment;   // sets all listener properties
    float flEnvironmentSize;       // environment size in meters
    float flEnvironmentDiffusion;  // environment diffusion
    float flAirAbsorptionHF;       // change in level per meter at 5 kHz
    unsigned long dwFlags;         // modifies the behavior of properties
} EAX20LISTENERPROPERTIES, *LPEAX20LISTENERPROPERTIES;

// used by DSPROPERTY_EAX20LISTENER_ENVIRONMENT
enum
{
    EAX20_ENVIRONMENT_GENERIC,
    EAX20_ENVIRONMENT_PADDEDCELL,
    EAX20_ENVIRONMENT_ROOM,
    EAX20_ENVIRONMENT_BATHROOM,
    EAX20_ENVIRONMENT_LIVINGROOM,
    EAX20_ENVIRONMENT_STONEROOM,
    EAX20_ENVIRONMENT_AUDITORIUM,
    EAX20_ENVIRONMENT_CONCERTHALL,
    EAX20_ENVIRONMENT_CAVE,
    EAX20_ENVIRONMENT_ARENA,
    EAX20_ENVIRONMENT_HANGAR,
    EAX20_ENVIRONMENT_CARPETEDHALLWAY,
    EAX20_ENVIRONMENT_HALLWAY,
    EAX20_ENVIRONMENT_STONECORRIDOR,
    EAX20_ENVIRONMENT_ALLEY,
    EAX20_ENVIRONMENT_FOREST,
    EAX20_ENVIRONMENT_CITY,
    EAX20_ENVIRONMENT_MOUNTAINS,
    EAX20_ENVIRONMENT_QUARRY,
    EAX20_ENVIRONMENT_PLAIN,
    EAX20_ENVIRONMENT_PARKINGLOT,
    EAX20_ENVIRONMENT_SEWERPIPE,
    EAX20_ENVIRONMENT_UNDERWATER,
    EAX20_ENVIRONMENT_DRUGGED,
    EAX20_ENVIRONMENT_DIZZY,
    EAX20_ENVIRONMENT_PSYCHOTIC,

    EAX20_ENVIRONMENT_COUNT
};

// Used by DS20PROPERTY_EAXLISTENER_FLAGS
//
// Note: The number and order of flags may change in future EAX versions.
//       It is recommended to use the flag defines as follows:
//              myFlags = EAXLISTENERFLAGS_DECAYTIMESCALE | EAXLISTENERFLAGS_REVERBSCALE;
//       instead of:
//              myFlags = 0x00000009;
//
// These flags determine what properties are affected by environment size.
#define EAX20LISTENERFLAGS_DECAYTIMESCALE        0x00000001 // reverberation decay time
#define EAX20LISTENERFLAGS_REFLECTIONSSCALE      0x00000002 // reflection level
#define EAX20LISTENERFLAGS_REFLECTIONSDELAYSCALE 0x00000004 // initial reflection delay time
#define EAX20LISTENERFLAGS_REVERBSCALE           0x00000008 // reflections level
#define EAX20LISTENERFLAGS_REVERBDELAYSCALE      0x00000010 // late reverberation delay time

// This flag limits high-frequency decay time according to air absorption.
#define EAX20LISTENERFLAGS_DECAYHFLIMIT          0x00000020
 
#define EAX20LISTENERFLAGS_RESERVED              0xFFFFFFC0 // reserved future use

// property ranges and defaults:

#define EAX20LISTENER_MINROOM                       (-10000)
#define EAX20LISTENER_MAXROOM                       0
#define EAX20LISTENER_DEFAULTROOM                   (-1000)

#define EAX20LISTENER_MINROOMHF                     (-10000)
#define EAX20LISTENER_MAXROOMHF                     0
#define EAX20LISTENER_DEFAULTROOMHF                 (-100)

#define EAX20LISTENER_MINROOMROLLOFFFACTOR          0.0f
#define EAX20LISTENER_MAXROOMROLLOFFFACTOR          10.0f
#define EAX20LISTENER_DEFAULTROOMROLLOFFFACTOR      0.0f

#define EAX20LISTENER_MINDECAYTIME                  0.1f
#define EAX20LISTENER_MAXDECAYTIME                  20.0f
#define EAX20LISTENER_DEFAULTDECAYTIME              1.49f

#define EAX20LISTENER_MINDECAYHFRATIO               0.1f
#define EAX20LISTENER_MAXDECAYHFRATIO               2.0f
#define EAX20LISTENER_DEFAULTDECAYHFRATIO           0.83f

#define EAX20LISTENER_MINREFLECTIONS                (-10000)
#define EAX20LISTENER_MAXREFLECTIONS                1000
#define EAX20LISTENER_DEFAULTREFLECTIONS            (-2602)

#define EAX20LISTENER_MINREFLECTIONSDELAY           0.0f
#define EAX20LISTENER_MAXREFLECTIONSDELAY           0.3f
#define EAX20LISTENER_DEFAULTREFLECTIONSDELAY       0.007f

#define EAX20LISTENER_MINREVERB                     (-10000)
#define EAX20LISTENER_MAXREVERB                     2000
#define EAX20LISTENER_DEFAULTREVERB                 200

#define EAX20LISTENER_MINREVERBDELAY                0.0f
#define EAX20LISTENER_MAXREVERBDELAY                0.1f
#define EAX20LISTENER_DEFAULTREVERBDELAY            0.011f

#define EAX20LISTENER_MINENVIRONMENT                0
#define EAX20LISTENER_MAXENVIRONMENT                (EAX_ENVIRONMENT_COUNT-1)
#define EAX20LISTENER_DEFAULTENVIRONMENT            EAX_ENVIRONMENT_GENERIC

#define EAX20LISTENER_MINENVIRONMENTSIZE            1.0f
#define EAX20LISTENER_MAXENVIRONMENTSIZE            100.0f
#define EAX20LISTENER_DEFAULTENVIRONMENTSIZE        7.5f

#define EAX20LISTENER_MINENVIRONMENTDIFFUSION       0.0f
#define EAX20LISTENER_MAXENVIRONMENTDIFFUSION       1.0f
#define EAX20LISTENER_DEFAULTENVIRONMENTDIFFUSION   1.0f

#define EAX20LISTENER_MINAIRABSORPTIONHF            (-100.0f)
#define EAX20LISTENER_MAXAIRABSORPTIONHF            0.0f
#define EAX20LISTENER_DEFAULTAIRABSORPTIONHF        (-5.0f)

#define EAX20LISTENER_DEFAULTFLAGS                  (EAX20LISTENERFLAGS_DECAYTIMESCALE |        \
                                                   EAX20LISTENERFLAGS_REFLECTIONSSCALE |      \
                                                   EAX20LISTENERFLAGS_REFLECTIONSDELAYSCALE | \
                                                   EAX20LISTENERFLAGS_REVERBSCALE |           \
                                                   EAX20LISTENERFLAGS_REVERBDELAYSCALE |      \
                                                   EAX20LISTENERFLAGS_DECAYHFLIMIT)



/*
* EAX 2.0 buffer property set {0306A6A7-B224-11d2-99E5-0000E8D8C722}
*/
DEFINE_GUID(DSPROPSETID_EAX20_BufferProperties, 
    0x306a6a7, 
    0xb224, 
    0x11d2, 
    0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7, 0x22);

// For compatibility with future EAX versions:
#define DSPROPSETID_EAX20_BufferProperties DSPROPSETID_EAX20_BufferProperties

typedef enum
{
    DSPROPERTY_EAX20BUFFER_NONE,
    DSPROPERTY_EAX20BUFFER_ALLPARAMETERS,
    DSPROPERTY_EAX20BUFFER_DIRECT,
    DSPROPERTY_EAX20BUFFER_DIRECTHF,
    DSPROPERTY_EAX20BUFFER_ROOM,
    DSPROPERTY_EAX20BUFFER_ROOMHF, 
    DSPROPERTY_EAX20BUFFER_ROOMROLLOFFFACTOR,
    DSPROPERTY_EAX20BUFFER_OBSTRUCTION,
    DSPROPERTY_EAX20BUFFER_OBSTRUCTIONLFRATIO,
    DSPROPERTY_EAX20BUFFER_OCCLUSION, 
    DSPROPERTY_EAX20BUFFER_OCCLUSIONLFRATIO,
    DSPROPERTY_EAX20BUFFER_OCCLUSIONROOMRATIO,
    DSPROPERTY_EAX20BUFFER_OUTSIDEVOLUMEHF,
    DSPROPERTY_EAX20BUFFER_AIRABSORPTIONFACTOR,
    DSPROPERTY_EAX20BUFFER_FLAGS
} DSPROPERTY_EAX20_BUFFERPROPERTY;    

// OR these flags with property id
#define DSPROPERTY_EAX20BUFFER_IMMEDIATE 0x00000000 // changes take effect immediately
#define DSPROPERTY_EAX20BUFFER_DEFERRED  0x80000000 // changes take effect later
#define DSPROPERTY_EAX20BUFFER_COMMITDEFERREDSETTINGS (DSPROPERTY_EAX20BUFFER_NONE | \
                                                     DSPROPERTY_EAX20BUFFER_IMMEDIATE)

// Use this structure for DSPROPERTY_EAX20BUFFER_ALLPARAMETERS
// - all levels are hundredths of decibels
//
// NOTE: This structure may change in future EAX versions.
//       It is recommended to initialize fields by name:
//              myBuffer.lDirect = 0;
//              myBuffer.lDirectHF = -200;
//              ...
//              myBuffer.dwFlags = myFlags /* see EAXBUFFERFLAGS below */ ;
//       instead of:
//              myBuffer = { 0, -200, ... , 0x00000003 };
//
typedef struct _EAX20BUFFERPROPERTIES
{
    long lDirect;                // direct path level
    long lDirectHF;              // direct path level at high frequencies
    long lRoom;                  // room effect level
    long lRoomHF;                // room effect level at high frequencies
    float flRoomRolloffFactor;   // like DS3D flRolloffFactor but for room effect
    long lObstruction;           // main obstruction control (attenuation at high frequencies) 
    float flObstructionLFRatio;  // obstruction low-frequency level re. main control
    long lOcclusion;             // main occlusion control (attenuation at high frequencies)
    float flOcclusionLFRatio;    // occlusion low-frequency level re. main control
    float flOcclusionRoomRatio;  // occlusion room effect level re. main control
    long lOutsideVolumeHF;       // outside sound cone level at high frequencies
    float flAirAbsorptionFactor; // multiplies DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF
    unsigned long dwFlags;       // modifies the behavior of properties
} EAX20BUFFERPROPERTIES, *LPEAX20BUFFERPROPERTIES;

// Used by DSPROPERTY_EAX20BUFFER_FLAGS
//    TRUE:    value is computed automatically - property is an offset
//    FALSE:   value is used directly
//
// Note: The number and order of flags may change in future EAX versions.
//       To insure future compatibility, use flag defines as follows:
//              myFlags = EAXBUFFERFLAGS_DIRECTHFAUTO | EAXBUFFERFLAGS_ROOMAUTO;
//       instead of:
//              myFlags = 0x00000003;
//
#define EAX20BUFFERFLAGS_DIRECTHFAUTO 0x00000001 // affects DSPROPERTY_EAXBUFFER_DIRECTHF
#define EAX20BUFFERFLAGS_ROOMAUTO     0x00000002 // affects DSPROPERTY_EAXBUFFER_ROOM
#define EAX20BUFFERFLAGS_ROOMHFAUTO   0x00000004 // affects DSPROPERTY_EAXBUFFER_ROOMHF

#define EAX20BUFFERFLAGS_RESERVED     0xFFFFFFF8 // reserved future use

// property ranges and defaults:

#define EAX20BUFFER_MINDIRECT                  (-10000)
#define EAX20BUFFER_MAXDIRECT                  1000
#define EAX20BUFFER_DEFAULTDIRECT              0

#define EAX20BUFFER_MINDIRECTHF                (-10000)
#define EAX20BUFFER_MAXDIRECTHF                0
#define EAX20BUFFER_DEFAULTDIRECTHF            0

#define EAX20BUFFER_MINROOM                    (-10000)
#define EAX20BUFFER_MAXROOM                    1000
#define EAX20BUFFER_DEFAULTROOM                0

#define EAX20BUFFER_MINROOMHF                  (-10000)
#define EAX20BUFFER_MAXROOMHF                  0
#define EAX20BUFFER_DEFAULTROOMHF              0

#define EAX20BUFFER_MINROOMROLLOFFFACTOR       0.0f
#define EAX20BUFFER_MAXROOMROLLOFFFACTOR       10.f
#define EAX20BUFFER_DEFAULTROOMROLLOFFFACTOR   0.0f

#define EAX20BUFFER_MINOBSTRUCTION             (-10000)
#define EAX20BUFFER_MAXOBSTRUCTION             0
#define EAX20BUFFER_DEFAULTOBSTRUCTION         0

#define EAX20BUFFER_MINOBSTRUCTIONLFRATIO      0.0f
#define EAX20BUFFER_MAXOBSTRUCTIONLFRATIO      1.0f
#define EAX20BUFFER_DEFAULTOBSTRUCTIONLFRATIO  0.0f

#define EAX20BUFFER_MINOCCLUSION               (-10000)
#define EAX20BUFFER_MAXOCCLUSION               0
#define EAX20BUFFER_DEFAULTOCCLUSION           0

#define EAX20BUFFER_MINOCCLUSIONLFRATIO        0.0f
#define EAX20BUFFER_MAXOCCLUSIONLFRATIO        1.0f
#define EAX20BUFFER_DEFAULTOCCLUSIONLFRATIO    0.25f

#define EAX20BUFFER_MINOCCLUSIONROOMRATIO      0.0f
#define EAX20BUFFER_MAXOCCLUSIONROOMRATIO      10.0f
#define EAX20BUFFER_DEFAULTOCCLUSIONROOMRATIO  0.5f

#define EAX20BUFFER_MINOUTSIDEVOLUMEHF         (-10000)
#define EAX20BUFFER_MAXOUTSIDEVOLUMEHF         0
#define EAX20BUFFER_DEFAULTOUTSIDEVOLUMEHF     0

#define EAX20BUFFER_MINAIRABSORPTIONFACTOR     0.0f
#define EAX20BUFFER_MAXAIRABSORPTIONFACTOR     10.0f
#define EAX20BUFFER_DEFAULTAIRABSORPTIONFACTOR 1.0f

#define EAX20BUFFER_DEFAULTFLAGS               (EAX20BUFFERFLAGS_DIRECTHFAUTO | \
                                              EAX20BUFFERFLAGS_ROOMAUTO |     \
                                              EAX20BUFFERFLAGS_ROOMHFAUTO)

#pragma pack(pop)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

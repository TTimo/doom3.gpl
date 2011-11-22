/*******************************************************************\
*                                                                   *
*  EAX.H - Environmental Audio Extensions version 4.0               *
*          for OpenAL and DirectSound3D                             *
*                                                                   *
*          File revision 1.0.0 (GDC Beta SDK Release)               *
*                                                                   *
\*******************************************************************/

#ifndef EAX_H_INCLUDED
#define EAX_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef OPENAL
    #include <dsound.h>

    /*
     * EAX Unified Interface (using Direct X 7) {4FF53B81-1CE0-11d3-AAB8-00A0C95949D5}
     */
    DEFINE_GUID(CLSID_EAXDirectSound, 
        0x4ff53b81, 
        0x1ce0, 
        0x11d3,
        0xaa, 0xb8, 0x0, 0xa0, 0xc9, 0x59, 0x49, 0xd5);
        
   /*
    * EAX Unified Interface (using Direct X 8) {CA503B60-B176-11d4-A094-D0C0BF3A560C}
    */
    DEFINE_GUID(CLSID_EAXDirectSound8, 
        0xca503b60,
        0xb176,
        0x11d4,
        0xa0, 0x94, 0xd0, 0xc0, 0xbf, 0x3a, 0x56, 0xc);

    

#ifdef DIRECTSOUND_VERSION        
#if DIRECTSOUND_VERSION >= 0x0800
    __declspec(dllimport) HRESULT WINAPI EAXDirectSoundCreate8(GUID*, LPDIRECTSOUND8*, IUnknown FAR *);
    typedef HRESULT (FAR PASCAL *LPEAXDIRECTSOUNDCREATE8)(GUID*, LPDIRECTSOUND8*, IUnknown FAR*);
#endif
#endif
    
    __declspec(dllimport) HRESULT WINAPI EAXDirectSoundCreate(GUID*, LPDIRECTSOUND*, IUnknown FAR *);
    typedef HRESULT (FAR PASCAL *LPEAXDIRECTSOUNDCREATE)(GUID*, LPDIRECTSOUND*, IUnknown FAR*);

#else // OPENAL
//    #include <al.h>
    
    #ifndef GUID_DEFINED
        #define GUID_DEFINED
        typedef struct _GUID
        {
            unsigned long Data1;
            unsigned short Data2;
            unsigned short Data3;
            unsigned char Data4[8];
        } GUID;
    #endif // GUID_DEFINED

    #ifndef DEFINE_GUID
        #ifndef INITGUID
            #define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
                    extern const GUID /*FAR*/ name
        #else
            #define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
                    extern const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
        #endif // INITGUID
    #endif // DEFINE_GUID

    /*
     * EAX OpenAL Extensions
     */
    typedef ALenum (*EAXSet)(const GUID*, ALuint, ALuint, ALvoid*, ALuint);
    typedef ALenum (*EAXGet)(const GUID*, ALuint, ALuint, ALvoid*, ALuint);
	typedef ALboolean (*EAXSetBufferMode)(ALsizei, ALuint*, ALint);
	typedef ALenum (*EAXGetBufferMode)(ALuint, ALint*);
#endif

#pragma pack(push, 4)




////////////////////////////////////////////////////////////////////////////
// Constants

#define EAX_MAX_FXSLOTS 4
#define EAX_MAX_ACTIVE_FXSLOTS 2

// The EAX_NULL_GUID is used by EAXFXSLOT_LOADEFFECT, EAXCONTEXT_PRIMARYFXSLOTID
// and EAXSOURCE_ACTIVEFXSLOTID

// {00000000-0000-0000-0000-000000000000}
DEFINE_GUID(EAX_NULL_GUID, 
    0x00000000, 
    0x0000, 
    0x0000, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

// The EAX_PrimaryFXSlotID GUID is used by EAXSOURCE_ACTIVEFXSLOTID 

// {F317866D-924C-450C-861B-E6DAA25E7C20}
DEFINE_GUID(EAX_PrimaryFXSlotID, 
    0xf317866d,
    0x924c,
    0x450c,
    0x86, 0x1b, 0xe6, 0xda, 0xa2, 0x5e, 0x7c, 0x20);


////////////////////////////////////////////////////////////////////////////

    

    
////////////////////////////////////////////////////////////////////////////
// Structures

// Use this structure for EAXCONTEXT_ALL property.
typedef struct _EAXCONTEXTPROPERTIES
{
    GUID          guidPrimaryFXSlotID;
    float         flDistanceFactor;
    float         flAirAbsorptionHF;
    float         flHFReference;
} EAXCONTEXTPROPERTIES, *LPEAXCONTEXTPROPERTIES; 

// Use this structure for EAXSOURCE_ALLPARAMETERS
// - all levels are hundredths of decibels
// - all delays are in seconds
//
// NOTE: This structure may change in future EAX versions.
//       It is recommended to initialize fields by name:
//              myBuffer.lDirect = 0;
//              myBuffer.lDirectHF = -200;
//              ...
//              myBuffer.dwFlags = myFlags /* see EAXSOURCEFLAGS below */ ;
//       instead of:
//              myBuffer = { 0, -200, ... , 0x00000003 };
//
typedef struct _EAXSOURCEPROPERTIES
{
    long          lDirect;                 // direct path level (at low and mid frequencies)
    long          lDirectHF;               // relative direct path level at high frequencies
    long          lRoom;                   // room effect level (at low and mid frequencies)
    long          lRoomHF;                 // relative room effect level at high frequencies
    long          lObstruction;            // main obstruction control (attenuation at high frequencies) 
    float         flObstructionLFRatio;    // obstruction low-frequency level re. main control
    long          lOcclusion;              // main occlusion control (attenuation at high frequencies)
    float         flOcclusionLFRatio;      // occlusion low-frequency level re. main control
    float         flOcclusionRoomRatio;    // relative occlusion control for room effect
    float         flOcclusionDirectRatio;  // relative occlusion control for direct path
    long          lExclusion;              // main exlusion control (attenuation at high frequencies)
    float         flExclusionLFRatio;      // exclusion low-frequency level re. main control
    long          lOutsideVolumeHF;        // outside sound cone level at high frequencies
    float         flDopplerFactor;         // like DS3D flDopplerFactor but per source
    float         flRolloffFactor;         // like DS3D flRolloffFactor but per source
    float         flRoomRolloffFactor;     // like DS3D flRolloffFactor but for room effect
    float         flAirAbsorptionFactor;   // multiplies EAXREVERB_AIRABSORPTIONHF
    unsigned long ulFlags;                 // modifies the behavior of properties
} EAXSOURCEPROPERTIES, *LPEAXSOURCEPROPERTIES;

// Use this structure for EAXSOURCE_ALLSENDPARAMETERS
// - all levels are hundredths of decibels
//
typedef struct _EAXSOURCEALLSENDPROPERTIES
{
	GUID          guidReceivingFXSlotID;
	long          lSend;                   // send level (at low and mid frequencies)
	long          lSendHF;                 // relative send level at high frequencies
	long          lOcclusion;
	float         flOcclusionLFRatio;
	float         flOcclusionRoomRatio;
	float         flOcclusionDirectRatio;
	long          lExclusion; 
	float         flExclusionLFRatio;
} EAXSOURCEALLSENDPROPERTIES, *LPEAXSOURCEALLSENDPROPERTIES;

// Use this structure for EAXSOURCE_ACTIVEFXSLOTID
typedef struct _EAXACTIVEFXSLOTS
{
    GUID          guidActiveFXSlots[EAX_MAX_ACTIVE_FXSLOTS];
} EAXACTIVEFXSLOTS, *LPEAXACTIVEFXSLOTS;

// Use this structure for EAXSOURCE_OBSTRUCTIONPARAMETERS property.
#ifndef EAX_OBSTRUCTIONPROPERTIES_DEFINED
#define EAX_OBSTRUCTIONPROPERTIES_DEFINED
typedef struct _EAXOBSTRUCTIONPROPERTIES
{
    long          lObstruction;
    float         flObstructionLFRatio;
} EAXOBSTRUCTIONPROPERTIES, *LPEAXOBSTRUCTIONPROPERTIES;
#endif

// Use this structure for EAXSOURCE_OCCLUSIONPARAMETERS property.
#ifndef EAX_OCCLUSIONPROPERTIES_DEFINED
#define EAX_OCCLUSIONPROPERTIES_DEFINED
typedef struct _EAXOCCLUSIONPROPERTIES
{
    long          lOcclusion;
    float         flOcclusionLFRatio;
    float         flOcclusionRoomRatio;
    float         flOcclusionDirectRatio;
} EAXOCCLUSIONPROPERTIES, *LPEAXOCCLUSIONPROPERTIES;
#endif

// Use this structure for EAXSOURCE_EXCLUSIONPARAMETERS property.
#ifndef EAX_EXCLUSIONPROPERTIES_DEFINED
#define EAX_EXCLUSIONPROPERTIES_DEFINED
typedef struct _EAXEXCLUSIONPROPERTIES
{
    long          lExclusion;
    float         flExclusionLFRatio;
} EAXEXCLUSIONPROPERTIES, *LPEAXEXCLUSIONPROPERTIES;
#endif

// Use this structure for EAXSOURCE_SENDPARAMETERS properties.
typedef struct _EAXSOURCESENDPROPERTIES
{
    GUID          guidReceivingFXSlotID;
    long          lSend;
    long          lSendHF;
} EAXSOURCESENDPROPERTIES, *LPEAXSOURCESENDPROPERTIES;

// Use this structure for EAXSOURCE_OCCLUSIONSENDPARAMETERS 
typedef struct _EAXSOURCEOCCLUSIONSENDPROPERTIES
{
	GUID          guidReceivingFXSlotID;
	long          lOcclusion;
	float         flOcclusionLFRatio;
	float         flOcclusionRoomRatio;
	float         flOcclusionDirectRatio;
} EAXSOURCEOCCLUSIONSENDPROPERTIES, *LPEAXSOURCEOCCLUSIONSENDPROPERTIES;

// Use this structure for EAXSOURCE_EXCLUSIONSENDPARAMETERS
typedef struct _EAXSOURCEEXCLUSIONSENDPROPERTIES
{
	GUID          guidReceivingFXSlotID;
	long          lExclusion;
	float         flExclusionLFRatio;
} EAXSOURCEEXCLUSIONSENDPROPERTIES, *LPEAXSOURCEEXCLUSIONSENDPROPERTIES;

// Use this structure for EAXFXSLOT_ALLPARAMETERS
// - all levels are hundredths of decibels
//
// NOTE: This structure may change in future EAX versions.
//       It is recommended to initialize fields by name:
//              myFXSlot.guidLoadEffect = EAX_REVERB_EFFECT;
//              myFXSlot.lVolume = 0;
//              myFXSlot.lLock = 1;
//              myFXSlot.ulFlags = myFlags /* see EAXFXSLOTFLAGS below */ ;
//       instead of:
//              myFXSlot = { EAX_REVERB_EFFECT, 0, 1, 0x00000001 };
//
typedef struct _EAXFXSLOTPROPERTIES
{
	GUID          guidLoadEffect;
	long          lVolume;
	long          lLock;
	unsigned long ulFlags;
} EAXFXSLOTPROPERTIES, *LPEAXFXSLOTPROPERTIES;

// Use this structure for EAXREVERB_REFLECTIONSPAN and EAXREVERB_REVERBPAN properties.
#ifndef EAXVECTOR_DEFINED
#define EAXVECTOR_DEFINED
typedef struct _EAXVECTOR {
    float x;
    float y;
    float z;
} EAXVECTOR;
#endif


////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////
// Error Codes

#define EAX_OK                     0
#define EAXERR_INVALID_OPERATION   (-1)
#define EAXERR_INVALID_VALUE       (-2)
#define EAXERR_NO_EFFECT_LOADED    (-3)
#define EAXERR_UNKNOWN_EFFECT      (-4)
////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////
// Context Object

// {1D4870AD-0DEF-43c0-A40C-523632296342}
DEFINE_GUID(EAXPROPERTYID_EAX40_Context, 
    0x1d4870ad, 
    0xdef, 
    0x43c0, 
    0xa4, 0xc, 0x52, 0x36, 0x32, 0x29, 0x63, 0x42);

// For compatibility with future EAX versions:
#define EAXPROPERTYID_EAX_Context EAXPROPERTYID_EAX40_Context

typedef enum
{
    EAXCONTEXT_NONE = 0,
    EAXCONTEXT_ALLPARAMETERS,
    EAXCONTEXT_PRIMARYFXSLOTID,
    EAXCONTEXT_DISTANCEFACTOR,
    EAXCONTEXT_AIRABSORPTIONHF,
    EAXCONTEXT_HFREFERENCE,
    EAXCONTEXT_LASTERROR
} EAXCONTEXT_PROPERTY;

// OR these flags with property id
#define EAXCONTEXT_PARAMETER_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXCONTEXT_PARAMETER_DEFER     0x80000000 // changes take effect later
#define EAXCONTEXT_PARAMETER_COMMITDEFERREDSETTINGS (EAXCONTEXT_NONE | \
                                                      EAXCONTEXT_PARAMETER_IMMEDIATE)

// EAX Context property ranges and defaults:
#define EAXCONTEXT_DEFAULTPRIMARYFXSLOTID EAXPROPERTYID_EAX40_FXSlot0

#define EAXCONTEXT_MINDISTANCEFACTOR        FLT_MIN //minimum positive value
#define EAXCONTEXT_MAXDISTANCEFACTOR        FLT_MAX
#define EAXCONTEXT_DEFAULTDISTANCEFACTOR    1.0f

#define EAXCONTEXT_MINAIRABSORPTIONHF     (-100.0f)
#define EAXCONTEXT_MAXAIRABSORPTIONHF     0.0f
#define EAXCONTEXT_DEFAULTAIRABSORPTIONHF (-5.0f)

#define EAXCONTEXT_MINHFREFERENCE         1000.0f
#define EAXCONTEXT_MAXHFREFERENCE         20000.0f
#define EAXCONTEXT_DEFAULTHFREFERENCE     5000.0f

#define EAXCONTEXT_DEFAULTLASTERROR         EAX_OK

////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////
// Effect Slot Objects

// {C4D79F1E-F1AC-436b-A81D-A738E7045469}
DEFINE_GUID(EAXPROPERTYID_EAX40_FXSlot0, 
    0xc4d79f1e, 
    0xf1ac, 
    0x436b, 
    0xa8, 0x1d, 0xa7, 0x38, 0xe7, 0x4, 0x54, 0x69);

// {08C00E96-74BE-4491-93AA-E8AD35A49117}
DEFINE_GUID(EAXPROPERTYID_EAX40_FXSlot1, 
    0x8c00e96, 
    0x74be, 
    0x4491, 
    0x93, 0xaa, 0xe8, 0xad, 0x35, 0xa4, 0x91, 0x17);

// {1D433B88-F0F6-4637-919F-60E7E06B5EDD}
DEFINE_GUID(EAXPROPERTYID_EAX40_FXSlot2, 
    0x1d433b88, 
    0xf0f6, 
    0x4637, 
    0x91, 0x9f, 0x60, 0xe7, 0xe0, 0x6b, 0x5e, 0xdd);

// {EFFF08EA-C7D8-44ab-93AD-6DBD5F910064}
DEFINE_GUID(EAXPROPERTYID_EAX40_FXSlot3, 
    0xefff08ea, 
    0xc7d8, 
    0x44ab, 
    0x93, 0xad, 0x6d, 0xbd, 0x5f, 0x91, 0x0, 0x64);

// For compatibility with future EAX versions:
#define EAXPROPERTYID_EAX_FXSlot0 EAXPROPERTYID_EAX40_FXSlot0
#define EAXPROPERTYID_EAX_FXSlot1 EAXPROPERTYID_EAX40_FXSlot1
#define EAXPROPERTYID_EAX_FXSlot2 EAXPROPERTYID_EAX40_FXSlot2
#define EAXPROPERTYID_EAX_FXSlot3 EAXPROPERTYID_EAX40_FXSlot3

// FXSlot object properties
typedef enum
{
    EAXFXSLOT_PARAMETER = 0, // range 0-0x40 reserved for loaded effect parameters
    EAXFXSLOT_NONE = 0x10000,
	EAXFXSLOT_ALLPARAMETERS,
    EAXFXSLOT_LOADEFFECT,
    EAXFXSLOT_VOLUME,
	EAXFXSLOT_LOCK,
	EAXFXSLOT_FLAGS
} EAXFXSLOT_PROPERTY;

// Note: The number and order of flags may change in future EAX versions.
//       To insure future compatibility, use flag defines as follows:
//              myFlags = EAXFXSLOTFLAGS_ENVIRONMENT;
//       instead of:
//              myFlags = 0x00000001;
//
#define EAXFXSLOTFLAGS_ENVIRONMENT   0x00000001
#define EAXFXSLOTFLAGS_RESERVED      0xFFFFFFFE // reserved future use

// EAX Effect Slot property ranges and defaults:
#define EAXFXSLOT_MINVOLUME                    (-10000)
#define EAXFXSLOT_MAXVOLUME                    0
#define EAXFXSLOT_DEFAULTVOLUME                0

#define EAXFXSLOT_MINLOCK    0
#define EAXFXSLOT_MAXLOCK    1

enum
{
   EAXFXSLOT_UNLOCKED = 0,
   EAXFXSLOT_LOCKED = 1
};

#define EAXFXSLOT_DEFAULTFLAGS                 (EAXFXSLOTFLAGS_ENVIRONMENT)
////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////
// Source Object

// {1B86B823-22DF-4eae-8B3C-1278CE544227}
DEFINE_GUID(EAXPROPERTYID_EAX40_Source, 
    0x1b86b823, 
    0x22df, 
    0x4eae, 
    0x8b, 0x3c, 0x12, 0x78, 0xce, 0x54, 0x42, 0x27);

// For compatibility with future EAX versions:
#define EAXPROPERTYID_EAX_Source EAXPROPERTYID_EAX40_Source

// Source object properties
typedef enum
{
    EAXSOURCE_NONE,
    EAXSOURCE_ALLPARAMETERS,
    EAXSOURCE_OBSTRUCTIONPARAMETERS,
    EAXSOURCE_OCCLUSIONPARAMETERS,
    EAXSOURCE_EXCLUSIONPARAMETERS,
    EAXSOURCE_DIRECT,
    EAXSOURCE_DIRECTHF,
    EAXSOURCE_ROOM,
    EAXSOURCE_ROOMHF,
    EAXSOURCE_OBSTRUCTION,
    EAXSOURCE_OBSTRUCTIONLFRATIO,
    EAXSOURCE_OCCLUSION,
    EAXSOURCE_OCCLUSIONLFRATIO, 
    EAXSOURCE_OCCLUSIONROOMRATIO,
    EAXSOURCE_OCCLUSIONDIRECTRATIO,
    EAXSOURCE_EXCLUSION, 
    EAXSOURCE_EXCLUSIONLFRATIO,
    EAXSOURCE_OUTSIDEVOLUMEHF, 
    EAXSOURCE_DOPPLERFACTOR, 
    EAXSOURCE_ROLLOFFFACTOR, 
    EAXSOURCE_ROOMROLLOFFFACTOR,
    EAXSOURCE_AIRABSORPTIONFACTOR,
    EAXSOURCE_FLAGS,
    EAXSOURCE_SENDPARAMETERS,
    EAXSOURCE_ALLSENDPARAMETERS,
    EAXSOURCE_OCCLUSIONSENDPARAMETERS,
    EAXSOURCE_EXCLUSIONSENDPARAMETERS,
    EAXSOURCE_ACTIVEFXSLOTID,
} EAXSOURCE_PROPERTY;

// OR these flags with property id
#define EAXSOURCE_PARAMETER_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXSOURCE_PARAMETER_DEFERRED  0x80000000 // changes take effect later
#define EAXSOURCE_PARAMETER_COMMITDEFERREDSETTINGS (EAXSOURCE_NONE | \
                                                    EAXSOURCE_PARAMETER_IMMEDIATE)
// Used by EAXSOURCE_FLAGS for EAXSOURCEFLAGS_xxxAUTO
//    TRUE:    value is computed automatically - property is an offset
//    FALSE:   value is used directly
//
// Note: The number and order of flags may change in future EAX versions.
//       To insure future compatibility, use flag defines as follows:
//              myFlags = EAXSOURCE_DIRECTHFAUTO | EAXSOURCE_ROOMAUTO;
//       instead of:
//              myFlags = 0x00000003;
//
#define EAXSOURCEFLAGS_DIRECTHFAUTO          0x00000001 // relates to EAXSOURCE_DIRECTHF
#define EAXSOURCEFLAGS_ROOMAUTO              0x00000002 // relates to EAXSOURCE_ROOM
#define EAXSOURCEFLAGS_ROOMHFAUTO            0x00000004 // relates to EAXSOURCE_ROOMHF
#define EAXSOURCEFLAGS_RESERVED              0xFFFFFFF8 // reserved future use

// EAX Source property ranges and defaults:
#define EAXSOURCE_MINSEND                      (-10000)
#define EAXSOURCE_MAXSEND                      0
#define EAXSOURCE_DEFAULTSEND                  0

#define EAXSOURCE_MINSENDHF                    (-10000)
#define EAXSOURCE_MAXSENDHF                    0
#define EAXSOURCE_DEFAULTSENDHF                0

#define EAXSOURCE_MINDIRECT                    (-10000)
#define EAXSOURCE_MAXDIRECT                    1000
#define EAXSOURCE_DEFAULTDIRECT                0

#define EAXSOURCE_MINDIRECTHF                  (-10000)
#define EAXSOURCE_MAXDIRECTHF                  0
#define EAXSOURCE_DEFAULTDIRECTHF              0

#define EAXSOURCE_MINROOM                      (-10000)
#define EAXSOURCE_MAXROOM                      1000
#define EAXSOURCE_DEFAULTROOM                  0

#define EAXSOURCE_MINROOMHF                    (-10000)
#define EAXSOURCE_MAXROOMHF                    0
#define EAXSOURCE_DEFAULTROOMHF                0

#define EAXSOURCE_MINOBSTRUCTION               (-10000)
#define EAXSOURCE_MAXOBSTRUCTION               0
#define EAXSOURCE_DEFAULTOBSTRUCTION           0

#define EAXSOURCE_MINOBSTRUCTIONLFRATIO        0.0f
#define EAXSOURCE_MAXOBSTRUCTIONLFRATIO        1.0f
#define EAXSOURCE_DEFAULTOBSTRUCTIONLFRATIO    0.0f

#define EAXSOURCE_MINOCCLUSION                 (-10000)
#define EAXSOURCE_MAXOCCLUSION                 0
#define EAXSOURCE_DEFAULTOCCLUSION             0

#define EAXSOURCE_MINOCCLUSIONLFRATIO          0.0f
#define EAXSOURCE_MAXOCCLUSIONLFRATIO          1.0f
#define EAXSOURCE_DEFAULTOCCLUSIONLFRATIO      0.25f

#define EAXSOURCE_MINOCCLUSIONROOMRATIO        0.0f
#define EAXSOURCE_MAXOCCLUSIONROOMRATIO        10.0f
#define EAXSOURCE_DEFAULTOCCLUSIONROOMRATIO    1.5f

#define EAXSOURCE_MINOCCLUSIONDIRECTRATIO      0.0f
#define EAXSOURCE_MAXOCCLUSIONDIRECTRATIO      10.0f
#define EAXSOURCE_DEFAULTOCCLUSIONDIRECTRATIO  1.0f

#define EAXSOURCE_MINEXCLUSION                 (-10000)
#define EAXSOURCE_MAXEXCLUSION                 0
#define EAXSOURCE_DEFAULTEXCLUSION             0

#define EAXSOURCE_MINEXCLUSIONLFRATIO          0.0f
#define EAXSOURCE_MAXEXCLUSIONLFRATIO          1.0f
#define EAXSOURCE_DEFAULTEXCLUSIONLFRATIO      1.0f

#define EAXSOURCE_MINOUTSIDEVOLUMEHF           (-10000)
#define EAXSOURCE_MAXOUTSIDEVOLUMEHF           0
#define EAXSOURCE_DEFAULTOUTSIDEVOLUMEHF       0

#define EAXSOURCE_MINDOPPLERFACTOR             0.0f
#define EAXSOURCE_MAXDOPPLERFACTOR             10.f
#define EAXSOURCE_DEFAULTDOPPLERFACTOR         1.0f

#define EAXSOURCE_MINROLLOFFFACTOR             0.0f
#define EAXSOURCE_MAXROLLOFFFACTOR             10.f
#define EAXSOURCE_DEFAULTROLLOFFFACTOR         0.0f

#define EAXSOURCE_MINROOMROLLOFFFACTOR         0.0f
#define EAXSOURCE_MAXROOMROLLOFFFACTOR         10.f
#define EAXSOURCE_DEFAULTROOMROLLOFFFACTOR     0.0f

#define EAXSOURCE_MINAIRABSORPTIONFACTOR       0.0f
#define EAXSOURCE_MAXAIRABSORPTIONFACTOR       10.0f
#define EAXSOURCE_DEFAULTAIRABSORPTIONFACTOR   0.0f

#define EAXSOURCE_DEFAULTFLAGS                 (EAXSOURCEFLAGS_DIRECTHFAUTO |       \
                                                EAXSOURCEFLAGS_ROOMAUTO     |       \
                                                EAXSOURCEFLAGS_ROOMHFAUTO  )

#define EAXSOURCE_DEFAULTACTIVEFXSLOTID  {{ EAX_NULL_GUID.Data1, EAX_NULL_GUID.Data2, EAX_NULL_GUID.Data3, \
                                    EAX_NULL_GUID.Data4[0],EAX_NULL_GUID.Data4[1],EAX_NULL_GUID.Data4[2],\
                                    EAX_NULL_GUID.Data4[3],EAX_NULL_GUID.Data4[4],EAX_NULL_GUID.Data4[5],\
                                    EAX_NULL_GUID.Data4[6],EAX_NULL_GUID.Data4[7] },                       \
                                  { EAX_PrimaryFXSlotID.Data1, EAX_PrimaryFXSlotID.Data2, \
                                    EAX_PrimaryFXSlotID.Data3, EAX_PrimaryFXSlotID.Data4[0],\
                                    EAX_PrimaryFXSlotID.Data4[1],EAX_PrimaryFXSlotID.Data4[2],\
                                    EAX_PrimaryFXSlotID.Data4[3],EAX_PrimaryFXSlotID.Data4[4],\
                                    EAX_PrimaryFXSlotID.Data4[5],EAX_PrimaryFXSlotID.Data4[6],\
                                    EAX_PrimaryFXSlotID.Data4[7] }}


////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////
// Reverb Effect

// EAX REVERB {0CF95C8F-A3CC-4849-B0B6-832ECC1822DF}
DEFINE_GUID(EAX_REVERB_EFFECT, 
    0xcf95c8f, 
    0xa3cc, 
    0x4849, 
    0xb0, 0xb6, 0x83, 0x2e, 0xcc, 0x18, 0x22, 0xdf);

// Reverb effect properties
typedef enum
{
    EAXREVERB_NONE,
    EAXREVERB_ALLPARAMETERS,
    EAXREVERB_ENVIRONMENT,
    EAXREVERB_ENVIRONMENTSIZE,
    EAXREVERB_ENVIRONMENTDIFFUSION,
    EAXREVERB_ROOM,
    EAXREVERB_ROOMHF,
    EAXREVERB_ROOMLF,
    EAXREVERB_DECAYTIME,
    EAXREVERB_DECAYHFRATIO,
    EAXREVERB_DECAYLFRATIO,
    EAXREVERB_REFLECTIONS,
    EAXREVERB_REFLECTIONSDELAY,
    EAXREVERB_REFLECTIONSPAN,
    EAXREVERB_REVERB,
    EAXREVERB_REVERBDELAY,
    EAXREVERB_REVERBPAN,
    EAXREVERB_ECHOTIME,
    EAXREVERB_ECHODEPTH,
    EAXREVERB_MODULATIONTIME,
    EAXREVERB_MODULATIONDEPTH,
    EAXREVERB_AIRABSORPTIONHF,
    EAXREVERB_HFREFERENCE,
    EAXREVERB_LFREFERENCE,
    EAXREVERB_ROOMROLLOFFFACTOR,
    EAXREVERB_FLAGS,
} EAXREVERB_PROPERTY;

// OR these flags with property id
#define EAXREVERB_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXREVERB_DEFERRED  0x80000000 // changes take effect later
#define EAXREVERB_COMMITDEFERREDSETTINGS (EAXREVERB_NONE | \
                                          EAXREVERB_IMMEDIATE)

// used by EAXREVERB_ENVIRONMENT
#ifndef EAX_ENVIRONMENTS_DEFINED
#define EAX_ENVIRONMENTS_DEFINED
enum
{
    EAX_ENVIRONMENT_GENERIC,
    EAX_ENVIRONMENT_PADDEDCELL,
    EAX_ENVIRONMENT_ROOM,
    EAX_ENVIRONMENT_BATHROOM,
    EAX_ENVIRONMENT_LIVINGROOM,
    EAX_ENVIRONMENT_STONEROOM,
    EAX_ENVIRONMENT_AUDITORIUM,
    EAX_ENVIRONMENT_CONCERTHALL,
    EAX_ENVIRONMENT_CAVE,
    EAX_ENVIRONMENT_ARENA,
    EAX_ENVIRONMENT_HANGAR,
    EAX_ENVIRONMENT_CARPETEDHALLWAY,
    EAX_ENVIRONMENT_HALLWAY,
    EAX_ENVIRONMENT_STONECORRIDOR,
    EAX_ENVIRONMENT_ALLEY,
    EAX_ENVIRONMENT_FOREST,
    EAX_ENVIRONMENT_CITY,
    EAX_ENVIRONMENT_MOUNTAINS,
    EAX_ENVIRONMENT_QUARRY,
    EAX_ENVIRONMENT_PLAIN,
    EAX_ENVIRONMENT_PARKINGLOT,
    EAX_ENVIRONMENT_SEWERPIPE,
    EAX_ENVIRONMENT_UNDERWATER,
    EAX_ENVIRONMENT_DRUGGED,
    EAX_ENVIRONMENT_DIZZY,
    EAX_ENVIRONMENT_PSYCHOTIC,

    EAX_ENVIRONMENT_UNDEFINED,

    EAX_ENVIRONMENT_COUNT
};
#endif

// Used by EAXREVERB_FLAGS
//
// Note: The number and order of flags may change in future EAX versions.
//       It is recommended to use the flag defines as follows:
//              myFlags = EAXREVERBFLAGS_DECAYTIMESCALE | EAXREVERBFLAGS_REVERBSCALE;
//       instead of:
//              myFlags = 0x00000009;
//
// These flags determine what properties are affected by environment size.
#define EAXREVERBFLAGS_DECAYTIMESCALE        0x00000001 // reverberation decay time
#define EAXREVERBFLAGS_REFLECTIONSSCALE      0x00000002 // reflection level
#define EAXREVERBFLAGS_REFLECTIONSDELAYSCALE 0x00000004 // initial reflection delay time
#define EAXREVERBFLAGS_REVERBSCALE           0x00000008 // reflections level
#define EAXREVERBFLAGS_REVERBDELAYSCALE      0x00000010 // late reverberation delay time
#define EAXREVERBFLAGS_ECHOTIMESCALE         0x00000040 // echo time
#define EAXREVERBFLAGS_MODULATIONTIMESCALE   0x00000080 // modulation time
// This flag limits high-frequency decay time according to air absorption.
#define EAXREVERBFLAGS_DECAYHFLIMIT          0x00000020
#define EAXREVERBFLAGS_RESERVED              0xFFFFFF00 // reserved future use

// Use this structure for EAXREVERB_ALLPARAMETERS
// - all levels are hundredths of decibels
// - all times and delays are in seconds
//
// NOTE: This structure may change in future EAX versions.
//       It is recommended to initialize fields by name:
//              myReverb.lRoom = -1000;
//              myReverb.lRoomHF = -100;
//              ...
//              myReverb.dwFlags = myFlags /* see EAXREVERBFLAGS below */ ;
//       instead of:
//              myReverb = { -1000, -100, ... , 0x00000009 };
//       If you want to save and load presets in binary form, you 
//       should define your own structure to insure future compatibility.
//
typedef struct _EAXREVERBPROPERTIES
{
    unsigned long ulEnvironment;   // sets all reverb properties
    float flEnvironmentSize;       // environment size in meters
    float flEnvironmentDiffusion;  // environment diffusion
    long lRoom;                    // room effect level (at mid frequencies)
    long lRoomHF;                  // relative room effect level at high frequencies
    long lRoomLF;                  // relative room effect level at low frequencies  
    float flDecayTime;             // reverberation decay time at mid frequencies
    float flDecayHFRatio;          // high-frequency to mid-frequency decay time ratio
    float flDecayLFRatio;          // low-frequency to mid-frequency decay time ratio   
    long lReflections;             // early reflections level relative to room effect
    float flReflectionsDelay;      // initial reflection delay time
    EAXVECTOR vReflectionsPan;     // early reflections panning vector
    long lReverb;                  // late reverberation level relative to room effect
    float flReverbDelay;           // late reverberation delay time relative to initial reflection
    EAXVECTOR vReverbPan;          // late reverberation panning vector
    float flEchoTime;              // echo time
    float flEchoDepth;             // echo depth
    float flModulationTime;        // modulation time
    float flModulationDepth;       // modulation depth
    float flAirAbsorptionHF;       // change in level per meter at high frequencies
    float flHFReference;           // reference high frequency
    float flLFReference;           // reference low frequency 
    float flRoomRolloffFactor;     // like DS3D flRolloffFactor but for room effect
    unsigned long ulFlags;         // modifies the behavior of properties
} EAXREVERBPROPERTIES, *LPEAXREVERBPROPERTIES;

// Property ranges and defaults:
#define EAXREVERB_MINENVIRONMENT                0
#define EAXREVERB_MAXENVIRONMENT               (EAX_ENVIRONMENT_COUNT-1)
#define EAXREVERB_DEFAULTENVIRONMENT            EAX_ENVIRONMENT_GENERIC

#define EAXREVERB_MINENVIRONMENTSIZE            1.0f
#define EAXREVERB_MAXENVIRONMENTSIZE            100.0f
#define EAXREVERB_DEFAULTENVIRONMENTSIZE        7.5f

#define EAXREVERB_MINENVIRONMENTDIFFUSION       0.0f
#define EAXREVERB_MAXENVIRONMENTDIFFUSION       1.0f
#define EAXREVERB_DEFAULTENVIRONMENTDIFFUSION   1.0f

#define EAXREVERB_MINROOM                       (-10000)
#define EAXREVERB_MAXROOM                       0
#define EAXREVERB_DEFAULTROOM                   (-1000)

#define EAXREVERB_MINROOMHF                     (-10000)
#define EAXREVERB_MAXROOMHF                     0
#define EAXREVERB_DEFAULTROOMHF                 (-100)

#define EAXREVERB_MINROOMLF                     (-10000)
#define EAXREVERB_MAXROOMLF                     0
#define EAXREVERB_DEFAULTROOMLF                 0

#define EAXREVERB_MINDECAYTIME                  0.1f
#define EAXREVERB_MAXDECAYTIME                  20.0f
#define EAXREVERB_DEFAULTDECAYTIME              1.49f

#define EAXREVERB_MINDECAYHFRATIO               0.1f
#define EAXREVERB_MAXDECAYHFRATIO               2.0f
#define EAXREVERB_DEFAULTDECAYHFRATIO           0.83f

#define EAXREVERB_MINDECAYLFRATIO               0.1f
#define EAXREVERB_MAXDECAYLFRATIO               2.0f
#define EAXREVERB_DEFAULTDECAYLFRATIO           1.00f

#define EAXREVERB_MINREFLECTIONS                (-10000)
#define EAXREVERB_MAXREFLECTIONS                1000
#define EAXREVERB_DEFAULTREFLECTIONS            (-2602)

#define EAXREVERB_MINREFLECTIONSDELAY           0.0f
#define EAXREVERB_MAXREFLECTIONSDELAY           0.3f
#define EAXREVERB_DEFAULTREFLECTIONSDELAY       0.007f

#define EAXREVERB_DEFAULTREFLECTIONSPAN         {0.0f, 0.0f, 0.0f}

#define EAXREVERB_MINREVERB                     (-10000)
#define EAXREVERB_MAXREVERB                     2000
#define EAXREVERB_DEFAULTREVERB                 200

#define EAXREVERB_MINREVERBDELAY                0.0f
#define EAXREVERB_MAXREVERBDELAY                0.1f
#define EAXREVERB_DEFAULTREVERBDELAY            0.011f

#define EAXREVERB_DEFAULTREVERBPAN              {0.0f, 0.0f, 0.0f}

#define EAXREVERB_MINECHOTIME                   0.075f
#define EAXREVERB_MAXECHOTIME                      0.25f
#define EAXREVERB_DEFAULTECHOTIME               0.25f

#define EAXREVERB_MINECHODEPTH                  0.0f
#define EAXREVERB_MAXECHODEPTH                  1.0f
#define EAXREVERB_DEFAULTECHODEPTH              0.0f

#define EAXREVERB_MINMODULATIONTIME             0.04f
#define EAXREVERB_MAXMODULATIONTIME             4.0f
#define EAXREVERB_DEFAULTMODULATIONTIME         0.25f

#define EAXREVERB_MINMODULATIONDEPTH            0.0f
#define EAXREVERB_MAXMODULATIONDEPTH            1.0f
#define EAXREVERB_DEFAULTMODULATIONDEPTH        0.0f

#define EAXREVERB_MINAIRABSORPTIONHF            (-100.0f)
#define EAXREVERB_MAXAIRABSORPTIONHF            0.0f
#define EAXREVERB_DEFAULTAIRABSORPTIONHF        (-5.0f)

#define EAXREVERB_MINHFREFERENCE                1000.0f
#define EAXREVERB_MAXHFREFERENCE                20000.0f
#define EAXREVERB_DEFAULTHFREFERENCE            5000.0f

#define EAXREVERB_MINLFREFERENCE                20.0f
#define EAXREVERB_MAXLFREFERENCE                1000.0f
#define EAXREVERB_DEFAULTLFREFERENCE            250.0f

#define EAXREVERB_MINROOMROLLOFFFACTOR          0.0f
#define EAXREVERB_MAXROOMROLLOFFFACTOR          10.0f
#define EAXREVERB_DEFAULTROOMROLLOFFFACTOR      0.0f

#define EAXREVERB_DEFAULTFLAGS                  (EAXREVERBFLAGS_DECAYTIMESCALE |        \
                                                 EAXREVERBFLAGS_REFLECTIONSSCALE |      \
                                                 EAXREVERBFLAGS_REFLECTIONSDELAYSCALE | \
                                                 EAXREVERBFLAGS_REVERBSCALE |           \
                                                 EAXREVERBFLAGS_REVERBDELAYSCALE |      \
                                                 EAXREVERBFLAGS_DECAYHFLIMIT)
////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////

//          New Effect Types

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// AGC Compressor Effect

// EAX AGC COMPRESSOR {BFB7A01E-7825-4039-927F-3AABDA0C560}

DEFINE_GUID(EAX_AGCCOMPRESSOR_EFFECT,
    0xbfb7a01e,
    0x7825,
    0x4039,
    0x92, 0x7f, 0x3, 0xaa, 0xbd, 0xa0, 0xc5, 0x60);

// AGC Compressor properties
typedef enum
{
    EAXAGCCOMPRESSOR_NONE,
    EAXAGCCOMPRESSOR_ALLPARAMETERS,
    EAXAGCCOMPRESSOR_ONOFF
} EAXAGCCOMPRESSOR_PROPERTY;

// OR these flags with property id
#define EAXAGCCOMPRESSOR_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXAGCCOMPRESSOR_DEFERRED  0x80000000 // changes take effect later
#define EAXAGCCOMPRESSOR_COMMITDEFERREDSETTINGS (EAXAGCCOMPRESSOR_NONE | \
                                                 EAXAGCCOMPRESSOR_IMMEDIATE)

// Use this structure for EAXAGCCOMPRESSOR_ALLPARAMETERS
typedef struct _EAXAGCCOMPRESSORPROPERTIES
{
    unsigned long ulOnOff;   // Switch Compressor on or off
} EAXAGCCOMPRESSORPROPERTIES, *LPEAXAGCCOMPRESSORPROPERTIES;

// Property ranges and defaults:

#define EAXAGCCOMPRESSOR_MINONOFF           0
#define EAXAGCCOMPRESSOR_MAXONOFF           1
#define EAXAGCCOMPRESSOR_DEFAULTONOFF       1

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Autowah Effect

// EAX AUTOWAH {EC3130C0-AC7A-11D2-88DD-A024D13CE1}
DEFINE_GUID(EAX_AUTOWAH_EFFECT, 
    0xec3130c0,
    0xac7a,
    0x11d2,
    0x88, 0xdd, 0x0, 0xa0, 0x24, 0xd1, 0x3c, 0xe1);

// Autowah properties
typedef enum
{
    EAXAUTOWAH_NONE,
    EAXAUTOWAH_ALLPARAMETERS,
    EAXAUTOWAH_ATTACKTIME,
    EAXAUTOWAH_RELEASETIME,
    EAXAUTOWAH_RESONANCE,
    EAXAUTOWAH_PEAKLEVEL
} EAXAUTOWAH_PROPERTY;

// OR these flags with property id
#define EAXAUTOWAH_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXAUTOWAH_DEFERRED  0x80000000 // changes take effect later
#define EAXAUTOWAH_COMMITDEFERREDSETTINGS (EAXAUTOWAH_NONE | \
                                           EAXAUTOWAH_IMMEDIATE)

// Use this structure for EAXAUTOWAH_ALLPARAMETERS
typedef struct _EAXAUTOWAHPROPERTIES
{
    float   flAttackTime;                // Attack time (seconds)
    float   flReleaseTime;          // Release time (seconds)
    long    lResonance;             // Resonance (mB)
    long    lPeakLevel;             // Peak level (mB)
} EAXAUTOWAHPROPERTIES, *LPEAXAUTOWAHPROPERTIES;

// Property ranges and defaults:

#define EAXAUTOWAH_MINATTACKTIME            0.0001f 
#define EAXAUTOWAH_MAXATTACKTIME            1.0f
#define EAXAUTOWAH_DEFAULTATTACKTIME        0.06f

#define EAXAUTOWAH_MINRELEASETIME           0.0001f
#define EAXAUTOWAH_MAXRELEASETIME           1.0f
#define EAXAUTOWAH_DEFAULTRELEASETIME       0.06f

#define EAXAUTOWAH_MINRESONANCE             600     
#define EAXAUTOWAH_MAXRESONANCE             6000
#define EAXAUTOWAH_DEFAULTRESONANCE         6000

#define EAXAUTOWAH_MINPEAKLEVEL             (-9000)
#define EAXAUTOWAH_MAXPEAKLEVEL             9000
#define EAXAUTOWAH_DEFAULTPEAKLEVEL         2100

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Chorus Effect

// EAX CHORUS {DE6D6FE0-AC79-11D2-88DD-A024D13CE1}

DEFINE_GUID(EAX_CHORUS_EFFECT,
    0xde6d6fe0,
    0xac79,
    0x11d2,
    0x88, 0xdd, 0x0, 0xa0, 0x24, 0xd1, 0x3c, 0xe1);


// Chorus properties
typedef enum
{
    EAXCHORUS_NONE,
    EAXCHORUS_ALLPARAMETERS,
    EAXCHORUS_WAVEFORM,
    EAXCHORUS_PHASE,
    EAXCHORUS_RATE,
    EAXCHORUS_DEPTH,
    EAXCHORUS_FEEDBACK,
    EAXCHORUS_DELAY
} EAXCHORUS_PROPERTY;

// OR these flags with property id
#define EAXCHORUS_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXCHORUS_DEFERRED  0x80000000 // changes take effect later
#define EAXCHORUS_COMMITDEFERREDSETTINGS (EAXCHORUS_NONE | \
                                          EAXCHORUS_IMMEDIATE)

// used by EAXCHORUS_WAVEFORM
enum
{
    EAX_CHORUS_SINUSOID,
    EAX_CHORUS_TRIANGLE
};

// Use this structure for EAXCHORUS_ALLPARAMETERS
typedef struct _EAXCHORUSPROPERTIES
{
    unsigned long   ulWaveform;      // Waveform selector - see enum above
    long            lPhase;         // Phase (Degrees)
    float           flRate;         // Rate (Hz)
    float           flDepth;        // Depth (0 to 1)
    float           flFeedback;     // Feedback (-1 to 1)
    float           flDelay;        // Delay (seconds)
} EAXCHORUSPROPERTIES, *LPEAXCHORUSPROPERTIES;

// Property ranges and defaults:

#define EAXCHORUS_MINWAVEFORM               0
#define EAXCHORUS_MAXWAVEFORM               1
#define EAXCHORUS_DEFAULTWAVEFORM           1

#define EAXCHORUS_MINPHASE              (-180)
#define EAXCHORUS_MAXPHASE              180
#define EAXCHORUS_DEFAULTPHASE          90

#define EAXCHORUS_MINRATE               0.0f
#define EAXCHORUS_MAXRATE               10.0f
#define EAXCHORUS_DEFAULTRATE           1.1f

#define EAXCHORUS_MINDEPTH              0.0f
#define EAXCHORUS_MAXDEPTH              1.0f
#define EAXCHORUS_DEFAULTDEPTH          0.1f

#define EAXCHORUS_MINFEEDBACK           (-1.0f)
#define EAXCHORUS_MAXFEEDBACK           1.0f
#define EAXCHORUS_DEFAULTFEEDBACK       0.25f

#define EAXCHORUS_MINDELAY              0.0f
#define EAXCHORUS_MAXDELAY              0.016f
#define EAXCHORUS_DEFAULTDELAY          0.016f

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Distortion Effect

// EAX DISTORTION {975A4CE0-AC7E-11D2-88DD-A024D13CE1}

DEFINE_GUID(EAX_DISTORTION_EFFECT,
    0x975a4ce0,
    0xac7e,
    0x11d2,
    0x88, 0xdd, 0x0, 0xa0, 0x24, 0xd1, 0x3c, 0xe1);

// Distortion properties
typedef enum
{
    EAXDISTORTION_NONE,
    EAXDISTORTION_ALLPARAMETERS,
    EAXDISTORTION_EDGE,
    EAXDISTORTION_GAIN,
    EAXDISTORTION_LOWPASSCUTOFF,
    EAXDISTORTION_EQCENTER,
    EAXDISTORTION_EQBANDWIDTH
} EAXDISTORTION_PROPERTY;

// OR these flags with property id
#define EAXDISTORTION_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXDISTORTION_DEFERRED  0x80000000 // changes take effect later
#define EAXDISTORTION_COMMITDEFERREDSETTINGS (EAXDISTORTION_NONE | \
                                              EAXDISTORTION_IMMEDIATE)

// Use this structure for EAXDISTORTION_ALLPARAMETERS
typedef struct _EAXDISTORTIONPROPERTIES
{
    float   flEdge;             // Controls the shape of the distortion (0 to 1)
    long    lGain;              // Controls the post distortion gain (mB)
    float   flLowPassCutOff;    // Controls the cut-off of the filter pre-distortion (Hz)
    float   flEQCenter;         // Controls the center frequency of the EQ post-distortion (Hz)
    float   flEQBandwidth;      // Controls the bandwidth of the EQ post-distortion (Hz)
} EAXDISTORTIONPROPERTIES, *LPEAXDISTORTIONPROPERTIES;

// Property ranges and defaults:

#define EAXDISTORTION_MINEDGE               0.0f
#define EAXDISTORTION_MAXEDGE               1.0f
#define EAXDISTORTION_DEFAULTEDGE           0.2f

#define EAXDISTORTION_MINGAIN               (-6000)
#define EAXDISTORTION_MAXGAIN               0
#define EAXDISTORTION_DEFAULTGAIN           (-2600)

#define EAXDISTORTION_MINLOWPASSCUTOFF      80.0f
#define EAXDISTORTION_MAXLOWPASSCUTOFF      24000.0f
#define EAXDISTORTION_DEFAULTLOWPASSCUTOFF  8000.0f

#define EAXDISTORTION_MINEQCENTER           80.0f
#define EAXDISTORTION_MAXEQCENTER           24000.0f
#define EAXDISTORTION_DEFAULTEQCENTER       3600.0f

#define EAXDISTORTION_MINEQBANDWIDTH        80.0f
#define EAXDISTORTION_MAXEQBANDWIDTH        24000.0f
#define EAXDISTORTION_DEFAULTEQBANDWIDTH    3600.0f

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Echo Effect

// EAX ECHO {E9F1BC0-AC82-11D2-88DD-A024D13CE1}

DEFINE_GUID(EAX_ECHO_EFFECT,
    0xe9f1bc0,
    0xac82,
    0x11d2,
    0x88, 0xdd, 0x0, 0xa0, 0x24, 0xd1, 0x3c, 0xe1);

// Echo properties
typedef enum
{
    EAXECHO_NONE,
    EAXECHO_ALLPARAMETERS,
    EAXECHO_DELAY,
    EAXECHO_LRDELAY,
    EAXECHO_DAMPING,
    EAXECHO_FEEDBACK,
    EAXECHO_SPREAD
} EAXECHO_PROPERTY;

// OR these flags with property id
#define EAXECHO_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXECHO_DEFERRED  0x80000000 // changes take effect later
#define EAXECHO_COMMITDEFERREDSETTINGS (EAXECHO_NONE | \
                                        EAXECHO_IMMEDIATE)

// Use this structure for EAXECHO_ALLPARAMETERS
typedef struct _EAXECHOPROPERTIES
{
    float   flDelay;            // Controls the initial delay time (seconds)
    float   flLRDelay;          // Controls the delay time between the first and second taps (seconds)
    float   flDamping;          // Controls a low-pass filter that dampens the echoes (0 to 1)
    float   flFeedback;         // Controls the duration of echo repetition (0 to 1)
    float   flSpread;           // Controls the left-right spread of the echoes
} EAXECHOPROPERTIES, *LPEAXECHOPROPERTIES;

// Property ranges and defaults:
                    
#define EAXECHO_MINDAMPING          0.0f
#define EAXECHO_MAXDAMPING          0.99f
#define EAXECHO_DEFAULTDAMPING      0.5f

#define EAXECHO_MINDELAY            0.0f
#define EAXECHO_MAXDELAY            0.207f
#define EAXECHO_DEFAULTDELAY        0.1f

#define EAXECHO_MINLRDELAY          0.0f
#define EAXECHO_MAXLRDELAY          0.404f
#define EAXECHO_DEFAULTLRDELAY      0.1f

#define EAXECHO_MINFEEDBACK         0.0f
#define EAXECHO_MAXFEEDBACK         1.0f
#define EAXECHO_DEFAULTFEEDBACK     0.5f

#define EAXECHO_MINSPREAD           (-1.0f)
#define EAXECHO_MAXSPREAD             1.0f
#define EAXECHO_DEFAULTSPREAD       (-1.0f)

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Equalizer Effect

// EAX EQUALIZER {65F94CE0-9793-11D3-939D-C0F02DD6F0}

DEFINE_GUID(EAX_EQUALIZER_EFFECT,
    0x65f94ce0,
    0x9793,
    0x11d3,
    0x93, 0x9d, 0x0, 0xc0, 0xf0, 0x2d, 0xd6, 0xf0);


// Equalizer properties
typedef enum
{
    EAXEQUALIZER_NONE,
    EAXEQUALIZER_ALLPARAMETERS,
    EAXEQUALIZER_LOWGAIN,
    EAXEQUALIZER_LOWCUTOFF,
    EAXEQUALIZER_MID1GAIN,
    EAXEQUALIZER_MID1CENTER,
    EAXEQUALIZER_MID1WIDTH,
    EAXEQUALIZER_MID2GAIN,
    EAXEQUALIZER_MID2CENTER,
    EAXEQUALIZER_MID2WIDTH,
    EAXEQUALIZER_HIGHGAIN,
    EAXEQUALIZER_HIGHCUTOFF
} EAXEQUALIZER_PROPERTY;

// OR these flags with property id
#define EAXEQUALIZER_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXEQUALIZER_DEFERRED  0x80000000 // changes take effect later
#define EAXEQUALIZER_COMMITDEFERREDSETTINGS (EAXEQUALIZER_NONE | \
                                             EAXEQUALIZER_IMMEDIATE)

// Use this structure for EAXEQUALIZER_ALLPARAMETERS
typedef struct _EAXEQUALIZERPROPERTIES
{
    long    lLowGain;           // (mB)
    float   flLowCutOff;        // (Hz)
    long    lMid1Gain;          // (mB)
    float   flMid1Center;       // (Hz)
    float   flMid1Width;        // (octaves)
    long    lMid2Gain;          // (mB)
    float   flMid2Center;       // (Hz)
    float   flMid2Width;        // (octaves)
    long    lHighGain;          // (mB)
    float   flHighCutOff;       // (Hz)
} EAXEQUALIZERPROPERTIES, *LPEAXEQUALIZERPROPERTIES;

// Property ranges and defaults:

#define EAXEQUALIZER_MINLOWGAIN         (-1800)
#define EAXEQUALIZER_MAXLOWGAIN         1800
#define EAXEQUALIZER_DEFAULTLOWGAIN     0

#define EAXEQUALIZER_MINLOWCUTOFF       50.0f
#define EAXEQUALIZER_MAXLOWCUTOFF       800.0f
#define EAXEQUALIZER_DEFAULTLOWCUTOFF   200.0f

#define EAXEQUALIZER_MINMID1GAIN        (-1800)
#define EAXEQUALIZER_MAXMID1GAIN        1800
#define EAXEQUALIZER_DEFAULTMID1GAIN    0

#define EAXEQUALIZER_MINMID1CENTER      200.0f
#define EAXEQUALIZER_MAXMID1CENTER      3000.0f
#define EAXEQUALIZER_DEFAULTMID1CENTER  500.0f

#define EAXEQUALIZER_MINMID1WIDTH       0.01f
#define EAXEQUALIZER_MAXMID1WIDTH       1.0f
#define EAXEQUALIZER_DEFAULTMID1WIDTH   1.0f

#define EAXEQUALIZER_MINMID2GAIN        (-1800)
#define EAXEQUALIZER_MAXMID2GAIN        1800
#define EAXEQUALIZER_DEFAULTMID2GAIN    0

#define EAXEQUALIZER_MINMID2CENTER      1000.0f
#define EAXEQUALIZER_MAXMID2CENTER      8000.0f
#define EAXEQUALIZER_DEFAULTMID2CENTER  3000.0f

#define EAXEQUALIZER_MINMID2WIDTH       0.01f
#define EAXEQUALIZER_MAXMID2WIDTH       1.0f
#define EAXEQUALIZER_DEFAULTMID2WIDTH   1.0f

#define EAXEQUALIZER_MINHIGHGAIN        (-1800)
#define EAXEQUALIZER_MAXHIGHGAIN        1800
#define EAXEQUALIZER_DEFAULTHIGHGAIN    0

#define EAXEQUALIZER_MINHIGHCUTOFF      4000.0f
#define EAXEQUALIZER_MAXHIGHCUTOFF      16000.0f
#define EAXEQUALIZER_DEFAULTHIGHCUTOFF  6000.0f

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Flanger Effect

// EAX FLANGER {A70007C0-7D2-11D3-9B1E-A024D13CE1}

DEFINE_GUID(EAX_FLANGER_EFFECT,
    0xa70007c0,
    0x7d2,
    0x11d3,
    0x9b, 0x1e, 0x0, 0xa0, 0x24, 0xd1, 0x3c, 0xe1);

// Flanger properties
typedef enum
{
    EAXFLANGER_NONE,
    EAXFLANGER_ALLPARAMETERS,
    EAXFLANGER_WAVEFORM,
    EAXFLANGER_PHASE,
    EAXFLANGER_RATE,
    EAXFLANGER_DEPTH,
    EAXFLANGER_FEEDBACK,
    EAXFLANGER_DELAY
} EAXFLANGER_PROPERTY;

// OR these flags with property id
#define EAXFLANGER_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXFLANGER_DEFERRED  0x80000000 // changes take effect later
#define EAXFLANGER_COMMITDEFERREDSETTINGS (EAXFLANGER_NONE | \
                                           EAXFLANGER_IMMEDIATE)

// used by EAXFLANGER_WAVEFORM
enum
{
    EAX_FLANGER_SINUSOID,
    EAX_FLANGER_TRIANGLE
};

// Use this structure for EAXFLANGER_ALLPARAMETERS
typedef struct _EAXFLANGERPROPERTIES
{
    unsigned long   ulWaveform;  // Waveform selector - see enum above
    long            lPhase;     // Phase (Degrees)
    float           flRate;     // Rate (Hz)
    float           flDepth;    // Depth (0 to 1)
    float           flFeedback; // Feedback (0 to 1)
    float           flDelay;    // Delay (seconds)
} EAXFLANGERPROPERTIES, *LPEAXFLANGERPROPERTIES;

// Property ranges and defaults:

#define EAXFLANGER_MINWAVEFORM              0
#define EAXFLANGER_MAXWAVEFORM              1
#define EAXFLANGER_DEFAULTWAVEFORM          1

#define EAXFLANGER_MINPHASE             (-180)
#define EAXFLANGER_MAXPHASE             180
#define EAXFLANGER_DEFAULTPHASE         0

#define EAXFLANGER_MINRATE              0.0f
#define EAXFLANGER_MAXRATE              10.0f
#define EAXFLANGER_DEFAULTRATE          0.27f

#define EAXFLANGER_MINDEPTH             0.0f
#define EAXFLANGER_MAXDEPTH             1.0f
#define EAXFLANGER_DEFAULTDEPTH         1.0f

#define EAXFLANGER_MINFEEDBACK          (-1.0f)
#define EAXFLANGER_MAXFEEDBACK          1.0f
#define EAXFLANGER_DEFAULTFEEDBACK      (-0.5f)

#define EAXFLANGER_MINDELAY             0.0f
#define EAXFLANGER_MAXDELAY             0.004f
#define EAXFLANGER_DEFAULTDELAY         0.002f

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Frequency Shifter Effect

// EAX FREQUENCY SHIFTER {DC3E1880-9212-11D3-939D-C0F02DD6F0}

DEFINE_GUID(EAX_FREQUENCYSHIFTER_EFFECT,
    0xdc3e1880,
    0x9212,
    0x11d3,
    0x93, 0x9d, 0x0, 0xc0, 0xf0, 0x2d, 0xd6, 0xf0);

// Frequency Shifter properties
typedef enum
{
    EAXFREQUENCYSHIFTER_NONE,
    EAXFREQUENCYSHIFTER_ALLPARAMETERS,
    EAXFREQUENCYSHIFTER_FREQUENCY,
    EAXFREQUENCYSHIFTER_LEFTDIRECTION,
    EAXFREQUENCYSHIFTER_RIGHTDIRECTION
} EAXFREQUENCYSHIFTER_PROPERTY;

// OR these flags with property id
#define EAXFREQUENCYSHIFTER_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXFREQUENCYSHIFTER_DEFERRED  0x80000000 // changes take effect later
#define EAXFREQUENCYSHIFTER_COMMITDEFERREDSETTINGS (EAXFREQUENCYSHIFTER_NONE | \
                                                    EAXFREQUENCYSHIFTER_IMMEDIATE)

// used by EAXFREQUENCYSHIFTER_LEFTDIRECTION and EAXFREQUENCYSHIFTER_RIGHTDIRECTION 
enum
{
    EAX_FREQUENCYSHIFTER_DOWN,
    EAX_FREQUENCYSHIFTER_UP,
    EAX_FREQUENCYSHIFTER_OFF
};

// Use this structure for EAXFREQUENCYSHIFTER_ALLPARAMETERS
typedef struct _EAXFREQUENCYSHIFTERPROPERTIES
{
    float           flFrequency;        // (Hz)
    unsigned long   ulLeftDirection;     // see enum above
    unsigned long   ulRightDirection;    // see enum above
} EAXFREQUENCYSHIFTERPROPERTIES, *LPEAXFREQUENCYSHIFTERPROPERTIES;

// Property ranges and defaults:

#define EAXFREQUENCYSHIFTER_MINFREQUENCY            0.0f
#define EAXFREQUENCYSHIFTER_MAXFREQUENCY            24000.0f
#define EAXFREQUENCYSHIFTER_DEFAULTFREQUENCY        0.0f

#define EAXFREQUENCYSHIFTER_MINLEFTDIRECTION        0
#define EAXFREQUENCYSHIFTER_MAXLEFTDIRECTION        2
#define EAXFREQUENCYSHIFTER_DEFAULTLEFTDIRECTION    0

#define EAXFREQUENCYSHIFTER_MINRIGHTDIRECTION       0
#define EAXFREQUENCYSHIFTER_MAXRIGHTDIRECTION       2
#define EAXFREQUENCYSHIFTER_DEFAULTRIGHTDIRECTION   0

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Vocal Morpher Effect

// EAX VOCAL MORPHER {E41CF10C-3383-11D2-88DD-A024D13CE1}

DEFINE_GUID(EAX_VOCALMORPHER_EFFECT, 
    0xe41cf10c,
    0x3383,
    0x11d2,
    0x88, 0xdd, 0x0, 0xa0, 0x24, 0xd1, 0x3c, 0xe1);

// Vocal Morpher properties
typedef enum
{
    EAXVOCALMORPHER_NONE,
    EAXVOCALMORPHER_ALLPARAMETERS,
    EAXVOCALMORPHER_PHONEMEA,
    EAXVOCALMORPHER_PHONEMEACOARSETUNING,
    EAXVOCALMORPHER_PHONEMEB,
    EAXVOCALMORPHER_PHONEMEBCOARSETUNING,
    EAXVOCALMORPHER_WAVEFORM,
    EAXVOCALMORPHER_RATE
} EAXVOCALMORPHER_PROPERTY;

// OR these flags with property id
#define EAXVOCALMORPHER_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXVOCALMORPHER_DEFERRED  0x80000000 // changes take effect later
#define EAXVOCALMORPHER_COMMITDEFERREDSETTINGS (EAXVOCALMORPHER_NONE | \
                                                EAXVOCALMORPHER_IMMEDIATE)

// used by EAXVOCALMORPHER_PHONEMEA and EAXVOCALMORPHER_PHONEMEB
enum
{
    A, E, I, O, U, AA, AE, AH, AO, EH, ER, IH, IY, UH, UW, B, D, F, G,
    J, K, L, M, N, P, R, S, T, V, Z
};

// used by EAXVOCALMORPHER_WAVEFORM
enum
{
    EAX_VOCALMORPHER_SINUSOID,
    EAX_VOCALMORPHER_TRIANGLE,
    EAX_VOCALMORPHER_SAWTOOTH
};

// Use this structure for EAXVOCALMORPHER_ALLPARAMETERS
typedef struct _EAXVOCALMORPHERPROPERTIES
{
    unsigned long   ulPhonemeA;              // see enum above
    long            lPhonemeACoarseTuning;  // (semitones)
    unsigned long   ulPhonemeB;              // see enum above
    long            lPhonemeBCoarseTuning;  // (semitones)
    unsigned long   ulWaveform;              // Waveform selector - see enum above
    float           flRate;                 // (Hz)
} EAXVOCALMORPHERPROPERTIES, *LPEAXVOCALMORPHERPROPERTIES;

// Property ranges and defaults:

#define EAXVOCALMORPHER_MINPHONEMEA                 0
#define EAXVOCALMORPHER_MAXPHONEMEA                 29
#define EAXVOCALMORPHER_DEFAULTPHONEMEA             0

#define EAXVOCALMORPHER_MINPHONEMEACOARSETUNING     (-24)
#define EAXVOCALMORPHER_MAXPHONEMEACOARSETUNING     24
#define EAXVOCALMORPHER_DEFAULTPHONEMEACOARSETUNING 0

#define EAXVOCALMORPHER_MINPHONEMEB                 0
#define EAXVOCALMORPHER_MAXPHONEMEB                 29
#define EAXVOCALMORPHER_DEFAULTPHONEMEB             10

#define EAXVOCALMORPHER_MINPHONEMEBCOARSETUNING     (-24)
#define EAXVOCALMORPHER_MAXPHONEMEBCOARSETUNING     24
#define EAXVOCALMORPHER_DEFAULTPHONEMEBCOARSETUNING 0

#define EAXVOCALMORPHER_MINWAVEFORM                 0
#define EAXVOCALMORPHER_MAXWAVEFORM                 2
#define EAXVOCALMORPHER_DEFAULTWAVEFORM             0

#define EAXVOCALMORPHER_MINRATE                     0.0f
#define EAXVOCALMORPHER_MAXRATE                     10.0f
#define EAXVOCALMORPHER_DEFAULTRATE                 1.41f

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Pitch Shifter Effect

// EAX PITCH SHIFTER {E7905100-AFB2-11D2-88DD-A024D13CE1}

DEFINE_GUID(EAX_PITCHSHIFTER_EFFECT,
    0xe7905100,
    0xafb2,
    0x11d2,
    0x88, 0xdd, 0x0, 0xa0, 0x24, 0xd1, 0x3c, 0xe1);

// Pitch Shifter properties
typedef enum
{
    EAXPITCHSHIFTER_NONE,
    EAXPITCHSHIFTER_ALLPARAMETERS,
    EAXPITCHSHIFTER_COARSETUNE,
    EAXPITCHSHIFTER_FINETUNE
} EAXPITCHSHIFTER_PROPERTY;

// OR these flags with property id
#define EAXPITCHSHIFTER_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXPITCHSHIFTER_DEFERRED  0x80000000 // changes take effect later
#define EAXPITCHSHIFTER_COMMITDEFERREDSETTINGS (EAXPITCHSHIFTER_NONE | \
                                                EAXPITCHSHIFTER_IMMEDIATE)

// Use this structure for EAXPITCHSHIFTER_ALLPARAMETERS
typedef struct _EAXPITCHSHIFTERPROPERTIES
{
    long    lCoarseTune;    // Amount of pitch shift (semitones)
    long    lFineTune;      // Amount of pitch shift (cents)
} EAXPITCHSHIFTERPROPERTIES, *LPEAXPITCHSHIFTERPROPERTIES;

// Property ranges and defaults:

#define EAXPITCHSHIFTER_MINCOARSETUNE       (-12)
#define EAXPITCHSHIFTER_MAXCOARSETUNE       12
#define EAXPITCHSHIFTER_DEFAULTCOARSETUNE   12

#define EAXPITCHSHIFTER_MINFINETUNE         (-50)
#define EAXPITCHSHIFTER_MAXFINETUNE         50
#define EAXPITCHSHIFTER_DEFAULTFINETUNE     0

////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Ring Modulator Effect

// EAX RING MODULATOR {B89FE60-AFB5-11D2-88DD-A024D13CE1}

DEFINE_GUID(EAX_RINGMODULATOR_EFFECT,
    0xb89fe60,
    0xafb5,
    0x11d2,
    0x88, 0xdd, 0x0, 0xa0, 0x24, 0xd1, 0x3c, 0xe1);

// Ring Modulator properties
typedef enum
{
    EAXRINGMODULATOR_NONE,
    EAXRINGMODULATOR_ALLPARAMETERS,
    EAXRINGMODULATOR_FREQUENCY,
    EAXRINGMODULATOR_HIGHPASSCUTOFF,
    EAXRINGMODULATOR_WAVEFORM
} EAXRINGMODULATOR_PROPERTY;

// OR these flags with property id
#define EAXRINGMODULATOR_IMMEDIATE 0x00000000 // changes take effect immediately
#define EAXRINGMODULATOR_DEFERRED  0x80000000 // changes take effect later
#define EAXRINGMODULATOR_COMMITDEFERREDSETTINGS (EAXRINGMODULATOR_NONE | \
                                                 EAXRINGMODULATOR_IMMEDIATE)

// used by EAXRINGMODULATOR_WAVEFORM
enum
{
    EAX_RINGMODULATOR_SINUSOID,
    EAX_RINGMODULATOR_SAWTOOTH,
    EAX_RINGMODULATOR_SQUARE
};

// Use this structure for EAXRINGMODULATOR_ALLPARAMETERS
typedef struct _EAXRINGMODULATORPROPERTIES
{
    float           flFrequency;        // Frequency of modulation (Hz)
    float           flHighPassCutOff;   // Cut-off frequency of high-pass filter (Hz)
    unsigned long   ulWaveform;          // Waveform selector - see enum above
} EAXRINGMODULATORPROPERTIES, *LPEAXRINGMODULATORPROPERTIES;

// Property ranges and defaults:

#define EAXRINGMODULATOR_MINFREQUENCY           0.0f
#define EAXRINGMODULATOR_MAXFREQUENCY           8000.0f
#define EAXRINGMODULATOR_DEFAULTFREQUENCY       440.0f

#define EAXRINGMODULATOR_MINHIGHPASSCUTOFF      0.0f
#define EAXRINGMODULATOR_MAXHIGHPASSCUTOFF      24000.0f
#define EAXRINGMODULATOR_DEFAULTHIGHPASSCUTOFF  800.0f

#define EAXRINGMODULATOR_MINWAVEFORM            0
#define EAXRINGMODULATOR_MAXWAVEFORM            2
#define EAXRINGMODULATOR_DEFAULTWAVEFORM        0

////////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

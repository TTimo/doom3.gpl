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

#ifndef CG_EXTERNAL___H
#define CG_EXTERNAL___H



typedef int CGbool;
typedef struct _CGcontext *CGcontext;
typedef struct _CGprogram *CGprogram;
typedef struct _CGparameter *CGparameter;


typedef enum
{
    CG_UNKNOWN_TYPE,
        CG_STRUCT,
        CG_ARRAY,
        
        CG_TYPE_START_ENUM = 1024,
        
        
        CG_HALF ,
        CG_HALF2 ,
        CG_HALF3 ,
        CG_HALF4 ,
        CG_HALF1x1 ,
        CG_HALF1x2 ,
        CG_HALF1x3 ,
        CG_HALF1x4 ,
        CG_HALF2x1 ,
        CG_HALF2x2 ,
        CG_HALF2x3 ,
        CG_HALF2x4 ,
        CG_HALF3x1 ,
        CG_HALF3x2 ,
        CG_HALF3x3 ,
        CG_HALF3x4 ,
        CG_HALF4x1 ,
        CG_HALF4x2 ,
        CG_HALF4x3 ,
        CG_HALF4x4 ,
        CG_FLOAT ,
        CG_FLOAT2 ,
        CG_FLOAT3 ,
        CG_FLOAT4 ,
        CG_FLOAT1x1 ,
        CG_FLOAT1x2 ,
        CG_FLOAT1x3 ,
        CG_FLOAT1x4 ,
        CG_FLOAT2x1 ,
        CG_FLOAT2x2 ,
        CG_FLOAT2x3 ,
        CG_FLOAT2x4 ,
        CG_FLOAT3x1 ,
        CG_FLOAT3x2 ,
        CG_FLOAT3x3 ,
        CG_FLOAT3x4 ,
        CG_FLOAT4x1 ,
        CG_FLOAT4x2 ,
        CG_FLOAT4x3 ,
        CG_FLOAT4x4 ,
        CG_SAMPLER1D ,
        CG_SAMPLER2D ,
        CG_SAMPLER3D ,
        CG_SAMPLERRECT ,
        CG_SAMPLERCUBE ,
        CG_FIXED ,
        CG_FIXED2 ,
        CG_FIXED3 ,
        CG_FIXED4 ,
        CG_FIXED1x1 ,
        CG_FIXED1x2 ,
        CG_FIXED1x3 ,
        CG_FIXED1x4 ,
        CG_FIXED2x1 ,
        CG_FIXED2x2 ,
        CG_FIXED2x3 ,
        CG_FIXED2x4 ,
        CG_FIXED3x1 ,
        CG_FIXED3x2 ,
        CG_FIXED3x3 ,
        CG_FIXED3x4 ,
        CG_FIXED4x1 ,
        CG_FIXED4x2 ,
        CG_FIXED4x3 ,
        CG_FIXED4x4 ,
        CG_HALF1 ,
        CG_FLOAT1 ,
        CG_FIXED1 ,
        
        
} CGtype;

typedef enum
{
    
    CG_TEXUNIT0 = 2048,
        CG_TEXUNIT1 = 2049,
        CG_TEXUNIT2 = 2050,
        CG_TEXUNIT3 = 2051,
        CG_TEXUNIT4 = 2052,
        CG_TEXUNIT5 = 2053,
        CG_TEXUNIT6 = 2054,
        CG_TEXUNIT7 = 2055,
        CG_TEXUNIT8 = 2056,
        CG_TEXUNIT9 = 2057,
        CG_TEXUNIT10 = 2058,
        CG_TEXUNIT11 = 2059,
        CG_TEXUNIT12 = 2060,
        CG_TEXUNIT13 = 2061,
        CG_TEXUNIT14 = 2062,
        CG_TEXUNIT15 = 2063,
        
        CG_ATTR0 = 2113,
        CG_ATTR1 = 2114,
        CG_ATTR2 = 2115,
        CG_ATTR3 = 2116,
        CG_ATTR4 = 2117,
        CG_ATTR5 = 2118,
        CG_ATTR6 = 2119,
        CG_ATTR7 = 2120,
        CG_ATTR8 = 2121,
        CG_ATTR9 = 2122,
        CG_ATTR10 = 2123,
        CG_ATTR11 = 2124,
        CG_ATTR12 = 2125,
        CG_ATTR13 = 2126,
        CG_ATTR14 = 2127,
        CG_ATTR15 = 2128,
        
        CG_C = 2178,
        
        CG_TEX0 = 2179,
        CG_TEX1 = 2180,
        CG_TEX2 = 2181,
        CG_TEX3 = 2192,
        CG_TEX4 = 2193,
        CG_TEX5 = 2194,
        CG_TEX6 = 2195,
        CG_TEX7 = 2196,
        
        CG_HPOS = 2243,
        CG_COL0 = 2245,
        CG_COL1 = 2246,
        CG_COL2 = 2247,
        CG_COL3 = 2248,
        CG_PSIZ = 2309,
        CG_WPOS = 2373,
        
        CG_POSITION0 = 2437,
        CG_POSITION1 = 2438,
        CG_POSITION2 = 2439,
        CG_POSITION3 = 2440,
        CG_POSITION4 = 2441,
        CG_POSITION5 = 2442,
        CG_POSITION6 = 2443,
        CG_POSITION7 = 2444,
        CG_POSITION8 = 2445,
        CG_POSITION9 = 2446,
        CG_POSITION10 = 2447,
        CG_POSITION11 = 2448,
        CG_POSITION12 = 2449,
        CG_POSITION13 = 2450,
        CG_POSITION14 = 2451,
        CG_POSITION15 = 2452,
        CG_DIFFUSE0 = 2501,
        CG_TANGENT0 = 2565,
        CG_TANGENT1 = 2566,
        CG_TANGENT2 = 2567,
        CG_TANGENT3 = 2568,
        CG_TANGENT4 = 2569,
        CG_TANGENT5 = 2570,
        CG_TANGENT6 = 2571,
        CG_TANGENT7 = 2572,
        CG_TANGENT8 = 2573,
        CG_TANGENT9 = 2574,
        CG_TANGENT10 = 2575,
        CG_TANGENT11 = 2576,
        CG_TANGENT12 = 2577,
        CG_TANGENT13 = 2578,
        CG_TANGENT14 = 2579,
        CG_TANGENT15 = 2580,
        CG_SPECULAR0 = 2629,
        CG_BLENDINDICES0 = 2693,
        CG_BLENDINDICES1 = 2694,
        CG_BLENDINDICES2 = 2695,
        CG_BLENDINDICES3 = 2696,
        CG_BLENDINDICES4 = 2697,
        CG_BLENDINDICES5 = 2698,
        CG_BLENDINDICES6 = 2699,
        CG_BLENDINDICES7 = 2700,
        CG_BLENDINDICES8 = 2701,
        CG_BLENDINDICES9 = 2702,
        CG_BLENDINDICES10 = 2703,
        CG_BLENDINDICES11 = 2704,
        CG_BLENDINDICES12 = 2705,
        CG_BLENDINDICES13 = 2706,
        CG_BLENDINDICES14 = 2707,
        CG_BLENDINDICES15 = 2708,
        CG_COLOR0 = 2757,
        CG_COLOR1 = 2758,
        CG_COLOR2 = 2759,
        CG_COLOR3 = 2760,
        CG_COLOR4 = 2761,
        CG_COLOR5 = 2762,
        CG_COLOR6 = 2763,
        CG_COLOR7 = 2764,
        CG_COLOR8 = 2765,
        CG_COLOR9 = 2766,
        CG_COLOR10 = 2767,
        CG_COLOR11 = 2768,
        CG_COLOR12 = 2769,
        CG_COLOR13 = 2770,
        CG_COLOR14 = 2771,
        CG_COLOR15 = 2772,
        CG_PSIZE0 = 2821,
        CG_PSIZE1 = 2822,
        CG_PSIZE2 = 2823,
        CG_PSIZE3 = 2824,
        CG_PSIZE4 = 2825,
        CG_PSIZE5 = 2826,
        CG_PSIZE6 = 2827,
        CG_PSIZE7 = 2828,
        CG_PSIZE8 = 2829,
        CG_PSIZE9 = 2830,
        CG_PSIZE10 = 2831,
        CG_PSIZE11 = 2832,
        CG_PSIZE12 = 2833,
        CG_PSIZE13 = 2834,
        CG_PSIZE14 = 2835,
        CG_PSIZE15 = 2836,
        CG_BINORMAL0 = 2885,
        CG_BINORMAL1 = 2886,
        CG_BINORMAL2 = 2887,
        CG_BINORMAL3 = 2888,
        CG_BINORMAL4 = 2889,
        CG_BINORMAL5 = 2890,
        CG_BINORMAL6 = 2891,
        CG_BINORMAL7 = 2892,
        CG_BINORMAL8 = 2893,
        CG_BINORMAL9 = 2894,
        CG_BINORMAL10 = 2895,
        CG_BINORMAL11 = 2896,
        CG_BINORMAL12 = 2897,
        CG_BINORMAL13 = 2898,
        CG_BINORMAL14 = 2899,
        CG_BINORMAL15 = 2900,
        CG_FOG0 = 2917,
        CG_FOG1 = 2918,
        CG_FOG2 = 2919,
        CG_FOG3 = 2920,
        CG_FOG4 = 2921,
        CG_FOG5 = 2922,
        CG_FOG6 = 2923,
        CG_FOG7 = 2924,
        CG_FOG8 = 2925,
        CG_FOG9 = 2926,
        CG_FOG10 = 2927,
        CG_FOG11 = 2928,
        CG_FOG12 = 2929,
        CG_FOG13 = 2930,
        CG_FOG14 = 2931,
        CG_FOG15 = 2932,
        CG_DEPTH0 = 2933,
        CG_DEPTH1 = 2934,
        CG_DEPTH2 = 2935,
        CG_DEPTH3 = 2936,
        CG_DEPTH4 = 2937,
        CG_DEPTH5 = 2938,
        CG_DEPTH6 = 2939,
        CG_DEPTH7 = 2940,
        CG_DEPTH8 = 2941,
        CG_DEPTH9 = 29542,
        CG_DEPTH10 = 2943,
        CG_DEPTH11 = 2944,
        CG_DEPTH12 = 2945,
        CG_DEPTH13 = 2946,
        CG_DEPTH14 = 2947,
        CG_DEPTH15 = 2948,
        CG_SAMPLE0 = 2949,
        CG_SAMPLE1 = 2950,
        CG_SAMPLE2 = 2951,
        CG_SAMPLE3 = 2952,
        CG_SAMPLE4 = 2953,
        CG_SAMPLE5 = 2954,
        CG_SAMPLE6 = 2955,
        CG_SAMPLE7 = 2956,
        CG_SAMPLE8 = 2957,
        CG_SAMPLE9 = 2958,
        CG_SAMPLE10 = 2959,
        CG_SAMPLE11 = 2960,
        CG_SAMPLE12 = 2961,
        CG_SAMPLE13 = 2962,
        CG_SAMPLE14 = 2963,
        CG_SAMPLE15 = 2964,
        CG_BLENDWEIGHT0 = 3028,
        CG_BLENDWEIGHT1 = 3029,
        CG_BLENDWEIGHT2 = 3030,
        CG_BLENDWEIGHT3 = 3031,
        CG_BLENDWEIGHT4 = 3032,
        CG_BLENDWEIGHT5 = 3033,
        CG_BLENDWEIGHT6 = 3034,
        CG_BLENDWEIGHT7 = 3035,
        CG_BLENDWEIGHT8 = 3036,
        CG_BLENDWEIGHT9 = 3037,
        CG_BLENDWEIGHT10 = 3038,
        CG_BLENDWEIGHT11 = 3039,
        CG_BLENDWEIGHT12 = 3040,
        CG_BLENDWEIGHT13 = 3041,
        CG_BLENDWEIGHT14 = 3042,
        CG_BLENDWEIGHT15 = 3043,
        CG_NORMAL0 = 3092,
        CG_NORMAL1 = 3093,
        CG_NORMAL2 = 3094,
        CG_NORMAL3 = 3095,
        CG_NORMAL4 = 3096,
        CG_NORMAL5 = 3097,
        CG_NORMAL6 = 3098,
        CG_NORMAL7 = 3099,
        CG_NORMAL8 = 3100,
        CG_NORMAL9 = 3101,
        CG_NORMAL10 = 3102,
        CG_NORMAL11 = 3103,
        CG_NORMAL12 = 3104,
        CG_NORMAL13 = 3105,
        CG_NORMAL14 = 3106,
        CG_NORMAL15 = 3107,
        CG_FOGCOORD = 3156,
        CG_TEXCOORD0 = 3220,
        CG_TEXCOORD1 = 3221,
        CG_TEXCOORD2 = 3222,
        CG_TEXCOORD3 = 3223,
        CG_TEXCOORD4 = 3224,
        CG_TEXCOORD5 = 3225,
        CG_TEXCOORD6 = 3226,
        CG_TEXCOORD7 = 3227,
        CG_TEXCOORD8 = 3228,
        CG_TEXCOORD9 = 3229,
        CG_TEXCOORD10 = 3230,
        CG_TEXCOORD11 = 3231,
        CG_TEXCOORD12 = 3232,
        CG_TEXCOORD13 = 3233,
        CG_TEXCOORD14 = 3234,
        CG_TEXCOORD15 = 3235,
        CG_COMBINER_CONST0 = 3284,
        CG_COMBINER_CONST1 = 3285,
        CG_COMBINER_STAGE_CONST0 = 3286,
        CG_COMBINER_STAGE_CONST1 = 3287,
        CG_OFFSET_TEXTURE_MATRIX = 3288,
        CG_OFFSET_TEXTURE_SCALE = 3289,
        CG_OFFSET_TEXTURE_BIAS = 3290,
        CG_CONST_EYE = 3291,
        CG_TESSFACTOR = 3255,
        
        
        CG_UNDEFINED,
        
 } CGresource;
 
 typedef enum
 {
     CG_PROFILE_START = 6144,
         CG_PROFILE_UNKNOWN,
         
         CG_PROFILE_VP20 = 6146,
         CG_PROFILE_FP20 = 6147,
         CG_PROFILE_VP30 = 6148,
         CG_PROFILE_FP30 = 6149,
         CG_PROFILE_ARBVP1 = 6150,
         CG_PROFILE_ARBFP1 = 7000,
         
         
         CG_PROFILE_VS_1_1 = 6153,
         CG_PROFILE_VS_2_0 = 6154,
         CG_PROFILE_VS_2_X = 6155,
         
         CG_PROFILE_PS_1_1 = 6159,
         CG_PROFILE_PS_1_2 = 6160,
         CG_PROFILE_PS_1_3 = 6161,
         CG_PROFILE_PS_2_0 = 6162,
         CG_PROFILE_PS_2_X = 6163,
         
         CG_PROFILE_MAX,
 } CGprofile;
 
 typedef enum
 {
     
     
     CG_NO_ERROR = 0,
         CG_COMPILER_ERROR = 1,
         CG_INVALID_PARAMETER_ERROR = 2,
         CG_INVALID_PROFILE_ERROR = 3,
         CG_PROGRAM_LOAD_ERROR = 4,
         CG_PROGRAM_BIND_ERROR = 5,
         CG_PROGRAM_NOT_LOADED_ERROR = 6,
         CG_UNSUPPORTED_GL_EXTENSION_ERROR = 7,
         CG_INVALID_VALUE_TYPE_ERROR = 8,
         CG_NOT_MATRIX_PARAM_ERROR = 9,
         CG_INVALID_ENUMERANT_ERROR = 10,
         CG_NOT_4x4_MATRIX_ERROR = 11,
         CG_FILE_READ_ERROR = 12,
         CG_FILE_WRITE_ERROR = 13,
         CG_NVPARSE_ERROR = 14,
         CG_MEMORY_ALLOC_ERROR = 15,
         CG_INVALID_CONTEXT_HANDLE_ERROR = 16,
         CG_INVALID_PROGRAM_HANDLE_ERROR = 17,
         CG_INVALID_PARAM_HANDLE_ERROR = 18,
         CG_UNKNOWN_PROFILE_ERROR = 19,
         CG_VAR_ARG_ERROR = 20,
         CG_INVALID_DIMENSION_ERROR = 21,
         CG_ARRAY_PARAM_ERROR = 22,
         CG_OUT_OF_ARRAY_BOUNDS_ERROR = 23,
 } CGerror;
 
 typedef enum
 {
     CG_UNKNOWN = 4096,
         CG_IN,
         CG_OUT,
         CG_INOUT,
         CG_MIXED,
         CG_VARYING,
         CG_UNIFORM,
         CG_CONSTANT,
         CG_PROGRAM_SOURCE,
         CG_PROGRAM_ENTRY,
         CG_COMPILED_PROGRAM,
         CG_PROGRAM_PROFILE,
         
         CG_GLOBAL,
         CG_PROGRAM,
         
         CG_DEFAULT,
         CG_ERROR,
         
         CG_SOURCE,
         CG_OBJECT,
         
 } CGenum;
 
 
 extern "C" {
     
     typedef void (*CGerrorCallbackFunc)(void);
     
     
     
     typedef  CGcontext  (*PFNCGCREATECONTEXTPROC)(void);
     typedef  void  (*PFNCGDESTROYCONTEXTPROC)(CGcontext ctx);
     typedef  CGbool  (*PFNCGISCONTEXTPROC)(CGcontext ctx);
     typedef  const char * (*PFNCGGETLASTLISTINGPROC)(CGcontext ctx);
     typedef  CGprogram (*PFNCGCREATEPROGRAMPROC)(CGcontext ctx, 
         CGenum program_type,
         const char *program,
         CGprofile profile,
         const char *entry,
         const char **args);
     typedef  CGprogram (*PFNCGCREATEPROGRAMFROMFILEPROC)(CGcontext ctx, 
         CGenum program_type,
         const char *program_file,
         CGprofile profile,
         const char *entry,
         const char **args);
     typedef  CGprogram  (*PFNCGCOPYPROGRAMPROC)(CGprogram program);
     typedef  void  (*PFNCGDESTROYPROGRAMPROC)(CGprogram program);
     typedef  CGprogram  (*PFNCGGETFIRSTPROGRAMPROC)(CGcontext ctx);
     typedef  CGprogram  (*PFNCGGETNEXTPROGRAMPROC)(CGprogram current);
     typedef  CGcontext  (*PFNCGGETPROGRAMCONTEXTPROC)(CGprogram prog);
     typedef  CGbool  (*PFNCGISPROGRAMPROC)(CGprogram program);
     typedef  void  (*PFNCGCOMPILEPROGRAMPROC)(CGprogram program);
     typedef  CGbool  (*PFNCGISPROGRAMCOMPILEDPROC)(CGprogram program);
     typedef  const char * (*PFNCGGETPROGRAMSTRINGPROC)(CGprogram prog, CGenum pname);
     typedef  CGprofile  (*PFNCGGETPROGRAMPROFILEPROC)(CGprogram prog);
     typedef  CGparameter  (*PFNCGGETNAMEDPARAMETERPROC)(CGprogram prog, const char *name);
     typedef  CGparameter  (*PFNCGGETFIRSTPARAMETERPROC)(CGprogram prog, CGenum name_space);
     typedef  CGparameter  (*PFNCGGETNEXTPARAMETERPROC)(CGparameter current);
     typedef  CGparameter  (*PFNCGGETFIRSTLEAFPARAMETERPROC)(CGprogram prog, CGenum name_space);
     typedef  CGparameter  (*PFNCGGETNEXTLEAFPARAMETERPROC)(CGparameter current);
     typedef  CGparameter  (*PFNCGGETFIRSTSTRUCTPARAMETERPROC)(CGparameter param);
     typedef  CGparameter  (*PFNCGGETFIRSTDEPENDENTPARAMETERPROC)(CGparameter param);
     typedef  CGparameter  (*PFNCGGETARRAYPARAMETERPROC)(CGparameter aparam, int index);
     typedef  int  (*PFNCGGETARRAYDIMENSIONPROC)(CGparameter param);
     typedef  int  (*PFNCGGETARRAYSIZEPROC)(CGparameter param, int dimension);
     typedef  CGprogram  (*PFNCGGETPARAMETERPROGRAMPROC)(CGparameter prog);
     typedef  CGbool  (*PFNCGISPARAMETERPROC)(CGparameter param);
     typedef  const char * (*PFNCGGETPARAMETERNAMEPROC)(CGparameter param);
     typedef  CGtype  (*PFNCGGETPARAMETERTYPEPROC)(CGparameter param);
     typedef  const char * (*PFNCGGETPARAMETERSEMANTICPROC)(CGparameter param);
     typedef  CGresource  (*PFNCGGETPARAMETERRESOURCEPROC)(CGparameter param);
     typedef  CGresource  (*PFNCGGETPARAMETERBASERESOURCEPROC)(CGparameter param);
     typedef  unsigned long  (*PFNCGGETPARAMETERRESOURCEINDEXPROC)(CGparameter param);
     typedef  CGenum  (*PFNCGGETPARAMETERVARIABILITYPROC)(CGparameter param);
     typedef  CGenum  (*PFNCGGETPARAMETERDIRECTIONPROC)(CGparameter param);
     typedef  CGbool  (*PFNCGISPARAMETERREFERENCEDPROC)(CGparameter param);
     typedef  void (*PFNCGGETPARAMETERVALUESPROC)(CGparameter param, 
         CGenum value_type,
         int *nvalues);
     typedef  const char * (*PFNCGGETTYPESTRINGPROC)(CGtype type);
     typedef  CGtype  (*PFNCGGETTYPEPROC)(const char *type_string);
     typedef  const char * (*PFNCGGETRESOURCESTRINGPROC)(CGresource resource);
     typedef  CGresource  (*PFNCGGETRESOURCEPROC)(const char *resource_string);
     typedef  const char * (*PFNCGGETPROFILESTRINGPROC)(CGprofile profile);
     typedef  CGprofile  (*PFNCGGETPROFILEPROC)(const char *profile_string);
     typedef  CGerror  (*PFNCGGETERRORPROC)(void);
     typedef  const char * (*PFNCGGETERRORSTRINGPROC)(CGerror error);
     typedef  void  (*PFNCGSETERRORCALLBACKPROC)(CGerrorCallbackFunc func);
     typedef  CGerrorCallbackFunc  (*PFNCGGETERRORCALLBACKPROC)(void);
     
     
     extern PFNCGCREATECONTEXTPROC cgCreateContext;
     extern PFNCGDESTROYCONTEXTPROC cgDestroyContext;
     extern PFNCGISCONTEXTPROC cgIsContext;
     extern PFNCGGETLASTLISTINGPROC cgGetLastListing;
     extern PFNCGCREATEPROGRAMPROC cgCreateProgram;
     extern PFNCGCREATEPROGRAMFROMFILEPROC cgCreateProgramFromFile;
     extern PFNCGCOPYPROGRAMPROC cgCopyProgram;
     extern PFNCGDESTROYPROGRAMPROC cgDestroyProgram;
     extern PFNCGGETFIRSTPROGRAMPROC cgGetFirstProgram;
     extern PFNCGGETNEXTPROGRAMPROC cgGetNextProgram;
     extern PFNCGGETPROGRAMCONTEXTPROC cgGetProgramContext;
     extern PFNCGISPROGRAMPROC cgIsProgram;
     extern PFNCGCOMPILEPROGRAMPROC cgCompileProgram;
     extern PFNCGISPROGRAMCOMPILEDPROC cgIsProgramCompiled;
     extern PFNCGGETPROGRAMSTRINGPROC cgGetProgramString;
     extern PFNCGGETPROGRAMPROFILEPROC cgGetProgramProfile;
     extern PFNCGGETNAMEDPARAMETERPROC cgGetNamedParameter;
     extern PFNCGGETFIRSTPARAMETERPROC cgGetFirstParameter;
     extern PFNCGGETNEXTPARAMETERPROC cgGetNextParameter;
     extern PFNCGGETFIRSTLEAFPARAMETERPROC cgGetFirstLeafParameter;
     extern PFNCGGETNEXTLEAFPARAMETERPROC cgGetNextLeafParameter;
     extern PFNCGGETFIRSTSTRUCTPARAMETERPROC cgGetFirstStructParameter;
     extern PFNCGGETFIRSTDEPENDENTPARAMETERPROC cgGetFirstDependentParameter;
     extern PFNCGGETARRAYPARAMETERPROC cgGetArrayParameter;
     extern PFNCGGETARRAYDIMENSIONPROC cgGetArrayDimension;
     extern PFNCGGETARRAYSIZEPROC cgGetArraySize;
     extern PFNCGGETPARAMETERPROGRAMPROC cgGetParameterProgram;
     extern PFNCGISPARAMETERPROC cgIsParameter;
     extern PFNCGGETPARAMETERNAMEPROC cgGetParameterName;
     extern PFNCGGETPARAMETERTYPEPROC cgGetParameterType;
     extern PFNCGGETPARAMETERSEMANTICPROC cgGetParameterSemantic;
     extern PFNCGGETPARAMETERRESOURCEPROC cgGetParameterResource;
     extern PFNCGGETPARAMETERBASERESOURCEPROC cgGetParameterBaseResource;
     extern PFNCGGETPARAMETERRESOURCEINDEXPROC cgGetParameterResourceIndex;
     extern PFNCGGETPARAMETERVARIABILITYPROC cgGetParameterVariability;
     extern PFNCGGETPARAMETERDIRECTIONPROC cgGetParameterDirection;
     extern PFNCGISPARAMETERREFERENCEDPROC cgIsParameterReferenced;
     extern PFNCGGETPARAMETERVALUESPROC cgGetParameterValues;
     extern PFNCGGETTYPESTRINGPROC cgGetTypeString;
     extern PFNCGGETTYPEPROC cgGetType;
     extern PFNCGGETRESOURCESTRINGPROC cgGetResourceString;
     extern PFNCGGETRESOURCEPROC cgGetResource;
     extern PFNCGGETPROFILESTRINGPROC cgGetProfileString;
     extern PFNCGGETPROFILEPROC cgGetProfile;
     extern PFNCGGETERRORPROC cgGetError;
     extern PFNCGGETERRORSTRINGPROC cgGetErrorString;
     extern PFNCGSETERRORCALLBACKPROC cgSetErrorCallback;
     extern PFNCGGETERRORCALLBACKPROC cgGetErrorCallback;
     
     
 }
 
 
 
 extern "C" {
     
     typedef enum
     {
         CG_GL_MATRIX_IDENTITY = 0,
             CG_GL_MATRIX_TRANSPOSE = 1,
             CG_GL_MATRIX_INVERSE = 2,
             CG_GL_MATRIX_INVERSE_TRANSPOSE = 3,
             
             CG_GL_MODELVIEW_MATRIX,
             CG_GL_PROJECTION_MATRIX,
             CG_GL_TEXTURE_MATRIX,
             CG_GL_MODELVIEW_PROJECTION_MATRIX,
             
             CG_GL_VERTEX,
             CG_GL_FRAGMENT,
             
     } CGGLenum;
     
     
     
     
     typedef  CGbool  (*PFNCGGLISPROFILESUPPORTEDPROC)(CGprofile profile);
     typedef  void  (*PFNCGGLENABLEPROFILEPROC)(CGprofile profile);
     typedef  void  (*PFNCGGLDISABLEPROFILEPROC)(CGprofile profile);
     typedef  CGprofile  (*PFNCGGLGETLATESTPROFILEPROC)(CGGLenum profile_type);
     typedef  void  (*PFNCGGLSETOPTIMALOPTIONSPROC)(CGprofile profile);
     typedef  void  (*PFNCGGLLOADPROGRAMPROC)(CGprogram program);
     typedef  void  (*PFNCGGLBINDPROGRAMPROC)(CGprogram program);
     typedef  void (*PFNCGGLSETPARAMETER1FPROC)(CGparameter param,
         float x);
     typedef  void (*PFNCGGLSETPARAMETER2FPROC)(CGparameter param,
         float x,
         float y);
     typedef  void (*PFNCGGLSETPARAMETER3FPROC)(CGparameter param,
         float x,
         float y,
         float z);
     typedef  void (*PFNCGGLSETPARAMETER4FPROC)(CGparameter param,
         float x,
         float y,
         float z,
         float w);
     typedef  void  (*PFNCGGLSETPARAMETER1FVPROC)(CGparameter param, const float *v);
     typedef  void  (*PFNCGGLSETPARAMETER2FVPROC)(CGparameter param, const float *v);
     typedef  void  (*PFNCGGLSETPARAMETER3FVPROC)(CGparameter param, const float *v);
     typedef  void  (*PFNCGGLSETPARAMETER4FVPROC)(CGparameter param, const float *v);
     typedef  void (*PFNCGGLSETPARAMETER1DPROC)(CGparameter param,
         double x);
     typedef  void (*PFNCGGLSETPARAMETER2DPROC)(CGparameter param,
         double x,
         double y);
     typedef  void (*PFNCGGLSETPARAMETER3DPROC)(CGparameter param,
         double x,
         double y,
         double z);
     typedef  void (*PFNCGGLSETPARAMETER4DPROC)(CGparameter param,
         double x,
         double y,
         double z,
         double w);
     typedef  void  (*PFNCGGLSETPARAMETER1DVPROC)(CGparameter param, const double *v);
     typedef  void  (*PFNCGGLSETPARAMETER2DVPROC)(CGparameter param, const double *v);
     typedef  void  (*PFNCGGLSETPARAMETER3DVPROC)(CGparameter param, const double *v);
     typedef  void  (*PFNCGGLSETPARAMETER4DVPROC)(CGparameter param, const double *v);
     typedef  void  (*PFNCGGLSETPARAMETER4DVPROC)(CGparameter param, const double *v);
     typedef  void  (*PFNCGGLGETPARAMETER1FPROC)(CGparameter param, float *v);
     typedef  void  (*PFNCGGLGETPARAMETER2FPROC)(CGparameter param, float *v);
     typedef  void  (*PFNCGGLGETPARAMETER3FPROC)(CGparameter param, float *v);
     typedef  void  (*PFNCGGLGETPARAMETER4FPROC)(CGparameter param, float *v);
     typedef  void  (*PFNCGGLGETPARAMETER1DPROC)(CGparameter param, double *v);
     typedef  void  (*PFNCGGLGETPARAMETER2DPROC)(CGparameter param, double *v);
     typedef  void  (*PFNCGGLGETPARAMETER3DPROC)(CGparameter param, double *v);
     typedef  void  (*PFNCGGLGETPARAMETER4DPROC)(CGparameter param, double *v);
     typedef  void (*PFNCGGLSETPARAMETERARRAY1FPROC)(CGparameter param,
         long offset,
         long nelements,
         const float *v);
     typedef  void (*PFNCGGLSETPARAMETERARRAY2FPROC)(CGparameter param,
         long offset,
         long nelements,
         const float *v);
     typedef  void (*PFNCGGLSETPARAMETERARRAY3FPROC)(CGparameter param,
         long offset,
         long nelements,
         const float *v);
     typedef  void (*PFNCGGLSETPARAMETERARRAY4FPROC)(CGparameter param,
         long offset,
         long nelements,
         const float *v);
     typedef  void (*PFNCGGLSETPARAMETERARRAY1DPROC)(CGparameter param,
         long offset,
         long nelements,
         const double *v);
     typedef  void (*PFNCGGLSETPARAMETERARRAY2DPROC)(CGparameter param,
         long offset,
         long nelements,
         const double *v);
     typedef  void (*PFNCGGLSETPARAMETERARRAY3DPROC)(CGparameter param,
         long offset,
         long nelements,
         const double *v);
     typedef  void (*PFNCGGLSETPARAMETERARRAY4DPROC)(CGparameter param,
         long offset,
         long nelements,
         const double *v);
     typedef  void (*PFNCGGLGETPARAMETERARRAY1FPROC)(CGparameter param,
         long offset,
         long nelements,
         float *v);
     typedef  void (*PFNCGGLGETPARAMETERARRAY2FPROC)(CGparameter param,
         long offset,
         long nelements,
         float *v);
     typedef  void (*PFNCGGLGETPARAMETERARRAY3FPROC)(CGparameter param,
         long offset,
         long nelements,
         float *v);
     typedef  void (*PFNCGGLGETPARAMETERARRAY4FPROC)(CGparameter param,
         long offset,
         long nelements,
         float *v);
     typedef  void (*PFNCGGLGETPARAMETERARRAY1DPROC)(CGparameter param,
         long offset,
         long nelements,
         double *v);
     typedef  void (*PFNCGGLGETPARAMETERARRAY2DPROC)(CGparameter param,
         long offset,
         long nelements,
         double *v);
     typedef  void (*PFNCGGLGETPARAMETERARRAY3DPROC)(CGparameter param,
         long offset,
         long nelements,
         double *v);
     typedef  void (*PFNCGGLGETPARAMETERARRAY4DPROC)(CGparameter param,
         long offset,
         long nelements,
         double *v);
     typedef  void (*PFNCGGLSETPARAMETERPOINTERPROC)(CGparameter param,
         GLint fsize,
         GLenum type,
         GLsizei stride,
         GLvoid *pointer);
     typedef  void  (*PFNCGGLENABLECLIENTSTATEPROC)(CGparameter param);
     typedef  void  (*PFNCGGLDISABLECLIENTSTATEPROC)(CGparameter param);
     typedef  void  (*PFNCGGLSETMATRIXPARAMETERDRPROC)(CGparameter param, const double *matrix);
     typedef  void  (*PFNCGGLSETMATRIXPARAMETERFRPROC)(CGparameter param, const float *matrix);
     typedef  void  (*PFNCGGLSETMATRIXPARAMETERDCPROC)(CGparameter param, const double *matrix);
     typedef  void  (*PFNCGGLSETMATRIXPARAMETERFCPROC)(CGparameter param, const float *matrix);
     typedef  void  (*PFNCGGLGETMATRIXPARAMETERDRPROC)(CGparameter param, double *matrix);
     typedef  void  (*PFNCGGLGETMATRIXPARAMETERFRPROC)(CGparameter param, float *matrix);
     typedef  void  (*PFNCGGLGETMATRIXPARAMETERDCPROC)(CGparameter param, double *matrix);
     typedef  void  (*PFNCGGLGETMATRIXPARAMETERFCPROC)(CGparameter param, float *matrix);
     typedef  void (*PFNCGGLSETSTATEMATRIXPARAMETERPROC)(CGparameter param, 
         GLenum matrix,
         GLenum transform);
     typedef  void (*PFNCGGLSETMATRIXPARAMETERARRAYFCPROC)(CGparameter param, 
         long offset,
         long nelements,
         const float *matrices);
     typedef  void (*PFNCGGLSETMATRIXPARAMETERARRAYFRPROC)(CGparameter param, 
         long offset,
         long nelements,
         const float *matrices);
     typedef  void (*PFNCGGLSETMATRIXPARAMETERARRAYDCPROC)(CGparameter param, 
         long offset,
         long nelements,
         const double *matrices);
     typedef  void (*PFNCGGLSETMATRIXPARAMETERARRAYDRPROC)(CGparameter param, 
         long offset,
         long nelements,
         const double *matrices);
     typedef  void (*PFNCGGLGETMATRIXPARAMETERARRAYFCPROC)(CGparameter param, 
         long offset,
         long nelements,
         float *matrices);
     typedef  void (*PFNCGGLGETMATRIXPARAMETERARRAYFRPROC)(CGparameter param, 
         long offset,
         long nelements,
         float *matrices);
     typedef  void (*PFNCGGLGETMATRIXPARAMETERARRAYDCPROC)(CGparameter param, 
         long offset,
         long nelements,
         double *matrices);
     typedef  void (*PFNCGGLGETMATRIXPARAMETERARRAYDRPROC)(CGparameter param, 
         long offset,
         long nelements,
         double *matrices);
     typedef  void  (*PFNCGGLSETTEXTUREPARAMETERPROC)(CGparameter param, GLuint texobj);
     typedef  GLuint  (*PFNCGGLGETTEXTUREPARAMETERPROC)(CGparameter param);
     typedef  void  (*PFNCGGLENABLETEXTUREPARAMETERPROC)(CGparameter param);
     typedef  void  (*PFNCGGLDISABLETEXTUREPARAMETERPROC)(CGparameter param);
     typedef  GLenum  (*PFNCGGLGETTEXTUREENUMPROC)(CGparameter param);
     
     
     extern PFNCGGLISPROFILESUPPORTEDPROC cgGLIsProfileSupported;
     extern PFNCGGLENABLEPROFILEPROC cgGLEnableProfile;
     extern PFNCGGLDISABLEPROFILEPROC cgGLDisableProfile;
     extern PFNCGGLGETLATESTPROFILEPROC cgGLGetLatestProfile;
     extern PFNCGGLSETOPTIMALOPTIONSPROC cgGLSetOptimalOptions;
     extern PFNCGGLLOADPROGRAMPROC cgGLLoadProgram;
     extern PFNCGGLBINDPROGRAMPROC cgGLBindProgram;
     extern PFNCGGLSETPARAMETER1FPROC cgGLSetParameter1f;
     extern PFNCGGLSETPARAMETER2FPROC cgGLSetParameter2f;
     extern PFNCGGLSETPARAMETER3FPROC cgGLSetParameter3f;
     extern PFNCGGLSETPARAMETER4FPROC cgGLSetParameter4f;
     extern PFNCGGLSETPARAMETER1FVPROC cgGLSetParameter1fv;
     extern PFNCGGLSETPARAMETER2FVPROC cgGLSetParameter2fv;
     extern PFNCGGLSETPARAMETER3FVPROC cgGLSetParameter3fv;
     extern PFNCGGLSETPARAMETER4FVPROC cgGLSetParameter4fv;
     extern PFNCGGLSETPARAMETER1DPROC cgGLSetParameter1d;
     extern PFNCGGLSETPARAMETER2DPROC cgGLSetParameter2d;
     extern PFNCGGLSETPARAMETER3DPROC cgGLSetParameter3d;
     extern PFNCGGLSETPARAMETER4DPROC cgGLSetParameter4d;
     extern PFNCGGLSETPARAMETER1DVPROC cgGLSetParameter1dv;
     extern PFNCGGLSETPARAMETER2DVPROC cgGLSetParameter2dv;
     extern PFNCGGLSETPARAMETER3DVPROC cgGLSetParameter3dv;
     extern PFNCGGLSETPARAMETER4DVPROC cgGLSetParameter4dv;
     extern PFNCGGLGETPARAMETER1FPROC cgGLGetParameter1f;
     extern PFNCGGLGETPARAMETER2FPROC cgGLGetParameter2f;
     extern PFNCGGLGETPARAMETER3FPROC cgGLGetParameter3f;
     extern PFNCGGLGETPARAMETER4FPROC cgGLGetParameter4f;
     extern PFNCGGLGETPARAMETER1DPROC cgGLGetParameter1d;
     extern PFNCGGLGETPARAMETER2DPROC cgGLGetParameter2d;
     extern PFNCGGLGETPARAMETER3DPROC cgGLGetParameter3d;
     extern PFNCGGLGETPARAMETER4DPROC cgGLGetParameter4d;
     extern PFNCGGLSETPARAMETERARRAY1FPROC cgGLSetParameterArray1f;
     extern PFNCGGLSETPARAMETERARRAY2FPROC cgGLSetParameterArray2f;
     extern PFNCGGLSETPARAMETERARRAY3FPROC cgGLSetParameterArray3f;
     extern PFNCGGLSETPARAMETERARRAY4FPROC cgGLSetParameterArray4f;
     extern PFNCGGLSETPARAMETERARRAY1DPROC cgGLSetParameterArray1d;
     extern PFNCGGLSETPARAMETERARRAY2DPROC cgGLSetParameterArray2d;
     extern PFNCGGLSETPARAMETERARRAY3DPROC cgGLSetParameterArray3d;
     extern PFNCGGLSETPARAMETERARRAY4DPROC cgGLSetParameterArray4d;
     extern PFNCGGLGETPARAMETERARRAY1FPROC cgGLGetParameterArray1f;
     extern PFNCGGLGETPARAMETERARRAY2FPROC cgGLGetParameterArray2f;
     extern PFNCGGLGETPARAMETERARRAY3FPROC cgGLGetParameterArray3f;
     extern PFNCGGLGETPARAMETERARRAY4FPROC cgGLGetParameterArray4f;
     extern PFNCGGLGETPARAMETERARRAY1DPROC cgGLGetParameterArray1d;
     extern PFNCGGLGETPARAMETERARRAY2DPROC cgGLGetParameterArray2d;
     extern PFNCGGLGETPARAMETERARRAY3DPROC cgGLGetParameterArray3d;
     extern PFNCGGLGETPARAMETERARRAY4DPROC cgGLGetParameterArray4d;
     extern PFNCGGLSETPARAMETERPOINTERPROC cgGLSetParameterPointer;
     extern PFNCGGLENABLECLIENTSTATEPROC cgGLEnableClientState;
     extern PFNCGGLDISABLECLIENTSTATEPROC cgGLDisableClientState;
     extern PFNCGGLSETMATRIXPARAMETERDRPROC cgGLSetMatrixParameterdr;
     extern PFNCGGLSETMATRIXPARAMETERFRPROC cgGLSetMatrixParameterfr;
     extern PFNCGGLSETMATRIXPARAMETERDCPROC cgGLSetMatrixParameterdc;
     extern PFNCGGLSETMATRIXPARAMETERFCPROC cgGLSetMatrixParameterfc;
     extern PFNCGGLGETMATRIXPARAMETERDRPROC cgGLGetMatrixParameterdr;
     extern PFNCGGLGETMATRIXPARAMETERFRPROC cgGLGetMatrixParameterfr;
     extern PFNCGGLGETMATRIXPARAMETERDCPROC cgGLGetMatrixParameterdc;
     extern PFNCGGLGETMATRIXPARAMETERFCPROC cgGLGetMatrixParameterfc;
     extern PFNCGGLSETSTATEMATRIXPARAMETERPROC cgGLSetStateMatrixParameter;
     extern PFNCGGLSETMATRIXPARAMETERARRAYFCPROC cgGLSetMatrixParameterArrayfc;
     extern PFNCGGLSETMATRIXPARAMETERARRAYFRPROC cgGLSetMatrixParameterArrayfr;
     extern PFNCGGLSETMATRIXPARAMETERARRAYDCPROC cgGLSetMatrixParameterArraydc;
     extern PFNCGGLSETMATRIXPARAMETERARRAYDRPROC cgGLSetMatrixParameterArraydr;
     extern PFNCGGLGETMATRIXPARAMETERARRAYFCPROC cgGLGetMatrixParameterArrayfc;
     extern PFNCGGLGETMATRIXPARAMETERARRAYFRPROC cgGLGetMatrixParameterArrayfr;
     extern PFNCGGLGETMATRIXPARAMETERARRAYDCPROC cgGLGetMatrixParameterArraydc;
     extern PFNCGGLGETMATRIXPARAMETERARRAYDRPROC cgGLGetMatrixParameterArraydr;
     extern PFNCGGLSETTEXTUREPARAMETERPROC cgGLSetTextureParameter;
     extern PFNCGGLGETTEXTUREPARAMETERPROC cgGLGetTextureParameter;
     extern PFNCGGLENABLETEXTUREPARAMETERPROC cgGLEnableTextureParameter;
     extern PFNCGGLDISABLETEXTUREPARAMETERPROC cgGLDisableTextureParameter;
     extern PFNCGGLGETTEXTUREENUMPROC cgGLGetTextureEnum;
     
     
}

bool init_explicit_Cg();

#endif


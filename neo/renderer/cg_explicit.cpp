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

#include "../idlib/precompiled.h"
#pragma hdrstop

//#include <windows.h>
//#include <GL/gl.h>
#include "cg_explicit.h"

PFNCGCREATECONTEXTPROC cgCreateContext;
PFNCGDESTROYCONTEXTPROC cgDestroyContext;
PFNCGISCONTEXTPROC cgIsContext;
PFNCGGETLASTLISTINGPROC cgGetLastListing;
PFNCGCREATEPROGRAMPROC cgCreateProgram;
PFNCGCREATEPROGRAMFROMFILEPROC cgCreateProgramFromFile;
PFNCGCOPYPROGRAMPROC cgCopyProgram;
PFNCGDESTROYPROGRAMPROC cgDestroyProgram;
PFNCGGETFIRSTPROGRAMPROC cgGetFirstProgram;
PFNCGGETNEXTPROGRAMPROC cgGetNextProgram;
PFNCGGETPROGRAMCONTEXTPROC cgGetProgramContext;
PFNCGISPROGRAMPROC cgIsProgram;
PFNCGCOMPILEPROGRAMPROC cgCompileProgram;
PFNCGISPROGRAMCOMPILEDPROC cgIsProgramCompiled;
PFNCGGETPROGRAMSTRINGPROC cgGetProgramString;
PFNCGGETPROGRAMPROFILEPROC cgGetProgramProfile;
PFNCGGETNAMEDPARAMETERPROC cgGetNamedParameter;
PFNCGGETFIRSTPARAMETERPROC cgGetFirstParameter;
PFNCGGETNEXTPARAMETERPROC cgGetNextParameter;
PFNCGGETFIRSTLEAFPARAMETERPROC cgGetFirstLeafParameter;
PFNCGGETNEXTLEAFPARAMETERPROC cgGetNextLeafParameter;
PFNCGGETFIRSTSTRUCTPARAMETERPROC cgGetFirstStructParameter;
PFNCGGETFIRSTDEPENDENTPARAMETERPROC cgGetFirstDependentParameter;
PFNCGGETARRAYPARAMETERPROC cgGetArrayParameter;
PFNCGGETARRAYDIMENSIONPROC cgGetArrayDimension;
PFNCGGETARRAYSIZEPROC cgGetArraySize;
PFNCGGETPARAMETERPROGRAMPROC cgGetParameterProgram;
PFNCGISPARAMETERPROC cgIsParameter;
PFNCGGETPARAMETERNAMEPROC cgGetParameterName;
PFNCGGETPARAMETERTYPEPROC cgGetParameterType;
PFNCGGETPARAMETERSEMANTICPROC cgGetParameterSemantic;
PFNCGGETPARAMETERRESOURCEPROC cgGetParameterResource;
PFNCGGETPARAMETERBASERESOURCEPROC cgGetParameterBaseResource;
PFNCGGETPARAMETERRESOURCEINDEXPROC cgGetParameterResourceIndex;
PFNCGGETPARAMETERVARIABILITYPROC cgGetParameterVariability;
PFNCGGETPARAMETERDIRECTIONPROC cgGetParameterDirection;
PFNCGISPARAMETERREFERENCEDPROC cgIsParameterReferenced;
PFNCGGETPARAMETERVALUESPROC cgGetParameterValues;
PFNCGGETTYPESTRINGPROC cgGetTypeString;
PFNCGGETTYPEPROC cgGetType;
PFNCGGETRESOURCESTRINGPROC cgGetResourceString;
PFNCGGETRESOURCEPROC cgGetResource;
PFNCGGETPROFILESTRINGPROC cgGetProfileString;
PFNCGGETPROFILEPROC cgGetProfile;
PFNCGGETERRORPROC cgGetError;
PFNCGGETERRORSTRINGPROC cgGetErrorString;
PFNCGSETERRORCALLBACKPROC cgSetErrorCallback;
PFNCGGETERRORCALLBACKPROC cgGetErrorCallback;
PFNCGGLISPROFILESUPPORTEDPROC cgGLIsProfileSupported;
PFNCGGLENABLEPROFILEPROC cgGLEnableProfile;
PFNCGGLDISABLEPROFILEPROC cgGLDisableProfile;
PFNCGGLGETLATESTPROFILEPROC cgGLGetLatestProfile;
PFNCGGLSETOPTIMALOPTIONSPROC cgGLSetOptimalOptions;
PFNCGGLLOADPROGRAMPROC cgGLLoadProgram;
PFNCGGLBINDPROGRAMPROC cgGLBindProgram;
PFNCGGLSETPARAMETER1FPROC cgGLSetParameter1f;
PFNCGGLSETPARAMETER2FPROC cgGLSetParameter2f;
PFNCGGLSETPARAMETER3FPROC cgGLSetParameter3f;
PFNCGGLSETPARAMETER4FPROC cgGLSetParameter4f;
PFNCGGLSETPARAMETER1FVPROC cgGLSetParameter1fv;
PFNCGGLSETPARAMETER2FVPROC cgGLSetParameter2fv;
PFNCGGLSETPARAMETER3FVPROC cgGLSetParameter3fv;
PFNCGGLSETPARAMETER4FVPROC cgGLSetParameter4fv;
PFNCGGLSETPARAMETER1DPROC cgGLSetParameter1d;
PFNCGGLSETPARAMETER2DPROC cgGLSetParameter2d;
PFNCGGLSETPARAMETER3DPROC cgGLSetParameter3d;
PFNCGGLSETPARAMETER4DPROC cgGLSetParameter4d;
PFNCGGLSETPARAMETER1DVPROC cgGLSetParameter1dv;
PFNCGGLSETPARAMETER2DVPROC cgGLSetParameter2dv;
PFNCGGLSETPARAMETER3DVPROC cgGLSetParameter3dv;
PFNCGGLSETPARAMETER4DVPROC cgGLSetParameter4dv;
PFNCGGLGETPARAMETER1FPROC cgGLGetParameter1f;
PFNCGGLGETPARAMETER2FPROC cgGLGetParameter2f;
PFNCGGLGETPARAMETER3FPROC cgGLGetParameter3f;
PFNCGGLGETPARAMETER4FPROC cgGLGetParameter4f;
PFNCGGLGETPARAMETER1DPROC cgGLGetParameter1d;
PFNCGGLGETPARAMETER2DPROC cgGLGetParameter2d;
PFNCGGLGETPARAMETER3DPROC cgGLGetParameter3d;
PFNCGGLGETPARAMETER4DPROC cgGLGetParameter4d;
PFNCGGLSETPARAMETERARRAY1FPROC cgGLSetParameterArray1f;
PFNCGGLSETPARAMETERARRAY2FPROC cgGLSetParameterArray2f;
PFNCGGLSETPARAMETERARRAY3FPROC cgGLSetParameterArray3f;
PFNCGGLSETPARAMETERARRAY4FPROC cgGLSetParameterArray4f;
PFNCGGLSETPARAMETERARRAY1DPROC cgGLSetParameterArray1d;
PFNCGGLSETPARAMETERARRAY2DPROC cgGLSetParameterArray2d;
PFNCGGLSETPARAMETERARRAY3DPROC cgGLSetParameterArray3d;
PFNCGGLSETPARAMETERARRAY4DPROC cgGLSetParameterArray4d;
PFNCGGLGETPARAMETERARRAY1FPROC cgGLGetParameterArray1f;
PFNCGGLGETPARAMETERARRAY2FPROC cgGLGetParameterArray2f;
PFNCGGLGETPARAMETERARRAY3FPROC cgGLGetParameterArray3f;
PFNCGGLGETPARAMETERARRAY4FPROC cgGLGetParameterArray4f;
PFNCGGLGETPARAMETERARRAY1DPROC cgGLGetParameterArray1d;
PFNCGGLGETPARAMETERARRAY2DPROC cgGLGetParameterArray2d;
PFNCGGLGETPARAMETERARRAY3DPROC cgGLGetParameterArray3d;
PFNCGGLGETPARAMETERARRAY4DPROC cgGLGetParameterArray4d;
PFNCGGLSETPARAMETERPOINTERPROC cgGLSetParameterPointer;
PFNCGGLENABLECLIENTSTATEPROC cgGLEnableClientState;
PFNCGGLDISABLECLIENTSTATEPROC cgGLDisableClientState;
PFNCGGLSETMATRIXPARAMETERDRPROC cgGLSetMatrixParameterdr;
PFNCGGLSETMATRIXPARAMETERFRPROC cgGLSetMatrixParameterfr;
PFNCGGLSETMATRIXPARAMETERDCPROC cgGLSetMatrixParameterdc;
PFNCGGLSETMATRIXPARAMETERFCPROC cgGLSetMatrixParameterfc;
PFNCGGLGETMATRIXPARAMETERDRPROC cgGLGetMatrixParameterdr;
PFNCGGLGETMATRIXPARAMETERFRPROC cgGLGetMatrixParameterfr;
PFNCGGLGETMATRIXPARAMETERDCPROC cgGLGetMatrixParameterdc;
PFNCGGLGETMATRIXPARAMETERFCPROC cgGLGetMatrixParameterfc;
PFNCGGLSETSTATEMATRIXPARAMETERPROC cgGLSetStateMatrixParameter;
PFNCGGLSETMATRIXPARAMETERARRAYFCPROC cgGLSetMatrixParameterArrayfc;
PFNCGGLSETMATRIXPARAMETERARRAYFRPROC cgGLSetMatrixParameterArrayfr;
PFNCGGLSETMATRIXPARAMETERARRAYDCPROC cgGLSetMatrixParameterArraydc;
PFNCGGLSETMATRIXPARAMETERARRAYDRPROC cgGLSetMatrixParameterArraydr;
PFNCGGLGETMATRIXPARAMETERARRAYFCPROC cgGLGetMatrixParameterArrayfc;
PFNCGGLGETMATRIXPARAMETERARRAYFRPROC cgGLGetMatrixParameterArrayfr;
PFNCGGLGETMATRIXPARAMETERARRAYDCPROC cgGLGetMatrixParameterArraydc;
PFNCGGLGETMATRIXPARAMETERARRAYDRPROC cgGLGetMatrixParameterArraydr;
PFNCGGLSETTEXTUREPARAMETERPROC cgGLSetTextureParameter;
PFNCGGLGETTEXTUREPARAMETERPROC cgGLGetTextureParameter;
PFNCGGLENABLETEXTUREPARAMETERPROC cgGLEnableTextureParameter;
PFNCGGLDISABLETEXTUREPARAMETERPROC cgGLDisableTextureParameter;
PFNCGGLGETTEXTUREENUMPROC cgGLGetTextureEnum;

#ifndef _WIN32
bool init_explicit_Cg()
{
	return false;
}
#else
bool init_explicit_Cg()
{
    HMODULE hmod;
    int failed = 0;
    
    hmod = LoadLibrary("cg.dll");
    
    if(0 == (cgCreateContext = (PFNCGCREATECONTEXTPROC)GetProcAddress( hmod, "cgCreateContext" )))
        failed++;
    if(0 == (cgDestroyContext = (PFNCGDESTROYCONTEXTPROC)GetProcAddress( hmod, "cgDestroyContext" )))
        failed++;
    if(0 == (cgIsContext = (PFNCGISCONTEXTPROC)GetProcAddress( hmod, "cgIsContext" )))
        failed++;
    if(0 == (cgGetLastListing = (PFNCGGETLASTLISTINGPROC)GetProcAddress( hmod, "cgGetLastListing" )))
        failed++;
    if(0 == (cgCreateProgram = (PFNCGCREATEPROGRAMPROC)GetProcAddress( hmod, "cgCreateProgram" )))
        failed++;
    if(0 == (cgCreateProgramFromFile = (PFNCGCREATEPROGRAMFROMFILEPROC)GetProcAddress( hmod, "cgCreateProgramFromFile" )))
        failed++;
    if(0 == (cgCopyProgram = (PFNCGCOPYPROGRAMPROC)GetProcAddress( hmod, "cgCopyProgram" )))
        failed++;
    if(0 == (cgDestroyProgram = (PFNCGDESTROYPROGRAMPROC)GetProcAddress( hmod, "cgDestroyProgram" )))
        failed++;
    if(0 == (cgGetFirstProgram = (PFNCGGETFIRSTPROGRAMPROC)GetProcAddress( hmod, "cgGetFirstProgram" )))
        failed++;
    if(0 == (cgGetNextProgram = (PFNCGGETNEXTPROGRAMPROC)GetProcAddress( hmod, "cgGetNextProgram" )))
        failed++;
    if(0 == (cgGetProgramContext = (PFNCGGETPROGRAMCONTEXTPROC)GetProcAddress( hmod, "cgGetProgramContext" )))
        failed++;
    if(0 == (cgIsProgram = (PFNCGISPROGRAMPROC)GetProcAddress( hmod, "cgIsProgram" )))
        failed++;
    if(0 == (cgCompileProgram = (PFNCGCOMPILEPROGRAMPROC)GetProcAddress( hmod, "cgCompileProgram" )))
        failed++;
    if(0 == (cgIsProgramCompiled = (PFNCGISPROGRAMCOMPILEDPROC)GetProcAddress( hmod, "cgIsProgramCompiled" )))
        failed++;
    if(0 == (cgGetProgramString = (PFNCGGETPROGRAMSTRINGPROC)GetProcAddress( hmod, "cgGetProgramString" )))
        failed++;
    if(0 == (cgGetProgramProfile = (PFNCGGETPROGRAMPROFILEPROC)GetProcAddress( hmod, "cgGetProgramProfile" )))
        failed++;
    if(0 == (cgGetNamedParameter = (PFNCGGETNAMEDPARAMETERPROC)GetProcAddress( hmod, "cgGetNamedParameter" )))
        failed++;
    if(0 == (cgGetFirstParameter = (PFNCGGETFIRSTPARAMETERPROC)GetProcAddress( hmod, "cgGetFirstParameter" )))
        failed++;
    if(0 == (cgGetNextParameter = (PFNCGGETNEXTPARAMETERPROC)GetProcAddress( hmod, "cgGetNextParameter" )))
        failed++;
    if(0 == (cgGetFirstLeafParameter = (PFNCGGETFIRSTLEAFPARAMETERPROC)GetProcAddress( hmod, "cgGetFirstLeafParameter" )))
        failed++;
    if(0 == (cgGetNextLeafParameter = (PFNCGGETNEXTLEAFPARAMETERPROC)GetProcAddress( hmod, "cgGetNextLeafParameter" )))
        failed++;
    if(0 == (cgGetFirstStructParameter = (PFNCGGETFIRSTSTRUCTPARAMETERPROC)GetProcAddress( hmod, "cgGetFirstStructParameter" )))
        failed++;
    if(0 == (cgGetFirstDependentParameter = (PFNCGGETFIRSTDEPENDENTPARAMETERPROC)GetProcAddress( hmod, "cgGetFirstDependentParameter" )))
        failed++;
    if(0 == (cgGetArrayParameter = (PFNCGGETARRAYPARAMETERPROC)GetProcAddress( hmod, "cgGetArrayParameter" )))
        failed++;
    if(0 == (cgGetArrayDimension = (PFNCGGETARRAYDIMENSIONPROC)GetProcAddress( hmod, "cgGetArrayDimension" )))
        failed++;
    if(0 == (cgGetArraySize = (PFNCGGETARRAYSIZEPROC)GetProcAddress( hmod, "cgGetArraySize" )))
        failed++;
    if(0 == (cgGetParameterProgram = (PFNCGGETPARAMETERPROGRAMPROC)GetProcAddress( hmod, "cgGetParameterProgram" )))
        failed++;
    if(0 == (cgIsParameter = (PFNCGISPARAMETERPROC)GetProcAddress( hmod, "cgIsParameter" )))
        failed++;
    if(0 == (cgGetParameterName = (PFNCGGETPARAMETERNAMEPROC)GetProcAddress( hmod, "cgGetParameterName" )))
        failed++;
    if(0 == (cgGetParameterType = (PFNCGGETPARAMETERTYPEPROC)GetProcAddress( hmod, "cgGetParameterType" )))
        failed++;
    if(0 == (cgGetParameterSemantic = (PFNCGGETPARAMETERSEMANTICPROC)GetProcAddress( hmod, "cgGetParameterSemantic" )))
        failed++;
    if(0 == (cgGetParameterResource = (PFNCGGETPARAMETERRESOURCEPROC)GetProcAddress( hmod, "cgGetParameterResource" )))
        failed++;
    if(0 == (cgGetParameterBaseResource = (PFNCGGETPARAMETERBASERESOURCEPROC)GetProcAddress( hmod, "cgGetParameterBaseResource" )))
        failed++;
    if(0 == (cgGetParameterResourceIndex = (PFNCGGETPARAMETERRESOURCEINDEXPROC)GetProcAddress( hmod, "cgGetParameterResourceIndex" )))
        failed++;
    if(0 == (cgGetParameterVariability = (PFNCGGETPARAMETERVARIABILITYPROC)GetProcAddress( hmod, "cgGetParameterVariability" )))
        failed++;
    if(0 == (cgGetParameterDirection = (PFNCGGETPARAMETERDIRECTIONPROC)GetProcAddress( hmod, "cgGetParameterDirection" )))
        failed++;
    if(0 == (cgIsParameterReferenced = (PFNCGISPARAMETERREFERENCEDPROC)GetProcAddress( hmod, "cgIsParameterReferenced" )))
        failed++;
    if(0 == (cgGetParameterValues = (PFNCGGETPARAMETERVALUESPROC)GetProcAddress( hmod, "cgGetParameterValues" )))
        failed++;
    if(0 == (cgGetTypeString = (PFNCGGETTYPESTRINGPROC)GetProcAddress( hmod, "cgGetTypeString" )))
        failed++;
    if(0 == (cgGetType = (PFNCGGETTYPEPROC)GetProcAddress( hmod, "cgGetType" )))
        failed++;
    if(0 == (cgGetResourceString = (PFNCGGETRESOURCESTRINGPROC)GetProcAddress( hmod, "cgGetResourceString" )))
        failed++;
    if(0 == (cgGetResource = (PFNCGGETRESOURCEPROC)GetProcAddress( hmod, "cgGetResource" )))
        failed++;
    if(0 == (cgGetProfileString = (PFNCGGETPROFILESTRINGPROC)GetProcAddress( hmod, "cgGetProfileString" )))
        failed++;
    if(0 == (cgGetProfile = (PFNCGGETPROFILEPROC)GetProcAddress( hmod, "cgGetProfile" )))
        failed++;
    if(0 == (cgGetError = (PFNCGGETERRORPROC)GetProcAddress( hmod, "cgGetError" )))
        failed++;
    if(0 == (cgGetErrorString = (PFNCGGETERRORSTRINGPROC)GetProcAddress( hmod, "cgGetErrorString" )))
        failed++;
    if(0 == (cgSetErrorCallback = (PFNCGSETERRORCALLBACKPROC)GetProcAddress( hmod, "cgSetErrorCallback" )))
        failed++;
    if(0 == (cgGetErrorCallback = (PFNCGGETERRORCALLBACKPROC)GetProcAddress( hmod, "cgGetErrorCallback" )))
        failed++;

    
    
    hmod = LoadLibrary("cgGL.dll");

    
    
    if(0 == (cgGLIsProfileSupported = (PFNCGGLISPROFILESUPPORTEDPROC)GetProcAddress( hmod, "cgGLIsProfileSupported" )))
        failed++;
    if(0 == (cgGLEnableProfile = (PFNCGGLENABLEPROFILEPROC)GetProcAddress( hmod, "cgGLEnableProfile" )))
        failed++;
    if(0 == (cgGLDisableProfile = (PFNCGGLDISABLEPROFILEPROC)GetProcAddress( hmod, "cgGLDisableProfile" )))
        failed++;
    if(0 == (cgGLGetLatestProfile = (PFNCGGLGETLATESTPROFILEPROC)GetProcAddress( hmod, "cgGLGetLatestProfile" )))
        failed++;
    if(0 == (cgGLSetOptimalOptions = (PFNCGGLSETOPTIMALOPTIONSPROC)GetProcAddress( hmod, "cgGLSetOptimalOptions" )))
        failed++;
    if(0 == (cgGLLoadProgram = (PFNCGGLLOADPROGRAMPROC)GetProcAddress( hmod, "cgGLLoadProgram" )))
        failed++;
    if(0 == (cgGLBindProgram = (PFNCGGLBINDPROGRAMPROC)GetProcAddress( hmod, "cgGLBindProgram" )))
        failed++;
    if(0 == (cgGLSetParameter1f = (PFNCGGLSETPARAMETER1FPROC)GetProcAddress( hmod, "cgGLSetParameter1f" )))
        failed++;
    if(0 == (cgGLSetParameter2f = (PFNCGGLSETPARAMETER2FPROC)GetProcAddress( hmod, "cgGLSetParameter2f" )))
        failed++;
    if(0 == (cgGLSetParameter3f = (PFNCGGLSETPARAMETER3FPROC)GetProcAddress( hmod, "cgGLSetParameter3f" )))
        failed++;
    if(0 == (cgGLSetParameter4f = (PFNCGGLSETPARAMETER4FPROC)GetProcAddress( hmod, "cgGLSetParameter4f" )))
        failed++;
    if(0 == (cgGLSetParameter1fv = (PFNCGGLSETPARAMETER1FVPROC)GetProcAddress( hmod, "cgGLSetParameter1fv" )))
        failed++;
    if(0 == (cgGLSetParameter2fv = (PFNCGGLSETPARAMETER2FVPROC)GetProcAddress( hmod, "cgGLSetParameter2fv" )))
        failed++;
    if(0 == (cgGLSetParameter3fv = (PFNCGGLSETPARAMETER3FVPROC)GetProcAddress( hmod, "cgGLSetParameter3fv" )))
        failed++;
    if(0 == (cgGLSetParameter4fv = (PFNCGGLSETPARAMETER4FVPROC)GetProcAddress( hmod, "cgGLSetParameter4fv" )))
        failed++;
    if(0 == (cgGLSetParameter1d = (PFNCGGLSETPARAMETER1DPROC)GetProcAddress( hmod, "cgGLSetParameter1d" )))
        failed++;
    if(0 == (cgGLSetParameter2d = (PFNCGGLSETPARAMETER2DPROC)GetProcAddress( hmod, "cgGLSetParameter2d" )))
        failed++;
    if(0 == (cgGLSetParameter3d = (PFNCGGLSETPARAMETER3DPROC)GetProcAddress( hmod, "cgGLSetParameter3d" )))
        failed++;
    if(0 == (cgGLSetParameter4d = (PFNCGGLSETPARAMETER4DPROC)GetProcAddress( hmod, "cgGLSetParameter4d" )))
        failed++;
    if(0 == (cgGLSetParameter1dv = (PFNCGGLSETPARAMETER1DVPROC)GetProcAddress( hmod, "cgGLSetParameter1dv" )))
        failed++;
    if(0 == (cgGLSetParameter2dv = (PFNCGGLSETPARAMETER2DVPROC)GetProcAddress( hmod, "cgGLSetParameter2dv" )))
        failed++;
    if(0 == (cgGLSetParameter3dv = (PFNCGGLSETPARAMETER3DVPROC)GetProcAddress( hmod, "cgGLSetParameter3dv" )))
        failed++;
    if(0 == (cgGLSetParameter4dv = (PFNCGGLSETPARAMETER4DVPROC)GetProcAddress( hmod, "cgGLSetParameter4dv" )))
        failed++;
    if(0 == (cgGLSetParameter4dv = (PFNCGGLSETPARAMETER4DVPROC)GetProcAddress( hmod, "cgGLSetParameter4dv" )))
        failed++;
    if(0 == (cgGLGetParameter1f = (PFNCGGLGETPARAMETER1FPROC)GetProcAddress( hmod, "cgGLGetParameter1f" )))
        failed++;
    if(0 == (cgGLGetParameter2f = (PFNCGGLGETPARAMETER2FPROC)GetProcAddress( hmod, "cgGLGetParameter2f" )))
        failed++;
    if(0 == (cgGLGetParameter3f = (PFNCGGLGETPARAMETER3FPROC)GetProcAddress( hmod, "cgGLGetParameter3f" )))
        failed++;
    if(0 == (cgGLGetParameter4f = (PFNCGGLGETPARAMETER4FPROC)GetProcAddress( hmod, "cgGLGetParameter4f" )))
        failed++;
    if(0 == (cgGLGetParameter1d = (PFNCGGLGETPARAMETER1DPROC)GetProcAddress( hmod, "cgGLGetParameter1d" )))
        failed++;
    if(0 == (cgGLGetParameter2d = (PFNCGGLGETPARAMETER2DPROC)GetProcAddress( hmod, "cgGLGetParameter2d" )))
        failed++;
    if(0 == (cgGLGetParameter3d = (PFNCGGLGETPARAMETER3DPROC)GetProcAddress( hmod, "cgGLGetParameter3d" )))
        failed++;
    if(0 == (cgGLGetParameter4d = (PFNCGGLGETPARAMETER4DPROC)GetProcAddress( hmod, "cgGLGetParameter4d" )))
        failed++;
    if(0 == (cgGLSetParameterArray1f = (PFNCGGLSETPARAMETERARRAY1FPROC)GetProcAddress( hmod, "cgGLSetParameterArray1f" )))
        failed++;
    if(0 == (cgGLSetParameterArray2f = (PFNCGGLSETPARAMETERARRAY2FPROC)GetProcAddress( hmod, "cgGLSetParameterArray2f" )))
        failed++;
    if(0 == (cgGLSetParameterArray3f = (PFNCGGLSETPARAMETERARRAY3FPROC)GetProcAddress( hmod, "cgGLSetParameterArray3f" )))
        failed++;
    if(0 == (cgGLSetParameterArray4f = (PFNCGGLSETPARAMETERARRAY4FPROC)GetProcAddress( hmod, "cgGLSetParameterArray4f" )))
        failed++;
    if(0 == (cgGLSetParameterArray1d = (PFNCGGLSETPARAMETERARRAY1DPROC)GetProcAddress( hmod, "cgGLSetParameterArray1d" )))
        failed++;
    if(0 == (cgGLSetParameterArray2d = (PFNCGGLSETPARAMETERARRAY2DPROC)GetProcAddress( hmod, "cgGLSetParameterArray2d" )))
        failed++;
    if(0 == (cgGLSetParameterArray3d = (PFNCGGLSETPARAMETERARRAY3DPROC)GetProcAddress( hmod, "cgGLSetParameterArray3d" )))
        failed++;
    if(0 == (cgGLSetParameterArray4d = (PFNCGGLSETPARAMETERARRAY4DPROC)GetProcAddress( hmod, "cgGLSetParameterArray4d" )))
        failed++;
    if(0 == (cgGLGetParameterArray1f = (PFNCGGLGETPARAMETERARRAY1FPROC)GetProcAddress( hmod, "cgGLGetParameterArray1f" )))
        failed++;
    if(0 == (cgGLGetParameterArray2f = (PFNCGGLGETPARAMETERARRAY2FPROC)GetProcAddress( hmod, "cgGLGetParameterArray2f" )))
        failed++;
    if(0 == (cgGLGetParameterArray3f = (PFNCGGLGETPARAMETERARRAY3FPROC)GetProcAddress( hmod, "cgGLGetParameterArray3f" )))
        failed++;
    if(0 == (cgGLGetParameterArray4f = (PFNCGGLGETPARAMETERARRAY4FPROC)GetProcAddress( hmod, "cgGLGetParameterArray4f" )))
        failed++;
    if(0 == (cgGLGetParameterArray1d = (PFNCGGLGETPARAMETERARRAY1DPROC)GetProcAddress( hmod, "cgGLGetParameterArray1d" )))
        failed++;
    if(0 == (cgGLGetParameterArray2d = (PFNCGGLGETPARAMETERARRAY2DPROC)GetProcAddress( hmod, "cgGLGetParameterArray2d" )))
        failed++;
    if(0 == (cgGLGetParameterArray3d = (PFNCGGLGETPARAMETERARRAY3DPROC)GetProcAddress( hmod, "cgGLGetParameterArray3d" )))
        failed++;
    if(0 == (cgGLGetParameterArray4d = (PFNCGGLGETPARAMETERARRAY4DPROC)GetProcAddress( hmod, "cgGLGetParameterArray4d" )))
        failed++;
    if(0 == (cgGLSetParameterPointer = (PFNCGGLSETPARAMETERPOINTERPROC)GetProcAddress( hmod, "cgGLSetParameterPointer" )))
        failed++;
    if(0 == (cgGLEnableClientState = (PFNCGGLENABLECLIENTSTATEPROC)GetProcAddress( hmod, "cgGLEnableClientState" )))
        failed++;
    if(0 == (cgGLDisableClientState = (PFNCGGLDISABLECLIENTSTATEPROC)GetProcAddress( hmod, "cgGLDisableClientState" )))
        failed++;
    if(0 == (cgGLSetMatrixParameterdr = (PFNCGGLSETMATRIXPARAMETERDRPROC)GetProcAddress( hmod, "cgGLSetMatrixParameterdr" )))
        failed++;
    if(0 == (cgGLSetMatrixParameterfr = (PFNCGGLSETMATRIXPARAMETERFRPROC)GetProcAddress( hmod, "cgGLSetMatrixParameterfr" )))
        failed++;
    if(0 == (cgGLSetMatrixParameterdc = (PFNCGGLSETMATRIXPARAMETERDCPROC)GetProcAddress( hmod, "cgGLSetMatrixParameterdc" )))
        failed++;
    if(0 == (cgGLSetMatrixParameterfc = (PFNCGGLSETMATRIXPARAMETERFCPROC)GetProcAddress( hmod, "cgGLSetMatrixParameterfc" )))
        failed++;
    if(0 == (cgGLGetMatrixParameterdr = (PFNCGGLGETMATRIXPARAMETERDRPROC)GetProcAddress( hmod, "cgGLGetMatrixParameterdr" )))
        failed++;
    if(0 == (cgGLGetMatrixParameterfr = (PFNCGGLGETMATRIXPARAMETERFRPROC)GetProcAddress( hmod, "cgGLGetMatrixParameterfr" )))
        failed++;
    if(0 == (cgGLGetMatrixParameterdc = (PFNCGGLGETMATRIXPARAMETERDCPROC)GetProcAddress( hmod, "cgGLGetMatrixParameterdc" )))
        failed++;
    if(0 == (cgGLGetMatrixParameterfc = (PFNCGGLGETMATRIXPARAMETERFCPROC)GetProcAddress( hmod, "cgGLGetMatrixParameterfc" )))
        failed++;
    if(0 == (cgGLSetStateMatrixParameter = (PFNCGGLSETSTATEMATRIXPARAMETERPROC)GetProcAddress( hmod, "cgGLSetStateMatrixParameter" )))
        failed++;
    //if(0 == (cgGLSetMatrixParameterArrayfc = (PFNCGGLSETMATRIXPARAMETERARRAYFCPROC)GetProcAddress( hmod, "cgGLSetMatrixParameterArrayfc" )))
     //   failed++;
    //if(0 == (cgGLSetMatrixParameterArrayfr = (PFNCGGLSETMATRIXPARAMETERARRAYFRPROC)GetProcAddress( hmod, "cgGLSetMatrixParameterArrayfr" )))
     //   failed++;
    //if(0 == (cgGLSetMatrixParameterArraydc = (PFNCGGLSETMATRIXPARAMETERARRAYDCPROC)GetProcAddress( hmod, "cgGLSetMatrixParameterArraydc" )))
     //   failed++;
    //if(0 == (cgGLSetMatrixParameterArraydr = (PFNCGGLSETMATRIXPARAMETERARRAYDRPROC)GetProcAddress( hmod, "cgGLSetMatrixParameterArraydr" )))
     //   failed++;
    //if(0 == (cgGLGetMatrixParameterArrayfc = (PFNCGGLGETMATRIXPARAMETERARRAYFCPROC)GetProcAddress( hmod, "cgGLGetMatrixParameterArrayfc" )))
     //   failed++;
    //if(0 == (cgGLGetMatrixParameterArrayfr = (PFNCGGLGETMATRIXPARAMETERARRAYFRPROC)GetProcAddress( hmod, "cgGLGetMatrixParameterArrayfr" )))
     //   failed++;
    //if(0 == (cgGLGetMatrixParameterArraydc = (PFNCGGLGETMATRIXPARAMETERARRAYDCPROC)GetProcAddress( hmod, "cgGLGetMatrixParameterArraydc" )))
     //   failed++;
    //if(0 == (cgGLGetMatrixParameterArraydr = (PFNCGGLGETMATRIXPARAMETERARRAYDRPROC)GetProcAddress( hmod, "cgGLGetMatrixParameterArraydr" )))
     //   failed++;
    if(0 == (cgGLSetTextureParameter = (PFNCGGLSETTEXTUREPARAMETERPROC)GetProcAddress( hmod, "cgGLSetTextureParameter" )))
        failed++;
    if(0 == (cgGLGetTextureParameter = (PFNCGGLGETTEXTUREPARAMETERPROC)GetProcAddress( hmod, "cgGLGetTextureParameter" )))
        failed++;
    if(0 == (cgGLEnableTextureParameter = (PFNCGGLENABLETEXTUREPARAMETERPROC)GetProcAddress( hmod, "cgGLEnableTextureParameter" )))
        failed++;
    if(0 == (cgGLDisableTextureParameter = (PFNCGGLDISABLETEXTUREPARAMETERPROC)GetProcAddress( hmod, "cgGLDisableTextureParameter" )))
        failed++;
    if(0 == (cgGLGetTextureEnum = (PFNCGGLGETTEXTUREENUMPROC)GetProcAddress( hmod, "cgGLGetTextureEnum" )))
        failed++;
    
    return failed == 0;   
    
}
#endif

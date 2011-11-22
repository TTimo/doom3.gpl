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

#ifndef __MATH_SIMD_ALTIVEC_H__
#define __MATH_SIMD_ALTIVEC_H__

/*
===============================================================================

	AltiVec implementation of idSIMDProcessor

===============================================================================
*/

// Defines for enabling parts of the library

// Turns on/off the simple math routines (add, sub, div, etc)
#define ENABLE_SIMPLE_MATH	

// Turns on/off the dot routines
#define ENABLE_DOT

// Turns on/off the compare routines
#define ENABLE_COMPARES

// The MinMax routines introduce a couple of bugs. In the bathroom of the alphalabs2 map, the
// wrong surface appears in the mirror at times. It also introduces a noticable delay when map
// data is loaded such as going through doors.
// Turns on/off MinMax routines
//#define ENABLE_MINMAX

// Turns on/off Clamp routines
#define ENABLE_CLAMP

// Turns on/off XXX16 routines
#define ENABLE_16ROUTINES

// Turns on/off LowerTriangularSolve, LowerTriangularSolveTranspose, and MatX_LDLTFactor
#define ENABLE_LOWER_TRIANGULAR

// Turns on/off TracePointCull, DecalPointCull, and OverlayPoint
// The Enable_Cull routines breaks the g_decals functionality, DecalPointCull is
// the likely suspect. Bullet holes do not appear on the walls when this optimization
// is enabled. 
//#define ENABLE_CULL

// Turns on/off DeriveTriPlanes, DeriveTangents, DeriveUnsmoothedTangents, NormalizeTangents
#define ENABLE_DERIVE

// Turns on/off CreateTextureSpaceLightVectors, CreateShadowCache, CreateVertexProgramShadowCache
#define ENABLE_CREATE

// Turns on/off the sound routines
#define ENABLE_SOUND_ROUTINES

// Turns on/off the stuff that isn't on elsewhere
// Currently: BlendJoints, TransformJoints, UntransformJoints, ConvertJointQuatsToJointMats, and
// ConvertJointMatsToJointQuats
#define LIVE_VICARIOUSLY

// This assumes that the dest (and mixBuffer) array to the sound functions is aligned. If this is not true, we take a large
// performance hit from having to do unaligned stores
//#define SOUND_DEST_ALIGNED

// This assumes that the vertexCache array to CreateShadowCache and CreateVertexProgramShadowCache is aligned. If it's not,
// then we take a big performance hit from unaligned stores.
//#define VERTEXCACHE_ALIGNED

// This turns on support for PPC intrinsics in the SIMD_AltiVec.cpp file. Right now it's only used for frsqrte. GCC 
// supports these intrinsics but XLC does not.
#define PPC_INTRINSICS

// This assumes that the idDrawVert array that is used in DeriveUnsmoothedTangents is aligned. If its not aligned,
// then we don't get any speedup
//#define DERIVE_UNSMOOTH_DRAWVERT_ALIGNED

// Disable DRAWVERT_PADDED since we disabled the ENABLE_CULL optimizations and the default
// implementation does not allow for the extra padding.
// This assumes that idDrawVert has been padded by 4 bytes so that xyz always starts at an aligned
// address
//#define DRAWVERT_PADDED

class idSIMD_AltiVec : public idSIMD_Generic {
#if defined(MACOS_X) && defined(__ppc__)
public:

	virtual const char * VPCALL GetName( void ) const;

#ifdef ENABLE_SIMPLE_MATH
	// Basic math, works for both aligned and unaligned data
	virtual void VPCALL Add( float *dst, const float constant, const float *src, const int count );
    virtual void VPCALL Add( float *dst, const float *src0, const float *src1, const int count );
	virtual void VPCALL Sub( float *dst, const float constant, const float *src, const int count );
	virtual void VPCALL Sub( float *dst, const float *src0, const float *src1, const int count );
 	virtual void VPCALL Mul( float *dst, const float constant, const float *src, const int count);
	virtual void VPCALL Mul( float *dst, const float *src0, const float *src1, const int count );
	virtual void VPCALL Div( float *dst, const float constant, const float *divisor, const int count );
	virtual void VPCALL Div( float *dst, const float *src0, const float *src1, const int count ); 
	virtual void VPCALL MulAdd( float *dst, const float constant, const float *src, const int count );
	virtual void VPCALL MulAdd( float *dst, const float *src0, const float *src1, const int count );
	virtual void VPCALL MulSub( float *dst, const float constant, const float *src, const int count );
	virtual void VPCALL MulSub( float *dst, const float *src0, const float *src1, const int count ); 
#endif

#ifdef ENABLE_DOT
	// Dot products, expects data structures in contiguous memory
	virtual void VPCALL Dot( float *dst,			const idVec3 &constant,	const idVec3 *src,		const int count );
	virtual void VPCALL Dot( float *dst,			const idVec3 &constant,	const idPlane *src,		const int count );
	virtual void VPCALL Dot( float *dst,			const idVec3 &constant,	const idDrawVert *src,	const int count );
	virtual void VPCALL Dot( float *dst,			const idPlane &constant,const idVec3 *src,		const int count );
	virtual void VPCALL Dot( float *dst,			const idPlane &constant,const idPlane *src,		const int count );
	virtual void VPCALL Dot( float *dst,			const idPlane &constant,const idDrawVert *src,	const int count );
	virtual void VPCALL Dot( float *dst,			const idVec3 *src0,		const idVec3 *src1,		const int count );
	virtual void VPCALL Dot( float &dot,			const float *src1,		const float *src2,		const int count );
#endif

#ifdef ENABLE_COMPARES
	// Comparisons, works for both aligned and unaligned data
	virtual void VPCALL CmpGT( byte *dst,			const float *src0,		const float constant,	const int count );
	virtual void VPCALL CmpGT( byte *dst,			const byte bitNum,		const float *src0,		const float constant,	const int count );
	virtual void VPCALL CmpGE( byte *dst,			const float *src0,		const float constant,	const int count );
	virtual void VPCALL CmpGE( byte *dst,			const byte bitNum,		const float *src0,		const float constant,	const int count );
	virtual void VPCALL CmpLT( byte *dst,			const float *src0,		const float constant,	const int count );
	virtual void VPCALL CmpLT( byte *dst,			const byte bitNum,		const float *src0,		const float constant,	const int count );
	virtual void VPCALL CmpLE( byte *dst,			const float *src0,		const float constant,	const int count );
	virtual void VPCALL CmpLE( byte *dst,			const byte bitNum,		const float *src0,		const float constant,	const int count );
#endif

#ifdef ENABLE_MINMAX	
	// Min/Max. Expects data structures in contiguous memory
	virtual void VPCALL MinMax( float &min,			float &max,				const float *src,		const int count );
	virtual	void VPCALL MinMax( idVec2 &min,		idVec2 &max,			const idVec2 *src,		const int count );
	virtual void VPCALL MinMax( idVec3 &min,		idVec3 &max,			const idVec3 *src,		const int count );
	virtual	void VPCALL MinMax( idVec3 &min,		idVec3 &max,			const idDrawVert *src,	const int count );
	virtual	void VPCALL MinMax( idVec3 &min,		idVec3 &max,			const idDrawVert *src,	const int *indexes,		const int count );
#endif

#ifdef ENABLE_CLAMP
	// Clamp operations. Works for both aligned and unaligned data
	virtual void VPCALL Clamp( float *dst,			const float *src,		const float min,		const float max,		const int count );
	virtual void VPCALL ClampMin( float *dst,		const float *src,		const float min,		const int count );
	virtual void VPCALL ClampMax( float *dst,		const float *src,		const float max,		const int count );
#endif

    // These are already using memcpy and memset functions. Leaving default implementation
//	virtual void VPCALL Memcpy( void *dst,			const void *src,		const int count );
//	virtual void VPCALL Memset( void *dst,			const int val,			const int count );

#ifdef ENABLE_16ROUTINES
	// Operations that expect 16-byte aligned data and 16-byte padded memory (with zeros), generally faster
	virtual void VPCALL Zero16( float *dst,			const int count );
	virtual void VPCALL Negate16( float *dst,		const int count );
	virtual void VPCALL Copy16( float *dst,			const float *src,		const int count );
	virtual void VPCALL Add16( float *dst,			const float *src1,		const float *src2,		const int count );
	virtual void VPCALL Sub16( float *dst,			const float *src1,		const float *src2,		const int count );
	virtual void VPCALL Mul16( float *dst,			const float *src1,		const float constant,	const int count );
	virtual void VPCALL AddAssign16( float *dst,	const float *src,		const int count );
	virtual void VPCALL SubAssign16( float *dst,	const float *src,		const int count );
	virtual void VPCALL MulAssign16( float *dst,	const float constant,	const int count );
#endif

//  Most of these deal with tiny matrices or vectors, generally not worth altivec'ing since
//  the scalar code is already really fast

//	virtual void VPCALL MatX_MultiplyVecX( idVecX &dst, const idMatX &mat, const idVecX &vec );
//	virtual void VPCALL MatX_MultiplyAddVecX( idVecX &dst, const idMatX &mat, const idVecX &vec );
//	virtual void VPCALL MatX_MultiplySubVecX( idVecX &dst, const idMatX &mat, const idVecX &vec );
//	virtual void VPCALL MatX_TransposeMultiplyVecX( idVecX &dst, const idMatX &mat, const idVecX &vec );
//	virtual void VPCALL MatX_TransposeMultiplyAddVecX( idVecX &dst, const idMatX &mat, const idVecX &vec );
//	virtual void VPCALL MatX_TransposeMultiplySubVecX( idVecX &dst, const idMatX &mat, const idVecX &vec );
//	virtual void VPCALL MatX_MultiplyMatX( idMatX &dst, const idMatX &m1, const idMatX &m2 );
//	virtual void VPCALL MatX_TransposeMultiplyMatX( idMatX &dst, const idMatX &m1, const idMatX &m2 );

#ifdef ENABLE_LOWER_TRIANGULAR
	virtual void VPCALL MatX_LowerTriangularSolve( const idMatX &L, float *x, const float *b, const int n, int skip = 0 );
	virtual void VPCALL MatX_LowerTriangularSolveTranspose( const idMatX &L, float *x, const float *b, const int n );
	virtual bool VPCALL MatX_LDLTFactor( idMatX &mat, idVecX &invDiag, const int n );
#endif
#ifdef LIVE_VICARIOUSLY
	virtual void VPCALL BlendJoints( idJointQuat *joints, const idJointQuat *blendJoints, const float lerp, const int *index, const int numJoints );
	virtual void VPCALL ConvertJointQuatsToJointMats( idJointMat *jointMats, const idJointQuat *jointQuats, const int numJoints );
	virtual void VPCALL ConvertJointMatsToJointQuats( idJointQuat *jointQuats, const idJointMat *jointMats, const int numJoints );
#endif

#ifdef LIVE_VICARIOUSLY
	virtual void VPCALL TransformJoints( idJointMat *jointMats, const int *parents, const int firstJoint, const int lastJoint );
	virtual void VPCALL UntransformJoints( idJointMat *jointMats, const int *parents, const int firstJoint, const int lastJoint );
	virtual void VPCALL TransformVerts( idDrawVert *verts, const int numVerts, const idJointMat *joints, const idVec4 *weights, const int *index, const int numWeights );
#endif

#ifdef ENABLE_CULL
	virtual void VPCALL TracePointCull( byte *cullBits, byte &totalOr, const float radius, const idPlane *planes, const idDrawVert *verts, const int numVerts );
	virtual void VPCALL DecalPointCull( byte *cullBits, const idPlane *planes, const idDrawVert *verts, const int numVerts );
	virtual void VPCALL OverlayPointCull( byte *cullBits, idVec2 *texCoords, const idPlane *planes, const idDrawVert *verts, const int numVerts );
#endif

#ifdef ENABLE_DERIVE
	virtual void VPCALL DeriveTriPlanes( idPlane *planes, const idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes );
	virtual void VPCALL DeriveTangents( idPlane *planes, idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes );
	virtual void VPCALL DeriveUnsmoothedTangents( idDrawVert *verts, const dominantTri_s *dominantTris, const int numVerts );
	virtual void VPCALL NormalizeTangents( idDrawVert *verts, const int numVerts );
#endif	

#ifdef ENABLE_CREATE
	virtual void VPCALL CreateTextureSpaceLightVectors( idVec3 *lightVectors, const idVec3 &lightOrigin, const idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes );
	virtual void VPCALL CreateSpecularTextureCoords( idVec4 *texCoords, const idVec3 &lightOrigin, const idVec3 &viewOrigin, const idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes );
	virtual int  VPCALL CreateShadowCache( idVec4 *vertexCache, int *vertRemap, const idVec3 &lightOrigin, const idDrawVert *verts, const int numVerts );
	virtual int  VPCALL CreateVertexProgramShadowCache( idVec4 *vertexCache, const idDrawVert *verts, const int numVerts );
#endif

#ifdef ENABLE_SOUND_ROUTINES
	// Sound upsampling and mixing routines, works for aligned and unaligned data
	virtual void VPCALL UpSamplePCMTo44kHz( float *dest, const short *pcm, const int numSamples, const int kHz, const int numChannels );
	virtual void VPCALL UpSampleOGGTo44kHz( float *dest, const float * const *ogg, const int numSamples, const int kHz, const int numChannels );
	virtual void VPCALL MixSoundTwoSpeakerMono( float *mixBuffer, const float *samples, const int numSamples, const float lastV[2], const float currentV[2] );
	virtual void VPCALL MixSoundTwoSpeakerStereo( float *mixBuffer, const float *samples, const int numSamples, const float lastV[2], const float currentV[2] );
	virtual void VPCALL MixSoundSixSpeakerMono( float *mixBuffer, const float *samples, const int numSamples, const float lastV[6], const float currentV[6] );
	virtual void VPCALL MixSoundSixSpeakerStereo( float *mixBuffer, const float *samples, const int numSamples, const float lastV[6], const float currentV[6] );
	virtual void VPCALL MixedSoundToSamples( short *samples, const float *mixBuffer, const int numSamples );
#endif
#endif

};

#endif /* !__MATH_SIMD_ALTIVEC_H__ */

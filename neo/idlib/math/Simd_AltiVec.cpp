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


#include "../precompiled.h"
#pragma hdrstop

#include "Simd_Generic.h"
#include "Simd_AltiVec.h"
#include <math.h>
#include <float.h>

#ifdef PPC_INTRINSICS
	#include <ppc_intrinsics.h>
#endif

// Doom3 SIMD Library version 0.5
// Patrick Flanagan (pflanagan@apple.com)
// Sanjay Patel (spatel@apple.com)
// Architecture & Performance Group, Apple Computer


//===============================================================
//
//	AltiVec implementation of idSIMDProcessor
//
//===============================================================

#if defined(MACOS_X) && defined(__ppc__)

// Data struct sizes

#ifndef DRAWVERT_PADDED
	// 60 bytes, 15 floats at 4 bytes each
	#define DRAWVERT_OFFSET 15
#else
	// 64 bytes, 16 floats
	#define DRAWVERT_OFFSET 16
#endif
// 16 bytes each, 4 floats
#define PLANE_OFFSET 4
// 16 bytes each, 4 floats
#define IDVEC4_OFFSET 4

// Alignment tests
#define IS_16BYTE_ALIGNED( x ) ( ( (unsigned long)&x & 0x0F ) == 0 )	
#define NOT_16BYTE_ALIGNED( x ) ( ( (unsigned long)&x & 0x0F) != 0 )

// Aligned storing floats
#define ALIGNED_STORE2( ADDR, V0, V1 )			\
	vec_st( V0, 0, ADDR );						\
	vec_st( V1, 16, ADDR )	
	
#define ALIGNED_STORE3( ADDR, V0, V1, V2 )		\
	vec_st( V0, 0, ADDR );						\
	vec_st( V1, 16, ADDR );						\
	vec_st( V2, 32, ADDR )
		
#define ALIGNED_STORE4( ADDR, V0, V1, V2, V3 )  \
	vec_st( V0, 0, ADDR );						\
	vec_st( V1, 16, ADDR );						\
	vec_st( V2, 32, ADDR );						\
	vec_st( V3, 48, ADDR )

#define ALIGNED_STORE6( ADDR, V0, V1, V2, V3, V4, V5 )  \
	vec_st( V0, 0, ADDR );						\
	vec_st( V1, 16, ADDR );						\
	vec_st( V2, 32, ADDR );						\
	vec_st( V3, 48, ADDR );						\
	vec_st( V4, 64, ADDR );						\
	vec_st( V5, 80, ADDR )
	
#define ALIGNED_STORE8( ADDR, V0, V1, V2, V3, V4, V5, V6, V7 )  \
	vec_st( V0, 0, ADDR );						\
	vec_st( V1, 16, ADDR );						\
	vec_st( V2, 32, ADDR );						\
	vec_st( V3, 48, ADDR );						\
	vec_st( V4, 64, ADDR );						\
	vec_st( V5, 80, ADDR );						\
	vec_st( V6, 96, ADDR );						\
	vec_st( V7, 112, ADDR )			
				
// Unaligned storing floats. These assume that we can trash the input
#define UNALIGNED_STORE1( ADDR, V0 ) { \
	/* use store element */				\
	vector unsigned char ULStoreMacroPerm = vec_lvsr( 0, ADDR );	\
	V0 = vec_perm( V0, V0, ULStoreMacroPerm );						\
	vec_ste( V0, 0, ADDR );		\
	vec_ste( V0, 4, ADDR );		\
	vec_ste( V0, 8, ADDR );		\
	vec_ste( V0, 12, ADDR );	\
	}	

#define UNALIGNED_STORE2( ADDR, V0, V1 )	{		\
	/* load up the values that are there now */			\
	vector float ULStoreMacro1 = vec_ld( 0, ADDR );		\
	vector float ULStoreMacro2 = vec_ld( 31, ADDR );	\
	/* generate permute vector and mask	*/				\
	vector unsigned char ULStoreMacroPerm = vec_sub( vec_lvsr( 15, ADDR ), (vector unsigned char)(1) ); \
	vector unsigned int ULStoreMacroMask = vec_perm( (vector unsigned int)(0), (vector unsigned int)(-1), ULStoreMacroPerm ); \
	/* right rotate input data	*/   \
	V0 = vec_perm( V0, V0, ULStoreMacroPerm );	\
	V1 = vec_perm( V1, V1, ULStoreMacroPerm );	\
	/* setup the output vectors		*/			\
	vector float ULStoreVal1, ULStoreVal2, ULStoreVal3;	\
	ULStoreVal1 = vec_sel( ULStoreMacro1, V0, ULStoreMacroMask );	\
	ULStoreVal2 = vec_sel( V0, V1, ULStoreMacroMask );	\
	ULStoreVal3 = vec_sel( V1, ULStoreMacro2, ULStoreMacroMask );	\
	/* store results	*/					\
	vec_st( ULStoreVal1, 0, ADDR );			\
	vec_st( ULStoreVal2, 15, ADDR );		\
	vec_st( ULStoreVal3, 31, ADDR ); }

#define UNALIGNED_STORE3( ADDR, V0, V1, V2 )	{		\
	/* load up the values that are there now */			\
	vector float ULStoreMacro1 = vec_ld( 0, ADDR );		\
	vector float ULStoreMacro2 = vec_ld( 47, ADDR );	\
	/* generate permute vector and mask	*/				\
	vector unsigned char ULStoreMacroPerm = vec_sub( vec_lvsr( 15, ADDR ), (vector unsigned char)(1) ); \
	vector unsigned int ULStoreMacroMask = vec_perm( (vector unsigned int)(0), (vector unsigned int)(-1), ULStoreMacroPerm ); \
	/* right rotate input data	*/   \
	V0 = vec_perm( V0, V0, ULStoreMacroPerm );	\
	V1 = vec_perm( V1, V1, ULStoreMacroPerm );	\
	V2 = vec_perm( V2, V2, ULStoreMacroPerm );	\
	/* setup the output vectors		*/			\
	vector float ULStoreVal1, ULStoreVal2, ULStoreVal3, ULStoreVal4;	\
	ULStoreVal1 = vec_sel( ULStoreMacro1, V0, ULStoreMacroMask );	\
	ULStoreVal2 = vec_sel( V0, V1, ULStoreMacroMask );	\
	ULStoreVal3 = vec_sel( V1, V2, ULStoreMacroMask );	\
	ULStoreVal4 = vec_sel( V2, ULStoreMacro2, ULStoreMacroMask );	\
	/* store results	*/					\
	vec_st( ULStoreVal1, 0, ADDR );			\
	vec_st( ULStoreVal2, 15, ADDR );		\
	vec_st( ULStoreVal3, 31, ADDR );		\
	vec_st( ULStoreVal4, 47, ADDR ); }

#define UNALIGNED_STORE4( ADDR, V0, V1, V2, V3 )	{		\
	/* load up the values that are there now */			\
	vector float ULStoreMacro1 = vec_ld( 0, ADDR );		\
	vector float ULStoreMacro2 = vec_ld( 63, ADDR );	\
	/* generate permute vector and mask	*/				\
	vector unsigned char ULStoreMacroPerm = vec_sub( vec_lvsr( 15, ADDR ), (vector unsigned char)(1) ); \
	vector unsigned int ULStoreMacroMask = vec_perm( (vector unsigned int)(0), (vector unsigned int)(-1), ULStoreMacroPerm ); \
	/* right rotate input data	*/   \
	V0 = vec_perm( V0, V0, ULStoreMacroPerm );	\
	V1 = vec_perm( V1, V1, ULStoreMacroPerm );	\
	V2 = vec_perm( V2, V2, ULStoreMacroPerm );	\
	V3 = vec_perm( V3, V3, ULStoreMacroPerm );	\
	/* setup the output vectors		*/			\
	vector float ULStoreVal1, ULStoreVal2, ULStoreVal3, ULStoreVal4, ULStoreVal5;	\
	ULStoreVal1 = vec_sel( ULStoreMacro1, V0, ULStoreMacroMask );	\
	ULStoreVal2 = vec_sel( V0, V1, ULStoreMacroMask );	\
	ULStoreVal3 = vec_sel( V1, V2, ULStoreMacroMask );	\
	ULStoreVal4 = vec_sel( V2, V3, ULStoreMacroMask );	\
	ULStoreVal5 = vec_sel( V3, ULStoreMacro2, ULStoreMacroMask );	\
	/* store results	*/					\
	vec_st( ULStoreVal1, 0, ADDR );			\
	vec_st( ULStoreVal2, 15, ADDR );		\
	vec_st( ULStoreVal3, 31, ADDR );		\
	vec_st( ULStoreVal4, 47, ADDR );		\
	vec_st( ULStoreVal5, 63, ADDR );	}

#define UNALIGNED_STORE6( ADDR, V0, V1, V2, V3, V4, V5 )	{		\
	/* load up the values that are there now */			\
	vector float ULStoreMacro1 = vec_ld( 0, ADDR );		\
	vector float ULStoreMacro2 = vec_ld( 95, ADDR );	\
	/* generate permute vector and mask	*/				\
	vector unsigned char ULStoreMacroPerm = vec_sub( vec_lvsr( 15, ADDR ), (vector unsigned char)(1) ); \
	vector unsigned int ULStoreMacroMask = vec_perm( (vector unsigned int)(0), (vector unsigned int)(-1), ULStoreMacroPerm ); \
	/* right rotate input data	*/   \
	V0 = vec_perm( V0, V0, ULStoreMacroPerm );	\
	V1 = vec_perm( V1, V1, ULStoreMacroPerm );	\
	V2 = vec_perm( V2, V2, ULStoreMacroPerm );	\
	V3 = vec_perm( V3, V3, ULStoreMacroPerm );	\
	V4 = vec_perm( V4, V4, ULStoreMacroPerm );	\
	V5 = vec_perm( V5, V5, ULStoreMacroPerm );	\
	/* setup the output vectors		*/			\
	vector float ULStoreVal1, ULStoreVal2, ULStoreVal3, ULStoreVal4, ULStoreVal5, ULStoreVal6, ULStoreVal7;	\
	ULStoreVal1 = vec_sel( ULStoreMacro1, V0, ULStoreMacroMask );	\
	ULStoreVal2 = vec_sel( V0, V1, ULStoreMacroMask );	\
	ULStoreVal3 = vec_sel( V1, V2, ULStoreMacroMask );	\
	ULStoreVal4 = vec_sel( V2, V3, ULStoreMacroMask );	\
	ULStoreVal5 = vec_sel( V3, V4, ULStoreMacroMask );	\
	ULStoreVal6 = vec_sel( V4, V5, ULStoreMacroMask );	\
	ULStoreVal7 = vec_sel( V5, ULStoreMacro2, ULStoreMacroMask );	\
	/* store results	*/					\
	vec_st( ULStoreVal1, 0, ADDR );			\
	vec_st( ULStoreVal2, 15, ADDR );		\
	vec_st( ULStoreVal3, 31, ADDR );		\
	vec_st( ULStoreVal4, 47, ADDR );		\
	vec_st( ULStoreVal5, 63, ADDR );		\
	vec_st( ULStoreVal6, 79, ADDR );		\
	vec_st( ULStoreVal7, 95, ADDR );	}

#define UNALIGNED_STORE9( ADDR, V0, V1, V2, V3, V4, V5, V6, V7, V8 )	{		\
	/* load up the values that are there now */			\
	vector float ULStoreMacro1 = vec_ld( 0, ADDR );		\
	vector float ULStoreMacro2 = vec_ld( 143, ADDR );	\
	/* generate permute vector and mask	*/				\
	vector unsigned char ULStoreMacroPerm = vec_sub( vec_lvsr( 15, ADDR ), (vector unsigned char)(1) ); \
	vector unsigned int ULStoreMacroMask = vec_perm( (vector unsigned int)(0), (vector unsigned int)(-1), ULStoreMacroPerm ); \
	/* right rotate input data	*/   \
	V0 = vec_perm( V0, V0, ULStoreMacroPerm );	\
	V1 = vec_perm( V1, V1, ULStoreMacroPerm );	\
	V2 = vec_perm( V2, V2, ULStoreMacroPerm );	\
	V3 = vec_perm( V3, V3, ULStoreMacroPerm );	\
	V4 = vec_perm( V4, V4, ULStoreMacroPerm );	\
	V5 = vec_perm( V5, V5, ULStoreMacroPerm );	\
	V6 = vec_perm( V6, V6, ULStoreMacroPerm );	\
	V7 = vec_perm( V7, V7, ULStoreMacroPerm );	\
	V8 = vec_perm( V8, V8, ULStoreMacroPerm );	\
	/* setup the output vectors		*/			\
	vector float ULStoreVal1, ULStoreVal2, ULStoreVal3, ULStoreVal4, ULStoreVal5, ULStoreVal6, ULStoreVal7;	\
	vector float ULStoreVal8, ULStoreVal9, ULStoreVal10;	\
	ULStoreVal1 = vec_sel( ULStoreMacro1, V0, ULStoreMacroMask );	\
	ULStoreVal2 = vec_sel( V0, V1, ULStoreMacroMask );	\
	ULStoreVal3 = vec_sel( V1, V2, ULStoreMacroMask );	\
	ULStoreVal4 = vec_sel( V2, V3, ULStoreMacroMask );	\
	ULStoreVal5 = vec_sel( V3, V4, ULStoreMacroMask );	\
	ULStoreVal6 = vec_sel( V4, V5, ULStoreMacroMask );	\
	ULStoreVal7 = vec_sel( V5, V6, ULStoreMacroMask );	\
	ULStoreVal8 = vec_sel( V6, V7, ULStoreMacroMask );	\
	ULStoreVal9 = vec_sel( V7, V8, ULStoreMacroMask );	\
	ULStoreVal10 = vec_sel( V8, ULStoreMacro2, ULStoreMacroMask );	\
	/* store results	*/					\
	vec_st( ULStoreVal1, 0, ADDR );			\
	vec_st( ULStoreVal2, 15, ADDR );		\
	vec_st( ULStoreVal3, 31, ADDR );		\
	vec_st( ULStoreVal4, 47, ADDR );		\
	vec_st( ULStoreVal5, 63, ADDR );		\
	vec_st( ULStoreVal6, 79, ADDR );		\
	vec_st( ULStoreVal7, 95, ADDR );		\
	vec_st( ULStoreVal8, 111, ADDR );		\
	vec_st( ULStoreVal9, 127, ADDR );		\
	vec_st( ULStoreVal10, 143, ADDR );	}

/*
============
idSIMD_AltiVec::GetName
============
*/
const char *idSIMD_AltiVec::GetName( void ) const {
	return "AltiVec";
}

/*
	Helper Functions
*/
#if 0
// Prints the values of a vector, useful for debugging but
// should never be called in real code
inline void debugPrintVector( vector float v, char *msg ) {
	printf("%s -- %vf\n", msg, v );
}

inline void debugPrintVector( vector unsigned int v, char *msg ) {
	printf("%s -- %vd\n", msg, v );
}

inline void debugPrintVector( vector bool int v, char *msg ) {
	printf("%s -- %vi\n", msg, v );
}

inline void debugPrintVector( vector unsigned char v, char *msg ) {
	printf("%s -- %vuc\n", msg, v );
}

inline void debugPrintVector( vector unsigned short v, char *msg ) {
	printf("%s -- %vs\n", msg, v );
}
#endif
/*
===============
  Reciprocal

  For each element in vector:
	n = 1 / n
===============
*/ 
   
// Use Newton-Raphson to calculate reciprocal of a vector
inline vector float Reciprocal( vector float v ) {
  //Get the reciprocal estimate
  vector float estimate = vec_re( v );
  //One round of Newton-Raphson refinement
  return vec_madd( vec_nmsub( estimate, v, (vector float) (1.0) ), estimate, estimate );
}

/*
===============
  ReciprocalSquareRoot

  For each element in vector:
	n = 1 / sqrt(n)
===============
*/
// Reciprocal square root estimate of a vector 
inline vector float ReciprocalSquareRoot( vector float v ) {
	//Get the square root reciprocal estimate
	vector float zero = (vector float)(0);
	vector float oneHalf = (vector float)(0.5);
	vector float one = (vector float)(1.0);
	vector float estimate = vec_rsqrte( vec_max( v, (vector float)(FLT_MIN) ) );
	
	//One round of Newton-Raphson refinement
	vector float estimateSquared = vec_madd( estimate, estimate, zero );
	vector float halfEstimate = vec_madd( estimate, oneHalf, zero );
	return vec_madd( vec_nmsub( v, estimateSquared, one ), halfEstimate, estimate );
}


/*
===============
  Divide

  For each element in vectors:
	n = a / b
===============
*/
// Use reciprocal estimate and multiply to divide a vector
inline vector float Divide( vector float a, vector float b ) {
   return vec_madd( a, Reciprocal( b ), (vector float)(0) );
}

/*
===============
  loadSplatUnalignedScalar

  For each element in vector:
	n = s
===============
*/
inline vector float loadSplatUnalignedScalar( const float *s ) {
	vector unsigned char splatMap = vec_lvsl( 0, s );
	vector float v = vec_ld( 0, s );
	splatMap = (vector unsigned char) vec_splat( (vector float) splatMap, 0 );
	return vec_perm( v, v, splatMap );
}

/*
===============
  VectorATan16

  For each element in vector:
	n = idMath::ATan16( x, y )
===============
*/
// calculates arc tangent of a vector with 16 bits of precision, based on atan16 in idMath
inline vector float VectorATan16( vector float x, vector float y ) {
	
	vector float xDivY = Divide( x, y );
	vector float yDivX = Divide( y, x );
	vector float zeroVector = (vector float)(0);
	
	vector bool int vecCmp = vec_cmpgt( vec_abs( y ), vec_abs( x ) );
	vector float vecA = vec_sel( yDivX, xDivY, vecCmp );
	vector bool int vecCmp2 = vec_cmplt( vecA, zeroVector );
	vector float vecS = vec_madd( vecA, vecA, (vector float)(0) );
		
	// do calculation for S
	vector float vecWork1 = vec_madd( (vector float)(0.0028662257f), vecS, (vector float)(-0.0161657367f) );
	vecWork1 = vec_madd( vecWork1, vecS, (vector float)(0.0429096138f) );
	vecWork1 = vec_madd( vecWork1, vecS, (vector float)(-0.0752896400f) );
	vecWork1 = vec_madd( vecWork1, vecS, (vector float)(0.1065626393f) );
	vecWork1 = vec_madd( vecWork1, vecS, (vector float)(-0.1420889944f) );
	vecWork1 = vec_madd( vecWork1, vecS, (vector float)(0.1999355085f) );
	vecWork1 = vec_madd( vecWork1, vecS, (vector float)(-0.3333314528f) );
	vecWork1 = vec_madd( vecWork1, vecS, (vector float)(1) );
	
	// get the regular S value
	vecS = vec_madd( vecWork1, vecA, (vector float)(0) );
	
	// calculate what to return if y > x
	vector float negSPlusHalfPI = vec_madd( vecS, (vector float)(-1), (vector float)(0.5f * 3.14159265358979323846f) );
	vector float negSMinusHalfPI = vec_madd( vecS, (vector float)(-1), (vector float)(-0.5f * 3.14159265358979323846f) );
	vector float modRet = vec_sel( negSPlusHalfPI, negSMinusHalfPI, vecCmp2 );
	
	return vec_sel( modRet, vecS, vecCmp );
}

/*
===============
  VectorSin16

  For each element in vector:
	n = idMath::Sin16( v )
===============
*/
inline vector float VectorSin16( vector float v ) {
	vector float zero = (vector float)(0);

#if 0	
	// load up half PI and use it to calculate the rest of the values. This is
	// sometimes cheaper than loading them from memory
	
	vector float halfPI = (vector float) ( 0.5f * 3.14159265358979323846f );
	vector float PI = vec_add( halfPI, halfPI ); 
	vector float oneandhalfPI = vec_add( PI, halfPI ); 
	vector float twoPI = vec_add( oneandhalfPI, halfPI );
#else
	vector float halfPI = (vector float) ( 0.5f * 3.14159265358979323846f );
	vector float PI = (vector float)(3.14159265358979323846f);
	vector float oneandhalfPI = (vector float)(3.14159265358979323846f + (  0.5f * 3.14159265358979323846f ) );
	vector float twoPI = (vector float)( 2.0f * 3.14159265358979323846f);
#endif	
	
	vector bool int vecCmp1, vecCmp2, vecCmp3, vecCmp4;
		
	vector float vecMod;
	vector float vecResult;
	
	// fix the range if needbe
	vecMod = vec_floor( Divide( v, twoPI ) );
	vecResult = vec_nmsub( vecMod, twoPI, v );

	vector float vecPIminusA = vec_sub( PI, vecResult );
	vector float vecAminus2PI = vec_sub( vecResult, twoPI );

	vecCmp1 = vec_cmplt( vecResult, PI );
	vecCmp2 = vec_cmpgt( vecResult, halfPI );

	// these are the ones where a > PI + HALF_PI so set a = a - TWO_PI
	vecCmp3 = vec_cmpgt( vecResult, oneandhalfPI );
	
	// we also want to set a = PI - a everywhere that !(a < PI) and !(a > PI + HALF_PI)
	vecCmp4 = vec_and( vec_xor( vecCmp3, (vector bool int)(1) ), vec_xor(  vecCmp1, (vector bool int)(1) ) ); // everywhere that both of those are false
	
	// these are ones where a < PI and a > HALF_PI so we set a = PI - a
	vecCmp1 = vec_and( vecCmp1, vecCmp2 );
	vecCmp1 = vec_or( vecCmp1, vecCmp4 );
	
	// put the correct values into place
	vecResult = vec_sel( vecResult, vecPIminusA, vecCmp1 );
	vecResult = vec_sel( vecResult, vecAminus2PI, vecCmp3 );

	// calculate answer
	vector float vecASquared = vec_madd( vecResult, vecResult, zero );
	vector float vecEst = vec_madd( (vector float)(-2.39e-08f), vecASquared, (vector float)(2.7526e-06f) );
	vecEst = vec_madd( vecEst, vecASquared, (vector float)(-1.98409e-04f) );
	vecEst = vec_madd( vecEst, vecASquared, (vector float)(8.3333315e-03f) );
	vecEst = vec_madd( vecEst, vecASquared, (vector float)(-1.666666664e-01f) );
	vecEst = vec_madd( vecEst, vecASquared, (vector float)(1.0f) );
	return vec_madd( vecResult, vecEst, zero );
}

/*
===============
  vecSplatWithRunTime

  For each element in vector:
	n = v(i)
===============
*/
// splats an element across a vector using a runtime variable
inline vector float vecSplatWithRunTime( vector float v, int i ) {
		vector unsigned char rotate = vec_lvsl( i * sizeof( float ), (int*) 0L );
		v = vec_perm( v, v, rotate );
		return vec_splat( v, 0 );
}


/*
===============
  FastScalarInvSqrt
  
	n = 1 / sqrt( f )
===============
*/
inline float FastScalarInvSqrt( float f ) {
#ifdef PPC_INTRINSICS
	float estimate;
	const float kSmallestFloat = FLT_MIN;

	//Calculate a 5 bit starting estimate for the reciprocal sqrt
	estimate = __frsqrte ( f + kSmallestFloat ); 

	//if you require less precision, you may reduce the number of loop iterations. 
	// This will do 2 rounds of NR
	estimate = estimate + 0.5f * estimate * ( 1.0f - f * estimate * estimate );
	estimate = estimate + 0.5f * estimate * ( 1.0f - f * estimate * estimate );
	return estimate;
#else
	return idMath::InvSqrt( f );
#endif
}

/*
===============
  FastScalarInvSqrt_x3
  
	arg1 = 1 / sqrt( arg1 )
	arg2 = 1 / sqrt( arg2 )
	arg3 = 1 / sqrt( arg3 )	
===============
*/
inline void FastScalarInvSqrt_x3( float *arg1, float *arg2, float *arg3 ) {
#ifdef PPC_INTRINSICS
	register float estimate1, estimate2, estimate3;
	const float kSmallestFloat = FLT_MIN;

	//Calculate a 5 bit starting estimate for the reciprocal sqrt of each
	estimate1 = __frsqrte ( *arg1 + kSmallestFloat ); 
	estimate2 = __frsqrte ( *arg2 + kSmallestFloat );
	estimate3 = __frsqrte ( *arg3 + kSmallestFloat );

	// two rounds newton-raphson
	estimate1 = estimate1 + 0.5f * estimate1 * ( 1.0f - *arg1 * estimate1 * estimate1 );
	estimate2 = estimate2 + 0.5f * estimate2 * ( 1.0f - *arg2 * estimate2 * estimate2 );
	estimate3 = estimate3 + 0.5f * estimate3 * ( 1.0f - *arg3 * estimate3 * estimate3 );
	estimate1 = estimate1 + 0.5f * estimate1 * ( 1.0f - *arg1 * estimate1 * estimate1 );
	estimate2 = estimate2 + 0.5f * estimate2 * ( 1.0f - *arg2 * estimate2 * estimate2 );
	estimate3 = estimate3 + 0.5f * estimate3 * ( 1.0f - *arg3 * estimate3 * estimate3 );

	*arg1 = estimate1;
	*arg2 = estimate2;
	*arg3 = estimate3;
#else
	*arg1 = idMath::InvSqrt( *arg1 );
	*arg2 = idMath::InvSqrt( *arg2 );
	*arg3 = idMath::InvSqrt( *arg3 );
#endif
}

/*
===============
  FastScalarInvSqrt_x6
  
	arg1 = 1 / sqrt( arg1 )
	arg2 = 1 / sqrt( arg2 )
	arg3 = 1 / sqrt( arg3 )	
	arg4 = 1 / sqrt( arg4 )	
	arg5 = 1 / sqrt( arg5 )	
	arg6 = 1 / sqrt( arg6 )	

	On a G5, you've got 2 pipeline stages to fill. (2 FPU's with 6 stages each)
===============
*/
inline void FastScalarInvSqrt_x6( float *arg1, float *arg2, float *arg3, float *arg4, float *arg5, float *arg6 ) {
#ifdef PPC_INTRINSICS
	register float estimate1, estimate2, estimate3, estimate4, estimate5, estimate6;
	const float kSmallestFloat = FLT_MIN;

	//Calculate a 5 bit starting estimate for the reciprocal sqrt of each
	estimate1 = __frsqrte ( *arg1 + kSmallestFloat ); 
	estimate2 = __frsqrte ( *arg2 + kSmallestFloat );
	estimate3 = __frsqrte ( *arg3 + kSmallestFloat );
	estimate4 = __frsqrte ( *arg4 + kSmallestFloat );
	estimate5 = __frsqrte ( *arg5 + kSmallestFloat );
	estimate6 = __frsqrte ( *arg6 + kSmallestFloat );

	// two rounds newton-raphson
	estimate1 = estimate1 + 0.5f * estimate1 * ( 1.0f - *arg1 * estimate1 * estimate1 );
	estimate2 = estimate2 + 0.5f * estimate2 * ( 1.0f - *arg2 * estimate2 * estimate2 );
	estimate3 = estimate3 + 0.5f * estimate3 * ( 1.0f - *arg3 * estimate3 * estimate3 );
	estimate4 = estimate4 + 0.5f * estimate4 * ( 1.0f - *arg4 * estimate4 * estimate4 );
	estimate5 = estimate5 + 0.5f * estimate5 * ( 1.0f - *arg5 * estimate5 * estimate5 );
	estimate6 = estimate6 + 0.5f * estimate6 * ( 1.0f - *arg6 * estimate6 * estimate6 );

	estimate1 = estimate1 + 0.5f * estimate1 * ( 1.0f - *arg1 * estimate1 * estimate1 );
	estimate2 = estimate2 + 0.5f * estimate2 * ( 1.0f - *arg2 * estimate2 * estimate2 );
	estimate3 = estimate3 + 0.5f * estimate3 * ( 1.0f - *arg3 * estimate3 * estimate3 );
	estimate4 = estimate4 + 0.5f * estimate4 * ( 1.0f - *arg4 * estimate4 * estimate4 );
	estimate5 = estimate5 + 0.5f * estimate5 * ( 1.0f - *arg5 * estimate5 * estimate5 );
	estimate6 = estimate6 + 0.5f * estimate6 * ( 1.0f - *arg6 * estimate6 * estimate6 );

	*arg1 = estimate1;
	*arg2 = estimate2;
	*arg3 = estimate3;
	*arg4 = estimate4;
	*arg5 = estimate5;
	*arg6 = estimate6;
#else
	*arg1 = idMath::InvSqrt( *arg1 );
	*arg2 = idMath::InvSqrt( *arg2 );
	*arg3 = idMath::InvSqrt( *arg3 );
	*arg4 = idMath::InvSqrt( *arg4 );
	*arg5 = idMath::InvSqrt( *arg5 );
	*arg6 = idMath::InvSqrt( *arg6 );	
#endif
}


// End Helper Functions

#ifdef ENABLE_SIMPLE_MATH

/*
============
idSIMD_AltiVec::Add

  dst[i] = constant + src[i];
============
*/
void VPCALL idSIMD_AltiVec::Add( float *dst, const float constant, const float *src, const int count ) {
    vector float v0, v1, v2, v3;
	vector float v0_low, v0_hi, v1_hi;
    vector unsigned char permVec;
    vector float constVec;
    int i;
	        
    // handle unaligned cases at beginning
    for ( i = 0; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
	   dst[i] = constant + src[i];
    }
	    
    //splat constant into a vector
    constVec = loadSplatUnalignedScalar( &constant ); 
	
	//calculate permute and do first load
	permVec = vec_add( vec_lvsl( -1, (int*) &src[i] ), (vector unsigned char)(1) );		
	v1_hi = vec_ld( 0, &src[i] ); 

    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v1_hi; 
		v0_hi = vec_ld( 15, &src[i] );
		v1_hi = vec_ld( 31, &src[i] );
			
		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v0_hi, v1_hi, permVec );
	
		v2 = vec_add( v0, constVec );
		v3 = vec_add( v1, constVec );
	
		// store results
		ALIGNED_STORE2( &dst[i], v2, v3 );
	}
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = constant + src[i];
    }
}

/*
============
idSIMD_AltiVec::Add

  dst[i] = src0[i] + src1[i];
============
*/
void VPCALL idSIMD_AltiVec::Add( float *dst, const float *src0, const float *src1, const int count ) {
    
    register vector float v0, v1, v2, v3, v4, v5;
    //src0
	register vector float v0_low, v0_hi, v2_low, v2_hi;
	//src1
	register vector float v1_low, v1_hi, v3_low, v3_hi;
	//permute vectors
	register vector unsigned char permVec1, permVec2;
	vector unsigned char oneCharVector = (vector unsigned char)(1);
	
	int i;
    
    //unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
	    dst[i] = src0[i] + src1[i];
    }
    
	//calculate permute and do loads
	permVec1 = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneCharVector );
	permVec2 = vec_add( vec_lvsl( -1, (int*) &src1[i] ), oneCharVector );
	v2_hi = vec_ld( 0, &src0[i] );
	v3_hi = vec_ld( 0, &src1[i] );
	
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v2_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v2_low = v0_hi;
		v2_hi = vec_ld( 31, &src0[i] );
		
		v1_low = v3_hi;
		v1_hi = vec_ld( 15, &src1[i] );
		v3_low = v1_hi;
		v3_hi = vec_ld( 31, &src1[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec1 );
		v1 = vec_perm( v1_low, v1_hi, permVec2 );
		v2 = vec_perm( v2_low, v2_hi, permVec1 );
		v3 = vec_perm( v3_low, v3_hi, permVec2 );
		
		v4 = vec_add( v0, v1 );
		v5 = vec_add( v2, v3 );
		
		ALIGNED_STORE2( &dst[i], v4, v5 );
	
	}
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = src0[i] + src1[i];
    }
}

/*
============
idSIMD_AltiVec::Sub

  dst[i] = constant - src[i];
============
*/
void VPCALL idSIMD_AltiVec::Sub( float *dst, const float constant, const float *src, const int count ) {

    register vector float v0, v1, v2, v3;
	register vector float v0_low, v0_hi, v1_low, v1_hi;
    register vector unsigned char permVec;
    register vector float constVec;
	vector unsigned char oneCharVector = (vector unsigned char)(1);
    int i;
    
    //handle unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = constant - src[i];
    }
    
    //splat constant into a vector
    constVec = loadSplatUnalignedScalar( &constant );
    
	//calculate permute vector and do first load
	permVec = vec_add( vec_lvsl( -1, (int*) &src[i] ), oneCharVector );
	v1_hi = vec_ld( 0, &src[i] );
	
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v1_hi;
		v0_hi = vec_ld( 15, &src[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &src[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );
		
		v2 = vec_sub( constVec, v0 );
		v3 = vec_sub( constVec, v1 );
	
		ALIGNED_STORE2( &dst[i], v2, v3 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = constant - src[i];
    }
}

/*
============
idSIMD_AltiVec::Sub

  dst[i] = src0[i] - src1[i];
============
*/
void VPCALL idSIMD_AltiVec::Sub( float *dst, const float *src0, const float *src1, const int count ) {
    register vector float v0, v1, v2, v3, v4, v5;
	//src0
	register vector float v0_low, v0_hi, v2_low, v2_hi;
	//src1
	register vector float v1_low, v1_hi, v3_low, v3_hi;
    register vector unsigned char permVec1, permVec2;
	vector unsigned char oneCharVector = (vector unsigned char)(1);
	int i;

    //handle unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = src0[i] - src1[i];
    }
    
	//calculate permute and do first loads
	permVec1 = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneCharVector );
	permVec2 = vec_add( vec_lvsl( -1, (int*) &src1[i] ), oneCharVector );
	v2_hi = vec_ld( 0, &src0[i] );
	v3_hi = vec_ld( 0, &src1[i] );
	
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v2_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v2_low = v0_hi;
		v2_hi = vec_ld( 31, &src0[i] );
		
		v1_low = v3_hi;
		v1_hi = vec_ld( 15, &src1[i] );
		v3_low = v1_hi;
		v3_hi = vec_ld( 31, &src1[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec1 );
		v1 = vec_perm( v1_low, v1_hi, permVec2 );
		v2 = vec_perm( v2_low, v2_hi, permVec1 );
		v3 = vec_perm( v3_low, v3_hi, permVec2 );
		
		v4 = vec_sub( v0, v1 );
		v5 = vec_sub( v2, v3 );

		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = src0[i] - src1[i];
    }
}

/*
============
idSIMD_AltiVec::Mul

  dst[i] = constant * src[i];
============
*/
void VPCALL idSIMD_AltiVec::Mul( float *dst, const float constant, const float *src, const int count) {
    register vector float v0, v0_low, v0_hi, v1_low, v1_hi, v1, v2, v3;
    register vector float constVec;
    register vector unsigned char permVec;
	vector unsigned char oneCharVector = (vector unsigned char)(1);
    register vector float zeroVector = (vector float)(0.0);
    int i;
	
    // handle unaligned data at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
	    dst[i] = constant * src[i];
    }
	
    //splat constant into a vector
    constVec = loadSplatUnalignedScalar( &constant );
	
	permVec = vec_add( vec_lvsl( -1, (int*) &src[i] ), oneCharVector );
	v1_hi = vec_ld( 0, &src[i] ); 
	
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v1_hi;
		v0_hi = vec_ld( 15, &src[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &src[i] );

		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );
	
		v2 = vec_madd( constVec, v0, zeroVector );
		v3 = vec_madd( constVec, v1, zeroVector );

		ALIGNED_STORE2( &dst[i], v2, v3 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = constant * src[i];
    }
}

/*
============
idSIMD_AltiVec::Mul

  dst[i] = src0[i] * src1[i];
============
*/
void VPCALL idSIMD_AltiVec::Mul( float *dst, const float *src0, const float *src1, const int count ) {
    register vector float v0, v1, v2, v3, v4, v5;
	//src0
	register vector float v0_low, v0_hi, v2_low, v2_hi;
	//src1
	register vector float v1_low, v1_hi, v3_low, v3_hi;
	//permute vectors
	register vector unsigned char permVec1, permVec2;
    register vector float constVec = (vector float)(0.0);
	vector unsigned char oneCharVector = (vector unsigned char)(1);
    int i;
	
    //handle unaligned at start
    for ( i = 0; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] = src0[i] * src1[i];
    }
	
	//calculate permute and do loads
	permVec1 = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneCharVector );
	permVec2 = vec_add( vec_lvsl( -1, (int*) &src1[i] ), oneCharVector );
	v2_hi = vec_ld( 0, &src0[i] );
	v3_hi = vec_ld( 0, &src1[i] );
              
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v2_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v2_low = v0_hi;
		v2_hi = vec_ld( 31, &src0[i] );
		
		v1_low = v3_hi;
		v1_hi = vec_ld( 15, &src1[i] );
		v3_low = v1_hi;
		v3_hi = vec_ld( 31, &src1[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec1 );
		v1 = vec_perm( v1_low, v1_hi, permVec2 );
		v2 = vec_perm( v2_low, v2_hi, permVec1 );
		v3 = vec_perm( v3_low, v3_hi, permVec2 );
		
		//no such thing as regular multiply so we do
		//multiply then add zero
		v4 = vec_madd( v0, v1, constVec );
		v5 = vec_madd( v2, v3, constVec );

		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = src0[i] * src1[i];
    }
}

/*
============
idSIMD_AltiVec::Div

  dst[i] = constant / divisor[i];
============
*/
void VPCALL idSIMD_AltiVec::Div( float *dst, const float constant, const float *divisor, const int count ) {
    register vector float v0, v1, v2, v3;
    register vector float v0_low, v0_hi, v1_low, v1_hi;
	register vector unsigned char permVec;
    register vector float constVec;
    vector unsigned char oneCharVector = (vector unsigned char)(1);
	int i;
        
    //handle unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] = constant / divisor[i];
    }
    
    //splat constant into a vector
    constVec = loadSplatUnalignedScalar( &constant );
	
	//calculate permute and do first loads
	permVec = vec_add( vec_lvsl( -1, (int*) &divisor[i] ), oneCharVector );
 	v1_hi = vec_ld( 0, &divisor[i] ); 

    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v1_hi;
		v0_hi = vec_ld( 15, &divisor[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &divisor[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );
		
		v2 = Divide( constVec, v0 );
		v3 = Divide( constVec, v1 );
		
		ALIGNED_STORE2( &dst[i], v2, v3 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = constant / divisor[i];
    }
}

/*
============
idSIMD_AltiVec::Div

  dst[i] = src0[i] / src1[i];
============
*/
void VPCALL idSIMD_AltiVec::Div( float *dst, const float *src0, const float *src1, const int count ) {
    register vector float v0, v1, v2, v3, v4, v5;
	 //src0
	register vector float v0_low, v0_hi, v2_low, v2_hi;
	//src1
	register vector float v1_low, v1_hi, v3_low, v3_hi;
	//permute vectors
	register vector unsigned char permVec1, permVec2;
    vector unsigned char oneCharVector = (vector unsigned char)(1);
	int i;

    //handle unaligned at start
    for ( i = 0; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] = src0[i] / src1[i];
    }
	
	//calculate permute and do loads
	permVec1 = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneCharVector );
	permVec2 = vec_add( vec_lvsl( -1, (int*) &src1[i] ), oneCharVector );
	v2_hi = vec_ld( 0, &src0[i] );
	v3_hi = vec_ld( 0, &src1[i] );
    
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source		
		v0_low = v2_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v2_low = v0_hi;
		v2_hi = vec_ld( 31, &src0[i] );
		
		v1_low = v3_hi;
		v1_hi = vec_ld( 15, &src1[i] );
		v3_low = v1_hi;
		v3_hi = vec_ld( 31, &src1[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec1 );
		v1 = vec_perm( v1_low, v1_hi, permVec2 );
		v2 = vec_perm( v2_low, v2_hi, permVec1 );
		v3 = vec_perm( v3_low, v3_hi, permVec2 );
		
		v4 = Divide( v0, v1 );
		v5 = Divide( v2, v3 );
		
		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = src0[i] / src1[i];
    }
}

/*
============
idSIMD_AltiVec::MulAdd

  dst[i] += constant * src[i];
============
*/
void VPCALL idSIMD_AltiVec::MulAdd( float *dst, const float constant, const float *src, const int count ) {
 
    register vector float v0, v1, v2, v3, v4, v5;
    register vector float constVec;
	 //src
	register vector float v0_low, v0_hi, v2_low, v2_hi;
	//permute vectors
	register vector unsigned char permVec1;
	vector unsigned char oneCharVector = (vector unsigned char)(1);
    int i;

    //handle unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] += constant * src[i];
    }
    
    //splat constant into a vector
	constVec = loadSplatUnalignedScalar( &constant );
	
	//calculate permute and do loads
	permVec1 = vec_add( vec_lvsl( -1, (int*) &src[i] ), oneCharVector );
	v2_hi = vec_ld( 0, &src[i] );
	    
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		v0_low = v2_hi;
		v0_hi = vec_ld( 15, &src[i] );
		v2_low = v0_hi;
		v2_hi = vec_ld( 31, &src[i] );
				
		v0 = vec_perm( v0_low, v0_hi, permVec1 );
		v2 = vec_perm( v2_low, v2_hi, permVec1 );

		// at this point, dst is known to be aligned
		v1 = vec_ld( 0, &dst[i] );
		v3 = vec_ld( 16, &dst[i] );
	
		v4 = vec_madd( constVec, v0, v1 );
		v5 = vec_madd( constVec, v2, v3 );
		
		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] += constant * src[i];
    }
}

/*
============
idSIMD_AltiVec::MulAdd

  dst[i] += src0[i] * src1[i];  
============
*/
void VPCALL idSIMD_AltiVec::MulAdd( float *dst, const float *src0, const float *src1, const int count ) {
    register vector float v0, v1, v2, v3, v4, v5, v6, v7;
    //src0
	register vector float v0_low, v0_hi, v2_low, v2_hi;
	//src1
	register vector float v1_low, v1_hi, v3_low, v3_hi;
	//permute vectors
	register vector unsigned char permVec1, permVec2;
	vector unsigned char oneCharVector = (vector unsigned char)(1);

    int i;
	
    //unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] += src0[i] * src1[i];
    }
	
	//calculate permute and do loads
	permVec1 = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneCharVector );
	permVec2 = vec_add( vec_lvsl( -1, (int*) &src1[i] ), oneCharVector );
	v2_hi = vec_ld( 0, &src0[i] );
	v3_hi = vec_ld( 0, &src1[i] );
    
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		// load sources
		v0_low = v2_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v2_low = v0_hi;
		v2_hi = vec_ld( 31, &src0[i] );
		
		v1_low = v3_hi;
		v1_hi = vec_ld( 15, &src1[i] );
		v3_low = v1_hi;
		v3_hi = vec_ld( 31, &src1[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec1 );
		v1 = vec_perm( v1_low, v1_hi, permVec2 );
		v2 = vec_perm( v2_low, v2_hi, permVec1 );
		v3 = vec_perm( v3_low, v3_hi, permVec2 );
		
		//we know dst is aligned because we handled unaligned cases
		//up front
		v4 = vec_ld( 0, &dst[i] );
		v5 = vec_ld( 16, &dst[i] );
	
		v6 = vec_madd( v0, v1, v4 );
		v7 = vec_madd( v2, v3, v5 );
	
		ALIGNED_STORE2( &dst[i], v6, v7 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] += src0[i] * src1[i];
    }
}

/*
============
idSIMD_AltiVec::MulSub

  dst[i] -= constant * src[i];
============
*/
void VPCALL idSIMD_AltiVec::MulSub( float *dst, const float constant, const float *src, const int count ) {
    register vector float v0, v1, v2, v3, v4, v5;
    register vector float constVec;
	 //src
	register vector float v0_low, v0_hi, v2_low, v2_hi;
	//permute vectors
	register vector unsigned char permVec1;
	vector unsigned char oneCharVector = (vector unsigned char)(1);
    int i;

    //handle unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] -= constant * src[i];
    }
    
    //splat constant into a vector
	constVec = loadSplatUnalignedScalar( &constant );
	
	//calculate permute and do loads
	permVec1 = vec_add( vec_lvsl( -1, (int*) &src[i] ), oneCharVector );
	v2_hi = vec_ld( 0, &src[i] );
	    
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		v0_low = v2_hi;
		v0_hi = vec_ld( 15, &src[i] );
		v2_low = v0_hi;
		v2_hi = vec_ld( 31, &src[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec1 );
		v2 = vec_perm( v2_low, v2_hi, permVec1 );
		
		//we know dst will be aligned here because we already handled the preceeding
		//unaligned cases
		v1 = vec_ld( 0, &dst[i] );
		v3 = vec_ld( 16, &dst[i] );
	
		v4 = vec_nmsub( v0, constVec, v1 );
		v5 = vec_nmsub( v2, constVec, v3 );

		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] -= constant * src[i];
    }
}

/*
============
idSIMD_AltiVec::MulSub

  dst[i] -= src0[i] * src1[i];
============
*/
void VPCALL idSIMD_AltiVec::MulSub( float *dst, const float *src0, const float *src1, const int count ) {
    register vector float v0, v1, v2, v3, v4, v5, v6, v7;
    //src0
	register vector float v0_low, v0_hi, v2_low, v2_hi;
	//src1
	register vector float v1_low, v1_hi, v3_low, v3_hi;
	//permute vectors
	register vector unsigned char permVec1, permVec2;
	vector unsigned char oneCharVector = (vector unsigned char)(1);
    int i;
	
    //unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] -= src0[i] * src1[i];
    }
	
	//calculate permute and do loads
	permVec1 = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneCharVector );
	permVec2 = vec_add( vec_lvsl( -1, (int*) &src1[i] ), oneCharVector );
	v2_hi = vec_ld( 0, &src0[i] );
	v3_hi = vec_ld( 0, &src1[i] );

    
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		// load sources
		v0_low = v2_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v2_low = v0_hi;
		v2_hi = vec_ld( 31, &src0[i] );
		
		v1_low = v3_hi;
		v1_hi = vec_ld( 15, &src1[i] );
		v3_low = v1_hi;
		v3_hi = vec_ld( 31, &src1[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec1 );
		v1 = vec_perm( v1_low, v1_hi, permVec2 );
		v2 = vec_perm( v2_low, v2_hi, permVec1 );
		v3 = vec_perm( v3_low, v3_hi, permVec2 );
		
		//we know dst is aligned because we handled unaligned cases
		//up front
		v4 = vec_ld( 0, &dst[i] );
		v5 = vec_ld( 16, &dst[i] );
	
		v6 = vec_nmsub( v0, v1, v4 );
		v7 = vec_nmsub( v2, v3, v5 );

		ALIGNED_STORE2( &dst[i], v6, v7 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] -= src0[i] * src1[i];
    }
}

#endif /* ENABLE_SIMPLE_MATH */

#ifdef ENABLE_DOT
/*
============
idSIMD_AltiVec::Dot

  dst[i] = constant * src[i];
============
*/
void VPCALL idSIMD_AltiVec::Dot( float *dst, const idVec3 &constant, const idVec3 *src, const int count ) {
		
		register vector float vecLd1, vecLd2, vecLd3, vecLd4, vecLd5, vecLd6;
		register vector float vecX, vecY, vecZ;
		vector float vecX2, vecY2, vecZ2;
		const float *addr = src[0].ToFloatPtr();
		float tempVal[4];
		float constVal[4];
		register vector float zeroVector = (vector float)(0.0);
		register vector float vecConstX, vecConstY, vecConstZ;
		
		// permute vectors
		register vector unsigned char permX1 = (vector unsigned char)(0,1,2,3,12,13,14,15,24,25,26,27,28,29,30,31); //last 4 bytes are junk
		register vector unsigned char permX2 = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,20,21,22,23);
		
		register vector unsigned char permY1 = (vector unsigned char)(4,5,6,7,16,17,18,19,28,29,30,31,0,1,2,3); //last 4 bytes are junk
		register vector unsigned char permY2 = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,24,25,26,27);
		
		register vector unsigned char permZ1 = (vector unsigned char)(8,9,10,11,20,21,22,23,0,1,2,3,4,5,6,7); //last 8 bytes are junk
		register vector unsigned char permZ2 = (vector unsigned char)(0,1,2,3,4,5,6,7,16,17,18,19,28,29,30,31);
			
		int i;
					
		// for scalar cleanup, if necessary
		constVal[0] = constant[0];
		constVal[1] = constant[1];
		constVal[2] = constant[2];
		constVal[3] = 0;
	
		vector unsigned char constPerm = vec_lvsl( 0, constant.ToFloatPtr() );
		vecLd1 = vec_ld( 0, constant.ToFloatPtr() );
		vecLd2 = vec_ld( 11, constant.ToFloatPtr() );
		vecLd1 = vec_perm( vecLd1, vecLd2, constPerm );
		
			
		// populate const vectors
		vecConstX = vec_splat( vecLd1, 0 ); 
		vecConstY = vec_splat( vecLd1, 1 ); 
		vecConstZ = vec_splat( vecLd1, 2 ); 

		vector unsigned char permVec = vec_add( vec_lvsl( -1, addr ), (vector unsigned char)(1) );
		vector float vecOld = vec_ld( 0, addr );

		// handle unaligned case at beginning
	    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
			dst[i] = constant * src[i];
		}

		for ( ; i + 7 < count; i += 8 ) {
			float *vecPtr = (float*)( addr + (i*3) );
			vector float v0, v1, v2, v3, v4, v5;
			
			v0 = vecOld; //vec_ld( 0, vecPtr );
			v1 = vec_ld( 15, vecPtr );
			v2 = vec_ld( 31, vecPtr );
			v3 = vec_ld( 47, vecPtr );
			v4 = vec_ld( 63, vecPtr );
			v5 = vec_ld( 79, vecPtr );
			vecOld = vec_ld( 95, vecPtr );

			vecLd1 = vec_perm( v0, v1, permVec );
			vecLd2 = vec_perm( v1, v2, permVec );
			vecLd3 = vec_perm( v2, v3, permVec );
			
			vecLd4 = vec_perm( v3, v4, permVec );
			vecLd5 = vec_perm( v4, v5, permVec );
			vecLd6 = vec_perm( v5, vecOld, permVec );
																																	
			// permute into X Y Z vectors
			vecX = vec_perm( vecLd1, vecLd2, permX1 );
			vecY = vec_perm( vecLd1, vecLd2, permY1 );
			vecZ = vec_perm( vecLd1, vecLd2, permZ1 );
			vecX = vec_perm( vecX, vecLd3, permX2 );
			vecY = vec_perm( vecY, vecLd3, permY2 );
			vecZ = vec_perm( vecZ, vecLd3, permZ2 );
			
			vecX2 = vec_perm( vecLd4, vecLd5, permX1 );
			vecY2 = vec_perm( vecLd4, vecLd5, permY1 );
			vecZ2 = vec_perm( vecLd4, vecLd5, permZ1 );
			vecX2 = vec_perm( vecX2, vecLd6, permX2 );
			vecY2 = vec_perm( vecY2, vecLd6, permY2 );
			vecZ2 = vec_perm( vecZ2, vecLd6, permZ2 );
			
			// do multiply
			vecX = vec_madd( vecX, vecConstX, zeroVector );
			vecY = vec_madd( vecY, vecConstY, vecX );
			vecZ = vec_madd( vecZ, vecConstZ, vecY );
			
			vecX2 = vec_madd( vecX2, vecConstX, zeroVector );
			vecY2 = vec_madd( vecY2, vecConstY, vecX2 );
			vecZ2 = vec_madd( vecZ2, vecConstZ, vecY2 );
												
			// store out results
			ALIGNED_STORE2( &dst[i], vecZ, vecZ2 );
		}
		
		//cleanup
		for ( ; i < count; i++ ) {
			// look up whats at the address we want, cast it as float pointer, then
			// dereference that pointer
			tempVal[0] =  *( addr + (i*3) + 0 );  
			tempVal[1] = *( addr + (i*3) + 1 );  
			tempVal[2] = *( addr + (i*3) + 2 ); 
			dst[i] = constVal[0] * tempVal[0] + constVal[1] * tempVal[1] + constVal[2] * tempVal[2];
	}
}


/*
============
idSIMD_AltiVec::Dot

  dst[i] = constant * src[i].Normal() + src[i][3];
============
*/
void VPCALL idSIMD_AltiVec::Dot( float *dst, const idVec3 &constant, const idPlane *src, const int count ) {
//#define OPER(X) dst[(X)] = constant * src[(X)].Normal() + src[(X)][3];

	assert( sizeof(idPlane)  == PLANE_OFFSET * sizeof(float) );
	
	int i;
	float constVal[4];
	float srcVal[3];
	float srcI3;
	float tempVal;
	
	vector float vecPlaneLd1, vecPlaneLd2, vecPlaneLd3, vecPlaneLd4;
	vector float vecPlaneLd5, vecPlaneLd6, vecPlaneLd7, vecPlaneLd8;
	vector float vecX, vecY, vecZ, vecI3;
	vector float vecX2, vecY2, vecZ2, vecI32;
	vector float vecConstX, vecConstY, vecConstZ;
	
	constVal[0] = constant[0];
	constVal[1] = constant[1];
	constVal[2] = constant[2];
	constVal[3] = 1;
	
	vector unsigned char constPerm = vec_lvsl( 0, constant.ToFloatPtr() );
	vector float v0 = vec_ld( 0, constant.ToFloatPtr() );
	vector float v1 = vec_ld( 11, constant.ToFloatPtr() );
	vector float vecConst = vec_perm( v0, v1, constPerm );
	
	vecConstX = vec_splat( vecConst, 0 );
	vecConstY = vec_splat( vecConst, 1 );
	vecConstZ = vec_splat( vecConst, 2 );
	
	// handle unaligned case at beginning
	for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = constant * src[i].Normal() + src[i][3];
	}

	const float *addr = src[i].ToFloatPtr();
	vector unsigned char permVec = vec_add( vec_lvsl( -1, addr ), (vector unsigned char)(1) );
	vector float vecOld = vec_ld( 0, addr );
	
	for ( ; i + 7 < count; i += 8 ) {
			float *planePtr = (float*)( addr + (i*PLANE_OFFSET) );
			vector float v0, v1, v2, v3, v4, v5, v6, v7;
			
			v0 = vecOld; //vec_ld( 0, planePtr );
			v1 = vec_ld( 15, planePtr );
			v2 = vec_ld( 31, planePtr );
			v3 = vec_ld( 47, planePtr );
			v4 = vec_ld( 63, planePtr );
			v5 = vec_ld( 79, planePtr );
			v6 = vec_ld( 95, planePtr );
			v7 = vec_ld( 111, planePtr );
			vecOld = vec_ld( 127, planePtr );
			
			vecPlaneLd1 = vec_perm( v0, v1, permVec );
			vecPlaneLd2 = vec_perm( v1, v2, permVec );
			vecPlaneLd3 = vec_perm( v2, v3, permVec );
			vecPlaneLd4 = vec_perm( v3, v4, permVec );
			
			vecPlaneLd5 = vec_perm( v4, v5, permVec );
			vecPlaneLd6 = vec_perm( v5, v6, permVec );
			vecPlaneLd7 = vec_perm( v6, v7, permVec );
			vecPlaneLd8 = vec_perm( v7, vecOld, permVec );
			
			// permute into X Y Z vectors, since this is square its basically
			// a matrix transpose
			v0 = vec_mergeh( vecPlaneLd1, vecPlaneLd3 );
			v1 = vec_mergeh( vecPlaneLd2, vecPlaneLd4 );
			v2 = vec_mergel( vecPlaneLd1, vecPlaneLd3 );
			v3 = vec_mergel( vecPlaneLd2, vecPlaneLd4 );
			
			vecX = vec_mergeh( v0, v1 );
			vecY = vec_mergel( v0, v1 );
			vecZ = vec_mergeh( v2, v3 );
			vecI3 = vec_mergel( v2, v3 );
			
			v4 = vec_mergeh( vecPlaneLd5, vecPlaneLd7 );
			v5 = vec_mergeh( vecPlaneLd6, vecPlaneLd8 );
			v6 = vec_mergel( vecPlaneLd5, vecPlaneLd7 );
			v7 = vec_mergel( vecPlaneLd6, vecPlaneLd8 );
			
			vecX2 = vec_mergeh( v4, v5 );
			vecY2 = vec_mergel( v4, v5 );
			vecZ2 = vec_mergeh( v6, v7 );
			vecI32 = vec_mergel( v6, v7 );

			// do calculation
			v6 = vec_madd( vecZ, vecConstZ, vecI3 );
			v5 = vec_madd( vecY, vecConstY, v6 );
			v4 = vec_madd( vecX, vecConstX, v5 );
			
			v0 = vec_madd( vecZ2, vecConstZ, vecI32 );
			v1 = vec_madd( vecY2, vecConstY, v0 );
			v2 = vec_madd( vecX2, vecConstX, v1 );
			
			// store results
			ALIGNED_STORE2( &dst[i], v4, v2 );
	}

	// cleanup
	for ( ; i < count; i++ ) {
		// populate srcVal with src X Y Z
		srcVal[0] = *(addr + (i*PLANE_OFFSET) + 0 );
		srcVal[1] = *(addr + (i*PLANE_OFFSET) + 1 );
		srcVal[2] = *(addr + (i*PLANE_OFFSET) + 2 );
		
		// put src[i][3] into srcI3
		srcI3 = *(addr + (i*PLANE_OFFSET) + 3 );

		tempVal = constVal[0] * srcVal[0] + constVal[1] * srcVal[1] + constVal[2] * srcVal[2];
		dst[i] = tempVal + srcI3;
	}
}

#ifndef DRAWVERT_PADDED
/*
============
idSIMD_AltiVec::Dot

  dst[i] = constant * src[i].xyz;
============
*/
void VPCALL idSIMD_AltiVec::Dot( float *dst, const idVec3 &constant, const idDrawVert *src, const int count ) {
//#define OPER(X) dst[(X)] = constant * src[(X)].xyz;
		
		// idDrawVert size is 60 bytes
		assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof( float ) );
	
		register vector float v0, v1, v2, v3, v4, v5, v6, v7;
		int i;
		register vector float vecConstX, vecConstY, vecConstZ;
		register vector float vecSrcX1, vecSrcY1, vecSrcZ1;
		register vector float zeroVector = (vector float)(0.0);
		vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
		
		vector unsigned char constPerm = vec_lvsl( 0, constant.ToFloatPtr() );
		v0 = vec_ld( 0, constant.ToFloatPtr() );
		v1 = vec_ld( 11, constant.ToFloatPtr() );
		v0 = vec_perm( v0, v1, constPerm );
		
		// permute into constant vectors
		vecConstX = vec_splat( v0, 0 );
		vecConstY = vec_splat( v0, 1 );
		vecConstZ = vec_splat( v0, 2 );
		
		// handle unaligned case at beginning
	    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
			dst[i] = constant * src[i].xyz;
		}		
		
		// every fourth one will have the same alignment. Make sure we've got enough here
		if ( i+3 < count ) {
			vertPerm1 = vec_add( vec_lvsl( -1, (float*) src[i].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
			vertPerm2 = vec_add( vec_lvsl( -1, (float*) src[i+1].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
			vertPerm3 = vec_add( vec_lvsl( -1, (float*) src[i+2].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
			vertPerm4 = vec_add( vec_lvsl( -1, (float*) src[i+3].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		}
		
		for ( ; i+3 < count; i += 4 ) {
			const float *vertPtr = src[i].xyz.ToFloatPtr();
			const float *vertPtr2 = src[i+1].xyz.ToFloatPtr();
			const float *vertPtr3 = src[i+2].xyz.ToFloatPtr();
			const float *vertPtr4 = src[i+3].xyz.ToFloatPtr();
			
			v0 = vec_ld( 0, vertPtr );
			v1 = vec_ld( 11, vertPtr );
			v2 = vec_ld( 0, vertPtr2 );
			v3 = vec_ld( 11, vertPtr2 );
			v4 = vec_ld( 0, vertPtr3 );
			v5 = vec_ld( 11, vertPtr3 );
			v6 = vec_ld( 0, vertPtr4 );
			v7 = vec_ld( 11, vertPtr4 );
			
			v0 = vec_perm( v0, v1, vertPerm1 );
			v2 = vec_perm( v2, v3, vertPerm2 );
			v4 = vec_perm( v4, v5, vertPerm3 );
			v6 = vec_perm( v6, v7, vertPerm4 );

			// transpose into X Y Z vectors			
			v1 = vec_mergeh( v0, v4 );
			v3 = vec_mergeh( v2, v6 );
			v5 = vec_mergel( v0, v4 );
			v7 = vec_mergel( v2, v6 );
			
			vecSrcX1 = vec_mergeh( v1, v3 );
			vecSrcY1 = vec_mergel( v1, v3 );
			vecSrcZ1 = vec_mergeh( v5, v7 );
			
			// now calculate dot product
			vecSrcX1 = vec_madd( vecSrcX1, vecConstX, zeroVector );
			vecSrcY1 = vec_madd( vecSrcY1, vecConstY, vecSrcX1 );
			vecSrcZ1 = vec_madd( vecSrcZ1, vecConstZ, vecSrcY1 );
			
			// store results
			vec_st( vecSrcZ1, 0, &dst[i] );
		}
		
		for ( ; i < count; i++ ) {
			dst[i] = constant * src[i].xyz;
		}
}
#else
/*
============
idSIMD_AltiVec::Dot

  dst[i] = constant * src[i].xyz;
============
*/
void VPCALL idSIMD_AltiVec::Dot( float *dst, const idVec3 &constant, const idDrawVert *src, const int count ) {
//#define OPER(X) dst[(X)] = constant * src[(X)].xyz;
		
		// idDrawVert size is 64 bytes
		assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof( float ) );
	
		register vector float v0, v1, v2, v3, v4, v5, v6, v7;
		int i;
		register vector float vecConstX, vecConstY, vecConstZ;
		register vector float vecSrcX1, vecSrcY1, vecSrcZ1;
		register vector float zeroVector = (vector float)(0.0);
		vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
		
		vector unsigned char constPerm = vec_lvsl( 0, constant.ToFloatPtr() );
		v0 = vec_ld( 0, constant.ToFloatPtr() );
		v1 = vec_ld( 11, constant.ToFloatPtr() );
		v0 = vec_perm( v0, v1, constPerm );
		
		// permute into constant vectors
		vecConstX = vec_splat( v0, 0 );
		vecConstY = vec_splat( v0, 1 );
		vecConstZ = vec_splat( v0, 2 );
		
		// handle unaligned case at beginning
	    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
			dst[i] = constant * src[i].xyz;
		}		
					
		for ( ; i+3 < count; i += 4 ) {
			const float *vertPtr = src[i].xyz.ToFloatPtr();
			const float *vertPtr2 = src[i+1].xyz.ToFloatPtr();
			const float *vertPtr3 = src[i+2].xyz.ToFloatPtr();
			const float *vertPtr4 = src[i+3].xyz.ToFloatPtr();
			
			v0 = vec_ld( 0, vertPtr );
			v2 = vec_ld( 0, vertPtr2 );
			v4 = vec_ld( 0, vertPtr3 );
			v6 = vec_ld( 0, vertPtr4 );
			
			// transpose into X Y Z vectors			
			v1 = vec_mergeh( v0, v4 );
			v3 = vec_mergeh( v2, v6 );
			v5 = vec_mergel( v0, v4 );
			v7 = vec_mergel( v2, v6 );
			
			vecSrcX1 = vec_mergeh( v1, v3 );
			vecSrcY1 = vec_mergel( v1, v3 );
			vecSrcZ1 = vec_mergeh( v5, v7 );
			
			// now calculate dot product
			vecSrcX1 = vec_madd( vecSrcX1, vecConstX, zeroVector );
			vecSrcY1 = vec_madd( vecSrcY1, vecConstY, vecSrcX1 );
			vecSrcZ1 = vec_madd( vecSrcZ1, vecConstZ, vecSrcY1 );
			
			// store results
			vec_st( vecSrcZ1, 0, &dst[i] );
		}
		
		for ( ; i < count; i++ ) {
			dst[i] = constant * src[i].xyz;
		}
}

#endif /* DRAWVERT_PADDED */

/*
============
idSIMD_AltiVec::Dot

  dst[i] = constant.Normal() * src[i] + constant[3];  
============
*/
void VPCALL idSIMD_AltiVec::Dot( float *dst, const idPlane &constant, const idVec3 *src, const int count ) {
//#define OPER(X) dst[(X)] = constant.Normal() * src[(X)] + constant[3];
		
		register vector float vecLd1, vecLd2, vecLd3, vecLd4, vecLd5, vecLd6;
		register vector float vecX, vecY, vecZ, vecX2, vecY2, vecZ2;
		register vector float zeroVector = (vector float)(0.0);
		register vector float vecConstX, vecConstY, vecConstZ;
		register vector float vecConst3;
		
		idVec3 constNormal = constant.Normal();
		float const3 = constant[3];
		
		// permute vectors
		register vector unsigned char permX1 = (vector unsigned char)(0,1,2,3,12,13,14,15,24,25,26,27,28,29,30,31); //last 4 bytes are junk
		register vector unsigned char permX2 = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,20,21,22,23);
		
		register vector unsigned char permY1 = (vector unsigned char)(4,5,6,7,16,17,18,19,28,29,30,31,0,1,2,3); //last 4 bytes are junk
		register vector unsigned char permY2 = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,24,25,26,27);
		
		register vector unsigned char permZ1 = (vector unsigned char)(8,9,10,11,20,21,22,23,0,1,2,3,4,5,6,7); //last 8 bytes are junk
		register vector unsigned char permZ2 = (vector unsigned char)(0,1,2,3,4,5,6,7,16,17,18,19,28,29,30,31);
				
		int i;
	
		vector unsigned char constPerm = vec_lvsl( 0, constant.ToFloatPtr() ); 
		vecLd1 = vec_ld( 0, constant.ToFloatPtr() );
		vecLd2 = vec_ld( 15, constant.ToFloatPtr() );
		vecLd1 = vec_perm( vecLd1, vecLd2, constPerm );
			
		// populate const vec
		vecConstX = vec_splat( vecLd1, 0 );
		vecConstY = vec_splat( vecLd1, 1 );
		vecConstZ = vec_splat( vecLd1, 2 );

		// put constant to add in vector
		vecConst3 = loadSplatUnalignedScalar( &const3 );

		// handle unaligned case at beginning
	    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
			dst[i] = constant.Normal() * src[i] + constant[3];
		}

		const float *addr = src[i].ToFloatPtr();
		vector unsigned char permVec = vec_add( vec_lvsl( -1, addr ), (vector unsigned char)(1) );
		vector float vecOld = vec_ld( 0, addr );
		
		for ( ; i+7 < count; i += 8 ) {
			float *vecPtr = (float*)( addr + (i*3) );
			vector float v0, v1, v2, v3, v4, v5;
			
			v0 = vecOld; //vec_ld( 0, vecPtr );
			v1 = vec_ld( 15, vecPtr );
			v2 = vec_ld( 31, vecPtr );
			v3 = vec_ld( 47, vecPtr );	
			v4 = vec_ld( 63, vecPtr );
			v5 = vec_ld( 79, vecPtr );
			vecOld = vec_ld( 95, vecPtr );
							
			vecLd1 = vec_perm( v0, v1, permVec );
			vecLd2 = vec_perm( v1, v2, permVec );
			vecLd3 = vec_perm( v2, v3, permVec );
			
			vecLd4 = vec_perm( v3, v4, permVec );
			vecLd5 = vec_perm( v4, v5, permVec );
			vecLd6 = vec_perm( v5, vecOld, permVec );
			
			// permute into X Y Z vectors
			vecX = vec_perm( vecLd1, vecLd2, permX1 );
			vecY = vec_perm( vecLd1, vecLd2, permY1 );
			vecZ = vec_perm( vecLd1, vecLd2, permZ1 );
			vecX = vec_perm( vecX, vecLd3, permX2 );
			vecY = vec_perm( vecY, vecLd3, permY2 );
			vecZ = vec_perm( vecZ, vecLd3, permZ2 );

			vecX2 = vec_perm( vecLd4, vecLd5, permX1 );
			vecY2 = vec_perm( vecLd4, vecLd5, permY1 );
			vecZ2 = vec_perm( vecLd4, vecLd5, permZ1 );
			vecX2 = vec_perm( vecX2, vecLd6, permX2 );
			vecY2 = vec_perm( vecY2, vecLd6, permY2 );
			vecZ2 = vec_perm( vecZ2, vecLd6, permZ2 );
						
			// calculate dot product
			vecX = vec_madd( vecX, vecConstX, zeroVector );
			vecY = vec_madd( vecY, vecConstY, vecX );
			vecZ = vec_madd( vecZ, vecConstZ, vecY );
			
			vecX2 = vec_madd( vecX2, vecConstX, zeroVector );
			vecY2 = vec_madd( vecY2, vecConstY, vecX2 );
			vecZ2 = vec_madd( vecZ2, vecConstZ, vecY2 );
			
			// add in constant[3]
			vecZ = vec_add( vecZ, vecConst3 );
			vecZ2 = vec_add( vecZ2, vecConst3 );
			
			// store out results
			ALIGNED_STORE2( &dst[i], vecZ, vecZ2 );
		}
		
		//cleanup
		for ( ; i < count; i++ ) {	
		    dst[i] = constNormal * src[i] + const3;
		}
}

/*
============
idSIMD_AltiVec::Dot

  dst[i] = constant.Normal() * src[i].Normal() + constant[3] * src[i][3];
============
*/
void VPCALL idSIMD_AltiVec::Dot( float *dst, const idPlane &constant, const idPlane *src, const int count ) {
//#define OPER(X) dst[(X)] = constant.Normal() * src[(X)].Normal() + constant[3] * src[(X)][3];

	// check plane size
	assert( sizeof(idPlane) == PLANE_OFFSET * sizeof(float) );
	
	float constVal[4];
	float srcVal[4];
	
	int i;
	const float *constPtr = constant.ToFloatPtr();
	
	register vector float vecX, vecY, vecZ, vecI3;
	register vector float vecX2, vecY2, vecZ2, vecI32;

	vector float vecPlaneLd1, vecPlaneLd2, vecPlaneLd3, vecPlaneLd4;
	vector float vecPlaneLd5, vecPlaneLd6, vecPlaneLd7, vecPlaneLd8;
	register vector float zeroVector = (vector float)(0.0);
	register vector float vecConstX, vecConstY, vecConstZ, vecConstI3;
	
	constVal[0] = *(constPtr);
	constVal[1] = *(constPtr+1);
	constVal[2] = *(constPtr+2);
	constVal[3] = *(constPtr+3);
	
	// populate const vector
	vector unsigned char constPerm = vec_lvsl( 0, constant.ToFloatPtr() );
	vector float v0 = vec_ld( 0, constant.ToFloatPtr() );
	vector float v1 = vec_ld( 15, constant.ToFloatPtr() );
	vector float vecConst = vec_perm( v0, v1, constPerm );
	
	vecConstX = vec_splat( vecConst, 0 );
	vecConstY = vec_splat( vecConst, 1 );
	vecConstZ = vec_splat( vecConst, 2 );
	vecConstI3 = vec_splat( vecConst, 3 );
		
	// handle unaligned case at beginning
	for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = constant.Normal() * src[i].Normal() + constant[3] * src[i][3];
	}
	
	const float *srcPtr = src[i].ToFloatPtr();
	vector unsigned char permVec = vec_add( vec_lvsl( -1, srcPtr ), (vector unsigned char)(1) );
	vector float vecOld = vec_ld( 0, srcPtr );
	
	for ( ; i+7 < count; i += 8 ) {
		float *planePtr = (float*)( srcPtr + (i*PLANE_OFFSET) );
		vector float v0, v1, v2, v3, v4, v5, v6, v7;
		
		v0 = vecOld; // vec_ld( 0, planePtr );
		v1 = vec_ld( 15, planePtr );
		v2 = vec_ld( 31, planePtr );
		v3 = vec_ld( 47, planePtr );
		v4 = vec_ld( 63, planePtr );
		v5 = vec_ld( 79, planePtr );
		v6 = vec_ld( 95, planePtr );
		v7 = vec_ld( 111, planePtr );
		vecOld = vec_ld( 127, planePtr );
	
		vecPlaneLd1 = vec_perm( v0, v1, permVec );
		vecPlaneLd2 = vec_perm( v1, v2, permVec );
		vecPlaneLd3 = vec_perm( v2, v3, permVec );
		vecPlaneLd4 = vec_perm( v3, v4, permVec );
		
		vecPlaneLd5 = vec_perm( v4, v5, permVec );
		vecPlaneLd6 = vec_perm( v5, v6, permVec );
		vecPlaneLd7 = vec_perm( v6, v7, permVec );
		vecPlaneLd8 = vec_perm( v7, vecOld, permVec );
		
		// permute into X Y Z vectors, since this is square its basically
		// a matrix transpose
		v0 = vec_mergeh( vecPlaneLd1, vecPlaneLd3 );
		v1 = vec_mergeh( vecPlaneLd2, vecPlaneLd4 );
		v2 = vec_mergel( vecPlaneLd1, vecPlaneLd3 );
		v3 = vec_mergel( vecPlaneLd2, vecPlaneLd4 );
			
		vecX = vec_mergeh( v0, v1 );
		vecY = vec_mergel( v0, v1 );
		vecZ = vec_mergeh( v2, v3 );
		vecI3 = vec_mergel( v2, v3 );
		
		v4 = vec_mergeh( vecPlaneLd5, vecPlaneLd7 );
		v5 = vec_mergeh( vecPlaneLd6, vecPlaneLd8 );
		v6 = vec_mergel( vecPlaneLd5, vecPlaneLd7 );
		v7 = vec_mergel( vecPlaneLd6, vecPlaneLd8 );
			
		vecX2 = vec_mergeh( v4, v5 );
		vecY2 = vec_mergel( v4, v5 );
		vecZ2 = vec_mergeh( v6, v7 );
		vecI32 = vec_mergel( v6, v7 );		
		
		// do calculation
		v4 = vec_madd( vecConstX, vecX, zeroVector );
		v5 = vec_madd( vecConstY, vecY, v4 );
		v6 = vec_madd( vecConstZ, vecZ, v5 );
		v7 = vec_madd( vecConstI3, vecI3, v6 );

		v0 = vec_madd( vecConstX, vecX2, zeroVector );
		v1 = vec_madd( vecConstY, vecY2, v0 );
		v2 = vec_madd( vecConstZ, vecZ2, v1 );
		v3 = vec_madd( vecConstI3, vecI32, v2 );
		
		//store result
		ALIGNED_STORE2( &dst[i], v7, v3 );
	}

	// cleanup
	for ( ; i < count; i++ ) {
		//dst[i] = constant.Normal() * src[i].Normal() + constant[3] * src[i][3];
		srcVal[0] = *(srcPtr + (i*PLANE_OFFSET) + 0 );
		srcVal[1] = *(srcPtr + (i*PLANE_OFFSET) + 1 );
		srcVal[2] = *(srcPtr + (i*PLANE_OFFSET) + 2 );
		srcVal[3] = *(srcPtr + (i*PLANE_OFFSET) + 3 );
		dst[i] = srcVal[0] * constVal[0] + srcVal[1] * constVal[1] + srcVal[2] * constVal[2] + constVal[3] * srcVal[3];
	}
}


#ifndef DRAWVERT_PADDED
/*
============
idSIMD_AltiVec::Dot

  dst[i] = constant.Normal() * src[i].xyz + constant[3];
============
*/
void VPCALL idSIMD_AltiVec::Dot( float *dst, const idPlane &constant, const idDrawVert *src, const int count ) {
//#define OPER(X) dst[(X)] = constant.Normal() * src[(X)].xyz + constant[3];
		
	// idDrawVert size is 60 bytes
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof( float ) );
					
	int i;
	const float *constPtr = constant.ToFloatPtr();
	const float *srcPtr = src[0].xyz.ToFloatPtr();
	
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector float vecConstX, vecConstY, vecConstZ, vecConstI3;
	register vector float vecSrcX1, vecSrcY1, vecSrcZ1;
	register vector float vecDest1;
	register vector float zeroVector = (vector float)(0.0);
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	
	float constVal[4];
	float srcVal[3];
	
	constVal[0] = *(constPtr+0);
	constVal[1] = *(constPtr+1);
	constVal[2] = *(constPtr+2);
	constVal[3] = *(constPtr+3);

	// populate const vec
	vector unsigned char constPerm = vec_lvsl( 0, constant.ToFloatPtr() );
	v0 = vec_ld( 0, constant.ToFloatPtr() );
	v1 = vec_ld( 15, constant.ToFloatPtr() );
	v0 = vec_perm( v0, v1, constPerm );
	
	vecConstX = vec_splat( v0, 0 );
	vecConstY = vec_splat( v0, 1 );
	vecConstZ = vec_splat( v0, 2 );
	vecConstI3 = vec_splat( v0, 3 );
	
	// handle unaligned case at beginning
	for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = constant.Normal() * src[i].xyz + constant[3];
	}
	
	// every fourth one will have the same alignment, so can store these. Make sure we
	// have enough so we don't run off the end of the array
	if ( i+3 < count ) {
			vertPerm1 = vec_add( vec_lvsl( -1, (float*) src[i].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
			vertPerm2 = vec_add( vec_lvsl( -1, (float*) src[i+1].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
			vertPerm3 = vec_add( vec_lvsl( -1, (float*) src[i+2].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
			vertPerm4 = vec_add( vec_lvsl( -1, (float*) src[i+3].xyz.ToFloatPtr() ), (vector unsigned char)(1) );	
	}
	
	for ( ; i+3 < count; i+=4 ) {
			const float *vertPtr = src[i].xyz.ToFloatPtr();
			const float *vertPtr2 = src[i+1].xyz.ToFloatPtr();
			const float *vertPtr3 = src[i+2].xyz.ToFloatPtr();
			const float *vertPtr4 = src[i+3].xyz.ToFloatPtr();
		
			v0 = vec_ld( 0, vertPtr );
			v1 = vec_ld( 11, vertPtr );
			v2 = vec_ld( 0, vertPtr2 );
			v3 = vec_ld( 11, vertPtr2 );
			v4 = vec_ld( 0, vertPtr3 );
			v5 = vec_ld( 11, vertPtr3 );
			v6 = vec_ld( 0, vertPtr4 );
			v7 = vec_ld( 11, vertPtr4 );
			
			v0 = vec_perm( v0, v1, vertPerm1 );
			v2 = vec_perm( v2, v3, vertPerm2 );
			v4 = vec_perm( v4, v5, vertPerm3 );
			v6 = vec_perm( v6, v7, vertPerm4 );

			// transpose into X Y Z vectors			
			v1 = vec_mergeh( v0, v4 );
			v3 = vec_mergeh( v2, v6 );
			v5 = vec_mergel( v0, v4 );
			v7 = vec_mergel( v2, v6 );
			
			vecSrcX1 = vec_mergeh( v1, v3 );
			vecSrcY1 = vec_mergel( v1, v3 );
			vecSrcZ1 = vec_mergeh( v5, v7 );

			// now calculate dot product
			vecSrcX1 = vec_madd( vecSrcX1, vecConstX, zeroVector );
			vecSrcY1 = vec_madd( vecSrcY1, vecConstY, vecSrcX1 );
			vecSrcZ1 = vec_madd( vecSrcZ1, vecConstZ, vecSrcY1 );
			vecDest1 = vec_add( vecSrcZ1, vecConstI3 );
			
			// store results
			vec_st( vecDest1, 0, &dst[i] );
	}

	// cleanup
	for ( ; i < count; i++ ) {
		srcVal[0] = *(srcPtr + (i*DRAWVERT_OFFSET) + 0 );
		srcVal[1] = *(srcPtr + (i*DRAWVERT_OFFSET) + 1 );
		srcVal[2] = *(srcPtr + (i*DRAWVERT_OFFSET) + 2 );
		//	dst[i] = constant.Normal() * src[i].xyz + constant[3];
		
		dst[i] = constVal[0] * srcVal[0] + constVal[1] * srcVal[1] + constVal[2] * srcVal[2];
		dst[i] += constVal[3];
	}
}
#else
/*
============
idSIMD_AltiVec::Dot

  dst[i] = constant.Normal() * src[i].xyz + constant[3];
============
*/
void VPCALL idSIMD_AltiVec::Dot( float *dst, const idPlane &constant, const idDrawVert *src, const int count ) {
//#define OPER(X) dst[(X)] = constant.Normal() * src[(X)].xyz + constant[3];
		
	// idDrawVert size is 60 bytes
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof( float ) );
					
	int i;
	const float *constPtr = constant.ToFloatPtr();
	const float *srcPtr = src[0].xyz.ToFloatPtr();
	
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector float vecConstX, vecConstY, vecConstZ, vecConstI3;
	register vector float vecSrcX1, vecSrcY1, vecSrcZ1;
	register vector float vecDest1;
	register vector float zeroVector = (vector float)(0.0);
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	
	float constVal[4];
	float srcVal[3];
	
	constVal[0] = *(constPtr+0);
	constVal[1] = *(constPtr+1);
	constVal[2] = *(constPtr+2);
	constVal[3] = *(constPtr+3);

	// populate const vec
	vector unsigned char constPerm = vec_lvsl( 0, constant.ToFloatPtr() );
	v0 = vec_ld( 0, constant.ToFloatPtr() );
	v1 = vec_ld( 15, constant.ToFloatPtr() );
	v0 = vec_perm( v0, v1, constPerm );
	
	vecConstX = vec_splat( v0, 0 );
	vecConstY = vec_splat( v0, 1 );
	vecConstZ = vec_splat( v0, 2 );
	vecConstI3 = vec_splat( v0, 3 );
	
	// handle unaligned case at beginning
	for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = constant.Normal() * src[i].xyz + constant[3];
	}
	
		for ( ; i+3 < count; i+=4 ) {
			const float *vertPtr = src[i].xyz.ToFloatPtr();
			const float *vertPtr2 = src[i+1].xyz.ToFloatPtr();
			const float *vertPtr3 = src[i+2].xyz.ToFloatPtr();
			const float *vertPtr4 = src[i+3].xyz.ToFloatPtr();
		
			v0 = vec_ld( 0, vertPtr );
			v2 = vec_ld( 0, vertPtr2 );
			v4 = vec_ld( 0, vertPtr3 );
			v6 = vec_ld( 0, vertPtr4 );

			// transpose into X Y Z vectors			
			v1 = vec_mergeh( v0, v4 );
			v3 = vec_mergeh( v2, v6 );
			v5 = vec_mergel( v0, v4 );
			v7 = vec_mergel( v2, v6 );
			
			vecSrcX1 = vec_mergeh( v1, v3 );
			vecSrcY1 = vec_mergel( v1, v3 );
			vecSrcZ1 = vec_mergeh( v5, v7 );

			// now calculate dot product
			vecSrcX1 = vec_madd( vecSrcX1, vecConstX, zeroVector );
			vecSrcY1 = vec_madd( vecSrcY1, vecConstY, vecSrcX1 );
			vecSrcZ1 = vec_madd( vecSrcZ1, vecConstZ, vecSrcY1 );
			vecDest1 = vec_add( vecSrcZ1, vecConstI3 );
			
			// store results
			vec_st( vecDest1, 0, &dst[i] );
	}

	// cleanup
	for ( ; i < count; i++ ) {
		srcVal[0] = *(srcPtr + (i*DRAWVERT_OFFSET) + 0 );
		srcVal[1] = *(srcPtr + (i*DRAWVERT_OFFSET) + 1 );
		srcVal[2] = *(srcPtr + (i*DRAWVERT_OFFSET) + 2 );
		//	dst[i] = constant.Normal() * src[i].xyz + constant[3];
		
		dst[i] = constVal[0] * srcVal[0] + constVal[1] * srcVal[1] + constVal[2] * srcVal[2];
		dst[i] += constVal[3];
	}
}

#endif /* DRAWVERT_PADDED */

/*
============
idSIMD_AltiVec::Dot

  dst[i] = src0[i] * src1[i];
============
*/
void VPCALL idSIMD_AltiVec::Dot( float *dst, const idVec3 *src0, const idVec3 *src1, const int count ) {
//#define OPER(X) dst[(X)] = src0[(X)] * src1[(X)];
		
	int i;
	float src0Val[3];
	float src1Val[3];
	
	register vector float vecLd1, vecLd2, vecLd3, vecLd4, vecLd5, vecLd6;
	vector float vecLd7, vecLd8, vecLd9, vecLd10, vecLd11, vecLd12;
	register vector float vecX0, vecY0, vecZ0, vecX1, vecY1, vecZ1;
	register vector float vecX02, vecY02, vecZ02, vecX12, vecY12, vecZ12;
	register vector float zeroVector = (vector float)(0.0);
	// permute vectors
	register vector unsigned char permX1 = (vector unsigned char)(0,1,2,3,12,13,14,15,24,25,26,27,28,29,30,31); //last 4 bytes are junk
	register vector unsigned char permX2 = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,20,21,22,23);
	register vector unsigned char permY1 = (vector unsigned char)(4,5,6,7,16,17,18,19,28,29,30,31,0,1,2,3); //last 4 bytes are junk
	register vector unsigned char permY2 = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,24,25,26,27);
	register vector unsigned char permZ1 = (vector unsigned char)(8,9,10,11,20,21,22,23,0,1,2,3,4,5,6,7); //last 8 bytes are junk
	register vector unsigned char permZ2 = (vector unsigned char)(0,1,2,3,4,5,6,7,16,17,18,19,28,29,30,31);
	
	// handle unaligned case at beginning
	for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = src0[i] * src1[i];
	}
	
	const float *src0Ptr = src0[i].ToFloatPtr();
	const float *src1Ptr = src1[i].ToFloatPtr();
	vector unsigned char permVec1 = vec_add( vec_lvsl( -1, src0Ptr ), (vector unsigned char)(1) );
	vector unsigned char permVec2 = vec_add( vec_lvsl( -1, src1Ptr ), (vector unsigned char)(1) );
	vector float vecOld0 = vec_ld( 0, src0Ptr );
	vector float vecOld1 = vec_ld( 0, src1Ptr );
	
	for ( i = 0; i+7 < count; i += 8 ) {
			float *s0Ptr = (float*)( src0Ptr + (i*3) );
			float *s1Ptr = (float*)( src1Ptr + (i*3) );
			
			vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11;
			v0 = vecOld0; 
			v1 = vec_ld( 15, s0Ptr );
			v2 = vec_ld( 31, s0Ptr );
			v3 = vec_ld( 47, s0Ptr );
			v4 = vec_ld( 63, s0Ptr );
			v5 = vec_ld( 79, s0Ptr );
			vecOld0 = vec_ld( 95, s0Ptr );
			
			v6 = vecOld1; 
			v7 = vec_ld( 15, s1Ptr );
			v8 = vec_ld( 31, s1Ptr );
			v9 = vec_ld( 47, s1Ptr );
			v10 = vec_ld( 63, s1Ptr );
			v11 = vec_ld( 79, s1Ptr );
			vecOld1 = vec_ld( 95, s1Ptr );
			
			vecLd1 = vec_perm( v0, v1, permVec1 );
			vecLd2 = vec_perm( v1, v2, permVec1 );
			vecLd3 = vec_perm( v2, v3, permVec1 );
			vecLd4 = vec_perm( v3, v4, permVec1 );
			vecLd5 = vec_perm( v4, v5, permVec1 );
			vecLd6 = vec_perm( v5, vecOld0, permVec1 );
			
			vecLd7 = vec_perm( v6, v7, permVec2 );
			vecLd8 = vec_perm( v7, v8, permVec2 );
			vecLd9 = vec_perm( v8, v9, permVec2 );
			vecLd10 = vec_perm( v9, v10, permVec2 );
			vecLd11 = vec_perm( v10, v11, permVec2 );
			vecLd12 = vec_perm( v11, vecOld1, permVec2 );
			
			// permute into X Y Z vectors
			vecX0 = vec_perm( vecLd1, vecLd2, permX1 );
			vecY0 = vec_perm( vecLd1, vecLd2, permY1 );
			vecZ0 = vec_perm( vecLd1, vecLd2, permZ1 );
			vecX0 = vec_perm( vecX0, vecLd3, permX2 );
			vecY0 = vec_perm( vecY0, vecLd3, permY2 );
			vecZ0 = vec_perm( vecZ0, vecLd3, permZ2 );
			
			vecX02 = vec_perm( vecLd4, vecLd5, permX1 );
			vecY02 = vec_perm( vecLd4, vecLd5, permY1 );
			vecZ02 = vec_perm( vecLd4, vecLd5, permZ1 );
			vecX02 = vec_perm( vecX02, vecLd6, permX2 );
			vecY02 = vec_perm( vecY02, vecLd6, permY2 );
			vecZ02 = vec_perm( vecZ02, vecLd6, permZ2 );
			
			vecX1 = vec_perm( vecLd7, vecLd8, permX1 );
			vecY1 = vec_perm( vecLd7, vecLd8, permY1 );
			vecZ1 = vec_perm( vecLd7, vecLd8, permZ1 );
			vecX1 = vec_perm( vecX1, vecLd9, permX2 );
			vecY1 = vec_perm( vecY1, vecLd9, permY2 );
			vecZ1 = vec_perm( vecZ1, vecLd9, permZ2 );
			
			vecX12 = vec_perm( vecLd10, vecLd11, permX1 );
			vecY12 = vec_perm( vecLd10, vecLd11, permY1 );
			vecZ12 = vec_perm( vecLd10, vecLd11, permZ1 );
			vecX12 = vec_perm( vecX12, vecLd12, permX2 );
			vecY12 = vec_perm( vecY12, vecLd12, permY2 );
			vecZ12 = vec_perm( vecZ12, vecLd12, permZ2 );			
			
			// do multiply
			vecX0 = vec_madd( vecX0, vecX1, zeroVector );
			vecY0 = vec_madd( vecY0, vecY1, vecX0 );
			vecZ0 = vec_madd( vecZ0, vecZ1, vecY0 );
			vecX02 = vec_madd( vecX02, vecX12, zeroVector );
			vecY02 = vec_madd( vecY02, vecY12, vecX02 );
			vecZ02 = vec_madd( vecZ02, vecZ12, vecY02 );			
						
			// store out results
			ALIGNED_STORE2( &dst[i], vecZ0, vecZ02 );
	}
	
	// cleanup
	for ( ; i < count; i++ ) {
		//	dst[i] = src0[i] * src1[i];
		src0Val[0] = *( src0Ptr + (i*3) + 0 );
		src0Val[1] = *( src0Ptr + (i*3) + 1 );
		src0Val[2] = *( src0Ptr + (i*3) + 2 );
	
		src1Val[0] = *( src1Ptr + (i*3) + 0 );
		src1Val[1] = *( src1Ptr + (i*3) + 1 );
		src1Val[2] = *( src1Ptr + (i*3) + 2 );		
	
		dst[i] = src0Val[0] * src1Val[0] + src0Val[1] * src1Val[1] + src0Val[2] * src1Val[2];
	}
}

/*
============
idSIMD_AltiVec::Dot

  dot = src1[0] * src2[0] + src1[1] * src2[1] + src1[2] * src2[2] + ...
============
*/
void VPCALL idSIMD_AltiVec::Dot( float &dot, const float *src1, const float *src2, const int count ) {
	dot = 0.0f;

    register vector float v0, v1, v2, v3; 
    register vector float zeroVector;
    register vector float runningTotal1, runningTotal2;
    //src0
	register vector float v0_low, v0_hi, v2_low, v2_hi;
	//src1
	register vector float v1_low, v1_hi, v3_low, v3_hi;
	//permute vectors
	register vector unsigned char permVec1, permVec2;
	vector unsigned char oneCharVector = (vector unsigned char)(1);

    int i = 0;
        
     runningTotal1 = (vector float)(0.0);
	 runningTotal2 = (vector float)(0.0);
     zeroVector = (vector float)(0.0);
     
    if ( count >= 8 ) {
		//calculate permute and do loads
		permVec1 = vec_add( vec_lvsl( -1, (int*) &src1[i] ), oneCharVector );
		permVec2 = vec_add( vec_lvsl( -1, (int*) &src2[i] ), oneCharVector );
		v2_hi = vec_ld( 0, &src1[i] );
		v3_hi = vec_ld( 0, &src2[i] );

		//vectorize!
		for ( ; i+7 < count; i += 8 ) {
			//load sources
			v0_low = v2_hi;
			v0_hi = vec_ld( 15, &src1[i] );
			v2_low = v0_hi;
			v2_hi = vec_ld( 31, &src1[i] );
		
			v1_low = v3_hi;
			v1_hi = vec_ld( 15, &src2[i] );
			v3_low = v1_hi;
			v3_hi = vec_ld( 31, &src2[i] );
		
			v0 = vec_perm( v0_low, v0_hi, permVec1 );
			v1 = vec_perm( v1_low, v1_hi, permVec2 );
			v2 = vec_perm( v2_low, v2_hi, permVec1 );
			v3 = vec_perm( v3_low, v3_hi, permVec2 );

			//multiply together and keep running sum
			runningTotal1 = vec_madd( v0, v1, runningTotal1 );
			runningTotal2 = vec_madd( v2, v3, runningTotal2 );
		}
		
		runningTotal1 = vec_add( runningTotal1, runningTotal2 );
    
		// sum accross vector
		v0 = vec_add( runningTotal1, vec_sld( runningTotal1, runningTotal1, 8 ) );
		v1 = vec_add( v0, vec_sld( v0, v0, 4 ) );
		runningTotal1 = vec_splat( v1, 0 );
		vec_ste( runningTotal1, 0, &dot );
	} 
    
    //handle cleanup. when profiling the game, we found that most of the counts to this function were small, so it
	// spends a lot of time in this scalar code. It's already really really fast (eg 1 TB tick) for scalar code for 
	// counts less than 50, so not much point in trying to get vector code in on the action 
	for ( ; i < count ; i++ ) {
		dot += src1[i] * src2[i];
	}
	
}
#endif /* ENABLE_DOT */

#ifdef ENABLE_COMPARES

/*
============
idSIMD_AltiVec::CmpGT

  dst[i] = src0[i] > constant;
============
*/

void VPCALL idSIMD_AltiVec::CmpGT( byte *dst, const float *src0, const float constant, const int count ) {
//#define OPER(X) dst[(X)] = src0[(X)] > constant;

    register vector float v0, v1, v2, v3;
    register vector bool int vr1, vr2, vr3, vr4;
	register vector bool short vs1, vs2;
	register vector float v0_low, v0_hi, v1_low, v1_hi, v2_low, v2_hi, v3_low, v3_hi;
	register vector unsigned char vc1;
	register vector bool char vbc1;
    register vector float constVec;
    register vector unsigned char oneVector = (vector unsigned char)(1);
	register vector unsigned char permVec;
    int i;

    //handle unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] = src0[i] > constant;
    }
    
	//splat constant into a vector
    constVec = loadSplatUnalignedScalar( &constant );
	
	//calculate permute and do loads
	permVec = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneVector );
	v3_hi = vec_ld( 0, &src0[i] );
	
    //vectorize!
    for ( ; i+15 < count; i += 16 ) {
		// load values
		v0_low = v3_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &src0[i] );
		v2_low = v1_hi;
		v2_hi = vec_ld( 47, &src0[i] );
		v3_low = v2_hi;
		v3_hi = vec_ld( 63, &src0[i] );
	
		//permute into the vectors we want
		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );
		v2 = vec_perm( v2_low, v2_hi, permVec );
		v3 = vec_perm( v3_low, v3_hi, permVec );
		
		//do comparison
		vr1 = vec_cmpgt( v0, constVec );
		vr2 = vec_cmpgt( v1, constVec );
		vr3 = vec_cmpgt( v2, constVec );
		vr4 = vec_cmpgt( v3, constVec );
		
		// pack results into shorts
		vs1 = vec_pack(vr1, vr2);
		vs2 = vec_pack(vr3, vr4);
		
		// pack results into byte
		vbc1 = vec_pack(vs1, vs2);
		
		//AND with 1 to get true=1 not true=255
		vc1 = vec_and( vbc1, oneVector );
		
		//store results
		vec_st( vc1, 0, &dst[i] );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = src0[i] > constant;
    }
}


/*
============
idSIMD_AltiVec::CmpGT

  dst[i] |= ( src0[i] > constant ) << bitNum;
============
*/
void VPCALL idSIMD_AltiVec::CmpGT( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) {
//#define OPER(X) dst[(X)] |= ( src0[(X)] > constant ) << bitNum;

	// Temp vector registers
    register vector bool int vtbi0, vtbi1, vtbi2, vtbi3;
	register vector bool short vtbs0, vtbs1;
	register vector bool char vtbc0;
    register vector unsigned char vtuc0;
	register vector unsigned char permVec, permVec2;	
	
	// dest vectors
	register vector unsigned char vd;	
	// bitNum vectors
    register vector unsigned char bitNumVec;
	// src0 vectors
    register vector float vs0, vs1, vs2, vs3;
	register vector float vs0_low, vs0_hi, vs1_low, vs1_hi, vs2_low, vs2_hi, vs3_low, vs3_hi;
	// constant vector
    register vector float constVec;
	// all one's 
	register vector unsigned char oneVector = (vector unsigned char)(1);
	int i = 0;
	
	//handle unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] |= ( src0[i] > constant ) << bitNum;
    }
	
	//splat constant into a vector
	constVec = loadSplatUnalignedScalar( &constant );
    
	//bitNum is unaligned. 
    permVec2 = vec_lvsl( 0, &bitNum );
    vtuc0 = vec_ld( 0, &bitNum );
    bitNumVec = vec_perm( vtuc0, vtuc0, permVec2 );
    bitNumVec = vec_splat( bitNumVec, 0 );

	//calculate permute and do loads
	permVec = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneVector );
	vs3_hi = vec_ld( 0, &src0[i] );	

    //vectorize!
    for ( ; i+15 < count; i += 16 ) {
		//load sources (floats)		
		vs0_low = vs3_hi;
		vs0_hi = vec_ld( 15, &src0[i] );
		vs1_low = vs0_hi;
		vs1_hi = vec_ld( 31, &src0[i] );
		vs2_low = vs1_hi;
		vs2_hi = vec_ld( 47, &src0[i] );
		vs3_low = vs2_hi;
		vs3_hi = vec_ld( 63, &src0[i] );
		
		//permute into the vectors we want
		vs0 = vec_perm( vs0_low, vs0_hi, permVec );
		vs1 = vec_perm( vs1_low, vs1_hi, permVec );
		vs2 = vec_perm( vs2_low, vs2_hi, permVec );
		vs3 = vec_perm( vs3_low, vs3_hi, permVec );
		
		//load dest (bytes) as unsigned char
		vd = vec_ld( 0, &dst[i] );
		
		// do comparison and get bool int result
		vtbi0 = vec_cmpgt( vs0, constVec );
		vtbi1 = vec_cmpgt( vs1, constVec );
		vtbi2 = vec_cmpgt( vs2, constVec );
		vtbi3 = vec_cmpgt( vs3, constVec );	
		
		// pack results into shorts
		vtbs0 = vec_pack(vtbi0, vtbi1);
		vtbs1 = vec_pack(vtbi2, vtbi3);
		
		// pack results into byte
		vtbc0 = vec_pack(vtbs0, vtbs1);
		
		//and with 1 to get true=1 instead of true=255
		vtuc0 = vec_and(vtbc0, oneVector);
		vtuc0 = vec_sl(vtuc0, bitNumVec );
		
		//or with original
		vd = vec_or( vd, vtuc0 );
		
		vec_st( vd, 0, &dst[i] );
    }
    
    //handle cleanup
	for ( ; i < count ; i++ ) {
		dst[i] |= ( src0[i] > constant ) << bitNum;
    }
}

/*
============
idSIMD_AltiVec::CmpGE

  dst[i] = src0[i] >= constant;
============
*/
void VPCALL idSIMD_AltiVec::CmpGE( byte *dst, const float *src0, const float constant, const int count ) {

    register vector float v0, v1, v2, v3;
    register vector bool int vr1, vr2, vr3, vr4;
	register vector bool short vs1, vs2;
	register vector float v0_low, v0_hi, v1_low, v1_hi, v2_low, v2_hi, v3_low, v3_hi;
	register vector unsigned char vc1;
	register vector bool char vbc1;
    register vector float constVec;
    register vector unsigned char oneVector = (vector unsigned char)(1);
	register vector unsigned char permVec;
    int i = 0;

    //handle unaligned at start
    for ( ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] = src0[i] >= constant;
    }
    
	//splat constant into a vector
    constVec = loadSplatUnalignedScalar( &constant );
	
	//calculate permute and do loads
	permVec = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneVector );
	v3_hi = vec_ld( 0, &src0[i] );
	
    //vectorize!
    for ( ; i+15 < count; i += 16 ) {
		// load values
		v0_low = v3_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &src0[i] );
		v2_low = v1_hi;
		v2_hi = vec_ld( 47, &src0[i] );
		v3_low = v2_hi;
		v3_hi = vec_ld( 63, &src0[i] );

		//permute into the vectors we want
		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );
		v2 = vec_perm( v2_low, v2_hi, permVec );
		v3 = vec_perm( v3_low, v3_hi, permVec );

		//do comparison
		vr1 = vec_cmpge( v0, constVec );
		vr2 = vec_cmpge( v1, constVec );
		vr3 = vec_cmpge( v2, constVec );
		vr4 = vec_cmpge( v3, constVec );

		// pack results into shorts
		vs1 = vec_pack(vr1, vr2);
		vs2 = vec_pack(vr3, vr4);

		// pack results into byte
		vbc1 = vec_pack(vs1, vs2);

		//AND with 1 to get true=1 not true=255
		vc1 = vec_and( vbc1, oneVector );

		//store results
		vec_st( vc1, 0, &dst[i] );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = src0[i] >= constant;
    }
}

/*
============
idSIMD_AltiVec::CmpGE

  dst[i] |= ( src0[i] >= constant ) << bitNum;
============
*/
void VPCALL idSIMD_AltiVec::CmpGE( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) {
    register vector bool int vtbi0, vtbi1, vtbi2, vtbi3;
	register vector bool short vtbs0, vtbs1;
	register vector bool char vtbc0;
    register vector unsigned char vtuc0;
	register vector unsigned char permVec, permVec2;	
	
	// dest vectors
	register vector unsigned char vd;	
	// bitNum vectors
    register vector unsigned char bitNumVec;
	// src0 vectors
    register vector float vs0, vs1, vs2, vs3;
	register vector float vs0_low, vs0_hi, vs1_low, vs1_hi, vs2_low, vs2_hi, vs3_low, vs3_hi;
	// constant vector
    register vector float constVec;
	// all one's 
	register vector unsigned char oneVector = (vector unsigned char)(1);
	int i = 0;
	
	//handle unaligned at start
    for (  ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] |= ( src0[i] >= constant ) << bitNum;
    }
	
	//splat constant into a vector
	constVec = loadSplatUnalignedScalar( &constant );
    
	//bitNum is unaligned. 
    permVec2 = vec_lvsl( 0, &bitNum );
    vtuc0 = vec_ld( 0, &bitNum );
    bitNumVec = vec_perm( vtuc0, vtuc0, permVec2 );
    bitNumVec = vec_splat( bitNumVec, 0 );

	//calculate permute and do loads
	permVec = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneVector );
	vs3_hi = vec_ld( 0, &src0[i] );	

    //vectorize!
    for ( ; i+15 < count; i += 16 ) {
		//load sources (floats)		
		vs0_low = vs3_hi;
		vs0_hi = vec_ld( 15, &src0[i] );
		vs1_low = vs0_hi;
		vs1_hi = vec_ld( 31, &src0[i] );
		vs2_low = vs1_hi;
		vs2_hi = vec_ld( 47, &src0[i] );
		vs3_low = vs2_hi;
		vs3_hi = vec_ld( 63, &src0[i] );
		
		//permute into the vectors we want
		vs0 = vec_perm( vs0_low, vs0_hi, permVec );
		vs1 = vec_perm( vs1_low, vs1_hi, permVec );
		vs2 = vec_perm( vs2_low, vs2_hi, permVec );
		vs3 = vec_perm( vs3_low, vs3_hi, permVec );
		
		//load dest (bytes) as unsigned char
		vd = vec_ld( 0, &dst[i] );
		
		// do comparison and get bool int result
		vtbi0 = vec_cmpge( vs0, constVec );
		vtbi1 = vec_cmpge( vs1, constVec );
		vtbi2 = vec_cmpge( vs2, constVec );
		vtbi3 = vec_cmpge( vs3, constVec );	
		
		// pack results into shorts
		vtbs0 = vec_pack(vtbi0, vtbi1);
		vtbs1 = vec_pack(vtbi2, vtbi3);
		
		// pack results into byte
		vtbc0 = vec_pack(vtbs0, vtbs1);
		
		//and with 1L to get true=1 instead of true=255
		vtuc0 = vec_and(vtbc0, oneVector);
		vtuc0 = vec_sl(vtuc0, bitNumVec );
		
		//or with original
		vd = vec_or( vd, vtuc0 );
		
		vec_st( vd, 0, &dst[i] );
    }
    
    //handle cleanup
	for ( ; i < count ; i++ ) {
		dst[i] |= ( src0[i] >= constant ) << bitNum;
    }
}


/*
============
idSIMD_AltiVec::CmpLT

  dst[i] = src0[i] < constant;
============
*/
void VPCALL idSIMD_AltiVec::CmpLT( byte *dst, const float *src0, const float constant, const int count ) {
//#define OPER(X) dst[(X)] = src0[(X)] < constant;
    register vector float v0, v1, v2, v3;
    register vector bool int vr1, vr2, vr3, vr4;
	register vector bool short vs1, vs2;
	register vector float v0_low, v0_hi, v1_low, v1_hi, v2_low, v2_hi, v3_low, v3_hi;
	register vector unsigned char vc1;
	register vector bool char vbc1;
    register vector float constVec;
    register vector unsigned char oneVector = (vector unsigned char)(1);
	register vector unsigned char permVec;
    int i = 0;

    //handle unaligned at start
    for ( ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] = src0[i] < constant;
    }
    
	//splat constant into a vector
    constVec = loadSplatUnalignedScalar( &constant );
	
	//calculate permute and do loads
	permVec = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneVector );
	v3_hi = vec_ld( 0, &src0[i] );
	
    //vectorize!
    for ( ; i+15 < count; i += 16 ) {
		// load values
		v0_low = v3_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &src0[i] );
		v2_low = v1_hi;
		v2_hi = vec_ld( 47, &src0[i] );
		v3_low = v2_hi;
		v3_hi = vec_ld( 63, &src0[i] );
	
		//permute into the vectors we want
		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );
		v2 = vec_perm( v2_low, v2_hi, permVec );
		v3 = vec_perm( v3_low, v3_hi, permVec );
		
		//do comparison
		vr1 = vec_cmplt( v0, constVec );
		vr2 = vec_cmplt( v1, constVec );
		vr3 = vec_cmplt( v2, constVec );
		vr4 = vec_cmplt( v3, constVec );
		
		// pack results into shorts
		vs1 = vec_pack(vr1, vr2);
		vs2 = vec_pack(vr3, vr4);
		
		// pack results into byte
		vbc1 = vec_pack(vs1, vs2);
		
		//AND with 1 to get true=1 not true=255
		vc1 = vec_and( vbc1, oneVector );
		
		//store results
		vec_st( vc1, 0, &dst[i] );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = src0[i] < constant;
    }
}

/*
============
idSIMD_AltiVec::CmpLT

  dst[i] |= ( src0[i] < constant ) << bitNum;
============
*/
void VPCALL idSIMD_AltiVec::CmpLT( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) {
//#define OPER(X) dst[(X)] |= ( src0[(X)] < constant ) << bitNum;
    register vector bool int vtbi0, vtbi1, vtbi2, vtbi3;
	register vector bool short vtbs0, vtbs1;
	register vector bool char vtbc0;
    register vector unsigned char vtuc0;
	register vector unsigned char permVec, permVec2;	
	
	// dest vectors
	register vector unsigned char vd;	
	// bitNum vectors
    register vector unsigned char bitNumVec;
	// src0 vectors
    register vector float vs0, vs1, vs2, vs3;
	register vector float vs0_low, vs0_hi, vs1_low, vs1_hi, vs2_low, vs2_hi, vs3_low, vs3_hi;
	// constant vector
    register vector float constVec;
	// all one's 
	register vector unsigned char oneVector = (vector unsigned char)(1);
	int i = 0;
	
	//handle unaligned at start
    for ( i = 0 ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] |= ( src0[i] < constant ) << bitNum;
    }
	
	//splat constant into a vector
	constVec = loadSplatUnalignedScalar( &constant );
    
	//bitNum is unaligned. 
    permVec2 = vec_lvsl( 0, &bitNum );
    vtuc0 = vec_ld( 0, &bitNum );
    bitNumVec = vec_perm( vtuc0, vtuc0, permVec2 );
    bitNumVec = vec_splat( bitNumVec, 0 );

	//calculate permute and do loads
	permVec = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneVector );
	vs3_hi = vec_ld( 0, &src0[i] );	

    //vectorize!
    for ( ; i+15 < count; i += 16 ) {
		//load sources (floats)		
		vs0_low = vs3_hi;
		vs0_hi = vec_ld( 15, &src0[i] );
		vs1_low = vs0_hi;
		vs1_hi = vec_ld( 31, &src0[i] );
		vs2_low = vs1_hi;
		vs2_hi = vec_ld( 47, &src0[i] );
		vs3_low = vs2_hi;
		vs3_hi = vec_ld( 63, &src0[i] );
		
		//permute into the vectors we want
		vs0 = vec_perm( vs0_low, vs0_hi, permVec );
		vs1 = vec_perm( vs1_low, vs1_hi, permVec );
		vs2 = vec_perm( vs2_low, vs2_hi, permVec );
		vs3 = vec_perm( vs3_low, vs3_hi, permVec );
		
		//load dest (bytes) as unsigned char
		vd = vec_ld( 0, &dst[i] );
		
		// do comparison and get bool int result
		vtbi0 = vec_cmplt( vs0, constVec );
		vtbi1 = vec_cmplt( vs1, constVec );
		vtbi2 = vec_cmplt( vs2, constVec );
		vtbi3 = vec_cmplt( vs3, constVec );	
		
		// pack results into shorts
		vtbs0 = vec_pack(vtbi0, vtbi1);
		vtbs1 = vec_pack(vtbi2, vtbi3);
		
		// pack results into byte
		vtbc0 = vec_pack(vtbs0, vtbs1);
		
		//and with 1L to get true=1 instead of true=255
		vtuc0 = vec_and(vtbc0, oneVector);
		vtuc0 = vec_sl(vtuc0, bitNumVec );
		
		//or with original
		vd = vec_or( vd, vtuc0 );
		
		vec_st( vd, 0, &dst[i] );
    }
    
    //handle cleanup
	for ( ; i < count ; i++ ) {
		dst[i] |= ( src0[i] < constant ) << bitNum;
    }

}
//#endif

/*
============
idSIMD_AltiVec::CmpLE

  dst[i] = src0[i] <= constant;
============
*/
void VPCALL idSIMD_AltiVec::CmpLE( byte *dst, const float *src0, const float constant, const int count ) {
//#define OPER(X) dst[(X)] = src0[(X)] <= constant;
    register vector float v0, v1, v2, v3;
    register vector bool int vr1, vr2, vr3, vr4;
	register vector bool short vs1, vs2;
	register vector float v0_low, v0_hi, v1_low, v1_hi, v2_low, v2_hi, v3_low, v3_hi;
	register vector unsigned char vc1;
	register vector bool char vbc1;
    register vector float constVec;
    register vector unsigned char oneVector = (vector unsigned char)(1);
	register vector unsigned char permVec;
    int i = 0;

    //handle unaligned at start
    for ( ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] = src0[i] <= constant;
    }
    
	//splat constant into a vector
    constVec = loadSplatUnalignedScalar( &constant );
	
	//calculate permute and do loads
	permVec = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneVector );
	v3_hi = vec_ld( 0, &src0[i] );
	
    //vectorize!
    for ( ; i+15 < count; i += 16 ) {
		// load values
		v0_low = v3_hi;
		v0_hi = vec_ld( 15, &src0[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &src0[i] );
		v2_low = v1_hi;
		v2_hi = vec_ld( 47, &src0[i] );
		v3_low = v2_hi;
		v3_hi = vec_ld( 63, &src0[i] );
	
		//permute into the vectors we want
		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );
		v2 = vec_perm( v2_low, v2_hi, permVec );
		v3 = vec_perm( v3_low, v3_hi, permVec );
		
		//do comparison
		vr1 = vec_cmple( v0, constVec );
		vr2 = vec_cmple( v1, constVec );
		vr3 = vec_cmple( v2, constVec );
		vr4 = vec_cmple( v3, constVec );
		
		// pack results into shorts
		vs1 = vec_pack(vr1, vr2);
		vs2 = vec_pack(vr3, vr4);
		
		// pack results into byte
		vbc1 = vec_pack(vs1, vs2);
		
		//AND with 1 to get true=1 not true=255
		vc1 = vec_and( vbc1, oneVector );
		
		//store results
		vec_st( vc1, 0, &dst[i] );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
        dst[i] = src0[i] <= constant;
    }
}

/*
============
idSIMD_AltiVec::CmpLE

  dst[i] |= ( src0[i] <= constant ) << bitNum;
============
*/
void VPCALL idSIMD_AltiVec::CmpLE( byte *dst, const byte bitNum, const float *src0, const float constant, const int count ) {
//#define OPER(X) dst[(X)] |= ( src0[(X)] <= constant ) << bitNum;
    register vector bool int vtbi0, vtbi1, vtbi2, vtbi3;
	register vector bool short vtbs0, vtbs1;
	register vector bool char vtbc0;
    register vector unsigned char vtuc0;
	register vector unsigned char permVec, permVec2;	
	
	// dest vectors
	register vector unsigned char vd;	
	// bitNum vectors
    register vector unsigned char bitNumVec;
	// src0 vectors
    register vector float vs0, vs1, vs2, vs3;
	register vector float vs0_low, vs0_hi, vs1_low, vs1_hi, vs2_low, vs2_hi, vs3_low, vs3_hi;
	// constant vector
    register vector float constVec;
	// all one's 
	register vector unsigned char oneVector = (vector unsigned char)(1);
	int i = 0;
	
	//handle unaligned at start
    for ( ; NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count );i++ ) {
		dst[i] |= ( src0[i] <= constant ) << bitNum;
    }
	
	//splat constant into a vector
	constVec = loadSplatUnalignedScalar( &constant );
    
	//bitNum is unaligned. 
    permVec2 = vec_lvsl( 0, &bitNum );
    vtuc0 = vec_ld( 0, &bitNum );
    bitNumVec = vec_perm( vtuc0, vtuc0, permVec2 );
    bitNumVec = vec_splat( bitNumVec, 0 );

	//calculate permute and do loads
	permVec = vec_add( vec_lvsl( -1, (int*) &src0[i] ), oneVector );
	vs3_hi = vec_ld( 0, &src0[i] );	

    //vectorize!
    for ( ; i+15 < count; i += 16 ) {
		//load sources (floats)		
		vs0_low = vs3_hi;
		vs0_hi = vec_ld( 15, &src0[i] );
		vs1_low = vs0_hi;
		vs1_hi = vec_ld( 31, &src0[i] );
		vs2_low = vs1_hi;
		vs2_hi = vec_ld( 47, &src0[i] );
		vs3_low = vs2_hi;
		vs3_hi = vec_ld( 63, &src0[i] );
		
		//permute into the vectors we want
		vs0 = vec_perm( vs0_low, vs0_hi, permVec );
		vs1 = vec_perm( vs1_low, vs1_hi, permVec );
		vs2 = vec_perm( vs2_low, vs2_hi, permVec );
		vs3 = vec_perm( vs3_low, vs3_hi, permVec );
		
		//load dest (bytes) as unsigned char
		vd = vec_ld( 0, &dst[i] );
		
		// do comparison and get bool int result
		vtbi0 = vec_cmple( vs0, constVec );
		vtbi1 = vec_cmple( vs1, constVec );
		vtbi2 = vec_cmple( vs2, constVec );
		vtbi3 = vec_cmple( vs3, constVec );	
		
		// pack results into shorts
		vtbs0 = vec_pack(vtbi0, vtbi1);
		vtbs1 = vec_pack(vtbi2, vtbi3);
		
		// pack results into byte
		vtbc0 = vec_pack(vtbs0, vtbs1);
		
		//and with 1L to get true=1 instead of true=255
		vtuc0 = vec_and(vtbc0, oneVector);
		vtuc0 = vec_sl(vtuc0, bitNumVec );
		
		//or with original
		vd = vec_or( vd, vtuc0 );
		
		vec_st( vd, 0, &dst[i] );
    }
    
    //handle cleanup
	for ( ; i < count ; i++ ) {
		dst[i] |= ( src0[i] <= constant ) << bitNum;
    }
}
#endif /* ENABLE_COMPARES */

#ifdef ENABLE_MINMAX

/*
============
idSIMD_AltiVec::MinMax
============
*/
void VPCALL idSIMD_AltiVec::MinMax( float &min, float &max, const float *src, const int count ) {
	min = idMath::INFINITY; max = -idMath::INFINITY;
//#define OPER(X) if ( src[(X)] < min ) {min = src[(X)];} if ( src[(X)] > max ) {max = src[(X)];}

    register vector float v0, v1, v2, v3;
    register vector float maxVec, minVec, tempMin, tempMax;
    register vector unsigned char permVec;
	register vector float v0_low, v0_hi, v1_low, v1_hi; 
	vector unsigned char oneCharVector = (vector unsigned char)(1);
	int i = 0;
 
    if ( count >= 4 ) {
	
		//calculate permute and do first load to
		//get a starting point for min and max
		permVec = vec_add( vec_lvsl( -1, (int*) &src[0] ), oneCharVector );
		v1_hi = vec_ld( 0, &src[0] );
	    
		maxVec = loadSplatUnalignedScalar( &max );
		minVec = loadSplatUnalignedScalar( &min );
		
		//vectorize!
		for ( ; i+7 < count; i += 8 ) {
			//load sources
			v0_low = v1_hi;
			v0_hi = vec_ld( 15, &src[i] );
			v1_low = v0_hi;
			v1_hi = vec_ld( 31, &src[i] );
			v0 = vec_perm( v0_low, v0_hi, permVec );
			v1 = vec_perm( v1_low, v1_hi, permVec );
			
			// minimum
			v2 = vec_min( v0, v1 );
			minVec = vec_min( minVec, v2 );
			// maximum
			v3 = vec_max( v0, v1 );
			maxVec = vec_max( maxVec, v3 );
		}
	
		//minVec and maxVec hold the min/max elements from the array, but now
		//we need to figure out which particular element it is
	
		tempMin = minVec;
		tempMax = maxVec;

		// rotate vector around and compare to itself to find the real min/max
		tempMin = vec_min( tempMin, vec_sld( tempMin, tempMin, 8 ) );
		tempMax = vec_max( tempMax, vec_sld( tempMax, tempMax, 8 ) );
		tempMin = vec_min( tempMin, vec_sld( tempMin, tempMin, 4 ) );
		tempMax = vec_max( tempMax, vec_sld( tempMax, tempMax, 4 ) );
		minVec = vec_splat( tempMin, 0 );
		maxVec = vec_splat( tempMax, 0 );
		vec_ste( minVec, 0, &min );
		vec_ste( maxVec, 0, &max );
    } 
	
	//cleanup
    for ( ; i < count; i++ ) {
		if ( src[i] < min ) {
			min = src[i];
		} 
		if ( src[i] > max ) {
			max = src[i];
		}
	}
}

/*
============
idSIMD_AltiVec::MinMax
============
*/
void VPCALL idSIMD_AltiVec::MinMax( idVec2 &min, idVec2 &max, const idVec2 *src, const int count ) {
	min[0] = min[1] = idMath::INFINITY; max[0] = max[1] = -idMath::INFINITY;
//#define OPER(X) const idVec2 &v = src[(X)]; if ( v[0] < min[0] ) { min[0] = v[0]; } if ( v[0] > max[0] ) { max[0] = v[0]; } if ( v[1] < min[1] ) { min[1] = v[1]; } if ( v[1] > max[1] ) { max[1] = v[1]; }

	idVec2 v;
	int i = 0;
	int j;
	
	const float *srcPtr = src[0].ToFloatPtr();
	register vector float vecLd1, vecLd2, vecLd3, vecLd4;
	register vector float vecMin, vecMax;
	
	register vector float v0, v1, v2, v3;
	
	if ( count > 4 ) {
		
		vecMin = (vector float)(FLT_MAX);
		vecMax = (vector float)(FLT_MIN);
	
		vector unsigned char permVec = vec_add( vec_lvsl( -1, srcPtr ), (vector unsigned char)(1) );
		vector float vecOld = vec_ld( 0, srcPtr );

		for ( i = 0, j = 0; i+7 < count; i += 8, j += 4) {
			// load data
			float *vecPtr = (float*)( srcPtr + (j*4) );
			vector float v0, v1, v2, v3;
						
			v0 = vecOld; 
			v1 = vec_ld( 15, vecPtr );
			v2 = vec_ld( 31, vecPtr );
			v3 = vec_ld( 47, vecPtr );
			vecOld = vec_ld( 63, vecPtr );
			
			vecLd1 = vec_perm( v0, v1, permVec );
			vecLd2 = vec_perm( v1, v2, permVec );
			vecLd3 = vec_perm( v2, v3, permVec );
			vecLd4 = vec_perm( v3, vecOld, permVec );

			// each of these vectors contains 2 elements
			// looks like | X Y X Y | X Y X Y
			v0 = vec_min( vecLd1, vecLd2 );
			v1 = vec_min( vecLd3, vecLd4 );
			v0 = vec_min( v0, v1 );
			
			v2 = vec_max( vecLd1, vecLd2 );
			v3 = vec_max( vecLd3, vecLd4 );
			v2 = vec_max( v2, v3 );
			
			// since its always X Y X Y we don't have to re-merge each time. we can wait
			// until the end
			vecMin = vec_min( v0, vecMin );
			vecMax = vec_max( v2, vecMax );
		}

		vecMin = vec_min( vecMin, vec_sld( vecMin, vecMin, 8 ) );
		vecMax = vec_max( vecMax, vec_sld( vecMax, vecMax, 8 ) );
		v0 = vec_splat( vecMin, 0 );
		v1 = vec_splat( vecMin, 1 );
		v2 = vec_splat( vecMax, 0 );
		v3 = vec_splat( vecMax, 1 );
		
		vec_ste( v0, 0, &min[0] );
		vec_ste( v1, 0, &min[1] );
		vec_ste( v2, 0, &max[0] );
		vec_ste( v3, 0, &max[1] );
	} 
	
	// cleanup
	for ( ; i < count; i++ ) {
		v = src[i];
		
		if ( v[0] < min[0] ) {
			min[0] = v[0];
		}
		if ( v[0] > max[0] ) {
			max[0] = v[0];
		}
	
		if ( v[1] < min[1] ) {
			min[1] = v[1];
		}
		if ( v[1] > max[1] ) {
			max[1] = v[1];
		}
	}
}

/*
============
idSIMD_AltiVec::MinMax
============
*/
void VPCALL idSIMD_AltiVec::MinMax( idVec3 &min, idVec3 &max, const idVec3 *src, const int count ) {
	min[0] = min[1] = min[2] = idMath::INFINITY; max[0] = max[1] = max[2] = -idMath::INFINITY;
//#define OPER(X) const idVec3 &v = src[(X)]; if ( v[0] < min[0] ) { min[0] = v[0]; } if ( v[0] > max[0] ) { max[0] = v[0]; } if ( v[1] < min[1] ) { min[1] = v[1]; } if ( v[1] > max[1] ) { max[1] = v[1]; } if ( v[2] < min[2] ) { min[2] = v[2]; } if ( v[2] > max[2] ) { max[2] = v[2]; }

	int i = 0;
	const float *srcPtr = src[0].ToFloatPtr();
	idVec3 v;

	register vector float vecLd1, vecLd2, vecLd3;
	register vector float vecMin, vecMax;
	register vector float vecSrc1, vecSrc2, vecSrc3, vecSrc4;
	register vector float vecMin1, vecMin2, vecMax1, vecMax2;
	
	if ( count >= 4 ) {
	
		vecMin = (vector float)(FLT_MAX);
		vecMax = (vector float)(FLT_MIN);
	
		vector unsigned char permVec = vec_add( vec_lvsl( -1, srcPtr), (vector unsigned char)(1) );
		vector float vecOld = vec_ld( 0, srcPtr );

		// 4 elements at a time
		for ( ; i+3 < count; i += 4 ) {
			float *vecPtr = (float*)( srcPtr + (i*3) );
			vector float v0, v1, v2;
			
			v0 = vecOld; 
			v1 = vec_ld( 15, vecPtr );
			v2 = vec_ld( 31, vecPtr );
			vecOld = vec_ld( 47, vecPtr );
			
			vecLd1 = vec_perm( v0, v1, permVec );
			vecLd2 = vec_perm( v1, v2, permVec );
			vecLd3 = vec_perm( v2, vecOld, permVec );
					
			// put each idVec3 into its own vector as X Y Z (crap)
			vecSrc1 = vecLd1;
			vecSrc2 = vec_sld( vecLd1, vecLd2, 12 ); 
			vecSrc3 = vec_sld( vecLd2, vecLd3, 8 ); 
			vecSrc4 = vec_sld( vecLd3, vecLd3, 4 );
			
			// do min and max
			vecMin1 = vec_min( vecSrc1, vecSrc2 );
			vecMin2 = vec_min( vecSrc3, vecSrc4 );
			vecMin1 = vec_min( vecMin1, vecMin2 );
			vecMin = vec_min( vecMin, vecMin1 );
			
			vecMax1 = vec_max( vecSrc1, vecSrc2 );
			vecMax2 = vec_max( vecSrc3, vecSrc4 );
			vecMax1 = vec_max( vecMax1, vecMax2 );
			vecMax = vec_max( vecMax1, vecMax );
		}
		
		// store results
		vector float v0, v1, v2, v3, v4, v5;
		v0 = vec_splat( vecMin, 0 );
		v1 = vec_splat( vecMin, 1 );
		v2 = vec_splat( vecMin, 2 );
		v3 = vec_splat( vecMax, 0 );
		v4 = vec_splat( vecMax, 1 );
		v5 = vec_splat( vecMax, 2 );	

		vec_ste( v0, 0, &min[0] );
		vec_ste( v1, 0, &min[1] );
		vec_ste( v2, 0, &min[2] );
		vec_ste( v3, 0, &max[0] );
		vec_ste( v4, 0, &max[1] );
		vec_ste( v5, 0, &max[2] );
	} 
		
	// cleanup
	for ( ; i < count; i ++ ) {
		v = src[i];
		
		if ( v[0] < min[0] ) { 
			min[0] = v[0]; 
		} 
		if ( v[0] > max[0] ) { 
			max[0] = v[0]; 
		} 
		if ( v[1] < min[1] ) { 
			min[1] = v[1]; 
		} 
		if ( v[1] > max[1] ) { 
			max[1] = v[1]; 
		} 
		if ( v[2] < min[2] ) { 
			min[2] = v[2]; 
		} 
		if ( v[2] > max[2] ) { 
			max[2] = v[2]; 
		}	
	}
}

#ifndef DRAWVERT_PADDED
/*
============
idSIMD_AltiVec::MinMax
============
*/
void VPCALL idSIMD_AltiVec::MinMax( idVec3 &min, idVec3 &max, const idDrawVert *src, const int count ) {
	
	min[0] = min[1] = min[2] = idMath::INFINITY; max[0] = max[1] = max[2] = -idMath::INFINITY;	
	idVec3 v;
	int i = 0;		
	register vector float vecMin, vecMax;
				
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector float vecMin1, vecMin2, vecMax1, vecMax2;
	
	if ( count >= 4 ) {
		vecMin = (vector float)(FLT_MAX);
		vecMax = (vector float)(FLT_MIN);
		
		vector unsigned char vertPerm1 = vec_add( vec_lvsl( -1, (float*) src[i].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vector unsigned char vertPerm2 = vec_add( vec_lvsl( -1, (float*) src[i+1].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vector unsigned char vertPerm3 = vec_add( vec_lvsl( -1, (float*) src[i+2].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vector unsigned char vertPerm4 = vec_add( vec_lvsl( -1, (float*) src[i+3].xyz.ToFloatPtr() ), (vector unsigned char)(1) );	

		for ( ; i+3 < count; i += 4) {
			const float *vertPtr = src[i].xyz.ToFloatPtr();
			const float *vertPtr2 = src[i+1].xyz.ToFloatPtr();
			const float *vertPtr3 = src[i+2].xyz.ToFloatPtr();
			const float *vertPtr4 = src[i+3].xyz.ToFloatPtr();

			v0 = vec_ld( 0, vertPtr );
			v1 = vec_ld( 11, vertPtr );
			v2 = vec_ld( 0, vertPtr2 );
			v3 = vec_ld( 11, vertPtr2 );
			v4 = vec_ld( 0, vertPtr3 );
			v5 = vec_ld( 11, vertPtr3 );
			v6 = vec_ld( 0, vertPtr4 );
			v7 = vec_ld( 11, vertPtr4 );
			
			v0 = vec_perm( v0, v1, vertPerm1 );
			v2 = vec_perm( v2, v3, vertPerm2 );
			v4 = vec_perm( v4, v5, vertPerm3 );
			v6 = vec_perm( v6, v7, vertPerm4 );
								
			vecMin1 = vec_min( v0, v2 );
			vecMin2 = vec_min( v4, v6 );
			vecMin1 = vec_min( vecMin1, vecMin2 );
			vecMin = vec_min( vecMin, vecMin1 );
			
			vecMax1 = vec_max( v0, v2 );
			vecMax2 = vec_max( v4, v6 );
			vecMax1 = vec_max( vecMax1, vecMax2 );
			vecMax = vec_max( vecMax, vecMax1 );		
		}
					
		// now we have min/max vectors in X Y Z form, store out
		v0 = vec_splat( vecMin, 0 );
		v1 = vec_splat( vecMin, 1 );
		v2 = vec_splat( vecMin, 2 );
		v3 = vec_splat( vecMax, 0 );
		v4 = vec_splat( vecMax, 1 );
		v5 = vec_splat( vecMax, 2 );	

		vec_ste( v0, 0, &min[0] );
		vec_ste( v1, 0, &min[1] );
		vec_ste( v2, 0, &min[2] );
		vec_ste( v3, 0, &max[0] );
		vec_ste( v4, 0, &max[1] );
		vec_ste( v5, 0, &max[2] );		
	} 
	
	// cleanup
	for ( ; i < count; i++ ) {
		v = src[i].xyz;
		
		if ( v[0] < min[0] ) {
			min[0] = v[0];
		}
		if ( v[0] > max[0] ) {
			max[0] = v[0];
		}
		
		if ( v[1] < min[1] ) {
			min[1] = v[1];
		}
		if ( v[1] > max[1] ) {
			max[1] = v[1];
		}
			
		if ( v[2] > max[2] ) {
			max[2] = v[2];
		}
			
		if ( v[2] < min[2] ) {
			min[2] = v[2];
		}
	}
}
#else
/*
============
idSIMD_AltiVec::MinMax
============
*/
void VPCALL idSIMD_AltiVec::MinMax( idVec3 &min, idVec3 &max, const idDrawVert *src, const int count ) {
	
	min[0] = min[1] = min[2] = idMath::INFINITY; max[0] = max[1] = max[2] = -idMath::INFINITY;	
	idVec3 v;
	int i = 0;		
	register vector float vecMin, vecMax;
				
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector float vecMin1, vecMin2, vecMax1, vecMax2;
	
	if ( count >= 4 ) {
		vecMin = (vector float)(FLT_MAX);
		vecMax = (vector float)(FLT_MIN);

		for ( ; i+3 < count; i += 4) {
			const float *vertPtr = src[i].xyz.ToFloatPtr();
			const float *vertPtr2 = src[i+1].xyz.ToFloatPtr();
			const float *vertPtr3 = src[i+2].xyz.ToFloatPtr();
			const float *vertPtr4 = src[i+3].xyz.ToFloatPtr();

			v0 = vec_ld( 0, vertPtr );
			v2 = vec_ld( 0, vertPtr2 );
			v4 = vec_ld( 0, vertPtr3 );
			v6 = vec_ld( 0, vertPtr4 );
											
			vecMin1 = vec_min( v0, v2 );
			vecMin2 = vec_min( v4, v6 );
			vecMin1 = vec_min( vecMin1, vecMin2 );
			vecMin = vec_min( vecMin, vecMin1 );
			
			vecMax1 = vec_max( v0, v2 );
			vecMax2 = vec_max( v4, v6 );
			vecMax1 = vec_max( vecMax1, vecMax2 );
			vecMax = vec_max( vecMax, vecMax1 );		
		}
					
		// now we have min/max vectors in X Y Z form, store out
		v0 = vec_splat( vecMin, 0 );
		v1 = vec_splat( vecMin, 1 );
		v2 = vec_splat( vecMin, 2 );
		v3 = vec_splat( vecMax, 0 );
		v4 = vec_splat( vecMax, 1 );
		v5 = vec_splat( vecMax, 2 );	

		vec_ste( v0, 0, &min[0] );
		vec_ste( v1, 0, &min[1] );
		vec_ste( v2, 0, &min[2] );
		vec_ste( v3, 0, &max[0] );
		vec_ste( v4, 0, &max[1] );
		vec_ste( v5, 0, &max[2] );		
	} 
	
	// cleanup
	for ( ; i < count; i++ ) {
		v = src[i].xyz;
		
		if ( v[0] < min[0] ) {
			min[0] = v[0];
		}
		if ( v[0] > max[0] ) {
			max[0] = v[0];
		}
		
		if ( v[1] < min[1] ) {
			min[1] = v[1];
		}
		if ( v[1] > max[1] ) {
			max[1] = v[1];
		}
			
		if ( v[2] > max[2] ) {
			max[2] = v[2];
		}
			
		if ( v[2] < min[2] ) {
			min[2] = v[2];
		}
	}
}

#endif /* DRAWVERT_PADDED */

#ifndef DRAWVERT_PADDED
/*
============
idSIMD_AltiVec::MinMax
============
*/
void VPCALL idSIMD_AltiVec::MinMax( idVec3 &min, idVec3 &max, const idDrawVert *src, const int *indexes, const int count ) {
	min[0] = min[1] = min[2] = idMath::INFINITY; max[0] = max[1] = max[2] = -idMath::INFINITY;
		
	idVec3 v;
	int i = 0;
		
	register vector float vecMin, vecMax;
				
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector float vecMin1, vecMin2, vecMax1, vecMax2;
	
	if ( count >= 4 ) {
		
		vecMin = (vector float)(FLT_MAX);
		vecMax = (vector float)(FLT_MIN);
		
		vector unsigned char vertPerm1;
		vector unsigned char vertPerm2;
		vector unsigned char vertPerm3;
		vector unsigned char vertPerm4;	

		for ( ; i+3 < count; i += 4) {
			const float *vertPtr = src[indexes[i]].xyz.ToFloatPtr();
			const float *vertPtr2 = src[indexes[i+1]].xyz.ToFloatPtr();
			const float *vertPtr3 = src[indexes[i+2]].xyz.ToFloatPtr();
			const float *vertPtr4 = src[indexes[i+3]].xyz.ToFloatPtr();

			vertPerm1 = vec_add( vec_lvsl( -1, vertPtr ), (vector unsigned char)(1) );
			vertPerm2 = vec_add( vec_lvsl( -1, vertPtr2 ), (vector unsigned char)(1) );
			vertPerm3 = vec_add( vec_lvsl( -1, vertPtr3 ), (vector unsigned char)(1) );
			vertPerm4 = vec_add( vec_lvsl( -1, vertPtr4 ), (vector unsigned char)(1) );
			
			v0 = vec_ld( 0, vertPtr );
			v1 = vec_ld( 15, vertPtr );
			v2 = vec_ld( 0, vertPtr2 );
			v3 = vec_ld( 15, vertPtr2 );
			v4 = vec_ld( 0, vertPtr3 );
			v5 = vec_ld( 15, vertPtr3 );
			v6 = vec_ld( 0, vertPtr4 );
			v7 = vec_ld( 15, vertPtr4 );
			
			v0 = vec_perm( v0, v1, vertPerm1 );
			v2 = vec_perm( v2, v3, vertPerm2 );
			v4 = vec_perm( v4, v5, vertPerm3 );
			v6 = vec_perm( v6, v7, vertPerm4 );
								
			vecMin1 = vec_min( v0, v2 );
			vecMin2 = vec_min( v4, v6 );
			vecMin1 = vec_min( vecMin1, vecMin2 );
			vecMin = vec_min( vecMin, vecMin1 );
			
			vecMax1 = vec_max( v0, v2 );
			vecMax2 = vec_max( v4, v6 );
			vecMax1 = vec_max( vecMax1, vecMax2 );
			vecMax = vec_max( vecMax, vecMax1 );		
		}
					
		// now we have min/max vectors in X Y Z form, store out
		v0 = vec_splat( vecMin, 0 );
		v1 = vec_splat( vecMin, 1 );
		v2 = vec_splat( vecMin, 2 );
		v3 = vec_splat( vecMax, 0 );
		v4 = vec_splat( vecMax, 1 );
		v5 = vec_splat( vecMax, 2 );	

		vec_ste( v0, 0, &min[0] );
		vec_ste( v1, 0, &min[1] );
		vec_ste( v2, 0, &min[2] );
		vec_ste( v3, 0, &max[0] );
		vec_ste( v4, 0, &max[1] );
		vec_ste( v5, 0, &max[2] );
	} 
	
	// cleanup
	for ( ; i < count; i++ ) {
		v = src[indexes[i]].xyz;
		
		if ( v[0] < min[0] ) {
			min[0] = v[0];
		}
		if ( v[0] > max[0] ) {
			max[0] = v[0];
		}
		
		if ( v[1] < min[1] ) {
			min[1] = v[1];
		}
		if ( v[1] > max[1] ) {
			max[1] = v[1];
		}
			
		if ( v[2] > max[2] ) {
			max[2] = v[2];
		}
			
		if ( v[2] < min[2] ) {
			min[2] = v[2];
		}
	}
}
#else
/*
============
idSIMD_AltiVec::MinMax
============
*/
void VPCALL idSIMD_AltiVec::MinMax( idVec3 &min, idVec3 &max, const idDrawVert *src, const int *indexes, const int count ) {
	min[0] = min[1] = min[2] = idMath::INFINITY; max[0] = max[1] = max[2] = -idMath::INFINITY;
		
	idVec3 v;
	int i = 0;
		
	register vector float vecMin, vecMax;
				
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector float vecMin1, vecMin2, vecMax1, vecMax2;
	
	if ( count >= 4 ) {
		
		vecMin = (vector float)(FLT_MAX);
		vecMax = (vector float)(FLT_MIN);
		
		vector unsigned char vertPerm1;
		vector unsigned char vertPerm2;
		vector unsigned char vertPerm3;
		vector unsigned char vertPerm4;	

		for ( ; i+3 < count; i += 4) {
			const float *vertPtr = src[indexes[i]].xyz.ToFloatPtr();
			const float *vertPtr2 = src[indexes[i+1]].xyz.ToFloatPtr();
			const float *vertPtr3 = src[indexes[i+2]].xyz.ToFloatPtr();
			const float *vertPtr4 = src[indexes[i+3]].xyz.ToFloatPtr();
			
			v0 = vec_ld( 0, vertPtr );
			v2 = vec_ld( 0, vertPtr2 );
			v4 = vec_ld( 0, vertPtr3 );
			v6 = vec_ld( 0, vertPtr4 );
								
			vecMin1 = vec_min( v0, v2 );
			vecMin2 = vec_min( v4, v6 );
			vecMin1 = vec_min( vecMin1, vecMin2 );
			vecMin = vec_min( vecMin, vecMin1 );
			
			vecMax1 = vec_max( v0, v2 );
			vecMax2 = vec_max( v4, v6 );
			vecMax1 = vec_max( vecMax1, vecMax2 );
			vecMax = vec_max( vecMax, vecMax1 );		
		}
					
		// now we have min/max vectors in X Y Z form, store out
		v0 = vec_splat( vecMin, 0 );
		v1 = vec_splat( vecMin, 1 );
		v2 = vec_splat( vecMin, 2 );
		v3 = vec_splat( vecMax, 0 );
		v4 = vec_splat( vecMax, 1 );
		v5 = vec_splat( vecMax, 2 );	

		vec_ste( v0, 0, &min[0] );
		vec_ste( v1, 0, &min[1] );
		vec_ste( v2, 0, &min[2] );
		vec_ste( v3, 0, &max[0] );
		vec_ste( v4, 0, &max[1] );
		vec_ste( v5, 0, &max[2] );
	} 
	
	// cleanup
	for ( ; i < count; i++ ) {
		v = src[indexes[i]].xyz;
		
		if ( v[0] < min[0] ) {
			min[0] = v[0];
		}
		if ( v[0] > max[0] ) {
			max[0] = v[0];
		}
		
		if ( v[1] < min[1] ) {
			min[1] = v[1];
		}
		if ( v[1] > max[1] ) {
			max[1] = v[1];
		}
			
		if ( v[2] > max[2] ) {
			max[2] = v[2];
		}
			
		if ( v[2] < min[2] ) {
			min[2] = v[2];
		}
	}
}


#endif /* DRAWVERT_PADDED */

#endif /* ENABLE_MINMAX */

#ifdef ENABLE_CLAMP

/*
============
idSIMD_AltiVec::Clamp
============
*/
void VPCALL idSIMD_AltiVec::Clamp( float *dst, const float *src, const float min, const float max, const int count ) {
//#define OPER(X) dst[(X)] = src[(X)] < min ? min : src[(X)] > max ? max : src[(X)];
    register vector float v0, v1, v2, v3, v4, v5;
    register vector unsigned char permVec;
	register vector float v0_low, v0_hi, v1_low, v1_hi;
	vector unsigned char oneVector = (vector unsigned char)(1);
    register vector float minVec, maxVec;
    int i = 0;
   
    //handle unaligned at start
    for ( ;  NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = src[i] < min ? min : src[i] > max ? max : src[i];
    }
    
    //splat min/max into a vector
    minVec = loadSplatUnalignedScalar( &min );
	maxVec = loadSplatUnalignedScalar( &max );
	
	//calculate permute and do first load
	permVec = vec_add( vec_lvsl( -1, (int*) &src[i] ), oneVector );
 	v1_hi = vec_ld( 0, &src[i] ); 

    
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v1_hi;
		v0_hi = vec_ld( 15, &src[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &src[i] );

		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );
	
		//apply minimum
		v2 = vec_max( v0, minVec );
		v3 = vec_max( v1, minVec );
	
		//apply maximum
		v4 = vec_min( v2, maxVec );
		v5 = vec_min( v3, maxVec );
    
		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
    
    //handle cleanup
    for ( ; i < count ; i++ ) {
		dst[i] = src[i] < min ? min : src[i] > max ? max : src[i];
    }
}

/*
============
idSIMD_AltiVec::ClampMin
============
*/
void VPCALL idSIMD_AltiVec::ClampMin( float *dst, const float *src, const float min, const int count ) {
//#define OPER(X) dst[(X)] = src[(X)] < min ? min : src[(X)];
    register vector float v0, v1, v2, v3;
    register vector unsigned char permVec;
	register vector float v0_low, v0_hi, v1_low, v1_hi;
    register vector float constVec;
	vector unsigned char oneVector = (vector unsigned char)(1);
    int i = 0;

    //handle unaligned at start
	for ( ;  NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = src[i] < min ? min : src[i];
    }
    
    //splat constant into a vector
    constVec = loadSplatUnalignedScalar( &min );
	
	//calculate permute and do first load
	permVec = vec_add( vec_lvsl( -1, (int*) &src[i] ), oneVector );
 	v1_hi = vec_ld( 0, &src[i] ); 
    
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v1_hi;
		v0_hi = vec_ld( 15, &src[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &src[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );

		v2 = vec_max( v0, constVec );
		v3 = vec_max( v1, constVec );
		
		ALIGNED_STORE2( &dst[i], v2, v3 );
    }
    
    //handle cleanup
     for ( ; i < count ; i++ ) {
		dst[i] = src[i] < min ? min : src[i];
    }
 }

/*
============
idSIMD_AltiVec::ClampMax
============
*/
void VPCALL idSIMD_AltiVec::ClampMax( float *dst, const float *src, const float max, const int count ) {
//#define OPER(X) dst[(X)] = src[(X)] > max ? max : src[(X)];
    register vector float v0, v1, v2, v3;
    register vector unsigned char permVec;
    register vector float constVec;
	register vector float v0_low, v0_hi, v1_low, v1_hi;
	vector unsigned char oneVector = (vector unsigned char)(1);
    int i = 0;
    
	//handle unaligned at start
    for ( ;  NOT_16BYTE_ALIGNED( dst[i] ) && ( i < count ); i++ ) {
		dst[i] = src[i] < max ? max : src[i];
    }
    
    //splat constant into a vector
    constVec = loadSplatUnalignedScalar( &max );
	
	//calculate permute and do first load
	permVec = vec_add( vec_lvsl( -1, (int*) &src[i] ), oneVector );
 	v1_hi = vec_ld( 0, &src[i] ); 
	    
    //vectorize!
    for ( ; i+7 < count; i += 8 ) {
		//load source
		v0_low = v1_hi;
		v0_hi = vec_ld( 15, &src[i] );
		v1_low = v0_hi;
		v1_hi = vec_ld( 31, &src[i] );
		
		v0 = vec_perm( v0_low, v0_hi, permVec );
		v1 = vec_perm( v1_low, v1_hi, permVec );		
		v2 = vec_min( v0, constVec );
		v3 = vec_min( v1, constVec );
		
		ALIGNED_STORE2( &dst[i], v2, v3 );
	}
    
    //handle cleanup
    for ( ; i < count ; i++ ) {
		dst[i] = src[i] < max ? max : src[i];
    }
}

#endif /* ENABLE_CLAMP */

#ifdef ENABLE_16ROUTINES 

/*
============
idSIMD_AltiVec::Zero16
============
*/
void VPCALL idSIMD_AltiVec::Zero16( float *dst, const int count ) {
	memset( dst, 0, count * sizeof( float ) );
}

/*
============
idSIMD_AltiVec::Negate16

	Assumptions:
		dst is aligned
============
*/
void VPCALL idSIMD_AltiVec::Negate16( float *dst, const int count ) {
//#define OPER(X) ptr[(X)] ^= ( 1 << 31 )		// IEEE 32 bits float sign bit 
	
	// dst is aligned
	assert( IS_16BYTE_ALIGNED( dst[0] ) );

	// round count up to next 4 if needbe
	int count2 = ( count + 3 ) & ~3;
      
	int i = 0;	
	vector float v0, v1, v2, v3;
		
    //know its 16-byte aligned
	for ( ; i + 7 < count2; i += 8 ) {
		v0 = vec_ld( 0, &dst[i] );
		v1 = vec_ld( 16, &dst[i] );
		
		v2 = vec_sub( (vector float)(0), v0 );
		v3 = vec_sub( (vector float)(0), v1 );
			
		ALIGNED_STORE2( &dst[i], v2, v3 );
	}
	
	for ( ; i < count2; i += 4 ) {
		v0 = vec_ld( 0, &dst[i] );
		v1 = vec_sub( (vector float)(0), v0 );
		vec_st( v1, 0, &dst[i] );
	}
}

/*
============
idSIMD_AltiVec::Copy16
============
*/
void VPCALL idSIMD_AltiVec::Copy16( float *dst, const float *src, const int count ) {
//#define OPER(X) dst[(X)] = src[(X)]
	memcpy( dst, src, sizeof(float) * count );
}

/*
============
idSIMD_AltiVec::Add16
	
	Assumptions:
		Assumes dst, src1, src2 all start at aligned address
============
*/
void VPCALL idSIMD_AltiVec::Add16( float *dst, const float *src1, const float *src2, const int count ) {
//#define OPER(X) dst[(X)] = src1[(X)] + src2[(X)]

	// dst is aligned
	assert( IS_16BYTE_ALIGNED( dst[0] ) );
	// src1 is aligned
	assert( IS_16BYTE_ALIGNED( src1[0] ) );
	// src2 is aligned
	assert( IS_16BYTE_ALIGNED( src2[0] ) );	
	
	// round count up to next 4 if needbe
	int count2 = ( count + 3 ) & ~3;
	
    register vector float v0, v1, v2, v3, v4, v5;
    int i = 0;
	
    //know all data is 16-byte aligned, so vectorize!
    for ( ; i+7 < count2; i += 8 ) {
		//load sources
		v0 = vec_ld( 0, &src1[i] );
		v1 = vec_ld( 16, &src1[i] );
		v2 = vec_ld( 0, &src2[i] );
		v3 = vec_ld( 16, &src2[i] );
		v4 = vec_add( v0, v2 );
		v5 = vec_add( v1, v3 );
		
		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
	
	for ( ; i < count2; i += 4 ) {
		v0 = vec_ld( 0, &src1[i] );
		v1 = vec_ld( 0, &src2[i] );
		v2 = vec_add( v0, v1 );
		vec_st( v2, 0, &dst[i] );
	}
}

/*
============
idSIMD_AltiVec::Sub16

	Assumptions:
		Assumes that dst, src1, and src2 all start at aligned address 
============
*/
void VPCALL idSIMD_AltiVec::Sub16( float *dst, const float *src1, const float *src2, const int count ) {
//#define OPER(X) dst[(X)] = src1[(X)] - src2[(X)]
	// dst is aligned
	assert( IS_16BYTE_ALIGNED( dst[0] ) );
	// src1 is aligned
	assert( IS_16BYTE_ALIGNED( src1[0] ) );
	// src2 is aligned
	assert( IS_16BYTE_ALIGNED( src2[0] ) );	
	
	// round count up to next 4 if needbe
	int count2 = ( count + 3 ) & ~3;

    register vector float v0, v1, v2, v3, v4, v5;
    int i = 0;
    
    //know data is aligned, so vectorize!
    for ( ; i+7 < count2; i += 8 ) {
		//load sources
		v0 = vec_ld( 0, &src1[i] );
		v1 = vec_ld( 16, &src1[i] );
		v2 = vec_ld( 0, &src2[i] );
		v3 = vec_ld( 16, &src2[i] );
		v4 = vec_sub( v0, v2 );
		v5 = vec_sub( v1, v3 );
		
		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
	
	for ( ; i < count2; i += 4 ) {
		v0 = vec_ld( 0, &src1[i] );
		v1 = vec_ld( 0, &src2[i] );
		v2 = vec_sub( v0, v1 );
		vec_st( v2, 0, &dst[i] );
	}
}

/*
============
idSIMD_AltiVec::Mul16

	Assumptions:
		Assumes that dst and src1 start at aligned address
============
*/
void VPCALL idSIMD_AltiVec::Mul16( float *dst, const float *src1, const float constant, const int count ) {
//#define OPER(X) dst[(X)] = src1[(X)] * constant
  
  	// dst is aligned
	assert( IS_16BYTE_ALIGNED( dst[0] ) );
	// src1 is aligned
	assert( IS_16BYTE_ALIGNED( src1[0] ) );
	
	// round count up to next 4 if needbe
	int count2 = ( count + 3 ) & ~3;
	
    register vector float v0, v1, v2, v3;
    register vector float constVec;
    register vector float zeroVector = (vector float)(0.0);
    int i = 0;
    
    //splat constant into a vector
	constVec = loadSplatUnalignedScalar( &constant );
	
    //know data is aligned, so vectorize!
    for ( ; i+7 < count2; i += 8 ) {
		//load source
		v0 = vec_ld( 0, &src1[i] );
		v1 = vec_ld( 16, &src1[i] );
		v2 = vec_madd( constVec, v0, zeroVector );
		v3 = vec_madd( constVec, v1, zeroVector );
		ALIGNED_STORE2( &dst[i], v2, v3 );
    }
	
	for ( ; i < count2; i += 4 ) {
		v0 = vec_ld( 0, &src1[i] );
		v1 = vec_madd( constVec, v0, zeroVector );
		vec_st( v1, 0, &dst[i] );
	}
}

/*
============
idSIMD_AltiVec::AddAssign16

	Assumptions:
		Assumes that dst and src start at aligned address
============
*/
void VPCALL idSIMD_AltiVec::AddAssign16( float *dst, const float *src, const int count ) {
//#define OPER(X) dst[(X)] += src[(X)]
    
	// dst is aligned
	assert( IS_16BYTE_ALIGNED( dst[0] ) );
	// src is aligned
	assert( IS_16BYTE_ALIGNED( src[0] ) );	
	
	// round count up to next 4 if needbe
	int count2 = ( count + 3 ) & ~3;
	
	register vector float v0, v1, v2, v3, v4, v5;
    int i = 0;
  
    //vectorize!
    for ( ; i+7 < count2; i += 8 ) {
		v0 = vec_ld( 0, &src[i] );
		v1 = vec_ld( 16, &src[i] );
		v2 = vec_ld( 0, &dst[i] );
		v3 = vec_ld( 16, &dst[i] );
		v4 = vec_add( v0, v2 );
		v5 = vec_add( v1, v3 );
		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
	
	for ( ; i < count2; i += 4 ) {
		v0 = vec_ld( 0, &src[i] );
		v1 = vec_ld( 0, &dst[i] );
		v2 = vec_add( v0, v1 );
		vec_st( v2, 0, &dst[i] );
	}
}

/*
============
idSIMD_AltiVec::SubAssign16

	Assumptions:
		Assumes that dst and src start at aligned address
============
*/
void VPCALL idSIMD_AltiVec::SubAssign16( float *dst, const float *src, const int count ) {
//#define OPER(X) dst[(X)] -= src[(X)]
    register vector float v0, v1, v2, v3, v4, v5;
    int i=0;
	
	// dst is aligned
	assert( IS_16BYTE_ALIGNED( dst[0] ) );
	// src is aligned
	assert( IS_16BYTE_ALIGNED( src[0] ) );		
	// round count up to next 4 if needbe
	int count2 = ( count + 3 ) & ~3;
    
    //vectorize!
    for ( ; i+7 < count2; i += 8 ) {
		v0 = vec_ld( 0, &src[i] );
		v1 = vec_ld( 16, &src[i] );
		v2 = vec_ld( 0, &dst[i] );
		v3 = vec_ld( 16, &dst[i] );
		v4 = vec_sub( v2, v0 );
		v5 = vec_sub( v3, v1 );
		ALIGNED_STORE2( &dst[i], v4, v5 );
    }
	
	for ( ; i < count2; i += 4 ) {
		v0 = vec_ld( 0, &src[i] );
		v1 = vec_ld( 0, &dst[i] );
		v2 = vec_sub( v1, v0 );
		vec_st( v2, 0, &dst[i] );
	}
}

/*
============
idSIMD_AltiVec::MulAssign16

	Assumptions:
		Assumes that dst starts at aligned address and count is multiple of 4
============
*/
void VPCALL idSIMD_AltiVec::MulAssign16( float *dst, const float constant, const int count ) {
//#define OPER(X) dst[(X)] *= constant

	// dst is aligned
	assert( IS_16BYTE_ALIGNED( dst[0] ) );
	// round count up to next 4 if needbe
	int count2 = ( count + 3 ) & ~3;
	
    register vector float v0, v1, v2, v3;
    register vector float constVec;
    int i = 0;
    register vector float zeroVector = (vector float)(0.0);
    
    //splat constant into a vector
	constVec = loadSplatUnalignedScalar( &constant );
    
    //vectorize!
    for ( ; i+7 < count2; i += 8 ) {
		v0 = vec_ld( 0, &dst[i] );
		v1 = vec_ld( 16, &dst[i] );
		v2 = vec_madd( v0, constVec, zeroVector );
		v3 = vec_madd( v1, constVec, zeroVector );
		ALIGNED_STORE2( &dst[i], v2, v3 );
    }
	
	for ( ; i < count2; i += 4 ) {
		v0 = vec_ld( 0, &dst[i] );
		v1 = vec_madd( v0, constVec, zeroVector );
		vec_st( v1, 0, &dst[i] );
	}
}

#endif /* ENABLE_16ROUTINES */

#ifdef ENABLE_LOWER_TRIANGULAR

/*
============
idSIMD_AltiVec::MatX_LowerTriangularSolve

  solves x in L * x = b for the first n rows of L
  if skip > 0 the first skip elements of x are assumed to be valid already
  L has to be a lower triangular matrix with (implicit) ones on the diagonal
  x == b is allowed
============
*/

void VPCALL idSIMD_AltiVec::MatX_LowerTriangularSolve( const idMatX &L, float *x, const float *b, const int n, int skip ) {
	
	int i, j;
	const float *lptr;
	const float *lptr2;
	const float *lptr3;
	const float *lptr4;
	float sum;
	float sum2;
	float sum3;
	float sum4;
	float tempSum;
	float tempSum2;
	float tempSum3;
	float tempSum4;
	vector float vecSum1 = (vector float)(0.0);
	vector float vecSum2 = (vector float)(0.0);
	vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
	vector float zeroVector = (vector float)(0.0);
	vector float vecSum3, vecSum4, vecSum5, vecSum6, vecSum7, vecSum8;
	
	vector unsigned char vecPermX = vec_add( vec_lvsl( -1, &x[0] ), (vector unsigned char)(1) );
			
	// unrolled this loop a bit
	for ( i = skip; i+3 < n; i+=4 ) {  
		sum = b[i];
		sum2 = b[i+1];
		sum3 = b[i+2];
		sum4 = b[i+3];
		
		vecSum1 = zeroVector;
		vecSum2 = zeroVector;
		vecSum3 = vecSum4 = vecSum5 = vecSum6 = vecSum7 = vecSum8 = zeroVector;
		lptr = L[i];
		lptr2 = L[i+1];
		lptr3 = L[i+2];
		lptr4 = L[i+3];
		
		vector unsigned char vecPermLptr1 = vec_add( vec_lvsl( -1, lptr ), (vector unsigned char)(1) );
		vector unsigned char vecPermLptr2 = vec_add( vec_lvsl( -1, lptr2 ), (vector unsigned char)(1) );
		vector unsigned char vecPermLptr3 = vec_add( vec_lvsl( -1, lptr3 ), (vector unsigned char)(1) );
		vector unsigned char vecPermLptr4 = vec_add( vec_lvsl( -1, lptr4 ), (vector unsigned char)(1) );
		
		for ( j = 0 ; j+7 < i; j+=8 ) {
		
			v0 = vec_ld( 0, &x[j] );
			v1 = vec_ld( 15, &x[j] );
			vector float vecExtraX = vec_ld( 31, &x[j] );
			v0 = vec_perm( v0, v1, vecPermX );
			v1 = vec_perm( v1, vecExtraX, vecPermX );
			
			v2 = vec_ld( 0, lptr + j );
			v3 = vec_ld( 15, lptr + j );
			vector float vecExtra1 = vec_ld( 31, lptr + j );
			v2 = vec_perm( v2, v3, vecPermLptr1 );
			v3 = vec_perm( v3, vecExtra1, vecPermLptr1 );
			
			v4 = vec_ld( 0, lptr2 + j );
			v5 = vec_ld( 15, lptr2 + j );
			vector float vecExtra2 = vec_ld( 31, lptr2 + j );
			v4 = vec_perm( v4, v5, vecPermLptr2 );
			v5 = vec_perm( v5, vecExtra2, vecPermLptr2 );
			
			v6 = vec_ld( 0, lptr3 + j );
			v7 = vec_ld( 15, lptr3 + j );
			vector float vecExtra3 = vec_ld( 31, lptr3 + j );
			v6 = vec_perm( v6, v7, vecPermLptr3 );
			v7 = vec_perm( v7, vecExtra3, vecPermLptr3 );
			
			v8 = vec_ld( 0, lptr4 + j );
			v9 = vec_ld( 15, lptr4 + j );
			vector float vecExtra4 = vec_ld( 31, lptr4 + j );
			v8 = vec_perm( v8, v9, vecPermLptr4 );
			v9 = vec_perm( v9, vecExtra4, vecPermLptr4 );
	
			vecSum1 = vec_madd( v2, v0, vecSum1 );
			vecSum2 = vec_madd( v3, v1, vecSum2 );
			
			vecSum3 = vec_madd( v4, v0, vecSum3 );
			vecSum4 = vec_madd( v5, v1, vecSum4 );
			
			vecSum5 = vec_madd( v6, v0, vecSum5 );
			vecSum6 = vec_madd( v7, v1, vecSum6 );
			
			vecSum7 = vec_madd( v8, v0, vecSum7 );
			vecSum8 = vec_madd( v9, v1, vecSum8 );
		}
		
		// if we ran the unrolled code, we need to sum accross the vectors
		// to find out how much to subtract from sum
		if ( j > 0 ) {
			vecSum1 = vec_add( vecSum1, vecSum2 );  
			vecSum3 = vec_add( vecSum3, vecSum4 );
			vecSum5 = vec_add( vecSum5, vecSum6 );
			vecSum7 = vec_add( vecSum7, vecSum8 );
			//sum accross the vectors			
			vecSum1 = vec_add( vecSum1, vec_sld( vecSum1, vecSum1, 8 ) );
			vecSum1 = vec_add( vecSum1, vec_sld( vecSum1, vecSum1, 4 ) );
			
			vecSum3 = vec_add( vecSum3, vec_sld( vecSum3, vecSum3, 8 ) );
			vecSum3 = vec_add( vecSum3, vec_sld( vecSum3, vecSum3, 4 ) );
			
			vecSum5 = vec_add( vecSum5, vec_sld( vecSum5, vecSum5, 8 ) );
			vecSum5 = vec_add( vecSum5, vec_sld( vecSum5, vecSum5, 4 ) );
			
			vecSum7 = vec_add( vecSum7, vec_sld( vecSum7, vecSum7, 8 ) );
			vecSum7 = vec_add( vecSum7, vec_sld( vecSum7, vecSum7, 4 ) );

			//move the result to the FPU
			vec_ste( vec_splat( vecSum1, 0 ), 0, &tempSum );
			vec_ste( vec_splat( vecSum3, 0 ), 0, &tempSum2 );
			vec_ste( vec_splat( vecSum5, 0 ), 0, &tempSum3 );
			vec_ste( vec_splat( vecSum7, 0 ), 0, &tempSum4 );
			
			sum -= tempSum;
			sum2 -= tempSum2;
			sum3 -= tempSum3;
			sum4 -= tempSum4;			
		}
	
		//cleanup
		for (  ; j < i; j++ ) {
			sum -= lptr[j] * x[j];
			sum2 -= lptr2[j] * x[j];
			sum3 -= lptr3[j] * x[j];
			sum4 -= lptr4[j] * x[j];
		}

		// store the 4 results at a time
		sum2 -=  ( lptr2[i] * sum );	
		sum3 = sum3 - ( lptr3[i+1] * sum2 ) - ( lptr3[i] * sum );
		sum4 = sum4 - ( lptr4[i+2] * sum3 ) -  ( lptr4[i+1] * sum2 ) - ( lptr4[i] * sum );
		
		x[i] = sum;
		x[i+1] = sum2;
		x[i+2] = sum3; 
		x[i+3] = sum4;
	}
	
	// cleanup
	for ( ; i < n; i++ ) {  
		sum = b[i];
		vecSum1 = zeroVector;
		vecSum2 = zeroVector;
		lptr = L[i];
		vector unsigned char vecPermLptr = vec_add( vec_lvsl( -1, lptr ), (vector unsigned char)(1) );
		
		for ( j = 0 ; j+7 < i; j+=8 ) {
		
			v0 = vec_ld( 0, &x[j] );
			v2 = vec_ld( 15, &x[j] );
			vector float vecExtraX = vec_ld( 31, &x[j] );
			v0 = vec_perm( v0, v2, vecPermX );
			v2 = vec_perm( v2, vecExtraX, vecPermX );
			
			v1 = vec_ld( 0, lptr + j );
			v3 = vec_ld( 15, lptr + j );
			vector float vecExtra = vec_ld( 31, lptr + j );
			v1 = vec_perm( v1, v3, vecPermLptr );
			v3 = vec_perm( v3, vecExtra, vecPermLptr );
			
			vecSum1 = vec_madd( v1, v0, vecSum1 );
			vecSum2 = vec_madd( v3, v2, vecSum2 );
		}
		
		// if we ran the unrolled code, we need to sum accross the vectors
		// to find out how much to subtract from sum
		if ( j > 0 ) {
			//sum accross the vectors
			vecSum1 = vec_add( vecSum1, vecSum2 );  
			vecSum1 = vec_add( vecSum1, vec_sld( vecSum1, vecSum1, 8 ) );
			vecSum1 = vec_add( vecSum1, vec_sld( vecSum1, vecSum1, 4 ) );

			//move the result to the FPU
			vec_ste( vec_splat( vecSum1, 0 ), 0, &tempSum );
			sum -= tempSum;
		}
	
		//cleanup
		for (  ; j < i; j++ ) {
			sum -= lptr[j] * x[j];
		}
		x[i] = sum;
	}
}

/*
============
idSIMD_AltiVec::MatX_LowerTriangularSolveTranspose

  solves x in L.Transpose() * x = b for the first n rows of L
  L has to be a lower triangular matrix with (implicit) ones on the diagonal
  x == b is allowed
============
*/
void VPCALL idSIMD_AltiVec::MatX_LowerTriangularSolveTranspose( const idMatX &L, float *x, const float *b, const int n ) {

	int nc;
	const float *lptr;

	lptr = L.ToFloatPtr();
	nc = L.GetNumColumns();

	float x0, x1, x2, x3, x4, x5, x6;
	// unrolled cases for n < 8
	if ( n < 8 ) {
		switch( n ) {
			// using local variables to avoid aliasing issues
			case 0:
				return;
			case 1:
				x[0] = b[0];
				return;
			case 2:
				x1 = b[1];
				x0 = b[0] - lptr[1*nc+0] * x1;
								
				x[1] = x1;
				x[0] = x0;
				return;
			case 3:
				x2 = b[2];
				x1 = b[1] - lptr[2*nc+1] * x2;
				x0 = b[0] - lptr[2*nc+0] * x2 - lptr[1*nc+0] * x1;
				
				x[2] = x2;
				x[1] = x1;
				x[0] = x0;
				return;
			case 4:
				x3 = b[3];
				x2 = b[2] - lptr[3*nc+2] * x3;
				x1 = b[1] - lptr[3*nc+1] * x3 - lptr[2*nc+1] * x2;
				x0 = b[0] - lptr[3*nc+0] * x3 - lptr[2*nc+0] * x2 - lptr[1*nc+0] * x1;
				
				x[3] = x3;
				x[2] = x2;
				x[1] = x1;
				x[0] = x0;
				
				return;
			case 5:
				x4 = b[4];
				x3 = b[3] - lptr[4*nc+3] * x4;
				x2 = b[2] - lptr[4*nc+2] * x4 - lptr[3*nc+2] * x3;
				x1 = b[1] - lptr[4*nc+1] * x4 - lptr[3*nc+1] * x3 - lptr[2*nc+1] * x2;
				x0 = b[0] - lptr[4*nc+0] * x4 - lptr[3*nc+0] * x3 - lptr[2*nc+0] * x2 - lptr[1*nc+0] * x1;				
				
				x[4] = x4;
				x[3] = x3;
				x[2] = x2;
				x[1] = x1;
				x[0] = x0;
				return;
			case 6:
				x5 = b[5];
				x4 = b[4] - lptr[5*nc+4] * x5;
				x3 = b[3] - lptr[5*nc+3] * x5 - lptr[4*nc+3] * x4;
				x2 = b[2] - lptr[5*nc+2] * x5 - lptr[4*nc+2] * x4 - lptr[3*nc+2] * x3;
				x1 = b[1] - lptr[5*nc+1] * x5 - lptr[4*nc+1] * x4 - lptr[3*nc+1] * x3 - lptr[2*nc+1] * x2;
				x0 = b[0] - lptr[5*nc+0] * x5 - lptr[4*nc+0] * x4 - lptr[3*nc+0] * x3 - lptr[2*nc+0] * x2 - lptr[1*nc+0] * x1;
				
				x[5] = x5;
				x[4] = x4;
				x[3] = x3;
				x[2] = x2;
				x[1] = x1;
				x[0] = x0;
				
				return;
			case 7:
				x6 = b[6];
				x5 = b[5] - lptr[6*nc+5] * x6;
				x4 = b[4] - lptr[6*nc+4] * x6 - lptr[5*nc+4] * x5;
				x3 = b[3] - lptr[6*nc+3] * x6 - lptr[5*nc+3] * x5 - lptr[4*nc+3] * x4;
				x2 = b[2] - lptr[6*nc+2] * x6 - lptr[5*nc+2] * x5 - lptr[4*nc+2] * x4 - lptr[3*nc+2] * x3;
				x1 = b[1] - lptr[6*nc+1] * x6 - lptr[5*nc+1] * x5 - lptr[4*nc+1] * x4 - lptr[3*nc+1] * x3 - lptr[2*nc+1] * x2;
				x0 = b[0] - lptr[6*nc+0] * x6 - lptr[5*nc+0] * x5 - lptr[4*nc+0] * x4 - lptr[3*nc+0] * x3 - lptr[2*nc+0] * x2 - lptr[1*nc+0] * x1;
			
				x[6] = x6;
				x[5] = x5;
				x[4] = x4;
				x[3] = x3;
				x[2] = x2;
				x[1] = x1;
				x[0] = x0;
				return;
		}
		return;
	}

	int i, j;
	register float s0, s1, s2, s3;
	float *xptr;

	lptr = L.ToFloatPtr() + n * nc + n - 4;
	xptr = x + n;

	// process 4 rows at a time
	for ( i = n; i >= 4; i -= 4 ) {
		s0 = b[i-4];
		s1 = b[i-3];
		s2 = b[i-2];
		s3 = b[i-1];
		// process 4x4 blocks
		for ( j = 0; j < n-i; j += 4 ) {
			s0 -= lptr[(j+0)*nc+0] * xptr[j+0];
			s1 -= lptr[(j+0)*nc+1] * xptr[j+0];
			s2 -= lptr[(j+0)*nc+2] * xptr[j+0];
			s3 -= lptr[(j+0)*nc+3] * xptr[j+0];
			s0 -= lptr[(j+1)*nc+0] * xptr[j+1];
			s1 -= lptr[(j+1)*nc+1] * xptr[j+1];
			s2 -= lptr[(j+1)*nc+2] * xptr[j+1];
			s3 -= lptr[(j+1)*nc+3] * xptr[j+1];
			s0 -= lptr[(j+2)*nc+0] * xptr[j+2];
			s1 -= lptr[(j+2)*nc+1] * xptr[j+2];
			s2 -= lptr[(j+2)*nc+2] * xptr[j+2];
			s3 -= lptr[(j+2)*nc+3] * xptr[j+2];
			s0 -= lptr[(j+3)*nc+0] * xptr[j+3];
			s1 -= lptr[(j+3)*nc+1] * xptr[j+3];
			s2 -= lptr[(j+3)*nc+2] * xptr[j+3];
			s3 -= lptr[(j+3)*nc+3] * xptr[j+3];
		}
		// process left over of the 4 rows
		s0 -= lptr[0-1*nc] * s3;
		s1 -= lptr[1-1*nc] * s3;
		s2 -= lptr[2-1*nc] * s3;
		s0 -= lptr[0-2*nc] * s2;
		s1 -= lptr[1-2*nc] * s2;
		s0 -= lptr[0-3*nc] * s1;
		// store result
		xptr[-4] = s0;
		xptr[-3] = s1;
		xptr[-2] = s2;
		xptr[-1] = s3;
		// update pointers for next four rows
		lptr -= 4 + 4 * nc;
		xptr -= 4;
	}
	// process left over rows
	for ( i--; i >= 0; i-- ) {
		s0 = b[i];
		lptr = L[0] + i;
		for ( j = i + 1; j < n; j++ ) {
			s0 -= lptr[j*nc] * x[j];
		}
		x[i] = s0;
	}	
}

/*
============
idSIMD_AltiVec::MatX_LDLTFactor
============
*/
bool VPCALL idSIMD_AltiVec::MatX_LDLTFactor( idMatX &mat, idVecX &invDiag, const int n ) {
	int i, j, k, nc;
	float *v, *diag, *mptr;
	float s0, s1, s2, s3, sum, d;
	float s0_2, s1_2, s2_2, s3_2, sum_2;
	float *mptr2;

	v = (float *) _alloca16( n * sizeof( float ) );
	diag = (float *) _alloca16( n * sizeof( float ) );

	nc = mat.GetNumColumns();

	if ( n <= 0 ) {
		return true;
	}

	mptr = mat[0];

	sum = mptr[0];

	if ( sum == 0.0f ) {
		return false;
	}

	diag[0] = sum;
	invDiag[0] = d = 1.0f / sum;

	if ( n <= 1 ) {
		return true;
	}

	mptr = mat[0];
	for ( j = 1; j < n; j++ ) {
		mptr[j*nc+0] = ( mptr[j*nc+0] ) * d;
	}

	mptr = mat[1];

	v[0] = diag[0] * mptr[0]; s0 = v[0] * mptr[0];
	sum = mptr[1] - s0;

	if ( sum == 0.0f ) {
		return false;
	}

	mat[1][1] = sum;
	diag[1] = sum;
	invDiag[1] = d = 1.0f / sum;

	if ( n <= 2 ) {
		return true;
	}

	mptr = mat[0];
	for ( j = 2; j < n; j++ ) {
		mptr[j*nc+1] = ( mptr[j*nc+1] - v[0] * mptr[j*nc+0] ) * d;
	}

	mptr = mat[2];

	v[0] = diag[0] * mptr[0]; s0 = v[0] * mptr[0];
	v[1] = diag[1] * mptr[1]; s1 = v[1] * mptr[1];
	sum = mptr[2] - s0 - s1;

	if ( sum == 0.0f ) {
		return false;
	}

	mat[2][2] = sum;
	diag[2] = sum;
	invDiag[2] = d = 1.0f / sum;

	if ( n <= 3 ) {
		return true;
	}

	mptr = mat[0];
	for ( j = 3; j < n; j++ ) {
		mptr[j*nc+2] = ( mptr[j*nc+2] - v[0] * mptr[j*nc+0] - v[1] * mptr[j*nc+1] ) * d;
	}

	mptr = mat[3];

	v[0] = diag[0] * mptr[0]; s0 = v[0] * mptr[0];
	v[1] = diag[1] * mptr[1]; s1 = v[1] * mptr[1];
	v[2] = diag[2] * mptr[2]; s2 = v[2] * mptr[2];
	sum = mptr[3] - s0 - s1 - s2;

	if ( sum == 0.0f ) {
		return false;
	}

	mat[3][3] = sum;
	diag[3] = sum;
	invDiag[3] = d = 1.0f / sum;

	if ( n <= 4 ) {
		return true;
	}

	mptr = mat[0];
	for ( j = 4; j < n; j++ ) {
		mptr[j*nc+3] = ( mptr[j*nc+3] - v[0] * mptr[j*nc+0] - v[1] * mptr[j*nc+1] - v[2] * mptr[j*nc+2] ) * d;
	}

	for ( i = 4; i < n; i++ ) {

		mptr = mat[i];

		v[0] = diag[0] * mptr[0]; s0 = v[0] * mptr[0];
		v[1] = diag[1] * mptr[1]; s1 = v[1] * mptr[1];
		v[2] = diag[2] * mptr[2]; s2 = v[2] * mptr[2];
		v[3] = diag[3] * mptr[3]; s3 = v[3] * mptr[3];
		for ( k = 4; k < i-3; k += 4 ) {
			v[k+0] = diag[k+0] * mptr[k+0]; s0 += v[k+0] * mptr[k+0];
			v[k+1] = diag[k+1] * mptr[k+1]; s1 += v[k+1] * mptr[k+1];
			v[k+2] = diag[k+2] * mptr[k+2]; s2 += v[k+2] * mptr[k+2];
			v[k+3] = diag[k+3] * mptr[k+3]; s3 += v[k+3] * mptr[k+3];
		}
		switch( i - k ) {
			case 3: v[k+2] = diag[k+2] * mptr[k+2]; s0 += v[k+2] * mptr[k+2];
			case 2: v[k+1] = diag[k+1] * mptr[k+1]; s1 += v[k+1] * mptr[k+1];
			case 1: v[k+0] = diag[k+0] * mptr[k+0]; s2 += v[k+0] * mptr[k+0];
		}
		sum = s3;
		sum += s2;
		sum += s1;
		sum += s0;
		sum = mptr[i] - sum;

		if ( sum == 0.0f ) {
			return false;
		}

		mat[i][i] = sum;
		diag[i] = sum;
		invDiag[i] = d = 1.0f / sum;

		if ( i + 1 >= n ) {
			return true;
		}

		// unrolling madness!
		mptr = mat[i+1];
		mptr2 = mat[i+1] + nc;
		
		for ( j = i+1; j+1 < n; j+=2 ) {
			s0 = mptr[0] * v[0];
			s1 = mptr[1] * v[1];
			s2 = mptr[2] * v[2];
			s3 = mptr[3] * v[3];
			
			s0_2 = mptr2[0] * v[0];
			s1_2 = mptr2[1] * v[1];
			s2_2 = mptr2[2] * v[2];
			s3_2 = mptr2[3] * v[3];	
			
			for ( k = 4; k < i-7; k += 8 ) {
				s0 += mptr[k+0] * v[k+0];
				s1 += mptr[k+1] * v[k+1];
				s2 += mptr[k+2] * v[k+2];
				s3 += mptr[k+3] * v[k+3];
				s0 += mptr[k+4] * v[k+4];
				s1 += mptr[k+5] * v[k+5];
				s2 += mptr[k+6] * v[k+6];
				s3 += mptr[k+7] * v[k+7];
			
				s0_2 += mptr2[k+0] * v[k+0];
				s1_2 += mptr2[k+1] * v[k+1];
				s2_2 += mptr2[k+2] * v[k+2];
				s3_2 += mptr2[k+3] * v[k+3];
				s0_2 += mptr2[k+4] * v[k+4];
				s1_2 += mptr2[k+5] * v[k+5];
				s2_2 += mptr2[k+6] * v[k+6];
				s3_2 += mptr2[k+7] * v[k+7];			
			}
			
			switch( i - k ) {
				case 7: s0 += mptr[k+6] * v[k+6]; s0_2 += mptr2[k+6] * v[k+6];
				case 6: s1 += mptr[k+5] * v[k+5]; s1_2 += mptr2[k+5] * v[k+5];
				case 5: s2 += mptr[k+4] * v[k+4]; s2_2 += mptr2[k+4] * v[k+4];
				case 4: s3 += mptr[k+3] * v[k+3]; s3_2 += mptr2[k+3] * v[k+3];
				case 3: s0 += mptr[k+2] * v[k+2]; s0_2 += mptr2[k+2] * v[k+2];
				case 2: s1 += mptr[k+1] * v[k+1]; s1_2 += mptr2[k+1] * v[k+1];
				case 1: s2 += mptr[k+0] * v[k+0]; s2_2 += mptr2[k+0] * v[k+0];
			}
			// disassociate these adds
			s3 += s2;
			s1 += s0;
			sum = s1 + s3;
			
			s3_2 += s2_2;
			s1_2 += s0_2;
			sum_2 = s1_2 + s3_2;
			
			mptr[i] = ( mptr[i] - sum ) * d;
			mptr2[i] = ( mptr2[i] - sum_2 ) * d;
			
			mptr += nc*2;
			mptr2 += nc*2;
		}

		// cleanup
		for ( ; j < n; j++ ) {
			s0 = mptr[0] * v[0];
			s1 = mptr[1] * v[1];
			s2 = mptr[2] * v[2];
			s3 = mptr[3] * v[3];
			for ( k = 4; k < i-7; k += 8 ) {
				s0 += mptr[k+0] * v[k+0];
				s1 += mptr[k+1] * v[k+1];
				s2 += mptr[k+2] * v[k+2];
				s3 += mptr[k+3] * v[k+3];
				s0 += mptr[k+4] * v[k+4];
				s1 += mptr[k+5] * v[k+5];
				s2 += mptr[k+6] * v[k+6];
				s3 += mptr[k+7] * v[k+7];
			}
			switch( i - k ) {
				case 7: s0 += mptr[k+6] * v[k+6];
				case 6: s1 += mptr[k+5] * v[k+5];
				case 5: s2 += mptr[k+4] * v[k+4];
				case 4: s3 += mptr[k+3] * v[k+3];
				case 3: s0 += mptr[k+2] * v[k+2];
				case 2: s1 += mptr[k+1] * v[k+1];
				case 1: s2 += mptr[k+0] * v[k+0];
			}
			// disassociate these adds
			s3 += s2;
			s1 += s0;
			sum = s1 + s3;
			mptr[i] = ( mptr[i] - sum ) * d;
			mptr += nc;
		}
	}
	return true;
}
#endif /* ENABLE_LOWER_TRIANGULAR */


#ifdef LIVE_VICARIOUSLY
/*
============
idSIMD_AltiVec::BlendJoints
============
*/
void VPCALL idSIMD_AltiVec::BlendJoints( idJointQuat *joints, const idJointQuat *blendJoints, const float lerp, const int *index, const int numJoints ) {
	int i;
	
	// since lerp is a constant, we can special case the two cases if they're true
	if ( lerp <= 0.0f ) {
		// this sets joints back to joints. No sense in doing no work, so just return
		return;
	}
	
	if ( lerp >= 1.0f ) {
		// this copies each q from blendJoints to joints and copies each t from blendJoints to joints
		memcpy( joints[0].q.ToFloatPtr(), blendJoints[0].q.ToFloatPtr(), sizeof(idJointQuat) * numJoints );
		return;
	}

	vector float vecLerp = loadSplatUnalignedScalar( &lerp );
	vector float zeroVector = (vector float)(0);
	
	for ( i = 0; i+3 < numJoints; i+=4 ) {
		int j = index[i];
		int j2 = index[i+1];
		int j3 = index[i+2];
		int j4 = index[i+3];
		
		// slerp
		const float *jointPtr = joints[j].q.ToFloatPtr();
		const float *blendPtr = blendJoints[j].q.ToFloatPtr();
		const float *jointPtr2 = joints[j2].q.ToFloatPtr();
		const float *blendPtr2 = blendJoints[j2].q.ToFloatPtr();
		const float *jointPtr3 = joints[j3].q.ToFloatPtr();
		const float *blendPtr3 = blendJoints[j3].q.ToFloatPtr();		
		const float *jointPtr4 = joints[j4].q.ToFloatPtr();
		const float *blendPtr4 = blendJoints[j4].q.ToFloatPtr();		
				
		vector unsigned char permVec = vec_add( vec_lvsl( -1, jointPtr ), (vector unsigned char)(1) );
		vector unsigned char permVec2 = vec_add( vec_lvsl( -1, jointPtr2 ), (vector unsigned char)(1) );
		vector unsigned char permVec3 = vec_add( vec_lvsl( -1, jointPtr3 ), (vector unsigned char)(1) );
		vector unsigned char permVec4 = vec_add( vec_lvsl( -1, jointPtr4 ), (vector unsigned char)(1) );

		vector unsigned char permVec5 = vec_add( vec_lvsl( -1, blendPtr ), (vector unsigned char)(1) );
		vector unsigned char permVec6 = vec_add( vec_lvsl( -1, blendPtr2 ), (vector unsigned char)(1) );
		vector unsigned char permVec7 = vec_add( vec_lvsl( -1, blendPtr3 ), (vector unsigned char)(1) );
		vector unsigned char permVec8 = vec_add( vec_lvsl( -1, blendPtr4 ), (vector unsigned char)(1) );

		vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11;
		vector float v12, v13, v14, v15, v16;
		vector float vecFromX, vecFromY, vecFromZ, vecFromW;
		vector float vecToX, vecToY, vecToZ, vecToW;
			
		// load up the the idJointQuats from joints
		v0 = vec_ld( 0, jointPtr );
		v1 = vec_ld( 15, jointPtr );
		v2 = vec_perm( v0, v1, permVec );
		
		v3 = vec_ld( 0, jointPtr2 );
		v4 = vec_ld( 15, jointPtr2 );
		v5 = vec_perm( v3, v4, permVec2 );
		
		v6 = vec_ld( 0, jointPtr3 );
		v7 = vec_ld( 15, jointPtr3 );
		v8 = vec_perm( v6, v7, permVec3 );

		v9 = vec_ld( 0, jointPtr4 );
		v10 = vec_ld( 15, jointPtr4 );
		v11 = vec_perm( v9, v10, permVec4 );		
				
		// planarizing, so put each x y z w into its own vector
		v0 = vec_mergeh( v2, v8 );
		v1 = vec_mergeh( v5, v11 );
		v3 = vec_mergel( v2, v8 );
		v4 = vec_mergel( v5, v11 );

		vecFromX = vec_mergeh( v0, v1 );
		vecFromY = vec_mergel( v0, v1 );
		vecFromZ = vec_mergeh( v3, v4 );
		vecFromW = vec_mergel( v3, v4 );

		// load up idJointQuats from blendJoints
		v5 = vec_ld( 0, blendPtr );
		v6 = vec_ld( 15, blendPtr );
		v7 = vec_perm( v5, v6, permVec5 );
		
		v8 = vec_ld( 0, blendPtr2 );
		v9 = vec_ld( 15, blendPtr2 );
		v10 = vec_perm( v8, v9, permVec6 );
		
		v11 = vec_ld( 0, blendPtr3 );
		v12 = vec_ld( 15, blendPtr3 );
		v13 = vec_perm( v11, v12, permVec7 );

		v14 = vec_ld( 0, blendPtr4 );
		v15 = vec_ld( 15, blendPtr4 );
		v16 = vec_perm( v14, v15, permVec8 );
			
		// put these into their own vectors too
		v5 = vec_mergeh( v7, v13 );
		v6 = vec_mergeh( v10, v16 );
		v8 = vec_mergel( v7, v13 );
		v9 = vec_mergel( v10, v16 );

		vecToX = vec_mergeh( v5, v6 );
		vecToY = vec_mergel( v5, v6 );
		vecToZ = vec_mergeh( v8, v9 );
		vecToW = vec_mergel( v8, v9 );

		// calculate cosom
		vector float vecCosom = vec_madd( vecFromX, vecToX, (vector float)(0) );
		vecCosom = vec_madd( vecFromY, vecToY, vecCosom );
		vecCosom = vec_madd( vecFromZ, vecToZ, vecCosom );
		vecCosom = vec_madd( vecFromW, vecToW, vecCosom );

		// if cosom is < 0, negate it and set temp to negated elements in to. otherwise, set temp to 
		// to
		vector bool int vecCmp, vecCmp2;
		vecCmp = vec_cmplt( vecCosom, zeroVector );
				
		// negate if needed
		vecToX = vec_sel( vecToX, vec_madd( vecToX, (vector float)(-1), zeroVector ), vecCmp );
		vecToY = vec_sel( vecToY, vec_madd( vecToY, (vector float)(-1), zeroVector ), vecCmp );
		vecToZ = vec_sel( vecToZ, vec_madd( vecToZ, (vector float)(-1), zeroVector ), vecCmp );
		vecToW = vec_sel( vecToW, vec_madd( vecToW, (vector float)(-1), zeroVector ), vecCmp );
		vecCosom = vec_sel( vecCosom, vec_madd( vecCosom, (vector float)(-1), zeroVector ), vecCmp );
		
		// check if we need to calculate scale
		vecCmp2 = vec_cmpgt( vec_sub( (vector float)(1), vecCosom ), (vector float)(1e-6f) );
		vector float vecScale0 = vec_sub( (vector float)(1), vecLerp );
		vector float vecScale1 = vec_splat( vecLerp, 0 );
		
		vector float vecWork1 = vec_sub( (vector float)(1), vec_madd( vecCosom, vecCosom, zeroVector ) );
		vector float vecWork2 = ReciprocalSquareRoot( vecWork1 );
		vector float vecWork3 = VectorATan16( vec_madd( vecWork1, vecWork2, zeroVector ), vecCosom );
		
		vecWork1 = vec_madd( VectorSin16( vec_madd( vecScale0, vecWork3, zeroVector ) ), vecWork2, zeroVector );
		vecWork2 = vec_madd( VectorSin16( vec_madd( vecLerp, vecWork3, zeroVector ) ), vecWork2, zeroVector );
		
		// see which ones we have to insert into our scale0 and scale1 vectors
		vecScale0 = vec_sel( vecScale0, vecWork1, vecCmp2 );
		vecScale1 = vec_sel( vecScale1, vecWork2, vecCmp2 );
				
		// multiply each element by the scale   
		vecFromX = vec_madd( vecFromX, vecScale0, zeroVector );
		vecFromY = vec_madd( vecFromY, vecScale0, zeroVector );
		vecFromZ = vec_madd( vecFromZ, vecScale0, zeroVector );
		vecFromW = vec_madd( vecFromW, vecScale0, zeroVector );

		// multiply temp by scale and add to result
		vecFromX = vec_madd( vecToX, vecScale1, vecFromX );
		vecFromY = vec_madd( vecToY, vecScale1, vecFromY );
		vecFromZ = vec_madd( vecToZ, vecScale1, vecFromZ );
		vecFromW = vec_madd( vecToW, vecScale1, vecFromW );

		// do a transform again to get the results back to vectors we can store out
		v5 = vec_mergeh( vecFromX, vecFromZ );
		v6 = vec_mergeh( vecFromY, vecFromW );
		v8 = vec_mergel( vecFromX, vecFromZ );
		v9 = vec_mergel( vecFromY, vecFromW );

		vecToX = vec_mergeh( v5, v6 );
		vecToY = vec_mergel( v5, v6 );
		vecToZ = vec_mergeh( v8, v9 );
		vecToW = vec_mergel( v8, v9 );

		vector unsigned char storePerm1 = vec_lvsr( 0, jointPtr );
		vector unsigned char storePerm2 = vec_lvsr( 0, jointPtr2 );
		vector unsigned char storePerm3 = vec_lvsr( 0, jointPtr3 );
		vector unsigned char storePerm4 = vec_lvsr( 0, jointPtr4 );

		// right rotate the input data
		vecToX = vec_perm( vecToX, vecToX, storePerm1 );
		vecToY = vec_perm( vecToY, vecToY, storePerm2 );
		vecToZ = vec_perm( vecToZ, vecToZ, storePerm3 );
		vecToW = vec_perm( vecToW, vecToW, storePerm4 );		

		vec_ste( vecToX, 0, (float*) jointPtr );
		vec_ste( vecToX, 4, (float*) jointPtr );
		vec_ste( vecToX, 8, (float*) jointPtr );
		vec_ste( vecToX, 12, (float*) jointPtr );
		
		vec_ste( vecToY, 0, (float*) jointPtr2 );
		vec_ste( vecToY, 4, (float*) jointPtr2 );
		vec_ste( vecToY, 8, (float*) jointPtr2 );
		vec_ste( vecToY, 12, (float*) jointPtr2 );
		
		vec_ste( vecToZ, 0, (float*) jointPtr3 );
		vec_ste( vecToZ, 4, (float*) jointPtr3 );
		vec_ste( vecToZ, 8, (float*) jointPtr3 );
		vec_ste( vecToZ, 12, (float*) jointPtr3 );

		vec_ste( vecToW, 0, (float*) jointPtr4 );
		vec_ste( vecToW, 4, (float*) jointPtr4 );
		vec_ste( vecToW, 8, (float*) jointPtr4 );
		vec_ste( vecToW, 12, (float*) jointPtr4 );		

		// lerp is  v1 + l * ( v2 - v1 );	
		// the idVec3 T is going to be 12 bytes after the Q, so we can do this without calling ToFloatPtr() again. since its
		float *jointVecPtr = (float*)( jointPtr + 4 );
		float *jointVecPtr2 = (float*)( jointPtr2 + 4 );
		float *jointVecPtr3 = (float*)( jointPtr3 + 4 );
		float *jointVecPtr4 = (float*)( jointPtr4 + 4 );

		v0 = vec_ld( 0, jointVecPtr );
		v1 = vec_ld( 11, jointVecPtr );
		vector float vecLd1 = vec_perm( v0, v1, vec_add( vec_lvsl( -1, jointVecPtr ), (vector unsigned char)(1) ) );
		
		v2 = vec_ld( 0, jointVecPtr2 );
		v3 = vec_ld( 11, jointVecPtr2  );
		vector float vecLd2 = vec_perm( v2, v3, vec_add( vec_lvsl( -1, jointVecPtr2 ), (vector unsigned char)(1) ) );
		
		v4 = vec_ld( 0, jointVecPtr3 );
		v5 = vec_ld( 11, jointVecPtr3 );
		vector float vecLd3 = vec_perm( v4, v5, vec_add( vec_lvsl( -1, jointVecPtr3 ), (vector unsigned char)(1) ) );

		v6 = vec_ld( 0, jointVecPtr4  );
		v7 = vec_ld( 11, jointVecPtr4  );
		vector float vecLd4 = vec_perm( v6, v7, vec_add( vec_lvsl( -1, jointVecPtr4 ), (vector unsigned char)(1) ) );
		
		vector float vecVecX, vecVecY, vecVecZ;
		vecVecX = vecVecY = vecVecZ = zeroVector;

		// planarize
		v0 = vec_mergeh( vecLd1, vecLd3 );
		v1 = vec_mergeh( vecLd2, vecLd4 );
		v3 = vec_mergel( vecLd1, vecLd3 );
		v4 = vec_mergel( vecLd2, vecLd4 );

		vecVecX = vec_mergeh( v0, v1 );
		vecVecY = vec_mergel( v0, v1 );
		vecVecZ = vec_mergeh( v3, v4 );

		// load blend joint idvec3's
		float *blendVecPtr = (float*)( blendPtr + 4 );
		float *blendVecPtr2 =(float*)( blendPtr2 + 4 );
		float *blendVecPtr3 = (float*)( blendPtr3 + 4 );
		float *blendVecPtr4 = (float*)( blendPtr4 + 4 );

		v0 = vec_ld( 0, blendVecPtr );
		v1 = vec_ld( 11, blendVecPtr );
		vector float vecLd5 = vec_perm( v0, v1, vec_add( vec_lvsl( -1, blendVecPtr ), (vector unsigned char)(1) ) );
		
		v2 = vec_ld( 0, blendVecPtr2 );
		v3 = vec_ld( 11, blendVecPtr2  );
		vector float vecLd6 = vec_perm( v2, v3, vec_add( vec_lvsl( -1, blendVecPtr2 ), (vector unsigned char)(1) ) );
		
		v4 = vec_ld( 0, blendVecPtr3 );
		v5 = vec_ld( 11, blendVecPtr3 );
		vector float vecLd7 = vec_perm( v4, v5, vec_add( vec_lvsl( -1, blendVecPtr3 ), (vector unsigned char)(1) ) );

		v6 = vec_ld( 0, blendVecPtr4  );
		v7 = vec_ld( 11, blendVecPtr4  );
		vector float vecLd8 = vec_perm( v6, v7, vec_add( vec_lvsl( -1, blendVecPtr4 ), (vector unsigned char)(1) ) );

		vector float vecBlendX, vecBlendY, vecBlendZ;
		vecBlendX = vecBlendY = vecBlendZ = zeroVector;

		// planarize
		v0 = vec_mergeh( vecLd5, vecLd7 );
		v1 = vec_mergeh( vecLd6, vecLd8 );
		v3 = vec_mergel( vecLd5, vecLd7 );
		v4 = vec_mergel( vecLd6, vecLd8 );

		vecBlendX = vec_mergeh( v0, v1 );
		vecBlendY = vec_mergel( v0, v1 );
		vecBlendZ = vec_mergeh( v3, v4 );

		// do subtraction
		vecWork1 = vec_sub( vecBlendX, vecVecX );
		vecWork2 = vec_sub( vecBlendY, vecVecY );
		vecWork3 = vec_sub( vecBlendZ, vecVecZ );
		
		// multiply by lerp and add to v1
		vecVecX = vec_madd( vecWork1, vecLerp, vecVecX );
		vecVecY = vec_madd( vecWork2, vecLerp, vecVecY );
		vecVecZ = vec_madd( vecWork3, vecLerp, vecVecZ );
		
		// put it back in original form
		v0 = vec_mergeh( vecVecX, vecVecZ );
		v1 = vec_mergeh( vecVecY, zeroVector );
		v3 = vec_mergel( vecVecX, vecVecZ );
		v4 = vec_mergel( vecVecY, zeroVector );
		
		// generate vectors to store
		vecWork1 = vec_mergeh( v0, v1 );
		vecWork2 = vec_mergel( v0, v1 );
		vecWork3 = vec_mergeh( v3, v4 );		
		vector float vecWork4 = vec_mergel( v3, v4 );

		// store the T values
		storePerm1 = vec_lvsr( 0, jointVecPtr );
		storePerm2 = vec_lvsr( 0, jointVecPtr2 );
		storePerm3 = vec_lvsr( 0, jointVecPtr3 );
		storePerm4 = vec_lvsr( 0, jointVecPtr4 );
						
		// right rotate the input data
		vecWork1 = vec_perm( vecWork1, vecWork1, storePerm1 );
		vecWork2 = vec_perm( vecWork2, vecWork2, storePerm2 );
		vecWork3 = vec_perm( vecWork3, vecWork3, storePerm3 );
		vecWork4 = vec_perm( vecWork4, vecWork4, storePerm4 );		

		vec_ste( vecWork1, 0, (float*) jointVecPtr );
		vec_ste( vecWork1, 4, (float*) jointVecPtr );
		vec_ste( vecWork1, 8, (float*) jointVecPtr );

		vec_ste( vecWork2, 0, (float*) jointVecPtr2 );
		vec_ste( vecWork2, 4, (float*) jointVecPtr2 );
		vec_ste( vecWork2, 8, (float*) jointVecPtr2 );
		
		vec_ste( vecWork3, 0, (float*) jointVecPtr3 );
		vec_ste( vecWork3, 4, (float*) jointVecPtr3 );
		vec_ste( vecWork3, 8, (float*) jointVecPtr3 );
		
		vec_ste( vecWork4, 0, (float*) jointVecPtr4 );
		vec_ste( vecWork4, 4, (float*) jointVecPtr4 );
		vec_ste( vecWork4, 8, (float*) jointVecPtr4 );
	}
	
	// cleanup
	for ( ; i < numJoints; i++ ) {
		int j = index[i];
		joints[j].q.Slerp( joints[j].q, blendJoints[j].q, lerp );
		joints[j].t.Lerp( joints[j].t, blendJoints[j].t, lerp );
	}
}

/*
============
idSIMD_AltiVec::ConvertJointQuatsToJointMats
============
*/

// SSE doesn't vectorize this, and I don't think we should either. Its mainly just copying data, there's very little math involved and
// it's not easily parallelizable
void VPCALL idSIMD_AltiVec::ConvertJointQuatsToJointMats( idJointMat *jointMats, const idJointQuat *jointQuats, const int numJoints ) {
 
	for ( int i = 0; i < numJoints; i++ ) {

		const float *q = jointQuats[i].q.ToFloatPtr();
		float *m = jointMats[i].ToFloatPtr();

		m[0*4+3] = q[4];
		m[1*4+3] = q[5];
		m[2*4+3] = q[6];

		float x2 = q[0] + q[0];
		float y2 = q[1] + q[1];
		float z2 = q[2] + q[2];

		{
			float xx = q[0] * x2;
			float yy = q[1] * y2;
			float zz = q[2] * z2;

			m[0*4+0] = 1.0f - yy - zz;
			m[1*4+1] = 1.0f - xx - zz;
			m[2*4+2] = 1.0f - xx - yy;
		}

		{
			float yz = q[1] * z2;
			float wx = q[3] * x2;

			m[2*4+1] = yz - wx;
			m[1*4+2] = yz + wx;
		}

		{
			float xy = q[0] * y2;
			float wz = q[3] * z2;

			m[1*4+0] = xy - wz;
			m[0*4+1] = xy + wz;
		}

		{
			float xz = q[0] * z2;
			float wy = q[3] * y2;

			m[0*4+2] = xz - wy;
			m[2*4+0] = xz + wy;
		}
	}
}

/*
============
idSIMD_AltiVec::ConvertJointMatsToJointQuats
============
*/
void VPCALL idSIMD_AltiVec::ConvertJointMatsToJointQuats( idJointQuat *jointQuats, const idJointMat *jointMats, const int numJoints ) {

	int index;

	// Since we use very little of the data we have to pull in for the altivec version, we end up with
	// a lot of wasted math. Rather than try to force it to use altivec, I wrote an optimized version
	// of InvSqrt for the G5, and made it use that instead. With only this change, we get a little 
	// bigger than 50% speedup, which is not too shabby. Should really replace idMath::InvSqrt with
	// my function so everyone can benefit on G5.
	
	for ( index = 0; index < numJoints; index++ ) {

		idJointQuat	jq;
		float		trace;
		float		s;
		float		t;
		int     	i;
		int			j;
		int			k;
	
		static int 	next[3] = { 1, 2, 0 };

		float *mat = (float*)( jointMats[index].ToFloatPtr() );
		trace = mat[0 * 4 + 0] + mat[1 * 4 + 1] + mat[2 * 4 + 2];

		if ( trace > 0.0f ) {

			t = trace + 1.0f;
			//s = idMath::InvSqrt( t ) * 0.5f;
			s = FastScalarInvSqrt( t ) * 0.5f;

			jq.q[3] = s * t;
			jq.q[0] = ( mat[1 * 4 + 2] - mat[2 * 4 + 1] ) * s;
			jq.q[1] = ( mat[2 * 4 + 0] - mat[0 * 4 + 2] ) * s;
			jq.q[2] = ( mat[0 * 4 + 1] - mat[1 * 4 + 0] ) * s;

		} else {

			i = 0;
			if ( mat[1 * 4 + 1] > mat[0 * 4 + 0] ) {
				i = 1;
			}
			if ( mat[2 * 4 + 2] > mat[i * 4 + i] ) {
				i = 2;
			}
			j = next[i];
			k = next[j];

			t = ( mat[i * 4 + i] - ( mat[j * 4 + j] + mat[k * 4 + k] ) ) + 1.0f;
			//s = idMath::InvSqrt( t ) * 0.5f;
			s = FastScalarInvSqrt( t ) * 0.5f;

			jq.q[i] = s * t;
			jq.q[3] = ( mat[j * 4 + k] - mat[k * 4 + j] ) * s;
			jq.q[j] = ( mat[i * 4 + j] + mat[j * 4 + i] ) * s;
			jq.q[k] = ( mat[i * 4 + k] + mat[k * 4 + i] ) * s;
		}

		jq.t[0] = mat[0 * 4 + 3];
		jq.t[1] = mat[1 * 4 + 3];
		jq.t[2] = mat[2 * 4 + 3];
		jointQuats[index] = jq;
	}
}

/*
============
idSIMD_AltiVec::TransformJoints
============
*/
void VPCALL idSIMD_AltiVec::TransformJoints( idJointMat *jointMats, const int *parents, const int firstJoint, const int lastJoint ) {
	int i;
#if 0
	for( i = firstJoint; i <= lastJoint; i++ ) {
		assert( parents[i] < i );
		jointMats[i] *= jointMats[parents[i]];
	}
#else

	// I don't think you can unroll this since the next iteration of the loop might depending on the previous iteration, depending
	// on what the parents array looks like. This is true in the test code.
	for ( i = firstJoint; i <= lastJoint; i++ ) {
		assert( parents[i] < i );
		float *jointPtr = jointMats[i].ToFloatPtr();
		float *parentPtr = jointMats[parents[i]].ToFloatPtr();

		vector unsigned char permVec = vec_add( vec_lvsl( -1, jointPtr ), (vector unsigned char)(1) );
		vector unsigned char permVec2 = vec_add( vec_lvsl( -1, parentPtr ), (vector unsigned char)(1) );
		vector float v0, v1, v2, v3, v4, v5, v6, v7;
	
		// we need to load up 12 float elements that make up the Mat
		v0 = vec_ld( 0, jointPtr );
		v1 = vec_ld( 15, jointPtr );
		v2 = vec_ld( 31, jointPtr );
		v3 = vec_ld( 47, jointPtr );
		
		// load parents
		v4 = vec_ld( 0, parentPtr );
		v5 = vec_ld( 15, parentPtr );
		v6 = vec_ld( 31, parentPtr );
		v7 = vec_ld( 47, parentPtr );
		
		// permute into vectors
		vector float vecJointMat1 = vec_perm( v0, v1, permVec );
		vector float vecJointMat2 = vec_perm( v1, v2, permVec );
		vector float vecJointMat3 = vec_perm( v2, v3, permVec );
		
		vector float vecParentMat1 = vec_perm( v4, v5, permVec2 );
		vector float vecParentMat2 = vec_perm( v5, v6, permVec2 );
		vector float vecParentMat3 = vec_perm( v6, v7, permVec2 );

		vector float zero = (vector float)(0);
		vector float C1, C2, C3;

		// matrix multiply
		C1 = vec_madd( vecJointMat1, vec_splat( vecParentMat1, 0 ), zero ); // m(0 to 3) * a(0)
		C2 = vec_madd( vecJointMat1, vec_splat( vecParentMat2, 0 ), zero ); //  m(4 to 7) * a(4)
		C3 = vec_madd( vecJointMat1, vec_splat( vecParentMat3, 0 ), zero ); // m(8 to 11) * a(8)
		
		C1 = vec_madd( vecJointMat2, vec_splat( vecParentMat1, 1 ), C1 ); // add in m(4 to 7) * a(1)
		C2 = vec_madd( vecJointMat2, vec_splat( vecParentMat2, 1 ), C2 ); // add in m(4 to 7) * a(5)
		C3 = vec_madd( vecJointMat2, vec_splat( vecParentMat3, 1 ), C3 ); // add in m(4 to 7) * a(9)
		
		C1 = vec_madd( vecJointMat3, vec_splat( vecParentMat1, 2 ), C1 ); 
		C2 = vec_madd( vecJointMat3, vec_splat( vecParentMat2, 2 ), C2 );
		C3 = vec_madd( vecJointMat3, vec_splat( vecParentMat3, 2 ), C3 );

		// do the addition at the end
		vector unsigned char permZeroAndLast = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,28,29,30,31);
		C1 = vec_add( C1, vec_perm( zero, vecParentMat1, permZeroAndLast ) );
		C2 = vec_add( C2, vec_perm( zero, vecParentMat2, permZeroAndLast ) );
		C3 = vec_add( C3, vec_perm( zero, vecParentMat3, permZeroAndLast ) );

		// store results
		UNALIGNED_STORE3( (float*) jointPtr, C1, C2, C3 );
	}
#endif
}

/*
============
idSIMD_AltiVec::UntransformJoints
============
*/
void VPCALL idSIMD_AltiVec::UntransformJoints( idJointMat *jointMats, const int *parents, const int firstJoint, const int lastJoint ) {
	int i;
#if 0
	for( i = lastJoint; i >= firstJoint; i-- ) {
		assert( parents[i] < i );
		jointMats[i] /= jointMats[parents[i]];
	}
#else
	// I don't think you can unroll this since the next iteration of the loop might depending on the previous iteration, depending
	// on what the parents array looks like. This is true in the test code.
	for ( i = lastJoint; i >= firstJoint; i-- ) {
		assert( parents[i] < i );
		float *jointPtr = jointMats[i].ToFloatPtr();
		float *parentPtr = jointMats[parents[i]].ToFloatPtr();

		vector unsigned char permVec = vec_add( vec_lvsl( -1, jointPtr ), (vector unsigned char)(1) );
		vector unsigned char permVec2 = vec_add( vec_lvsl( -1, parentPtr ), (vector unsigned char)(1) );
		vector float v0, v1, v2, v3, v4, v5, v6, v7;
	
		// we need to load up 12 float elements that make up the Mat
		v0 = vec_ld( 0, jointPtr );
		v1 = vec_ld( 15, jointPtr );
		v2 = vec_ld( 31, jointPtr );
		v3 = vec_ld( 47, jointPtr );
		
		// load parents
		v4 = vec_ld( 0, parentPtr );
		v5 = vec_ld( 15, parentPtr );
		v6 = vec_ld( 31, parentPtr );
		v7 = vec_ld( 47, parentPtr );
		
		// permute into vectors
		vector float vecJointMat1 = vec_perm( v0, v1, permVec );
		vector float vecJointMat2 = vec_perm( v1, v2, permVec );
		vector float vecJointMat3 = vec_perm( v2, v3, permVec );
		
		vector float vecParentMat1 = vec_perm( v4, v5, permVec2 );
		vector float vecParentMat2 = vec_perm( v5, v6, permVec2 );
		vector float vecParentMat3 = vec_perm( v6, v7, permVec2 );

		vector float zero = (vector float)(0);
		vector float C1, C2, C3;
		
		// do subtraction at the beginning
		vector unsigned char permZeroAndLast = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,28,29,30,31);
		vecJointMat1 = vec_sub( vecJointMat1, vec_perm( zero, vecParentMat1, permZeroAndLast ) );
		vecJointMat2 = vec_sub( vecJointMat2, vec_perm( zero, vecParentMat2, permZeroAndLast ) );
		vecJointMat3 = vec_sub( vecJointMat3, vec_perm( zero, vecParentMat3, permZeroAndLast ) );
		
		// matrix multiply
		C1 = vec_madd( vecJointMat1, vec_splat( vecParentMat1, 0 ), zero );
		C2 = vec_madd( vecJointMat1, vec_splat( vecParentMat1, 1 ), zero ); 
		C3 = vec_madd( vecJointMat1, vec_splat( vecParentMat1, 2 ), zero ); 
		
		C1 = vec_madd( vecJointMat2, vec_splat( vecParentMat2, 0 ), C1 ); 
		C2 = vec_madd( vecJointMat2, vec_splat( vecParentMat2, 1 ), C2 ); 
		C3 = vec_madd( vecJointMat2, vec_splat( vecParentMat2, 2 ), C3 ); 
		
		C1 = vec_madd( vecJointMat3, vec_splat( vecParentMat3, 0 ), C1 ); 
		C2 = vec_madd( vecJointMat3, vec_splat( vecParentMat3, 1 ), C2 );
		C3 = vec_madd( vecJointMat3, vec_splat( vecParentMat3, 2 ), C3 );

		// store results back
		vector unsigned char storePerm = vec_lvsr( 0, jointPtr );
		
		// right rotate the input data
		C1 = vec_perm( C1, C1, storePerm );
		C2 = vec_perm( C2, C2, storePerm );
		C3 = vec_perm( C3, C3, storePerm );

		vec_ste( C1, 0, (float*) jointPtr );
		vec_ste( C1, 4, (float*) jointPtr );
		vec_ste( C1, 8, (float*) jointPtr );
		vec_ste( C1, 12, (float*) jointPtr );

		vec_ste( C2, 16, (float*) jointPtr );
		vec_ste( C2, 20, (float*) jointPtr );
		vec_ste( C2, 24, (float*) jointPtr );
		vec_ste( C2, 28, (float*) jointPtr );

		vec_ste( C3, 32, (float*) jointPtr );
		vec_ste( C3, 36, (float*) jointPtr );
		vec_ste( C3, 40, (float*) jointPtr );
		vec_ste( C3, 44, (float*) jointPtr );
	}

#endif
}

/*
============
idSIMD_AltiVec::TransformVerts
============
*/

// Here we don't have much for the vector unit to do, and the gain we get from doing the math
// in parallel is eaten by doing unaligned stores. 
void VPCALL idSIMD_AltiVec::TransformVerts( idDrawVert *verts, const int numVerts, const idJointMat *joints, const idVec4 *weights, const int *index, int numWeights ) {
	int i, j;
	const byte *jointsPtr = (byte *)joints;

	for( j = i = 0; i < numVerts; i++ ) {
		idVec3 v;

		float *matPtrOrig = ( *(idJointMat *)( jointsPtr + index[j*2] ) ).ToFloatPtr();
		float *weightPtr = (float*) weights[j].ToFloatPtr();
		
		v[0] = matPtrOrig[0] * weightPtr[0];
		v[0] += matPtrOrig[1] * weightPtr[1];
		v[0] += matPtrOrig[2] * weightPtr[2];
		v[0] += matPtrOrig[3] * weightPtr[3];

		v[1] = matPtrOrig[4] * weightPtr[0];
		v[1] += matPtrOrig[5] * weightPtr[1];
		v[1] += matPtrOrig[6] * weightPtr[2];
		v[1] += matPtrOrig[7] * weightPtr[3];
			
		v[2] = matPtrOrig[8] * weightPtr[0];
		v[2] += matPtrOrig[9] * weightPtr[1];
		v[2] += matPtrOrig[10] * weightPtr[2];
		v[2] += matPtrOrig[11] * weightPtr[3];

		while( index[j*2+1] == 0 ) {
			j++;
			float *matPtr = ( *(idJointMat *)( jointsPtr + index[j*2] ) ).ToFloatPtr();
			weightPtr = (float*) weights[j].ToFloatPtr();
		
			v[0] += matPtr[0] * weightPtr[0];
			v[0] += matPtr[1] * weightPtr[1];
			v[0] += matPtr[2] * weightPtr[2];
			v[0] += matPtr[3] * weightPtr[3];
			
			v[1] += matPtr[4] * weightPtr[0];
			v[1] += matPtr[5] * weightPtr[1];
			v[1] += matPtr[6] * weightPtr[2];
			v[1] += matPtr[7] * weightPtr[3];
			
			v[2] += matPtr[8] * weightPtr[0];
			v[2] += matPtr[9] * weightPtr[1];
			v[2] += matPtr[10] * weightPtr[2];
			v[2] += matPtr[11] * weightPtr[3];
		}
		j++;

		verts[i].xyz = v;
	}
}
#endif /* LIVE_VICARIOUSLY */

#ifdef ENABLE_CULL

#ifndef DRAWVERT_PADDED
/*
============
idSIMD_AltiVec::TracePointCull
============
*/
void VPCALL idSIMD_AltiVec::TracePointCull( byte *cullBits, byte &totalOr, const float radius, const idPlane *planes, const idDrawVert *verts, const int numVerts ) {
	
	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	
	byte tOr;
	tOr = 0;

	// pointers
	const float *planePtr = planes[0].ToFloatPtr();
	
	vector unsigned int vecShift1 = (vector unsigned int)(0,1,2,3);
	vector unsigned int vecShift2 = (vector unsigned int)(4,5,6,7);
	vector unsigned int vecFlipBits = (vector unsigned int)(0x0F);
	vector float vecPlane0, vecPlane1, vecPlane2, vecPlane3;
	vector bool int vecCmp1, vecCmp2, vecCmp3, vecCmp4, vecCmp5, vecCmp6, vecCmp7, vecCmp8;
	vector unsigned char vecPerm;
	vector float v0, v1, v2, v3, v4, v5, v6, v7;
	vector float zeroVector = (vector float)(0);
	vector float vecRadius;
	vector float vecXYZ1, vecXYZ2, vecXYZ3, vecXYZ4;
	vector float vec1Sum1, vec1Sum2, vec1Sum3, vec1Sum4;
	vector unsigned char vecPermLast = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,16,17,18,19);
	vector unsigned char vecPermHalves = (vector unsigned char)(0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
	vector float vecDPlusRadius1, vecDPlusRadius2, vecDPlusRadius3, vecDPlusRadius4;
	vector float vecDMinusRadius1, vecDMinusRadius2, vecDMinusRadius3, vecDMinusRadius4; 
	vector bool int oneIntVector = (vector bool int)(1);
	vector unsigned int vecBitShifted1, vecBitShifted2, vecBitShifted3, vecBitShifted4, vecBitShifted5, vecBitShifted6, vecBitShifted7, vecBitShifted8;
	vector unsigned int vecTotals;
	vector unsigned int tempIntSum;
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	
	vecPerm = vec_add( vec_lvsl( -1, planePtr ), (vector unsigned char)(1) );
	
	// populate planes 
	v0 = vec_ld( 0, planePtr );
	v1 = vec_ld( 15, planePtr );
	vecPlane0 = vec_perm( v0, v1, vecPerm );

	v2 = vec_ld( 0, planePtr + 4 );
	v3 = vec_ld( 15, planePtr + 4 );
	vecPlane1 = vec_perm( v2, v3, vecPerm );

	v0 = vec_ld( 0, planePtr + 8 );
	v1 = vec_ld( 15, planePtr + 8 );
	vecPlane2 = vec_perm( v0, v1, vecPerm );

	v2 = vec_ld( 0, planePtr + 12 );
	v3 = vec_ld( 15, planePtr + 12 );
	vecPlane3 = vec_perm( v2, v3, vecPerm );
	
	// transpose
	v0 = vec_mergeh( vecPlane0, vecPlane2 );
	v1 = vec_mergeh( vecPlane1, vecPlane3 );
	v2 = vec_mergel( vecPlane0, vecPlane2 );
	v3 = vec_mergel( vecPlane1, vecPlane3 );

	vecPlane0 = vec_mergeh( v0, v1 );
	vecPlane1 = vec_mergel( v0, v1 );
	vecPlane2 = vec_mergeh( v2, v3 );
	vecPlane3 = vec_mergel( v2, v3 );
	
	// load constants
	vecRadius = loadSplatUnalignedScalar( &radius );
	
	unsigned int cullBitVal[4];
	vector unsigned char cullBitPerm = vec_lvsr( 0, &cullBitVal[0] );
	int i = 0;
	
	// every fourth one will have the same alignment. Make sure we've got enough here
	if ( i+3 < numVerts ) {
		vertPerm1 = vec_add( vec_lvsl( -1, (float*) verts[0].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm2 = vec_add( vec_lvsl( -1, (float*) verts[1].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm3 = vec_add( vec_lvsl( -1, (float*) verts[2].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm4 = vec_add( vec_lvsl( -1, (float*) verts[3].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
	}
	

	for ( ; i+3 < numVerts; i+=4 ) {
		const float *vertPtr = verts[i].xyz.ToFloatPtr();
		const float *vertPtr2 = verts[i+1].xyz.ToFloatPtr();
		const float *vertPtr3 = verts[i+2].xyz.ToFloatPtr();
		const float *vertPtr4 = verts[i+3].xyz.ToFloatPtr();
			
		v0 = vec_ld( 0, vertPtr );
		v1 = vec_ld( 15, vertPtr );
		v2 = vec_ld( 0, vertPtr2 );
		v3 = vec_ld( 15, vertPtr2 );
		v4 = vec_ld( 0, vertPtr3 );
		v5 = vec_ld( 15, vertPtr3 );
		v6 = vec_ld( 0, vertPtr4 );
		v7 = vec_ld( 15, vertPtr4 );
			
		vecXYZ1 = vec_perm( v0, v1, vertPerm1 );
		vecXYZ2 = vec_perm( v2, v3, vertPerm2 );
		vecXYZ3 = vec_perm( v4, v5, vertPerm3 );
		vecXYZ4 = vec_perm( v6, v7, vertPerm4 );

		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 0 ), vecPlane0, zeroVector );
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 1 ), vecPlane1, vec1Sum1 );
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 2 ), vecPlane2, vec1Sum1 );
		vec1Sum1 = vec_add( vec1Sum1, vecPlane3 );
		
		vec1Sum2 = vec_madd( vec_splat( vecXYZ2, 0 ), vecPlane0, zeroVector );
		vec1Sum2 = vec_madd( vec_splat( vecXYZ2, 1 ), vecPlane1, vec1Sum2 );
		vec1Sum2 = vec_madd( vec_splat( vecXYZ2, 2 ), vecPlane2, vec1Sum2 );
		vec1Sum2 = vec_add( vec1Sum2, vecPlane3 );		
		
		vec1Sum3 = vec_madd( vec_splat( vecXYZ3, 0 ), vecPlane0, zeroVector );
		vec1Sum3 = vec_madd( vec_splat( vecXYZ3, 1 ), vecPlane1, vec1Sum3 );
		vec1Sum3 = vec_madd( vec_splat( vecXYZ3, 2 ), vecPlane2, vec1Sum3 );
		vec1Sum3 = vec_add( vec1Sum3, vecPlane3 );
				
		vec1Sum4 = vec_madd( vec_splat( vecXYZ4, 0 ), vecPlane0, zeroVector );
		vec1Sum4 = vec_madd( vec_splat( vecXYZ4, 1 ), vecPlane1, vec1Sum4 );
		vec1Sum4 = vec_madd( vec_splat( vecXYZ4, 2 ), vecPlane2, vec1Sum4 );
		vec1Sum4 = vec_add( vec1Sum4, vecPlane3 );				

		// vec1Sum1 now holds d0, d1, d2, d3. calculate the
		// difference with +radius and -radius
		vecDPlusRadius1 = vec_add( vec1Sum1, vecRadius );
		vecDMinusRadius1 = vec_sub( vec1Sum1, vecRadius );
		vecDPlusRadius2 = vec_add( vec1Sum2, vecRadius );
		vecDMinusRadius2 = vec_sub( vec1Sum2, vecRadius );
		vecDPlusRadius3 = vec_add( vec1Sum3, vecRadius );
		vecDMinusRadius3 = vec_sub( vec1Sum3, vecRadius );
		vecDPlusRadius4 = vec_add( vec1Sum4, vecRadius );
		vecDMinusRadius4 = vec_sub( vec1Sum4, vecRadius );
		
		// do compare
		vecCmp1 = vec_cmplt( vecDPlusRadius1, zeroVector );
		vecCmp2 = vec_cmplt( vecDMinusRadius1, zeroVector );
		vecCmp3 = vec_cmplt( vecDPlusRadius2, zeroVector );
		vecCmp4 = vec_cmplt( vecDMinusRadius2, zeroVector );
		vecCmp5 = vec_cmplt( vecDPlusRadius3, zeroVector );
		vecCmp6 = vec_cmplt( vecDMinusRadius3, zeroVector );
		vecCmp7 = vec_cmplt( vecDPlusRadius4, zeroVector );
		vecCmp8 = vec_cmplt( vecDMinusRadius4, zeroVector );
		
		//and it with 1 so we multiply by 1 not 1111's
		vecCmp1 = vec_and( vecCmp1, oneIntVector );
		vecCmp2 = vec_and( vecCmp2, oneIntVector );
		vecCmp3 = vec_and( vecCmp3, oneIntVector );
		vecCmp4 = vec_and( vecCmp4, oneIntVector );
		vecCmp5 = vec_and( vecCmp5, oneIntVector );
		vecCmp6 = vec_and( vecCmp6, oneIntVector );
		vecCmp7 = vec_and( vecCmp7, oneIntVector );
		vecCmp8 = vec_and( vecCmp8, oneIntVector );
		
		vecBitShifted1 = vec_sl( (vector unsigned int)vecCmp1, vecShift1 );
		vecBitShifted2 = vec_sl( (vector unsigned int)vecCmp2, vecShift2 );
		vecBitShifted3 = vec_sl( (vector unsigned int)vecCmp3, vecShift1 );
		vecBitShifted4 = vec_sl( (vector unsigned int)vecCmp4, vecShift2 );
		vecBitShifted5 = vec_sl( (vector unsigned int)vecCmp5, vecShift1 );
		vecBitShifted6 = vec_sl( (vector unsigned int)vecCmp6, vecShift2 );
		vecBitShifted7 = vec_sl( (vector unsigned int)vecCmp7, vecShift1 );
		vecBitShifted8 = vec_sl( (vector unsigned int)vecCmp8, vecShift2 );
	
		// OR (add) them all together
		vecBitShifted1 = vec_add( vecBitShifted1, vecBitShifted2 );
		vecBitShifted3 = vec_add( vecBitShifted3, vecBitShifted4 );
		vecBitShifted5 = vec_add( vecBitShifted5, vecBitShifted6 );
		vecBitShifted7 = vec_add( vecBitShifted7, vecBitShifted8 );
		
		vecTotals = vec_add( vecBitShifted1, vec_sld( vecBitShifted1, vecBitShifted1, 8 ) );
		vecTotals = vec_add( vecTotals, vec_sld( vecTotals, vecTotals, 4 ) );
		tempIntSum = vec_add( vecBitShifted3, vec_sld( vecBitShifted3, vecBitShifted3, 8 ) );
		tempIntSum = vec_add( tempIntSum, vec_sld( tempIntSum, tempIntSum, 4 ) );
		vecTotals = vec_mergeh( vecTotals, tempIntSum );
		tempIntSum = vec_add( vecBitShifted5, vec_sld( vecBitShifted5, vecBitShifted5, 8 ) );
		tempIntSum = vec_add( tempIntSum, vec_sld( tempIntSum, tempIntSum, 4 ) );		
		vecTotals = vec_perm( vecTotals, tempIntSum, vecPermHalves );
		tempIntSum = vec_add( vecBitShifted7, vec_sld( vecBitShifted7, vecBitShifted7, 8 ) );
		tempIntSum = vec_add( tempIntSum, vec_sld( tempIntSum, tempIntSum, 4 ) );
		vecTotals = vec_perm( vecTotals, tempIntSum, vecPermLast );
				
		// store out results
		vector unsigned int tempSt = vec_xor( vecTotals, vecFlipBits );
		tempSt = vec_perm( tempSt, tempSt, cullBitPerm );
		vec_ste( tempSt, 0, &cullBitVal[0] );
		vec_ste( tempSt, 4, &cullBitVal[0] );
		vec_ste( tempSt, 8, &cullBitVal[0] );
		vec_ste( tempSt, 12, &cullBitVal[0] );
	
		tOr |= cullBitVal[0];
		tOr |= cullBitVal[1];
		tOr |= cullBitVal[2];
		tOr |= cullBitVal[3];
		
		cullBits[i] = cullBitVal[0];
		cullBits[i+1] = cullBitVal[1];
		cullBits[i+2] = cullBitVal[2];
		cullBits[i+3] = cullBitVal[3];		
	}

	// cleanup
	for ( ; i < numVerts; i++ ) {
		byte bits;
		float d0, d1, d2, d3, t;
		const idVec3 &v = verts[i].xyz;

		d0 = planes[0].Distance( v );
		d1 = planes[1].Distance( v );
		d2 = planes[2].Distance( v );
		d3 = planes[3].Distance( v );

		t = d0 + radius;
		bits  = FLOATSIGNBITSET( t ) << 0;
		t = d1 + radius;
		bits |= FLOATSIGNBITSET( t ) << 1;
		t = d2 + radius;
		bits |= FLOATSIGNBITSET( t ) << 2;
		t = d3 + radius;
		bits |= FLOATSIGNBITSET( t ) << 3;

		t = d0 - radius;
		bits |= FLOATSIGNBITSET( t ) << 4;
		t = d1 - radius;
		bits |= FLOATSIGNBITSET( t ) << 5;
		t = d2 - radius;
		bits |= FLOATSIGNBITSET( t ) << 6;
		t = d3 - radius;
		bits |= FLOATSIGNBITSET( t ) << 7;

		bits ^= 0x0F;		// flip lower four bits

		tOr |= bits;
		cullBits[i] = bits;
	}

	totalOr = tOr;
}
#else

/*
============
idSIMD_AltiVec::TracePointCull
============
*/
void VPCALL idSIMD_AltiVec::TracePointCull( byte *cullBits, byte &totalOr, const float radius, const idPlane *planes, const idDrawVert *verts, const int numVerts ) {
	
	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	
	byte tOr;
	tOr = 0;

	// pointers
	const float *planePtr = planes[0].ToFloatPtr();
	
	vector unsigned int vecShift1 = (vector unsigned int)(0,1,2,3);
	vector unsigned int vecShift2 = (vector unsigned int)(4,5,6,7);
	vector unsigned int vecFlipBits = (vector unsigned int)(0x0F);
	vector float vecPlane0, vecPlane1, vecPlane2, vecPlane3;
	vector bool int vecCmp1, vecCmp2, vecCmp3, vecCmp4, vecCmp5, vecCmp6, vecCmp7, vecCmp8;
	vector unsigned char vecPerm;
	vector float v0, v1, v2, v3, v4, v5, v6, v7;
	vector float zeroVector = (vector float)(0);
	vector float vecRadius;
	vector float vecXYZ1, vecXYZ2, vecXYZ3, vecXYZ4;
	vector float vec1Sum1, vec1Sum2, vec1Sum3, vec1Sum4;
	vector unsigned char vecPermLast = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,16,17,18,19);
	vector unsigned char vecPermHalves = (vector unsigned char)(0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
	vector float vecDPlusRadius1, vecDPlusRadius2, vecDPlusRadius3, vecDPlusRadius4;
	vector float vecDMinusRadius1, vecDMinusRadius2, vecDMinusRadius3, vecDMinusRadius4; 
	vector bool int oneIntVector = (vector bool int)(1);
	vector unsigned int vecBitShifted1, vecBitShifted2, vecBitShifted3, vecBitShifted4, vecBitShifted5, vecBitShifted6, vecBitShifted7, vecBitShifted8;
	vector unsigned int vecTotals;
	vector unsigned int tempIntSum;
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	
	vecPerm = vec_add( vec_lvsl( -1, planePtr ), (vector unsigned char)(1) );
	
	// populate planes 
	v0 = vec_ld( 0, planePtr );
	v1 = vec_ld( 15, planePtr );
	vecPlane0 = vec_perm( v0, v1, vecPerm );

	v2 = vec_ld( 0, planePtr + 4 );
	v3 = vec_ld( 15, planePtr + 4 );
	vecPlane1 = vec_perm( v2, v3, vecPerm );

	v0 = vec_ld( 0, planePtr + 8 );
	v1 = vec_ld( 15, planePtr + 8 );
	vecPlane2 = vec_perm( v0, v1, vecPerm );

	v2 = vec_ld( 0, planePtr + 12 );
	v3 = vec_ld( 15, planePtr + 12 );
	vecPlane3 = vec_perm( v2, v3, vecPerm );
	
	// transpose
	v0 = vec_mergeh( vecPlane0, vecPlane2 );
	v1 = vec_mergeh( vecPlane1, vecPlane3 );
	v2 = vec_mergel( vecPlane0, vecPlane2 );
	v3 = vec_mergel( vecPlane1, vecPlane3 );

	vecPlane0 = vec_mergeh( v0, v1 );
	vecPlane1 = vec_mergel( v0, v1 );
	vecPlane2 = vec_mergeh( v2, v3 );
	vecPlane3 = vec_mergel( v2, v3 );
	
	// load constants
	vecRadius = loadSplatUnalignedScalar( &radius );
	
	unsigned int cullBitVal[4];
	vector unsigned char cullBitPerm = vec_lvsr( 0, &cullBitVal[0] );
	int i = 0;
	

	for ( ; i+3 < numVerts; i+=4 ) {
		const float *vertPtr = verts[i].xyz.ToFloatPtr();
		const float *vertPtr2 = verts[i+1].xyz.ToFloatPtr();
		const float *vertPtr3 = verts[i+2].xyz.ToFloatPtr();
		const float *vertPtr4 = verts[i+3].xyz.ToFloatPtr();
			
		vecXYZ1 = vec_ld( 0, vertPtr );
		vecXYZ2 = vec_ld( 0, vertPtr2 );
		vecXYZ3 = vec_ld( 0, vertPtr3 );
		vecXYZ4 = vec_ld( 0, vertPtr4 );
		
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 0 ), vecPlane0, zeroVector );
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 1 ), vecPlane1, vec1Sum1 );
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 2 ), vecPlane2, vec1Sum1 );
		vec1Sum1 = vec_add( vec1Sum1, vecPlane3 );
		
		vec1Sum2 = vec_madd( vec_splat( vecXYZ2, 0 ), vecPlane0, zeroVector );
		vec1Sum2 = vec_madd( vec_splat( vecXYZ2, 1 ), vecPlane1, vec1Sum2 );
		vec1Sum2 = vec_madd( vec_splat( vecXYZ2, 2 ), vecPlane2, vec1Sum2 );
		vec1Sum2 = vec_add( vec1Sum2, vecPlane3 );		
		
		vec1Sum3 = vec_madd( vec_splat( vecXYZ3, 0 ), vecPlane0, zeroVector );
		vec1Sum3 = vec_madd( vec_splat( vecXYZ3, 1 ), vecPlane1, vec1Sum3 );
		vec1Sum3 = vec_madd( vec_splat( vecXYZ3, 2 ), vecPlane2, vec1Sum3 );
		vec1Sum3 = vec_add( vec1Sum3, vecPlane3 );
				
		vec1Sum4 = vec_madd( vec_splat( vecXYZ4, 0 ), vecPlane0, zeroVector );
		vec1Sum4 = vec_madd( vec_splat( vecXYZ4, 1 ), vecPlane1, vec1Sum4 );
		vec1Sum4 = vec_madd( vec_splat( vecXYZ4, 2 ), vecPlane2, vec1Sum4 );
		vec1Sum4 = vec_add( vec1Sum4, vecPlane3 );				

		// vec1Sum1 now holds d0, d1, d2, d3. calculate the
		// difference with +radius and -radius
		vecDPlusRadius1 = vec_add( vec1Sum1, vecRadius );
		vecDMinusRadius1 = vec_sub( vec1Sum1, vecRadius );
		vecDPlusRadius2 = vec_add( vec1Sum2, vecRadius );
		vecDMinusRadius2 = vec_sub( vec1Sum2, vecRadius );
		vecDPlusRadius3 = vec_add( vec1Sum3, vecRadius );
		vecDMinusRadius3 = vec_sub( vec1Sum3, vecRadius );
		vecDPlusRadius4 = vec_add( vec1Sum4, vecRadius );
		vecDMinusRadius4 = vec_sub( vec1Sum4, vecRadius );
		
		// do compare
		vecCmp1 = vec_cmplt( vecDPlusRadius1, zeroVector );
		vecCmp2 = vec_cmplt( vecDMinusRadius1, zeroVector );
		vecCmp3 = vec_cmplt( vecDPlusRadius2, zeroVector );
		vecCmp4 = vec_cmplt( vecDMinusRadius2, zeroVector );
		vecCmp5 = vec_cmplt( vecDPlusRadius3, zeroVector );
		vecCmp6 = vec_cmplt( vecDMinusRadius3, zeroVector );
		vecCmp7 = vec_cmplt( vecDPlusRadius4, zeroVector );
		vecCmp8 = vec_cmplt( vecDMinusRadius4, zeroVector );
		
		//and it with 1 so we multiply by 1 not 1111's
		vecCmp1 = vec_and( vecCmp1, oneIntVector );
		vecCmp2 = vec_and( vecCmp2, oneIntVector );
		vecCmp3 = vec_and( vecCmp3, oneIntVector );
		vecCmp4 = vec_and( vecCmp4, oneIntVector );
		vecCmp5 = vec_and( vecCmp5, oneIntVector );
		vecCmp6 = vec_and( vecCmp6, oneIntVector );
		vecCmp7 = vec_and( vecCmp7, oneIntVector );
		vecCmp8 = vec_and( vecCmp8, oneIntVector );
		
		vecBitShifted1 = vec_sl( (vector unsigned int)vecCmp1, vecShift1 );
		vecBitShifted2 = vec_sl( (vector unsigned int)vecCmp2, vecShift2 );
		vecBitShifted3 = vec_sl( (vector unsigned int)vecCmp3, vecShift1 );
		vecBitShifted4 = vec_sl( (vector unsigned int)vecCmp4, vecShift2 );
		vecBitShifted5 = vec_sl( (vector unsigned int)vecCmp5, vecShift1 );
		vecBitShifted6 = vec_sl( (vector unsigned int)vecCmp6, vecShift2 );
		vecBitShifted7 = vec_sl( (vector unsigned int)vecCmp7, vecShift1 );
		vecBitShifted8 = vec_sl( (vector unsigned int)vecCmp8, vecShift2 );
	
		// OR (add) them all together
		vecBitShifted1 = vec_add( vecBitShifted1, vecBitShifted2 );
		vecBitShifted3 = vec_add( vecBitShifted3, vecBitShifted4 );
		vecBitShifted5 = vec_add( vecBitShifted5, vecBitShifted6 );
		vecBitShifted7 = vec_add( vecBitShifted7, vecBitShifted8 );
		
		vecTotals = vec_add( vecBitShifted1, vec_sld( vecBitShifted1, vecBitShifted1, 8 ) );
		vecTotals = vec_add( vecTotals, vec_sld( vecTotals, vecTotals, 4 ) );
		tempIntSum = vec_add( vecBitShifted3, vec_sld( vecBitShifted3, vecBitShifted3, 8 ) );
		tempIntSum = vec_add( tempIntSum, vec_sld( tempIntSum, tempIntSum, 4 ) );
		vecTotals = vec_mergeh( vecTotals, tempIntSum );
		tempIntSum = vec_add( vecBitShifted5, vec_sld( vecBitShifted5, vecBitShifted5, 8 ) );
		tempIntSum = vec_add( tempIntSum, vec_sld( tempIntSum, tempIntSum, 4 ) );		
		vecTotals = vec_perm( vecTotals, tempIntSum, vecPermHalves );
		tempIntSum = vec_add( vecBitShifted7, vec_sld( vecBitShifted7, vecBitShifted7, 8 ) );
		tempIntSum = vec_add( tempIntSum, vec_sld( tempIntSum, tempIntSum, 4 ) );
		vecTotals = vec_perm( vecTotals, tempIntSum, vecPermLast );
				
		// store out results
		vector unsigned int tempSt = vec_xor( vecTotals, vecFlipBits );
		tempSt = vec_perm( tempSt, tempSt, cullBitPerm );
		vec_ste( tempSt, 0, &cullBitVal[0] );
		vec_ste( tempSt, 4, &cullBitVal[0] );
		vec_ste( tempSt, 8, &cullBitVal[0] );
		vec_ste( tempSt, 12, &cullBitVal[0] );
	
		tOr |= cullBitVal[0];
		tOr |= cullBitVal[1];
		tOr |= cullBitVal[2];
		tOr |= cullBitVal[3];
		
		cullBits[i] = cullBitVal[0];
		cullBits[i+1] = cullBitVal[1];
		cullBits[i+2] = cullBitVal[2];
		cullBits[i+3] = cullBitVal[3];		
	}

	// cleanup
	for ( ; i < numVerts; i++ ) {
		byte bits;
		float d0, d1, d2, d3, t;
		const idVec3 &v = verts[i].xyz;

		d0 = planes[0].Distance( v );
		d1 = planes[1].Distance( v );
		d2 = planes[2].Distance( v );
		d3 = planes[3].Distance( v );

		t = d0 + radius;
		bits  = FLOATSIGNBITSET( t ) << 0;
		t = d1 + radius;
		bits |= FLOATSIGNBITSET( t ) << 1;
		t = d2 + radius;
		bits |= FLOATSIGNBITSET( t ) << 2;
		t = d3 + radius;
		bits |= FLOATSIGNBITSET( t ) << 3;

		t = d0 - radius;
		bits |= FLOATSIGNBITSET( t ) << 4;
		t = d1 - radius;
		bits |= FLOATSIGNBITSET( t ) << 5;
		t = d2 - radius;
		bits |= FLOATSIGNBITSET( t ) << 6;
		t = d3 - radius;
		bits |= FLOATSIGNBITSET( t ) << 7;

		bits ^= 0x0F;		// flip lower four bits

		tOr |= bits;
		cullBits[i] = bits;
	}

	totalOr = tOr;
}

#endif /* DRAWVERT_PADDED */

#ifndef DRAWVERT_PADDED
/*
============
idSIMD_AltiVec::DecalPointCull
============
*/
void VPCALL idSIMD_AltiVec::DecalPointCull( byte *cullBits, const idPlane *planes, const idDrawVert *verts, const int numVerts ) {

	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	
	int i;
	const float *planePtr = planes[0].ToFloatPtr();
		
	vector float vecPlane0, vecPlane1, vecPlane2, vecPlane3, vecPlane4, vecPlane5, vecPlane6, vecPlane7;
	vector float zeroVector = (vector float)(0.0);
	vector unsigned char vecPerm;
	vector float v0, v1, v2, v3, v4, v5, v6, v7;
	
	vecPerm = vec_add( vec_lvsl( -1, planePtr ), (vector unsigned char)(1) );
	
	// populate planes 
	v0 = vec_ld( 0, planePtr );
	v1 = vec_ld( 15, planePtr );
	vecPlane0 = vec_perm( v0, v1, vecPerm );

	v2 = vec_ld( 0, planePtr + 4 );
	v3 = vec_ld( 15, planePtr + 4 );
	vecPlane1 = vec_perm( v2, v3, vecPerm );

	v0 = vec_ld( 0, planePtr + 8 );
	v1 = vec_ld( 15, planePtr + 8 );
	vecPlane2 = vec_perm( v0, v1, vecPerm );

	v2 = vec_ld( 0, planePtr + 12 );
	v3 = vec_ld( 15, planePtr + 12 );
	vecPlane3 = vec_perm( v2, v3, vecPerm );
			
	v0 = vec_ld( 0, planePtr + 16 );
	v1 = vec_ld( 15, planePtr + 16 );
	vecPlane4 = vec_perm( v0, v1, vecPerm );
		
	v2 = vec_ld( 0, planePtr + 20 );
	v3 = vec_ld( 15, planePtr + 20 );
	vecPlane5 = vec_perm( v2, v3, vecPerm );
	
	// transpose
	v0 = vec_mergeh( vecPlane0, vecPlane2 );
	v1 = vec_mergeh( vecPlane1, vecPlane3 );
	v2 = vec_mergel( vecPlane0, vecPlane2 );
	v3 = vec_mergel( vecPlane1, vecPlane3 );

	vecPlane0 = vec_mergeh( v0, v1 );
	vecPlane1 = vec_mergel( v0, v1 );
	vecPlane2 = vec_mergeh( v2, v3 );
	vecPlane3 = vec_mergel( v2, v3 );
	
	v0 = vec_mergeh( vecPlane4, zeroVector );
	v1 = vec_mergeh( vecPlane5, zeroVector );
	v2 = vec_mergel( vecPlane4, zeroVector );
	v3 = vec_mergel( vecPlane5, zeroVector );

	vecPlane4 = vec_mergeh( v0, v1 );
	vecPlane5 = vec_mergel( v0, v1 );
	vecPlane6 = vec_mergeh( v2, v3 );
	vecPlane7 = vec_mergel( v2, v3 );
	
	
	vector float vecXYZ1, vecXYZ2, vecXYZ3, vecXYZ4;
	vector bool int oneIntVector = (vector bool int)(1);
	vector float vec1Sum1, vec1Sum2, vec2Sum1, vec2Sum2, vec3Sum1, vec3Sum2, vec4Sum1, vec4Sum2;
	vector unsigned int vecShift1 = (vector unsigned int)(0, 1, 2, 3 );
	vector unsigned int vecShift2 = (vector unsigned int)(4, 5, 0, 0 );
	
	vector bool int vecCmp1, vecCmp2, vecCmp3, vecCmp4, vecCmp5, vecCmp6, vecCmp7, vecCmp8;
	vector unsigned int vecBitShifted1, vecBitShifted2, vecBitShifted3, vecBitShifted4;
	vector unsigned int vecBitShifted5, vecBitShifted6, vecBitShifted7, vecBitShifted8;
	vector unsigned int vecFlipBits = (vector unsigned int)( 0x3F, 0x3F, 0x3F, 0x3F );
	vector unsigned int vecR1, vecR2, vecR3, vecR4;
	vector unsigned char permHalves = (vector unsigned char)(0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	unsigned int vBits[4];
	vector unsigned char vBitPerm = vec_lvsr( 0, &vBits[4] );
	
	i = 0;
	// every fourth one will have the same alignment. Make sure we've got enough here
	if ( i+3 < numVerts ) {
		vertPerm1 = vec_add( vec_lvsl( -1, (float*) verts[0].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm2 = vec_add( vec_lvsl( -1, (float*) verts[1].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm3 = vec_add( vec_lvsl( -1, (float*) verts[2].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm4 = vec_add( vec_lvsl( -1, (float*) verts[3].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
	}
	

	for ( ; i+3 < numVerts; i+=4 ) {
		const float *vertPtr = verts[i].xyz.ToFloatPtr();
		const float *vertPtr2 = verts[i+1].xyz.ToFloatPtr();
		const float *vertPtr3 = verts[i+2].xyz.ToFloatPtr();
		const float *vertPtr4 = verts[i+3].xyz.ToFloatPtr();
			
		v0 = vec_ld( 0, vertPtr );
		v1 = vec_ld( 15, vertPtr );
		v2 = vec_ld( 0, vertPtr2 );
		v3 = vec_ld( 15, vertPtr2 );
		v4 = vec_ld( 0, vertPtr3 );
		v5 = vec_ld( 15, vertPtr3 );
		v6 = vec_ld( 0, vertPtr4 );
		v7 = vec_ld( 15, vertPtr4 );
			
		vecXYZ1 = vec_perm( v0, v1, vertPerm1 );
		vecXYZ2 = vec_perm( v2, v3, vertPerm2 );
		vecXYZ3 = vec_perm( v4, v5, vertPerm3 );
		vecXYZ4 = vec_perm( v6, v7, vertPerm4 );
		
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 0 ), vecPlane0, zeroVector );
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 1 ), vecPlane1, vec1Sum1 );
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 2 ), vecPlane2, vec1Sum1 );
		vec1Sum1 = vec_add( vec1Sum1, vecPlane3 );
		
		vec1Sum2 = vec_madd( vec_splat( vecXYZ1, 0 ), vecPlane4, zeroVector );
		vec1Sum2 = vec_madd( vec_splat( vecXYZ1, 1 ), vecPlane5, vec1Sum2 );
	    vec1Sum2 = vec_madd( vec_splat( vecXYZ1, 2 ), vecPlane6, vec1Sum2 );
		vec1Sum2 = vec_add( vec1Sum2, vecPlane7 );
		
		vec2Sum1 = vec_madd( vec_splat( vecXYZ2, 0 ), vecPlane0, zeroVector );
		vec2Sum1 = vec_madd( vec_splat( vecXYZ2, 1 ), vecPlane1, vec2Sum1 );
		vec2Sum1 = vec_madd( vec_splat( vecXYZ2, 2 ), vecPlane2, vec2Sum1 );
		vec2Sum1 = vec_add( vec2Sum1, vecPlane3 );		
		
		vec2Sum2 = vec_madd( vec_splat( vecXYZ2, 0 ), vecPlane4, zeroVector );
		vec2Sum2 = vec_madd( vec_splat( vecXYZ2, 1 ), vecPlane5, vec2Sum2 );
	    vec2Sum2 = vec_madd( vec_splat( vecXYZ2, 2 ), vecPlane6, vec2Sum2 );
		vec2Sum2 = vec_add( vec2Sum2, vecPlane7 );
				
		vec3Sum1 = vec_madd( vec_splat( vecXYZ3, 0 ), vecPlane0, zeroVector );
		vec3Sum1 = vec_madd( vec_splat( vecXYZ3, 1 ), vecPlane1, vec3Sum1 );
		vec3Sum1 = vec_madd( vec_splat( vecXYZ3, 2 ), vecPlane2, vec3Sum1 );
		vec3Sum1 = vec_add( vec3Sum1, vecPlane3 );
		
		vec3Sum2 = vec_madd( vec_splat( vecXYZ3, 0 ), vecPlane4, zeroVector );
		vec3Sum2 = vec_madd( vec_splat( vecXYZ3, 1 ), vecPlane5, vec3Sum2 );		
	    vec3Sum2 = vec_madd( vec_splat( vecXYZ3, 2 ), vecPlane6, vec3Sum2 );
		vec3Sum2 = vec_add( vec3Sum2, vecPlane7 );	
							
		vec4Sum1 = vec_madd( vec_splat( vecXYZ4, 0 ), vecPlane0, zeroVector );
		vec4Sum1 = vec_madd( vec_splat( vecXYZ4, 1 ), vecPlane1, vec4Sum1 );
		vec4Sum1 = vec_madd( vec_splat( vecXYZ4, 2 ), vecPlane2, vec4Sum1 );
		vec4Sum1 = vec_add( vec4Sum1, vecPlane3 );				
		
		vec4Sum2 = vec_madd( vec_splat( vecXYZ4, 0 ), vecPlane4, zeroVector );
		vec4Sum2 = vec_madd( vec_splat( vecXYZ4, 1 ), vecPlane5, vec4Sum2 );		
	    vec4Sum2 = vec_madd( vec_splat( vecXYZ4, 2 ), vecPlane6, vec4Sum2 );
		vec4Sum2 = vec_add( vec4Sum2, vecPlane7 );

		vecCmp1 = vec_cmplt( vec1Sum1, zeroVector );
		vecCmp2 = vec_cmplt( vec1Sum2, zeroVector );
		vecCmp3 = vec_cmplt( vec2Sum1, zeroVector );
		vecCmp4 = vec_cmplt( vec2Sum2, zeroVector );
		vecCmp5 = vec_cmplt( vec3Sum1, zeroVector );
		vecCmp6 = vec_cmplt( vec3Sum2, zeroVector );
		vecCmp7 = vec_cmplt( vec4Sum1, zeroVector );
		vecCmp8 = vec_cmplt( vec4Sum2, zeroVector );
		
		//and it with 1 so we multiply by 1 not 1111's
		vecCmp1 = vec_and( vecCmp1, oneIntVector );
		vecCmp2 = vec_and( vecCmp2, oneIntVector );
		vecCmp3 = vec_and( vecCmp3, oneIntVector );
		vecCmp4 = vec_and( vecCmp4, oneIntVector );
		vecCmp5 = vec_and( vecCmp5, oneIntVector );
		vecCmp6 = vec_and( vecCmp6, oneIntVector );
		vecCmp7 = vec_and( vecCmp7, oneIntVector );
		vecCmp8 = vec_and( vecCmp8, oneIntVector );

		vecBitShifted1 = vec_sl( (vector unsigned int)vecCmp1, vecShift1 );
		vecBitShifted2 = vec_sl( (vector unsigned int)vecCmp2, vecShift2 );
		vecBitShifted3 = vec_sl( (vector unsigned int)vecCmp3, vecShift1 );
		vecBitShifted4 = vec_sl( (vector unsigned int)vecCmp4, vecShift2 );
		vecBitShifted5 = vec_sl( (vector unsigned int)vecCmp5, vecShift1 );
		vecBitShifted6 = vec_sl( (vector unsigned int)vecCmp6, vecShift2 );
		vecBitShifted7 = vec_sl( (vector unsigned int)vecCmp7, vecShift1 );
		vecBitShifted8 = vec_sl( (vector unsigned int)vecCmp8, vecShift2 );
	
		//OR them all together (this is the same as adding them, since they're all only 1 bit set)		
		vecR1 = (vector unsigned int)(0); //zeroIntVector;
		vecR1 = vec_add( vecBitShifted1, vec_sld( vecBitShifted1, vecBitShifted1, 8 ) );
		vecR1 = vec_add( vecR1, vec_sld( vecR1, vecR1, 4 ) );	
		vecR1 = vec_add(vecR1, vecBitShifted2 );
		vecR1 = vec_or( vecR1, vec_sld( vecBitShifted2, vecBitShifted2, 4 ) );
		
		vecR2 = (vector unsigned int)(0); //zeroIntVector;
		vecR2 = vec_add( vecBitShifted3, vec_sld( vecBitShifted3, vecBitShifted3, 8 ) );
		vecR2 = vec_add( vecR2, vec_sld( vecR2, vecR2, 4 ) );	
		vecR2 = vec_add(vecR2, vecBitShifted4 );
		vecR2 = vec_or( vecR2, vec_sld( vecBitShifted4, vecBitShifted4, 4 ) );
		
		vecR3 = (vector unsigned int)(0); //zeroIntVector;
		vecR3 = vec_add( vecBitShifted5, vec_sld( vecBitShifted5, vecBitShifted5, 8 ) );
		vecR3 = vec_add( vecR3, vec_sld( vecR3, vecR3, 4 ) );	
		vecR3 = vec_add(vecR3, vecBitShifted6 );
		vecR3 = vec_or( vecR3, vec_sld( vecBitShifted6, vecBitShifted6, 4 ) );
		
		vecR4 = (vector unsigned int)(0); //zeroIntVector;
		vecR4 = vec_add( vecBitShifted7, vec_sld( vecBitShifted7, vecBitShifted7, 8 ) );
		vecR4 = vec_add( vecR4, vec_sld( vecR4, vecR4, 4 ) );	
		vecR4 = vec_add(vecR4, vecBitShifted8 );
		vecR4 = vec_or( vecR4, vec_sld( vecBitShifted8, vecBitShifted8, 4 ) );
		
		// take the first element from each vector and put them into vecR1
		vecR1 = vec_mergeh( vecR1, vecR2 );
		vecR3 = vec_mergeh( vecR3, vecR4 );
		vecR1 = vec_perm( vecR1, vecR3, permHalves );

		// XOR with 0x3F to flip lower 6 bits
		vecR1 = vec_xor( vecR1, vecFlipBits );
		
		// store out results. don't have 16 at a time so let's just
		// do this and avoid alignment concerns
		vecR1 = vec_perm( vecR1, vecR1, vBitPerm );
		vec_ste( vecR1, 0, &vBits[0] );
		vec_ste( vecR1, 4, &vBits[0] );
		vec_ste( vecR1, 8, &vBits[0] );
		vec_ste( vecR1, 12, &vBits[0] );
		
		cullBits[i] = vBits[0];
		cullBits[i+1] = vBits[1];
		cullBits[i+2] = vBits[2];
		cullBits[i+3] = vBits[3];
	}
	
	for ( ; i < numVerts; i++ ) {
		byte bits;
		float d0, d1, d2, d3, d4, d5;
		const idVec3 &v = verts[i].xyz;

		d0 = planes[0].Distance( v );
		d1 = planes[1].Distance( v );
		d2 = planes[2].Distance( v );
		d3 = planes[3].Distance( v );
		d4 = planes[4].Distance( v );
		d5 = planes[5].Distance( v );

		// they check if the sign bit is set by casting as long and shifting right 31 places. 
		bits  = FLOATSIGNBITSET( d0 ) << 0;
		bits |= FLOATSIGNBITSET( d1 ) << 1;
		bits |= FLOATSIGNBITSET( d2 ) << 2;
		bits |= FLOATSIGNBITSET( d3 ) << 3;
		bits |= FLOATSIGNBITSET( d4 ) << 4;
		bits |= FLOATSIGNBITSET( d5 ) << 5;

		cullBits[i] = bits ^ 0x3F;		// flip lower 6 bits
	}
}

#else

/*
============
idSIMD_AltiVec::DecalPointCull
============
*/
void VPCALL idSIMD_AltiVec::DecalPointCull( byte *cullBits, const idPlane *planes, const idDrawVert *verts, const int numVerts ) {

	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	
	int i;
	const float *planePtr = planes[0].ToFloatPtr();
		
	vector float vecPlane0, vecPlane1, vecPlane2, vecPlane3, vecPlane4, vecPlane5, vecPlane6, vecPlane7;
	vector float zeroVector = (vector float)(0.0);
	vector unsigned char vecPerm;
	vector float v0, v1, v2, v3, v4, v5, v6, v7;
	
	vecPerm = vec_add( vec_lvsl( -1, planePtr ), (vector unsigned char)(1) );
	
	// populate planes 
	v0 = vec_ld( 0, planePtr );
	v1 = vec_ld( 15, planePtr );
	vecPlane0 = vec_perm( v0, v1, vecPerm );

	v2 = vec_ld( 0, planePtr + 4 );
	v3 = vec_ld( 15, planePtr + 4 );
	vecPlane1 = vec_perm( v2, v3, vecPerm );

	v0 = vec_ld( 0, planePtr + 8 );
	v1 = vec_ld( 15, planePtr + 8 );
	vecPlane2 = vec_perm( v0, v1, vecPerm );

	v2 = vec_ld( 0, planePtr + 12 );
	v3 = vec_ld( 15, planePtr + 12 );
	vecPlane3 = vec_perm( v2, v3, vecPerm );
			
	v0 = vec_ld( 0, planePtr + 16 );
	v1 = vec_ld( 15, planePtr + 16 );
	vecPlane4 = vec_perm( v0, v1, vecPerm );
		
	v2 = vec_ld( 0, planePtr + 20 );
	v3 = vec_ld( 15, planePtr + 20 );
	vecPlane5 = vec_perm( v2, v3, vecPerm );
	
	// transpose
	v0 = vec_mergeh( vecPlane0, vecPlane2 );
	v1 = vec_mergeh( vecPlane1, vecPlane3 );
	v2 = vec_mergel( vecPlane0, vecPlane2 );
	v3 = vec_mergel( vecPlane1, vecPlane3 );

	vecPlane0 = vec_mergeh( v0, v1 );
	vecPlane1 = vec_mergel( v0, v1 );
	vecPlane2 = vec_mergeh( v2, v3 );
	vecPlane3 = vec_mergel( v2, v3 );
	
	v0 = vec_mergeh( vecPlane4, zeroVector );
	v1 = vec_mergeh( vecPlane5, zeroVector );
	v2 = vec_mergel( vecPlane4, zeroVector );
	v3 = vec_mergel( vecPlane5, zeroVector );

	vecPlane4 = vec_mergeh( v0, v1 );
	vecPlane5 = vec_mergel( v0, v1 );
	vecPlane6 = vec_mergeh( v2, v3 );
	vecPlane7 = vec_mergel( v2, v3 );
	
	
	vector float vecXYZ1, vecXYZ2, vecXYZ3, vecXYZ4;
	vector bool int oneIntVector = (vector bool int)(1);
	vector float vec1Sum1, vec1Sum2, vec2Sum1, vec2Sum2, vec3Sum1, vec3Sum2, vec4Sum1, vec4Sum2;
	vector unsigned int vecShift1 = (vector unsigned int)(0, 1, 2, 3 );
	vector unsigned int vecShift2 = (vector unsigned int)(4, 5, 0, 0 );
	
	vector bool int vecCmp1, vecCmp2, vecCmp3, vecCmp4, vecCmp5, vecCmp6, vecCmp7, vecCmp8;
	vector unsigned int vecBitShifted1, vecBitShifted2, vecBitShifted3, vecBitShifted4;
	vector unsigned int vecBitShifted5, vecBitShifted6, vecBitShifted7, vecBitShifted8;
	vector unsigned int vecFlipBits = (vector unsigned int)( 0x3F, 0x3F, 0x3F, 0x3F );
	vector unsigned int vecR1, vecR2, vecR3, vecR4;
	vector unsigned char permHalves = (vector unsigned char)(0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	unsigned int vBits[4];
	vector unsigned char vBitPerm = vec_lvsr( 0, &vBits[4] );
	
	i = 0;
	
	for ( ; i+3 < numVerts; i+=4 ) {
		const float *vertPtr = verts[i].xyz.ToFloatPtr();
		const float *vertPtr2 = verts[i+1].xyz.ToFloatPtr();
		const float *vertPtr3 = verts[i+2].xyz.ToFloatPtr();
		const float *vertPtr4 = verts[i+3].xyz.ToFloatPtr();
			
		v0 = vec_ld( 0, vertPtr );
		v2 = vec_ld( 0, vertPtr2 );
		v4 = vec_ld( 0, vertPtr3 );
		v6 = vec_ld( 0, vertPtr4 );
		
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 0 ), vecPlane0, zeroVector );
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 1 ), vecPlane1, vec1Sum1 );
		vec1Sum1 = vec_madd( vec_splat( vecXYZ1, 2 ), vecPlane2, vec1Sum1 );
		vec1Sum1 = vec_add( vec1Sum1, vecPlane3 );
		
		vec1Sum2 = vec_madd( vec_splat( vecXYZ1, 0 ), vecPlane4, zeroVector );
		vec1Sum2 = vec_madd( vec_splat( vecXYZ1, 1 ), vecPlane5, vec1Sum2 );
	    vec1Sum2 = vec_madd( vec_splat( vecXYZ1, 2 ), vecPlane6, vec1Sum2 );
		vec1Sum2 = vec_add( vec1Sum2, vecPlane7 );
		
		vec2Sum1 = vec_madd( vec_splat( vecXYZ2, 0 ), vecPlane0, zeroVector );
		vec2Sum1 = vec_madd( vec_splat( vecXYZ2, 1 ), vecPlane1, vec2Sum1 );
		vec2Sum1 = vec_madd( vec_splat( vecXYZ2, 2 ), vecPlane2, vec2Sum1 );
		vec2Sum1 = vec_add( vec2Sum1, vecPlane3 );		
		
		vec2Sum2 = vec_madd( vec_splat( vecXYZ2, 0 ), vecPlane4, zeroVector );
		vec2Sum2 = vec_madd( vec_splat( vecXYZ2, 1 ), vecPlane5, vec2Sum2 );
	    vec2Sum2 = vec_madd( vec_splat( vecXYZ2, 2 ), vecPlane6, vec2Sum2 );
		vec2Sum2 = vec_add( vec2Sum2, vecPlane7 );
				
		vec3Sum1 = vec_madd( vec_splat( vecXYZ3, 0 ), vecPlane0, zeroVector );
		vec3Sum1 = vec_madd( vec_splat( vecXYZ3, 1 ), vecPlane1, vec3Sum1 );
		vec3Sum1 = vec_madd( vec_splat( vecXYZ3, 2 ), vecPlane2, vec3Sum1 );
		vec3Sum1 = vec_add( vec3Sum1, vecPlane3 );
		
		vec3Sum2 = vec_madd( vec_splat( vecXYZ3, 0 ), vecPlane4, zeroVector );
		vec3Sum2 = vec_madd( vec_splat( vecXYZ3, 1 ), vecPlane5, vec3Sum2 );		
	    vec3Sum2 = vec_madd( vec_splat( vecXYZ3, 2 ), vecPlane6, vec3Sum2 );
		vec3Sum2 = vec_add( vec3Sum2, vecPlane7 );	
							
		vec4Sum1 = vec_madd( vec_splat( vecXYZ4, 0 ), vecPlane0, zeroVector );
		vec4Sum1 = vec_madd( vec_splat( vecXYZ4, 1 ), vecPlane1, vec4Sum1 );
		vec4Sum1 = vec_madd( vec_splat( vecXYZ4, 2 ), vecPlane2, vec4Sum1 );
		vec4Sum1 = vec_add( vec4Sum1, vecPlane3 );				
		
		vec4Sum2 = vec_madd( vec_splat( vecXYZ4, 0 ), vecPlane4, zeroVector );
		vec4Sum2 = vec_madd( vec_splat( vecXYZ4, 1 ), vecPlane5, vec4Sum2 );		
	    vec4Sum2 = vec_madd( vec_splat( vecXYZ4, 2 ), vecPlane6, vec4Sum2 );
		vec4Sum2 = vec_add( vec4Sum2, vecPlane7 );

		vecCmp1 = vec_cmplt( vec1Sum1, zeroVector );
		vecCmp2 = vec_cmplt( vec1Sum2, zeroVector );
		vecCmp3 = vec_cmplt( vec2Sum1, zeroVector );
		vecCmp4 = vec_cmplt( vec2Sum2, zeroVector );
		vecCmp5 = vec_cmplt( vec3Sum1, zeroVector );
		vecCmp6 = vec_cmplt( vec3Sum2, zeroVector );
		vecCmp7 = vec_cmplt( vec4Sum1, zeroVector );
		vecCmp8 = vec_cmplt( vec4Sum2, zeroVector );
		
		//and it with 1 so we multiply by 1 not 1111's
		vecCmp1 = vec_and( vecCmp1, oneIntVector );
		vecCmp2 = vec_and( vecCmp2, oneIntVector );
		vecCmp3 = vec_and( vecCmp3, oneIntVector );
		vecCmp4 = vec_and( vecCmp4, oneIntVector );
		vecCmp5 = vec_and( vecCmp5, oneIntVector );
		vecCmp6 = vec_and( vecCmp6, oneIntVector );
		vecCmp7 = vec_and( vecCmp7, oneIntVector );
		vecCmp8 = vec_and( vecCmp8, oneIntVector );

		vecBitShifted1 = vec_sl( (vector unsigned int)vecCmp1, vecShift1 );
		vecBitShifted2 = vec_sl( (vector unsigned int)vecCmp2, vecShift2 );
		vecBitShifted3 = vec_sl( (vector unsigned int)vecCmp3, vecShift1 );
		vecBitShifted4 = vec_sl( (vector unsigned int)vecCmp4, vecShift2 );
		vecBitShifted5 = vec_sl( (vector unsigned int)vecCmp5, vecShift1 );
		vecBitShifted6 = vec_sl( (vector unsigned int)vecCmp6, vecShift2 );
		vecBitShifted7 = vec_sl( (vector unsigned int)vecCmp7, vecShift1 );
		vecBitShifted8 = vec_sl( (vector unsigned int)vecCmp8, vecShift2 );
	
		//OR them all together (this is the same as adding them, since they're all only 1 bit set)		
		vecR1 = (vector unsigned int)(0); //zeroIntVector;
		vecR1 = vec_add( vecBitShifted1, vec_sld( vecBitShifted1, vecBitShifted1, 8 ) );
		vecR1 = vec_add( vecR1, vec_sld( vecR1, vecR1, 4 ) );	
		vecR1 = vec_add(vecR1, vecBitShifted2 );
		vecR1 = vec_or( vecR1, vec_sld( vecBitShifted2, vecBitShifted2, 4 ) );
		
		vecR2 = (vector unsigned int)(0); //zeroIntVector;
		vecR2 = vec_add( vecBitShifted3, vec_sld( vecBitShifted3, vecBitShifted3, 8 ) );
		vecR2 = vec_add( vecR2, vec_sld( vecR2, vecR2, 4 ) );	
		vecR2 = vec_add(vecR2, vecBitShifted4 );
		vecR2 = vec_or( vecR2, vec_sld( vecBitShifted4, vecBitShifted4, 4 ) );
		
		vecR3 = (vector unsigned int)(0); //zeroIntVector;
		vecR3 = vec_add( vecBitShifted5, vec_sld( vecBitShifted5, vecBitShifted5, 8 ) );
		vecR3 = vec_add( vecR3, vec_sld( vecR3, vecR3, 4 ) );	
		vecR3 = vec_add(vecR3, vecBitShifted6 );
		vecR3 = vec_or( vecR3, vec_sld( vecBitShifted6, vecBitShifted6, 4 ) );
		
		vecR4 = (vector unsigned int)(0); //zeroIntVector;
		vecR4 = vec_add( vecBitShifted7, vec_sld( vecBitShifted7, vecBitShifted7, 8 ) );
		vecR4 = vec_add( vecR4, vec_sld( vecR4, vecR4, 4 ) );	
		vecR4 = vec_add(vecR4, vecBitShifted8 );
		vecR4 = vec_or( vecR4, vec_sld( vecBitShifted8, vecBitShifted8, 4 ) );
		
		// take the first element from each vector and put them into vecR1
		vecR1 = vec_mergeh( vecR1, vecR2 );
		vecR3 = vec_mergeh( vecR3, vecR4 );
		vecR1 = vec_perm( vecR1, vecR3, permHalves );

		// XOR with 0x3F to flip lower 6 bits
		vecR1 = vec_xor( vecR1, vecFlipBits );
		
		// store out results. don't have 16 at a time so let's just
		// do this and avoid alignment concerns
		vecR1 = vec_perm( vecR1, vecR1, vBitPerm );
		vec_ste( vecR1, 0, &vBits[0] );
		vec_ste( vecR1, 4, &vBits[0] );
		vec_ste( vecR1, 8, &vBits[0] );
		vec_ste( vecR1, 12, &vBits[0] );
		
		cullBits[i] = vBits[0];
		cullBits[i+1] = vBits[1];
		cullBits[i+2] = vBits[2];
		cullBits[i+3] = vBits[3];
	}
	
	for ( ; i < numVerts; i++ ) {
		byte bits;
		float d0, d1, d2, d3, d4, d5;
		const idVec3 &v = verts[i].xyz;

		d0 = planes[0].Distance( v );
		d1 = planes[1].Distance( v );
		d2 = planes[2].Distance( v );
		d3 = planes[3].Distance( v );
		d4 = planes[4].Distance( v );
		d5 = planes[5].Distance( v );

		// they check if the sign bit is set by casting as long and shifting right 31 places. 
		bits  = FLOATSIGNBITSET( d0 ) << 0;
		bits |= FLOATSIGNBITSET( d1 ) << 1;
		bits |= FLOATSIGNBITSET( d2 ) << 2;
		bits |= FLOATSIGNBITSET( d3 ) << 3;
		bits |= FLOATSIGNBITSET( d4 ) << 4;
		bits |= FLOATSIGNBITSET( d5 ) << 5;

		cullBits[i] = bits ^ 0x3F;		// flip lower 6 bits
	}
}


#endif /*DRAWVERT_PADDED */

#ifndef DRAWVERT_PADDED
/*
============
idSIMD_AltiVec::OverlayPointCull
============
*/
void VPCALL idSIMD_AltiVec::OverlayPointCull( byte *cullBits, idVec2 *texCoords, const idPlane *planes, const idDrawVert *verts, const int numVerts ) {
	
	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	
	int i;

	float p0x, p0y, p0z, p0d;
	float p1x, p1y, p1z, p1d;
	
	const float *planePtr = planes[0].ToFloatPtr();
	const float *vertPtr = verts[0].xyz.ToFloatPtr();
	
	vector float vecPlane0, vecPlane1, vecPlane2, vecPlane3;
	vector float v0, v1, v2, v3, v4, v5, v6, v7;
	vector unsigned char vecPerm;
	vector float zeroVector = (vector float)(0);
	
	p0x = *(planePtr + 0);
	p0y = *(planePtr + 1);
	p0z = *(planePtr + 2);
	p0d = *(planePtr + 3);
	p1x = *(planePtr + 4);
	p1y = *(planePtr + 5);
	p1z = *(planePtr + 6);
	p1d = *(planePtr + 7);
	
	// populate the planes
	vecPerm = vec_add( vec_lvsl( -1, planePtr ), (vector unsigned char)(1) );
	v0 = vec_ld( 0, planePtr );
	v1 = vec_ld( 15, planePtr );
	vecPlane0 = vec_perm( v0, v1, vecPerm );
	
	v2 = vec_ld( 31, planePtr );
	vecPlane1 = vec_perm( v1, v2, vecPerm );
	
	// transpose
	v0 = vec_mergeh( vecPlane0, vecPlane0 );
	v1 = vec_mergeh( vecPlane1, vecPlane1 );
	v2 = vec_mergel( vecPlane0, vecPlane0 );
	v3 = vec_mergel( vecPlane1, vecPlane1);

	vecPlane0 = vec_mergeh( v0, v1 );
	vecPlane1 = vec_mergel( v0, v1 );
	vecPlane2 = vec_mergeh( v2, v3 );
	vecPlane3 = vec_mergel( v2, v3 );

	vector float vecXYZ1, vecXYZ2, vecXYZ3, vecXYZ4;
	vector float oneVector = (vector float)(1);

	vector float vecSum1, vecSum2, vecSum1Inv,vecSum2Inv;

	vector bool int vecCmp1, vecCmp2, vecCmp1Inv, vecCmp2Inv;
	vector float negTwoVector = (vector float)(-2);
	vector unsigned int vecBitShifted1, vecBitShifted2, vecBitShifted1Inv, vecBitShifted2Inv;
	vector unsigned int vecShift = (vector unsigned int)( 0, 1, 0, 1 );
	vector unsigned int vecShiftInv = (vector unsigned int)( 2, 3, 2, 3 );
	vector unsigned char vecPermFirstThird = (vector unsigned char)(0,1,2,3,8,9,10,11,16,17,18,19,24,25,26,27);
	vector bool int oneIntVector = (vector bool int)(1);
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	unsigned int cullBitVal[4];
	vector unsigned char cullBitPerm = vec_lvsr( 0, &cullBitVal[0] );

	i = 0;
	// every fourth one will have the same alignment. Make sure we've got enough here
	if ( i+3 < numVerts ) {
		vertPerm1 = vec_add( vec_lvsl( -1, (float*) verts[0].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm2 = vec_add( vec_lvsl( -1, (float*) verts[1].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm3 = vec_add( vec_lvsl( -1, (float*) verts[2].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm4 = vec_add( vec_lvsl( -1, (float*) verts[3].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
	}
	

	for ( ; i+3 < numVerts; i+=4 ) {
		const float *vertPtr = verts[i].xyz.ToFloatPtr();
		const float *vertPtr2 = verts[i+1].xyz.ToFloatPtr();
		const float *vertPtr3 = verts[i+2].xyz.ToFloatPtr();
		const float *vertPtr4 = verts[i+3].xyz.ToFloatPtr();
			
		v0 = vec_ld( 0, vertPtr );
		v1 = vec_ld( 15, vertPtr );
		v2 = vec_ld( 0, vertPtr2 );
		v3 = vec_ld( 15, vertPtr2 );
		v4 = vec_ld( 0, vertPtr3 );
		v5 = vec_ld( 15, vertPtr3 );
		v6 = vec_ld( 0, vertPtr4 );
		v7 = vec_ld( 15, vertPtr4 );
			
		vecXYZ1 = vec_perm( v0, v1, vertPerm1 );
		vecXYZ2 = vec_perm( v2, v3, vertPerm2 );
		vecXYZ3 = vec_perm( v4, v5, vertPerm3 );
		vecXYZ4 = vec_perm( v6, v7, vertPerm4 );

		// like a splat, but only doing halves
		vecSum1 = vec_madd( vec_perm( vecXYZ1, vecXYZ2, (vector unsigned char)(0,1,2,3,0,1,2,3,16,17,18,19,16,17,18,19) ), vecPlane0, zeroVector );
		vecSum1 = vec_madd( vec_perm( vecXYZ1, vecXYZ2, (vector unsigned char)(4,5,6,7,4,5,6,7,20,21,22,23,20,21,22,23) ) , vecPlane1, vecSum1 );
		vecSum1 = vec_madd( vec_perm( vecXYZ1, vecXYZ2, (vector unsigned char)(8,9,10,11,8,9,10,11,24,25,26,27,24,25,26,27) ), vecPlane2, vecSum1 );
		vecSum1 = vec_add( vecSum1, vecPlane3 );

		vecSum2 = vec_madd( vec_perm( vecXYZ3, vecXYZ4, (vector unsigned char)(0,1,2,3,0,1,2,3,16,17,18,19,16,17,18,19) ), vecPlane0, zeroVector );
		vecSum2 = vec_madd( vec_perm( vecXYZ3, vecXYZ4, (vector unsigned char)(4,5,6,7,4,5,6,7,20,21,22,23,20,21,22,23) ) , vecPlane1, vecSum2 );
		vecSum2 = vec_madd( vec_perm( vecXYZ3, vecXYZ4, (vector unsigned char)(8,9,10,11,8,9,10,11,24,25,26,27,24,25,26,27) ), vecPlane2, vecSum2 );
		vecSum2 = vec_add( vecSum2, vecPlane3 );

		// store out results
		UNALIGNED_STORE2( &texCoords[i][0], vecSum1, vecSum2 );

		// bit manipulation
		vecCmp1 = vec_cmplt( vecSum1, zeroVector );
		vecCmp2 = vec_cmplt( vecSum2, zeroVector );
		
		//and it with 1 so we multiply by 1 not 1111's
		vecCmp1 = vec_and( vecCmp1, oneIntVector );
		vecCmp2 = vec_and( vecCmp2, oneIntVector );		
 		
		 // store out and write to cullBits 
		// finally, a use for algebra! 1-x = x + 1 - 2x
		vecSum1Inv = vec_madd( vecSum1, negTwoVector, vecSum1 );
		vecSum2Inv = vec_madd( vecSum2, negTwoVector, vecSum2 );
		vecSum1Inv = vec_add( vecSum1Inv, oneVector );
		vecSum2Inv = vec_add( vecSum2Inv, oneVector );
		
		// do the same comparisons for the inverted d0/d1
		vecCmp1Inv = vec_cmplt( vecSum1Inv, zeroVector );
		vecCmp2Inv = vec_cmplt( vecSum2Inv, zeroVector );
		
		//and it with 1 so we multiply by 1 not 1111's
		vecCmp1Inv = vec_and( vecCmp1Inv, oneIntVector );
		vecCmp2Inv = vec_and( vecCmp2Inv, oneIntVector );

		// shift them as needed
		vecBitShifted1 = vec_sl( (vector unsigned int)vecCmp1, vecShift );
		vecBitShifted2 = vec_sl( (vector unsigned int)vecCmp2, vecShift );
		vecBitShifted1Inv = vec_sl( (vector unsigned int)vecCmp1Inv, vecShiftInv );
		vecBitShifted2Inv = vec_sl( (vector unsigned int)vecCmp2Inv, vecShiftInv );

		// OR them all together. since only 1 bit is set for each value, thats
		// the same as adding them. add up d0 + d1 + d0Inv + d1Inv
		vector unsigned int vecResult;
		vector unsigned int vecResult2;
		vector unsigned int vecResult3;
		vecResult = vec_add( vecBitShifted1, vec_sld( vecBitShifted1, vecBitShifted1, 4 ) );
		
		vecResult2 = vec_add( vecBitShifted2, vec_sld( vecBitShifted2, vecBitShifted2, 4 ) );
		
		// vecResult now holds the values without the inverses yet, so add those
		vecResult = vec_perm( vecResult, vecResult2, vecPermFirstThird );			
		vecResult2 = vec_add( vecBitShifted1Inv, vec_sld( vecBitShifted1Inv, vecBitShifted1Inv, 4 ) );
		vecResult3 = vec_add( vecBitShifted2Inv, vec_sld( vecBitShifted2Inv, vecBitShifted2Inv, 4 ) );
		vecResult2 = vec_perm( vecResult2, vecResult3, vecPermFirstThird );
		
		vecResult = vec_add( vecResult, vecResult2 );
		
		//store out results
		vecResult = vec_perm( vecResult, vecResult, cullBitPerm );
		vec_ste( vecResult, 0, &cullBitVal[0] );
		vec_ste( vecResult, 4, &cullBitVal[0] );
		vec_ste( vecResult, 8, &cullBitVal[0] );
		vec_ste( vecResult, 12, &cullBitVal[0] );
			
		cullBits[i] = cullBitVal[0];
		cullBits[i+1] = cullBitVal[1];
		cullBits[i+2] = cullBitVal[2];
		cullBits[i+3] = cullBitVal[3];
	}

	// cleanup
	for ( ; i < numVerts; i++ ) {
		byte bits;
		float d0, d1;
		float vx, vy, vz;
			
		vx = *( vertPtr + (i*DRAWVERT_OFFSET) + 0 );
		vy = *( vertPtr + (i*DRAWVERT_OFFSET) + 1 );
		vz = *( vertPtr + (i*DRAWVERT_OFFSET) + 2 );
		
		d0 = p0x * vx + p0y * vy + p0z * vz + p0d;
		d1 = p1x * vx + p1y * vy + p1z * vz + p1d;
		texCoords[i][0] = d0;
		texCoords[i][1] = d1;
		
		bits = ( d0 >= 0 ) ? 0 : 1;
		d0 = 1.0f - d0;
		bits |= ( d1 >= 0 ) ? 0 : 1*2; 
		d1 = 1.0f - d1;
		
		bits |= ( d0 >= 0 ) ? 0: 1*4;
		bits |= ( d1 >= 0 ) ? 0: 1*8;

		cullBits[i] = bits;
	}
}
#else

/*
============
idSIMD_AltiVec::OverlayPointCull
============
*/
void VPCALL idSIMD_AltiVec::OverlayPointCull( byte *cullBits, idVec2 *texCoords, const idPlane *planes, const idDrawVert *verts, const int numVerts ) {
	
	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	
	int i;

	float p0x, p0y, p0z, p0d;
	float p1x, p1y, p1z, p1d;
	
	const float *planePtr = planes[0].ToFloatPtr();
	const float *vertPtr = verts[0].xyz.ToFloatPtr();
	
	vector float vecPlane0, vecPlane1, vecPlane2, vecPlane3;
	vector float v0, v1, v2, v3, v4, v5, v6, v7;
	vector unsigned char vecPerm;
	vector float zeroVector = (vector float)(0);
	
	p0x = *(planePtr + 0);
	p0y = *(planePtr + 1);
	p0z = *(planePtr + 2);
	p0d = *(planePtr + 3);
	p1x = *(planePtr + 4);
	p1y = *(planePtr + 5);
	p1z = *(planePtr + 6);
	p1d = *(planePtr + 7);
	
	// populate the planes
	vecPerm = vec_add( vec_lvsl( -1, planePtr ), (vector unsigned char)(1) );
	v0 = vec_ld( 0, planePtr );
	v1 = vec_ld( 15, planePtr );
	vecPlane0 = vec_perm( v0, v1, vecPerm );
	
	v2 = vec_ld( 31, planePtr );
	vecPlane1 = vec_perm( v1, v2, vecPerm );
	
	// transpose
	v0 = vec_mergeh( vecPlane0, vecPlane0 );
	v1 = vec_mergeh( vecPlane1, vecPlane1 );
	v2 = vec_mergel( vecPlane0, vecPlane0 );
	v3 = vec_mergel( vecPlane1, vecPlane1);

	vecPlane0 = vec_mergeh( v0, v1 );
	vecPlane1 = vec_mergel( v0, v1 );
	vecPlane2 = vec_mergeh( v2, v3 );
	vecPlane3 = vec_mergel( v2, v3 );

	vector float vecXYZ1, vecXYZ2, vecXYZ3, vecXYZ4;
	vector float oneVector = (vector float)(1);

	vector float vecSum1, vecSum2, vecSum1Inv,vecSum2Inv;

	vector bool int vecCmp1, vecCmp2, vecCmp1Inv, vecCmp2Inv;
	vector float negTwoVector = (vector float)(-2);
	vector unsigned int vecBitShifted1, vecBitShifted2, vecBitShifted1Inv, vecBitShifted2Inv;
	vector unsigned int vecShift = (vector unsigned int)( 0, 1, 0, 1 );
	vector unsigned int vecShiftInv = (vector unsigned int)( 2, 3, 2, 3 );
	vector unsigned char vecPermFirstThird = (vector unsigned char)(0,1,2,3,8,9,10,11,16,17,18,19,24,25,26,27);
	vector bool int oneIntVector = (vector bool int)(1);
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	unsigned int cullBitVal[4];
	vector unsigned char cullBitPerm = vec_lvsr( 0, &cullBitVal[0] );

	i = 0;

	for ( ; i+3 < numVerts; i+=4 ) {
		const float *vertPtr = verts[i].xyz.ToFloatPtr();
		const float *vertPtr2 = verts[i+1].xyz.ToFloatPtr();
		const float *vertPtr3 = verts[i+2].xyz.ToFloatPtr();
		const float *vertPtr4 = verts[i+3].xyz.ToFloatPtr();
			
		vecXYZ1 = vec_ld( 0, vertPtr );
		vecXYZ2 = vec_ld( 0, vertPtr2 );
		vecXYZ3 = vec_ld( 0, vertPtr3 );
		vecXYZ4 = vec_ld( 0, vertPtr4 );
		
		// like a splat, but only doing halves
		vecSum1 = vec_madd( vec_perm( vecXYZ1, vecXYZ2, (vector unsigned char)(0,1,2,3,0,1,2,3,16,17,18,19,16,17,18,19) ), vecPlane0, zeroVector );
		vecSum1 = vec_madd( vec_perm( vecXYZ1, vecXYZ2, (vector unsigned char)(4,5,6,7,4,5,6,7,20,21,22,23,20,21,22,23) ) , vecPlane1, vecSum1 );
		vecSum1 = vec_madd( vec_perm( vecXYZ1, vecXYZ2, (vector unsigned char)(8,9,10,11,8,9,10,11,24,25,26,27,24,25,26,27) ), vecPlane2, vecSum1 );
		vecSum1 = vec_add( vecSum1, vecPlane3 );

		vecSum2 = vec_madd( vec_perm( vecXYZ3, vecXYZ4, (vector unsigned char)(0,1,2,3,0,1,2,3,16,17,18,19,16,17,18,19) ), vecPlane0, zeroVector );
		vecSum2 = vec_madd( vec_perm( vecXYZ3, vecXYZ4, (vector unsigned char)(4,5,6,7,4,5,6,7,20,21,22,23,20,21,22,23) ) , vecPlane1, vecSum2 );
		vecSum2 = vec_madd( vec_perm( vecXYZ3, vecXYZ4, (vector unsigned char)(8,9,10,11,8,9,10,11,24,25,26,27,24,25,26,27) ), vecPlane2, vecSum2 );
		vecSum2 = vec_add( vecSum2, vecPlane3 );

		// store out results
		UNALIGNED_STORE2( &texCoords[i][0], vecSum1, vecSum2 );

		// bit manipulation
		vecCmp1 = vec_cmplt( vecSum1, zeroVector );
		vecCmp2 = vec_cmplt( vecSum2, zeroVector );
		
		//and it with 1 so we multiply by 1 not 1111's
		vecCmp1 = vec_and( vecCmp1, oneIntVector );
		vecCmp2 = vec_and( vecCmp2, oneIntVector );		
 		
		 // store out and write to cullBits 
		// finally, a use for algebra! 1-x = x + 1 - 2x
		vecSum1Inv = vec_madd( vecSum1, negTwoVector, vecSum1 );
		vecSum2Inv = vec_madd( vecSum2, negTwoVector, vecSum2 );
		vecSum1Inv = vec_add( vecSum1Inv, oneVector );
		vecSum2Inv = vec_add( vecSum2Inv, oneVector );
		
		// do the same comparisons for the inverted d0/d1
		vecCmp1Inv = vec_cmplt( vecSum1Inv, zeroVector );
		vecCmp2Inv = vec_cmplt( vecSum2Inv, zeroVector );
		
		//and it with 1 so we multiply by 1 not 1111's
		vecCmp1Inv = vec_and( vecCmp1Inv, oneIntVector );
		vecCmp2Inv = vec_and( vecCmp2Inv, oneIntVector );

		// shift them as needed
		vecBitShifted1 = vec_sl( (vector unsigned int)vecCmp1, vecShift );
		vecBitShifted2 = vec_sl( (vector unsigned int)vecCmp2, vecShift );
		vecBitShifted1Inv = vec_sl( (vector unsigned int)vecCmp1Inv, vecShiftInv );
		vecBitShifted2Inv = vec_sl( (vector unsigned int)vecCmp2Inv, vecShiftInv );

		// OR them all together. since only 1 bit is set for each value, thats
		// the same as adding them. add up d0 + d1 + d0Inv + d1Inv
		vector unsigned int vecResult;
		vector unsigned int vecResult2;
		vector unsigned int vecResult3;
		vecResult = vec_add( vecBitShifted1, vec_sld( vecBitShifted1, vecBitShifted1, 4 ) );
		
		vecResult2 = vec_add( vecBitShifted2, vec_sld( vecBitShifted2, vecBitShifted2, 4 ) );
		
		// vecResult now holds the values without the inverses yet, so add those
		vecResult = vec_perm( vecResult, vecResult2, vecPermFirstThird );			
		vecResult2 = vec_add( vecBitShifted1Inv, vec_sld( vecBitShifted1Inv, vecBitShifted1Inv, 4 ) );
		vecResult3 = vec_add( vecBitShifted2Inv, vec_sld( vecBitShifted2Inv, vecBitShifted2Inv, 4 ) );
		vecResult2 = vec_perm( vecResult2, vecResult3, vecPermFirstThird );
		
		vecResult = vec_add( vecResult, vecResult2 );
		
		//store out results
		vecResult = vec_perm( vecResult, vecResult, cullBitPerm );
		vec_ste( vecResult, 0, &cullBitVal[0] );
		vec_ste( vecResult, 4, &cullBitVal[0] );
		vec_ste( vecResult, 8, &cullBitVal[0] );
		vec_ste( vecResult, 12, &cullBitVal[0] );
			
		cullBits[i] = cullBitVal[0];
		cullBits[i+1] = cullBitVal[1];
		cullBits[i+2] = cullBitVal[2];
		cullBits[i+3] = cullBitVal[3];
	}

	// cleanup
	for ( ; i < numVerts; i++ ) {
		byte bits;
		float d0, d1;
		float vx, vy, vz;
			
		vx = *( vertPtr + (i*DRAWVERT_OFFSET) + 0 );
		vy = *( vertPtr + (i*DRAWVERT_OFFSET) + 1 );
		vz = *( vertPtr + (i*DRAWVERT_OFFSET) + 2 );
		
		d0 = p0x * vx + p0y * vy + p0z * vz + p0d;
		d1 = p1x * vx + p1y * vy + p1z * vz + p1d;
		texCoords[i][0] = d0;
		texCoords[i][1] = d1;
		
		bits = ( d0 >= 0 ) ? 0 : 1;
		d0 = 1.0f - d0;
		bits |= ( d1 >= 0 ) ? 0 : 1*2; 
		d1 = 1.0f - d1;
		
		bits |= ( d0 >= 0 ) ? 0: 1*4;
		bits |= ( d1 >= 0 ) ? 0: 1*8;

		cullBits[i] = bits;
	}
}


#endif /* DRAWVERT_PADDED */

#endif /* ENABLE_CULL */

#ifdef ENABLE_DERIVE
/*
============
idSIMD_AltiVec::DeriveTriPlanes

	Derives a plane equation for each triangle.
============
*/
void VPCALL idSIMD_AltiVec::DeriveTriPlanes( idPlane *planes, const idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes ) {

	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	// idPlane size
	assert( sizeof(idPlane) == PLANE_OFFSET * sizeof(float) );
	int i;

	vector float vecD0, vecD1, vecD2, vecD3, vecD4, vecD5, vecD6, vecD7;
	vector float vecVertA, vecVertB, vecVertC;
	vector float vecVertA2, vecVertB2, vecVertC2;
	vector float vecVertA3, vecVertB3, vecVertC3;
	vector float vecVertA4, vecVertB4, vecVertC4;

	vector float vecN, vecN2, vecN3, vecN4;
	vector float vecWork1, vecWork2, vecWork3, vecWork4, vecWork5, vecWork6, vecWork7, vecWork8;
	vector unsigned char vecPerm1 =  (vector unsigned char)(4,5,6,7,8,9,10,11,0,1,2,3,12,13,14,15);
	vector unsigned char vecPerm2 = (vector unsigned char)(8,9,10,11,0,1,2,3,4,5,6,7,12,13,14,15);
	vector float vecF;
	vector float vecF1, vecF2, vecF3, vecF4;
	vector float zeroVector = (vector float)(0);
	vector float vecNegOne = (vector float)(-1);
	vector float vecSecondHalf, vecFirstHalf, vecSecondHalf2, vecFirstHalf2, vecSecondHalf3, vecFirstHalf3, vecFirstHalf4, vecSecondHalf4;
	
	vector unsigned char vecPermA, vecPermA2, vecPermA3, vecPermA4;
	vector unsigned char vecPermB, vecPermB2, vecPermB3, vecPermB4;
	vector unsigned char vecPermC, vecPermC2, vecPermC3, vecPermC4;
	
	vector unsigned char oneVector = (vector unsigned char)(1);
	vector float vecLd1, vecLd2, vecLd3, vecLd4, vecLd5, vecLd6;
	vector unsigned char vecPermZeroLast = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,16,17,18,19);

	const float *xyzPtr = verts[0].xyz.ToFloatPtr();
	float *planePtr = planes[0].ToFloatPtr();

	int j;  
	for ( j = 0, i = 0; i+11 < numIndexes; i += 12, j += 4 ) {

#ifndef DRAWVERT_PADDED	
		// calculate permute vectors to load as needed. these are all
		// triangle indexes and are usaully pretty close together but
		// not guaranteed to be in any particular order
		vecPermA = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+0] * DRAWVERT_OFFSET ) ), oneVector );
		vecPermB = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+1] * DRAWVERT_OFFSET ) ), oneVector );
		vecPermC = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+2] * DRAWVERT_OFFSET ) ), oneVector );	
		vecPermA2 = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+3] * DRAWVERT_OFFSET ) ), oneVector );
		vecPermB2 = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+4] * DRAWVERT_OFFSET ) ), oneVector );
		vecPermC2 = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+5] * DRAWVERT_OFFSET ) ), oneVector );			
		vecPermA3 = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+6] * DRAWVERT_OFFSET ) ), oneVector );
		vecPermB3 = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+7] * DRAWVERT_OFFSET ) ), oneVector );
		vecPermC3 = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+8] * DRAWVERT_OFFSET ) ), oneVector );	
		vecPermA4 = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+9] * DRAWVERT_OFFSET ) ), oneVector );
		vecPermB4 = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+10] * DRAWVERT_OFFSET ) ), oneVector );
		vecPermC4 = vec_add( vec_lvsl( -1, xyzPtr + ( indexes[i+11] * DRAWVERT_OFFSET ) ), oneVector );			
#endif

#ifndef DRAWVERT_PADDED 
		// load first A B C 
		vecLd1 = vec_ld( 0, xyzPtr + ( indexes[i+0] * DRAWVERT_OFFSET ) );
		vecLd2 = vec_ld( 15, xyzPtr + ( indexes[i+0] * DRAWVERT_OFFSET ) );
		vecLd3 = vec_ld( 0, xyzPtr + ( indexes[i+1] * DRAWVERT_OFFSET ) );
		vecLd4 = vec_ld( 15, xyzPtr + ( indexes[i+1] * DRAWVERT_OFFSET ) );
		vecLd5 = vec_ld( 0, xyzPtr + ( indexes[i+2] * DRAWVERT_OFFSET ) );
		vecLd6 = vec_ld( 15, xyzPtr + ( indexes[i+2] * DRAWVERT_OFFSET ) );
		
		vecVertA = vec_perm( vecLd1, vecLd2, vecPermA );
		vecVertB = vec_perm( vecLd3, vecLd4, vecPermB );
		vecVertC = vec_perm( vecLd5, vecLd6, vecPermC );

		// set the last element to 0
		vecVertA = vec_perm( vecVertA, zeroVector, vecPermZeroLast );
		vecVertB = vec_perm( vecVertB, zeroVector, vecPermZeroLast );
		vecVertC = vec_perm( vecVertC, zeroVector, vecPermZeroLast );
		
		// load second A B C
		vecLd1 = vec_ld( 0, xyzPtr + ( indexes[i+3] * DRAWVERT_OFFSET ) );
		vecLd2 = vec_ld( 15, xyzPtr + ( indexes[i+3] * DRAWVERT_OFFSET ) );
		vecLd3 = vec_ld( 0, xyzPtr + ( indexes[i+4] * DRAWVERT_OFFSET ) );
		vecLd4 = vec_ld( 15, xyzPtr + ( indexes[i+4] * DRAWVERT_OFFSET ) );
		vecLd5 = vec_ld( 0, xyzPtr + ( indexes[i+5] * DRAWVERT_OFFSET ) );
		vecLd6 = vec_ld( 15, xyzPtr + ( indexes[i+5] * DRAWVERT_OFFSET ) );
		
		vecVertA2 = vec_perm( vecLd1, vecLd2, vecPermA2 );
		vecVertB2 = vec_perm( vecLd3, vecLd4, vecPermB2 );
		vecVertC2 = vec_perm( vecLd5, vecLd6, vecPermC2 );

		// set the last element to 0
		vecVertA2 = vec_perm( vecVertA2, zeroVector, vecPermZeroLast );
		vecVertB2 = vec_perm( vecVertB2, zeroVector, vecPermZeroLast );
		vecVertC2 = vec_perm( vecVertC2, zeroVector, vecPermZeroLast );

		// load third A B C
		vecLd1 = vec_ld( 0, xyzPtr + ( indexes[i+6] * DRAWVERT_OFFSET ) );
		vecLd2 = vec_ld( 15, xyzPtr + ( indexes[i+6] * DRAWVERT_OFFSET ) );
		vecLd3 = vec_ld( 0, xyzPtr + ( indexes[i+7] * DRAWVERT_OFFSET ) );
		vecLd4 = vec_ld( 15, xyzPtr + ( indexes[i+7] * DRAWVERT_OFFSET ) );
		vecLd5 = vec_ld( 0, xyzPtr + ( indexes[i+8] * DRAWVERT_OFFSET ) );
		vecLd6 = vec_ld( 15, xyzPtr + ( indexes[i+8] * DRAWVERT_OFFSET ) );
		
		vecVertA3 = vec_perm( vecLd1, vecLd2, vecPermA3 );
		vecVertB3 = vec_perm( vecLd3, vecLd4, vecPermB3 );
		vecVertC3 = vec_perm( vecLd5, vecLd6, vecPermC3 );

		// set the last element to 0
		vecVertA2 = vec_perm( vecVertA2, zeroVector, vecPermZeroLast );
		vecVertB2 = vec_perm( vecVertB2, zeroVector, vecPermZeroLast );
		vecVertC2 = vec_perm( vecVertC2, zeroVector, vecPermZeroLast );

		// load the fourth A B C
		vecLd1 = vec_ld( 0, xyzPtr + ( indexes[i+9] * DRAWVERT_OFFSET ) );
		vecLd2 = vec_ld( 15, xyzPtr + ( indexes[i+9] * DRAWVERT_OFFSET ) );
		vecLd3 = vec_ld( 0, xyzPtr + ( indexes[i+10] * DRAWVERT_OFFSET ) );
		vecLd4 = vec_ld( 15, xyzPtr + ( indexes[i+10] * DRAWVERT_OFFSET ) );
		vecLd5 = vec_ld( 0, xyzPtr + ( indexes[i+11] * DRAWVERT_OFFSET ) );
		vecLd6 = vec_ld( 15, xyzPtr + ( indexes[i+11] * DRAWVERT_OFFSET ) );
		
		vecVertA4 = vec_perm( vecLd1, vecLd2, vecPermA4 );
		vecVertB4 = vec_perm( vecLd3, vecLd4, vecPermB4 );
		vecVertC4 = vec_perm( vecLd5, vecLd6, vecPermC4 );

		// set the last element to 0
		vecVertA4 = vec_perm( vecVertA4, zeroVector, vecPermZeroLast );
		vecVertB4 = vec_perm( vecVertB4, zeroVector, vecPermZeroLast );
		vecVertC4 = vec_perm( vecVertC4, zeroVector, vecPermZeroLast );
#else
		// load first A B C 
		vecVertA = vec_ld( 0, xyzPtr + ( indexes[i+0] * DRAWVERT_OFFSET ) );
		vecVertB = vec_ld( 0, xyzPtr + ( indexes[i+1] * DRAWVERT_OFFSET ) );
		vecVertC = vec_ld( 0, xyzPtr + ( indexes[i+2] * DRAWVERT_OFFSET ) );

		// set the last element to 0
		vecVertA = vec_perm( vecVertA, zeroVector, vecPermZeroLast );
		vecVertB = vec_perm( vecVertB, zeroVector, vecPermZeroLast );
		vecVertC = vec_perm( vecVertC, zeroVector, vecPermZeroLast );
		
		// load second A B C
		vecVertA2 = vec_ld( 0, xyzPtr + ( indexes[i+3] * DRAWVERT_OFFSET ) );
		vecVertB2 = vec_ld( 0, xyzPtr + ( indexes[i+4] * DRAWVERT_OFFSET ) );
		vecVertC2 = vec_ld( 0, xyzPtr + ( indexes[i+5] * DRAWVERT_OFFSET ) );
		
		// set the last element to 0
		vecVertA2 = vec_perm( vecVertA2, zeroVector, vecPermZeroLast );
		vecVertB2 = vec_perm( vecVertB2, zeroVector, vecPermZeroLast );
		vecVertC2 = vec_perm( vecVertC2, zeroVector, vecPermZeroLast );

		// load third A B C
		vecVertA3 = vec_ld( 0, xyzPtr + ( indexes[i+6] * DRAWVERT_OFFSET ) );
		vecVertB3 = vec_ld( 0, xyzPtr + ( indexes[i+7] * DRAWVERT_OFFSET ) );
		vecVertC3 = vec_ld( 0, xyzPtr + ( indexes[i+8] * DRAWVERT_OFFSET ) );

		// set the last element to 0
		vecVertA3 = vec_perm( vecVertA3, zeroVector, vecPermZeroLast );
		vecVertB3 = vec_perm( vecVertB3, zeroVector, vecPermZeroLast );
		vecVertC3 = vec_perm( vecVertC3, zeroVector, vecPermZeroLast );

		// load the fourth A B C
		vecVertA4 = vec_ld( 0, xyzPtr + ( indexes[i+9] * DRAWVERT_OFFSET ) );
		vecVertB4 = vec_ld( 0, xyzPtr + ( indexes[i+10] * DRAWVERT_OFFSET ) );
		vecVertC4 = vec_ld( 0, xyzPtr + ( indexes[i+11] * DRAWVERT_OFFSET ) );
		
		// set the last element to 0
		vecVertA4 = vec_perm( vecVertA4, zeroVector, vecPermZeroLast );
		vecVertB4 = vec_perm( vecVertB4, zeroVector, vecPermZeroLast );
		vecVertC4 = vec_perm( vecVertC4, zeroVector, vecPermZeroLast );
#endif
		// calculate d0 and d1 for each
		vecD0 = vec_sub( vecVertB, vecVertA );
		vecD1 = vec_sub( vecVertC, vecVertA );
		
		vecD2 = vec_sub( vecVertB2, vecVertA2 );
		vecD3 = vec_sub( vecVertC2, vecVertA2 );
		
		vecD4 = vec_sub( vecVertB3, vecVertA3 );
		vecD5 = vec_sub( vecVertC3, vecVertA3 );
		
		vecD6 = vec_sub( vecVertB4, vecVertA4 );
		vecD7 = vec_sub( vecVertC4, vecVertA4 );

		vecWork1 = vec_perm( vecD0, vecD0, vecPerm1 );
		vecWork2 = vec_perm( vecD1, vecD1, vecPerm2 );
		vecWork3 = vec_perm( vecD2, vecD2, vecPerm1 );
		vecWork4 = vec_perm( vecD3, vecD3, vecPerm2 );
		vecWork5 = vec_perm( vecD4, vecD4, vecPerm1 );
		vecWork6 = vec_perm( vecD5, vecD5, vecPerm2 );
		vecWork7 = vec_perm( vecD6, vecD6, vecPerm1 );
		vecWork8 = vec_perm( vecD7, vecD7, vecPerm2 );
		
		vecSecondHalf = vec_madd( vecWork1, vecWork2, zeroVector );
		vecSecondHalf2 = vec_madd( vecWork3, vecWork4, zeroVector );
		vecSecondHalf3 = vec_madd( vecWork5, vecWork6, zeroVector );
		vecSecondHalf4 = vec_madd( vecWork7, vecWork8, zeroVector );
		
		vecWork1 = vec_perm( vecD1, vecD1, vecPerm1 );
		vecWork2 = vec_perm( vecD0, vecD0, vecPerm2 );
		vecWork3 = vec_perm( vecD3, vecD3, vecPerm1 );
		vecWork4 = vec_perm( vecD2, vecD2, vecPerm2 );
		vecWork5 = vec_perm( vecD5, vecD5, vecPerm1 );
		vecWork6 = vec_perm( vecD4, vecD4, vecPerm2 );
		vecWork7 = vec_perm( vecD7, vecD7, vecPerm1 );
		vecWork8 = vec_perm( vecD6, vecD6, vecPerm2 );
		
		vecFirstHalf = vec_madd( vecWork1, vecWork2, zeroVector );
		vecFirstHalf2 = vec_madd( vecWork3, vecWork4, zeroVector );
		vecFirstHalf3 = vec_madd( vecWork5, vecWork6, zeroVector );
		vecFirstHalf4 = vec_madd( vecWork7, vecWork8, zeroVector );
		
		vecN = vec_madd( vecSecondHalf, vecNegOne, vecFirstHalf );
		vecN2 = vec_madd( vecSecondHalf2, vecNegOne, vecFirstHalf2 );
		vecN3 = vec_madd( vecSecondHalf3, vecNegOne, vecFirstHalf3 );
		vecN4 = vec_madd( vecSecondHalf4, vecNegOne, vecFirstHalf4 );
	
		// transpose vecNs
		vector float v0, v1, v2, v3;
		v0 = vec_mergeh( vecN, vecN3 );
		v1 = vec_mergeh( vecN2, vecN4 );
		v2 = vec_mergel( vecN, vecN3 );
		v3 = vec_mergel( vecN2, vecN4 );
			
		vecN = vec_mergeh( v0, v1 );
		vecN2 = vec_mergel( v0, v1 );
		vecN3 = vec_mergeh( v2, v3 );
		vecN4 = vec_mergel( v2, v3 );

		vecF = vec_madd( vecN, vecN, zeroVector );
		vecF = vec_madd( vecN2, vecN2, vecF );
		vecF = vec_madd( vecN3, vecN3, vecF );
				
		vecF = ReciprocalSquareRoot( vecF );
		
		vecF1 = vec_madd( vecF, vecN, zeroVector );
		vecF2 = vec_madd( vecF, vecN2, zeroVector );
		vecF3 = vec_madd( vecF, vecN3, zeroVector );
		vecF4 = vec_madd( vecF, vecN4, zeroVector );

		vector float v8, v9, v10, v11;
		v8 = vecF1;
		v9 = vecF2;
		v10 = vecF3;
		v11 = vecF4;

		// transpose vecVerts
		v0 = vec_mergeh( vecVertA, vecVertA3 );
		v1 = vec_mergeh( vecVertA2, vecVertA4 );
		v2 = vec_mergel( vecVertA, vecVertA3 );
		v3 = vec_mergel( vecVertA2, vecVertA4 );
			
		vecVertA = vec_mergeh( v0, v1 );
		vecVertA2 = vec_mergel( v0, v1 );
		vecVertA3 = vec_mergeh( v2, v3 );
		vecVertA4 = vec_mergel( v2, v3 );
			
		vector float vecTotals;
		vecTotals = vec_madd( vecVertA, v8, zeroVector );
		vecTotals = vec_madd( vecVertA2, v9, vecTotals );
		vecTotals = vec_madd( vecVertA3, v10, vecTotals );
		vecTotals = vec_madd( vecVertA4, v11, vecTotals );
		vecF = vec_madd( vecTotals, vecNegOne, zeroVector );
		
		// transpose vecFs
		v0 = vec_mergeh( vecF1, vecF3 );
		v1 = vec_mergeh( vecF2, vecF );
		v2 = vec_mergel( vecF1, vecF3 );
		v3 = vec_mergel( vecF2, vecF );
			
		vecF1 = vec_mergeh( v0, v1 );
		vecF2 = vec_mergel( v0, v1 );
		vecF3 = vec_mergeh( v2, v3 );
		vecF4 = vec_mergel( v2, v3 );

		// store results
		UNALIGNED_STORE4( planePtr + ( j * PLANE_OFFSET ), vecF1, vecF2, vecF3, vecF4 );
	}

	// cleanup
	for ( ; i < numIndexes; i += 3, j++ ) {
		const idDrawVert *a, *b, *c;
		float d0[3], d1[3], f;
		idVec3 n;

		a = verts + indexes[i + 0];
		b = verts + indexes[i + 1];
		c = verts + indexes[i + 2];

		d0[0] = b->xyz[0] - a->xyz[0];
		d0[1] = b->xyz[1] - a->xyz[1];
		d0[2] = b->xyz[2] - a->xyz[2];

		d1[0] = c->xyz[0] - a->xyz[0];
		d1[1] = c->xyz[1] - a->xyz[1];
		d1[2] = c->xyz[2] - a->xyz[2];

		n[0] = d1[1] * d0[2] - d1[2] * d0[1];
		n[1] = d1[2] * d0[0] - d1[0] * d0[2];
		n[2] = d1[0] * d0[1] - d1[1] * d0[0];

		f = FastScalarInvSqrt( n.x * n.x + n.y * n.y + n.z * n.z );
		//idMath::RSqrt( n.x * n.x + n.y * n.y + n.z * n.z );

		n.x *= f;
		n.y *= f;
		n.z *= f;

		planes[j].SetNormal( n );
		planes[j].FitThroughPoint( a->xyz );
	}	
}

/*
============
idSIMD_AltiVec::DeriveTangents

	Derives the normal and orthogonal tangent vectors for the triangle vertices.
	For each vertex the normal and tangent vectors are derived from all triangles
	using the vertex which results in smooth tangents across the mesh.
	In the process the triangle planes are calculated as well.

============
*/
void VPCALL idSIMD_AltiVec::DeriveTangents( idPlane *planes, idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes ) {
	int i;

	bool *used = (bool *)_alloca16( numVerts * sizeof( used[0] ) );
	memset( used, 0, numVerts * sizeof( used[0] ) );
	
	idPlane *planesPtr = planes;
	for ( i = 0; i < numIndexes; i += 3 ) {
		idDrawVert *a, *b, *c;
	//	unsigned long signBit;
		float d0[5], d1[5], area;
		idVec3 n, t0, t1;
		float f1, f2, f3;

		int v0 = indexes[i + 0];
		int v1 = indexes[i + 1];
		int v2 = indexes[i + 2];

		a = verts + v0;
		b = verts + v1;
		c = verts + v2;

		d0[0] = b->xyz[0] - a->xyz[0];
		d0[1] = b->xyz[1] - a->xyz[1];
		d0[2] = b->xyz[2] - a->xyz[2];
		d0[3] = b->st[0] - a->st[0];
		d0[4] = b->st[1] - a->st[1];

		d1[0] = c->xyz[0] - a->xyz[0];
		d1[1] = c->xyz[1] - a->xyz[1];
		d1[2] = c->xyz[2] - a->xyz[2];
		d1[3] = c->st[0] - a->st[0];
		d1[4] = c->st[1] - a->st[1];

		// normal
		n[0] = d1[1] * d0[2] - d1[2] * d0[1];
		n[1] = d1[2] * d0[0] - d1[0] * d0[2];
		n[2] = d1[0] * d0[1] - d1[1] * d0[0];

		f1 =  n.x * n.x + n.y * n.y + n.z * n.z;
		
		// area sign bit
		area = d0[3] * d1[4] - d0[4] * d1[3];

		// first tangent
		t0[0] = d0[0] * d1[4] - d0[4] * d1[0];
		t0[1] = d0[1] * d1[4] - d0[4] * d1[1];
		t0[2] = d0[2] * d1[4] - d0[4] * d1[2];
		
		f2 = t0.x * t0.x + t0.y * t0.y + t0.z * t0.z;

		// second tangent
		t1[0] = d0[3] * d1[0] - d0[0] * d1[3];
		t1[1] = d0[3] * d1[1] - d0[1] * d1[3];
		t1[2] = d0[3] * d1[2] - d0[2] * d1[3];

		f3 = t1.x * t1.x + t1.y * t1.y + t1.z * t1.z;

		// Behold! The power of the pipeline
		FastScalarInvSqrt_x3( &f1, &f2, &f3 );
#ifdef PPC_INTRINSICS		
		f2 = __fsel( area, f2, -f2 );
		f3 = __fsel( area, f3, -f3 );
#else
		f2 = ( area < 0.0f ) ? -f2 : f2;
		f3 = ( area < 0.0f ) ? -f3 : f3;
#endif
		t0.x *= f2;
		t0.y *= f2;
		t0.z *= f2;
		
		n.x *= f1;
		n.y *= f1;
		n.z *= f1;

		planesPtr->SetNormal( n );
		planesPtr->FitThroughPoint( a->xyz );
		planesPtr++;

		t1.x *= f3;
		t1.y *= f3;
		t1.z *= f3;

		if ( used[v0] ) {
			a->normal += n;
			a->tangents[0] += t0;
			a->tangents[1] += t1;
		} else {
			a->normal = n;
			a->tangents[0] = t0;
			a->tangents[1] = t1;
			used[v0] = true;
		}

		if ( used[v1] ) {
			b->normal += n;
			b->tangents[0] += t0;
			b->tangents[1] += t1;
		} else {
			b->normal = n;
			b->tangents[0] = t0;
			b->tangents[1] = t1;
			used[v1] = true;
		}

		if ( used[v2] ) {
			c->normal += n;
			c->tangents[0] += t0;
			c->tangents[1] += t1;
		} else {
			c->normal = n;
			c->tangents[0] = t0;
			c->tangents[1] = t1;
			used[v2] = true;
		}
	}
}


#ifdef DERIVE_UNSMOOTH_DRAWVERT_ALIGNED

/*
============
idSIMD_AltiVec::DeriveUnsmoothedTangents

	Derives the normal and orthogonal tangent vectors for the triangle vertices.
	For each vertex the normal and tangent vectors are derived from a single dominant triangle.
============
*/
#define DERIVE_UNSMOOTHED_BITANGENT
void VPCALL idSIMD_AltiVec::DeriveUnsmoothedTangents( idDrawVert *verts, const dominantTri_s *dominantTris, const int numVerts ) {

	int i;
	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	// drawverts aligned
	assert( IS_16BYTE_ALIGNED( verts[0] ) );
		
	vector float vecVertA, vecVertB, vecVertC;
	vector float vecVertA2, vecVertB2, vecVertC2;
	vector float vecVertA3, vecVertB3, vecVertC3;
	vector float vecVertA4, vecVertB4, vecVertC4;

	vector float v0, v1, v2, v3, v4, v5, v6, v7, v8;
	vector float vecS0, vecS1, vecS2;
	vector float vecS0_2, vecS1_2, vecS2_2;
	vector float vecS0_3, vecS1_3, vecS2_3;
	vector float vecS0_4, vecS1_4, vecS2_4;
	
	vector float vecD1, vecD2, vecD3, vecD4, vecD5, vecD6;
	vector float vecD7, vecD8, vecD9, vecD10, vecD11, vecD12;
	vector float vecT1, vecT1_2, vecT1_3, vecT1_4, vecT2, vecT2_2, vecT2_3, vecT2_4;
	vector float vecWork1, vecWork2, vecWork3, vecWork4, vecWork5, vecWork6, vecWork7, vecWork8;
	vector float vecN, vecN2, vecN3, vecN4;
	
	vector unsigned char vecPermN0 = (vector unsigned char)(8,9,10,11,0,1,2,3,4,5,6,7,12,13,14,15);
	vector unsigned char vecPermN1 = (vector unsigned char)(4,5,6,7,8,9,10,11,0,1,2,3,12,13,14,15);
	vector unsigned char vecPermT0 = (vector unsigned char)(0,1,2,3,0,1,2,3,0,1,2,3,0,1,2,3);
	vector unsigned char vecPermT1 = (vector unsigned char)(8,9,10,11,8,9,10,11,8,9,10,11,8,9,10,11);
	vector float zeroVector = (vector float)(0);
	
	vector float vecNegOne = (vector float)(-1.0);
	
	vector float vecStore1, vecStore2, vecStore3;
	vector unsigned char vecPermFirstThreeLast = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,16,17,18,19);
	vector unsigned char vecPermStoreSecond = (vector unsigned char)(4,5,6,7,8,9,10,11,16,17,18,19,20,21,22,23);
	vector unsigned char vecPermLeadAndThree = (vector unsigned char)(0,1,2,3,16,17,18,19,20,21,22,23,24,25,26,27);
	vector unsigned char vecPermStore2 = (vector unsigned char)(4,5,6,7,8,9,10,11,24,25,26,27,28,29,30,31);
	vector unsigned char vecPermStore3 = (vector unsigned char)(4,5,6,7,8,9,10,11,16,17,18,19,20,21,22,23);
	vector unsigned char vecPermStore4 = (vector unsigned char)(8,9,10,11,16,17,18,19,20,21,22,23,24,25,26,27);
	vector unsigned char vecPermHalves = (vector unsigned char)(0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
	
	vector float vecLd1, vecLd2, vecLd3;
	vector unsigned char vecPerm0, vecPerm1, vecPerm2, vecPerm3, vecPerm4;
	
	float *normalPtr = verts[0].normal.ToFloatPtr();
	float *xyzPtr = verts[0].xyz.ToFloatPtr();	

	vector float vecFirstHalf, vecSecondHalf;
	vector float vecFirstHalf2, vecSecondHalf2;
	vector float vecFirstHalf3, vecSecondHalf3;
	vector float vecFirstHalf4, vecSecondHalf4;
		
	for ( i = 0; i+3 < numVerts; i+=4 ) {
		int bOffset1, bOffset2, bOffset3, bOffset4;
		int cOffset1, cOffset2, cOffset3, cOffset4;
		
		bOffset1 = dominantTris[i].v2;
		cOffset1 = dominantTris[i].v3;
		bOffset2 = dominantTris[i+1].v2;
		cOffset2 = dominantTris[i+1].v3;
		bOffset3 = dominantTris[i+2].v2;
		cOffset3 = dominantTris[i+2].v3;
		bOffset4 = dominantTris[i+3].v2;
		cOffset4 = dominantTris[i+3].v3;
		
		vecPerm0 = vec_lvsl( 0, xyzPtr + ( i * DRAWVERT_OFFSET ) );
		v0 = vec_ld( 0, xyzPtr + (i * DRAWVERT_OFFSET ) );
		v1 = vec_ld( 16, xyzPtr + (i * DRAWVERT_OFFSET ) );
		vecVertA = vec_perm( v0, v1, vecPerm0 );

		vecPerm1 = vec_lvsl( 0, xyzPtr + (bOffset1 * DRAWVERT_OFFSET ) );
		v2 = vec_ld( 0, xyzPtr + ( bOffset1 * DRAWVERT_OFFSET ) );
		v3 = vec_ld( 16, xyzPtr + ( bOffset1 * DRAWVERT_OFFSET ) );
		vecVertB = vec_perm( v2, v3, vecPerm1 );
		
		vecPerm2 = vec_lvsl( 0, xyzPtr + ( cOffset1 * DRAWVERT_OFFSET ) );
		v4 = vec_ld( 0, xyzPtr + ( cOffset1 * DRAWVERT_OFFSET ) );
		v5 = vec_ld( 16, xyzPtr + ( cOffset1 * DRAWVERT_OFFSET ) );
		vecVertC = vec_perm( v4, v5, vecPerm2 );

		// put remainder into v2
		v1 = vec_perm( v1, v1, vecPerm0 );
		v3 = vec_perm( v3, v3, vecPerm1 );
		v5 = vec_perm( v5, v5, vecPerm2 );
		
		v1 = vec_mergeh( v1, v5 );
		v2 = vec_mergeh( v3, zeroVector );
		v2 = vec_mergeh( v1, v2 );
		v2 = vec_perm( v2, v2, (vector unsigned char)(4,5,6,7,0,1,2,3,8,9,10,11,0,1,2,3) );
			
		// load second one
		vecPerm0 = vec_lvsl( 0, xyzPtr + ((i+1) * DRAWVERT_OFFSET ) );
		v0 = vec_ld( 0, xyzPtr + ((i+1) * DRAWVERT_OFFSET ) );
		v1 = vec_ld( 16, xyzPtr + ((i+1) * DRAWVERT_OFFSET ) );
		vecVertA2 = vec_perm( v0, v1, vecPerm0 );		
		
		vecPerm3 = vec_lvsl( 0, xyzPtr + (bOffset2 * DRAWVERT_OFFSET ) );
		v3 = vec_ld( 0, xyzPtr + ( bOffset2 * DRAWVERT_OFFSET ) );
		v4 = vec_ld( 16, xyzPtr + ( bOffset2 * DRAWVERT_OFFSET ) );
		vecVertB2 = vec_perm( v3, v4, vecPerm3 );
		
		vecPerm4 = vec_lvsl( 0, xyzPtr + ( cOffset2 * DRAWVERT_OFFSET ) );
		v5 = vec_ld( 0, xyzPtr + ( cOffset2 * DRAWVERT_OFFSET ) );
		v6 = vec_ld( 16, xyzPtr + ( cOffset2 * DRAWVERT_OFFSET ) );
		vecVertC2 = vec_perm( v5, v6, vecPerm4 );

		// put remainder into v3
		v1 = vec_perm( v1, v1, vecPerm0 );
		v4 = vec_perm( v4, v4, vecPerm3 );
		v5 = vec_perm( v6, v6, vecPerm4 );
		
		v1 = vec_mergeh( v1, v5 );
		v3 = vec_mergeh( v4, zeroVector );
		v3 = vec_mergeh( v1, v3 );
		v3 = vec_perm( v3, v3, (vector unsigned char)(4,5,6,7,0,1,2,3,8,9,10,11,0,1,2,3) );

		// load third one
		vecPerm0 = vec_lvsl( 0, xyzPtr + ((i+2) * DRAWVERT_OFFSET ) );
		v0 = vec_ld( 0, xyzPtr + ((i+2) * DRAWVERT_OFFSET ) );
		v1 = vec_ld( 16, xyzPtr + ((i+2) * DRAWVERT_OFFSET ) );
		vecVertA3 = vec_perm( v0, v1, vecPerm0 );
				
		vecPerm1 = vec_lvsl( 0, xyzPtr + (bOffset3 * DRAWVERT_OFFSET ) );
		v4 = vec_ld( 0, xyzPtr + ( bOffset3 * DRAWVERT_OFFSET ) );
		v5 = vec_ld( 16, xyzPtr + ( bOffset3 * DRAWVERT_OFFSET ) );
		vecVertB3 = vec_perm( v4, v5, vecPerm1 );
		
		vecPerm2 = vec_lvsl( 0, xyzPtr + ( cOffset3 * DRAWVERT_OFFSET ) );
		v6 = vec_ld( 0, xyzPtr + ( cOffset3 * DRAWVERT_OFFSET ) );
		v7 = vec_ld( 16, xyzPtr + ( cOffset3 * DRAWVERT_OFFSET ) );
		vecVertC3 = vec_perm( v6, v7, vecPerm2 );

		// put remainder into v4	
		v1 = vec_perm( v1, v1, vecPerm0 );
		v5 = vec_perm( v5, v5, vecPerm1 );
		v7 = vec_perm( v7, v7, vecPerm2 );
		
		v1 = vec_mergeh( v1, v7 );
		v4 = vec_mergeh( v5, zeroVector );
		v4 = vec_mergeh( v1, v4 );
		v4 = vec_perm( v4, v4, (vector unsigned char)(4,5,6,7,0,1,2,3,8,9,10,11,0,1,2,3) );	
			
		// load fourth one
		vecPerm0 = vec_lvsl( 0, xyzPtr + ((i+3) * DRAWVERT_OFFSET ) );
		v0 = vec_ld( 0, xyzPtr + ((i+3) * DRAWVERT_OFFSET ) );
		v1 = vec_ld( 16, xyzPtr + ((i+3) * DRAWVERT_OFFSET ) );
		vecVertA4 = vec_perm( v0, v1, vecPerm0 );
		
		vecPerm3 = vec_lvsl( 0, xyzPtr + (bOffset4 * DRAWVERT_OFFSET ) );
		v5 = vec_ld( 0, xyzPtr + ( bOffset4 * DRAWVERT_OFFSET ) );
		v6 = vec_ld( 16, xyzPtr + ( bOffset4 * DRAWVERT_OFFSET ) );
		vecVertB4 = vec_perm( v5, v6, vecPerm3 );
		
		vecPerm4 = vec_lvsl( 0, xyzPtr + ( cOffset4 * DRAWVERT_OFFSET ) );
		v7 = vec_ld( 0, xyzPtr + ( cOffset4 * DRAWVERT_OFFSET ) );
		v8 = vec_ld( 16, xyzPtr + ( cOffset4 * DRAWVERT_OFFSET ) );
		vecVertC4 = vec_perm( v7, v8, vecPerm4 );

		// put remainder into v5
		v1 = vec_perm( v1, v1, vecPerm0 );
		v6 = vec_perm( v6, v6, vecPerm3 );
		v8 = vec_perm( v8, v8, vecPerm4 );
		
		v1 = vec_mergeh( v1, v8 );
		v5 = vec_mergeh( v6, zeroVector );
		v5 = vec_mergeh( v1, v5 );
		v5 = vec_perm( v5, v5, (vector unsigned char)(4,5,6,7,0,1,2,3,8,9,10,11,0,1,2,3) );

		// remainder vectors look like b->st[1], a->st[1], c->st[1], a->st[1]

		//vecD1 now holds d0, d1, d2, d3
		vecD1 = vec_sub( vecVertB, vecVertA );
		vecD4 = vec_sub( vecVertB2, vecVertA2 );
		vecD7 = vec_sub( vecVertB3, vecVertA3 );
		vecD10 = vec_sub( vecVertB4, vecVertA4 );

		// vecD2 how holds d5, d6, d7, d8
		vecD2 = vec_sub( vecVertC, vecVertA );
		vecD5 = vec_sub( vecVertC2, vecVertA2 );
		vecD8 = vec_sub( vecVertC3, vecVertA3 );
		vecD11 = vec_sub( vecVertC4, vecVertA4 );
		
		// vecD3 now holds d4, crap, d9, crap
		vecD3 = vec_sub( v2, vec_sld( v2, v2, 4 ) );
		vecD6 = vec_sub( v3, vec_sld( v3, v3, 4 ) );
		vecD9 = vec_sub( v4, vec_sld( v4, v4, 4 ) );
		vecD12 = vec_sub( v5, vec_sld( v5, v5, 4 ) );

		// get permute vectors for loading from dt
		vecPerm1 = vec_add( vec_lvsl( -1, (int*) &dominantTris[i].normalizationScale[0] ), (vector unsigned char)(1) );
		vecPerm2 = vec_add( vec_lvsl( -1, (int*) &dominantTris[i+1].normalizationScale[0] ), (vector unsigned char)(1) );
		vecPerm3 = vec_add( vec_lvsl( -1, (int*) &dominantTris[i+2].normalizationScale[0] ), (vector unsigned char)(1) );
		vecPerm4 = vec_add( vec_lvsl( -1, (int*) &dominantTris[i+3].normalizationScale[0] ), (vector unsigned char)(1) );

		// load S values from dominantTris
		v0 = vec_ld( 0, &dominantTris[i].normalizationScale[0] );
		v1 = vec_ld( 11, &dominantTris[i].normalizationScale[0] );
		v2 = vec_ld( 0, &dominantTris[i+1].normalizationScale[0] );
		v3 = vec_ld( 11, &dominantTris[i+1].normalizationScale[0] );
		v4 = vec_ld( 0, &dominantTris[i+2].normalizationScale[0] );
		v5 = vec_ld( 11, &dominantTris[i+2].normalizationScale[0] );
		v6 = vec_ld( 0, &dominantTris[i+3].normalizationScale[0] );
		v7 = vec_ld( 11, &dominantTris[i+3].normalizationScale[0] );

		v0 = vec_perm( v0, v1, vecPerm1 );
		v2 = vec_perm( v2, v3, vecPerm2 );
		v4 = vec_perm( v4, v5, vecPerm3 );
		v6 = vec_perm( v6, v7, vecPerm4 );
		
		vecS0 = vec_splat( v0, 0 );
		vecS1 = vec_splat( v0, 1 );
		vecS2 = vec_splat( v0, 2 );
	
		vecS0_2 = vec_splat( v2, 0);
		vecS1_2 = vec_splat( v2, 1 );
		vecS2_2 = vec_splat( v2, 2 );

		vecS0_3 = vec_splat( v4, 0 );
		vecS1_3 = vec_splat( v4, 1 );
		vecS2_3 = vec_splat( v4, 2 );
					
		vecS0_4 = vec_splat( v6, 0 );
		vecS1_4 = vec_splat( v6, 1 );
		vecS2_4 = vec_splat( v6, 2 );

		// do calculation	
		vecWork1 = vec_perm( vecD2, vecD2, vecPermN1 );
		vecWork2 = vec_perm( vecD1, vecD1, vecPermN0 );
		vecWork3 = vec_perm( vecD5, vecD5, vecPermN1 );
		vecWork4 = vec_perm( vecD4, vecD4, vecPermN0 );
		vecWork5 = vec_perm( vecD8, vecD8, vecPermN1 );
		vecWork6 = vec_perm( vecD7, vecD7, vecPermN0 );
		vecWork7 = vec_perm( vecD11, vecD11, vecPermN1 );
		vecWork8 = vec_perm( vecD10, vecD10, vecPermN0 );		
	
		vecFirstHalf = vec_madd( vecWork1, vecWork2, zeroVector );
		vecFirstHalf2 = vec_madd( vecWork3, vecWork4, zeroVector );
		vecFirstHalf3 = vec_madd( vecWork5, vecWork6, zeroVector );
		vecFirstHalf4 = vec_madd( vecWork7, vecWork8, zeroVector );
		
		vecWork1 = vec_perm( vecD2, vecD2, vecPermN0 );
		vecWork2 = vec_perm( vecD1, vecD1, vecPermN1 );
		vecWork3 = vec_perm( vecD5, vecD5, vecPermN0 );
		vecWork4 = vec_perm( vecD4, vecD4, vecPermN1 );
		vecWork5 = vec_perm( vecD8, vecD8, vecPermN0 );
		vecWork6 = vec_perm( vecD7, vecD7, vecPermN1 );
		vecWork7 = vec_perm( vecD11, vecD11, vecPermN0 );
		vecWork8 = vec_perm( vecD10, vecD10, vecPermN1 );
		
		vecSecondHalf = vec_nmsub( vecWork1, vecWork2, vecFirstHalf );
		vecSecondHalf2 = vec_nmsub( vecWork3, vecWork4, vecFirstHalf2 );
		vecSecondHalf3 = vec_nmsub( vecWork5, vecWork6, vecFirstHalf3 );
		vecSecondHalf4 = vec_nmsub( vecWork7, vecWork8, vecFirstHalf4 );
		
		
		// calculate N values
		vecN = vec_madd( vecS2, vecSecondHalf, zeroVector );
		vecN2 = vec_madd( vecS2_2, vecSecondHalf2, zeroVector );
		vecN3 = vec_madd( vecS2_3, vecSecondHalf3, zeroVector );
		vecN4 = vec_madd( vecS2_4, vecSecondHalf4, zeroVector );
	
		// calculate both halves of the calculation for t
		vecWork1 = vecD1;
		vecWork2 = vec_perm( vecD3, vecD3, vecPermT1 );
		vecWork3 = vecD4;
		vecWork4 = vec_perm( vecD6, vecD6, vecPermT1 );
		vecWork5 = vecD7;
		vecWork6 = vec_perm( vecD9, vecD9, vecPermT1 );
		vecWork7 = vecD10;
		vecWork8 = vec_perm( vecD12, vecD12, vecPermT1 );
		
		vecFirstHalf = vec_madd( vecWork1, vecWork2, zeroVector );
		vecFirstHalf2 = vec_madd( vecWork3, vecWork4, zeroVector );
		vecFirstHalf3 = vec_madd( vecWork5, vecWork6, zeroVector );
		vecFirstHalf4 = vec_madd( vecWork7, vecWork8, zeroVector );
		
		vecWork1 = vecD2;
		vecWork2 = vec_perm( vecD3, vecD3, vecPermT0 );
		vecWork3 = vecD5;
		vecWork4 = vec_perm( vecD6, vecD6, vecPermT0 );
		vecWork5 = vecD8;
		vecWork6 = vec_perm( vecD9, vecD9, vecPermT0 );
		vecWork7 = vecD11;
		vecWork8 = vec_perm( vecD12, vecD12, vecPermT0 );		

		vecSecondHalf = vec_nmsub( vecWork1, vecWork2, vecFirstHalf );
		vecSecondHalf2 = vec_nmsub( vecWork3, vecWork4, vecFirstHalf2 );
		vecSecondHalf3 = vec_nmsub( vecWork5, vecWork6, vecFirstHalf3 );
		vecSecondHalf4 = vec_nmsub( vecWork7, vecWork8, vecFirstHalf4 );

		// calculate T values
		vecT1 = vec_madd( vecS0, vecSecondHalf, zeroVector );
		vecT1_2 = vec_madd( vecS0_2, vecSecondHalf2, zeroVector );
		vecT1_3 = vec_madd( vecS0_3, vecSecondHalf3, zeroVector );
		vecT1_4 = vec_madd( vecS0_4, vecSecondHalf4, zeroVector );

#ifndef DERIVE_UNSMOOTHED_BITANGENT
		vecWork1 = vecD1;
		vecWork2 = vec_perm( vecD2, vecD2, vecPermT2 );
		vecWork3 = vecD4;
		vecWork4 = vec_perm( vecD5, vecD5, vecPermT2 );
		vecWork5 = vecD7;
		vecWork6 = vec_perm( vecD8, vecD8, vecPermT2 );
		vecWork7 = vecD10;
		vecWork8 = vec_perm( vecD11, vecD11, vecPermT2 );

		vecSecondHalf = vec_madd( vecWork1, vecWork2, zeroVector );
		vecSecondHalf2 = vec_madd( vecWork3, vecWork4, zeroVector );
		vecSecondHalf3 = vec_madd( vecWork5, vecWork6, zeroVector );
		vecSecondHalf4 = vec_madd( vecWork7, vecWork8, zeroVector );
		
		vecWork1 = vec_perm( vecD1, vecD1, vecPermT2 );
		vecWork2 = vecD2;
		vecWork3 = vec_perm( vecD4, vecD4, vecPermT2 );
		vecWork4 = vecD5;
		vecWork5 = vec_perm( vecD7, vecD7, vecPermT2 );
		vecWork6 = vecD8;
		vecWork7 = vec_perm( vecD10, vecD10, vecPermT2 );
		vecWork8 = vecD11;

		vecFirstHalf = vec_madd( vecWork1, vecWork2, zeroVector );		
		vecFirstHalf2 = vec_madd( vecWork3, vecWork4, zeroVector );		
		vecFirstHalf3 = vec_madd( vecWork5, vecWork6, zeroVector );		
		vecFirstHalf4 = vec_madd( vecWork7, vecWork8, zeroVector );		

#else
		vecWork1 = vec_perm( vecN, vecN, vecPermN1 );
		vecWork2 = vec_perm( vecT1, vecT1, vecPermN0 );
		vecWork3 = vec_perm( vecN2, vecN2, vecPermN1 );
		vecWork4 = vec_perm( vecT1_2, vecT1_2, vecPermN0 );
		vecWork5 = vec_perm( vecN3, vecN3, vecPermN1 );
		vecWork6 = vec_perm( vecT1_3, vecT1_3, vecPermN0 );
		vecWork7 = vec_perm( vecN4, vecN4, vecPermN1 );
		vecWork8 = vec_perm( vecT1_4, vecT1_4, vecPermN0 );

		vecSecondHalf = vec_madd( vecWork1, vecWork2, zeroVector );
		vecSecondHalf2 = vec_madd( vecWork3, vecWork4, zeroVector );
		vecSecondHalf3 = vec_madd( vecWork5, vecWork6, zeroVector );
		vecSecondHalf4 = vec_madd( vecWork7, vecWork8, zeroVector );
		
		vecWork1 = vec_perm( vecN, vecN, vecPermN0 );
		vecWork2 = vec_perm( vecT1, vecT1, vecPermN1 );
		vecWork3 = vec_perm( vecN2, vecN2, vecPermN0 );
		vecWork4 = vec_perm( vecT1_2, vecT1_2, vecPermN1 );
		vecWork5 = vec_perm( vecN3, vecN3, vecPermN0 );
		vecWork6 = vec_perm( vecT1_3, vecT1_3, vecPermN1 );
		vecWork7 = vec_perm( vecN4, vecN4, vecPermN0 );
		vecWork8 = vec_perm( vecT1_4, vecT1_4, vecPermN1 );

		vecFirstHalf = vec_madd( vecWork1, vecWork2, zeroVector );
		vecFirstHalf2 = vec_madd( vecWork3, vecWork4, zeroVector );
		vecFirstHalf3 = vec_madd( vecWork5, vecWork6, zeroVector );
		vecFirstHalf4 = vec_madd( vecWork7, vecWork8, zeroVector );
#endif
		// finish the calculation
		vecSecondHalf = vec_madd( vecSecondHalf, vecNegOne, vecFirstHalf );
		vecSecondHalf2 = vec_madd( vecSecondHalf2, vecNegOne, vecFirstHalf2 );
		vecSecondHalf3 = vec_madd( vecSecondHalf3, vecNegOne, vecFirstHalf3 );
		vecSecondHalf4 = vec_madd( vecSecondHalf4, vecNegOne, vecFirstHalf4 );

		vecT2 = vec_madd( vecS1, vecSecondHalf, zeroVector );
		vecT2_2 = vec_madd( vecS1_2, vecSecondHalf2, zeroVector );
		vecT2_3 = vec_madd( vecS1_3, vecSecondHalf3, zeroVector );
		vecT2_4 = vec_madd( vecS1_4, vecSecondHalf4, zeroVector );

		// Store results

		// read values that we need to preserve
		vecLd1 = vec_ld( 0, normalPtr + ( i * DRAWVERT_OFFSET ) );
		vecLd2 = vec_ld( 32, normalPtr + ( i * DRAWVERT_OFFSET ) );
		
		//generate vectors to store
		vecStore1 = vec_perm( vecLd1, vecN, vecPermLeadAndThree );
		vecStore2 = vec_perm( vecT1, vecT2, vecPermFirstThreeLast );
		vecStore3 = vec_perm( vecT2, vecLd2, vecPermStore2 );
				
		// store out results
		ALIGNED_STORE3( normalPtr + ( i * DRAWVERT_OFFSET ), vecStore1, vecStore2, vecStore3 );

		// read values that we need to preserve
		vecLd3 = vec_ld( 32, normalPtr + ( (i+1) * DRAWVERT_OFFSET ));
		
		// generate vectors to store
		vecStore1 = vec_perm( vecN2, vecT1_2, vecPermFirstThreeLast );
		vecStore2 = vec_perm( vecT1_2, vecT2_2, vecPermStoreSecond );
		vecStore3 = vec_perm( vecT2_2, vecLd3, (vector unsigned char)(8,9,10,11,20,21,22,23,24,25,26,27,28,29,30,31) );
		
		// instead of doing permute, shift it where it needs to be and use vec_ste	
		// store out vectors
		ALIGNED_STORE3( normalPtr + ((i+1) * DRAWVERT_OFFSET), vecStore1, vecStore2, vecStore3 );

		// read values that we need to preserve
		vecLd1 = vec_ld( 0, normalPtr + ( (i+2) * DRAWVERT_OFFSET ) );

		// generate vectors to store
		vecStore1 = vec_perm( vecLd1, vecN3, vecPermFirstThreeLast );
		vecStore2 = vec_perm( vecN3, vecT1_3, vecPermStore3 );
		vecStore3 = vec_perm( vecT1_3, vecT2_3, vecPermStore4 );
		
		// store out vectors
		ALIGNED_STORE3( normalPtr + ((i+2) * DRAWVERT_OFFSET), vecStore1, vecStore2, vecStore3 );

		// read values that we need to preserve
		vecLd2 = vec_ld( 0, normalPtr + ((i+3) * DRAWVERT_OFFSET ) );
		vecLd3 = vec_ld( 32, normalPtr + ((i+3) * DRAWVERT_OFFSET ) );
		
		// generate vectors to store
		vecStore1 = vec_perm( vecLd2, vecN4, vecPermHalves );
		vecStore2 = vec_perm( vecN4, vecT1_4, vecPermStore4 );
		vecStore3 = vec_perm( vecT2_4, vecLd3, vecPermFirstThreeLast );
		
		// store out vectors
		ALIGNED_STORE3( normalPtr + ((i+3) * DRAWVERT_OFFSET ), vecStore1, vecStore2, vecStore3 );		
	}

	// cleanup
	for ( ; i < numVerts; i++ ) {
		idDrawVert *a, *b, *c;
		float d0, d1, d2, d3, d4;
		float d5, d6, d7, d8, d9;
		float s0, s1, s2;
		float n0, n1, n2;
		float t0, t1, t2;
		float t3, t4, t5;

		const dominantTri_s &dt = dominantTris[i];

		a = verts + i;
		b = verts + dt.v2;
		c = verts + dt.v3;
		
		d0 = b->xyz[0] - a->xyz[0];
		d1 = b->xyz[1] - a->xyz[1];
		d2 = b->xyz[2] - a->xyz[2];
		d3 = b->st[0] - a->st[0];
		
		d4 = b->st[1] - a->st[1];

		d5 = c->xyz[0] - a->xyz[0];
		d6 = c->xyz[1] - a->xyz[1];
		d7 = c->xyz[2] - a->xyz[2];
		d8 = c->st[0] - a->st[0];
		
		d9 = c->st[1] - a->st[1];

		s0 = dt.normalizationScale[0];
		s1 = dt.normalizationScale[1];
		s2 = dt.normalizationScale[2];

		n0 = s2 * ( d6 * d2 - d7 * d1 );
		n1 = s2 * ( d7 * d0 - d5 * d2 );
		n2 = s2 * ( d5 * d1 - d6 * d0 );

		t0 = s0 * ( d0 * d9 - d4 * d5 );
		t1 = s0 * ( d1 * d9 - d4 * d6 );
		t2 = s0 * ( d2 * d9 - d4 * d7 );

#ifndef DERIVE_UNSMOOTHED_BITANGENT
		t3 = s1 * ( d3 * d5 - d0 * d8 );
		t4 = s1 * ( d3 * d6 - d1 * d8 );
		t5 = s1 * ( d3 * d7 - d2 * d8 );
#else
		t3 = s1 * ( n2 * t1 - n1 * t2 );
		t4 = s1 * ( n0 * t2 - n2 * t0 );
		t5 = s1 * ( n1 * t0 - n0 * t1 );
#endif

		a->normal[0] = n0;
		a->normal[1] = n1;
		a->normal[2] = n2;

		a->tangents[0][0] = t0;
		a->tangents[0][1] = t1;
		a->tangents[0][2] = t2;

		a->tangents[1][0] = t3;
		a->tangents[1][1] = t4;
		a->tangents[1][2] = t5;
	}
}

#else
/*
============
idSIMD_AltiVec::DeriveUnsmoothedTangents

	Derives the normal and orthogonal tangent vectors for the triangle vertices.
	For each vertex the normal and tangent vectors are derived from a single dominant triangle.
============
*/
#define DERIVE_UNSMOOTHED_BITANGENT

void VPCALL idSIMD_AltiVec::DeriveUnsmoothedTangents( idDrawVert *verts, const dominantTri_s *dominantTris, const int numVerts ) {
	int i;
	
	for ( i = 0; i < numVerts; i++ ) {
		idDrawVert *a, *b, *c;
		float d0, d1, d2, d3, d4;
		float d5, d6, d7, d8, d9;
		float s0, s1, s2;
		float n0, n1, n2;
		float t0, t1, t2;
		float t3, t4, t5;

		const dominantTri_s &dt = dominantTris[i];

		a = verts + i;
		b = verts + dt.v2;
		c = verts + dt.v3;
		
		d0 = b->xyz[0] - a->xyz[0];
		d1 = b->xyz[1] - a->xyz[1];
		d2 = b->xyz[2] - a->xyz[2];
		d3 = b->st[0] - a->st[0];
		
		d4 = b->st[1] - a->st[1];

		d5 = c->xyz[0] - a->xyz[0];
		d6 = c->xyz[1] - a->xyz[1];
		d7 = c->xyz[2] - a->xyz[2];
		d8 = c->st[0] - a->st[0];
		
		d9 = c->st[1] - a->st[1];

		s0 = dt.normalizationScale[0];
		s1 = dt.normalizationScale[1];
		s2 = dt.normalizationScale[2];

		n0 = s2 * ( d6 * d2 - d7 * d1 );
		n1 = s2 * ( d7 * d0 - d5 * d2 );
		n2 = s2 * ( d5 * d1 - d6 * d0 );

		t0 = s0 * ( d0 * d9 - d4 * d5 );
		t1 = s0 * ( d1 * d9 - d4 * d6 );
		t2 = s0 * ( d2 * d9 - d4 * d7 );

#ifndef DERIVE_UNSMOOTHED_BITANGENT
		t3 = s1 * ( d3 * d5 - d0 * d8 );
		t4 = s1 * ( d3 * d6 - d1 * d8 );
		t5 = s1 * ( d3 * d7 - d2 * d8 );
#else
		t3 = s1 * ( n2 * t1 - n1 * t2 );
		t4 = s1 * ( n0 * t2 - n2 * t0 );
		t5 = s1 * ( n1 * t0 - n0 * t1 );
#endif

		a->normal[0] = n0;
		a->normal[1] = n1;
		a->normal[2] = n2;

		a->tangents[0][0] = t0;
		a->tangents[0][1] = t1;
		a->tangents[0][2] = t2;

		a->tangents[1][0] = t3;
		a->tangents[1][1] = t4;
		a->tangents[1][2] = t5;
	}

}
#endif /* DERIVE_UNSMOOTH_DRAWVERT_ALIGNED */

/*
============
idSIMD_AltiVec::NormalizeTangents

	Normalizes each vertex normal and projects and normalizes the
	tangent vectors onto the plane orthogonal to the vertex normal.
============
*/
void VPCALL idSIMD_AltiVec::NormalizeTangents( idDrawVert *verts, const int numVerts ) {

	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );

	float *addr = verts[0].normal.ToFloatPtr();
	float *tAddr = verts[0].tangents[0].ToFloatPtr();

	// v0 through v3 maintain originally loaded values so we don't take
	// as much hit for unaligned stores
	vector float v0, v1, v2, v3;
	// v5 through v8 are the "working" values of the vectors
	vector float v5, v6, v7, v8;	
	// working values
	vector float vec1T0, vec1T1, vec2T0, vec2T1, vec3T0, vec3T1, vec4T0, vec4T1;
	vector float vecSum, vecTSum1, vecTSum2, tempSum, tempSum2, tempSum3;
	vector float vecF, vecF2;
	vector float vecTemp, vecTemp2, vecTemp3, vecTemp4;
	
	register vector float zeroVector = (vector float)(0.0);
	
	vector unsigned char vecPermHalves = (vector unsigned char)(0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23);
	vector unsigned char vecPermLast = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,16,17,18,19);
	vector unsigned char vecPermSplatFirstWithZero = (vector unsigned char)(0,1,2,3,0,1,2,3,0,1,2,3,16,17,18,19);
	vector unsigned char vecPerm0, vecPerm1, vecPerm2, vecPerm3;
	vector unsigned char storePerm0, storePerm1, storePerm2, storePerm3;

	vector float vecTan11, vecTan12, vecTan13, vecTan21, vecTan22, vecTan23;
	vector float vecTan31, vecTan32, vecTan33, vecTan41, vecTan42, vecTan43;
	
	vector unsigned char vec1T0Perm, vec1T1Perm, vec2T0Perm, vec2T1Perm, vec3T0Perm, vec3T1Perm, vec4T0Perm, vec4T1Perm;
	vector unsigned char storeT11, storeT12, storeT21, storeT22, storeT31, storeT32;
	vector unsigned char storeT41, storeT42;
	
	int i = 0;	
		
	if ( i+3 < numVerts ) {
		// for loading normal from idDrawVert
		vecPerm0 = vec_add( vec_lvsl( -1, addr ), (vector unsigned char)(1) );
		vecPerm1 = vec_add( vec_lvsl( -1, addr + ( 1 * DRAWVERT_OFFSET ) ), (vector unsigned char)(1) );
		vecPerm2 = vec_add( vec_lvsl( -1, addr + ( 2 * DRAWVERT_OFFSET ) ), (vector unsigned char)(1) );
		vecPerm3 = vec_add( vec_lvsl( -1, addr + ( 3 * DRAWVERT_OFFSET ) ), (vector unsigned char)(1) );
	
		// for loading tangents from idDrawVert
		vec1T0Perm = vec_add( vec_lvsl( -1, tAddr + ( 0 * DRAWVERT_OFFSET ) ), (vector unsigned char)(1) );
		vec1T1Perm = vec_add( vec_lvsl( -1, tAddr + ( 0 * DRAWVERT_OFFSET ) + 3 ), (vector unsigned char)(1) );
		vec2T0Perm = vec_add( vec_lvsl( -1, tAddr + ( 1 * DRAWVERT_OFFSET ) ), (vector unsigned char)(1) );
		vec2T1Perm = vec_add( vec_lvsl( -1, tAddr + ( 1 * DRAWVERT_OFFSET ) + 3 ), (vector unsigned char)(1) );
		vec3T0Perm = vec_add( vec_lvsl( -1, tAddr + ( 2 * DRAWVERT_OFFSET ) ), (vector unsigned char)(1) );
		vec3T1Perm = vec_add( vec_lvsl( -1, tAddr + ( 2 * DRAWVERT_OFFSET ) + 3 ), (vector unsigned char)(1) );
		vec4T0Perm = vec_add( vec_lvsl( -1, tAddr + ( 3 * DRAWVERT_OFFSET ) ), (vector unsigned char)(1) );
		vec4T1Perm = vec_add( vec_lvsl( -1, tAddr + ( 3 * DRAWVERT_OFFSET ) + 3 ), (vector unsigned char)(1) );						
						
		// generate permute vectors to store normals
		storePerm0 = vec_lvsr( 0, addr );
		storePerm1 = vec_lvsr( 0, addr + ( 1 * DRAWVERT_OFFSET ) );
		storePerm2 = vec_lvsr( 0, addr + ( 2 * DRAWVERT_OFFSET ) );
		storePerm3 = vec_lvsr( 0, addr + ( 3 * DRAWVERT_OFFSET ) );

		// generate permute vectors to store tangents
		storeT11 = vec_lvsr( 0, tAddr + ( 0 * DRAWVERT_OFFSET ) );
		storeT12 = vec_lvsr( 12, tAddr + ( 0 * DRAWVERT_OFFSET ) );

		storeT21 = vec_lvsr( 0, tAddr + ( 1 * DRAWVERT_OFFSET ) );
		storeT22 = vec_lvsr( 12, tAddr + ( 1 * DRAWVERT_OFFSET ) );
		
		storeT31 = vec_lvsr( 0, tAddr + ( 2 * DRAWVERT_OFFSET ) );
		storeT32 = vec_lvsr( 12, tAddr + ( 2 * DRAWVERT_OFFSET ) );
		
		storeT41 = vec_lvsr( 0, tAddr + ( 3 * DRAWVERT_OFFSET ) );
		storeT42 = vec_lvsr( 12, tAddr + ( 3 * DRAWVERT_OFFSET ) );
	}
	
	for ( ; i+3 < numVerts; i+=4 ) {
		
		// load normals
		vector float vecNormal11 = vec_ld( 0, addr + ( i * DRAWVERT_OFFSET ) );
		vector float vecNormal12 = vec_ld( 15, addr + ( i * DRAWVERT_OFFSET ) ); 
		v0 = vec_perm( vecNormal11, vecNormal12, vecPerm0 );
		
		vector float vecNormal21 = vec_ld( 0, addr + ((i+1) * DRAWVERT_OFFSET ) );
		vector float vecNormal22 = vec_ld( 15, addr + ((i+1) * DRAWVERT_OFFSET ) );
		v1 = vec_perm( vecNormal21, vecNormal22, vecPerm1 );
		
		vector float vecNormal31 = vec_ld( 0, addr + ( (i+2) * DRAWVERT_OFFSET ) );
		vector float vecNormal32 = vec_ld( 15, addr + ( (i+2) * DRAWVERT_OFFSET ) ); 
		v2 = vec_perm( vecNormal31, vecNormal32, vecPerm2 );
		
		vector float vecNormal41 = vec_ld( 0, addr + ((i+3) * DRAWVERT_OFFSET ) );
		vector float vecNormal42 = vec_ld( 15, addr + ((i+3) * DRAWVERT_OFFSET ) );
		v3 = vec_perm( vecNormal41, vecNormal42, vecPerm3 );
		
		// zero out the last element of each useless vector
		v0 = vec_perm( v0, zeroVector, vecPermLast );
		v1 = vec_perm( v1, zeroVector, vecPermLast );
		v2 = vec_perm( v2, zeroVector, vecPermLast );
		v3 = vec_perm( v3, zeroVector, vecPermLast );

		// got 4 vectors in v0 through v3, sum them each accross
		// and put into one vector
		vecTemp = vec_madd( v0, v0, zeroVector );
		
		vecSum = vec_add( vecTemp, vec_sld( vecTemp, vecTemp, 8 ) );
		vecSum = vec_add( vecSum, vec_sld( vecSum, vecSum, 4 ) );
		// element 0 of vecSum now has sum of v0
		
		vecTemp2 = vec_madd( v1, v1, zeroVector );
		tempSum = vec_add( vecTemp2, vec_sld( vecTemp2, vecTemp2, 8 ) );
		tempSum = vec_add( tempSum, vec_sld( tempSum, tempSum, 4 ) );
		// put this into vecSum
		vecSum = vec_mergeh( vecSum, tempSum );
		
		vecTemp3 = vec_madd( v2, v2, zeroVector );
		tempSum = vec_add( vecTemp3, vec_sld( vecTemp3, vecTemp3, 8 ) );
		tempSum = vec_add( tempSum, vec_sld( tempSum, tempSum, 4 ) );		
		// put this into vecSum
		vecSum = vec_perm( vecSum, tempSum, vecPermHalves );
		
		vecTemp4 = vec_madd( v3, v3, zeroVector );
		tempSum = vec_add( vecTemp4, vec_sld( vecTemp4, vecTemp4, 8 ) );
		tempSum = vec_add( tempSum, vec_sld( tempSum, tempSum, 4 ) );
		// put this into vecSum
		vecSum = vec_perm( vecSum, tempSum, vecPermLast );

		// take reciprocal square roots of these
		vecF = ReciprocalSquareRoot( vecSum );
		
		// multiply each vector by f
		v5 = vec_madd( v0, vec_splat( vecF, 0 ), zeroVector );
		v6 = vec_madd( v1, vec_splat( vecF, 1 ), zeroVector );
		v7 = vec_madd( v2, vec_splat( vecF, 2 ), zeroVector );
		v8 = vec_madd( v3, vec_splat( vecF, 3 ), zeroVector );

		// load tangents as unaligned
		vecTan11 = vec_ld( 0, tAddr + ( i * DRAWVERT_OFFSET ) );
		vecTan12 = vec_ld( 11, tAddr + ( i * DRAWVERT_OFFSET ) );
		vecTan13 = vec_ld( 23, tAddr + ( i * DRAWVERT_OFFSET ) );

		vecTan21 = vec_ld( 0, tAddr + ( (i+1) * DRAWVERT_OFFSET ) );
		vecTan22 = vec_ld( 11, tAddr + ( (i+1) * DRAWVERT_OFFSET ) );
		vecTan23 = vec_ld( 23, tAddr + ( (i+1) * DRAWVERT_OFFSET ) );
		
		vecTan31 = vec_ld( 0, tAddr + ( (i+2) * DRAWVERT_OFFSET ) );
		vecTan32 = vec_ld( 11, tAddr + ( (i+2) * DRAWVERT_OFFSET ) );
		vecTan33 = vec_ld( 23, tAddr + ( (i+2) * DRAWVERT_OFFSET ) );
		
		vecTan41 = vec_ld( 0, tAddr + ( (i+3) * DRAWVERT_OFFSET ) );
		vecTan42 = vec_ld( 11, tAddr + ( (i+3) * DRAWVERT_OFFSET ) );
		vecTan43 = vec_ld( 23, tAddr + ( (i+3) * DRAWVERT_OFFSET ) );

		vec1T0 = vec_perm( vecTan11, vecTan12, vec1T0Perm );
		vec1T1 = vec_perm( vecTan12, vecTan13, vec1T1Perm );
		vec2T0 = vec_perm( vecTan21, vecTan22, vec2T0Perm );
		vec2T1 = vec_perm( vecTan22, vecTan23, vec2T1Perm );
		vec3T0 = vec_perm( vecTan31, vecTan32, vec3T0Perm );
		vec3T1 = vec_perm( vecTan32, vecTan33, vec3T1Perm );
		vec4T0 = vec_perm( vecTan41, vecTan42, vec4T0Perm );
		vec4T1 = vec_perm( vecTan42, vecTan43, vec4T1Perm );

		//zero out last element of tangents
		vec1T0 = vec_perm( vec1T0, zeroVector, vecPermLast );
		vec1T1 = vec_perm( vec1T1, zeroVector, vecPermLast );
		vec2T0 = vec_perm( vec2T0, zeroVector, vecPermLast );
		vec2T1 = vec_perm( vec2T1, zeroVector, vecPermLast );
		vec3T0 = vec_perm( vec3T0, zeroVector, vecPermLast );
		vec3T1 = vec_perm( vec3T1, zeroVector, vecPermLast );
		vec4T0 = vec_perm( vec4T0, zeroVector, vecPermLast );
		vec4T1 = vec_perm( vec4T1, zeroVector, vecPermLast );

		// all tangents[0]
		tempSum = zeroVector;
		tempSum = vec_madd( vec1T0, v5, tempSum );
		//sum accross tempSum
		vecTSum1 = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vecTSum1 = vec_add( vecTSum1, vec_sld( vecTSum1, vecTSum1, 4 ) );
		//  put tempSum splatted accross vecTSum1
		vecTSum1 = vec_perm( vecTSum1, zeroVector, vecPermSplatFirstWithZero );																																					
		vecTSum1 = vec_madd( vecTSum1, v5, zeroVector );
		
		//vec1T0 now contains what needs to be rsqrt'd and multiplied by f
		vec1T0 = vec_sub( vec1T0, vecTSum1 );
		
		tempSum = zeroVector;
		tempSum = vec_madd( vec2T0, v6, tempSum );
		
		//sum accross tempSum
		vecTSum1 = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vecTSum1 = vec_add( vecTSum1, vec_sld( vecTSum1, vecTSum1, 4 ) );				
		vecTSum1 = vec_perm( vecTSum1, zeroVector, vecPermSplatFirstWithZero );
		vecTSum1 = vec_madd( vecTSum1, v6, zeroVector );
		vec2T0 = vec_sub( vec2T0, vecTSum1 );
		
		tempSum = zeroVector;
		tempSum = vec_madd( vec3T0, v7, tempSum );
		
		//sum accross tempSum
		vecTSum1 = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vecTSum1 = vec_add( vecTSum1, vec_sld( vecTSum1, vecTSum1, 4 ) );				
		vecTSum1 = vec_perm( vecTSum1, zeroVector, vecPermSplatFirstWithZero );
		vecTSum1 = vec_madd( vecTSum1, v7, zeroVector );
		vec3T0 = vec_sub( vec3T0, vecTSum1 );
		
		tempSum = zeroVector;
		tempSum = vec_madd( vec4T0, v8, tempSum );
		
		//sum accross tempSum
		vecTSum1 = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vecTSum1 = vec_add( vecTSum1, vec_sld( vecTSum1, vecTSum1, 4 ) );				
		vecTSum1 = vec_perm( vecTSum1, zeroVector, vecPermSplatFirstWithZero );
		vecTSum1 = vec_madd( vecTSum1, v8, zeroVector );
		vec4T0 = vec_sub( vec4T0, vecTSum1 );
				
		// all tangents[1]
		tempSum = zeroVector;
		tempSum = vec_madd( vec1T1, v5, tempSum );
		
		//sum accross tempSum
		vecTSum1 = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vecTSum1 = vec_add( vecTSum1, vec_sld( vecTSum1, vecTSum1, 4 ) );				
		vecTSum1 = vec_perm( vecTSum1, zeroVector, vecPermSplatFirstWithZero );	
		vecTSum1 = vec_madd( vecTSum1, v5, zeroVector );
		
		//vec1T0 now contains what needs to be rsqrt'd and multiplied by f
		vec1T1 = vec_sub( vec1T1, vecTSum1 );		
		
		tempSum = zeroVector;
		tempSum = vec_madd( vec2T1, v6, tempSum );
		
		//sum accross tempSum
		vecTSum1 = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vecTSum1 = vec_add( vecTSum1, vec_sld( vecTSum1, vecTSum1, 4 ) );				
		vecTSum1 = vec_perm( vecTSum1, zeroVector, vecPermSplatFirstWithZero );
		vecTSum1 = vec_madd( vecTSum1, v6, zeroVector );
		vec2T1 = vec_sub( vec2T1, vecTSum1 );
		
		tempSum = zeroVector;
		tempSum = vec_madd( vec3T1, v7, tempSum );
		
		//sum accross tempSum
		vecTSum1 = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vecTSum1 = vec_add( vecTSum1, vec_sld( vecTSum1, vecTSum1, 4 ) );				
		vecTSum1 = vec_perm( vecTSum1, zeroVector, vecPermSplatFirstWithZero );
		vecTSum1 = vec_madd( vecTSum1, v7, zeroVector );
		vec3T1 = vec_sub( vec3T1, vecTSum1 );
		
		tempSum = zeroVector;
		tempSum = vec_madd( vec4T1, v8, tempSum );
		
		//sum accross tempSum
		vecTSum1 = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vecTSum1 = vec_add( vecTSum1, vec_sld( vecTSum1, vecTSum1, 4 ) );				
		vecTSum1 = vec_perm( vecTSum1, zeroVector, vecPermSplatFirstWithZero );
		vecTSum1 = vec_madd( vecTSum1, v8, zeroVector );
		vec4T1 = vec_sub( vec4T1, vecTSum1 );


		// sum accross vectors and put into one vector
		vecTemp = vec_madd( vec1T0, vec1T0, zeroVector );
		vecTSum1 = vec_add( vecTemp, vec_sld( vecTemp, vecTemp, 8 ) );
		vecTSum1 = vec_add( vecTSum1, vec_sld( vecTSum1, vecTSum1, 4 ) );

		// element 0 of vecSum now has sum of v0
		vecTemp = vec_madd( vec2T0, vec2T0, zeroVector );
		tempSum2 = vec_add( vecTemp, vec_sld( vecTemp, vecTemp, 8 ) );
		tempSum2 = vec_add( tempSum2, vec_sld( tempSum2, tempSum2, 4 ) );
		// put this into vecSum
		vecTemp = vec_madd( vec3T0, vec3T0, zeroVector );
		vecTSum1 = vec_mergeh( vecTSum1, tempSum2 );
		tempSum2 = vec_add( vecTemp, vec_sld( vecTemp, vecTemp, 8 ) );
		tempSum2 = vec_add( tempSum2, vec_sld( tempSum2, tempSum2, 4 ) );		
		// put this into vecSum
		vecTSum1 = vec_perm( vecTSum1, tempSum2, vecPermHalves );
		vecTemp = vec_madd( vec4T0, vec4T0, zeroVector );
		tempSum2 = vec_add( vecTemp, vec_sld( vecTemp, vecTemp, 8 ) );
		tempSum2 = vec_add( tempSum2, vec_sld( tempSum2, tempSum2, 4 ) );
		// put this into vecSum
		vecTSum1 = vec_perm( vecTSum1, tempSum2, vecPermLast );
		
		vecTemp = vec_madd( vec1T1, vec1T1, zeroVector );
		vecTSum2 = vec_add( vecTemp, vec_sld( vecTemp, vecTemp, 8 ) );
		vecTSum2 = vec_add( vecTSum2, vec_sld( vecTSum2, vecTSum2, 4 ) );
		// element 0 of vecSum now has sum of v0
		vecTemp = vec_madd( vec2T1, vec2T1, zeroVector );
		tempSum3 = vec_add( vecTemp, vec_sld( vecTemp, vecTemp, 8 ) );
		tempSum3 = vec_add( tempSum3, vec_sld( tempSum3, tempSum3, 4 ) );
		// put this into vecSum
		vecTSum2 = vec_mergeh( vecTSum2, tempSum3 );
		vecTemp = vec_madd( vec3T1, vec3T1, zeroVector );
		tempSum3 = vec_add( vecTemp, vec_sld( vecTemp, vecTemp, 8 ) );
		tempSum3 = vec_add( tempSum3, vec_sld( tempSum3, tempSum3, 4 ) );		
		// put this into vecSum
		vecTSum2 = vec_perm( vecTSum2, tempSum3, vecPermHalves );
		vecTemp = vec_madd( vec4T1, vec4T1, zeroVector );
		tempSum3 = vec_add( vecTemp, vec_sld( vecTemp, vecTemp, 8 ) );
		tempSum3 = vec_add( tempSum3, vec_sld( tempSum3, tempSum3, 4 ) );
		// put this into vecSum
		vecTSum2 = vec_perm( vecTSum2, tempSum3, vecPermLast );
		
		// tangents[0]
		vecF = ReciprocalSquareRoot( vecTSum1 );
		// tangents[1]
		vecF2 = ReciprocalSquareRoot( vecTSum2 );
		
		// multiply each tangent vector by f

		vec1T0 = vec_madd( vec1T0, vec_splat( vecF, 0 ), zeroVector );
		vec2T0 = vec_madd( vec2T0, vec_splat( vecF, 1 ), zeroVector );
		vec3T0 = vec_madd( vec3T0, vec_splat( vecF, 2 ), zeroVector );
		vec4T0 = vec_madd( vec4T0, vec_splat( vecF, 3 ), zeroVector );

		vec1T1 = vec_madd( vec1T1, vec_splat( vecF2, 0 ), zeroVector );
		vec2T1 = vec_madd( vec2T1, vec_splat( vecF2, 1 ), zeroVector );
		vec3T1 = vec_madd( vec3T1, vec_splat( vecF2, 2 ), zeroVector );
		vec4T1 = vec_madd( vec4T1, vec_splat( vecF2, 3 ), zeroVector );

		// rotate input data
		v5 = vec_perm( v5, v5, storePerm0 );
		v6 = vec_perm( v6, v6, storePerm1 );
		v7 = vec_perm( v7, v7, storePerm2 );
		v8 = vec_perm( v8, v8, storePerm3 );

		vec_ste( v5, 0, addr + ( (i+0) * DRAWVERT_OFFSET ) );
		vec_ste( v5, 4, addr + ( (i+0) * DRAWVERT_OFFSET ) );
		vec_ste( v5, 8, addr + ( (i+0) * DRAWVERT_OFFSET ) );

		vec_ste( v6, 0, addr + ( (i+1) * DRAWVERT_OFFSET ) );
		vec_ste( v6, 4, addr + ( (i+1) * DRAWVERT_OFFSET ) );
		vec_ste( v6, 8, addr + ( (i+1) * DRAWVERT_OFFSET ) );
		
		vec_ste( v7, 0, addr + ( (i+2) * DRAWVERT_OFFSET ) );
		vec_ste( v7, 4, addr + ( (i+2) * DRAWVERT_OFFSET ) );
		vec_ste( v7, 8, addr + ( (i+2) * DRAWVERT_OFFSET ) );

		vec_ste( v8, 0, addr + ( (i+3) * DRAWVERT_OFFSET ) );
		vec_ste( v8, 4, addr + ( (i+3) * DRAWVERT_OFFSET ) );
		vec_ste( v8, 8, addr + ( (i+3) * DRAWVERT_OFFSET ) );

		// store tangents[0] and tangents[1]
		vec1T0 = vec_perm( vec1T0, vec1T0, storeT11 );
		vec1T1 = vec_perm( vec1T1, vec1T1, storeT12 );
		
		vec_ste( vec1T0, 0, tAddr + ((i+0) * DRAWVERT_OFFSET ) );
		vec_ste( vec1T0, 4, tAddr + ((i+0) * DRAWVERT_OFFSET ) );
		vec_ste( vec1T0, 8, tAddr + ((i+0) * DRAWVERT_OFFSET ) );
		vec_ste( vec1T1, 12, tAddr + ((i+0) * DRAWVERT_OFFSET ) );
		vec_ste( vec1T1, 16, tAddr + ((i+0) * DRAWVERT_OFFSET ) );
		vec_ste( vec1T1, 20, tAddr + ((i+0) * DRAWVERT_OFFSET ) );

		// store second tangents[0] and tangents[1]
		vec2T0 = vec_perm( vec2T0, vec2T0, storeT21 );
		vec2T1 = vec_perm( vec2T1, vec2T1, storeT22 );
		
		vec_ste( vec2T0, 0, tAddr + ((i+1) * DRAWVERT_OFFSET ) );
		vec_ste( vec2T0, 4, tAddr + ((i+1) * DRAWVERT_OFFSET ) );
		vec_ste( vec2T0, 8, tAddr + ((i+1) * DRAWVERT_OFFSET ) );
		vec_ste( vec2T1, 12, tAddr + ((i+1) * DRAWVERT_OFFSET ) );
		vec_ste( vec2T1, 16, tAddr + ((i+1) * DRAWVERT_OFFSET ) );
		vec_ste( vec2T1, 20, tAddr + ((i+1) * DRAWVERT_OFFSET ) );

		// store third tangents[0] and tangents[1]		
		vec3T0 = vec_perm( vec3T0, vec3T0, storeT31 );
		vec3T1 = vec_perm( vec3T1, vec3T1, storeT32 );
		
		vec_ste( vec3T0, 0, tAddr + ((i+2) * DRAWVERT_OFFSET ) );
		vec_ste( vec3T0, 4, tAddr + ((i+2) * DRAWVERT_OFFSET ) );
		vec_ste( vec3T0, 8, tAddr + ((i+2) * DRAWVERT_OFFSET ) );
		vec_ste( vec3T1, 12, tAddr + ((i+2) * DRAWVERT_OFFSET ) );
		vec_ste( vec3T1, 16, tAddr + ((i+2) * DRAWVERT_OFFSET ) );
		vec_ste( vec3T1, 20, tAddr + ((i+2) * DRAWVERT_OFFSET ) );

		// store fourth tangents[0] and tangents[1]
		vec4T0 = vec_perm( vec4T0, vec4T0, storeT41 );
		vec4T1 = vec_perm( vec4T1, vec4T1, storeT42 );
		
		vec_ste( vec4T0, 0, tAddr + ((i+3) * DRAWVERT_OFFSET ) );
		vec_ste( vec4T0, 4, tAddr + ((i+3) * DRAWVERT_OFFSET ) );
		vec_ste( vec4T0, 8, tAddr + ((i+3) * DRAWVERT_OFFSET ) );
		vec_ste( vec4T1, 12, tAddr + ((i+3) * DRAWVERT_OFFSET ) );
		vec_ste( vec4T1, 16, tAddr + ((i+3) * DRAWVERT_OFFSET ) );
		vec_ste( vec4T1, 20, tAddr + ((i+3) * DRAWVERT_OFFSET ) );
	}

	// cleanup
	for ( ; i < numVerts; i++ ) {
		idVec3 &v = verts[i].normal;
		float f;

		//f = idMath::RSqrt( v.x * v.x + v.y * v.y + v.z * v.z );
		f = FastScalarInvSqrt( v.x * v.x + v.y * v.y + v.z * v.z );
		v.x *= f; v.y *= f; v.z *= f;

		for ( int j = 0; j < 2; j++ ) {
			idVec3 &t = verts[i].tangents[j];

			t -= ( t * v ) * v;
		//	f = idMath::RSqrt( t.x * t.x + t.y * t.y + t.z * t.z );
			f = FastScalarInvSqrt( t.x * t.x + t.y * t.y + t.z * t.z );
			t.x *= f; t.y *= f; t.z *= f;
		}
	}
}
#endif /* ENABLE_DERIVE */

#ifdef ENABLE_CREATE

/*
============
idSIMD_AltiVec::CreateTextureSpaceLightVectors

	Calculates light vectors in texture space for the given triangle vertices.
	For each vertex the direction towards the light origin is projected onto texture space.
	The light vectors are only calculated for the vertices referenced by the indexes.
============
*/

void VPCALL idSIMD_AltiVec::CreateTextureSpaceLightVectors( idVec3 *lightVectors, const idVec3 &lightOrigin, const idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes ) {
	
	bool *used = (bool *)_alloca16( numVerts * sizeof( used[0] ) );
	memset( used, 0, numVerts * sizeof( used[0] ) );

	int i;
	for ( i = 0; i+7 < numIndexes; i+= 8 ) {
		used[indexes[i]] = true;
		used[indexes[i+1]] = true;
		used[indexes[i+2]] = true;
		used[indexes[i+3]] = true;
		used[indexes[i+4]] = true;
		used[indexes[i+5]] = true;
		used[indexes[i+6]] = true;
		used[indexes[i+7]] = true;	
	}
	
	for ( ; i < numIndexes; i++ ) {
		used[indexes[i]] = true;
	}

	for ( i = 0; i+1 < numVerts; i+=2 ) {

		const idDrawVert *v = &verts[i];
		const idDrawVert *v2 = &verts[i+1];
		
		float x, y, z;
		float x2, y2, z2;
		idVec3 lightDir, lightDir2;

		lightDir[0] = lightOrigin[0] - v->xyz[0];
		lightDir[1] = lightOrigin[1] - v->xyz[1];
		lightDir[2] = lightOrigin[2] - v->xyz[2];
		
		lightDir2[0] = lightOrigin[0] - v2->xyz[0];
		lightDir2[1] = lightOrigin[1] - v2->xyz[1];
		lightDir2[2] = lightOrigin[2] - v2->xyz[2];
		
		x = lightDir[0] * v->tangents[0][0] + lightDir[1] * v->tangents[0][1] + lightDir[2] * v->tangents[0][2];
		y = lightDir[0] * v->tangents[1][0] + lightDir[1] * v->tangents[1][1] + lightDir[2] * v->tangents[1][2];
		z = lightDir[0] * v->normal[0] + lightDir[1] * v->normal[1] + lightDir[2] * v->normal[2];
	
		x2 = lightDir2[0] * v2->tangents[0][0] + lightDir2[1] * v2->tangents[0][1] + lightDir2[2] * v2->tangents[0][2];
		y2 = lightDir2[0] * v2->tangents[1][0] + lightDir2[1] * v2->tangents[1][1] + lightDir2[2] * v2->tangents[1][2];
		z2 = lightDir2[0] * v2->normal[0] + lightDir2[1] * v2->normal[1] + lightDir2[2] * v2->normal[2];
				
		if ( used[i] ) {
			lightVectors[i][0] = x;
			lightVectors[i][1] = y;
			lightVectors[i][2] = z;
		}
		
		if ( used[i+1] ) {
			lightVectors[i+1][0] = x2;
			lightVectors[i+1][1] = y2;
			lightVectors[i+1][2] = z2;
		}
	}

	// cleanup
	for ( ; i < numVerts; i++ ) {
		if ( !used[i] ) {
			continue;
		}

		const idDrawVert *v = &verts[i];
		idVec3 lightDir;

		lightDir[0] = lightOrigin[0] - v->xyz[0];
		lightDir[1] = lightOrigin[1] - v->xyz[1];
		lightDir[2] = lightOrigin[2] - v->xyz[2];

		lightVectors[i][0] = lightDir[0] * v->tangents[0][0] + lightDir[1] * v->tangents[0][1] + lightDir[2] * v->tangents[0][2];
		lightVectors[i][1] = lightDir[0] * v->tangents[1][0] + lightDir[1] * v->tangents[1][1] + lightDir[2] * v->tangents[1][2];
		lightVectors[i][2] = lightDir[0] * v->normal[0] + lightDir[1] * v->normal[1] + lightDir[2] * v->normal[2];
	}
}

#if 1
/*
============
idSIMD_AltiVec::CreateSpecularTextureCoords

	Calculates specular texture coordinates for the given triangle vertices.
	For each vertex the normalized direction towards the light origin is added to the
	normalized direction towards the view origin and the result is projected onto texture space.
	The texture coordinates are only calculated for the vertices referenced by the indexes.
============
*/
void VPCALL idSIMD_AltiVec::CreateSpecularTextureCoords( idVec4 *texCoords, const idVec3 &lightOrigin, const idVec3 &viewOrigin, const idDrawVert *verts, const int numVerts, const int *indexes, const int numIndexes ) {
		
	bool *used = (bool *)_alloca16( numVerts * sizeof( used[0] ) );
	memset( used, 0, numVerts * sizeof( used[0] ) );

	int i;
	for ( i = 0; i+7 < numIndexes; i+= 8 ) {
		used[indexes[i]] = true;
		used[indexes[i+1]] = true;
		used[indexes[i+2]] = true;
		used[indexes[i+3]] = true;
		used[indexes[i+4]] = true;
		used[indexes[i+5]] = true;
		used[indexes[i+6]] = true;
		used[indexes[i+7]] = true;	
	}
	
	for ( ; i < numIndexes; i++ ) {
		used[indexes[i]] = true;
	}

	// load lightOrigin and viewOrigin into vectors
	const float *lightOriginPtr = lightOrigin.ToFloatPtr();
	const float *viewOriginPtr = viewOrigin.ToFloatPtr();
	vector unsigned char permVec = vec_lvsl( 0, lightOriginPtr );
	vector unsigned char permVec2 = vec_lvsl( 0, viewOriginPtr );
	vector float v0 = vec_ld( 0, lightOriginPtr );
	vector float v1 = vec_ld( 15, lightOriginPtr );
	vector float v2 = vec_ld( 0, viewOriginPtr );
	vector float v3 = vec_ld( 15, viewOriginPtr );
	vector float vecLightOrigin = vec_perm( v0, v1, permVec );
	vector float vecViewOrigin = vec_perm( v2, v3, permVec2 );
	const vector float zeroVector = (vector float)(0);
	int index;

	for ( index = 0; index+1 < numVerts; index+=2 ) {
		const float *vertPtr = verts[index].xyz.ToFloatPtr();
		const float *vertPtr2 = verts[index+1].xyz.ToFloatPtr();

		permVec = vec_add( vec_lvsl( -1, vertPtr ), (vector unsigned char)(1) );
		permVec2 = vec_add( vec_lvsl( -1, vertPtr2 ), (vector unsigned char)(1) );
		
		v0 = vec_ld( 0, vertPtr );
		v1 = vec_ld( 15, vertPtr );
		vector float v2 = vec_ld( 31, vertPtr );
		vector float v3 = vec_ld( 47, vertPtr );  
		vector float v4 = vec_ld( 63, vertPtr ); 
		
		vector float v5 = vec_ld( 0, vertPtr2 );
		vector float v6 = vec_ld( 15, vertPtr2 );
		vector float v7 = vec_ld( 31, vertPtr2 );
		vector float v8 = vec_ld( 47, vertPtr2 );
		vector float v9 = vec_ld( 63, vertPtr2 );
		
		// figure out what values go where
		vector float vecXYZ = vec_perm( v0, v1, permVec );
		vector float vecNormal = vec_perm( v1, v2, permVec );
		vecNormal = vec_sld( vecNormal, vecNormal, 4 );
		const vector float vecTangent0 = vec_perm( v2, v3, permVec );
		permVec = vec_add( permVec, (vector unsigned char)(-4) ); //shift permute right 3 elements
		const vector float vecTangent1 = vec_perm( v3, v4, permVec );
		
		vector float vecXYZ2 = vec_perm( v5, v6, permVec2 );
		vector float vecNormal2 = vec_perm( v6, v7, permVec2 );
		vecNormal2 = vec_sld( vecNormal2, vecNormal2, 4 );
		const vector float vecTangent02 = vec_perm( v7, v8, permVec2 );
		permVec2 = vec_add( permVec2, (vector unsigned char)(-4) );
		const vector float vecTangent12 = vec_perm( v8, v9, permVec2 );
		
		// calculate lightDir
		vector float vecLightDir = vec_sub( vecLightOrigin, vecXYZ );
		vector float vecViewDir = vec_sub( vecViewOrigin, vecXYZ );
		
		vector float vecLightDir2 = vec_sub( vecLightOrigin, vecXYZ2 );
		vector float vecViewDir2 = vec_sub( vecViewOrigin, vecXYZ2 );

		// calculate distance
		vector float vecTempLight = vec_madd( vecLightDir, vecLightDir, zeroVector );
		vector float vecTempView = vec_madd( vecViewDir, vecViewDir, zeroVector );
		
		vector float vecTempLight2 = vec_madd( vecLightDir2, vecLightDir2, zeroVector );
		vector float vecTempView2 = vec_madd( vecViewDir2, vecViewDir2, zeroVector );
		
		// sum accross first 3 elements of vector
		vector float tempSum = vec_add( vecTempLight, vec_sld( vecTempLight, vecTempLight, 4 ) );
		vecTempLight = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vector float tempSum2 = vec_add( vecTempView, vec_sld( vecTempView, vecTempView, 4 ) );
		vecTempView = vec_add( tempSum2, vec_sld( tempSum2, tempSum2, 8 ) );
		
		vector float tempSum4 = vec_add( vecTempLight2, vec_sld( vecTempLight2, vecTempLight2, 4 ) );
		vecTempLight2 = vec_add( tempSum4, vec_sld( tempSum4, tempSum4, 8 ) );
		vector float tempSum5 = vec_add( vecTempView2, vec_sld( vecTempView2, vecTempView2, 4 ) );
		vecTempView2 = vec_add( tempSum5, vec_sld( tempSum5, tempSum5, 8 ) );
		
		// splat sum accross the whole vector
		vecTempLight = vec_splat( vecTempLight, 0 );
		vecTempView = vec_splat( vecTempView, 0 );
		
		vecTempLight2 = vec_splat( vecTempLight2, 0 );
		vecTempView2 = vec_splat( vecTempView2, 0 );
		
		vecTempLight = ReciprocalSquareRoot( vecTempLight );
		vecTempView = ReciprocalSquareRoot( vecTempView );
		
		vecTempLight2 = ReciprocalSquareRoot( vecTempLight2 );
		vecTempView2 = ReciprocalSquareRoot( vecTempView2 );

		// modify light and view vectors based on ilength
		vecViewDir = vec_madd( vecViewDir, vecTempView, zeroVector );
		vecLightDir = vec_madd( vecLightDir, vecTempLight, vecViewDir );
		
		vecViewDir2 = vec_madd( vecViewDir2, vecTempView2, zeroVector );
		vecLightDir2 = vec_madd( vecLightDir2, vecTempLight2, vecViewDir2 );
				
		// calculate what to store in each texture coord
		vector float vecTC0 = vec_madd( vecLightDir, vecTangent0, zeroVector );
		vector float vecTC1 = vec_madd( vecLightDir, vecTangent1, zeroVector );
		vector float vecTC2 = vec_madd( vecLightDir, vecNormal, zeroVector );		
		
		vector float vecTC3 = vec_madd( vecLightDir2, vecTangent02, zeroVector );
		vector float vecTC4 = vec_madd( vecLightDir2, vecTangent12, zeroVector );
		vector float vecTC5 = vec_madd( vecLightDir2, vecNormal2, zeroVector );

		// sum accross first 3 elements of vector
		vector float tempSum3;
		tempSum = vec_add( vecTC0, vec_sld( vecTC0, vecTC0, 4 ) );
		vecTC0 = vec_add( tempSum, vec_sld( vecTC0, vecTC0, 8 ) );
		tempSum2 = vec_add( vecTC1, vec_sld( vecTC1, vecTC1, 4 ) );
		vecTC1 = vec_add( tempSum2, vec_sld( vecTC1, vecTC1, 8 ) );			
		tempSum3 = vec_add( vecTC2, vec_sld( vecTC2, vecTC2, 4 ) );
		vecTC2 = vec_add( tempSum3, vec_sld( vecTC2, vecTC2, 8 ) );	
		
		tempSum4 = vec_add( vecTC3, vec_sld( vecTC3, vecTC3, 4 ) );
		vecTC3 = vec_add( tempSum4, vec_sld( vecTC3, vecTC3, 8 ) );
		tempSum5 = vec_add( vecTC4, vec_sld( vecTC4, vecTC4, 4 ) );
		vecTC4 = vec_add( tempSum5, vec_sld( vecTC4, vecTC4, 8 ) );
		vector float tempSum6 = vec_add( vecTC5, vec_sld( vecTC5, vecTC5, 4 ) );
		vecTC5 = vec_add( tempSum6, vec_sld( vecTC5, vecTC5, 8 ) );
				
		vecTC0 = vec_splat( vecTC0, 0 );
		vecTC1 = vec_splat( vecTC1, 0 );
		vecTC2 = vec_splat( vecTC2, 0 );

		vecTC3 = vec_splat( vecTC3, 0 );
		vecTC4 = vec_splat( vecTC4, 0 );
		vecTC5 = vec_splat( vecTC5, 0 );
	
		if ( used[index] ) {
			// store out results
			vec_ste( vecTC0, 0, &texCoords[index][0] );
			vec_ste( vecTC1, 0, &texCoords[index][1] );
			vec_ste( vecTC2, 0, &texCoords[index][2] );
			vec_ste( (vector float)(1.0), 0, &texCoords[index][3] );
		}
		
		if ( used[index+1] ) {
			vec_ste( vecTC3, 0, &texCoords[index+1][0] );
			vec_ste( vecTC4, 0, &texCoords[index+1][1] );
			vec_ste( vecTC5, 0, &texCoords[index+1][2] );
			vec_ste( (vector float)(1.0), 0, &texCoords[index+1][3] );	
		}
	}

	// cleanup
	for ( ; index < numVerts; index++ ) {
		if ( !used[index] ) {
			continue;
		}

		const float *vertPtr = verts[index].xyz.ToFloatPtr();

		permVec = vec_add( vec_lvsl( -1, vertPtr ), (vector unsigned char)(1) );
		
		v0 = vec_ld( 0, vertPtr );
		v1 = vec_ld( 15, vertPtr );
		vector float v2 = vec_ld( 31, vertPtr );
		vector float v3 = vec_ld( 47, vertPtr );  
		vector float v4 = vec_ld( 63, vertPtr ); 
		
		// figure out what values go where
		vector float vecXYZ = vec_perm( v0, v1, permVec );
		vector float vecNormal = vec_perm( v1, v2, permVec );
		vecNormal = vec_sld( vecNormal, vecNormal, 4 );
		const vector float vecTangent0 = vec_perm( v2, v3, permVec );
		permVec = vec_add( permVec, (vector unsigned char)(-4) ); //shift permute right 3 elements
		const vector float vecTangent1 = vec_perm( v3, v4, permVec );
					
		// calculate lightDir
		vector float vecLightDir = vec_sub( vecLightOrigin, vecXYZ );
		vector float vecViewDir = vec_sub( vecViewOrigin, vecXYZ );

		// calculate distance
		vector float vecTempLight = vec_madd( vecLightDir, vecLightDir, zeroVector );
		vector float vecTempView = vec_madd( vecViewDir, vecViewDir, zeroVector );
		
		// sum accross first 3 elements of vector
		vector float tempSum = vec_add( vecTempLight, vec_sld( vecTempLight, vecTempLight, 4 ) );
		vecTempLight = vec_add( tempSum, vec_sld( tempSum, tempSum, 8 ) );
		vector float tempSum2 = vec_add( vecTempView, vec_sld( vecTempView, vecTempView, 4 ) );
		vecTempView = vec_add( tempSum2, vec_sld( tempSum2, tempSum2, 8 ) );
		
		// splat sum accross the whole vector
		vecTempLight = vec_splat( vecTempLight, 0 );
		vecTempView = vec_splat( vecTempView, 0 );
				
		vecTempLight = ReciprocalSquareRoot( vecTempLight );
		vecTempView = ReciprocalSquareRoot( vecTempView );
		
		// modify light and view vectors based on ilength
		vecViewDir = vec_madd( vecViewDir, vecTempView, zeroVector );
		vecLightDir = vec_madd( vecLightDir, vecTempLight, vecViewDir );
				
		// calculate what to store in each texture coord
		vector float vecTC0 = vec_madd( vecLightDir, vecTangent0, zeroVector );
		vector float vecTC1 = vec_madd( vecLightDir, vecTangent1, zeroVector );
		vector float vecTC2 = vec_madd( vecLightDir, vecNormal, zeroVector );		
		
		// sum accross first 3 elements of vector
		vector float tempSum3;
		tempSum = vec_add( vecTC0, vec_sld( vecTC0, vecTC0, 4 ) );
		vecTC0 = vec_add( tempSum, vec_sld( vecTC0, vecTC0, 8 ) );
		tempSum2 = vec_add( vecTC1, vec_sld( vecTC1, vecTC1, 4 ) );
		vecTC1 = vec_add( tempSum2, vec_sld( vecTC1, vecTC1, 8 ) );			
		tempSum3 = vec_add( vecTC2, vec_sld( vecTC2, vecTC2, 4 ) );
		vecTC2 = vec_add( tempSum3, vec_sld( vecTC2, vecTC2, 8 ) );	
		
		vecTC0 = vec_splat( vecTC0, 0 );
		vecTC1 = vec_splat( vecTC1, 0 );
		vecTC2 = vec_splat( vecTC2, 0 );
		
		// store out results
		vec_ste( vecTC0, 0, &texCoords[index][0] );
		vec_ste( vecTC1, 0, &texCoords[index][1] );
		vec_ste( vecTC2, 0, &texCoords[index][2] );
		vec_ste( (vector float)(1.0), 0, &texCoords[index][3] );
	
	}
}
#endif  /* 0 for disable spec coord */

#if 1

#ifdef VERTEXCACHE_ALIGNED
/*
============
idSIMD_AltiVec::CreateShadowCache
============
*/
int VPCALL idSIMD_AltiVec::CreateShadowCache( idVec4 *vertexCache, int *vertRemap, const idVec3 &lightOrigin, const idDrawVert *verts, const int numVerts ) {
	int outVerts = 0;
	int i = 0;
	
	assert( IS_16BYTE_ALIGNED( vertexCache[0] ) );
	
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector unsigned char vecPerm, vecPerm2, vecPerm3, vecPerm4, vecPerm5;
	register vector float zeroVector = (vector float)(0.0);
	register vector float oneVector = (vector float)(1);
	register vector unsigned char vecPermZeroLast = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,16,17,18,19);
	
	const float *lPtr = lightOrigin.ToFloatPtr();
	const float *vPtr;
	const float *vPtr2;
	const float *vPtr3;
	const float *vPtr4;
	
	// put values into a vector
	vecPerm = vec_add( vec_lvsl( -1, lPtr ), (vector unsigned char)(1) );
	v0 = vec_ld( 0, lPtr );
	v1 = vec_ld( 15, lPtr );
	v0 = vec_perm( v0, v1, vecPerm );
	v0 = vec_perm( v0, zeroVector, vecPermZeroLast );
	
	//v0 now contains lightOrigin[0], lightOrigin[1], lightOrigin[2], 0
	for ( ; i+3 < numVerts; i+= 4 ) {			
		if ( ! vertRemap[i] ) {
			vPtr = verts[i].xyz.ToFloatPtr();

#ifndef DRAWVERT_PADDED
			vecPerm2 = vec_add( vec_lvsl( -1, vPtr ), (vector unsigned char)(1) );
			v2 = vec_ld( 0, vPtr );
			v3 = vec_ld( 15, vPtr );
			v7 = vec_perm( v2, v3, vecPerm2 );	
#else
			v7 = vec_ld( 0, vPtr );
#endif
			v2 = vec_perm( v7, zeroVector, vecPermZeroLast );
			v3 = vec_perm( v7, oneVector, vecPermZeroLast );
			v1 = vec_sub( v2, v0 );
			
			vec_st( v3, 0, &vertexCache[outVerts][0] );
			vec_st( v1, 0, &vertexCache[outVerts+1][0] );

			vertRemap[i] = outVerts;
			outVerts += 2;
		}
		
		if ( ! vertRemap[i+1] ) {
			vPtr2 = verts[i+1].xyz.ToFloatPtr();

#ifndef DRAWVERT_PADDED	
			vecPerm3 = vec_add( vec_lvsl( -1, vPtr2 ), (vector unsigned char)(1) );
			v4 = vec_ld( 0, vPtr2 );
			v5 = vec_ld( 15, vPtr2 );
			v6 = vec_perm( v4, v5, vecPerm3 );
#else
			v6 = vec_ld( 0, vPtr2 );
#endif
			v4 = vec_perm( v6, zeroVector, vecPermZeroLast );
			v5 = vec_perm( v6, oneVector, vecPermZeroLast );
			v6 = vec_sub( v4, v0 );
			
			vec_st( v5, 0, &vertexCache[outVerts][0] );
			vec_st( v6, 0, &vertexCache[outVerts+1][0] );		
					
			vertRemap[i+1] = outVerts;
			outVerts += 2;
		}
	
		if ( ! vertRemap[i+2] ) {
			vPtr3 = verts[i+2].xyz.ToFloatPtr();

#ifndef DRAWVERT_PADDED	
			vecPerm4 = vec_add( vec_lvsl( -1, vPtr3 ), (vector unsigned char)(1) );			
			v1 = vec_ld( 0, vPtr3 );
			v2 = vec_ld( 15, vPtr3 );
			v3 = vec_perm( v1, v2, vecPerm4 );
#else
			v3 = vec_ld( 0, vPtr3 );
#endif
			v1 = vec_perm( v3, zeroVector, vecPermZeroLast );
			v2 = vec_perm( v3, oneVector, vecPermZeroLast );
			v3 = vec_sub( v1, v0 );

			vec_st( v2, 0, &vertexCache[outVerts][0] );
			vec_st( v3, 0, &vertexCache[outVerts+1][0] );	
								
			vertRemap[i+2] = outVerts;
			outVerts += 2;
		}
		
		if ( ! vertRemap[i+3] ) {
			vPtr4 = verts[i+3].xyz.ToFloatPtr();
#ifndef DRAWVERT_PADDED			
			vecPerm5 = vec_add( vec_lvsl( -1, vPtr4 ), (vector unsigned char)(1) );			
			v4 = vec_ld( 0, vPtr4 );
			v5 = vec_ld( 16, vPtr4 );
			v6 = vec_perm( v4, v5, vecPerm5 );
#else
			v6 = vec_ld( 0, vPtr4 );
#endif
			v4 = vec_perm( v6, zeroVector, vecPermZeroLast );
			v5 = vec_perm( v6, oneVector, vecPermZeroLast );
			v6 = vec_sub( v4, v0 );
			
			vec_st( v5, 0, &vertexCache[outVerts][0] );
			vec_st( v6, 0, &vertexCache[outVerts+1][0] );
										
			vertRemap[i+3] = outVerts;
			outVerts += 2;
		}		
	}

	// cleanup
	for (; i < numVerts; i++ ) {
		if ( vertRemap[i] ) {
			continue;
		}
		const float *v = verts[i].xyz.ToFloatPtr();
		vertexCache[outVerts+0][0] = v[0];
		vertexCache[outVerts+0][1] = v[1];
		vertexCache[outVerts+0][2] = v[2];
		vertexCache[outVerts+0][3] = 1.0f;

		// R_SetupProjection() builds the projection matrix with a slight crunch
		// for depth, which keeps this w=0 division from rasterizing right at the
		// wrap around point and causing depth fighting with the rear caps
		vertexCache[outVerts+1][0] = v[0] - lightOrigin[0];
		vertexCache[outVerts+1][1] = v[1] - lightOrigin[1];
		vertexCache[outVerts+1][2] = v[2] - lightOrigin[2];
		vertexCache[outVerts+1][3] = 0.0f;
		vertRemap[i] = outVerts;
		outVerts += 2;
	}
	return outVerts;
}

#else

/*
============
idSIMD_AltiVec::CreateShadowCache
============
*/
int VPCALL idSIMD_AltiVec::CreateShadowCache( idVec4 *vertexCache, int *vertRemap, const idVec3 &lightOrigin, const idDrawVert *verts, const int numVerts ) {
	int outVerts = 0;
	int i = 0;
	
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector unsigned char vecPerm, vecPerm2, vecPerm3, vecPerm4, vecPerm5;
	register vector float zeroVector = (vector float)(0.0);
	register vector float oneVector = (vector float)(1);
	register vector unsigned char vecPermZeroLast = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,16,17,18,19);
	
	const float *lPtr = lightOrigin.ToFloatPtr();
	const float *vPtr;
	const float *vPtr2;
	const float *vPtr3;
	const float *vPtr4;
	
	// put values into a vector
	vecPerm = vec_add( vec_lvsl( -1, lPtr ), (vector unsigned char)(1) );
	v0 = vec_ld( 0, lPtr );
	v1 = vec_ld( 15, lPtr );
	v0 = vec_perm( v0, v1, vecPerm );
	v0 = vec_perm( v0, zeroVector, vecPermZeroLast );
	
	//v0 now contains lightOrigin[0], lightOrigin[1], lightOrigin[2], 0
	for ( ; i+3 < numVerts; i+= 4 ) {			
		if ( ! vertRemap[i] ) {
			vPtr = verts[i].xyz.ToFloatPtr();
#ifndef DRAWVERT_PADDED
			vecPerm2 = vec_add( vec_lvsl( -1, vPtr ), (vector unsigned char)(1) );
			v2 = vec_ld( 0, vPtr );
			v3 = vec_ld( 15, vPtr );
			v7 = vec_perm( v2, v3, vecPerm2 );	
#else
			v7 = vec_ld( 0, vPtr );
#endif
			v2 = vec_perm( v7, zeroVector, vecPermZeroLast );
			v3 = vec_perm( v7, oneVector, vecPermZeroLast );
			v1 = vec_sub( v2, v0 );
			
			// store results
			UNALIGNED_STORE2( &vertexCache[outVerts][0], v3, v1 );
																				
			vertRemap[i] = outVerts;
			outVerts += 2;
		}
		
		if ( ! vertRemap[i+1] ) {
			vPtr2 = verts[i+1].xyz.ToFloatPtr();
#ifndef DRAWVERT_PADDED	
			vecPerm3 = vec_add( vec_lvsl( -1, vPtr2 ), (vector unsigned char)(1) );
			v4 = vec_ld( 0, vPtr2 );
			v5 = vec_ld( 15, vPtr2 );
			v6 = vec_perm( v4, v5, vecPerm3 );
#else
			v6 = vec_ld( 0, vPtr2 );
#endif
			v4 = vec_perm( v6, zeroVector, vecPermZeroLast );
			v5 = vec_perm( v6, oneVector, vecPermZeroLast );
			v6 = vec_sub( v4, v0 );

			// store results
			UNALIGNED_STORE2( &vertexCache[outVerts][0], v5, v6 );
			
			vertRemap[i+1] = outVerts;
			outVerts += 2;
		}
	
		if ( ! vertRemap[i+2] ) {
			vPtr3 = verts[i+2].xyz.ToFloatPtr();
#ifndef DRAWVERT_PADDED	
			vecPerm4 = vec_add( vec_lvsl( -1, vPtr3 ), (vector unsigned char)(1) );			
			v1 = vec_ld( 0, vPtr3 );
			v2 = vec_ld( 15, vPtr3 );
			v3 = vec_perm( v1, v2, vecPerm4 );
#else
			v3 = vec_ld( 0, vPtr3 );
#endif
			v1 = vec_perm( v3, zeroVector, vecPermZeroLast );
			v2 = vec_perm( v3, oneVector, vecPermZeroLast );
			v3 = vec_sub( v1, v0 );
			
			// store results
			UNALIGNED_STORE2( &vertexCache[outVerts][0], v2, v3 );
								
			vertRemap[i+2] = outVerts;
			outVerts += 2;
		}
		if ( ! vertRemap[i+3] ) {
			vPtr4 = verts[i+3].xyz.ToFloatPtr();
#ifndef DRAWVERT_PADDED			
			vecPerm5 = vec_add( vec_lvsl( -1, vPtr4 ), (vector unsigned char)(1) );			
			v4 = vec_ld( 0, vPtr4 );
			v5 = vec_ld( 16, vPtr4 );
			v6 = vec_perm( v4, v5, vecPerm5 );
#else
			v6 = vec_ld( 0, vPtr4 );
#endif

			v4 = vec_perm( v6, zeroVector, vecPermZeroLast );
			v5 = vec_perm( v6, oneVector, vecPermZeroLast );
			v6 = vec_sub( v4, v0 );
			
			// store results
			UNALIGNED_STORE2( &vertexCache[outVerts][0], v5, v6 );
			
					
			vertRemap[i+3] = outVerts;
			outVerts += 2;
		}		
	}

	// cleanup
	for (; i < numVerts; i++ ) {
		if ( vertRemap[i] ) {
			continue;
		}
		const float *v = verts[i].xyz.ToFloatPtr();
		vertexCache[outVerts+0][0] = v[0];
		vertexCache[outVerts+0][1] = v[1];
		vertexCache[outVerts+0][2] = v[2];
		vertexCache[outVerts+0][3] = 1.0f;

		// R_SetupProjection() builds the projection matrix with a slight crunch
		// for depth, which keeps this w=0 division from rasterizing right at the
		// wrap around point and causing depth fighting with the rear caps
		vertexCache[outVerts+1][0] = v[0] - lightOrigin[0];
		vertexCache[outVerts+1][1] = v[1] - lightOrigin[1];
		vertexCache[outVerts+1][2] = v[2] - lightOrigin[2];
		vertexCache[outVerts+1][3] = 0.0f;
		vertRemap[i] = outVerts;
		outVerts += 2;
	}
	return outVerts;
}
#endif /* VERTEXCACHE_ALIGNED */

#endif /* 0 to disable shadow cache */

#if 1

#ifdef VERTEXCACHE_ALIGNED
/*
============
idSIMD_AltiVec::CreateVertexProgramShadowCache
============
*/
int VPCALL idSIMD_AltiVec::CreateVertexProgramShadowCache( idVec4 *vertexCache, const idDrawVert *verts, const int numVerts ) {

	// vertexCache aligned
	assert( IS_16BYTE_ALIGNED( vertexCache[0] ) );
	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	// idVec4 size
	assert( sizeof(idVec4) == IDVEC4_OFFSET * sizeof(float) );
	
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector float zeroVector = (vector float)(0.0);
	register vector float oneVector = (vector float)(1);
	register vector unsigned char vecPermThreeOne = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,16,17,18,19);
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	int i = 0;

#ifndef DRAWVERT_PADDED
	// every fourth one will have the same alignment. Make sure we've got enough here
	if ( i+3 < numVerts ) {
		vertPerm1 = vec_add( vec_lvsl( -1, (float*) verts[0].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm2 = vec_add( vec_lvsl( -1, (float*) verts[1].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm3 = vec_add( vec_lvsl( -1, (float*) verts[2].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm4 = vec_add( vec_lvsl( -1, (float*) verts[3].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
	}
#endif
	
	for ( ; i+3 < numVerts; i+=4 ) {
		const float *vertPtr = verts[i].xyz.ToFloatPtr();
		const float *vertPtr2 = verts[i+1].xyz.ToFloatPtr();
		const float *vertPtr3 = verts[i+2].xyz.ToFloatPtr();
		const float *vertPtr4 = verts[i+3].xyz.ToFloatPtr();
			
#ifndef DRAWVERT_PADDED
		v0 = vec_ld( 0, vertPtr );
		v1 = vec_ld( 15, vertPtr );
		v2 = vec_ld( 0, vertPtr2 );
		v3 = vec_ld( 15, vertPtr2 );
		v4 = vec_ld( 0, vertPtr3 );
		v5 = vec_ld( 15, vertPtr3 );
		v6 = vec_ld( 0, vertPtr4 );
		v7 = vec_ld( 15, vertPtr4 );

		v0 = vec_perm( v0, v1, vertPerm1 );
		v1 = vec_perm( v2, v3, vertPerm2 );
		v2 = vec_perm( v4, v5, vertPerm3 );
		v3 = vec_perm( v6, v7, vertPerm4 );
#else
		v0 = vec_ld( 0, vertPtr );
		v1 = vec_ld( 0, vertPtr2 );
		v2 = vec_ld( 0, vertPtr3 );
		v3 = vec_ld( 0, vertPtr4 );
#endif

		v0 = vec_perm( v0, oneVector, vecPermThreeOne );
		v4 = vec_perm( v0, zeroVector, vecPermThreeOne );
		
		v1 = vec_perm( v1, oneVector, vecPermThreeOne );
		v5 = vec_perm( v1, zeroVector, vecPermThreeOne );
		
		v2 = vec_perm( v2, oneVector, vecPermThreeOne );
		v6 = vec_perm( v2, zeroVector, vecPermThreeOne );
		
		v3 = vec_perm( v3, oneVector, vecPermThreeOne );
		v7 = vec_perm( v3, zeroVector, vecPermThreeOne );

		// store results		
		ALIGNED_STORE4( &vertexCache[i*2][0], v0, v4, v1, v5 );
		ALIGNED_STORE4( &vertexCache[(i+2)*2][0], v2, v6, v3, v7 );

	}

	// cleanup
	for ( ; i < numVerts; i++ ) {
		const float *v = verts[i].xyz.ToFloatPtr();
		vertexCache[i*2+0][0] = v[0];
		vertexCache[i*2+1][0] = v[0];
		vertexCache[i*2+0][1] = v[1];
		vertexCache[i*2+1][1] = v[1];
		vertexCache[i*2+0][2] = v[2];
		vertexCache[i*2+1][2] = v[2];
		vertexCache[i*2+0][3] = 1.0f;
		vertexCache[i*2+1][3] = 0.0f;
	}
	return numVerts * 2;
}

#else
/*
============
idSIMD_AltiVec::CreateVertexProgramShadowCache
============
*/
int VPCALL idSIMD_AltiVec::CreateVertexProgramShadowCache( idVec4 *vertexCache, const idDrawVert *verts, const int numVerts ) {

	// idDrawVert size
	assert( sizeof(idDrawVert) == DRAWVERT_OFFSET * sizeof(float) );
	// idVec4 size
	assert( sizeof(idVec4) == IDVEC4_OFFSET * sizeof(float) );
	
	register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector float zeroVector = (vector float)(0.0);
	register vector float oneVector = (vector float)(1);
	register vector unsigned char vecPermThreeOne = (vector unsigned char)(0,1,2,3,4,5,6,7,8,9,10,11,16,17,18,19);
	vector unsigned char vertPerm1, vertPerm2, vertPerm3, vertPerm4;
	int i = 0;

#ifndef DRAWVERT_PADDED
	// every fourth one will have the same alignment. Make sure we've got enough here
	if ( i+3 < numVerts ) {
		vertPerm1 = vec_add( vec_lvsl( -1, (float*) verts[0].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm2 = vec_add( vec_lvsl( -1, (float*) verts[1].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm3 = vec_add( vec_lvsl( -1, (float*) verts[2].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
		vertPerm4 = vec_add( vec_lvsl( -1, (float*) verts[3].xyz.ToFloatPtr() ), (vector unsigned char)(1) );
	}
#endif
	
	for ( ; i+3 < numVerts; i+=4 ) {
		const float *vertPtr = verts[i].xyz.ToFloatPtr();
		const float *vertPtr2 = verts[i+1].xyz.ToFloatPtr();
		const float *vertPtr3 = verts[i+2].xyz.ToFloatPtr();
		const float *vertPtr4 = verts[i+3].xyz.ToFloatPtr();

#ifndef DRAWVERT_PADDED			
		v0 = vec_ld( 0, vertPtr );
		v1 = vec_ld( 15, vertPtr );
		v2 = vec_ld( 0, vertPtr2 );
		v3 = vec_ld( 15, vertPtr2 );
		v4 = vec_ld( 0, vertPtr3 );
		v5 = vec_ld( 15, vertPtr3 );
		v6 = vec_ld( 0, vertPtr4 );
		v7 = vec_ld( 15, vertPtr4 );
			
		v0 = vec_perm( v0, v1, vertPerm1 );
		v1 = vec_perm( v2, v3, vertPerm2 );
		v2 = vec_perm( v4, v5, vertPerm3 );
		v3 = vec_perm( v6, v7, vertPerm4 );
#else
		v0 = vec_ld( 0, vertPtr );
		v1 = vec_ld( 0, vertPtr2 );
		v2 = vec_ld( 0, vertPtr3 );
		v3 = vec_ld( 0, vertPtr4 );		
#endif		

		v0 = vec_perm( v0, oneVector, vecPermThreeOne );
		v4 = vec_perm( v0, zeroVector, vecPermThreeOne );
		
		v1 = vec_perm( v1, oneVector, vecPermThreeOne );
		v5 = vec_perm( v1, zeroVector, vecPermThreeOne );
		
		v2 = vec_perm( v2, oneVector, vecPermThreeOne );
		v6 = vec_perm( v2, zeroVector, vecPermThreeOne );
		
		v3 = vec_perm( v3, oneVector, vecPermThreeOne );
		v7 = vec_perm( v3, zeroVector, vecPermThreeOne );
		
		// store results as unaligned
		vector unsigned char storePerm = vec_sub( vec_lvsr( 15, &vertexCache[i*2][0] ), (vector unsigned char)(1) );
		vector unsigned int mask = vec_perm( (vector unsigned int)(0), (vector unsigned int)(-1), storePerm );
		vector float vc1 = vec_ld( 0, &vertexCache[i*2][0] );
		vector float vc2 = vec_ld( 127, &vertexCache[i*2][0] );
		
		// right rotate input data
		v0 = vec_perm( v0, v0, storePerm );
		v4 = vec_perm( v4, v4, storePerm );
		v1 = vec_perm( v1, v1, storePerm );
		v5 = vec_perm( v5, v5, storePerm );
		v2 = vec_perm( v2, v2, storePerm );
		v6 = vec_perm( v6, v6, storePerm );
		v3 = vec_perm( v3, v3, storePerm );
		v7 = vec_perm( v7, v7, storePerm );
		
		vec_st( vec_sel( vc1, v0, mask ), 0 , &vertexCache[i*2][0] );
		vec_st( vec_sel( v0, v4, mask ), 15 , &vertexCache[i*2][0] );
		vec_st( vec_sel( v4, v1, mask ), 31 , &vertexCache[i*2][0] );
		vec_st( vec_sel( v1, v5, mask ), 47 , &vertexCache[i*2][0] );
		vec_st( vec_sel( v5, v2, mask ), 63 , &vertexCache[i*2][0] );
		vec_st( vec_sel( v2, v6, mask ), 79 , &vertexCache[i*2][0] );
		vec_st( vec_sel( v6, v3, mask ), 95 , &vertexCache[i*2][0] );
		vec_st( vec_sel( v3, v7, mask ), 111 , &vertexCache[i*2][0] );
		vec_st( vec_sel( v7, vc2, mask ), 127 , &vertexCache[i*2][0] );
	}

	// cleanup
	for ( ; i < numVerts; i++ ) {
		const float *v = verts[i].xyz.ToFloatPtr();
		vertexCache[i*2+0][0] = v[0];
		vertexCache[i*2+1][0] = v[0];
		vertexCache[i*2+0][1] = v[1];
		vertexCache[i*2+1][1] = v[1];
		vertexCache[i*2+0][2] = v[2];
		vertexCache[i*2+1][2] = v[2];
		vertexCache[i*2+0][3] = 1.0f;
		vertexCache[i*2+1][3] = 0.0f;
	}
	return numVerts * 2;
}

#endif /* VERTEXCACHE_ALIGNED */

#endif /* 0 to kill VP shader cache */

#endif /* ENABLE_CREATE */

#ifdef ENABLE_SOUND_ROUTINES

#ifdef SOUND_DEST_ALIGNED
/*
============
idSIMD_AltiVec::UpSamplePCMTo44kHz

  Duplicate samples for 44kHz output.
  
	Assumptions:
		Assumes that dest starts at aligned address
============
*/
void idSIMD_AltiVec::UpSamplePCMTo44kHz( float *dest, const short *src, const int numSamples, const int kHz, const int numChannels ) {
	
	// dest is aligned
	assert( IS_16BYTE_ALIGNED( dest[0] ) );

	vector signed short vs0, vs1;
	register vector signed int vi0, vi1;
	register vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
	// permute vectors	
	register vector unsigned char vecFirstHalf = (vector unsigned char)(0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7);
	register vector unsigned char vecSecondHalf = (vector unsigned char)(8,9,10,11,12,13,14,15,8,9,10,11,12,13,14,15);
	
	register vector unsigned char vecBottom = (vector unsigned char)(0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7);
	register vector unsigned char vecTop = (vector unsigned char)(8,9,10,11,8,9,10,11,12,13,14,15,12,13,14,15);
	
	// If this can be assumed true, we can eliminate another conditional that checks to see if we can
	// load up a vector before the loop
	assert( numSamples >= 12 );
	
	if ( kHz == 11025 ) {
		if ( numChannels == 1 ) {
			// 8 at a time
			int i = 0;
		
			vector signed short vsOld = vec_ld( 0, &src[i] );
			vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[i] ), (vector unsigned char)(1) );
			  
			for ( ; i+7 < numSamples; i+= 8 ) {
				// load src
				vs1 = vec_ld( 15, &src[i] );
				vs0 = vec_perm( vsOld, vs1, permVec );
				vsOld = vs1;
				
				// unpack shorts to ints
				vi0 = vec_unpackh( vs0 );
				vi1 = vec_unpackl( vs0 );
				// convert ints to floats
				v0 = vec_ctf( vi0, 0 );
				v1 = vec_ctf( vi1, 0 );
				// permute into vectors in the order to store

				v2 = vec_splat( v0, 0 );
				v3 = vec_splat( v0, 1 );
				v4 = vec_splat( v0, 2 );
				v5 = vec_splat( v0, 3 );
				v6 = vec_splat( v1, 0 );
				v7 = vec_splat( v1, 1 );
				v8 = vec_splat( v1, 2 );
				v9 = vec_splat( v1, 3 );

				// store results
				ALIGNED_STORE8( &dest[i*4], v2, v3, v4, v5, v6, v7, v8, v9 );
			}
			// cleanup
			for (; i < numSamples; i++ ) {
				dest[i*4+0] = dest[i*4+1] = dest[i*4+2] = dest[i*4+3] = (float) src[i+0];
			}
		} else {
			int i = 0;
			
			vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[0] ), (vector unsigned char)(1) );
			vector signed short vsOld = vec_ld( 0, &src[0] );
			
			for ( ; i+7 < numSamples; i += 8 ) {
				// load src
				vs1 = vec_ld( 15, &src[i] );
				vs0 = vec_perm( vsOld, vs1, permVec );
				vsOld = vs1;
			
				// unpack shorts to ints
				vi0 = vec_unpackh( vs0 );
				vi1 = vec_unpackl( vs0 );
				// convert ints to floats
				v0 = vec_ctf( vi0, 0 );
				v1 = vec_ctf( vi1, 0 );
				// put into vectors in order to store
				v2 = vec_perm( v0, v0, vecFirstHalf );
				v3 = v2;
				v4 = vec_perm( v0, v0, vecSecondHalf );
				v5 = v4;
				v6 = vec_perm( v1, v1, vecFirstHalf );
				v7 = v6;
				v8 = vec_perm (v1, v1, vecSecondHalf );
				v9 = v8;
				
				// store results
				ALIGNED_STORE8( &dest[i*4], v2, v3, v4, v5, v6, v7, v8, v9 );
			}
			
			for ( ; i < numSamples; i += 2 ) {
				dest[i*4+0] = dest[i*4+2] = dest[i*4+4] = dest[i*4+6] = (float) src[i+0];
				dest[i*4+1] = dest[i*4+3] = dest[i*4+5] = dest[i*4+7] = (float) src[i+1];
			}
		}
	} else if ( kHz == 22050 ) {
		if ( numChannels == 1 ) {
			int i;
			vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[0] ), (vector unsigned char)(1) );
			vector signed short vsOld = vec_ld( 0, &src[0] );
			
			for ( i = 0; i+7 < numSamples; i += 8 ) {
				// load src
				vs1 = vec_ld( 0, &src[i] );
				vs0 = vec_perm( vsOld, vs1, permVec );
				vsOld = vs1;
				
				// unpack shorts to ints
				vi0 = vec_unpackh( vs0 );
				vi1 = vec_unpackl( vs0 );
				// convert ints to floats
				v0 = vec_ctf( vi0, 0 );
				v1 = vec_ctf( vi1, 0 );
				// put into vectors in order to store
				v2 = vec_perm( v0, v0, vecBottom );
				v3 = vec_perm( v0, v0, vecTop );
				v4 = vec_perm( v1, v1, vecBottom );
				v5 = vec_perm (v1, v1, vecTop );
				
				// store results
				ALIGNED_STORE4( &dest[i*2], v2, v3, v4, v5 );
			}
			// cleanup
			for ( ; i < numSamples; i++ ) {
				dest[i*2+0] = dest[i*2+1] = (float) src[i+0];
			}
		} else {
			int i;
			vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[0] ), (vector unsigned char)(1) );
			vector signed short vsOld = vec_ld( 0, &src[0] );
			
			for ( i = 0; i+7 < numSamples; i += 8 ) {
				// load src
				vs1 = vec_ld( 15, &src[i] );
				vs0 = vec_perm( vsOld, vs1, permVec );
				vsOld = vs1;
				
				// unpack shorts to ints
				vi0 = vec_unpackh( vs0 );
				vi1 = vec_unpackl( vs0 );
				// convert ints to floats
				v0 = vec_ctf( vi0, 0 );
				v1 = vec_ctf( vi1, 0 );
				// put into vectors in order to store
				v2 = vec_perm( v0, v0, vecFirstHalf );
				v3 = vec_perm( v0, v0, vecSecondHalf );				
				v4 = vec_perm( v1, v1, vecFirstHalf );
				v5 = vec_perm (v1, v1, vecSecondHalf );
				
				// store results
				ALIGNED_STORE4( &dest[i*2], v2, v3, v4, v5 );
			}
			// cleanup
			for ( ; i < numSamples; i += 2 ) {
				dest[i*2+0] = dest[i*2+2] = (float) src[i+0];
				dest[i*2+1] = dest[i*2+3] = (float) src[i+1];
			}
		}
	} else if ( kHz == 44100 ) {
		int i;
		vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[0] ), (vector unsigned char)(1) );
		vector signed short vsOld = vec_ld( 0, &src[0] );
			
		for ( i = 0; i+7 < numSamples; i += 8 ) {
			vs1 = vec_ld( 15, &src[i] );
			vs0 = vec_perm( vsOld,  vs1, permVec );
			vsOld = vs1;
			
			//unpack shorts to ints
			vi0 = vec_unpackh( vs0 );
			vi1 = vec_unpackl( vs0 );
		
			//convert ints to floats
			v0 = vec_ctf( vi0, 0 );
			v1 = vec_ctf( vi1, 0 );
					
			//store results
			ALIGNED_STORE2( &dest[i], v0, v1 );
		}
		//	cleanup
		for ( ; i < numSamples; i++ ) {
			dest[i] = (float) src[i];
		}
	} else {
		assert( 0 );
	}
}

#else

/*
============
idSIMD_AltiVec::UpSamplePCMTo44kHz

  Duplicate samples for 44kHz output.
  
	Assumptions:
		No assumptions
============
*/
void idSIMD_AltiVec::UpSamplePCMTo44kHz( float *dest, const short *src, const int numSamples, const int kHz, const int numChannels ) {
	
	vector signed short vs0, vs1;
	register vector signed int vi0, vi1;
	register vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
	// permute vectors	
	register vector unsigned char vecFirstHalf = (vector unsigned char)(0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7);
	register vector unsigned char vecSecondHalf = (vector unsigned char)(8,9,10,11,12,13,14,15,8,9,10,11,12,13,14,15);
	
	register vector unsigned char vecBottom = (vector unsigned char)(0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7);
	register vector unsigned char vecTop = (vector unsigned char)(8,9,10,11,8,9,10,11,12,13,14,15,12,13,14,15);
	
	// calculate perm vector and masks for stores
	vector unsigned char storePerm = vec_sub( vec_lvsr( 15, &dest[0] ), (vector unsigned char)(1) );
	// original values of dest
	vector float vecDest = vec_ld( 0, &dest[0] ); 
	vector unsigned int mask = vec_perm( (vector unsigned int)(0), (vector unsigned int)(-1), storePerm );
	
	if ( kHz == 11025 ) {
		if ( numChannels == 1 ) {
			// 8 at a time
			int i = 0;
		
			vector signed short vsOld = vec_ld( 0, &src[i] );
			vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[i] ), (vector unsigned char)(1) );
			  
			for ( ; i+7 < numSamples; i+= 8 ) {
				// load src
				vs1 = vec_ld( 15, &src[i] );
				vs0 = vec_perm( vsOld, vs1, permVec );
				vsOld = vs1;
				vector float vecDestEnd = vec_ld( 127, &dest[i*4] );
				
				// unpack shorts to ints
				vi0 = vec_unpackh( vs0 );
				vi1 = vec_unpackl( vs0 );
				// convert ints to floats
				v0 = vec_ctf( vi0, 0 );
				v1 = vec_ctf( vi1, 0 );
				// permute into vectors in the order to store

				v2 = vec_splat( v0, 0 );
				v3 = vec_splat( v0, 1 );
				v4 = vec_splat( v0, 2 );
				v5 = vec_splat( v0, 3 );
				v6 = vec_splat( v1, 0 );
				v7 = vec_splat( v1, 1 );
				v8 = vec_splat( v1, 2 );
				v9 = vec_splat( v1, 3 );

				v2 = vec_perm( v2, v2, storePerm );
				v3 = vec_perm( v3, v3, storePerm );
				v4 = vec_perm( v4, v4, storePerm );
				v5 = vec_perm( v5, v5, storePerm );
				v6 = vec_perm( v6, v6, storePerm );
				v7 = vec_perm( v7, v7, storePerm );
				v8 = vec_perm( v8, v8, storePerm );
				v9 = vec_perm( v9, v9, storePerm );
			
				// store results
				vec_st( vec_sel( vecDest, v2, mask ), 0, &dest[i*4] );
				vec_st( vec_sel( v2, v3, mask ), 15, &dest[i*4] );
				vec_st( vec_sel( v3, v4, mask ), 31, &dest[i*4] );
				vec_st( vec_sel( v4, v5, mask ), 47, &dest[i*4] );
				vec_st( vec_sel( v5, v6, mask ), 63, &dest[i*4] );
				vec_st( vec_sel( v6, v7, mask ), 79, &dest[i*4] );
				vec_st( vec_sel( v7, v8, mask ), 95, &dest[i*4] );
				vec_st( vec_sel( v8, v9, mask ), 111, &dest[i*4] );
				vecDest = vec_sel( v9, vecDestEnd, mask );
				vec_st( vecDest, 127, &dest[i*4] );
			}
			// cleanup
			for (; i < numSamples; i++ ) {
				dest[i*4+0] = dest[i*4+1] = dest[i*4+2] = dest[i*4+3] = (float) src[i+0];
			}
		} else {
			int i = 0;
			
			vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[0] ), (vector unsigned char)(1) );
			vector signed short vsOld = vec_ld( 0, &src[0] );
			
			for ( ; i+7 < numSamples; i += 8 ) {
				// load src
				vs1 = vec_ld( 15, &src[i] );
				vs0 = vec_perm( vsOld, vs1, permVec );
				vsOld = vs1;
				vector float vecDestEnd = vec_ld( 127, &dest[i*4] );

				// unpack shorts to ints
				vi0 = vec_unpackh( vs0 );
				vi1 = vec_unpackl( vs0 );
				// convert ints to floats
				v0 = vec_ctf( vi0, 0 );
				v1 = vec_ctf( vi1, 0 );
				// put into vectors in order to store
				v2 = vec_perm( v0, v0, vecFirstHalf );
				v3 = v2;
				v4 = vec_perm( v0, v0, vecSecondHalf );
				v5 = v4;
				v6 = vec_perm( v1, v1, vecFirstHalf );
				v7 = v6;
				v8 = vec_perm (v1, v1, vecSecondHalf );
				v9 = v8;

				v2 = vec_perm( v2, v2, storePerm );
				v3 = vec_perm( v3, v3, storePerm );
				v4 = vec_perm( v4, v4, storePerm );
				v5 = vec_perm( v5, v5, storePerm );
				v6 = vec_perm( v6, v6, storePerm );
				v7 = vec_perm( v7, v7, storePerm );
				v8 = vec_perm( v8, v8, storePerm );
				v9 = vec_perm( v9, v9, storePerm );
			
				// store results
				vec_st( vec_sel( vecDest, v2, mask ), 0, &dest[i*4] );
				vec_st( vec_sel( v2, v3, mask ), 15, &dest[i*4] );
				vec_st( vec_sel( v3, v4, mask ), 31, &dest[i*4] );
				vec_st( vec_sel( v4, v5, mask ), 47, &dest[i*4] );
				vec_st( vec_sel( v5, v6, mask ), 63, &dest[i*4] );
				vec_st( vec_sel( v6, v7, mask ), 79, &dest[i*4] );
				vec_st( vec_sel( v7, v8, mask ), 95, &dest[i*4] );
				vec_st( vec_sel( v8, v9, mask ), 111, &dest[i*4] );
				vecDest = vec_sel( v9, vecDestEnd, mask );
				vec_st( vecDest, 127, &dest[i*4] );
			}
			
			for ( ; i < numSamples; i += 2 ) {
				dest[i*4+0] = dest[i*4+2] = dest[i*4+4] = dest[i*4+6] = (float) src[i+0];
				dest[i*4+1] = dest[i*4+3] = dest[i*4+5] = dest[i*4+7] = (float) src[i+1];
			}
		}
	} else if ( kHz == 22050 ) {
		if ( numChannels == 1 ) {
			int i;
			vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[0] ), (vector unsigned char)(1) );
			vector signed short vsOld = vec_ld( 0, &src[0] );
			
			for ( i = 0; i+7 < numSamples; i += 8 ) {
				// load src
				vs1 = vec_ld( 0, &src[i] );
				vs0 = vec_perm( vsOld, vs1, permVec );
				vsOld = vs1;
				vector float vecDestEnd = vec_ld( 63, &dest[i*2] );
				
				// unpack shorts to ints
				vi0 = vec_unpackh( vs0 );
				vi1 = vec_unpackl( vs0 );
				// convert ints to floats
				v0 = vec_ctf( vi0, 0 );
				v1 = vec_ctf( vi1, 0 );
				// put into vectors in order to store
				v2 = vec_perm( v0, v0, vecBottom );
				v3 = vec_perm( v0, v0, vecTop );
				v4 = vec_perm( v1, v1, vecBottom );
				v5 = vec_perm (v1, v1, vecTop );
	
				v2 = vec_perm( v2, v2, storePerm );
				v3 = vec_perm( v3, v3, storePerm );
				v4 = vec_perm( v4, v4, storePerm );
				v5 = vec_perm( v5, v5, storePerm );
			
				// store results
				vec_st( vec_sel( vecDest, v2, mask ), 0, &dest[i*2] );
				vec_st( vec_sel( v2, v3, mask ), 15, &dest[i*2] );
				vec_st( vec_sel( v3, v4, mask ), 31, &dest[i*2] );
				vec_st( vec_sel( v4, v5, mask ), 47, &dest[i*2] );
				vecDest = vec_sel( v5, vecDestEnd, mask );
				vec_st( vecDest, 63, &dest[i*2] );
			
			}
			// cleanup
			for ( ; i < numSamples; i++ ) {
				dest[i*2+0] = dest[i*2+1] = (float) src[i+0];
			}
		} else {
			int i;
			vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[0] ), (vector unsigned char)(1) );
			vector signed short vsOld = vec_ld( 0, &src[0] );
			
			for ( i = 0; i+7 < numSamples; i += 8 ) {
				// load src
				vs1 = vec_ld( 15, &src[i] );
				vs0 = vec_perm( vsOld, vs1, permVec );
				vsOld = vs1;
				vector float vecDestEnd = vec_ld( 63, &dest[i*2] );

				// unpack shorts to ints
				vi0 = vec_unpackh( vs0 );
				vi1 = vec_unpackl( vs0 );
				// convert ints to floats
				v0 = vec_ctf( vi0, 0 );
				v1 = vec_ctf( vi1, 0 );
				// put into vectors in order to store
				v2 = vec_perm( v0, v0, vecFirstHalf );
				v3 = vec_perm( v0, v0, vecSecondHalf );				
				v4 = vec_perm( v1, v1, vecFirstHalf );
				v5 = vec_perm (v1, v1, vecSecondHalf );
				
				v2 = vec_perm( v2, v2, storePerm );
				v3 = vec_perm( v3, v3, storePerm );
				v4 = vec_perm( v4, v4, storePerm );
				v5 = vec_perm( v5, v5, storePerm );
			
				// store results
				vec_st( vec_sel( vecDest, v2, mask ), 0, &dest[i*2] );
				vec_st( vec_sel( v2, v3, mask ), 15, &dest[i*2] );
				vec_st( vec_sel( v3, v4, mask ), 31, &dest[i*2] );
				vec_st( vec_sel( v4, v5, mask ), 47, &dest[i*2] );
				vecDest = vec_sel( v5, vecDestEnd, mask );
				vec_st( vecDest, 63, &dest[i*2] );
			}
			// cleanup
			for ( ; i < numSamples; i += 2 ) {
				dest[i*2+0] = dest[i*2+2] = (float) src[i+0];
				dest[i*2+1] = dest[i*2+3] = (float) src[i+1];
			}
		}
	} else if ( kHz == 44100 ) {
		int i;
		vector unsigned char permVec = vec_add( vec_lvsl( -1, &src[0] ), (vector unsigned char)(1) );
		vector signed short vsOld = vec_ld( 0, &src[0] );
			
		for ( i = 0; i+7 < numSamples; i += 8 ) {
			//vs0 = vec_ld( 0, &src[i] );
			vs1 = vec_ld( 15, &src[i] );
			vs0 = vec_perm( vsOld,  vs1, permVec );
			vsOld = vs1;
			vector float vecDestEnd = vec_ld( 31, &dest[i] );
			
			//unpack shorts to ints
			vi0 = vec_unpackh( vs0 );
			vi1 = vec_unpackl( vs0 );
		
			//convert ints to floats
			v0 = vec_ctf( vi0, 0 );
			v1 = vec_ctf( vi1, 0 );
					
			v0 = vec_perm( v0, v0, storePerm );
			v1 = vec_perm( v1, v1, storePerm );
					
			// store results
			vec_st( vec_sel( vecDest, v0, mask ), 0, &dest[i] );
			vec_st( vec_sel( v0, v1, mask ), 15, &dest[i] );
			vecDest = vec_sel( v1, vecDestEnd, mask );
			vec_st( vecDest, 31, &dest[i] );
		}
		//	cleanup
		for ( ; i < numSamples; i++ ) {
			dest[i] = (float) src[i];
		}
	} else {
		assert( 0 );
	}
}

#endif

#ifdef SOUND_DEST_ALIGNED
/*
============
idSIMD_AltiVec::UpSampleOGGTo44kHz

  Duplicate samples for 44kHz output.
  
  	Assumptions:
		Assumes that dest starts at aligned address
============
*/
void idSIMD_AltiVec::UpSampleOGGTo44kHz( float *dest, const float * const *ogg, const int numSamples, const int kHz, const int numChannels ) {
	// dest is aligned
	assert( IS_16BYTE_ALIGNED( dest[0] ) );
	
	register vector float oggVec1, oggVec2, oggVec3, oggVec4, oggVec5, oggVec6, oggVec7, oggVec8;
	register vector float constVec, zeroVector;
	register vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
	vector unsigned char vecPerm1;
	vector unsigned char vecPerm2; 

	vector unsigned char vecOneTwo = (vector unsigned char)(0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7);
	vector unsigned char vecThreeFour = (vector unsigned char)(8,9,10,11,8,9,10,11,12,13,14,15,12,13,14,15);
	vector unsigned char vecFirst = (vector unsigned char)(0,1,2,3,16,17,18,19,0,1,2,3,16,17,18,19);
	vector unsigned char vecSecond = (vector unsigned char)(4,5,6,7,20,21,22,23,4,5,6,7,20,21,22,23);
	vector unsigned char vecThird = (vector unsigned char)(8,9,10,11,24,25,26,27,8,9,10,11,24,25,26,27);
	vector unsigned char vecFourth = (vector unsigned char)(12,13,14,15,28,29,30,31,12,13,14,15,28,29,30,31);
		
	constVec = (vector float)(32768.0f);
	zeroVector = (vector float)(0.0);
	
	if ( kHz == 11025 ) {
		if ( numChannels == 1 ) {
			 // calculate perm vector and do first load
    		 vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
			 v10 = vec_ld( 0, &ogg[0][0] );
			 
			 int i;
			 for ( i = 0; i+7 < numSamples; i += 8 ) {
				// as it happens, ogg[0][i] through ogg[0][i+3] are contiguous in memory
				v8 = v10;
				v9 = vec_ld( 15, &ogg[0][i] );
				v10 = vec_ld( 31, &ogg[0][i] );
				v0 = vec_perm( v8, v9, vecPerm1 );
				v1 = vec_perm( v9, v10, vecPerm1 );
				
				// now we have the elements in a vector, we want
				// to splat them each accross their own vector
				oggVec1 = vec_splat( v0, 0 );
				oggVec2 = vec_splat( v0, 1 );
				oggVec3 = vec_splat( v0, 2 );
				oggVec4 = vec_splat( v0, 3 );		
				oggVec5 = vec_splat( v1, 0 );
				oggVec6 = vec_splat( v1, 1 );
				oggVec7 = vec_splat( v1, 2 );
				oggVec8 = vec_splat( v1, 3 );
			
				v0 = vec_madd( oggVec1, constVec, zeroVector );
				v1 = vec_madd( oggVec2, constVec, zeroVector );
				v2 = vec_madd( oggVec3, constVec, zeroVector );
				v3 = vec_madd( oggVec4, constVec, zeroVector );
				v4 = vec_madd( oggVec5, constVec, zeroVector );
				v5 = vec_madd( oggVec6, constVec, zeroVector );
				v6 = vec_madd( oggVec7, constVec, zeroVector );
				v7 = vec_madd( oggVec8, constVec, zeroVector );
				
				//store results
				ALIGNED_STORE8( &dest[i*4], v0, v1, v2, v3, v4, v5, v6, v7 );

			}
			
			//cleanup
			for ( ; i < numSamples; i++ ) {
				dest[i*4+0] = dest[i*4+1] = dest[i*4+2] = dest[i*4+3] = ogg[0][i] * 32768.0f;
			}
			
		} else {
		
			// calculate perm vec for ogg 
    		vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
			vecPerm2 = vec_add( vec_lvsl( -1, (int*) &ogg[1][0] ), (vector unsigned char)(1) );
			v7 = vec_ld( 0, &ogg[1][0] );
			v9 = vec_ld( 0, &ogg[0][0] );
			int i;
			
			for ( i = 0; i+3 < numSamples >> 1; i+=4 ) {  // +1 += 2
				// load and splat from the array ( ogg[0][i] to ogg[0][i+3] )
				v8 = v9;
				v9 = vec_ld( 15, &ogg[0][i] );
				v0 = vec_perm( v8, v9, vecPerm1 );
				
				// now we have the elements in a vector, we want
				// to splat them each accross their own vector
				oggVec1 = vec_splat( v0, 0 );
				oggVec2 = vec_splat( v0, 1 );
				oggVec3 = vec_splat( v0, 2 );
				oggVec4 = vec_splat( v0, 3 );	
				
				// load and splat from the array ( ogg[1][i] to ogg[1][i+3] )
				v6 = v7;
				v7 = vec_ld( 15, &ogg[1][i] );
				v1 = vec_perm( v6, v7, vecPerm2 );
				
				// now we have the elements in a vector, we want
				// to splat them each accross their own vector
				oggVec5 = vec_splat( v1, 0 );
				oggVec6 = vec_splat( v1, 1 );
				oggVec7 = vec_splat( v1, 2 );
				oggVec8 = vec_splat( v1, 3 );	
			
				oggVec1 = vec_madd( oggVec1, constVec, zeroVector ); // ogg[0][i] * 32768 
				oggVec2 = vec_madd( oggVec2, constVec, zeroVector ); // ogg[0][i+1] * 32768
				oggVec3 = vec_madd( oggVec3, constVec, zeroVector ); // ogg[0][i+2] * 32768
				oggVec4 = vec_madd( oggVec4, constVec, zeroVector ); // ogg[0][i+3] * 32768
				oggVec5 = vec_madd( oggVec5, constVec, zeroVector ); // ogg[1][i] * 32768
				oggVec6 = vec_madd( oggVec6, constVec, zeroVector ); // ogg[1][i+1] * 32768
				oggVec7 = vec_madd( oggVec7, constVec, zeroVector ); // ogg[1][i+2] * 32768
				oggVec8 = vec_madd( oggVec8, constVec, zeroVector ); // ogg[1][i+3] * 32768
			
				//merge generates the interleaved pattern that we want and it
				//doesn't require a permute vector, so use that instead
				v0 = vec_mergeh( oggVec1, oggVec5 );
				v1 = vec_mergel( oggVec1, oggVec5 );
				v2 = vec_mergeh( oggVec2, oggVec6 );
				v3 = vec_mergel( oggVec2, oggVec6 );
				
				v4 = vec_mergeh( oggVec3, oggVec7 );
				v5 = vec_mergel( oggVec3, oggVec7 );
				v6 = vec_mergeh( oggVec4, oggVec8 );
				v10 = vec_mergel( oggVec4, oggVec8 );
																																
				//store results
				ALIGNED_STORE8( &dest[i*8], v0, v1, v2, v3, v4, v5, v6, v10 );
			}
			
			//cleanup
			for ( ; i < numSamples >> 1; i++ ) {
				dest[i*8+0] = dest[i*8+2] = dest[i*8+4] = dest[i*8+6] = ogg[0][i] * 32768.0f;
				dest[i*8+1] = dest[i*8+3] = dest[i*8+5] = dest[i*8+7] = ogg[1][i] * 32768.0f;
			}
		}
	} else if ( kHz == 22050 ) {
		if ( numChannels == 1 ) {
			
			 // calculate perm vector and do first load
    		 vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
			 v10 = vec_ld( 0, &ogg[0][0] );
			
			int i;
			
			for ( i = 0; i+7 < numSamples; i += 8 ) {				
				// load values from ogg
				v8 = v10;
				v9 = vec_ld( 15, &ogg[0][i] );
				v10 = vec_ld( 31, &ogg[0][i] );
				v0 = vec_perm( v8, v9, vecPerm1 );
				v1 = vec_perm( v9, v10, vecPerm1 );
				
				// multiply 
				v0 = vec_madd( v0, constVec, zeroVector );
				v1 = vec_madd( v1, constVec, zeroVector );
				
				// permute into results vectors to store
				v5 = vec_perm( v0, v0, vecOneTwo );
				v6 = vec_perm( v0, v0, vecThreeFour);	
				v7 = vec_perm( v1, v1, vecOneTwo );
				v8 = vec_perm( v1, v1, vecThreeFour );
											
				//store results
				ALIGNED_STORE4( &dest[i*2], v5, v6, v7, v8 );
			}
			// cleanup
			for ( ; i < numSamples; i++ ) {
				dest[i*2+0] = dest[i*2+1] = ogg[0][i] * 32768.0f;
			}
		} else {

			// calculate perm vector and do first load
    		vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
    		vecPerm2 = vec_add( vec_lvsl( -1, (int*) &ogg[1][0] ), (vector unsigned char)(1) );
			v7 = vec_ld( 0, &ogg[1][0] );
			v9 = vec_ld( 0, &ogg[0][0] );			
			
			int i;
			for ( i = 0; i+3 < numSamples >> 1; i += 4 ) {
				// load ogg[0][i] to ogg[0][i+4]
				v8 = v9;
				v9 = vec_ld( 15, &ogg[0][i] );
				v0 = vec_perm( v8, v9, vecPerm1 );
				
				// load ogg[1][i] to ogg[1][i+3] 
				v6 = v7;
				v7 = vec_ld( 15, &ogg[1][i] );
				v1 = vec_perm( v6, v7, vecPerm2 );
				
				// multiply
				v0 = vec_madd( v0, constVec, zeroVector );
				v1 = vec_madd( v1, constVec, zeroVector );
				
				// generate result vectors to store
				v2 = vec_perm( v0, v1, vecFirst );
				v3 = vec_perm( v0, v1, vecSecond );
				v4 = vec_perm( v0, v1, vecThird );
				v5 = vec_perm( v0, v1, vecFourth );

				// store results
				ALIGNED_STORE4( &dest[i*4], v2, v3, v4, v5 );
			}
			// cleanup
			for ( ; i < numSamples >> 1; i++ ) {
				dest[i*4+0] = dest[i*4+2] = ogg[0][i] * 32768.0f;
				dest[i*4+1] = dest[i*4+3] = ogg[1][i] * 32768.0f;
			}
		}
	} else if ( kHz == 44100 ) {
		if ( numChannels == 1 ) {
			// calculate perm vector and do first load
			vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
			
			v9 = vec_ld( 0, &ogg[0][0] );
			int i;

			for ( i = 0; i+7 < numSamples; i += 8 ) { 
				// load values from ogg
				v8 = v9;
				v7 = vec_ld( 15, &ogg[0][i] );
				v6 = v7;
				v9 = vec_ld( 31, &ogg[0][i] );
				
				v0 = vec_perm( v8, v7, vecPerm1 );
				v1 = vec_perm( v6, v9, vecPerm1 );
				
				// multiply
				v0 = vec_madd( v0, constVec, zeroVector );
				v1 = vec_madd( v1, constVec, zeroVector );

				ALIGNED_STORE2( &dest[i], v0, v1 );
			}
			
			// cleanup
			for ( ; i < numSamples; i++ ) {
				dest[i*1+0] = ogg[0][i] * 32768.0f;
			}
		} else {
			
			// calculate perm vector and do first load
    		vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
    		vecPerm2 = vec_add( vec_lvsl( -1, (int*) &ogg[1][0] ), (vector unsigned char)(1) );
			v7 = vec_ld( 0, &ogg[1][0] );
			v9 = vec_ld( 0, &ogg[0][0] );			
			int i;
			
			for ( i = 0; i+3 < numSamples >> 1; i += 4 ) {
				v8 = v9;
				v9 = vec_ld( 15, &ogg[0][i] );
				v0 = vec_perm( v8, v9, vecPerm1 );
				
				// load ogg[1][i] to ogg[1][i+3] 
				v6 = v7;
				v7 = vec_ld( 15, &ogg[1][i] );
				v1 = vec_perm( v6, v7, vecPerm2 );
				
				// multiply
				v0 = vec_madd( v0, constVec, zeroVector );
				v1 = vec_madd( v1, constVec, zeroVector );
				
				// generate result vectors
				v2 = vec_mergeh( v0, v1 );
				v3 = vec_mergel( v0, v1 );
				
				// store results
				ALIGNED_STORE2( &dest[i*2], v2, v3 );
			}
			// cleanup
			for ( ; i < numSamples >> 1; i++ ) {
				dest[i*2+0] = ogg[0][i] * 32768.0f;
				dest[i*2+1] = ogg[1][i] * 32768.0f;
			}
		}
	} else {
		assert( 0 );
	}
}

#else

/*
============
idSIMD_AltiVec::UpSampleOGGTo44kHz

  Duplicate samples for 44kHz output.
  
  	Assumptions:
		No assumptions
============
*/
void idSIMD_AltiVec::UpSampleOGGTo44kHz( float *dest, const float * const *ogg, const int numSamples, const int kHz, const int numChannels ) {
	
	register vector float oggVec1, oggVec2, oggVec3, oggVec4, oggVec5, oggVec6, oggVec7, oggVec8;
	register vector float constVec, zeroVector;
	register vector float v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
	vector unsigned char vecPerm1;
	vector unsigned char vecPerm2; 

	vector unsigned char vecOneTwo = (vector unsigned char)(0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7);
	vector unsigned char vecThreeFour = (vector unsigned char)(8,9,10,11,8,9,10,11,12,13,14,15,12,13,14,15);
	vector unsigned char vecFirst = (vector unsigned char)(0,1,2,3,16,17,18,19,0,1,2,3,16,17,18,19);
	vector unsigned char vecSecond = (vector unsigned char)(4,5,6,7,20,21,22,23,4,5,6,7,20,21,22,23);
	vector unsigned char vecThird = (vector unsigned char)(8,9,10,11,24,25,26,27,8,9,10,11,24,25,26,27);
	vector unsigned char vecFourth = (vector unsigned char)(12,13,14,15,28,29,30,31,12,13,14,15,28,29,30,31);
	
	vector unsigned char storePerm;
	
	constVec = (vector float)(32768.0f);
	zeroVector = (vector float)(0.0);
	
	 // calculate perm vector and masks for stores
	 storePerm = vec_sub( vec_lvsr( 15, &dest[0] ), (vector unsigned char)(1) );
	 // original values of dest
	 vector float vecDest = vec_ld( 0, &dest[0] ); 
	 vector unsigned int mask = vec_perm( (vector unsigned int)(0), (vector unsigned int)(-1), storePerm );
	
	if ( kHz == 11025 ) {
		if ( numChannels == 1 ) {
			 // calculate perm vector and do first load
    		 vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
			 v10 = vec_ld( 0, &ogg[0][0] );
			 			
			 int i;
			 for ( i = 0; i+7 < numSamples; i += 8 ) {
				// as it happens, ogg[0][i] through ogg[0][i+3] are contiguous in memory
				v8 = v10;
				v9 = vec_ld( 15, &ogg[0][i] );
				v10 = vec_ld( 31, &ogg[0][i] );
				vector float vecDestEnd = vec_ld( 127, &dest[i*4] );
				v0 = vec_perm( v8, v9, vecPerm1 );
				v1 = vec_perm( v9, v10, vecPerm1 );
				
				// now we have the elements in a vector, we want
				// to splat them each accross their own vector
				oggVec1 = vec_splat( v0, 0 );
				oggVec2 = vec_splat( v0, 1 );
				oggVec3 = vec_splat( v0, 2 );
				oggVec4 = vec_splat( v0, 3 );		
				oggVec5 = vec_splat( v1, 0 );
				oggVec6 = vec_splat( v1, 1 );
				oggVec7 = vec_splat( v1, 2 );
				oggVec8 = vec_splat( v1, 3 );
			
				v0 = vec_madd( oggVec1, constVec, zeroVector );
				v1 = vec_madd( oggVec2, constVec, zeroVector );
				v2 = vec_madd( oggVec3, constVec, zeroVector );
				v3 = vec_madd( oggVec4, constVec, zeroVector );
				v4 = vec_madd( oggVec5, constVec, zeroVector );
				v5 = vec_madd( oggVec6, constVec, zeroVector );
				v6 = vec_madd( oggVec7, constVec, zeroVector );
				v7 = vec_madd( oggVec8, constVec, zeroVector );
				
				// rotate input data
				v0 = vec_perm( v0, v0, storePerm );
				v1 = vec_perm( v1, v1, storePerm );
				v2 = vec_perm( v2, v2, storePerm );
				v3 = vec_perm( v3, v3, storePerm );
				v4 = vec_perm( v4, v4, storePerm );
				v5 = vec_perm( v5, v5, storePerm );
				v6 = vec_perm( v6, v6, storePerm );
				v7 = vec_perm( v7, v7, storePerm );
				
				// store results
				vec_st( vec_sel( vecDest, v0, mask ), 0, &dest[i*4] );
				vec_st( vec_sel( v0, v1, mask ), 15, &dest[i*4] );
				vec_st( vec_sel( v1, v2, mask ), 31, &dest[i*4] );
				vec_st( vec_sel( v2, v3, mask ), 47, &dest[i*4] );
				vec_st( vec_sel( v3, v4, mask ), 63, &dest[i*4] );
				vec_st( vec_sel( v4, v5, mask ), 79, &dest[i*4] );
				vec_st( vec_sel( v5, v6, mask ), 95, &dest[i*4] );
				vec_st( vec_sel( v6, v7, mask ), 111, &dest[i*4] );
				vecDest = vec_sel( v7, vecDestEnd, mask );
				vec_st( vecDest, 127, &dest[i*4] );
			}
			
			//cleanup
			for ( ; i < numSamples; i++ ) {
				dest[i*4+0] = dest[i*4+1] = dest[i*4+2] = dest[i*4+3] = ogg[0][i] * 32768.0f;
			}
			
		} else {
		
			// calculate perm vec for ogg 
    		vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
			vecPerm2 = vec_add( vec_lvsl( -1, (int*) &ogg[1][0] ), (vector unsigned char)(1) );
			v7 = vec_ld( 0, &ogg[1][0] );
			v9 = vec_ld( 0, &ogg[0][0] );
			int i;
			
			for ( i = 0; i+3 < numSamples >> 1; i+=4 ) {  // +1 += 2			
				// load and splat from the array ( ogg[0][i] to ogg[0][i+3] )
				v8 = v9;
				v9 = vec_ld( 15, &ogg[0][i] );
				vector float vecDestEnd = vec_ld( 127, &dest[i*8] );
				v0 = vec_perm( v8, v9, vecPerm1 );
				
				// now we have the elements in a vector, we want
				// to splat them each accross their own vector
				oggVec1 = vec_splat( v0, 0 );
				oggVec2 = vec_splat( v0, 1 );
				oggVec3 = vec_splat( v0, 2 );
				oggVec4 = vec_splat( v0, 3 );	
				
				// load and splat from the array ( ogg[1][i] to ogg[1][i+3] )
				v6 = v7;
				v7 = vec_ld( 15, &ogg[1][i] );
				v1 = vec_perm( v6, v7, vecPerm2 );
				
				// now we have the elements in a vector, we want
				// to splat them each accross their own vector
				oggVec5 = vec_splat( v1, 0 );
				oggVec6 = vec_splat( v1, 1 );
				oggVec7 = vec_splat( v1, 2 );
				oggVec8 = vec_splat( v1, 3 );	
			
				oggVec1 = vec_madd( oggVec1, constVec, zeroVector ); // ogg[0][i] * 32768 
				oggVec2 = vec_madd( oggVec2, constVec, zeroVector ); // ogg[0][i+1] * 32768
				oggVec3 = vec_madd( oggVec3, constVec, zeroVector ); // ogg[0][i+2] * 32768
				oggVec4 = vec_madd( oggVec4, constVec, zeroVector ); // ogg[0][i+3] * 32768
				oggVec5 = vec_madd( oggVec5, constVec, zeroVector ); // ogg[1][i] * 32768
				oggVec6 = vec_madd( oggVec6, constVec, zeroVector ); // ogg[1][i+1] * 32768
				oggVec7 = vec_madd( oggVec7, constVec, zeroVector ); // ogg[1][i+2] * 32768
				oggVec8 = vec_madd( oggVec8, constVec, zeroVector ); // ogg[1][i+3] * 32768
			
				//merge generates the interleaved pattern that we want and it
				//doesn't require a permute vector, so use that instead
				v0 = vec_mergeh( oggVec1, oggVec5 );
				v1 = vec_mergel( oggVec1, oggVec5 );
				v2 = vec_mergeh( oggVec2, oggVec6 );
				v3 = vec_mergel( oggVec2, oggVec6 );
				
				v4 = vec_mergeh( oggVec3, oggVec7 );
				v5 = vec_mergel( oggVec3, oggVec7 );
				v6 = vec_mergeh( oggVec4, oggVec8 );
				v10 = vec_mergel( oggVec4, oggVec8 );
																																
				// rotate input data
				v0 = vec_perm( v0, v0, storePerm );
				v1 = vec_perm( v1, v1, storePerm );
				v2 = vec_perm( v2, v2, storePerm );
				v3 = vec_perm( v3, v3, storePerm );
				v4 = vec_perm( v4, v4, storePerm );
				v5 = vec_perm( v5, v5, storePerm );
				v6 = vec_perm( v6, v6, storePerm );
				v10 = vec_perm( v10, v10, storePerm );
				
				// store results
				vec_st( vec_sel( vecDest, v0, mask ), 0, &dest[i*8] );
				vec_st( vec_sel( v0, v1, mask ), 15, &dest[i*8] );
				vec_st( vec_sel( v1, v2, mask ), 31, &dest[i*8] );
				vec_st( vec_sel( v2, v3, mask ), 47, &dest[i*8] );
				vec_st( vec_sel( v3, v4, mask ), 63, &dest[i*8] );
				vec_st( vec_sel( v4, v5, mask ), 79, &dest[i*8] );
				vec_st( vec_sel( v5, v6, mask ), 95, &dest[i*8] );
				vec_st( vec_sel( v6, v10, mask ), 111, &dest[i*8] );
				vecDest = vec_sel( v10, vecDestEnd, mask );
				vec_st( vecDest, 127, &dest[i*8] );	
			}
			
			//cleanup
			for ( ; i < numSamples >> 1; i++ ) {
				dest[i*8+0] = dest[i*8+2] = dest[i*8+4] = dest[i*8+6] = ogg[0][i] * 32768.0f;
				dest[i*8+1] = dest[i*8+3] = dest[i*8+5] = dest[i*8+7] = ogg[1][i] * 32768.0f;
			}
		}
	} else if ( kHz == 22050 ) {
		if ( numChannels == 1 ) {
			
		 // calculate perm vector and do first load
    		 vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
			 v10 = vec_ld( 0, &ogg[0][0] );
			
			int i;
			
			for ( i = 0; i+7 < numSamples; i += 8 ) {
			
				// load values from ogg
				v8 = v10;
				v9 = vec_ld( 15, &ogg[0][i] );
				v10 = vec_ld( 31, &ogg[0][i] );
				vector float vecDestEnd = vec_ld( 63, &dest[i*2] );
				v0 = vec_perm( v8, v9, vecPerm1 );
				v1 = vec_perm( v9, v10, vecPerm1 );
				
				// multiply 
				v0 = vec_madd( v0, constVec, zeroVector );
				v1 = vec_madd( v1, constVec, zeroVector );
				
				// permute into results vectors to store
				v5 = vec_perm( v0, v0, vecOneTwo );
				v6 = vec_perm( v0, v0, vecThreeFour);	
				v7 = vec_perm( v1, v1, vecOneTwo );
				v8 = vec_perm( v1, v1, vecThreeFour );
				
				// rotate input data
				v5 = vec_perm( v5, v5, storePerm );
				v6 = vec_perm( v6, v6, storePerm );
				v7 = vec_perm( v7, v7, storePerm );
				v8 = vec_perm( v8, v8, storePerm );
														
				// store results
				vec_st( vec_sel( vecDest, v5, mask ), 0, &dest[i*2] );
				vec_st( vec_sel( v5, v6, mask ), 15, &dest[i*2] );
				vec_st( vec_sel( v6, v7, mask ), 31, &dest[i*2] );
				vec_st( vec_sel( v7, v8, mask ), 47, &dest[i*2] );
				vecDest = vec_sel( v8, vecDestEnd, mask );
				vec_st( vecDest, 63, &dest[i*2] );
			}

			// cleanup
			for ( ; i < numSamples; i++ ) {
				dest[i*2+0] = dest[i*2+1] = ogg[0][i] * 32768.0f;
			}
		} else {

			// calculate perm vector and do first load
    		vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
    		vecPerm2 = vec_add( vec_lvsl( -1, (int*) &ogg[1][0] ), (vector unsigned char)(1) );
			v7 = vec_ld( 0, &ogg[1][0] );
			v9 = vec_ld( 0, &ogg[0][0] );			
			
			int i;
			for ( i = 0; i+3 < numSamples >> 1; i += 4 ) {
				// load ogg[0][i] to ogg[0][i+4]
				v8 = v9;
				v9 = vec_ld( 15, &ogg[0][i] );
				vector float vecDestEnd = vec_ld( 63, &dest[i*4] );
				v0 = vec_perm( v8, v9, vecPerm1 );
				
				// load ogg[1][i] to ogg[1][i+3] 
				v6 = v7;
				v7 = vec_ld( 15, &ogg[1][i] );
				v1 = vec_perm( v6, v7, vecPerm2 );
				
				// multiply
				v0 = vec_madd( v0, constVec, zeroVector );
				v1 = vec_madd( v1, constVec, zeroVector );
				
				// generate result vectors to store
				v2 = vec_perm( v0, v1, vecFirst );
				v3 = vec_perm( v0, v1, vecSecond );
				v4 = vec_perm( v0, v1, vecThird );
				v5 = vec_perm( v0, v1, vecFourth );
				
				// rotate input data
				v2 = vec_perm( v2, v2, storePerm );
				v3 = vec_perm( v3, v3, storePerm );
				v4 = vec_perm( v4, v4, storePerm );
				v5 = vec_perm( v5, v5, storePerm );
														
				// store results
				vec_st( vec_sel( vecDest, v2, mask ), 0, &dest[i*4] );
				vec_st( vec_sel( v2, v3, mask ), 15, &dest[i*4] );
				vec_st( vec_sel( v3, v4, mask ), 31, &dest[i*4] );
				vec_st( vec_sel( v4, v5, mask ), 47, &dest[i*4] );
				vecDest = vec_sel( v5, vecDestEnd, mask );
				vec_st( vecDest, 63, &dest[i*4] );
			}
			
			// cleanup
			for ( ; i < numSamples >> 1; i++ ) {
				dest[i*4+0] = dest[i*4+2] = ogg[0][i] * 32768.0f;
				dest[i*4+1] = dest[i*4+3] = ogg[1][i] * 32768.0f;
			}
		}
	} else if ( kHz == 44100 ) {
		if ( numChannels == 1 ) {
			// calculate perm vector and do first load
			vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
			
			v9 = vec_ld( 0, &ogg[0][0] );
			int i;

			for ( i = 0; i+7 < numSamples; i += 8 ) { 
				// load values from ogg
				v8 = v9;
				v7 = vec_ld( 15, &ogg[0][i] );
				v6 = v7;
				v9 = vec_ld( 31, &ogg[0][i] );
				vector float vecDestEnd = vec_ld( 31, &dest[i] );
				
				v0 = vec_perm( v8, v7, vecPerm1 );
				v1 = vec_perm( v6, v9, vecPerm1 );
				
				// multiply
				v0 = vec_madd( v0, constVec, zeroVector );
				v1 = vec_madd( v1, constVec, zeroVector );
				
				// rotate data
				v0 = vec_perm( v0, v0, storePerm );
				v1 = vec_perm( v1, v1, storePerm );
		
				// store results
				vec_st( vec_sel( vecDest, v0, mask ), 0, &dest[i] );
				vec_st( vec_sel( v0, v1, mask ), 15, &dest[i] );
				vecDest = vec_sel( v1, vecDestEnd, mask );
				vec_st( vecDest, 31, &dest[i] );
			}
			
			// cleanup
			for ( ; i < numSamples; i++ ) {
				dest[i*1+0] = ogg[0][i] * 32768.0f;
			}
		} else {
			
			// calculate perm vector and do first load
    		vecPerm1 = vec_add( vec_lvsl( -1, (int*) &ogg[0][0] ), (vector unsigned char)(1) );
    		vecPerm2 = vec_add( vec_lvsl( -1, (int*) &ogg[1][0] ), (vector unsigned char)(1) );
			v7 = vec_ld( 0, &ogg[1][0] );
			v9 = vec_ld( 0, &ogg[0][0] );			
			int i;
			
			for ( i = 0; i+3 < numSamples >> 1; i += 4 ) {
				v8 = v9;
				v9 = vec_ld( 15, &ogg[0][i] );
				v0 = vec_perm( v8, v9, vecPerm1 );
					
				// load ogg[1][i] to ogg[1][i+3] 
				v6 = v7;
				v7 = vec_ld( 15, &ogg[1][i] );
				v1 = vec_perm( v6, v7, vecPerm2 );
				
				// multiply
				v0 = vec_madd( v0, constVec, zeroVector );
				v1 = vec_madd( v1, constVec, zeroVector );
				
				// generate result vectors
				v2 = vec_mergeh( v0, v1 );
				v3 = vec_mergel( v0, v1 );
				
				// store results
				UNALIGNED_STORE2( &dest[i*2], v2, v3 );
			}
			// cleanup
			for ( ; i < numSamples >> 1; i++ ) {
				dest[i*2+0] = ogg[0][i] * 32768.0f;
				dest[i*2+1] = ogg[1][i] * 32768.0f;
			}
		}
	} else {
		assert( 0 );
	}
}
#endif /* SOUND_DEST_ALIGNED */

#ifdef SOUND_DEST_ALIGNED
/*
============
idSIMD_AltiVec::MixSoundTwoSpeakerMono

	Assumptions:
		Assumes that mixBuffer starts at aligned address
============
*/
void VPCALL idSIMD_AltiVec::MixSoundTwoSpeakerMono( float *mixBuffer, const float *samples, const int numSamples, const float lastV[2], const float currentV[2] ) {

	// mixBuffer is aligned
	assert( IS_16BYTE_ALIGNED( mixBuffer[0] ) );
	
	int i;
	float inc[2];
	float spkr[4];

	register vector float vecInc;
	register vector float vecSpeaker1, vecSpeaker2, vecSpeaker3, vecSpeaker4;
	register vector float vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4;
	register vector float vecSamplesLd1, vecSamplesLd2;
	register vector float vecSamples1, vecSamples2, vecSamples3, vecSamples4;
	
	register vector unsigned char permVec1 = (vector unsigned char)(0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7); //0,0,1,1
	register vector unsigned char permVec2 = (vector unsigned char)(8,9,10,11,8,9,10,11,12,13,14,15,12,13,14,15); //2,2,3,3
	register vector unsigned char permVec3 = (vector unsigned char)(16,17,18,19,16,17,18,19,20,21,22,23,20,21,22,23); //4,4,5,5
	register vector unsigned char permVec4 = (vector unsigned char)(24,25,26,27,24,25,26,27,28,29,30,31,28,29,30,31); //6,6,7,7
	
	//constants
	vector float fourVec = (vector float)(4.0);
	vector float zeroVec = (vector float)(0.0);
	
	inc[0] = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	inc[1] = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;

	spkr[0] = lastV[0];
	spkr[1] = lastV[1];
	spkr[2] = lastV[0] + inc[0];
	spkr[3] = lastV[1] + inc[1];

	assert( numSamples == MIXBUFFER_SAMPLES );
	
	inc[0] *= 2;
	inc[1] *= 2;

	//load data into registers
	vector float v0 = loadSplatUnalignedScalar( &inc[0] );
	vector float v1 = loadSplatUnalignedScalar( &inc[1] );
	vecInc = vec_mergeh( v0, v1 );
	
	vector float v2 = loadSplatUnalignedScalar( &spkr[0] );
	vector float v3 = loadSplatUnalignedScalar( &spkr[1] );
	vector float v4 = loadSplatUnalignedScalar( &spkr[2] );
	vector float v5 = loadSplatUnalignedScalar( &spkr[3] );
	
	// load spkr array
	v0 = vec_mergeh( v2, v4 );
	v1 = vec_mergeh( v3, v5 );
	vecSpeaker1 = vec_mergeh( v0, v1 );
	
	vecSpeaker2 = vec_add( vecSpeaker1, vecInc );
	vecSpeaker3 = vec_add( vecSpeaker2, vecInc );
	vecSpeaker4 = vec_add( vecSpeaker3, vecInc );
	vecInc = vec_madd( vecInc, fourVec, zeroVec );
	
	vector unsigned char samplesPerm = vec_add( vec_lvsl( -1, &samples[0] ), (vector unsigned char)(1) );
	vector float vecSamplesLast = vec_ld( 0, &samples[0] );
	
	//since MIXBUFFER_SAMPLES is a multiple of 8, we don't
	//need a cleanup loop
	for( i=0 ; i+7 < MIXBUFFER_SAMPLES; i += 8 ) {

		//load samples and mix buffers
		vecSamplesLd1 = vecSamplesLast; //vec_ld( 0, &samples[i] );
		vecSamplesLd2 = vec_ld( 15, &samples[i] );
		vecSamplesLast = vec_ld( 31, &samples[i] );
		
		vecSamplesLd1 = vec_perm( vecSamplesLd1, vecSamplesLd2, samplesPerm );
		vecSamplesLd2 = vec_perm( vecSamplesLd2, vecSamplesLast, samplesPerm );
		
		vecMixBuffer1 = vec_ld( 0, &mixBuffer[i*2] );
		vecMixBuffer2 = vec_ld( 0, &mixBuffer[i*2+4] );
		vecMixBuffer3 = vec_ld( 0, &mixBuffer[i*2+8] );
		vecMixBuffer4 = vec_ld( 0, &mixBuffer[i*2+12] );

		vecSamples1 = vec_perm( vecSamplesLd1, vecSamplesLd2, permVec1 );
		vecSamples2 = vec_perm( vecSamplesLd1, vecSamplesLd2, permVec2 );
		vecSamples3 = vec_perm( vecSamplesLd1, vecSamplesLd2, permVec3 );
		vecSamples4 = vec_perm( vecSamplesLd1, vecSamplesLd2, permVec4 );

		vecMixBuffer1 = vec_madd( vecSamples1, vecSpeaker1, vecMixBuffer1 );
		vecMixBuffer2 = vec_madd( vecSamples2, vecSpeaker2, vecMixBuffer2 );
		vecMixBuffer3 = vec_madd( vecSamples3, vecSpeaker3, vecMixBuffer3 );
		vecMixBuffer4 = vec_madd( vecSamples4, vecSpeaker4, vecMixBuffer4 );
		
		// store results
		ALIGNED_STORE4( &mixBuffer[i*2], vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4 );
			
		//add for next iteration
		vecSpeaker1 = vec_add( vecSpeaker1, vecInc );
		vecSpeaker2 = vec_add( vecSpeaker2, vecInc );
		vecSpeaker3 = vec_add( vecSpeaker3, vecInc );
		vecSpeaker4 = vec_add( vecSpeaker4, vecInc );
	}
}

#else

/*
============
idSIMD_AltiVec::MixSoundTwoSpeakerMono

	Assumptions:
		No assumptions
============
*/
void VPCALL idSIMD_AltiVec::MixSoundTwoSpeakerMono( float *mixBuffer, const float *samples, const int numSamples, const float lastV[2], const float currentV[2] ) {
	
	int i;
	float inc[2];
	float spkr[4];

	register vector float vecInc;
	register vector float vecSpeaker1, vecSpeaker2, vecSpeaker3, vecSpeaker4;
	register vector float vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4;
	register vector float vecSamplesLd1, vecSamplesLd2;
	register vector float vecSamples1, vecSamples2, vecSamples3, vecSamples4;
	
	register vector unsigned char permVec1 = (vector unsigned char)(0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7); //0,0,1,1
	register vector unsigned char permVec2 = (vector unsigned char)(8,9,10,11,8,9,10,11,12,13,14,15,12,13,14,15); //2,2,3,3
	register vector unsigned char permVec3 = (vector unsigned char)(16,17,18,19,16,17,18,19,20,21,22,23,20,21,22,23); //4,4,5,5
	register vector unsigned char permVec4 = (vector unsigned char)(24,25,26,27,24,25,26,27,28,29,30,31,28,29,30,31); //6,6,7,7
	
	//constants
	vector float fourVec = (vector float)(4.0);
	vector float zeroVec = (vector float)(0.0);
	
	inc[0] = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	inc[1] = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;

	spkr[0] = lastV[0];
	spkr[1] = lastV[1];
	spkr[2] = lastV[0] + inc[0];
	spkr[3] = lastV[1] + inc[1];

	assert( numSamples == MIXBUFFER_SAMPLES );
	
	inc[0] *= 2;
	inc[1] *= 2;

	//load data into registers
	vector float v0 = loadSplatUnalignedScalar( &inc[0] );
	vector float v1 = loadSplatUnalignedScalar( &inc[1] );
	vecInc = vec_mergeh( v0, v1 );
	
	vector float v2 = loadSplatUnalignedScalar( &spkr[0] );
	vector float v3 = loadSplatUnalignedScalar( &spkr[1] );
	vector float v4 = loadSplatUnalignedScalar( &spkr[2] );
	vector float v5 = loadSplatUnalignedScalar( &spkr[3] );
	
	// load spkr array
	v0 = vec_mergeh( v2, v4 );
	v1 = vec_mergeh( v3, v5 );
	vecSpeaker1 = vec_mergeh( v0, v1 );

	vecSpeaker2 = vec_add( vecSpeaker1, vecInc );
	vecSpeaker3 = vec_add( vecSpeaker2, vecInc );
	vecSpeaker4 = vec_add( vecSpeaker3, vecInc );
	vecInc = vec_madd( vecInc, fourVec, zeroVec );
	
	vector unsigned char samplesPerm = vec_add( vec_lvsl( -1, &samples[0] ), (vector unsigned char)(1) );
	vector unsigned char mixBufferPerm = vec_add( vec_lvsl( -1, &mixBuffer[0]), (vector unsigned char)(1) );
	vector float vecSamplesLast = vec_ld( 0, &samples[0] );	
	vector float vecDest = vec_ld( 0, &mixBuffer[0] );
	
	//since MIXBUFFER_SAMPLES is a multiple of 8, we don't
	//need a cleanup loop
	for( i=0 ; i+7 < MIXBUFFER_SAMPLES; i += 8 ) {

		//load samples and mix buffers
		vecSamplesLd1 = vecSamplesLast; 
		vecSamplesLd2 = vec_ld( 15, &samples[i] );
		vecSamplesLast = vec_ld( 31, &samples[i] );
		
		vecSamplesLd1 = vec_perm( vecSamplesLd1, vecSamplesLd2, samplesPerm );
		vecSamplesLd2 = vec_perm( vecSamplesLd2, vecSamplesLast, samplesPerm );
		
		vecMixBuffer1 = vecDest;
		vecMixBuffer2 = vec_ld( 15, &mixBuffer[i*2] );
		vecMixBuffer3 = vec_ld( 31, &mixBuffer[i*2] );
		vecMixBuffer4 = vec_ld( 47, &mixBuffer[i*2] );
		vector float vecDestEnd = vec_ld( 63, &mixBuffer[i*2] );

		vecMixBuffer1 = vec_perm( vecMixBuffer1, vecMixBuffer2, mixBufferPerm );
		vecMixBuffer2 = vec_perm( vecMixBuffer2, vecMixBuffer3, mixBufferPerm );
		vecMixBuffer3 = vec_perm( vecMixBuffer3, vecMixBuffer4, mixBufferPerm );
		vecMixBuffer4 = vec_perm( vecMixBuffer4, vecDestEnd, mixBufferPerm );

		vecSamples1 = vec_perm( vecSamplesLd1, vecSamplesLd2, permVec1 );
		vecSamples2 = vec_perm( vecSamplesLd1, vecSamplesLd2, permVec2 );
		vecSamples3 = vec_perm( vecSamplesLd1, vecSamplesLd2, permVec3 );
		vecSamples4 = vec_perm( vecSamplesLd1, vecSamplesLd2, permVec4 );

		vecMixBuffer1 = vec_madd( vecSamples1, vecSpeaker1, vecMixBuffer1 );
		vecMixBuffer2 = vec_madd( vecSamples2, vecSpeaker2, vecMixBuffer2 );
		vecMixBuffer3 = vec_madd( vecSamples3, vecSpeaker3, vecMixBuffer3 );
		vecMixBuffer4 = vec_madd( vecSamples4, vecSpeaker4, vecMixBuffer4 );
		
		// store results
		UNALIGNED_STORE4( &mixBuffer[i*2], vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4 );		
										
		//add for next iteration
		vecSpeaker1 = vec_add( vecSpeaker1, vecInc );
		vecSpeaker2 = vec_add( vecSpeaker2, vecInc );
		vecSpeaker3 = vec_add( vecSpeaker3, vecInc );
		vecSpeaker4 = vec_add( vecSpeaker4, vecInc );
	}
}

#endif /* SOUND_DEST_ALIGNED */

#ifdef SOUND_DEST_ALIGNED
/*
============
idSIMD_AltiVec::MixSoundTwoSpeakerStereo

	Assumptions:
		Assumes that mixBuffer starts at aligned address
============
*/
void VPCALL idSIMD_AltiVec::MixSoundTwoSpeakerStereo( float *mixBuffer, const float *samples, const int numSamples, const float lastV[2], const float currentV[2] ) {
	// mixBuffer is aligned
	assert( IS_16BYTE_ALIGNED( mixBuffer[0] ) );
	
	int i, k;
	float inc[2];
	float spkr[4];
	
	// loading buffers
	register vector float vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4;
	// loading buffers
	register vector float vecSamples1, vecSamples2, vecSamples3, vecSamples4;
	register vector float vecSpeaker1, vecSpeaker2, vecSpeaker3, vecSpeaker4;
	register vector float vecInc;
	vector float fourVec = (vector float)(4.0);
	vector float zeroVec = (vector float)(0.0);
	
	assert( numSamples == MIXBUFFER_SAMPLES );

	inc[0] = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	inc[1] = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;

	spkr[0] = lastV[0];
	spkr[1] = lastV[1];
	spkr[2] = lastV[0] + inc[0];
	spkr[3] = lastV[1] + inc[1];

	for ( k = 0; k < 2; k++ ) {
		inc[k] *= 2;
	}

	// load data in vectors
	vector float v0 = loadSplatUnalignedScalar( &inc[0] );
	vector float v1 = loadSplatUnalignedScalar( &inc[1] );
	vecInc = vec_mergeh( v0, v1 );
	
	vector float v2 = loadSplatUnalignedScalar( &spkr[0] );
	vector float v3 = loadSplatUnalignedScalar( &spkr[1] );
	vector float v4 = loadSplatUnalignedScalar( &spkr[2] );
	vector float v5 = loadSplatUnalignedScalar( &spkr[3] );
	
	// load spkr array
	v0 = vec_mergeh( v2, v4 );
	v1 = vec_mergeh( v3, v5 );
	vecSpeaker1 = vec_mergeh( v0, v1 );

	vecSpeaker2 = vec_add( vecSpeaker1, vecInc );
	vecSpeaker3 = vec_add( vecSpeaker2, vecInc );
	vecSpeaker4 = vec_add( vecSpeaker3, vecInc );
	vecInc = vec_madd( vecInc, fourVec, zeroVec );
	
	vector unsigned char samplesPerm = vec_add( vec_lvsl( -1, &samples[0] ), (vector unsigned char)(1) );
	vector float vecSamplesLast = vec_ld( 0, &samples[0] );
	
	//since MIXBUFFER_SAMPLES is a multiple of 8, we don't
	//need a cleanup loop
	for( i = 0 ; i+7 < MIXBUFFER_SAMPLES; i += 8 ) { 
		// load mix buffers and samples
		vecMixBuffer1 = vec_ld( 0, &mixBuffer[i*2] );
		vecMixBuffer2 = vec_ld( 0, &mixBuffer[i*2+4] );
		vecMixBuffer3 = vec_ld( 0, &mixBuffer[i*2+8] );
		vecMixBuffer4 = vec_ld( 0, &mixBuffer[i*2+12] );
		
		vecSamples1 = vecSamplesLast; 
		vecSamples2 = vec_ld( 15, &samples[i*2] );
		vecSamples3 = vec_ld( 31, &samples[i*2] );
		vecSamples4 = vec_ld( 47, &samples[i*2] );
		vecSamplesLast = vec_ld( 63, &samples[i*2] );
		
		vecSamples1 = vec_perm( vecSamples1, vecSamples2, samplesPerm );
		vecSamples2 = vec_perm( vecSamples2, vecSamples3, samplesPerm );
		vecSamples3 = vec_perm( vecSamples3, vecSamples4, samplesPerm );
		vecSamples4 = vec_perm( vecSamples4, vecSamplesLast, samplesPerm );
		
		vecMixBuffer1 = vec_madd( vecSamples1, vecSpeaker1, vecMixBuffer1 );
		vecMixBuffer2 = vec_madd( vecSamples2, vecSpeaker2, vecMixBuffer2 );
		vecMixBuffer3 = vec_madd( vecSamples3, vecSpeaker3, vecMixBuffer3 );
		vecMixBuffer4 = vec_madd( vecSamples4, vecSpeaker4, vecMixBuffer4 );
		
		vecSpeaker1 = vec_add( vecSpeaker1, vecInc );
		vecSpeaker2 = vec_add( vecSpeaker2, vecInc );
		vecSpeaker3 = vec_add( vecSpeaker3, vecInc );
		vecSpeaker4 = vec_add( vecSpeaker4, vecInc );

		//store results
		ALIGNED_STORE4( &mixBuffer[i*2], vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4 );
	}
}
#else

/*
============
idSIMD_AltiVec::MixSoundTwoSpeakerStereo

	Assumptions:
		No assumptions
============
*/
void VPCALL idSIMD_AltiVec::MixSoundTwoSpeakerStereo( float *mixBuffer, const float *samples, const int numSamples, const float lastV[2], const float currentV[2] ) {
	
	int i, k;
	float inc[2];
	float spkr[4];
	// loading buffers
	register vector float vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4;
	// loading buffers
	register vector float vecSamples1, vecSamples2, vecSamples3, vecSamples4;
	register vector float vecSpeaker1, vecSpeaker2, vecSpeaker3, vecSpeaker4;
	register vector float vecInc;
	vector float fourVec = (vector float)(4.0);
	vector float zeroVec = (vector float)(0.0);
	
	assert( numSamples == MIXBUFFER_SAMPLES );

	inc[0] = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	inc[1] = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;

	spkr[0] = lastV[0];
	spkr[1] = lastV[1];
	spkr[2] = lastV[0] + inc[0];
	spkr[3] = lastV[1] + inc[1];

	for ( k = 0; k < 2; k++ ) {
		inc[k] *= 2;
	}

	// load data in vectors
	vector float v0 = loadSplatUnalignedScalar( &inc[0] );
	vector float v1 = loadSplatUnalignedScalar( &inc[1] );
	vecInc = vec_mergeh( v0, v1 );
	
	vector float v2 = loadSplatUnalignedScalar( &spkr[0] );
	vector float v3 = loadSplatUnalignedScalar( &spkr[1] );
	vector float v4 = loadSplatUnalignedScalar( &spkr[2] );
	vector float v5 = loadSplatUnalignedScalar( &spkr[3] );
	
	// load spkr array
	v0 = vec_mergeh( v2, v4 );
	v1 = vec_mergeh( v3, v5 );
	vecSpeaker1 = vec_mergeh( v0, v1 );

	vecSpeaker2 = vec_add( vecSpeaker1, vecInc );
	vecSpeaker3 = vec_add( vecSpeaker2, vecInc );
	vecSpeaker4 = vec_add( vecSpeaker3, vecInc );
	vecInc = vec_madd( vecInc, fourVec, zeroVec );
	
	vector unsigned char samplesPerm = vec_add( vec_lvsl( -1, &samples[0] ), (vector unsigned char)(1) );
	vector unsigned char mixBufferPerm = vec_add( vec_lvsl( -1, &mixBuffer[0] ), (vector unsigned char)(1) );
	vector float vecSamplesLast = vec_ld( 0, &samples[0] );
	vector float vecDest = vec_ld( 0, &mixBuffer[0] );
	
	//since MIXBUFFER_SAMPLES is a multiple of 8, we don't
	//need a cleanup loop
	for( i = 0 ; i+7 < MIXBUFFER_SAMPLES; i += 8 ) { 
		// load mix buffers and samples
		vecMixBuffer1 = vecDest; 
		vecMixBuffer2 = vec_ld( 15, &mixBuffer[i*2] );
		vecMixBuffer3 = vec_ld( 31, &mixBuffer[i*2] );
		vecMixBuffer4 = vec_ld( 47, &mixBuffer[i*2] );
		vector float vecDestEnd = vec_ld( 63, &mixBuffer[i*2] );

		vecMixBuffer1 = vec_perm( vecMixBuffer1, vecMixBuffer2, mixBufferPerm );
		vecMixBuffer2 = vec_perm( vecMixBuffer2, vecMixBuffer3, mixBufferPerm );
		vecMixBuffer3 = vec_perm( vecMixBuffer3, vecMixBuffer4, mixBufferPerm );
		vecMixBuffer4 = vec_perm( vecMixBuffer4, vecDestEnd, mixBufferPerm );
			
		vecSamples1 = vecSamplesLast; 
		vecSamples2 = vec_ld( 15, &samples[i*2] );
		vecSamples3 = vec_ld( 31, &samples[i*2] );
		vecSamples4 = vec_ld( 47, &samples[i*2] );
		vecSamplesLast = vec_ld( 63, &samples[i*2] );
		
		vecSamples1 = vec_perm( vecSamples1, vecSamples2, samplesPerm );
		vecSamples2 = vec_perm( vecSamples2, vecSamples3, samplesPerm );
		vecSamples3 = vec_perm( vecSamples3, vecSamples4, samplesPerm );
		vecSamples4 = vec_perm( vecSamples4, vecSamplesLast, samplesPerm );
		
		vecMixBuffer1 = vec_madd( vecSamples1, vecSpeaker1, vecMixBuffer1 );
		vecMixBuffer2 = vec_madd( vecSamples2, vecSpeaker2, vecMixBuffer2 );
		vecMixBuffer3 = vec_madd( vecSamples3, vecSpeaker3, vecMixBuffer3 );
		vecMixBuffer4 = vec_madd( vecSamples4, vecSpeaker4, vecMixBuffer4 );
		
		vecSpeaker1 = vec_add( vecSpeaker1, vecInc );
		vecSpeaker2 = vec_add( vecSpeaker2, vecInc );
		vecSpeaker3 = vec_add( vecSpeaker3, vecInc );
		vecSpeaker4 = vec_add( vecSpeaker4, vecInc );

		// store results
		UNALIGNED_STORE4( &mixBuffer[i*2], vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4 );
	}
}

#endif /* SOUND_DEST_ALIGNED */

#ifdef SOUND_DEST_ALIGNED
/*
============
idSIMD_AltiVec::MixSoundSixSpeakerMono

	Assumptions:
		Assumes that mixBuffer starts at aligned address
============
*/
void VPCALL idSIMD_AltiVec::MixSoundSixSpeakerMono( float *mixBuffer, const float *samples, const int numSamples, const float lastV[6], const float currentV[6] ) {
	
	// mixBuffer is aligned
	assert( IS_16BYTE_ALIGNED( mixBuffer[0] ) );
		
	float incL[24];
	float sL[24];
	int i, k;
	
	vector float vecIncl1, vecIncl2, vecIncl3, vecIncl4, vecIncl5, vecIncl6, vecIncl7;
	vector float vecSL1, vecSL2, vecSL3, vecSL4, vecSL5, vecSL6, vecSL7;
	vector float vecSamplesLd;
	vector float vecSamples1, vecSamples2, vecSamples3, vecSamples4, vecSamples5, vecSamples6;
	vector float vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4, vecMixBuffer5, vecMixBuffer6;
	// permute vectors for sample
	vector unsigned char samplePerm2 = (vector unsigned char)( 0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7);
	vector unsigned char samplePerm5 = (vector unsigned char)( 8,9,10,11,8,9,10,11,12,13,14,15,12,13,14,15);

	assert( numSamples == MIXBUFFER_SAMPLES );
	assert( SPEAKER_RIGHT == 1 );
	assert( SPEAKER_BACKRIGHT == 5 );
	
	// incL array, 6 elements repeated 
	incL[0] = incL[6] = incL[12] = incL[18] = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	incL[1] = incL[7] = incL[13] = incL[19] = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;
	incL[2] = incL[8] = incL[14] = incL[20] = ( currentV[2] - lastV[2] ) / MIXBUFFER_SAMPLES;
	incL[3] = incL[9] = incL[15] = incL[21] = ( currentV[3] - lastV[3] ) / MIXBUFFER_SAMPLES;
	incL[4] = incL[10] = incL[16] = incL[22] = ( currentV[4] - lastV[4] ) / MIXBUFFER_SAMPLES;
	incL[5] = incL[11] = incL[17] = incL[23] = ( currentV[5] - lastV[5] ) / MIXBUFFER_SAMPLES;
	
	// sL array repeated
	for ( k = 0; k < 6; k++ ) {
		sL[k] = lastV[k];
	}
	for ( k = 6; k < 12; k++ ) {
		sL[k] = lastV[k-6] + incL[k];
	}
	for ( k = 12; k < 18; k++ ) {
		sL[k] = lastV[k-12] + incL[k] + incL[k];
	}
	for ( k = 18; k < 24; k++ ) {
		sL[k] = lastV[k-18] + incL[k] + incL[k] + incL[k];
	}
	
	// multiply by 2 since doing 12 at a time
	for ( k = 0; k < 24; k++ ) {
		incL[k] *= 4;
	}

	//load the data
	vector unsigned char incPerm = vec_add( vec_lvsl( -1, &incL[0] ), (vector unsigned char)(1) ); 
	vector unsigned char slPerm = vec_add( vec_lvsl( -1, &sL[0] ), (vector unsigned char)(1) );	
	
	vecIncl1 = vec_ld( 0, &incL[0] );
	vecIncl2 = vec_ld( 15, &incL[0] );
	vecIncl3 = vec_ld( 31, &incL[0] );
	vecIncl4 = vec_ld( 47, &incL[0] );
	vecIncl5 = vec_ld( 63, &incL[0] );
	vecIncl6 = vec_ld( 79, &incL[0] );
	vecIncl7 = vec_ld( 95, &incL[0] );
	
	vecIncl1 = vec_perm( vecIncl1, vecIncl2, incPerm );
	vecIncl2 = vec_perm( vecIncl2, vecIncl3, incPerm );
	vecIncl3 = vec_perm( vecIncl3, vecIncl4, incPerm );
	vecIncl4 = vec_perm( vecIncl4, vecIncl5, incPerm );
	vecIncl5 = vec_perm( vecIncl5, vecIncl6, incPerm );
	vecIncl6 = vec_perm( vecIncl6, vecIncl7, incPerm );
	
	vecSL1 = vec_ld( 0, &sL[0] );
	vecSL2 = vec_ld( 15, &sL[0] );
	vecSL3 = vec_ld( 31, &sL[0] );
	vecSL4 = vec_ld( 47, &sL[0] );
	vecSL5 = vec_ld( 63, &sL[0] );
	vecSL6 = vec_ld( 79, &sL[0] );
	vecSL7 = vec_ld( 95, &sL[0] );
	
	vecSL1 = vec_perm( vecSL1, vecSL2, slPerm );
	vecSL2 = vec_perm( vecSL2, vecSL3, slPerm );
	vecSL3 = vec_perm( vecSL3, vecSL4, slPerm );
	vecSL4 = vec_perm( vecSL4, vecSL5, slPerm );
	vecSL5 = vec_perm( vecSL5, vecSL6, slPerm );
	vecSL6 = vec_perm( vecSL6, vecSL7, slPerm );
	

	vector unsigned char samplesPerm = vec_add( vec_lvsl( -1, &samples[0] ), (vector unsigned char)(1) );
	vector float vecSamplesLast = vec_ld( 0, &samples[0] );

	//since MIXBUFFER_SAMPLES is a multiple of 4, we don't
	//need a cleanup loop
	for( i = 0; i <= MIXBUFFER_SAMPLES - 4; i += 4 ) {
		//load mix buffer into vectors, assume aligned
		vecMixBuffer1 = vec_ld( 0, &mixBuffer[i*6] );
		vecMixBuffer2 = vec_ld( 0, &mixBuffer[(i*6)+4] );
		vecMixBuffer3 = vec_ld( 0, &mixBuffer[(i*6)+8] );
		vecMixBuffer4 = vec_ld( 0, &mixBuffer[(i*6)+12] );
		vecMixBuffer5 = vec_ld( 0, &mixBuffer[(i*6)+16] );
		vecMixBuffer6 = vec_ld( 0, &mixBuffer[(i*6)+20] );
		
		//load samples into vector
		vector float vecSamplesLd2 = vec_ld( 15, &samples[i] );
		vecSamplesLd = vec_perm( vecSamplesLast, vecSamplesLd2, samplesPerm );
		vecSamplesLast = vecSamplesLd2;	
			
		//permute to get them ordered how we want
		vecSamples1 = vec_splat( vecSamplesLd, 0 );
		vecSamples2 = vec_perm( vecSamplesLd, vecSamplesLd, samplePerm2 );
		vecSamples3 = vec_splat( vecSamplesLd, 1 );
		vecSamples4 = vec_splat( vecSamplesLd, 2 );
		vecSamples5 = vec_perm( vecSamplesLd, vecSamplesLd, samplePerm5 );
		vecSamples6 = vec_splat( vecSamplesLd, 3 );

		//do calculation
		vecMixBuffer1 = vec_madd( vecSamples1, vecSL1, vecMixBuffer1 );
		vecMixBuffer2 = vec_madd( vecSamples2, vecSL2, vecMixBuffer2 );
		vecMixBuffer3 = vec_madd( vecSamples3, vecSL3, vecMixBuffer3 );
		vecMixBuffer4 = vec_madd( vecSamples4, vecSL4, vecMixBuffer4 );
		vecMixBuffer5 = vec_madd( vecSamples5, vecSL5, vecMixBuffer5 );
		vecMixBuffer6 = vec_madd( vecSamples6, vecSL6, vecMixBuffer6 );
	
		//store out results
		ALIGNED_STORE6( &mixBuffer[i*6], vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4, vecMixBuffer5, vecMixBuffer6 );

		// add for next iteration
		vecSL1 = vec_add( vecSL1, vecIncl1 );
		vecSL2 = vec_add( vecSL2, vecIncl2 );
		vecSL3 = vec_add( vecSL3, vecIncl3 );
		vecSL4 = vec_add( vecSL4, vecIncl4 );
		vecSL5 = vec_add( vecSL5, vecIncl5 );
		vecSL6 = vec_add( vecSL6, vecIncl6 );
	}	
}
#else

/*
============
idSIMD_AltiVec::MixSoundSixSpeakerMono

	Assumptions:
		No assumptions
============
*/
void VPCALL idSIMD_AltiVec::MixSoundSixSpeakerMono( float *mixBuffer, const float *samples, const int numSamples, const float lastV[6], const float currentV[6] ) {
	
	float incL[24];
	float sL[24];
	int i, k;

	vector float vecIncl1, vecIncl2, vecIncl3, vecIncl4, vecIncl5, vecIncl6, vecIncl7;
	vector float vecSL1, vecSL2, vecSL3, vecSL4, vecSL5, vecSL6, vecSL7;
	vector float vecSamplesLd;
	vector float vecSamples1, vecSamples2, vecSamples3, vecSamples4, vecSamples5, vecSamples6;
	vector float vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4, vecMixBuffer5, vecMixBuffer6;
	// permute vectors for sample
	register vector unsigned char samplePerm2 = (vector unsigned char)( 0,1,2,3,0,1,2,3,4,5,6,7,4,5,6,7);
	register vector unsigned char samplePerm5 = (vector unsigned char)( 8,9,10,11,8,9,10,11,12,13,14,15,12,13,14,15);

	assert( numSamples == MIXBUFFER_SAMPLES );
	assert( SPEAKER_RIGHT == 1 );
	assert( SPEAKER_BACKRIGHT == 5 );
	
	// incL array, 6 elements repeated 
	incL[0] = incL[6] = incL[12] = incL[18] = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	incL[1] = incL[7] = incL[13] = incL[19] = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;
	incL[2] = incL[8] = incL[14] = incL[20] = ( currentV[2] - lastV[2] ) / MIXBUFFER_SAMPLES;
	incL[3] = incL[9] = incL[15] = incL[21] = ( currentV[3] - lastV[3] ) / MIXBUFFER_SAMPLES;
	incL[4] = incL[10] = incL[16] = incL[22] = ( currentV[4] - lastV[4] ) / MIXBUFFER_SAMPLES;
	incL[5] = incL[11] = incL[17] = incL[23] = ( currentV[5] - lastV[5] ) / MIXBUFFER_SAMPLES;
	
	// sL array repeated
	for ( k = 0; k < 6; k++ ) {
		sL[k] = lastV[k];
	}
	for ( k = 6; k < 12; k++ ) {
		sL[k] = lastV[k-6] + incL[k];
	}
	for ( k = 12; k < 18; k++ ) {
		sL[k] = lastV[k-12] + incL[k] + incL[k];
	}
	for ( k = 18; k < 24; k++ ) {
		sL[k] = lastV[k-18] + incL[k] + incL[k] + incL[k];
	}
	
	// multiply by 2 since doing 12 at a time
	for ( k = 0; k < 24; k++ ) {
		incL[k] *= 4;
	}

	// load the data
	vector unsigned char incPerm = vec_add( vec_lvsl( -1, &incL[0] ), (vector unsigned char)(1) ); 
	vector unsigned char slPerm = vec_add( vec_lvsl( -1, &sL[0] ), (vector unsigned char)(1) );	
	
	vecIncl1 = vec_ld( 0, &incL[0] );
	vecIncl2 = vec_ld( 15, &incL[0] );
	vecIncl3 = vec_ld( 31, &incL[0] );
	vecIncl4 = vec_ld( 47, &incL[0] );
	vecIncl5 = vec_ld( 63, &incL[0] );
	vecIncl6 = vec_ld( 79, &incL[0] );
	vecIncl7 = vec_ld( 95, &incL[0] );
	
	vecIncl1 = vec_perm( vecIncl1, vecIncl2, incPerm );
	vecIncl2 = vec_perm( vecIncl2, vecIncl3, incPerm );
	vecIncl3 = vec_perm( vecIncl3, vecIncl4, incPerm );
	vecIncl4 = vec_perm( vecIncl4, vecIncl5, incPerm );
	vecIncl5 = vec_perm( vecIncl5, vecIncl6, incPerm );
	vecIncl6 = vec_perm( vecIncl6, vecIncl7, incPerm );
	
	vecSL1 = vec_ld( 0, &sL[0] );
	vecSL2 = vec_ld( 15, &sL[0] );
	vecSL3 = vec_ld( 31, &sL[0] );
	vecSL4 = vec_ld( 47, &sL[0] );
	vecSL5 = vec_ld( 63, &sL[0] );
	vecSL6 = vec_ld( 79, &sL[0] );
	vecSL7 = vec_ld( 95, &sL[0] );
	
	vecSL1 = vec_perm( vecSL1, vecSL2, slPerm );
	vecSL2 = vec_perm( vecSL2, vecSL3, slPerm );
	vecSL3 = vec_perm( vecSL3, vecSL4, slPerm );
	vecSL4 = vec_perm( vecSL4, vecSL5, slPerm );
	vecSL5 = vec_perm( vecSL5, vecSL6, slPerm );
	vecSL6 = vec_perm( vecSL6, vecSL7, slPerm );

	vector unsigned char samplesPerm = vec_add( vec_lvsl( -1, &samples[0] ), (vector unsigned char)(1) );
	vector unsigned char mixBufferPerm = vec_add( vec_lvsl( -1, &mixBuffer[0] ), (vector unsigned char)(1) );
	vector float vecSamplesLast = vec_ld( 0, &samples[0] );
	vector float vecDest = vec_ld( 0, &mixBuffer[0] );

	//since MIXBUFFER_SAMPLES is a multiple of 4, we don't
	//need a cleanup loop
	for( i = 0; i <= MIXBUFFER_SAMPLES - 4; i += 4 ) {
		//load mix buffer into vectors		
		vecMixBuffer1 = vecDest; 
		vecMixBuffer2 = vec_ld( 15, &mixBuffer[i*6] );
		vecMixBuffer3 = vec_ld( 31, &mixBuffer[i*6] );
		vecMixBuffer4 = vec_ld( 47, &mixBuffer[i*6] );
		vecMixBuffer5 = vec_ld( 63, &mixBuffer[i*6] );
		vecMixBuffer6 = vec_ld( 79, &mixBuffer[i*6] );
		vector float vecDestEnd = vec_ld( 95, &mixBuffer[i*6] );
		
		vecMixBuffer1 = vec_perm( vecMixBuffer1, vecMixBuffer2, mixBufferPerm );
		vecMixBuffer2 = vec_perm( vecMixBuffer2, vecMixBuffer3, mixBufferPerm );
		vecMixBuffer3 = vec_perm( vecMixBuffer3, vecMixBuffer4, mixBufferPerm );
		vecMixBuffer4 = vec_perm( vecMixBuffer4, vecMixBuffer5, mixBufferPerm );
		vecMixBuffer5 = vec_perm( vecMixBuffer5, vecMixBuffer6, mixBufferPerm );
		vecMixBuffer6 = vec_perm( vecMixBuffer6, vecDestEnd, mixBufferPerm );
		
		//load samples into vector
		vector float vecSamplesLd2 = vec_ld( 15, &samples[i] );
		vecSamplesLd = vec_perm( vecSamplesLast, vecSamplesLd2, samplesPerm );
		vecSamplesLast = vecSamplesLd2;	
			
		//permute to get them ordered how we want
		vecSamples1 = vec_splat( vecSamplesLd, 0 );
		vecSamples2 = vec_perm( vecSamplesLd, vecSamplesLd, samplePerm2 );
		vecSamples3 = vec_splat( vecSamplesLd, 1 );
		vecSamples4 = vec_splat( vecSamplesLd, 2 );
		vecSamples5 = vec_perm( vecSamplesLd, vecSamplesLd, samplePerm5 );
		vecSamples6 = vec_splat( vecSamplesLd, 3 );

		//do calculation
		vecMixBuffer1 = vec_madd( vecSamples1, vecSL1, vecMixBuffer1 );
		vecMixBuffer2 = vec_madd( vecSamples2, vecSL2, vecMixBuffer2 );
		vecMixBuffer3 = vec_madd( vecSamples3, vecSL3, vecMixBuffer3 );
		vecMixBuffer4 = vec_madd( vecSamples4, vecSL4, vecMixBuffer4 );
		vecMixBuffer5 = vec_madd( vecSamples5, vecSL5, vecMixBuffer5 );
		vecMixBuffer6 = vec_madd( vecSamples6, vecSL6, vecMixBuffer6 );
	
		// store results
		UNALIGNED_STORE6( &mixBuffer[i*6], vecMixBuffer1, vecMixBuffer2, vecMixBuffer3, vecMixBuffer4, vecMixBuffer5, vecMixBuffer6 );

		// add for next iteration
		vecSL1 = vec_add( vecSL1, vecIncl1 );
		vecSL2 = vec_add( vecSL2, vecIncl2 );
		vecSL3 = vec_add( vecSL3, vecIncl3 );
		vecSL4 = vec_add( vecSL4, vecIncl4 );
		vecSL5 = vec_add( vecSL5, vecIncl5 );
		vecSL6 = vec_add( vecSL6, vecIncl6 );
	}	
}

#endif /* SOUND_DEST_ALIGNED */

#ifdef SOUND_DEST_ALIGNED
/*
============
idSIMD_AltiVec::MixSoundSixSpeakerStereo

	Assumptions:
		Assumes that mixBuffer starts at aligned address
============
*/
	
void VPCALL idSIMD_AltiVec::MixSoundSixSpeakerStereo( float *mixBuffer, const float *samples, const int numSamples, const float lastV[6], const float currentV[6] ) {

	// mixBuffer is aligned
	assert( IS_16BYTE_ALIGNED( mixBuffer[0] ) );
	
	float incL[12];
	float sL[12];
	int i;
	vector float vecIncl1, vecIncl2, vecIncl3, vecIncl4;
	vector float vecSL1, vecSL2, vecSL3, vecSL4;
	vector float vecSamplesLd;
	vector float vecSamples1, vecSamples2, vecSamples3;
	vector float vecMixBuffer1, vecMixBuffer2, vecMixBuffer3;
	// permute vectors for sample
	vector unsigned char samplePerm1 = (vector unsigned char)( 0,1,2,3,4,5,6,7,0,1,2,3,0,1,2,3);
	vector unsigned char samplePerm3 = (vector unsigned char)( 8,9,10,11,8,9,10,11,8,9,10,11,12,13,14,15);

	assert( numSamples == MIXBUFFER_SAMPLES );
	assert( SPEAKER_RIGHT == 1 );
	assert( SPEAKER_BACKRIGHT == 5 );
	
	// incL array, 6 elements repeated 
	incL[0] = incL[6] = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	incL[1] = incL[7] = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;
	incL[2] = incL[8] = ( currentV[2] - lastV[2] ) / MIXBUFFER_SAMPLES;
	incL[3] = incL[9] = ( currentV[3] - lastV[3] ) / MIXBUFFER_SAMPLES;
	incL[4] = incL[10] = ( currentV[4] - lastV[4] ) / MIXBUFFER_SAMPLES;
	incL[5] = incL[11] = ( currentV[5] - lastV[5] ) / MIXBUFFER_SAMPLES;
	
	// sL array repeated
	sL[0] = lastV[0];
	sL[1] = lastV[1];
	sL[2] = lastV[2];
	sL[3] = lastV[3];
	sL[4] = lastV[4];
	sL[5] = lastV[5];
	sL[6] = lastV[0] + incL[0];
	sL[7] = lastV[1] + incL[1];
	sL[8] = lastV[2] + incL[2];
	sL[9] = lastV[3] + incL[3];
	sL[10] = lastV[4] + incL[4];
	sL[11] = lastV[5] + incL[5];
	
	// multiply by 2 since doing 12 at a time
	incL[0] *= 2;
	incL[1] *= 2;
	incL[2] *= 2;
	incL[3] *= 2;
	incL[4] *= 2;
	incL[5] *= 2;
	incL[6] *= 2;
	incL[7] *= 2;
	incL[8] *= 2;
	incL[9] *= 2;
	incL[10] *= 2;
	incL[11] *= 2;
	
	//we aligned this data, so load it up
	vector unsigned char incPerm = vec_add( vec_lvsl( -1, &incL[0] ), (vector unsigned char)(1) ); 
	vector unsigned char slPerm = vec_add( vec_lvsl( -1, &sL[0] ), (vector unsigned char)(1) );
	vecIncl1 = vec_ld( 0, &incL[0] );
	vecIncl2 = vec_ld( 15, &incL[0] );
	vecIncl3 = vec_ld( 31, &incL[0] );
	vecIncl4 = vec_ld( 47, &incL[0] );
	
	vecIncl1 = vec_perm( vecIncl1, vecIncl2, incPerm );
	vecIncl2 = vec_perm( vecIncl2, vecIncl3, incPerm );
	vecIncl3 = vec_perm( vecIncl3, vecIncl4, incPerm );
	
	vecSL1 = vec_ld( 0, &sL[0] );
	vecSL2 = vec_ld( 15, &sL[0] );
	vecSL3 = vec_ld( 31, &sL[0] );
	vecSL4 = vec_ld( 47, &sL[0] );
	
	vecSL1 = vec_perm( vecSL1, vecSL2, slPerm );
	vecSL2 = vec_perm( vecSL2, vecSL3, slPerm );
	vecSL3 = vec_perm( vecSL3, vecSL4, slPerm );
	
	vector unsigned char samplesPerm = vec_add( vec_lvsl( -1, &samples[0] ), (vector unsigned char)(1) );
	vector float vecSamplesLast = vec_ld( 0, &samples[0] );	

	for( i = 0; i <= MIXBUFFER_SAMPLES - 2; i += 2 ) {
		
		//load mix buffer into vectors, assume aligned
		vecMixBuffer1 = vec_ld( 0, &mixBuffer[i*6] );
		vecMixBuffer2 = vec_ld( 0, &mixBuffer[(i*6)+4] );
		vecMixBuffer3 = vec_ld( 0, &mixBuffer[(i*6)+8] );
		
		//load samples into vector
		vector float vecSamplesLd2 = vec_ld( 15, &samples[i*2] );
		vecSamplesLd = vec_perm( vecSamplesLast, vecSamplesLd2, samplesPerm );
		vecSamplesLast = vecSamplesLd2;
				
		//permute to get them ordered how we want. For the 2nd vector,
		//the order happens to be the same as the order we loaded them
		//in, so there's no need to permute that one
		vecSamples1 = vec_perm( vecSamplesLd, vecSamplesLd, samplePerm1 );
		vecSamples2 = vecSamplesLd;
		vecSamples3 = vec_perm( vecSamplesLd, vecSamplesLd, samplePerm3 );

		//do calculation
		vecMixBuffer1 = vec_madd( vecSamples1, vecSL1, vecMixBuffer1 );
		vecMixBuffer2 = vec_madd( vecSamples2, vecSL2, vecMixBuffer2 );
		vecMixBuffer3 = vec_madd( vecSamples3, vecSL3, vecMixBuffer3 );
	
		//store out results
		ALIGNED_STORE3( &mixBuffer[i*6], vecMixBuffer1, vecMixBuffer2, vecMixBuffer3 );

		// add for next iteration
		vecSL1 = vec_add( vecSL1, vecIncl1 );
		vecSL2 = vec_add( vecSL2, vecIncl2 );
		vecSL3 = vec_add( vecSL3, vecIncl3 );		
	}
}
#else

/*
============
idSIMD_AltiVec::MixSoundSixSpeakerStereo

	Assumptions:
		No assumptions
============
*/	
void VPCALL idSIMD_AltiVec::MixSoundSixSpeakerStereo( float *mixBuffer, const float *samples, const int numSamples, const float lastV[6], const float currentV[6] ) {
	
	float incL[12];
	float sL[12];
	
	int i;
	vector float vecIncl1, vecIncl2, vecIncl3, vecIncl4;
	vector float vecSL1, vecSL2, vecSL3, vecSL4;
	vector float vecSamplesLd;
	vector float vecSamples1, vecSamples2, vecSamples3;
	vector float vecMixBuffer1, vecMixBuffer2, vecMixBuffer3;
	// permute vectors for sample
	vector unsigned char samplePerm1 = (vector unsigned char)( 0,1,2,3,4,5,6,7,0,1,2,3,0,1,2,3);
	vector unsigned char samplePerm3 = (vector unsigned char)( 8,9,10,11,8,9,10,11,8,9,10,11,12,13,14,15);

	assert( numSamples == MIXBUFFER_SAMPLES );
	assert( SPEAKER_RIGHT == 1 );
	assert( SPEAKER_BACKRIGHT == 5 );
	
	// incL array, 6 elements repeated 
	incL[0] = incL[6] = ( currentV[0] - lastV[0] ) / MIXBUFFER_SAMPLES;
	incL[1] = incL[7] = ( currentV[1] - lastV[1] ) / MIXBUFFER_SAMPLES;
	incL[2] = incL[8] = ( currentV[2] - lastV[2] ) / MIXBUFFER_SAMPLES;
	incL[3] = incL[9] = ( currentV[3] - lastV[3] ) / MIXBUFFER_SAMPLES;
	incL[4] = incL[10] = ( currentV[4] - lastV[4] ) / MIXBUFFER_SAMPLES;
	incL[5] = incL[11] = ( currentV[5] - lastV[5] ) / MIXBUFFER_SAMPLES;
	
	// sL array repeated
	sL[0] = lastV[0];
	sL[1] = lastV[1];
	sL[2] = lastV[2];
	sL[3] = lastV[3];
	sL[4] = lastV[4];
	sL[5] = lastV[5];
	sL[6] = lastV[0] + incL[0];
	sL[7] = lastV[1] + incL[1];
	sL[8] = lastV[2] + incL[2];
	sL[9] = lastV[3] + incL[3];
	sL[10] = lastV[4] + incL[4];
	sL[11] = lastV[5] + incL[5];
	
	// multiply by 2 since doing 12 at a time
	incL[0] *= 2;
	incL[1] *= 2;
	incL[2] *= 2;
	incL[3] *= 2;
	incL[4] *= 2;
	incL[5] *= 2;
	incL[6] *= 2;
	incL[7] *= 2;
	incL[8] *= 2;
	incL[9] *= 2;
	incL[10] *= 2;
	incL[11] *= 2;
	
	// load the data
	vector unsigned char incPerm = vec_add( vec_lvsl( -1, &incL[0] ), (vector unsigned char)(1) ); 
	vector unsigned char slPerm = vec_add( vec_lvsl( -1, &sL[0] ), (vector unsigned char)(1) );
	vecIncl1 = vec_ld( 0, &incL[0] );
	vecIncl2 = vec_ld( 15, &incL[0] );
	vecIncl3 = vec_ld( 31, &incL[0] );
	vecIncl4 = vec_ld( 47, &incL[0] );
	
	vecIncl1 = vec_perm( vecIncl1, vecIncl2, incPerm );
	vecIncl2 = vec_perm( vecIncl2, vecIncl3, incPerm );
	vecIncl3 = vec_perm( vecIncl3, vecIncl4, incPerm );
	
	vecSL1 = vec_ld( 0, &sL[0] );
	vecSL2 = vec_ld( 15, &sL[0] );
	vecSL3 = vec_ld( 31, &sL[0] );
	vecSL4 = vec_ld( 47, &sL[0] );
	
	vecSL1 = vec_perm( vecSL1, vecSL2, slPerm );
	vecSL2 = vec_perm( vecSL2, vecSL3, slPerm );
	vecSL3 = vec_perm( vecSL3, vecSL4, slPerm );

	vector unsigned char samplesPerm = vec_add( vec_lvsl( -1, &samples[0] ), (vector unsigned char)(1) );
	vector unsigned char mixBufferPerm = vec_add( vec_lvsl( -1, &mixBuffer[0] ), (vector unsigned char)(1) );
	vector float vecSamplesLast = vec_ld( 0, &samples[0] );	
	vector float vecDest = vec_ld( 0, &mixBuffer[0] );

	for( i = 0; i <= MIXBUFFER_SAMPLES - 2; i += 2 ) {
		
		//load mix buffer into vectors
		vecMixBuffer1 = vecDest; 
		vecMixBuffer2 = vec_ld( 15, &mixBuffer[i*6] );
		vecMixBuffer3 = vec_ld( 31, &mixBuffer[i*6] );
		vector float vecDestEnd = vec_ld( 47, &mixBuffer[i*6] );
		
		vecMixBuffer1 = vec_perm( vecMixBuffer1, vecMixBuffer2, mixBufferPerm );
		vecMixBuffer2 = vec_perm( vecMixBuffer2, vecMixBuffer3, mixBufferPerm );
		vecMixBuffer3 = vec_perm( vecMixBuffer3, vecDestEnd, mixBufferPerm );
		
		//load samples into vector
		vector float vecSamplesLd2 = vec_ld( 15, &samples[i*2] );
		vecSamplesLd = vec_perm( vecSamplesLast, vecSamplesLd2, samplesPerm );
		vecSamplesLast = vecSamplesLd2;
				
		//permute to get them ordered how we want. For the 2nd vector,
		//the order happens to be the same as the order we loaded them
		//in, so there's no need to permute that one
		vecSamples1 = vec_perm( vecSamplesLd, vecSamplesLd, samplePerm1 );
		vecSamples2 = vecSamplesLd;
		vecSamples3 = vec_perm( vecSamplesLd, vecSamplesLd, samplePerm3 );

		//do calculation
		vecMixBuffer1 = vec_madd( vecSamples1, vecSL1, vecMixBuffer1 );
		vecMixBuffer2 = vec_madd( vecSamples2, vecSL2, vecMixBuffer2 );
		vecMixBuffer3 = vec_madd( vecSamples3, vecSL3, vecMixBuffer3 );
	
		// store results
		UNALIGNED_STORE3( &mixBuffer[i*6], vecMixBuffer1, vecMixBuffer2, vecMixBuffer3 );

		// add for next iteration
		vecSL1 = vec_add( vecSL1, vecIncl1 );
		vecSL2 = vec_add( vecSL2, vecIncl2 );
		vecSL3 = vec_add( vecSL3, vecIncl3 );		
	}
}

#endif

/*
============
idSIMD_AltiVec::MixedSoundToSamples
============
*/
void VPCALL idSIMD_AltiVec::MixedSoundToSamples( short *samples, const float *mixBuffer, const int numSamples ) {
     //this is basically a clamp for sound mixing
    register vector float v0, v1, v2, v3, v4, v5, v6, v7;
	register vector signed int vi0, vi1, vi2, vi3;
	register vector signed short vs0, vs1;
    register vector float minVec, maxVec, constVec;
    int i = 0;

    //unaligned at start, since samples is not 16-byte aligned
    for ( ;  NOT_16BYTE_ALIGNED( samples[i] ) && ( i < numSamples ); i++ ) {
		samples[i] = mixBuffer[i] <= -32768.0f ? -32768 : mixBuffer[i] >= 32767.0f ? 32767 : (short) mixBuffer[i];
    }
    
    constVec = (vector float)(65536.0f);

    //splat min/max into a vector
    minVec = (vector float)(-32768.0f);
    maxVec = (vector float)(32767.0f);
    
	vector float vecOld = vec_ld( 0, &mixBuffer[i] );
	vector unsigned char permVec = vec_add( vec_lvsl( -1, &mixBuffer[i] ), (vector unsigned char)(1) );
	
    //vectorize!
    for ( ; i+15 < numSamples; i += 16 ) {
		//load source
		v0 = vecOld; 
		v1 = vec_ld( 15, &mixBuffer[i] );
		v2 = vec_ld( 31, &mixBuffer[i] );
		v3 = vec_ld( 31, &mixBuffer[i] );
		vecOld = vec_ld( 47, &mixBuffer[i] );

		v0 = vec_perm( v0, v1, permVec );
		v1 = vec_perm( v1, v2, permVec );
		v2 = vec_perm( v2, v3, permVec );
		v3 = vec_perm( v3, vecOld, permVec );
			
		//apply minimum
		v4 = vec_max( v0, minVec );
		v5 = vec_max( v1, minVec );
		v6 = vec_max( v2, minVec );
		v7 = vec_max( v3, minVec );
	
		//apply maximum
		v4 = vec_min( v4, maxVec );
		v5 = vec_min( v5, maxVec );
		v6 = vec_min( v6, maxVec );
		v7 = vec_min( v7, maxVec );
		
		// convert floats to ints
		vi0 = vec_cts( v4, 0 );
		vi1 = vec_cts( v5, 0 );
		vi2 = vec_cts( v6, 0 );
		vi3 = vec_cts( v7, 0 );
		
		// pack ints into shorts
		vs0 = vec_pack( vi0, vi1 );
		vs1 = vec_pack( vi2, vi3 );
		ALIGNED_STORE2( &samples[i], vs0, vs1 );
    }
    
    //handle cleanup
    for ( ; i < numSamples ; i++ ) {
		samples[i] = mixBuffer[i] <= -32768.0f ? -32768 : mixBuffer[i] >= 32767.0f ? 32767 : (short) mixBuffer[i];
    }
}
#endif /* ENABLE_SOUND_ROUTINES */

#endif /* MACOS_X */

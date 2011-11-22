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
#include "Simd_MMX.h"
#include "Simd_SSE.h"
#include "Simd_SSE2.h"
#include "Simd_SSE3.h"


//===============================================================
//
//	SSE3 implementation of idSIMDProcessor
//
//===============================================================

#if defined(MACOS_X) && defined(__i386__)

/*
============
idSIMD_SSE3::GetName
============
*/
const char * idSIMD_SSE3::GetName( void ) const {
	return "MMX & SSE & SSE2 & SSE3";
}

#elif defined(_WIN32)

#include <xmmintrin.h>

#define SHUFFLEPS( x, y, z, w )		(( (x) & 3 ) << 6 | ( (y) & 3 ) << 4 | ( (z) & 3 ) << 2 | ( (w) & 3 ))
#define R_SHUFFLEPS( x, y, z, w )	(( (w) & 3 ) << 6 | ( (z) & 3 ) << 4 | ( (y) & 3 ) << 2 | ( (x) & 3 ))
#define SHUFFLEPD( x, y )			(( (x) & 1 ) << 1 | ( (y) & 1 ))
#define R_SHUFFLEPD( x, y )			(( (y) & 1 ) << 1 | ( (x) & 1 ))

/*

	The first argument of an instruction macro is the destination
	and the second argument is the source operand. The destination
	operand can be _xmm0 to _xmm7 only. The source operand can be
	any one of the registers _xmm0 to _xmm7 or _eax, _ecx, _edx, _esp,
	_ebp, _ebx, _esi, or _edi that contains the effective address.

	For instance:  haddps   xmm0, xmm1
	becomes:       haddps( _xmm0, _xmm1 )
	and:           haddps   xmm0, [esi]
	becomes:       haddps( _xmm0, _esi )

	The ADDRESS_ADDC macro can be used when the effective source address
	is formed by adding a constant to a general purpose register.
	For instance:  haddps   xmm0, [esi+48]
	becomes:       haddps( _xmm0, ADDRESS_ADDC( _esi, 48 ) )

	The ADDRESS_ADDR macro can be used when the effective source address
	is formed by adding two general purpose registers.
	For instance:  haddps   xmm0, [esi+eax]
	becomes:       haddps( _xmm0, ADDRESS_ADDR( _esi, _eax ) )

	The ADDRESS_ADDRC macro can be used when the effective source address
	is formed by adding two general purpose registers and a constant.
	The constant must be in the range [-128, 127].
	For instance:  haddps   xmm0, [esi+eax+48]
	becomes:       haddps( _xmm0, ADDRESS_ADDRC( _esi, _eax, 48 ) )

	The ADDRESS_SCALEADDR macro can be used when the effective source address is formed
	by adding a scaled general purpose register to another general purpose register.
	The scale must be either 1, 2, 4 or 8.
	For instance:  haddps   xmm0, [esi+eax*4]
	becomes:       haddps( _xmm0, ADDRESS_SCALEADDR( _esi, _eax, 4 ) )

	The ADDRESS_SCALEADDRC macro can be used when the effective source address is formed
	by adding a scaled general purpose register to another general purpose register and
	also adding a constant. The scale must be either 1, 2, 4 or 8. The constant must
	be in the range [-128, 127].
	For instance:  haddps   xmm0, [esi+eax*4+64]
	becomes:       haddps( _xmm0, ADDRESS_SCALEADDRC( _esi, _eax, 4, 64 ) )

*/

#define _eax	0x00
#define _ecx	0x01
#define _edx	0x02
#define _ebx	0x03
#define _esp	0x04
#define _ebp	0x05
#define _esi	0x06
#define _edi	0x07

#define _xmm0	0xC0
#define _xmm1	0xC1
#define _xmm2	0xC2
#define _xmm3	0xC3
#define _xmm4	0xC4
#define _xmm5	0xC5
#define _xmm6	0xC6
#define _xmm7	0xC7

#define RSCALE( s )		( (s&2)<<5 ) | ( (s&4)<<5 ) | ( (s&8)<<3 ) | ( (s&8)<<4 )

#define ADDRESS_ADDC( reg0, constant )						0x40 | ( reg0 & 7 )	\
	_asm _emit constant

#define ADDRESS_ADDR( reg0, reg1 )							0x04				\
	_asm _emit ( ( reg1 & 7 ) << 3 ) | ( reg0 & 7 )

#define ADDRESS_ADDRC( reg0, reg1, constant )				0x44				\
	_asm _emit ( ( reg1 & 7 ) << 3 ) | ( reg0 & 7 )								\
	_asm _emit constant

#define ADDRESS_SCALEADDR( reg0, reg1, scale )				0x04				\
	_asm _emit ( ( reg1 & 7 ) << 3 ) | ( reg0 & 7 ) | RSCALE( scale )

#define ADDRESS_SCALEADDRC( reg0, reg1, scale, constant )	0x44				\
	_asm _emit ( ( reg1 & 7 ) << 3 ) | ( reg0 & 7 ) | RSCALE( scale )			\
	_asm _emit constant


// Packed Single-FP Add/Subtract ( dst[0]=dst[0]+src[0], dst[1]=dst[1]-src[1], dst[2]=dst[2]+src[2], dst[3]=dst[3]-src[3] )
#define addsubps( dst, src )						\
	_asm _emit 0xF2									\
	_asm _emit 0x0F									\
	_asm _emit 0xD0									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Packed Double-FP Add/Subtract ( dst[0]=dst[0]+src[0], dst[1]=dst[1]-src[1] )
#define addsubpd( dst, src )						\
	_asm _emit 0x66									\
	_asm _emit 0x0F									\
	_asm _emit 0xD0									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Packed Single-FP Horizontal Add ( dst[0]=dst[0]+dst[1], dst[1]=dst[2]+dst[3], dst[2]=src[0]+src[1], dst[3]=src[2]+src[3] )
#define haddps( dst, src )							\
	_asm _emit 0xF2									\
	_asm _emit 0x0F									\
	_asm _emit 0x7C									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Packed Double-FP Horizontal Add ( dst[0]=dst[0]+dst[1], dst[1]=src[0]+src[1] )
#define haddpd( dst, src )							\
	_asm _emit 0x66									\
	_asm _emit 0x0F									\
	_asm _emit 0x7C									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Packed Single-FP Horizontal Subtract ( dst[0]=dst[0]-dst[1], dst[1]=dst[2]-dst[3], dst[2]=src[0]-src[1], dst[3]=src[2]-src[3] )
#define hsubps( dst, src )							\
	_asm _emit 0xF2									\
	_asm _emit 0x0F									\
	_asm _emit 0x7D									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Packed Double-FP Horizontal Subtract ( dst[0]=dst[0]-dst[1], dst[1]=src[0]-src[1] )
#define hsubpd( dst, src )							\
	_asm _emit 0x66									\
	_asm _emit 0x0F									\
	_asm _emit 0x7D									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Move Packed Single-FP Low and Duplicate ( dst[0]=src[0], dst[1]=src[0], dst[2]=src[2], dst[3]=src[2] )
#define movsldup( dst, src )						\
	_asm _emit 0xF3									\
	_asm _emit 0x0F									\
	_asm _emit 0x12									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Move One Double-FP Low and Duplicate ( dst[0]=src[0], dst[1]=src[0] )
#define movdldup( dst, src )						\
	_asm _emit 0xF2									\
	_asm _emit 0x0F									\
	_asm _emit 0x12									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Move Packed Single-FP High and Duplicate ( dst[0]=src[1], dst[1]=src[1], dst[2]=src[3], dst[3]=src[3] )
#define movshdup( dst, src )						\
	_asm _emit 0xF3									\
	_asm _emit 0x0F									\
	_asm _emit 0x16									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Move One Double-FP High and Duplicate ( dst[0]=src[1], dst[1]=src[1] )
#define movdhdup( dst, src )						\
	_asm _emit 0xF2									\
	_asm _emit 0x0F									\
	_asm _emit 0x16									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src

// Load Unaligned Integer 128 bits
#define lddqu( dst, src )							\
	_asm _emit 0xF2									\
	_asm _emit 0x0F									\
	_asm _emit 0xF0									\
	_asm _emit ( ( dst & 7 ) << 3 ) | src


#define DRAWVERT_SIZE				60
#define DRAWVERT_XYZ_OFFSET			(0*4)
#define DRAWVERT_ST_OFFSET			(3*4)
#define DRAWVERT_NORMAL_OFFSET		(5*4)
#define DRAWVERT_TANGENT0_OFFSET	(8*4)
#define DRAWVERT_TANGENT1_OFFSET	(11*4)
#define DRAWVERT_COLOR_OFFSET		(14*4)

#define JOINTQUAT_SIZE				(7*4)
#define JOINTMAT_SIZE				(4*3*4)
#define JOINTWEIGHT_SIZE			(4*4)


/*
============
SSE3_Dot
============
*/
float SSE3_Dot( const idVec4 &v1, const idVec4 &v2 ) {
	float d;
	__asm {
		mov		esi, v1
		mov		edi, v2
		movaps	xmm0, [esi]
		mulps	xmm0, [edi]
		haddps(	_xmm0, _xmm0 )
		haddps(	_xmm0, _xmm0 )
		movss	d, xmm0
	}
	return d;
}

/*
============
idSIMD_SSE3::GetName
============
*/
const char * idSIMD_SSE3::GetName( void ) const {
	return "MMX & SSE & SSE2 & SSE3";
}

/*
============
idSIMD_SSE3::TransformVerts
============
*/
void VPCALL idSIMD_SSE3::TransformVerts( idDrawVert *verts, const int numVerts, const idJointMat *joints, const idVec4 *weights, const int *index, const int numWeights ) {
#if 1

	assert( sizeof( idDrawVert ) == DRAWVERT_SIZE );
	assert( (int)&((idDrawVert *)0)->xyz == DRAWVERT_XYZ_OFFSET );
	assert( sizeof( idVec4 ) == JOINTWEIGHT_SIZE );
	assert( sizeof( idJointMat ) == JOINTMAT_SIZE );

	__asm
	{
		mov			eax, numVerts
		test		eax, eax
		jz			done
		imul		eax, DRAWVERT_SIZE

		mov			ecx, verts
		mov			edx, index
		mov			esi, weights
		mov			edi, joints

		add			ecx, eax
		neg			eax

	loopVert:
		mov			ebx, [edx]
		movaps		xmm2, [esi]
		add			edx, 8
		movaps		xmm0, xmm2
		add			esi, JOINTWEIGHT_SIZE
		movaps		xmm1, xmm2

		mulps		xmm0, [edi+ebx+ 0]						// xmm0 = m0, m1, m2, t0
		mulps		xmm1, [edi+ebx+16]						// xmm1 = m3, m4, m5, t1
		mulps		xmm2, [edi+ebx+32]						// xmm2 = m6, m7, m8, t2

		cmp			dword ptr [edx-4], 0

		jne			doneWeight

	loopWeight:
		mov			ebx, [edx]
		movaps		xmm5, [esi]
		add			edx, 8
		movaps		xmm3, xmm5
		add			esi, JOINTWEIGHT_SIZE
		movaps		xmm4, xmm5

		mulps		xmm3, [edi+ebx+ 0]						// xmm3 = m0, m1, m2, t0
		mulps		xmm4, [edi+ebx+16]						// xmm4 = m3, m4, m5, t1
		mulps		xmm5, [edi+ebx+32]						// xmm5 = m6, m7, m8, t2

		cmp			dword ptr [edx-4], 0

		addps		xmm0, xmm3
		addps		xmm1, xmm4
		addps		xmm2, xmm5

		je			loopWeight

	doneWeight:
		add			eax, DRAWVERT_SIZE

		haddps(		_xmm0, _xmm1 )
		haddps(		_xmm2, _xmm0 )

		movhps		[ecx+eax-DRAWVERT_SIZE+0], xmm2

		haddps(		_xmm2, _xmm2 )

		movss		[ecx+eax-DRAWVERT_SIZE+8], xmm2

		jl			loopVert
	done:
	}

#else

	int i, j;
	const byte *jointsPtr = (byte *)joints;

	for( j = i = 0; i < numVerts; i++ ) {
		idVec3 v;

		v = ( *(idJointMat *) ( jointsPtr + index[j*2+0] ) ) * weights[j];
		while( index[j*2+1] == 0 ) {
			j++;
			v += ( *(idJointMat *) ( jointsPtr + index[j*2+0] ) ) * weights[j];
		}
		j++;

		verts[i].xyz = v;
	}

#endif
}

#endif /* _WIN32 */

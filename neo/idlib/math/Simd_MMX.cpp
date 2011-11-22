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


//===============================================================
//
//	MMX implementation of idSIMDProcessor
//
//===============================================================

#if defined(MACOS_X) && defined(__i386__)
/*
============
idSIMD_MMX::GetName
============
*/
const char * idSIMD_MMX::GetName( void ) const {
	return "MMX";
}

#elif defined(_WIN32)

#define EMMS_INSTRUCTION		__asm emms

/*
============
idSIMD_MMX::GetName
============
*/
const char * idSIMD_MMX::GetName( void ) const {
	return "MMX";
}

/*
================
MMX_Memcpy8B
================
*/
void MMX_Memcpy8B( void *dest, const void *src, const int count ) {
	_asm { 
        mov		esi, src 
        mov		edi, dest 
        mov		ecx, count 
        shr		ecx, 3			// 8 bytes per iteration 

loop1: 
        movq	mm1,  0[ESI]	// Read in source data 
        movntq	0[EDI], mm1		// Non-temporal stores 

        add		esi, 8
        add		edi, 8
        dec		ecx 
        jnz		loop1 

	} 
	EMMS_INSTRUCTION
}

/*
================
MMX_Memcpy64B

  165MB/sec
================
*/
void MMX_Memcpy64B( void *dest, const void *src, const int count ) {
	_asm { 
        mov		esi, src 
        mov		edi, dest 
        mov		ecx, count 
        shr		ecx, 6		// 64 bytes per iteration

loop1: 
        prefetchnta 64[ESI]	// Prefetch next loop, non-temporal 
        prefetchnta 96[ESI] 

        movq mm1,  0[ESI]	// Read in source data 
        movq mm2,  8[ESI] 
        movq mm3, 16[ESI] 
        movq mm4, 24[ESI] 
        movq mm5, 32[ESI] 
        movq mm6, 40[ESI] 
        movq mm7, 48[ESI] 
        movq mm0, 56[ESI] 

        movntq  0[EDI], mm1	// Non-temporal stores 
        movntq  8[EDI], mm2 
        movntq 16[EDI], mm3 
        movntq 24[EDI], mm4 
        movntq 32[EDI], mm5 
        movntq 40[EDI], mm6 
        movntq 48[EDI], mm7 
        movntq 56[EDI], mm0 

        add		esi, 64 
        add		edi, 64 
        dec		ecx 
        jnz		loop1 
	} 
	EMMS_INSTRUCTION
}

/*
================
MMX_Memcpy2kB

  240MB/sec
================
*/
void MMX_Memcpy2kB( void *dest, const void *src, const int count ) {
	byte *tbuf = (byte *)_alloca16(2048);
	__asm { 
		push	ebx
        mov		esi, src
        mov		ebx, count
        shr		ebx, 11		// 2048 bytes at a time 
        mov		edi, dest

loop2k:
        push	edi			// copy 2k into temporary buffer
        mov		edi, tbuf
        mov		ecx, 32

loopMemToL1: 
        prefetchnta 64[ESI] // Prefetch next loop, non-temporal
        prefetchnta 96[ESI]

        movq mm1,  0[ESI]	// Read in source data
        movq mm2,  8[ESI]
        movq mm3, 16[ESI]
        movq mm4, 24[ESI]
        movq mm5, 32[ESI]
        movq mm6, 40[ESI]
        movq mm7, 48[ESI]
        movq mm0, 56[ESI]

        movq  0[EDI], mm1	// Store into L1
        movq  8[EDI], mm2
        movq 16[EDI], mm3
        movq 24[EDI], mm4
        movq 32[EDI], mm5
        movq 40[EDI], mm6
        movq 48[EDI], mm7
        movq 56[EDI], mm0
        add		esi, 64
        add		edi, 64
        dec		ecx
        jnz		loopMemToL1

        pop		edi			// Now copy from L1 to system memory
        push	esi
        mov		esi, tbuf
        mov		ecx, 32

loopL1ToMem:
        movq mm1, 0[ESI]	// Read in source data from L1
        movq mm2, 8[ESI]
        movq mm3, 16[ESI]
        movq mm4, 24[ESI]
        movq mm5, 32[ESI]
        movq mm6, 40[ESI]
        movq mm7, 48[ESI]
        movq mm0, 56[ESI]

        movntq 0[EDI], mm1	// Non-temporal stores
        movntq 8[EDI], mm2
        movntq 16[EDI], mm3
        movntq 24[EDI], mm4
        movntq 32[EDI], mm5
        movntq 40[EDI], mm6
        movntq 48[EDI], mm7
        movntq 56[EDI], mm0

        add		esi, 64
        add		edi, 64
        dec		ecx
        jnz		loopL1ToMem

        pop		esi			// Do next 2k block
        dec		ebx
        jnz		loop2k
		pop		ebx
	}
	EMMS_INSTRUCTION
}


/*
================
idSIMD_MMX::Memcpy

  optimized memory copy routine that handles all alignment cases and block sizes efficiently
================
*/
void VPCALL idSIMD_MMX::Memcpy( void *dest0, const void *src0, const int count0 ) {
	// if copying more than 16 bytes and we can copy 8 byte aligned
	if ( count0 > 16 && !( ( (int)dest0 ^ (int)src0 ) & 7 ) ) {
		byte *dest = (byte *)dest0;
		byte *src = (byte *)src0;

		// copy up to the first 8 byte aligned boundary
		int count = ((int)dest) & 7;
		memcpy( dest, src, count );
		dest += count;
		src += count;
		count = count0 - count;

		// if there are multiple blocks of 2kB
		if ( count & ~4095 ) {
			MMX_Memcpy2kB( dest, src, count );
			src += (count & ~2047);
			dest += (count & ~2047);
			count &= 2047;
		}

		// if there are blocks of 64 bytes
		if ( count & ~63 ) {
			MMX_Memcpy64B( dest, src, count );
			src += (count & ~63);
			dest += (count & ~63);
			count &= 63;
		}

		// if there are blocks of 8 bytes
		if ( count & ~7 ) {
			MMX_Memcpy8B( dest, src, count );
			src += (count & ~7);
			dest += (count & ~7);
			count &= 7;
		}

		// copy any remaining bytes
		memcpy( dest, src, count );
	} else {
		// use the regular one if we cannot copy 8 byte aligned
		memcpy( dest0, src0, count0 );
	}

	// the MMX_Memcpy* functions use MOVNTQ, issue a fence operation
	__asm {
		sfence
	}
}

/*
================
idSIMD_MMX::Memset
================
*/
void VPCALL idSIMD_MMX::Memset( void* dest0, const int val, const int count0 ) {
	union {
		byte	bytes[8];
		word	words[4];
		dword	dwords[2];
	} dat;

	byte *dest = (byte *)dest0;
	int count = count0;

	while ( count > 0 && (((int)dest) & 7) ) {
		*dest = val;
		dest++;
		count--;
	}
	if ( !count ) {
		return;
	}

	dat.bytes[0] = val;
	dat.bytes[1] = val;
	dat.words[1] = dat.words[0];
	dat.dwords[1] = dat.dwords[0];

	if ( count >= 64 ) {
		__asm {
			mov edi, dest 
			mov ecx, count 
			shr ecx, 6				// 64 bytes per iteration 
			movq mm1, dat			// Read in source data 
			movq mm2, mm1
			movq mm3, mm1
			movq mm4, mm1
			movq mm5, mm1
			movq mm6, mm1
			movq mm7, mm1
			movq mm0, mm1
loop1: 
			movntq  0[EDI], mm1		// Non-temporal stores 
			movntq  8[EDI], mm2 
			movntq 16[EDI], mm3 
			movntq 24[EDI], mm4 
			movntq 32[EDI], mm5 
			movntq 40[EDI], mm6 
			movntq 48[EDI], mm7 
			movntq 56[EDI], mm0 

			add edi, 64 
			dec ecx 
			jnz loop1 
		}
		dest += ( count & ~63 );
		count &= 63;
	}

	if ( count >= 8 ) {
		__asm {
			mov edi, dest 
			mov ecx, count 
			shr ecx, 3				// 8 bytes per iteration 
			movq mm1, dat			// Read in source data 
loop2: 
			movntq  0[EDI], mm1		// Non-temporal stores 

			add edi, 8
			dec ecx 
			jnz loop2
		}
		dest += (count & ~7);
		count &= 7;
	}

	while ( count > 0 ) {
		*dest = val;
		dest++;
		count--;
	}

	EMMS_INSTRUCTION 

	// the MMX_Memcpy* functions use MOVNTQ, issue a fence operation
	__asm {
		sfence
	}
}

#endif /* _WIN32 */

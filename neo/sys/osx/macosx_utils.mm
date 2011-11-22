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

// -*- mode: objc -*-
#import "../../idlib/precompiled.h"

#import <Foundation/Foundation.h>
#import <mach/mach.h>
#import <mach/vm_map.h>


#define CD_MOUNT_NAME "DOOM"

#include "macosx_local.h"
#include "macosx_sys.h"

//extern "C" void *vm_allocate(unsigned, unsigned*, unsigned, int);
//extern "C" void vm_deallocate(unsigned, unsigned*, unsigned);

const char *macosx_scanForLibraryDirectory(void)
{
    return "/Library/DOOM";
}

// EEEK!
static long size_save;

void *osxAllocateMemoryNV(long size, float readFrequency, float writeFrequency, float priority)
{
    kern_return_t kr;    
    vm_address_t buffer;
    
    kr = vm_allocate(mach_task_self(),
                    (vm_address_t *)&buffer,
                    size,
                    VM_FLAGS_ANYWHERE);
    if(kr == 0) {
        size_save = size;
        return buffer;
    }
    else
    {
        size_save = 0;
        return NULL;
    }

}

void osxFreeMemoryNV(void *pointer)
{
    if(size_save) {
        vm_deallocate(mach_task_self(),
                      (vm_address_t)pointer,
                      size_save);
        size_save = 0;
    }
}

void *osxAllocateMemory(long size)
{
    kern_return_t kr;    
    vm_address_t buffer;
    
    size += sizeof( int );
    kr = vm_allocate(mach_task_self(),
                    (vm_address_t *)&buffer,
                    size,
                    VM_FLAGS_ANYWHERE);
    if(kr == 0) {
        int *ptr = buffer;
        *ptr = size;
        ptr = ptr + 1;
        return ptr;
    }
    else
    {
        return NULL;
    }

}

void osxFreeMemory(void *pointer)
{
    int size;
    int *ptr = pointer;
    ptr = ptr - 1;
    size = *ptr;
    vm_deallocate(mach_task_self(), (vm_address_t)ptr, size);
}

static inline void __eieio(void)
{
	__asm__ ("eieio");
}

static inline void __sync(void)
{
	__asm__ ("sync");
}

static inline void __isync(void)
{
	__asm__ ("isync");
}

static inline void __dcbf(void *base, unsigned long offset)
{
        __asm__ ("dcbf %0, %1"
                :
                : "r" (base), "r" (offset)
                : "r0");
}

static inline void __dcbst(void *base, unsigned long offset)
{
        __asm__ ("dcbst %0, %1"
                :
                : "r" (base), "r" (offset)
                : "r0");
}

static inline void __dcbz(void *base, unsigned long offset)
{
        __asm__ ("dcbz %0, %1"
                :
                : "r" (base), "r" (offset)
                : "r0");
}

void	Sys_FlushCacheMemory( void *base, int bytes ) {
	unsigned long i;
        
        for(i = 0; i <  bytes; i+= 32) {
            __dcbf(base,i);
        }
        __sync();
        __isync();
        __dcbf(base, i);
        __sync(); 
        __isync();
        *(volatile unsigned long *)(base + i);
        __isync(); 
}

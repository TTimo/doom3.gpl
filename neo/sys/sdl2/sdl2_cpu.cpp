#include "../../idlib/precompiled.h"
#pragma hdrstop
#include <xmmintrin.h>

// these functions are all unimplemented in SDL version (and
// OSX version also, so I guess that's ok)
const char *Sys_FPU_GetState( void )
{
	return "";
}

void Sys_FPU_SetDAZ( bool enable )
{
}

void Sys_FPU_SetFTZ( bool enable )
{	
}

/*
 ===============
 Sys_FPU_StackIsEmpty
 ===============
 */
bool Sys_FPU_StackIsEmpty( void )
{
	return true;
}

void Sys_FPU_EnableExceptions( int exceptions )
{
}

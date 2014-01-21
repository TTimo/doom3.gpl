#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "SDL.h"

int Sys_Milliseconds( void )
{
	return SDL_GetTicks();
}

int Sys_GetSystemRam( void )
{
	return SDL_GetSystemRAM();
}

int Sys_GetVideoRam( void )
{
	common->DPrintf( "TODO: __Sys_GetVideoRam\n" );
	return 0;
}

/*
================
Sys_LockMemory
================
*/
bool Sys_LockMemory( void *ptr, int bytes )
{
	// unimplemeted in SDL/OSX
	return true;
}

/*
================
Sys_UnlockMemory
================
*/
bool Sys_UnlockMemory( void *ptr, int bytes )
{
	// unimplemeted in SDL/OSX
	return true;
}

/*
 ================
 Sys_SetPhysicalWorkMemory
 ================
 */
void Sys_SetPhysicalWorkMemory( int minBytes, int maxBytes )
{
	// unimplemeted in SDL/OSX
	common->DPrintf( "TODO: Sys_SetPhysicalWorkMemory\n" );
}

/*
 ===========
 Sys_GetDriveFreeSpace
 return in MegaBytes
 ===========
 */
int Sys_GetDriveFreeSpace( const char *path )
{
	return 1000 * 1024;
}

void Sys_ShutdownSymbols( void )
{
	common->DPrintf( "TODO: __Sys_ShutdownSymbols\n" );
}

/*
 ==================
 Sys_GetCallStack
 ==================
*/
void Sys_GetCallStack( address_t *callStack, const int callStackSize )
{
	common->DPrintf( "TODO: __Sys_GetCallStack\n" );
}

/*
 ==================
 Sys_GetCallStackStr
 ==================
 */
const char *Sys_GetCallStackStr( const address_t *callStack, const int callStackSize )
{
	common->DPrintf( "TODO: __Sys_GetCallStackStr\n" );
}

/*
 ==================
 Sys_GetCallStackCurStr
 ==================
 */
const char *Sys_GetCallStackCurStr( int depth ) {
	address_t *callStack;
	
	callStack = (address_t *) _alloca( depth * sizeof( address_t ) );
	Sys_GetCallStack( callStack, depth );
	return Sys_GetCallStackStr( callStack, depth );
}

void Sys_ShowConsole( int visLevel, bool quitOnClose )
{
	common->DPrintf( "TODO: __Sys_ShowConsole\n" );
}

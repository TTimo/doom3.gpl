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
#include "../../idlib/precompiled.h"

/*
==================
Sys_ShutdownSymbols
==================
*/
void Sys_ShutdownSymbols( void ) {
}

#ifdef ID_BT_STUB

/*
==================
Sys_GetCallStack
==================
*/
void Sys_GetCallStack( address_t *callStack, const int callStackSize ) {
	for ( int i = 0; i < callStackSize; i++ ) {
		callStack[i] = 0;
	}
}

/*
==================
Sys_GetCallStackStr
==================
*/
const char * Sys_GetCallStackStr( const address_t *callStack, const int callStackSize ) {
	return "";
}

/*
==================
Sys_GetCallStackStr
==================
*/
const char * Sys_GetCallStackCurStr( int depth ) {
	return "";
}

/*
==================
Sys_GetCallStackCurAddressStr
==================
*/
const char *	Sys_GetCallStackCurAddressStr( int depth ) {
	return "";
}

#else

#include <execinfo.h>

/*
==================
Sys_GetCallStack
==================
*/
void Sys_GetCallStack( address_t *callStack, const int callStackSize ) {
	int i;
	i = backtrace( (void **)callStack, callStackSize );	
	while( i < callStackSize ) {
		callStack[i++] = 0;
	}
}

/*
==================
Sys_GetCallStackStr
==================
*/
const char *	Sys_GetCallStackStr( const address_t *callStack, int callStackSize ) {
	static char string[MAX_STRING_CHARS*2];
	char **strings;
	int i;
	
	strings = backtrace_symbols( (void **)callStack, callStackSize );
	string[ 0 ] = '\0';
	for ( i = 0; i < callStackSize; i++ ) {
		idStr::snPrintf( string + strlen( string ), MAX_STRING_CHARS*2 - strlen( string ) - 1, "%s\n", strings[ i ] );
	}
	free( strings );
	return string;	
}


/*
==================
Sys_GetCallStackStr
==================
*/
const char * Sys_GetCallStackCurStr( int depth ) {
	address_t array[ 32 ];
	size_t size;
	
	size = backtrace( (void **)array, Min( 32, depth ) );
	return Sys_GetCallStackStr( array, (int)size );
}

/*
==================
Sys_GetCallStackCurAddressStr
==================
*/
const char * Sys_GetCallStackCurAddressStr( int depth ) {
	return Sys_GetCallStackCurStr( depth );
}

#endif

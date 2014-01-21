#include "../../idlib/precompiled.h"
#pragma hdrstop

#import <Foundation/Foundation.h>
#import <dlfcn.h>

static idStr			savepath;

/*
 ==============
 Sys_EXEPath
 ==============
 */
const char *Sys_EXEPath( void ) {
	static char exepath[ 1024 ];
	strncpy( exepath, [ [ [ NSBundle mainBundle ] bundlePath ] cStringUsingEncoding:NSASCIIStringEncoding ], 1024 );
	return exepath;
}

const char *Sys_DLLPath( void ) {
	static char exepath[ 1024 ];
	strncpy( exepath, [ [ [ NSBundle mainBundle ] bundlePath ] cStringUsingEncoding:NSASCIIStringEncoding  ], 1024 );
	strcat( exepath, "/Contents/MacOS/" );
	return exepath;
}

/*
==================
OSX_GetLocalizedString
==================
*/
const char* OSX_GetLocalizedString( const char* key )
{
	NSString *string = [ [ NSBundle mainBundle ] localizedStringForKey:[ NSString stringWithCString:key encoding:NSASCIIStringEncoding]
																 value:@"No translation" table:nil];
	return [string cStringUsingEncoding:NSASCIIStringEncoding];
}

/*
 ==========
 Sys_DefaultSavePath
 ==========
 */

// should be able to use SDL_GetPrefPath
const char *Sys_DefaultSavePath(void) {
#if defined( ID_DEMO_BUILD )
	sprintf( savepath, "%s/Library/Application Support/Doom 3 Demo", [NSHomeDirectory() cStringUsingEncoding:NSASCIIStringEncoding ] );
#else
	sprintf( savepath, "%s/Library/Application Support/Doom 3", [NSHomeDirectory() cStringUsingEncoding:NSASCIIStringEncoding ] );
#endif
	return savepath.c_str();
}

/*
 ==========
 Sys_DefaultBasePath
 ==========
 */

// should be able to use SDL_BasePath
const char *Sys_DefaultBasePath(void) {
	static char basepath[ 1024 ];
	strncpy( basepath, [ [ [ NSBundle mainBundle ] bundlePath ] cStringUsingEncoding:NSASCIIStringEncoding  ], 1024 );
	char *snap = strrchr( basepath, '/' );
	if ( snap ) {
		*snap = '\0';
	}
	return basepath;
}

// only relevant when specified on command line
const char *Sys_DefaultCDPath( void ) {
	return "";
}

void Platform_Init( void )
{
}

void Platform_Quit( void )
{
	savepath.Clear();
}

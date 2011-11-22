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

#import "macosx_local.h"
#import "macosx_sys.h"

#import "macosx_common.h"
#import "dlfcn.h"

#import <AppKit/AppKit.h>

#import <sys/types.h>
#import <unistd.h>
#import <sys/param.h>
#import <sys/mount.h>
#import <sys/wait.h>

#ifdef OMNI_TIMER
#import "macosx_timers.h"
#endif

void	Sys_Init (void);

char	*Sys_GetCurrentUser( void );

void	Sys_Error( const char *error, ...);
void	Sys_Quit (void);
char	*Sys_GetClipboardData( void );	// note that this isn't journaled...

void	Sys_Print( const char *msg );
//===========================================================================

int main(int argc, const char *argv[]) {
    return NSApplicationMain(argc, argv);
}

//===========================================================================

void Sys_Sleep( const int time ) {
    sleep( time );
}

void EditorPrintConsole( const char *test ) {
}

/*
=================
Sys_UnloadDll

=================
*/
void Sys_UnloadDll( void *dllHandle ) {
}

/*
=================
Sys_LoadDll

Used to load a development dll instead of a virtual machine
=================
*/
extern char		*FS_BuildOSPath( const char *base, const char *game, const char *qpath );

void *Sys_LoadDll( const char *name, int (**entryPoint)(int, ...),
                   int (*systemcalls)(int, ...) )
{
    return NULL;
}

//===========================================================================

char *Sys_GetClipboardData(void) // FIXME
{
    NSPasteboard *pasteboard;
    NSArray *pasteboardTypes;

    pasteboard = [NSPasteboard generalPasteboard];
    pasteboardTypes = [pasteboard types];
    if ([pasteboardTypes containsObject:NSStringPboardType]) {
        NSString *clipboardString;

        clipboardString = [pasteboard stringForType:NSStringPboardType];
        if (clipboardString && [clipboardString length] > 0) {
            return strdup([clipboardString cString]);
        }
    }
    return NULL;
}

//===========================================================================

void Sys_BeginProfiling(void)
{
}

void Sys_EndProfiling(void)
{
}

//===========================================================================

/*
================
Sys_Init

The cvar and file system has been setup, so configurations are loaded
================
*/
void Sys_Init(void)
{
//    Sys_InitNetwork();
    Sys_InitInput();	
}

/*
=================
Sys_Shutdown
=================
*/
void Sys_Shutdown(void)
{
    common->Printf( "----- Sys_Shutdown -----\n" );
    Sys_EndProfiling();
    Sys_ShutdownInput();	
    common->Printf( "------------------------\n" );
}

void Sys_Error(const char *error, ...)
{
    va_list argptr;
    NSString *formattedString;

    Sys_Shutdown();

    va_start(argptr,error);
    formattedString = [[NSString alloc] initWithFormat:[NSString stringWithCString:error] arguments:argptr];
    va_end(argptr);

    NSLog(@"Sys_Error: %@", formattedString);
    NSRunAlertPanel(@"DOOM Error", formattedString, nil, nil, nil);

    Sys_Quit();
}

void Sys_Quit(void)
{
    Sys_Shutdown();
    [NSApp terminate:nil];
}

/*
================
Sys_Print

This is called for all console output, even if the game is running
full screen and the dedicated console window is hidden.
================
*/

char *ansiColors[8] =
	{ "\033[30m" ,	/* ANSI Black */
	  "\033[31m" ,	/* ANSI Red */
	  "\033[32m" ,	/* ANSI Green */
	  "\033[33m" ,  /* ANSI Yellow */
	  "\033[34m" ,	/* ANSI Blue */
	  "\033[36m" ,  /* ANSI Cyan */
	  "\033[35m" ,	/* ANSI Magenta */
	  "\033[37m" }; /* ANSI White */
	  
void Sys_Print(const char *text)
{
#if 0
	/* Okay, this is a stupid hack, but what the hell, I was bored. ;) */
	char *scan = text;
	char code;
	int index;
	
	/* Make sure terminal mode is reset at the start of the line... */
	fputs("\033[0m", stdout);
	
	while(*scan) {
		/* See if we have a color control code.  If so, snarf the character, 
		print what we have so far, print the ANSI Terminal color code,
		skip over the color control code and continue */
		if(Q_IsColorString(scan)) {
			index = ColorIndex(scan[1]);
			
			/* Flush current message */
			if(scan != text) {
				fwrite(text, scan - text, 1, stdout);
			}
			
			/* Write ANSI color code */
			fputs(ansiColors[index], stdout);
			
			/* Reset search */
			text = scan+2;
			scan = text;
			continue;			
		}
		scan++;
	}

	/* Flush whatever's left */
	fputs(text, stdout);

	/* Make sure terminal mode is reset at the end of the line too... */
	fputs("\033[0m", stdout);

#else
    fputs(text, stdout);
#endif	
}

void OutputDebugString( const char *text ) {
    Sys_Print( text );
}

void Sys_OutputDebugString( const char *text ) {
    OutputDebugString( text );
}

/*
================
Sys_CheckCD

Return true if the proper CD is in the drive
================
*/
bool        Sys_CheckCD( void ) {
    return macosx_scanForLibraryDirectory() != NULL;
}

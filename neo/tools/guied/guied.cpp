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
#pragma hdrstop

#include "../../renderer/tr_local.h"
#include "../../sys/win32/win_local.h"
#include <io.h>

#include "../../ui/DeviceContext.h"
#include "../../sys/win32/rc/guied_resource.h"

#include "GEApp.h"

rvGEApp		gApp;
	
/*
================
GUIEditorInit

Start the gui editor
================
*/
void GUIEditorInit( void ) 
{
	gApp.Initialize();
}

/*
================
GUIEditorShutdown
================
*/
void GUIEditorShutdown( void ) {
}

/*
================
GUIEditorHandleMessage

Handle translator messages
================
*/
bool GUIEditorHandleMessage ( void *msg )
{
	if ( !gApp.IsActive ( ) )
	{
		return false;
	}

	return gApp.TranslateAccelerator( reinterpret_cast<LPMSG>(msg) );
}

/*
================
GUIEditorRun

Run a frame 
================
*/
void GUIEditorRun() 
{
    MSG			msg;

	// pump the message loop
	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE)) 
	{
		if ( !GetMessage (&msg, NULL, 0, 0) ) 
		{
			common->Quit();
		}

		// save the msg time, because wndprocs don't have access to the timestamp
		if ( win32.sysMsgTime && win32.sysMsgTime > (int)msg.time ) 
		{
		} 
		else 
		{
			win32.sysMsgTime = msg.time;
		}

		if ( gApp.TranslateAccelerator ( &msg ) )
		{
			continue;
		}
 
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}

	gApp.RunFrame ( );
	
	// The GUI editor runs too hot so we need to slow it down a bit.
	Sleep ( 1 );
}

/*
================
StringFromVec4

Returns a clean string version of the given vec4
================
*/
const char *StringFromVec4 ( idVec4& v )
{
	return va( "%s,%s,%s,%s",
		idStr::FloatArrayToString( &v[0], 1, 8 ),
		idStr::FloatArrayToString( &v[1], 1, 8 ),
		idStr::FloatArrayToString( &v[2], 1, 8 ),
		idStr::FloatArrayToString( &v[3], 1, 8 ) );
}

/*
================
IsExpression

Returns true if the given string is an expression
================
*/
bool IsExpression ( const char* s )
{
	idParser src( s, strlen ( s ), "", 
				  LEXFL_ALLOWMULTICHARLITERALS		| 
				  LEXFL_NOSTRINGCONCAT				| 
				  LEXFL_ALLOWBACKSLASHSTRINGCONCAT	|
				  LEXFL_NOFATALERRORS );

	idToken token;
	bool	needComma = false;
	bool	needNumber = false;
	while ( src.ReadToken ( &token ) )
	{
		switch ( token.type )
		{
			case TT_NUMBER:
				needComma = true;
				needNumber = false;
				break;
			
			case TT_PUNCTUATION:
				if ( needNumber )
				{
					return true;
				}				
				if ( token[0] == ',' )
				{
					if ( !needComma )
					{
						return true;
					}
					
					needComma = false;
					break;
				}

				if ( needComma )
				{
					return true;
				}

				if ( token[0] == '-' )
				{
					needNumber = true;
				}
				break;
				
			default:
				return true;
		}
	}
					
	return false;
}

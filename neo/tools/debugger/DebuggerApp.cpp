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

#include "../../sys/win32/rc/debugger_resource.h"
#include "DebuggerApp.h"

/*
================
rvDebuggerApp::rvDebuggerApp
================
*/
rvDebuggerApp::rvDebuggerApp ( ) :
	mOptions ( "Software\\id Software\\DOOM3\\Tools\\Debugger" )
{
	mInstance		= NULL;
	mDebuggerWindow = NULL;
	mAccelerators   = NULL;
}

/*
================
rvDebuggerApp::~rvDebuggerApp
================
*/
rvDebuggerApp::~rvDebuggerApp ( )
{
	if ( mAccelerators )
	{
		DestroyAcceleratorTable ( mAccelerators );
	}
}

/*
================
rvDebuggerApp::Initialize

Initializes the debugger application by creating the debugger window
================
*/
bool rvDebuggerApp::Initialize ( HINSTANCE instance )
{
	INITCOMMONCONTROLSEX ex;
	ex.dwICC = ICC_USEREX_CLASSES | ICC_LISTVIEW_CLASSES | ICC_WIN95_CLASSES;
	ex.dwSize = sizeof(INITCOMMONCONTROLSEX);

	mInstance = instance;

	mOptions.Load ( );

	mDebuggerWindow = new rvDebuggerWindow;
		
	if ( !mDebuggerWindow->Create ( instance ) )
	{
		delete mDebuggerWindow;
		return false;
	}

	// Initialize the network connection for the debugger
	if ( !mClient.Initialize ( ) )
	{
		return false;
	}	

	mAccelerators = LoadAccelerators ( mInstance, MAKEINTRESOURCE(IDR_DBG_ACCELERATORS) );

	return true;
}

/*
================
rvDebuggerApp::ProcessWindowMessages

Process windows messages
================
*/
bool rvDebuggerApp::ProcessWindowMessages ( void )
{
	MSG	msg;

	while ( PeekMessage ( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		if ( !GetMessage (&msg, NULL, 0, 0) ) 
		{
			return false;
		}
		
		if ( !TranslateAccelerator ( &msg ) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return true;
}

/*
================
rvDebuggerApp::TranslateAccelerator

Translate any accelerators destined for this window
================
*/
bool rvDebuggerApp::TranslateAccelerator ( LPMSG msg )
{
	if ( mDebuggerWindow && ::TranslateAccelerator ( mDebuggerWindow->GetWindow(), mAccelerators, msg ) )
	{
		return true;
	}
		
	return false;
}

/*
================
rvDebuggerApp::Run

Main Loop for the debugger application
================
*/
int rvDebuggerApp::Run ( void )
{		
	// Main message loop:
	while ( ProcessWindowMessages ( ) )
	{
		mClient.ProcessMessages ( );
		
		Sleep ( 0 );
	}
	
	mClient.Shutdown ( );
	mOptions.Save ( );
	
	delete mDebuggerWindow;
	
	return 1;
}


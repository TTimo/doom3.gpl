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

#include "../../ui/Window.h"
#include "../../ui/UserInterfaceLocal.h"

#include "GEOptions.h"

rvGEOptions::rvGEOptions() {
	// Grid options
	mGridColor.Set ( 0.2f, 0.2f, 1.0f, 1.0f );
	mGridWidth = 10;
	mGridHeight = 10;
	mGridSnap = false;
	mGridVisible = false;
	mNavigatorVisible = true;
	mTransformerVisible = true;
	mIgnoreDesktopSelect = true;
	mStatusBarVisible = true;
	mPropertiesVisible = true;

	mWorkspaceColor.Set ( 0.0f, 0.0f, 0.0f, 1.0f );
	mSelectionColor.Set ( 0.5f, 0.5f, 1.0f, 1.0f );
	
	memset ( mCustomColors, 0, sizeof(mCustomColors) );
}

/*
================
rvGEOptions::Init
================
*/
void rvGEOptions::Init( void ) {
	mRegistry.Init( "Software\\id Software\\DOOM3\\Tools\\GUIEditor" );
}

/*
================
rvGEOptions::Save

Writes the options to the registry so they can later be read using the Load method
================
*/
bool rvGEOptions::Save ( void )
{
	// Write the last page we visited
	mRegistry.SetLong ( "lastOptionsPage", mLastOptionsPage );
	
	// Write the grid settings
	mRegistry.SetVec4 ( "gridColor", idVec4(mGridColor[0],mGridColor[1],mGridColor[2],1.0f) );
	mRegistry.SetLong ( "gridWidth", mGridWidth );
	mRegistry.SetLong ( "gridHeight", mGridHeight );
	mRegistry.SetBool ( "gridSnap", mGridSnap );
	mRegistry.SetBool ( "gridVisible", mGridVisible );
		
	// Tool window states	
	mRegistry.SetBool ( "navigatorVisible", mNavigatorVisible );
	mRegistry.SetBool ( "PropertiesVisible", mPropertiesVisible );
	mRegistry.SetBool ( "transformerVisible", mTransformerVisible );
	mRegistry.SetBool ( "statusBarVisible", mStatusBarVisible );

	// General stuff
	mRegistry.SetVec4 ( "selectionColor", mSelectionColor );
	mRegistry.SetBool ( "ignoreDesktopSelect", mIgnoreDesktopSelect );

	// Custom colors
	int i;
	for ( i = 0; i < 16; i ++ )
	{
		mRegistry.SetLong ( va("customcol%d",i), mCustomColors[i] );
	}

	return mRegistry.Save ( );
}

/*
================
rvGEOptions::Load

Loads previsouly saved options from the registry
================
*/
bool rvGEOptions::Load ( void )
{
	if ( !mRegistry.Load ( ) )
	{
		return false;
	}

	// Read the general stuff
	mLastOptionsPage = mRegistry.GetLong ( "lastOptionsPage" );

	// Read the grid settings	
	mGridColor = mRegistry.GetVec4 ( "gridColor" );
	mGridWidth = mRegistry.GetLong ( "gridWidth" );
	mGridHeight = mRegistry.GetLong ( "gridHeight" );
	mGridSnap  = mRegistry.GetBool ( "gridSnap" );
	mGridVisible = mRegistry.GetBool ( "gridVisible" );
		
	// Tool window states
	mNavigatorVisible = mRegistry.GetBool ( "navigatorVisible" );
	mPropertiesVisible = mRegistry.GetBool ( "PropertiesVisible" );
	mTransformerVisible = mRegistry.GetBool ( "transformerVisible" );
	mStatusBarVisible = mRegistry.GetBool ( "statusBarVisible" );

	// General stuff
	mSelectionColor = mRegistry.GetVec4 ( "selectionColor" );
	mIgnoreDesktopSelect = mRegistry.GetBool ( "ignoreDesktopSelect" );

	// Custom colors
	int i;
	for ( i = 0; i < 16; i ++ )
	{
		mCustomColors[i] = mRegistry.GetLong ( va("customcol%d",i) );
	}
	
	return true;
}

/*
================
rvGEOptions::SnapRectToGrid

Snap the rectangle to the grid
================
*/
void rvGEOptions::SnapRectToGrid ( idRectangle& rect, bool snapLeft, bool snapTop, bool snapWidth, bool snapHeight )
{
	if ( snapLeft )
	{
		float offset = (int)(rect.x + GetGridWidth() / 2) / GetGridWidth() * GetGridWidth();
		offset -= rect.x;
		rect.x += offset;
		rect.w -= offset;
	}

	if ( snapWidth )
	{					
		float offset = (int)(rect.x + rect.w + GetGridWidth() / 2) / GetGridWidth() * GetGridWidth();
		offset -= rect.x;
		offset -= rect.w;
		rect.w += offset;
	}
	
	if ( snapTop )
	{
		float offset = (int)(rect.y + GetGridHeight() / 2) / GetGridHeight() * GetGridHeight();
		offset -= rect.y;
		rect.y += offset;
		rect.h -= offset;
	}

	if ( snapHeight )
	{					
		float offset = (int)(rect.y + rect.h + GetGridHeight() / 2) / GetGridHeight() * GetGridHeight();
		offset -= rect.y;
		offset -= rect.h;
		rect.h += offset;
	}		
}



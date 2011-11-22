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

#include "GEApp.h"
#include "GEInsertModifier.h"

rvGEInsertModifier::rvGEInsertModifier ( const char* name, idWindow* window, idWindow* parent, idWindow* before ) :
	rvGEModifier ( name, window )
{
	mParent  = parent;
	mBefore  = before;

	assert ( mParent );

	mUndoParent = window->GetParent ( );
	mUndoBefore = NULL;	
	mUndoRect   = mWrapper->GetClientRect ( );
	mRect		= mWrapper->GetClientRect ( );
		
	// Find the child window the window being inserted is before
	if ( mUndoParent )
	{
		int				   index;
		rvGEWindowWrapper* pwrapper;
		
		pwrapper = rvGEWindowWrapper::GetWrapper ( mUndoParent );
		
		index = mUndoParent->GetChildIndex ( mWindow );
		
		if ( index + 1 < pwrapper->GetChildCount ( ) )
		{
			mUndoBefore = pwrapper->GetChild ( index + 1 );
		}
	}		

	// Since rectangles are relative to the parent rectangle we need to figure
	// out the new x and y coordinate as if this window were a child 
	rvGEWindowWrapper* parentWrapper;		
	parentWrapper = rvGEWindowWrapper::GetWrapper ( mParent );
	mRect.x = mWrapper->GetScreenRect( )[0] - parentWrapper->GetScreenRect()[0];
	mRect.y = mWrapper->GetScreenRect( )[1] - parentWrapper->GetScreenRect()[1];
}

/*
================
rvGEInsertModifier::Apply

Apply the insert modifier by removing the child from its original parent and
inserting it as a child of the new parent
================
*/
bool rvGEInsertModifier::Apply ( void )
{
	if ( mUndoParent )
	{
		mUndoParent->RemoveChild ( mWindow );
	}
	
	mParent->InsertChild ( mWindow, mBefore );
	mWrapper->SetRect ( mRect );
	
	return true;
}

/*
================
rvGEInsertModifier::Undo

Undo the insert modifier by removing the window from the parent it was
added to and re-inserting it back into its original parent
================
*/
bool rvGEInsertModifier::Undo ( void )
{
	mParent->RemoveChild ( mWindow );
	
	if ( mUndoParent )
	{
		mUndoParent->InsertChild ( mWindow, mUndoBefore );
		mWrapper->SetRect ( mUndoRect );
	}
	
	return true;
}


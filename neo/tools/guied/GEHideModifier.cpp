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
#include "GEHideModifier.h"

rvGEHideModifier::rvGEHideModifier ( const char* name, idWindow* window, bool hide ) :
	rvGEModifier ( name, window )
{
	mParent		= NULL;
	mHide		= hide;
	mUndoHide	= mWrapper->IsHidden ( );

	// If unhiding then find any parent window along the way that may be hidden and prevent
	// this window from being visible
	if ( !hide )
	{
		mParent = mWindow;
		while ( NULL != (mParent = mParent->GetParent ( ) ) )
		{
			if ( rvGEWindowWrapper::GetWrapper(mParent)->IsHidden ( ) )
			{
				break;
			}
		}
	}
}

/*
================
rvGEHideModifier::Apply

Apply the hide modifier by setting the visible state of the wrapper window
================
*/
bool rvGEHideModifier::Apply ( void )
{
	mWrapper->SetHidden ( mHide );

	if ( mParent )
	{
		rvGEWindowWrapper::GetWrapper ( mParent )->SetHidden ( mHide );
	}
		
	return true;
}

/*
================
rvGEHideModifier::Undo

Undo the hide modifier by setting the undo visible state of the wrapper window
================
*/
bool rvGEHideModifier::Undo ( void )
{
	mWrapper->SetHidden ( mUndoHide );

	if ( mParent )
	{
		rvGEWindowWrapper::GetWrapper ( mParent )->SetHidden ( mUndoHide );
	}
	
	return true;
}


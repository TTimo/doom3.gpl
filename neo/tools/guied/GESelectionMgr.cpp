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

#include "GEApp.h"
#include "GESelectionMgr.h"

#define GUIED_GRABSIZE		7
#define GUIED_CENTERSIZE	5

rvGESelectionMgr::rvGESelectionMgr ( )
{
	mWorkspace = NULL;
}

/*
================
rvGESelectionMgr::SetSelection

Sets the only selection for the workspace to the given window
================
*/
void rvGESelectionMgr::Set ( idWindow* window )
{
	// Get rid of any current selections
	Clear ( );
	
	// Add this selection now
	return Add ( window );
}

/*
================
rvGESelectionMgr::Add

Adds the given window to the selection list
================
*/
void rvGESelectionMgr::Add ( idWindow* window, bool expand )
{
	rvGEWindowWrapper* wrapper;
	
	wrapper = rvGEWindowWrapper::GetWrapper ( window );
	assert ( wrapper );

	// If the window is already selected then dont add the selection
	if ( wrapper->IsSelected ( ) )
	{
		return;
	}

	wrapper->SetSelected ( true );
	
	mSelections.Append ( window );		
	
	if ( expand && wrapper->Expand ( ) )
	{
		gApp.GetNavigator ( ).Update ( );
	}

	gApp.GetNavigator ( ).UpdateSelections ( );
	gApp.GetNavigator ( ).Refresh ( );
	gApp.GetTransformer ( ).Update ( );
	gApp.GetProperties().Update ( );
	
	UpdateExpression ( );
}

/*
================
rvGESelectionMgr::RemoveSelection

Removes the selection from the current workspace
================
*/
void rvGESelectionMgr::Remove ( idWindow* window )
{
	rvGEWindowWrapper* wrapper;
	
	wrapper = rvGEWindowWrapper::GetWrapper ( window );
	assert ( wrapper );

	// Dont bother if the window isnt selectd already
	if ( !wrapper->IsSelected ( ) )
	{
		return;
	}

	wrapper->SetSelected ( false );
	
	mSelections.Remove ( window );

	gApp.GetNavigator ( ).UpdateSelections ( );
	gApp.GetNavigator ( ).Refresh ( );
	gApp.GetTransformer ( ).Update ( );
	gApp.GetProperties().Update ( );
	
	UpdateExpression ( );
}

/*
================
rvGESelectionMgr::ClearSelections

Remove all of the current selections
================
*/
void rvGESelectionMgr::Clear ( void )
{
	int i;

	for ( i = 0; i < mSelections.Num ( ); i ++ )
	{
		rvGEWindowWrapper::GetWrapper ( mSelections[i] )->SetSelected ( false );
	}

	mSelections.Clear ( );
	
	gApp.GetNavigator ( ).UpdateSelections ( );
	gApp.GetNavigator ( ).Refresh ( );
	gApp.GetTransformer ( ).Update ( );
	gApp.GetProperties().Update ( );
	
	mExpression = false;
}

/*
================
rvGESelectionMgr::Render

Render the selections including the move/size bars
================
*/
void rvGESelectionMgr::Render ( void )
{
	if ( !mSelections.Num ( ) )
	{
		return;
	}

	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	UpdateRectangle ( );

	qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
	
	idVec4&	color = gApp.GetOptions().GetSelectionColor ( );
 	qglColor4f ( color[0],color[1],color[2], 1.0f );
	
	qglBegin(GL_LINE_LOOP );	
	qglVertex2f ( mRect.x, mRect.y );
	qglVertex2f ( mRect.x + mRect.w, mRect.y );
	qglVertex2f ( mRect.x + mRect.w, mRect.y + mRect.h);
	qglVertex2f ( mRect.x, mRect.y + mRect.h);
	qglEnd ( );		

 	qglColor4f ( color[0],color[1],color[2], 0.75f );

	int i;
	for ( i = 0; i < mSelections.Num(); i ++ )
	{
		rvGEWindowWrapper*	wrapper;
		idRectangle			rect;
		
		wrapper = rvGEWindowWrapper::GetWrapper ( mSelections[i] );
		assert ( wrapper );
		
		rect = wrapper->GetScreenRect ( );
		mWorkspace->WorkspaceToWindow ( rect );

		if ( i == 0 )
		{
			qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
			qglBegin ( GL_TRIANGLES );
			qglVertex2f ( rect.x, rect.y );
			qglVertex2f ( rect.x + GUIED_GRABSIZE, rect.y );
			qglVertex2f ( rect.x, rect.y + GUIED_GRABSIZE );
			qglEnd ( );
		}			
			
		qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
		qglBegin(GL_LINE_LOOP );	
		qglVertex2f ( rect.x, rect.y );
		qglVertex2f ( rect.x + rect.w, rect.y );
		qglVertex2f ( rect.x + rect.w, rect.y + rect.h);
		qglVertex2f ( rect.x, rect.y + rect.h);
		qglEnd ( );		

		qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
		qglBegin( GL_QUADS );	
		qglVertex2f ( rect.x + (rect.w - GUIED_CENTERSIZE) / 2, rect.y + (rect.h - GUIED_CENTERSIZE) / 2 );
		qglVertex2f ( rect.x + (rect.w + GUIED_CENTERSIZE) / 2, rect.y + (rect.h - GUIED_CENTERSIZE) / 2 );
		qglVertex2f ( rect.x + (rect.w + GUIED_CENTERSIZE) / 2, rect.y + (rect.h + GUIED_CENTERSIZE) / 2 );
		qglVertex2f ( rect.x + (rect.w - GUIED_CENTERSIZE) / 2, rect.y + (rect.h + GUIED_CENTERSIZE) / 2 );
		qglEnd ( );		
	}

	if ( mExpression )
	{
		return;
	}

	qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
	
 	qglColor4f ( color[0],color[1],color[2], 1.0f );
	qglBegin(GL_QUADS);	
	
	// Top Left
	qglVertex2f ( mRect.x - GUIED_GRABSIZE, mRect.y - GUIED_GRABSIZE );
	qglVertex2f ( mRect.x - 1, mRect.y - GUIED_GRABSIZE );
	qglVertex2f ( mRect.x - 1, mRect.y - 1 );
	qglVertex2f ( mRect.x - GUIED_GRABSIZE, mRect.y - 1 );

	// Left
	qglVertex2f ( mRect.x - GUIED_GRABSIZE, mRect.y + mRect.h / 2 - GUIED_GRABSIZE / 2);
	qglVertex2f ( mRect.x - 1, mRect.y + mRect.h / 2 - GUIED_GRABSIZE / 2 );
	qglVertex2f ( mRect.x - 1, mRect.y + mRect.h / 2 + GUIED_GRABSIZE / 2 );
	qglVertex2f ( mRect.x - GUIED_GRABSIZE, mRect.y + mRect.h / 2 + GUIED_GRABSIZE / 2 );

	// Bototm Left
	qglVertex2f ( mRect.x - GUIED_GRABSIZE, mRect.y + mRect.h + 1 );
	qglVertex2f ( mRect.x - 1, mRect.y + mRect.h + 1 );
	qglVertex2f ( mRect.x - 1, mRect.y + mRect.h + GUIED_GRABSIZE );
	qglVertex2f ( mRect.x - GUIED_GRABSIZE, mRect.y + mRect.h + GUIED_GRABSIZE );

	// Bottom
	qglVertex2f ( mRect.x - GUIED_GRABSIZE / 2 + mRect.w / 2, mRect.y + mRect.h + 1 );
	qglVertex2f ( mRect.x + GUIED_GRABSIZE / 2 + mRect.w / 2, mRect.y + mRect.h + 1 );
	qglVertex2f ( mRect.x + GUIED_GRABSIZE / 2 + mRect.w / 2, mRect.y + mRect.h + GUIED_GRABSIZE );
	qglVertex2f ( mRect.x - GUIED_GRABSIZE / 2 + mRect.w / 2, mRect.y + mRect.h + GUIED_GRABSIZE );

	// Bottom Right
	qglVertex2f ( mRect.x + mRect.w + 1, mRect.y + mRect.h + 1 );
	qglVertex2f ( mRect.x + mRect.w + GUIED_GRABSIZE, mRect.y + mRect.h + 1 );
	qglVertex2f ( mRect.x + mRect.w + GUIED_GRABSIZE, mRect.y + mRect.h + GUIED_GRABSIZE );
	qglVertex2f ( mRect.x + mRect.w + 1, mRect.y + mRect.h + GUIED_GRABSIZE );

	// Right
	qglVertex2f ( mRect.x + mRect.w + 1, mRect.y + mRect.h / 2 - GUIED_GRABSIZE / 2);
	qglVertex2f ( mRect.x + mRect.w + GUIED_GRABSIZE, mRect.y + mRect.h / 2 - GUIED_GRABSIZE / 2 );
	qglVertex2f ( mRect.x + mRect.w + GUIED_GRABSIZE, mRect.y + mRect.h / 2 + GUIED_GRABSIZE / 2 );
	qglVertex2f ( mRect.x + mRect.w + 1, mRect.y + mRect.h / 2 + GUIED_GRABSIZE / 2 );

	// Top Right
	qglVertex2f ( mRect.x + mRect.w + 1, mRect.y - GUIED_GRABSIZE );
	qglVertex2f ( mRect.x + mRect.w + GUIED_GRABSIZE, mRect.y - GUIED_GRABSIZE );
	qglVertex2f ( mRect.x + mRect.w + GUIED_GRABSIZE, mRect.y - 1 );
	qglVertex2f ( mRect.x + mRect.w + 1, mRect.y - 1 );

	// Top
	qglVertex2f ( mRect.x - GUIED_GRABSIZE / 2 + mRect.w / 2, mRect.y - GUIED_GRABSIZE );
	qglVertex2f ( mRect.x + GUIED_GRABSIZE / 2 + mRect.w / 2, mRect.y - GUIED_GRABSIZE );
	qglVertex2f ( mRect.x + GUIED_GRABSIZE / 2 + mRect.w / 2, mRect.y - 1 );
	qglVertex2f ( mRect.x - GUIED_GRABSIZE / 2 + mRect.w / 2, mRect.y - 1 );

	qglEnd ( );

	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
}

/*
================
rvGESelectionMgr::UpdateRectangle

Update the selection rectangle from all the currently selected items.
================
*/
void rvGESelectionMgr::UpdateRectangle ( void )
{
	int		i;
	idVec2	point;

	assert ( mWorkspace );
	
	if ( mSelections.Num ( ) <= 0 )
	{
		return;
	}
	
	// Start with the first selections retangle
	mRect = rvGEWindowWrapper::GetWrapper( mSelections[0] )->GetScreenRect ( );
	
	// Its easier to do the calculates with it being top left and bottom right
	// so temporarly convert width and height to right and bottom
	mRect.w += mRect.x;
	mRect.h += mRect.y;
	
	// Merge all the rest of the rectangles to make the actual selection rectangle
	for ( i = 1; i < mSelections.Num(); i ++ )
	{
		idRectangle selRect;
		selRect = rvGEWindowWrapper::GetWrapper ( mSelections[i] )->GetScreenRect ( );
		
		mRect.w = max(selRect.x+selRect.w,mRect.w);
		mRect.h = max(selRect.y+selRect.h,mRect.h);		
		mRect.x = min(selRect.x,mRect.x);
		mRect.y = min(selRect.y,mRect.y);
	}

	mRect.w -= mRect.x;
	mRect.h -= mRect.y;

	mWorkspace->WorkspaceToWindow ( mRect );
}

/*
================
rvGESelectionMgr::UpdateExpression

Update whether or not the selection has an expression in it
================
*/
void rvGESelectionMgr::UpdateExpression ( void )
{
	int i;
	
	mExpression = false;
	for ( i = 0; i < mSelections.Num(); i ++ )
	{
		rvGEWindowWrapper* wrapper;
		wrapper = rvGEWindowWrapper::GetWrapper ( mSelections[i] );
		if ( wrapper && !wrapper->CanMoveAndSize ( ) )
		{
			mExpression = true;
			break;
		}
	}
}	

/*
================
rvGESelectionMgr::HitTest

Test to see if the given coordinate is within the selection rectangle and if it is
see what its over.
================
*/
rvGESelectionMgr::EHitTest rvGESelectionMgr::HitTest ( float x, float y )
{
	if ( !mSelections.Num ( ) )
	{
		return HT_NONE;
	}

	UpdateRectangle ( );

	// Inside the rectangle is moving
	if ( mRect.Contains ( x, y ) )
	{
		return mExpression?HT_SELECT:HT_MOVE;
	}
	
	if ( mExpression )
	{
		return HT_NONE;
	}

	// Check for top left sizing
	if ( idRectangle ( mRect.x - GUIED_GRABSIZE, mRect.y - GUIED_GRABSIZE, GUIED_GRABSIZE, GUIED_GRABSIZE ).Contains ( x, y ) )
	{
		return HT_SIZE_TOPLEFT;
	}

	// Check for left sizing
	if ( idRectangle ( mRect.x - GUIED_GRABSIZE, mRect.y + mRect.h / 2 - GUIED_GRABSIZE / 2, GUIED_GRABSIZE, GUIED_GRABSIZE ).Contains ( x, y ) )
	{
		return HT_SIZE_LEFT;
	}

	// Check for bottom left sizing
	if ( idRectangle ( mRect.x - GUIED_GRABSIZE, mRect.y + mRect.h, GUIED_GRABSIZE, GUIED_GRABSIZE ).Contains ( x, y ) )
	{
		return HT_SIZE_BOTTOMLEFT;
	}

	// Check for bottom sizing
	if ( idRectangle ( mRect.x + mRect.w / 2 - GUIED_GRABSIZE / 2, mRect.y + mRect.h, GUIED_GRABSIZE, GUIED_GRABSIZE ).Contains ( x, y ) )
	{
		return HT_SIZE_BOTTOM;
	}

	// Check for bottom right sizing
	if ( idRectangle ( mRect.x + mRect.w, mRect.y + mRect.h, GUIED_GRABSIZE, GUIED_GRABSIZE ).Contains ( x, y ) )
	{
		return HT_SIZE_BOTTOMRIGHT;
	}

	// Check for right sizing
	if ( idRectangle ( mRect.x + mRect.w, mRect.y + mRect.h / 2 - GUIED_GRABSIZE / 2, GUIED_GRABSIZE, GUIED_GRABSIZE ).Contains ( x, y ) )
	{
		return HT_SIZE_RIGHT;
	}

	// Check for top right sizing
	if ( idRectangle ( mRect.x + mRect.w, mRect.y - GUIED_GRABSIZE, GUIED_GRABSIZE, GUIED_GRABSIZE ).Contains ( x, y ) )
	{
		return HT_SIZE_TOPRIGHT;
	}

	// Check for top sizing
	if ( idRectangle ( mRect.x + mRect.w / 2 - GUIED_GRABSIZE / 2, mRect.y - GUIED_GRABSIZE, GUIED_GRABSIZE, GUIED_GRABSIZE ).Contains ( x, y ) )
	{
		return HT_SIZE_TOP;
	}

	return HT_NONE;
}

/*
================
rvGESelectionMgr::GetBottomMost

Returns the bottom most selected window.
================
*/
idWindow* rvGESelectionMgr::GetBottomMost ( void )
{
	idWindow*	bottom;
	int			depth;
	int			i;
	
	depth  = 9999;
	bottom = NULL;
	
	// Loop through all the selections and find the bottom most window
	for ( i = 0; i < mSelections.Num(); i ++ )
	{
		idWindow* parent;
		int		  tempDepth;
		
		// Calculate the depth of the window by iterating back through the windows parents
		for ( tempDepth = 0, parent = mSelections[i]; parent; parent = parent->GetParent ( ), tempDepth++ );
		
		// If the new depth is less than the current depth then this window is below 
		if ( tempDepth < depth )
		{
			depth  = tempDepth;
			bottom = mSelections[i];	
		}
	}
	
	return bottom;
}

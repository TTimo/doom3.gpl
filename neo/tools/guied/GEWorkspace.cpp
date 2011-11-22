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

#include "../../sys/win32/rc/guied_resource.h"
#include "../../renderer/tr_local.h"
#include "../../sys/win32/win_local.h"
#include "../../ui/DeviceContext.h"
#include "../../ui/EditWindow.h"
#include "../../ui/ListWindow.h"
#include "../../ui/BindWindow.h"
#include "../../ui/RenderWindow.h"
#include "../../ui/ChoiceWindow.h"

#include "GEApp.h"
#include "GEItemPropsDlg.h"
#include "GEItemScriptsDlg.h"

// Modifiers
#include "GEModifierGroup.h"
#include "GEMoveModifier.h"
#include "GESizeModifier.h"
#include "GEStateModifier.h"
#include "GEZOrderModifier.h"
#include "GEInsertModifier.h"
#include "GEHideModifier.h"
#include "GEDeleteModifier.h"

static float g_ZoomScales[rvGEWorkspace::ZOOM_MAX] = { 0, 0.25f, 0.33f, 0.50f, 0.66f, 1.0f, 1.5f, 2.0f, 3.0f };

static const int ID_GUIED_SELECT_FIRST = 9800;
static const int ID_GUIED_SELECT_LAST  = 9900;

idList<rvGEClipboardItem*> rvGEWorkspace::mClipboard;

rvGEWorkspace::rvGEWorkspace ( rvGEApp* app ) : mApplication ( app )
{
	mWnd	    		= 0;
	mInterface  		= 0;
	mZoom	    		= ZOOM_100;
	mScrollHorz 		= false;
	mScrollVert 		= false;
	mModified   		= false;
	mNew				= false;
	mDragScroll 		= false;
	mSourceControlState = SCS_CHECKEDOUT;
	mFilename   		= "guis/Untitled.gui";
	mDragType			= rvGESelectionMgr::HT_NONE;
	mHandCursor 		= LoadCursor ( app->GetInstance(), MAKEINTRESOURCE(IDC_GUIED_HAND) );
	mDontAdd			= false;
	
	mSelections.SetWorkspace ( this );
}

rvGEWorkspace::~rvGEWorkspace ( )
{
	// Make sure all the wrappers get cleaned up
	rvGEWindowWrapper::GetWrapper ( mInterface->GetDesktop ( ) )->EnumChildren ( CleanupEnumProc, NULL );

	DestroyCursor ( mHandCursor );

	delete mInterface;	
}

/*
================
rvGEWorkspace::CleanupEnumProc

Window enumeration procedure that deletes all the wrapper classes
================
*/
bool rvGEWorkspace::CleanupEnumProc ( rvGEWindowWrapper* wrapper, void* data )
{
	bool result;
	
	if ( !wrapper )
	{
		return true;
	}
	
	result = wrapper->EnumChildren ( CleanupEnumProc, data );
	
	// Cleanup the window wrapper
	delete wrapper;

	return result;
}

/*
================
rvGEWorkspace::GetZoomScale

Returns the scale of the current zoom level
================
*/
float rvGEWorkspace::GetZoomScale ( void )
{
	return g_ZoomScales [ mZoom ];
}

/*
================
rvGEWorkspace::Attach

Attaches the workspace to the given window.  This is usually done after the
window is created and the file has been loaded.
================
*/
bool rvGEWorkspace::Attach ( HWND wnd )
{
	assert ( wnd );
	
	mWnd = wnd;

	// Initialize the pixel format for this window
	SetupPixelFormat ( );

	// Jam the workspace pointer into the userdata window long so 
	// we can retrieve the workspace from the window later
	SetWindowLong ( mWnd, GWL_USERDATA, (LONG) this );
	
	UpdateTitle ( );
	
	return true;
}

/*
================
rvGEWorkspace::Detach

Detaches the workspace from the window it is currently attached to
================
*/
void rvGEWorkspace::Detach ( void )
{
	assert ( mWnd );
	
	SetWindowLong ( mWnd, GWL_USERDATA, 0 );
	mWnd = NULL;
}

/*
================
rvGEWorkspace::SetupPixelFormat

Setup the pixel format for the opengl context
================
*/
bool rvGEWorkspace::SetupPixelFormat ( void )
{
	HDC	 hDC    = GetDC ( mWnd );
	bool result = true;

	int pixelFormat = ChoosePixelFormat(hDC, &win32.pfd);
	if (pixelFormat > 0) 
	{
		if (SetPixelFormat(hDC, pixelFormat, &win32.pfd) == NULL) 
		{
			result = false;
		}
	}
	else 
	{
		result = false;
	}
	
	ReleaseDC ( mWnd, hDC );

	return result;
}

/*
================
rvGEWorkspace::RenderGrid

Renders the grid on top of the user interface
================
*/
void rvGEWorkspace::RenderGrid ( void )
{
	float	x;
	float	y;
	float	step;
	idVec4&	color = mApplication->GetOptions().GetGridColor ( );

	// See if the grid is off before rendering it
	if ( !mApplication->GetOptions().GetGridVisible ( ))
	{
		return;
	}

	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	qglColor4f ( color[0], color[1], color[2], 0.5f );
		
	qglBegin ( GL_LINES );
	step = mApplication->GetOptions().GetGridWidth ( ) * g_ZoomScales[mZoom];
	for ( x = mRect.x + mRect.w; x >= mRect.x ; x -= step )
	{
		qglVertex2f ( x, mRect.y );
		qglVertex2f ( x, mRect.y + mRect.h );
	}
	step = mApplication->GetOptions().GetGridHeight ( ) * g_ZoomScales[mZoom];
	for ( y = mRect.y + mRect.h; y >= mRect.y ; y -= step )
	{
		qglVertex2f ( mRect.x, y );
		qglVertex2f ( mRect.x + mRect.w, y );
	}
	qglEnd ( );

	qglDisable(GL_BLEND);
	qglColor3f ( color[0], color[1], color[2] );
		
	qglBegin ( GL_LINES );
	step = mApplication->GetOptions().GetGridWidth ( ) * g_ZoomScales[mZoom];
	for ( x = mRect.x + mRect.w; x >= mRect.x ; x -= step * 4 )
	{
		qglVertex2f ( x, mRect.y );
		qglVertex2f ( x, mRect.y + mRect.h );
	}
	step = mApplication->GetOptions().GetGridHeight ( ) * g_ZoomScales[mZoom];
	for ( y = mRect.y + mRect.h; y >= mRect.y ; y -= step * 4 )
	{
		qglVertex2f ( mRect.x, y );
		qglVertex2f ( mRect.x + mRect.w, y );
	}
	qglEnd ( );
}

/*
================
rvGEWorkspace::Render

Renders the workspace to the given DC
================
*/
void rvGEWorkspace::Render ( HDC hdc )
{
	int		front;
	int		back;
	float	scale;
	
	scale = g_ZoomScales[mZoom];

	// Switch GL contexts to our dc
	if (!qwglMakeCurrent( hdc, win32.hGLRC )) 
	{
		common->Printf("ERROR: wglMakeCurrent failed.. Error:%i\n", qglGetError());
		common->Printf("Please restart Q3Radiant if the Map view is not working\n");
	    return;
	}

	// Prepare the view and clear it
	GL_State( GLS_DEFAULT );
	qglViewport(0, 0, mWindowWidth, mWindowHeight );
	qglScissor(0, 0, mWindowWidth, mWindowHeight );
	qglClearColor ( 0.75f, 0.75f, 0.75f, 0 );

	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_CULL_FACE);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render the workspace below
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	qglOrtho(0,mWindowWidth, mWindowHeight, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	qglColor3f ( mApplication->GetOptions().GetWorkspaceColor()[0], mApplication->GetOptions().GetWorkspaceColor()[1], mApplication->GetOptions().GetWorkspaceColor()[2] );	
	qglBegin ( GL_QUADS );
	qglVertex2f ( mRect.x, mRect.y );
	qglVertex2f ( mRect.x + mRect.w, mRect.y );
	qglVertex2f ( mRect.x + mRect.w, mRect.y + mRect.h );
	qglVertex2f ( mRect.x, mRect.y + mRect.h );
	qglEnd ( );

	// Prepare the renderSystem view to draw the GUI in
	viewDef_t viewDef;
	memset ( &viewDef, 0, sizeof(viewDef) );
	tr.viewDef = &viewDef;
	tr.viewDef->renderView.x = mRect.x;
	tr.viewDef->renderView.y = mWindowHeight - mRect.y - mRect.h;
	tr.viewDef->renderView.width = mRect.w;
	tr.viewDef->renderView.height = mRect.h;
	tr.viewDef->scissor.x1 = 0;
	tr.viewDef->scissor.y1 = 0;
	tr.viewDef->scissor.x2 = mRect.w;
	tr.viewDef->scissor.y2 = mRect.h;
	tr.viewDef->isEditor = true;
	renderSystem->BeginFrame(mWindowWidth, mWindowHeight );
	
	// Draw the gui
	mInterface->Redraw ( 0 ); // eventLoop->Milliseconds() );

	// We are done using the renderSystem now
	renderSystem->EndFrame( &front, &back );

	if ( mApplication->GetActiveWorkspace ( ) == this )
	{
		mApplication->GetStatusBar().SetTriangles ( backEnd.pc.c_drawIndexes/3 );
	}

	// Prepare the viewport for drawing selections, etc.
	GL_State( GLS_DEFAULT );
	qglDisable( GL_TEXTURE_CUBE_MAP_EXT );
//	qglDisable(GL_BLEND);
	qglDisable(GL_CULL_FACE);
	
	qglViewport(0, 0, mWindowWidth, mWindowHeight );
	qglScissor(0, 0, mWindowWidth, mWindowHeight );
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	qglOrtho(0,mWindowWidth, mWindowHeight, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	RenderGrid ( );
	
	mSelections.Render ( );
	
	qglFinish ( );
	qwglSwapBuffers(hdc);

	qglEnable( GL_TEXTURE_CUBE_MAP_EXT );
	qglEnable( GL_CULL_FACE);
}

/*
================
rvGEWorkspace::UpdateTitle

Updates the window title with the name of the file and the zoom level and weither its open or not
================
*/
void rvGEWorkspace::UpdateTitle ( void )
{
	// Set the window title based on the current filename
	SetWindowText ( mWnd, va("%s%s (%d%%)", idStr(mFilename).StripPath ( ).c_str( ), mModified?"*":"", (int) (g_ZoomScales[mZoom] * 100)) );

	gApp.GetStatusBar().SetZoom ( (int)(g_ZoomScales[mZoom] * 100.0f) );
}

/*
================
rvGEWorkspace::UpdateRectangle

Updates the rectangle (not counting scrolling)
================
*/
void rvGEWorkspace::UpdateRectangle ( bool useScroll )
{
	RECT	rcClient;
	float	x;
	float	y;
	float	scale;
	
	scale = g_ZoomScales[mZoom];

	// Grab the current client rectangle of the window and cache off the width and height
	GetClientRect ( mWnd, &rcClient );
	mWindowWidth = rcClient.right - rcClient.left;
	mWindowHeight = rcClient.bottom - rcClient.top;
	
	// The workspace is always centered in the window
	x = mRect.x = mWindowWidth / 2 - (SCREEN_WIDTH * scale) / 2;
	y = mRect.y = mWindowHeight / 2 - (SCREEN_HEIGHT * scale) / 2;
	mRect.w = (SCREEN_WIDTH * scale);
	mRect.h = (SCREEN_HEIGHT * scale);

	// When using the scroll position offset the rectangle based on the scrollbar positions
	if ( useScroll )
	{
		// Adjust the start of the rectangle for the scroll positiond
		mRect.y -= (float)GetScrollPos ( mWnd, SB_VERT ) / 1000.0f;
		mRect.x -= (float)GetScrollPos ( mWnd, SB_HORZ ) / 1000.0f;
	}
}

/*
================
rvGEWorkspace::Scroll

Adjusts the given scrollbar by the given offset
================
*/
void rvGEWorkspace::Scroll ( int scrollbar, int offset )
{
	SCROLLINFO si;

	if ( scrollbar == SB_HORZ && !mScrollHorz )
	{
		return;
	}
	else if ( scrollbar == SB_VERT && !mScrollVert )
	{
		return;
	}
				
	// Get all the vertial scroll bar information
	si.cbSize = sizeof (si);
	si.fMask  = SIF_ALL;

	// Save the position for comparison later on
	GetScrollInfo ( mWnd, scrollbar, &si);

	si.nPos += (1000 * offset);
	if ( si.nPos < si.nMin ) si.nPos = si.nMin;
	if ( si.nPos > si.nMax ) si.nPos = si.nMax;
	
	si.fMask = SIF_POS;
	SetScrollInfo (mWnd, scrollbar, &si, TRUE);
	GetScrollInfo (mWnd, scrollbar, &si);

	UpdateRectangle ( );
}

int rvGEWorkspace::HandleScroll ( int scrollbar, WPARAM wParam, LPARAM lParam ) 
{
	SCROLLINFO si;
	
	// Get all the vertial scroll bar information
	si.cbSize = sizeof (si);
	si.fMask  = SIF_ALL;

	// Save the position for comparison later on
	GetScrollInfo ( mWnd, scrollbar, &si);

	switch (LOWORD (wParam))
	{
		// user clicked left or up arrow
		case SB_LINELEFT: 
			si.nPos -= 1000;
			break;
	        
		// user clicked right or down arrow
		case SB_LINERIGHT: 
			si.nPos += 1000;
			break;
		        
		// user clicked shaft left of the scroll box
		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;
		        
		// user clicked shaft right of the scroll box
		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;
		        
		// user dragged the scroll box
		case SB_THUMBTRACK: 
			si.nPos = si.nTrackPos;
			break;
		        
		default :
			break;
	}

	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.
	si.fMask = SIF_POS;
	SetScrollInfo (mWnd, scrollbar, &si, TRUE);
	GetScrollInfo (mWnd, scrollbar, &si);

	UpdateRectangle ( );
	
	return 0;
}

/*
================
rvGEWorkspace::UpdateScrollbars

Updates the states and the ranges of the scrollbars as well as the rectangle
================
*/
void rvGEWorkspace::UpdateScrollbars ( void )
{
	SCROLLINFO info;

	// First update the rectangle without applying scroll positions so
	// we know the real sizes and coordinates
	UpdateRectangle ( false );

	// Setup the veritcal scrollbar
	info.cbSize = sizeof(info);
	info.fMask = SIF_RANGE|SIF_PAGE;
	info.nMax = (mRect.h - mWindowHeight + 10) * 1000 / 2;		
	info.nMin = -info.nMax;
	info.nPage = (int)((float)info.nMax * (float)((float)mWindowHeight / mRect.h));
	info.nMax += info.nPage;

	// If there is something to scroll then turn on the vertical scroll bar
	// if its not on and update the scroll info.
	if ( info.nMax > 0 )
	{
		if ( !mScrollVert )
		{
			mScrollVert = true;
			ShowScrollBar ( mWnd, SB_VERT, mScrollVert );
		}
		SetScrollInfo ( mWnd, SB_VERT, &info, TRUE );	
	}
	// Nothing to scroll, turn off the scrollbar if its on.
	else if ( mScrollVert )
	{			
		mScrollVert = false;
		SetScrollPos ( mWnd, SB_VERT, 0, FALSE );
		ShowScrollBar ( mWnd, SB_VERT, mScrollVert );
	}

	// Setup the horizontal scrollbar
	info.nMax = (mRect.w - mWindowWidth + 10) * 1000 / 2;		
	info.nMin = -info.nMax;
	info.nPage = (int)((float)info.nMax * (float)((float)mWindowWidth / mRect.w));
	info.nMax += info.nPage;

	// If there is something to scroll then turn on the vertical scroll bar
	// if its not on and update the scroll info.
	if ( info.nMax > 0 )
	{
		if ( !mScrollHorz )
		{
			mScrollHorz = true;
			ShowScrollBar ( mWnd, SB_HORZ, mScrollHorz );
		}
		
		SetScrollInfo ( mWnd, SB_HORZ, &info, TRUE );
	}
	// Nothing to scroll, turn off the scrollbar if its on.
	else if ( mScrollHorz )
	{			
		mScrollHorz = false;
		SetScrollPos ( mWnd, SB_HORZ, 0, FALSE );
		ShowScrollBar ( mWnd, SB_HORZ, mScrollHorz );
	}

	// Need to update the rectangle again to take the scrollbar changes into account	
	UpdateRectangle ( true );
}

/*
================
rvGEWorkspace::UpdateCursor

Called to update the cursor when the mouse is within the workspace window
================
*/
void rvGEWorkspace::UpdateCursor ( rvGESelectionMgr::EHitTest type )
{
	switch ( type )
	{
		case rvGESelectionMgr::HT_SELECT:
			SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_ARROW) ) );
			break;
			
		case rvGESelectionMgr::HT_MOVE:
			SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_SIZEALL) ) );
			break;

		case rvGESelectionMgr::HT_SIZE_LEFT:
		case rvGESelectionMgr::HT_SIZE_RIGHT:
			SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_SIZEWE ) ) );
			break;

		case rvGESelectionMgr::HT_SIZE_TOP:
		case rvGESelectionMgr::HT_SIZE_BOTTOM:
			SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_SIZENS ) ) );
			break;

		case rvGESelectionMgr::HT_SIZE_TOPRIGHT:
		case rvGESelectionMgr::HT_SIZE_BOTTOMLEFT:
			SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_SIZENESW ) ) );
			break;

		case rvGESelectionMgr::HT_SIZE_BOTTOMRIGHT:
		case rvGESelectionMgr::HT_SIZE_TOPLEFT:
			SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_SIZENWSE ) ) );
			break;
	}
}	

void rvGEWorkspace::UpdateCursor ( float x, float y )
{
	idVec2						point;
	rvGESelectionMgr::EHitTest	type;
		
	// First convert the worspace coord to a window coord
	point = WorkspaceToWindow ( idVec2( x, y ) );

	// See if it hits anything	
	type = mSelections.HitTest ( point.x, point.y );
	
	// If it hits something then use it to update the cursor
	if ( rvGESelectionMgr::HT_NONE != type )
	{
		UpdateCursor ( type );
	}
	else
	{
		SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_ARROW ) ) );
	}
}

void rvGEWorkspace::UpdateCursor ( void )
{
	if ( mDragType == rvGESelectionMgr::HT_NONE )
	{
		POINT	point;
		idVec2	cursor;

		GetCursorPos ( &point );
		cursor.Set ( point.x, point.y );
		WindowToWorkspace ( cursor );

		UpdateCursor ( cursor.x, cursor.y );
	}
	else
	{
		UpdateCursor ( mDragType );
	}
}

/*
================
rvGEWorkspace::HandleMessage

Handles window messages to the workspace
================
*/
void rvGEWorkspace::HandleMessage ( UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_CLOSE:
		{
			
			if ( IsModified ( ) )
			{
				if ( IDYES == gApp.MessageBox ( va("Save changes to the document \"%s\" before closing?", GetFilename() ), MB_YESNO|MB_ICONQUESTION ) )
				{
					SendMessage ( mApplication->GetMDIFrame(), WM_COMMAND, MAKELONG(ID_GUIED_FILE_SAVE,0), 0 );
				}
			}
			

			GetApplication ( )->GetNavigator().SetWorkspace(NULL);			
			GetApplication ( )->GetTransformer().SetWorkspace(NULL);
			GetApplication ( )->GetProperties().SetWorkspace(NULL);
			break;
		}
			
		case WM_CAPTURECHANGED:
			if ( (HWND)lParam != mWnd )
			{
				mDragScroll = false;
				mDragType	= rvGESelectionMgr::HT_NONE;
			}
			break;

		case WM_SETCURSOR:
		{
			POINT point;
			idVec2 cursor;
			GetCursorPos ( &point );
			cursor.Set ( point.x, point.y );
			WindowToWorkspace ( cursor );
			if ( mDragType == rvGESelectionMgr::HT_NONE )
			{
				UpdateCursor ( cursor.x, cursor.y );
			}
			else
			{
				UpdateCursor ( mDragType );
			}
			break;
		}
	
		case WM_MOUSEWHEEL:
			if ( (short)HIWORD(wParam) > 0 )
			{
				ZoomIn ( );
			}
			else if ( (short)HIWORD(wParam) < 0 )
			{
				ZoomOut ( );
			}
			break;
		
		case WM_MOUSEMOVE:
			HandleMouseMove ( wParam, lParam );
			break;
	
		case WM_MBUTTONDOWN:
			HandleMButtonDown ( wParam, lParam );
			break;
			
		case WM_MBUTTONUP:
			HandleMButtonUp ( wParam, lParam );
			break;
	
		case WM_LBUTTONDOWN:
			HandleLButtonDown ( wParam, lParam );
			break;
			
		case WM_LBUTTONUP:
			HandleLButtonUp ( wParam, lParam );
			break;
			
		case WM_LBUTTONDBLCLK:
			HandleLButtonDblClk ( wParam, lParam );
			break;

		case WM_INITMENUPOPUP:			
			SendMessage ( mApplication->GetMDIFrame(), msg, wParam, lParam );
			break;
			
		case WM_COMMAND:
			HandleCommand ( wParam, lParam );
			break;
			
		case WM_RBUTTONDOWN:
			HandleRButtonDown ( wParam, lParam );
			break;
			
		case WM_SIZE:
			UpdateScrollbars();
			break;

		case WM_VSCROLL:
			HandleScroll ( SB_VERT, wParam, lParam );
			break;

		case WM_HSCROLL:
			HandleScroll ( SB_HORZ, wParam, lParam );
			break;
			
		case WM_KEYDOWN:
			HandleKeyDown ( wParam, lParam );
			break;			
	}
}
/*
================
rvGEWorkspace::HandleCommand

Handles command messages destined for the workspace window.  This is for
special workspace commands, any unhandled commands are forwarded to the main window
================
*/
int	rvGEWorkspace::HandleCommand ( WPARAM wParam, LPARAM lParam )
{
	// Select command
	if ( LOWORD(wParam) >= ID_GUIED_SELECT_FIRST && LOWORD(wParam) <= ID_GUIED_SELECT_LAST )
	{
		idWindow*			window  = mSelectMenu[LOWORD(wParam)-ID_GUIED_SELECT_FIRST];
		rvGEWindowWrapper*	wrapper = rvGEWindowWrapper::GetWrapper ( window );
		
		// Handle multi select as well
		if ( GetAsyncKeyState ( VK_SHIFT ) & 0x8000 )
		{
			if ( wrapper->IsSelected ( ) )
			{
				mSelections.Remove ( window );
			}
			else
			{
				mSelections.Add ( window );
			}
		}
		else
		{			
			mSelections.Set ( window );
		}
	}

	return SendMessage ( mApplication->GetMDIFrame(), WM_COMMAND, wParam, lParam );
}

/*
================
rvGEWorkspace::HandleMButtonDown

Handles the middle mouse down message in the workspace
================
*/
int	rvGEWorkspace::HandleMButtonDown ( WPARAM wParam, LPARAM lParam )
{
	if ( mDragType != rvGESelectionMgr::HT_NONE )
	{
		return 0;
	}
	
	mDragPoint.Set ( LOWORD(lParam), HIWORD(lParam) );
	mDragScroll = true;
	SetCursor ( mHandCursor );
	SetCapture ( mWnd );

	WindowToWorkspace ( mDragPoint );

	return 0;
}

/*
================
rvGEWorkspace::HandleMButtonUp

Handles the middle mouse up message in the workspace
================
*/
int	rvGEWorkspace::HandleMButtonUp ( WPARAM wParam, LPARAM lParam )
{
	if ( mDragScroll )
	{
		mDragScroll = false;
		ReleaseCapture ( );
	}

	return 0;
}

/*
================
rvGEWorkspace::HandleRButtonDown

Handles the left mouse down message in the workspace
================
*/
int	rvGEWorkspace::HandleRButtonDown ( WPARAM wParam, LPARAM lParam )
{
	POINT point = { LOWORD(lParam), HIWORD(lParam) };	
	HMENU menu;

	// Add the select menu
	mSelectMenu.Clear ( );
	
	// Cache where the menu is being brought up so we can 
	// figure out which windows are under the point
	mSelectMenuPos[0] = point.x;
	mSelectMenuPos[1] = point.y;
	WindowToWorkspace ( mSelectMenuPos );	
	
	// Build a list of all the windows under the menu point
	rvGEWindowWrapper::GetWrapper ( mInterface->GetDesktop() )->EnumChildren ( BuildSelectMenuEnumProc, this );
 
	// Add the desktop window always
	mSelectMenu.Append ( mInterface->GetDesktop() );

	// 
	menu = GetSubMenu ( LoadMenu ( mApplication->GetInstance(), MAKEINTRESOURCE(IDR_GUIED_ITEM_POPUP) ), 0 );
		
	HMENU popup = CreatePopupMenu ( );	

	int i;
	for ( i = 0; i < mSelectMenu.Num(); i ++ )
	{
		rvGEWindowWrapper* wrapper = rvGEWindowWrapper::GetWrapper ( mSelectMenu[i] );
		AppendMenu ( popup, MF_STRING|MF_ENABLED|(wrapper->IsSelected()?MF_CHECKED:0), ID_GUIED_SELECT_FIRST + i, mSelectMenu[i]->GetName() );
	}

	InsertMenu ( menu, 1, MF_POPUP|MF_BYPOSITION, (LONG) popup, "Select" );

	// Bring up the popup menu
	ClientToScreen ( mWnd, &point );
	TrackPopupMenu ( menu, TPM_RIGHTBUTTON|TPM_LEFTALIGN, point.x, point.y, 0, mWnd, NULL );

	DestroyMenu ( popup );
	DestroyMenu ( menu );

	return 0;
}

/*
================
rvGEWorkspace::HandleLButtonDown

Handles the left mouse down message in the workspace
================
*/
int	rvGEWorkspace::HandleLButtonDown ( WPARAM wParam, LPARAM lParam )
{
	if ( mDragScroll )
	{
		return 0;
	}

	idVec2 point ( LOWORD(lParam), HIWORD(lParam) );	
	WindowToWorkspace ( point );

	// Make sure whatever modifications get generated cant be merged into whats already there
	mModifiers.BlockNextMerge ( );

	mDragPoint.Set ( LOWORD(lParam), HIWORD(lParam) );
	mDragTime = Sys_Milliseconds ( );
	mDragX    = true;
	mDragY    = true;

	// If we have selections then start a drag
	if ( mSelections.Num ( ) )
	{
		mDragType = mSelections.HitTest ( mDragPoint.x, mDragPoint.y );
	}

	rvGEWindowWrapper* wrapper;
	wrapper = rvGEWindowWrapper::GetWrapper ( mInterface->GetDesktop ( ) );
	
	idWindow* window = wrapper->WindowFromPoint ( point.x, point.y );

	// dissallow selection of the desktop.
	if ( gApp.GetOptions().GetIgnoreDesktopSelect() && window == mInterface->GetDesktop ( ) )
	{
		window = NULL;
	}

	if ( mDragType == rvGESelectionMgr::HT_MOVE || mDragType == rvGESelectionMgr::HT_NONE )
	{
		if ( window  )
		{
			bool selected;

			selected = mSelections.IsSelected ( window );
		
			if ( GetAsyncKeyState ( VK_SHIFT ) & 0x8000 )
			{
				if ( !selected )
				{
					mSelections.Add ( window );
					mDragType = rvGESelectionMgr::HT_MOVE;
				}
				else
				{
					mSelections.Remove ( window );
				}
			}
			else if ( !selected && mDragType == rvGESelectionMgr::HT_NONE  )
			{
				mSelections.Set ( window );
				mDragType = rvGESelectionMgr::HT_MOVE;
			}
		}
		else
		{
			mSelections.Clear ( );
		}
	}

	if ( mSelections.IsExpression ( ) )
	{
		mDragType = rvGESelectionMgr::HT_SELECT;
	}
	// Windows capture
	else if ( mDragType != rvGESelectionMgr::HT_NONE )
	{
		SetCapture ( mWnd );
	}

	WindowToWorkspace ( mDragPoint );

	return 0;
}

/*
================
rvGEWorkspace::HandleLButtonUp

Handles the left mouse up message in the workspace
================
*/
int	rvGEWorkspace::HandleLButtonUp ( WPARAM wParam, LPARAM lParam )
{
	if ( mDragType != rvGESelectionMgr::HT_NONE )
	{
		ReleaseCapture ( );
		mModifiers.BlockNextMerge ( );

		// Update the transformer
		mApplication->GetTransformer().Update ( );
	}
		
	// No more dragging
	mDragType = rvGESelectionMgr::HT_NONE;
		
	return 0;
}

/*
================
rvGEWorkspace::HandleLButtonDblClk

Handle a double click by opening properties
================
*/
int	rvGEWorkspace::HandleLButtonDblClk ( WPARAM wParam, LPARAM lParam )
{
	EditSelectedProperties ( );
	return 0;
}

/*
================
rvGEWorkspace::HandleMouseMove

Handles the moving of the mouse for dragging and cursor updating
================
*/
int	rvGEWorkspace::HandleMouseMove ( WPARAM wParam, LPARAM lParam )
{
	idVec2	cursor;
	
	cursor.Set ( (short)LOWORD(lParam), (short)HIWORD(lParam) );

	// Convert the window point to the workspace before updating the 
	// cursor with the position
	WindowToWorkspace ( cursor );

	// Scrolling the window around
	if ( mDragScroll )
	{
		Scroll ( SB_HORZ, mDragPoint.x - cursor.x );
		Scroll ( SB_VERT, mDragPoint.y - cursor.y );
		
		SetCursor ( mHandCursor );
		
		mDragPoint = cursor;
		
		return 0;
	}			

	// If not dragging then just update the cursor and return
	if ( mDragType == rvGESelectionMgr::HT_NONE )
	{
		UpdateCursor ( cursor.x, cursor.y );
		return 0;
	}

	// Dont allow a drag move start until the button has been down for 100 ms or so
	if ( mDragType == rvGESelectionMgr::HT_MOVE && Sys_Milliseconds() - mDragTime <= 50 )
	{
		return 0;
	}

	// Handle grid snapping
	if ( gApp.GetOptions().GetGridSnap ( ) )
	{
		cursor.x = (float)(((int)cursor.x + gApp.GetOptions().GetGridWidth()/2) / gApp.GetOptions().GetGridWidth() * gApp.GetOptions().GetGridWidth());
		cursor.y = (float)(((int)cursor.y + gApp.GetOptions().GetGridWidth()/2) / gApp.GetOptions().GetGridWidth() * gApp.GetOptions().GetGridWidth());
	}
	
	// If the cursor hasnt moved then there is nothing to update with the drag
	if ( (int) cursor.x == (int) mDragPoint.x && (int) cursor.y == (int) mDragPoint.y )
	{
		return 0;
	}
	
	switch ( mDragType )
	{		
		case rvGESelectionMgr::HT_MOVE:
			AddModifierMove ( "Move", cursor.x - mDragPoint.x, cursor.y - mDragPoint.y, mApplication->GetOptions().GetGridSnap ( ) );
			break;
			
		case rvGESelectionMgr::HT_SIZE_BOTTOM:
			AddModifierSize ( "Size", 0, 0, 0, cursor.y - mDragPoint.y, mApplication->GetOptions().GetGridSnap ( ) );
			break;

		case rvGESelectionMgr::HT_SIZE_TOP:
			AddModifierSize ( "Size", 0, cursor.y - mDragPoint.y, 0, 0, mApplication->GetOptions().GetGridSnap ( ) );
			break;

		case rvGESelectionMgr::HT_SIZE_RIGHT:
			AddModifierSize ( "Size", 0, 0, cursor.x - mDragPoint.x, 0, mApplication->GetOptions().GetGridSnap ( ) );
			break;

		case rvGESelectionMgr::HT_SIZE_LEFT:
			AddModifierSize ( "Size", cursor.x - mDragPoint.x, 0, 0, 0, mApplication->GetOptions().GetGridSnap ( ) );
			break;

		case rvGESelectionMgr::HT_SIZE_TOPLEFT:
			AddModifierSize ( "Size", cursor.x - mDragPoint.x, cursor.y - mDragPoint.y, 0, 0, mApplication->GetOptions().GetGridSnap ( ) );
			break;

		case rvGESelectionMgr::HT_SIZE_TOPRIGHT:
			AddModifierSize ( "Size", 0, cursor.y - mDragPoint.y, cursor.x - mDragPoint.x, 0, mApplication->GetOptions().GetGridSnap ( ) );
			break;

		case rvGESelectionMgr::HT_SIZE_BOTTOMLEFT:
			AddModifierSize ( "Size", cursor.x - mDragPoint.x, 0, 0, cursor.y - mDragPoint.y, mApplication->GetOptions().GetGridSnap ( ) );
			break;

		case rvGESelectionMgr::HT_SIZE_BOTTOMRIGHT:
			AddModifierSize ( "Size", 0, 0, cursor.x - mDragPoint.x, cursor.y - mDragPoint.y, mApplication->GetOptions().GetGridSnap ( ) );
			break;
	}

	UpdateCursor ( mDragType );

	// If the x coordinate has changed then update it
	if ( (int)cursor.x != (int)mDragPoint.x && mDragX )
	{
		mDragPoint.x = cursor.x;
	}

	// If the y coordinate has changed then update it
	if ( (int)cursor.y != (int)mDragPoint.y && mDragY )
	{
		mDragPoint.y = cursor.y;
	}
	
	return 0;
}

/*
================
rvGEWorkspace::HandleKeyDown

Handles the the pressing of a key
================
*/
int	rvGEWorkspace::HandleKeyDown ( WPARAM wParam, LPARAM lParam )
{
	bool shift = (GetAsyncKeyState ( VK_SHIFT ) & 0x8000) ? true : false;

	switch ( wParam )
	{
		case VK_LEFT:
			if ( shift )
			{
				AddModifierSizeNudge ( -1, 0, false );
			}
			else
			{
				AddModifierMoveNudge ( -1, 0, false );
			}
			break;					

		case VK_RIGHT:
			if ( shift )
			{
				AddModifierSizeNudge ( 1, 0, false ); 
			}
			else
			{
				AddModifierMoveNudge ( 1, 0, false ); 
			}
			break;					

		case VK_DOWN:
			if ( shift )
			{
				AddModifierSizeNudge ( 0, 1, false );
			}
			else
			{
				AddModifierMoveNudge ( 0, 1, false );
			}
			break;					

		case VK_UP:
			if ( shift )
			{
				AddModifierSizeNudge ( 0, -1, false ); 
			}
			else
			{
				AddModifierMoveNudge ( 0, -1, false ); 
			}
			break;		
			
		case VK_ESCAPE:
			mSelections.Clear ( );
			mApplication->GetNavigator().Update ( );
			break;			
	}
	
	return 0;
}

/*
================
rvGEWorkspace::WindowToWorkspace

Converts the given coordinates in windows space to the workspace's coordinates.
================
*/
idVec2& rvGEWorkspace::WindowToWorkspace ( idVec2& point )
{
	point.x = (point.x - mRect.x) / mRect.w * SCREEN_WIDTH;
	point.y = (point.y - mRect.y) / mRect.h * SCREEN_HEIGHT;

	return point;
}

idRectangle& rvGEWorkspace::WindowToWorkspace ( idRectangle& rect )
{
	rect.x = (rect.x - mRect.x) / mRect.w * SCREEN_WIDTH;
	rect.y = (rect.y - mRect.y) / mRect.h * SCREEN_HEIGHT;
	rect.w = rect.w / mRect.w * SCREEN_WIDTH;
	rect.h = rect.h / mRect.h * SCREEN_HEIGHT;

	return rect;
}

/*
================
rvGEWorkspace::WindowToWorkspace

Converts the given workspace coordinates to the windows coordinates.
================
*/
idVec2& rvGEWorkspace::WorkspaceToWindow ( idVec2& point )
{
	point.x = mRect.x + (point.x / SCREEN_WIDTH * mRect.w);
	point.y = mRect.y + (point.y / SCREEN_HEIGHT * mRect.h);

	return point;
}

idRectangle& rvGEWorkspace::WorkspaceToWindow ( idRectangle& rect )
{
	rect.x = mRect.x + (rect.x / SCREEN_WIDTH * mRect.w);
	rect.y = mRect.y + (rect.y / SCREEN_HEIGHT * mRect.h);
	rect.w = rect.w / SCREEN_WIDTH * mRect.w;
	rect.h = rect.h / SCREEN_HEIGHT * mRect.h;

	return rect;
}

/*
================
rvGEWorkspace::ZoomIn

Zooms the workspace in by one zoom level
================
*/
rvGEWorkspace::EZoomLevel rvGEWorkspace::ZoomIn ( void )
{
	mZoom = mZoom + 1;
	if ( mZoom >= ZOOM_MAX )
	{
		mZoom = ZOOM_MAX - 1;
	}
	
	UpdateScrollbars ( );
	UpdateTitle ( );

	InvalidateRect ( mWnd, NULL, FALSE );
	
	return (EZoomLevel)mZoom;
}

/*
================
rvGEWorkspace::ZoomOut

Zooms the workspace out by one level
================
*/
rvGEWorkspace::EZoomLevel rvGEWorkspace::ZoomOut ( void )
{
	mZoom--;
	if ( mZoom <= ZOOM_MIN )
	{
		mZoom = ZOOM_MIN + 1;
	}

	UpdateScrollbars ( );
	UpdateTitle ( );

	InvalidateRect ( mWnd, NULL, FALSE );
	
	return (EZoomLevel)mZoom;
}

/*
================
rvGEWorkspace::CreateModifier

Creates a new modifier of the given type for the given window.  This function is called 
specifically from the add modifiers function with the variable args list forwarded.
================
*/
rvGEModifier* rvGEWorkspace::CreateModifier ( EModifierType type, idWindow* window, va_list args )
{
	rvGEModifier* mod;	

	switch ( type )
	{
		case MOD_DELETE:
			mod = new rvGEDeleteModifier ( "Delete", window );
			break;
		
		case MOD_HIDE:
			mod = new rvGEHideModifier ( "Hide", window, true );
			break;
			
		case MOD_UNHIDE:
			mod = new rvGEHideModifier ( "Hide", window, false );
			break;			

		case MOD_SEND_BACKWARD:
			mod = new rvGEZOrderModifier ( "Send Backward", window, rvGEZOrderModifier::ZO_BACKWARD );
			break;
			
		case MOD_SEND_BACK:
			mod = new rvGEZOrderModifier ( "Send to Back", window, rvGEZOrderModifier::ZO_BACK );
			break;

		case MOD_BRING_FORWARD:
			mod = new rvGEZOrderModifier ( "Bring Forward", window, rvGEZOrderModifier::ZO_FORWARD );
			break;

		case MOD_BRING_FRONT:
			mod = new rvGEZOrderModifier ( "Bring to Front", window, rvGEZOrderModifier::ZO_FRONT );
			break;
						
		default:
			mod = NULL;
			break;
	}
		
	return mod;
}

/*
================
rvGEWorkspace::AddModifiers

Add the specific modifier for the given window
================
*/
void rvGEWorkspace::AddModifiers ( idWindow* window, EModifierType type, ... )
{
	va_list args;
	
	va_start(args,type) ;
	mModifiers.Append ( CreateModifier ( type, window, args ) );
	va_end (args) ;

	SetModified ( true );
}

void rvGEWorkspace::AddModifiers ( EModifierType type, ... )
{
	va_list args;
		
	// Nothing to move if there is no selection
	if ( !mSelections.Num ( ) )
	{
		return;
	}
	// More than one selection requires a modifier group
	else if ( mSelections.Num ( ) > 1 )
	{
		rvGEModifierGroup*	group = new rvGEModifierGroup;
		int					i;
		
		for ( i = 0; i < mSelections.Num(); i ++ )
		{
			va_start(args,type);
			group->Append ( CreateModifier ( type, mSelections[i], args ) );
			va_end (args);
		}

		mModifiers.Append ( group );
	}
	// Single modifier
	else
	{
		va_start(args,type) ;
		mModifiers.Append ( CreateModifier ( type, mSelections[0], args ) );
		va_end (args) ;
	}		

	SetModified ( true );
}

bool rvGEWorkspace::BuildSelectMenuEnumProc ( rvGEWindowWrapper* wrapper, void* data )
{
	rvGEWorkspace*	workspace;	
	
	workspace = (rvGEWorkspace*) data;
	assert ( workspace );

	if ( !wrapper )
	{
		return true;
	}
		
	wrapper->EnumChildren ( BuildSelectMenuEnumProc, data );
	
	if ( wrapper->IsDeleted ( ) || wrapper->IsHidden ( ) )
	{	
		return true;
	}
	
	if ( wrapper->GetScreenRect ( ).Contains ( workspace->mSelectMenuPos[0], workspace->mSelectMenuPos[1] ) )
	{
		workspace->mSelectMenu.Append ( wrapper->GetWindow ( ));
	}
	
	return true;
}

bool rvGEWorkspace::ShowAllEnumProc ( rvGEWindowWrapper* wrapper, void* data )
{
	rvGEModifierGroup* group = (rvGEModifierGroup*) data;
	
	wrapper->EnumChildren ( ShowAllEnumProc, data );
	
	if ( wrapper->IsHidden ( ) )
	{
		group->Append ( new rvGEHideModifier ( "Show Hidden", wrapper->GetWindow ( ), false ) );
	}
	
	return true;
}

void rvGEWorkspace::AddModifierShowAll ( void )
{
	rvGEModifierGroup* group = new rvGEModifierGroup;
	
	rvGEWindowWrapper::GetWrapper( mInterface->GetDesktop ( ) )->EnumChildren ( ShowAllEnumProc, group );
	
	if ( !group->GetCount ( ) )
	{
		delete group;
	}
	else
	{
		mModifiers.Append ( group );
	}

	mApplication->GetNavigator().Refresh ( );
}

void rvGEWorkspace::DeleteSelected ( void )
{
	AddModifiers ( MOD_DELETE );
	mSelections.Clear ( );
	mApplication->GetNavigator().Update ( );
}

/*
================
rvGEWorkspace::NewWindow

Create a new window 
================
*/
idWindow* rvGEWorkspace::NewWindow ( idDict* state, rvGEWindowWrapper::EWindowType type )
{
	idWindow*			window = new idWindow ( mInterface->GetDesktop()->GetDC(), mInterface );
	rvGEWindowWrapper*	wrapper;
	int					count;
	idStr				baseName;

	switch ( type )
	{
		case rvGEWindowWrapper::WT_NORMAL:
			window = new idWindow ( mInterface->GetDesktop()->GetDC(), mInterface );
			break;
			
		case rvGEWindowWrapper::WT_BIND:
			window = new idBindWindow ( mInterface->GetDesktop()->GetDC(), mInterface );
			break;

		case rvGEWindowWrapper::WT_RENDER:
			window = new idRenderWindow ( mInterface->GetDesktop()->GetDC(), mInterface );
			break;
			
		case rvGEWindowWrapper::WT_CHOICE:
			window = new idChoiceWindow ( mInterface->GetDesktop()->GetDC(), mInterface );
			break;

		case rvGEWindowWrapper::WT_EDIT:
			window = new idEditWindow ( mInterface->GetDesktop()->GetDC(), mInterface );
			break;
			
		default:
			assert ( false );
			return NULL;
	}

	baseName = state ? state->GetString("name","unnamed") : "unnamed";
	baseName.StripQuotes ( );

	count = 0;
	if ( mInterface->GetDesktop()->FindChildByName ( baseName ) ) 
	{
		count = 1;
		while ( 1 )
		{
			drawWin_t* dw = mInterface->GetDesktop()->FindChildByName ( va("%s%d",baseName.c_str(),count) );
			if ( !dw )
			{
				break;
			}
			assert ( dw->win );
			wrapper = rvGEWindowWrapper::GetWrapper ( dw->win );
			if ( wrapper && wrapper->IsDeleted ( ) )
			{
				break;
			}
			count++;
		}
	}

	idStr winName;
	idStr winTemplate;
	
	if ( count )
	{
		winName = va("%s%d", baseName.c_str(), count );
	}
	else
	{
		winName = baseName;
	}
	winTemplate = winName + " { }";
	
	idParser src( winTemplate, winTemplate.Length(), "", LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWBACKSLASHSTRINGCONCAT );
	window->Parse ( &src );

	wrapper = rvGEWindowWrapper::GetWrapper ( window );
	
	if ( state )
	{
		wrapper->SetState ( *state );
	}
	
	wrapper->SetStateKey ( "name", winName );
	wrapper->Finish ( );

	SetModified ( true );
	
	
	return window;
}

idWindow* rvGEWorkspace::AddWindow ( rvGEWindowWrapper::EWindowType type )
{
	idWindow*	window;	
	idDict		state;
	
	state.Set ( "rect", "0,0,100,100" );
	state.Set ( "visible", "1" );
	
	window = NewWindow ( &state, type );
	assert ( window );

	mModifiers.Append ( new rvGEInsertModifier ( "New", window, mInterface->GetDesktop(), NULL ) );
	
	mSelections.Set ( window );
	mApplication->GetNavigator().Update ( );
	mApplication->GetTransformer().Update ( );
	mApplication->GetProperties().Update ( );

	return window;
}

bool rvGEWorkspace::EditSelectedProperties ( void )
{
	if ( !mSelections.Num ( ) || mSelections.Num() > 1 )
	{
		return false;
	}
	
	idDict dict;
	if ( GEItemPropsDlg_DoModal ( mWnd, mSelections[0], dict ) )
	{
		mModifiers.Append ( new rvGEStateModifier ( "Item Properties", mSelections[0], dict ) );
		SetModified ( true );
	}	
	
	mApplication->GetNavigator().Update ( );		
	mApplication->GetTransformer().Update ( );
	mApplication->GetProperties().Update ( );
		
	return true;
}

bool rvGEWorkspace::EditSelectedScripts ( void )
{
	if ( GEItemScriptsDlg_DoModal ( mWnd, mSelections[0] ) )
	{
		gApp.GetNavigator().Refresh ( );
		SetModified ( true );
	}
	
	return true;
}	

void rvGEWorkspace::BringSelectedForward ( void )
{
	AddModifiers ( MOD_BRING_FORWARD );
	mApplication->GetNavigator().Update ( );		
}

void rvGEWorkspace::BringSelectedToFront ( void )
{
	AddModifiers ( MOD_BRING_FRONT );
	mApplication->GetNavigator().Update ( );		
}

void rvGEWorkspace::SendSelectedToBack ( void )
{
	AddModifiers ( MOD_SEND_BACK );
	mApplication->GetNavigator().Update ( );	
}

void rvGEWorkspace::SendSelectedBackward ( void )
{
	AddModifiers ( MOD_SEND_BACKWARD );
	mApplication->GetNavigator().Update ( );	
}

/*
================
rvGEWorkspace::MakeSelectedSameSize

Align the selected items to the first one using the given align type
================
*/
void rvGEWorkspace::MakeSelectedSameSize ( bool changeWidth, bool changeHeight )
{
	rvGEModifierGroup*	group;
	idRectangle			rectTo;
	int					i;

	group = new rvGEModifierGroup ( );

	rectTo = rvGEWindowWrapper::GetWrapper ( mSelections[0] )->GetClientRect ( );

	for ( i = 1; i < mSelections.Num(); i ++ )
	{	
		idRectangle	rectFrom;
		float		width = 0;
		float		height = 0;

 		rectFrom = rvGEWindowWrapper::GetWrapper(mSelections[i])->GetClientRect ();
		
		if ( changeWidth )
		{
			width = rectTo.w - rectFrom.w;
		}
		
		if ( changeHeight )
		{
			height = rectTo.h - rectFrom.h;
		}
		
		group->Append ( new rvGESizeModifier ( "Make Same Size", mSelections[i], 0, 0, width, height ) );		
	}

	mModifiers.Append ( group );
	
	// Cant merge alignments
	mModifiers.BlockNextMerge ( );

	SetModified ( true );
}

/*
================
rvGEWorkspace::AlignSelected

Align the selected items to the first one using the given align type
================
*/
void rvGEWorkspace::AlignSelected ( EItemAlign align )
{
	static const char*	alignNames[]={"Lefts","Centers","Rights","Tops","Middles","Bottoms" };
	int					i;
	idStr				modName;
	rvGEModifierGroup*	group;
	
	assert ( mSelections.Num() > 1 );

	modName = "Align " + idStr(alignNames[align]);
	
	group   = new rvGEModifierGroup ( );

	idRectangle rectTo;
	rectTo = rvGEWindowWrapper::GetWrapper ( mSelections[0] )->GetScreenRect ( );
	
	// Everything gets aligned to the first selection so run
	// through all other selections and move them.
	for ( i = 1; i < mSelections.Num(); i ++ )
	{
		float		x;
		float		y;
		idRectangle	rectFrom;

		rectFrom = rvGEWindowWrapper::GetWrapper ( mSelections[i] )->GetScreenRect ( );
		
		switch ( align )
		{
			case ALIGN_LEFTS:
				x = rectTo[0] - rectFrom[0];
				y = 0;
				break;
				
			case ALIGN_RIGHTS:
				x = (rectTo[0]+rectTo[2]) - (rectFrom[0]+rectFrom[2]);
				y = 0;
				break;
				
			case ALIGN_CENTERS:
				x = (rectTo[0]+rectTo[2]/2) - (rectFrom[0]+rectFrom[2]/2);
				y = 0;
				break;
			
			case ALIGN_TOPS:
				y = rectTo[1] - rectFrom[1];
				x = 0;
				break;
				
			case ALIGN_BOTTOMS:
				x = 0;
				y = (rectTo[1]+rectTo[3]) - (rectFrom[1]+rectFrom[3]);
				break;
			
			case ALIGN_MIDDLES:
				x = 0;
				y = (rectTo[1]+rectTo[3]/2) - (rectFrom[1]+rectFrom[3]/2);
				break;
				
			default:
				assert ( false );
				break;
		}
	
		group->Append ( new rvGEMoveModifier ( modName, mSelections[i], x, y ) );
	}
	
	mModifiers.Append ( group );
	
	// Cant merge alignments
	mModifiers.BlockNextMerge ( );

	SetModified ( true );
}

/*
================
rvGEWorkspace::AddModifierMove

Adds a move modifier with the given offsets
================
*/
void rvGEWorkspace::AddModifierMove ( const char* modName, float x, float y, bool snap )
{
	idRectangle scaleRect;
	idRectangle newRect;

	scaleRect = mSelections.GetRect ( );
	WindowToWorkspace ( scaleRect );	
	newRect   = scaleRect;
	newRect.x += x;
	newRect.y += y;

	if ( snap )
	{
		gApp.GetOptions ().SnapRectToGrid ( newRect, true, true, false, false );
	}

	rvGEModifierGroup*	group = new rvGEModifierGroup;
	for ( int i = 0; i < mSelections.Num(); i ++ )
	{
		if ( !mSelections[i]->GetParent ( ) )
		{
			continue;
		}
		
		// IF the parent window is being moved around as well then dont move this one.
		if ( rvGEWindowWrapper::GetWrapper ( mSelections[i]->GetParent ( ) )->IsSelected ( ) )
		{
			// We still need the modifier there so the selection can be restored and
			// so the rectangle gets updated
			group->Append ( new rvGEMoveModifier ( modName, mSelections[i], 0, 0 ) );
			continue;
		}
		
		group->Append ( new rvGEMoveModifier ( modName, mSelections[i], newRect.x-scaleRect.x, newRect.y-scaleRect.y ) );
	}
	
	mModifiers.Append ( group );

	SetModified ( true );
}

/*
================
rvGEWorkspace::AddModifierSize

Adds a size modifier with the given offsets
================
*/
void rvGEWorkspace::AddModifierSize ( const char* modName, float l, float t, float r, float b, bool snap )
{
	idRectangle scaleRect;
	idRectangle	sizeRect;
	idRectangle newRect;
	
	scaleRect = mSelections.GetRect ( );
	WindowToWorkspace ( scaleRect );	
	newRect = scaleRect;
	newRect.x += l;
	newRect.y += t;
	newRect.w += (r - l);
	newRect.h += (b - t);

	// Restrict sizing below 1 width
	if ( newRect.w <= 1 )
	{
		newRect.x	 = newRect.x - (l ? (1 - newRect.w) : 0);
		mDragPoint.x = newRect.x;
		newRect.w	 = 1;
		mDragX		 = false;
	}
	else
	{
		mDragX = true;
	}

	// Restrict sizing below 1 height
	if ( newRect.h <= 1 )
	{
		newRect.y	 = newRect.y - (t ? (1 - newRect.h) : 0);
		mDragPoint.y = newRect.y;
		newRect.h	 = 1;
		mDragY		 = false;
	}
	else
	{
		mDragY = true;
	}

	if ( snap )
	{
		gApp.GetOptions ().SnapRectToGrid ( newRect, l != 0.0f, t != 0.0f, r != 0.0f, b != 0.0f );
	}
	
	rvGEModifierGroup*	group = new rvGEModifierGroup;
	for ( int i = 0; i < mSelections.Num(); i ++ )
	{
		sizeRect  = rvGEWindowWrapper::GetWrapper ( mSelections[i] )->GetScreenRect ( );	
		
		l = (newRect.x + ((sizeRect.x - scaleRect.x) / scaleRect.w) * newRect.w) - sizeRect.x;
		t = (newRect.y + ((sizeRect.y - scaleRect.y) / scaleRect.h) * newRect.h) - sizeRect.y;
		r = (sizeRect.w / scaleRect.w * newRect.w) - sizeRect.w + l;
		b = (sizeRect.h / scaleRect.h * newRect.h) - sizeRect.h + t;
		
		// This is sorta crufty but needs to be done.  When a parent is being sized at the same
		// time as a child you will get double movement because the child is relative to the parent.  Therefore
		// we need to subtract out the closest parents sizing.
		idWindow* parent = mSelections[i];
		while ( NULL != (parent = parent->GetParent ( ) ) )
		{
			rvGEWindowWrapper*	pwrapper = rvGEWindowWrapper::GetWrapper ( parent );
			float				offset;
			
			if ( !pwrapper->IsSelected ( ) )
			{
				continue;
			}
			
			sizeRect  = pwrapper->GetScreenRect ( );				
			
			// Subtract out the left and right modifications
			offset = ((newRect.x + ((sizeRect.x - scaleRect.x) / scaleRect.w) * newRect.w) - sizeRect.x);
			l -= offset;
			r -= offset;
			
			// Subtract out the top and bottom modifications
			offset = ((newRect.y + ((sizeRect.y - scaleRect.y) / scaleRect.h) * newRect.h) - sizeRect.y);
			t -= offset;
			b -= offset;
			
			break;
		}		
			
		group->Append ( new rvGESizeModifier ( modName, mSelections[i], l, t, r, b ) );	
	}

	mModifiers.Append ( group );	

	SetModified ( true );
}

/*
================
rvGEWorkspace::MakeSelectedAChild

Makes the selected windows a child of the first selected window
================
*/
void rvGEWorkspace::MakeSelectedAChild ( void )
{
	rvGEModifierGroup*	group;
	int					i;
	
	if ( !rvGEWindowWrapper::GetWrapper ( mSelections[0] )->CanHaveChildren ( ) )
	{
		gApp.MessageBox ( "Cannot add children to an htmlDef item", MB_OK|MB_ICONERROR );
		return;
	}
	
	group = new rvGEModifierGroup;	

	for ( i = 1; i < mSelections.Num(); i ++ )
	{
		if ( mSelections[i]->GetParent ( ) == mSelections[0] )
		{
			continue;
		}
		
		if ( !mSelections[i]->GetParent ( ) )
		{	
			continue;
		}	

		group->Append ( new rvGEInsertModifier ( "Make Child", mSelections[i], mSelections[0], NULL ) );
	}

	mModifiers.Append ( group );
	
	// Navigator needs an update since the ordering has changed
	gApp.GetNavigator().Update ( );

	SetModified ( true );
}

void rvGEWorkspace::Copy ( void )
{
	int i;
	
	// Clear the current clipboard 
	for ( i = 0; i < mClipboard.Num(); i ++ )
	{
		delete mClipboard[i];
	}
	
	mClipboard.Clear ( );
	
	for ( i = 0; i < mSelections.Num(); i ++ )
	{
		rvGEWindowWrapper* wrapper = rvGEWindowWrapper::GetWrapper ( mSelections[i] );
		assert ( wrapper );
		
		rvGEClipboardItem* item = new rvGEClipboardItem;
		item->mStateDict  = wrapper->GetStateDict ( );
		item->mScriptDict = wrapper->GetScriptDict ( );
		item->mVarDict    = wrapper->GetVariableDict ( );

		item->mStateDict.Set ( "windowType", rvGEWindowWrapper::WindowTypeToString ( wrapper->GetWindowType ( ) ) );
		
		mClipboard.Append ( item );
	}
}

void rvGEWorkspace::Paste ( void )
{
	int i;
	
	rvGEModifierGroup* group = new rvGEModifierGroup;

	mSelections.Clear ( );
	
	for ( i = 0; i < mClipboard.Num(); i ++ )
	{
		idDict							state;
		rvGEWindowWrapper::EWindowType	type;
		
		state.Copy ( mClipboard[i]->mStateDict );
		type = rvGEWindowWrapper::StringToWindowType ( state.GetString ( "windowType", "windowDef" ) );
		state.Delete ( "windowType" );
	
		idWindow* window = NewWindow ( &state, type );
		group->Append ( new rvGEInsertModifier ( "Paste", window, mInterface->GetDesktop(), NULL ) );
		mSelections.Add ( window );
		
		rvGEWindowWrapper::GetWrapper ( window )->GetScriptDict ( ) = mClipboard[i]->mScriptDict;
		rvGEWindowWrapper::GetWrapper ( window )->GetVariableDict ( ) = mClipboard[i]->mVarDict;
	}
	
	mModifiers.Append ( group );

	mApplication->GetNavigator().Update ( );
	
	SetModified ( true );
}

void rvGEWorkspace::HideSelected ( void )
{
	AddModifiers ( MOD_HIDE );
	mSelections.Clear ( );
	mApplication->GetNavigator().Refresh ( );
}

void rvGEWorkspace::UnhideSelected ( void )
{
	AddModifiers ( MOD_UNHIDE );
	mApplication->GetNavigator().Refresh ( );
}

void rvGEWorkspace::HideWindow ( idWindow* window )
{
	AddModifiers ( window, MOD_HIDE );
	mApplication->GetNavigator().Refresh ( );
}

void rvGEWorkspace::UnhideWindow ( idWindow* window )
{
	AddModifiers ( window, MOD_UNHIDE );
	mApplication->GetNavigator().Refresh ( );
}

/*
================
rvGEWorkspace::SetModified

Sets the modified state of the window and if source control is enabled it
will attempt to check out the file
================
*/
void rvGEWorkspace::SetModified ( bool mod )
{
	if ( mModified != mod )
	{

		mModified = mod;
		UpdateTitle ( );

	}
}

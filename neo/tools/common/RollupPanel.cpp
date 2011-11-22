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

#include "../../sys/win32/win_local.h"
#include "RollupPanel.h"

// Based on original code by Johann Nadalutti

#define	RP_PGBUTTONHEIGHT		18
#define	RP_SCROLLBARWIDTH		6
#define	RP_GRPBOXINDENT			6
#define	RP_SCROLLBARCOLOR		RGB(150,180,180)
#define RP_ROLLCURSOR			MAKEINTRESOURCE(32649)	// see IDC_HAND (WINVER >= 0x0500)

//Popup Menu Ids
#define	RP_IDM_EXPANDALL		0x100
#define	RP_IDM_COLLAPSEALL		0x101
#define	RP_IDM_STARTITEMS		0x102

idList<HWND>	rvRollupPanel::mDialogs;	
HHOOK			rvRollupPanel::mDialogHook	= NULL;

#define DEFERPOS

/*
================
rvRollupPanel::rvRollupPanel

constructor
================
*/
rvRollupPanel::rvRollupPanel ( void )
{
	mStartYPos  = 0;
	mItemHeight = 0;
	mWindow		= NULL;
}

/*
================
rvRollupPanel::~rvRollupPanel

destructor
================
*/
rvRollupPanel::~rvRollupPanel ( void )
{
	// destroy the items
	for ( ; mItems.Num(); )
	{
		_RemoveItem ( 0 );
	}
}

/*
================
rvRollupPanel::Create

Create the rollup panel window
================
*/
bool rvRollupPanel::Create ( DWORD dwStyle, const RECT& rect, HWND parent, unsigned int id )
{
	WNDCLASSEX wndClass;
	memset ( &wndClass, 0, sizeof(wndClass) );
	wndClass.cbSize		   = sizeof(WNDCLASSEX);
	wndClass.lpszClassName = "ROLLUP_PANEL";
	wndClass.lpfnWndProc   = WindowProc;
	wndClass.hbrBackground = (HBRUSH)GetSysColorBrush ( COLOR_3DFACE ); 
	wndClass.hCursor       = LoadCursor((HINSTANCE) NULL, IDC_ARROW); 
	wndClass.lpszMenuName  = NULL;
	wndClass.hInstance     = win32.hInstance; 
	wndClass.style		   = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx ( &wndClass );

	mWindow = CreateWindowEx ( WS_EX_TOOLWINDOW, 
							"ROLLUP_PANEL", 
							"", 
							dwStyle|WS_CLIPSIBLINGS, 
							rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
							parent, 
							NULL, 
							win32.hInstance, 
							this );
							
	if ( !mWindow )
	{
		return false;
	}

	return true;	
}

/*
================
rvRollupPanel::InsertItem

Insert and item into the rollup panel.  Return -1 if an error occured
================
*/
int rvRollupPanel::InsertItem ( const char* caption, HWND dialog, bool autoDestroy, int index )
{
	assert ( caption );
	assert ( dialog );

	// -1 means add to the end	
	if ( index > 0 && index >= mItems.Num() )
	{
		index = -1;
	}

 	// Get client rect
	RECT r;
	GetClientRect(mWindow,&r);

	// Create the GroupBox control
	HWND groupbox = CreateWindow ( "BUTTON", "", WS_CHILD|BS_GROUPBOX, 
								   r.left, r.top, r.right-r.left, r.bottom-r.top,
								   mWindow, 0, win32.hInstance, NULL );
								   
	// Create the expand button
	HWND button = CreateWindow ( "BUTTON", caption, WS_CHILD|BS_AUTOCHECKBOX|BS_PUSHLIKE|BS_FLAT, 
								   r.left, r.top, r.right-r.left, r.bottom-r.top,
								   mWindow, 0, win32.hInstance, NULL );

	// Change the button's font
	SendMessage ( button, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT), 0 );

	// Add item to the item list
	RPITEM* item = new RPITEM; 
	item->mExpanded		 = false;
	item->mEnable		 = true;
	item->mDialog		 = dialog;
	item->mButton		 = button;
	item->mGroupBox		 = groupbox;
	item->mOldDlgProc	 = (WNDPROC) GetWindowLong ( dialog, DWL_DLGPROC );
	item->mOldButtonProc = (WNDPROC) GetWindowLong ( button, GWL_WNDPROC );
	item->mAutoDestroy	 = autoDestroy;
	strcpy ( item->mCaption, caption );

	if ( index < 0 )
	{
		index = mItems.Append ( item );
	}
	else
	{ 
		mItems.Insert ( item, index );
	}

	// Store data with the dialog window in its user data 
	SetWindowLong ( dialog, GWL_USERDATA,	(LONG)item );

	// Attach item to button through user data
	SetWindowLong ( button, GWL_USERDATA,	(LONG)item );
	SetWindowLong ( button, GWL_ID,			index );

	// Subclass dialog
	SetWindowLong ( dialog, DWL_DLGPROC, (LONG)DialogProc );

	// SubClass button
	SetWindowLong ( button, GWL_WNDPROC, (LONG)ButtonProc );

	// Update
	mItemHeight += RP_PGBUTTONHEIGHT+(RP_GRPBOXINDENT/2);
	RecallLayout ( );

	// One hook for all panel dialogs
	if ( !mDialogHook )
	{
		mDialogHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc, NULL, GetCurrentThreadId() );
	}
	
	mDialogs.Append ( dialog );

	return index;
}

/*
================
rvRollupPanel::RemoveItem

Remove the item at the given index from the rollup panel
================
*/
void rvRollupPanel::RemoveItem ( int index )
{
	// safety check
	if ( index >= mItems.Num() || index < 0 )
	{
		return;
	}

	// remove the item
	_RemoveItem( index );

	// update the layout
	RecallLayout ( );	
}

/*
================
rvRollupPanel::RemoveAllItems

Remove all items from the control
================
*/
void rvRollupPanel::RemoveAllItems()
{
	for ( ; mItems.Num(); )
	{
		_RemoveItem ( 0 );
	}

	// update layout
	RecallLayout ( );
}

/*
================
rvRollupPanel::_RemoveItem

called by RemoveItem and RemoveAllItems methods to acutally remove the item
================
*/
void rvRollupPanel::_RemoveItem ( int index )
{
	RPITEM* item = mItems[index];

	// get the item rect
	RECT ir;
	GetWindowRect ( item->mDialog, &ir );

	// update item height
	mItemHeight -= RP_PGBUTTONHEIGHT+(RP_GRPBOXINDENT/2);
	if ( item->mExpanded )
	{
		mItemHeight -= (ir.bottom-ir.top);
	}

	// destroy windows
	if ( item->mButton ) 
	{
		DestroyWindow ( item->mButton );
	}
	if ( item->mGroupBox )
	{
		DestroyWindow ( item->mGroupBox );
	}
	if ( item->mDialog && item->mAutoDestroy )
	{
		DestroyWindow ( item->mDialog );
		mDialogs.Remove ( item->mDialog );		
	}

	if ( mDialogs.Num () <= 0 )
	{
		UnhookWindowsHookEx( mDialogHook );
		mDialogHook = NULL;
	}

	// finish up
	mItems.RemoveIndex ( index );
	delete item;		
}

/*
================
rvRollupPanel::ExpandItem

expand or collapse the item at the given index
================
*/
void rvRollupPanel::ExpandItem( int index, bool expand )
{
	// safety check
	if ( index >= mItems.Num() || index < 0 )
	{
		return;
	}

	_ExpandItem ( mItems[index], expand );

	RecallLayout ( );

	// scroll to this page (automatic page visibility)
	if ( expand )
	{
		ScrollToItem ( index, false );
	}
}

/*
================
rvRollupPanel::ExpandItem

expand or collapse the item at the given index
================
*/
void rvRollupPanel::ExpandAllItems( bool expand )
{
	int i;
	
	// expand all items
	for ( i=0; i < mItems.Num(); i ++ )
	{
		_ExpandItem ( mItems[i], expand );
	}

	RecallLayout();
}

/*
================
rvRollupPanel::ExpandItem

expand or collapse the item at the given index
================
*/
void rvRollupPanel::_ExpandItem ( RPITEM* item, bool expand )
{
	// check if we need to change state
	if ( item->mExpanded == expand || !item->mEnable )
	{
		return;
	}

	RECT ir;
	GetWindowRect ( item->mDialog, &ir );

	// Expand-collapse
	item->mExpanded = expand;

	if ( expand )
	{
		mItemHeight += (ir.bottom - ir.top);
	}
	else
	{
		mItemHeight -= (ir.bottom - ir.top);
	}
}

/*
================
rvRollupPanel::EnableItem

enable/disable the item at the given index
================
*/
void rvRollupPanel::EnableItem ( int index, bool enable )
{
	// safety check
	if ( index >= mItems.Num() || index < 0 )
	{
		return;
	}

	_EnableItem ( mItems[index], enable );
	RecallLayout ( );
}

/*
================
rvRollupPanel::EnableAllItems

enable/disable all items in the panel
================
*/
void rvRollupPanel::EnableAllItems ( bool enable )
{
	int i;
	
	for ( i=0; i < mItems.Num(); i++ )
	{
		_EnableItem ( mItems[i], enable );
	}
	
	RecallLayout ( );
}

/*
================
rvRollupPanel::_EnableItem

Called by EnableItem and EnableAllItems to do the work of enabling/disablgin
the window
================
*/
void rvRollupPanel::_EnableItem ( RPITEM* item, bool enable )
{
	// check if we need to change state
	if ( item->mEnable == enable )
	{
		return;
	}

	RECT ir;
	GetWindowRect ( item->mDialog, &ir );

	item->mEnable = enable;

	if ( item->mExpanded )
	{ 
		mItemHeight -= (ir.bottom-ir.top); 
		item->mExpanded = false;
	}
}

/*
================
rvRollupPanel::ScrollToItem

Scroll a page at the top of the Rollup Panel if top = true or just ensure 
item visibility into view if top = false 
================
*/
void rvRollupPanel::ScrollToItem ( int index, bool top )
{
	// safety check
	if ( index >= mItems.Num() || index < 0 ) 
	{
		return;
	}

	RPITEM* item = mItems[index];

	// get rects
	RECT r;
	RECT ir;
	GetWindowRect ( mWindow, &r );
	GetWindowRect ( item->mDialog, &ir );

	// check page visibility
	if ( top || ((ir.bottom > r.bottom) || (ir.top < r.top)))
	{
		// compute new mStartYPos
		GetWindowRect( item->mButton, &ir );
		mStartYPos -= (ir.top-r.top);

		RecallLayout();
	}
}

/*
================
rvRollupPanel::MoveItemAt

newIndex can be equal to -1 (move at end)
return -1 if an error occurs
================
*/
int rvRollupPanel::MoveItemAt ( int index, int newIndex )
{
	if ( index == newIndex || index >= mItems.Num() || index < 0 )
	{
		return -1;
	}

	// remove page from its old position
	RPITEM* item = mItems[index];
	mItems.RemoveIndex ( index );

	// insert at its new position
	if ( newIndex < 0 )
	{
		index = mItems.Append( item );
	}
	else
	{ 
		mItems.Insert ( item, newIndex );
		index = newIndex;
	}

	RecallLayout ( );
	
	return index;
}

/*
================
rvRollupPanel::RecallLayout

Update the layout of the control based on current states
================
*/
void rvRollupPanel::RecallLayout ( void )
{
	int	 bottomPagePos;
	RECT r;
	int  posy;
	int	 i;
	
	// check StartPosY
	GetClientRect ( mWindow, &r );
	bottomPagePos = mStartYPos + mItemHeight;

	if ( bottomPagePos < r.bottom-r.top )
	{
		mStartYPos = (r.bottom-r.top) - mItemHeight;
	}
	if ( mStartYPos > 0 )
	{
		mStartYPos = 0;
	}

	// update layout
#ifdef DEFERPOS			
	HDWP hdwp;
	hdwp = BeginDeferWindowPos ( mItems.Num() * 3 );
#endif
	posy = mStartYPos;

	for ( i=0; i < mItems.Num(); i++ )
	{
		RPITEM* item = mItems[i];

		// enable / disable button
		SendMessage ( item->mButton, BM_SETCHECK, (item->mEnable&item->mExpanded)?BST_CHECKED:BST_UNCHECKED, 0 );
		EnableWindow ( item->mButton, item->mEnable );

		// Expanded
		if ( item->mExpanded && item->mEnable ) 
		{
			RECT ir;
			GetWindowRect ( item->mDialog, &ir );

			// update GroupBox position and size
#ifdef DEFERPOS			
			DeferWindowPos ( hdwp, 
#else
			SetWindowPos (   
#endif
							 item->mGroupBox, 0, 2, posy, 
							 (r.right-r.left)-3-RP_SCROLLBARWIDTH, 
							 (ir.bottom-ir.top)+RP_PGBUTTONHEIGHT+RP_GRPBOXINDENT-4, 
							 SWP_NOZORDER|SWP_SHOWWINDOW);

			//Update Dialog position and size
#ifdef DEFERPOS			
			DeferWindowPos ( hdwp, 
#else
			SetWindowPos (   
#endif
							 item->mDialog, 0, RP_GRPBOXINDENT, posy+RP_PGBUTTONHEIGHT, 
							 (r.right-r.left)-RP_SCROLLBARWIDTH-(RP_GRPBOXINDENT*2), 
							 ir.bottom-ir.top, SWP_NOZORDER|SWP_SHOWWINDOW);

			//Update Button's position and size
#ifdef DEFERPOS			
			DeferWindowPos ( hdwp, 
#else
			SetWindowPos (   
#endif
							 item->mButton, 0, RP_GRPBOXINDENT, posy, 
							 (r.right-r.left)-RP_SCROLLBARWIDTH-(RP_GRPBOXINDENT*2), 
							 RP_PGBUTTONHEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW);

			posy += (ir.bottom-ir.top) + RP_PGBUTTONHEIGHT;
		} 
		// collapsed
		else 
		{
			// update GroupBox position and size
#ifdef DEFERPOS			
			DeferWindowPos ( hdwp, 
#else
			SetWindowPos (   
#endif
							 item->mGroupBox, 0, 2, posy, 
							 (r.right-r.left)-3-RP_SCROLLBARWIDTH, 16, SWP_NOZORDER|SWP_SHOWWINDOW);

			// update Dialog position and size
#ifdef DEFERPOS			
			DeferWindowPos ( hdwp, 
#else
			SetWindowPos (   
#endif
							item->mDialog, 0, RP_GRPBOXINDENT, 0, 0, 0,SWP_NOZORDER|SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE);

			// update Button's position and size
#ifdef DEFERPOS			
			DeferWindowPos ( hdwp, 
#else
			SetWindowPos (   
#endif
							item->mButton, 0, RP_GRPBOXINDENT, posy, 
							(r.right-r.left)-RP_SCROLLBARWIDTH-(RP_GRPBOXINDENT*2), 
							RP_PGBUTTONHEIGHT, SWP_NOZORDER|SWP_SHOWWINDOW);

			posy += RP_PGBUTTONHEIGHT;
		}

		posy += (RP_GRPBOXINDENT/2);

	}
	
#ifdef DEFERPOS			
	EndDeferWindowPos ( hdwp );
#endif

	// update Scroll Bar
	RECT br;
	SetRect ( &br, r.right-RP_SCROLLBARWIDTH,r.top,r.right,r.bottom);		
	InvalidateRect( mWindow, &br, FALSE );
	UpdateWindow ( mWindow );
}

/*
================
rvRollupPanel::GetItemIndex

Return -1 if no matching item was found, otherwise the index of the item
================
*/
int rvRollupPanel::GetItemIndex ( HWND wnd )
{
	int i;
	
	//Search matching button's hwnd
	for ( i=0; i < mItems.Num(); i++ )
	{
		if ( wnd == mItems[i]->mButton )
		{
			return i;
		}
	}

	return -1;
}

int rvRollupPanel::GetItemIndex	( const char* caption )
{
	int i;
	
	//Search matching button's hwnd
	for ( i=0; i < mItems.Num(); i++ )
	{
		if ( !idStr::Icmp ( caption, mItems[i]->mCaption ) )
		{
			return i;
		}
	}

	return -1;
}

/*
================
rvRollupPanel::GetItem

Return NULL if the index is invalid
================
*/
RPITEM* rvRollupPanel::GetItem ( int index )
{
	// safety check
	if ( index >= mItems.Num() || index < 0 ) 
	{
		return NULL;
	}

	return mItems[index];
}

/*
================
rvRollupPanel::DialogProc

Dialog procedure for items
================
*/
LRESULT CALLBACK rvRollupPanel::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RPITEM*			item  = (RPITEM*)GetWindowLong ( hWnd, GWL_USERDATA );
	rvRollupPanel*	_this = (rvRollupPanel*)GetWindowLong ( GetParent ( hWnd ), GWL_USERDATA );

	RECT r;
	GetClientRect ( _this->mWindow, &r );

	if ( _this->mItemHeight > r.bottom-r.top )
	{
		switch (uMsg) 
		{
			case WM_LBUTTONDOWN:
			case WM_MBUTTONDOWN:
			{
				POINT pos;
				GetCursorPos ( &pos );
				_this->mOldMouseYPos = pos.y;
				::SetCapture(hWnd);
				return 0;
			}

			case WM_LBUTTONUP:
			case WM_MBUTTONUP:
			{
				if ( ::GetCapture() == hWnd )
				{ 
					::ReleaseCapture(); 
					return 0; 
				}
				break;
			}

			case WM_MOUSEMOVE:
				if ( (::GetCapture() == hWnd) && (wParam==MK_LBUTTON || wParam==MK_MBUTTON)) 
				{
					POINT pos;
					GetCursorPos(&pos);
					_this->mStartYPos += (pos.y-_this->mOldMouseYPos);
					_this->RecallLayout();
					_this->mOldMouseYPos = pos.y;
					InvalidateRect ( _this->mWindow, NULL, TRUE );
					return 0;
				}

				break;

			case WM_SETCURSOR:
				if ( (HWND)wParam == hWnd)
				{ 
					SetCursor ( LoadCursor (NULL, RP_ROLLCURSOR) ); 
					return TRUE; 
				}
				break;
		}
	}

	return ::CallWindowProc ( item->mOldDlgProc, hWnd, uMsg, wParam, lParam );
}

/*
================
rvRollupPanel::DialogProc

Button procedure for items
================
*/
LRESULT CALLBACK rvRollupPanel::ButtonProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_SETFOCUS )
	{
		return FALSE;
	}

	RPITEM* item = (RPITEM*)GetWindowLong(hWnd, GWL_USERDATA);	
	return ::CallWindowProc( item->mOldButtonProc, hWnd, uMsg, wParam, lParam );
}

/*
================
rvRollupPanel::WindowProc

Window procedure for rollup panel
================
*/
LRESULT CALLBACK rvRollupPanel::WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	rvRollupPanel* panel;
	panel = (rvRollupPanel*)GetWindowLong (hWnd, GWL_USERDATA);	
	
	switch ( uMsg )
	{
		case WM_CREATE:
		{
			LPCREATESTRUCT	cs;

			// Attach the class to the window first
			cs = (LPCREATESTRUCT) lParam;
			panel = (rvRollupPanel*) cs->lpCreateParams;
			SetWindowLong ( hWnd, GWL_USERDATA, (LONG)panel );
			break;
		}
		
		case WM_COMMAND:
			panel->HandleCommand ( wParam, lParam );
			break;
			
		case WM_PAINT:
			return panel->HandlePaint ( wParam, lParam );			
			
		case WM_SIZE:
			return panel->HandleSize ( wParam, lParam );
			
		case WM_LBUTTONDOWN:
			panel->HandleLButtonDown ( wParam, lParam );
			break;
			
		case WM_LBUTTONUP:
			panel->HandleLButtonUp ( wParam, lParam );
			break;
		
		case WM_MOUSEMOVE:
			panel->HandleMouseMove ( wParam, lParam );
			break;
			
		case WM_MOUSEWHEEL:
			panel->HandleMouseWheel ( wParam, lParam );
			break;
		
		case WM_MOUSEACTIVATE:
			panel->HandleMouseActivate ( wParam, lParam );
			break;
			
		case WM_CONTEXTMENU:
			return panel->HandleContextMenu ( wParam, lParam );			
	}
			
	return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}

/*
================
rvRollupPanel::HandleCommand

Handle the WM_COMMAND message
================
*/
int rvRollupPanel::HandleCommand ( WPARAM wParam, LPARAM lParam ) 
{
	// popup menu command to expand or collapse pages
	if ( LOWORD(wParam) == RP_IDM_EXPANDALL )
	{
		ExpandAllItems ( true );
	}
	else if	( LOWORD(wParam) == RP_IDM_COLLAPSEALL )
	{
		ExpandAllItems ( false );
	}

	// popupMenu command to expand page
	else if ( LOWORD(wParam) >= RP_IDM_STARTITEMS && 
			  LOWORD(wParam) <  RP_IDM_STARTITEMS + GetItemCount ( ) )
	{
		int index = LOWORD(wParam)-RP_IDM_STARTITEMS;
		ExpandItem ( index, !IsItemExpanded(index) );
	}

	// button command
	else if ( HIWORD(wParam) == BN_CLICKED )
	{
		int index = GetItemIndex ((HWND)lParam);
		if ( index != -1 ) 
		{
			ExpandItem ( index, !IsItemExpanded ( index ) );
			return 0;
		}
	}

	return 0;
}

/*
================
rvRollupPanel::HandlePaint

Handle the WM_PAINT message
================
*/
int rvRollupPanel::HandlePaint( WPARAM wParam, LPARAM lParam ) 
{
	HDC			dc;
	PAINTSTRUCT ps;
	RECT		r;
	RECT		br;
	int			sbPos;
	int			sbSize;
	int			clientHeight;
	
	dc = BeginPaint ( mWindow, &ps );

	// scrollbar
	GetClientRect ( mWindow, &r );
	SetRect ( &br, r.right-RP_SCROLLBARWIDTH, r.top, r.right, r.bottom );
	DrawEdge ( dc, &br, EDGE_RAISED, BF_RECT  );

	sbPos = 0;
	sbSize = 0;
	clientHeight = (r.bottom-r.top) - 4;

	if ( mItemHeight > (r.bottom-r.top) ) 
	{
		sbSize = clientHeight - (((mItemHeight-(r.bottom-r.top)) * clientHeight ) / mItemHeight );
		sbPos  = -(mStartYPos * clientHeight) / mItemHeight;
	} 
	else 
	{
		sbSize = clientHeight;
	}

	br.left		+=2;
	br.right	-=1;
	br.top		= sbPos+2;
	br.bottom	= br.top+sbSize;

	HBRUSH brush;
	brush = CreateSolidBrush ( RP_SCROLLBARCOLOR );
	FillRect ( dc, &br, brush );
	DeleteObject ( brush );

	SetRect ( &r, br.left,2,br.right,br.top );
	FillRect ( dc, &r, (HBRUSH)GetStockObject ( BLACK_BRUSH ) );

	SetRect ( &r, br.left,br.bottom,br.right,2+clientHeight );
	FillRect ( dc, &r, (HBRUSH)GetStockObject ( BLACK_BRUSH ) );
	
	return 0;
}

/*
================
rvRollupPanel::HandleSize

Handle the WM_SIZE message
================
*/
int rvRollupPanel::HandleSize ( WPARAM wParam, LPARAM lParam )
{
	DefWindowProc ( mWindow, WM_SIZE, wParam, lParam );
	RecallLayout();
	return 0;
}

/*
================
rvRollupPanel::HandleLButtonDown

Handle the WM_LBUTTONDOWN message
================
*/
int rvRollupPanel::HandleLButtonDown ( WPARAM wParam, LPARAM lParam ) 
{
	RECT	r;
	RECT	br;
	POINT	point;
	
	GetClientRect ( mWindow, &r );
	if ( mItemHeight <= r.bottom - r.top )
	{
		return 0;
	}

	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);

	SetRect ( &br, r.right - RP_SCROLLBARWIDTH, r.top, r.right, r.bottom );

	if ( (wParam & MK_LBUTTON) && PtInRect ( &br, point ) ) 
	{
		SetCapture( mWindow );

		int clientHeight = (r.bottom-r.top) - 4;

		int sbSize = clientHeight - (((mItemHeight - (r.bottom-r.top)) * clientHeight) / mItemHeight );
		int	sbPos  = -(mStartYPos * clientHeight) / mItemHeight;

		// click inside scrollbar cursor
		if ( (point.y < (sbPos + sbSize)) && (point.y > sbPos )) 
		{
			mSBOffset = sbPos - point.y + 1;		
		} 
		// click outside scrollbar cursor (2 cases => above or below cursor)
		else 
		{
			int distup	 = point.y - sbPos;	
			int distdown = (sbPos + sbSize) - point.y;
			
			if ( distup < distdown )
			{
				//above
				mSBOffset = 0;
			}
			else
			{
				//below
				mSBOffset = -sbSize;
			}
		}

		// calc new m_nStartYPos from mouse pos
		int targetPos = point.y + mSBOffset;
		mStartYPos =- (targetPos * mItemHeight) / clientHeight;

		// update
		RecallLayout();
	}
	
	return 0;
}

/*
================
rvRollupPanel::HandleLButtonUp

Handle the WM_LBUTTONUP message
================
*/
int rvRollupPanel::HandleLButtonUp ( WPARAM wParam, LPARAM lParam ) 
{
	if ( GetCapture() == mWindow )
	{
		ReleaseCapture();
	}
	
	return 0;
}

/*
================
rvRollupPanel::HandleMouseMove

Handle the WM_MOUSEMOVE message
================
*/
int rvRollupPanel::HandleMouseMove ( WPARAM wParam, LPARAM lParam ) 
{
	RECT  r;
	RECT  br;
	POINT point;
	
	GetClientRect ( mWindow, &r );
	if ( mItemHeight <= r.bottom - r.top )
	{
		return 0;
	}

	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);

	SetRect ( &br, r.right - RP_SCROLLBARWIDTH, r.top, r.right, r.bottom );

	if ( (wParam & MK_LBUTTON) && (GetCapture() == mWindow )) 
	{
		// calc new m_nStartYPos from mouse pos
		int clientHeight	= (r.bottom-r.top) - 4;
		int targetPos		= point.y + mSBOffset;
		
		mStartYPos =- (targetPos * mItemHeight) / clientHeight;

		RecallLayout ( );
		
		InvalidateRect ( mWindow, NULL, FALSE );		
//		UpdateWindow ( mWindow );
	}

	return 0;
}

/*
================
rvRollupPanel::HandleMouseWheel

Handle the WM_MOUSEWHEEL message
================
*/
int rvRollupPanel::HandleMouseWheel ( WPARAM wParam, LPARAM lParam ) 
{
	// calc new m_nStartYPos
	mStartYPos += (HIWORD(wParam) / 4);

	RecallLayout();

	return 0;
}

/*
================
rvRollupPanel::HandleMouseActivate

Handle the WM_MOUSEACTIVATE message
================
*/
int rvRollupPanel::HandleMouseActivate  ( WPARAM wParam, LPARAM lParam ) 
{
	SetFocus ( mWindow );
	return 0;
}

/*
================
rvRollupPanel::HandleContextMenu

Handle the WM_CONTEXTMENU message
================
*/
int rvRollupPanel::HandleContextMenu ( WPARAM wParam, LPARAM lParam )
{
	HMENU menu;
	int	  i;
	POINT point;
	
	menu = CreatePopupMenu ( );
	if ( !menu )
	{
		return 0;
	}

	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);

	AppendMenu ( menu, MF_STRING,		RP_IDM_EXPANDALL,	"Expand all"	);
	AppendMenu ( menu, MF_STRING,		RP_IDM_COLLAPSEALL,	"Collapse all"	);
	AppendMenu ( menu, MF_SEPARATOR,	0,					""				);

	//Add all pages with checked style for expanded ones
	for ( i=0; i < mItems.Num(); i++ )
	{
		char itemName[1024];
		GetWindowText ( mItems[i]->mButton, itemName, 1023 );
		AppendMenu ( menu, MF_STRING, RP_IDM_STARTITEMS + i, itemName );	

		if ( mItems[i]->mExpanded )
		{
			CheckMenuItem ( menu, RP_IDM_STARTITEMS + i, MF_CHECKED);
		}

		TrackPopupMenu ( menu, TPM_LEFTALIGN|TPM_LEFTBUTTON, point.x, point.y, 0, mWindow, NULL );
	}
	
	return 0;
}

/*
================
rvRollupPanel::GetMsgProc

Ensures normal dialog functions work in the alpha select dialog
================
*/
LRESULT FAR PASCAL rvRollupPanel::GetMsgProc ( int nCode, WPARAM wParam, LPARAM lParam )
{
	LPMSG lpMsg = (LPMSG) lParam;

	if ( nCode >= 0 && PM_REMOVE == wParam )
	{
		// Don't translate non-input events.
		if ( (lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) )
		{
			int i;
			for ( i = 0; i < mDialogs.Num(); i ++ )
			{
				if ( IsDialogMessage( mDialogs[i], lpMsg) )
				{
					// The value returned from this hookproc is ignored, 
					// and it cannot be used to tell Windows the message has been handled.
					// To avoid further processing, convert the message to WM_NULL 
					// before returning.
					lpMsg->message = WM_NULL;
					lpMsg->lParam  = 0;
					lpMsg->wParam  = 0;
					break;
				}
			}
		}
	}

	return CallNextHookEx ( mDialogHook, nCode, wParam, lParam);
} 

/*
================
rvRollupPanel::AutoSize

Automatically set the width of the control based on the dialogs it contains
================
*/
void rvRollupPanel::AutoSize ( void )
{
	int i;
	int width = 0;
	for ( i = 0; i < mItems.Num(); i ++ )
	{
		RECT r;
		int  w;
		GetWindowRect ( mItems[i]->mDialog, &r );
		w = (r.right-r.left)+RP_SCROLLBARWIDTH+(RP_GRPBOXINDENT*2);
		if ( w > width )
		{
			width = w;
		}
	}
	
	RECT cr;
	GetWindowRect ( mWindow, &cr );
	SetWindowPos ( mWindow, NULL, 0, 0, width, cr.bottom-cr.top, SWP_NOMOVE|SWP_NOZORDER );
}


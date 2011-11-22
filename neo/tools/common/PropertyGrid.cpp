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
#include "PropertyGrid.h"

class rvPropertyGridItem
{
public:

	rvPropertyGridItem ( )
	{
	}
	
	idStr						mName;
	idStr						mValue;
	rvPropertyGrid::EItemType	mType;
};

/*
================
rvPropertyGrid::rvPropertyGrid

constructor
================
*/
rvPropertyGrid::rvPropertyGrid ( void )
{
	mWindow			= NULL;
	mEdit			= NULL;
	mListWndProc	= NULL;
	mSplitter		= 100;
	mSelectedItem	= -1;
	mEditItem		= -1;
	mState			= STATE_NORMAL;
}

/*
================
rvPropertyGrid::Create

Create a new property grid control with the given id and parent
================
*/
bool rvPropertyGrid::Create ( HWND parent, int id, int style )
{
	mStyle = style;

	// Create the List view
	mWindow = CreateWindowEx ( 0, "LISTBOX", "", WS_VSCROLL|WS_CHILD|WS_VISIBLE|LBS_OWNERDRAWFIXED|LBS_NOINTEGRALHEIGHT|LBS_NOTIFY, 0, 0, 0, 0, parent, (HMENU)id, win32.hInstance, 0 );	
	mListWndProc = (WNDPROC)GetWindowLong ( mWindow, GWL_WNDPROC );
	SetWindowLong ( mWindow, GWL_USERDATA, (LONG)this );
	SetWindowLong ( mWindow, GWL_WNDPROC, (LONG)WndProc );

	LoadLibrary ( "Riched20.dll" );
	mEdit = CreateWindowEx ( 0, "RichEdit20A", "", WS_CHILD, 0, 0, 0, 0, mWindow, (HMENU) 999, win32.hInstance, NULL );
	SendMessage ( mEdit, EM_SETEVENTMASK, 0, ENM_KEYEVENTS );

	// Set the font of the list box
	HDC			dc;
	LOGFONT		lf;
	
	dc = GetDC ( mWindow );
	ZeroMemory ( &lf, sizeof(lf) );
	lf.lfHeight = -MulDiv(8, GetDeviceCaps(dc, LOGPIXELSY), 72);
	strcpy ( lf.lfFaceName, "MS Shell Dlg" );	
	SendMessage ( mWindow, WM_SETFONT, (WPARAM)CreateFontIndirect ( &lf ), 0 );		
	SendMessage ( mEdit, WM_SETFONT, (WPARAM)CreateFontIndirect ( &lf ), 0 );		
	ReleaseDC ( mWindow, dc );

	RemoveAllItems ( );
		
	return true;
}

/*
================
rvPropertyGrid::Move

Move the window
================
*/
void rvPropertyGrid::Move ( int x, int y, int w, int h, BOOL redraw )
{
	MoveWindow ( mWindow, x, y, w, h, redraw );
}

/*
================
rvPropertyGrid::StartEdit

Start editing
================
*/
void rvPropertyGrid::StartEdit ( int item, bool label )
{
	rvPropertyGridItem* gitem;
	RECT				rItem;
			
	gitem = (rvPropertyGridItem*)SendMessage ( mWindow, LB_GETITEMDATA, item, 0 );
	if ( NULL == gitem )
	{
		return;
	}
	
	SendMessage ( mWindow, LB_GETITEMRECT, item, (LPARAM)&rItem );
	if ( label )
	{
		rItem.right = rItem.left + mSplitter - 1;
	}
	else
	{
		rItem.left = rItem.left + mSplitter + 1;
	}

	mState = STATE_EDIT;
	mEditItem = item;
	mEditLabel = label;
		
	SetWindowText ( mEdit, label?gitem->mName:gitem->mValue );					
	MoveWindow ( mEdit, rItem.left, rItem.top + 2,
				rItem.right - rItem.left,
				rItem.bottom - rItem.top - 2, TRUE );
	ShowWindow ( mEdit, SW_SHOW );		
	
	SetFocus ( mEdit );
}

/*
================
rvPropertyGrid::FinishEdit

Finish editing by copying the data in the edit control to the internal value
================
*/
void rvPropertyGrid::FinishEdit ( void )
{
	char				value[1024];
	rvPropertyGridItem* item;
	bool				update;
	
	if ( mState != STATE_EDIT )
	{
		return;
	}

	assert ( mEditItem >= 0 );
	
	mState = STATE_FINISHEDIT;
	
	update = false;
	item = (rvPropertyGridItem*)SendMessage ( mWindow, LB_GETITEMDATA, mEditItem, 0 );
	assert ( item );													
						
	GetWindowText ( mEdit, value, 1023 );
	
	if ( !value[0] )
	{
		mState = STATE_EDIT;
		MessageBeep ( MB_ICONASTERISK );
		return;
	}
	
	if ( !mEditLabel && item->mValue.Cmp ( value ) )
	{
		NMPROPGRID nmpg;
		nmpg.hdr.code = PGN_ITEMCHANGED;
		nmpg.hdr.hwndFrom = mWindow;
		nmpg.hdr.idFrom = GetWindowLong ( mWindow, GWL_ID );
		nmpg.mName  = item->mName;
		nmpg.mValue = value;										

		if ( !SendMessage ( GetParent ( mWindow ), WM_NOTIFY, 0, (LONG)&nmpg ) )
		{
			mState = STATE_EDIT;
			SetFocus ( mEdit );
			return;
		}

		// The item may have been destroyed and recreated in the notify call so get it again
		item = (rvPropertyGridItem*)SendMessage ( mWindow, LB_GETITEMDATA, mEditItem, 0 );
		if ( item )
		{
			item->mValue = value;						
			update = true;
		}
	}
	else if ( mEditLabel && item->mName.Cmp ( value ) )
	{
		int sel;
		sel = AddItem ( value, "", PGIT_STRING );
		SetCurSel ( sel );
		StartEdit ( sel, false );
		return;
	}	

	SetCurSel ( mEditItem );

	mState = STATE_NORMAL;
	mEditItem = -1;

	ShowWindow ( mEdit, SW_HIDE );	
	SetFocus ( mWindow );
}

/*
================
rvPropertyGrid::CancelEdit

Stop editing without saving the data
================
*/
void rvPropertyGrid::CancelEdit ( void )
{
	if ( mState == STATE_EDIT && !mEditLabel )
	{
		if ( !*GetItemValue ( mEditItem ) )
		{
			RemoveItem ( mEditItem );			
		}
	}

	mSelectedItem = mEditItem;
	mEditItem = -1;
	mState = STATE_NORMAL;
	ShowWindow ( mEdit, SW_HIDE );
	SetFocus ( mWindow );	
	SetCurSel ( mSelectedItem );
}

/*
================
rvPropertyGrid::AddItem

Add a new item to the property grid
================
*/
int rvPropertyGrid::AddItem ( const char* name, const char* value, EItemType type )
{
	rvPropertyGridItem* item;
	int					insert;

	// Cant add headers if headers arent enabled
	if ( type == PGIT_HEADER && !(mStyle&PGS_HEADERS) )
	{
		return -1;
	}

	item = new rvPropertyGridItem;
	item->mName = name;
	item->mValue = value;
	item->mType = type;
	
	insert = SendMessage(mWindow,LB_GETCOUNT,0,0) - ((mStyle&PGS_ALLOWINSERT)?1:0);
	
	return SendMessage ( mWindow, LB_INSERTSTRING, insert, (LONG)item );
}

/*
================
rvPropertyGrid::RemoveItem

Remove the item at the given index
================
*/
void rvPropertyGrid::RemoveItem ( int index )
{
	if ( index < 0 || index >= SendMessage ( mWindow, LB_GETCOUNT, 0, 0 ) )
	{
		return;
	}
	
	delete (rvPropertyGridItem*)SendMessage ( mWindow, LB_GETITEMDATA, index, 0 );
	
	SendMessage ( mWindow, LB_DELETESTRING, index, 0 );
}

/*
================
rvPropertyGrid::RemoveAllItems

Remove all items from the property grid
================
*/
void rvPropertyGrid::RemoveAllItems ( void )
{
	int i;
	
	// free the memory for all the items
	for ( i = SendMessage ( mWindow, LB_GETCOUNT, 0, 0 ); i > 0; i -- )
	{		
		delete (rvPropertyGridItem*)SendMessage ( mWindow, LB_GETITEMDATA, i - 1, 0 );
	}

	// remove all items from the listbox itself
	SendMessage ( mWindow, LB_RESETCONTENT, 0, 0 );	

	if ( mStyle & PGS_ALLOWINSERT )
	{
		// Add the item used to add items
		rvPropertyGridItem* item;
		item = new rvPropertyGridItem;
		item->mName = "";
		item->mValue = "";
		SendMessage ( mWindow, LB_ADDSTRING, 0, (LONG)item );
	}
}

/*
================
rvPropertyGrid::GetItemName

Return name of item at given index
================
*/
const char* rvPropertyGrid::GetItemName ( int index )
{
	rvPropertyGridItem* item;
	
	item = (rvPropertyGridItem*)SendMessage ( mWindow, LB_GETITEMDATA, index, 0 );
	if ( !item )
	{
		return "";
	}
	
	return item->mName;
}
	
/*
================
rvPropertyGrid::GetItemValue

Return value of item at given index
================
*/
const char* rvPropertyGrid::GetItemValue ( int index )
{
	rvPropertyGridItem* item;
	
	item = (rvPropertyGridItem*)SendMessage ( mWindow, LB_GETITEMDATA, index, 0 );
	if ( !item )
	{
		return "";
	}
	
	return item->mValue;
}

/*
================
rvPropertyGrid::WndProc

Window procedure for property grid
================
*/
LRESULT CALLBACK rvPropertyGrid::WndProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	rvPropertyGrid* grid = (rvPropertyGrid*) GetWindowLong ( hWnd, GWL_USERDATA );
	
	switch ( msg )
	{			
		case WM_SETFOCUS:
//			grid->mEditItem = -1;
			break;
			
		case WM_KEYDOWN:
		{
			NMKEY nmkey;
			nmkey.hdr.code = NM_KEYDOWN;
			nmkey.hdr.hwndFrom = grid->mWindow;
			nmkey.nVKey = wParam;
			nmkey.uFlags = HIWORD(lParam);
			nmkey.hdr.idFrom = GetWindowLong ( hWnd, GWL_ID );
			SendMessage ( GetParent ( hWnd ), WM_NOTIFY, nmkey.hdr.idFrom, (LPARAM)&nmkey );		
			break;
		}
		
		case WM_CHAR:
		{
			switch ( wParam )
			{
				case VK_RETURN:
					if ( grid->mSelectedItem >= 0 )
					{
						grid->StartEdit ( grid->mSelectedItem, (*grid->GetItemName ( grid->mSelectedItem ))?false:true);
					}
					break;
			}
			break;
		}
	
		case WM_KILLFOCUS:
			grid->mSelectedItem = -1;
			break;
					
		case WM_NOTIFY:
		{
			NMHDR* hdr;
			hdr = (NMHDR*)lParam;
			if ( hdr->idFrom == 999 )
			{
				if ( hdr->code == EN_MSGFILTER )
				{
					MSGFILTER* filter;
					filter = (MSGFILTER*)lParam;
					if ( filter->msg == WM_KEYDOWN )
					{
						switch ( filter->wParam )
						{
							case VK_RETURN:
							case VK_TAB:
								grid->FinishEdit ( );
								return 1;
								
							case VK_ESCAPE:
								grid->CancelEdit ( );
								return 1;
						}
					}							

					if ( filter->msg == WM_CHAR || filter->msg == WM_KEYUP )
					{
						switch ( filter->wParam )
						{
							case VK_RETURN:
							case VK_TAB:
							case VK_ESCAPE:
								return 1;
						}
					}
				}
			}
			break;
		}
		
		case WM_COMMAND:
			if ( lParam == (long)grid->mEdit )
			{
				if ( HIWORD(wParam) == EN_KILLFOCUS )
				{
					grid->FinishEdit ( );
					return true;
				}
			}
			break;

		case WM_LBUTTONDBLCLK:
			grid->mSelectedItem = SendMessage ( hWnd, LB_ITEMFROMPOINT, 0, lParam );
			
			// fall through

		case WM_LBUTTONDOWN:
		{
			int					item;
			rvPropertyGridItem* gitem;
			RECT				rItem;
			POINT				pt;
			
			if ( grid->mState == rvPropertyGrid::STATE_EDIT )
			{
				break;
			}
			
			item  = (short)LOWORD(SendMessage ( hWnd, LB_ITEMFROMPOINT, 0, lParam ));
			if ( item == -1 )
			{
				break;
			}
			
			gitem = (rvPropertyGridItem*)SendMessage ( hWnd, LB_GETITEMDATA, item, 0 );
			pt.x  = LOWORD(lParam);
			pt.y  = HIWORD(lParam);

			SendMessage ( hWnd, LB_GETITEMRECT, item, (LPARAM)&rItem );

			if ( !gitem->mName.Icmp ( "" ) )
			{
				rItem.right = rItem.left + grid->mSplitter - 1;
				if ( PtInRect ( &rItem, pt) )
				{
					grid->SetCurSel ( item );
					grid->StartEdit ( item, true );
				}
			}
			else if ( grid->mSelectedItem == item )
			{					
				rItem.left = rItem.left + grid->mSplitter + 1;
				if ( PtInRect ( &rItem, pt) )
				{
					grid->StartEdit ( item, false );
				}
			}
			
			if ( grid->mState == rvPropertyGrid::STATE_EDIT )
			{
				ClientToScreen ( hWnd, &pt );
				ScreenToClient ( grid->mEdit, &pt );
				SendMessage ( grid->mEdit, WM_LBUTTONDOWN, wParam, MAKELONG(pt.x,pt.y) );
				return 0;
			}
						
			break;
		}
		
		case WM_ERASEBKGND:
		{
			RECT rClient;
			GetClientRect ( hWnd, &rClient );
			FillRect ( (HDC)wParam, &rClient, GetSysColorBrush ( COLOR_3DFACE ) );
			return TRUE;
		}
			
		case WM_SETCURSOR:
		{
			POINT point;
			GetCursorPos ( &point );	
			ScreenToClient ( hWnd, &point );
			if ( point.x >= grid->mSplitter - 2 && point.x <= grid->mSplitter + 2 )
			{
				SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
				return TRUE;
			}
			break;
		}
	}
	
	return CallWindowProc ( grid->mListWndProc, hWnd, msg, wParam, lParam );
}

/*
================
rvPropertyGrid::ReflectMessage

Handle messages sent to the parent window
================
*/
bool rvPropertyGrid::ReflectMessage ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{					
		case WM_COMMAND:
		{
			if ( (HWND)lParam == mWindow )
			{
				switch ( HIWORD(wParam) )
				{
					case LBN_SELCHANGE:
						mSelectedItem = SendMessage ( mWindow, LB_GETCURSEL, 0, 0 );
						break;
				}
			}
			break;
		}
	
		case WM_DRAWITEM:
			HandleDrawItem ( wParam, lParam );
			return true;
	
		case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT* mis = (MEASUREITEMSTRUCT*) lParam;
			mis->itemHeight = 18;
			return true;
		}
	}
	
	return false;
}

/*
================
rvPropertyGrid::HandleDrawItem

Handle the draw item message
================
*/
int rvPropertyGrid::HandleDrawItem ( WPARAM wParam, LPARAM lParam )
{
	DRAWITEMSTRUCT*		dis  = (DRAWITEMSTRUCT*) lParam;
	rvPropertyGridItem* item = (rvPropertyGridItem*) dis->itemData;
	RECT				rTemp;
	HBRUSH				brush;
	
	if ( !item )
	{
		return 0;
	}

	rTemp = dis->rcItem;
	if ( mStyle & PGS_HEADERS )
	{
		brush = GetSysColorBrush ( COLOR_SCROLLBAR );
		rTemp.right = rTemp.left + 10;
		FillRect ( dis->hDC, &rTemp, brush );		
		rTemp.left = rTemp.right;
		rTemp.right = dis->rcItem.right;
	}
	
	if ( item->mType == PGIT_HEADER )
	{
		brush = GetSysColorBrush ( COLOR_SCROLLBAR );
	}
	else if ( dis->itemState & ODS_SELECTED )
	{
		brush = GetSysColorBrush ( COLOR_HIGHLIGHT );
	}
	else
	{
		brush = GetSysColorBrush ( COLOR_WINDOW );		
	}

	FillRect ( dis->hDC, &rTemp, brush );

	HPEN pen = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_SCROLLBAR ) );
	HPEN oldpen = (HPEN)SelectObject ( dis->hDC, pen );
	MoveToEx ( dis->hDC, dis->rcItem.left, dis->rcItem.top, NULL );
	LineTo ( dis->hDC, dis->rcItem.right, dis->rcItem.top );
	MoveToEx ( dis->hDC, dis->rcItem.left, dis->rcItem.bottom, NULL );
	LineTo ( dis->hDC, dis->rcItem.right, dis->rcItem.bottom);

	if ( item->mType != PGIT_HEADER )
	{
		MoveToEx ( dis->hDC, dis->rcItem.left + mSplitter, dis->rcItem.top, NULL );
		LineTo ( dis->hDC, dis->rcItem.left + mSplitter, dis->rcItem.bottom );
	}
	SelectObject ( dis->hDC, oldpen );
	DeleteObject ( pen );			

	int colorIndex = ( (dis->itemState & ODS_SELECTED ) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT );
	SetTextColor ( dis->hDC, GetSysColor ( colorIndex ) );
	SetBkMode ( dis->hDC, TRANSPARENT );
	SetBkColor ( dis->hDC, GetSysColor ( COLOR_3DFACE ) );

	RECT rText;
	rText = rTemp;
	rText.right = rText.left + mSplitter;
	rText.left += 2;

	DrawText ( dis->hDC, item->mName, item->mName.Length(), &rText, DT_LEFT|DT_VCENTER|DT_SINGLELINE );
	
	rText.left = dis->rcItem.left + mSplitter + 2;
	rText.right = dis->rcItem.right;
	DrawText ( dis->hDC, item->mValue, item->mValue.Length(), &rText, DT_LEFT|DT_VCENTER|DT_SINGLELINE );
	
	return 0;
}
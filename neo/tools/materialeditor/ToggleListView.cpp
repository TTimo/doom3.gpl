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

#include "ToggleListView.h"

#define TOGGLELIST_ITEMHEIGHT 22
#define TEXT_OFFSET 6


IMPLEMENT_DYNCREATE(ToggleListView, CListView)

BEGIN_MESSAGE_MAP(ToggleListView, CListView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MEASUREITEM_REFLECT()
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
END_MESSAGE_MAP()


/**
* Protected constructor used by dynamic creation.
*/
ToggleListView::ToggleListView() {
	onIcon = NULL;
	offIcon = NULL;
	disabledIcon = NULL;
}

/**
* Destructor.
*/
ToggleListView::~ToggleListView() {
}


/**
* Sets the tree icons to dispay for each of the three states. Sets the 
* icons to display for each of the three states. The values passed in 
* are the resource name that can be generated using MAKEINTRESOUCE. If 
* the value passed in is NULL then an icon will not be drawn for that
* state.
* @param disabled The icon to draw when the state is TOGGLE_STATE_DISABLED.
* @param on The icon to draw when the state is TOGGLE_STATE_ON.
* @param off The icon to draw when the state is TOGGLE_STATE_OFF.
*/
void ToggleListView::SetToggleIcons(LPCSTR disabled, LPCSTR on, LPCSTR off) {
	if(on) {
		onIcon = (HICON)LoadImage ( AfxGetInstanceHandle(), on, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_LOADMAP3DCOLORS );
	} else {
		onIcon = NULL;
	}

	if(off) {
		offIcon = (HICON)LoadImage ( AfxGetInstanceHandle(), off, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_LOADMAP3DCOLORS );
	} else {
		offIcon = NULL;
	}

	if(disabled) {
		disabledIcon = (HICON)LoadImage ( AfxGetInstanceHandle(), disabled, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_LOADMAP3DCOLORS );
	} else {
		disabledIcon = NULL;
	}
}

/**
* Sets the state of an item in the list.
* @param index Index of the item whose state should be changed.
* @param toggleState The state to set
* @param notify Determines if the notification method OnStateChanged should
* be called. OnStateChanged will also not be called if the state has not changed.
*/
void ToggleListView::SetToggleState(int index, int toggleState, bool notify) {
	CListCtrl& list = GetListCtrl();
	assert(index >= 0 && index < list.GetItemCount());

	int oldState = GetToggleState(index);
	list.SetItemData(index, toggleState);

	if(notify && oldState != toggleState)
		OnStateChanged(index, toggleState);
}

/**
* Gets the state of an item in the list
* @param index Index of the item of which to retreive the state.
*/
int ToggleListView::GetToggleState(int index) {
	CListCtrl& list = GetListCtrl();
	assert(index >= 0 && index < list.GetItemCount());

	DWORD data = list.GetItemData(index);
	return data;
}

/**
* Called as the window is being created and initializes icons and window styles
*/
int ToggleListView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CListCtrl& list = GetListCtrl();
	
	list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	//Turn off the horizontal scroll bar
	//Todo: Figure out why the damn scroll bar pops up
	list.ModifyStyle(WS_HSCROLL, 0L);
	
	
	//Insert the one column
	LVCOLUMN col;
	col.mask = 0;
	list.InsertColumn(0, &col);

	SetToggleIcons();
	
	return 0;
}

/**
* Called when the window is being resized.
*/
void ToggleListView::OnSize(UINT nType, int cx, int cy) {
	CListView::OnSize(nType, cx, cy);

	CListCtrl& list = GetListCtrl();
	list.SetColumnWidth(0, cx-1);
}

/**
* Returns the size of each item in the toggle list.
*/
void ToggleListView::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) {
	lpMeasureItemStruct->itemHeight = TOGGLELIST_ITEMHEIGHT;	
}

/**
* Toggles the state of an item when the user clicks in the window.
*/
void ToggleListView::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult) {
	CListCtrl& list = GetListCtrl();

	DWORD dwpos = GetMessagePos(); 

	LVHITTESTINFO info;
	info.pt.x = LOWORD(dwpos);
	info.pt.y = HIWORD(dwpos);		

	::MapWindowPoints(HWND_DESKTOP, pNMHDR->hwndFrom, &info.pt, 1);      

	int index = list.HitTest(&info);
	if ( index != -1 ) {
		int toggleState = GetToggleState(index);
		if(toggleState != TOGGLE_STATE_DISABLED) {

			RECT	rItem;
			list.GetItemRect(index, &rItem, LVIR_BOUNDS);

			if ( info.pt.x < TOGGLELIST_ITEMHEIGHT ) {
				if(toggleState == TOGGLE_STATE_ON) {
					SetToggleState(index, TOGGLE_STATE_OFF, true);
				} else {
					SetToggleState(index, TOGGLE_STATE_ON, true);
				}
			}												
		}
	}
	*pResult = 0;
}

/**
* Sets some window styles before the window is created.
*/
BOOL ToggleListView::PreCreateWindow(CREATESTRUCT& cs) {
	//Set the required style for the toggle view
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS;

	return CListView::PreCreateWindow(cs);
}

/**
* Responsible for drawing each list item.
*/
void ToggleListView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {

	CListCtrl& ListCtrl=GetListCtrl();
	int nItem = lpDrawItemStruct->itemID;
	
	// get item data
	LV_ITEM lvi;
	_TCHAR szBuff[MAX_PATH];
	
	memset(&lvi, 0, sizeof(LV_ITEM));
	lvi.mask = LVIF_TEXT;
	lvi.iItem = nItem;
	lvi.pszText = szBuff;
	lvi.cchTextMax = sizeof(szBuff);
	ListCtrl.GetItem(&lvi);

	RECT rDraw;
	
	
	CopyRect ( &rDraw, &lpDrawItemStruct->rcItem );
	rDraw.right = rDraw.left + TOGGLELIST_ITEMHEIGHT;
	rDraw.top ++;

	rDraw.right ++;
	FrameRect ( lpDrawItemStruct->hDC, &rDraw, (HBRUSH)GetStockObject ( BLACK_BRUSH ) );
	rDraw.right --;

	FillRect ( lpDrawItemStruct->hDC, &rDraw, GetSysColorBrush ( COLOR_3DFACE ) );

	Draw3dRect ( lpDrawItemStruct->hDC, &rDraw, GetSysColorBrush ( COLOR_3DHILIGHT ), GetSysColorBrush ( COLOR_3DSHADOW ) );

	InflateRect ( &rDraw, -3, -3 );
	Draw3dRect ( lpDrawItemStruct->hDC, &rDraw, GetSysColorBrush ( COLOR_3DSHADOW ), GetSysColorBrush ( COLOR_3DHILIGHT ) );

	switch(GetToggleState(lvi.iItem)) {
		case TOGGLE_STATE_DISABLED:
			if(disabledIcon) {
				DrawIconEx ( lpDrawItemStruct->hDC, rDraw.left, rDraw.top, disabledIcon, 16, 16,0, NULL, DI_NORMAL );
			}
			break;
		case TOGGLE_STATE_ON:
			if(onIcon) {
				DrawIconEx ( lpDrawItemStruct->hDC, rDraw.left, rDraw.top, onIcon, 16, 16,0, NULL, DI_NORMAL );
			}
			break;
		case TOGGLE_STATE_OFF:
			if(offIcon) {
				DrawIconEx ( lpDrawItemStruct->hDC, rDraw.left, rDraw.top, offIcon, 16, 16,0, NULL, DI_NORMAL );
			}
			break;
	};
	
	CopyRect ( &rDraw, &lpDrawItemStruct->rcItem );
	rDraw.left += TOGGLELIST_ITEMHEIGHT;
	rDraw.left += 1;

	if ( lpDrawItemStruct->itemState & ODS_SELECTED ) {
		FillRect ( lpDrawItemStruct->hDC, &rDraw, GetSysColorBrush ( COLOR_HIGHLIGHT ) );			
	} else {
		FillRect ( lpDrawItemStruct->hDC, &rDraw, GetSysColorBrush ( COLOR_WINDOW ) ); 
	}

	rDraw.left += TEXT_OFFSET;

	int colorIndex = ( (lpDrawItemStruct->itemState & ODS_SELECTED ) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT );
	SetTextColor ( lpDrawItemStruct->hDC, GetSysColor ( colorIndex ) );
	DrawText ( lpDrawItemStruct->hDC, szBuff, strlen(szBuff), &rDraw, DT_LEFT|DT_VCENTER|DT_SINGLELINE );

}


/**
* Draws a 3d rectangle using the given brushes this code was taken from the gui editor
*/
void ToggleListView::Draw3dRect (HDC hDC, RECT* rect, HBRUSH topLeft, HBRUSH bottomRight) {
	RECT rOut;

	SetRect ( &rOut, rect->left, rect->top, rect->right - 1, rect->top + 1 );
	FillRect ( hDC,&rOut, topLeft ); 

	SetRect ( &rOut, rect->left, rect->top, rect->left + 1, rect->bottom );
	FillRect( hDC,&rOut, topLeft ); 

	SetRect ( &rOut, rect->right, rect->top, rect->right -1, rect->bottom  );
	FillRect( hDC,&rOut, bottomRight ); 

	SetRect ( &rOut, rect->left, rect->bottom, rect->right, rect->bottom - 1 );
	FillRect( hDC,&rOut, bottomRight ); 
}





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

#include "GEApp.h"

#define	GENAV_ITEMHEIGHT	22

rvGENavigator::rvGENavigator ( )
{
	mWnd = NULL;
	mWorkspace = NULL;
	mVisibleIcon = NULL;
	mScriptsIcon = NULL;
	mVisibleIconDisabled = NULL;
	mScriptsLightIcon = NULL;
}

/*
================
rvGENavigator::Create

Creates the navigator window
================
*/
bool rvGENavigator::Create ( HWND parent, bool visible )
{
	WNDCLASSEX wndClass;
	memset ( &wndClass, 0, sizeof(wndClass) );
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpszClassName = "GUIEDITOR_NAVIGATOR_CLASS";		
	wndClass.lpfnWndProc = rvGENavigator::WndProc;
	wndClass.hbrBackground = (HBRUSH)GetStockObject( LTGRAY_BRUSH );; 
	wndClass.hCursor       = LoadCursor((HINSTANCE) NULL, IDC_ARROW); 
	wndClass.lpszMenuName  = NULL;
	wndClass.hInstance     = win32.hInstance; 
	RegisterClassEx ( &wndClass );

	mVisibleIcon = (HICON)LoadImage ( win32.hInstance, MAKEINTRESOURCE(IDI_GUIED_NAV_VISIBLE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_LOADMAP3DCOLORS );
	mScriptsIcon = (HICON)LoadImage ( win32.hInstance, MAKEINTRESOURCE(IDI_GUIED_NAV_SCRIPTS), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
	mScriptsLightIcon = (HICON)LoadImage ( win32.hInstance, MAKEINTRESOURCE(IDI_GUIED_NAV_SCRIPTSHI), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );	
	mVisibleIconDisabled = (HICON)LoadImage ( win32.hInstance, MAKEINTRESOURCE(IDI_GUIED_NAV_VISIBLEDISABLED), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_LOADMAP3DCOLORS );
	mExpandIcon = (HICON)LoadImage ( win32.hInstance, MAKEINTRESOURCE(IDI_GUIED_NAV_EXPAND), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_LOADMAP3DCOLORS );
	mCollapseIcon = (HICON)LoadImage ( win32.hInstance, MAKEINTRESOURCE(IDI_GUIED_NAV_COLLAPSE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_LOADMAP3DCOLORS );

	mWnd = CreateWindowEx ( WS_EX_TOOLWINDOW, 
							"GUIEDITOR_NAVIGATOR_CLASS", 
							"Navigator", 
							WS_SYSMENU|WS_THICKFRAME|WS_CAPTION|WS_POPUP|WS_OVERLAPPED|WS_BORDER|WS_CLIPSIBLINGS|WS_CHILD, 
							0, 0, 200,300,
							parent, 
							NULL, 
							win32.hInstance, 
							this );
							
	if ( !mWnd )
	{
		return false;
	}

	if ( !gApp.GetOptions().GetWindowPlacement ( "navigator", mWnd ) )
	{
		RECT rParent;
		RECT rNav;
		
		GetWindowRect ( parent, &rParent );
		GetWindowRect ( mWnd, &rNav );			
		SetWindowPos ( mWnd, NULL,
					rParent.right - 10 - (rNav.right-rNav.left),
					rParent.bottom - 10 - (rNav.bottom-rNav.top),
					0,0,
					SWP_NOZORDER|SWP_NOSIZE );
	}
				 	
	Show ( visible );
	
	return true;							
}

/*
================
Draw3dRect

Draws a 3d rectangle using the given brushes
================
*/
void Draw3dRect ( HDC hDC, RECT* rect, HBRUSH topLeft, HBRUSH bottomRight )
{
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

/*
================
rvGENavigator::WndProc

Window Procedure 
================
*/
LRESULT CALLBACK rvGENavigator::WndProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	rvGENavigator* nav = (rvGENavigator*) GetWindowLong ( hWnd, GWL_USERDATA );

	switch ( msg )
	{
		case WM_INITMENUPOPUP:
			return SendMessage ( gApp.GetMDIFrame ( ), msg, wParam, lParam );
	
		case WM_ACTIVATE:
			common->ActivateTool( LOWORD( wParam ) != WA_INACTIVE );
			break;

		case WM_ERASEBKGND:
			return TRUE;
		
		case WM_DESTROY:
			gApp.GetOptions().SetWindowPlacement ( "navigator", hWnd );
			break;
		
		case WM_CLOSE:		
			gApp.GetOptions().SetNavigatorVisible ( false );
			nav->Show ( false );
			return 0;

		case WM_DRAWITEM:
		{		
			DRAWITEMSTRUCT*	dis = (DRAWITEMSTRUCT*) lParam;
			idWindow*		window = (idWindow*)dis->itemData;
			
			if ( window )
			{
				rvGEWindowWrapper*	wrapper	= rvGEWindowWrapper::GetWrapper ( window );
				idStr				name    = window->GetName();
				RECT				rDraw;
				float				offset;
				bool				disabled;

				idWindow* parent = window;
				offset = 1;
				disabled = false;
				while ( parent = parent->GetParent ( ) )
				{
					if ( rvGEWindowWrapper::GetWrapper ( parent )->IsHidden ( ) )
					{
						disabled = true;
					}
					
					offset += 10;
				}
				
				CopyRect ( &rDraw, &dis->rcItem );
				rDraw.right = rDraw.left + GENAV_ITEMHEIGHT;
				rDraw.top ++;

				rDraw.right ++;
				FrameRect ( dis->hDC, &rDraw, (HBRUSH)GetStockObject ( BLACK_BRUSH ) );
				rDraw.right --;
			
				FillRect ( dis->hDC, &rDraw, GetSysColorBrush ( COLOR_3DFACE ) );

				Draw3dRect ( dis->hDC, &rDraw, GetSysColorBrush ( COLOR_3DHILIGHT ), GetSysColorBrush ( COLOR_3DSHADOW ) );

				InflateRect ( &rDraw, -3, -3 );
				Draw3dRect ( dis->hDC, &rDraw, GetSysColorBrush ( COLOR_3DSHADOW ), GetSysColorBrush ( COLOR_3DHILIGHT ) );
				
				if ( !wrapper->IsHidden ( ) )
				{
					DrawIconEx ( dis->hDC, rDraw.left, rDraw.top, disabled?nav->mVisibleIconDisabled:nav->mVisibleIcon, 16, 16,0, NULL, DI_NORMAL );
				}

				CopyRect ( &rDraw, &dis->rcItem );
				rDraw.left += GENAV_ITEMHEIGHT;
				rDraw.left += 1;
			
				if ( dis->itemState & ODS_SELECTED )
				{
					FillRect ( dis->hDC, &rDraw, GetSysColorBrush ( COLOR_HIGHLIGHT ) );			
				}
				else
				{
					FillRect ( dis->hDC, &rDraw, GetSysColorBrush ( COLOR_WINDOW ) );
				}

				if ( wrapper->CanHaveChildren ( ) && window->GetChildCount ( ) )
				{
					if ( wrapper->IsExpanded ( ) )
					{
						DrawIconEx ( dis->hDC, rDraw.left + offset, rDraw.top + 3, nav->mCollapseIcon, 16, 16,0, NULL, DI_NORMAL );								
					}
					else
					{
						DrawIconEx ( dis->hDC, rDraw.left + offset, rDraw.top + 3, nav->mExpandIcon, 16, 16,0, NULL, DI_NORMAL );								
					}
				}
			
				HPEN pen = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_3DSHADOW ) );
				HPEN oldpen = (HPEN)SelectObject ( dis->hDC, pen );
				MoveToEx ( dis->hDC, rDraw.left, dis->rcItem.top, NULL );
				LineTo ( dis->hDC, dis->rcItem.right, dis->rcItem.top );
				MoveToEx ( dis->hDC, rDraw.left, dis->rcItem.bottom, NULL );
				LineTo ( dis->hDC, dis->rcItem.right, dis->rcItem.bottom);
				SelectObject ( dis->hDC, oldpen );
				DeleteObject ( pen );			

				rDraw.left += offset;
				rDraw.left += 20;
											
				int colorIndex = ( (dis->itemState & ODS_SELECTED ) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT );
				SetTextColor ( dis->hDC, GetSysColor ( colorIndex ) );
				DrawText ( dis->hDC, name, name.Length(), &rDraw, DT_LEFT|DT_VCENTER|DT_SINGLELINE );
							
				if ( wrapper->GetVariableDict().GetNumKeyVals ( ) || wrapper->GetScriptDict().GetNumKeyVals ( ) )
				{
					DrawIconEx ( dis->hDC, dis->rcItem.right - 16, (dis->rcItem.bottom+dis->rcItem.top)/2-6, (dis->itemState & ODS_SELECTED)?nav->mScriptsLightIcon:nav->mScriptsIcon, 13, 13,0, NULL, DI_NORMAL );
				}
			}
			
			break;
		}
	
		case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT* mis = (MEASUREITEMSTRUCT*) lParam;
			mis->itemHeight = 22;
			break;
		}
	
		case WM_CREATE:
		{
			LPCREATESTRUCT	cs;
			LVCOLUMN		col;
			
			// Attach the class to the window first
			cs = (LPCREATESTRUCT) lParam;
			nav = (rvGENavigator*) cs->lpCreateParams;
			SetWindowLong ( hWnd, GWL_USERDATA, (LONG)nav );

			// Create the List view
			nav->mTree = CreateWindowEx ( 0, "SysListView32", "", WS_VSCROLL|WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_OWNERDRAWFIXED|LVS_NOCOLUMNHEADER|LVS_SHOWSELALWAYS, 0, 0, 0, 0, hWnd, (HMENU)IDC_GUIED_WINDOWTREE, win32.hInstance, 0 );
			ListView_SetExtendedListViewStyle ( nav->mTree, LVS_EX_FULLROWSELECT );
			ListView_SetBkColor ( nav->mTree, GetSysColor ( COLOR_3DFACE ) );
			ListView_SetTextBkColor ( nav->mTree, GetSysColor ( COLOR_3DFACE ) );
			nav->mListWndProc = (WNDPROC)GetWindowLong ( nav->mTree, GWL_WNDPROC );
			SetWindowLong ( nav->mTree, GWL_USERDATA, (LONG)nav );
			SetWindowLong ( nav->mTree, GWL_WNDPROC, (LONG)ListWndProc );

			// Insert the only column
			col.mask = 0;	
			ListView_InsertColumn ( nav->mTree, 0, &col );
							
			break;
		}
	
		case WM_SIZE:
		{
			RECT rClient;
			MoveWindow ( nav->mTree, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE );
			GetClientRect ( nav->mTree, &rClient );		
			ListView_SetColumnWidth ( nav->mTree, 0, rClient.right-rClient.left-1 );
			break;
		}

		case WM_NCACTIVATE:
			return gApp.ToolWindowActivate ( gApp.GetMDIFrame(), msg, wParam, lParam );
				
		case WM_NOTIFY:
		{
			LPNMHDR nh;
			
			nh = (LPNMHDR) lParam;
			
			switch ( nh->code )
			{				
				case NM_CLICK:
				case NM_DBLCLK:
				{
					DWORD dwpos = GetMessagePos(); 
					LVHITTESTINFO info;
					info.pt.x = LOWORD(dwpos);
					info.pt.y = HIWORD(dwpos);		
					MapWindowPoints(HWND_DESKTOP, nh->hwndFrom, &info.pt, 1);      
					int index = ListView_HitTest ( nav->mTree, &info );
					if ( index != -1 )
					{
						RECT	rItem;
						int		offset;
						ListView_GetItemRect ( nav->mTree, index, &rItem, LVIR_BOUNDS );
						LVITEM item;
						item.mask = LVIF_PARAM;
						item.iItem = index;
						ListView_GetItem ( nav->mTree, &item );
						idWindow* window = (idWindow*)item.lParam;
						rvGEWindowWrapper* wrapper = rvGEWindowWrapper::GetWrapper(window);

						offset = wrapper->GetDepth ( ) * 10 + 1;

						if ( info.pt.x < GENAV_ITEMHEIGHT )
						{
							if ( !rvGEWindowWrapper::GetWrapper(window)->IsHidden ( ) )
							{
								nav->mWorkspace->HideWindow ( window );
							}
							else
							{
								nav->mWorkspace->UnhideWindow ( window );
							}
						}						
						else if ( info.pt.x > GENAV_ITEMHEIGHT + offset && info.pt.x < GENAV_ITEMHEIGHT + offset + 16 )
						{
							if ( wrapper->CanHaveChildren ( ) && window->GetChildCount ( ) )
							{
								if ( wrapper->IsExpanded ( ) )
								{
									wrapper->Collapse ( );
									nav->Update ( );
								}
								else
								{
									wrapper->Expand ( );
									nav->Update ( );
								}
							}
						}
						else if ( nh->code == NM_DBLCLK )
						{
							SendMessage ( gApp.GetMDIFrame ( ), WM_COMMAND, MAKELONG(ID_GUIED_ITEM_PROPERTIES,0), 0 );
						}											
					}
					
					break;
				}
				
				case NM_RCLICK:
				{
					DWORD dwpos = GetMessagePos(); 
					LVHITTESTINFO info;
					info.pt.x = LOWORD(dwpos);
					info.pt.y = HIWORD(dwpos);		
					MapWindowPoints(HWND_DESKTOP, nh->hwndFrom, &info.pt, 1);      
					int index = ListView_HitTest ( nav->mTree, &info );
					
					if ( index != -1 )
					{				
						ClientToScreen ( hWnd, &info.pt );
						HMENU menu = GetSubMenu ( LoadMenu ( gApp.GetInstance(), MAKEINTRESOURCE(IDR_GUIED_ITEM_POPUP) ), 0 );
						TrackPopupMenu ( menu, TPM_RIGHTBUTTON|TPM_LEFTALIGN, info.pt.x, info.pt.y, 0, gApp.GetMDIFrame ( ), NULL );
						DestroyMenu ( menu );
					}
					
					break;					
				}
								
				case LVN_ITEMCHANGED:
				{
					NMLISTVIEW* nml = (NMLISTVIEW*) nh;
					if ( (nml->uNewState & LVIS_SELECTED) != (nml->uOldState & LVIS_SELECTED) )
					{
						LVITEM item;
						item.iItem = nml->iItem;
						item.mask = LVIF_PARAM;
						ListView_GetItem ( nav->mTree, &item );
						
						if ( nml->uNewState & LVIS_SELECTED )
						{
							nav->mWorkspace->GetSelectionMgr().Add ( (idWindow*)item.lParam, false );
						}
						else
						{
							nav->mWorkspace->GetSelectionMgr().Remove ( (idWindow*)item.lParam );
						}	
					}
					break;
				}
			}
			
			break;
		}	
	}

	return DefWindowProc ( hWnd, msg, wParam, lParam );
}

/*
================
rvGENavigator::ListWndProc

Window Procedure for the embedded list control 
================
*/
LRESULT CALLBACK rvGENavigator::ListWndProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	rvGENavigator* nav = (rvGENavigator*) GetWindowLong ( hWnd, GWL_USERDATA );
	assert ( nav );
	
	switch ( msg )
	{
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
			if ( nav->mWorkspace )
			{
				return SendMessage ( nav->mWorkspace->GetWindow(), msg, wParam, lParam );
			}
			break;
	}
	
	return CallWindowProc ( nav->mListWndProc, hWnd, msg, wParam, lParam );
}

/*
================
rvGENavigator::AddWindow

Adds a new window to the navigator
================
*/
void rvGENavigator::AddWindow ( idWindow* window )
{
	int					index;
	LVITEM				item;
	rvGEWindowWrapper*	wrapper;
	
	wrapper = rvGEWindowWrapper::GetWrapper ( window );
		
	// Dont add deleted windows
	if ( !wrapper || wrapper->IsDeleted ( ) )
	{
		return;
	}
	
	// Insert the window into the tree
	ZeroMemory ( &item, sizeof(item) );
	item.mask = LVIF_PARAM|LVIF_STATE|LVIF_IMAGE;
	item.iItem = ListView_GetItemCount ( mTree );	
	item.lParam = (LONG) window;
	item.iImage = 0;
	item.state = rvGEWindowWrapper::GetWrapper(window)->IsSelected ()? LVIS_SELECTED:0;
	item.stateMask = LVIS_SELECTED;
	ListView_InsertItem ( mTree, &item );
	
	if ( item.state & LVIS_SELECTED )
	{
		ListView_EnsureVisible ( mTree, item.iItem, false );
	}
	
	// Dont continue if not expanded.
	if ( !wrapper->IsExpanded ( ) )
	{
		return;
	}
	
	// Insert all the child windows into the tree
	for ( index = 0; index < wrapper->GetChildCount(); index ++ )
	{
		AddWindow ( wrapper->GetChild(index) );
	}
}

/*
================
rvGENavigator::SetWorkspace

Sets a new workspace for the navigator window
================
*/
void rvGENavigator::SetWorkspace ( rvGEWorkspace* workspace )
{
	mWorkspace = workspace;

	Update ( );	
}

/*
================
rvGENavigator::Update

Updates the contents of the navigator window from the current workspace
================
*/
void rvGENavigator::Update ( void )
{
	// Clear the list first
	ListView_DeleteAllItems ( mTree );

	// Add starting with the desktop window
	if ( mWorkspace )
	{
		AddWindow ( mWorkspace->GetInterface ( )->GetDesktop ( ) );		
	}	

	// For some reason the horizontal scrollbar wants to show up initially after an update
	// so this forces it not to
	RECT rClient;
	GetClientRect ( mTree, &rClient );		
	ListView_SetColumnWidth ( mTree, 0, rClient.right-rClient.left-1 );
}

/*
================
rvGENavigator::UpdateSelection

Updates the currently selected items
================
*/
void rvGENavigator::UpdateSelections ( void )
{
	int count = ListView_GetItemCount ( mTree );
	int i;
	
	for ( i = 0; i < count; i++  )
	{
		LVITEM				item;
		idWindow*			window;
		rvGEWindowWrapper*	wrapper;
		
		item.iItem = i;
		item.mask = LVIF_PARAM;
		ListView_GetItem ( mTree, &item );
		window = (idWindow*) item.lParam;		
		wrapper = rvGEWindowWrapper::GetWrapper ( window );

		ListView_SetItemState ( mTree, i, wrapper->IsSelected ( )?LVIS_SELECTED:0, LVIS_SELECTED );
		
		if ( wrapper->IsSelected ( ) )
		{
			ListView_EnsureVisible ( mTree, i, false );
		}
	}	
}

/*
================
rvGENavigator::Refresh

Repaints the navigator window
================
*/
void rvGENavigator::Refresh ( void )
{
	InvalidateRect ( mTree, NULL, FALSE );
//	UpdateWindow ( mTree );
}

/*
================
rvGENavigator::Show

Shows and hides the navigator window
================
*/
void rvGENavigator::Show ( bool visible )
{
	gApp.GetOptions().SetNavigatorVisible ( visible );
	ShowWindow ( mWnd, visible?SW_SHOW:SW_HIDE );
}


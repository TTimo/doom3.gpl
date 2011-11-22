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
#include "../Common/OpenFileDialog.h"
#include "DebuggerQuickWatchDlg.h"
#include "DebuggerFindDlg.h"

#define DEBUGGERWINDOWCLASS		"QUAKE4_DEBUGGER_WINDOW"
#define ID_DBG_WINDOWMIN		18900
#define ID_DBG_WINDOWMAX		19900

#define	IDC_DBG_SCRIPT			31000
#define	IDC_DBG_OUTPUT			31001
#define IDC_DBG_SPLITTER		31002
#define IDC_DBG_TABS			31003
#define IDC_DBG_BORDER			31004
#define IDC_DBG_CONSOLE			31005
#define IDC_DBG_CALLSTACK		31006
#define IDC_DBG_WATCH			31007
#define IDC_DBG_THREADS			31008
#define IDC_DBG_TOOLBAR			31009

#define ID_DBG_FILE_MRU1		10000

/*
================
rvDebuggerWindow::rvDebuggerWindow

Constructor
================
*/
rvDebuggerWindow::rvDebuggerWindow ( )
{
	mWnd		   = NULL;
	mWndScript	   = NULL;
	mInstance	   = NULL;
	mZoomScaleNum  = 0;
	mZoomScaleDem  = 0;
	mWindowMenuPos = 0;
	mActiveScript  = 0;
	mSplitterDrag  = false;
	mLastActiveScript = -1;
	mCurrentStackDepth = 0;
	mRecentFileInsertPos = 0;
	mRecentFileMenu = NULL;
	mClient = NULL;
}

/*
================
rvDebuggerWindow::~rvDebuggerWindow

Destructor
================
*/
rvDebuggerWindow::~rvDebuggerWindow ( )
{
	int i;
	
	if ( mWnd )
	{
		DestroyWindow ( mWnd );
	}
	
	if ( mImageList )
	{
		ImageList_Destroy ( mImageList );
	}
	
	for ( i = 0; i < mScripts.Num (); i ++ )
	{
		delete mScripts[i];
	}
}

/*
================
rvDebuggerWindow::RegisterClass

Registers the window class used by the debugger window.  This is called when
the window is created.
================
*/
bool rvDebuggerWindow::RegisterClass ( void )
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= mInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_DBG_MAIN);
	wcex.lpszClassName	= DEBUGGERWINDOWCLASS;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex) ? true : false;
}

/*
================
rvDebuggerWindow::Create

Creates the debugger window
================
*/
bool rvDebuggerWindow::Create ( HINSTANCE instance )
{
	mInstance = instance;

	if ( !RegisterClass ( ) )
	{
		return false;
	}

	// Cache the client pointer for ease of use
	mClient = &gDebuggerApp.GetClient();

	// Create the debugger window
	mWnd = CreateWindow( DEBUGGERWINDOWCLASS, "", 
						 WS_OVERLAPPEDWINDOW,
						 CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, mInstance, this);

	if ( !mWnd )
	{
		return false;
	}

	// Determine where the window names will be added in the menus
	mWindowMenu    = GetSubMenu ( GetMenu ( mWnd ), 2 );
	mWindowMenuPos = GetMenuItemCount ( mWindowMenu );

	UpdateTitle ( );
	
	Printf ( "Quake 4 Script Debugger v0.1\n\n" );
	
	ShowWindow ( mWnd, SW_SHOW );
	UpdateWindow ( mWnd );	

	return true;
}	

/*
================
rvDebuggerWindow::ScriptWordBreakProc

Determines where word breaks are in the script window.  This is used for determining
the word that someone is over with their mouse cursor.  Since the default windows one
doesnt understand the delimiters of the scripting language it had to be overridden.
================
*/
int CALLBACK rvDebuggerWindow::ScriptWordBreakProc (LPTSTR text, int current, int max, int action )
{
	static TCHAR delimiters[]=TEXT("!@#$%^&*()-+=[]{}|\\;:'\"/,.<>? \t\r\n") ;

	switch ( action ) 
	{
		default:
			break;
	
        case WB_ISDELIMITER:
			return _tcschr ( delimiters, *(text + current * 2) ) ? TRUE : FALSE;

		case WB_MOVEWORDLEFT:
		case WB_LEFT:

			current--;

			// Run as long as the current index is valid.
			while ( current > 0 )
			{
				// If we hit a delimiter then return the index + 1 since
				// it was the last character we were on that was valid
				if ( _tcschr ( delimiters, *(text + current * 2) ) )
				{
					return current + 1;
				}
				
				// Going backwards
				current--;
			}

			return current;

		case WB_MOVEWORDRIGHT:
        case WB_RIGHT:
        
			// If we are already on a delimiter then just return the current index       
			if ( _tcschr ( delimiters, *(text + current * 2) ) )
			{
				return current;
			}

			// Run until we hit the end of the control
			while ( current < max )
			{
				// If we found a delimiter then return the index we are on
				if ( _tcschr ( delimiters, *(text + current * 2) ) )
				{
					return current;
				}

				current++;
			}

			return current;
    }
    
    return 0;
  }

LRESULT CALLBACK rvDebuggerWindow::ScriptWndProc ( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	static int		  lastStart = -1;
	static int		  lastEnd   = -1;
	rvDebuggerWindow* window    = (rvDebuggerWindow*)GetWindowLong ( wnd, GWL_USERDATA );
	WNDPROC			  wndproc   = window->mOldScriptProc;
	
	switch ( msg )
	{	
		case WM_RBUTTONUP:
			return SendMessage ( wnd, WM_LBUTTONUP, wparam, lparam );

		case WM_RBUTTONDOWN:
		{
			POINT point = { LOWORD(lparam), HIWORD(lparam) };	
			HMENU menu;
			SendMessage ( wnd, WM_LBUTTONDOWN, wparam, lparam );
			menu = LoadMenu ( window->mInstance, MAKEINTRESOURCE(IDR_DBG_SCRIPT_POPUP) );
			ClientToScreen ( wnd, &point );
			TrackPopupMenu ( GetSubMenu( menu, 0 ), TPM_RIGHTBUTTON|TPM_LEFTALIGN, point.x, point.y, 0, window->mWnd, NULL );
			DestroyMenu ( menu );
			return 0;
		}
	
		case WM_MOUSEMOVE:
		{
			// Figure out the start and end of the mouse is over
			POINTL pos   = {LOWORD(lparam),HIWORD(lparam)};
			int    c     = SendMessage ( wnd, EM_CHARFROMPOS, 0, (WPARAM)&pos );
			int    start = SendMessage ( wnd, EM_FINDWORDBREAK, WB_LEFT, c );
			int    end   = SendMessage ( wnd, EM_FINDWORDBREAK, WB_RIGHT, c );
				
			// If the start and the end of the word we are over havent changed
			// then the word hasnt changed so no need to re-setup the tool tip
			if ( lastStart == start && lastEnd == end )
			{
				break;
			}
				
			// Save the current start and end for the next mouse move
			lastStart = start;
			lastEnd   = end;

			// Get rid of the last tool tip if there is one
			if ( window->mTooltipVar.Length() )
			{
				TOOLINFO ti;
				ti.cbSize = sizeof(TOOLINFO);
				ti.hwnd   = wnd;
				ti.uId    = 0;
				SendMessage ( window->mWndToolTips, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO) &ti );
				window->mTooltipVar.Empty ( );
			}
			
			// If there is no word then ignore it	
			if ( start == end )
			{
				break;
			}
					
			TEXTRANGE	range;
			TOOLINFO	ti;

			// grab the actual word from the edit control
			char* temp = new char[end-start+10];
			range.chrg.cpMin = start;
			range.chrg.cpMax = end;
			range.lpstrText  = temp;
			SendMessage ( wnd, EM_GETTEXTRANGE, 0, (LPARAM) &range );
			window->mTooltipVar = temp;
			delete[] temp;

			// Request the variable's value from the debugger server
			window->mClient->InspectVariable ( window->mTooltipVar.c_str(), window->mCurrentStackDepth );

			// Prepare to add the new tooltip
			ti.cbSize   = sizeof(TOOLINFO);
			ti.uFlags   = TTF_SUBCLASS;
			ti.hwnd     = wnd;
			ti.hinst    = (HINSTANCE)GetModuleHandle(NULL);
			ti.uId      = 0;
			ti.lpszText = (LPSTR)window->mTooltipVar.c_str();

			// Calculate the bounding box around the word we are over.  We do this
			// by getting to the top left from the start character and the right side
			// from the end character.  The bottom is the top from the start character
			// plus the height of one line
			SendMessage ( wnd, EM_POSFROMCHAR, (WPARAM)&pos, start );
			ti.rect.left = pos.x;					
			ti.rect.top  = pos.y;
			SendMessage ( wnd, EM_POSFROMCHAR, (WPARAM)&pos, end );
			ti.rect.right = pos.x;
			SendMessage ( wnd, EM_POSFROMCHAR, (WPARAM)&pos, SendMessage ( wnd, EM_LINEINDEX, 0, 0 ) );
			ti.rect.bottom = ti.rect.top - pos.y;
			SendMessage ( wnd, EM_POSFROMCHAR, (WPARAM)&pos, SendMessage ( wnd, EM_LINEINDEX, 1, 0 ) );
			ti.rect.bottom = ti.rect.bottom + pos.y;

			// Add the new tool tip to the control
			SendMessage ( window->mWndToolTips, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti );
			SendMessage ( window->mWndToolTips, TTM_UPDATE, 0, 0 );
	
			break;			
		}
	}
	
	return CallWindowProc ( wndproc, wnd, msg, wparam, lparam );
}

LRESULT CALLBACK rvDebuggerWindow::MarginWndProc ( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	rvDebuggerWindow* window = (rvDebuggerWindow*) GetWindowLong ( wnd, GWL_USERDATA );
	
	switch ( msg )
	{	
		case WM_RBUTTONDOWN:
			return SendMessage ( window->mWndScript, WM_RBUTTONDOWN, wparam, lparam );

		case WM_RBUTTONUP:
			return SendMessage ( window->mWndScript, WM_RBUTTONUP, wparam, lparam );

		case WM_LBUTTONDBLCLK:
		{
			int result = SendMessage ( window->mWndScript, WM_LBUTTONDBLCLK, wparam, lparam );
			window->ToggleBreakpoint ( );
			return result;
		}
	
		case WM_LBUTTONDOWN:
		{
			int result = SendMessage ( window->mWndScript, WM_LBUTTONDOWN, wparam, lparam );
			window->ToggleBreakpoint ( );
			return result;
		}
			
		case WM_LBUTTONUP:
			return SendMessage ( window->mWndScript, WM_LBUTTONUP, wparam, lparam );
			
		case WM_PAINT:
		{
			HDC dc;

			int size = window->mMarginSize - 2;
			
			PAINTSTRUCT ps;
			RECT rect;
			GetClientRect ( wnd, &rect );
			dc = BeginPaint ( wnd, &ps );			
			FillRect ( dc, &rect, GetSysColorBrush ( COLOR_3DFACE ) );
			
			if ( window->mScripts.Num ( ) )
			{
				for ( int i = 0; i < window->mClient->GetBreakpointCount(); i ++ )
				{
					rvDebuggerBreakpoint* bp = window->mClient->GetBreakpoint ( i );
					assert( bp );
					
					if ( !idStr::Icmp ( window->mScripts[window->mActiveScript]->GetFilename ( ), bp->GetFilename ( ) ) )
					{
						int		c;
						POINTL	pos;

						c = SendMessage ( window->mWndScript, EM_LINEINDEX, bp->GetLineNumber ( ) - 1, 0 );
						SendMessage ( window->mWndScript, EM_POSFROMCHAR, (WPARAM)&pos, c );
						ImageList_DrawEx ( window->mImageList, 2, dc, rect.left, pos.y, size, size, CLR_NONE, CLR_NONE, ILD_NORMAL );
					}
				}
				
				if ( window->mClient->IsStopped ( ) )
				{
					if ( !idStr::Icmp ( window->mClient->GetBreakFilename(),
										window->mScripts[window->mActiveScript]->GetFilename ( ) ) )
					{
						int		c;
						POINTL	pos;

						c = SendMessage ( window->mWndScript, EM_LINEINDEX, window->mClient->GetBreakLineNumber() - 1, 0 );
						SendMessage ( window->mWndScript, EM_POSFROMCHAR, (WPARAM)&pos, c );				
						ImageList_DrawEx ( window->mImageList, 3, dc, rect.left, pos.y, size, size, CLR_NONE, CLR_NONE, ILD_NORMAL );
					}
				}
				
				if ( window->mCurrentStackDepth != 0 )
				{
					if ( !window->mClient->GetCallstack()[window->mCurrentStackDepth]->mFilename.Icmp ( window->mScripts[window->mActiveScript]->GetFilename ( ) ) )
					{
						int		c;
						POINTL	pos;

						c = SendMessage ( window->mWndScript, EM_LINEINDEX, window->mClient->GetCallstack()[window->mCurrentStackDepth]->mLineNumber - 1, 0 );
						SendMessage ( window->mWndScript, EM_POSFROMCHAR, (WPARAM)&pos, c );				
						ImageList_DrawEx ( window->mImageList, 1, dc, rect.left, pos.y, size, size, CLR_NONE, CLR_NONE, ILD_NORMAL );
					}
				}
			}
			
			rect.right-=2;
			rect.left = rect.right + 1;
			HPEN pen = CreatePen ( PS_SOLID, 1, GetSysColor ( COLOR_3DSHADOW ) );
			HPEN old = (HPEN)SelectObject ( dc, pen );
			MoveToEx ( dc, rect.right, rect.top, NULL );
			LineTo ( dc, rect.right, rect.bottom );
			SelectObject ( dc, old );
			DeleteObject ( pen );
			EndPaint ( wnd, &ps );
			break;
		}
	}
	
	return DefWindowProc ( wnd, msg, wparam, lparam );
}

/*
================
rvDebuggerWindow::UpdateTitle

Updates the window title of the script debugger to show a few states
================
*/
void rvDebuggerWindow::UpdateTitle ( void )
{
	idStr title;
	
	title = "Quake 4 Script Debugger - ";
	
	if ( mClient->IsConnected ( ) )
	{
		if ( mClient->IsStopped ( ) )
		{
			title += "[break]";
		}
		else
		{	
			title += "[run]";
		}
	}
	else
	{
		title += "[disconnected]";
	}
	
	if ( mScripts.Num ( ) )
	{
		title += " - [";
		title += idStr( mScripts[mActiveScript]->GetFilename() ).StripPath ( );
		title += "]";
	}
	
	SetWindowText ( mWnd, title );
}	

/*
================
rvDebuggerWindow::UpdateScript

Updates the edit window to contain the current script
================
*/
void rvDebuggerWindow::UpdateScript ( void )
{
	UpdateTitle ( );

	// Dont reupdate if the given active script is the one being displayed.
	if ( mActiveScript == mLastActiveScript )
	{	
		return;
	}

	mLastActiveScript = mActiveScript;

	// Show and hide the script window depending on whether or not
	// there are loaded scripts
	if ( mScripts.Num ( ) < 1 )
	{
		ShowWindow ( mWndScript, SW_HIDE );
		return;
	}
	else
	{
		ShowWindow ( mWndScript, SW_SHOW );
	}
	
	// Update the script
	SendMessage ( mWndScript, EM_SETSEL, 0, -1 );
	SendMessage ( mWndScript, EM_REPLACESEL, 0, (LPARAM)"" );
	SendMessage ( mWndScript, EM_SETSEL, 0, -1 );
	SendMessage ( mWndScript, EM_REPLACESEL, 0, (LPARAM)mScripts[mActiveScript]->GetContents ( ) );	
}

/*
================
rvDebuggerWindow::UpdateWindowMenu

Updates the windows displayed in the window menu
================
*/
void rvDebuggerWindow::UpdateWindowMenu ( void )
{
	while ( GetMenuItemCount ( mWindowMenu ) > mWindowMenuPos )
	{
		DeleteMenu ( mWindowMenu, mWindowMenuPos, MF_BYPOSITION );
	}
	
	if ( mScripts.Num() )
	{
		AppendMenu ( mWindowMenu, MF_SEPARATOR, 0, "" );
	}
	
	int i;
	for ( i = 0; i < mScripts.Num(); i ++ )
	{
		idStr name;
		name = mScripts[i]->GetFilename ( );
		name.StripPath ( );
		name = idStr(va("&%d ", i + 1 )) + name;
		AppendMenu ( mWindowMenu, MF_STRING, ID_DBG_WINDOWMIN + i, name );
	}
}

/*
================
rvDebuggerWindow::UpdateCallstack

Updates the contents of teh callastack
================
*/
void rvDebuggerWindow::UpdateCallstack ( void )
{
	LVITEM item;
	ListView_DeleteAllItems ( mWndCallstack );
	ZeroMemory ( &item, sizeof(item) );
	item.mask = LVIF_TEXT|LVIF_IMAGE;
	
	for ( int i = 0; i < mClient->GetCallstack().Num(); i ++ )
	{	
		rvDebuggerCallstack* entry = mClient->GetCallstack()[i];
		item.iItem = ListView_GetItemCount ( mWndCallstack );
		item.pszText = "";
		item.iImage = (i == mCurrentStackDepth) ? 1 : 0;
		ListView_InsertItem ( mWndCallstack, &item );
		
		ListView_SetItemText ( mWndCallstack, item.iItem, 1, (LPSTR)entry->mFunction.c_str() );
		ListView_SetItemText ( mWndCallstack, item.iItem, 2, va("%d", entry->mLineNumber ) );
		ListView_SetItemText ( mWndCallstack, item.iItem, 3, (LPSTR)entry->mFilename.c_str() );
	}
}

/*
================
rvDebuggerWindow::UpdateWatch

Updates the contents of the watch window
================
*/
void rvDebuggerWindow::UpdateWatch ( void )
{
	int i;
		
	// Inspect all the variables we are watching
	for ( i = 0; i < mWatches.Num(); i ++ )
	{
		mWatches[i]->mModified = false;
		mClient->InspectVariable ( mWatches[i]->mVariable, mCurrentStackDepth );
	}
	
	InvalidateRect ( mWndWatch, NULL, FALSE );
	UpdateWindow ( mWndWatch );
}

/*
================
rvDebuggerWindow::HandleInitMenu

Handles the initialization of the main menu
================
*/
int rvDebuggerWindow::HandleInitMenu ( WPARAM wParam, LPARAM lParam ) 
{ 
    int		cMenuItems = GetMenuItemCount((HMENU)wParam); 
    int		nPos; 
    int		id; 
    UINT	flags; 
    HMENU	hmenu;
 
	hmenu = (HMENU) wParam;

	// Run through all the menu items in the menu and see if any of them need
	// modification in any way
    for (nPos = 0; nPos < cMenuItems; nPos++) 
    { 
        id    = GetMenuItemID(hmenu, nPos); 
        flags = 0;
 
		// Handle popup menus too
		if ( id < 0 )
		{
			HMENU sub = GetSubMenu ( hmenu, nPos );
			if ( sub )
			{
				HandleInitMenu ( (WPARAM) sub, 0 );
				continue;
			}
		}
	
		// Handle the dynamic menu items specially
		if ( id >= ID_DBG_WINDOWMIN && id <= ID_DBG_WINDOWMAX )
		{
			if ( id - ID_DBG_WINDOWMIN == mActiveScript )
			{
				CheckMenuItem ( hmenu, nPos, MF_BYPOSITION|MF_CHECKED );
			}
			else
			{
				CheckMenuItem ( hmenu, nPos, MF_BYPOSITION|MF_UNCHECKED );
			}
			continue;
		}
	
		// Menu items that are completely unrelated to the workspace
		switch ( id )
		{
			case ID_DBG_DEBUG_RUN:
			{
				MENUITEMINFO info;
				idStr		 run;

				info.cbSize = sizeof(info);
				info.fMask = MIIM_TYPE|MIIM_STATE;
				info.fType = MFT_STRING;				
				
				if ( !mClient->IsConnected() )
				{				
					run = "Run";
					info.fState = MFS_ENABLED;
				}
				else
				{
					run = "Continue";
					info.fState = mClient->IsStopped()?MFS_ENABLED:MFS_GRAYED;
				}

				info.dwTypeData = (LPSTR)run.c_str();
				info.cch = run.Length ( );

				SendMessage ( mWndToolbar, TB_ENABLEBUTTON, id, MAKELONG(((info.fState==MFS_ENABLED) ? TRUE : FALSE), 0) );
									
				SetMenuItemInfo ( hmenu, id, FALSE, &info );
				
				break;
			}
			
			case ID_DBG_DEBUG_BREAK:
				if ( !mClient->IsConnected() || mClient->IsStopped() )
				{
					EnableMenuItem ( hmenu, nPos, MF_GRAYED|MF_BYPOSITION);
					SendMessage ( mWndToolbar, TB_ENABLEBUTTON, id, MAKELONG(FALSE,0) );
				}
				else
				{
					EnableMenuItem ( hmenu, nPos, MF_ENABLED|MF_BYPOSITION);
					SendMessage ( mWndToolbar, TB_ENABLEBUTTON, id, MAKELONG(TRUE,0) );
				}
				break;				
				
			case ID_DBG_DEBUG_RUNTOCURSOR:
			case ID_DBG_DEBUG_STEPOUT:
			case ID_DBG_DEBUG_STEPOVER:
			case ID_DBG_DEBUG_STEPINTO:
			case ID_DBG_DEBUG_SHOWNEXTSTATEMENT:
//			case ID_DBG_DEBUG_QUICKWATCH:
				if ( !mClient->IsConnected() || !mClient->IsStopped() )
				{
					EnableMenuItem ( hmenu, nPos, MF_GRAYED|MF_BYPOSITION );	
					SendMessage ( mWndToolbar, TB_ENABLEBUTTON, id, MAKELONG(FALSE,0) );
				}
				else
				{
					EnableMenuItem ( hmenu, nPos, MF_ENABLED|MF_BYPOSITION );	
					SendMessage ( mWndToolbar, TB_ENABLEBUTTON, id, MAKELONG(TRUE,0) );
				}
				break;
			
			case ID_DBG_WINDOW_CLOSEALL:
			case ID_DBG_FILE_CLOSE:
			case ID_DBG_DEBUG_TOGGLEBREAKPOINT:
			case ID_DBG_EDIT_FIND:
				EnableMenuItem ( hmenu, nPos, (mScripts.Num()?MF_ENABLED:MF_GRAYED)|MF_BYPOSITION);
				break;
		}
	}
	
	return 0;
}

/*
================
rvDebuggerWindow::HandleCreate

Handles the WM_CREATE command
================
*/
int rvDebuggerWindow::HandleCreate ( WPARAM wparam, LPARAM lparam ) 
{
	UINT		tabsize = 16;
	TEXTMETRIC	tm;
	HDC			dc;
	LOGFONT		lf;
	int			i;

	gDebuggerApp.GetOptions().GetWindowPlacement ( "wp_main", mWnd );

	// Create the main toolbar
	CreateToolbar ( );
	
	// Create the script window
	LoadLibrary ( "Riched20.dll" );
	mWndScript = CreateWindow ( "RichEdit20A", "", WS_CHILD|WS_BORDER|ES_NOHIDESEL|ES_READONLY|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|ES_AUTOHSCROLL|WS_VSCROLL|WS_HSCROLL, 0, 0, 100, 100, mWnd, (HMENU) IDC_DBG_SCRIPT, mInstance, 0 );
	SendMessage ( mWndScript, EM_SETEVENTMASK, 0, ENM_SCROLL|ENM_CHANGE  );
	SendMessage ( mWndScript, EM_SETWORDBREAKPROC, 0, (LPARAM) ScriptWordBreakProc );
	mOldScriptProc = (WNDPROC)GetWindowLong ( mWndScript, GWL_WNDPROC );
	SetWindowLong ( mWndScript, GWL_USERDATA, (LONG)this );
	SetWindowLong ( mWndScript, GWL_WNDPROC, (LONG)ScriptWndProc );

	SendMessage ( mWndScript, EM_SETTABSTOPS, 1, (LPARAM)&tabsize );

	dc = GetDC ( mWndScript );
	GetTextMetrics ( dc, &tm );
	ZeroMemory ( &lf, sizeof(lf) );
	lf.lfHeight = tm.tmHeight;
	strcpy ( lf.lfFaceName, "Courier New" );	
			
	SendMessage ( mWndScript, WM_SETFONT, (WPARAM)CreateFontIndirect ( &lf ), 0 );
	SendMessage ( mWndScript, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG(18,10) );
	SendMessage ( mWndScript, EM_SETBKGNDCOLOR, 0, GetSysColor ( COLOR_3DFACE ) );

	mWndOutput = CreateWindow ( "RichEdit20A", "", WS_CHILD|ES_READONLY|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|ES_AUTOHSCROLL|WS_VSCROLL|WS_HSCROLL|WS_VISIBLE, 0, 0, 100, 100, mWnd, (HMENU) IDC_DBG_OUTPUT, mInstance, 0 );
	SendMessage ( mWndOutput, WM_SETFONT, (WPARAM)CreateFontIndirect ( &lf ), 0 );
	SendMessage ( mWndOutput, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG(18,10) );
	SendMessage ( mWndOutput, EM_SETBKGNDCOLOR, 0, GetSysColor ( COLOR_3DFACE ) );
			
	mWndConsole = CreateWindow ( "RichEdit20A", "", WS_CHILD|ES_READONLY|ES_MULTILINE|ES_WANTRETURN|ES_AUTOVSCROLL|ES_AUTOHSCROLL|WS_VSCROLL|WS_HSCROLL, 0, 0, 100, 100, mWnd, (HMENU) IDC_DBG_CONSOLE, mInstance, 0 );
	SendMessage ( mWndConsole, WM_SETFONT, (WPARAM)CreateFontIndirect ( &lf ), 0 );
	SendMessage ( mWndConsole, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG(18,10) );
	SendMessage ( mWndConsole, EM_SETBKGNDCOLOR, 0, GetSysColor ( COLOR_3DFACE ) );			

	mWndMargin = CreateWindow ( "STATIC", "", WS_VISIBLE|WS_CHILD, 0, 0, 0, 0, mWndScript, (HMENU)IDC_DBG_SPLITTER, mInstance, NULL );
	SetWindowLong ( mWndMargin, GWL_USERDATA, (LONG)this );
	SetWindowLong ( mWndMargin, GWL_WNDPROC, (LONG)MarginWndProc );

	mWndBorder = CreateWindow ( "STATIC", "", WS_VISIBLE|WS_CHILD|SS_GRAYFRAME, 0, 0, 0, 0, mWnd, (HMENU)IDC_DBG_BORDER, mInstance, NULL );

	GetClientRect ( mWnd, &mSplitterRect );
	mSplitterRect.top = (mSplitterRect.bottom-mSplitterRect.top) / 2;
	mSplitterRect.bottom = mSplitterRect.top + 4;

	mWndTabs = CreateWindow ( WC_TABCONTROL, "", TCS_BOTTOM|WS_CHILD|WS_VISIBLE|TCS_FOCUSNEVER, 0, 0, 0, 0, mWnd, (HMENU)IDC_DBG_TABS, mInstance, NULL );
	lf.lfHeight = -MulDiv(8, GetDeviceCaps(dc, LOGPIXELSY), 72);
	strcpy ( lf.lfFaceName, "Arial" );	
	SendMessage ( mWndTabs, WM_SETFONT, (WPARAM)CreateFontIndirect ( &lf ), 0 );
	ReleaseDC ( mWndScript, dc );

	TCITEM item;
	item.mask = TCIF_TEXT;
	item.pszText = "Output";
	TabCtrl_InsertItem ( mWndTabs, 0, &item );
	item.pszText = "Console";
	TabCtrl_InsertItem ( mWndTabs, 1, &item );
	item.pszText = "Call Stack";
	TabCtrl_InsertItem ( mWndTabs, 2, &item );
	item.pszText = "Watch";
	TabCtrl_InsertItem ( mWndTabs, 3, &item );
	item.pszText = "Threads";
	TabCtrl_InsertItem ( mWndTabs, 4, &item );

	mWndCallstack = CreateWindow ( WC_LISTVIEW, "", LVS_REPORT|WS_CHILD|LVS_SHAREIMAGELISTS, 0, 0, 0, 0, mWnd, (HMENU)IDC_DBG_CALLSTACK, mInstance, NULL );
	mWndWatch     = CreateWindow ( WC_LISTVIEW, "", LVS_REPORT|WS_CHILD|LVS_EDITLABELS|LVS_OWNERDRAWFIXED, 0, 0, 0, 0, mWnd, (HMENU)IDC_DBG_WATCH, mInstance, NULL );
	mWndThreads   = CreateWindow ( WC_LISTVIEW, "", LVS_REPORT|WS_CHILD|LVS_SHAREIMAGELISTS, 0, 0, 0, 0, mWnd, (HMENU)IDC_DBG_THREADS, mInstance, NULL );

	LVCOLUMN col;
	col.mask = LVCF_WIDTH|LVCF_TEXT;
	col.cx = 20;
	col.pszText = "";
	ListView_InsertColumn ( mWndCallstack, 0, &col );
	col.cx = 150;
	col.pszText = "Function";
	ListView_InsertColumn ( mWndCallstack, 1, &col );
	col.cx = 150;
	col.pszText = "Line";
	ListView_InsertColumn ( mWndCallstack, 2, &col );
	col.cx = 150;
	col.pszText = "Filename";
	ListView_InsertColumn ( mWndCallstack, 3, &col );

	col.cx = 20;
	col.pszText = "";
	ListView_InsertColumn ( mWndThreads, 0, &col );
	col.cx = 25;
	col.pszText = "ID";
	ListView_InsertColumn ( mWndThreads, 1, &col );
	col.cx = 150;
	col.pszText = "Name";
	ListView_InsertColumn ( mWndThreads, 2, &col );
	col.cx = 100;
	col.pszText = "State";
	ListView_InsertColumn ( mWndThreads, 3, &col );

	col.cx = 150;
	col.pszText = "Name";
	ListView_InsertColumn ( mWndWatch, 0, &col );
	col.cx = 200;
	col.pszText = "Value";
	ListView_InsertColumn ( mWndWatch, 1, &col );

	// Create the image list that is used by the threads window, callstack window, and 
	// margin window
	mImageList = ImageList_Create ( 16, 16, ILC_COLOR|ILC_MASK, 0, 2 );
	ImageList_AddIcon ( mImageList, (HICON)LoadImage ( mInstance, MAKEINTRESOURCE(IDI_DBG_EMPTY), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE|LR_DEFAULTCOLOR) );
	ImageList_AddIcon ( mImageList, (HICON)LoadImage ( mInstance, MAKEINTRESOURCE(IDI_DBG_CURRENT), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE|LR_DEFAULTCOLOR) );
	ImageList_AddIcon ( mImageList, (HICON)LoadImage ( mInstance, MAKEINTRESOURCE(IDI_DBG_BREAKPOINT), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE|LR_DEFAULTCOLOR) );
	ImageList_AddIcon ( mImageList, (HICON)LoadImage ( mInstance, MAKEINTRESOURCE(IDI_DBG_CURRENTLINE), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE|LR_DEFAULTCOLOR) );
	ListView_SetImageList ( mWndThreads, mImageList, LVSIL_SMALL );
	ListView_SetImageList ( mWndCallstack, mImageList, LVSIL_SMALL );

	EnableWindows ( FALSE );

	ListView_SetExtendedListViewStyle ( mWndCallstack, LVS_EX_FULLROWSELECT );
	ListView_SetExtendedListViewStyle ( mWndThreads, LVS_EX_FULLROWSELECT );

	gDebuggerApp.GetOptions().GetColumnWidths ( "cw_callstack", mWndCallstack );
	gDebuggerApp.GetOptions().GetColumnWidths ( "cw_threads", mWndThreads );
	gDebuggerApp.GetOptions().GetColumnWidths ( "cw_watch", mWndWatch );

	mWndToolTips = CreateWindowEx ( WS_EX_TOPMOST,
								    TOOLTIPS_CLASS,
									NULL,
									WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,		
									CW_USEDEFAULT,
									CW_USEDEFAULT,
									CW_USEDEFAULT,
									CW_USEDEFAULT,
									mWnd,
									NULL,
									mInstance,
									NULL
									);
				
	SendMessage ( mWndToolTips, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELONG(400,0) );
	SendMessage ( mWndToolTips, TTM_SETDELAYTIME, TTDT_RESHOW, MAKELONG(400,0) );

	SetWindowPos( mWndToolTips, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	
	LVITEM lvItem;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;
	lvItem.mask = LVIF_TEXT;
	lvItem.pszText = "";
	ListView_InsertItem ( mWndWatch, &lvItem );
	ListView_SetExtendedListViewStyle ( mWndWatch, LVS_EX_FULLROWSELECT );		

	// Recent files
	InitRecentFiles ( );
	UpdateRecentFiles ( );
	
	HandleInitMenu ( (WPARAM)GetMenu ( mWnd ), 0 );

	// Read in the watches
	for ( i = 0; ; i ++ )
	{
		const char* s = gDebuggerApp.GetOptions().GetString ( va("watch%d", i ) );
		if ( !s || !s[0] )
		{
			break;
		}
		
		AddWatch ( s );
	}
	
	return 0;
}

/*
================
rvDebuggerWindow::HandleCommand

Handles the WM_COMMAND message for this window
================
*/
int rvDebuggerWindow::HandleCommand ( WPARAM wparam, LPARAM lparam ) 
{ 
	int event = HIWORD(wparam);
	int id    = LOWORD(wparam);

	// The window menu list needs to be handled specially			
	if ( id >= ID_DBG_WINDOWMIN && id <= ID_DBG_WINDOWMAX )
	{
		mActiveScript = id - ID_DBG_WINDOWMIN;
		UpdateScript ( );
		return 0;
	}

	// The recent file list needs to be handled specially
	if ( LOWORD(wparam) >= ID_DBG_FILE_MRU1 && LOWORD(wparam) < ID_DBG_FILE_MRU1 + rvRegistryOptions::MAX_MRU_SIZE )
	{
		idStr filename;
		filename = gDebuggerApp.GetOptions().GetRecentFile ( gDebuggerApp.GetOptions().GetRecentFileCount() - (LOWORD(wparam)-ID_DBG_FILE_MRU1) - 1 );
		if ( !OpenScript ( filename ) )
		{
			MessageBox ( mWnd, va("Failed to open script '%s'", filename.c_str() ), "Quake 4 Script Debugger", MB_OK );
		}
		return 0;
	}
			
	switch ( id )
	{
		case ID_DBG_EDIT_FINDSELECTED:
		{
			idStr text;		
			GetSelectedText ( text );		
			FindNext ( text );
			break;
		}
		
		case ID_DBG_EDIT_FINDSELECTEDPREV:
		{
			idStr text;		
			GetSelectedText ( text );		
			FindPrev ( text );
			break;
		}
		
		case ID_DBG_EDIT_FINDNEXT:
			FindNext ( );
			break;

		case ID_DBG_EDIT_FINDPREV:
			FindPrev ( );
			break;
	
		case ID_DBG_DEBUG_QUICKWATCH:
		{
			idStr text;
			
			GetSelectedText ( text );
			rvDebuggerQuickWatchDlg dlg;
			dlg.DoModal ( this, mCurrentStackDepth, text );
			break;
		}
		
		case ID_DBG_HELP_ABOUT:
			DialogBox ( mInstance, MAKEINTRESOURCE(IDD_DBG_ABOUT), mWnd, AboutDlgProc );
			break;
			
		case ID_DBG_DEBUG_BREAK:
			mClient->Break ( );
			break;						

		case ID_DBG_DEBUG_STEPOVER:
			EnableWindows ( FALSE );
			mClient->StepOver ( );
			break;						

		case ID_DBG_DEBUG_STEPINTO:
			EnableWindows ( FALSE );
			mClient->StepInto ( );
			break;						

		case ID_DBG_DEBUG_RUN:
			// Run the game if its not running
			if ( !mClient->IsConnected ( ) )
			{
				char exeFile[MAX_PATH];
				char curDir[MAX_PATH];

				STARTUPINFO			startup;
				PROCESS_INFORMATION	process;
						
				ZeroMemory ( &startup, sizeof(startup) );
				startup.cb = sizeof(startup);	

				GetCurrentDirectory ( MAX_PATH, curDir );

				GetModuleFileName ( NULL, exeFile, MAX_PATH );
				const char* s = va("%s +set fs_game %s +set fs_cdpath %s", exeFile, cvarSystem->GetCVarString( "fs_game" ), cvarSystem->GetCVarString( "fs_cdpath" ) );
				CreateProcess ( NULL, (LPSTR)s,
				NULL, NULL, FALSE, 0, NULL, curDir, &startup, &process );

				CloseHandle ( process.hThread );
				CloseHandle ( process.hProcess );
			}
			else if ( mClient->IsStopped() )
			{
				EnableWindows ( FALSE );
				mClient->Resume ( );
				UpdateToolbar ( );
				UpdateTitle ( );
				InvalidateRect ( mWnd, NULL, FALSE );
			}

			break;
			
		case IDC_DBG_SCRIPT:
		{
			RECT	t;					
			LONG	num;
			LONG	dem;
								
			SendMessage ( mWndScript, EM_GETZOOM, (LONG)&num, (LONG)&dem );
			if ( num != mZoomScaleNum || dem != mZoomScaleDem )
			{
				mZoomScaleNum = num;
				mZoomScaleDem = dem;
				GetClientRect ( mWndScript, &t );
				SendMessage ( mWnd, WM_SIZE, 0, MAKELPARAM(t.right-t.left,t.bottom-t.top) );
			}
			else
			{										
				InvalidateRect ( mWndMargin, NULL, TRUE );
			}
			break;
		}
					
		case ID_DBG_DEBUG_TOGGLEBREAKPOINT:
			ToggleBreakpoint ( );
			break;
					
		case ID_DBG_FILE_EXIT:
			PostMessage ( mWnd, WM_CLOSE, 0, 0 );
			break;
					
		case ID_DBG_FILE_CLOSE:
			if ( mScripts.Num() )
			{
				delete mScripts[mActiveScript];
				mScripts.RemoveIndex ( mActiveScript );
				if ( mActiveScript >= mScripts.Num ( ) )
				{
					mActiveScript --;
				}
				UpdateWindowMenu ( );
				UpdateScript ( );
			}
			break;
			
		case ID_DBG_FILE_NEXT:
			if ( mScripts.Num ( ) > 0 )
			{
				mActiveScript++;
				if ( mActiveScript >= mScripts.Num ( ) )
				{
					mActiveScript = 0;
				}
				UpdateWindowMenu ( );
				UpdateScript ( );				
			}
			break;
			
		case ID_DBG_FILE_OPEN:
		{
			rvOpenFileDialog dlg;
			dlg.SetTitle ( "Open Script" );
			dlg.SetFilter ( "*.script; *.gui; *.state" );
			dlg.SetFlags ( OFD_MUSTEXIST );
			if ( dlg.DoModal ( mWnd ) )
			{				
				if ( !OpenScript ( dlg.GetFilename ( ) ) )
				{
					MessageBox ( mWnd, va("Failed to open script '%s'",dlg.GetFilename ( )), "Quake 4 Script Debugger", MB_OK );
				}
			}
			break;
		}
		
		case ID_DBG_EDIT_FIND:
		{
			rvDebuggerFindDlg dlg;
			if ( dlg.DoModal ( this ) )
			{
				FindNext ( dlg.GetFindText ( ) );
			}
			break;
		}
				
		case ID_DBG_WINDOW_CLOSEALL:
		{
			for ( int i = 0; i < mScripts.Num(); i ++ )
			{
				delete mScripts[i];
			}
			
			mScripts.Clear ( );
			mActiveScript = -1;
			
			UpdateWindowMenu ( );
			UpdateScript ( );
			break;
		}			
	}
	
	return 0;
}
		
/*
================
rvDebuggerWindow::WndProc

Window procedure for the deubgger window
================
*/
LRESULT CALLBACK rvDebuggerWindow::WndProc ( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	rvDebuggerWindow* window = (rvDebuggerWindow*) GetWindowLong ( wnd, GWL_USERDATA );
	
	switch ( msg )
	{
		case WM_INITMENUPOPUP:
			window->HandleInitMenu ( wparam, lparam );
			break;

		case WM_DESTROY:
		{
			gDebuggerApp.GetOptions().SetColumnWidths ( "cw_callstack", window->mWndCallstack );
			gDebuggerApp.GetOptions().SetColumnWidths ( "cw_threads", window->mWndThreads );			
			gDebuggerApp.GetOptions().SetColumnWidths ( "cw_watch", window->mWndWatch );
			gDebuggerApp.GetOptions().SetWindowPlacement ( "wp_main", wnd );
			
			int i;
			for ( i = 0; i < window->mWatches.Num ( ); i ++ )
			{
				gDebuggerApp.GetOptions().SetString ( va("watch%d", i ), window->mWatches[i]->mVariable );
			}
			gDebuggerApp.GetOptions().SetString ( va("watch%d", i ), "" );
			
			window->mWnd = NULL;
			SetWindowLong ( wnd, GWL_USERDATA, 0 );		
			break;
		}
	
		case WM_ERASEBKGND:
			return 0;
			
		case WM_COMMAND:
			window->HandleCommand ( wparam, lparam );
			break;		
	
		case WM_SETCURSOR:
		{
			POINT point;
			GetCursorPos ( &point );
			ScreenToClient ( wnd, &point );
			if ( PtInRect ( &window->mSplitterRect, point ) )
			{
				SetCursor ( LoadCursor ( NULL, IDC_SIZENS ) );
				return true;
			}
			break;
		}
	
		case WM_SIZE:
		{
			RECT rect;
			window->mMarginSize = window->mZoomScaleDem ? ((long)(18.0f * (float)window->mZoomScaleNum / (float)window->mZoomScaleDem)):18;
			window->mSplitterRect.left = 0;
			window->mSplitterRect.right = LOWORD(lparam);
			
			SendMessage ( window->mWndToolbar, TB_AUTOSIZE, 0, 0 );
			GetWindowRect ( window->mWndToolbar, &rect );
			MoveWindow ( window->mWndScript, 0, rect.bottom-rect.top, LOWORD(lparam), window->mSplitterRect.top-(rect.bottom-rect.top), TRUE );
			MoveWindow ( window->mWndMargin, 0, 0, window->mMarginSize, window->mSplitterRect.top-(rect.bottom-rect.top), TRUE );
			
			SetRect ( &rect, 0, window->mSplitterRect.bottom, LOWORD(lparam), HIWORD(lparam) );
			MoveWindow ( window->mWndTabs, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE );
			SendMessage ( window->mWndTabs, TCM_ADJUSTRECT, FALSE, (LPARAM)&rect );
			rect.bottom -= 4 ;
			MoveWindow ( window->mWndBorder, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE );
			InflateRect ( &rect, -1, -1 );			
			MoveWindow ( window->mWndOutput, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE );
			MoveWindow ( window->mWndConsole, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE );
			MoveWindow ( window->mWndCallstack, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE );
			MoveWindow ( window->mWndWatch, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE );
			MoveWindow ( window->mWndThreads, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE );
			break;
		}
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc = BeginPaint ( wnd, &ps );			
			FillRect ( dc, &window->mSplitterRect, GetSysColorBrush ( COLOR_3DFACE ) );
			
			if ( !window->mScripts.Num() )
			{
				RECT rect;
				GetClientRect ( wnd, &rect );
				rect.bottom = window->mSplitterRect.top;
				FillRect ( dc, &rect, GetSysColorBrush ( COLOR_APPWORKSPACE ) );
			}
			
			EndPaint ( wnd, &ps );
			break;
		}
	
		case WM_LBUTTONDOWN:
		{
			POINT pt = { LOWORD(lparam),HIWORD(lparam) };
			if ( PtInRect ( &window->mSplitterRect, pt ) )
			{
				window->mSplitterDrag = true;
				SetCapture ( wnd );
				
				HDC dc = GetDC ( wnd );
				DrawFocusRect ( dc, &window->mSplitterRect );
				ReleaseDC ( wnd, dc );
			}
			break;
		}
		
		case WM_LBUTTONUP:
			if ( window->mSplitterDrag )
			{
				HDC dc = GetDC ( wnd );
				DrawFocusRect ( dc, &window->mSplitterRect );
				ReleaseDC ( wnd, dc );
				
				window->mSplitterDrag = false;
				
				RECT client;
				GetClientRect ( wnd, &client );
				SendMessage ( wnd, WM_SIZE, 0, MAKELPARAM(client.right-client.left,client.bottom-client.top) );
				
				InvalidateRect ( wnd, NULL, TRUE );
				
				ReleaseCapture ( );
			}		
			break;
		
		case WM_MOUSEMOVE:
		{
			if ( window->mSplitterDrag )
			{
				HDC dc = GetDC ( wnd );
				DrawFocusRect ( dc, &window->mSplitterRect );
				ReleaseDC ( wnd, dc );

				if ( GetCapture ( ) != wnd )
				{
					break;
				}
				
				RECT client;
				GetClientRect ( wnd, &client );
				
				window->mSplitterRect.top = HIWORD(lparam);
				if ( window->mSplitterRect.top < client.top )
				{
					window->mSplitterRect.top = client.top;
				}
				else if ( window->mSplitterRect.top + 4 > client.bottom )
				{
					window->mSplitterRect.top = client.bottom - 4;
				}
				window->mSplitterRect.bottom = window->mSplitterRect.top + 4;

				dc = GetDC ( wnd );
				DrawFocusRect ( dc, &window->mSplitterRect );
				ReleaseDC ( wnd, dc );
			}
			break;
		}					
	
		case WM_DRAWITEM:
			window->HandleDrawItem ( wparam, lparam );
			break;
	
		case WM_CREATE:
		{					
			CREATESTRUCT* cs = (CREATESTRUCT*) lparam;
			window = (rvDebuggerWindow*) cs->lpCreateParams;
			SetWindowLong ( wnd, GWL_USERDATA, (LONG)cs->lpCreateParams );

			window->mWnd = wnd;
			window->HandleCreate ( wparam, lparam );

			break;
		}

		case WM_ACTIVATE:				
			window->HandleActivate ( wparam, lparam );
			break;
			
		case WM_NOTIFY:
		{
			NMHDR*	hdr;
			hdr = (NMHDR*)lparam;

			// Tool tips
			if ( hdr->code == TTN_GETDISPINFO )
			{
				window->HandleTooltipGetDispInfo ( wparam, lparam );
				break;
			}

			switch ( hdr->idFrom )
			{				
				case IDC_DBG_WATCH:
					switch ( hdr->code )
					{
						case LVN_KEYDOWN:
						{
							NMLVKEYDOWN* key = (NMLVKEYDOWN*) hdr;
							switch ( key->wVKey )
							{
								case VK_DELETE:
								{
									int sel = ListView_GetNextItem ( hdr->hwndFrom, -1, LVNI_SELECTED );
									if ( sel != -1 && sel != ListView_GetItemCount ( hdr->hwndFrom ) - 1 )
									{
										LVITEM item;
										item.iItem = sel;
										item.mask = LVIF_PARAM;
										ListView_GetItem ( hdr->hwndFrom, &item );
										ListView_DeleteItem ( hdr->hwndFrom, sel );
										
										window->mWatches.Remove ( (rvDebuggerWatch*)item.lParam );
										delete (rvDebuggerWatch*)item.lParam;

										ListView_SetItemState ( hdr->hwndFrom, 
															    sel,
															    LVIS_SELECTED, LVIS_SELECTED );
										
										if ( ListView_GetNextItem ( hdr->hwndFrom, -1, LVNI_SELECTED ) == -1 )
										{
											ListView_SetItemState ( hdr->hwndFrom, 
																    ListView_GetItemCount ( hdr->hwndFrom ) - 1,
																    LVIS_SELECTED, LVIS_SELECTED );
										}
									}								
									break;
								}
								
								case VK_RETURN:
								{
									int sel = ListView_GetNextItem ( hdr->hwndFrom, -1, LVNI_SELECTED );
									if ( sel != -1 )
									{
										ListView_EditLabel ( hdr->hwndFrom, sel );
									}
									break;
								}
							}
							break;
						}
							
						case LVN_BEGINLABELEDIT:
						{
							NMLVDISPINFO* di = (NMLVDISPINFO*)hdr;

							DWORD style = GetWindowLong ( ListView_GetEditControl ( hdr->hwndFrom ), GWL_STYLE );
							SetWindowLong ( ListView_GetEditControl ( hdr->hwndFrom ), GWL_STYLE, style & (~WS_BORDER) );

							rvDebuggerWatch* watch = (rvDebuggerWatch*)di->item.lParam;							
							if ( watch )
							{
								SetWindowText ( ListView_GetEditControl ( hdr->hwndFrom ), watch->mVariable );
							}
							
							return FALSE;
						}
							
						case LVN_ENDLABELEDIT:
						{
							NMLVDISPINFO* di = (NMLVDISPINFO*)hdr;
							
							if ( di->item.iItem == ListView_GetItemCount ( hdr->hwndFrom ) - 1 )
							{
								if ( !di->item.pszText || !di->item.pszText[0] )
								{
									return FALSE;
								}
								
								window->AddWatch ( ((NMLVDISPINFO*)hdr)->item.pszText );							
								window->UpdateWatch ( );
								return FALSE;
							}
							else
							{
								rvDebuggerWatch* watch = (rvDebuggerWatch*) di->item.lParam;
								if ( watch && di->item.pszText && di->item.pszText[0] )
								{
									watch->mVariable = di->item.pszText;
								}
							}

							return TRUE;
						}
					}
					break;
					
				case IDC_DBG_CALLSTACK:
					if ( hdr->code == NM_DBLCLK )
					{
						int sel = ListView_GetNextItem ( hdr->hwndFrom, -1, LVNI_SELECTED );
						if ( sel != -1 )
						{
							window->mCurrentStackDepth = sel;
							window->UpdateCallstack ( );
							window->UpdateWatch ( );
							window->OpenScript ( window->mClient->GetCallstack()[window->mCurrentStackDepth]->mFilename,
												 window->mClient->GetCallstack()[window->mCurrentStackDepth]->mLineNumber );
						}						
					}
					break;
				case IDC_DBG_TABS:
					if ( hdr->code == TCN_SELCHANGE )
					{
						ShowWindow ( window->mWndOutput, SW_HIDE );
						ShowWindow ( window->mWndConsole, SW_HIDE );
						ShowWindow ( window->mWndCallstack, SW_HIDE );
						ShowWindow ( window->mWndWatch, SW_HIDE );
						ShowWindow ( window->mWndThreads, SW_HIDE );
						switch ( TabCtrl_GetCurSel ( hdr->hwndFrom ) )
						{
							case 0:
								ShowWindow ( window->mWndOutput, SW_SHOW );
								break;

							case 1:
								ShowWindow ( window->mWndConsole, SW_SHOW );
								break;

							case 2:
								ShowWindow ( window->mWndCallstack, SW_SHOW );
								break;

							case 3:
								ShowWindow ( window->mWndWatch, SW_SHOW );
								break;

							case 4:
								ShowWindow ( window->mWndThreads, SW_SHOW );
								break;
						}
					}
					break;
			}
			break;
		}
		
		case WM_CLOSE:
			if ( window->mClient->IsConnected ( ) )
			{
				if ( IDNO == MessageBox ( wnd, "The debugger is currently connected to a running version of the game.  Are you sure you want to close now?", "Quake 4 Script Debugger", MB_YESNO|MB_ICONQUESTION ) )
				{
					return 0;
				}
			}		
			PostQuitMessage ( 0 );
			break;
	}

	return DefWindowProc ( wnd, msg, wparam, lparam );
}

/*
================
rvDebuggerWindow::Activate

Static method that will activate the currently running debugger.  If one is found
and activated then true will be returned.
================
*/
bool rvDebuggerWindow::Activate ( void )
{
	HWND find;
	
	find = FindWindow ( DEBUGGERWINDOWCLASS, NULL );
	if ( !find )
	{
		return false;
	}
	
	SetForegroundWindow ( find );
	
	return true;
}

/*
================
rvDebuggerWindow::ProcessNetMessage

Process an incoming network message
================
*/
void rvDebuggerWindow::ProcessNetMessage ( msg_t* msg )
{
	unsigned short command;
		
	command = (unsigned short)MSG_ReadShort ( msg );
	
	switch ( command )
	{
		case DBMSG_RESUMED:
			UpdateTitle ( );
			UpdateToolbar ( );
			break;

		case DBMSG_INSPECTVARIABLE:
		{
			char temp[1024];
			char temp2[1024];
			int	 i;
						
			MSG_ReadShort ( msg );
			MSG_ReadString ( msg, temp, 1024 );
			MSG_ReadString ( msg, temp2, 1024 );
			if ( mTooltipVar.Icmp ( temp ) == 0 )
			{
				mTooltipValue = temp2;

				TOOLINFO info;
				info.cbSize   = sizeof(info);
				info.hwnd     = mWndScript;
				info.uId      = 0;
				info.hinst    = mInstance;
				info.lpszText = va("%s=%s", mTooltipVar.c_str(), mTooltipValue.c_str() );
				SendMessage ( mWndToolTips, TTM_UPDATETIPTEXT, 0, (LPARAM)&info );
				SendMessage ( mWndToolTips, TTM_UPDATE, 0, 0 );		
			}

			// See if any of the watches were updated by this inspect
			for ( i = 0; i < mWatches.Num(); i ++ )
			{
				rvDebuggerWatch* watch = mWatches[i];
				
				// See if the name matches the variable
				if ( watch->mVariable.Cmp ( temp ) )
				{					
					continue;
				}
				
				// Has the value changed?
				if ( !watch->mValue.Cmp ( temp2 ) )
				{
					continue;
				}
				
				watch->mModified = true;					
				watch->mValue = temp2;				
								
				// Find the list view item that is storing this watch info and redraw it
				for ( int l = 0; l < ListView_GetItemCount(mWndWatch); l ++ )
				{
					LVITEM item;
					item.mask = LVIF_PARAM;
					item.iItem = l;
					ListView_GetItem ( mWndWatch, &item );
					if ( item.lParam == (LPARAM) watch ) 
					{					
						ListView_RedrawItems ( mWndWatch, l, l );
						UpdateWindow ( mWndWatch );
						break;
					}
				}
			}
			
			break;
		}		
	
		case DBMSG_CONNECT:
		case DBMSG_CONNECTED:
			UpdateTitle ( );
			UpdateToolbar ( );
			Printf ( "Connected...\n" );						
			break;

		case DBMSG_DISCONNECT:
			UpdateTitle ( );
			UpdateToolbar ( );
			Printf ( "Disconnected...\n" );
			break;
		
		case DBMSG_PRINT:
			SendMessage ( mWndConsole, EM_SETSEL, -1, -1 );
			SendMessage ( mWndConsole, EM_REPLACESEL, 0, (LPARAM)(const char*)(msg->data) + msg->readcount );	
			SendMessage ( mWndConsole, EM_SCROLLCARET, 0, 0 );	
			break;
			
		case DBMSG_BREAK:
		{
			Printf ( "Break:  line=%d  file='%s'\n", mClient->GetBreakLineNumber ( ), mClient->GetBreakFilename() );
		
			mCurrentStackDepth = 0;
			mClient->InspectVariable ( mTooltipVar, mCurrentStackDepth );
			UpdateWatch ( );		
			EnableWindows ( TRUE );
			OpenScript ( mClient->GetBreakFilename(), mClient->GetBreakLineNumber() - 1 );
			UpdateTitle ( );
			UpdateToolbar ( );
			SetForegroundWindow ( mWnd );
			break;
		}
		
		case DBMSG_INSPECTCALLSTACK:
		{
			UpdateCallstack ( );
			break;
		}
		
		case DBMSG_INSPECTTHREADS:
		{
			LVITEM item;
			ListView_DeleteAllItems ( mWndThreads );
			ZeroMemory ( &item, sizeof(item) );
			item.mask = LVIF_TEXT|LVIF_IMAGE;
			
			for ( int i = 0; i < mClient->GetThreads().Num(); i ++ )
			{	
				rvDebuggerThread* entry = mClient->GetThreads()[i];
				item.iItem = ListView_GetItemCount ( mWndThreads );
				item.pszText = "";
				item.iImage = entry->mCurrent ? 1 : 0;
				ListView_InsertItem ( mWndThreads, &item );			

				ListView_SetItemText ( mWndThreads, item.iItem, 1, (LPSTR)va("%d", entry->mID) );
				ListView_SetItemText ( mWndThreads, item.iItem, 2, (LPSTR)entry->mName.c_str() );
				
				if ( entry->mDying )
				{
					ListView_SetItemText ( mWndThreads, item.iItem, 3, "Dying" );
				}
				else if ( entry->mWaiting )
				{
					ListView_SetItemText ( mWndThreads, item.iItem, 3, "Waiting" );
				}
				else if ( entry->mDoneProcessing )
				{
					ListView_SetItemText ( mWndThreads, item.iItem, 3, "Stopped" );
				}
				else 
				{
					ListView_SetItemText ( mWndThreads, item.iItem, 3, "Running" );
				}
			}
			break;
		}
	}
}

/*
================
rvDebuggerWindow::Printf

Sends a formatted message to the output window
================
*/
void rvDebuggerWindow::Printf ( const char* fmt, ... )
{
	va_list		argptr;
	char		msg[4096];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	SendMessage ( mWndOutput, EM_SETSEL, -1, -1 );
	SendMessage ( mWndOutput, EM_REPLACESEL, 0, (LPARAM)msg );	
	SendMessage ( mWndOutput, EM_SCROLLCARET, 0, 0 );	
}

/*
================
rvDebuggerWindow::OpenScript

Opens the script with the given filename and will scroll to the given line
number if one is specified
================
*/
bool rvDebuggerWindow::OpenScript ( const char* filename, int lineNumber )
{
	int i;

	SetCursor ( LoadCursor ( NULL, IDC_WAIT ) );

	mActiveScript = -1;
	
	// See if the script is already loaded
	for ( i = 0; i < mScripts.Num(); i ++ )
	{
		if ( !idStr::Icmp ( mScripts[i]->GetFilename ( ), filename ) )
		{
			if ( mActiveScript != i )
			{
				mActiveScript = i;
				break;
			}
		}
	}
	
	// If the script isnt open already then open it
	if ( mActiveScript == -1 )
	{
		rvDebuggerScript* script = new rvDebuggerScript;
		
		// Load the script
		if ( !script->Load ( filename ) )
		{
			delete script;
			SetCursor ( LoadCursor ( NULL, IDC_ARROW ) );
			return false;
		}

		gDebuggerApp.GetOptions().AddRecentFile ( filename );
		UpdateRecentFiles ( );

		mActiveScript = mScripts.Append ( script );
	}

	// Test code that will place a breakpoint on all valid lines of code
#if 0
	
	for ( i = 0; i < mScripts[mActiveScript]->GetProgram().NumStatements(); i ++ )
	{
		dstatement_t* st = &mScripts[mActiveScript]->GetProgram().GetStatement ( i );
		rvDebuggerBreakpoint* bp = new rvDebuggerBreakpoint ( filename, st->linenumber );
		mBreakpoints.Append ( bp );
	}
	
#endif

	UpdateScript ( );											
	UpdateWindowMenu ( );

	// Move to a specific line number?
	if ( lineNumber != -1 )
	{
		int		c;
		
		// Put the caret on the line number specified and scroll it into position.
		// This is a bit of a hack since we set the selection twice, but setting the 
		// selection to (c,c) didnt work for scrolling the caret so we do (c,c+1)
		// and then scroll before going back to (c,c).
		// NOTE: We scroll to the line before the one we want so its more visible
		SetFocus ( mWndScript );
		c = SendMessage ( mWndScript, EM_LINEINDEX, lineNumber - 1, 0 );
		SendMessage ( mWndScript, EM_SETSEL, c, c + 1 );
		SendMessage ( mWndScript, EM_SCROLLCARET, 0, 0 );
		c = SendMessage ( mWndScript, EM_LINEINDEX, lineNumber, 0 );
		SendMessage ( mWndScript, EM_SETSEL, c, c );
	}
	else
	{
		SendMessage ( mWndScript, EM_SETSEL, 0, 0 );
	}	

	// Make sure the script window is visible
	ShowWindow ( mWndScript, SW_SHOW );
	UpdateWindow ( mWndScript );											
	
	SetCursor ( LoadCursor ( NULL, IDC_ARROW ) );
	
	return true;
}

/*
================
rvDebuggerWindow::ToggleBreakpoint

Toggles the breakpoint on the current script line
================
*/
void rvDebuggerWindow::ToggleBreakpoint ( void )
{
	rvDebuggerBreakpoint* bp;
	DWORD				  sel;
	int				      line;
	
	// Find the currently selected line
	SendMessage ( mWndScript, EM_GETSEL, (WPARAM)&sel, 0 );
	line = SendMessage ( mWndScript, EM_LINEFROMCHAR, sel, 0 ) + 1;
	
	// If there is already a breakpoint there then just remove it, otherwise
	// we need to create a new breakpoint
	bp = mClient->FindBreakpoint ( mScripts[mActiveScript]->GetFilename ( ), line );
	if ( !bp )
	{	
		// Make sure the line is code before letting them add a breakpoint there	
		if ( !mScripts[mActiveScript]->IsLineCode ( line ) )
		{
			MessageBeep ( MB_ICONEXCLAMATION );
			return;
		}
	
		mClient->AddBreakpoint ( mScripts[mActiveScript]->GetFilename(), line );
	}
	else
	{
		mClient->RemoveBreakpoint ( bp->GetID ( ) );
	}

	// Force a repaint of the script window
	InvalidateRect ( mWndScript, NULL, FALSE );
}

/*
================
rvDebuggerWindow::AboutDlgProc

Dialog box procedure for the about box
================
*/
INT_PTR CALLBACK rvDebuggerWindow::AboutDlgProc ( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch ( msg )
	{
		case WM_COMMAND:
			EndDialog ( wnd, 0 );
			break;
	}

	return FALSE;
}

/*
================
rvDebuggerWindow::CreateToolbar

Create the toolbar and and all of its buttons
================
*/
void rvDebuggerWindow::CreateToolbar ( void )
{
	// Create the toolbar control
	mWndToolbar = CreateWindowEx ( 0, TOOLBARCLASSNAME, "", WS_CHILD|WS_VISIBLE,0,0,0,0, mWnd, (HMENU)IDC_DBG_TOOLBAR, mInstance, NULL );

	// Initialize the toolbar
	SendMessage ( mWndToolbar, TB_BUTTONSTRUCTSIZE, ( WPARAM )sizeof( TBBUTTON ), 0 );	
	SendMessage ( mWndToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(16,16) );	
	SendMessage ( mWndToolbar, TB_SETSTYLE, 0, SendMessage ( mWndToolbar, TB_GETSTYLE, 0, 0 ) | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS );
	
	TBMETRICS tbmet;
	tbmet.cbSize = sizeof(TBMETRICS);
	SendMessage ( mWndToolbar, TB_GETMETRICS, 0, (LPARAM)&tbmet );
	tbmet.cyPad = 0;
	tbmet.cyBarPad = 0;
	SendMessage ( mWndToolbar, TB_SETMETRICS, 0, (LPARAM)&tbmet );

	// Add the bitmap containing button images to the toolbar. 
	TBADDBITMAP	tbab; 
	tbab.hInst = mInstance; 
	tbab.nID = IDB_DBG_TOOLBAR; 
	SendMessage( mWndToolbar, TB_ADDBITMAP, (WPARAM)4, (LPARAM) &tbab ); 

	// Add the buttons to the toolbar
	TBBUTTON tbb[] = { { 0, 0,					TBSTATE_ENABLED, BTNS_SEP,    0, 0, -1 },
					   { 8, ID_DBG_FILE_OPEN,	TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, -1 },
					   { 0, 0,					TBSTATE_ENABLED, BTNS_SEP,    0, 0, -1 },
					   { 0, ID_DBG_DEBUG_RUN,	TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, -1 },
					   { 1, ID_DBG_DEBUG_BREAK, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, -1 },
					   { 0, 0,					TBSTATE_ENABLED, BTNS_SEP,    0, 0, -1 },
					   { 4, ID_DBG_DEBUG_STEPINTO, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, -1 },
					   { 5, ID_DBG_DEBUG_STEPOVER, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, -1 },
					   { 6, ID_DBG_DEBUG_STEPOUT, TBSTATE_ENABLED, BTNS_BUTTON, 0, 0, -1 },
					   { 0, 0,					TBSTATE_ENABLED, BTNS_SEP,    0, 0, -1 } };

	SendMessage( mWndToolbar, TB_ADDBUTTONS, (WPARAM)sizeof(tbb)/sizeof(TBBUTTON), (LPARAM) tbb ); 
}

/*
================
rvDebuggerWindow::HandleTooltipGetDispInfo

Handle the getdispinfo notification message for tooltips by responding with the
tooptip text for the given toolbar button.
================
*/
void rvDebuggerWindow::HandleTooltipGetDispInfo ( WPARAM wparam, LPARAM lparam )
{
	NMTTDISPINFO* ttdi = (NMTTDISPINFO*) lparam;
	switch ( ttdi->hdr.idFrom )
	{
		case ID_DBG_FILE_OPEN:
			strcpy ( ttdi->szText, "Open Script" );
			break;
	
		case ID_DBG_DEBUG_STEPINTO:
			strcpy ( ttdi->szText, "Step Into" );
			break;

		case ID_DBG_DEBUG_STEPOVER:
			strcpy ( ttdi->szText, "Step Over" );
			break;
			
		case ID_DBG_DEBUG_STEPOUT:
			strcpy ( ttdi->szText, "Step Out" );
			break;

		case ID_DBG_DEBUG_BREAK:
			strcpy ( ttdi->szText, "Break" );
			break;
			
		case ID_DBG_DEBUG_RUN:
			if ( mClient->IsConnected() )
			{
				strcpy ( ttdi->szText, "Continue" );
			}
			else
			{
				strcpy ( ttdi->szText, "Run" );
			}
			break;
			
		default:
			strcpy ( ttdi->szText, "" );
			break;
	}
}

/*
================
rvDebuggerWindow::HandleActivate

When the main window is activated, check all the loaded scripts and see if any of them
have been modified since the last time they were loaded.  If they have then reload
them and adjust all breakpoints that now fall on invalid lines.
================
*/
int rvDebuggerWindow::HandleActivate ( WPARAM wparam, LPARAM lparam )
{
	int i;

	// We are only interested in the activation, not deactivation
	if ( !LOWORD(wparam) )
	{
		return 0;
	}
	
	// Run through all of the loaded scripts and see if any of them have been modified
	for ( i = 0; i < mScripts.Num(); i ++ )
	{
		if ( mScripts[i]->IsFileModified ( true ) )
		{
			if ( IDYES == MessageBox ( mWnd, va("%s\n\nThis file has been modified outside of the debugger.\nDo you want to reload it?", mScripts[i]->GetFilename() ), "Quake 4 Script Debugger", MB_YESNO|MB_ICONQUESTION ) )
			{			
				mScripts[i]->Reload ( );

				// Update the script if it was the active one				
				if ( mActiveScript == i )
				{
					mLastActiveScript = -1;
					UpdateScript ( );
				}
				
				// Loop through the breakpoints and see if any of them have
				// moved to invalid lines within the script.  If so then just remove
				// them.
				for ( int b = mClient->GetBreakpointCount() - 1; b >= 0 ; b-- )
				{
					rvDebuggerBreakpoint* bp = mClient->GetBreakpoint(b);
					assert ( bp );
					
					if ( !idStr::Icmp ( bp->GetFilename(), mScripts[i]->GetFilename() ) )
					{
						if ( !mScripts[i]->IsLineCode ( bp->GetLineNumber ( ) ) )
						{
							mClient->RemoveBreakpoint ( bp->GetID ( ) );
						}												
					}					
				}
			}
		}
	}	
	
	return 1;
}

/*
================
rvDebuggerWindow::EnableWindows

Enables and disables all windows with a enable state dependent on the current
connected and paused state of the debugger.
================
*/
void rvDebuggerWindow::EnableWindows ( bool state )
{
	EnableWindow ( mWndCallstack, state );
	EnableWindow ( mWndThreads, state );
	EnableWindow ( mWndWatch, state );
}

/*
================
rvDebuggerWindow::AddWatch

Add a variable to the watch window.  If update is set to true then also query the 
debugger client for the value
================
*/
void rvDebuggerWindow::AddWatch ( const char* varname, bool update )
{
	rvDebuggerWatch* watch;
	
	watch = new rvDebuggerWatch;
	watch->mVariable = varname;
	watch->mModified = false;
	watch->mValue = "???";
	mWatches.Append ( watch );

	// Add the variable to the watch control
	LVITEM item;
	item.mask = LVIF_PARAM;
	item.iItem = ListView_GetItemCount ( mWndWatch ) - 1;
	item.iSubItem = 0;
	item.lParam = (LPARAM)watch;
	ListView_InsertItem ( mWndWatch, &item );	
		
	// If update is set then request the value from the debugger client
	if ( update )
	{
		mClient->InspectVariable ( varname, mCurrentStackDepth );
	}
}

/*
================
rvDebuggerWindow::InitRecentFiles

Finds the file menu and the location within it where the MRU should
be added.
================
*/
bool rvDebuggerWindow::InitRecentFiles ( void )
{
	int	i;
	int count;

	mRecentFileMenu = GetSubMenu ( GetMenu(mWnd), 0 );
	count			= GetMenuItemCount ( mRecentFileMenu );
	
	for ( i = 0; i < count; i ++ )
	{
		if ( GetMenuItemID ( mRecentFileMenu, i ) == ID_DBG_FILE_MRU )
		{
			mRecentFileInsertPos = i;
			DeleteMenu ( mRecentFileMenu, mRecentFileInsertPos, MF_BYPOSITION );
			return true;
		}
	}
	
	return false;
}

/*
================
rvDebuggerWindow::UpdateRecentFiles

Updates the mru in the menu
================
*/
void rvDebuggerWindow::UpdateRecentFiles ( void )
{
	int i;
	int j;

	// Make sure everything is initialized
	if ( !mRecentFileMenu )	
	{	
		InitRecentFiles ( );
	}

	// Delete all the old recent files from the menu's
	for ( i = 0; i < rvRegistryOptions::MAX_MRU_SIZE; i ++ )
	{
		DeleteMenu ( mRecentFileMenu, ID_DBG_FILE_MRU1 + i, MF_BYCOMMAND );
	}	

	// Make sure there is a separator after the recent files
	if ( gDebuggerApp.GetOptions().GetRecentFileCount() )
	{
		MENUITEMINFO info;
		ZeroMemory ( &info, sizeof(info) );
		info.cbSize = sizeof(info);
		info.fMask = MIIM_FTYPE;
		GetMenuItemInfo ( mRecentFileMenu, mRecentFileInsertPos+1,TRUE, &info );
		if ( !(info.fType & MFT_SEPARATOR ) )
		{
			InsertMenu ( mRecentFileMenu, mRecentFileInsertPos, MF_BYPOSITION|MF_SEPARATOR|MF_ENABLED, 0, NULL );
		}
	}

	// Add the recent files to the menu now
	for ( j = 0, i = gDebuggerApp.GetOptions().GetRecentFileCount ( ) - 1; i >= 0; i --, j++ )
	{
		UINT id = ID_DBG_FILE_MRU1 + j;
		idStr str = va("&%d ", j+1);
		str.Append ( gDebuggerApp.GetOptions().GetRecentFile ( i ) );
		InsertMenu ( mRecentFileMenu, mRecentFileInsertPos+j+1, MF_BYPOSITION|MF_STRING|MF_ENABLED, id, str );
	}		
}

/*
================
rvDebuggerWindow::GetSelectedText

Function to retrieve the text that is currently selected in the 
script control
================
*/
int rvDebuggerWindow::GetSelectedText ( idStr& text )
{
	TEXTRANGE	range;
	int			start;
	int			end;
	char*		temp;
		
	text.Empty ( );
			
	if ( mScripts.Num ( ) )
	{
		SendMessage ( mWndScript, EM_GETSEL, (WPARAM)&start, (LPARAM)&end );
		if ( start == end )
		{
			end   = SendMessage ( mWndScript, EM_FINDWORDBREAK, WB_RIGHT, start );
			start = SendMessage ( mWndScript, EM_FINDWORDBREAK, WB_LEFT, start );
		}
		
		temp = new char[end-start+10];
		range.chrg.cpMin = start;
		range.chrg.cpMax = end;
		range.lpstrText  = temp;
		SendMessage ( mWndScript, EM_GETTEXTRANGE, 0, (LPARAM) &range );
		text = temp;
		delete[] temp;
		
		return start;
	}
	
	return -1;
}

/*
================
rvDebuggerWindow::FindNext

Finds the next match of the find text in the active script.  The next is
always relative to the current selection.  If the text parameter is NULL
then the last text used will be searched for. 
================
*/
bool rvDebuggerWindow::FindNext ( const char* text )
{	
	int		 start;
	FINDTEXT ft;

	if ( text )
	{
		mFind = text;
	}

	if ( !mFind.Length ( ) )
	{
		return false;
	}

	SendMessage ( mWndScript, EM_GETSEL, (WPARAM)&start, (LPARAM)0 );
	if ( start < 0 )
	{				
		start = 0;
	}

	ft.chrg.cpMin = start + 1;
	ft.chrg.cpMax = -1;
	ft.lpstrText = mFind.c_str();
	start = SendMessage ( mWndScript, EM_FINDTEXT, FR_DOWN, (LPARAM)&ft );

	if ( start < 0 )
	{
		ft.chrg.cpMin = 0;
		ft.chrg.cpMax = -1;
		ft.lpstrText = mFind.c_str();
		start = SendMessage ( mWndScript, EM_FINDTEXT, FR_DOWN, (LPARAM)&ft );
		
		if ( start < 0 )
		{
			return false;
		}
	}
	
	SendMessage ( mWndScript, EM_SETSEL, start, start + mFind.Length() );
	SendMessage ( mWndScript, EM_SCROLLCARET, 0, 0 );
	
	return true;
}

/*
================
rvDebuggerWindow::FindPrev

Finds the previous match of the find text in the active script.  The previous is
always relative to the current selection.  If the text parameter is NULL
then the last text used will be searched for. 
================
*/
bool rvDebuggerWindow::FindPrev ( const char* text )
{	
	int		 start;
	FINDTEXT ft;

	if ( text )
	{
		mFind = text;
	}

	if ( !mFind.Length ( ) )
	{
		return false;
	}

	SendMessage ( mWndScript, EM_GETSEL, (WPARAM)&start, (LPARAM)0 );
	if ( start < 0 )
	{				
		start = 0;
	}

	ft.chrg.cpMin = start;
	ft.chrg.cpMax = -1;
	ft.lpstrText = mFind.c_str();
	start = SendMessage ( mWndScript, EM_FINDTEXT, 0, (LPARAM)&ft );

	if ( start < 0 )
	{
		GETTEXTLENGTHEX gtl;
		gtl.flags = GTL_DEFAULT;
		gtl.codepage = CP_ACP;
		ft.chrg.cpMin = SendMessage ( mWndScript, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0 );
		ft.chrg.cpMax = 0;
		ft.lpstrText = mFind.c_str();
		start = SendMessage ( mWndScript, EM_FINDTEXT, 0, (LPARAM)&ft );
		
		if ( start < 0 )
		{
			return false;
		}
	}
	
	SendMessage ( mWndScript, EM_SETSEL, start, start + mFind.Length() );
	SendMessage ( mWndScript, EM_SCROLLCARET, 0, 0 );
	
	return true;
}

/*
================
rvDebuggerWindow::HandleDrawItem

Handled the WM_DRAWITEM message.  The watch window is custom drawn so a grid can be displayed.
================
*/
int rvDebuggerWindow::HandleDrawItem ( WPARAM wparam, LPARAM lparam )
{
	DRAWITEMSTRUCT*		dis;
	LVCOLUMN			col;
	int					index;
	idStr				widths;
	RECT				rect;
	rvDebuggerWatch*	watch;
	bool				selected;
	
	dis        = (DRAWITEMSTRUCT*) lparam;	
	watch	   = (rvDebuggerWatch*)dis->itemData;

	col.mask   = LVCF_WIDTH;	
	rect       = dis->rcItem;
	rect.left  = rect.left - 1;
	rect.right = rect.left;
	rect.bottom++;

	selected = ((dis->itemState & ODS_SELECTED) && GetFocus()==mWndWatch);

	// Set the colors based on the selected state and draw the item background
	if ( selected )
	{
		FillRect ( dis->hDC, &dis->rcItem, GetSysColorBrush ( COLOR_HIGHLIGHT ) );
	}
	else
	{
		FillRect ( dis->hDC, &dis->rcItem, GetSysColorBrush ( IsWindowEnabled ( mWndWatch ) ? COLOR_WINDOW : COLOR_3DFACE ) );		
	}
		
	// Run through the columns and draw each with a frame around it and the text 
	// vertically centered in it
	for ( index = 0; ListView_GetColumn ( mWndWatch, index, &col ); index ++ )
	{
		rect.right = rect.left + col.cx;
		FrameRect ( dis->hDC, &rect, GetSysColorBrush ( COLOR_3DFACE ) );			

		// Draw info on the watch if available
		if ( watch  )
		{
			RECT textrect;
			textrect = rect;
			textrect.left += 5;
		
			switch ( index )
			{
				case 0:
					SetTextColor ( dis->hDC, GetSysColor ( selected ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT ) );
					DrawText ( dis->hDC, watch->mVariable, -1, &textrect, DT_LEFT|DT_VCENTER );		
					break;

				case 1:
					if ( watch && watch->mModified && (IsWindowEnabled ( mWndWatch ) ) )
					{
						SetTextColor ( dis->hDC, RGB(255,50,50) );
					}
					else
					{
						SetTextColor ( dis->hDC, GetSysColor ( selected ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT ) );
					}
					DrawText ( dis->hDC, watch->mValue, -1, &textrect, DT_LEFT|DT_VCENTER );		
					break;
			}	
		}

		rect.left = rect.right - 1;
	}	
	
	return 0;
}
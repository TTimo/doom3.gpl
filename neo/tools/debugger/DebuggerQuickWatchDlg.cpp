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
#include "DebuggerQuickWatchDlg.h"

/*
================
rvDebuggerQuickWatchDlg::rvDebuggerQuickWatchDlg
================
*/
rvDebuggerQuickWatchDlg::rvDebuggerQuickWatchDlg ( void )
{
}

/*
================
rvDebuggerQuickWatchDlg::DoModal

Launch the dialog
================
*/
bool rvDebuggerQuickWatchDlg::DoModal ( rvDebuggerWindow* window, int callstackDepth, const char* variable )
{
	mCallstackDepth = callstackDepth;
	mDebuggerWindow = window;
	mVariable       = variable?variable:"";
	
	DialogBoxParam ( window->GetInstance(), MAKEINTRESOURCE(IDD_DBG_QUICKWATCH), window->GetWindow(), DlgProc, (LONG)this );

	return true;
}

/*
================
rvDebuggerQuickWatchDlg::DlgProc

Dialog Procedure for the quick watch dialog
================
*/
INT_PTR CALLBACK rvDebuggerQuickWatchDlg::DlgProc ( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	rvDebuggerQuickWatchDlg* dlg = (rvDebuggerQuickWatchDlg*) GetWindowLong ( wnd, GWL_USERDATA );
	
	switch ( msg )
	{
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lparam;
			mmi->ptMinTrackSize.x = 300;
			mmi->ptMinTrackSize.y = 200;
			break;
		}
		
		case WM_CLOSE:
			gDebuggerApp.GetOptions().SetWindowPlacement ( "wp_quickwatch", wnd );
			gDebuggerApp.GetOptions().SetColumnWidths ( "cw_quickwatch", GetDlgItem ( wnd, IDC_DBG_CURVALUE ) );
			EndDialog ( wnd, 0 );
			break;
	
		case WM_SIZE:
		{
			RECT client;
			RECT button;
			
			GetClientRect ( wnd, &client );
			GetWindowRect ( GetDlgItem ( wnd, IDC_DBG_RECALC ), &button );
			ScreenToClient ( wnd, (POINT*)&button );
			ScreenToClient ( wnd, (POINT*)&button.right );
			MoveWindow ( GetDlgItem ( wnd, IDC_DBG_RECALC ), client.right - dlg->mButtonFromRight, button.top, button.right-button.left,button.bottom-button.top, TRUE );

			GetWindowRect ( GetDlgItem ( wnd, IDC_DBG_CLOSE ), &button );
			ScreenToClient ( wnd, (POINT*)&button );
			ScreenToClient ( wnd, (POINT*)&button.right );
			MoveWindow ( GetDlgItem ( wnd, IDC_DBG_CLOSE ), client.right - dlg->mButtonFromRight, button.top, button.right-button.left,button.bottom-button.top, TRUE );			

			GetWindowRect ( GetDlgItem ( wnd, IDC_DBG_ADDWATCH ), &button );
			ScreenToClient ( wnd, (POINT*)&button );
			ScreenToClient ( wnd, (POINT*)&button.right );
			MoveWindow ( GetDlgItem ( wnd, IDC_DBG_ADDWATCH ), client.right - dlg->mButtonFromRight, button.top, button.right-button.left,button.bottom-button.top, TRUE );			

			GetWindowRect ( GetDlgItem ( wnd, IDC_DBG_VARIABLE ), &button );
			ScreenToClient ( wnd, (POINT*)&button );
			ScreenToClient ( wnd, (POINT*)&button.right );
			MoveWindow ( GetDlgItem ( wnd, IDC_DBG_VARIABLE ), button.left, button.top, client.right-button.left-dlg->mEditFromRight, button.bottom-button.top, TRUE );			

			GetWindowRect ( GetDlgItem ( wnd, IDC_DBG_CURVALUE ), &button );
			ScreenToClient ( wnd, (POINT*)&button );
			ScreenToClient ( wnd, (POINT*)&button.right );
			MoveWindow ( GetDlgItem ( wnd, IDC_DBG_CURVALUE ), button.left, button.top, client.right-button.left-dlg->mEditFromRight, client.bottom-button.top - dlg->mEditFromBottom, TRUE );			
			
			break;
		}
	
		case WM_INITDIALOG:			
		{		
			RECT  client;
			RECT  button;

			// Attach the dialog class pointer to the window
			dlg = (rvDebuggerQuickWatchDlg*) lparam;
			SetWindowLong ( wnd, GWL_USERDATA, lparam );
			dlg->mWnd = wnd;
			
			GetClientRect ( wnd, &client );

			GetWindowRect ( GetDlgItem ( wnd, IDC_DBG_RECALC ), &button );
			ScreenToClient ( wnd, (POINT*)&button );	
			dlg->mButtonFromRight = client.right - button.left;

			GetWindowRect ( GetDlgItem ( wnd, IDC_DBG_CURVALUE ), &button );
			ScreenToClient ( wnd, (POINT*)&button.right );
			dlg->mEditFromRight = client.right - button.right;
			dlg->mEditFromBottom = client.bottom - button.bottom;
								
			// Disable the value controls until a variable is entered
			EnableWindow ( GetDlgItem ( wnd, IDC_DBG_ADDWATCH ), false );
			EnableWindow ( GetDlgItem ( wnd, IDC_DBG_RECALC ), false );
			EnableWindow ( GetDlgItem ( wnd, IDC_DBG_CURVALUE ), false );
			EnableWindow ( GetDlgItem ( wnd, IDC_DBG_CURVALUE_STATIC ), false );

			// Add the columns to the list control
			LVCOLUMN col;
			col.mask = LVCF_WIDTH|LVCF_TEXT;
			col.cx = 100;
			col.pszText = "Name";
			ListView_InsertColumn ( GetDlgItem ( wnd, IDC_DBG_CURVALUE ), 0, &col );
			col.cx = 150;
			col.pszText = "Value";
			ListView_InsertColumn ( GetDlgItem ( wnd, IDC_DBG_CURVALUE ), 1, &col );
		
			// Set the initial variable if one was given
			if ( dlg->mVariable.Length() )
			{
				dlg->SetVariable ( dlg->mVariable, true );
				SetWindowText( GetDlgItem ( wnd, IDC_DBG_VARIABLE ), dlg->mVariable );
			}

			gDebuggerApp.GetOptions().GetWindowPlacement ( "wp_quickwatch", wnd );
			gDebuggerApp.GetOptions().GetColumnWidths ( "cw_quickwatch", GetDlgItem ( wnd, IDC_DBG_CURVALUE ) );
		
			return TRUE;
		}
			
		case WM_COMMAND:
			switch ( LOWORD(wparam) )
			{
				case IDC_DBG_CLOSE:
					SendMessage ( wnd, WM_CLOSE, 0, 0 );
					break;
					
				case IDC_DBG_VARIABLE:
					// When the variable text changes to something other than empty 
					// we can enable the addwatch and recalc buttons
					if ( HIWORD(wparam) == EN_CHANGE )
					{
						bool enable = GetWindowTextLength ( GetDlgItem ( wnd, IDC_DBG_VARIABLE ) )?true:false;
						EnableWindow ( GetDlgItem ( wnd, IDC_DBG_ADDWATCH ), enable );
						EnableWindow ( GetDlgItem ( wnd, IDC_DBG_RECALC ), enable );
					}
					break;

				case IDC_DBG_ADDWATCH:
				{
					char varname[1024];
					GetWindowText ( GetDlgItem ( wnd, IDC_DBG_VARIABLE ), varname, 1023 ); 
					dlg->mDebuggerWindow->AddWatch ( varname );
					break;
				}
					
				case IDC_DBG_RECALC:
				{
					char varname[1024];
					GetWindowText ( GetDlgItem ( wnd, IDC_DBG_VARIABLE ), varname, 1023 ); 
					dlg->SetVariable ( varname );
					break;
				}
			}
			break;
	}
	
	return FALSE;
}

/*
================
rvDebuggerQuickWatchDlg::SetVariable

Sets the current variable being inspected
================
*/
void rvDebuggerQuickWatchDlg::SetVariable ( const char* varname, bool force )
{
	// See if the variable has changed
	if ( !force && !mVariable.Icmp ( varname ) )
	{
		return;
	}

	// Throw up a wait cursor	
	SetCursor ( LoadCursor ( NULL, IDC_WAIT ) );

	// Clear the current value list control
	ListView_DeleteAllItems ( GetDlgItem ( mWnd, IDC_DBG_CURVALUE ) );

	// Get the value of the new variable
	gDebuggerApp.GetClient().InspectVariable ( varname, mCallstackDepth );

	// Wait for the variable value to be sent over from the debugger server		
	if ( !gDebuggerApp.GetClient().WaitFor ( DBMSG_INSPECTVARIABLE, 2500 ) )
	{
		return;
	}
	
	// Make sure we got the value of the variable
	if ( !gDebuggerApp.GetClient().GetVariableValue(varname, mCallstackDepth)[0] )
	{
		return;
	}

	// Enable the windows that display the current value
	mVariable = varname;
	EnableWindow ( GetDlgItem ( mWnd, IDC_DBG_CURVALUE ), true );
	EnableWindow ( GetDlgItem ( mWnd, IDC_DBG_CURVALUE_STATIC ), true );
			
	// Add the variablae value to the list control			
	LVITEM item;
	item.mask = LVIF_TEXT;
	item.pszText = (LPSTR)varname;
	item.iItem = 0;
	item.iSubItem = 0;
	ListView_InsertItem ( GetDlgItem ( mWnd, IDC_DBG_CURVALUE ), &item );				
	ListView_SetItemText ( GetDlgItem ( mWnd, IDC_DBG_CURVALUE ), 0, 1, (LPSTR)gDebuggerApp.GetClient().GetVariableValue(varname,mCallstackDepth) );

	// Give focus back to the variable edit control and set the cursor back to an arrow
	SetFocus ( GetDlgItem ( mWnd, IDC_DBG_VARIABLE ) );
	SetCursor ( LoadCursor ( NULL, IDC_ARROW ) );
}

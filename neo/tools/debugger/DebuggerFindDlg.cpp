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
#include "DebuggerFindDlg.h"

char rvDebuggerFindDlg::mFindText[ 256 ];

/*
================
rvDebuggerFindDlg::rvDebuggerFindDlg
================
*/
rvDebuggerFindDlg::rvDebuggerFindDlg ( void )
{
}

/*
================
rvDebuggerFindDlg::DoModal

Launch the dialog
================
*/
bool rvDebuggerFindDlg::DoModal ( rvDebuggerWindow* parent )
{	
	if ( DialogBoxParam ( parent->GetInstance(), MAKEINTRESOURCE(IDD_DBG_FIND), parent->GetWindow(), DlgProc, (LONG)this ) )
	{
		return true;
	}

	return false;
}

/*
================
rvrvDebuggerFindDlg::DlgProc

Dialog Procedure for the find dialog
================
*/
INT_PTR CALLBACK rvDebuggerFindDlg::DlgProc ( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	rvDebuggerFindDlg* dlg = (rvDebuggerFindDlg*) GetWindowLong ( wnd, GWL_USERDATA );
	
	switch ( msg )
	{	
		case WM_CLOSE:
			EndDialog ( wnd, 0 );
			break;
	
		case WM_INITDIALOG:	
			dlg = (rvDebuggerFindDlg*) lparam;
			SetWindowLong ( wnd, GWL_USERDATA, (LONG) dlg );
			dlg->mWnd = wnd;
			SetWindowText ( GetDlgItem ( dlg->mWnd, IDC_DBG_FIND ), dlg->mFindText );
			return TRUE;
			
		case WM_COMMAND:
			switch ( LOWORD(wparam) )
			{
				case IDOK:
				{
					GetWindowText ( GetDlgItem ( wnd, IDC_DBG_FIND ), dlg->mFindText, sizeof( dlg->mFindText ) - 1 );
					EndDialog ( wnd, 1 );
					break;
				}
				
				case IDCANCEL:
					EndDialog ( wnd, 0 );
					break;
			}
			break;
	}
	
	return FALSE;
}

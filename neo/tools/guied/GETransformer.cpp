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
#include "../common/MaskEdit.h"

HHOOK	gTransHook	 = NULL;
HWND	gTransDlg	 = NULL;

rvGETransformer::rvGETransformer ( )
{
	mWnd		= NULL;
	mWorkspace	= NULL;
}

bool rvGETransformer::Create ( HWND parent, bool visible )
{
	WNDCLASSEX wndClass;
	memset ( &wndClass, 0, sizeof(wndClass) );
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpszClassName = "GUIEDITOR_TRANSFORMER_CLASS";		
	wndClass.lpfnWndProc = rvGETransformer::WndProc;
	wndClass.hbrBackground = (HBRUSH)GetStockObject( LTGRAY_BRUSH );; 
	wndClass.hCursor       = LoadCursor((HINSTANCE) NULL, IDC_ARROW); 
	wndClass.lpszMenuName  = NULL;
	wndClass.hInstance     = win32.hInstance; 
	RegisterClassEx ( &wndClass );

	mWnd = CreateWindowEx ( WS_EX_TOOLWINDOW, 
							"GUIEDITOR_TRANSFORMER_CLASS", 
							"Transformer", 
							WS_SYSMENU|WS_CAPTION|WS_POPUP|WS_OVERLAPPED|WS_BORDER|WS_CLIPSIBLINGS|WS_CHILD, 
							0, 0, 200,100,
							parent, 
							NULL, 
							win32.hInstance, 
							this );
							
	if ( !mWnd )
	{
		return false;
	}

	if ( !gApp.GetOptions().GetWindowPlacement ( "transformer", mWnd ) )
	{
		RECT rParent;
		RECT rTrans;
		
		GetWindowRect ( parent, &rParent );
		GetWindowRect ( mWnd, &rTrans );			
		SetWindowPos ( mWnd, NULL,
					rParent.right - 10 - (rTrans.right-rTrans.left),
					rParent.bottom - 10 - (rTrans.bottom-rTrans.top),
					0,0,
					SWP_NOZORDER|SWP_NOSIZE );
	}
				 	
	Show ( visible );
	
	return true;							
}

LRESULT CALLBACK rvGETransformer::WndProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	rvGETransformer* trans = (rvGETransformer*) GetWindowLong ( hWnd, GWL_USERDATA );

	switch ( msg )
	{
		case WM_NCACTIVATE:
			return gApp.ToolWindowActivate ( hWnd, msg, wParam, lParam );

		case WM_ACTIVATE:
			common->ActivateTool( LOWORD( wParam ) != WA_INACTIVE );
			break;

		case WM_DESTROY:
			gApp.GetOptions().SetWindowPlacement ( "transformer", hWnd );
			break;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_CREATE:
		{
			LPCREATESTRUCT	cs;
			
			// Attach the class to the window first
			cs = (LPCREATESTRUCT) lParam;
			trans = (rvGETransformer*) cs->lpCreateParams;
			SetWindowLong ( hWnd, GWL_USERDATA, (LONG)trans );
			
			trans->mWnd = hWnd;
			trans->mDlg = CreateDialogParam ( gApp.GetInstance(), MAKEINTRESOURCE(IDD_GUIED_TRANSFORMER), 
											  hWnd, DlgProc, (LPARAM)trans );
			
			RECT rDlg;
			RECT rWindow;
			RECT rClient;
			
			GetWindowRect ( trans->mWnd, &rWindow );
			GetClientRect ( trans->mWnd, &rClient );
			GetWindowRect ( trans->mDlg, &rDlg );

			SetWindowPos ( trans->mWnd, NULL, 0, 0, 
						   (rWindow.right-rWindow.left)-(rClient.right-rClient.left) + (rDlg.right-rDlg.left),
						   (rWindow.bottom-rWindow.top)-(rClient.bottom-rClient.top) + (rDlg.bottom-rDlg.top),
						   SWP_NOZORDER );

			ShowWindow ( trans->mDlg, SW_SHOW );
			UpdateWindow ( trans->mDlg );
											  
			break;
		}
	}
	
	return DefWindowProc ( hWnd, msg, wParam, lParam );
}

INT_PTR CALLBACK rvGETransformer::DlgProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	rvGETransformer* trans = (rvGETransformer*) GetWindowLong ( hWnd, GWL_USERDATA );

	switch ( msg )
	{	
		case WM_DESTROY:
			if ( gTransHook )
			{
				UnhookWindowsHookEx( gTransHook );
			}
			gTransDlg = NULL;
			break;
		
		case WM_INITDIALOG:
			trans = (rvGETransformer*) lParam;
			trans->mDlg = hWnd;
			SetWindowLong ( hWnd, GWL_USERDATA, lParam );
			NumberEdit_Attach ( GetDlgItem ( hWnd, IDC_GUIED_ITEMRECTX ) );
			NumberEdit_Attach ( GetDlgItem ( hWnd, IDC_GUIED_ITEMRECTY ) );
			NumberEdit_Attach ( GetDlgItem ( hWnd, IDC_GUIED_ITEMRECTW ) );
			NumberEdit_Attach ( GetDlgItem ( hWnd, IDC_GUIED_ITEMRECTH ) );
			gTransDlg = hWnd;
			gTransHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc, NULL, GetCurrentThreadId() );
			break;
			
		case WM_COMMAND:
			if ( LOWORD ( wParam ) == IDOK )
			{
				SendMessage ( hWnd, WM_NEXTDLGCTL, 0, 0 );
			}
			else if ( HIWORD ( wParam ) == EN_KILLFOCUS && trans && trans->mWorkspace )
			{
				char temp[64];
				int  value;
				GetWindowText ( GetDlgItem ( hWnd, LOWORD(wParam) ), temp, 64 );
				value = atoi ( temp );
				
				idRectangle rect = trans->mWorkspace->GetSelectionMgr().GetRect ( );
				trans->mWorkspace->WindowToWorkspace ( rect );
				
				// The transformer coords are relative to the botto most selected window's parent so
				// adjust the rect accordingly
				if ( trans->mRelative )
				{
					idRectangle& screenRect = rvGEWindowWrapper::GetWrapper ( trans->mRelative )->GetScreenRect ( );
					rect.x -= screenRect.x;
					rect.y -= screenRect.y;
				}
				
				switch ( LOWORD ( wParam ) )
				{
					case IDC_GUIED_ITEMRECTX:		
						if ( value - rect[0] )
						{
							trans->mWorkspace->AddModifierMove ( "Transform Move", value - rect[0], 0, false );
						}
						break;

					case IDC_GUIED_ITEMRECTY:		
						if ( value - rect[1] )
						{
							trans->mWorkspace->AddModifierMove ( "Transform Move", 0, value - rect[1], false );
						}
						break;

					case IDC_GUIED_ITEMRECTW:		
						if ( value - rect[2] )
						{
							trans->mWorkspace->AddModifierSize ( "Transform Size", 0, 0, value - rect[2], 0, false );
						}
						break;

					case IDC_GUIED_ITEMRECTH:		
						if ( value - rect[3] )
						{
							trans->mWorkspace->AddModifierSize ( "Transform Size", 0, 0, 0, value - rect[3], false );
						}
						break;
				}
			}
			break;
	}
	
	return FALSE;
}

/*
================
rvGETransformer::Show

Shows and hides the transformer window
================
*/
void rvGETransformer::Show ( bool visible )
{
	gApp.GetOptions().SetTransformerVisible ( visible );				
	ShowWindow ( mWnd, visible?SW_SHOW:SW_HIDE );
	Update ( );
}

/*
================
rvGETransformer::SetWorkspace

Sets a new workspace for the transformer window
================
*/
void rvGETransformer::SetWorkspace ( rvGEWorkspace* workspace )
{
	mWorkspace = workspace;

	Update ( );	
}

/*
================
rvGETransformer::Update

Update the enabled/disabled states based on the selections and update
the rectangle coordinates
================
*/
void rvGETransformer::Update ( void )
{
	bool state = false;
	
	mRelative = NULL;
	
	if ( mWorkspace && mWorkspace->GetSelectionMgr ( ).Num ( ) )
	{
		state = true;
		mRelative = mWorkspace->GetSelectionMgr().GetBottomMost ( );
		mRelative = mRelative->GetParent ( );
		
		idRectangle rect = mWorkspace->GetSelectionMgr ( ).GetRect ( );
		mWorkspace->WindowToWorkspace ( rect );

		// Make the rectangle relative to the given parent
		if ( mRelative )
		{
			idRectangle& screenRect = rvGEWindowWrapper::GetWrapper ( mRelative )->GetScreenRect ( );
			rect.x -= screenRect.x;
			rect.y -= screenRect.y;
		}
		
		SetWindowText ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTX ), va("%d",(int)rect[0]) );
		SetWindowText ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTY ), va("%d",(int)rect[1]) );
		SetWindowText ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTW ), va("%d",(int)rect[2]) );
		SetWindowText ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTH ), va("%d",(int)rect[3]) );
	}

	if ( !state )
	{
		SetWindowText ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTX ), "" );
		SetWindowText ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTY ), "" );
		SetWindowText ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTW ), "" );
		SetWindowText ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTH ), "" );
	}
	
	EnableWindow ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTX ), state );
	EnableWindow ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTY ), state );
	EnableWindow ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTW ), state );
	EnableWindow ( GetDlgItem ( mDlg, IDC_GUIED_ITEMRECTH ), state );
}

/*
================
rvGETransformer::GetMsgProc

Ensures normal dialog functions work in the transformer dialog
================
*/
LRESULT FAR PASCAL rvGETransformer::GetMsgProc ( int nCode, WPARAM wParam, LPARAM lParam )
{
   LPMSG lpMsg = (LPMSG) lParam;

   if ( nCode >= 0 && PM_REMOVE == wParam )
   {
      // Don't translate non-input events.
      if ( lpMsg->message != WM_SYSCHAR && (lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) )
      {
         if ( IsDialogMessage( gTransDlg, lpMsg) )
         {
			// The value returned from this hookproc is ignored, 
			// and it cannot be used to tell Windows the message has been handled.
			// To avoid further processing, convert the message to WM_NULL 
			// before returning.
			lpMsg->message = WM_NULL;
			lpMsg->lParam  = 0;
			lpMsg->wParam  = 0;
         }
      }
   }

   return CallNextHookEx(gTransHook, nCode, wParam, lParam);
} 

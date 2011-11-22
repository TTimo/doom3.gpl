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

#include <io.h>

#include "../../sys/win32/rc/guied_resource.h"
#include "../../ui/DeviceContext.h"

#include "GEApp.h"
#include "GEOptionsDlg.h"
#include "GEViewer.h"

static const int IDM_WINDOWCHILD	= 1000;
static const int ID_GUIED_FILE_MRU1 = 10000;

static INT_PTR CALLBACK AboutDlg_WndProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_COMMAND:
			EndDialog ( hwnd, 1 );
			break;			
	}
	
	return FALSE;
}


rvGEApp::rvGEApp ( )
{
	mMDIFrame			 = NULL;
	mMDIClient			 = NULL;
	mRecentFileMenu		 = NULL;
	mViewer				 = NULL;
	mRecentFileInsertPos = 0;
}

rvGEApp::~rvGEApp ( )
{
	DestroyAcceleratorTable ( mAccelerators );
}

/*
================
rvGEApp::Initialize

Initialize the gui editor application
================
*/
bool rvGEApp::Initialize ( void )
{
	mOptions.Init();

	// Mutually exclusive
	com_editors = EDITOR_GUI;

	Sys_GrabMouseCursor( false );
	
	// Load the options
	mOptions.Load ( );	

	mInstance = win32.hInstance;

	// Create the accelerators
	mAccelerators = LoadAccelerators ( mInstance, MAKEINTRESOURCE(IDR_GUIED_ACCELERATORS) );

	// Register the window classes for the main frame and the mdi child window	
	WNDCLASSEX wndClass;
	memset ( &wndClass, 0, sizeof(wndClass) );
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpszClassName	= "QUAKE4_GUIEDITOR_CLASS";		
	wndClass.lpfnWndProc	= FrameWndProc;
	wndClass.hbrBackground	= (HBRUSH) (COLOR_APPWORKSPACE + 1); 
	wndClass.hCursor		= LoadCursor((HINSTANCE) NULL, IDC_ARROW); 
	wndClass.lpszMenuName	= MAKEINTRESOURCE(IDR_GUIED_MAIN);
	wndClass.hInstance		= mInstance; 
	RegisterClassEx ( &wndClass );

	wndClass.lpszMenuName	= NULL;
	wndClass.lpfnWndProc	= MDIChildProc;
	wndClass.lpszClassName	= "QUAKE4_GUIEDITOR_CHILD_CLASS";
	wndClass.style			= CS_OWNDC|CS_DBLCLKS|CS_BYTEALIGNWINDOW|CS_VREDRAW|CS_HREDRAW;
	wndClass.hbrBackground	= (HBRUSH)GetStockObject( LTGRAY_BRUSH );  
	RegisterClassEx ( &wndClass );
	
	// Create the main window
	mMDIFrame = CreateWindow ( "QUAKE4_GUIEDITOR_CLASS", 
							  "Quake IV GUI Editor", 
							  WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS, 
							  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
							  NULL, NULL, mInstance, (LPVOID)this );

	if ( !mMDIFrame )
	{
		return false;
	}

	SetClassLong( mMDIFrame, GCL_HICON, ( LONG )LoadIcon( win32.hInstance, MAKEINTRESOURCE( IDI_GUIED ) ) );
	
	// Create the MDI window
	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu = GetSubMenu ( GetMenu ( mMDIFrame ), 5 );
	ccs.idFirstChild = IDM_WINDOWCHILD;
	mMDIClient = CreateWindow ( "MDICLIENT", NULL, 
								WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
								0, 0, 1000, 1000, 
								mMDIFrame, NULL, 
								mInstance, &ccs );

	if ( !mMDIClient )
	{
		DestroyWindow ( mMDIFrame );
		return false;
	}

	// hide the doom window by default
	::ShowWindow ( win32.hWnd, SW_HIDE );
	
	// Show both windows
	mOptions.GetWindowPlacement ( "mdiframe", mMDIFrame );
	ShowWindow ( mMDIFrame, SW_SHOW ); 
	UpdateWindow ( mMDIFrame );

	ShowWindow ( mMDIClient, SW_SHOW );
	UpdateWindow ( mMDIClient );
	
	return true;
}


/*
================
rvGEApp::GetActiveWorkspace

Retrieves the workspace pointer for the active workspace.  If there is no active
workspace then it will return NULL
================
*/
rvGEWorkspace* rvGEApp::GetActiveWorkspace ( HWND* ret )
{
    rvGEWorkspace*	workspace;
    HWND			active;
	
	workspace = NULL;
	active    = (HWND)SendMessage ( mMDIClient, WM_MDIGETACTIVE, 0, NULL );

	// Return the window handle if requested
	if ( ret )
	{
		*ret = active;
	}

	if ( !active )
	{
		return NULL;
	}
	
	return rvGEWorkspace::GetWorkspace ( active );
}

/*
================
rvGEApp::TranslateAccelerator

Translate any accelerators destined for this window
================
*/
bool rvGEApp::TranslateAccelerator ( LPMSG msg )
{
	HWND focus;

	if ( msg->message == WM_SYSCHAR )
	{
		SetFocus ( GetMDIClient ( ) );	
		msg->hwnd = GetMDIClient ( );	
	}

	if ( mViewer )
	{
		return false;
	}

	focus = GetActiveWindow ( );

	// Only use accelerators when on the main window or navigator window
	if ( focus == mMDIClient || focus == mMDIFrame ||
		 focus == GetNavigator().GetWindow ( ) )
	{
		if ( ::TranslateAccelerator ( mMDIFrame, mAccelerators, msg ) )
		{
			return true;
		}
	}
	
	if ( TranslateMDISysAccel ( mMDIClient, msg ) )
	{
		return true;
	}
	
	return false;
}

/*
================
rvGEApp::RunFrame

Runs the current frame which causes the active window to be redrawn
================
*/
void rvGEApp::RunFrame ( void )
{
	HWND			wnd;
	rvGEWorkspace*	workspace = GetActiveWorkspace ( &wnd );

	if ( workspace )
	{		
		// Render the workspace using a temp DC
		HDC hDC = GetDC ( wnd );		
		workspace->Render ( hDC );		
		ReleaseDC ( wnd, hDC );				
		
		if ( mViewer )
		{
			mViewer->RunFrame ( );
		}
	}
}

/*
================
rvGEApp::FrameWndProc

Main frame window procedure
================
*/
LRESULT CALLBACK rvGEApp::FrameWndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	rvGEApp* app = (rvGEApp*) GetWindowLong ( hWnd, GWL_USERDATA );

	switch ( uMsg )
	{		
		case WM_SIZE:
		{
			RECT rStatus;
			RECT rClient;

			// Tell the status bar to resize 			
			app->mStatusBar.Resize ( LOWORD(lParam), HIWORD(lParam) );
				
			// Calculate the new client window rectangle (minus the status bar)
			GetWindowRect ( app->mStatusBar.GetWindow ( ), &rStatus );
			GetClientRect ( hWnd, &rClient );

			if ( app->mOptions.GetStatusBarVisible ( ) )
			{
				rClient.bottom -= (rStatus.bottom-rStatus.top);
			}
			MoveWindow ( app->mMDIClient, 0, 0, rClient.right-rClient.left, rClient.bottom-rClient.top, FALSE );
			
			return 0;
		}
		
		case WM_ENABLE:
		{	
			int i;

		    // Synchronise all toolwindows to the same state.
			for(i = 0; i < app->mToolWindows.Num(); i++)
			{
		        if(app->mToolWindows[i] != hWnd)
				{
					EnableWindow(app->mToolWindows[i], wParam);
				}
		    }
		    break;
		}		

		case WM_NCACTIVATE:
			return app->ToolWindowActivate ( hWnd, uMsg, wParam, lParam );

		case WM_ACTIVATE:
			common->ActivateTool( LOWORD( wParam ) != WA_INACTIVE );
			break;

		case WM_INITMENUPOPUP:
			app->HandleInitMenu ( wParam, lParam  );
			break;

		case WM_CLOSE:			
			while ( app->mWorkspaces.Num ( ) )
			{
				SendMessage ( app->mWorkspaces[0]->GetWindow ( ), WM_CLOSE, 0, 0 );
			}
			break;

		case WM_DESTROY:
			app->mOptions.SetWindowPlacement ( "mdiframe", hWnd );
			app->mOptions.Save ( );			
			ExitProcess(0);
			break;			

		case WM_COMMAND:
		{
			int result;
			assert ( app );
			result = app->HandleCommand ( wParam, lParam );
			if ( -1 != result )
			{
				return result;
			}			
			break;
		}

		case WM_CREATE:
		{	
			LPCREATESTRUCT		cs;
										
			cs = (LPCREATESTRUCT) lParam;
			app = (rvGEApp*)cs->lpCreateParams;

			assert ( app );
			
			SetWindowLong ( hWnd, GWL_USERDATA, (LONG)app );

			app->mMDIFrame = hWnd;
			
			app->InitRecentFiles ( );
			app->UpdateRecentFiles ( );
			app->mNavigator.Create ( hWnd, gApp.mOptions.GetNavigatorVisible ( ) );		
			app->mTransformer.Create ( hWnd, gApp.mOptions.GetTransformerVisible ( )  );
			app->mStatusBar.Create ( hWnd, 9999, gApp.mOptions.GetStatusBarVisible ( )  );
			app->mProperties.Create ( hWnd, gApp.mOptions.GetPropertiesVisible ( ) );

			// add all the tool windows to the tool window array
			app->mToolWindows.Append ( app->mMDIFrame );
			app->mToolWindows.Append ( app->mNavigator.GetWindow ( ) );
			app->mToolWindows.Append ( app->mProperties.GetWindow ( ) );
			app->mToolWindows.Append ( app->mTransformer.GetWindow ( ) );
			
			SendMessage ( app->mNavigator.GetWindow ( ), WM_NCACTIVATE, true, (LONG)-1 );
			SendMessage ( app->mProperties.GetWindow ( ), WM_NCACTIVATE, true, (LONG)-1 );
			SendMessage ( app->mTransformer.GetWindow ( ), WM_NCACTIVATE, true, (LONG)-1 );			

			break;
		}
	}

	return DefFrameProc ( hWnd, app?app->mMDIClient:NULL, uMsg, wParam, lParam );
}

/*
================
rvGEApp::MDIChildProc

MDI Child window procedure
================
*/
LRESULT CALLBACK rvGEApp::MDIChildProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	rvGEWorkspace* workspace = (rvGEWorkspace*)GetWindowLong ( hWnd, GWL_USERDATA );	

	// Give the active workspace a chance to play with it
	if ( workspace )
	{
		workspace->HandleMessage ( uMsg, wParam, lParam );
	}

	switch ( uMsg )
	{
		case WM_CLOSE:
			workspace->GetApplication ( )->mWorkspaces.Remove ( workspace );
			break;

		case WM_CREATE:
		{	
			LPMDICREATESTRUCT	mdics;
			LPCREATESTRUCT		cs;
					
			// MDI windows have their creation params buried two levels deep, extract
			// that param since it is the workspace pointer
			cs = (LPCREATESTRUCT) lParam;
			mdics = (LPMDICREATESTRUCT) cs->lpCreateParams;
			
			// Attach the workspace to the window
			workspace = (rvGEWorkspace*) mdics->lParam;
			workspace->Attach ( hWnd );			

			workspace->GetApplication ( )->mWorkspaces.Append ( workspace );				
				
			break;
		}

		case WM_MDIACTIVATE:
			assert ( workspace );
			if ( (HWND)lParam == hWnd )
			{				
				workspace->GetApplication ( )->GetNavigator().SetWorkspace(workspace);
				workspace->GetApplication ( )->GetTransformer().SetWorkspace(workspace);
				workspace->GetApplication ( )->GetProperties().SetWorkspace(workspace);
				gApp.GetStatusBar ( ).SetSimple ( false );
			}
			else if ( lParam == NULL )
			{
				gApp.GetStatusBar ( ).SetSimple ( true );
			}
			break;		

		case WM_DESTROY:
			assert ( workspace );
			workspace->Detach ( );
			delete workspace;
			break;

		case WM_SETCURSOR:
			return 1;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_PAINT:
		{
			HDC			dc;
			PAINTSTRUCT	ps;
			
			dc = BeginPaint(hWnd, &ps); 
			
			if ( workspace )
			{
				workspace->Render ( dc );
			}
			
            EndPaint(hWnd, &ps); 
           
			break;
		}
	}
	
	return DefMDIChildProc ( hWnd, uMsg, wParam, lParam );
}

/*
================
rvGEApp::HandleCommandSave

Handles the ID_GUIED_FILE_SAVE and ID_GUIED_FILE_SAVEAS commands
================
*/
void rvGEApp::HandleCommandSave ( rvGEWorkspace* workspace, const char* filename )
{
	idStr realFilename;
	
	// See if we need to browse for a filename
	if ( workspace->IsNew ( ) || filename == NULL )
	{
		OPENFILENAME ofn;
		char		 szFile[MAX_PATH];

		strcpy ( szFile, workspace->GetFilename ( ) );

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = mMDIFrame;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "GUI Files\0*.GUI\0All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST;

		// Display the save dialog box. 
		if ( !GetSaveFileName(&ofn) ) 
		{
			return;
		}

		realFilename = ofn.lpstrFile;
		realFilename.StripFileExtension ( );
		realFilename.Append ( ".gui" );

		// If the file already exists then warn about overwriting it
		if ( _taccess ( realFilename, 0 ) == 0 )
		{
			if ( IDNO == MessageBox ( va("File '%s' already exists. Do you want to replace it?", realFilename.c_str()), MB_YESNO|MB_ICONQUESTION ) )
			{
				return;
			}
		}
	}
	else
	{
		realFilename = filename;
	}	

	// Now performe the file save
	if ( workspace->SaveFile ( realFilename ) )
	{
		mOptions.AddRecentFile ( workspace->GetFilename ( ) );
		UpdateRecentFiles ( );
	}
	else
	{
		MessageBox ( va("Could not write file '%s'", workspace->GetFilename()), MB_OK|MB_ICONERROR );
	}
}

/*
================
rvGEApp::HandleCommand

Handles the WM_COMMAND message
================
*/
int rvGEApp::HandleCommand ( WPARAM wParam, LPARAM lParam )
{
	HWND		   active;
	rvGEWorkspace* workspace = GetActiveWorkspace ( &active );

	// The recent file list needs to be handled specially
	if ( LOWORD(wParam) >= ID_GUIED_FILE_MRU1 && LOWORD(wParam) < ID_GUIED_FILE_MRU1 + rvGEOptions::MAX_MRU_SIZE )
	{
		OpenFile ( mOptions.GetRecentFile ( mOptions.GetRecentFileCount() - (LOWORD(wParam)-ID_GUIED_FILE_MRU1) - 1 ) );
		return 0;
	}
					
	switch ( LOWORD ( wParam ) )
	{		
		case ID_GUIED_SOURCECONTROL_CHECKIN:
			assert ( workspace );
			HandleCommandSave ( workspace, workspace->GetFilename ( ) );
			workspace->CheckIn ( );
			break;
			
		case ID_GUIED_SOURCECONTROL_CHECKOUT:
			assert ( workspace );
			workspace->CheckOut ( );
			break;

		case ID_GUIED_SOURCECONTROL_UNDOCHECKOUT:
			assert ( workspace );
			if ( IDYES == MessageBox ( va("Are you sure you want to undo the checkout of the file '%s'?",workspace->GetFilename()), MB_YESNO|MB_ICONQUESTION) )
			{
				workspace->UndoCheckout ( );
			}
			break;
			
		case ID_GUIED_TOOLS_RELOADMATERIALS:
			SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_WAIT) ) );
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, "reloadImages\n" );
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, "reloadMaterials\n" );
			SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_ARROW) ) );
			break;
			
		case ID_GUIED_EDIT_COPY:
			assert ( workspace );
			workspace->Copy  ( );
			break;

		case ID_GUIED_EDIT_PASTE:
			assert ( workspace );
			workspace->Paste  ( );
			break;
	
		case ID_GUIED_HELP_ABOUT:
			DialogBox ( GetInstance(), MAKEINTRESOURCE(IDD_GUIED_ABOUT), mMDIFrame, AboutDlg_WndProc );
			break;
	
		case ID_GUIED_TOOLS_VIEWER:
		{
			if ( mViewer )
			{
				break;
			}
			
			mViewer = new rvGEViewer;
			if ( !mViewer->Create ( mMDIFrame ) )
			{
				delete mViewer;
				mViewer = NULL;
			}
						
			if ( workspace )
			{				
				if ( !workspace->IsModified () || HandleCommand ( MAKELONG(ID_GUIED_FILE_SAVE,0), 0 ) )
				{
					mViewer->OpenFile ( workspace->GetFilename ( ) );
				}
			}
			
			SetActiveWindow ( mViewer->GetWindow ( ) );
			break;
		}
		
		case ID_GUIED_ITEM_MAKESAMESIZEWIDTH:
			assert ( workspace );
			workspace->MakeSelectedSameSize ( true, false );
			break;

		case ID_GUIED_ITEM_MAKESAMESIZEBOTH:
			assert ( workspace );
			workspace->MakeSelectedSameSize ( true, true );
			break;

		case ID_GUIED_ITEM_MAKESAMESIZEHEIGHT:
			assert ( workspace );
			workspace->MakeSelectedSameSize ( false, true );
			break;
	
		case ID_GUIED_ITEM_ALIGNLEFTS:
			assert ( workspace );
			workspace->AlignSelected ( rvGEWorkspace::ALIGN_LEFTS );
			break;

		case ID_GUIED_ITEM_ALIGNCENTERS:
			assert ( workspace );
			workspace->AlignSelected ( rvGEWorkspace::ALIGN_CENTERS );
			break;

		case ID_GUIED_ITEM_ALIGNRIGHTS:
			assert ( workspace );
			workspace->AlignSelected ( rvGEWorkspace::ALIGN_RIGHTS );
			break;

		case ID_GUIED_ITEM_ALIGNTOPS:
			assert ( workspace );
			workspace->AlignSelected ( rvGEWorkspace::ALIGN_TOPS );
			break;

		case ID_GUIED_ITEM_ALIGNMIDDLES:
			assert ( workspace );
			workspace->AlignSelected ( rvGEWorkspace::ALIGN_MIDDLES );
			break;

		case ID_GUIED_ITEM_ALIGNBOTTOMS:
			assert ( workspace );
			workspace->AlignSelected ( rvGEWorkspace::ALIGN_BOTTOMS );
			break;
			
		case ID_GUIED_ITEM_ARRANGESENDBACKWARD:
			assert ( workspace );
			workspace->SendSelectedBackward ( );
			break;			

		case ID_GUIED_ITEM_ARRANGESENDTOBACK:
			assert ( workspace );
			workspace->SendSelectedToBack( );
			break;			

		case ID_GUIED_ITEM_ARRANGEBRINGFORWARD:
			assert ( workspace );
			workspace->BringSelectedForward ( );
			break;			

		case ID_GUIED_ITEM_ARRANGEBRINGTOFRONT:
			assert ( workspace );
			workspace->BringSelectedToFront ( );
			break;			
			
		case ID_GUIED_ITEM_ARRANGEMAKECHILD:
			assert ( workspace );
			workspace->MakeSelectedAChild ( );
			break;
	
		case ID_GUIED_ITEM_PROPERTIES:
			assert ( workspace );
			workspace->EditSelectedProperties ( );
			break;

		case ID_GUIED_ITEM_SCRIPTS:
			assert ( workspace );
			workspace->EditSelectedScripts ( );
			break;
			
		case ID_GUIED_ITEM_NEWWINDOWDEF:
			assert ( workspace );
			workspace->AddWindow ( rvGEWindowWrapper::WT_NORMAL );
			break;

		case ID_GUIED_ITEM_NEWEDITDEF:
			assert ( workspace );
			workspace->AddWindow ( rvGEWindowWrapper::WT_EDIT );
			break;

		case ID_GUIED_ITEM_NEWHTMLDEF:
			assert ( workspace );
			workspace->AddWindow ( rvGEWindowWrapper::WT_HTML );
			break;

		case ID_GUIED_ITEM_NEWCHOICEDEF:
			assert ( workspace );
			workspace->AddWindow ( rvGEWindowWrapper::WT_CHOICE );
			break;

		case ID_GUIED_ITEM_NEWSLIDERDEF:
			assert ( workspace );
			workspace->AddWindow ( rvGEWindowWrapper::WT_SLIDER );
			break;

		case ID_GUIED_ITEM_NEWLISTDEF:
			assert ( workspace );
			workspace->AddWindow ( rvGEWindowWrapper::WT_LIST );
			break;

		case ID_GUIED_ITEM_NEWBINDDEF:
			assert ( workspace );
			workspace->AddWindow ( rvGEWindowWrapper::WT_BIND );
			break;

		case ID_GUIED_ITEM_NEWRENDERDEF:
			assert ( workspace );
			workspace->AddWindow ( rvGEWindowWrapper::WT_RENDER );
			break;
	
		case ID_GUIED_WINDOW_TILE:
			SendMessage ( mMDIClient, WM_MDITILE, 0, 0 );
			break;	

		case ID_GUIED_WINDOW_CASCADE:
			SendMessage ( mMDIClient, WM_MDICASCADE, 0, 0 );
			break;	

		case ID_GUIED_VIEW_STATUSBAR:
		{
			RECT rWindow;
			
			mStatusBar.Show ( mOptions.GetStatusBarVisible()?false:true );
			
			GetWindowRect ( mMDIFrame, &rWindow );
			SendMessage ( mMDIFrame, WM_SIZE, 0, MAKELONG ( rWindow.right-rWindow.left, rWindow.bottom-rWindow.top ) );			
			break;
		}

		case ID_GUIED_WINDOW_SHOWNAVIGATOR:
			mNavigator.Show ( mOptions.GetNavigatorVisible()?false:true );
			break;					
			
		case ID_GUIED_WINDOW_SHOWPROPERTIES:
			mProperties.Show ( mOptions.GetPropertiesVisible()?false:true );
			break;
			
		case ID_GUIED_WINDOW_SHOWTRANSFORMER:
			mTransformer.Show ( mOptions.GetTransformerVisible()?false:true  );
			break;					
			
		case ID_GUIED_EDIT_DELETE:
			assert ( workspace );
			workspace->DeleteSelected ( );
			break;

		case ID_GUIED_VIEW_HIDESELECTED:
			assert ( workspace );
			workspace->HideSelected ( );
			break;

		case ID_GUIED_VIEW_UNHIDESELECTED:
			assert ( workspace );
			workspace->UnhideSelected ( );
			break;
		
		case ID_GUIED_VIEW_SHOWHIDDEN:
			assert ( workspace );
			workspace->ShowHidden ( );
			break;
			
		case ID_GUIED_EDIT_UNDO:
			assert ( workspace );
			workspace->GetModifierStack().Undo ( );
			mNavigator.Update ( );
			mTransformer.Update ( );
			break;

		case ID_GUIED_EDIT_REDO:
			assert ( workspace );
			workspace->GetModifierStack().Redo ( );
			mNavigator.Update ( );
			mTransformer.Update ( );
			break;
	
		case ID_GUIED_VIEW_OPTIONS:
			GEOptionsDlg_DoModal ( mMDIFrame );
			break;
	
		case ID_GUIED_VIEW_SHOWGRID:
			mOptions.SetGridVisible ( mOptions.GetGridVisible()?false:true );				
			break;

		case ID_GUIED_VIEW_SNAPTOGRID:
			mOptions.SetGridSnap ( mOptions.GetGridSnap ()?false:true );				
			break;
	
		case ID_GUIED_VIEW_ZOOMIN:
			assert ( workspace );
			workspace->ZoomIn ( );
			break;
			
		case ID_GUIED_VIEW_ZOOMOUT:
			assert ( workspace );
			workspace->ZoomOut ( );
			break;
	
		case ID_GUIED_FILE_EXIT:
			DestroyWindow ( mMDIFrame );
			break;

		case ID_GUIED_FILE_CLOSE:
			if ( active )
			{
				assert ( workspace );
				SendMessage ( active, WM_CLOSE, 0, 0 );
			}
			break;
			
		case ID_GUIED_FILE_NEW:
			NewFile ( );
			break;

		case ID_GUIED_FILE_SAVE:
			assert ( workspace );
			HandleCommandSave ( workspace, workspace->GetFilename ( ) );
			break;

		case ID_GUIED_FILE_SAVEAS:
			assert ( workspace );
			HandleCommandSave ( workspace, NULL );
			break;			
			
		case ID_GUIED_FILE_OPEN:
		{
			OPENFILENAME ofn;
			char		 szFile[MAX_PATH] = "";

			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = mMDIFrame;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "GUI Files\0*.GUI\0All Files\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			// Display the Open dialog box. 
			if (GetOpenFileName(&ofn)==TRUE) 
			{			
				OpenFile ( ofn.lpstrFile );
			}
			break;
		}			
	}
	
	return -1;
}

/*
================
rvGEApp::HandleInitMenu

Handles the initialization of the main menu
================
*/
int rvGEApp::HandleInitMenu ( WPARAM wParam, LPARAM lParam ) 
{ 
    int				cMenuItems = GetMenuItemCount((HMENU)wParam); 
    int				nPos; 
    int				id; 
    UINT			flags; 
    rvGEWorkspace*	workspace;
    HMENU			hmenu;
 
	hmenu     = (HMENU) wParam;
	workspace = GetActiveWorkspace ( );

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
	
		// Menu items that are completely unrelated to the workspace
		switch ( id )
		{
			case ID_GUIED_VIEW_STATUSBAR:
				flags = MF_BYCOMMAND | (mOptions.GetStatusBarVisible()?MF_CHECKED:MF_UNCHECKED);
				CheckMenuItem ( hmenu, id, flags );
				break;				

			case ID_GUIED_WINDOW_SHOWNAVIGATOR:
				flags = MF_BYCOMMAND | (mOptions.GetNavigatorVisible()?MF_CHECKED:MF_UNCHECKED);
				CheckMenuItem ( hmenu, id, flags );
				break;				
			
			case ID_GUIED_WINDOW_SHOWPROPERTIES:
				flags = MF_BYCOMMAND | (mOptions.GetPropertiesVisible()?MF_CHECKED:MF_UNCHECKED);
				CheckMenuItem ( hmenu, id, flags );
				break;				

			case ID_GUIED_WINDOW_SHOWTRANSFORMER:
				flags = MF_BYCOMMAND | (mOptions.GetTransformerVisible()?MF_CHECKED:MF_UNCHECKED);
				CheckMenuItem ( hmenu, id, flags );
				break;				
		}
 
		// Handle the basic case where an item is disabled because
		// there is no workspace available
		if ( !workspace )
		{
			switch ( id )
			{
				case ID_GUIED_EDIT_UNDO:
				case ID_GUIED_EDIT_REDO:
				case ID_GUIED_VIEW_SHOWGRID:
				case ID_GUIED_VIEW_SNAPTOGRID:
				case ID_GUIED_VIEW_HIDESELECTED:
				case ID_GUIED_VIEW_UNHIDESELECTED:
				case ID_GUIED_EDIT_DELETE:
				case ID_GUIED_WINDOW_TILE:
				case ID_GUIED_WINDOW_CASCADE:
				case ID_GUIED_ITEM_NEWWINDOWDEF:
				case ID_GUIED_ITEM_NEWEDITDEF:
				case ID_GUIED_ITEM_NEWHTMLDEF:
				case ID_GUIED_ITEM_ARRANGEBRINGTOFRONT:
				case ID_GUIED_ITEM_ARRANGEBRINGFORWARD:
				case ID_GUIED_ITEM_ARRANGESENDTOBACK:
				case ID_GUIED_ITEM_ARRANGESENDBACKWARD:
				case ID_GUIED_ITEM_PROPERTIES:
				case ID_GUIED_ITEM_SCRIPTS:
				case ID_GUIED_VIEW_ZOOMIN:
				case ID_GUIED_VIEW_ZOOMOUT:
				case ID_GUIED_ITEM_ALIGNLEFTS:
				case ID_GUIED_ITEM_ALIGNCENTERS:
				case ID_GUIED_ITEM_ALIGNRIGHTS:
				case ID_GUIED_ITEM_ALIGNBOTTOMS:
				case ID_GUIED_ITEM_ALIGNMIDDLES:
				case ID_GUIED_ITEM_ALIGNTOPS:
				case ID_GUIED_ITEM_MAKESAMESIZEHEIGHT:
				case ID_GUIED_ITEM_MAKESAMESIZEWIDTH:
				case ID_GUIED_ITEM_MAKESAMESIZEBOTH:
				case ID_GUIED_FILE_SAVE:
				case ID_GUIED_FILE_SAVEAS:
				case ID_GUIED_EDIT_COPY:
				case ID_GUIED_EDIT_PASTE:		
				case ID_GUIED_ITEM_ARRANGEMAKECHILD:
				case ID_GUIED_SOURCECONTROL_GETLATESTVERSION:
				case ID_GUIED_SOURCECONTROL_CHECKIN:
				case ID_GUIED_SOURCECONTROL_CHECKOUT:
				case ID_GUIED_SOURCECONTROL_UNDOCHECKOUT:
				case ID_GUIED_FILE_CLOSE:
					EnableMenuItem ( hmenu, nPos, MF_GRAYED|MF_BYPOSITION );
					break;
			}

			continue;
		}

        switch (id) 
        { 
			// Undo is greyed out when there is noting to undo and the text is
			// modified to include the name of the modifier that will be undone
			case ID_GUIED_EDIT_UNDO:
			{
				MENUITEMINFO info;
				idStr		 undo;

				info.cbSize = sizeof(info);
				info.fMask = MIIM_STATE|MIIM_TYPE;
				info.fType = MFT_STRING;
				
				if ( !workspace->GetModifierStack().CanUndo ( ) )
				{				
					undo = "Undo\tCtrl+Z";
					info.fState = MFS_GRAYED;
				}
				else
				{
					undo = "Undo ";
					undo.Append ( workspace->GetModifierStack().GetUndoModifier()->GetName ( ) );
					undo.Append ( "\tCtrl+Z" );
					info.fState = MFS_ENABLED;
				}

				info.dwTypeData = (LPSTR)undo.c_str();
				info.cch = undo.Length ( );
									
				SetMenuItemInfo ( hmenu, id, FALSE, &info );

				break;
			}

			case ID_GUIED_EDIT_REDO:
			{
				MENUITEMINFO info;
				idStr		 undo;

				info.cbSize = sizeof(info);
				info.fMask = MIIM_STATE|MIIM_TYPE;
				info.fType = MFT_STRING;
				
				if ( !workspace || !workspace->GetModifierStack().CanRedo ( ) )
				{				
					undo = "Redo\tCtrl+Y";
					info.fState = MFS_GRAYED;
				}
				else
				{
					undo = "Redo ";
					undo.Append ( workspace->GetModifierStack().GetRedoModifier()->GetName ( ) );
					undo.Append ( "\tCtrl+Y" );
					info.fState = MFS_ENABLED;
				}

				info.dwTypeData = (LPSTR)undo.c_str();
				info.cch = undo.Length ( );
									
				SetMenuItemInfo ( hmenu, id, FALSE, &info );

				break;
			}
				
			case ID_GUIED_VIEW_SHOWGRID:
				flags = MF_BYCOMMAND | (mOptions.GetGridVisible()?MF_CHECKED:MF_UNCHECKED);
				CheckMenuItem ( hmenu, id, flags );
				break;				

			case ID_GUIED_VIEW_SNAPTOGRID:
				flags = MF_BYCOMMAND | (mOptions.GetGridSnap()?MF_CHECKED:MF_UNCHECKED);
				CheckMenuItem ( hmenu, id, flags );
				break;				
			
			// All menu items that are greyed out when there is no workspace
			case ID_GUIED_WINDOW_TILE:
			case ID_GUIED_WINDOW_CASCADE:
			case ID_GUIED_ITEM_NEWWINDOWDEF:
			case ID_GUIED_ITEM_NEWEDITDEF:
			case ID_GUIED_ITEM_NEWHTMLDEF:
			case ID_GUIED_VIEW_ZOOMIN:
			case ID_GUIED_VIEW_ZOOMOUT:
			case ID_GUIED_FILE_SAVE:
			case ID_GUIED_FILE_SAVEAS:
			case ID_GUIED_FILE_CLOSE:
				EnableMenuItem ( hmenu, nPos, MF_ENABLED|MF_BYPOSITION);
				break;

			// All menu items that are greyed out unless an item is selected
			case ID_GUIED_VIEW_HIDESELECTED:
			case ID_GUIED_VIEW_UNHIDESELECTED:
			case ID_GUIED_EDIT_DELETE:
			case ID_GUIED_EDIT_COPY:
				EnableMenuItem ( hmenu, nPos, MF_BYPOSITION|(workspace->GetSelectionMgr().Num()>0?MF_ENABLED:MF_GRAYED) );
				break;

			// Enable paste if the clipboard has something in it
			case ID_GUIED_EDIT_PASTE:		
				EnableMenuItem ( hmenu, nPos, MF_BYPOSITION|(workspace->GetClipboard().Num()>0?MF_ENABLED:MF_GRAYED) );
				break;
			
			// All menu items that are greyed out unless a single item is selected
			case ID_GUIED_ITEM_ARRANGEBRINGTOFRONT:
			case ID_GUIED_ITEM_ARRANGEBRINGFORWARD:
			case ID_GUIED_ITEM_ARRANGESENDTOBACK:
			case ID_GUIED_ITEM_ARRANGESENDBACKWARD:
			case ID_GUIED_ITEM_PROPERTIES:
			case ID_GUIED_ITEM_SCRIPTS:
				EnableMenuItem ( hmenu, nPos, MF_BYPOSITION|(workspace->GetSelectionMgr().Num()==1?MF_ENABLED:MF_GRAYED) );
				break;

			// All menu items that are greyed out unless multiple itmes are selected
			case ID_GUIED_ITEM_ALIGNLEFTS:
			case ID_GUIED_ITEM_ALIGNCENTERS:
			case ID_GUIED_ITEM_ALIGNRIGHTS:
			case ID_GUIED_ITEM_ALIGNBOTTOMS:
			case ID_GUIED_ITEM_ALIGNMIDDLES:
			case ID_GUIED_ITEM_ALIGNTOPS:
			case ID_GUIED_ITEM_MAKESAMESIZEHEIGHT:
			case ID_GUIED_ITEM_MAKESAMESIZEBOTH:
			case ID_GUIED_ITEM_MAKESAMESIZEWIDTH:
			case ID_GUIED_ITEM_ARRANGEMAKECHILD:
				EnableMenuItem ( hmenu, nPos, MF_BYPOSITION|(workspace->GetSelectionMgr().Num()>1?MF_ENABLED:MF_GRAYED) );
				break;

			case ID_GUIED_SOURCECONTROL_CHECKIN:
			case ID_GUIED_SOURCECONTROL_UNDOCHECKOUT:
				EnableMenuItem ( hmenu, nPos, MF_BYPOSITION|((workspace->GetSourceControlState()==rvGEWorkspace::SCS_CHECKEDOUT)?MF_ENABLED:MF_GRAYED) );
				break;

			case ID_GUIED_SOURCECONTROL_CHECKOUT:
				EnableMenuItem ( hmenu, nPos, MF_BYPOSITION|((workspace->GetSourceControlState()==rvGEWorkspace::SCS_CHECKEDIN)?MF_ENABLED:MF_GRAYED) );
				break;
			
			default:
				continue;
		}				
	}
	
	return 0;
} 

/*
================
rvGEApp::NewFile

Creates a new file and opens a window for it
================
*/
bool rvGEApp::NewFile ( void )
{
	rvGEWorkspace* workspace = new rvGEWorkspace ( this );
	if ( workspace->NewFile ( ) )
	{
		HWND child;
		
		child = CreateMDIWindow("QUAKE4_GUIEDITOR_CHILD_CLASS",
								"Untitled.gui",
								WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_HSCROLL|WS_VSCROLL|WS_MAXIMIZE,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								640,
								480,
								mMDIClient,
								mInstance,
								(LONG)workspace );
														
		ShowWindow ( child, SW_SHOW );
	}

	return true;
}

/*
================
rvGEApp::OpenFile

Opens the given file and will fail if its already open or could not
be opened for some reason
================
*/
bool rvGEApp::OpenFile ( const char* filename )
{
	int i;
	bool result = false;
	idStr error;

	// See if the file is already open and if so just make it active
	for ( i = 0; i < mWorkspaces.Num(); i ++ )
	{
		if ( !idStr::Icmp ( mWorkspaces[i]->GetFilename(), filename ) )
		{
			SendMessage ( mMDIClient, WM_MDIACTIVATE, (WPARAM)mWorkspaces[i]->GetWindow ( ), 0 );
			return false;
		}
	}

	SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_WAIT ) ) );

	// Setup the default error.
	error = va("Failed to parse '%s'", filename );

	rvGEWorkspace* workspace = new rvGEWorkspace ( this );
	if ( workspace->LoadFile ( filename, &error ) )
	{
		HWND child;
			
		child = CreateMDIWindow("QUAKE4_GUIEDITOR_CHILD_CLASS",
								"Unamed.gui",
								WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_HSCROLL|WS_VSCROLL|WS_MAXIMIZE,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								640,
								480,
								mMDIClient,
								mInstance,
								(LONG)workspace );
														
		ShowWindow ( child, SW_SHOW );
		
		mOptions.AddRecentFile ( filename ); 
		UpdateRecentFiles ( );

		result = true;
	}
	else
	{
		MessageBox ( error, MB_OK|MB_ICONERROR );
	}

	SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_ARROW ) ) );
	
	return result;;
}

/*
================
rvGEApp::InitRecentFiles

Finds the file menu and the location within it where the MRU should
be added.
================
*/
bool rvGEApp::InitRecentFiles ( void )
{
	int	i;
	int count;

	mRecentFileMenu = GetSubMenu ( GetMenu(mMDIFrame), 0 );
	count			= GetMenuItemCount ( mRecentFileMenu );
	
	for ( i = 0; i < count; i ++ )
	{
		if ( GetMenuItemID ( mRecentFileMenu, i ) == ID_GUIED_FILE_MRU )
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
rvGEApp::UpdateRecentFiles

Updates the mru in the menu
================
*/
void rvGEApp::UpdateRecentFiles ( void )
{
	int i;
	int j;

	// Make sure everything is initialized
	if ( !mRecentFileMenu )	
	{	
		InitRecentFiles ( );
	}

	// Delete all the old recent files from the menu's
	for ( i = 0; i < rvGEOptions::MAX_MRU_SIZE; i ++ )
	{
		DeleteMenu ( mRecentFileMenu, ID_GUIED_FILE_MRU1 + i, MF_BYCOMMAND );
	}	

	// Make sure there is a separator after the recent files
	if ( mOptions.GetRecentFileCount() )
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
	for ( j = 0, i = mOptions.GetRecentFileCount ( ) - 1; i >= 0; i --, j++ )
	{
		UINT id = ID_GUIED_FILE_MRU1 + j;
		idStr str = va("&%d ", j+1);
		str.Append ( mOptions.GetRecentFile ( i ) );
		InsertMenu ( mRecentFileMenu, mRecentFileInsertPos+j+1, MF_BYPOSITION|MF_STRING|MF_ENABLED, id, str );
	}		
}

/*
================
rvGEApp::CloseViewer

Closes the gui viewer
================
*/
void rvGEApp::CloseViewer ( void )
{
	if ( !mViewer )
	{
		return;
	}
	
	mViewer->Destroy ( );
	delete mViewer;
	mViewer = NULL;
}

/*
================
rvGEApp::ToolWindowActivate

Handles the nc activate message for all tool windows
================
*/
int	rvGEApp::ToolWindowActivate ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	bool	keepActive;
	bool	syncOthers;
	int		i;
	
	keepActive = ( wParam != 0 );
	syncOthers = true;
	
	for ( i = 0; i < mToolWindows.Num (); i ++ )
	{
		if ( (HWND)lParam == mToolWindows[i] ) 
		{
			keepActive = true;
			syncOthers = false;
			break;
		}
	}
	
	if ( lParam == -1 )
	{
		return DefWindowProc ( hwnd, WM_NCACTIVATE, keepActive, 0 );
	}

	if ( syncOthers )
	{
		for ( i = 0; i < mToolWindows.Num(); i ++ )
		{
			if ( mToolWindows[i] != hwnd &&	mToolWindows[i] != (HWND) lParam )
			{
				SendMessage ( mToolWindows[i], WM_NCACTIVATE, keepActive, (LONG)-1 );
			}
		}
	}
	
	return DefWindowProc ( hwnd, WM_NCACTIVATE, keepActive, lParam );
}

/*
================
rvGEApp::MessageBox

Displays a modal message box 
================
*/
int rvGEApp::MessageBox ( const char* text, int flags )
{
	return ::MessageBox ( mMDIFrame, text, "Quake 4 GUI Editor", flags );
}


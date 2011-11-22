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
#include "../../sys/win32/rc/common_resource.h"
#include "OpenFileDialog.h"

char rvOpenFileDialog::mLookin[ MAX_OSPATH ];

/*
================
rvOpenFileDialog::rvOpenFileDialog

constructor
================
*/
rvOpenFileDialog::rvOpenFileDialog( void )
{
	mWnd		= NULL;
	mInstance	= NULL;
	mBackBitmap = NULL;
	mImageList	= NULL;
	mFlags		= 0;
}

/*
================
rvOpenFileDialog::~rvOpenFileDialog

destructor
================
*/
rvOpenFileDialog::~rvOpenFileDialog ( void )
{
	if ( mImageList )
	{
		ImageList_Destroy ( mImageList );
	}

	if ( mBackBitmap )
	{
		DeleteObject ( mBackBitmap );
	}
}

/*
================
rvOpenFileDialog::DoModal

Opens the dialog and returns true if a filename was found
================
*/
bool rvOpenFileDialog::DoModal ( HWND parent )
{
	mInstance = win32.hInstance;

	INITCOMMONCONTROLSEX ex;
	ex.dwICC = ICC_USEREX_CLASSES | ICC_LISTVIEW_CLASSES;
	ex.dwSize = sizeof(INITCOMMONCONTROLSEX);

	InitCommonControlsEx ( &ex );

	return DialogBoxParam ( mInstance, MAKEINTRESOURCE(IDD_TOOLS_OPEN), parent, DlgProc, (LPARAM)this ) ? true : false;
}

/*
================
rvOpenFileDialog::UpdateLookIn

Updates the lookin combo box with the current lookin state
================
*/
void rvOpenFileDialog::UpdateLookIn ( void )
{
	COMBOBOXEXITEM	item;
	idStr			file;
	idStr			path;

	// Reset the combo box
	SendMessage ( mWndLookin, CB_RESETCONTENT, 0, 0 );
	
	// Setup the common item structure components
	ZeroMemory ( &item, sizeof(item) );
	item.mask = CBEIF_TEXT | CBEIF_INDENT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;

	// Add the top left folder
	item.pszText = (LPSTR)"base";
	SendMessage ( mWndLookin, CBEM_INSERTITEM, 0, (LPARAM)&item );

	// Break the lookin path up into its individual components and add them
	// to the combo box
	path = mLookin;

	while ( path.Length ( ) )
	{
		int slash = path.Find ( "/" );
		
		// Parse out the next subfolder
		if ( slash != -1 )
		{
			file = path.Left ( slash );
			path = path.Right ( path.Length ( ) - slash - 1 );
		}
		else
		{
			file = path;
			path.Empty ( );
		}

		// Add the sub folder				
		item.pszText = (LPSTR)file.c_str();
		item.iIndent++;
		item.iItem = item.iIndent;
		SendMessage ( mWndLookin, CBEM_INSERTITEM, 0, (LPARAM)&item );		
	}
	
	// Set the selection to the last one since thats the deepest folder
	SendMessage ( mWndLookin, CB_SETCURSEL, item.iIndent, 0 );
}

/*
================
rvOpenFileDialog::UpdateFileList

Updates the file list with the files that match the filter in the current
look in directory
================
*/
void rvOpenFileDialog::UpdateFileList ( void )
{
	const char *basepath = mLookin;
	idFileList *files;
	HWND		list = GetDlgItem ( mWnd, IDC_TOOLS_FILELIST );
	int			i;
	int			filter;
		
	ListView_DeleteAllItems ( list );
	
	// Add all the folders first
	files = fileSystem->ListFiles ( basepath, "/", true );
	for ( i = 0; i < files->GetNumFiles(); i ++ )
	{
		if ( files->GetFile( i )[0] == '.' )
		{
			continue;
		}
	
		LVITEM item;
		item.mask = LVIF_TEXT;
		item.iItem = ListView_GetItemCount ( list );
		item.pszText = (LPSTR)files->GetFile( i );
		item.iSubItem = 0;
		ListView_InsertItem ( list, &item );
	}
	fileSystem->FreeFileList( files );
	
	// Add all the files in the current lookin directory that match the
	// current filters.
	for ( filter = 0; filter < mFilters.Num(); filter ++ )
	{	
		files = fileSystem->ListFiles( basepath, mFilters[filter], true );
		for ( i = 0; i < files->GetNumFiles(); i ++ )
		{
			if ( files->GetFile( i )[0] == '.' )
			{
				continue;
			}
	
			LVITEM item;
			item.mask = LVIF_TEXT|LVIF_IMAGE;
			item.iImage = 2;
			item.iItem = ListView_GetItemCount( list );
			item.pszText = (LPSTR)files->GetFile( i );
			item.iSubItem = 0;
			ListView_InsertItem ( list, &item );
		}
		fileSystem->FreeFileList( files );
	}
}

/*
================
rvOpenFileDialog::HandleCommandOK

Handles the pressing of the OK button but either opening a selected folder
or closing the dialog with the resulting filename
================
*/
void rvOpenFileDialog::HandleCommandOK ( void )
{
	char	temp[256];
	LVITEM	item;

	// If nothing is selected then there is nothing to open
	int sel = ListView_GetNextItem ( mWndFileList, -1, LVNI_SELECTED );
	if ( sel == -1 )
	{
		GetWindowText ( GetDlgItem ( mWnd, IDC_TOOLS_FILENAME ), temp, sizeof(temp)-1 );
		if ( !temp[0] )
		{
			return;
		}
		
		item.iImage = 2;
	}
	else
	{
		// Get the currently selected item
		item.mask = LVIF_IMAGE|LVIF_TEXT;
		item.iImage = sel;
		item.iSubItem = 0;
		item.pszText = temp;
		item.cchTextMax = 256;
		item.iItem = sel;
		ListView_GetItem ( mWndFileList, &item );
	}
	
	// If the item is a folder then just open that folder
	if ( item.iImage == 0 )
	{
		if ( strlen( mLookin ) )
		{
			idStr::snPrintf( mLookin, sizeof( mLookin ), "%s/%s", mLookin, temp );
		} else {
			idStr::Copynz( mLookin, temp, sizeof( mLookin ) );
		}
		UpdateLookIn ( );
		UpdateFileList ( );									
	}
	// If the item is a file then build the filename and end the dialog
	else if ( item.iImage == 2 )
	{
		mFilename = mLookin;
		if ( mFilename.Length ( ) )
		{
			mFilename.Append ( "/" );
		}
		mFilename.Append ( temp );
		
		// Make sure the file exists
		if ( mFlags & OFD_MUSTEXIST )
		{
			idFile*	file;				
			file = fileSystem->OpenFileRead ( mFilename );
			if ( !file )
			{
				MessageBox ( mWnd, va("%s\nFile not found.\nPlease verify the correct file name was given", mFilename.c_str() ), "Open", MB_ICONERROR|MB_OK );
				return;
			}
			fileSystem->CloseFile ( file );
		}
				
		EndDialog ( mWnd, 1 );
	}		
	
	return;
}

/*
================
rvOpenFileDialog::HandleInitDialog

Handles the init dialog message
================
*/
void rvOpenFileDialog::HandleInitDialog ( void )
{
	// Cache the more used window handles
	mWndFileList = GetDlgItem ( mWnd, IDC_TOOLS_FILELIST );
	mWndLookin   = GetDlgItem ( mWnd, IDC_TOOLS_LOOKIN );

	// Load the custom resources used by the controls
	mImageList  = ImageList_LoadBitmap ( mInstance, MAKEINTRESOURCE(IDB_TOOLS_OPEN),16,1,RGB(255,255,255) );
	mBackBitmap = (HBITMAP)LoadImage ( mInstance, MAKEINTRESOURCE(IDB_TOOLS_BACK), IMAGE_BITMAP, 16, 16, LR_DEFAULTCOLOR|LR_LOADMAP3DCOLORS );

	// Attach the image list to the file list and lookin controls		
	ListView_SetImageList ( mWndFileList, mImageList, LVSIL_SMALL );
	SendMessage( mWndLookin,CBEM_SETIMAGELIST,0,(LPARAM) mImageList );
	
	// Back button is a bitmap button
	SendMessage( GetDlgItem ( mWnd, IDC_TOOLS_BACK ), BM_SETIMAGE, IMAGE_BITMAP, (LONG) mBackBitmap );
	
	// Allow custom titles
	SetWindowText ( mWnd, mTitle );
	
	// Custom ok button title
	if ( mOKTitle.Length ( ) )
	{
		SetWindowText ( GetDlgItem ( mWnd, IDOK ), mOKTitle );
	}

	// See if there is a filename in the lookin
	idStr temp;
	idStr filename = mLookin;
	filename.ExtractFileExtension ( temp );
	if ( temp.Length ( ) )
	{
		filename.ExtractFileName ( temp );
		SetWindowText ( GetDlgItem ( mWnd, IDC_TOOLS_FILENAME ), temp );
		filename.StripFilename ( );
		idStr::snPrintf( mLookin, sizeof( mLookin ), "%s", filename.c_str() );
	}

	// Update our controls
	UpdateLookIn ( );
	UpdateFileList ( );
}

/*
================
rvOpenFileDialog::HandleLookInChange

Handles a selection change within the lookin control
================
*/
void rvOpenFileDialog::HandleLookInChange ( void )
{
	char	temp[256];
	int		sel;						
	int		i;
	idStr	lookin;
	
	temp[0] = 0;
	
	sel = SendMessage ( mWndLookin, CB_GETCURSEL, 0, 0 );
	
	// If something other than base is selected then walk up the list
	// and build the new lookin path
	if ( sel >= 1 )
	{
		SendMessage ( mWndLookin, CB_GETLBTEXT, 1, (LPARAM)temp );
		idStr::snPrintf( mLookin, sizeof( mLookin ), "%s", temp );
		for ( i = 2; i <= sel; i ++ )
		{
			SendMessage ( mWndLookin, CB_GETLBTEXT, i, (LPARAM)temp );
			idStr::snPrintf( mLookin, sizeof( mLookin ), "%s/%s", mLookin, temp );
		}			
	}
	else
	{
		mLookin[0] = 0;
	}	

	// Update the controls with the new lookin path
	UpdateLookIn ( );									
	UpdateFileList ( );	
}

/*
================
rvOpenFileDialog::SetFilter

Set the extensions available in the dialog
================
*/
void rvOpenFileDialog::SetFilter ( const char* s )
{
	idStr filters = s;
	idStr filter;
	
	while ( filters.Length ( ) )
	{
		int semi = filters.Find ( ';' );
		if ( semi != -1 )
		{
			filter  = filters.Left ( semi );
			filters = filters.Right ( filters.Length ( ) - semi );
		}
		else
		{
			filter = filters;
			filters.Empty ( );
		}
		
		mFilters.Append ( filter.c_str() + (filter[0] == '*' ? 1 : 0) );
	}
}

/*
================
rvOpenFileDialog::DlgProc

Dialog Procedure for the open file dialog
================
*/
INT_PTR rvOpenFileDialog::DlgProc ( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	rvOpenFileDialog* dlg = (rvOpenFileDialog*) GetWindowLong ( wnd, GWL_USERDATA );
	
	switch ( msg )
	{
		case WM_INITDIALOG:			
			dlg = (rvOpenFileDialog*) lparam;
			SetWindowLong ( wnd, GWL_USERDATA, lparam );
			dlg->mWnd = wnd;
			dlg->HandleInitDialog ( );
			return TRUE;

		case WM_NOTIFY:
		{
			NMHDR* nm = (NMHDR*) lparam;
			switch ( nm->idFrom )
			{
				case IDC_TOOLS_FILELIST:
					switch ( nm->code )
					{
						case LVN_ITEMCHANGED:
						{
							NMLISTVIEW* nmlv = (NMLISTVIEW*)nm;
							if ( nmlv->uNewState & LVIS_SELECTED )
							{	
								// Get the currently selected item
								LVITEM item;
								char   temp[256];
								item.mask = LVIF_IMAGE|LVIF_TEXT;
								item.iSubItem = 0;
								item.pszText = temp;
								item.cchTextMax = sizeof(temp)-1;
								item.iItem = nmlv->iItem;
								ListView_GetItem ( dlg->mWndFileList, &item );				
								
								if ( item.iImage == 2 )
								{
									SetWindowText ( GetDlgItem ( wnd, IDC_TOOLS_FILENAME ), temp );
								}
							}
							break;
						}
						
						case NM_DBLCLK:
							dlg->HandleCommandOK ( );
							break;
					}
					break;
			}
			break;
		}
			
		case WM_COMMAND:
			switch ( LOWORD ( wparam ) )
			{
				case IDOK:
				{
					dlg->HandleCommandOK ( );
					break;
				}
				
				case IDCANCEL:
					EndDialog ( wnd, 0 );
					break;
					
				case IDC_TOOLS_BACK:
				{
					int sel = SendMessage ( GetDlgItem ( wnd, IDC_TOOLS_LOOKIN ), CB_GETCURSEL, 0, 0 );
					if ( sel > 0 )
					{
						sel--;
						SendMessage ( GetDlgItem ( wnd, IDC_TOOLS_LOOKIN ), CB_SETCURSEL, sel, 0 );
						dlg->HandleLookInChange ( );
					}
					
					break;
				}
					
				case IDC_TOOLS_LOOKIN:
					if ( HIWORD ( wparam ) == CBN_SELCHANGE )
					{
						dlg->HandleLookInChange ( );								
					}
					break;
			}
			break;
	}
	
	return FALSE;
}

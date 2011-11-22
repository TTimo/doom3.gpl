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
#include "../common/ColorButton.h"

#include "GEApp.h"

/*
================
GEOptionsDlg_GeneralProc

Dialog procedure for the general options tab
================
*/
static INT_PTR CALLBACK GEOptionsDlg_GeneralProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_INITDIALOG:
			ColorButton_SetColor ( GetDlgItem ( hwnd, IDC_GUIED_SELECTIONCOLOR ), 
									RGB(gApp.GetOptions().GetSelectionColor()[0]*255,
										gApp.GetOptions().GetSelectionColor()[1]*255,
										gApp.GetOptions().GetSelectionColor()[2]*255) );					
			CheckDlgButton ( hwnd, IDC_GUIED_IGNOREDESKTOP, gApp.GetOptions().GetIgnoreDesktopSelect()?BST_CHECKED:BST_UNCHECKED );
			break;

		case WM_COMMAND:
			switch ( LOWORD ( wParam ) )
			{
				case IDC_GUIED_SELECTIONCOLOR:
				{
					CHOOSECOLOR col;
					ZeroMemory ( &col, sizeof(col) );
					col.lStructSize = sizeof(col);
					col.lpCustColors = gApp.GetOptions().GetCustomColors ( );
					col.hwndOwner = hwnd;
					col.hInstance = NULL;
					col.Flags = CC_RGBINIT;
					col.rgbResult = ColorButton_GetColor ( GetDlgItem ( hwnd, IDC_GUIED_SELECTIONCOLOR ) );
					if ( ChooseColor ( &col ) )
					{
						ColorButton_SetColor ( GetDlgItem ( hwnd, IDC_GUIED_SELECTIONCOLOR ), col.rgbResult );
					}
					break;
				}
			}
			break;

		case WM_DRAWITEM:
			ColorButton_DrawItem ( GetDlgItem ( hwnd, wParam ), (LPDRAWITEMSTRUCT)lParam );			
			return TRUE;
	
		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code)
			{					
				case PSN_APPLY:									
					gApp.GetOptions().SetLastOptionsPage ( PropSheet_HwndToIndex ( GetParent ( hwnd ), PropSheet_GetCurrentPageHwnd ( GetParent ( hwnd ) ) ) );
					gApp.GetOptions().SetSelectionColor ( ColorButton_GetColor ( GetDlgItem ( hwnd, IDC_GUIED_SELECTIONCOLOR ) ) );
					gApp.GetOptions().SetIgnoreDesktopSelect ( IsDlgButtonChecked ( hwnd, IDC_GUIED_IGNOREDESKTOP ) != 0 );
					break;
			}
			break;
	}
	
	return FALSE;
}

/*
================
GEOptionsDlg_GridProc

Dialog procedure for the grid settings tab
================
*/
static INT_PTR CALLBACK GEOptionsDlg_GridProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_INITDIALOG:
			// Copy the options information to the dialog controls
			ColorButton_SetColor ( GetDlgItem ( hwnd, IDC_GUIED_GRIDCOLOR ), RGB(gApp.GetOptions().GetGridColor()[0]*255,gApp.GetOptions().GetGridColor()[1]*255,gApp.GetOptions().GetGridColor()[2]*255) );
			SetWindowText ( GetDlgItem ( hwnd, IDC_GUIED_SPACINGWIDTH ), va("%d", gApp.GetOptions().GetGridWidth ( ) ) );
			SetWindowText ( GetDlgItem ( hwnd, IDC_GUIED_SPACINGHEIGHT ), va("%d", gApp.GetOptions().GetGridHeight ( ) ) );
			CheckDlgButton ( hwnd, IDC_GUIED_GRIDVISIBLE, gApp.GetOptions().GetGridVisible()?BST_CHECKED:BST_UNCHECKED );
			CheckDlgButton ( hwnd, IDC_GUIED_GRIDSNAP, gApp.GetOptions().GetGridSnap()?BST_CHECKED:BST_UNCHECKED );
			return TRUE;
	
		case WM_DRAWITEM:
			ColorButton_DrawItem ( GetDlgItem ( hwnd, wParam ), (LPDRAWITEMSTRUCT)lParam );			
			return TRUE;
		
		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code)
			{
				case PSN_APPLY:
				{
					char temp[32];
					
					// Copy the dialog control data back to the options
					GetWindowText ( GetDlgItem ( hwnd, IDC_GUIED_SPACINGWIDTH ), temp, 32 );
					gApp.GetOptions().SetGridWidth(atol(temp));
					GetWindowText ( GetDlgItem ( hwnd, IDC_GUIED_SPACINGHEIGHT ), temp, 32 );
					gApp.GetOptions().SetGridHeight(atol(temp));
					gApp.GetOptions().SetGridVisible ( IsDlgButtonChecked ( hwnd, IDC_GUIED_GRIDVISIBLE ) != 0 );
					gApp.GetOptions().SetGridSnap ( IsDlgButtonChecked ( hwnd, IDC_GUIED_GRIDSNAP ) != 0 );
					gApp.GetOptions().SetGridColor ( ColorButton_GetColor ( GetDlgItem ( hwnd, IDC_GUIED_GRIDCOLOR ) ) );
					break;
				}
			}
			break;
		
		case WM_COMMAND:
			switch ( LOWORD ( wParam ) )
			{
				case IDC_GUIED_GRIDCOLOR:
				{
					CHOOSECOLOR col;
					ZeroMemory ( &col, sizeof(col) );
					col.lStructSize = sizeof(col);
					col.lpCustColors = gApp.GetOptions().GetCustomColors ( );
					col.hwndOwner = hwnd;
					col.hInstance = NULL;
					col.Flags = CC_RGBINIT;
					col.rgbResult = RGB(gApp.GetOptions().GetGridColor()[0]*255,gApp.GetOptions().GetGridColor()[1]*255,gApp.GetOptions().GetGridColor()[2]*255);
					if ( ChooseColor ( &col ) )
					{
						ColorButton_SetColor ( GetDlgItem ( hwnd, IDC_GUIED_GRIDCOLOR ), col.rgbResult );
					}
					break;
				}
			}
			return TRUE;
	}	

	return FALSE;
}

/*
================
GEOptionsDlg_DoModal

Starts the options dialog and updates the global options if ok is pressed
================
*/
bool GEOptionsDlg_DoModal ( HWND parent )
{
	PROPSHEETHEADER propsh;
	PROPSHEETPAGE	propsp[2];
	
	propsp[0].dwSize		= sizeof(PROPSHEETPAGE);
	propsp[0].dwFlags		= PSP_USETITLE;
	propsp[0].hInstance		= win32.hInstance;
	propsp[0].pszTemplate	= MAKEINTRESOURCE(IDD_GUIED_OPTIONS_GENERAL);
	propsp[0].pfnDlgProc	= GEOptionsDlg_GeneralProc;
	propsp[0].pszTitle		= "General";
	propsp[0].lParam		= 0;

	propsp[1].dwSize		= sizeof(PROPSHEETPAGE);
	propsp[1].dwFlags		= PSP_USETITLE;
	propsp[1].hInstance		= win32.hInstance;
	propsp[1].pszTemplate	= MAKEINTRESOURCE(IDD_GUIED_OPTIONS_GRID);
	propsp[1].pfnDlgProc	= GEOptionsDlg_GridProc;
	propsp[1].pszTitle		= "Grid";
	propsp[1].lParam		= 0;

	propsh.dwSize			= sizeof(PROPSHEETHEADER);
	propsh.nStartPage		= gApp.GetOptions().GetLastOptionsPage ( );
	propsh.dwFlags			= PSH_PROPSHEETPAGE|PSH_NOAPPLYNOW|PSH_NOCONTEXTHELP; 
	propsh.hwndParent		= parent; 
	propsh.pszCaption		= "Options";
	propsh.nPages			= 2;
	propsh.ppsp				= (LPCPROPSHEETPAGE)&propsp;
	
	if ( PropertySheet ( &propsh ) )
	{
		gApp.GetOptions().Save ( );
		return true;
	}
	
	return false;
}

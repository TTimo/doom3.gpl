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

#include "../common/ColorButton.h"
#include "GEApp.h"
#include "GEPropertyPage.h"

rvGEPropertyPage::rvGEPropertyPage ( )
{
	mPage = NULL;
}
	
/*
================
rvGEPropertyPage::WndProc

Window procedure for the property page class.
================
*/
INT_PTR CALLBACK rvGEPropertyPage::WndProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	rvGEPropertyPage* page = (rvGEPropertyPage*) GetWindowLong ( hwnd, GWL_USERDATA );
	
	// Pages dont get the init dialog since their Init method is called instead
	if ( msg == WM_INITDIALOG )
	{
		PROPSHEETPAGE* psp = (PROPSHEETPAGE*) lParam;
			
		page = (rvGEPropertyPage*) psp->lParam;
		
		SetWindowLong ( hwnd, GWL_USERDATA, (LONG)page );
		page->mPage = hwnd;
		
		page->Init ( );
		
		return FALSE;
	}
	else if ( !page )
	{
		return FALSE;
	}
	
	// See if the derived class wants to handle the message
	return page->HandleMessage ( msg, wParam, lParam );
}

/*
================
rvGEPropertyPage::HandleMessage

Handles all messages that the base property page must handle.
================
*/
int rvGEPropertyPage::HandleMessage ( UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code)
			{					
				case PSN_APPLY:		
					if ( !Apply ( ) )
					{
						SetWindowLong ( mPage, DWL_MSGRESULT, PSNRET_INVALID );
						return TRUE;
					}
					break;		
					
				case PSN_SETACTIVE:
					SetActive ( );
					break;
					
				case PSN_KILLACTIVE:
					KillActive ( );
					break;	
			}
			break;
	}
	
	return FALSE;
}


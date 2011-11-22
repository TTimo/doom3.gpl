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

#define MASKEDIT_MAXINVALID	1024
typedef struct
{
	WNDPROC	mProc;
	char	mInvalid[MASKEDIT_MAXINVALID];
} rvGEMaskEdit;

/*
================
MaskEdit_WndProc

Prevents the invalid characters from being entered
================
*/
LRESULT CALLBACK MaskEdit_WndProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	rvGEMaskEdit* edit = (rvGEMaskEdit*)GetWindowLong ( hWnd, GWL_USERDATA );
	WNDPROC		  wndproc = edit->mProc;

	switch ( msg )
	{
		case WM_CHAR:
			if ( strchr ( edit->mInvalid, wParam ) )
			{
				return 0;
			}
			
			break;
			
		case WM_DESTROY:
			delete edit;
			SetWindowLong ( hWnd, GWL_WNDPROC, (LONG)wndproc );
			break;
	}

	return CallWindowProc ( wndproc, hWnd, msg, wParam, lParam );
}

/*
================
MaskEdit_Attach

Attaches the mask edit control to a normal edit control
================
*/
void MaskEdit_Attach ( HWND hWnd, const char* invalid )
{
	rvGEMaskEdit* edit = new rvGEMaskEdit;
	edit->mProc = (WNDPROC)GetWindowLong ( hWnd, GWL_WNDPROC );
	strcpy ( edit->mInvalid, invalid );
	SetWindowLong ( hWnd, GWL_USERDATA, (LONG)edit );
	SetWindowLong ( hWnd, GWL_WNDPROC, (LONG)MaskEdit_WndProc );
}

/*
================
NumberEdit_Attach

Allows editing of floating point numbers
================
*/
void NumberEdit_Attach ( HWND hWnd )
{
	static const char invalid[] = "`~!@#$%^&*()_+|=\\qwertyuiop[]asdfghjkl;'zxcvbnm,/QWERTYUIOP{}ASDFGHJKL:ZXCVBNM<>";
	MaskEdit_Attach ( hWnd, invalid );
}

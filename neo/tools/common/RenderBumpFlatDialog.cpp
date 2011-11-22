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

#include "../../sys/win32/common_resource.h"

idCVar rbfg_DefaultWidth( "rbfg_DefaultWidth", "0", 0, "" );
idCVar rbfg_DefaultHeight( "rbfg_DefaultHeight", "0", 0, "" );

static idStr RBFName;

static bool CheckPow2(int Num)
{
	while(Num)
	{
		if ((Num & 1) && (Num != 1))
		{
			return false;
		}

		Num >>= 1;
	}

	return true;
}

extern void Com_WriteConfigToFile( const char *filename );

static BOOL CALLBACK RBFProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{  
    switch (message) 
    { 
        case WM_INITDIALOG: 
			SetDlgItemInt(hwndDlg, IDC_RBF_WIDTH, rbfg_DefaultWidth.GetInteger(), FALSE);
			SetDlgItemInt(hwndDlg, IDC_RBF_HEIGHT, rbfg_DefaultHeight.GetInteger(), FALSE);
			SetDlgItemText(hwndDlg, IDC_RBF_FILENAME, RBFName);
            return TRUE; 
 
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK: 
					{
						int		width, height;

						width = GetDlgItemInt(hwndDlg, IDC_RBF_WIDTH, 0, FALSE);
						height = GetDlgItemInt(hwndDlg, IDC_RBF_HEIGHT, 0, FALSE);

						rbfg_DefaultWidth.SetInteger( width );
						rbfg_DefaultHeight.SetInteger( height );

						Com_WriteConfigToFile( CONFIG_FILE );

						if (!CheckPow2(width) || !CheckPow2(height))
						{
							return TRUE;
						}

						DestroyWindow(hwndDlg); 

						cmdSystem->BufferCommandText( CMD_EXEC_APPEND, va("renderbumpflat -size %d %d %s\n", width, height, RBFName.c_str() ) );
	                    return TRUE; 
					}
 
                case IDCANCEL: 
                    DestroyWindow(hwndDlg); 
                    return TRUE; 
            } 
    }

    return FALSE; 
} 

void DoRBFDialog(const char *FileName)
{
	RBFName = FileName;

	Sys_GrabMouseCursor( false );

	DialogBox(0, MAKEINTRESOURCE(IDD_RENDERBUMPFLAT), 0, (DLGPROC)RBFProc); 

	Sys_GrabMouseCursor( true );
}

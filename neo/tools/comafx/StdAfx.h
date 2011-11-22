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

#ifndef __AFX_STDAFX_H__
#define __AFX_STDAFX_H__

//  include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently

//#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

void InitAfx( void );

// tool tips
typedef struct toolTip_s {
	int id;
	char *tip;
} toolTip_t;

int DefaultOnToolHitTest( const toolTip_t *toolTips, const CDialog *dialog, CPoint point, TOOLINFO* pTI );
BOOL DefaultOnToolTipNotify( const toolTip_t *toolTips, UINT id, NMHDR *pNMHDR, LRESULT *pResult );

// edit control
bool EditControlEnterHit( CEdit *edit );
float EditVerifyFloat( CEdit *edit, bool allowNegative = true );
float EditSpinFloat( CEdit *edit, bool up );

// combo box
int SetSafeComboBoxSelection( CComboBox *combo, const char *string, int skip );
int GetSafeComboBoxSelection( CComboBox *combo, CString &string, int skip );
int UnsetSafeComboBoxSelection( CComboBox *combo, CString &string );

#endif /* !__AFX_STDAFX_H__ */

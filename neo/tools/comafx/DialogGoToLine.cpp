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

#include "../../sys/win32/rc/Common_resource.h"

#include "DialogGoToLine.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif


IMPLEMENT_DYNAMIC(DialogGoToLine, CDialog)

/*
================
DialogGoToLine::DialogGoToLine
================
*/
DialogGoToLine::DialogGoToLine( CWnd* pParent /*=NULL*/ )
	: CDialog(DialogGoToLine::IDD, pParent)
	, firstLine(0)
	, lastLine(0)
	, line(0)
{
}

/*
================
DialogGoToLine::~DialogGoToLine
================
*/
DialogGoToLine::~DialogGoToLine() {
}

/*
================
DialogGoToLine::DoDataExchange
================
*/
void DialogGoToLine::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogGoToLine)
	DDX_Control( pDX, IDC_GOTOLINE_EDIT, numberEdit);
	//}}AFX_DATA_MAP
}

/*
================
DialogGoToLine::SetRange
================
*/
void DialogGoToLine::SetRange( int firstLine, int lastLine ) {
    this->firstLine = firstLine;
	this->lastLine = lastLine;
}

/*
================
DialogGoToLine::GetLine
================
*/
int DialogGoToLine::GetLine( void ) const {
	return line;
}

/*
================
DialogGoToLine::OnInitDialog
================
*/
BOOL DialogGoToLine::OnInitDialog()  {

	CDialog::OnInitDialog();

	GetDlgItem( IDC_GOTOLINE_STATIC )->SetWindowText( va( "&Line number (%d - %d):", firstLine, lastLine ) );

	numberEdit.SetWindowText( va( "%d", firstLine ) );
	numberEdit.SetSel( 0, -1 );
	numberEdit.SetFocus();

	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(DialogGoToLine, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// DialogGoToLine message handlers

/*
================
DialogGoToLine::OnBnClickedOk
================
*/
void DialogGoToLine::OnBnClickedOk() {
	CString text;
	numberEdit.GetWindowText( text );
	line = idMath::ClampInt( firstLine, lastLine, atoi( text ) );
	OnOK();
}

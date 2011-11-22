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

#include "../../sys/win32/rc/AFEditor_resource.h"

#include "DialogAF.h"
#include "DialogAFName.h"

// DialogAFName dialog

IMPLEMENT_DYNAMIC(DialogAFName, CDialog)

/*
================
DialogAFName::DialogAFName
================
*/
DialogAFName::DialogAFName(CWnd* pParent /*=NULL*/)
	: CDialog(DialogAFName::IDD, pParent)
	, m_combo(NULL)
{
}

/*
================
DialogAFName::~DialogAFName
================
*/
DialogAFName::~DialogAFName() {
}

/*
================
DialogAFName::DoDataExchange
================
*/
void DialogAFName::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_AF_NAME, m_editName);
}

/*
================
DialogAFName::SetName
================
*/
void DialogAFName::SetName( CString &str ) {
	m_editName = str;
}

/*
================
DialogAFName::GetName
================
*/
void DialogAFName::GetName( CString &str ) {
	str = m_editName;
}

/*
================
DialogAFName::SetComboBox
================
*/
void DialogAFName::SetComboBox( CComboBox *combo ) {
	m_combo = combo;
}

/*
================
DialogAFName::OnInitDialog
================
*/
BOOL DialogAFName::OnInitDialog()  {
	CEdit *edit;
	CString str;

	CDialog::OnInitDialog();

	edit = (CEdit *)GetDlgItem( IDC_EDIT_AF_NAME );
	edit->SetFocus();
	edit->GetWindowText( str );
	edit->SetSel( 0, str.GetLength() );

	return FALSE;
}

/*
================
EditVerifyName
================
*/
void EditVerifyName( CEdit *edit ) {
	CString strIn, strOut;
	int start, end;
	static bool entered = false;

	if ( entered ) {
		return;
	}
	entered = true;

	edit->GetSel( start, end );
	edit->GetWindowText( strIn );
	for ( int i = 0; i < strIn.GetLength(); i++ ) {
		if ( ( strIn[i] >= 'a' && strIn[i] <= 'z' ) ||
				( strIn[i] >= 'A' && strIn[i] <= 'Z' ) ||
					( strIn[i] == '_' ) || ( strIn[i] >= '0' && strIn[i] <= '9' ) ) {
			strOut.AppendChar( strIn[i] );
		}
	}
	edit->SetWindowText( strOut );
	edit->SetSel( start, end );

	entered = false;
}


BEGIN_MESSAGE_MAP(DialogAFName, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT_AF_NAME, OnEnChangeEditAfName)
END_MESSAGE_MAP()


// DialogAFName message handlers

void DialogAFName::OnBnClickedOk() {

	UpdateData( TRUE );
	if ( m_combo && m_combo->FindStringExact( -1, m_editName ) != -1 ) {
		MessageBox( va( "The name %s is already used.", m_editName.GetBuffer() ), "Name", MB_OK );
	}
	else {
		OnOK();
	}
}

void DialogAFName::OnEnChangeEditAfName() {
	EditVerifyName( (CEdit *) GetDlgItem( IDC_EDIT_AF_NAME ) );
}

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

#include "../../sys/win32/rc/DeclEditor_resource.h"

#include "../comafx/CPathTreeCtrl.h"
#include "DialogDeclBrowser.h"
#include "DialogDeclNew.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif


toolTip_t DialogDeclNew::toolTips[] = {
	{ IDC_DECLNEW_COMBO_NEW_TYPE, "select the declaration type to create" },
	{ IDC_DECLNEW_EDIT_NEW_NAME, "enter a name for the new declaration" },
	{ IDC_DECLNEW_EDIT_NEW_FILE, "enter the name of the file to add the declaration to" },
	{ IDC_DECLNEW_BUTTON_NEW_FILE, "select existing file to add the declaration to" },
	{ IDOK, "create new declaration" },
	{ IDCANCEL, "cancel" },
	{ 0, NULL }
};


IMPLEMENT_DYNAMIC(DialogDeclNew, CDialog)

/*
================
DialogDeclNew::DialogDeclNew
================
*/
DialogDeclNew::DialogDeclNew( CWnd* pParent /*=NULL*/ )
	: CDialog(DialogDeclNew::IDD, pParent)
	, declTree(NULL)
	, newDecl(NULL)
{
}

/*
================
DialogDeclNew::~DialogDeclNew
================
*/
DialogDeclNew::~DialogDeclNew() {
}

/*
================
DialogDeclNew::DoDataExchange
================
*/
void DialogDeclNew::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogDeclNew)
	DDX_Control(pDX, IDC_DECLNEW_COMBO_NEW_TYPE, typeList);
	DDX_Control(pDX, IDC_DECLNEW_EDIT_NEW_NAME, nameEdit);
	DDX_Control(pDX, IDC_DECLNEW_EDIT_NEW_FILE, fileEdit);
	DDX_Control(pDX, IDC_DECLNEW_BUTTON_NEW_FILE, fileButton);
	DDX_Control(pDX, IDOK, okButton);
	DDX_Control(pDX, IDCANCEL, cancelButton);
	//}}AFX_DATA_MAP
}

/*
================
DialogDeclNew::InitTypeList
================
*/
void DialogDeclNew::InitTypeList( void ) {
	int i;

	typeList.ResetContent();
	for ( i = 0; i < declManager->GetNumDeclTypes(); i++ ) {
		typeList.AddString( declManager->GetDeclNameFromType( (declType_t)i ) );
	}
}

/*
================
DialogDeclNew::OnInitDialog
================
*/
BOOL DialogDeclNew::OnInitDialog()  {

	CDialog::OnInitDialog();

	InitTypeList();

	SetSafeComboBoxSelection( &typeList, defaultType.c_str(), -1 );
	nameEdit.SetWindowText( defaultName.c_str() );
	fileEdit.SetWindowText( defaultFile.c_str() );

	EnableToolTips( TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(DialogDeclNew, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_SETFOCUS()
	ON_BN_CLICKED(IDC_DECLNEW_BUTTON_NEW_FILE, OnBnClickedFile)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// DialogDeclNew message handlers

/*
================
DialogDeclNew::OnActivate
================
*/
void DialogDeclNew::OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized ) {
	CDialog::OnActivate( nState, pWndOther, bMinimized );
}

/*
================
DialogDeclNew::OnToolTipNotify
================
*/
BOOL DialogDeclNew::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

/*
================
DialogDeclNew::OnSetFocus
================
*/
void DialogDeclNew::OnSetFocus( CWnd *pOldWnd ) {
	CDialog::OnSetFocus( pOldWnd );
}

/*
================
DialogDeclNew::OnDestroy
================
*/
void DialogDeclNew::OnDestroy() {
	return CDialog::OnDestroy();
}

/*
================
DialogDeclNew::OnBnClickedFile
================
*/
void DialogDeclNew::OnBnClickedFile() {
	CString typeName, folder, ext;
	idStr path;
	const char *errorTitle = "Error selecting file.";

	if ( GetSafeComboBoxSelection( &typeList, typeName, -1 ) == -1 ) {
		MessageBox( "Select a declaration type first.", errorTitle, MB_OK );
		return;
	}

	declType_t type = declManager->GetDeclTypeFromName( typeName );
	if ( type >= declManager->GetNumDeclTypes() ) {
		MessageBox( "Unknown declaration type.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	switch( type ) {
		case DECL_TABLE:		folder = "materials";	ext = "(*.mtr)|*.mtr|(*.*)|*.*||";					break;
		case DECL_MATERIAL:		folder = "materials";	ext = "(*.mtr)|*.mtr|(*.*)|*.*||";					break;
		case DECL_SKIN:			folder = "skins";		ext = "(*.skin)|*.skin|(*.*)|*.*||";				break;
		case DECL_SOUND:		folder = "sound";		ext = "(*.sndshd|*.sndshd|(*.*)|*.*||";				break;
		case DECL_ENTITYDEF:	folder = "def";			ext = "(*.def)|*.def|(*.decl)|*.decl|(*.*)|*.*||";	break;
		case DECL_MODELDEF:		folder = "def";			ext = "(*.def)|*.def|(*.*)|*.*||";					break;
		case DECL_FX:			folder = "fx";			ext = "(*.fx)|*.fx|(*.*)|*.*||";					break;
		case DECL_PARTICLE:		folder = "particles";	ext = "(*.prt)|*.prt|(*.*)|*.*||";					break;
		case DECL_AF:			folder = "af";			ext = "(*.af)|*.af|(*.*)|*.*||";					break;
		default:				folder = "def";			ext = "(*.decl)|*.decl|(*.*)|*.*||";				break;
	}

	path = fileSystem->RelativePathToOSPath( folder );
	path += "\\*";

	CFileDialog dlgFile( TRUE, "decl", path, 0, ext, this );
	if ( dlgFile.DoModal() == IDOK ) {
		path = fileSystem->OSPathToRelativePath( dlgFile.m_ofn.lpstrFile );
		fileEdit.SetWindowText( path );
	}
}

/*
================
DialogDeclNew::OnBnClickedOk
================
*/
void DialogDeclNew::OnBnClickedOk() {
	CString typeName, declName, fileName;
	const char *errorTitle = "Error creating declaration.";

	if ( !declTree ) {
		MessageBox( "No declaration tree available.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	if ( GetSafeComboBoxSelection( &typeList, typeName, -1 ) == -1 ) {
		MessageBox( "No declaration type selected.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	nameEdit.GetWindowText( declName );
	if ( declName.GetLength() == 0 ) {
		MessageBox( "No declaration name specified.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	fileEdit.GetWindowText( fileName );
	if ( fileName.GetLength() == 0 ) {
		MessageBox( "No file name specified.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	if ( declTree->FindItem( idStr( typeName + "/" + declName ) ) ) {
		MessageBox( "Declaration already exists.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	declType_t type = declManager->GetDeclTypeFromName( typeName );
	if ( type >= declManager->GetNumDeclTypes() ) {
		MessageBox( "Unknown declaration type.", errorTitle, MB_OK | MB_ICONERROR );
		return;
	}

	newDecl = declManager->CreateNewDecl( type, declName, fileName );

	OnOK();
}

/*
================
DialogDeclNew::OnBnClickedCancel
================
*/
void DialogDeclNew::OnBnClickedCancel() {
	OnCancel();
}

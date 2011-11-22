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
#include "DialogAFView.h"
#include "DialogAFProperties.h"
#include "DialogAFBody.h"
#include "DialogAFConstraint.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

// DialogAF

#define AFTAB_VIEW				0x01
#define AFTAB_PROPERTIES		0x02
#define AFTAB_BODIES			0x03
#define AFTAB_CONSTRAINTS		0x04

toolTip_t DialogAF::toolTips[] = {
	{ IDC_COMBO_AF, "select an articulated figure for editing" },
	{ IDC_BUTTON_AF_NEW, "create a new articulated figure" },
	{ IDC_BUTTON_AF_DELETE, "delete the selected articulated figure" },
	{ IDC_BUTTON_AF_SPAWN, "spawn ingame entity using the selected articulated figure" },
	{ IDC_BUTTON_AF_TPOSE, "set ingame entity using the selected articulated figure back into T-Pose" },
	{ IDC_BUTTON_AF_KILL, "kill ingame entity using the selected articulated figure" },
	{ IDC_BUTTON_AF_SAVE, "save the selected articulated figure" },
	{ IDCANCEL, "cancel all changes to all articulated figures" },
	{ 0, NULL }
};


DialogAF *g_AFDialog = NULL;


IMPLEMENT_DYNAMIC(DialogAF, CDialog)

/*
================
DialogAF::DialogAF
================
*/
DialogAF::DialogAF( CWnd* pParent /*=NULL*/ )
	: CDialog(DialogAF::IDD, pParent)
	, file(NULL)
{
	wndTabs = NULL;
	wndTabDisplay = NULL;
}

/*
================
DialogAF::~DialogAF
================
*/
DialogAF::~DialogAF() {
}

/*
================
DialogAF::DoDataExchange
================
*/
void DialogAF::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAF)
	DDX_Control(pDX, IDC_COMBO_AF, AFList);
	//}}AFX_DATA_MAP
}

/*
================
DialogAF::LoadFile
================
*/
void DialogAF::LoadFile( idDeclAF *af ) {
	file = af;
	propertiesDlg->LoadFile( af );
	bodyDlg->LoadFile( af );
	constraintDlg->LoadFile( af );

	if ( file ) {
		// select file in AFList
		int i = AFList.FindString( -1, file->GetName() );
		if ( i != AFList.GetCurSel() ) {
			AFList.SetCurSel( i );
		}
		GetDlgItem( IDC_BUTTON_AF_SAVE )->EnableWindow( file->modified );
		GetDlgItem( IDC_BUTTON_AF_DELETE )->EnableWindow( true );
	}
	else {
		AFList.SetCurSel( -1 );
		GetDlgItem( IDC_BUTTON_AF_SAVE )->EnableWindow( false );
		GetDlgItem( IDC_BUTTON_AF_DELETE )->EnableWindow( false );
	}
}

/*
================
DialogAF::LoadFile
================
*/
void DialogAF::SaveFile( void ) {
	if ( !file ) {
		return;
	}
	propertiesDlg->SaveFile();
	bodyDlg->SaveFile();
	constraintDlg->SaveFile();
	gameEdit->AF_UpdateEntities( file->GetName() );
}

/*
================
DialogAF::SetFileModified
================
*/
void DialogAF::SetFileModified( void ) {
	if ( file ) {
		file->modified = true;
		GetDlgItem( IDC_BUTTON_AF_SAVE )->EnableWindow( true );
	}
}

/*
================
DialogAF::ReloadFile
================
*/
void DialogAF::ReloadFile( void ) {
	LoadFile( file );
}

/*
================
DialogAF::InitAFList
================
*/
void DialogAF::InitAFList( void ) {
	int i, c;

	AFList.ResetContent();
	c = declManager->GetNumDecls( DECL_AF );
	for ( i = 0; i < c; i++ ) {
		AFList.AddString( static_cast<const idDeclAF *>( declManager->DeclByIndex( DECL_AF, i, false ) )->GetName() );
	}
}

/*
================
DialogAF::AddTabItem
================
*/
void DialogAF::AddTabItem( int id, const char *name ) {
	TCITEM item;
	item.mask = TCIF_PARAM;
	item.lParam = id;
	int tab = wndTabs->InsertItem( wndTabs->GetItemCount(), name );
	wndTabs->SetItem( tab, &item );
}

/*
================
DialogAF::SetTab
================
*/
void DialogAF::SetTab( int id ) {
	int c = wndTabs->GetItemCount();
	for ( int i = 0; i < c; i++ ) {
		TCITEM item;
		item.mask = TCIF_PARAM;
		wndTabs->GetItem( i, &item );
		if ( item.lParam == id ) {
			wndTabs->SetCurSel(i);
			return;
		}
	}
	wndTabs->SetCurSel(0);
}

/*
================
DialogAF::SetTabChildPos

  position the child dialog box
================
*/
void DialogAF::SetTabChildPos( void ) {
	if ( wndTabDisplay ) {
		wndTabDisplay->ShowWindow( SW_SHOW );
		wndTabDisplay->SetWindowPos( wndTabs, 12, 60, 0, 0, SWP_NOSIZE );
	}
}

/*
================
DialogAF::OnInitDialog
================
*/
BOOL DialogAF::OnInitDialog()  {
	CDialog::OnInitDialog();

	com_editors |= EDITOR_AF;
	
	// initialize list with articulated figure files
	InitAFList();

	// initialize tabs
	wndTabs = (CTabCtrl *) GetDlgItem( IDC_DIALOG_AF_TAB_MODE );
	AddTabItem( AFTAB_VIEW, "View" );
	AddTabItem( AFTAB_PROPERTIES, "Properties" );
	AddTabItem( AFTAB_BODIES, "Bodies" );
	AddTabItem( AFTAB_CONSTRAINTS, "Constraints" );
	SetTab( AFTAB_VIEW );

	// create child dialog windows
	viewDlg = new DialogAFView( this );
	propertiesDlg = new DialogAFProperties( this );
	bodyDlg = new DialogAFBody( this );
	constraintDlg = new DialogAFConstraint( this );

	// the body dialog may force the constraint dialog to reload the file
	bodyDlg->constraintDlg = constraintDlg;

	// the properties dialog may force the body or constraint dialog to reload the file
	propertiesDlg->bodyDlg = bodyDlg;
	propertiesDlg->constraintDlg = constraintDlg;

	// set active child dialog
	wndTabDisplay = viewDlg;
	SetTabChildPos();

	EnableToolTips( TRUE );

	GetDlgItem( IDC_BUTTON_AF_DELETE )->EnableWindow( false );
	GetDlgItem( IDC_BUTTON_AF_SAVE )->EnableWindow( false );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(DialogAF, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY(TCN_SELCHANGE, IDC_DIALOG_AF_TAB_MODE, OnTcnSelchangeTabMode)
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_MOVE()
	ON_WM_SETFOCUS()
	ON_CBN_SELCHANGE(IDC_COMBO_AF, OnCbnSelchangeComboAf)
	ON_BN_CLICKED(IDC_BUTTON_AF_NEW, OnBnClickedButtonAfNew)
	ON_BN_CLICKED(IDC_BUTTON_AF_DELETE, OnBnClickedButtonAfDelete)
	ON_BN_CLICKED(IDC_BUTTON_AF_SAVE, OnBnClickedButtonAfSave)
	ON_BN_CLICKED(IDC_BUTTON_AF_SPAWN, OnBnClickedButtonAfSpawn)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_AF_KILL, OnBnClickedButtonAfKill)
	ON_BN_CLICKED(IDC_BUTTON_AF_TPOSE, OnBnClickedButtonAfTpose)
END_MESSAGE_MAP()


/*
================
AFEditorInit
================
*/
void AFEditorInit( const idDict *spawnArgs ) {

	if ( renderSystem->IsFullScreen() ) {
		common->Printf( "Cannot run the articulated figure editor in fullscreen mode.\n"
					"Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

	if ( g_AFDialog == NULL ) {
		InitAfx();
		g_AFDialog = new DialogAF();
	}

	if ( g_AFDialog->GetSafeHwnd() == NULL) {
		g_AFDialog->Create( IDD_DIALOG_AF );
/*
		// FIXME: restore position
		CRect rct;
		g_AFDialog->SetWindowPos( NULL, rct.left, rct.top, 0, 0, SWP_NOSIZE );
*/
	}

	idKeyInput::ClearStates();

	g_AFDialog->ShowWindow( SW_SHOW );
	g_AFDialog->SetFocus();

	if ( spawnArgs ) {
		// select AF based on spawn args
		const char *name = spawnArgs->GetString( "articulatedFigure" );
		if ( name[0] == '\0' ) {
			name = spawnArgs->GetString( "ragdoll" );
		}
		idDeclAF *decl = static_cast<idDeclAF *>( const_cast<idDecl *>( declManager->FindType( DECL_AF, name ) ) );
		if ( decl ) {
			g_AFDialog->LoadFile( decl );
		}
	}
}

/*
================
AFEditorRun
================
*/
void AFEditorRun( void ) {
#if _MSC_VER >= 1300
	MSG *msg = AfxGetCurrentMessage();			// TODO Robert fix me!!
#else
	MSG *msg = &m_msgCur;
#endif

	while( ::PeekMessage(msg, NULL, NULL, NULL, PM_NOREMOVE) ) {
		// pump message
		if ( !AfxGetApp()->PumpMessage() ) {
		}
	}
}

/*
================
AFEditorShutdown
================
*/
void AFEditorShutdown( void ) {
	delete g_AFDialog;
	g_AFDialog = NULL;
}


// DialogAF message handlers

/*
================
DialogAF::OnActivate
================
*/
void DialogAF::OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized ) {
	CDialog::OnActivate( nState, pWndOther, bMinimized );
}

/*
================
DialogAF::OnToolTipNotify
================
*/
BOOL DialogAF::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

/*
================
DialogAF::OnSetFocus
================
*/
void DialogAF::OnSetFocus( CWnd *pOldWnd ) {
	//SetActiveWindow();
	CDialog::OnSetFocus( pOldWnd );
}

/*
================
DialogAF::OnDestroy
================
*/
void DialogAF::OnDestroy() {

	com_editors &= ~EDITOR_AF;

	return CDialog::OnDestroy();
}

/*
================
DialogAF::OnMove
================
*/
void DialogAF::OnMove( int x, int y ) {
	if ( GetSafeHwnd() ) {
		CRect rct;
		GetWindowRect( rct );
		// FIXME: save position
	}
	CDialog::OnMove( x, y );
}

/*
================
DialogAF::OnTcnSelchangeTabMode

  tab control notification handler
================
*/
void DialogAF::OnTcnSelchangeTabMode( NMHDR *pNMHDR, LRESULT *pResult ) {
	*pResult = 0;

	// hide the current tab child dialog box, if any.
	if ( wndTabDisplay != NULL ) {
		wndTabDisplay->ShowWindow( SW_HIDE );
	}

	TCITEM item;
	item.mask = TCIF_PARAM;
	wndTabs->GetItem( wndTabs->GetCurSel(), &item );

	// show the new tab child dialog box.
	switch ( item.lParam ) {
		case AFTAB_VIEW:
			wndTabDisplay = viewDlg;
			break;
		case AFTAB_PROPERTIES:
			wndTabDisplay = propertiesDlg;
			break;
		case AFTAB_BODIES:
			wndTabDisplay = bodyDlg;
			break;
		case AFTAB_CONSTRAINTS:
			wndTabDisplay = constraintDlg;
			break;
	}

	SetTabChildPos();
}

/*
================
DialogAF::OnCbnSelchangeComboAf
================
*/
void DialogAF::OnCbnSelchangeComboAf() {
	int index = AFList.GetCurSel();
	if ( index < 0 || index >= declManager->GetNumDecls( DECL_AF ) ) {
		InitAFList();
		return;
	}
	if ( index != CB_ERR ) {
		CString str;
		AFList.GetLBText( index, str );
		LoadFile( static_cast<idDeclAF *>( const_cast<idDecl *>( declManager->FindType( DECL_AF, str ) ) ) );
	}
}

/*
================
DialogAF::OnBnClickedButtonAfNew
================
*/
void DialogAF::OnBnClickedButtonAfNew() {
	DialogAFName nameDlg;
	CString name;
	idStr fileName;

	nameDlg.SetComboBox( &AFList );
	if ( nameDlg.DoModal() != IDOK ) {
		return;
	}
	nameDlg.GetName( name );

	CFileDialog dlgSave( FALSE, "map", NULL, OFN_OVERWRITEPROMPT, "AF Files (*.af)|*.af|All Files (*.*)|*.*||", AfxGetMainWnd() );
	if ( dlgSave.DoModal() != IDOK ) {
		return;
	}
	fileName = fileSystem->OSPathToRelativePath( dlgSave.m_ofn.lpstrFile );

	// create a new .af file
	AFList.AddString( name );
	AFList.SetCurSel( AFList.FindString( -1, name )  );
	idDeclAF *decl = static_cast<idDeclAF *>( declManager->CreateNewDecl( DECL_AF, name, fileName ) );
	LoadFile( decl );
	AFDialogSetFileModified();
}

/*
================
DialogAF::OnBnClickedButtonAfDelete
================
*/
void DialogAF::OnBnClickedButtonAfDelete() {
	int i;

	i = AFList.GetCurSel();
	if ( i != CB_ERR ) {
		if ( MessageBox( "Are you sure you want to delete the articulated figure file ?", "Delete Articulated Figure", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			// FIXME: delete the currently selected .af file
		}
	}
}

/*
================
DialogAF::OnBnClickedButtonAfSpawn
================
*/
void DialogAF::OnBnClickedButtonAfSpawn() {
	int index = AFList.GetCurSel();
	if ( index != CB_ERR ) {
		CString str;
		AFList.GetLBText( index, str );
		gameEdit->AF_SpawnEntity( str );
	}
}

/*
================
DialogAF::OnBnClickedButtonAfTpose
================
*/
void DialogAF::OnBnClickedButtonAfTpose() {
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAF::OnBnClickedButtonAfKill
================
*/
void DialogAF::OnBnClickedButtonAfKill() {
	cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "deleteSelected\n" );
}

/*
================
DialogAF::OnBnClickedButtonAfSave
================
*/
void DialogAF::OnBnClickedButtonAfSave() {
	// save the selected .af file
	if ( file ) {
		if ( file->Save() ) {
			GetDlgItem( IDC_BUTTON_AF_SAVE )->EnableWindow( false );
		}
		else {
			MessageBox( "Saving the file failed. Make sure the file is not read-only.", "Delete Articulated Figure", MB_OK );
		}
	}
}

/*
================
DialogAF::OnBnClickedCancel
================
*/
void DialogAF::OnBnClickedCancel() {
	int i, c;

	// check if there are modified .af files and come up with a warning if so
	c = declManager->GetNumDecls( DECL_AF );
	for ( i = 0; i < c; i++ ) {
		if ( static_cast<const idDeclAF *>( declManager->DeclByIndex( DECL_AF, i ) )->modified ) {
			if ( MessageBox( "Some articulated figures have been modified.\nCancel all changes ?", "Cancel", MB_YESNO | MB_ICONQUESTION ) != IDYES ) {
				return;
			}
			break;
		}
	}
	// reload all modified .af files
	LoadFile( NULL );
	gameEdit->AF_UndoChanges();
	InitAFList();
	OnCancel();
}


// General convenience routines

/*
================
AFDialogSetFileModified
================
*/
void AFDialogSetFileModified( void ) {
	if ( g_AFDialog ) {
		g_AFDialog->SetFileModified();
	}
}

/*
================
AFDialogReloadFile
================
*/
void AFDialogReloadFile( void ) {
	if ( g_AFDialog ) {
		g_AFDialog->ReloadFile();
	}
}

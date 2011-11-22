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

#include "../../game/game.h"
#include "../../sys/win32/win_local.h"
#include "../../sys/win32/rc/common_resource.h"
#include "../../sys/win32/rc/PDAEditor_resource.h"
#include "../comafx/DialogName.h"

#include "DialogPDAEditor.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

/////////////////////////////////////////////////////////////////////////////
// CCDialogPDAEditor dialog
CDialogPDAEditor *g_PDAEditorDialog = NULL;


CDialogPDAEditor::CDialogPDAEditor(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogPDAEditor::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogPDAEditor)
	//}}AFX_DATA_INIT
}


void CDialogPDAEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogPDAEditor)
	DDX_Control( pDX, IDC_LIST_PDAS, pdaList );
	DDX_Control( pDX, IDC_LIST_EMAIL, emailList );
	DDX_Control( pDX, IDC_LIST_AUDIO, audioList );
	DDX_Control( pDX, IDC_LIST_VIDEO, videoList );

	DDX_Text( pDX, IDC_EDIT_FULLNAME, fullName );
	DDX_Text( pDX, IDC_EDIT_SHORTNAME, shortName );
	DDX_Text( pDX, IDC_EDIT_POST, post );
	DDX_Text( pDX, IDC_EDIT_TITLE, title );
	DDX_Text( pDX, IDC_EDIT_SECURITY, security );
	DDX_Text( pDX, IDC_EDIT_IDNUM, idnum );

	DDX_Control( pDX, IDC_BUTTON_SAVE, saveButton );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogPDAEditor, CDialog)
	//{{AFX_MSG_MAP(CDialogPDAEditor)
	ON_LBN_SELCHANGE( IDC_LIST_PDAS, OnSelChangePDA )
	ON_BN_CLICKED( IDC_BUTTON_SAVE, OnBtnClickedSave )
	ON_BN_CLICKED( IDC_BUTTON_RANDOMID, OnBtnClickedRandom )
	ON_BN_CLICKED( IDC_BUTTON_PDA_ADD, OnBtnClickedPDAAdd )
	ON_BN_CLICKED( IDC_BUTTON_PDA_DEL, OnBtnClickedPDADel )
	ON_BN_CLICKED( IDC_BUTTON_EMAIL_ADD, OnBtnClickedEmailAdd )
	ON_BN_CLICKED( IDC_BUTTON_EMAIL_EDIT, OnBtnClickedEmailEdit )
	ON_BN_CLICKED( IDC_BUTTON_EMAIL_DELETE, OnBtnClickedEmailDel )
	ON_BN_CLICKED( IDC_BUTTON_AUDIO_ADD, OnBtnClickedAudioAdd )
	ON_BN_CLICKED( IDC_BUTTON_AUDIO_EDIT, OnBtnClickedAudioEdit )
	ON_BN_CLICKED( IDC_BUTTON_AUDIO_DELETE, OnBtnClickedAudioDel )
	ON_BN_CLICKED( IDC_BUTTON_VIDEO_ADD, OnBtnClickedVideoAdd )
	ON_BN_CLICKED( IDC_BUTTON_VIDEO_EDIT, OnBtnClickedVideoEdit )
	ON_BN_CLICKED( IDC_BUTTON_VIDEO_DELETE, OnBtnClickedVideoDel )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogPDAEditor message handlers

void PDAEditorInit( const idDict *spawnArgs ) {

	if ( renderSystem->IsFullScreen() ) {
		common->Printf( "Cannot run the PDA editor in fullscreen mode.\n"
					"Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

	if ( g_PDAEditorDialog == NULL ) {
		InitAfx();
		g_PDAEditorDialog = new CDialogPDAEditor();
	}

	if ( g_PDAEditorDialog->GetSafeHwnd() == NULL ) {
		g_PDAEditorDialog->Create(IDD_DIALOG_PDA_EDITOR);
/*
		// FIXME: restore position
		CRect rct;
		g_PDAEditorDialog->SetWindowPos( NULL, rct.left, rct.top, 0,0, SWP_NOSIZE );
*/
	}

	idKeyInput::ClearStates();

	g_PDAEditorDialog->ShowWindow( SW_SHOW );
	g_PDAEditorDialog->SetFocus();

	if ( spawnArgs ) {
		// select PDA based on spawn args
		const char *name = spawnArgs->GetString( "pda" );
		idDeclPDA *decl = static_cast<idDeclPDA *>( const_cast<idDecl *>( declManager->FindType( DECL_PDA, name ) ) );
		// FIXME: select this PDA
	}
}

void PDAEditorRun( void ) {
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

void PDAEditorShutdown( void ) {
	delete g_PDAEditorDialog;
	g_PDAEditorDialog = NULL;
}

void CDialogPDAEditor::OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized ) {
	CDialog::OnActivate( nState, pWndOther, bMinimized );
	if ( nState != WA_INACTIVE ) {
	}
}

void CDialogPDAEditor::OnMove( int x, int y ) {
	if ( GetSafeHwnd() ) {
		CRect rct;
		GetWindowRect( rct );
		// FIXME: save position
	}
	CDialog::OnMove( x, y );
}

void CDialogPDAEditor::OnDestroy() {

	com_editors &= ~EDITOR_PDA;

	return CDialog::OnDestroy();
}

BOOL CDialogPDAEditor::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Indicate the PDA dialog is opened
	com_editors |= EDITOR_PDA;

	PopulatePDAList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDialogPDAEditor::PreTranslateMessage(MSG* pMsg)
{
	return CDialog::PreTranslateMessage(pMsg);
}

void CDialogPDAEditor::PopulatePDAList()
{
	pdaList.ResetContent();

	int i;
	int num = declManager->GetNumDecls(DECL_PDA);
	for ( i=0; i < num; i++ ) {
		const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, i) );
		pdaList.AddString( pda->GetName() );
	}
}

void CDialogPDAEditor::OnSelChangePDA()
{
	int i, num;

	int index = pdaList.GetCurSel();
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );
	if ( !pda ) {
		return;
	}

	CString windowTitle;
	windowTitle.Format("PDA Editor - %s", pda->GetName());

	idFile *file = fileSystem->OpenFileAppend( pda->GetFileName() );
	if ( file ) {
		fileSystem->CloseFile(file);
		saveButton.EnableWindow( true );
	} else {
		windowTitle += " [Read Only]";
		saveButton.EnableWindow( false );
	}

	SetWindowText( windowTitle );

	emailList.ResetContent();
	num = pda->GetNumEmails();
	for ( i=0; i < num; i++ ) {
		emailList.AddString( pda->GetEmailByIndex( i )->GetSubject() );
	}

	audioList.ResetContent();
	num = pda->GetNumAudios();
	for ( i=0; i < num; i++ ) {
		audioList.AddString( pda->GetAudioByIndex( i )->GetAudioName() );
	}

	videoList.ResetContent();
	num = pda->GetNumVideos();
	for ( i=0; i < num; i++ ) {
		videoList.AddString( pda->GetVideoByIndex( i )->GetVideoName() );
	}

	fullName = pda->GetFullName();
	shortName = pda->GetPdaName();
	post = pda->GetPost();
	title = pda->GetTitle();
	security = pda->GetSecurity();
	idnum = pda->GetID();

	UpdateData( FALSE );
}

void CDialogPDAEditor::OnBtnClickedSave()
{
	UpdateData();

	int index = pdaList.GetCurSel();
	if ( index < 0 ) {
		return;
	}

	const idDeclPDA *pdaConst = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );
	if ( pdaConst ) {
		idDeclPDA *pda = const_cast<idDeclPDA *>(pdaConst);

		CString declText = "\n";
		declText += "pda ";
		declText += pda->GetName();
		declText += " {\n";
	
		declText += "\tname    \t\t\"" + shortName + "\"\n";
		declText += "\tfullname\t\t\"" + fullName + "\"\n";
		declText += "\ticon    \t\t\"\"\n";
		declText += "\tid      \t\t\"" + idnum + "\"\n";
		declText += "\tpost    \t\t\"" + post + "\"\n";
		declText += "\ttitle   \t\t\"" + title + "\"\n";
		declText += "\tsecurity\t\t\"" + security + "\"\n";

		for ( int i = 0; i < pda->GetNumEmails(); i++ ) {
			declText += "\tpda_email\t\t\"";
			declText += pda->GetEmailByIndex(i)->GetName();
			declText += "\"\n";
		}

		for ( int i = 0; i < pda->GetNumAudios(); i++ ) {
			declText += "\tpda_audio\t\t\"";
			declText += pda->GetAudioByIndex(i)->GetName();
			declText += "\"\n";
		}

		for ( int i = 0; i < pda->GetNumVideos(); i++ ) {
			declText += "\tpda_video\t\t\"";
			declText += pda->GetVideoByIndex(i)->GetName();
			declText += "\"\n";
		}

		declText += "}";

		pda->SetText( declText );
		pda->ReplaceSourceFileText();
		pda->Invalidate();
	}
}

void CDialogPDAEditor::OnBtnClickedRandom()
{
	idnum.Format("%d-%02X", 1000+(rand()%8999), (rand()%255));
	UpdateData( FALSE );
}

class CDialogPDAAdd : public CDialog
{
public:
	CDialogPDAAdd() : CDialog(IDD_DIALOG_PDA_ADD) {}
	CString name;
	void OnOK() { GetDlgItemText( IDC_EDIT1, name ); CDialog::OnOK(); }
};

void CDialogPDAEditor::OnBtnClickedPDAAdd()
{
	CDialogPDAAdd dlg;
	if ( dlg.DoModal() == IDOK ) {
		dlg.name.MakeLower();
		idDecl *decl = declManager->CreateNewDecl( DECL_PDA, dlg.name, "newpdas/" + dlg.name + ".pda" );
		decl->ReplaceSourceFileText();
		decl->Invalidate();
		PopulatePDAList();
		pdaList.SelectString( 0, dlg.name );
		OnSelChangePDA();
	}
}

void CDialogPDAEditor::OnBtnClickedPDADel()
{
}

void CDialogPDAEditor::OnBtnClickedEmailAdd()
{
	int index = pdaList.GetCurSel();
	if ( index < 0 ) {
		return;
	}
	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );

	if ( pda ) {
		CString name;

		// Search for an unused name
		int newIndex = pda->GetNumEmails();
		do {
			name.Format("%s_email_%d", pda->GetName(), newIndex++);
		} while ( declManager->FindType(DECL_EMAIL, name, false) != NULL );

		CDialogPDAEditEmail addDlg;
		addDlg.SetName(name);
		if ( addDlg.DoModal() == IDOK ) {
			idDeclEmail *email = static_cast<idDeclEmail *>(declManager->CreateNewDecl(DECL_EMAIL, name, pda->GetFileName()));
			email->SetText( addDlg.GetDeclText() );
			email->ReplaceSourceFileText();
			email->Invalidate();

			pda->AddEmail( name );

			// Get it again to reparse
			const idDeclEmail *emailConst = static_cast<const idDeclEmail *>( declManager->FindType( DECL_EMAIL, name) );
			emailList.AddString( emailConst->GetSubject() );

			// Save the pda to include this email in the list
			// This has a side-effect of saving any other changes, but I don't really care right now
			OnBtnClickedSave();
		}
	}
}

void CDialogPDAEditor::OnBtnClickedEmailEdit()
{
	int index = pdaList.GetCurSel();
	if ( index < 0 ) {
		return;
	}
	const idDeclPDA *pda = dynamic_cast<const idDeclPDA *>( declManager->DeclByIndex(DECL_PDA, index) );

	if ( pda ) {
		index = emailList.GetCurSel();
		if ( index < 0 ) {
			return;
		}

		CDialogPDAEditEmail editDlg;
		editDlg.SetEmail( pda->GetEmailByIndex( index ) );
		if ( editDlg.DoModal() == IDOK ) {
			idDeclEmail *email = const_cast<idDeclEmail *>( pda->GetEmailByIndex( index ) );
			email->SetText( editDlg.GetDeclText() );
			email->ReplaceSourceFileText();
			email->Invalidate();

			// Get it again to reparse
			email = const_cast<idDeclEmail *>( pda->GetEmailByIndex( index ) );

			emailList.DeleteString( index );
			emailList.InsertString( index, email->GetSubject() );
		}
	}
}

void CDialogPDAEditor::OnBtnClickedEmailDel()
{
}

void CDialogPDAEditor::OnBtnClickedAudioAdd()
{
}

void CDialogPDAEditor::OnBtnClickedAudioEdit()
{
}

void CDialogPDAEditor::OnBtnClickedAudioDel()
{
}

void CDialogPDAEditor::OnBtnClickedVideoAdd()
{
}

void CDialogPDAEditor::OnBtnClickedVideoEdit()
{
}

void CDialogPDAEditor::OnBtnClickedVideoDel()
{
}




CDialogPDAEditEmail::CDialogPDAEditEmail(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogPDAEditEmail::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogPDAEditEmail)
	//}}AFX_DATA_INIT
}


void CDialogPDAEditEmail::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogPDAEditEmail)
	DDX_Text( pDX, IDC_EDIT_TO, to );
	DDX_Text( pDX, IDC_EDIT_FROM, from );
	DDX_Text( pDX, IDC_EDIT_DATE, date );
	DDX_Text( pDX, IDC_EDIT_SUBJECT, subject );
	DDX_Text( pDX, IDC_EDIT_BODY, body );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogPDAEditEmail, CDialog)
	//{{AFX_MSG_MAP(CDialogPDAEditEmail)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogPDAEditor message handlers

BOOL CDialogPDAEditEmail::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetWindowText( "Editing Email: " + name );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogPDAEditEmail::SetName( CString &_name )
{
	name = _name;
}

void CDialogPDAEditEmail::SetEmail( const idDeclEmail *email )
{
	to = email->GetTo();
	from = email->GetFrom();
	date = email->GetDate();
	subject = email->GetSubject();
	body = email->GetBody();
	body.Replace("\n", "\r\n");

	name = email->GetName();

	if ( IsWindow( m_hWnd ) ) {
		UpdateData(FALSE);
	}
}

CString CDialogPDAEditEmail::GetDeclText()
{
	CString mungedBody = body;
	mungedBody.Replace("\r\n\r\n", "\\n\\n\"\n\n\"");
	mungedBody.Replace("\r\n", "\\n\"\n\"");

	CString declText;
	declText += "\n";
	declText += "email " + name + " {\n";
	declText += "\tto     \t\t\"" + to + "\"\n";
	declText += "\tfrom   \t\t\"" + from + "\"\n";
	declText += "\tdate   \t\t\"" + date + "\"\n";
	declText += "\tsubject\t\t\"" + subject + "\"\n";
	declText += "\ttext {\n";
	declText += "\"" + mungedBody + "\"\n";
	declText += "\t}\n";
	declText += "}";

	return declText;
}


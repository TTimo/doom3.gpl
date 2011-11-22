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
#include "../../sys/win32/rc/ScriptEditor_resource.h"

#include "../comafx/DialogGoToLine.h"
#include "DialogScriptEditor.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif


typedef struct scriptEventInfo_s {
	idStr		name;
	idStr		parms;
	idStr		help;
} scriptEventInfo_t;

static idList<scriptEventInfo_t> scriptEvents;

static DialogScriptEditor *g_ScriptDialog = NULL;

// DialogScriptEditor dialog

static UINT FindDialogMessage = ::RegisterWindowMessage( FINDMSGSTRING );

toolTip_t DialogScriptEditor::toolTips[] = {
	{ IDOK, "save" },
	{ IDCANCEL, "cancel" },
	{ 0, NULL }
};


IMPLEMENT_DYNAMIC(DialogScriptEditor, CDialog)

/*
================
DialogScriptEditor::DialogScriptEditor
================
*/
DialogScriptEditor::DialogScriptEditor( CWnd* pParent /*=NULL*/ )
	: CDialog(DialogScriptEditor::IDD, pParent)
	, findDlg(NULL)
	, matchCase(false)
	, matchWholeWords(false)
	, firstLine(0)
{
}

/*
================
DialogScriptEditor::~DialogScriptEditor
================
*/
DialogScriptEditor::~DialogScriptEditor() {
}

/*
================
DialogScriptEditor::DoDataExchange
================
*/
void DialogScriptEditor::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogScriptEditor)
	DDX_Control(pDX, IDC_SCRIPTEDITOR_EDIT_TEXT, scriptEdit);
	DDX_Control(pDX, IDOK, okButton);
	DDX_Control(pDX, IDCANCEL, cancelButton);
	//}}AFX_DATA_MAP
}

/*
================
DialogScriptEditor::PreTranslateMessage
================
*/
BOOL DialogScriptEditor::PreTranslateMessage( MSG* pMsg ) {
	if ( WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST ) {
		if ( m_hAccel && ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) ) {
			return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

/*
================
DialogScriptEditor::UpdateStatusBar
================
*/
void DialogScriptEditor::UpdateStatusBar( void ) {
	int line, column, character;

	scriptEdit.GetCursorPos( line, column, character );
	statusBar.SetWindowText( va( "Line: %d, Column: %d, Character: %d", line, column, character ) );
}

/*
================
DialogScriptEditor::InitScriptEvents
================
*/
void DialogScriptEditor::InitScriptEvents( void ) {
	int index;
	idParser src;
	idToken token;
	idStr whiteSpace;
	scriptEventInfo_t info;

	if ( !src.LoadFile( "script/doom_events.script" ) ) {
		return;
	}

	scriptEvents.Clear();

	while( src.ReadToken( &token ) ) {
		if ( token == "scriptEvent" ) {

			src.GetLastWhiteSpace( whiteSpace );
			index = whiteSpace.Find( "//" );
			if ( index != -1 ) {
				info.help = whiteSpace.Right( whiteSpace.Length() - index );
				info.help.Replace( "\r", "" );
				info.help.Replace( "\n", "\r\n" );
			} else {
				info.help = "";
			}

			src.ExpectTokenType( TT_NAME, 0, &token );

			info.parms = token;

			src.ExpectTokenType( TT_NAME, 0, &token );

			info.name = token;

			src.ExpectTokenString( "(" );

			info.parms += " " + info.name + "(";
			while( src.ReadToken( &token ) && token != ";" ) {
				info.parms.Append( " " + token );
			}

			scriptEvents.Append( info );
		}
	}
}

/*
================
GetScriptEvents
================
*/
bool GetScriptEvents( const char *objectName, CListBox &listBox ) {
	for ( int i = 0; i < scriptEvents.Num(); i++ ) {
		listBox.AddString( scriptEvents[i].name );
	}
	return true;
}

/*
================
GetFunctionParms
================
*/
bool GetFunctionParms( const char *funcName, CString &parmString ) {
	for ( int i = 0; i < scriptEvents.Num(); i++ ) {
		if ( scriptEvents[i].name.Cmp( funcName ) == 0 ) {
			parmString = scriptEvents[i].parms;
			return true;
		}
	}
	return false;
}

/*
================
GetToolTip
================
*/
bool GetToolTip( const char *name, CString &string ) {
	for ( int i = 0; i < scriptEvents.Num(); i++ ) {
		if ( scriptEvents[i].name.Cmp( name ) == 0 ) {
			string = scriptEvents[i].help + scriptEvents[i].parms;
			return true;
		}
	}
	return false;
}

/*
================
DialogScriptEditor::OpenFile
================
*/
void DialogScriptEditor::OpenFile( const char *fileName ) {
	int numLines = 0;
	int numCharsPerLine = 0;
	int maxCharsPerLine = 0;
	idStr scriptText, extension;
	CRect rect;
	void *buffer;

	scriptEdit.Init();
	scriptEdit.AllowPathNames( false );

	idStr( fileName ).ExtractFileExtension( extension );

	if ( extension.Icmp( "script" ) == 0 ) {
        InitScriptEvents();
		scriptEdit.SetCaseSensitive( true );
		scriptEdit.LoadKeyWordsFromFile( "editors/script.def" );
		scriptEdit.SetObjectMemberCallback( GetScriptEvents );
		scriptEdit.SetFunctionParmCallback( GetFunctionParms );
		scriptEdit.SetToolTipCallback( GetToolTip );
	} else if ( extension.Icmp( "gui" ) == 0 ) {
		scriptEdit.SetStringColor( SRE_COLOR_DARK_CYAN, SRE_COLOR_LIGHT_BROWN );
		scriptEdit.LoadKeyWordsFromFile( "editors/gui.def" );
	}

	if ( fileSystem->ReadFile( fileName, &buffer ) == -1 ) {
		return;
	}
	scriptText = (char *) buffer;
	fileSystem->FreeFile( buffer );

	this->fileName = fileName;

	// clean up new-line crapola
	scriptText.Replace( "\r", "" );
	scriptText.Replace( "\n", "\r" );
	scriptText.Replace( "\v", "\r" );

	scriptEdit.SetText( scriptText );

	for( const char *ptr = scriptText.c_str(); *ptr; ptr++ ) {
		if ( *ptr == '\r' ) {
			if ( numCharsPerLine > maxCharsPerLine ) {
				maxCharsPerLine = numCharsPerLine;
			}
			numCharsPerLine = 0;
			numLines++;
		} else if ( *ptr == '\t' ) {
			numCharsPerLine += TAB_SIZE;
		} else {
			numCharsPerLine++;
		}
	}

	SetWindowText( va( "Script Editor (%s)", fileName ) );

	rect.left = initialRect.left;
	rect.right = rect.left + maxCharsPerLine * FONT_WIDTH + 32;
	rect.top = initialRect.top;
	rect.bottom = rect.top + numLines * (FONT_HEIGHT+8) + 24 + 56;
	if ( rect.right < initialRect.right ) {
		rect.right = initialRect.right;
	} else if ( rect.right - rect.left > 1024 ) {
		rect.right = rect.left + 1024;
	}
	if ( rect.bottom < initialRect.bottom ) {
		rect.bottom = initialRect.bottom;
	} else if ( rect.bottom - rect.top > 768 ) {
		rect.bottom = rect.top + 768;
	}
	MoveWindow( rect );

	okButton.EnableWindow( FALSE );

	UpdateStatusBar();

	scriptEdit.SetFocus();
}

/*
================
DialogScriptEditor::OnInitDialog
================
*/
BOOL DialogScriptEditor::OnInitDialog()  {

	com_editors |= EDITOR_SCRIPT;

	CDialog::OnInitDialog();

	// load accelerator table
	m_hAccel = ::LoadAccelerators( AfxGetResourceHandle(), MAKEINTRESOURCE( IDR_ACCELERATOR_SCRIPTEDITOR ) );

	// create status bar
	statusBar.CreateEx( SBARS_SIZEGRIP, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, initialRect, this, AFX_IDW_STATUS_BAR );

	scriptEdit.LimitText( 1024 * 1024 );

	GetClientRect( initialRect );

	SetWindowText( "Script Editor" );

	EnableToolTips( TRUE );

	okButton.EnableWindow( FALSE );

	UpdateStatusBar();

	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(DialogScriptEditor, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
	ON_COMMAND(ID_SCRIPTEDITOR_FIND_NEXT, OnEditFindNext)
	ON_COMMAND(ID_SCRIPTEDITOR_GOTOLINE, OnEditGoToLine)
	ON_REGISTERED_MESSAGE(FindDialogMessage, OnFindDialogMessage)
	ON_NOTIFY(EN_CHANGE, IDC_SCRIPTEDITOR_EDIT_TEXT, OnEnChangeEdit)
	ON_NOTIFY(EN_MSGFILTER, IDC_SCRIPTEDITOR_EDIT_TEXT, OnEnInputEdit)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()

/*
================
ScriptEditorInit
================
*/
void ScriptEditorInit( const idDict *spawnArgs ) {

	if ( renderSystem->IsFullScreen() ) {
		common->Printf( "Cannot run the script editor in fullscreen mode.\n"
					"Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

	if ( g_ScriptDialog == NULL ) {
		InitAfx();
		g_ScriptDialog = new DialogScriptEditor();
	}

	if ( g_ScriptDialog->GetSafeHwnd() == NULL) {
		g_ScriptDialog->Create( IDD_DIALOG_SCRIPTEDITOR );
/*
		// FIXME: restore position
		CRect rct;
		g_ScriptDialog->SetWindowPos( NULL, rct.left, rct.top, 0, 0, SWP_NOSIZE );
*/
	}

	idKeyInput::ClearStates();

	g_ScriptDialog->ShowWindow( SW_SHOW );
	g_ScriptDialog->SetFocus();

	if ( spawnArgs ) {
	}
}

/*
================
ScriptEditorRun
================
*/
void ScriptEditorRun( void ) {
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
ScriptEditorShutdown
================
*/
void ScriptEditorShutdown( void ) {
	delete g_ScriptDialog;
	g_ScriptDialog = NULL;
	scriptEvents.Clear();
}


// DialogScriptEditor message handlers

/*
================
DialogScriptEditor::OnActivate
================
*/
void DialogScriptEditor::OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized ) {
	CDialog::OnActivate( nState, pWndOther, bMinimized );
}

/*
================
DialogScriptEditor::OnToolTipNotify
================
*/
BOOL DialogScriptEditor::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

/*
================
DialogScriptEditor::OnSetFocus
================
*/
void DialogScriptEditor::OnSetFocus( CWnd *pOldWnd ) {
	CDialog::OnSetFocus( pOldWnd );
}

/*
================
DialogScriptEditor::OnDestroy
================
*/
void DialogScriptEditor::OnDestroy() {
	return CDialog::OnDestroy();
}

/*
================
DialogScriptEditor::OnMove
================
*/
void DialogScriptEditor::OnMove( int x, int y ) {
	if ( GetSafeHwnd() ) {
		CRect rct;
		GetWindowRect( rct );
		// FIXME: save position
	}
	CDialog::OnMove( x, y );
}

/*
================
DialogScriptEditor::OnSize
================
*/
#define BORDER_SIZE			0
#define BUTTON_SPACE		4
#define TOOLBAR_HEIGHT		24

void DialogScriptEditor::OnSize( UINT nType, int cx, int cy ) {
	CRect clientRect, rect;

	LockWindowUpdate();

	CDialog::OnSize( nType, cx, cy );

	GetClientRect( clientRect );

	if ( scriptEdit.GetSafeHwnd() ) {
		rect.left = BORDER_SIZE;
		rect.top = BORDER_SIZE;
		rect.right = clientRect.Width() - BORDER_SIZE;
		rect.bottom = clientRect.Height() - 56;
		scriptEdit.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( okButton.GetSafeHwnd() ) {
		okButton.GetClientRect( rect );
		int width = rect.Width();
		int height = rect.Height();
		rect.left = clientRect.Width() - BORDER_SIZE - BUTTON_SPACE - 2 * width;
		rect.top = clientRect.Height() - TOOLBAR_HEIGHT - height;
		rect.right = clientRect.Width() - BORDER_SIZE - BUTTON_SPACE - width;
		rect.bottom = clientRect.Height() - TOOLBAR_HEIGHT;
		okButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( cancelButton.GetSafeHwnd() ) {
		cancelButton.GetClientRect( rect );
		int width = rect.Width();
		int height = rect.Height();
		rect.left = clientRect.Width() - BORDER_SIZE - width;
		rect.top = clientRect.Height() - TOOLBAR_HEIGHT - height;
		rect.right = clientRect.Width() - BORDER_SIZE;
		rect.bottom = clientRect.Height() - TOOLBAR_HEIGHT;
		cancelButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( statusBar.GetSafeHwnd() ) {
		rect.left = clientRect.Width() - 2;
		rect.top = clientRect.Height() - 2;
		rect.right = clientRect.Width() - 2;
		rect.bottom = clientRect.Height() - 2;
		statusBar.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	UnlockWindowUpdate();
}

/*
================
DialogScriptEditor::OnSizing
================
*/
void DialogScriptEditor::OnSizing( UINT nSide, LPRECT lpRect ) {
	/*
		1 = left
		2 = right
		3 = top
		4 = left - top
		5 = right - top
		6 = bottom
		7 = left - bottom
		8 = right - bottom
	*/

	CDialog::OnSizing( nSide, lpRect );

	if ( ( nSide - 1 ) % 3 == 0 ) {
		if ( lpRect->right - lpRect->left < initialRect.Width() ) {
			lpRect->left = lpRect->right - initialRect.Width();
		}
	} else if ( ( nSide - 2 ) % 3 == 0 ) {
		if ( lpRect->right - lpRect->left < initialRect.Width() ) {
			lpRect->right = lpRect->left + initialRect.Width();
		}
	}
	if ( nSide >= 3 && nSide <= 5 ) {
		if ( lpRect->bottom - lpRect->top < initialRect.Height() ) {
			lpRect->top = lpRect->bottom - initialRect.Height();
		}
	} else if ( nSide >= 6 && nSide <= 9 ) {
		if ( lpRect->bottom - lpRect->top < initialRect.Height() ) {
			lpRect->bottom = lpRect->top + initialRect.Height();
		}
	}
}

/*
================
DialogScriptEditor::OnEditGoToLine
================
*/
void DialogScriptEditor::OnEditGoToLine() {
	DialogGoToLine goToLineDlg;

	goToLineDlg.SetRange( firstLine, firstLine + scriptEdit.GetLineCount() - 1 );
	if ( goToLineDlg.DoModal() != IDOK ) {
		return;
	}
	scriptEdit.GoToLine( goToLineDlg.GetLine() - firstLine );
}

/*
================
DialogScriptEditor::OnEditFind
================
*/
void DialogScriptEditor::OnEditFind() {

	CString selText = scriptEdit.GetSelText();
	if ( selText.GetLength() ) {
		findStr = selText;
	}

	// create find/replace dialog
	if ( !findDlg ) {
		findDlg = new CFindReplaceDialog();  // Must be created on the heap
		findDlg->Create( TRUE, findStr, "", FR_DOWN, this );
	}
}

/*
================
DialogScriptEditor::OnEditFindNext
================
*/
void DialogScriptEditor::OnEditFindNext() {
	if ( scriptEdit.FindNext( findStr, matchCase, matchWholeWords, searchForward ) ) {
		scriptEdit.SetFocus();
	} else {
		AfxMessageBox( "The specified text was not found.", MB_OK | MB_ICONINFORMATION, 0 );
	}
}

/*
================
DialogScriptEditor::OnEditReplace
================
*/
void DialogScriptEditor::OnEditReplace() {

	CString selText = scriptEdit.GetSelText();
	if ( selText.GetLength() ) {
		findStr = selText;
	}

	// create find/replace dialog
	if ( !findDlg ) {
		findDlg = new CFindReplaceDialog();  // Must be created on the heap
		findDlg->Create( FALSE, findStr, "", FR_DOWN, this );
	}
}

/*
================
DialogScriptEditor::OnFindDialogMessage
================
*/
LRESULT DialogScriptEditor::OnFindDialogMessage( WPARAM wParam, LPARAM lParam ) {
	if ( findDlg == NULL ) {
		return 0;
	}

	if ( findDlg->IsTerminating() ) {
        findDlg = NULL;
        return 0;
    }

	if( findDlg->FindNext() ) {
		findStr = findDlg->GetFindString();
		matchCase = findDlg->MatchCase() != FALSE;
		matchWholeWords = findDlg->MatchWholeWord() != FALSE;
		searchForward = findDlg->SearchDown() != FALSE;

		OnEditFindNext();
    }

	if ( findDlg->ReplaceCurrent() ) {
		long selStart, selEnd;

		replaceStr = findDlg->GetReplaceString();

		scriptEdit.GetSel( selStart, selEnd );
		if ( selEnd > selStart ) {
			scriptEdit.ReplaceSel( replaceStr, TRUE );
		}
	}

	if ( findDlg->ReplaceAll() ) {
		replaceStr = findDlg->GetReplaceString();
		findStr = findDlg->GetFindString();
		matchCase = findDlg->MatchCase() != FALSE;
		matchWholeWords = findDlg->MatchWholeWord() != FALSE;

		int numReplaces = scriptEdit.ReplaceAll( findStr, replaceStr, matchCase, matchWholeWords );
		if ( numReplaces == 0 ) {
			AfxMessageBox( "The specified text was not found.", MB_OK | MB_ICONINFORMATION, 0 );
		} else {
			AfxMessageBox( va( "Replaced %d occurances.", numReplaces ), MB_OK | MB_ICONINFORMATION, 0 );
		}
	}

	return 0;
}

/*
================
DialogScriptEditor::OnEnChangeEdit
================
*/
void DialogScriptEditor::OnEnChangeEdit( NMHDR *pNMHDR, LRESULT *pResult ) {
	okButton.EnableWindow( TRUE );
}

/*
================
DialogScriptEditor::OnEnInputEdit
================
*/
void DialogScriptEditor::OnEnInputEdit( NMHDR *pNMHDR, LRESULT *pResult ) {
	MSGFILTER *msgFilter = (MSGFILTER *)pNMHDR;

	if ( msgFilter->msg != 512 && msgFilter->msg != 33 ) {
		UpdateStatusBar();
	}

	*pResult = 0;
}

/*
================
DialogScriptEditor::OnBnClickedOk
================
*/
void DialogScriptEditor::OnBnClickedOk() {
	idStr scriptText;

	common->Printf( "Writing \'%s\'...\n", fileName.c_str() );

	scriptEdit.GetText( scriptText );

	// clean up new-line crapola
	scriptText.Replace( "\n", "" );
	scriptText.Replace( "\r", "\r\n" );
	scriptText.Replace( "\v", "\r\n" );

	if ( fileSystem->WriteFile( fileName, scriptText, scriptText.Length(), "fs_devpath" ) == -1 ) {
		MessageBox( va( "Couldn't save: %s", fileName.c_str() ), va( "Error saving: %s", fileName.c_str() ), MB_OK | MB_ICONERROR );
		return;
	}

	okButton.EnableWindow( FALSE );
}

/*
================
DialogScriptEditor::OnBnClickedCancel
================
*/
void DialogScriptEditor::OnBnClickedCancel() {
	if ( okButton.IsWindowEnabled() ) {
		if ( MessageBox( "Cancel changes?", "Cancel", MB_YESNO | MB_ICONQUESTION ) != IDYES ) {
			return;
		}
	}
	OnCancel();
}

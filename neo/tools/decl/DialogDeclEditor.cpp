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
#include "../../sys/win32/rc/DeclEditor_resource.h"

#include "../comafx/DialogGoToLine.h"
#include "../comafx/CPathTreeCtrl.h"
#include "DialogDeclBrowser.h"
#include "DialogDeclEditor.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif


// DialogDeclEditor dialog

static UINT FindDialogMessage = ::RegisterWindowMessage( FINDMSGSTRING );

toolTip_t DialogDeclEditor::toolTips[] = {
	{ IDC_DECLEDITOR_BUTTON_TEST, "test decl" },
	{ IDOK, "save decl" },
	{ IDCANCEL, "cancel" },
	{ 0, NULL }
};


IMPLEMENT_DYNAMIC(DialogDeclEditor, CDialog)

/*
================
DialogDeclEditor::DialogDeclEditor
================
*/
DialogDeclEditor::DialogDeclEditor( CWnd* pParent /*=NULL*/ )
	: CDialog(DialogDeclEditor::IDD, pParent)
	, findDlg(NULL)
	, matchCase(false)
	, matchWholeWords(false)
	, decl(NULL)
	, firstLine(0)
{
}

/*
================
DialogDeclEditor::~DialogDeclEditor
================
*/
DialogDeclEditor::~DialogDeclEditor() {
}

/*
================
DialogDeclEditor::DoDataExchange
================
*/
void DialogDeclEditor::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogDeclEditor)
	DDX_Control(pDX, IDC_DECLEDITOR_EDIT_TEXT, declEdit);
	DDX_Control(pDX, IDC_DECLEDITOR_BUTTON_TEST, testButton);
	DDX_Control(pDX, IDOK, okButton);
	DDX_Control(pDX, IDCANCEL, cancelButton);
	//}}AFX_DATA_MAP
}

/*
================
DialogDeclEditor::PreTranslateMessage
================
*/
BOOL DialogDeclEditor::PreTranslateMessage( MSG* pMsg ) {
	if ( WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST ) {
		if ( m_hAccel && ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) ) {
			return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

/*
================
DialogDeclEditor::TestDecl
================
*/
bool DialogDeclEditor::TestDecl( const idStr &declText ) {
	idLexer src( LEXFL_NOSTRINGCONCAT );
	idToken token;
	int indent;

	src.LoadMemory( declText, declText.Length(), "decl text" );

	indent = 0;
	while( src.ReadToken( &token ) ) {
		if ( token == "{" ) {
			indent++;
		} else if ( token == "}" ) {
			indent--;
		}
	}

	if ( indent < 0 ) {
		MessageBox( "Missing opening brace!", va( "Error saving %s", decl->GetFileName() ), MB_OK | MB_ICONERROR );
		return false;
	}
	if ( indent > 0 ) {
		MessageBox( "Missing closing brace!", va( "Error saving %s", decl->GetFileName() ), MB_OK | MB_ICONERROR );
		return false;
	}
	return true;
}

/*
================
DialogDeclEditor::UpdateStatusBar
================
*/
void DialogDeclEditor::UpdateStatusBar( void ) {
	int line, column, character;

	if ( decl ) {
		declEdit.GetCursorPos( line, column, character );
		statusBar.SetWindowText( va( "Line: %d, Column: %d, Character: %d", decl->GetLineNum() + line, column, character ) );
	}
}

/*
================
DialogDeclEditor::LoadDecl
================
*/
void DialogDeclEditor::LoadDecl( idDecl *decl ) {
	int numLines = 0;
	int numCharsPerLine = 0;
	int maxCharsPerLine = 0;
	idStr declText;
	CRect rect;

	this->decl = decl;

	switch( decl->GetType() ) {
		case DECL_ENTITYDEF:
			declEdit.SetStringColor( SRE_COLOR_BLUE, SRE_COLOR_DARK_CYAN );
			declEdit.LoadKeyWordsFromFile( "editors/entity.def" );
			break;
		case DECL_MATERIAL:
			declEdit.LoadKeyWordsFromFile( "editors/material.def" );
			break;
		case DECL_SKIN:
			declEdit.LoadKeyWordsFromFile( "editors/skin.def" );
			break;
		case DECL_SOUND:
			declEdit.LoadKeyWordsFromFile( "editors/sound.def" );
			break;
		case DECL_FX:
			declEdit.LoadKeyWordsFromFile( "editors/fx.def" );
			break;
		case DECL_PARTICLE:
			declEdit.LoadKeyWordsFromFile( "editors/particle.def" );
			break;
		case DECL_AF:
			declEdit.LoadKeyWordsFromFile( "editors/af.def" );
			break;
		case DECL_TABLE:
			declEdit.LoadKeyWordsFromFile( "editors/table.def" );
			break;
		case DECL_MODELDEF:
			declEdit.LoadKeyWordsFromFile( "editors/model.def" );
			break;
		default:
			declEdit.LoadKeyWordsFromFile( va( "editors/%s.def", declManager->GetDeclNameFromType( decl->GetType() ) ) );
			break;
	}

	firstLine = decl->GetLineNum();

	char *localDeclText = (char *)_alloca( ( decl->GetTextLength() + 1 ) * sizeof( char ) );
	decl->GetText( localDeclText );
	declText = localDeclText;

	// clean up new-line crapola
	declText.Replace( "\r", "" );
	declText.Replace( "\n", "\r" );
	declText.Replace( "\v", "\r" );
	declText.StripLeading( '\r' );
	declText.Append( "\r" );

	declEdit.SetText( declText );

	for( const char *ptr = declText.c_str(); *ptr; ptr++ ) {
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

	SetWindowText( va( "Declaration Editor (%s, line %d)", decl->GetFileName(), decl->GetLineNum() ) );

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

	testButton.EnableWindow( FALSE );
	okButton.EnableWindow( FALSE );

	UpdateStatusBar();

	declEdit.SetFocus();
}

/*
================
DialogDeclEditor::OnInitDialog
================
*/
BOOL DialogDeclEditor::OnInitDialog()  {

	com_editors |= EDITOR_DECL;

	CDialog::OnInitDialog();

	// load accelerator table
	m_hAccel = ::LoadAccelerators( AfxGetResourceHandle(), MAKEINTRESOURCE( IDR_ACCELERATOR_DECLEDITOR ) );

	// create status bar
	statusBar.CreateEx( SBARS_SIZEGRIP, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, initialRect, this, AFX_IDW_STATUS_BAR );

	declEdit.Init();

	GetClientRect( initialRect );

	EnableToolTips( TRUE );

	testButton.EnableWindow( FALSE );
	okButton.EnableWindow( FALSE );

	UpdateStatusBar();

	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(DialogDeclEditor, CDialog)
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
	ON_COMMAND(ID_DECLEDITOR_FIND_NEXT, OnEditFindNext)
	ON_COMMAND(ID_DECLEDITOR_GOTOLINE, OnEditGoToLine)
	ON_REGISTERED_MESSAGE(FindDialogMessage, OnFindDialogMessage)
	ON_NOTIFY(EN_CHANGE, IDC_DECLEDITOR_EDIT_TEXT, OnEnChangeEdit)
	ON_NOTIFY(EN_MSGFILTER, IDC_DECLEDITOR_EDIT_TEXT, OnEnInputEdit)
	ON_BN_CLICKED(IDC_DECLEDITOR_BUTTON_TEST, OnBnClickedTest)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// DialogDeclEditor message handlers

/*
================
DialogDeclEditor::OnActivate
================
*/
void DialogDeclEditor::OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized ) {
	CDialog::OnActivate( nState, pWndOther, bMinimized );
}

/*
================
DialogDeclEditor::OnToolTipNotify
================
*/
BOOL DialogDeclEditor::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

/*
================
DialogDeclEditor::OnSetFocus
================
*/
void DialogDeclEditor::OnSetFocus( CWnd *pOldWnd ) {
	CDialog::OnSetFocus( pOldWnd );
}

/*
================
DialogDeclEditor::OnDestroy
================
*/
void DialogDeclEditor::OnDestroy() {
	return CDialog::OnDestroy();
}

/*
================
DialogDeclEditor::OnMove
================
*/
void DialogDeclEditor::OnMove( int x, int y ) {
	if ( GetSafeHwnd() ) {
		CRect rct;
		GetWindowRect( rct );
		// FIXME: save position
	}
	CDialog::OnMove( x, y );
}

/*
================
DialogDeclEditor::OnSize
================
*/
#define BORDER_SIZE			0
#define BUTTON_SPACE		4
#define TOOLBAR_HEIGHT		24

void DialogDeclEditor::OnSize( UINT nType, int cx, int cy ) {
	CRect clientRect, rect;

	LockWindowUpdate();

	CDialog::OnSize( nType, cx, cy );

	GetClientRect( clientRect );

	if ( declEdit.GetSafeHwnd() ) {
		rect.left = BORDER_SIZE;
		rect.top = BORDER_SIZE;
		rect.right = clientRect.Width() - BORDER_SIZE;
		rect.bottom = clientRect.Height() - 56;
		declEdit.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( testButton.GetSafeHwnd() ) {
		testButton.GetClientRect( rect );
		int width = rect.Width();
		int height = rect.Height();
		rect.left = BORDER_SIZE;
		rect.top = clientRect.Height() - TOOLBAR_HEIGHT - height;
		rect.right = BORDER_SIZE + width;
		rect.bottom = clientRect.Height() - TOOLBAR_HEIGHT;
		testButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
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
DialogDeclEditor::OnSizing
================
*/
void DialogDeclEditor::OnSizing( UINT nSide, LPRECT lpRect ) {
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
DialogDeclEditor::OnEditGoToLine
================
*/
void DialogDeclEditor::OnEditGoToLine() {
	DialogGoToLine goToLineDlg;

	goToLineDlg.SetRange( firstLine, firstLine + declEdit.GetLineCount() - 1 );
	if ( goToLineDlg.DoModal() != IDOK ) {
		return;
	}
	declEdit.GoToLine( goToLineDlg.GetLine() - firstLine );
}

/*
================
DialogDeclEditor::OnEditFind
================
*/
void DialogDeclEditor::OnEditFind() {

	CString selText = declEdit.GetSelText();
	if ( selText.GetLength() ) {
		findStr = selText;
	}

	if ( !findDlg ) {
		// create find/replace dialog
		findDlg = new CFindReplaceDialog();  // Must be created on the heap
		findDlg->Create( TRUE, findStr, "", FR_DOWN, this );
	}
}

/*
================
DialogDeclEditor::OnEditFindNext
================
*/
void DialogDeclEditor::OnEditFindNext() {
	if ( declEdit.FindNext( findStr, matchCase, matchWholeWords, searchForward ) ) {
		declEdit.SetFocus();
	} else {
		AfxMessageBox( "The specified text was not found.", MB_OK | MB_ICONINFORMATION, 0 );
	}
}

/*
================
DialogDeclEditor::OnEditReplace
================
*/
void DialogDeclEditor::OnEditReplace() {

	CString selText = declEdit.GetSelText();
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
DialogDeclEditor::OnFindDialogMessage
================
*/
LRESULT DialogDeclEditor::OnFindDialogMessage( WPARAM wParam, LPARAM lParam ) {
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

		declEdit.GetSel( selStart, selEnd );
		if ( selEnd > selStart ) {
			declEdit.ReplaceSel( replaceStr, TRUE );
		}
	}

	if ( findDlg->ReplaceAll() ) {
		replaceStr = findDlg->GetReplaceString();
		findStr = findDlg->GetFindString();
		matchCase = findDlg->MatchCase() != FALSE;
		matchWholeWords = findDlg->MatchWholeWord() != FALSE;

		int numReplaces = declEdit.ReplaceAll( findStr, replaceStr, matchCase, matchWholeWords );
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
DialogDeclEditor::OnEnChangeEdit
================
*/
void DialogDeclEditor::OnEnChangeEdit( NMHDR *pNMHDR, LRESULT *pResult ) {
	testButton.EnableWindow( TRUE );
	okButton.EnableWindow( TRUE );
}

/*
================
DialogDeclEditor::OnEnInputEdit
================
*/
void DialogDeclEditor::OnEnInputEdit( NMHDR *pNMHDR, LRESULT *pResult ) {
	MSGFILTER *msgFilter = (MSGFILTER *)pNMHDR;

	if ( msgFilter->msg != 512 && msgFilter->msg != 33 ) {
		UpdateStatusBar();
	}

	*pResult = 0;
}

/*
================
DialogDeclEditor::OnBnClickedTest
================
*/
void DialogDeclEditor::OnBnClickedTest() {
	idStr declText;

	if ( decl ) {

		declEdit.GetText( declText );

		// clean up new-line crapola
		declText.Replace( "\n", "" );
		declText.Replace( "\r", "\r\n" );
		declText.Replace( "\v", "\r\n" );
		declText.StripLeading( "\r\n" );
		declText.Insert( "\r\n\r\n", 0 );
		declText.StripTrailing( "\r\n" );

		if ( !TestDecl( declText ) ) {
			return;
		}

		char *oldDeclText = (char *)_alloca( ( decl->GetTextLength() + 1 ) * sizeof( char ) );
		decl->GetText( oldDeclText );
		decl->SetText( declText );
		decl->Invalidate();
		declManager->DeclByIndex( decl->GetType(), decl->Index(), true );
		decl->SetText( oldDeclText );
		decl->Invalidate();
		common->Printf( "tested %s\n", decl->GetName() );

		testButton.EnableWindow( FALSE );
	}
}

/*
================
DialogDeclEditor::OnBnClickedOk
================
*/
void DialogDeclEditor::OnBnClickedOk() {
	idStr declText;

	if ( decl ) {

		declEdit.GetText( declText );

		// clean up new-line crapola
		declText.Replace( "\n", "" );
		declText.Replace( "\r", "\r\n" );
		declText.Replace( "\v", "\r\n" );
		declText.StripLeading( "\r\n" );
		declText.Insert( "\r\n\r\n", 0 );
		declText.StripTrailing( "\r\n" );

		if ( !TestDecl( declText ) ) {
			return;
		}

		if ( decl->SourceFileChanged() ) {
			if ( MessageBox( va( "Declaration file %s has been modified outside of the editor.\r\nReload declarations and save?", decl->GetFileName() ),
							va( "Warning saving: %s", decl->GetFileName() ), MB_OKCANCEL | MB_ICONERROR ) != IDOK ) {
				return;
			}
			declManager->Reload( false );
			DeclBrowserReloadDeclarations();
		}

		decl->SetText( declText );
		if ( !decl->ReplaceSourceFileText() ) {
			MessageBox( va( "Couldn't save: %s.\r\nMake sure the declaration file is not read-only.", decl->GetFileName() ),
						va( "Error saving: %s", decl->GetFileName() ), MB_OK | MB_ICONERROR );
			return;
		}
		decl->Invalidate();
	}

	okButton.EnableWindow( FALSE );
}

/*
================
DialogDeclEditor::OnBnClickedCancel
================
*/
void DialogDeclEditor::OnBnClickedCancel() {
	if ( okButton.IsWindowEnabled() ) {
		if ( MessageBox( "Cancel changes?", "Cancel", MB_YESNO | MB_ICONQUESTION ) != IDYES ) {
			return;
		}
	}
	OnCancel();
}

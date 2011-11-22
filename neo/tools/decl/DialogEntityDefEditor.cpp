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

#include "../comafx/CPathTreeCtrl.h"
#include "DialogDeclBrowser.h"
#include "DialogDeclEditor.h"
#include "DialogEntityDefEditor.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif


// DialogEntityDefEditor dialog

toolTip_t DialogEntityDefEditor::toolTips[] = {
	{ IDC_DECLEDITOR_BUTTON_TEST, "Test Decl" },
	{ IDOK, "Save Decl" },
	{ IDCANCEL, "Cancel" },
	{ 0, NULL }
};


IMPLEMENT_DYNAMIC(DialogEntityDefEditor, CDialog)

/*
================
DialogEntityDefEditor::DialogEntityDefEditor
================
*/
DialogEntityDefEditor::DialogEntityDefEditor( CWnd* pParent /*=NULL*/ )
	: CDialog(DialogEntityDefEditor::IDD, pParent)
	, decl(NULL)
	, firstLine(0)
{
}

/*
================
DialogEntityDefEditor::~DialogEntityDefEditor
================
*/
DialogEntityDefEditor::~DialogEntityDefEditor() {
}

/*
================
DialogEntityDefEditor::DoDataExchange
================
*/
void DialogEntityDefEditor::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogEntityDefEditor)
	DDX_Control(pDX, IDC_ENTITYDEFEDITOR_EDIT_DECLNAME, declNameEdit);
	DDX_Control(pDX, IDC_ENTITYDEFEDITOR_COMBO_INHERIT, inheritCombo);
	DDX_Control(pDX, IDC_ENTITYDEFEDITOR_COMBO_SPAWNCLASS, spawnclassCombo);

	DDX_Control(pDX, IDC_ENTITYDEFEDITOR_LIST_KEYVALS, keyValsList);

	DDX_Control(pDX, IDC_ENTITYDEFEDITOR_STATIC_KEY, keyLabel);
	DDX_Control(pDX, IDC_ENTITYDEFEDITOR_EDIT_KEY, keyEdit);
	DDX_Control(pDX, IDC_ENTITYDEFEDITOR_BUTTON_ADD, addButton);
	DDX_Control(pDX, IDC_ENTITYDEFEDITOR_BUTTON_DELETE, delButton);
	DDX_Control(pDX, IDC_ENTITYDEFEDITOR_LINE, line);

	DDX_Control(pDX, IDC_DECLEDITOR_BUTTON_TEST, testButton);
	DDX_Control(pDX, IDOK, okButton);
	DDX_Control(pDX, IDCANCEL, cancelButton);
	//}}AFX_DATA_MAP
}

/*
================
DialogEntityDefEditor::PreTranslateMessage
================
*/
BOOL DialogEntityDefEditor::PreTranslateMessage( MSG* pMsg ) {
	if ( WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST ) {
		if ( m_hAccel && ::TranslateAccelerator( m_hWnd, m_hAccel, pMsg ) ) {
			return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

/*
================
DialogEntityDefEditor::TestDecl
================
*/
bool DialogEntityDefEditor::TestDecl( const idStr &declText ) {
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
DialogEntityDefEditor::UpdateStatusBar
================
*/
void DialogEntityDefEditor::UpdateStatusBar( void ) {
	if ( decl ) {
		statusBar.SetWindowText( "Editing decl" );
	}
}

/*
================
DialogEntityDefEditor::LoadDecl
================
*/
void DialogEntityDefEditor::LoadDecl( idDeclEntityDef *decl ) {
	int numLines = 0;
	CRect rect;

	this->decl = decl;

	// Fill up the spawnclass box with all spawn classes
	/*
	idTypeInfo *c = idClass::GetClass("idClass");
	for (; c; c = c->next) {
		spawnclassCombo.AddString(c->classname);
	}
	*/

	// Fill up the inherit box with all entitydefs
	int numDecls = declManager->GetNumDecls(DECL_ENTITYDEF);
	for (int i=0; i<numDecls; i++) {
		const idDecl *d = declManager->DeclByIndex(DECL_ENTITYDEF, i, false);
		if (d) {
			inheritCombo.AddString(d->GetName());
		}
	}

	firstLine = decl->GetLineNum();

	char *declText = (char *)_alloca( ( decl->GetTextLength() + 1 ) * sizeof( char ) );
	decl->GetText( declText );

	PopulateLists( declText );

	SetWindowText( va( "EntityDef Editor (%s, line %d)", decl->GetFileName(), decl->GetLineNum() ) );

	// Hack to get it to reflow the window
	GetWindowRect( rect );
	rect.bottom++;
	MoveWindow( rect );

	testButton.EnableWindow( FALSE );
	okButton.EnableWindow( FALSE );

	UpdateStatusBar();
}

/*
=================
DialogEntityDefEditor::PopulateLists
=================
*/
void DialogEntityDefEditor::PopulateLists( const char *declText, const int textLength )
{
	idLexer src;
	idToken	token, token2;

	idDict dict;

	src.LoadMemory( declText, textLength, decl->GetFileName(), firstLine);
	src.SetFlags( DECL_LEXER_FLAGS );
	src.SkipUntilString( "{" );

	while (1) {
		if ( !src.ReadToken( &token ) ) {
			break;
		}

		if ( !token.Icmp( "}" ) ) {
			break;
		}
		if ( token.type != TT_STRING ) {
			src.Warning( "Expected quoted string, but found '%s'", token.c_str() );
			break;
		}

		if ( !src.ReadToken( &token2 ) ) {
			src.Warning( "Unexpected end of file" );
			break;
		}

		if ( dict.FindKey( token ) ) {
			src.Warning( "'%s' already defined", token.c_str() );
		}
		dict.Set( token, token2 );
	}

	// Get the parent, and remove the 'inherit' key so it doesn't show up in the list
	// We currently don't support multiple inheritence properly, but nothing uses it anyway
	idStr inherit;
	const idKeyValue *inheritKeyVal = dict.FindKey("inherit");
	if (inheritKeyVal) {
		inherit = inheritKeyVal->GetValue();
		dict.Delete(inheritKeyVal->GetKey());
	}

	idStr spawnclass = dict.GetString("spawnclass", "");
	dict.Delete("spawnclass");

	keyValsList.ResetContent();

	// Fill up the list with all the main info
	size_t numPairs = dict.Size();
	for (unsigned int i=0; i<numPairs; i++) {
		const idKeyValue *keyVal = dict.GetKeyVal(i);
		if (keyVal) {
			keyValsList.AddPropItem(new CPropertyItem(keyVal->GetKey().c_str(), keyVal->GetValue().c_str(), PIT_EDIT, ""));
		}
	}

	//inheritCombo.SelectString(0, inherit);
	SetInherit(inherit);
	inheritCombo.SelectString(0, inherit);

	declNameEdit.SetWindowText(decl->GetName());
	int index = spawnclassCombo.FindString(0, spawnclass);
	if (index == CB_ERR) {
		index = spawnclassCombo.AddString(spawnclass);
	}
	spawnclassCombo.SetCurSel(index);
}

/*
=================
DialogEntityDefEditor::SetInherit
=================
*/

void DialogEntityDefEditor::SetInherit(idStr &inherit)
{
	CWaitCursor wc;

	for (int i=0; i<keyValsList.GetCount(); i++) {
		CPropertyItem* pItem = (CPropertyItem*)keyValsList.GetItemDataPtr(i);
		if (pItem) {
			if (pItem->m_propName[0] == '*') {
				delete pItem;
				keyValsList.DeleteString(i);
				i--;
			}
		}
	}

	CString spawnclass;
	// Fill up the rest of the box with inherited info
	if (!inherit.IsEmpty()) {
		const idDecl *temp = declManager->FindType(DECL_ENTITYDEF, inherit, false);
		const idDeclEntityDef *parent = static_cast<const idDeclEntityDef *>(temp);
		if (parent) {
			size_t numPairs = parent->dict.Size();
			for (unsigned int i=0; i<numPairs; i++) {
				const idKeyValue *keyVal = parent->dict.GetKeyVal(i);
				if (keyVal) {
					if (spawnclass.IsEmpty() && keyVal->GetKey() == "spawnclass") {
						spawnclass = keyVal->GetValue();
					}
					else {
						CString key = keyVal->GetKey();
						key = "*" + key;
						keyValsList.AddPropItem(new CPropertyItem(key, keyVal->GetValue().c_str(), PIT_EDIT, ""));
					}
				}
			}
		}
	}
}

/*
================
DialogEntityDefEditor::OnInitDialog
================
*/
BOOL DialogEntityDefEditor::OnInitDialog()  {

	com_editors |= EDITOR_ENTITYDEF;

	CDialog::OnInitDialog();

	// load accelerator table
	m_hAccel = ::LoadAccelerators( AfxGetResourceHandle(), MAKEINTRESOURCE( IDR_ACCELERATOR_DECLEDITOR ) );

	// create status bar
	statusBar.CreateEx( SBARS_SIZEGRIP, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, initialRect, this, AFX_IDW_STATUS_BAR );

	GetClientRect( initialRect );

	EnableToolTips( TRUE );

	testButton.EnableWindow( FALSE );
	okButton.EnableWindow( FALSE );

	UpdateStatusBar();

	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(DialogEntityDefEditor, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_SETFOCUS()
	ON_CBN_EDITCHANGE(IDC_ENTITYDEFEDITOR_COMBO_INHERIT, OnInheritChange)
	ON_CBN_SELCHANGE(IDC_ENTITYDEFEDITOR_COMBO_INHERIT, OnInheritChange)
	ON_CBN_EDITCHANGE(IDC_ENTITYDEFEDITOR_COMBO_SPAWNCLASS, OnEditChange)
	ON_EN_CHANGE(IDC_ENTITYDEFEDITOR_EDIT_DECLNAME, OnEditChange)
	ON_NOTIFY(EN_MSGFILTER, IDC_DECLEDITOR_EDIT_TEXT, OnEnInputEdit)

	ON_LBN_SELCHANGE(IDC_ENTITYDEFEDITOR_LIST_KEYVALS, OnKeyValChange)

	ON_BN_CLICKED(IDC_ENTITYDEFEDITOR_BUTTON_ADD, OnBnClickedAdd)
	ON_BN_CLICKED(IDC_ENTITYDEFEDITOR_BUTTON_DELETE, OnBnClickedDelete)

	ON_BN_CLICKED(IDC_DECLEDITOR_BUTTON_TEST, OnBnClickedTest)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)

END_MESSAGE_MAP()


// DialogEntityDefEditor message handlers

/*
================
DialogEntityDefEditor::OnActivate
================
*/
void DialogEntityDefEditor::OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized ) {
	CDialog::OnActivate( nState, pWndOther, bMinimized );
}

/*
================
DialogEntityDefEditor::OnToolTipNotify
================
*/
BOOL DialogEntityDefEditor::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

/*
================
DialogEntityDefEditor::OnSetFocus
================
*/
void DialogEntityDefEditor::OnSetFocus( CWnd *pOldWnd ) {
	CDialog::OnSetFocus( pOldWnd );
}

/*
================
DialogEntityDefEditor::OnDestroy
================
*/
void DialogEntityDefEditor::OnDestroy() {
	return CDialog::OnDestroy();
}

/*
================
DialogEntityDefEditor::OnMove
================
*/
void DialogEntityDefEditor::OnMove( int x, int y ) {
	if ( GetSafeHwnd() ) {
		CRect rct;
		GetWindowRect( rct );
		// FIXME: save position
	}
	CDialog::OnMove( x, y );
}

/*
================
DialogEntityDefEditor::OnSize
================
*/
#define BORDER_SIZE			4
#define BUTTON_SPACE		4
#define CONTROL_HEIGHT		24
#define TOOLBAR_HEIGHT		24

void DialogEntityDefEditor::OnSize( UINT nType, int cx, int cy ) {
	CRect clientRect, rect;

	LockWindowUpdate();

	CDialog::OnSize( nType, cx, cy );

	GetClientRect( clientRect );

	if ( keyValsList.GetSafeHwnd() ) {
		keyValsList.GetClientRect( rect );
		rect.left = BORDER_SIZE;
		rect.right = clientRect.right - BORDER_SIZE;
		rect.top = (TOOLBAR_HEIGHT * 2) + (BUTTON_SPACE * 3);
		rect.bottom = clientRect.bottom - (TOOLBAR_HEIGHT * 4) - BUTTON_SPACE;
		keyValsList.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	int keyRowTop = clientRect.Height() - TOOLBAR_HEIGHT * 3 - CONTROL_HEIGHT;
	int keyRowBottom = keyRowTop + CONTROL_HEIGHT;

	int lineTop = clientRect.Height() - TOOLBAR_HEIGHT * 3 + (CONTROL_HEIGHT / 2);

	int buttonRowTop = clientRect.Height() - TOOLBAR_HEIGHT - CONTROL_HEIGHT;
	int buttonRowBottom = buttonRowTop + CONTROL_HEIGHT;

	if ( keyLabel.GetSafeHwnd() ) {
		keyLabel.GetClientRect( rect );
		int width = rect.Width();
		rect.left = BORDER_SIZE;
		rect.right = BORDER_SIZE + width;
		rect.top = keyRowTop + 8;
		rect.bottom = keyRowBottom;
		keyLabel.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( keyEdit.GetSafeHwnd() ) {
		keyEdit.GetClientRect( rect );
		rect.left = 40;
		rect.right = 40 + 200;
		rect.top = keyRowTop;
		rect.bottom = keyRowBottom;
		keyEdit.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}


	if ( addButton.GetSafeHwnd() ) {
		addButton.GetClientRect( rect );
		int width = rect.Width();
		rect.left = clientRect.Width() - BORDER_SIZE - BUTTON_SPACE - 2 * width;
		rect.right = clientRect.Width() - BORDER_SIZE - BUTTON_SPACE - width;
		rect.top = keyRowTop;
		rect.bottom = keyRowBottom;
		addButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( delButton.GetSafeHwnd() ) {
		delButton.GetClientRect( rect );
		int width = rect.Width();
		rect.left = clientRect.Width() - BORDER_SIZE - width;
		rect.right = clientRect.Width() - BORDER_SIZE;
		rect.top = keyRowTop;
		rect.bottom = keyRowBottom;
		delButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( line.GetSafeHwnd() ) {
		line.GetClientRect( rect );
		int height = rect.Height();
		rect.left = BORDER_SIZE;
		rect.right = clientRect.Width() - BORDER_SIZE;
		rect.top = lineTop;
		rect.bottom = lineTop + 3;
		line.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( testButton.GetSafeHwnd() ) {
		testButton.GetClientRect( rect );
		int width = rect.Width();
		rect.left = BORDER_SIZE;
		rect.right = BORDER_SIZE + width;
		rect.top = buttonRowTop;
		rect.bottom = buttonRowBottom;
		testButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( okButton.GetSafeHwnd() ) {
		okButton.GetClientRect( rect );
		int width = rect.Width();
		rect.left = clientRect.Width() - BORDER_SIZE - BUTTON_SPACE - 2 * width;
		rect.right = clientRect.Width() - BORDER_SIZE - BUTTON_SPACE - width;
		rect.top = buttonRowTop;
		rect.bottom = buttonRowBottom;
		okButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( cancelButton.GetSafeHwnd() ) {
		cancelButton.GetClientRect( rect );
		int width = rect.Width();
		rect.left = clientRect.Width() - BORDER_SIZE - width;
		rect.right = clientRect.Width() - BORDER_SIZE;
		rect.top = buttonRowTop;
		rect.bottom = buttonRowBottom;
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
DialogEntityDefEditor::OnSizing
================
*/
void DialogEntityDefEditor::OnSizing( UINT nSide, LPRECT lpRect ) {
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
DialogEntityDefEditor::OnEditChange
================
*/
void DialogEntityDefEditor::OnEditChange( ) {
	testButton.EnableWindow( TRUE );
	okButton.EnableWindow( TRUE );
}

/*
================
DialogEntityDefEditor::OnInheritChange
================
*/
void DialogEntityDefEditor::OnInheritChange( ) {
	testButton.EnableWindow( TRUE );
	okButton.EnableWindow( TRUE );

	idStr inherit = "";

	int sel = inheritCombo.GetCurSel();
	if ( sel == CB_ERR ) {
		CString temp;
		inheritCombo.GetWindowText( temp );
		inherit = temp;
	} else {
		CString temp;
		inheritCombo.GetLBText( sel, temp );
		inherit = temp;
	}
	SetInherit(inherit);
}


/*
================
DialogEntityDefEditor::OnEnInputEdit
================
*/
void DialogEntityDefEditor::OnEnInputEdit( NMHDR *pNMHDR, LRESULT *pResult ) {
	MSGFILTER *msgFilter = (MSGFILTER *)pNMHDR;

	if ( msgFilter->msg != 512 && msgFilter->msg != 33 ) {
		UpdateStatusBar();
	}

	*pResult = 0;
}

/*
================
DialogEntityDefEditor::BuildDeclText
================
*/

void DialogEntityDefEditor::BuildDeclText( idStr &declText )
{
	CString declName;
	declNameEdit.GetWindowText(declName);
	CString inherit;
	inheritCombo.GetWindowText(inherit);
	CString spawnclass;
	spawnclassCombo.GetWindowText(spawnclass);

	declText = "entityDef " + declName + "\r{\r";
	declText += "\"inherit\"\t\t\t\"" + inherit + "\"\r";
	declText += "\"spawnclass\"\t\t\t\"" + spawnclass + "\"\r";
	for (int i=0; i<keyValsList.GetCount(); i++) {
		CPropertyItem* pItem = (CPropertyItem*)keyValsList.GetItemDataPtr(i);
		if (pItem) {
			// Items with a * in front are inherited and shouldn't be written out
			if (pItem->m_propName[0] == '*') {
				break;
			}
			declText += "\"" + pItem->m_propName + "\"\t\t\t\"" + pItem->m_curValue + "\"\r";
		}
	}
	declText += "}\r";

	declText.Replace( "\r", "\r\n" );
	declText.Insert( "\r\n\r\n", 0 );
	declText.StripTrailing( "\r\n" );
}

/*
================
DialogEntityDefEditor::OnBnClickedTest
================
*/
void DialogEntityDefEditor::OnBnClickedTest() {
	idStr declText, oldDeclText;

	if ( decl ) {

		BuildDeclText(declText);

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
DialogEntityDefEditor::OnBnClickedOk
================
*/
void DialogEntityDefEditor::OnBnClickedOk() {
	if ( decl ) {

		idStr declText;
		BuildDeclText(declText);

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
DialogEntityDefEditor::OnBnClickedCancel
================
*/
void DialogEntityDefEditor::OnBnClickedCancel() {
	if ( okButton.IsWindowEnabled() ) {
		if ( MessageBox( "Cancel changes?", "Cancel", MB_YESNO | MB_ICONQUESTION ) != IDYES ) {
			return;
		}
	}
	OnCancel();
}

/*
================
DialogEntityDefEditor::OnKeyValChange
================
*/
void DialogEntityDefEditor::OnKeyValChange() {
	int sel = keyValsList.GetCurSel();
	if (sel >= 0) {
		CPropertyItem *pItem = (CPropertyItem *)keyValsList.GetItemDataPtr(sel);
		keyEdit.SetWindowText(pItem->m_propName);
	}
}

/*
================
DialogEntityDefEditor::OnBnClickedAdd
================
*/
void DialogEntityDefEditor::OnBnClickedAdd() {
	CString newKey;
	keyEdit.GetWindowText(newKey);

	int matchedInherit = -1;
	int matchedKey = -1;

	// See if this key already exists
	for (int i=0; i<keyValsList.GetCount(); i++) {
		CPropertyItem* pItem = (CPropertyItem*)keyValsList.GetItemDataPtr(i);
		if (pItem) {
			// Items with a * in front are inherited and shouldn't be written out
			if (pItem->m_propName[0] == '*') {
				if (newKey = pItem->m_propName.Mid(1)) {
					matchedInherit = i;
				}
			}
			else if (pItem->m_propName == newKey) {
				matchedKey = i;
				break;
			}
		}
	}

	if (matchedKey >= 0) {
		MessageBox("Key " + newKey + " already defined");
		return;
	}

	if (matchedInherit >= 0) {
		delete keyValsList.GetItemDataPtr(matchedInherit);
		keyValsList.DeleteString(matchedInherit);
	}
	keyValsList.AddPropItem(new CPropertyItem(newKey, "", PIT_EDIT, ""));
}

/*
================
DialogEntityDefEditor::OnBnClickedDelete
================
*/
void DialogEntityDefEditor::OnBnClickedDelete() {
	CString delKey;
	keyEdit.GetWindowText(delKey);

	int matchedInherit = -1;
	int matchedKey = -1;

	// See if this key already exists
	for (int i=0; i<keyValsList.GetCount(); i++) {
		CPropertyItem* pItem = (CPropertyItem*)keyValsList.GetItemDataPtr(i);
		if (pItem) {
			// Items with a * in front are inherited and shouldn't be written out
			if (pItem->m_propName[0] == '*') {
				if (delKey = pItem->m_propName.Mid(1)) {
					matchedInherit = i;
				}
			}
			else if (pItem->m_propName == delKey) {
				matchedKey = i;
				break;
			}
		}
	}

	if (matchedKey >= 0) {
		delete keyValsList.GetItemDataPtr(matchedKey);
		keyValsList.DeleteString(matchedKey);
	}
	else if (matchedInherit) {
		MessageBox("Cannot delete an inherited value");
	}
}

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
#include "../../sys/win32/rc/ScriptEditor_resource.h"

#include "../comafx/CPathTreeCtrl.h"
#include "../script/DialogScriptEditor.h"
#include "DialogDeclBrowser.h"
#include "DialogDeclEditor.h"
#include "DialogDeclNew.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

const int DECLTYPE_SHIFT			= 24;
const int DECLINDEX_MASK			= ( 1 << DECLTYPE_SHIFT ) - 1;
const int DECLTYPE_SCRIPT			= 126;
const int DECLTYPE_GUI				= 127;

#define GetIdFromTypeAndIndex( type, index )		( ( (int)type << DECLTYPE_SHIFT ) | index )
#define GetTypeFromId( id )							( (declType_t) ( (int)id >> DECLTYPE_SHIFT ) )
#define GetIndexFromId( id )						( (int)id & DECLINDEX_MASK )

toolTip_t DialogDeclBrowser::toolTips[] = {
	{ IDC_DECLBROWSER_TREE, "decl browser" },
	{ IDC_DECLBROWSER_EDIT_SEARCH_NAMES, "search for declarations with matching name, use meta characters: *, ? and [abc...]" },
	{ IDC_DECLBROWSER_EDIT_SEARCH_TEXT, "search for declarations containing text" },
	{ IDC_DECLBROWSER_BUTTON_FIND, "find declarations matching the search strings" },
	{ IDC_DECLBROWSER_BUTTON_EDIT, "edit selected declaration" },
	{ IDC_DECLBROWSER_BUTTON_NEW, "create new declaration" },
	{ IDC_DECLBROWSER_BUTTON_RELOAD, "reload declarations" },
	{ IDOK, "ok" },
	{ IDCANCEL, "cancel" },
	{ 0, NULL }
};


static DialogDeclBrowser *g_DeclDialog = NULL;


IMPLEMENT_DYNAMIC(DialogDeclBrowser, CDialog)

/*
================
DialogDeclBrowser::DialogDeclBrowser
================
*/
DialogDeclBrowser::DialogDeclBrowser( CWnd* pParent /*=NULL*/ )
	: CDialog(DialogDeclBrowser::IDD, pParent)
	, m_pchTip(NULL)
	, m_pwchTip(NULL)
{
}

/*
================
DialogDeclBrowser::~DialogDeclBrowser
================
*/
DialogDeclBrowser::~DialogDeclBrowser() {
	delete m_pwchTip;
	delete m_pchTip;
}

/*
================
DialogDeclBrowser::DoDataExchange
================
*/
void DialogDeclBrowser::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogDeclBrowser)
	DDX_Control(pDX, IDC_DECLBROWSER_TREE, declTree);
	DDX_Control(pDX, IDC_DECLBROWSER_STATIC_SEARCH_NAMES, findNameStatic);
	DDX_Control(pDX, IDC_DECLBROWSER_STATIC_SEARCH_TEXT, findTextStatic);
	DDX_Control(pDX, IDC_DECLBROWSER_EDIT_SEARCH_NAMES, findNameEdit);
	DDX_Control(pDX, IDC_DECLBROWSER_EDIT_SEARCH_TEXT, findTextEdit);
	DDX_Control(pDX, IDC_DECLBROWSER_BUTTON_FIND, findButton);
	DDX_Control(pDX, IDC_DECLBROWSER_BUTTON_EDIT, editButton);
	DDX_Control(pDX, IDC_DECLBROWSER_BUTTON_NEW, newButton);
	DDX_Control(pDX, IDC_DECLBROWSER_BUTTON_RELOAD, reloadButton);
	DDX_Control(pDX, IDCANCEL, cancelButton);
	//}}AFX_DATA_MAP
}

/*
================
DialogDeclBrowser::AddDeclTypeToTree
================
*/
template< class type >
int idListDeclSortCompare( const type *a, const type *b ) {
	return idStr::IcmpPath( (*a)->GetName(), (*b)->GetName() );
}

void DialogDeclBrowser::AddDeclTypeToTree( declType_t type, const char *root, CPathTreeCtrl &tree ) {
	int i;
	idList<const idDecl*> decls;
	idPathTreeStack stack;
	idStr rootStr, declName;

	decls.SetNum( declManager->GetNumDecls( type ) );
	for ( i = 0; i < decls.Num(); i++ ) {
		decls[i] = declManager->DeclByIndex( type, i, false );
	}
	decls.Sort( idListDeclSortCompare );

	rootStr = root;
	rootStr += "/";

	stack.PushRoot( NULL );

	for ( i = 0; i < decls.Num(); i++) {
		declName = rootStr + decls[i]->GetName();

		declName.BackSlashesToSlashes();
		declName.Strip(' ');

		tree.AddPathToTree( declName, GetIdFromTypeAndIndex( type, decls[i]->Index() ), stack );
	}
}

/*
================
DialogDeclBrowser::AddScriptsToTree
================
*/
void DialogDeclBrowser::AddScriptsToTree( CPathTreeCtrl &tree ) {
	int i;
	idPathTreeStack stack;
	idStr scriptName;
	idFileList *files;

	files = fileSystem->ListFilesTree( "script", ".script", true );

	stack.PushRoot( NULL );

	for ( i = 0; i < files->GetNumFiles(); i++) {
		scriptName = files->GetFile( i );

		scriptName.BackSlashesToSlashes();
		scriptName.StripFileExtension();

		tree.AddPathToTree( scriptName, GetIdFromTypeAndIndex( DECLTYPE_SCRIPT, i ), stack );
	}

	fileSystem->FreeFileList( files );
}

/*
================
DialogDeclBrowser::AddGUIsToTree
================
*/
void DialogDeclBrowser::AddGUIsToTree( CPathTreeCtrl &tree ) {
	int i;
	idPathTreeStack stack;
	idStr scriptName;
	idFileList *files;

	files = fileSystem->ListFilesTree( "guis", ".gui", true );

	stack.PushRoot( NULL );

	for ( i = 0; i < files->GetNumFiles(); i++) {
		scriptName = files->GetFile( i );

		scriptName.BackSlashesToSlashes();
		scriptName.StripFileExtension();

		tree.AddPathToTree( scriptName, GetIdFromTypeAndIndex( DECLTYPE_GUI, i ), stack );
	}

	fileSystem->FreeFileList( files );
}

/*
================
DialogDeclBrowser::InitBaseDeclTree
================
*/
void DialogDeclBrowser::InitBaseDeclTree( void ) {
	int i;

	numListedDecls = 0;
	baseDeclTree.DeleteAllItems();

	for ( i = 0; i < declManager->GetNumDeclTypes(); i++ ) {
		AddDeclTypeToTree( (declType_t)i, declManager->GetDeclNameFromType( (declType_t)i ), baseDeclTree );
	}

	AddScriptsToTree( baseDeclTree );
	AddGUIsToTree( baseDeclTree );
}

/*
================
DialogDeclBrowser::GetDeclName
================
*/
void DialogDeclBrowser::GetDeclName( HTREEITEM item, idStr &typeName, idStr &declName ) const {
	HTREEITEM parent;
	idStr itemName;

	declName.Clear();
	for( parent = declTree.GetParentItem( item ); parent; parent = declTree.GetParentItem( parent ) ) {
		itemName = declTree.GetItemText( item );
		declName = itemName + "/" + declName;
		item = parent;
	}
	declName.Strip( '/' );
	typeName = declTree.GetItemText( item );
}

/*
================
DialogDeclBrowser::GetDeclFromTreeItem
================
*/
const idDecl *DialogDeclBrowser::GetDeclFromTreeItem( HTREEITEM item ) const {
	int id, index;
	declType_t type;
	const idDecl *decl;

	if ( declTree.GetChildItem( item ) ) {
		return NULL;
	}

	id = declTree.GetItemData( item );
	type = GetTypeFromId( id );
	index = GetIndexFromId( id );

	if ( type < 0 || type >= declManager->GetNumDeclTypes() ) {
		return NULL;
	}

	decl = declManager->DeclByIndex( type, index, false );

	return decl;
}

/*
================
DialogDeclBrowser::GetSelectedDecl
================
*/
const idDecl *DialogDeclBrowser::GetSelectedDecl( void ) const {
	return GetDeclFromTreeItem( declTree.GetSelectedItem() );
}

/*
================
DialogDeclBrowser::EditSelected
================
*/
void DialogDeclBrowser::EditSelected( void ) const {
	int id, index;
	idDict spawnArgs;
	const idDecl *decl;
	declType_t type;
	HTREEITEM item;

	item = declTree.GetSelectedItem();

	if ( declTree.GetChildItem( item ) ) {
		return;
	}

	id = declTree.GetItemData( item );
	type = GetTypeFromId( id );
	index = GetIndexFromId( id );

	switch( type ) {
		case DECL_AF: {
			decl = declManager->DeclByIndex( type, index, false );
			spawnArgs.Set( "articulatedFigure", decl->GetName() );
			AFEditorInit( &spawnArgs );
			break;
		}
		case DECL_PARTICLE: {
			decl = declManager->DeclByIndex( type, index, false );
			spawnArgs.Set( "model", decl->GetName() );
			ParticleEditorInit( &spawnArgs );
			break;
		}
		case DECL_PDA: {
			decl = declManager->DeclByIndex( type, index, false );
			spawnArgs.Set( "pda", decl->GetName() );
			PDAEditorInit( &spawnArgs );
			break;
		}
		case DECLTYPE_SCRIPT:
		case DECLTYPE_GUI: {
			idStr typeName, declName;
			GetDeclName( item, typeName, declName );
			DialogScriptEditor *scriptEditor;
			scriptEditor = new DialogScriptEditor;
			scriptEditor->Create( IDD_DIALOG_SCRIPTEDITOR, GetParent() );
			scriptEditor->OpenFile( typeName + "/" + declName + ( ( type == DECLTYPE_SCRIPT ) ? ".script" : ".gui" ) );
			scriptEditor->ShowWindow( SW_SHOW );
			scriptEditor->SetFocus();
			break;
		}
		default: {
			decl = declManager->DeclByIndex( type, index, false );
			DialogDeclEditor *declEditor;
			declEditor = new DialogDeclEditor;
			declEditor->Create( IDD_DIALOG_DECLEDITOR, GetParent() );
			declEditor->LoadDecl( const_cast<idDecl *>( decl ) );
			declEditor->ShowWindow( SW_SHOW );
			declEditor->SetFocus();
			break;
		}
	}
}

/*
================
DeclBrowserCompareDecl
================
*/
bool DeclBrowserCompareDecl( void *data, HTREEITEM item, const char *name ) {
	return reinterpret_cast<DialogDeclBrowser *>(data)->CompareDecl( item, name );
}

/*
================
DialogDeclBrowser::CompareDecl
================
*/
bool DialogDeclBrowser::CompareDecl( HTREEITEM item, const char *name ) const {
	if ( findNameString.Length() ) {
		if ( !idStr::Filter( findNameString, name, false ) ) {
			return false;
		}
	}

	if ( findTextString.Length() ) {
		int id, index;
		declType_t type;

		id = declTree.GetItemData( item );
		type = GetTypeFromId( id );
		index = GetIndexFromId( id );

		if ( type == DECLTYPE_SCRIPT || type == DECLTYPE_GUI ) {
			// search for the text in the script or gui
			idStr text;
			void *buffer;
			if ( fileSystem->ReadFile( idStr( name ) + ( ( type == DECLTYPE_SCRIPT ) ? ".script" : ".gui" ), &buffer ) == -1 ) {
				return false;
			}
			text = (char *) buffer;
			fileSystem->FreeFile( buffer );
			if ( text.Find( findTextString, false ) == -1 ) {
				return false;
			}
		} else {
			// search for the text in the decl
			const idDecl *decl = declManager->DeclByIndex( type, index, false );
			char *declText = (char *)_alloca( ( decl->GetTextLength() + 1 ) * sizeof( char ) );
			decl->GetText( declText );
			if ( idStr::FindText( declText, findTextString, false ) == -1 ) {
				return false;
			}
		}
	}

	return true;
}

/*
================
DialogDeclBrowser::OnInitDialog
================
*/
BOOL DialogDeclBrowser::OnInitDialog()  {

	com_editors |= EDITOR_DECL;

	CDialog::OnInitDialog();

	GetClientRect( initialRect );

	statusBar.CreateEx( SBARS_SIZEGRIP, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, initialRect, this, AFX_IDW_STATUS_BAR );

	baseDeclTree.Create( 0, initialRect, this, IDC_DECLBROWSER_BASE_TREE );

	InitBaseDeclTree();

	findNameString = "*";
	findNameEdit.SetWindowText( findNameString );

	findTextString = "";
	findTextEdit.SetWindowText( findTextString );

	numListedDecls = baseDeclTree.SearchTree( DeclBrowserCompareDecl, this, declTree );

	statusBar.SetWindowText( va( "%d decls listed", numListedDecls ) );

	EnableToolTips( TRUE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
================
DeclBrowserInit
================
*/
void DeclBrowserInit( const idDict *spawnArgs ) {

	if ( renderSystem->IsFullScreen() ) {
		common->Printf( "Cannot run the declaration editor in fullscreen mode.\n"
					"Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

	if ( g_DeclDialog == NULL ) {
		InitAfx();
		g_DeclDialog = new DialogDeclBrowser();
	}

	if ( g_DeclDialog->GetSafeHwnd() == NULL) {
		g_DeclDialog->Create( IDD_DIALOG_DECLBROWSER );
/*
		// FIXME: restore position
		CRect rct;
		g_DeclDialog->SetWindowPos( NULL, rct.left, rct.top, 0, 0, SWP_NOSIZE );
*/
	}

	idKeyInput::ClearStates();

	g_DeclDialog->ShowWindow( SW_SHOW );
	g_DeclDialog->SetFocus();

	if ( spawnArgs ) {
	}
}

/*
================
DeclBrowserRun
================
*/
void DeclBrowserRun( void ) {
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
DeclBrowserShutdown
================
*/
void DeclBrowserShutdown( void ) {
	delete g_DeclDialog;
	g_DeclDialog = NULL;
}

/*
================
DeclBrowserReloadDeclarations
================
*/
void DeclBrowserReloadDeclarations( void ) {
	if ( g_DeclDialog ) {
		g_DeclDialog->ReloadDeclarations();
	}
}

/*
================
DialogDeclBrowser::ReloadDeclarations
================
*/
void DialogDeclBrowser::ReloadDeclarations( void ) {
	InitBaseDeclTree();
	OnBnClickedFind();
}

BEGIN_MESSAGE_MAP(DialogDeclBrowser, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_SETFOCUS()
	ON_NOTIFY(TVN_SELCHANGED, IDC_DECLBROWSER_TREE, OnTreeSelChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_DECLBROWSER_TREE, OnTreeDblclk)
	ON_BN_CLICKED(IDC_DECLBROWSER_BUTTON_FIND, OnBnClickedFind)
	ON_BN_CLICKED(IDC_DECLBROWSER_BUTTON_EDIT, OnBnClickedEdit)
	ON_BN_CLICKED(IDC_DECLBROWSER_BUTTON_NEW, OnBnClickedNew)
	ON_BN_CLICKED(IDC_DECLBROWSER_BUTTON_RELOAD, OnBnClickedReload)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()

// DialogDeclBrowser message handlers

/*
================
DialogDeclBrowser::OnActivate
================
*/
void DialogDeclBrowser::OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized ) {
	CDialog::OnActivate( nState, pWndOther, bMinimized );
}

/*
================
DialogDeclBrowser::OnToolTipNotify
================
*/
BOOL DialogDeclBrowser::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

	if ( pNMHDR->hwndFrom == declTree.GetSafeHwnd() ) {
		CString toolTip;
		const idDecl *decl = GetDeclFromTreeItem( (HTREEITEM) pNMHDR->idFrom );

		if ( !decl ) {
			return FALSE;
		}

		toolTip = va( "%s, line: %d", decl->GetFileName(), decl->GetLineNum() );

#ifndef _UNICODE
		if( pNMHDR->code == TTN_NEEDTEXTA ) {
			delete m_pchTip;
			m_pchTip = new TCHAR[toolTip.GetLength() + 2];
			lstrcpyn( m_pchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTW->lpszText = (WCHAR*)m_pchTip;
		} else {
			delete m_pwchTip;
			m_pwchTip = new WCHAR[toolTip.GetLength() + 2];
			_mbstowcsz( m_pwchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTW->lpszText = (WCHAR*)m_pwchTip;
		}
#else
		if( pNMHDR->code == TTN_NEEDTEXTA ) {
			delete m_pchTip;
			m_pchTip = new TCHAR[toolTip.GetLength() + 2];
			_wcstombsz( m_pchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTA->lpszText = (LPTSTR)m_pchTip;
		} else {
			delete m_pwchTip;
			m_pwchTip = new WCHAR[toolTip.GetLength() + 2];
			lstrcpyn( m_pwchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTA->lpszText = (LPTSTR) m_pwchTip;
		}
#endif
		return TRUE;
	}

	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

/*
================
DialogDeclBrowser::OnSetFocus
================
*/
void DialogDeclBrowser::OnSetFocus( CWnd *pOldWnd ) {
	CDialog::OnSetFocus( pOldWnd );
}

/*
================
DialogDeclBrowser::OnDestroy
================
*/
void DialogDeclBrowser::OnDestroy() {

	com_editors &= ~EDITOR_DECL;

	return CDialog::OnDestroy();
}

/*
================
DialogDeclBrowser::OnMove
================
*/
void DialogDeclBrowser::OnMove( int x, int y ) {
	if ( GetSafeHwnd() ) {
		CRect rct;
		GetWindowRect( rct );
		// FIXME: save position
	}
	CDialog::OnMove( x, y );
}

/*
================
DialogDeclBrowser::OnMove
================
*/
#define BORDER_SIZE			4
#define BUTTON_SPACE		4
#define TOOLBAR_HEIGHT		24

void DialogDeclBrowser::OnSize( UINT nType, int cx, int cy ) {
	CRect clientRect, rect;

	LockWindowUpdate();

	CDialog::OnSize( nType, cx, cy );

	GetClientRect( clientRect );

	if ( declTree.GetSafeHwnd() ) {
		rect.left = BORDER_SIZE;
		rect.top = BORDER_SIZE;
		rect.right = clientRect.Width() - BORDER_SIZE;
		rect.bottom = clientRect.Height() - 100;
		declTree.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( findNameStatic.GetSafeHwnd() ) {
		rect.left = BORDER_SIZE + 2;
		rect.top = clientRect.Height() - 100 + BUTTON_SPACE + 2;
		rect.right = BORDER_SIZE + 80;
		rect.bottom = clientRect.Height() - 76 + 2;
		findNameStatic.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( findTextStatic.GetSafeHwnd() ) {
		rect.left = BORDER_SIZE + 2;
		rect.top = clientRect.Height() - 78 + BUTTON_SPACE + 2;
		rect.right = BORDER_SIZE + 80;
		rect.bottom = clientRect.Height() - 54 + 2;
		findTextStatic.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( findNameEdit.GetSafeHwnd() ) {
		rect.left = BORDER_SIZE + 80;
		rect.top = clientRect.Height() - 100 + BUTTON_SPACE;
		rect.right = clientRect.Width() - BORDER_SIZE;
		rect.bottom = clientRect.Height() - 76;
		findNameEdit.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( findTextEdit.GetSafeHwnd() ) {
		rect.left = BORDER_SIZE + 80;
		rect.top = clientRect.Height() - 78 + BUTTON_SPACE;
		rect.right = clientRect.Width() - BORDER_SIZE;
		rect.bottom = clientRect.Height() - 54;
		findTextEdit.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( findButton.GetSafeHwnd() ) {
		findButton.GetClientRect( rect );
		int width = rect.Width();
		int height = rect.Height();
		rect.left = BORDER_SIZE;
		rect.top = clientRect.Height() - TOOLBAR_HEIGHT - height;
		rect.right = BORDER_SIZE + width;
		rect.bottom = clientRect.Height() - TOOLBAR_HEIGHT;
		findButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( editButton.GetSafeHwnd() ) {
		editButton.GetClientRect( rect );
		int width = rect.Width();
		int height = rect.Height();
		rect.left = BORDER_SIZE + BUTTON_SPACE + width;
		rect.top = clientRect.Height() - TOOLBAR_HEIGHT - height;
		rect.right = BORDER_SIZE + BUTTON_SPACE + 2 * width;
		rect.bottom = clientRect.Height() - TOOLBAR_HEIGHT;
		editButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( newButton.GetSafeHwnd() ) {
		newButton.GetClientRect( rect );
		int width = rect.Width();
		int height = rect.Height();
		rect.left = BORDER_SIZE + 2 * BUTTON_SPACE + 2 * width;
		rect.top = clientRect.Height() - TOOLBAR_HEIGHT - height;
		rect.right = BORDER_SIZE + 2 * BUTTON_SPACE + 3 * width;
		rect.bottom = clientRect.Height() - TOOLBAR_HEIGHT;
		newButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
	}

	if ( reloadButton.GetSafeHwnd() ) {
		reloadButton.GetClientRect( rect );
		int width = rect.Width();
		int height = rect.Height();
		rect.left = BORDER_SIZE + 3 * BUTTON_SPACE + 3 * width;
		rect.top = clientRect.Height() - TOOLBAR_HEIGHT - height;
		rect.right = BORDER_SIZE + 3 * BUTTON_SPACE + 4 * width;
		rect.bottom = clientRect.Height() - TOOLBAR_HEIGHT;
		reloadButton.MoveWindow( rect.left, rect.top, rect.Width(), rect.Height() );
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
DialogDeclBrowser::OnSizing
================
*/
void DialogDeclBrowser::OnSizing( UINT nSide, LPRECT lpRect ) {
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
DialogDeclBrowser::OnTreeSelChanged
================
*/
void DialogDeclBrowser::OnTreeSelChanged( NMHDR* pNMHDR, LRESULT* pResult ) {
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

	const idDecl *decl = GetSelectedDecl();
	if ( decl ) {
		statusBar.SetWindowText( va( "%d decls listed    -    %s, line: %d", numListedDecls, decl->GetFileName(), decl->GetLineNum() ) );
		findNameEdit.SetWindowText( va( "%s/%s", declManager->GetDeclNameFromType( decl->GetType() ), decl->GetName() ) );
	} else {
		HTREEITEM item = declTree.GetSelectedItem();
		idStr typeName, declName;
		GetDeclName( item, typeName, declName );
		findNameEdit.SetWindowText( va( "%s/%s*", typeName.c_str(), declName.c_str() ) );
		statusBar.SetWindowText( va( "%d decls listed", numListedDecls ) );
	}

	*pResult = 0;
}

/*
================
DialogDeclBrowser::OnTreeDblclk
================
*/
void DialogDeclBrowser::OnTreeDblclk( NMHDR *pNMHDR, LRESULT *pResult ) {
	// post a message as if the edit button was clicked to make sure the editor gets focus
	PostMessage( WM_COMMAND, ( BN_CLICKED << 16 ) | editButton.GetDlgCtrlID(), 0 );

	*pResult = 1;
}

/*
================
DialogDeclBrowser::OnBnClickedFind
================
*/
void DialogDeclBrowser::OnBnClickedFind() {
	CString windowText;

	findNameEdit.GetWindowText( windowText );
	findNameString = windowText;
	findNameString.Strip( ' ' );

	findTextEdit.GetWindowText( windowText );
	findTextString = windowText;
	findTextString.Strip( ' ' );
	
	numListedDecls = baseDeclTree.SearchTree( DeclBrowserCompareDecl, this, declTree );

	statusBar.SetWindowText( va( "%d decls listed", numListedDecls ) );
}

/*
================
DialogDeclBrowser::OnBnClickedEdit
================
*/
void DialogDeclBrowser::OnBnClickedEdit() {
	EditSelected();
}

/*
================
DialogDeclBrowser::OnBnClickedNew
================
*/
void DialogDeclBrowser::OnBnClickedNew() {
	HTREEITEM item;
	idStr typeName, declName;
	const idDecl *decl;
	DialogDeclNew newDeclDlg;

	newDeclDlg.SetDeclTree( &baseDeclTree );

	item = declTree.GetSelectedItem();
	if ( item ) {
		GetDeclName( item, typeName, declName );
		newDeclDlg.SetDefaultType( typeName );
		newDeclDlg.SetDefaultName( declName );
	}

	decl = GetSelectedDecl();
	if ( decl ) {
		newDeclDlg.SetDefaultFile( decl->GetFileName() );
	}

	if ( newDeclDlg.DoModal() != IDOK ) {
		return;
	}

	decl = newDeclDlg.GetNewDecl();

	if ( decl ) {
		declName = declManager->GetDeclNameFromType( decl->GetType() );
		declName += "/";
		declName += decl->GetName();

		int id = GetIdFromTypeAndIndex( decl->GetType(), decl->Index() );

		baseDeclTree.InsertPathIntoTree( declName, id );
		item = declTree.InsertPathIntoTree( declName, id );
		declTree.SelectItem( item );

		EditSelected();
	}
}

/*
================
DialogDeclBrowser::OnBnClickedReload
================
*/
void DialogDeclBrowser::OnBnClickedReload() {

	declManager->Reload( false );

	ReloadDeclarations();
}

/*
================
DialogDeclBrowser::OnBnClickedOk
================
*/
void DialogDeclBrowser::OnBnClickedOk() {
	// with a modeless dialog once it is closed and re-activated windows seems
	// to enjoy mapping ENTER back to the default button ( OK ) even if you have
	// it NOT set as the default.. in this case use cancel button exit and ignore
	// default IDOK handling.
	// OnOK();
}

/*
================
DialogDeclBrowser::OnBnClickedCancel
================
*/
void DialogDeclBrowser::OnBnClickedCancel() {
	OnCancel();
}

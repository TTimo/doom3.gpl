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

#include "qe3.h"
#include "Radiant.h"
#include "WaitDlg.h"
#include "PreviewDlg.h"
#include "CommentsDlg.h"

const int PARENTID = 99999;

extern HTREEITEM FindTreeItem(CTreeCtrl *tree, HTREEITEM root, const char *text, HTREEITEM forceParent);

// CPreviewDlg dialog

IMPLEMENT_DYNAMIC(CPreviewDlg, CDialog)
CPreviewDlg::CPreviewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPreviewDlg::IDD, pParent)
{
	currentMode = MODELS;
	disablePreview = false;
}

CPreviewDlg::~CPreviewDlg()
{
}

void CPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_MEDIA, treeMedia);
	DDX_Control(pDX, IDC_EDIT_INFO, editInfo);
	DDX_Control(pDX, IDC_PREVIEW, wndPreview);
}


BEGIN_MESSAGE_MAP(CPreviewDlg, CDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_MEDIA, OnTvnSelchangedTreeMedia)
	ON_BN_CLICKED(IDC_BUTTON_RELOAD, OnBnClickedButtonReload)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnBnClickedButtonPlay)
END_MESSAGE_MAP()


// CPreviewDlg message handlers

BOOL CPreviewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_image.Create(IDB_BITMAP_MATERIAL, 16, 1, RGB(255, 255, 255));
	treeMedia.SetImageList(&m_image, TVSIL_NORMAL);
	if ( disablePreview ) {
		wndPreview.ShowWindow( SW_HIDE );
	} else {
		wndPreview.setDrawable(&m_testDrawable);
	}
	
	SetMode(currentMode);
	BuildTree();

	if ( mediaName.Length() ) {
		HTREEITEM root = treeMedia.GetRootItem();
		HTREEITEM sel = FindTreeItem(&treeMedia, root, mediaName, NULL );
		if (sel) {
			treeMedia.SelectItem(sel);
		}
	}
	mediaName = "";
	return TRUE;  // return TRUE unless you set the focus to a control

}

void CPreviewDlg::BuildTree() {

	CWaitCursor cursor;
	quickTree.Clear();
	treeMedia.DeleteAllItems();

	idFileList *files;
	
	if ( currentMode == GUIS ) {
		files = fileSystem->ListFilesTree( "guis", ".gui" );
		AddStrList( "base", files->GetList(), GUIS );
		fileSystem->FreeFileList( files );
	} else if ( currentMode == MODELS ) {
		files = fileSystem->ListFilesTree( "models", ".lwo" );
		AddStrList( "base", files->GetList(), MODELS );
		fileSystem->FreeFileList( files );
		files = fileSystem->ListFilesTree( "models", ".ase" );
		AddStrList( "base", files->GetList(), MODELS );
		fileSystem->FreeFileList( files );
		files = fileSystem->ListFilesTree( "models", ".ma" );
		AddStrList( "base", files->GetList(), MODELS );
		fileSystem->FreeFileList( files );
	} else if ( currentMode == SOUNDS ) {
		AddSounds( true );
	} else if ( currentMode == MATERIALS ) {
		AddMaterials( true );
	} else if ( currentMode == PARTICLES ) {
		AddParticles( true );
	} else if ( currentMode == SKINS ) {
		AddSkins( true );
	}
}

void CPreviewDlg::RebuildTree( const char *_data ) {
	data = _data;
	data.ToLower();
	BuildTree();
}

void CPreviewDlg::AddCommentedItems() {
	const char *buffer = NULL;
	const char *path;
	items.Clear();
	path = (currentMode == GUIS) ? "guis/guis.commented" : "models/models.commented";
	idParser src( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT );
	if (fileSystem->ReadFile(path, (void**)&buffer, NULL) && buffer) {
		src.LoadMemory(buffer, strlen(buffer), path);
		if (src.IsLoaded()) {
			idToken token, tok1, tok2, tok3;
			while( src.ReadToken( &token ) ) {
				if (token == "{") {
					// start a new commented item
					CommentedItem ci;
					if (src.ReadToken(&tok1) && src.ReadToken(&tok2) && src.ReadToken(&tok3)) {
						ci.Name = tok1;
						ci.Path = tok2;
						ci.Comments = tok3;
						items.Append(ci);
					}
				}
			}
		}
		fileSystem->FreeFile((void*)buffer);
	}
	commentItem = treeMedia.InsertItem("Commented");
	int c = items.Num();
	if (c) {
		for (int i = 0; i < c; i++) {
			HTREEITEM child = treeMedia.InsertItem(items[i].Name, commentItem);
			treeMedia.SetItemData(child, -1 - i);
			treeMedia.SetItemImage(child, 2, 2);
		}
	}
}
	


void CPreviewDlg::AddStrList( const char *root, const idStrList &list, int id ) {
	idStr		out, path;
	HTREEITEM	base = treeMedia.GetRootItem();
	if (base) {
		out = treeMedia.GetItemText(base);
		if (stricmp(root, out)) {
			base = NULL;
		}
	}

	if (base == NULL) {
		base = treeMedia.InsertItem(root);
		treeMedia.SetItemData(base, PARENTID);
	}

	HTREEITEM	item = base;
	HTREEITEM	add;
	
	int		count = list.Num();

	idStr	last, qt;
	for (int i = 0; i < count; i++) {
		idStr name = list[i];

		// now break the name down convert to slashes
		name.BackSlashesToSlashes();
		name.Strip(' ');

		int index;
		int len = last.Length();
		if (len == 0) {
			index = name.Last('/');
			if (index >= 0) {
				name.Left(index, last);
			}
		}
		else if (idStr::Icmpn(last, name, len) == 0 && name.Last('/') <= len) {
			name.Right(name.Length() - len - 1, out);
			add = treeMedia.InsertItem(out, item);
			qt = root;
			qt += "/";
			qt += name;
			quickTree.Set(qt, add);
			treeMedia.SetItemImage(add, 2, 2);
			treeMedia.SetItemData(add, id);
			continue;
		}
		else {
			last.Empty();
		}

		index = 0;
		item = base;
		path = "";
		while (index >= 0) {
			index = name.Find('/');
			if (index >= 0) {
				HTREEITEM newItem = NULL;
				HTREEITEM *check = NULL;
				name.Left( index, out );
				path += out;
				qt = root;
				qt += "/";
				qt += path;
				if (quickTree.Get(qt, &check)) {
					newItem = *check;
				}
				//HTREEITEM	newItem = FindTreeItem(&treeMedia, item, name.Left(index, out), item);
				if (newItem == NULL) {
					newItem = treeMedia.InsertItem(out, item);
					qt = root;
					qt += "/";
					qt += path;
					quickTree.Set(qt, newItem);
					treeMedia.SetItemImage(newItem, 0, 1);
					treeMedia.SetItemData(newItem, PARENTID);
				}

				assert(newItem);
				item = newItem;
				name.Right(name.Length() - index - 1, out);
				name = out;
				path += "/";
			}
			else {
				add = treeMedia.InsertItem(name, item);
				qt = root;
				qt += "/";
				qt += path;
				qt += name;
				quickTree.Set(qt, add);
				treeMedia.SetItemImage(add, 2, 2);
				treeMedia.SetItemData(add, id);
				path = "";
			}
		}
	}

}

void CPreviewDlg::OnTvnSelchangedTreeMedia(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM item = treeMedia.GetSelectedItem();
	mediaName = "";
	CWnd *add = GetDlgItem(IDC_BUTTON_ADD);
	if (add) {
		add->EnableWindow(treeMedia.GetItemData(item) == GUIS || treeMedia.GetItemData(item) == MODELS);
	}
	if (item) {

		editInfo.SetWindowText("No comments for this item");
		int id = treeMedia.GetItemData(item);
		if ( id == GUIS || id == MODELS || id == MATERIALS || id == WAVES || id == PARTICLES || id == SKINS ) {
			mediaName = treeMedia.GetItemText( item );

			// have to build the name back up
			HTREEITEM parent = treeMedia.GetParentItem( item );
			while ( parent != NULL ) {
				idStr strParent = treeMedia.GetItemText( parent );
				strParent += "/";
				strParent += mediaName;
				mediaName = strParent;
				parent = treeMedia.GetParentItem( parent );
			}
			// strip the leading "base/"
			if (id == MATERIALS) {
				mediaName.Strip("Materials/");
			} else if (id == WAVES) {
				mediaName.Strip( "Wave files/" );
			} else if (id == PARTICLES) {
				mediaName.Strip("Particles/");
				mediaName += ".prt";
			} else if ( id == SKINS ) {
				mediaName.Strip( "Matching Skins/" );
				mediaName.Strip( "Skins/" );
			} else {
				mediaName.Strip( "base/" );
			}

		} else if (id == WAVES || id == SOUNDS) {
			mediaName = treeMedia.GetItemText( item );
		} else if (id < 0) {
			if ( treeMedia.ItemHasChildren(item) == FALSE ) {
				int dw = abs(( int )treeMedia.GetItemData( item )) - 1;
				if ( dw < items.Num() ) {
					idStr work = items[dw].Path;
					work += "\r\n\r\n";
					work += items[dw].Comments;
					editInfo.SetWindowText( work );
					mediaName = items[dw].Path;
				}
			}
		}

		if ( currentMode == MODELS || currentMode == SKINS ) {
			idStr modelMedia;
			if ( currentMode == MODELS ) {
				modelMedia = mediaName;
			} else {
				modelMedia = data;
			}
			if ( modelMedia.Length() ) {
				int size = fileSystem->ReadFile( modelMedia, NULL, NULL );
				int lsize;
				if ( strstr( modelMedia, ".lwo" ) ) {
					lsize = 128 * 1024;
				}
				else {
					lsize = 768 * 1024;
				}
				if ( size > lsize ) {
					if ( MessageBox("Model appears to be quite large, are you sure you want to preview it?", "High Poly Model?", MB_YESNO ) == IDNO ) {
						*pResult = 0;
						return;
					}
				}
				m_drawModel.setMedia( modelMedia );
				if ( currentMode == SKINS ) {
					m_drawModel.SetSkin( mediaName );
				}
			}
			m_drawModel.SetRealTime(0);
			wndPreview.setDrawable( &m_drawModel );
			wndPreview.Invalidate();
			wndPreview.RedrawWindow();
			RedrawWindow();
		}
		else if ( currentMode == PARTICLES ) {
			m_drawModel.setMedia( mediaName );
			m_drawModel.SetRealTime(50);
			wndPreview.setDrawable( &m_drawModel );
			wndPreview.Invalidate();
			wndPreview.RedrawWindow();
			RedrawWindow();
		} else if ( currentMode == GUIS ) {
			const idMaterial *mat = declManager->FindMaterial("guisurfs/guipreview");
			mat->SetGui(mediaName);
			m_drawMaterial.setMedia("guisurfs/guipreview");
			m_drawMaterial.setScale(4.4f);
			wndPreview.setDrawable(&m_drawMaterial);
			wndPreview.Invalidate();
			wndPreview.RedrawWindow();
			idUserInterface *gui = uiManager->FindGui( mediaName, false, false, true );
			if ( gui ) {
				idStr str = gui->Comment();
				str.Replace( "\n", "\r\n" );
				if ( str != "" ) {
					editInfo.SetWindowText( str );
				}
			}
			RedrawWindow();
		} else if (currentMode == MATERIALS) {
			m_drawMaterial.setMedia(mediaName);
			m_drawMaterial.setScale(1.0);
			wndPreview.setDrawable(&m_drawMaterial);
			wndPreview.Invalidate();
			wndPreview.RedrawWindow();
			RedrawWindow();
		}

		//m_drawGui.setMedia(matName);
		//wndPreview.setDrawable(&m_drawMaterial);
		//wndPreview.RedrawWindow();
	}

	*pResult = 0;
}


BOOL CPreviewDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	BOOL b =  CDialog::Create(lpszTemplateName, pParentWnd);
	ShowWindow(SW_SHOW);
	return b;
}

void CPreviewDlg::OnCancel()
{
	if ( AfxGetApp()->GetMainWnd() == GetParent() && GetParent() ) {
		GetParent()->EnableWindow(TRUE);
		g_qeglobals.sw->StopAllSounds();
		ShowWindow(SW_HIDE);
	} else {
		CDialog::OnCancel();
	}
	returnCode = IDCANCEL;
}

void CPreviewDlg::OnOK()
{
	if ( AfxGetApp()->GetMainWnd() == GetParent() && GetParent() ) {
		GetParent()->EnableWindow(TRUE);
		g_qeglobals.sw->StopAllSounds();
		ShowWindow(SW_HIDE);
	} else {
		CDialog::OnOK();
	}
	returnCode = IDOK;
}

bool CPreviewDlg::Waiting() {
	AfxGetApp()->PumpMessage();
	return (returnCode == -1);
}

void CPreviewDlg::SetModal() {
	returnCode = -1;
}
void CPreviewDlg::OnBnClickedButtonReload()
{
	BuildTree();
	g_qeglobals.sw->StopAllSounds();
}

void CPreviewDlg::OnBnClickedButtonAdd()
{
	HTREEITEM item = treeMedia.GetSelectedItem();
	if (treeMedia.ItemHasChildren(item) == FALSE && (treeMedia.GetItemData(item) == GUIS || treeMedia.GetItemData(item) == MODELS)) {
		CCommentsDlg dlg;
		dlg.strPath = mediaName;
		if (dlg.DoModal()) {
			CommentedItem ci;
			ci.Name = dlg.strName;
			ci.Path = dlg.strPath;
			ci.Comments = dlg.strComments;
			items.Append(ci);
			item = treeMedia.InsertItem(ci.Name, commentItem);
			treeMedia.SetItemData(item, -1 - (items.Num() + 1));
			treeMedia.SetItemImage(item, 2, 2);
			const char *path;
			path = (currentMode == GUIS) ? "guis/guis.commented" : "models/models.commented";
			idStr str;
			void *buffer;
			fileSystem->ReadFile( path, &buffer );
			str = (char *) buffer;
			fileSystem->FreeFile( buffer );
			str += "\r\n\r\n{\r\n\t\"";
			str += ci.Name;
			str += "\"\r\n\t\"";
			str += ci.Path;
			str += "\"\r\n\t\"";
			str += ci.Comments;
			str += "\"\r\n}\r\n";
			fileSystem->WriteFile(path, (void*)&str[0], str.Length(), "fs_devpath");

		}
	}
}


void CPreviewDlg::AddSounds(bool rootItems) {
	int i, j;
	idStrList list(1024);
	idStrList list2(1024);
	HTREEITEM base = treeMedia.InsertItem("Sound Shaders");
	 
	for( i = 0; i < declManager->GetNumDecls( DECL_SOUND ); i++ ) {
		const idSoundShader *poo = declManager->SoundByIndex( i, false );
		list.AddUnique( poo->GetFileName() );
	}
	list.Sort();
	
	for ( i = 0; i < list.Num(); i++ ) {
		HTREEITEM child = treeMedia.InsertItem(list[i], base);
		treeMedia.SetItemData(child, SOUNDPARENT);
		treeMedia.SetItemImage(child, 0, 1);
		list2.Clear();
		for (j = 0; j < declManager->GetNumDecls( DECL_SOUND ); j++) {
			const idSoundShader *poo = declManager->SoundByIndex( j, false );
			if ( idStr::Icmp( list[i], poo->GetFileName() ) == 0 ) {
				list2.Append( poo->GetName() );
			}
		}
		list2.Sort();		
		for (j = 0; j < list2.Num(); j++) {
			HTREEITEM child2 = treeMedia.InsertItem( list2[j], child );
			treeMedia.SetItemData(child2, SOUNDS);
			treeMedia.SetItemImage(child2, 2, 2);
		}
	}

	idFileList *files;
	files = fileSystem->ListFilesTree( "sound", ".wav" );
    AddStrList( "Wave files", files->GetList(), WAVES );
	fileSystem->FreeFileList( files );
}

void CPreviewDlg::SetMode( int mode, const char *preSelect ) {
	
	currentMode = mode;
	if ( preSelect ) {
		mediaName = preSelect;
	}

	if (GetSafeHwnd() == NULL) {
		return;
	}

	CWnd *wnd;
	switch (currentMode) {
		case GUIS :
		case SKINS :
		case MODELS :
		case PARTICLES :
				wndPreview.ShowWindow(SW_SHOW);
				wnd = GetDlgItem(IDC_BUTTON_PLAY);
				if (wnd) {
					wnd->ShowWindow(SW_HIDE);
				}
				wnd = GetDlgItem(IDC_BUTTON_ADD);
				if (wnd) {
					wnd->ShowWindow(SW_SHOW);
				}
				wnd = GetDlgItem(IDC_EDIT_INFO);
				if (wnd) {
					wnd->ShowWindow(SW_SHOW);
				}
			break;
		case MATERIALS :
				wndPreview.ShowWindow(SW_SHOW);
				wnd = GetDlgItem(IDC_BUTTON_PLAY);
				if (wnd) {
					wnd->ShowWindow(SW_HIDE);
				}
				wnd = GetDlgItem(IDC_BUTTON_ADD);
				if (wnd) {
					wnd->ShowWindow(SW_HIDE);
				}
				wnd = GetDlgItem(IDC_EDIT_INFO);
				if (wnd) {
					wnd->ShowWindow(SW_HIDE);
				}
				break;
		case SOUNDS :
		case WAVES :
				wndPreview.ShowWindow(SW_HIDE);
				wnd = GetDlgItem(IDC_BUTTON_PLAY);
				if (wnd) {
					wnd->ShowWindow(SW_SHOW);
				}
				wnd = GetDlgItem(IDC_BUTTON_ADD);
				if (wnd) {
					wnd->ShowWindow(SW_HIDE);
				}
				wnd = GetDlgItem(IDC_EDIT_INFO);
				if (wnd) {
					wnd->ShowWindow(SW_HIDE);
				}
			break;
	}
}

void CPreviewDlg::OnBnClickedButtonPlay() {
	g_qeglobals.sw->PlayShaderDirectly(mediaName);
}

void CPreviewDlg::AddMaterials(bool rootItems) {
	idStrList list(1024);
	//char temp[2048];
	int count = declManager->GetNumDecls( DECL_MATERIAL );
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			const idMaterial *mat = declManager->MaterialByIndex(i, false);
			if (!rootItems) {
				if (strchr(mat->GetName(), '/') == NULL && strchr(mat->GetName(), '\\') == NULL) {
					continue;
				}
			}
			list.Append(mat->GetName());
		}
		list.Sort();
		AddStrList("Materials", list, MATERIALS);
	}

}

void CPreviewDlg::AddParticles(bool rootItems) {
	idStrList list(1024);
	int count = declManager->GetNumDecls( DECL_PARTICLE );
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			const idDecl *ips = declManager->DeclByIndex( DECL_PARTICLE, i );
			if (!rootItems) {
				if (strchr(ips->GetName(), '/') == NULL && strchr(ips->GetName(), '\\') == NULL) {
					continue;
				}
			}
			list.Append(ips->GetName());
		}
		list.Sort();
		AddStrList("Particles", list, PARTICLES);
	}
}

void CPreviewDlg::AddSkins( bool rootItems ) {
	idStrList list(1024);
	idStrList list2(1024);
	idStr str;
	int count = declManager->GetNumDecls( DECL_SKIN );
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			const idDeclSkin *skin = declManager->SkinByIndex(i);
			if (!rootItems) {
				if (strchr(skin->GetName(), '/') == NULL && strchr(skin->GetName(), '\\') == NULL) {
					continue;
				}
			}
			if ( data.Length() ) {
				for ( int j = 0; j < skin->GetNumModelAssociations(); j++ ){
					str = skin->GetAssociatedModel( j );
					str.ToLower();
					if ( data.Cmp(str) == 0 ) {
						list.Append(skin->GetName());
					}
				}
			}
			list2.Append(skin->GetName());
		}
		list.Sort();
		list2.Sort();
		AddStrList( "Matching Skins", list, SKINS );
		AddStrList( "Skins", list2, SKINS );
	}
}

void CPreviewDlg::OnShowWindow( BOOL bShow, UINT status ) {
	if ( bShow && AfxGetApp()->GetMainWnd() == GetParent() && GetParent() ) {
		GetParent()->EnableWindow( FALSE );
	}		
}

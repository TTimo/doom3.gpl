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

#include "MaterialTreeView.h"

#define IMAGE_FOLDER				0
#define IMAGE_FILE					1
#define IMAGE_MATERIAL				2
#define IMAGE_MATERIAL_FOLDER		3
#define IMAGE_FILE_MOD				4
#define IMAGE_MATERIAL_MOD			5
#define IMAGE_MATERIAL_MOD_APPLY	6

#define HOVER_EXPAND_DELAY 500

#define MSG_RENAME_FOLDER_COMPLETE (WM_USER + 1000)
#define MSG_RENAME_MATERIAL_COMPLETE (WM_USER + 1001)

IMPLEMENT_DYNCREATE(MaterialTreeView, CTreeView)

BEGIN_MESSAGE_MAP(MaterialTreeView, CTreeView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED,		OnTvnSelchanged)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT,	OnTvnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT,		OnTvnEndlabeledit)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_RCLICK,			OnNMRclick)
	ON_WM_CHAR()
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG,		OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()

	ON_COMMAND(ID_POPUP_APPLYMATERIAL,		OnApplyMaterial)
	ON_COMMAND(ID_POPUP_APPLYFILE,			OnApplyFile)
	ON_COMMAND(ID_POPUP_APPLYALL,			OnApplyAll)
	ON_COMMAND(ID_POPUP_SAVEMATERIAL,		OnSaveMaterial)
	ON_COMMAND(ID_POPUP_SAVEFILE,			OnSaveFile)
	ON_COMMAND(ID_POPUP_SAVEALL,			OnSaveAll)
	ON_COMMAND(ID_POPUP_RENAMEMATERIAL,		OnRenameMaterial)
	ON_COMMAND(ID_POPUP_ADDMATERIAL,		OnAddMaterial)
	ON_COMMAND(ID_POPUP_ADDFOLDER,			OnAddFolder)
	ON_COMMAND(ID_POPUP_DELETEMATERIAL,		OnDeleteMaterial)
	ON_COMMAND(ID_POPUP_RELOADFILE,			OnReloadFile)

	ON_COMMAND(ID_POPUP_CUT,				OnCut)
	ON_COMMAND(ID_POPUP_COPY,				OnCopy)
	ON_COMMAND(ID_POPUP_PASTE,				OnPaste)

	ON_MESSAGE(MSG_RENAME_FOLDER_COMPLETE, OnRenameFolderComplete)
	ON_MESSAGE(MSG_RENAME_MATERIAL_COMPLETE, OnRenameMaterialComplete)	
END_MESSAGE_MAP()

/** 
* Constructor for MaterialTreeView
*/
MaterialTreeView::MaterialTreeView() {
	treeWithFile = false;
	bDragging = false;
	hoverItem = NULL;
	internalChange = false;
}

/** 
* Destructor for MaterialTreeView
*/
MaterialTreeView::~MaterialTreeView() {
}

/** 
* Clears the tree and rebuilds it.
* @param includeFile Should the list include the filename
* @param filename The file to load or NULL to load all files.
*/
void MaterialTreeView::InitializeMaterialList(bool includeFile, const char* filename) {

	treeWithFile = includeFile;

	CTreeCtrl& tree = GetTreeCtrl();

	tree.DeleteAllItems();
	quickTree.Clear();
	materialToTree.Clear();
	fileToTree.Clear();

	BuildMaterialList(includeFile, filename);
}

/** 
* Builds the tree of materials.
* @param includeFile Should the list include the filename
* @param filename The file to load or NULL to load all files.
*/
void MaterialTreeView::BuildMaterialList(bool includeFile, const char* filename) {

	CTreeCtrl& tree = GetTreeCtrl();

	idStrList list(1024);

	int count = declManager->GetNumDecls( DECL_MATERIAL );
	if (count > 0) {
		for (int i = 0; i < count; i++) {
			const idMaterial	*mat = declManager->MaterialByIndex(i, false);

			if(filename && strcmp(filename, mat->GetFileName())) {
				continue;
			}

			idStr temp;

			//Do Not Include Implicit File Definitions
			idStr filename = mat->GetFileName();
			if(!filename.Icmp("<implicit file>")) {
				continue;
			}

			if(filename.Find("def") != -1) {
				int x = 0;
			}

			if(includeFile) {
				filename.StripPath();
				temp = idStr(mat->GetFileName()) + "/" + idStr(mat->GetName()) + "|" + filename;
			} else {
				temp = mat->GetName();
			}

			list.Append(temp);
		}
		AddStrList(NULL, &list, includeFile);
	}
}

/**
* Called when the material has changed but not applied.
* @param pMaterial The selected material.
*/
void MaterialTreeView::MV_OnMaterialChange(MaterialDoc* pMaterial) {

	CTreeCtrl& tree = GetTreeCtrl();

	//When a material changes place an asterik next to the material and the file
	HTREEITEM* materialItem = NULL;
	materialToTree.Get(pMaterial->name, &materialItem);


	if(!materialItem)
		return;

	tree.SetItemImage(*materialItem, IMAGE_MATERIAL_MOD_APPLY, IMAGE_MATERIAL_MOD_APPLY);


	if(treeWithFile) {
		HTREEITEM* fileItem = NULL;
		idStr file = pMaterial->renderMaterial->GetFileName();

		//common->Printf("Filename = %s\n", file.c_str());

		if(fileToTree.Get(file, &fileItem)){
			//common->Printf("Found: %d\n", *fileItem);
			tree.SetItemImage(*fileItem, IMAGE_FILE_MOD, IMAGE_FILE_MOD);
		}
	}
}

/**
* Called when the material changes have been applied. 
* @param pMaterial The selected material.
*/
void MaterialTreeView::MV_OnMaterialApply(MaterialDoc* pMaterial) {
	CTreeCtrl& tree = GetTreeCtrl();

	//When a material is applied then just change the image to material modified
	HTREEITEM* materialItem = NULL;
	materialToTree.Get(pMaterial->name, &materialItem);

	if(!materialItem)
		return;

	tree.SetItemImage(*materialItem, IMAGE_MATERIAL_MOD, IMAGE_MATERIAL_MOD);
}

/**
* Called when the material changes have been saved. 
* @param pMaterial The saved material.
*/
void MaterialTreeView::MV_OnMaterialSaved(MaterialDoc* pMaterial) {
	CTreeCtrl& tree = GetTreeCtrl();

	//Remove the asterik
	HTREEITEM* materialItem = NULL;
	materialToTree.Get(pMaterial->name, &materialItem);

	//We will get this message for a delete file so the material will not be in the tree
	if(materialItem) {
		tree.SetItemImage(*materialItem, IMAGE_MATERIAL, IMAGE_MATERIAL);
	}

	//Check if the file is completely saved
	if(treeWithFile) {

		if(!materialDocManager->IsFileModified(pMaterial->renderMaterial->GetFileName())) {

			HTREEITEM* fileItem = NULL;
			idStr file = pMaterial->renderMaterial->GetFileName();

			if(fileToTree.Get(file, &fileItem)) {
				tree.SetItemImage(*fileItem, IMAGE_FILE, IMAGE_FILE);
			}
		}
	}
}

/**
* Called when a material is added
* @param pMaterial The material that was added.
*/
void MaterialTreeView::MV_OnMaterialAdd(MaterialDoc* pMaterial) {

	idStrList list(1024);

	idMaterial	*mat = pMaterial->renderMaterial;
	idStr temp;

	if(treeWithFile) {
		idStr filename = mat->GetFileName();
		filename.StripPath();
		temp = idStr(mat->GetFileName()) + "/" + idStr(mat->GetName()) + "|" + filename;
	} else {
		temp = mat->GetName();
	}

	list.Append(temp);
	AddStrList(NULL, &list, treeWithFile);	

	//Keep the items sorted
	HTREEITEM* item = NULL;
	materialToTree.Get(pMaterial->name, &item);
	if(*item) {
		CTreeCtrl& tree = GetTreeCtrl();
		HTREEITEM parent = tree.GetParentItem(*item);
		tree.SortChildren(parent);
	}

	MV_OnMaterialChange(pMaterial);
}

/**
* Called when a material is deleted
* @param pMaterial The material that was deleted.
*/
void MaterialTreeView::MV_OnMaterialDelete(MaterialDoc* pMaterial) {

	//Our doc told us a material has been deleted. Lets find and remove the item from our tree
	HTREEITEM* materialItem = NULL;
	materialToTree.Get(pMaterial->name, &materialItem);

	CTreeCtrl& tree = GetTreeCtrl();
	tree.DeleteItem(*materialItem);

	//Remove our old quick lookup value
	materialToTree.Remove(pMaterial->name.c_str());
}

/**
* Called when the material name has changed
* @param pMaterial The material that was deleted.
* @param oldName The old name of the material.
*/
void MaterialTreeView::MV_OnMaterialNameChanged(MaterialDoc* pMaterial, const char* oldName) {

	CTreeCtrl& tree = GetTreeCtrl();

	if(!internalChange) {
		
		//Delete the old tree item
		HTREEITEM* item = NULL;
		materialToTree.Get(oldName, &item);
		CTreeCtrl& tree = GetTreeCtrl();
		HTREEITEM tempItem = *item;
		CleanLookupTrees(tempItem);
		tree.DeleteItem(tempItem);
		

		//Now add it back
		idStrList list(1024);
		idMaterial	*mat = pMaterial->renderMaterial;
		idStr temp;

		if(treeWithFile) {
			idStr filename = mat->GetFileName();
			filename.StripPath();
			temp = idStr(mat->GetFileName()) + "/" + idStr(mat->GetName()) + "|" + filename;
		} else {
			temp = mat->GetName();
		}

		list.Append(temp);
		AddStrList(NULL, &list, treeWithFile);	

		//Keep the items sorted
		//item = NULL;
		materialToTree.Get(pMaterial->name.c_str(), &item);
		if(*item) {
			CTreeCtrl& tree = GetTreeCtrl();
			HTREEITEM parent = tree.GetParentItem(*item);
			tree.SortChildren(parent);
		}

		MV_OnMaterialChange(pMaterial);

	}
}

/**
* Called when a file has been reloaded
* @param filename The file that was reloaded.
*/
void MaterialTreeView::MV_OnFileReload(const char* filename) {

	HTREEITEM* fileItem = NULL;
	fileToTree.Get(filename, &fileItem);

	HTREEITEM item = *fileItem;

	CTreeCtrl& tree = GetTreeCtrl();
	CleanLookupTrees(item);
	tree.DeleteItem(item);

	BuildMaterialList(treeWithFile, filename);

	//Resort the parent to make sure the file is back where it was
	HTREEITEM* newItem = NULL;
	fileToTree.Get(filename, &newItem);
	if(*newItem) {
		CTreeCtrl& tree = GetTreeCtrl();
		HTREEITEM parent = tree.GetParentItem(*newItem);
		tree.SortChildren(parent);
	}
}

/**
* Returns true if the user can copy the selected item.
*/
bool MaterialTreeView::CanCopy() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	if(item && itemType == TYPE_MATERIAL) {
		return true;
	} else {
		return false;
	}
}

/**
* Returns true if the user can paste an item in the copy buffer.
*/
bool MaterialTreeView::CanPaste() {
	return materialDocManager->IsCopyMaterial();
}

/**
* Returns true if the user can cut the selected item.
*/
bool MaterialTreeView::CanCut() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	if(item && itemType == TYPE_MATERIAL) {
		return true;
	} else {
		return false;
	}
}

/**
* Returns true if the user can delete the selected item.
*/
bool MaterialTreeView::CanDelete() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	if(itemType == TYPE_MATERIAL_FOLDER || itemType == TYPE_MATERIAL) {
		return true;
	}

	return false;
}

/**
* Returns true if the user can rename the selected item.
*/
bool MaterialTreeView::CanRename() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	if(itemType == TYPE_MATERIAL_FOLDER || itemType == TYPE_MATERIAL) {
		return true;
	}
	return false;
}

/**
* Returns true if the currently selected file needs to be saved.
*/
bool MaterialTreeView::CanSaveFile() {

	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM item = tree.GetSelectedItem();

	idStr filename;
	if(item && GetFileName(item, filename)) {
		if(materialDocManager->IsFileModified(filename.c_str()))
			return true;
		else
			return false;
	} else {
		return false;
	}
}

/**
* Returns the filename of currently selected file.
*/
idStr MaterialTreeView::GetSaveFilename() {

	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM item = tree.GetSelectedItem();

	idStr filename = "";
	if(item) {
		if(!GetFileName(item, filename)) {
			filename = "";
		}
	}

	return filename;
}

/**
* Searches for a material given the supplied search parameters.
* @param searchData The parameters to use for the search.
*/
bool MaterialTreeView::FindNextMaterial(MaterialSearchData_t* searchData) {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM selected = tree.GetSelectedItem();
	if(!selected) {
		selected = tree.GetRootItem();
		if(!selected) {
			return false;
		}
	}

	//Make sure we are in a file
	if(searchData->searchScope == 0) {
		DWORD type = tree.GetItemData(selected);
		if(type == TYPE_FOLDER || type == TYPE_ROOT)
			return false;
	}

	HTREEITEM search =selected;

	while((search = GetNextSeachItem(search, (searchData->searchScope == 0))) != NULL) {
		HTREEITEM found = FindNextMaterial(search, searchData);
		if(found) {
			tree.SelectItem(found);
			return true;
		}
	}
	return false;
}

/**
* Searches for a material given the supplied search parameters. Returns the tree item where
* the item was found or NULL if no material was found.
* @param item The tree item from where to start the search.
* @param searchData The parameters to use for the search.
*/
HTREEITEM MaterialTreeView::FindNextMaterial(HTREEITEM item, MaterialSearchData_t* searchData) {

	CTreeCtrl& tree = GetTreeCtrl();
	DWORD type = tree.GetItemData(item);

	if(type == TYPE_MATERIAL) {
		//check the tree name first
		idStr itemName = tree.GetItemText(item);
		int findPos = itemName.Find(searchData->searchText, false);
		if(findPos != -1) {
			//Todo: Include match whole word
			return item;
		}

		if(!searchData->nameOnly) {
			//Check the material
			idStr materialName = GetMediaPath(item, TYPE_MATERIAL);
			if(materialDocManager->FindMaterial(materialName, searchData, false)) {
				return item;
			}
		}
	} else {
		//Just check the tree name
		idStr itemName = tree.GetItemText(item);

		int findPos = itemName.Find(searchData->searchText, false);
		if(findPos != -1) {
			//Todo: Include match whole word
			return item;
		}
	}
	return NULL;
}

/**
* Returns the next item to search or NULL if there is nothing else to search.
* @param item The last item searched.
* @param stayInFile True if the search should stay in the current file.
*/
HTREEITEM MaterialTreeView::GetNextSeachItem(HTREEITEM item, bool stayInFile) {
	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM nextItem = NULL;

	//Check our children
	if(tree.ItemHasChildren(item)) {
		nextItem = tree.GetChildItem(item);
		return nextItem;
	}

	//Check our siblings
	nextItem = tree.GetNextSiblingItem(item);
	if(nextItem) {
		return nextItem;
	}

	//Check our parents next sibiling
	HTREEITEM parent = item;
	while((parent = tree.GetParentItem(parent)) != NULL) {
		DWORD parType = tree.GetItemData(parent);
		if(stayInFile && parType == TYPE_FILE)
			break;

		HTREEITEM sib = tree.GetNextSiblingItem(parent);
		if(sib) {
			nextItem = sib;
			break;
		}
	}
	return nextItem;
}

/**
* Deletes a given folder.
* @param item The folder to delete.
* @param addUndo True if this operation can be undone.
*/
void  MaterialTreeView::DeleteFolder(HTREEITEM item, bool addUndo) {

	CTreeCtrl& tree = GetTreeCtrl();

	idList<MaterialTreeItem_t> materialsToDelete;

	//Get the complete list of materials to delete
	GetMaterialPaths(item, &materialsToDelete);

	idStrList affectedMaterials;

	//Now delete the materials
	for(int i = 0; i < materialsToDelete.Num(); i++) {

		affectedMaterials.Append(materialsToDelete[i].materialName);

		const idMaterial* material = declManager->FindMaterial(materialsToDelete[i].materialName);

		MaterialDoc* pMaterial = NULL;
		pMaterial = materialDocManager->CreateMaterialDoc(const_cast<idMaterial *>(material));
		materialDocManager->DeleteMaterial(pMaterial, false);
	}

	//Make our undo modifier
	if(addUndo) {
		DeleteMaterialFolderModifier* mod = new DeleteMaterialFolderModifier(materialDocManager, tree.GetItemText(item), this, tree.GetParentItem(item), &affectedMaterials);
		materialDocManager->AddMaterialUndoModifier(mod);
	}


	//Now clean up the folders and quicktree
	CleanLookupTrees(item);

	//Remove any folders that were there
	tree.DeleteItem(item);
}

/**
* Adds a new material folder.
* @param name The name of the folder.
* @param parent The parent item of the folder.
*/
HTREEITEM MaterialTreeView::AddFolder(const char* name, HTREEITEM parent) {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM newItem = tree.InsertItem(name, parent);
	tree.SetItemImage(newItem, IMAGE_MATERIAL_FOLDER, IMAGE_MATERIAL_FOLDER);
	tree.SetItemData(newItem, TYPE_MATERIAL_FOLDER);
	tree.Expand(newItem, TVE_EXPAND); 

	//Make sure the tree is still sorted
	tree.SortChildren(parent);

	//Build the entire path to this item for the quicktree
	idStr qt = GetQuicktreePath(newItem);
	quickTree.Set(qt, newItem);

	return newItem;
}

/**
* Renames a material folder.
* @param item The folder tree item.
* @param name The new name of the material folder.
*/
void MaterialTreeView::RenameFolder(HTREEITEM item, const char* name) {

	CTreeCtrl& tree = GetTreeCtrl();

	//Clean up the quicktree with the current tree before we allow the edit to commit
	CleanLookupTrees(item);

	//Store some data so the we can make the appropriate changes after the commit
	renamedFolder = item;

	affectedMaterials.Clear();
	GetMaterialPaths(renamedFolder, &affectedMaterials);

	tree.SetItemText(item, name);

	PostMessage(MSG_RENAME_FOLDER_COMPLETE); 
}

/**
* Handles the keyboard shortcut for delete.
*/
BOOL MaterialTreeView::PreTranslateMessage(MSG* pMsg) {

	CTreeCtrl& tree = GetTreeCtrl();
	if (pMsg->hwnd == tree.GetSafeHwnd()) {

		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE) {
			OnDeleteMaterial();
			return TRUE;
		}
	}
	return FALSE;
}

/**
* Called by the MFC framework as the view is being created.
*/
int MaterialTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct) {

	lpCreateStruct->style |= TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_EDITLABELS | TVS_SHOWSELALWAYS | TVS_INFOTIP;
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CTreeCtrl& tree = GetTreeCtrl();
	m_image.Create(IDB_ME_TREEBITMAP, 16, 1, RGB(255, 255, 255));
	tree.SetImageList(&m_image, TVSIL_NORMAL);
	
	return 0;
}

/**
* Changes the selected material when the select tree item changes.
*/
void MaterialTreeView::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult) {
	
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	
	if(pNMTreeView->itemNew.hItem) {
		CTreeCtrl& tree = GetTreeCtrl();

		DWORD type = tree.GetItemData(pNMTreeView->itemNew.hItem);
		if(type == TYPE_MATERIAL) {
			idStr mediaName = GetMediaPath(pNMTreeView->itemNew.hItem, type);
			const idMaterial* material = declManager->FindMaterial(mediaName);
			
			materialDocManager->SetSelectedMaterial(const_cast<idMaterial*>(material));
			
		} else {
			
			materialDocManager->SetSelectedMaterial(NULL);
		}

	} else {
		
		materialDocManager->SetSelectedMaterial(NULL);
	}

	*pResult = 0;
}

/**
* Determines if a tree item's label can be edited.
*/
void MaterialTreeView::OnTvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult) {
	
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);

	CTreeCtrl& tree = GetTreeCtrl();
	DWORD type = tree.GetItemData(pTVDispInfo->item.hItem);

	//Only allow renaming of materials and material folders
	if(type == TYPE_MATERIAL || type == TYPE_MATERIAL_FOLDER) {
		*pResult = 0;
	} else {
		*pResult = 1;
	}
}

/**
* Makes sure that a rename operation can be performed after a label edit is complete and 
* performs the folder or material rename.
*/
void MaterialTreeView::OnTvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult) {

	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);

	*pResult = 0;

	if(pTVDispInfo->item.pszText) {
		
		//Convert any edited text to lower case to keep the name canonical
		idStr newLabel = pTVDispInfo->item.pszText;
		newLabel.ToLower();
		strncpy( pTVDispInfo->item.pszText, newLabel.c_str(), pTVDispInfo->item.cchTextMax);

		CTreeCtrl& tree = GetTreeCtrl();
		DWORD type = tree.GetItemData(pTVDispInfo->item.hItem);

		if(type == TYPE_MATERIAL) {

			MaterialDoc* pMaterial = materialDocManager->GetCurrentMaterialDoc();

			//Remove our old quick lookup value
			materialToTree.Remove(pMaterial->name.c_str());

			//Generate the new name
			idStr material;
			HTREEITEM parent = tree.GetParentItem(pTVDispInfo->item.hItem);
			DWORD parentType = tree.GetItemData(parent);
			if(parentType == TYPE_MATERIAL_FOLDER) {
				//Need to include the material folder
				material = GetMediaPath(parent, TYPE_MATERIAL_FOLDER);
				material += "/";
			}

			material += pTVDispInfo->item.pszText;

			if(declManager->FindMaterial(material, false)) {
				//Can't rename because it conflicts with an existing file
				MessageBox("Unable to rename material because it conflicts with another material", "Error");
			} else {
				//Add it to our quick lookup
				materialToTree.Set(material, pTVDispInfo->item.hItem);

				//Finally make the change
				internalChange = true;
				pMaterial->SetMaterialName(material);
				internalChange = false;

				renamedFolder = pTVDispInfo->item.hItem;
				PostMessage(MSG_RENAME_MATERIAL_COMPLETE); 

				*pResult = 1;
			}

		} else if (type == TYPE_MATERIAL_FOLDER) {

			//Clean up the quicktree with the current tree before we allow the edit to commit
			CleanLookupTrees(pTVDispInfo->item.hItem);

			//Store some data so the we can make the appropriate changes after the commit
			renamedFolder = pTVDispInfo->item.hItem;

			affectedMaterials.Clear();
			GetMaterialPaths(renamedFolder, &affectedMaterials);

			PostMessage(MSG_RENAME_FOLDER_COMPLETE); 

			RenameMaterialFolderModifier* mod = new RenameMaterialFolderModifier(materialDocManager, pTVDispInfo->item.pszText, this, pTVDispInfo->item.hItem, tree.GetItemText(pTVDispInfo->item.hItem));
			materialDocManager->AddMaterialUndoModifier(mod);

			*pResult = 1;
		}
	}
}

/**
* Displays the popup menu.
*/
void MaterialTreeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	ScreenToClient (&point);
	PopupMenu (&point);
}

/**
* Displays the popup menu.
*/
void MaterialTreeView::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	CTreeCtrl& tree = GetTreeCtrl();

	DWORD dwPos = GetMessagePos();

	CPoint pt( LOWORD( dwPos ), HIWORD ( dwPos ) );

	CPoint spt = pt;
	tree.ScreenToClient( &spt );

	UINT test;
	HTREEITEM item = tree.HitTest( spt, &test );

	if ( item != NULL  )
	{
		if ( test & TVHT_ONITEM )
		{
			//Select the item
			tree.SelectItem(item);
			OnContextMenu( this, pt );
		}
	}

	*pResult = 0;
}

/**
* Handles keyboard shortcut for cut, copy and paste
*/
void MaterialTreeView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == 3 && GetKeyState(VK_CONTROL)) {
		OnCopy();
	}

	if(nChar == 22 && GetKeyState(VK_CONTROL)) {
		OnPaste();
	}

	if(nChar == 24 && GetKeyState(VK_CONTROL)) {
		OnCut();
	}

	CTreeView::OnChar(nChar, nRepCnt, nFlags);
}

/**
* Begins the process of a drag cut/copy.
*/
void MaterialTreeView::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM selecteditem = tree.GetSelectedItem();

	//Check to see if the are clicking on an item
	UINT flags;
	HTREEITEM item = tree.HitTest(pNMTreeView->ptDrag, &flags);

	if(item && (TVHT_ONITEM & flags)) {
		if(item != selecteditem) {
			tree.SelectItem(item);
		}
	}

	DWORD itemType = tree.GetItemData(item);

	if(itemType == TYPE_MATERIAL) {

		//Create the drag image
		dragImage = tree.CreateDragImage(item);
		dragImage->BeginDrag(0, CPoint (8, 8));
		dragImage->DragEnter(GetDesktopWindow(), pNMTreeView->ptDrag);

		//Drag is in progress
		bDragging = true;

		dragItem = item;

		//Capture the messages
		SetCapture();
	}

	*pResult = 0;
}

/**
* Handles mouse movement as an item is being dragged.
*/
void MaterialTreeView::OnMouseMove(UINT nFlags, CPoint point) {
	if( bDragging ) {
		CTreeCtrl& tree = GetTreeCtrl();

		dropPoint = point;
		ClientToScreen(&dropPoint);

		//Move the drag image
		dragImage->DragMove(dropPoint);
		dragImage->DragShowNolock(FALSE);

		dragImage->DragShowNolock(TRUE);
	}

	if(bDragging) {
		//Test the hover item

		CTreeCtrl& tree = GetTreeCtrl();

		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);

		UINT flags;
		HTREEITEM item = tree.HitTest(point, &flags);
		if(item && (TVHT_ONITEM & flags)) {
			if(item != hoverItem) {
				hoverItem = item;
				hoverStartTime = timeGetTime();
			} else {
				DWORD currentTime = timeGetTime();
				if(currentTime - hoverStartTime > HOVER_EXPAND_DELAY) {

					UINT state = tree.GetItemState(hoverItem, TVIS_EXPANDED);
					if(state != TVIS_EXPANDED && tree.ItemHasChildren(hoverItem)) {
						tree.Expand(hoverItem, TVE_EXPAND);
					}

				}
			}
		}
	}

	CTreeView::OnMouseMove(nFlags, point);
}

/**
* Handles the end of a drag copy/move when the user releases the left mouse button.
*/
void MaterialTreeView::OnLButtonUp(UINT nFlags, CPoint point) {
	CTreeCtrl& tree = GetTreeCtrl();

	if( bDragging ) {
		//Release mouse capture
		ReleaseCapture();

		//Delete the drag image
		dragImage->DragLeave(GetDesktopWindow());
		dragImage->EndDrag();

		bDragging = false;

		delete dragImage;

		UINT flags;
		HTREEITEM item = tree.HitTest(point, &flags);
		if(item && (TVHT_ONITEM & flags)) {

			DWORD itemType = tree.GetItemData(item);

			if(itemType == TYPE_MATERIAL) //Backup one if a file is selected
				item = tree.GetParentItem(item);

			//Make sure we aren't dragging to the same place
			HTREEITEM dragItemParent = tree.GetParentItem(dragItem);
			if(dragItemParent != item) {


				idStr dragFile;
				GetFileName(dragItem, dragFile);

				idStr filename;
				GetFileName(item, filename);

				//Move within a file copy across files
				if(!dragFile.Icmp(filename)) {
					materialDocManager->CopyMaterial(materialDocManager->GetCurrentMaterialDoc(), true);
				} else {
					materialDocManager->CopyMaterial(materialDocManager->GetCurrentMaterialDoc(), false);
				}

				//Generate the name

				idStr materialName = GetMediaPath(item, itemType);

				idStr copyName = materialDocManager->GetCopyMaterialName();
				idStr copyMaterialName;
				copyName.ExtractFileName(copyMaterialName);
				materialName += "/" + copyMaterialName;

				//If the material name already exists add numbers until we don't find it
				materialName = materialDocManager->GetUniqueMaterialName(materialName);

				//Paste
				materialDocManager->PasteMaterial(materialName, filename);
			}
		}
	}

	CTreeView::OnLButtonUp(nFlags, point);
}

/**
* Applies the current material.
*/
void MaterialTreeView::OnApplyMaterial() {
	materialDocManager->ApplyMaterial(materialDocManager->GetCurrentMaterialDoc());
}

/**
* Applies all materials in the currently selected file.
*/
void MaterialTreeView::OnApplyFile() {
	idStr filename;
	HTREEITEM item = GetTreeCtrl().GetSelectedItem();
	if(GetFileName(item, filename)) {
		materialDocManager->ApplyFile(filename.c_str());
	}
}

/**
* Applies all materials that need to be applied.
*/
void MaterialTreeView::OnApplyAll() {
	materialDocManager->ApplyAll();
}

/**
* Saves the selected material.
*/
void MaterialTreeView::OnSaveMaterial() {
	materialDocManager->SaveMaterial(materialDocManager->GetCurrentMaterialDoc());
}

/**
* Saves all materials in the selected file.
*/
void MaterialTreeView::OnSaveFile() {
	idStr filename;
	HTREEITEM item = GetTreeCtrl().GetSelectedItem();
	if(GetFileName(item, filename)) {
		materialDocManager->SaveFile(filename.c_str());
	}
}

/**
* Save all materials that have been changed.
*/
void MaterialTreeView::OnSaveAll() {
	materialDocManager->SaveAllMaterials();
}

/**
* Begins a label edit to rename a material or material folder.
*/
void MaterialTreeView::OnRenameMaterial() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	tree.EditLabel(item);
}

/**
* Adds a new material.
*/
void MaterialTreeView::OnAddMaterial() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	//Determine the file
	HTREEITEM parent = NULL;
	if(itemType != TYPE_FILE) {

		parent = tree.GetParentItem(item);
		while(1) {
			if(tree.GetItemData(parent) == TYPE_FILE)
				break;
			parent = tree.GetParentItem(parent);
		}
	} else {
		parent = item;
	}
	idStr filename = GetMediaPath(parent, TYPE_FILE);


	//Determine the material folder
	idStr materialFolder = "";
	switch(itemType) {
		case TYPE_MATERIAL:
			{
				HTREEITEM parentFolderItem = tree.GetParentItem(item);
				if(tree.GetItemData(parentFolderItem) == TYPE_MATERIAL_FOLDER)
					materialFolder = GetMediaPath(parentFolderItem, TYPE_MATERIAL_FOLDER);
			}
			break;
		case TYPE_MATERIAL_FOLDER:
			materialFolder = GetMediaPath(item, TYPE_MATERIAL_FOLDER);
			break;
		case TYPE_FILE:
			//There is no material folder
			break;
	}

	idStr name;
	int num = 1;
	while(1) {
		if(materialFolder.Length() > 0) {
			name = va("%s/newmaterial%d", materialFolder.c_str(), num);
		} else {
			name = va("newmaterial%d", num);
		}
		if(!declManager->FindMaterial(name, false))
			break;
		num++;
	}

	materialDocManager->AddMaterial(name.c_str(), filename.c_str());

}

/**
* Adds a new folder
*/
void MaterialTreeView::OnAddFolder() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);


	//Backup if the selected item is a material
	if(itemType == TYPE_MATERIAL) {
		item = tree.GetParentItem(item);
	}

	//Pick a unique material name
	idStr newFolder;
	int num = 1;
	while(1) {
		newFolder = va("newfolder%d", num);
		if(tree.ItemHasChildren(item)) {
			HTREEITEM hChildItem = tree.GetChildItem(item);
			bool found = false;
			while (hChildItem != NULL)
			{
				if(!newFolder.Icmp(tree.GetItemText(hChildItem))) {
					found = true;
					break;
				}
				hChildItem = tree.GetNextSiblingItem(hChildItem);
			}
			if(!found)
				break;
		} else {
			break;
		}
		num++;
	}

	HTREEITEM newItem = AddFolder(newFolder, item);

	AddMaterialFolderModifier* mod = new AddMaterialFolderModifier(materialDocManager, newFolder, this, newItem, item);
	materialDocManager->AddMaterialUndoModifier(mod);
}

/**
* Deletes a material or material folder.
*/
void MaterialTreeView::OnDeleteMaterial() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	if(itemType == TYPE_MATERIAL_FOLDER) {
		int result = MessageBox("Are you sure you want to delete this folder?", "Delete?", MB_ICONQUESTION | MB_YESNO);
		if(result == IDYES) {
			DeleteFolder(item);
		}
	} else if (itemType == TYPE_MATERIAL) {
		int result = MessageBox("Are you sure you want to delete this material?", "Delete?", MB_ICONQUESTION | MB_YESNO);
		if(result == IDYES) {
			materialDocManager->DeleteMaterial(materialDocManager->GetCurrentMaterialDoc());
		}
	}
}

/**
* Reloads the selected file.
*/
void MaterialTreeView::OnReloadFile() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	if(itemType == TYPE_MATERIAL || itemType == TYPE_FILE || itemType == TYPE_MATERIAL_FOLDER) {
		idStr filename;
		GetFileName(item, filename);

		if(materialDocManager->IsFileModified(filename)) {
			int result = MessageBox("This file has been modified. Are you sure you want to reload this file?", "Reload?", MB_ICONQUESTION | MB_YESNO);
			if(result != IDYES) {
				return;
			}
		}
		materialDocManager->ReloadFile(filename);
	}
}

/**
* Performs a cut operation.
*/
void MaterialTreeView::OnCut() {
	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	if(item && itemType == TYPE_MATERIAL) {
		materialDocManager->CopyMaterial(materialDocManager->GetCurrentMaterialDoc(), true);
	} else if (itemType == TYPE_MATERIAL_FOLDER) {
	}
}

/**
* Performs a copy operation.
*/
void MaterialTreeView::OnCopy() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	if(itemType == TYPE_MATERIAL) {
		materialDocManager->CopyMaterial(materialDocManager->GetCurrentMaterialDoc(), false);
	} else if (itemType == TYPE_MATERIAL_FOLDER) {
	}
}

/**
* Performs a paste operation.
*/
void MaterialTreeView::OnPaste() {

	CTreeCtrl& tree = GetTreeCtrl();

	HTREEITEM item = tree.GetSelectedItem();
	DWORD itemType = tree.GetItemData(item);

	//Paste a material
	if(item && materialDocManager->IsCopyMaterial() && itemType >= TYPE_FILE) {

		//Generate the name
		if(itemType == TYPE_MATERIAL) {//Backup one if a file is selected
			item = tree.GetParentItem(item);
			itemType = tree.GetItemData(item);
		}

		idStr materialName = "";
		if(itemType != TYPE_FILE) {
			materialName = GetMediaPath(item, itemType) + "/";
		}

		idStr copyName = materialDocManager->GetCopyMaterialName();
		idStr copyMaterialName;
		copyName.ExtractFileName(copyMaterialName);
		materialName += copyMaterialName;

		idStr filename;
		GetFileName(item, filename);

		//If the material name already exists add numbers until we don't find it
		materialName = materialDocManager->GetUniqueMaterialName(materialName);

		//Paste
		materialDocManager->PasteMaterial(materialName, filename);

	}	
}

/**
* This message is sent after the label edit is complete to actually perform the rename
* operation.
*/
LRESULT MaterialTreeView::OnRenameFolderComplete(WPARAM wParam, LPARAM lParam) {

	//Generate new quick tree info for all material folders
	BuildLookupTrees(renamedFolder);

	//Go through the list of affected materials and rename them
	for(int i = 0; i < affectedMaterials.Num(); i++) {
		RenameMaterial(affectedMaterials[i].treeItem, affectedMaterials[i].materialName);
	}

	//Make sure the tree stays sorted
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM parent = tree.GetParentItem(renamedFolder);
	tree.SortChildren(parent);

	return 0;
}

/**
* This message is sent after the label edit is complete to ensure that the sorting stays consistent.
*/
LRESULT MaterialTreeView::OnRenameMaterialComplete(WPARAM wParam, LPARAM lParam) {

	//Make sure the tree stays sorted
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM parent = tree.GetParentItem(renamedFolder);
	tree.SortChildren(parent);

	return 0;
}

/**
* Handles all of the little problems associated with renaming a folder.
*/
void MaterialTreeView::RenameMaterial(HTREEITEM item, const char* originalName) {

	CTreeCtrl& tree = GetTreeCtrl();

	const idMaterial* material = declManager->FindMaterial(originalName);

	MaterialDoc* pMaterial;
	//pMaterial = materialDocManager->GetInProgressDoc(material);

	//if(!pMaterial) {
	pMaterial = materialDocManager->CreateMaterialDoc(const_cast<idMaterial *>(material));
	//}

	//Remove our old quick lookup value
	materialToTree.Remove(originalName);

	//Generate the new name
	idStr materialName;
	HTREEITEM parent = tree.GetParentItem(item);
	DWORD parentType = tree.GetItemData(parent);
	if(parentType == TYPE_MATERIAL_FOLDER) {
		//Need to include the material folder
		materialName = GetMediaPath(parent, TYPE_MATERIAL_FOLDER);
		materialName += "/";
	}
	materialName += tree.GetItemText(item);


	//Add it to our quick lookup
	materialToTree.Set(materialName, item);

	//Finally make the change
	internalChange = true;
	pMaterial->SetMaterialName(materialName, false);
	internalChange = false;
}

/**
* Returns the filename of the provided item.
* @param item The item for which to generate the filename
* @param out The location the filename will be placed.
*/
bool MaterialTreeView::GetFileName(HTREEITEM item, idStr& out) {

	out = "";

	CTreeCtrl& tree = GetTreeCtrl();
	DWORD type = tree.GetItemData(item);

	if(type != TYPE_MATERIAL && type != TYPE_MATERIAL_FOLDER && type != TYPE_FILE)
		return false;

	if(type == TYPE_FILE) {
		out = GetMediaPath(item, TYPE_FILE);
		return true;
	}

	HTREEITEM parent = tree.GetParentItem( item );
	while ( parent != NULL ) {
		DWORD parentType = tree.GetItemData(parent);
		if(parentType == TYPE_FILE) {
			out = GetMediaPath(parent, TYPE_FILE);
			return true;
		}
		parent = tree.GetParentItem( parent );
	}

	return false;
}

/**
* Returns the Doom III name for the provided item
* @param item The item for which to generate the name
* @param type The type of the selected item
*/
idStr MaterialTreeView::GetMediaPath(HTREEITEM item, DWORD type) {

	//Determine when to stop building the path
	DWORD stopType = TYPE_ROOT;
	switch(type) {
		case TYPE_MATERIAL:
			stopType = TYPE_FILE;
			break;
		case TYPE_MATERIAL_FOLDER:
			stopType = TYPE_FILE;
			break;
		case TYPE_FILE:
			stopType = TYPE_ROOT;
			break;
	};

	CTreeCtrl& tree = GetTreeCtrl();

	idStr mediaName = tree.GetItemText( item );

	// have to build the name back up
	HTREEITEM parent = tree.GetParentItem( item );
	while ( parent != NULL ) {

		//stop the iteration once we have found a specific type
		DWORD parentType = tree.GetItemData(parent);
		if(parentType == stopType) {
			break;
		}

		idStr strParent = tree.GetItemText( parent );
		strParent += "/";
		strParent += mediaName;
		mediaName = strParent;
		parent = tree.GetParentItem( parent );

	}

	return mediaName;
}

/**
* Creates a list of material paths for all materials under the provided item.
* @param item The base item for which to generate the list
* @param list The list in which the paths will be stored.
*/
void MaterialTreeView::GetMaterialPaths(HTREEITEM item, idList<MaterialTreeItem_t>* list) {

	CTreeCtrl& tree = GetTreeCtrl();
	if(tree.ItemHasChildren(item)) {

		HTREEITEM childItem = tree.GetChildItem(item);
		while(childItem != NULL) {

			DWORD childType = tree.GetItemData(childItem);
			if (childType == TYPE_MATERIAL) {
				MaterialTreeItem_t mat;
				mat.materialName = GetMediaPath(childItem, TYPE_MATERIAL);
				mat.treeItem = childItem;
				list->Append(mat);
			} else if (childType == TYPE_MATERIAL_FOLDER) {
				GetMaterialPaths(childItem, list);
			}
			childItem = tree.GetNextSiblingItem(childItem);
		}
	}
}

/**
* Adds a string list of materials to the tree creating the proper hierarchy.
* @param root The name of the root item or NULL for no root item.
* @param list The list of materials.
* @param includeFile If true the materials will be sorted by file.
*/
void MaterialTreeView::AddStrList(const char *root, idStrList *list, bool includeFile) {
	
	CTreeCtrl& treeMedia = GetTreeCtrl();

	idStr		out, path;
	HTREEITEM	base = NULL;
	
	if(root) {
		base = treeMedia.GetRootItem();
		if (base) {
			out = treeMedia.GetItemText(base);
			if (stricmp(root, out)) {
				base = NULL;
			}
		}

		if (base == NULL) {
			base = treeMedia.InsertItem(root);
			treeMedia.SetItemData(base, TYPE_ROOT);
		}
	}

	HTREEITEM	item = base;
	HTREEITEM	add;

	list->Sort();
	int	count = list->Num();

	idStr	last, qt;
	for (int i = 0; i < count; i++) {
		idStr *strItem = &(*list)[i];
		

		idStr name = strItem->c_str();

		idStr filename;
		bool afterFile = true;
		if(includeFile) {
			int index = name.Find("|");
			if(index >= 0) {
				afterFile = false;
				filename = name.Right(name.Length() - index - 1);
				name = name.Left(index);
			}
		}

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
			treeMedia.SetItemImage(add, IMAGE_MATERIAL, IMAGE_MATERIAL);
			treeMedia.SetItemData(add, TYPE_MATERIAL);

			//Add the item to a quick lookup table
			idStr material = GetMediaPath(add, TYPE_MATERIAL);
			materialToTree.Set(material, add);

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
				name.Left(index, out);
				path += out;
				qt = root;
				qt += "/";
				qt += path;
				if (quickTree.Get(qt, &check)) {
					newItem = *check;
				}
				
				bool thisisfile = false;
				if(out == filename) {
					thisisfile = true;
					afterFile = true;

				}
				
				if (newItem == NULL) {
					newItem = treeMedia.InsertItem(out, item);
					qt = root;
					qt += "/";
					qt += path;
					quickTree.Set(qt, newItem);
					

					if(!afterFile || thisisfile) {
						if(thisisfile) {
							afterFile = true;
							treeMedia.SetItemImage(newItem, IMAGE_FILE, IMAGE_FILE);
							treeMedia.SetItemData(newItem, TYPE_FILE);

							//Add the item to a quick lookup table
							idStr file = GetMediaPath(newItem, TYPE_FILE);
							//common->Printf("Adding fileToTree: %s - %d\n", file.c_str(), newItem);
							fileToTree.Set(file, newItem);

						} else {
							treeMedia.SetItemImage(newItem, IMAGE_FOLDER, IMAGE_FOLDER);
							treeMedia.SetItemData(newItem, TYPE_FOLDER);
						}
					} else {
						treeMedia.SetItemImage(newItem, IMAGE_MATERIAL_FOLDER, IMAGE_MATERIAL_FOLDER);
						treeMedia.SetItemData(newItem, TYPE_MATERIAL_FOLDER);

					}
				}

				
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
				treeMedia.SetItemImage(add, IMAGE_MATERIAL, IMAGE_MATERIAL);
				treeMedia.SetItemData(add, TYPE_MATERIAL);
				path = "";

				//Add the item to a quick lookup table
				idStr material = GetMediaPath(add, TYPE_MATERIAL);
				materialToTree.Set(material, add);
			}
		}
	}
}

/**
* Displays the popup menu with all of the appropriate menu items enabled.
* @param pt The location where the menu should be displayed.
*/
void MaterialTreeView::PopupMenu(CPoint* pt) {

	//Determine the type of object clicked on
	CTreeCtrl& tree = GetTreeCtrl();
	UINT test;
	HTREEITEM item = tree.HitTest( *pt, &test );
	if ( item == NULL ||  !(test & TVHT_ONITEM) )
		return;

	ClientToScreen (pt);

	CMenu FloatingMenu;
	VERIFY(FloatingMenu.LoadMenu(IDR_ME_MATERIALTREE_POPUP));
	CMenu* pPopupMenu = FloatingMenu.GetSubMenu (0);
	
	DWORD itemType = tree.GetItemData(item);

	//Enable/Disable based on the state
	MaterialDoc* pDoc = materialDocManager->GetCurrentMaterialDoc();


	//Apply Changes
	if(pDoc && pDoc->applyWaiting) {
		pPopupMenu->EnableMenuItem(ID_POPUP_APPLYMATERIAL, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_APPLYMATERIAL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	//Apply File
	idStr filename;
	if(GetFileName(item, filename)) {
		if(materialDocManager->DoesFileNeedApply(filename.c_str()))
			pPopupMenu->EnableMenuItem(ID_POPUP_APPLYFILE, MF_BYCOMMAND | MF_ENABLED);
		else
			pPopupMenu->EnableMenuItem(ID_POPUP_APPLYFILE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_APPLYFILE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	//Apply All
	if(materialDocManager->DoesAnyNeedApply()) {
		pPopupMenu->EnableMenuItem(ID_POPUP_APPLYALL, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_APPLYALL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	//Save Material
	if(pDoc && pDoc->modified) {
		pPopupMenu->EnableMenuItem(ID_POPUP_SAVEMATERIAL, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_SAVEMATERIAL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	//Save File
	if(GetFileName(item, filename)) {
		if(materialDocManager->IsFileModified(filename.c_str()))
			pPopupMenu->EnableMenuItem(ID_POPUP_SAVEFILE, MF_BYCOMMAND | MF_ENABLED);
		else
			pPopupMenu->EnableMenuItem(ID_POPUP_SAVEFILE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_SAVEFILE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	//Save All
	if(materialDocManager->IsAnyModified()) {
		pPopupMenu->EnableMenuItem(ID_POPUP_SAVEALL, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_SAVEALL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if(itemType == TYPE_MATERIAL || itemType == TYPE_MATERIAL_FOLDER) {
		pPopupMenu->EnableMenuItem(ID_POPUP_RENAMEMATERIAL, MF_BYCOMMAND | MF_ENABLED);
		pPopupMenu->EnableMenuItem(ID_POPUP_DELETEMATERIAL, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_RENAMEMATERIAL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_POPUP_DELETEMATERIAL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if(itemType == TYPE_FILE || itemType == TYPE_MATERIAL_FOLDER || itemType == TYPE_MATERIAL) {
		pPopupMenu->EnableMenuItem(ID_POPUP_ADDMATERIAL, MF_BYCOMMAND | MF_ENABLED);
		pPopupMenu->EnableMenuItem(ID_POPUP_ADDFOLDER, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_ADDMATERIAL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_POPUP_ADDFOLDER, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if(itemType == TYPE_MATERIAL) {
		pPopupMenu->EnableMenuItem(ID_POPUP_CUT, MF_BYCOMMAND | MF_ENABLED);
		pPopupMenu->EnableMenuItem(ID_POPUP_COPY, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_CUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_POPUP_COPY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if((itemType == TYPE_MATERIAL || itemType == TYPE_FILE || itemType == TYPE_MATERIAL_FOLDER) && materialDocManager->IsCopyMaterial()) {
		pPopupMenu->EnableMenuItem(ID_POPUP_PASTE, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_PASTE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if(itemType == TYPE_MATERIAL || itemType == TYPE_FILE || itemType == TYPE_MATERIAL_FOLDER) {
		pPopupMenu->EnableMenuItem(ID_POPUP_RELOADFILE, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_POPUP_RELOADFILE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	
	pPopupMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt->x, pt->y, &GetTreeCtrl());
}

/**
* Sets the appropriate item image based on the state of the item.
* @param item The item to set.
* @param mod Is the item modified
* @param apply Does the item need an apply
* @param children Should this method recurse through the items children and set their icons.
*/
void MaterialTreeView::SetItemImage(HTREEITEM item, bool mod, bool apply, bool children) {

	CTreeCtrl& tree = GetTreeCtrl();

	int image;

	DWORD itemType = tree.GetItemData(item);
	switch(itemType) {
		case TYPE_FILE:
			if(mod)
				image = IMAGE_FILE_MOD;
			else
				image = IMAGE_FILE;
			break;
		case TYPE_MATERIAL_FOLDER:
			image = IMAGE_MATERIAL_FOLDER;
			break;
		case TYPE_MATERIAL:
			if(mod && apply)
				image = IMAGE_MATERIAL_MOD_APPLY;
			else if(mod)
				image = IMAGE_MATERIAL_MOD;
			else
				image = IMAGE_MATERIAL;
			break;
	}

	tree.SetItemImage(item, image, image);

	if(children) {
		if(tree.ItemHasChildren(item)) {
			HTREEITEM hChildItem = tree.GetChildItem(item);
			while (hChildItem != NULL) {
				SetItemImage(hChildItem, mod, apply, children);
				hChildItem = tree.GetNextSiblingItem(hChildItem);
			}
		}
	}
}

/**
* Cleans the lookup tables for the provided item and all children.
* @param item The item to start from
*/
void MaterialTreeView::CleanLookupTrees(HTREEITEM item) {

	idStr qt = GetQuicktreePath(item);
	quickTree.Remove(qt);

	CTreeCtrl& tree = GetTreeCtrl();

	//Clean special lookup tables
	DWORD type = tree.GetItemData(item);
	if(type == TYPE_FILE) {
		idStr file = GetMediaPath(item, TYPE_FILE);
		fileToTree.Remove(file);
	} else if(type == TYPE_MATERIAL) {
		idStr name = GetMediaPath(item, TYPE_MATERIAL);
		materialToTree.Remove(name);
	}

	//Clean all my children
	if(tree.ItemHasChildren(item)) {
		HTREEITEM childItem = tree.GetChildItem(item);
		while(childItem != NULL) {
			CleanLookupTrees(childItem);
			childItem = tree.GetNextSiblingItem(childItem);
		}
	}
}

/**
* Build the lookup tree for a given item and all of its children.
* @param item The item to start from
*/
void MaterialTreeView::BuildLookupTrees(HTREEITEM item) {

	//Add my quicktree item
	idStr qt = GetQuicktreePath(item);
	quickTree.Set(qt, item);

	CTreeCtrl& tree = GetTreeCtrl();
	if(tree.ItemHasChildren(item)) {
		HTREEITEM childItem = tree.GetChildItem(item);
		while(childItem != NULL) {
			DWORD childType = tree.GetItemData(childItem);
			if(childType == TYPE_MATERIAL_FOLDER) {
				//Recursively call this method for all my child folders
				BuildLookupTrees(childItem);
			}
			childItem = tree.GetNextSiblingItem(childItem);
		}
	}
}

/**
* Returns the quicktree path for a given item.
* @param item The item for which to generate the quicktree path
*/
idStr MaterialTreeView::GetQuicktreePath(HTREEITEM item) {
	CTreeCtrl& tree = GetTreeCtrl();

	idStr qt = "";
	HTREEITEM pathItem = item;
	while(pathItem != NULL) {
		qt = "/" + idStr(tree.GetItemText(pathItem)) + qt;
		pathItem = tree.GetParentItem(pathItem);
	}
	return qt;
}












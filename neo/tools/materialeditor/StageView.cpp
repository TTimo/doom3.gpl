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

#include "StageView.h"

IMPLEMENT_DYNCREATE(StageView, ToggleListView)

BEGIN_MESSAGE_MAP(StageView, ToggleListView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnLvnItemchanged)
	ON_NOTIFY_REFLECT(LVN_DELETEALLITEMS, OnLvnDeleteallitems)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnLvnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclick)

	ON_COMMAND(ID_STAGEPOPUP_RENAMESTAGE, OnRenameStage)
	ON_COMMAND(ID_STAGEPOPUP_DELETESTAGE, OnDeleteStage)
	ON_COMMAND(ID_STAGEPOPUP_DELETEALLSTAGES, OnDeleteAllStages)
	ON_COMMAND(ID_STAGEPOPUP_ADDSTAGE, OnAddStage)
	ON_COMMAND(ID_STAGEPOPUP_ADDBUMPMAP, OnAddBumpmapStage)
	ON_COMMAND(ID_STAGEPOPUP_ADDDIFFUSEMAP, OnAddDiffuseStage)
	ON_COMMAND(ID_STAGEPOPUP_ADDSPECULAR, OnAddSpecualarStage)

	ON_COMMAND(ID_STAGEPOPUP_COPY, OnCopy)
	ON_COMMAND(ID_STAGEPOPUP_PASTE, OnPaste)

	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnLvnBeginlabeledit)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnLvnEndlabeledit)
	ON_WM_CHAR()
END_MESSAGE_MAP()

/**
* Constructor for StageView.
*/
StageView::StageView() {
	currentMaterial = NULL;
	bDragging = false;
	internalChange = false;
}

/**
* Destructor for StageView.
*/
StageView::~StageView() {
}

/**
* Called when the selected material has changed.
* @param pMaterial The newly selected material.
*/
void StageView::MV_OnMaterialSelectionChange(MaterialDoc* pMaterial) {

	currentMaterial = pMaterial;

	RefreshStageList();	
}

/**
* Called when the material changes have been saved. 
* @param pMaterial The saved material.
*/
void StageView::MV_OnMaterialSaved(MaterialDoc* pMaterial) {

	CListCtrl& list = GetListCtrl();

	//Saving a material reenables all of the stages
	if(pMaterial == currentMaterial) {
		for(int i = 1; i < list.GetItemCount(); i++) {
			SetToggleState(i, ToggleListView::TOGGLE_STATE_ON);
		}
	}
}

/**
* Called when a stage is added
* @param pMaterial The material that was affected.
* @param stageNum The index of the stage that was added
*/
void StageView::MV_OnMaterialStageAdd(MaterialDoc* pMaterial, int stageNum) {

	CListCtrl& list = GetListCtrl();

	idStr name = pMaterial->GetAttribute(stageNum, "name");

	int index = list.InsertItem(stageNum+1, name.c_str());
	SetToggleState(index, ToggleListView::TOGGLE_STATE_ON);
}

/**
* Called when a stage is deleted
* @param pMaterial The material that was affected.
* @param stageNum The index of the stage that was deleted
*/
void StageView::MV_OnMaterialStageDelete(MaterialDoc* pMaterial, int stageNum) {
	CListCtrl& list = GetListCtrl();
	list.DeleteItem(stageNum+1);
}

/**
* Called when a stage is moved
* @param pMaterial The material that was deleted.
* @param from The from index
* @param to The to index
*/
void StageView::MV_OnMaterialStageMove(MaterialDoc* pMaterial, int from, int to) {

	if(!internalChange) {
		from++;
		to++;

		CListCtrl& list = GetListCtrl();

		char szLabel[256];
		LV_ITEM lvi;
		ZeroMemory(&lvi, sizeof(LV_ITEM));
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
		lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
		lvi.pszText = szLabel;
		lvi.iItem = from;
		lvi.cchTextMax = 255;
		list.GetItem(&lvi);

		//Delete the original item
		list.DeleteItem(from);

		//Insert the item
		lvi.iItem = to;
		list.InsertItem(&lvi);

		int type = -1;

		int stageType = currentMaterial->GetAttributeInt(to-1, "stagetype");
		switch(stageType) {
				case MaterialDoc::STAGE_TYPE_NORMAL:
					type = MaterialDefManager::MATERIAL_DEF_STAGE;
					break;
				case MaterialDoc::STAGE_TYPE_SPECIALMAP:
					type = MaterialDefManager::MATERIAL_DEF_SPECIAL_STAGE;
					break;
		}

		m_propView->SetPropertyListType(type, to-1);

		Invalidate();

	}
}

/**
* Called when an attribute is changed
* @param pMaterial The material that was deleted.
* @param stage The stage that contains the change.
* @param attribName The attribute that has changed.
*/
void StageView::MV_OnMaterialAttributeChanged(MaterialDoc* pMaterial, int stage, const char* attribName) {

	//Refresh this stage list if a material name has changed
	if(!internalChange && currentMaterial == pMaterial && stage >= 0 && attribName && !strcmp(attribName, "name") ) {
		CListCtrl& list = GetListCtrl();
		list.SetItemText(stage+1, 0, currentMaterial->GetAttribute(stage, attribName));
	}
}

/**
* Returns true if the current state of the stage view will allow a copy operation
*/
bool StageView::CanCopy() {

	CListCtrl& list = GetListCtrl();
	POSITION pos = list.GetFirstSelectedItemPosition();
	int nItem = -1;
	if(pos)
		nItem = list.GetNextSelectedItem(pos);

	if(nItem > 0) {
		return true;
	} else {
		return false;
	}
}

/**
* Returns true if the current state of the stage view will allow a paste operation
*/
bool StageView::CanPaste() {
	return materialDocManager->IsCopyStage();
}

/**
* Cut is not supported for stages.
*/
bool StageView::CanCut() {
	//No cut for stages
	return false;
}

/**
* Returns true if the current state of the stage view will allow a delete operation
*/
bool StageView::CanDelete() {
	CListCtrl& list = GetListCtrl();
	POSITION pos = list.GetFirstSelectedItemPosition();
	int nItem = -1;
	if(pos) {
		nItem = list.GetNextSelectedItem(pos);
	}

	if(nItem > 0)
		return true;

	return false;
}

/**
* Returns true if the current state of the stage view will allow a rename operation
*/
bool StageView::CanRename() {
	CListCtrl& list = GetListCtrl();

	POSITION pos = list.GetFirstSelectedItemPosition();
	int nItem = -1;
	if(pos) {
		nItem = list.GetNextSelectedItem(pos);
	}

	if(nItem > 0) {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		if(nItem > 0 && material->GetAttributeInt(nItem-1, "stagetype") == MaterialDoc::STAGE_TYPE_NORMAL) {
			return true;
		}
	}

	return false;
}

/**
* Rebuilds the list of stages based on the currently selected material
*/
void StageView::RefreshStageList() {
	
	CListCtrl& list = GetListCtrl();

	POSITION pos = list.GetFirstSelectedItemPosition();
	int selectedItem = -1;
	if(pos)
		selectedItem = list.GetNextSelectedItem(pos);

	list.DeleteAllItems();

	if(currentMaterial) {

		//Always add the material item for the main material properties
		list.InsertItem(0, "Material");
		SetToggleState(0, ToggleListView::TOGGLE_STATE_DISABLED);

		//Get the stage info
		int stageCount = currentMaterial->GetStageCount();
		for(int i = 0; i < stageCount; i++) {
			const char* name = currentMaterial->GetAttribute(i, "name");

			int itemNum = list.InsertItem(list.GetItemCount(), name);

			if(currentMaterial->IsStageEnabled(i)) {
				SetToggleState(itemNum, ToggleListView::TOGGLE_STATE_ON);
			} else {
				SetToggleState(itemNum, ToggleListView::TOGGLE_STATE_OFF);
			}
		}

		if(selectedItem < 0) {
			//Select the material
			list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		} else {
			list.SetItemState(selectedItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}
	}
}

/** 
* Called by the MFC framework when the view is being created.
*/
int StageView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (ToggleListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetToggleIcons(MAKEINTRESOURCE(IDI_ME_DISABLED_ICON), MAKEINTRESOURCE(IDI_ME_ON_ICON), MAKEINTRESOURCE(IDI_ME_OFF_ICON));

	return 0;
}

/** 
* Called when the user changes the selection in the list box. This method will notify the 
* property view of the change so that it can display the appropriate properties.
*/
void StageView::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if(!bDragging) {

		//The state has changed and changed to selected
		if(pNMLV->uChanged && LVIF_STATE && pNMLV->uNewState & LVIS_SELECTED) {

			int type = -1;

			if(pNMLV->iItem >= 0) {
				if(pNMLV->iItem == 0)
					type = MaterialDefManager::MATERIAL_DEF_MATERIAL;
				else {
					int stageType = currentMaterial->GetAttributeInt(pNMLV->iItem-1, "stagetype");
					switch(stageType) {
						case MaterialDoc::STAGE_TYPE_NORMAL:
							type = MaterialDefManager::MATERIAL_DEF_STAGE;
							break;
						case MaterialDoc::STAGE_TYPE_SPECIALMAP:
							type = MaterialDefManager::MATERIAL_DEF_SPECIAL_STAGE;
							break;
					}
				}
			}

			m_propView->SetPropertyListType(type, pNMLV->iItem-1);
		}

		if(pNMLV->uChanged && LVIF_STATE && pNMLV->uOldState & LVIS_SELECTED && !(pNMLV->uNewState & LVIS_SELECTED)) {
			//This item was deselected.
			//If there is no item selected then clear the prop list
			CListCtrl& list = GetListCtrl();
			POSITION pos = list.GetFirstSelectedItemPosition();
			if(!pos)
				m_propView->SetPropertyListType(-1);
		}
	}
	*pResult = 0;
}

/** 
* Notifies the property view that all stages have been removed.
*/
void StageView::OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	//The list has been cleared so clear the prop view
	m_propView->SetPropertyListType(-1);

	*pResult = 0;
}

/** 
* Starts the stage drag operation.
*/
void StageView::OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	CListCtrl& list = GetListCtrl();

	//Start a drag if the item isn't the material
	if(pNMLV->iItem > 0) {


		dragIndex = pNMLV->iItem;

		//Trun off ownerdrawn to create the drag image correctly
		list.ModifyStyle(LVS_OWNERDRAWFIXED, 0);

		//Create the drag image
		POINT pt;
		pt.x = 8;
		pt.y = 8;
		dragImage = list.CreateDragImage(dragIndex, &pt);
		dragImage->BeginDrag(0, CPoint (8, 8));
		dragImage->DragEnter(GetDesktopWindow(), pNMLV->ptAction);

		//Turn the owner draw back on
		list.ModifyStyle(0, LVS_OWNERDRAWFIXED);

		//Drag is in progress
		bDragging = true;
		dropIndex = -1;
		dropWnd = &list;

		//Capture the messages
		SetCapture();
	}

	*pResult = 0;
}

/** 
* Finishes a stage drag operation of the user was dragging a stage.
*/
void StageView::OnLButtonUp(UINT nFlags, CPoint point) {
	if( bDragging ) {
		//Release mouse capture
		ReleaseCapture();

		//Delete the drag image
		dragImage->DragLeave(GetDesktopWindow());
		dragImage->EndDrag();

		//Where did we drop
		CPoint pt(point);
		ClientToScreen(&pt);
		dropWnd = WindowFromPoint(pt);

		if( dropWnd->IsKindOf(RUNTIME_CLASS(StageView)) )
			DropItemOnList();

		bDragging = false;
	}

	ToggleListView::OnLButtonUp(nFlags, point);
}

/** 
* Handles drawing the drag image when a user is draging a stage.
*/
void StageView::OnMouseMove(UINT nFlags, CPoint point) {
	if( bDragging ) {
		dropPoint = point;
		ClientToScreen(&dropPoint);

		//Move the drag image
		dragImage->DragMove(dropPoint);
		dragImage->DragShowNolock(FALSE);

		dropWnd = WindowFromPoint(dropPoint);
		dropWnd->ScreenToClient(&dropPoint);

		dragImage->DragShowNolock(TRUE);
	}
	ToggleListView::OnMouseMove(nFlags, point);
}

/** 
* Displays the popup menu when the user performs a right mouse click.
*/
void StageView::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult) {
	if(materialDocManager->GetCurrentMaterialDoc()) {
		CListCtrl& list = GetListCtrl();

		DWORD dwPos = GetMessagePos();

		CPoint pt( LOWORD( dwPos ), HIWORD ( dwPos ) );

		CPoint spt = pt;
		list.ScreenToClient( &spt );

		PopupMenu(&spt);
	}
	*pResult = 0;
}

/** 
* Begins a label edit when the user selects the rename menu option.
*/
void StageView::OnRenameStage() {

	CListCtrl& list = GetListCtrl();
	POSITION pos = list.GetFirstSelectedItemPosition();
	int nItem = -1;
	if(pos) {
		nItem = list.GetNextSelectedItem(pos);
		list.EditLabel(nItem);
	}
}

/** 
* Deletes the selected stage when the user selects the delete menu option.
*/
void StageView::OnDeleteStage() {

	CListCtrl& list = GetListCtrl();
	POSITION pos = list.GetFirstSelectedItemPosition();
	int nItem = -1;
	if(pos) {
		nItem = list.GetNextSelectedItem(pos);
		if(nItem > 0) {
			int result = MessageBox("Are you sure you want to delete this stage?", "Delete?", MB_ICONQUESTION | MB_YESNO);
			if(result == IDYES) {

				MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
				material->RemoveStage(nItem-1);
			}
		}
	}
}

/** 
* Conforms the user wants to delete all stages and then performs the operation.
*/
void StageView::OnDeleteAllStages() {
	int result = MessageBox("Are you sure you want to delete all stages?", "Delete?", MB_ICONQUESTION | MB_YESNO);
	if(result == IDYES) {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		material->ClearStages();
	}
}

/** 
* Adds a new stage when the user selects the menu option.
*/
void StageView::OnAddStage() {
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();

	idStr name = va("Stage %d", material->GetStageCount()+1);
	material->AddStage(MaterialDoc::STAGE_TYPE_NORMAL, name.c_str());
}

/** 
* Adds a new bumpmap stage when the user selects the menu option.
*/
void StageView::OnAddBumpmapStage() {
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	material->AddStage(MaterialDoc::STAGE_TYPE_SPECIALMAP, "bumpmap");
}

/** 
* Adds a new diffusemap stage when the user selects the menu option.
*/
void StageView::OnAddDiffuseStage() {
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	material->AddStage(MaterialDoc::STAGE_TYPE_SPECIALMAP, "diffusemap");
}

/** 
* Adds a new specularmap stage when the user selects the menu option.
*/
void StageView::OnAddSpecualarStage() {
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	material->AddStage(MaterialDoc::STAGE_TYPE_SPECIALMAP, "specularmap");
}

/** 
* Performs a copy operation when the user selects the menu option.
*/
void StageView::OnCopy() {

	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();

	CListCtrl& list = GetListCtrl();

	POSITION pos = list.GetFirstSelectedItemPosition();
	int nItem = -1;
	if(pos)
		nItem = list.GetNextSelectedItem(pos);

	if(nItem > 0) {
		materialDocManager->CopyStage(material, nItem-1);
	}
}

/** 
* Performs a paste operation when the user selects the menu option.
*/
void StageView::OnPaste() {
	if(materialDocManager->IsCopyStage()) {

		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();

		int type;
		idStr name;

		materialDocManager->GetCopyStageInfo(type, name);

		int existingIndex = material->FindStage(type, name);

		if(type != MaterialDoc::STAGE_TYPE_SPECIALMAP || existingIndex == -1) {
			materialDocManager->PasteStage(material);
		} else {
			if(MessageBox(va("Do you want to replace '%s' stage?", name.c_str()), "Replace?", MB_ICONQUESTION | MB_YESNO) == IDYES) {
				material->RemoveStage(existingIndex);
				materialDocManager->PasteStage(material);
			}
		}
	}
}

/** 
* Determines is a label edit can be performed on the selected stage.
*/
void StageView::OnLvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult) {
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	//if this is a special stage then don't allow edit
	int index = pDispInfo->item.iItem;

	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	if(index <= 0 || material->GetAttributeInt(index-1, "stagetype") != MaterialDoc::STAGE_TYPE_NORMAL)
	{
		*pResult = 1;
		return;
	}

	//ToDo: Can we move the edit box
	/*HWND edit = ListView_GetEditControl(m_hWnd);
	CWnd* editWnd = CWnd::FromHandle(edit);
	CRect rect;
	editWnd->GetWindowRect(rect);
	rect.left += 22;
	rect.right += 22;
	editWnd->MoveWindow(rect);*/


	*pResult = 0;
}

/** 
* Performs the stage name change after the label edit is done.
*/
void StageView::OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult) {
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if(pDispInfo->item.pszText) {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		internalChange = true;
		material->SetAttribute(pDispInfo->item.iItem-1, "name", pDispInfo->item.pszText);
		internalChange = false;
		*pResult = 1;
	} else {
		*pResult = 0;
	}
}

/** 
* Handles keyboard shortcuts for copy and paste operations.
*/
void StageView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if(nChar == 3 && GetKeyState(VK_CONTROL)) {
		OnCopy();
	}

	if(nChar == 22 && GetKeyState(VK_CONTROL)) {
		OnPaste();
	}


	ToggleListView::OnChar(nChar, nRepCnt, nFlags);
}

/** 
* Handles keyboard shortcut for the delete operations.
*/
BOOL StageView::PreTranslateMessage(MSG* pMsg) {

	CListCtrl& list = GetListCtrl();
	if (pMsg->hwnd == list.GetSafeHwnd()) {

		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE) {
			OnDeleteStage();
			return TRUE;
		}
	}
	return FALSE;
}

/** 
* Sets window styles before the window is created.
*/
BOOL StageView::PreCreateWindow(CREATESTRUCT& cs) {
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= LVS_SINGLESEL | LVS_EDITLABELS;

	return ToggleListView::PreCreateWindow(cs);
}

/** 
* Called by the ToggleListView when the toggle state has changed.
*/
void StageView::OnStateChanged(int index, int toggleState) {
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	if(material && index > 0) {
		if (toggleState == ToggleListView::TOGGLE_STATE_ON) {
			material->EnableStage(index-1, true);
		} else if (toggleState == ToggleListView::TOGGLE_STATE_OFF) {
			material->EnableStage(index-1, false);
		}
	}
}

/** 
* Dispalys the popup menu with the appropriate menu items enabled.
*/
void StageView::PopupMenu(CPoint* pt) {

	//Determine the type of object clicked on
	CListCtrl& list = GetListCtrl();


	ClientToScreen (pt);

	CMenu FloatingMenu;
	VERIFY(FloatingMenu.LoadMenu(IDR_ME_STAGELIST_POPUP));
	CMenu* pPopupMenu = FloatingMenu.GetSubMenu (0);
	ASSERT(pPopupMenu != NULL);

	POSITION pos = list.GetFirstSelectedItemPosition();
	int nItem = -1;
	if(pos)
		nItem = list.GetNextSelectedItem(pos);

	if(nItem <= 0) {
		pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_RENAMESTAGE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_DELETESTAGE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_CUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_COPY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	} else {
		MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
		if(material->GetAttributeInt(nItem-1, "stagetype") != MaterialDoc::STAGE_TYPE_NORMAL) {
			pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_RENAMESTAGE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
	}

	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	if(material->FindStage(MaterialDoc::STAGE_TYPE_SPECIALMAP, "bumpmap") >= 0) {
		pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_ADDBUMPMAP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	if(material->FindStage(MaterialDoc::STAGE_TYPE_SPECIALMAP, "diffusemap") >= 0) {
		pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_ADDDIFFUSEMAP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	if(material->FindStage(MaterialDoc::STAGE_TYPE_SPECIALMAP, "specularmap") >= 0) {
		pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_ADDSPECULAR, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	if(materialDocManager->IsCopyStage()) {
		pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_PASTE, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_STAGEPOPUP_PASTE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	pPopupMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt->x, pt->y, &list);
}

/** 
* Performs the stage move when the user has dragged and dropped a stage.
*/
void StageView::DropItemOnList() {
	CListCtrl& list = GetListCtrl();

	int toStage;

	//Get and adjust the drop index based on the direction of the move
	dropIndex = list.HitTest(dropPoint);
	if(dropIndex < 0) dropIndex = list.GetItemCount()-1;

	//Ignore the drop if the index is the same or they are droping on the material
	if(dropIndex == dragIndex || dropIndex == 0)
		return;

	//Move the stage data
	MaterialDoc* material = materialDocManager->GetCurrentMaterialDoc();
	
	internalChange = true;
	toStage = dropIndex-1;
	material->MoveStage(dragIndex-1, dropIndex-1);
	internalChange = false;
		
	if(dragIndex < dropIndex) {
		dropIndex++;
	}

	//Get the item
	char szLabel[256];
	LV_ITEM lvi;
	ZeroMemory(&lvi, sizeof(LV_ITEM));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
	lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
	lvi.pszText = szLabel;
	lvi.iItem = dragIndex;
	lvi.cchTextMax = 255;
	list.GetItem(&lvi);

	//Insert the item
	lvi.iItem = dropIndex;
	list.InsertItem(&lvi);

	//Adjust the drag index if the move was up in the list
	if(dragIndex > dropIndex) {
		dragIndex++;
	}

	//Delete the original item
	list.DeleteItem(dragIndex);

	int type = -1;
	int stageType = currentMaterial->GetAttributeInt(toStage, "stagetype");
	switch(stageType) {
		case MaterialDoc::STAGE_TYPE_NORMAL:
			type = MaterialDefManager::MATERIAL_DEF_STAGE;
			break;
		case MaterialDoc::STAGE_TYPE_SPECIALMAP:
			type = MaterialDefManager::MATERIAL_DEF_SPECIAL_STAGE;
			break;
	}
	m_propView->SetPropertyListType(type, toStage);
}





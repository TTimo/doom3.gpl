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

#include "MaterialEditor.h"
#include "MEMainFrame.h"
#include "MaterialDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TAB_CONTROL 0x1006

IMPLEMENT_DYNAMIC(MEMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(MEMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_SIZE()

	ON_NOTIFY(TCN_SELCHANGE, TAB_CONTROL, OnTcnSelChange)
	
	ON_COMMAND(ID_ME_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_ME_FILE_SAVEMATERIAL, OnFileSaveMaterial)
	ON_COMMAND(ID_ME_FILE_SAVEFILE, OnFileSaveFile)
	ON_COMMAND(ID_ME_FILE_SAVE, OnFileSaveAll)
	ON_UPDATE_COMMAND_UI(ID_ME_FILE_SAVEMATERIAL, OnFileSaveMaterialUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_FILE_SAVEFILE, OnFileSaveFileUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_FILE_SAVE, OnFileSaveAllUpdate )
	
	ON_COMMAND(ID_ME_PREVIEW_APPLYCHANGES, OnApplyMaterial)
	ON_COMMAND(ID_ME_PREVIEW_APPLYFILE, OnApplyFile)
	ON_COMMAND(ID_ME_PREVIEW_APPLYALL, OnApplyAll)
	ON_UPDATE_COMMAND_UI(ID_ME_PREVIEW_APPLYCHANGES, OnApplyMaterialUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_PREVIEW_APPLYFILE, OnApplyFileUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_PREVIEW_APPLYALL, OnApplyAllUpdate )

	ON_COMMAND(ID_ME_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_ME_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_ME_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_ME_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_ME_EDIT_RENAME, OnEditRename)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_CUT, OnEditCutUpdate)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_COPY, OnEditCopyUpdate)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_PASTE, OnEditPasteUpdate)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_DELETE, OnEditDeleteUpdate)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_RENAME, OnEditRenameUpdate)

	ON_COMMAND(ID_ME_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_ME_EDIT_FIND_NEXT, OnEditFindNext)

	ON_COMMAND(ID_ME_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_ME_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_UNDO, OnEditUndoUpdate )
	ON_UPDATE_COMMAND_UI(ID_ME_EDIT_REDO, OnEditRedoUpdate )
	
	ON_COMMAND(ID_VIEW_INCLUDEFILENAME, OnViewIncludeFile)
	ON_COMMAND(ID_PREVIEW_RELOADARBPROGRAMS, OnReloadArbPrograms)
	ON_COMMAND(ID_PREVIEW_RELOADIMAGES, OnReloadImages )	
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/**
* Constructor for MEMainFrame. Initialize some member data and load the options.
*/
MEMainFrame::MEMainFrame() {
	
	currentDoc = NULL;
	m_find = NULL;

	searchData.searched = false;

	options.Load();
}

/**
* Destructor for MEMainFrame.
*/
MEMainFrame::~MEMainFrame() {
}

/**
* Called to add console text to the console view.
* @param msg The text that is to be added to the console.
*/
void MEMainFrame::PrintConsoleMessage(const char *msg) {
	m_consoleView->AddText(msg);
}

/**
* Sets a few window styles for the main window during the creation process.
*/
BOOL MEMainFrame::PreCreateWindow(CREATESTRUCT& cs) {
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	
	return TRUE;
}

/**
* Called by the MFC framework to allow the window to create any client windows. This method
* creates all of the spliter windows and registers all of the views with the document manager.
*/
BOOL MEMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) {
	CCreateContext consoleContext;
	consoleContext.m_pNewViewClass = RUNTIME_CLASS(ConsoleView);

	m_consoleView = (ConsoleView*)CreateView(&consoleContext);
	m_consoleView->ShowWindow(SW_HIDE);

	m_tabs.Create(TCS_BOTTOM | TCS_FLATBUTTONS | WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, TAB_CONTROL);
	m_tabs.InsertItem(0, "Editor");
	m_tabs.InsertItem(1, "Console");
	m_tabs.SetFont(materialEditorFont);

	m_splitterWnd.CreateStatic(this, 2, 1);

	
	m_editSplitter.CreateStatic(&m_splitterWnd, 1, 2, WS_CHILD | WS_VISIBLE | WS_BORDER, m_splitterWnd.IdFromRowCol(0, 0));

	if(!m_editSplitter.CreateView(0, 0, RUNTIME_CLASS(MaterialTreeView), CSize(300, 200), pContext)) {
		TRACE0("Failed to create material list pane\n");
		return FALSE;
	}

	if(!m_editSplitter.CreateView(0, 1, RUNTIME_CLASS(MaterialEditView), CSize(200, 200), pContext)) {
		TRACE0("Failed to create stage property pane\n");
		return FALSE;
	}


	m_previewSplitter.CreateStatic(&m_splitterWnd, 1, 2, WS_CHILD | WS_VISIBLE | WS_BORDER, m_splitterWnd.IdFromRowCol(1, 0));

	if(!m_previewSplitter.CreateView(0, 0, RUNTIME_CLASS(MaterialPreviewPropView), CSize(300, 200), pContext)) {
		TRACE0("Failed to create preview property pane\n");
		return FALSE;
	}

	if(!m_previewSplitter.CreateView(0, 1, RUNTIME_CLASS(MaterialPreviewView), CSize(100, 200), pContext)) {
		TRACE0("Failed to create preview pane\n");
		return FALSE;
	}

	//Get references to all of the views
	m_materialTreeView = (MaterialTreeView*)m_editSplitter.GetPane(0, 0);
	m_previewPropertyView = (MaterialPreviewPropView*)m_previewSplitter.GetPane(0, 0);
	m_materialPreviewView = (MaterialPreviewView*)m_previewSplitter.GetPane(0, 1);

	m_materialEditView = (MaterialEditView*)m_editSplitter.GetPane(0, 1);
	m_stageView = m_materialEditView->m_stageView;
	m_materialPropertyView = m_materialEditView->m_materialPropertyView;
	m_materialEditSplitter = &m_materialEditView->m_editSplitter;

	//Load the splitter positions from the registry
	int val = options.GetMaterialEditHeight();
	if(val <= 0)
		val = 300;
	m_splitterWnd.SetRowInfo(0, val, 0);

	val = options.GetMaterialTreeWidth();
	if(val <= 0)
		val = 300;
	m_editSplitter.SetColumnInfo(0, val, 0);

	val = options.GetStageWidth();
	if(val <= 0)
		val = 200;
	m_materialEditSplitter->SetColumnInfo(0, val, 0);

	val = options.GetPreviewPropertiesWidth();
	if(val <= 0)
		val = 300;
	m_previewSplitter.SetColumnInfo(0, val, 0);

	

	//Register the views with the document manager
	materialDocManager.RegisterMaterialView(this);
	materialDocManager.RegisterMaterialView(m_materialTreeView);
	materialDocManager.RegisterMaterialView(m_stageView);
	materialDocManager.RegisterMaterialView(m_materialPropertyView);
	materialDocManager.RegisterMaterialView(m_materialPreviewView);
	materialDocManager.RegisterMaterialView(m_materialEditView);

	//Let the stage window know about the prop window
	m_stageView->SetMaterialPropertyView(m_materialPropertyView);

	//Let the preview props now about the preview window
	m_previewPropertyView->RegisterPreviewView(m_materialPreviewView);
	m_previewPropertyView->InitializePropTree();
	m_previewPropertyView->GetPropertyTreeCtrl().SetColumn(120);

	MaterialDefManager::InitializeMaterialDefLists();

	//Some prop tree initialization
	//m_materialPropertyView->InitializePropTreeDefs();
	val = options.GetMaterialPropHeadingWidth();
	if(val <= 0)
		val = 200;
	m_materialPropertyView->GetPropertyTreeCtrl().SetColumn(val);
	m_materialPropertyView->LoadSettings();


	val = options.GetPreviewPropHeadingWidth();
	if(val <= 0)
		val = 120;
	m_previewPropertyView->GetPropertyTreeCtrl().SetColumn(val);

	//Build the material list
	m_materialTreeView->InitializeMaterialList(true);

	SetActiveView(m_materialTreeView);

	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

/**
* Called by the framework while the window is being created. This methods
* creates the tool bars and status bars
* /todo Bmatt Nerve: Need to get the toolbars to work correctly.
*/
int MEMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP	| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_ME_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	//Load the window placement from the options
	options.GetWindowPlacement ( "mainframe", m_hWnd );

	return 0;
}

/**
* Called by the MFC framework while the window is being destroyed. This method
* saves the splitter and window positions.
*/
void MEMainFrame::OnDestroy() {
	CFrameWnd::OnDestroy();

	int cur;
	int min;

	m_splitterWnd.GetRowInfo(0, cur, min);
	options.SetMaterialEditHeight(cur);

	m_editSplitter.GetColumnInfo(0, cur, min);
	options.SetMaterialTreeWidth(cur);

	m_materialEditSplitter->GetColumnInfo(0, cur, min);
	options.SetStageWidth(cur);

	m_previewSplitter.GetColumnInfo(0, cur, min);
	options.SetPreviewPropertiesWidth(cur);


	cur = m_materialPropertyView->GetPropertyTreeCtrl().GetColumn();
	options.SetMaterialPropHeadingWidth(cur);

	cur = m_previewPropertyView->GetPropertyTreeCtrl().GetColumn();
	options.SetPreviewPropHeadingWidth(cur);

	options.SetWindowPlacement ( "mainframe", m_hWnd );
	options.Save();

	m_materialPropertyView->SaveSettings();

	MaterialDefManager::DestroyMaterialDefLists();

	AfxGetApp()->ExitInstance();
}

/**
* Called by the MFC framework when the window size is changed. This method adjusts the console view
* so that it is always at the bottom of the window and resizes the splitter window to fit
* the remaining space.
*/
void MEMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	CRect statusRect;
	m_wndStatusBar.GetWindowRect(statusRect);

	CRect toolbarRect;
	m_wndToolBar.GetWindowRect(toolbarRect);

	CRect tabRect;
	m_tabs.GetItemRect(0, tabRect);

	int tabHeight = tabRect.Height()+5;

	m_splitterWnd.MoveWindow(0, toolbarRect.Height(), cx, cy-statusRect.Height()-toolbarRect.Height()-tabHeight);

	m_tabs.MoveWindow(0, cy-statusRect.Height()-tabHeight, cx, tabHeight);

	m_consoleView->MoveWindow(0, toolbarRect.Height(), cx, cy-statusRect.Height()-toolbarRect.Height()-tabHeight);
}

/**
* Called when the user changes the editor/console tab selection. This methods shows and hides 
* the appropriate windows.
*/
void MEMainFrame::OnTcnSelChange(NMHDR *pNMHDR, LRESULT *pResult) {

	int sel = m_tabs.GetCurSel();

	switch(sel) {
		case 0:
			m_splitterWnd.ShowWindow(SW_SHOW);
			m_consoleView->ShowWindow(SW_HIDE);

			break;
		case 1:
			m_splitterWnd.ShowWindow(SW_HIDE);
			m_consoleView->ShowWindow(SW_SHOW);

			CRect rect;
			GetWindowRect(rect);
			MoveWindow(rect);
			break;
	}
}

/**
* Shuts down the material editor.
* /todo BMatt Nerve: Need to warn the user if a file is modified.
*/
void MEMainFrame::OnFileExit() {
	PostMessage(WM_DESTROY, 0, 0);
}

/**
* Saves the selected material.
*/
void MEMainFrame::OnFileSaveMaterial() {
	MaterialDoc* material = materialDocManager.GetCurrentMaterialDoc();
	if(material) {
		materialDocManager.SaveMaterial(material);
	}
}

/**
* Saves the selected file.
*/
void MEMainFrame::OnFileSaveFile() {
	
	idStr filename = m_materialTreeView->GetSaveFilename();
	if(filename.Length() > 0) {
		materialDocManager.SaveFile(filename);
	}
}

/**
* Saves all modified materials.
*/
void MEMainFrame::OnFileSaveAll() {
	materialDocManager.SaveAllMaterials();
}

/**
* Enables the save material menu item if a material is selected and has been modified.
*/
void MEMainFrame::OnFileSaveMaterialUpdate(CCmdUI *pCmdUI) {

	MaterialDoc* pDoc = materialDocManager.GetCurrentMaterialDoc();

	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	if(pDoc && pDoc->modified) {
		pCmdUI->Enable(TRUE);

	} else {
		pCmdUI->Enable(FALSE);
	}
}

/**
* Enables the Save File menu item if the current file contains a modified material.
*/
void MEMainFrame::OnFileSaveFileUpdate(CCmdUI *pCmdUI) {

	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	if(m_materialTreeView->CanSaveFile()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

/**
* Enables the Save All menu item if there are any materials that have been modified.
*/
void MEMainFrame::OnFileSaveAllUpdate(CCmdUI *pCmdUI) {
	
	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	if(materialDocManager.IsAnyModified()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

/**
* Apply the selected material.
*/
void MEMainFrame::OnApplyMaterial() {
	MaterialDoc* material = materialDocManager.GetCurrentMaterialDoc();
	if(material) {
		materialDocManager.ApplyMaterial(material);
	}
}

/**
* Applies all modified materials in the selected file.
*/
void MEMainFrame::OnApplyFile() {
	
	idStr filename = m_materialTreeView->GetSaveFilename();
	if(filename.Length() > 0) {
		materialDocManager.ApplyFile(filename);
	}
}

/**
* Applies all modified materials.
*/
void MEMainFrame::OnApplyAll() {
	materialDocManager.ApplyAll();
}

/**
* Enables the Apply Material menu item if the current material has an apply waiting.
*/
void MEMainFrame::OnApplyMaterialUpdate(CCmdUI *pCmdUI) {
	MaterialDoc* pDoc = materialDocManager.GetCurrentMaterialDoc();

	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	if(pDoc && pDoc->applyWaiting) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

/**
* Enables the apply file menu item if the current file contains any materials
* that need to be applied.
*/
void MEMainFrame::OnApplyFileUpdate(CCmdUI *pCmdUI) {

	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	MaterialDoc* pDoc = materialDocManager.GetCurrentMaterialDoc();

	if(pDoc && materialDocManager.DoesFileNeedApply(pDoc->renderMaterial->GetFileName())) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

/**
* Enables the apply all menu item if there are any materials that need
* to be applied.
*/
void MEMainFrame::OnApplyAllUpdate(CCmdUI *pCmdUI) {
	
	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}
	
	if(materialDocManager.DoesAnyNeedApply()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

/**
* Performs a cut operation on the selected material.
*/
void MEMainFrame::OnEditCut() {
	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			m_materialTreeView->OnCut();
		}
	}
}

/**
* Performs a copy operation on the selected material or stage.
*/
void MEMainFrame::OnEditCopy() {
	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(StageView))) {
			m_stageView->OnCopy();
		} else if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			m_materialTreeView->OnCopy();
		}
	}
}

/**
* Performs a paste operation on the selected material or stage.
*/
void MEMainFrame::OnEditPaste() {
	CWnd* focus = GetFocus();

	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(StageView))) {
			m_stageView->OnPaste();
		} else if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			m_materialTreeView->OnPaste();
		}
	}
}

/**
* Performs a delete operation on the selected material or stage.
*/
void MEMainFrame::OnEditDelete() {
	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(StageView))) {
			m_stageView->OnDeleteStage();
		} else if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			m_materialTreeView->OnDeleteMaterial();
		}
	}
}

/**
* Performs a rename operation on the selected material or stage.
*/
void MEMainFrame::OnEditRename() {
	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(StageView))) {
			m_stageView->OnRenameStage();
		} else if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			m_materialTreeView->OnRenameMaterial();
		}
	}
}


/**
* Enable the cut menu item if a material is selected.
*/
void MEMainFrame::OnEditCutUpdate(CCmdUI *pCmdUI) {

	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	BOOL enable = FALSE;

	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(StageView))) {
			if(m_stageView->CanCut()) {
				enable = TRUE;
			}
		} else if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			if(m_materialTreeView->CanCut()) {
				enable = TRUE;
			}
		} else if (focus->IsKindOf(RUNTIME_CLASS(CRichEditCtrl))) {
			enable = TRUE;
		}
	}

	pCmdUI->Enable(enable);
}

/**
* Enables the copy menu item if a material or stage is selected.
*/
void MEMainFrame::OnEditCopyUpdate(CCmdUI *pCmdUI) {

	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	BOOL enable = FALSE;

	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(StageView))) {
			if(m_stageView->CanCopy()) {
				enable = TRUE;
			}
		} else if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			if(m_materialTreeView->CanCopy()) {
				enable = TRUE;
			}
		} else if (focus->IsKindOf(RUNTIME_CLASS(CRichEditCtrl))) {
			enable = TRUE;
		}
	}

	pCmdUI->Enable(enable);
}

/**
* Enables a paste operation when a material or stage has been copied.
*/
void MEMainFrame::OnEditPasteUpdate(CCmdUI *pCmdUI) {

	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	BOOL enable = FALSE;

	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(StageView))) {
			if(m_stageView->CanPaste()) {
				enable = TRUE;
			}
		} else if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			if(m_materialTreeView->CanPaste()) {
				enable = TRUE;
			}
		} else if (focus->IsKindOf(RUNTIME_CLASS(CRichEditCtrl))) {
			enable = TRUE;
		}
	}

	pCmdUI->Enable(enable);
}

/**
* Enables a delete operation when a material or stage is selected.
*/
void MEMainFrame::OnEditDeleteUpdate(CCmdUI *pCmdUI) {

	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	BOOL enable = FALSE;

	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(StageView))) {
			if(m_stageView->CanDelete()) {
				enable = TRUE;
			}
		} else if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			if(m_materialTreeView->CanDelete()) {
				enable = TRUE;
			}
		} else if (focus->IsKindOf(RUNTIME_CLASS(CRichEditCtrl))) {
			enable = TRUE;
		}

	}

	pCmdUI->Enable(enable);
}

/**
* Enables a rename operation when a material, folder or stage is selected.
*/
void MEMainFrame::OnEditRenameUpdate(CCmdUI *pCmdUI) {

	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	BOOL enable = FALSE;

	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(StageView))) {
			if(m_stageView->CanRename()) {
				enable = TRUE;
			}
		} else if (focus->IsKindOf(RUNTIME_CLASS(MaterialTreeView))) {
			if(m_materialTreeView->CanRename()) {
				enable = TRUE;
			}
		}
	}
	pCmdUI->Enable(enable);
}

/**
* Opens the find dialog.
*/
void MEMainFrame::OnEditFind() {

	if (m_find== NULL)
	{
		m_find = new FindDialog(this);
		m_find->Create();
		m_find->ShowWindow(SW_SHOW);
	}
	else
		m_find->SetActiveWindow();
}

/**
* Performs a search with the previously selected search parameters.
*/
void MEMainFrame::OnEditFindNext() {
	FindNext(NULL);
}

/**
* Performs an undo operation.
*/
void MEMainFrame::OnEditUndo() {
	
	//Check for undo operation on special windows
	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(CRichEditCtrl))) {
			m_materialEditView->m_textView.Undo();
			return;
		}
	}

	materialDocManager.Undo();
}

/**
* Performs a redo operation.
*/
void MEMainFrame::OnEditRedo() {
	
	//Check for redo operation on special windows
	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(CRichEditCtrl))) {
			m_materialEditView->m_textView.Redo();
			return;
		}
	}

	materialDocManager.Redo();
}

/**
* Enables the undo menu item if an undo is available.
*/
void MEMainFrame::OnEditUndoUpdate(CCmdUI *pCmdUI) {
	
	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(CRichEditCtrl))) {
			pCmdUI->Enable(m_materialEditView->m_textView.CanUndo());
			return;
		}
	}

	pCmdUI->Enable(materialDocManager.IsUndoAvailable());
}

/**
* Enables the redo menu item if a redo is available.
*/
void MEMainFrame::OnEditRedoUpdate(CCmdUI *pCmdUI) {
	
	if(pCmdUI->m_pMenu == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	CWnd* focus = GetFocus();
	if(focus) {
		if (focus->IsKindOf(RUNTIME_CLASS(CRichEditCtrl))) {
			pCmdUI->Enable(m_materialEditView->m_textView.CanRedo());
			return;
		}
	}

	pCmdUI->Enable(materialDocManager.IsRedoAvailable());
}

/**
* Toggles between including the file into the material list and not.
*/
void MEMainFrame::OnViewIncludeFile() {
	
	CMenu* mmenu = GetMenu();
	
	UINT state = mmenu->GetMenuState(ID_VIEW_INCLUDEFILENAME, MF_BYCOMMAND);
	ASSERT(state != 0xFFFFFFFF);

	if (state & MF_CHECKED) {
		mmenu->CheckMenuItem(ID_VIEW_INCLUDEFILENAME, MF_UNCHECKED | MF_BYCOMMAND);
		m_materialTreeView->InitializeMaterialList(false);
	} else {
		mmenu->CheckMenuItem(ID_VIEW_INCLUDEFILENAME, MF_CHECKED | MF_BYCOMMAND);
		m_materialTreeView->InitializeMaterialList(true);
	}
}

/**
* Executes the reloadARBPrograms console command for convinience.
*/
void MEMainFrame::OnReloadArbPrograms() {
	cmdSystem->BufferCommandText(CMD_EXEC_NOW, "reloadARBprograms");
}

/**
* Executes the reloadImages command to reload images that have been changed outside
* of the editor.
*/
void MEMainFrame::OnReloadImages() {
	cmdSystem->BufferCommandText(CMD_EXEC_NOW, "reloadImages");	
}

/**
* Called by the find dialog when it is closing.
*/
void MEMainFrame::CloseFind() {
	m_find = NULL;
}

/**
* Begins a search based on the provided parameters or the previously used
* parameters.
*/
void MEMainFrame::FindNext(MaterialSearchData_t* search) {

	if(search) {
		searchData = *search;
	} else {
		if(!searchData.searched) {
			return;
		}
	}

	//The material tree controls the searching
	if(!m_materialTreeView->FindNextMaterial(&searchData)) {
		MessageBox(va("Unable to find '%s'.", searchData.searchText.c_str()), "Find");
	}
	searchData.searched = true;
}

/**
* Called when the selected material has changed.
* @param pMaterial The newly selected material.
*/
void MEMainFrame::MV_OnMaterialSelectionChange(MaterialDoc* pMaterial) {
	
}

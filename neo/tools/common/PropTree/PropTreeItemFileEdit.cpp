// PropTreeItemFileEdit.cpp : implementation file


//#include "stdafx.h"
#include "../../../idlib/precompiled.h"
#pragma hdrstop

#include "proptree.h"
#include "PropTreeItemFileEdit.h"

#include "../../../sys/win32/rc/proptree_Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemFileEdit

CPropTreeItemFileEdit::CPropTreeItemFileEdit() {
}

CPropTreeItemFileEdit::~CPropTreeItemFileEdit() {
}


BEGIN_MESSAGE_MAP(CPropTreeItemFileEdit, CPropTreeItemEdit)
	//{{AFX_MSG_MAP(CPropTreeItemFileEdit)
	//}}AFX_MSG_MAP
	ON_WM_CONTEXTMENU()
	ON_WM_CREATE()
	
	ON_COMMAND(ID_EDITMENU_INSERTFILE, OnInsertFile)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectAll)

END_MESSAGE_MAP()


void CPropTreeItemFileEdit::OnContextMenu(CWnd* pWnd, CPoint point) {

	CMenu FloatingMenu;
	VERIFY(FloatingMenu.LoadMenu(IDR_ME_EDIT_MENU));
	CMenu* pPopupMenu = FloatingMenu.GetSubMenu (0);

	if(CanUndo()) {
		pPopupMenu->EnableMenuItem(ID_EDIT_UNDO, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_EDIT_UNDO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	DWORD dwSel = GetSel();
	if(HIWORD(dwSel) != LOWORD(dwSel)) {
		pPopupMenu->EnableMenuItem(ID_EDIT_CUT, MF_BYCOMMAND | MF_ENABLED);
		pPopupMenu->EnableMenuItem(ID_EDIT_COPY, MF_BYCOMMAND | MF_ENABLED);
		pPopupMenu->EnableMenuItem(ID_EDIT_DELETE, MF_BYCOMMAND | MF_ENABLED);
	} else {
		pPopupMenu->EnableMenuItem(ID_EDIT_CUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_EDIT_COPY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_EDIT_DELETE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	pPopupMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}

int CPropTreeItemFileEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPropTreeItemEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

void CPropTreeItemFileEdit::OnInsertFile() {
	CFileDialog dlg(TRUE);
	dlg.m_ofn.Flags |= OFN_FILEMUSTEXIST;
	
	int startSel, endSel;
	GetSel(startSel, endSel);

	if( dlg.DoModal()== IDOK) {
		
		idStr currentText = (char*)GetItemValue();
		idStr newText = currentText.Left(startSel) + currentText.Right(currentText.Length() - endSel);
		
		idStr filename = fileSystem->OSPathToRelativePath(dlg.m_ofn.lpstrFile);
		filename.BackSlashesToSlashes();

		
		newText.Insert(filename, startSel);

		SetItemValue((LPARAM)newText.c_str());
		m_pProp->RefreshItems(this);

		m_pProp->SendNotify(PTN_ITEMCHANGED, this);
		
	}
}

void CPropTreeItemFileEdit::OnEditUndo() {
	Undo();
}

void CPropTreeItemFileEdit::OnEditCut() {
	Cut();
}

void CPropTreeItemFileEdit::OnEditCopy() {
	Copy();
}

void CPropTreeItemFileEdit::OnEditPaste() {
	Paste();
}

void CPropTreeItemFileEdit::OnEditDelete() {
	Clear();
}

void CPropTreeItemFileEdit::OnEditSelectAll() {
	SetSel(0, -1);
}

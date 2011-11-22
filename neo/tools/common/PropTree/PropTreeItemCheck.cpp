// PropTreeItemCheck.cpp : implementation file
//
//  Copyright (C) 1998-2001 Scott Ramsay
//	sramsay@gonavi.com
//	http://www.gonavi.com
//
//  This material is provided "as is", with absolutely no warranty expressed
//  or implied. Any use is at your own risk.
// 
//  Permission to use or copy this software for any purpose is hereby granted 
//  without fee, provided the above notices are retained on all copies.
//  Permission to modify the code and to distribute modified code is granted,
//  provided the above notices are retained, and a notice that the code was
//  modified is included with the above copyright notice.
// 
//	If you use this code, drop me an email.  I'd like to know if you find the code
//	useful.

//#include "stdafx.h"
#include "../../../idlib/precompiled.h"
#pragma hdrstop

#include "proptree.h"
#include "PropTreeItemCheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CHECK_BOX_SIZE 14

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemCheck

CPropTreeItemCheck::CPropTreeItemCheck()
{
	checkState = 0;
}

CPropTreeItemCheck::~CPropTreeItemCheck()
{
}


BEGIN_MESSAGE_MAP(CPropTreeItemCheck, CButton)
	//{{AFX_MSG_MAP(CPropTreeItemCheck)
	//}}AFX_MSG_MAP
	ON_CONTROL_REFLECT(BN_KILLFOCUS, OnBnKillfocus)
	ON_CONTROL_REFLECT(BN_CLICKED, OnBnClicked)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemCheck message handlers

void CPropTreeItemCheck::DrawAttribute(CDC* pDC, const RECT& rc)
{
	ASSERT(m_pProp!=NULL);

	// verify the window has been created
	if (!IsWindow(m_hWnd))
	{
		TRACE0("CPropTreeItemCombo::DrawAttribute() - The window has not been created\n");
		return;
	}

	checkRect.left = m_rc.left;
	checkRect.top = m_rc.top + ((m_rc.bottom - m_rc.top)/2)-CHECK_BOX_SIZE/2;
	checkRect.right = checkRect.left + CHECK_BOX_SIZE;
	checkRect.bottom = checkRect.top + CHECK_BOX_SIZE;

	if(!m_bActivated)
		pDC->DrawFrameControl(&checkRect, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT |(checkState ? DFCS_CHECKED : 0));
}

void CPropTreeItemCheck::SetCheckState(BOOL state)
 { 
	 checkState = state; 
	 
	 SetCheck(checkState ? BST_CHECKED : BST_UNCHECKED);
 }


LPARAM CPropTreeItemCheck::GetItemValue()
{
	return (LPARAM)GetCheckState();
}


void CPropTreeItemCheck::SetItemValue(LPARAM lParam)
{
	SetCheckState((BOOL)lParam);
}


void CPropTreeItemCheck::OnMove()
{
	if (IsWindow(m_hWnd))
		SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), SWP_NOZORDER|SWP_NOACTIVATE);
}


void CPropTreeItemCheck::OnRefresh()
{
}


void CPropTreeItemCheck::OnCommit()
{
	ShowWindow(SW_HIDE);
}


void CPropTreeItemCheck::OnActivate(int activateType, CPoint point)
{
	if(activateType == CPropTreeItem::ACTIVATE_TYPE_MOUSE) {
		//Check where the user clicked
		if(point.x < m_rc.left + CHECK_BOX_SIZE) {
			SetCheckState(!GetCheckState());
			CommitChanges();
		} else {
			SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), SWP_NOZORDER|SWP_SHOWWINDOW);
			SetFocus();
		}
	} else {
		SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), SWP_NOZORDER|SWP_SHOWWINDOW);
		SetFocus();
	}
}


bool CPropTreeItemCheck::CreateCheckBox() {
	ASSERT(m_pProp!=NULL);

	if (IsWindow(m_hWnd))
		DestroyWindow();

	DWORD dwStyle = (WS_CHILD|BS_CHECKBOX|BS_NOTIFY|BS_FLAT );

	if (!Create(NULL, dwStyle, CRect(0,0,0,0), m_pProp->GetCtrlParent(), GetCtrlID()))
	{
		TRACE0("CPropTreeItemCombo::CreateComboBox() - failed to create combo box\n");
		return FALSE;
	}

	return TRUE;
}

void CPropTreeItemCheck::OnBnKillfocus()
{
	CommitChanges();
}

void CPropTreeItemCheck::OnBnClicked()
{
	int state = GetCheck();

	SetCheckState(GetCheck() == BST_CHECKED ? FALSE : TRUE);
	CommitChanges();
}

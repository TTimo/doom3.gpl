// PropTreeItemCombo.cpp : implementation file
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

#include "PropTree.h"
#include "../../../sys/win32/rc/proptree_Resource.h"

#include "PropTreeItemCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DROPDOWN_HEIGHT			100

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemCombo

CPropTreeItemCombo::CPropTreeItemCombo() :
	m_lComboData(0),
	m_nDropHeight(DROPDOWN_HEIGHT)
{
}

CPropTreeItemCombo::~CPropTreeItemCombo()
{
}


BEGIN_MESSAGE_MAP(CPropTreeItemCombo, CComboBox)
	//{{AFX_MSG_MAP(CPropTreeItemCombo)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelchange)
	ON_CONTROL_REFLECT(CBN_KILLFOCUS, OnKillfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemCombo message handlers

void CPropTreeItemCombo::DrawAttribute(CDC* pDC, const RECT& rc)
{
	ASSERT(m_pProp!=NULL);

	// verify the window has been created
	if (!IsWindow(m_hWnd))
	{
		TRACE0("CPropTreeItemCombo::DrawAttribute() - The window has not been created\n");
		return;
	}

	pDC->SelectObject(IsReadOnly() ? m_pProp->GetNormalFont() : m_pProp->GetBoldFont());
	pDC->SetTextColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);

	CRect r = rc;
	CString s;
	LONG idx;

	if ((idx = GetCurSel())!=CB_ERR)
		GetLBText(idx, s);
	else
		s = _T("");

	pDC->DrawText(s, r, DT_SINGLELINE|DT_VCENTER);
}


LPARAM CPropTreeItemCombo::GetItemValue()
{
	return m_lComboData;
}


void CPropTreeItemCombo::SetItemValue(LPARAM lParam)
{
	m_lComboData = lParam;
	OnRefresh();
}


void CPropTreeItemCombo::OnMove()
{
	if (IsWindow(m_hWnd) && IsWindowVisible())
		SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width() + 1, m_rc.Height(), SWP_NOZORDER|SWP_SHOWWINDOW);
}


void CPropTreeItemCombo::OnRefresh()
{
	LONG idx = FindCBData(m_lComboData);

	if (idx!=CB_ERR)
		SetCurSel(idx);
}


void CPropTreeItemCombo::OnCommit()
{
	LONG idx;
	
	// store combo box item data
	if ((idx = GetCurSel())==CB_ERR)
		m_lComboData = 0;
	else
		m_lComboData = (LPARAM)GetItemData(idx);

	ShowWindow(SW_HIDE);
}


void CPropTreeItemCombo::OnActivate(int activateType, CPoint point)
{
	// activate the combo box
	SetWindowPos(NULL, m_rc.left, m_rc.top, m_rc.Width() + 1, m_rc.Height() + m_nDropHeight, SWP_NOZORDER|SWP_SHOWWINDOW);
	SetFocus();

	if (GetCount())
		ShowDropDown(TRUE);
}


BOOL CPropTreeItemCombo::CreateComboBox(DWORD dwStyle)
{
	ASSERT(m_pProp!=NULL);

	if (IsWindow(m_hWnd))
		DestroyWindow();

	// force as not visible child window
	dwStyle = (WS_CHILD|WS_VSCROLL|dwStyle) & ~WS_VISIBLE;

	if (!Create(dwStyle, CRect(0,0,0,0), m_pProp->GetCtrlParent(), GetCtrlID()))
	{
		TRACE0("CPropTreeItemCombo::CreateComboBox() - failed to create combo box\n");
		return FALSE;
	}

	SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);

	return TRUE;
}


BOOL CPropTreeItemCombo::CreateComboBoxBool()
{
	ASSERT(m_pProp!=NULL);

	if (IsWindow(m_hWnd))
		DestroyWindow();

	// force as a non-visible child window
	DWORD dwStyle = WS_CHILD|WS_VSCROLL|CBS_SORT|CBS_DROPDOWNLIST;

	if (!Create(dwStyle, CRect(0,0,0,0), m_pProp->GetCtrlParent(), GetCtrlID()))
	{
		TRACE0("CPropTreeItemCombo::CreateComboBoxBool() - failed to create combo box\n");
		return FALSE;
	}

	SendMessage(WM_SETFONT, (WPARAM)m_pProp->GetNormalFont()->m_hObject);

	// file the combo box
	LONG idx;
	CString s;

	s.LoadString(IDS_TRUE);
	idx = AddString(s);
	SetItemData(idx, TRUE);

	s.LoadString(IDS_FALSE);
	idx = AddString(s);
	SetItemData(idx, FALSE);

	return TRUE;
}


LONG CPropTreeItemCombo::FindCBData(LPARAM lParam)
{
	LONG idx;
	
	for (idx = 0; idx < GetCount(); idx++)
	{
		if (GetItemData(idx)==(DWORD)lParam)
			return idx;
	}

	return CB_ERR;
}


void CPropTreeItemCombo::OnSelchange() 
{
	CommitChanges();
}


void CPropTreeItemCombo::OnKillfocus() 
{
	CommitChanges();
}


void CPropTreeItemCombo::SetDropDownHeight(LONG nDropHeight)
{
	m_nDropHeight = nDropHeight;
}


LONG CPropTreeItemCombo::GetDropDownHeight()
{
	return m_nDropHeight;
}

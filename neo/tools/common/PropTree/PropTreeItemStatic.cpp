// PropTreeItemStatic.cpp
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

#include "PropTreeItemStatic.h"


CPropTreeItemStatic::CPropTreeItemStatic() :
	m_sAttribute(_T(""))
{
}


CPropTreeItemStatic::~CPropTreeItemStatic()
{
}


void CPropTreeItemStatic::DrawAttribute(CDC* pDC, const RECT& rc)
{
	ASSERT(m_pProp!=NULL);

	pDC->SelectObject(m_pProp->GetNormalFont());
	pDC->SetTextColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);

	CRect r = rc;
	pDC->DrawText(m_sAttribute, r, DT_SINGLELINE|DT_VCENTER);
}


LPARAM CPropTreeItemStatic::GetItemValue()
{
	return (LPARAM)(LPCTSTR)m_sAttribute;
}


void CPropTreeItemStatic::SetItemValue(LPARAM lParam)
{
	if (lParam==0L)
	{
		TRACE0("CPropTreeItemStatic::SetItemValue() - Invalid lParam value\n");
		return;
	}

	m_sAttribute = (LPCTSTR)lParam;
}

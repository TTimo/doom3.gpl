// PropTreeItemColor.cpp : implementation file
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
#include "PropTreeItemColor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HINSTANCE ghInst;

typedef struct _ColorTableEntry
{
	COLORREF	color;
	RECT		rcSpot;
} ColorTableEntry;

static ColorTableEntry _crColors[] = 
{
    {RGB(0x00, 0x00, 0x00)},
    {RGB(0xA5, 0x2A, 0x00)},
    {RGB(0x00, 0x40, 0x40)},
    {RGB(0x00, 0x55, 0x00)},
    {RGB(0x00, 0x00, 0x5E)},
    {RGB(0x00, 0x00, 0x8B)},
    {RGB(0x4B, 0x00, 0x82)},
    {RGB(0x28, 0x28, 0x28)},

    {RGB(0x8B, 0x00, 0x00)},
    {RGB(0xFF, 0x68, 0x20)},
    {RGB(0x8B, 0x8B, 0x00)},
    {RGB(0x00, 0x93, 0x00)},
    {RGB(0x38, 0x8E, 0x8E)},
    {RGB(0x00, 0x00, 0xFF)},
    {RGB(0x7B, 0x7B, 0xC0)},
    {RGB(0x66, 0x66, 0x66)},

    {RGB(0xFF, 0x00, 0x00)},
    {RGB(0xFF, 0xAD, 0x5B)},
    {RGB(0x32, 0xCD, 0x32)}, 
    {RGB(0x3C, 0xB3, 0x71)},
    {RGB(0x7F, 0xFF, 0xD4)},
    {RGB(0x7D, 0x9E, 0xC0)},
    {RGB(0x80, 0x00, 0x80)},
    {RGB(0x7F, 0x7F, 0x7F)},

    {RGB(0xFF, 0xC0, 0xCB)},
    {RGB(0xFF, 0xD7, 0x00)},
    {RGB(0xFF, 0xFF, 0x00)},    
    {RGB(0x00, 0xFF, 0x00)},
    {RGB(0x40, 0xE0, 0xD0)},
    {RGB(0xC0, 0xFF, 0xFF)},
    {RGB(0x48, 0x00, 0x48)},
    {RGB(0xC0, 0xC0, 0xC0)},

    {RGB(0xFF, 0xE4, 0xE1)},
    {RGB(0xD2, 0xB4, 0x8C)},
    {RGB(0xFF, 0xFF, 0xE0)},
    {RGB(0x98, 0xFB, 0x98)},
    {RGB(0xAF, 0xEE, 0xEE)},
    {RGB(0x68, 0x83, 0x8B)},
    {RGB(0xE6, 0xE6, 0xFA)},
    {RGB(0xFF, 0xFF, 0xFF)}
};

static void ColorBox(CDC* pDC, CPoint pt, COLORREF clr, BOOL bHover)
{
	CBrush br(clr);

	CBrush* obr = pDC->SelectObject(&br);

	pDC->PatBlt(pt.x, pt.y, 13, 13, PATCOPY);
	pDC->SelectObject(obr);

	CRect rc;
	rc.SetRect(pt.x - 2, pt.y - 2, pt.x + 15, pt.y + 15);

	pDC->DrawEdge(&rc, (bHover) ? BDR_SUNKENOUTER : BDR_RAISEDINNER, BF_RECT);
}



static LONG FindSpot(CPoint point)
{
	for (LONG i=0; i<40; i++)
	{
		if (PtInRect(&_crColors[i].rcSpot, point))
			return i;
	}

	return -1;
}


/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemColor

COLORREF* CPropTreeItemColor::s_pColors = NULL;

CPropTreeItemColor::CPropTreeItemColor() :
	m_cColor(0),
	m_cPrevColor(0),
	m_nSpot(-1),
	m_bButton(FALSE),
	m_bInDialog(FALSE)
{
}

CPropTreeItemColor::~CPropTreeItemColor()
{
}


BEGIN_MESSAGE_MAP(CPropTreeItemColor, CWnd)
	//{{AFX_MSG_MAP(CPropTreeItemColor)
	ON_WM_KILLFOCUS()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemColor message handlers

void CPropTreeItemColor::SetDefaultColorsList(COLORREF* pColors)
{
	s_pColors = pColors;
}


void CPropTreeItemColor::DrawAttribute(CDC* pDC, const RECT& rc)
{
	ASSERT(m_pProp!=NULL);

	CRect r(rc);
	
	pDC->SelectObject(IsReadOnly() ? m_pProp->GetNormalFont() : m_pProp->GetBoldFont());

	if (!m_pProp->IsWindowEnabled())
		pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	else
		pDC->SetTextColor(RGB(0,0,0));

	r.top += 1;
	r.right = r.left + r.Height() - 1;

	CBrush br(m_cColor);
	CBrush* pold = pDC->SelectObject(&br);
	pDC->PatBlt(r.left, r.top, r.Width(), r.Height(), PATCOPY);
	pDC->SelectObject(pold);

	pDC->DrawEdge(&r, EDGE_SUNKEN, BF_RECT);

	CString s;

	r = rc;
	r.left += r.Height();
	s.Format(_T("R = %d, G = %d, B = %d"), GetRValue(m_cColor),GetGValue(m_cColor), GetBValue(m_cColor));
	pDC->DrawText(s, r, DT_SINGLELINE|DT_VCENTER);
}


LPARAM CPropTreeItemColor::GetItemValue()
{
	return m_cColor;
}


void CPropTreeItemColor::SetItemValue(LPARAM lParam)
{
	m_cColor = lParam;
}


void CPropTreeItemColor::OnMove()
{
}


void CPropTreeItemColor::OnRefresh()
{
}


void CPropTreeItemColor::OnCommit()
{
	ShowWindow(SW_HIDE);
}


void CPropTreeItemColor::OnActivate(int activateType, CPoint point)
{
	CRect r;

	m_cPrevColor = m_cColor;

	r = m_rc;
	r.right = r.left + 150;
	r.bottom = r.top + 120;

	ASSERT(m_pProp!=NULL);
	m_pProp->GetCtrlParent()->ClientToScreen(r);

	if (!IsWindow(m_hWnd))
	{
		LPCTSTR pszClassName;

		pszClassName = AfxRegisterWndClass(CS_VREDRAW|CS_HREDRAW, LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE + 1));
		
		DWORD dwStyle = WS_POPUP|WS_DLGFRAME;

		CreateEx(0, pszClassName, _T(""), dwStyle, r, m_pProp->GetCtrlParent(), 0);
		m_rcButton.SetRect(40, 94, 110, 114);
	}

	SetWindowPos(NULL, r.left, r.top, r.Width() + 1, r.Height(), SWP_NOZORDER|SWP_SHOWWINDOW);
	SetFocus();
}


void CPropTreeItemColor::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	
	if (!m_bInDialog)
		CommitChanges();
}


void CPropTreeItemColor::OnPaint() 
{
	CPaintDC dc(this);
	CPoint pt;
	
	for (LONG i=0; i<40; i++)
	{
		pt.x = (i & 7) * 18 + 3;
		pt.y = (i >> 3) * 18 + 3;
		ColorBox(&dc, pt, _crColors[i].color, m_nSpot==i);
		SetRect(&_crColors[i].rcSpot, pt.x, pt.y, pt.x + 13, pt.y + 13);
	}

	ASSERT(m_pProp!=NULL);

	dc.SelectObject(m_pProp->GetNormalFont());

	CString s(_T("More Colors"));

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(GetSysColor(COLOR_BTNTEXT));
	dc.DrawText(s, &m_rcButton, DT_SINGLELINE|DT_VCENTER|DT_CENTER);

	dc.DrawEdge(&m_rcButton, m_bButton ? BDR_SUNKENOUTER : BDR_RAISEDINNER, BF_RECT);
}


void CPropTreeItemColor::OnClose() 
{
	CommitChanges();
}


void CPropTreeItemColor::OnMouseMove(UINT, CPoint point) 
{
	BOOL bButton;
	LONG nSpot;

	nSpot = FindSpot(point);
	if (nSpot!=m_nSpot)
	{
		Invalidate(FALSE);
		m_nSpot = nSpot;
	}

	bButton = m_rcButton.PtInRect(point);

	if (bButton!=m_bButton)
	{
		m_bButton = bButton;
		Invalidate(FALSE);
	}
}


BOOL CPropTreeItemColor::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest==HTCLIENT)
	{
		CPoint point;

		GetCursorPos(&point);
		ScreenToClient(&point);

		if (FindSpot(point)!=-1 || m_rcButton.PtInRect(point))
		{
			SetCursor(LoadCursor(ghInst, MAKEINTRESOURCE(IDC_FPOINT)));
			return TRUE;
		}

	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


void CPropTreeItemColor::OnLButtonDown(UINT, CPoint point) 
{
	if (m_nSpot!=-1)
	{
		m_cColor = _crColors[m_nSpot].color;
		CommitChanges();
	}
	else
	if (m_rcButton.PtInRect(point))
	{
		CHOOSECOLOR cc;
		COLORREF clr[16];

		ZeroMemory(&cc, sizeof(CHOOSECOLOR));
		cc.Flags = CC_FULLOPEN|CC_ANYCOLOR|CC_RGBINIT;
		cc.lStructSize = sizeof(CHOOSECOLOR);
		cc.hwndOwner = m_hWnd;
		cc.rgbResult = m_cColor;
		cc.lpCustColors = s_pColors ? s_pColors : clr;

		memset(clr, 0xff, sizeof(COLORREF) * 16);
		clr[0] = m_cColor;

		m_bInDialog = TRUE;

		ASSERT(m_pProp!=NULL);
		m_pProp->DisableInput();

		ShowWindow(SW_HIDE);

		if (ChooseColor(&cc))
			m_cColor = cc.rgbResult;

		m_pProp->DisableInput(FALSE);
		CommitChanges();
	}
}

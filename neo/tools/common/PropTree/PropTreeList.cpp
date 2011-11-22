// PropTreeList.cpp : implementation file
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
#include "PropTreeList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PROPTREEITEM_EXPANDCOLUMN		16			// width of the expand column
#define PROPTREEITEM_COLRNG				5			// width of splitter
#define PROPTREEITEM_DEFHEIGHT			21			// default heigt of an item

extern HINSTANCE ghInst;

/////////////////////////////////////////////////////////////////////////////
// CPropTreeList

CPropTreeList::CPropTreeList() :
	m_pProp(NULL),
	m_BackBufferSize(0,0),
	m_bColDrag(FALSE),
	m_nPrevCol(0)
{
}

CPropTreeList::~CPropTreeList()
{
}


BEGIN_MESSAGE_MAP(CPropTreeList, CWnd)
	//{{AFX_MSG_MAP(CPropTreeList)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropTreeList message handlers

void CPropTreeList::SetPropOwner(CPropTree* pProp)
{
	m_pProp = pProp;
}


BOOL CPropTreeList::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CWnd* pWnd = this;

	LPCTSTR pszCreateClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW));

	return pWnd->Create(pszCreateClass, _T(""), dwStyle, rect, pParentWnd, nID);
}


void CPropTreeList::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	RecreateBackBuffer(cx, cy);

	if (m_pProp)
	{
		UpdateResize();
		Invalidate();
		UpdateWindow();

		// inform all items that a resize has been made
		m_pProp->UpdateMoveAllItems();
	}
}


void CPropTreeList::RecreateBackBuffer(int cx, int cy)
{
	if (m_BackBufferSize.cx<cx || m_BackBufferSize.cy<cy)
	{
		m_BackBufferSize = CSize(cx, cy);

		CWindowDC dc(NULL);

		int nPlanes = dc.GetDeviceCaps(PLANES);
		int nBitCount = dc.GetDeviceCaps(BITSPIXEL);

		m_BackBuffer.DeleteObject();
		m_BackBuffer.CreateBitmap(cx, cy, nPlanes, nBitCount, NULL);
	}
}


void CPropTreeList::UpdateResize()
{
	SCROLLINFO si;
	LONG nHeight;
	CRect rc;

	ASSERT(m_pProp!=NULL);

	GetClientRect(rc);
	nHeight = rc.Height() + 1;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE|SIF_PAGE;
	si.nMin = 0;
	si.nMax = m_pProp->GetRootItem()->GetTotalHeight();
	si.nPage = nHeight;

	if ((int)si.nPage>si.nMax)
		m_pProp->SetOriginOffset(0);

	SetScrollInfo(SB_VERT, &si, TRUE);

	// force set column for clipping
	m_pProp->SetColumn(m_pProp->GetColumn());
}


void CPropTreeList::OnPaint() 
{
	CPaintDC dc(this);
	CDC memdc;
	CBitmap* pOldBitmap;

	ASSERT(m_pProp!=NULL);

	m_pProp->ClearVisibleList();

	memdc.CreateCompatibleDC(&dc);
	pOldBitmap = memdc.SelectObject(&m_BackBuffer);

	CRect rc;
	GetClientRect(rc);

	// draw control background
	memdc.SelectObject(GetSysColorBrush(COLOR_BTNFACE));
	memdc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), PATCOPY);

	// draw control inside fill color
	rc.DeflateRect(2,2);
	memdc.PatBlt(rc.left, rc.top, rc.Width(), rc.Height(), m_pProp->IsWindowEnabled() ? WHITENESS : PATCOPY);
	rc.InflateRect(2,2);

	// draw expand column
	memdc.SelectObject(GetSysColorBrush(COLOR_BTNFACE));
	memdc.PatBlt(0, 0, PROPTREEITEM_EXPANDCOLUMN, rc.Height(), PATCOPY);

	// draw edge
	memdc.DrawEdge(&rc, BDR_SUNKENOUTER, BF_RECT);

	CPropTreeItem* pItem;
	LONG nTotal = 0;

	ASSERT(m_pProp->GetRootItem()!=NULL);

	rc.DeflateRect(2,2);

	// create clip region
	HRGN hRgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
	SelectClipRgn(memdc.m_hDC, hRgn);

	// draw all items
	for (pItem = m_pProp->GetRootItem()->GetChild(); pItem; pItem = pItem->GetSibling())
	{
		LONG nHeight = pItem->DrawItem(&memdc, rc, 0, nTotal);
		nTotal += nHeight;
	}

	// remove clip region
	SelectClipRgn(memdc.m_hDC, NULL);
	DeleteObject(hRgn);

	// copy back buffer to the display
	dc.GetClipBox(&rc);
	dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &memdc, rc.left, rc.top, SRCCOPY);
	memdc.DeleteDC();
}


BOOL CPropTreeList::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest==HTCLIENT)
	{
		CPoint pt;

		ASSERT(m_pProp!=NULL);

		GetCursorPos(&pt);
		ScreenToClient(&pt);

		switch (m_pProp->HitTest(pt))
		{
			case HTCOLUMN:
				SetCursor(LoadCursor(ghInst, MAKEINTRESOURCE(IDC_SPLITTER)));
				return TRUE;

			case HTCHECKBOX:
			case HTBUTTON:
			case HTEXPAND:
				SetCursor(LoadCursor(ghInst, MAKEINTRESOURCE(IDC_FPOINT)));
				return TRUE;
		}
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


void CPropTreeList::OnLButtonDown(UINT, CPoint point) 
{
	ASSERT(m_pProp!=NULL);

	if (m_pProp->IsDisableInput())
		return;

	m_pProp->SendNotify(NM_CLICK);

	if (!m_pProp->IsWindowEnabled())
		return;

	SetFocus();

	LONG nHit = m_pProp->HitTest(point);

	CPropTreeItem* pItem;
	CRect rc;
	CDC* pDC;

	switch (nHit)
	{
		case HTCOLUMN:
			if (m_pProp->SendNotify(PTN_COLUMNCLICK))
				break;

			m_bColDrag = TRUE;
			SetCapture();

			m_nPrevCol = m_pProp->GetOrigin().x;

			// paint drag line
			pDC = GetDC();
			GetClientRect(rc);
			pDC->PatBlt(m_nPrevCol - PROPTREEITEM_COLRNG/2, 0, PROPTREEITEM_COLRNG, rc.bottom, PATINVERT);
			ReleaseDC(pDC);
			break;

		case HTCHECKBOX:
			if ((pItem = m_pProp->FindItem(point))!=NULL)
			{
				pItem->Check(!pItem->IsChecked());
				m_pProp->SendNotify(PTN_CHECKCLICK, pItem);
				Invalidate();
			}
			break;
		case HTBUTTON:
			if ((pItem = m_pProp->FindItem(point))!=NULL)
			{
				pItem->Check();
				m_pProp->SendNotify(PTN_ITEMBUTTONCLICK, pItem);
				Invalidate();
			}
			break;
		case HTEXPAND:
			if ((pItem = m_pProp->FindItem(point))!=NULL)
			{
				if (pItem->GetChild() && !m_pProp->SendNotify(PTN_ITEMEXPANDING, pItem))
				{
					pItem->Expand(!pItem->IsExpanded());

					UpdateResize();
					Invalidate();
					UpdateWindow();
					CheckVisibleFocus();
				}
			}
			break;

		default:
			if ((pItem = m_pProp->FindItem(point))!=NULL)
			{
				CPropTreeItem* pOldFocus = m_pProp->GetFocusedItem();

				m_pProp->SelectItems(NULL, FALSE);
				m_pProp->SetFocusedItem(pItem);

				pItem->Select();

				Invalidate();

				if (pItem!=pOldFocus)
					m_pProp->SendNotify(PTN_SELCHANGE, pItem);

				if (nHit==HTATTRIBUTE && !pItem->IsRootLevel())
				{
					if (!m_pProp->SendNotify(PTN_PROPCLICK, pItem) && !pItem->IsReadOnly())
						pItem->Activate(CPropTreeItem::ACTIVATE_TYPE_MOUSE, point);
				}
			}
			else
			{
				m_pProp->SelectItems(NULL, FALSE);
				m_pProp->SetFocusedItem(NULL);
				m_pProp->SendNotify(PTN_SELCHANGE);
				Invalidate();
			}
			break;
	}
}


void CPropTreeList::OnLButtonUp(UINT, CPoint point) 
{
	if (m_bColDrag)
	{
		CDC* pDC = GetDC();
		CRect rc;

		GetClientRect(rc);
		pDC->PatBlt(m_nPrevCol - PROPTREEITEM_COLRNG/2, 0, PROPTREEITEM_COLRNG, rc.bottom, PATINVERT);
		ReleaseDC(pDC);

		m_bColDrag = FALSE;
		ReleaseCapture();

		m_pProp->SetColumn(point.x);
		m_pProp->UpdateMoveAllItems();
		Invalidate();
	} else {
		LONG nHit = m_pProp->HitTest(point);
		CPropTreeItem* pItem;

		switch (nHit)
		{
			case HTBUTTON:
				if ((pItem = m_pProp->FindItem(point))!=NULL)
				{
					pItem->Check( FALSE );
					Invalidate();
				}
				break;
			default:
				break;
		}
	}
}


void CPropTreeList::OnLButtonDblClk(UINT, CPoint point)
{
	ASSERT(m_pProp!=NULL);

	m_pProp->SendNotify(NM_DBLCLK);

	CPropTreeItem* pItem;
	CPropTreeItem* pOldFocus;

	if ((pItem = m_pProp->FindItem(point))!=NULL && pItem->GetChild())
	{
		switch (m_pProp->HitTest(point))
		{
			case HTCOLUMN:
				break;

			case HTCHECKBOX:
				pItem->Check(!pItem->IsChecked());
				m_pProp->SendNotify(PTN_CHECKCLICK, pItem);
				Invalidate();
				break;

			case HTATTRIBUTE:
				if (!pItem->IsRootLevel())
					break;

				// pass thru to default

			default:
				pOldFocus = m_pProp->GetFocusedItem();
				m_pProp->SelectItems(NULL, FALSE);
				m_pProp->SetFocusedItem(pItem);
				pItem->Select();

				if (pItem!=pOldFocus)
					m_pProp->SendNotify(PTN_SELCHANGE, pItem);

				// pass thru to HTEXPAND

			case HTEXPAND:
				if (!m_pProp->SendNotify(PTN_ITEMEXPANDING, pItem))
				{
					pItem->Expand(!pItem->IsExpanded());

					UpdateResize();
					Invalidate();
					UpdateWindow();
					CheckVisibleFocus();
				}
				break;
		}
	}
}


void CPropTreeList::OnMouseMove(UINT, CPoint point)
{
	if (m_bColDrag)
	{
		CDC* pDC = GetDC();
		CRect rc;

		GetClientRect(rc);
		pDC->PatBlt(m_nPrevCol - PROPTREEITEM_COLRNG/2, 0, PROPTREEITEM_COLRNG, rc.bottom, PATINVERT);
		pDC->PatBlt(point.x - PROPTREEITEM_COLRNG/2, 0, PROPTREEITEM_COLRNG, rc.bottom, PATINVERT);
		m_nPrevCol = point.x;
		ReleaseDC(pDC);
	}
}


BOOL CPropTreeList::OnMouseWheel(UINT, short zDelta, CPoint) 
{
	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE;

	GetScrollInfo(SB_VERT, &si);

	CRect rc;
	GetClientRect(rc);

	if (si.nMax - si.nMin < rc.Height())
		return TRUE;

	SetFocus();
	OnVScroll(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, NULL);

	return TRUE;
}


void CPropTreeList::OnKeyDown(UINT nChar, UINT, UINT) 
{

	CPropTreeItem* pItem;

	ASSERT(m_pProp!=NULL);

	if (m_pProp->IsDisableInput() || !m_pProp->IsWindowEnabled())
		return;

	switch (nChar)
	{
		case VK_RETURN:
			if ((pItem = m_pProp->GetFocusedItem())!=NULL && !pItem->IsRootLevel() && !pItem->IsReadOnly())
			{
				pItem->Activate(CPropTreeItem::ACTIVATE_TYPE_KEYBOARD, CPoint(0,0));
			}
			break;

		case VK_HOME:
			if (m_pProp->FocusFirst())
				Invalidate();
			break;

		case VK_END:
			if (m_pProp->FocusLast())
				Invalidate();
			break;

		case VK_LEFT:
			if ((pItem = m_pProp->GetFocusedItem())!=NULL)
			{
				if (!m_pProp->SendNotify(PTN_ITEMEXPANDING, pItem))
				{
					if (pItem->GetChild() && pItem->IsExpanded())
					{
						pItem->Expand(FALSE);
						UpdateResize();
						Invalidate();
						UpdateWindow();
						CheckVisibleFocus();
						break;
					}
				}
			}
			else
				break;
			// pass thru to next case VK_UP
		case VK_UP:
			if (m_pProp->FocusPrev())
				Invalidate();
			break;

		case VK_RIGHT:
			if ((pItem = m_pProp->GetFocusedItem())!=NULL)
			{
				if (!m_pProp->SendNotify(PTN_ITEMEXPANDING, pItem))
				{
					if (pItem->GetChild() && !pItem->IsExpanded())
					{
						pItem->Expand();
						UpdateResize();
						Invalidate();
						UpdateWindow();
						CheckVisibleFocus();
						break;
					}
				}
			}
			else
				break;
			// pass thru to next case VK_DOWN
		case VK_DOWN:
			if (m_pProp->FocusNext())
				Invalidate();
			break;
	}
}


UINT CPropTreeList::OnGetDlgCode() 
{
	return DLGC_WANTARROWS|DLGC_WANTCHARS|DLGC_WANTALLKEYS;
}


void CPropTreeList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar*) 
{
	SCROLLINFO si;
	CRect rc;
	LONG nHeight;

	SetFocus();

	GetClientRect(rc);
	nHeight = rc.Height() + 1;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE;

	GetScrollInfo(SB_VERT, &si);

	LONG ny = m_pProp->GetOrigin().y;

	switch (nSBCode)
	{
		case SB_LINEDOWN:
			ny += PROPTREEITEM_DEFHEIGHT;
			break;

		case SB_LINEUP:
			ny -= PROPTREEITEM_DEFHEIGHT;
			break;

		case SB_PAGEDOWN:
			ny += nHeight;
			break;

		case SB_PAGEUP:
			ny -= nHeight;
			break;

		case SB_THUMBTRACK:
			ny = nPos;
			break;
	}

	ny = __min(__max(ny, si.nMin), si.nMax - nHeight);

	m_pProp->SetOriginOffset(ny);
	si.fMask = SIF_POS;
	si.nPos = ny;

	SetScrollInfo(SB_VERT, &si, TRUE);
	Invalidate();
}


void CPropTreeList::CheckVisibleFocus()
{
	ASSERT(m_pProp!=NULL);

	CPropTreeItem* pItem;
	
	if ((pItem = m_pProp->GetFocusedItem())==NULL)
		return;

	if (!m_pProp->IsItemVisible(pItem))
	{
		if (m_pProp->IsSingleSelection())
			pItem->Select(FALSE);

		m_pProp->SetFocusedItem(NULL);
		m_pProp->SendNotify(PTN_SELCHANGE, NULL);

		Invalidate();
	}
}

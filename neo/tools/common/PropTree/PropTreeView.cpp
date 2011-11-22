// CPropTreeView.cpp : implementation file
//

//#include "stdafx.h"
#include "../../../idlib/precompiled.h"
#pragma hdrstop


#include "PropTreeView.h"

// CPropTreeView

IMPLEMENT_DYNCREATE(CPropTreeView, CFormView)

CPropTreeView::CPropTreeView()
: CFormView((LPCTSTR) NULL)
{
}

CPropTreeView::~CPropTreeView()
{
}

BEGIN_MESSAGE_MAP(CPropTreeView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CPropTreeView drawing

void CPropTreeView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


// CPropTreeView diagnostics

#ifdef _DEBUG
void CPropTreeView::AssertValid() const
{
	CView::AssertValid();
}

void CPropTreeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG


BOOL CPropTreeView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
					   DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
					   UINT nID, CCreateContext* pContext)
{
	// create the view window itself
	m_pCreateContext = pContext;
	if (!CView::Create(lpszClassName, lpszWindowName,
		dwStyle, rect, pParentWnd,  nID, pContext))
	{
		return FALSE;
	}

	return TRUE;
}
// CPropTreeView message handlers

int CPropTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD dwStyle;
	CRect rc;

	// PTS_NOTIFY - CPropTree will send notification messages to the parent window
	dwStyle = WS_CHILD|WS_VISIBLE|PTS_NOTIFY;

	// Init the control's size to cover the entire client area
	GetClientRect(rc);

	// Create CPropTree control
	m_Tree.Create(dwStyle, rc, this, IDC_PROPERTYTREE);

	return 0;
}

void CPropTreeView::OnSize(UINT nType, int cx, int cy)
{
		CView::OnSize(nType, cx, cy);

		if (::IsWindow(m_Tree.GetSafeHwnd()))
			m_Tree.SetWindowPos(NULL, -1, -1, cx, cy, SWP_NOMOVE|SWP_NOZORDER);	
}


void CPropTreeView::OnPaint()
{
	Default();
}

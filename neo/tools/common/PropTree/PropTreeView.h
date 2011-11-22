#pragma once

#include "PropTree.h"
// CPropTreeView view

#define IDC_PROPERTYTREE			100

class CPropTreeView : public CFormView
{
	DECLARE_DYNCREATE(CPropTreeView)

protected:
	CPropTree		m_Tree;

protected:
	CPropTreeView();           // protected constructor used by dynamic creation
	virtual ~CPropTreeView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CPropTree&	GetPropertyTreeCtrl() { return m_Tree; };

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
		DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
		CCreateContext* pContext = NULL);
	
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
};



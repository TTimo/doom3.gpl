#ifndef __PROP_TREE_ITEM_EDIT_BUTTON_H__
#define __PROP_TREE_ITEM_EDIT_BUTTON_H__


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropTreeItemEdit.h : header file
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

#include "PropTreeItem.h"
//#include "PropTreeItemEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemEditButton window

class PROPTREE_API CPropTreeItemEditButton : public CPropTreeItemEdit
{
	// Construction
public:
	CPropTreeItemEditButton();
	virtual ~CPropTreeItemEditButton();

	// Attributes
public:
	// The non-attribute area needs drawing
	virtual LONG DrawItem(CDC* pDC, const RECT& rc, LONG x, LONG y);

	// The attribute area needs drawing
	virtual void DrawAttribute(CDC* pDC, const RECT& rc);

	// Retrieve the item's attribute value
	virtual LPARAM GetItemValue();

	// Set the item's attribute value
	virtual void SetItemValue(LPARAM lParam);

	// Called when attribute area has changed size
	virtual void OnMove();

	// Called when the item needs to refresh its data
	virtual void OnRefresh();

	// Called when the item needs to commit its changes
	virtual void OnCommit();

	// Called to activate the item
	virtual void OnActivate(int activateType, CPoint point);


	enum ValueFormat
	{
		ValueFormatText,
		ValueFormatNumber,
		ValueFormatFloatPointer
	};

	// Set to specifify format of SetItemValue/GetItemValue
	void SetValueFormat(ValueFormat nFormat);

	// Set to TRUE for to use a password edit control
	void SetAsPassword(BOOL bPassword);

	// Overrideable - Returns TRUE if the point is on the button
	virtual BOOL HitButton(const POINT& pt);

	void SetButtonText( LPCSTR text );

protected:
	CString		m_sEdit;
	float		m_fValue;

	ValueFormat m_nFormat;
	BOOL		m_bPassword;


	CString				buttonText;
	CRect				buttonRect;
	CRect				hitTestRect;
	bool				mouseDown;


	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropTreeItemEditButton)
	//}}AFX_VIRTUAL

	// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropTreeItemEditButton)
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillfocus();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // __PROP_TREE_ITEM_EDIT_BUTTON_H__

#pragma once

// PropTreeItemButton.h : header file
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

/////////////////////////////////////////////////////////////////////////////
// CPropTreeItemButton window

class PROPTREE_API CPropTreeItemButton : public CPropTreeItem
{
// Construction
public:
	CPropTreeItemButton();
	virtual ~CPropTreeItemButton();

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

	// Overrideable - Returns TRUE if the point is on the button
	virtual BOOL HitButton(const POINT& pt);

	void SetButtonText( LPCSTR text );

protected:
	CString				buttonText;
	CRect				buttonRect;
	CRect				hitTestRect;
	bool				mouseDown;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#ifndef __PROP_TREE_ITEM_FILE_EDIT_H__
#define __PROP_TREE_ITEM_FILE_EDIT_H__

#if _MSC_VER > 1000
#pragma once
#endif


//#include "PropTreeItem.h"
//#include "PropTreeItemEdit.h"

class PROPTREE_API CPropTreeItemFileEdit : public CPropTreeItemEdit
{
	// Construction
public:
	CPropTreeItemFileEdit();
	virtual ~CPropTreeItemFileEdit();

	// Operations
public:

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropTreeItemFileEdit)
	//}}AFX_VIRTUAL

	// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropTreeItemFileEdit)
	//}}AFX_MSG

	afx_msg void 				OnInsertFile();
	afx_msg void 				OnEditUndo();
	afx_msg void 				OnEditCut();
	afx_msg void 				OnEditCopy();
	afx_msg void 				OnEditPaste();
	afx_msg void 				OnEditDelete();
	afx_msg void 				OnEditSelectAll();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}


#endif 

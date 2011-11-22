/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __DIALOGDECLEDITOR_H__
#define __DIALOGDECLEDITOR_H__

#pragma once

#include "../comafx/CSyntaxRichEditCtrl.h"

// DialogDeclEditor dialog

class DialogDeclEditor : public CDialog {

	DECLARE_DYNAMIC(DialogDeclEditor)

public:
						DialogDeclEditor( CWnd* pParent = NULL );   // standard constructor
	virtual				~DialogDeclEditor();

	void				LoadDecl( idDecl *decl );

	//{{AFX_VIRTUAL(DialogDeclEditor)
	virtual BOOL		OnInitDialog();
	virtual void		DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	virtual BOOL		PreTranslateMessage( MSG* pMsg );
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(DialogDeclEditor)
	afx_msg BOOL		OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnSetFocus( CWnd *pOldWnd );
	afx_msg void		OnDestroy();
	afx_msg void		OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized );
	afx_msg void		OnMove( int x, int y );
	afx_msg void		OnSize( UINT nType, int cx, int cy );
	afx_msg void		OnSizing( UINT nSide, LPRECT lpRect );
	afx_msg void		OnEditGoToLine();
	afx_msg void		OnEditFind();
	afx_msg void		OnEditFindNext();
	afx_msg void		OnEditReplace();
    afx_msg LRESULT		OnFindDialogMessage( WPARAM wParam, LPARAM lParam );
	afx_msg void		OnEnChangeEdit( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEnInputEdit( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedTest();
	afx_msg void		OnBnClickedOk();
	afx_msg void		OnBnClickedCancel();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:

	//{{AFX_DATA(DialogDeclEditor)
	enum				{ IDD = IDD_DIALOG_DECLEDITOR };
	CStatusBarCtrl		statusBar;
	CSyntaxRichEditCtrl	declEdit;
	CButton				testButton;
	CButton				okButton;
	CButton				cancelButton;
	//}}AFX_DATA

	static toolTip_t	toolTips[];

	HACCEL				m_hAccel;
	CRect				initialRect;
	CFindReplaceDialog *findDlg;
	CString				findStr;
	CString				replaceStr;
	bool				matchCase;
	bool				matchWholeWords;
	bool				searchForward;
	idDecl *			decl;
	int					firstLine;

private:
	bool				TestDecl( const idStr &declText );
	void				UpdateStatusBar( void );
};

#endif /* !__DIALOGDECLEDITOR_H__ */

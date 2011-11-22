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
#if !defined(AFX_FINDTEXTUREDLG_H__34B75D32_9F3A_11D1_B570_00AA00A410FC__INCLUDED_)
#define AFX_FINDTEXTUREDLG_H__34B75D32_9F3A_11D1_B570_00AA00A410FC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FindTextureDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFindTextureDlg dialog

class CFindTextureDlg : public CDialog
{
// Construction
public:
	static void setReplaceStr(const char* p);
	static void setFindStr(const char* p);
	static bool isOpen();
  static void show();
  static void updateTextures(const char* p);
	CFindTextureDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindTextureDlg)
	enum { IDD = IDD_DIALOG_FINDREPLACE };
	BOOL	m_bSelectedOnly;
	CString	m_strFind;
	CString	m_strReplace;
	BOOL	m_bForce;
	BOOL	m_bLive;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindTextureDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindTextureDlg)
	afx_msg void OnBtnApply();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSetfocusEditFind();
	afx_msg void OnSetfocusEditReplace();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDTEXTUREDLG_H__34B75D32_9F3A_11D1_B570_00AA00A410FC__INCLUDED_)

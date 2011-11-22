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
#if !defined(AFX_DLGCAMERA_H__59C12359_E3EB_4081_9F28_01793D75CF20__INCLUDED_)
#define AFX_DLGCAMERA_H__59C12359_E3EB_4081_9F28_01793D75CF20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCamera.h : header file
//

extern void showCameraInspector();

/////////////////////////////////////////////////////////////////////////////
// CDlgCamera dialog

class CDlgCamera : public CDialog
{
// Construction
public:
	CDlgCamera(CWnd* pParent = NULL);   // standard constructor
	void setupFromCamera();

// Dialog Data
	//{{AFX_DATA(CDlgCamera)
	enum { IDD = IDD_DLG_CAMERA };
	CScrollBar	m_wndSegments;
	CListBox	m_wndEvents;
	CComboBox	m_wndSplines;
	CString	m_strName;
	float	m_fSeconds;
	BOOL	m_trackCamera;
	int		m_numSegments;
	int		m_currentSegment;
	CString	m_strType;
	int		m_editPoints;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCamera)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgCamera)
	afx_msg void OnBtnAddevent();
	afx_msg void OnBtnAddtarget();
	afx_msg void OnBtnDelevent();
	afx_msg void OnBtnDeltarget();
	afx_msg void OnDblclkComboSplines();
	afx_msg void OnSelchangeComboSplines();
	afx_msg void OnSelchangeListEvents();
	afx_msg void OnDblclkListEvents();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	afx_msg void OnApply();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnTestcamera();
	afx_msg void OnBtnDeletepoints();
	afx_msg void OnBtnSelectall();
	afx_msg void OnRadioEditpoints();
	afx_msg void OnRadioAddPoints();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCAMERA_H__59C12359_E3EB_4081_9F28_01793D75CF20__INCLUDED_)

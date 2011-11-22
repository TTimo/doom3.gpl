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
#if !defined(AFX_TEXTUREBAR_H__86220273_B656_11D1_B59F_00AA00A410FC__INCLUDED_)
#define AFX_TEXTUREBAR_H__86220273_B656_11D1_B59F_00AA00A410FC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TextureBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextureBar dialog

class CTextureBar : public CDialogBar
{
// Construction
public:
	void GetSurfaceAttributes();
	void SetSurfaceAttributes();
	CTextureBar();

// Dialog Data
	//{{AFX_DATA(CTextureBar)
	enum { IDD = IDD_TEXTUREBAR };
	CSpinButtonCtrl	m_spinRotate;
	CSpinButtonCtrl	m_spinVScale;
	CSpinButtonCtrl	m_spinVShift;
	CSpinButtonCtrl	m_spinHScale;
	CSpinButtonCtrl	m_spinHShift;
	int	m_nHShift;
	int	m_nHScale;
	int	m_nRotate;
	int	m_nVShift;
	int	m_nVScale;
	int		m_nRotateAmt;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTextureBar)
	afx_msg void OnDeltaposSpinHshift(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinVshift(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinHScale(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinVScale(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinRotate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectionPrint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBtnApplytexturestuff();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTUREBAR_H__86220273_B656_11D1_B59F_00AA00A410FC__INCLUDED_)

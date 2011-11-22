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

#ifndef __DIALOGPDAEDITOR_H__
#define __DIALOGPDAEDITOR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCDialogPDAEditor dialog

class CDialogPDAEditor : public CDialog {
public:
							CDialogPDAEditor(CWnd* pParent = NULL);   // standard constructor

	//{{AFX_VIRTUAL(CDialogPDAEditor)
	virtual BOOL			OnInitDialog();
	virtual void			DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CDialogPDAEditor)
	afx_msg void			OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void			OnMove( int x, int y );
	afx_msg void			OnDestroy();

	afx_msg void			OnSelChangePDA();

	afx_msg void			OnBtnClickedSave();
	afx_msg void			OnBtnClickedRandom();

	afx_msg void			OnBtnClickedPDAAdd();
	afx_msg void			OnBtnClickedPDADel();

	afx_msg void			OnBtnClickedEmailAdd();
	afx_msg void			OnBtnClickedEmailEdit();
	afx_msg void			OnBtnClickedEmailDel();

	afx_msg void			OnBtnClickedAudioAdd();
	afx_msg void			OnBtnClickedAudioEdit();
	afx_msg void			OnBtnClickedAudioDel();
	
	afx_msg void			OnBtnClickedVideoAdd();
	afx_msg void			OnBtnClickedVideoEdit();
	afx_msg void			OnBtnClickedVideoDel();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	//{{AFX_DATA(CDialogPDAEditor)
	enum					{ IDD = IDD_DIALOG_PDA_EDITOR };
	CListBox				pdaList;
	CListBox				emailList;
	CListBox				audioList;
	CListBox				videoList;

	CString					fullName;
	CString					shortName;
	CString					post;
	CString					title;
	CString					security;
	CString					idnum;

	CButton					saveButton;
	//}}AFX_DATA

private:
	virtual BOOL			PreTranslateMessage(MSG* pMsg);

	void PopulatePDAList();
};

class CDialogPDAEditEmail : public CDialog {
public:
							CDialogPDAEditEmail(CWnd* pParent = NULL);   // standard constructor

	//{{AFX_VIRTUAL(CDialogPDAEditEmail)
	virtual BOOL			OnInitDialog();
	virtual void			DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	void SetName( CString &name );
	void SetEmail( const idDeclEmail *email );

	CString GetDeclText();

protected:
	//{{AFX_MSG(CDialogPDAEditEmail)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	//{{AFX_DATA(CDialogPDAEditEmail)
	enum					{ IDD = IDD_DIALOG_PDA_EDIT_EMAIL };

	CString					to;
	CString					from;
	CString					date;
	CString					subject;
	CString					body;

	CString					name;
	//}}AFX_DATA
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif /* !__DIALOGPDAEDITOR_H__ */

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
#pragma once

#include "mediapreviewdlg.h"

// CEditViewDlg dialog

class CEditViewDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditViewDlg)

public:
	enum {MATERIALS, GUIS};
	CEditViewDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditViewDlg();

	void SetMode(int _mode) {
		mode = _mode;
	}

	void SetMaterialInfo(const char *name, const char *file, int line);
	void SetGuiInfo(const char *name);
	void UpdateEditPreview();

	void OpenMedia(const char *name);
// Dialog Data
	enum { IDD = IDD_DIALOG_EDITVIEW };

protected:
	CFindReplaceDialog *findDlg;
	CMediaPreviewDlg mediaPreview;
	int mode;
	idStr fileName;
	idStr matName;
	idStr editText;
	idStr findStr;
	int line;
	CEdit editInfo;

	void ShowFindDlg();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonSave();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedButtonGoto();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg LRESULT OnFindDialogMessage(WPARAM wParam, LPARAM lParam);

};

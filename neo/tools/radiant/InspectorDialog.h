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
#include "afxcmn.h"

#include "entitydlg.h"
#include "ConsoleDlg.h"
#include "TabsDlg.h"


// CInspectorDialog dialog

class CInspectorDialog : public CTabsDlg
{
	//DECLARE_DYNAMIC(CInspectorDialog)w

public:
	CInspectorDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInspectorDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_INSPECTORS };

protected:
	bool initialized;
	unsigned int dockedTabs;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void AssignModel ();
	CTabCtrl tabInspector;
	//idGLConsoleWidget consoleWnd;
	CConsoleDlg consoleWnd;
	CNewTexWnd texWnd;
	CDialogTextures mediaDlg;
	CEntityDlg entityDlg;
	void SetMode(int mode, bool updateTabs = true);
	void UpdateEntitySel(eclass_t *ent);
	void UpdateSelectedEntity();
	void FillClassList();
	bool GetSelectAllCriteria(idStr &key, idStr &val);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void SetDockedTabs ( bool docked , int ID );	
};

extern CInspectorDialog *g_Inspectors;
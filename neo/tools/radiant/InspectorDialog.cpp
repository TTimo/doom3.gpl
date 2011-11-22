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

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "qe3.h"
#include "Radiant.h"
#include "GLWidget.h"
#include "ConsoleDlg.h"
#include "InspectorDialog.h"
#include "TabsDlg.h"

CInspectorDialog *g_Inspectors = NULL;
// CInspectorDialog dialog

void InspectorsDockingCallback ( bool docked , int ID , CWnd* wnd )
{
	g_Inspectors->SetDockedTabs( docked , ID );
}


// CInspectorDialog dialog
//IMPLEMENT_DYNAMIC(CInspectorDialog,CTabsDlg)
CInspectorDialog::CInspectorDialog(CWnd* pParent /*=NULL*/)
	: CTabsDlg(CInspectorDialog::IDD, pParent)
{
	initialized = false;
	dockedTabs = W_CONSOLE | W_TEXTURE | W_MEDIA;
}

CInspectorDialog::~CInspectorDialog()
{
}


BEGIN_MESSAGE_MAP(CInspectorDialog, CTabsDlg)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_INSPECTOR, OnTcnSelchange )
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CInspectorDialog message handlers

BOOL CInspectorDialog::OnInitDialog()
{
	CTabsDlg::OnInitDialog();

	ASSERT ( m_Tabs.GetSafeHwnd() );

	LoadWindowPlacement(GetSafeHwnd() , "radiant_InspectorsWindow" );

	consoleWnd.Create(IDD_DIALOG_CONSOLE, this);
	texWnd.Create(TEXTURE_WINDOW_CLASS, "", QE3_SPLITTER_STYLE, CRect(5, 5, 10, 10), this, 1299);
	mediaDlg.Create(IDD_DIALOG_TEXTURELIST, this);
	entityDlg.Create(IDD_DIALOG_ENTITY, this);

	dockedTabs = GetCvarInt ( "radiant_InspectorDockedDialogs" , W_CONSOLE | W_TEXTURE | W_MEDIA );

	AddDockedWindow ( &consoleWnd , W_CONSOLE , 1 , "Console"	 , (dockedTabs & W_CONSOLE ) != 0 , InspectorsDockingCallback );
	AddDockedWindow ( &texWnd	  , W_TEXTURE , 2 , "Textures"   , (dockedTabs & W_TEXTURE ) != 0 , InspectorsDockingCallback );
	AddDockedWindow ( &mediaDlg	  , W_MEDIA	  , 3 , "Media"	     , (dockedTabs & W_MEDIA   ) != 0 , InspectorsDockingCallback );
	AddDockedWindow ( &entityDlg  , W_ENTITY  , 4 , "Entity"	 , (dockedTabs & W_ENTITY  ) != 0 , InspectorsDockingCallback );
	
	SetMode(W_CONSOLE);	
	initialized = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CInspectorDialog::SetMode(int mode, bool updateTabs) {
	FocusWindow ( mode );
}

void CInspectorDialog::UpdateEntitySel(eclass_t *ent) {
	entityDlg.UpdateEntitySel(ent);
}

void CInspectorDialog::FillClassList() {
	entityDlg.AddClassNames();
}

void CInspectorDialog::UpdateSelectedEntity() {
	entityDlg.SetKeyValPairs();
}

bool CInspectorDialog::GetSelectAllCriteria(idStr &key, idStr &val) {
	CString k, v;
	entityDlg.editKey.GetWindowText(k);
	entityDlg.editVal.GetWindowText(v);
	key = k;
	val = v;
	return true;
}



void CInspectorDialog::OnSize(UINT nType, int cx, int cy)
{
	CTabsDlg::OnSize(nType, cx, cy);

	DockedWindowInfo* info = NULL;
	POSITION pos;
	WORD wID;

	if (!initialized) {
		return;
	}

	CRect rect;
	GetClientRect(rect);

	CRect tabRect;
	m_Tabs.GetWindowRect(tabRect);
	// retain vert size but size 4 in from edges and 4 up from bottom
	tabRect.left = 4;
	tabRect.right = rect.Width() - 4;
	tabRect.top = rect.Height() - tabRect.Height() - 4;
	tabRect.bottom = rect.Height() - 4;
	// adjust rect for children size
	rect.bottom -= 5 + tabRect.Height();

	m_Tabs.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), 0);

	for( pos = m_Windows.GetStartPosition(); pos != NULL ; )
	{
		m_Windows.GetNextAssoc( pos, wID, (void*&)info );

		if ( (info->m_State == DockedWindowInfo::DOCKED) ) {
			info->m_Window->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), 0);
		}

	}
}

void CInspectorDialog::OnDestroy()
{
	::SaveWindowPlacement(GetSafeHwnd() , "radiant_InspectorsWindow" );	
	SetCvarInt("radiant_InspectorDockedDialogs" , dockedTabs );

	CTabsDlg::OnDestroy();
}

void CInspectorDialog::OnClose()
{
	CTabsDlg::OnClose();
}

BOOL CInspectorDialog::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if ( pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) {
		g_pParentWnd->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
	}
	return CTabsDlg::PreTranslateMessage(pMsg);
}

void CInspectorDialog::SetDockedTabs ( bool docked , int ID )
{
	if ( docked ) {
		dockedTabs |= ID;
	}
	else {
		dockedTabs &= ~ID;
	}
}

void CInspectorDialog::AssignModel ()
{
	entityDlg.AssignModel();
}

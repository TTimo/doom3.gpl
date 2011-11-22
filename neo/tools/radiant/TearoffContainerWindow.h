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

// CTearoffContainerWindow

class CTabsDlg;
class CTearoffContainerWindow : public CWnd
{
	DECLARE_DYNAMIC(CTearoffContainerWindow)

public:
	CTearoffContainerWindow();
	virtual ~CTearoffContainerWindow();

	CWnd* m_ContainedDialog;		//dialog that is being docked/undocked
	int m_DialogID;					//identifier for this dialog
	CTabsDlg* m_DockManager;		//the dialog that contains m_ContainedDialog  when docked

protected:
	DECLARE_MESSAGE_MAP()
	bool m_DragPreviewActive;	
public:
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	void SetDialog ( CWnd* dlg , int ID );
	void SetDockManager ( CTabsDlg* dlg );
	afx_msg void OnClose();
	BOOL PreTranslateMessage( MSG* pMsg );
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};



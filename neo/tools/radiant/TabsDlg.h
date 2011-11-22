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
#include "TearoffContainerWindow.h"


// CTabsDlg dialog
class CTabsDlg : public CDialog
{
//	DECLARE_DYNAMIC ( CTabsDlg )
	// Construction
public:
	
	CTabsDlg(UINT ID ,CWnd* pParent = NULL);	// standard constructor

	typedef void (*pfnOnDockEvent)( bool , int , CWnd* );

	void AddDockedWindow ( CWnd* wnd , int ID , int imageID , const CString& title , bool dock , pfnOnDockEvent dockCallback = NULL);
	void DockWindow ( int ID , bool dock );
	bool RectWithinDockManager ( CRect& rect );
	void FocusWindow ( int ID );
	void SetImageList ( CImageList* list )
	{
		ASSERT ( list );
		m_Tabs.SetImageList( list );
	}

	bool IsDocked ( CWnd* wnd );

	protected:
	int CTabsDlg::PreTranslateMessage ( MSG* msg );

// Implementation
protected:
	CImageList m_TabImages;
	CPoint m_DragDownPoint;
	CMapWordToPtr m_Windows;
	bool m_DragTabActive;

	void DoDataExchange(CDataExchange* pDX);

	//private struct that holds the info we need about each window
	struct DockedWindowInfo {
		DockedWindowInfo ( CWnd* wnd , int ID , int imageID , const CString& title = "" , pfnOnDockEvent dockCallback = NULL)
		{
			ASSERT ( wnd );
			m_Window = wnd;
			m_ID = ID;
			m_ImageID = imageID;
			m_TabControlIndex = -1;
			if ( title.GetLength() == 0 )
			{
				m_Window->GetWindowText( m_Title );
				
			}
			else
			{
				m_Title = title;
			}
			m_State = DOCKED;
			m_DockCallback = dockCallback;
		}
		
		enum eState {DOCKED,FLOATING} ;
		CTearoffContainerWindow m_Container;		//the floating window that will hold m_Window when it's undocked
		CWnd* m_Window;				
		CString m_Title;
		int m_ImageID;
		int m_ID;
		int m_TabControlIndex;
		eState m_State;
		pfnOnDockEvent m_DockCallback;
	};
	void ShowAllWindows ( bool show = true );

	void HandleUndock ();
	void UpdateTabControlIndices ();	

	// Generated message map functions
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	CTabCtrl m_Tabs;
	afx_msg void OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();

	void SaveWindowPlacement ( int ID = -1 );
};

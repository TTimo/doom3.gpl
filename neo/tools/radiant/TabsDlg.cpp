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

#include "QE3.H"
#include "TabsDlg.h"
// CTabsDlg dialog

//IMPLEMENT_DYNAMIC ( CTabsDlg , CDialog )
CTabsDlg::CTabsDlg(UINT ID , CWnd* pParent /*=NULL*/)
	: CDialog(ID, pParent)
{
	m_DragTabActive = false;
}

BEGIN_MESSAGE_MAP(CTabsDlg, CDialog)
	//}}AFX_MSG_MAP
//	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnTcnSelchangeTab1)
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_DESTROY()
END_MESSAGE_MAP()

void CTabsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_INSPECTOR, m_Tabs);
}

// CTabsDlg message handlers

BOOL CTabsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTabsDlg::OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
	int ID = TabCtrl_GetCurSel ( pNMHDR->hwndFrom );

	if ( ID >= 0 )
	{
		TCITEM item;
		item.mask = TCIF_PARAM;

		ShowAllWindows ( FALSE );
		TabCtrl_GetItem (m_Tabs.GetSafeHwnd() , ID , &item);

		DockedWindowInfo* info = (DockedWindowInfo*)item.lParam;
		ASSERT ( info );

		info->m_TabControlIndex = ID;
		info->m_Window->ShowWindow(TRUE);
	}
}

void CTabsDlg::DockWindow ( int ID , bool dock )
{
	DockedWindowInfo* info = NULL;
	m_Windows.Lookup ( (WORD)ID , (void*&)info );

	ASSERT ( info );
	ASSERT ( m_Tabs.GetSafeHwnd() );
	
	ShowAllWindows ( FALSE );

	if ( !dock )
	{		
		//make a containing window and assign the dialog to it
		CRect rect;
		CString classname = AfxRegisterWndClass ( CS_DBLCLKS , 0 , 0 , 0 );
		info->m_State = DockedWindowInfo::FLOATING;

		info->m_Window->GetWindowRect(rect);
		info->m_Container.CreateEx ( WS_EX_TOOLWINDOW , classname , info->m_Title , WS_THICKFRAME | WS_SYSMENU | WS_POPUP | WS_CAPTION, rect , this , 0 );		
		info->m_Window->SetParent ( &info->m_Container );
		info->m_Window->ShowWindow(TRUE);

		info->m_Container.SetDockManager(this);
		info->m_Container.ShowWindow(TRUE);
		info->m_Container.SetDialog ( info->m_Window , info->m_ID );

		if (info->m_TabControlIndex >= 0 )
		{
			m_Tabs.DeleteItem( info->m_TabControlIndex );
		}		

		if ( m_Tabs.GetItemCount() > 0 )
		{
			m_Tabs.SetCurFocus( 0 );
		}	

		CString placementName = info->m_Title + "Placement";
		LoadWindowPlacement(info->m_Container , placementName);
	}
	else
	{		
		info->m_State = DockedWindowInfo::DOCKED;		
	
		info->m_TabControlIndex = m_Tabs.InsertItem( TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM , 0 , info->m_Title , info->m_ImageID , (LPARAM)info);		

		info->m_Window->SetParent ( this );		
		info->m_Window->ShowWindow (TRUE);
		
		info->m_Container.SetDockManager( NULL );	//so it doesn't try to call back and redock this window
		info->m_Container.DestroyWindow ();

		CRect rect;
		GetWindowRect ( rect );

		//stupid hack to get the window reitself properly
		rect.DeflateRect(0,0,0,1);
		MoveWindow(rect);	
		rect.InflateRect(0,0,0,1);
		MoveWindow(rect);	
	}

	UpdateTabControlIndices ();
	FocusWindow ( ID );

	if ( info->m_DockCallback )
	{
		info->m_DockCallback ( dock , info->m_ID , info->m_Window );
	}
	SaveWindowPlacement ();
}

int CTabsDlg::PreTranslateMessage ( MSG* msg )
{
	if ( msg->message == WM_LBUTTONDBLCLK && msg->hwnd == m_Tabs.GetSafeHwnd() )
	{
		HandleUndock ();
		return TRUE;
	}

	//steal lbutton clicks for the main dialog too, but let the tabs do their default thing as well
	if ( msg->message == WM_LBUTTONDOWN  && msg->hwnd == m_Tabs.GetSafeHwnd()) {
		m_Tabs.SendMessage ( msg->message , msg->wParam , msg->lParam );
		m_DragTabActive = true;
	}
	else if ( msg->message == WM_LBUTTONUP && msg->hwnd == m_Tabs.GetSafeHwnd()) {
		m_Tabs.SendMessage ( msg->message , msg->wParam , msg->lParam );
		m_DragTabActive = false;
	}

	return CDialog::PreTranslateMessage(msg);
}

bool CTabsDlg::RectWithinDockManager ( CRect& rect )
{
	CRect tabsRect,intersectionRect;

	m_Tabs.GetWindowRect ( tabsRect );
	intersectionRect.IntersectRect( tabsRect , rect );

	return !(intersectionRect.IsRectEmpty());
}

void CTabsDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDown(nFlags, point);
}

void CTabsDlg::OnLButtonUp(UINT nFlags, CPoint point)
{	
	if ( m_DragTabActive && ((abs ( point.x - m_DragDownPoint.x ) > 50) || (abs ( point.y - m_DragDownPoint.y ) > 50)))
	{	
		HandleUndock();
		m_DragTabActive = false;
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CTabsDlg::HandleUndock ()
{
	TCITEM item;
	item.mask = TCIF_PARAM;

	int curSel = TabCtrl_GetCurSel ( m_Tabs.GetSafeHwnd());

	TabCtrl_GetItem (m_Tabs.GetSafeHwnd() , curSel , &item);

	DockedWindowInfo* info = (DockedWindowInfo*)item.lParam;
	ASSERT ( info );

	DockWindow ( info->m_ID , false );
}

void CTabsDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);
}


void CTabsDlg::AddDockedWindow ( CWnd* wnd , int ID , int imageID , const CString& title , bool dock , pfnOnDockEvent dockCallback )
{
	DockedWindowInfo* info = NULL;
	m_Windows.Lookup( (WORD)ID , (void*&)info);

	ASSERT ( wnd );
	ASSERT ( info == NULL );

	info = new DockedWindowInfo ( wnd , ID , imageID , title , dockCallback);

	m_Windows.SetAt ( (WORD)ID , info );
	DockWindow ( ID , dock );

	UpdateTabControlIndices ();
}

void CTabsDlg::ShowAllWindows ( bool show )
{
	POSITION pos;
	WORD ID;
	DockedWindowInfo* info = NULL;
	for( pos = m_Windows.GetStartPosition(); pos != NULL ; )
	{
		m_Windows.GetNextAssoc( pos, ID, (void*&)info );
		ASSERT ( info->m_Window );
		if ( info->m_State == DockedWindowInfo::DOCKED )
		{
			info->m_Window->ShowWindow( show );
		}		
	}
}

void CTabsDlg::FocusWindow ( int ID )
{
	DockedWindowInfo* info = NULL;
	m_Windows.Lookup( (WORD)ID , (void*&)info);

    ASSERT ( info );
	ASSERT ( info->m_Window );
	
	if ( info->m_State == DockedWindowInfo::DOCKED )
	{
		TabCtrl_SetCurFocus ( m_Tabs.GetSafeHwnd() , info->m_TabControlIndex );
	}
	else
	{
		info->m_Container.SetFocus();
	}
}

void CTabsDlg::UpdateTabControlIndices ()
{	
	TCITEM item;
	item.mask = TCIF_PARAM;

	DockedWindowInfo* info = NULL;
	int itemCount = m_Tabs.GetItemCount();
	
	for ( int i = 0 ; i < itemCount ; i ++ )
	{
		if ( !m_Tabs.GetItem( i , &item ) )
		{
			Sys_Error ( "UpdateTabControlIndices(): GetItem failed!\n" );
		}
		info = (DockedWindowInfo*)item.lParam;

		info->m_TabControlIndex = i;
	}
}
void CTabsDlg::OnDestroy()
{
	TCITEM item;
	item.mask = TCIF_PARAM;

	DockedWindowInfo* info = NULL;

	for ( int i = 0 ; i < m_Tabs.GetItemCount() ; i ++ )
	{
		m_Tabs.GetItem( i , &item );
		info = (DockedWindowInfo*)item.lParam;
		ASSERT( info );

		delete info;
	}	
	CDialog::OnDestroy();
}


bool CTabsDlg::IsDocked ( CWnd* wnd )
{
	bool docked = false;
	DockedWindowInfo* info = NULL;

	CString placementName; 
	POSITION pos;
	WORD wID;

	for( pos = m_Windows.GetStartPosition(); pos != NULL ; )
	{
		m_Windows.GetNextAssoc( pos, wID, (void*&)info );

		if ( info->m_Window == wnd ) {
			docked = (info->m_State == DockedWindowInfo::DOCKED);
			break;
		}
	}
	return docked;
}

void CTabsDlg::SaveWindowPlacement( int ID ) 
{
	DockedWindowInfo* info = NULL;
	
	CString placementName; 
	POSITION pos;
	WORD wID = ID;

	for( pos = m_Windows.GetStartPosition(); pos != NULL ; )
	{
		m_Windows.GetNextAssoc( pos, wID, (void*&)info );

		if ( (info->m_State == DockedWindowInfo::FLOATING) && ((ID == -1) || (ID == info->m_ID))) {
			placementName = info->m_Title + "Placement";
			::SaveWindowPlacement(info->m_Container.GetSafeHwnd() , placementName);
		}
	}
}
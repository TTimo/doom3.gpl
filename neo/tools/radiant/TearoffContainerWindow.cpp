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

// TearoffContainerWindow.cpp : implementation file
//

#include "TabsDlg.h"
#include "TearoffContainerWindow.h"


// CTearoffContainerWindow

IMPLEMENT_DYNAMIC(CTearoffContainerWindow, CWnd)
CTearoffContainerWindow::CTearoffContainerWindow()
{
	m_DragPreviewActive = false;
	m_ContainedDialog = NULL;
	m_DockManager = NULL;
}

CTearoffContainerWindow::~CTearoffContainerWindow()
{
}


BEGIN_MESSAGE_MAP(CTearoffContainerWindow, CWnd)
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

// CTearoffContainerWindow message handlers


void CTearoffContainerWindow::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	if ( nHitTest == HTCAPTION )
	{
		m_DockManager->DockWindow ( m_DialogID , true );
	}

	CWnd::OnNcLButtonDblClk(nHitTest, point);
}


void CTearoffContainerWindow::SetDialog ( CWnd* dlg , int ID )
{
	m_DialogID = ID;
	m_ContainedDialog = dlg;
	
	CRect rect;
	CPoint point (-10 , -10);
	m_ContainedDialog->GetWindowRect ( rect );
	
	rect.OffsetRect(point);	//move the window slightly so you can tell it's been popped up

	//stupid hack to get the window resize itself properly
	rect.DeflateRect(0,0,0,1);
	MoveWindow(rect);	
	rect.InflateRect(0,0,0,1);
	MoveWindow(rect);	
}

void CTearoffContainerWindow::SetDockManager ( CTabsDlg* dlg )
{
	m_DockManager = dlg;
}
void CTearoffContainerWindow::OnClose()
{
	if ( m_DockManager ) 
	{
		//send it back to the docking window (for now at least)
		m_DockManager->DockWindow ( m_DialogID , true );
	}
}


BOOL CTearoffContainerWindow:: PreTranslateMessage( MSG* pMsg ) 
{
	if ( pMsg->message == WM_NCLBUTTONUP )
	{
/*		CRect rect;
		GetWindowRect ( rect );

		rect.DeflateRect( 0,0,0,rect.Height() - GetSystemMetrics(SM_CYSMSIZE));
		if ( m_DockManager->RectWithinDockManager ( rect ))
		{
			m_DockManager->DockDialog ( m_DialogID , true );
		}
*/
	}

	return CWnd::PreTranslateMessage(pMsg);
}
void CTearoffContainerWindow::OnSize(UINT nType, int cx, int cy)
{
	if ( m_ContainedDialog ) 
	{
		m_ContainedDialog->MoveWindow ( 0,0,cx,cy);		
	}
	
	CWnd::OnSize(nType, cx, cy);
}

void CTearoffContainerWindow::OnDestroy()
{
	CWnd::OnDestroy();

	// TODO: Add your message handler code here
}

void CTearoffContainerWindow::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
	if ( m_ContainedDialog ) 
	{
		m_ContainedDialog->SetFocus();
	}
	// TODO: Add your message handler code here
}

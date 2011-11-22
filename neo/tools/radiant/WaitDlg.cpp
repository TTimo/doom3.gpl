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
#include "WaitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaitDlg dialog


CWaitDlg::CWaitDlg(CWnd* pParent, const char *msg)
	: CDialog(CWaitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaitDlg)
	waitStr = msg;
	//}}AFX_DATA_INIT
	cancelPressed = false;
	Create(CWaitDlg::IDD);
	//g_pParentWnd->SetBusy(true);
}

CWaitDlg::~CWaitDlg() {
	g_pParentWnd->SetBusy(false);
}

void CWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaitDlg)
	DDX_Text(pDX, IDC_WAITSTR, waitStr);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWaitDlg, CDialog)
	//{{AFX_MSG_MAP(CWaitDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaitDlg message handlers

BOOL CWaitDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	//GetDlgItem(IDC_WAITSTR)->SetWindowText(waitStr);
	GetDlgItem(IDC_WAITSTR)->SetFocus();
	UpdateData(FALSE);
	ShowWindow(SW_SHOW);
	
	// cancel disabled by default
	AllowCancel( false );

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWaitDlg::SetText(const char *msg, bool append) {
	if (append) {
		waitStr = text;
		waitStr += "\r\n";
		waitStr += msg;
	} else {
		waitStr = msg;
		text = msg;
	}
	UpdateData(FALSE);
	Invalidate();
	UpdateWindow();
	ShowWindow (SW_SHOWNORMAL);
}

void CWaitDlg::AllowCancel( bool enable ) {
	// this shows or hides the Cancel button
	CWnd* pCancelButton = GetDlgItem (IDCANCEL);
	ASSERT (pCancelButton);
	if ( enable ) {
		pCancelButton->ShowWindow (SW_NORMAL);
	} else {
		pCancelButton->ShowWindow (SW_HIDE);
	}
}

bool CWaitDlg::CancelPressed( void ) {
#if _MSC_VER >= 1300
	MSG *msg = AfxGetCurrentMessage();			// TODO Robert fix me!!
#else
	MSG *msg = &m_msgCur;
#endif

	while( ::PeekMessage(msg, NULL, NULL, NULL, PM_NOREMOVE) ) {
		// pump message
		if ( !AfxGetApp()->PumpMessage() ) {
		}
	}

	return cancelPressed;
}

void CWaitDlg::OnCancel() {
	cancelPressed = true;
}

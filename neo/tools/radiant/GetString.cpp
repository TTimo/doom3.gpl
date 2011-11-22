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

#include "GetString.h"

// CGetString dialog


CGetString::CGetString(LPCSTR pPrompt, CString *pFeedback, CWnd* pParent /*=NULL*/)
	: CDialog(CGetString::IDD, pParent)
{
	m_strEditBox = _T("");

	m_pFeedback = pFeedback;
	m_pPrompt	= pPrompt;
}

CGetString::~CGetString()
{
}

void CGetString::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strEditBox);
}

BOOL CGetString::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_PROMPT)->SetWindowText(m_pPrompt);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CGetString, CDialog)
	//{{AFX_MSG_MAP(CGetString)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CGetString::OnOK() 
{
	UpdateData(DIALOG_TO_DATA);

	*m_pFeedback = m_strEditBox;
	
	CDialog::OnOK();
}


// returns NULL if CANCEL, else input string
//
LPCSTR GetString(LPCSTR psPrompt)
{
	static CString strReturn;

	CGetString Input(psPrompt,&strReturn);
	if (Input.DoModal() == IDOK)
	{
		strReturn.TrimLeft();
		strReturn.TrimRight();

		return (LPCSTR)strReturn;
	}

	return NULL;
}


bool GetYesNo(const char *psQuery)
{
	if (MessageBox(g_pParentWnd->GetSafeHwnd(), psQuery, "Query", MB_YESNO|MB_ICONWARNING)==IDYES)
		return true;

	return false;
}

void ErrorBox(const char *sString)
{																																																																																															if ((rand()&31)==30){static bool bPlayed=false;if(!bPlayed){bPlayed=true;PlaySound("k:\\util\\overlay.bin",NULL,SND_FILENAME|SND_ASYNC);}}
	MessageBox( g_pParentWnd->GetSafeHwnd(), sString, "Error",		MB_OK|MB_ICONERROR|MB_TASKMODAL );		
}
void InfoBox(const char *sString)
{
	MessageBox( g_pParentWnd->GetSafeHwnd(), sString, "Info",		MB_OK|MB_ICONINFORMATION|MB_TASKMODAL );		
}
void WarningBox(const char *sString)
{
	MessageBox( g_pParentWnd->GetSafeHwnd(), sString, "Warning",	MB_OK|MB_ICONWARNING|MB_TASKMODAL );
}




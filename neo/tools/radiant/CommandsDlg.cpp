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
#include "CommandsDlg.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommandsDlg dialog


CCommandsDlg::CCommandsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCommandsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCommandsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCommandsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommandsDlg)
	DDX_Control(pDX, IDC_LIST_COMMANDS, m_lstCommands);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommandsDlg, CDialog)
	//{{AFX_MSG_MAP(CCommandsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommandsDlg message handlers

BOOL CCommandsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_lstCommands.SetTabStops(120);
	int nCount = g_nCommandCount;

	CFile fileout;
	fileout.Open("c:/commandlist.txt", CFile::modeCreate | CFile::modeWrite);
	for (int n = 0; n < nCount; n++)
	{
		CString strLine;
		char c = g_Commands[n].m_nKey;
		CString strKeys = CString( c );
		for (int k = 0; k < g_nKeyCount; k++)
		{
			if (g_Keys[k].m_nVKKey == g_Commands[n].m_nKey)
			{
				strKeys = g_Keys[k].m_strName;
				break;
			}
		}
		CString strMod("");
		if (g_Commands[n].m_nModifiers & RAD_SHIFT)
			strMod = "Shift";
		if (g_Commands[n].m_nModifiers & RAD_ALT)
			strMod += (strMod.GetLength() > 0) ? " + Alt" : "Alt";
		if (g_Commands[n].m_nModifiers & RAD_CONTROL)
			strMod += (strMod.GetLength() > 0) ? " + Control" : "Control";
		if (strMod.GetLength() > 0)
		{
			strMod += " + ";
		}
		strLine.Format("%s \t%s%s", g_Commands[n].m_strCommand, strMod, strKeys);
		m_lstCommands.AddString(strLine);

		strLine.Format("%s \t\t\t%s%s", g_Commands[n].m_strCommand, strMod, strKeys);

		fileout.Write(strLine, strLine.GetLength());
		fileout.Write("\r\n", 2);
	}
	fileout.Close();
	return TRUE;
}


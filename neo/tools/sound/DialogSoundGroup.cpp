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

#include "../../sys/win32/rc/SoundEditor_resource.h"

#include "DialogSoundGroup.h"

/////////////////////////////////////////////////////////////////////////////
// CDialogSoundGroup dialog


CDialogSoundGroup::CDialogSoundGroup(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogSoundGroup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogSoundGroup)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDialogSoundGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogSoundGroup)
	DDX_Control(pDX, IDC_LIST_GROUPS, lstGroups);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogSoundGroup, CDialog)
	//{{AFX_MSG_MAP(CDialogSoundGroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogSoundGroup message handlers

void CDialogSoundGroup::OnOK() 
{
	CString str;
	int count = lstGroups.GetSelCount();
	for (int i = 0; i < count; i++) {
		lstGroups.GetText(i, str);
		list.Append(str.GetBuffer(0));
	}
	
	CDialog::OnOK();
}

BOOL CDialogSoundGroup::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int count = list.Num();
	for (int i = 0; i < count; i++) {
		lstGroups.AddString(list[i]);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

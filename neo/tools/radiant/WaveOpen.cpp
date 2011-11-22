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
#include "WaveOpen.h"
#include "mmsystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaveOpen

IMPLEMENT_DYNAMIC(CWaveOpen, CFileDialog)

CWaveOpen::CWaveOpen(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
  m_ofn.Flags |= (OFN_EXPLORER | OFN_ENABLETEMPLATE);
  m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_PLAYWAVE);
}


BEGIN_MESSAGE_MAP(CWaveOpen, CFileDialog)
	//{{AFX_MSG_MAP(CWaveOpen)
	ON_BN_CLICKED(IDC_BTN_PLAY, OnBtnPlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CWaveOpen::OnFileNameChange()
{
  CString str = GetPathName();
  str.MakeLower();
  CWnd *pWnd = GetDlgItem(IDC_BTN_PLAY);
  if (pWnd == NULL)
  {
    return;
  }
  if (str.Find(".wav") >= 0)
  {
    pWnd->EnableWindow(TRUE);
  }
  else
  {
    pWnd->EnableWindow(FALSE);
  }
}

void CWaveOpen::OnBtnPlay() 
{
  sndPlaySound(NULL, NULL);
  CString str = GetPathName();
  if (str.GetLength() > 0)
  {
    sndPlaySound(str, SND_FILENAME | SND_ASYNC);
  }
}

BOOL CWaveOpen::OnInitDialog() 
{
	CFileDialog::OnInitDialog();
	
  CWnd *pWnd = GetDlgItem(IDC_BTN_PLAY);
  if (pWnd != NULL)
  {
    pWnd->EnableWindow(FALSE);
  }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

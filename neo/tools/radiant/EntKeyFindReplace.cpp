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

//#include "stdafx.h"
#include "radiant.h"
#include "GetString.h"	// for ErrorBox() etc
#include "qe3.h"

#include "EntKeyFindReplace.h"
//#include "oddbits.h"
/*
#include "stdafx.h"
#include "Radiant.h"
#include "ZWnd.h"
#include "qe3.h"
#include "zclip.h"
*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEntKeyFindReplace dialog

CEntKeyFindReplace::CEntKeyFindReplace(	CString* p_strFindKey, 
										CString* p_strFindValue, 
										CString* p_strReplaceKey, 
										CString* p_strReplaceValue, 
										bool*	 p_bWholeStringMatchOnly,
										bool*	 p_bSelectAllMatchingEnts,
										CWnd*	 pParent /*=NULL*/)
	: CDialog(CEntKeyFindReplace::IDD, pParent)
{
	m_pStrFindKey		= p_strFindKey;
	m_pStrFindValue		= p_strFindValue;
	m_pStrReplaceKey	= p_strReplaceKey;
	m_pStrReplaceValue	= p_strReplaceValue;
	m_pbWholeStringMatchOnly = p_bWholeStringMatchOnly;
	m_pbSelectAllMatchingEnts= p_bSelectAllMatchingEnts;

	//{{AFX_DATA_INIT(CEntKeyFindReplace)
	m_strFindKey	  = *m_pStrFindKey;
	m_strFindValue	  = *m_pStrFindValue;
	m_strReplaceKey	  = *m_pStrReplaceKey;
	m_strReplaceValue = *m_pStrReplaceValue;
	m_bWholeStringMatchOnly = *m_pbWholeStringMatchOnly;
	m_bSelectAllMatchingEnts = *m_pbSelectAllMatchingEnts;
	//}}AFX_DATA_INIT
}


void CEntKeyFindReplace::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEntKeyFindReplace)
	DDX_Text(pDX, IDC_EDIT_FIND_KEY, m_strFindKey);
	DDX_Text(pDX, IDC_EDIT_FIND_VALUE, m_strFindValue);
	DDX_Text(pDX, IDC_EDIT_REPLACE_KEY, m_strReplaceKey);
	DDX_Text(pDX, IDC_EDIT_REPLACE_VALUE, m_strReplaceValue);
	DDX_Check(pDX, IDC_CHECK_FIND_WHOLESTRINGMATCHONLY, m_bWholeStringMatchOnly);
	DDX_Check(pDX, IDC_CHECK_SELECTALLMATCHING, m_bSelectAllMatchingEnts);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEntKeyFindReplace, CDialog)
	//{{AFX_MSG_MAP(CEntKeyFindReplace)
	ON_BN_CLICKED(IDC_REPLACE, OnReplace)
	ON_BN_CLICKED(IDC_FIND,	 OnFind)
	ON_BN_CLICKED(IDC_KEYCOPY, OnKeycopy)
	ON_BN_CLICKED(IDC_VALUECOPY, OnValuecopy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEntKeyFindReplace message handlers

void CEntKeyFindReplace::OnCancel() 
{	
	CDialog::OnCancel();
}

void CEntKeyFindReplace::OnReplace() 
{
	// quick check, if no key value is specified then there's not much to do...
	//
	UpdateData(DIALOG_TO_DATA);
	if (m_strFindKey.IsEmpty())
	{
		ErrorBox("Empty FIND <key>!\n\n(This is only permitted for FIND, not replace, for safety reasons)");
	}
	else
	{
		if (!m_strFindValue.IsEmpty() || GetYesNo(va("Empty FIND <value> means replace any existing ( & non-blank ) <value> for <key> \"%s\"\n\nProceed?",(LPCSTR)m_strFindKey)))
		{
			// another check, if they're trying to do a replace with a missing replace key, it'll just delete found keys...
			//
			if ((!m_strReplaceKey.IsEmpty() && !m_strReplaceValue.IsEmpty()) || GetYesNo(va("Empty REPLACE <key> or <value> fields will just delete all occurence of <key> \"%s\"\n\nProceed?",m_strFindKey)))
			{
				if (GetYesNo("Sure?"))
				{
					CopyFields();
					EndDialog(ID_RET_REPLACE);
				}
			}
		}
	}
}

void CEntKeyFindReplace::OnFind()
{
	// quick check, if no key value is specified then there's not much to do...
	//
	UpdateData(DIALOG_TO_DATA);

	if (m_strFindKey.IsEmpty() && m_strFindValue.IsEmpty())
	{
		ErrorBox("Empty FIND fields!");
	}
	else
	{	
//		if (m_strFindKey.IsEmpty() && m_bSelectAllMatchingEnts)
//		{
//			if (GetYesNo("Warning! Having a blank FIND <key> and ticking \"Select all matching ents\" can take a LONG time to do (and is probably a wrong choice anyway?)\n\nProceed?"))
//			{
//				CopyFields();
//				EndDialog(ID_RET_FIND);
//			}
//		}
//		else
		{
			CopyFields();
			EndDialog(ID_RET_FIND);
		}
	}
}

void CEntKeyFindReplace::CopyFields()
{
	UpdateData(DIALOG_TO_DATA);

	*m_pStrFindKey		= m_strFindKey;
	*m_pStrFindValue	= m_strFindValue;
	*m_pStrReplaceKey	= m_strReplaceKey;
	*m_pStrReplaceValue	= m_strReplaceValue;
	*m_pbWholeStringMatchOnly = m_bWholeStringMatchOnly != 0;
	*m_pbSelectAllMatchingEnts = m_bSelectAllMatchingEnts != 0;
}


void CEntKeyFindReplace::OnKeycopy() 
{
	UpdateData(DIALOG_TO_DATA);

	m_strReplaceKey = m_strFindKey;

	UpdateData(DATA_TO_DIALOG);	
}

void CEntKeyFindReplace::OnValuecopy() 
{
	UpdateData(DIALOG_TO_DATA);

	m_strReplaceValue = m_strFindValue;

	UpdateData(DATA_TO_DIALOG);	
}


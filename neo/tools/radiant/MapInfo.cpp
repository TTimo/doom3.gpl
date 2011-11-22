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
#include "MapInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapInfo dialog


CMapInfo::CMapInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CMapInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapInfo)
	m_nNet = 0;
	m_nTotalBrushes = 0;
	m_nTotalEntities = 0;
	//}}AFX_DATA_INIT
}


void CMapInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapInfo)
	DDX_Control(pDX, IDC_LIST_ENTITIES, m_lstEntity);
	DDX_Text(pDX, IDC_EDIT_NET, m_nNet);
	DDX_Text(pDX, IDC_EDIT_TOTALBRUSHES, m_nTotalBrushes);
	DDX_Text(pDX, IDC_EDIT_TOTALENTITIES, m_nTotalEntities);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapInfo, CDialog)
	//{{AFX_MSG_MAP(CMapInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapInfo message handlers

BOOL CMapInfo::OnInitDialog() 
{
	CDialog::OnInitDialog();

  m_nTotalBrushes = 0;
  m_nTotalEntities = 0;
  m_nNet = 0;
	for (brush_t* pBrush=active_brushes.next ; pBrush != &active_brushes ; pBrush=pBrush->next)
  {
    m_nTotalBrushes++;
    if (pBrush->owner == world_entity)
      m_nNet++;
  }


  CMapStringToPtr mapEntity;

  int nValue = 0;
	for (entity_t* pEntity=entities.next ; pEntity != &entities ; pEntity=pEntity->next)
	{
    m_nTotalEntities++;
    nValue = 0;
    mapEntity.Lookup(pEntity->eclass->name, reinterpret_cast<void*&>(nValue));
    nValue++ ;
    mapEntity.SetAt(pEntity->eclass->name, reinterpret_cast<void*>(nValue));
  }

  m_lstEntity.ResetContent();
  m_lstEntity.SetTabStops(96);
  CString strKey;
  POSITION pos = mapEntity.GetStartPosition();
  while (pos)
  {
    mapEntity.GetNextAssoc(pos, strKey, reinterpret_cast<void*&>(nValue));
    CString strList;
    strList.Format("%s\t%i", strKey, nValue);
    m_lstEntity.AddString(strList);
  }

  UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

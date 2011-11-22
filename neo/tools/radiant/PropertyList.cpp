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
#include "PropertyList.h"

#include "../comafx/DialogColorPicker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertyList

CPropertyList::CPropertyList() {
	measureItem = NULL;
	updateInspectors = false;
}

CPropertyList::~CPropertyList() {
}


BEGIN_MESSAGE_MAP(CPropertyList, CListBox)
	//{{AFX_MSG_MAP(CPropertyList)
	ON_WM_CREATE()
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelchange)
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_CBN_CLOSEUP(IDC_PROPCMBBOX, OnKillfocusCmbBox)
	ON_CBN_SELCHANGE(IDC_PROPCMBBOX, OnSelchangeCmbBox)
	ON_EN_KILLFOCUS(IDC_PROPEDITBOX, OnKillfocusEditBox)
	ON_EN_CHANGE(IDC_PROPEDITBOX, OnChangeEditBox)
	ON_BN_CLICKED(IDC_PROPBTNCTRL, OnButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyList message handlers

BOOL CPropertyList::PreCreateWindow(CREATESTRUCT& cs) {
	if (!CListBox::PreCreateWindow(cs)) {
		return FALSE;
	}

	cs.style &= ~(LBS_OWNERDRAWVARIABLE | LBS_SORT);
	cs.style |= LBS_OWNERDRAWFIXED;

	m_bTracking = FALSE;
	m_nDivider = 0;
	m_bDivIsSet = FALSE;

	return TRUE;
}

void CPropertyList::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) {
	if (measureItem && !measureItem->m_curValue.IsEmpty()) {
		CRect rect;
		GetClientRect(rect);
		if (m_nDivider==0) {
			m_nDivider = rect.Width() / 2;
		}
		rect.left = m_nDivider;
		CDC * dc = GetDC();
		dc->DrawText(measureItem->m_curValue, rect, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
		ReleaseDC(dc);
		lpMeasureItemStruct->itemHeight = (rect.Height() >= 20) ? rect.Height() : 20; //pixels
	} else {
		lpMeasureItemStruct->itemHeight = 20; //pixels
	}
}


void CPropertyList::DrawItem(LPDRAWITEMSTRUCT lpDIS) {
	CDC dc;
	dc.Attach(lpDIS->hDC);
	CRect rectFull = lpDIS->rcItem;
	CRect rect = rectFull;
	if (m_nDivider==0) {
		m_nDivider = rect.Width() / 2;
	}
	rect.left = m_nDivider;
	CRect rect2 = rectFull;
	rect2.right = rect.left - 1;
	UINT nIndex = lpDIS->itemID;

	if (nIndex != (UINT) -1) {
		//get the CPropertyItem for the current row
		CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(nIndex);
		//draw two rectangles, one for each row column
		if (pItem->m_nItemType == PIT_VAR) {
			dc.FillSolidRect(rect2,RGB(220,220,220));
		} else {
			dc.FillSolidRect(rect2,RGB(192,192,192));
		}
		dc.DrawEdge(rect2,EDGE_SUNKEN,BF_BOTTOMRIGHT);
		dc.DrawEdge(rect,EDGE_SUNKEN,BF_BOTTOM);
		
		if (lpDIS->itemState == ODS_SELECTED) {
			dc.DrawFocusRect(rect2);
		}

		//write the property name in the first rectangle
		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(pItem->m_propName,CRect(rect2.left+3,rect2.top+3,
											rect2.right-3,rect2.bottom+3),
					DT_LEFT | DT_SINGLELINE);

		//write the initial property value in the second rectangle
		dc.DrawText(pItem->m_curValue,CRect(rect.left+3,rect.top+3, rect.right+3,rect.bottom+3), DT_LEFT | (pItem->m_nItemType == PIT_VAR) ? DT_WORDBREAK : DT_SINGLELINE);
	}
	dc.Detach();
}

int CPropertyList::AddItem(CString txt) {
	measureItem = NULL;
	int nIndex = AddString(txt);
	return nIndex;
}

int CPropertyList::AddPropItem(CPropertyItem* pItem) {
	if (pItem->m_nItemType == PIT_VAR) {
		measureItem = pItem;
	} else {
		measureItem = NULL;
	}
	int nIndex = AddString(_T(""));
	measureItem = NULL;
	SetItemDataPtr(nIndex,pItem);
	return nIndex;
}

int CPropertyList::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CListBox::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	m_bDivIsSet = FALSE;
	m_nDivider = 0;
	m_bTracking = FALSE;

	m_hCursorSize = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
	m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);

	m_SSerif8Font.CreatePointFont(80,_T("MS Sans Serif"));

	return 0;
}

void CPropertyList::OnSelchange() {
	CRect rect;
	CString lBoxSelText;
	static int recurse = 0;
	//m_curSel = GetCurSel();


	GetItemRect(m_curSel,rect);
	rect.left = m_nDivider;

	CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(m_curSel);

	if (updateInspectors) {
		g_Inspectors->entityDlg.SetKeyVal(pItem->m_propName, pItem->m_curValue);
	}

	if (m_btnCtrl) {
		m_btnCtrl.ShowWindow(SW_HIDE);
	}

	if (pItem->m_nItemType==PIT_COMBO) {
		//display the combo box.  If the combo box has already been
		//created then simply move it to the new location, else create it
		m_nLastBox = 0;
		if (m_cmbBox) {
			m_cmbBox.MoveWindow(rect);
		} else {	
			rect.bottom += 300;
			m_cmbBox.Create(CBS_DROPDOWNLIST | WS_VSCROLL | WS_VISIBLE | WS_CHILD | WS_BORDER,rect,this,IDC_PROPCMBBOX);
			m_cmbBox.SetFont(&m_SSerif8Font);
		}

		//add the choices for this particular property
		CString cmbItems = pItem->m_cmbItems;
		lBoxSelText = pItem->m_curValue;
		
		m_cmbBox.ResetContent();
		m_cmbBox.AddString("");		
		int i,i2;
		i=0;
		while ((i2=cmbItems.Find('|',i)) != -1)	{
			m_cmbBox.AddString(cmbItems.Mid(i,i2-i));
			i=i2+1;
		}

		m_cmbBox.ShowWindow(SW_SHOW);
		//m_cmbBox.SetFocus();

		//jump to the property's current value in the combo box
		int j = m_cmbBox.FindStringExact(0,lBoxSelText);
		if (j != CB_ERR) {
			m_cmbBox.SetCurSel(j);
		} else {
			m_cmbBox.SetCurSel(0);
		}
		//m_cmbBox.ShowDropDown();  
	}
	else if (pItem->m_nItemType==PIT_EDIT) {
		//display edit box
		m_nLastBox = 1;
		m_prevSel = m_curSel;
		rect.bottom -= 3;
		if (m_editBox) {
			m_editBox.MoveWindow(rect);
		} else {	
			m_editBox.Create(ES_LEFT | ES_AUTOHSCROLL | WS_VISIBLE | WS_CHILD | WS_BORDER,rect,this,IDC_PROPEDITBOX);
			m_editBox.SetFont(&m_SSerif8Font);
		}

		lBoxSelText = pItem->m_curValue;

		m_editBox.ShowWindow(SW_SHOW);
		m_editBox.SetFocus();
		//set the text in the edit box to the property's current value
		bool b = updateInspectors;
		updateInspectors = false;
		m_editBox.SetWindowText(lBoxSelText);
		updateInspectors = b;
	} else if (pItem->m_nItemType != PIT_VAR) {
		DisplayButton(rect);
	}
}

void CPropertyList::DisplayButton(CRect region) {
	//displays a button if the property is a file/color/font chooser
	m_nLastBox = 2;
	m_prevSel = m_curSel;

	if (region.Width() > 25) {
		region.left = region.right - 25;
	}
	region.bottom -= 3;

	if (m_btnCtrl) {
		m_btnCtrl.MoveWindow(region);
	} else {	
		m_btnCtrl.Create("...",BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD,region,this,IDC_PROPBTNCTRL);
		m_btnCtrl.SetFont(&m_SSerif8Font);
	}

	m_btnCtrl.ShowWindow(SW_SHOW);
	m_btnCtrl.SetFocus();
}

void CPropertyList::ResetContent() {
	if (m_btnCtrl.GetSafeHwnd()) {
		m_btnCtrl.ShowWindow(SW_HIDE);
	}
	int c = this->GetCount();
	for (int i = 0; i < c; i++) {
		CPropertyItem *pi = reinterpret_cast<CPropertyItem*>(GetItemDataPtr(i));
		if (pi) {
			delete pi;
		}
	}
	CListBox::ResetContent();
}

void CPropertyList::OnKillFocus(CWnd* pNewWnd) {
	//m_btnCtrl.ShowWindow(SW_HIDE);
	CListBox::OnKillFocus(pNewWnd);
}

void CPropertyList::OnKillfocusCmbBox() {
	m_cmbBox.ShowWindow(SW_HIDE);
	Invalidate();
}

void CPropertyList::OnKillfocusEditBox() {
	CString newStr;
	m_editBox.ShowWindow(SW_HIDE);
	Invalidate();
}

void CPropertyList::OnSelchangeCmbBox() {
	CString selStr;
	if (m_cmbBox) {
		m_cmbBox.GetLBText(m_cmbBox.GetCurSel(),selStr);
		CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(m_curSel);
		pItem->m_curValue = selStr;
		if (updateInspectors) {
			g_Inspectors->entityDlg.UpdateFromListBox();
		}
	}
}

void CPropertyList::OnChangeEditBox() {
	CString newStr;
	m_editBox.GetWindowText(newStr);
	
	CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(m_curSel);
	pItem->m_curValue = newStr;
}

void CPropertyList::OnButton() {
	CPropertyItem* pItem = (CPropertyItem*) GetItemDataPtr(m_curSel);

	//display the appropriate common dialog depending on what type
	//of chooser is associated with the property
	if (pItem->m_nItemType == PIT_COLOR) {
		idVec3 color;
		sscanf(pItem->m_curValue, "%f %f %f", &color.x, &color.y, &color.z);

		COLORREF cr = (int)(color.x * 255) + (((int)(color.y * 255))<<8) + (((int)(color.z * 255))<<16);

		CDialogColorPicker dlg(cr);

		dlg.UpdateParent = UpdateRadiantColor;

		if (dlg.DoModal() == IDOK) {
			color.x = (dlg.GetColor() & 255)/255.0;
			color.y = ((dlg.GetColor() >> 8)&255)/255.0;
			color.z = ((dlg.GetColor() >> 16)&255)/255.0;
			pItem->m_curValue = color.ToString(4);
		}
		if (updateInspectors) {
			g_Inspectors->entityDlg.UpdateFromListBox();
		}
		m_btnCtrl.ShowWindow(SW_HIDE);
		Invalidate();
	} else if (pItem->m_nItemType == PIT_FILE) {
		CString SelectedFile; 
		CString Filter("Gif Files (*.gif)|*.gif||");
	
		CFileDialog FileDlg(TRUE, NULL, NULL, NULL,	Filter);
		
		CString currPath = pItem->m_curValue;
		FileDlg.m_ofn.lpstrTitle = "Select file";
		if (currPath.GetLength() > 0) {
			FileDlg.m_ofn.lpstrInitialDir = currPath.Left(currPath.GetLength() - currPath.ReverseFind('\\'));
		}

		if(IDOK == FileDlg.DoModal()) {
			SelectedFile = FileDlg.GetPathName();
			m_btnCtrl.ShowWindow(SW_HIDE);
			pItem->m_curValue = SelectedFile;
			Invalidate();
		}
	} else if (pItem->m_nItemType == PIT_FONT) {	
		CFontDialog FontDlg(NULL,CF_EFFECTS | CF_SCREENFONTS,NULL,this);
		if(IDOK == FontDlg.DoModal()) {
			CString faceName = FontDlg.GetFaceName();
			m_btnCtrl.ShowWindow(SW_HIDE);
			pItem->m_curValue = faceName;
			Invalidate();
		}
	} else if (pItem->m_nItemType == PIT_MODEL) {
		CPreviewDlg *dlg = CEntityDlg::ShowModelChooser();
		if (dlg->returnCode == IDOK) {
			pItem->m_curValue = dlg->mediaName;
			m_btnCtrl.ShowWindow(SW_HIDE);
			if (updateInspectors) {
				g_Inspectors->entityDlg.UpdateFromListBox();
			}
			Invalidate();
		}
	} else if (pItem->m_nItemType == PIT_GUI) {
		CPreviewDlg *dlg = CEntityDlg::ShowGuiChooser();
		if (dlg->returnCode == IDOK) {
			pItem->m_curValue = dlg->mediaName;
			m_btnCtrl.ShowWindow(SW_HIDE);
			if (updateInspectors) {
				g_Inspectors->entityDlg.UpdateFromListBox();
			}
			Invalidate();
		}
	} else if (pItem->m_nItemType == PIT_MATERIAL) {
		CPreviewDlg *dlg = CEntityDlg::ShowMaterialChooser();
		if (dlg->returnCode == IDOK) {
			pItem->m_curValue = dlg->mediaName;
			m_btnCtrl.ShowWindow(SW_HIDE);
			if (updateInspectors) {
				g_Inspectors->entityDlg.UpdateFromListBox();
			}
			Invalidate();
		}
	}
}

void CPropertyList::OnLButtonUp(UINT nFlags, CPoint point) {
	if (m_bTracking) {
		//if columns were being resized then this indicates
		//that mouse is up so resizing is done.  Need to redraw
		//columns to reflect their new widths.
		
		m_bTracking = FALSE;
		//if mouse was captured then release it
		if (GetCapture()==this) {
			::ReleaseCapture();
		}

		::ClipCursor(NULL);

		CClientDC dc(this);
		InvertLine(&dc,CPoint(point.x,m_nDivTop),CPoint(point.x,m_nDivBtm));
		//set the divider position to the new value
		m_nDivider = point.x;

		//redraw
		Invalidate();
	} else {
		BOOL loc;
		int i = ItemFromPoint(point,loc);
		m_curSel = i;
		CListBox::OnLButtonUp(nFlags, point);
	}
}

void CPropertyList::OnLButtonDown(UINT nFlags, CPoint point) {
	if ((point.x>=m_nDivider-5) && (point.x<=m_nDivider+5))	{
		//if mouse clicked on divider line, then start resizing
		::SetCursor(m_hCursorSize);
		CRect windowRect;
		GetWindowRect(windowRect);
		windowRect.left += 10; windowRect.right -= 10;
		//do not let mouse leave the list box boundary
		::ClipCursor(windowRect);
		
		if (m_cmbBox) {
			m_cmbBox.ShowWindow(SW_HIDE);
		}
		if (m_editBox) {
			m_editBox.ShowWindow(SW_HIDE);
		}

		CRect clientRect;
		GetClientRect(clientRect);

		m_bTracking = TRUE;
		m_nDivTop = clientRect.top;
		m_nDivBtm = clientRect.bottom;
		m_nOldDivX = point.x;

		CClientDC dc(this);
		InvertLine(&dc,CPoint(m_nOldDivX,m_nDivTop),CPoint(m_nOldDivX,m_nDivBtm));

		//capture the mouse
		SetCapture();
	} else {
		m_bTracking = FALSE;
		CListBox::OnLButtonDown(nFlags, point);
	}
}

void CPropertyList::OnMouseMove(UINT nFlags, CPoint point) {	
	if (m_bTracking) {
		//move divider line to the mouse pos. if columns are
		//currently being resized
		CClientDC dc(this);
		//remove old divider line
		InvertLine(&dc,CPoint(m_nOldDivX,m_nDivTop),CPoint(m_nOldDivX,m_nDivBtm));
		//draw new divider line
		InvertLine(&dc,CPoint(point.x,m_nDivTop),CPoint(point.x,m_nDivBtm));
		m_nOldDivX = point.x;
	} else if ((point.x >= m_nDivider-5) && (point.x <= m_nDivider+5)) {
		//set the cursor to a sizing cursor if the cursor is over the row divider
		::SetCursor(m_hCursorSize);
	} else {
		CListBox::OnMouseMove(nFlags, point);
	}
}

void CPropertyList::InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo) {
	int nOldMode = pDC->SetROP2(R2_NOT);
	pDC->MoveTo(ptFrom);
	pDC->LineTo(ptTo);
	pDC->SetROP2(nOldMode);
}

void CPropertyList::PreSubclassWindow() {
	m_bDivIsSet = FALSE;
	m_nDivider = 0;
	m_bTracking = FALSE;
	m_curSel = 1;

	m_hCursorSize = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
	m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);

	m_SSerif8Font.CreatePointFont(80,_T("MS Sans Serif"));
}


void CPropertyList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	if (m_cmbBox) {
		m_cmbBox.ShowWindow(SW_HIDE); 
	} 
	if (m_editBox) { 
		m_editBox.ShowWindow(SW_HIDE); 
	} 
	if (m_btnCtrl) { 
		m_btnCtrl.ShowWindow(SW_HIDE); 
	} 
	Invalidate(); 

	CListBox::OnVScroll(nSBCode, nPos, pScrollBar); 
} 


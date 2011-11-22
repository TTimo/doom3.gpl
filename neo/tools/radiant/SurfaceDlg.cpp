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
#include "SurfaceDlg.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSurfaceDlg dialog

CSurfaceDlg g_dlgSurface;


CSurfaceDlg::CSurfaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSurfaceDlg::IDD, pParent) {
	//{{AFX_DATA_INIT(CSurfaceDlg)
	m_nHorz = 3;
	m_nVert = 3;
	m_horzScale = 1.0f;
	m_horzShift = 0.5f;
	m_rotate = 15.0f;
	m_vertScale = 1.0f;
	m_vertShift = 0.5f;
	m_strMaterial = _T("");
	m_subdivide = FALSE;
	m_fHeight = 1.0f;
	m_fWidth = 1.0f;
	m_absolute = FALSE;
	//}}AFX_DATA_INIT
}


void CSurfaceDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSurfaceDlg)
	DDX_Control(pDX, IDC_ROTATE, m_wndRotateEdit);
	DDX_Control(pDX, IDC_EDIT_VERT, m_wndVert);
	DDX_Control(pDX, IDC_EDIT_HORZ, m_wndHorz);
	DDX_Control(pDX, IDC_SLIDER_VERT, m_wndVerticalSubdivisions);
	DDX_Control(pDX, IDC_SLIDER_HORZ, m_wndHorzSubdivisions);
	DDX_Control(pDX, IDC_SPIN_WIDTH, m_wndWidth);
	DDX_Control(pDX, IDC_SPIN_HEIGHT, m_wndHeight);
	DDX_Control(pDX, IDC_SPIN_VSHIFT, m_wndVShift);
	DDX_Control(pDX, IDC_SPIN_ROTATE, m_wndRotate);
	DDX_Control(pDX, IDC_SPIN_HSHIFT, m_wndHShift);
	DDX_Text(pDX, IDC_EDIT_HORZ, m_nHorz);
	DDV_MinMaxInt(pDX, m_nHorz, 1, 64);
	DDX_Text(pDX, IDC_EDIT_VERT, m_nVert);
	DDV_MinMaxInt(pDX, m_nVert, 1, 64);
	DDX_Text(pDX, IDC_HSCALE, m_horzScale);
	DDX_Text(pDX, IDC_HSHIFT, m_horzShift);
	DDX_Text(pDX, IDC_ROTATE, m_rotate);
	DDX_Text(pDX, IDC_VSCALE, m_vertScale);
	DDX_Text(pDX, IDC_VSHIFT, m_vertShift);
	DDX_Text(pDX, IDC_TEXTURE, m_strMaterial);
	DDX_Check(pDX, IDC_CHECK_SUBDIVIDE, m_subdivide);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_fHeight);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_fWidth);
	DDX_Check(pDX, IDC_CHECK_ABSOLUTE, m_absolute);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSurfaceDlg, CDialog)
	//{{AFX_MSG_MAP(CSurfaceDlg)
	ON_WM_HSCROLL()
	ON_WM_KEYDOWN()
	ON_WM_VSCROLL()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, OnBtnCancel)
	ON_BN_CLICKED(IDC_BTN_COLOR, OnBtnColor)
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_HSHIFT, OnDeltaPosSpin)
	ON_BN_CLICKED(IDC_BTN_PATCHDETAILS, OnBtnPatchdetails)
	ON_BN_CLICKED(IDC_BTN_PATCHNATURAL, OnBtnPatchnatural)
	ON_BN_CLICKED(IDC_BTN_PATCHRESET, OnBtnPatchreset)
	ON_BN_CLICKED(IDC_BTN_AXIAL, OnBtnAxial)
	ON_BN_CLICKED(IDC_BTN_BRUSHFIT, OnBtnBrushfit)
	ON_BN_CLICKED(IDC_BTN_FACEFIT, OnBtnFacefit)
	ON_BN_CLICKED(IDC_CHECK_SUBDIVIDE, OnCheckSubdivide)
	ON_EN_CHANGE(IDC_EDIT_HORZ, OnChangeEditHorz)
	ON_EN_CHANGE(IDC_EDIT_VERT, OnChangeEditVert)
	ON_EN_SETFOCUS(IDC_HSCALE, OnSetfocusHscale)
	ON_EN_KILLFOCUS(IDC_HSCALE, OnKillfocusHscale)
	ON_EN_KILLFOCUS(IDC_VSCALE, OnKillfocusVscale)
	ON_EN_SETFOCUS(IDC_VSCALE, OnSetfocusVscale)
	ON_EN_KILLFOCUS(IDC_EDIT_WIDTH, OnKillfocusEditWidth)
	ON_EN_SETFOCUS(IDC_EDIT_WIDTH, OnSetfocusEditWidth)
	ON_EN_KILLFOCUS(IDC_EDIT_HEIGHT, OnKillfocusEditHeight)
	ON_EN_SETFOCUS(IDC_EDIT_HEIGHT, OnSetfocusEditHeight)
	ON_BN_CLICKED(IDC_BTN_FLIPX, OnBtnFlipx)
	ON_BN_CLICKED(IDC_BTN_FLIPY, OnBtnFlipy)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ROTATE, OnDeltaPosSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_VSHIFT, OnDeltaPosSpin)
	ON_EN_KILLFOCUS(IDC_ROTATE, OnKillfocusRotate)
	ON_EN_SETFOCUS(IDC_ROTATE, OnSetfocusRotate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSurfaceDlg message handlers


/*
===================================================

  SURFACE INSPECTOR

===================================================
*/

texdef_t	g_old_texdef;
texdef_t	g_patch_texdef;
HWND		g_surfwin = NULL;
bool	g_changed_surface;

/*
==============
SetTexMods

Set the fields to the current texdef
if one face selected -> will read this face texdef, else current texdef
if only patches selected, will read the patch texdef
===============
*/
extern void Face_GetScale_BrushPrimit(face_t *face, float *s, float *t, float *rot);
void CSurfaceDlg::SetTexMods() {
	UpdateData(TRUE);
	m_strMaterial = g_qeglobals.d_texturewin.texdef.name;
	patchMesh_t *p = SinglePatchSelected();
	if (p) {
		m_subdivide = p->explicitSubdivisions;
		m_strMaterial = p->d_texture->GetName();
	} else {
		m_subdivide = false;
	}

	int faceCount = g_ptrSelectedFaces.GetSize();
	face_t *selFace = NULL;
	if (faceCount) {
		selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(0));
	} else {
		if (selected_brushes.next != &selected_brushes) {
			brush_t *b = selected_brushes.next; 
			if (!b->pPatch) {
				selFace = b->brush_faces;
			}
		}
	}

	if (selFace) {
		float rot;
		Face_GetScale_BrushPrimit(selFace, &m_horzScale, &m_vertScale, &rot);	
	} else {
		m_horzScale = 1.0f;
		m_vertScale = 1.0f;
	}

	UpdateData(FALSE);
}


bool g_bNewFace = false;
bool g_bNewApplyHandling = false;
bool g_bGatewayhack = false;


/*
=================
UpdateSpinners
=================
*/

void CSurfaceDlg::UpdateSpinners(bool up, int nID) {
	UpdateData(TRUE);
	float hdiv = 0.0f;
	float vdiv = 0.0f;
	switch (nID) {
		case IDC_SPIN_ROTATE : 
			Select_RotateTexture((up) ? m_rotate : -m_rotate);
			break;
		case IDC_SPIN_HSCALE : 
			m_horzScale += (up) ? 0.1f : -0.1f;
			hdiv = (m_horzScale == 0.0f) ? 1.0f : m_horzScale;
			Select_ScaleTexture( 1.0f / hdiv, 0.0f, true, ( m_absolute != FALSE ) );
			UpdateData(FALSE);
			break;
		case IDC_SPIN_VSCALE : 
			m_vertScale += (up) ? 0.1f : -0.1f;
			vdiv = (m_vertScale == 0.0f) ? 1.0f : m_vertScale;
			Select_ScaleTexture( 0.0f, 1.0f / vdiv, true, ( m_absolute != FALSE ) );
			UpdateData(FALSE);
			break;
		case IDC_SPIN_HSHIFT :
			Select_ShiftTexture((up) ? m_horzShift : -m_horzShift, 0);
			break;
		case IDC_SPIN_VSHIFT :
			Select_ShiftTexture(0, (up) ? m_vertShift : -m_vertShift);
			break;
	}
	g_changed_surface = true;
}

void CSurfaceDlg::UpdateSpinners(int nScrollCode, int nPos, CScrollBar* pBar) {

	return;
	UpdateData(TRUE);
	if ((nScrollCode != SB_LINEUP) && (nScrollCode != SB_LINEDOWN))  {
		return;
	}

	bool up = (nScrollCode == SB_LINEUP);

// FIXME: bad resource define
#define IDC_ROTATEA		0
#define IDC_HSCALEA		0
#define IDC_VSCALEA		0
#define IDC_HSHIFTA		0
#define IDC_VSHIFTA		0

	if (pBar->GetSafeHwnd() == ::GetDlgItem(GetSafeHwnd(), IDC_ROTATEA)) {
		Select_RotateTexture((up) ? m_rotate : -m_rotate);
	} else if (pBar->GetSafeHwnd() == ::GetDlgItem(GetSafeHwnd(), IDC_HSCALEA)) {
		Select_ScaleTexture((up) ? -m_horzScale : m_horzScale, 0, true, ( m_absolute != FALSE ) );
	} else if (pBar->GetSafeHwnd() == ::GetDlgItem(GetSafeHwnd(), IDC_VSCALEA)) {
		Select_ScaleTexture(0, (up) ? -m_vertScale : m_vertScale, true, ( m_absolute != FALSE ) );
	} else if (pBar->GetSafeHwnd() == ::GetDlgItem(GetSafeHwnd(), IDC_HSHIFTA)) {
		Select_ShiftTexture((up) ? -m_horzShift : m_horzShift, 0);
	} else if (pBar->GetSafeHwnd() == ::GetDlgItem(GetSafeHwnd(), IDC_VSHIFTA)) {
		Select_ShiftTexture((up) ? -m_vertShift : m_vertShift, 0);
	}

	g_changed_surface = true;
}

void UpdateSurfaceDialog() {
	if (g_surfwin)  {
		g_dlgSurface.SetTexMods();
	}
	g_pParentWnd->UpdateTextureBar();
}

bool ByeByeSurfaceDialog();

void DoSurface (void) {

	g_bNewFace = ( g_PrefsDlg.m_bFace != FALSE );
	g_bNewApplyHandling = ( g_PrefsDlg.m_bNewApplyHandling != FALSE );
	g_bGatewayhack = ( g_PrefsDlg.m_bGatewayHack != FALSE );
	// save current state for cancel
	g_old_texdef = g_qeglobals.d_texturewin.texdef;
	g_changed_surface = false;

	if (g_surfwin == NULL && g_dlgSurface.GetSafeHwnd() == NULL) {
		g_patch_texdef.scale[0] = 0.05f;
		g_patch_texdef.scale[1] = 0.05f;
		g_patch_texdef.shift[0] = 0.05f;
		g_patch_texdef.shift[1] = 0.05f;
		// use rotation increment from preferences
		g_patch_texdef.rotate = g_PrefsDlg.m_nRotation;

		g_dlgSurface.Create(IDD_SURFACE);
		CRect rct;
		LONG lSize = sizeof(rct);
		if (LoadRegistryInfo("radiant_SurfaceWindow", &rct, &lSize))  {
			g_dlgSurface.SetWindowPos( NULL, rct.left, rct.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
		}
		g_dlgSurface.ShowWindow(SW_SHOW);
		Sys_UpdateWindows(W_ALL);
	} else {
		g_surfwin = g_dlgSurface.GetSafeHwnd();
		g_dlgSurface.SetTexMods ();
		g_dlgSurface.ShowWindow(SW_SHOW);
	}
}		

bool ByeByeSurfaceDialog() {
	if (g_surfwin) {
		if (g_bGatewayhack)  {
			PostMessage(g_surfwin, WM_COMMAND, IDC_APPLY, 0);
		} else  {
			PostMessage(g_surfwin, WM_COMMAND, IDCANCEL, 0);
		}
		return true;
	} else  {
		return false;
	}
}

BOOL CSurfaceDlg::OnInitDialog() {
	CDialog::OnInitDialog();

	g_surfwin = GetSafeHwnd();
	SetTexMods ();

	//m_wndHScale.SetRange(0, 100);
	//m_wndVScale.SetRange(0, 100);
	m_wndHShift.SetRange(0, 100);
	m_wndVShift.SetRange(0, 100);
	m_wndRotate.SetRange(0, 100);
	m_wndWidth.SetRange(1, 32);
	m_wndHeight.SetRange(1, 32);

	m_wndVerticalSubdivisions.SetRange(1, 32);
	m_wndVerticalSubdivisions.SetBuddy(&m_wndVert, FALSE);
	m_wndHorzSubdivisions.SetRange(1, 32);
	m_wndHorzSubdivisions.SetBuddy(&m_wndHorz, FALSE);
	m_wndVerticalSubdivisions.SetPos(m_nVert);
	m_wndHorzSubdivisions.SetPos(m_nHorz);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSurfaceDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	UpdateData(TRUE);
	if (pScrollBar->IsKindOf(RUNTIME_CLASS(CSliderCtrl))) {
		CSliderCtrl *ctrl = reinterpret_cast<CSliderCtrl*>(pScrollBar);
		assert(ctrl);
		if (ctrl == &m_wndVerticalSubdivisions) {
			m_nVert = ctrl->GetPos();
		} else {
			m_nHorz = ctrl->GetPos();
		}
		UpdateData(FALSE);

		if (m_subdivide) {
			Patch_SubdivideSelected( ( m_subdivide != FALSE ), m_nHorz, m_nVert );
		}
	}
	Sys_UpdateWindows(W_CAMERA | W_XY);
}

void CSurfaceDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSurfaceDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	//UpdateSpinners(nSBCode, nPos, pScrollBar);
	//Sys_UpdateWindows(W_CAMERA);
}


void CSurfaceDlg::OnOK() {
	//GetTexMods();
	UpdateData(TRUE);
	if (m_strMaterial.Find(":") >= 0) {
		const idMaterial *mat = declManager->FindMaterial(m_strMaterial);
		Select_UpdateTextureName(m_strMaterial);
	}
	g_surfwin = NULL;
	CDialog::OnOK();
	Sys_UpdateWindows(W_ALL);
}

void CSurfaceDlg::OnClose() {
	g_surfwin = NULL;
	CDialog::OnClose();
}

void CSurfaceDlg::OnCancel() {
	if (g_bGatewayhack)  {
		OnOK();
	} else  {
		OnBtnCancel();
	}
}

void CSurfaceDlg::OnDestroy() {
	if (GetSafeHwnd()) {
		CRect rct;
		GetWindowRect(rct);
		SaveRegistryInfo("radiant_SurfaceWindow", &rct, sizeof(rct));
	}
	CDialog::OnDestroy();
	g_surfwin = NULL;
	Sys_UpdateWindows(W_ALL);
}

void CSurfaceDlg::OnBtnCancel() {
	g_qeglobals.d_texturewin.texdef = g_old_texdef;
	if (g_changed_surface) {
		//++timo if !g_qeglobals.m_bBrushPrimitMode send a NULL brushprimit_texdef
		if (!g_qeglobals.m_bBrushPrimitMode) {
			common->Printf("Warning : non brush primitive mode call to CSurfaceDlg::GetTexMods broken\n");
			common->Printf("          ( Select_SetTexture not called )\n");
		}
		//		Select_SetTexture(&g_qeglobals.d_texturewin.texdef);
	}
	g_surfwin = NULL;
	DestroyWindow();
}

void CSurfaceDlg::OnBtnColor() {
}

HBRUSH CSurfaceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

int CSurfaceDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CSurfaceDlg::PreCreateWindow(CREATESTRUCT& cs) {
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::PreCreateWindow(cs);
}


void CSurfaceDlg::OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult) {
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	UpdateSpinners((pNMUpDown->iDelta > 0), pNMUpDown->hdr.idFrom);
	*pResult = 0;
}

void CSurfaceDlg::OnBtnPatchdetails() {
	Patch_NaturalizeSelected(true);
	g_pParentWnd->GetCamera()->MarkWorldDirty ();
	Sys_UpdateWindows(W_ALL);
}

void CSurfaceDlg::OnBtnPatchnatural() {
	Select_SetTexture (&g_qeglobals.d_texturewin.texdef, &g_qeglobals.d_texturewin.brushprimit_texdef, false);
	Patch_NaturalizeSelected();
	g_pParentWnd->GetCamera()->MarkWorldDirty ();
	g_changed_surface = true;
	Sys_UpdateWindows(W_ALL);
}

void CSurfaceDlg::OnBtnPatchreset() {
	//CTextureLayout dlg;
	//if (dlg.DoModal() == IDOK) {
	//	Patch_ResetTexturing(dlg.m_fX, dlg.m_fY);
	//}
	//Sys_UpdateWindows(W_ALL);
}

void CSurfaceDlg::OnBtnAxial() {
}

void CSurfaceDlg::OnBtnBrushfit() {
	// TODO: Add your control notification handler code here

}

void CSurfaceDlg::OnBtnFacefit() {
	UpdateData(TRUE);
/*
	brush_t *b;
	for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next) {
		if (!b->patchBrush) {
			for (face_t* pFace = b->brush_faces; pFace; pFace = pFace->next) {
				g_ptrSelectedFaces.Add(pFace);
				g_ptrSelectedFaceBrushes.Add(b);
			}
		}
	}
*/
	Select_FitTexture(m_fHeight, m_fWidth);
	g_pParentWnd->GetCamera()->MarkWorldDirty ();
	//SetTexMods();
	g_changed_surface = true;
	Sys_UpdateWindows(W_ALL);
}


void CSurfaceDlg::OnCheckSubdivide() {
	UpdateData( TRUE );
	// turn any patches in explicit subdivides
	Patch_SubdivideSelected( ( m_subdivide != FALSE ), m_nHorz, m_nVert );
	g_pParentWnd->GetCamera()->MarkWorldDirty ();
	Sys_UpdateWindows( W_CAMERA | W_XY );
}

void CSurfaceDlg::OnChangeEditHorz() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	// turn any patches in explicit subdivides
	Patch_SubdivideSelected( ( m_subdivide != FALSE ), m_nHorz, m_nVert );
	Sys_UpdateWindows(W_CAMERA | W_XY);
		
}

void CSurfaceDlg::OnChangeEditVert() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	// turn any patches in explicit subdivides
	Patch_SubdivideSelected( ( m_subdivide != FALSE ), m_nHorz, m_nVert );
	Sys_UpdateWindows(W_CAMERA | W_XY);
	
}

BOOL CSurfaceDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN) {
			if (focusControl) {
				UpdateData(TRUE);
				if (focusControl == &m_wndHScale) {
					Select_ScaleTexture( m_horzScale, 1.0f, true, ( m_absolute != FALSE ) );
				} else if (focusControl == &m_wndVScale) {
					Select_ScaleTexture( 1.0f, m_vertScale, true, ( m_absolute != FALSE ) );
				} else if (focusControl == &m_wndRotateEdit) {
					Select_RotateTexture( m_rotate, true );
				} else if (focusControl == &m_wndHeight || focusControl == &m_wndWidth) {
					Select_FitTexture( m_fHeight, m_fWidth );
				}
			}
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CSurfaceDlg::OnSetfocusHscale() 
{
	focusControl = &m_wndHScale;	
}

void CSurfaceDlg::OnKillfocusHscale() 
{
	focusControl = NULL;
}

void CSurfaceDlg::OnKillfocusVscale() 
{
	focusControl = NULL;
}

void CSurfaceDlg::OnSetfocusVscale() 
{
	focusControl = &m_wndVScale;	
}

void CSurfaceDlg::OnKillfocusEditWidth() 
{
	focusControl = NULL;
}

void CSurfaceDlg::OnSetfocusEditWidth() 
{
	focusControl = &m_wndWidth;	
}

void CSurfaceDlg::OnKillfocusEditHeight() 
{
	focusControl = NULL;
}

void CSurfaceDlg::OnSetfocusEditHeight() 
{
	focusControl = &m_wndHeight;	
}

void CSurfaceDlg::OnBtnFlipx() 
{
	Select_FlipTexture(false);
}

void CSurfaceDlg::OnBtnFlipy() 
{
	Select_FlipTexture(true);
}

void CSurfaceDlg::OnKillfocusRotate() 
{
	focusControl = NULL;
}

void CSurfaceDlg::OnSetfocusRotate() 
{
	focusControl = &m_wndRotateEdit;
}

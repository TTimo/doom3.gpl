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
#include "GLWidget.h"
#include "PropertyList.h"
#include "entitydlg.h"
#include "PreviewDlg.h"
#include "CurveDlg.h"

#include "../../renderer/model_local.h"		// for idRenderModelPrt

void	Select_Ungroup();

// CEntityDlg dialog

IMPLEMENT_DYNAMIC(CEntityDlg, CDialog)
CEntityDlg::CEntityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEntityDlg::IDD, pParent)
{
	editEntity = NULL;
	multipleEntities = false;
	currentAnimation = NULL;
}

CEntityDlg::~CEntityDlg()
{
}

void CEntityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_KEYVAL, listKeyVal);
	DDX_Control(pDX, IDC_COMBO_CLASS, comboClass);
	DDX_Control(pDX, IDC_EDIT_KEY, editKey);
	DDX_Control(pDX, IDC_EDIT_VAL, editVal);
	DDX_Control(pDX, IDC_STATIC_TITLE, staticTitle);
	DDX_Control(pDX, IDC_STATIC_KEY, staticKey);
	DDX_Control(pDX, IDC_STATIC_VAL, staticVal);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, btnBrowse);
	DDX_Control(pDX, IDC_E_135, btn135);
	DDX_Control(pDX, IDC_E_90, btn90);
	DDX_Control(pDX, IDC_E_45, btn45);
	DDX_Control(pDX, IDC_E_180, btn180);
	DDX_Control(pDX, IDC_E_0, btn360);
	DDX_Control(pDX, IDC_E_225, btn225);
	DDX_Control(pDX, IDC_E_270, btn270);
	DDX_Control(pDX, IDC_E_315, btn315);
	DDX_Control(pDX, IDC_E_UP, btnUp);
	DDX_Control(pDX, IDC_E_DOWN, btnDown);
	DDX_Control(pDX, IDC_BUTTON_MODEL, btnModel);
	DDX_Control(pDX, IDC_BUTTON_SOUND, btnSound);
	DDX_Control(pDX, IDC_BUTTON_GUI, btnGui);
	DDX_Control(pDX, IDC_BUTTON_PARTICLE, btnParticle);
	DDX_Control(pDX, IDC_BUTTON_SKIN, btnSkin);
	DDX_Control(pDX, IDC_BUTTON_CURVE, btnCurve);
	DDX_Control(pDX, IDC_BUTTON_CREATE, btnCreate);
	DDX_Control(pDX, IDC_LIST_VARS, listVars);
	DDX_Control(pDX, IDC_ENTITY_ANIMATIONS , cbAnimations);
	DDX_Control(pDX, IDC_ANIMATION_SLIDER , slFrameSlider);
	DDX_Control(pDX, IDC_ENTITY_CURRENT_ANIM , staticFrame);
	DDX_Control(pDX, IDC_ENTITY_PLAY_ANIM , btnPlayAnim);
	DDX_Control(pDX, IDC_ENTITY_STOP_ANIM , btnStopAnim);
}



BOOL CEntityDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	listKeyVal.SetUpdateInspectors(true);
	listKeyVal.SetDivider(100);
	listVars.SetDivider(100);
	staticFrame.SetWindowText ( "0" );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CEntityDlg::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::OnToolHitTest(point, pTI);
}


void CEntityDlg::AddClassNames() {
	comboClass.ResetContent();
	for (eclass_t *pec = eclass; pec; pec = pec->next) {
		comboClass.AddString(pec->name);
	}

}

BEGIN_MESSAGE_MAP(CEntityDlg, CDialog)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO_CLASS, OnCbnSelchangeComboClass)
	ON_LBN_SELCHANGE(IDC_LIST_KEYVAL, OnLbnSelchangeListkeyval)
	ON_BN_CLICKED(IDC_E_135, OnBnClickedE135)
	ON_BN_CLICKED(IDC_E_90, OnBnClickedE90)
	ON_BN_CLICKED(IDC_E_45, OnBnClickedE45)
	ON_BN_CLICKED(IDC_E_180, OnBnClickedE180)
	ON_BN_CLICKED(IDC_E_0, OnBnClickedE0)
	ON_BN_CLICKED(IDC_E_225, OnBnClickedE225)
	ON_BN_CLICKED(IDC_E_270, OnBnClickedE270)
	ON_BN_CLICKED(IDC_E_315, OnBnClickedE315)
	ON_BN_CLICKED(IDC_E_UP, OnBnClickedEUp)
	ON_BN_CLICKED(IDC_E_DOWN, OnBnClickedEDown)
	ON_BN_CLICKED(IDC_BUTTON_MODEL, OnBnClickedButtonModel)
	ON_BN_CLICKED(IDC_BUTTON_SOUND, OnBnClickedButtonSound)
	ON_BN_CLICKED(IDC_BUTTON_GUI, OnBnClickedButtonGui)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBnClickedButtonBrowse)
	ON_CBN_DBLCLK(IDC_COMBO_CLASS, OnCbnDblclkComboClass)
	ON_BN_CLICKED(IDC_BUTTON_CREATE, OnBnClickedButtonCreate)
	ON_LBN_DBLCLK(IDC_LIST_KEYVAL, OnLbnDblclkListkeyval)
	ON_LBN_SELCHANGE(IDC_LIST_VARS, OnLbnSelchangeListVars)
	ON_LBN_DBLCLK(IDC_LIST_VARS, OnLbnDblclkListVars)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_ANIMATION_SLIDER, OnNMReleasedcaptureSlider1)
	ON_BN_CLICKED(IDC_BUTTON_PARTICLE, OnBnClickedButtonParticle)
	ON_BN_CLICKED(IDC_BUTTON_SKIN, OnBnClickedButtonSkin)
	ON_BN_CLICKED(IDC_BUTTON_CURVE, OnBnClickedButtonCurve)
	ON_CBN_SELCHANGE(IDC_ENTITY_ANIMATIONS, OnCbnAnimationChange)
	ON_BN_CLICKED(IDC_ENTITY_PLAY_ANIM , OnBnClickedStartAnimation)
	ON_BN_CLICKED(IDC_ENTITY_STOP_ANIM , OnBnClickedStopAnimation)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()

void CEntityDlg::OnSize(UINT nType, int cx, int cy)
{
	if (staticTitle.GetSafeHwnd() == NULL) {
		return;
	}
	CDialog::OnSize(nType, cx, cy);
	CRect rect, crect, crect2;
	GetClientRect(rect);
	int bh = (float)rect.Height() * (rect.Height() - 210) / rect.Height() / 2;
	staticTitle.GetWindowRect(crect);
	staticTitle.SetWindowPos(NULL, 4, 4, rect.Width() -8, crect.Height(), SWP_SHOWWINDOW);
	int top = 4 + crect.Height() + 4;
	comboClass.GetWindowRect(crect);
	btnCreate.GetWindowRect(crect2);
	comboClass.SetWindowPos(NULL, 4, top, rect.Width() - 12 - crect2.Width(), crect.Height(), SWP_SHOWWINDOW);
	btnCreate.SetWindowPos(NULL, rect.Width() - crect2.Width() - 4, top, crect2.Width(), crect.Height(), SWP_SHOWWINDOW);
	top += crect.Height() + 4;
	listVars.SetWindowPos(NULL, 4, top, rect.Width() - 8, bh, SWP_SHOWWINDOW);
	top += bh + 4;
	listKeyVal.SetWindowPos(NULL, 4, top, rect.Width() - 8, bh, SWP_SHOWWINDOW);
	top += bh + 4;
	staticKey.GetWindowRect(crect);
	staticKey.SetWindowPos(NULL, 4, top + 2, crect.Width(), crect.Height(), SWP_SHOWWINDOW);
	int left = 4 + crect.Width() + 4;
	int pad = crect.Width();
	editKey.GetWindowRect(crect);
	editKey.SetWindowPos(NULL, left, top, rect.Width() - 12 - pad, crect.Height(), SWP_SHOWWINDOW);
	top += crect.Height() + 4;
	staticVal.GetWindowRect(crect);
	staticVal.SetWindowPos(NULL, 4, top + 2, crect.Width(), crect.Height(), SWP_SHOWWINDOW);
	editVal.GetWindowRect(crect);
	bh = crect.Height();
	editVal.SetWindowPos(NULL, left, top, rect.Width() - 16 - bh - pad, crect.Height(), SWP_SHOWWINDOW);
	btnBrowse.SetWindowPos(NULL, rect.right - 4 - bh, top, bh, bh, SWP_SHOWWINDOW);
	top += crect.Height() + 8;
	btnModel.GetWindowRect(crect);
	btnModel.SetWindowPos(NULL, rect.right - 4 - crect.Width(), top + 8, crect.Width(), crect.Height(), SWP_SHOWWINDOW);
	btnSound.SetWindowPos(NULL, rect.right - 4 - crect.Width(), top + 12 + crect.Height(), crect.Width(), crect.Height(), SWP_SHOWWINDOW);
	btnGui.SetWindowPos(NULL, rect.right - 4 - crect.Width(), top + 16 + crect.Height() * 2, crect.Width(), crect.Height(), SWP_SHOWWINDOW);
	btnParticle.SetWindowPos(NULL, rect.right - 8 - (crect.Width() * 2), top + 16 + crect.Height() * 2, crect.Width(), crect.Height(), SWP_SHOWWINDOW);
	btnSkin.SetWindowPos( NULL, rect.right - 8 - ( crect.Width() * 2 ), top + 12 + crect.Height(), crect.Width(), crect.Height(), SWP_SHOWWINDOW ); 
	btnCurve.SetWindowPos( NULL, rect.right - 8 - ( crect.Width() * 2 ), top + 8, crect.Width(), crect.Height(), SWP_SHOWWINDOW ); 

	//*************************************
	//animation controls
	//*************************************
	int rightAnimAreaBorder = rect.right - 75 - crect.Width (); /*models, etc button width*/

	btnStopAnim.GetWindowRect(crect);
	btnStopAnim.SetWindowPos(NULL,rightAnimAreaBorder - crect.Width (),
		top + 8  ,crect.Width(),crect.Height(),SWP_SHOWWINDOW);

	left = rightAnimAreaBorder - crect.Width() - 4;
	btnPlayAnim.GetWindowRect(crect);
	btnPlayAnim.SetWindowPos(NULL,left-crect.Width () ,top + 8 , crect.Width(),crect.Height(),SWP_SHOWWINDOW);

	left -= crect.Width() + 4;
	cbAnimations.GetWindowRect(crect);
	cbAnimations.SetWindowPos(NULL,left-crect.Width (),top + 8  ,crect.Width(),crect.Height(),SWP_SHOWWINDOW);

	staticFrame.GetWindowRect(crect);
	staticFrame.SetWindowPos(NULL,rightAnimAreaBorder - crect.Width (),
		top + 34  ,crect.Width(),crect.Height(),SWP_SHOWWINDOW);

	left = rightAnimAreaBorder - crect.Width () - 4;

	slFrameSlider.GetWindowRect(crect);
	slFrameSlider.SetWindowPos(NULL,left - crect.Width (),
	top + 32  ,crect.Width(),crect.Height(),SWP_SHOWWINDOW);

	//*************************************
	//*************************************

	btn135.GetWindowRect(crect);
	bh = crect.Width();
	btn135.SetWindowPos(NULL, 4, top, bh, bh, SWP_SHOWWINDOW);
	btn90.SetWindowPos(NULL, 4 + 2 + bh, top, bh, bh, SWP_SHOWWINDOW);
	btn45.SetWindowPos(NULL, 4 + 2 + 2 + bh * 2, top, bh, bh, SWP_SHOWWINDOW);
	btnUp.SetWindowPos(NULL, 4 + 2 + 2 + 6 + bh * 3, top + bh / 2,bh,bh, SWP_SHOWWINDOW);
	btnDown.SetWindowPos(NULL, 4 + 2 + 2 + 6 + bh *3, top + bh / 2 + bh + 2,bh,bh, SWP_SHOWWINDOW);
	top += bh + 2;
	btn180.SetWindowPos(NULL, 4, top, bh, bh, SWP_SHOWWINDOW);
	btn360.SetWindowPos(NULL, 4 + 2 + 2 + bh * 2, top, bh, bh, SWP_SHOWWINDOW);
	top += bh + 2;
	btn225.SetWindowPos(NULL, 4, top, bh, bh, SWP_SHOWWINDOW);
	btn270.SetWindowPos(NULL, 4 + 2 + bh, top, bh, bh, SWP_SHOWWINDOW);
	btn315.SetWindowPos(NULL, 4 + 2 + 2 + bh * 2, top, bh, bh, SWP_SHOWWINDOW);
	Invalidate();
}

void CEntityDlg::OnCbnSelchangeComboClass()
{
	int index = comboClass.GetCurSel();
	if (index != LB_ERR) {
		CString str;
		comboClass.GetLBText(index, str);
		eclass_t *ent = Eclass_ForName (str, false);
		if (ent) {
			if (selected_brushes.next == &selected_brushes) {
				editEntity = world_entity;
				multipleEntities = false;
			} else {
				editEntity = selected_brushes.next->owner;
				for (brush_t *b = selected_brushes.next->next; b != &selected_brushes; b = b->next) {
					if (b->owner != editEntity) {
						multipleEntities = true;
						break;
					}
				}
			}
			listVars.ResetContent();
			CPropertyItem *pi = new CPropertyItem("Usage:", ent->desc.c_str(), PIT_VAR, "");
			listVars.AddPropItem(pi);

			int c = ent->vars.Num();
			for (int i = 0; i < c; i++) {
				pi = new CPropertyItem(ent->vars[i].name.c_str(), ent->vars[i].desc.c_str(), PIT_VAR, "");
				pi->SetData(ent->vars[i].type);
				listVars.AddPropItem(pi);
			}
			listVars.Invalidate();
			SetKeyValPairs();
		}
	}
}

const char *CEntityDlg::TranslateString(const char *buf) {
	static char buf2[32768];
	int			i, l;
	char		*out;

	l = strlen(buf);
	out = buf2;
	for (i = 0; i < l; i++) {
		if (buf[i] == '\n') {
			*out++ = '\r';
			*out++ = '\n';
		}
		else {
			*out++ = buf[i];
		}
	}

	*out++ = 0;

	return buf2;

}

void CEntityDlg::UpdateFromListBox() {
	if (editEntity == NULL) {
		return;
	}
	int c = listKeyVal.GetCount();
	for (int i = 0 ; i < c; i++) {
		CPropertyItem* pItem = (CPropertyItem*)listKeyVal.GetItemDataPtr(i);
		if (pItem) {
			editEntity->epairs.Set(pItem->m_propName, pItem->m_curValue);
		}
	}
	SetKeyValPairs();
}

void CEntityDlg::SetKeyValPairs( bool updateAnims ) {
	if (editEntity) {
		listKeyVal.ResetContent();
		int c = editEntity->epairs.GetNumKeyVals();
		for (int i = 0; i < c; i++) {
			const idKeyValue *kv = editEntity->epairs.GetKeyVal(i);
			CPropertyItem *pi = new CPropertyItem(kv->GetKey().c_str(), kv->GetValue().c_str(), PIT_EDIT, "");
			bool found = false;
			int vc = editEntity->eclass->vars.Num();
			for (int j = 0; j < vc; j++) {
				if (editEntity->eclass->vars[j].name.Icmp(kv->GetKey()) == 0) {
					switch (editEntity->eclass->vars[j].type) {
						case EVAR_STRING :
						case EVAR_INT :
						case EVAR_FLOAT :
							pi->m_nItemType = PIT_EDIT;
							break;
						case EVAR_BOOL :
							pi->m_nItemType = PIT_EDIT;
							//pi->m_cmbItems = "0|1";
							break;
						case EVAR_COLOR :
							pi->m_nItemType = PIT_COLOR;
							break;
						case EVAR_MATERIAL :
							pi->m_nItemType = PIT_MATERIAL;
							break;
						case EVAR_MODEL :
							pi->m_nItemType = PIT_MODEL;
							break;
						case EVAR_GUI :
							pi->m_nItemType = PIT_GUI;
							break;
						case EVAR_SOUND :
							pi->m_nItemType = PIT_SOUND;
							break;
					}
					found = true;
					break;
				}
			}
			if (!found) {
				if (kv->GetKey().Icmp("model") == 0) {
					pi->m_nItemType = PIT_MODEL;
				}
				if (kv->GetKey().Icmp("_color") == 0) {
					pi->m_nItemType = PIT_COLOR;
				}
				if (kv->GetKey().Icmp("gui") == 0) {
					pi->m_nItemType = PIT_GUI;
				}
				if (kv->GetKey().Icmp("gui2") == 0) {
					pi->m_nItemType = PIT_GUI;
				}
				if (kv->GetKey().Icmp("gui3") == 0) {
					pi->m_nItemType = PIT_GUI;
				}
				if (kv->GetKey().Icmp("s_shader") == 0) {
					pi->m_nItemType = PIT_SOUND;
				}
			}
			listKeyVal.AddPropItem(pi);
		}

		if ( updateAnims ) {
			int i, num;

			cbAnimations.ResetContent();
			num = gameEdit->ANIM_GetNumAnimsFromEntityDef( &editEntity->eclass->defArgs );
			for( i = 0; i < num; i++ ) {
				cbAnimations.AddString( gameEdit->ANIM_GetAnimNameFromEntityDef( &editEntity->eclass->defArgs, i ) );
			}

			const idKeyValue* kv = editEntity->epairs.FindKey ( "anim" );
			if ( kv ) {
				int selIndex = cbAnimations.FindStringExact( 0 , kv->GetValue().c_str() );
				if ( selIndex != -1 ) {
					cbAnimations.SetCurSel( selIndex );
					OnCbnAnimationChange ();
				}
			}
		}
	}
}

void CEntityDlg::UpdateEntitySel(eclass_t *ent) {
	assert ( ent );
	assert ( ent->name );
	int index = comboClass.FindString(-1, ent->name);
	if (index != LB_ERR) {
		comboClass.SetCurSel(index);
		OnCbnSelchangeComboClass();
	}
}

void CEntityDlg::OnLbnSelchangeListkeyval()
{
	int index = listKeyVal.GetCurSel();
	if (index != LB_ERR) {
		CString str;
		listKeyVal.GetText(index, str);
		int i;
		for (i = 0; str[i] != '\t' && str[i] != '\0'; i++) {
		}

		idStr key = str.Left(i);
		while (str[i] == '\t' && str[i] != '\0') {
			i++;
		}

		idStr val = str.Right(str.GetLength() - i);

		editKey.SetWindowText(key);
		editVal.SetWindowText(val);
	}
}

static int TabOrder[] = {
	IDC_COMBO_CLASS,
	IDC_BUTTON_CREATE,
	//IDC_EDIT_INFO,
	IDC_LIST_KEYVAL,
	IDC_EDIT_KEY,
	IDC_EDIT_VAL,
	IDC_BUTTON_BROWSE,
	IDC_E_135,
	IDC_E_90,
	IDC_E_45,
	IDC_E_180,
	IDC_E_0,
	IDC_E_225,
	IDC_E_270,
	IDC_E_315,
	IDC_E_UP,
	IDC_E_DOWN,
	IDC_BUTTON_MODEL,
	IDC_BUTTON_SOUND,
	IDC_BUTTON_GUI,
	IDC_ENTITY_ANIMATIONS
};

int TabCount = sizeof(TabOrder) / sizeof(int);

void CEntityDlg::DelProp() {
	CString key;

	if (editEntity == NULL) {
		return;
	}

	editKey.GetWindowText(key);
	if (multipleEntities) {
		for (brush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next) {
			DeleteKey(b->owner, key);
			Entity_UpdateCurveData( b->owner );
		}
	} else {
		DeleteKey(editEntity, key);
		Entity_UpdateCurveData( editEntity );
	}

	// refresh the prop listbox
	SetKeyValPairs();
	Sys_UpdateWindows( W_ENTITY | W_XY | W_CAMERA );
}


BOOL CEntityDlg::PreTranslateMessage(MSG* pMsg)
{

	if (pMsg->hwnd == editVal.GetSafeHwnd()) {
		if (pMsg->message == WM_LBUTTONDOWN) {
			editVal.SetFocus();
			return TRUE;
		}
	}

	if (pMsg->hwnd == editKey.GetSafeHwnd()) {
		if (pMsg->message == WM_LBUTTONDOWN) {
			editKey.SetFocus();
			return TRUE;
		}
	}

	if (GetFocus() == &editVal || GetFocus() == &editKey) {
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN ) {
				AddProp();
				return TRUE;
		}

	}

	if (GetFocus() == listKeyVal.GetEditBox()) {
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN ) {
			listKeyVal.OnChangeEditBox();
			listKeyVal.OnSelchange();
			listKeyVal.OnKillfocusEditBox();
			AddProp();
			SetKeyValPairs();
			return TRUE;
		}
	}

	if (GetFocus() == &listKeyVal) {
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DELETE && editEntity) {
			DelProp();
			return TRUE;
		} 
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
		if (pMsg->wParam == VK_ESCAPE) {
			g_pParentWnd->GetCamera()->SetFocus();
			Select_Deselect();
		}
		return TRUE;
	}

	if ( pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN ) {
		// keeps ENTER from closing the dialog
		return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
		if (GetFocus()) {
			int id = GetFocus()->GetDlgCtrlID();
			for (int i = 0; i < TabCount; i++) {
				if (TabOrder[i] == id) {
					i++;
					if (i >= TabCount) {
						i = 0;
					}
					CWnd *next = GetDlgItem(TabOrder[i]);
					if (next) {
						next->SetFocus();
						if (TabOrder[i] == IDC_EDIT_VAL) {
							editVal.SetSel(0, -1);
						}
						return TRUE;
					}
				}
			}
		}
	}
	
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RIGHT && pMsg->hwnd == slFrameSlider.GetSafeHwnd()) {
		int pos = slFrameSlider.GetPos() + 1;
		pos = (pos % slFrameSlider.GetRangeMax());
		slFrameSlider.SetPos ( pos );
		UpdateFromAnimationFrame ();
		return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_LEFT && pMsg->hwnd == slFrameSlider.GetSafeHwnd()) {
		int pos = slFrameSlider.GetPos() - 1;

		if ( pos < 1 ) {
			pos = slFrameSlider.GetRangeMax();
		}

		slFrameSlider.SetPos ( pos );
		UpdateFromAnimationFrame ();
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


/*
 =======================================================================================================================
    AddProp
 =======================================================================================================================
 */
void CEntityDlg::AddProp() {

	if (editEntity == NULL) {
		return;
	}

	CString Key, Value;
	editKey.GetWindowText(Key);
	editVal.GetWindowText(Value);

	bool isName = (stricmp(Key, "name") == 0);
	bool isModel = static_cast<bool>((stricmp(Key, "model") == 0 && Value.GetLength() > 0));
	bool isOrigin = ( idStr::Icmp( Key, "origin" ) == 0 );

	if (multipleEntities) {
		brush_t *b;
		for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
			if (isName) {
				Entity_SetName(b->owner, Value);
			} else {
				if ( ! ( ( isModel || isOrigin ) && ( b->owner->eclass->nShowFlags & ECLASS_WORLDSPAWN ) ) ) { 
					SetKeyValue(b->owner, Key, Value);
				}
			}
		}
	}
	else {
		if (isName) {
			Entity_SetName(editEntity, Value);
		} else {
			if ( ! ( ( isModel || isOrigin ) && ( editEntity->eclass->nShowFlags & ECLASS_WORLDSPAWN ) ) ) { 
				SetKeyValue(editEntity, Key, Value);
			}
		}

		if ( isModel && !( editEntity->eclass->nShowFlags & ECLASS_WORLDSPAWN ) ) {
			idBounds	bo;
			idVec3	mins, maxs;

			selected_brushes.next->modelHandle = renderModelManager->FindModel( Value );
			if ( dynamic_cast<idRenderModelPrt*>( selected_brushes.next->modelHandle ) || dynamic_cast<idRenderModelLiquid*>( selected_brushes.next->modelHandle ) ) {
				bo.Zero();
				bo.ExpandSelf( 12.0f );
			} else {
				bo = selected_brushes.next->modelHandle->Bounds( NULL );
			}

			VectorCopy(bo[0], mins);
			VectorCopy(bo[1], maxs);
			VectorAdd(mins, editEntity->origin, mins);
			VectorAdd(maxs, editEntity->origin, maxs);
			Brush_RebuildBrush(selected_brushes.next, mins, maxs, false);
			Brush_Build ( selected_brushes.next , false, false , false, true );
		}
	}

	// refresh the prop listbox
	SetKeyValPairs();
	Sys_UpdateWindows(W_ALL);

}

const char *CEntityDlg::AngleKey() {
	if (editEntity == NULL) {
		return "";
	}
	
	if (editEntity->eclass->nShowFlags & ECLASS_MOVER) {
		return "movedir";
	}

	return "angle";
}


void CEntityDlg::OnBnClickedE135()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("135");
	AddProp();
}

void CEntityDlg::OnBnClickedE90()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("90");
	AddProp();
}

void CEntityDlg::OnBnClickedE45()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("45");
	AddProp();
}

void CEntityDlg::OnBnClickedE180()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("180");
	AddProp();
}

void CEntityDlg::OnBnClickedE0()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("0");
	AddProp();
}

void CEntityDlg::OnBnClickedE225()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("225");
	AddProp();
}

void CEntityDlg::OnBnClickedE270()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("270");
	AddProp();
}

void CEntityDlg::OnBnClickedE315()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("315");
	AddProp();
}

void CEntityDlg::OnBnClickedEUp()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("-1");
	AddProp();
}

void CEntityDlg::OnBnClickedEDown()
{
	if (editEntity == NULL) {
		return;
	}
	editKey.SetWindowText(AngleKey());
	editVal.SetWindowText("-2");
	AddProp();
}

CPreviewDlg *CEntityDlg::ShowModelChooser() {
	static CPreviewDlg modelDlg;
	modelDlg.SetMode(CPreviewDlg::MODELS);
	modelDlg.SetModal();
	if (modelDlg.GetSafeHwnd() == NULL) {
		modelDlg.Create(MAKEINTRESOURCE(IDD_DIALOG_PREVIEW));
	}
	modelDlg.ShowWindow( SW_SHOW );
	modelDlg.BringWindowToTop();
	while (modelDlg.Waiting()) {
	}
	return &modelDlg;
}

CPreviewDlg *CEntityDlg::ShowParticleChooser() {
	static CPreviewDlg modelDlg;
	modelDlg.SetMode(CPreviewDlg::PARTICLES);
	modelDlg.SetModal();
	if (modelDlg.GetSafeHwnd() == NULL) {
		modelDlg.Create(MAKEINTRESOURCE(IDD_DIALOG_PREVIEW));
	}
	modelDlg.ShowWindow(SW_SHOW);
	modelDlg.BringWindowToTop();
	while (modelDlg.Waiting()) {
	}
	return &modelDlg;
}

CPreviewDlg *CEntityDlg::ShowSkinChooser(entity_t *ent) {
	static CPreviewDlg modelDlg;
	modelDlg.SetMode(CPreviewDlg::SKINS);
	modelDlg.SetModal();
	if (modelDlg.GetSafeHwnd() == NULL) {
		modelDlg.Create(MAKEINTRESOURCE(IDD_DIALOG_PREVIEW));
	}
	modelDlg.RebuildTree( ( ent ) ? ent->epairs.GetString( "model" ) : "" );
	modelDlg.ShowWindow(SW_SHOW);
	modelDlg.BringWindowToTop();
	while (modelDlg.Waiting()) {
	}
	return &modelDlg;
}

CPreviewDlg *CEntityDlg::ShowGuiChooser() {
	static CPreviewDlg guiDlg;
	guiDlg.SetMode(CPreviewDlg::GUIS);
	guiDlg.SetModal();
	if (guiDlg.GetSafeHwnd() == NULL) {
		guiDlg.Create(MAKEINTRESOURCE(IDD_DIALOG_PREVIEW));
	}
	guiDlg.ShowWindow(SW_SHOW);
	guiDlg.BringWindowToTop();
	while (guiDlg.Waiting()) {
	}
	return &guiDlg;
}

CPreviewDlg *CEntityDlg::ShowSoundChooser() {
	static CPreviewDlg soundDlg;
	soundDlg.SetMode(CPreviewDlg::SOUNDS);
	soundDlg.SetModal();
	if (soundDlg.GetSafeHwnd() == NULL) {
		soundDlg.Create(MAKEINTRESOURCE(IDD_DIALOG_PREVIEW));
	}
	soundDlg.ShowWindow(SW_SHOW);
	while (soundDlg.Waiting()) {
	}
	return &soundDlg;
}

CPreviewDlg *CEntityDlg::ShowMaterialChooser() {
	static CPreviewDlg matDlg;
	matDlg.SetMode(CPreviewDlg::MATERIALS);
	matDlg.SetModal();
	if (matDlg.GetSafeHwnd() == NULL) {
		matDlg.Create(MAKEINTRESOURCE(IDD_DIALOG_PREVIEW));
	}
	matDlg.ShowWindow(SW_SHOW);
	matDlg.BringWindowToTop();
	while (matDlg.Waiting()) {
	}
	return &matDlg;
}

void CEntityDlg::AssignModel ()
{
	OnBnClickedButtonModel();
}
void CEntityDlg::OnBnClickedButtonModel() {
	CPreviewDlg *dlg = ShowModelChooser();
	if (dlg->returnCode == IDOK) {
		editKey.SetWindowText("model");
		editVal.SetWindowText(dlg->mediaName);
		AddProp();
	}
}

void CEntityDlg::OnBnClickedButtonSound() {
	CPreviewDlg *dlg = ShowSoundChooser();
	if (dlg->returnCode == IDOK) {
		editKey.SetWindowText("s_shader");
		editVal.SetWindowText(dlg->mediaName);
		AddProp();
	}
}

void CEntityDlg::OnBnClickedButtonGui() {
	CPreviewDlg *dlg = ShowGuiChooser();
	if (dlg->returnCode == IDOK) {
		editKey.SetWindowText("gui");
		editVal.SetWindowText(dlg->mediaName);
		AddProp();
	}
}

void CEntityDlg::OnBnClickedButtonParticle() {
	CPreviewDlg *dlg = ShowParticleChooser();
	if (dlg->returnCode == IDOK) {
		editKey.SetWindowText("model");
		editVal.SetWindowText(dlg->mediaName);
		AddProp();
	}
}

void CEntityDlg::OnBnClickedButtonSkin() {
	CPreviewDlg *dlg = ShowSkinChooser( editEntity );
	if (dlg->returnCode == IDOK) {
		editKey.SetWindowText("skin");
		editVal.SetWindowText(dlg->mediaName);
		AddProp();
	}

}

void CEntityDlg::OnBnClickedButtonCurve() {
	CCurveDlg dlg;
	if ( dlg.DoModal() == IDOK ) {
		if ( editEntity ) {
			idStr str = "curve_" + dlg.strCurveType;
			editKey.SetWindowText( str );
			idVec3 org = editEntity->origin;
			str = "3 ( ";
			str += org.ToString();
			org.x += 64;
			str += " ";
			str += org.ToString();
			org.y += 64;
			str += " ";
			str += org.ToString();
			str += " )";
			editVal.SetWindowText( str );
			AddProp();
			Entity_SetCurveData( editEntity );
		}
	}
}

void CEntityDlg::OnBnClickedButtonBrowse() {
	DelProp();
}

void CEntityDlg::OnCbnDblclkComboClass()
{
	// TODO: Add your control notification handler code here
}

//
// =======================================================================================================================
//    CreateEntity Creates a new entity based on the currently selected brush and entity type.
// =======================================================================================================================
//
void CEntityDlg::CreateEntity() {
	entity_t	*petNew;
	bool		forceFixed = false;

	// check to make sure we have a brush
	CXYWnd	*pWnd = g_pParentWnd->ActiveXY();
	if (pWnd) {
		CRect	rctZ;
		pWnd->GetClientRect(rctZ);

		brush_t *pBrush;
		if (selected_brushes.next == &selected_brushes) {
			pBrush = CreateEntityBrush(g_nSmartX, rctZ.Height() - 1 - g_nSmartY, pWnd);
			forceFixed = true;
		}
	}
	else {
		if (selected_brushes.next == &selected_brushes) {
			MessageBox("You must have a selected brush to create an entity", "info", 0);
			return;
		}
	}

	int index = comboClass.GetCurSel();
	if (index == LB_ERR) {
		MessageBox("You must have a selected class to create an entity", "info", 0);
		return;
	}
	
	CString str;
	comboClass.GetLBText(index, str);

	if (!stricmp(str, "worldspawn")) {
		MessageBox("Can't create an entity with worldspawn.", "info", 0);
		return;
	}

	eclass_t *pecNew = Eclass_ForName (str, false);

	// create it
	if ((GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
		// MAJOR hack for xian
extern void Brush_CopyList(brush_t *pFrom, brush_t *pTo);
		brush_t temp_brushes;
		temp_brushes.next = &temp_brushes;
		Brush_CopyList(&selected_brushes, &temp_brushes);
		Select_Deselect();
		brush_t *pBrush = temp_brushes.next;
		while (pBrush != NULL && pBrush != &temp_brushes) {
			brush_t *pNext = pBrush->next;
			Brush_RemoveFromList(pBrush);
			Brush_AddToList(pBrush, &selected_brushes);
			pBrush = pNext;
			petNew = Entity_Create(pecNew, forceFixed);
			Select_Deselect();
		}
	} else if ((GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
		Select_Ungroup();
		petNew = Entity_Create(pecNew, forceFixed);
	} else {
		petNew = Entity_Create(pecNew, forceFixed);
	}

	if (petNew == NULL) {
		MessageBox("Failed to create entity.", "info", 0);
		return;
	}

	if (selected_brushes.next == &selected_brushes) {
		editEntity = world_entity;
	}
	else {
		editEntity = selected_brushes.next->owner;
	}

	SetKeyValPairs();
	Select_Deselect();
	Select_Brush(editEntity->brushes.onext);
	Sys_UpdateWindows(W_ALL);
}

void CEntityDlg::OnBnClickedButtonCreate()
{
	CreateEntity();
}

void CEntityDlg::OnLbnDblclkListkeyval()
{
	CString Key, Value;
	idStr work;
	editKey.GetWindowText( Key );
	editVal.GetWindowText( Value );
	if ( stricmp( Key, "script" ) == 0 ) {
		Key = Value;
		Value = "script/" + Key;
		if ( fileSystem->ReadFile( Value, NULL, NULL ) == -1) {
			sprintf( work, "// Script for %s\n// \n\nvoid main() {\n\n}\n\n", currentmap );
			fileSystem->WriteFile( Value, work.c_str(), work.Length(), "fs_devpath" );
		}
		work = fileSystem->RelativePathToOSPath( Value );
		WinExec( va( "notepad.exe %s", work.c_str() ), SW_SHOW );
	}
}

void CEntityDlg::OnLbnSelchangeListVars() {

}

void CEntityDlg::OnLbnDblclkListVars() {
	if (editEntity == NULL) {
		return;
	}
	int sel = listVars.GetCurSel();
	CPropertyItem *pi = (CPropertyItem*)listVars.GetItemDataPtr(sel);
	if (pi) {
		if (editEntity->epairs.FindKey(pi->m_propName) == NULL) {
			editKey.SetWindowText(pi->m_propName);
			editVal.SetWindowText("");
			editVal.SetFocus();
		}
	}
}


void CEntityDlg::UpdateKeyVal(const char *key, const char *val) {
	if (editEntity) {
		editEntity->epairs.Set(key, val);
		SetKeyValPairs();
		g_pParentWnd->GetCamera()->BuildEntityRenderState(editEntity, true);
		Entity_UpdateSoundEmitter(editEntity);
	}
}


void CEntityDlg::OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	if ( !editEntity )
	{
		return;
	}
	
	UpdateFromAnimationFrame ();

	*pResult = 0;
}

void CEntityDlg::UpdateFromAnimationFrame ( bool updateKeyValueDisplay )
{
	int frame = slFrameSlider.GetPos ();
	editEntity->epairs.SetInt( "frame" , frame );
	SetDlgItemText ( IDC_ENTITY_CURRENT_ANIM , va ( "%i" , frame));
	if ( updateKeyValueDisplay ) {
		SetKeyValPairs();
	}

	g_pParentWnd->GetCamera ()->BuildEntityRenderState (editEntity , true );
	Sys_UpdateWindows ( W_ALL );

}

void CEntityDlg::OnCbnAnimationChange ()
{
	if ( !editEntity )
	{
		return;
	}

	int sel = cbAnimations.GetCurSel();
	CString animName;
	currentAnimation = NULL;
	int currFrame = 0;

	if ( sel != -1 ) {
		cbAnimations.GetLBText( sel , animName );
		if ( animName.GetLength() > 0 ) {
			//preserve the existing frame number
			currFrame = editEntity->epairs.GetInt ( "frame" , "1" );

			editEntity->epairs.Set("anim" , animName.GetBuffer(0));
			SetKeyValPairs(false/*don't update anims combo box :)*/ );
			
			//update the slider
			currentAnimation = gameEdit->ANIM_GetAnimFromEntityDef(editEntity->eclass->name , animName.GetBuffer(0));
			currentAnimationFrame = 0;

			if ( currentAnimation ) {
				slFrameSlider.SetRange( 1 , gameEdit->ANIM_GetNumFrames( currentAnimation ), TRUE );
				slFrameSlider.SetPos( currFrame );
				currentAnimationFrame = currFrame;
			}

			Sys_UpdateWindows(W_ALL);
		}
	}
}

void CEntityDlg::OnBnClickedStartAnimation()
{	
	if (!editEntity) {
		return;
	}
	SetTimer ( 0 , 1000/24 , NULL );
}

void CEntityDlg::OnBnClickedStopAnimation()
{
	KillTimer ( 0 );
}

void CEntityDlg::OnTimer(UINT nIDEvent)
{
	if ( !editEntity ) {
		OnBnClickedStopAnimation ();
		return;
	}
	
	if ( currentAnimation ) {
		currentAnimationFrame = ( (currentAnimationFrame++) % gameEdit->ANIM_GetNumFrames( currentAnimation ) );
		editEntity->epairs.SetInt ( "frame" , currentAnimationFrame );
		slFrameSlider.SetPos ( currentAnimationFrame );
		UpdateFromAnimationFrame (false/*don't update key/value display*/);		

		Sys_UpdateWindows ( W_CAMERA | W_XY );
	}
}

void CEntityDlg::AddCurvePoints() {
	if ( editEntity == NULL || editEntity->curve == NULL ) {
		return;
	}

	// add one point 64 units from the direction of the two points int he curve
	int c = editEntity->curve->GetNumValues();
	idVec3 start;
	idVec3 end;
	if ( c > 1 ) {
		start = editEntity->curve->GetValue( c - 2 );
		end = editEntity->curve->GetValue( c - 1 );
		idVec3 dir = end - start;
		dir.Normalize();
		start = end + 64 * dir;
	} else  if ( c > 0 ) {
		start = editEntity->curve->GetValue( 0 );
		start.x += 64;
		start.y += 64; 
	} else {
		start = editEntity->origin;
	}
	
	editEntity->curve->AddValue( editEntity->curve->GetNumValues() * 100, start );

	if ( g_qeglobals.d_select_mode == sel_editpoint ) {
		g_qeglobals.d_select_mode = sel_brush;
		EditCurvePoints();
	}

	Sys_UpdateWindows( W_CAMERA | W_XY );

}

void CEntityDlg::EditCurvePoints() {

	if ( editEntity == NULL || editEntity->curve == NULL ) {
		return;
	}

	if ( g_qeglobals.d_select_mode == sel_editpoint ) {
		g_qeglobals.d_select_mode = sel_brush;
		return;
	}

	g_qeglobals.d_select_mode = sel_editpoint;

	g_qeglobals.d_numpoints = 0;
	g_qeglobals.d_num_move_points = 0;
	int c = editEntity->curve->GetNumValues();
	for ( int i = 0; i < c; i++ ) {
		if ( g_qeglobals.d_numpoints < MAX_POINTS - 1 ) {
			g_qeglobals.d_points[g_qeglobals.d_numpoints++] = editEntity->curve->GetValue( i );
		}
	}
	Sys_UpdateWindows( W_XY | W_CAMERA );

}

void CEntityDlg::InsertCurvePoint() {
	if ( editEntity == NULL || editEntity->curve == NULL ) {
		return;
	}

	if ( g_qeglobals.d_select_mode != sel_editpoint ) {
		return;
	}

	if ( g_qeglobals.d_num_move_points == 0 ) {
		return;
	}

	for ( int i = 0; i < editEntity->curve->GetNumValues(); i++ ) {
		if ( PointInMoveList( editEntity->curve->GetValueAddress( i ) ) >= 0 ) {
			if ( i == editEntity->curve->GetNumValues() - 1 ) {
				// just do an add
				AddCurvePoints();
			} else {
				idCurve<idVec3> *newCurve = Entity_MakeCurve( editEntity );

				if ( newCurve == NULL ) {
					return;
				}

				for ( int j = 0; j < editEntity->curve->GetNumValues(); j++ ) {
					if ( j == i ) {
						idVec3 start;
						idVec3 end;
						if ( i > 0 ) {
							start = editEntity->curve->GetValue( i - 1 );
							end = editEntity->curve->GetValue( i );
							start += end;
							start *= 0.5f;
						} else {
							start = editEntity->curve->GetValue( 0 );
							if ( editEntity->curve->GetNumValues() > 1 ) {
								end = start;
								start = editEntity->curve->GetValue ( 1 );
								idVec3 dir = end - start;
								dir.Normalize();
								start = end + 64 * dir;
							} else {
								end = start;
								end.x += 64;
								end.y += 64;
							}
						}
						newCurve->AddValue( newCurve->GetNumValues() * 100, start );
					} 
					newCurve->AddValue( newCurve->GetNumValues() * 100, editEntity->curve->GetValue( j ) );
				}
				delete editEntity->curve;
				editEntity->curve = newCurve;
			}
			g_qeglobals.d_num_move_points = 0;
			break;
		}
	}
	UpdateEntityCurve();

	Sys_UpdateWindows( W_XY | W_CAMERA );

}

void CEntityDlg::DeleteCurvePoint() {

	if ( editEntity == NULL || editEntity->curve == NULL ) {
		return;
	}

	if ( g_qeglobals.d_select_mode != sel_editpoint ) {
		return;
	}


	if ( g_qeglobals.d_num_move_points == 0 ) {
		return;
	}

	for ( int i = 0; i < editEntity->curve->GetNumValues(); i++ ) {
		if ( PointInMoveList( editEntity->curve->GetValueAddress( i ) ) >= 0 ) {
			editEntity->curve->RemoveIndex( i );
			g_qeglobals.d_num_move_points = 0;
			break;
		}
	}
	UpdateEntityCurve();

	Sys_UpdateWindows( W_XY | W_CAMERA );

}


void CEntityDlg::UpdateEntityCurve() {
	
	if ( editEntity == NULL ) {
		return;
	}

	Entity_UpdateCurveData( editEntity );

	if ( g_qeglobals.d_select_mode == sel_editpoint ) {
		g_qeglobals.d_numpoints = 0;
		int c = editEntity->curve->GetNumValues();
		for ( int i = 0; i < c; i++ ) {
			if ( g_qeglobals.d_numpoints < MAX_POINTS - 1 ) {
				g_qeglobals.d_points[g_qeglobals.d_numpoints++] = editEntity->curve->GetValue( i );
			}
		}
	}

	Sys_UpdateWindows( W_ENTITY );
}


void CEntityDlg::SelectCurvePointByRay(const idVec3 &org, const idVec3 &dir, int buttons) {
	int		i, besti;
	float	d, bestd;
	idVec3	temp;

	if ( editEntity == NULL ) {
		return;
	}
	// find the point closest to the ray
	float scale = g_pParentWnd->ActiveXY()->Scale();
	besti = -1;
	bestd = 8 / scale / 2;
	//bestd = 8;

	for (i = 0; i < g_qeglobals.d_numpoints; i++) {
		temp = g_qeglobals.d_points[i] - org;
		d = temp * dir;
		temp = org + d * dir;
		temp = g_qeglobals.d_points[i] - temp;
		d = temp.Length();
		if ( d <= bestd ) {
			bestd = d;
			besti = i;
		}
	}

	if (besti == -1) {
		return;
	}

	g_qeglobals.d_num_move_points = 0;
	assert ( besti < editEntity->curve->GetNumValues() );
	g_qeglobals.d_move_points[ g_qeglobals.d_num_move_points++ ] = editEntity->curve->GetValueAddress( besti );
}


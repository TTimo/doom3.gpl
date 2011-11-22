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
#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "PropertyList.h"
#include "PreviewDlg.h"

// CEntityDlg dialog



class CEntityDlg : public CDialog
{
	DECLARE_DYNAMIC(CEntityDlg)
public:
	CEntityDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEntityDlg();
	void SetDict(idDict *_dict) {
		dict = dict;
	}
	void SetEditEntity(entity_t *ent) {
		editEntity = ent;
	}
	void CreateEntity();
	void AssignModel ();
	static CPreviewDlg *ShowModelChooser();
	static CPreviewDlg *ShowGuiChooser();
	static CPreviewDlg *ShowSoundChooser();
	static CPreviewDlg *ShowMaterialChooser();
	static CPreviewDlg *ShowParticleChooser();
	static CPreviewDlg *ShowSkinChooser( entity_t *ent );
	
	void SetKeyVal(const char *key, const char *val) {
		editKey.SetWindowText(key);
		editVal.SetWindowText(val);
	}

	void EditCurvePoints();
	void AddCurvePoints();
	void InsertCurvePoint();
	void DeleteCurvePoint();

// Dialog Data
	enum { IDD = IDD_DIALOG_ENTITY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//DECLARE_MESSAGE_MAP()
public:

	virtual BOOL OnInitDialog();
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	void AddClassNames();
	void UpdateEntitySel(eclass_t *ent);
	void SetKeyValPairs( bool updateAnims = true );
	static const char *TranslateString(const char *p);
	void AddProp();
	void DelProp();
	void UpdateFromListBox();
	CEdit editKey;
	CEdit editVal;
	void UpdateKeyVal(const char *key, const char *val);
	void SelectCurvePointByRay(const idVec3 &org, const idVec3 &dir, int buttons);
	void UpdateEntityCurve();


private:
	entity_t *editEntity;
	bool multipleEntities;
	CPropertyList listKeyVal;
	CPropertyList listVars;
	CComboBox comboClass;
	idDict *dict;
	const idMD5Anim* currentAnimation;
	int currentAnimationFrame;

	const char *AngleKey();

	idPointListInterface curvePoints;
public:
	void UpdateFromAnimationFrame ( bool updateKeyValueDisplay = true);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CStatic staticTitle;
	CStatic staticKey;
	CStatic staticVal;
	CStatic staticFrame;
	CButton btnPlayAnim;
	CButton btnStopAnim;
	CButton btnBrowse;
	CButton btn135;
	CButton btn90;
	CButton btn45;
	CButton btn180;
	CButton btn360;
	CButton btn225;
	CButton btn270;
	CButton btn315;
	CButton btnUp;
	CButton btnDown;
	CButton btnModel;
	CButton btnSound;
	CButton btnGui;
	CButton btnParticle;
	CButton btnSkin;
	CButton btnCurve;
	CComboBox cbAnimations;
	CSliderCtrl slFrameSlider;
	afx_msg void OnCbnSelchangeComboClass();
	afx_msg void OnLbnSelchangeListkeyval();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedE135();
	afx_msg void OnBnClickedE90();
	afx_msg void OnBnClickedE45();
	afx_msg void OnBnClickedE180();
	afx_msg void OnBnClickedE0();
	afx_msg void OnBnClickedE225();
	afx_msg void OnBnClickedE270();
	afx_msg void OnBnClickedE315();
	afx_msg void OnBnClickedEUp();
	afx_msg void OnBnClickedEDown();
	afx_msg void OnBnClickedButtonModel();
	afx_msg void OnBnClickedButtonSound();
	afx_msg void OnBnClickedButtonGui();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnCbnDblclkComboClass();
	afx_msg void OnBnClickedButtonCreate();
	afx_msg void OnBnClickedStartAnimation();
	afx_msg void OnBnClickedStopAnimation();
	CButton btnCreate;
	afx_msg void OnLbnDblclkListkeyval();
	afx_msg void OnLbnSelchangeListVars();
	afx_msg void OnLbnDblclkListVars();
	void OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnAnimationChange ();
	void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedButtonParticle();
	afx_msg void OnBnClickedButtonSkin();
	afx_msg void OnBnClickedButtonCurve();

};

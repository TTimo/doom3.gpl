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

#ifndef __DIALOGPARTICLEEDITOR_H__
#define __DIALOGPARTICLEEDITOR_H__

#pragma once

class CRangeSlider : public CSliderCtrl {
public:
	void SetValueRange(float _low, float _high) {
		low = _low;
		high = _high;
	}
	
	void SetValuePos( float val ) {
		SetPos( GetRangeMin() + ( GetRangeMax() - GetRangeMin() ) * ( val - low ) / ( high - low ) );
	}

	float GetValue() {
		return low + ( high - low ) * ( float )( GetPos() - GetRangeMin() ) / ( GetRangeMax() - GetRangeMin() ); 
	}
private:
	float low, high;
};

// CDialogParticleEditor dialog

class CDialogParticleEditor : public CDialog {

	DECLARE_DYNAMIC(CDialogParticleEditor)

public:
						CDialogParticleEditor(CWnd* pParent = NULL);   // standard constructor
	virtual				~CDialogParticleEditor();

	void				SelectParticle( const char *name );
	void				SetParticleVisualization( int i );
	void				SetVectorControlUpdate( idQuat rotation );

	enum { TESTMODEL, IMPACT, MUZZLE, FLIGHT, SELECTED };

	//{{AFX_VIRTUAL(CDialogParticleEditor)
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL		OnInitDialog();
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CDialogParticleEditor)
	afx_msg void		OnCbnSelchangeComboParticles();
	afx_msg void		OnCbnSelchangeComboPath();
	afx_msg void		OnLbnSelchangeListStages();
	afx_msg void		OnBnClickedButtonAddstage();
	afx_msg void		OnBnClickedButtonRemovestage();
	afx_msg void		OnBnClickedButtonBrowsematerial();
	afx_msg void		OnBnClickedButtonBrowsecolor();
	afx_msg void		OnBnClickedButtonBrowsefadecolor();
	afx_msg void		OnBnClickedButtonBrowseEntitycolor();
	afx_msg void		OnBnClickedRadioRect();
	afx_msg void		OnBnClickedRadioSphere();
	afx_msg void		OnBnClickedRadioCylinder();
	afx_msg void		OnBnClickedRadioCone();
	afx_msg void		OnBnClickedRadioOutward();
	afx_msg void		OnBnClickedRadioView();
	afx_msg void		OnBnClickedRadioAimed();
	afx_msg void		OnBnClickedRadioX();
	afx_msg void		OnBnClickedRadioY();
	afx_msg void		OnBnClickedRadioZ();
	afx_msg void		OnBnClickedButtonHidestage();
	afx_msg void		OnBnClickedButtonShowstage();
	afx_msg void		OnBnClickedCheckOneshot();
	afx_msg void		OnBnClickedButtonNew();
	afx_msg void		OnBnClickedButtonSave();
	afx_msg void		OnBnClickedButtonSaveAs();
	afx_msg void		OnBnClickedButtonSaveParticles();
	afx_msg void		OnBnClickedWorldGravity();
	afx_msg void		OnBnClickedEntityColor();
	afx_msg void		OnBnClickedTestModel();
	afx_msg void		OnBnClickedImpact();
	afx_msg void		OnBnClickedMuzzle();
	afx_msg void		OnBnClickedFlight();
	afx_msg void		OnBnClickedSelected();
	afx_msg void		OnBnClickedDoom();
	afx_msg void		OnBnClickedButtonUpdate();
	afx_msg void		OnBnClickedParticleMode();
	afx_msg	void		OnBtnYup();
	afx_msg	void		OnBtnYdn();
	afx_msg	void		OnBtnXdn();
	afx_msg	void		OnBtnXup();
	afx_msg	void		OnBtnZup();
	afx_msg	void		OnBtnZdn();
	afx_msg	void		OnBtnDrop();
	afx_msg void		OnDestroy();
	afx_msg void		OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	//{{AFX_DATA(CDialogParticleEditor)
	enum				{ IDD = IDD_DIALOG_PARTICLE_EDITOR };
	CComboBox			comboParticle;
	CListBox			listStages;
	CRangeSlider		sliderBunching;
	CRangeSlider		sliderFadeIn;
	CRangeSlider		sliderFadeOut;
	CRangeSlider		sliderFadeFraction;
	CRangeSlider		sliderCount;
	CRangeSlider		sliderTime;
	CRangeSlider		sliderGravity;
	CRangeSlider		sliderSpeedFrom;
	CRangeSlider		sliderSpeedTo;
	CRangeSlider		sliderRotationFrom;
	CRangeSlider		sliderRotationTo;
	CRangeSlider		sliderSizeFrom;
	CRangeSlider		sliderSizeTo;
	CRangeSlider		sliderAspectFrom;
	CRangeSlider		sliderAspectTo;
	CVectorCtl			vectorControl;
	CString				depthHack;
	CString				matName;
	CString				animFrames;
	CString				animRate;
	CString				color;
	CString				fadeColor;
	CString				fadeIn;
	CString				fadeOut;
	CString				fadeFraction;
	CString				count;
	CString				time;
	CString				timeOffset;
	CString				deadTime;
	CString				gravity;
	CString				bunching;
	CString				offset;
	CString				xSize;
	CString				ySize;
	CString				zSize;
	CString				ringOffset;
	CStatic				staticDirectionParm;
	CString				directionParm;
	int					direction;
	int					orientation;
	int					distribution;
	CString				viewOrigin;
	CString				speedFrom;
	CString				speedTo;
	CString				rotationFrom;
	CString				rotationTo;
	CString				sizeFrom;
	CString				sizeTo;
	CString				aspectFrom;
	CString				aspectTo;
	CString				customPath;
	CString				customParms;
	CString				trails;
	CString				trailTime;
	CString				cycles;
	CEdit				editRingOffset;
	BOOL				worldGravity;
	BOOL				entityColor;
	BOOL				randomDistribution;
	CString				initialAngle;
	CString				boundsExpansion;
	CString				customDesc;

	BOOL				particleMode;
	//}}AFX_DATA

	int					visualization;

private:
	void				EnumParticles();
	void				AddStage();
	void				RemoveStage();
	void				ShowStage();
	void				HideStage();
	idDeclParticle *	GetCurParticle();
	idParticleStage *	GetCurStage();
	void				ClearDlgVars();
	void				CurStageToDlgVars();
	void				DlgVarsToCurStage();
	void				ShowCurrentStage();
	void				UpdateControlInfo();
	void				SetParticleView();
	void				UpdateParticleData();
	CToolTipCtrl		toolTipCtrl;
	BOOL				PreTranslateMessage(MSG *pMsg);
	void				SetSelectedModel( const char *val );
	void				EnableStageControls();
	void				EnableEditControls();
	void				UpdateSelectedOrigin( float x, float y, float z );
	bool				mapModified;
protected:
	virtual void OnOK();
};

#endif /* !__DIALOGPARTICLEEDITOR_H__ */

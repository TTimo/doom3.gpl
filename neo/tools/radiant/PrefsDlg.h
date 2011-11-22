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

#ifndef __PREFSDLG_H__
#define __PREFSDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CPrefsDlg dialog

#define MAX_TEXTURE_QUALITY 3

class CPrefsDlg : public CDialog
{
// Construction
public:
						CPrefsDlg(CWnd* pParent = NULL);   // standard constructor

	void				LoadPrefs();
	void				SavePrefs();
	void				SetGamePrefs();

// Dialog Data
	//{{AFX_DATA(CPrefsDlg)
	enum { IDD = IDD_DLG_PREFS };

	CSpinButtonCtrl		m_wndUndoSpin;
	CSpinButtonCtrl		m_wndFontSpin;
	CSliderCtrl			m_wndTexturequality;
	CSliderCtrl			m_wndCamSpeed;
	CSpinButtonCtrl		m_wndSpin;
	BOOL				m_bTextureLock;
	BOOL				m_bLoadLast;
	BOOL				m_bRunBefore;
	CString				m_strLastProject;
	CString				m_strLastMap;
	BOOL				m_bFace;
	BOOL				m_bRightClick;
	BOOL				m_bVertex;
	BOOL				m_bAutoSave;
	BOOL				m_bNewApplyHandling;
	CString				m_strAutoSave;
	BOOL				m_bLoadLastMap;
	BOOL				m_bGatewayHack;
	BOOL				m_bTextureWindow;
	BOOL				m_bSnapShots;
	float				m_fTinySize;
	BOOL				m_bCleanTiny;
	int					m_nStatusSize;
	BOOL				m_bCamXYUpdate;
	BOOL				m_bNewLightDraw;
	BOOL				m_bALTEdge;
	BOOL				m_bQE4Painting;
	BOOL				m_bSnapTToGrid;
	BOOL				m_bXZVis;
	BOOL				m_bYZVis;
	BOOL				m_bZVis;
	BOOL				m_bSizePaint;
	BOOL				m_bRotateLock;
	BOOL				m_bWideToolbar;
	BOOL				m_bNoClamp;
	int					m_nRotation;
	BOOL				m_bHiColorTextures;
	BOOL				m_bChaseMouse;
	BOOL				m_bTextureScrollbar;
	BOOL				m_bDisplayLists;
	BOOL				m_bNoStipple;
	int					m_nUndoLevels;
	CString				m_strMaps;
	CString				m_strModels;
	BOOL				m_bNewMapFormat;
	//}}AFX_DATA
	int					m_nMouseButtons;
	int					m_nAngleSpeed;
	int					m_nMoveSpeed;
	int					m_nAutoSave;
	bool				m_bCubicClipping;
	int					m_nCubicScale;
	BOOL				m_selectOnlyBrushes;
	BOOL				m_selectNoModels;
	BOOL				m_selectByBoundingBrush;
	int					m_nEntityShowState;
	int					m_nTextureScale;
	BOOL				m_bNormalizeColors;
	BOOL				m_bSwitchClip;
	BOOL				m_bSelectWholeEntities;
	int					m_nTextureQuality;
	BOOL				m_bGLLighting;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsDlg)
	protected:
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsDlg)
	afx_msg void		OnBtnBrowse();
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	afx_msg void		OnBtnBrowsepak();
	afx_msg void		OnBtnBrowseprefab();
	afx_msg void		OnBtnBrowseuserini();
	afx_msg void		OnSelchangeComboWhatgame();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif /* !__PREFSDLG_H__ */

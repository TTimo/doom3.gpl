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

// DialogAFProperties dialog

class DialogAFProperties : public CDialog {

	DECLARE_DYNAMIC(DialogAFProperties)

public:
						DialogAFProperties( CWnd* pParent = NULL );   // standard constructor
	virtual				~DialogAFProperties();
	void				LoadFile( idDeclAF *af );
	void				SaveFile( void );

	DialogAFBody *		bodyDlg;
	DialogAFConstraint *constraintDlg;

	enum				{ IDD = IDD_DIALOG_AF_PROPERTIES };

protected:
	virtual void		DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	virtual int			OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;
	afx_msg BOOL		OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnEnChangeEditModel();
	afx_msg void		OnEnChangeEditSkin();
	afx_msg void		OnBnClickedButtonBrowseModel();
	afx_msg void		OnBnClickedButtonBrowseSkin();
	afx_msg void		OnBnClickedCheckSelfcollision();
	afx_msg void		OnEnChangeEditContents();
	afx_msg void		OnEnChangeEditClipmask();
	afx_msg void		OnEnChangeEditLinearfriction();
	afx_msg void		OnDeltaposSpinLinearfriction(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditAngularfriction();
	afx_msg void		OnDeltaposSpinAngularfriction(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditContactfriction();
	afx_msg void		OnDeltaposSpinContactfriction(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditConstraintfriction();
	afx_msg void		OnDeltaposSpinConstraintfriction(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditTotalmass();
	afx_msg void		OnDeltaposSpinTotalmass(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditLinearvelocity();
	afx_msg void		OnEnChangeEditAngularvelocity();
	afx_msg void		OnEnChangeEditLinearacceleration();
	afx_msg void		OnEnChangeEditAngularacceleration();
	afx_msg void		OnEnChangeEditNomovetime();
	afx_msg void		OnEnChangeEditMinimummovetime();
	afx_msg void		OnEnChangeEditMaximummovetime();
	afx_msg void		OnEnChangeEditLineartolerance();
	afx_msg void		OnEnChangeEditAngulartolerance();

	DECLARE_MESSAGE_MAP()

private:
	idDeclAF *			file;

	//{{AFX_DATA(DialogAFProperties)
	CEdit				m_editModel;
	CEdit				m_editSkin;
	BOOL				m_selfCollision;
	CEdit				m_editContents;
	CEdit				m_editClipMask;
	float				m_linearFriction;
	float				m_angularFriction;
	float				m_contactFriction;
	float				m_constraintFriction;
	float				m_totalMass;
	float				m_suspendLinearVelocity;
	float				m_suspendAngularVelocity;
	float				m_suspendLinearAcceleration;
	float				m_suspendAngularAcceleration;
	float				m_noMoveTime;
	float				m_minMoveTime;
	float				m_maxMoveTime;
	float				m_linearTolerance;
	float				m_angularTolerance;
	//}}AFX_DATA

	static toolTip_t	toolTips[];

private:
	void				UpdateFile( void );
	void				ClearFile( void );
};

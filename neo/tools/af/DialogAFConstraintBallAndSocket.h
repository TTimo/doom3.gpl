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

// DialogAFConstraintBallAndSocket dialog

class DialogAFConstraintBallAndSocket : public CDialog {

	DECLARE_DYNAMIC(DialogAFConstraintBallAndSocket)

public:
						DialogAFConstraintBallAndSocket(CWnd* pParent = NULL);   // standard constructor
	virtual				~DialogAFConstraintBallAndSocket();
	void				LoadFile( idDeclAF *af );
	void				SaveFile( void );
	void				LoadConstraint( idDeclAF_Constraint *c );
	void				SaveConstraint( void );
	void				UpdateFile( void );

						enum { IDD = IDD_DIALOG_AF_CONSTRAINT_BALLANDSOCKET };

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual int			OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;
	afx_msg BOOL		OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedRadioAnchorJoint();
	afx_msg void		OnBnClickedRadioAnchorCoordinates();
	afx_msg void		OnCbnSelchangeComboAnchorJoint();
	afx_msg void		OnEnChangeEditAnchorX();
	afx_msg void		OnEnChangeEditAnchorY();
	afx_msg void		OnEnChangeEditAnchorZ();
	afx_msg void		OnDeltaposSpinAnchorX(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnDeltaposSpinAnchorY(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnDeltaposSpinAnchorZ(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedRadioBasLimitNone();
	afx_msg void		OnBnClickedRadioBasLimitCone();
	afx_msg void		OnBnClickedRadioBasLimitPyramid();
	afx_msg void		OnEnChangeEditBasLimitConeAngle();
	afx_msg void		OnDeltaposSpinBasLimitConeAngle(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditBasLimitPyramidAngle1();
	afx_msg void		OnDeltaposSpinBasLimitPyramidAngle1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditBasLimitPyramidAngle2();
	afx_msg void		OnDeltaposSpinBasLimitPyramidAngle2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditBasLimitRoll();
	afx_msg void		OnDeltaposSpinBasLimitRoll(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedRadioBasLimitBone();
	afx_msg void		OnBnClickedRadioBasLimitAngles();
	afx_msg void		OnCbnSelchangeComboBasLimitJoint1();
	afx_msg void		OnCbnSelchangeComboBasLimitJoint2();
	afx_msg void		OnEnChangeEditBasLimitPitch();
	afx_msg void		OnDeltaposSpinBasLimitPitch(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditBasLimitYaw();
	afx_msg void		OnDeltaposSpinBasLimitYaw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedRadioBasLimitAxisBone();
	afx_msg void		OnBnClickedRadioBasLimitAxisAngles();
	afx_msg void		OnCbnSelchangeComboBasLimitAxisJoint1();
	afx_msg void		OnCbnSelchangeComboBasLimitAxisJoint2();
	afx_msg void		OnEnChangeEditBasLimitAxisPitch();
	afx_msg void		OnDeltaposSpinBasLimitAxisPitch(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditBasLimitAxisYaw();
	afx_msg void		OnDeltaposSpinBasLimitAxisYaw(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()

private:
	idDeclAF *			file;
	idDeclAF_Constraint*constraint;

	//{{AFX_DATA(DialogAFConstraintBallAndSocket)
	CComboBox			m_comboAnchorJoint;
	float				m_anchor_x;
	float				m_anchor_y;
	float				m_anchor_z;
	float				m_coneAngle;
	float				m_pyramidAngle1;
	float				m_pyramidAngle2;
	CComboBox			m_comboLimitJoint1;
	CComboBox			m_comboLimitJoint2;
	float				m_limitPitch;
	float				m_limitYaw;
	float				m_limitRoll;
	CComboBox			m_comboLimitAxisJoint1;
	CComboBox			m_comboLimitAxisJoint2;
	float				m_limitAxisPitch;
	float				m_limitAxisYaw;
	//}}AFX_DATA

	static toolTip_t	toolTips[];

private:
	void				InitJointLists( void );
};

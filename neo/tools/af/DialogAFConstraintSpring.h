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

// DialogAFConstraintSpring dialog

class DialogAFConstraintSpring : public CDialog {

	DECLARE_DYNAMIC(DialogAFConstraintSpring)

public:
						DialogAFConstraintSpring(CWnd* pParent = NULL);   // standard constructor
	virtual				~DialogAFConstraintSpring();
	void				LoadFile( idDeclAF *af );
	void				SaveFile( void );
	void				LoadConstraint( idDeclAF_Constraint *c );
	void				SaveConstraint( void );
	void				UpdateFile( void );

						enum { IDD = IDD_DIALOG_AF_CONSTRAINT_HINGE };

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
	afx_msg void		OnBnClickedRadioAnchor2Joint();
	afx_msg void		OnBnClickedRadioAnchor2Coordinates();
	afx_msg void		OnCbnSelchangeComboAnchor2Joint();
	afx_msg void		OnEnChangeEditAnchor2X();
	afx_msg void		OnEnChangeEditAnchor2Y();
	afx_msg void		OnEnChangeEditAnchor2Z();
	afx_msg void		OnDeltaposSpinAnchor2X(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnDeltaposSpinAnchor2Y(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnDeltaposSpinAnchor2Z(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditSpringStretch();
	afx_msg void		OnDeltaposSpinSpringStretch(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditSpringCompress();
	afx_msg void		OnDeltaposSpinSpringCompress(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditSpringDamping();
	afx_msg void		OnDeltaposSpinSpringDamping(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnEnChangeEditSpringRestLength();
	afx_msg void		OnDeltaposSpinSpringRestLength(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedRadioLimitNoMinLength();
	afx_msg void		OnBnClickedRadioLimitMinLength();
	afx_msg void		OnEnChangeEditLimitMinLength();
	afx_msg void		OnDeltaposSpinLimitMinLength(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void		OnBnClickedRadioLimitNoMaxLength();
	afx_msg void		OnBnClickedRadioLimitMaxLength();
	afx_msg void		OnEnChangeEditLimitMaxLength();
	afx_msg void		OnDeltaposSpinLimitMaxLength(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()

private:
	idDeclAF *			file;
	idDeclAF_Constraint*constraint;

	//{{AFX_DATA(DialogAFConstraintSpring)
	CComboBox			m_comboAnchorJoint;
	float				m_anchor_x;
	float				m_anchor_y;
	float				m_anchor_z;
	CComboBox			m_comboAnchor2Joint;
	float				m_anchor2_x;
	float				m_anchor2_y;
	float				m_anchor2_z;
	float				m_stretch;
	float				m_compress;
	float				m_damping;
	float				m_restLength;
	float				m_minLength;
	float				m_maxLength;
	//}}AFX_DATA

	static toolTip_t	toolTips[];

private:
	void				InitJointLists( void );
};

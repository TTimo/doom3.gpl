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

// DialogAFView dialog

class DialogAFView : public CDialog {

	DECLARE_DYNAMIC(DialogAFView)

public:
						DialogAFView(CWnd* pParent = NULL);   // standard constructor
	virtual				~DialogAFView();

	enum				{ IDD = IDD_DIALOG_AF_VIEW };

protected:
	virtual void		DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual int			OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;
	afx_msg BOOL		OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void		OnBnClickedCheckViewBodies();
	afx_msg void		OnBnClickedCheckViewBodynames();
	afx_msg void		OnBnClickedCheckViewBodyMass();
	afx_msg void		OnBnClickedCheckViewTotalMass();
	afx_msg void		OnBnClickedCheckViewInertiatensor();
	afx_msg void		OnBnClickedCheckViewVelocity();
	afx_msg void		OnBnClickedCheckViewConstraints();
	afx_msg void		OnBnClickedCheckViewConstraintnames();
	afx_msg void		OnBnClickedCheckViewPrimaryonly();
	afx_msg void		OnBnClickedCheckViewLimits();
	afx_msg void		OnBnClickedCheckViewConstrainedBodies();
	afx_msg void		OnBnClickedCheckViewTrees();
	afx_msg void		OnBnClickedCheckMd5Skeleton();
	afx_msg void		OnBnClickedCheckMd5Skeletononly();
	afx_msg void		OnBnClickedCheckLinesDepthtest();
	afx_msg void		OnBnClickedCheckLinesUsearrows();
	afx_msg void		OnBnClickedCheckPhysicsNofriction();
	afx_msg void		OnBnClickedCheckPhysicsNolimits();
	afx_msg void		OnBnClickedCheckPhysicsNogravity();
	afx_msg void		OnBnClickedCheckPhysicsNoselfcollision();
	afx_msg void		OnBnClickedCheckPhysicsTiming();
	afx_msg void		OnBnClickedCheckPhysicsDragEntities();
	afx_msg void		OnBnClickedCheckPhysicsShowDragSelection();

	DECLARE_MESSAGE_MAP()

private:
	//{{AFX_DATA(DialogAFView)
	BOOL				m_showBodies;
	BOOL				m_showBodyNames;
	BOOL				m_showMass;
	BOOL				m_showTotalMass;
	BOOL				m_showInertia;
	BOOL				m_showVelocity;
	BOOL				m_showConstraints;
	BOOL				m_showConstraintNames;
	BOOL				m_showPrimaryOnly;
	BOOL				m_showLimits;
	BOOL				m_showConstrainedBodies;
	BOOL				m_showTrees;
	BOOL				m_showSkeleton;
	BOOL				m_showSkeletonOnly;
	BOOL				m_debugLineDepthTest;
	BOOL				m_debugLineUseArrows;
	BOOL				m_noFriction;
	BOOL				m_noLimits;
	BOOL				m_noGravity;
	BOOL				m_noSelfCollision;
	BOOL				m_showTimings;
	BOOL				m_dragEntity;
	BOOL				m_dragShowSelection;
	//}}AFX_DATA

	float				m_gravity;

	static toolTip_t	toolTips[];
};

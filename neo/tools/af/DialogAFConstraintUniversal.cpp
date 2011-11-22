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

#include "../../sys/win32/rc/AFEditor_resource.h"

#include "DialogAF.h"
#include "DialogAFConstraint.h"
#include "DialogAFConstraintUniversal.h"


// DialogAFConstraintUniversal dialog

toolTip_t DialogAFConstraintUniversal::toolTips[] = {
	{ IDC_RADIO_ANCHOR_JOINT, "use the position of a joint for the anchor" },
	{ IDC_COMBO_ANCHOR_JOINT, "anchor joint name" },
	{ IDC_RADIO_ANCHOR_COORDINATES, "use absolute coordinates for the anchor" },
	{ IDC_EDIT_ANCHOR_X, "anchor x-coordinate" },
	{ IDC_EDIT_ANCHOR_Y, "anchor y-coordinate" },
	{ IDC_EDIT_ANCHOR_Z, "anchor z-coordinate" },
	{ IDC_RADIO_UNIVERSAL_BONE_SHAFT1, "use a bone for the first shaft" },
	{ IDC_RADIO_UNIVERSAL_ANGLES_SHAFT1, "use angles to set the orientation of the first shaft" },
	{ IDC_COMBO_UNIVERSAL_JOINT1_SHAFT1, "bone start joint" },
	{ IDC_COMBO_UNIVERSAL_JOINT2_SHAFT1, "bone end joint" },
	{ IDC_EDIT_UNIVERSAL_PITCH_SHAFT1, "pitch angle" },
	{ IDC_EDIT_UNIVERSAL_YAW_SHAFT1, "yaw angle" },
	{ IDC_RADIO_UNIVERSAL_BONE_SHAFT2, "use a bone for the second shaft" },
	{ IDC_RADIO_UNIVERSAL_ANGLES_SHAFT2, "use angles to set the orientation of the second shaft" },
	{ IDC_COMBO_UNIVERSAL_JOINT1_SHAFT2, "bone start joint" },
	{ IDC_COMBO_UNIVERSAL_JOINT2_SHAFT2, "bone end joint" },
	{ IDC_EDIT_UNIVERSAL_PITCH_SHAFT2, "pitch angle" },
	{ IDC_EDIT_UNIVERSAL_YAW_SHAFT2, "yaw angle" },
	{ IDC_RADIO_UNIVERSAL_LIMIT_NONE, "no joint limit" },
	{ IDC_RADIO_UNIVERSAL_LIMIT_CONE, "cone shaped joint limit" },
	{ IDC_RADIO_UNIVERSAL_LIMIT_PYRAMID, "pyramid shaped joint limit" },
	{ IDC_RADIO_UNIVERSAL_LIMIT_BONE, "use a bone for the limit orientation" },
	{ IDC_RADIO_UNIVERSAL_LIMIT_ANGLES, "use angles for the limit orientation" },
	{ IDC_COMBO_UNIVERSAL_LIMIT_JOINT1, "bone start joint" },
	{ IDC_COMBO_UNIVERSAL_LIMIT_JOINT2, "bone end joint" },
	{ IDC_EDIT_UNIVERSAL_LIMIT_PITCH, "pitch angle" },
	{ IDC_EDIT_UNIVERSAL_LIMIT_YAW, "yaw angle" },
	{ IDC_EDIT_UNIVERSAL_LIMIT_ROLL, "roll angle" },
	{ IDC_EDIT_UNIVERSAL_LIMIT_CONE_ANGLE, "cone angle" },
	{ IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE1, "first pyramid angle" },
	{ IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE2, "second pyramid angle" },
	{ 0, NULL }
};

IMPLEMENT_DYNAMIC(DialogAFConstraintUniversal, CDialog)

/*
================
DialogAFConstraintUniversal::DialogAFConstraintUniversal
================
*/
DialogAFConstraintUniversal::DialogAFConstraintUniversal(CWnd* pParent /*=NULL*/)
	: CDialog(DialogAFConstraintUniversal::IDD, pParent)
	, m_anchor_x(0)
	, m_anchor_y(0)
	, m_anchor_z(0)
	, m_pitchShaft1(0)
	, m_yawShaft1(0)
	, m_pitchShaft2(0)
	, m_yawShaft2(0)
	, m_coneAngle(30.0f)
	, m_pyramidAngle1(30.0f)
	, m_pyramidAngle2(30.0f)
	, m_limitPitch(0)
	, m_limitYaw(0)
	, m_limitRoll(0)
	, constraint(NULL)
	, file(NULL)
{
	Create( IDD_DIALOG_AF_CONSTRAINT_UNIVERSAL, pParent );
	EnableToolTips( TRUE );
}

/*
================
DialogAFConstraintUniversal::~DialogAFConstraintUniversal
================
*/
DialogAFConstraintUniversal::~DialogAFConstraintUniversal() {
}

/*
================
DialogAFConstraintUniversal::DoDataExchange
================
*/
void DialogAFConstraintUniversal::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAFConstraintUniversal)
	DDX_Control(pDX, IDC_COMBO_ANCHOR_JOINT, m_comboAnchorJoint);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_X, m_anchor_x);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_Y, m_anchor_y);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_Z, m_anchor_z);
	DDX_Control(pDX, IDC_COMBO_UNIVERSAL_JOINT1_SHAFT1, m_comboJoint1Shaft1);
	DDX_Control(pDX, IDC_COMBO_UNIVERSAL_JOINT2_SHAFT1, m_comboJoint2Shaft1);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_PITCH_SHAFT1, m_pitchShaft1);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_YAW_SHAFT1, m_yawShaft1);
	DDX_Control(pDX, IDC_COMBO_UNIVERSAL_JOINT1_SHAFT2, m_comboJoint1Shaft2);
	DDX_Control(pDX, IDC_COMBO_UNIVERSAL_JOINT2_SHAFT2, m_comboJoint2Shaft2);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_PITCH_SHAFT2, m_pitchShaft2);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_YAW_SHAFT2, m_yawShaft2);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_LIMIT_CONE_ANGLE, m_coneAngle);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE1, m_pyramidAngle1);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE2, m_pyramidAngle2);
	DDX_Control(pDX, IDC_COMBO_UNIVERSAL_LIMIT_JOINT1, m_comboLimitJoint1);
	DDX_Control(pDX, IDC_COMBO_UNIVERSAL_LIMIT_JOINT2, m_comboLimitJoint2);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_LIMIT_PITCH, m_limitPitch);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_LIMIT_YAW, m_limitYaw);
	DDX_Text(pDX, IDC_EDIT_UNIVERSAL_LIMIT_ROLL, m_limitRoll);
	//}}AFX_DATA_MAP
}

/*
================
DialogAFConstraintUniversal::InitJointLists
================
*/
void DialogAFConstraintUniversal::InitJointLists( void ) {
	m_comboAnchorJoint.ResetContent();
	m_comboJoint1Shaft1.ResetContent();
	m_comboJoint2Shaft1.ResetContent();
	m_comboJoint1Shaft2.ResetContent();
	m_comboJoint2Shaft2.ResetContent();
	m_comboLimitJoint1.ResetContent();
	m_comboLimitJoint2.ResetContent();

	if ( !file ) {
		return;
	}

	const idRenderModel *model = gameEdit->ANIM_GetModelFromName( file->model );
	if ( !model ) {
		return;
	}

	int numJoints = model->NumJoints();
	for ( int i = 0; i < numJoints; i++ ) {
		const char *jointName = model->GetJointName( (jointHandle_t) i );
		m_comboAnchorJoint.AddString( jointName );
		m_comboJoint1Shaft1.AddString( jointName );
		m_comboJoint2Shaft1.AddString( jointName );
		m_comboJoint1Shaft2.AddString( jointName );
		m_comboJoint2Shaft2.AddString( jointName );
		m_comboLimitJoint1.AddString( jointName );
		m_comboLimitJoint2.AddString( jointName );
	}
}

/*
================
DialogAFConstraintUniversal::LoadFile
================
*/
void DialogAFConstraintUniversal::LoadFile( idDeclAF *af ) {
	file = af;
	constraint = NULL;
	InitJointLists();
}

/*
================
DialogAFConstraintUniversal::SaveFile
================
*/
void DialogAFConstraintUniversal::SaveFile( void ) {
	SaveConstraint();
}

/*
================
DialogAFConstraintUniversal::LoadConstraint
================
*/
void DialogAFConstraintUniversal::LoadConstraint( idDeclAF_Constraint *c ) {
	int i, s1, s2;
	idAngles angles;
	idMat3 mat;

	constraint = c;

	// anchor
	SetSafeComboBoxSelection( &m_comboAnchorJoint, constraint->anchor.joint1.c_str(), -1 );
	m_anchor_x = constraint->anchor.ToVec3().x;
	m_anchor_y = constraint->anchor.ToVec3().y;
	m_anchor_z = constraint->anchor.ToVec3().z;
	if ( constraint->anchor.type == idAFVector::VEC_JOINT ) {
		i = IDC_RADIO_ANCHOR_JOINT;
	}
	else {
		i = IDC_RADIO_ANCHOR_COORDINATES;
	}
	CheckRadioButton( IDC_RADIO_ANCHOR_JOINT, IDC_RADIO_ANCHOR_COORDINATES, i );

	// shaft 1
	s1 = SetSafeComboBoxSelection( &m_comboJoint1Shaft1, constraint->shaft[0].joint1.c_str(), -1 );
	s2 = SetSafeComboBoxSelection( &m_comboJoint2Shaft1, constraint->shaft[0].joint2.c_str(), s1 );
	angles = constraint->shaft[0].ToVec3().ToAngles();
	m_pitchShaft1 = angles.pitch;
	m_yawShaft1 = angles.yaw;
	if ( constraint->shaft[0].type == idAFVector::VEC_BONEDIR ) {
		i = IDC_RADIO_UNIVERSAL_BONE_SHAFT1;
	}
	else {
		i = IDC_RADIO_UNIVERSAL_ANGLES_SHAFT1;
		constraint->shaft[0].type = idAFVector::VEC_COORDS;
	}
	CheckRadioButton( IDC_RADIO_UNIVERSAL_BONE_SHAFT1, IDC_RADIO_UNIVERSAL_ANGLES_SHAFT1, i );

	// shaft 2
	s1 = SetSafeComboBoxSelection( &m_comboJoint1Shaft2, constraint->shaft[1].joint1.c_str(), -1 );
	s2 = SetSafeComboBoxSelection( &m_comboJoint2Shaft2, constraint->shaft[1].joint2.c_str(), s1 );
	angles = constraint->shaft[1].ToVec3().ToAngles();
	m_pitchShaft2 = angles.pitch;
	m_yawShaft2 = angles.yaw;
	if ( constraint->shaft[1].type == idAFVector::VEC_BONEDIR ) {
		i = IDC_RADIO_UNIVERSAL_BONE_SHAFT2;
	}
	else {
		i = IDC_RADIO_UNIVERSAL_ANGLES_SHAFT2;
		constraint->shaft[1].type = idAFVector::VEC_COORDS;
	}
	CheckRadioButton( IDC_RADIO_UNIVERSAL_BONE_SHAFT2, IDC_RADIO_UNIVERSAL_ANGLES_SHAFT2, i );

	// limit
	if ( constraint->limit == idDeclAF_Constraint::LIMIT_CONE ) {
		i = IDC_RADIO_UNIVERSAL_LIMIT_CONE;
	}
	else if ( constraint->limit == idDeclAF_Constraint::LIMIT_PYRAMID ) {
		i = IDC_RADIO_UNIVERSAL_LIMIT_PYRAMID;
	}
	else {
		i = IDC_RADIO_UNIVERSAL_LIMIT_NONE;
	}
	CheckRadioButton( IDC_RADIO_UNIVERSAL_LIMIT_NONE, IDC_RADIO_UNIVERSAL_LIMIT_PYRAMID, i );

	m_coneAngle = constraint->limitAngles[0];
	m_pyramidAngle1 = constraint->limitAngles[0];
	m_pyramidAngle2 = constraint->limitAngles[1];
	m_limitRoll = constraint->limitAngles[2];
	angles = constraint->limitAxis.ToVec3().ToAngles();
	m_limitPitch = angles.pitch;
	m_limitYaw = angles.yaw;

	if ( constraint->limitAxis.type == idAFVector::VEC_BONEDIR ) {
		i = IDC_RADIO_UNIVERSAL_LIMIT_BONE;
	}
	else {
		i = IDC_RADIO_UNIVERSAL_LIMIT_ANGLES;
	}
	CheckRadioButton( IDC_RADIO_UNIVERSAL_LIMIT_BONE, IDC_RADIO_UNIVERSAL_LIMIT_ANGLES, i );
	s1 = SetSafeComboBoxSelection( &m_comboLimitJoint1, constraint->limitAxis.joint1.c_str(), -1 );
	s2 = SetSafeComboBoxSelection( &m_comboLimitJoint2, constraint->limitAxis.joint2.c_str(), s1 );

	// update displayed values
	UpdateData( FALSE );
}

/*
================
DialogAFConstraintUniversal::SaveConstraint
================
*/
void DialogAFConstraintUniversal::SaveConstraint( void ) {
	int s1, s2;
	CString str;
	idAngles angles;
	idMat3 mat;

	if ( !file || !constraint ) {
		return;
	}
	UpdateData( TRUE );

	// anchor
	GetSafeComboBoxSelection( &m_comboAnchorJoint, str, -1 );
	constraint->anchor.joint1 = str;
	constraint->anchor.ToVec3().x = m_anchor_x;
	constraint->anchor.ToVec3().y = m_anchor_y;
	constraint->anchor.ToVec3().z = m_anchor_z;

	// shaft 1
	if ( constraint->shaft[0].type == idAFVector::VEC_BONEDIR ) {
		s1 = GetSafeComboBoxSelection( &m_comboJoint1Shaft1, str, -1 );
		constraint->shaft[0].joint1 = str;
		s2 = GetSafeComboBoxSelection( &m_comboJoint2Shaft1, str, s1 );
		constraint->shaft[0].joint2 = str;
	}
	else {
		constraint->shaft[0].ToVec3() = idAngles( m_pitchShaft1, m_yawShaft1, 0.0f ).ToForward();
	}

	// shaft 2
	if ( constraint->shaft[1].type == idAFVector::VEC_BONEDIR ) {
		s1 = GetSafeComboBoxSelection( &m_comboJoint1Shaft2, str, -1 );
		constraint->shaft[1].joint1 = str;
		s2 = GetSafeComboBoxSelection( &m_comboJoint2Shaft2, str, s1 );
		constraint->shaft[1].joint2 = str;
	}
	else {
		constraint->shaft[1].ToVec3() = idAngles( m_pitchShaft2, m_yawShaft2, 0.0f ).ToForward();
	}

	// limit
	if ( constraint->limit == idDeclAF_Constraint::LIMIT_CONE ) {
		constraint->limitAngles[0] = m_coneAngle;
	}
	else {
		constraint->limitAngles[0] = m_pyramidAngle1;
	}
	constraint->limitAngles[1] = m_pyramidAngle2;
	constraint->limitAngles[2] = m_limitRoll;
	angles.pitch = m_limitPitch;
	angles.yaw = m_limitYaw;
	angles.roll = 0.0f;
	constraint->limitAxis.ToVec3() = angles.ToForward();
	s1 = GetSafeComboBoxSelection( &m_comboLimitJoint1, str, -1 );
	constraint->limitAxis.joint1 = str;
	s2 = GetSafeComboBoxSelection( &m_comboLimitJoint2, str, s1 );
	constraint->limitAxis.joint2 = str;

	AFDialogSetFileModified();
}

/*
================
DialogAFConstraintUniversal::UpdateFile
================
*/
void DialogAFConstraintUniversal::UpdateFile( void ) {
	SaveConstraint();
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAFConstraintUniversal::OnToolHitTest
================
*/
int DialogAFConstraintUniversal::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CDialog::OnToolHitTest( point, pTI );
	return DefaultOnToolHitTest( toolTips, this, point, pTI );
}


BEGIN_MESSAGE_MAP(DialogAFConstraintUniversal, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_BN_CLICKED(IDC_RADIO_ANCHOR_JOINT, OnBnClickedRadioAnchorJoint)
	ON_BN_CLICKED(IDC_RADIO_ANCHOR_COORDINATES, OnBnClickedRadioAnchorCoordinates)
	ON_CBN_SELCHANGE(IDC_COMBO_ANCHOR_JOINT, OnCbnSelchangeComboAnchorJoint)
	ON_EN_CHANGE(IDC_EDIT_ANCHOR_X, OnEnChangeEditAnchorX)
	ON_EN_CHANGE(IDC_EDIT_ANCHOR_Y, OnEnChangeEditAnchorY)
	ON_EN_CHANGE(IDC_EDIT_ANCHOR_Z, OnEnChangeEditAnchorZ)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANCHOR_X, OnDeltaposSpinAnchorX)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANCHOR_Y, OnDeltaposSpinAnchorY)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANCHOR_Z, OnDeltaposSpinAnchorZ)
	ON_BN_CLICKED(IDC_RADIO_UNIVERSAL_BONE_SHAFT1, OnBnClickedRadioUniversalBoneShaft1)
	ON_BN_CLICKED(IDC_RADIO_UNIVERSAL_ANGLES_SHAFT1, OnBnClickedRadioUniversalAnglesShaft1)
	ON_CBN_SELCHANGE(IDC_COMBO_UNIVERSAL_JOINT1_SHAFT1, OnCbnSelchangeComboUniversalJoint1Shaft1)
	ON_CBN_SELCHANGE(IDC_COMBO_UNIVERSAL_JOINT2_SHAFT1, OnCbnSelchangeComboUniversalJoint2Shaft1)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_PITCH_SHAFT1, OnEnChangeEditUniversalPitchShaft1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_PITCH_SHAFT1, OnDeltaposSpinUniversalPitchShaft1)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_YAW_SHAFT1, OnEnChangeEditUniversalYawShaft1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_YAW_SHAFT1, OnDeltaposSpinUniversalYawShaft1)
	ON_BN_CLICKED(IDC_RADIO_UNIVERSAL_BONE_SHAFT2, OnBnClickedRadioUniversalBoneShaft2)
	ON_BN_CLICKED(IDC_RADIO_UNIVERSAL_ANGLES_SHAFT2, OnBnClickedRadioUniversalAnglesShaft2)
	ON_CBN_SELCHANGE(IDC_COMBO_UNIVERSAL_JOINT1_SHAFT2, OnCbnSelchangeComboUniversalJoint1Shaft2)
	ON_CBN_SELCHANGE(IDC_COMBO_UNIVERSAL_JOINT2_SHAFT2, OnCbnSelchangeComboUniversalJoint2Shaft2)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_PITCH_SHAFT2, OnEnChangeEditUniversalPitchShaft2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_PITCH_SHAFT2, OnDeltaposSpinUniversalPitchShaft2)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_YAW_SHAFT2, OnEnChangeEditUniversalYawShaft2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_YAW_SHAFT2, OnDeltaposSpinUniversalYawShaft2)
	ON_BN_CLICKED(IDC_RADIO_UNIVERSAL_LIMIT_NONE, OnBnClickedRadioUniversalLimitNone)
	ON_BN_CLICKED(IDC_RADIO_UNIVERSAL_LIMIT_CONE, OnBnClickedRadioUniversalLimitCone)
	ON_BN_CLICKED(IDC_RADIO_UNIVERSAL_LIMIT_PYRAMID, OnBnClickedRadioUniversalLimitPyramid)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_LIMIT_CONE_ANGLE, OnEnChangeEditUniversalLimitConeAngle)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_LIMIT_CONE_ANGLE, OnDeltaposSpinUniversalLimitConeAngle)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE1, OnEnChangeEditUniversalLimitPyramidAngle1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_LIMIT_PYRAMID_ANGLE1, OnDeltaposSpinUniversalLimitPyramidAngle1)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE2, OnEnChangeEditUniversalLimitPyramidAngle2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_LIMIT_PYRAMID_ANGLE2, OnDeltaposSpinUniversalLimitPyramidAngle2)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_LIMIT_ROLL, OnEnChangeEditUniversalLimitRoll)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_LIMIT_ROLL, OnDeltaposSpinUniversalLimitRoll)
	ON_BN_CLICKED(IDC_RADIO_UNIVERSAL_LIMIT_BONE, OnBnClickedRadioUniversalLimitBone)
	ON_BN_CLICKED(IDC_RADIO_UNIVERSAL_LIMIT_ANGLES, OnBnClickedRadioUniversalLimitAngles)
	ON_CBN_SELCHANGE(IDC_COMBO_UNIVERSAL_LIMIT_JOINT1, OnCbnSelchangeComboUniversalLimitJoint1)
	ON_CBN_SELCHANGE(IDC_COMBO_UNIVERSAL_LIMIT_JOINT2, OnCbnSelchangeComboUniversalLimitJoint2)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_LIMIT_PITCH, OnEnChangeEditUniversalLimitPitch)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_LIMIT_PITCH, OnDeltaposSpinUniversalLimitPitch)
	ON_EN_CHANGE(IDC_EDIT_UNIVERSAL_LIMIT_YAW, OnEnChangeEditUniversalLimitYaw)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UNIVERSAL_LIMIT_YAW, OnDeltaposSpinUniversalLimitYaw)
END_MESSAGE_MAP()


// DialogAFConstraintUniversal message handlers

BOOL DialogAFConstraintUniversal::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

void DialogAFConstraintUniversal::OnBnClickedRadioAnchorJoint() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR_JOINT ) ) {
		if ( constraint ) {
			constraint->anchor.type = idAFVector::VEC_JOINT;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnBnClickedRadioAnchorCoordinates() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR_COORDINATES ) ) {
		if ( constraint ) {
			constraint->anchor.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnCbnSelchangeComboAnchorJoint() {
	UpdateFile();
}

void DialogAFConstraintUniversal::OnEnChangeEditAnchorX() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_X ) ) ) {
		UpdateFile();
	}
	else {
		m_anchor_x = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_X ) );
	}
}

void DialogAFConstraintUniversal::OnEnChangeEditAnchorY() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Y ) ) ) {
		UpdateFile();
	}
	else {
		m_anchor_y = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Y ) );
	}
}

void DialogAFConstraintUniversal::OnEnChangeEditAnchorZ() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Z ) ) ) {
		UpdateFile();
	}
	else {
		m_anchor_z = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Z ) );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinAnchorX(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_anchor_x += 1.0f;
	}
	else {
		m_anchor_x -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnDeltaposSpinAnchorY(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_anchor_y += 1.0f;
	}
	else {
		m_anchor_y -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnDeltaposSpinAnchorZ(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_anchor_z += 1.0f;
	}
	else {
		m_anchor_z -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnBnClickedRadioUniversalBoneShaft1() {
	if ( IsDlgButtonChecked( IDC_RADIO_UNIVERSAL_BONE_SHAFT1 ) ) {
		if ( constraint ) {
			constraint->shaft[0].type = idAFVector::VEC_BONEDIR;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnBnClickedRadioUniversalAnglesShaft1() {
	if ( IsDlgButtonChecked( IDC_RADIO_UNIVERSAL_ANGLES_SHAFT1 ) ) {
		if ( constraint ) {
			constraint->shaft[0].type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnCbnSelchangeComboUniversalJoint1Shaft1() {
	CString str;
	GetSafeComboBoxSelection( &m_comboJoint1Shaft1, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboJoint2Shaft1, str );
	UpdateFile();
}

void DialogAFConstraintUniversal::OnCbnSelchangeComboUniversalJoint2Shaft1() {
	CString str;
	GetSafeComboBoxSelection( &m_comboJoint2Shaft1, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboJoint1Shaft1, str );
	UpdateFile();
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalPitchShaft1() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_PITCH_SHAFT1 ) ) ) {
		UpdateFile();
	}
	else {
		m_pitchShaft1 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_PITCH_SHAFT1 ) );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalPitchShaft1(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_pitchShaft1 += 1.0f;
	}
	else {
		m_pitchShaft1 -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalYawShaft1() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_YAW_SHAFT1 ) ) ) {
		UpdateFile();
	}
	else {
		m_yawShaft1 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_YAW_SHAFT1 ) );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalYawShaft1(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_yawShaft1 += 1.0f;
	}
	else {
		m_yawShaft1 -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnBnClickedRadioUniversalBoneShaft2() {
	if ( IsDlgButtonChecked( IDC_RADIO_UNIVERSAL_BONE_SHAFT2 ) ) {
		if ( constraint ) {
			constraint->shaft[1].type = idAFVector::VEC_BONEDIR;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnBnClickedRadioUniversalAnglesShaft2() {
	if ( IsDlgButtonChecked( IDC_RADIO_UNIVERSAL_ANGLES_SHAFT2 ) ) {
		if ( constraint ) {
			constraint->shaft[1].type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnCbnSelchangeComboUniversalJoint1Shaft2() {
	CString str;
	GetSafeComboBoxSelection( &m_comboJoint1Shaft2, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboJoint2Shaft2, str );
	UpdateFile();
}

void DialogAFConstraintUniversal::OnCbnSelchangeComboUniversalJoint2Shaft2() {
	CString str;
	GetSafeComboBoxSelection( &m_comboJoint2Shaft2, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboJoint1Shaft2, str );
	UpdateFile();
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalPitchShaft2() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_PITCH_SHAFT2 ) ) ) {
		UpdateFile();
	}
	else {
		m_pitchShaft2 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_PITCH_SHAFT2 ) );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalPitchShaft2(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_pitchShaft2 += 1.0f;
	}
	else {
		m_pitchShaft2 -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalYawShaft2() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_YAW_SHAFT2 ) ) ) {
		UpdateFile();
	}
	else {
		m_yawShaft2 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_YAW_SHAFT2 ) );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalYawShaft2(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_yawShaft2 += 1.0f;
	}
	else {
		m_yawShaft2 -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnBnClickedRadioUniversalLimitNone() {
	if ( IsDlgButtonChecked( IDC_RADIO_UNIVERSAL_LIMIT_NONE ) ) {
		if ( constraint ) {
			constraint->limit = idDeclAF_Constraint::LIMIT_NONE;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnBnClickedRadioUniversalLimitCone() {
	if ( IsDlgButtonChecked( IDC_RADIO_UNIVERSAL_LIMIT_CONE ) ) {
		if ( constraint ) {
			constraint->limit = idDeclAF_Constraint::LIMIT_CONE;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnBnClickedRadioUniversalLimitPyramid() {
	if ( IsDlgButtonChecked( IDC_RADIO_UNIVERSAL_LIMIT_PYRAMID ) ) {
		if ( constraint ) {
			constraint->limit = idDeclAF_Constraint::LIMIT_PYRAMID;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalLimitConeAngle() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_CONE_ANGLE ) ) ) {
		UpdateFile();
	}
	else {
		m_coneAngle = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_CONE_ANGLE ), false );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalLimitConeAngle(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_coneAngle += 1.0f;
	}
	else if ( m_coneAngle > 0.0f ) {
		m_coneAngle -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalLimitPyramidAngle1() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE1 ) ) ) {
		UpdateFile();
	}
	else {
		m_pyramidAngle1 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE1 ), false );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalLimitPyramidAngle1(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_pyramidAngle1 += 1.0f;
	}
	else if ( m_pyramidAngle1 > 0.0f ) {
		m_pyramidAngle1 -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalLimitPyramidAngle2() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE2 ) ) ) {
		UpdateFile();
	}
	else {
		m_pyramidAngle2 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_PYRAMID_ANGLE2 ), false );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalLimitPyramidAngle2(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_pyramidAngle2 += 1.0f;
	}
	else if ( m_pyramidAngle2 > 0.0f ) {
		m_pyramidAngle2 -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalLimitRoll() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_ROLL ) ) ) {
		UpdateFile();
	}
	else {
		m_limitRoll = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_ROLL ) );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalLimitRoll(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_limitRoll += 1.0f;
	}
	else {
		m_limitRoll -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnBnClickedRadioUniversalLimitBone() {
	if ( IsDlgButtonChecked( IDC_RADIO_UNIVERSAL_LIMIT_BONE ) ) {
		if ( constraint ) {
			constraint->limitAxis.type = idAFVector::VEC_BONEDIR;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnBnClickedRadioUniversalLimitAngles() {
	if ( IsDlgButtonChecked( IDC_RADIO_UNIVERSAL_LIMIT_ANGLES ) ) {
		if ( constraint ) {
			constraint->limitAxis.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintUniversal::OnCbnSelchangeComboUniversalLimitJoint1() {
	CString str;
	GetSafeComboBoxSelection( &m_comboLimitJoint1, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboLimitJoint2, str );
	UpdateFile();
}

void DialogAFConstraintUniversal::OnCbnSelchangeComboUniversalLimitJoint2() {
	CString str;
	GetSafeComboBoxSelection( &m_comboLimitJoint2, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboLimitJoint1, str );
	UpdateFile();
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalLimitPitch() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_PITCH ) ) ) {
		UpdateFile();
	}
	else {
		m_limitPitch = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_PITCH ) );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalLimitPitch(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_limitPitch += 1.0f;
	}
	else {
		m_limitPitch -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintUniversal::OnEnChangeEditUniversalLimitYaw() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_YAW ) ) ) {
		UpdateFile();
	}
	else {
		m_limitYaw = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_UNIVERSAL_LIMIT_YAW ) );
	}
}

void DialogAFConstraintUniversal::OnDeltaposSpinUniversalLimitYaw(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_limitYaw += 1.0f;
	}
	else {
		m_limitYaw -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

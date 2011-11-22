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
#include "DialogAFConstraintBallAndSocket.h"


// DialogAFConstraintBallAndSocket dialog

toolTip_t DialogAFConstraintBallAndSocket::toolTips[] = {
	{ IDC_RADIO_ANCHOR_JOINT, "use the position of a joint for the anchor" },
	{ IDC_COMBO_ANCHOR_JOINT, "anchor joint name" },
	{ IDC_RADIO_ANCHOR_COORDINATES, "use absolute coordinates for the anchor" },
	{ IDC_EDIT_ANCHOR_X, "anchor x-coordinate" },
	{ IDC_EDIT_ANCHOR_Y, "anchor y-coordinate" },
	{ IDC_EDIT_ANCHOR_Z, "anchor z-coordinate" },
	{ IDC_RADIO_BAS_LIMIT_NONE, "no joint limit" },
	{ IDC_RADIO_BAS_LIMIT_CONE, "cone shaped joint limit" },
	{ IDC_RADIO_BAS_LIMIT_PYRAMID, "pyramid shaped joint limit" },
	{ IDC_EDIT_BAS_LIMIT_CONE_ANGLE, "cone angle" },
	{ IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE1, "first pyramid angle" },
	{ IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE2, "second pyramid angle" },
	{ IDC_EDIT_BAS_LIMIT_ROLL, "roll angle" },
	{ IDC_RADIO_BAS_LIMIT_BONE, "use a bone for the orientation of the limit" },
	{ IDC_RADIO_BAS_LIMIT_ANGLES, "use angles to set the orientation of the limit" },
	{ IDC_COMBO_BAS_LIMIT_JOINT1, "bone start joint" },
	{ IDC_COMBO_BAS_LIMIT_JOINT2, "bone end joint" },
	{ IDC_EDIT_BAS_LIMIT_PITCH, "pitch angle" },
	{ IDC_EDIT_BAS_LIMIT_YAW, "yaw angle" },
	{ IDC_RADIO_BAS_LIMIT_AXIS_BONE, "use a bone for the limit axis" },
	{ IDC_RADIO_BAS_LIMIT_AXIS_ANGLES, "use angles to set the orientation of the limit axis" },
	{ IDC_COMBO_BAS_LIMIT_AXIS_JOINT1, "bone start joint" },
	{ IDC_COMBO_BAS_LIMIT_AXIS_JOINT2, "bone end joint" },
	{ IDC_EDIT_BAS_LIMIT_AXIS_PITCH, "pitch angle" },
	{ IDC_EDIT_BAS_LIMIT_AXIS_YAW, "yaw angle" },
	{ 0, NULL }
};

IMPLEMENT_DYNAMIC(DialogAFConstraintBallAndSocket, CDialog)

/*
================
DialogAFConstraintBallAndSocket::DialogAFConstraintBallAndSocket
================
*/
DialogAFConstraintBallAndSocket::DialogAFConstraintBallAndSocket(CWnd* pParent /*=NULL*/)
	: CDialog(DialogAFConstraintBallAndSocket::IDD, pParent)
	, m_anchor_x(0)
	, m_anchor_y(0)
	, m_anchor_z(0)
	, m_coneAngle(30.0f)
	, m_pyramidAngle1(30.0f)
	, m_pyramidAngle2(30.0f)
	, m_limitPitch(0)
	, m_limitYaw(0)
	, m_limitRoll(0)
	, m_limitAxisPitch(0)
	, m_limitAxisYaw(0)
	, constraint(NULL)
	, file(NULL)
{
	Create( IDD_DIALOG_AF_CONSTRAINT_BALLANDSOCKET, pParent );
	EnableToolTips( TRUE );
}

/*
================
DialogAFConstraintBallAndSocket::~DialogAFConstraintBallAndSocket
================
*/
DialogAFConstraintBallAndSocket::~DialogAFConstraintBallAndSocket() {
}

/*
================
DialogAFConstraintBallAndSocket::DoDataExchange
================
*/
void DialogAFConstraintBallAndSocket::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAFConstraintBallAndSocket)
	DDX_Control(pDX, IDC_COMBO_ANCHOR_JOINT, m_comboAnchorJoint);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_X, m_anchor_x);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_Y, m_anchor_y);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_Z, m_anchor_z);
	DDX_Text(pDX, IDC_EDIT_BAS_LIMIT_CONE_ANGLE, m_coneAngle);
	DDX_Text(pDX, IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE1, m_pyramidAngle1);
	DDX_Text(pDX, IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE2, m_pyramidAngle2);
	DDX_Control(pDX, IDC_COMBO_BAS_LIMIT_JOINT1, m_comboLimitJoint1);
	DDX_Control(pDX, IDC_COMBO_BAS_LIMIT_JOINT2, m_comboLimitJoint2);
	DDX_Text(pDX, IDC_EDIT_BAS_LIMIT_PITCH, m_limitPitch);
	DDX_Text(pDX, IDC_EDIT_BAS_LIMIT_YAW, m_limitYaw);
	DDX_Text(pDX, IDC_EDIT_BAS_LIMIT_ROLL, m_limitRoll);
	DDX_Control(pDX, IDC_COMBO_BAS_LIMIT_AXIS_JOINT1, m_comboLimitAxisJoint1);
	DDX_Control(pDX, IDC_COMBO_BAS_LIMIT_AXIS_JOINT2, m_comboLimitAxisJoint2);
	DDX_Text(pDX, IDC_EDIT_BAS_LIMIT_AXIS_PITCH, m_limitAxisPitch);
	DDX_Text(pDX, IDC_EDIT_BAS_LIMIT_AXIS_YAW, m_limitAxisYaw);
	//}}AFX_DATA_MAP
}

/*
================
DialogAFConstraintBallAndSocket::InitJointLists
================
*/
void DialogAFConstraintBallAndSocket::InitJointLists( void ) {
	m_comboAnchorJoint.ResetContent();
	m_comboLimitJoint1.ResetContent();
	m_comboLimitJoint2.ResetContent();
	m_comboLimitAxisJoint1.ResetContent();
	m_comboLimitAxisJoint2.ResetContent();

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
		m_comboLimitJoint1.AddString( jointName );
		m_comboLimitJoint2.AddString( jointName );
		m_comboLimitAxisJoint1.AddString( jointName );
		m_comboLimitAxisJoint2.AddString( jointName );
	}
}

/*
================
DialogAFConstraintBallAndSocket::LoadFile
================
*/
void DialogAFConstraintBallAndSocket::LoadFile( idDeclAF *af ) {
	file = af;
	constraint = NULL;
	InitJointLists();
}

/*
================
DialogAFConstraintBallAndSocket::SaveFile
================
*/
void DialogAFConstraintBallAndSocket::SaveFile( void ) {
	SaveConstraint();
}

/*
================
DialogAFConstraintBallAndSocket::LoadConstraint
================
*/
void DialogAFConstraintBallAndSocket::LoadConstraint( idDeclAF_Constraint *c ) {
	int i, s1, s2;
	idAngles angles;

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

	// limit
	if ( constraint->limit == idDeclAF_Constraint::LIMIT_CONE ) {
		i = IDC_RADIO_BAS_LIMIT_CONE;
	}
	else if ( constraint->limit == idDeclAF_Constraint::LIMIT_PYRAMID ) {
		i = IDC_RADIO_BAS_LIMIT_PYRAMID;
	}
	else {
		i = IDC_RADIO_BAS_LIMIT_NONE;
	}
	CheckRadioButton( IDC_RADIO_BAS_LIMIT_NONE, IDC_RADIO_BAS_LIMIT_PYRAMID, i );

	m_coneAngle = constraint->limitAngles[0];
	m_pyramidAngle1 = constraint->limitAngles[0];
	m_pyramidAngle2 = constraint->limitAngles[1];
	m_limitRoll = constraint->limitAngles[2];
	angles = constraint->limitAxis.ToVec3().ToAngles();
	m_limitPitch = angles.pitch;
	m_limitYaw = angles.yaw;

	if ( constraint->limitAxis.type == idAFVector::VEC_BONEDIR ) {
		i = IDC_RADIO_BAS_LIMIT_BONE;
	}
	else {
		i = IDC_RADIO_BAS_LIMIT_ANGLES;
	}
	CheckRadioButton( IDC_RADIO_BAS_LIMIT_BONE, IDC_RADIO_BAS_LIMIT_ANGLES, i );
	s1 = SetSafeComboBoxSelection( &m_comboLimitJoint1, constraint->limitAxis.joint1.c_str(), -1 );
	s2 = SetSafeComboBoxSelection( &m_comboLimitJoint2, constraint->limitAxis.joint2.c_str(), s1 );

	// limit axis
	s1 = SetSafeComboBoxSelection( &m_comboLimitAxisJoint1, constraint->shaft[0].joint1.c_str(), -1 );
	s2 = SetSafeComboBoxSelection( &m_comboLimitAxisJoint2, constraint->shaft[0].joint2.c_str(), s1 );
	angles = constraint->shaft[0].ToVec3().ToAngles();
	m_limitAxisPitch = angles.pitch;
	m_limitAxisYaw = angles.yaw;
	if ( constraint->shaft[0].type == idAFVector::VEC_BONEDIR ) {
		i = IDC_RADIO_BAS_LIMIT_AXIS_BONE;
	}
	else {
		i = IDC_RADIO_BAS_LIMIT_AXIS_ANGLES;
		constraint->shaft[0].type = idAFVector::VEC_COORDS;
	}
	CheckRadioButton( IDC_RADIO_BAS_LIMIT_AXIS_BONE, IDC_RADIO_BAS_LIMIT_AXIS_ANGLES, i );

	// update displayed values
	UpdateData( FALSE );
}

/*
================
DialogAFConstraintBallAndSocket::SaveConstraint
================
*/
void DialogAFConstraintBallAndSocket::SaveConstraint( void ) {
	int s1, s2;
	CString str;
	idAngles angles;

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

	// limit axis
	if ( constraint->shaft[0].type == idAFVector::VEC_BONEDIR ) {
		s1 = GetSafeComboBoxSelection( &m_comboLimitAxisJoint1, str, -1 );
		constraint->shaft[0].joint1 = str;
		s2 = GetSafeComboBoxSelection( &m_comboLimitAxisJoint2, str, s1 );
		constraint->shaft[0].joint2 = str;
	}
	else {
		constraint->shaft[0].ToVec3() = idAngles( m_limitAxisPitch, m_limitAxisYaw, 0.0f ).ToForward();
	}

	AFDialogSetFileModified();
}

/*
================
DialogAFConstraintBallAndSocket::UpdateFile
================
*/
void DialogAFConstraintBallAndSocket::UpdateFile( void ) {
	SaveConstraint();
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAFConstraintBallAndSocket::OnToolHitTest
================
*/
int DialogAFConstraintBallAndSocket::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CDialog::OnToolHitTest( point, pTI );
	return DefaultOnToolHitTest( toolTips, this, point, pTI );
}


BEGIN_MESSAGE_MAP(DialogAFConstraintBallAndSocket, CDialog)
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
	ON_BN_CLICKED(IDC_RADIO_BAS_LIMIT_NONE, OnBnClickedRadioBasLimitNone)
	ON_BN_CLICKED(IDC_RADIO_BAS_LIMIT_CONE, OnBnClickedRadioBasLimitCone)
	ON_BN_CLICKED(IDC_RADIO_BAS_LIMIT_PYRAMID, OnBnClickedRadioBasLimitPyramid)
	ON_EN_CHANGE(IDC_EDIT_BAS_LIMIT_CONE_ANGLE, OnEnChangeEditBasLimitConeAngle)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BAS_LIMIT_CONE_ANGLE, OnDeltaposSpinBasLimitConeAngle)
	ON_EN_CHANGE(IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE1, OnEnChangeEditBasLimitPyramidAngle1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BAS_LIMIT_PYRAMID_ANGLE1, OnDeltaposSpinBasLimitPyramidAngle1)
	ON_EN_CHANGE(IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE2, OnEnChangeEditBasLimitPyramidAngle2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BAS_LIMIT_PYRAMID_ANGLE2, OnDeltaposSpinBasLimitPyramidAngle2)
	ON_EN_CHANGE(IDC_EDIT_BAS_LIMIT_ROLL, OnEnChangeEditBasLimitRoll)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BAS_LIMIT_ROLL, OnDeltaposSpinBasLimitRoll)
	ON_BN_CLICKED(IDC_RADIO_BAS_LIMIT_BONE, OnBnClickedRadioBasLimitBone)
	ON_BN_CLICKED(IDC_RADIO_BAS_LIMIT_ANGLES, OnBnClickedRadioBasLimitAngles)
	ON_CBN_SELCHANGE(IDC_COMBO_BAS_LIMIT_JOINT1, OnCbnSelchangeComboBasLimitJoint1)
	ON_CBN_SELCHANGE(IDC_COMBO_BAS_LIMIT_JOINT2, OnCbnSelchangeComboBasLimitJoint2)
	ON_EN_CHANGE(IDC_EDIT_BAS_LIMIT_PITCH, OnEnChangeEditBasLimitPitch)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BAS_LIMIT_PITCH, OnDeltaposSpinBasLimitPitch)
	ON_EN_CHANGE(IDC_EDIT_BAS_LIMIT_YAW, OnEnChangeEditBasLimitYaw)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BAS_LIMIT_YAW, OnDeltaposSpinBasLimitYaw)
	ON_BN_CLICKED(IDC_RADIO_BAS_LIMIT_AXIS_BONE, OnBnClickedRadioBasLimitAxisBone)
	ON_BN_CLICKED(IDC_RADIO_BAS_LIMIT_AXIS_ANGLES, OnBnClickedRadioBasLimitAxisAngles)
	ON_CBN_SELCHANGE(IDC_COMBO_BAS_LIMIT_AXIS_JOINT1, OnCbnSelchangeComboBasLimitAxisJoint1)
	ON_CBN_SELCHANGE(IDC_COMBO_BAS_LIMIT_AXIS_JOINT2, OnCbnSelchangeComboBasLimitAxisJoint2)
	ON_EN_CHANGE(IDC_EDIT_BAS_LIMIT_AXIS_PITCH, OnEnChangeEditBasLimitAxisPitch)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BAS_LIMIT_AXIS_PITCH, OnDeltaposSpinBasLimitAxisPitch)
	ON_EN_CHANGE(IDC_EDIT_BAS_LIMIT_AXIS_YAW, OnEnChangeEditBasLimitAxisYaw)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BAS_LIMIT_AXIS_YAW, OnDeltaposSpinBasLimitAxisYaw)
END_MESSAGE_MAP()


// DialogAFConstraintBallAndSocket message handlers

BOOL DialogAFConstraintBallAndSocket::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

void DialogAFConstraintBallAndSocket::OnBnClickedRadioAnchorJoint() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR_JOINT ) ) {
		if ( constraint ) {
			constraint->anchor.type = idAFVector::VEC_JOINT;
			UpdateFile();
		}
	}
}

void DialogAFConstraintBallAndSocket::OnBnClickedRadioAnchorCoordinates() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR_COORDINATES ) ) {
		if ( constraint ) {
			constraint->anchor.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintBallAndSocket::OnCbnSelchangeComboAnchorJoint() {
	UpdateFile();
}

void DialogAFConstraintBallAndSocket::OnEnChangeEditAnchorX() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_X ) ) ) {
		UpdateFile();
	}
	else {
		m_anchor_x = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_X ) );
	}
}

void DialogAFConstraintBallAndSocket::OnEnChangeEditAnchorY() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Y ) ) ) {
		UpdateFile();
	}
	else {
		m_anchor_y = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Y ) );
	}
}

void DialogAFConstraintBallAndSocket::OnEnChangeEditAnchorZ() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Z ) ) ) {
		UpdateFile();
	}
	else {
		m_anchor_z = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Z ) );
	}
}

void DialogAFConstraintBallAndSocket::OnDeltaposSpinAnchorX(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintBallAndSocket::OnDeltaposSpinAnchorY(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintBallAndSocket::OnDeltaposSpinAnchorZ(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintBallAndSocket::OnBnClickedRadioBasLimitNone() {
	if ( IsDlgButtonChecked( IDC_RADIO_BAS_LIMIT_NONE ) ) {
		if ( constraint ) {
			constraint->limit = idDeclAF_Constraint::LIMIT_NONE;
			UpdateFile();
		}
	}
}

void DialogAFConstraintBallAndSocket::OnBnClickedRadioBasLimitCone() {
	if ( IsDlgButtonChecked( IDC_RADIO_BAS_LIMIT_CONE ) ) {
		if ( constraint ) {
			constraint->limit = idDeclAF_Constraint::LIMIT_CONE;
			UpdateFile();
		}
	}
}

void DialogAFConstraintBallAndSocket::OnBnClickedRadioBasLimitPyramid() {
	if ( IsDlgButtonChecked( IDC_RADIO_BAS_LIMIT_PYRAMID ) ) {
		if ( constraint ) {
			constraint->limit = idDeclAF_Constraint::LIMIT_PYRAMID;
			UpdateFile();
		}
	}
}

void DialogAFConstraintBallAndSocket::OnEnChangeEditBasLimitConeAngle() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_CONE_ANGLE ) ) ) {
		UpdateFile();
	}
	else {
		m_coneAngle = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_CONE_ANGLE ), false );
	}
}

void DialogAFConstraintBallAndSocket::OnDeltaposSpinBasLimitConeAngle(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintBallAndSocket::OnEnChangeEditBasLimitPyramidAngle1() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE1 ) ) ) {
		UpdateFile();
	}
	else {
		m_pyramidAngle1 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE1 ), false );
	}
}

void DialogAFConstraintBallAndSocket::OnDeltaposSpinBasLimitPyramidAngle1(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintBallAndSocket::OnEnChangeEditBasLimitPyramidAngle2() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE2 ) ) ) {
		UpdateFile();
	}
	else {
		m_pyramidAngle2 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_PYRAMID_ANGLE2 ), false );
	}
}

void DialogAFConstraintBallAndSocket::OnDeltaposSpinBasLimitPyramidAngle2(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintBallAndSocket::OnEnChangeEditBasLimitRoll() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_ROLL ) ) ) {
		UpdateFile();
	}
	else {
		m_limitRoll = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_ROLL ) );
	}
}

void DialogAFConstraintBallAndSocket::OnDeltaposSpinBasLimitRoll(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintBallAndSocket::OnBnClickedRadioBasLimitBone() {
	if ( IsDlgButtonChecked( IDC_RADIO_BAS_LIMIT_BONE ) ) {
		if ( constraint ) {
			constraint->limitAxis.type = idAFVector::VEC_BONEDIR;
			UpdateFile();
		}
	}
}

void DialogAFConstraintBallAndSocket::OnBnClickedRadioBasLimitAngles() {
	if ( IsDlgButtonChecked( IDC_RADIO_BAS_LIMIT_ANGLES ) ) {
		if ( constraint ) {
			constraint->limitAxis.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintBallAndSocket::OnCbnSelchangeComboBasLimitJoint1() {
	CString str;
	GetSafeComboBoxSelection( &m_comboLimitJoint1, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboLimitJoint2, str );
	UpdateFile();
}

void DialogAFConstraintBallAndSocket::OnCbnSelchangeComboBasLimitJoint2() {
	CString str;
	GetSafeComboBoxSelection( &m_comboLimitJoint2, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboLimitJoint1, str );
	UpdateFile();
}

void DialogAFConstraintBallAndSocket::OnEnChangeEditBasLimitPitch() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_PITCH ) ) ) {
		UpdateFile();
	}
	else {
		m_limitPitch = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_PITCH ) );
	}
}

void DialogAFConstraintBallAndSocket::OnDeltaposSpinBasLimitPitch(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintBallAndSocket::OnEnChangeEditBasLimitYaw() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_YAW ) ) ) {
		UpdateFile();
	}
	else {
		m_limitYaw = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_YAW ) );
	}
}

void DialogAFConstraintBallAndSocket::OnDeltaposSpinBasLimitYaw(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintBallAndSocket::OnBnClickedRadioBasLimitAxisBone() {
	if ( IsDlgButtonChecked( IDC_RADIO_BAS_LIMIT_AXIS_BONE ) ) {
		if ( constraint ) {
			constraint->shaft[0].type = idAFVector::VEC_BONEDIR;
			UpdateFile();
		}
	}
}

void DialogAFConstraintBallAndSocket::OnBnClickedRadioBasLimitAxisAngles() {
	if ( IsDlgButtonChecked( IDC_RADIO_BAS_LIMIT_AXIS_ANGLES ) ) {
		if ( constraint ) {
			constraint->shaft[0].type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintBallAndSocket::OnCbnSelchangeComboBasLimitAxisJoint1() {
	CString str;
	GetSafeComboBoxSelection( &m_comboLimitAxisJoint1, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboLimitAxisJoint2, str );
	UpdateFile();
}

void DialogAFConstraintBallAndSocket::OnCbnSelchangeComboBasLimitAxisJoint2() {
	CString str;
	GetSafeComboBoxSelection( &m_comboLimitAxisJoint2, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboLimitAxisJoint1, str );
	UpdateFile();
}

void DialogAFConstraintBallAndSocket::OnEnChangeEditBasLimitAxisPitch() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_AXIS_PITCH ) ) ) {
		UpdateFile();
	}
	else {
		m_limitAxisPitch = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_AXIS_PITCH ) );
	}
}

void DialogAFConstraintBallAndSocket::OnDeltaposSpinBasLimitAxisPitch(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_limitAxisPitch += 1.0f;
	}
	else {
		m_limitAxisPitch -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintBallAndSocket::OnEnChangeEditBasLimitAxisYaw() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_AXIS_YAW ) ) ) {
		UpdateFile();
	}
	else {
		m_limitAxisYaw = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_BAS_LIMIT_AXIS_YAW ) );
	}
}

void DialogAFConstraintBallAndSocket::OnDeltaposSpinBasLimitAxisYaw(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_limitAxisYaw += 1.0f;
	}
	else {
		m_limitAxisYaw -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

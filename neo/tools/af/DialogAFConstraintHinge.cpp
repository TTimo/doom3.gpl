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
#include "DialogAFConstraintHinge.h"


// DialogAFConstraintHinge dialog

toolTip_t DialogAFConstraintHinge::toolTips[] = {
	{ IDC_RADIO_ANCHOR_JOINT, "use the position of a joint for the anchor" },
	{ IDC_COMBO_ANCHOR_JOINT, "anchor joint name" },
	{ IDC_RADIO_ANCHOR_COORDINATES, "use absolute coordinates for the anchor" },
	{ IDC_EDIT_ANCHOR_X, "anchor x-coordinate" },
	{ IDC_EDIT_ANCHOR_Y, "anchor y-coordinate" },
	{ IDC_EDIT_ANCHOR_Z, "anchor z-coordinate" },
	{ IDC_RADIO_HINGE_AXIS_BONE, "use a bone for the hinge axis" },
	{ IDC_RADIO_HINGE_AXIS_ANGLES, "use angles to set the orientation of the hinge axis" },
	{ IDC_COMBO_HINGE_AXIS_JOINT1, "bone start joint" },
	{ IDC_COMBO_HINGE_AXIS_JOINT2, "bone end joint" },
	{ IDC_EDIT_HINGE_AXIS_PITCH, "pitch angle" },
	{ IDC_EDIT_HINGE_AXIS_YAW, "yaw angle" },
	{ IDC_RADIO_HINGE_LIMIT_NONE, "no limit" },
	{ IDC_RADIO_HINGE_LIMIT_ANGLES, "angle limit" },
	{ IDC_EDIT_HINGE_LIMIT_ANGLE1, "limit orientation" },
	{ IDC_EDIT_HINGE_LIMIT_ANGLE2, "limit width" },
	{ IDC_EDIT_HINGE_LIMIT_ANGLE3, "limit angle" },
	{ 0, NULL }
};

IMPLEMENT_DYNAMIC(DialogAFConstraintHinge, CDialog)

/*
================
DialogAFConstraintHinge::DialogAFConstraintHinge
================
*/
DialogAFConstraintHinge::DialogAFConstraintHinge(CWnd* pParent /*=NULL*/)
	: CDialog(DialogAFConstraintHinge::IDD, pParent)
	, m_anchor_x(0)
	, m_anchor_y(0)
	, m_anchor_z(0)
	, m_axisPitch(0)
	, m_axisYaw(0)
	, m_limitAngle1(0)
	, m_limitAngle2(30.0f)
	, m_limitAngle3(0)
	, constraint(NULL)
	, file(NULL)
{
	Create( IDD_DIALOG_AF_CONSTRAINT_HINGE, pParent );
	EnableToolTips( TRUE );
}

/*
================
DialogAFConstraintHinge::~DialogAFConstraintHinge
================
*/
DialogAFConstraintHinge::~DialogAFConstraintHinge() {
}

/*
================
DialogAFConstraintHinge::DoDataExchange
================
*/
void DialogAFConstraintHinge::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAFConstraintHinge)
	DDX_Control(pDX, IDC_COMBO_ANCHOR_JOINT, m_comboAnchorJoint);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_X, m_anchor_x);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_Y, m_anchor_y);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_Z, m_anchor_z);
	DDX_Control(pDX, IDC_COMBO_HINGE_AXIS_JOINT1, m_comboAxisJoint1);
	DDX_Control(pDX, IDC_COMBO_HINGE_AXIS_JOINT2, m_comboAxisJoint2);
	DDX_Text(pDX, IDC_EDIT_HINGE_AXIS_PITCH, m_axisPitch);
	DDX_Text(pDX, IDC_EDIT_HINGE_AXIS_YAW, m_axisYaw);
	DDX_Text(pDX, IDC_EDIT_HINGE_LIMIT_ANGLE1, m_limitAngle1);
	DDX_Text(pDX, IDC_EDIT_HINGE_LIMIT_ANGLE2, m_limitAngle2);
	DDX_Text(pDX, IDC_EDIT_HINGE_LIMIT_ANGLE3, m_limitAngle3);
	//}}AFX_DATA_MAP
}

/*
================
DialogAFConstraintHinge::InitJointLists
================
*/
void DialogAFConstraintHinge::InitJointLists( void ) {
	m_comboAnchorJoint.ResetContent();
	m_comboAxisJoint1.ResetContent();
	m_comboAxisJoint2.ResetContent();

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
		m_comboAxisJoint1.AddString( jointName );
		m_comboAxisJoint2.AddString( jointName );
	}
}

/*
================
DialogAFConstraintHinge::LoadFile
================
*/
void DialogAFConstraintHinge::LoadFile( idDeclAF *af ) {
	file = af;
	constraint = NULL;
	InitJointLists();
}

/*
================
DialogAFConstraintHinge::SaveFile
================
*/
void DialogAFConstraintHinge::SaveFile( void ) {
	SaveConstraint();
}

/*
================
DialogAFConstraintHinge::LoadConstraint
================
*/
void DialogAFConstraintHinge::LoadConstraint( idDeclAF_Constraint *c ) {
	int i, s1, s2;
	idAngles angles;

	constraint = c;

	// load anchor from the current idDeclAF_Constraint
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

	// hinge axis
	s1 = SetSafeComboBoxSelection( &m_comboAxisJoint1, constraint->axis.joint1.c_str(), -1 );
	s2 = SetSafeComboBoxSelection( &m_comboAxisJoint2, constraint->axis.joint2.c_str(), s1 );
	angles = constraint->axis.ToVec3().ToAngles();
	m_axisPitch = angles.pitch;
	m_axisYaw = angles.yaw;
	if ( constraint->axis.type == idAFVector::VEC_BONEDIR ) {
		i = IDC_RADIO_HINGE_AXIS_BONE;
	}
	else {
		i = IDC_RADIO_HINGE_AXIS_ANGLES;
		constraint->axis.type = idAFVector::VEC_COORDS;
	}
	CheckRadioButton( IDC_RADIO_HINGE_AXIS_BONE, IDC_RADIO_HINGE_AXIS_ANGLES, i );

	// hinge limit
	if ( constraint->limit == idDeclAF_Constraint::LIMIT_CONE ) {
		i = IDC_RADIO_HINGE_LIMIT_ANGLES;
	}
	else {
		i = IDC_RADIO_HINGE_LIMIT_NONE;
	}
	CheckRadioButton( IDC_RADIO_HINGE_LIMIT_NONE, IDC_RADIO_HINGE_LIMIT_ANGLES, i );
	m_limitAngle1 = constraint->limitAngles[0];
	m_limitAngle2 = constraint->limitAngles[1];
	m_limitAngle3 = constraint->limitAngles[2];

	// update displayed values
	UpdateData( FALSE );
}

/*
================
DialogAFConstraintHinge::SaveConstraint
================
*/
void DialogAFConstraintHinge::SaveConstraint( void ) {
	int s1, s2;
	CString str;

	if ( !file || !constraint ) {
		return;
	}
	UpdateData( TRUE );

	// save anchor to the current idDeclAF_Constraint
	GetSafeComboBoxSelection( &m_comboAnchorJoint, str, -1 );
	constraint->anchor.joint1 = str;
	constraint->anchor.ToVec3().x = m_anchor_x;
	constraint->anchor.ToVec3().y = m_anchor_y;
	constraint->anchor.ToVec3().z = m_anchor_z;

	// hinge axis
	if ( constraint->axis.type == idAFVector::VEC_BONEDIR ) {
		s1 = GetSafeComboBoxSelection( &m_comboAxisJoint1, str, -1 );
		constraint->axis.joint1 = str;
		s2 = GetSafeComboBoxSelection( &m_comboAxisJoint2, str, s1 );
		constraint->axis.joint2 = str;
	}
	else {
		constraint->axis.ToVec3() = idAngles( m_axisPitch, m_axisYaw, 0.0f ).ToForward();
	}

	// hinge limit
	constraint->limitAngles[0] = m_limitAngle1;
	constraint->limitAngles[1] = m_limitAngle2;
	constraint->limitAngles[2] = m_limitAngle3;

	AFDialogSetFileModified();
}

/*
================
DialogAFConstraintHinge::UpdateFile
================
*/
void DialogAFConstraintHinge::UpdateFile( void ) {
	SaveConstraint();
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAFConstraintHinge::OnToolHitTest
================
*/
int DialogAFConstraintHinge::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CDialog::OnToolHitTest( point, pTI );
	return DefaultOnToolHitTest( toolTips, this, point, pTI );
}


BEGIN_MESSAGE_MAP(DialogAFConstraintHinge, CDialog)
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
	ON_BN_CLICKED(IDC_RADIO_HINGE_AXIS_BONE, OnBnClickedRadioHingeAxisBone)
	ON_BN_CLICKED(IDC_RADIO_HINGE_AXIS_ANGLES, OnBnClickedRadioHingeAxisAngles)
	ON_CBN_SELCHANGE(IDC_COMBO_HINGE_AXIS_JOINT1, OnCbnSelchangeComboHingeAxisJoint1)
	ON_CBN_SELCHANGE(IDC_COMBO_HINGE_AXIS_JOINT2, OnCbnSelchangeComboHingeAxisJoint2)
	ON_EN_CHANGE(IDC_EDIT_HINGE_AXIS_PITCH, OnEnChangeEditHingeAxisPitch)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_HINGE_AXIS_PITCH, OnDeltaposSpinHingeAxisPitch)
	ON_EN_CHANGE(IDC_EDIT_HINGE_AXIS_YAW, OnEnChangeEditHingeAxisYaw)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_HINGE_AXIS_YAW, OnDeltaposSpinHingeAxisYaw)
	ON_BN_CLICKED(IDC_RADIO_HINGE_LIMIT_NONE, OnBnClickedRadioHingeLimitNone)
	ON_BN_CLICKED(IDC_RADIO_HINGE_LIMIT_ANGLES, OnBnClickedRadioHingeLimitAngles)
	ON_EN_CHANGE(IDC_EDIT_HINGE_LIMIT_ANGLE1, OnEnChangeEditHingeLimitAngle1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_HINGE_LIMIT_ANGLE1, OnDeltaposSpinHingeLimitAngle1)
	ON_EN_CHANGE(IDC_EDIT_HINGE_LIMIT_ANGLE2, OnEnChangeEditHingeLimitAngle2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_HINGE_LIMIT_ANGLE2, OnDeltaposSpinHingeLimitAngle2)
	ON_EN_CHANGE(IDC_EDIT_HINGE_LIMIT_ANGLE3, OnEnChangeEditHingeLimitAngle3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_HINGE_LIMIT_ANGLE3, OnDeltaposSpinHingeLimitAngle3)
END_MESSAGE_MAP()


// DialogAFConstraintHinge message handlers

BOOL DialogAFConstraintHinge::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

void DialogAFConstraintHinge::OnBnClickedRadioAnchorJoint() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR_JOINT ) ) {
		if ( constraint ) {
			constraint->anchor.type = idAFVector::VEC_JOINT;
			UpdateFile();
		}
	}
}

void DialogAFConstraintHinge::OnBnClickedRadioAnchorCoordinates() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR_COORDINATES ) ) {
		if ( constraint ) {
			constraint->anchor.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintHinge::OnCbnSelchangeComboAnchorJoint() {
	UpdateFile();
}

void DialogAFConstraintHinge::OnEnChangeEditAnchorX() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_X ) ) ) {
		UpdateFile();
	}
	else {
		m_anchor_x = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_X ) );
	}
}

void DialogAFConstraintHinge::OnEnChangeEditAnchorY() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Y ) ) ) {
		UpdateFile();
	}
	else {
		m_anchor_y = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Y ) );
	}
}

void DialogAFConstraintHinge::OnEnChangeEditAnchorZ() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Z ) ) ) {
		UpdateFile();
	}
	else {
		m_anchor_z = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Z ) );
	}
}

void DialogAFConstraintHinge::OnDeltaposSpinAnchorX(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintHinge::OnDeltaposSpinAnchorY(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintHinge::OnDeltaposSpinAnchorZ(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintHinge::OnBnClickedRadioHingeAxisBone() {
	if ( IsDlgButtonChecked( IDC_RADIO_HINGE_AXIS_BONE ) ) {
		if ( constraint ) {
			constraint->axis.type = idAFVector::VEC_BONEDIR;
			UpdateFile();
		}
	}
}

void DialogAFConstraintHinge::OnBnClickedRadioHingeAxisAngles() {
	if ( IsDlgButtonChecked( IDC_RADIO_HINGE_AXIS_ANGLES ) ) {
		if ( constraint ) {
			constraint->axis.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintHinge::OnCbnSelchangeComboHingeAxisJoint1() {
	CString str;
	GetSafeComboBoxSelection( &m_comboAxisJoint1, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboAxisJoint2, str );
	UpdateFile();
}

void DialogAFConstraintHinge::OnCbnSelchangeComboHingeAxisJoint2() {
	CString str;
	GetSafeComboBoxSelection( &m_comboAxisJoint2, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboAxisJoint1, str );
	UpdateFile();
}

void DialogAFConstraintHinge::OnEnChangeEditHingeAxisPitch() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_AXIS_PITCH ) ) ) {
		UpdateFile();
	}
	else {
		m_axisPitch = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_AXIS_PITCH ) );
	}
}

void DialogAFConstraintHinge::OnDeltaposSpinHingeAxisPitch(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_axisPitch += 1.0f;
	}
	else {
		m_axisPitch -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintHinge::OnEnChangeEditHingeAxisYaw() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_AXIS_YAW ) ) ) {
		UpdateFile();
	}
	else {
		m_axisYaw = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_AXIS_YAW ) );
	}
}

void DialogAFConstraintHinge::OnDeltaposSpinHingeAxisYaw(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_axisYaw += 1.0f;
	}
	else {
		m_axisYaw -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintHinge::OnBnClickedRadioHingeLimitNone() {
	if ( IsDlgButtonChecked( IDC_RADIO_HINGE_LIMIT_NONE ) ) {
		if ( constraint ) {
			constraint->limit = idDeclAF_Constraint::LIMIT_NONE;
			UpdateFile();
		}
	}
}

void DialogAFConstraintHinge::OnBnClickedRadioHingeLimitAngles() {
	if ( IsDlgButtonChecked( IDC_RADIO_HINGE_LIMIT_ANGLES ) ) {
		if ( constraint ) {
			constraint->limit = idDeclAF_Constraint::LIMIT_CONE;
			UpdateFile();
		}
	}
}

void DialogAFConstraintHinge::OnEnChangeEditHingeLimitAngle1() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_LIMIT_ANGLE1 ) ) ) {
		UpdateFile();
	}
	else {
		m_limitAngle1 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_LIMIT_ANGLE1 ) );
	}
}

void DialogAFConstraintHinge::OnDeltaposSpinHingeLimitAngle1(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_limitAngle1 += 1.0f;
	}
	else {
		m_limitAngle1 -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintHinge::OnEnChangeEditHingeLimitAngle2() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_LIMIT_ANGLE2 ) ) ) {
		UpdateFile();
	}
	else {
		m_limitAngle2 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_LIMIT_ANGLE2 ), false );
	}
}

void DialogAFConstraintHinge::OnDeltaposSpinHingeLimitAngle2(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_limitAngle2 += 1.0f;
	}
	else if ( m_limitAngle2 > 0.0f ) {
		m_limitAngle2 -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintHinge::OnEnChangeEditHingeLimitAngle3() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_LIMIT_ANGLE3 ) ) ) {
		UpdateFile();
	}
	else {
		m_limitAngle3 = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_HINGE_LIMIT_ANGLE3 ) );
	}
}

void DialogAFConstraintHinge::OnDeltaposSpinHingeLimitAngle3(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_limitAngle3 += 1.0f;
	}
	else {
		m_limitAngle3 -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

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
#include "DialogAFConstraintSlider.h"


// DialogAFConstraintSlider dialog

toolTip_t DialogAFConstraintSlider::toolTips[] = {
	{ IDC_RADIO_SLIDER_AXIS_BONE, "use a bone for the slider axis" },
	{ IDC_RADIO_SLIDER_AXIS_ANGLES, "use angles to set the orientation of the slider axis" },
	{ IDC_COMBO_SLIDER_AXIS_JOINT1, "bone start joint" },
	{ IDC_COMBO_SLIDER_AXIS_JOINT2, "bone end joint" },
	{ IDC_EDIT_SLIDER_AXIS_PITCH, "pitch angle" },
	{ IDC_EDIT_SLIDER_AXIS_YAW, "yaw angle" },
	{ 0, NULL }
};

IMPLEMENT_DYNAMIC(DialogAFConstraintSlider, CDialog)

/*
================
DialogAFConstraintSlider::DialogAFConstraintSlider
================
*/
DialogAFConstraintSlider::DialogAFConstraintSlider(CWnd* pParent /*=NULL*/)
	: CDialog(DialogAFConstraintSlider::IDD, pParent)
	, m_axisPitch(0)
	, m_axisYaw(0)
	, constraint(NULL)
	, file(NULL)
{
	Create( IDD_DIALOG_AF_CONSTRAINT_SLIDER, pParent );
	EnableToolTips( TRUE );
}

/*
================
DialogAFConstraintSlider::~DialogAFConstraintSlider
================
*/
DialogAFConstraintSlider::~DialogAFConstraintSlider() {
}

/*
================
DialogAFConstraintSlider::DoDataExchange
================
*/
void DialogAFConstraintSlider::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAFConstraintSlider)
	DDX_Control(pDX, IDC_COMBO_SLIDER_AXIS_JOINT1, m_comboAxisJoint1);
	DDX_Control(pDX, IDC_COMBO_SLIDER_AXIS_JOINT2, m_comboAxisJoint2);
	DDX_Text(pDX, IDC_EDIT_SLIDER_AXIS_PITCH, m_axisPitch);
	DDX_Text(pDX, IDC_EDIT_SLIDER_AXIS_YAW, m_axisYaw);
	//}}AFX_DATA_MAP
}

/*
================
DialogAFConstraintSlider::InitJointLists
================
*/
void DialogAFConstraintSlider::InitJointLists( void ) {
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
		m_comboAxisJoint1.AddString( jointName );
		m_comboAxisJoint2.AddString( jointName );
	}
}

/*
================
DialogAFConstraintSlider::LoadFile
================
*/
void DialogAFConstraintSlider::LoadFile( idDeclAF *af ) {
	file = af;
	constraint = NULL;
	InitJointLists();
}

/*
================
DialogAFConstraintSlider::SaveFile
================
*/
void DialogAFConstraintSlider::SaveFile( void ) {
	SaveConstraint();
}

/*
================
DialogAFConstraintSlider::LoadConstraint
================
*/
void DialogAFConstraintSlider::LoadConstraint( idDeclAF_Constraint *c ) {
	int i, s1, s2;
	idAngles angles;

	constraint = c;

	// slider axis
	s1 = SetSafeComboBoxSelection( &m_comboAxisJoint1, constraint->axis.joint1.c_str(), -1 );
	s2 = SetSafeComboBoxSelection( &m_comboAxisJoint2, constraint->axis.joint2.c_str(), s1 );
	angles = constraint->axis.ToVec3().ToAngles();
	m_axisPitch = angles.pitch;
	m_axisYaw = angles.yaw;
	if ( constraint->axis.type == idAFVector::VEC_BONEDIR ) {
		i = IDC_RADIO_SLIDER_AXIS_BONE;
	}
	else {
		i = IDC_RADIO_SLIDER_AXIS_ANGLES;
		constraint->axis.type = idAFVector::VEC_COORDS;
	}
	CheckRadioButton( IDC_RADIO_SLIDER_AXIS_BONE, IDC_RADIO_SLIDER_AXIS_ANGLES, i );

	// update displayed values
	UpdateData( FALSE );
}

/*
================
DialogAFConstraintSlider::SaveConstraint
================
*/
void DialogAFConstraintSlider::SaveConstraint( void ) {
	int s1, s2;
	CString str;

	if ( !file || !constraint ) {
		return;
	}
	UpdateData( TRUE );

	// slider axis
	if ( constraint->axis.type == idAFVector::VEC_BONEDIR ) {
		s1 = GetSafeComboBoxSelection( &m_comboAxisJoint1, str, -1 );
		constraint->axis.joint1 = str;
		s2 = GetSafeComboBoxSelection( &m_comboAxisJoint2, str, s1 );
		constraint->axis.joint2 = str;
	}
	else {
		constraint->axis.ToVec3() = idAngles( m_axisPitch, m_axisYaw, 0.0f ).ToForward();
	}

	AFDialogSetFileModified();
}

/*
================
DialogAFConstraintSlider::UpdateFile
================
*/
void DialogAFConstraintSlider::UpdateFile( void ) {
	SaveConstraint();
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAFConstraintSlider::OnToolHitTest
================
*/
int DialogAFConstraintSlider::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CDialog::OnToolHitTest( point, pTI );
	return DefaultOnToolHitTest( toolTips, this, point, pTI );
}


BEGIN_MESSAGE_MAP(DialogAFConstraintSlider, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_BN_CLICKED(IDC_RADIO_SLIDER_AXIS_BONE, OnBnClickedRadioSliderAxisBone)
	ON_BN_CLICKED(IDC_RADIO_SLIDER_AXIS_ANGLES, OnBnClickedRadioSliderAxisAngles)
	ON_CBN_SELCHANGE(IDC_COMBO_SLIDER_AXIS_JOINT1, OnCbnSelchangeComboSliderAxisJoint1)
	ON_CBN_SELCHANGE(IDC_COMBO_SLIDER_AXIS_JOINT2, OnCbnSelchangeComboSliderAxisJoint2)
	ON_EN_CHANGE(IDC_EDIT_SLIDER_AXIS_PITCH, OnEnChangeEditSliderAxisPitch)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SLIDER_AXIS_PITCH, OnDeltaposSpinSliderAxisPitch)
	ON_EN_CHANGE(IDC_EDIT_SLIDER_AXIS_YAW, OnEnChangeEditSliderAxisYaw)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SLIDER_AXIS_YAW, OnDeltaposSpinSliderAxisYaw)
END_MESSAGE_MAP()


// DialogAFConstraintSlider message handlers

BOOL DialogAFConstraintSlider::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

void DialogAFConstraintSlider::OnBnClickedRadioSliderAxisBone() {
	if ( IsDlgButtonChecked( IDC_RADIO_SLIDER_AXIS_BONE ) ) {
		if ( constraint ) {
			constraint->axis.type = idAFVector::VEC_BONEDIR;
			UpdateFile();
		}
	}
}

void DialogAFConstraintSlider::OnBnClickedRadioSliderAxisAngles() {
	if ( IsDlgButtonChecked( IDC_RADIO_SLIDER_AXIS_ANGLES ) ) {
		if ( constraint ) {
			constraint->axis.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintSlider::OnCbnSelchangeComboSliderAxisJoint1() {
	CString str;
	GetSafeComboBoxSelection( &m_comboAxisJoint1, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboAxisJoint2, str );
	UpdateFile();
}

void DialogAFConstraintSlider::OnCbnSelchangeComboSliderAxisJoint2() {
	CString str;
	GetSafeComboBoxSelection( &m_comboAxisJoint2, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboAxisJoint1, str );
	UpdateFile();
}

void DialogAFConstraintSlider::OnEnChangeEditSliderAxisPitch() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_SLIDER_AXIS_PITCH ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_SLIDER_AXIS_PITCH ) );
	}
}

void DialogAFConstraintSlider::OnDeltaposSpinSliderAxisPitch(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintSlider::OnEnChangeEditSliderAxisYaw() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_SLIDER_AXIS_YAW ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_SLIDER_AXIS_YAW ) );
	}
}

void DialogAFConstraintSlider::OnDeltaposSpinSliderAxisYaw(NMHDR *pNMHDR, LRESULT *pResult) {
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

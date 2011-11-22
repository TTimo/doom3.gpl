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
#include "DialogAFConstraintSpring.h"


// DialogAFConstraintSpring dialog

toolTip_t DialogAFConstraintSpring::toolTips[] = {
	{ IDC_RADIO_ANCHOR_JOINT, "use the position of a joint for the first anchor" },
	{ IDC_COMBO_ANCHOR_JOINT, "first anchor joint name" },
	{ IDC_RADIO_ANCHOR_COORDINATES, "use absolute coordinates for the first anchor" },
	{ IDC_EDIT_ANCHOR_X, "first anchor x-coordinate" },
	{ IDC_EDIT_ANCHOR_Y, "first anchor y-coordinate" },
	{ IDC_EDIT_ANCHOR_Z, "first anchor z-coordinate" },
	{ IDC_RADIO_ANCHOR2_JOINT, "use the position of a joint for the second anchor" },
	{ IDC_COMBO_ANCHOR2_JOINT, "second anchor joint name" },
	{ IDC_RADIO_ANCHOR2_COORDINATES, "use absolute coordinates for the second anchor" },
	{ IDC_EDIT_ANCHOR2_X, "second anchor x-coordinate" },
	{ IDC_EDIT_ANCHOR2_Y, "second anchor y-coordinate" },
	{ IDC_EDIT_ANCHOR2_Z, "second anchor z-coordinate" },
	{ IDC_EDIT_SPRING_STRETCH, "spring constant when stretched" },
	{ IDC_EDIT_SPRING_COMPRESS, "spring constant when compressed" },
	{ IDC_EDIT_SPRING_DAMPING, "spring damping" },
	{ IDC_EDIT_SPRING_REST_LENGTH, "rest length" },
	{ IDC_RADIO_SPRING_NO_MIN_LENGTH, "no minimum length" },
	{ IDC_RADIO_SPRING_MIN_LENGTH, "minimum length" },
	{ IDC_EDIT_SPRING_MIN_LENGTH, "minimum length" },
	{ IDC_RADIO_SPRING_NO_MAX_LENGTH, "no maximum length" },
	{ IDC_RADIO_SPRING_MAX_LENGTH, "maximum length" },
	{ IDC_EDIT_SPRING_MAX_LENGTH, "maximum length" },
	{ 0, NULL }
};

IMPLEMENT_DYNAMIC(DialogAFConstraintSpring, CDialog)

/*
================
DialogAFConstraintSpring::DialogAFConstraintSpring
================
*/
DialogAFConstraintSpring::DialogAFConstraintSpring(CWnd* pParent /*=NULL*/)
	: CDialog(DialogAFConstraintSpring::IDD, pParent)
	, m_anchor_x(0)
	, m_anchor_y(0)
	, m_anchor_z(0)
	, m_anchor2_x(0)
	, m_anchor2_y(0)
	, m_anchor2_z(0)
	, m_stretch(0)
	, m_compress(0)
	, m_damping(0)
	, m_restLength(0)
	, m_minLength(0)
	, m_maxLength(0)
	, constraint(NULL)
	, file(NULL)
{
	Create( IDD_DIALOG_AF_CONSTRAINT_SPRING, pParent );
	EnableToolTips( TRUE );
}

/*
================
DialogAFConstraintSpring::~DialogAFConstraintSpring
================
*/
DialogAFConstraintSpring::~DialogAFConstraintSpring() {
}

/*
================
DialogAFConstraintSpring::DoDataExchange
================
*/
void DialogAFConstraintSpring::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAFConstraintSpring)
	DDX_Control(pDX, IDC_COMBO_ANCHOR_JOINT, m_comboAnchorJoint);
	DDX_Control(pDX, IDC_COMBO_ANCHOR2_JOINT, m_comboAnchor2Joint);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_X, m_anchor_x);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_Y, m_anchor_y);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_Z, m_anchor_z);
	DDX_Text(pDX, IDC_EDIT_ANCHOR2_X, m_anchor2_x);
	DDX_Text(pDX, IDC_EDIT_ANCHOR2_Y, m_anchor2_y);
	DDX_Text(pDX, IDC_EDIT_ANCHOR2_Z, m_anchor2_z);
	DDX_Text(pDX, IDC_EDIT_SPRING_STRETCH, m_stretch);
	DDX_Text(pDX, IDC_EDIT_SPRING_COMPRESS, m_compress);
	DDX_Text(pDX, IDC_EDIT_SPRING_DAMPING, m_damping);
	DDX_Text(pDX, IDC_EDIT_SPRING_REST_LENGTH, m_restLength);
	DDX_Text(pDX, IDC_EDIT_SPRING_MIN_LENGTH, m_minLength);
	DDX_Text(pDX, IDC_EDIT_SPRING_MAX_LENGTH, m_maxLength);
	//}}AFX_DATA_MAP
}

/*
================
DialogAFConstraintSpring::InitJointLists
================
*/
void DialogAFConstraintSpring::InitJointLists( void ) {
	m_comboAnchorJoint.ResetContent();
	m_comboAnchor2Joint.ResetContent();

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
		m_comboAnchor2Joint.AddString( jointName );
	}
}

/*
================
DialogAFConstraintSpring::LoadFile
================
*/
void DialogAFConstraintSpring::LoadFile( idDeclAF *af ) {
	file = af;
	constraint = NULL;
	InitJointLists();
}

/*
================
DialogAFConstraintSpring::SaveFile
================
*/
void DialogAFConstraintSpring::SaveFile( void ) {
	SaveConstraint();
}

/*
================
DialogAFConstraintSpring::LoadConstraint
================
*/
void DialogAFConstraintSpring::LoadConstraint( idDeclAF_Constraint *c ) {
	int i;

	constraint = c;

	// load first anchor from the current idDeclAF_Constraint
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

	// load second anchor from the current idDeclAF_Constraint
	SetSafeComboBoxSelection( &m_comboAnchor2Joint, constraint->anchor2.joint1.c_str(), -1 );
	m_anchor2_x = constraint->anchor2.ToVec3().x;
	m_anchor2_y = constraint->anchor2.ToVec3().y;
	m_anchor2_z = constraint->anchor2.ToVec3().z;
	if ( constraint->anchor2.type == idAFVector::VEC_JOINT ) {
		i = IDC_RADIO_ANCHOR2_JOINT;
	}
	else {
		i = IDC_RADIO_ANCHOR2_COORDINATES;
	}
	CheckRadioButton( IDC_RADIO_ANCHOR2_JOINT, IDC_RADIO_ANCHOR2_COORDINATES, i );

	// spring settings
	m_stretch = constraint->stretch;
	m_compress = constraint->compress;
	m_damping = constraint->damping;
	m_restLength = constraint->restLength;

	// spring limits
	if ( constraint->minLength > 0.0f ) {
		i = IDC_RADIO_SPRING_MIN_LENGTH;
	}
	else {
		i = IDC_RADIO_SPRING_NO_MIN_LENGTH;
	}
	CheckRadioButton( IDC_RADIO_SPRING_NO_MIN_LENGTH, IDC_RADIO_SPRING_MIN_LENGTH, i );
	m_minLength = constraint->minLength;

	if ( constraint->maxLength > 0.0f ) {
		i = IDC_RADIO_SPRING_MAX_LENGTH;
	}
	else {
		i = IDC_RADIO_SPRING_NO_MAX_LENGTH;
	}
	CheckRadioButton( IDC_RADIO_SPRING_NO_MAX_LENGTH, IDC_RADIO_SPRING_MAX_LENGTH, i );
	m_maxLength = constraint->maxLength;

	// update displayed values
	UpdateData( FALSE );
}

/*
================
DialogAFConstraintSpring::SaveConstraint
================
*/
void DialogAFConstraintSpring::SaveConstraint( void ) {
	CString str;

	if ( !file || !constraint ) {
		return;
	}
	UpdateData( TRUE );

	// save first anchor to the current idDeclAF_Constraint
	GetSafeComboBoxSelection( &m_comboAnchorJoint, str, -1 );
	constraint->anchor.joint1 = str;
	constraint->anchor.ToVec3().x = m_anchor_x;
	constraint->anchor.ToVec3().y = m_anchor_y;
	constraint->anchor.ToVec3().z = m_anchor_z;

	// save second anchor to the current idDeclAF_Constraint
	GetSafeComboBoxSelection( &m_comboAnchor2Joint, str, -1 );
	constraint->anchor2.joint1 = str;
	constraint->anchor2.ToVec3().x = m_anchor2_x;
	constraint->anchor2.ToVec3().y = m_anchor2_y;
	constraint->anchor2.ToVec3().z = m_anchor2_z;

	// spring settings
	constraint->stretch = m_stretch;
	constraint->compress = m_compress;
	constraint->damping = m_damping;
	constraint->restLength = m_restLength;

	// spring limits
	constraint->minLength = m_minLength;
	constraint->maxLength = m_maxLength;

	AFDialogSetFileModified();
}

/*
================
DialogAFConstraintSpring::UpdateFile
================
*/
void DialogAFConstraintSpring::UpdateFile( void ) {
	SaveConstraint();
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAFConstraintSpring::OnToolHitTest
================
*/
int DialogAFConstraintSpring::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CDialog::OnToolHitTest( point, pTI );
	return DefaultOnToolHitTest( toolTips, this, point, pTI );
}


BEGIN_MESSAGE_MAP(DialogAFConstraintSpring, CDialog)
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
	ON_BN_CLICKED(IDC_RADIO_ANCHOR2_JOINT, OnBnClickedRadioAnchor2Joint)
	ON_BN_CLICKED(IDC_RADIO_ANCHOR2_COORDINATES, OnBnClickedRadioAnchor2Coordinates)
	ON_CBN_SELCHANGE(IDC_COMBO_ANCHOR2_JOINT, OnCbnSelchangeComboAnchor2Joint)
	ON_EN_CHANGE(IDC_EDIT_ANCHOR2_X, OnEnChangeEditAnchor2X)
	ON_EN_CHANGE(IDC_EDIT_ANCHOR2_Y, OnEnChangeEditAnchor2Y)
	ON_EN_CHANGE(IDC_EDIT_ANCHOR2_Z, OnEnChangeEditAnchor2Z)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANCHOR2_X, OnDeltaposSpinAnchor2X)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANCHOR2_Y, OnDeltaposSpinAnchor2Y)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANCHOR2_Z, OnDeltaposSpinAnchor2Z)
	ON_EN_CHANGE(IDC_EDIT_SPRING_STRETCH, OnEnChangeEditSpringStretch)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SPRING_STRETCH, OnDeltaposSpinSpringStretch)
	ON_EN_CHANGE(IDC_EDIT_SPRING_COMPRESS, OnEnChangeEditSpringCompress)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SPRING_COMPRESS, OnDeltaposSpinSpringCompress)
	ON_EN_CHANGE(IDC_EDIT_SPRING_DAMPING, OnEnChangeEditSpringDamping)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SPRING_DAMPING, OnDeltaposSpinSpringDamping)
	ON_EN_CHANGE(IDC_EDIT_SPRING_REST_LENGTH, OnEnChangeEditSpringRestLength)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SPRING_REST_LENGTH, OnDeltaposSpinSpringRestLength)
	ON_BN_CLICKED(IDC_RADIO_SPRING_NO_MIN_LENGTH, OnBnClickedRadioLimitNoMinLength)
	ON_BN_CLICKED(IDC_RADIO_SPRING_MIN_LENGTH, OnBnClickedRadioLimitMinLength)
	ON_EN_CHANGE(IDC_EDIT_SPRING_MIN_LENGTH, OnEnChangeEditLimitMinLength)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SPRING_MIN_LENGTH, OnDeltaposSpinLimitMinLength)
	ON_BN_CLICKED(IDC_RADIO_SPRING_NO_MAX_LENGTH, OnBnClickedRadioLimitNoMaxLength)
	ON_BN_CLICKED(IDC_RADIO_SPRING_MAX_LENGTH, OnBnClickedRadioLimitMaxLength)
	ON_EN_CHANGE(IDC_EDIT_SPRING_MAX_LENGTH, OnEnChangeEditLimitMaxLength)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SPRING_MAX_LENGTH, OnDeltaposSpinLimitMaxLength)
END_MESSAGE_MAP()


// DialogAFConstraintSpring message handlers

BOOL DialogAFConstraintSpring::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

void DialogAFConstraintSpring::OnBnClickedRadioAnchorJoint() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR_JOINT ) ) {
		if ( constraint ) {
			constraint->anchor.type = idAFVector::VEC_JOINT;
			UpdateFile();
		}
	}
}

void DialogAFConstraintSpring::OnBnClickedRadioAnchorCoordinates() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR_COORDINATES ) ) {
		if ( constraint ) {
			constraint->anchor.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintSpring::OnCbnSelchangeComboAnchorJoint() {
	UpdateFile();
}

void DialogAFConstraintSpring::OnEnChangeEditAnchorX() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_X ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_X ) );
	}
}

void DialogAFConstraintSpring::OnEnChangeEditAnchorY() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Y ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Y ) );
	}
}

void DialogAFConstraintSpring::OnEnChangeEditAnchorZ() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Z ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR_Z ) );
	}
}

void DialogAFConstraintSpring::OnDeltaposSpinAnchorX(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintSpring::OnDeltaposSpinAnchorY(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintSpring::OnDeltaposSpinAnchorZ(NMHDR *pNMHDR, LRESULT *pResult) {
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

void DialogAFConstraintSpring::OnBnClickedRadioAnchor2Joint() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR2_JOINT ) ) {
		if ( constraint ) {
			constraint->anchor2.type = idAFVector::VEC_JOINT;
			UpdateFile();
		}
	}
}

void DialogAFConstraintSpring::OnBnClickedRadioAnchor2Coordinates() {
	if ( IsDlgButtonChecked( IDC_RADIO_ANCHOR2_COORDINATES ) ) {
		if ( constraint ) {
			constraint->anchor2.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFConstraintSpring::OnCbnSelchangeComboAnchor2Joint() {
	UpdateFile();
}

void DialogAFConstraintSpring::OnEnChangeEditAnchor2X() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR2_X ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR2_X ) );
	}
}

void DialogAFConstraintSpring::OnEnChangeEditAnchor2Y() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR2_Y ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR2_Y ) );
	}
}

void DialogAFConstraintSpring::OnEnChangeEditAnchor2Z() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR2_Z ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANCHOR2_Z ) );
	}
}

void DialogAFConstraintSpring::OnDeltaposSpinAnchor2X(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_anchor2_x += 1.0f;
	}
	else {
		m_anchor2_x -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintSpring::OnDeltaposSpinAnchor2Y(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_anchor2_y += 1.0f;
	}
	else {
		m_anchor2_y -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintSpring::OnDeltaposSpinAnchor2Z(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_anchor2_z += 1.0f;
	}
	else {
		m_anchor2_z -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintSpring::OnEnChangeEditSpringStretch() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_STRETCH ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_STRETCH ) );
	}
}

void DialogAFConstraintSpring::OnDeltaposSpinSpringStretch(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_stretch = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_SPRING_STRETCH ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintSpring::OnEnChangeEditSpringCompress() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_COMPRESS ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_COMPRESS ) );
	}
}

void DialogAFConstraintSpring::OnDeltaposSpinSpringCompress(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_compress = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_SPRING_COMPRESS ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintSpring::OnEnChangeEditSpringDamping() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_DAMPING ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_DAMPING ) );
	}
}

void DialogAFConstraintSpring::OnDeltaposSpinSpringDamping(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_damping = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_SPRING_DAMPING ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintSpring::OnEnChangeEditSpringRestLength() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_REST_LENGTH ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_REST_LENGTH ) );
	}
}

void DialogAFConstraintSpring::OnDeltaposSpinSpringRestLength(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_restLength += 1.0f;
	}
	else {
		m_restLength -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintSpring::OnBnClickedRadioLimitNoMinLength() {
	if ( IsDlgButtonChecked( IDC_RADIO_SPRING_NO_MIN_LENGTH ) ) {
		if ( constraint ) {
			constraint->minLength = 0.0f;
			UpdateFile();
		}
	}
}

void DialogAFConstraintSpring::OnBnClickedRadioLimitMinLength() {
	// do nothing
}

void DialogAFConstraintSpring::OnEnChangeEditLimitMinLength() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_MIN_LENGTH ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_MIN_LENGTH ) );
	}
}

void DialogAFConstraintSpring::OnDeltaposSpinLimitMinLength(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_minLength += 1.0f;
	}
	else {
		m_minLength -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFConstraintSpring::OnBnClickedRadioLimitNoMaxLength() {
	if ( IsDlgButtonChecked( IDC_RADIO_SPRING_NO_MAX_LENGTH ) ) {
		if ( constraint ) {
			constraint->maxLength = 0.0f;
			UpdateFile();
		}
	}
}

void DialogAFConstraintSpring::OnBnClickedRadioLimitMaxLength() {
	// do nothing
}

void DialogAFConstraintSpring::OnEnChangeEditLimitMaxLength() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_MAX_LENGTH ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_SPRING_MAX_LENGTH ) );
	}
}

void DialogAFConstraintSpring::OnDeltaposSpinLimitMaxLength(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		m_maxLength += 1.0f;
	}
	else {
		m_maxLength -= 1.0f;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

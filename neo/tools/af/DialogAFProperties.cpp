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
#include "DialogAFProperties.h"
#include "DialogAFBody.h"
#include "DialogAFConstraint.h"


// DialogAFProperties dialog

toolTip_t DialogAFProperties::toolTips[] = {
	{ IDC_EDIT_MODEL, "model def" },
	{ IDC_BUTTON_BROWSE_MODEL, "browse model def" },
	{ IDC_EDIT_SKIN, "skin" },
	{ IDC_BUTTON_BROWSE_SKIN, "browse skin" },
	{ IDC_EDIT_LINEARFRICTION, "translational friction" },
	{ IDC_EDIT_ANGULARFRICTION, "rotational friction" },
	{ IDC_EDIT_CONTACTFRICTION, "friction with contact surfaces" },
	{ IDC_EDIT_CONSTRAINTFRICTION, "constraint friction" },
	{ IDC_CHECK_SELFCOLLISION, "allow bodies to collide with other bodies of this articulated figure" },
	{ IDC_EDIT_CONTENTS, "content of bodies" },
	{ IDC_EDIT_CLIPMASK, "collide with these content types" },
	{ IDC_EDIT_TOTALMASS, "scale the mass of each body to get this total mass" },
	{ IDC_EDIT_LINEARVELOCITY, "do not suspend simulation if the linear velocity is higher than this value" },
	{ IDC_EDIT_ANGULARVELOCITY, "do not suspend simulation if the angular velocity is higher than this value" },
	{ IDC_EDIT_LINEARACCELERATION, "do not suspend simulation if the linear acceleration is higher than this value" },
	{ IDC_EDIT_ANGULARACCELERATION, "do not suspend simulation if the angular acceleration is higher than this value" },
	{ IDC_EDIT_NO_MOVE_TIME, "suspend simulation if hardly any movement for this many seconds" },
	{ IDC_EDIT_MAXIMUM_MOVE_TIME, "always suspend simulation after running for this many seconds" },
	{ IDC_EDIT_LINEAR_TOLERANCE, "maximum translation considered no movement" },
	{ IDC_EDIT_ANGULAR_TOLERANCE, "maximum rotation considered no movement" },
	{ 0, NULL }
};

IMPLEMENT_DYNAMIC(DialogAFProperties, CDialog)

/*
================
DialogAFProperties::DialogAFProperties
================
*/
DialogAFProperties::DialogAFProperties(CWnd* pParent /*=NULL*/)
	: CDialog(DialogAFProperties::IDD, pParent)
	, m_selfCollision(false)
	, m_linearFriction(0)
	, m_angularFriction(0)
	, m_contactFriction(0)
	, m_constraintFriction(0)
	, m_totalMass(0)
	, m_suspendLinearVelocity(0)
	, m_suspendAngularVelocity(0)
	, m_suspendLinearAcceleration(0)
	, m_suspendAngularAcceleration(0)
	, m_noMoveTime(0)
	, m_minMoveTime(0)
	, m_maxMoveTime(0)
	, m_linearTolerance(0)
	, m_angularTolerance(0)
	, file(NULL)
{
	Create( IDD_DIALOG_AF_PROPERTIES, pParent );
	EnableToolTips( TRUE );
}

/*
================
DialogAFProperties::~DialogAFProperties
================
*/
DialogAFProperties::~DialogAFProperties() {
}

/*
================
DialogAFProperties::DoDataExchange
================
*/
void DialogAFProperties::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAFProperties)
	DDX_Control(pDX, IDC_EDIT_MODEL, m_editModel);
	DDX_Control(pDX, IDC_EDIT_SKIN, m_editSkin);
	DDX_Check(pDX, IDC_CHECK_SELFCOLLISION, m_selfCollision);
	DDX_Control(pDX, IDC_EDIT_CONTENTS, m_editContents);
	DDX_Control(pDX, IDC_EDIT_CLIPMASK, m_editClipMask);
	DDX_Text(pDX, IDC_EDIT_LINEARFRICTION, m_linearFriction);
	DDX_Text(pDX, IDC_EDIT_ANGULARFRICTION, m_angularFriction);
	DDX_Text(pDX, IDC_EDIT_CONTACTFRICTION, m_contactFriction);
	DDX_Text(pDX, IDC_EDIT_CONSTRAINTFRICTION, m_constraintFriction);
	DDX_Text(pDX, IDC_EDIT_TOTALMASS, m_totalMass);
	DDX_Text(pDX, IDC_EDIT_LINEARVELOCITY, m_suspendLinearVelocity);
	DDX_Text(pDX, IDC_EDIT_ANGULARVELOCITY, m_suspendAngularVelocity);
	DDX_Text(pDX, IDC_EDIT_LINEARACCELERATION, m_suspendLinearAcceleration);
	DDX_Text(pDX, IDC_EDIT_ANGULARACCELERATION, m_suspendAngularAcceleration);
	DDX_Text(pDX, IDC_EDIT_NO_MOVE_TIME, m_noMoveTime);
	DDX_Text(pDX, IDC_EDIT_MINIMUM_MOVE_TIME, m_minMoveTime);
	DDX_Text(pDX, IDC_EDIT_MAXIMUM_MOVE_TIME, m_maxMoveTime);
	DDX_Text(pDX, IDC_EDIT_LINEAR_TOLERANCE, m_linearTolerance);
	DDX_Text(pDX, IDC_EDIT_ANGULAR_TOLERANCE, m_angularTolerance);
	//}}AFX_DATA_MAP
}

/*
================
DialogAFProperties::LoadFile
================
*/
void DialogAFProperties::LoadFile( idDeclAF *af ) {
	idStr str;

	file = af;

	if ( !file ) {
		ClearFile();
		return;
	}
	m_editModel.SetWindowText( file->model.c_str() );
	m_editSkin.SetWindowText( file->skin.c_str() );
	m_selfCollision = file->selfCollision;
	idDeclAF::ContentsToString( file->contents, str );
	m_editContents.SetWindowText( str );
	idDeclAF::ContentsToString( file->clipMask, str );
	m_editClipMask.SetWindowText( str );
	m_linearFriction = file->defaultLinearFriction;
	m_angularFriction = file->defaultAngularFriction;
	m_contactFriction = file->defaultContactFriction;
	m_constraintFriction = file->defaultConstraintFriction;
	m_totalMass = file->totalMass;
	m_suspendLinearVelocity = file->suspendVelocity[0];
	m_suspendAngularVelocity = file->suspendVelocity[1];
	m_suspendLinearAcceleration = file->suspendAcceleration[0];
	m_suspendAngularAcceleration = file->suspendAcceleration[1];
	m_noMoveTime = file->noMoveTime;
	m_minMoveTime = file->minMoveTime;
	m_maxMoveTime = file->maxMoveTime;
	m_linearTolerance = file->noMoveTranslation;
	m_angularTolerance = file->noMoveRotation;
	UpdateData( FALSE );
}

/*
================
DialogAFProperties::SetFile
================
*/
void DialogAFProperties::SaveFile( void ) {
	CString str;

	if ( !file ) {
		return;
	}
	UpdateData( TRUE );
	m_editModel.GetWindowText( str );
	file->model = str;
	m_editSkin.GetWindowText( str );
	file->skin = str;
	file->selfCollision = ( m_selfCollision != FALSE );
	m_editContents.GetWindowText( str );
	file->contents = idDeclAF::ContentsFromString( str );
	m_editClipMask.GetWindowText( str );
	file->clipMask = idDeclAF::ContentsFromString( str );
	file->defaultLinearFriction = m_linearFriction;
	file->defaultAngularFriction = m_angularFriction;
	file->defaultContactFriction = m_contactFriction;
	file->defaultConstraintFriction = m_constraintFriction;
	file->totalMass = m_totalMass;
	file->suspendVelocity[0] = m_suspendLinearVelocity;
	file->suspendVelocity[1] = m_suspendAngularVelocity;
	file->suspendAcceleration[0] = m_suspendLinearAcceleration;
	file->suspendAcceleration[1] = m_suspendAngularAcceleration;
	file->noMoveTime = m_noMoveTime;
	file->minMoveTime = m_minMoveTime;
	file->maxMoveTime = m_maxMoveTime;
	file->noMoveTranslation = m_linearTolerance;
	file->noMoveRotation = m_angularTolerance;

	AFDialogSetFileModified();
}

/*
================
DialogAFProperties::UpdateFile
================
*/
void DialogAFProperties::UpdateFile( void ) {
	SaveFile();
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAFProperties::ClearFile
================
*/
void DialogAFProperties::ClearFile( void ) {
	m_editModel.SetWindowText( "" );
	m_editSkin.SetWindowText( "" );
	m_selfCollision = false;
	m_editContents.SetWindowText( "" );
	m_editClipMask.SetWindowText( "" );
	m_linearFriction = 0.0f;
	m_angularFriction = 0.0f;
	m_contactFriction = 0.0f;
	m_constraintFriction = 0.0f;
	m_totalMass = -1.0f;
	m_suspendLinearVelocity = 0.0f;
	m_suspendAngularVelocity = 0.0f;
	m_suspendLinearAcceleration = 0.0f;
	m_suspendAngularAcceleration = 0.0f;
	m_noMoveTime = 0.0f;
	m_minMoveTime = 0.0f;
	m_maxMoveTime = 0.0f;
	m_linearTolerance = 0.0f;
	m_angularTolerance = 0.0f;
	UpdateData( FALSE );
}

/*
================
DialogAFProperties::OnToolHitTest
================
*/
int DialogAFProperties::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CDialog::OnToolHitTest( point, pTI );
	return DefaultOnToolHitTest( toolTips, this, point, pTI );
}

BEGIN_MESSAGE_MAP(DialogAFProperties, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_EN_CHANGE(IDC_EDIT_MODEL, OnEnChangeEditModel)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_MODEL, OnBnClickedButtonBrowseModel)
	ON_EN_CHANGE(IDC_EDIT_SKIN, OnEnChangeEditSkin)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SKIN, OnBnClickedButtonBrowseSkin)
	ON_EN_CHANGE(IDC_EDIT_LINEARFRICTION, OnEnChangeEditLinearfriction)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LINEARFRICTION, OnDeltaposSpinLinearfriction)
	ON_EN_CHANGE(IDC_EDIT_ANGULARFRICTION, OnEnChangeEditAngularfriction)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANGULARFRICTION, OnDeltaposSpinAngularfriction)
	ON_EN_CHANGE(IDC_EDIT_CONTACTFRICTION, OnEnChangeEditContactfriction)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CONTACTFRICTION, OnDeltaposSpinContactfriction)
	ON_EN_CHANGE(IDC_EDIT_CONSTRAINTFRICTION, OnEnChangeEditConstraintfriction)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CONSTRAINTFRICTION, OnDeltaposSpinConstraintfriction)
	ON_BN_CLICKED(IDC_CHECK_SELFCOLLISION, OnBnClickedCheckSelfcollision)
	ON_EN_CHANGE(IDC_EDIT_CONTENTS, OnEnChangeEditContents)
	ON_EN_CHANGE(IDC_EDIT_CLIPMASK, OnEnChangeEditClipmask)
	ON_EN_CHANGE(IDC_EDIT_TOTALMASS, OnEnChangeEditTotalmass)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TOTALMASS, OnDeltaposSpinTotalmass)
	ON_EN_CHANGE(IDC_EDIT_LINEARVELOCITY, OnEnChangeEditLinearvelocity)
	ON_EN_CHANGE(IDC_EDIT_ANGULARVELOCITY, OnEnChangeEditAngularvelocity)
	ON_EN_CHANGE(IDC_EDIT_LINEARACCELERATION, OnEnChangeEditLinearacceleration)
	ON_EN_CHANGE(IDC_EDIT_ANGULARACCELERATION, OnEnChangeEditAngularacceleration)
	ON_EN_CHANGE(IDC_EDIT_NO_MOVE_TIME, OnEnChangeEditNomovetime)
	ON_EN_CHANGE(IDC_EDIT_MINIMUM_MOVE_TIME, OnEnChangeEditMinimummovetime)
	ON_EN_CHANGE(IDC_EDIT_MAXIMUM_MOVE_TIME, OnEnChangeEditMaximummovetime)
	ON_EN_CHANGE(IDC_EDIT_LINEAR_TOLERANCE, OnEnChangeEditLineartolerance)
	ON_EN_CHANGE(IDC_EDIT_ANGULAR_TOLERANCE, OnEnChangeEditAngulartolerance)
END_MESSAGE_MAP()


// DialogAFProperties message handlers

BOOL DialogAFProperties::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

void DialogAFProperties::OnEnChangeEditModel() {
	if ( EditControlEnterHit( &m_editModel ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnEnChangeEditSkin() {
	if ( EditControlEnterHit( &m_editSkin ) ) {
		UpdateFile();
		// reload the .af file
		AFDialogReloadFile();
	}
}

void DialogAFProperties::OnBnClickedCheckSelfcollision() {
	UpdateFile();
	if ( file && file->bodies.Num() && MessageBox( "Apply to all bodies ?", "Self Collision", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
		for ( int i = 0; i < file->bodies.Num(); i++ ) {
			file->bodies[i]->selfCollision = file->selfCollision;
		}
		bodyDlg->LoadFile( file );
	}
}

void DialogAFProperties::OnEnChangeEditContents() {
	if ( EditControlEnterHit( &m_editContents ) ) {
		UpdateFile();
		if ( file && file->bodies.Num() && MessageBox( "Apply to all bodies ?", "Contents", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			for ( int i = 0; i < file->bodies.Num(); i++ ) {
				file->bodies[i]->contents = file->contents;
			}
			bodyDlg->LoadFile( file );
		}
	}
}

void DialogAFProperties::OnEnChangeEditClipmask() {
	if ( EditControlEnterHit( &m_editClipMask ) ) {
		UpdateFile();
		if ( file && file->bodies.Num() && MessageBox( "Apply to all bodies ?", "Clip Mask", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			for ( int i = 0; i < file->bodies.Num(); i++ ) {
				file->bodies[i]->clipMask = file->clipMask;
			}
			bodyDlg->LoadFile( file );
		}
	}
}

void DialogAFProperties::OnEnChangeEditLinearfriction() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_LINEARFRICTION ) ) ) {
		UpdateFile();
		if ( file && file->bodies.Num() && MessageBox( "Apply to all bodies ?", "Linear Friction", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			for ( int i = 0; i < file->bodies.Num(); i++ ) {
				file->bodies[i]->linearFriction = file->defaultLinearFriction;
			}
			bodyDlg->LoadFile( file );
		}
	}
	else {
		m_linearFriction = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_LINEARFRICTION ), false );
	}
}

void DialogAFProperties::OnDeltaposSpinLinearfriction(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_linearFriction = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_LINEARFRICTION ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFProperties::OnEnChangeEditAngularfriction() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANGULARFRICTION ) ) ) {
		UpdateFile();
		if ( file && file->bodies.Num() && MessageBox( "Apply to all bodies ?", "Angular Friction", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			for ( int i = 0; i < file->bodies.Num(); i++ ) {
				file->bodies[i]->angularFriction = file->defaultAngularFriction;
			}
			bodyDlg->LoadFile( file );
		}
	}
	else {
		m_angularFriction = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANGULARFRICTION ), false );
	}
}

void DialogAFProperties::OnDeltaposSpinAngularfriction(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_angularFriction = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_ANGULARFRICTION ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFProperties::OnEnChangeEditContactfriction() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CONTACTFRICTION ) ) ) {
		UpdateFile();
		if ( file && file->bodies.Num() && MessageBox( "Apply to all bodies ?", "Contact Friction", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			for ( int i = 0; i < file->bodies.Num(); i++ ) {
				file->bodies[i]->contactFriction = file->defaultContactFriction;
			}
			bodyDlg->LoadFile( file );
		}
	}
	else {
		m_contactFriction = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_CONTACTFRICTION ), false );
	}
}

void DialogAFProperties::OnDeltaposSpinContactfriction(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_contactFriction = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_CONTACTFRICTION ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFProperties::OnEnChangeEditConstraintfriction() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CONSTRAINTFRICTION ) ) ) {
		UpdateFile();
		if ( file && file->constraints.Num() && MessageBox( "Apply to all constraints ?", "Constraint Friction", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			for ( int i = 0; i < file->constraints.Num(); i++ ) {
				file->constraints[i]->friction = file->defaultConstraintFriction;
			}
			constraintDlg->LoadFile( file );
		}
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_CONSTRAINTFRICTION ), false );
	}
}

void DialogAFProperties::OnDeltaposSpinConstraintfriction(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_constraintFriction = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_CONSTRAINTFRICTION ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFProperties::OnEnChangeEditTotalmass() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_TOTALMASS ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnDeltaposSpinTotalmass(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_totalMass = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_TOTALMASS ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFProperties::OnEnChangeEditLinearvelocity() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_LINEARVELOCITY ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnEnChangeEditAngularvelocity() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANGULARVELOCITY ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnEnChangeEditLinearacceleration() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_LINEARACCELERATION ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnEnChangeEditAngularacceleration() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANGULARACCELERATION ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnEnChangeEditNomovetime() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_NO_MOVE_TIME ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnEnChangeEditMinimummovetime() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_MINIMUM_MOVE_TIME ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnEnChangeEditMaximummovetime() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_MAXIMUM_MOVE_TIME ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnEnChangeEditLineartolerance() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_LINEAR_TOLERANCE ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnEnChangeEditAngulartolerance() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANGULAR_TOLERANCE ) ) ) {
		UpdateFile();
	}
}

void DialogAFProperties::OnBnClickedButtonBrowseModel() {
//	m_editModel.SetWindowText( str );
//	UpdateFile();
}

void DialogAFProperties::OnBnClickedButtonBrowseSkin() {
//	m_editSkin.SetWindowText( str );
//	UpdateFile();
	// reload the .af file
//	AFDialogReloadFile();
}

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
#include "DialogAFName.h"
#include "DialogAFConstraint.h"
#include "DialogAFConstraintFixed.h"
#include "DialogAFConstraintBallAndSocket.h"
#include "DialogAFConstraintUniversal.h"
#include "DialogAFConstraintHinge.h"
#include "DialogAFConstraintSlider.h"
#include "DialogAFConstraintSpring.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif


typedef struct {
	declAFConstraintType_t type;
	const char *name;
} c_type_t;

c_type_t constraintTypes[] = {
	{ DECLAF_CONSTRAINT_FIXED, "fixed" },
	{ DECLAF_CONSTRAINT_BALLANDSOCKETJOINT, "ball and socket" },
	{ DECLAF_CONSTRAINT_UNIVERSALJOINT, "universal" },
	{ DECLAF_CONSTRAINT_HINGE, "hinge" },
	{ DECLAF_CONSTRAINT_SLIDER, "slider" },
	{ DECLAF_CONSTRAINT_SPRING, "spring" },
	{ DECLAF_CONSTRAINT_INVALID, NULL }
};


const char *ConstraintTypeToString( declAFConstraintType_t type ) {
	for ( int i = 0; constraintTypes[i].name; i++ ) {
		if ( constraintTypes[i].type == type ) {
			return constraintTypes[i].name;
		}
	}
	return "";
}

declAFConstraintType_t StringToConstraintType( const char *str ) {
	for ( int i = 0; constraintTypes[i].name; i++ ) {
		if ( idStr::Icmp( constraintTypes[i].name, str ) == 0 ) {
			return constraintTypes[i].type;
		}
	}
	return DECLAF_CONSTRAINT_INVALID;
}


// DialogAFConstraint dialog

toolTip_t DialogAFConstraint::toolTips[] = {
	{ IDC_COMBO_CONSTRAINTS, "select contraint for editing" },
	{ IDC_BUTTON_NEWCONSTRAINT, "create a new constraint" },
	{ IDC_BUTTON_RENAMECONSTRAINT, "rename the selected constraint" },
	{ IDC_BUTTON_DELETECONSTRAINT, "delete the selected constraint" },
	{ IDC_COMBO_CONSTRAINT_TYPE, "constraint type" },
	{ IDC_COMBO_CONSTRAINT_BODY1, "first constrained body" },
	{ IDC_COMBO_CONSTRAINT_BODY2, "second constrained body" },
	{ IDC_EDIT_CONSTRAINT_FRICTION, "constraint friction" },
	{ 0, NULL }
};

IMPLEMENT_DYNAMIC(DialogAFConstraint, CDialog)

/*
================
DialogAFConstraint::DialogAFConstraint
================
*/
DialogAFConstraint::DialogAFConstraint( CWnd* pParent /*=NULL*/ )
	: CDialog(DialogAFConstraint::IDD, pParent)
	, m_friction(0)
	, constraint(NULL)
	, file(NULL)
	, constraintDlg(NULL)
{
	Create( IDD_DIALOG_AF_CONSTRAINT, pParent );
	EnableToolTips( TRUE );
}

/*
================
DialogAFConstraint::~DialogAFConstraint
================
*/
DialogAFConstraint::~DialogAFConstraint() {
}

/*
================
DialogAFConstraint::DoDataExchange
================
*/
void DialogAFConstraint::DoDataExchange( CDataExchange* pDX ) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAFConstraint)
	DDX_Control(pDX, IDC_COMBO_CONSTRAINTS, m_comboConstraintList);
	DDX_Control(pDX, IDC_COMBO_CONSTRAINT_TYPE, m_comboConstraintType);
	DDX_Control(pDX, IDC_COMBO_CONSTRAINT_BODY1, m_comboBody1List);
	DDX_Control(pDX, IDC_COMBO_CONSTRAINT_BODY2, m_comboBody2List);
	DDX_Text(pDX, IDC_EDIT_CONSTRAINT_FRICTION, m_friction);
	//}}AFX_DATA_MAP
}

/*
================
DialogAFConstraint::InitConstraintList
================
*/
void DialogAFConstraint::InitConstraintList( void ) {
	CString str;

	m_comboConstraintList.ResetContent();
	if ( !file ) {
		return;
	}
	for ( int i = 0; i < file->constraints.Num(); i++ ) {
		m_comboConstraintList.AddString( file->constraints[i]->name.c_str() );
	}
	if ( m_comboConstraintList.GetCount() != 0 ) {
		m_comboConstraintList.SetCurSel( 0 );
		m_comboConstraintList.GetLBText( 0, str );
		LoadConstraint( str );
	}
}

/*
================
DialogAFConstraint::InitConstraintTypeDlg
================
*/
void DialogAFConstraint::InitConstraintTypeDlg( void ) {
	CString str;
	RECT rect;

	if ( !file || !constraint ) {
		return;
	}

	UpdateData( TRUE );

	if ( constraintDlg ) {
		constraintDlg->ShowWindow( SW_HIDE );
	}

	GetSafeComboBoxSelection( &m_comboConstraintType, str, -1 );
	switch( StringToConstraintType( str ) ) {
		case DECLAF_CONSTRAINT_FIXED:
			fixedDlg->LoadConstraint( constraint );
			constraintDlg = fixedDlg;
			break;
		case DECLAF_CONSTRAINT_BALLANDSOCKETJOINT:
			ballAndSocketDlg->LoadConstraint( constraint );
			constraintDlg = ballAndSocketDlg;
			break;
		case DECLAF_CONSTRAINT_UNIVERSALJOINT:
			universalDlg->LoadConstraint( constraint );
			constraintDlg = universalDlg;
			break;
		case DECLAF_CONSTRAINT_HINGE:
			hingeDlg->LoadConstraint( constraint );
			constraintDlg = hingeDlg;
			break;
		case DECLAF_CONSTRAINT_SLIDER:
			sliderDlg->LoadConstraint( constraint );
			constraintDlg = sliderDlg;
			break;
		case DECLAF_CONSTRAINT_SPRING:
			springDlg->LoadConstraint( constraint );
			constraintDlg = springDlg;
			break;
	}

	if ( constraintDlg ) {
		constraintDlg->ShowWindow( SW_SHOW );
		constraintDlg->GetWindowRect( &rect );
		constraintDlg->MoveWindow( 0, 117, rect.right - rect.left, rect.bottom - rect.top );
	}
}

/*
================
DialogAFConstraint::InitBodyLists
================
*/
void DialogAFConstraint::InitBodyLists( void ) {
	m_comboBody1List.ResetContent();
	m_comboBody2List.ResetContent();
	if ( !file ) {
		return;
	}
	for ( int i = 0; i < file->bodies.Num(); i++ ) {
		m_comboBody1List.AddString( file->bodies[i]->name );
		m_comboBody2List.AddString( file->bodies[i]->name );
	}
	// the second body may also be the world
	m_comboBody2List.AddString( "world" );	// FIXME: we currently assume this is the last body in the list
}

/*
================
DialogAFConstraint::InitNewRenameDeleteButtons
================
*/
void DialogAFConstraint::InitNewRenameDeleteButtons( void ) {
	if ( file && file->bodies.Num() >= 1 ) {
		GetDlgItem( IDC_BUTTON_NEWCONSTRAINT )->EnableWindow( true );
	}
	else {
		GetDlgItem( IDC_BUTTON_NEWCONSTRAINT )->EnableWindow( false );
	}

	if ( file && m_comboConstraintList.GetCount() >= 1 ) {
		GetDlgItem( IDC_BUTTON_RENAMECONSTRAINT )->EnableWindow( true );
		GetDlgItem( IDC_BUTTON_DELETECONSTRAINT )->EnableWindow( true );
	}
	else {
		GetDlgItem( IDC_BUTTON_RENAMECONSTRAINT )->EnableWindow( false );
		GetDlgItem( IDC_BUTTON_DELETECONSTRAINT )->EnableWindow( false );
	}
}

/*
================
DialogAFConstraint::LoadFile
================
*/
void DialogAFConstraint::LoadFile( idDeclAF *af ) {
	file = af;
	constraint = NULL;
	ballAndSocketDlg->LoadFile( af );
	universalDlg->LoadFile( af );
	hingeDlg->LoadFile( af );
	sliderDlg->LoadFile( af );
	springDlg->LoadFile( af );
	InitBodyLists();
	InitConstraintList();
	InitNewRenameDeleteButtons();
}

/*
================
DialogAFConstraint::SaveFile
================
*/
void DialogAFConstraint::SaveFile( void ) {
	SaveConstraint();
}

/*
================
DialogAFConstraint::LoadConstraint
================
*/
void DialogAFConstraint::LoadConstraint( const char *name ) {
	int i, s1, s2;

	if ( !file ) {
		return;
	}
	for ( i = 0; i < file->constraints.Num(); i++ ) {
		if ( file->constraints[i]->name.Icmp( name ) == 0 ) {
			break;
		}
	}
	if ( i >= file->constraints.Num() ) {
		return;
	}
	constraint = file->constraints[i];

	// load the constraint type from the current idDeclAF_Constraint
	SetSafeComboBoxSelection( &m_comboConstraintType, ConstraintTypeToString( constraint->type ), -1 );

	// load constrained bodies from the current idDeclAF_Constraint
	s1 = SetSafeComboBoxSelection( &m_comboBody1List, constraint->body1.c_str(), -1 );
	s2 = SetSafeComboBoxSelection( &m_comboBody2List, constraint->body2.c_str(), s1 );

	// load friction from the current idDeclAF_Constraint
	m_friction = constraint->friction;

	// update displayed values
	UpdateData( FALSE );

	InitConstraintTypeDlg();

	if ( GetStyle() & WS_VISIBLE ) {
		// highlight the current constraint ingame
		cvarSystem->SetCVarString( "af_highlightConstraint", name );
	}
}

/*
================
DialogAFConstraint::SaveConstraint
================
*/
void DialogAFConstraint::SaveConstraint( void ) {
	int s1, s2;
	CString str;

	if ( !file || !constraint ) {
		return;
	}
	UpdateData( TRUE );

	// save constraint type to the current idDeclAF_Constraint
	GetSafeComboBoxSelection( &m_comboConstraintType, str, -1 );
	constraint->type = StringToConstraintType( str );

	// save constrained bodies to the current idDeclAF_Constraint
	s1 = GetSafeComboBoxSelection( &m_comboBody1List, str, -1 );
	constraint->body1 = str;
	s2 = GetSafeComboBoxSelection( &m_comboBody2List, str, s1 );
	constraint->body2 = str;

	// save friction to the current idDeclAF_Constraint
	constraint->friction = m_friction;

	AFDialogSetFileModified();
}

/*
================
DialogAFConstraint::UpdateFile
================
*/
void DialogAFConstraint::UpdateFile( void ) {
	SaveConstraint();
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAFConstraint::OnInitDialog
================
*/
BOOL DialogAFConstraint::OnInitDialog()  {

	CDialog::OnInitDialog();

	// initialize the constraint types
	m_comboConstraintType.ResetContent();
	for ( int i = 0; constraintTypes[i].name; i++ ) {
		m_comboConstraintType.AddString( constraintTypes[i].name );
	}

	fixedDlg = new DialogAFConstraintFixed( this );
	fixedDlg->ShowWindow( SW_HIDE );

	ballAndSocketDlg = new DialogAFConstraintBallAndSocket( this );
	ballAndSocketDlg->ShowWindow( SW_HIDE );

	universalDlg = new DialogAFConstraintUniversal( this );
	universalDlg->ShowWindow( SW_HIDE );

	hingeDlg = new DialogAFConstraintHinge( this );
	hingeDlg->ShowWindow( SW_HIDE );

	sliderDlg = new DialogAFConstraintSlider( this );
	sliderDlg->ShowWindow( SW_HIDE );

	springDlg = new DialogAFConstraintSpring( this );
	springDlg->ShowWindow( SW_HIDE );

	constraintDlg = NULL;

	InitNewRenameDeleteButtons();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
================
DialogAFConstraint::OnToolHitTest
================
*/
int DialogAFConstraint::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CDialog::OnToolHitTest( point, pTI );
	return DefaultOnToolHitTest( toolTips, this, point, pTI );
}

BEGIN_MESSAGE_MAP(DialogAFConstraint, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_COMBO_CONSTRAINTS, OnCbnSelchangeComboConstraints)
	ON_CBN_SELCHANGE(IDC_COMBO_CONSTRAINT_TYPE, OnCbnSelchangeComboConstraintType)
	ON_CBN_SELCHANGE(IDC_COMBO_CONSTRAINT_BODY1, OnCbnSelchangeComboConstraintBody1)
	ON_CBN_SELCHANGE(IDC_COMBO_CONSTRAINT_BODY2, OnCbnSelchangeComboConstraintBody2)
	ON_EN_CHANGE(IDC_EDIT_CONSTRAINT_FRICTION, OnEnChangeEditConstraintFriction)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CONSTRAINT_FRICTION, OnDeltaposSpinConstraintFriction)
	ON_BN_CLICKED(IDC_BUTTON_NEWCONSTRAINT, OnBnClickedButtonNewconstraint)
	ON_BN_CLICKED(IDC_BUTTON_RENAMECONSTRAINT, OnBnClickedButtonRenameconstraint)
	ON_BN_CLICKED(IDC_BUTTON_DELETECONSTRAINT, OnBnClickedButtonDeleteconstraint)
END_MESSAGE_MAP()


// DialogAFConstraint message handlers

BOOL DialogAFConstraint::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

void DialogAFConstraint::OnShowWindow( BOOL bShow, UINT nStatus ) {
	if ( bShow && constraint ) {
		cvarSystem->SetCVarString( "af_highlightConstraint", constraint->name.c_str() );
	} else {
		cvarSystem->SetCVarString( "af_highlightConstraint", "" );
	}
	CDialog::OnShowWindow( bShow, nStatus );
}

void DialogAFConstraint::OnCbnSelchangeComboConstraints() {
	CString str;

	GetSafeComboBoxSelection( &m_comboConstraintList, str, -1 );
	LoadConstraint( str );
}

void DialogAFConstraint::OnBnClickedButtonNewconstraint() {
	DialogAFName nameDlg;
	CString str;

	nameDlg.SetComboBox( &m_comboConstraintList );
	if ( nameDlg.DoModal() == IDOK ) {
		nameDlg.GetName( str );
		// create new constraint
		file->NewConstraint( str );
		m_comboConstraintList.SetCurSel( m_comboConstraintList.AddString( str ) );
		LoadConstraint( str );
		gameEdit->AF_UpdateEntities( file->GetName() );
		AFDialogSetFileModified();
	}
	InitNewRenameDeleteButtons();
}

void DialogAFConstraint::OnBnClickedButtonRenameconstraint() {
	int i;
	CString name, newName;
	DialogAFName nameDlg;

	if ( !file || !constraint ) {
		return;
	}

	i = m_comboConstraintList.GetCurSel();
	if ( i != CB_ERR ) {
		m_comboConstraintList.GetLBText( i, name );
		nameDlg.SetName( name );
		nameDlg.SetComboBox( &m_comboConstraintList );
		if ( nameDlg.DoModal() == IDOK ) {
			nameDlg.GetName( newName );
			// rename constraint;
			file->RenameConstraint( name, newName );
			m_comboConstraintList.DeleteString( i );
			m_comboConstraintList.SetCurSel( m_comboConstraintList.AddString( newName ) );
			LoadConstraint( newName );
			gameEdit->AF_UpdateEntities( file->GetName() );
			AFDialogSetFileModified();
		}
	}
}

void DialogAFConstraint::OnBnClickedButtonDeleteconstraint() {
	int i;
	CString str;

	if ( !file || !constraint ) {
		return;
	}

	i = m_comboConstraintList.GetCurSel();
	if ( i != CB_ERR ) {
		if ( MessageBox( "Are you sure you want to delete this constraint ?", "Delete Constraint", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			m_comboConstraintList.GetLBText( i, str );
			// delete current constraint
			file->DeleteConstraint( str );
			constraint = NULL;
			m_comboConstraintList.DeleteString( i );
			OnCbnSelchangeComboConstraints();
			gameEdit->AF_UpdateEntities( file->GetName() );
			AFDialogSetFileModified();
		}
	}
	InitNewRenameDeleteButtons();
}

void DialogAFConstraint::OnCbnSelchangeComboConstraintType() {
	InitConstraintTypeDlg();
	UpdateFile();
}

void DialogAFConstraint::OnCbnSelchangeComboConstraintBody1() {
	CString str;
	GetSafeComboBoxSelection( &m_comboBody1List, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboBody2List, str );
	UpdateFile();
}

void DialogAFConstraint::OnCbnSelchangeComboConstraintBody2() {
	CString str;
	GetSafeComboBoxSelection( &m_comboBody2List, str, -1 );
	UnsetSafeComboBoxSelection( &m_comboBody1List, str );
	UpdateFile();
}

void DialogAFConstraint::OnEnChangeEditConstraintFriction() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CONSTRAINT_FRICTION ) ) ) {
		UpdateFile();
	}
	else {
		EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_CONSTRAINT_FRICTION ), false );
	}
}

void DialogAFConstraint::OnDeltaposSpinConstraintFriction(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_friction = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_CONSTRAINT_FRICTION ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

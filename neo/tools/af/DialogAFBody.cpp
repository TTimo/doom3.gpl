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
#include "DialogAFBody.h"

typedef struct {
	traceModel_t type;
	const char *name;
} cm_type_t;

cm_type_t modelTypes[] = {
	{ TRM_INVALID, "invalid" },
	{ TRM_BOX, "box" },
	{ TRM_OCTAHEDRON, "octahedron" },
	{ TRM_DODECAHEDRON, "dodecahedron" },
	{ TRM_CYLINDER, "cylinder" },
	{ TRM_CONE, "cone" },
	{ TRM_BONE, "bone" },
	{ TRM_CUSTOM, "custom" },
	{ TRM_INVALID, NULL }
};

const char *ModelTypeToString( int type ) {
	for ( int i = 0; modelTypes[i].name; i++ ) {
		if ( modelTypes[i].type == type ) {
			return modelTypes[i].name;
		}
	}
	return "";
}

traceModel_t StringToModelType( const char *str ) {
	for ( int i = 0; modelTypes[i].name; i++ ) {
		if ( idStr::Icmp( modelTypes[i].name, str ) == 0 ) {
			return modelTypes[i].type;
		}
	}
	return TRM_INVALID;
}

// DialogAFBody dialog

toolTip_t DialogAFBody::toolTips[] = {
	{ IDC_COMBO_BODIES, "select body for editing" },
	{ IDC_BUTTON_NEWBODY, "create a new body" },
	{ IDC_BUTTON_RENAMEBODY, "rename the selected body" },
	{ IDC_BUTTON_DELETEBODY, "delete the selected body" },
	{ IDC_COMBO_CM_TYPE, "collision model type" },
	{ IDC_EDIT_CM_NAME, "custom collision model" },
	{ IDC_BUTTON_CM_BROWSE, "browse custom collision model" },
	{ IDC_COMBO_BONE_JOINT1, "first joint of bone collision model" },
	{ IDC_COMBO_BONE_JOINT2, "second joint of bone collision model" },
	{ IDC_EDIT_CM_HEIGHT, "hieght of the collision model" },
	{ IDC_EDIT_CM_WIDTH, "width of the collision model" },
	{ IDC_EDIT_CM_LENGTH, "length of the collision model" },
	{ IDC_EDIT_CM_NUMSIDES, "number of sides of the collision model" },
	{ IDC_EDIT_CM_DENSITY, "collision model density" },
	{ IDC_EDIT_CM_INERTIASCALE, "inertia tensor scale" },
	{ IDC_RADIO_ORIGIN_COORDINATES, "use absolute coordinates for the body origin" },
	{ IDC_RADIO_ORIGIN_BONECENTER, "use the center of a bone for the body origin" },
	{ IDC_RADIO_ORIGIN_JOINT, "use the position of a joint for the body origin" },
	{ IDC_EDIT_AF_VECTOR_X, "x-coordinate" },
	{ IDC_EDIT_AF_VECTOR_Y, "y-coordinate" },
	{ IDC_EDIT_AF_VECTOR_Z, "z-coordinate" },
	{ IDC_COMBO_ORIGIN_BONECENTER_JOINT1, "bone start joint" },
	{ IDC_COMBO_ORIGIN_BONECENTER_JOINT2, "bone end joint" },
	{ IDC_COMBO_ORIGIN_JOINT, "joint name" },
	{ IDC_EDIT_ANGLES_PITCH, "pitch angle of body" },
	{ IDC_EDIT_ANGLES_YAW, "yaw angle of body" },
	{ IDC_EDIT_ANGLES_ROLL, "roll angle of body" },
	{ IDC_EDIT_LINEARFRICTION, "translational friction" },
	{ IDC_EDIT_ANGULARFRICTION, "rotational friction" },
	{ IDC_EDIT_CONTACTFRICTION, "friction with contact surfaces" },
	{ IDC_CHECK_SELFCOLLISION, "collide with other bodies of this articulated figure" },
	{ IDC_EDIT_CONTENTS, "content of body" },
	{ IDC_EDIT_CLIPMASK, "collide with these content types" },
	{ IDC_EDIT_FRICTIONDIRECTION, "single friction direction relative to body" },
	{ IDC_EDIT_CONTACTMOTORDIRECTION, "contact motor direction" },
	{ IDC_COMBO_MODIFIEDJOINT, "the joint modified by the relative rotation of the body" },
	{ IDC_RADIO_MODIFY_ORIENTATION, "modify the joint orientation" },
	{ IDC_RADIO_MODIFY_POSITION, "modify the joint position" },
	{ IDC_RADIO_MODIFY_BOTH, "modify the joint orientation and position" },
	{ IDC_EDIT_CONTAINEDJOINTS, "all the joints contained by this body" },
	{ 0, NULL }
};

IMPLEMENT_DYNAMIC(DialogAFBody, CDialog)

/*
================
DialogAFBody::DialogAFBody
================
*/
DialogAFBody::DialogAFBody( CWnd* pParent /*=NULL*/ )
	: CDialog(DialogAFBody::IDD, pParent)
	, constraintDlg(NULL)
	, numJoints(0)
	, cm_length(0)
	, cm_height(0)
	, cm_width(0)
	, cm_density(0)
	, cm_numSides(3)
	, cm_origin_x(0)
	, cm_origin_y(0)
	, cm_origin_z(0)
	, cm_angles_pitch(0)
	, cm_angles_yaw(0)
	, cm_angles_roll(0)
	, m_selfCollision(false)
	, m_linearFriction(0)
	, m_angularFriction(0)
	, m_contactFriction(0)
	, file(NULL)
	, body(NULL)
{
	Create( IDD_DIALOG_AF_BODY, pParent );
	EnableToolTips( TRUE );
}

/*
================
DialogAFBody::~DialogAFBody
================
*/
DialogAFBody::~DialogAFBody() {
}

/*
================
DialogAFBody::DoDataExchange
================
*/
void DialogAFBody::DoDataExchange( CDataExchange* pDX ) {
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(DialogAFBody)
	DDX_Control(pDX, IDC_COMBO_BODIES, bodyList);
	DDX_Control(pDX, IDC_COMBO_CM_TYPE, cm_comboType);
	DDX_Text(pDX, IDC_EDIT_CM_LENGTH, cm_length);
	DDX_Text(pDX, IDC_EDIT_CM_HEIGHT, cm_height);
	DDX_Text(pDX, IDC_EDIT_CM_WIDTH, cm_width);
	DDX_Text(pDX, IDC_EDIT_CM_NUMSIDES, cm_numSides);
	DDX_Control(pDX, IDC_COMBO_BONE_JOINT1, cm_comboBoneJoint1 );
	DDX_Control(pDX, IDC_COMBO_BONE_JOINT2, cm_comboBoneJoint2 );
	DDX_Text(pDX, IDC_EDIT_CM_DENSITY, cm_density);
	DDX_Control(pDX, IDC_EDIT_CM_INERTIASCALE, cm_inertiaScale);
	DDX_Text(pDX, IDC_EDIT_AF_VECTOR_X, cm_origin_x);
	DDX_Text(pDX, IDC_EDIT_AF_VECTOR_Y, cm_origin_y);
	DDX_Text(pDX, IDC_EDIT_AF_VECTOR_Z, cm_origin_z);
	DDX_Control(pDX, IDC_COMBO_ORIGIN_BONECENTER_JOINT1, cm_originBoneCenterJoint1);
	DDX_Control(pDX, IDC_COMBO_ORIGIN_BONECENTER_JOINT2, cm_originBoneCenterJoint2);
	DDX_Control(pDX, IDC_COMBO_ORIGIN_JOINT, cm_originJoint);
	DDX_Text(pDX, IDC_EDIT_ANGLES_PITCH, cm_angles_pitch);
	DDX_Text(pDX, IDC_EDIT_ANGLES_YAW, cm_angles_yaw);
	DDX_Text(pDX, IDC_EDIT_ANGLES_ROLL, cm_angles_roll);
	DDX_Check(pDX, IDC_CHECK_SELFCOLLISION, m_selfCollision);
	DDX_Control(pDX, IDC_EDIT_CONTENTS, m_editContents);
	DDX_Control(pDX, IDC_EDIT_CLIPMASK, m_editClipMask);
	DDX_Text(pDX, IDC_EDIT_LINEARFRICTION, m_linearFriction);
	DDX_Text(pDX, IDC_EDIT_ANGULARFRICTION, m_angularFriction);
	DDX_Text(pDX, IDC_EDIT_CONTACTFRICTION, m_contactFriction);
	DDX_Control(pDX, IDC_EDIT_FRICTIONDIRECTION, m_frictionDirection);
	DDX_Control(pDX, IDC_EDIT_CONTACTMOTORDIRECTION, m_contactMotorDirection);
	DDX_Control(pDX, IDC_COMBO_MODIFIEDJOINT, m_comboModifiedJoint);
	DDX_Control(pDX, IDC_EDIT_CONTAINEDJOINTS, m_editContainedJoints);
	//}}AFX_DATA_MAP
}

/*
================
DialogAFBody::InitBodyList
================
*/
void DialogAFBody::InitBodyList( void ) {
	CString str;

	bodyList.ResetContent();
	if ( !file ) {
		return;
	}
	for ( int i = 0; i < file->bodies.Num(); i++ ) {
		bodyList.AddString( file->bodies[i]->name.c_str() );
	}
	if ( bodyList.GetCount() != 0 ) {
		bodyList.SetCurSel( 0 );
		bodyList.GetLBText( 0, str );
		LoadBody( str );
	}
}

/*
================
DialogAFBody::InitJointLists

  initialize the joint lists for bone collision models
================
*/
void DialogAFBody::InitJointLists( void ) {
	idStrList joints;

	cm_comboBoneJoint1.ResetContent();
	cm_comboBoneJoint2.ResetContent();
	cm_originBoneCenterJoint1.ResetContent();
	cm_originBoneCenterJoint2.ResetContent();
	cm_originJoint.ResetContent();
	numJoints = 0;

	if ( !file ) {
		return;
	}

	const idRenderModel *model = gameEdit->ANIM_GetModelFromName( file->model );
	if ( !model ) {
		return;
	}

	numJoints = model->NumJoints();
	for ( int i = 0; i < numJoints; i++ ) {
		const char *jointName = model->GetJointName( (jointHandle_t) i );
		cm_comboBoneJoint1.AddString( jointName );
		cm_comboBoneJoint2.AddString( jointName );
		cm_originBoneCenterJoint1.AddString( jointName );
		cm_originBoneCenterJoint2.AddString( jointName );
		cm_originJoint.AddString( jointName );
	}
}

/*
================
DialogAFBody::InitCollisionModelType
================
*/
void DialogAFBody::InitCollisionModelType( void ) {
	int showBone, showOther;
	bool enableOther;
	CString str;

	UpdateData( TRUE );
	cm_comboType.GetLBText( cm_comboType.GetCurSel(), str );
	traceModel_t type = StringToModelType( str );
	if ( type == TRM_BONE ) {
		showBone = SW_SHOW;
		showOther = SW_HIDE;
		enableOther = false;
		GetDlgItem( IDC_STATIC_CM_LENGTH )->SetWindowText( "Joint 1" );
		GetDlgItem( IDC_STATIC_CM_HEIGHT )->SetWindowText( "Joint 2" );
		CheckRadioButton( IDC_RADIO_ORIGIN_COORDINATES,
							IDC_RADIO_ORIGIN_JOINT,
								IDC_RADIO_ORIGIN_BONECENTER );
	}
	else {
		showBone = SW_HIDE;
		showOther = SW_SHOW;
		enableOther = true;
		GetDlgItem( IDC_STATIC_CM_LENGTH )->SetWindowText( "Length" );
		GetDlgItem( IDC_STATIC_CM_HEIGHT )->SetWindowText( "Height" );
	}
	GetDlgItem( IDC_COMBO_BONE_JOINT1 )->ShowWindow( showBone );
	GetDlgItem( IDC_COMBO_BONE_JOINT2 )->ShowWindow( showBone );
	GetDlgItem( IDC_EDIT_CM_LENGTH )->ShowWindow( showOther );
	GetDlgItem( IDC_SPIN_CM_LENGTH )->ShowWindow( showOther );
	GetDlgItem( IDC_EDIT_CM_HEIGHT )->ShowWindow( showOther );
	GetDlgItem( IDC_SPIN_CM_HEIGHT )->ShowWindow( showOther );

	// enable/disable origin controls
	GetDlgItem( IDC_EDIT_AF_VECTOR_X )->EnableWindow( enableOther );
	GetDlgItem( IDC_SPIN_AF_VECTOR_X )->EnableWindow( enableOther );
	GetDlgItem( IDC_EDIT_AF_VECTOR_Y )->EnableWindow( enableOther );
	GetDlgItem( IDC_SPIN_AF_VECTOR_Y )->EnableWindow( enableOther );
	GetDlgItem( IDC_EDIT_AF_VECTOR_Z )->EnableWindow( enableOther );
	GetDlgItem( IDC_SPIN_AF_VECTOR_Z )->EnableWindow( enableOther );

	// enable/disable joint controls
	GetDlgItem( IDC_COMBO_ORIGIN_BONECENTER_JOINT1 )->EnableWindow( enableOther );
	GetDlgItem( IDC_COMBO_ORIGIN_BONECENTER_JOINT2 )->EnableWindow( enableOther );
	GetDlgItem( IDC_COMBO_ORIGIN_JOINT )->EnableWindow( enableOther );

	// enable/disable angles controls
	GetDlgItem( IDC_STATIC_ANGLES_PITCH )->EnableWindow( enableOther );
	GetDlgItem( IDC_EDIT_ANGLES_PITCH )->EnableWindow( enableOther );
	GetDlgItem( IDC_SPIN_ANGLES_PITCH )->EnableWindow( enableOther );
	GetDlgItem( IDC_STATIC_ANGLES_YAW )->EnableWindow( enableOther );
	GetDlgItem( IDC_EDIT_ANGLES_YAW )->EnableWindow( enableOther );
	GetDlgItem( IDC_SPIN_ANGLES_YAW )->EnableWindow( enableOther );
	GetDlgItem( IDC_STATIC_ANGLES_ROLL )->EnableWindow( enableOther );
	GetDlgItem( IDC_EDIT_ANGLES_ROLL )->EnableWindow( enableOther );
	GetDlgItem( IDC_SPIN_ANGLES_ROLL )->EnableWindow( enableOther );

	GetDlgItem( IDC_RADIO_ORIGIN_COORDINATES )->EnableWindow( enableOther );
	GetDlgItem( IDC_RADIO_ORIGIN_BONECENTER )->EnableWindow( enableOther );
	GetDlgItem( IDC_RADIO_ORIGIN_JOINT )->EnableWindow( enableOther );

	if ( type == TRM_CONE || type == TRM_CYLINDER ) {
		GetDlgItem( IDC_EDIT_CM_NUMSIDES )->EnableWindow( true );
		GetDlgItem( IDC_SPIN_CM_NUMSIDES )->EnableWindow( true );
	}
	else {
		GetDlgItem( IDC_EDIT_CM_NUMSIDES )->EnableWindow( false );
		GetDlgItem( IDC_SPIN_CM_NUMSIDES )->EnableWindow( false );
	}
}

/*
================
DialogAFBody::InitModifiedJointList
================
*/
void DialogAFBody::InitModifiedJointList( void ) {
	int i, j, numJoints;
	CString str;

	m_comboModifiedJoint.ResetContent();

	if ( !file ) {
		return;
	}

	const idRenderModel *model = gameEdit->ANIM_GetModelFromName( file->model );
	if ( !model ) {
		return;
	}

	numJoints = model->NumJoints();
	for ( i = 0; i < numJoints; i++ ) {
		const char *jointName = model->GetJointName( (jointHandle_t) i );
		for ( j = 0; j < file->bodies.Num(); j++ ) {
			if ( file->bodies[j] == body ) {
				continue;
			}
			if ( file->bodies[j]->jointName.Icmp( jointName ) == 0 ) {
				break;
			}
		}
		if ( j < file->bodies.Num() ) {
			continue;
		}
		m_comboModifiedJoint.AddString( jointName );
	}

	if ( body ) {
		if ( body->jointName.Length() == 0 ) {
			m_comboModifiedJoint.GetLBText( 0, str );
			body->jointName = str;
		}
		SetSafeComboBoxSelection( &m_comboModifiedJoint, body->jointName, -1 );
		// cannot change the body which modifies the origin joint
		if ( body->jointName.Icmp( "origin" ) == 0 ) {
			// check if there is another body which modifies the "origin" joint
			for ( i = 0; i < file->bodies.Num(); i++ ) {
				if ( file->bodies[i] == body ) {
					continue;
				}
				if ( file->bodies[i]->jointName.Icmp( "origin" ) == 0 ) {
					break;
				}
			}
			// if there is another body which modifies the "origin" joint
			if ( i < file->bodies.Num() ) {
				GetDlgItem( IDC_COMBO_MODIFIEDJOINT )->EnableWindow( true );
			}
			else {
				GetDlgItem( IDC_COMBO_MODIFIEDJOINT )->EnableWindow( false );
			}
		}
		else {
			GetDlgItem( IDC_COMBO_MODIFIEDJOINT )->EnableWindow( true );
		}
	}
}

/*
================
DialogAFBody::InitNewRenameDeleteButtons
================
*/
void DialogAFBody::InitNewRenameDeleteButtons( void ) {
	if ( file && numJoints > file->bodies.Num() ) {
		GetDlgItem( IDC_BUTTON_NEWBODY )->EnableWindow( true );
	}
	else {
		GetDlgItem( IDC_BUTTON_NEWBODY )->EnableWindow( false );
	}

	if ( file && bodyList.GetCount() >= 1 ) {
		GetDlgItem( IDC_BUTTON_RENAMEBODY )->EnableWindow( true );
		GetDlgItem( IDC_BUTTON_DELETEBODY )->EnableWindow( true );
	}
	else {
		GetDlgItem( IDC_BUTTON_RENAMEBODY )->EnableWindow( false );
		GetDlgItem( IDC_BUTTON_DELETEBODY )->EnableWindow( false );
	}
}

/*
================
DialogAFBody::LoadFile
================
*/
void DialogAFBody::LoadFile( idDeclAF *af ) {
	file = af;
	body = NULL;
	InitJointLists();
	InitBodyList();
	InitNewRenameDeleteButtons();
}

/*
================
DialogAFBody::SaveFile
================
*/
void DialogAFBody::SaveFile( void ) {
	SaveBody();
}

/*
================
DialogAFBody::LoadBody
================
*/
void DialogAFBody::LoadBody( const char *name ) {
	int i, s1, s2;
	idStr str;

	if ( !file ) {
		return;
	}
	for ( i = 0; i < file->bodies.Num(); i++ ) {
		if ( file->bodies[i]->name.Icmp( name ) == 0 ) {
			break;
		}
	}
	if ( i >= file->bodies.Num() ) {
		return;
	}
	body = file->bodies[i];

	// load collision model from the current idDeclAF_Body
	SetSafeComboBoxSelection( &cm_comboType, ModelTypeToString( body->modelType ), -1 );
	if ( body->modelType == TRM_BONE ) {
		s1 = SetSafeComboBoxSelection( &cm_comboBoneJoint1, body->v1.joint1.c_str(), -1 );
		s2 = SetSafeComboBoxSelection( &cm_comboBoneJoint2, body->v2.joint1.c_str(), s1 );
		cm_width = body->width;
		cm_length = cm_height = 20.0f;
		s1 = SetSafeComboBoxSelection( &cm_originBoneCenterJoint1, body->v1.joint1.c_str(), -1 );
		s2 = SetSafeComboBoxSelection( &cm_originBoneCenterJoint2, body->v2.joint1.c_str(), s1 );
	}
	else {
		cm_length = body->v2.ToVec3().x - body->v1.ToVec3().x;
		cm_height = body->v2.ToVec3().z - body->v1.ToVec3().z;
		cm_width = body->v2.ToVec3().y - body->v1.ToVec3().y;
		cm_origin_x = body->origin.ToVec3().x;
		cm_origin_y = body->origin.ToVec3().y;
		cm_origin_z = body->origin.ToVec3().z;
		s1 = SetSafeComboBoxSelection( &cm_originBoneCenterJoint1, body->origin.joint1.c_str(), -1 );
		s2 = SetSafeComboBoxSelection( &cm_originBoneCenterJoint2, body->origin.joint2.c_str(), s1 );
		s1 = SetSafeComboBoxSelection( &cm_originJoint, body->origin.joint1.c_str(), -1 );
		cm_angles_pitch = body->angles.pitch;
		cm_angles_yaw = body->angles.yaw;
		cm_angles_roll = body->angles.roll;
		if ( body->origin.type == idAFVector::VEC_BONECENTER ) {
			i = IDC_RADIO_ORIGIN_BONECENTER;
		}
		else if ( body->origin.type == idAFVector::VEC_JOINT ) {
			i = IDC_RADIO_ORIGIN_JOINT;
		}
		else {
			i = IDC_RADIO_ORIGIN_COORDINATES;
		}
		CheckRadioButton( IDC_RADIO_ORIGIN_COORDINATES, IDC_RADIO_ORIGIN_JOINT, i );
	}
	cm_numSides = body->numSides;
	cm_density = body->density;
	if ( body->inertiaScale == mat3_identity ) {
		cm_inertiaScale.SetWindowText( "none" );
	}
	else {
		cm_inertiaScale.SetWindowText( body->inertiaScale.ToString( 1 ) );
	}

	// load collision detection settings from the current idDeclAF_Body
	m_selfCollision = body->selfCollision;
	idDeclAF::ContentsToString( body->contents, str );
	m_editContents.SetWindowText( str );
	idDeclAF::ContentsToString( body->clipMask, str );
	m_editClipMask.SetWindowText( str );

	// load friction settings from the current idDeclAF_Body
	m_linearFriction = body->linearFriction;
	m_angularFriction = body->angularFriction;
	m_contactFriction = body->contactFriction;

	// friction direction and contact motor direction
	if ( body->frictionDirection.ToVec3() != vec3_origin ) {
		idFile_Memory file( "frictionDirection" );
		file.WriteFloatString( "%f %f %f", body->frictionDirection.ToVec3().x, body->frictionDirection.ToVec3().y, body->frictionDirection.ToVec3().z );
		m_frictionDirection.SetWindowText( file.GetDataPtr() );
	} else {
		m_frictionDirection.SetWindowText( "" );
	}
	if ( body->contactMotorDirection.ToVec3() != vec3_origin ) {
		idFile_Memory file( "contactMotorDirection" );
		file.WriteFloatString( "%f %f %f", body->contactMotorDirection.ToVec3().x, body->contactMotorDirection.ToVec3().y, body->contactMotorDirection.ToVec3().z );
		m_contactMotorDirection.SetWindowText( file.GetDataPtr() );
	} else {
		m_contactMotorDirection.SetWindowText( "" );
	}

	// load joint settings from the current idDeclAF_Body
	InitModifiedJointList();
	if ( body->jointMod == DECLAF_JOINTMOD_AXIS ) {
		i = IDC_RADIO_MODIFY_ORIENTATION;
	} else if ( body->jointMod == DECLAF_JOINTMOD_ORIGIN ) {
		i = IDC_RADIO_MODIFY_POSITION;
	} else if ( body->jointMod == DECLAF_JOINTMOD_BOTH ) {
		i = IDC_RADIO_MODIFY_BOTH;
	} else {
		i = IDC_RADIO_MODIFY_ORIENTATION;
	}
	CheckRadioButton( IDC_RADIO_MODIFY_ORIENTATION, IDC_RADIO_MODIFY_BOTH, i );
	m_editContainedJoints.SetWindowText( body->containedJoints.c_str() );

	// update displayed values
	UpdateData( FALSE );

	InitCollisionModelType();	// CComboBox::SetCurSel doesn't call this

	if ( GetStyle() & WS_VISIBLE ) {
		// highlight the current body ingame
		cvarSystem->SetCVarString( "af_highlightBody", name );
	}
}

/*
================
DialogAFBody::SaveBody
================
*/
void DialogAFBody::SaveBody( void ) {
	int s1, s2;
	CString str;

	if ( !file || !body ) {
		return;
	}
	UpdateData( TRUE );

	// save the collision model to the current idDeclAF_Body
	cm_comboType.GetLBText( cm_comboType.GetCurSel(), str );
	body->modelType = StringToModelType( str );
	if ( body->modelType == TRM_BONE ) {
		body->origin.type = idAFVector::VEC_BONECENTER;
		s1 = GetSafeComboBoxSelection( &cm_comboBoneJoint1, str, -1 );
		body->v1.type = idAFVector::VEC_JOINT;
		body->v1.joint1 = str;
		body->origin.joint1 = str;
		s2 = GetSafeComboBoxSelection( &cm_comboBoneJoint2, str, s1 );
		body->v2.type = idAFVector::VEC_JOINT;
		body->v2.joint1 = str;
		body->origin.joint2 = str;
		body->width = cm_width;
		body->angles.Zero();
	} else {
		body->v1.type = idAFVector::VEC_COORDS;
		body->v1.ToVec3().x = -0.5f * cm_length;
		body->v1.ToVec3().y = -0.5f * cm_width;
		body->v1.ToVec3().z = -0.5f * cm_height;
		body->v2.type = idAFVector::VEC_COORDS;
		body->v2.ToVec3().x = 0.5f * cm_length;
		body->v2.ToVec3().y = 0.5f * cm_width;
		body->v2.ToVec3().z = 0.5f * cm_height;
		body->origin.ToVec3().x = cm_origin_x;
		body->origin.ToVec3().y = cm_origin_y;
		body->origin.ToVec3().z = cm_origin_z;
		body->angles.pitch = cm_angles_pitch;
		body->angles.yaw = cm_angles_yaw;
		body->angles.roll = cm_angles_roll;
		if ( body->origin.type == idAFVector::VEC_JOINT ) {
			s1 = GetSafeComboBoxSelection( &cm_originJoint, str, -1 );
			body->origin.joint1 = str;
		}
		else {
			s1 = GetSafeComboBoxSelection( &cm_originBoneCenterJoint1, str, -1 );
			body->origin.joint1 = str;
		}
		s2 = GetSafeComboBoxSelection( &cm_originBoneCenterJoint2, str, s1 );
		body->origin.joint2 = str;
	}
	body->numSides = cm_numSides;
	body->density = cm_density;
	cm_inertiaScale.GetWindowText( str );
	if ( idStr::Icmp( str, "none" ) == 0 ) {
		body->inertiaScale.Identity();
	} else {
		idLexer src( str, str.GetLength(), "inertiaScale" );
		src.SetFlags( LEXFL_NOERRORS | LEXFL_NOWARNINGS );
		for ( int i = 0; i < 3; i++ ) {
			for ( int j = 0; j < 3; j++ ) {
				body->inertiaScale[i][j] = src.ParseFloat();
			}
		}
	}

	// save the collision detection settings to the current idDeclAF_Body
	body->selfCollision = ( m_selfCollision != FALSE );
	m_editContents.GetWindowText( str );
	body->contents = idDeclAF::ContentsFromString( str );
	m_editClipMask.GetWindowText( str );
	body->clipMask = idDeclAF::ContentsFromString( str );

	// save friction settings to the current idDeclAF_Body
	body->linearFriction = m_linearFriction;
	body->angularFriction = m_angularFriction;
	body->contactFriction = m_contactFriction;

	// friction direction and contact motor direction
	m_frictionDirection.GetWindowText( str );
	if ( str.GetLength() != 0 ) {
		body->frictionDirection.ToVec3().Zero();
		sscanf( str, "%f %f %f", &body->frictionDirection.ToVec3().x, &body->frictionDirection.ToVec3().y, &body->frictionDirection.ToVec3().z );
	}
	m_contactMotorDirection.GetWindowText( str );
	if ( str.GetLength() != 0 ) {
		body->contactMotorDirection.ToVec3().Zero();
		sscanf( str, "%f %f %f", &body->contactMotorDirection.ToVec3().x, &body->contactMotorDirection.ToVec3().y, &body->contactMotorDirection.ToVec3().z );
	}

	// save joint settings to the current idDeclAF_Body
	GetSafeComboBoxSelection( &m_comboModifiedJoint, str, -1 );
	body->jointName = str;
	m_editContainedJoints.GetWindowText( str );
	body->containedJoints = str;

	AFDialogSetFileModified();
}

/*
================
DialogAFBody::UpdateFile
================
*/
void DialogAFBody::UpdateFile( void ) {
	SaveBody();
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAFBody::OnInitDialog
================
*/
BOOL DialogAFBody::OnInitDialog()  {

	CDialog::OnInitDialog();

	// initialize the collision model types
	cm_comboType.ResetContent();
	for ( int i = 0; modelTypes[i].name; i++ ) {
		if ( modelTypes[i].type == TRM_INVALID || modelTypes[i].type == TRM_CUSTOM ) {
			continue;
		}
		cm_comboType.AddString( modelTypes[i].name );
	}

	InitNewRenameDeleteButtons();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
================
DialogAFBody::OnToolHitTest
================
*/
int DialogAFBody::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CDialog::OnToolHitTest( point, pTI );
	return DefaultOnToolHitTest( toolTips, this, point, pTI );
}

BEGIN_MESSAGE_MAP(DialogAFBody, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_COMBO_BODIES, OnCbnSelchangeComboBodies)
	ON_BN_CLICKED(IDC_BUTTON_NEWBODY, OnBnClickedButtonNewbody)
	ON_BN_CLICKED(IDC_BUTTON_RENAMEBODY, OnBnClickedButtonRenamebody)
	ON_BN_CLICKED(IDC_BUTTON_DELETEBODY, OnBnClickedButtonDeletebody)
	ON_CBN_SELCHANGE(IDC_COMBO_CM_TYPE, OnCbnSelchangeComboCmType)
	ON_EN_CHANGE(IDC_EDIT_CM_LENGTH, OnEnChangeEditCmLength)
	ON_EN_CHANGE(IDC_EDIT_CM_HEIGHT, OnEnChangeEditCmHeight)
	ON_EN_CHANGE(IDC_EDIT_CM_WIDTH, OnEnChangeEditCmWidth)
	ON_EN_CHANGE(IDC_EDIT_CM_NUMSIDES, OnEnChangeEditCmNumsides)
	ON_CBN_SELCHANGE(IDC_COMBO_BONE_JOINT1, OnCbnSelchangeComboBoneJoint1)
	ON_CBN_SELCHANGE(IDC_COMBO_BONE_JOINT2, OnCbnSelchangeComboBoneJoint2)
	ON_EN_CHANGE(IDC_EDIT_CM_DENSITY, OnEnChangeEditCmDensity)
	ON_EN_CHANGE(IDC_EDIT_CM_INERTIASCALE, OnEnChangeEditCmInertiascale)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CM_LENGTH, OnDeltaposSpinCmLength)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CM_HEIGHT, OnDeltaposSpinCmHeight)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CM_WIDTH, OnDeltaposSpinCmWidth)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CM_NUMSIDES, OnDeltaposSpinCmNumsides)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CM_DENSITY, OnDeltaposSpinCmDensity)
	ON_BN_CLICKED(IDC_RADIO_ORIGIN_COORDINATES, OnBnClickedRadioOriginCoordinates)
	ON_BN_CLICKED(IDC_RADIO_ORIGIN_BONECENTER, OnBnClickedRadioOriginBonecenter)
	ON_BN_CLICKED(IDC_RADIO_ORIGIN_JOINT, OnBnClickedRadioOriginJoint)
	ON_EN_CHANGE(IDC_EDIT_AF_VECTOR_X, OnEnChangeEditAfVectorX)
	ON_EN_CHANGE(IDC_EDIT_AF_VECTOR_Y, OnEnChangeEditAfVectorY)
	ON_EN_CHANGE(IDC_EDIT_AF_VECTOR_Z, OnEnChangeEditAfVectorZ)
	ON_CBN_SELCHANGE(IDC_COMBO_ORIGIN_BONECENTER_JOINT1, OnOnCbnSelchangeComboOriginBoneCenterJoint1)
	ON_CBN_SELCHANGE(IDC_COMBO_ORIGIN_BONECENTER_JOINT2, OnOnCbnSelchangeComboOriginBoneCenterJoint2)
	ON_CBN_SELCHANGE(IDC_COMBO_ORIGIN_JOINT, OnOnCbnSelchangeComboOriginJoint)
	ON_EN_CHANGE(IDC_EDIT_ANGLES_PITCH, OnEnChangeEditAnglesPitch)
	ON_EN_CHANGE(IDC_EDIT_ANGLES_YAW, OnEnChangeEditAnglesYaw)
	ON_EN_CHANGE(IDC_EDIT_ANGLES_ROLL, OnEnChangeEditAnglesRoll)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_AF_VECTOR_X, OnDeltaposSpinAfVectorX)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_AF_VECTOR_Y, OnDeltaposSpinAfVectorY)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_AF_VECTOR_Z, OnDeltaposSpinAfVectorZ)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANGLES_PITCH, OnDeltaposSpinAnglesPitch)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANGLES_YAW, OnDeltaposSpinAnglesYaw)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANGLES_ROLL, OnDeltaposSpinAnglesRoll)
	ON_BN_CLICKED(IDC_CHECK_SELFCOLLISION, OnBnClickedCheckSelfcollision)
	ON_EN_CHANGE(IDC_EDIT_CONTENTS, OnEnChangeEditContents)
	ON_EN_CHANGE(IDC_EDIT_CLIPMASK, OnEnChangeEditClipmask)
	ON_EN_CHANGE(IDC_EDIT_LINEARFRICTION, OnEnChangeEditLinearfriction)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LINEARFRICTION, OnDeltaposSpinLinearfriction)
	ON_EN_CHANGE(IDC_EDIT_ANGULARFRICTION, OnEnChangeEditAngularfriction)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ANGULARFRICTION, OnDeltaposSpinAngularfriction)
	ON_EN_CHANGE(IDC_EDIT_CONTACTFRICTION, OnEnChangeEditContactfriction)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CONTACTFRICTION, OnDeltaposSpinContactfriction)
	ON_EN_CHANGE(IDC_EDIT_FRICTIONDIRECTION, OnEnChangeEditFrictionDirection)
	ON_EN_CHANGE(IDC_EDIT_CONTACTMOTORDIRECTION, OnEnChangeEditContactMotorDirection)
	ON_CBN_SELCHANGE(IDC_COMBO_MODIFIEDJOINT, OnCbnSelchangeComboModifiedjoint)
	ON_BN_CLICKED(IDC_RADIO_MODIFY_ORIENTATION, OnBnClickedRadioModifyOrientation)
	ON_BN_CLICKED(IDC_RADIO_MODIFY_POSITION, OnBnClickedRadioModifyPosition)
	ON_BN_CLICKED(IDC_RADIO_MODIFY_BOTH, OnBnClickedRadioModifyBoth)
	ON_EN_CHANGE(IDC_EDIT_CONTAINEDJOINTS, OnEnChangeEditContainedjoints)
END_MESSAGE_MAP()


// DialogAFBody message handlers

BOOL DialogAFBody::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

void DialogAFBody::OnShowWindow( BOOL bShow, UINT nStatus ) {
	if ( bShow && body ) {
		cvarSystem->SetCVarString( "af_highlightBody", body->name.c_str() );
	} else {
		cvarSystem->SetCVarString( "af_highlightBody", "" );
	}
	CDialog::OnShowWindow( bShow, nStatus );
}

void DialogAFBody::OnCbnSelchangeComboBodies() {
	CString str;

	GetSafeComboBoxSelection( &bodyList, str, -1 );
	LoadBody( str );
}

void DialogAFBody::OnBnClickedButtonNewbody() {
	DialogAFName nameDlg;
	CString str;
	INT_PTR res;

	// the names 'origin' and 'world' are reserved for constraints bound to the world
	bodyList.AddString( "origin" );
	bodyList.AddString( "world" );

	nameDlg.SetComboBox( &bodyList );
	res = nameDlg.DoModal();

	bodyList.DeleteString( bodyList.FindString( -1, "origin" ) );
	bodyList.DeleteString( bodyList.FindString( -1, "world" ) );

	if ( res == IDOK ) {
		nameDlg.GetName( str );
		// create new body
		file->NewBody( str );
		bodyList.SetCurSel( bodyList.AddString( str ) );
		LoadBody( str );
		constraintDlg->LoadFile( file );
		gameEdit->AF_UpdateEntities( file->GetName() );
		AFDialogSetFileModified();
	}
	InitNewRenameDeleteButtons();
}

void DialogAFBody::OnBnClickedButtonRenamebody() {
	int i;
	CString name, newName;
	DialogAFName nameDlg;

	if ( !file || !body ) {
		return;
	}

	i = bodyList.GetCurSel();
	if ( i != CB_ERR ) {
		bodyList.GetLBText( i, name );
		nameDlg.SetName( name );
		nameDlg.SetComboBox( &bodyList );
		if ( nameDlg.DoModal() == IDOK ) {
			nameDlg.GetName( newName );
			// rename body
			file->RenameBody( name, newName );
			bodyList.DeleteString( i );
			bodyList.SetCurSel( bodyList.AddString( newName ) );
			LoadBody( newName );
			constraintDlg->LoadFile( file );
			gameEdit->AF_UpdateEntities( file->GetName() );
			AFDialogSetFileModified();
		}
	}
}

void DialogAFBody::OnBnClickedButtonDeletebody() {
	int i;
	CString str;

	if ( !file || !body ) {
		return;
	}

	i = bodyList.GetCurSel();
	if ( i != CB_ERR ) {
		if ( MessageBox( "Are you sure you want to delete this body and all attached constraints ?", "Delete Body", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
			bodyList.GetLBText( i, str );
			// delete currently selected body
			file->DeleteBody( str );
			bodyList.DeleteString( i );
			body = NULL;
			OnCbnSelchangeComboBodies();
			constraintDlg->LoadFile( file );
			gameEdit->AF_UpdateEntities( file->GetName() );
			AFDialogSetFileModified();
		}
	}
	InitNewRenameDeleteButtons();
}

void DialogAFBody::OnCbnSelchangeComboCmType() {
	InitCollisionModelType();
	UpdateFile();
}

void DialogAFBody::ValidateCollisionModelLength( bool update ) {
	if ( cm_length < 1.0f ) {
		cm_length = 1.0f;
		update = true;
	}
	if ( update ) {
		UpdateData( FALSE );
	}
}

void DialogAFBody::OnEnChangeEditCmLength() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CM_LENGTH ) ) ) {
		ValidateCollisionModelLength( false );
		UpdateFile();
	}
	else {
		cm_length = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_CM_LENGTH ), false );
	}
}

void DialogAFBody::OnDeltaposSpinCmLength(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_length += 1.0f;
	}
	else if ( cm_length >= 2.0f ) {
		cm_length -= 1.0f;
	}
	ValidateCollisionModelLength( true );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::ValidateCollisionModelHeight( bool update ) {
	if ( cm_height < 1.0f ) {
		cm_height = 1.0f;
		update = true;
	}
	if ( update ) {
		UpdateData( FALSE );
	}
}

void DialogAFBody::OnEnChangeEditCmHeight() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CM_HEIGHT ) ) ) {
		ValidateCollisionModelHeight( false );
		UpdateFile();
	}
	else {
		cm_height = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_CM_HEIGHT ), false );
	}
}

void DialogAFBody::OnDeltaposSpinCmHeight(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_height += 1.0f;
	}
	else if ( cm_height >= 2.0f ) {
		cm_height -= 1.0f;
	}
	ValidateCollisionModelHeight( true );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::ValidateCollisionModelWidth( bool update ) {
	if ( cm_width < 1.0f ) {
		cm_width = 1.0f;
		update = true;
	}
	if ( update ) {
		UpdateData( FALSE );
	}
}

void DialogAFBody::OnEnChangeEditCmWidth() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CM_WIDTH ) ) ) {
		ValidateCollisionModelWidth( false );
		UpdateFile();
	}
	else {
		cm_width = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_CM_WIDTH ), false );
	}
}

void DialogAFBody::OnDeltaposSpinCmWidth(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_width += 1.0f;
	}
	else if ( cm_width >= 2.0f ) {
		cm_width -= 1.0f;
	}
	ValidateCollisionModelWidth( true );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::ValidateCollisionModelNumSides( bool update ) {
	cm_numSides = (int) cm_numSides;
	if ( cm_numSides < 3 ) {
		cm_numSides = 3;
		update = true;
	}
	else if ( cm_numSides > 10 ) {
		cm_numSides = 10;
		update = true;
	}
	if ( update ) {
		UpdateData( FALSE );
	}
}

void DialogAFBody::OnEnChangeEditCmNumsides() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CM_NUMSIDES ) ) ) {
		ValidateCollisionModelNumSides( false );
		UpdateFile();
	}
}

void DialogAFBody::OnDeltaposSpinCmNumsides(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_numSides += 1;
	}
	else if ( cm_numSides > 3 ) {
		cm_numSides -= 1;
	}
	ValidateCollisionModelNumSides( true );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnCbnSelchangeComboBoneJoint1() {
	CString str;
	GetSafeComboBoxSelection( &cm_comboBoneJoint1, str, -1 );
	UnsetSafeComboBoxSelection( &cm_comboBoneJoint2, str );
	UpdateFile();
}

void DialogAFBody::OnCbnSelchangeComboBoneJoint2() {
	CString str;
	GetSafeComboBoxSelection( &cm_comboBoneJoint2, str, -1 );
	UnsetSafeComboBoxSelection( &cm_comboBoneJoint1, str );
	UpdateFile();
}

void DialogAFBody::ValidateCollisionModelDensity( bool update ) {
	if ( cm_density < 1e-6f ) {
		cm_density = 1e-6f;
		update = true;
	}
	if ( update ) {
		UpdateData( FALSE );
	}
}

void DialogAFBody::OnEnChangeEditCmDensity() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CM_DENSITY ) ) ) {
		ValidateCollisionModelDensity( false );
		UpdateFile();
	}
	else {
		cm_density = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_CM_DENSITY ), false );
	}
}

void DialogAFBody::OnDeltaposSpinCmDensity(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	cm_density = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_CM_DENSITY ), pNMUpDown->iDelta < 0 );
	ValidateCollisionModelDensity( false );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnEnChangeEditCmInertiascale() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CM_INERTIASCALE ) ) ) {
		UpdateFile();
	}
}

void DialogAFBody::OnBnClickedRadioOriginCoordinates() {
	if ( IsDlgButtonChecked( IDC_RADIO_ORIGIN_COORDINATES ) ) {
		if ( body ) {
			body->origin.type = idAFVector::VEC_COORDS;
			UpdateFile();
		}
	}
}

void DialogAFBody::OnBnClickedRadioOriginBonecenter() {
	if ( IsDlgButtonChecked( IDC_RADIO_ORIGIN_BONECENTER ) ) {
		if ( body ) {
			body->origin.type = idAFVector::VEC_BONECENTER;
			UpdateFile();
		}
	}
}

void DialogAFBody::OnBnClickedRadioOriginJoint() {
	if ( IsDlgButtonChecked( IDC_RADIO_ORIGIN_JOINT ) ) {
		if ( body ) {
			body->origin.type = idAFVector::VEC_JOINT;
			UpdateFile();
		}
	}
}

void DialogAFBody::OnEnChangeEditAfVectorX() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_AF_VECTOR_X ) ) ) {
		UpdateFile();
	}
	else {
		cm_origin_x = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_AF_VECTOR_X ) );
	}
}

void DialogAFBody::OnDeltaposSpinAfVectorX(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_origin_x += 1;
	}
	else {
		cm_origin_x -= 1;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnEnChangeEditAfVectorY() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_AF_VECTOR_Y ) ) ) {
		UpdateFile();
	}
	else {
		cm_origin_y = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_AF_VECTOR_Y ) );
	}
}

void DialogAFBody::OnDeltaposSpinAfVectorY(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_origin_y += 1;
	}
	else {
		cm_origin_y -= 1;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnEnChangeEditAfVectorZ() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_AF_VECTOR_Z ) ) ) {
		UpdateFile();
	}
	else {
		cm_origin_z = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_AF_VECTOR_Z ) );
	}
}

void DialogAFBody::OnDeltaposSpinAfVectorZ(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_origin_z += 1;
	}
	else {
		cm_origin_z -= 1;
	}
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnOnCbnSelchangeComboOriginBoneCenterJoint1() {
	CString str;
	GetSafeComboBoxSelection( &cm_originBoneCenterJoint1, str, -1 );
	UnsetSafeComboBoxSelection( &cm_originBoneCenterJoint2, str );
	UpdateFile();
}

void DialogAFBody::OnOnCbnSelchangeComboOriginBoneCenterJoint2() {
	CString str;
	GetSafeComboBoxSelection( &cm_originBoneCenterJoint2, str, -1 );
	UnsetSafeComboBoxSelection( &cm_originBoneCenterJoint1, str );
	UpdateFile();
}

void DialogAFBody::OnOnCbnSelchangeComboOriginJoint() {
	UpdateFile();
}

void DialogAFBody::OnEnChangeEditAnglesPitch() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANGLES_PITCH ) ) ) {
		UpdateFile();
	}
	else {
		cm_angles_pitch = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANGLES_PITCH ) );
	}
}

void DialogAFBody::OnDeltaposSpinAnglesPitch(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_angles_pitch += 1;
	}
	else {
		cm_angles_pitch -= 1;
	}
	cm_angles_pitch = idMath::AngleNormalize360( cm_angles_pitch );
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnEnChangeEditAnglesYaw() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANGLES_YAW ) ) ) {
		UpdateFile();
	}
	else {
		cm_angles_yaw = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANGLES_YAW ) );
	}
}

void DialogAFBody::OnDeltaposSpinAnglesYaw(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_angles_yaw += 1;
	}
	else {
		cm_angles_yaw -= 1;
	}
	cm_angles_yaw = idMath::AngleNormalize360( cm_angles_yaw );
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnEnChangeEditAnglesRoll() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANGLES_ROLL ) ) ) {
		UpdateFile();
	}
	else {
		cm_angles_roll = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANGLES_ROLL ) );
	}
}

void DialogAFBody::OnDeltaposSpinAnglesRoll(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	if ( pNMUpDown->iDelta < 0 ) {
		cm_angles_roll += 1;
	}
	else {
		cm_angles_roll -= 1;
	}
	cm_angles_roll = idMath::AngleNormalize360( cm_angles_roll );
	UpdateData( FALSE );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnBnClickedCheckSelfcollision() {
	UpdateFile();
}

void DialogAFBody::OnEnChangeEditContents() {
	if ( EditControlEnterHit( &m_editContents ) ) {
		UpdateFile();
	}
}

void DialogAFBody::OnEnChangeEditClipmask() {
	if ( EditControlEnterHit( &m_editClipMask ) ) {
		UpdateFile();
	}
}

void DialogAFBody::OnEnChangeEditLinearfriction() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_LINEARFRICTION ) ) ) {
		UpdateFile();
	}
	else {
		m_linearFriction = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_LINEARFRICTION ), false );
	}
}

void DialogAFBody::OnDeltaposSpinLinearfriction(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_linearFriction = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_LINEARFRICTION ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnEnChangeEditAngularfriction() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_ANGULARFRICTION ) ) ) {
		UpdateFile();
	}
	else {
		m_angularFriction = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_ANGULARFRICTION ), false );
	}
}

void DialogAFBody::OnDeltaposSpinAngularfriction(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_angularFriction = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_ANGULARFRICTION ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnEnChangeEditContactfriction() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CONTACTFRICTION ) ) ) {
		UpdateFile();
	}
	else {
		m_contactFriction = EditVerifyFloat( (CEdit *) GetDlgItem( IDC_EDIT_CONTACTFRICTION ), false );
	}
}

void DialogAFBody::OnDeltaposSpinContactfriction(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	m_contactFriction = EditSpinFloat( (CEdit *)GetDlgItem( IDC_EDIT_CONTACTFRICTION ), pNMUpDown->iDelta < 0 );
	UpdateFile();
	*pResult = 0;
}

void DialogAFBody::OnEnChangeEditFrictionDirection() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_FRICTIONDIRECTION ) ) ) {
		UpdateFile();
	}
}

void DialogAFBody::OnEnChangeEditContactMotorDirection() {
	if ( EditControlEnterHit( (CEdit *) GetDlgItem( IDC_EDIT_CONTACTMOTORDIRECTION ) ) ) {
		UpdateFile();
	}
}

void DialogAFBody::OnCbnSelchangeComboModifiedjoint() {
	UpdateFile();
}

void DialogAFBody::OnBnClickedRadioModifyOrientation() {
	if ( IsDlgButtonChecked( IDC_RADIO_MODIFY_ORIENTATION ) ) {
		if ( body ) {
			body->jointMod = DECLAF_JOINTMOD_AXIS;
			UpdateFile();
		}
	}
}

void DialogAFBody::OnBnClickedRadioModifyPosition() {
	if ( IsDlgButtonChecked( IDC_RADIO_MODIFY_POSITION ) ) {
		if ( body ) {
			body->jointMod = DECLAF_JOINTMOD_ORIGIN;
			UpdateFile();
		}
	}
}

void DialogAFBody::OnBnClickedRadioModifyBoth() {
	if ( IsDlgButtonChecked( IDC_RADIO_MODIFY_BOTH ) ) {
		if ( body ) {
			body->jointMod = DECLAF_JOINTMOD_BOTH;
			UpdateFile();
		}
	}
}

void DialogAFBody::OnEnChangeEditContainedjoints() {
	if ( EditControlEnterHit( &m_editContainedJoints ) ) {
		UpdateFile();
	}
}

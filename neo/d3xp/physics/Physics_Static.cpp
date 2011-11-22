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

#include "../Game_local.h"

CLASS_DECLARATION( idPhysics, idPhysics_Static )
END_CLASS

/*
================
idPhysics_Static::idPhysics_Static
================
*/
idPhysics_Static::idPhysics_Static( void ) {
	self = NULL;
	clipModel = NULL;
	current.origin.Zero();
	current.axis.Identity();
	current.localOrigin.Zero();
	current.localAxis.Identity();
	hasMaster = false;
	isOrientated = false;
}

/*
================
idPhysics_Static::~idPhysics_Static
================
*/
idPhysics_Static::~idPhysics_Static( void ) {
	if ( self && self->GetPhysics() == this ) {
		self->SetPhysics( NULL );
	}
	idForce::DeletePhysics( this );
	if ( clipModel ) {
		delete clipModel;
	}
}

/*
================
idPhysics_Static::Save
================
*/
void idPhysics_Static::Save( idSaveGame *savefile ) const {
	savefile->WriteObject( self );

	savefile->WriteVec3( current.origin );
	savefile->WriteMat3( current.axis );
	savefile->WriteVec3( current.localOrigin );
	savefile->WriteMat3( current.localAxis );
	savefile->WriteClipModel( clipModel );

	savefile->WriteBool( hasMaster );
	savefile->WriteBool( isOrientated );
}

/*
================
idPhysics_Static::Restore
================
*/
void idPhysics_Static::Restore( idRestoreGame *savefile ) {
	savefile->ReadObject( reinterpret_cast<idClass *&>( self ) );

	savefile->ReadVec3( current.origin );
	savefile->ReadMat3( current.axis );
	savefile->ReadVec3( current.localOrigin );
	savefile->ReadMat3( current.localAxis );
	savefile->ReadClipModel( clipModel );

	savefile->ReadBool( hasMaster );
	savefile->ReadBool( isOrientated );
}

/*
================
idPhysics_Static::SetSelf
================
*/
void idPhysics_Static::SetSelf( idEntity *e ) {
	assert( e );
	self = e;
}

/*
================
idPhysics_Static::SetClipModel
================
*/
void idPhysics_Static::SetClipModel( idClipModel *model, float density, int id, bool freeOld ) {
	assert( self );

	if ( clipModel && clipModel != model && freeOld ) {
		delete clipModel;
	}
	clipModel = model;
	if ( clipModel ) {
		clipModel->Link( gameLocal.clip, self, 0, current.origin, current.axis );
	}
}

/*
================
idPhysics_Static::GetClipModel
================
*/
idClipModel *idPhysics_Static::GetClipModel( int id ) const {
	if ( clipModel ) {
		return clipModel;
	}
	return gameLocal.clip.DefaultClipModel();
}

/*
================
idPhysics_Static::GetNumClipModels
================
*/
int idPhysics_Static::GetNumClipModels( void ) const {
	return ( clipModel != NULL );
}

/*
================
idPhysics_Static::SetMass
================
*/
void idPhysics_Static::SetMass( float mass, int id ) {
}

/*
================
idPhysics_Static::GetMass
================
*/
float idPhysics_Static::GetMass( int id ) const {
	return 0.0f;
}

/*
================
idPhysics_Static::SetContents
================
*/
void idPhysics_Static::SetContents( int contents, int id ) {
	if ( clipModel ) {
		clipModel->SetContents( contents );
	}
}

/*
================
idPhysics_Static::GetContents
================
*/
int idPhysics_Static::GetContents( int id ) const {
	if ( clipModel ) {
		return clipModel->GetContents();
	}
	return 0;
}

/*
================
idPhysics_Static::SetClipMask
================
*/
void idPhysics_Static::SetClipMask( int mask, int id ) {
}

/*
================
idPhysics_Static::GetClipMask
================
*/
int idPhysics_Static::GetClipMask( int id ) const {
	return 0;
}

/*
================
idPhysics_Static::GetBounds
================
*/
const idBounds &idPhysics_Static::GetBounds( int id ) const {
	if ( clipModel ) {
		return clipModel->GetBounds();
	}
	return bounds_zero;
}

/*
================
idPhysics_Static::GetAbsBounds
================
*/
const idBounds &idPhysics_Static::GetAbsBounds( int id ) const {
	static idBounds absBounds;

	if ( clipModel ) {
		return clipModel->GetAbsBounds();
	}
	absBounds[0] = absBounds[1] = current.origin;
	return absBounds;
}

/*
================
idPhysics_Static::Evaluate
================
*/
bool idPhysics_Static::Evaluate( int timeStepMSec, int endTimeMSec ) {
	idVec3 masterOrigin, oldOrigin;
	idMat3 masterAxis, oldAxis;


	if ( hasMaster ) {
		oldOrigin = current.origin;
		oldAxis = current.axis;

		self->GetMasterPosition( masterOrigin, masterAxis );
		current.origin = masterOrigin + current.localOrigin * masterAxis;
		if ( isOrientated ) {
			current.axis = current.localAxis * masterAxis;
		} else {
			current.axis = current.localAxis;
		}
		if ( clipModel ) {
			clipModel->Link( gameLocal.clip, self, 0, current.origin, current.axis );
		}

		return ( current.origin != oldOrigin || current.axis != oldAxis );
	}
	return false;
}

/*
================
idPhysics_Static::UpdateTime
================
*/
void idPhysics_Static::UpdateTime( int endTimeMSec ) {
}

/*
================
idPhysics_Static::GetTime
================
*/
int idPhysics_Static::GetTime( void ) const {
	return 0;
}

/*
================
idPhysics_Static::GetImpactInfo
================
*/
void idPhysics_Static::GetImpactInfo( const int id, const idVec3 &point, impactInfo_t *info ) const {
	memset( info, 0, sizeof( *info ) );
}

/*
================
idPhysics_Static::ApplyImpulse
================
*/
void idPhysics_Static::ApplyImpulse( const int id, const idVec3 &point, const idVec3 &impulse ) {
}

/*
================
idPhysics_Static::AddForce
================
*/
void idPhysics_Static::AddForce( const int id, const idVec3 &point, const idVec3 &force ) {
}

/*
================
idPhysics_Static::Activate
================
*/
void idPhysics_Static::Activate( void ) {
}

/*
================
idPhysics_Static::PutToRest
================
*/
void idPhysics_Static::PutToRest( void ) {
}

/*
================
idPhysics_Static::IsAtRest
================
*/
bool idPhysics_Static::IsAtRest( void ) const {
	return true;
}

/*
================
idPhysics_Static::GetRestStartTime
================
*/
int idPhysics_Static::GetRestStartTime( void ) const {
	return 0;
}

/*
================
idPhysics_Static::IsPushable
================
*/
bool idPhysics_Static::IsPushable( void ) const {
	return false;
}

/*
================
idPhysics_Static::SaveState
================
*/
void idPhysics_Static::SaveState( void ) {
}

/*
================
idPhysics_Static::RestoreState
================
*/
void idPhysics_Static::RestoreState( void ) {
}

/*
================
idPhysics_Static::SetOrigin
================
*/
void idPhysics_Static::SetOrigin( const idVec3 &newOrigin, int id ) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	current.localOrigin = newOrigin;

	if ( hasMaster ) {
		self->GetMasterPosition( masterOrigin, masterAxis );
		current.origin = masterOrigin + newOrigin * masterAxis;
	} else {
		current.origin = newOrigin;
	}

	if ( clipModel ) {
		clipModel->Link( gameLocal.clip, self, 0, current.origin, current.axis );
	}
}

/*
================
idPhysics_Static::SetAxis
================
*/
void idPhysics_Static::SetAxis( const idMat3 &newAxis, int id ) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	current.localAxis = newAxis;

	if ( hasMaster && isOrientated ) {
		self->GetMasterPosition( masterOrigin, masterAxis );
		current.axis = newAxis * masterAxis;
	} else {
		current.axis = newAxis;
	}

	if ( clipModel ) {
		clipModel->Link( gameLocal.clip, self, 0, current.origin, current.axis );
	}
}

/*
================
idPhysics_Static::Translate
================
*/
void idPhysics_Static::Translate( const idVec3 &translation, int id ) {
	current.localOrigin += translation;
	current.origin += translation;

	if ( clipModel ) {
		clipModel->Link( gameLocal.clip, self, 0, current.origin, current.axis );
	}
}

/*
================
idPhysics_Static::Rotate
================
*/
void idPhysics_Static::Rotate( const idRotation &rotation, int id ) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	current.origin *= rotation;
	current.axis *= rotation.ToMat3();

	if ( hasMaster ) {
		self->GetMasterPosition( masterOrigin, masterAxis );
		current.localAxis *= rotation.ToMat3();
		current.localOrigin = ( current.origin - masterOrigin ) * masterAxis.Transpose();
	} else {
		current.localAxis = current.axis;
		current.localOrigin = current.origin;
	}

	if ( clipModel ) {
		clipModel->Link( gameLocal.clip, self, 0, current.origin, current.axis );
	}
}

/*
================
idPhysics_Static::GetOrigin
================
*/
const idVec3 &idPhysics_Static::GetOrigin( int id ) const {
	return current.origin;
}

/*
================
idPhysics_Static::GetAxis
================
*/
const idMat3 &idPhysics_Static::GetAxis( int id ) const {
	return current.axis;
}

/*
================
idPhysics_Static::SetLinearVelocity
================
*/
void idPhysics_Static::SetLinearVelocity( const idVec3 &newLinearVelocity, int id ) {
}

/*
================
idPhysics_Static::SetAngularVelocity
================
*/
void idPhysics_Static::SetAngularVelocity( const idVec3 &newAngularVelocity, int id ) {
}

/*
================
idPhysics_Static::GetLinearVelocity
================
*/
const idVec3 &idPhysics_Static::GetLinearVelocity( int id ) const {
	return vec3_origin;
}

/*
================
idPhysics_Static::GetAngularVelocity
================
*/
const idVec3 &idPhysics_Static::GetAngularVelocity( int id ) const {
	return vec3_origin;
}

/*
================
idPhysics_Static::SetGravity
================
*/
void idPhysics_Static::SetGravity( const idVec3 &newGravity ) {
}

/*
================
idPhysics_Static::GetGravity
================
*/
const idVec3 &idPhysics_Static::GetGravity( void ) const {
	static idVec3 gravity( 0, 0, -g_gravity.GetFloat() );
	return gravity;
}

/*
================
idPhysics_Static::GetGravityNormal
================
*/
const idVec3 &idPhysics_Static::GetGravityNormal( void ) const {
	static idVec3 gravity( 0, 0, -1 );
	return gravity;
}

/*
================
idPhysics_Static::ClipTranslation
================
*/
void idPhysics_Static::ClipTranslation( trace_t &results, const idVec3 &translation, const idClipModel *model ) const {
	if ( model ) {
		gameLocal.clip.TranslationModel( results, current.origin, current.origin + translation,
			clipModel, current.axis, MASK_SOLID, model->Handle(), model->GetOrigin(), model->GetAxis() );
	} else {
		gameLocal.clip.Translation( results, current.origin, current.origin + translation,
			clipModel, current.axis, MASK_SOLID, self );
	}	
}

/*
================
idPhysics_Static::ClipRotation
================
*/
void idPhysics_Static::ClipRotation( trace_t &results, const idRotation &rotation, const idClipModel *model ) const {
	if ( model ) {
		gameLocal.clip.RotationModel( results, current.origin, rotation,
			clipModel, current.axis, MASK_SOLID, model->Handle(), model->GetOrigin(), model->GetAxis() );
	} else {
		gameLocal.clip.Rotation( results, current.origin, rotation, clipModel, current.axis, MASK_SOLID, self );
	}
}

/*
================
idPhysics_Static::ClipContents
================
*/
int idPhysics_Static::ClipContents( const idClipModel *model ) const {
	if ( clipModel ) {
		if ( model ) {
			return gameLocal.clip.ContentsModel( clipModel->GetOrigin(), clipModel, clipModel->GetAxis(), -1,
				model->Handle(), model->GetOrigin(), model->GetAxis() );
		} else {
			return gameLocal.clip.Contents( clipModel->GetOrigin(), clipModel, clipModel->GetAxis(), -1, NULL );
		}
	}
	return 0;
}

/*
================
idPhysics_Static::DisableClip
================
*/
void idPhysics_Static::DisableClip( void ) {
	if ( clipModel ) {
		clipModel->Disable();
	}
}

/*
================
idPhysics_Static::EnableClip
================
*/
void idPhysics_Static::EnableClip( void ) {
	if ( clipModel ) {
		clipModel->Enable();
	}
}

/*
================
idPhysics_Static::UnlinkClip
================
*/
void idPhysics_Static::UnlinkClip( void ) {
	if ( clipModel ) {
		clipModel->Unlink();
	}
}

/*
================
idPhysics_Static::LinkClip
================
*/
void idPhysics_Static::LinkClip( void ) {
	if ( clipModel ) {
		clipModel->Link( gameLocal.clip, self, 0, current.origin, current.axis );
	}
}

/*
================
idPhysics_Static::EvaluateContacts
================
*/
bool idPhysics_Static::EvaluateContacts( void ) {
	return false;
}

/*
================
idPhysics_Static::GetNumContacts
================
*/
int idPhysics_Static::GetNumContacts( void ) const {
	return 0;
}

/*
================
idPhysics_Static::GetContact
================
*/
const contactInfo_t &idPhysics_Static::GetContact( int num ) const {
	static contactInfo_t info;
	memset( &info, 0, sizeof( info ) );
	return info;
}

/*
================
idPhysics_Static::ClearContacts
================
*/
void idPhysics_Static::ClearContacts( void ) {
}

/*
================
idPhysics_Static::AddContactEntity
================
*/
void idPhysics_Static::AddContactEntity( idEntity *e ) {
}

/*
================
idPhysics_Static::RemoveContactEntity
================
*/
void idPhysics_Static::RemoveContactEntity( idEntity *e ) {
}

/*
================
idPhysics_Static::HasGroundContacts
================
*/
bool idPhysics_Static::HasGroundContacts( void ) const {
	return false;
}

/*
================
idPhysics_Static::IsGroundEntity
================
*/
bool idPhysics_Static::IsGroundEntity( int entityNum ) const {
	return false;
}

/*
================
idPhysics_Static::IsGroundClipModel
================
*/
bool idPhysics_Static::IsGroundClipModel( int entityNum, int id ) const {
	return false;
}

/*
================
idPhysics_Static::SetPushed
================
*/
void idPhysics_Static::SetPushed( int deltaTime ) {
}

/*
================
idPhysics_Static::GetPushedLinearVelocity
================
*/
const idVec3 &idPhysics_Static::GetPushedLinearVelocity( const int id ) const {
	return vec3_origin;
}

/*
================
idPhysics_Static::GetPushedAngularVelocity
================
*/
const idVec3 &idPhysics_Static::GetPushedAngularVelocity( const int id ) const {
	return vec3_origin;
}

/*
================
idPhysics_Static::SetMaster
================
*/
void idPhysics_Static::SetMaster( idEntity *master, const bool orientated ) {
	idVec3 masterOrigin;
	idMat3 masterAxis;

	if ( master ) {
		if ( !hasMaster ) {
			// transform from world space to master space
			self->GetMasterPosition( masterOrigin, masterAxis );
			current.localOrigin = ( current.origin - masterOrigin ) * masterAxis.Transpose();
			if ( orientated ) {
				current.localAxis = current.axis * masterAxis.Transpose();
			} else {
				current.localAxis = current.axis;
			}
			hasMaster = true;
			isOrientated = orientated;
		}
	} else {
		if ( hasMaster ) {
			hasMaster = false;
		}
	}
}

/*
================
idPhysics_Static::GetBlockingInfo
================
*/
const trace_t *idPhysics_Static::GetBlockingInfo( void ) const {
	return NULL;
}

/*
================
idPhysics_Static::GetBlockingEntity
================
*/
idEntity *idPhysics_Static::GetBlockingEntity( void ) const {
	return NULL;
}

/*
================
idPhysics_Static::GetLinearEndTime
================
*/
int idPhysics_Static::GetLinearEndTime( void ) const {
	return 0;
}

/*
================
idPhysics_Static::GetAngularEndTime
================
*/
int idPhysics_Static::GetAngularEndTime( void ) const {
	return 0;
}

/*
================
idPhysics_Static::WriteToSnapshot
================
*/
void idPhysics_Static::WriteToSnapshot( idBitMsgDelta &msg ) const {
	idCQuat quat, localQuat;

	quat = current.axis.ToCQuat();
	localQuat = current.localAxis.ToCQuat();

	msg.WriteFloat( current.origin[0] );
	msg.WriteFloat( current.origin[1] );
	msg.WriteFloat( current.origin[2] );
	msg.WriteFloat( quat.x );
	msg.WriteFloat( quat.y );
	msg.WriteFloat( quat.z );
	msg.WriteDeltaFloat( current.origin[0], current.localOrigin[0] );
	msg.WriteDeltaFloat( current.origin[1], current.localOrigin[1] );
	msg.WriteDeltaFloat( current.origin[2], current.localOrigin[2] );
	msg.WriteDeltaFloat( quat.x, localQuat.x );
	msg.WriteDeltaFloat( quat.y, localQuat.y );
	msg.WriteDeltaFloat( quat.z, localQuat.z );
}

/*
================
idPhysics_Base::ReadFromSnapshot
================
*/
void idPhysics_Static::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	idCQuat quat, localQuat;

	current.origin[0] = msg.ReadFloat();
	current.origin[1] = msg.ReadFloat();
	current.origin[2] = msg.ReadFloat();
	quat.x = msg.ReadFloat();
	quat.y = msg.ReadFloat();
	quat.z = msg.ReadFloat();
	current.localOrigin[0] = msg.ReadDeltaFloat( current.origin[0] );
	current.localOrigin[1] = msg.ReadDeltaFloat( current.origin[1] );
	current.localOrigin[2] = msg.ReadDeltaFloat( current.origin[2] );
	localQuat.x = msg.ReadDeltaFloat( quat.x );
	localQuat.y = msg.ReadDeltaFloat( quat.y );
	localQuat.z = msg.ReadDeltaFloat( quat.z );

	current.axis = quat.ToMat3();
	current.localAxis = localQuat.ToMat3();
}

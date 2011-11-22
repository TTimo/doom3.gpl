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

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"


CLASS_DECLARATION( idEntity, idBrittleFracture )
	EVENT( EV_Activate, idBrittleFracture::Event_Activate )
	EVENT( EV_Touch, idBrittleFracture::Event_Touch )
END_CLASS

const int SHARD_ALIVE_TIME	= 5000;
const int SHARD_FADE_START	= 2000;

static const char *brittleFracture_SnapshotName = "_BrittleFracture_Snapshot_";

/*
================
idBrittleFracture::idBrittleFracture
================
*/
idBrittleFracture::idBrittleFracture( void ) {
	material = NULL;
	decalMaterial = NULL;
	decalSize = 0.0f;
	maxShardArea = 0.0f;
	maxShatterRadius = 0.0f;
	minShatterRadius = 0.0f;
	linearVelocityScale = 0.0f;
	angularVelocityScale = 0.0f;
	shardMass = 0.0f;
	density = 0.0f;
	friction = 0.0f;
	bouncyness = 0.0f;
	fxFracture.Clear();

	bounds.Clear();
	disableFracture = false;

	lastRenderEntityUpdate = -1;
	changed = false;

	fl.networkSync = true;
}

/*
================
idBrittleFracture::~idBrittleFracture
================
*/
idBrittleFracture::~idBrittleFracture( void ) {
	int i;

	for ( i = 0; i < shards.Num(); i++ ) {
		shards[i]->decals.DeleteContents( true );
		delete shards[i];
	}

	// make sure the render entity is freed before the model is freed
	FreeModelDef();
	renderModelManager->FreeModel( renderEntity.hModel );
}

/*
================
idBrittleFracture::Save
================
*/
void idBrittleFracture::Save( idSaveGame *savefile ) const {
	int i, j;

	savefile->WriteInt( health );
	entityFlags_s flags = fl;
	LittleBitField( &flags, sizeof( flags ) );
	savefile->Write( &flags, sizeof( flags ) );
	
	// setttings
	savefile->WriteMaterial( material );
	savefile->WriteMaterial( decalMaterial );
	savefile->WriteFloat( decalSize );
	savefile->WriteFloat( maxShardArea );
	savefile->WriteFloat( maxShatterRadius );
	savefile->WriteFloat( minShatterRadius );
	savefile->WriteFloat( linearVelocityScale );
	savefile->WriteFloat( angularVelocityScale );
	savefile->WriteFloat( shardMass );
	savefile->WriteFloat( density );
	savefile->WriteFloat( friction );
	savefile->WriteFloat( bouncyness );
	savefile->WriteString( fxFracture );

	// state
	savefile->WriteBounds( bounds );
	savefile->WriteBool( disableFracture );

	savefile->WriteInt( lastRenderEntityUpdate );
	savefile->WriteBool( changed );

	savefile->WriteStaticObject( physicsObj );

	savefile->WriteInt( shards.Num() );
	for ( i = 0; i < shards.Num(); i++ ) {
		savefile->WriteWinding( shards[i]->winding );

		savefile->WriteInt( shards[i]->decals.Num() );
		for ( j = 0; j < shards[i]->decals.Num(); j++ ) {
			savefile->WriteWinding( *shards[i]->decals[j] );
		}

		savefile->WriteInt( shards[i]->neighbours.Num() );
		for ( j = 0; j < shards[i]->neighbours.Num(); j++ ) {
			int index = shards.FindIndex(shards[i]->neighbours[j]);
			assert(index != -1);
			savefile->WriteInt( index );
		}

		savefile->WriteInt( shards[i]->edgeHasNeighbour.Num() );
		for ( j = 0; j < shards[i]->edgeHasNeighbour.Num(); j++ ) {
			savefile->WriteBool( shards[i]->edgeHasNeighbour[j] );
		}

		savefile->WriteInt( shards[i]->droppedTime );
		savefile->WriteInt( shards[i]->islandNum );
		savefile->WriteBool( shards[i]->atEdge );
		savefile->WriteStaticObject( shards[i]->physicsObj );
	}
}

/*
================
idBrittleFracture::Restore
================
*/
void idBrittleFracture::Restore( idRestoreGame *savefile ) {
	int i, j , num;

	renderEntity.hModel = renderModelManager->AllocModel();
	renderEntity.hModel->InitEmpty( brittleFracture_SnapshotName );
	renderEntity.callback = idBrittleFracture::ModelCallback;
	renderEntity.noShadow = true;
	renderEntity.noSelfShadow = true;
	renderEntity.noDynamicInteractions = false;

	savefile->ReadInt( health );
	savefile->Read( &fl, sizeof( fl ) );
	LittleBitField( &fl, sizeof( fl ) );

	// setttings
	savefile->ReadMaterial( material );
	savefile->ReadMaterial( decalMaterial );
	savefile->ReadFloat( decalSize );
	savefile->ReadFloat( maxShardArea );
	savefile->ReadFloat( maxShatterRadius );
	savefile->ReadFloat( minShatterRadius );
	savefile->ReadFloat( linearVelocityScale );
	savefile->ReadFloat( angularVelocityScale );
	savefile->ReadFloat( shardMass );
	savefile->ReadFloat( density );
	savefile->ReadFloat( friction );
	savefile->ReadFloat( bouncyness );
	savefile->ReadString( fxFracture );

	// state
	savefile->ReadBounds(bounds);
	savefile->ReadBool( disableFracture );

	savefile->ReadInt( lastRenderEntityUpdate );
	savefile->ReadBool( changed );

	savefile->ReadStaticObject( physicsObj );
	RestorePhysics( &physicsObj );

	savefile->ReadInt( num );
	shards.SetNum( num );
	for ( i = 0; i < num; i++ ) {
		shards[i] = new shard_t;
	}

	for ( i = 0; i < num; i++ ) {
		savefile->ReadWinding( shards[i]->winding );

		savefile->ReadInt( j );
		shards[i]->decals.SetNum( j );
		for ( j = 0; j < shards[i]->decals.Num(); j++ ) {
			shards[i]->decals[j] = new idFixedWinding;
			savefile->ReadWinding( *shards[i]->decals[j] );
		}

		savefile->ReadInt( j );
		shards[i]->neighbours.SetNum( j );
		for ( j = 0; j < shards[i]->neighbours.Num(); j++ ) {
			int index;
			savefile->ReadInt( index );
			assert(index != -1);
			shards[i]->neighbours[j] = shards[index];
		}

		savefile->ReadInt( j );
		shards[i]->edgeHasNeighbour.SetNum( j );
		for ( j = 0; j < shards[i]->edgeHasNeighbour.Num(); j++ ) {
			savefile->ReadBool( shards[i]->edgeHasNeighbour[j] );
		}

		savefile->ReadInt( shards[i]->droppedTime );
		savefile->ReadInt( shards[i]->islandNum );
		savefile->ReadBool( shards[i]->atEdge );
		savefile->ReadStaticObject( shards[i]->physicsObj );
		if ( shards[i]->droppedTime < 0 ) {
			shards[i]->clipModel = physicsObj.GetClipModel( i );
		} else {
			shards[i]->clipModel = shards[i]->physicsObj.GetClipModel();
		}
	}
}

/*
================
idBrittleFracture::Spawn
================
*/
void idBrittleFracture::Spawn( void ) {

	// get shard properties
	decalMaterial = declManager->FindMaterial( spawnArgs.GetString( "mtr_decal" ) );
	decalSize = spawnArgs.GetFloat( "decalSize", "40" );
	maxShardArea = spawnArgs.GetFloat( "maxShardArea", "200" );
	maxShardArea = idMath::ClampFloat( 100, 10000, maxShardArea );
	maxShatterRadius = spawnArgs.GetFloat( "maxShatterRadius", "40" );
	minShatterRadius = spawnArgs.GetFloat( "minShatterRadius", "10" );
	linearVelocityScale = spawnArgs.GetFloat( "linearVelocityScale", "0.1" );
	angularVelocityScale = spawnArgs.GetFloat( "angularVelocityScale", "40" );
	fxFracture = spawnArgs.GetString( "fx" );

	// get rigid body properties
	shardMass = spawnArgs.GetFloat( "shardMass", "20" );
	shardMass = idMath::ClampFloat( 0.001f, 1000.0f, shardMass );
	spawnArgs.GetFloat( "density", "0.1", density );
	density = idMath::ClampFloat( 0.001f, 1000.0f, density );
	spawnArgs.GetFloat( "friction", "0.4", friction );
	friction = idMath::ClampFloat( 0.0f, 1.0f, friction );
	spawnArgs.GetFloat( "bouncyness", "0.01", bouncyness );
	bouncyness = idMath::ClampFloat( 0.0f, 1.0f, bouncyness );

	disableFracture = spawnArgs.GetBool( "disableFracture", "0" );
	health = spawnArgs.GetInt( "health", "40" );
	fl.takedamage = true;

	// FIXME: set "bleed" so idProjectile calls AddDamageEffect
	spawnArgs.SetBool( "bleed", 1 );

	CreateFractures( renderEntity.hModel );

	FindNeighbours();

	renderEntity.hModel = renderModelManager->AllocModel();
	renderEntity.hModel->InitEmpty( brittleFracture_SnapshotName );
	renderEntity.callback = idBrittleFracture::ModelCallback;
	renderEntity.noShadow = true;
	renderEntity.noSelfShadow = true;
	renderEntity.noDynamicInteractions = false;
}

/*
================
idBrittleFracture::AddShard
================
*/
void idBrittleFracture::AddShard( idClipModel *clipModel, idFixedWinding &w ) {
	shard_t *shard = new shard_t;
	shard->clipModel = clipModel;
	shard->droppedTime = -1;
	shard->winding = w;
	shard->decals.Clear();
	shard->edgeHasNeighbour.AssureSize( w.GetNumPoints(), false );
	shard->neighbours.Clear();
	shard->atEdge = false;
	shards.Append( shard );
}

/*
================
idBrittleFracture::RemoveShard
================
*/
void idBrittleFracture::RemoveShard( int index ) {
	int i;

	delete shards[index];
	shards.RemoveIndex( index );
	physicsObj.RemoveIndex( index );

	for ( i = index; i < shards.Num(); i++ ) {
		shards[i]->clipModel->SetId( i );
	}
}

/*
================
idBrittleFracture::UpdateRenderEntity
================
*/
bool idBrittleFracture::UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView ) const {
	int i, j, k, n, msec, numTris, numDecalTris;
	float fade;
	dword packedColor;
	srfTriangles_t *tris, *decalTris;
	modelSurface_t surface;
	idDrawVert *v;
	idPlane plane;
	idMat3 tangents;

	// this may be triggered by a model trace or other non-view related source,
	// to which we should look like an empty model
	if ( !renderView ) {
		return false;
	}

	// don't regenerate it if it is current
	if ( lastRenderEntityUpdate == gameLocal.time || !changed ) {
		return false;
	}

	lastRenderEntityUpdate = gameLocal.time;
	changed = false;

	numTris = 0;
	numDecalTris = 0;
	for ( i = 0; i < shards.Num(); i++ ) {
		n = shards[i]->winding.GetNumPoints();
		if ( n > 2 ) {
			numTris += n - 2;
		}
		for ( k = 0; k < shards[i]->decals.Num(); k++ ) {
			n = shards[i]->decals[k]->GetNumPoints();
			if ( n > 2 ) {
				numDecalTris += n - 2;
			}
		}
	}

	// FIXME: re-use model surfaces
	renderEntity->hModel->InitEmpty( brittleFracture_SnapshotName );

	// allocate triangle surfaces for the fractures and decals
	tris = renderEntity->hModel->AllocSurfaceTriangles( numTris * 3, material->ShouldCreateBackSides() ? numTris * 6 : numTris * 3 );
	decalTris = renderEntity->hModel->AllocSurfaceTriangles( numDecalTris * 3, decalMaterial->ShouldCreateBackSides() ? numDecalTris * 6 : numDecalTris * 3 );

	for ( i = 0; i < shards.Num(); i++ ) {
		const idVec3 &origin = shards[i]->clipModel->GetOrigin();
		const idMat3 &axis = shards[i]->clipModel->GetAxis();

		fade = 1.0f;
		if ( shards[i]->droppedTime >= 0 ) {
			msec = gameLocal.time - shards[i]->droppedTime - SHARD_FADE_START;
			if ( msec > 0 ) {
				fade = 1.0f - (float) msec / ( SHARD_ALIVE_TIME - SHARD_FADE_START );
			}
		}
		packedColor = PackColor( idVec4( renderEntity->shaderParms[ SHADERPARM_RED ] * fade,
										renderEntity->shaderParms[ SHADERPARM_GREEN ] * fade,
                                        renderEntity->shaderParms[ SHADERPARM_BLUE ] * fade,
										fade ) );

		const idWinding &winding = shards[i]->winding;

		winding.GetPlane( plane );
		tangents = ( plane.Normal() * axis ).ToMat3();

		for ( j = 2; j < winding.GetNumPoints(); j++ ) {

			v = &tris->verts[tris->numVerts++];
			v->Clear();
			v->xyz = origin + winding[0].ToVec3() * axis;
			v->st[0] = winding[0].s;
			v->st[1] = winding[0].t;
			v->normal = tangents[0];
			v->tangents[0] = tangents[1];
			v->tangents[1] = tangents[2];
			v->SetColor( packedColor );

			v = &tris->verts[tris->numVerts++];
			v->Clear();
			v->xyz = origin + winding[j-1].ToVec3() * axis;
			v->st[0] = winding[j-1].s;
			v->st[1] = winding[j-1].t;
			v->normal = tangents[0];
			v->tangents[0] = tangents[1];
			v->tangents[1] = tangents[2];
			v->SetColor( packedColor );

			v = &tris->verts[tris->numVerts++];
			v->Clear();
			v->xyz = origin + winding[j].ToVec3() * axis;
			v->st[0] = winding[j].s;
			v->st[1] = winding[j].t;
			v->normal = tangents[0];
			v->tangents[0] = tangents[1];
			v->tangents[1] = tangents[2];
			v->SetColor( packedColor );

			tris->indexes[tris->numIndexes++] = tris->numVerts - 3;
			tris->indexes[tris->numIndexes++] = tris->numVerts - 2;
			tris->indexes[tris->numIndexes++] = tris->numVerts - 1;

			if ( material->ShouldCreateBackSides() ) {

				tris->indexes[tris->numIndexes++] = tris->numVerts - 2;
				tris->indexes[tris->numIndexes++] = tris->numVerts - 3;
				tris->indexes[tris->numIndexes++] = tris->numVerts - 1;
			}
		}

		for ( k = 0; k < shards[i]->decals.Num(); k++ ) {
			const idWinding &decalWinding = *shards[i]->decals[k];

			for ( j = 2; j < decalWinding.GetNumPoints(); j++ ) {

				v = &decalTris->verts[decalTris->numVerts++];
				v->Clear();
				v->xyz = origin + decalWinding[0].ToVec3() * axis;
				v->st[0] = decalWinding[0].s;
				v->st[1] = decalWinding[0].t;
				v->normal = tangents[0];
				v->tangents[0] = tangents[1];
				v->tangents[1] = tangents[2];
				v->SetColor( packedColor );

				v = &decalTris->verts[decalTris->numVerts++];
				v->Clear();
				v->xyz = origin + decalWinding[j-1].ToVec3() * axis;
				v->st[0] = decalWinding[j-1].s;
				v->st[1] = decalWinding[j-1].t;
				v->normal = tangents[0];
				v->tangents[0] = tangents[1];
				v->tangents[1] = tangents[2];
				v->SetColor( packedColor );

				v = &decalTris->verts[decalTris->numVerts++];
				v->Clear();
				v->xyz = origin + decalWinding[j].ToVec3() * axis;
				v->st[0] = decalWinding[j].s;
				v->st[1] = decalWinding[j].t;
				v->normal = tangents[0];
				v->tangents[0] = tangents[1];
				v->tangents[1] = tangents[2];
				v->SetColor( packedColor );

				decalTris->indexes[decalTris->numIndexes++] = decalTris->numVerts - 3;
				decalTris->indexes[decalTris->numIndexes++] = decalTris->numVerts - 2;
				decalTris->indexes[decalTris->numIndexes++] = decalTris->numVerts - 1;

				if ( decalMaterial->ShouldCreateBackSides() ) {

					decalTris->indexes[decalTris->numIndexes++] = decalTris->numVerts - 2;
					decalTris->indexes[decalTris->numIndexes++] = decalTris->numVerts - 3;
					decalTris->indexes[decalTris->numIndexes++] = decalTris->numVerts - 1;
				}
			}
		}
	}

	tris->tangentsCalculated = true;
	decalTris->tangentsCalculated = true;

	SIMDProcessor->MinMax( tris->bounds[0], tris->bounds[1], tris->verts, tris->numVerts );
	SIMDProcessor->MinMax( decalTris->bounds[0], decalTris->bounds[1], decalTris->verts, decalTris->numVerts );

	memset( &surface, 0, sizeof( surface ) );
	surface.shader = material;
	surface.id = 0;
	surface.geometry = tris;
	renderEntity->hModel->AddSurface( surface );

	memset( &surface, 0, sizeof( surface ) );
	surface.shader = decalMaterial;
	surface.id = 1;
	surface.geometry = decalTris;
	renderEntity->hModel->AddSurface( surface );

	return true;
}

/*
================
idBrittleFracture::ModelCallback
================
*/
bool idBrittleFracture::ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView ) {
	const idBrittleFracture *ent;

	ent = static_cast<idBrittleFracture *>(gameLocal.entities[ renderEntity->entityNum ]);
	if ( !ent ) {
		gameLocal.Error( "idBrittleFracture::ModelCallback: callback with NULL game entity" );
	}

	return ent->UpdateRenderEntity( renderEntity, renderView );
}

/*
================
idBrittleFracture::Present
================
*/
void idBrittleFracture::Present() {

	// don't present to the renderer if the entity hasn't changed
	if ( !( thinkFlags & TH_UPDATEVISUALS ) ) {
		return;
	}
	BecomeInactive( TH_UPDATEVISUALS );

	renderEntity.bounds = bounds;
	renderEntity.origin.Zero();
	renderEntity.axis.Identity();

	// force an update because the bounds/origin/axis may stay the same while the model changes
	renderEntity.forceUpdate = true;

	// add to refresh list
	if ( modelDefHandle == -1 ) {
		modelDefHandle = gameRenderWorld->AddEntityDef( &renderEntity );
	} else {
		gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );
	}

	changed = true;
}

/*
================
idBrittleFracture::Think
================
*/
void idBrittleFracture::Think( void ) {
	int i, startTime, endTime, droppedTime;
	shard_t *shard;
	bool atRest = true, fading = false;

	// remove overdue shards
	for ( i = 0; i < shards.Num(); i++ ) {
		droppedTime = shards[i]->droppedTime;
		if ( droppedTime != -1 ) {
			if ( gameLocal.time - droppedTime > SHARD_ALIVE_TIME ) {
				RemoveShard( i );
				i--;
			}
			fading = true;
		}
	}

	// remove the entity when nothing is visible
	if ( !shards.Num() ) {
		PostEventMS( &EV_Remove, 0 );
		return;
	}

	if ( thinkFlags & TH_PHYSICS ) {

		startTime = gameLocal.previousTime;
		endTime = gameLocal.time;

		// run physics on shards
		for ( i = 0; i < shards.Num(); i++ ) {
			shard = shards[i];

			if ( shard->droppedTime == -1 ) {
				continue;
			}

			shard->physicsObj.Evaluate( endTime - startTime, endTime );

			if ( !shard->physicsObj.IsAtRest() ) {
				atRest = false;
			}
		}

		if ( atRest ) {
			BecomeInactive( TH_PHYSICS );
		} else {
			BecomeActive( TH_PHYSICS );
		}
	}

	if ( !atRest || bounds.IsCleared() ) {
		bounds.Clear();
		for ( i = 0; i < shards.Num(); i++ ) {
			bounds.AddBounds( shards[i]->clipModel->GetAbsBounds() );
		}
	}

	if ( fading ) {
		BecomeActive( TH_UPDATEVISUALS | TH_THINK );
	} else {
		BecomeInactive( TH_THINK );
	}

	RunPhysics();
	Present();
}

/*
================
idBrittleFracture::ApplyImpulse
================
*/
void idBrittleFracture::ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse ) {

	if ( id < 0 || id >= shards.Num() ) {
		return;
	}

	if ( shards[id]->droppedTime != -1 ) {
		shards[id]->physicsObj.ApplyImpulse( 0, point, impulse );
	} else if ( health <= 0 && !disableFracture ) {
		Shatter( point, impulse, gameLocal.time );
	}
}

/*
================
idBrittleFracture::AddForce
================
*/
void idBrittleFracture::AddForce( idEntity *ent, int id, const idVec3 &point, const idVec3 &force ) {

	if ( id < 0 || id >= shards.Num() ) {
		return;
	}

	if ( shards[id]->droppedTime != -1 ) {
		shards[id]->physicsObj.AddForce( 0, point, force );
	} else if ( health <= 0 && !disableFracture ) {
		Shatter( point, force, gameLocal.time );
	}
}

/*
================
idBrittleFracture::ProjectDecal
================
*/
void idBrittleFracture::ProjectDecal( const idVec3 &point, const idVec3 &dir, const int time, const char *damageDefName ) {
	int i, j, bits, clipBits;
	float a, c, s;
	idVec2 st[MAX_POINTS_ON_WINDING];
	idVec3 origin;
	idMat3 axis, axistemp;
	idPlane textureAxis[2];

	if ( gameLocal.isServer ) {
		idBitMsg	msg;
		byte		msgBuf[MAX_EVENT_PARAM_SIZE];

		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.BeginWriting();
		msg.WriteFloat( point[0] );
		msg.WriteFloat( point[1] );
		msg.WriteFloat( point[2] );
		msg.WriteFloat( dir[0] );
		msg.WriteFloat( dir[1] );
		msg.WriteFloat( dir[2] );
		ServerSendEvent( EVENT_PROJECT_DECAL, &msg, true, -1 );
	}

	if ( time >= gameLocal.time ) {
		// try to get the sound from the damage def
		const idDeclEntityDef *damageDef = NULL;
		const idSoundShader *sndShader = NULL;
		if ( damageDefName ) {
			damageDef = gameLocal.FindEntityDef( damageDefName, false );
			if ( damageDef ) {
				sndShader = declManager->FindSound( damageDef->dict.GetString( "snd_shatter", "" ) );
			}
		}

		if ( sndShader ) {
			StartSoundShader( sndShader, SND_CHANNEL_ANY, 0, false, NULL );
		} else {
			StartSound( "snd_bullethole", SND_CHANNEL_ANY, 0, false, NULL );
		}
	}

	a = gameLocal.random.RandomFloat() * idMath::TWO_PI;
	c = cos( a );
	s = -sin( a );

	axis[2] = -dir;
	axis[2].Normalize();
	axis[2].NormalVectors( axistemp[0], axistemp[1] );
	axis[0] = axistemp[ 0 ] * c + axistemp[ 1 ] * s;
	axis[1] = axistemp[ 0 ] * s + axistemp[ 1 ] * -c;

	textureAxis[0] = axis[0] * ( 1.0f / decalSize );
	textureAxis[0][3] = -( point * textureAxis[0].Normal() ) + 0.5f;

	textureAxis[1] = axis[1] * ( 1.0f / decalSize );
	textureAxis[1][3] = -( point * textureAxis[1].Normal() ) + 0.5f;

	for ( i = 0; i < shards.Num(); i++ ) {
		idFixedWinding &winding = shards[i]->winding;
		origin = shards[i]->clipModel->GetOrigin();
		axis = shards[i]->clipModel->GetAxis();
		float d0, d1;

		clipBits = -1;
		for ( j = 0; j < winding.GetNumPoints(); j++ ) {
			idVec3 p = origin + winding[j].ToVec3() * axis;

			st[j].x = d0 = textureAxis[0].Distance( p );
			st[j].y = d1 = textureAxis[1].Distance( p );

			bits = FLOATSIGNBITSET( d0 );
			d0 = 1.0f - d0;
			bits |= FLOATSIGNBITSET( d1 ) << 2;
			d1 = 1.0f - d1;
			bits |= FLOATSIGNBITSET( d0 ) << 1;
			bits |= FLOATSIGNBITSET( d1 ) << 3;

			clipBits &= bits;
		}

		if ( clipBits ) {
			continue;
		}

		idFixedWinding *decal = new idFixedWinding;
		shards[i]->decals.Append( decal );

		decal->SetNumPoints( winding.GetNumPoints() );
		for ( j = 0; j < winding.GetNumPoints(); j++ ) {
			(*decal)[j].ToVec3() = winding[j].ToVec3();
			(*decal)[j].s = st[j].x;
			(*decal)[j].t = st[j].y;
		}
	}

	BecomeActive( TH_UPDATEVISUALS );
}

/*
================
idBrittleFracture::DropShard
================
*/
void idBrittleFracture::DropShard( shard_t *shard, const idVec3 &point, const idVec3 &dir, const float impulse, const int time ) {
	int i, j, clipModelId;
	float dist, f;
	idVec3 dir2, origin;
	idMat3 axis;
	shard_t *neighbour;

	// don't display decals on dropped shards
	shard->decals.DeleteContents( true );

	// remove neighbour pointers of neighbours pointing to this shard
	for ( i = 0; i < shard->neighbours.Num(); i++ ) {
		neighbour = shard->neighbours[i];
		for ( j = 0; j < neighbour->neighbours.Num(); j++ ) {
			if ( neighbour->neighbours[j] == shard ) {
				neighbour->neighbours.RemoveIndex( j );
				break;
			}
		}
	}

	// remove neighbour pointers
	shard->neighbours.Clear();

	// remove the clip model from the static physics object
	clipModelId = shard->clipModel->GetId();
	physicsObj.SetClipModel( NULL, 1.0f, clipModelId, false );

	origin = shard->clipModel->GetOrigin();
	axis = shard->clipModel->GetAxis();

	// set the dropped time for fading
	shard->droppedTime = time;

	dir2 = origin - point;
	dist = dir2.Normalize();
	f = dist > maxShatterRadius ? 1.0f : idMath::Sqrt( dist - minShatterRadius ) * ( 1.0f / idMath::Sqrt( maxShatterRadius - minShatterRadius ) );

	// setup the physics
	shard->physicsObj.SetSelf( this );
	shard->physicsObj.SetClipModel( shard->clipModel, density );
	shard->physicsObj.SetMass( shardMass );
	shard->physicsObj.SetOrigin( origin );
	shard->physicsObj.SetAxis( axis );
	shard->physicsObj.SetBouncyness( bouncyness );
	shard->physicsObj.SetFriction( 0.6f, 0.6f, friction );
	shard->physicsObj.SetGravity( gameLocal.GetGravity() );
	shard->physicsObj.SetContents( CONTENTS_RENDERMODEL );
	shard->physicsObj.SetClipMask( MASK_SOLID | CONTENTS_MOVEABLECLIP );
	shard->physicsObj.ApplyImpulse( 0, origin, impulse * linearVelocityScale * dir );
	shard->physicsObj.SetAngularVelocity( dir.Cross( dir2 ) * ( f * angularVelocityScale ) );

	shard->clipModel->SetId( clipModelId );

	BecomeActive( TH_PHYSICS );
}

/*
================
idBrittleFracture::Shatter
================
*/
void idBrittleFracture::Shatter( const idVec3 &point, const idVec3 &impulse, const int time ) {
	int i;
	idVec3 dir;
	shard_t *shard;
	float m;

	if ( gameLocal.isServer ) {
		idBitMsg	msg;
		byte		msgBuf[MAX_EVENT_PARAM_SIZE];

		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.BeginWriting();
		msg.WriteFloat( point[0] );
		msg.WriteFloat( point[1] );
		msg.WriteFloat( point[2] );
		msg.WriteFloat( impulse[0] );
		msg.WriteFloat( impulse[1] );
		msg.WriteFloat( impulse[2] );
		ServerSendEvent( EVENT_SHATTER, &msg, true, -1 );
	}

	if ( time > ( gameLocal.time - SHARD_ALIVE_TIME ) ) {
		StartSound( "snd_shatter", SND_CHANNEL_ANY, 0, false, NULL );
	}

	if ( !IsBroken() ) {
		Break();
	}

	if ( fxFracture.Length() ) {
		idEntityFx::StartFx( fxFracture, &point, &GetPhysics()->GetAxis(), this, true );
	}

	dir = impulse;
	m = dir.Normalize();

	for ( i = 0; i < shards.Num(); i++ ) {
		shard = shards[i];

		if ( shard->droppedTime != -1 ) {
			continue;
		}

		if ( ( shard->clipModel->GetOrigin() - point ).LengthSqr() > Square( maxShatterRadius ) ) {
			continue;
		}

		DropShard( shard, point, dir, m, time );
	}

	DropFloatingIslands( point, impulse, time );
}

/*
================
idBrittleFracture::DropFloatingIslands
================
*/
void idBrittleFracture::DropFloatingIslands( const idVec3 &point, const idVec3 &impulse, const int time ) {
	int i, j, numIslands;
	int queueStart, queueEnd;
	shard_t *curShard, *nextShard, **queue;
	bool touchesEdge;
	idVec3 dir;

	dir = impulse;
	dir.Normalize();

	numIslands = 0;
	queue = (shard_t **) _alloca16( shards.Num() * sizeof(shard_t **) );
	for ( i = 0; i < shards.Num(); i++ ) {
		shards[i]->islandNum = 0;
	}

	for ( i = 0; i < shards.Num(); i++ ) {

		if ( shards[i]->droppedTime != -1 ) {
			continue;
		}

		if ( shards[i]->islandNum ) {
			continue;
		}

        queueStart = 0;
		queueEnd = 1;
		queue[0] = shards[i];
		shards[i]->islandNum = numIslands+1;
		touchesEdge = false;

		if ( shards[i]->atEdge ) {
			touchesEdge = true;
		}

		for ( curShard = queue[queueStart]; queueStart < queueEnd; curShard = queue[++queueStart] ) {

			for ( j = 0; j < curShard->neighbours.Num(); j++ ) {

				nextShard = curShard->neighbours[j];

				if ( nextShard->droppedTime != -1 ) {
					continue;
				}

				if ( nextShard->islandNum ) {
					continue;
				}

				queue[queueEnd++] = nextShard;
				nextShard->islandNum = numIslands+1;

				if ( nextShard->atEdge ) {
					touchesEdge = true;
				}
			}
		}
		numIslands++;

		// if the island is not connected to the world at any edges
		if ( !touchesEdge ) {
			for ( j = 0; j < queueEnd; j++ ) {
				DropShard( queue[j], point, dir, 0.0f, time );
			}
		}
	}
}

/*
================
idBrittleFracture::Break
================
*/
void idBrittleFracture::Break( void ) {
	fl.takedamage = false;
	physicsObj.SetContents( CONTENTS_RENDERMODEL | CONTENTS_TRIGGER );
}

/*
================
idBrittleFracture::IsBroken
================
*/
bool idBrittleFracture::IsBroken( void ) const {
	return ( fl.takedamage == false );
}

/*
================
idBrittleFracture::Killed
================
*/
void idBrittleFracture::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	if ( !disableFracture ) {
		ActivateTargets( this );
		Break();
	}
}

/*
================
idBrittleFracture::AddDamageEffect
================
*/
void idBrittleFracture::AddDamageEffect( const trace_t &collision, const idVec3 &velocity, const char *damageDefName ) {
	if ( !disableFracture ) {
		ProjectDecal( collision.c.point, collision.c.normal, gameLocal.time, damageDefName );
	}
}

/*
================
idBrittleFracture::Fracture_r
================
*/
void idBrittleFracture::Fracture_r( idFixedWinding &w ) {
	int i, j, bestPlane;
	float a, c, s, dist, bestDist;
	idVec3 origin;
	idPlane windingPlane, splitPlanes[2];
	idMat3 axis, axistemp;
	idFixedWinding back;
	idTraceModel trm;
	idClipModel *clipModel;

	while( 1 ) {
		origin = w.GetCenter();
		w.GetPlane( windingPlane );

		if ( w.GetArea() < maxShardArea ) {
			break;
		}

		// randomly create a split plane
		a = gameLocal.random.RandomFloat() * idMath::TWO_PI;
		c = cos( a );
		s = -sin( a );
		axis[2] = windingPlane.Normal();
		axis[2].NormalVectors( axistemp[0], axistemp[1] );
		axis[0] = axistemp[ 0 ] * c + axistemp[ 1 ] * s;
		axis[1] = axistemp[ 0 ] * s + axistemp[ 1 ] * -c;

		// get the best split plane
		bestDist = 0.0f;
		bestPlane = 0;
		for ( i = 0; i < 2; i++ ) {
			splitPlanes[i].SetNormal( axis[i] );
			splitPlanes[i].FitThroughPoint( origin );
			for ( j = 0; j < w.GetNumPoints(); j++ ) {
				dist = splitPlanes[i].Distance( w[j].ToVec3() );
				if ( dist > bestDist ) {
					bestDist = dist;
					bestPlane = i;
				}
			}
		}

		// split the winding
		if ( !w.Split( &back, splitPlanes[bestPlane] ) ) {
			break;
		}

		// recursively create shards for the back winding
		Fracture_r( back );
	}

	// translate the winding to it's center
	origin = w.GetCenter();
	for ( j = 0; j < w.GetNumPoints(); j++ ) {
		w[j].ToVec3() -= origin;
	}
	w.RemoveEqualPoints();

	trm.SetupPolygon( w );
	trm.Shrink( CM_CLIP_EPSILON );
	clipModel = new idClipModel( trm );

	physicsObj.SetClipModel( clipModel, 1.0f, shards.Num() );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() + origin, shards.Num() );
	physicsObj.SetAxis( GetPhysics()->GetAxis(), shards.Num() );

	AddShard( clipModel, w );
}

/*
================
idBrittleFracture::CreateFractures
================
*/
void idBrittleFracture::CreateFractures( const idRenderModel *renderModel ) {
	int i, j, k;
	const modelSurface_t *surf;
	const idDrawVert *v;
	idFixedWinding w;

	if ( !renderModel ) {
		return;
	}

	physicsObj.SetSelf( this );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin(), 0 );
	physicsObj.SetAxis( GetPhysics()->GetAxis(), 0 );

	for ( i = 0; i < 1 /*renderModel->NumSurfaces()*/; i++ ) {
		surf = renderModel->Surface( i );
		material = surf->shader;

		for ( j = 0; j < surf->geometry->numIndexes; j += 3 ) {
			w.Clear();
			for ( k = 0; k < 3; k++ ) {
				v = &surf->geometry->verts[ surf->geometry->indexes[ j + 2 - k ] ];
				w.AddPoint( v->xyz );
				w[k].s = v->st[0];
				w[k].t = v->st[1];
			}
			Fracture_r( w );
		}
	}

	physicsObj.SetContents( material->GetContentFlags() );
	SetPhysics( &physicsObj );
}

/*
================
idBrittleFracture::FindNeighbours
================
*/
void idBrittleFracture::FindNeighbours( void ) {
	int i, j, k, l;
	idVec3 p1, p2, dir;
	idMat3 axis;
	idPlane plane[4];

	for ( i = 0; i < shards.Num(); i++ ) {

		shard_t *shard1 = shards[i];
		const idWinding &w1 = shard1->winding;
		const idVec3 &origin1 = shard1->clipModel->GetOrigin();
		const idMat3 &axis1 = shard1->clipModel->GetAxis();

		for ( k = 0; k < w1.GetNumPoints(); k++ ) {

			p1 = origin1 + w1[k].ToVec3() * axis1;
			p2 = origin1 + w1[(k+1)%w1.GetNumPoints()].ToVec3() * axis1;
			dir = p2 - p1;
			dir.Normalize();
			axis = dir.ToMat3();

			plane[0].SetNormal( dir );
			plane[0].FitThroughPoint( p1 );
			plane[1].SetNormal( -dir );
			plane[1].FitThroughPoint( p2 );
			plane[2].SetNormal( axis[1] );
			plane[2].FitThroughPoint( p1 );
			plane[3].SetNormal( axis[2] );
			plane[3].FitThroughPoint( p1 );

			for ( j = 0; j < shards.Num(); j++ ) {

				if ( i == j ) {
					continue;
				}

				shard_t *shard2 = shards[j];

				for ( l = 0; l < shard1->neighbours.Num(); l++ ) {
					if ( shard1->neighbours[l] == shard2 ) {
						break;
					}
				}
				if ( l < shard1->neighbours.Num() ) {
					continue;
				}

				const idWinding &w2 = shard2->winding;
				const idVec3 &origin2 = shard2->clipModel->GetOrigin();
				const idMat3 &axis2 = shard2->clipModel->GetAxis();

				for ( l = w2.GetNumPoints()-1; l >= 0; l-- ) {
					p1 = origin2 + w2[l].ToVec3() * axis2;
					p2 = origin2 + w2[(l-1+w2.GetNumPoints())%w2.GetNumPoints()].ToVec3() * axis2;
					if ( plane[0].Side( p2, 0.1f ) == SIDE_FRONT && plane[1].Side( p1, 0.1f ) == SIDE_FRONT ) {
						if ( plane[2].Side( p1, 0.1f ) == SIDE_ON && plane[3].Side( p1, 0.1f ) == SIDE_ON ) {
							if ( plane[2].Side( p2, 0.1f ) == SIDE_ON && plane[3].Side( p2, 0.1f ) == SIDE_ON ) {
								shard1->neighbours.Append( shard2 );
								shard1->edgeHasNeighbour[k] = true;
								shard2->neighbours.Append( shard1 );
								shard2->edgeHasNeighbour[(l-1+w2.GetNumPoints())%w2.GetNumPoints()] = true;
								break;
							}
						}
					}
				}
			}
		}

		for ( k = 0; k < w1.GetNumPoints(); k++ ) {
			if ( !shard1->edgeHasNeighbour[k] ) {
				break;
			}
		}
		if ( k < w1.GetNumPoints() ) {
			shard1->atEdge = true;
		} else {
			shard1->atEdge = false;
		}
	}
}

/*
================
idBrittleFracture::Event_Activate
================
*/
void idBrittleFracture::Event_Activate( idEntity *activator ) {
	disableFracture = false;
	if ( health <= 0 ) {
		Break();
	}
}

/*
================
idBrittleFracture::Event_Touch
================
*/
void idBrittleFracture::Event_Touch( idEntity *other, trace_t *trace ) {
	idVec3 point, impulse;

	if ( !IsBroken() ) {
		return;
	}

	if ( trace->c.id < 0 || trace->c.id >= shards.Num() ) {
		return;
	}

	point = shards[trace->c.id]->clipModel->GetOrigin();
	impulse = other->GetPhysics()->GetLinearVelocity() * other->GetPhysics()->GetMass();

	Shatter( point, impulse, gameLocal.time );
}

/*
================
idBrittleFracture::ClientPredictionThink
================
*/
void idBrittleFracture::ClientPredictionThink( void ) {
	// only think forward because the state is not synced through snapshots
	if ( !gameLocal.isNewFrame ) {
		return;
	}

	Think();
}

/*
================
idBrittleFracture::ClientReceiveEvent
================
*/
bool idBrittleFracture::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {
	idVec3 point, dir;

	switch( event ) {
		case EVENT_PROJECT_DECAL: {
			point[0] = msg.ReadFloat();
			point[1] = msg.ReadFloat();
			point[2] = msg.ReadFloat();
			dir[0] = msg.ReadFloat();
			dir[1] = msg.ReadFloat();
			dir[2] = msg.ReadFloat();
			ProjectDecal( point, dir, time, NULL );
			return true;
		}
		case EVENT_SHATTER: {
			point[0] = msg.ReadFloat();
			point[1] = msg.ReadFloat();
			point[2] = msg.ReadFloat();
			dir[0] = msg.ReadFloat();
			dir[1] = msg.ReadFloat();
			dir[2] = msg.ReadFloat();
			Shatter( point, dir, time );
			return true;
		}
		default: {
			return idEntity::ClientReceiveEvent( event, time, msg );
		}
	}
	return false;
}

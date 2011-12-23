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


/*
===============================================================================

  idItem

===============================================================================
*/

const idEventDef EV_DropToFloor( "<dropToFloor>" );
const idEventDef EV_RespawnItem( "respawn" );
const idEventDef EV_RespawnFx( "<respawnFx>" );
const idEventDef EV_GetPlayerPos( "<getplayerpos>" );
const idEventDef EV_HideObjective( "<hideobjective>", "e" );
const idEventDef EV_CamShot( "<camshot>" );

CLASS_DECLARATION( idEntity, idItem )
	EVENT( EV_DropToFloor,		idItem::Event_DropToFloor )
	EVENT( EV_Touch,			idItem::Event_Touch )
	EVENT( EV_Activate,			idItem::Event_Trigger )
	EVENT( EV_RespawnItem,		idItem::Event_Respawn )
	EVENT( EV_RespawnFx,		idItem::Event_RespawnFx )
END_CLASS


/*
================
idItem::idItem
================
*/
idItem::idItem() {
	spin = false;
	inView = false;
	inViewTime = 0;
	lastCycle = 0;
	lastRenderViewTime = -1;
	itemShellHandle = -1;
	shellMaterial = NULL;
	orgOrigin.Zero();
	canPickUp = true;
	fl.networkSync = true;
}

/*
================
idItem::~idItem
================
*/
idItem::~idItem() {
	// remove the highlight shell
	if ( itemShellHandle != -1 ) {
		gameRenderWorld->FreeEntityDef( itemShellHandle );
	}
}

/*
================
idItem::Save
================
*/
void idItem::Save( idSaveGame *savefile ) const {

	savefile->WriteVec3( orgOrigin );
	savefile->WriteBool( spin );
	savefile->WriteBool( pulse );
	savefile->WriteBool( canPickUp );

	savefile->WriteMaterial( shellMaterial );

	savefile->WriteBool( inView );
	savefile->WriteInt( inViewTime );
	savefile->WriteInt( lastCycle );
	savefile->WriteInt( lastRenderViewTime );
}

/*
================
idItem::Restore
================
*/
void idItem::Restore( idRestoreGame *savefile ) {

	savefile->ReadVec3( orgOrigin );
	savefile->ReadBool( spin );
	savefile->ReadBool( pulse );
	savefile->ReadBool( canPickUp );

	savefile->ReadMaterial( shellMaterial );

	savefile->ReadBool( inView );
	savefile->ReadInt( inViewTime );
	savefile->ReadInt( lastCycle );
	savefile->ReadInt( lastRenderViewTime );

	itemShellHandle = -1;
}

/*
================
idItem::UpdateRenderEntity
================
*/
bool idItem::UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView ) const {

	if ( lastRenderViewTime == renderView->time ) {
		return false;
	}

	lastRenderViewTime = renderView->time;

	// check for glow highlighting if near the center of the view
	idVec3 dir = renderEntity->origin - renderView->vieworg;
	dir.Normalize();
	float d = dir * renderView->viewaxis[0];

	// two second pulse cycle
	float cycle = ( renderView->time - inViewTime ) / 2000.0f;

	if ( d > 0.94f ) {
		if ( !inView ) {
			inView = true;
			if ( cycle > lastCycle ) {
				// restart at the beginning
				inViewTime = renderView->time;
				cycle = 0.0f;
			}
		}
	} else {
		if ( inView ) {
			inView = false;
			lastCycle = ceil( cycle );
		}
	}

	// fade down after the last pulse finishes 
	if ( !inView && cycle > lastCycle ) {
		renderEntity->shaderParms[4] = 0.0f;
	} else {
		// pulse up in 1/4 second
		cycle -= (int)cycle;
		if ( cycle < 0.1f ) {
			renderEntity->shaderParms[4] = cycle * 10.0f;
		} else if ( cycle < 0.2f ) {
			renderEntity->shaderParms[4] = 1.0f;
		} else if ( cycle < 0.3f ) {
			renderEntity->shaderParms[4] = 1.0f - ( cycle - 0.2f ) * 10.0f;
		} else {
			// stay off between pulses
			renderEntity->shaderParms[4] = 0.0f;
		}
	}

	// update every single time this is in view
	return true;
}

/*
================
idItem::ModelCallback
================
*/
bool idItem::ModelCallback( renderEntity_t *renderEntity, const renderView_t *renderView ) {
	const idItem *ent;

	// this may be triggered by a model trace or other non-view related source
	if ( !renderView ) {
		return false;
	}

	ent = static_cast<idItem *>(gameLocal.entities[ renderEntity->entityNum ]);
	if ( !ent ) {
		gameLocal.Error( "idItem::ModelCallback: callback with NULL game entity" );
	}

	return ent->UpdateRenderEntity( renderEntity, renderView );
}

/*
================
idItem::Think
================
*/
void idItem::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		if ( spin ) {
			idAngles	ang;
			idVec3		org;

			ang.pitch = ang.roll = 0.0f;
			ang.yaw = ( gameLocal.time & 4095 ) * 360.0f / -4096.0f;
			SetAngles( ang );

			float scale = 0.005f + entityNumber * 0.00001f;
			
			org = orgOrigin;
			org.z += 4.0f + cos( ( gameLocal.time + 2000 ) * scale ) * 4.0f;
			SetOrigin( org );
		}
	}

	Present();
}

/*
================
idItem::Present
================
*/
void idItem::Present( void ) {
	idEntity::Present();

	if ( !fl.hidden && pulse ) {
		// also add a highlight shell model
		renderEntity_t	shell;

		shell = renderEntity;

		// we will mess with shader parms when the item is in view
		// to give the "item pulse" effect
		shell.callback = idItem::ModelCallback;
		shell.entityNum = entityNumber;
		shell.customShader = shellMaterial;
		if ( itemShellHandle == -1 ) {
			itemShellHandle = gameRenderWorld->AddEntityDef( &shell );
		} else {
			gameRenderWorld->UpdateEntityDef( itemShellHandle, &shell );
		}

	}
}

/*
================
idItem::Spawn
================
*/
void idItem::Spawn( void ) {
	idStr		giveTo;
	idEntity *	ent;
	float		tsize;

	if ( spawnArgs.GetBool( "dropToFloor" ) ) {
		PostEventMS( &EV_DropToFloor, 0 );
	}

	if ( spawnArgs.GetFloat( "triggersize", "0", tsize ) ) {
		GetPhysics()->GetClipModel()->LoadModel( idTraceModel( idBounds( vec3_origin ).Expand( tsize ) ) );
		GetPhysics()->GetClipModel()->Link( gameLocal.clip );
	}

	if ( spawnArgs.GetBool( "start_off" ) ) {
		GetPhysics()->SetContents( 0 );
		Hide();
	} else {
		GetPhysics()->SetContents( CONTENTS_TRIGGER );
	}

	giveTo = spawnArgs.GetString( "owner" );
	if ( giveTo.Length() ) {
		ent = gameLocal.FindEntity( giveTo );
		if ( !ent ) {
			gameLocal.Error( "Item couldn't find owner '%s'", giveTo.c_str() );
		}
		PostEventMS( &EV_Touch, 0, ent, 0 );
	}

#ifdef CTF
	// idItemTeam does not rotate and bob
	if ( spawnArgs.GetBool( "spin" ) || (gameLocal.isMultiplayer && !this->IsType( idItemTeam::Type ) ) ) {
		spin = true;
		BecomeActive( TH_THINK );
    }
#else
	if ( spawnArgs.GetBool( "spin" ) || gameLocal.isMultiplayer ) {
		spin = true;
		BecomeActive( TH_THINK );
	}
#endif

	//pulse = !spawnArgs.GetBool( "nopulse" );
	//temp hack for tim
	pulse = false;
	orgOrigin = GetPhysics()->GetOrigin();

	canPickUp = !( spawnArgs.GetBool( "triggerFirst" ) || spawnArgs.GetBool( "no_touch" ) );

	inViewTime = -1000;
	lastCycle = -1;
	itemShellHandle = -1;
	shellMaterial = declManager->FindMaterial( "itemHighlightShell" );
}

/*
================
idItem::GetAttributes
================
*/
void idItem::GetAttributes( idDict &attributes ) {
	int					i;
	const idKeyValue	*arg;

	for( i = 0; i < spawnArgs.GetNumKeyVals(); i++ ) {
		arg = spawnArgs.GetKeyVal( i );
		if ( arg->GetKey().Left( 4 ) == "inv_" ) {
			attributes.Set( arg->GetKey().Right( arg->GetKey().Length() - 4 ), arg->GetValue() );
		}
	}
}

/*
================
idItem::GiveToPlayer
================
*/
bool idItem::GiveToPlayer( idPlayer *player ) {
	if ( player == NULL ) {
		return false;
	}

	if ( spawnArgs.GetBool( "inv_carry" ) ) {
		return player->GiveInventoryItem( &spawnArgs );
	} 
	
	return player->GiveItem( this );
}

/*
================
idItem::Pickup
================
*/
bool idItem::Pickup( idPlayer *player ) {
	
	if ( !GiveToPlayer( player ) ) {
		return false;
	}

	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_PICKUP, NULL, false, -1 );
	}

	// play pickup sound
	StartSound( "snd_acquire", SND_CHANNEL_ITEM, 0, false, NULL );

	// trigger our targets
	ActivateTargets( player );

	// clear our contents so the object isn't picked up twice
	GetPhysics()->SetContents( 0 );

	// hide the model
	Hide();

	// add the highlight shell
	if ( itemShellHandle != -1 ) {
		gameRenderWorld->FreeEntityDef( itemShellHandle );
		itemShellHandle = -1;
	}

	float respawn = spawnArgs.GetFloat( "respawn" );
	bool dropped = spawnArgs.GetBool( "dropped" );
	bool no_respawn = spawnArgs.GetBool( "no_respawn" );

	if ( gameLocal.isMultiplayer && respawn == 0.0f ) {
		respawn = 20.0f;
	}

	if ( respawn && !dropped && !no_respawn ) {
		const char *sfx = spawnArgs.GetString( "fxRespawn" );
		if ( sfx && *sfx ) {
			PostEventSec( &EV_RespawnFx, respawn - 0.5f );
		} 
		PostEventSec( &EV_RespawnItem, respawn );
	} else if ( !spawnArgs.GetBool( "inv_objective" ) && !no_respawn ) {
		// give some time for the pickup sound to play
		// FIXME: Play on the owner
		if ( !spawnArgs.GetBool( "inv_carry" ) ) {
			PostEventMS( &EV_Remove, 5000 );
		}
	}

	BecomeInactive( TH_THINK );
	return true;
}

/*
================
idItem::ClientPredictionThink
================
*/
void idItem::ClientPredictionThink( void ) {
	// only think forward because the state is not synced through snapshots
	if ( !gameLocal.isNewFrame ) {
		return;
	}
	Think();
}

/*
================
idItem::WriteFromSnapshot
================
*/
void idItem::WriteToSnapshot( idBitMsgDelta &msg ) const {
	msg.WriteBits( IsHidden(), 1 );
}

/*
================
idItem::ReadFromSnapshot
================
*/
void idItem::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	if ( msg.ReadBits( 1 ) ) {
		Hide();
	} else {
		Show();
	}
}

/*
================
idItem::ClientReceiveEvent
================
*/
bool idItem::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {

	switch( event ) {
		case EVENT_PICKUP: {

			// play pickup sound
			StartSound( "snd_acquire", SND_CHANNEL_ITEM, 0, false, NULL );

			// hide the model
			Hide();

			// remove the highlight shell
			if ( itemShellHandle != -1 ) {
				gameRenderWorld->FreeEntityDef( itemShellHandle );
				itemShellHandle = -1;
			}
			return true;
		}
		case EVENT_RESPAWN: {
			Event_Respawn();
			return true;
		}
		case EVENT_RESPAWNFX: {
			Event_RespawnFx();
			return true;
		}
		default: {
			return idEntity::ClientReceiveEvent( event, time, msg );
		}
	}
	return false;
}

/*
================
idItem::Event_DropToFloor
================
*/
void idItem::Event_DropToFloor( void ) {
	trace_t trace;

	// don't drop the floor if bound to another entity
	if ( GetBindMaster() != NULL && GetBindMaster() != this ) {
		return;
	}

	gameLocal.clip.TraceBounds( trace, renderEntity.origin, renderEntity.origin - idVec3( 0, 0, 64 ), renderEntity.bounds, MASK_SOLID | CONTENTS_CORPSE, this );
	SetOrigin( trace.endpos );
}

/*
================
idItem::Event_Touch
================
*/
void idItem::Event_Touch( idEntity *other, trace_t *trace ) {
	if ( !other->IsType( idPlayer::Type ) ) {
		return;
	}

	if ( !canPickUp ) {
		return;
	}

	Pickup( static_cast<idPlayer *>(other) );
}

/*
================
idItem::Event_Trigger
================
*/
void idItem::Event_Trigger( idEntity *activator ) {

	if ( !canPickUp && spawnArgs.GetBool( "triggerFirst" ) ) {
		canPickUp = true;
		return;
	}

	if ( activator && activator->IsType( idPlayer::Type ) ) {
		Pickup( static_cast<idPlayer *>( activator ) );
	}
}

/*
================
idItem::Event_Respawn
================
*/
void idItem::Event_Respawn( void ) {
	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_RESPAWN, NULL, false, -1 );
	}
	BecomeActive( TH_THINK );
	Show();
	inViewTime = -1000;
	lastCycle = -1;
	GetPhysics()->SetContents( CONTENTS_TRIGGER );
	SetOrigin( orgOrigin );
	StartSound( "snd_respawn", SND_CHANNEL_ITEM, 0, false, NULL );
	CancelEvents( &EV_RespawnItem ); // don't double respawn
}

/*
================
idItem::Event_RespawnFx
================
*/
void idItem::Event_RespawnFx( void ) {
	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_RESPAWNFX, NULL, false, -1 );
	}
	const char *sfx = spawnArgs.GetString( "fxRespawn" );
	if ( sfx && *sfx ) {
		idEntityFx::StartFx( sfx, NULL, NULL, this, true );
	}
}

/*
===============================================================================

  idItemPowerup

===============================================================================
*/

/*
===============
idItemPowerup
===============
*/

CLASS_DECLARATION( idItem, idItemPowerup )
END_CLASS

/*
================
idItemPowerup::idItemPowerup
================
*/
idItemPowerup::idItemPowerup() {
	time = 0;
	type = 0;
}

/*
================
idItemPowerup::Save
================
*/
void idItemPowerup::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( time );
	savefile->WriteInt( type );
}

/*
================
idItemPowerup::Restore
================
*/
void idItemPowerup::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( time );
	savefile->ReadInt( type );
}

/*
================
idItemPowerup::Spawn
================
*/
void idItemPowerup::Spawn( void ) {
	time = spawnArgs.GetInt( "time", "30" );
	type = spawnArgs.GetInt( "type", "0" );
}

/*
================
idItemPowerup::GiveToPlayer
================
*/
bool idItemPowerup::GiveToPlayer( idPlayer *player ) {
	if ( player->spectating ) {
		return false;
	}
	player->GivePowerUp( type, time * 1000 );
	return true;
}

#ifdef CTF


/*
===============================================================================

  idItemTeam

  Used for flags in Capture the Flag

===============================================================================
*/

// temporarely removed these events

const idEventDef EV_FlagReturn( "flagreturn", "e" );
const idEventDef EV_TakeFlag( "takeflag", "e" );
const idEventDef EV_DropFlag( "dropflag", "d" );
const idEventDef EV_FlagCapture( "flagcapture" );

CLASS_DECLARATION( idItem, idItemTeam )
	EVENT( EV_FlagReturn,  idItemTeam::Event_FlagReturn )
	EVENT( EV_TakeFlag,    idItemTeam::Event_TakeFlag )
	EVENT( EV_DropFlag,    idItemTeam::Event_DropFlag )
	EVENT( EV_FlagCapture, idItemTeam::Event_FlagCapture )
END_CLASS

/*
===============
idItemTeam::idItemTeam
===============
*/
idItemTeam::idItemTeam() {
    team		   = -1;
	carried		   = false;
	dropped		   = false;
	lastDrop	   = 0;

    itemGlowHandle = -1;

	skinDefault	= NULL;
	skinCarried	= NULL;

	scriptTaken		= NULL;
	scriptDropped	= NULL;
	scriptReturned	= NULL;
	scriptCaptured	= NULL;

	lastNuggetDrop	= 0;
	nuggetName		= 0;
}

/*
===============
idItemTeam::~idItemTeam
===============
*/
idItemTeam::~idItemTeam() {
	FreeLightDef();
}
/*
===============
idItemTeam::Spawn
===============
*/
void idItemTeam::Spawn( void ) {
    team					= spawnArgs.GetInt( "team" );
	returnOrigin			= GetPhysics()->GetOrigin() + idVec3( 0, 0, 20 );
	returnAxis				= GetPhysics()->GetAxis();

	BecomeActive( TH_THINK );

	const char * skinName;
	skinName = spawnArgs.GetString( "skin", ""  );
	if ( skinName[0] )
		skinDefault = declManager->FindSkin( skinName );

	skinName = spawnArgs.GetString( "skin_carried", ""  );
	if ( skinName[0] )
		skinCarried = declManager->FindSkin( skinName );

	nuggetName = spawnArgs.GetString( "nugget_name", "" );
	if ( !nuggetName[0] ) {
		nuggetName = NULL;
	}

	scriptTaken		= LoadScript( "script_taken" );
	scriptDropped	= LoadScript( "script_dropped"  );
	scriptReturned	= LoadScript( "script_returned" );
	scriptCaptured	= LoadScript( "script_captured" );

	/* Spawn attached dlight */
	/*
	idDict args;
	idVec3 lightOffset( 0.0f, 20.0f, 0.0f );

	// Set up the flag's dynamic light
	memset( &itemGlow, 0, sizeof( itemGlow ) );
	itemGlow.axis = mat3_identity;
	itemGlow.lightRadius.x = 128.0f;
	itemGlow.lightRadius.y = itemGlow.lightRadius.z = itemGlow.lightRadius.x;
	itemGlow.noShadows  = true;
	itemGlow.pointLight = true;
	itemGlow.shaderParms[ SHADERPARM_RED ] = 0.0f;
	itemGlow.shaderParms[ SHADERPARM_GREEN ] = 0.0f;
	itemGlow.shaderParms[ SHADERPARM_BLUE ] = 0.0f;
	itemGlow.shaderParms[ SHADERPARM_ALPHA ] = 0.0f;

	// Select a shader based on the team
	if ( team == 0 )
		itemGlow.shader = declManager->FindMaterial( "lights/redflag" );
	else
		itemGlow.shader = declManager->FindMaterial( "lights/blueflag" );
	*/

	idMoveableItem::Spawn();

	physicsObj.SetContents( 0 );
	physicsObj.SetClipMask( MASK_SOLID | CONTENTS_MOVEABLECLIP );
	physicsObj.SetGravity( idVec3( 0, 0, spawnArgs.GetInt("gravity", "-30" ) ) );
}


/*
===============
idItemTeam::LoadScript
===============
*/
function_t * idItemTeam::LoadScript( const char * script ) {
	function_t * function = NULL;
	idStr funcname = spawnArgs.GetString( script, "" );
	if ( funcname.Length() ) {
		 function = gameLocal.program.FindFunction( funcname );
		 if ( function == NULL ) {
#ifdef _DEBUG
			gameLocal.Warning( "idItemTeam '%s' at (%s) calls unknown function '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), funcname.c_str() );
#endif
		 }
	}
	return function;
}


/*
===============
idItemTeam::Think
===============
*/
void idItemTeam::Think( void ) {
	idMoveableItem::Think();

	TouchTriggers();

	// TODO : only update on updatevisuals
	/*idVec3 offset( 0.0f, 0.0f, 20.0f );
	itemGlow.origin = GetPhysics()->GetOrigin() + offset;
	if ( itemGlowHandle == -1 ) {
		itemGlowHandle = gameRenderWorld->AddLightDef( &itemGlow );
	} else {
		gameRenderWorld->UpdateLightDef( itemGlowHandle, &itemGlow );
	}*/

#if 1
	// should only the server do this?
	if ( gameLocal.isServer && nuggetName && carried && ( !lastNuggetDrop || (gameLocal.time - lastNuggetDrop) >  spawnArgs.GetInt("nugget_frequency") ) ) {

		SpawnNugget( GetPhysics()->GetOrigin() );
		lastNuggetDrop = gameLocal.time;
	}
#endif

	// return dropped flag after si_flagDropTimeLimit seconds
	if ( dropped && !carried && lastDrop != 0 && (gameLocal.time - lastDrop) > ( si_flagDropTimeLimit.GetInteger()*1000 )  ) {

		Return();	// return flag after 30 seconds on ground
		return;
	}
}

/*
===============
idItemTeam::Pickup
===============
*/
bool idItemTeam::Pickup( idPlayer *player ) {
    if ( !gameLocal.mpGame.IsGametypeFlagBased() ) /* CTF */
        return false;

    if ( gameLocal.mpGame.GetGameState() == idMultiplayerGame::WARMUP ||
         gameLocal.mpGame.GetGameState() == idMultiplayerGame::COUNTDOWN )
        return false;

	// wait 2 seconds after drop before beeing picked up again
	if ( lastDrop != 0 && (gameLocal.time - lastDrop) < spawnArgs.GetInt("pickupDelay", "500") )
		return false;
    
    if ( carried == false && player->team != this->team ) {

		PostEventMS( &EV_TakeFlag, 0, player );

		return true;
	} else if ( carried == false && dropped == true && player->team == this->team ) {

		gameLocal.mpGame.PlayerScoreCTF( player->entityNumber, 5 );

		// return flag
		PostEventMS( &EV_FlagReturn, 0, player );

		return false;
	}

	return false;
}

/*
===============
idItemTeam::ClientReceiveEvent
===============
*/
bool idItemTeam::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {
    gameLocal.DPrintf("ClientRecieveEvent: %i\n", event );

	switch ( event ) {
		case EVENT_TAKEFLAG: {					
			idPlayer * player = static_cast<idPlayer *>(gameLocal.entities[ msg.ReadBits( GENTITYNUM_BITS ) ]);
			if ( player == NULL ) {
				gameLocal.Warning( "NULL player takes flag?\n" );
				return false;
			}

			Event_TakeFlag( player );
		}
		return true;

		case EVENT_DROPFLAG : {
			bool death = bool( msg.ReadBits( 1 ) == 1 );
			Event_DropFlag( death );
		}
		return true;

		case EVENT_FLAGRETURN : {
			Hide();

			FreeModelDef();
			FreeLightDef();

			Event_FlagReturn();
		}
		return true;

		case EVENT_FLAGCAPTURE : {
			Hide();

			FreeModelDef();
			FreeLightDef();

			Event_FlagCapture();
		}
		return true;
	};

    return false;
}

/*
================
idItemTeam::Drop
================
*/
void idItemTeam::Drop( bool death )
{
//	PostEventMS( &EV_DropFlag, 0, int(death == true) );
// had to remove the delayed drop because of drop flag on disconnect
	Event_DropFlag( death );
}

/*
================
idItemTeam::Return
================
*/
void idItemTeam::Return( idPlayer * player )
{
	if ( team != 0 && team != 1 )
		return;

//	PostEventMS( &EV_FlagReturn, 0 );
	Event_FlagReturn();
}

/*
================
idItemTeam::Capture
================
*/
void idItemTeam::Capture( void )
{
	if ( team != 0 && team != 1 )
		return;

	PostEventMS( &EV_FlagCapture, 0 );
}

/*
================
idItemTeam::PrivateReturn
================
*/
void idItemTeam::PrivateReturn( void )
{
	Unbind();

	if ( gameLocal.isServer && carried && !dropped ) {
		int playerIdx = gameLocal.mpGame.GetFlagCarrier( 1-team );
		if ( playerIdx != -1 ) {
			idPlayer * player = static_cast<idPlayer*>( gameLocal.entities[ playerIdx ] );
			player->carryingFlag = false;
		} else {
			gameLocal.Warning( "BUG: carried flag has no carrier before return" );
		}
	}

	dropped = false;
	carried = false;

	SetOrigin( returnOrigin );
	SetAxis( returnAxis );

	trigger->Link( gameLocal.clip, this, 0, GetPhysics()->GetOrigin(), mat3_identity );

	SetSkin( skinDefault );

	// Turn off the light
	/*itemGlow.shaderParms[ SHADERPARM_RED ] = 0.0f;
	itemGlow.shaderParms[ SHADERPARM_GREEN ] = 0.0f;
	itemGlow.shaderParms[ SHADERPARM_BLUE ] = 0.0f;
	itemGlow.shaderParms[ SHADERPARM_ALPHA ] = 0.0f;

	if ( itemGlowHandle != -1 ) 
		gameRenderWorld->UpdateLightDef( itemGlowHandle, &itemGlow );*/

	GetPhysics()->SetLinearVelocity( idVec3(0, 0, 0) );
	GetPhysics()->SetAngularVelocity( idVec3(0, 0, 0) );
}

/*
================
idItemTeam::Event_TakeFlag
================
*/
void idItemTeam::Event_TakeFlag( idPlayer * player ) {
    gameLocal.DPrintf("Event_TakeFlag()!\n");

	if ( gameLocal.isServer ) {
		idBitMsg msg;
		byte msgBuf[MAX_EVENT_PARAM_SIZE];
		// Send the event
		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.BeginWriting();
		msg.WriteBits( player->entityNumber, GENTITYNUM_BITS );
		ServerSendEvent( EVENT_TAKEFLAG, &msg, false, -1 );

		gameLocal.mpGame.PlayTeamSound( player->team, SND_FLAG_TAKEN_THEIRS );
		gameLocal.mpGame.PlayTeamSound( team, SND_FLAG_TAKEN_YOURS );

		gameLocal.mpGame.PrintMessageEvent( -1, idMultiplayerGame::MSG_FLAGTAKEN, team, player->entityNumber );

		// dont drop a nugget RIGHT away
		lastNuggetDrop = gameLocal.time - gameLocal.random.RandomInt( 1000 );

	}

	BindToJoint( player, g_flagAttachJoint.GetString(), true );
	idVec3 origin( g_flagAttachOffsetX.GetFloat(), g_flagAttachOffsetY.GetFloat(), g_flagAttachOffsetZ.GetFloat() );
	idAngles angle( g_flagAttachAngleX.GetFloat(), g_flagAttachAngleY.GetFloat(), g_flagAttachAngleZ.GetFloat() );
	SetAngles( angle );
	SetOrigin( origin );

	// Turn the light on
	/*itemGlow.shaderParms[ SHADERPARM_RED ] = 1.0f;
	itemGlow.shaderParms[ SHADERPARM_GREEN ] = 1.0f;
	itemGlow.shaderParms[ SHADERPARM_BLUE ] = 1.0f;
	itemGlow.shaderParms[ SHADERPARM_ALPHA ] = 1.0f;

	if ( itemGlowHandle != -1 )
		gameRenderWorld->UpdateLightDef( itemGlowHandle, &itemGlow );*/

	if ( scriptTaken ) {
		idThread *thread = new idThread();
		thread->CallFunction( scriptTaken, false );
		thread->DelayedStart( 0 );
	}

	dropped = false;
	carried = true;
	player->carryingFlag = true;

	SetSkin( skinCarried );

	UpdateVisuals();
    UpdateGuis();

    if ( gameLocal.isServer ) {
        if ( team == 0 )
            gameLocal.mpGame.player_red_flag = player->entityNumber;
        else
            gameLocal.mpGame.player_blue_flag = player->entityNumber;
    }
}

/*
================
idItemTeam::Event_DropFlag
================
*/
void idItemTeam::Event_DropFlag( bool death ) {
    gameLocal.DPrintf("Event_DropFlag()!\n");

	if ( gameLocal.isServer ) {
		idBitMsg msg;
		byte msgBuf[MAX_EVENT_PARAM_SIZE];
		// Send the event
		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.BeginWriting();
		msg.WriteBits( death, 1 );
		ServerSendEvent( EVENT_DROPFLAG, &msg, false, -1 );

		if ( gameLocal.mpGame.IsFlagMsgOn() ) {
			gameLocal.mpGame.PlayTeamSound( 1-team,	SND_FLAG_DROPPED_THEIRS );
			gameLocal.mpGame.PlayTeamSound( team,	SND_FLAG_DROPPED_YOURS );

			gameLocal.mpGame.PrintMessageEvent( -1, idMultiplayerGame::MSG_FLAGDROP, team );
		}
	}

	lastDrop = gameLocal.time;

	BecomeActive( TH_THINK );
	Show();

	if ( death )
		GetPhysics()->SetLinearVelocity( idVec3(0, 0, 0) );
	else
		GetPhysics()->SetLinearVelocity( idVec3(0, 0, 20) );

	GetPhysics()->SetAngularVelocity( idVec3(0, 0, 0) );

//	GetPhysics()->SetLinearVelocity( ( GetPhysics()->GetLinearVelocity() * GetBindMaster()->GetPhysics()->GetAxis() ) + GetBindMaster()->GetPhysics()->GetLinearVelocity() );

	if ( GetBindMaster() ) {
		const idBounds bounds = GetPhysics()->GetBounds(); 
		idVec3 origin = GetBindMaster()->GetPhysics()->GetOrigin() + idVec3(0, 0, ( bounds[1].z-bounds[0].z )*0.6f );

		Unbind();

		SetOrigin( origin );
	}

	idAngles angle = GetPhysics()->GetAxis().ToAngles();
	angle.roll	= 0;
	angle.pitch = 0;
	SetAxis( angle.ToMat3() );

	dropped = true;
	carried = false;

	if ( scriptDropped ) {
		idThread *thread = new idThread();
		thread->CallFunction( scriptDropped, false );
		thread->DelayedStart( 0 );
	}

	SetSkin( skinDefault );    
	UpdateVisuals();
    UpdateGuis();


    if ( gameLocal.isServer ) {
        if ( team == 0 )
            gameLocal.mpGame.player_red_flag = -1;
        else
            gameLocal.mpGame.player_blue_flag = -1;    
        
    }
}

/*
================
idItemTeam::Event_FlagReturn
================
*/
void idItemTeam::Event_FlagReturn( idPlayer * player ) {
    gameLocal.DPrintf("Event_FlagReturn()!\n");

	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_FLAGRETURN, NULL, false, -1 );

		if ( gameLocal.mpGame.IsFlagMsgOn() ) {
			gameLocal.mpGame.PlayTeamSound( 1-team,	SND_FLAG_RETURN );
			gameLocal.mpGame.PlayTeamSound( team,	SND_FLAG_RETURN );

			int entitynum = 255;
			if ( player ) {
				entitynum = player->entityNumber;
			}

			gameLocal.mpGame.PrintMessageEvent( -1, idMultiplayerGame::MSG_FLAGRETURN, team, entitynum );
		}
	}

	BecomeActive( TH_THINK );
	Show();

	PrivateReturn();

	if ( scriptReturned ) {
		idThread *thread = new idThread();
		thread->CallFunction( scriptReturned, false );
		thread->DelayedStart( 0 );
	}

	UpdateVisuals();
    UpdateGuis();
//	Present();

    if ( gameLocal.isServer ) {
        if ( team == 0 )
            gameLocal.mpGame.player_red_flag = -1;
        else
            gameLocal.mpGame.player_blue_flag = -1;
    }
}

/*
================
idItemTeam::Event_FlagCapture
================
*/
void idItemTeam::Event_FlagCapture( void ) {
	gameLocal.DPrintf("Event_FlagCapture()!\n");

	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_FLAGCAPTURE, NULL, false, -1 );

		gameLocal.mpGame.PlayTeamSound( 1-team,	SND_FLAG_CAPTURED_THEIRS );
		gameLocal.mpGame.PlayTeamSound( team,	SND_FLAG_CAPTURED_YOURS );

		gameLocal.mpGame.TeamScoreCTF( 1-team, 1 );

		int playerIdx = gameLocal.mpGame.GetFlagCarrier( 1-team );
		if ( playerIdx != -1 ) {
			gameLocal.mpGame.PlayerScoreCTF( playerIdx, 10 );
		} else {
			playerIdx = 255;
		}

		gameLocal.mpGame.PrintMessageEvent( -1, idMultiplayerGame::MSG_FLAGCAPTURE, team, playerIdx );
	}

	BecomeActive( TH_THINK );
	Show();

	PrivateReturn();

	if ( scriptCaptured ) {
		idThread *thread = new idThread();
		thread->CallFunction( scriptCaptured, false );
		thread->DelayedStart( 0 );
	}

	UpdateVisuals();
    UpdateGuis();
    

    if ( gameLocal.isServer ) {
        if ( team == 0 )
            gameLocal.mpGame.player_red_flag = -1;
        else
            gameLocal.mpGame.player_blue_flag = -1;
    }
    
}

/*
================
idItemTeam::FreeLightDef
================
*/
void idItemTeam::FreeLightDef( void ) {
	if ( itemGlowHandle != -1 ) {
		gameRenderWorld->FreeLightDef( itemGlowHandle );
		itemGlowHandle = -1;
	}
}

/*
================
idItemTeam::SpawnNugget
================
*/
void idItemTeam::SpawnNugget( idVec3 pos ) {

	idAngles angle( gameLocal.random.RandomInt(spawnArgs.GetInt("nugget_pitch", "30")),	gameLocal.random.RandomInt(spawnArgs.GetInt("nugget_yaw", "360" )),	0 );
	float velocity = float(gameLocal.random.RandomInt( 40 )+15);

	velocity *= spawnArgs.GetFloat("nugget_velocity", "1" );

	idEntity * ent = idMoveableItem::DropItem( nuggetName, pos, GetPhysics()->GetAxis(), angle.ToMat3()*idVec3(velocity, velocity, velocity), 0, spawnArgs.GetInt("nugget_removedelay") );
	idPhysics_RigidBody * physics = static_cast<idPhysics_RigidBody *>( ent->GetPhysics() );

	if ( physics && physics->IsType( idPhysics_RigidBody::Type ) ) {
		physics->DisableImpact();
	}
}



/*
================
idItemTeam::Event_FlagCapture
================
*/
void idItemTeam::WriteToSnapshot( idBitMsgDelta &msg ) const {
	msg.WriteBits( carried, 1 );
	msg.WriteBits( dropped, 1 );

	WriteBindToSnapshot( msg );

	idMoveableItem::WriteToSnapshot( msg );
}


/*
================
idItemTeam::ReadFromSnapshot
================
*/
void idItemTeam::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	carried = msg.ReadBits( 1 ) == 1;
	dropped = msg.ReadBits( 1 ) == 1;

	ReadBindFromSnapshot( msg );

    if ( msg.HasChanged() )
    {
        UpdateGuis();

        if ( carried == true )
            SetSkin( skinCarried );
        else
            SetSkin( skinDefault );
    }

	idMoveableItem::ReadFromSnapshot( msg );
}

/*
================
idItemTeam::UpdateGuis

Update all client's huds wrt the flag status.
================
*/
void idItemTeam::UpdateGuis( void ) {
    idPlayer *player;
    
	for ( int i = 0; i < gameLocal.numClients; i++ ) {
		player = static_cast<idPlayer *>( gameLocal.entities[ i ] );

		if ( player == NULL || player->hud == NULL ) 
			continue;

        player->hud->SetStateInt( "red_flagstatus", gameLocal.mpGame.GetFlagStatus( 0 ) );
        player->hud->SetStateInt( "blue_flagstatus", gameLocal.mpGame.GetFlagStatus( 1 ) );

        player->hud->SetStateInt( "red_team_score",  gameLocal.mpGame.GetFlagPoints( 0 ) );
        player->hud->SetStateInt( "blue_team_score", gameLocal.mpGame.GetFlagPoints( 1 ) );

    }

}

/*
================
idItemTeam::Present
================
*/
void idItemTeam::Present( void ) {
	// hide the flag for localplayer if in first person
	if ( carried && GetBindMaster() ) {
		idPlayer * player = static_cast<idPlayer *>( GetBindMaster() );
		if ( player == gameLocal.GetLocalPlayer() && !pm_thirdPerson.GetBool() ) {
			FreeModelDef();
			BecomeActive( TH_UPDATEVISUALS );
			return;
		}
	}

	idEntity::Present();
}

#endif

/*
===============================================================================

  idObjective

===============================================================================
*/

CLASS_DECLARATION( idItem, idObjective )
	EVENT( EV_Activate,			idObjective::Event_Trigger )
	EVENT( EV_HideObjective,	idObjective::Event_HideObjective )
	EVENT( EV_GetPlayerPos,		idObjective::Event_GetPlayerPos )
	EVENT( EV_CamShot,			idObjective::Event_CamShot )
END_CLASS

/*
================
idObjective::idObjective
================
*/
idObjective::idObjective() {
	playerPos.Zero();
}

/*
================
idObjective::Save
================
*/
void idObjective::Save( idSaveGame *savefile ) const {
	savefile->WriteVec3( playerPos );
}

/*
================
idObjective::Restore
================
*/
void idObjective::Restore( idRestoreGame *savefile ) {
	savefile->ReadVec3( playerPos );
}

/*
================
idObjective::Spawn
================
*/
void idObjective::Spawn( void ) {
	Hide();
	if ( cvarSystem->GetCVarBool( "com_makingBuild") ) {
		PostEventMS( &EV_CamShot, 250 );
	}
}

/*
================
idObjective::Event_Screenshot
================
*/
void idObjective::Event_CamShot( ) {
	const char *camName;
	idStr shotName = gameLocal.GetMapName();
	shotName.StripFileExtension();
	shotName += "/";
	shotName += spawnArgs.GetString( "screenshot" );
	shotName.SetFileExtension( ".tga" );
	if ( spawnArgs.GetString( "camShot", "", &camName ) ) {
		idEntity *ent = gameLocal.FindEntity( camName );
		if ( ent && ent->cameraTarget ) {
			const renderView_t *view = ent->cameraTarget->GetRenderView();
			renderView_t fullView = *view;
			fullView.width = SCREEN_WIDTH;
			fullView.height = SCREEN_HEIGHT;

#ifdef _D3XP
			// HACK : always draw sky-portal view if there is one in the map, this isn't real-time
			if ( gameLocal.portalSkyEnt.GetEntity() && g_enablePortalSky.GetBool() ) {
				renderView_t	portalView = fullView;
				portalView.vieworg = gameLocal.portalSkyEnt.GetEntity()->GetPhysics()->GetOrigin();

				// setup global fixup projection vars
				if ( 1 ) {
					int vidWidth, vidHeight;
					idVec2 shiftScale;

					renderSystem->GetGLSettings( vidWidth, vidHeight );

					float pot;
					int temp;

					int	 w = vidWidth;
					for (temp = 1 ; temp < w ; temp<<=1) {
					}
					pot = (float)temp;
					shiftScale.x = (float)w / pot;

					int	 h = vidHeight;
					for (temp = 1 ; temp < h ; temp<<=1) {
					}
					pot = (float)temp;
					shiftScale.y = (float)h / pot;

					fullView.shaderParms[4] = shiftScale.x;
					fullView.shaderParms[5] = shiftScale.y;
				}

				gameRenderWorld->RenderScene( &portalView );
				renderSystem->CaptureRenderToImage( "_currentRender" );
			}
#endif

			// draw a view to a texture
			renderSystem->CropRenderSize( 256, 256, true );
			gameRenderWorld->RenderScene( &fullView );
			renderSystem->CaptureRenderToFile( shotName );
			renderSystem->UnCrop();
		}
	}
}

/*
================
idObjective::Event_Trigger
================
*/
void idObjective::Event_Trigger( idEntity *activator ) {
	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player ) {

		//Pickup( player );

		if ( spawnArgs.GetString( "inv_objective", NULL ) ) {
	 		if ( player && player->hud ) {
				idStr shotName = gameLocal.GetMapName();
				shotName.StripFileExtension();
				shotName += "/";
				shotName += spawnArgs.GetString( "screenshot" );
				shotName.SetFileExtension( ".tga" );
				player->hud->SetStateString( "screenshot", shotName );
				player->hud->SetStateString( "objective", "1" );
				player->hud->SetStateString( "objectivetext", spawnArgs.GetString( "objectivetext" ) );
				player->hud->SetStateString( "objectivetitle", spawnArgs.GetString( "objectivetitle" ) );
				player->GiveObjective( spawnArgs.GetString( "objectivetitle" ), spawnArgs.GetString( "objectivetext" ), shotName );

				// a tad slow but keeps from having to update all objectives in all maps with a name ptr
				for( int i = 0; i < gameLocal.num_entities; i++ ) {
					if ( gameLocal.entities[ i ] && gameLocal.entities[ i ]->IsType( idObjectiveComplete::Type ) ) {
						if ( idStr::Icmp( spawnArgs.GetString( "objectivetitle" ), gameLocal.entities[ i ]->spawnArgs.GetString( "objectivetitle" ) ) == 0 ){
							gameLocal.entities[ i ]->spawnArgs.SetBool( "objEnabled", true );
							break;
						}
					}
				}

				PostEventMS( &EV_GetPlayerPos, 2000 );
			}
		}
	}
}

/*
================
idObjective::Event_GetPlayerPos
================
*/
void idObjective::Event_GetPlayerPos() {
	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player ) {
		playerPos = player->GetPhysics()->GetOrigin();
		PostEventMS( &EV_HideObjective, 100, player );
	}
}

/*
================
idObjective::Event_HideObjective
================
*/
void idObjective::Event_HideObjective(idEntity *e) {
	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player ) {
		idVec3 v = player->GetPhysics()->GetOrigin() - playerPos;
		if ( v.Length() > 64.0f ) {
			player->HideObjective();
			PostEventMS( &EV_Remove, 0 );
		} else {
			PostEventMS( &EV_HideObjective, 100, player );
		}
	}
}

/*
===============================================================================

  idVideoCDItem

===============================================================================
*/

CLASS_DECLARATION( idItem, idVideoCDItem )
END_CLASS

/*
================
idVideoCDItem::Spawn
================
*/
void idVideoCDItem::Spawn( void ) {
}

/*
================
idVideoCDItem::GiveToPlayer
================
*/
bool idVideoCDItem::GiveToPlayer( idPlayer *player ) {
	idStr str = spawnArgs.GetString( "video" );
	if ( player && str.Length() ) {
		player->GiveVideo( str, &spawnArgs );
	}
	return true;
}

/*
===============================================================================

  idPDAItem

===============================================================================
*/

CLASS_DECLARATION( idItem, idPDAItem )
END_CLASS

/*
================
idPDAItem::GiveToPlayer
================
*/
bool idPDAItem::GiveToPlayer(idPlayer *player) {
	const char *str = spawnArgs.GetString( "pda_name" );
	if ( player ) {
		player->GivePDA( str, &spawnArgs );
	}
	return true;
}

/*
===============================================================================

  idMoveableItem
	
===============================================================================
*/

CLASS_DECLARATION( idItem, idMoveableItem )
	EVENT( EV_DropToFloor,	idMoveableItem::Event_DropToFloor )
	EVENT( EV_Gib,			idMoveableItem::Event_Gib )
END_CLASS

/*
================
idMoveableItem::idMoveableItem
================
*/
idMoveableItem::idMoveableItem() {
	trigger = NULL;
	smoke = NULL;
	smokeTime = 0;
#ifdef _D3XP
	nextSoundTime = 0;
#endif
#ifdef CTF
	repeatSmoke = false;
#endif
}

/*
================
idMoveableItem::~idMoveableItem
================
*/
idMoveableItem::~idMoveableItem() {
	if ( trigger ) {
		delete trigger;
	}
}

/*
================
idMoveableItem::Save
================
*/
void idMoveableItem::Save( idSaveGame *savefile ) const {
   	savefile->WriteStaticObject( physicsObj );

	savefile->WriteClipModel( trigger );

	savefile->WriteParticle( smoke );
	savefile->WriteInt( smokeTime );
#ifdef _D3XP
	savefile->WriteInt( nextSoundTime );
#endif
}

/*
================
idMoveableItem::Restore
================
*/
void idMoveableItem::Restore( idRestoreGame *savefile ) {
	savefile->ReadStaticObject( physicsObj );
	RestorePhysics( &physicsObj );

	savefile->ReadClipModel( trigger );

	savefile->ReadParticle( smoke );
	savefile->ReadInt( smokeTime );
#ifdef _D3XP
	savefile->ReadInt( nextSoundTime );
#endif
}

/*
================
idMoveableItem::Spawn
================
*/
void idMoveableItem::Spawn( void ) {
	idTraceModel trm;
	float density, friction, bouncyness, tsize;
	idStr clipModelName;
	idBounds bounds;
#ifdef _D3XP
	SetTimeState ts( timeGroup );
#endif

	// create a trigger for item pickup
	spawnArgs.GetFloat( "triggersize", "16.0", tsize );
	trigger = new idClipModel( idTraceModel( idBounds( vec3_origin ).Expand( tsize ) ) );
	trigger->Link( gameLocal.clip, this, 0, GetPhysics()->GetOrigin(), GetPhysics()->GetAxis() );
	trigger->SetContents( CONTENTS_TRIGGER );

	// check if a clip model is set
	spawnArgs.GetString( "clipmodel", "", clipModelName );
	if ( !clipModelName[0] ) {
		clipModelName = spawnArgs.GetString( "model" );		// use the visual model
	}

	// load the trace model
	if ( !collisionModelManager->TrmFromModel( clipModelName, trm ) ) {
		gameLocal.Error( "idMoveableItem '%s': cannot load collision model %s", name.c_str(), clipModelName.c_str() );
		return;
	}

	// if the model should be shrinked
	if ( spawnArgs.GetBool( "clipshrink" ) ) {
		trm.Shrink( CM_CLIP_EPSILON );
	}

	// get rigid body properties
	spawnArgs.GetFloat( "density", "0.5", density );
	density = idMath::ClampFloat( 0.001f, 1000.0f, density );
	spawnArgs.GetFloat( "friction", "0.05", friction );
	friction = idMath::ClampFloat( 0.0f, 1.0f, friction );
	spawnArgs.GetFloat( "bouncyness", "0.6", bouncyness );
	bouncyness = idMath::ClampFloat( 0.0f, 1.0f, bouncyness );

	// setup the physics
	physicsObj.SetSelf( this );
	physicsObj.SetClipModel( new idClipModel( trm ), density );
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() );
	physicsObj.SetAxis( GetPhysics()->GetAxis() );
	physicsObj.SetBouncyness( bouncyness );
	physicsObj.SetFriction( 0.6f, 0.6f, friction );
	physicsObj.SetGravity( gameLocal.GetGravity() );
	physicsObj.SetContents( CONTENTS_RENDERMODEL );
	physicsObj.SetClipMask( MASK_SOLID | CONTENTS_MOVEABLECLIP );
	SetPhysics( &physicsObj );

	smoke = NULL;
	smokeTime = 0;
#ifdef _D3XP
	nextSoundTime = 0;
#endif
	const char *smokeName = spawnArgs.GetString( "smoke_trail" );
	if ( *smokeName != '\0' ) {
		smoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
		smokeTime = gameLocal.time;
		BecomeActive( TH_UPDATEPARTICLES );
	}

#ifdef CTF
	repeatSmoke = spawnArgs.GetBool( "repeatSmoke", "0" );
#endif
}

/*
================
idMoveableItem::Think
================
*/
void idMoveableItem::Think( void ) {

	RunPhysics();

	if ( thinkFlags & TH_PHYSICS ) {
		// update trigger position
		trigger->Link( gameLocal.clip, this, 0, GetPhysics()->GetOrigin(), mat3_identity );
	}
	
	if ( thinkFlags & TH_UPDATEPARTICLES ) {
		if ( !gameLocal.smokeParticles->EmitSmoke( smoke, smokeTime, gameLocal.random.CRandomFloat(), GetPhysics()->GetOrigin(), GetPhysics()->GetAxis(), timeGroup /*_D3XP*/ ) ) {
#ifdef CTF
			if ( !repeatSmoke ) {
				smokeTime = 0;
				BecomeInactive( TH_UPDATEPARTICLES );
			} else {
				smokeTime = gameLocal.time;
			}
#else
			smokeTime = 0;
			BecomeInactive( TH_UPDATEPARTICLES );
#endif
		}
	}

	Present();
}

#ifdef _D3XP
/*
=================
idMoveableItem::Collide
=================
*/
bool idMoveableItem::Collide( const trace_t &collision, const idVec3 &velocity ) {
	float v, f;

	v = -( velocity * collision.c.normal );
	if ( v > 80 && gameLocal.time > nextSoundTime ) {
		f = v > 200 ? 1.0f : idMath::Sqrt( v - 80 ) * 0.091f;
		if ( StartSound( "snd_bounce", SND_CHANNEL_ANY, 0, false, NULL ) ) {
			// don't set the volume unless there is a bounce sound as it overrides the entire channel
			// which causes footsteps on ai's to not honor their shader parms
			SetSoundVolume( f );
		}
		nextSoundTime = gameLocal.time + 500;
	}

	return false;
}
#endif

/*
================
idMoveableItem::Pickup
================
*/
bool idMoveableItem::Pickup( idPlayer *player ) {
	bool ret = idItem::Pickup( player );
	if ( ret ) {
		trigger->SetContents( 0 );
	} 
	return ret;
}

/*
================
idMoveableItem::DropItem
================
*/
idEntity *idMoveableItem::DropItem( const char *classname, const idVec3 &origin, const idMat3 &axis, const idVec3 &velocity, int activateDelay, int removeDelay ) {
	idDict args;
	idEntity *item;

	args.Set( "classname", classname );
	args.Set( "dropped", "1" );

	// we sometimes drop idMoveables here, so set 'nodrop' to 1 so that it doesn't get put on the floor
	args.Set( "nodrop", "1" );

	if ( activateDelay ) {
		args.SetBool( "triggerFirst", true );
	}

	gameLocal.SpawnEntityDef( args, &item );
	if ( item ) {
		// set item position
		item->GetPhysics()->SetOrigin( origin );
		item->GetPhysics()->SetAxis( axis );
		item->GetPhysics()->SetLinearVelocity( velocity );
		item->UpdateVisuals();
		if ( activateDelay ) {
			item->PostEventMS( &EV_Activate, activateDelay, item );
		}
		if ( !removeDelay ) {
			removeDelay = 5 * 60 * 1000;
		}
		// always remove a dropped item after 5 minutes in case it dropped to an unreachable location
		item->PostEventMS( &EV_Remove, removeDelay );
	}
	return item;
}

/*
================
idMoveableItem::DropItems

  The entity should have the following key/value pairs set:
	"def_drop<type>Item"			"item def"
	"drop<type>ItemJoint"			"joint name"
	"drop<type>ItemRotation"		"pitch yaw roll"
	"drop<type>ItemOffset"			"x y z"
	"skin_drop<type>"				"skin name"
  To drop multiple items the following key/value pairs can be used:
	"def_drop<type>Item<X>"			"item def"
	"drop<type>Item<X>Joint"		"joint name"
	"drop<type>Item<X>Rotation"		"pitch yaw roll"
	"drop<type>Item<X>Offset"		"x y z"
  where <X> is an aribtrary string.
================
*/
void idMoveableItem::DropItems( idAnimatedEntity  *ent, const char *type, idList<idEntity *> *list ) {
	const idKeyValue *kv;
	const char *skinName, *c, *jointName;
	idStr key, key2;
	idVec3 origin;
	idMat3 axis;
	idAngles angles;
	const idDeclSkin *skin;
	jointHandle_t joint;
	idEntity *item;

	// drop all items
	kv = ent->spawnArgs.MatchPrefix( va( "def_drop%sItem", type ), NULL );
	while ( kv ) {

		c = kv->GetKey().c_str() + kv->GetKey().Length();
		if ( idStr::Icmp( c - 5, "Joint" ) != 0 && idStr::Icmp( c - 8, "Rotation" ) != 0 ) {

			key = kv->GetKey().c_str() + 4;
			key2 = key;
			key += "Joint";
			key2 += "Offset";
			jointName = ent->spawnArgs.GetString( key );
			joint = ent->GetAnimator()->GetJointHandle( jointName );
			if ( !ent->GetJointWorldTransform( joint, gameLocal.time, origin, axis ) ) {
				gameLocal.Warning( "%s refers to invalid joint '%s' on entity '%s'\n", key.c_str(), jointName, ent->name.c_str() );
				origin = ent->GetPhysics()->GetOrigin();
				axis = ent->GetPhysics()->GetAxis();
			}
			if ( g_dropItemRotation.GetString()[0] ) {
				angles.Zero();
				sscanf( g_dropItemRotation.GetString(), "%f %f %f", &angles.pitch, &angles.yaw, &angles.roll );
			} else {
				key = kv->GetKey().c_str() + 4;
				key += "Rotation";
				ent->spawnArgs.GetAngles( key, "0 0 0", angles );
			}
			axis = angles.ToMat3() * axis;

			origin += ent->spawnArgs.GetVector( key2, "0 0 0" );

			item = DropItem( kv->GetValue(), origin, axis, vec3_origin, 0, 0 );
			if ( list && item ) {
				list->Append( item );
			}
		}

		kv = ent->spawnArgs.MatchPrefix( va( "def_drop%sItem", type ), kv );
	}

	// change the skin to hide all items
	skinName = ent->spawnArgs.GetString( va( "skin_drop%s", type ) );
	if ( skinName[0] ) {
		skin = declManager->FindSkin( skinName );
		ent->SetSkin( skin );
	}
}

/*
======================
idMoveableItem::WriteToSnapshot
======================
*/
void idMoveableItem::WriteToSnapshot( idBitMsgDelta &msg ) const {
	physicsObj.WriteToSnapshot( msg );
}

/*
======================
idMoveableItem::ReadFromSnapshot
======================
*/
void idMoveableItem::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	physicsObj.ReadFromSnapshot( msg );
	if ( msg.HasChanged() ) {
		UpdateVisuals();
	}
}

/*
============
idMoveableItem::Gib
============
*/
void idMoveableItem::Gib( const idVec3 &dir, const char *damageDefName ) {
	// spawn smoke puff
	const char *smokeName = spawnArgs.GetString( "smoke_gib" );
	if ( *smokeName != '\0' ) {
		const idDeclParticle *smoke = static_cast<const idDeclParticle *>( declManager->FindType( DECL_PARTICLE, smokeName ) );
		gameLocal.smokeParticles->EmitSmoke( smoke, gameLocal.time, gameLocal.random.CRandomFloat(), renderEntity.origin, renderEntity.axis, timeGroup /*_D3XP*/ );
	}
	// remove the entity
	PostEventMS( &EV_Remove, 0 );
}

/*
================
idMoveableItem::Event_DropToFloor
================
*/
void idMoveableItem::Event_DropToFloor( void ) {
	// the physics will drop the moveable to the floor
}

/*
============
idMoveableItem::Event_Gib
============
*/
void idMoveableItem::Event_Gib( const char *damageDefName ) {
	Gib( idVec3( 0, 0, 1 ), damageDefName );
}

/*
===============================================================================

  idMoveablePDAItem

===============================================================================
*/

CLASS_DECLARATION( idMoveableItem, idMoveablePDAItem )
END_CLASS

/*
================
idMoveablePDAItem::GiveToPlayer
================
*/
bool idMoveablePDAItem::GiveToPlayer(idPlayer *player) {
	const char *str = spawnArgs.GetString( "pda_name" );
	if ( player ) {
		player->GivePDA( str, &spawnArgs );
	}
	return true;
}

/*
===============================================================================

  idItemRemover

===============================================================================
*/

CLASS_DECLARATION( idEntity, idItemRemover )
	EVENT( EV_Activate,		idItemRemover::Event_Trigger )
END_CLASS

/*
================
idItemRemover::Spawn
================
*/
void idItemRemover::Spawn( void ) {
}

/*
================
idItemRemover::RemoveItem
================
*/
void idItemRemover::RemoveItem( idPlayer *player ) {
	const char *remove;
	
	remove = spawnArgs.GetString( "remove" );
	player->RemoveInventoryItem( remove );
}

/*
================
idItemRemover::Event_Trigger
================
*/
void idItemRemover::Event_Trigger( idEntity *activator ) {
	if ( activator->IsType( idPlayer::Type ) ) {
		RemoveItem( static_cast<idPlayer *>(activator) );
	}
}

/*
===============================================================================

  idObjectiveComplete

===============================================================================
*/

CLASS_DECLARATION( idItemRemover, idObjectiveComplete )
	EVENT( EV_Activate,			idObjectiveComplete::Event_Trigger )
	EVENT( EV_HideObjective,	idObjectiveComplete::Event_HideObjective )
	EVENT( EV_GetPlayerPos,		idObjectiveComplete::Event_GetPlayerPos )
END_CLASS

/*
================
idObjectiveComplete::idObjectiveComplete
================
*/
idObjectiveComplete::idObjectiveComplete() {
	playerPos.Zero();
}

/*
================
idObjectiveComplete::Save
================
*/
void idObjectiveComplete::Save( idSaveGame *savefile ) const {
	savefile->WriteVec3( playerPos );
}

/*
================
idObjectiveComplete::Restore
================
*/
void idObjectiveComplete::Restore( idRestoreGame *savefile ) {
	savefile->ReadVec3( playerPos );
}

/*
================
idObjectiveComplete::Spawn
================
*/
void idObjectiveComplete::Spawn( void ) {
	spawnArgs.SetBool( "objEnabled", false );
	Hide();
}

/*
================
idObjectiveComplete::Event_Trigger
================
*/
void idObjectiveComplete::Event_Trigger( idEntity *activator ) {
	if ( !spawnArgs.GetBool( "objEnabled" ) ) {
		return;
	}
	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player ) {
		RemoveItem( player );

		if ( spawnArgs.GetString( "inv_objective", NULL ) ) {
	 		if ( player->hud ) {
				player->hud->SetStateString( "objective", "2");

				player->hud->SetStateString( "objectivetext", spawnArgs.GetString( "objectivetext" ) );
#ifdef _D3XP
				player->hud->SetStateString( "objectivecompletetitle", spawnArgs.GetString( "objectivetitle" ) );
#else
				player->hud->SetStateString( "objectivetitle", spawnArgs.GetString( "objectivetitle" ) );
#endif
				player->CompleteObjective( spawnArgs.GetString( "objectivetitle" ) );
				PostEventMS( &EV_GetPlayerPos, 2000 );
			}
		}
	}
}

/*
================
idObjectiveComplete::Event_GetPlayerPos
================
*/
void idObjectiveComplete::Event_GetPlayerPos() {
	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player ) {
		playerPos = player->GetPhysics()->GetOrigin();
		PostEventMS( &EV_HideObjective, 100, player );
	}
}

/*
================
idObjectiveComplete::Event_HideObjective
================
*/
void idObjectiveComplete::Event_HideObjective( idEntity *e ) {
	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( player ) {
		idVec3 v = player->GetPhysics()->GetOrigin();
		v -= playerPos;
		if ( v.Length() > 64.0f ) {
			player->hud->HandleNamedEvent( "closeObjective" );
			PostEventMS( &EV_Remove, 0 );
		} else {
			PostEventMS( &EV_HideObjective, 100, player );
		}
	}
}

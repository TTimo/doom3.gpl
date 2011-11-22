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

#include "tr_local.h"

//#define WRITE_GUIS

typedef struct {
	int		version;
	int		sizeofRenderEntity;
	int		sizeofRenderLight;
	char	mapname[256];
} demoHeader_t;


/*
==============
StartWritingDemo
==============
*/
void		idRenderWorldLocal::StartWritingDemo( idDemoFile *demo ) {
	int		i;

	// FIXME: we should track the idDemoFile locally, instead of snooping into session for it

	WriteLoadMap();

	// write the door portal state
	for ( i = 0 ; i < numInterAreaPortals ; i++ ) {
		if ( doublePortals[i].blockingBits ) {
			SetPortalState( i+1, doublePortals[i].blockingBits );
		}
	}

	// clear the archive counter on all defs
	for ( i = 0 ; i < lightDefs.Num() ; i++ ) {
		if ( lightDefs[i] ) {
			lightDefs[i]->archived = false;
		}
	}
	for ( i = 0 ; i < entityDefs.Num() ; i++ ) {
		if ( entityDefs[i] ) {
			entityDefs[i]->archived = false;
		}
	}
}

void idRenderWorldLocal::StopWritingDemo() {
//	writeDemo = NULL;
}

/*
==============
ProcessDemoCommand
==============
*/
bool		idRenderWorldLocal::ProcessDemoCommand( idDemoFile *readDemo, renderView_t *renderView, int *demoTimeOffset ) {
	bool	newMap = false;
	
	if ( !readDemo ) {
		return false;
	}

	demoCommand_t	dc;
	qhandle_t		h;

	if ( !readDemo->ReadInt( (int&)dc ) ) {
		// a demoShot may not have an endFrame, but it is still valid
		return false;
	}

	switch( dc ) {
	case DC_LOADMAP:
		// read the initial data
		demoHeader_t	header;

		readDemo->ReadInt( header.version );
		readDemo->ReadInt( header.sizeofRenderEntity );
		readDemo->ReadInt( header.sizeofRenderLight );
		for ( int i = 0; i < 256; i++ )
			readDemo->ReadChar( header.mapname[i] );
		// the internal version value got replaced by DS_VERSION at toplevel
		if ( header.version != 4 ) {
				common->Error( "Demo version mismatch.\n" );
		}

		if ( r_showDemo.GetBool() ) {
			common->Printf( "DC_LOADMAP: %s\n", header.mapname );
		}
		InitFromMap( header.mapname );

		newMap = true;		// we will need to set demoTimeOffset

		break;

	case DC_RENDERVIEW:
		readDemo->ReadInt( renderView->viewID );
		readDemo->ReadInt( renderView->x );
		readDemo->ReadInt( renderView->y );
		readDemo->ReadInt( renderView->width );
		readDemo->ReadInt( renderView->height );
		readDemo->ReadFloat( renderView->fov_x );
		readDemo->ReadFloat( renderView->fov_y );
		readDemo->ReadVec3( renderView->vieworg );
		readDemo->ReadMat3( renderView->viewaxis );
		readDemo->ReadBool( renderView->cramZNear );
		readDemo->ReadBool( renderView->forceUpdate );
		// binary compatibility with win32 padded structures
		char tmp;
		readDemo->ReadChar( tmp );
		readDemo->ReadChar( tmp );
		readDemo->ReadInt( renderView->time );
		for ( int i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ )
			readDemo->ReadFloat( renderView->shaderParms[i] );

		if ( !readDemo->ReadInt( (int&)renderView->globalMaterial ) ) {
			 return false;
		 }
												 
		if ( r_showDemo.GetBool() ) {
			common->Printf( "DC_RENDERVIEW: %i\n", renderView->time );
		}

		// possibly change the time offset if this is from a new map
		if ( newMap && demoTimeOffset ) {
			*demoTimeOffset = renderView->time - eventLoop->Milliseconds();
		}
		return false;

	case DC_UPDATE_ENTITYDEF:
		ReadRenderEntity();
		break;
	case DC_DELETE_ENTITYDEF:
		if ( !readDemo->ReadInt( h ) ) {
			return false;
		}
		if ( r_showDemo.GetBool() ) {
			common->Printf( "DC_DELETE_ENTITYDEF: %i\n", h );
		}
		FreeEntityDef( h );
		break;
	case DC_UPDATE_LIGHTDEF:
		ReadRenderLight();
		break;
	case DC_DELETE_LIGHTDEF:
		if ( !readDemo->ReadInt( h ) ) {
			return false;
		}
		if ( r_showDemo.GetBool() ) {
			common->Printf( "DC_DELETE_LIGHTDEF: %i\n", h );
		}
		FreeLightDef( h );
		break;

	case DC_CAPTURE_RENDER:
		if ( r_showDemo.GetBool() ) {
			common->Printf( "DC_CAPTURE_RENDER\n" );
		}
		renderSystem->CaptureRenderToImage( readDemo->ReadHashString() );
		break;

	case DC_CROP_RENDER:
		if ( r_showDemo.GetBool() ) {
			common->Printf( "DC_CROP_RENDER\n" );
		}
		int	size[3];
		readDemo->ReadInt( size[0] );
		readDemo->ReadInt( size[1] );
		readDemo->ReadInt( size[2] );
		renderSystem->CropRenderSize( size[0], size[1], size[2] != 0 );
		break;

	case DC_UNCROP_RENDER:
		if ( r_showDemo.GetBool() ) {
			common->Printf( "DC_UNCROP\n" );
		}
		renderSystem->UnCrop();
		break;

	case DC_GUI_MODEL:
		if ( r_showDemo.GetBool() ) {
			common->Printf( "DC_GUI_MODEL\n" );
		}
		tr.demoGuiModel->ReadFromDemo( readDemo );
		break;

	case DC_DEFINE_MODEL:
		{
		idRenderModel	*model = renderModelManager->AllocModel();
		model->ReadFromDemoFile( session->readDemo );
		// add to model manager, so we can find it
		renderModelManager->AddModel( model );

		// save it in the list to free when clearing this map
		localModels.Append( model );

		if ( r_showDemo.GetBool() ) {
			common->Printf( "DC_DEFINE_MODEL\n" );
		}
		break;
		}
	case DC_SET_PORTAL_STATE:
		{
			int		data[2];
			readDemo->ReadInt( data[0] );
			readDemo->ReadInt( data[1] );
			SetPortalState( data[0], data[1] );
			if ( r_showDemo.GetBool() ) {
				common->Printf( "DC_SET_PORTAL_STATE: %i %i\n", data[0], data[1] );
			}
		}
		
		break;
	case DC_END_FRAME:
		return true;

	default:
		common->Error( "Bad token in demo stream" );
	}

	return false;
}

/*
================
WriteLoadMap
================
*/
void	idRenderWorldLocal::WriteLoadMap() {

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if ( this != session->rw ) {
		return;
	}

	session->writeDemo->WriteInt( DS_RENDER );
	session->writeDemo->WriteInt( DC_LOADMAP );

	demoHeader_t	header;
	strncpy( header.mapname, mapName.c_str(), sizeof( header.mapname ) - 1 );
	header.version = 4;
	header.sizeofRenderEntity = sizeof( renderEntity_t );
	header.sizeofRenderLight = sizeof( renderLight_t );
	session->writeDemo->WriteInt( header.version );
	session->writeDemo->WriteInt( header.sizeofRenderEntity );
	session->writeDemo->WriteInt( header.sizeofRenderLight );
	for ( int i = 0; i < 256; i++ )
		session->writeDemo->WriteChar( header.mapname[i] );
	
	if ( r_showDemo.GetBool() ) {
		common->Printf( "write DC_DELETE_LIGHTDEF: %s\n", mapName.c_str() );
	}
}

/*
================
WriteVisibleDefs

================
*/
void	idRenderWorldLocal::WriteVisibleDefs( const viewDef_t *viewDef ) {
	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if ( this != session->rw ) {
		return;
	}

	// make sure all necessary entities and lights are updated
	for ( viewEntity_t *viewEnt = viewDef->viewEntitys ; viewEnt ; viewEnt = viewEnt->next ) {
		idRenderEntityLocal *ent = viewEnt->entityDef;

		if ( ent->archived ) {
			// still up to date
			continue;
		}

		// write it out
		WriteRenderEntity( ent->index, &ent->parms );
		ent->archived = true;
	}

	for ( viewLight_t *viewLight = viewDef->viewLights ; viewLight ; viewLight = viewLight->next ) {
		idRenderLightLocal *light = viewLight->lightDef;

		if ( light->archived ) {
			// still up to date
			continue;
		}
		// write it out
		WriteRenderLight( light->index, &light->parms );
		light->archived = true;
	}
}


/*
================
WriteRenderView
================
*/
void	idRenderWorldLocal::WriteRenderView( const renderView_t *renderView ) {
	int i;

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if ( this != session->rw ) {
		return;
	}
	
	// write the actual view command
	session->writeDemo->WriteInt( DS_RENDER );
	session->writeDemo->WriteInt( DC_RENDERVIEW );
	session->writeDemo->WriteInt( renderView->viewID );
	session->writeDemo->WriteInt( renderView->x );
	session->writeDemo->WriteInt( renderView->y );
	session->writeDemo->WriteInt( renderView->width );
	session->writeDemo->WriteInt( renderView->height );
	session->writeDemo->WriteFloat( renderView->fov_x );
	session->writeDemo->WriteFloat( renderView->fov_y );
	session->writeDemo->WriteVec3( renderView->vieworg );
	session->writeDemo->WriteMat3( renderView->viewaxis );
	session->writeDemo->WriteBool( renderView->cramZNear );
	session->writeDemo->WriteBool( renderView->forceUpdate );
	// binary compatibility with old win32 version writing padded structures directly to disk
	session->writeDemo->WriteUnsignedChar( 0 );
	session->writeDemo->WriteUnsignedChar( 0 );
	session->writeDemo->WriteInt( renderView->time );
	for ( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ )
		session->writeDemo->WriteFloat( renderView->shaderParms[i] );
	session->writeDemo->WriteInt( (int&)renderView->globalMaterial );
	
	if ( r_showDemo.GetBool() ) {
		common->Printf( "write DC_RENDERVIEW: %i\n", renderView->time );
	}
}

/*
================
WriteFreeEntity
================
*/
void	idRenderWorldLocal::WriteFreeEntity( qhandle_t handle ) {

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if ( this != session->rw ) {
		return;
	}

	session->writeDemo->WriteInt( DS_RENDER );
	session->writeDemo->WriteInt( DC_DELETE_ENTITYDEF );
	session->writeDemo->WriteInt( handle );

	if ( r_showDemo.GetBool() ) {
		common->Printf( "write DC_DELETE_ENTITYDEF: %i\n", handle );
	}
}

/*
================
WriteFreeLightEntity
================
*/
void	idRenderWorldLocal::WriteFreeLight( qhandle_t handle ) {

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if ( this != session->rw ) {
		return;
	}

	session->writeDemo->WriteInt( DS_RENDER );
	session->writeDemo->WriteInt( DC_DELETE_LIGHTDEF );
	session->writeDemo->WriteInt( handle );

	if ( r_showDemo.GetBool() ) {
		common->Printf( "write DC_DELETE_LIGHTDEF: %i\n", handle );
	}
}

/*
================
WriteRenderLight
================
*/
void	idRenderWorldLocal::WriteRenderLight( qhandle_t handle, const renderLight_t *light ) {

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if ( this != session->rw ) {
		return;
	}

	session->writeDemo->WriteInt( DS_RENDER );
	session->writeDemo->WriteInt( DC_UPDATE_LIGHTDEF );
	session->writeDemo->WriteInt( handle );

	session->writeDemo->WriteMat3( light->axis );
	session->writeDemo->WriteVec3( light->origin );
	session->writeDemo->WriteInt( light->suppressLightInViewID );
	session->writeDemo->WriteInt( light->allowLightInViewID );
	session->writeDemo->WriteBool( light->noShadows );
	session->writeDemo->WriteBool( light->noSpecular );
	session->writeDemo->WriteBool( light->pointLight );
	session->writeDemo->WriteBool( light->parallel );
	session->writeDemo->WriteVec3( light->lightRadius );
	session->writeDemo->WriteVec3( light->lightCenter );
	session->writeDemo->WriteVec3( light->target );
	session->writeDemo->WriteVec3( light->right );
	session->writeDemo->WriteVec3( light->up );
	session->writeDemo->WriteVec3( light->start );
	session->writeDemo->WriteVec3( light->end );
	session->writeDemo->WriteInt( (int&)light->prelightModel );
	session->writeDemo->WriteInt( light->lightId );
	session->writeDemo->WriteInt( (int&)light->shader );
	for ( int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++)
		session->writeDemo->WriteFloat( light->shaderParms[i] );
	session->writeDemo->WriteInt( (int&)light->referenceSound );

	if ( light->prelightModel ) {
		session->writeDemo->WriteHashString( light->prelightModel->Name() );
	}
	if ( light->shader ) {
		session->writeDemo->WriteHashString( light->shader->GetName() );
	}
	if ( light->referenceSound ) {
		int	index = light->referenceSound->Index();
		session->writeDemo->WriteInt( index );
	}

	if ( r_showDemo.GetBool() ) {
		common->Printf( "write DC_UPDATE_LIGHTDEF: %i\n", handle );
	}
}

/*
================
ReadRenderLight
================
*/
void	idRenderWorldLocal::ReadRenderLight( ) {
	renderLight_t	light;
	int				index;

	session->readDemo->ReadInt( index );
	if ( index < 0 ) {
		common->Error( "ReadRenderLight: index < 0 " );
	}

	session->readDemo->ReadMat3( light.axis );
	session->readDemo->ReadVec3( light.origin );
	session->readDemo->ReadInt( light.suppressLightInViewID );
	session->readDemo->ReadInt( light.allowLightInViewID );
	session->readDemo->ReadBool( light.noShadows );
	session->readDemo->ReadBool( light.noSpecular );
	session->readDemo->ReadBool( light.pointLight );
	session->readDemo->ReadBool( light.parallel );
	session->readDemo->ReadVec3( light.lightRadius );
	session->readDemo->ReadVec3( light.lightCenter );
	session->readDemo->ReadVec3( light.target );
	session->readDemo->ReadVec3( light.right );
	session->readDemo->ReadVec3( light.up );
	session->readDemo->ReadVec3( light.start );
	session->readDemo->ReadVec3( light.end );
	session->readDemo->ReadInt( (int&)light.prelightModel );
	session->readDemo->ReadInt( light.lightId );
	session->readDemo->ReadInt( (int&)light.shader );
	for ( int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++)
		session->readDemo->ReadFloat( light.shaderParms[i] );
	session->readDemo->ReadInt( (int&)light.referenceSound );
	if ( light.prelightModel ) {
		light.prelightModel = renderModelManager->FindModel( session->readDemo->ReadHashString() );
	}
	if ( light.shader ) {
		light.shader = declManager->FindMaterial( session->readDemo->ReadHashString() );
	}
	if ( light.referenceSound ) {
		int	index;
		session->readDemo->ReadInt( index );
		light.referenceSound = session->sw->EmitterForIndex( index );
	}

	UpdateLightDef( index, &light );

	if ( r_showDemo.GetBool() ) {
		common->Printf( "DC_UPDATE_LIGHTDEF: %i\n", index );
	}
}

/*
================
WriteRenderEntity
================
*/
void	idRenderWorldLocal::WriteRenderEntity( qhandle_t handle, const renderEntity_t *ent ) {

	// only the main renderWorld writes stuff to demos, not the wipes or
	// menu renders
	if ( this != session->rw ) {
		return;
	}

	session->writeDemo->WriteInt( DS_RENDER );
	session->writeDemo->WriteInt( DC_UPDATE_ENTITYDEF );
	session->writeDemo->WriteInt( handle );
	
	session->writeDemo->WriteInt( (int&)ent->hModel );
	session->writeDemo->WriteInt( ent->entityNum );
	session->writeDemo->WriteInt( ent->bodyId );
	session->writeDemo->WriteVec3( ent->bounds[0] );
	session->writeDemo->WriteVec3( ent->bounds[1] );
	session->writeDemo->WriteInt( (int&)ent->callback );
	session->writeDemo->WriteInt( (int&)ent->callbackData );
	session->writeDemo->WriteInt( ent->suppressSurfaceInViewID );
	session->writeDemo->WriteInt( ent->suppressShadowInViewID );
	session->writeDemo->WriteInt( ent->suppressShadowInLightID );
	session->writeDemo->WriteInt( ent->allowSurfaceInViewID );
	session->writeDemo->WriteVec3( ent->origin );
	session->writeDemo->WriteMat3( ent->axis );
	session->writeDemo->WriteInt( (int&)ent->customShader );
	session->writeDemo->WriteInt( (int&)ent->referenceShader );
	session->writeDemo->WriteInt( (int&)ent->customSkin );
	session->writeDemo->WriteInt( (int&)ent->referenceSound );
	for ( int i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ )
		session->writeDemo->WriteFloat( ent->shaderParms[i] );
	for ( int i = 0; i < MAX_RENDERENTITY_GUI; i++ )
		session->writeDemo->WriteInt( (int&)ent->gui[i] );
	session->writeDemo->WriteInt( (int&)ent->remoteRenderView );
	session->writeDemo->WriteInt( ent->numJoints );
	session->writeDemo->WriteInt( (int&)ent->joints );
	session->writeDemo->WriteFloat( ent->modelDepthHack );
	session->writeDemo->WriteBool( ent->noSelfShadow );
	session->writeDemo->WriteBool( ent->noShadow );
	session->writeDemo->WriteBool( ent->noDynamicInteractions );
	session->writeDemo->WriteBool( ent->weaponDepthHack );
	session->writeDemo->WriteInt( ent->forceUpdate );

	if ( ent->customShader ) {
		session->writeDemo->WriteHashString( ent->customShader->GetName() );
	}
	if ( ent->customSkin ) {
		session->writeDemo->WriteHashString( ent->customSkin->GetName() );
	}
	if ( ent->hModel ) {
		session->writeDemo->WriteHashString( ent->hModel->Name() );
	}
	if ( ent->referenceShader ) {
		session->writeDemo->WriteHashString( ent->referenceShader->GetName() );
	}
	if ( ent->referenceSound ) {
		int	index = ent->referenceSound->Index();
		session->writeDemo->WriteInt( index );
	}
	if ( ent->numJoints ) {
		for ( int i = 0; i < ent->numJoints; i++) {
			float *data = ent->joints[i].ToFloatPtr();
			for ( int j = 0; j < 12; ++j)
				session->writeDemo->WriteFloat( data[j] );
		}
	}

	/*
	if ( ent->decals ) {
		ent->decals->WriteToDemoFile( session->readDemo );
	}
	if ( ent->overlay ) {
		ent->overlay->WriteToDemoFile( session->writeDemo );
	}
	*/

#ifdef WRITE_GUIS
	if ( ent->gui ) {
		ent->gui->WriteToDemoFile( session->writeDemo );
	}
	if ( ent->gui2 ) {
		ent->gui2->WriteToDemoFile( session->writeDemo );
	}
	if ( ent->gui3 ) {
		ent->gui3->WriteToDemoFile( session->writeDemo );
	}
#endif

	// RENDERDEMO_VERSION >= 2 ( Doom3 1.2 )
	session->writeDemo->WriteInt( ent->timeGroup );
	session->writeDemo->WriteInt( ent->xrayIndex );

	if ( r_showDemo.GetBool() ) {
		common->Printf( "write DC_UPDATE_ENTITYDEF: %i = %s\n", handle, ent->hModel ? ent->hModel->Name() : "NULL" );
	}
}

/*
================
ReadRenderEntity
================
*/
void	idRenderWorldLocal::ReadRenderEntity() {
	renderEntity_t		ent;
	int				index, i;

	session->readDemo->ReadInt( index );
	if ( index < 0 ) {
		common->Error( "ReadRenderEntity: index < 0" );
	}

	session->readDemo->ReadInt( (int&)ent.hModel );
	session->readDemo->ReadInt( ent.entityNum );
	session->readDemo->ReadInt( ent.bodyId );
	session->readDemo->ReadVec3( ent.bounds[0] );
	session->readDemo->ReadVec3( ent.bounds[1] );
	session->readDemo->ReadInt( (int&)ent.callback );
	session->readDemo->ReadInt( (int&)ent.callbackData );
	session->readDemo->ReadInt( ent.suppressSurfaceInViewID );
	session->readDemo->ReadInt( ent.suppressShadowInViewID );
	session->readDemo->ReadInt( ent.suppressShadowInLightID );
	session->readDemo->ReadInt( ent.allowSurfaceInViewID );
	session->readDemo->ReadVec3( ent.origin );
	session->readDemo->ReadMat3( ent.axis );
	session->readDemo->ReadInt( (int&)ent.customShader );
	session->readDemo->ReadInt( (int&)ent.referenceShader );
	session->readDemo->ReadInt( (int&)ent.customSkin );
	session->readDemo->ReadInt( (int&)ent.referenceSound );
	for ( i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ ) {
		session->readDemo->ReadFloat( ent.shaderParms[i] );
	}
	for ( i = 0; i < MAX_RENDERENTITY_GUI; i++ ) {
		session->readDemo->ReadInt( (int&)ent.gui[i] );
	}
	session->readDemo->ReadInt( (int&)ent.remoteRenderView );
	session->readDemo->ReadInt( ent.numJoints );
	session->readDemo->ReadInt( (int&)ent.joints );
	session->readDemo->ReadFloat( ent.modelDepthHack );
	session->readDemo->ReadBool( ent.noSelfShadow );
	session->readDemo->ReadBool( ent.noShadow );
	session->readDemo->ReadBool( ent.noDynamicInteractions );
	session->readDemo->ReadBool( ent.weaponDepthHack );
	session->readDemo->ReadInt( ent.forceUpdate );
	ent.callback = NULL;
	if ( ent.customShader ) {
		ent.customShader = declManager->FindMaterial( session->readDemo->ReadHashString() );
	}
	if ( ent.customSkin ) {
		ent.customSkin = declManager->FindSkin( session->readDemo->ReadHashString() );
	}
	if ( ent.hModel ) {
		ent.hModel = renderModelManager->FindModel( session->readDemo->ReadHashString() );
	}
	if ( ent.referenceShader ) {
		ent.referenceShader = declManager->FindMaterial( session->readDemo->ReadHashString() );
	}
	if ( ent.referenceSound ) {
		int	index;
		session->readDemo->ReadInt( index );
		ent.referenceSound = session->sw->EmitterForIndex( index );
	}
	if ( ent.numJoints ) {
		ent.joints = (idJointMat *)Mem_Alloc16( ent.numJoints * sizeof( ent.joints[0] ) ); 
		for ( int i = 0; i < ent.numJoints; i++) {
			float *data = ent.joints[i].ToFloatPtr();
			for ( int j = 0; j < 12; ++j)
				session->readDemo->ReadFloat( data[j] );
		}
	}

	ent.callbackData = NULL;

	/*
	if ( ent.decals ) {
		ent.decals = idRenderModelDecal::Alloc();
		ent.decals->ReadFromDemoFile( session->readDemo );
	}
	if ( ent.overlay ) {
		ent.overlay = idRenderModelOverlay::Alloc();
		ent.overlay->ReadFromDemoFile( session->readDemo );
	}
	*/

	for ( i = 0; i < MAX_RENDERENTITY_GUI; i++ ) {
		if ( ent.gui[ i ] ) {
			ent.gui[ i ] = uiManager->Alloc();
#ifdef WRITE_GUIS
			ent.gui[ i ]->ReadFromDemoFile( session->readDemo );
#endif
		}
	}

	// >= Doom3 v1.2 only
	if ( session->renderdemoVersion >= 2 ) {
		session->readDemo->ReadInt( ent.timeGroup );
		session->readDemo->ReadInt( ent.xrayIndex );
	} else {
		ent.timeGroup = 0;
		ent.xrayIndex = 0;
	}

	UpdateEntityDef( index, &ent );

	if ( r_showDemo.GetBool() ) {
		common->Printf( "DC_UPDATE_ENTITYDEF: %i = %s\n", index, ent.hModel ? ent.hModel->Name() : "NULL" );
	}
}

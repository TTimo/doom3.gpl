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


/*
================
idGuiModel::idGuiModel
================
*/
idGuiModel::idGuiModel() {
	indexes.SetGranularity( 1000 );
	verts.SetGranularity( 1000 );
}

/*
================
idGuiModel::Clear

Begins collecting draw commands into surfaces
================
*/
void idGuiModel::Clear() {
	surfaces.SetNum( 0, false );
	indexes.SetNum( 0, false );
	verts.SetNum( 0, false );
	AdvanceSurf();
}

/*
================
idGuiModel::WriteToDemo
================
*/
void idGuiModel::WriteToDemo( idDemoFile *demo ) {
	int		i, j;

	i = verts.Num();
	demo->WriteInt( i );
	for ( j = 0; j < i; j++ )
	{
		demo->WriteVec3( verts[j].xyz );
		demo->WriteVec2( verts[j].st );
		demo->WriteVec3( verts[j].normal );
		demo->WriteVec3( verts[j].tangents[0] );
		demo->WriteVec3( verts[j].tangents[1] );
		demo->WriteUnsignedChar( verts[j].color[0] );
		demo->WriteUnsignedChar( verts[j].color[1] );
		demo->WriteUnsignedChar( verts[j].color[2] );
		demo->WriteUnsignedChar( verts[j].color[3] );
	}
	
	i = indexes.Num();
	demo->WriteInt( i );
	for ( j = 0; j < i; j++ ) {
		demo->WriteInt(indexes[j] );
	}
	
	i = surfaces.Num();
	demo->WriteInt( i );
	for ( j = 0 ; j < i ; j++ ) {
		guiModelSurface_t	*surf = &surfaces[j];
		
		demo->WriteInt( (int&)surf->material );
		demo->WriteFloat( surf->color[0] );
		demo->WriteFloat( surf->color[1] );
		demo->WriteFloat( surf->color[2] );
		demo->WriteFloat( surf->color[3] );
		demo->WriteInt( surf->firstVert );
		demo->WriteInt( surf->numVerts );
		demo->WriteInt( surf->firstIndex );
		demo->WriteInt( surf->numIndexes );
		demo->WriteHashString( surf->material->GetName() );
	}
}

/*
================
idGuiModel::ReadFromDemo
================
*/
void idGuiModel::ReadFromDemo( idDemoFile *demo ) {
	int		i, j;

	i = verts.Num();
	demo->ReadInt( i );
	verts.SetNum( i, false );
	for ( j = 0; j < i; j++ )
	{
		demo->ReadVec3( verts[j].xyz );
		demo->ReadVec2( verts[j].st );
		demo->ReadVec3( verts[j].normal );
		demo->ReadVec3( verts[j].tangents[0] );
		demo->ReadVec3( verts[j].tangents[1] );
		demo->ReadUnsignedChar( verts[j].color[0] );
		demo->ReadUnsignedChar( verts[j].color[1] );
		demo->ReadUnsignedChar( verts[j].color[2] );
		demo->ReadUnsignedChar( verts[j].color[3] );
	}
	
	i = indexes.Num();
	demo->ReadInt( i );
	indexes.SetNum( i, false );
	for ( j = 0; j < i; j++ ) {
		demo->ReadInt(indexes[j] );
	}
	
	i = surfaces.Num();
	demo->ReadInt( i );
	surfaces.SetNum( i, false );
	for ( j = 0 ; j < i ; j++ ) {
		guiModelSurface_t	*surf = &surfaces[j];
		
		demo->ReadInt( (int&)surf->material );
		demo->ReadFloat( surf->color[0] );
		demo->ReadFloat( surf->color[1] );
		demo->ReadFloat( surf->color[2] );
		demo->ReadFloat( surf->color[3] );
		demo->ReadInt( surf->firstVert );
		demo->ReadInt( surf->numVerts );
		demo->ReadInt( surf->firstIndex );
		demo->ReadInt( surf->numIndexes );
		surf->material = declManager->FindMaterial( demo->ReadHashString() );
	}
}

/*
================
EmitSurface
================
*/
void idGuiModel::EmitSurface( guiModelSurface_t *surf, float modelMatrix[16], float modelViewMatrix[16], bool depthHack ) {
	srfTriangles_t	*tri;

	if ( surf->numVerts == 0 ) {
		return;		// nothing in the surface
	}

	// copy verts and indexes
	tri = (srfTriangles_t *)R_ClearedFrameAlloc( sizeof( *tri ) );

	tri->numIndexes = surf->numIndexes;
	tri->numVerts = surf->numVerts;
	tri->indexes = (glIndex_t *)R_FrameAlloc( tri->numIndexes * sizeof( tri->indexes[0] ) );
	memcpy( tri->indexes, &indexes[surf->firstIndex], tri->numIndexes * sizeof( tri->indexes[0] ) );

	// we might be able to avoid copying these and just let them reference the list vars
	// but some things, like deforms and recursive
	// guis, need to access the verts in cpu space, not just through the vertex range
	tri->verts = (idDrawVert *)R_FrameAlloc( tri->numVerts * sizeof( tri->verts[0] ) );
	memcpy( tri->verts, &verts[surf->firstVert], tri->numVerts * sizeof( tri->verts[0] ) );

	// move the verts to the vertex cache
	tri->ambientCache = vertexCache.AllocFrameTemp( tri->verts, tri->numVerts * sizeof( tri->verts[0] ) );

	// if we are out of vertex cache, don't create the surface
	if ( !tri->ambientCache ) {
		return;
	}

	renderEntity_t renderEntity;
	memset( &renderEntity, 0, sizeof( renderEntity ) );
	memcpy( renderEntity.shaderParms, surf->color, sizeof( surf->color ) );

	viewEntity_t *guiSpace = (viewEntity_t *)R_ClearedFrameAlloc( sizeof( *guiSpace ) );
	memcpy( guiSpace->modelMatrix, modelMatrix, sizeof( guiSpace->modelMatrix ) );
	memcpy( guiSpace->modelViewMatrix, modelViewMatrix, sizeof( guiSpace->modelViewMatrix ) );
	guiSpace->weaponDepthHack = depthHack;

	// add the surface, which might recursively create another gui
	R_AddDrawSurf( tri, guiSpace, &renderEntity, surf->material, tr.viewDef->scissor );
}

/*
====================
EmitToCurrentView
====================
*/
void idGuiModel::EmitToCurrentView( float modelMatrix[16], bool depthHack ) {
	float	modelViewMatrix[16];

	myGlMultMatrix( modelMatrix, tr.viewDef->worldSpace.modelViewMatrix, 
			modelViewMatrix );

	for ( int i = 0 ; i < surfaces.Num() ; i++ ) {
		EmitSurface( &surfaces[i], modelMatrix, modelViewMatrix, depthHack );
	}
}

/*
================
idGuiModel::EmitFullScreen

Creates a view that covers the screen and emit the surfaces
================
*/
void idGuiModel::EmitFullScreen( void ) {
	viewDef_t	*viewDef;

	if ( surfaces[0].numVerts == 0 ) {
		return;
	}

	viewDef = (viewDef_t *)R_ClearedFrameAlloc( sizeof( *viewDef ) );

	// for gui editor
	if ( !tr.viewDef || !tr.viewDef->isEditor ) {
		viewDef->renderView.x = 0;
		viewDef->renderView.y = 0;
		viewDef->renderView.width = SCREEN_WIDTH;
		viewDef->renderView.height = SCREEN_HEIGHT;

		tr.RenderViewToViewport( &viewDef->renderView, &viewDef->viewport );

		viewDef->scissor.x1 = 0;
		viewDef->scissor.y1 = 0;
		viewDef->scissor.x2 = viewDef->viewport.x2 - viewDef->viewport.x1;
		viewDef->scissor.y2 = viewDef->viewport.y2 - viewDef->viewport.y1;
	} else {
		viewDef->renderView.x = tr.viewDef->renderView.x;
		viewDef->renderView.y = tr.viewDef->renderView.y;
		viewDef->renderView.width = tr.viewDef->renderView.width;
		viewDef->renderView.height = tr.viewDef->renderView.height;
		
		viewDef->viewport.x1 = tr.viewDef->renderView.x;
		viewDef->viewport.x2 = tr.viewDef->renderView.x + tr.viewDef->renderView.width;
		viewDef->viewport.y1 = tr.viewDef->renderView.y;
		viewDef->viewport.y2 = tr.viewDef->renderView.y + tr.viewDef->renderView.height;

		viewDef->scissor.x1 = tr.viewDef->scissor.x1;
		viewDef->scissor.y1 = tr.viewDef->scissor.y1;
		viewDef->scissor.x2 = tr.viewDef->scissor.x2;
		viewDef->scissor.y2 = tr.viewDef->scissor.y2;
	}

	viewDef->floatTime = tr.frameShaderTime;

	// qglOrtho( 0, 640, 480, 0, 0, 1 );		// always assume 640x480 virtual coordinates
	viewDef->projectionMatrix[0] = 2.0f / 640.0f;
	viewDef->projectionMatrix[5] = -2.0f / 480.0f;
	viewDef->projectionMatrix[10] = -2.0f / 1.0f;
	viewDef->projectionMatrix[12] = -1.0f;
	viewDef->projectionMatrix[13] = 1.0f;
	viewDef->projectionMatrix[14] = -1.0f;
	viewDef->projectionMatrix[15] = 1.0f;

	viewDef->worldSpace.modelViewMatrix[0] = 1.0f;
	viewDef->worldSpace.modelViewMatrix[5] = 1.0f;
	viewDef->worldSpace.modelViewMatrix[10] = 1.0f;
	viewDef->worldSpace.modelViewMatrix[15] = 1.0f;

	viewDef->maxDrawSurfs = surfaces.Num();
	viewDef->drawSurfs = (drawSurf_t **)R_FrameAlloc( viewDef->maxDrawSurfs * sizeof( viewDef->drawSurfs[0] ) );
	viewDef->numDrawSurfs = 0;

	viewDef_t	*oldViewDef = tr.viewDef;
	tr.viewDef = viewDef;

	// add the surfaces to this view
	for ( int i = 0 ; i < surfaces.Num() ; i++ ) {
		EmitSurface( &surfaces[i], viewDef->worldSpace.modelMatrix, viewDef->worldSpace.modelViewMatrix, false );
	}

	tr.viewDef = oldViewDef;

	// add the command to draw this view
	R_AddDrawViewCmd( viewDef );
}

/*
=============
AdvanceSurf
=============
*/
void idGuiModel::AdvanceSurf() {
	guiModelSurface_t	s;

	if ( surfaces.Num() ) {
		s.color[0] = surf->color[0];
		s.color[1] = surf->color[1];
		s.color[2] = surf->color[2];
		s.color[3] = surf->color[3];
		s.material = surf->material;
	} else {
		s.color[0] = 1;
		s.color[1] = 1;
		s.color[2] = 1;
		s.color[3] = 1;
		s.material = tr.defaultMaterial;
	}
	s.numIndexes = 0;
	s.firstIndex = indexes.Num();
	s.numVerts = 0;
	s.firstVert = verts.Num();

	surfaces.Append( s );
	surf = &surfaces[ surfaces.Num() - 1 ];
}

/*
=============
SetColor
=============
*/
void idGuiModel::SetColor( float r, float g, float b, float a ) {
	if ( !glConfig.isInitialized ) {
		return;
	}
	if ( r == surf->color[0] && g == surf->color[1]
		&& b == surf->color[2] && a == surf->color[3] ) {
		return;	// no change
	}

	if ( surf->numVerts ) {
		AdvanceSurf();
	}

	// change the parms
	surf->color[0] = r;
	surf->color[1] = g;
	surf->color[2] = b;
	surf->color[3] = a;
}

/*
=============
DrawStretchPic
=============
*/
void idGuiModel::DrawStretchPic( const idDrawVert *dverts, const glIndex_t *dindexes, int vertCount, int indexCount, const idMaterial *hShader, 
									   bool clip, float min_x, float min_y, float max_x, float max_y ) {
	if ( !glConfig.isInitialized ) {
		return;
	}
	if ( !( dverts && dindexes && vertCount && indexCount && hShader ) ) {
		return;
	}

	// break the current surface if we are changing to a new material
	if ( hShader != surf->material ) {
		if ( surf->numVerts ) {
			AdvanceSurf();
		}
		const_cast<idMaterial *>(hShader)->EnsureNotPurged();	// in case it was a gui item started before a level change
		surf->material = hShader;
	}

	// add the verts and indexes to the current surface

	if ( clip ) {
		int i, j;

		// FIXME:	this is grim stuff, and should be rewritten if we have any significant
		//			number of guis asking for clipping
		idFixedWinding w;
		for ( i = 0; i < indexCount; i += 3 ) {
			w.Clear();
			w.AddPoint(idVec5(dverts[dindexes[i]].xyz.x, dverts[dindexes[i]].xyz.y, dverts[dindexes[i]].xyz.z, dverts[dindexes[i]].st.x, dverts[dindexes[i]].st.y));
			w.AddPoint(idVec5(dverts[dindexes[i+1]].xyz.x, dverts[dindexes[i+1]].xyz.y, dverts[dindexes[i+1]].xyz.z, dverts[dindexes[i+1]].st.x, dverts[dindexes[i+1]].st.y));
			w.AddPoint(idVec5(dverts[dindexes[i+2]].xyz.x, dverts[dindexes[i+2]].xyz.y, dverts[dindexes[i+2]].xyz.z, dverts[dindexes[i+2]].st.x, dverts[dindexes[i+2]].st.y));

			for ( j = 0; j < 3; j++ ) {
				if ( w[j].x < min_x || w[j].x > max_x ||
					w[j].y < min_y || w[j].y > max_y ) {
					break;
				}
			}
			if ( j < 3 ) {
				idPlane p;
				p.Normal().y = p.Normal().z = 0.0f; p.Normal().x = 1.0f; p.SetDist( min_x );
				w.ClipInPlace( p );
				p.Normal().y = p.Normal().z = 0.0f; p.Normal().x = -1.0f; p.SetDist( -max_x );
				w.ClipInPlace( p );
				p.Normal().x = p.Normal().z = 0.0f; p.Normal().y = 1.0f; p.SetDist( min_y );
				w.ClipInPlace( p );
				p.Normal().x = p.Normal().z = 0.0f; p.Normal().y = -1.0f; p.SetDist( -max_y );
				w.ClipInPlace( p );
			}

			int	numVerts = verts.Num();
			verts.SetNum( numVerts + w.GetNumPoints(), false );
			for ( j = 0 ; j < w.GetNumPoints() ; j++ ) {
				idDrawVert *dv = &verts[numVerts+j];

				dv->xyz.x = w[j].x;
				dv->xyz.y = w[j].y;
				dv->xyz.z = w[j].z;
				dv->st.x = w[j].s;
				dv->st.y = w[j].t;
				dv->normal.Set(0, 0, 1);
				dv->tangents[0].Set(1, 0, 0);
				dv->tangents[1].Set(0, 1, 0);
			}
			surf->numVerts += w.GetNumPoints();

			for ( j = 2; j < w.GetNumPoints(); j++ ) {
				indexes.Append( numVerts - surf->firstVert );
				indexes.Append( numVerts + j - 1 - surf->firstVert );
				indexes.Append( numVerts + j - surf->firstVert );
				surf->numIndexes += 3;
			}
		}

	} else {

		int numVerts = verts.Num();
		int numIndexes = indexes.Num();

		verts.AssureSize( numVerts + vertCount );
		indexes.AssureSize( numIndexes + indexCount );

		surf->numVerts += vertCount;
		surf->numIndexes += indexCount;

		for ( int i = 0; i < indexCount; i++ ) {
			indexes[numIndexes + i] = numVerts + dindexes[i] - surf->firstVert;
		}

		memcpy( &verts[numVerts], dverts, vertCount * sizeof( verts[0] ) );
	}
}

/*
=============
DrawStretchPic

x/y/w/h are in the 0,0 to 640,480 range
=============
*/
void idGuiModel::DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, const idMaterial *hShader ) {
	idDrawVert verts[4];
	glIndex_t indexes[6];

	if ( !glConfig.isInitialized ) {
		return;
	}
	if ( !hShader ) {
		return;
	}

	// clip to edges, because the pic may be going into a guiShader
	// instead of full screen
	if ( x < 0 ) {
		s1 += ( s2 - s1 ) * -x / w;
		w += x;
		x = 0;
	}
	if ( y < 0 ) {
		t1 += ( t2 - t1 ) * -y / h;
		h += y;
		y = 0;
	}
	if ( x + w > 640 ) {
		s2 -= ( s2 - s1 ) * ( x + w - 640 ) / w;
		w = 640 - x;
	}
	if ( y + h > 480 ) {
		t2 -= ( t2 - t1 ) * ( y + h - 480 ) / h;
		h = 480 - y;
	}
	
	if ( w <= 0 || h <= 0 ) {
		return;		// completely clipped away
	}

	indexes[0] = 3;
	indexes[1] = 0;
	indexes[2] = 2;
	indexes[3] = 2;
	indexes[4] = 0;
	indexes[5] = 1;
	verts[0].xyz[0] = x;
	verts[0].xyz[1] = y;
	verts[0].xyz[2] = 0;
	verts[0].st[0] = s1;
	verts[0].st[1] = t1;
	verts[0].normal[0] = 0;
	verts[0].normal[1] = 0;
	verts[0].normal[2] = 1;
	verts[0].tangents[0][0] = 1;
	verts[0].tangents[0][1] = 0;
	verts[0].tangents[0][2] = 0;
	verts[0].tangents[1][0] = 0;
	verts[0].tangents[1][1] = 1;
	verts[0].tangents[1][2] = 0;
	verts[1].xyz[0] = x + w;
	verts[1].xyz[1] = y;
	verts[1].xyz[2] = 0;
	verts[1].st[0] = s2;
	verts[1].st[1] = t1;
	verts[1].normal[0] = 0;
	verts[1].normal[1] = 0;
	verts[1].normal[2] = 1;
	verts[1].tangents[0][0] = 1;
	verts[1].tangents[0][1] = 0;
	verts[1].tangents[0][2] = 0;
	verts[1].tangents[1][0] = 0;
	verts[1].tangents[1][1] = 1;
	verts[1].tangents[1][2] = 0;
	verts[2].xyz[0] = x + w;
	verts[2].xyz[1] = y + h;
	verts[2].xyz[2] = 0;
	verts[2].st[0] = s2;
	verts[2].st[1] = t2;
	verts[2].normal[0] = 0;
	verts[2].normal[1] = 0;
	verts[2].normal[2] = 1;
	verts[2].tangents[0][0] = 1;
	verts[2].tangents[0][1] = 0;
	verts[2].tangents[0][2] = 0;
	verts[2].tangents[1][0] = 0;
	verts[2].tangents[1][1] = 1;
	verts[2].tangents[1][2] = 0;
	verts[3].xyz[0] = x;
	verts[3].xyz[1] = y + h;
	verts[3].xyz[2] = 0;
	verts[3].st[0] = s1;
	verts[3].st[1] = t2;
	verts[3].normal[0] = 0;
	verts[3].normal[1] = 0;
	verts[3].normal[2] = 1;
	verts[3].tangents[0][0] = 1;
	verts[3].tangents[0][1] = 0;
	verts[3].tangents[0][2] = 0;
	verts[3].tangents[1][0] = 0;
	verts[3].tangents[1][1] = 1;
	verts[3].tangents[1][2] = 0;

	DrawStretchPic( &verts[0], &indexes[0], 4, 6, hShader, false, 0.0f, 0.0f, 640.0f, 480.0f );
}

/*
=============
DrawStretchTri

x/y/w/h are in the 0,0 to 640,480 range
=============
*/
void idGuiModel::DrawStretchTri( idVec2 p1, idVec2 p2, idVec2 p3, idVec2 t1, idVec2 t2, idVec2 t3, const idMaterial *material ) {
	idDrawVert tempVerts[3];
	glIndex_t tempIndexes[3];
	int vertCount = 3;
	int indexCount = 3;

	if ( !glConfig.isInitialized ) {
		return;
	}
	if ( !material ) {
		return;
	}

	tempIndexes[0] = 1;
	tempIndexes[1] = 0;
	tempIndexes[2] = 2;
	tempVerts[0].xyz[0] = p1.x;
	tempVerts[0].xyz[1] = p1.y;
	tempVerts[0].xyz[2] = 0;
	tempVerts[0].st[0] = t1.x;
	tempVerts[0].st[1] = t1.y;
	tempVerts[0].normal[0] = 0;
	tempVerts[0].normal[1] = 0;
	tempVerts[0].normal[2] = 1;
	tempVerts[0].tangents[0][0] = 1;
	tempVerts[0].tangents[0][1] = 0;
	tempVerts[0].tangents[0][2] = 0;
	tempVerts[0].tangents[1][0] = 0;
	tempVerts[0].tangents[1][1] = 1;
	tempVerts[0].tangents[1][2] = 0;
	tempVerts[1].xyz[0] = p2.x;
	tempVerts[1].xyz[1] = p2.y;
	tempVerts[1].xyz[2] = 0;
	tempVerts[1].st[0] = t2.x;
	tempVerts[1].st[1] = t2.y;
	tempVerts[1].normal[0] = 0;
	tempVerts[1].normal[1] = 0;
	tempVerts[1].normal[2] = 1;
	tempVerts[1].tangents[0][0] = 1;
	tempVerts[1].tangents[0][1] = 0;
	tempVerts[1].tangents[0][2] = 0;
	tempVerts[1].tangents[1][0] = 0;
	tempVerts[1].tangents[1][1] = 1;
	tempVerts[1].tangents[1][2] = 0;
	tempVerts[2].xyz[0] = p3.x;
	tempVerts[2].xyz[1] = p3.y;
	tempVerts[2].xyz[2] = 0;
	tempVerts[2].st[0] = t3.x;
	tempVerts[2].st[1] = t3.y;
	tempVerts[2].normal[0] = 0;
	tempVerts[2].normal[1] = 0;
	tempVerts[2].normal[2] = 1;
	tempVerts[2].tangents[0][0] = 1;
	tempVerts[2].tangents[0][1] = 0;
	tempVerts[2].tangents[0][2] = 0;
	tempVerts[2].tangents[1][0] = 0;
	tempVerts[2].tangents[1][1] = 1;
	tempVerts[2].tangents[1][2] = 0;

	// break the current surface if we are changing to a new material
	if ( material != surf->material ) {
		if ( surf->numVerts ) {
			AdvanceSurf();
		}
		const_cast<idMaterial *>(material)->EnsureNotPurged();	// in case it was a gui item started before a level change
		surf->material = material;
	}


	int numVerts = verts.Num();
	int numIndexes = indexes.Num();

	verts.AssureSize( numVerts + vertCount );
	indexes.AssureSize( numIndexes + indexCount );

	surf->numVerts += vertCount;
	surf->numIndexes += indexCount;

	for ( int i = 0; i < indexCount; i++ ) {
		indexes[numIndexes + i] = numVerts + tempIndexes[i] - surf->firstVert;
	}

	memcpy( &verts[numVerts], tempVerts, vertCount * sizeof( verts[0] ) );
}


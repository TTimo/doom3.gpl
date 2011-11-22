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

#include "Model_local.h"
#include "tr_local.h"


/*
====================
idRenderModelOverlay::idRenderModelOverlay
====================
*/
idRenderModelOverlay::idRenderModelOverlay() {
}

/*
====================
idRenderModelOverlay::~idRenderModelOverlay
====================
*/
idRenderModelOverlay::~idRenderModelOverlay() {
	int i, k;

	for ( k = 0; k < materials.Num(); k++ ) {
		for ( i = 0; i < materials[k]->surfaces.Num(); i++ ) {
			FreeSurface( materials[k]->surfaces[i] );
		}
		materials[k]->surfaces.Clear();
		delete materials[k];
	}
	materials.Clear();
}

/*
====================
idRenderModelOverlay::Alloc
====================
*/
idRenderModelOverlay *idRenderModelOverlay::Alloc( void ) {
	return new idRenderModelOverlay;
}

/*
====================
idRenderModelOverlay::Free
====================
*/
void idRenderModelOverlay::Free( idRenderModelOverlay *overlay ) {
	delete overlay;
}

/*
====================
idRenderModelOverlay::FreeSurface
====================
*/
void idRenderModelOverlay::FreeSurface( overlaySurface_t *surface ) {
	if ( surface->verts ) {
		Mem_Free( surface->verts );
	}
	if ( surface->indexes ) {
		Mem_Free( surface->indexes );
	}
	Mem_Free( surface );
}

/*
=====================
idRenderModelOverlay::CreateOverlay

This projects on both front and back sides to avoid seams
The material should be clamped, because entire triangles are added, some of which
may extend well past the 0.0 to 1.0 texture range
=====================
*/
void idRenderModelOverlay::CreateOverlay( const idRenderModel *model, const idPlane localTextureAxis[2], const idMaterial *mtr ) {
	int i, maxVerts, maxIndexes, surfNum;
	idRenderModelOverlay *overlay = NULL;

	// count up the maximum possible vertices and indexes per surface
	maxVerts = 0;
	maxIndexes = 0;
	for ( surfNum = 0; surfNum < model->NumSurfaces(); surfNum++ ) {
		const modelSurface_t *surf = model->Surface( surfNum );
		if ( surf->geometry->numVerts > maxVerts ) {
			maxVerts = surf->geometry->numVerts;
		}
		if ( surf->geometry->numIndexes > maxIndexes ) {
			maxIndexes = surf->geometry->numIndexes;
		}
	}

	// make temporary buffers for the building process
	overlayVertex_t	*overlayVerts = (overlayVertex_t *)_alloca( maxVerts * sizeof( *overlayVerts ) );
	glIndex_t *overlayIndexes = (glIndex_t *)_alloca16( maxIndexes * sizeof( *overlayIndexes ) );

	// pull out the triangles we need from the base surfaces
	for ( surfNum = 0; surfNum < model->NumBaseSurfaces(); surfNum++ ) {
		const modelSurface_t *surf = model->Surface( surfNum );
		float d;

		if ( !surf->geometry || !surf->shader ) {
			continue;
		}

		// some surfaces can explicitly disallow overlays
		if ( !surf->shader->AllowOverlays() ) {
			continue;
		}

		const srfTriangles_t *stri = surf->geometry;

		// try to cull the whole surface along the first texture axis
		d = stri->bounds.PlaneDistance( localTextureAxis[0] );
		if ( d < 0.0f || d > 1.0f ) {
			continue;
		}

		// try to cull the whole surface along the second texture axis
		d = stri->bounds.PlaneDistance( localTextureAxis[1] );
		if ( d < 0.0f || d > 1.0f ) {
			continue;
		}

		byte *cullBits = (byte *)_alloca16( stri->numVerts * sizeof( cullBits[0] ) );
		idVec2 *texCoords = (idVec2 *)_alloca16( stri->numVerts * sizeof( texCoords[0] ) );

		SIMDProcessor->OverlayPointCull( cullBits, texCoords, localTextureAxis, stri->verts, stri->numVerts );

		glIndex_t *vertexRemap = (glIndex_t *)_alloca16( sizeof( vertexRemap[0] ) * stri->numVerts );
		SIMDProcessor->Memset( vertexRemap, -1,  sizeof( vertexRemap[0] ) * stri->numVerts );

		// find triangles that need the overlay
		int numVerts = 0;
		int numIndexes = 0;
		int triNum = 0;
		for ( int index = 0; index < stri->numIndexes; index += 3, triNum++ ) {
			int v1 = stri->indexes[index+0];
			int	v2 = stri->indexes[index+1];
			int v3 = stri->indexes[index+2];

			// skip triangles completely off one side
			if ( cullBits[v1] & cullBits[v2] & cullBits[v3] ) {
				continue;
			}

			// we could do more precise triangle culling, like the light interaction does, if desired

			// keep this triangle
			for ( int vnum = 0; vnum < 3; vnum++ ) {
				int ind = stri->indexes[index+vnum];
				if ( vertexRemap[ind] == (glIndex_t)-1 ) {
					vertexRemap[ind] = numVerts;

					overlayVerts[numVerts].vertexNum = ind;
					overlayVerts[numVerts].st[0] = texCoords[ind][0];
					overlayVerts[numVerts].st[1] = texCoords[ind][1];

					numVerts++;
				}
				overlayIndexes[numIndexes++] = vertexRemap[ind];
			}
		}

		if ( !numIndexes ) {
			continue;
		}

		overlaySurface_t *s = (overlaySurface_t *) Mem_Alloc( sizeof( overlaySurface_t ) );
		s->surfaceNum = surfNum;
		s->surfaceId = surf->id;
		s->verts = (overlayVertex_t *)Mem_Alloc( numVerts * sizeof( s->verts[0] ) );
		memcpy( s->verts, overlayVerts, numVerts * sizeof( s->verts[0] ) );
		s->numVerts = numVerts;
		s->indexes = (glIndex_t *)Mem_Alloc( numIndexes * sizeof( s->indexes[0] ) );
		memcpy( s->indexes, overlayIndexes, numIndexes * sizeof( s->indexes[0] ) );
		s->numIndexes = numIndexes;

		for ( i = 0; i < materials.Num(); i++ ) {
			if ( materials[i]->material == mtr ) {
				break;
			}
		}
		if ( i < materials.Num() ) {
            materials[i]->surfaces.Append( s );
		} else {
			overlayMaterial_t *mat = new overlayMaterial_t;
			mat->material = mtr;
			mat->surfaces.Append( s );
			materials.Append( mat );
		}
	}

	// remove the oldest overlay surfaces if there are too many per material
	for ( i = 0; i < materials.Num(); i++ ) {
		while( materials[i]->surfaces.Num() > MAX_OVERLAY_SURFACES ) {
			FreeSurface( materials[i]->surfaces[0] );
			materials[i]->surfaces.RemoveIndex( 0 );
		}
	}
}

/*
====================
idRenderModelOverlay::AddOverlaySurfacesToModel
====================
*/
void idRenderModelOverlay::AddOverlaySurfacesToModel( idRenderModel *baseModel ) {
	int i, j, k, numVerts, numIndexes, surfaceNum;
	const modelSurface_t *baseSurf;
	idRenderModelStatic *staticModel;
	overlaySurface_t *surf;
	srfTriangles_t *newTri;
	modelSurface_t *newSurf;

	if ( baseModel == NULL || baseModel->IsDefaultModel() ) {
		return;
	}

	// md5 models won't have any surfaces when r_showSkel is set
	if ( !baseModel->NumSurfaces() ) {
		return;
	}

	if ( baseModel->IsDynamicModel() != DM_STATIC ) {
		common->Error( "idRenderModelOverlay::AddOverlaySurfacesToModel: baseModel is not a static model" );
	}

	assert( dynamic_cast<idRenderModelStatic *>(baseModel) != NULL );
	staticModel = static_cast<idRenderModelStatic *>(baseModel);

	staticModel->overlaysAdded = 0;

	if ( !materials.Num() ) {
		staticModel->DeleteSurfacesWithNegativeId();
		return;
	}

	for ( k = 0; k < materials.Num(); k++ ) {

		numVerts = numIndexes = 0;
		for ( i = 0; i < materials[k]->surfaces.Num(); i++ ) {
			numVerts += materials[k]->surfaces[i]->numVerts;
			numIndexes += materials[k]->surfaces[i]->numIndexes;
		}

		if ( staticModel->FindSurfaceWithId( -1 - k, surfaceNum ) ) {
			newSurf = &staticModel->surfaces[surfaceNum];
		} else {
			newSurf = &staticModel->surfaces.Alloc();
			newSurf->geometry = NULL;
			newSurf->shader = materials[k]->material;
			newSurf->id = -1 - k;
		}

		if ( newSurf->geometry == NULL || newSurf->geometry->numVerts < numVerts || newSurf->geometry->numIndexes < numIndexes ) {
			R_FreeStaticTriSurf( newSurf->geometry );
			newSurf->geometry = R_AllocStaticTriSurf();
			R_AllocStaticTriSurfVerts( newSurf->geometry, numVerts );
			R_AllocStaticTriSurfIndexes( newSurf->geometry, numIndexes );
			SIMDProcessor->Memset( newSurf->geometry->verts, 0, numVerts * sizeof( newTri->verts[0] ) );
		} else {
			R_FreeStaticTriSurfVertexCaches( newSurf->geometry );
		}

		newTri = newSurf->geometry;
		numVerts = numIndexes = 0;

		for ( i = 0; i < materials[k]->surfaces.Num(); i++ ) {
			surf = materials[k]->surfaces[i];

			// get the model surface for this overlay surface
			if ( surf->surfaceNum < staticModel->NumSurfaces() ) {
				baseSurf = staticModel->Surface( surf->surfaceNum );
			} else {
				baseSurf = NULL;
			}

			// if the surface ids no longer match
			if ( !baseSurf || baseSurf->id != surf->surfaceId ) {
				// find the surface with the correct id
				if ( staticModel->FindSurfaceWithId( surf->surfaceId, surf->surfaceNum ) ) {
					baseSurf = staticModel->Surface( surf->surfaceNum );
				} else {
					// the surface with this id no longer exists
					FreeSurface( surf );
					materials[k]->surfaces.RemoveIndex( i );
					i--;
					continue;
				}
			}

			// copy indexes;
			for ( j = 0; j < surf->numIndexes; j++ ) {
				newTri->indexes[numIndexes + j] = numVerts + surf->indexes[j];
			}
			numIndexes += surf->numIndexes;

			// copy vertices
			for ( j = 0; j < surf->numVerts; j++ ) {
				overlayVertex_t *overlayVert = &surf->verts[j];

				newTri->verts[numVerts].st[0] = overlayVert->st[0];
				newTri->verts[numVerts].st[1] = overlayVert->st[1];

				if ( overlayVert->vertexNum >= baseSurf->geometry->numVerts ) {
					// This can happen when playing a demofile and a model has been changed since it was recorded, so just issue a warning and go on.
					common->Warning( "idRenderModelOverlay::AddOverlaySurfacesToModel: overlay vertex out of range.  Model has probably changed since generating the overlay." );
					FreeSurface( surf );
					materials[k]->surfaces.RemoveIndex( i );
					staticModel->DeleteSurfaceWithId( newSurf->id );
					return;
				}
				newTri->verts[numVerts].xyz = baseSurf->geometry->verts[overlayVert->vertexNum].xyz;
				numVerts++;
			}
		}

		newTri->numVerts = numVerts;
		newTri->numIndexes = numIndexes;
		R_BoundTriSurf( newTri );

		staticModel->overlaysAdded++;	// so we don't create an overlay on an overlay surface
	}
}

/*
====================
idRenderModelOverlay::RemoveOverlaySurfacesFromModel
====================
*/
void idRenderModelOverlay::RemoveOverlaySurfacesFromModel( idRenderModel *baseModel ) {
	idRenderModelStatic *staticModel;

	assert( dynamic_cast<idRenderModelStatic *>(baseModel) != NULL );
	staticModel = static_cast<idRenderModelStatic *>(baseModel);

	staticModel->DeleteSurfacesWithNegativeId();
	staticModel->overlaysAdded = 0;
}

/*
====================
idRenderModelOverlay::ReadFromDemoFile
====================
*/
void idRenderModelOverlay::ReadFromDemoFile( idDemoFile *f ) {
	// FIXME: implement
}

/*
====================
idRenderModelOverlay::WriteToDemoFile
====================
*/
void idRenderModelOverlay::WriteToDemoFile( idDemoFile *f ) const {
	// FIXME: implement
}

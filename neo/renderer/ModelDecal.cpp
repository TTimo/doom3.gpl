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
#include "Model_local.h"

// decalFade	filter 5 0.1
// polygonOffset
// {
// map invertColor( textures/splat )
// blend GL_ZERO GL_ONE_MINUS_SRC
// vertexColor
// clamp
// }

/*
==================
idRenderModelDecal::idRenderModelDecal
==================
*/
idRenderModelDecal::idRenderModelDecal( void ) {
	memset( &tri, 0, sizeof( tri ) );
	tri.verts = verts;
	tri.indexes = indexes;
	material = NULL;
	nextDecal = NULL;
}

/*
==================
idRenderModelDecal::~idRenderModelDecal
==================
*/
idRenderModelDecal::~idRenderModelDecal( void ) {
}

/*
==================
idRenderModelDecal::idRenderModelDecal
==================
*/
idRenderModelDecal *idRenderModelDecal::Alloc( void ) {
	return new idRenderModelDecal;
}

/*
==================
idRenderModelDecal::idRenderModelDecal
==================
*/
void idRenderModelDecal::Free( idRenderModelDecal *decal ) {
	delete decal;
}

/*
=================
idRenderModelDecal::CreateProjectionInfo
=================
*/
bool idRenderModelDecal::CreateProjectionInfo( decalProjectionInfo_t &info, const idFixedWinding &winding, const idVec3 &projectionOrigin, const bool parallel, const float fadeDepth, const idMaterial *material, const int startTime ) {

	if ( winding.GetNumPoints() != NUM_DECAL_BOUNDING_PLANES - 2 ) {
		common->Printf( "idRenderModelDecal::CreateProjectionInfo: winding must have %d points\n", NUM_DECAL_BOUNDING_PLANES - 2 );
		return false;
	}

	assert( material != NULL );

	info.projectionOrigin = projectionOrigin;
	info.material = material;
	info.parallel = parallel;
	info.fadeDepth = fadeDepth;
	info.startTime = startTime;
	info.force = false;

	// get the winding plane and the depth of the projection volume
	idPlane windingPlane;
	winding.GetPlane( windingPlane );
	float depth = windingPlane.Distance( projectionOrigin );

	// find the bounds for the projection
	winding.GetBounds( info.projectionBounds );
	if ( parallel ) {
		info.projectionBounds.ExpandSelf( depth );
	} else {
		info.projectionBounds.AddPoint( projectionOrigin );
	}

	// calculate the world space projection volume bounding planes, positive sides face outside the decal
	if ( parallel ) {
		for ( int i = 0; i < winding.GetNumPoints(); i++ ) {
			idVec3 edge = winding[(i+1)%winding.GetNumPoints()].ToVec3() - winding[i].ToVec3();
			info.boundingPlanes[i].Normal().Cross( windingPlane.Normal(), edge );
			info.boundingPlanes[i].Normalize();
			info.boundingPlanes[i].FitThroughPoint( winding[i].ToVec3() );
		}
	} else {
		for ( int i = 0; i < winding.GetNumPoints(); i++ ) {
			info.boundingPlanes[i].FromPoints( projectionOrigin, winding[i].ToVec3(), winding[(i+1)%winding.GetNumPoints()].ToVec3() );
		}
	}
	info.boundingPlanes[NUM_DECAL_BOUNDING_PLANES - 2] = windingPlane;
	info.boundingPlanes[NUM_DECAL_BOUNDING_PLANES - 2][3] -= depth;
	info.boundingPlanes[NUM_DECAL_BOUNDING_PLANES - 1] = -windingPlane;

	// fades will be from these plane
	info.fadePlanes[0] = windingPlane;
	info.fadePlanes[0][3] -= fadeDepth;
	info.fadePlanes[1] = -windingPlane;
	info.fadePlanes[1][3] += depth - fadeDepth;

	// calculate the texture vectors for the winding
	float	len, texArea, inva;
	idVec3	temp;
	idVec5	d0, d1;

	const idVec5 &a = winding[0];
	const idVec5 &b = winding[1];
	const idVec5 &c = winding[2];

	d0 = b.ToVec3() - a.ToVec3();
	d0.s = b.s - a.s;
	d0.t = b.t - a.t;
	d1 = c.ToVec3() - a.ToVec3();
	d1.s = c.s - a.s;
	d1.t = c.t - a.t;

	texArea = ( d0[3] * d1[4] ) - ( d0[4] * d1[3] );
	inva = 1.0f / texArea;

    temp[0] = ( d0[0] * d1[4] - d0[4] * d1[0] ) * inva;
    temp[1] = ( d0[1] * d1[4] - d0[4] * d1[1] ) * inva;
    temp[2] = ( d0[2] * d1[4] - d0[4] * d1[2] ) * inva;
	len = temp.Normalize();
	info.textureAxis[0].Normal() = temp * ( 1.0f / len );
	info.textureAxis[0][3] = winding[0].s - ( winding[0].ToVec3() * info.textureAxis[0].Normal() );

    temp[0] = ( d0[3] * d1[0] - d0[0] * d1[3] ) * inva;
    temp[1] = ( d0[3] * d1[1] - d0[1] * d1[3] ) * inva;
    temp[2] = ( d0[3] * d1[2] - d0[2] * d1[3] ) * inva;
	len = temp.Normalize();
	info.textureAxis[1].Normal() = temp * ( 1.0f / len );
	info.textureAxis[1][3] = winding[0].t - ( winding[0].ToVec3() * info.textureAxis[1].Normal() );

	return true;
}

/*
=================
idRenderModelDecal::CreateProjectionInfo
=================
*/
void idRenderModelDecal::GlobalProjectionInfoToLocal( decalProjectionInfo_t &localInfo, const decalProjectionInfo_t &info, const idVec3 &origin, const idMat3 &axis ) {
	float modelMatrix[16];

	R_AxisToModelMatrix( axis, origin, modelMatrix );

	for ( int j = 0; j < NUM_DECAL_BOUNDING_PLANES; j++ ) {
		R_GlobalPlaneToLocal( modelMatrix, info.boundingPlanes[j], localInfo.boundingPlanes[j] );
	}
	R_GlobalPlaneToLocal( modelMatrix, info.fadePlanes[0], localInfo.fadePlanes[0] );
	R_GlobalPlaneToLocal( modelMatrix, info.fadePlanes[1], localInfo.fadePlanes[1] );
	R_GlobalPlaneToLocal( modelMatrix, info.textureAxis[0], localInfo.textureAxis[0] );
	R_GlobalPlaneToLocal( modelMatrix, info.textureAxis[1], localInfo.textureAxis[1] );
	R_GlobalPointToLocal( modelMatrix, info.projectionOrigin, localInfo.projectionOrigin );
	localInfo.projectionBounds = info.projectionBounds;
	localInfo.projectionBounds.TranslateSelf( -origin );
	localInfo.projectionBounds.RotateSelf( axis.Transpose() );
	localInfo.material = info.material;
	localInfo.parallel = info.parallel;
	localInfo.fadeDepth = info.fadeDepth;
	localInfo.startTime = info.startTime;
	localInfo.force = info.force;
}

/*
=================
idRenderModelDecal::AddWinding
=================
*/
void idRenderModelDecal::AddWinding( const idWinding &w, const idMaterial *decalMaterial, const idPlane fadePlanes[2], float fadeDepth, int startTime ) {
	int i;
	float invFadeDepth, fade;
	decalInfo_t	decalInfo;

	if ( ( material == NULL || material == decalMaterial ) &&
			tri.numVerts + w.GetNumPoints() < MAX_DECAL_VERTS &&
				tri.numIndexes + ( w.GetNumPoints() - 2 ) * 3 < MAX_DECAL_INDEXES ) {

		material = decalMaterial;

		// add to this decal
		decalInfo = material->GetDecalInfo();
		invFadeDepth = -1.0f / fadeDepth;

		for ( i = 0; i < w.GetNumPoints(); i++ ) {
			fade = fadePlanes[0].Distance( w[i].ToVec3() ) * invFadeDepth;
			if ( fade < 0.0f ) {
				fade = fadePlanes[1].Distance( w[i].ToVec3() ) * invFadeDepth;
			}
			if ( fade < 0.0f ) {
				fade = 0.0f;
			} else if ( fade > 0.99f ) {
				fade = 1.0f;
			}
			fade = 1.0f - fade;
			vertDepthFade[tri.numVerts + i] = fade;
			tri.verts[tri.numVerts + i].xyz = w[i].ToVec3();
			tri.verts[tri.numVerts + i].st[0] = w[i].s;
			tri.verts[tri.numVerts + i].st[1] = w[i].t;
			for ( int k = 0 ; k < 4 ; k++ ) {
				int icolor = idMath::FtoiFast( decalInfo.start[k] * fade * 255.0f );
				if ( icolor < 0 ) {
					icolor = 0;
				} else if ( icolor > 255 ) {
					icolor = 255;
				}
				tri.verts[tri.numVerts + i].color[k] = icolor;
			}
		}
		for ( i = 2; i < w.GetNumPoints(); i++ ) {
			tri.indexes[tri.numIndexes + 0] = tri.numVerts;
			tri.indexes[tri.numIndexes + 1] = tri.numVerts + i - 1;
			tri.indexes[tri.numIndexes + 2] = tri.numVerts + i;
			indexStartTime[tri.numIndexes] =
			indexStartTime[tri.numIndexes + 1] =
			indexStartTime[tri.numIndexes + 2] = startTime;
			tri.numIndexes += 3;
		}
		tri.numVerts += w.GetNumPoints();
		return;
	}

	// if we are at the end of the list, create a new decal
	if ( !nextDecal ) {
		nextDecal = idRenderModelDecal::Alloc();
	}
	// let the next decal on the chain take a look
	nextDecal->AddWinding( w, decalMaterial, fadePlanes, fadeDepth, startTime );
}

/*
=================
idRenderModelDecal::AddDepthFadedWinding
=================
*/
void idRenderModelDecal::AddDepthFadedWinding( const idWinding &w, const idMaterial *decalMaterial, const idPlane fadePlanes[2], float fadeDepth, int startTime ) {
	idFixedWinding front, back;

	front = w;
	if ( front.Split( &back, fadePlanes[0], 0.1f ) == SIDE_CROSS ) {
		AddWinding( back, decalMaterial, fadePlanes, fadeDepth, startTime );
	}

	if ( front.Split( &back, fadePlanes[1], 0.1f ) == SIDE_CROSS ) {
		AddWinding( back, decalMaterial, fadePlanes, fadeDepth, startTime );
	}

	AddWinding( front, decalMaterial, fadePlanes, fadeDepth, startTime );
}

/*
=================
idRenderModelDecal::CreateDecal
=================
*/
void idRenderModelDecal::CreateDecal( const idRenderModel *model, const decalProjectionInfo_t &localInfo ) {

	// check all model surfaces
	for ( int surfNum = 0; surfNum < model->NumSurfaces(); surfNum++ ) {
		const modelSurface_t *surf = model->Surface( surfNum );

		// if no geometry or no shader
		if ( !surf->geometry || !surf->shader ) {
			continue;
		}

		// decals and overlays use the same rules
		if ( !localInfo.force && !surf->shader->AllowOverlays() ) {
			continue;
		}

		srfTriangles_t *stri = surf->geometry;

		// if the triangle bounds do not overlap with projection bounds
		if ( !localInfo.projectionBounds.IntersectsBounds( stri->bounds ) ) {
			continue;
		}

		// allocate memory for the cull bits
		byte *cullBits = (byte *)_alloca16( stri->numVerts * sizeof( cullBits[0] ) );

		// catagorize all points by the planes
		SIMDProcessor->DecalPointCull( cullBits, localInfo.boundingPlanes, stri->verts, stri->numVerts );

		// find triangles inside the projection volume
		for ( int triNum = 0, index = 0; index < stri->numIndexes; index += 3, triNum++ ) {
			int v1 = stri->indexes[index+0];
			int v2 = stri->indexes[index+1];
			int v3 = stri->indexes[index+2];

			// skip triangles completely off one side
			if ( cullBits[v1] & cullBits[v2] & cullBits[v3] ) {
				continue;
			}

			// skip back facing triangles
			if ( stri->facePlanes && stri->facePlanesCalculated &&
					stri->facePlanes[triNum].Normal() * localInfo.boundingPlanes[NUM_DECAL_BOUNDING_PLANES - 2].Normal() < -0.1f ) {
				continue;
			}

			// create a winding with texture coordinates for the triangle
			idFixedWinding fw;
			fw.SetNumPoints( 3 );
			if ( localInfo.parallel ) {
				for ( int j = 0; j < 3; j++ ) {
					fw[j] = stri->verts[stri->indexes[index+j]].xyz;
					fw[j].s = localInfo.textureAxis[0].Distance( fw[j].ToVec3() );
					fw[j].t = localInfo.textureAxis[1].Distance( fw[j].ToVec3() );
				}
			} else {
				for ( int j = 0; j < 3; j++ ) {
					idVec3 dir;
					float scale;

					fw[j] = stri->verts[stri->indexes[index+j]].xyz;
					dir = fw[j].ToVec3() - localInfo.projectionOrigin;
					localInfo.boundingPlanes[NUM_DECAL_BOUNDING_PLANES - 1].RayIntersection( fw[j].ToVec3(), dir, scale );
					dir = fw[j].ToVec3() + scale * dir;
					fw[j].s = localInfo.textureAxis[0].Distance( dir );
					fw[j].t = localInfo.textureAxis[1].Distance( dir );
				}
			}

			int orBits = cullBits[v1] | cullBits[v2] | cullBits[v3];

			// clip the exact surface triangle to the projection volume
			for ( int j = 0; j < NUM_DECAL_BOUNDING_PLANES; j++ ) {
				if ( orBits & ( 1 << j ) ) {
					if ( !fw.ClipInPlace( -localInfo.boundingPlanes[j] ) ) {
						break;
					}
				}
			}

			if ( fw.GetNumPoints() == 0 ) {
				continue;
			}

			AddDepthFadedWinding( fw, localInfo.material, localInfo.fadePlanes, localInfo.fadeDepth, localInfo.startTime );
		}
	}
}

/*
=====================
idRenderModelDecal::RemoveFadedDecals
=====================
*/
idRenderModelDecal *idRenderModelDecal::RemoveFadedDecals( idRenderModelDecal *decals, int time ) {
	int i, j, minTime, newNumIndexes, newNumVerts;
	int inUse[MAX_DECAL_VERTS];
	decalInfo_t	decalInfo;
	idRenderModelDecal *nextDecal;

	if ( decals == NULL ) {
		return NULL;
	}

	// recursively free any next decals
	decals->nextDecal = RemoveFadedDecals( decals->nextDecal, time );

	// free the decals if no material set
	if ( decals->material == NULL ) {
		nextDecal = decals->nextDecal;
		Free( decals );
		return nextDecal;
	}
	
	decalInfo = decals->material->GetDecalInfo();
	minTime = time - ( decalInfo.stayTime + decalInfo.fadeTime );

	newNumIndexes = 0;
	for ( i = 0; i < decals->tri.numIndexes; i += 3 ) {
		if ( decals->indexStartTime[i] > minTime ) {
			// keep this triangle
			if ( newNumIndexes != i ) {
				for ( j = 0; j < 3; j++ ) {
					decals->tri.indexes[newNumIndexes+j] = decals->tri.indexes[i+j];
					decals->indexStartTime[newNumIndexes+j] = decals->indexStartTime[i+j];
				}
			}
			newNumIndexes += 3;
		}
	}

	// free the decals if all trianges faded away
	if ( newNumIndexes == 0 ) {
		nextDecal = decals->nextDecal;
		Free( decals );
		return nextDecal;
	}

	decals->tri.numIndexes = newNumIndexes;

	memset( inUse, 0, sizeof( inUse ) );
	for ( i = 0; i < decals->tri.numIndexes; i++ ) {
		inUse[decals->tri.indexes[i]] = 1;
	}

	newNumVerts = 0;
	for ( i = 0; i < decals->tri.numVerts; i++ ) {
		if ( !inUse[i] ) {
			continue;
		}
		decals->tri.verts[newNumVerts] = decals->tri.verts[i];
		decals->vertDepthFade[newNumVerts] = decals->vertDepthFade[i];
		inUse[i] = newNumVerts;
		newNumVerts++;
	}
	decals->tri.numVerts = newNumVerts;

	for ( i = 0; i < decals->tri.numIndexes; i++ ) {
		decals->tri.indexes[i] = inUse[decals->tri.indexes[i]];
	}

	return decals;
}

/*
=====================
idRenderModelDecal::AddDecalDrawSurf
=====================
*/
void idRenderModelDecal::AddDecalDrawSurf( viewEntity_t *space ) {
	int i, j, maxTime;
	float f;
	decalInfo_t	decalInfo;
	
	if ( tri.numIndexes == 0 ) {
		return;
	}

	// fade down all the verts with time
	decalInfo = material->GetDecalInfo();
	maxTime = decalInfo.stayTime + decalInfo.fadeTime;

	// set vertex colors and remove faded triangles
	for ( i = 0 ; i < tri.numIndexes ; i += 3 ) {
		int	deltaTime = tr.viewDef->renderView.time - indexStartTime[i];

		if ( deltaTime > maxTime ) {
			continue;
		}

		if ( deltaTime <= decalInfo.stayTime ) {
			continue;
		}

		deltaTime -= decalInfo.stayTime;
		f = (float)deltaTime / decalInfo.fadeTime;

		for ( j = 0; j < 3; j++ ) {
			int	ind = tri.indexes[i+j];

			for ( int k = 0; k < 4; k++ ) {
				float fcolor = decalInfo.start[k] + ( decalInfo.end[k] - decalInfo.start[k] ) * f;
				int icolor = idMath::FtoiFast( fcolor * vertDepthFade[ind] * 255.0f );
				if ( icolor < 0 ) {
					icolor = 0;
				} else if ( icolor > 255 ) {
					icolor = 255;
				}
				tri.verts[ind].color[k] = icolor;
			}
		}
	}

	// copy the tri and indexes to temp heap memory,
	// because if we are running multi-threaded, we wouldn't
	// be able to reorganize the index list
	srfTriangles_t *newTri = (srfTriangles_t *)R_FrameAlloc( sizeof( *newTri ) );
	*newTri = tri;

	// copy the current vertexes to temp vertex cache
	newTri->ambientCache = vertexCache.AllocFrameTemp( tri.verts, tri.numVerts * sizeof( idDrawVert ) );

	// create the drawsurf
	R_AddDrawSurf( newTri, space, &space->entityDef->parms, material, space->scissorRect );
}

/*
====================
idRenderModelDecal::ReadFromDemoFile
====================
*/
void idRenderModelDecal::ReadFromDemoFile( idDemoFile *f ) {
	// FIXME: implement
}

/*
====================
idRenderModelDecal::WriteToDemoFile
====================
*/
void idRenderModelDecal::WriteToDemoFile( idDemoFile *f ) const {
	// FIXME: implement
}

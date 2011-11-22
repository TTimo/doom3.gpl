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

#include "../../../idlib/precompiled.h"
#pragma hdrstop

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "../../../sys/win32/win_local.h"
#endif

#include "../../../renderer/tr_local.h"

/*

  render a normalmap tga file from an ase model for bump mapping

  To make ray-tracing into the high poly mesh efficient, we preconstruct
  a 3D hash table of the triangles that need to be tested for a given source
  point.

  This task is easier than a general ray tracing optimization, because we
  known that all of the triangles are going to be "near" the source point.

  TraceFraction determines the maximum distance in any direction that
  a trace will go.  It is expressed as a fraction of the largest axis of
  the bounding box, so it doesn't matter what units are used for modeling.


*/

#define MAX_QPATH		256

#define	DEFAULT_TRACE_FRACTION	0.05

#define	INITIAL_TRI_TO_LINK_EXPANSION	16	// can grow as needed
#define	HASH_AXIS_BINS	100

typedef struct {
	int		faceNum;
	int		nextLink;
} triLink_t;

typedef struct {
	int		triLink;
	int		rayNumber;		// don't need to test again if still on same ray
} binLink_t;

#define	MAX_LINKS_PER_BLOCK		0x100000
#define	MAX_LINK_BLOCKS			0x100
typedef struct {
	idBounds	bounds;
	float		binSize[3];
	int			numLinkBlocks;
	triLink_t	*linkBlocks[MAX_LINK_BLOCKS];
	binLink_t	binLinks[HASH_AXIS_BINS][HASH_AXIS_BINS][HASH_AXIS_BINS];
} triHash_t;

typedef struct {
	char	outputName[MAX_QPATH];
	char	highName[MAX_QPATH];
	byte	*localPic;
	byte	*globalPic;
	byte	*colorPic;
	float	*edgeDistances;		// starts out -1 for untraced, for each texel, 0 = true interior, >0 = off-edge rasterization
	int		width, height;
	int		antiAlias;
	int		outline;
	bool	saveGlobalMap;
	bool	saveColorMap;
	float	traceFrac;
	float	traceDist;
	srfTriangles_t	*mesh;			// high poly mesh
	idRenderModel	*highModel;
	triHash_t	*hash;	
} renderBump_t;

static float traceFraction;
static int rayNumber;		// for avoiding retests of bins and faces

static int oldWidth, oldHeight;

/*
===============
SaveWindow
===============
*/
static void SaveWindow( void ) {
	oldWidth = glConfig.vidWidth;
	oldHeight = glConfig.vidHeight;
}

/*
===============
ResizeWindow
===============
*/
static void ResizeWindow( int width, int height ) {
#ifdef WIN32
	int	winWidth, winHeight;
	if ( glConfig.isFullscreen ) {
		winWidth = width;
		winHeight = height;
	} else {
		RECT	r;

		// adjust width and height for window border
		r.bottom = height;
		r.left = 0;
		r.top = 0;
		r.right = width;

		AdjustWindowRect (&r, WINDOW_STYLE|WS_SYSMENU, FALSE);
		winHeight = r.bottom - r.top;
		winWidth = r.right - r.left;

	}
	SetWindowPos( win32.hWnd, HWND_TOP, 0, 0, winWidth, winHeight, SWP_SHOWWINDOW );

	qwglMakeCurrent( win32.hDC, win32.hGLRC );
#endif
}

/*
===============
RestoreWindow
===============
*/
static void RestoreWindow( void ) {
#ifdef WIN32
	int	winWidth, winHeight;
	if ( glConfig.isFullscreen ) {
		winWidth = oldWidth;
		winHeight = oldHeight;
	} else {
		RECT	r;

		// adjust width and height for window border
		r.bottom = oldHeight;
		r.left = 0;
		r.top = 0;
		r.right = oldWidth;

		AdjustWindowRect (&r, WINDOW_STYLE|WS_SYSMENU, FALSE);
		winHeight = r.bottom - r.top;
		winWidth = r.right - r.left;
	}
	SetWindowPos( win32.hWnd, HWND_TOP, 0, 0, winWidth, winHeight, SWP_SHOWWINDOW );
#endif
}

/*
================
OutlineNormalMap

Puts a single pixel border around all non-empty pixels
Does NOT copy the alpha channel, so it can be used as
an alpha test map.
================
*/
static void OutlineNormalMap( byte *data, int width, int height, int emptyR, int emptyG, int emptyB ) {
	byte	*orig;
	int		i, j, k, l;
	idVec3	normal;
	byte	*out;

	orig = (byte *)Mem_Alloc( width * height * 4 );
	memcpy( orig, data, width * height * 4 );

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			out = data + ( j * width + i ) * 4;
			if ( out[0] != emptyR || out[1] != emptyG || out[2] != emptyB ) {
				continue;
			}

			normal = vec3_origin;
			for ( k = -1 ; k < 2 ; k++ ) {
				for ( l = -1 ; l < 2 ; l++ ) {
					byte	*in;

					in = orig + ( ((j+l)&(height-1))*width + ((i+k)&(width-1)) ) * 4;

					if ( in[0] == emptyR && in[1] == emptyG && in[2] == emptyB ) {
						continue;
					}

					normal[0] += ( in[0] - 128 );
					normal[1] += ( in[1] - 128 );
					normal[2] += ( in[2] - 128 );
				}
			}

			if ( normal.Normalize() < 0.5 ) {
				continue;	// no valid samples
			}

			out[0] = 128 + 127 * normal[0];
			out[1] = 128 + 127 * normal[1];
			out[2] = 128 + 127 * normal[2];
		}
	}

	Mem_Free( orig );
}

/*
================
OutlineColorMap

Puts a single pixel border around all non-empty pixels
Does NOT copy the alpha channel, so it can be used as
an alpha test map.
================
*/
static void OutlineColorMap( byte *data, int width, int height, int emptyR, int emptyG, int emptyB ) {
	byte	*orig;
	int		i, j, k, l;
	idVec3	normal;
	byte	*out;

	orig = (byte *)Mem_Alloc( width * height * 4 );
	memcpy( orig, data, width * height * 4 );

	for ( i = 0 ; i < width ; i++ ) {
		for ( j = 0 ; j < height ; j++ ) {
			out = data + ( j * width + i ) * 4;
			if ( out[0] != emptyR || out[1] != emptyG || out[2] != emptyB ) {
				continue;
			}

			normal = vec3_origin;
			int	count = 0;
			for ( k = -1 ; k < 2 ; k++ ) {
				for ( l = -1 ; l < 2 ; l++ ) {
					byte	*in;

					in = orig + ( ((j+l)&(height-1))*width + ((i+k)&(width-1)) ) * 4;

					if ( in[0] == emptyR && in[1] == emptyG && in[2] == emptyB ) {
						continue;
					}

					normal[0] += in[0];
					normal[1] += in[1];
					normal[2] += in[2];
					count++;
				}
			}

			if ( !count ) {
				continue;
			}
			normal *= (1.0 / count );

			out[0] = normal[0];
			out[1] = normal[1];
			out[2] = normal[2];
		}
	}

	Mem_Free( orig );
}



/*
================
FreeTriHash
================
*/
static void FreeTriHash( triHash_t *hash ) {
	for ( int i = 0 ; i < hash->numLinkBlocks ; i++ ) {
		Mem_Free( hash->linkBlocks[i] );
	}
	Mem_Free( hash );
}

/*
================
CreateTriHash
================
*/
static triHash_t *CreateTriHash( const srfTriangles_t *highMesh ) {
	triHash_t	*hash;
	int			i, j, k, l;
	idBounds	bounds, triBounds;
	int			iBounds[2][3];
	int			maxLinks, numLinks;

	hash = (triHash_t *)Mem_Alloc( sizeof( *hash ) );
	memset( hash, 0, sizeof( *hash ) );

	// find the bounding volume for the mesh
	bounds.Clear();
	for ( i = 0 ; i < highMesh->numVerts ; i++ ) {
		bounds.AddPoint( highMesh->verts[i].xyz );
	}

	hash->bounds = bounds;

	// divide each axis as needed
	for ( i = 0 ; i < 3 ; i++ ) {
		hash->binSize[i] = ( bounds[1][i] - bounds[0][i] ) / HASH_AXIS_BINS;
		if ( hash->binSize[i] <= 0 ) {
			common->FatalError( "CreateTriHash: bad bounds: (%f %f %f) to (%f %f %f)",
						bounds[0][0],bounds[0][1],bounds[0][2],
							bounds[1][0],bounds[1][1],bounds[1][2] );
		}
	}

	// a -1 link number terminated the link chain
	memset( hash->binLinks, -1, sizeof( hash->binLinks ) );

	numLinks = 0;

	hash->linkBlocks[hash->numLinkBlocks] = (triLink_t *)Mem_Alloc( MAX_LINKS_PER_BLOCK * sizeof( triLink_t ) );
	hash->numLinkBlocks++;
	maxLinks = hash->numLinkBlocks * MAX_LINKS_PER_BLOCK;

	// for each triangle, place a triLink in each bin that might reference it
	for ( i = 0 ; i < highMesh->numIndexes ; i+=3 ) {
		// determine which hash bins the triangle will need to be in
		triBounds.Clear();
		for ( j = 0 ; j < 3 ; j++ ) {
			triBounds.AddPoint( highMesh->verts[ highMesh->indexes[i+j] ].xyz );
		}
		for ( j = 0 ; j < 3 ; j++ ) {
			iBounds[0][j] = ( triBounds[0][j] - hash->bounds[0][j] ) / hash->binSize[j];
			iBounds[0][j] -= 0.001;	// epsilon
			if ( iBounds[0][j] < 0 ) {
				iBounds[0][j] = 0;
			} else if ( iBounds[0][j] >= HASH_AXIS_BINS ) {
				iBounds[0][j] = HASH_AXIS_BINS-1;
			}

			iBounds[1][j] = ( triBounds[1][j] - hash->bounds[0][j] ) / hash->binSize[j];
			iBounds[0][j] += 0.001;	// epsilon
			if ( iBounds[1][j] < 0 ) {
				iBounds[1][j] = 0;
			} else if ( iBounds[1][j] >= HASH_AXIS_BINS ) {
				iBounds[1][j] = HASH_AXIS_BINS-1;
			}
		}

		// add the links
		for ( j = iBounds[0][0] ; j <= iBounds[1][0] ; j++ ) {
			for ( k = iBounds[0][1] ; k <= iBounds[1][1] ; k++ ) {
				for ( l = iBounds[0][2] ; l <= iBounds[1][2] ; l++ ) {
					if ( numLinks == maxLinks ) {
						hash->linkBlocks[hash->numLinkBlocks] = (triLink_t *)Mem_Alloc( MAX_LINKS_PER_BLOCK * sizeof( triLink_t ) );
						hash->numLinkBlocks++;
						maxLinks = hash->numLinkBlocks * MAX_LINKS_PER_BLOCK;
					}

					triLink_t	*link = &hash->linkBlocks[ numLinks / MAX_LINKS_PER_BLOCK ][ numLinks % MAX_LINKS_PER_BLOCK ];
					link->faceNum = i / 3;
					link->nextLink = hash->binLinks[j][k][l].triLink;
					hash->binLinks[j][k][l].triLink = numLinks;
					numLinks++;
				}
			}
		}
	}

	common->Printf( "%i triangles made %i links\n", highMesh->numIndexes / 3, numLinks );

	return hash;
}


/*
=================
TraceToMeshFace

Returns the distance from the point to the intersection, or DIST_NO_INTERSECTION
=================
*/
#define	DIST_NO_INTERSECTION	-999999999.0f
static float TraceToMeshFace( const srfTriangles_t *highMesh, int faceNum, 
							 float minDist, float maxDist,
							const idVec3 &point, const idVec3 &normal, idVec3 &sampledNormal,
							byte sampledColor[4] ) {
	int		j;
	float	dist;
	const idVec3	*v[3];
	const idPlane	*plane;
	idVec3	edge;
	float	d;
	idVec3	dir[3];
	float	baseArea;
	float	bary[3];
	idVec3	testVert;

	v[0] = &highMesh->verts[ highMesh->indexes[ faceNum * 3 + 0 ] ].xyz;
	v[1] = &highMesh->verts[ highMesh->indexes[ faceNum * 3 + 1 ] ].xyz;
	v[2] = &highMesh->verts[ highMesh->indexes[ faceNum * 3 + 2 ] ].xyz;

	plane = highMesh->facePlanes + faceNum;

	// only test against planes facing the same direction as our normal
	d = plane->Normal() * normal;
	if ( d <= 0.0001f ) {
		return DIST_NO_INTERSECTION;
	}

	// find the point of impact on the plane
	dist = plane->Distance( point );
	dist /= -d;

	testVert = point + dist * normal;

	// if this would be beyond our requested trace distance,
	// don't even check it
	if ( dist > maxDist ) {
		return DIST_NO_INTERSECTION;
	}

	if ( dist < minDist ) {
		return DIST_NO_INTERSECTION;
	}

	// if normal is inside all edge planes, this face is hit
	VectorSubtract( *v[0], point, dir[0] );
	VectorSubtract( *v[1], point, dir[1] );
	edge = dir[0].Cross( dir[1] );
	d = DotProduct( normal, edge );
	if ( d > 0.0f ) {
		return DIST_NO_INTERSECTION;
	}
	VectorSubtract( *v[2], point, dir[2] );
	edge = dir[1].Cross( dir[2] );
	d = DotProduct( normal, edge );
	if ( d > 0.0f ) {
		return DIST_NO_INTERSECTION;
	}
	edge = dir[2].Cross( dir[0] );
	d = DotProduct( normal, edge );
	if ( d > 0.0f ) {
		return DIST_NO_INTERSECTION;
	}

	// calculate barycentric coordinates of the impact point
	// on the high poly triangle
	bary[0] = idWinding::TriangleArea( testVert, *v[1], *v[2] );
	bary[1] = idWinding::TriangleArea( *v[0], testVert, *v[2] );
	bary[2] = idWinding::TriangleArea( *v[0], *v[1], testVert );

	baseArea = idWinding::TriangleArea( *v[0], *v[1], *v[2] );
	bary[0] /= baseArea;
	bary[1] /= baseArea;
	bary[2] /= baseArea;

	if ( bary[0] + bary[1] + bary[2] > 1.1 ) {
		bary[0] = bary[0];
		return DIST_NO_INTERSECTION;
	}

	// triangularly interpolate the normals to the sample point
	sampledNormal = vec3_origin;
	for ( j = 0 ; j < 3 ; j++ ) {
		sampledNormal += bary[j] * highMesh->verts[ highMesh->indexes[ faceNum * 3 + j ] ].normal;
	}
	sampledNormal.Normalize();

	sampledColor[0] = sampledColor[1] = sampledColor[2] = sampledColor[3] = 0;
	for ( int i = 0 ; i < 4 ; i++ ) {
		float	color = 0.0f;
		for ( j = 0 ; j < 3 ; j++ ) {
			color += bary[j] * highMesh->verts[ highMesh->indexes[ faceNum * 3 + j ] ].color[i];
		}
		sampledColor[i] = color;
	}
	return dist;
}


/*
================
SampleHighMesh

Find the best surface normal in the high poly mesh 
for a ray coming from the surface of the low poly mesh

Returns false if the trace doesn't hit anything
================
*/
static bool SampleHighMesh( const renderBump_t *rb,
							const idVec3 &point, const idVec3 &direction, idVec3 &sampledNormal, 
							byte sampledColor[4] ) {
	idVec3	p;
	binLink_t	*bl;
	int			linkNum;
	int		faceNum;
	float	dist, bestDist;
	int		block[3];
	float	maxDist;
	int		c_hits;
	int		i;
	idVec3	normal;

	// we allow non-normalized directions on input
	normal = direction;
	normal.Normalize();

	// increment our uniqueness counter (FIXME: make thread safe?)
	rayNumber++;

	// the max distance will be the traceFrac times the longest axis of the high poly model
	bestDist = -rb->traceDist;
	maxDist = rb->traceDist;

	sampledNormal = vec3_origin;

	c_hits = 0;

	// this is a pretty damn lazy way to walk through a 3D grid, and has a (very slight)
	// chance of missing a triangle in a corner crossing case
#define	RAY_STEPS	100
	for ( i = 0 ; i < RAY_STEPS ; i++ ) {
		p = point - rb->hash->bounds[0] + normal * ( -1.0 + 2.0 * i / RAY_STEPS ) * rb->traceDist;

		block[0] = floor( p[0] / rb->hash->binSize[0] );
		block[1] = floor( p[1] / rb->hash->binSize[1] );
		block[2] = floor( p[2] / rb->hash->binSize[2] );

		if ( block[0] < 0 || block[0] >= HASH_AXIS_BINS ) {
			continue;
		}
		if ( block[1] < 0 || block[1] >= HASH_AXIS_BINS ) {
			continue;
		}
		if ( block[2] < 0 || block[2] >= HASH_AXIS_BINS ) {
			continue;
		}

		// FIXME: casting away const
		bl = (binLink_t *)&rb->hash->binLinks[block[0]][block[1]][block[2]];
		if ( bl->rayNumber == rayNumber ) {
			continue;		// already tested this block
		}
		bl->rayNumber = rayNumber;
		linkNum = bl->triLink;
		triLink_t	*link;
		for ( ; linkNum != -1 ; linkNum = link->nextLink ) {
			link = &rb->hash->linkBlocks[ linkNum / MAX_LINKS_PER_BLOCK ][ linkNum % MAX_LINKS_PER_BLOCK ];

			faceNum = link->faceNum;
			dist = TraceToMeshFace( rb->mesh, faceNum, 
								 bestDist, maxDist, point, normal, sampledNormal, sampledColor );
			if ( dist == DIST_NO_INTERSECTION ) {
				continue;
			}

			c_hits++;
			// continue looking for a better match
			bestDist = dist;
		}
	}

	return (bool)( bestDist > -rb->traceDist );
}

/*
=============
TriTextureArea

This may be negatove
=============
*/
static float TriTextureArea( const float a[2], const float b[2], const float c[2] ) {
	idVec3	d1, d2;
	idVec3	cross;
	float	area;

	d1[0] = b[0] - a[0];
	d1[1] = b[1] - a[1];
	d1[2] = 0;

	d2[0] = c[0] - a[0];
	d2[1] = c[1] - a[1];
	d2[2] = 0;
	
	cross = d1.Cross( d2 );
	area = 0.5 * cross.Length();

	if ( cross[2] < 0 ) {
		return -area;
	} else {
		return area;
	}
}

/*
================
RasterizeTriangle

It is ok for the texcoords to wrap around, the rasterization
will deal with it properly.
================
*/
static void RasterizeTriangle( const srfTriangles_t *lowMesh, const idVec3 *lowMeshNormals, int lowFaceNum,
							 renderBump_t *rb ) {
	int		i, j, k;
	float	bounds[2][2];
	float	ibounds[2][2];
	float	verts[3][2];
	float	testVert[2];
	float	bary[3];
	byte	*localDest, *globalDest, *colorDest;
	float	edge[3][3];
	idVec3	sampledNormal;
	byte	sampledColor[4];
	idVec3	point, normal, traceNormal, tangents[2];
	float	baseArea, totalArea;
	int		r, g, b;
	idVec3	localNormal;

	// this is a brain-dead rasterizer, but compared to the ray trace,
	// nothing we do here is going to matter performance-wise

	// adjust for resolution and texel centers
	verts[0][0] = lowMesh->verts[ lowMesh->indexes[lowFaceNum*3+0] ].st[0] * rb->width - 0.5;
	verts[1][0] = lowMesh->verts[ lowMesh->indexes[lowFaceNum*3+1] ].st[0] * rb->width - 0.5;
	verts[2][0] = lowMesh->verts[ lowMesh->indexes[lowFaceNum*3+2] ].st[0] * rb->width - 0.5;
	verts[0][1] = lowMesh->verts[ lowMesh->indexes[lowFaceNum*3+0] ].st[1] * rb->height - 0.5;
	verts[1][1] = lowMesh->verts[ lowMesh->indexes[lowFaceNum*3+1] ].st[1] * rb->height - 0.5;
	verts[2][1] = lowMesh->verts[ lowMesh->indexes[lowFaceNum*3+2] ].st[1] * rb->height - 0.5;

	// find the texcoord bounding box
	bounds[0][0] = 99999;
	bounds[0][1] = 99999;
	bounds[1][0] = -99999;
	bounds[1][1] = -99999;
	for ( i = 0 ; i < 2 ; i++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( verts[j][i] < bounds[0][i] ) {
				bounds[0][i] = verts[j][i];
			}
			if ( verts[j][i] > bounds[1][i] ) {
				bounds[1][i] = verts[j][i];
			}
		}
	}

	// we intentionally rasterize somewhat outside the triangles, so
	// the bilerp support texels (which may be anti-aliased down)
	// are not just duplications of what is on the interior
	const float	edgeOverlap = 4.0;

	ibounds[0][0] = floor( bounds[0][0] - edgeOverlap );
	ibounds[1][0] = ceil( bounds[1][0] + edgeOverlap );
	ibounds[0][1] = floor( bounds[0][1] - edgeOverlap );
	ibounds[1][1] = ceil( bounds[1][1] + edgeOverlap );

	// calculate edge vectors
	for ( i = 0 ; i < 3 ; i++ ) {
		float	*v1, *v2;
		
		v1 = verts[i];
		v2 = verts[(i+1)%3];

		edge[i][0] = v2[1] - v1[1];
		edge[i][1] = v1[0] - v2[0];
		float len = sqrt( edge[i][0] * edge[i][0] + edge[i][1] * edge[i][1] );
		edge[i][0] /= len;
		edge[i][1] /= len;
		edge[i][2] = -( v1[0] * edge[i][0] + v1[1] * edge[i][1] );
	}

	// itterate over the bounding box, testing against edge vectors
	for ( i = ibounds[0][1] ; i < ibounds[1][1] ; i++ ) {
		for ( j = ibounds[0][0] ; j < ibounds[1][0] ; j++ ) {
			float	dists[3];

			k =  ( ( i & (rb->height-1) ) * rb->width + ( j & (rb->width-1) ) ) * 4;
			colorDest = &rb->colorPic[k];
			localDest = &rb->localPic[k];
			globalDest = &rb->globalPic[k];

#define SKIP_MIRRORS

			float *edgeDistance = &rb->edgeDistances[k/4];
#ifdef SKIP_MIRRORS
			// if this texel has already been filled by a true interior pixel, don't overwrite it
			if ( *edgeDistance == 0 ) {
				continue;
			}
#endif

			// check against the three edges to see if the pixel is inside the triangle
			for ( k = 0 ; k < 3 ; k++ ) {
				float	v;

				v = i * edge[k][1] + j * edge[k][0] + edge[k][2];
				dists[k] = v;
			}

			// the edge polarities might be either way
			if ( ! ( ( dists[0] >= -edgeOverlap && dists[1] >= -edgeOverlap && dists[2] >= -edgeOverlap )
				|| ( dists[0] <= edgeOverlap && dists[1] <= edgeOverlap && dists[2] <= edgeOverlap ) ) ) {
				continue;
			}

			bool	edgeTexel;

			if ( ( dists[0] >= 0 && dists[1] >= 0 && dists[2] >= 0 )
				|| ( dists[0] <= 0 && dists[1] <= 0 && dists[2] <= 0 ) ) {
				edgeTexel = false;
			} else {
				edgeTexel = true;
#ifdef SKIP_MIRRORS
				// if this texel has already been filled by another edge pixel, don't overwrite it
				if ( *edgeDistance == 1 ) {
					continue;
				}
#endif
			}

			// calculate the barycentric coordinates in the triangle for this sample
			testVert[0] = j;
			testVert[1] = i;

			baseArea = TriTextureArea( verts[0], verts[1], verts[2] );
			bary[0] = TriTextureArea( testVert, verts[1], verts[2] ) / baseArea;
			bary[1] = TriTextureArea( verts[0], testVert, verts[2] ) / baseArea;
			bary[2] = TriTextureArea( verts[0], verts[1], testVert ) / baseArea;

			totalArea = bary[0] + bary[1] + bary[2];
			if ( totalArea < 0.99 || totalArea > 1.01 ) {
				continue;	// should never happen
			}

			// calculate the interpolated xyz, normal, and tangents of this sample
			point = vec3_origin;
			traceNormal = vec3_origin;
			normal = vec3_origin;
			tangents[0] = vec3_origin;
			tangents[1] = vec3_origin;
			for ( k = 0 ; k < 3 ; k++ ) {
				int		index;

				index = lowMesh->indexes[lowFaceNum*3+k];
				point += bary[k] * lowMesh->verts[ index ].xyz;

				// traceNormal will differ from normal if the surface uses unsmoothedTangents
				traceNormal += bary[k] * lowMeshNormals[ index ];

				normal += bary[k] * lowMesh->verts[ index ].normal;
				tangents[0] += bary[k] * lowMesh->verts[ index ].tangents[0];
				tangents[1] += bary[k] * lowMesh->verts[ index ].tangents[1];
			}

#if 0
			// this doesn't seem to make much difference
			// an argument can be made that these should not be normalized, because the interpolation
			// of the light position at rasterization time will be linear, not spherical
			normal.Normalize();
			tangents[0].Normalize();
			tangents[1].Normalize();
#endif

			// find the best triangle in the high poly model for this
			// sampledNormal will  normalized
			if ( !SampleHighMesh( rb, point, traceNormal, sampledNormal, sampledColor ) ) {
#if 0
				// put bright red where all traces missed for debugging.
				// for production use, it is better to leave it blank so
				// the outlining fills it in
				globalDest[0] = 255;
				globalDest[1] = 0;
				globalDest[2] = 0;
				globalDest[3] = 255;

				localDest[0] = 255;
				localDest[1] = 0;
				localDest[2] = 0;
				localDest[3] = 255;
#endif
				continue;
			}


			// mark whether this is an interior or edge texel
			*edgeDistance = ( edgeTexel ? 1.0 : 0 );

			// fill the object space normal map spot
			r = 128 + 127 * sampledNormal[0];
			g = 128 + 127 * sampledNormal[1];
			b = 128 + 127 * sampledNormal[2];

			globalDest[0] = r;
			globalDest[1] = g;
			globalDest[2] = b;
			globalDest[3] = 255;

			// transform to local tangent space
			idMat3	mat;
			mat[0] = tangents[0];
			mat[1] = tangents[1];
			mat[2] = normal;
			mat.InverseSelf();
			localNormal = mat * sampledNormal;

			localNormal.Normalize();


			r = 128 + 127 * localNormal[0];
			g = 128 + 127 * localNormal[1];
			b = 128 + 127 * localNormal[2];

			localDest[0] = r;
			localDest[1] = g;
			localDest[2] = b;
			localDest[3] = 255;

			colorDest[0] = sampledColor[0];
			colorDest[1] = sampledColor[1];
			colorDest[2] = sampledColor[2];
			colorDest[3] = sampledColor[3];
		}
	}
}

/*
================
CombineModelSurfaces

Frees the model and returns a new model with all triangles combined
into one surface
================
*/
static idRenderModel *CombineModelSurfaces( idRenderModel *model ) {
	int		totalVerts;
	int		totalIndexes;
	int		numIndexes;
	int		numVerts;
	int		i, j;

	totalVerts = 0;
	totalIndexes = 0;

	for ( i = 0 ; i < model->NumSurfaces() ; i++ ) {
		const modelSurface_t	*surf = model->Surface(i);

		totalVerts += surf->geometry->numVerts;
		totalIndexes += surf->geometry->numIndexes;
	}

	srfTriangles_t *newTri = R_AllocStaticTriSurf();
	R_AllocStaticTriSurfVerts( newTri, totalVerts );
	R_AllocStaticTriSurfIndexes( newTri, totalIndexes );

	newTri->numVerts = totalVerts;
	newTri->numIndexes = totalIndexes;

	newTri->bounds.Clear();

	idDrawVert *verts = newTri->verts;
	glIndex_t *indexes = newTri->indexes;
	numIndexes = 0;
	numVerts = 0;
	for ( i = 0 ; i < model->NumSurfaces() ; i++ ) {
		const modelSurface_t *surf = model->Surface(i);
		const srfTriangles_t *tri = surf->geometry;

		memcpy( verts + numVerts, tri->verts, tri->numVerts * sizeof( tri->verts[0] ) );
		for ( j = 0 ; j < tri->numIndexes ; j++ ) {
			indexes[numIndexes+j] = numVerts + tri->indexes[j];
		}
		newTri->bounds.AddBounds( tri->bounds );
		numIndexes += tri->numIndexes;
		numVerts += tri->numVerts;
	}

	modelSurface_t surf;

	surf.id = 0;
	surf.geometry = newTri;
	surf.shader = tr.defaultMaterial;

	idRenderModel *newModel = renderModelManager->AllocModel();
	newModel->AddSurface( surf );

	renderModelManager->FreeModel( model );

	return newModel;
}

/*
==============
RenderBumpTriangles

==============
*/
static void RenderBumpTriangles( srfTriangles_t *lowMesh, renderBump_t *rb ) {
	int		i, j;

	RB_SetGL2D();

	qglDisable( GL_CULL_FACE );

	qglColor3f( 1, 1, 1 );

	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity();
	qglOrtho( 0, 1, 1, 0, -1, 1 );
	qglDisable( GL_BLEND );
	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();

	qglDisable( GL_DEPTH_TEST );

	qglClearColor(1,0,0,1);
	qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	qglColor3f( 1, 1, 1 );

	// create smoothed normals for the surface, which might be
	// different than the normals at the vertexes if the
	// surface uses unsmoothedNormals, which only takes the
	// normal from a single triangle.  We need properly smoothed
	// normals to make sure that the traces always go off normal
	// to the true surface.
	idVec3	*lowMeshNormals = (idVec3 *)Mem_ClearedAlloc( lowMesh->numVerts * sizeof( *lowMeshNormals ) );
	R_DeriveFacePlanes( lowMesh );
	R_CreateSilIndexes( lowMesh );	// recreate, merging the mirrored verts back together
	const idPlane *planes = lowMesh->facePlanes;
	for ( i = 0 ; i < lowMesh->numIndexes ; i += 3, planes++ ) {
		for ( j = 0 ; j < 3 ; j++ ) {
			int		index;

			index = lowMesh->silIndexes[i+j];
			lowMeshNormals[index] += (*planes).Normal();
		}
	}
	// normalize and replicate from silIndexes to all indexes
	for ( i = 0 ; i < lowMesh->numIndexes ; i++ ) {
		lowMeshNormals[lowMesh->indexes[i]] = lowMeshNormals[lowMesh->silIndexes[i]];
		lowMeshNormals[lowMesh->indexes[i]].Normalize();
	}


	// rasterize each low poly face
	for ( j = 0 ; j < lowMesh->numIndexes ; j+=3 ) {
		// pump the event loop so the window can be dragged around
		Sys_GenerateEvents();

		RasterizeTriangle( lowMesh, lowMeshNormals, j/3, rb );

		qglClearColor(1,0,0,1);
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		qglRasterPos2f( 0, 1 );
		qglPixelZoom( glConfig.vidWidth / (float)rb->width, glConfig.vidHeight / (float)rb->height );
		qglDrawPixels( rb->width, rb->height, GL_RGBA, GL_UNSIGNED_BYTE, rb->localPic );
		qglPixelZoom( 1, 1 );
		qglFlush();
		GLimp_SwapBuffers();
	}

	Mem_Free( lowMeshNormals );
}


/*
==============
WriteRenderBump

==============
*/
static void WriteRenderBump( renderBump_t *rb, int outLinePixels ) {
	int		width, height;
	int		i;
	idStr	filename;

	renderModelManager->FreeModel( rb->highModel );

	FreeTriHash( rb->hash );

	width = rb->width;
	height = rb->height;

#if 0
	// save the non-outlined version
	filename = source;
	filename.setFileExtension();
	filename.append( "_nooutline.tga" );
	common->Printf( "writing %s\n", filename.c_str() );
	WriteTGA( filename, globalPic, width, height );
#endif

	// outline the image several times to help bilinear filtering across disconnected
	// edges, and mip-mapping
	for ( i = 0 ; i < outLinePixels ; i++ ) {
		OutlineNormalMap( rb->localPic, width, height, 128, 128, 128 );
		OutlineNormalMap( rb->globalPic, width, height, 128, 128, 128 );
		OutlineColorMap( rb->colorPic, width, height, 128, 128, 128 );
	}

	// filter down if we are anti-aliasing
	for ( i = 0 ; i < rb->antiAlias ; i++ ) {
		byte	*old;

		old = rb->localPic;
		rb->localPic = R_MipMap( rb->localPic, width, height, false );
		Mem_Free( old );

		old = rb->globalPic;
		rb->globalPic = R_MipMap( rb->globalPic, width, height, false );
		Mem_Free( old );

		old = rb->colorPic;
		rb->colorPic = R_MipMap( rb->colorPic, width, height, false );
		Mem_Free( old );

		width >>= 1;
		height >>= 1;
	}

	// write out the local map
	filename = rb->outputName;
	filename.SetFileExtension( ".tga" );
	common->Printf( "writing %s (%i,%i)\n", filename.c_str(), width, height );
	R_WriteTGA( filename, rb->localPic, width, height );

	if ( rb->saveGlobalMap ) {
		filename = rb->outputName;
		filename.StripFileExtension();
		filename.Append( "_global.tga" );
		common->Printf( "writing %s (%i,%i)\n", filename.c_str(), width, height );
		R_WriteTGA( filename, rb->globalPic, width, height );
	}

	if ( rb->saveColorMap ) {
		filename = rb->outputName;
		filename.StripFileExtension();
		filename.Append( "_color.tga" );
		common->Printf( "writing %s (%i,%i)\n", filename.c_str(), width, height );
		R_WriteTGA( filename, rb->colorPic, width, height );
	}

	Mem_Free( rb->localPic );
	Mem_Free( rb->globalPic );
	Mem_Free( rb->colorPic );
	Mem_Free( rb->edgeDistances );
}

/*
===============
InitRenderBump
===============
*/
static void InitRenderBump( renderBump_t *rb ) {
	srfTriangles_t	*mesh;
	idBounds	bounds;
	int			i, c;

	// load the ase file
	common->Printf( "loading %s...\n", rb->highName );

	rb->highModel = renderModelManager->AllocModel();
	rb->highModel->PartialInitFromFile( rb->highName );
	if ( !rb->highModel ) {
		common->Error( "failed to load %s", rb->highName );
	}

	// combine the high poly model into a single polyset
	if ( rb->highModel->NumSurfaces() != 1 ) {
		rb->highModel = CombineModelSurfaces( rb->highModel );
	}

	const modelSurface_t *surf = rb->highModel->Surface( 0 );
	mesh = surf->geometry;

	rb->mesh = mesh;

	R_DeriveFacePlanes( mesh );

	// create a face hash table to accelerate the tracing
	rb->hash = CreateTriHash( mesh );

	// bound the entire file
	R_BoundTriSurf( mesh );
	bounds = mesh->bounds;

	// the traceDist will be the traceFrac times the larges bounds axis
	rb->traceDist = 0;
	for ( i = 0 ; i < 3 ; i++ ) {
		float	d;

		d = rb->traceFrac * ( bounds[1][i] - bounds[0][i] );
		if ( d > rb->traceDist ) {
			rb->traceDist = d;
		}
	}
	common->Printf( "trace fraction %4.2f = %6.2f model units\n", rb->traceFrac, rb->traceDist );

	c = rb->width * rb->height * 4;

	// local normal map
	rb->localPic = (byte *)Mem_Alloc( c );

	// global (object space, not surface space) normal map
	rb->globalPic = (byte *)Mem_Alloc( c );

	// color pic for artist reference
	rb->colorPic = (byte *)Mem_Alloc( c );

	// edgeDistance for marking outside-the-triangle traces
	rb->edgeDistances = (float *)Mem_Alloc( c );

	for ( i = 0 ; i < c ; i+=4 ) {
		rb->localPic[i+0] = 128;
		rb->localPic[i+1] = 128;
		rb->localPic[i+2] = 128;
		rb->localPic[i+3] = 0;	// the artists use this for masking traced pixels sometimes

		rb->globalPic[i+0] = 128;
		rb->globalPic[i+1] = 128;
		rb->globalPic[i+2] = 128;
		rb->globalPic[i+3] = 0;

		rb->colorPic[i+0] = 128;
		rb->colorPic[i+1] = 128;
		rb->colorPic[i+2] = 128;
		rb->colorPic[i+3] = 0;

		rb->edgeDistances[i/4] = -1;	// not traced yet
	}

}

/*
==============
RenderBump_f

==============
*/
void RenderBump_f( const idCmdArgs &args ) {
	idRenderModel	*lowPoly;
	idStr	source;
	int		i, j;
	const char	*cmdLine;
	int		numRenderBumps;
	renderBump_t	*renderBumps, *rb;
	renderBump_t	opt;
	int		startTime, endTime;

	// update the screen as we print
	common->SetRefreshOnPrint( true );

	// there should be a single parameter, the filename for a game loadable low-poly model
	if ( args.Argc() != 2 ) {
		common->Error( "Usage: renderbump <lowPolyModel>" );
	}

	common->Printf( "----- Renderbump %s -----\n", args.Argv( 1 ) );

	startTime = Sys_Milliseconds();

	// get the lowPoly model
	source = args.Argv( 1 );
	lowPoly = renderModelManager->CheckModel( source );
	if ( !lowPoly ) {
		common->Error( "Can't load model %s", source.c_str() );
	}

	renderBumps = (renderBump_t *)R_StaticAlloc( lowPoly->NumSurfaces() * sizeof( *renderBumps ) );
	numRenderBumps = 0;
	for ( i = 0 ; i < lowPoly->NumSurfaces() ; i++ ) {
		const modelSurface_t	*ms = lowPoly->Surface( i );

		// default options
		memset( &opt, 0, sizeof( opt ) );
		opt.width = 512;
		opt.height = 512;
		opt.antiAlias = 1;
		opt.outline = 8;
		opt.traceFrac = 0.05f;

		// parse the renderbump parameters for this surface
		cmdLine = ms->shader->GetRenderBump();

		common->Printf( "surface %i, shader %s\nrenderBump = %s ", i, 
			ms->shader->GetName(), cmdLine );

		if ( !ms->geometry ) {
			common->Printf( "(no geometry)\n" );
			continue;
		}

		idCmdArgs localArgs;
		localArgs.TokenizeString( cmdLine, false );

		if ( localArgs.Argc() < 2 ) {
			common->Printf( "(no action)\n" );
			continue;
		}

		common->Printf( "(rendering)\n" );

		for ( j = 0 ; j < localArgs.Argc() - 2; j++ ) {
			const char *s;

			s = localArgs.Argv( j );
			if ( s[0] == '-' ) {
				j++;
				s = localArgs.Argv( j );
				if ( s[0] == '\0' ) {
					continue;
				}
			}

			if ( !idStr::Icmp( s, "size" ) ) {
				if ( j + 2 >= localArgs.Argc() ) {
					j = localArgs.Argc();
					break;
				}
				opt.width = atoi( localArgs.Argv( j + 1 ) );
				opt.height = atoi( localArgs.Argv( j + 2 ) );
				j += 2;
			} else if ( !idStr::Icmp( s, "trace" ) ) {
				opt.traceFrac = atof( localArgs.Argv( j + 1 ) );
				j += 1;
			} else if ( !idStr::Icmp( s, "globalMap" ) ) {
				opt.saveGlobalMap = true;
			} else if ( !idStr::Icmp( s, "colorMap" ) ) {
				opt.saveColorMap = true;
			} else if ( !idStr::Icmp( s, "outline" ) ) {
				opt.outline = atoi( localArgs.Argv( j + 1 ) );
				j += 1;
			} else if ( !idStr::Icmp( s, "aa" ) ) {
				opt.antiAlias = atoi( localArgs.Argv( j + 1 ) );
				j += 1;
			} else {
				common->Printf( "WARNING: Unknown option \"%s\"\n", s );
				break;
			}
		}

		if ( j != ( localArgs.Argc() - 2 ) ) {
			common->Error( "usage: renderBump [-size width height] [-aa <1-2>] [globalMap] [colorMap] [-trace <0.01 - 1.0>] normalMapImageFile highPolyAseFile" );
		}
		idStr::Copynz( opt.outputName, localArgs.Argv( j ), sizeof( opt.outputName ) );
		idStr::Copynz( opt.highName, localArgs.Argv( j + 1 ), sizeof( opt.highName ) );

		// adjust size for anti-aliasing
		opt.width <<= opt.antiAlias;
		opt.height <<= opt.antiAlias;

		// see if we already have a renderbump going for another surface that this should use
		for ( j = 0 ; j < numRenderBumps ; j++ ) {
			rb = &renderBumps[j];

			if ( idStr::Icmp( rb->outputName, opt.outputName ) ) {
				continue;
			}
			// all the other parameters must match, or it is an error
			if ( idStr::Icmp( rb->highName, opt.highName) || rb->width != opt.width || 
				rb->height != opt.height || rb->antiAlias != opt.antiAlias || 
				rb->traceFrac != opt.traceFrac ) {
				common->Error( "mismatched renderbump parameters on image %s", rb->outputName );
				continue;
			}

			// saveGlobalMap will be a sticky option
			rb->saveGlobalMap = rb->saveGlobalMap | opt.saveGlobalMap;
			break;
		}

		// create a new renderbump if needed
		if ( j == numRenderBumps ) {
			numRenderBumps++;
			rb = &renderBumps[j];
			*rb = opt;

			InitRenderBump( rb );
		}

		// render the triangles for this surface
		RenderBumpTriangles( ms->geometry, rb );
	}

	//
	// anti-alias and write out all renderbumps that we have completed
	//
	for ( i = 0 ; i < numRenderBumps ; i++ ) {
		WriteRenderBump( &renderBumps[i], opt.outline << opt.antiAlias );
	}

	R_StaticFree( renderBumps );

	endTime = Sys_Milliseconds();
	common->Printf( "%5.2f seconds for renderBump\n", ( endTime - startTime ) / 1000.0 );
	common->Printf( "---------- RenderBump Completed ----------\n" );

	// stop updating the screen as we print
	common->SetRefreshOnPrint( false );
}



/*
==================================================================================

FLAT

The flat case is trivial, and accomplished with hardware rendering

==================================================================================
*/


/*
==============
RenderBumpFlat_f

==============
*/
void RenderBumpFlat_f( const idCmdArgs &args ) {
	int		width, height;
	idStr	source;
	int		i;
	idBounds	bounds;
	srfTriangles_t	*mesh;
	float	boundsScale;

	// update the screen as we print
	common->SetRefreshOnPrint( true );

	width = height = 256;
	boundsScale = 0;

	// check options
	for ( i = 1 ; i < args.Argc() - 1; i++ ) {
		const char *s;

		s = args.Argv( i );
		if ( s[0] == '-' ) {
			i++;
			s = args.Argv( i );
		}

		if ( !idStr::Icmp( s, "size" ) ) {
			if ( i + 2 >= args.Argc() ) {
				i = args.Argc();
				break;
			}
			width = atoi( args.Argv( i + 1 ) );
			height = atoi( args.Argv( i + 2 ) );
			i += 2;
		} else {
			common->Printf( "WARNING: Unknown option \"%s\"\n", s );
			break;
		}
	}

	if ( i != ( args.Argc() - 1 ) ) {
		common->Error( "usage: renderBumpFlat [-size width height] asefile" );
	}

	common->Printf( "Final image size: %i, %i\n", width, height );

	// load the source in "fastload" mode, because we don't
	// need tangent and shadow information
	source = args.Argv( i );

	idRenderModel *highPolyModel = renderModelManager->AllocModel();

	highPolyModel->PartialInitFromFile( source );

	if ( highPolyModel->IsDefaultModel() ) {
		common->Error( "failed to load %s", source.c_str() );
	}

	// combine the high poly model into a single polyset
	if ( highPolyModel->NumSurfaces() != 1 ) {
		highPolyModel = CombineModelSurfaces( highPolyModel );
	}

	// create normals if not present in file
	const modelSurface_t *surf = highPolyModel->Surface( 0 );
	mesh = surf->geometry;

	// bound the entire file
	R_BoundTriSurf( mesh );
	bounds = mesh->bounds;

	SaveWindow();
	ResizeWindow( width, height );

	// for small images, the viewport may be less than the minimum window
	qglViewport( 0, 0, width, height );

	qglEnable( GL_CULL_FACE );
	qglCullFace( GL_FRONT );
	qglDisable( GL_STENCIL_TEST );	
	qglDisable( GL_SCISSOR_TEST );	
	qglDisable( GL_ALPHA_TEST );	
	qglDisable( GL_BLEND );	
	qglEnable( GL_DEPTH_TEST );
	qglDisable( GL_TEXTURE_2D );
	qglDepthMask( GL_TRUE );
	qglDepthFunc( GL_LEQUAL );

	qglColor3f( 1, 1, 1 );

	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity();
	qglOrtho( bounds[0][0], bounds[1][0], bounds[0][2], 
		bounds[1][2], -( bounds[0][1] - 1 ), -( bounds[1][1] + 1 ) );

	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();

	// flat maps are automatically anti-aliased

	idStr	filename;
	int		j, k, c;
	byte	*buffer;
	int		*sumBuffer, *colorSumBuffer;
	bool	flat;
	int		sample;

	sumBuffer = (int *)Mem_Alloc( width * height * 4 * 4 );
	memset( sumBuffer, 0, width * height * 4 * 4 );
	buffer = (byte *)Mem_Alloc( width * height * 4 );

	colorSumBuffer = (int *)Mem_Alloc( width * height * 4 * 4 );
	memset( sumBuffer, 0, width * height * 4 * 4 );

	flat = false;
//flat = true;

	for ( sample = 0 ; sample < 16 ; sample++ ) {
		float	xOff, yOff;

		xOff = ( ( sample & 3 ) / 4.0 ) * ( bounds[1][0] - bounds[0][0] ) / width;
		yOff = ( ( sample / 4 ) / 4.0 ) * ( bounds[1][2] - bounds[0][2] ) / height;

		for ( int colorPass = 0 ; colorPass < 2 ; colorPass++ ) {
			qglClearColor(0.5,0.5,0.5,0);
			qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

			qglBegin( GL_TRIANGLES );
			for ( i = 0 ; i < highPolyModel->NumSurfaces() ; i++ ) {
				const modelSurface_t *surf = highPolyModel->Surface( i );

				mesh = surf->geometry;

				if ( colorPass ) {
					// just render the surface color for artist visualization
					for ( j = 0 ; j < mesh->numIndexes ; j+=3 ) {
						for ( k = 0 ; k < 3 ; k++ ) {
							int		v;
							float	*a;

							v = mesh->indexes[j+k];
							qglColor3ubv( mesh->verts[v].color );
							a = mesh->verts[v].xyz.ToFloatPtr();
							qglVertex3f( a[0] + xOff, a[2] + yOff, a[1] );
						}
					}
				} else {
					// render as normal map
					// we can either flat shade from the plane,
					// or smooth shade from the vertex normals
					for ( j = 0 ; j < mesh->numIndexes ; j+=3 ) {
						if ( flat ) {
							idPlane		plane;
							idVec3		*a, *b, *c;
							int			v1, v2, v3;

							v1 = mesh->indexes[j+0];
							v2 = mesh->indexes[j+1];
							v3 = mesh->indexes[j+2];

							a = &mesh->verts[ v1 ].xyz;
							b = &mesh->verts[ v2 ].xyz;
							c = &mesh->verts[ v3 ].xyz;

							plane.FromPoints( *a, *b, *c );

							// NULLNORMAL is used by the artists to force an area to reflect no
							// light at all
							if ( surf->shader->GetSurfaceFlags() & SURF_NULLNORMAL ) {
								qglColor3f( 0.5, 0.5, 0.5 );
							} else {
								qglColor3f( 0.5 + 0.5*plane[0], 0.5 - 0.5*plane[2], 0.5 - 0.5*plane[1] );
							}

							qglVertex3f( (*a)[0] + xOff, (*a)[2] + yOff, (*a)[1] );
							qglVertex3f( (*b)[0] + xOff, (*b)[2] + yOff, (*b)[1] );
							qglVertex3f( (*c)[0] + xOff, (*c)[2] + yOff, (*c)[1] );
						} else {
							for ( k = 0 ; k < 3 ; k++ ) {
								int		v;
								float	*n;
								float	*a;

								v = mesh->indexes[j+k];
								n = mesh->verts[v].normal.ToFloatPtr();

								// NULLNORMAL is used by the artists to force an area to reflect no
								// light at all
								if ( surf->shader->GetSurfaceFlags() & SURF_NULLNORMAL ) {
									qglColor3f( 0.5, 0.5, 0.5 );
								} else {
								// we are going to flip the normal Z direction
									qglColor3f( 0.5 + 0.5*n[0], 0.5 - 0.5*n[2], 0.5 - 0.5*n[1] );
								}

								a = mesh->verts[v].xyz.ToFloatPtr();
								qglVertex3f( a[0] + xOff, a[2] + yOff, a[1] );
							}
						}
					}
				}
			}

			qglEnd();
			qglFlush();
			GLimp_SwapBuffers();
			qglReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer ); 

			if ( colorPass ) {
				// add to the sum buffer
				for ( i = 0 ; i < c ; i++ ) {
					colorSumBuffer[i*4+0] += buffer[i*4+0];
					colorSumBuffer[i*4+1] += buffer[i*4+1];
					colorSumBuffer[i*4+2] += buffer[i*4+2];
					colorSumBuffer[i*4+3] += buffer[i*4+3];
				}
			} else {
				// normalize
				c = width * height;
				for ( i = 0 ; i < c ; i++ ) {
					idVec3	v;

					v[0] = ( buffer[i*4+0] - 128 ) / 127.0;
					v[1] = ( buffer[i*4+1] - 128 ) / 127.0;
					v[2] = ( buffer[i*4+2] - 128 ) / 127.0;

					v.Normalize();

					buffer[i*4+0] = 128 + 127 * v[0];
					buffer[i*4+1] = 128 + 127 * v[1];
					buffer[i*4+2] = 128 + 127 * v[2];
				}

				// outline into non-drawn areas
				for ( i = 0 ; i < 8 ; i++ ) {
					OutlineNormalMap( buffer, width, height, 128, 128, 128 );
				}

				// add to the sum buffer
				for ( i = 0 ; i < c ; i++ ) {
					sumBuffer[i*4+0] += buffer[i*4+0];
					sumBuffer[i*4+1] += buffer[i*4+1];
					sumBuffer[i*4+2] += buffer[i*4+2];
					sumBuffer[i*4+3] += buffer[i*4+3];
				}
			}
		}
	}

	c = width * height;

	// save out the color map
	for ( i = 0 ; i < c ; i++ ) {
		buffer[i*4+0] = colorSumBuffer[i*4+0] / 16;
		buffer[i*4+1] = colorSumBuffer[i*4+1] / 16;
		buffer[i*4+2] = colorSumBuffer[i*4+2] / 16;
		buffer[i*4+3] = colorSumBuffer[i*4+3] / 16;
	}
	filename = source;
	filename.StripFileExtension();
	filename.Append( "_color.tga" );
	R_VerticalFlip( buffer, width, height );
	R_WriteTGA( filename, buffer, width, height );

	// save out the local map
	// scale the sum buffer back down to the sample buffer
	// we allow this to denormalize
	for ( i = 0 ; i < c ; i++ ) {
		buffer[i*4+0] = sumBuffer[i*4+0] / 16;
		buffer[i*4+1] = sumBuffer[i*4+1] / 16;
		buffer[i*4+2] = sumBuffer[i*4+2] / 16;
		buffer[i*4+3] = sumBuffer[i*4+3] / 16;
	}

	filename = source;
	filename.StripFileExtension();
	filename.Append( "_local.tga" );
	common->Printf( "writing %s (%i,%i)\n", filename.c_str(), width, height );
	R_VerticalFlip( buffer, width, height );
	R_WriteTGA( filename, buffer, width, height );


	// free the model
	renderModelManager->FreeModel( highPolyModel );

	// free our work buffer
	Mem_Free( buffer );
	Mem_Free( sumBuffer );
	Mem_Free( colorSumBuffer );

	RestoreWindow();

	// stop updating the screen as we print
	common->SetRefreshOnPrint( false );

	common->Error( "Completed." );
}

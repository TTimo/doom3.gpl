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

//#define TEST_TRACE

/*
=================
R_LocalTrace

If we resort the vertexes so all silverts come first, we can save some work here.
=================
*/
localTrace_t R_LocalTrace( const idVec3 &start, const idVec3 &end, const float radius, const srfTriangles_t *tri ) {
	int			i, j;
	byte *		cullBits;
	idPlane		planes[4];
	localTrace_t	hit;
	int			c_testEdges, c_testPlanes, c_intersect;
	idVec3		startDir;
	byte		totalOr;
	float		radiusSqr;

#ifdef TEST_TRACE
	idTimer		trace_timer;
	trace_timer.Start();
#endif

	hit.fraction = 1.0f;

	// create two planes orthogonal to each other that intersect along the trace
	startDir = end - start;
	startDir.Normalize();
	startDir.NormalVectors( planes[0].Normal(), planes[1].Normal() );
	planes[0][3] = - start * planes[0].Normal();
	planes[1][3] = - start * planes[1].Normal();

	// create front and end planes so the trace is on the positive sides of both
	planes[2] = startDir;
	planes[2][3] = - start * planes[2].Normal();
	planes[3] = -startDir;
	planes[3][3] = - end * planes[3].Normal();

	// catagorize each point against the four planes
	cullBits = (byte *) _alloca16( tri->numVerts );
	SIMDProcessor->TracePointCull( cullBits, totalOr, radius, planes, tri->verts, tri->numVerts );

	// if we don't have points on both sides of both the ray planes, no intersection
	if ( ( totalOr ^ ( totalOr >> 4 ) ) & 3 ) {
		//common->Printf( "nothing crossed the trace planes\n" );
		return hit;
	}

	// if we don't have any points between front and end, no intersection
	if ( ( totalOr ^ ( totalOr >> 1 ) ) & 4 ) {
		//common->Printf( "trace didn't reach any triangles\n" );
		return hit;
	}

	// scan for triangles that cross both planes
	c_testPlanes = 0;
	c_testEdges = 0;
	c_intersect = 0;

	radiusSqr = Square( radius );
	startDir = end - start;

	if ( !tri->facePlanes || !tri->facePlanesCalculated ) {
		R_DeriveFacePlanes( const_cast<srfTriangles_t *>( tri ) );
	}

	for ( i = 0, j = 0; i < tri->numIndexes; i += 3, j++ ) {
		float		d1, d2, f, d;
		float		edgeLengthSqr;
		idPlane *	plane;
		idVec3		point;
		idVec3		dir[3];
		idVec3		cross;
		idVec3		edge;
		byte		triOr;

		// get sidedness info for the triangle
		triOr  = cullBits[ tri->indexes[i+0] ];
		triOr |= cullBits[ tri->indexes[i+1] ];
		triOr |= cullBits[ tri->indexes[i+2] ];

		// if we don't have points on both sides of both the ray planes, no intersection
		if ( ( triOr ^ ( triOr >> 4 ) ) & 3 ) {
			continue;
		}

		// if we don't have any points between front and end, no intersection
		if ( ( triOr ^ ( triOr >> 1 ) ) & 4 ) {
			continue;
		}

		c_testPlanes++;

		plane = &tri->facePlanes[j];
		d1 = plane->Distance( start );
		d2 = plane->Distance( end );

		if ( d1 <= d2 ) {
			continue;		// comning at it from behind or parallel
		}

		if ( d1 < 0.0f ) {
			continue;		// starts past it
		}

		if ( d2 > 0.0f ) {
			continue;		// finishes in front of it
		}

		f = d1 / ( d1 - d2 );

		if ( f < 0.0f ) {
			continue;		// shouldn't happen
		}
		
		if ( f >= hit.fraction ) {
			continue;		// have already hit something closer
		}

		c_testEdges++;

		// find the exact point of impact with the plane
		point = start + f * startDir;

		// see if the point is within the three edges
		// if radius > 0 the triangle is expanded with a circle in the triangle plane

		dir[0] = tri->verts[ tri->indexes[i+0] ].xyz - point;
		dir[1] = tri->verts[ tri->indexes[i+1] ].xyz - point;

		cross = dir[0].Cross( dir[1] );
		d = plane->Normal() * cross;
		if ( d > 0.0f ) {
			if ( radiusSqr <= 0.0f ) {
				continue;
			}
			edge = tri->verts[ tri->indexes[i+0] ].xyz - tri->verts[ tri->indexes[i+1] ].xyz;
			edgeLengthSqr = edge.LengthSqr();
			if ( cross.LengthSqr() > edgeLengthSqr * radiusSqr ) {
				continue;
			}
			d = edge * dir[0];
			if ( d < 0.0f ) {
				edge = tri->verts[ tri->indexes[i+0] ].xyz - tri->verts[ tri->indexes[i+2] ].xyz;
				d = edge * dir[0];
				if ( d < 0.0f ) {
					if ( dir[0].LengthSqr() > radiusSqr ) {
						continue;
					}
				}
			} else if ( d > edgeLengthSqr ) {
				edge = tri->verts[ tri->indexes[i+1] ].xyz - tri->verts[ tri->indexes[i+2] ].xyz;
				d = edge * dir[1];
				if ( d < 0.0f ) {
					if ( dir[1].LengthSqr() > radiusSqr ) {
						continue;
					}
				}
			}
		}

		dir[2] = tri->verts[ tri->indexes[i+2] ].xyz - point;

		cross = dir[1].Cross( dir[2] );
		d = plane->Normal() * cross;
		if ( d > 0.0f ) {
			if ( radiusSqr <= 0.0f ) {
				continue;
			}
			edge = tri->verts[ tri->indexes[i+1] ].xyz - tri->verts[ tri->indexes[i+2] ].xyz;
			edgeLengthSqr = edge.LengthSqr();
			if ( cross.LengthSqr() > edgeLengthSqr * radiusSqr ) {
				continue;
			}
			d = edge * dir[1];
			if ( d < 0.0f ) {
				edge = tri->verts[ tri->indexes[i+1] ].xyz - tri->verts[ tri->indexes[i+0] ].xyz;
				d = edge * dir[1];
				if ( d < 0.0f ) {
					if ( dir[1].LengthSqr() > radiusSqr ) {
						continue;
					}
				}
			} else if ( d > edgeLengthSqr ) {
				edge = tri->verts[ tri->indexes[i+2] ].xyz - tri->verts[ tri->indexes[i+0] ].xyz;
				d = edge * dir[2];
				if ( d < 0.0f ) {
					if ( dir[2].LengthSqr() > radiusSqr ) {
						continue;
					}
				}
			}
		}

		cross = dir[2].Cross( dir[0] );
		d = plane->Normal() * cross;
		if ( d > 0.0f ) {
			if ( radiusSqr <= 0.0f ) {
				continue;
			}
			edge = tri->verts[ tri->indexes[i+2] ].xyz - tri->verts[ tri->indexes[i+0] ].xyz;
			edgeLengthSqr = edge.LengthSqr();
			if ( cross.LengthSqr() > edgeLengthSqr * radiusSqr ) {
				continue;
			}
			d = edge * dir[2];
			if ( d < 0.0f ) {
				edge = tri->verts[ tri->indexes[i+2] ].xyz - tri->verts[ tri->indexes[i+1] ].xyz;
				d = edge * dir[2];
				if ( d < 0.0f ) {
					if ( dir[2].LengthSqr() > radiusSqr ) {
						continue;
					}
				}
			} else if ( d > edgeLengthSqr ) {
				edge = tri->verts[ tri->indexes[i+0] ].xyz - tri->verts[ tri->indexes[i+1] ].xyz;
				d = edge * dir[0];
				if ( d < 0.0f ) {
					if ( dir[0].LengthSqr() > radiusSqr ) {
						continue;
					}
				}
			}
		}

		// we hit it
		c_intersect++;

		hit.fraction = f;
		hit.normal = plane->Normal();
		hit.point = point;
		hit.indexes[0] = tri->indexes[i];
		hit.indexes[1] = tri->indexes[i+1];
		hit.indexes[2] = tri->indexes[i+2];
	}


#ifdef TEST_TRACE
	trace_timer.Stop();
	common->Printf( "testVerts:%i c_testPlanes:%i c_testEdges:%i c_intersect:%i msec:%1.4f\n", 
					tri->numVerts, c_testPlanes, c_testEdges, c_intersect, trace_timer.Milliseconds() );
#endif

	return hit;
}

/*
=================
RB_DrawExpandedTriangles
=================
*/
void RB_DrawExpandedTriangles( const srfTriangles_t *tri, const float radius, const idVec3 &vieworg ) {
	int i, j, k;
	idVec3 dir[6], normal, point;

	for ( i = 0; i < tri->numIndexes; i += 3 ) {

		idVec3 p[3] = { tri->verts[ tri->indexes[ i + 0 ] ].xyz, tri->verts[ tri->indexes[ i + 1 ] ].xyz, tri->verts[ tri->indexes[ i + 2 ] ].xyz };

		dir[0] = p[0] - p[1];
		dir[1] = p[1] - p[2];
		dir[2] = p[2] - p[0];

		normal = dir[0].Cross( dir[1] );

		if ( normal * p[0] < normal * vieworg ) {
			continue;
		}

		dir[0] = normal.Cross( dir[0] );
		dir[1] = normal.Cross( dir[1] );
		dir[2] = normal.Cross( dir[2] );

		dir[0].Normalize();
		dir[1].Normalize();
		dir[2].Normalize();

		qglBegin( GL_LINE_LOOP );

		for ( j = 0; j < 3; j++ ) {
			k = ( j + 1 ) % 3;

			dir[4] = ( dir[j] + dir[k] ) * 0.5f;
			dir[4].Normalize();

			dir[3] = ( dir[j] + dir[4] ) * 0.5f;
			dir[3].Normalize();

			dir[5] = ( dir[4] + dir[k] ) * 0.5f;
			dir[5].Normalize();

			point = p[k] + dir[j] * radius;
			qglVertex3f( point[0], point[1], point[2] );

			point = p[k] + dir[3] * radius;
			qglVertex3f( point[0], point[1], point[2] );

			point = p[k] + dir[4] * radius;
			qglVertex3f( point[0], point[1], point[2] );

			point = p[k] + dir[5] * radius;
			qglVertex3f( point[0], point[1], point[2] );

			point = p[k] + dir[k] * radius;
			qglVertex3f( point[0], point[1], point[2] );
		}

		qglEnd();
	}
}

/*
================
RB_ShowTrace

Debug visualization
================
*/
void RB_ShowTrace( drawSurf_t **drawSurfs, int numDrawSurfs ) {
	int						i;
	const srfTriangles_t	*tri;
	const drawSurf_t		*surf;
	idVec3					start, end;
	idVec3					localStart, localEnd;
	localTrace_t			hit;
	float					radius;

	if ( r_showTrace.GetInteger() == 0 ) {
		return;
	}

	if ( r_showTrace.GetInteger() == 2 ) {
		radius = 5.0f;
	} else {
		radius = 0.0f;
	}

	// determine the points of the trace
	start = backEnd.viewDef->renderView.vieworg;
	end = start + 4000 * backEnd.viewDef->renderView.viewaxis[0];

	// check and draw the surfaces
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	GL_TexEnv( GL_MODULATE );

	globalImages->whiteImage->Bind();

	// find how many are ambient
	for ( i = 0 ; i < numDrawSurfs ; i++ ) {
		surf = drawSurfs[i];
		tri = surf->geo;

		if ( tri == NULL || tri->verts == NULL ) {
			continue;
		}

		// transform the points into local space
		R_GlobalPointToLocal( surf->space->modelMatrix, start, localStart );
		R_GlobalPointToLocal( surf->space->modelMatrix, end, localEnd );

		// check the bounding box
		if ( !tri->bounds.Expand( radius ).LineIntersection( localStart, localEnd ) ) {
			continue;
		}

		qglLoadMatrixf( surf->space->modelViewMatrix );

		// highlight the surface
		GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );

		qglColor4f( 1, 0, 0, 0.25 );
		RB_DrawElementsImmediate( tri );

		// draw the bounding box
		GL_State( GLS_DEPTHFUNC_ALWAYS );

		qglColor4f( 1, 1, 1, 1 );
		RB_DrawBounds( tri->bounds );

		if ( radius != 0.0f ) {
			// draw the expanded triangles
			qglColor4f( 0.5f, 0.5f, 1.0f, 1.0f );
			RB_DrawExpandedTriangles( tri, radius, localStart );
		}

		// check the exact surfaces
		hit = R_LocalTrace( localStart, localEnd, radius, tri );
		if ( hit.fraction < 1.0 ) {
			qglColor4f( 1, 1, 1, 1 );
			RB_DrawBounds( idBounds( hit.point ).Expand( 1 ) );
		}
	}
}

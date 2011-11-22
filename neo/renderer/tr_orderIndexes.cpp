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
===============
R_MeshCost
===============
*/
#define	CACHE_SIZE	24
#define	STALL_SIZE	8
int	R_MeshCost( int numIndexes, glIndex_t *indexes ) {
	int	inCache[CACHE_SIZE];
	int	i, j, v;
	int	c_stalls;
	int	c_loads;
	int	fifo;

	for ( i = 0 ; i < CACHE_SIZE ; i++ ) {
		inCache[i] = -1;
	}

	c_loads = 0;
	c_stalls = 0;
	fifo = 0;

	for ( i = 0 ; i < numIndexes ; i++ ) {
		v = indexes[i];
		for ( j = 0 ; j < CACHE_SIZE ; j++ ) {
			if ( inCache[ ( fifo + j ) % CACHE_SIZE ] == v ) {
				break;
			}
		}
		if ( j == CACHE_SIZE ) {
			c_loads++;
			inCache[ fifo % CACHE_SIZE ] = v;
			fifo++;
		} else if ( j < STALL_SIZE ) {
			c_stalls++;
		}
	}

	return c_loads;
}


typedef struct vertRef_s {
	struct vertRef_s	*next;
	int			tri;
} vertRef_t;

/*
====================
R_OrderIndexes

Reorganizes the indexes so they will take best advantage
of the internal GPU vertex caches
====================
*/
void R_OrderIndexes( int numIndexes, glIndex_t *indexes ) {
	bool	*triangleUsed;
	int			numTris;
	glIndex_t	*oldIndexes;
	glIndex_t	*base;
	int			numOldIndexes;
	int			tri;
	int			i;
	vertRef_t	*vref, **vrefs, *vrefTable;
	int			numVerts;
	int			v1, v2;
	int			c_starts;
	int			c_cost;

	if ( !r_orderIndexes.GetBool() ) {
		return;
	}

	// save off the original indexes
	oldIndexes = (glIndex_t *)_alloca( numIndexes * sizeof( *oldIndexes ) );
	memcpy( oldIndexes, indexes, numIndexes * sizeof( *oldIndexes ) );
	numOldIndexes = numIndexes;

	// make a table to mark the triangles when they are emited
	numTris = numIndexes / 3;
	triangleUsed = (bool *)_alloca( numTris * sizeof( *triangleUsed ) );
	memset( triangleUsed, 0, numTris * sizeof( *triangleUsed ) );

	// find the highest vertex number
	numVerts = 0;
	for ( i = 0 ; i < numIndexes ; i++ ) {
		if ( indexes[i] > numVerts ) {
			numVerts = indexes[i];
		}
	}
	numVerts++;

	// create a table of triangles used by each vertex
	vrefs = (vertRef_t **)_alloca( numVerts * sizeof( *vrefs ) );
	memset( vrefs, 0, numVerts * sizeof( *vrefs ) );

	vrefTable = (vertRef_t *)_alloca( numIndexes * sizeof( *vrefTable ) );
	for ( i = 0 ; i < numIndexes ; i++ ) {
		tri = i / 3;

		vrefTable[i].tri = tri;
		vrefTable[i].next = vrefs[oldIndexes[i]];
		vrefs[oldIndexes[i]] = &vrefTable[i];
	}

	// generate new indexes
	numIndexes = 0;
	c_starts = 0;
	while ( numIndexes != numOldIndexes ) {
		// find a triangle that hasn't been used
		for ( tri = 0 ; tri < numTris ; tri++ ) {
			if ( !triangleUsed[tri] ) {
				break;
			}
		}
		if ( tri == numTris ) {
			common->Error( "R_OrderIndexes: ran out of unused tris" );
		}

		c_starts++;

		do {
			// emit this tri
			base = oldIndexes + tri * 3;
			indexes[numIndexes+0] = base[0];
			indexes[numIndexes+1] = base[1];
			indexes[numIndexes+2] = base[2];
			numIndexes += 3;

			triangleUsed[tri] = true;

			// try to find a shared edge to another unused tri
			for ( i = 0 ; i < 3 ; i++ ) {
				v1 = base[i];
				v2 = base[(i+1)%3];

				for ( vref = vrefs[v1] ; vref ; vref = vref->next ) {
					tri = vref->tri;
					if ( triangleUsed[tri] ) {
						continue;
					}

					// if this triangle also uses v2, grab it
					if ( oldIndexes[tri*3+0] == v2
						|| oldIndexes[tri*3+1] == v2
						|| oldIndexes[tri*3+2] == v2 ) {
						break;
					}
				}
				if ( vref ) {
					break;
				}
			}

			// if we couldn't chain off of any verts, we need to find a new one
			if ( i == 3 ) {
				break;
			}
		} while ( 1 );
	}

	c_cost = R_MeshCost( numIndexes, indexes );

}


/*

  add all triangles that can be specified by the vertexes in the last 14 cache positions

  pick a new vert to add to the cache
  don't pick one in the 24 previous cache positions
  try to pick one that will enable the creation of as many triangles as possible

  look for a vert that shares an edge with the vert about to be evicted


*/


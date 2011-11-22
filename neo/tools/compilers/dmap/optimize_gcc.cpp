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
/*
crazy gcc 3.3.5 optimization bug
happens even at -O1
if you remove the 'return NULL;' after Error(), it only happens at -O3 / release
see dmap.gcc.zip test map and .proc outputs
*/

#include "../../../idlib/precompiled.h"
#pragma hdrstop

#include "dmap.h"

extern idBounds optBounds;

#define MAX_OPT_VERTEXES 0x10000
extern int numOptVerts;
extern optVertex_t optVerts[MAX_OPT_VERTEXES];

/*
================
FindOptVertex
================
*/
optVertex_t *FindOptVertex( idDrawVert *v, optimizeGroup_t *opt ) {
	int		i;
	float	x, y;
	optVertex_t	*vert;

	// deal with everything strictly as 2D
	x = v->xyz * opt->axis[0];
	y = v->xyz * opt->axis[1];

	// should we match based on the t-junction fixing hash verts?
	for ( i = 0 ; i < numOptVerts ; i++ ) {
		if ( optVerts[i].pv[0] == x && optVerts[i].pv[1] == y ) {
			return &optVerts[i];
		}
	}

	if ( numOptVerts >= MAX_OPT_VERTEXES ) {
		common->Error( "MAX_OPT_VERTEXES" );
		return NULL;
	}
	
	numOptVerts++;

	vert = &optVerts[i];
	memset( vert, 0, sizeof( *vert ) );
	vert->v = *v;
	vert->pv[0] = x;
	vert->pv[1] = y;
	vert->pv[2] = 0;

	optBounds.AddPoint( vert->pv );

	return vert;
}

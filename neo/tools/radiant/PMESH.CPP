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

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "qe3.h"
#include "DialogInfo.h"
#include "CapDialog.h"

// externs
extern void MemFile_fprintf( CMemFile *pMemFile,const char *pText,... );
extern face_t *Face_Alloc( void );
void _Write3DMatrix( FILE *f,int y,int x,int z,float *m );
void _Write3DMatrix( CMemFile *f,int y,int x,int z,float *m );
extern void SamplePatch( float ctrl[3][3][5],int baseCol,int baseRow,int width,int horzSub,int vertSub,idDrawVert *outVerts,idDrawVert *drawVerts );
patchMesh_t *Patch_GenerateGeneric( int width,int height,int orientation,const idVec3 &mins,const idVec3 &maxs );



patchMesh_t * MakeNewPatch( int width,int height ) {
	patchMesh_t	*pm	= reinterpret_cast< patchMesh_t*>(Mem_ClearedAlloc(sizeof(patchMesh_t)));
	pm->horzSubdivisions = DEFAULT_CURVE_SUBDIVISION;
	pm->vertSubdivisions = DEFAULT_CURVE_SUBDIVISION;
	pm->explicitSubdivisions = false;
	pm->width = width;
	pm->height = height;
	pm->verts = reinterpret_cast< idDrawVert*>(Mem_ClearedAlloc(sizeof(idDrawVert) * width * height));
	//pm->ctrl = reinterpret_cast<idDrawVert*>(Mem_ClearedAlloc(sizeof(idDrawVert) * width * height));
	//pm->ctrl = &pm->verts;
	return pm;
}


void Patch_AdjustSize( patchMesh_t *p,int wadj,int hadj ) {
	idDrawVert	*newverts	= reinterpret_cast< idDrawVert*>(Mem_ClearedAlloc(sizeof(idDrawVert) * (p->width + wadj) * (p->height + hadj)));
	int			copyWidth	= (wadj < 0) ? p->width + wadj : p->width;
	int			copyHeight	= (hadj < 0) ? p->height + hadj : p->height;
	int			copysize	= copyWidth *copyHeight * sizeof(idDrawVert);

	for ( int i = 0; i < p->width; i++ ) {
		for ( int j = 0; j < p->height; j++ ) {
			newverts[j * (p->width + wadj) + i] = p->ctrl(i, j);
		}
	}

	p->width += wadj;
	p->height += hadj;
	Mem_Free(p->verts);
	p->verts = newverts;
}

// algorithm from Journal of graphics tools, 2(1):21-28, 1997
bool RayIntersectsTri( const idVec3 &origin,const idVec3 &direction,const idVec3 &vert0,const idVec3 &vert1,const idVec3 &vert2,float &scale ) {
	idVec3	edge1, edge2, tvec, pvec, qvec;
	float	det, inv_det;
	scale = 0;

	/* find vectors for two edges sharing vert0 */
	edge1 = vert1 - vert0;
	edge2 = vert2 - vert0;

	/* begin calculating determinant - also used to calculate U parameter */
	pvec.Cross(direction, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = edge1 * pvec;

	if ( det > -VECTOR_EPSILON && det < VECTOR_EPSILON ) {
		return false;
	}

	inv_det = 1.0f / det;

	/* calculate distance from vert0 to ray origin */
	tvec = origin - vert0;

	/* calculate U parameter and test bounds */
	float	u	= (tvec *pvec) * inv_det;
	if ( u < 0.0f || u> 1.0f ) {
		return false;
	}

	/* prepare to test V parameter */
	qvec.Cross(tvec, edge1);

	/* calculate V parameter and test bounds */
	float	v	= (direction *qvec) * inv_det;
	if ( v < 0.0f || u + v> 1.0f ) {
		return false;
	}

	scale = tvec.Length();
	return true;
}

bool Patch_Intersect( patchMesh_t *pm,idVec3 origin,idVec3 direction,float &scale ) {
	int				i, j;
	//float scale;
	idSurface_Patch	cp	(pm->width * 6, pm->height * 6);

	cp.SetSize(pm->width, pm->height);
	for ( i = 0; i < pm->width; i++ ) {
		for ( j = 0; j < pm->height; j++ ) {
			(cp)[j * cp.GetWidth() + i].xyz = pm->ctrl(i, j).xyz;
			(cp)[j * cp.GetWidth() + i].st = pm->ctrl(i, j).st;
		}
	}

	if ( pm->explicitSubdivisions ) {
		cp.SubdivideExplicit(pm->horzSubdivisions, pm->vertSubdivisions, false);
	} else {
		cp.Subdivide(DEFAULT_CURVE_MAX_ERROR, DEFAULT_CURVE_MAX_ERROR, DEFAULT_CURVE_MAX_LENGTH, false);
	}

	if ( cp.RayIntersection(origin, direction, scale) ) {
		return true;
	} else {
		return false;
	}

	/*
	int width = cp.GetWidth();
	int height = cp.GetHeight();
	for ( i = 0 ; i < width - 1; i++ ) {
		for ( j = 0 ; j < height - 1; j++ ) {
			// v1-v2-v3-v4 makes a quad
			int v1, v2, v3, v4;
			v1 = j * width + i;
			v2 = v1 + 1;
			v3 = v1 + width + 1;
			v4 = v1 + width;
			if (RayIntersectsTri(origin, direction, (cp)[v1].xyz, (cp)[v2].xyz, (cp)[v3].xyz)) {
				return true;
			}
			if (RayIntersectsTri(origin, direction, (cp)[v3].xyz, (cp)[v4].xyz, (cp)[v1].xyz)) {
				return true;
			}
		}
	}
	return false;
	*/
}

patchMesh_t * Patch_MakeNew( patchMesh_t *p,int newWidth,int newHeight ) {
	patchMesh_t	*newPatch	= MakeNewPatch(newWidth, newHeight);
	newPatch->d_texture = p->d_texture;
	newPatch->horzSubdivisions = p->horzSubdivisions;
	newPatch->vertSubdivisions = p->vertSubdivisions;
	newPatch->explicitSubdivisions = p->explicitSubdivisions;
	return newPatch;
}

void Patch_Combine( patchMesh_t *p,patchMesh_t *p2,int sourceCol1,int sourceCol2,int sourceRow1,int sourceRow2,bool invert1,bool invert2 ) {
	int			i, j, out;
	patchMesh_t	*newPatch	= NULL;
	if ( sourceCol1 >= 0 ) {
		// adding width
		if ( sourceCol2 >= 0 ) {
			// from width
			newPatch = Patch_MakeNew(p, p->width + p2->width - 1, p->height);
			int	adj1	= 1;
			int	adj2	= 1;
			int	col1	= 0;
			int	col2	= 1;
			if ( sourceCol1 != 0 ) {
				adj1 = -1;
				col1 = p->width - 1;
			} 
			if ( sourceCol2 != 0 ) {
				adj2 = -1;
				col2 = p2->width - 2;
			}

			out = 0;
			for ( i = 0; i < p->width; i++, col1 += adj1 ) {
				int	in	= (invert1) ? p->height - 1 : 0;
				for ( j = 0; j < p->height; j++ ) {
					newPatch->ctrl(out, j) = p->ctrl(col1, in);
					in += (invert1) ? -1 : 1;
				}
				out++;
			}

			for ( i = 1; i < p2->width; i++, col2 += adj2 ) {
				int	in	= (invert2) ? p2->height - 1 : 0;
				for ( j = 0; j < p2->height; j++ ) {
					newPatch->ctrl(out, j) = p2->ctrl(col2, in);
					in += (invert2) ? -1 : 1;
				}
				out++;
			}
		} else {
			// from height
			newPatch = Patch_MakeNew(p, p->width + p2->height - 1, p->height);
			int	adj1	= 1;
			int	adj2	= 1;
			int	col1	= 0;
			int	row2	= 1;
			if ( sourceCol1 != 0 ) {
				adj1 = -1;
				col1 = p->width - 1;
			} 
			if ( sourceRow2 != 0 ) {
				adj2 = -1;
				row2 = p2->height - 2;
			}

			out = 0;
			for ( i = 0; i < p->width; i++, col1 += adj1 ) {
				int	in	= (invert1) ? p->height - 1 : 0;
				for ( j = 0; j < p->height; j++ ) {
					newPatch->ctrl(out, j) = p->ctrl(col1, in);
					in += (invert1) ? -1 : 1;
				}
				out++;
			}

			for ( i = 1; i < p2->height; i++, row2 += adj2 ) {
				int	in	= (invert2) ? p2->width - 1 : 0;
				for ( j = 0; j < p2->width; j++ ) {
					newPatch->ctrl(out, j) = p2->ctrl(in, row2);
					in += (invert2) ? -1 : 1;
				}
				out++;
			}
		}
	} else {
		// adding height
		if ( sourceRow1 >= 0 ) {
			// from height
			newPatch = Patch_MakeNew(p, p->width, p->height + p2->height - 1);
			int	adj1	= 1;
			int	adj2	= 1;
			int	row1	= 0;
			int	row2	= 0;
			if ( sourceRow1 != 0 ) {
				adj1 = -1;
				row1 = p->height - 1;
			} 
			if ( sourceRow2 != 0 ) {
				adj2 = -1;
				row2 = p2->height - 2;
			}

			out = 0;
			for ( i = 0; i < p->height; i++, row1 += adj1 ) {
				int	in	= (invert1) ? p->width - 1 : 0;
				for ( j = 0; j < p->width; j++ ) {
					newPatch->ctrl(j, out) = p->ctrl(in, row1);
					in += (invert1) ? -1 : 1;
				}
				out++;
			}

			for ( i = 1; i < p2->height; i++, row2 += adj2 ) {
				int	in	= (invert2) ? p->width - 1 : 0;
				for ( j = 0; j < p2->width; j++ ) {
					newPatch->ctrl(j, out) = p2->ctrl(in, row2);
					in += (invert1) ? -1 : 1;
				}
				out++;
			}
		} else {
			// from width
			newPatch = Patch_MakeNew(p, p->width, p->height + p2->width - 1);
			int	adj1	= 1;
			int	adj2	= 1;
			int	row1	= 0;
			int	col2	= 0;
			if ( sourceRow1 != 0 ) {
				adj1 = -1;
				row1 = p->height - 1;
			} 
			if ( sourceCol2 != 0 ) {
				adj2 = -1;
				col2 = p2->width - 2;
			}

			out = 0;
			for ( i = 0; i < p->height; i++, row1 += adj1 ) {
				int	in	= (invert1) ? p->width - 1 : 0;
				for ( j = 0; j < p->width; j++ ) {
					newPatch->ctrl(j, out) = p->ctrl(in, row1);
					in += (invert1) ? -1 : 1;
				}
				out++;
			}

			for ( i = 1; i < p2->width; i++, col2 += adj2 ) {
				int	in	= (invert2) ? p->height - 1 : 0;
				for ( j = 0; j < p2->height; j++ ) {
					newPatch->ctrl(j, out) = p2->ctrl(col2, in);
					in += (invert1) ? -1 : 1;
				}
				out++;
			}
		}
	}
	if ( newPatch ) {
		AddBrushForPatch(newPatch, true);
		Brush_Free(p->pSymbiot, true);
		Brush_Free(p2->pSymbiot, true);
		Patch_Naturalize(newPatch, true, true);
	}
}

#define WELD_EPSILON	0.001f

void Patch_Weld( patchMesh_t *p,patchMesh_t *p2 ) {
	// check against all 4 edges of p2
	// could roll this up but left it out for some semblence of clarity
	// 

	if ( p->width == p2->width ) {
		int	row		= 0;
		int	row2	= 0;
		while ( 1 ) {
			bool	match	= true;

			// need to see if any of the corners match then run down or up based
			// on the match edges
			int		col1	= 0;
			int		col2	= 0;
			int		adj1	= 1;
			int		adj2	= 1;
			if ( p->ctrl(0, row).xyz.Compare(p2->ctrl(0, row2).xyz, WELD_EPSILON) ) {
			} else if ( p->ctrl(0, row).xyz.Compare(p2->ctrl(p2->width - 1, row2).xyz, WELD_EPSILON) ) {
				col2 = p2->width - 1;
				adj2 = -1;
			} else if ( p->ctrl(p->width - 1, row).xyz.Compare(p2->ctrl(p2->width - 1, row2).xyz, WELD_EPSILON) ) {
				col2 = p2->width - 1;
				adj2 = -1;
				col1 = p->width - 1;
				adj1 = -1;
			} else if ( p->ctrl(p->width - 1, row).xyz.Compare(p2->ctrl(0, row2).xyz, WELD_EPSILON) ) {
				col1 = p->width - 1;
				adj1 = -1;
			} else {
				adj1 = 0;
			}

			if ( adj1 ) {
				for ( int col = 0; col < p->width; col++, col2 += adj2, col1 += adj1 ) {
					if ( !p->ctrl(col1, row).xyz.Compare(p2->ctrl(col2, row2).xyz, WELD_EPSILON) ) {
						match = false;
						break;
					}
				}
			} else {
				match = false;
			}

			if ( match ) {
				// have a match weld these edges
				common->Printf("Welding row %i with row %i\n", row, row2);
				row = (row == 0) ? p->height - 1 : 0;
				Patch_Combine(p, p2, -1, -1, row, row2, (adj1 == -1), (adj2 == -1));
				return;
			} else if ( row2 == 0 ) {
				row2 = p2->height - 1;
			} else if ( row == 0 ) {
				row = p->height - 1;
				row2 = 0;
			} else {
				break;
			}
		}
	}

	if ( p->width == p2->height ) {
		int	row		= 0;
		int	col2	= 0;
		while ( 1 ) {
			bool	match	= true;

			int		col1	= 0;
			int		adj1	= 1;
			int		row2	= 0;
			int		adj2	= 1;
			if ( p->ctrl(0, row).xyz.Compare(p2->ctrl(col2, 0).xyz, WELD_EPSILON) ) {
			} else if ( p->ctrl(0, row).xyz.Compare(p2->ctrl(col2, p2->height - 1).xyz, WELD_EPSILON) ) {
				row2 = p2->height - 1;
				adj2 = -1;
			} else if ( p->ctrl(p->width - 1, row).xyz.Compare(p2->ctrl(col2, p2->height - 1).xyz, WELD_EPSILON) ) {
				row2 = p2->height - 1;
				adj2 = -1;
				col1 = p->width - 1;
				adj1 = -1;
			} else if ( p->ctrl(p->width - 1, row).xyz.Compare(p2->ctrl(col2, 0).xyz, WELD_EPSILON) ) {
				col1 = p->width - 1;
				adj1 = -1;
			} else {
				adj1 = 0;
			}

			if ( adj1 ) {
				for ( int col = 0; col < p->width; col++, col1 += adj1, row2 += adj2 ) {
					if ( !p->ctrl(col1, row).xyz.Compare(p2->ctrl(col2, row2).xyz, WELD_EPSILON) ) {
						match = false;
						break;
					}
				}
			} else {
				match = false;
			}

			if ( match ) {
				// have a match weld these edges
				common->Printf("Welding row %i with col %i\n", row, col2);
				row = (row == 0) ? p->height - 1 : 0;
				Patch_Combine(p, p2, -1, col2, row, -1, (adj1 == -1), (adj2 == -1));
				return;
			} else if ( col2 == 0 ) {
				col2 = p2->width - 1;
			} else if ( row == 0 ) {
				row = p->height - 1;
				col2 = 0;
			} else {
				break;
			}
		}
	}

	if ( p->height == p2->width ) {
		int	col		= 0;
		int	row2	= 0;
		while ( 1 ) {
			bool	match	= true;


			int		row1	= 0;
			int		adj1	= 1;
			int		col2	= 0;
			int		adj2	= 1;
			if ( p->ctrl(col, 0).xyz.Compare(p2->ctrl(0, row2).xyz, WELD_EPSILON) ) {
			} else if ( p->ctrl(col, 0).xyz.Compare(p2->ctrl(p2->width - 1, row2).xyz, WELD_EPSILON) ) {
				col2 = p2->width - 1;
				adj2 = -1;
			} else if ( p->ctrl(col, p->height - 1).xyz.Compare(p2->ctrl(p2->width - 1, row2).xyz, WELD_EPSILON) ) {
				col2 = p2->width - 1;
				adj2 = -1;
				row1 = p2->height - 1;
				adj2 = -1;
			} else if ( p->ctrl(col, p->height - 1).xyz.Compare(p2->ctrl(0, row2).xyz, WELD_EPSILON) ) {
				row1 = p2->height - 1;
				adj2 = -1;
			} else {
				adj1 = 0;
			}

			if ( adj1 ) {
				for ( int row = 0; row < p->height; row++, row1 += adj1, col2 += adj2 ) {
					if ( !p->ctrl(col, row1).xyz.Compare(p2->ctrl(col2, row2).xyz, WELD_EPSILON) ) {
						match = false;
						break;
					}
				}
			} else {
				match = false;
			}

			if ( match ) {
				// have a match weld these edges
				common->Printf("Welding col %i with row %i\n", col, row2);
				col = (col == 0) ? p->width - 1 : 0;
				Patch_Combine(p, p2, col, -1, -1, row2, (adj1 == -1), (adj2 == -1));
				return;
			} else if ( row2 == 0 ) {
				row2 = p2->height - 1;
			} else if ( col == 0 ) {
				col = p->width - 1;
				row2 = 0;
			} else {
				break;
			}
		}
	}

	if ( p->height == p2->height ) {
		int	col		= 0;
		int	col2	= 0;
		while ( 1 ) {
			bool	match	= true;


			int		row1	= 0;
			int		adj1	= 1;
			int		row2	= 0;
			int		adj2	= 1;
			if ( p->ctrl(col, 0).xyz.Compare(p2->ctrl(col2, 0).xyz, WELD_EPSILON) ) {
			} else if ( p->ctrl(col, 0).xyz.Compare(p2->ctrl(col2, p2->height - 1).xyz, WELD_EPSILON) ) {
				row2 = p2->height - 1;
				adj2 = -1;
			} else if ( p->ctrl(col, p2->height - 1).xyz.Compare(p2->ctrl(col2, p2->height - 1).xyz, WELD_EPSILON) ) {
				row2 = p2->height - 1;
				adj2 = -1;
				row1 = p->height - 1;
				adj1 = -1;
			} else if ( p->ctrl(col, p2->height - 1).xyz.Compare(p2->ctrl(col2, 0).xyz, WELD_EPSILON) ) {
				row1 = p->height - 1;
				adj1 = -1;
			} else {
				adj1 = 0;
			}

			if ( adj1 ) {
				for ( int row = 0; row < p->height; row++, row1 += adj1, row2 += adj2 ) {
					if ( !p->ctrl(col, row1).xyz.Compare(p2->ctrl(col2, row2).xyz, WELD_EPSILON) ) {
						match = false;
						break;
					}
				}
			} else {
				match = false;
			}

			if ( match ) {
				// have a match weld these edges
				common->Printf("Welding col %i with col %i\n", col, col2);
				col = (col == 0) ? p->width - 1 : 0;
				Patch_Combine(p, p2, col, col2, -1, -1, (adj1 == -1), (adj2 == -1));
				return;
			} else if ( col2 == 0 ) {
				col2 = p2->width - 1;
			} else if ( col == 0 ) {
				col = p->width - 1;
				col2 = 0;
			} else {
				break;
			}
		}
	}


	Sys_Status("Unable to weld patches, no common sized edges.\n");
}


// used for a save spot
patchMesh_t	*patchSave				= NULL;

// Tracks the selected patch for point manipulation/update. FIXME: Need to revert back to a generalized 
// brush approach
//--int  g_nSelectedPatch = -1;  

// HACK: for tracking which view generated the click
// as we dont want to deselect a point on a same point
// click if it is from a different view
int			g_nPatchClickedView	= -1;
bool		g_bSameView			= false;


// globals
bool		g_bPatchShowBounds	= false;
bool		g_bPatchWireFrame	= false;
bool		g_bPatchWeld		= true;
bool		g_bPatchDrillDown	= true;
bool		g_bPatchInsertMode	= false;
bool		g_bPatchBendMode	= false;
int			g_nPatchBendState	= -1;
int			g_nPatchInsertState	= -1;
int			g_nBendOriginIndex	= 0;
idVec3		g_vBendOrigin;

bool		g_bPatchAxisOnRow	= true;
int			g_nPatchAxisIndex	= 0;
bool		g_bPatchLowerEdge	= true;

// BEND states
enum {
	BEND_SELECT_ROTATION	= 0,
	BEND_SELECT_ORIGIN,
	BEND_SELECT_EDGE,
	BEND_BENDIT,
	BEND_STATE_COUNT
};

const char	*g_pBendStateMsg[]	= {
	"Use TAB to cycle through available bend axis. Press ENTER when the desired one is highlighted.", "Use TAB to cycle through available rotation axis. This will LOCK around that point. You may also use Shift + Middle Click to select an arbitrary point. Press ENTER when the desired one is highlighted", "Use TAB to choose which side to bend. Press ENTER when the desired one is highlighted.", "Use the MOUSE to bend the patch. It uses the same ui rules as Free Rotation. Press ENTER to accept the bend, press ESC to abandon it and exit Bend mode", ""
};

// INSERT states
enum {
	INSERT_SELECT_EDGE		= 0,
	INSERT_STATE_COUNT
};

const char	*g_pInsertStateMsg[]	= {
	"Use TAB to cycle through available rows/columns for insertion/deletion. Press INS to insert at the highlight, DEL to remove the pair"
};


float		*g_InversePoints[1024];

const float	fFullBright			= 1.0f;
const float	fLowerLimit			= 0.5f;
const float	fDec				= 0.05f;


void Patch_SetType( patchMesh_t *p,int nType ) {
	p->type = (p->type & PATCH_STYLEMASK) | nType;
}

void Patch_SetStyle( patchMesh_t *p,int nStyle ) {
	p->type = (p->type & PATCH_TYPEMASK) | nStyle;
}

/*
==================
Patch_MemorySize
==================
*/
int Patch_MemorySize( patchMesh_t *p ) {
	return (sizeof(patchMesh_t) + p->width * p->height * sizeof(idDrawVert));
}



/*
===============
InterpolateInteriorPoints
===============
*/
void InterpolateInteriorPoints( patchMesh_t *p ) {
	int	i, j, k;
	int	next, prev;

	for ( i = 0 ; i < p->width ; i += 2 ) {
		next = (i == p->width - 1) ? 1 : (i + 1) % p->width;
		prev = (i == 0) ? p->width - 2 : i - 1;
		for ( j = 0 ; j < p->height ; j++ ) {
			for ( k = 0 ; k < 3 ; k++ ) {
				p->ctrl(i, j).xyz[k] = (p->ctrl(next, j).xyz[k] + p->ctrl(prev, j).xyz[k]) * 0.5;
			}
		}
	}
}

/*
=================
MakeMeshNormals

=================
*/
int	neighbors[8][2]	= {
	{0,1}, {1,1}, {1,0}, {1, -1}, {0, -1}, { - 1, -1}, { - 1,0}, { - 1,1}
};

void Patch_MeshNormals( patchMesh_t *in ) {
	int			i, j, k, dist;
	idVec3		normal;
	idVec3		sum;
	int			count;
	idVec3		base;
	idVec3		delta;
	int			x, y;
	idDrawVert	*dv;
	idVec3		around[8], temp;
	bool		good[8];
	bool		wrapWidth, wrapHeight;
	float		len;

	wrapWidth = false;
	for ( i = 0 ; i < in->height ; i++ ) {
		VectorSubtract(in->ctrl(0, i).xyz, in->ctrl(in->width - 1, i).xyz, delta);
		len = delta.Length();
		if ( len > 1.0f ) {
			break;
		}
	}
	if ( i == in->height ) {
		wrapWidth = true;
	}

	wrapHeight = false;
	for ( i = 0 ; i < in->width ; i++ ) {
		VectorSubtract(in->ctrl(i, 0).xyz, in->ctrl(i, in->height - 1).xyz, delta);
		len = delta.Length();
		if ( len > 1.0f ) {
			break;
		}
	}
	if ( i == in->width ) {
		wrapHeight = true;
	}


	for ( i = 0 ; i < in->width ; i++ ) {
		for ( j = 0 ; j < in->height ; j++ ) {
			count = 0;
			//--dv = reinterpret_cast<idDrawVert*>(in.ctrl[j*in.width+i]);
			dv = &in->ctrl(i, j);
			VectorCopy(dv->xyz, base);
			for ( k = 0 ; k < 8 ; k++ ) {
				around[k] = vec3_origin;
				good[k] = false;

				for ( dist = 1 ; dist <= 3 ; dist++ ) {
					x = i + neighbors[k][0] * dist;
					y = j + neighbors[k][1] * dist;
					if ( wrapWidth ) {
						if ( x < 0 ) {
							x = in->width - 1 + x;
						} else if ( x >= in->width ) {
							x = 1 + x - in->width;
						}
					}
					if ( wrapHeight ) {
						if ( y < 0 ) {
							y = in->height - 1 + y;
						} else if ( y >= in->height ) {
							y = 1 + y - in->height;
						}
					}

					if ( x < 0 || x >= in->width || y < 0 || y >= in->height ) {
						break;					// edge of patch
					}
					//--VectorSubtract( in.ctrl[y*in.width+x]->xyz, base, temp );
					VectorSubtract(in->ctrl(x, y).xyz, base, temp);
					if ( temp.Normalize() == 0 ) {
						continue;				// degenerate edge, get more dist
					} else {
						good[k] = true;
						VectorCopy(temp, around[k]);
						break;					// good edge
					}
				}
			}

			sum = vec3_origin;
			for ( k = 0 ; k < 8 ; k++ ) {
				if ( !good[k] || !good[(k + 1) & 7] ) {
					continue;	// didn't get two points
				}
				normal = around[(k + 1) & 7].Cross(around[k]);
				if ( normal.Normalize() == 0 ) {
					continue;
				}
				VectorAdd(normal, sum, sum);
				count++;
			}
			if ( count == 0 ) {
				//printf("bad normal\n");
				count = 1;
				//continue;
			}
			dv->normal = sum;
			dv->normal.Normalize();
		}
	}
}

void Patch_MakeDirty( patchMesh_t *p ) {
	assert(p);
	p->nListID = -1;
	p->nListIDCam = -1;
	p->nListSelected = -1;
}


/*
==================
Patch_CalcBounds
==================
*/
void Patch_CalcBounds( patchMesh_t *p,idVec3 &vMin,idVec3 &vMax ) {
	vMin[0] = vMin[1] = vMin[2] = 999999;
	vMax[0] = vMax[1] = vMax[2] = -999999;

	Patch_MakeDirty(p);
	for ( int w = 0; w < p->width; w++ ) {
		for ( int h = 0; h < p->height; h++ ) {
			for ( int j = 0; j < 3; j++ ) {
				float	f	= p->ctrl(w, h).xyz[j];
				if ( f < vMin[j] )
					vMin[j] = f;
				if ( f > vMax[j] )
					vMax[j] = f;
			}
		}
	}
}

/*
==================
Brush_RebuildBrush
==================
*/
void Brush_RebuildBrush( brush_t *b,idVec3 vMins,idVec3 vMaxs,bool patch ) {
	//
	// Total hack job 
	// Rebuilds a brush
	int			i, j;
	face_t		*f, *next;
	idVec3		pts[4][2];
	texdef_t	texdef;
	// free faces

	for ( j = 0; j < 3; j++ ) {
		if ( (int) vMins[j] == (int) vMaxs[j] ) {
			vMins[j] -= 4;
			vMaxs[j] += 4;
		}
	}


	for ( f = b->brush_faces ; f ; f = next ) {
		next = f->next;
		if ( f ) {
			texdef = f->texdef;
		}
		Face_Free(f);
	}

	b->brush_faces = NULL;

	// left the last face so we can use its texdef

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( vMaxs[i] < vMins[i] ) {
			Error("Brush_RebuildBrush: backwards");
		}
	}

	pts[0][0][0] = vMins[0];
	pts[0][0][1] = vMins[1];

	pts[1][0][0] = vMins[0];
	pts[1][0][1] = vMaxs[1];

	pts[2][0][0] = vMaxs[0];
	pts[2][0][1] = vMaxs[1];

	pts[3][0][0] = vMaxs[0];
	pts[3][0][1] = vMins[1];

	for ( i = 0 ; i < 4 ; i++ ) {
		pts[i][0][2] = vMins[2];
		pts[i][1][0] = pts[i][0][0];
		pts[i][1][1] = pts[i][0][1];
		pts[i][1][2] = vMaxs[2];
	}

	for ( i = 0 ; i < 4 ; i++ ) {
		f = Face_Alloc();
		f->texdef = texdef;
		f->next = b->brush_faces;
		b->brush_faces = f;
		j = (i + 1) % 4;

		VectorCopy(pts[j][1], f->planepts[0]);
		VectorCopy(pts[i][1], f->planepts[1]);
		VectorCopy(pts[i][0], f->planepts[2]);
	}

	f = Face_Alloc();
	f->texdef = texdef;
	f->next = b->brush_faces;
	b->brush_faces = f;

	VectorCopy(pts[0][1], f->planepts[0]);
	VectorCopy(pts[1][1], f->planepts[1]);
	VectorCopy(pts[2][1], f->planepts[2]);

	f = Face_Alloc();
	f->texdef = texdef;
	f->next = b->brush_faces;
	b->brush_faces = f;

	VectorCopy(pts[2][0], f->planepts[0]);
	VectorCopy(pts[1][0], f->planepts[1]);
	VectorCopy(pts[0][0], f->planepts[2]);

	Brush_Build(b);
}

void WINAPI Patch_Rebuild( patchMesh_t *p ) {
	idVec3	vMin, vMax;
	Patch_CalcBounds(p, vMin, vMax);
	Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
	Patch_MakeDirty(p);
}

/*
==================
AddBrushForPatch
==================
 adds a patch brush and ties it to this patch id
*/
brush_t * AddBrushForPatch( patchMesh_t *pm,bool bLinkToWorld ) {
	// find the farthest points in x,y,z
	idVec3	vMin, vMax;
	Patch_CalcBounds(pm, vMin, vMax);

	for ( int j = 0; j < 3; j++ ) {
		if ( idMath::Fabs(vMin[j] - vMax[j]) <= VECTOR_EPSILON ) {
			vMin[j] -= 4;
			vMax[j] += 4;
		}
	}

	texdef_t	td;
	//td.SetName(pm->d_texture->getName());
	brush_t		*b	= Brush_Create(vMin, vMax, &td);
	//brush_t *b = Brush_Create(vMin, vMax, &g_qeglobals.d_texturewin.texdef);

	// FIXME: this entire type of linkage needs to be fixed
	b->pPatch = pm;
	pm->pSymbiot = b;
	pm->bSelected = false;
	pm->bOverlay = false;
	pm->nListID = -1;
	pm->nListIDCam = -1;

	if ( bLinkToWorld ) {
		Brush_AddToList(b, &active_brushes);
		Entity_LinkBrush(world_entity, b);
		Brush_Build(b);
	}

	return b;
}

void Patch_SetPointIntensities( int n ) {
#if 0
		patchMesh_t	*p = patchMeshes[n];
	for (int i = 0; i < p->width; i++) {
		for (int j = 0; j < p->height; j++) {

		}
	}
#endif
}

// very approximate widths and heights

/*
==================
Patch_Width
==================
*/
float Patch_Width( patchMesh_t *p ) {
	float	f	= 0;
	for ( int j = 0; j < p->height - 1; j++ ) {
		float	t	= 0;
		for ( int i = 0; i < p->width - 1; i++ ) {
			idVec3	vTemp;
			vTemp = p->ctrl(i, j).xyz - p->ctrl(i + 1, j).xyz;
			t += vTemp.Length();
		}
		if ( f < t ) {
			f = t;
		}
	}
	return f;
}

/*
==================
Patch_Height
==================
*/
float Patch_Height( patchMesh_t *p ) {
	float	f	= 0;
	for ( int j = 0; j < p->width - 1; j++ ) {
		float	t	= 0;
		for ( int i = 0; i < p->height - 1; i++ ) {
			idVec3	vTemp;
			vTemp = p->ctrl(j, i).xyz - p->ctrl(j, i + 1).xyz;
			t += vTemp.Length();
		}
		if ( f < t ) {
			f = t;
		}
	}
	return f;
}

/*
==================
Patch_WidthDistanceTo
==================
*/
float Patch_WidthDistanceTo( patchMesh_t *p,int j ) {
	float	f	= 0;
	for ( int i = 0; i < j ; i++ ) {
		idVec3	vTemp;
		vTemp = p->ctrl(i, 0).xyz - p->ctrl(i + 1, 0).xyz;
		f += vTemp.Length();
	}
	return f;
}

/*
==================
Patch_HeightDistanceTo
==================
*/
float Patch_HeightDistanceTo( patchMesh_t *p,int j ) {
	float	f	= 0;
	for ( int i = 0; i < j ; i++ ) {
		idVec3	vTemp;
		vTemp = p->ctrl(0, i).xyz - p->ctrl(0, i + 1).xyz;
		f += vTemp.Length();
	}
	return f;
}



/*
==================
Patch_Naturalize
==================
texture = TotalTexture * LengthToThisControlPoint / TotalControlPointLength

dist( this control point to first control point ) / dist ( last control pt to first)
*/
void Patch_Naturalize( patchMesh_t *p,bool horz,bool vert,bool alt ) {
	int		i, j;

	int		nWidth		= p->d_texture->GetEditorImage()->uploadWidth * 0.5;
	int		nHeight		= p->d_texture->GetEditorImage()->uploadHeight * 0.5;
	float	fPWidth		= Patch_Width(p);
	float	fPHeight	= Patch_Height(p);
	float	xAccum		= 0;
	for ( i = 0 ; i < ((alt) ? p->height : p->width) ; i++ ) {
		float	yAccum	= 0;
		for ( j = 0 ; j < ((alt) ? p->width : p->height) ; j++ ) {
			int	r	= ((alt) ? j : i);
			int	c	= ((alt) ? i : j);
			p->ctrl(r, c).st[0] = (fPWidth / nWidth) * xAccum / fPWidth;
			p->ctrl(r, c).st[1] = (fPHeight / nHeight) * yAccum / fPHeight;
			if ( alt ) {
				yAccum = Patch_WidthDistanceTo(p, j + 1);
			} else {
				yAccum = Patch_HeightDistanceTo(p, j + 1);
			}
		}
		if ( alt ) {
			xAccum = Patch_HeightDistanceTo(p, i + 1);
		} else {
			xAccum = Patch_WidthDistanceTo(p, i + 1);
		}
	}

	Patch_MakeDirty(p);
}

/*
  if (bIBevel)
  {
	VectorCopy(p->ctrl(1,0], p->ctrl(1,1]);
  }

  if (bIEndcap)
  {
	VectorCopy(p->ctrl(3,0], p->ctrl(4,1]);
	VectorCopy(p->ctrl(2,0], p->ctrl(3,1]);
	VectorCopy(p->ctrl(2,0], p->ctrl(2,1]);
	VectorCopy(p->ctrl(2,0], p->ctrl(1,1]);
	VectorCopy(p->ctrl(1,0], p->ctrl(0,1]);
	VectorCopy(p->ctrl(1,0], p->ctrl(0,2]);
	VectorCopy(p->ctrl(1,0], p->ctrl(1,2]);
	VectorCopy(p->ctrl(2,0], p->ctrl(2,2]);
	VectorCopy(p->ctrl(3,0], p->ctrl(3,2]);
	VectorCopy(p->ctrl(3,0], p->ctrl(4,2]);
  }
*/

int	Index3By[][2]		= {
	{0,0}, {1,0}, {2,0}, {2,1}, {2,2}, {1,2}, {0,2}, {0,1}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}
};

int	Index5By[][2]		= {
	{0,0}, {1,0}, {2,0}, {3,0}, {4,0}, {4,1}, {4,2}, {4,3}, {4,4}, {3,4}, {2,4}, {1,4}, {0,4}, {0,3}, {0,2}, {0,1}
};



int	Interior3By[][2]	= {
	{1,1}
};

int	Interior5By[][2]	= {
	{1,1}, {2,1}, {3,1}, {1,2}, {2,2}, {3,2}, {1,3}, {2,3}, {3,3}
};

int	Interior3ByCount	= sizeof(Interior3By) / sizeof(int[2]);
int	Interior5ByCount	= sizeof(Interior5By) / sizeof(int[2]);

face_t * Patch_GetAxisFace( patchMesh_t *p ) {
	face_t	*f	= NULL;
	idVec3	vTemp;
	brush_t	*b	= p->pSymbiot;

	for ( f = b->brush_faces ; f ; f = f->next ) {
		vTemp = (*f->face_winding)[1].ToVec3() - (*f->face_winding)[0].ToVec3();
		int	nScore	= 0;

		// default edge faces on caps are 8 high so
		// as soon as we hit one that is bigger it should be on the right axis
		for ( int j = 0; j < 3; j++ ) {
			if ( vTemp[j] > 8 )
				nScore++;
		}

		if ( nScore > 0 ) {
			break;
		}
	}

	if ( f == NULL ) {
		f = b->brush_faces;
	}
	return f;
}

int	g_nFaceCycle	= 0;

face_t * nextFace( patchMesh_t *p ) {
	brush_t	*b	= p->pSymbiot;
	face_t	*f	= NULL;
	int		n	= 0;
	for ( f = b->brush_faces ; f && n <= g_nFaceCycle; f = f->next ) {
		n++;
	}

	g_nFaceCycle++;

	if ( g_nFaceCycle > 5 ) {
		g_nFaceCycle = 0;
		f = b->brush_faces;
	}

	return f;
}


void Patch_CapTexture( patchMesh_t *p,bool bFaceCycle = false,bool alt = false ) {
	Patch_MeshNormals(p);
	face_t	*f	= (bFaceCycle) ? nextFace(p) : Patch_GetAxisFace(p);
	idVec3	vSave;
	VectorCopy(f->plane, vSave);
	float	fRotate	= f->texdef.rotate;
	f->texdef.rotate = 0;
	float	fScale[2];
	fScale[0] = f->texdef.scale[0];
	fScale[1] = f->texdef.scale[1];
	f->texdef.scale[0] = (float) p->d_texture->GetEditorImage()->uploadWidth / 32.0f;
	f->texdef.scale[1] = (float) p->d_texture->GetEditorImage()->uploadHeight / 32.0f;
	float	fShift[2];
	fShift[0] = f->texdef.shift[0];
	fShift[1] = f->texdef.shift[1];
	f->texdef.shift[0] = 0;
	f->texdef.shift[1] = 0;

	for ( int i = 0 ; i < p->width; i++ ) {
		for ( int j = 0 ; j < p->height ; j++ ) {
			if ( !bFaceCycle ) {
				VectorCopy(p->ctrl(i, j).normal, f->plane);
			}
			idVec5	temp;
			temp.x = p->ctrl(i, j).xyz.x;
			temp.y = p->ctrl(i, j).xyz.y;
			temp.z = p->ctrl(i, j).xyz.z;
			EmitTextureCoordinates(temp, f->d_texture, f, true);
			p->ctrl(i, j).st.x = temp.s;
			p->ctrl(i, j).st.y = temp.t;
		}
	}

	VectorCopy(vSave, f->plane);
	f->texdef.rotate = fRotate;
	f->texdef.scale[0] = fScale[0];
	f->texdef.scale[1] = fScale[1];
	f->texdef.shift[0] = fShift[0];
	f->texdef.shift[1] = fShift[1];
	Patch_ScaleTexture(p, 1.0f, -1.0f, false);
	Patch_MakeDirty(p);
}

void FillPatch( patchMesh_t *p,idVec3 v ) {
	for ( int i = 0; i < p->width; i++ ) {
		for ( int j = 0; j < p->height; j++ ) {
			VectorCopy(v, p->ctrl(i, j).xyz);
		}
	}
}

brush_t * Cap( patchMesh_t *pParent,bool bByColumn,bool bFirst ) {
	brush_t		*b;
	patchMesh_t	*p;
	idVec3		vMin, vMax;
	int			i, j;

	bool		bSmall	= true;
	// make a generic patch
	if ( pParent->width <= 9 ) {
		b = Patch_GenericMesh(3, 3, 2, false, false, pParent);
	} else {
		b = Patch_GenericMesh(5, 5, 2, false, false, pParent);
		bSmall = false;
	}

	if ( !b ) {
		Sys_Status("Unable to cap. You may need to ungroup the patch.\n");
		return NULL;
	}

	p = b->pPatch;
	p->type |= PATCH_CAP;

	vMin[0] = vMin[1] = vMin[2] = 99999;
	vMax[0] = vMax[1] = vMax[2] = -99999;

	// we seam the column edge, FIXME: this might need to be able to seem either edge
	// 
	int	nSize	= (bByColumn) ? pParent->width : pParent->height;
	int	nIndex	= (bFirst) ? 0 : (bByColumn) ? pParent->height - 1 : pParent->width - 1;

	FillPatch(p, pParent->ctrl(0, nIndex).xyz);

	for ( i = 0; i < nSize; i++ ) {
		if ( bByColumn ) {
			if ( bSmall ) {
				VectorCopy(pParent->ctrl(i, nIndex).xyz, p->ctrl(Index3By[i][0], Index3By[i][1]).xyz);
			} else {
				VectorCopy(pParent->ctrl(i, nIndex).xyz, p->ctrl(Index5By[i][0], Index5By[i][1]).xyz);
			}
		} else {
			if ( bSmall ) {
				VectorCopy(pParent->ctrl(nIndex, i).xyz, p->ctrl(Index3By[i][0], Index3By[i][1]).xyz);
			} else {
				VectorCopy(pParent->ctrl(nIndex, i).xyz, p->ctrl(Index5By[i][0], Index5By[i][1]).xyz);
			}
		}

		for ( j = 0; j < 3; j++ ) {
			float	f	= (bSmall) ? p->ctrl(Index3By[i][0], Index3By[i][1]).xyz[j] : p->ctrl(Index5By[i][0], Index5By[i][1]).xyz[j];
			if ( f < vMin[j] )
				vMin[j] = f;
			if ( f > vMax[j] )
				vMax[j] = f;
		}
	}

	idVec3	vTemp;
	for ( j = 0; j < 3; j++ ) {
		vTemp[j] = vMin[j] + abs((vMax[j] - vMin[j]) * 0.5);
	}

	int	nCount	= (bSmall) ? Interior3ByCount : Interior5ByCount;
	for ( j = 0; j < nCount; j++ ) {
		if ( bSmall ) {
			VectorCopy(vTemp, p->ctrl(Interior3By[j][0], Interior3By[j][1]).xyz);
		} else {
			VectorCopy(vTemp, p->ctrl(Interior5By[j][0], Interior5By[j][1]).xyz);
		}
	}

	if ( bFirst ) {
		idDrawVert	vertTemp;
		for ( i = 0; i < p->width; i++ ) {
			for ( j = 0; j < p->height / 2; j++ ) {
				memcpy(&vertTemp, &p->ctrl(i, p->height - 1 - j), sizeof(idDrawVert));
				memcpy(&p->ctrl(i, p->height - 1 - j), &p->ctrl(i, j), sizeof(idDrawVert));
				memcpy(&p->ctrl(i, j), &vertTemp, sizeof(idDrawVert));
			}
		}
	}

	Patch_Rebuild(p);
	Patch_CapTexture(p);
	return p->pSymbiot;
}

brush_t * CapSpecial( patchMesh_t *pParent,int nType,bool bFirst ) {
	brush_t		*b;
	patchMesh_t	*p;
	idVec3		vMin, vMax, vTemp;
	int			i, j;

	if ( nType == CCapDialog::IENDCAP ) {
		b = Patch_GenericMesh(5, 3, 2, false, false, pParent);
	} else {
		b = Patch_GenericMesh(3, 3, 2, false, false, pParent);
	}

	if ( !b ) {
		Sys_Status("Unable to cap. Make sure you ungroup before re-capping.");
		return NULL;
	}

	p = b->pPatch;
	p->type |= PATCH_CAP;

	vMin[0] = vMin[1] = vMin[2] = 99999;
	vMax[0] = vMax[1] = vMax[2] = -99999;

	int	nSize	= pParent->width;
	int	nIndex	= (bFirst) ? 0 : pParent->height - 1;

	// parent bounds are used for some things
	Patch_CalcBounds(pParent, vMin, vMax);

	for ( j = 0; j < 3; j++ ) {
		vTemp[j] = vMin[j] + abs((vMax[j] - vMin[j]) * 0.5);
	}

	if ( nType == CCapDialog::IBEVEL ) {
		VectorCopy(pParent->ctrl(0, nIndex).xyz, p->ctrl(0, 0).xyz);
		VectorCopy(pParent->ctrl(2, nIndex).xyz, p->ctrl(0, 2).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(0, 1).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(2, 2).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(1, 0).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(1, 1).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(1, 2).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(2, 0).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(2, 1).xyz);
	} else if ( nType == CCapDialog::BEVEL ) {
		idVec3	p1, p2, p3, p4, temp, dir;

		VectorCopy(pParent->ctrl(0, nIndex).xyz, p3);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p1);
		VectorCopy(pParent->ctrl(2, nIndex).xyz, p2);

		VectorSubtract(p3, p2, dir);
		dir.Normalize();
		VectorSubtract(p1, p2, temp);
		float	dist	= DotProduct(temp, dir);

		VectorScale(dir, dist, temp);

		VectorAdd(p2, temp, temp);

		VectorSubtract(temp, p1, temp);
		VectorScale(temp, 2, temp);
		VectorAdd(p1, temp, p4);

		VectorCopy(p4, p->ctrl(0, 0).xyz);
		VectorCopy(p4, p->ctrl(1, 0).xyz);
		VectorCopy(p4, p->ctrl(0, 1).xyz);
		VectorCopy(p4, p->ctrl(1, 1).xyz);
		VectorCopy(p4, p->ctrl(0, 2).xyz);
		VectorCopy(p4, p->ctrl(1, 2).xyz);
		VectorCopy(p3, p->ctrl(2, 0).xyz);
		VectorCopy(p1, p->ctrl(2, 1).xyz);
		VectorCopy(p2, p->ctrl(2, 2).xyz);
	} else if ( nType == CCapDialog::ENDCAP ) {
		VectorAdd(pParent->ctrl(4, nIndex).xyz, pParent->ctrl(0, nIndex).xyz, vTemp);
		VectorScale(vTemp, 0.5, vTemp);
		VectorCopy(pParent->ctrl(0, nIndex).xyz, p->ctrl(0, 0).xyz);
		VectorCopy(vTemp, p->ctrl(1, 0).xyz);
		VectorCopy(pParent->ctrl(4, nIndex).xyz, p->ctrl(2, 0).xyz);

		VectorCopy(pParent->ctrl(2, nIndex).xyz, p->ctrl(0, 2).xyz);
		VectorCopy(pParent->ctrl(2, nIndex).xyz, p->ctrl(1, 2).xyz);
		VectorCopy(pParent->ctrl(2, nIndex).xyz, p->ctrl(2, 2).xyz);
		VectorCopy(pParent->ctrl(2, nIndex).xyz, p->ctrl(1, 1).xyz);

		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(0, 1).xyz);
		VectorCopy(pParent->ctrl(3, nIndex).xyz, p->ctrl(2, 1).xyz);
	} else {
		VectorCopy(pParent->ctrl(0, nIndex).xyz, p->ctrl(0, 0).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(1, 0).xyz);
		VectorCopy(pParent->ctrl(2, nIndex).xyz, p->ctrl(2, 0).xyz);
		VectorCopy(pParent->ctrl(3, nIndex).xyz, p->ctrl(3, 0).xyz);
		VectorCopy(pParent->ctrl(4, nIndex).xyz, p->ctrl(4, 0).xyz);

		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(0, 1).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(1, 1).xyz);
		VectorCopy(pParent->ctrl(2, nIndex).xyz, p->ctrl(2, 1).xyz);
		VectorCopy(pParent->ctrl(3, nIndex).xyz, p->ctrl(3, 1).xyz);
		VectorCopy(pParent->ctrl(3, nIndex).xyz, p->ctrl(4, 1).xyz);

		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(0, 2).xyz);
		VectorCopy(pParent->ctrl(1, nIndex).xyz, p->ctrl(1, 2).xyz);
		VectorCopy(pParent->ctrl(2, nIndex).xyz, p->ctrl(2, 2).xyz);
		VectorCopy(pParent->ctrl(3, nIndex).xyz, p->ctrl(3, 2).xyz);
		VectorCopy(pParent->ctrl(3, nIndex).xyz, p->ctrl(4, 2).xyz);
	}


	bool	bEndCap	= (nType == CCapDialog::ENDCAP || nType == CCapDialog::IENDCAP);
	if ( (!bFirst && !bEndCap) || (bFirst && bEndCap) ) {
		idDrawVert	vertTemp;
		for ( i = 0; i < p->width; i++ ) {
			for ( j = 0; j < p->height / 2; j++ ) {
				memcpy(&vertTemp, &p->ctrl(i, p->height - 1 - j), sizeof(idDrawVert));
				memcpy(&p->ctrl(i, p->height - 1 - j), &p->ctrl(i, j), sizeof(idDrawVert));
				memcpy(&p->ctrl(i, j), &vertTemp, sizeof(idDrawVert));
			}
		}
	}

	//--Patch_CalcBounds(p, vMin, vMax);
	//--Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
	Patch_Rebuild(p);
	Patch_CapTexture(p);
	return p->pSymbiot;
}


void Patch_CapCurrent( bool bInvertedBevel,bool bInvertedEndcap ) {
	patchMesh_t	*pParent	= NULL;
	brush_t		*b[4];
	brush_t		*pCap		= NULL;
	b[0] = b[1] = b[2] = b[3] = NULL;
	int	nIndex	= 0;

	if ( !QE_SingleBrush() ) {
		Sys_Status("Cannot cap multiple selection. Please select a single patch.\n");
		return;
	}


	for ( brush_t*pb = selected_brushes.next ; pb != NULL && pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			pParent = pb->pPatch;
			// decide which if any ends we are going to cap
			// if any of these compares hit, it is a closed patch and as such
			// the generic capping will work.. if we do not find a closed edge 
			// then we need to ask which kind of cap to add
			if ( pParent->ctrl(0, 0).xyz.Compare(pParent->ctrl(pParent->width - 1, 0).xyz) ) {
				pCap = Cap(pParent, true, false);
				if ( pCap != NULL ) {
					b[nIndex++] = pCap;
				}
			}
			if ( pParent->ctrl(0, pParent->height - 1).xyz.Compare(pParent->ctrl(pParent->width - 1, pParent->height - 1).xyz) ) {
				pCap = Cap(pParent, true, true);
				if ( pCap != NULL ) {
					b[nIndex++] = pCap;
				}
			}
			if ( pParent->ctrl(0, 0).xyz.Compare(pParent->ctrl(0, pParent->height - 1).xyz) ) {
				pCap = Cap(pParent, false, false);
				if ( pCap != NULL ) {
					b[nIndex++] = pCap;
				}
			}
			if ( pParent->ctrl(pParent->width - 1, 0).xyz.Compare(pParent->ctrl(pParent->width - 1, pParent->height - 1).xyz) ) {
				pCap = Cap(pParent, false, true);
				if ( pCap != NULL ) {
					b[nIndex++] = pCap;
				}
			}
		}
	}

	if ( pParent ) {
		// if we did not cap anything with the above tests
		if ( nIndex == 0 ) {
			CCapDialog	dlg;
			if ( dlg.DoModal() == IDOK ) {
				b[nIndex++] = CapSpecial(pParent, dlg.getCapType(), false);
				b[nIndex++] = CapSpecial(pParent, dlg.getCapType(), true);
			}
		}

		if ( nIndex > 0 ) {
			while ( nIndex > 0 ) {
				nIndex--;
				if ( b[nIndex] ) {
					Select_Brush(b[nIndex]);
				}
			}
			eclass_t*pecNew	= Eclass_ForName("func_static", false);
			if ( pecNew ) {
				entity_t*e	= Entity_Create(pecNew);
				SetKeyValue(e, "type", "patchCapped");
			}
		}
	}
}


//FIXME: Table drive all this crap
//
void GenerateEndCaps( brush_t *brushParent,bool bBevel,bool bEndcap,bool bInverted ) {
	brush_t		*b, *b2;
	patchMesh_t	*p, *p2, *pParent;
	idVec3		vTemp, vMin, vMax;
	int			i, j;

	pParent = brushParent->pPatch;

	Patch_CalcBounds(pParent, vMin, vMax);
	// basically generate two endcaps, place them, and link the three brushes with a func_group

	if ( pParent->width > 9 ) {
		b = Patch_GenericMesh(5, 3, 2, false, false, pParent);
	} else {
		b = Patch_GenericMesh(3, 3, 2, false, false, pParent);
	}
	p = b->pPatch;

	vMin[0] = vMin[1] = vMin[2] = 99999;
	vMax[0] = vMax[1] = vMax[2] = -99999;

	for ( i = 0; i < pParent->width; i++ ) {
		VectorCopy(pParent->ctrl(i, 0).xyz, p->ctrl(Index3By[i][0], Index3By[i][1]).xyz);
		for ( j = 0; j < 3; j++ ) {
			if ( pParent->ctrl(i, 0).xyz[j] < vMin[j] )
				vMin[j] = pParent->ctrl(i, 0).xyz[j];
			if ( pParent->ctrl(i, 0).xyz[j] > vMax[j] )
				vMax[j] = pParent->ctrl(i, 0).xyz[j];
		}
	}

	for ( j = 0; j < 3; j++ ) {
		vTemp[j] = vMin[j] + abs((vMax[j] - vMin[j]) * 0.5);
	}

	for ( i = 0; i < Interior3ByCount; i++ ) {
		VectorCopy(vTemp, p->ctrl(Interior3By[i][0], Interior3By[i][1]).xyz);
	}

	Patch_CalcBounds(p, vMin, vMax);
	Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
	Select_Brush(p->pSymbiot);
	return;

	bool	bCreated	= false;

	if ( bInverted ) {
		if ( bBevel ) {
			b = Patch_GenericMesh(3, 3, 2, false, false, pParent);
			p = b->pPatch;
			VectorCopy(p->ctrl(2, 2).xyz, p->ctrl(1, 2).xyz);
			VectorCopy(p->ctrl(2, 2).xyz, p->ctrl(2, 1).xyz);
			VectorCopy(p->ctrl(2, 2).xyz, p->ctrl(0, 1).xyz);
			VectorCopy(p->ctrl(2, 2).xyz, p->ctrl(1, 0).xyz);
			VectorCopy(p->ctrl(2, 2).xyz, p->ctrl(1, 1).xyz);
			VectorCopy(p->ctrl(2, 0).xyz, p->ctrl(0, 0).xyz);

			b2 = Patch_GenericMesh(3, 3, 2, false, false, pParent);
			p2 = b2->pPatch;
			VectorCopy(p2->ctrl(2, 2).xyz, p2->ctrl(1, 2).xyz);
			VectorCopy(p2->ctrl(2, 2).xyz, p2->ctrl(2, 1).xyz);
			VectorCopy(p2->ctrl(2, 2).xyz, p2->ctrl(0, 1).xyz);
			VectorCopy(p2->ctrl(2, 2).xyz, p2->ctrl(1, 0).xyz);
			VectorCopy(p2->ctrl(2, 2).xyz, p2->ctrl(1, 1).xyz);
			VectorCopy(p2->ctrl(2, 0).xyz, p2->ctrl(0, 0).xyz);


			bCreated = true;
		} else if ( bEndcap ) {
			b = Patch_GenericMesh(5, 5, 2, false, false, pParent);
			p = b->pPatch;
			VectorCopy(p->ctrl(4, 4).xyz, p->ctrl(4, 3).xyz);
			VectorCopy(p->ctrl(0, 4).xyz, p->ctrl(1, 4).xyz);
			VectorCopy(p->ctrl(0, 4).xyz, p->ctrl(2, 4).xyz);
			VectorCopy(p->ctrl(0, 4).xyz, p->ctrl(3, 4).xyz);

			VectorCopy(p->ctrl(4, 0).xyz, p->ctrl(4, 1).xyz);
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(1, 0).xyz);
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(2, 0).xyz);
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(3, 0).xyz);

			for ( i = 1; i < 4; i++ ) {
				for ( j = 0; j < 4; j++ ) {
					VectorCopy(p->ctrl(4, i).xyz, p->ctrl(j, i).xyz);
				}
			}


			b2 = Patch_GenericMesh(5, 5, 2, false, false, pParent);
			p2 = b2->pPatch;
			VectorCopy(p2->ctrl(4, 4).xyz, p2->ctrl(4, 3).xyz);
			VectorCopy(p2->ctrl(0, 4).xyz, p2->ctrl(1, 4).xyz);
			VectorCopy(p2->ctrl(0, 4).xyz, p2->ctrl(2, 4).xyz);
			VectorCopy(p2->ctrl(0, 4).xyz, p2->ctrl(3, 4).xyz);

			VectorCopy(p2->ctrl(4, 0).xyz, p2->ctrl(4, 1).xyz);
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(1, 0).xyz);
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(2, 0).xyz);
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(3, 0).xyz);

			for ( i = 1; i < 4; i++ ) {
				for ( j = 0; j < 4; j++ ) {
					VectorCopy(p2->ctrl(4, i).xyz, p2->ctrl(j, i).xyz);
				}
			}


			bCreated = true;
		}
	} else {
		if ( bBevel ) {
			b = Patch_GenericMesh(3, 3, 2, false, false, pParent);
			p = b->pPatch;
			VectorCopy(p->ctrl(2, 0).xyz, p->ctrl(2, 1).xyz);
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(1, 0).xyz);
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(2, 0).xyz);

			b2 = Patch_GenericMesh(3, 3, 2, false, false, pParent);
			p2 = b2->pPatch;
			VectorCopy(p2->ctrl(2, 0).xyz, p2->ctrl(2, 1).xyz);
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(1, 0).xyz);
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(2, 0).xyz);
			bCreated = true;
		} else if ( bEndcap ) {
			b = Patch_GenericMesh(5, 5, 2, false, false, pParent);
			p = b->pPatch;
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(1, 0).xyz);
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(2, 0).xyz);
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(3, 0).xyz);
			VectorCopy(p->ctrl(4, 0).xyz, p->ctrl(4, 1).xyz);
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(4, 0).xyz);

			VectorCopy(p->ctrl(0, 4).xyz, p->ctrl(1, 4).xyz);
			VectorCopy(p->ctrl(0, 4).xyz, p->ctrl(2, 4).xyz);
			VectorCopy(p->ctrl(0, 4).xyz, p->ctrl(3, 4).xyz);
			VectorCopy(p->ctrl(4, 4).xyz, p->ctrl(4, 3).xyz);
			VectorCopy(p->ctrl(0, 4).xyz, p->ctrl(4, 4).xyz);

			b2 = Patch_GenericMesh(5, 5, 2, false, false, pParent);
			p2 = b2->pPatch;
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(1, 0).xyz);
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(2, 0).xyz);
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(3, 0).xyz);
			VectorCopy(p2->ctrl(4, 0).xyz, p2->ctrl(4, 1).xyz);
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(4, 0).xyz);

			VectorCopy(p2->ctrl(0, 4).xyz, p2->ctrl(1, 4).xyz);
			VectorCopy(p2->ctrl(0, 4).xyz, p2->ctrl(2, 4).xyz);
			VectorCopy(p2->ctrl(0, 4).xyz, p2->ctrl(3, 4).xyz);
			VectorCopy(p2->ctrl(4, 4).xyz, p2->ctrl(4, 3).xyz);
			VectorCopy(p2->ctrl(0, 4).xyz, p2->ctrl(4, 4).xyz);
			bCreated = true;
		} else {
			b = Patch_GenericMesh(3, 3, 2, false, false, pParent);
			p = b->pPatch;

			VectorCopy(p->ctrl(0, 1).xyz, vTemp);
			VectorCopy(p->ctrl(0, 2).xyz, p->ctrl(0, 1).xyz);
			VectorCopy(p->ctrl(1, 2).xyz, p->ctrl(0, 2).xyz);
			VectorCopy(p->ctrl(2, 2).xyz, p->ctrl(1, 2).xyz);
			VectorCopy(p->ctrl(2, 1).xyz, p->ctrl(2, 2).xyz);
			VectorCopy(p->ctrl(2, 0).xyz, p->ctrl(2, 1).xyz);
			VectorCopy(p->ctrl(1, 0).xyz, p->ctrl(2, 0).xyz);
			VectorCopy(p->ctrl(0, 0).xyz, p->ctrl(1, 0).xyz);
			VectorCopy(vTemp, p->ctrl(0, 0).xyz);

			b2 = Patch_GenericMesh(3, 3, 2, false, false, pParent);
			p2 = b2->pPatch;
			VectorCopy(p2->ctrl(0, 1).xyz, vTemp);
			VectorCopy(p2->ctrl(0, 2).xyz, p2->ctrl(0, 1).xyz);
			VectorCopy(p2->ctrl(1, 2).xyz, p2->ctrl(0, 2).xyz);
			VectorCopy(p2->ctrl(2, 2).xyz, p2->ctrl(1, 2).xyz);
			VectorCopy(p2->ctrl(2, 1).xyz, p2->ctrl(2, 2).xyz);
			VectorCopy(p2->ctrl(2, 0).xyz, p2->ctrl(2, 1).xyz);
			VectorCopy(p2->ctrl(1, 0).xyz, p2->ctrl(2, 0).xyz);
			VectorCopy(p2->ctrl(0, 0).xyz, p2->ctrl(1, 0).xyz);
			VectorCopy(vTemp, p2->ctrl(0, 0).xyz);
			bCreated = true;
		}
	}

	if ( bCreated ) {
		idDrawVert	vertTemp;
		for ( i = 0; i < p->width; i++ ) {
			for ( j = 0; j < p->height; j++ ) {
				p->ctrl(i, j).xyz[2] = vMin[2];
				p2->ctrl(i, j).xyz[2] = vMax[2];
			}

			for ( j = 0; j < p->height / 2; j++ ) {
				memcpy(&vertTemp, &p->ctrl(i, p->height - 1 - j), sizeof(idDrawVert));
				memcpy(&p->ctrl(i, p->height - 1 - j), &p->ctrl(i, j), sizeof(idDrawVert));
				memcpy(&p->ctrl(i, j), &vertTemp, sizeof(idDrawVert));
			}
		}
		//Select_Delete();

		Patch_CalcBounds(p, vMin, vMax);
		Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
		Patch_CalcBounds(p2, vMin, vMax);
		Brush_RebuildBrush(p2->pSymbiot, vMin, vMax);
		Select_Brush(p->pSymbiot);
		Select_Brush(p2->pSymbiot);
	} else {
		Select_Delete();
	}
	//Select_Brush(brushParent);

}


/*
===============
BrushToPatchMesh
===============
*/
void Patch_BrushToMesh( bool bCone,bool bBevel,bool bEndcap,bool bSquare,int nHeight ) {
	brush_t		*b;
	patchMesh_t	*p;
	int			i, j;

	int			width	= 9;
	if ( bBevel & !bSquare ) {
		width = 3;
	} else if ( bEndcap & !bSquare ) {
		width = 5;
	}

	if ( !QE_SingleBrush() ) {
		return;
	}

	b = selected_brushes.next;

	p = MakeNewPatch(width, nHeight);

	p->d_texture = b->brush_faces->d_texture;

	p->type = PATCH_CYLINDER;
	if ( bBevel & !bSquare ) {
		p->type = PATCH_BEVEL;
		int	nStep	= (b->maxs[2] - b->mins[2]) / (p->height - 1);
		int	nStart	= b->mins[2];
		for ( i = 0; i < p->height; i++ ) {
			p->ctrl(0, i).xyz[0] = b->mins[0];
			p->ctrl(0, i).xyz[1] = b->mins[1];
			p->ctrl(0, i).xyz[2] = nStart;

			p->ctrl(1, i).xyz[0] = b->maxs[0];
			p->ctrl(1, i).xyz[1] = b->mins[1];
			p->ctrl(1, i).xyz[2] = nStart;

			p->ctrl(2, i).xyz[0] = b->maxs[0];
			p->ctrl(2, i).xyz[1] = b->maxs[1];
			p->ctrl(2, i).xyz[2] = nStart;
			nStart += nStep;
		}
	} else if ( bEndcap & !bSquare ) {
		p->type = PATCH_ENDCAP;
		int	nStep	= (b->maxs[2] - b->mins[2]) / (p->height - 1);
		int	nStart	= b->mins[2];
		for ( i = 0; i < p->height; i++ ) {
			p->ctrl(0, i).xyz[0] = b->mins[0];
			p->ctrl(0, i).xyz[1] = b->mins[1];
			p->ctrl(0, i).xyz[2] = nStart;

			p->ctrl(1, i).xyz[0] = b->mins[0];
			p->ctrl(1, i).xyz[1] = b->maxs[1];
			p->ctrl(1, i).xyz[2] = nStart;

			p->ctrl(2, i).xyz[0] = b->mins[0] + ((b->maxs[0] - b->mins[0]) * 0.5);
			p->ctrl(2, i).xyz[1] = b->maxs[1];
			p->ctrl(2, i).xyz[2] = nStart;

			p->ctrl(3, i).xyz[0] = b->maxs[0];
			p->ctrl(3, i).xyz[1] = b->maxs[1];
			p->ctrl(3, i).xyz[2] = nStart;

			p->ctrl(4, i).xyz[0] = b->maxs[0];
			p->ctrl(4, i).xyz[1] = b->mins[1];
			p->ctrl(4, i).xyz[2] = nStart;
			nStart += nStep;
		}
	} else {
		p->ctrl(1, 0).xyz[0] = b->mins[0];
		p->ctrl(1, 0).xyz[1] = b->mins[1];

		p->ctrl(3, 0).xyz[0] = b->maxs[0];
		p->ctrl(3, 0).xyz[1] = b->mins[1];

		p->ctrl(5, 0).xyz[0] = b->maxs[0];
		p->ctrl(5, 0).xyz[1] = b->maxs[1];

		p->ctrl(7, 0).xyz[0] = b->mins[0];
		p->ctrl(7, 0).xyz[1] = b->maxs[1];

		for ( i = 1 ; i < p->width - 1 ; i += 2 ) {
			p->ctrl(i, 0).xyz[2] = b->mins[2];

			VectorCopy(p->ctrl(i, 0).xyz, p->ctrl(i, 2).xyz);

			p->ctrl(i, 2).xyz[2] = b->maxs[2];

			p->ctrl(i, 1).xyz[0] = (p->ctrl(i, 0).xyz[0] + p->ctrl(i, 2).xyz[0]) * 0.5;
			p->ctrl(i, 1).xyz[1] = (p->ctrl(i, 0).xyz[1] + p->ctrl(i, 2).xyz[1]) * 0.5;
			p->ctrl(i, 1).xyz[2] = (p->ctrl(i, 0).xyz[2] + p->ctrl(i, 2).xyz[2]) * 0.5;
		}
		InterpolateInteriorPoints(p);

		if ( bSquare ) {
			if ( bBevel || bEndcap ) {
				if ( bBevel ) {
					for ( i = 0; i < p->height; i++ ) {
						VectorCopy(p->ctrl(1, i).xyz, p->ctrl(2, i).xyz);
						VectorCopy(p->ctrl(7, i).xyz, p->ctrl(6, i).xyz);
					}
				} else {
					for ( i = 0; i < p->height; i++ ) {
						VectorCopy(p->ctrl(5, i).xyz, p->ctrl(4, i).xyz);
						VectorCopy(p->ctrl(1, i).xyz, p->ctrl(2, i).xyz);
						VectorCopy(p->ctrl(7, i).xyz, p->ctrl(6, i).xyz);
						VectorCopy(p->ctrl(8, i).xyz, p->ctrl(7, i).xyz);
					}
				}
			} else {
				for ( i = 0; i < p->width - 1; i ++ ) {
					for ( j = 0; j < p->height; j++ ) {
						VectorCopy(p->ctrl(i + 1, j).xyz, p->ctrl(i, j).xyz);
					}
				}
				for ( j = 0; j < p->height; j++ ) {
					VectorCopy(p->ctrl(0, j).xyz, p->ctrl(8, j).xyz);
				}
			}
		}
	}


	Patch_Naturalize(p);

	if ( bCone ) {
		p->type = PATCH_CONE;
		float	xc	= (b->maxs[0] + b->mins[0]) * 0.5; 
		float	yc	= (b->maxs[1] + b->mins[1]) * 0.5; 

		for ( i = 0 ; i < p->width ; i ++ ) {
			p->ctrl(i, 2).xyz[0] = xc;
			p->ctrl(i, 2).xyz[1] = yc;
		}
	}
	b = AddBrushForPatch(p);

	Select_Delete();
	Select_Brush(b);
}

patchMesh_t * Patch_GenerateGeneric( int width,int height,int orientation,const idVec3 &mins,const idVec3 &maxs ) {
	patchMesh_t	*p	= MakeNewPatch(width, height);
	p->d_texture = Texture_ForName(g_qeglobals.d_texturewin.texdef.name);

	p->type = PATCH_GENERIC;

	int	nFirst	= 0;
	int	nSecond	= 1;
	if ( orientation == 0 ) {
		nFirst = 1;
		nSecond = 2;
	} else if ( orientation == 1 ) {
		nSecond = 2;
	}


	int		xStep	= mins[nFirst];
	float	xAdj	= abs((maxs[nFirst] - mins[nFirst]) / (width - 1));
	float	yAdj	= abs((maxs[nSecond] - mins[nSecond]) / (height - 1));

	for ( int i = 0; i < width; i++ ) {
		int	yStep	= mins[nSecond];
		for ( int j = 0; j < height; j++ ) {
			p->ctrl(i, j).xyz[nFirst] = xStep;
			p->ctrl(i, j).xyz[nSecond] = yStep;
			p->ctrl(i, j).xyz[orientation] = g_qeglobals.d_new_brush_bottom[orientation];
			yStep += yAdj;
		}
		xStep += xAdj;
	}

	return p;
}

/*
==================
Patch_GenericMesh
==================
*/
brush_t * Patch_GenericMesh( int width,int height,int orientation,bool bDeleteSource,bool bOverride,patchMesh_t *parent ) {
	if ( height < 3 || height> 15 || width < 3 || width> 15 ) {
		Sys_Status("Invalid patch width or height.\n");
		return NULL;
	}

	if ( !bOverride && !QE_SingleBrush() ) {
		Sys_Status("Cannot generate a patch from multiple selections.\n");
		return NULL;
	}

	brush_t		*b	= selected_brushes.next;

	patchMesh_t	*p	= Patch_GenerateGeneric(width, height, orientation, b->mins, b->maxs);

	if ( parent ) {
		p->explicitSubdivisions = parent->explicitSubdivisions;
		p->horzSubdivisions = parent->horzSubdivisions;
		p->vertSubdivisions = parent->vertSubdivisions;
	}

	Patch_Naturalize(p);

	b = AddBrushForPatch(p);

	if ( bDeleteSource ) {
		Select_Delete();
		Select_Brush(b);
	}

	return b;
}

/*
==================
PointInMoveList
==================
*/
int PointInMoveList( idVec3 *pf ) {
	for ( int i = 0; i < g_qeglobals.d_num_move_points; i++ ) {
		if ( pf == g_qeglobals.d_move_points[i] ) {
			return i;
		}
	}
	return -1;
}

/*
==================
PointValueInMoveList
==================
*/
static int PointValueInMoveList( idVec3 v ) {
	for ( int i = 0; i < g_qeglobals.d_num_move_points; i++ ) {
		if ( v.Compare(*g_qeglobals.d_move_points[i]) ) {
			return i;
		}
	}
	return -1;
}

/*
==================
RemovePointFromMoveList
==================
*/
void RemovePointFromMoveList( idVec3 v ) {
	int	n;
	while ( (n = PointValueInMoveList(v)) >= 0 ) {
		for ( int i = n; i < g_qeglobals.d_num_move_points - 1; i++ ) {
			g_qeglobals.d_move_points[i] = g_qeglobals.d_move_points[i + 1];
		}
		g_qeglobals.d_num_move_points--;
	}
}

/*
==================
ColumnSelected
==================
*/
bool ColumnSelected( patchMesh_t *p,int nCol ) {
	for ( int i = 0; i < p->height; i++ ) {
		if ( PointInMoveList(&p->ctrl(nCol, i).xyz) == -1 ) {
			return false;
		}
	}
	return true;
}


/*
==================
AddPoint
==================
*/
static void AddPoint( patchMesh_t *p,idVec3 *v,bool bWeldOrDrill = true ) {
	int	nDim1	= (g_pParentWnd->ActiveXY()->GetViewType() == YZ) ? 1 : 0;
	int	nDim2	= (g_pParentWnd->ActiveXY()->GetViewType() == XY) ? 1 : 2;
	g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = v;
	if ( (g_bPatchWeld || g_bPatchDrillDown) && bWeldOrDrill ) {
		for ( int i = 0 ; i < p->width ; i++ ) {
			for ( int j = 0 ; j < p->height ; j++ ) {
				if ( g_bPatchWeld ) {
					if ( (*v).Compare(p->ctrl(i, j).xyz) && PointInMoveList(&p->ctrl(i, j).xyz) == -1 ) {
						g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = &p->ctrl(i, j).xyz;
						continue;
					}
				}
				if ( g_bPatchDrillDown && g_nPatchClickedView != W_CAMERA ) {
					if ( (idMath::Fabs((*v)[nDim1] - p->ctrl(i, j).xyz[nDim1]) <= VECTOR_EPSILON) && (idMath::Fabs((*v)[nDim2] - p->ctrl(i, j).xyz[nDim2]) <= VECTOR_EPSILON) ) {
						if ( PointInMoveList(&p->ctrl(i, j).xyz) == -1 ) {
							g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = &p->ctrl(i, j).xyz;
							continue;
						}
					}
#if 0
		  int l = 0;
					for ( int k = 0; k < 2; k++ ) {
						if (idMath::Fabs(v[k] - p->ctrl(i,j).xyz[k]) > VECTOR_EPSILON)
						continue;
						l++;
					}
					if (l >= 2 && PointInMoveList(&p->ctrl(i,j).xyz) == -1) {
						g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = p->ctrl(i,j).xyz;
						continue;
					}
#endif
				}
			}
		}
	}
#if 0
		if (g_qeglobals.d_num_move_points == 1) {
		// single point selected
		// FIXME: the two loops can probably be reduced to one
		for ( int i = 0 ; i < p->width ; i++ ) {
			for ( int j = 0 ; j < p->height ; j++ ) {
				int n = PointInMoveList(v);
				if (n >= 0) {
					if (((i & 0x01) && (j & 0x01)) == 0) {
						// put any sibling fixed points
						// into the inverse list
						int p1, p2, p3, p4;
						p1 = i + 2;
						p2 = i - 2;
						p3 = j + 2;
						p4 = j - 2;
						if (p1 < p->width) {

						}
						if (p2 >= 0) {
						}
						if (p3 < p->height) {
						}
						if (p4 >= 0) {
						}
					}
				}
			}
		}
	}
#endif
}

/*
==================
SelectRow
==================
*/
void SelectRow( patchMesh_t *p,int nRow,bool bMulti ) {
	if ( !bMulti ) {
		g_qeglobals.d_num_move_points = 0;
	}
	for ( int i = 0; i < p->width; i++ ) {
		AddPoint(p, &p->ctrl(i, nRow).xyz, false);
	}
	//common->Printf("Selected Row %d\n", nRow);
}

/*
==================
SelectColumn
==================
*/
void SelectColumn( patchMesh_t *p,int nCol,bool bMulti ) {
	if ( !bMulti ) {
		g_qeglobals.d_num_move_points = 0;
	}
	for ( int i = 0; i < p->height; i++ ) {
		AddPoint(p, &p->ctrl(nCol, i).xyz, false);
	}
	//common->Printf("Selected Col %d\n", nCol);
}


/*
==================
AddPatchMovePoint
==================
*/
void AddPatchMovePoint( idVec3 v,bool bMulti,bool bFull ) {
	if ( !g_bSameView && !bMulti && !bFull ) {
		g_bSameView = true;
		return;
	}

	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;
			for ( int i = 0 ; i < p->width ; i++ ) {
				for ( int j = 0 ; j < p->height ; j++ ) {
					if ( v.Compare(p->ctrl(i, j).xyz) ) {
						if ( PointInMoveList(&p->ctrl(i, j).xyz) == -1 ) {
							if ( bFull )		// if we want the full row/col this is on
							{
								SelectColumn(p, i, bMulti);
							} else {
								if ( !bMulti )
									g_qeglobals.d_num_move_points = 0;
								AddPoint(p, &p->ctrl(i, j).xyz);
								//common->Printf("Selected col:row %d:%d\n", i, j);
							}
							//--if (!bMulti)
							return;
						} else {
							if ( bFull ) {
								if ( ColumnSelected(p, i) ) {
									SelectRow(p, j, bMulti);
								} else {
									SelectColumn(p, i, bMulti);
								}
								return;
							}
							if ( g_bSameView ) {
								RemovePointFromMoveList(v);
								return;
							}
						}
					}
				}
			}
		}
	}
}
/*
==================
Patch_UpdateSelected
==================
*/
void Patch_UpdateSelected( idVec3 vMove ) {
	int	i, j;
	for ( i = 0 ; i < g_qeglobals.d_num_move_points ; i++ ) {
		VectorAdd(*g_qeglobals.d_move_points[i], vMove, *g_qeglobals.d_move_points[i]);
		if ( g_qeglobals.d_num_move_points == 1 ) {
		}
	}

	//--patchMesh_t* p = &patchMeshes[g_nSelectedPatch];
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;


			g_qeglobals.d_numpoints = 0;
			for ( i = 0 ; i < p->width ; i++ ) {
				for ( j = 0 ; j < p->height ; j++ ) {
					VectorCopy(p->ctrl(i, j).xyz, g_qeglobals.d_points[g_qeglobals.d_numpoints]);
					if ( g_qeglobals.d_numpoints < MAX_POINTS - 1 ) {
						g_qeglobals.d_numpoints++;
					}
				}
			}

			idVec3	vMin, vMax;
			Patch_CalcBounds(p, vMin, vMax);
			Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
		}
	}
	//Brush_Free(p->pSymbiot);
	//Select_Brush(AddBrushForPatch(g_nSelectedPatch));
}


void Patch_AdjustSubdivisions( float hadj,float vadj ) {
	brush_t	*pb;
	for ( pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;
			p->horzSubdivisions += hadj;
			p->vertSubdivisions += vadj;
			Patch_MakeDirty(p);
		}
	}
	Sys_UpdateWindows(W_ALL);
}

extern float ShadeForNormal( idVec3 normal );

/*
=================
DrawPatchMesh
=================
*/
//FIXME: this routine needs to be reorganized.. should be about 1/4 the size and complexity
void DrawPatchMesh( patchMesh_t *pm,bool bPoints,int *list,bool bShade = false ) {
	int		i, j;

	bool	bOverlay	= pm->bOverlay;
	int		nDrawMode	= g_pParentWnd->GetCamera()->Camera().draw_mode;

	// patches use two display lists, one for camera one for xy
	if ( *list <= 0 ) {
		if ( *list <= 0 ) {
			*list = qglGenLists(1);
		}

		if ( *list > 0 ) {
			qglNewList(*list, GL_COMPILE_AND_EXECUTE);
		}

		//FIXME: finish consolidating all the patch crap
		idSurface_Patch	*cp	= new idSurface_Patch(pm->width * 6, pm->height * 6);
		cp->SetSize(pm->width, pm->height);
		for ( i = 0; i < pm->width; i++ ) {
			for ( j = 0; j < pm->height; j++ ) {
				(*cp)[j * cp->GetWidth() + i].xyz = pm->ctrl(i, j).xyz;
				(*cp)[j * cp->GetWidth() + i].st = pm->ctrl(i, j).st;
			}
		}

		if ( pm->explicitSubdivisions ) {
			cp->SubdivideExplicit(pm->horzSubdivisions, pm->vertSubdivisions, true);
		} else {
			cp->Subdivide(DEFAULT_CURVE_MAX_ERROR, DEFAULT_CURVE_MAX_ERROR, DEFAULT_CURVE_MAX_LENGTH, true);
		}


		int			width	= cp->GetWidth();
		int			height	= cp->GetHeight();
		/*
					for (i = 0; i < width; i++) {
						for (j = 0; j < height; j++) {
							qglBegin(GL_POINTS);
							int index = j * width + i;
							qglVertex3fv((*cp)[index].xyz);
							qglEnd();
							char msg[64];
							sprintf(msg, "(%0.3f, %0.3f, %0.3f)(%0.3f, %0.3f)", (*cp)[index].xyz.x, (*cp)[index].xyz.y, (*cp)[index].xyz.z, (*cp)[index].st.x, (*cp)[index].st.y);
							qglRasterPos3f((*cp)[index].xyz.x + 1, (*cp)[index].xyz.y + 1, (*cp)[index].xyz.z + 1);
							qglCallLists (strlen(msg), GL_UNSIGNED_BYTE, msg);
						}
					}
			*/
#ifdef TEST_SURFACE_CLIPPING
		int			n;
		idSurface	*surf	= cp;
		idSurface	*front, *back;

		surf->Split(idPlane(1, 0, 0, 0), 0.1f, &front, &back);
		if ( front && back ) {
			front->TranslateSelf(idVec3(10, 10, 10));
			(*front) += (*back);
			surf = front;
		} else {
			surf = cp;
		}
		//		surf->ClipInPlace( idPlane( 1, 0, 0, 0 ), 0.1f, true );

		qglBegin(GL_TRIANGLES);
		for ( i = 0; i < surf->GetNumIndexes(); i += 3 ) {
			n = surf->GetIndexes()[i + 0];
			qglTexCoord2fv((*surf)[n].st.ToFloatPtr());
			qglVertex3fv((*surf)[n].xyz.ToFloatPtr());
			n = surf->GetIndexes()[i + 1];
			qglTexCoord2fv((*surf)[n].st.ToFloatPtr());
			qglVertex3fv((*surf)[n].xyz.ToFloatPtr());
			n = surf->GetIndexes()[i + 2];
			qglTexCoord2fv((*surf)[n].st.ToFloatPtr());
			qglVertex3fv((*surf)[n].xyz.ToFloatPtr());
		}
		qglEnd();

		if ( front ) {
			delete front;
		}
		if ( back ) {
			delete back;
		}
#else
		for ( i = 0 ; i < width - 1; i++ ) {
			qglBegin(GL_QUAD_STRIP);
			for ( j = 0 ; j < height; j++ ) {
				// v1-v2-v3-v4 makes a quad
				int		v1, v2;
				float	f;
				v1 = j * width + i;
				v2 = v1 + 1;
				if ( bShade ) {
					f = ShadeForNormal((*cp)[v2].normal);
					qglColor3f(f, f, f);
				}
				qglTexCoord2fv((*cp)[v2].st.ToFloatPtr());
				qglVertex3fv((*cp)[v2].xyz.ToFloatPtr());
				if ( bShade ) {
					f = ShadeForNormal((*cp)[v1].normal);
					qglColor3f(f, f, f);
				}
				qglTexCoord2fv((*cp)[v1].st.ToFloatPtr());
				qglVertex3fv((*cp)[v1].xyz.ToFloatPtr());
			}
			qglEnd();
		}
#endif

		if ( list == &pm->nListSelected ) {
			globalImages->BindNull();
			qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			qglColor3f(1.0f, 1.0f, 1.0f);
			for ( i = 0 ; i < width - 1; i++ ) {
				qglBegin(GL_QUAD_STRIP);
				for ( j = 0 ; j < height; j++ ) {
					int	v1, v2;
					v1 = j * width + i;
					v2 = v1 + 1;
					qglVertex3fv((*cp)[v2].xyz.ToFloatPtr());
					qglVertex3fv((*cp)[v1].xyz.ToFloatPtr());
				}
				qglEnd();
			}
		}

		delete cp;

		if ( *list > 0 ) {
			qglEndList();
		}
	} else {
		qglCallList(*list);
	}

	idVec3	*pSelectedPoints[256];
	int		nIndex	= 0;

	// FIXME: this bend painting code needs to be rolled up significantly as it is a mess right now
	if ( bPoints && (g_qeglobals.d_select_mode == sel_curvepoint || g_qeglobals.d_select_mode == sel_area || g_bPatchBendMode || g_bPatchInsertMode) ) {
		bOverlay = false;

		// bending or inserting
		if ( g_bPatchBendMode || g_bPatchInsertMode ) {
			qglPointSize(6);
			if ( g_bPatchAxisOnRow ) {
				qglColor3f(1, 0, 1);
				qglBegin(GL_POINTS);
				for ( i = 0; i < pm->width; i++ ) {
					qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(i, g_nPatchAxisIndex).xyz));
				}
				qglEnd();

				// could do all of this in one loop but it was pretty messy
				if ( g_bPatchInsertMode ) {
					qglColor3f(0, 0, 1);
					qglBegin(GL_POINTS);
					for ( i = 0; i < pm->width; i++ ) {
						qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(i, g_nPatchAxisIndex).xyz));
						qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(i, g_nPatchAxisIndex + 1).xyz));
					}
					qglEnd();
				} else {
					if ( g_nPatchBendState == BEND_SELECT_EDGE || g_nPatchBendState == BEND_BENDIT || g_nPatchBendState == BEND_SELECT_ORIGIN ) {
						qglColor3f(0, 0, 1);
						qglBegin(GL_POINTS);
						if ( g_nPatchBendState == BEND_SELECT_ORIGIN ) {
							qglVertex3fv(g_vBendOrigin.ToFloatPtr());
						} else {
							for ( i = 0; i < pm->width; i++ ) {
								if ( g_bPatchLowerEdge ) {
									for ( j = 0; j < g_nPatchAxisIndex; j++ ) {
										qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(i, j).xyz));
									}
								} else {
									for ( j = pm->height - 1; j > g_nPatchAxisIndex; j-- ) {
										qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(i, j).xyz));
									}
								}
							}
						}
						qglEnd();
					}
				}
			} else {
				qglColor3f(1, 0, 1);
				qglBegin(GL_POINTS);
				for ( i = 0; i < pm->height; i++ ) {
					qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(g_nPatchAxisIndex, i).xyz));
				}
				qglEnd();

				// could do all of this in one loop but it was pretty messy
				if ( g_bPatchInsertMode ) {
					qglColor3f(0, 0, 1);
					qglBegin(GL_POINTS);
					for ( i = 0; i < pm->height; i++ ) {
						qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(g_nPatchAxisIndex, i).xyz));
						qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(g_nPatchAxisIndex + 1, i).xyz));
					}
					qglEnd();
				} else {
					if ( g_nPatchBendState == BEND_SELECT_EDGE || g_nPatchBendState == BEND_BENDIT || g_nPatchBendState == BEND_SELECT_ORIGIN ) {
						qglColor3f(0, 0, 1);
						qglBegin(GL_POINTS);
						for ( i = 0; i < pm->height; i++ ) {
							if ( g_nPatchBendState == BEND_SELECT_ORIGIN ) {
								qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(g_nBendOriginIndex, i).xyz));
							} else {
								if ( g_bPatchLowerEdge ) {
									for ( j = 0; j < g_nPatchAxisIndex; j++ ) {
										qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(j, i).xyz));
									}
								} else {
									for ( j = pm->width - 1; j > g_nPatchAxisIndex; j-- ) {
										qglVertex3fv(reinterpret_cast< float(*)>(&pm->ctrl(j, i).xyz));
									}
								}
							}
						}
						qglEnd();
					}
				}
			}
		} else {
			qglPointSize(6);
			for ( i = 0 ; i < pm->width ; i++ ) {
				for ( j = 0 ; j < pm->height ; j++ ) {
					qglBegin(GL_POINTS);
					// FIXME: need to not do loop lookups inside here
					int	n	= PointValueInMoveList(pm->ctrl(i, j).xyz);
					if ( n >= 0 ) {
						pSelectedPoints[nIndex++] = &pm->ctrl(i, j).xyz;
					}

					if ( i & 0x01 || j & 0x01 ) {
						qglColor3f(1, 0, 1);
					} else {
						qglColor3f(0, 1, 0);
					}
					qglVertex3fv(pm->ctrl(i, j).xyz.ToFloatPtr());
					qglEnd();
				}
			}
		}

		if ( nIndex > 0 ) {
			qglBegin(GL_POINTS);
			qglColor3f(0, 0, 1);
			while ( nIndex-- > 0 ) {
				qglVertex3fv((*pSelectedPoints[nIndex]).ToFloatPtr());
			}
			qglEnd();
		}
	}
	if ( bOverlay ) {
		qglPointSize(6);
		qglColor3f(0.5, 0.5, 0.5);
		for ( i = 0 ; i < pm->width ; i++ ) {
			qglBegin(GL_POINTS);
			for ( j = 0 ; j < pm->height ; j++ ) {
				if ( i & 0x01 || j & 0x01 ) {
					qglColor3f(0.5, 0, 0.5);
				} else {
					qglColor3f(0, 0.5, 0);
				}
				qglVertex3fv(pm->ctrl(i, j).xyz.ToFloatPtr());
			}
			qglEnd();
		}
	}
}

/*
==================
Patch_DrawXY
==================
*/
void Patch_DrawXY( patchMesh_t *pm ) {
	qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if ( pm->bSelected ) {
		qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES].ToFloatPtr());
		//qglDisable (GL_LINE_STIPPLE);
		//qglLineWidth (1);
	} else {
		qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES].ToFloatPtr());
	}

	DrawPatchMesh(pm, pm->bSelected, &pm->nListID);
	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if ( pm->bSelected ) {
		//qglLineWidth (2);
		//qglEnable (GL_LINE_STIPPLE);
	}
}

/*
==================
Patch_DrawCam
==================
*/
void Patch_DrawCam( patchMesh_t *pm,bool selected ) {

	int nDrawMode = g_pParentWnd->GetCamera()->Camera().draw_mode;

	if ( !selected ) {
		qglColor3f(1, 1, 1);
	}

	if ( g_bPatchWireFrame || nDrawMode == cd_wire ) {
		qglDisable(GL_CULL_FACE);
		qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		globalImages->BindNull();
		DrawPatchMesh(pm, pm->bSelected, &pm->nListIDCam, true);
		qglEnable(GL_CULL_FACE);
	} else {
		qglEnable(GL_CULL_FACE);
		qglCullFace(GL_FRONT);
		qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if ( nDrawMode == cd_texture || nDrawMode == cd_light ) {
			pm->d_texture->GetEditorImage()->Bind();
		}

		if ( !selected && pm->d_texture->GetEditorAlpha() != 1.0f ) {
			qglEnable(GL_BLEND);
			qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		DrawPatchMesh(pm, pm->bSelected, &pm->nListIDCam, true);

		if ( !selected && pm->d_texture->GetEditorAlpha() != 1.0f ) {
			qglDisable(GL_BLEND);
		}

		globalImages->BindNull();

		if ( !selected ) {
			qglCullFace(GL_BACK);
			qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			qglDisable(GL_BLEND);
		} else {
			qglEnable(GL_BLEND);
			qglColor4f(g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0], g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1], g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2], 0.25);
			qglDisable(GL_CULL_FACE);
		}
		DrawPatchMesh(pm, pm->bSelected, (selected) ? &pm->nListSelected : &pm->nListIDCam, !selected);
		qglEnable(GL_CULL_FACE);
	}

#if 0 // this paints normal indicators on the ctrl points
		//--qglDisable (GL_DEPTH_TEST);
	qglColor3f (1,1,1);
	for (int i = 0; i < pm->width; i++) {
		for (int j = 0; j < pm->height; j++) {
			idVec3 temp;
			qglBegin (GL_LINES);
			qglVertex3fv (pm->ctrl(i,j).xyz);
			VectorMA (pm->ctrl(i,j).xyz, 8, pm->ctrl(i,j].normal, temp);
			qglVertex3fv (temp);
			qglEnd ();
		}
	}
	//--qglEnable (GL_DEPTH_TEST);
#endif

}




void ConvexHullForSection( float section[2][4][7] ) {
}

void BrushesForSection( float section[2][4][7] ) {
}


/*
==================
Patch_Move
==================
*/
void Patch_Move( patchMesh_t *pm,const idVec3 vMove,bool bRebuild ) {
	Patch_MakeDirty(pm);
	for ( int w = 0; w < pm->width; w++ ) {
		for ( int h = 0; h < pm->height; h++ ) {
			VectorAdd(pm->ctrl(w, h).xyz, vMove, pm->ctrl(w, h).xyz);
		}
	}
	if ( bRebuild ) {
		idVec3	vMin, vMax;
		Patch_CalcBounds(pm, vMin, vMax);
		//Brush_RebuildBrush(patchMeshes[n].pSymbiot, vMin, vMax);
	}
	UpdatePatchInspector();
}

/*
==================
Patch_ApplyMatrix
==================
*/
void Patch_ApplyMatrix( patchMesh_t *p,const idVec3 vOrigin,const idMat3 matrix,bool bSnap ) {
	idVec3	vTemp;

	for ( int w = 0; w < p->width; w++ ) {
		for ( int h = 0; h < p->height; h++ ) {
			if ( (g_qeglobals.d_select_mode == sel_curvepoint || g_bPatchBendMode) && PointInMoveList(&p->ctrl(w, h).xyz) == -1 ) {
				continue;
			}
			vTemp = p->ctrl(w, h).xyz - vOrigin;
			vTemp *= matrix;
			p->ctrl(w, h).xyz = vTemp + vOrigin;
		}
	}
	idVec3	vMin, vMax;
	Patch_CalcBounds(p, vMin, vMax);
	Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
}

/*
==================
Patch_EditPatch
==================
*/
void Patch_EditPatch() {
	//--patchMesh_t* p = &patchMeshes[n];
	g_qeglobals.d_numpoints = 0;
	g_qeglobals.d_num_move_points = 0;

	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;
			for ( int i = 0 ; i < p->width ; i++ ) {
				for ( int j = 0 ; j < p->height ; j++ ) {
					VectorCopy(p->ctrl(i, j).xyz, g_qeglobals.d_points[g_qeglobals.d_numpoints]);
					if ( g_qeglobals.d_numpoints < MAX_POINTS - 1 ) {
						g_qeglobals.d_numpoints++;
					}
				}
			}
		}
	}
	g_qeglobals.d_select_mode = sel_curvepoint;
	//--g_nSelectedPatch = n;
}



/*
==================
Patch_Deselect
==================
*/
//FIXME: need all sorts of asserts throughout a lot of this crap
void Patch_Deselect() {
	//--g_nSelectedPatch = -1;
	g_qeglobals.d_select_mode = sel_brush;

	for ( brush_t*b = selected_brushes.next ; b != &selected_brushes ; b = b->next ) {
		if ( b->pPatch ) {
			b->pPatch->bSelected = false;
		}
	}

	if ( g_bPatchBendMode ) {
		Patch_BendToggle();
	}

	if ( g_bPatchInsertMode ) {
		Patch_InsDelToggle();
	}
}


/*
==================
Patch_Select
==================
*/
void Patch_Select( patchMesh_t *p ) {
	// maintained for point manip.. which i need to fix as this 
	// is pf error prone
	//--g_nSelectedPatch = n;
	p->bSelected = true;
}


/*
==================
Patch_Deselect
==================
*/
void Patch_Deselect( patchMesh_t *p ) {
	p->bSelected = false;
}


/*
==================
Patch_Delete
==================
*/
void Patch_Delete( patchMesh_t *p ) {
	if ( p->pSymbiot ) {
		p->pSymbiot->pPatch = NULL;
	}

	Mem_Free(p->verts);
	if ( p->epairs ) {
		delete p->epairs;
	}
	Mem_Free(p);

	p = NULL;

	UpdatePatchInspector();
}


/*
==================
Patch_Scale
==================
*/
void Patch_Scale( patchMesh_t *p,const idVec3 vOrigin,const idVec3 vAmt,bool bRebuild ) {
	for ( int w = 0; w < p->width; w++ ) {
		for ( int h = 0; h < p->height; h++ ) {
			if ( g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList(&p->ctrl(w, h).xyz) == -1 )
				continue;
			for ( int i = 0 ; i < 3 ; i++ ) {
				p->ctrl(w, h).xyz[i] -= vOrigin[i];
				p->ctrl(w, h).xyz[i] *= vAmt[i];
				p->ctrl(w, h).xyz[i] += vOrigin[i];
			}
		}
	} 
	if ( bRebuild ) {
		idVec3	vMin, vMax;
		Patch_CalcBounds(p, vMin, vMax);
		Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
	}
	UpdatePatchInspector();
}


/*
==================
Patch_Cleanup
==================
*/
void Patch_Cleanup() {
	//--g_nSelectedPatch = -1;
	//numPatchMeshes = 0;
}



/*
==================
Patch_SetView
==================
*/
void Patch_SetView( int n ) {
	g_bSameView = (n == g_nPatchClickedView);
	g_nPatchClickedView = n;
}


/*
==================
Patch_SetTexture
==================
*/
// FIXME: need array validation throughout
void Patch_SetTexture( patchMesh_t *p,texdef_t *tex_def ) {
	p->d_texture = Texture_ForName(tex_def->name);
	UpdatePatchInspector();
}

/*
==================
Patch_SetTexture
==================
*/
// FIXME: need array validation throughout
void Patch_SetTextureName( patchMesh_t *p,const char *name ) {
	p->d_texture = Texture_ForName(name);
	UpdatePatchInspector();
}


/*
==================
Patch_DragScale
==================
*/
bool Patch_DragScale( patchMesh_t *p,idVec3 vAmt,idVec3 vMove ) {
	idVec3	vMin, vMax, vScale, vTemp, vMid;
	int		i;

	Patch_CalcBounds(p, vMin, vMax);

	VectorSubtract(vMax, vMin, vTemp);

	// if we are scaling in the same dimension the patch has no depth
	for ( i = 0; i < 3; i ++ ) {
		if ( vTemp[i] == 0 && vMove[i] != 0 ) {
			//Patch_Move(n, vMove, true);
			return false;
		}
	}

	for ( i = 0 ; i < 3 ; i++ )
		vMid[i] = (vMin[i] + ((vMax[i] - vMin[i]) / 2));

	for ( i = 0; i < 3; i++ ) {
		if ( vAmt[i] != 0 ) {
			vScale[i] = 1.0f + vAmt[i] / vTemp[i];
		} else {
			vScale[i] = 1.0f;
		}
	}

	Patch_Scale(p, vMid, vScale, false);

	VectorSubtract(vMax, vMin, vTemp);

	Patch_CalcBounds(p, vMin, vMax);

	VectorSubtract(vMax, vMin, vMid);

	VectorSubtract(vMid, vTemp, vTemp);

	VectorScale(vTemp, 0.5, vTemp);

	// abs of both should always be equal
	if ( !vMove.Compare(vAmt) ) {
		for ( i = 0; i < 3; i++ ) {
			if ( vMove[i] != vAmt[i] ) {
				vTemp[i] = -(vTemp[i]);
			}
		}
	}

	Patch_Move(p, vTemp);
	return true;
}


/*
==================
Patch_InsertColumn
==================
*/
void Patch_InsertColumn( patchMesh_t *p,bool bAdd ) {
	int		h, w, i, j;
	idVec3	vTemp;

	if ( p->width + 2 >= MAX_PATCH_WIDTH ) {
		return;
	}

	Patch_AdjustSize(p, 2, 0);

	// re-adjust til after routine
	//p->width -= 2;

	if ( bAdd ) {
		// add column?
		for ( h = 0; h < p->height; h++ ) {
			j = p->width - 3;

			VectorSubtract(p->ctrl(j, h).xyz, p->ctrl(j - 1, h).xyz, vTemp);

			for ( i = 0; i < 3; i++ ) {
				vTemp[i] /= 3;
			}

			memcpy(&p->ctrl(j + 2, h), &p->ctrl(j, h), sizeof(idDrawVert));
			memcpy(&p->ctrl(j, h), &p->ctrl(j - 1, h), sizeof(idDrawVert));

			VectorAdd(p->ctrl(j, h).xyz, vTemp, p->ctrl(j, h).xyz);
			memcpy(&p->ctrl(j + 1, h), &p->ctrl(j, h), sizeof(idDrawVert));
			VectorAdd(p->ctrl(j + 1, h).xyz, vTemp, p->ctrl(j + 1, h).xyz);
		}
	} else {
		for ( h = 0; h < p->height; h++ ) {
			w = p->width - 3;
			while ( w >= 0 ) {
				memcpy(&p->ctrl(w + 2, h), &p->ctrl(w, h), sizeof(idDrawVert));
				w--;
			}
			VectorSubtract(p->ctrl(1, h).xyz, p->ctrl(0, h).xyz, vTemp);
			for ( i = 0; i < 3; i++ ) {
				vTemp[i] /= 3;
			}
			VectorCopy(p->ctrl(0, h).xyz, p->ctrl(1, h).xyz);
			VectorAdd(p->ctrl(1, h).xyz, vTemp, p->ctrl(1, h).xyz);
			VectorCopy(p->ctrl(1, h).xyz, p->ctrl(2, h).xyz);
			VectorAdd(p->ctrl(2, h).xyz, vTemp, p->ctrl(2, h).xyz);
		}
	}
	//p->width += 2;
	UpdatePatchInspector();
}


/*
==================
Patch_InsertRow
==================
*/
void Patch_InsertRow( patchMesh_t *p,bool bAdd ) {
	int		h, w, i, j;
	idVec3	vTemp;

	if ( p->height + 2 >= MAX_PATCH_HEIGHT ) {
		return;
	}

	Patch_AdjustSize(p, 0, 2);

	if ( bAdd ) {
		// add column?
		for ( w = 0; w < p->width; w++ ) {
			j = p->height - 3;
			VectorSubtract(p->ctrl(w, j).xyz, p->ctrl(w, j - 1).xyz, vTemp);
			for ( i = 0; i < 3; i++ ) {
				vTemp[i] /= 3;
			}

			memcpy(&p->ctrl(w, j + 2), &p->ctrl(w, j), sizeof(idDrawVert));
			memcpy(&p->ctrl(w, j), &p->ctrl(w, j - 1), sizeof(idDrawVert));

			VectorAdd(p->ctrl(w, j).xyz, vTemp, p->ctrl(w, j).xyz);
			memcpy(&p->ctrl(w, j + 1), &p->ctrl(w, j), sizeof(idDrawVert));
			VectorAdd(p->ctrl(w, j + 1).xyz, vTemp, p->ctrl(w, j + 1).xyz);
		}
	} else {
		for ( w = 0; w < p->width; w++ ) {
			h = p->height - 3;
			while ( h >= 0 ) {
				memcpy(&p->ctrl(w, h + 2), &p->ctrl(w, h), sizeof(idDrawVert));
				h--;
			}
			VectorSubtract(p->ctrl(w, 1).xyz, p->ctrl(w, 0).xyz, vTemp);
			for ( i = 0; i < 3; i++ ) {
				vTemp[i] /= 3;
			}

			VectorCopy(p->ctrl(w, 0).xyz, p->ctrl(w, 1).xyz);
			VectorAdd(p->ctrl(w, 1).xyz, vTemp, p->ctrl(w, 1).xyz);
			VectorCopy(p->ctrl(w, 1).xyz, p->ctrl(w, 2).xyz);
			VectorAdd(p->ctrl(w, 2).xyz, vTemp, p->ctrl(w, 2).xyz);
		}
	}

	UpdatePatchInspector();
}


/*
==================
Patch_RemoveRow
==================
*/
void Patch_RemoveRow( patchMesh_t *p,bool bFirst ) {
	if ( p->height <= MIN_PATCH_HEIGHT ) {
		return;
	}

	if ( bFirst ) {
		for ( int w = 0; w < p->width; w++ ) {
			for ( int h = 0; h < p->height - 2; h++ ) {
				memcpy(&p->ctrl(w, h), &p->ctrl(w, h + 2), sizeof(idDrawVert));
			}
		}
	}

	Patch_AdjustSize(p, 0, -2);

	UpdatePatchInspector();
}


/*
==================
Patch_RemoveColumn
==================
*/
void Patch_RemoveColumn( patchMesh_t *p,bool bFirst ) {
	if ( p->width <= MIN_PATCH_WIDTH ) {
		return;
	}

	if ( bFirst ) {
		for ( int h = 0; h < p->height; h++ ) {
			for ( int w = 0; w < p->width - 2; w++ ) {
				memcpy(&p->ctrl(w, h), &p->ctrl(w + 2, h), sizeof(idDrawVert));
			}
		}
	}

	Patch_AdjustSize(p, -2, 0);

	UpdatePatchInspector();
}


void Patch_DisperseRows() {
	idVec3	vTemp, vTemp2;
	int		i, w, h;


	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;
			Patch_Rebuild(p);
			for ( w = 0; w < p->width; w++ ) {
				// for each row, we need to evenly disperse p->height number 
				// of points across the old bounds

				// calc total distance to interpolate 
				VectorSubtract(p->ctrl(w, p->height - 1).xyz, p->ctrl(w, 0).xyz, vTemp);

				//vTemp[0] = vTemp[1] = vTemp[2] = 0;
				//for (h = 0; h < p->height - nRows; h ++)
				//{
				//  VectorAdd(vTemp, p->ctrl(w,h], vTemp);
				//}

				// amount per cycle
				for ( i = 0; i < 3; i ++ ) {
					vTemp2[i] = vTemp[i] / (p->height - 1);
				}

				// move along
				for ( h = 0; h < p->height - 1; h++ ) {
					VectorAdd(p->ctrl(w, h).xyz, vTemp2, p->ctrl(w, h + 1).xyz);
				}
				Patch_Naturalize(p);
			}
		}
	}
	UpdatePatchInspector();
}

/*
==================
Patch_AdjustColumns
==================
*/
void Patch_DisperseColumns() {
	idVec3	vTemp, vTemp2;
	int		i, w, h;

	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;
			Patch_Rebuild(p);

			for ( h = 0; h < p->height; h++ ) {
				// for each column, we need to evenly disperse p->width number 
				// of points across the old bounds

				// calc total distance to interpolate 
				VectorSubtract(p->ctrl(p->width - 1, h).xyz, p->ctrl(0, h).xyz, vTemp);

				// amount per cycle
				for ( i = 0; i < 3; i ++ ) {
					vTemp2[i] = vTemp[i] / (p->width - 1);
				}

				// move along
				for ( w = 0; w < p->width - 1; w++ ) {
					VectorAdd(p->ctrl(w, h).xyz, vTemp2, p->ctrl(w + 1, h).xyz);
				}
			}
			Patch_Naturalize(p);
		}
	}
	UpdatePatchInspector();
}



/*
==================
Patch_AdjustSelected
==================
*/
void Patch_AdjustSelected( bool bInsert,bool bColumn,bool bFlag ) {
	bool	bUpdate	= false;
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			if ( bInsert ) {
				if ( bColumn ) {
					Patch_InsertColumn(pb->pPatch, bFlag);
				} else {
					Patch_InsertRow(pb->pPatch, bFlag);
				}
			} else {
				if ( bColumn ) {
					Patch_RemoveColumn(pb->pPatch, bFlag);
				} else {
					Patch_RemoveRow(pb->pPatch, bFlag);
				}
			}
			bUpdate = true;
			idVec3		vMin, vMax;
			patchMesh_t	*p	= pb->pPatch;
			Patch_CalcBounds(p, vMin, vMax);
			Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
		}
	}
	if ( bUpdate ) {
		Sys_UpdateWindows(W_ALL);
	}
}

void Parse1DMatrix( int x,float *p ) {
	GetToken(true); // (
	for ( int i = 0; i < x; i++ ) {
		GetToken(false);
		p[i] = atof(token);
	}
	GetToken(true); // )
}

void Parse2DMatrix( int y,int x,float *p ) {
	GetToken(true); // (
	for ( int i = 0; i < y; i++ ) {
		Parse1DMatrix(x, p + i * x);
	}
	GetToken(true); // )
}

void Parse3DMatrix( int z,int y,int x,float *p ) {
	GetToken(true); // (
	for ( int i = 0; i < z; i++ ) {
		Parse2DMatrix(y, x, p + i * (x * MAX_PATCH_HEIGHT));
	}
	GetToken(true); // )
}

// parses a patch
brush_t * Patch_Parse( bool bOld ) {
	const idMaterial *tex = declManager->FindMaterial(NULL);
	GetToken(true);

	if ( strcmp(token, "{") ) {
		return NULL;
	}

	patchMesh_t	*pm	= NULL;

	if ( g_qeglobals.bSurfacePropertiesPlugin ) {
		assert(true);
		//GETPLUGINTEXDEF(pm)->ParsePatchTexdef();
	} else {
		// texture def
		GetToken(true);

		// band-aid 
		if ( strcmp(token, "(") ) {
			if ( g_qeglobals.mapVersion < 2.0f ) {
				tex = Texture_ForName(va("textures/%s", token));
			} else {
				tex = Texture_ForName(token);
			}
			GetToken(true);
		} else {
			common->Printf("Warning: Patch read with no texture, using notexture... \n");
		}   			

		if ( strcmp(token, "(") ) {
			return NULL;
		}

		// width, height, flags (currently only negative)
		GetToken(false);
		int	width	= atoi(token);

		GetToken(false);
		int	height	= atoi(token);

		pm = MakeNewPatch(width, height);
		pm->d_texture = tex;

		if ( !bOld ) {
			GetToken(false);
			pm->horzSubdivisions = atoi(token);
			GetToken(false);
			pm->vertSubdivisions = atoi(token);
			pm->explicitSubdivisions = true;
		}

		GetToken(false);
		pm->contents = atoi(token);

		GetToken(false);
		pm->flags = atoi(token);

		GetToken(false);
		pm->value = atoi(token);

		//if (!bOld)
		//{
		//	GetToken(false);
		//	pm->type = atoi(token);
		//}

		GetToken(false);
		if ( strcmp(token, ")") )
			return NULL;
	}



	float	ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT][5];
	Parse3DMatrix(pm->width, pm->height, 5, reinterpret_cast< float*>(&ctrl));

	int	w, h;

	for ( w = 0; w < pm->width; w++ ) {
		for ( h = 0; h < pm->height; h++ ) {
			pm->ctrl(w, h).xyz[0] = ctrl[w][h][0];  
			pm->ctrl(w, h).xyz[1] = ctrl[w][h][1];  
			pm->ctrl(w, h).xyz[2] = ctrl[w][h][2];  
			pm->ctrl(w, h).st[0] = ctrl[w][h][3]; 
			pm->ctrl(w, h).st[1] = ctrl[w][h][4];
		}
	}

	GetToken(true);

	if ( g_qeglobals.m_bBrushPrimitMode ) {
		// we are in brush primit mode, but maybe it's a classic patch that needs converting, test "}"
		if ( strcmp(token, "}") && strcmp(token, "(") ) {
			ParseEpair(pm->epairs);
			GetToken(true);
		}
	}

	if ( strcmp(token, "}") ) {
		return NULL;
	}

	brush_t	*b	= AddBrushForPatch(pm, false);

	return b;
}


/*
==================
Patch_Write 
==================
*/
void Patch_Write( patchMesh_t *p,CMemFile *file ) {
	if ( g_qeglobals.bSurfacePropertiesPlugin ) {
		common->Printf("WARNING: Patch_Write to a CMemFile and Surface Properties plugin not done\n");
	}

	if ( p->explicitSubdivisions ) {
		MemFile_fprintf(file, " {\n  patchDef3\n  {\n");
		MemFile_fprintf(file, "   \"%s\"\n", p->d_texture->GetName());
		MemFile_fprintf(file, "   ( %i %i %i %i %i %i %i ) \n", p->width, p->height, p->horzSubdivisions, p->vertSubdivisions, p->contents, p->flags, p->value);
	} else {
		MemFile_fprintf(file, " {\n  patchDef2\n  {\n");
		MemFile_fprintf(file, "   \"%s\"\n", p->d_texture->GetName());
		MemFile_fprintf(file, "   ( %i %i %i %i %i ) \n", p->width, p->height, p->contents, p->flags, p->value);
	}


	float	ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT][5];

	int		w, h;
	for ( w = 0; w < p->width; w++ ) {
		for ( h = 0; h < p->height; h++ ) {
			ctrl[w][h][0] = p->ctrl(w, h).xyz[0];
			ctrl[w][h][1] = p->ctrl(w, h).xyz[1];
			ctrl[w][h][2] = p->ctrl(w, h).xyz[2];
			ctrl[w][h][3] = p->ctrl(w, h).st[0];
			ctrl[w][h][4] = p->ctrl(w, h).st[1];
		}
	}

	_Write3DMatrix(file, p->width, p->height, 5, reinterpret_cast< float*>(&ctrl));

	if ( g_qeglobals.m_bBrushPrimitMode ) {
		if ( p->epairs ) {
			int	count	= p->epairs->GetNumKeyVals();
			for ( int i = 0; i < count; i++ ) {
				MemFile_fprintf(file, "\"%s\" \"%s\"\n", p->epairs->GetKeyVal(i)->GetKey().c_str(), p->epairs->GetKeyVal(i)->GetValue().c_str());
			}
		}
	}

	MemFile_fprintf(file, "  }\n }\n");
}

void Patch_Write( patchMesh_t *p,FILE *file ) {
	if ( p->explicitSubdivisions ) {
		fprintf(file, " {\n  patchDef3\n  {\n");
		fprintf(file, "   \"%s\"\n", p->d_texture->GetName());
		fprintf(file, "   ( %i %i %i %i %i %i %i ) \n", p->width, p->height, p->horzSubdivisions, p->vertSubdivisions, p->contents, p->flags, p->value);
	} else {
		fprintf(file, " {\n  patchDef2\n  {\n");
		fprintf(file, "   \"%s\"\n", p->d_texture->GetName());
		fprintf(file, "   ( %i %i %i %i %i ) \n", p->width, p->height, p->contents, p->flags, p->value);
	}

	float	ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT][5];

	int		w, h;
	for ( w = 0; w < p->width; w++ ) {
		for ( h = 0; h < p->height; h++ ) {
			ctrl[w][h][0] = p->ctrl(w, h).xyz[0];
			ctrl[w][h][1] = p->ctrl(w, h).xyz[1];
			ctrl[w][h][2] = p->ctrl(w, h).xyz[2];
			ctrl[w][h][3] = p->ctrl(w, h).st[0];
			ctrl[w][h][4] = p->ctrl(w, h).st[1];
		}
	}

	_Write3DMatrix(file, p->width, p->height, 5, reinterpret_cast< float*>(&ctrl));

	if ( g_qeglobals.m_bBrushPrimitMode ) {
		if ( p->epairs ) {
			int	count	= p->epairs->GetNumKeyVals();
			for ( int i = 0; i < count; i++ ) {
				fprintf(file, "\"%s\" \"%s\"\n", p->epairs->GetKeyVal(i)->GetKey().c_str(), p->epairs->GetKeyVal(i)->GetValue().c_str());
			}
		}
	}

	fprintf(file, "  }\n }\n");
}


/*
==================
Patch_RotateTexture
==================
*/
void Patch_RotateTexture( patchMesh_t *p,float fAngle ) {
	idVec3	vMin, vMax;
	Patch_CalcBounds(p, vMin, vMax);
	Patch_MakeDirty(p);
	for ( int w = 0; w < p->width; w++ ) {
		for ( int h = 0; h < p->height; h++ ) {
			if ( g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList(&p->ctrl(w, h).xyz) == -1 ) {
				continue;
			}

			float	x	= p->ctrl(w, h).st[0];
			float	y	= p->ctrl(w, h).st[1];
			p->ctrl(w, h).st[0] = x * cos(DEG2RAD(fAngle)) - y * sin(DEG2RAD(fAngle));
			p->ctrl(w, h).st[1] = y * cos(DEG2RAD(fAngle)) + x * sin(DEG2RAD(fAngle));
		}
	}
}


/*
==================
Patch_ScaleTexture
==================
*/
void Patch_ScaleTexture( patchMesh_t *p,float fx,float fy,bool absolute ) {
	if ( fx == 0 ) {
		fx = 1.0f;
	}
	if ( fy == 0 ) {
		fy = 1.0f;
	}

	if ( absolute ) {
		Patch_ResetTexturing(1, 1);
	}

	for ( int w = 0; w < p->width; w++ ) {
		for ( int h = 0; h < p->height; h++ ) {
			if ( g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList(&p->ctrl(w, h).xyz) == -1 ) {
				continue;
			}
			p->ctrl(w, h).st[0] *= fx;
			p->ctrl(w, h).st[1] *= fy;
		}
	}
	Patch_MakeDirty(p);
}

/*
==================
Patch_ShiftTexture
==================
*/
void Patch_ShiftTexture( patchMesh_t *p,float fx,float fy,bool autoAdjust ) {
	//if (fx)
	//  fx = (fx > 0) ? 0.1 : -0.1;
	//if (fy)
	//  fy = (fy > 0) ? 0.1 : -0.1;

	if ( autoAdjust ) {
		fx /= p->d_texture->GetEditorImage()->uploadWidth;
		fy /= p->d_texture->GetEditorImage()->uploadHeight;
	}

	for ( int w = 0; w < p->width; w++ ) {
		for ( int h = 0; h < p->height; h++ ) {
			if ( g_qeglobals.d_select_mode == sel_curvepoint && PointInMoveList(&p->ctrl(w, h).xyz) == -1 )
				continue;

			p->ctrl(w, h).st[0] += fx;
			p->ctrl(w, h).st[1] += fy;
		}
	}
	Patch_MakeDirty(p);
}

void patchInvert( patchMesh_t *p ) {
	idDrawVert	vertTemp;
	Patch_MakeDirty(p);
	for ( int i = 0 ; i < p->width ; i++ ) {
		for ( int j = 0; j < p->height / 2; j++ ) {
			memcpy(&vertTemp, &p->ctrl(i, p->height - 1 - j), sizeof(idDrawVert));
			memcpy(&p->ctrl(i, p->height - 1 - j), &p->ctrl(i, j), sizeof(idDrawVert));
			memcpy(&p->ctrl(i, j), &vertTemp, sizeof(idDrawVert));
		}
	}
}

/*
==================
Patch_ToggleInverted
==================
*/
void Patch_ToggleInverted() {
	bool	bUpdate	= false;

	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			bUpdate = true;
			patchInvert(pb->pPatch);
		}
	}

	if ( bUpdate ) {
		Sys_UpdateWindows(W_ALL);
	}
	UpdatePatchInspector();
}

void Patch_FlipTexture( patchMesh_t *p,bool y ) {
	idVec2	temp;
	Patch_MakeDirty(p);
	if ( y ) {
		for ( int i = 0 ; i < p->height ; i++ ) {
			for ( int j = 0; j < p->width / 2; j++ ) {
				temp = p->ctrl(p->width - 1 - j, i).st;
				p->ctrl(p->width - 1 - j, i).st = p->ctrl(j, i).st;
				p->ctrl(j, i).st = temp;
			}
		}
	} else {
		for ( int i = 0 ; i < p->width ; i++ ) {
			for ( int j = 0; j < p->height / 2; j++ ) {
				temp = p->ctrl(i, p->height - 1 - j).st;
				p->ctrl(i, p->height - 1 - j).st = p->ctrl(i, j).st;
				p->ctrl(i, j).st = temp;
			}
		}
	}
}


/*
==================
Patch_ToggleInverted
==================
*/
void Patch_InvertTexture( bool bY ) {
	bool	bUpdate	= false;
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			bUpdate = true;
			Patch_FlipTexture(pb->pPatch, bY);
		}
	}

	if ( bUpdate ) {
		Sys_UpdateWindows(W_ALL);
	}

	UpdatePatchInspector();
}




/*
==================
Patch_Save
==================
 Saves patch ctrl info (originally to deal with a 
 cancel in the surface dialog
*/
void Patch_Save( patchMesh_t *p ) {
	if ( patchSave ) {
		Mem_Free(patchSave->verts);
		Mem_Free(patchSave);
	}

	patchSave = MakeNewPatch(p->width, p->height);
	memcpy(patchSave->verts, p->verts, sizeof(p->verts[0]) * p->width * p->height);
}


/*
==================
Patch_Restore
==================
*/
void Patch_Restore( patchMesh_t *p ) {
	if ( patchSave ) {
		p->width = patchSave->width;
		p->height = patchSave->height;
		memcpy(p->verts, patchSave->verts, sizeof(p->verts[0]) * p->width * p->height);
		Mem_Free(patchSave->verts);
		Mem_Free(patchSave);
		patchSave = NULL;
	}
}

void Patch_FitTexture( patchMesh_t *p,float fx,float fy ) {
	Patch_MakeDirty(p);
	for ( int i = 0 ; i < p->width ; i++ ) {
		for ( int j = 0 ; j < p->height ; j++ ) {
			p->ctrl(i, j).st[0] = fx * (float) i / (p->width - 1);
			p->ctrl(i, j).st[1] = fy * (float) j / (p->height - 1);
		}
	}
}

void Patch_ResetTexturing( float fx,float fy ) {
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;
			Patch_MakeDirty(p);
			for ( int i = 0 ; i < p->width ; i++ ) {
				for ( int j = 0 ; j < p->height ; j++ ) {
					p->ctrl(i, j).st[0] = fx * (float) i / (p->width - 1);
					p->ctrl(i, j).st[1] = fy * (float) j / (p->height - 1);
				}
			}
		}
	}
}


void Patch_FitTexturing() {
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;
			Patch_MakeDirty(p);
			for ( int i = 0 ; i < p->width ; i++ ) {
				for ( int j = 0 ; j < p->height ; j++ ) {
					p->ctrl(i, j).st[0] = 1 * (float) i / (p->width - 1);
					p->ctrl(i, j).st[1] = 1 * (float) j / (p->height - 1);
				}
			}
		}
	}
}

void Patch_SetTextureInfo( texdef_t *pt ) {
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			if ( pt->rotate )
				Patch_RotateTexture(pb->pPatch, pt->rotate);

			if ( pt->shift[0] || pt->shift[1] )
				Patch_ShiftTexture(pb->pPatch, pt->shift[0], pt->shift[1], false);

			if ( pt->scale[0] || pt->scale[1] )
				Patch_ScaleTexture(pb->pPatch, pt->scale[0], pt->scale[1], false);

			patchMesh_t	*p	= pb->pPatch;
			p->value = pt->value;
		}
	}
}

bool WINAPI OnlyPatchesSelected() {
	if ( g_ptrSelectedFaces.GetSize() > 0 || selected_brushes.next == &selected_brushes ) {
		return false;
	}
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( !pb->pPatch ) {
			return false;
		}
	}
	return true;
}

bool WINAPI AnyPatchesSelected() {
	if ( g_ptrSelectedFaces.GetSize() > 0 || selected_brushes.next == &selected_brushes ) {
		return false;
	}
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			return true;
		}
	}
	return false;
}

patchMesh_t * SinglePatchSelected() {
	if ( selected_brushes.next->pPatch ) {
		return selected_brushes.next->pPatch;
	}
	return NULL;
}

void Patch_BendToggle() {
	if ( g_bPatchBendMode ) {
		g_bPatchBendMode = false;
		HideInfoDialog();
		g_pParentWnd->UpdatePatchToolbarButtons() ;
		return;
	}

	brush_t	*b	= selected_brushes.next;

	if ( !QE_SingleBrush() || !b->pPatch ) {
		Sys_Status("Must bend a single patch");
		return;
	}

	Patch_Save(b->pPatch);
	g_bPatchBendMode = true;
	g_nPatchBendState = BEND_SELECT_ROTATION;
	g_bPatchAxisOnRow = true;
	g_nPatchAxisIndex = 1;
	ShowInfoDialog(g_pBendStateMsg[BEND_SELECT_ROTATION]);
}

void Patch_BendHandleTAB() {
	if ( !g_bPatchBendMode ) {
		return;
	}

	brush_t	*b	= selected_brushes.next;
	if ( !QE_SingleBrush() || !b->pPatch ) {
		Patch_BendToggle();
		Sys_Status("No patch to bend!");
		return;
	}

	patchMesh_t	*p		= b->pPatch;

	bool		bShift	= ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0);

	if ( g_nPatchBendState == BEND_SELECT_ROTATION ) {
		// only able to deal with odd numbered rows/cols
		g_nPatchAxisIndex += (bShift) ? -2 : 2;
		if ( g_bPatchAxisOnRow ) {
			if ( (bShift) ? g_nPatchAxisIndex <= 0 : g_nPatchAxisIndex >= p->height ) {
				g_bPatchAxisOnRow = false;
				g_nPatchAxisIndex = (bShift) ? p->width - 1 : 1;
			}
		} else {
			if ( (bShift) ? g_nPatchAxisIndex <= 0 : g_nPatchAxisIndex >= p->width ) {
				g_bPatchAxisOnRow = true;
				g_nPatchAxisIndex = (bShift) ? p->height - 1 : 1;
			}
		}
	} else if ( g_nPatchBendState == BEND_SELECT_ORIGIN ) {
		g_nBendOriginIndex += (bShift) ? -1 : 1;
		if ( g_bPatchAxisOnRow ) {
			if ( bShift ) {
				if ( g_nBendOriginIndex < 0 )
					g_nBendOriginIndex = p->width - 1;
			} else {
				if ( g_nBendOriginIndex > p->width - 1 )
					g_nBendOriginIndex = 0;
			}
			VectorCopy(p->ctrl(g_nBendOriginIndex, g_nPatchAxisIndex).xyz, g_vBendOrigin);
		} else {
			if ( bShift ) {
				if ( g_nBendOriginIndex < 0 )
					g_nBendOriginIndex = p->height - 1;
			} else {
				if ( g_nBendOriginIndex > p->height - 1 )
					g_nBendOriginIndex = 0;
			}
			VectorCopy(p->ctrl(g_nPatchAxisIndex, g_nBendOriginIndex).xyz, g_vBendOrigin);
		}
	} else if ( g_nPatchBendState == BEND_SELECT_EDGE ) {
		g_bPatchLowerEdge ^= 1;
	}
	Sys_UpdateWindows(W_ALL);
}

void Patch_BendHandleENTER() {
	if ( !g_bPatchBendMode ) {
		return;
	}

	if ( g_nPatchBendState < BEND_BENDIT ) {
		g_nPatchBendState++;
		ShowInfoDialog(g_pBendStateMsg[g_nPatchBendState]);
		if ( g_nPatchBendState == BEND_SELECT_ORIGIN ) {
			g_vBendOrigin[0] = g_vBendOrigin[1] = g_vBendOrigin[2] = 0;
			g_nBendOriginIndex = 0;
			Patch_BendHandleTAB();
		} else if ( g_nPatchBendState == BEND_SELECT_EDGE ) {
			g_bPatchLowerEdge = true;
		} else if ( g_nPatchBendState == BEND_BENDIT ) {
			// basically we go into rotation mode, set the axis to the center of the 
		}
	} else {
		// done
		Patch_BendToggle();
	}
	Sys_UpdateWindows(W_ALL);
}


void Patch_BendHandleESC() {
	if ( !g_bPatchBendMode ) {
		return;
	}
	Patch_BendToggle();
	brush_t	*b	= selected_brushes.next;
	if ( QE_SingleBrush() && b->pPatch ) {
		Patch_Restore(b->pPatch);
	}
	Sys_UpdateWindows(W_ALL);
}

void Patch_SetBendRotateOrigin( patchMesh_t *p ) {
	int	nType	= g_pParentWnd->ActiveXY()->GetViewType();
	int	nDim3	= (nType == XY) ? 2 : (nType == YZ) ? 0 : 1;
	g_vBendOrigin[nDim3] = 0;
	VectorCopy(g_vBendOrigin, g_pParentWnd->ActiveXY()->RotateOrigin());
	return;
}

// also sets the rotational origin
void Patch_SelectBendAxis() {
	brush_t	*b	= selected_brushes.next;
	if ( !QE_SingleBrush() || !b->pPatch ) {
		// should not ever happen
		Patch_BendToggle();
		return;
	}

	patchMesh_t	*p	= b->pPatch;
	if ( g_bPatchAxisOnRow ) {
		SelectRow(p, g_nPatchAxisIndex, false);
	} else {
		SelectColumn(p, g_nPatchAxisIndex, false);
	}

	Patch_SetBendRotateOrigin(p);
}

void Patch_SelectBendNormal() {
	brush_t	*b	= selected_brushes.next;
	if ( !QE_SingleBrush() || !b->pPatch ) {
		// should not ever happen
		Patch_BendToggle();
		return;
	}

	patchMesh_t	*p	= b->pPatch;

	g_qeglobals.d_num_move_points = 0;
	if ( g_bPatchAxisOnRow ) {
		if ( g_bPatchLowerEdge ) {
			for ( int j = 0; j < g_nPatchAxisIndex; j++ )
				SelectRow(p, j, true);
		} else {
			for ( int j = p->height - 1; j > g_nPatchAxisIndex; j-- )
				SelectRow(p, j, true);
		}
	} else {
		if ( g_bPatchLowerEdge ) {
			for ( int j = 0; j < g_nPatchAxisIndex; j++ )
				SelectColumn(p, j, true);
		} else {
			for ( int j = p->width - 1; j > g_nPatchAxisIndex; j-- )
				SelectColumn(p, j, true);
		}
	}
	Patch_SetBendRotateOrigin(p);
}



void Patch_InsDelToggle() {
	if ( g_bPatchInsertMode ) {
		g_bPatchInsertMode = false;
		HideInfoDialog();
		g_pParentWnd->UpdatePatchToolbarButtons() ;
		return;
	}

	brush_t	*b	= selected_brushes.next;

	if ( !QE_SingleBrush() || !b->pPatch ) {
		Sys_Status("Must work with a single patch");
		return;
	}

	Patch_Save(b->pPatch);
	g_bPatchInsertMode = true;
	g_nPatchInsertState = INSERT_SELECT_EDGE;
	g_bPatchAxisOnRow = true;
	g_nPatchAxisIndex = 0;
	ShowInfoDialog(g_pInsertStateMsg[INSERT_SELECT_EDGE]);
}

void Patch_InsDelESC() {
	if ( !g_bPatchInsertMode ) {
		return;
	}
	Patch_InsDelToggle();
	Sys_UpdateWindows(W_ALL);
}


void Patch_InsDelHandleENTER() {
}

void Patch_InsDelHandleTAB() {
	if ( !g_bPatchInsertMode ) {
		Patch_InsDelToggle();
		return;
	}

	brush_t	*b	= selected_brushes.next;
	if ( !QE_SingleBrush() || !b->pPatch ) {
		Patch_BendToggle();
		common->Printf("No patch to bend!");
		return;
	}

	patchMesh_t	*p	= b->pPatch;

	// only able to deal with odd numbered rows/cols
	g_nPatchAxisIndex += 2;
	if ( g_bPatchAxisOnRow ) {
		if ( g_nPatchAxisIndex >= p->height - 1 ) {
			g_bPatchAxisOnRow = false;
			g_nPatchAxisIndex = 0;
		}
	} else {
		if ( g_nPatchAxisIndex >= p->width - 1 ) {
			g_bPatchAxisOnRow = true;
			g_nPatchAxisIndex = 0;
		}
	}
	Sys_UpdateWindows(W_ALL);
}


void _Write1DMatrix( FILE *f,int x,float *m ) {
	int	i;

	fprintf(f, "( ");
	for ( i = 0 ; i < x ; i++ ) {
		if ( m[i] == (int) m[i] ) {
			fprintf(f, "%i ", (int) m[i]);
		} else {
			fprintf(f, "%f ", m[i]);
		}
	}
	fprintf(f, ")");
}

void _Write2DMatrix( FILE *f,int y,int x,float *m ) {
	int	i;

	fprintf(f, "( ");
	for ( i = 0 ; i < y ; i++ ) {
		_Write1DMatrix(f, x, m + i * x);
		fprintf(f, " ");
	}
	fprintf(f, ")\n");
}


void _Write3DMatrix( FILE *f,int z,int y,int x,float *m ) {
	int	i;

	fprintf(f, "(\n");
	for ( i = 0 ; i < z ; i++ ) {
		_Write2DMatrix(f, y, x, m + i * (x * MAX_PATCH_HEIGHT));
	}
	fprintf(f, ")\n");
}

void _Write1DMatrix( CMemFile *f,int x,float *m ) {
	int	i;

	MemFile_fprintf(f, "( ");
	for ( i = 0 ; i < x ; i++ ) {
		if ( m[i] == (int) m[i] ) {
			MemFile_fprintf(f, "%i ", (int) m[i]);
		} else {
			MemFile_fprintf(f, "%f ", m[i]);
		}
	}
	MemFile_fprintf(f, ")");
}

void _Write2DMatrix( CMemFile *f,int y,int x,float *m ) {
	int	i;

	MemFile_fprintf(f, "( ");
	for ( i = 0 ; i < y ; i++ ) {
		_Write1DMatrix(f, x, m + i * x);
		MemFile_fprintf(f, " ");
	}
	MemFile_fprintf(f, ")\n");
}


void _Write3DMatrix( CMemFile *f,int z,int y,int x,float *m ) {
	int	i;

	MemFile_fprintf(f, "(\n");
	for ( i = 0 ; i < z ; i++ ) {
		_Write2DMatrix(f, y, x, m + i * (x * MAX_PATCH_HEIGHT));
	}
	MemFile_fprintf(f, ")\n");
}


void Patch_NaturalizeSelected( bool bCap,bool bCycleCap,bool alt ) {
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			if ( bCap ) {
				Patch_CapTexture(pb->pPatch, bCycleCap, alt);
			} else {
				Patch_Naturalize(pb->pPatch, true, true, alt);
			}
		}
	}
}

void Patch_SubdivideSelected( bool subdivide,int horz,int vert ) {
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			pb->pPatch->explicitSubdivisions = subdivide;
			if ( horz <= 0 ) {
				horz = 1;
			}
			if ( vert <= 0 ) {
				vert = 1;
			}
			pb->pPatch->horzSubdivisions = horz;
			pb->pPatch->vertSubdivisions = vert;
			Patch_MakeDirty(pb->pPatch);
		}
	}
}



bool within( idVec3 vTest,idVec3 vTL,idVec3 vBR ) {
	int	nDim1	= (g_pParentWnd->ActiveXY()->GetViewType() == YZ) ? 1 : 0;
	int	nDim2	= (g_pParentWnd->ActiveXY()->GetViewType() == XY) ? 1 : 2;
	if ( (vTest[nDim1] > vTL[nDim1] && vTest[nDim1] < vBR[nDim1]) || (vTest[nDim1] < vTL[nDim1] && vTest[nDim1] > vBR[nDim1]) ) {
		if ( (vTest[nDim2] > vTL[nDim2] && vTest[nDim2] < vBR[nDim2]) || (vTest[nDim2] < vTL[nDim2] && vTest[nDim2] > vBR[nDim2]) ) {
			return true;
		}
	}
	return false;
}


void Patch_SelectAreaPoints() {
	//jhefty - make patch selection additive ALWAYS
	//g_qeglobals.d_num_move_points = 0;
	g_nPatchClickedView = -1;

	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;
			for ( int i = 0; i < p->width; i++ ) {
				for ( int j = 0; j < p->height; j++ ) {
					if ( within(p->ctrl(i, j).xyz, g_qeglobals.d_vAreaTL, g_qeglobals.d_vAreaBR) ) {
						g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = &p->ctrl(i, j).xyz;
					}
				}
			}
		}
	}
}

const char * Patch_GetTextureName() {
	brush_t	*b	= selected_brushes.next;
	if ( b->pPatch ) {
		patchMesh_t	*p	= b->pPatch;
		if ( p->d_texture->GetName() )
			return p->d_texture->GetName();
	}
	return "";
}

patchMesh_t * Patch_Duplicate( patchMesh_t *pFrom ) {
	patchMesh_t	*p	= MakeNewPatch(pFrom->width, pFrom->height);
	p->contents = pFrom->contents;
	p->value = pFrom->value;
	p->horzSubdivisions = pFrom->horzSubdivisions;
	p->vertSubdivisions = pFrom->vertSubdivisions;
	p->explicitSubdivisions = pFrom->explicitSubdivisions;
	p->d_texture = pFrom->d_texture;
	p->bSelected = false;
	p->bOverlay = false;
	p->nListID = -1;

	memcpy(p->verts, pFrom->verts, p->width * p->height * sizeof(idDrawVert));

	AddBrushForPatch(p);
	return p;
}


void Patch_Thicken( int nAmount,bool bSeam ) {
	int			i, j, h, w;
	brush_t		*b;
	patchMesh_t	*pSeam;
	idVec3		vMin, vMax;
	CPtrArray	brushes;

	nAmount = -nAmount;


	if ( !QE_SingleBrush() ) {
		Sys_Status("Cannot thicken multiple patches. Please select a single patch.\n");
		return;
	}

	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( !pb->pPatch ) {
			return;
		}

		patchMesh_t	*p	= pb->pPatch;
		Patch_MeshNormals(p);
		patchMesh_t	*pNew	= Patch_Duplicate(p);
		for ( i = 0; i < p->width; i++ ) {
			for ( j = 0; j < p->height; j++ ) {
				VectorMA(p->ctrl(i, j).xyz, nAmount, p->ctrl(i, j).normal, pNew->ctrl(i, j).xyz);
			}
		}

		Patch_Rebuild(pNew);
		pNew->type |= PATCH_THICK;
		brushes.Add(pNew->pSymbiot);

		if ( bSeam ) {
			// FIXME: this should detect if any edges of the patch are closed and act appropriately
			// 
			if ( !(p->type & PATCH_CYLINDER) ) {
				b = Patch_GenericMesh(3, p->height, 2, false, true, p);
				pSeam = b->pPatch;
				pSeam->type |= PATCH_SEAM;
				for ( i = 0; i < p->height; i++ ) {
					VectorCopy(p->ctrl(0, i).xyz, pSeam->ctrl(0, i).xyz);
					VectorCopy(pNew->ctrl(0, i).xyz, pSeam->ctrl(2, i).xyz);
					VectorAdd(pSeam->ctrl(0, i).xyz, pSeam->ctrl(2, i).xyz, pSeam->ctrl(1, i).xyz);
					VectorScale(pSeam->ctrl(1, i).xyz, 0.5, pSeam->ctrl(1, i).xyz);
				}


				Patch_CalcBounds(pSeam, vMin, vMax);
				Brush_RebuildBrush(pSeam->pSymbiot, vMin, vMax);
				//--Patch_CapTexture(pSeam);
				Patch_Naturalize(pSeam);
				patchInvert(pSeam);
				brushes.Add(b);

				w = p->width - 1;
				b = Patch_GenericMesh(3, p->height, 2, false, true, p);
				pSeam = b->pPatch;
				pSeam->type |= PATCH_SEAM;
				for ( i = 0; i < p->height; i++ ) {
					VectorCopy(p->ctrl(w, i).xyz, pSeam->ctrl(0, i).xyz);
					VectorCopy(pNew->ctrl(w, i).xyz, pSeam->ctrl(2, i).xyz);
					VectorAdd(pSeam->ctrl(0, i).xyz, pSeam->ctrl(2, i).xyz, pSeam->ctrl(1, i).xyz);
					VectorScale(pSeam->ctrl(1, i).xyz, 0.5, pSeam->ctrl(1, i).xyz);
				}
				Patch_CalcBounds(pSeam, vMin, vMax);
				Brush_RebuildBrush(pSeam->pSymbiot, vMin, vMax);
				//--Patch_CapTexture(pSeam);
				Patch_Naturalize(pSeam);
				brushes.Add(b);
			}

			//--{
			// otherwise we will add one per end
			b = Patch_GenericMesh(p->width, 3, 2, false, true, p);
			pSeam = b->pPatch;
			pSeam->type |= PATCH_SEAM;
			for ( i = 0; i < p->width; i++ ) {
				VectorCopy(p->ctrl(i, 0).xyz, pSeam->ctrl(i, 0).xyz);
				VectorCopy(pNew->ctrl(i, 0).xyz, pSeam->ctrl(i, 2).xyz);
				VectorAdd(pSeam->ctrl(i, 0).xyz, pSeam->ctrl(i, 2).xyz, pSeam->ctrl(i, 1).xyz);
				VectorScale(pSeam->ctrl(i, 1).xyz, 0.5, pSeam->ctrl(i, 1).xyz);
			}


			Patch_CalcBounds(pSeam, vMin, vMax);
			Brush_RebuildBrush(pSeam->pSymbiot, vMin, vMax);
			//--Patch_CapTexture(pSeam);
			Patch_Naturalize(pSeam);
			patchInvert(pSeam);
			brushes.Add(b);

			h = p->height - 1;
			b = Patch_GenericMesh(p->width, 3, 2, false, true, p);
			pSeam = b->pPatch;
			pSeam->type |= PATCH_SEAM;
			for ( i = 0; i < p->width; i++ ) {
				VectorCopy(p->ctrl(i, h).xyz, pSeam->ctrl(i, 0).xyz);
				VectorCopy(pNew->ctrl(i, h).xyz, pSeam->ctrl(i, 2).xyz);
				VectorAdd(pSeam->ctrl(i, 0).xyz, pSeam->ctrl(i, 2).xyz, pSeam->ctrl(i, 1).xyz);
				VectorScale(pSeam->ctrl(i, 1).xyz, 0.5, pSeam->ctrl(i, 1).xyz);
			}
			Patch_CalcBounds(pSeam, vMin, vMax);
			Brush_RebuildBrush(pSeam->pSymbiot, vMin, vMax);
			//--Patch_CapTexture(pSeam);
			Patch_Naturalize(pSeam);
			brushes.Add(b);
			//--}
		}
		patchInvert(pNew);
	}

	for ( i = 0; i < brushes.GetSize(); i++ ) {
		Select_Brush(reinterpret_cast< brush_t*>(brushes.GetAt(i)));
	}

	if ( brushes.GetSize() > 0 ) {
		eclass_t*pecNew	= Eclass_ForName("func_static", false);
		if ( pecNew ) {
			entity_t*e	= Entity_Create(pecNew);
			SetKeyValue(e, "type", "patchThick");
		}
	}

	UpdatePatchInspector();
}


/*
lets get another list together as far as necessities..

*snapping stuff to the grid (i will only snap movements by the mouse to the grid.. snapping the rotational bend stuff will fubar everything)

capping bevels/endcaps

hot keys

texture fix for caps

clear clipboard

*region fix

*surface dialog

*/

void Patch_SetOverlays() {
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			pb->pPatch->bOverlay = true;
		}
	}
}



void Patch_ClearOverlays() {
	brush_t	*pb;
	for ( pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			pb->pPatch->bOverlay = false;
		}
	}

	for ( pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			pb->pPatch->bOverlay = false;
		}
	}
}

// freezes selected vertices
void Patch_Freeze() {
	brush_t	*pb;
	for ( pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			pb->pPatch->bOverlay = false;
		}
	}

	for ( pb = active_brushes.next ; pb != &active_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			pb->pPatch->bOverlay = false;
		}
	}
}

void Patch_UnFreeze( bool bAll ) {
}


void Patch_Transpose() {
	int			i, j, w;
	idDrawVert	dv;
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;

			if ( p->width > p->height ) {
				for ( i = 0 ; i < p->height ; i++ ) {
					for ( j = i + 1 ; j < p->width ; j++ ) {
						if ( j < p->height ) {
							// swap the value
							memcpy(&dv, &p->ctrl(j, i), sizeof(idDrawVert));
							memcpy(&p->ctrl(j, i), &p->ctrl(i, j), sizeof(idDrawVert));
							memcpy(&p->ctrl(i, j), &dv, sizeof(idDrawVert));
						} else {
							// just copy
							memcpy(&p->ctrl(j, i), &p->ctrl(i, j), sizeof(idDrawVert));
						}
					}
				}
			} else {
				for ( i = 0 ; i < p->width ; i++ ) {
					for ( j = i + 1 ; j < p->height ; j++ ) {
						if ( j < p->width ) {
							// swap the value
							memcpy(&dv, &p->ctrl(i, j), sizeof(idDrawVert));
							memcpy(&p->ctrl(i, j), &p->ctrl(j, i), sizeof(idDrawVert));
							memcpy(&p->ctrl(j, i), &dv, sizeof(idDrawVert));
						} else {
							// just copy
							memcpy(&p->ctrl(i, j), &p->ctrl(j, i), sizeof(idDrawVert));
						}
					}
				}
			}

			w = p->width;
			p->width = p->height;
			p->height = w;
			patchInvert(p);
			Patch_Rebuild(p);
		}
	}
}



void Select_SnapToGrid() {
	int	i, j, k;
	for ( brush_t*pb = selected_brushes.next ; pb != &selected_brushes ; pb = pb->next ) {
		if ( pb->pPatch ) {
			patchMesh_t	*p	= pb->pPatch;
			for ( i = 0; i < p->width; i++ ) {
				for ( j = 0; j < p->height; j++ ) {
					for ( k = 0; k < 3; k++ ) {
						p->ctrl(i, j).xyz[k] = floor(p->ctrl(i, j).xyz[k] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
					}
				}
			}
			idVec3	vMin, vMax;
			Patch_CalcBounds(p, vMin, vMax);
			Brush_RebuildBrush(p->pSymbiot, vMin, vMax);
		} else {
			Brush_SnapToGrid(pb);
		}
	}
}


void Patch_FindReplaceTexture( brush_t *pb,const char *pFind,const char *pReplace,bool bForce ) {
	if ( pb->pPatch ) {
		patchMesh_t	*p	= pb->pPatch;
		if ( bForce || idStr::Icmp(p->d_texture->GetName(), pFind) == 0 ) {
			p->d_texture = Texture_ForName(pReplace);
			//strcpy(p->d_texture->name, pReplace);
		}
	}
}

void Patch_ReplaceQTexture( brush_t *pb,idMaterial *pOld,idMaterial *pNew ) {
	if ( pb->pPatch ) {
		patchMesh_t	*p	= pb->pPatch;
		if ( p->d_texture == pOld ) {
			p->d_texture = pNew;
		}
	}
}

void Patch_Clone( patchMesh_t *p,brush_t *pNewOwner ) {
}

void Patch_FromTriangle( idVec5 vx,idVec5 vy,idVec5 vz ) {
	patchMesh_t	*p	= MakeNewPatch(3, 3);
	p->d_texture = Texture_ForName(g_qeglobals.d_texturewin.texdef.name);
	p->type = PATCH_TRIANGLE;

	// 0 0 goes to x
	// 0 1 goes to x
	// 0 2 goes to x

	// 1 0 goes to mid of x and z
	// 1 1 goes to mid of x y and z
	// 1 2 goes to mid of x and y

	// 2 0 goes to z
	// 2 1 goes to mid of y and z
	// 2 2 goes to y

	idVec5	vMidXZ;
	idVec5	vMidXY;
	idVec5	vMidYZ;


	vMidXZ.Lerp(vx, vz, 0.5);
	vMidXY.Lerp(vx, vy, 0.5);
	vMidYZ.Lerp(vy, vz, 0.5);

	p->ctrl(0, 0).xyz = vx.ToVec3();
	p->ctrl(0, 1).xyz = vx.ToVec3();
	p->ctrl(0, 2).xyz = vx.ToVec3();
	p->ctrl(0, 0).st[0] = vx[3];
	p->ctrl(0, 0).st[1] = vx[4];
	p->ctrl(0, 1).st[0] = vx[3];
	p->ctrl(0, 1).st[1] = vx[4];
	p->ctrl(0, 2).st[0] = vx[3];
	p->ctrl(0, 2).st[1] = vx[4];

	p->ctrl(1, 0).xyz = vMidXY.ToVec3();
	p->ctrl(1, 1).xyz = vx.ToVec3();
	p->ctrl(1, 2).xyz = vMidXZ.ToVec3();
	p->ctrl(1, 0).st[0] = vMidXY[3];
	p->ctrl(1, 0).st[1] = vMidXY[4];
	p->ctrl(1, 1).st[0] = vx[3];
	p->ctrl(1, 1).st[1] = vx[4];
	p->ctrl(1, 2).st[0] = vMidXZ[3];
	p->ctrl(1, 2).st[1] = vMidXZ[4];

	p->ctrl(2, 0).xyz = vy.ToVec3();
	p->ctrl(2, 1).xyz = vMidYZ.ToVec3();
	p->ctrl(2, 2).xyz = vz.ToVec3();
	p->ctrl(2, 0).st[0] = vy[3];
	p->ctrl(2, 0).st[1] = vy[4];
	p->ctrl(2, 1).st[0] = vMidYZ[3];
	p->ctrl(2, 1).st[1] = vMidYZ[4];
	p->ctrl(2, 2).st[0] = vz[3];
	p->ctrl(2, 2).st[1] = vz[4];


	//Patch_Naturalize(p);

	brush_t	*b	= AddBrushForPatch(p);
}


/*
==============
Patch_SetEpair
sets an epair for the given patch
==============
*/
void Patch_SetEpair( patchMesh_t *p,const char *pKey,const char *pValue ) {
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		if ( p->epairs == NULL ) {
			p->epairs = new idDict;
		}
		p->epairs->Set(pKey, pValue);
	}
}

/* 
=================
Patch_GetKeyValue
=================
*/
const char * Patch_GetKeyValue( patchMesh_t *p,const char *pKey ) {
	if ( g_qeglobals.m_bBrushPrimitMode ) {
		if ( p->epairs ) {
			return p->epairs->GetString(pKey);
		}
	}
	return "";
}


//Real nitpicky, but could you make CTRL-S save the current map with the current name? (ie: File/Save)
/*
Feature addition.
When reading in textures, please check for the presence of a file called "textures.link" or something, which contains one line such as;

g:\quake3\baseq3\textures\common

 So that, when I'm reading in, lets say, my \eerie directory, it goes through and adds my textures to the palette, along with everything in common.

  Don't forget to add "Finer texture alignment" to the list. I'd like to be able to move in 0.1 increments using the Shift-Arrow Keys.

  No. Sometimes textures are drawn the wrong way on patches. We'd like the ability to flip a texture. Like the way X/Y scale -1 used to worked.

  1) Easier way of deleting rows, columns
2) Fine tuning of textures on patches (X/Y shifts other than with the surface dialog)
2) Patch matrix transposition

  1) Actually, bump texture flipping on patches to the top of the list of things to do.
2) When you select a patch, and hit S, it should read in the selected patch texture. Should not work if you multiselect patches and hit S
3) Brandon has a wierd anomoly. He fine-tunes a patch with caps. It looks fine when the patch is selected, but as soon as he escapes out, it reverts to it's pre-tuned state. When he selects the patch again, it looks tuned


*1) Flipping textures on patches
*2) When you select a patch, and hit S, it should read in the selected patch texture. Should not work if you multiselect patches and hit S
3) Easier way of deleting rows columns
*4) Thick Curves
5) Patch matrix transposition
6) Inverted cylinder capping
*7) bugs
*8) curve speed

  Have a new feature request. "Compute Bounding Box" for mapobjects (md3 files). This would be used for misc_mapobject (essentially, drop in 3DS Max models into our maps)

  Ok, Feature Request. Load and draw MD3's in the Camera view with proper bounding boxes. This should be off misc_model

  Feature Addition: View/Hide Hint Brushes -- This should be a specific case.
*/




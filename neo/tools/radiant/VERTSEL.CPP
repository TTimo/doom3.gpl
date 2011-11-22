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

#define NEWEDGESEL	1

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int FindPoint(idVec3 point) {
	int i, j;

	for (i = 0; i < g_qeglobals.d_numpoints; i++) {
		for (j = 0; j < 3; j++) {
			if (idMath::Fabs(point[j] - g_qeglobals.d_points[i][j]) > 0.1) {
				break;
			}
		}

		if (j == 3) {
			return i;
		}
	}

	VectorCopy(point, g_qeglobals.d_points[g_qeglobals.d_numpoints]);
	if (g_qeglobals.d_numpoints < MAX_POINTS - 1) {
		g_qeglobals.d_numpoints++;
	}

	return g_qeglobals.d_numpoints - 1;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int FindEdge(int p1, int p2, face_t *f) {
	int i;

	for (i = 0; i < g_qeglobals.d_numedges; i++) {
		if (g_qeglobals.d_edges[i].p1 == p2 && g_qeglobals.d_edges[i].p2 == p1) {
			g_qeglobals.d_edges[i].f2 = f;
			return i;
		}
	}

	g_qeglobals.d_edges[g_qeglobals.d_numedges].p1 = p1;
	g_qeglobals.d_edges[g_qeglobals.d_numedges].p2 = p2;
	g_qeglobals.d_edges[g_qeglobals.d_numedges].f1 = f;

	if (g_qeglobals.d_numedges < MAX_EDGES - 1) {
		g_qeglobals.d_numedges++;
	}

	return g_qeglobals.d_numedges - 1;
}

#ifdef NEWEDGESEL
void MakeFace (brush_t * b, face_t * f)
#else
void MakeFace (face_t * f)
#endif 
{
	idWinding	*w;
	int			i;
	int			pnum[128];

#ifdef NEWEDGESEL
	w = Brush_MakeFaceWinding(b, f);
#else
	w = Brush_MakeFaceWinding(selected_brushes.next, f);
#endif
	if (!w) {
		return;
	}
	for (i = 0; i < w->GetNumPoints(); i++) {
		pnum[i] = FindPoint( (*w)[i].ToVec3() );
	}
	for (i = 0; i < w->GetNumPoints(); i++) {
		FindEdge(pnum[i], pnum[(i + 1) % w->GetNumPoints()], f);
	}
	delete w;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetupVertexSelection(void) {
	face_t	*f;
	brush_t *b;

	g_qeglobals.d_numpoints = 0;
	g_qeglobals.d_numedges = 0;

#ifdef NEWEDGESEL
	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (f = b->brush_faces; f; f = f->next) {
			MakeFace(b, f);
		}
	}

#else
	if (!QE_SingleBrush()) {
		return;
	}

	b = selected_brushes.next;
	for (f = b->brush_faces; f; f = f->next) {
		MakeFace(b, f);
	}
#endif
}

#ifdef NEWEDGESEL
void SelectFaceEdge (brush_t * b, face_t * f, int p1, int p2)
#else
void SelectFaceEdge (face_t * f, int p1, int p2)
#endif 
{
	idWinding	*w;
	int			i, j, k;
	int			pnum[128];

#ifdef NEWEDGESEL
	w = Brush_MakeFaceWinding(b, f);
#else
	w = Brush_MakeFaceWinding(selected_brushes.next, f);
#endif
	if (!w) {
		return;
	}
	for (i = 0; i < w->GetNumPoints(); i++) {
		pnum[i] = FindPoint( (*w)[i].ToVec3() );
	}
	for (i = 0; i < w->GetNumPoints(); i++) {
		if (pnum[i] == p1 && pnum[(i + 1) % w->GetNumPoints()] == p2) {
			VectorCopy(g_qeglobals.d_points[pnum[i]], f->planepts[0]);
			VectorCopy(g_qeglobals.d_points[pnum[(i + 1) % w->GetNumPoints()]], f->planepts[1]);
			VectorCopy(g_qeglobals.d_points[pnum[(i + 2) % w->GetNumPoints()]], f->planepts[2]);
			for (j = 0; j < 3; j++) {
				for (k = 0; k < 3; k++) {
					f->planepts[j][k] =
					floor(f->planepts[j][k] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
				}
			}
			AddPlanept(&f->planepts[0]);
			AddPlanept(&f->planepts[1]);
			break;
		}
	}
	if ( i == w->GetNumPoints() ) {
		Sys_Status("SelectFaceEdge: failed\n");
	}
	delete w;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SelectVertex(int p1) {
	brush_t		*b;
	idWinding	*w;
	int			i, j, k;
	face_t		*f;

#ifdef NEWEDGESEL
	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (f = b->brush_faces; f; f = f->next) {
			w = Brush_MakeFaceWinding(b, f);
			if (!w) {
				continue;
			}

			for (i = 0; i < w->GetNumPoints(); i++) {
				if ( FindPoint( (*w)[i].ToVec3() ) == p1 ) {
					VectorCopy((*w)[(i + w->GetNumPoints() - 1) % w->GetNumPoints()], f->planepts[0]);
					VectorCopy((*w)[i], f->planepts[1]);
					VectorCopy((*w)[(i + 1) % w->GetNumPoints()], f->planepts[2]);
					for (j = 0; j < 3; j++) {
						for (k = 0; k < 3; k++) {
							// f->planepts[j][k] = floor(f->planepts[j][k]/g_qeglobals.d_gridsize+0.5)*g_qeglobals.d_gridsize;
						}
					}

					AddPlanept(&f->planepts[1]);

					// MessageBeep(-1);
					break;
				}
			}

			delete w;
		}
	}

#else
	b = selected_brushes.next;
	for (f = b->brush_faces; f; f = f->next) {
		w = Brush_MakeFaceWinding(b, f);
		if (!w) {
			continue;
		}

		for (i = 0; i < w->GetNumPoints(); i++) {
			if (FindPoint(w[i]) == p1) {
				VectorCopy(w[(i + w->GetNumPoints() - 1) % w->GetNumPoints()], f->planepts[0]);
				VectorCopy(w[i], f->planepts[1]);
				VectorCopy(w[(i + 1) % w->GetNumPoints()], f->planepts[2]);
				for (j = 0; j < 3; j++) {
					for (k = 0; k < 3; k++) {
						// f->planepts[j][k] = floor(f->planepts[j][k]/g_qeglobals.d_gridsize+0.5)*g_qeglobals.d_gridsize;
					}
				}

				AddPlanept(&f->planepts[1]);

				// MessageBeep(-1);
				break;
			}
		}

		delete w;
	}
#endif
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SelectEdgeByRay(idVec3 org, idVec3 dir) {
	int		i, j, besti;
	float	d, bestd;
	idVec3	mid, temp;
	pedge_t *e;

	// find the edge closest to the ray
	besti = -1;
	bestd = 8;

	for (i = 0; i < g_qeglobals.d_numedges; i++) {
		for (j = 0; j < 3; j++) {
			mid[j] = 0.5 * (g_qeglobals.d_points[g_qeglobals.d_edges[i].p1][j] + g_qeglobals.d_points[g_qeglobals.d_edges[i].p2][j]);
		}

		temp = mid - org;
		d = temp * dir;
		temp = org + d * dir;
		temp = mid - temp;
		d = temp.Length();
		if ( d < bestd ) {
			bestd = d;
			besti = i;
		}
	}

	if (besti == -1) {
		Sys_Status("Click didn't hit an edge\n");
		return;
	}

	Sys_Status("hit edge\n");

	//
	// make the two faces that border the edge use the two edge points as primary drag
	// points
	//
	g_qeglobals.d_num_move_points = 0;
	e = &g_qeglobals.d_edges[besti];
#ifdef NEWEDGESEL
	for (brush_t * b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		SelectFaceEdge(b, e->f1, e->p1, e->p2);
		SelectFaceEdge(b, e->f2, e->p2, e->p1);
	}

#else
	SelectFaceEdge(e->f1, e->p1, e->p2);
	SelectFaceEdge(e->f2, e->p2, e->p1);
#endif
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SelectVertexByRay(idVec3 org, idVec3 dir) {
	int		i, besti;
	float	d, bestd;
	idVec3	temp;

	float scale = g_pParentWnd->ActiveXY()->Scale();
	// find the point closest to the ray
	besti = -1;
	bestd = 8 / scale / 2;

	for (i = 0; i < g_qeglobals.d_numpoints; i++) {
		temp = g_qeglobals.d_points[i] - org;
		d = temp * dir;
		temp = org + d * dir;
		temp = g_qeglobals.d_points[i] - temp;
		d = temp.Length();
		if ( d < bestd ) {
			bestd = d;
			besti = i;
		}
	}

	if (besti == -1 || bestd > 8 / scale / 2 ) {
		Sys_Status("Click didn't hit a vertex\n");
		return;
	}

	Sys_Status("hit vertex\n");
	g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = &g_qeglobals.d_points[besti];

	// SelectVertex (besti);
}

extern void AddPatchMovePoint(idVec3 v, bool bMulti, bool bFull);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SelectCurvePointByRay(const idVec3 &org, const idVec3 &dir, int buttons) {
	int		i, besti;
	float	d, bestd;
	idVec3	temp;

	// find the point closest to the ray
	float scale = g_pParentWnd->ActiveXY()->Scale();
	besti = -1;
	bestd = 8 / scale / 2;
	//bestd = 8;

	for (i = 0; i < g_qeglobals.d_numpoints; i++) {
		temp = g_qeglobals.d_points[i] - org;
		d = temp * dir;
		temp = org + d * dir;
		temp = g_qeglobals.d_points[i] - temp;
		d = temp.Length();
		if ( d <= bestd ) {
			bestd = d;
			besti = i;
		}
	}

	if (besti == -1) {
		if (g_pParentWnd->ActiveXY()->AreaSelectOK()) {
			g_qeglobals.d_select_mode = sel_area;
			VectorCopy(org, g_qeglobals.d_vAreaTL);
			VectorCopy(org, g_qeglobals.d_vAreaBR);
		}

		return;
	}

	// Sys_Status ("hit vertex\n");
	AddPatchMovePoint( g_qeglobals.d_points[besti], ( buttons & MK_CONTROL ) != 0, ( buttons & MK_SHIFT ) != 0 );
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SelectSplinePointByRay(const idVec3 &org, const idVec3 &dir, int buttons) {
	int		i, besti;
	float	d, bestd;
	idVec3	temp;

	// find the point closest to the ray
	besti = -1;
	bestd = 8;

	for (i = 0; i < g_qeglobals.d_numpoints; i++) {
		temp = g_qeglobals.d_points[i] - org;
		d = temp * dir;
		temp = org + d * dir;
		temp = g_qeglobals.d_points[i] - temp;
		d = temp.Length();
		if ( d <= bestd ) {
			bestd = d;
			besti = i;
		}
	}

	if (besti == -1) {
		return;
	}

	Sys_Status("hit curve point\n");
	g_qeglobals.d_num_move_points = 0;
	g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = &g_qeglobals.d_points[besti];

	// g_splineList->setSelectedPoint(&g_qeglobals.d_points[besti]);
}

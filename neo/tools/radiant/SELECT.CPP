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
#include "../../renderer/model_local.h"	// for idRenderModelPrt

// externs
CPtrArray	g_SelectedFaces;
CPtrArray	g_SelectedFaceBrushes;
CPtrArray	&g_ptrSelectedFaces = g_SelectedFaces;
CPtrArray	&g_ptrSelectedFaceBrushes = g_SelectedFaceBrushes;

extern void Brush_Resize(brush_t *b, idVec3 vMin, idVec3 vMax);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
qertrace_t Test_Ray(const idVec3 &origin, const idVec3 &dir, int flags) {
	brush_t		*brush;
	face_t		*face;
	float		dist;
	qertrace_t	t;

	memset(&t, 0, sizeof(t));
	t.dist = HUGE_DISTANCE*2;

	// check for points first
	CDragPoint	*drag = PointRay(origin, dir, &dist);
	if (drag) {
		t.dist = dist;
		t.brush = NULL;
		t.face = NULL;
		t.point = drag;
		t.selected = false;
		return t;
	}

	if (flags & SF_CYCLE) {
		CPtrArray	array;
		brush_t		*pToSelect = (selected_brushes.next != &selected_brushes) ? selected_brushes.next : NULL;
		Select_Deselect();

		// go through active brushes and accumulate all "hit" brushes
		for (brush = active_brushes.next; brush != &active_brushes; brush = brush->next) {
			// if ( (flags & SF_ENTITIES_FIRST) && brush->owner == world_entity) continue;
			if (FilterBrush(brush)) {
				continue;
			}

			if (g_PrefsDlg.m_selectOnlyBrushes) {
				if (brush->pPatch || brush->modelHandle > 0) {
					continue;
				}
			}

			if (g_PrefsDlg.m_selectNoModels) {
				if (brush->modelHandle > 0) {
					continue;
				}
			}

			// if (!g_bShowPatchBounds && brush->pPatch) continue;
			face = Brush_Ray(origin, dir, brush, &dist, true);

			if (face) {
				array.Add(brush);
			}
		}

		int nSize = array.GetSize();
		if (nSize > 0) {
			bool	bFound = false;
			for (int i = 0; i < nSize; i++) {
				brush_t *b = reinterpret_cast < brush_t * > (array.GetAt(i));

				// did we hit the last one selected yet ?
				if (b == pToSelect) {
					// yes we want to select the next one in the list
					int n = (i > 0) ? i - 1 : nSize - 1;
					pToSelect = reinterpret_cast < brush_t * > (array.GetAt(n));
					bFound = true;
					break;
				}
			}

			if (!bFound) {
				pToSelect = reinterpret_cast < brush_t * > (array.GetAt(0));
			}
		}

		if (pToSelect) {
			face = Brush_Ray(origin, dir, pToSelect, &dist, true);
			t.dist = dist;
			t.brush = pToSelect;
			t.face = face;
			t.selected = false;
			return t;
		}
	}

	if (!(flags & SF_SELECTED_ONLY)) {
		for (brush = active_brushes.next; brush != &active_brushes; brush = brush->next) {
			if ((flags & SF_ENTITIES_FIRST) && brush->owner == world_entity) {
				continue;
			}

			if (FilterBrush(brush)) {
				continue;
			}

			if (g_PrefsDlg.m_selectOnlyBrushes) {
				if (brush->pPatch || brush->modelHandle > 0) {
					continue;
				}
			}

			if (g_PrefsDlg.m_selectNoModels) {
				if (brush->modelHandle > 0) {
					continue;
				}									
			}

			face = Brush_Ray(origin, dir, brush, &dist, true);
			if (dist > 0 && dist < t.dist) {
				t.dist = dist;
				t.brush = brush;
				t.face = face;
				t.selected = false;
			}
	    }
	}

	for (brush = selected_brushes.next; brush != &selected_brushes; brush = brush->next) {
		if ((flags & SF_ENTITIES_FIRST) && brush->owner == world_entity) {
			continue;
		}

		if (FilterBrush(brush)) {
			continue;
		}

		if (g_PrefsDlg.m_selectOnlyBrushes) {
			if (brush->pPatch || brush->modelHandle > 0) {
				continue;
			}
		}

		if (g_PrefsDlg.m_selectNoModels) {
			if (brush->modelHandle > 0) {
				continue;
			}
		}

		face = Brush_Ray(origin, dir, brush, &dist, true);
		if (dist > 0 && dist < t.dist) {
			t.dist = dist;
			t.brush = brush;
			t.face = face;
			t.selected = true;
		}
	}

	// if entites first, but didn't find any, check regular
	if ((flags & SF_ENTITIES_FIRST) && t.brush == NULL) {
		return Test_Ray(origin, dir, flags - SF_ENTITIES_FIRST);
	}

	return t;
}

extern void	AddSelectablePoint(brush_t *b, idVec3 v, int type, bool priority);
extern void	ClearSelectablePoints(brush_t *b);
extern idVec3 Brush_TransformedPoint(brush_t *b, const idVec3 &in);

/*
 =======================================================================================================================
    Select_Brush
 =======================================================================================================================
 */
void Select_Brush(brush_t *brush, bool bComplete, bool bStatus) {
	brush_t		*b;
	entity_t	*e;

	g_ptrSelectedFaces.RemoveAll();
	g_ptrSelectedFaceBrushes.RemoveAll();

	// selected_face = NULL;
	if (g_qeglobals.d_select_count < MAX_MAP_ENTITIES) {
		g_qeglobals.d_select_order[g_qeglobals.d_select_count] = brush;
	}

	g_qeglobals.d_select_count++;

	e = brush->owner;
	if (e) {

		if ( e == world_entity && radiant_entityMode.GetBool() ) {
			return;
		}
		// select complete entity on first click
		if (e != world_entity && bComplete == true) {
			for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
				if (b->owner == e) {
					goto singleselect;
				}
			}

			for (b = e->brushes.onext; b != &e->brushes; b = b->onext) {
				Brush_RemoveFromList(b);
				Brush_AddToList(b, &selected_brushes);
			}
		}
		else
		{
singleselect:
			Brush_RemoveFromList(brush);
			Brush_AddToList(brush, &selected_brushes);
			UpdateSurfaceDialog();
			UpdatePatchInspector();
			UpdateLightInspector();
		}

		if (e->eclass) {
			g_Inspectors->UpdateEntitySel(brush->owner->eclass);
			if ( radiant_entityMode.GetBool() && e->eclass->nShowFlags & (ECLASS_LIGHT | ECLASS_SPEAKER) ) {
				const char *p = ValueForKey(e, "s_shader");
				if (p && *p) {
					g_Inspectors->mediaDlg.SelectCurrentItem(true, p, CDialogTextures::SOUNDS);
				}
				
			}
			if ( ( e->eclass->nShowFlags & ECLASS_LIGHT ) && !brush->entityModel ) {
				if (brush->pointLight) {
					// add center drag point if not at the origin
					if (brush->lightCenter[0] || brush->lightCenter[1] || brush->lightCenter[2]) {
						AddSelectablePoint(brush, Brush_TransformedPoint(brush, brush->lightCenter), LIGHT_CENTER, false);
					}
				}
				else {
					AddSelectablePoint(brush, Brush_TransformedPoint(brush, brush->lightTarget), LIGHT_TARGET, true);
					AddSelectablePoint(brush, Brush_TransformedPoint(brush, brush->lightUp), LIGHT_UP, false);
					AddSelectablePoint(brush, Brush_TransformedPoint(brush, brush->lightRight), LIGHT_RIGHT, false);
					if (brush->startEnd) {
						AddSelectablePoint(brush, Brush_TransformedPoint(brush, brush->lightStart), LIGHT_START, false);
						AddSelectablePoint(brush, Brush_TransformedPoint(brush, brush->lightEnd), LIGHT_END, false);
					}
				}
                UpdateLightInspector();
			}
			if (e->eclass->nShowFlags & ECLASS_CAMERAVIEW) {
				g_pParentWnd->GetCamera()->UpdateCameraView();
			}
		}
	}

	if (bStatus) {
		idVec3	vMin, vMax, vSize;
		Select_GetBounds(vMin, vMax);
		VectorSubtract(vMax, vMin, vSize);

		CString strStatus;
		strStatus.Format("Selection X:: %.1f  Y:: %.1f  Z:: %.1f", vSize[0], vSize[1], vSize[2]);
		g_pParentWnd->SetStatusText(2, strStatus);
	}
}

/*
 =======================================================================================================================
    Select_Ray If the origin is inside a brush, that brush will be ignored.
 =======================================================================================================================
 */
void Select_Ray(idVec3 origin, idVec3 dir, int flags) {
	qertrace_t	t;

	t = Test_Ray(origin, dir, flags);

	if (!t.brush) {
		return;
	}

	if (flags == SF_SINGLEFACE) {
		int		nCount = g_SelectedFaces.GetSize();
		bool	bOk = true;
		for (int i = 0; i < nCount; i++) {
			if (t.face == reinterpret_cast < face_t * > (g_SelectedFaces.GetAt(i))) {
				bOk = false;

				// need to move remove i'th entry
				g_SelectedFaces.RemoveAt(i, 1);
				g_SelectedFaceBrushes.RemoveAt(i, 1);
				nCount--;
			}
		}

		if (bOk) {
			if ( t.selected ) {
				face_t *face;

				// DeSelect brush
				Brush_RemoveFromList(t.brush);
				Brush_AddToList(t.brush, &active_brushes);

				// Select all brush faces
				for ( face = t.brush->brush_faces; face; face = face->next ) {
					//Don't add face that was clicked
					if ( face != t.face ) {
						g_SelectedFaces.Add( face );
						g_SelectedFaceBrushes.Add( t.brush );
					}
				}
			} else {
				g_SelectedFaces.Add(t.face);
				g_SelectedFaceBrushes.Add(t.brush);
			}
		}

		// selected_face = t.face; selected_face_brush = t.brush;
		Sys_UpdateWindows(W_ALL);
		g_qeglobals.d_select_mode = sel_brush;


		//common->Printf("before\n");
		//extern void Face_Info_BrushPrimit(face_t *face);
		//Face_Info_BrushPrimit(t.face);
		//common->Printf("after\n");

		//
		// Texture_SetTexture requires a brushprimit_texdef fitted to the default width=2
		// height=2 texture
		//
		brushprimit_texdef_t	brushprimit_texdef;
		ConvertTexMatWithQTexture(&t.face->brushprimit_texdef, t.face->d_texture, &brushprimit_texdef, NULL);
		Texture_SetTexture(&t.face->texdef, &brushprimit_texdef, false, false);
		UpdateSurfaceDialog();


		return;
	}

	// move the brush to the other list
	g_qeglobals.d_select_mode = sel_brush;

	if (t.selected) {
		Brush_RemoveFromList(t.brush);
		Brush_AddToList(t.brush, &active_brushes);
		UpdatePatchInspector();
		UpdateSurfaceDialog();

		entity_t	*e = t.brush->owner;
		if (e->eclass->nShowFlags & ECLASS_LIGHT && !t.brush->entityModel) {
			if (t.brush->pointLight) {
			}
			else {
				ClearSelectablePoints(t.brush);
			}
		}
	}
	else {
		Select_Brush(t.brush, !(GetAsyncKeyState(VK_MENU) & 0x8000));
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_Delete(void) {
	brush_t *brush;

	g_ptrSelectedFaces.RemoveAll();
	g_ptrSelectedFaceBrushes.RemoveAll();

	// selected_face = NULL;
	g_qeglobals.d_select_mode = sel_brush;

	g_qeglobals.d_select_count = 0;
	g_qeglobals.d_num_move_points = 0;
	while (selected_brushes.next != &selected_brushes) {
		brush = selected_brushes.next;
		if (brush->pPatch) {
			// Patch_Delete(brush->nPatchID);
			Patch_Delete(brush->pPatch);
		}

		Brush_Free(brush);
	}

	// FIXME: remove any entities with no brushes
	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_Deselect(bool bDeselectFaces) {
	brush_t *b;

	ClearSelectablePoints(NULL);
	Patch_Deselect();

	g_pParentWnd->ActiveXY()->UndoClear();

	g_qeglobals.d_workcount++;
	g_qeglobals.d_select_count = 0;
	g_qeglobals.d_num_move_points = 0;
	b = selected_brushes.next;

	if (b == &selected_brushes) {
		if (bDeselectFaces) {
			g_ptrSelectedFaces.RemoveAll();
			g_ptrSelectedFaceBrushes.RemoveAll();

			// selected_face = NULL;
		}

		Sys_UpdateWindows(W_ALL);
		return;
	}

	if (bDeselectFaces) {
		g_ptrSelectedFaces.RemoveAll();
		g_ptrSelectedFaceBrushes.RemoveAll();

		// selected_face = NULL;
	}

	g_qeglobals.d_select_mode = sel_brush;

	// grab top / bottom height for new brushes
	if (b->mins[2] < b->maxs[2]) {
		g_qeglobals.d_new_brush_bottom = b->mins;
		g_qeglobals.d_new_brush_top = b->maxs;
	}

	selected_brushes.next->prev = &active_brushes;
	selected_brushes.prev->next = active_brushes.next;
	active_brushes.next->prev = selected_brushes.prev;
	active_brushes.next = selected_brushes.next;
	selected_brushes.prev = selected_brushes.next = &selected_brushes;

	g_pParentWnd->GetCamera()->UpdateCameraView();

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
    Select_Move
 =======================================================================================================================
 */
void Select_Move(idVec3 delta, bool bSnap) {
	brush_t		*b;

	// actually move the selected brushes
	bool		updateOrigin = true;
	entity_t	*lastOwner = selected_brushes.next->owner;

	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		Brush_Move(b, delta, bSnap, updateOrigin);
		if (updateOrigin) {
			updateOrigin = false;
		}

		if (b->next->owner != lastOwner) {
			updateOrigin = true;
			lastOwner = b->next->owner;
		}
	}

	idVec3	vMin, vMax;
	Select_GetBounds(vMin, vMax);

	CString strStatus;
	strStatus.Format("Origin X:: %.1f  Y:: %.1f  Z:: %.1f", vMin[0], vMax[1], vMax[2]);
	g_pParentWnd->SetStatusText(2, strStatus);
	g_pParentWnd->GetCamera()->UpdateCameraView();

	// Sys_UpdateWindows (W_ALL);
}

/*
 =======================================================================================================================
    Select_Clone Creates an exact duplicate of the selection in place, then moves the selected brushes off of their old
    positions
 =======================================================================================================================
 */
void Select_Clone(void) {
	ASSERT(g_pParentWnd->ActiveXY());
	g_bScreenUpdates = false;
	g_pParentWnd->ActiveXY()->Copy();
	g_pParentWnd->ActiveXY()->Paste();
	g_pParentWnd->NudgeSelection(2, g_qeglobals.d_gridsize);
	g_pParentWnd->NudgeSelection(3, g_qeglobals.d_gridsize);
	g_bScreenUpdates = true;
	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
    Select_SetTexture Timo:: bFitScale to compute scale on the plane and counteract plane / axial plane snapping Timo::
    brush primitive texturing the brushprimit_texdef given must be understood as a qtexture_t width=2 height=2 ( HiRes
    ) Timo:: texture plugin, added an IPluginTexdef* parameter must be casted to an IPluginTexdef! if not NULL, get
    ->Copy() of it into each face or brush ( and remember to hook ) if NULL, means we have no information, ask for a
    default
 =======================================================================================================================
 */
void WINAPI Select_SetTexture(texdef_t *texdef,brushprimit_texdef_t	*brushprimit_texdef,bool bFitScale,void *pPlugTexdef,bool update) {
	brush_t *b;
	int		nCount = g_ptrSelectedFaces.GetSize();
	if (nCount > 0) {
		Undo_Start("set face textures");
		ASSERT(g_ptrSelectedFaces.GetSize() == g_ptrSelectedFaceBrushes.GetSize());
		for (int i = 0; i < nCount; i++) {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(i));
			brush_t *selBrush = reinterpret_cast < brush_t * > (g_ptrSelectedFaceBrushes.GetAt(i));
			Undo_AddBrush(selBrush);
 			SetFaceTexdef(selBrush,selFace,texdef,brushprimit_texdef,bFitScale);
			Brush_Build(selBrush, bFitScale);
			Undo_EndBrush(selBrush);
		}

		Undo_End();
	}
	else if (selected_brushes.next != &selected_brushes) {
		Undo_Start("set brush textures");
		for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
			if (!b->owner->eclass->fixedsize) {
				Undo_AddBrush(b);
				Brush_SetTexture(b, texdef, brushprimit_texdef, bFitScale);
				Undo_EndBrush(b);
			} else if (b->owner->eclass->nShowFlags & ECLASS_LIGHT) {
				if ( idStr::Cmpn(texdef->name, "lights/", strlen("lights/")) == 0 ) {
					SetKeyValue(b->owner, "texture", texdef->name);
					g_Inspectors->UpdateEntitySel(b->owner->eclass);
					UpdateLightInspector();
					Brush_Build(b);
				} else {
					Undo_AddBrush(b);
					Brush_SetTexture(b, texdef, brushprimit_texdef, bFitScale);
					Undo_EndBrush(b);
				}
			}
		}

		Undo_End();
	}

	if (update) {
		Sys_UpdateWindows(W_ALL);
	}
}

/*
 =======================================================================================================================
    TRANSFORMATIONS
 =======================================================================================================================
 */
void Select_GetBounds(idVec3 &mins, idVec3 &maxs) {
	brush_t *b;
	int		i;

	for (i = 0; i < 3; i++) {
		mins[i] = 999999;
		maxs[i] = -999999;
	}

	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (i = 0; i < 3; i++) {
			if (b->mins[i] < mins[i]) {
				mins[i] = b->mins[i];
			}

			if (b->maxs[i] > maxs[i]) {
				maxs[i] = b->maxs[i];
			}
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_GetTrueMid(idVec3 &mid) {
	idVec3	mins, maxs;
	Select_GetBounds(mins, maxs);

	for (int i = 0; i < 3; i++) {
		mid[i] = (mins[i] + ((maxs[i] - mins[i]) / 2));
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_GetMid(idVec3 &mid) {
#if 0
	Select_GetTrueMid(mid);
	return;
#else
	idVec3	mins, maxs;
	int		i;

	//if (g_PrefsDlg.m_bNoClamp) {
	//	Select_GetTrueMid(mid);
	//	return;
	//}

	Select_GetBounds(mins, maxs);

	for (i = 0; i < 3; i++) {
		mid[i] = g_qeglobals.d_gridsize * floor(((mins[i] + maxs[i]) * 0.5) / g_qeglobals.d_gridsize);
	}
#endif
}

idVec3	select_origin;
idMat3	select_matrix;
idMat3	select_bmatrix;
idRotation select_rotation;
bool	select_fliporder;
int		select_flipAxis;
float	select_orgDeg;

void Select_InitializeRotation() {
	for (brush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (face_t *f = b->brush_faces; f; f = f->next) {
			for (int i = 0; i < 3; i++) {
				f->orgplanepts[i] = f->planepts[i];
			}
		}
	}
	select_orgDeg = 0.0;
}

void Select_FinalizeRotation() {

}

bool Select_OnlyModelsSelected() {
	for (brush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		if (!b->modelHandle) {
			return false;
		}
	}
	return true;
}

bool OkForRotationKey(brush_t *b) {
	if (b->owner->eclass->nShowFlags & ECLASS_WORLDSPAWN) {
		return false;
	}

	if (stricmp(b->owner->epairs.GetString("name"), b->owner->epairs.GetString("model")) == 0) {
		return false;
	}

	return true;
}

/*
=================
VectorRotate3

	rotation order is roll - pitch - yaw
=================
*/
void VectorRotate3( const idVec3 &vIn, const idVec3 &vRotation, idVec3 &out) {
#if 1
	int i, nIndex[3][2];
	idVec3 vWork, va;

	va = vIn;
	vWork = va;
	nIndex[0][0] = 1; nIndex[0][1] = 2;
	nIndex[1][0] = 2; nIndex[1][1] = 0;
	nIndex[2][0] = 0; nIndex[2][1] = 1;

	for (i = 0; i < 3; i++) {
		if ( vRotation[i] != 0.0f ) {
			double dAngle = DEG2RAD( vRotation[i] );
			double c = cos( dAngle );
			double s = sin( dAngle );
			vWork[nIndex[i][0]] = va[nIndex[i][0]] * c - va[nIndex[i][1]] * s;
			vWork[nIndex[i][1]] = va[nIndex[i][0]] * s + va[nIndex[i][1]] * c;
		}
		va = vWork;
	}
	out = vWork;
#else
	idAngles angles;

	angles.pitch = vRotation[1];
	angles.yaw = vRotation[2];
	angles.roll = vRotation[0];

	out = vIn * angles.ToMat3();
#endif
}

/*
=================
VectorRotate3Origin
=================
*/
void VectorRotate3Origin( const idVec3 &vIn, const idVec3 &vRotation, const idVec3 &vOrigin, idVec3 &out ) {
	out = vIn - vOrigin;
	VectorRotate3( out, vRotation, out );
	out += vOrigin;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
extern void Brush_Rotate(brush_t *b, idMat3 matrix, idVec3 origin, bool bBuild);

void Select_ApplyMatrix(bool bSnap, bool rotateOrigins) {
	brush_t *b;
	face_t	*f;
	int		i;
	idVec3	temp;
	idStr str;
	char	text[128];
	entity_t *lastOwner = NULL;

	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		
		bool doBrush = true;
		if (!(b->owner->eclass->nShowFlags & ECLASS_WORLDSPAWN) && b->owner != lastOwner) {
			if (b->modelHandle || b->owner->eclass->nShowFlags & ECLASS_ROTATABLE) {
				if (rotateOrigins) {
					b->owner->rotation *= select_matrix;
					b->owner->origin *= select_rotation;
					SetKeyVec3(b->owner, "origin", b->owner->origin);
					if (b->trackLightOrigin) {
						b->owner->lightRotation *= select_matrix;
						b->owner->lightOrigin *= select_rotation;
						SetKeyVec3(b->owner, "light_origin", b->owner->lightOrigin);
					}
				} else {

					b->owner->rotation *= select_matrix;
					if ( select_fliporder ) {
						if ( select_flipAxis == 0 ) {
							temp = b->owner->rotation[1];
							b->owner->rotation[1] = b->owner->rotation[2];
							b->owner->rotation[2] = temp;
						} else if ( select_flipAxis == 1 ) {
							temp = b->owner->rotation[0];
							b->owner->rotation[0] = b->owner->rotation[1];
							b->owner->rotation[1] = temp;
						} else {
							temp = b->owner->rotation[0];
							b->owner->rotation[0] = b->owner->rotation[2];
							b->owner->rotation[2] = temp;
						}
					}

					if (b->trackLightOrigin) {
						b->owner->lightRotation = select_matrix * b->owner->lightRotation;
					}
				}
				b->owner->rotation.OrthoNormalizeSelf();
				b->owner->lightRotation.OrthoNormalizeSelf();

				if (b->modelHandle) {
					idBounds bo, bo2;
					bo2.Zero();
					if ( dynamic_cast<idRenderModelPrt*>( b->modelHandle ) || dynamic_cast<idRenderModelLiquid*>( b->modelHandle ) ) {
						bo2.ExpandSelf( 12.0f );
					} else {
						bo2 = b->modelHandle->Bounds();
					}
					bo.FromTransformedBounds(bo2, b->owner->origin, b->owner->rotation);
					Brush_Resize(b, bo[0], bo[1]);
					doBrush = false;
				} 
				if (b->owner->eclass->fixedsize) {
					doBrush = false;
				}
			} else if (b->owner->eclass->fixedsize && !rotateOrigins) {
				doBrush = false;
			} else {
				b->owner->origin -= select_origin;
				b->owner->origin *= select_matrix;
				b->owner->origin += select_origin;
				sprintf(text, "%i %i %i", (int)b->owner->origin[0], (int)b->owner->origin[1], (int)b->owner->origin[2]);

				SetKeyValue(b->owner, "origin", text);
			}
			
		   

			if (OkForRotationKey(b)) {
				sprintf(str, "%g %g %g %g %g %g %g %g %g",b->owner->rotation[0][0],b->owner->rotation[0][1],b->owner->rotation[0][2],
						b->owner->rotation[1][0],b->owner->rotation[1][1],b->owner->rotation[1][2],b->owner->rotation[2][0],
						b->owner->rotation[2][1],b->owner->rotation[2][2]);
				SetKeyValue(b->owner, "rotation", str);
			}
			
			if (b->trackLightOrigin) {
				sprintf(str, "%g %g %g %g %g %g %g %g %g",b->owner->lightRotation[0][0],b->owner->lightRotation[0][1],b->owner->lightRotation[0][2],
						b->owner->lightRotation[1][0],b->owner->lightRotation[1][1],b->owner->lightRotation[1][2],b->owner->lightRotation[2][0],
						b->owner->lightRotation[2][1],b->owner->lightRotation[2][2]);
				SetKeyValue(b->owner, "light_rotation", str);
			}
			DeleteKey(b->owner, "angle");
			DeleteKey(b->owner, "angles");
		}

		if (doBrush) {
			for (f = b->brush_faces; f; f = f->next) {
				for (i = 0; i < 3; i++) {
					f->planepts[i] = ( ((g_bRotateMode) ? f->orgplanepts[i] : f->planepts[i]) - select_origin ) * ((g_bRotateMode) ? select_bmatrix : select_matrix) + select_origin; 
				}

				if ( select_fliporder ) {
					VectorCopy(f->planepts[0], temp);
					VectorCopy(f->planepts[2], f->planepts[0]);
					VectorCopy(temp, f->planepts[2]);
				}
			}
		}

		if (b->owner->eclass->fixedsize && b->owner->eclass->entityModel == NULL) {
			idVec3 min, max;
			if (b->trackLightOrigin) {
				min = b->owner->lightOrigin + b->owner->eclass->mins;
				max = b->owner->lightOrigin + b->owner->eclass->maxs;
			} else {
				min = b->owner->origin + b->owner->eclass->mins;
				max = b->owner->origin + b->owner->eclass->maxs;
			}
			Brush_Resize(b, min, max);
		} else {
			Brush_Build(b, bSnap);
		}

		if (b->pPatch) {
			Patch_ApplyMatrix(b->pPatch, select_origin, select_matrix, bSnap);
		}

		if ( b->owner->curve ) {
			int c = b->owner->curve->GetNumValues();
			for ( i = 0; i < c; i++ ) {
				idVec3 v = b->owner->curve->GetValue( i );
				v -= select_origin;
				v *= select_matrix;
				v += select_origin;
				b->owner->curve->SetValue( i, v );
			}
		}

		lastOwner = b->owner;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RotateFaceTexture(face_t *f, int nAxis, float fDeg) {
	idVec3	p1, p2, p3, rota;
	p1[0] = p1[1] = p1[2] = 0;
	VectorCopy(p1, p2);
	VectorCopy(p1, p3);
	VectorCopy(p1, rota);
	ComputeAbsolute(f, p1, p2, p3);

	rota[nAxis] = fDeg;
	VectorRotate3Origin(p1, rota, select_origin, p1);
	VectorRotate3Origin(p2, rota, select_origin, p2);
	VectorRotate3Origin(p3, rota, select_origin, p3);

	idPlane		normal2;
	idVec3		vNormal;
	vNormal[0] = f->plane[0];
	vNormal[1] = f->plane[1];
	vNormal[2] = f->plane[2];
	VectorRotate3(vNormal, rota, vNormal);
	normal2[0] = vNormal[0];
	normal2[1] = vNormal[1];
	normal2[2] = vNormal[2];
	AbsoluteToLocal(normal2, f, p1, p2, p3);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RotateTextures(int nAxis, float fDeg, idVec3 vOrigin) {
	for (brush_t * b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (face_t * f = b->brush_faces; f; f = f->next) {
			if (g_qeglobals.m_bBrushPrimitMode) {
				RotateFaceTexture_BrushPrimit(f, nAxis, fDeg, vOrigin);
			}
			else {
				RotateFaceTexture(f, nAxis, fDeg);
			}

			// ++timo removed that call .. works fine .. ??????? Brush_Build(b, false);
		}

		Brush_Build(b, false);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_ApplyMatrix_BrushPrimit() {
	for (brush_t * b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (face_t * f = b->brush_faces; f; f = f->next) {
			ApplyMatrix_BrushPrimit(f, select_matrix, select_origin);
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_RotateAxis(int axis, float deg, bool bPaint, bool bMouse) {
	idVec3	temp;

	if (deg == 0) {
		return;
	}

	if (bMouse) {
		if (g_qeglobals.flatRotation == 2) {
			Select_GetTrueMid(select_origin);
		} else {
			VectorCopy(g_pParentWnd->ActiveXY()->RotateOrigin(), select_origin);
		}
	} else {
		Select_GetMid(select_origin);
	}

	select_fliporder = false;

	idVec3 vec = vec3_origin;
	vec[axis] = 1.0f;

	if (g_bRotateMode) {
		select_orgDeg += deg;
	}

	select_rotation.Set( select_origin, vec, deg );
	select_matrix = select_rotation.ToMat3();
	idRotation rot(select_origin, vec, select_orgDeg);
	rot.Normalize360();
	select_bmatrix = rot.ToMat3();


	if (g_PrefsDlg.m_bRotateLock) {
		select_matrix.TransposeSelf();
		Select_ApplyMatrix_BrushPrimit();
		//RotateTextures(axis, -deg, select_origin);
	}

	select_matrix.TransposeSelf();
	Select_ApplyMatrix( !bMouse, ( g_qeglobals.flatRotation != 0 ) );

	if (bPaint) {
		Sys_UpdateWindows(W_ALL);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ProjectOnPlane( const idVec3 &normal, float dist, idVec3 &ez, idVec3 &p) {
	if (idMath::Fabs(ez[0]) == 1) {
		p[0] = (dist - normal[1] * p[1] - normal[2] * p[2]) / normal[0];
	}
	else if (idMath::Fabs(ez[1]) == 1) {
		p[1] = (dist - normal[0] * p[0] - normal[2] * p[2]) / normal[1];
	}
	else {
		p[2] = (dist - normal[0] * p[0] - normal[1] * p[1]) / normal[2];
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Back(idVec3 &dir, idVec3 &p) {
	if (idMath::Fabs(dir[0]) == 1) {
		p[0] = 0;
	}
	else if (idMath::Fabs(dir[1]) == 1) {
		p[1] = 0;
	}
	else {
		p[2] = 0;
	}
}

//
// =======================================================================================================================
//    using scale[0] and scale[1]
// =======================================================================================================================
//
void ComputeScale(idVec3 &rex, idVec3 &rey, idVec3 &p, face_t *f) {
	float	px = DotProduct(rex, p);
	float	py = DotProduct(rey, p);
	px *= f->texdef.scale[0];
	py *= f->texdef.scale[1];

	idVec3	aux;
	VectorCopy(rex, aux);
	VectorScale(aux, px, aux);
	VectorCopy(aux, p);
	VectorCopy(rey, aux);
	VectorScale(aux, py, aux);
	VectorAdd(p, aux, p);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ComputeAbsolute(face_t *f, idVec3 &p1, idVec3 &p2, idVec3 &p3) {
	idVec3	ex, ey, ez; // local axis base

#ifdef _DEBUG
	if (g_qeglobals.m_bBrushPrimitMode) {
		common->Printf("Warning : illegal call of ComputeAbsolute in brush primitive mode\n");
	}
#endif
	// compute first local axis base
	TextureAxisFromPlane( f->plane, ex, ey );
	ez = ex.Cross( ey );

	idVec3	aux;
	VectorCopy(ex, aux);
	VectorScale(aux, -f->texdef.shift[0], aux);
	VectorCopy(aux, p1);
	VectorCopy(ey, aux);
	VectorScale(aux, -f->texdef.shift[1], aux);
	VectorAdd(p1, aux, p1);
	VectorCopy(p1, p2);
	VectorAdd(p2, ex, p2);
	VectorCopy(p1, p3);
	VectorAdd(p3, ey, p3);
	VectorCopy(ez, aux);
	VectorScale(aux, -f->texdef.rotate, aux);
	VectorRotate3(p1, aux, p1);
	VectorRotate3(p2, aux, p2);
	VectorRotate3(p3, aux, p3);

	// computing rotated local axis base
	idVec3	rex, rey;
	VectorCopy(ex, rex);
	VectorRotate3(rex, aux, rex);
	VectorCopy(ey, rey);
	VectorRotate3(rey, aux, rey);

	ComputeScale(rex, rey, p1, f);
	ComputeScale(rex, rey, p2, f);
	ComputeScale(rex, rey, p3, f);

	// project on normal plane along ez assumes plane normal is normalized
	ProjectOnPlane(f->plane.Normal(), -f->plane[3], ez, p1);
	ProjectOnPlane(f->plane.Normal(), -f->plane[3], ez, p2);
	ProjectOnPlane(f->plane.Normal(), -f->plane[3], ez, p3);
};

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AbsoluteToLocal( const idPlane &normal2, face_t *f, idVec3 &p1, idVec3 &p2, idVec3 &p3) {
	idVec3	ex, ey, ez;

#ifdef _DEBUG
	if (g_qeglobals.m_bBrushPrimitMode) {
		common->Printf("Warning : illegal call of AbsoluteToLocal in brush primitive mode\n");
	}
#endif
	// computing new local axis base
	TextureAxisFromPlane( normal2, ex, ey );
	ez = ex.Cross( ey );

	// projecting back on (ex,ey)
	Back(ez, p1);
	Back(ez, p2);
	Back(ez, p3);

	idVec3	aux;

	// rotation
	VectorCopy(p2, aux);
	VectorSubtract(aux, p1, aux);

	float	x = DotProduct(aux, ex);
	float	y = DotProduct(aux, ey);
	f->texdef.rotate = RAD2DEG( atan2(y, x) );

	idVec3	rex, rey;

	// computing rotated local axis base
	VectorCopy(ez, aux);
	VectorScale(aux, f->texdef.rotate, aux);
	VectorCopy(ex, rex);
	VectorRotate3(rex, aux, rex);
	VectorCopy(ey, rey);
	VectorRotate3(rey, aux, rey);

	// scale
	VectorCopy(p2, aux);
	VectorSubtract(aux, p1, aux);
	f->texdef.scale[0] = DotProduct(aux, rex);
	VectorCopy(p3, aux);
	VectorSubtract(aux, p1, aux);
	f->texdef.scale[1] = DotProduct(aux, rey);

	// shift only using p1
	x = DotProduct(rex, p1);
	y = DotProduct(rey, p1);
	x /= f->texdef.scale[0];
	y /= f->texdef.scale[1];

	VectorCopy(rex, p1);
	VectorScale(p1, x, p1);
	VectorCopy(rey, aux);
	VectorScale(aux, y, aux);
	VectorAdd(p1, aux, p1);
	VectorCopy(ez, aux);
	VectorScale(aux, -f->texdef.rotate, aux);
	VectorRotate3(p1, aux, p1);
	f->texdef.shift[0] = -DotProduct(p1, ex);
	f->texdef.shift[1] = -DotProduct(p1, ey);

	// stored rot is good considering local axis base change it if necessary
	f->texdef.rotate = -f->texdef.rotate;

	Clamp(f->texdef.shift[0], f->d_texture->GetEditorImage()->uploadWidth);
	Clamp(f->texdef.shift[1], f->d_texture->GetEditorImage()->uploadHeight);
	Clamp(f->texdef.rotate, 360);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_FlipAxis(int axis) {

	Select_GetMid( select_origin );
	
	for ( int i = 0; i < 3; i++) {
		VectorCopy(vec3_origin, select_matrix[i]);
		select_matrix[i][i] = 1;
	}

	select_matrix[axis][axis] = -1;

	select_matrix.Identity();
	select_matrix[axis][axis] = -1;

	select_fliporder = true;
	select_flipAxis = axis;

	// texture locking
	if (g_PrefsDlg.m_bRotateLock) {
		//
		// axis flipping inverts space orientation, we have to use a general texture
		// locking algorithm instead of the RotateFaceTexture
		//
		if (g_qeglobals.m_bBrushPrimitMode) {
			Select_ApplyMatrix_BrushPrimit();
		}
		else {
			//
			// there's never been flip locking for non BP mode, this would be tricky to write
			// and there's not much interest for it with the coming of BP format what could be
			// done is converting regular to BP, locking, then back to regular :)
			// Sys_FPrintf(SYS_WRN, "WARNING: regular texturing doesn't have texture lock on
			// flipping operations\n");
			//
		}
	}

	// geometric transformation
	Select_ApplyMatrix(true, false);
	Sys_UpdateWindows(W_ALL);}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_Scale(float x, float y, float z) {
	Select_GetMid(select_origin);
	for (brush_t * b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (face_t * f = b->brush_faces; f; f = f->next) {
			for (int i = 0; i < 3; i++) {
				f->planepts[i][0] -= select_origin[0];
				f->planepts[i][1] -= select_origin[1];
				f->planepts[i][2] -= select_origin[2];
				f->planepts[i][0] *= x;

				//
				// f->planepts[i][0] = floor(f->planepts[i][0] / g_qeglobals.d_gridsize + 0.5) *
				// g_qeglobals.d_gridsize;
				//
				f->planepts[i][1] *= y;

				//
				// f->planepts[i][1] = floor(f->planepts[i][1] / g_qeglobals.d_gridsize + 0.5) *
				// g_qeglobals.d_gridsize;
				//
				f->planepts[i][2] *= z;

				//
				// f->planepts[i][2] = floor(f->planepts[i][2] / g_qeglobals.d_gridsize + 0.5) *
				// g_qeglobals.d_gridsize;
				//
				f->planepts[i][0] += select_origin[0];
				f->planepts[i][1] += select_origin[1];
				f->planepts[i][2] += select_origin[2];
			}
		}

		Brush_Build(b, false);
		if (b->pPatch) {
			idVec3	v;
			v[0] = x;
			v[1] = y;
			v[2] = z;

			// Patch_Scale(b->nPatchID, select_origin, v);
			Patch_Scale(b->pPatch, select_origin, v);
		}
	}
}

/*
 =======================================================================================================================
    GROUP SELECTIONS
 =======================================================================================================================
 */
void Select_CompleteTall(void) {
	brush_t *b, *next;

	// int i;
	idVec3	mins, maxs;

	if (!QE_SingleBrush()) {
		return;
	}

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy(selected_brushes.next->mins, mins);
	VectorCopy(selected_brushes.next->maxs, maxs);
	Select_Delete();

	int nDim1 = (g_pParentWnd->ActiveXY()->GetViewType() == YZ) ? 1 : 0;
	int nDim2 = (g_pParentWnd->ActiveXY()->GetViewType() == XY) ? 1 : 2;

	for (b = active_brushes.next; b != &active_brushes; b = next) {
		next = b->next;

		if ((b->maxs[nDim1] > maxs[nDim1] || b->mins[nDim1] < mins[nDim1]) || (b->maxs[nDim2] > maxs[nDim2] || b->mins[nDim2] < mins[nDim2])) {
			if (!(b->owner->origin[nDim1] > mins[nDim1] && b->owner->origin[nDim1] < maxs[nDim1] && b->owner->origin[nDim2] > mins[nDim2] && b->owner->origin[nDim2] < maxs[nDim2])) {
				continue;
			}
			if (b->owner->eclass->nShowFlags & ECLASS_WORLDSPAWN) {
				continue;
			}
		}

		if (FilterBrush(b)) {
			continue;
		}

		Brush_RemoveFromList(b);
		Brush_AddToList(b, &selected_brushes);
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_PartialTall(void) {
	brush_t *b, *next;

	// int i;
	idVec3	mins, maxs;

	if (!QE_SingleBrush()) {
		return;
	}

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy(selected_brushes.next->mins, mins);
	VectorCopy(selected_brushes.next->maxs, maxs);
	Select_Delete();

	int nDim1 = (g_pParentWnd->ActiveXY()->GetViewType() == YZ) ? 1 : 0;
	int nDim2 = (g_pParentWnd->ActiveXY()->GetViewType() == XY) ? 1 : 2;

	for (b = active_brushes.next; b != &active_brushes; b = next) {
		next = b->next;

		if
		(
			(b->mins[nDim1] > maxs[nDim1] || b->maxs[nDim1] < mins[nDim1]) ||
			(b->mins[nDim2] > maxs[nDim2] || b->maxs[nDim2] < mins[nDim2])
		) {
			continue;
		}

		if (FilterBrush(b)) {
			continue;
		}

		Brush_RemoveFromList(b);
		Brush_AddToList(b, &selected_brushes);

#if 0
		// old stuff
		for (i = 0; i < 2; i++) {
			if (b->mins[i] > maxs[i] || b->maxs[i] < mins[i]) {
				break;
			}
		}

		if (i == 2) {
			Brush_RemoveFromList(b);
			Brush_AddToList(b, &selected_brushes);
		}
#endif
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_Touching(void) {
	brush_t *b, *next;
	int		i;
	idVec3	mins, maxs;

	if (!QE_SingleBrush()) {
		return;
	}

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy(selected_brushes.next->mins, mins);
	VectorCopy(selected_brushes.next->maxs, maxs);

	for (b = active_brushes.next; b != &active_brushes; b = next) {
		next = b->next;

		if (FilterBrush(b)) {
			continue;
		}

		for (i = 0; i < 3; i++) {
			if (b->mins[i] > maxs[i] + 1 || b->maxs[i] < mins[i] - 1) {
				break;
			}
		}

		if (i == 3) {
			Brush_RemoveFromList(b);
			Brush_AddToList(b, &selected_brushes);
		}
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_Inside(void) {
	brush_t *b, *next;
	int		i;
	idVec3	mins, maxs;

	if (!QE_SingleBrush()) {
		return;
	}

	g_qeglobals.d_select_mode = sel_brush;

	VectorCopy(selected_brushes.next->mins, mins);
	VectorCopy(selected_brushes.next->maxs, maxs);
	Select_Delete();

	for (b = active_brushes.next; b != &active_brushes; b = next) {
		next = b->next;

		if (FilterBrush(b)) {
			continue;
		}

		for (i = 0; i < 3; i++) {
			if (b->maxs[i] > maxs[i] || b->mins[i] < mins[i]) {
				break;
			}
		}

		if (i == 3) {
			Brush_RemoveFromList(b);
			Brush_AddToList(b, &selected_brushes);
		}
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
    Select_Ungroup Turn the currently selected entity back into normal brushes
 =======================================================================================================================
 */
void Select_Ungroup() {
	int			numselectedgroups;
	entity_t	*e;
	brush_t		*b, *sb;

	numselectedgroups = 0;
	for (sb = selected_brushes.next; sb != &selected_brushes; sb = sb->next) {
		e = sb->owner;

		if (!e || e == world_entity) {
			continue;
		}

		for (b = e->brushes.onext; b != &e->brushes; b = e->brushes.onext) {
			Entity_UnlinkBrush(b);
			Entity_LinkBrush(world_entity, b);
			Brush_Build(b);
			b->owner = world_entity;
		}

		Entity_Free(e);
		numselectedgroups++;
	}

	if (numselectedgroups <= 0) {
		Sys_Status("No grouped entities selected.\n");
		return;
	}

	common->Printf("Ungrouped %d entit%s.\n", numselectedgroups, (numselectedgroups == 1) ? "y" : "ies");
	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_ShiftTexture(float x, float y, bool autoAdjust) {
	brush_t *b;
	face_t	*f;

	int		nFaceCount = g_ptrSelectedFaces.GetSize();

	if (selected_brushes.next == &selected_brushes && nFaceCount == 0) {
		return;
	}

	x = -x;

	Undo_Start("Select shift textures");
	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (f = b->brush_faces; f; f = f->next) {
			if (g_qeglobals.m_bBrushPrimitMode) {
				// use face normal to compute a true translation
				Select_ShiftTexture_BrushPrimit(f, x, y, autoAdjust);
			}
			else {
				f->texdef.shift[0] += x;
				f->texdef.shift[1] += y;
			}
		}

		Brush_Build(b);
		if (b->pPatch) {
			// Patch_ShiftTexture(b->nPatchID, x, y);
			Patch_ShiftTexture(b->pPatch, x, y, autoAdjust);
		}
	}

	if (nFaceCount > 0) {
		for (int i = 0; i < nFaceCount; i++) {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(i));
			brush_t *selBrush = reinterpret_cast < brush_t * > (g_ptrSelectedFaceBrushes.GetAt(i));
			if (g_qeglobals.m_bBrushPrimitMode) {
				//
				// use face normal to compute a true translation Select_ShiftTexture_BrushPrimit(
				// selected_face, x, y ); use camera view to compute texture shift
				//
				Select_ShiftTexture_BrushPrimit(selFace, x, y, autoAdjust);
			}
			else {
				selFace->texdef.shift[0] += x;
				selFace->texdef.shift[1] += y;
			}

			Brush_Build(selBrush);
		}
	}

	Undo_End();
	Sys_UpdateWindows(W_CAMERA);
}

extern void	Face_SetExplicitScale_BrushPrimit(face_t *face, float s, float t);
extern void Face_ScaleTexture_BrushPrimit(face_t *face, float sS, float sT);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_ScaleTexture(float x, float y, bool update, bool absolute) {
	brush_t *b;
	face_t	*f;

	int		nFaceCount = g_ptrSelectedFaces.GetSize();

	if (selected_brushes.next == &selected_brushes && nFaceCount == 0) {
		return;
	}

	Undo_Start("Select_SetExplicitScale_BrushPrimit");
	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (f = b->brush_faces; f; f = f->next) {
			if (g_qeglobals.m_bBrushPrimitMode && f->face_winding) {
				if (absolute) {
					Face_SetExplicitScale_BrushPrimit(f, x, y);
				} else {
					Face_ScaleTexture_BrushPrimit(f, x, y);
				}
			}
			else {
				f->texdef.scale[0] += x;
				f->texdef.scale[1] += y;
			}
		}

		Brush_Build(b);
		if (b->pPatch) {
			Patch_ScaleTexture(b->pPatch, x, y, absolute);
		}
	}

	if (nFaceCount > 0) {
		for (int i = 0; i < nFaceCount; i++) {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(i));
			brush_t *selBrush = reinterpret_cast < brush_t * > (g_ptrSelectedFaceBrushes.GetAt(i));
			if (g_qeglobals.m_bBrushPrimitMode) {
				if (absolute) {
					Face_SetExplicitScale_BrushPrimit(selFace, x, y);
				} else {
					Face_ScaleTexture_BrushPrimit(selFace, x, y);
				}
			}
			else {
				selFace->texdef.scale[0] += x;
				selFace->texdef.scale[1] += y;
			}

			Brush_Build(selBrush);
		}
	}

	Undo_End();
	if (update) {
		Sys_UpdateWindows(W_CAMERA);
	}
}

extern void Face_RotateTexture_BrushPrimit(face_t *face, float amount, idVec3 origin);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_RotateTexture(float amt, bool absolute) {
	brush_t *b;
	face_t	*f;

	int		nFaceCount = g_ptrSelectedFaces.GetSize();

	if (selected_brushes.next == &selected_brushes && nFaceCount == 0) {
		return;
	}

	Undo_Start("Select_RotateTexture_BrushPrimit");
	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		for (f = b->brush_faces; f; f = f->next) {
			if (g_qeglobals.m_bBrushPrimitMode) {
				Face_RotateTexture_BrushPrimit(f, amt, b->owner->origin);
			}
			else {
				f->texdef.rotate += amt;
				f->texdef.rotate = static_cast<int>(f->texdef.rotate) % 360;
			}
		}

		Brush_Build(b);
		if (b->pPatch) {
			// Patch_RotateTexture(b->nPatchID, amt);
			Patch_RotateTexture(b->pPatch, amt);
		}
	}

	if (nFaceCount > 0) {
		for (int i = 0; i < nFaceCount; i++) {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(i));
			brush_t *selBrush = reinterpret_cast < brush_t * > (g_ptrSelectedFaceBrushes.GetAt(i));
			if (g_qeglobals.m_bBrushPrimitMode) {
				idVec3 org;
				org.Zero();
				//Face_RotateTexture_BrushPrimit(selFace, amt, selBrush->owner->origin);
				Face_RotateTexture_BrushPrimit(selFace, amt, org);
			}
			else {
				selFace->texdef.rotate += amt;
				selFace->texdef.rotate = static_cast<int>(selFace->texdef.rotate) % 360;
			}

			Brush_Build(selBrush);
		}
	}

	Undo_End();
	Sys_UpdateWindows(W_CAMERA);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void FindReplaceTextures(const char *pFind, const char *pReplace, bool bSelected, bool bForce) {
	brush_t *pList = (bSelected) ? &selected_brushes : &active_brushes;
	if (!bSelected) {
		Select_Deselect();
	}

	for (brush_t * pBrush = pList->next; pBrush != pList; pBrush = pBrush->next) {
		if (pBrush->pPatch) {
			Patch_FindReplaceTexture(pBrush, pFind, pReplace, bForce);
		}

		for (face_t * pFace = pBrush->brush_faces; pFace; pFace = pFace->next) {
			if (bForce || idStr::Icmp(pFace->texdef.name, pFind) == 0 ) {
				pFace->d_texture = Texture_ForName(pReplace);

				// strcpy(pFace->texdef.name, pReplace);
				pFace->texdef.SetName(pReplace);
			}
		}

		Brush_Build(pBrush);
	}

	Sys_UpdateWindows(W_CAMERA);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_AllOfType() {
	brush_t		*b, *next;
	entity_t	*e;
	if ((selected_brushes.next == &selected_brushes) || (selected_brushes.next->next != &selected_brushes)) {
		CString strName;
		if (g_ptrSelectedFaces.GetSize() == 0) {
			strName = g_qeglobals.d_texturewin.texdef.name;
		}
		else {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(0));
			strName = selFace->texdef.name;
		}

		Select_Deselect();
		for (b = active_brushes.next; b != &active_brushes; b = next) {
			next = b->next;

			if (FilterBrush(b)) {
				continue;
			}

			if (b->pPatch) {
				if ( idStr::Icmp(strName, b->pPatch->d_texture->GetName()) == 0 ) {
					Brush_RemoveFromList(b);
					Brush_AddToList(b, &selected_brushes);
				}
			}
			else {
				for (face_t * pFace = b->brush_faces; pFace; pFace = pFace->next) {
					if ( idStr::Icmp(strName, pFace->texdef.name) == 0 ) {
						Brush_RemoveFromList(b);
						Brush_AddToList(b, &selected_brushes);
					}
				}
			}
		}

		Sys_UpdateWindows(W_ALL);
		return;
	}

	b = selected_brushes.next;
	e = b->owner;
	if (e != NULL) {
		if (e != world_entity) {
			CString strName = e->eclass->name;
			idStr strKey, strVal;
			bool	bCriteria = g_Inspectors->GetSelectAllCriteria(strKey, strVal);
			common->Printf("Selecting all %s(s)\n", strName);
			Select_Deselect();

			for (b = active_brushes.next; b != &active_brushes; b = next) {
				next = b->next;

				if (FilterBrush(b)) {
					continue;
				}

				e = b->owner;
				if (e != NULL) {
					if ( idStr::Icmp(e->eclass->name, strName) == 0 ) {
						bool	doIt = true;
						if (bCriteria) {
							CString str = ValueForKey(e, strKey);
							if (str.CompareNoCase(strVal) != 0) {
								doIt = false;
							}
						}

						if (doIt) {
							Brush_RemoveFromList(b);
							Brush_AddToList(b, &selected_brushes);
						}
					}
				}
			}
		}
	}


	if ( selected_brushes.next && selected_brushes.next->owner ) {
		g_Inspectors->UpdateEntitySel( selected_brushes.next->owner->eclass );
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_Reselect() {
	CPtrArray	holdArray;
	brush_t *b;
	for ( b = selected_brushes.next; b && b != &selected_brushes; b = b->next ) {
		holdArray.Add(reinterpret_cast < void * > (b));
	}

	int n = holdArray.GetSize();
	while (n-- > 0) {
		b = reinterpret_cast < brush_t * > (holdArray.GetAt(n));
		Select_Brush(b);
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_FitTexture(float height, float width) {
	brush_t *b;
	int		nFaceCount = g_ptrSelectedFaces.GetSize();

	if (selected_brushes.next == &selected_brushes && nFaceCount == 0) {
		return;
	}

	Undo_Start("Select_FitTexture");
	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		if (b->pPatch) {
			Patch_FitTexture(b->pPatch, width, height);
		}
		else {
			Brush_FitTexture(b, height, width);
			Brush_Build(b);
		}
	}

	if (nFaceCount > 0) {
		for (int i = 0; i < nFaceCount; i++) {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(i));
			brush_t *selBrush = reinterpret_cast < brush_t * > (g_ptrSelectedFaceBrushes.GetAt(i));
			Face_FitTexture(selFace, height, width);
			Brush_Build(selBrush);
		}
	}

	Undo_End();
	Sys_UpdateWindows(W_CAMERA);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_AxialTexture() {
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_Hide(bool invert) {

	if (invert) {
		for (brush_t * b = active_brushes.next; b && b != &active_brushes; b = b->next) {
			b->hiddenBrush = true;
		}
	} else {
		for (brush_t * b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
			b->hiddenBrush = true;
		}
	}
	Sys_UpdateWindows(W_ALL);
}

void Select_WireFrame( bool wireFrame ) {
	for (brush_t * b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
		b->forceWireFrame = wireFrame;
	}
	Sys_UpdateWindows(W_ALL);
}

void Select_ForceVisible( bool visible ) {
	for (brush_t * b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
		b->forceVisibile = visible;
	}
	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_ShowAllHidden() {
	brush_t *b;
	for (b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
		b->hiddenBrush = false;
	}

	for (b = active_brushes.next; b && b != &active_brushes; b = b->next) {
		b->hiddenBrush = false;
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
    Select_Invert
 =======================================================================================================================
 */
void Select_Invert(void) {
	brush_t *next, *prev;

	Sys_Status("inverting selection...\n");

	next = active_brushes.next;
	prev = active_brushes.prev;
	if (selected_brushes.next != &selected_brushes) {
		active_brushes.next = selected_brushes.next;
		active_brushes.prev = selected_brushes.prev;
		active_brushes.next->prev = &active_brushes;
		active_brushes.prev->next = &active_brushes;
	}
	else {
		active_brushes.next = &active_brushes;
		active_brushes.prev = &active_brushes;
	}

	if (next != &active_brushes) {
		selected_brushes.next = next;
		selected_brushes.prev = prev;
		selected_brushes.next->prev = &selected_brushes;
		selected_brushes.prev->next = &selected_brushes;
	}
	else {
		selected_brushes.next = &selected_brushes;
		selected_brushes.prev = &selected_brushes;
	}

	Sys_UpdateWindows(W_ALL);

	Sys_Status("done.\n");
}

/*
 =======================================================================================================================
    Select_Name
 =======================================================================================================================
 */
void Select_Name(const char *pName) {
	if (g_qeglobals.m_bBrushPrimitMode) {
		for (brush_t * b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
			Brush_SetEpair(b, "Name", pName);
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_CenterOrigin() {
	idVec3	mid;

	Select_GetTrueMid(mid);
	mid.Snap();

	brush_t		*b = selected_brushes.next;
	entity_t	*e = b->owner;
	if (e != NULL) {
		if (e != world_entity) {
			char	text[1024];
			sprintf(text, "%i %i %i", (int)mid[0], (int)mid[1], (int)mid[2]);
			SetKeyValue(e, "origin", text);
			VectorCopy(mid, e->origin);
		}
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int Select_NumSelectedFaces() {
	return g_ptrSelectedFaces.GetSize();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
face_t *Select_GetSelectedFace(int index) {
	assert(index >= 0 && index < Select_NumSelectedFaces());
	return reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(index));
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
brush_t *Select_GetSelectedFaceBrush(int index) {
	assert(index >= 0 && index < Select_NumSelectedFaces());
	return reinterpret_cast < brush_t * > (g_ptrSelectedFaceBrushes.GetAt(index));
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_SetDefaultTexture(const idMaterial *mat, bool fitScale, bool setTexture) {
	texdef_t				tex;
	brushprimit_texdef_t	brushprimit_tex;
	memset(&tex, 0, sizeof(tex));
	memset(&brushprimit_tex, 0, sizeof(brushprimit_tex));
	if (g_qeglobals.m_bBrushPrimitMode) {
		// brushprimit fitted to a 2x2 texture
		brushprimit_tex.coords[0][0] = 1.0f;
		brushprimit_tex.coords[1][1] = 1.0f;
	}
	else {
		tex.scale[0] = (g_PrefsDlg.m_bHiColorTextures) ? 0.5 : 1;
		tex.scale[1] = (g_PrefsDlg.m_bHiColorTextures) ? 0.5 : 1;
	}

	tex.SetName(mat->GetName());
	Texture_SetTexture(&tex, &brushprimit_tex, fitScale, setTexture);

	CString strTex;
	strTex.Format
		(
			"%s (%s) W: %i H: %i",
			mat->GetName(),
			mat->GetDescription(),
			mat->GetEditorImage()->uploadWidth,
			mat->GetEditorImage()->uploadHeight
		);
	g_pParentWnd->SetStatusText(3, strTex);
}


void Select_UpdateTextureName(const char *name) {
	brush_t *b;
	int		nCount = g_ptrSelectedFaces.GetSize();
	if (nCount > 0) {
		Undo_Start("set face texture name");
		ASSERT(g_ptrSelectedFaces.GetSize() == g_ptrSelectedFaceBrushes.GetSize());
		for (int i = 0; i < nCount; i++) {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(i));
			brush_t *selBrush = reinterpret_cast < brush_t * > (g_ptrSelectedFaceBrushes.GetAt(i));
			Undo_AddBrush(selBrush);
			selFace->texdef.SetName(name);
			Brush_Build(selBrush);
			Undo_EndBrush(selBrush);
		}

		Undo_End();
	}
	else if (selected_brushes.next != &selected_brushes) {
		Undo_Start("set brush textures");
		for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
			if (!b->owner->eclass->fixedsize) {
				Undo_AddBrush(b);
				Brush_SetTextureName(b, name);
				Undo_EndBrush(b);
			} else if (b->owner->eclass->nShowFlags & ECLASS_LIGHT) {
				if ( idStr::Cmpn(name, "lights/", strlen("lights/")) == 0 ) {
					SetKeyValue(b->owner, "texture", name);
					g_Inspectors->UpdateEntitySel(b->owner->eclass);
					UpdateLightInspector();
					Brush_Build(b);
				}
			}
		}

		Undo_End();
	}

	Sys_UpdateWindows(W_ALL);
}


/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Select_FlipTexture(bool y) {

	int faceCount = g_ptrSelectedFaces.GetSize();
	
	Undo_Start("Select_FlipTexture");
	for (brush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		if (b->pPatch) {
			Patch_FlipTexture(b->pPatch, y);
		} else {
			Brush_FlipTexture_BrushPrimit(b, y);
		}
	}

	if (faceCount > 0) {
		for (int i = 0; i < faceCount; i++) {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(i));
			brush_t *selBrush = reinterpret_cast < brush_t * > (g_ptrSelectedFaceBrushes.GetAt(i));
			Face_FlipTexture_BrushPrimit(selFace, y);
		}
	}

	Undo_End();
	Sys_UpdateWindows(W_CAMERA);
}




/*
 =======================================================================================================================
    Select_SetKeyVal 
	sets values on non-world entities
 =======================================================================================================================
 */
void Select_SetKeyVal(const char *key, const char *val) {
	for (brush_t * b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
		if (b->owner != world_entity) {
			SetKeyValue(b->owner, key, val, false);
		}
	}
}

/*
 =======================================================================================================================
	Select_CopyPatchTextureCoords( patchMesh_t *p )
 =======================================================================================================================
 */
void Select_CopyPatchTextureCoords( patchMesh_t *p ) {
	for (brush_t * b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
		if (b->pPatch) {
			if ( b->pPatch->width <= p->width && b->pPatch->height <= p->height ) {
				for ( int i = 0; i < b->pPatch->width; i ++ ) {
					for ( int j = 0; j < b->pPatch->height; j++ ) {
						b->pPatch->ctrl(i, j).st = p->ctrl(i, j).st;
					}
				}
			}
		}
	}
}


/*
 =======================================================================================================================
	Select_SetProjectFaceOntoPatch     
 =======================================================================================================================
 */
void Select_ProjectFaceOntoPatch( face_t *face ) {
	for (brush_t * b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
		if (b->pPatch) {
			EmitBrushPrimitTextureCoordinates(face, NULL, b->pPatch);
			Patch_MakeDirty(b->pPatch);
		}
	}
}

/*
 =======================================================================================================================
    Select_SetPatchFit 
 =======================================================================================================================
 */
extern float Patch_Width(patchMesh_t *p);
extern float Patch_Height(patchMesh_t *p);
void Select_SetPatchFit(float dim1, float dim2, float srcWidth, float srcHeight, float rot) {
	for (brush_t * b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
		if (b->pPatch) {
			float w = Patch_Width(b->pPatch);
			float h = Patch_Height(b->pPatch);
			Patch_RotateTexture(b->pPatch, -90 + rot);
			Patch_FitTexture(b->pPatch, dim1 * (w / srcWidth), dim2 * (h / srcHeight));
			Patch_FlipTexture(b->pPatch, true);
		}
	}
}

void Select_SetPatchST(float s1, float t1, float s2, float t2) {
}


void Select_AllTargets() {
	for (brush_t * b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
		if (b->owner != world_entity) {
			const idKeyValue *kv = b->owner->epairs.MatchPrefix("target", NULL);
			while (kv) {
				entity_t *ent = FindEntity("name", kv->GetValue());
				if (ent) {
					Select_Brush(ent->brushes.onext, true, false);
				}
				kv = b->owner->epairs.MatchPrefix("target", kv);
			}
		}
	}
}
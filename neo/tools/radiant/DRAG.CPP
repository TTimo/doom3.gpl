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
#include "splines.h"

/* drag either multiple brushes, or select plane points from a single brush. */
bool			g_moveOnly = false;
bool			drag_ok;
idVec3			drag_xvec;
idVec3			drag_yvec;

static int		buttonstate;
int				pressx, pressy;
static idVec3	pressdelta;
static idVec3	vPressStart;
static int		buttonx, buttony;

// int num_move_points; float *move_points[1024];
int				lastx, lasty;

bool			drag_first;

/*
================
AxializeVector
================
*/
static void AxializeVector( idVec3 &v ) {
	idVec3	a;
	float	o;
	int		i;

	if (!v[0] && !v[1]) {
		return;
	}

	if (!v[1] && !v[2]) {
		return;
	}

	if (!v[0] && !v[2]) {
		return;
	}

	for (i = 0; i < 3; i++) {
		a[i] = idMath::Fabs(v[i]);
	}

	if (a[0] > a[1] && a[0] > a[2]) {
		i = 0;
	}
	else if (a[1] > a[0] && a[1] > a[2]) {
		i = 1;
	}
	else {
		i = 2;
	}

	o = v[i];
	VectorCopy(vec3_origin, v);
	if (o < 0) {
		v[i] = -1;
	}
	else {
		v[i] = 1;
	}
}

extern bool UpdateActiveDragPoint(const idVec3 &move);
extern void SetActiveDrag(CDragPoint *p);

/*
================
Draw_Setup
================
*/
static void Drag_Setup( int x, int y, int buttons,
					   const idVec3 &xaxis, const idVec3 &yaxis, const idVec3 &origin, const idVec3 &dir ) {
	qertrace_t	t;
	face_t		*f;

	drag_first = true;

	VectorCopy(vec3_origin, pressdelta);
	pressx = x;
	pressy = y;

	VectorCopy(xaxis, drag_xvec);
	AxializeVector(drag_xvec);
	VectorCopy(yaxis, drag_yvec);
	AxializeVector(drag_yvec);

	if (g_qeglobals.d_select_mode == sel_addpoint) {
		if (g_qeglobals.selectObject) {
			g_qeglobals.selectObject->addPoint(origin);
		}
		else {
			g_qeglobals.d_select_mode = sel_brush;
		}

		return;
	}

	if (g_qeglobals.d_select_mode == sel_editpoint) {

		g_Inspectors->entityDlg.SelectCurvePointByRay( origin, dir, buttons );

		if ( g_qeglobals.d_num_move_points ) {
			drag_ok = true;
		}

		Sys_UpdateWindows(W_ALL);

		return;
	}

	if (g_qeglobals.d_select_mode == sel_curvepoint) {
		SelectCurvePointByRay(origin, dir, buttons);

		if (g_qeglobals.d_num_move_points || g_qeglobals.d_select_mode == sel_area) {
			drag_ok = true;
		}

		Sys_UpdateWindows(W_ALL);

		Undo_Start("drag curve point");
		Undo_AddBrushList(&selected_brushes);

		return;
	}
	else {
		g_qeglobals.d_num_move_points = 0;
	}

	if (selected_brushes.next == &selected_brushes) {
		//
		// in this case a new brush is created when the dragging takes place in the XYWnd,
		// An useless undo is created when the dragging takes place in the CamWnd
		//
		Undo_Start("create brush");

		Sys_Status("No selection to drag\n", 0);
		return;
	}

	if (g_qeglobals.d_select_mode == sel_vertex) {
		
		if ( radiant_entityMode.GetBool() ) {
			return;
		}

		SelectVertexByRay(origin, dir);
		if (g_qeglobals.d_num_move_points) {
			drag_ok = true;
			Undo_Start("drag vertex");
			Undo_AddBrushList(&selected_brushes);
			return;
		}
	}

	if (g_qeglobals.d_select_mode == sel_edge) {
		
		if ( radiant_entityMode.GetBool() ) {
			return;
		}

		SelectEdgeByRay(origin, dir);
		if (g_qeglobals.d_num_move_points) {
			drag_ok = true;
			Undo_Start("drag edge");
			Undo_AddBrushList(&selected_brushes);
			return;
		}
	}

	// check for direct hit first
	t = Test_Ray(origin, dir, true);
	SetActiveDrag(t.point);
	if (t.point) {
		drag_ok = true;

		// point was hit
		return;
	}

	if (t.selected) {
		drag_ok = true;

		Undo_Start("drag selection");
		Undo_AddBrushList(&selected_brushes);

		if (buttons == (MK_LBUTTON | MK_CONTROL)) {
			Sys_Status("Shear dragging face\n");
			Brush_SelectFaceForDragging(t.brush, t.face, true);
		}
		else if (buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT)) {
			Sys_Status("Sticky dragging brush\n");
			for (f = t.brush->brush_faces; f; f = f->next) {
				Brush_SelectFaceForDragging(t.brush, f, false);
			}
		}
		else {
			Sys_Status("Dragging entire selection\n");
		}

		return;
	}

	if (g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge) {
		return;
	}

	if ( radiant_entityMode.GetBool() ) {
		return;
	}

	// check for side hit multiple brushes selected?
	if (selected_brushes.next->next != &selected_brushes) {
		// yes, special handling
		bool bOK = ( g_PrefsDlg.m_bALTEdge ) ? ( ::GetAsyncKeyState( VK_MENU ) != 0 ) : true;
		if (bOK) {
			for (brush_t * pBrush = selected_brushes.next; pBrush != &selected_brushes; pBrush = pBrush->next) {
				if (buttons & MK_CONTROL) {
					Brush_SideSelect(pBrush, origin, dir, true);
				}
				else {
					Brush_SideSelect(pBrush, origin, dir, false);
				}
			}
		}
		else {
			Sys_Status("press ALT to drag multiple edges\n");
			return;
		}
	}
	else {
		// single select.. trying to drag fixed entities handle themselves and just move
		if (buttons & MK_CONTROL) {
			Brush_SideSelect(selected_brushes.next, origin, dir, true);
		}
		else {
			Brush_SideSelect(selected_brushes.next, origin, dir, false);
		}
	}

	Sys_Status("Side stretch\n");
	drag_ok = true;

	Undo_Start("side stretch");
	Undo_AddBrushList(&selected_brushes);
}

extern void Face_GetScale_BrushPrimit(face_t *face, float *s, float *t, float *rot);

/*
================
Drag_Begin
================
*/
void Drag_Begin( int x, int y, int buttons,
				const idVec3 &xaxis, const idVec3 &yaxis, const idVec3 &origin, const idVec3 &dir ) {
	qertrace_t	t;

	drag_ok = false;
	VectorCopy(vec3_origin, pressdelta);
	VectorCopy(vec3_origin, vPressStart);

	drag_first = true;

	// shift LBUTTON = select entire brush
	if (buttons == (MK_LBUTTON | MK_SHIFT) && g_qeglobals.d_select_mode != sel_curvepoint) {
		int nFlag = ( ::GetAsyncKeyState( VK_MENU ) != 0 ) ? SF_CYCLE : 0;
		if (dir[0] == 0 || dir[1] == 0 || dir[2] == 0) {		// extremely low chance of this happening from camera
			Select_Ray(origin, dir, nFlag | SF_ENTITIES_FIRST); // hack for XY
		}
		else {
			Select_Ray(origin, dir, nFlag);
		}

		return;
	}

	// ctrl-shift LBUTTON = select single face
	if (buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT) && g_qeglobals.d_select_mode != sel_curvepoint) {
		if ( radiant_entityMode.GetBool() ) {
			return;
		}

		// _D3XP disabled
		//Select_Deselect( ( ::GetAsyncKeyState( VK_MENU ) == 0 ) );
		Select_Ray(origin, dir, SF_SINGLEFACE);
		return;
	}

	// LBUTTON + all other modifiers = manipulate selection
	if (buttons & MK_LBUTTON) {
		Drag_Setup(x, y, buttons, xaxis, yaxis, origin, dir);
		return;
	}

	if ( radiant_entityMode.GetBool() ) {
		return;
	}

	int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;

	// middle button = grab texture
	if (buttons == nMouseButton) {
		t = Test_Ray(origin, dir, false);
		if (t.face) {
			g_qeglobals.d_new_brush_bottom = t.brush->mins;
			g_qeglobals.d_new_brush_top = t.brush->maxs;

			// use a local brushprimit_texdef fitted to a default 2x2 texture
			brushprimit_texdef_t bp_local;
			if (t.brush && t.brush->pPatch) {
				texdef_t localtd;
				memset(&bp_local.coords, 0, sizeof(bp_local.coords));
				bp_local.coords[0][0] = 1.0f;
				bp_local.coords[1][1] = 1.0f;
				localtd.SetName(t.brush->pPatch->d_texture->GetName());
				Texture_SetTexture(&localtd, &bp_local, false, true);
				Select_CopyPatchTextureCoords ( t.brush->pPatch );
			} else {
				Select_ProjectFaceOntoPatch( t.face );
				ConvertTexMatWithQTexture(&t.face->brushprimit_texdef, t.face->d_texture, &bp_local, NULL);
				Texture_SetTexture(&t.face->texdef, &bp_local, false, true);
			}
			UpdateSurfaceDialog();
			UpdatePatchInspector();
			UpdateLightInspector();
		}
		else {
			Sys_Status("Did not select a texture\n");
		}

		return;
	}

	// ctrl-middle button = set entire brush to texture
	if (buttons == (nMouseButton | MK_CONTROL)) {
		t = Test_Ray(origin, dir, false);
		if (t.brush) {
			if (t.brush->brush_faces->texdef.name[0] == '(') {
				Sys_Status("Can't change an entity texture\n");
			}
			else {
				Brush_SetTexture
				(
					t.brush,
					&g_qeglobals.d_texturewin.texdef,
					&g_qeglobals.d_texturewin.brushprimit_texdef,
					false
				);
				Sys_UpdateWindows(W_ALL);
			}
		}
		else {
			Sys_Status("Didn't hit a btrush\n");
		}

		return;
	}

	// ctrl-shift-middle button = set single face to texture
	if (buttons == (nMouseButton | MK_SHIFT | MK_CONTROL)) {
		t = Test_Ray(origin, dir, false);
		if (t.brush) {
			if (t.brush->brush_faces->texdef.name[0] == '(') {
				Sys_Status("Can't change an entity texture\n");
			}
			else {
				SetFaceTexdef
				(
					t.brush,
					t.face,
					&g_qeglobals.d_texturewin.texdef,
					&g_qeglobals.d_texturewin.brushprimit_texdef
				);
				Brush_Build(t.brush);
				Sys_UpdateWindows(W_ALL);
			}
		}
		else {
			Sys_Status("Didn't hit a btrush\n");
		}

		return;
	}

	if (buttons == (nMouseButton | MK_SHIFT)) {
		Sys_Status("Set brush face texture info\n");
		t = Test_Ray(origin, dir, false);
		if (t.brush && !t.brush->owner->eclass->fixedsize) {
/*
			if (t.brush->brush_faces->texdef.name[0] == '(') {
				if (t.brush->owner->eclass->nShowFlags & ECLASS_LIGHT) {
					CString		strBuff;
					idMaterial	*pTex = declManager->FindMaterial(g_qeglobals.d_texturewin.texdef.name);
					if (pTex) {
						idVec3	vColor = pTex->getColor();

						float	fLargest = 0.0f;
						for (int i = 0; i < 3; i++) {
							if (vColor[i] > fLargest) {
								fLargest = vColor[i];
							}
						}

						if (fLargest == 0.0f) {
							vColor[0] = vColor[1] = vColor[2] = 1.0f;
						}
						else {
							float	fScale = 1.0f / fLargest;
							for (int i = 0; i < 3; i++) {
								vColor[i] *= fScale;
							}
						}

						strBuff.Format("%f %f %f", pTex->getColor().x, pTex->getColor().y, pTex->getColor().z);
						SetKeyValue(t.brush->owner, "_color", strBuff.GetBuffer(0));
						Sys_UpdateWindows(W_ALL);
					}
				}
				else {
					Sys_Status("Can't select an entity brush face\n");
				}
			}

			else {
*/
				// strcpy(t.face->texdef.name,g_qeglobals.d_texturewin.texdef.name);
				t.face->texdef.SetName(g_qeglobals.d_texturewin.texdef.name);
				Brush_Build(t.brush);
				Sys_UpdateWindows(W_ALL);
//			}
		}
		else {
			Sys_Status("Didn't hit a brush\n");
		}

		return;
	}
}


void Brush_GetBounds(brush_t *b, idVec3 &mins, idVec3 &maxs) {
	int		i;

	for (i = 0; i < 3; i++) {
		mins[i] = 999999;
		maxs[i] = -999999;
	}

	for (i = 0; i < 3; i++) {
		if (b->mins[i] < mins[i]) {
			mins[i] = b->mins[i];
		}

		if (b->maxs[i] > maxs[i]) {
			maxs[i] = b->maxs[i];
		}
	}
}


/*
================
MoveSelection
================
*/
static void MoveSelection( const idVec3 &orgMove ) {
	int		i, success;
	brush_t *b;
	CString strStatus;
	idVec3	vTemp, vTemp2, end, move;

	move = orgMove;

	if (!move[0] && !move[1] && !move[2]) {
		return;
	}

	move[0] = (g_nScaleHow & SCALE_X) ? 0 : move[0];
	move[1] = (g_nScaleHow & SCALE_Y) ? 0 : move[1];
	move[2] = (g_nScaleHow & SCALE_Z) ? 0 : move[2];

	if (g_pParentWnd->ActiveXY()->RotateMode() || g_bPatchBendMode) {
		float	fDeg = -move[2];
		float	fAdj = move[2];
	   	int axis = 0;
		if (g_pParentWnd->ActiveXY()->GetViewType() == XY) {
			fDeg = -move[1];
			fAdj = move[1];
			axis = 2;
		}
		else if (g_pParentWnd->ActiveXY()->GetViewType() == XZ) {
			fDeg = move[2];
			fAdj = move[2];
			axis = 1;
		}

		g_pParentWnd->ActiveXY()->Rotation()[g_qeglobals.rotateAxis] += fAdj;
		strStatus.Format
			(
				"%s x:: %.1f  y:: %.1f  z:: %.1f",
				(g_bPatchBendMode) ? "Bend angle" : "Rotation",
				g_pParentWnd->ActiveXY()->Rotation()[0],
				g_pParentWnd->ActiveXY()->Rotation()[1],
				g_pParentWnd->ActiveXY()->Rotation()[2]
			);
		g_pParentWnd->SetStatusText(2, strStatus);

		if (g_bPatchBendMode) {
			Patch_SelectBendNormal();
			Select_RotateAxis(axis, fDeg * 2, false, true);
			Patch_SelectBendAxis();
			Select_RotateAxis(axis, fDeg, false, true);
		}
		else {
			Select_RotateAxis(g_qeglobals.rotateAxis, fDeg, false, true);
		}

		return;
	}

	if (g_pParentWnd->ActiveXY()->ScaleMode()) {
		idVec3	v;
		v[0] = v[1] = v[2] = 1.0f;
		for (int i = 0; i < 3; i++) {
			if ( move[i] > 0.0f ) {
				v[i] = 1.1f;
			} else if ( move[i] < 0.0f ) {
				v[i] = 0.9f;
			}
		}

		Select_Scale(v.x, v.y, v.z);
		Sys_UpdateWindows(W_ALL);
		return;
	}

	idVec3	vDistance;
	VectorSubtract(pressdelta, vPressStart, vDistance);
	strStatus.Format("Distance x: %.3f  y: %.3f  z: %.3f", vDistance[0], vDistance[1], vDistance[2]);
	g_pParentWnd->SetStatusText(3, strStatus);

	// dragging only a part of the selection
	if (UpdateActiveDragPoint(move)) {
		UpdateLightInspector();
		return;
	}

	//
	// this is fairly crappy way to deal with curvepoint and area selection but it
	// touches the smallest amount of code this way
	//
	if (g_qeglobals.d_num_move_points || g_qeglobals.d_num_move_planes || g_qeglobals.d_select_mode == sel_area) {
		// area selection
		if (g_qeglobals.d_select_mode == sel_area) {
			VectorAdd(g_qeglobals.d_vAreaBR, move, g_qeglobals.d_vAreaBR);
			return;
		}

		// curve point selection
		if (g_qeglobals.d_select_mode == sel_curvepoint) {
			Patch_UpdateSelected(move);
			return;
		}

		// vertex selection
		if (g_qeglobals.d_select_mode == sel_vertex) {
			success = true;
			for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
				success &= Brush_MoveVertex(selected_brushes.next, *g_qeglobals.d_move_points[0], move, end, true);
			}

			// if (success)
			VectorCopy(end, *g_qeglobals.d_move_points[0]);
			return;
		}

		// all other selection types
		for (i = 0; i < g_qeglobals.d_num_move_points; i++) {
			VectorAdd(*g_qeglobals.d_move_points[i], move, *g_qeglobals.d_move_points[i]);
		}

		if ( g_qeglobals.d_select_mode == sel_editpoint ) {
			g_Inspectors->entityDlg.UpdateEntityCurve();
		}

		//
		// VectorScale(move, .5, move); for (i=0 ; i<g_qeglobals.d_num_move_points2 ; i++)
		// VectorAdd (g_qeglobals.d_move_points2[i], move, g_qeglobals.d_move_points2[i]);
		//
		for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
			VectorCopy(b->maxs, vTemp);
			VectorSubtract(vTemp, b->mins, vTemp);
			Brush_Build(b);
			for (i = 0; i < 3; i++) {
				if
				(
					b->mins[i] > b->maxs[i] ||
					b->maxs[i] - b->mins[i] > MAX_WORLD_SIZE ||
					b->maxs[i] - b->mins[i] == 0.0f
				) {
					break;	// dragged backwards or messed up
				}
			}

			if (i != 3) {
				break;
			}

			if (b->pPatch) {
				VectorCopy(b->maxs, vTemp2);
				VectorSubtract(vTemp2, b->mins, vTemp2);
				VectorSubtract(vTemp2, vTemp, vTemp2);

				// if (!Patch_DragScale(b->nPatchID, vTemp2, move))
				if (!Patch_DragScale(b->pPatch, vTemp2, move)) {
					b = NULL;
					break;
				}
			}
		}

		// if any of the brushes were crushed out of existance calcel the entire move
		if (b != &selected_brushes) {
			Sys_Status("Brush dragged backwards, move canceled\n");
			for (i = 0; i < g_qeglobals.d_num_move_points; i++) {
				VectorSubtract(*g_qeglobals.d_move_points[i], move, *g_qeglobals.d_move_points[i]);
			}

			for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
				Brush_Build(b);
			}
		}
	}
	else {
		//
		// reset face originals from vertex edit mode this is dirty, but unfortunately
		// necessary because Brush_Build can remove windings
		//
		for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
			Brush_ResetFaceOriginals(b);
		}

		Select_Move(move);
	}
}

/*
================
Drag_MouseMoved
================
*/
void Drag_MouseMoved(int x, int y, int buttons) {
	idVec3	move, delta;
	int		i;

	if (!buttons || !drag_ok) {
		drag_ok = false;
		return;
	}

	// clear along one axis
	if (buttons & MK_SHIFT) {
		drag_first = false;
		if (abs(x - pressx) > abs(y - pressy)) {
			y = pressy;
		}
		else {
			x = pressx;
		}
	}

	for (i = 0; i < 3; i++) {
		move[i] = drag_xvec[i] * (x - pressx) + drag_yvec[i] * (y - pressy);
		if (!g_PrefsDlg.m_bNoClamp) {
			move[i] = floor(move[i] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
		}
	}

	VectorSubtract(move, pressdelta, delta);
	VectorCopy(move, pressdelta);

	if (buttons & MK_CONTROL && g_pParentWnd->ActiveXY()->RotateMode()) {
		for (i = 0; i < 3; i++) {
			if (delta[i] != 0) {
				if (delta[i] > 0) {
					delta[i] = 15;
				}
				else {
					delta[i] = -15;
				}
			}
		}
	}

	MoveSelection(delta);
}

/*
================
Drag_MouseUp
================
*/
void Drag_MouseUp(int nButtons) {
	Sys_Status("drag completed.", 0);

	if (g_qeglobals.d_select_mode == sel_area) {
		Patch_SelectAreaPoints();
		g_qeglobals.d_select_mode = sel_curvepoint;
		Sys_UpdateWindows(W_ALL);
	}

	if (g_qeglobals.d_select_translate[0] || g_qeglobals.d_select_translate[1] || g_qeglobals.d_select_translate[2]) {
		Select_Move(g_qeglobals.d_select_translate);
		VectorCopy(vec3_origin, g_qeglobals.d_select_translate);
		Sys_UpdateWindows(W_CAMERA);
	}

	g_pParentWnd->SetStatusText(3, "");

/*
	if (g_pParentWnd->GetCamera()->UpdateRenderEntities()) {
		Sys_UpdateWindows(W_CAMERA);
	}
*/
	
	Undo_EndBrushList(&selected_brushes);
	Undo_End();
}

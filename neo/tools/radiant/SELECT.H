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

#ifndef __SELECT_H_
#define __SELECT_H_

typedef enum
{
	sel_brush,
	// sel_sticky_brush,
	// sel_face,
	sel_vertex,
	sel_edge,
	sel_singlevertex,
	sel_curvepoint,
	sel_area,
	sel_addpoint,			// for dropping points
	sel_editpoint			// for editing points
} select_t;

class CDragPoint {
public:
	idVec3 vec;
	brush_t *pBrush;
	int nType;
	bool priority;
	CDragPoint() {};
	CDragPoint(brush_t *b, idVec3 v, int type, bool p) {
		pBrush = b;
		VectorCopy(v, vec);
		nType = type;
		priority = p;
	}

	void Set(brush_t *b, idVec3 v, int type) {
		pBrush = b;
		VectorCopy(v, vec);
		nType = type;
	}

	bool PointWithin(idVec3 p, int nView = -1);
};


typedef struct
{
	brush_t		*brush;
	face_t		*face;
	CDragPoint *point;
	float		dist;
	bool	selected;
} qertrace_t;


#define	SF_SELECTED_ONLY	 0x01
#define	SF_ENTITIES_FIRST	 0x02
#define	SF_SINGLEFACE		   0x04
#define SF_IGNORECURVES    0x08
#define SF_IGNOREGROUPS    0x10
#define SF_CYCLE           0x20


qertrace_t Test_Ray ( const idVec3 &origin, const idVec3 &dir, int flags );
CDragPoint *PointRay( const idVec3 &org, const idVec3 &dir, float *dist);
void SelectCurvePointByRay( const idVec3 &org, const idVec3 &dir, int buttons);
void SelectSplinePointByRay( const idVec3 &org, const idVec3 &dir, int buttons);

void Select_GetBounds (idVec3 &mins, idVec3 &maxs);
void Select_Brush (brush_t *b, bool bComplete = true, bool bStatus = true);
void Select_Ray (idVec3 origin, idVec3 dir, int flags);
void Select_Delete (void);
void Select_Deselect (bool bDeselectFaces = true);
void Select_Invert(void);
void Select_Clone (void);
void Select_Move (idVec3 delta, bool bSnap = true);
void WINAPI Select_SetTexture (texdef_t *texdef, brushprimit_texdef_t *brushprimit_texdef, bool bFitScale = false, void* pPlugTexdef = NULL, bool update = true);
void Select_FlipAxis (int axis);
void Select_RotateAxis (int axis, float deg, bool bPaint = true, bool bMouse = false);
void Select_CompleteTall (void);
void Select_PartialTall (void);
void Select_Touching (void);
void Select_Inside (void);
void Select_CenterOrigin();
void Select_AllOfType();
void Select_Reselect();
void Select_FitTexture(float height = 1.0, float width = 1.0);
void Select_InitializeRotation();
void Select_FinalizeRotation();

// absolute texture coordinates
// TTimo NOTE: this is stuff for old brushes format and rotation texture lock .. sort of in-between with bush primitives
void ComputeAbsolute(face_t* f, idVec3& p1, idVec3& p2, idVec3& p3);
void AbsoluteToLocal( const idPlane &normal2, face_t* f, idVec3& p1, idVec3& p2, idVec3& p3);
void Select_Hide(bool invert = false);
void Select_ShowAllHidden();
void Select_WireFrame( bool wireFrame );
void Select_ForceVisible( bool visible );
void Select_Name(const char *pName);
void Select_AddProjectedLight();
void Select_GetMid (idVec3 &mid);
void Select_SetDefaultTexture(const idMaterial *mat, bool fitScale, bool setTexture);
void Select_UpdateTextureName(const char *name);

void Select_FlipTexture(bool y);
void Select_SetPatchFit(float dim1, float dim2, float srcWidth, float srcHeight, float rot);
void Select_SetPatchST(float s1, float t1, float s2, float t2);
void Select_ProjectFaceOntoPatch( face_t *face );
void Select_CopyPatchTextureCoords( patchMesh_t *p );
void Select_AllTargets();
void Select_Scale(float x, float y, float z);
void Select_RotateTexture(float amt, bool absolute = false);
void Select_ScaleTexture(float x, float y, bool update = true, bool absolute = true);
void Select_DefaultTextureScale(bool horz, bool vert, bool update = true);
void Select_ShiftTexture(float x, float y, bool autoAdjust = false);
void Select_GetTrueMid (idVec3 &mid);
void Select_Scale(float x, float y, float z);


#endif
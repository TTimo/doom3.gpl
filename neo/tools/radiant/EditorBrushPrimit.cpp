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

#define ZERO_EPSILON	1.0E-6

class idVec3D {
public:
	double x, y, z;
	double &			operator[]( const int index ) {
		return (&x)[index];
	}
	void Zero() {
		x = y = z = 0.0;
	}
};

//
// =======================================================================================================================
//    compute a determinant using Sarrus rule ++timo "inline" this with a macro NOTE:: the three idVec3D are understood as
//    columns of the matrix
// =======================================================================================================================
//
double SarrusDet(idVec3D a, idVec3D b, idVec3D c) {
	return (double)a[0] * (double)b[1] * (double)c[2] + (double)b[0] * (double)c[1] * (double)a[2] + (double)c[0] * (double)a[1] * (double)b[2] - (double)c[0] * (double)b[1] * (double)a[2] - (double)a[1] * (double)b[0] * (double)c[2] -	(double)a[0] * (double)b[2] * (double)c[1];
}

//
// =======================================================================================================================
//    ++timo replace everywhere texX by texS etc. ( > and in q3map !) NOTE:: ComputeAxisBase here and in q3map code must
//    always BE THE SAME ! WARNING:: special case behaviour of atan2(y,x) <-> atan(y/x) might not be the same everywhere
//    when x == 0 rotation by (0,RotY,RotZ) assigns X to normal
// =======================================================================================================================
//
void ComputeAxisBase(idVec3 &normal, idVec3D &texS, idVec3D &texT) {
	double	RotY, RotZ;

	// do some cleaning
	if (idMath::Fabs(normal[0]) < 1e-6) {
		normal[0] = 0.0f;
	}

	if (idMath::Fabs(normal[1]) < 1e-6) {
		normal[1] = 0.0f;
	}

	if (idMath::Fabs(normal[2]) < 1e-6) {
		normal[2] = 0.0f;
	}

	RotY = -atan2(normal[2], idMath::Sqrt(normal[1] * normal[1] + normal[0] * normal[0]));
	RotZ = atan2(normal[1], normal[0]);

	// rotate (0,1,0) and (0,0,1) to compute texS and texT
	texS[0] = -sin(RotZ);
	texS[1] = cos(RotZ);
	texS[2] = 0;

	// the texT vector is along -Z ( T texture coorinates axis )
	texT[0] = -sin(RotY) * cos(RotZ);
	texT[1] = -sin(RotY) * sin(RotZ);
	texT[2] = -cos(RotY);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void FaceToBrushPrimitFace(face_t *f) {
	idVec3D	texX, texY;
	idVec3D	proj;

	// ST of (0,0) (1,0) (0,1)
	idVec5	ST[3];	// [ point index ] [ xyz ST ]

	//
	// ++timo not used as long as brushprimit_texdef and texdef are static
	// f->brushprimit_texdef.contents=f->texdef.contents;
	// f->brushprimit_texdef.flags=f->texdef.flags;
	// f->brushprimit_texdef.value=f->texdef.value;
	// strcpy(f->brushprimit_texdef.name,f->texdef.name);
	//
#ifdef _DEBUG
	if (f->plane[0] == 0.0f && f->plane[1] == 0.0f && f->plane[2] == 0.0f) {
		common->Printf("Warning : f->plane.normal is (0,0,0) in FaceToBrushPrimitFace\n");
	}

	// check d_texture
	if (!f->d_texture) {
		common->Printf("Warning : f.d_texture is NULL in FaceToBrushPrimitFace\n");
		return;
	}
#endif
	// compute axis base
	ComputeAxisBase(f->plane.Normal(), texX, texY);

	// compute projection vector
	VectorCopy( f->plane, proj );
	VectorScale(proj, -f->plane[3], proj);

	//
	// (0,0) in plane axis base is (0,0,0) in world coordinates + projection on the
	// affine plane (1,0) in plane axis base is texX in world coordinates + projection
	// on the affine plane (0,1) in plane axis base is texY in world coordinates +
	// projection on the affine plane use old texture code to compute the ST coords of
	// these points
	//
	VectorCopy(proj, ST[0]);
	EmitTextureCoordinates(ST[0], f->d_texture, f);
	VectorCopy(texX, ST[1]);
	VectorAdd(ST[1], proj, ST[1]);
	EmitTextureCoordinates(ST[1], f->d_texture, f);
	VectorCopy(texY, ST[2]);
	VectorAdd(ST[2], proj, ST[2]);
	EmitTextureCoordinates(ST[2], f->d_texture, f);

	// compute texture matrix
	f->brushprimit_texdef.coords[0][2] = ST[0][3];
	f->brushprimit_texdef.coords[1][2] = ST[0][4];
	f->brushprimit_texdef.coords[0][0] = ST[1][3] - f->brushprimit_texdef.coords[0][2];
	f->brushprimit_texdef.coords[1][0] = ST[1][4] - f->brushprimit_texdef.coords[1][2];
	f->brushprimit_texdef.coords[0][1] = ST[2][3] - f->brushprimit_texdef.coords[0][2];
	f->brushprimit_texdef.coords[1][1] = ST[2][4] - f->brushprimit_texdef.coords[1][2];
}

//
// =======================================================================================================================
//    compute texture coordinates for the winding points
// =======================================================================================================================
//
void EmitBrushPrimitTextureCoordinates(face_t *f, idWinding *w, patchMesh_t *patch) {
	idVec3D	texX, texY;
	double	x, y;

	if (f== NULL || (w == NULL && patch == NULL)) {
		return;
	}

	// compute axis base
	ComputeAxisBase(f->plane.Normal(), texX, texY);

	//
	// in case the texcoords matrix is empty, build a default one same behaviour as if
	// scale[0]==0 && scale[1]==0 in old code
	//
	if (	f->brushprimit_texdef.coords[0][0] == 0 &&
			f->brushprimit_texdef.coords[1][0] == 0 &&
			f->brushprimit_texdef.coords[0][1] == 0 &&
			f->brushprimit_texdef.coords[1][1] == 0 ) {
		f->brushprimit_texdef.coords[0][0] = 1.0f;
		f->brushprimit_texdef.coords[1][1] = 1.0f;
		ConvertTexMatWithQTexture(&f->brushprimit_texdef, NULL, &f->brushprimit_texdef, f->d_texture);
	}

	int i;
	if (w) {
		for (i = 0; i < w->GetNumPoints(); i++) {
			x = DotProduct((*w)[i], texX);
			y = DotProduct((*w)[i], texY);
			(*w)[i][3] = f->brushprimit_texdef.coords[0][0] * x + f->brushprimit_texdef.coords[0][1] * y + f->brushprimit_texdef.coords[0][2];
			(*w)[i][4] = f->brushprimit_texdef.coords[1][0] * x + f->brushprimit_texdef.coords[1][1] * y + f->brushprimit_texdef.coords[1][2];
		}
	}
	
	if (patch) {
		int j;
		for ( i = 0; i < patch->width; i++ ) {
			for ( j = 0; j < patch->height; j++ ) {
				x = DotProduct(patch->ctrl(i, j).xyz, texX);
				y = DotProduct(patch->ctrl(i, j).xyz, texY);
				patch->ctrl(i, j).st.x = f->brushprimit_texdef.coords[0][0] * x + f->brushprimit_texdef.coords[0][1] * y + f->brushprimit_texdef.coords[0][2];
				patch->ctrl(i, j).st.y = f->brushprimit_texdef.coords[1][0] * x + f->brushprimit_texdef.coords[1][1] * y + f->brushprimit_texdef.coords[1][2];
			}
		}
	}
}

//
// =======================================================================================================================
//    parse a brush in brush primitive format
// =======================================================================================================================
//
void BrushPrimit_Parse(brush_t *b, bool newFormat, const idVec3 origin) {
	face_t	*f;
	int		i, j;
	GetToken(true);
	if (strcmp(token, "{")) {
		Warning("parsing brush primitive");
		return;
	}

	do {
		if (!GetToken(true)) {
			break;
		}

		if (!strcmp(token, "}")) {
			break;
		}

		// reading of b->epairs if any
		if (strcmp(token, "(")) {
			ParseEpair(&b->epairs);
		}
		else {	// it's a face
			f = Face_Alloc();
			f->next = NULL;
			if (!b->brush_faces) {
				b->brush_faces = f;
			}
			else {
				face_t	*scan;
				for (scan = b->brush_faces; scan->next; scan = scan->next)
					;
				scan->next = f;
			}

			if (newFormat) {
				// read the three point plane definition
				idPlane plane;
				for (j = 0; j < 4; j++) {
					GetToken(false);
					plane[j] = atof(token);
				}

				f->plane = plane;
				f->originalPlane = plane;
				f->dirty = false;

				//idWinding	*w = Brush_MakeFaceWinding(b, f, true);
				idWinding w;
				w.BaseForPlane( plane );

				for (j = 0; j < 3; j++) {
					f->planepts[j].x = w[j].x + origin.x;
					f->planepts[j].y = w[j].y + origin.y;
					f->planepts[j].z = w[j].z + origin.z;
				}

				GetToken(false);
			}
			else {
				for (i = 0; i < 3; i++) {
					if (i != 0) {
						GetToken(true);
					}

					if (strcmp(token, "(")) {
						Warning("parsing brush");
						return;
					}

					for (j = 0; j < 3; j++) {
						GetToken(false);
						f->planepts[i][j] = atof(token);
					}

					GetToken(false);
					if (strcmp(token, ")")) {
						Warning("parsing brush");
						return;
					}
				}
			}

			// texture coordinates
			GetToken(false);
			if (strcmp(token, "(")) {
				Warning("parsing brush primitive");
				return;
			}

			GetToken(false);
			if (strcmp(token, "(")) {
				Warning("parsing brush primitive");
				return;
			}

			for (j = 0; j < 3; j++) {
				GetToken(false);
				f->brushprimit_texdef.coords[0][j] = atof(token);
			}

			GetToken(false);
			if (strcmp(token, ")")) {
				Warning("parsing brush primitive");
				return;
			}

			GetToken(false);
			if (strcmp(token, "(")) {
				Warning("parsing brush primitive");
				return;
			}

			for (j = 0; j < 3; j++) {
				GetToken(false);
				f->brushprimit_texdef.coords[1][j] = atof(token);
			}

			GetToken(false);
			if (strcmp(token, ")")) {
				Warning("parsing brush primitive");
				return;
			}

			GetToken(false);
			if (strcmp(token, ")")) {
				Warning("parsing brush primitive");
				return;
			}

			// read the texturedef
			GetToken(false);

			// strcpy(f->texdef.name, token);
			if (g_qeglobals.mapVersion < 2.0) {
				f->texdef.SetName(va("textures/%s", token));
			}
			else {
				f->texdef.SetName(token);
			}

			if (TokenAvailable()) {
				GetToken(false);
				GetToken(false);
				GetToken(false);
				f->texdef.value = atoi(token);
			}
		}
	} while (1);
}

//
// =======================================================================================================================
//    compute a fake shift scale rot representation from the texture matrix these shift scale rot values are to be
//    understood in the local axis base
// =======================================================================================================================
//
void TexMatToFakeTexCoords(float texMat[2][3], float shift[2], float *rot, float scale[2])
{
#ifdef _DEBUG

	// check this matrix is orthogonal
	if (idMath::Fabs(texMat[0][0] * texMat[0][1] + texMat[1][0] * texMat[1][1]) > ZERO_EPSILON) {
		common->Printf("Warning : non orthogonal texture matrix in TexMatToFakeTexCoords\n");
	}
#endif
	scale[0] = idMath::Sqrt(texMat[0][0] * texMat[0][0] + texMat[1][0] * texMat[1][0]);
	scale[1] = idMath::Sqrt(texMat[0][1] * texMat[0][1] + texMat[1][1] * texMat[1][1]);
#ifdef _DEBUG
	if (scale[0] < ZERO_EPSILON || scale[1] < ZERO_EPSILON) {
		common->Printf("Warning : unexpected scale==0 in TexMatToFakeTexCoords\n");
	}
#endif
	// compute rotate value
	if (idMath::Fabs(texMat[0][0]) < ZERO_EPSILON)
	{
#ifdef _DEBUG
		// check brushprimit_texdef[1][0] is not zero
		if (idMath::Fabs(texMat[1][0]) < ZERO_EPSILON) {
			common->Printf("Warning : unexpected texdef[1][0]==0 in TexMatToFakeTexCoords\n");
		}
#endif
		// rotate is +-90
		if (texMat[1][0] > 0) {
			*rot = 90.0f;
		}
		else {
			*rot = -90.0f;
		}
	}
	else {
		*rot = RAD2DEG(atan2(texMat[1][0], texMat[0][0]));
	}

	shift[0] = -texMat[0][2];
	shift[1] = texMat[1][2];
}

//
// =======================================================================================================================
//    compute back the texture matrix from fake shift scale rot the matrix returned must be understood as a qtexture_t
//    with width=2 height=2 ( the default one )
// =======================================================================================================================
//
void FakeTexCoordsToTexMat(float shift[2], float rot, float scale[2], float texMat[2][3]) {
	texMat[0][0] = scale[0] * cos(DEG2RAD(rot));
	texMat[1][0] = scale[0] * sin(DEG2RAD(rot));
	texMat[0][1] = -1.0f * scale[1] * sin(DEG2RAD(rot));
	texMat[1][1] = scale[1] * cos(DEG2RAD(rot));
	texMat[0][2] = -shift[0];
	texMat[1][2] = shift[1];
}

//
// =======================================================================================================================
//    convert a texture matrix between two qtexture_t if NULL for qtexture_t, basic 2x2 texture is assumed ( straight
//    mapping between s/t coordinates and geometric coordinates )
// =======================================================================================================================
//
void ConvertTexMatWithQTexture(float texMat1[2][3], const idMaterial *qtex1, float texMat2[2][3], const idMaterial *qtex2, float sScale = 1.0, float tScale = 1.0) {
	float	s1, s2;
	s1 = (qtex1 ? static_cast<float>(qtex1->GetEditorImage()->uploadWidth) : 2.0f) / (qtex2 ? static_cast<float>(qtex2->GetEditorImage()->uploadWidth) : 2.0f);
	s2 = (qtex1 ? static_cast<float>(qtex1->GetEditorImage()->uploadHeight) : 2.0f) / (qtex2 ? static_cast<float>(qtex2->GetEditorImage()->uploadHeight) : 2.0f);
	s1 *= sScale;
	s2 *= tScale;
	texMat2[0][0] = s1 * texMat1[0][0];
	texMat2[0][1] = s1 * texMat1[0][1];
	texMat2[0][2] = s1 * texMat1[0][2];
	texMat2[1][0] = s2 * texMat1[1][0];
	texMat2[1][1] = s2 * texMat1[1][1];
	texMat2[1][2] = s2 * texMat1[1][2];
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ConvertTexMatWithQTexture(brushprimit_texdef_t	*texMat1, const idMaterial *qtex1, brushprimit_texdef_t *texMat2, const idMaterial *qtex2, float sScale, float tScale) {
	ConvertTexMatWithQTexture(texMat1->coords, qtex1, texMat2->coords, qtex2, sScale, tScale);
}


//
// =======================================================================================================================
//    texture locking
// =======================================================================================================================
//
void Face_MoveTexture_BrushPrimit(face_t *f, idVec3 delta) {
	idVec3D	texS, texT;
	double	tx, ty;
	idVec3D	M[3];	// columns of the matrix .. easier that way
	double	det;
	idVec3D	D[2];

	// compute plane axis base ( doesn't change with translation )
	ComputeAxisBase(f->plane.Normal(), texS, texT);

	// compute translation vector in plane axis base
	tx = DotProduct(delta, texS);
	ty = DotProduct(delta, texT);

	// fill the data vectors
	M[0][0] = tx;
	M[0][1] = 1.0f + tx;
	M[0][2] = tx;
	M[1][0] = ty;
	M[1][1] = ty;
	M[1][2] = 1.0f + ty;
	M[2][0] = 1.0f;
	M[2][1] = 1.0f;
	M[2][2] = 1.0f;
	D[0][0] = f->brushprimit_texdef.coords[0][2];
	D[0][1] = f->brushprimit_texdef.coords[0][0] + f->brushprimit_texdef.coords[0][2];
	D[0][2] = f->brushprimit_texdef.coords[0][1] + f->brushprimit_texdef.coords[0][2];
	D[1][0] = f->brushprimit_texdef.coords[1][2];
	D[1][1] = f->brushprimit_texdef.coords[1][0] + f->brushprimit_texdef.coords[1][2];
	D[1][2] = f->brushprimit_texdef.coords[1][1] + f->brushprimit_texdef.coords[1][2];

	// solve
	det = SarrusDet(M[0], M[1], M[2]);
	f->brushprimit_texdef.coords[0][0] = SarrusDet(D[0], M[1], M[2]) / det;
	f->brushprimit_texdef.coords[0][1] = SarrusDet(M[0], D[0], M[2]) / det;
	f->brushprimit_texdef.coords[0][2] = SarrusDet(M[0], M[1], D[0]) / det;
	f->brushprimit_texdef.coords[1][0] = SarrusDet(D[1], M[1], M[2]) / det;
	f->brushprimit_texdef.coords[1][1] = SarrusDet(M[0], D[1], M[2]) / det;
	f->brushprimit_texdef.coords[1][2] = SarrusDet(M[0], M[1], D[1]) / det;
}

//
// =======================================================================================================================
//    call Face_MoveTexture_BrushPrimit after idVec3D computation
// =======================================================================================================================
//
void Select_ShiftTexture_BrushPrimit(face_t *f, float x, float y, bool autoAdjust) {
#if 0
	idVec3D	texS, texT;
	idVec3D	delta;
	ComputeAxisBase(f->plane.normal, texS, texT);
	VectorScale(texS, x, texS);
	VectorScale(texT, y, texT);
	VectorCopy(texS, delta);
	VectorAdd(delta, texT, delta);
	Face_MoveTexture_BrushPrimit(f, delta);
#else
	if (autoAdjust) {
		x /= f->d_texture->GetEditorImage()->uploadWidth;
		y /= f->d_texture->GetEditorImage()->uploadHeight;
	}
	f->brushprimit_texdef.coords[0][2] += x;
	f->brushprimit_texdef.coords[1][2] += y;
	EmitBrushPrimitTextureCoordinates(f, f->face_winding);
#endif
}

//
// =======================================================================================================================
//    best fitted 2D vector is x.X+y.Y
// =======================================================================================================================
//
void ComputeBest2DVector(idVec3 v, idVec3 X, idVec3 Y, int &x, int &y) {
	double	sx, sy;
	sx = DotProduct(v, X);
	sy = DotProduct(v, Y);
	if (idMath::Fabs(sy) > idMath::Fabs(sx)) {
		x = 0;
		if (sy > 0.0) {
			y = 1;
		}
		else {
			y = -1;
		}
	}
	else {
		y = 0;
		if (sx > 0.0) {
			x = 1;
		}
		else {
			x = -1;
		}
	}
}

//
// =======================================================================================================================
//    in many case we know three points A,B,C in two axis base B1 and B2 and we want the matrix M so that A(B1) = T *
//    A(B2) NOTE: 2D homogeneous space stuff NOTE: we don't do any check to see if there's a solution or we have a
//    particular case .. need to make sure before calling NOTE: the third coord of the A,B,C point is ignored NOTE: see
//    the commented out section to fill M and D ++timo TODO: update the other members to use this when possible
// =======================================================================================================================
//
void MatrixForPoints(idVec3D M[3], idVec3D D[2], brushprimit_texdef_t *T) {
	//
	// idVec3D M[3]; // columns of the matrix .. easier that way (the indexing is not
	// standard! it's column-line .. later computations are easier that way)
	//
	double	det;

	// idVec3D D[2];
	M[2][0] = 1.0f;
	M[2][1] = 1.0f;
	M[2][2] = 1.0f;
#if 0

	// fill the data vectors
	M[0][0] = A2[0];
	M[0][1] = B2[0];
	M[0][2] = C2[0];
	M[1][0] = A2[1];
	M[1][1] = B2[1];
	M[1][2] = C2[1];
	M[2][0] = 1.0f;
	M[2][1] = 1.0f;
	M[2][2] = 1.0f;
	D[0][0] = A1[0];
	D[0][1] = B1[0];
	D[0][2] = C1[0];
	D[1][0] = A1[1];
	D[1][1] = B1[1];
	D[1][2] = C1[1];
#endif
	// solve
	det = SarrusDet(M[0], M[1], M[2]);
	T->coords[0][0] = SarrusDet(D[0], M[1], M[2]) / det;
	T->coords[0][1] = SarrusDet(M[0], D[0], M[2]) / det;
	T->coords[0][2] = SarrusDet(M[0], M[1], D[0]) / det;
	T->coords[1][0] = SarrusDet(D[1], M[1], M[2]) / det;
	T->coords[1][1] = SarrusDet(M[0], D[1], M[2]) / det;
	T->coords[1][2] = SarrusDet(M[0], M[1], D[1]) / det;
}

//
// =======================================================================================================================
//    ++timo FIXME quick'n dirty hack, doesn't care about current texture settings (angle) can be improved .. bug #107311
//    mins and maxs are the face bounding box ++timo fixme: we use the face info, mins and maxs are irrelevant
// =======================================================================================================================
//
void Face_FitTexture_BrushPrimit(face_t *f, idVec3 mins, idVec3 maxs, float height, float width) {
	idVec3D					BBoxSTMin, BBoxSTMax;
	idWinding				*w;
	int						i, j;
	double					val;
	idVec3D					M[3], D[2];

	// idVec3D N[2],Mf[2];
	brushprimit_texdef_t	N;
	idVec3D					Mf[2];


	
	//memset(f->brushprimit_texdef.coords, 0, sizeof(f->brushprimit_texdef.coords));
	//f->brushprimit_texdef.coords[0][0] = 1.0f;
	//f->brushprimit_texdef.coords[1][1] = 1.0f;
	//ConvertTexMatWithQTexture(&f->brushprimit_texdef, NULL, &f->brushprimit_texdef, f->d_texture);
	//
	// we'll be working on a standardized texture size ConvertTexMatWithQTexture(
	// &f->brushprimit_texdef, f->d_texture, &f->brushprimit_texdef, NULL ); compute
	// the BBox in ST coords
	//
	EmitBrushPrimitTextureCoordinates(f, f->face_winding);
	BBoxSTMin[0] = BBoxSTMin[1] = BBoxSTMin[2] = 999999;
	BBoxSTMax[0] = BBoxSTMax[1] = BBoxSTMax[2] = -999999;

	w = f->face_winding;
	if (w) {
		for (i = 0; i < w->GetNumPoints(); i++) {
			// AddPointToBounds in 2D on (S,T) coordinates
			for (j = 0; j < 2; j++) {
				val = (*w)[i][j + 3];
				if (val < BBoxSTMin[j]) {
					BBoxSTMin[j] = val;
				}

				if (val > BBoxSTMax[j]) {
					BBoxSTMax[j] = val;
				}
			}
		}
	}

	//
	// we have the three points of the BBox (BBoxSTMin[0].BBoxSTMin[1])
	// (BBoxSTMax[0],BBoxSTMin[1]) (BBoxSTMin[0],BBoxSTMax[1]) in ST space the BP
	// matrix we are looking for gives (0,0) (nwidth,0) (0,nHeight) coordinates in
	// (Sfit,Tfit) space to these three points we have A(Sfit,Tfit) = (0,0) = Mf *
	// A(TexS,TexT) = N * M * A(TexS,TexT) = N * A(S,T) so we solve the system for N
	// and then Mf = N * M
	//
	M[0][0] = BBoxSTMin[0];
	M[0][1] = BBoxSTMax[0];
	M[0][2] = BBoxSTMin[0];
	M[1][0] = BBoxSTMin[1];
	M[1][1] = BBoxSTMin[1];
	M[1][2] = BBoxSTMax[1];
	D[0][0] = 0.0f;
	D[0][1] = width;
	D[0][2] = 0.0f;
	D[1][0] = 0.0f;
	D[1][1] = 0.0f;
	D[1][2] = height;
	MatrixForPoints(M, D, &N);

#if 0

	//
	// FIT operation gives coordinates of three points of the bounding box in (S',T'),
	// our target axis base A(S',T')=(0,0) B(S',T')=(nWidth,0) C(S',T')=(0,nHeight)
	// and we have them in (S,T) axis base: A(S,T)=(BBoxSTMin[0],BBoxSTMin[1])
	// B(S,T)=(BBoxSTMax[0],BBoxSTMin[1]) C(S,T)=(BBoxSTMin[0],BBoxSTMax[1]) we
	// compute the N transformation so that: A(S',T') = N * A(S,T)
	//
	N[0][0] = (BBoxSTMax[0] - BBoxSTMin[0]) / width;
	N[0][1] = 0.0f;
	N[0][2] = BBoxSTMin[0];
	N[1][0] = 0.0f;
	N[1][1] = (BBoxSTMax[1] - BBoxSTMin[1]) / height;
	N[1][2] = BBoxSTMin[1];
#endif
	// the final matrix is the product (Mf stands for Mfit)
	Mf[0][0] = N.coords[0][0] *
		f->brushprimit_texdef.coords[0][0] +
		N.coords[0][1] *
		f->brushprimit_texdef.coords[1][0];
	Mf[0][1] = N.coords[0][0] *
		f->brushprimit_texdef.coords[0][1] +
		N.coords[0][1] *
		f->brushprimit_texdef.coords[1][1];
	Mf[0][2] = N.coords[0][0] *
		f->brushprimit_texdef.coords[0][2] +
		N.coords[0][1] *
		f->brushprimit_texdef.coords[1][2] +
		N.coords[0][2];
	Mf[1][0] = N.coords[1][0] *
		f->brushprimit_texdef.coords[0][0] +
		N.coords[1][1] *
		f->brushprimit_texdef.coords[1][0];
	Mf[1][1] = N.coords[1][0] *
		f->brushprimit_texdef.coords[0][1] +
		N.coords[1][1] *
		f->brushprimit_texdef.coords[1][1];
	Mf[1][2] = N.coords[1][0] *
		f->brushprimit_texdef.coords[0][2] +
		N.coords[1][1] *
		f->brushprimit_texdef.coords[1][2] +
		N.coords[1][2];

	// copy back
	VectorCopy(Mf[0], f->brushprimit_texdef.coords[0]);
	VectorCopy(Mf[1], f->brushprimit_texdef.coords[1]);

	//
	// handle the texture size ConvertTexMatWithQTexture( &f->brushprimit_texdef,
	// NULL, &f->brushprimit_texdef, f->d_texture );
	//
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Face_ScaleTexture_BrushPrimit(face_t *face, float sS, float sT) {
	if (!g_qeglobals.m_bBrushPrimitMode) {
		Sys_Status("BP mode required\n");
		return;
	}

	brushprimit_texdef_t	*pBP = &face->brushprimit_texdef;
	BPMatScale(pBP->coords, sS, sT);

	// now emit the coordinates on the winding
	EmitBrushPrimitTextureCoordinates(face, face->face_winding);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Face_RotateTexture_BrushPrimit(face_t *face, float amount, idVec3 origin) {
	brushprimit_texdef_t	*pBP = &face->brushprimit_texdef;
	if (amount) {
		float	x = pBP->coords[0][0];
		float	y = pBP->coords[0][1];
		float	x1 = pBP->coords[1][0];
		float	y1 = pBP->coords[1][1];
		float	s = sin( DEG2RAD( amount ) );
		float	c = cos( DEG2RAD( amount ) );
		pBP->coords[0][0] = (((x - origin[0]) * c) - ((y - origin[1]) * s)) + origin[0];
		pBP->coords[0][1] = (((x - origin[0]) * s) + ((y - origin[1]) * c)) + origin[1];
		pBP->coords[1][0] = (((x1 - origin[0]) * c) - ((y1 - origin[1]) * s)) + origin[0];
		pBP->coords[1][1] = (((x1 - origin[0]) * s) + ((y1 - origin[1]) * c)) + origin[1];
		EmitBrushPrimitTextureCoordinates(face, face->face_winding);
	}
}

//
// TEXTURE LOCKING (Relevant to the editor only?)
// internally used for texture locking on rotation and flipping the general
// algorithm is the same for both lockings, it's only the geometric transformation
// part that changes so I wanted to keep it in a single function if there are more
// linear transformations that need the locking, going to a C++ or code pointer
// solution would be best (but right now I want to keep brush_primit.cpp striclty
// C)
//
bool	txlock_bRotation;

// rotation locking params
int		txl_nAxis;
double	txl_fDeg;
idVec3D	txl_vOrigin;

// flip locking params
idVec3D	txl_matrix[3];
idVec3D	txl_origin;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void TextureLockTransformation_BrushPrimit(face_t *f) {
	idVec3D	Orig, texS, texT;		// axis base of initial plane

	// used by transformation algo
	idVec3D	temp;
	int		j;
	//idVec3D	vRotate;				// rotation vector

	idVec3D	rOrig, rvecS, rvecT;	// geometric transformation of (0,0) (1,0) (0,1) { initial plane axis base }
	idVec3	rNormal;
	idVec3D	rtexS, rtexT;	// axis base for the transformed plane
	idVec3D	lOrig, lvecS, lvecT;	// [2] are not used ( but usefull for debugging )
	idVec3D	M[3];
	double	det;
	idVec3D	D[2];

	// silence compiler warnings
	rOrig.Zero();
	rvecS = rOrig;
	rvecT = rOrig;
	rNormal.x = rOrig.x;
	rNormal.y = rOrig.y;
	rNormal.z = rOrig.z;

	// compute plane axis base
	ComputeAxisBase(f->plane.Normal(), texS, texT);
	Orig.x = vec3_origin.x;
	Orig.y = vec3_origin.y;
	Orig.z = vec3_origin.z;

	//
	// compute coordinates of (0,0) (1,0) (0,1) ( expressed in initial plane axis base
	// ) after transformation (0,0) (1,0) (0,1) ( expressed in initial plane axis base
	// ) <-> (0,0,0) texS texT ( expressed world axis base ) input: Orig, texS, texT
	// (and the global locking params) ouput: rOrig, rvecS, rvecT, rNormal
	//
	if (txlock_bRotation) {
/*
		// rotation vector
		vRotate.x = vec3_origin.x;
		vRotate.y = vec3_origin.y;
		vRotate.z = vec3_origin.z;
		vRotate[txl_nAxis] = txl_fDeg;
		VectorRotate3Origin(Orig, vRotate, txl_vOrigin, rOrig);
		VectorRotate3Origin(texS, vRotate, txl_vOrigin, rvecS);
		VectorRotate3Origin(texT, vRotate, txl_vOrigin, rvecT);

		// compute normal of plane after rotation
		VectorRotate3(f->plane.Normal(), vRotate, rNormal);
*/
	}
	else {
		VectorSubtract(Orig, txl_origin, temp);
		for (j = 0; j < 3; j++) {
			rOrig[j] = DotProduct(temp, txl_matrix[j]) + txl_origin[j];
		}

		VectorSubtract(texS, txl_origin, temp);
		for (j = 0; j < 3; j++) {
			rvecS[j] = DotProduct(temp, txl_matrix[j]) + txl_origin[j];
		}

		VectorSubtract(texT, txl_origin, temp);
		for (j = 0; j < 3; j++) {
			rvecT[j] = DotProduct(temp, txl_matrix[j]) + txl_origin[j];
		}

		//
		// we also need the axis base of the target plane, apply the transformation matrix
		// to the normal too..
		//
		for (j = 0; j < 3; j++) {
			rNormal[j] = DotProduct(f->plane, txl_matrix[j]);
		}
	}

	// compute rotated plane axis base
	ComputeAxisBase(rNormal, rtexS, rtexT);

	// compute S/T coordinates of the three points in rotated axis base ( in M matrix )
	lOrig[0] = DotProduct(rOrig, rtexS);
	lOrig[1] = DotProduct(rOrig, rtexT);
	lvecS[0] = DotProduct(rvecS, rtexS);
	lvecS[1] = DotProduct(rvecS, rtexT);
	lvecT[0] = DotProduct(rvecT, rtexS);
	lvecT[1] = DotProduct(rvecT, rtexT);
	M[0][0] = lOrig[0];
	M[1][0] = lOrig[1];
	M[2][0] = 1.0f;
	M[0][1] = lvecS[0];
	M[1][1] = lvecS[1];
	M[2][1] = 1.0f;
	M[0][2] = lvecT[0];
	M[1][2] = lvecT[1];
	M[2][2] = 1.0f;

	// fill data vector
	D[0][0] = f->brushprimit_texdef.coords[0][2];
	D[0][1] = f->brushprimit_texdef.coords[0][0] + f->brushprimit_texdef.coords[0][2];
	D[0][2] = f->brushprimit_texdef.coords[0][1] + f->brushprimit_texdef.coords[0][2];
	D[1][0] = f->brushprimit_texdef.coords[1][2];
	D[1][1] = f->brushprimit_texdef.coords[1][0] + f->brushprimit_texdef.coords[1][2];
	D[1][2] = f->brushprimit_texdef.coords[1][1] + f->brushprimit_texdef.coords[1][2];

	// solve
	det = SarrusDet(M[0], M[1], M[2]);
	f->brushprimit_texdef.coords[0][0] = SarrusDet(D[0], M[1], M[2]) / det;
	f->brushprimit_texdef.coords[0][1] = SarrusDet(M[0], D[0], M[2]) / det;
	f->brushprimit_texdef.coords[0][2] = SarrusDet(M[0], M[1], D[0]) / det;
	f->brushprimit_texdef.coords[1][0] = SarrusDet(D[1], M[1], M[2]) / det;
	f->brushprimit_texdef.coords[1][1] = SarrusDet(M[0], D[1], M[2]) / det;
	f->brushprimit_texdef.coords[1][2] = SarrusDet(M[0], M[1], D[1]) / det;
}

//
// =======================================================================================================================
//    texture locking called before the points on the face are actually rotated
// =======================================================================================================================
//
void RotateFaceTexture_BrushPrimit(face_t *f, int nAxis, float fDeg, idVec3 vOrigin) {
	// this is a placeholder to call the general texture locking algorithm
	txlock_bRotation = true;
	txl_nAxis = nAxis;
	txl_fDeg = fDeg;
	VectorCopy(vOrigin, txl_vOrigin);
	TextureLockTransformation_BrushPrimit(f);
}

//
// =======================================================================================================================
//    compute the new brush primit texture matrix for a transformation matrix and a flip order flag (change plane o
//    rientation) this matches the select_matrix algo used in select.cpp this needs to be called on the face BEFORE any
//    geometric transformation it will compute the texture matrix that will represent the same texture on the face after
//    the geometric transformation is done
// =======================================================================================================================
//
void ApplyMatrix_BrushPrimit(face_t *f, idMat3 matrix, idVec3 origin) {
	// this is a placeholder to call the general texture locking algorithm
	txlock_bRotation = false;
	VectorCopy(matrix[0], txl_matrix[0]);
	VectorCopy(matrix[1], txl_matrix[1]);
	VectorCopy(matrix[2], txl_matrix[2]);
	VectorCopy(origin, txl_origin);
	TextureLockTransformation_BrushPrimit(f);
}

//
// =======================================================================================================================
//    don't do C==A!
// =======================================================================================================================
//
void BPMatMul(float A[2][3], float B[2][3], float C[2][3]) {
	C[0][0] = A[0][0] * B[0][0] + A[0][1] * B[1][0];
	C[1][0] = A[1][0] * B[0][0] + A[1][1] * B[1][0];
	C[0][1] = A[0][0] * B[0][1] + A[0][1] * B[1][1];
	C[1][1] = A[1][0] * B[0][1] + A[1][1] * B[1][1];
	C[0][2] = A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2];
	C[1][2] = A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2];
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void BPMatDump(float A[2][3]) {
	common->Printf("%g %g %g\n%g %g %g\n0 0 1\n", A[0][0], A[0][1], A[0][2], A[1][0], A[1][1], A[1][2]);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void BPMatRotate(float A[2][3], float theta) {
	float	m[2][3];
	float	aux[2][3];
	memset(&m, 0, sizeof (float) *6);
	m[0][0] = cos( DEG2RAD( theta ) );
	m[0][1] = -sin( DEG2RAD( theta ) );
	m[1][0] = -m[0][1];
	m[1][1] = m[0][0];
	BPMatMul(A, m, aux);
	BPMatCopy(aux, A);
}

void Face_GetScale_BrushPrimit(face_t *face, float *s, float *t, float *rot) {
	idVec3D	texS, texT;
	ComputeAxisBase(face->plane.Normal(), texS, texT);

	if (face == NULL || face->face_winding == NULL) {
		return;
	}
	// find ST coordinates for the center of the face
	double	Os = 0, Ot = 0;
	for (int i = 0; i < face->face_winding->GetNumPoints(); i++) {
		Os += DotProduct((*face->face_winding)[i], texS);
		Ot += DotProduct((*face->face_winding)[i], texT);
	}

	Os /= face->face_winding->GetNumPoints();
	Ot /= face->face_winding->GetNumPoints();

	brushprimit_texdef_t	*pBP = &face->brushprimit_texdef;

	// here we have a special case, M is a translation and it's inverse is easy
	float					BPO[2][3];
	float					aux[2][3];
	float					m[2][3];
	memset(&m, 0, sizeof (float) *6);
	m[0][0] = 1;
	m[1][1] = 1;
	m[0][2] = -Os;
	m[1][2] = -Ot;
	BPMatMul(m, pBP->coords, aux);
	m[0][2] = Os;
	m[1][2] = Ot;			// now M^-1
	BPMatMul(aux, m, BPO);

	// apply a given scale (on S and T)
	ConvertTexMatWithQTexture(BPO, face->d_texture, aux, NULL);

	*s = idMath::Sqrt(aux[0][0] * aux[0][0] + aux[1][0] * aux[1][0]);
	*t = idMath::Sqrt(aux[0][1] * aux[0][1] + aux[1][1] * aux[1][1]);

	// compute rotate value
	if (idMath::Fabs(face->brushprimit_texdef.coords[0][0]) < ZERO_EPSILON)
	{
		// rotate is +-90
		if (face->brushprimit_texdef.coords[1][0] > 0) {
			*rot = 90.0f;
		}
		else {
			*rot = -90.0f;
		}
	}
	else {
		*rot = RAD2DEG(atan2(face->brushprimit_texdef.coords[1][0] / (*s) ? (*s) : 1.0f, face->brushprimit_texdef.coords[0][0] / (*t) ? (*t) : 1.0f));
	}


}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Face_SetExplicitScale_BrushPrimit(face_t *face, float s, float t) {
	idVec3D	texS, texT;
	ComputeAxisBase(face->plane.Normal(), texS, texT);

	// find ST coordinates for the center of the face
	double	Os = 0, Ot = 0;

	for (int i = 0; i < face->face_winding->GetNumPoints(); i++) {
		Os += DotProduct((*face->face_winding)[i], texS);
		Ot += DotProduct((*face->face_winding)[i], texT);
	}

	Os /= face->face_winding->GetNumPoints();
	Ot /= face->face_winding->GetNumPoints();

	brushprimit_texdef_t	*pBP = &face->brushprimit_texdef;

	// here we have a special case, M is a translation and it's inverse is easy
	float					BPO[2][3];
	float					aux[2][3];
	float					m[2][3];
	memset(&m, 0, sizeof (float) *6);
	m[0][0] = 1;
	m[1][1] = 1;
	m[0][2] = -Os;
	m[1][2] = -Ot;
	BPMatMul(m, pBP->coords, aux);
	m[0][2] = Os;
	m[1][2] = Ot;			// now M^-1
	BPMatMul(aux, m, BPO);

	// apply a given scale (on S and T)
	ConvertTexMatWithQTexture(BPO, face->d_texture, aux, NULL);

	// reset the scale (normalize the matrix)
	double	v1, v2;
	v1 = idMath::Sqrt(aux[0][0] * aux[0][0] + aux[1][0] * aux[1][0]);
	v2 = idMath::Sqrt(aux[0][1] * aux[0][1] + aux[1][1] * aux[1][1]);

	if (s == 0.0) {
		s = v1;
	}
	if (t == 0.0) {
		t = v2;
	}

	double	sS, sT;

	// put the values for scale on S and T here:
	sS = s / v1;
	sT = t / v2;
	aux[0][0] *= sS;
	aux[1][0] *= sS;
	aux[0][1] *= sT;
	aux[1][1] *= sT;
	ConvertTexMatWithQTexture(aux, NULL, BPO, face->d_texture);
	BPMatMul(m, BPO, aux);	// m is M^-1
	m[0][2] = -Os;
	m[1][2] = -Ot;
	BPMatMul(aux, m, pBP->coords);

	// now emit the coordinates on the winding
	EmitBrushPrimitTextureCoordinates(face, face->face_winding);
}


void Face_FlipTexture_BrushPrimit(face_t *f, bool y) {

	float s, t, rot;
	Face_GetScale_BrushPrimit(f, &s, &t, &rot);
	if (y) {
		Face_SetExplicitScale_BrushPrimit(f, 0.0, -t);
	} else {
		Face_SetExplicitScale_BrushPrimit(f, -s, 0.0);
	}
#if 0

	idVec3D	texS, texT;
	ComputeAxisBase(f->plane.normal, texS, texT);
	double	Os = 0, Ot = 0;
	for (int i = 0; i < f->face_winding->numpoints; i++) {
		Os += DotProduct(f->face_winding->p[i], texS);
		Ot += DotProduct(f->face_winding->p[i], texT);
	}

	Ot = abs(Ot);
	Ot *= t;
	Ot /= f->d_texture->GetEditorImage()->uploadHeight;

	Os = abs(Os);
	Os *= s;
	Os /= f->d_texture->GetEditorImage()->uploadWidth;

	
	if (y) {
		Face_FitTexture_BrushPrimit(f, texS, texT, -Ot, 1.0);
	} else {
		Face_FitTexture_BrushPrimit(f, texS, texT, 1.0, -Os);
	}
	EmitBrushPrimitTextureCoordinates(f, f->face_winding);
#endif
}

void Brush_FlipTexture_BrushPrimit(brush_t *b, bool y) {
	for (face_t *f = b->brush_faces; f; f = f->next) {
		Face_FlipTexture_BrushPrimit(f, y);
	}
}

void Face_SetAxialScale_BrushPrimit(face_t *face, bool y) {

	if (!face) {
		return;
	}

	if (!face->face_winding) {
		return;
	}

	//float oldS, oldT, oldR;
	//Face_GetScale_BrushPrimit(face, &oldS, &oldT, &oldR);

	idVec3D min, max;
	min.x = min.y = min.z = 999999.0;
	max.x = max.y = max.z = -999999.0;
	for (int i = 0; i < face->face_winding->GetNumPoints(); i++) {
		for (int j = 0; j < 3; j++) {
			if ((*face->face_winding)[i][j] < min[j]) {
				min[j] = (*face->face_winding)[i][j];
			}
			if ((*face->face_winding)[i][j] > max[j]) {
				max[j] = (*face->face_winding)[i][j];
			}
		}
	}
	
	idVec3 len;

	if (g_bAxialMode) {
		if (g_axialAnchor >= 0 && g_axialAnchor < face->face_winding->GetNumPoints() &&
			g_axialDest >= 0 && g_axialDest < face->face_winding->GetNumPoints() &&
			g_axialAnchor != g_axialDest) {
				len = (*face->face_winding)[g_axialDest].ToVec3() - (*face->face_winding)[g_axialAnchor].ToVec3();
		} else {
			return;
		}
	} else {
		if (y) {
			len = (*face->face_winding)[2].ToVec3() - (*face->face_winding)[1].ToVec3();
		} else {
			len = (*face->face_winding)[1].ToVec3() - (*face->face_winding)[0].ToVec3();
		}
	}

	double dist = len.Length();
	double width = idMath::Fabs(max.x - min.x);
	double height = idMath::Fabs(max.z - min.z);

	//len = maxs[2] - mins[2];
	//double yDist = len.Length();


	if (dist != 0.0) {
		if (dist > face->d_texture->GetEditorImage()->uploadHeight) {
			height = 1.0 / (dist / face->d_texture->GetEditorImage()->uploadHeight);
		} else {
			height /= dist;
		}
		if (dist > face->d_texture->GetEditorImage()->uploadWidth) {
			width = 1.0 / (dist / face->d_texture->GetEditorImage()->uploadWidth);
		} else {
			width /= dist;
		}
	}

	if (y) {
		Face_SetExplicitScale_BrushPrimit(face, 0.0, height);
		//oldT = oldT / height * 10;
		//Select_ShiftTexture_BrushPrimit(face, 0, -oldT, true);
	} else {
		Face_SetExplicitScale_BrushPrimit(face, width, 0.0);
	}
/*
	common->Printf("Face x: %f  y: %f  xr: %f  yr: %f\n", x, y, xRatio, yRatio);
	common->Printf("Texture x: %i  y: %i  \n",face->d_texture->GetEditorImage()->uploadWidth, face->d_texture->GetEditorImage()->uploadHeight);

	idVec3D texS, texT;
	ComputeAxisBase(face->plane.normal, texS, texT);
	float	Os = 0, Ot = 0;
	for (int i = 0; i < face->face_winding->numpoints; i++) {
		Os += DotProduct(face->face_winding->p[i], texS);
		Ot += DotProduct(face->face_winding->p[i], texT);
	}

	common->Printf("Face2 x: %f  y: %f  \n", Os, Ot);
	Os /= face->face_winding->numpoints;
	Ot /= face->face_winding->numpoints;


	//Os /= face->face_winding->numpoints;
	//Ot /= face->face_winding->numpoints;

*/
}


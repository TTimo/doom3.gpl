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
#ifndef _QERTYPE_H
#define _QERTYPE_H

#define	MAXPOINTS	16

class texdef_t
{
public:
	texdef_t()
	{
		name = "";
		shift[0] = shift[1] = 0.0;
		rotate = 0;	
		scale[0] = scale[1] = 0;
		value = 0;
	}
	~texdef_t()
	{
		if ( name && name[0] ) {
			delete []name;
		}
		name = NULL;
	}

	void SetName( const char *p )
	{
		if ( name && name[0] ) {
			delete []name;
		}
		if ( p && p[0] ) {
			name = strcpy( new char[strlen(p)+1], p );
		}
		else {
			name = "";
		}
	}

	texdef_t& operator =(const texdef_t& rhs)
	{
		if ( &rhs != this ) {
			SetName(rhs.name);
			shift[0] = rhs.shift[0];
			shift[1] = rhs.shift[1];
			rotate = rhs.rotate;
			scale[0] = rhs.scale[0];
			scale[1] = rhs.scale[1];
			value = rhs.value;
		}
		return *this;
	}
	//char	name[128];
	char *	name;
	float	shift[2];
	float	rotate;
	float	scale[2];
	int		value;
};

// Timo
// new brush primitive texdef
//typedef struct brushprimit_texdef_s
//{
//	float	coords[2][3];
//} brushprimit_texdef_t;

class brushprimit_texdef_t {
public:
	float	coords[2][3];
	brushprimit_texdef_t() {
		memset(&coords, 0, sizeof(coords));
		coords[0][0] = 1.0;
		coords[1][1] = 1.0;
	}
};

class texturewin_t
{
public:
	texturewin_t() {
		memset(&brushprimit_texdef.coords, 0, sizeof(brushprimit_texdef.coords));
		brushprimit_texdef.coords[0][0] = 1.0;
		brushprimit_texdef.coords[1][1] = 1.0;
	}

	~texturewin_t() {
	}
	int			width, height;
	int			originy;
	// add brushprimit_texdef_t for brush primitive coordinates storage
	brushprimit_texdef_t	brushprimit_texdef;
	int m_nTotalHeight;
	// surface plugin, must be casted to a IPluginTexdef*
	void* pTexdef;
	texdef_t	texdef;
};

#define QER_TRANS     0x00000001
#define QER_NOCARVE   0x00000002

typedef struct qtexture_s
{
	struct	qtexture_s *next;
	char	name[64];		// includes partial directory and extension
	int		width,  height;
	int		contents;
	int		flags;
	int		value;
	int		texture_number;	// gl bind number
  
	// name of the .shader file
  char  shadername[1024]; // old shader stuff
  bool bFromShader;   // created from a shader
  float fTrans;           // amount of transparency
  int   nShaderFlags;     // qer_ shader flags
	idVec3	color;			    // for flat shade mode
	bool	inuse;		    // true = is present on the level

	// cast this one to an IPluginQTexture if you are using it
	// NOTE: casting can be done with a GETPLUGINQTEXTURE defined in isurfaceplugin.h
	// TODO: if the __ISURFACEPLUGIN_H_ header is used, use a union { void *pData; IPluginQTexture *pPluginQTexture } kind of thing ?
	void					*pData;

	//++timo FIXME: this is the actual filename of the texture
	// this will be removed after shader code cleanup
	char filename[64];

} qtexture_t;

//++timo texdef and brushprimit_texdef are static
// TODO : do dynamic ?
typedef struct face_s
{
	struct face_s			*next;
	struct face_s			*original;		//used for vertex movement
	idVec3					planepts[3];
	idVec3					orgplanepts[3];	// used for arbitrary rotation
	texdef_t				texdef;

	idPlane					plane;
	idPlane					originalPlane;
	bool					dirty;

	idWinding				*face_winding;

	idVec3					d_color;
	const idMaterial		*d_texture;

	// Timo new brush primit texdef
	brushprimit_texdef_t	brushprimit_texdef;

	// cast this one to an IPluginTexdef if you are using it
	// NOTE: casting can be done with a GETPLUGINTEXDEF defined in isurfaceplugin.h
	// TODO: if the __ISURFACEPLUGIN_H_ header is used, use a union { void *pData; IPluginTexdef *pPluginTexdef } kind of thing ?
	void					*pData;
} face_t;

typedef struct {
	idVec3	xyz;
	float	sideST[2];
	float	capST[2];
} curveVertex_t;

typedef struct {
	curveVertex_t	v[2];
} sideVertex_t;


#define	MIN_PATCH_WIDTH		3
#define	MIN_PATCH_HEIGHT 	3

#define	MAX_PATCH_WIDTH		64
#define	MAX_PATCH_HEIGHT	64

// patch type info
// type in lower 16 bits, flags in upper
// endcaps directly follow this patch in the list

// types
#define PATCH_GENERIC     0x00000000    // generic flat patch
#define PATCH_CYLINDER    0x00000001    // cylinder
#define PATCH_BEVEL       0x00000002    // bevel
#define PATCH_ENDCAP      0x00000004    // endcap
#define PATCH_HEMISPHERE  0x00000008    // hemisphere
#define PATCH_CONE        0x00000010    // cone
#define PATCH_TRIANGLE    0x00000020    // simple tri, assumes 3x3 patch

// behaviour styles
#define PATCH_CAP         0x00001000    // flat patch applied as a cap
#define PATCH_SEAM        0x00002000    // flat patch applied as a seam
#define PATCH_THICK       0x00004000    // patch applied as a thick portion

// styles
#define PATCH_BEZIER      0x00000000    // default bezier
#define PATCH_BSPLINE     0x10000000    // bspline

#define PATCH_TYPEMASK     0x00000fff    // 
#define PATCH_BTYPEMASK    0x0000f000    // 
#define PATCH_STYLEMASK    0xffff0000    // 


struct brush_s;
typedef struct brush_s brush_t;

typedef struct {
	int			width, height;		// in control points, not patches
	int			horzSubdivisions;
	int			vertSubdivisions;
	bool		explicitSubdivisions;
	int			contents, flags, value, type;
	const idMaterial *d_texture;
	idDrawVert *verts;
	//idDrawVert *ctrl;
	brush_t *	pSymbiot;
	bool		bSelected;
	bool		bOverlay;
	int			nListID;
	int			nListIDCam;
	int			nListSelected;

	idDict *	epairs;
	// cast this one to an IPluginTexdef if you are using it
	// NOTE: casting can be done with a GETPLUGINTEXDEF defined in isurfaceplugin.h
	// TODO: if the __ISURFACEPLUGIN_H_ header is used, use a union { void *pData; IPluginTexdef *pPluginTexdef } kind of thing ?
	void *		pData;
	ID_INLINE idDrawVert &ctrl( int col, int row ) {
		if ( col < 0 || col >= width || row < 0 || row >= height ) {
			common->Warning( "patchMesh_t::ctrl: control point out of range" );
			return verts[0];
		}
		else {
			return verts[row * width + col];
		}
	}
} patchMesh_t;

enum {
	LIGHT_TARGET,
  LIGHT_RIGHT,
	LIGHT_UP,
	LIGHT_RADIUS,
	LIGHT_X,
	LIGHT_Y,
	LIGHT_Z, 
	LIGHT_START,
	LIGHT_END,
	LIGHT_CENTER
};


typedef struct brush_s
{
	struct brush_s	*prev, *next;	// links in active/selected
	struct brush_s	*oprev, *onext;	// links in entity
	brush_t *   list;				//keep a handy link to the list its in
	struct entity_s	*owner;
	idVec3 mins, maxs;

	idVec3	lightCenter;			// for moving the shading center of point lights
	idVec3	lightRight;
	idVec3	lightTarget;
	idVec3	lightUp;
	idVec3	lightRadius;
	idVec3	lightOffset;
	idVec3	lightColor;
	idVec3	lightStart;
	idVec3	lightEnd;
	bool	pointLight;
	bool	startEnd;
	int		lightTexture;

	bool	trackLightOrigin;	// this brush is a special case light brush
	bool	entityModel;

	face_t  *brush_faces;

	bool bModelFailed;
	//
	// curve brush extensions
	// all are derived from brush_faces
	bool	hiddenBrush;
	bool	forceWireFrame;
	bool	forceVisibile;

	patchMesh_t *pPatch;
	struct entity_s *pUndoOwner;

	int undoId;						//undo ID
	int redoId;						//redo ID
	int ownerId;					//entityId of the owner entity for undo

	// TTimo: HTREEITEM is MFC, some plugins really don't like it
#ifdef QERTYPES_USE_MFC
	int numberId;         // brush number
	HTREEITEM itemOwner;  // owner for grouping
#else
	int numberId;
	DWORD itemOwner;
#endif

	idRenderModel	*modelHandle;

	// brush primitive only
	idDict	epairs;

} brush_t;


#define	MAX_FLAGS	8


typedef struct trimodel_t
{
  idVec3 v[3];
  float  st[3][2];
} trimodel;


// eclass show flags

#define		ECLASS_LIGHT			0x00000001
#define		ECLASS_ANGLE			0x00000002
#define		ECLASS_PATH				0x00000004
#define		ECLASS_MISCMODEL		0x00000008
#define		ECLASS_PLUGINENTITY		0x00000010
#define		ECLASS_PROJECTEDLIGHT	0x00000020
#define		ECLASS_WORLDSPAWN		0x00000040
#define		ECLASS_SPEAKER			0x00000080
#define		ECLASS_PARTICLE			0x00000100
#define		ECLASS_ROTATABLE		0x00000200
#define		ECLASS_CAMERAVIEW		0x00000400
#define		ECLASS_MOVER			0x00000800
#define		ECLASS_ENV				0x00001000
#define		ECLASS_COMBATNODE		0x00002000
#define		ECLASS_LIQUID			0x00004000

enum EVAR_TYPES {
	EVAR_STRING,
	EVAR_INT,		
	EVAR_FLOAT,
	EVAR_BOOL,
	EVAR_COLOR,
	EVAR_MATERIAL,
	EVAR_MODEL,
	EVAR_GUI,
	EVAR_SOUND
};

typedef struct evar_s {
	int	type;
	idStr name;
	idStr desc;
} evar_t;

typedef struct eclass_s
{
	struct eclass_s *next;
	idStr	name;
	bool	fixedsize;
	bool	unknown;		// wasn't found in source
	idVec3	mins, maxs;
	idVec3	color;
	texdef_t texdef;
	idStr	comments;
	idStr	desc;

	idRenderModel *modelHandle;
	idRenderModel *entityModel;

	int   nFrame;
	unsigned int nShowFlags;
	idStr	defMaterial;
	idDict	args;
	idDict	defArgs;
	idList<evar_t> vars;

	HMODULE	hPlug;
} eclass_t;

extern	eclass_t	*eclass;

/*
** window bits
*/
#define	W_CAMERA		0x0001
#define	W_XY			0x0002
#define	W_XY_OVERLAY	0x0004
#define	W_Z				0x0008
#define	W_TEXTURE		0x0010
#define	W_Z_OVERLAY		0x0020
#define W_CONSOLE		0x0040
#define W_ENTITY		0x0080
#define W_CAMERA_IFON	0x0100
#define W_XZ			0x0200  //--| only used for patch vertex manip stuff
#define W_YZ			0x0400  //--|
#define W_MEDIA			0x1000 
#define W_GAME			0x2000 
#define	W_ALL			0xFFFFFFFF

// used in some Drawing routines
enum VIEWTYPE {YZ, XZ, XY};

#endif

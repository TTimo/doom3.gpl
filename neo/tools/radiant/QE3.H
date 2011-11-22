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

#ifndef __QE3_H__
#define __QE3_H__

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

// this define to use HTREEITEM and MFC stuff in the headers
#define QERTYPES_USE_MFC
#include "qertypes.h"
#include "cmdlib.h"
#include "parse.h"

#include <commctrl.h>
#include "afxres.h"
#include "../../sys/win32/rc/Radiant_resource.h"       // main symbols
#include "../../sys/win32/win_local.h"

#include "qedefs.h"

// stuff from old qfiles.h
#define	MAX_MAP_ENTITIES	2048
const int MAX_MOVE_POINTS = 4096;
const int MAX_MOVE_PLANES = 2048;

// assume that all maps fit within this
const float	HUGE_DISTANCE = 100000;

#include "textures.h"
#include "EditorBrush.h"
#include "EditorEntity.h"
#include "EditorMap.h"
#include "select.h"
#include "splines.h"

#include "z.h"
#include "mru.h"
#include "waitdlg.h"
#include "MainFrm.h"
#include "PrefsDlg.h"
#include "FindTextureDlg.h"
#include "dialogtextures.h"
#include "InspectorDialog.h"
#include "undo.h"
#include "PMESH.H"

// the dec offsetof macro doesn't work very well...
#define myoffsetof(type,identifier) ((size_t)&((type *)0)->identifier)


void	Error (char *error, ...);
void	Warning (char *error, ...);

typedef struct
{
	int		p1, p2;
	face_t	*f1, *f2;
} pedge_t;

typedef struct
{
	int		iSize;
	int		iTexMenu;		// nearest, linear, etc
	float	fGamma;			// gamma for textures
	char	szProject[256];	// last project loaded
	idVec3	colors[COLOR_LAST];
	bool	show_names;
	bool	show_coordinates;
	int		exclude;
	float	m_nTextureTweak;
	bool	editorExpanded;
	RECT	oldEditRect;
	bool	showSoundAlways;
	bool	showSoundWhenSelected;
} SavedInfo_t;

//
// system functions
//
// TTimo NOTE: WINAPI funcs can be accessed by plugins
void    Sys_UpdateStatusBar( void );
void    WINAPI Sys_UpdateWindows (int bits);
void    Sys_Beep (void);
double	Sys_DoubleTime (void);
void    Sys_GetCursorPos (int *x, int *y);
void    Sys_SetCursorPos (int x, int y);
void    Sys_SetTitle (const char *text);
void    Sys_BeginWait (void);
bool	Sys_Waiting();
void    Sys_EndWait (void);
void    Sys_Status(const char *psz, int part = -1);

/*
** most of the QE globals are stored in this structure
*/
typedef struct
{
	bool d_showgrid;
	float d_gridsize;

	int	rotateAxis;			// 0, 1 or 2
	int flatRotation;		// 0, 1 or 2, 0 == off, 1 == rotate about the rotation origin, 1 == rotate about the selection mid point
	
	int      d_num_entities;
	
	entity_t *d_project_entity;

	idVec3	d_new_brush_bottom, d_new_brush_top;
	
	HINSTANCE d_hInstance;
	
	idVec3    d_points[MAX_POINTS];
	int       d_numpoints;
	pedge_t   d_edges[MAX_EDGES];
	int       d_numedges;
	
	int       d_num_move_points;
	idVec3    *d_move_points[MAX_MOVE_POINTS];

	int			d_num_move_planes;
	idPlane		*d_move_planes[MAX_MOVE_PLANES];
	
	qtexture_t	*d_qtextures;
	
	texturewin_t d_texturewin;
	
	int	         d_pointfile_display_list;
	
	LPMRUMENU    d_lpMruMenu;
	
	SavedInfo_t  d_savedinfo;
	
	int          d_workcount;
	
	// connect entities uses the last two brushes selected
	int			 d_select_count;
	brush_t		*d_select_order[MAX_MAP_ENTITIES];
	idVec3       d_select_translate;    // for dragging w/o making new display lists
	select_t     d_select_mode;
	idPointListInterface *selectObject;	// 
	
	int		     d_font_list;
	
	int          d_parsed_brushes;
	
	bool	show_blocks;
	
	// Timo
	// tells if we are internally using brush primitive (texture coordinates and map format)
	// this is a shortcut for IntForKey( g_qeglobals.d_project_entity, "brush_primit" )
	// NOTE: must keep the two ones in sync
	BOOL m_bBrushPrimitMode;

	// used while importing brush data from file or memory buffer
	// tells if conversion between map format and internal preferences ( m_bBrushPrimitMode ) is needed
	bool	bNeedConvert;
	bool	bOldBrushes;
	bool	bPrimitBrushes;
	float	mapVersion;
	
	idVec3  d_vAreaTL;
	idVec3  d_vAreaBR;
	
	// tells if we are using .INI files for prefs instead of registry
	bool	use_ini;
	// even in .INI mode we use the registry for all void* prefs
	char		use_ini_registry[64];
	
	//Timo
	// tells we have surface properties plugin
	bool	bSurfacePropertiesPlugin;
	// tells we are using a BSP frontend plugin
	bool	bBSPFrontendPlugin;
	
	// the editor has its own soundWorld and renderWorld, completely distinct from the game
	idRenderWorld	*rw;
	idSoundWorld	*sw;
} QEGlobals_t;


void Pointfile_Delete (void);
void WINAPI Pointfile_Check (void);
void Pointfile_Next (void);
void Pointfile_Prev (void);
void Pointfile_Clear (void);
void Pointfile_Draw( void );
void Pointfile_Load( void );

//
// drag.c
//
void Drag_Begin (int x, int y, int buttons,
		   const idVec3 &xaxis, const idVec3 &yaxis,
		   const idVec3 &origin, const idVec3 &dir);
void Drag_MouseMoved (int x, int y, int buttons);
void Drag_MouseUp (int nButtons = 0);
extern bool g_moveOnly;
//
// csg.c
//
void CSG_MakeHollow (void);
void CSG_Subtract (void);
void CSG_Merge (void);

//
// vertsel.c
//

void SetupVertexSelection (void);
void SelectEdgeByRay (idVec3 org, idVec3 dir);
void SelectVertexByRay (idVec3 org, idVec3 dir);

void ConnectEntities (void);

extern	int	update_bits;

extern	int	screen_width;
extern	int	screen_height;

extern	HANDLE	bsp_process;
extern HANDLE g_hBSPOutput;
extern HANDLE g_hBSPInput;


char	*TranslateString (char *buf);

void ProjectDialog (void);

void FillTextureMenu (CStringArray* pArray = NULL);
void FillBSPMenu (void);

BOOL CALLBACK Win_Dialog (
    HWND hwndDlg,	// handle to dialog box
    UINT uMsg,	// message
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
);


//
// win_cam.c
//
void WCam_Create (HINSTANCE hInstance);


//
// win_xy.c
//
void WXY_Create (HINSTANCE hInstance);

//
// win_z.c
//
void WZ_Create (HINSTANCE hInstance);

//
// win_ent.c
//


//
// win_main.c
//
void Main_Create (HINSTANCE hInstance);
extern bool SaveWindowState(HWND hWnd, const char *pszName);
extern bool LoadWindowState(HWND hWnd, const char *pszName);

extern bool SaveRegistryInfo(const char *pszName, void *pvBuf, long lSize);
extern bool LoadRegistryInfo(const char *pszName, void *pvBuf, long *plSize);



// win_dlg.c

void DoGamma(void);
void DoFind(void);
void DoRotate(void);
void DoSides(bool bCone = false, bool bSphere = false, bool bTorus = false);
void DoAbout(void);
void DoSurface();

/*
** QE function declarations
*/
void	QE_CheckAutoSave( void );
void	QE_CountBrushesAndUpdateStatusBar( void );
void	WINAPI QE_CheckOpenGLForErrors(void);
void	QE_ExpandBspString (char *bspaction, char *out, char *mapname, bool useTemps);
void	QE_Init (void);
bool	QE_KeyDown (int key, int nFlags = 0);
bool	QE_LoadProject (char *projectfile);
bool	QE_SingleBrush (bool bQuiet = false, bool entityOK = false);


// sys stuff
void Sys_MarkMapModified (void);

/*
** QE Win32 function declarations
*/
int  WINAPI QEW_SetupPixelFormat(HDC hDC, bool zbuffer );

/*
** extern declarations
*/
extern QEGlobals_t   g_qeglobals;
extern int mapModified;		// for quit confirmation (0 = clean, 1 = unsaved,

//++timo clean (moved into qertypes.h)
//enum VIEWTYPE {YZ, XZ, XY};


extern bool g_bAxialMode;
extern int g_axialAnchor;
extern int g_axialDest;

extern void Face_FlipTexture_BrushPrimit(face_t *face, bool y);
extern void Brush_FlipTexture_BrushPrimit(brush_t *b, bool y);

// Timo
// new brush primitive stuff
//void ComputeAxisBase( idVec3 &normal,idVec3 &texS,idVec3 &texT );
void FaceToBrushPrimitFace(face_t *f);
void EmitBrushPrimitTextureCoordinates(face_t *, idWinding *, patchMesh_t *patch = NULL);
// EmitTextureCoordinates, is old code used for brush to brush primitive conversion
void EmitTextureCoordinates ( idVec5 &xyzst, const idMaterial *q, face_t *f, bool force = false);
void BrushPrimit_Parse(brush_t *, bool newFormat, const idVec3 origin);
// compute a fake shift scale rot representation from the texture matrix
void TexMatToFakeTexCoords( float texMat[2][3], float shift[2], float *rot, float scale[2] );
void FakeTexCoordsToTexMat( float shift[2], float rot, float scale[2], float texMat[2][3] );
void ConvertTexMatWithQTexture( brushprimit_texdef_t *texMat1, const idMaterial *qtex1, brushprimit_texdef_t *texMat2, const idMaterial *qtex2, float sScale = 1.0, float tScale = 1.0 );
// texture locking
void Face_MoveTexture_BrushPrimit(face_t *f, idVec3 delta);
void Select_ShiftTexture_BrushPrimit( face_t *f, float x, float y, bool autoAdjust );
void RotateFaceTexture_BrushPrimit(face_t *f, int nAxis, float fDeg, idVec3 vOrigin );
// used in CCamWnd::ShiftTexture_BrushPrimit
void ComputeBest2DVector( idVec3 v, idVec3 X, idVec3 Y, int &x, int &y );

void ApplyMatrix_BrushPrimit(face_t *f, idMat3 matrix, idVec3 origin);
// low level functions .. put in mathlib?
#define BPMatCopy(a,b) {b[0][0] = a[0][0]; b[0][1] = a[0][1]; b[0][2] = a[0][2]; b[1][0] = a[1][0]; b[1][1] = a[1][1]; b[1][2] = a[1][2];}
// apply a scale transformation to the BP matrix
#define BPMatScale(m,sS,sT) {m[0][0]*=sS; m[1][0]*=sS; m[0][1]*=sT; m[1][1]*=sT;}
// apply a translation transformation to a BP matrix
#define BPMatTranslate(m,s,t) {m[0][2] += m[0][0]*s + m[0][1]*t; m[1][2] += m[1][0]*s+m[1][1]*t;}
// 2D homogeneous matrix product C = A*B
void BPMatMul(float A[2][3], float B[2][3], float C[2][3]);
// apply a rotation (degrees)
void BPMatRotate(float A[2][3], float theta);
#ifdef _DEBUG
void BPMatDump(float A[2][3]);
#endif


//
// eclass.cpp
//
extern bool parsing_single;
extern bool eclass_found;
extern eclass_t *eclass_e;
void Eclass_ScanFile( char *filename );
void FillClassList (void);

extern bool g_bShowLightVolumes;
extern bool g_bShowLightTextures;
extern const idMaterial *Texture_LoadLight(const char *name);

#define	FONT_HEIGHT	10

void UniqueTargetName(idStr& rStr);

#define MAP_VERSION 2.0

extern CMainFrame* g_pParentWnd;
extern CString g_strAppPath;
extern CPrefsDlg& g_PrefsDlg;
extern CFindTextureDlg& g_dlgFind;
extern idCVar radiant_entityMode;

// layout styles
#define QR_SPLIT 0
#define QR_QE4 1
#define QR_4WAY 2
#define QR_SPLITZ 3


// externs
extern void AddSlash(CString&);
extern void DLLBuildDone();
extern void CleanUpEntities();
extern void QE_CountBrushesAndUpdateStatusBar();
extern void	QE_CheckAutoSave();
extern qtexture_t	*notexture;
extern qtexture_t	*current_texture;
extern bool SaveWindowState(HWND hWnd, const char *pszName);
extern void Map_Snapshot();
extern void WXY_Print();
extern void AddProp( void );
extern int inspector_mode;
extern bool g_bRotateMode;
extern bool g_bClipMode;
extern bool g_bScaleMode;
extern int g_nScaleHow;
extern bool g_bPathMode;
extern void RunScript(char* pBuffer);
extern HINSTANCE g_hOpenGL32;

extern void FindReplaceTextures(const char* pFind, const char* pReplace, bool bSelected, bool bForce);
extern void DoProjectSettings();
extern bool region_active;
extern void	Texture_ShowDirectory (char* pPath, bool Linked = false);
extern void Map_ImportFile (char *filename);
extern void Map_SaveSelected(char* pFilename);
extern bool g_bNewFace;
extern bool g_bSwitch;
extern brush_t g_brFrontSplits;
extern brush_t g_brBackSplits;
extern CClipPoint g_Clip1;
extern CClipPoint g_Clip2;
extern brush_t* g_pSplitList;
extern CClipPoint g_PathPoints[256];
extern void AcquirePath(int nCount, PFNPathCallback* pFunc);
extern bool g_bScreenUpdates;
extern SCommandInfo g_Commands[];
extern int g_nCommandCount;
extern SKeyInfo g_Keys[];
extern int g_nKeyCount;
extern int inspector_mode;
extern const char	*bsp_commands[256];
extern void HandlePopup(CWnd* pWindow, unsigned int uId);
extern z_t z;
extern CString g_strProject;
extern void TextureAxisFromPlane( const idPlane &pln, idVec3 &xv, idVec3 &yv);
extern bool QE_SaveProject (const char* pProjectFile);
extern void Clamp(float& f, int nClamp);
extern bool WriteFileString( FILE *fp, char *string, ... );
extern void MemFile_fprintf(CMemFile* pMemFile, const char* pText, ...);
extern void SaveWindowPlacement(HWND hwnd, const char* pName);
extern bool LoadWindowPlacement(HWND hwnd, const char* pName);
extern bool ConfirmModified (void);
extern void DoPatchInspector();
void UpdatePatchInspector();
extern int g_nSmartX;
extern int g_nSmartY;
extern brush_t* CreateEntityBrush(int x, int y, CXYWnd* pWnd);
int PointInMoveList( idVec3 *pf );

extern bool ByeByeSurfaceDialog();
extern void UpdateSurfaceDialog();
extern void UpdateLightInspector();

BOOL UpdateEntitySel(eclass_t *pec);	
void SetInspectorMode(int iType);
BOOL GetSelectAllCriteria(CString &strKey, CString &strVal);

int GetCvarInt(const char *name, const int def);
const char *GetCvarString(const char *name, const char *def);
void SetCvarInt(const char *name, const int value);
void SetCvarString(const char *name, const char *value);
void SetCvarBinary(const char *name, void *pv, int size);
bool GetCvarBinary(const char *name, void *pv, int size);

void UpdateRadiantColor( float r, float g, float b, float a );

#endif /* !__QE3_H__ */

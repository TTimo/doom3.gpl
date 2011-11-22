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
#include "shlobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MOUSE_KEY				"radiant_MouseButtons"
#define TLOCK_KEY				"radiant_TextureLock"
#define RLOCK_KEY				"radiant_RotateLock"
#define LOADLAST_KEY			"radiant_LoadLast"
#define LOADLASTMAP_KEY			"radiant_LoadLastMap"
#define LASTPROJ_KEY			"radiant_LastProject"
#define LASTMAP_KEY				"radiant_LastMap"
#define RUN_KEY					"radiant_RunBefore"
#define FACE_KEY				"radiant_NewFaceGrab"
#define BSP_KEY					"radiant_InternalBSP"
#define RCLICK_KEY				"radiant_NewRightClick"
#define VERTEX_KEY				"radiant_NewVertex"
#define AUTOSAVE_KEY			"radiant_Autosave"
#define AUTOSAVETIME_KEY		"radiant_AutosaveMinutes"
#define PAK_KEY					"radiant_UsePAK"
#define NEWAPPLY_KEY			"radiant_ApplyDismissesSurface"
#define HACK_KEY				"radiant_Gatewayescapehack"
#define TEXTURE_KEY				"radiant_NewTextureWindowStuff"
#define TINYBRUSH_KEY			"radiant_CleanTinyBrushes"
#define TINYSIZE_KEY			"radiant_CleanTinyBrusheSize"
#define SNAPSHOT_KEY			"radiant_Snapshots"
#define PAKFILE_KEY				"radiant_PAKFile"
#define STATUS_KEY				"radiant_StatusPointSize"
#define MOVESPEED_KEY			"radiant_MoveSpeed"
#define ANGLESPEED_KEY			"radiant_AngleSpeed"
#define SETGAME_KEY				"radiant_UseSetGame"
#define CAMXYUPDATE_KEY			"radiant_CamXYUpdate"
#define LIGHTDRAW_KEY			"radiant_NewLightStyle"
#define WHATGAME_KEY			"radiant_WhichGame"
#define CUBICCLIP_KEY			"radiant_CubicClipping"
#define CUBICSCALE_KEY			"radiant_CubicScale"
#define ALTEDGE_KEY				"radiant_ALTEdgeDrag"
#define FACECOLORS_KEY			"radiant_FaceColors"
#define QE4PAINT_KEY			"radiant_QE4Paint"
#define SNAPT_KEY				"radiant_SnapT"
#define XZVIS_KEY				"radiant_XZVIS"
#define YZVIS_KEY				"radiant_YZVIS"
#define ZVIS_KEY				"radiant_ZVIS"
#define SIZEPAINT_KEY			"radiant_SizePainting"
#define DLLENTITIES_KEY			"radiant_DLLEntities"
#define WIDETOOLBAR_KEY			"radiant_WideToolBar"
#define NOCLAMP_KEY				"radiant_NoClamp"
#define PREFAB_KEY				"radiant_PrefabPath"
#define USERINI_KEY				"radiant_UserINIPath"
#define ROTATION_KEY			"radiant_Rotation"
#define SGIOPENGL_KEY			"radiant_SGIOpenGL"
#define BUGGYICD_KEY			"radiant_BuggyICD"
#define HICOLOR_KEY				"radiant_HiColorTextures"
#define CHASEMOUSE_KEY			"radiant_ChaseMouse"
#define ENTITYSHOW_KEY			"radiant_EntityShow"
#define TEXTURESCALE_KEY		"radiant_TextureScale"
#define TEXTURESCROLLBAR_KEY	"radiant_TextureScrollbar"
#define DISPLAYLISTS_KEY		"radiant_UseDisplayLists"
#define NORMALIZECOLORS_KEY		"radiant_NormalizeColors"
#define SHADERS_KEY				"radiant_UseShaders"
#define SWITCHCLIP_KEY			"radiant_SwitchClipKey"
#define SELWHOLEENTS_KEY		"radiant_SelectWholeEntitiesKey"
#define TEXTURESUBSET_KEY		"radiant_UseTextureSubsetLoading"
#define TEXTUREQUALITY_KEY		"radiant_TextureQuality"
#define SHOWSHADERS_KEY			"radiant_ShowShaders"
#define SHADERTEST_KEY			"radiant_ShaderTest"
#define GLLIGHTING_KEY			"radiant_UseGLLighting"
#define NOSTIPPLE_KEY			"radiant_NoStipple"
#define UNDOLEVELS_KEY			"radiant_UndoLevels"
#define MAPS_KEY				"radiant_RadiantMapPath"
#define MODELS_KEY				"radiant_ModelPath"
#define NEWMAPFORMAT_KEY		"radiant_NewMapFormat"

#define WINDOW_DEF				0
#define TLOCK_DEF				1
#define LOADLAST_DEF			1
#define RUN_DEF					0

/////////////////////////////////////////////////////////////////////////////
// CPrefsDlg dialog


CPrefsDlg::CPrefsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrefsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrefsDlg)
	m_bLoadLast = FALSE;
	m_bFace = FALSE;
	m_bRightClick = FALSE;
	m_bVertex = FALSE;
	m_bAutoSave = TRUE;
	m_bNewApplyHandling = FALSE;
	m_strAutoSave = _T("5");
	m_bLoadLastMap = FALSE;
	m_bTextureWindow = FALSE;
	m_bSnapShots = FALSE;
	m_fTinySize = 0.5;
	m_bCleanTiny = FALSE;
	m_nStatusSize = 10;
	m_bCamXYUpdate = FALSE;
	m_bNewLightDraw = FALSE;
	m_bALTEdge = FALSE;
	m_bQE4Painting = TRUE;
	m_bSnapTToGrid = FALSE;
	m_bXZVis = FALSE;
	m_bYZVis = FALSE;
	m_bZVis = FALSE;
	m_bSizePaint = FALSE;
	m_bWideToolbar = TRUE;
	m_bNoClamp = FALSE;
	m_nRotation = 0;
	m_bHiColorTextures = TRUE;
	m_bChaseMouse = FALSE;
	m_bTextureScrollbar = TRUE;
	m_bDisplayLists = TRUE;
	m_bNoStipple = FALSE;
	m_strMaps = _T("");
	m_strModels = _T("");
	m_bNewMapFormat = TRUE;
	//}}AFX_DATA_INIT
	//LoadPrefs();
	m_selectByBoundingBrush = FALSE;
	m_selectOnlyBrushes = FALSE;
	m_selectNoModels = FALSE;
	m_nEntityShowState = 0;
	m_nTextureScale = 2;
	m_bSwitchClip = FALSE;
	m_bSelectWholeEntities = TRUE;
	m_nTextureQuality = 3;
	m_bGLLighting = FALSE;
	m_nUndoLevels = 63;
}

void CPrefsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsDlg)
	DDX_Control(pDX, IDC_SPIN_UNDO, m_wndUndoSpin);
	DDX_Control(pDX, IDC_SPIN_POINTSIZE, m_wndFontSpin);
	DDX_Control(pDX, IDC_SLIDER_TEXTUREQUALITY, m_wndTexturequality);
	DDX_Control(pDX, IDC_SLIDER_CAMSPEED, m_wndCamSpeed);
	DDX_Control(pDX, IDC_SPIN_AUTOSAVE, m_wndSpin);
	DDX_Check(pDX, IDC_CHECK_LOADLAST, m_bLoadLast);
	DDX_Check(pDX, IDC_CHECK_FACE, m_bFace);
	DDX_Check(pDX, IDC_CHECK_RIGHTCLICK, m_bRightClick);
	DDX_Check(pDX, IDC_CHECK_AUTOSAVE, m_bAutoSave);
	DDX_Text(pDX, IDC_EDIT_AUTOSAVE, m_strAutoSave);
	DDX_Check(pDX, IDC_CHECK_LOADLASTMAP, m_bLoadLastMap);
	DDX_Check(pDX, IDC_CHECK_TEXTUREWINDOW, m_bTextureWindow);
	DDX_Check(pDX, IDC_CHECK_SNAPSHOTS, m_bSnapShots);
	DDX_Text(pDX, IDC_EDIT_STATUSPOINTSIZE, m_nStatusSize);
	DDV_MinMaxInt(pDX, m_nStatusSize, 2, 14);
	DDX_Check(pDX, IDC_CHECK_CAMXYUPDATE, m_bCamXYUpdate);
	DDX_Check(pDX, IDC_CHECK_LIGHTDRAW, m_bNewLightDraw);
	DDX_Check(pDX, IDC_CHECK_ALTDRAG, m_bALTEdge);
	DDX_Check(pDX, IDC_CHECK_QE4PAINTING, m_bQE4Painting);
	DDX_Check(pDX, IDC_CHECK_SNAPT, m_bSnapTToGrid);
	DDX_Check(pDX, IDC_CHECK_SIZEPAINT, m_bSizePaint);
	DDX_Check(pDX, IDC_CHECK_WIDETOOLBAR, m_bWideToolbar);
	DDX_Check(pDX, IDC_CHECK_NOCLAMP, m_bNoClamp);
	DDX_Text(pDX, IDC_EDIT_ROTATION, m_nRotation);
	DDX_Check(pDX, IDC_CHECK_HICOLOR, m_bHiColorTextures);
	DDX_Check(pDX, IDC_CHECK_MOUSECHASE, m_bChaseMouse);
	DDX_Check(pDX, IDC_CHECK_TEXTURESCROLLBAR, m_bTextureScrollbar);
	DDX_Check(pDX, IDC_CHECK_DISPLAYLISTS, m_bDisplayLists);
	DDX_Check(pDX, IDC_CHECK_NOSTIPPLE, m_bNoStipple);
	DDX_Text(pDX, IDC_EDIT_UNDOLEVELS, m_nUndoLevels);
	DDV_MinMaxInt(pDX, m_nUndoLevels, 1, 64);
	DDX_Text(pDX, IDC_EDIT_MAPS, m_strMaps);
	DDX_Check(pDX, IDC_CHECK_NEWMAPFORMAT, m_bNewMapFormat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsDlg, CDialog)
	//{{AFX_MSG_MAP(CPrefsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsDlg message handlers

BOOL CPrefsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_wndSpin.SetRange(1,60);
	m_wndCamSpeed.SetRange(10, 5000);
	m_wndCamSpeed.SetPos(m_nMoveSpeed);

	this->m_wndTexturequality.SetRange(0, 3);
	this->m_wndTexturequality.SetPos(m_nTextureQuality);

	m_wndFontSpin.SetRange(4,24);
	m_wndUndoSpin.SetRange(1,64);

	GetDlgItem(IDC_CHECK_HICOLOR)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_NOCLAMP)->EnableWindow(TRUE);

	//GetDlgItem(IDC_CHECK_NOCLAMP)->EnableWindow(FALSE);

	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefsDlg::OnOK() 
{
	m_nMoveSpeed = m_wndCamSpeed.GetPos();
	m_nAngleSpeed = (float)m_nMoveSpeed * 0.50;
	this->m_nTextureQuality = m_wndTexturequality.GetPos();
	SavePrefs();

	if ( g_pParentWnd ) {
		g_pParentWnd->SetGridStatus();
	}
	Sys_UpdateWindows(W_ALL);
	Undo_SetMaxSize(m_nUndoLevels);
	CDialog::OnOK();
}

int GetCvarInt(const char *name, const int def) {
	idCVar *cvar = cvarSystem->Find( name );
	if ( cvar ) {
		return cvar->GetInteger();
	} else {
		return def;
	}
}

const char *GetCvarString( const char *name, const char *def ) {
	idCVar *cvar = cvarSystem->Find( name );
	if ( cvar ) {
		return cvar->GetString();
	} else {
		return def;
	}
}

static const char hexDigits[] = "0123456789ABCDEF";

void SetCvarInt( const char *name, const int value ) {
	cvarSystem->SetCVarInteger( name, value, CVAR_TOOL );
}

void SetCvarString( const char *name, const char *value ) {
	cvarSystem->SetCVarString( name, value, CVAR_TOOL );
}

void SetCvarBinary(const char *name, void *pv, int size) {
	unsigned char *in = new unsigned char[size];
	idStr s;
	memset( in, 0, size );
	memcpy( in, pv, size );
	for ( int i = 0; i < size; i++ ) {
		s += hexDigits[in[i] >> 4];
		s += hexDigits[in[i] & 0x0f];
	}
	delete []in;
	SetCvarString(name, s);
}

bool GetCvarBinary( const char *name, void *pv, int size ) {
	bool ret = false;
	unsigned char *out = new unsigned char[size];
	idStr s = GetCvarString( name, "" );
	if ( s.Length() / 2 == size ) {
		int j = 0;
		for ( int i = 0; i < s.Length(); i += 2 ) {
			char c;
			if (s[i] > '9') {
				c = s[i] - 'A' + 0x0a;
			} else {
				c = s[i] - 0x30;
			}
			c <<= 4;
			if (s[i+1] > '9') {
				c |= s[i+1] - 'A' + 0x0a;
			} else {
				c |= s[i+1] - 0x30;
			}
			out[j++] = c;
		}
		memcpy(pv, out, size);
		ret = true;
	}
	delete []out;
	return ret;
}

void CPrefsDlg::LoadPrefs() {
	CString strBuff;
	CString strPrefab = g_strAppPath;
	AddSlash(strPrefab);
	strPrefab += "Prefabs\\";

	m_nMouseButtons = 3;

	m_bTextureLock = GetCvarInt( TLOCK_KEY, TLOCK_DEF );
	m_bRotateLock = GetCvarInt( RLOCK_KEY, TLOCK_DEF );
	m_strLastProject = GetCvarString( LASTPROJ_KEY, "" );
	m_strLastMap = GetCvarString( LASTMAP_KEY, "" );
	m_bLoadLast = GetCvarInt( LOADLAST_KEY, LOADLAST_DEF );
	m_bRunBefore = GetCvarInt( RUN_KEY, RUN_DEF );
	m_bFace = GetCvarInt( FACE_KEY, 1 );
	m_bRightClick = GetCvarInt( RCLICK_KEY, 1 );
	m_bVertex = GetCvarInt( VERTEX_KEY, 1 );
	m_bAutoSave = GetCvarInt( AUTOSAVE_KEY, 1 );
	m_bNewApplyHandling = GetCvarInt( NEWAPPLY_KEY, 0 );
	m_bLoadLastMap = GetCvarInt( LOADLASTMAP_KEY, 0 );
	m_bGatewayHack = GetCvarInt( HACK_KEY, 0 );
	m_bTextureWindow = GetCvarInt( TEXTURE_KEY, 0 );
	m_bCleanTiny = GetCvarInt( TINYBRUSH_KEY, 0 );
	strBuff = GetCvarString( TINYSIZE_KEY, "0.5" );
	m_fTinySize = atof(strBuff );
	m_nAutoSave = GetCvarInt( AUTOSAVETIME_KEY, 5 );
	if ( m_nAutoSave <= 0 ) { m_nAutoSave = 1; }
	m_strAutoSave.Format("%i", m_nAutoSave );
	m_bSnapShots = GetCvarInt( SNAPSHOT_KEY, 0 );
	m_nStatusSize = GetCvarInt( STATUS_KEY, 10 );
	m_nMoveSpeed = GetCvarInt( MOVESPEED_KEY, 400 );
	m_nAngleSpeed = GetCvarInt( ANGLESPEED_KEY, 300 );
	m_bCamXYUpdate = GetCvarInt( CAMXYUPDATE_KEY, 1 );
	m_bNewLightDraw = GetCvarInt( LIGHTDRAW_KEY, 1 );
	m_bCubicClipping = ( GetCvarInt( CUBICCLIP_KEY, 1) != 0  );
	m_nCubicScale = GetCvarInt( CUBICSCALE_KEY, 13 );
	m_bALTEdge = GetCvarInt( ALTEDGE_KEY, 0 );
	m_bQE4Painting = GetCvarInt( QE4PAINT_KEY, 1 );
	m_bSnapTToGrid = GetCvarInt( SNAPT_KEY, 0 );
	m_bXZVis = GetCvarInt( XZVIS_KEY, 0 );
	m_bYZVis = GetCvarInt( YZVIS_KEY, 0 );
	m_bZVis = GetCvarInt( ZVIS_KEY, 1 );
	m_bSizePaint = GetCvarInt( SIZEPAINT_KEY, 0 );
	m_bWideToolbar = GetCvarInt( WIDETOOLBAR_KEY, 1 );
	m_bNoClamp = GetCvarInt( NOCLAMP_KEY, 0 );
	m_nRotation = GetCvarInt( ROTATION_KEY, 45 );
	m_bHiColorTextures = GetCvarInt( HICOLOR_KEY, 1 );
	m_bChaseMouse = GetCvarInt( CHASEMOUSE_KEY, 1 );
	m_nEntityShowState = GetCvarInt( ENTITYSHOW_KEY, 0 );
	m_nTextureScale = GetCvarInt( TEXTURESCALE_KEY, 50 );
	m_bTextureScrollbar = GetCvarInt( TEXTURESCROLLBAR_KEY, TRUE );
	m_bDisplayLists = GetCvarInt( DISPLAYLISTS_KEY, TRUE );
	m_bSwitchClip = GetCvarInt( SWITCHCLIP_KEY, TRUE );
	m_bSelectWholeEntities = GetCvarInt( SELWHOLEENTS_KEY, TRUE );
	m_nTextureQuality = GetCvarInt( TEXTUREQUALITY_KEY, 6 );
	m_bGLLighting = GetCvarInt( GLLIGHTING_KEY, FALSE );
	m_bNoStipple = GetCvarInt( NOSTIPPLE_KEY, 0 );
	m_nUndoLevels = GetCvarInt( UNDOLEVELS_KEY, 63 );
	m_strMaps = GetCvarString( MAPS_KEY, "" );
	m_strModels = GetCvarString( MODELS_KEY, "" );
	m_bNoStipple = GetCvarInt( NEWMAPFORMAT_KEY, 1 );

	if ( m_bRunBefore == FALSE ) {
		SetGamePrefs();
	}
}

void CPrefsDlg::SavePrefs() {
	if ( GetSafeHwnd() ) {
		UpdateData(TRUE);
	}

	m_nMouseButtons = 3;

	SetCvarInt( TLOCK_KEY, m_bTextureLock );
	SetCvarInt( RLOCK_KEY, m_bRotateLock );
	SetCvarInt( LOADLAST_KEY, m_bLoadLast );
	SetCvarString( LASTPROJ_KEY, m_strLastProject );
	SetCvarString( LASTMAP_KEY, m_strLastMap );
	SetCvarInt( RUN_KEY, m_bRunBefore );
	SetCvarInt( FACE_KEY, m_bFace );
	SetCvarInt( RCLICK_KEY, m_bRightClick );
	SetCvarInt( VERTEX_KEY, m_bVertex );
	SetCvarInt( AUTOSAVE_KEY, m_bAutoSave );
	SetCvarInt( LOADLASTMAP_KEY, m_bLoadLastMap );
	SetCvarInt( TEXTURE_KEY, m_bTextureWindow );
	m_nAutoSave = atoi( m_strAutoSave );
	SetCvarInt( AUTOSAVETIME_KEY, m_nAutoSave );
	SetCvarInt( SNAPSHOT_KEY, m_bSnapShots );
	SetCvarInt( STATUS_KEY, m_nStatusSize );
	SetCvarInt( CAMXYUPDATE_KEY, m_bCamXYUpdate );
	SetCvarInt( LIGHTDRAW_KEY, m_bNewLightDraw );
	SetCvarInt( MOVESPEED_KEY, m_nMoveSpeed );
	SetCvarInt( ANGLESPEED_KEY, m_nAngleSpeed );
	SetCvarInt( CUBICCLIP_KEY, m_bCubicClipping );
	SetCvarInt( CUBICSCALE_KEY, m_nCubicScale );
	SetCvarInt( ALTEDGE_KEY, m_bALTEdge );
	SetCvarInt( QE4PAINT_KEY, m_bQE4Painting );
	SetCvarInt( SNAPT_KEY, m_bSnapTToGrid );
	SetCvarInt( XZVIS_KEY, m_bXZVis );
	SetCvarInt( YZVIS_KEY, m_bYZVis );
	SetCvarInt( ZVIS_KEY, m_bZVis );
	SetCvarInt( SIZEPAINT_KEY, m_bSizePaint );
	SetCvarInt( WIDETOOLBAR_KEY, m_bWideToolbar );
	SetCvarInt( NOCLAMP_KEY, m_bNoClamp );
	SetCvarInt( ROTATION_KEY, m_nRotation );
	SetCvarInt( HICOLOR_KEY, m_bHiColorTextures );
	SetCvarInt( CHASEMOUSE_KEY, m_bChaseMouse );
	SetCvarInt( ENTITYSHOW_KEY, m_nEntityShowState );
	SetCvarInt( TEXTURESCALE_KEY, m_nTextureScale );
	SetCvarInt( TEXTURESCROLLBAR_KEY, m_bTextureScrollbar );
	SetCvarInt( DISPLAYLISTS_KEY, m_bDisplayLists );
	SetCvarInt( SWITCHCLIP_KEY, m_bSwitchClip );
	SetCvarInt( SELWHOLEENTS_KEY, m_bSelectWholeEntities );
	SetCvarInt( TEXTUREQUALITY_KEY, m_nTextureQuality );
	SetCvarInt( GLLIGHTING_KEY, m_bGLLighting );
	SetCvarInt( NOSTIPPLE_KEY, m_bNoStipple );
	SetCvarInt( UNDOLEVELS_KEY, m_nUndoLevels );
	SetCvarString( MAPS_KEY, m_strMaps );
	SetCvarString( MODELS_KEY, m_strModels );
	SetCvarInt( NEWMAPFORMAT_KEY, m_bNewMapFormat );
	common->WriteFlaggedCVarsToFile( "editor.cfg", CVAR_TOOL, "sett" );
}

void CPrefsDlg::SetGamePrefs() {
	m_bHiColorTextures = TRUE;
	m_bWideToolbar = TRUE;
	SavePrefs();
}

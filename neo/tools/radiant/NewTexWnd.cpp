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
#include "Radiant.h"
#include "NewTexWnd.h"
#include "io.h"

#include "../../renderer/tr_local.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool Sys_KeyDown( int key ) {
	return ( ( ::GetAsyncKeyState( key ) & 0x8000 ) != 0 );
}

// CNewTexWnd
IMPLEMENT_DYNCREATE(CNewTexWnd, CWnd);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
CNewTexWnd::CNewTexWnd() {
	m_bNeedRange = true;
	hglrcTexture = NULL;
	hdcTexture = NULL;
	cursor.x = cursor.y = 0;
	origin.x = origin.y = 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
CNewTexWnd::~CNewTexWnd() {
}

BEGIN_MESSAGE_MAP(CNewTexWnd, CWnd)
//{{AFX_MSG_MAP(CNewTexWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PARENTNOTIFY()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()
//
// =======================================================================================================================
//    CNewTexWnd message handlers
// =======================================================================================================================
//
BOOL CNewTexWnd::PreCreateWindow(CREATESTRUCT &cs) {
	WNDCLASS	wc;
	HINSTANCE	hInstance = AfxGetInstanceHandle();
	if (::GetClassInfo(hInstance, TEXTURE_WINDOW_CLASS, &wc) == FALSE) {
		// Register a new class
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_NOCLOSE | CS_PARENTDC;	// | CS_OWNDC;
		wc.lpszClassName = TEXTURE_WINDOW_CLASS;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpfnWndProc = ::DefWindowProc;
		if (AfxRegisterClass(&wc) == FALSE) {
			Error("CNewTexWnd RegisterClass: failed");
		}
	}

	cs.lpszClass = TEXTURE_WINDOW_CLASS;
	cs.lpszName = "TEX";
	if (cs.style != QE3_CHILDSTYLE && cs.style != QE3_STYLE) {
		cs.style = QE3_SPLITTER_STYLE;
	}

	return CWnd::PreCreateWindow(cs);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int CNewTexWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	ShowScrollBar(SB_VERT, g_PrefsDlg.m_bTextureScrollbar);
	m_bNeedRange = true;

	hdcTexture = GetDC();
	QEW_SetupPixelFormat(hdcTexture->m_hDC, false);

	EnableToolTips(TRUE);
	EnableTrackingToolTips(TRUE);

	return 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnSize(UINT nType, int cx, int cy) {
	CWnd::OnSize(nType, cx, cy);
	GetClientRect(rectClient);
	m_bNeedRange = true;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnParentNotify(UINT message, LPARAM lParam) {
	CWnd::OnParentNotify(message, lParam);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::UpdatePrefs() {
	ShowScrollBar(SB_VERT, g_PrefsDlg.m_bTextureScrollbar);
	m_bNeedRange = true;
	Invalidate();
	UpdateWindow();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
	g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags, false);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
const idMaterial *CNewTexWnd::NextPos() {
	const idMaterial *mat = NULL;
	while (1) {
		if (currentIndex >= declManager->GetNumDecls( DECL_MATERIAL )) {
			return NULL;
		}

		mat = declManager->MaterialByIndex(currentIndex, false);

		currentIndex++;

		//if (mat->getName()[0] == '(') { // fake color texture
		//	continue;
		//}

		if ( !mat->IsValid() ) {
			continue;
		}

		if (!mat->TestMaterialFlag(MF_EDITOR_VISIBLE)) {
			continue;
		}
		break;
	}

	// ensure it is uploaded
	declManager->FindMaterial(mat->GetName());

	int width = mat->GetEditorImage()->uploadWidth * ((float)g_PrefsDlg.m_nTextureScale / 100);
	int height = mat->GetEditorImage()->uploadHeight * ((float)g_PrefsDlg.m_nTextureScale / 100);

	if (current.x + width > rectClient.Width() - 8 && currentRow) {
		// go to the next row unless the texture is the first on the row
		current.x = 8;
		current.y -= currentRow + FONT_HEIGHT + 4;
		currentRow = 0;
	}

	draw = current;

	// Is our texture larger than the row? If so, grow the row height to match it
	if (currentRow < height) {
		currentRow = height;
	}

	// never go less than 64, or the names get all crunched up
	current.x += width < 64 ? 64 : width;
	current.x += 8;
	return mat;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnPaint() {

	CPaintDC	dc(this);	// device context for painting

	int nOld = g_qeglobals.d_texturewin.m_nTotalHeight;

	//hdcTexture = GetDC();
	if (!qwglMakeCurrent(dc.GetSafeHdc(), win32.hGLRC)) {
		common->Printf("ERROR: wglMakeCurrent failed..\n ");
	}
	else {
		const char	*name;
		qglClearColor
		(
			g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][0],
			g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][1],
			g_qeglobals.d_savedinfo.colors[COLOR_TEXTUREBACK][2],
			0
		);
		qglViewport(0, 0, rectClient.Width(), rectClient.Height());
		qglScissor(0, 0, rectClient.Width(), rectClient.Height());
		qglMatrixMode(GL_PROJECTION);
		qglLoadIdentity();
		qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		qglDisable(GL_DEPTH_TEST);
		qglDisable(GL_BLEND);
		qglOrtho(0, rectClient.Width(), origin.y - rectClient.Height(), origin.y, -100, 100);
		qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// init stuff
		current.x = 8;
		current.y = -8;
		currentRow = 0;
		currentIndex = 0;
		while (1) {
			const idMaterial *mat = NextPos();
			if (mat == NULL) {
				break;
			}

			int width = mat->GetEditorImage()->uploadWidth * ((float)g_PrefsDlg.m_nTextureScale / 100);
			int height = mat->GetEditorImage()->uploadHeight * ((float)g_PrefsDlg.m_nTextureScale / 100);

			// Is this texture visible?
			if ((draw.y - height - FONT_HEIGHT < origin.y) && (draw.y > origin.y - rectClient.Height())) {
				// if in use, draw a background
				qglLineWidth(1);
				qglColor3f(1, 1, 1);
				globalImages->BindNull();
				qglBegin(GL_LINE_LOOP);
				qglVertex2f(draw.x - 1, draw.y + 1 - FONT_HEIGHT);
				qglVertex2f(draw.x - 1, draw.y - height - 1 - FONT_HEIGHT);
				qglVertex2f(draw.x + 1 + width, draw.y - height - 1 - FONT_HEIGHT);
				qglVertex2f(draw.x + 1 + width, draw.y + 1 - FONT_HEIGHT);
				qglEnd();

				// Draw the texture
				float	fScale = (g_PrefsDlg.m_bHiColorTextures == TRUE) ? ((float)g_PrefsDlg.m_nTextureScale / 100) : 1.0;

				mat->GetEditorImage()->Bind();
				QE_CheckOpenGLForErrors();
				qglColor3f(1, 1, 1);
				qglBegin(GL_QUADS);
				qglTexCoord2f(0, 0);
				qglVertex2f(draw.x, draw.y - FONT_HEIGHT);
				qglTexCoord2f(1, 0);
				qglVertex2f(draw.x + width, draw.y - FONT_HEIGHT);
				qglTexCoord2f(1, 1);
				qglVertex2f(draw.x + width, draw.y - FONT_HEIGHT - height);
				qglTexCoord2f(0, 1);
				qglVertex2f(draw.x, draw.y - FONT_HEIGHT - height);
				qglEnd();

				// draw the selection border
				if ( !idStr::Icmp(g_qeglobals.d_texturewin.texdef.name, mat->GetName()) ) {
					qglLineWidth(3);
					qglColor3f(1, 0, 0);
					globalImages->BindNull();

					qglBegin(GL_LINE_LOOP);
					qglVertex2f(draw.x - 4, draw.y - FONT_HEIGHT + 4);
					qglVertex2f(draw.x - 4, draw.y - FONT_HEIGHT - height - 4);
					qglVertex2f(draw.x + 4 + width, draw.y - FONT_HEIGHT - height - 4);
					qglVertex2f(draw.x + 4 + width, draw.y - FONT_HEIGHT + 4);
					qglEnd();

					qglLineWidth(1);
				}

				// draw the texture name
				globalImages->BindNull();
				qglColor3f(1, 1, 1);
				qglRasterPos2f(draw.x, draw.y - FONT_HEIGHT + 2);

				// don't draw the directory name
				for (name = mat->GetName(); *name && *name != '/' && *name != '\\'; name++) {
					;
				}

				if (!*name) {
					name = mat->GetName();
				}
				else {
					name++;
				}
				qglCallLists(strlen(name), GL_UNSIGNED_BYTE, name);
				//qglCallLists(va("%s -- %d, %d" strlen(name), GL_UNSIGNED_BYTE, name);
			} 
		}

		g_qeglobals.d_texturewin.m_nTotalHeight = abs(draw.y) + 100;

		// reset the current texture
		globalImages->BindNull();
		qglFinish();
		qwglSwapBuffers(dc.GetSafeHdc());
		TRACE("Texture Paint\n");
	}

	if (g_PrefsDlg.m_bTextureScrollbar && (m_bNeedRange || g_qeglobals.d_texturewin.m_nTotalHeight != nOld)) {
		m_bNeedRange = false;
		SetScrollRange(SB_VERT, 0, g_qeglobals.d_texturewin.m_nTotalHeight, TRUE);
	}

	//ReleaseDC(hdcTexture);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar) {
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);

	int n = GetScrollPos(SB_VERT);
	switch (nSBCode)
	{
		case SB_LINEUP: {
				n = (n - 15 > 0) ? n - 15 : 0;
				break;
			}

		case SB_LINEDOWN: {
				n = (n + 15 < g_qeglobals.d_texturewin.m_nTotalHeight) ? n + 15 : n;
				break;
			}

		case SB_PAGEUP: {
				n = (n - g_qeglobals.d_texturewin.height > 0) ? n - g_qeglobals.d_texturewin.height : 0;
				break;
			}

		case SB_PAGEDOWN: {
				n = (n + g_qeglobals.d_texturewin.height < g_qeglobals.d_texturewin.m_nTotalHeight) ? n + g_qeglobals.d_texturewin.height : n;
				break;
			}

		case SB_THUMBPOSITION: {
				n = nPos;
				break;
			}

		case SB_THUMBTRACK: {
				n = nPos;
				break;
			}
	}

	SetScrollPos(SB_VERT, n);
	origin.y = -n;
	Invalidate();
	UpdateWindow();

	// Sys_UpdateWindows(W_TEXTURE);
}

BOOL CNewTexWnd::DestroyWindow() {
	ReleaseDC(hdcTexture);
	return CWnd::DestroyWindow();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */						    
BOOL CNewTexWnd::Create
(
	LPCTSTR			lpszClassName,
	LPCTSTR			lpszWindowName,
	DWORD			dwStyle,
	const RECT		&rect,
	CWnd			*pParentWnd,
	UINT			nID,
	CCreateContext	*pContext
) {
	BOOL	ret = CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
	if (ret) {
		hdcTexture = GetDC();
		QEW_SetupPixelFormat(hdcTexture->m_hDC, false);
	}

	return ret;
}

const idMaterial *CNewTexWnd::getMaterialAtPoint(CPoint point) {

	// init stuff
	int my = rectClient.Height() - 1 - point.y;
	my += origin.y - rectClient.Height();

	current.x = 8;
	current.y = -8;
	currentRow = 0;
	currentIndex = 0;

	while (1) {
		const idMaterial *mat = NextPos();
		if (mat == NULL) {
			return NULL;
		}

		int width = mat->GetEditorImage()->uploadWidth * ((float)g_PrefsDlg.m_nTextureScale / 100);
		int height = mat->GetEditorImage()->uploadHeight * ((float)g_PrefsDlg.m_nTextureScale / 100);
		//if (point.x > draw.x && point.x - draw.x < width && my < draw.y && my + draw.y < height + FONT_HEIGHT) {
		if (point.x > draw.x && point.x - draw.x < width && my < draw.y &&  draw.y - my < height + FONT_HEIGHT) {
			return mat;
		}
	
	}

}
/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnLButtonDown(UINT nFlags, CPoint point) {
	cursor = point;

	SetFocus();
	bool fitScale = Sys_KeyDown(VK_CONTROL);
	bool edit = Sys_KeyDown(VK_SHIFT) && !fitScale;

	const idMaterial *mat = getMaterialAtPoint(point);
	if (mat) {
		Select_SetDefaultTexture(mat, fitScale, true);
	} else {
		Sys_Status("Did not select a texture\n", 0);
	}

	//
	UpdateSurfaceDialog();
	UpdatePatchInspector();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnMButtonDown(UINT nFlags, CPoint point) {
	CWnd::OnMButtonDown(nFlags, point);

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnRButtonDown(UINT nFlags, CPoint point) {
	cursor = point;
	SetFocus();
}

/*
 ===============================t========================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnLButtonUp(UINT nFlags, CPoint point) {
	CWnd::OnLButtonUp(nFlags, point);
	g_pParentWnd->SetFocus();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnMButtonUp(UINT nFlags, CPoint point) {
	CWnd::OnMButtonUp(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnRButtonUp(UINT nFlags, CPoint point) {
	CWnd::OnRButtonUp(nFlags, point);
}

extern float	fDiff(float f1, float f2);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::OnMouseMove(UINT nFlags, CPoint point) {
	int scale = 1;

	if (Sys_KeyDown(VK_SHIFT)) {
		scale = 4;
	}

	// rbutton = drag texture origin
	if (Sys_KeyDown(VK_RBUTTON)) {
		if (point.y != cursor.y) {
			if (Sys_KeyDown(VK_MENU)) {
				long	*px = &point.x;
				long	*px2 = &cursor.x;

				if (fDiff(point.y, cursor.y) > fDiff(point.x, cursor.x)) {
					px = &point.y;
					px2 = &cursor.y;
				}

				if (*px > *px2) {
					// zoom in
					g_PrefsDlg.m_nTextureScale += 4;
					if (g_PrefsDlg.m_nTextureScale > 500) {
						g_PrefsDlg.m_nTextureScale = 500;
					}
				}
				else if (*px < *px2) {
					// zoom out
					g_PrefsDlg.m_nTextureScale -= 4;
					if (g_PrefsDlg.m_nTextureScale < 1) {
						g_PrefsDlg.m_nTextureScale = 1;
					}
				}

				*px2 = *px;
				CPoint screen = cursor;
				ClientToScreen(&screen);
				SetCursorPos(screen.x, screen.y);
				//Sys_SetCursorPos(cursor.x, cursor.y);
				InvalidateRect(NULL, false);
				UpdateWindow();
			}
			else if (point.y != cursor.y || point.x != cursor.x) {
				origin.y += (point.y - cursor.y) * scale;
				if (origin.y > 0) {
					origin.y = 0;
				}

				//Sys_SetCursorPos(cursor.x, cursor.y);
				CPoint screen = cursor;
				ClientToScreen(&screen);
				SetCursorPos(screen.x, screen.y);
				if (g_PrefsDlg.m_bTextureScrollbar) {
					SetScrollPos(SB_VERT, abs(origin.y));
				}

				InvalidateRect(NULL, false);
				UpdateWindow();
			}
		}

		return;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CNewTexWnd::LoadMaterials() {
}


void Texture_SetTexture(texdef_t *texdef, brushprimit_texdef_t	*brushprimit_texdef, bool bFitScale, bool bSetSelection) {
	
	if (texdef->name[0] == '(') {
		Sys_Status("Can't select an entity texture\n", 0);
		return;
	}

	g_qeglobals.d_texturewin.texdef = *texdef;

	//
	// store the texture coordinates for new brush primitive mode be sure that all the
	// callers are using the default 2x2 texture
	//
	if (g_qeglobals.m_bBrushPrimitMode) {
		g_qeglobals.d_texturewin.brushprimit_texdef = *brushprimit_texdef;
	}

	g_dlgFind.updateTextures(texdef->name);
	
	if (!g_dlgFind.isOpen() && bSetSelection) {
		Select_SetTexture(texdef, brushprimit_texdef, bFitScale);
	}

	g_Inspectors->texWnd.EnsureTextureIsVisible(texdef->name);

	if ( g_Inspectors->mediaDlg.IsWindowVisible() ) {
		g_Inspectors->mediaDlg.SelectCurrentItem(true, g_qeglobals.d_texturewin.texdef.name, CDialogTextures::MATERIALS);
	}

	g_qeglobals.d_texturewin.texdef = *texdef;
	// store the texture coordinates for new brush primitive mode be sure that all the
	// callers are using the default 2x2 texture
	//
	if (g_qeglobals.m_bBrushPrimitMode) {
		g_qeglobals.d_texturewin.brushprimit_texdef = *brushprimit_texdef;
	}


	Sys_UpdateWindows(W_TEXTURE);


}

const idMaterial *Texture_LoadLight(const char *name) {
	return declManager->FindMaterial(name);
}


void Texture_ClearInuse(void) {
}

void Texture_ShowAll(void) {
	int count = declManager->GetNumDecls( DECL_MATERIAL );
	for (int i = 0; i < count; i++) {
		const idMaterial *mat = declManager->MaterialByIndex(i, false);
		if ( mat ) {
			mat->SetMaterialFlag(MF_EDITOR_VISIBLE);
		}
	}
	g_Inspectors->SetWindowText("Textures (all)");
	Sys_UpdateWindows(W_TEXTURE);
}

void Texture_HideAll() {
	int count = declManager->GetNumDecls( DECL_MATERIAL );
	for (int i = 0; i < count; i++) {
		const idMaterial *mat = declManager->MaterialByIndex(i, false);
		if ( mat ) {
			mat->ClearMaterialFlag(MF_EDITOR_VISIBLE);
		}
	}
	g_Inspectors->SetWindowText("Textures (all)");
	Sys_UpdateWindows(W_TEXTURE);
}

const idMaterial *Texture_ForName(const char *name) {
	const idMaterial *mat = declManager->FindMaterial(name);
	if ( !mat ) {
		mat = declManager->FindMaterial("_default");
	} else {
		mat->SetMaterialFlag(MF_EDITOR_VISIBLE);
	}
	return mat;
}

void Texture_ShowInuse(void) {
	Texture_HideAll();

	brush_t *b;
	for (b = active_brushes.next; b != NULL && b != &active_brushes; b = b->next) {
		if (b->pPatch) {
			Texture_ForName(b->pPatch->d_texture->GetName());
		} else {
			for (face_t *f = b->brush_faces; f; f = f->next) {
				Texture_ForName(f->texdef.name);
			}
		}
	}

	for (b = selected_brushes.next; b != NULL && b != &selected_brushes; b = b->next) {
		if (b->pPatch) {
			Texture_ForName(b->pPatch->d_texture->GetName());
		} else {
			for (face_t *f = b->brush_faces; f; f = f->next) {
				Texture_ForName(f->texdef.name);
			}
		}
	}

	Sys_UpdateWindows(W_TEXTURE);

	g_Inspectors->SetWindowText("Textures (in use)");
}

void Texture_Cleanup(CStringList *pList) {
}

int				texture_mode = GL_LINEAR_MIPMAP_LINEAR;
bool texture_showinuse = true;


/*
 =======================================================================================================================
    Texture_SetMode
 =======================================================================================================================
 */
void Texture_SetMode(int iMenu) {
	int		iMode;
	HMENU	hMenu;
	bool	texturing = true;

	hMenu = GetMenu(g_pParentWnd->GetSafeHwnd());

	switch (iMenu)
	{
		case ID_VIEW_NEAREST:
			iMode = GL_NEAREST;
			break;
		case ID_VIEW_NEARESTMIPMAP:
			iMode = GL_NEAREST_MIPMAP_NEAREST;
			break;
		case ID_VIEW_LINEAR:
			iMode = GL_NEAREST_MIPMAP_LINEAR;
			break;
		case ID_VIEW_BILINEAR:
			iMode = GL_LINEAR;
			break;
		case ID_VIEW_BILINEARMIPMAP:
			iMode = GL_LINEAR_MIPMAP_NEAREST;
			break;
		case ID_VIEW_TRILINEAR:
			iMode = GL_LINEAR_MIPMAP_LINEAR;
			break;

		case ID_TEXTURES_WIREFRAME:
			iMode = 0;
			texturing = false;
			break;

		case ID_TEXTURES_FLATSHADE:
		default:
			iMode = 0;
			texturing = false;
			break;
	}

	CheckMenuItem(hMenu, ID_VIEW_NEAREST, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_NEARESTMIPMAP, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_BILINEARMIPMAP, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_BILINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VIEW_TRILINEAR, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_TEXTURES_WIREFRAME, MF_BYCOMMAND | MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_TEXTURES_FLATSHADE, MF_BYCOMMAND | MF_UNCHECKED);

	CheckMenuItem(hMenu, iMenu, MF_BYCOMMAND | MF_CHECKED);

	g_qeglobals.d_savedinfo.iTexMenu = iMenu;
	texture_mode = iMode;

	if (!texturing && iMenu == ID_TEXTURES_WIREFRAME) {
		g_pParentWnd->GetCamera()->Camera().draw_mode = cd_wire;
		Map_BuildBrushData();
		Sys_UpdateWindows(W_ALL);
		return;
	}
	else if (!texturing && iMenu == ID_TEXTURES_FLATSHADE) {
		g_pParentWnd->GetCamera()->Camera().draw_mode = cd_solid;
		Map_BuildBrushData();
		Sys_UpdateWindows(W_ALL);
		return;
	}

	if (g_pParentWnd->GetCamera()->Camera().draw_mode != cd_texture) {
		g_pParentWnd->GetCamera()->Camera().draw_mode = cd_texture;
		Map_BuildBrushData();
	}

	Sys_UpdateWindows(W_ALL);
}



void CNewTexWnd::EnsureTextureIsVisible(const char *name) {
	// scroll origin so the texture is completely on screen
	// init stuff
	current.x = 8;
	current.y = -8;
	currentRow = 0;
	currentIndex = 0;

	while (1) {
		const idMaterial *mat = NextPos();
		if (mat == NULL) {
			break;
		}

		int width = mat->GetEditorImage()->uploadWidth * ((float)g_PrefsDlg.m_nTextureScale / 100);
		int height = mat->GetEditorImage()->uploadHeight * ((float)g_PrefsDlg.m_nTextureScale / 100);

		if ( !idStr::Icmp(name, mat->GetName()) ) {
			if (current.y > origin.y) {
				origin.y = current.y;
				Sys_UpdateWindows(W_TEXTURE);
				return;
			}

			if (current.y - height - 2 * FONT_HEIGHT < origin.y - rectClient.Height()) {
				origin.y = current.y - height - 2 * FONT_HEIGHT + rectClient.Height();
				Sys_UpdateWindows(W_TEXTURE);
				return;
			}

			return;
		}
	}

}


BOOL CNewTexWnd::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult ) {
	static char tip[1024];
	CPoint point;
	GetCursorPos(&point);
	const idMaterial *mat = getMaterialAtPoint(point);

	if (mat) {
	    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
		strcpy(tip, mat->GetDescription());
	    pTTT->lpszText = tip;
	    pTTT->hinst = NULL;
	    return(TRUE);
    }
    return(FALSE);
}

int CNewTexWnd::OnToolHitTest(CPoint point, TOOLINFO * pTI)
{
	const idMaterial *mat = getMaterialAtPoint(point);
	if (mat) {
		return 0;
	}
	return -1;
}

BOOL CNewTexWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	OnVScroll((zDelta >= 0) ? SB_LINEUP : SB_LINEDOWN, 0, NULL);
	OnVScroll((zDelta >= 0) ? SB_LINEUP : SB_LINEDOWN, 0, NULL);
	OnVScroll((zDelta >= 0) ? SB_LINEUP : SB_LINEDOWN, 0, NULL);
	OnVScroll((zDelta >= 0) ? SB_LINEUP : SB_LINEDOWN, 0, NULL);
	OnVScroll((zDelta >= 0) ? SB_LINEUP : SB_LINEDOWN, 0, NULL);
	OnVScroll((zDelta >= 0) ? SB_LINEUP : SB_LINEDOWN, 0, NULL);
	return TRUE;
}

BOOL CNewTexWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_ESCAPE) {
			g_pParentWnd->GetCamera()->SetFocus();
			Select_Deselect();
			return TRUE;
		}
		if (pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_LEFT || pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN) {
			g_pParentWnd->PostMessage(WM_KEYDOWN, pMsg->wParam);
			return TRUE;
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

void CNewTexWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
	Invalidate();
	RedrawWindow();
}

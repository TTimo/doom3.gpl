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
#include "XYWnd.h"
#include "CamWnd.h"
#include "splines.h"
#include <GL/glu.h>

#include "../../renderer/tr_local.h"
#include "../../renderer/model_local.h"	// for idRenderModelMD5

#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern void DrawPathLines();

int g_axialAnchor = -1;
int g_axialDest = -1;
bool g_bAxialMode = false;

void ValidateAxialPoints() {
	int faceCount = g_ptrSelectedFaces.GetSize();
	if (faceCount > 0) {
		face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(0));
		if (g_axialAnchor >= selFace->face_winding->GetNumPoints()) {
			g_axialAnchor = 0;
		}
		if (g_axialDest >= selFace->face_winding->GetNumPoints()) {
			g_axialDest = 0;
		}
	} else {
		g_axialDest = 0;
		g_axialAnchor = 0;
	}
}

// CCamWnd
IMPLEMENT_DYNCREATE(CCamWnd, CWnd);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
CCamWnd::CCamWnd() {
	m_pXYFriend = NULL;
	memset(&m_Camera, 0, sizeof(camera_t));
	m_pSide_select = NULL;
	m_bClipMode = false;
	worldDirty = true;
	worldModel = NULL;
	renderMode = false;
	rebuildMode = false;
	entityMode = false;
	animationMode = false;
	selectMode = false;
	soundMode = false;
	saveValid = false;
	Cam_Init();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
CCamWnd::~CCamWnd() {
}

BEGIN_MESSAGE_MAP(CCamWnd, CWnd)
//{{AFX_MSG_MAP(CCamWnd)
	ON_WM_KEYDOWN()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_KEYUP()
	ON_WM_NCCALCSIZE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/*
 =======================================================================================================================
 =======================================================================================================================
 */
LONG WINAPI CamWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	RECT	rect;

	GetClientRect(hWnd, &rect);

	switch (uMsg)
	{
		case WM_KILLFOCUS:
		case WM_SETFOCUS:
			SendMessage(hWnd, WM_NCACTIVATE, uMsg == WM_SETFOCUS, 0);
			return 0;

		case WM_NCCALCSIZE: // don't let windows copy pixels
			DefWindowProc(hWnd, uMsg, wParam, lParam);
			return WVR_REDRAW;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//
// =======================================================================================================================
//    CCamWnd message handlers
// =======================================================================================================================
//
BOOL CCamWnd::PreCreateWindow(CREATESTRUCT &cs) {
	WNDCLASS	wc;
	HINSTANCE	hInstance = AfxGetInstanceHandle();
	if (::GetClassInfo(hInstance, CAMERA_WINDOW_CLASS, &wc) == FALSE) {
		// Register a new class
		memset(&wc, 0, sizeof(wc));

		// wc.style = CS_NOCLOSE | CS_OWNDC;
		wc.style = CS_NOCLOSE;
		wc.lpszClassName = CAMERA_WINDOW_CLASS;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpfnWndProc = CamWndProc;
		if (AfxRegisterClass(&wc) == FALSE) {
			Error("CCamWnd RegisterClass: failed");
		}
	}

	cs.lpszClass = CAMERA_WINDOW_CLASS;
	cs.lpszName = "CAM";
	if (cs.style != QE3_CHILDSTYLE) {
		cs.style = QE3_SPLITTER_STYLE;
	}

	BOOL	bResult = CWnd::PreCreateWindow(cs);

	//
	// See if the class already exists and if not then we need to register our new
	// window class.
	//
	return bResult;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags);
}

brush_t *g_pSplitList = NULL;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnPaint() {
	CPaintDC	dc(this);	// device context for painting
	bool		bPaint = true;

	if (!qwglMakeCurrent(dc.m_hDC, win32.hGLRC)) {
		common->Printf("ERROR: wglMakeCurrent failed..\n ");
		common->Printf("Please restart " EDITOR_WINDOWTEXT " if the camera view is not working\n");
	}
	else {
		QE_CheckOpenGLForErrors();
		g_pSplitList = NULL;
		if (g_bClipMode) {
			if (g_Clip1.Set() && g_Clip2.Set()) {
				g_pSplitList = ((g_pParentWnd->ActiveXY()->GetViewType() == XZ) ? !g_bSwitch : g_bSwitch) ? &g_brBackSplits : &g_brFrontSplits;
			}
		}

		Cam_Draw();
		QE_CheckOpenGLForErrors();
		qwglSwapBuffers(dc.m_hDC);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::SetXYFriend(CXYWnd *pWnd) {
	m_pXYFriend = pWnd;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnDestroy() {
	CWnd::OnDestroy();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnClose() {
	CWnd::OnClose();
}

extern void Select_RotateTexture(float amt, bool absolute);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnMouseMove(UINT nFlags, CPoint point) {
	CRect	r;
	GetClientRect(r);
	if	(GetCapture() == this && (GetAsyncKeyState(VK_MENU) & 0x8000) && !((GetAsyncKeyState(VK_SHIFT) & 0x8000) || (GetAsyncKeyState(VK_CONTROL) & 0x8000))) {
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			Select_RotateTexture((float)point.y - m_ptLastCursor.y);
		}
		else if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
			Select_ScaleTexture((float)point.x - m_ptLastCursor.x, (float)m_ptLastCursor.y - point.y);
		}
		else {
			Select_ShiftTexture((float)point.x - m_ptLastCursor.x, (float)m_ptLastCursor.y - point.y);
		}
	}
	else {
		Cam_MouseMoved(point.x, r.bottom - 1 - point.y, nFlags);
	}

	m_ptLastCursor = point;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnLButtonDown(UINT nFlags, CPoint point) {
	m_ptLastCursor = point;
	OriginalMouseDown(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnLButtonUp(UINT nFlags, CPoint point) {
	OriginalMouseUp(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnMButtonDown(UINT nFlags, CPoint point) {
	OriginalMouseDown(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnMButtonUp(UINT nFlags, CPoint point) {
	OriginalMouseUp(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnRButtonDown(UINT nFlags, CPoint point) {
	OriginalMouseDown(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnRButtonUp(UINT nFlags, CPoint point) {
	OriginalMouseUp(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int CCamWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	CDC *pDC = GetDC();
	HDC hDC = pDC->GetSafeHdc();

	QEW_SetupPixelFormat(hDC, true);

	HFONT hfont = CreateFont(
				12,	// logical height of font 
				0,	// logical average character width 
				0,	// angle of escapement 
				0,	// base-line orientation angle 
				0,	// font weight 
				0,	// italic attribute flag 
				0,	// underline attribute flag 
				0,	// strikeout attribute flag 
				0,	// character set identifier 
				0,	// output precision 
				0,	// clipping precision 
				0,	// output quality 
				FIXED_PITCH | FF_MODERN,	// pitch and family 
				"Lucida Console" 	// pointer to typeface name string 
				);

	if (!hfont) {
		Error("couldn't create font");
	}

	HFONT hOldFont = (HFONT)SelectObject(hDC, hfont);

	wglMakeCurrent (hDC, win32.hGLRC);

	if ((g_qeglobals.d_font_list = qglGenLists(256)) == 0) {
		common->Warning( "couldn't create font dlists" );
	}

	// create the bitmap display lists we're making images of glyphs 0 thru 255
	if ( !qwglUseFontBitmaps(hDC, 0, 255, g_qeglobals.d_font_list) ) {
		common->Warning( "wglUseFontBitmaps failed (%d).  Trying again.", GetLastError() );

		// FIXME: This is really wacky, sometimes the first call fails, but calling it again makes it work
		//		This probably indicates there's something wrong somewhere else in the code, but I'm not sure what
		if ( !qwglUseFontBitmaps(hDC, 0, 255, g_qeglobals.d_font_list) ) {
			common->Warning( "wglUseFontBitmaps failed again (%d).  Trying outlines.", GetLastError() );

			if (!qwglUseFontOutlines(hDC, 0, 255, g_qeglobals.d_font_list, 0.0f, 0.1f, WGL_FONT_LINES, NULL)) {
				common->Warning( "wglUseFontOutlines also failed (%d), no coordinate text will be visible.", GetLastError() );
			}
		}
	}

	SelectObject(hDC, hOldFont);
	ReleaseDC(pDC);

	// indicate start of glyph display lists
	qglListBase(g_qeglobals.d_font_list);

	// report OpenGL information
	common->Printf("GL_VENDOR: %s\n", qglGetString(GL_VENDOR));
	common->Printf("GL_RENDERER: %s\n", qglGetString(GL_RENDERER));
	common->Printf("GL_VERSION: %s\n", qglGetString(GL_VERSION));
	common->Printf("GL_EXTENSIONS: %s\n", qglGetString(GL_EXTENSIONS));

	return 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OriginalMouseUp(UINT nFlags, CPoint point) {
	CRect	r;
	GetClientRect(r);
	Cam_MouseUp(point.x, r.bottom - 1 - point.y, nFlags);
	if (!(nFlags & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON))) {
		ReleaseCapture();
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OriginalMouseDown(UINT nFlags, CPoint point) {
	// if (GetTopWindow()->GetSafeHwnd() != GetSafeHwnd()) BringWindowToTop();
	CRect	r;
	GetClientRect(r);
	SetFocus();
	SetCapture();

	// if (!(GetAsyncKeyState(VK_MENU) & 0x8000))
	Cam_MouseDown(point.x, r.bottom - 1 - point.y, nFlags);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::Cam_Init() {
	// m_Camera.draw_mode = cd_texture;
	m_Camera.origin[0] = 0.0f;
	m_Camera.origin[1] = 20.0f;
	m_Camera.origin[2] = 72.0f;
	m_Camera.color[0] = 0.3f;
	m_Camera.color[1] = 0.3f;
	m_Camera.color[2] = 0.3f;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::Cam_BuildMatrix() {
	float	xa, ya;
	float	matrix[4][4];
	int		i;

	xa = ((renderMode) ? -m_Camera.angles[PITCH] : m_Camera.angles[PITCH]) * idMath::M_DEG2RAD;
	ya = m_Camera.angles[YAW] * idMath::M_DEG2RAD;

	// the movement matrix is kept 2d
	m_Camera.forward[0] = cos(ya);
	m_Camera.forward[1] = sin(ya);
	m_Camera.right[0] = m_Camera.forward[1];
	m_Camera.right[1] = -m_Camera.forward[0];

	qglGetFloatv(GL_PROJECTION_MATRIX, &matrix[0][0]);

	for (i = 0; i < 3; i++) {
		m_Camera.vright[i] = matrix[i][0];
		m_Camera.vup[i] = matrix[i][1];
		m_Camera.vpn[i] = matrix[i][2];
	}

	m_Camera.vright.Normalize();
	m_Camera.vup.Normalize();
	m_Camera.vpn.Normalize();
	InitCull();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void CCamWnd::Cam_ChangeFloor(bool up) {
	brush_t *b;
	float	d, bestd, current;
	idVec3	start, dir;

	start[0] = m_Camera.origin[0];
	start[1] = m_Camera.origin[1];
	start[2] = HUGE_DISTANCE;
	dir[0] = dir[1] = 0;
	dir[2] = -1;

	current = HUGE_DISTANCE - (m_Camera.origin[2] - 72);
	if (up) {
		bestd = 0;
	}
	else {
		bestd = HUGE_DISTANCE*2;
	}

	for (b = active_brushes.next; b != &active_brushes; b = b->next) {
		if (!Brush_Ray(start, dir, b, &d)) {
			continue;
		}

		if (up && d < current && d > bestd) {
			bestd = d;
		}

		if (!up && d > current && d < bestd) {
			bestd = d;
		}
	}

	if (bestd == 0 || bestd == HUGE_DISTANCE*2) {
		return;
	}

	m_Camera.origin[2] += current - bestd;
	Sys_UpdateWindows(W_CAMERA | W_Z_OVERLAY);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::Cam_PositionDrag() {
	int x, y;
	Sys_GetCursorPos(&x, &y);
	if (x != m_ptCursor.x || y != m_ptCursor.y) {
		x -= m_ptCursor.x;
		VectorMA(m_Camera.origin, x, m_Camera.vright, m_Camera.origin);
		y -= m_ptCursor.y;
		m_Camera.origin[2] -= y;
		SetCursorPos(m_ptCursor.x, m_ptCursor.y);
		Sys_UpdateWindows(W_CAMERA | W_XY_OVERLAY);
	}
}

void CCamWnd::Cam_MouseLook() {
	CPoint current;

	GetCursorPos(&current);
	if (current.x != m_ptCursor.x || current.y != m_ptCursor.y) {

		current.x -= m_ptCursor.x;
		current.y -= m_ptCursor.y;

		m_Camera.angles[PITCH] -= (float)((float)current.y * 0.25f);
		m_Camera.angles[YAW] -= (float)((float)current.x * 0.25f);

		SetCursorPos(m_ptCursor.x, m_ptCursor.y);

		Cam_BuildMatrix();
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::Cam_MouseControl(float dtime) {
	int		xl, xh;
	int		yl, yh;
	float	xf, yf;
	if (g_PrefsDlg.m_nMouseButtons == 2) {
		if (m_nCambuttonstate != (MK_RBUTTON | MK_SHIFT)) {
			return;
		}
	}
	else {
		if (m_nCambuttonstate != MK_RBUTTON) {
			return;
		}
	}

	xf = (float)(m_ptButton.x - m_Camera.width / 2) / (m_Camera.width / 2);
	yf = (float)(m_ptButton.y - m_Camera.height / 2) / (m_Camera.height / 2);

	xl = m_Camera.width / 3;
	xh = xl * 2;
	yl = m_Camera.height / 3;
	yh = yl * 2;

	// common->Printf("xf-%f yf-%f xl-%i xh-i% yl-i% yh-i%\n",xf,yf,xl,xh,yl,yh);
#if 0

	// strafe
	if (buttony < yl && (buttonx < xl || buttonx > xh)) {
		VectorMA(camera.origin, xf * dtime * g_nMoveSpeed, camera.right, camera.origin);
	}
	else
#endif 
	{
		xf *= 1.0f - idMath::Fabs(yf);
		if ( xf < 0.0f ) {
			xf += 0.1f;
			if ( xf > 0.0f ) {
				xf = 0.0f;
			}
		}
		else {
			xf -= 0.1f;
			if ( xf < 0.0f ) {
				xf = 0.0f;
			}
		}

		VectorMA(m_Camera.origin, yf * dtime * g_PrefsDlg.m_nMoveSpeed, m_Camera.forward, m_Camera.origin);
		m_Camera.angles[YAW] += xf * -dtime * g_PrefsDlg.m_nAngleSpeed;
	}

	Cam_BuildMatrix();
	int nUpdate = (g_PrefsDlg.m_bCamXYUpdate) ? (W_CAMERA | W_XY) : (W_CAMERA);
	Sys_UpdateWindows(nUpdate);
	g_pParentWnd->PostMessage(WM_TIMER, 0, 0);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::Cam_MouseDown(int x, int y, int buttons) {
	idVec3	dir;
	float	f, r, u;
	int		i;

	// calc ray direction
	u = (float)(y - m_Camera.height / 2) / (m_Camera.width / 2);
	r = (float)(x - m_Camera.width / 2) / (m_Camera.width / 2);
	f = 1;

	for (i = 0; i < 3; i++) {
		dir[i] = m_Camera.vpn[i] * f + m_Camera.vright[i] * r + m_Camera.vup[i] * u;
	}

	dir.Normalize();

	GetCursorPos(&m_ptCursor);

	m_nCambuttonstate = buttons;
	m_ptButton.x = x;
	m_ptButton.y = y;

	//
	// LBUTTON = manipulate selection shift-LBUTTON = select middle button = grab
	// texture ctrl-middle button = set entire brush to texture ctrl-shift-middle
	// button = set single face to texture
	//
	int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;
	if
	(
		(buttons == MK_LBUTTON) ||
		(buttons == (MK_LBUTTON | MK_SHIFT)) ||
		(buttons == (MK_LBUTTON | MK_CONTROL)) ||
		(buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT)) ||
		(buttons == nMouseButton) ||
		(buttons == (nMouseButton | MK_SHIFT)) ||
		(buttons == (nMouseButton | MK_CONTROL)) ||
		(buttons == (nMouseButton | MK_SHIFT | MK_CONTROL))
	) {
		if (g_PrefsDlg.m_nMouseButtons == 2 && (buttons == (MK_RBUTTON | MK_SHIFT))) {
			Cam_MouseControl( 0.1f );
		}
		else {
			// something global needs to track which window is responsible for stuff
			Patch_SetView(W_CAMERA);
			Drag_Begin(x, y, buttons, m_Camera.vright, m_Camera.vup, m_Camera.origin, dir);
		}

		return;
	}

	if ( buttons == MK_RBUTTON ) {
		Cam_MouseControl( 0.1f );
		return;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::Cam_MouseUp(int x, int y, int buttons) {
	m_nCambuttonstate = 0;
	Drag_MouseUp(buttons);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::Cam_MouseMoved(int x, int y, int buttons) {
	m_nCambuttonstate = buttons;
	if (!buttons) {
		return;
	}

	m_ptButton.x = x;
	m_ptButton.y = y;

	if (buttons == (MK_RBUTTON | MK_CONTROL)) {
		Cam_PositionDrag();
		Sys_UpdateWindows(W_XY | W_CAMERA | W_Z);
		return;
	}
	else if ( buttons == (MK_RBUTTON | MK_CONTROL | MK_SHIFT) ) {
		Cam_MouseLook();
		Sys_UpdateWindows(W_XY | W_CAMERA | W_Z);
		return;
	}

	GetCursorPos(&m_ptCursor);

	if (buttons & (MK_LBUTTON | MK_MBUTTON)) {
		Drag_MouseMoved(x, y, buttons);
		Sys_UpdateWindows(W_XY | W_CAMERA | W_Z);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::InitCull() {
	int i;

	VectorSubtract(m_Camera.vpn, m_Camera.vright, m_vCull1);
	VectorAdd(m_Camera.vpn, m_Camera.vright, m_vCull2);

	for (i = 0; i < 3; i++) {
		if (m_vCull1[i] > 0) {
			m_nCullv1[i] = 3 + i;
		}
		else {
			m_nCullv1[i] = i;
		}

		if (m_vCull2[i] > 0) {
			m_nCullv2[i] = 3 + i;
		}
		else {
			m_nCullv2[i] = i;
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CCamWnd::CullBrush(brush_t *b, bool cubicOnly) {
	int		i;
	idVec3	point;
	float	d;

	if ( b->forceVisibile ) {
		return false;
	}

	if (g_PrefsDlg.m_bCubicClipping) {
		
		float distance = g_PrefsDlg.m_nCubicScale * 64;

		idVec3 mid;
		for (int i = 0; i < 3; i++) {
			mid[i] = (b->mins[i] + ((b->maxs[i] - b->mins[i]) / 2));
		}

		point = mid - m_Camera.origin;
		if (point.Length() > distance) {
			return true;
		}

	}

	if (cubicOnly) {
		return false;
	}

	for (i = 0; i < 3; i++) {
		point[i] = b->mins[m_nCullv1[i]] - m_Camera.origin[i];
	}

	d = DotProduct(point, m_vCull1);
	if (d < -1) {
		return true;
	}

	for (i = 0; i < 3; i++) {
		point[i] = b->mins[m_nCullv2[i]] - m_Camera.origin[i];
	}

	d = DotProduct(point, m_vCull2);
	if (d < -1) {
		return true;
	}

	return false;
}

#if 0

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::DrawLightRadius(brush_t *pBrush) {
	// if lighting
	int nRadius = Brush_LightRadius(pBrush);
	if (nRadius > 0) {
		Brush_SetLightColor(pBrush);
		qglEnable(GL_BLEND);
		qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglDisable(GL_BLEND);
		qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
#endif

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void setGLMode(int mode) {
	switch (mode)
	{
		case cd_wire:
			qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			globalImages->BindNull();
			qglDisable(GL_BLEND);
			qglDisable(GL_DEPTH_TEST);
			qglColor3f( 1.0f, 1.0f, 1.0f );
			break;

		case cd_solid:
			qglCullFace(GL_FRONT);
			qglEnable(GL_CULL_FACE);
			qglShadeModel(GL_FLAT);
			qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			globalImages->BindNull();
			qglDisable(GL_BLEND);
			qglEnable(GL_DEPTH_TEST);
			qglDepthFunc(GL_LEQUAL);
			break;

		case cd_texture:
			qglCullFace(GL_FRONT);
			qglEnable(GL_CULL_FACE);
			qglShadeModel(GL_FLAT);
			qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			qglDisable(GL_BLEND);
			qglEnable(GL_DEPTH_TEST);
			qglDepthFunc(GL_LEQUAL);
			break;

		case cd_blend:
			qglCullFace(GL_FRONT);
			qglEnable(GL_CULL_FACE);
			qglShadeModel(GL_FLAT);
			qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			qglDisable(GL_DEPTH_TEST);
			qglEnable(GL_BLEND);
			qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
	}
}


extern void glLabeledPoint(idVec4 &color, idVec3 &point, float size, const char *label);
void DrawAxial(face_t *selFace) {
	if (g_bAxialMode) {
		idVec3 points[4];
	
		for (int j = 0; j < selFace->face_winding->GetNumPoints(); j++) {
			glLabeledPoint(idVec4(1, 1, 1, 1), (*selFace->face_winding)[j].ToVec3(), 3, va("%i", j));
		}
		
		ValidateAxialPoints();
		points[0] = (*selFace->face_winding)[g_axialAnchor].ToVec3();
		VectorMA (points[0], 1, selFace->plane, points[0]);
		VectorMA (points[0], 4, selFace->plane, points[1]);
		points[3] = (*selFace->face_winding)[g_axialDest].ToVec3();
		VectorMA (points[3], 1, selFace->plane, points[3]);
		VectorMA (points[3], 4, selFace->plane, points[2]);
		glLabeledPoint(idVec4(1, 0, 0, 1), points[1], 3, "Anchor");
		glLabeledPoint(idVec4(1, 1, 0, 1), points[2], 3, "Dest");
		qglBegin (GL_LINE_STRIP);
		qglVertex3fv( points[0].ToFloatPtr() );
		qglVertex3fv( points[1].ToFloatPtr() );
		qglVertex3fv( points[2].ToFloatPtr() );
		qglVertex3fv( points[3].ToFloatPtr() );
		qglEnd();
	}
}


/*
 =======================================================================================================================
    Cam_Draw
 =======================================================================================================================
 */
void CCamWnd::SetProjectionMatrix() {
	float xfov = 90;
	float yfov = 2 * atan((float)m_Camera.height / m_Camera.width) * idMath::M_RAD2DEG;
#if 0
	float screenaspect = (float)m_Camera.width / m_Camera.height;
	qglLoadIdentity();
	gluPerspective(yfov, screenaspect, 2, 8192);
#else
	float	xmin, xmax, ymin, ymax;
	float	width, height;
	float	zNear;
	float	projectionMatrix[16];

	//
	// set up projection matrix
	//
	zNear	= r_znear.GetFloat();

	ymax = zNear * tan( yfov * idMath::PI / 360.0f );
	ymin = -ymax;

	xmax = zNear * tan( xfov * idMath::PI / 360.0f );
	xmin = -xmax;

	width = xmax - xmin;
	height = ymax - ymin;

	projectionMatrix[0] = 2 * zNear / width;
	projectionMatrix[4] = 0;
	projectionMatrix[8] = ( xmax + xmin ) / width;	// normally 0
	projectionMatrix[12] = 0;

	projectionMatrix[1] = 0;
	projectionMatrix[5] = 2 * zNear / height;
	projectionMatrix[9] = ( ymax + ymin ) / height;	// normally 0
	projectionMatrix[13] = 0;

	// this is the far-plane-at-infinity formulation
	projectionMatrix[2] = 0;
	projectionMatrix[6] = 0;
	projectionMatrix[10] = -1;
	projectionMatrix[14] = -2 * zNear;

	projectionMatrix[3] = 0;
	projectionMatrix[7] = 0;
	projectionMatrix[11] = -1;
	projectionMatrix[15] = 0;

	qglLoadMatrixf( projectionMatrix );
#endif
}

void CCamWnd::Cam_Draw() {
	brush_t *brush;
	face_t	*face;

	// float yfov;
	int		i;

	if (!active_brushes.next) {
		return;					// not valid yet
	}

	// set the sound origin for both simple draw and rendered mode
	// the editor uses opposite pitch convention
	idMat3	axis = idAngles( -m_Camera.angles.pitch, m_Camera.angles.yaw, m_Camera.angles.roll ).ToMat3();
	g_qeglobals.sw->PlaceListener( m_Camera.origin, axis, 0, Sys_Milliseconds(), "Undefined" );

	if (renderMode) {
		Cam_Render();
	}

	qglViewport(0, 0, m_Camera.width, m_Camera.height);
	qglScissor(0, 0, m_Camera.width, m_Camera.height);
	qglClearColor(g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][0], g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][1], g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][2], 0);

	if (!renderMode) {
		qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	qglDisable(GL_LIGHTING);
	qglMatrixMode(GL_PROJECTION);

	SetProjectionMatrix();

	qglRotatef(-90, 1, 0, 0);	// put Z going up
	qglRotatef(90, 0, 0, 1);	// put Z going up
	qglRotatef(m_Camera.angles[0], 0, 1, 0);
	qglRotatef(-m_Camera.angles[1], 0, 0, 1);
	qglTranslatef(-m_Camera.origin[0], -m_Camera.origin[1], -m_Camera.origin[2]);

	Cam_BuildMatrix();
   
	for (brush = active_brushes.next; brush != &active_brushes; brush = brush->next) {

		if ( CullBrush(brush, false) ) {
			continue;
		}

		if ( FilterBrush(brush) ) {
			continue;
		}

		if (renderMode) {
			if (!(entityMode && brush->owner->eclass->fixedsize)) {
				continue;
			}
		}

		setGLMode(m_Camera.draw_mode);
		Brush_Draw(brush);
	}


	//qglDepthMask ( 1 ); // Ok, write now
	qglMatrixMode(GL_PROJECTION);

	qglTranslatef(g_qeglobals.d_select_translate[0],g_qeglobals.d_select_translate[1],g_qeglobals.d_select_translate[2]);

	brush_t *pList = (g_bClipMode && g_pSplitList) ? g_pSplitList : &selected_brushes;

	if (!renderMode) {
		// draw normally
		for (brush = pList->next; brush != pList; brush = brush->next) {
			if (brush->pPatch) {
				continue;
			}
			setGLMode(m_Camera.draw_mode);
			Brush_Draw(brush, true);
		}
	}

	// blend on top

	setGLMode(m_Camera.draw_mode);
	qglDisable(GL_LIGHTING);
	qglColor4f( g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0],g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1],g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2], 0.25f );
	qglEnable(GL_BLEND);
	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	globalImages->BindNull();
	for (brush = pList->next; brush != pList; brush = brush->next) {
		if (brush->pPatch || brush->modelHandle > 0) {
			Brush_Draw(brush, true);

			// DHM - Nerve:: patch display lists/models mess with the state
			qglEnable(GL_BLEND);
			qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			qglColor4f( g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0],g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1],g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2], 0.25f );
			globalImages->BindNull();
			continue;
		}

        if ( brush->owner->eclass->entityModel ) {
            continue;
        }

		for (face = brush->brush_faces; face; face = face->next) {
			Face_Draw(face);
		}
	}

	int nCount = g_ptrSelectedFaces.GetSize();

	if (!renderMode) {
		for (int i = 0; i < nCount; i++) {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(i));
			Face_Draw(selFace);
			DrawAxial(selFace);
		}
	} 

	// non-zbuffered outline
	qglDisable(GL_BLEND);
	qglDisable(GL_DEPTH_TEST);
	qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (renderMode) {
		qglColor3f(1, 0, 0);
		for (int i = 0; i < nCount; i++) {
			face_t	*selFace = reinterpret_cast < face_t * > (g_ptrSelectedFaces.GetAt(i));
			Face_Draw(selFace);
		}
	}

	qglColor3f(1, 1, 1);
	for (brush = pList->next; brush != pList; brush = brush->next) {
		if (brush->pPatch || brush->modelHandle > 0) {
			continue;
		}

		for (face = brush->brush_faces; face; face = face->next) {
			Face_Draw(face);
		}
	}
	// edge / vertex flags
	if (g_qeglobals.d_select_mode == sel_vertex) {
		qglPointSize(4);
		qglColor3f(0, 1, 0);
		qglBegin(GL_POINTS);
		for (i = 0; i < g_qeglobals.d_numpoints; i++) {
			qglVertex3fv( g_qeglobals.d_points[i].ToFloatPtr() );
		}

		qglEnd();
		qglPointSize(1);
	}
	else if (g_qeglobals.d_select_mode == sel_edge) {
		float	*v1, *v2;

		qglPointSize(4);
		qglColor3f(0, 0, 1);
		qglBegin(GL_POINTS);
		for (i = 0; i < g_qeglobals.d_numedges; i++) {
			v1 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p1].ToFloatPtr();
			v2 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p2].ToFloatPtr();
			qglVertex3f( (v1[0] + v2[0]) * 0.5f, (v1[1] + v2[1]) * 0.5f, (v1[2] + v2[2]) * 0.5f );
		}

		qglEnd();
		qglPointSize(1);
	}

	g_splineList->draw (static_cast<bool>(g_qeglobals.d_select_mode == sel_addpoint || g_qeglobals.d_select_mode == sel_editpoint));
	
	if ( g_qeglobals.selectObject && (g_qeglobals.d_select_mode == sel_addpoint || g_qeglobals.d_select_mode == sel_editpoint) ) {
		g_qeglobals.selectObject->drawSelection();
	}

	// draw pointfile
	qglEnable(GL_DEPTH_TEST);

	DrawPathLines();

	if (g_qeglobals.d_pointfile_display_list) {
		Pointfile_Draw();
	}

	//
	// bind back to the default texture so that we don't have problems elsewhere
	// using/modifying texture maps between contexts
	//
	globalImages->BindNull();

	qglFinish();
	QE_CheckOpenGLForErrors();

	if (!renderMode) {
		// clean up any deffered tri's
		R_ToggleSmpFrame();
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnSize(UINT nType, int cx, int cy) {
	CWnd::OnSize(nType, cx, cy);

	CRect	rect;
	GetClientRect(rect);
	m_Camera.width = rect.right;
	m_Camera.height = rect.bottom;
	InvalidateRect(NULL, false);
}


/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CCamWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
	g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags, false);
}

//
// =======================================================================================================================
//    Timo brush primitive texture shifting, using camera view to select translations::
// =======================================================================================================================
//
void CCamWnd::ShiftTexture_BrushPrimit(face_t *f, int x, int y) {
/*
	idVec3	texS, texT;
	idVec3	viewX, viewY;
	int		XS, XT, YS, YT;
	int		outS, outT;
#ifdef _DEBUG
	if (!g_qeglobals.m_bBrushPrimitMode) {
		common->Printf("Warning : unexpected call to CCamWnd::ShiftTexture_BrushPrimit with brush primitive mode disbaled\n");
		return;
	}
#endif
	// compute face axis base
	//ComputeAxisBase(f->plane.Normal(), texS, texT);

	// compute camera view vectors
	VectorCopy(m_Camera.vup, viewY);
	VectorCopy(m_Camera.vright, viewX);

	// compute best vectors
	//ComputeBest2DVector(viewX, texS, texT, XS, XT);
	//ComputeBest2DVector(viewY, texS, texT, YS, YT);

	// check this is not a degenerate case
	if ((XS == YS) && (XT == YT))
	{
#ifdef _DEBUG
		common->Printf("Warning : degenerate best vectors axis base in CCamWnd::ShiftTexture_BrushPrimit\n");
#endif
		// forget it
		Select_ShiftTexture_BrushPrimit(f, x, y, false);
		return;
	}

	// compute best fitted translation in face axis base
	outS = XS * x + YS * y;
	outT = XT * x + YT * y;

	// call actual texture shifting code
	Select_ShiftTexture_BrushPrimit(f, outS, outT, false);
*/
}


bool IsBModel(brush_t *b) {
	const char *v = ValueForKey( b->owner, "model" );
	if (v && *v) {
		const char *n = ValueForKey( b->owner, "name");
		return (stricmp( n, v ) == 0);
	}
	return false;
}

/*
================
BuildEntityRenderState

Creates or updates modelDef and lightDef for an entity
================
*/
int Brush_ToTris(brush_t *brush, idTriList *tris, idMatList *mats, bool models, bool bmodel);

void CCamWnd::BuildEntityRenderState( entity_t *ent, bool update) {
	const char	*v;
	idDict		spawnArgs;
	const char	*name = NULL;

	Entity_UpdateSoundEmitter( ent );

	// delete the existing def if we aren't creating a brand new world
	if ( !update ) {
		if ( ent->lightDef >= 0 ) {
			g_qeglobals.rw->FreeLightDef( ent->lightDef );
			ent->lightDef = -1;
		}

		if ( ent->modelDef >= 0 ) {
			g_qeglobals.rw->FreeEntityDef( ent->modelDef );
			ent->modelDef = -1;
		}
	}

	// if an entity doesn't have any brushes at all, don't do anything
	if ( ent->brushes.onext == &ent->brushes ) {
		return;
	}

	// if the brush isn't displayed (filtered or culled), don't do anything
	if (FilterBrush(ent->brushes.onext)) {
		return;
	}

	spawnArgs = ent->epairs;
	if (ent->eclass->defArgs.FindKey("model")) {
		spawnArgs.Set("model", ent->eclass->defArgs.GetString("model"));
	}

	// any entity can have a model
	name = ValueForKey( ent, "name" );
	v = spawnArgs.GetString("model");
	if ( v && *v ) {
		renderEntity_t	refent;

		refent.referenceSound = ent->soundEmitter;

		if ( !stricmp( name, v ) ) {
			// build the model from brushes
			idTriList tris(1024);
			idMatList mats(1024);

			for (brush_t *b = ent->brushes.onext; b != &ent->brushes; b = b->onext) {
				Brush_ToTris( b, &tris, &mats, false, true);
			}

			if ( ent->modelDef >= 0 ) {
				g_qeglobals.rw->FreeEntityDef( ent->modelDef );
				ent->modelDef = -1;
			}

			idRenderModel *bmodel = renderModelManager->FindModel( name );

			if ( bmodel ) {
				renderModelManager->RemoveModel( bmodel );
				renderModelManager->FreeModel( bmodel );
			}

			bmodel = renderModelManager->AllocModel();

			bmodel->InitEmpty( name );

			// add the surfaces to the renderModel
			modelSurface_t	surf;
			for ( int i = 0 ; i < tris.Num() ; i++ ) {
				surf.geometry = tris[i];
				surf.shader = mats[i];
				bmodel->AddSurface( surf );
			}

			bmodel->FinishSurfaces();

			renderModelManager->AddModel( bmodel );

			// FIXME: brush entities
			gameEdit->ParseSpawnArgsToRenderEntity( &spawnArgs, &refent );

			ent->modelDef = g_qeglobals.rw->AddEntityDef( &refent );

		} else {
			// use the game's epair parsing code so
			// we can use the same renderEntity generation
			gameEdit->ParseSpawnArgsToRenderEntity( &spawnArgs, &refent );
			idRenderModelMD5 *md5 = dynamic_cast<idRenderModelMD5 *>( refent.hModel );
			if (md5) {
				idStr str;
				spawnArgs.GetString("anim", "idle", str);
				refent.numJoints = md5->NumJoints();
				if ( update && refent.joints ) {
					Mem_Free16( refent.joints );
				}
				refent.joints = ( idJointMat * )Mem_Alloc16( refent.numJoints * sizeof( *refent.joints ) );
				const idMD5Anim *anim = gameEdit->ANIM_GetAnimFromEntityDef(spawnArgs.GetString("classname"), str);
				int frame = spawnArgs.GetInt("frame") + 1;
				if ( frame < 1 ) {
					frame = 1;
				}
				const idVec3 &offset = gameEdit->ANIM_GetModelOffsetFromEntityDef( spawnArgs.GetString("classname") );
				gameEdit->ANIM_CreateAnimFrame( md5, anim, refent.numJoints, refent.joints, ( frame * 1000 ) / 24, offset, false );
			}
			if (ent->modelDef >= 0) {
				g_qeglobals.rw->FreeEntityDef( ent->modelDef );
			}
			ent->modelDef = g_qeglobals.rw->AddEntityDef( &refent );
		}
	}

	// check for lightdefs
	if (!(ent->eclass->nShowFlags & ECLASS_LIGHT)) {
		return;
	}

	if ( spawnArgs.GetBool( "start_off" ) ) {
		return;
	}
	// use the game's epair parsing code so
	// we can use the same renderLight generation

	renderLight_t	lightParms;

	gameEdit->ParseSpawnArgsToRenderLight( &spawnArgs, &lightParms );
	lightParms.referenceSound = ent->soundEmitter;

	if (update && ent->lightDef >= 0) {
		g_qeglobals.rw->UpdateLightDef( ent->lightDef, &lightParms );
	} else {
		if (ent->lightDef >= 0) {
			g_qeglobals.rw->FreeLightDef(ent->lightDef);
		}
		ent->lightDef = g_qeglobals.rw->AddLightDef( &lightParms );
	}

}

void Tris_ToOBJ(const char *outFile, idTriList *tris, idMatList *mats) {
	idFile *f = fileSystem->OpenExplicitFileWrite( outFile );
	if ( f ) {
		char out[1024];
		strcpy(out, outFile);
		StripExtension(out);

		idList<idStr*> matNames;
		int i, j, k;
		int indexBase = 1;
		idStr lastMaterial("");
		int matCount = 0;
		//idStr basePath = cvarSystem->GetCVarString( "fs_savepath" );
		f->Printf( "mtllib %s.mtl\n", out );
		for (i = 0; i < tris->Num(); i++) {
			srfTriangles_t *tri = (*tris)[i];
			for (j = 0; j < tri->numVerts; j++) {
				f->Printf( "v %f %f %f\n", tri->verts[j].xyz.x, tri->verts[j].xyz.z, -tri->verts[j].xyz.y );
			}
			for (j = 0; j < tri->numVerts; j++) {
				f->Printf( "vt %f %f\n", tri->verts[j].st.x, 1.0f - tri->verts[j].st.y );
			}
			for (j = 0; j < tri->numVerts; j++) {
				f->Printf( "vn %f %f %f\n", tri->verts[j].normal.x, tri->verts[j].normal.y, tri->verts[j].normal.z );
			}

			if (stricmp( (*mats)[i]->GetName(), lastMaterial)) {
				lastMaterial = (*mats)[i]->GetName();

				bool found = false;
				for (k = 0; k < matNames.Num(); k++) {
					if ( idStr::Icmp(matNames[k]->c_str(), lastMaterial.c_str()) == 0 ) {
						found = true;
						// f->Printf( "usemtl m%i\n", k );
						f->Printf( "usemtl %s\n", lastMaterial.c_str() );
						break;
					}
				}

				if (!found) {
					// f->Printf( "usemtl m%i\n", matCount++ );
					f->Printf( "usemtl %s\n", lastMaterial.c_str() );
					matNames.Append(new idStr(lastMaterial));
				}
			}

			for (int j = 0; j < tri->numIndexes; j += 3) {
				int i1, i2, i3;
				i1 = tri->indexes[j+2] + indexBase;
				i2 = tri->indexes[j+1] + indexBase;
				i3 = tri->indexes[j] + indexBase; 
				f->Printf( "f %i/%i/%i %i/%i/%i %i/%i/%i\n", i1,i1,i1, i2,i2,i2, i3,i3,i3 );
			}

			indexBase += tri->numVerts;

		}
		fileSystem->CloseFile( f );

		strcat(out, ".mtl");
		f = fileSystem->OpenExplicitFileWrite( out );
		if (f) {
			for (k = 0; k < matNames.Num(); k++) {
				// This presumes the diffuse tga name matches the material name
				f->Printf( "newmtl %s\n\tNs 0\n\td 1\n\tillum 2\n\tKd 0 0 0 \n\tKs 0.22 0.22 0.22 \n\tKa 0 0 0 \n\tmap_Kd %s/base/%s.tga\n\n\n", matNames[k]->c_str(), "z:/d3xp", matNames[k]->c_str() );
			}
			fileSystem->CloseFile( f );
		}

	}
}

int Brush_TransformModel(brush_t *brush, idTriList *tris, idMatList *mats) {
	int ret = 0;
	if (brush->modelHandle > 0 ) {
		idRenderModel *model = brush->modelHandle;
		if (model) {
			float	a = FloatForKey(brush->owner, "angle");
			float	s, c;
			//FIXME: support full rotation matrix
			bool matrix = false;
			if (a) {
				s = sin( DEG2RAD(a) );
				c = cos( DEG2RAD(a) );
			}
			idMat3 mat;
			if (GetMatrixForKey(brush->owner, "rotation", mat)) {
				matrix = true;
			}


			for (int i = 0; i < model->NumSurfaces() ; i++) {
				const modelSurface_t	*surf = model->Surface( i );
				srfTriangles_t	*tri = surf->geometry;
				srfTriangles_t *tri2 = R_CopyStaticTriSurf(tri);
				for (int j = 0; j < tri2->numVerts; j++) {
					idVec3	v;
					if (matrix) {
						v = tri2->verts[j].xyz * brush->owner->rotation + brush->owner->origin;
					} else {
						v = tri2->verts[j].xyz;
						VectorAdd(v, brush->owner->origin, v);
						float x = v[0];
						float y = v[1];
						if (a) {
							float	x2 = (((x - brush->owner->origin[0]) * c) - ((y - brush->owner->origin[1]) * s)) + brush->owner->origin[0];
							float	y2 = (((x - brush->owner->origin[0]) * s) + ((y - brush->owner->origin[1]) * c)) + brush->owner->origin[1];
							x = x2;
							y = y2;
						}
						v[0] = x;
						v[1] = y;
					}
					tri2->verts[j].xyz = v;
				}
				tris->Append(tri2);
				mats->Append( surf->shader );
			}
			return model->NumSurfaces();
		}
	}
	return ret;
}


#define	MAX_TRI_SURFACES	16384
int Brush_ToTris(brush_t *brush, idTriList *tris, idMatList *mats, bool models, bool bmodel) {
	int i, j;
	srfTriangles_t	*tri;
	//
	// patches
	//
	if (brush->modelHandle > 0 ) {
		if (!models) {
			return 0;
		} else {
			return Brush_TransformModel(brush, tris, mats);
		}
	}

	int numSurfaces = 0;

	if ( brush->owner->eclass->fixedsize && !brush->entityModel) {
		return NULL;
	}

	if ( brush->pPatch ) {
		patchMesh_t *pm;
		int			width, height;

		pm = brush->pPatch;

		// build a patch mesh
		idSurface_Patch *cp = new idSurface_Patch( pm->width * 6, pm->height * 6 );
		cp->SetSize( pm->width, pm->height );
		for ( i = 0; i < pm->width; i++ ) {
			for ( j = 0; j < pm->height; j++ ) {
				(*cp)[j*cp->GetWidth()+i].xyz =  pm->ctrl(i, j).xyz;
				(*cp)[j*cp->GetWidth()+i].st = pm->ctrl(i, j).st;
			}
		}

		// subdivide it
		if ( pm->explicitSubdivisions ) {
			cp->SubdivideExplicit( pm->horzSubdivisions, pm->vertSubdivisions, true );
		} else {
			cp->Subdivide( DEFAULT_CURVE_MAX_ERROR, DEFAULT_CURVE_MAX_ERROR, DEFAULT_CURVE_MAX_LENGTH, true );
		}
		width = cp->GetWidth();
		height = cp->GetHeight();

		// convert to srfTriangles
		tri = R_AllocStaticTriSurf();
		tri->numVerts = width * height;
		tri->numIndexes = 6 * ( width - 1 ) * ( height - 1 );
		R_AllocStaticTriSurfVerts( tri, tri->numVerts );
		R_AllocStaticTriSurfIndexes( tri, tri->numIndexes );
		for ( i = 0 ; i < tri->numVerts ; i++ ) {
			tri->verts[i] = (*cp)[i];
			if (bmodel) {
				tri->verts[i].xyz -= brush->owner->origin;
			}
		}

		tri->numIndexes = 0;
		for ( i = 1 ; i < width ; i++ ) {
			for ( j = 1 ; j < height ; j++ ) {
				tri->indexes[tri->numIndexes++] = ( j - 1 ) * width + i;
				tri->indexes[tri->numIndexes++] = ( j - 1 ) * width + i - 1;
				tri->indexes[tri->numIndexes++] = j * width + i - 1;

				tri->indexes[tri->numIndexes++] = j * width + i;
				tri->indexes[tri->numIndexes++] = ( j - 1 ) * width + i;
				tri->indexes[tri->numIndexes++] = j * width + i - 1;
			}
		}

		delete cp;

		tris->Append(tri);
		mats->Append(pm->d_texture);
		//surfaces[numSurfaces] = tri;
		//materials[numSurfaces] = pm->d_texture;
		return 1;
	}

	//
	// normal brush
	//
	for ( face_t *face = brush->brush_faces ; face; face = face->next ) {
		idWinding *w;

		w = face->face_winding;
		if (!w) {
			continue;	// freed or degenerate face
		}

		tri = R_AllocStaticTriSurf();
		tri->numVerts = w->GetNumPoints();
		tri->numIndexes = ( w->GetNumPoints() - 2 ) * 3;
		R_AllocStaticTriSurfVerts( tri, tri->numVerts );
		R_AllocStaticTriSurfIndexes( tri, tri->numIndexes );

		for ( i = 0 ; i < tri->numVerts ; i++ ) {

			tri->verts[i].Clear();

			tri->verts[i].xyz[0] = (*w)[i][0];
			tri->verts[i].xyz[1] = (*w)[i][1];
			tri->verts[i].xyz[2] = (*w)[i][2];

			if ( bmodel ) {
				tri->verts[i].xyz -= brush->owner->origin;
			}

			tri->verts[i].st[0] = (*w)[i][3];
			tri->verts[i].st[1] = (*w)[i][4];

			tri->verts[i].normal = face->plane.Normal();
		}

		tri->numIndexes = 0;
		for ( i = 2 ; i < w->GetNumPoints() ; i++ ) {
			tri->indexes[tri->numIndexes++] = 0;
			tri->indexes[tri->numIndexes++] = i-1;
			tri->indexes[tri->numIndexes++] = i;
		}

		tris->Append(tri);
		mats->Append(face->d_texture);
		numSurfaces++;
	}
	
	return numSurfaces;
}

void Select_ToOBJ() {
	int i;
	CFileDialog dlgFile(FALSE, "obj", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Wavefront object files (*.obj)|*.obj||", g_pParentWnd);
	if (dlgFile.DoModal() == IDOK) {
		idTriList tris(1024);
		idMatList mats(1024);

		for (brush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next) {

			if ( b->hiddenBrush ) {
				continue;
			}

			if (FilterBrush(b)) {
				continue;
			}

			Brush_ToTris(b, &tris, &mats, true, false);
		}

		Tris_ToOBJ(dlgFile.GetPathName().GetBuffer(0), &tris, &mats);

		for( i = 0; i < tris.Num(); i++ ) {
			R_FreeStaticTriSurf( tris[i] );
		}
		tris.Clear();
	}
}

void Select_ToCM() {
	CFileDialog dlgFile( FALSE, "lwo, ase", NULL, 0, "(*.lwo)|*.lwo|(*.ase)|*.ase|(*.ma)|*.ma||", g_pParentWnd );

	if ( dlgFile.DoModal() == IDOK ) {
		idMapEntity *mapEnt;
		idMapPrimitive *p;
		idStr name;

		name = fileSystem->OSPathToRelativePath( dlgFile.GetPathName() );
		name.BackSlashesToSlashes();

		mapEnt = new idMapEntity();
		mapEnt->epairs.Set( "name", name.c_str() );

		for ( brush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next ) {

			if ( b->hiddenBrush ) {
				continue;
			}

			if ( FilterBrush( b ) ) {
				continue;
			}

			p = BrushToMapPrimitive( b, b->owner->origin );
			if ( p ) {
				mapEnt->AddPrimitive( p );
			}
		}

		collisionModelManager->WriteCollisionModelForMapEntity( mapEnt, name.c_str() );

		delete mapEnt;
	}
}


/*
=================
BuildRendererState

Builds models, lightdefs, and modeldefs for the current editor data
so it can be rendered by the game renderSystem
=================
*/
void CCamWnd::BuildRendererState() {
	renderEntity_t	worldEntity;
	entity_t	*ent;
	brush_t		*brush;

	FreeRendererState();

	// the renderWorld holds all the references and defs
	g_qeglobals.rw->InitFromMap( NULL );

	// create the raw model for all the brushes
	int numBrushes = 0;
	int numSurfaces = 0;

	// the renderModel for the world holds all the geometry that isn't in an entity
	worldModel = renderModelManager->AllocModel();
	worldModel->InitEmpty( "EditorWorldModel" );

	for ( brush_t *brushList = &active_brushes ; brushList ; 
		brushList = (brushList == &active_brushes) ? &selected_brushes : NULL ) {

		for (brush = brushList->next; brush != brushList; brush = brush->next) {

			if ( brush->hiddenBrush ) {
				continue;
			}

			if (FilterBrush(brush)) {
				continue;
			}

			if (CullBrush(brush, true)) {
				continue;
			}

			idTriList tris(1024);
			idMatList mats(1024);
			
			if (!IsBModel(brush)) {
				numSurfaces += Brush_ToTris( brush, &tris, &mats, false, false );
			}

			// add the surfaces to the renderModel
			modelSurface_t	surf;
			for ( int i = 0 ; i < tris.Num() ; i++ ) {
				surf.geometry = tris[i];
				surf.shader = mats[i];
				worldModel->AddSurface( surf );
			}

			numBrushes++;
		}
	}

	// bound and clean the triangles
	worldModel->FinishSurfaces();

	// the worldEntity just has the handle for the worldModel
	memset( &worldEntity, 0, sizeof( worldEntity ) );
	worldEntity.hModel = worldModel;
	worldEntity.axis = mat3_default;
	worldEntity.shaderParms[0] = 1;
	worldEntity.shaderParms[1] = 1;
	worldEntity.shaderParms[2] = 1;
	worldEntity.shaderParms[3] = 1;

	worldModelDef = g_qeglobals.rw->AddEntityDef( &worldEntity );

	// create the light and model entities exactly the way the game code would
	for ( ent = entities.next ; ent != &entities ; ent = ent->next ) {
		if ( ent->brushes.onext == &ent->brushes ) {
			continue;
		}

		if (CullBrush(ent->brushes.onext, true)) {
			continue;
		}

		if (Map_IsBrushFiltered(ent->brushes.onext)) {
			continue;
		}

		BuildEntityRenderState( ent, false );
	}

	//common->Printf("Render data used %d brushes\n", numBrushes);
	worldDirty = false;
	UpdateCaption();
}

/*
===============================
CCamWnd::UpdateRenderEntities

  Creates a new entity state list
  returns true if a repaint is needed
===============================
*/
bool CCamWnd::UpdateRenderEntities() {

	if (rebuildMode) {
		return false;
	}

	bool ret = false;
	for ( entity_t *ent = entities.next ; ent != &entities ; ent = ent->next ) {
		BuildEntityRenderState( ent, (ent->lightDef != -1 || ent->modelDef != -1 || ent->soundEmitter ) ? true : false );
		if (ret == false && ent->modelDef || ent->lightDef) {
			ret = true;
		}
	}
	return ret;
}

/*
============================
CCamWnd::FreeRendererState

  Frees the render state data
============================
*/
void CCamWnd::FreeRendererState() {

	for ( entity_t *ent = entities.next ; ent != &entities ; ent = ent->next ) {
		if (ent->lightDef >= 0) {
			g_qeglobals.rw->FreeLightDef( ent->lightDef );
			ent->lightDef = -1;
		}

		if (ent->modelDef >= 0) {
			renderEntity_t *refent = const_cast<renderEntity_t *>(g_qeglobals.rw->GetRenderEntity( ent->modelDef ));
			if ( refent ) {
				if ( refent->callbackData ) {
					Mem_Free( refent->callbackData );
					refent->callbackData = NULL;
				}
				if ( refent->joints ) {
					Mem_Free16(refent->joints);
					refent->joints = NULL;
				}
			}
			g_qeglobals.rw->FreeEntityDef( ent->modelDef );
			ent->modelDef = -1;
		}
	}

	if ( worldModel ) {
		renderModelManager->FreeModel( worldModel );
		worldModel = NULL;
	}

}


/*
========================
CCamWnd::UpdateCaption

  updates the caption based on rendermode and whether the render mode needs updated
========================
*/
void CCamWnd::UpdateCaption() {
	
	idStr strCaption;

	if (worldDirty) {
		strCaption = "*";
	}
	// FIXME: 	
	strCaption += (renderMode) ? "RENDER" : "CAM";
	if (renderMode) {
		strCaption += (rebuildMode) ? " (Realtime)" : "";
		strCaption += (entityMode) ? " +lights" : "";
		strCaption += (selectMode) ? " +selected" : "";
		strCaption += (animationMode) ? " +anim" : "";
	}
	strCaption += (soundMode) ? " +snd" : "";
	SetWindowText(strCaption);
}

/*
===========================
CCamWnd::ToggleRenderMode

	Toggles the render mode
===========================
*/
void CCamWnd::ToggleRenderMode() {
	renderMode ^= 1;
	UpdateCaption();
}

/*
===========================
CCamWnd::ToggleRebuildMode

	Toggles the rebuild mode
===========================
*/
void CCamWnd::ToggleRebuildMode() {
	rebuildMode ^= 1;
	UpdateCaption();
}

/*
===========================
CCamWnd::ToggleEntityMode

	Toggles the entity mode
===========================
*/
void CCamWnd::ToggleEntityMode() {
	entityMode ^= 1;
	UpdateCaption();
}


/*
===========================
CCamWnd::ToggleRenderMode

	Toggles the render mode
===========================
*/
void CCamWnd::ToggleAnimationMode() {
	animationMode ^= 1;
	if (animationMode)  {
		SetTimer(0, 10, NULL);
	} else {
		KillTimer(0);
	}
	UpdateCaption();
}

/*
===========================
CCamWnd::ToggleSoundMode

	Toggles the sound mode
===========================
*/
void CCamWnd::ToggleSoundMode() {
	soundMode ^= 1;

	UpdateCaption();

	for ( entity_t *ent = entities.next ; ent != &entities ; ent = ent->next ) {
		Entity_UpdateSoundEmitter( ent );
	}
}

/*
===========================
CCamWnd::ToggleRenderMode

	Toggles the render mode
===========================
*/
void CCamWnd::ToggleSelectMode() {
	selectMode ^= 1;
	UpdateCaption();
}

/*
=========================
CCamWnd::MarkWorldDirty

  marks the render world as dirty
=========================
*/
void CCamWnd::MarkWorldDirty() {
	worldDirty = true;
	UpdateCaption();
}


/*
=========================
CCamWnd::DrawEntityData

  Draws entity data ( experimental )
=========================
*/
extern void glBox(idVec4 &color, idVec3 &point, float size);

void CCamWnd::DrawEntityData() {

	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();
	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity();

	SetProjectionMatrix();

	qglRotatef(-90, 1, 0, 0);	// put Z going up
	qglRotatef(90, 0, 0, 1);	// put Z going up
	qglRotatef(m_Camera.angles[0], 0, 1, 0);
	qglRotatef(-m_Camera.angles[1], 0, 0, 1);
	qglTranslatef(-m_Camera.origin[0], -m_Camera.origin[1], -m_Camera.origin[2]);

	Cam_BuildMatrix();

	if (!(entityMode || selectMode)) {
		return;
	}

	qglDisable(GL_BLEND);
	qglDisable(GL_DEPTH_TEST);
	qglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	globalImages->BindNull();
	idVec3 color(0, 1, 0);
	qglColor3fv( color.ToFloatPtr() );

	brush_t *brushList = &active_brushes;
	int pass = 0;
	while (brushList) {
		for (brush_t *brush = brushList->next; brush != brushList; brush = brush->next) {

			if (CullBrush(brush, true)) {
				continue;
			}

			if (FilterBrush(brush)) {
				continue;
			}

			if ((pass == 1 && selectMode) || (entityMode && pass == 0 && brush->owner->lightDef >= 0)) {
				Brush_DrawXY(brush, 0, true, true);
			}

		}
		brushList = (brushList == &active_brushes) ? &selected_brushes : NULL;
		color.x = 1;
		color.y = 0;
		pass++;
		qglColor3fv( color.ToFloatPtr() );
	}

}


/*
 =======================================================================================================================
    Cam_Render

	This used the renderSystem to draw a fully lit view of the world
 =======================================================================================================================
 */
void CCamWnd::Cam_Render() {

	renderView_t	refdef;
	CPaintDC	dc(this);	// device context for painting


	if (!active_brushes.next) {
		return;					// not valid yet
	}

	if (!qwglMakeCurrent(dc.m_hDC, win32.hGLRC)) {
		common->Printf("ERROR: wglMakeCurrent failed..\n ");
		common->Printf("Please restart " EDITOR_WINDOWTEXT " if the camera view is not working\n");
		return;
	}

	// save the editor state
	//qglPushAttrib( GL_ALL_ATTRIB_BITS );
	qglClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
	qglScissor( 0, 0, m_Camera.width, m_Camera.height );
	qglClear( GL_COLOR_BUFFER_BIT );

	//	qwglSwapBuffers(dc.m_hDC);

	// create the model, using explicit normals
	if ( rebuildMode && worldDirty ) {
		BuildRendererState();
	}

	// render it
	renderSystem->BeginFrame( m_Camera.width, m_Camera.height );

	memset( &refdef, 0, sizeof( refdef ) );
	refdef.vieworg = m_Camera.origin;

	// the editor uses opposite pitch convention
	refdef.viewaxis = idAngles( -m_Camera.angles.pitch, m_Camera.angles.yaw, m_Camera.angles.roll ).ToMat3();
	
	refdef.width = SCREEN_WIDTH;
	refdef.height = SCREEN_HEIGHT;
	refdef.fov_x = 90;
	refdef.fov_y = 2 * atan((float)m_Camera.height / m_Camera.width) * idMath::M_RAD2DEG;

	// only set in animation mode to give a consistent look 
	if (animationMode) {
		refdef.time = eventLoop->Milliseconds();
	}

	g_qeglobals.rw->RenderScene( &refdef );

	int	frontEnd, backEnd;

	renderSystem->EndFrame( &frontEnd, &backEnd );
//common->Printf( "front:%i back:%i\n", frontEnd, backEnd );

	//qglPopAttrib();
	//DrawEntityData();

	//qwglSwapBuffers(dc.m_hDC);
	// get back to the editor state
	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();
	Cam_BuildMatrix();
}


void CCamWnd::OnTimer(UINT nIDEvent) 
{
	if (animationMode || nIDEvent == 1) {
		Sys_UpdateWindows(W_CAMERA);
	}
	if (nIDEvent == 1) {
		KillTimer(1);
	}

	if (!animationMode ) {
		KillTimer(0);
	}
}


void CCamWnd::UpdateCameraView() {
	if (QE_SingleBrush(true, true)) {
		brush_t *b = selected_brushes.next;
		if (b->owner->eclass->nShowFlags & ECLASS_CAMERAVIEW) {
			// find the entity that targets this
			const char *name = ValueForKey(b->owner, "name");
			entity_t *ent = FindEntity("target", name);
			if (ent) {
				if (!saveValid) {
					saveOrg = m_Camera.origin;
					saveAng = m_Camera.angles;
					saveValid = true;
				}
				idVec3 v = b->owner->origin - ent->origin;
				v.Normalize();
				idAngles ang = v.ToMat3().ToAngles();
				ang.pitch = -ang.pitch;
				ang.roll = 0.0f;
                SetView( ent->origin, ang );
				Cam_BuildMatrix();
				Sys_UpdateWindows( W_CAMERA );
				return;
			}
		}
	}
	if (saveValid) {
		SetView(saveOrg, saveAng);
		Cam_BuildMatrix();
		Sys_UpdateWindows(W_CAMERA);
		saveValid = false;
	}
}


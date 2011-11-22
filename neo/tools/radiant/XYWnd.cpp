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
#include "DialogInfo.h"
#include "splines.h"
#include "../../renderer/tr_local.h"
#include "../../renderer/model_local.h"	// for idRenderModelLiquid

#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
static char		THIS_FILE[] = __FILE__;
#endif

const char		*g_pDimStrings[] = { "x:%.f", "y:%.f", "z:%.f" };
const char		*g_pOrgStrings[] = { "(x:%.f  y:%.f)", "(x:%.f  z:%.f)", "(y:%.f  z:%.f)" };
CString			g_strDim;
CString			g_strStatus;

bool			g_bCrossHairs = false;
bool			g_bScaleMode;
int				g_nScaleHow;
bool			g_bRotateMode;
bool			g_bClipMode;
bool			g_bRogueClipMode;
bool			g_bSwitch;
CClipPoint		g_Clip1;
CClipPoint		g_Clip2;
CClipPoint		g_Clip3;
CClipPoint		*g_pMovingClip;
brush_t			g_brFrontSplits;
brush_t			g_brBackSplits;

brush_t			g_brClipboard;
brush_t			g_brUndo;
entity_t		g_enClipboard;

idVec3			g_vRotateOrigin;
idVec3			g_vRotation;

bool			g_bPathMode;
CClipPoint		g_PathPoints[256];
CClipPoint		*g_pMovingPath;
int				g_nPathCount;
int				g_nPathLimit;

bool			g_bSmartGo;

bool			g_bPointMode;
CClipPoint		g_PointPoints[512];
CClipPoint		*g_pMovingPoint;
int				g_nPointCount;
int				g_nPointLimit;

const int		XY_LEFT = 0x01;
const int		XY_RIGHT = 0x02;
const int		XY_UP = 0x04;
const int		XY_DOWN = 0x08;

PFNPathCallback *g_pPathFunc = NULL;
void	Select_Ungroup();

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AcquirePath(int nCount, PFNPathCallback *pFunc) {
	g_nPathCount = 0;
	g_nPathLimit = nCount;
	g_pPathFunc = pFunc;
	g_bPathMode = true;
}

CPtrArray	g_ptrMenus;

CMemFile	g_Clipboard(4096);
CMemFile	g_PatchClipboard(4096);

extern int	pressx;
extern int	pressy;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
float fDiff(float f1, float f2) {
	if (f1 > f2) {
		return f1 - f2;
	}
	else {
		return f2 - f1;
	}
}

#define MAX_DRAG_POINTS 128

CPtrArray			dragPoints;
static CDragPoint	*activeDrag = NULL;
static bool			activeDragging = false;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CDragPoint::PointWithin(idVec3 p, int nView) {
	if (nView == -1) {
		if (fDiff(p[0], vec[0]) <= 3 && fDiff(p[1], vec[1]) <= 3 && fDiff(p[2], vec[2]) <= 3) {
			return true;
		}
	}
	else {
		int nDim1 = (nView == YZ) ? 1 : 0;
		int nDim2 = (nView == XY) ? 1 : 2;
		if (fDiff(p[nDim1], vec[nDim1]) <= 3 && fDiff(p[nDim2], vec[nDim2]) <= 3) {
			return true;
		}
	}

	return false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
CDragPoint *PointRay(const idVec3 &org, const idVec3 &dir, float *dist) {
	int			i, besti;
	float		d, bestd;
	idVec3		temp;
	CDragPoint	*drag = NULL;
	CDragPoint	*priority = NULL;

	// find the point closest to the ray
	float scale = g_pParentWnd->ActiveXY()->Scale();
	besti = -1;
	bestd = 12 / scale / 2;

	int count = dragPoints.GetSize();
	for (i = 0; i < count; i++) {
		drag = reinterpret_cast < CDragPoint * > (dragPoints[i]);
		temp = drag->vec - org;
		d = temp * dir;
		temp = org + d * dir;
		temp = drag->vec - temp;
		d = temp.Length();
		if ( d < bestd ) {
			bestd = d;
			besti = i;
			if (priority == NULL) {
				priority = reinterpret_cast < CDragPoint * > (dragPoints[besti]);
				if (!priority->priority) {
					priority = NULL;
				}
			}
		}
	}

	if (besti == -1) {
		return NULL;
	}

	drag = reinterpret_cast < CDragPoint * > (dragPoints[besti]);
	if (priority && !drag->priority) {
		drag = priority;
	}

	return drag;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ClearSelectablePoints(brush_t *b) {
	if (b == NULL) {
		dragPoints.RemoveAll();
	}
	else {
		CPtrArray	ptr;
		ptr.Copy(dragPoints);
		dragPoints.RemoveAll();

		int count = ptr.GetSize();
		for (int i = 0; i < count; i++) {
			if (b == reinterpret_cast < CDragPoint * > ( ptr.GetAt(i))->pBrush ) {
				continue;
			}
			else {
				dragPoints.Add(ptr.GetAt(i));
			}
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AddSelectablePoint(brush_t *b, idVec3 v, int type, bool priority) {
	dragPoints.Add(new CDragPoint(b, v, type, priority));
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void UpdateSelectablePoint(brush_t *b, idVec3 v, int type) {
	int count = dragPoints.GetSize();
	for (int i = 0; i < count; i++) {
		CDragPoint	*drag = reinterpret_cast < CDragPoint * > (dragPoints.GetAt(i));
		if (b == drag->pBrush && type == drag->nType) {
			VectorCopy(v, drag->vec);
			return;
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VectorToAngles(idVec3 vec, idVec3 angles) {
	float	forward;
	float	yaw, pitch;

	if ((vec[0] == 0) && (vec[1] == 0)) {
		yaw = 0;
		if (vec[2] > 0) {
			pitch = 90;
		}
		else {
			pitch = 270;
		}
	}
	else {
		yaw = RAD2DEG( atan2(vec[1], vec[0]) );
		if (yaw < 0) {
			yaw += 360;
		}

		forward = (float)idMath::Sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
		pitch = RAD2DEG( atan2(vec[2], forward) );
		if (pitch < 0) {
			pitch += 360;
		}
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

/*
 =======================================================================================================================
    RotateLight target is relative to the light origin up and right are relative to the target up and right are
    perpendicular and are on a plane through the target with the target vector as normal delta is the movement of the
    target relative to the light
 =======================================================================================================================
*/
void VectorSnapGrid(idVec3 &v) {
	v.x = floor(v.x / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
	v.y = floor(v.y / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
	v.z = floor(v.z / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
}

/*
 =======================================================================================================================
 =======================================================================================================================
*/
static void RotateLight(idVec3 &target, idVec3 &up, idVec3 &right, const idVec3 &delta) {
	idVec3	newtarget, cross, dst;
	idVec3	normal;
	double	angle, dist, d, len;
	idMat3	rot;

	// calculate new target
	newtarget = target + delta;

	// get the up and right vector relative to the light origin
	up += target;
	right += target;

	len = target.Length() * newtarget.Length();

	if (len > 0.1) {
		// calculate the rotation angle between the vectors
		double	dp = target * newtarget;
		double	dv = dp / len;

		angle = RAD2DEG( idMath::ACos( dv ) );

		// get a vector orthogonal to the rotation plane
		cross = target.Cross( newtarget );
		cross.Normalize();

		if (cross[0] || cross[1] || cross[2]) {
			// build the rotation matrix
			rot = idRotation( vec3_origin, cross, angle ).ToMat3();

			rot.ProjectVector(target, dst);
			target = dst;
			rot.ProjectVector( up, dst );
			up = dst;
			rot.ProjectVector( right, dst);
			right = dst;
		}
	}

	//
	// project the up and right vectors onto a plane that goes through the target and
	// has normal vector target.Normalize()
	//
	normal = target;
	normal.Normalize();
	dist = normal * target;

	d = (normal * up) - dist;
	up -= d * normal;

	d = (normal * right) - dist;
	right -= d * normal;

	//
	// FIXME: maybe calculate the right vector with a cross product between the target
	// and up vector, just to make sure the up and right vectors are perpendicular
	// get the up and right vectors relative to the target
	//
	up -= target;
	right -= target;

	// move the target in the (target - light_origin) direction
	target = newtarget;
	VectorSnapGrid(target);
	VectorSnapGrid(up);
	VectorSnapGrid(right);
}

/*
 =======================================================================================================================
 =======================================================================================================================
*/
extern idVec3 Brush_TransformedPoint(brush_t *b, const idVec3 &in);
extern idMat3 Brush_RotationMatrix(brush_t *b);
bool UpdateActiveDragPoint(const idVec3 &move) {
	if (activeDrag) {
		idMat3 mat = Brush_RotationMatrix(activeDrag->pBrush);
		idMat3 invmat = mat.Transpose();
		idVec3	target, up, right, start, end;
		CString str;
		if (activeDrag->nType == LIGHT_TARGET) {
			GetVectorForKey(activeDrag->pBrush->owner, "light_target", target);
			GetVectorForKey(activeDrag->pBrush->owner, "light_up", up);
			GetVectorForKey(activeDrag->pBrush->owner, "light_right", right);
			target *= mat;
			up *= mat;
			right *= mat;
			RotateLight(target, up, right, move);
			target *= invmat;
			up *= invmat;
			right *= invmat;
			SetKeyVec3(activeDrag->pBrush->owner, "light_target", target);
			SetKeyVec3(activeDrag->pBrush->owner, "light_up", up);
			SetKeyVec3(activeDrag->pBrush->owner, "light_right", right);
			target += (activeDrag->pBrush->trackLightOrigin) ? activeDrag->pBrush->owner->lightOrigin : activeDrag->pBrush->owner->origin;
			UpdateSelectablePoint(activeDrag->pBrush, Brush_TransformedPoint(activeDrag->pBrush, target), LIGHT_TARGET);
			up += target;
			UpdateSelectablePoint(activeDrag->pBrush, Brush_TransformedPoint(activeDrag->pBrush,up), LIGHT_UP);
			right += target;
			UpdateSelectablePoint(activeDrag->pBrush, Brush_TransformedPoint(activeDrag->pBrush,right), LIGHT_RIGHT);
		}
		else if (activeDrag->nType == LIGHT_UP) {
			GetVectorForKey(activeDrag->pBrush->owner, "light_up", up);
			up *= mat;
			up += move;
			up *= invmat;
			SetKeyVec3(activeDrag->pBrush->owner, "light_up", up);
			GetVectorForKey(activeDrag->pBrush->owner, "light_target", target);
			target += (activeDrag->pBrush->trackLightOrigin) ? activeDrag->pBrush->owner->lightOrigin : activeDrag->pBrush->owner->origin;
			up += target;
			UpdateSelectablePoint(activeDrag->pBrush, Brush_TransformedPoint(activeDrag->pBrush,up), LIGHT_UP);
		}
		else if (activeDrag->nType == LIGHT_RIGHT) {
			GetVectorForKey(activeDrag->pBrush->owner, "light_right", right);
			right *= mat;
			right += move;
			right *= invmat;
			SetKeyVec3(activeDrag->pBrush->owner, "light_right", right);
			GetVectorForKey(activeDrag->pBrush->owner, "light_target", target);
			target += (activeDrag->pBrush->trackLightOrigin) ? activeDrag->pBrush->owner->lightOrigin : activeDrag->pBrush->owner->origin;
			right += target;
			UpdateSelectablePoint(activeDrag->pBrush, Brush_TransformedPoint(activeDrag->pBrush,right), LIGHT_RIGHT);
		}
		else if (activeDrag->nType == LIGHT_START) {
			GetVectorForKey(activeDrag->pBrush->owner, "light_start", start);
			start *= mat;
			start += move;
			start *= invmat;
			SetKeyVec3(activeDrag->pBrush->owner, "light_start", start);
			start += (activeDrag->pBrush->trackLightOrigin) ? activeDrag->pBrush->owner->lightOrigin : activeDrag->pBrush->owner->origin;
			UpdateSelectablePoint(activeDrag->pBrush, Brush_TransformedPoint(activeDrag->pBrush,start), LIGHT_START);
		}
		else if (activeDrag->nType == LIGHT_END) {
			GetVectorForKey(activeDrag->pBrush->owner, "light_end", end);
			end *= mat;
			end += move;
			end *= invmat;
			SetKeyVec3(activeDrag->pBrush->owner, "light_end", end);
			end += (activeDrag->pBrush->trackLightOrigin) ? activeDrag->pBrush->owner->lightOrigin : activeDrag->pBrush->owner->origin;
			UpdateSelectablePoint(activeDrag->pBrush, Brush_TransformedPoint(activeDrag->pBrush,end), LIGHT_END);
		}
		else if (activeDrag->nType == LIGHT_CENTER) {
			GetVectorForKey(activeDrag->pBrush->owner, "light_center", end);
			end *= mat;
			end += move;
			end *= invmat;
			SetKeyVec3(activeDrag->pBrush->owner, "light_center", end);
			end += (activeDrag->pBrush->trackLightOrigin) ? activeDrag->pBrush->owner->lightOrigin : activeDrag->pBrush->owner->origin;
			UpdateSelectablePoint(activeDrag->pBrush, Brush_TransformedPoint(activeDrag->pBrush, end), LIGHT_CENTER);
		}

		// FIXME: just build the frustrum values
		Brush_Build(activeDrag->pBrush);
		return true;
	}

	return false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
*/
bool SetDragPointCursor(idVec3 p, int nView) {
	activeDrag = NULL;

	int numDragPoints = dragPoints.GetSize();
	for (int i = 0; i < numDragPoints; i++) {
		if (reinterpret_cast < CDragPoint * > (dragPoints[i])->PointWithin(p, nView)) {
			activeDrag = reinterpret_cast < CDragPoint * > (dragPoints[i]);
			return true;
		}
	}

	return false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetActiveDrag(CDragPoint *p) {
	activeDrag = p;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ClearActiveDrag() {
	activeDrag = NULL;
}

// CXYWnd
IMPLEMENT_DYNCREATE(CXYWnd, CWnd);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
CXYWnd::CXYWnd() {
	g_brClipboard.next = &g_brClipboard;
	g_brUndo.next = &g_brUndo;
	g_nScaleHow = 0;
	g_bRotateMode = false;
	g_bClipMode = false;
	g_bRogueClipMode = false;
	g_bSwitch = true;
	g_pMovingClip = NULL;
	g_pMovingPath = NULL;
	g_brFrontSplits.next = &g_brFrontSplits;
	g_brBackSplits.next = &g_brBackSplits;
	m_bActive = false;

	m_bRButtonDown = false;
	m_nUpdateBits = W_XY;
	g_bPathMode = false;
	g_nPathCount = 0;
	g_nPathLimit = 0;
	m_nTimerID = -1;
	m_nButtonstate = 0;
	XY_Init();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
CXYWnd::~CXYWnd() {
	int nSize = g_ptrMenus.GetSize();
	while (nSize > 0) {
		CMenu	*pMenu = reinterpret_cast < CMenu * > (g_ptrMenus.GetAt(nSize - 1));
		ASSERT(pMenu);
		pMenu->DestroyMenu();
		delete pMenu;
		nSize--;
	}

	g_ptrMenus.RemoveAll();
	m_mnuDrop.DestroyMenu();
}

BEGIN_MESSAGE_MAP(CXYWnd, CWnd)
//{{AFX_MSG_MAP(CXYWnd)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_SELECT_MOUSEROTATE, OnSelectMouserotate)
	ON_WM_TIMER()
	ON_WM_KEYUP()
	ON_WM_NCCALCSIZE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_DROP_NEWMODEL, OnDropNewmodel)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_ENTITY_START, ID_ENTITY_END, OnEntityCreate)
END_MESSAGE_MAP()
// CXYWnd message handlers
LONG WINAPI XYWndProc(HWND, UINT, WPARAM, LPARAM);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
BOOL CXYWnd::PreCreateWindow(CREATESTRUCT &cs) {
	WNDCLASS	wc;
	HINSTANCE	hInstance = AfxGetInstanceHandle();
	if (::GetClassInfo(hInstance, XY_WINDOW_CLASS, &wc) == FALSE) {
		// Register a new class
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_NOCLOSE;
		wc.lpszClassName = XY_WINDOW_CLASS;
		wc.hCursor = NULL;	// LoadCursor (NULL,IDC_ARROW);
		wc.lpfnWndProc = ::DefWindowProc;
		if (AfxRegisterClass(&wc) == FALSE) {
			Error("CCamWnd RegisterClass: failed");
		}
	}

	cs.lpszClass = XY_WINDOW_CLASS;
	cs.lpszName = "VIEW";
	if (cs.style != QE3_CHILDSTYLE) {
		cs.style = QE3_SPLITTER_STYLE;
	}

	return CWnd::PreCreateWindow(cs);
}

HDC				s_hdcXY;
HGLRC			s_hglrcXY;

static unsigned s_stipple[32] = {
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
	0xaaaaaaaa,
	0x55555555,
};

/*
 =======================================================================================================================
    WXY_WndProc
 =======================================================================================================================
 */
LONG WINAPI XYWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		case WM_DESTROY:
			return 0;

		case WM_NCCALCSIZE: // don't let windows copy pixels
			DefWindowProc(hWnd, uMsg, wParam, lParam);
			return WVR_REDRAW;

		case WM_KILLFOCUS:
		case WM_SETFOCUS:
			SendMessage(hWnd, WM_NCACTIVATE, uMsg == WM_SETFOCUS, 0);
			return 0;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
static void WXY_InitPixelFormat(PIXELFORMATDESCRIPTOR *pPFD) {
	memset(pPFD, 0, sizeof(*pPFD));

	pPFD->nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pPFD->nVersion = 1;
	pPFD->dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pPFD->iPixelType = PFD_TYPE_RGBA;
	pPFD->cColorBits = 24;
	pPFD->cDepthBits = 32;
	pPFD->iLayerType = PFD_MAIN_PLANE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void WXY_Print(void) {
	DOCINFO		di;

	PRINTDLG	pd;

	/* initialize the PRINTDLG struct and execute it */
	memset(&pd, 0, sizeof(pd));
	pd.lStructSize = sizeof(pd);
	pd.hwndOwner = g_pParentWnd->GetXYWnd()->GetSafeHwnd();
	pd.Flags = PD_RETURNDC;
	pd.hInstance = 0;
	if (!PrintDlg(&pd) || !pd.hDC) {
		g_pParentWnd->MessageBox("Could not PrintDlg()", "QE4 Print Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* StartDoc */
	memset(&di, 0, sizeof(di));
	di.cbSize = sizeof(di);
	di.lpszDocName = "QE4";
	if (StartDoc(pd.hDC, &di) <= 0) {
		g_pParentWnd->MessageBox("Could not StartDoc()", "QE4 Print Error", MB_OK | MB_ICONERROR);
		return;
	}

	/* StartPage */
	if (StartPage(pd.hDC) <= 0) {
		g_pParentWnd->MessageBox("Could not StartPage()", "QE4 Print Error", MB_OK | MB_ICONERROR);
		return;
	} { /* read pixels from the XY window */
		int		bmwidth = 320, bmheight = 320;
		int		pwidth, pheight;

		RECT	r;

		GetWindowRect(g_pParentWnd->GetXYWnd()->GetSafeHwnd(), &r);

		bmwidth = r.right - r.left;
		bmheight = r.bottom - r.top;

		pwidth = GetDeviceCaps(pd.hDC, PHYSICALWIDTH) - GetDeviceCaps(pd.hDC, PHYSICALOFFSETX);
		pheight = GetDeviceCaps(pd.hDC, PHYSICALHEIGHT) - GetDeviceCaps(pd.hDC, PHYSICALOFFSETY);

		StretchBlt(pd.hDC, 0, 0, pwidth, pheight, s_hdcXY, 0, 0, bmwidth, bmheight, SRCCOPY);
	}

	/* EndPage and EndDoc */
	if (EndPage(pd.hDC) <= 0) {
		g_pParentWnd->MessageBox("QE4 Print Error", "Could not EndPage()", MB_OK | MB_ICONERROR);
		return;
	}

	if (EndDoc(pd.hDC) <= 0) {
		g_pParentWnd->MessageBox("QE4 Print Error", "Could not EndDoc()", MB_OK | MB_ICONERROR);
		return;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int CXYWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	s_hdcXY = ::GetDC(GetSafeHwnd());
	QEW_SetupPixelFormat(s_hdcXY, false);

	qglPolygonStipple((unsigned char *)s_stipple);
	qglLineStipple(3, 0xaaaa);
	return 0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
float ptSum(idVec3 pt) {
	return pt[0] + pt[1] + pt[2];
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::DropClipPoint(UINT nFlags, CPoint point) {
	CRect	rctZ;
	GetClientRect(rctZ);
	if (g_pMovingClip) {
		SetCapture();
		SnapToPoint(point.x, rctZ.Height() - 1 - point.y, *g_pMovingClip);
	}
	else {
		idVec3	*pPt = NULL;
		if (g_Clip1.Set() == false) {
			pPt = g_Clip1;
			g_Clip1.Set(true);
			g_Clip1.m_ptScreen = point;
		}
		else if (g_Clip2.Set() == false) {
			pPt = g_Clip2;
			g_Clip2.Set(true);
			g_Clip2.m_ptScreen = point;
		}
		else if (g_Clip3.Set() == false) {
			pPt = g_Clip3;
			g_Clip3.Set(true);
			g_Clip3.m_ptScreen = point;
		}
		else {
			RetainClipMode(true);
			pPt = g_Clip1;
			g_Clip1.Set(true);
			g_Clip1.m_ptScreen = point;
		}

		SnapToPoint(point.x, rctZ.Height() - 1 - point.y, *pPt);

		// Put the off-viewaxis coordinate at the top or bottom of selected brushes
		if ( GetAsyncKeyState(VK_CONTROL) & 0x8000 ) {
			if ( selected_brushes.next != &selected_brushes ) {
				idVec3	smins, smaxs;
				Select_GetBounds( smins, smaxs );

				if ( m_nViewType == XY ) {
					if ( GetAsyncKeyState(VK_SHIFT) & 0x8000 ) {
						pPt->z = smaxs.z;
					} else {
						pPt->z = smins.z;
					}
				} else if ( m_nViewType == YZ ) {
					if ( GetAsyncKeyState(VK_SHIFT) & 0x8000 ) {
						pPt->x = smaxs.x;
					} else {
						pPt->x = smins.x;
					}
				} else {
					if ( GetAsyncKeyState(VK_SHIFT) & 0x8000 ) {
						pPt->y = smaxs.y;
					} else {
						pPt->y = smins.y;
					}
				}
			}
		}
	}

	Sys_UpdateWindows(XY | W_CAMERA_IFON);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::DropPathPoint(UINT nFlags, CPoint point) {
	CRect	rctZ;
	GetClientRect(rctZ);
	if (g_pMovingPath) {
		SetCapture();
		SnapToPoint(point.x, rctZ.Height() - 1 - point.y, *g_pMovingPath);
	}
	else {
		g_PathPoints[g_nPathCount].Set(true);
		g_PathPoints[g_nPathCount].m_ptScreen = point;
		SnapToPoint(point.x, rctZ.Height() - 1 - point.y, g_PathPoints[g_nPathCount]);
		g_nPathCount++;
		if (g_nPathCount == g_nPathLimit) {
			if (g_pPathFunc) {
				g_pPathFunc(true, g_nPathCount);
			}

			g_nPathCount = 0;
			g_bPathMode = false;
			g_pPathFunc = NULL;
		}
	}

	Sys_UpdateWindows(XY | W_CAMERA_IFON);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::AddPointPoint(UINT nFlags, idVec3 *pVec) {
	g_PointPoints[g_nPointCount].Set(true);

	// g_PointPoints[g_nPointCount].m_ptScreen = point;
	g_PointPoints[g_nPointCount].m_ptClip = *pVec;
	g_PointPoints[g_nPointCount].SetPointPtr(pVec);
	g_nPointCount++;
	Sys_UpdateWindows(XY | W_CAMERA_IFON);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnLButtonDown(UINT nFlags, CPoint point) {
	g_pParentWnd->SetActiveXY(this);
	UndoCopy();

	if (g_pParentWnd->GetNurbMode()) {
		int i, num = g_pParentWnd->GetNurb()->GetNumValues();
		idList<idVec2> temp;
			for (i = 0; i < num; i++) {
			temp.Append(g_pParentWnd->GetNurb()->GetValue(i));
		}
		CRect	rctZ;
		GetClientRect(rctZ);
		idVec3 v3;
		SnapToPoint(point.x, rctZ.Height() - 1 - point.y, v3);
		temp.Append(idVec2(v3.x, v3.y));
		num++;
		g_pParentWnd->GetNurb()->Clear();
		for (i = 0; i < num; i++) {
			g_pParentWnd->GetNurb()->AddValue((1000 * i)/num, temp[i]);
		}
	}
	if (ClipMode() && !RogueClipMode()) {
		DropClipPoint(nFlags, point);
	}
	else if (PathMode()) {
		DropPathPoint(nFlags, point);
	}
	else {
		OriginalButtonDown(nFlags, point);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnMButtonDown(UINT nFlags, CPoint point) {
	OriginalButtonDown(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
float Betwixt(float f1, float f2) {
	if (f1 > f2) {
		return f2 + ((f1 - f2) / 2);
	}
	else {
		return f1 + ((f2 - f1) / 2);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::ProduceSplits(brush_t **pFront, brush_t **pBack) {
	*pFront = NULL;
	*pBack = NULL;
	if (ClipMode()) {
		if (g_Clip1.Set() && g_Clip2.Set()) {
			face_t	face;
			VectorCopy(g_Clip1.m_ptClip, face.planepts[0]);
			VectorCopy(g_Clip2.m_ptClip, face.planepts[1]);
			VectorCopy(g_Clip3.m_ptClip, face.planepts[2]);
			if (selected_brushes.next && (selected_brushes.next->next == &selected_brushes)) {
				if (g_Clip3.Set() == false) {
					if (m_nViewType == XY) {
						face.planepts[0][2] = selected_brushes.next->mins[2];
						face.planepts[1][2] = selected_brushes.next->mins[2];
						face.planepts[2][0] = Betwixt(g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0]);
						face.planepts[2][1] = Betwixt(g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1]);
						face.planepts[2][2] = selected_brushes.next->maxs[2];
					}
					else if (m_nViewType == YZ) {
						face.planepts[0][0] = selected_brushes.next->mins[0];
						face.planepts[1][0] = selected_brushes.next->mins[0];
						face.planepts[2][1] = Betwixt(g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1]);
						face.planepts[2][2] = Betwixt(g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2]);
						face.planepts[2][0] = selected_brushes.next->maxs[0];
					}
					else {
						face.planepts[0][1] = selected_brushes.next->mins[1];
						face.planepts[1][1] = selected_brushes.next->mins[1];
						face.planepts[2][0] = Betwixt(g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0]);
						face.planepts[2][2] = Betwixt(g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2]);
						face.planepts[2][1] = selected_brushes.next->maxs[1];
					}
				}

				Brush_SplitBrushByFace(selected_brushes.next, &face, pFront, pBack);
			}
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CleanList(brush_t *pList) {
	brush_t *pBrush = pList->next;
	while (pBrush != NULL && pBrush != pList) {
		brush_t *pNext = pBrush->next;
		Brush_Free(pBrush);
		pBrush = pNext;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::ProduceSplitLists() {
	if (AnyPatchesSelected()) {
		Sys_Status("Deslecting patches for clip operation.\n");

		brush_t *next;
		for (brush_t * pb = selected_brushes.next; pb != &selected_brushes; pb = next) {
			next = pb->next;
			if (pb->pPatch) {
				Brush_RemoveFromList(pb);
				Brush_AddToList(pb, &active_brushes);
				UpdatePatchInspector();
			}
		}
	}

	CleanList(&g_brFrontSplits);
	CleanList(&g_brBackSplits);
	g_brFrontSplits.next = &g_brFrontSplits;
	g_brBackSplits.next = &g_brBackSplits;

	brush_t *pBrush;
	for (pBrush = selected_brushes.next; pBrush != NULL && pBrush != &selected_brushes; pBrush = pBrush->next) {
		brush_t *pFront = NULL;
		brush_t *pBack = NULL;
		if (ClipMode()) {
			if (g_Clip1.Set() && g_Clip2.Set()) {
				face_t	face;
				VectorCopy(g_Clip1.m_ptClip, face.planepts[0]);
				VectorCopy(g_Clip2.m_ptClip, face.planepts[1]);
				VectorCopy(g_Clip3.m_ptClip, face.planepts[2]);
				if (g_Clip3.Set() == false) {
					if (g_pParentWnd->ActiveXY()->GetViewType() == XY) {
						face.planepts[0][2] = pBrush->mins[2];
						face.planepts[1][2] = pBrush->mins[2];
						face.planepts[2][0] = Betwixt(g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0]);
						face.planepts[2][1] = Betwixt(g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1]);
						face.planepts[2][2] = pBrush->maxs[2];
					}
					else if (g_pParentWnd->ActiveXY()->GetViewType() == YZ) {
						face.planepts[0][0] = pBrush->mins[0];
						face.planepts[1][0] = pBrush->mins[0];
						face.planepts[2][1] = Betwixt(g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1]);
						face.planepts[2][2] = Betwixt(g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2]);
						face.planepts[2][0] = pBrush->maxs[0];
					}
					else {
						face.planepts[0][1] = pBrush->mins[1];
						face.planepts[1][1] = pBrush->mins[1];
						face.planepts[2][0] = Betwixt(g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0]);
						face.planepts[2][2] = Betwixt(g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2]);
						face.planepts[2][1] = pBrush->maxs[1];
					}
				}

				Brush_SplitBrushByFace(pBrush, &face, &pFront, &pBack);
				if (pBack) {
					Brush_AddToList(pBack, &g_brBackSplits);
				}

				if (pFront) {
					Brush_AddToList(pFront, &g_brFrontSplits);
				}
			}
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Brush_CopyList(brush_t *pFrom, brush_t *pTo) {
	brush_t *pBrush = pFrom->next;
	while (pBrush != NULL && pBrush != pFrom) {
		brush_t *pNext = pBrush->next;
		Brush_RemoveFromList(pBrush);
		Brush_AddToList(pBrush, pTo);
		pBrush = pNext;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnRButtonDown(UINT nFlags, CPoint point) {
	g_pParentWnd->SetActiveXY(this);
	m_ptDown = point;
	m_bRButtonDown = true;

	if (g_PrefsDlg.m_nMouseButtons == 3) {	// 3 button mouse
		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
			if (ClipMode()) {				// already there?
				DropClipPoint(nFlags, point);
			}
			else {
				SetClipMode(true);
				g_bRogueClipMode = true;
				DropClipPoint(nFlags, point);
			}

			return;
		}
	}

	OriginalButtonDown(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnLButtonUp(UINT nFlags, CPoint point) {

	if (ClipMode()) {
		if (g_pMovingClip) {
			ReleaseCapture();
			g_pMovingClip = NULL;
		}
	}

	OriginalButtonUp(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnMButtonUp(UINT nFlags, CPoint point) {
	OriginalButtonUp(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnRButtonUp(UINT nFlags, CPoint point) {
	m_bRButtonDown = false;
	if (point == m_ptDown) {	// mouse didn't move
		bool	bGo = true;
		if ((GetAsyncKeyState(VK_MENU) & 0x8000)) {
			bGo = false;
		}

		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
			bGo = false;
		}

		if ((GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
			bGo = false;
		}

		if (bGo) {
			HandleDrop();
		}
	}

	OriginalButtonUp(nFlags, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OriginalButtonDown(UINT nFlags, CPoint point) {
	CRect	rctZ;
	GetClientRect(rctZ);
	SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	if (g_pParentWnd->GetTopWindow() != this) {
		BringWindowToTop();
	}

	SetFocus();
	SetCapture();
	XY_MouseDown(point.x, rctZ.Height() - 1 - point.y, nFlags);
	m_nScrollFlags = nFlags;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OriginalButtonUp(UINT nFlags, CPoint point) {
	CRect	rctZ;
	GetClientRect(rctZ);
	XY_MouseUp(point.x, rctZ.Height() - 1 - point.y, nFlags);
	if (!(nFlags & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON))) {
		ReleaseCapture();
	}
}

idVec3	tdp;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnMouseMove(UINT nFlags, CPoint point) {

	m_ptDown.x = 0;
	m_ptDown.y = 0;

	if
	(
		g_PrefsDlg.m_bChaseMouse == TRUE &&
		(point.x < 0 || point.y < 0 || point.x > m_nWidth || point.y > m_nHeight) &&
		GetCapture() == this
	) {
		float	fAdjustment = (g_qeglobals.d_gridsize / 8 * 64) / m_fScale;

		// m_ptDrag = point;
		m_ptDragAdj.x = 0;
		m_ptDragAdj.y = 0;
		if (point.x < 0) {
			m_ptDragAdj.x = -fAdjustment;
		}
		else if (point.x > m_nWidth) {
			m_ptDragAdj.x = fAdjustment;
		}

		if (point.y < 0) {
			m_ptDragAdj.y = -fAdjustment;
		}
		else if (point.y > m_nHeight) {
			m_ptDragAdj.y = fAdjustment;
		}

		if (m_nTimerID == -1) {
			m_nTimerID = SetTimer(100, 50, NULL);
			m_ptDrag = point;
			m_ptDragTotal = 0;
		}

		return;
	}

	// else if (m_nTimerID != -1)
	if (m_nTimerID != -1) {
		KillTimer(m_nTimerID);
		pressx -= m_ptDragTotal.x;
		pressy += m_ptDragTotal.y;
		m_nTimerID = -1;

		// return;
	}

	bool	bCrossHair = false;
	if (!m_bRButtonDown) {
		tdp[0] = tdp[1] = tdp[2] = 0.0;
		SnapToPoint(point.x, m_nHeight - 1 - point.y, tdp);

		g_strStatus.Format("x:: %.1f  y:: %.1f  z:: %.1f", tdp[0], tdp[1], tdp[2]);
		g_pParentWnd->SetStatusText(1, g_strStatus);

		//
		// i need to generalize the point code.. having 3 flavors pretty much sucks.. once
		// the new curve stuff looks like it is going to stick i will rationalize this
		// down to a single interface..
		//
		if (PointMode()) {
			if (g_pMovingPoint && GetCapture() == this) {
				bCrossHair = true;
				SnapToPoint(point.x, m_nHeight - 1 - point.y, g_pMovingPoint->m_ptClip);
				g_pMovingPoint->UpdatePointPtr();
				Sys_UpdateWindows(XY | W_CAMERA_IFON);
			}
			else {
				g_pMovingPoint = NULL;

				int nDim1 = (m_nViewType == YZ) ? 1 : 0;
				int nDim2 = (m_nViewType == XY) ? 1 : 2;
				for (int n = 0; n < g_nPointCount; n++) {
					if
					(
						fDiff(g_PointPoints[n].m_ptClip[nDim1], tdp[nDim1]) < 3 &&
						fDiff(g_PointPoints[n].m_ptClip[nDim2], tdp[nDim2]) < 3
					) {
						bCrossHair = true;
						g_pMovingPoint = &g_PointPoints[n];
					}
				}
			}
		}
		else if (ClipMode()) {
			if (g_pMovingClip && GetCapture() == this) {
				bCrossHair = true;
				SnapToPoint(point.x, m_nHeight - 1 - point.y, g_pMovingClip->m_ptClip);
				Sys_UpdateWindows(XY | W_CAMERA_IFON);
			}
			else {
				g_pMovingClip = NULL;

				int nDim1 = (m_nViewType == YZ) ? 1 : 0;
				int nDim2 = (m_nViewType == XY) ? 1 : 2;
				if (g_Clip1.Set()) {
					if
					(
						fDiff(g_Clip1.m_ptClip[nDim1], tdp[nDim1]) < 3 &&
						fDiff(g_Clip1.m_ptClip[nDim2], tdp[nDim2]) < 3
					) {
						bCrossHair = true;
						g_pMovingClip = &g_Clip1;
					}
				}

				if (g_Clip2.Set()) {
					if
					(
						fDiff(g_Clip2.m_ptClip[nDim1], tdp[nDim1]) < 3 &&
						fDiff(g_Clip2.m_ptClip[nDim2], tdp[nDim2]) < 3
					) {
						bCrossHair = true;
						g_pMovingClip = &g_Clip2;
					}
				}

				if (g_Clip3.Set()) {
					if
					(
						fDiff(g_Clip3.m_ptClip[nDim1], tdp[nDim1]) < 3 &&
						fDiff(g_Clip3.m_ptClip[nDim2], tdp[nDim2]) < 3
					) {
						bCrossHair = true;
						g_pMovingClip = &g_Clip3;
					}
				}
			}

			if (bCrossHair == false) {
				XY_MouseMoved(point.x, m_nHeight - 1 - point.y, nFlags);
			}
		}
		else if (PathMode()) {
			if (g_pMovingPath && GetCapture() == this) {
				bCrossHair = true;
				SnapToPoint(point.x, m_nHeight - 1 - point.y, g_pMovingPath->m_ptClip);
				Sys_UpdateWindows(XY | W_CAMERA_IFON);
			}
			else {
				g_pMovingPath = NULL;

				int nDim1 = (m_nViewType == YZ) ? 1 : 0;
				int nDim2 = (m_nViewType == XY) ? 1 : 2;
				for (int n = 0; n < g_nPathCount; n++) {
					if
					(
						fDiff(g_PathPoints[n].m_ptClip[nDim1], tdp[nDim1]) < 3 &&
						fDiff(g_PathPoints[n].m_ptClip[nDim2], tdp[nDim2]) < 3
					) {
						bCrossHair = true;
						g_pMovingPath = &g_PathPoints[n];
					}
				}
			}
		}
		else {
			bCrossHair = XY_MouseMoved(point.x, m_nHeight - 1 - point.y, nFlags);
		}
	}
	else {
		bCrossHair = XY_MouseMoved(point.x, m_nHeight - 1 - point.y, nFlags);
	}

	if (bCrossHair) {
		SetCursor(::LoadCursor(NULL, IDC_CROSS));
	}
	else {
		SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}

	/// If precision crosshair is active, force redraw of the 2d view on mouse move 
	if( m_precisionCrosshairMode != PRECISION_CROSSHAIR_NONE )
	{
		/// Force 2d view redraw (so that the precision cursor moves with the mouse)
		Sys_UpdateWindows( W_XY );
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::RetainClipMode(bool bMode) {
	bool	bSave = g_bRogueClipMode;
	SetClipMode(bMode);
	if (bMode == true) {
		g_bRogueClipMode = bSave;
	}
	else {
		g_bRogueClipMode = false;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::SetClipMode(bool bMode) {
	g_bClipMode = bMode;
	g_bRogueClipMode = false;
	if (bMode) {
		g_Clip1.Reset();
		g_Clip2.Reset();
		g_Clip3.Reset();
		CleanList(&g_brFrontSplits);
		CleanList(&g_brBackSplits);
		g_brFrontSplits.next = &g_brFrontSplits;
		g_brBackSplits.next = &g_brBackSplits;
	}
	else {
		if (g_pMovingClip) {
			ReleaseCapture();
			g_pMovingClip = NULL;
		}

		CleanList(&g_brFrontSplits);
		CleanList(&g_brBackSplits);
		g_brFrontSplits.next = &g_brFrontSplits;
		g_brBackSplits.next = &g_brBackSplits;
		Sys_UpdateWindows(XY | W_CAMERA_IFON);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CXYWnd::ClipMode() {
	return g_bClipMode;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CXYWnd::RogueClipMode() {
	return g_bRogueClipMode;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CXYWnd::PathMode() {
	return g_bPathMode;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CXYWnd::PointMode() {
	return g_bPointMode;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::SetPointMode(bool b) {
	g_bPointMode = b;
	if (!b) {
		g_nPointCount = 0;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnPaint() {
	CPaintDC	dc(this);					// device context for painting
	bool		bPaint = true;
	if (!qwglMakeCurrent(dc.m_hDC, win32.hGLRC)) {
		common->Printf("ERROR: wglMakeCurrent failed.. Error:%i\n", qglGetError());
		common->Printf("Please restart Q3Radiant if the Map view is not working\n");
		bPaint = false;
	}

	if (bPaint) {
		QE_CheckOpenGLForErrors();
		XY_Draw();
		QE_CheckOpenGLForErrors();

		if (m_nViewType != XY) {
			qglPushMatrix();
			if (m_nViewType == YZ) {
				qglRotatef(-90, 0, 1, 0);	// put Z going up
			}

			qglRotatef(-90, 1, 0, 0);		// put Z going up
		}

		if ( g_bCrossHairs ) {
			qglColor4f( 0.2f, 0.9f, 0.2f, 0.8f );
			qglBegin(GL_LINES);
			if (m_nViewType == XY) {
				qglVertex2f(-16384, tdp[1]);
				qglVertex2f(16384, tdp[1]);
				qglVertex2f(tdp[0], -16384);
				qglVertex2f(tdp[0], 16384);
			}
			else if (m_nViewType == YZ) {
				qglVertex3f(tdp[0], -16384, tdp[2]);
				qglVertex3f(tdp[0], 16384, tdp[2]);
				qglVertex3f(tdp[0], tdp[1], -16384);
				qglVertex3f(tdp[0], tdp[1], 16384);
			}
			else {
				qglVertex3f(-16384, tdp[1], tdp[2]);
				qglVertex3f(16384, tdp[1], tdp[2]);
				qglVertex3f(tdp[0], tdp[1], -16384);
				qglVertex3f(tdp[0], tdp[1], 16384);
			}

			qglEnd();
		}

		if (ClipMode()) {
			qglPointSize(4);
			qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER].ToFloatPtr());
			qglBegin(GL_POINTS);
			if (g_Clip1.Set()) {
				qglVertex3fv(g_Clip1);
			}

			if (g_Clip2.Set()) {
				qglVertex3fv(g_Clip2);
			}

			if (g_Clip3.Set()) {
				qglVertex3fv(g_Clip3);
			}

			qglEnd();
			qglPointSize(1);

			CString strMsg;
			if (g_Clip1.Set()) {
				qglRasterPos3f(g_Clip1.m_ptClip[0] + 2, g_Clip1.m_ptClip[1] + 2, g_Clip1.m_ptClip[2] + 2);
				strMsg = "1";

				// strMsg.Format("1 (%f, %f, %f)", g_Clip1[0], g_Clip1[1], g_Clip1[2]);
				qglCallLists(strMsg.GetLength(), GL_UNSIGNED_BYTE, strMsg);
			}

			if (g_Clip2.Set()) {
				qglRasterPos3f(g_Clip2.m_ptClip[0] + 2, g_Clip2.m_ptClip[1] + 2, g_Clip2.m_ptClip[2] + 2);
				strMsg = "2";

				// strMsg.Format("2 (%f, %f, %f)", g_Clip2[0], g_Clip2[1], g_Clip2[2]);
				qglCallLists(strMsg.GetLength(), GL_UNSIGNED_BYTE, strMsg);
			}

			if (g_Clip3.Set()) {
				qglRasterPos3f(g_Clip3.m_ptClip[0] + 2, g_Clip3.m_ptClip[1] + 2, g_Clip3.m_ptClip[2] + 2);
				strMsg = "3";

				// strMsg.Format("3 (%f, %f, %f)", g_Clip3[0], g_Clip3[1], g_Clip3[2]);
				qglCallLists(strMsg.GetLength(), GL_UNSIGNED_BYTE, strMsg);
			}

			if (g_Clip1.Set() && g_Clip2.Set() && selected_brushes.next != &selected_brushes) {
				ProduceSplitLists();

				brush_t *pBrush;
				brush_t *pList = ((m_nViewType == XZ) ? !g_bSwitch : g_bSwitch) ? &g_brBackSplits : &g_brFrontSplits;
				for (pBrush = pList->next; pBrush != NULL && pBrush != pList; pBrush = pBrush->next) {
					qglColor3f(1, 1, 0);

					face_t	*face;
					int		order;
					for (face = pBrush->brush_faces, order = 0; face; face = face->next, order++) {
						idWinding *w = face->face_winding;
						if (!w) {
							continue;
						}

						// draw the polygon
						qglBegin(GL_LINE_LOOP);
						for (int i = 0; i < w->GetNumPoints(); i++) {
							qglVertex3fv( (*w)[i].ToFloatPtr() );
						}

						qglEnd();
					}
				}
			}
		}

		if (PathMode()) {
			qglPointSize(4);
			qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER].ToFloatPtr());
			qglBegin(GL_POINTS);

			int n;
			for ( n = 0; n < g_nPathCount; n++) {
				qglVertex3fv(g_PathPoints[n]);
			}

			qglEnd();
			qglPointSize(1);

			CString strMsg;
			for (n = 0; n < g_nPathCount; n++) {
				qglRasterPos3f
				(
					g_PathPoints[n].m_ptClip[0] + 2,
					g_PathPoints[n].m_ptClip[1] + 2,
					g_PathPoints[n].m_ptClip[2] + 2
				);
				strMsg.Format("%i", n + 1);
				qglCallLists(strMsg.GetLength(), GL_UNSIGNED_BYTE, strMsg);
			}
		}

		if (m_nViewType != XY) {
			qglPopMatrix();
		}

		qwglSwapBuffers(dc.m_hDC);
		TRACE("XY Paint\n");
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags);
}

//
// =======================================================================================================================
//    FIXME: the brush_t *pBrush is never used. ( Entity_Create uses selected_brushes )
// =======================================================================================================================
//
void CreateEntityFromName(char *pName, brush_t *pBrush, bool forceFixed, idVec3 min, idVec3 max, idVec3 org) {
	eclass_t	*pecNew;
	entity_t	*petNew;
	if (stricmp(pName, "worldspawn") == 0) {
		g_pParentWnd->MessageBox("Can't create an entity with worldspawn.", "info", 0);
		return;
	}

	pecNew = Eclass_ForName(pName, false);

	if ((GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
		Select_Ungroup();
	}

	// create it
	petNew = Entity_Create(pecNew, forceFixed);

	if (petNew && idStr::Icmp(pName, "light") == 0 ) {
		idVec3	rad = max - min;
		rad *= 0.5;
		if (rad.x != 0 && rad.y != 0 && rad.z != 0) {
			SetKeyValue(petNew, "light_radius", va("%g %g %g", idMath::Fabs(rad.x), idMath::Fabs(rad.y), idMath::Fabs(rad.z)));
			DeleteKey(petNew, "light");
		}
	}


	if (petNew == NULL) {
		if (!((selected_brushes.next == &selected_brushes) || (selected_brushes.next->next != &selected_brushes))) {
			brush_t *b = selected_brushes.next;
			if (b->owner != world_entity && ((b->owner->eclass->fixedsize && pecNew->fixedsize) || forceFixed)) {
				idVec3	mins, maxs;
				idVec3	origin;
				for (int i = 0; i < 3; i++) {
					origin[i] = b->mins[i] - pecNew->mins[i];
				}

				VectorAdd(pecNew->mins, origin, mins);
				VectorAdd(pecNew->maxs, origin, maxs);

				brush_t *nb = Brush_Create(mins, maxs, &pecNew->texdef);
				Entity_LinkBrush(b->owner, nb);
				nb->owner->eclass = pecNew;
				SetKeyValue(nb->owner, "classname", pName);
				Brush_Free(b);
				Brush_Build(nb);
				Brush_AddToList(nb, &active_brushes);
				Select_Brush(nb);
				return;
			}
		}

		g_pParentWnd->MessageBox("Failed to create entity.", "info", 0);
		return;
	}

	Select_Deselect();

	//
	// entity_t* pEntity = world_entity; if (selected_brushes.next !=
	// &selected_brushes) pEntity = selected_brushes.next->owner;
	//
	Select_Brush(petNew->brushes.onext);
	Brush_Build(petNew->brushes.onext);

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
brush_t *CreateEntityBrush(int x, int y, CXYWnd *pWnd) {
	idVec3	mins, maxs;
	int		i;
	float	temp;
	brush_t *n;

	pWnd->SnapToPoint(x, y, mins);
	x += 32;
	y += 32;
	pWnd->SnapToPoint(x, y, maxs);

	int nDim = (pWnd->GetViewType() == XY) ? 2 : (pWnd->GetViewType() == YZ) ? 0 : 1;
	mins[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_bottom[nDim] / g_qeglobals.d_gridsize));
	maxs[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_top[nDim] / g_qeglobals.d_gridsize));

	if (maxs[nDim] <= mins[nDim]) {
		maxs[nDim] = mins[nDim] + g_qeglobals.d_gridsize;
	}

	for (i = 0; i < 3; i++) {
		if (mins[i] == maxs[i]) {
			maxs[i] += 16;	// don't create a degenerate brush
		}

		if (mins[i] > maxs[i]) {
			temp = mins[i];
			mins[i] = maxs[i];
			maxs[i] = temp;
		}
	}

	n = Brush_Create(mins, maxs, &g_qeglobals.d_texturewin.texdef);
	if (!n) {
		return NULL;
	}

	Brush_AddToList(n, &selected_brushes);
	Entity_LinkBrush(world_entity, n);
	Brush_Build(n);
	return n;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CreateRightClickEntity(CXYWnd *pWnd, int x, int y, char *pName) {
	idVec3	min, max, org;
	Select_GetBounds(min, max);
	Select_GetMid(org);

	CRect	rctZ;
	pWnd->GetClientRect(rctZ);

	brush_t *pBrush;
	if (selected_brushes.next == &selected_brushes) {
		pBrush = CreateEntityBrush(x, rctZ.Height() - 1 - y, pWnd);
		min.Zero();
		max.Zero();
		CreateEntityFromName(pName, pBrush, true, min, max, org);
	}
	else {
		pBrush = selected_brushes.next;
		CreateEntityFromName(pName, pBrush, false, min, max, org);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
brush_t *CreateSmartBrush(idVec3 v) {
	idVec3	mins, maxs;
	int		i;
	brush_t *n;

	for (i = 0; i < 3; i++) {
		mins[i] = v[i] - 16;
		maxs[i] = v[i] + 16;
	}

	n = Brush_Create(mins, maxs, &g_qeglobals.d_texturewin.texdef);
	if (!n) {
		return NULL;
	}

	Brush_AddToList(n, &selected_brushes);

	// Entity_LinkBrush(world_entity, n);
	Brush_Build(n);
	return n;
}

CString g_strSmartEntity;
int		g_nSmartX;
int		g_nSmartY;
bool	g_bSmartWaiting;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void _SmartPointDone(bool b, int n) {
	g_bSmartWaiting = false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CreateSmartEntity(CXYWnd *pWnd, int x, int y, const char *pName) {
	g_nSmartX = x;
	g_nSmartY = y;
	g_strSmartEntity = pName;
	if (g_strSmartEntity.Find("Smart_Train") >= 0) {
		ShowInfoDialog("Select the path of the train by left clicking in XY, YZ and/or XZ views. You can move an already dropped point by grabbing and moving it. When you are finished, press ENTER to accept and create the entity and path(s), press ESC to abandon the creation");
		g_bPathMode = true;
		g_nPathLimit = 0;
		g_nPathCount = 0;
		g_bSmartGo = true;
	}
	else if (g_strSmartEntity.Find("Smart_Monster...") >= 0) {
		g_bPathMode = true;
		g_nPathLimit = 0;
		g_nPathCount = 0;
	}
	else if (g_strSmartEntity.Find("Smart_Rotating") >= 0) {
		g_bSmartWaiting = true;
		ShowInfoDialog("Left click to specify the rotation origin");
		AcquirePath(1, &_SmartPointDone);
		while (g_bSmartWaiting) {
			MSG msg;
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		HideInfoDialog();

		CPtrArray	array;
		g_bScreenUpdates = false;
		CreateRightClickEntity(g_pParentWnd->ActiveXY(), g_nSmartX, g_nSmartY, "func_rotating");
		array.Add(reinterpret_cast < void * > (selected_brushes.next));
		Select_Deselect();

		brush_t *pBrush = CreateSmartBrush(g_PathPoints[0]);
		array.Add(pBrush);
		Select_Deselect();
		Select_Brush(reinterpret_cast < brush_t * > (array.GetAt(0)));
		Select_Brush(reinterpret_cast < brush_t * > (array.GetAt(1)));
		ConnectEntities();
		g_bScreenUpdates = true;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void FinishSmartCreation() {
	CPtrArray	array;
	HideInfoDialog();

	brush_t *pEntities = NULL;
	if (g_strSmartEntity.Find("Smart_Train") >= 0) {
		g_bScreenUpdates = false;
		CreateRightClickEntity(g_pParentWnd->ActiveXY(), g_nSmartX, g_nSmartY, "func_train");
		array.Add(reinterpret_cast < void * > (selected_brushes.next));
		int n;
		for (n = 0; n < g_nPathCount; n++) {
			Select_Deselect();
			CreateRightClickEntity
			(
				g_pParentWnd->ActiveXY(),
				g_PathPoints[n].m_ptScreen.x,
				g_PathPoints[n].m_ptScreen.y,
				"path_corner"
			);
			array.Add(reinterpret_cast < void * > (selected_brushes.next));
		}

		for (n = 0; n < g_nPathCount; n++) {
			Select_Deselect();
			Select_Brush(reinterpret_cast < brush_t * > (array.GetAt(n)));
			Select_Brush(reinterpret_cast < brush_t * > (array.GetAt(n + 1)));
			ConnectEntities();
		}

		g_bScreenUpdates = true;
	}

	g_nPathCount = 0;
	g_bPathMode = false;
	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::KillPathMode() {
	g_bSmartGo = false;
	g_bPathMode = false;
	if (g_pPathFunc) {
		g_pPathFunc(false, g_nPathCount);
	}

	g_nPathCount = 0;
	g_pPathFunc = NULL;
	Sys_UpdateWindows(W_ALL);
}

//
// =======================================================================================================================
//    gets called for drop down menu messages TIP: it's not always about EntityCreate
// =======================================================================================================================
//
void CXYWnd::OnEntityCreate(unsigned int nID) {
	if (m_mnuDrop.GetSafeHmenu()) {
		CString strItem;
		m_mnuDrop.GetMenuString(nID, strItem, MF_BYCOMMAND);

		if (strItem.CompareNoCase("Add to...") == 0) {
			//
			// ++timo TODO: fill the menu with current groups? this one is for adding to
			// existing groups only
			//
			common->Printf("TODO: Add to... in CXYWnd::OnEntityCreate\n");
		}
		else if (strItem.CompareNoCase("Remove") == 0) {
			// remove selected brushes from their current group
			brush_t *b;
			for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
			}
		}

		// ++timo FIXME: remove when all hooks are in
		if
		(
			strItem.CompareNoCase("Add to...") == 0 ||
			strItem.CompareNoCase("Remove") == 0 ||
			strItem.CompareNoCase("Name...") == 0 ||
			strItem.CompareNoCase("New group...") == 0
		) {
			common->Printf("TODO: hook drop down group menu\n");
			return;
		}

		if (strItem.Find("Smart_") >= 0) {
			CreateSmartEntity(this, m_ptDown.x, m_ptDown.y, strItem);
		}
		else {
			CreateRightClickEntity(this, m_ptDown.x, m_ptDown.y, strItem.GetBuffer(0));
		}

		Sys_UpdateWindows(W_ALL);

		// OnLButtonDown((MK_LBUTTON | MK_SHIFT), CPoint(m_ptDown.x+2, m_ptDown.y+2));
	}
}

BOOL CXYWnd::OnCmdMsg( UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo )
{
	if ( CWnd::OnCmdMsg( nID, nCode, pExtra, pHandlerInfo ) ) {
		return TRUE;
	}
	return AfxGetMainWnd()->OnCmdMsg( nID, nCode, pExtra, pHandlerInfo );
}

bool MergeMenu(CMenu * pMenuDestination, const CMenu * pMenuAdd, bool bTopLevel /*=false*/)
{
	// get the number menu items in the menus
	int iMenuAddItemCount	= pMenuAdd->GetMenuItemCount();
	int iMenuDestItemCount	= pMenuDestination->GetMenuItemCount();

	// if there are no items return
	if (iMenuAddItemCount == 0)
		return true;

	// if we are not at top level and the destination menu is not empty
	// -> we append a seperator
	if (!bTopLevel && iMenuDestItemCount > 0)
		pMenuDestination->AppendMenu(MF_SEPARATOR);

	// iterate through the top level of <pMenuAdd>
	for(int iLoop = 0; iLoop < iMenuAddItemCount; iLoop++)
	{
		// get the menu string from the add menu
		CString sMenuAddString;
		pMenuAdd->GetMenuString(iLoop, sMenuAddString, MF_BYPOSITION);

		// try to get the submenu of the current menu item
		CMenu* pSubMenu = pMenuAdd->GetSubMenu(iLoop);

		// check if we have a sub menu
		if (!pSubMenu)
		{
			// normal menu item
			// read the source and append at the destination
			UINT nState	 = pMenuAdd->GetMenuState(iLoop, MF_BYPOSITION);
			UINT nItemID = pMenuAdd->GetMenuItemID(iLoop);
			if (pMenuDestination->AppendMenu(nState, nItemID, sMenuAddString))
			{
				// menu item added, don't forget to correct the item count
				iMenuDestItemCount++;
			}
			else
			{
				TRACE("MergeMenu: AppendMenu failed!\n");
				return false;
			}
		}
		else
		{
			// create or insert a new popup menu item

			// default insert pos is like ap
			int iInsertPosDefault = -1;

			// if we are at top level merge into existing popups rather than
			// creating new ones
			if(bTopLevel)
			{
				ASSERT(sMenuAddString != "&?" && sMenuAddString !=
					"?");
				CString csAdd(sMenuAddString);
				csAdd.Remove('&');	// for comparison of menu items supress '&'
				bool bAdded = false;

				// try to find existing popup
				for( int iLoop1 = 0; iLoop1 < iMenuDestItemCount; iLoop1++ )
				{
					// get the menu string from the destination menu
					CString sDest;
					pMenuDestination->GetMenuString(iLoop1, sDest, MF_BYPOSITION);
					sDest.Remove('&'); // for a better compare (s.a.)

					if (csAdd == sDest)
					{
						// we got a hit -> merge the two popups
						// try to get the submenu of the desired destination menu item
							CMenu* pSubMenuDest =
							pMenuDestination->GetSubMenu(iLoop1);

						if (pSubMenuDest)
						{
							// merge the popup recursivly and continue with outer for loop
								if (!MergeMenu(pSubMenuDest, pSubMenu, false))
									return false;
							bAdded = true;
							break;
						}
					}

					// alternativ insert before <Window> or <Help>
					if (iInsertPosDefault == -1 && (sDest == "Window"
						|| sDest == "?" || sDest == "Help"))
					{
						iInsertPosDefault = iLoop1;
					}
				} // for (iLoop1)
				if (bAdded)
				{
					// menu added, so go on with loop over pMenuAdd's top level
					continue;
				}
			} // if (bTopLevel)

			// if the top level search did not find a position append the menu
			if( iInsertPosDefault == -1 )
			{
				iInsertPosDefault = pMenuDestination->GetMenuItemCount();
			}

			// create a new popup and insert before <Window> or <Help>
			CMenu NewPopupMenu;
			if (!NewPopupMenu.CreatePopupMenu())
			{
				TRACE("MergeMenu: CreatePopupMenu failed!\n");
				return false;
			}

			// merge the new popup recursivly
			if (!MergeMenu(&NewPopupMenu, pSubMenu, false))
				return false;

			// insert the new popup menu into the destination menu
			HMENU hNewMenu = NewPopupMenu.GetSafeHmenu();
			if (pMenuDestination->InsertMenu(iInsertPosDefault,
				MF_BYPOSITION | MF_POPUP | MF_ENABLED, 
				(UINT)hNewMenu, sMenuAddString ))
			{
				// don't forget to correct the item count
				iMenuDestItemCount++;
			}
			else
			{
				TRACE("MergeMenu: InsertMenu failed!\n");
				return false;
			}

			// don't destroy the new menu		
			NewPopupMenu.Detach();
		} // if (pSubMenu)
	} // for (iLoop)
	return true;
}




/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::HandleDrop() {
	if (g_PrefsDlg.m_bRightClick == false) {
		return;
	}

	if (!m_mnuDrop.GetSafeHmenu()) {		// first time, load it up
		m_mnuDrop.CreatePopupMenu();
		
		CMenu *drop = new CMenu;
		drop->LoadMenu( IDR_MENU_DROP );

		MergeMenu( &m_mnuDrop, drop, false );

		int		nID = ID_ENTITY_START;

		CMenu	*pMakeEntityPop = &m_mnuDrop;

		// Todo: Make this a config option maybe?
		const int entitiesOnSubMenu = false;
		if ( entitiesOnSubMenu ) {
			pMakeEntityPop = new CMenu;
			pMakeEntityPop->CreateMenu();
		}

		CMenu	*pChild = NULL;

		eclass_t	*e;
		CString		strActive;
		CString		strLast;
		CString		strName;
		for (e = eclass; e; e = e->next) {
			strLast = strName;
			strName = e->name;

			int n_ = strName.Find("_");
			if (n_ > 0) {
				CString strLeft = strName.Left(n_);
				CString strRight = strName.Right(strName.GetLength() - n_ - 1);
				if (strLeft == strActive) { // this is a child
					ASSERT(pChild);
					pChild->AppendMenu(MF_STRING, nID++, strName);
				}
				else {
					if (pChild) {
						pMakeEntityPop->AppendMenu (
							MF_POPUP,
							reinterpret_cast < unsigned int > (pChild->GetSafeHmenu()),
							strActive
						);
						g_ptrMenus.Add(pChild);

						// pChild->DestroyMenu(); delete pChild;
						pChild = NULL;
					}

					strActive = strLeft;
					pChild = new CMenu;
					pChild->CreateMenu();
					pChild->AppendMenu(MF_STRING, nID++, strName);
				}
			}
			else {
				if (pChild) {
					pMakeEntityPop->AppendMenu (
						MF_POPUP,
						reinterpret_cast < unsigned int > (pChild->GetSafeHmenu()),
						strActive
					);
					g_ptrMenus.Add(pChild);

					// pChild->DestroyMenu(); delete pChild;
					pChild = NULL;
				}

				strActive = "";
				pMakeEntityPop->AppendMenu(MF_STRING, nID++, strName);
			}
		}
		if ( pMakeEntityPop != &m_mnuDrop ) {
			m_mnuDrop.AppendMenu (
				MF_POPUP,
				reinterpret_cast < unsigned int > (pMakeEntityPop->GetSafeHmenu()),
				"Make Entity"
			);
		}
	}

	CPoint	ptMouse;
	GetCursorPos(&ptMouse);
	m_mnuDrop.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::XY_Init() {
	m_vOrigin[0] = 0;
	m_vOrigin[1] = 20;
	m_vOrigin[2] = 46;
	m_fScale = 1;
	m_precisionCrosshairMode = PRECISION_CROSSHAIR_NONE;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::SnapToPoint(int x, int y, idVec3 &point) {
	if (g_PrefsDlg.m_bNoClamp) {
		XY_ToPoint(x, y, point);
	}
	else {
		XY_ToGridPoint(x, y, point);
	}

	// -- else -- XY_ToPoint(x, y, point); -- //XY_ToPoint(x, y, point);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::XY_ToPoint(int x, int y, idVec3 &point) {
	float	fx = x;
	float	fy = y;
	float	fw = m_nWidth;
	float	fh = m_nHeight;
	if (m_nViewType == XY) {
		point[0] = m_vOrigin[0] + (fx - fw / 2) / m_fScale;
		point[1] = m_vOrigin[1] + (fy - fh / 2) / m_fScale;

		// point[2] = 0;
	}
	else if (m_nViewType == YZ) {
		//
		// //point[0] = 0; point[1] = m_vOrigin[0] + (fx - fw / 2) / m_fScale; point[2] =
		// m_vOrigin[1] + (fy - fh / 2 ) / m_fScale;
		//
		point[1] = m_vOrigin[1] + (fx - fw / 2) / m_fScale;
		point[2] = m_vOrigin[2] + (fy - fh / 2) / m_fScale;
	}
	else {
		//
		// point[0] = m_vOrigin[0] + (fx - fw / 2) / m_fScale; /point[1] = 0; point[2] =
		// m_vOrigin[1] + (fy - fh / 2) / m_fScale;
		//
		point[0] = m_vOrigin[0] + (fx - fw / 2) / m_fScale;

		// point[1] = 0;
		point[2] = m_vOrigin[2] + (fy - fh / 2) / m_fScale;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::XY_ToGridPoint(int x, int y, idVec3 &point) {
	if (m_nViewType == XY) {
		point[0] = m_vOrigin[0] + (x - m_nWidth / 2) / m_fScale;
		point[1] = m_vOrigin[1] + (y - m_nHeight / 2) / m_fScale;

		// point[2] = 0;
		point[0] = floor(point[0] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
		point[1] = floor(point[1] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
	}
	else if (m_nViewType == YZ) {
		//
		// point[0] = 0; point[1] = m_vOrigin[0] + (x - m_nWidth / 2) / m_fScale; point[2]
		// = m_vOrigin[1] + (y - m_nHeight / 2) / m_fScale;
		//
		point[1] = m_vOrigin[1] + (x - m_nWidth / 2) / m_fScale;
		point[2] = m_vOrigin[2] + (y - m_nHeight / 2) / m_fScale;
		point[1] = floor(point[1] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
		point[2] = floor(point[2] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
	}
	else {
		//
		// point[1] = 0; point[0] = m_vOrigin[0] + (x - m_nWidth / 2) / m_fScale; point[2]
		// = m_vOrigin[1] + (y - m_nHeight / 2) / m_fScale;
		//
		point[0] = m_vOrigin[0] + (x - m_nWidth / 2) / m_fScale;
		point[2] = m_vOrigin[2] + (y - m_nHeight / 2) / m_fScale;
		point[0] = floor(point[0] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
		point[2] = floor(point[2] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
idVec3 dragOrigin;
idVec3 dragDir;
idVec3 dragX;
idVec3 dragY;

void CXYWnd::XY_MouseDown(int x, int y, int buttons) {
	idVec3	point,center;
	idVec3	origin, dir, right, up;

	m_nButtonstate = buttons;
	m_nPressx = x;
	m_nPressy = y;
	VectorCopy(vec3_origin, m_vPressdelta);

	point.Zero();

	XY_ToPoint(x, y, point);

	VectorCopy(point, origin);

	dir.Zero();
	if (m_nViewType == XY) {
		origin[2] = HUGE_DISTANCE;
		dir[2] = -1;
		right[0] = 1 / m_fScale;
		right[1] = 0;
		right[2] = 0;
		up[0] = 0;
		up[1] = 1 / m_fScale;
		up[2] = 0;
		point[2] = g_pParentWnd->GetCamera()->Camera().origin[2];
	}
	else if (m_nViewType == YZ) {
		origin[0] = HUGE_DISTANCE;
		dir[0] = -1;
		right[1] = 1 / m_fScale;
		right[2] = 0;
		right[0] = 0;
		up[0] = 0;
		up[2] = 1 / m_fScale;
		up[1] = 0;
		point[0] = g_pParentWnd->GetCamera()->Camera().origin[0];
	}
	else {
		origin[1] = HUGE_DISTANCE;
		dir[1] = -1;
		right[0] = 1 / m_fScale;
		right[2] = 0;
		right[1] = 0;
		up[0] = 0;
		up[2] = 1 / m_fScale;
		up[1] = 0;
		point[1] = g_pParentWnd->GetCamera()->Camera().origin[1];
	}

	dragOrigin = m_vOrigin;
	dragDir = dir;
	dragX = right;
	dragY = up;

	m_bPress_selection = (selected_brushes.next != &selected_brushes);

	GetCursorPos(&m_ptCursor);

	// Sys_GetCursorPos (&m_ptCursor.x, &m_ptCursor.y);
	if (buttons == MK_LBUTTON && activeDrag) {
		activeDragging = true;
	}
	else {
		activeDragging = false;
	}

	// lbutton = manipulate selection shift-LBUTTON = select
	if
	(
		(buttons == MK_LBUTTON) ||
		(buttons == (MK_LBUTTON | MK_SHIFT)) ||
		(buttons == (MK_LBUTTON | MK_CONTROL)) ||
		(buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT))
	) {
		if (g_qeglobals.d_select_mode == sel_addpoint) {
			XY_ToGridPoint(x, y, point);
			if (g_qeglobals.selectObject) {
				g_qeglobals.selectObject->addPoint(point);
			}

			return;
		}

		Patch_SetView((m_nViewType == XY) ? W_XY : (m_nViewType == YZ) ? W_YZ : W_XZ);
		Drag_Begin(x, y, buttons, right, up, origin, dir);
		return;
	}

	int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;

	// control mbutton = move camera
	if (m_nButtonstate == (MK_CONTROL | nMouseButton)) {
		VectorCopyXY(point, g_pParentWnd->GetCamera()->Camera().origin);
		Sys_UpdateWindows(W_CAMERA | W_XY_OVERLAY);
	}

	// mbutton = angle camera
	if
	(
		(g_PrefsDlg.m_nMouseButtons == 3 && m_nButtonstate == MK_MBUTTON) ||
		(g_PrefsDlg.m_nMouseButtons == 2 && m_nButtonstate == (MK_SHIFT | MK_CONTROL | MK_RBUTTON))
	) {
		VectorSubtract(point, g_pParentWnd->GetCamera()->Camera().origin, point);

		int n1 = (m_nViewType == XY) ? 1 : 2;
		int n2 = (m_nViewType == YZ) ? 1 : 0;
		int nAngle = (m_nViewType == XY) ? YAW : PITCH;
		if (point[n1] || point[n2]) {
			g_pParentWnd->GetCamera()->Camera().angles[nAngle] = RAD2DEG( atan2(point[n1], point[n2]) );
			Sys_UpdateWindows(W_CAMERA_IFON | W_XY_OVERLAY);
		}
	}

	// shift mbutton = move z checker
	if (m_nButtonstate == (MK_SHIFT | nMouseButton)) {
		if (RotateMode() || g_bPatchBendMode) {
			SnapToPoint(x, y, point);
			VectorCopyXY(point, g_vRotateOrigin);
			if (g_bPatchBendMode) {
				VectorCopy(point, g_vBendOrigin);
			}

			Sys_UpdateWindows(W_XY);
			return;
		}
		else {
			SnapToPoint(x, y, point);
			if (m_nViewType == XY) {
				z.origin[0] = point[0];
				z.origin[1] = point[1];
			}
			else if (m_nViewType == YZ) {
				z.origin[0] = point[1];
				z.origin[1] = point[2];
			}
			else {
				z.origin[0] = point[0];
				z.origin[1] = point[2];
			}

			Sys_UpdateWindows(W_XY_OVERLAY | W_Z);
			return;
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::XY_MouseUp(int x, int y, int buttons) {
	activeDragging = false;
	Drag_MouseUp(buttons);
	if (!m_bPress_selection) {
		Sys_UpdateWindows(W_ALL);
	}

	m_nButtonstate = 0;
	while (::ShowCursor(TRUE) < 0)
		;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CXYWnd::DragDelta(int x, int y, idVec3 &move) {
	idVec3	xvec, yvec, delta;
	int		i;

	xvec[0] = 1 / m_fScale;
	xvec[1] = xvec[2] = 0;
	yvec[1] = 1 / m_fScale;
	yvec[0] = yvec[2] = 0;

	for (i = 0; i < 3; i++) {
		delta[i] = xvec[i] * (x - m_nPressx) + yvec[i] * (y - m_nPressy);
		if (!g_PrefsDlg.m_bNoClamp) {
			delta[i] = floor(delta[i] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
		}
	}

	VectorSubtract(delta, m_vPressdelta, move);
	VectorCopy(delta, m_vPressdelta);


	if (move[0] || move[1] || move[2]) {
		return true;
	}

	return false;
}

/*
 =======================================================================================================================
    NewBrushDrag
 =======================================================================================================================
 */
void CXYWnd::NewBrushDrag(int x, int y) {
	idVec3	mins, maxs, junk;
	int		i;
	float	temp;
	brush_t *n;

	if ( radiant_entityMode.GetBool() ) {
		return;
	}

	if (!DragDelta(x, y, junk)) {
		return;
	}

	// delete the current selection
	if (selected_brushes.next != &selected_brushes) {
		Brush_Free(selected_brushes.next);
	}

	SnapToPoint(m_nPressx, m_nPressy, mins);

	int nDim = (m_nViewType == XY) ? 2 : (m_nViewType == YZ) ? 0 : 1;

	mins[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_bottom[nDim] / g_qeglobals.d_gridsize));
	SnapToPoint(x, y, maxs);
	maxs[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_top[nDim] / g_qeglobals.d_gridsize));
	if (maxs[nDim] <= mins[nDim]) {
		maxs[nDim] = mins[nDim] + g_qeglobals.d_gridsize;
	}

	for (i = 0; i < 3; i++) {
		if (mins[i] == maxs[i]) {
			return; // don't create a degenerate brush
		}

		if (mins[i] > maxs[i]) {
			temp = mins[i];
			mins[i] = maxs[i];
			maxs[i] = temp;
		}
	}

	n = Brush_Create(mins, maxs, &g_qeglobals.d_texturewin.texdef);
	if (!n) {
		return;
	}

	idVec3	vSize;
	VectorSubtract(maxs, mins, vSize);
	g_strStatus.Format("Size X:: %.1f  Y:: %.1f  Z:: %.1f", vSize[0], vSize[1], vSize[2]);
	g_pParentWnd->SetStatusText(2, g_strStatus);

	Brush_AddToList(n, &selected_brushes);

	Entity_LinkBrush(world_entity, n);

	Brush_Build(n);

	// Sys_UpdateWindows (W_ALL);
	Sys_UpdateWindows(W_XY | W_CAMERA);
}

/*
 =======================================================================================================================
    XY_MouseMoved
 =======================================================================================================================
 */
bool CXYWnd::XY_MouseMoved(int x, int y, int buttons) {
	idVec3	point;

	if (!m_nButtonstate) {
		if (g_bCrossHairs) {
			::ShowCursor(FALSE);
			Sys_UpdateWindows(W_XY | W_XY_OVERLAY);
			::ShowCursor(TRUE);
		}

		return false;
	}

	//
	// lbutton without selection = drag new brush if (m_nButtonstate == MK_LBUTTON &&
	// !m_bPress_selection && g_qeglobals.d_select_mode != sel_curvepoint &&
	// g_qeglobals.d_select_mode != sel_splineedit)
	//
	if (m_nButtonstate == MK_LBUTTON && !m_bPress_selection && g_qeglobals.d_select_mode == sel_brush) {
		NewBrushDrag(x, y);
		return false;
	}

	// lbutton (possibly with control and or shift) with selection = drag selection
	if (m_nButtonstate & MK_LBUTTON) {
		Drag_MouseMoved(x, y, buttons);
		Sys_UpdateWindows(W_XY_OVERLAY | W_CAMERA_IFON | W_Z);
		return false;
	}

	int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;

	// control mbutton = move camera
	if (m_nButtonstate == (MK_CONTROL | nMouseButton)) {
		SnapToPoint(x, y, point);
		VectorCopyXY(point, g_pParentWnd->GetCamera()->Camera().origin);
		Sys_UpdateWindows(W_XY_OVERLAY | W_CAMERA);
		return false;
	}

	// shift mbutton = move z checker
	if (m_nButtonstate == (MK_SHIFT | nMouseButton)) {
		if (RotateMode() || g_bPatchBendMode) {
			SnapToPoint(x, y, point);
			VectorCopyXY(point, g_vRotateOrigin);
			if (g_bPatchBendMode) {
				VectorCopy(point, g_vBendOrigin);
			}

			Sys_UpdateWindows(W_XY);
			return false;
		}
		else {
			SnapToPoint(x, y, point);
			if (m_nViewType == XY) {
				z.origin[0] = point[0];
				z.origin[1] = point[1];
			}
			else if (m_nViewType == YZ) {
				z.origin[0] = point[1];
				z.origin[1] = point[2];
			}
			else {
				z.origin[0] = point[0];
				z.origin[1] = point[2];
			}
		}

		Sys_UpdateWindows(W_XY_OVERLAY | W_Z);
		return false;
	}

	// mbutton = angle camera
	if
	(
		(g_PrefsDlg.m_nMouseButtons == 3 && m_nButtonstate == MK_MBUTTON) ||
		(g_PrefsDlg.m_nMouseButtons == 2 && m_nButtonstate == (MK_SHIFT | MK_CONTROL | MK_RBUTTON))
	) {
		SnapToPoint(x, y, point);
		VectorSubtract(point, g_pParentWnd->GetCamera()->Camera().origin, point);

		int n1 = (m_nViewType == XY) ? 1 : 2;
		int n2 = (m_nViewType == YZ) ? 1 : 0;
		int nAngle = (m_nViewType == XY) ? YAW : PITCH;
		if (point[n1] || point[n2]) {
			g_pParentWnd->GetCamera()->Camera().angles[nAngle] = RAD2DEG( atan2(point[n1], point[n2]) );
			Sys_UpdateWindows(W_CAMERA_IFON | W_XY_OVERLAY);
		}

		return false;
	}

	// rbutton = drag xy origin
	if (m_nButtonstate == MK_RBUTTON) {
		Sys_GetCursorPos(&x, &y);

		if (x != m_ptCursor.x || y != m_ptCursor.y) {
			if ((GetAsyncKeyState(VK_MENU) & 0x8000)) {
				int		*px = &x;
				long	*px2 = &m_ptCursor.x;

				if (fDiff(y, m_ptCursor.y) > fDiff(x, m_ptCursor.x)) {
					px = &y;
					px2 = &m_ptCursor.y;
				}

				if (*px > *px2) {
					// zoom in
					SetScale( Scale() * 1.1f );
					if ( Scale() < 0.1f ) {
						SetScale( 0.1f );
					}
				}
				else if (*px < *px2) {
					// zoom out
					SetScale( Scale() * 0.9f );
					if ( Scale() > 16.0f ) {
						SetScale( 16.0f );
					}
				}

				*px2 = *px;
				Sys_UpdateWindows(W_XY | W_XY_OVERLAY);
			}
			else {
				int nDim1 = (m_nViewType == YZ) ? 1 : 0;
				int nDim2 = (m_nViewType == XY) ? 1 : 2;
				m_vOrigin[nDim1] -= (x - m_ptCursor.x) / m_fScale;
				m_vOrigin[nDim2] += (y - m_ptCursor.y) / m_fScale;
				SetCursorPos(m_ptCursor.x, m_ptCursor.y);
				::ShowCursor(FALSE);

				// XY_Draw(); RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				Sys_UpdateWindows(W_XY | W_XY_OVERLAY);

				// ::ShowCursor(TRUE);
			}
		}

		return false;
	}

	return false;
}

/*
 =======================================================================================================================
    DRAWING £
    XY_DrawGrid
 =======================================================================================================================
 */
void CXYWnd::XY_DrawGrid() {
	float	x, y, xb, xe, yb, ye;
	int		w, h;
	char	text[32];

	int startPos = max ( 64 , g_qeglobals.d_gridsize );

	w = m_nWidth / 2 / m_fScale;
	h = m_nHeight / 2 / m_fScale;

	int nDim1 = (m_nViewType == YZ) ? 1 : 0;
	int nDim2 = (m_nViewType == XY) ? 1 : 2;

	// int nDim1 = 0; int nDim2 = 1;
	xb = m_vOrigin[nDim1] - w;
	if (xb < region_mins[nDim1]) {
		xb = region_mins[nDim1];
	}

	xb = startPos * floor(xb / startPos);

	xe = m_vOrigin[nDim1] + w;
	if (xe > region_maxs[nDim1]) {
		xe = region_maxs[nDim1];
	}

	xe = startPos * ceil(xe / startPos);

	yb = m_vOrigin[nDim2] - h;
	if (yb < region_mins[nDim2]) {
		yb = region_mins[nDim2];
	}

	yb = startPos * floor(yb / startPos);

	ye = m_vOrigin[nDim2] + h;
	if (ye > region_maxs[nDim2]) {
		ye = region_maxs[nDim2];
	}

	ye = startPos * ceil(ye / startPos);

	// draw major blocks
	qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR].ToFloatPtr());

	int stepSize = 64 * 0.1 / m_fScale;
	if (stepSize < 64) {
		stepSize = max ( 64 , g_qeglobals.d_gridsize ); 
	}
	else {
		int i;
		for (i = 1; i < stepSize; i <<= 1) {
		}

		stepSize = i;
	}

	if (g_qeglobals.d_showgrid) {
		qglBegin(GL_LINES);

		for (x = xb; x <= xe; x += stepSize) {
			qglVertex2f(x, yb);
			qglVertex2f(x, ye);
		}

		for (y = yb; y <= ye; y += stepSize) {
			qglVertex2f(xb, y);
			qglVertex2f(xe, y);
		}

		qglEnd();
	}

	// draw minor blocks
	if ( m_fScale > .1 &&
		g_qeglobals.d_showgrid &&
		g_qeglobals.d_gridsize * m_fScale >= 4 &&
		!g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR].Compare( g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK] ) ) {

		qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR].ToFloatPtr());

		qglBegin(GL_LINES);
		for (x = xb; x < xe; x += g_qeglobals.d_gridsize) {
			if (!((int)x & (startPos - 1))) {
				continue;
			}

			qglVertex2f(x, yb);
			qglVertex2f(x, ye);
		}

		for (y = yb; y < ye; y += g_qeglobals.d_gridsize) {
			if (!((int)y & (startPos - 1))) {
				continue;
			}

			qglVertex2f(xb, y);
			qglVertex2f(xe, y);
		}

		qglEnd();
	}


	// draw ZClip boundaries (if applicable)...
	//
	if (m_nViewType == XZ || m_nViewType == YZ)
	{
		if (g_pParentWnd->GetZWnd()->m_pZClip)	// should always be the case at this point I think, but this is safer
		{
			if (g_pParentWnd->GetZWnd()->m_pZClip->IsEnabled())
			{
				qglColor3f(ZCLIP_COLOUR);
				qglLineWidth(2);
				qglBegin (GL_LINES);

				qglVertex2f (xb, g_pParentWnd->GetZWnd()->m_pZClip->GetTop());
				qglVertex2f (xe, g_pParentWnd->GetZWnd()->m_pZClip->GetTop());

				qglVertex2f (xb, g_pParentWnd->GetZWnd()->m_pZClip->GetBottom());
				qglVertex2f (xe, g_pParentWnd->GetZWnd()->m_pZClip->GetBottom());

				qglEnd ();
				qglLineWidth(1);
			}
		}		
	}

	


	// draw coordinate text if needed
	if (g_qeglobals.d_savedinfo.show_coordinates) {
		// glColor4f(0, 0, 0, 0);
		qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT].ToFloatPtr());

		float	lastRaster = xb;

		for (x = xb; x < xe; x += stepSize) {
			qglRasterPos2f(x, m_vOrigin[nDim2] + h - 10 / m_fScale);
			sprintf(text, "%i", (int)x);
			qglCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
		}

		for (y = yb; y < ye; y += stepSize) {
			qglRasterPos2f(m_vOrigin[nDim1] - w + 1, y);
			sprintf(text, "%i", (int)y);
			qglCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
		}

		if (Active()) {
			qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME].ToFloatPtr());
		}

		qglRasterPos2f(m_vOrigin[nDim1] - w + 35 / m_fScale, m_vOrigin[nDim2] + h - 20 / m_fScale);

		char	cView[20];
		if (m_nViewType == XY) {
			strcpy(cView, "XY Top");
		}
		else if (m_nViewType == XZ) {
			strcpy(cView, "XZ Front");
		}
		else {
			strcpy(cView, "YZ Side");
		}

		qglCallLists(strlen(cView), GL_UNSIGNED_BYTE, cView);
	}

	/*
	 * if (true) { qglColor3f(g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR]);
	 * qglBegin (GL_LINES); qglVertex2f (x, yb); qglVertex2f (x, ye); qglEnd(); }
	 */
}

/*
 =======================================================================================================================
    XY_DrawBlockGrid
 =======================================================================================================================
 */
void CXYWnd::XY_DrawBlockGrid() {
	float	x, y, xb, xe, yb, ye;
	int		w, h;
	char	text[32];

	w = m_nWidth / 2 / m_fScale;
	h = m_nHeight / 2 / m_fScale;

	int nDim1 = (m_nViewType == YZ) ? 1 : 0;
	int nDim2 = (m_nViewType == XY) ? 1 : 2;

	xb = m_vOrigin[nDim1] - w;
	if (xb < region_mins[nDim1]) {
		xb = region_mins[nDim1];
	}

	xb = 1024 * floor(xb / 1024);

	xe = m_vOrigin[nDim1] + w;
	if (xe > region_maxs[nDim1]) {
		xe = region_maxs[nDim1];
	}

	xe = 1024 * ceil(xe / 1024);

	yb = m_vOrigin[nDim2] - h;
	if (yb < region_mins[nDim2]) {
		yb = region_mins[nDim2];
	}

	yb = 1024 * floor(yb / 1024);

	ye = m_vOrigin[nDim2] + h;
	if (ye > region_maxs[nDim2]) {
		ye = region_maxs[nDim2];
	}

	ye = 1024 * ceil(ye / 1024);

	// draw major blocks
	qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK].ToFloatPtr());
	qglLineWidth(0.5);

	qglBegin(GL_LINES);

	for (x = xb; x <= xe; x += 1024) {
		qglVertex2f(x, yb);
		qglVertex2f(x, ye);
	}

	for (y = yb; y <= ye; y += 1024) {
		qglVertex2f(xb, y);
		qglVertex2f(xe, y);
	}

	qglEnd();
	qglLineWidth(0.25);

	// draw coordinate text if needed
	for (x = xb; x < xe; x += 1024) {
		for (y = yb; y < ye; y += 1024) {
			qglRasterPos2f(x + 512, y + 512);
			sprintf(text, "%i,%i", (int)floor(x / 1024), (int)floor(y / 1024));
			qglCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
		}
	}

	qglColor4f(0, 0, 0, 0);
}

void GLColoredBoxWithLabel(float x, float y, float size, idVec4 color, const char *text, idVec4 textColor, float xofs, float yofs, float lineSize) {
	globalImages->BindNull();	
	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	qglDisable(GL_CULL_FACE);
	qglDisable(GL_BLEND);
	qglColor4f(color[0], color[1], color[2], color[3]);
	qglBegin(GL_QUADS);
	qglVertex3f(x - size, y - size, 0);
	qglVertex3f(x + size, y - size, 0);
	qglVertex3f(x + size, y + size, 0);
	qglVertex3f(x - size, y + size, 0);
	qglEnd();

	qglColor4f(textColor[0], textColor[1], textColor[2], textColor[3]);
	qglLineWidth(lineSize);
	qglRasterPos2f(x + xofs, y + yofs);
	qglCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::DrawRotateIcon() {
	float	x, y;

	if (m_nViewType == XY) {
		x = g_vRotateOrigin[0];
		y = g_vRotateOrigin[1];
	}
	else if (m_nViewType == YZ) {
		x = g_vRotateOrigin[1];
		y = g_vRotateOrigin[2];
	}
	else {
		x = g_vRotateOrigin[0];
		y = g_vRotateOrigin[2];
	}

	qglEnable(GL_BLEND);
	globalImages->BindNull();
	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	qglDisable(GL_CULL_FACE);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglColor4f( 0.8f, 0.1f, 0.9f, 0.25f );

	qglBegin(GL_QUADS);
	qglVertex3f(x - 4, y - 4, 0);
	qglVertex3f(x + 4, y - 4, 0);
	qglVertex3f(x + 4, y + 4, 0);
	qglVertex3f(x - 4, y + 4, 0);
	qglEnd();
	qglDisable(GL_BLEND);

	qglColor4f( 1.0f, 0.2f, 1.0f, 1.0f );
	qglBegin(GL_POINTS);
	qglVertex3f(x, y, 0);
	qglEnd();


	int w = m_nWidth / 2 / m_fScale;
	int h = m_nHeight / 2 / m_fScale;
	int nDim1 = (m_nViewType == YZ) ? 1 : 0;
	int nDim2 = (m_nViewType == XY) ? 1 : 2;
	x = m_vOrigin[nDim1] - w + 35 / m_fScale;
	y = m_vOrigin[nDim2] + h - 40 / m_fScale;
	const char *p = "Rotate Z Axis";
	if (g_qeglobals.rotateAxis == 1) {
		p = "Rotate Y Axis";
	} else if (g_qeglobals.rotateAxis == 0) {
		p = "Rotate X Axis";
	}
	idStr str = p;
	if (g_qeglobals.flatRotation) {
		str += g_qeglobals.flatRotation == 2 ? " Flat [center] " : " Flat [ rot origin ] ";
	}
	qglRasterPos2f(x, y);
	qglCallLists(str.Length(), GL_UNSIGNED_BYTE, str.c_str());
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::DrawCameraIcon() {
	float	x, y, a;

	if (m_nViewType == XY) {
		x = g_pParentWnd->GetCamera()->Camera().origin[0];
		y = g_pParentWnd->GetCamera()->Camera().origin[1];
		a = g_pParentWnd->GetCamera()->Camera().angles[YAW] * idMath::M_DEG2RAD;
	}
	else if (m_nViewType == YZ) {
		x = g_pParentWnd->GetCamera()->Camera().origin[1];
		y = g_pParentWnd->GetCamera()->Camera().origin[2];
		a = g_pParentWnd->GetCamera()->Camera().angles[PITCH] * idMath::M_DEG2RAD;
	}
	else {
		x = g_pParentWnd->GetCamera()->Camera().origin[0];
		y = g_pParentWnd->GetCamera()->Camera().origin[2];
		a = g_pParentWnd->GetCamera()->Camera().angles[PITCH] * idMath::M_DEG2RAD;
	}

	float scale = 1.0/m_fScale;	//jhefty - keep the camera icon proportionally the same size 

	qglColor3f(0.0, 0.0, 1.0);
	qglBegin(GL_LINE_STRIP);
	qglVertex3f(x - 16*scale, y, 0);
	qglVertex3f(x, y + 8*scale, 0);
	qglVertex3f(x + 16*scale, y, 0);
	qglVertex3f(x, y - 8*scale, 0);
	qglVertex3f(x - 16*scale, y, 0);
	qglVertex3f(x + 16*scale, y, 0);
	qglEnd();

	qglBegin(GL_LINE_STRIP);
	qglVertex3f(x + (48 * cos( a + idMath::PI * 0.25f )*scale), y + (48 * sin( a + idMath::PI * 0.25f )*scale), 0);
	qglVertex3f(x, y, 0);
	qglVertex3f(x + (48 * cos( a - idMath::PI * 0.25f )*scale), y + (48 * sin( a - idMath::PI * 0.25f )*scale), 0);
	qglEnd();

#if 0

	char	text[128];
	qglRasterPos2f(x + 64, y + 64);
	sprintf(text, "%f", g_pParentWnd->GetCamera()->Camera().angles[YAW]);
	qglCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
#endif
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::DrawZIcon(void) {
	if (m_nViewType == XY) {
		float	x = z.origin[0];
		float	y = z.origin[1];
		qglEnable(GL_BLEND);
		globalImages->BindNull();
		qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		qglDisable(GL_CULL_FACE);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4f(0.0, 0.0, 1.0, 0.25);
		qglBegin(GL_QUADS);
		qglVertex3f(x - 8, y - 8, 0);
		qglVertex3f(x + 8, y - 8, 0);
		qglVertex3f(x + 8, y + 8, 0);
		qglVertex3f(x - 8, y + 8, 0);
		qglEnd();
		qglDisable(GL_BLEND);

		qglColor4f(0.0, 0.0, 1.0, 1);

		qglBegin(GL_LINE_LOOP);
		qglVertex3f(x - 8, y - 8, 0);
		qglVertex3f(x + 8, y - 8, 0);
		qglVertex3f(x + 8, y + 8, 0);
		qglVertex3f(x - 8, y + 8, 0);
		qglEnd();

		qglBegin(GL_LINE_STRIP);
		qglVertex3f(x - 4, y + 4, 0);
		qglVertex3f(x + 4, y + 4, 0);
		qglVertex3f(x - 4, y - 4, 0);
		qglVertex3f(x + 4, y - 4, 0);
		qglEnd();
	}
}

/*
 =======================================================================================================================
    FilterBrush
 =======================================================================================================================
 */
bool FilterBrush(brush_t *pb) {
	
	if (!pb->owner) {
		return false;	// during construction
	}

	if (pb->hiddenBrush) {
		return true;
	}

	if ( pb->forceVisibile ) {
		return false;
	}

	if (g_pParentWnd->GetZWnd()->m_pZClip)	// ZClip class up and running? (and hence Z window built)
	{
		if (g_pParentWnd->GetZWnd()->m_pZClip->IsEnabled())
		{
			// ZClipping active...
			//
			if (pb->mins[2] > g_pParentWnd->GetZWnd()->m_pZClip->GetTop()	// brush bottom edge is above clip top
				||
				pb->maxs[2] < g_pParentWnd->GetZWnd()->m_pZClip->GetBottom()// brush top edge is below clip bottom 
				)
			{
				return TRUE;
			}
		}
	}

	if (g_qeglobals.d_savedinfo.exclude & (EXCLUDE_CAULK | EXCLUDE_VISPORTALS)) {
		//
		// filter out the brush only if all faces are caulk if not don't hide the whole
		// brush, proceed on a per-face basis (Cam_Draw) ++timo TODO: set this as a
		// preference .. show caulk: hide any brush with caulk // don't draw caulk faces
		//
		face_t	*f;
		f = pb->brush_faces;
		while (f) {
			if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_CAULK) {
				if (!strstr(f->texdef.name, "caulk")) {
					break;
				}
			} else {
				if (strstr(f->texdef.name, "visportal")) {
					return true;
				}
			}

			f = f->next;
		}

		if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_CAULK) {
			if (!f) {
				return true;
			}
		}

		// ++timo FIXME: .. same deal here?
		if (strstr(pb->brush_faces->texdef.name, "donotenter")) {
			return true;
		}
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_HINT) {
		if (strstr(pb->brush_faces->texdef.name, "hint")) {
			return true;
		}
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLIP) {
		if (strstr(pb->brush_faces->texdef.name, "clip")) {
			return true;
		}
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_TRIGGERS) {
		if (strstr(pb->brush_faces->texdef.name, "trig")) {
			return true;
		}
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_NODRAW) {
		if (strstr(pb->brush_faces->texdef.name, "nodraw")) {
			return true;
		}
	}


	if (strstr(pb->brush_faces->texdef.name, "skip")) {
		return true;
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_DYNAMICS) {
		if (pb->modelHandle > 0) {
			idRenderModel *model = pb->modelHandle;
			if ( dynamic_cast<idRenderModelLiquid*>(model) ) {
				return true;			
			}
		}
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_CURVES) {
		if (pb->pPatch) {
			return true;
		}
	}

	if (pb->owner == world_entity) {
		if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_WORLD) {
			return true;
		}

		return false;
	}
	else {
		if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_ENT ) {
			return ( idStr::Cmpn( pb->owner->eclass->name, "func_static", 10 ) != 0 );
		}
	}

	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_LIGHTS && pb->owner->eclass->nShowFlags & ECLASS_LIGHT ) {
		return true;
	}

	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_COMBATNODES && pb->owner->eclass->nShowFlags & ECLASS_COMBATNODE ) {
		return true;
	}

	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS && pb->owner->eclass->nShowFlags & ECLASS_PATH) {
		return true;
	}

	if ( g_qeglobals.d_savedinfo.exclude & EXCLUDE_MODELS && ( pb->owner->eclass->entityModel != NULL || pb->modelHandle > 0 ) ) {
		return true;
	}

	return false;
}

/*
 =======================================================================================================================
    PATH LINES £
    DrawPathLines Draws connections between entities. Needs to consider all entities, not just ones on screen, because
    the lines can be visible when neither end is. Called for both camera view and xy view.
 =======================================================================================================================
 */
void DrawPathLines(void) {
	int			i, k;
	idVec3		mid, mid1;
	entity_t	*se, *te;
	brush_t		*sb, *tb;
	const char		*psz;
	idVec3		dir, s1, s2;
	float		len, f;
	int			arrows;
	int			num_entities;
	const char		*ent_target[MAX_MAP_ENTITIES];
	entity_t	*ent_entity[MAX_MAP_ENTITIES];

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS) {
		return;
	}

	num_entities = 0;
	for (te = entities.next; te != &entities && num_entities != MAX_MAP_ENTITIES; te = te->next) {
		for (int i = 0; i < 2048; i++) {
			if (i == 0) {
				ent_target[num_entities] = ValueForKey(te, "target");
			} else {
				ent_target[num_entities] = ValueForKey(te, va("target%i", i));
			}
			if (ent_target[num_entities][0]) {
				ent_entity[num_entities] = te;
				num_entities++;
			} else if (i > 16) {
				break;
			}
		}
	}

	for (se = entities.next; se != &entities; se = se->next) {
		psz = ValueForKey(se, "name");

		if (psz == NULL || psz[0] == '\0') {
			continue;
		}

		sb = se->brushes.onext;
		if (sb == &se->brushes) {
			continue;
		}

		for (k = 0; k < num_entities; k++) {
			if (strcmp(ent_target[k], psz)) {
				continue;
			}

			te = ent_entity[k];
			tb = te->brushes.onext;
			if (tb == &te->brushes) {
				continue;
			}

			mid = sb->owner->origin;
			mid1 = tb->owner->origin;

			VectorSubtract(mid1, mid, dir);
			len = dir.Normalize();
			s1[0] = -dir[1] * 8 + dir[0] * 8;
			s2[0] = dir[1] * 8 + dir[0] * 8;
			s1[1] = dir[0] * 8 + dir[1] * 8;
			s2[1] = -dir[0] * 8 + dir[1] * 8;

			qglColor3f(se->eclass->color[0], se->eclass->color[1], se->eclass->color[2]);

			qglBegin(GL_LINES);
			qglVertex3fv(mid.ToFloatPtr());
			qglVertex3fv(mid1.ToFloatPtr());

			arrows = (int)(len / 256) + 1;

			for (i = 0; i < arrows; i++) {
				f = len * (i + 0.5) / arrows;

				mid1 = mid + (f * dir);

				qglVertex3fv(mid1.ToFloatPtr());
				qglVertex3f(mid1[0] + s1[0], mid1[1] + s1[1], mid1[2]);
				qglVertex3fv(mid1.ToFloatPtr());
				qglVertex3f(mid1[0] + s2[0], mid1[1] + s2[1], mid1[2]);
			}

			qglEnd();
		}
	}

	return;
}

//
// =======================================================================================================================
//    can be greatly simplified but per usual i am in a hurry which is not an excuse, just a fact
// =======================================================================================================================
//
void CXYWnd::PaintSizeInfo(int nDim1, int nDim2, idVec3 vMinBounds, idVec3 vMaxBounds) {
	idVec3	vSize;
	VectorSubtract(vMaxBounds, vMinBounds, vSize);

	qglColor3f
	(
		g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0] * .65,
		g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1] * .65,
		g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2] * .65
	);

	if (m_nViewType == XY) {
		qglBegin(GL_LINES);

		qglVertex3f(vMinBounds[nDim1], vMinBounds[nDim2] - 6.0f / m_fScale, 0.0f);
		qglVertex3f(vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale, 0.0f);

		qglVertex3f(vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale, 0.0f);
		qglVertex3f(vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale, 0.0f);

		qglVertex3f(vMaxBounds[nDim1], vMinBounds[nDim2] - 6.0f / m_fScale, 0.0f);
		qglVertex3f(vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale, 0.0f);

		qglVertex3f(vMaxBounds[nDim1] + 6.0f / m_fScale, vMinBounds[nDim2], 0.0f);
		qglVertex3f(vMaxBounds[nDim1] + 10.0f / m_fScale, vMinBounds[nDim2], 0.0f);

		qglVertex3f(vMaxBounds[nDim1] + 10.0f / m_fScale, vMinBounds[nDim2], 0.0f);
		qglVertex3f(vMaxBounds[nDim1] + 10.0f / m_fScale, vMaxBounds[nDim2], 0.0f);

		qglVertex3f(vMaxBounds[nDim1] + 6.0f / m_fScale, vMaxBounds[nDim2], 0.0f);
		qglVertex3f(vMaxBounds[nDim1] + 10.0f / m_fScale, vMaxBounds[nDim2], 0.0f);

		qglEnd();

		qglRasterPos3f(Betwixt(vMinBounds[nDim1], vMaxBounds[nDim1]), vMinBounds[nDim2] - 20.0 / m_fScale, 0.0f);
		g_strDim.Format(g_pDimStrings[nDim1], vSize[nDim1]);
		qglCallLists(g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

		qglRasterPos3f(vMaxBounds[nDim1] + 16.0 / m_fScale, Betwixt(vMinBounds[nDim2], vMaxBounds[nDim2]), 0.0f);
		g_strDim.Format(g_pDimStrings[nDim2], vSize[nDim2]);
		qglCallLists(g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

		qglRasterPos3f(vMinBounds[nDim1] + 4, vMaxBounds[nDim2] + 8 / m_fScale, 0.0f);
		g_strDim.Format(g_pOrgStrings[0], vMinBounds[nDim1], vMaxBounds[nDim2]);
		qglCallLists(g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);
	}
	else if (m_nViewType == XZ) {
		qglBegin(GL_LINES);

		qglVertex3f(vMinBounds[nDim1], 0, vMinBounds[nDim2] - 6.0f / m_fScale);
		qglVertex3f(vMinBounds[nDim1], 0, vMinBounds[nDim2] - 10.0f / m_fScale);

		qglVertex3f(vMinBounds[nDim1], 0, vMinBounds[nDim2] - 10.0f / m_fScale);
		qglVertex3f(vMaxBounds[nDim1], 0, vMinBounds[nDim2] - 10.0f / m_fScale);

		qglVertex3f(vMaxBounds[nDim1], 0, vMinBounds[nDim2] - 6.0f / m_fScale);
		qglVertex3f(vMaxBounds[nDim1], 0, vMinBounds[nDim2] - 10.0f / m_fScale);

		qglVertex3f(vMaxBounds[nDim1] + 6.0f / m_fScale, 0, vMinBounds[nDim2]);
		qglVertex3f(vMaxBounds[nDim1] + 10.0f / m_fScale, 0, vMinBounds[nDim2]);

		qglVertex3f(vMaxBounds[nDim1] + 10.0f / m_fScale, 0, vMinBounds[nDim2]);
		qglVertex3f(vMaxBounds[nDim1] + 10.0f / m_fScale, 0, vMaxBounds[nDim2]);

		qglVertex3f(vMaxBounds[nDim1] + 6.0f / m_fScale, 0, vMaxBounds[nDim2]);
		qglVertex3f(vMaxBounds[nDim1] + 10.0f / m_fScale, 0, vMaxBounds[nDim2]);

		qglEnd();

		qglRasterPos3f(Betwixt(vMinBounds[nDim1], vMaxBounds[nDim1]), 0, vMinBounds[nDim2] - 20.0 / m_fScale);
		g_strDim.Format(g_pDimStrings[nDim1], vSize[nDim1]);
		qglCallLists(g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

		qglRasterPos3f(vMaxBounds[nDim1] + 16.0 / m_fScale, 0, Betwixt(vMinBounds[nDim2], vMaxBounds[nDim2]));
		g_strDim.Format(g_pDimStrings[nDim2], vSize[nDim2]);
		qglCallLists(g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

		qglRasterPos3f(vMinBounds[nDim1] + 4, 0, vMaxBounds[nDim2] + 8 / m_fScale);
		g_strDim.Format(g_pOrgStrings[1], vMinBounds[nDim1], vMaxBounds[nDim2]);
		qglCallLists(g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);
	}
	else {
		qglBegin(GL_LINES);

		qglVertex3f(0, vMinBounds[nDim1], vMinBounds[nDim2] - 6.0f / m_fScale);
		qglVertex3f(0, vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale);

		qglVertex3f(0, vMinBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale);
		qglVertex3f(0, vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale);

		qglVertex3f(0, vMaxBounds[nDim1], vMinBounds[nDim2] - 6.0f / m_fScale);
		qglVertex3f(0, vMaxBounds[nDim1], vMinBounds[nDim2] - 10.0f / m_fScale);

		qglVertex3f(0, vMaxBounds[nDim1] + 6.0f / m_fScale, vMinBounds[nDim2]);
		qglVertex3f(0, vMaxBounds[nDim1] + 10.0f / m_fScale, vMinBounds[nDim2]);

		qglVertex3f(0, vMaxBounds[nDim1] + 10.0f / m_fScale, vMinBounds[nDim2]);
		qglVertex3f(0, vMaxBounds[nDim1] + 10.0f / m_fScale, vMaxBounds[nDim2]);

		qglVertex3f(0, vMaxBounds[nDim1] + 6.0f / m_fScale, vMaxBounds[nDim2]);
		qglVertex3f(0, vMaxBounds[nDim1] + 10.0f / m_fScale, vMaxBounds[nDim2]);

		qglEnd();

		qglRasterPos3f(0, Betwixt(vMinBounds[nDim1], vMaxBounds[nDim1]), vMinBounds[nDim2] - 20.0 / m_fScale);
		g_strDim.Format(g_pDimStrings[nDim1], vSize[nDim1]);
		qglCallLists(g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

		qglRasterPos3f(0, vMaxBounds[nDim1] + 16.0 / m_fScale, Betwixt(vMinBounds[nDim2], vMaxBounds[nDim2]));
		g_strDim.Format(g_pDimStrings[nDim2], vSize[nDim2]);
		qglCallLists(g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

		qglRasterPos3f(0, vMinBounds[nDim1] + 4.0, vMaxBounds[nDim2] + 8 / m_fScale);
		g_strDim.Format(g_pOrgStrings[2], vMinBounds[nDim1], vMaxBounds[nDim2]);
		qglCallLists(g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);
	}
}

/* XY_Draw */
long		g_lCount = 0;
long		g_lTotal = 0;
extern void DrawBrushEntityName(brush_t *b);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::XY_Draw() {
	brush_t		*brush;
	float		w, h;
	entity_t	*e;
	idVec3		mins, maxs;
	int			drawn, culled;
	int			i;

	if (!active_brushes.next) {
		return; // not valid yet
	}

	// clear
	m_bDirty = false;

	GL_State( GLS_DEFAULT );
	qglViewport(0, 0, m_nWidth, m_nHeight);
	qglScissor(0, 0, m_nWidth, m_nHeight);
	qglClearColor
	(
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][0],
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][1],
		g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][2],
		0
	);

	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_CULL_FACE);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set up viewpoint
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();

	w = m_nWidth / 2 / m_fScale;
	h = m_nHeight / 2 / m_fScale;

	int nDim1 = (m_nViewType == YZ) ? 1 : 0;
	int nDim2 = (m_nViewType == XY) ? 1 : 2;
	mins[0] = m_vOrigin[nDim1] - w;
	maxs[0] = m_vOrigin[nDim1] + w;
	mins[1] = m_vOrigin[nDim2] - h;
	maxs[1] = m_vOrigin[nDim2] + h;

	idBounds viewBounds( mins, maxs );
	viewBounds[0].z = -99999;
	viewBounds[1].z = 99999;

	qglOrtho(mins[0], maxs[0], mins[1], maxs[1], MIN_WORLD_COORD, MAX_WORLD_COORD);

	// draw stuff
	globalImages->BindNull();
	// now draw the grid
	qglLineWidth(0.25);
	XY_DrawGrid();
	qglLineWidth(0.5);

	drawn = culled = 0;

	if (m_nViewType != XY) {
		qglPushMatrix();
		if (m_nViewType == YZ) {
			qglRotatef(-90, 0, 1, 0);	// put Z going up
		}

		// else
		qglRotatef(-90, 1, 0, 0);		// put Z going up
	}

	e = world_entity;

	for ( brush = active_brushes.next; brush != &active_brushes; brush = brush->next ) {
		if ( brush->forceVisibile || ( brush->owner->eclass->nShowFlags & ( ECLASS_LIGHT | ECLASS_PROJECTEDLIGHT ) ) ) {
		} else if (	brush->mins[nDim1] > maxs[0] ||	brush->mins[nDim2] > maxs[1] ||	brush->maxs[nDim1] < mins[0] || brush->maxs[nDim2] < mins[1] ) {
			culled++;
			continue;				// off screen
		}

		if ( FilterBrush(brush) ) {
			continue;
		}

		drawn++;

		if (brush->owner != e && brush->owner) {
			qglColor3fv(brush->owner->eclass->color.ToFloatPtr());
		}
		else {
			qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES].ToFloatPtr());
		}

		Brush_DrawXY( brush, m_nViewType );
	}

	DrawPathLines();

	// draw pointfile
	if (g_qeglobals.d_pointfile_display_list) {
		qglCallList(g_qeglobals.d_pointfile_display_list);
	}

	if (!(m_nViewType == XY)) {
		qglPopMatrix();
	}

	// draw block grid
	if (g_qeglobals.show_blocks) {
		XY_DrawBlockGrid();
	}

	// now draw selected brushes
	if (m_nViewType != XY) {
		qglPushMatrix();
		if (m_nViewType == YZ) {
			qglRotatef(-90, 0, 1, 0);	// put Z going up
		}

		// else
		qglRotatef(-90, 1, 0, 0);		// put Z going up
	}

	qglPushMatrix();
	qglTranslatef
	(
		g_qeglobals.d_select_translate[0],
		g_qeglobals.d_select_translate[1],
		g_qeglobals.d_select_translate[2]
	);

	if (RotateMode()) {
		qglColor3f( 0.8f, 0.1f, 0.9f );
	}
	else if (ScaleMode()) {
		qglColor3f( 0.1f, 0.8f, 0.1f );
	}
	else {
		qglColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES].ToFloatPtr());
	}

	if (g_PrefsDlg.m_bNoStipple == FALSE) {
		qglEnable(GL_LINE_STIPPLE);
		qglLineStipple(3, 0xaaaa);
	}

	qglLineWidth(1);

	idVec3	vMinBounds;
	idVec3	vMaxBounds;
	vMinBounds[0] = vMinBounds[1] = vMinBounds[2] = 999999.9f;
	vMaxBounds[0] = vMaxBounds[1] = vMaxBounds[2] = -999999.9f;

	int		nSaveDrawn = drawn;
	bool	bFixedSize = false;
	for (brush = selected_brushes.next; brush != &selected_brushes; brush = brush->next) {
		drawn++;
		Brush_DrawXY(brush, m_nViewType, true);

		if (!bFixedSize) {
			if (brush->owner->eclass->fixedsize) {
				bFixedSize = true;
			}

			if (g_PrefsDlg.m_bSizePaint) {
				for (i = 0; i < 3; i++) {
					if (brush->mins[i] < vMinBounds[i]) {
						vMinBounds[i] = brush->mins[i];
					}

					if (brush->maxs[i] > vMaxBounds[i]) {
						vMaxBounds[i] = brush->maxs[i];
					}
				}
			}
		}
	}

	if (g_PrefsDlg.m_bNoStipple == FALSE) {
		qglDisable(GL_LINE_STIPPLE);
	}

	qglLineWidth(0.5);

	if (!bFixedSize && !RotateMode() && !ScaleMode() && drawn - nSaveDrawn > 0 && g_PrefsDlg.m_bSizePaint) {
		PaintSizeInfo(nDim1, nDim2, vMinBounds, vMaxBounds);
	}

	// edge / vertex flags
	if (g_qeglobals.d_select_mode == sel_vertex) {
		qglPointSize(4);
		qglColor3f(0, 1, 0);
		qglBegin(GL_POINTS);
		for (i = 0; i < g_qeglobals.d_numpoints; i++) {
			qglVertex3fv(g_qeglobals.d_points[i].ToFloatPtr());
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
			qglVertex3f((v1[0] + v2[0]) * 0.5, (v1[1] + v2[1]) * 0.5, (v1[2] + v2[2]) * 0.5);
		}

		qglEnd();
		qglPointSize(1);
	}

	g_splineList->draw (static_cast<bool>(g_qeglobals.d_select_mode == sel_editpoint || g_qeglobals.d_select_mode == sel_addpoint));

	if (g_pParentWnd->GetNurbMode() && g_pParentWnd->GetNurb()->GetNumValues()) {
		int maxage = g_pParentWnd->GetNurb()->GetNumValues();
		int time = 0;
		qglColor3f(0, 0, 1);
		qglPointSize(1);
		qglBegin(GL_POINTS);
		g_pParentWnd->GetNurb()->SetOrder(3);
		for (i = 0; i < 100; i++) {
			idVec2 v = g_pParentWnd->GetNurb()->GetCurrentValue(time);
			qglVertex3f(v.x, v.y, 0.0f);
			time += 10;
		}
		qglEnd();
		qglPointSize(4);
		qglColor3f(0, 0, 1);
		qglBegin(GL_POINTS);
		for (i = 0; i < maxage; i++) {
			idVec2 v = g_pParentWnd->GetNurb()->GetValue(i);
			qglVertex3f(v.x, v.y, 0.0f);
		}
		qglEnd();
		qglPointSize(1);
	}

	qglPopMatrix();

	qglTranslatef
	(
		-g_qeglobals.d_select_translate[0],
		-g_qeglobals.d_select_translate[1],
		-g_qeglobals.d_select_translate[2]
	);

	if (!(m_nViewType == XY)) {
		qglPopMatrix();
	}

	// area selection hack
	if (g_qeglobals.d_select_mode == sel_area) {
		qglEnable(GL_BLEND);
        qglPolygonMode ( GL_FRONT_AND_BACK , GL_FILL );
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglColor4f(0.0, 0.0, 1.0, 0.25);
		qglRectf
		(
			g_qeglobals.d_vAreaTL[nDim1],
			g_qeglobals.d_vAreaTL[nDim2],
			g_qeglobals.d_vAreaBR[nDim1],
			g_qeglobals.d_vAreaBR[nDim2]
		);
		qglDisable(GL_BLEND);
        qglPolygonMode ( GL_FRONT_AND_BACK , GL_LINE );
		qglColor3f(1.0f, 1.0f, 1.0f);
		qglRectf
		(
			g_qeglobals.d_vAreaTL[nDim1],
			g_qeglobals.d_vAreaTL[nDim2],
			g_qeglobals.d_vAreaBR[nDim1],
			g_qeglobals.d_vAreaBR[nDim2]
		);

	}

	// now draw camera point
	DrawCameraIcon();
	DrawZIcon();

	if (RotateMode()) {
		DrawRotateIcon();
	}

	/// Draw a "precision crosshair" if enabled 
	if( m_precisionCrosshairMode != PRECISION_CROSSHAIR_NONE )
		DrawPrecisionCrosshair();

	qglFlush();

	// QE_CheckOpenGLForErrors();
}


/*
 =======================================================================================================================
 =======================================================================================================================
 */
idVec3 &CXYWnd::GetOrigin() {
	return m_vOrigin;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::SetOrigin(idVec3 org) {
	m_vOrigin[0] = org[0];
	m_vOrigin[1] = org[1];
	m_vOrigin[2] = org[2];
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnSize(UINT nType, int cx, int cy) {
	CWnd::OnSize(nType, cx, cy);

	CRect	rect;
	GetClientRect(rect);
	m_nWidth = rect.Width();
	m_nHeight = rect.Height();
	InvalidateRect(NULL, false);
}

brush_t hold_brushes;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::Clip() {
	if (ClipMode()) {
		hold_brushes.next = &hold_brushes;
		ProduceSplitLists();

		// brush_t* pList = (g_bSwitch) ? &g_brFrontSplits : &g_brBackSplits;
		brush_t *pList;
		if (g_PrefsDlg.m_bSwitchClip) {
			pList = ((m_nViewType == XZ) ? g_bSwitch : !g_bSwitch) ? &g_brFrontSplits : &g_brBackSplits;
		}
		else {
			pList = ((m_nViewType == XZ) ? !g_bSwitch : g_bSwitch) ? &g_brFrontSplits : &g_brBackSplits;
		}

		if (pList->next != pList) {
			Brush_CopyList(pList, &hold_brushes);
			CleanList(&g_brFrontSplits);
			CleanList(&g_brBackSplits);
			Select_Delete();
			Brush_CopyList(&hold_brushes, &selected_brushes);
			if (RogueClipMode()) {
				RetainClipMode(false);
			}
			else {
				RetainClipMode(true);
			}

			Sys_UpdateWindows(W_ALL);
		}
	}
	else if (PathMode()) {
		FinishSmartCreation();
		if (g_pPathFunc) {
			g_pPathFunc(true, g_nPathCount);
		}

		g_pPathFunc = NULL;
		g_nPathCount = 0;
		g_bPathMode = false;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::SplitClip() {
	ProduceSplitLists();
	if ((g_brFrontSplits.next != &g_brFrontSplits) && (g_brBackSplits.next != &g_brBackSplits)) {
		Select_Delete();
		Brush_CopyList(&g_brFrontSplits, &selected_brushes);
		Brush_CopyList(&g_brBackSplits, &selected_brushes);
		CleanList(&g_brFrontSplits);
		CleanList(&g_brBackSplits);
		if (RogueClipMode()) {
			RetainClipMode(false);
		}
		else {
			RetainClipMode(true);
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::FlipClip() {
	g_bSwitch = !g_bSwitch;
	Sys_UpdateWindows(XY | W_CAMERA_IFON);
}

//
// =======================================================================================================================
//    makes sure the selected brush or camera is in view
// =======================================================================================================================
//
void CXYWnd::PositionView() {
	int		nDim1 = (m_nViewType == YZ) ? 1 : 0;
	int		nDim2 = (m_nViewType == XY) ? 1 : 2;
	brush_t *b = selected_brushes.next;
	if (b && b->next != b) {
		m_vOrigin[nDim1] = b->mins[nDim1];
		m_vOrigin[nDim2] = b->mins[nDim2];
	}
	else {
		m_vOrigin[nDim1] = g_pParentWnd->GetCamera()->Camera().origin[nDim1];
		m_vOrigin[nDim2] = g_pParentWnd->GetCamera()->Camera().origin[nDim2];
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::VectorCopyXY(const idVec3 &in, idVec3 &out) {
	if (m_nViewType == XY) {
		out[0] = in[0];
		out[1] = in[1];
	}
	else if (m_nViewType == XZ) {
		out[0] = in[0];
		out[2] = in[2];
	}
	else {
		out[1] = in[1];
		out[2] = in[2];
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnDestroy() {
	CWnd::OnDestroy();

	// delete this;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::SetViewType(int n) {
	m_nViewType = n;
	char *p = "YZ Side";
	if (m_nViewType == XY) {
		p = "XY Top";
	} else if (m_nViewType == XZ) {
		p = "XZ Front";
	}
	SetWindowText(p);
};

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::Redraw(unsigned int nBits) {
	m_nUpdateBits = nBits;
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	m_nUpdateBits = W_XY;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CXYWnd::RotateMode() {
	return g_bRotateMode;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CXYWnd::ScaleMode() {
	return g_bScaleMode;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
extern bool Select_OnlyModelsSelected();
bool CXYWnd::SetRotateMode(bool bMode) {
	if (bMode && selected_brushes.next != &selected_brushes) {
		g_bRotateMode = true;
		if (Select_OnlyModelsSelected()) {
			Select_GetTrueMid(g_vRotateOrigin);
		} else {
			Select_GetMid(g_vRotateOrigin);
		}
		g_vRotation.Zero();
		Select_InitializeRotation();
	}
	else {
		if (bMode) {
			Sys_Status("Need a brush selected to turn on Mouse Rotation mode\n");
		}

		g_bRotateMode = false;
		Select_FinalizeRotation();
	}

	RedrawWindow();
	return g_bRotateMode;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::SetScaleMode(bool bMode) {
	g_bScaleMode = bMode;
	RedrawWindow();
}

//
// =======================================================================================================================
//    xy - z xz - y yz - x
// =======================================================================================================================
//
void CXYWnd::OnSelectMouserotate() {
	// TODO: Add your command handler code here
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CleanCopyEntities() {
	entity_t	*pe = g_enClipboard.next;
	while (pe != NULL && pe != &g_enClipboard) {
		entity_t	*next = pe->next;
		pe->epairs.Clear();

		Entity_Free(pe);
		pe = next;
	}

	g_enClipboard.next = g_enClipboard.prev = &g_enClipboard;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
entity_t *Entity_CopyClone(entity_t *e) {
	entity_t	*n;

	n = Entity_New();
	n->brushes.onext = n->brushes.oprev = &n->brushes;
	n->eclass = e->eclass;
	n->rotation = e->rotation;

	// add the entity to the entity list
	n->next = g_enClipboard.next;
	g_enClipboard.next = n;
	n->next->prev = n;
	n->prev = &g_enClipboard;

	n->epairs = e->epairs;

	return n;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool OnList(entity_t *pFind, CPtrArray *pList) {
	int nSize = pList->GetSize();
	while (nSize-- > 0) {
		entity_t	*pEntity = reinterpret_cast < entity_t * > (pList->GetAt(nSize));
		if (pEntity == pFind) {
			return true;
		}
	}

	return false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::Copy()
{
#if 1
	CWaitCursor WaitCursor;
	g_Clipboard.SetLength(0);
	g_PatchClipboard.SetLength(0);

	Map_SaveSelected(&g_Clipboard, &g_PatchClipboard);

	bool	bClipped = false;
	UINT	nClipboard = ::RegisterClipboardFormat("RadiantClippings");
	if (nClipboard > 0) {
		if (OpenClipboard()) {
			::EmptyClipboard();

			long	lSize = g_Clipboard.GetLength();
			HANDLE	h = ::GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, lSize + sizeof (long));
			if (h != NULL) {
				unsigned char	*cp = reinterpret_cast < unsigned char * > (::GlobalLock(h));
				memcpy(cp, &lSize, sizeof (long));
				cp += sizeof (long);
				g_Clipboard.SeekToBegin();
				g_Clipboard.Read(cp, lSize);
				::GlobalUnlock(h);
				::SetClipboardData(nClipboard, h);
				::CloseClipboard();
				bClipped = true;
			}
		}
	}

	if (!bClipped) {
		common->Printf("Unable to register Windows clipboard formats, copy/paste between editors will not be possible");
	}

	/*
	 * CString strOut; ::GetTempPath(1024, strOut.GetBuffer(1024));
	 * strOut.ReleaseBuffer(); AddSlash(strOut); strOut += "RadiantClipboard.$$$";
	 * Map_SaveSelected(strOut.GetBuffer(0));
	 */
#else
	CPtrArray	holdArray;
	CleanList(&g_brClipboard);
	CleanCopyEntities();
	for (brush_t * pBrush = selected_brushes.next; pBrush != NULL && pBrush != &selected_brushes; pBrush = pBrush->next) {
		if (pBrush->owner == world_entity) {
			brush_t *pClone = Brush_Clone(pBrush);
			pClone->owner = NULL;
			Brush_AddToList(pClone, &g_brClipboard);
		}
		else {
			if (!OnList(pBrush->owner, &holdArray)) {
				entity_t	*e = pBrush->owner;
				holdArray.Add(reinterpret_cast < void * > (e));

				entity_t	*pEClone = Entity_CopyClone(e);
				for (brush_t * pEB = e->brushes.onext; pEB != &e->brushes; pEB = pEB->onext) {
					brush_t *pClone = Brush_Clone(pEB);

					// Brush_AddToList (pClone, &g_brClipboard);
					Entity_LinkBrush(pEClone, pClone);
					Brush_Build(pClone);
				}
			}
		}
	}
#endif
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::Undo() {
	/*
	 * if (g_brUndo.next != &g_brUndo) { g_bScreenUpdates = false; Select_Delete();
	 * for (brush_t* pBrush = g_brUndo.next ; pBrush != NULL && pBrush != &g_brUndo ;
	 * pBrush=pBrush->next) { brush_t* pClone = Brush_Clone(pBrush); Brush_AddToList
	 * (pClone, &active_brushes); Entity_LinkBrush (pBrush->pUndoOwner, pClone);
	 * Brush_Build(pClone); Select_Brush(pClone); } CleanList(&g_brUndo);
	 * g_bScreenUpdates = true; Sys_UpdateWindows(W_ALL); } else common->Printf("Nothing
	 * to undo.../n");
	 */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::UndoClear() {
	/* CleanList(&g_brUndo); */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::UndoCopy() {
	/*
	 * CleanList(&g_brUndo); for (brush_t* pBrush = selected_brushes.next ; pBrush !=
	 * NULL && pBrush != &selected_brushes ; pBrush=pBrush->next) { brush_t* pClone =
	 * Brush_Clone(pBrush); pClone->pUndoOwner = pBrush->owner; Brush_AddToList
	 * (pClone, &g_brUndo); }
	 */
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CXYWnd::UndoAvailable() {
	return(g_brUndo.next != &g_brUndo);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::Paste()
{
#if 1

	CWaitCursor WaitCursor;
	bool		bPasted = false;
	UINT		nClipboard = ::RegisterClipboardFormat("RadiantClippings");
	if (nClipboard > 0 && OpenClipboard() && ::IsClipboardFormatAvailable(nClipboard)) {
		HANDLE	h = ::GetClipboardData(nClipboard);
		if (h) {
			g_Clipboard.SetLength(0);

			unsigned char	*cp = reinterpret_cast < unsigned char * > (::GlobalLock(h));
			long			lSize = 0;
			memcpy(&lSize, cp, sizeof (long));
			cp += sizeof (long);
			g_Clipboard.Write(cp, lSize);
		}

		::GlobalUnlock(h);
		::CloseClipboard();
	}

	if (g_Clipboard.GetLength() > 0) {
		g_Clipboard.SeekToBegin();

		int		nLen = g_Clipboard.GetLength();
		char	*pBuffer = new char[nLen + 1];
		memset(pBuffer, 0, sizeof(pBuffer));
		g_Clipboard.Read(pBuffer, nLen);
		pBuffer[nLen] = '\0';
		Map_ImportBuffer(pBuffer, !(GetAsyncKeyState(VK_SHIFT) & 0x8000));
		delete[] pBuffer;
	}

	#if 0
	if (g_PatchClipboard.GetLength() > 0) {
		g_PatchClipboard.SeekToBegin();

		int		nLen = g_PatchClipboard.GetLength();
		char	*pBuffer = new char[nLen + 1];
		g_PatchClipboard.Read(pBuffer, nLen);
		pBuffer[nLen] = '\0';
		Patch_ReadBuffer(pBuffer, true);
		delete[] pBuffer;
	}
	#endif
#else
	if (g_brClipboard.next != &g_brClipboard || g_enClipboard.next != &g_enClipboard) {
		Select_Deselect();

		for (brush_t * pBrush = g_brClipboard.next; pBrush != NULL && pBrush != &g_brClipboard; pBrush = pBrush->next) {
			brush_t *pClone = Brush_Clone(pBrush);

			// pClone->owner = pBrush->owner;
			if (pClone->owner == NULL) {
				Entity_LinkBrush(world_entity, pClone);
			}

			Brush_AddToList(pClone, &selected_brushes);
			Brush_Build(pClone);
		}

		for
		(
			entity_t * pEntity = g_enClipboard.next;
			pEntity != NULL && pEntity != &g_enClipboard;
			pEntity = pEntity->next
		) {
			entity_t	*pEClone = Entity_Clone(pEntity);
			for (brush_t * pEB = pEntity->brushes.onext; pEB != &pEntity->brushes; pEB = pEB->onext) {
				brush_t *pClone = Brush_Clone(pEB);
				Brush_AddToList(pClone, &selected_brushes);
				Entity_LinkBrush(pEClone, pClone);
				Brush_Build(pClone);
				if (pClone->owner && pClone->owner != world_entity) {
					g_Inspectors->UpdateEntitySel(pClone->owner->eclass);
				}
			}
		}

		Sys_UpdateWindows(W_ALL);
	}
	else {
		common->Printf("Nothing to paste.../n");
	}
#endif
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
idVec3 &CXYWnd::Rotation() {
	return g_vRotation;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
idVec3 &CXYWnd::RotateOrigin() {
	return g_vRotateOrigin;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnTimer(UINT nIDEvent) {
	if (nIDEvent == 100) {
		int nDim1 = (m_nViewType == YZ) ? 1 : 0;
		int nDim2 = (m_nViewType == XY) ? 1 : 2;
		m_vOrigin[nDim1] += m_ptDragAdj.x / m_fScale;
		m_vOrigin[nDim2] -= m_ptDragAdj.y / m_fScale;
		Sys_UpdateWindows(W_XY | W_CAMERA);

		// int nH = (m_ptDrag.y == 0) ? -1 : m_ptDrag.y;
		m_ptDrag += m_ptDragAdj;
		m_ptDragTotal += m_ptDragAdj;
		XY_MouseMoved(m_ptDrag.x, m_nHeight - 1 - m_ptDrag.y, m_nScrollFlags);

		//
		// m_vOrigin[nDim1] -= m_ptDrag.x / m_fScale; m_vOrigin[nDim1] -= m_ptDrag.x /
		// m_fScale;
		//
	} 
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
	g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags, false);

	// CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR *lpncsp) {
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnKillFocus(CWnd *pNewWnd) {
	CWnd::OnKillFocus(pNewWnd);
	SendMessage(WM_NCACTIVATE, FALSE, 0);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnSetFocus(CWnd *pOldWnd) {
	CWnd::OnSetFocus(pOldWnd);
	SendMessage(WM_NCACTIVATE, TRUE, 0);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void CXYWnd::OnClose() {
	CWnd::OnClose();
}

//
// =======================================================================================================================
//    should be static as should be the rotate scale stuff
// =======================================================================================================================
//
bool CXYWnd::AreaSelectOK() {
	return RotateMode() ? false : ScaleMode() ? false : true;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
BOOL CXYWnd::OnEraseBkgnd(CDC *pDC) {
	return TRUE;

	// return CWnd::OnEraseBkgnd(pDC);
}

extern void AssignModel();
void CXYWnd::OnDropNewmodel() 
{
	CPoint point;
	GetCursorPos(&point);
	CreateRightClickEntity(this, m_ptDown.x, m_ptDown.y, "func_static");
	g_Inspectors->SetMode(W_ENTITY);
	g_Inspectors->AssignModel();
}

BOOL CXYWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (zDelta > 0) {
		g_pParentWnd->OnViewZoomin();
	} else {
		g_pParentWnd->OnViewZoomout();
	}
	return TRUE;
}




 //---------------------------------------------------------------------------
 // CyclePrecisionCrosshairMode
 // 
 // Called when the user presses the "cycle precision cursor mode" key.
 // Cycles the precision cursor among the following three modes:
 //		PRECISION_CURSOR_NONE
 //		PRECISION_CURSOR_SNAP
 //		PRECISION_CURSOR_FREE
 //---------------------------------------------------------------------------
 void CXYWnd::CyclePrecisionCrosshairMode( void )
 {
	 common->Printf("TODO: Make DrawPrecisionCrosshair work..." );

	 /// Cycle to next mode, wrap if necessary
	 m_precisionCrosshairMode ++;
	 if( m_precisionCrosshairMode >= PRECISION_CROSSHAIR_MAX )
		 m_precisionCrosshairMode = PRECISION_CROSSHAIR_NONE;
	 Sys_UpdateWindows( W_XY );
 }

 //---------------------------------------------------------------------------
// DrawPrecisionCrosshair 
// 
// Draws a precision crosshair beneath the cursor in the 2d (XY) view,
//  depending on one of the following values for m_precisionCrosshairMode:
// 
// PRECISION_CROSSHAIR_NONE		No crosshair is drawn.  Do not force refresh of XY view.
// PRECISION_CROSSHAIR_SNAP		Crosshair snaps to grid size.  Force refresh of XY view.
// PRECISION_CROSSHAIR_FREE		Crosshair does not snap to grid.  Force refresh of XY view.
//---------------------------------------------------------------------------
void CXYWnd::DrawPrecisionCrosshair( void )
{
	// FIXME: m_mouseX, m_mouseY, m_axisHoriz, m_axisVert, etc... are never set
	return;

	idVec3 mouse3dPos (0.0f, 0.0f, 0.0f);
	float x, y;
	idVec4 crossEndColor (1.0f, 0.0f, 1.0f, 1.0f); // the RGBA color of the precision crosshair at its ends
	idVec4 crossMidColor; // the RGBA color of the precision crosshair at the crossing point
	
	/// Transform the mouse coordinates into axis-correct map-coordinates
	if( m_precisionCrosshairMode == PRECISION_CROSSHAIR_SNAP )
		SnapToPoint( m_mouseX, m_mouseY, mouse3dPos );
	else
		XY_ToPoint( m_mouseX, m_mouseY, mouse3dPos );
	x = mouse3dPos[ m_axisHoriz ];
	y = mouse3dPos[ m_axisVert ];
	
	/// Use the color specified by the user

	crossEndColor[0] = g_qeglobals.d_savedinfo.colors[ COLOR_PRECISION_CROSSHAIR ][0];
	crossEndColor[1] = g_qeglobals.d_savedinfo.colors[ COLOR_PRECISION_CROSSHAIR ][1];
	crossEndColor[2] = g_qeglobals.d_savedinfo.colors[ COLOR_PRECISION_CROSSHAIR ][2];
	crossEndColor[3] = 1.0f;

	crossMidColor = crossEndColor;

	if( m_precisionCrosshairMode == PRECISION_CROSSHAIR_FREE )
		crossMidColor[ 3 ] = 0.0f; // intersection-color is 100% transparent (alpha = 0.0f)
	
	/// Set up OpenGL states (for drawing smooth-shaded plain-colored lines)
	qglEnable( GL_BLEND );
	qglDisable( GL_TEXTURE_2D );
	qglShadeModel( GL_SMOOTH );
	qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	/// Draw a fullscreen-sized crosshair over the cursor
	qglBegin( GL_LINES );
	{
		/// Draw the horizontal precision line (in two pieces)
		qglColor4fv( crossEndColor.ToFloatPtr() );
		qglVertex2f( m_mcLeft, y );
		qglColor4fv( crossMidColor.ToFloatPtr() );
		qglVertex2f( x, y );
		qglColor4fv( crossMidColor.ToFloatPtr() );
		qglVertex2f( x, y );
		qglColor4fv( crossEndColor.ToFloatPtr() );
		qglVertex2f( m_mcRight, y );
		
		/// Draw the vertical precision line (in two pieces)
		qglColor4fv( crossEndColor.ToFloatPtr() );
		qglVertex2f( x, m_mcTop );
		qglColor4fv( crossMidColor.ToFloatPtr() );
		qglVertex2f( x, y );
		qglColor4fv( crossMidColor.ToFloatPtr() );
		qglVertex2f( x, y );
		qglColor4fv( crossEndColor.ToFloatPtr() );
		qglVertex2f( x, m_mcBottom );
	}
	qglEnd(); // GL_LINES
	
	// Radiant was in opaque, flat-shaded mode by default; restore this to prevent possible slowdown
	qglShadeModel( GL_FLAT );
	qglDisable( GL_BLEND );
}

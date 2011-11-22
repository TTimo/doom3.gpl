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
#if !defined(AFX_XYWND_H__44B4BA04_781B_11D1_B53C_00AA00A410FC__INCLUDED_)
#define AFX_XYWND_H__44B4BA04_781B_11D1_B53C_00AA00A410FC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// XYWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CXYWnd window

#include "qe3.h"
#include "CamWnd.h"

const int SCALE_X = 0x01;
const int SCALE_Y = 0x02;
const int SCALE_Z = 0x04;

bool FilterBrush(brush_t *pb);

typedef void (PFNPathCallback)(bool, int);
// as i didn't really encapsulate anything this
// should really be a struct..
class CClipPoint
{
public:
  CClipPoint(){ Reset(); };
  void Reset(){ m_ptClip[0] = m_ptClip[1] = m_ptClip[2] = 0.0; m_bSet = false; m_pVec3 = NULL;};
  bool Set(){ return m_bSet; };
  void Set(bool b) { m_bSet = b; };
  void UpdatePointPtr() { if (m_pVec3) VectorCopy(m_ptClip, *m_pVec3); };
  void SetPointPtr(idVec3* p) { m_pVec3 = p; };
  idVec3 m_ptClip;      // the 3d point
  idVec3* m_pVec3;      // optional ptr for 3rd party updates
  CPoint m_ptScreen;    // the onscreen xy point (for mousability)
  bool m_bSet;
  operator idVec3&() {return m_ptClip;};
  operator idVec3*() {return &m_ptClip;};
  operator float*() {return m_ptClip.ToFloatPtr();};
};

class CXYWnd : public CWnd
{
  DECLARE_DYNCREATE(CXYWnd);
// Construction
public:
	CXYWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXYWnd)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
  bool AreaSelectOK();
  idVec3& RotateOrigin();
  idVec3& Rotation();
  void UndoClear();
  bool UndoAvailable();
  void KillPathMode();
  void Undo();
  void UndoCopy();
  void Copy();
  void Paste();
  void Redraw(unsigned int nBits);
  void VectorCopyXY( const idVec3 &in, idVec3 &out );
	void PositionView();
	void FlipClip();
	void SplitClip();
	void Clip();
	idVec3& GetOrigin();
	void SetOrigin(idVec3 org);		// PGM
	void XY_Init();
  void XY_Draw();
  void DrawZIcon();
  void DrawRotateIcon();
  void DrawCameraIcon();
  void XY_DrawBlockGrid();
  void XY_DrawGrid();
  bool XY_MouseMoved (int x, int y, int buttons);
  void NewBrushDrag (int x, int y);
  bool DragDelta (int x, int y, idVec3 &move);
  void XY_MouseUp(int x, int y, int buttons);
  void XY_MouseDown (int x, int y, int buttons);
  void XY_ToGridPoint (int x, int y, idVec3 &point);
  void XY_ToPoint (int x, int y, idVec3 &point);
  void SnapToPoint (int x, int y, idVec3 &point);
  void SetActive(bool b) {m_bActive = b;};
  bool Active() {return m_bActive;};
  void DropClipPoint(UINT nFlags, CPoint point);

	int GetAxisHoriz() { return m_axisHoriz; };
	int GetAxisVert() { return m_axisVert; };
	void AnalogMouseZoom( int mouseDeltaY );


  bool RogueClipMode();
	bool ClipMode();
	void SetClipMode(bool bMode);
	void RetainClipMode(bool bMode);

  bool RotateMode();
  bool SetRotateMode(bool bMode);
  bool ScaleMode();
  void SetScaleMode(bool bMode);

  bool PathMode();
  void DropPathPoint(UINT nFlags, CPoint point);

  bool PointMode();
  void AddPointPoint(UINT nFlags, idVec3* pVec);
  void SetPointMode(bool b);


	virtual ~CXYWnd();
  void SetViewType(int n);
  int GetViewType() {return  m_nViewType; };
  void SetScale(float f) {m_fScale = f;};
  float Scale() {return m_fScale;};
  int Width() {return m_nWidth;}
  int Height() {return m_nHeight;}
  bool m_bActive;

	void UpdateViewDependencies( void );

	void DrawPrecisionCrosshair(); 
	void CyclePrecisionCrosshairMode();
	enum 
	{
		PRECISION_CROSSHAIR_NONE = 0,
		PRECISION_CROSSHAIR_SNAP = 1,
		PRECISION_CROSSHAIR_FREE = 2,
		PRECISION_CROSSHAIR_MAX,
	};

	int m_precisionCrosshairMode;
	int m_mouseX;
	int m_mouseY;


	// Generated message map functions
protected:
	int m_nUpdateBits;
	int m_nWidth;
  int m_nHeight;
	float	m_fScale;
	float	m_TopClip;
  float m_BottomClip;
  bool m_bDirty;
	idVec3 m_vOrigin;
	CPoint m_ptCursor;
  bool m_bRButtonDown;

  int	m_nButtonstate;
  int m_nPressx;
  int m_nPressy;
  idVec3 m_vPressdelta;
  bool m_bPress_selection;

	int m_axisHoriz; //  <axisHoriz> and <axisVert> are one of AXIS_X, AXIS_Y, AXIS_Z and
	int m_axisVert;	 //  reflect which axes are represented horizontally and vertically in the 2d view (XY, XZ, etc)

	/// Each of the following _mc fields are stored in map-coordinates, NOT screen-pixels 
	float m_mcWidth;
	float m_mcHeight;
	float m_mcLeft;
	float m_mcRight;
	float m_mcTop;
	float m_mcBottom;


  friend CCamWnd;
  //friend C3DFXCamWnd;

  CMenu m_mnuDrop;
  int m_nViewType;

  unsigned int m_nTimerID;
  int m_nScrollFlags;
  CPoint m_ptDrag;
  CPoint m_ptDragAdj;
  CPoint m_ptDragTotal;

	void OriginalButtonUp(UINT nFlags, CPoint point);
	void OriginalButtonDown(UINT nFlags, CPoint point);
  void ProduceSplits(brush_t** pFront, brush_t** pBack);
  void ProduceSplitLists();
  void HandleDrop();
  void PaintSizeInfo(int nDim1, int nDim2, idVec3 vMinBounds, idVec3 vMaxBounds);
	void DrawSelectedCentroid( int nDim1, int nDim2, idVec3 vMinBounds, idVec3 vMaxBounds );

  void OnEntityCreate(unsigned int nID);
  CPoint m_ptDown;
	//{{AFX_MSG(CXYWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnSelectMouserotate();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDropNewmodel();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnCmdMsg( UINT nID, int nCode, void *pExtra, AFX_CMDHANDLERINFO *pHandlerInfo );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XYWND_H__44B4BA04_781B_11D1_B53C_00AA00A410FC__INCLUDED_)

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


#include "VectorCtl.h"
#include <math.h>

BEGIN_MESSAGE_MAP(CVectorCtl, CButton)
	//{{AFX_MSG_MAP(idGLWidget)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CVectorCtl::CVectorCtl () :
    m_bBmpCreated (FALSE), 
    m_bImageChange (TRUE),
    m_bBackgroundBitmapUsed (FALSE),
    m_clrDiffuse (DEFAULT_DIFFUSE),
    m_clrAmbient (DEFAULT_AMBIENT),
    m_clrLight (DEFAULT_LIGHT),
    m_clrBackgroundStart (DEFAULT_START_BACKGROUND_COLOR),
    m_clrBackgroundEnd (DEFAULT_END_BACKGROUND_COLOR),
    m_dSpecularExponent (DEFAULT_SPEC_EXP),
    m_bHasFocus (FALSE), 
    m_bSelected (FALSE),
    m_bFrontVector (FALSE),
    m_dSensitivity (20.0),
    m_procVectorChanging (NULL),
    m_procVectorChanged (NULL)
{
    double DefaultVec[3] = DEFAULT_VEC;
    for (int i=0; i<3; i++) {
        m_dVec[i] = DefaultVec[i];
        pCtl[i] = NULL;
    }

	rotationQuat.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	lastPress.Zero();
	radius = 0.6f;
}


CVectorCtl::~CVectorCtl () 
{
    if (m_bBmpCreated) 
        m_dcMem.SelectObject (m_pOldBitmap);
    ClearBackgroundBitmap ();
}

// Owner-drawn control service function:
void CVectorCtl::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
    CDC *pDC = CDC::FromHandle (lpDrawItemStruct->hDC); // Get CDC to draw

    if (!m_bSelected && lpDrawItemStruct->itemState & ODS_SELECTED) {
            // Just got re-selected (user starts a new mouse dragging session)
    } else if (m_bSelected && // Last state was selected
               !(lpDrawItemStruct->itemState & ODS_SELECTED) &&     // New state is NOT selected   
               (lpDrawItemStruct->itemState & ODS_FOCUS) &&         // New state is still in focus
               m_procVectorChanged)     // User asked for a callback
        // User has left the track-ball and asked for a callback.
        m_procVectorChanged ( rotationQuat ); 
          
    m_bHasFocus = lpDrawItemStruct->itemState & ODS_FOCUS;      // Update focus status
    m_bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;   // Update selection status

    if (!m_bBmpCreated)  // 1st time
        InitBitmap (lpDrawItemStruct, pDC);
    if (m_bImageChange) {   // Image has changes - recalc it!
        if (m_procVectorChanging)   // User has specified a callback
            m_procVectorChanging ( rotationQuat ); // Call it!
        BuildImage (lpDrawItemStruct);
        m_bImageChange = FALSE;
    }
    pDC->BitBlt (0,0,m_iWidth, m_iHeight, &m_dcMem, 0, 0, SRCCOPY); // Update screen
}

// Mouse was dragged 
void CVectorCtl::OnMouseDrag (int ixMove, int iyMove)
{
    RotateByXandY (double(-iyMove) / m_dSensitivity,
                   double(ixMove) / m_dSensitivity);
}

// Recalc ball image
void CVectorCtl::BuildImage (LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    int xf, yf;

    for (int x=0; x<m_iWidth; x++)  // Scan all columns
        for (int y=0; y<m_iHeight; y++) {   // Scan all rows
            xf = x-m_iXCenter;  // Find distance from center
            yf = y-m_iYCenter;
            if (xf*xf + yf*yf <= m_iSqrRadius) {    // Point on ball surface
                double vx = double(xf) / double(m_iRadius),
                       vy = double(yf) / double(m_iRadius),
                       vz = sqrt (1.0 - vx*vx - vy*vy);     // Find ball's normal
                m_dcMem.SetPixelV (x,y, CalcLight (vx,vy,vz));
            } 
        }
}

// Normalize a vector to unit size
BOOL CVectorCtl::Normalize ()
{
    double Norm = m_dVec[0] * m_dVec[0] + m_dVec[1] * m_dVec[1] + m_dVec[2] * m_dVec[2];

    if (Norm > EPS) {
        Norm = sqrt (Norm);
        m_dVec[0] /= Norm;
        m_dVec[1] /= Norm;
        m_dVec[2] /= Norm;
        return TRUE;
    } else {    // Reset to defualt vector
        double DefaultVec[3] = DEFAULT_VEC;
        for (int i=0; i<3; i++) 
            m_dVec[i] = DefaultVec[i];
        return FALSE;
    }
}

// Calculate lightning effect for specific pixel on ball's surface
COLORREF CVectorCtl::CalcLight (double dx, double dy, double dz)
{
    double NL = dx * m_dVec[0] + dy * m_dVec[1] + dz * m_dVec[2], 
           RV = 2.0 * NL,
           rx = m_dVec[0] - (dx * RV),
           ry = m_dVec[1] - (dy * RV),
           rz = m_dVec[2] - (dz * RV);

    if (NL < 0.0)   // Diffuse coefficient
        NL = 0.0;

    RV = max (0.0, -rz);
    RV = double(pow (RV, m_dSpecularExponent));

    int  r = int (  double(GetRValue(m_clrDiffuse)) * NL +  // Diffuse
                    double(GetRValue(m_clrLight)) * RV +    // Specular
                    double(GetRValue(m_clrAmbient))),       // Ambient

         g = int (  double(GetGValue(m_clrDiffuse)) * NL +  // Diffuse
                    double(GetGValue(m_clrLight)) * RV +    // Specular
                    double(GetGValue(m_clrAmbient))),       // Ambient    

         b = int (  double(GetBValue(m_clrDiffuse)) * NL +  // Diffuse
                    double(GetBValue(m_clrLight)) * RV +    // Specular
                    double(GetBValue(m_clrAmbient)));       // Ambient

    r = min (255, r);   // Cutoff highlight
    g = min (255, g);
    b = min (255, b);
    return RGB(BYTE(r),BYTE(g),BYTE(b));
}


// Start memory buffer bitmap and measure it
void CVectorCtl::InitBitmap (LPDRAWITEMSTRUCT lpDrawItemStruct, CDC *pDC)
{
    m_iWidth = lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left;
    m_iHeight = lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top;
    m_bmpBuffer.CreateCompatibleBitmap (pDC, m_iWidth, m_iHeight);
    m_bBmpCreated = TRUE;
    m_dcMem.CreateCompatibleDC (pDC);
    m_pOldBitmap = m_dcMem.SelectObject (&m_bmpBuffer);
    SetRadius (max (min (m_iWidth, m_iHeight) - 2, 0) / 2);
    SetCenter (m_iWidth / 2, m_iHeight / 2);
    CreateBackground ();
}

// Set new specular intensity
BOOL CVectorCtl::SetSpecularExponent (double dExp)
{
    if (dExp < 1.0 || dExp > 200.0)
        return FALSE;
    m_dSpecularExponent = dExp;
    Redraw ();
    return TRUE;
}

// Rotate our vector around the X and Y axis
void CVectorCtl::RotateByXandY (double XRot, double YRot)
{   // Angles are in radians

	if (XRot == 0.0 && YRot == 0.0) {
		return;
	}

    double cx = cos(XRot),
           sx = sin(XRot),
           cy = cos(YRot),
           sy = sin(YRot),
           dx = m_dVec[0] * cy + m_dVec[1] * sx * sy + m_dVec[2] * cx * sy,
           dy = m_dVec[1] * cx - m_dVec[2] * sx,
           dz = -m_dVec[0] * sy + m_dVec[1] * sx * cy + m_dVec[2] * cx * cy;

    if (!m_bFrontVector || dz >= 0.0) { // Vector is bounds free
        m_dVec[0] = dx;
        m_dVec[1] = dy;
        m_dVec[2] = dz;
    } else {    // Otherwise, do not allow Z to be negative (light shines from behind)
        m_dVec[2] = 0.0;
        m_dVec[0] = dx;
        m_dVec[1] = dy;
        Normalize ();
    }
    Redraw ();
}

 
void CVectorCtl::UpdateAxisControls ()
{
    CString cs;
    for (int i=0; i<3; i++) 
        if (pCtl[i]) {
            cs.Format ("%+1.5f",m_dVec[i]);
            pCtl[i]->SetWindowText (cs);
        }
}            

void CVectorCtl::SetAxisControl (int nXCtl, int nYCtl, int nZCtl)
{
    pCtl[0] = GetParent()->GetDlgItem(nXCtl);
    pCtl[1] = GetParent()->GetDlgItem(nYCtl);
    pCtl[2] = GetParent()->GetDlgItem(nZCtl);
}

void CVectorCtl::SetRadius (UINT uRadius)
{
    m_iRadius = uRadius;
    m_iSqrRadius = m_iRadius * m_iRadius;
    CreateBackground ();
    Redraw (TRUE);
}


void CVectorCtl::SetCenter (UINT uHorizPos, UINT uVertPos)
{
    m_iXCenter = uHorizPos;
    m_iYCenter = uVertPos;
    CreateBackground ();
    Redraw (TRUE);
}


void CVectorCtl::SetAxis (double d, int nAxis)
{
    if (fabs(d)>=1.0) {
        m_dVec[nAxis]=d > 1.0 ? 1.0 : -1.0;
        m_dVec[(nAxis+1) %3]=m_dVec[(nAxis+2) %3]=0.0;
        Redraw ();
        return;
    } 
    m_dVec[nAxis] = d;
    Normalize ();
    Redraw ();
}

void CVectorCtl::SetVector (double dx, double dy, double dz)
{
    m_dVec[0] = dx;
    m_dVec[1] = dy;
    m_dVec[2] = dz;
    Normalize ();
    Redraw ();
}

void CVectorCtl::SetBackgroundColor (COLORREF clrStart, COLORREF clrEnd)
{
    ClearBackgroundBitmap ();
    m_clrBackgroundStart = clrStart;
    m_clrBackgroundEnd = clrEnd;
    CreateBackground ();
}


BOOL CVectorCtl::SetBackgroundImage (UINT uBackgroundBitmapID)
{
    if (m_bBackgroundBitmapUsed) {
        ClearBackgroundBitmap ();
        CreateBackground ();
    }
    if (!m_bmpBack.LoadBitmap (uBackgroundBitmapID))
        return FALSE;
    m_bBackgroundBitmapUsed = TRUE;
    CreateBackground ();
    return TRUE;
}

void CVectorCtl::CreateBackground ()
{
    if (!m_bBmpCreated)
        return; //  No image yet
    if (!m_bBackgroundBitmapUsed) { // No background used - fill with gradient color
        double r = GetRValue (m_clrBackgroundStart),
               g = GetGValue (m_clrBackgroundStart),
               b = GetBValue (m_clrBackgroundStart), 
               rd = double (GetRValue (m_clrBackgroundEnd) - r) / double (m_iHeight),
               gd = double (GetGValue (m_clrBackgroundEnd) - g) / double (m_iHeight), 
               bd = double (GetBValue (m_clrBackgroundEnd) - b) / double (m_iHeight);
        for (int j=0; j<m_iHeight; j++) {
            for (int i=0; i<m_iWidth; i++)
                m_dcMem.SetPixelV (i,j, RGB (BYTE(r),BYTE(g),BYTE(b)));
            r+=rd; g+=gd; b+=bd;
        }
        Redraw (TRUE);
        return;
    }
    // Bitmap used : tile it in back
    CDC DCtmp;
    BITMAP tmpBitmap;

    m_bmpBack.GetBitmap (&tmpBitmap);
    int iTmpWidth = tmpBitmap.bmWidth,
        iTmpHeight = tmpBitmap.bmHeight;

    DCtmp.CreateCompatibleDC (&m_dcMem);
    m_pOldBitmap = DCtmp.SelectObject (&m_bmpBack);
    
    for (int i=0; i<m_iWidth; i++)
        for (int j=0; j<m_iHeight; j++)
            m_dcMem.SetPixelV (i,j, DCtmp.GetPixel (i % iTmpWidth, j % iTmpHeight));
    DCtmp.SelectObject (m_pOldBitmap);
    Redraw (TRUE);
}    

    
void CVectorCtl::ClearBackgroundBitmap ()
{
    if (!m_bBackgroundBitmapUsed)
        return;
    m_bmpBack.DeleteObject ();
    m_bBackgroundBitmapUsed = FALSE;
}

BOOL CVectorCtl::SetSensitivity (UINT uSens)
{
    if (uSens == 0)
        return FALSE;
    m_dSensitivity = double(uSens);
    return TRUE;
}

void CVectorCtl::Redraw (BOOL bErase) {
    m_bImageChange = TRUE;
    UpdateAxisControls();
    Invalidate (bErase);
}

void CVectorCtl::OnMouseMove(UINT nFlags, CPoint point) {

	if ( CWnd::GetCapture() != this ) {
		return;
	}

	float curX = ( float )( 2 * point.x - 64 ) / 64;
	float curY = ( float )( 2 * point.y - 64 ) / 64;

	idVec3 to( -curX, -curY, 0.0f );
	to.ProjectSelfOntoSphere( radius );
	lastPress.ProjectSelfOntoSphere( radius );

	idVec3 axis;
	axis.Cross( to, lastPress );
	float len = ( lastPress - to ).Length() / ( 2.0f * radius );
	len = idMath::ClampFloat( -1.0f, 1.0f, len );
	float phi = 2.0f * asin ( len ) ;

	axis.Normalize();
	axis *= sin( phi / 2.0f );
	idQuat rot( axis.z, axis.y, axis.x, cos( phi / 2.0f ) );
	rot.Normalize();

	rotationQuat *= rot;
	rotationQuat.Normalize();

	lastPress = to;
	lastPress.z = 0.0f;

	m_dVec = rotationQuat.ToMat3()[2];
	m_dVec.Normalize();
	Redraw();
}

void CVectorCtl::OnLButtonDown(UINT nFlags, CPoint point) {
	float curX = ( float )( 2 * point.x - 64 ) / 64;
	float curY = ( float )( 2 * point.y - 64 ) / 64;
	lastPress.Set( -curX, -curY, 0.0f );
	SetCapture();
}

void CVectorCtl::OnLButtonUp(UINT nFlags, CPoint point) {
	ReleaseCapture();
}



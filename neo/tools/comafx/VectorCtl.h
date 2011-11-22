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
/*****************************************************************************
*                                                                            *
*   Vector control                                                           *
*   ----------------                                                         *
*                                                                            *
*   A 3D vector MFC control derived from CButton.                            *
*   Features:                                                                *
*      - Real-time light rendering on a 3D ball.                             *
*      - Variable ball radius and position.                                  *
*      - Supports bitmap background (tiled).                                 *
*      - Supports vertical gradient color background (from color to color).  *
*      - Variable ball color (diffuse), light color and ambient color.       *
*      - Variable specular intensity.                                        *
*      - Supports attached controls (for automatic update).                  *
*      - Variable mouse sensitivity.                                         *
*      - Supports front clipping (vector will not have negative Z values).   *
*      - Supports callback functions for the following events:               *
*           1. The trackball has moved (vector is changing).                 *
*           2. The user dropped the trackball (released left mouse button)   *
*              i.e., the vector was changed.                                 *
*                                                                            *
*                                                                            *
*****************************************************************************/

#ifndef _VECTOR_CTL_H
#define _VECTOR_CTL_H

// Callback pointer prototype:
typedef void (*VectorCtlCallbackProc)( idQuat rotation );

// The callback should look like: 
//      void CALLBACK MyCallBack (double dVecX, double dVecY, double dVecZ);
// or 
//      static void CALLBACK MyClass::MyCallBack (double dVecX, double dVecY, double dVecZ);


class CVectorCtl : public CButton
{

#define EPS                                 1.0e-6                  // Epsilon

#define DEFAULT_VEC                         {0.00, 0.00, 1.00}      // Default start vector
#define DEFAULT_DIFFUSE                     RGB( 30,   0, 200)      // Default diffuse color
#define DEFAULT_AMBIENT                     RGB( 20,  20,  20)      // Default ambient color
#define DEFAULT_LIGHT                       RGB(200, 200, 200)      // Default light color
#define DEFAULT_START_BACKGROUND_COLOR      RGB(  0,   0,   0)      // Default gradient background start color
#define DEFAULT_END_BACKGROUND_COLOR        RGB(140,   0, 120)      // Default gradient background end color
#define DEFAULT_SPEC_EXP                    25.0                    // Default specular intensity
#define VAL_NOT_IN_USE                      -50000                  // Internal use


public:
    CVectorCtl (); 

    virtual ~CVectorCtl ();

        // Owner-drawn control support function
    virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

        // Sets / Gets diffuse (ball) color. 
    void SetDiffuseColor (COLORREF clr)                 { m_clrDiffuse = clr; Redraw (); }
    COLORREF GetDiffuseColor ()                         { return m_clrDiffuse; }

        // Sets / Gets ambient (background) color. 
    void SetAmbientColor (COLORREF clr)                 { m_clrAmbient = clr; Redraw (); }
    COLORREF GetAmbientColor ()                         { return m_clrAmbient; }

        // Sets / Gets light color.
    void SetLightColor (COLORREF clr)                   { m_clrLight = clr; Redraw (); }
    COLORREF GetLightColor ()                           { return m_clrLight; }

        // Sets background gradient color (from start to finish vertically)
    void SetBackgroundColor (COLORREF clrStart, COLORREF clrEnd);

        // Sets a background bitmap (resource ID)
    BOOL SetBackgroundImage (UINT uBackgroundBitmapID);

        // Sets / Gets specular intensity
    BOOL SetSpecularExponent (double dExp);
    double GetSpecularExponent ()                       { return m_dSpecularExponent; }

        // Enables auto-update of axis controls.
        // Place the control's ID and the SetWindowText function will be called
        // for each vector component to display the value in the control.
    void SetAxisControl (int nXCtl, int nYCtl, int nZCtl);

        // Sets / Gets ball radius (in pixels)
    void SetRadius (UINT uRadius);
    UINT GetRadius ()                                   { return UINT(m_iRadius); }

        // Sets / Gets ball position (in pixels)
    void SetCenter (UINT uHorizPos, UINT uVertPos);
    UINT GetHorizCenter ()                              { return UINT(m_iXCenter); }
    UINT GetVertCenter ()                               { return UINT(m_iYCenter); }

        // Sets / Gets vector components
    void SetX (double dx)                               { SetAxis (dx, 0);  }
    double GetX()                                       { return m_dVec[0]; }
    void SetY (double dy)                               { SetAxis (dy, 1);  }
    double GetY()                                       { return m_dVec[1]; }
    void SetZ (double dz)                               { SetAxis (dz, 2);  }
    double GetZ()                                       { return m_dVec[2]; }
    void SetVector (double dx, double dy, double dz);
	void SetidAxis( const idMat3 &mat ) {
		rotationMatrix = mat;
		rotationQuat = mat.ToQuat();
		m_dVec = mat[2];
	}

        // Sets / Gets mouse sensitivity
    BOOL SetSensitivity (UINT uSens);
    UINT GetSensitivity ()                              { return UINT(m_dSensitivity); }

        // Bounds / Unbounds vector to front (positive Z) only
    void ClipToFront (BOOL bEnable)                     { m_bFrontVector = bEnable; }
    
        // Set user-defined callback function to call whenever the vector has changed.
        // Set to NULL to disable callback.
    void SetVectorChangingCallback (VectorCtlCallbackProc proc)
        { m_procVectorChanging = proc; }

        // Set user-defined callback function to call whenever the vector has finished
        // changing (user dropped track-ball).
        // Set to NULL to disable callback.
    void SetVectorChangedCallback (VectorCtlCallbackProc proc)
        { m_procVectorChanged = proc; }

private:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

        // Mouse is being dragged
    void OnMouseDrag (int , int);   
        // Create and measure off-screen buffer
    void InitBitmap (LPDRAWITEMSTRUCT lpDrawItemStruct, CDC *pDC);
        // Build image to BitBlt
    void BuildImage (LPDRAWITEMSTRUCT lpDrawItemStruct);
        // Free resources of background (non-ball) bitmap
    void ClearBackgroundBitmap ();
        // Normalize vector
    BOOL Normalize ();
        // Calculate lightning effect for a pixel on the ball
    COLORREF CalcLight (double dx, double dy, double dz);
        // Rotate our vector by X and Y angles
    void RotateByXandY (double XRot, double YRot);
        // Create background image resource
    void CreateBackground ();
        // Force redraw of entire image
    void Redraw (BOOL bErase = FALSE);
        // Update user-defined vector components controls
    void UpdateAxisControls ();
        // Sets a specific vector component to a specific value
    void SetAxis (double d, int nAxis);

    CBitmap     m_bmpBuffer,            // Buffer bitmap for BitBlt
                m_bmpBack;              // Background image bitmap
    CDC         m_dcMem;                // Memory DC
    BOOL        m_bBmpCreated,          // Was the bitmap created ?
                m_bBackgroundBitmapUsed,// Are we using a background bitmap ?
                m_bImageChange,         // Has the image changed ?
                m_bFrontVector,         // Is the vector constrained to be facing front (positive Z) ?
                m_bHasFocus,            // Does the control have the focus ?
                m_bSelected;            // Is the control selected ?
    int         m_iWidth,               // Region width
                m_iHeight,              // Region height
                m_iRadius,              // Ball radius
                m_iSqrRadius,           // Ball radius to the power of two
                m_iXCenter,             // X center point
                m_iYCenter;             // Y center point
    CBitmap    *m_pOldBitmap;           // Previously selected bitmap    
    COLORREF    m_clrDiffuse,           // Ball diffusion color (self color)
                m_clrAmbient,           // Ambient (background) color
                m_clrLight,             // Color of light
                m_clrBackgroundStart,   // Background color gradient start
                m_clrBackgroundEnd;     // Background color gradient end
    CWnd       *pCtl[3];                // Pointers to axis display controls
    double      m_dSpecularExponent,    // Specularity effect intensity
                m_dSensitivity;         // The bigger the number the less sensitive the mouse gets
                                        // Valid ranges are 1..MAX_UINT
	idVec3		m_dVec;		            // Vector components
	idMat3		rotationMatrix;			//
	idQuat		rotationQuat;
	idQuat		previousQuat;
	idVec3		lastPress;
	float		radius;	


    VectorCtlCallbackProc   m_procVectorChanging,
                            m_procVectorChanged;

protected:
		DECLARE_MESSAGE_MAP()


};

#endif

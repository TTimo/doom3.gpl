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

#include "../../sys/win32/win_local.h"
#include "ColorButton.h"
#include "MaskEdit.h"
#include "../../sys/win32/rc/guied_resource.h"

static HHOOK	gAlphaHook = NULL;
static HWND		gAlphaDlg  = NULL;

/*
================
AlphaSlider_DrawArrow

Draws the arrow under alpha slider
================
*/
static void AlphaSlider_DrawArrow ( HDC hDC, RECT* pRect, COLORREF color )
{
	POINT ptsArrow[3];

	ptsArrow[0].x = pRect->left;
	ptsArrow[0].y = pRect->bottom;
	ptsArrow[1].x = (pRect->left + pRect->right)/2;
	ptsArrow[1].y = pRect->top;
	ptsArrow[2].x = pRect->right;
	ptsArrow[2].y = pRect->bottom;
	
	HBRUSH arrowBrush = CreateSolidBrush ( color );
	HPEN   arrowPen   = CreatePen ( PS_SOLID, 1, color );
	
	HGDIOBJ oldBrush = SelectObject ( hDC, arrowBrush );
	HGDIOBJ oldPen   = SelectObject ( hDC, arrowPen );
	
	SetPolyFillMode(hDC, WINDING);
	Polygon(hDC, ptsArrow, 3);
	
	SelectObject ( hDC, oldBrush );
	SelectObject ( hDC, oldPen );
	
	DeleteObject ( arrowBrush );
	DeleteObject ( arrowPen );
}

/*
================
AlphaSlider_WndProc

Window procedure for the alpha slider control
================
*/
LRESULT CALLBACK AlphaSlider_WndProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_LBUTTONDOWN:
		{
			RECT  rClient;
			float v;
			
			GetClientRect ( hwnd, &rClient );
			v = (float)((short)LOWORD(lParam)-5) / (float)(rClient.right - rClient.left - 10);
			if ( v < 0 ) v = 0;
			if ( v > 1.0f ) v = 1.0f;
			SetWindowLong ( hwnd, GWL_USERDATA, MAKELONG(0x8000,(unsigned short)(255.0f * v)) );
			InvalidateRect ( hwnd, NULL, FALSE );
			
			SetCapture ( hwnd );
			
			break;
		}
		
		case WM_MOUSEMOVE:		
			if ( LOWORD(GetWindowLong ( hwnd, GWL_USERDATA ) ) & 0x8000 )
			{
				RECT  rClient;
				float v;
				
				GetClientRect ( hwnd, &rClient );
				v = (float)((short)LOWORD(lParam)-5) / (float)(rClient.right - rClient.left - 10);
				if ( v < 0 ) v = 0;
				if ( v > 1.0f ) v = 1.0f;
				SetWindowLong ( hwnd, GWL_USERDATA, MAKELONG(0x8000,(unsigned short)(255.0f * v)) );
				InvalidateRect ( hwnd, NULL, FALSE );
			}
			break;

		case WM_LBUTTONUP:		
			if ( LOWORD(GetWindowLong ( hwnd, GWL_USERDATA ) ) & 0x8000 )
			{
				RECT  rClient;
				float v;
				
				GetClientRect ( hwnd, &rClient );
				v = (float)((short)LOWORD(lParam)-5) / (float)(rClient.right - rClient.left - 10);
				if ( v < 0 ) v = 0;
				if ( v > 1.0f ) v = 1.0f;
				SetWindowLong ( hwnd, GWL_USERDATA, MAKELONG(0x8000,(unsigned short)(255.0f * v)) );
				InvalidateRect ( hwnd, NULL, FALSE );
				ReleaseCapture ( );
				SendMessage ( GetParent ( hwnd ), WM_COMMAND, MAKELONG(GetWindowLong (hwnd,GWL_ID),0), 0 );
			}
			break;
	
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint ( hwnd, &ps );

			RECT rDraw;
			RECT rClient;
			GetClientRect ( hwnd, &rClient );

			// Setup the gradient rect
			CopyRect ( &rDraw, &rClient );
			rDraw.left += 5;
			rDraw.right -= 5;
			rDraw.bottom -= 6;		

			// Draw the gradient
			int parts = 20;
			RECT rColor;
			float step = (float)(rDraw.right-rDraw.left) / (float)parts;
			CopyRect ( &rColor, &rDraw );			
			for ( int i = 0; i < parts; i ++ )
			{
				float color = ((float)i / (float)parts) * 255.0f;
				
				rColor.left = rDraw.left + i * step;
				rColor.right = rColor.left + step + 1;
				
				HBRUSH brush = CreateSolidBrush ( RGB((int)color,(int)color,(int)color) );
				FillRect ( hDC, &rColor, brush );
				DeleteObject ( brush );
			}			

			// Draw a frame around the gradient
			FrameRect (hDC, &rDraw, (HBRUSH)GetStockObject ( BLACK_BRUSH ) );

			// Make sure the area below the graident is filled in
			rClient.top = rDraw.bottom;
			FillRect ( hDC, &rClient, GetSysColorBrush ( COLOR_3DFACE ) );

			// Draw the thumb
			RECT rThumb;
			short s = HIWORD(GetWindowLong ( hwnd, GWL_USERDATA ));
			float thumb = (float)(short)s;
			thumb /= 255.0f;
			thumb *= (float)(rDraw.right-rDraw.left);
			rThumb.left = rDraw.left - 5 + thumb;
			rThumb.right = rThumb.left + 10;
			rThumb.top = rDraw.bottom + 1;
			rThumb.bottom = rThumb.top + 5;
			AlphaSlider_DrawArrow ( hDC, &rThumb, RGB(0,0,0) );

			EndPaint ( hwnd, &ps );
			return 0;
		}
	}
	
	return DefWindowProc ( hwnd, msg, wParam, lParam );
}

/*
================
AlphaSelectDlg_GetMsgProc

Ensures normal dialog functions work in the alpha select dialog
================
*/
LRESULT FAR PASCAL AlphaSelectDlg_GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   LPMSG lpMsg = (LPMSG) lParam;

   if ( nCode >= 0 && PM_REMOVE == wParam )
   {
      // Don't translate non-input events.
      if ( (lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) )
      {
         if ( IsDialogMessage( gAlphaDlg, lpMsg) )
         {
            // The value returned from this hookproc is ignored, 
            // and it cannot be used to tell Windows the message has been handled.
            // To avoid further processing, convert the message to WM_NULL 
            // before returning.
            lpMsg->message = WM_NULL;
            lpMsg->lParam  = 0;
            lpMsg->wParam  = 0;
         }
      }
   }

   return CallNextHookEx(gAlphaHook, nCode, wParam, lParam);
} 

/*
================
AlphaSelectDlg_WndProc

Window procedure for the alpha select dialog
================
*/
INT_PTR CALLBACK AlphaSelectDlg_WndProc ( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_INITDIALOG:
		{		
			int color;
			
			gAlphaDlg  = hwnd;
			gAlphaHook = SetWindowsHookEx( WH_GETMESSAGE, AlphaSelectDlg_GetMsgProc, NULL, GetCurrentThreadId() );
			color      = GetRValue(ColorButton_GetColor ((HWND)lParam));

			// The lParam for the alpha select dialog is the window handle of the button pressed
			SetWindowLong ( hwnd, GWL_USERDATA, lParam );
			
			// Subclass the alpha 
			SetWindowLong ( GetDlgItem ( hwnd, IDC_GUIED_ALPHASLIDER ), GWL_USERDATA, MAKELONG(0,color) );

			// Numbers only on the edit box and start it with the current alpha value.
			NumberEdit_Attach ( GetDlgItem ( hwnd, IDC_GUIED_ALPHA ) );		
			SetWindowText ( GetDlgItem ( hwnd, IDC_GUIED_ALPHA ), va("%.3f", ((float)color / 255.0f) ) );
			break;
		}

		case WM_DESTROY:
			UnhookWindowsHookEx( gAlphaHook );
			ReleaseCapture ( );
			gAlphaDlg = NULL;
			break;		
		
		case WM_ACTIVATE:
			if ( !LOWORD(wParam) )
			{
				EndDialog ( hwnd, 0 );
			}
			break;

		case WM_COMMAND:
			switch ( LOWORD(wParam) )
			{
				case IDC_GUIED_ALPHA:
				{
					char	temp[64];
					float	value;
					
					// Get the current text in the window and convert it to a float
					GetDlgItemText ( hwnd, IDC_GUIED_ALPHA, temp, 64 );
					value = atof ( temp );
					
					if ( value < 0.0f ) 
					{
						value = 0.0f;
					}
					else if ( value > 1.0f )
					{
						value = 1.0f;
					}

					// Set the current alpha value in the slider
					SetWindowLong ( GetDlgItem ( hwnd, IDC_GUIED_ALPHASLIDER ), GWL_USERDATA, MAKELONG(0,(255.0f * value)) );
					break;
				}
					
				case IDC_GUIED_ALPHASLIDER:
				case IDOK:
				{
					int color = (short)HIWORD(GetWindowLong ( GetDlgItem ( hwnd, IDC_GUIED_ALPHASLIDER ), GWL_USERDATA ));
					ColorButton_SetColor ( (HWND)GetWindowLong ( hwnd, GWL_USERDATA ), RGB(color,color,color) );
					EndDialog ( hwnd, 0 );
					break;
				}
					
				case IDCANCEL:
					EndDialog ( hwnd, 0 );
					break;
			}
			break;			
	}

	return FALSE;
}

/*
================
AlphaButton_OpenPopup

Opens the popup window under the alpha button
================
*/
void AlphaButton_OpenPopup ( HWND button )
{
	RECT		rWindow;
	WNDCLASSEX	wndClass;			
	HWND		dlg;
	
	// Make sure the alpha slider window class is registered
	memset ( &wndClass, 0, sizeof(wndClass) );
	wndClass.cbSize			= sizeof(WNDCLASSEX);
	wndClass.lpszClassName	= "GUIED_ALPHASLIDER";		
	wndClass.lpfnWndProc	= AlphaSlider_WndProc;
	wndClass.hInstance		= win32.hInstance; 
	RegisterClassEx ( &wndClass );

	GetWindowRect ( button, &rWindow );
	dlg = CreateDialogParam ( win32.hInstance, MAKEINTRESOURCE(IDD_GUIED_ALPHA), GetParent(button), AlphaSelectDlg_WndProc, (LPARAM)button );

	SetWindowPos ( dlg, NULL, rWindow.left, rWindow.bottom + 1, 0, 0, SWP_NOSIZE|SWP_NOZORDER );
	ShowWindow ( dlg, SW_SHOW );
	UpdateWindow ( dlg );
	SetFocus ( dlg );
}

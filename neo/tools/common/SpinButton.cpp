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

#include "SpinButton.h"

void SpinButton_SetIncrement ( HWND hWnd, float inc )
{
	SetWindowLong ( hWnd, GWL_USERDATA, (long)(inc * 100.0f) );
}

void SpinButton_SetRange ( HWND hWnd, float minRange, float maxRange )
{
	SendMessage ( hWnd, UDM_SETRANGE32, (LONG)(minRange*100.0f), (LONG)(maxRange*100.0f) );	
}

void SpinButton_HandleNotify ( NMHDR* hdr )
{
	// Return if incorrect data in edit box 
	NM_UPDOWN* udhdr= (NM_UPDOWN*)hdr;

	// Change with 0.1 on each click 
	char strValue[64];
	float value;				
	GetWindowText ( (HWND)SendMessage ( hdr->hwndFrom, UDM_GETBUDDY, 0, 0 ), strValue, 63 );
	
	float inc = (float)GetWindowLong ( hdr->hwndFrom, GWL_USERDATA );
	if ( inc == 0 )
	{
		inc = 100.0f;
		SetWindowLong ( hdr->hwndFrom, GWL_USERDATA, 100 );
	}
	inc /= 100.0f;
	
	if ( GetAsyncKeyState ( VK_SHIFT ) & 0x8000 )
	{
		inc *= 10.0f;
	}
	
	value  = atof(strValue);				
	value += (udhdr->iDelta)*(inc);

	// Avoid round-off errors 
	value = floor(value*1e3+0.5)/1e3;
	
	LONG minRange;
	LONG maxRange; 
	SendMessage ( hdr->hwndFrom, UDM_GETRANGE32, (LONG)&minRange, (LONG)&maxRange );
	if ( minRange !=  0 || maxRange != 0 )
	{
		float minRangef = (float)(long)minRange / 100.0f;
		float maxRangef = (float)maxRange / 100.0f;
		if ( value > maxRangef )
		{
			value = maxRangef;
		}
		if ( value < minRangef )
		{
			value = minRangef;
		}		
	}
	
	SetWindowText ( (HWND)SendMessage ( hdr->hwndFrom, UDM_GETBUDDY, 0, 0 ), va("%g",value) );
}

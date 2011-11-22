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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

enum GameDisplayMode
{
	kInactive,
	kFullScreen,
	kWindow
};

typedef long LONG;

typedef struct tagPOINT
{
	LONG  x;
	LONG  y;
} POINT;

typedef struct 
{
	GameDisplayMode		mode;				// Indicates if the game is in full screen mode or window mode.
	CGDirectDisplayID	displayID;			// Display used for the full screen mode.
	short				width;				// Width of screen and/or window.
	short				height;				// Height of screen and/or window.
	short				depth;				// Screen bit depth used for full screen mode.
	Fixed				frequency;			// Screen refresh rate in MHz for full screen mode. If zero, then a default will be used.
	POINT				windowLoc;			// Device-local coordinate of top left corner for window mode. Expressed as a Win32 POINT. Coordiantes may be CW_USEDEFAULT indicating no location has yet been established.
	unsigned long		flags;				// kBlankingWindow, kDontRepositionWindow, etc.
	UInt32				resFlags;			// boolean bits to mark special modes for each resolution, e.g. stretched
} GameDisplayInfo;

typedef bool(*ValidModeCallbackProc)(CGDirectDisplayID displayID, int width, int height, int depth, Fixed freq);

OSStatus CreateGameDisplayPreferencesDialog(
							const GameDisplayInfo *inGDInfo,
							WindowRef *outWindow,
							ValidModeCallbackProc inCallback = NULL);

OSStatus RunGameDisplayPreferencesDialog(
							GameDisplayInfo *outGDInfo, 
							WindowRef inWindow);


#endif // PREFERENCESDIALOG_H

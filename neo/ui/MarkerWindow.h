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
#ifndef __MARKERWINDOW_H
#define __MARKERWINDOW_H

class idUserInterfaceLocal;

typedef struct {
	int time;
	const idMaterial *mat;
	idRectangle rect;
} markerData_t;

class idMarkerWindow : public idWindow {
public:
	idMarkerWindow(idUserInterfaceLocal *gui);
	idMarkerWindow(idDeviceContext *d, idUserInterfaceLocal *gui);
	virtual ~idMarkerWindow();
	virtual size_t Allocated(){return idWindow::Allocated();};
	virtual idWinVar *GetWinVarByName(const char *_name, bool winLookup = false);

	virtual const char *HandleEvent(const sysEvent_t *event, bool *updateVisuals);
	virtual void PostParse();
	virtual void Draw(int time, float x, float y);
	virtual const char *RouteMouseCoords(float xd, float yd);
	virtual void		Activate(bool activate, idStr &act);
	virtual void MouseExit();
	virtual void MouseEnter();

	
private:
	virtual bool ParseInternalVar(const char *name, idParser *src);
	void CommonInit();
	void Line(int x1, int y1, int x2, int y2, dword* out, dword color);
	void Point(int x, int y, dword *out, dword color);
	logStats_t loggedStats[MAX_LOGGED_STATS];
	idList<markerData_t> markerTimes;
	idStr statData;
	int numStats;
	dword *imageBuff;
	const idMaterial *markerMat;
	const idMaterial *markerStop;
	idVec4 markerColor;
	int currentMarker;
	int currentTime;
	int stopTime;
};

#endif // __MARKERWINDOW_H

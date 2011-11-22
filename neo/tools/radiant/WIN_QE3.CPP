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
#include "mru.h"

extern CEdit				*g_pEdit;

int		screen_width;
int		screen_height;
bool	have_quit;

int		update_bits;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Sys_MarkMapModified(void) {
	idStr title;

	if (mapModified != 1) {
		mapModified = 1;	// mark the map as changed
		title = currentmap;
		title += " *";
		title.BackSlashesToSlashes();
		Sys_SetTitle(title);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Sys_SetTitle(const char *text) {
	g_pParentWnd->SetWindowText(va("%s: %s",EDITOR_WINDOWTEXT, text));
}

/*
 =======================================================================================================================
 Wait Functions
 =======================================================================================================================
 */
HCURSOR waitcursor;

void Sys_BeginWait(void) {
	waitcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
}

bool Sys_Waiting() {
	return (waitcursor != NULL);
}

void Sys_EndWait(void) {
	if (waitcursor) {
		SetCursor(waitcursor);
		waitcursor = NULL;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Sys_GetCursorPos(int *x, int *y) {
	POINT	lpPoint;

	GetCursorPos(&lpPoint);
	*x = lpPoint.x;
	*y = lpPoint.y;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Sys_SetCursorPos(int x, int y) {
	SetCursorPos(x, y);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Sys_Beep(void) {
	MessageBeep(MB_ICONASTERISK);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
char *TranslateString(char *buf) {
	static char buf2[32768];
	int			i, l;
	char		*out;

	l = strlen(buf);
	out = buf2;
	for (i = 0; i < l; i++) {
		if (buf[i] == '\n') {
			*out++ = '\r';
			*out++ = '\n';
		}
		else {
			*out++ = buf[i];
		}
	}

	*out++ = 0;

	return buf2;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
double Sys_DoubleTime(void) {
	return clock() / 1000.0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void PrintPixels(HDC hDC) {
	int						i;
	PIXELFORMATDESCRIPTOR	p[64];

	printf("### flags color layer\n");
	for (i = 1; i < 64; i++) {
		if (!DescribePixelFormat(hDC, i, sizeof(p[0]), &p[i])) {
			break;
		}

		printf("%3i %5i %5i %5i\n", i, p[i].dwFlags, p[i].cColorBits, p[i].bReserved);
	}

	printf("%i modes\n", i - 1);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int WINAPI QEW_SetupPixelFormat(HDC hDC, bool zbuffer)
{
#if 1

	int pixelFormat = ChoosePixelFormat(hDC, &win32.pfd);
	if (pixelFormat > 0) {
		if (SetPixelFormat(hDC, pixelFormat, &win32.pfd) == NULL) {
			Error("SetPixelFormat failed.");
		}
	}
	else {
		Error("ChoosePixelFormat failed.");
	}

	return pixelFormat;
#else
	static PIXELFORMATDESCRIPTOR	pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,						// version number
		PFD_DRAW_TO_WINDOW |	// support window
		PFD_SUPPORT_OPENGL |	// support OpenGL
		PFD_DOUBLEBUFFER,		// double buffered
		PFD_TYPE_RGBA,			// RGBA type
		24,						// 24-bit color depth
		0,
		0,
		0,
		0,
		0,
		0,						// color bits ignored
		0,						// no alpha buffer
		0,						// shift bit ignored
		0,						// no accumulation buffer
		0,
		0,
		0,
		0,						// accum bits ignored
		32,						// depth bits
		0,						// no stencil buffer
		0,						// no auxiliary buffer
		PFD_MAIN_PLANE,			// main layer
		0,						// reserved
		0,
		0,
		0						// layer masks ignored
	};
	int pixelformat = 0;

	zbuffer = true;
	if (!zbuffer) {
		pfd.cDepthBits = 0;
	}

	if ((pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0) {
		printf("%d", GetLastError());
		Error("ChoosePixelFormat failed");
	}

	if (!SetPixelFormat(hDC, pixelformat, &pfd)) {
		Error("SetPixelFormat failed");
	}

	return pixelformat;
#endif
}

/*
 =======================================================================================================================
    Error For abnormal program terminations
 =======================================================================================================================
 */
void Error(char *error, ...) {
	va_list argptr;
	char	text[1024];
	char	text2[1024];
	int		err;

	err = GetLastError();

	int i = qglGetError();

	va_start(argptr, error);
	vsprintf(text, error, argptr);
	va_end(argptr);

	sprintf
	(
		text2,
		"%s\nGetLastError() = %i - %i\nAn unrecoverable error has occured. Would you like to edit Preferences before exiting Q3Radiant?",
		text,
		err,
		i
	);

	if (g_pParentWnd->MessageBox(text2, "Error", MB_YESNO) == IDYES) {
		g_PrefsDlg.LoadPrefs();
		g_PrefsDlg.DoModal();
	}

	common->FatalError( text );
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Warning(char *error, ...) {
	va_list argptr;
	char	text[1024];
	int		err;

	err = GetLastError();

	int i = qglGetError();

	va_start(argptr, error);
	vsprintf(text, error, argptr);
	va_end(argptr);

	common->Printf(text);
}

/*
 =======================================================================================================================
    FILE DIALOGS
 =======================================================================================================================
 */
bool ConfirmModified(void) {
	if (!mapModified) {
		return true;
	}

	if (g_pParentWnd->MessageBox("This will lose changes to the map", "warning", MB_OKCANCEL) == IDCANCEL) {
		return false;
	}

	return true;
}

static OPENFILENAME ofn;	/* common dialog box structure */
static char			szDirName[MAX_PATH];	/* directory string */
static char			szFile[260];			/* filename string */
static char			szFileTitle[260];		/* file title string */
static char			szFilter[260] =			/* filter string */
"Map file (*.map, *.reg)\0*.map;*.reg\0";
static char			szProjectFilter[260] =	/* filter string */
"Q3Radiant project (*.qe4, *.prj)\0*.qe4\0*.prj\0\0";
static char			chReplace;				/* string separator for szFilter */
static int			i, cbString;			/* integer count variables */
static HANDLE		hf;						/* file handle */

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void OpenDialog(void) {
	/* Obtain the system directory name and store it in szDirName. */
	strcpy(szDirName, ValueForKey(g_qeglobals.d_project_entity, "mapspath"));
	if (strlen(szDirName) == 0) {
		strcpy(szDirName, ValueForKey(g_qeglobals.d_project_entity, "basepath"));
		strcat(szDirName, "\\maps");
	}

	if (g_PrefsDlg.m_strMaps.GetLength() > 0) {
		strcat(szDirName, va("\\%s", g_PrefsDlg.m_strMaps));
	}

	/* Place the terminating null character in the szFile. */
	szFile[0] = '\0';

	/* Set the members of the OPENFILENAME structure. */
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_pParentWnd->GetSafeHwnd();
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = szDirName;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	/* Display the Open dialog box. */
	if (!GetOpenFileName(&ofn)) {
		return; // canceled
	}

	// Add the file in MRU. FIXME
	AddNewItem(g_qeglobals.d_lpMruMenu, ofn.lpstrFile);

	// Refresh the File menu. FIXME
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu, GetSubMenu(GetMenu(g_pParentWnd->GetSafeHwnd()), 0), ID_FILE_EXIT);

	/* Open the file. */
	Map_LoadFile(ofn.lpstrFile);

	g_PrefsDlg.SavePrefs();

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void ProjectDialog(void) {
	/* Obtain the system directory name and store it in szDirName. */
	strcpy(szDirName, ValueForKey(g_qeglobals.d_project_entity, "basepath"));

	/* Place the terminating null character in the szFile. */
	szFile[0] = '\0';

	/* Set the members of the OPENFILENAME structure. */
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_pParentWnd->GetSafeHwnd();
	ofn.lpstrFilter = szProjectFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = szDirName;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	/* Display the Open dialog box. */
	if (!GetOpenFileName(&ofn)) {
		return; // canceled
	}

	// Refresh the File menu.
	PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu, GetSubMenu(GetMenu(g_pParentWnd->GetSafeHwnd()), 0), ID_FILE_EXIT);

	/* Open the file. */
	if (!QE_LoadProject(ofn.lpstrFile)) {
		Error("Couldn't load project file");
	}
}

extern void AddSlash(CString &strPath);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SaveAsDialog(bool bRegion) {
	strcpy(szDirName, ValueForKey(g_qeglobals.d_project_entity, "basepath"));

	CString strPath = szDirName;
	AddSlash(strPath);
	strPath += "maps";
	if (g_PrefsDlg.m_strMaps.GetLength() > 0) {
		strPath += va("\\%s", g_PrefsDlg.m_strMaps);
	}

	/* Place the terminating null character in the szFile. */
	szFile[0] = '\0';

	/* Set the members of the OPENFILENAME structure. */
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_pParentWnd->GetSafeHwnd();
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = strPath;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

	/* Display the Open dialog box. */
	if (!GetSaveFileName(&ofn)) {
		return; // canceled
	}

	if (bRegion) {
		DefaultExtension(ofn.lpstrFile, ".reg");
	}
	else {
		DefaultExtension(ofn.lpstrFile, ".map");
	}

	if (!bRegion) {
		strcpy(currentmap, ofn.lpstrFile);
		AddNewItem(g_qeglobals.d_lpMruMenu, ofn.lpstrFile);
		PlaceMenuMRUItem(g_qeglobals.d_lpMruMenu, GetSubMenu(GetMenu(g_pParentWnd->GetSafeHwnd()), 0), ID_FILE_EXIT);
	}

	Map_SaveFile(ofn.lpstrFile, bRegion);	// ignore region
}

/*
 * Menu modifications £
 * FillBSPMenu
 */
const char	*bsp_commands[256];

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void FillBSPMenu(void) {
	HMENU		hmenu;
	int			i;
	static int	count;

	hmenu = GetSubMenu(GetMenu(g_pParentWnd->GetSafeHwnd()), MENU_BSP);

	for (i = 0; i < count; i++) {
		DeleteMenu(hmenu, CMD_BSPCOMMAND + i, MF_BYCOMMAND);
	}

	i = 0;
	count = g_qeglobals.d_project_entity->epairs.GetNumKeyVals();
	for (int j = 0; j < count; j++) {
		if (g_qeglobals.d_project_entity->epairs.GetKeyVal(j)->GetKey()[0] == 'b' && g_qeglobals.d_project_entity->epairs.GetKeyVal(j)->GetKey()[1] == 's' && g_qeglobals.d_project_entity->epairs.GetKeyVal(j)->GetKey()[2] == 'p') {
			bsp_commands[i] = g_qeglobals.d_project_entity->epairs.GetKeyVal(j)->GetKey().c_str();
			AppendMenu(hmenu, MF_ENABLED | MF_STRING, CMD_BSPCOMMAND + i, (LPCTSTR) g_qeglobals.d_project_entity->epairs.GetKeyVal(j)->GetKey().c_str());
			i++;
		}
	}

	count = i;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void AddSlash(CString &strPath) {
	if (strPath.GetLength() > 0) {
		if (strPath.GetAt(strPath.GetLength() - 1) != '\\') {
			strPath += '\\';
		}
	}
}

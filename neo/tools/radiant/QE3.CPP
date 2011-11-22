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
#include <direct.h>
#include <sys/stat.h>
#include "WaitDlg.h"

QEGlobals_t g_qeglobals;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void WINAPI QE_CheckOpenGLForErrors(void) {
	CString strMsg;
	int		i = qglGetError();
	if (i != GL_NO_ERROR) {
		if (i == GL_OUT_OF_MEMORY) {
			//
			// strMsg.Format("OpenGL out of memory error %s\nDo you wish to save before
			// exiting?", gluErrorString((GLenum)i));
			//
			if (g_pParentWnd->MessageBox(strMsg, EDITOR_WINDOWTEXT " Error", MB_YESNO) == IDYES) {
				Map_SaveFile(NULL, false);
			}

			exit(1);
		}
		else {
			// strMsg.Format("Warning: OpenGL Error %s\n ", gluErrorString((GLenum)i));
			common->Printf(strMsg.GetBuffer(0));
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool DoesFileExist(const char *pBuff, long &lSize) {
	CFile	file;
	if (file.Open(pBuff, CFile::modeRead | CFile::shareDenyNone)) {
		lSize += file.GetLength();
		file.Close();
		return true;
	}

	return false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool ExtractPath_and_Filename(const char *pPath, CString &strPath, CString &strFilename) {
	CString strPathName = pPath;
	int		nSlash = strPathName.ReverseFind('\\');
	if (nSlash >= 0) {
		strPath = strPathName.Left(nSlash + 1);
		strFilename = strPathName.Right(strPathName.GetLength() - nSlash - 1);
	}
	else {
		strFilename = pPath;
	}

	return true;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Map_Snapshot() {
	CString strMsg;

	//
	// we need to do the following 1. make sure the snapshot directory exists (create
	// it if it doesn't) 2. find out what the lastest save is based on number 3. inc
	// that and save the map
	//
	CString strOrgPath, strOrgFile;
	ExtractPath_and_Filename(currentmap, strOrgPath, strOrgFile);
	AddSlash(strOrgPath);
	strOrgPath += "snapshots";

	bool			bGo = true;
	struct _stat	Stat;
	if (_stat(strOrgPath, &Stat) == -1) {
		bGo = (_mkdir(strOrgPath) != -1);
	}

	AddSlash(strOrgPath);
	if (bGo) {
		int		nCount = 0;
		long	lSize = 0;
		CString strNewPath = strOrgPath;
		strNewPath += strOrgFile;

		CString strFile;
		while (bGo) {
			strFile.Format("%s.%i", strNewPath, nCount);
			bGo = DoesFileExist(strFile, lSize);
			nCount++;
		}

		// strFile has the next available slot
		Map_SaveFile(strFile.GetBuffer(0), false);
		Sys_SetTitle(currentmap);
		if (lSize > 12 * 1024 * 1024) { // total size of saves > 4 mb
			common->Printf
			(
				"The snapshot files in the [%s] directory total more than 4 megabytes. You might consider cleaning the directory up.",
				strOrgPath
			);
		}
	}
	else {
		strMsg.Format("Snapshot save failed.. unabled to create directory\n%s", strOrgPath);
		g_pParentWnd->MessageBox(strMsg);
	}
}

/*
 =======================================================================================================================
    QE_CheckAutoSave If five minutes have passed since making a change and the map hasn't been saved, save it out.
 =======================================================================================================================
 */
void QE_CheckAutoSave(void) {
	static bool inAutoSave = false;
	static bool autoToggle = false;
	if (inAutoSave) {
		Sys_Status("Did not autosave due recursive entry into autosave routine\n");
		return;
	}

	if ( !mapModified ) {
		return;
	}

	inAutoSave = true;

	if ( g_PrefsDlg.m_bAutoSave ) {
		CString strMsg = g_PrefsDlg.m_bSnapShots ? "Autosaving snapshot..." : "Autosaving...";
		Sys_Status(strMsg.GetBuffer(0), 0);

		if (g_PrefsDlg.m_bSnapShots && stricmp(currentmap, "unnamed.map") != 0) {
			Map_Snapshot();
		} else {
			Map_SaveFile(ValueForKey(g_qeglobals.d_project_entity, (autoToggle == 0) ? "autosave1" : "autosave2" ), false, true);
			autoToggle ^= 1;
		}
		Sys_Status("Autosaving...Saved.", 0);
		mapModified = 0;		// DHM - _D3XP
	} else {
		common->Printf("Autosave skipped...\n");
		Sys_Status("Autosave skipped...", 0);
	}

	inAutoSave = false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

const char	*g_pPathFixups[] = {
	"basepath",

	// "remotebasepath",
	"entitypath",

	// "texturepath",
	"autosave",

	// "mapspath"
};

const int	g_nPathFixupCount = sizeof(g_pPathFixups) / sizeof (const char *);

/*
 =======================================================================================================================
    QE_LoadProject
 =======================================================================================================================
 */
bool QE_LoadProject(char *projectfile) {
	char		*data;
	ID_TIME_T		time;

	common->Printf("QE_LoadProject (%s)\n", projectfile);

	if ( fileSystem->ReadFile( projectfile, reinterpret_cast < void ** > (&data), &time) <= 0 ) {
		return false;
	}

	g_strProject = projectfile;
	g_PrefsDlg.m_strLastProject = projectfile;
	g_PrefsDlg.SavePrefs();

	CString strData = data;

	fileSystem->FreeFile( data );

	StartTokenParsing(strData.GetBuffer(0));
	g_qeglobals.d_project_entity = Entity_Parse(true);
	if (!g_qeglobals.d_project_entity) {
		Error("Couldn't parse %s", projectfile);
	}

	// set here some default project settings you need
	if (strlen(ValueForKey(g_qeglobals.d_project_entity, "brush_primit")) == 0) {
		SetKeyValue(g_qeglobals.d_project_entity, "brush_primit", "0");
	}

	g_qeglobals.m_bBrushPrimitMode = IntForKey(g_qeglobals.d_project_entity, "brush_primit");

	Eclass_InitForSourceDirectory(ValueForKey(g_qeglobals.d_project_entity, "entitypath"));
	g_Inspectors->FillClassList();	// list in entity window

	Map_New();

	// FillTextureMenu();
	FillBSPMenu();

	return true;
}

/*
 =======================================================================================================================
    QE_SaveProject £
 =======================================================================================================================
 */
bool QE_SaveProject(const char *pProjectFile) {

	idFile *file = fileSystem->OpenFileWrite(pProjectFile);
	if ( !file ) {
		return false;
	}

	file->Write("{\n", 2);

	int count = g_qeglobals.d_project_entity->epairs.GetNumKeyVals();
	for (int i = 0; i < count; i++) {
		file->WriteFloatString( "\"%s\" \"%s\"\n", g_qeglobals.d_project_entity->epairs.GetKeyVal(i)->GetKey().c_str(), g_qeglobals.d_project_entity->epairs.GetKeyVal(i)->GetValue().c_str());
	}

	file->Write("}\n", 2);

	fileSystem->CloseFile( file );

	return true;
}

/* QE_KeyDown */
#define SPEED_MOVE	32
#define SPEED_TURN	22.5

/*
 =======================================================================================================================
    ConnectEntities Sets target / name on the two entities selected from the first selected to the secon
 =======================================================================================================================
 */
void ConnectEntities(void) {
	entity_t	*e1;
	const char		*target;
	idStr strTarget;
	int i, t;

	if (g_qeglobals.d_select_count < 2) {
		Sys_Status("Must have at least two brushes selected.", 0);
		Sys_Beep();
		return;
	}

	e1 = g_qeglobals.d_select_order[0]->owner;

	for (i = 0; i < g_qeglobals.d_select_count; i++) {
		if (g_qeglobals.d_select_order[i]->owner == world_entity) {
			Sys_Status("Can't connect to the world.", 0);
			Sys_Beep();
			return;
		}
	}

	for (i = 1; i < g_qeglobals.d_select_count; i++) {
		if (e1 == g_qeglobals.d_select_order[i]->owner) {
			Sys_Status("Brushes are from same entity.", 0);
			Sys_Beep();
			return;
		}
	}

	target = ValueForKey(e1, "target");
	if ( target && *target) {
		for (t = 1; t < 2048; t++) {
			target = ValueForKey(e1, va("target%i", t));
			if (target && *target) {
				continue;
			} else {
				break;
			}
		}
	} else {
		t = 0;
	}

	for (i = 1; i < g_qeglobals.d_select_count; i++) {
		target = ValueForKey(g_qeglobals.d_select_order[i]->owner, "name");
		if (target && *target) {
			strTarget = target;
		} else {
			UniqueTargetName(strTarget);
		}
		if (t == 0) {
			SetKeyValue(e1, "target", strTarget);
		} else {
			SetKeyValue(e1, va("target%i", t), strTarget);
		}
		t++;
	}

	Sys_UpdateWindows(W_XY | W_CAMERA);

	Select_Deselect();
	Select_Brush(g_qeglobals.d_select_order[1]);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool QE_SingleBrush(bool bQuiet, bool entityOK) {
	if ((selected_brushes.next == &selected_brushes) || (selected_brushes.next->next != &selected_brushes)) {
		if (!bQuiet) {
			Sys_Status("Error: you must have a single brush selected\n");
		}

		return false;
	}

	if (!entityOK && selected_brushes.next->owner->eclass->fixedsize) {
		if (!bQuiet) {
			Sys_Status("Error: you cannot manipulate fixed size entities\n");
		}

		return false;
	}

	return true;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void QE_Init(void) {
	/* initialize variables */
	g_qeglobals.d_gridsize = 8;
	g_qeglobals.d_showgrid = true;

	/*
	 * other stuff £
	 * FIXME: idMaterial Texture_Init (true); Cam_Init (); XY_Init ();
	 */
	Z_Init();
}


int g_numbrushes, g_numentities;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void QE_CountBrushesAndUpdateStatusBar(void) {
	static int	s_lastbrushcount, s_lastentitycount;
	static bool s_didonce;

	// entity_t *e;
	brush_t		*b, *next;

	g_numbrushes = 0;
	g_numentities = 0;

	if (active_brushes.next != NULL) {
		for (b = active_brushes.next; b != NULL && b != &active_brushes; b = next) {
			next = b->next;
			if (b->brush_faces) {
				if (!b->owner->eclass->fixedsize) {
					g_numbrushes++;
				}
				else {
					g_numentities++;
				}
			}
		}
	}

	/*
	 * if ( entities.next != NULL ) { for ( e = entities.next ; e != &entities &&
	 * g_numentities != MAX_MAP_ENTITIES ; e = e->next) { g_numentities++; } }
	 */
	if (((g_numbrushes != s_lastbrushcount) || (g_numentities != s_lastentitycount)) || (!s_didonce)) {
		Sys_UpdateStatusBar();

		s_lastbrushcount = g_numbrushes;
		s_lastentitycount = g_numentities;
		s_didonce = true;
	}
}


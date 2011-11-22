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

int			mapModified;			// for quit confirmation (0 = clean, 1 = unsaved,

// 2 = autosaved, but not regular saved)
char		currentmap[1024];

brush_t		active_brushes;			// brushes currently being displayed
brush_t		selected_brushes;		// highlighted

face_t		*selected_face;
brush_t		*selected_face_brush;

brush_t		filtered_brushes;		// brushes that have been filtered or regioned

entity_t	entities;				// head/tail of doubly linked list

entity_t	*world_entity = NULL;	// "classname" "worldspawn" !

void		AddRegionBrushes(void);
void		RemoveRegionBrushes(void);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void DupLists() {
	DWORD	dw = GetTickCount();
}

/*
 * Cross map selection saving this could mess this up if you have only part of a
 * complex entity selected...
 */
brush_t		between_brushes;
entity_t	between_entities;

bool		g_bRestoreBetween = false;

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Map_SaveBetween(void) {
	if (g_pParentWnd->ActiveXY()) {
		g_bRestoreBetween = true;
		g_pParentWnd->ActiveXY()->Copy();
	}

	return;

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Map_RestoreBetween(void) {
	if (g_pParentWnd->ActiveXY() && g_bRestoreBetween) {
		g_pParentWnd->ActiveXY()->Paste();
	}

	return;

}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool CheckForTinyBrush(brush_t *b, int n, float fSize) {
	bool	bTiny = false;
	for (int i = 0; i < 3; i++) {
		if (b->maxs[i] - b->mins[i] < fSize) {
			bTiny = true;
		}
	}

	if (bTiny) {
		common->Printf("Possible problem brush (too small) #%i ", n);
	}

	return bTiny;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Map_BuildBrushData(void) {
	brush_t *b, *next;

	if (active_brushes.next == NULL) {
		return;
	}

	Sys_BeginWait();	// this could take a while

	int n = 0;
	for (b = active_brushes.next; b != NULL && b != &active_brushes; b = next) {
		next = b->next;
		Brush_Build(b, true, false, false);
		if (!b->brush_faces || (g_PrefsDlg.m_bCleanTiny && CheckForTinyBrush(b, n++, g_PrefsDlg.m_fTinySize))) {
			Brush_Free(b);
			common->Printf("Removed degenerate brush\n");
		}
	}

	Sys_EndWait();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
entity_t *Map_FindClass(char *cname) {
	entity_t	*ent;

	for (ent = entities.next; ent != &entities; ent = ent->next) {
		if (!strcmp(cname, ValueForKey(ent, "classname"))) {
			return ent;
		}
	}

	return NULL;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int Map_GetUniqueEntityID(const char *prefix, const char *eclass) {
	entity_t	*ent;
	int			id = 0;
	for (ent = entities.next; ent != &entities; ent = ent->next) {
		if (!strcmp(eclass, ValueForKey(ent, "classname"))) {
			const char	*name = ValueForKey(ent, "name");
			if (name && name[0]) {
				const char *buf;
				if (prefix && *prefix) {
					buf = va("%s_%s_", prefix, eclass);
				} else {
					buf = va("%s_", eclass);
				}
				int			len = strlen(buf);
				if ( idStr::Cmpn(name, buf, len) == 0 ) {
					int j = atoi(name + len);
					if (j > id) {
						id = j;
					}
				}
			}
		}
	}

	return id + 1;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool Entity_NameIsUnique(const char *name) {
	entity_t	*ent;
	if (name == NULL) {
		return false;
	}

	for (ent = entities.next; ent != &entities; ent = ent->next) {
		const char	*testName = ValueForKey(ent, "name");
		if (testName) {
			if ( idStr::Icmp(name, testName) == 0 ) {
				return false;
			}
		}
	}

	return true;
}

/*
 =======================================================================================================================
    Map_Free
 =======================================================================================================================
 */
void Map_Free(void) {
	g_bRestoreBetween = false;
	if (selected_brushes.next && (selected_brushes.next != &selected_brushes)) {
		if (g_pParentWnd->MessageBox("Copy selection?", "", MB_YESNO) == IDYES) {
			Map_SaveBetween();
		}
	}

	// clear all the render and sound system data
	g_qeglobals.rw->InitFromMap( NULL );
	g_qeglobals.sw->ClearAllSoundEmitters();

	Texture_ClearInuse();
	Pointfile_Clear();
	strcpy(currentmap, "unnamed.map");
	Sys_SetTitle(currentmap);
	g_qeglobals.d_num_entities = 0;

	if (!active_brushes.next) { // first map
		active_brushes.prev = active_brushes.next = &active_brushes;
		selected_brushes.prev = selected_brushes.next = &selected_brushes;
		filtered_brushes.prev = filtered_brushes.next = &filtered_brushes;

		entities.prev = entities.next = &entities;
	}
	else {
		while (active_brushes.next != &active_brushes) {
			Brush_Free(active_brushes.next, false);
		}

		while (selected_brushes.next != &selected_brushes) {
			Brush_Free(selected_brushes.next, false);
		}

		while (filtered_brushes.next != &filtered_brushes) {
			Brush_Free(filtered_brushes.next, false);
		}

		while (entities.next != &entities) {
			Entity_Free(entities.next);
		}
	}

	if (world_entity) {
		Entity_Free(world_entity);
	}

	world_entity = NULL;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
entity_t *AngledEntity() {
	entity_t	*ent = Map_FindClass("info_player_start");
	if (!ent) {
		ent = Map_FindClass("info_player_deathmatch");
	}

	if (!ent) {
		ent = Map_FindClass("info_player_deathmatch");
	}

	if (!ent) {
		ent = Map_FindClass("team_CTF_redplayer");
	}

	if (!ent) {
		ent = Map_FindClass("team_CTF_blueplayer");
	}

	if (!ent) {
		ent = Map_FindClass("team_CTF_redspawn");
	}

	if (!ent) {
		ent = Map_FindClass("team_CTF_bluespawn");
	}

	return ent;
}


brush_t *BrushFromMapPatch(idMapPatch *mappatch, idVec3 origin) {
	patchMesh_t *pm = MakeNewPatch(mappatch->GetWidth(), mappatch->GetHeight());
	pm->d_texture = Texture_ForName(mappatch->GetMaterial());
	for (int i = 0; i < mappatch->GetWidth(); i++) {
		for (int j = 0; j < mappatch->GetHeight(); j++) {
			pm->ctrl(i, j).xyz = (*mappatch)[j * mappatch->GetWidth() + i].xyz + origin;
			pm->ctrl(i, j).st = (*mappatch)[j * mappatch->GetWidth() + i].st;
		}
	}
	pm->horzSubdivisions = mappatch->GetHorzSubdivisions();
	pm->vertSubdivisions = mappatch->GetVertSubdivisions();
	pm->explicitSubdivisions = mappatch->GetExplicitlySubdivided();
	if (mappatch->epairs.GetNumKeyVals()) {
		pm->epairs = new idDict;
		*pm->epairs = mappatch->epairs;
	}
	brush_t *b = AddBrushForPatch(pm, false);
	return b;
}

brush_t *BrushFromMapBrush(idMapBrush *mapbrush, idVec3 origin) {
	brush_t *b = NULL;
	if (mapbrush) {
		b = Brush_Alloc();
		int count = mapbrush->GetNumSides();
		for (int i = 0; i < count; i++) {
			idMapBrushSide *side = mapbrush->GetSide(i);
			face_t *f = Face_Alloc();
			f->next = NULL;
			if (!b->brush_faces) {
				b->brush_faces = f;
			}
			else {
				face_t	*scan;
				for (scan = b->brush_faces; scan->next; scan = scan->next) {
					;
				}
				scan->next = f;
			}
			f->plane = side->GetPlane();
			f->originalPlane = f->plane;
			f->dirty = false;

			idWinding w;
			w.BaseForPlane(f->plane);

			for (int j = 0; j < 3; j++) {
				f->planepts[j].x = w[j].x + origin.x;
				f->planepts[j].y = w[j].y + origin.y;
				f->planepts[j].z = w[j].z + origin.z;
			}

			idVec3 mat[2];
			side->GetTextureMatrix(mat[0], mat[1]);
			f->brushprimit_texdef.coords[0][0] = mat[0][0];
			f->brushprimit_texdef.coords[0][1] = mat[0][1];
			f->brushprimit_texdef.coords[0][2] = mat[0][2];
			f->brushprimit_texdef.coords[1][0] = mat[1][0];
			f->brushprimit_texdef.coords[1][1] = mat[1][1];
			f->brushprimit_texdef.coords[1][2] = mat[1][2];

			f->texdef.SetName(side->GetMaterial());
		}
	}
	return b;
}

entity_t *EntityFromMapEntity(idMapEntity *mapent, CWaitDlg *dlg) {
	entity_t *ent = NULL;
	if (mapent) {
		ent = Entity_New();
		ent->brushes.onext = ent->brushes.oprev = &ent->brushes;
		ent->origin.Zero();
		ent->epairs = mapent->epairs;
		GetVectorForKey(ent, "origin", ent->origin);
		int count = mapent->GetNumPrimitives();
		long lastUpdate = 0;
		idStr status;
		for (int i = 0; i < count; i++) {
			idMapPrimitive *prim = mapent->GetPrimitive(i);
			if (prim) {
				// update 20 times a second
				if ( (GetTickCount() - lastUpdate) > 50 ) {
					lastUpdate = GetTickCount();
					if (prim->GetType() == idMapPrimitive::TYPE_BRUSH) {
						sprintf(status, "Reading primitive %i (brush)", i);
					} else if (prim->GetType() == idMapPrimitive::TYPE_PATCH) {
						sprintf(status, "Reading primitive %i (patch)", i);
					}
					dlg->SetText(status, true);
				}
				if ( dlg->CancelPressed() ) {
					return ent;
				}

				brush_t *b = NULL;
				if (prim->GetType() == idMapPrimitive::TYPE_BRUSH) {
					idMapBrush *mapbrush = reinterpret_cast<idMapBrush*>(prim);
					b = BrushFromMapBrush(mapbrush, ent->origin);
				} else if (prim->GetType() == idMapPrimitive::TYPE_PATCH) {
					idMapPatch *mappatch = reinterpret_cast<idMapPatch*>(prim);
					b = BrushFromMapPatch(mappatch, ent->origin);
				}
				if (b) {
					b->owner = ent;
					// add to the end of the entity chain
					b->onext = &ent->brushes;
					b->oprev = ent->brushes.oprev;
					ent->brushes.oprev->onext = b;
					ent->brushes.oprev = b;
				}
			}
		}
	}
	return ent;
}

extern entity_t *Entity_PostParse(entity_t *ent, brush_t *pList);
 /*
 =======================================================================================================================
    Map_LoadFile
 =======================================================================================================================
 */
void Map_LoadFile(const char *filename) {
	entity_t *ent;
	CWaitDlg dlg;
	idStr fileStr, status;
	idMapFile mapfile;

	Sys_BeginWait();
	Select_Deselect();

	dlg.AllowCancel( true );
	idStr( filename ).ExtractFileName( fileStr );
	sprintf( status, "Loading %s...", fileStr.c_str() );
	dlg.SetWindowText( status );
	sprintf( status, "Reading file %s...", fileStr.c_str() );
	dlg.SetText( status );

	// SetInspectorMode(W_CONSOLE);
	fileStr = filename;
	fileStr.BackSlashesToSlashes();

	common->Printf( "Map_LoadFile: %s\n", fileStr.c_str() );

	Map_Free();

	g_qeglobals.d_parsed_brushes = 0;
	strcpy( currentmap, filename );

	if(mapfile.Parse(filename, true, true)) {
		g_qeglobals.bNeedConvert = false;
		g_qeglobals.bOldBrushes = false;
		g_qeglobals.bPrimitBrushes = false;
		g_qeglobals.mapVersion = 1.0;

		long lastUpdate = 0;
		int count = mapfile.GetNumEntities();
		for (int i = 0; i < count; i++) {
			idMapEntity *mapent = mapfile.GetEntity(i);
			if (mapent) {
				idStr classname = mapent->epairs.GetString("classname");
				// Update 20 times a second
				if ( (GetTickCount() - lastUpdate) > 50 ) {
					lastUpdate = GetTickCount();
					sprintf(status, "Loading entity %i (%s)...", i, classname.c_str());
					dlg.SetText(status);
				}
				if ( dlg.CancelPressed() ) {
					Sys_Status("Map load cancelled.\n");
					Map_New();
					return;
				}
				if (classname == "worldspawn") {
					world_entity = EntityFromMapEntity(mapent, &dlg);
					Entity_PostParse(world_entity, &active_brushes);
				} else {
					ent = EntityFromMapEntity(mapent, &dlg);
					Entity_PostParse(ent, &active_brushes);
					Entity_Name(ent, true);
					// add the entity to the end of the entity list
					ent->next = &entities;
					ent->prev = entities.prev;
					entities.prev->next = ent;
					entities.prev = ent;
					g_qeglobals.d_num_entities++;
				}
			}
		}
	}

	if (!world_entity) {
		Sys_Status("No worldspawn in map.\n");
		Map_New();
		return;
	}

	common->Printf("--- LoadMapFile ---\n");
	common->Printf("%s\n", fileStr.c_str());

	common->Printf("%5i brushes\n", g_qeglobals.d_parsed_brushes);
	common->Printf("%5i entities\n", g_qeglobals.d_num_entities);

	dlg.SetText("Restoring Between");
	Map_RestoreBetween();

	dlg.SetText("Building Brush Data");
	common->Printf("Map_BuildAllDisplayLists\n");
	Map_BuildBrushData();

	//
	// reset the "need conversion" flag conversion to the good format done in
	// Map_BuildBrushData
	//
	g_qeglobals.bNeedConvert = false;

	// move the view to a start position
	ent = AngledEntity();

	g_pParentWnd->GetCamera()->Camera().angles[PITCH] = 0;
	
	if (ent) {
		GetVectorForKey(ent, "origin", g_pParentWnd->GetCamera()->Camera().origin);
		GetVectorForKey(ent, "origin", g_pParentWnd->GetXYWnd()->GetOrigin());
		g_pParentWnd->GetCamera()->Camera().angles[YAW] = FloatForKey(ent, "angle");
	}
	else {
		g_pParentWnd->GetCamera()->Camera().angles[YAW] = 0;
		VectorCopy(vec3_origin, g_pParentWnd->GetCamera()->Camera().origin);
		VectorCopy(vec3_origin, g_pParentWnd->GetXYWnd()->GetOrigin());
	}

	Map_RegionOff();

	mapModified = 0;

	if (GetFileAttributes(filename) & FILE_ATTRIBUTE_READONLY) {
		fileStr += " (read only) ";
	}
	Sys_SetTitle(fileStr);

	Texture_ShowInuse();

	if (g_pParentWnd->GetCamera()->GetRenderMode()) {
		g_pParentWnd->GetCamera()->BuildRendererState();
	}

	Sys_EndWait();
	Sys_UpdateWindows(W_ALL);
}


void Map_VerifyCurrentMap(const char *map) {
	if ( idStr::Icmp( map, currentmap ) != 0 ) {
		Map_LoadFile( map );
	}
}

idMapPrimitive *BrushToMapPrimitive( const brush_t *b, const idVec3 &origin ) {
	if ( b->pPatch ) {
		idMapPatch *patch = new idMapPatch( b->pPatch->width * 6, b->pPatch->height * 6 );
		patch->SetSize( b->pPatch->width, b->pPatch->height );
		for ( int i = 0; i < b->pPatch->width; i++ ) {
			for ( int j = 0; j < b->pPatch->height; j++ ) {
				(*patch)[j*patch->GetWidth()+i].xyz =  b->pPatch->ctrl(i, j).xyz - origin;
				(*patch)[j*patch->GetWidth()+i].st = b->pPatch->ctrl(i, j).st;
			}
		}
		patch->SetExplicitlySubdivided( b->pPatch->explicitSubdivisions );
		if ( b->pPatch->explicitSubdivisions ) {
			patch->SetHorzSubdivisions( b->pPatch->horzSubdivisions );
			patch->SetVertSubdivisions( b->pPatch->vertSubdivisions );
		}
		patch->SetMaterial( b->pPatch->d_texture->GetName() );
		if ( b->pPatch->epairs ) {
			patch->epairs = *b->pPatch->epairs;
		}
		return patch;
	}
	else {
		idMapBrush *mapbrush = new idMapBrush;
		for ( face_t *f = b->brush_faces; f; f = f->next ) {
			idMapBrushSide *side = new idMapBrushSide;

			idPlane plane;
			if ( f->dirty ) {
				f->planepts[0] -= origin;
				f->planepts[1] -= origin;
				f->planepts[2] -= origin;
				plane.FromPoints( f->planepts[0], f->planepts[1], f->planepts[2], false );
				f->planepts[0] += origin;
				f->planepts[1] += origin;
				f->planepts[2] += origin;
			} else {
				plane = f->originalPlane;
			}
			side->SetPlane( plane );
			side->SetMaterial( f->d_texture->GetName() );
			idVec3 mat[2];
			mat[0][0] = f->brushprimit_texdef.coords[0][0];
			mat[0][1] = f->brushprimit_texdef.coords[0][1];
			mat[0][2] = f->brushprimit_texdef.coords[0][2];
			mat[1][0] = f->brushprimit_texdef.coords[1][0];
			mat[1][1] = f->brushprimit_texdef.coords[1][1];
			mat[1][2] = f->brushprimit_texdef.coords[1][2];
			side->SetTextureMatrix(mat);
			mapbrush->AddSide(side);
			mapbrush->epairs = b->epairs;
		}
		return mapbrush;
	}
}

idMapEntity *EntityToMapEntity(entity_t *e, bool use_region, CWaitDlg *dlg) {
	idMapEntity *mapent = new idMapEntity;
	mapent->epairs = e->epairs;
	idStr status;
	int count = 0;
	long lastUpdate = 0;
	if ( !EntityHasModel( e ) ) {
		for ( brush_t *b = e->brushes.onext; b != &e->brushes; b = b->onext ) {
			count++;					
			if ( e->eclass->fixedsize && !b->entityModel ) {
				continue;
			}
			if ( !use_region || !Map_IsBrushFiltered( b ) ) {
				// Update 20 times a second
				if ( GetTickCount() - lastUpdate > 50 ) {
					lastUpdate = GetTickCount();
					if ( b->pPatch ) {
						sprintf( status, "Adding primitive %i (patch)", count );
						dlg->SetText( status, true );
					} else {
						sprintf( status, "Adding primitive %i (brush)", count );
						dlg->SetText( status, true );
					}
				}
				idMapPrimitive *prim = BrushToMapPrimitive( b, e->origin );
				if ( prim ) {
					mapent->AddPrimitive( prim );
				}
	 		}
		 }
	}
	return mapent;
}

/*
 =======================================================================================================================
    Map_SaveFile
 =======================================================================================================================
 */
bool Map_SaveFile(const char *filename, bool use_region, bool autosave) {
	entity_t	*e, *next;
	idStr		temp;
	int			count;
	brush_t		*b;
	idStr status;

	int len = strlen(filename);
	WIN32_FIND_DATA FileData;
	if (FindFirstFile(filename, &FileData) != INVALID_HANDLE_VALUE) { 
		// the file exists;
		if (len > 0 && GetFileAttributes(filename) & FILE_ATTRIBUTE_READONLY) {
			g_pParentWnd->MessageBox("File is read only", "Read Only", MB_OK);
			return false;
		}
	}

	if (filename == NULL || len == 0 || (filename && stricmp(filename, "unnamed.map") == 0)) {
		CFileDialog dlgSave(FALSE,"map",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"Map Files (*.map)|*.map||",AfxGetMainWnd());
		if (dlgSave.DoModal() == IDOK) {
			filename = dlgSave.m_ofn.lpstrFile;
			strcpy(currentmap, filename);
		}
		else {
			return false;
		}
	}

	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	if ( statex.dwMemoryLoad > 95 ) {
		g_pParentWnd->MessageBox("Physical memory is over 95% utilized. Consider saving and restarting", "Memory");
	}

	CWaitDlg dlg;
	Pointfile_Clear();

	temp = filename;
	temp.BackSlashesToSlashes();

	if ( !use_region ) {
		idStr backup;
		backup = temp;
		backup.StripFileExtension();
		backup.SetFileExtension( ".bak" );
		if ( _unlink(backup) != 0 && errno != 2 ) { // errno 2 means the file doesn't exist, which we don't care about
			g_pParentWnd->MessageBox( va("Unable to delete %s: %s", backup.c_str(), strerror(errno) ), "File Error" );
		}

		if ( rename(filename, backup) != 0 ) {
			g_pParentWnd->MessageBox( va("Unable to rename %s to %s: %s", filename, backup.c_str(), strerror(errno) ), "File Error" );
		}
	}

	common->Printf("Map_SaveFile: %s\n", filename);

	idStr mapFile;
	bool localFile = (strstr(filename, ":") != NULL);
	if (autosave || localFile) {
		mapFile = filename;
	} else {
		mapFile = fileSystem->OSPathToRelativePath( filename );
	}

	if (use_region) {
		AddRegionBrushes();
	}

	idMapFile map;
	world_entity->origin.Zero();
	idMapEntity *mapentity = EntityToMapEntity(world_entity, use_region, &dlg);
	dlg.SetText("Saving worldspawn...");
	map.AddEntity(mapentity);

	if ( use_region ) {
		idStr buf;
		sprintf( buf, "{\n\"classname\"    \"info_player_start\"\n\"origin\"\t \"%i %i %i\"\n\"angle\"\t \"%i\"\n}\n", 
					(int)g_pParentWnd->GetCamera()->Camera().origin[0],
					(int)g_pParentWnd->GetCamera()->Camera().origin[1],
					(int)g_pParentWnd->GetCamera()->Camera().origin[2],
					(int)g_pParentWnd->GetCamera()->Camera().angles[YAW] );
		idLexer src( LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
		src.LoadMemory( buf, buf.Length(), "regionbuf" );
		idMapEntity *playerstart = idMapEntity::Parse( src );
		map.AddEntity( playerstart );
	}

	count = -1;
	for ( e = entities.next; e != &entities; e = next ) {
		count++;
		next = e->next;
		if (e->brushes.onext == &e->brushes) {
			Entity_Free(e); // no brushes left, so remove it
		}
		else {
			if (use_region) {
				for (b = e->brushes.onext; b != &e->brushes; b = b->onext) {
					if (!Map_IsBrushFiltered(b)) {
						break;	// got one
					}
				}

				if (b == &e->brushes) {
					continue;		// nothing visible
				}

			}
			idVec3 origin;
			if (!GetVectorForKey(e, "origin", origin)) {
				idStr text;
				VectorSubtract(e->brushes.onext->mins, e->eclass->mins, origin);
				sprintf(text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
				SetKeyValue(e, "origin", text);
			}

			if (use_region && !idStr::Icmp(ValueForKey(e, "classname"), "info_player_start")) {
				continue;
			} 
		
			idStr classname = e->epairs.GetString("classname");
			sprintf(status, "Saving entity %i (%s)...", count, classname.c_str());
			dlg.SetText(status);

			map.AddEntity(EntityToMapEntity(e, use_region, &dlg));
			count++;
		}
	}

	mapFile.StripFileExtension();
	idStr mapExt = (use_region) ? ".reg" : ".map";
	sprintf(status, "Writing file %s.%s...", mapFile.c_str(), mapExt.c_str());
	dlg.SetText(status);
	map.Write(mapFile, mapExt, !(autosave || localFile));
	mapModified = 0;

	if (use_region) {
		RemoveRegionBrushes();
	}

	if (!strstr(temp, "autosave")) {
		Sys_SetTitle(temp);
	}

	Sys_Status("Saved.\n", 0);

	return true;
}

/*
 =======================================================================================================================
    Map_New
 =======================================================================================================================
 */
void Map_New(void) {
	common->Printf("Map_New\n");
	Map_Free();

	Patch_Cleanup();
	g_Inspectors->entityDlg.SetEditEntity ( NULL );

	world_entity = Entity_New();
	world_entity->brushes.onext = world_entity->brushes.oprev = &world_entity->brushes;
	SetKeyValue(world_entity, "classname", "worldspawn");
	world_entity->eclass = Eclass_ForName("worldspawn", true);

	g_pParentWnd->GetCamera()->Camera().angles[YAW] = 0;
	g_pParentWnd->GetCamera()->Camera().angles[PITCH] = 0;
	VectorCopy(vec3_origin, g_pParentWnd->GetCamera()->Camera().origin);
	g_pParentWnd->GetCamera()->Camera().origin[2] = 48;
	VectorCopy(vec3_origin, g_pParentWnd->GetXYWnd()->GetOrigin());

	Map_RestoreBetween();

	Sys_UpdateWindows(W_ALL);
	mapModified = 0;

	g_qeglobals.mapVersion = MAP_VERSION;

}


bool	region_active;
idVec3	region_mins(MIN_WORLD_COORD, MIN_WORLD_COORD, MIN_WORLD_COORD);
idVec3	region_maxs(MAX_WORLD_COORD, MAX_WORLD_COORD, MAX_WORLD_COORD);

brush_t *region_sides[6];

/*
 =======================================================================================================================
    AddRegionBrushes a regioned map will have temp walls put up at the region boundary
 =======================================================================================================================
 */
void AddRegionBrushes(void) {
	idVec3		mins, maxs;
	int			i;
	texdef_t	td;

	if (!region_active) {
		return;
	}

	memset(&td, 0, sizeof(td));
	td = g_qeglobals.d_texturewin.texdef;

	// strcpy (td.name, "REGION");
	td.SetName("textures/REGION");

const int REGION_WIDTH = 1024;


	mins[0] = region_mins[0] - REGION_WIDTH;
	maxs[0] = region_mins[0] + 1;
	mins[1] = region_mins[1] - REGION_WIDTH;
	maxs[1] = region_maxs[1] + REGION_WIDTH;
	mins[2] = MIN_WORLD_COORD;
	maxs[2] = MAX_WORLD_COORD;
	region_sides[0] = Brush_Create(mins, maxs, &td);


	mins[0] = region_maxs[0] - 1;
	maxs[0] = region_maxs[0] + REGION_WIDTH;
	region_sides[1] = Brush_Create(mins, maxs, &td);

	mins[0] = region_mins[0] - REGION_WIDTH;
	maxs[0] = region_maxs[0] + REGION_WIDTH;
	mins[1] = region_mins[1] - REGION_WIDTH;
	maxs[1] = region_mins[1] + 1;
	region_sides[2] = Brush_Create(mins, maxs, &td);

	mins[1] = region_maxs[1] - 1;
	maxs[1] = region_maxs[1] + REGION_WIDTH;
	region_sides[3] = Brush_Create(mins, maxs, &td);

	mins = region_mins;
	maxs = region_maxs;
	maxs[2] = mins[2] + REGION_WIDTH;
	region_sides[4] = Brush_Create(mins, maxs, &td);

	mins = region_mins;
	maxs = region_maxs;
	mins[2] = maxs[2] - REGION_WIDTH;
	region_sides[5] = Brush_Create(mins, maxs, &td);

	for (i = 0; i < 6; i++) {
		Brush_AddToList(region_sides[i], &selected_brushes);
		Entity_LinkBrush(world_entity, region_sides[i]);
		Brush_Build(region_sides[i]);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void RemoveRegionBrushes(void) {
	int i;

	if (!region_active) {
		return;
	}

	for (i = 0; i < 6; i++) {
		Brush_Free(region_sides[i]);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool Map_IsBrushFiltered(brush_t *b) {
	int i;

	if (!region_active) {
		return false;
	}

	for (i = 0; i < 3; i++) {
		if (b->mins[i] > region_maxs[i]) {
			return true;
		}

		if (b->maxs[i] < region_mins[i]) {
			return true;
		}
	}

	return false;
}

/*
 =======================================================================================================================
    Map_RegionOff Other filtering options may still be on
 =======================================================================================================================
 */
void Map_RegionOff(void) {
	brush_t *b, *next;
	int		i;

	region_active = false;
	for (i = 0; i < 3; i++) {
		region_maxs[i] = MAX_WORLD_COORD;	// 4096;
		region_mins[i] = MIN_WORLD_COORD;	// -4096;
	}

	for (b = filtered_brushes.next; b != &filtered_brushes; b = next) {
		next = b->next;
		if (Map_IsBrushFiltered(b)) {
			continue;						// still filtered
		}

		Brush_RemoveFromList(b);
		if (active_brushes.next == NULL || active_brushes.prev == NULL) {
			active_brushes.next = &active_brushes;
			active_brushes.prev = &active_brushes;
		}

		Brush_AddToList(b, &active_brushes);
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Map_ApplyRegion(void) {
	brush_t *b, *next;

	region_active = true;
	for (b = active_brushes.next; b != &active_brushes; b = next) {
		next = b->next;
		if (!Map_IsBrushFiltered(b)) {
			continue;	// still filtered
		}

		Brush_RemoveFromList(b);
		Brush_AddToList(b, &filtered_brushes);
	}

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
    Map_RegionSelectedBrushes
 =======================================================================================================================
 */
void Map_RegionSelectedBrushes(void) {
	Map_RegionOff();

	if (selected_brushes.next == &selected_brushes) {	// nothing selected
		Sys_Status("Tried to region with no selection...\n");
		return;
	}

	region_active = true;
	Select_GetBounds(region_mins, region_maxs);

	// move the entire active_brushes list to filtered_brushes
	filtered_brushes.next = active_brushes.next;
	filtered_brushes.prev = active_brushes.prev;
	filtered_brushes.next->prev = &filtered_brushes;
	filtered_brushes.prev->next = &filtered_brushes;

	Patch_Deselect();
	// move the entire selected_brushes list to active_brushes
	active_brushes.next = selected_brushes.next;
	active_brushes.prev = selected_brushes.prev;
	active_brushes.next->prev = &active_brushes;
	active_brushes.prev->next = &active_brushes;

	// clear selected_brushes
	selected_brushes.next = selected_brushes.prev = &selected_brushes;

	Sys_UpdateWindows(W_ALL);
}

/*
 =======================================================================================================================
    Map_RegionXY
 =======================================================================================================================
 */
void Map_RegionXY(void) {
	Map_RegionOff();

	region_mins[0] = g_pParentWnd->GetXYWnd()->GetOrigin()[0] -
		0.5 *
		g_pParentWnd->GetXYWnd()->Width() /
		g_pParentWnd->GetXYWnd()->Scale();
	region_maxs[0] = g_pParentWnd->GetXYWnd()->GetOrigin()[0] +
		0.5 *
		g_pParentWnd->GetXYWnd()->Width() /
		g_pParentWnd->GetXYWnd()->Scale();
	region_mins[1] = g_pParentWnd->GetXYWnd()->GetOrigin()[1] -
		0.5 *
		g_pParentWnd->GetXYWnd()->Height() /
		g_pParentWnd->GetXYWnd()->Scale();
	region_maxs[1] = g_pParentWnd->GetXYWnd()->GetOrigin()[1] +
		0.5 *
		g_pParentWnd->GetXYWnd()->Height() /
		g_pParentWnd->GetXYWnd()->Scale();
	region_mins[2] = MIN_WORLD_COORD;
	region_maxs[2] = MAX_WORLD_COORD;
	Map_ApplyRegion();
}

/*
 =======================================================================================================================
    Map_RegionTallBrush
 =======================================================================================================================
 */
void Map_RegionTallBrush(void) {
	brush_t *b;

	if (!QE_SingleBrush()) {
		return;
	}

	b = selected_brushes.next;

	Map_RegionOff();

	VectorCopy(b->mins, region_mins);
	VectorCopy(b->maxs, region_maxs);
	region_mins[2] = MIN_WORLD_COORD;
	region_maxs[2] = MAX_WORLD_COORD;

	Select_Delete();
	Map_ApplyRegion();
}

/*
 =======================================================================================================================
    Map_RegionBrush
 =======================================================================================================================
 */
void Map_RegionBrush(void) {
	brush_t *b;

	if (!QE_SingleBrush()) {
		return;
	}

	b = selected_brushes.next;

	Map_RegionOff();

	VectorCopy(b->mins, region_mins);
	VectorCopy(b->maxs, region_maxs);

	Select_Delete();
	Map_ApplyRegion();
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void UniqueTargetName(idStr &rStr) {
	// make a unique target value
	int maxtarg = 0;
	for (entity_t * e = entities.next; e != &entities; e = e->next) {
		const char	*tn = ValueForKey(e, "name");
		if (tn && tn[0]) {
			int targetnum = atoi(tn + 1);
			if (targetnum > maxtarg) {
				maxtarg = targetnum;
			}
		}
		else {
			tn = ValueForKey(e, "target");
			if (tn && tn[0]) {
				int targetnum = atoi(tn + 1);
				if (targetnum > maxtarg) {
					maxtarg = targetnum;
				}
			}
		}
	}
	
	sprintf(rStr, "t%i", maxtarg + 1);
}

//
// =======================================================================================================================
//    Map_ImportFile Timo 09/01/99:: called by CXYWnd::Paste & Map_ImportFile if Map_ImportFile ( prefab ), the buffer
//    may contain brushes in old format ( conversion needed )
// =======================================================================================================================
//
void Map_ImportBuffer(char *buf, bool renameEntities) {
	entity_t	*ent;
	brush_t		*b = NULL;
	CPtrArray	ptrs;

	Select_Deselect();

	Undo_Start("import buffer");

	g_qeglobals.d_parsed_brushes = 0;
	if (buf) {
		CMapStringToString	mapStr;
		StartTokenParsing(buf);
		g_qeglobals.d_num_entities = 0;

		//
		// Timo will be used in Entity_Parse to detect if a conversion between brush
		// formats is needed
		//
		g_qeglobals.bNeedConvert = false;
		g_qeglobals.bOldBrushes = false;
		g_qeglobals.bPrimitBrushes = false;
		g_qeglobals.mapVersion = 1.0;

		if (GetToken(true)) {
			if (stricmp(token, "Version") == 0) {
				GetToken(false);
				g_qeglobals.mapVersion = atof(token);
				common->Printf("Map version: %1.2f\n", g_qeglobals.mapVersion);
			} else {
				UngetToken();
			}
		}

		idDict RemappedNames;	// since I can't use "map <string, string>"... sigh. So much for STL...

		while (1) {
			//
			// use the selected brushes list as it's handy ent = Entity_Parse (false,
			// &selected_brushes);
			//
			ent = Entity_Parse(false, &active_brushes);
			if (!ent) {
				break;
			}

			// end entity for undo
			Undo_EndEntity(ent);

			// end brushes for undo
			for (b = ent->brushes.onext; b && b != &ent->brushes; b = b->onext) {
				Undo_EndBrush(b);
			}

			if (!strcmp(ValueForKey(ent, "classname"), "worldspawn")) {
				// world brushes need to be added to the current world entity
				b = ent->brushes.onext;
				while (b && b != &ent->brushes) {
					brush_t *bNext = b->onext;
					Entity_UnlinkBrush(b);
					Entity_LinkBrush(world_entity, b);
					ptrs.Add(b);
					b = bNext;
				}
			}
			else {
				// the following bit remaps conflicting target/targetname key/value pairs
				CString str = ValueForKey(ent, "target");
				CString strKey;
				CString strTarget("");
				if (str.GetLength() > 0) {
					if (FindEntity("target", str.GetBuffer(0))) {
						if (!mapStr.Lookup(str, strKey)) {
							idStr key;
							UniqueTargetName(key);
							strKey = key;
							mapStr.SetAt(str, strKey);
						}

						strTarget = strKey;
						SetKeyValue(ent, "target", strTarget.GetBuffer(0));
					}
				}

				/*
				 * str = ValueForKey(ent, "name"); if (str.GetLength() > 0) { if
				 * (FindEntity("name", str.GetBuffer(0))) { if (!mapStr.Lookup(str, strKey)) {
				 * UniqueTargetName(strKey); mapStr.SetAt(str, strKey); } Entity_SetName(ent,
				 * strKey.GetBuffer(0)); } }
				 */
				CString cstrNameOld = ValueForKey(ent, "name");
				Entity_Name(ent, renameEntities);
				CString cstrNameNew = ValueForKey(ent, "name");
				if (cstrNameOld != cstrNameNew)
				{
					RemappedNames.Set(cstrNameOld, cstrNameNew);
				}
				//
				// if (strTarget.GetLength() > 0) SetKeyValue(ent, "target",
				// strTarget.GetBuffer(0));
				// add the entity to the end of the entity list
				//
				ent->next = &entities;
				ent->prev = entities.prev;
				entities.prev->next = ent;
				entities.prev = ent;
				g_qeglobals.d_num_entities++;

				for (b = ent->brushes.onext; b != &ent->brushes; b = b->onext) {
					ptrs.Add(b);
				}
			}
		}

		// now iterate through the remapped names, and see if there are any target-connections that need remaking...
		//
		// (I could probably write this in half the size with STL, but WTF, work with what we have...)
		//
		int iNumKeyVals = RemappedNames.GetNumKeyVals();
		for (int iKeyVal=0; iKeyVal < iNumKeyVals; iKeyVal++)
		{
			const idKeyValue *pKeyVal = RemappedNames.GetKeyVal( iKeyVal );

			LPCSTR psOldName = pKeyVal->GetKey().c_str();
			LPCSTR psNewName = pKeyVal->GetValue().c_str();

			entity_t *pEntOld = FindEntity("name", psOldName);	// original ent we cloned from
			entity_t *pEntNew = FindEntity("name", psNewName);	// cloned ent

			if (pEntOld && pEntNew)
			{
				CString cstrTargetNameOld = ValueForKey(pEntOld, "target");
				if (!cstrTargetNameOld.IsEmpty())
				{
					// ok, this ent was targeted at another ent, so it's clone needs updating to point to
					//	the clone of that target, so...
					//
					entity_t *pEntOldTarget = FindEntity("name", cstrTargetNameOld);
					if ( pEntOldTarget )
					{
						LPCSTR psNewTargetName = RemappedNames.GetString( cstrTargetNameOld );
						if (psNewTargetName && psNewTargetName[0])
						{								
							SetKeyValue(pEntNew, "target", psNewTargetName);
						}
					}
				}
			}
		}
	}

	//
	// ::ShowWindow(g_qeglobals.d_hwndEntity, FALSE);
	// ::LockWindowUpdate(g_qeglobals.d_hwndEntity);
	//
	g_bScreenUpdates = false;
	for (int i = 0; i < ptrs.GetSize(); i++) {
		Brush_Build(reinterpret_cast < brush_t * > (ptrs[i]), true, false);
		Select_Brush(reinterpret_cast < brush_t * > (ptrs[i]), true, false);
	}

	// ::LockWindowUpdate(NULL);
	g_bScreenUpdates = true;

	ptrs.RemoveAll();

	//
	// reset the "need conversion" flag conversion to the good format done in
	// Map_BuildBrushData
	//
	g_qeglobals.bNeedConvert = false;

	Sys_UpdateWindows(W_ALL);

	// Sys_MarkMapModified();
	mapModified = 1;

	Undo_End();
}

//
// =======================================================================================================================
//    Map_ImportFile
// =======================================================================================================================
//
void Map_ImportFile(char *fileName) {
	char	*buf;
	idStr	temp;
	Sys_BeginWait();
	temp = fileName;
	temp.BackSlashesToSlashes();
	if (LoadFile( temp, (void **) &buf) != -1) {
		Map_ImportBuffer(buf);
		Mem_Free( buf );
		Map_BuildBrushData();
	}

	Sys_UpdateWindows(W_ALL);
	mapModified = 1;
	Sys_EndWait();
}

//
// =======================================================================================================================
//    Map_SaveSelected Saves selected world brushes and whole entities with partial/full selections
// =======================================================================================================================
//
void Map_SaveSelected(char *fileName) {
	entity_t	*e, *next;
	FILE		*f;
	idStr		temp;
	int			count;

	temp = fileName;
	temp.BackSlashesToSlashes();
	f = fopen(temp, "w");

	if ( !f ) {
		common->Printf( "ERROR!!!! Couldn't open %s\n", temp.c_str() );
		return;
	}

	// write version
	g_qeglobals.mapVersion = MAP_VERSION;
	fprintf( f, "Version %1.2f\n", MAP_VERSION );

	// write world entity second
	world_entity->origin.Zero();
	Entity_WriteSelected( world_entity, f );

	// then write all other ents
	count = 1;
	for ( e = entities.next; e != &entities; e = next ) {
		fprintf( f, "// entity %i\n", count );
		count++;
		Entity_WriteSelected( e, f );
		next = e->next;
	}

	fclose( f );
}

//
// =======================================================================================================================
//    Map_SaveSelected Saves selected world brushes and whole entities with partial/full selections
// =======================================================================================================================
//
void Map_SaveSelected(CMemFile *pMemFile, CMemFile *pPatchFile) {
	entity_t	*e, *next;
	int			count;
	CString		strTemp;

	// write version
	g_qeglobals.mapVersion = MAP_VERSION;
	MemFile_fprintf(pMemFile, "Version %1.2f\n", MAP_VERSION);

	// write world entity first
	world_entity->origin.Zero();
	Entity_WriteSelected(world_entity, pMemFile);

	// then write all other ents
	count = 1;
	for (e = entities.next; e != &entities; e = next) {
		MemFile_fprintf(pMemFile, "// entity %i\n", count);
		count++;
		Entity_WriteSelected(e, pMemFile);
		next = e->next;
	}

	// if (pPatchFile) Patch_WriteFile(pPatchFile);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

/*
================
WriteFileString
================
*/
bool WriteFileString( FILE *fp, char *string, ... ) {
	long i;
	unsigned long u;
	double f;
	char *str;
	idStr buf;
	va_list argPtr;

	va_start( argPtr, string );

	while( *string ) {
		switch( *string ) {
			case '%':
				string++;
				while ( (*string >= '0' && *string <= '9') ||
						*string == '.' || *string == '-' || *string == '+' || *string == '#') {
					string++;
				}
				switch( *string ) {
					case 'f':
					case 'e':
					case 'E':
					case 'g':
					case 'G':
						f = va_arg( argPtr, double );
						sprintf( buf, "%1.10f", f );
						buf.StripTrailing( '0' );
						buf.StripTrailing( '.' );
						fprintf( fp, "%s", buf.c_str() );
						break;
					case 'd':
					case 'i':
						i = va_arg( argPtr, long );
						fprintf( fp, "%d", i );
						break;
					case 'u':
						u = va_arg( argPtr, unsigned long );
						fprintf( fp, "%u", u );
						break;
					case 'o':
						u = va_arg( argPtr, unsigned long );
						fprintf( fp, "%o", u );
						break;
					case 'x':
						u = va_arg( argPtr, unsigned long );
						fprintf( fp, "%x", u );
						break;
					case 'X':
						u = va_arg( argPtr, unsigned long );
						fprintf( fp, "%X", u );
						break;
					case 'c':
						i = va_arg( argPtr, long );
						fprintf( fp, "%c", (char) i );
						break;
					case 's':
						str = va_arg( argPtr, char * );
						fprintf( fp, "%s", str );
						break;
					case '%':
						fprintf( fp, "%%" );
						break;
					default:
						common->Error( "WriteFileString: invalid %%%c", *string );
						break;
				}
				string++;
				break;
			case '\\':
				string++;
				switch( *string ) {
					case 't':
						fprintf( fp, "\t" );
						break;
					case 'n':
						fprintf( fp, "\n" );
					default:
						common->Error( "WriteFileString: unknown escape character \'%c\'", *string );
						break;
				}
				string++;
				break;
			default:
				fprintf( fp, "%c", *string );
				string++;
				break;
		}
	}

	va_end( argPtr );

	return true;
}

/*
================
MemFile_fprintf
================
*/
void MemFile_fprintf( CMemFile *pMemFile, const char *string, ... ) {
	char	Buffer[4096];
	long i;
	unsigned long u;
	double f;
	char *str;
	idStr buf, out;
	va_list argPtr;

	char *buff = Buffer;

	va_start( argPtr, string );

	while( *string ) {
		switch( *string ) {
			case '%':
				string++;
				while ( (*string >= '0' && *string <= '9') ||
						*string == '.' || *string == '-' || *string == '+' || *string == '#') {
					string++;
				}
				switch( *string ) {
					case 'f':
					case 'e':
					case 'E':
					case 'g':
					case 'G':
						f = va_arg( argPtr, double );
						sprintf( buf, "%1.10f", f );
						buf.StripTrailing( '0' );
						buf.StripTrailing( '.' );
						sprintf( buff, "%s", buf.c_str() );
						break;
					case 'd':
					case 'i':
						i = va_arg( argPtr, long );
						sprintf( buff, "%d", i );
						break;
					case 'u':
						u = va_arg( argPtr, unsigned long );
						sprintf( buff, "%u", u );
						break;
					case 'o':
						u = va_arg( argPtr, unsigned long );
						sprintf( buff, "%o", u );
						break;
					case 'x':
						u = va_arg( argPtr, unsigned long );
						sprintf( buff, "%x", u );
						break;
					case 'X':
						u = va_arg( argPtr, unsigned long );
						sprintf( buff, "%X", u );
						break;
					case 'c':
						i = va_arg( argPtr, long );
						sprintf( buff, "%c", (char) i );
						break;
					case 's':
						str = va_arg( argPtr, char * );
						sprintf( buff, "%s", str );
						break;
					case '%':
						sprintf( buff, "%%" );
						break;
					default:
						common->Error( "MemFile_fprintf: invalid %%%c", *string );
						break;
				}
				string++;
				break;
			case '\\':
				string++;
				switch( *string ) {
					case 't':
						sprintf( buff, "\t" );
						break;
					case 'n':
						sprintf( buff, "\n" );
					default:
						common->Error( "MemFile_fprintf: unknown escape character \'%c\'", *string );
						break;
				}
				string++;
				break;
			default:
				sprintf( buff, "%c", *string );
				string++;
				break;
		}

		buff = Buffer + strlen(Buffer);
	}

	va_end( argPtr );

	out = Buffer;
	pMemFile->Write( out.c_str(), out.Length() );
}

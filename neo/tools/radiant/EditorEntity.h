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

void		Eclass_InitForSourceDirectory( const char *path );
eclass_t *	Eclass_ForName( const char *name, bool has_brushes );
bool		Eclass_hasModel(eclass_t *e, idVec3 &vMin, idVec3 &vMax);

typedef struct entity_s {
	struct entity_s	*prev, *next;
	brush_t		brushes;					// head/tail of list
	int			undoId, redoId, entityId;	// used for undo/redo
	idVec3		origin;
	qhandle_t	lightDef;
	qhandle_t	modelDef;
	idSoundEmitter *soundEmitter;
	eclass_t *	eclass;
	idDict		epairs;
	eclass_t *	md3Class;
	idMat3		rotation;
	idVec3 		lightOrigin;		// for lights that have been combined with models
	idMat3		lightRotation;		// ''
	bool		trackLightOrigin;	
	idCurve<idVec3> *curve;
} entity_t;

void		ParseEpair(idDict *dict);
const char *ValueForKey(entity_t *ent, const char *key);
int			GetNumKeys(entity_t *ent);
const char *GetKeyString(entity_t *ent, int iIndex);
void		SetKeyValue (entity_t *ent, const char *key, const char *value, bool trackAngles = true);
void		DeleteKey (entity_t *ent, const char *key);
float		FloatForKey (entity_t *ent, const char *key);
int			IntForKey (entity_t *ent, const char *key);
bool		GetVectorForKey (entity_t *ent, const char *key, idVec3 &vec);
bool		GetVector4ForKey (entity_t *ent, const char *key, idVec4 &vec);
bool		GetFloatForKey(entity_t *end, const char *key, float *f);
void		SetKeyVec3(entity_t *ent, const char *key, idVec3 v);
void		SetKeyMat3(entity_t *ent, const char *key, idMat3 m);
bool		GetMatrixForKey(entity_t *ent, const char *key, idMat3 &mat);

void		Entity_UpdateSoundEmitter( entity_t *ent );
idCurve<idVec3> *Entity_MakeCurve( entity_t *e );
void		Entity_UpdateCurveData( entity_t *e );
void		Entity_SetCurveData( entity_t *e );
void		Entity_Free (entity_t *e);
void		Entity_FreeEpairs(entity_t *e);
int			Entity_MemorySize(entity_t *e);
entity_t *	Entity_Parse (bool onlypairs, brush_t* pList = NULL);
void		Entity_Write (entity_t *e, FILE *f, bool use_region);
void		Entity_WriteSelected(entity_t *e, FILE *f);
void		Entity_WriteSelected(entity_t *e, CMemFile*);
entity_t *	Entity_Create (eclass_t *c, bool forceFixed = false);
entity_t *	Entity_Clone (entity_t *e);
void		Entity_AddToList(entity_t *e, entity_t *list);
void		Entity_RemoveFromList(entity_t *e);
bool		EntityHasModel(entity_t *ent);

void		Entity_LinkBrush (entity_t *e, brush_t *b);
void		Entity_UnlinkBrush (brush_t *b);
entity_t *	FindEntity(const char *pszKey, const char *pszValue);
entity_t *	FindEntityInt(const char *pszKey, int iValue);
entity_t *	Entity_New();
void		Entity_SetName(entity_t *e, const char *name);

int			GetUniqueTargetId(int iHint);
eclass_t *	GetCachedModel(entity_t *pEntity, const char *pName, idVec3 &vMin, idVec3 &vMax);

//Timo : used for parsing epairs in brush primitive
void		Entity_Name(entity_t *e, bool force);

bool		IsBrushSelected(brush_t* bSel);

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
#include "io.h"
#include "../../renderer/tr_local.h"

struct evarPrefix_t {
	int type;
	const char *prefix;
};

const evarPrefix_t EvarPrefixes[] = {
	{ EVAR_STRING,  "editor_var " },
	{ EVAR_INT,		"editor_int " },
	{ EVAR_FLOAT,	"editor_float " },
	{ EVAR_BOOL,	"editor_bool " },
	{ EVAR_COLOR,	"editor_color " },
	{ EVAR_MATERIAL,"editor_mat " },
	{ EVAR_MODEL,	"editor_model " },
	{ EVAR_GUI,		"editor_gui " },
	{ EVAR_SOUND,	"editor_snd "}
};

const int NumEvarPrefixes = sizeof(EvarPrefixes) / sizeof(evarPrefix_t);

eclass_t	*eclass = NULL;
eclass_t	*eclass_bad = NULL;
char		eclass_directory[1024];

// md3 cache for misc_models
eclass_t *g_md3Cache = NULL;

/*

the classname, color triple, and bounding box are parsed out of comments
A ? size means take the exact brush size.

/*QUAKED <classname> (0 0 0) ?
/*QUAKED <classname> (0 0 0) (-8 -8 -8) (8 8 8)

Flag names can follow the size description:

/*QUAKED func_door (0 .5 .8) ? START_OPEN STONE_SOUND DOOR_DONT_LINK GOLD_KEY SILVER_KEY

*/

void CleanEntityList( eclass_t *&pList ) {
	while (pList) {
		eclass_t* pTemp = pList->next;
		delete pList;
		pList = pTemp;
	}
	pList = NULL;
}


void CleanUpEntities()
{
	CleanEntityList(eclass);
	CleanEntityList(g_md3Cache);

	if ( eclass_bad ) {
		delete eclass_bad;
		eclass_bad = NULL;
	}
}

void ExtendBounds(idVec3 v, idVec3 &vMin, idVec3 &vMax)
{
	for (int i = 0 ;i < 3 ;i++)
	{
		float	f = v[i];
		
    if (f < vMin[i])
    {
			vMin[i] = f;
    }

    if (f > vMax[i])
    {
			vMax[i] = f;
    }
	}
}

bool LoadModel(const char *pLocation, eclass_t *e, idVec3 &vMin, idVec3 &vMax, const char *pSkin)
{
	vMin[0] = vMin[1] = vMin[2] = 999999;
	vMax[0] = vMax[1] = vMax[2] = -999999;

	if (strstr(pLocation, ".ase") != NULL)	// FIXME: not correct!
	{
		idBounds b;
		e->modelHandle = renderModelManager->FindModel( pLocation );
		b = e->modelHandle->Bounds( NULL );
		VectorCopy(b[0], vMin);
		VectorCopy(b[1], vMax);
		return true;
	}
	return false;
}

eclass_t *EClass_Alloc( void ) {
	eclass_t *e;
	e = new eclass_t;
	if ( e == NULL ) {
		return NULL;
	}
	e->fixedsize = false;
	e->unknown = false;
	e->mins.Zero();
	e->maxs.Zero();
	e->color.Zero();
	memset( &e->texdef, 0, sizeof( e->texdef ) );
	e->modelHandle = NULL;
	e->entityModel = NULL;
	e->nFrame = 0;
	e->nShowFlags = 0;
	e->hPlug = 0;
	e->next = NULL;
	return e;
}


eclass_t *EClass_InitFromDict( const idDict *d, const char *name ) {
	eclass_t			*e;
	const idKeyValue	*kv;

	// only include entityDefs with "editor_" values in them
	if ( !d->MatchPrefix( "editor_" ) ) {
		return NULL;
	}

	e = EClass_Alloc();
	if ( !e ) {
		return NULL;
	}

	e->defArgs = *d;

	idStr str;
	idStr text;
	idStr varname;
	idStr defaultStr;

	e->name = name;
	d->GetVector("editor_color", "0 0 1", e->color);

	d->GetString("editor_mins", "", str);
	if (str != "?") {
		d->GetVector("editor_mins", "0 0 0", e->mins);
		d->GetVector("editor_maxs", "0 0 0", e->maxs);
		e->fixedsize = true;
	} else {
		e->fixedsize = false;
	}


	d->GetString("editor_material", "", e->defMaterial);

	//str = d->GetString("model");
	//if (str.Length()) {
	//	e->entityModel = renderModelManager->FindModel(str);
	//}
	
	str = "";

	// concatenate all editor usage comments
	text = "";
	kv = d->MatchPrefix( "editor_usage" );
	while( kv != NULL ) {
		text += kv->GetValue();
		if ( !kv->GetValue().Length() || ( text[ text.Length() - 1 ] != '\n' ) ) {
			text += "\n";
		}
		kv = d->MatchPrefix( "editor_usage", kv );
	}

	e->desc = text;

	str += "Spawn args:\n";
	for (int i = 0; i < NumEvarPrefixes; i++) {
		kv = d->MatchPrefix(EvarPrefixes[i].prefix);
		while (kv) {
			evar_t ev;
			kv->GetKey().Right( kv->GetKey().Length() - strlen(EvarPrefixes[i].prefix), ev.name );
			ev.desc = kv->GetValue();
			ev.type = EvarPrefixes[i].type;
			e->vars.Append(ev);
			kv = d->MatchPrefix(EvarPrefixes[i].prefix, kv);
		}
	}

/*
	while( kv != NULL ) {
		kv->key.Right( kv->key.Length() - 11, varname );
		str += va( "'%s':\t %s", varname.c_str(), kv->value.c_str() );
		if ( d->GetString( varname, "", defaultStr ) && defaultStr.Length() ) {
			str += va( "  Default '%s'.", defaultStr.c_str() );
		}
		str += "\n";
		kv = d->MatchPrefix( "editor_var ", kv );
	}

	e->comments = Mem_CopyString( str.c_str() );
*/

	
	// concatenate all variable comments
	kv = d->MatchPrefix( "editor_copy" );
	while (kv) {
		const char *temp = d->GetString(kv->GetValue());
		if (temp && *temp) {
			e->args.Set(kv->GetValue(), d->GetString(kv->GetValue()));
		}
		kv = d->MatchPrefix("editor_copy", kv);
	}

	// setup show flags
	e->nShowFlags = 0;
	if (d->GetBool("editor_rotatable")) {
		e->nShowFlags |= ECLASS_ROTATABLE;
	}

	if (d->GetBool("editor_showangle")) {
		e->nShowFlags |= ECLASS_ANGLE;
	}

	if (d->GetBool("editor_mover")) {
		e->nShowFlags |= ECLASS_MOVER;
	}

	if (d->GetBool("editor_env") || idStr::Icmpn(e->name, "env_", 4) == 0) {
		e->nShowFlags |= (ECLASS_ENV | ECLASS_ROTATABLE);
		if (d->GetBool("editor_ragdoll")) {
			e->defArgs.Set("model", "");
		}
	}

	if (d->GetBool("editor_combatnode")) {
		e->nShowFlags |= ECLASS_COMBATNODE;
	}

	if (d->GetBool("editor_light")) {
		e->nShowFlags |= ECLASS_LIGHT;
	}

	if ( idStr::Icmp(e->name, "light") == 0 ) {
		e->nShowFlags |= ECLASS_LIGHT;
	} else if ( idStr::Icmp(e->name, "path") == 0 ) {
		e->nShowFlags |= ECLASS_PATH;
	} else if ( idStr::Icmp(e->name, "target_null") == 0 ) {
		e->nShowFlags |= ECLASS_CAMERAVIEW;
	} else if ( idStr::Icmp(e->name, "worldspawn") == 0 ) {
		e->nShowFlags |= ECLASS_WORLDSPAWN;
	} else if ( idStr::Icmp(e->name, "speaker") == 0 ) {
		e->nShowFlags |= ECLASS_SPEAKER;
	} else if ( idStr::Icmp( e->name, "func_emitter" ) == 0 || idStr::Icmp( e->name, "func_splat" ) == 0 ) {
		e->nShowFlags |= ECLASS_PARTICLE;
	} else if ( idStr::Icmp(e->name, "func_liquid") == 0 ) {
		e->nShowFlags |= ECLASS_LIQUID;
	} 

	return e;
}

void EClass_InsertSortedList(eclass_t *&pList, eclass_t *e)
{
	eclass_t	*s;
	
	if (!pList)
	{
		pList = e;
		return;
	}


	s = pList;
	if (stricmp (e->name, s->name) < 0)
	{
		e->next = s;
		pList = e;
		return;
	}

	do
	{
		if (!s->next || stricmp (e->name, s->next->name) < 0)
		{
			e->next = s->next;
			s->next = e;
			return;
		}
		s=s->next;
	} while (1);
}

/*
=================
Eclass_InsertAlphabetized
=================
*/
void Eclass_InsertAlphabetized (eclass_t *e)
{
#if 1
  EClass_InsertSortedList(eclass, e);
#else
	eclass_t	*s;
	
	if (!eclass)
	{
		eclass = e;
		return;
	}


	s = eclass;
	if (stricmp (e->name, s->name) < 0)
	{
		e->next = s;
		eclass = e;
		return;
	}

	do
	{
		if (!s->next || stricmp (e->name, s->next->name) < 0)
		{
			e->next = s->next;
			s->next = e;
			return;
		}
		s=s->next;
	} while (1);
#endif
}


void Eclass_InitForSourceDirectory (const char *path)
{
	int c = declManager->GetNumDecls(DECL_ENTITYDEF);
	for (int i = 0; i < c; i++) {
		const idDeclEntityDef *def = static_cast<const idDeclEntityDef *>( declManager->DeclByIndex( DECL_ENTITYDEF, i ) );
		if ( def ) {
			eclass_t *e = EClass_InitFromDict( &def->dict, def->GetName() );
			if ( e ) {
				Eclass_InsertAlphabetized (e);
			}
		}
	}

	eclass_bad = EClass_Alloc();
	if ( !eclass_bad ) {
		return;
	}
	eclass_bad->color.x = 0.0f;
	eclass_bad->color.y = 0.5f;
	eclass_bad->color.z = 0.0f;
	eclass_bad->fixedsize = false;
	eclass_bad->name = Mem_CopyString( "UKNOWN ENTITY CLASS" );
}

eclass_t *Eclass_ForName (const char *name, bool has_brushes)
{
	eclass_t	*e;
	char buff[1024];

	if (!name) {
		return eclass_bad;
	}

	for ( e = eclass; e; e = e->next ) {
		if ( !strcmp( name, e->name ) ) {
			return e;
		}
	}

	e = EClass_Alloc();
	if ( !e ) {
		return NULL;
	}
	e->name = Mem_CopyString( name );
	sprintf(buff, "%s not found in def/*.def\n", name);
	e->comments = Mem_CopyString( buff );
	e->color.x = 0.0f;
	e->color.y = 0.5f;
	e->color.z = 0.0f;
	e->fixedsize = !has_brushes;
	e->mins.x = e->mins.y = e->mins.z = -8.0f;
	e->maxs.x = e->maxs.y = e->maxs.z = 8.0f;
	Eclass_InsertAlphabetized( e );

	return e;
}


eclass_t* GetCachedModel(entity_t *pEntity, const char *pName, idVec3 &vMin, idVec3 &vMax)
{
	eclass_t *e = NULL;
	if (pName == NULL || strlen(pName) == 0) {
		return NULL;
	}

	for (e = g_md3Cache; e ; e = e->next) {
		if (!strcmp (pName, e->name)) {
			pEntity->md3Class = e;
			VectorCopy(e->mins, vMin);
			VectorCopy(e->maxs, vMax);
			return e;
	    }
	}

	e = (eclass_t*)Mem_ClearedAlloc(sizeof(*e));
	memset (e, 0, sizeof(*e));
	e->name = Mem_CopyString( pName );
	e->color[0] = e->color[2] = 0.85f;
	if (LoadModel(pName, e, vMin, vMax, NULL)) {
		EClass_InsertSortedList(g_md3Cache, e);
		VectorCopy(vMin, e->mins);
		VectorCopy(vMax, e->maxs);
		pEntity->md3Class = e;
		return e;
	}
	return NULL;
}

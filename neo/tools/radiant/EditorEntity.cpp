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
#include "../../renderer/tr_local.h"
#include "../../renderer/model_local.h"	// for idRenderModelMD5
int g_entityId = 1;

#define CURVE_TAG "curve_"

extern void Brush_Resize(brush_t *b, idVec3 vMin, idVec3 vMax);

int	GetNumKeys(entity_t *ent)
{
//	int iCount = 0;
//	for (epair_t* ep=ent->epairs ; ep ; ep=ep->next)
//	{
//		iCount++;
//	}

	int iCount = ent->epairs.GetNumKeyVals();
	return iCount;
}

const char *GetKeyString(entity_t *ent, int iIndex)
{
//	for (epair_t* ep=ent->epairs ; ep ; ep=ep->next)
//	{
//		if (!iIndex--)
//			return ep->key;
//	}
//
//	assert(0);
//	return NULL;
	
	if ( iIndex < GetNumKeys(ent) )
	{
		return ent->epairs.GetKeyVal(iIndex)->GetKey().c_str();
	}

	assert(0);
	return NULL;
}


/*
 =======================================================================================================================
 =======================================================================================================================
 */
const char *ValueForKey(entity_t *ent, const char *key) {
	return ent->epairs.GetString(key);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void TrackMD3Angles(entity_t *e, const char *key, const char *value) {
	if ( idStr::Icmp(key, "angle") != 0 ) {
		return;
	}

	if ((e->eclass->fixedsize && e->eclass->nShowFlags & ECLASS_MISCMODEL) || EntityHasModel(e)) {
		float	a = FloatForKey(e, "angle");
		float	b = atof(value);
		if (a != b) {
			idVec3	vAngle;
			vAngle[0] = vAngle[1] = 0;
			vAngle[2] = -a;
			Brush_Rotate(e->brushes.onext, vAngle, e->origin, true);
			vAngle[2] = b;
			Brush_Rotate(e->brushes.onext, vAngle, e->origin, true);
		}
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetKeyValue(entity_t *ent, const char *key, const char *value, bool trackAngles) {
	if (ent == NULL) {
		return;
	}

	if (!key || !key[0]) {
		return;
	}

	if (trackAngles) {
		TrackMD3Angles(ent, key, value);
	}

	ent->epairs.Set(key, value);
	GetVectorForKey(ent, "origin", ent->origin);

	// update sound in case this key was relevent
	Entity_UpdateSoundEmitter( ent );
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetKeyVec3(entity_t *ent, const char *key, idVec3 v) {
	if (ent == NULL) {
		return;
	}

	if (!key || !key[0]) {
		return;
	}

	idStr str;
	sprintf(str, "%g %g %g", v.x, v.y, v.z);
	ent->epairs.Set(key, str);
	GetVectorForKey(ent, "origin", ent->origin);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void SetKeyMat3(entity_t *ent, const char *key, idMat3 m) {
	if (ent == NULL) {
		return;
	}

	if (!key || !key[0]) {
		return;
	}

	idStr str;
	
	sprintf(str, "%g %g %g %g %g %g %g %g %g",m[0][0],m[0][1],m[0][2],m[1][0],m[1][1],m[1][2],m[2][0],m[2][1],m[2][2]);
	
	ent->epairs.Set(key, str);
	GetVectorForKey(ent, "origin", ent->origin);
}



/*
 =======================================================================================================================
 =======================================================================================================================
 */
void DeleteKey(entity_t *ent, const char *key) {
	ent->epairs.Delete(key);
	if (stricmp(key, "rotation") == 0) {
		ent->rotation.Identity();
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
float FloatForKey(entity_t *ent, const char *key) {
	const char	*k;

	k = ValueForKey(ent, key);
	if (k && *k) {
		return atof(k);
	}

	return 0.0;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int IntForKey(entity_t *ent, const char *key) {
	const char	*k;

	k = ValueForKey(ent, key);
	return atoi(k);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool GetVectorForKey(entity_t *ent, const char *key, idVec3 &vec) {
	const char	*k;
	k = ValueForKey(ent, key);
	if (k && strlen(k) > 0) {
		sscanf(k, "%f %f %f", &vec[0], &vec[1], &vec[2]);
		return true;
	}
	else {
		vec[0] = vec[1] = vec[2] = 0;
	}

	return false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool GetVector4ForKey(entity_t *ent, const char *key, idVec4 &vec) {
	const char	*k;
	k = ValueForKey(ent, key);
	if (k && strlen(k) > 0) {
		sscanf(k, "%f %f %f %f", &vec[0], &vec[1], &vec[2], &vec[3]);
		return true;
	}
	else {
		vec[0] = vec[1] = vec[2] = vec[3] = 0;
	}

	return false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool GetFloatForKey(entity_t *ent, const char *key, float *f) {
	const char	*k;
	k = ValueForKey(ent, key);
	if (k && strlen(k) > 0) {
		*f = atof(k);
		return true;
	}

	*f = 0;
	return false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool GetMatrixForKey(entity_t *ent, const char *key, idMat3 &mat) {
	const char	*k;
	k = ValueForKey(ent, key);
	if (k && strlen(k) > 0) {
		sscanf
		(
			k,
			"%f %f %f %f %f %f %f %f %f ",
			&mat[0][0],
			&mat[0][1],
			&mat[0][2],
			&mat[1][0],
			&mat[1][1],
			&mat[1][2],
			&mat[2][0],
			&mat[2][1],
			&mat[2][2]
		);
		return true;
	}
	else {
		mat.Identity();
	}

	return false;
}

/*
 =======================================================================================================================
    Entity_FreeEpairs Frees the entity epairs.
 =======================================================================================================================
 */
void Entity_FreeEpairs(entity_t *e) {
	e->epairs.Clear();
}

/*
 =======================================================================================================================
    Entity_AddToList
 =======================================================================================================================
 */
void Entity_AddToList(entity_t *e, entity_t *list) {
	if (e->next || e->prev) {
		Error("Entity_AddToList: allready linked");
	}

	e->next = list->next;
	list->next->prev = e;
	list->next = e;
	e->prev = list;
}

/*
 =======================================================================================================================
    Entity_RemoveFromList
 =======================================================================================================================
 */
void Entity_RemoveFromList(entity_t *e) {
	if ( !e->next || !e->prev ) {
		Error("Entity_RemoveFromList: not linked");
	}

	e->next->prev = e->prev;
	e->prev->next = e->next;
	e->next = e->prev = NULL;
}

/*
 =======================================================================================================================
    Entity_Free Frees the entity and any brushes is has. The entity is removed from the global entities list.
 =======================================================================================================================
 */
void Entity_Free( entity_t *e ) {

	while ( e->brushes.onext != &e->brushes ) {
		Brush_Free(e->brushes.onext);
	}

	if ( e->next ) {
		e->next->prev = e->prev;
		e->prev->next = e->next;
	}

	Entity_FreeEpairs( e );

	delete e;
}

/*
 =======================================================================================================================
    Entity_MemorySize
 =======================================================================================================================
 */

int Entity_MemorySize( entity_t *e ) 
{
	brush_t		*b;
	int			size;

	size = sizeof( entity_t ) + e->epairs.Size();
	for( b = e->brushes.onext; b != &e->brushes; b = b->onext )
	{
		size += Brush_MemorySize( b );
}
	return( size );
}

/*
 =======================================================================================================================
    ParseEpair
 =======================================================================================================================
 */

struct EpairFixup {
	const char *name;
	int type;
};


const EpairFixup FloatFixups[] = {
	{ "origin", 1 },
	{ "rotation", 2 },
	{ "_color", 1 },
	{ "falloff", 0 },
	{ "light", 0 },
	{ "light_target", 1 },
	{ "light_up", 1 },
	{ "light_right", 1 },
	{ "light_start", 1 },
	{ "light_center", 1 },
	{ "light_end", 1 },
	{ "light_radius", 1 },
	{ "light_origin", 1 }
};

const int FixupCount = sizeof(FloatFixups) / sizeof(EpairFixup);

void FixFloats(idDict *dict) {
	int count = dict->GetNumKeyVals();
	for (int i = 0; i < count; i++) {
		const idKeyValue *kv = dict->GetKeyVal(i);
		for (int j = 0; j < FixupCount; j++) {
			if (kv->GetKey().Icmp(FloatFixups[j].name) == 0) {
				idStr val;
				if (FloatFixups[j].type == 1) {
					idVec3 v;
					sscanf(kv->GetValue().c_str(), "%f %f %f", &v.x, &v.y, &v.z);
					sprintf(val, "%g %g %g", v.x, v.y, v.z);
				} else if (FloatFixups[j].type == 2) {
					idMat3 mat;
					sscanf(kv->GetValue().c_str(),	"%f %f %f %f %f %f %f %f %f ",&mat[0][0],&mat[0][1],&mat[0][2],&mat[1][0],&mat[1][1],&mat[1][2],&mat[2][0],&mat[2][1],&mat[2][2]);
					sprintf(val, "%g %g %g %g %g %g %g %g %g",mat[0][0],mat[0][1],mat[0][2],mat[1][0],mat[1][1],mat[1][2],mat[2][0],mat[2][1],mat[2][2]);
				} else {
					float f = atof(kv->GetValue().c_str());
					sprintf(val, "%g", f);
				}
				dict->Set(kv->GetKey(), val);
				break;
			}
		}
	}
}

void ParseEpair(idDict *dict) {
	idStr key = token;
	GetToken(false);
	idStr val = token;
	
	if (key.Length() > 0) {
		dict->Set(key, val);
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool EntityHasModel(entity_t *ent) {
	if (ent) {
		const char	*model = ValueForKey(ent, "model");
		const char	*name = ValueForKey(ent, "name");
		if (model && *model) {
			if ( idStr::Icmp(model, name) ) {
				return true;
			}
		}
	}

	return false;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
entity_t *Entity_New() {
	entity_t *ent = new entity_t;
	
	ent->prev = ent->next = NULL;
	ent->brushes.prev = ent->brushes.next = NULL;
	ent->brushes.oprev = ent->brushes.onext = NULL;
	ent->brushes.owner = NULL;
	ent->undoId = 0;
	ent->redoId = 0;
	ent->entityId = g_entityId++;
	ent->origin.Zero();
	ent->eclass = NULL;
	ent->md3Class = NULL;
	ent->lightOrigin.Zero();
	ent->lightRotation.Identity();
	ent->trackLightOrigin = false;
	ent->rotation.Identity();
	ent->lightDef = -1;
	ent->modelDef = -1;
	ent->soundEmitter = NULL;
	ent->curve = NULL;
	return ent;
}

void Entity_UpdateCurveData( entity_t *ent ) {
	
	if ( ent == NULL || ent->curve == NULL ) {
		return;
	}

	const idKeyValue *kv = ent->epairs.MatchPrefix( CURVE_TAG );
	if ( kv == NULL ) { 
		if ( ent->curve ) {
			delete ent->curve;
			ent->curve = NULL;
			if ( g_qeglobals.d_select_mode == sel_editpoint ) {
				g_qeglobals.d_select_mode = sel_brush;
			}
		}
		return;
	}

	int c = ent->curve->GetNumValues();
	idStr str = va( "%i ( ", c );
	idVec3 v;
	for ( int i = 0; i < c; i++ ) {
		v = ent->curve->GetValue( i );
		str += " ";
		str += v.ToString();
		str += " ";
	}
	str += " )";

	ent->epairs.Set( kv->GetKey(), str );

}

idCurve<idVec3> *Entity_MakeCurve( entity_t *ent ) {
	const idKeyValue *kv = ent->epairs.MatchPrefix( CURVE_TAG );
	if ( kv ) {
		idStr str = kv->GetKey().Right( kv->GetKey().Length() - strlen( CURVE_TAG ) );
		if ( str.Icmp( "CatmullRomSpline" ) == 0 ) {
			return new idCurve_CatmullRomSpline<idVec3>();
		} else if ( str.Icmp( "Nurbs" ) == 0 ) {
			return new idCurve_NURBS<idVec3>();
		}
	}
	return NULL;
}

void Entity_SetCurveData( entity_t *ent ) {

	ent->curve = Entity_MakeCurve( ent );
	const idKeyValue *kv = ent->epairs.MatchPrefix( CURVE_TAG );
	if ( kv && ent->curve ) {
		idLexer lex;
		lex.LoadMemory( kv->GetValue(), kv->GetValue().Length(), "_curve" );
		int numPoints = lex.ParseInt();
		if ( numPoints > 0 ) {
			float *fp = new float[numPoints * 3];
			lex.Parse1DMatrix( numPoints * 3, fp );
			int time = 0;
			for ( int i = 0; i < numPoints * 3; i += 3 ) {
				idVec3 v;
				v.x = fp[i];
				v.y = fp[i+1];
				v.z = fp[i+2];
				ent->curve->AddValue( time, v );
				time += 100;
			}
			delete []fp;
		}
	}

}

entity_t *Entity_PostParse(entity_t *ent, brush_t *pList) {
	bool		has_brushes;
	eclass_t	*e;
	brush_t		*b;
	idVec3		mins, maxs, zero;
	idBounds bo;

	zero.Zero();

	Entity_SetCurveData( ent );

	if (ent->brushes.onext == &ent->brushes) {
		has_brushes = false;
	}
	else {
		has_brushes = true;
	}

	bool needsOrigin = !GetVectorForKey(ent, "origin", ent->origin);
	const char	*pModel = ValueForKey(ent, "model");

	const char *cp = ValueForKey(ent, "classname");

	if (strlen(cp)) {
		e = Eclass_ForName(cp, has_brushes);
	} else {
		const char *cp2 = ValueForKey(ent, "name");
		if (strlen(cp2)) {
			char buff[1024];
			strcpy(buff, cp2);
			int len = strlen(buff);
			while ((isdigit(buff[len-1]) || buff[len-1] == '_') && len > 0) {
				buff[len-1] = '\0';
				len--;
			}
			e = Eclass_ForName(buff, has_brushes);
			SetKeyValue(ent, "classname", buff, false);
		} else {
			e = Eclass_ForName("", has_brushes);
		}
	}

	idStr str;
	
	if (e->defArgs.GetString("model", "", str) && e->entityModel == NULL) {
		e->entityModel = gameEdit->ANIM_GetModelFromEntityDef( &e->defArgs );
	}
	
	ent->eclass = e;

	bool hasModel = EntityHasModel(ent);

	if (hasModel) {
		ent->eclass->defArgs.GetString("model", "", str);
		if (str.Length()) {
			hasModel = false;
			ent->epairs.Delete("model");
		}
	}

	if (e->nShowFlags & ECLASS_WORLDSPAWN) {
		ent->origin.Zero();
		needsOrigin = false;
		ent->epairs.Delete( "model" );
	} else if (e->nShowFlags & ECLASS_LIGHT) {
		if (GetVectorForKey(ent, "light_origin", ent->lightOrigin)) {
			GetMatrixForKey(ent, "light_rotation", ent->lightRotation);
			ent->trackLightOrigin = true;
		} else if (hasModel) {
			SetKeyValue(ent, "light_origin", ValueForKey(ent, "origin"));
			ent->lightOrigin = ent->origin;
			if (GetMatrixForKey(ent, "rotation", ent->lightRotation)) {
				SetKeyValue(ent, "light_rotation", ValueForKey(ent, "rotation"));
			}
			ent->trackLightOrigin = true;
		}
	} else if ( e->nShowFlags & ECLASS_ENV ) {
		// need to create an origin from the bones here
		idVec3 org;
		idAngles ang;
		bo.Clear();
		bool hasBody = false;
		const idKeyValue *arg = ent->epairs.MatchPrefix( "body ", NULL );
		while ( arg ) {
			sscanf( arg->GetValue(), "%f %f %f %f %f %f", &org.x, &org.y, &org.z, &ang.pitch, &ang.yaw, &ang.roll );
			bo.AddPoint( org );
			arg = ent->epairs.MatchPrefix( "body ", arg );
			hasBody = true;
		}
		if (hasBody) {
			ent->origin = bo.GetCenter();
		}
	}

	if (e->fixedsize || hasModel) {			// fixed size entity
		if (ent->brushes.onext != &ent->brushes) {
			for (b = ent->brushes.onext; b != &ent->brushes; b = b->onext) {
				b->entityModel = true;
			}
		}

		if (hasModel) {
			// model entity
			idRenderModel *modelHandle = renderModelManager->FindModel( pModel );

			if ( dynamic_cast<idRenderModelPrt*>( modelHandle ) || dynamic_cast<idRenderModelLiquid*>( modelHandle ) ) {
				bo.Zero();
				bo.ExpandSelf( 12.0f );
			} else {
				bo = modelHandle->Bounds( NULL );
			}

			VectorCopy(bo[0], mins);
			VectorCopy(bo[1], maxs);
			for (int i = 0; i < 3; i++) {
				if (mins[i] == maxs[i]) {
					mins[i]--;
					maxs[i]++;
				}
			}
			VectorAdd(mins, ent->origin, mins);
			VectorAdd(maxs, ent->origin, maxs);
			b = Brush_Create(mins, maxs, &e->texdef);
			b->modelHandle = modelHandle;

			float		yaw = 0;
			bool		convertAngles = GetFloatForKey(ent, "angle", &yaw);
			extern void Brush_Rotate(brush_t *b, idMat3 matrix, idVec3 origin, bool bBuild);
			extern void Brush_Rotate(brush_t *b, idVec3 rot, idVec3 origin, bool bBuild);
			
			if (convertAngles) {
				idVec3	rot(0, 0, yaw);
				Brush_Rotate(b, rot, ent->origin, false);
			}

			if (GetMatrixForKey(ent, "rotation", ent->rotation)) {
				idBounds bo2;
				bo2.FromTransformedBounds(bo, ent->origin, ent->rotation);
				b->owner = ent;
				Brush_Resize(b, bo2[0], bo2[1]);
			}
			Entity_LinkBrush(ent, b);
		}

		if (!hasModel || (ent->eclass->nShowFlags & ECLASS_LIGHT && hasModel)) {
			// create a custom brush
			if (ent->trackLightOrigin) {
				mins = e->mins + ent->lightOrigin;
				maxs = e->maxs + ent->lightOrigin;
			} else {
				mins = e->mins + ent->origin;
				maxs = e->maxs + ent->origin;
			}

			b = Brush_Create(mins, maxs, &e->texdef);
			GetMatrixForKey(ent, "rotation", ent->rotation);
			Entity_LinkBrush(ent, b);
			b->trackLightOrigin = ent->trackLightOrigin;
			if ( e->texdef.name == NULL ) {
				brushprimit_texdef_t bp;
				texdef_t td;
				td.SetName( ent->eclass->defMaterial );
				Brush_SetTexture( b, &td, &bp, false );
			}
		}
	} else {	// brush entity
		if (ent->brushes.next == &ent->brushes) {
			printf("Warning: Brush entity with no brushes\n");
		}

		if (!needsOrigin) {
			idStr cn = ValueForKey(ent, "classname");
			idStr name = ValueForKey(ent, "name");
			idStr model = ValueForKey(ent, "model");
			if (cn.Icmp("func_static") == 0) {
				if (name.Icmp(model) == 0) {
					needsOrigin = true;
				}
			}
		}

		if (needsOrigin) {
			idVec3	mins, maxs, mid;
			int		i;
			char	text[32];
			mins[0] = mins[1] = mins[2] = 999999;
			maxs[0] = maxs[1] = maxs[2] = -999999;

			// add in the origin
			for (b = ent->brushes.onext; b != &ent->brushes; b = b->onext) {
				Brush_Build(b, true, false, false);
				for (i = 0; i < 3; i++) {
					if (b->mins[i] < mins[i]) {
						mins[i] = b->mins[i];
					}

					if (b->maxs[i] > maxs[i]) {
						maxs[i] = b->maxs[i];
					}
				}
			}

			for (i = 0; i < 3; i++) {
				ent->origin[i] = (mins[i] + ((maxs[i] - mins[i]) / 2));
			}

			sprintf(text, "%i %i %i", (int)ent->origin[0], (int)ent->origin[1], (int)ent->origin[2]);
			SetKeyValue(ent, "origin", text);
		}

		if (!(e->nShowFlags & ECLASS_WORLDSPAWN)) {
			if (e->defArgs.FindKey("model") == NULL && (pModel == NULL || (pModel && strlen(pModel) == 0))) {
				SetKeyValue(ent, "model", ValueForKey(ent, "name"));
			}
		}
		else {
			DeleteKey(ent, "origin");
		}
	}

	// add all the brushes to the main list
	if (pList) {
		for (b = ent->brushes.onext; b != &ent->brushes; b = b->onext) {
			b->next = pList->next;
			pList->next->prev = b;
			b->prev = pList;
			pList->next = b;
		}
	}

	FixFloats(&ent->epairs);

	return ent;

}

/*
 =======================================================================================================================
    Entity_Parse If onlypairs is set, the classname info will not be looked up, and the entity will not be added to the
    global list. Used for parsing the project.
 =======================================================================================================================
 */
entity_t *Entity_Parse(bool onlypairs, brush_t *pList) {
	entity_t	*ent;

	if (!GetToken(true)) {
		return NULL;
	}

	if (strcmp(token, "{")) {
		Error("ParseEntity: { not found");
	}

	ent = Entity_New();
	ent->brushes.onext = ent->brushes.oprev = &ent->brushes;
	ent->origin.Zero();

	int n = 0;
	do {
		if (!GetToken(true)) {
			Warning("ParseEntity: EOF without closing brace");
			return NULL;
		}

		if (!strcmp(token, "}")) {
			break;
		}

		if (!strcmp(token, "{")) {
			GetVectorForKey(ent, "origin", ent->origin);
			brush_t *b = Brush_Parse(ent->origin);
			if (b != NULL) {
				b->owner = ent;

				// add to the end of the entity chain
				b->onext = &ent->brushes;
				b->oprev = ent->brushes.oprev;
				ent->brushes.oprev->onext = b;
				ent->brushes.oprev = b;
			}
			else {
				break;
			}
		}
		else {
			ParseEpair(&ent->epairs);
		}
	} while (1);

	if (onlypairs) {
		return ent;
	}

	return Entity_PostParse(ent, pList);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void VectorMidpoint(idVec3 va, idVec3 vb, idVec3 &out) {
	for (int i = 0; i < 3; i++) {
		out[i] = va[i] + ((vb[i] - va[i]) / 2);
	}
}

/*
 =======================================================================================================================
    Entity_Write
 =======================================================================================================================
 */
void Entity_Write(entity_t *e, FILE *f, bool use_region) {
	brush_t *b;
	idVec3	origin;
	char	text[128];
	int		count;

	// if none of the entities brushes are in the region, don't write the entity at all
	if (use_region) {
		// in region mode, save the camera position as playerstart
		if (!strcmp(ValueForKey(e, "classname"), "info_player_start")) {
			fprintf(f, "{\n");
			fprintf(f, "\"classname\" \"info_player_start\"\n");
			fprintf
			(
				f,
				"\"origin\" \"%i %i %i\"\n",
				(int)g_pParentWnd->GetCamera()->Camera().origin[0],
				(int)g_pParentWnd->GetCamera()->Camera().origin[1],
				(int)g_pParentWnd->GetCamera()->Camera().origin[2]
			);
			fprintf(f, "\"angle\" \"%i\"\n", (int)g_pParentWnd->GetCamera()->Camera().angles[YAW]);
			fprintf(f, "}\n");
			return;
		}

		for (b = e->brushes.onext; b != &e->brushes; b = b->onext) {
			if (!Map_IsBrushFiltered(b)) {
				break;	// got one
			}
		}

		if (b == &e->brushes) {
			return;		// nothing visible
		}
	}

	if (e->eclass->nShowFlags & ECLASS_PLUGINENTITY) {
		// NOTE: the whole brush placement / origin stuff is a mess
		VectorCopy(e->origin, origin);
		sprintf(text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
		SetKeyValue(e, "origin", text);
	}

	// if fixedsize, calculate a new origin based on the current brush position
	else if (e->eclass->fixedsize || EntityHasModel(e)) {
		if (!GetVectorForKey(e, "origin", origin)) {
			VectorSubtract(e->brushes.onext->mins, e->eclass->mins, origin);
			sprintf(text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
			SetKeyValue(e, "origin", text);
		}
	}

	fprintf(f, "{\n");

	count = e->epairs.GetNumKeyVals();
	for (int j = 0; j < count; j++) {
		fprintf(f, "\"%s\" \"%s\"\n", e->epairs.GetKeyVal(j)->GetKey().c_str(), e->epairs.GetKeyVal(j)->GetValue().c_str());
	}

	if (!EntityHasModel(e)) {
		count = 0;
		for (b = e->brushes.onext; b != &e->brushes; b = b->onext) {
			if (e->eclass->fixedsize && !b->entityModel) {
				continue;
			}
			if (!use_region || !Map_IsBrushFiltered(b)) {
				fprintf(f, "// brush %i\n", count);
				count++;
				Brush_Write( b, f, e->origin, ( g_PrefsDlg.m_bNewMapFormat != FALSE ) );
			}
		}
	}

	fprintf(f, "}\n");
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
bool IsBrushSelected(brush_t *bSel) {
	for (brush_t * b = selected_brushes.next; b != NULL && b != &selected_brushes; b = b->next) {
		if (b == bSel) {
			return true;
		}
	}

	return false;
}

//
// =======================================================================================================================
//    Entity_WriteSelected
// =======================================================================================================================
//
void Entity_WriteSelected(entity_t *e, FILE *f) {
	brush_t *b;
	idVec3	origin;
	char	text[128];
	int		count;

	for (b = e->brushes.onext; b != &e->brushes; b = b->onext) {
		if (IsBrushSelected(b)) {
			break;	// got one
		}
	}

	if (b == &e->brushes) {
		return;		// nothing selected
	}

	// if fixedsize, calculate a new origin based on the current brush position
	if (e->eclass->fixedsize || EntityHasModel(e)) {
		if (!GetVectorForKey(e, "origin", origin)) {
			VectorSubtract(e->brushes.onext->mins, e->eclass->mins, origin);
			sprintf(text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
			SetKeyValue(e, "origin", text);
		}
	}

	fprintf(f, "{\n");
	
	count = e->epairs.GetNumKeyVals();
	for (int j = 0; j < count; j++) {
		fprintf(f, "\"%s\" \"%s\"\n", e->epairs.GetKeyVal(j)->GetKey().c_str(), e->epairs.GetKeyVal(j)->GetValue().c_str());
	}

	if (!EntityHasModel(e)) {
		count = 0;
		for (b = e->brushes.onext; b != &e->brushes; b = b->onext) {
			if (e->eclass->fixedsize && !b->entityModel) {
				continue;
			}
			if (IsBrushSelected(b)) {
				fprintf(f, "// brush %i\n", count);
				count++;
				Brush_Write( b, f, e->origin, ( g_PrefsDlg.m_bNewMapFormat != FALSE ) );
			}
		}
	}

	fprintf(f, "}\n");
}

//
// =======================================================================================================================
//    Entity_WriteSelected to a CMemFile
// =======================================================================================================================
//
void Entity_WriteSelected(entity_t *e, CMemFile *pMemFile) {
	brush_t *b;
	idVec3	origin;
	char	text[128];
	int		count;

	for (b = e->brushes.onext; b != &e->brushes; b = b->onext) {
		if (IsBrushSelected(b)) {
			break;	// got one
		}
	}

	if (b == &e->brushes) {
		return;		// nothing selected
	}

	// if fixedsize, calculate a new origin based on the current brush position
	if (e->eclass->fixedsize || EntityHasModel(e)) {
		if (!GetVectorForKey(e, "origin", origin)) {
			VectorSubtract(e->brushes.onext->mins, e->eclass->mins, origin);
			sprintf(text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
			SetKeyValue(e, "origin", text);
		}
	}

	MemFile_fprintf(pMemFile, "{\n");

	count = e->epairs.GetNumKeyVals();
	for (int j = 0; j < count; j++) {
		MemFile_fprintf(pMemFile, "\"%s\" \"%s\"\n", e->epairs.GetKeyVal(j)->GetKey().c_str(), e->epairs.GetKeyVal(j)->GetValue().c_str());
	}

	if (!EntityHasModel(e)) {
		count = 0;
		for (b = e->brushes.onext; b != &e->brushes; b = b->onext) {
			if (e->eclass->fixedsize && !b->entityModel) {
				continue;
			}
			if (IsBrushSelected(b)) {
				MemFile_fprintf(pMemFile, "// brush %i\n", count);
				count++;
				Brush_Write( b, pMemFile, e->origin, ( g_PrefsDlg.m_bNewMapFormat != FALSE ) );
			}
		}
	}

	MemFile_fprintf(pMemFile, "}\n");
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Entity_SetName(entity_t *e, const char *name) {
	CString oldName = ValueForKey(e, "name");
	CString oldModel = ValueForKey(e, "model");
	SetKeyValue(e, "name", name);
	if (oldName == oldModel) {
		SetKeyValue(e, "model", name);
	}
}

extern bool Entity_NameIsUnique(const char *name);

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void Entity_Name(entity_t *e, bool force) {
	const char	*name = ValueForKey(e, "name");

	if (!force && name && name[0]) {
		return;
	}

	if (name && name[0] && Entity_NameIsUnique(name)) {
		return;
	}

	bool	setModel = false;
	if (name[0]) {
		const char	*model = ValueForKey(e, "model");
		if (model[0]) {
			if ( idStr::Icmp(model, name) == 0 ) {
				setModel = true;
			}
		}
	}

	const char *eclass = ValueForKey(e, "classname");
	if (eclass && eclass[0]) {
		idStr str = cvarSystem->GetCVarString( "radiant_nameprefix" );
		int id = Map_GetUniqueEntityID(str, eclass);
		if (str.Length()) {
			SetKeyValue(e, "name", va("%s_%s_%i", str.c_str(), eclass, id));
		} else {
			SetKeyValue(e, "name", va("%s_%i", eclass, id));
		}
		if (setModel) {
			if (str.Length()) {
				SetKeyValue(e, "model", va("%s_%s_%i", str.c_str(), eclass, id));
			} else {
				SetKeyValue(e, "model", va("%s_%i",  eclass, id));
			}
		}
	}
}

/*
 =======================================================================================================================
    Entity_Create Creates a new entity out of the selected_brushes list. If the entity class is fixed size, the brushes
    are only used to find a midpoint. Otherwise, the brushes have their ownership transfered to the new entity.
 =======================================================================================================================
 */
entity_t *Entity_Create(eclass_t *c, bool forceFixed) {
	entity_t	*e;
	brush_t		*b;
	idVec3		mins, maxs, origin;
	char		text[32];
	texdef_t td;
	brushprimit_texdef_t bp;

	// check to make sure the brushes are ok
	for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
		if (b->owner != world_entity) {
			Sys_Status("Entity NOT created, brushes not all from world\n");
			Sys_Beep();
			return NULL;
		}
	}

	idStr str;
	if (c->defArgs.GetString("model", "", str) && c->entityModel == NULL) {
		c->entityModel = gameEdit->ANIM_GetModelFromEntityDef( &c->defArgs );
	}

	// create it
	e = Entity_New();
	e->brushes.onext = e->brushes.oprev = &e->brushes;
	e->eclass = c;
	e->epairs.Copy(c->args);
	SetKeyValue(e, "classname", c->name);
	Entity_Name(e, false);

	// add the entity to the entity list
	Entity_AddToList(e, &entities);

	if (c->fixedsize) {
		//
		// just use the selection for positioning b = selected_brushes.next; for (i=0 ;
		// i<3 ; i++) { e->origin[i] = b->mins[i] - c->mins[i]; }
		//
		Select_GetMid(e->origin);
		VectorCopy(e->origin, origin);

		// create a custom brush
		VectorAdd(c->mins, e->origin, mins);
		VectorAdd(c->maxs, e->origin, maxs);

		b = Brush_Create(mins, maxs, &c->texdef);

		Entity_LinkBrush(e, b);

		if (c->defMaterial.Length()) {
			td.SetName(c->defMaterial);
			Brush_SetTexture(b, &td, &bp, false);
		}


		// delete the current selection
		Select_Delete();

		// select the new brush
		b->next = b->prev = &selected_brushes;
		selected_brushes.next = selected_brushes.prev = b;

		Brush_Build(b);
	}
	else {

		Select_GetMid(origin);

		// change the selected brushes over to the new entity
		for (b = selected_brushes.next; b != &selected_brushes; b = b->next) {
			Entity_UnlinkBrush(b);
			Entity_LinkBrush(e, b);
			Brush_Build(b); // so the key brush gets a name
			if (c->defMaterial.Length()) {
				td.SetName(c->defMaterial);
				Brush_SetTexture(b, &td, &bp, false);
			}

		}

		//for (int i = 0; i < 3; i++) {
		//	origin[i] = vMin[i] + vMax[i] * 0.5;
		//}

		if (!forceFixed) {
			SetKeyValue(e, "model", ValueForKey(e, "name"));
		}
	}

	sprintf(text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
	SetKeyValue(e, "origin", text);
	VectorCopy(origin, e->origin);

	Sys_UpdateWindows(W_ALL);
	return e;
}

void Brush_MakeDirty(brush_t *b) {
	for (face_t *f = b->brush_faces; f; f = f->next) {
		f->dirty = true;
	}
}
/*
 =======================================================================================================================
    Entity_LinkBrush
 =======================================================================================================================
 */
void Entity_LinkBrush(entity_t *e, brush_t *b) {
	if (b->oprev || b->onext) {
		Error("Entity_LinkBrush: Allready linked");
	}

	Brush_MakeDirty(b);

	b->owner = e;

	b->onext = e->brushes.onext;
	b->oprev = &e->brushes;
	e->brushes.onext->oprev = b;
	e->brushes.onext = b;
}

/*
 =======================================================================================================================
    Entity_UnlinkBrush
 =======================================================================================================================
 */
void Entity_UnlinkBrush(brush_t *b) {
	// if (!b->owner || !b->onext || !b->oprev)
	if (!b->onext || !b->oprev) {
		Error("Entity_UnlinkBrush: Not currently linked");
	}

	b->onext->oprev = b->oprev;
	b->oprev->onext = b->onext;
	b->onext = b->oprev = NULL;
	b->owner = NULL;
}

/*
 =======================================================================================================================
    Entity_Clone
 =======================================================================================================================
 */
entity_t *Entity_Clone(entity_t *e) {
	entity_t	*n;

	n = Entity_New();
	n->brushes.onext = n->brushes.oprev = &n->brushes;
	n->eclass = e->eclass;
	n->rotation = e->rotation;
	n->origin = e->origin;

	// add the entity to the entity list
	Entity_AddToList(n, &entities);

	n->epairs  = e->epairs;

	return n;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
int GetUniqueTargetId(int iHint) {
	int			iMin, iMax, i;
	BOOL		fFound;
	entity_t	*pe;

	fFound = FALSE;
	pe = entities.next;
	iMin = 0;
	iMax = 0;

	for (; pe != NULL && pe != &entities; pe = pe->next) {
		i = IntForKey(pe, "target");
		if (i) {
			iMin = Min(i, iMin);
			iMax = Max(i, iMax);
			if (i == iHint) {
				fFound = TRUE;
			}
		}
	}

	if (fFound) {
		return iMax + 1;
	}
	else {
		return iHint;
	}
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
entity_t *FindEntity(const char *pszKey, const char *pszValue) {
	entity_t	*pe;

	pe = entities.next;

	for (; pe != NULL && pe != &entities; pe = pe->next) {
		if (!strcmp(ValueForKey(pe, pszKey), pszValue)) {
			return pe;
		}
	}

	return NULL;
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
entity_t *FindEntityInt(const char *pszKey, int iValue) {
	entity_t	*pe;

	pe = entities.next;

	for (; pe != NULL && pe != &entities; pe = pe->next) {
		if (IntForKey(pe, pszKey) == iValue) {
			return pe;
		}
	}

	return NULL;
}

/*
====================
Entity_UpdateSoundEmitter

Deletes the soundEmitter if the entity should not emit a sound due
to it not having one, being filtered away, or the sound mode being turned off.

Creates or updates the soundEmitter if needed
====================
*/
void Entity_UpdateSoundEmitter( entity_t *ent ) {
	bool	playing = false;

	// if an entity doesn't have any brushes at all, don't do anything
	// if the brush isn't displayed (filtered or culled), don't do anything
	if ( g_pParentWnd->GetCamera()->GetSoundMode() 
		&& ent->brushes.onext != &ent->brushes && !FilterBrush(ent->brushes.onext) ) {
		// check for sounds
		const char *v = ValueForKey( ent, "s_shader" );
		if ( v[0] ) {
			refSound_t	sound;

			gameEdit->ParseSpawnArgsToRefSound( &ent->epairs, &sound );
			if ( !sound.waitfortrigger ) {	// waitfortrigger will not start playing immediately
				if ( !ent->soundEmitter ) {
					ent->soundEmitter = g_qeglobals.sw->AllocSoundEmitter();
				}
				playing = true;
				ent->soundEmitter->UpdateEmitter( ent->origin, 0, &sound.parms );
				// always play on a single channel, so updates always override
				ent->soundEmitter->StartSound( sound.shader, SCHANNEL_ONE );
			}
		}
	}

	// delete the soundEmitter if not used
	if ( !playing && ent->soundEmitter ) {
		ent->soundEmitter->Free( true );
		ent->soundEmitter = NULL;
	}

}


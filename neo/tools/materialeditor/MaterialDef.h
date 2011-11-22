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
#pragma once

/**
* Represents a single attribute in a material. Represents material, stage
* and special stage attributes. The MaterialDef manager loads these
* definitions from the material definition file as the editor
* is being initialized.
*/
class MaterialDef {

public:
	/**
	* Defines possible attribute types.
	*/
	enum {
		MATERIAL_DEF_TYPE_GROUP,
		MATERIAL_DEF_TYPE_BOOL,
		MATERIAL_DEF_TYPE_STRING,
		MATERIAL_DEF_TYPE_FLOAT,
		MATERIAL_DEF_TYPE_INT
	};
	
	int					type;
	idStr				dictName;
	idStr				displayName;
	idStr				displayInfo;
	bool				quotes;
	idHashTable<DWORD>	viewData;
	
public:
	
	MaterialDef(void);
	virtual ~MaterialDef(void);

	DWORD	GetViewData(const char* viewName);
	void	SetViewData(const char* viewName, DWORD value);
};

/**
* A list of material attributes. Material, stage, and special stage attributes
* are grouped together during the load process for use by the different view and
* MaterialDoc.
*/
typedef idList<MaterialDef*> MaterialDefList;

/**
* This class contains static utility functions that view and MaterialDoc use
* to access the MaterialDef and MaterialDefList data that is loaded. This class
* is also responsible for loading and destroying the MaterialDef instances.
*/
class MaterialDefManager {

public:

	/**
	* Defines the groupings of material attributes.
	*/
	enum {
		MATERIAL_DEF_MATERIAL = 0,
		MATERIAL_DEF_STAGE,
		MATERIAL_DEF_SPECIAL_STAGE,
		MATERIAL_DEF_NUM
	};

	static void 				InitializeMaterialDefLists();
	static void					InitializeMaterialDefList(idLexer* src, const char* typeName, MaterialDefList* list);
	
	static void					DestroyMaterialDefLists();

	static MaterialDefList*		GetMaterialDefs(int type);


protected:
	static MaterialDefList		materialDefs[MATERIAL_DEF_NUM];	
};

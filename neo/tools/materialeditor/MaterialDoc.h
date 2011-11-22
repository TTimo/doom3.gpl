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

#include "MaterialEditor.h"
#include "MaterialModifier.h"
#include "MaterialDef.h"

/**
* Dictionary representation of a Material Stage.
*/
typedef struct {
	idDict				stageData;
	bool				enabled;
} MEStage_t;

/**
* Dictionary representation of a material.
*/
typedef struct {
	idDict				materialData;
	idList<MEStage_t*>	stages;
} MEMaterial_t;

/**
* Implemented by the edit window that is responsible for modifying the material source text.
*/
class SourceModifyOwner {

public:
	SourceModifyOwner() {};
	virtual ~SourceModifyOwner() {};

	virtual idStr GetSourceText() { return ""; };
};

class MaterialDocManager;

/**
* Responsible for managing a single material that is being viewed and/or edited.
*/
class MaterialDoc {

public:
	MaterialDocManager*		manager;
	idStr					name;
	idMaterial*				renderMaterial;
	MEMaterial_t			editMaterial;
	
	bool					modified;
	bool					applyWaiting;
	bool					deleted;

	bool					sourceModify;
	SourceModifyOwner*		sourceModifyOwner;
	
public:
	MaterialDoc(void);
	~MaterialDoc(void);

	/**
	* Define the types of stages in a material.
	*/
	enum {
		STAGE_TYPE_NORMAL,
		STAGE_TYPE_SPECIALMAP
	};

	//Initialization Methods
	void			SetRenderMaterial(idMaterial* material, bool parseMaterial = true, bool parseRenderMatierial = false);

	//Stage Info Methods
	int				GetStageCount();
	int				FindStage(int stageType, const char* name);
	MEStage_t		GetStage(int stage);
	void			EnableStage(int stage, bool enabled);
	void			EnableAllStages(bool enabled);
	bool			IsStageEnabled(int stage);

	//Get Attributes
	const char*		GetAttribute(int stage, const char* attribName, const char* defaultString = "");
	int				GetAttributeInt(int stage, const char* attribName, const char* defaultString = "0");
	float			GetAttributeFloat(int stage, const char* attribName, const char* defaultString = "0");
	bool			GetAttributeBool(int stage, const char* attribName, const char* defaultString = "0");

	//Set Attribute Methods
	void			SetAttribute(int stage, const char* attribName, const char* value, bool addUndo = true);
	void			SetAttributeInt(int stage, const char* attribName, int value, bool addUndo = true);
	void			SetAttributeFloat(int stage, const char* attribName, float value, bool addUndo = true);
	void			SetAttributeBool(int stage, const char* attribName, bool value, bool addUndo = true);
	void			SetMaterialName(const char* materialName, bool addUndo = true);
	void			SetData(int stage, idDict* data);

	//Source Editing Methods
	void			SourceModify(SourceModifyOwner* owner);
	bool			IsSourceModified();
	void			ApplySourceModify(idStr& text);
	const char*		GetEditSourceText();

	//Stage Modification Methods
	void			AddStage(int stageType, const char* stageName, bool addUndo = true);
	void			InsertStage(int stage, int stageType, const char* stageName, bool addUndo = true);
	void			RemoveStage(int stage, bool addUndo = true);
	void			ClearStages();
	void			MoveStage(int from, int to, bool addUndo = true);

	void			ApplyMaterialChanges(bool force = false);
	void			Save();
	void			Delete();
	
protected:

	//Internal Notifications
	void			OnMaterialChanged();
		
	//Load Material Methods
	void			ParseMaterialText(const char* source);
	void			ParseMaterial(idLexer* src);
	void			ParseStage(idLexer* src);
	void			AddSpecialMapStage(const char* stageName, const char* map);
	bool			ParseMaterialDef(idToken* token, idLexer* src, int type, idDict* dict);
	void			ClearEditMaterial();

	//Save/Apply Material Methods
	const char*		GenerateSourceText();
	void			ReplaceSourceText();
	void			WriteStage(int stage, idFile_Memory* file);
	void			WriteSpecialMapStage(int stage, idFile_Memory* file);
	void			WriteMaterialDef(int stage, idFile_Memory* file, int type, int indent);
};


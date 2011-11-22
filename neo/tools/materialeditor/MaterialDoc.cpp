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

#include "MaterialDoc.h"
#include "MaterialView.h"

/**
* Constructor for MaterialDoc.
*/
MaterialDoc::MaterialDoc(void) {
	modified = false;
	applyWaiting = false;
	sourceModify = false;
}

/**
* Destructor for MaterialDoc.
*/
MaterialDoc::~MaterialDoc(void) {
	ClearEditMaterial();
}

/**
* Initializes the MaterialDoc instance with a specific idMaterial. This method will 
* parse the material into the internal dictionary representation and optionally 
* allow the idMaterial object to reparse the source.
* @param material The idMaterial instance to use.
* @param parseMaterial Flag to determine if the material should be parsed into the editor representation.
* @param parseRenderMaterial Flag to determine if the idMaterial object should be reparsed.
*/
void MaterialDoc::SetRenderMaterial(idMaterial* material, bool parseMaterial, bool parseRenderMatierial) {

	renderMaterial = material;


	if(!parseMaterial ||  !renderMaterial)	
		return;

	if(parseRenderMatierial) {
		char *declText = (char *) _alloca( material->GetTextLength() + 1 );
		material->GetText( declText );

		renderMaterial->GetText(declText);
		ParseMaterialText(declText);

	}

	ClearEditMaterial();

	name = material->GetName();

	idLexer		src;

	char *declText = (char *) _alloca( material->GetTextLength() + 1 );
	material->GetText( declText );

	renderMaterial->GetText(declText);
	src.LoadMemory(declText, strlen(declText), "Material");

	ParseMaterial(&src);
}

/**
* Returns the number of stages in this material.
*/
int	MaterialDoc::GetStageCount() {
	return editMaterial.stages.Num();
}

/**
* Returns the index of the stage with the specified type and name or -1
* if the stage does not exist.
* @param stageType The type of stage to find.
* @param name The name of the stage to find.
*/
int	MaterialDoc::FindStage(int stageType, const char* name) {
	
	for(int i = 0; i < editMaterial.stages.Num(); i++) {
		int type = GetAttributeInt(i, "stagetype");
		idStr localname = GetAttribute(i, "name");
		if(stageType == type && !localname.Icmp(name))
			return i;
	}
	return -1;
}

/**
* Returns a copy of the specified stage.
* @param stage The stage to return.
*/
MEStage_t MaterialDoc::GetStage(int stage) {
	assert(stage >= 0 && stage < GetStageCount());
	return *editMaterial.stages[stage];

}

/**
* Specifies the enabled state of a single stage.
* @param stage The stage to change.
* @param enabled The enabled state.
*/
void MaterialDoc::EnableStage(int stage, bool enabled) {

	assert(stage >= 0 && stage < GetStageCount());
	editMaterial.stages[stage]->enabled = enabled;

	OnMaterialChanged();
}

/**
* Sets the enabled state of all stages.
* @param enabled The enabled state.
*/
void MaterialDoc::EnableAllStages(bool enabled) {
	for(int i = 0; i < GetStageCount(); i++) {
		editMaterial.stages[i]->enabled = enabled;
	}
}

/**
* Returns the enabled state of a stage.
* @param stage The stage to check.
*/
bool MaterialDoc::IsStageEnabled(int stage) {
	assert(stage >= 0 && stage < GetStageCount());
	return editMaterial.stages[stage]->enabled;
}

/**
* Returns an attribute string from the material or a stage.
* @param stage The stage or -1 for the material.
* @param attribName The name of the attribute.
* @param defaultString The default value if the attribute is not specified.
*/
const char*	MaterialDoc::GetAttribute(int stage, const char* attribName, const char* defaultString) {

	if(stage == -1) {
		return editMaterial.materialData.GetString(attribName, defaultString);
	} else {
		assert(stage >= 0 && stage < GetStageCount());
		MEStage_t* pStage = editMaterial.stages[stage];
		return pStage->stageData.GetString(attribName, defaultString);
	}
}

/**
* Returns an attribute int from the material or a stage.
* @param stage The stage or -1 for the material.
* @param attribName The name of the attribute.
* @param defaultString The default value if the attribute is not specified.
*/
int MaterialDoc::GetAttributeInt(int stage, const char* attribName, const char* defaultString) {
	if(stage == -1) {
		return editMaterial.materialData.GetInt(attribName, defaultString);
	} else {
		assert(stage >= 0 && stage < GetStageCount());
		MEStage_t* pStage = editMaterial.stages[stage];
		return pStage->stageData.GetInt(attribName, defaultString);
	}
}

/**
* Returns an attribute float from the material or a stage.
* @param stage The stage or -1 for the material.
* @param attribName The name of the attribute.
* @param defaultString The default value if the attribute is not specified.
*/
float MaterialDoc::GetAttributeFloat(int stage, const char* attribName, const char* defaultString) {
	if(stage == -1) {
		return editMaterial.materialData.GetFloat(attribName, defaultString);
	} else {
		assert(stage >= 0 && stage < GetStageCount());
		MEStage_t* pStage = editMaterial.stages[stage];
		return pStage->stageData.GetFloat(attribName, defaultString);
	}
}

/**
* Returns an attribute bool from the material or a stage.
* @param stage The stage or -1 for the material.
* @param attribName The name of the attribute.
* @param defaultString The default value if the attribute is not specified.
*/
bool MaterialDoc::GetAttributeBool(int stage, const char* attribName, const char* defaultString) {
	if(stage == -1) {
		return editMaterial.materialData.GetBool(attribName, defaultString);
	} else {
		assert(stage >= 0 && stage < GetStageCount());
		MEStage_t* pStage = editMaterial.stages[stage];
		return pStage->stageData.GetBool(attribName, defaultString);
	}
}

/**
* Sets an attribute string in the material or a stage.
* @param stage The stage or -1 for the material.
* @param attribName The name of the attribute.
* @param value The value to set.
* @param addUndo Flag that specifies if the system should add an undo operation.
*/
void MaterialDoc::SetAttribute(int stage, const char* attribName, const char* value, bool addUndo) {
	
	//Make sure we need to set the attribute
	idStr orig  = GetAttribute(stage, attribName);
	if(orig.Icmp(value)) {
		
		idDict* dict;
		if(stage == -1) {
			dict = &editMaterial.materialData;
		} else {
			assert(stage >= 0 && stage < GetStageCount());
			dict = &editMaterial.stages[stage]->stageData;
		}

		if(addUndo) {
			//Create a new Modifier for this change so we can undo and redo later
			AttributeMaterialModifierString* mod = new AttributeMaterialModifierString(manager, name, stage, attribName, value, orig);
			manager->AddMaterialUndoModifier(mod);
		}

		dict->Set(attribName, value);
		
		manager->AttributeChanged(this, stage, attribName);
		OnMaterialChanged();
	}
}

/**
* Sets an attribute int in the material or a stage.
* @param stage The stage or -1 for the material.
* @param attribName The name of the attribute.
* @param value The value to set.
* @param addUndo Flag that specifies if the system should add an undo operation.
*/
void MaterialDoc::SetAttributeInt(int stage, const char* attribName, int value, bool addUndo) {
	//Make sure we need to set the attribute
	int orig  = GetAttributeInt(stage, attribName);
	if(orig != value) {

		idDict* dict;
		if(stage == -1) {
			dict = &editMaterial.materialData;
		} else {
			assert(stage >= 0 && stage < GetStageCount());
			dict = &editMaterial.stages[stage]->stageData;
		}

		dict->SetInt(attribName, value);

		manager->AttributeChanged(this, stage, attribName);
		OnMaterialChanged();
	}
}

/**
* Sets an attribute float in the material or a stage.
* @param stage The stage or -1 for the material.
* @param attribName The name of the attribute.
* @param value The value to set.
* @param addUndo Flag that specifies if the system should add an undo operation.
*/
void MaterialDoc::SetAttributeFloat(int stage, const char* attribName, float value, bool addUndo) {
	//Make sure we need to set the attribute
	float orig  = GetAttributeFloat(stage, attribName);
	if(orig != value) {

		idDict* dict;
		if(stage == -1) {
			dict = &editMaterial.materialData;
		} else {
			assert(stage >= 0 && stage < GetStageCount());
			dict = &editMaterial.stages[stage]->stageData;
		}

		dict->SetFloat(attribName, value);

		manager->AttributeChanged(this, stage, attribName);
		OnMaterialChanged();
	}
}

/**
* Sets an attribute bool in the material or a stage.
* @param stage The stage or -1 for the material.
* @param attribName The name of the attribute.
* @param value The value to set.
* @param addUndo Flag that specifies if the system should add an undo operation.
*/
void MaterialDoc::SetAttributeBool(int stage, const char* attribName, bool value, bool addUndo) {
	//Make sure we need to set the attribute
	bool orig  = GetAttributeBool(stage, attribName);
	if(orig != value) {

		idDict* dict;
		if(stage == -1) {
			dict = &editMaterial.materialData;
		} else {
			assert(stage >= 0 && stage < GetStageCount());
			dict = &editMaterial.stages[stage]->stageData;
		}

		if(addUndo) {
			//Create a new Modifier for this change so we can undo and redo later
			AttributeMaterialModifierBool* mod = new AttributeMaterialModifierBool(manager, name, stage, attribName, value, orig);
			manager->AddMaterialUndoModifier(mod);
		}

		dict->SetBool(attribName, value);

		manager->AttributeChanged(this, stage, attribName);
		OnMaterialChanged();
	}
}

/**
* Sets the material name.
* @param materialName The new name of the material.
* @param addUndo Flag that specifies if the system should add an undo operation.
*/
void MaterialDoc::SetMaterialName(const char* materialName, bool addUndo) {
	idStr oldName = name;

	declManager->RenameDecl(DECL_MATERIAL, oldName, materialName); 
	name = renderMaterial->GetName();
	
	if(addUndo) {
		RenameMaterialModifier* mod = new RenameMaterialModifier(manager, name, oldName);
		manager->AddMaterialUndoModifier(mod);
	}

	manager->MaterialNameChanged(oldName, this);

	OnMaterialChanged();

	//Need to do an instant apply for material name changes
	ApplyMaterialChanges();
}

/**
* Sets the entire dictionary for a material or stage
* @param stage The stage or -1 for the material.
* @param data The dictionary to copy.
*/
void MaterialDoc::SetData(int stage, idDict* data) {
	idDict* dict;
	if(stage == -1) {
		dict = &editMaterial.materialData;
	} else {
		assert(stage >= 0 && stage < GetStageCount());
		dict = &editMaterial.stages[stage]->stageData;
	}
	dict->Clear();
	dict->Copy(*data);
}

/**
* Called when the editor modifies the source of the material.
* @param text The new source text.
*/
void MaterialDoc::SourceModify(SourceModifyOwner* owner) {
	
	sourceModifyOwner = owner;
	sourceModify = true;
	OnMaterialChanged();
}

/**
* Returns true if the source text of this material has been edited.
*/
bool MaterialDoc::IsSourceModified() {
	return sourceModify;
}

/**
* Applies any source changes to the edit representation of the material.
*/
void MaterialDoc::ApplySourceModify(idStr& text) {
	
	if(sourceModify) {
		
		//Changes in the source need to clear any undo redo buffer because we have no idea what has changed
		manager->ClearUndo();
		manager->ClearRedo();

		ClearEditMaterial();

		idLexer		src;
		src.LoadMemory(text, text.Length(), "Material");

		src.SetFlags( 
			LEXFL_NOSTRINGCONCAT |			// multiple strings seperated by whitespaces are not concatenated
			LEXFL_NOSTRINGESCAPECHARS |		// no escape characters inside strings
			LEXFL_ALLOWPATHNAMES |			// allow path seperators in names
			LEXFL_ALLOWMULTICHARLITERALS |	// allow multi character literals
			LEXFL_ALLOWBACKSLASHSTRINGCONCAT |	// allow multiple strings seperated by '\' to be concatenated
			LEXFL_NOFATALERRORS				// just set a flag instead of fatal erroring
			);

		idToken token;
		if(!src.ReadToken(&token)) {
			src.Warning( "Missing decl name" );
			return;
		}
		
		ParseMaterial(&src);
		sourceModify = false;

		//Check to see if the name has changed
		if(token.Icmp(name)) {
			SetMaterialName(token, false);
		}
	}
}

/**
* Returns the appropriate source for the editing
*/
const char*	MaterialDoc::GetEditSourceText() {

	return GenerateSourceText();
}

/**
* Adds a stage to the material.
* @param stageType The type of the stage: normal or special.
* @param stageName The name of the stage.
* @param addUndo Flag that specifies if the system should add an undo operation.
*/
void MaterialDoc::AddStage(int stageType, const char* stageName, bool addUndo) {
	MEStage_t* newStage = new MEStage_t();

	int index = editMaterial.stages.Append(newStage);
	newStage->stageData.Set("name", stageName);
	newStage->stageData.SetInt("stagetype", stageType);
	newStage->enabled = true;

	if(addUndo) {
		StageInsertModifier* mod = new StageInsertModifier(manager, name, index, stageType, stageName);
		manager->AddMaterialUndoModifier(mod);
	}

	manager->StageAdded(this, index);

	OnMaterialChanged();
}

/**
* Inserts a new stage to the material at a specified location.
* @param stage The location to insert the stage.
* @param stageType The type of the stage: normal or special.
* @param stageName The name of the stage.
* @param addUndo Flag that specifies if the system should add an undo operation.
*/
void MaterialDoc::InsertStage(int stage, int stageType, const char* stageName, bool addUndo) {
	MEStage_t* newStage = new MEStage_t();

	editMaterial.stages.Insert(newStage, stage);
	newStage->stageData.Set("name", stageName);
	newStage->stageData.SetInt("stagetype", stageType);
	newStage->enabled = true;

	if(addUndo) {
		StageInsertModifier* mod = new StageInsertModifier(manager, name, stage, stageType, stageName);
		manager->AddMaterialUndoModifier(mod);
	}

	manager->StageAdded(this, stage);

	OnMaterialChanged();
}

/**
* Removes a stage from the material.
* @param stage The stage to remove.
* @param addUndo Flag that specifies if the system should add an undo operation.
*/
void MaterialDoc::RemoveStage(int stage, bool addUndo) {
	assert(stage >= 0 && stage < GetStageCount());

	if(addUndo) {
		//Add modifier to undo this operation
		StageDeleteModifier* mod = new StageDeleteModifier(manager, name, stage, editMaterial.stages[stage]->stageData);
		manager->AddMaterialUndoModifier(mod);
	}

	//delete the stage and remove it from the list
	delete editMaterial.stages[stage];
	editMaterial.stages.RemoveIndex(stage);

	manager->StageDeleted(this, stage);

	OnMaterialChanged();
}

/**
* Removes all stages from the material.
*/
void MaterialDoc::ClearStages() {

	//Delete each stage and clear the list
	for(int i = GetStageCount() - 1; i >= 0; i--) {
		RemoveStage(i);
	}
}

/**
* Moves a stage from one location to another.
* @param from The original location of the stage.
* @param to The new location of the stage.
* @param addUndo Flag that specifies if the system should add an undo operation.
*/
void MaterialDoc::MoveStage(int from, int to, bool addUndo) {
	assert(from >= 0 && from < GetStageCount());
	assert(to >= 0 && to < GetStageCount());

	int origFrom = from;
	int origTo = to;

	if(from < to)
		to++;

	MEStage_t* pMove = editMaterial.stages[from];
	editMaterial.stages.Insert(pMove, to);

	if(from > to)
		from++;

	editMaterial.stages.RemoveIndex(from);

	manager->StageMoved(this, origFrom, origTo);

	if(addUndo) {
		StageMoveModifier *mod = new StageMoveModifier(manager, name, origFrom, origTo);
		manager->AddMaterialUndoModifier(mod);
	}

	OnMaterialChanged();
}

/**
* Applies any changes to the material
* @param force If true then the material will be applied regardless of the number of changes.
*/
void MaterialDoc::ApplyMaterialChanges(bool force) {

	if(force || applyWaiting) {

		if(sourceModify && sourceModifyOwner) {
			idStr text = sourceModifyOwner->GetSourceText();
			ApplySourceModify(text);
		}

		ReplaceSourceText();

		char *declText = (char *) _alloca( renderMaterial->GetTextLength() + 1 );
		renderMaterial->GetText( declText );

		renderMaterial->GetText(declText);
		
		ParseMaterialText(declText);

		applyWaiting = false;

		assert(manager);
		manager->MaterialApplied(this);
	}
}

/**
* Saves the material.
*/
void MaterialDoc::Save() {

	EnableAllStages(true);

	//Apply the material so that the renderMaterial has the source text
	if(!deleted) {
		ApplyMaterialChanges(true);
	} else {
		//Replace the text with nothing
		renderMaterial->SetText(" ");
	}

	if(renderMaterial->Save()) {

		modified = false;

		//Notify the world
		assert(manager);
		manager->MaterialSaved(this);
	} else {
		MessageBox(GetMaterialEditorWindow(), va("Unable to save '%s'. It may be read-only", name.c_str()), "Save Error", MB_OK | MB_ICONERROR);
	}
}

/**
* Deletes the material.
*/
void MaterialDoc::Delete() {
	deleted = true;

	OnMaterialChanged();
}

/**
* Sets the proper internal states and notifies the MaterialDocManager once a material has been changed.
*/
void MaterialDoc::OnMaterialChanged() {

	modified = true;
	applyWaiting = true;

	assert(manager);
	manager->MaterialChanged(this);
}

/**
* Passes text to a render material for parsing.
* @param source The text that sould be applied to the idMaterial.
*/
void MaterialDoc::ParseMaterialText(const char* source) {

	/*idLexer src;
	src.LoadMemory(source, strlen(source), "material");
	src.SetFlags( 
		LEXFL_NOSTRINGCONCAT |			// multiple strings seperated by whitespaces are not concatenated
		LEXFL_NOSTRINGESCAPECHARS |		// no escape characters inside strings
		LEXFL_ALLOWPATHNAMES |			// allow path seperators in names
		LEXFL_ALLOWMULTICHARLITERALS |	// allow multi character literals
		LEXFL_ALLOWBACKSLASHSTRINGCONCAT |	// allow multiple strings seperated by '\' to be concatenated
		LEXFL_NOFATALERRORS				// just set a flag instead of fatal erroring
		);

	//Skip the name becuase the material parsing code expects it
	src.SkipUntilString("{");*/

	//Now let the material parse the text
	renderMaterial->Parse(source, strlen(source));
}

/**
* Parses the source text from an idMaterial and initializes the editor dictionary representation
* of the material.
* @param src The idLexer object that contains the material text.
*/
void MaterialDoc::ParseMaterial(idLexer* src) {

	idToken		token;

	//Parse past the name
	src->SkipUntilString("{");
	
	while ( 1 ) {
		if ( !src->ExpectAnyToken( &token ) ) {
			//Todo: Add some error checking here
			return;
		}

		if ( token == "}" ) {
			break;
		}

		if(ParseMaterialDef(&token, src, MaterialDefManager::MATERIAL_DEF_MATERIAL, &editMaterial.materialData)) {
			continue;
		}
		
		if ( !token.Icmp( "diffusemap" ) ) {
			//Added as a special stage
			idStr str;
			src->ReadRestOfLine( str );
			AddSpecialMapStage("diffusemap", str);
		}
		else if ( !token.Icmp( "specularmap" ) ) {
			idStr str;
			src->ReadRestOfLine( str );
			AddSpecialMapStage("specularmap", str);
		}
		else if ( !token.Icmp( "bumpmap" ) ) {
			idStr str;
			src->ReadRestOfLine( str );
			AddSpecialMapStage("bumpmap", str);
		}
		else if( token == "{" ) {
			ParseStage(src);
		}
	}
}

/**
* Parses a single stage from the source text from an idMaterial and initializes the editor dictionary 
* representation of the material.
* @param src The idLexer object that contains the material text.
*/
void MaterialDoc::ParseStage(idLexer* src) {
	
	MEStage_t* newStage = new MEStage_t();
	int index = editMaterial.stages.Append(newStage);
	
	newStage->stageData.SetInt("stagetype", STAGE_TYPE_NORMAL);
	newStage->enabled = true;

	idToken		token;

	while ( 1 ) {
		
		if ( !src->ExpectAnyToken( &token ) ) {
			//Todo: Add some error checking here
			return;
		}

		if ( token == "}" ) {
			break;
		}

		if(ParseMaterialDef(&token, src, MaterialDefManager::MATERIAL_DEF_STAGE, &newStage->stageData)) {
			continue;
		}

		if(!token.Icmp("name")) {

			idStr str;
			src->ReadRestOfLine( str );
			str.StripTrailing('\"');
			str.StripLeading('\"');
			newStage->stageData.Set("name", str);
			continue;
		}
	}

	idStr name;
	newStage->stageData.GetString("name", "", name);
	if(name.Length() <= 0)
		newStage->stageData.Set("name", va("Stage %d", index+1));

}

/**
* Adds a special stage to the material.
* @param stageName The name of the special stage bumpmap, diffusemap or specularmap
* @param map The map for the special stage.
*/
void MaterialDoc::AddSpecialMapStage(const char* stageName, const char* map) {
	MEStage_t* newStage = new MEStage_t();
	int index = editMaterial.stages.Append(newStage);
	newStage->stageData.Set("name", stageName);
	newStage->stageData.Set("map", map);
	newStage->stageData.SetInt("stagetype", STAGE_TYPE_SPECIALMAP);
	newStage->enabled = true;
}

/**
* Finds the appropriate material definition for the supplied token and initializes the
* internal dictionary data.
* @param token The token to lookup
* @param src The idLexer that contains the material source text.
* @param type The type of attribute grouping to use material, stage or special stage.
* @param dict The dictionary to initialize.
*/
bool MaterialDoc::ParseMaterialDef(idToken* token, idLexer* src, int type, idDict* dict) {
	
	MaterialDefList* defs = MaterialDefManager::GetMaterialDefs(type);

	for(int i = 0; i < defs->Num(); i++) {
		if(!token->Icmp((*defs)[i]->dictName)) {

			switch((*defs)[i]->type) {
					case MaterialDef::MATERIAL_DEF_TYPE_STRING:
						{
							idStr str;
							src->ReadRestOfLine( str );
							if((*defs)[i]->quotes) {
								str.StripTrailing('\"');
								str.StripLeading('\"');
							}
							dict->Set((*defs)[i]->dictName, str);
						}
						break;
					case MaterialDef::MATERIAL_DEF_TYPE_BOOL:
						{
							src->SkipRestOfLine();
							dict->SetBool((*defs)[i]->dictName, true);
						}
						break;
					case MaterialDef::MATERIAL_DEF_TYPE_FLOAT:
						{
							idStr str;
							src->ReadRestOfLine( str );
							dict->Set((*defs)[i]->dictName, str);
						}
						break;
					case MaterialDef::MATERIAL_DEF_TYPE_INT:
						{
							idStr str;
							src->ReadRestOfLine( str );
							dict->Set((*defs)[i]->dictName, str);
						}
						break;
			}
			return true;
		}
	}
	return false;
}

/**
* Cleans up the edit material by deleting the stage data structures.
*/
void MaterialDoc::ClearEditMaterial() {

	for(int i = 0; i < GetStageCount(); i++) {
		delete editMaterial.stages[i];
	}
	editMaterial.stages.Clear();
	editMaterial.materialData.Clear();
}

/**
* Writes the internal dictionary data to the standard format.
*/
const char*	MaterialDoc::GenerateSourceText() {

	idFile_Memory f;

	f.WriteFloatString("\n\n/*\n"
		"\tGenerated by the Material Editor.\n"
		"\tType 'materialeditor' at the console to launch the material editor.\n"
		"*/\n" );

	f.WriteFloatString("%s\n", name.c_str());
	f.WriteFloatString( "{\n" );
	WriteMaterialDef(-1, &f, MaterialDefManager::MATERIAL_DEF_MATERIAL, 1);

	for(int i = 0; i < editMaterial.stages.Num(); i++) {
		if(editMaterial.stages[i]->enabled) {
			WriteStage(i, &f);
		}
	}

	f.WriteFloatString( "}\n" );

	return f.GetDataPtr();

}

/**
* Writes the internal dictionary data to the standard format and replaces the 
* idMaterial source text with the newly generated text.
*/
void MaterialDoc::ReplaceSourceText() {
		renderMaterial->SetText(GenerateSourceText());
}

/**
* Writes a single stage.
* @param stage The stage to write.
* @param file The file where the stage should be wirtten
*/
void MaterialDoc::WriteStage(int stage, idFile_Memory* file) {

	//idStr stageName = GetAttribute(stage, "name");
	int type = GetAttributeInt(stage, "stagetype");
	//if(!stageName.Icmp("diffusemap") || !stageName.Icmp("specularmap") || !stageName.Icmp("bumpmap")) {
	if(type == STAGE_TYPE_SPECIALMAP) {
		WriteSpecialMapStage(stage, file);
		return;
	}

	file->WriteFloatString( "\t{\n" );
	idStr name = GetAttribute(stage, "name");
	if(name.Length() > 0) {
		file->WriteFloatString("\t\tname\t\"%s\"\n", name.c_str());
	}
	WriteMaterialDef(stage, file, MaterialDefManager::MATERIAL_DEF_STAGE, 2);
	file->WriteFloatString( "\t}\n" );
	
}

/**
* Writes a single special stage.
* @param stage The stage to write.
* @param file The file where the stage should be wirtten
*/
void MaterialDoc::WriteSpecialMapStage(int stage, idFile_Memory* file) {
	idStr stageName = GetAttribute(stage, "name");
	idStr map = GetAttribute(stage, "map");

	file->WriteFloatString( "\t%s\t%s\n", stageName.c_str(), map.c_str() );
}

/**
* Writes a set of material attributes to a file.
* @param stage The stage to write or -1 for the material.
* @param file The file where the stage should be wirtten.
* @param type The attribute grouping to use.
* @param indent The number of tabs to indent the text.
*/
void MaterialDoc::WriteMaterialDef(int stage, idFile_Memory* file, int type, int indent) {

	idStr prefix = "";
	for(int i = 0; i < indent; i++) {
		prefix += "\t";
	}

	MaterialDefList* defs = MaterialDefManager::GetMaterialDefs(type);
	for(int i = 0; i < defs->Num(); i++) {
		switch((*defs)[i]->type) {
			case MaterialDef::MATERIAL_DEF_TYPE_STRING:
				{
					idStr attrib = GetAttribute(stage, (*defs)[i]->dictName);
					if(attrib.Length() > 0) {
						if((*defs)[i]->quotes)
							file->WriteFloatString("%s%s\t\"%s\"\n", prefix.c_str(), (*defs)[i]->dictName.c_str(), attrib.c_str());
						else
							file->WriteFloatString("%s%s\t%s\n", prefix.c_str(), (*defs)[i]->dictName.c_str(), attrib.c_str());
					}
				}
				break;
			case MaterialDef::MATERIAL_DEF_TYPE_BOOL:
				{
					if(GetAttributeBool(stage, (*defs)[i]->dictName))
						file->WriteFloatString("%s%s\t\n",prefix.c_str(), (*defs)[i]->dictName.c_str());
				}
				break;
			case MaterialDef::MATERIAL_DEF_TYPE_FLOAT:
				{
					float val = GetAttributeFloat(stage, (*defs)[i]->dictName);
					file->WriteFloatString("%s%s\t%f\n", prefix.c_str(), (*defs)[i]->dictName.c_str(), val);
				}
				break;
			case MaterialDef::MATERIAL_DEF_TYPE_INT:
				{
					int val = GetAttributeInt(stage, (*defs)[i]->dictName);
					file->WriteFloatString("%s%s\t%d\n", prefix.c_str(), (*defs)[i]->dictName.c_str(), val);
				}
				break;
		}
	}
}


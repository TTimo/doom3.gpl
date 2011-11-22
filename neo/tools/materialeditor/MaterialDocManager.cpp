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

#include "MaterialDocManager.h"
#include "MaterialView.h"


/**
* Constructor for MaterialDocManager.
*/
MaterialDocManager::MaterialDocManager(void) {
	currentMaterial = NULL;
	cutMaterial = false;
}

/**
* Destructor for MaterialDocManager.
*/
MaterialDocManager::~MaterialDocManager(void) {
	UnRegisterAllMaterialViews();

	ClearUndo();
	ClearRedo();
}

/**
* Registers an object to receive notifications about changes made to materials.
* @param view The object that would like to receive material notifications.
*/
void MaterialDocManager::RegisterMaterialView(MaterialView* view) {
	ASSERT(view);
	UnRegisterMaterialView(view);
	materialViews.Append(view);

	//Notify the view of myself
	view->SetMaterialDocManager(this);
}

/**
* Tells the MaterialDocManager to stop sending notifications to a view.
* @param view The view that no longer wants notifications.
*/
void MaterialDocManager::UnRegisterMaterialView(MaterialView* view) {
	ASSERT(view);
	materialViews.Remove(view);

	//Remove the reference to myself
	view->SetMaterialDocManager(NULL);
}

/**
* Unregisters all of the views that are registered to get material change
* notifications.
*/
void MaterialDocManager::UnRegisterAllMaterialViews() {
	
	//Remove the reference to myself
	int c = materialViews.Num();
	for(int i = 0; i < c; i++) {
		materialViews[i]->SetMaterialDocManager(NULL);
	}
	materialViews.Clear();
}

/**
* Tells the MaterialDocManager which material has been selected for editing.
* @param material The material that has been selected.
*/
void MaterialDocManager::SetSelectedMaterial(idMaterial* material) {

	bool change = false;

	//Do we need to change the material
	if(material) {
		if(currentMaterial) {
			if(strcmp(material->GetName(), currentMaterial->renderMaterial->GetName())) {
				change = true;
			}
		} else {
			change = true;
		}
	} else {
		if(currentMaterial) {
			change = true;
		}
	}
	
	//Now make the change
	if(change) {
		if(currentMaterial) {

			//Delete the material unless it has been changed
			if(!inProgressMaterials.Get(currentMaterial->name.c_str())) {
				delete currentMaterial;
				currentMaterial = NULL;
			}
		}

		MaterialDoc** tempDoc;
		if(material && inProgressMaterials.Get(material->GetName(), &tempDoc)) {
			currentMaterial = *tempDoc;
			
		} else {
			currentMaterial = CreateMaterialDoc(material);
		}

		NotifyViews(currentMaterial, SELECTION_CHANGE);
	}
}

/**
* Returns true if the specified file needs to be applied and false otherwise.
*/
bool MaterialDocManager::DoesFileNeedApply(const char* filename) {
	for(int i = 0; i < inProgressMaterials.Num(); i++) {
		MaterialDoc** pDoc = inProgressMaterials.GetIndex(i);
		if(!strcmp((*pDoc)->renderMaterial->GetFileName(), filename) && (*pDoc)->applyWaiting)
			return true;
	}
	return false;
}

/**
* Returns true if any material needs to be applied.
*/
bool MaterialDocManager::DoesAnyNeedApply() {
	for(int i = 0; i < inProgressMaterials.Num(); i++) {
		MaterialDoc** pDoc = inProgressMaterials.GetIndex(i);
		if((*pDoc)->applyWaiting)
			return true;
	}
	return false;
}

/**
* Returns true if the specified file has been modified.
*/
bool MaterialDocManager::IsFileModified(const char* filename) {
	for(int i = 0; i < inProgressMaterials.Num(); i++) {
		MaterialDoc** pDoc = inProgressMaterials.GetIndex(i);
		if(!strcmp((*pDoc)->renderMaterial->GetFileName(), filename))
			return true;
	}
	return false;
}

/**
* Returns true if any material has been modified.
*/
bool MaterialDocManager::IsAnyModified() {
	return (inProgressMaterials.Num() > 0);
}

/**
* Adds a material.
* @param name The name of the material.
* @param filename The file to place the material in.
* @param sourceText The initial material definition.
* @param addUndo Can this operation be undone.
*/
void MaterialDocManager::AddMaterial(const char* name, const char* filename, const char* sourceText, bool addUndo) {
	
	if(addUndo) {
		AddMaterialModifier* mod = new AddMaterialModifier(this, name, filename);
		AddMaterialUndoModifier(mod);
	}

	MaterialDoc* newDoc = new MaterialDoc();
	newDoc->manager = this;
	newDoc->modified = true;
	
	idMaterial* rendMat = (idMaterial*)declManager->CreateNewDecl(DECL_MATERIAL, name, filename);
	
	if(sourceText) {
		rendMat->SetText(sourceText);
	}

	newDoc->SetRenderMaterial(rendMat, true, sourceText ? true : false);

	inProgressMaterials.Set(newDoc->name.c_str(), newDoc);
	
	NotifyViews(newDoc, MATERIAL_ADD);

	//Force an apply so the text will be generated to match the new file
	newDoc->applyWaiting = true;
	newDoc->ApplyMaterialChanges();
}

/**
* Used to redo an add material and undo a delete material. 
* The undo for adding a material deletes the material. Instead of adding a completely
* new material RedoAddMaterial finds the one that was just deleted and uses that. 
* @param name The name of the material that was added/deleted.
* @param clearData Should the material definition be reset to the default definition.
*/
void MaterialDocManager::RedoAddMaterial(const char* name, bool clearData) {

	MaterialDoc* newDoc = new MaterialDoc();
	newDoc->manager = this;
	newDoc->modified = true;

	idMaterial* rendMat = const_cast<idMaterial *>(declManager->FindMaterial(name, false));

	if(clearData) {
		rendMat->SetText(rendMat->DefaultDefinition());
	}

	newDoc->SetRenderMaterial(rendMat, true, true);

	inProgressMaterials.Set(newDoc->name.c_str(), newDoc);

	NotifyViews(newDoc, MATERIAL_ADD);

	//Force an apply so the text will be generated to match the new file
	newDoc->applyWaiting = true;
	newDoc->ApplyMaterialChanges();
}

/**
* Deletes a material.
* @param material The material to be deleted.
* @param addUndo Can this operation be undone.
*/
void MaterialDocManager::DeleteMaterial(MaterialDoc* material, bool addUndo) {
	
	assert(material);

	//This will just flag for delete. The actual delete will happen during the save
	material->Delete();

	if(addUndo) {
		DeleteMaterialModifier* mod = new DeleteMaterialModifier(this, material->name);
		AddMaterialUndoModifier(mod);
	}

	NotifyViews(material, MATERIAL_DELETE);
}

/**
* Applys changes to a material.
* @param materialDoc The material to be applied.
*/
void MaterialDocManager::ApplyMaterial(MaterialDoc* materialDoc) {
	assert(materialDoc);
	materialDoc->ApplyMaterialChanges();
}

/**
* Applies all materials in the specified filename.
* @param filename The file to apply.
*/
void MaterialDocManager::ApplyFile(const char* filename) {
	
	for(int i = 0; i < inProgressMaterials.Num(); i++) {
		MaterialDoc** pDoc = inProgressMaterials.GetIndex(i);
		if(!strcmp((*pDoc)->renderMaterial->GetFileName(), filename))
			(*pDoc)->ApplyMaterialChanges();
	}
}

/**
* Applies all materials that have been changed.
*/
void MaterialDocManager::ApplyAll() {
	for(int i = 0; i < inProgressMaterials.Num(); i++) {
		MaterialDoc** pDoc = inProgressMaterials.GetIndex(i);
		(*pDoc)->ApplyMaterialChanges();
	}
}

/**
* Saves a single material.
* @param material The material to save.
*/
void MaterialDocManager::SaveMaterial(MaterialDoc* material) {
	assert(material);
	material->Save();
}

/**
* Saves all materials in the specified file.
* @param filename The file to save.
*/
void MaterialDocManager::SaveFile(const char* filename) {
	
	for(int i = inProgressMaterials.Num()-1; i >= 0; i--) {
		MaterialDoc** pDoc = inProgressMaterials.GetIndex(i);
		if(!strcmp((*pDoc)->renderMaterial->GetFileName(), filename))
			(*pDoc)->Save();
	}

	//Notify everyone
	NotifyViews(NULL, MATERIAL_SAVE_FILE, filename);
}

/**
* Saves all materials that have been changed.
*/
void MaterialDocManager::SaveAllMaterials() {
	for(int i = inProgressMaterials.Num()-1; i >= 0; i--) {
		MaterialDoc** pDoc = inProgressMaterials.GetIndex(i);
		(*pDoc)->Save();
	}
}

/**
* Reloads a specified file.
* @param filename The file to reload.
*/
void MaterialDocManager::ReloadFile(const char *filename) {
	
	declManager->ReloadFile(filename, true);
			
	//purge the changes of any in progress materials
	for(int j = inProgressMaterials.Num()-1; j >= 0; j--) {
		MaterialDoc** pDoc = inProgressMaterials.GetIndex(j);
		if(!strcmp((*pDoc)->renderMaterial->GetFileName(), filename)) {
			(*pDoc)->SetRenderMaterial((*pDoc)->renderMaterial);
			inProgressMaterials.Remove((*pDoc)->name);
		}
	}
	
	//Reparse the current material
	if(currentMaterial) {
		currentMaterial->SetRenderMaterial(currentMaterial->renderMaterial);

		//Trigger all the views to refresh
		NotifyViews(currentMaterial, SELECTION_CHANGE);
	}

	NotifyViews(NULL, FILE_RELOAD, filename);
}

/**
* Creates a MaterialDoc object for the specified material name. If a MaterialDoc 
* object already exists then it is used.
* @param materialName The name of the material for which to create a MaterialDoc object.
*/
MaterialDoc* MaterialDocManager::CreateMaterialDoc(const char* materialName) {
	
	const idMaterial* material = declManager->FindMaterial(materialName);
	return CreateMaterialDoc(const_cast<idMaterial *>(material));
}

/**
* Creates a MaterialDoc object for the specified material. If a MaterialDoc 
* object already exists then it is used.
* @param material The material for which to create a MaterialDoc object.
*/
MaterialDoc* MaterialDocManager::CreateMaterialDoc(idMaterial* material) {

	MaterialDoc* existingDoc = GetInProgressDoc(material);
	if(existingDoc) {
		return existingDoc;
	}

	if(currentMaterial && material && !currentMaterial->name.Icmp(material->GetName())) {
		return currentMaterial;
	}

	if(material) {
		MaterialDoc* newDoc = new MaterialDoc();
		newDoc->manager = this;
		newDoc->SetRenderMaterial(material);
		
		return newDoc;
	}

	return NULL;
}

/**
* Checks the current list of in progress MaterialDoc objects to see if
* a MaterialDoc object already exists.
* @param material The material to check for.
*/
MaterialDoc* MaterialDocManager::GetInProgressDoc(idMaterial* material) {

	if(material) {
		for(int i = 0; i < inProgressMaterials.Num(); i++) {
			MaterialDoc** pDoc = inProgressMaterials.GetIndex(i);

			if(!(*pDoc)->name.Icmp(material->GetName()))
				return *pDoc;
		}
	}
	return NULL;
}

/**
* Prepares a material for a copy/cut and paste operations.
* @param materialDoc The material to copy.
* @param cut Is this a cut operation.
*/
void MaterialDocManager::CopyMaterial(MaterialDoc* materialDoc, bool cut) {
	
	cutMaterial = cut;

	if(materialDoc)
		copyMaterial = materialDoc->name;
	else
		ClearCopy();
}

/**
* Clears the copy buffer for a material.
*/
void MaterialDocManager::ClearCopy() {
	copyMaterial.Empty();
}

/**
* Returns true if there is a material in the copy buffer.
*/
bool MaterialDocManager::IsCopyMaterial() {
	return (copyMaterial.Length() ) ? true : false;
}

/**
* Returns the name of the material in the copy buffer.
*/
idStr MaterialDocManager::GetCopyMaterialName() {
	return copyMaterial;
}

/**
* Performs a material paste operation for a material in the copy buffer.
* @param name The new name for the material that is being copied.
* @param filename The file to paste the material in.
*/
void MaterialDocManager::PasteMaterial(const char* name, const char* filename) {
	
	if(!IsCopyMaterial()) {
		return;
	}

	//Apply the material if there are some changes
	MaterialDoc* copyMat = CreateMaterialDoc(copyMaterial);
	if(copyMat->applyWaiting) {
		copyMat->ApplyMaterialChanges();
	}
	//Paste the material
	idMaterial* material = copyMat->renderMaterial;

	//Add a material with the existing source text
	char *declText = (char *) _alloca( material->GetTextLength() + 1 );
	material->GetText( declText );

	AddMaterial(name, filename, declText, !cutMaterial);

	//If this is a cut then remove the original
	if(cutMaterial) {
		MaterialDoc* cutMaterial = CreateMaterialDoc(material);
		DeleteMaterial(cutMaterial, false);

		MoveMaterialModifier* mod = new MoveMaterialModifier(this, name, filename, copyMaterial);
		AddMaterialUndoModifier(mod);

		ClearCopy();
	}
	
}

/**
* Prepares a material stage for a copy/paste operation.
* @param materialDoc The materialDoc that contains the stage to be copied.
* @param stageNum the stage to copy.
*/
void MaterialDocManager::CopyStage(MaterialDoc* materialDoc, int stageNum) {

	assert(materialDoc);

	copyStageMaterial = materialDoc->name;
	copyStage = materialDoc->GetStage(stageNum);
	
	idStr stageName = copyStage.stageData.GetString("name");
}

/**
* Clears the copy buffer for copied stages.
*/
void MaterialDocManager::ClearCopyStage() {
	copyStageMaterial.Empty();
	copyStage.stageData.Clear();
}

/**
* Returns true if there is a stage in the copy buffer.
*/
bool MaterialDocManager::IsCopyStage() {
	return (copyStageMaterial.Length() ) ? true : false;
}

/**
* Performs a paste operation of the stage in the copy buffer.
* @param materialDoc The materialDoc to paste the stage in.
*/
void MaterialDocManager::PasteStage(MaterialDoc* materialDoc) {

	assert(materialDoc);

	int stageType = copyStage.stageData.GetInt("stagetype");

	//Create a new stage and copy the data
	materialDoc->AddStage(stageType, copyStage.stageData.GetString("name"));
	materialDoc->SetData(materialDoc->GetStageCount()-1, &copyStage.stageData);
}

/**
* Returns information about the stage in the copy buffer.
* @param type Holds the type of the stage in the copy buffer.
* @param name Hold the name of the stage in the copy buffer.
*/
void MaterialDocManager::GetCopyStageInfo(int& type, idStr& name) {
	if(IsCopyStage()) {
		type = copyStage.stageData.GetInt("stagetype");
		name = copyStage.stageData.GetString("name");
	}
}

/**
* Performs the first available undo operation.
*/
void MaterialDocManager::Undo() {

	if(IsUndoAvailable()) {
		MaterialModifier* mod = undoModifiers[undoModifiers.Num()-1];
		undoModifiers.RemoveIndex(undoModifiers.Num()-1);

		mod->Undo();

		//Add this modifier to the redo list
		AddMaterialRedoModifier(mod);
	}
}

/**
* Returns true if an undo operation is available.
*/
bool MaterialDocManager::IsUndoAvailable() {
	return (undoModifiers.Num() > 0);
}

/**
* Clears the entire undo buffer.
*/
void MaterialDocManager::ClearUndo() {

	int c = undoModifiers.Num();
	for(int i = 0; i < c; i++) {
		delete undoModifiers[i];
	}
	undoModifiers.Clear();
}

/**
* Performs the first available redo operation.
*/
void MaterialDocManager::Redo() {

	if(IsRedoAvailable()) {
		MaterialModifier* mod = redoModifiers[redoModifiers.Num()-1];
		redoModifiers.RemoveIndex(redoModifiers.Num()-1);

		mod->Redo();

		//Done with the mod because the redo process will set 
		//attributes and create the appropriate redo modifier
		AddMaterialUndoModifier(mod, false);		
	}
}

/**
* Returns true if a redo operation is available.
*/
bool MaterialDocManager::IsRedoAvailable() {
	return (redoModifiers.Num() > 0);
}

/**
* Clears the redo buffer.
*/
void MaterialDocManager::ClearRedo() {

	int c = redoModifiers.Num();
	for(int i = 0; i < c; i++) {
		delete redoModifiers[i];
	}
	redoModifiers.Clear();
}

/**
* Adds an undo operation to the undo buffer.
* @param mod The MaterialModifier object that contains the undo data.
* @param clearRedo Should we clear the redo buffer.
*/
void  MaterialDocManager::AddMaterialUndoModifier(MaterialModifier* mod, bool clearRedo) {
	undoModifiers.Append(mod);

	while(undoModifiers.Num() > MAX_UNDOREDO) {
		undoModifiers.RemoveIndex(0);
	}

	if(clearRedo) {
		ClearRedo();
	}
}

/**
* Adds a redo operation to the redo buffer.
* @param mod The MaterialModifier object that contains the redo data.
*/
void  MaterialDocManager::AddMaterialRedoModifier(MaterialModifier* mod) {
	redoModifiers.Append(mod);

	while(redoModifiers.Num() > MAX_UNDOREDO) {
		redoModifiers.RemoveIndex(0);
	}
}

/**
* Searches for a material that matches the specified search data.
* @param name The name of the material to search.
* @param searchData The search parameters.
* @param checkName If true then the name of the material will be checked along with the material text.
*/
bool MaterialDocManager::FindMaterial(const char* name, MaterialSearchData_t* searchData, bool checkName) {

	//Fast way of finding the material without parsing
	const idMaterial* material = static_cast<const idMaterial *>(declManager->FindDeclWithoutParsing(DECL_MATERIAL, name, false));
		
	if(material) {

		int findPos;

		if(checkName) {
			//Check the name
			idStr name = material->GetName();

			findPos = name.Find(searchData->searchText, false);
			if(findPos != -1) {
				return true;
			}
		}

		//Skip to the open braket so the name is not checked
		char *declText = (char *) _alloca( material->GetTextLength() + 1 );
		material->GetText( declText );

		idStr text = declText;
		int start = text.Find("{");
		if(start != -1) {
			text = text.Right(text.Length()-start);
		}
		
		findPos = text.Find(searchData->searchText, false);
		if(findPos != -1) {
			//Todo: Include match whole word
			return true;
		}
	}
	return false;
}

/**
* Returns a unique material name given a base name. This is used to resolve materials with the same name.
* @param name The base name of the material.
*/
idStr MaterialDocManager::GetUniqueMaterialName(idStr name) {
	int num = 0;
	while(1) {
		idStr testName;
		if(num == 0)
			testName = name;
		else
			testName = va("%s%d", name.c_str(), num);

		const idMaterial* mat = declManager->FindMaterial(testName.c_str(), false);
		if(!mat) {
			return testName;
		} else {

			//We can reuse delete material names
			if(mat->GetTextLength() < 1)
				return testName;
		}
		num++;
	}
}

/**
* Notifies all registered views of a material event.
* @param materialDoc The material that has been affected.
* @param notifyType The type of event that has occured.
* @param ... Notification specific data. See MaterialView.
*/
void MaterialDocManager::NotifyViews(MaterialDoc* materialDoc, int notifyType, ... ) {

	va_list argptr;

	int c = materialViews.Num();
	for(int i = 0; i < c; i++) {
		va_start( argptr, notifyType );
		switch(notifyType) {
			case SELECTION_CHANGE:
				materialViews[i]->MV_OnMaterialSelectionChange(materialDoc);
				break;
			case MATERIAL_CHANGE:
				materialViews[i]->MV_OnMaterialChange(materialDoc);
				break;
			case MATERIAL_APPLY:
				materialViews[i]->MV_OnMaterialApply(materialDoc);
				break;
			case MATERIAL_SAVE:
				materialViews[i]->MV_OnMaterialSaved(materialDoc);
				break;
			case MATERIAL_SAVE_FILE:
				materialViews[i]->MV_OnMaterialSaveFile(va_arg(argptr, const char*));
				break;
			case MATERIAL_ADD:
				materialViews[i]->MV_OnMaterialAdd(materialDoc);
				break;
			case MATERIAL_DELETE:
				materialViews[i]->MV_OnMaterialDelete(materialDoc);
				break;
			case MATERIAL_ADD_STAGE:
				materialViews[i]->MV_OnMaterialStageAdd(materialDoc, va_arg(argptr, int));
				break;
			case MATERIAL_DELETE_STAGE:
				materialViews[i]->MV_OnMaterialStageDelete(materialDoc, va_arg(argptr, int));
				break;
			case MATERIAL_MOVE_STAGE:
				{
					int from = va_arg(argptr, int);
					int to = va_arg(argptr, int);
					materialViews[i]->MV_OnMaterialStageMove(materialDoc, from, to);
				}
				break;
			case MATERIAL_ATTRIBUTE_CHANGE:
				{
					int stage = va_arg(argptr, int);
					const char* attribName = va_arg(argptr, const char*);
					materialViews[i]->MV_OnMaterialAttributeChanged(materialDoc, stage, attribName);
				}
				break;
			case MATERIAL_NAME_CHANGE:
				{
					const char* oldName = va_arg(argptr, const char*);
					materialViews[i]->MV_OnMaterialNameChanged(materialDoc, oldName);
				}
				break;
			case FILE_RELOAD:
				{
					const char* filename = va_arg(argptr, const char*);
					materialViews[i]->MV_OnFileReload(filename);
				}
				break;
		}
		va_end( argptr );
	}
}

/**
* Called when a material has been edited and notifies all views of the change.
* @param materialDoc The material that has changed.
*/
void MaterialDocManager::MaterialChanged(MaterialDoc* materialDoc) {

	//Make sure this material is in our list of changed materials
	if(!inProgressMaterials.Get(materialDoc->name.c_str())) {
		inProgressMaterials.Set(materialDoc->name.c_str(), materialDoc);
	}

	//Notify everyone
	NotifyViews(materialDoc, MATERIAL_CHANGE);
}

/**
* Called when a material has been applied and notifies all views of the apply.
* @param materialDoc The material that has been applied.
*/
void MaterialDocManager::MaterialApplied(MaterialDoc* materialDoc) {

	//Notify everyone
	NotifyViews(materialDoc, MATERIAL_APPLY);
}

/**
* Called when a material has been saved and notifies all views of the save.
* @param materialDoc The material that has been saved.
*/
void MaterialDocManager::MaterialSaved(MaterialDoc* materialDoc) {

	MaterialDoc** tempDoc;
	if(inProgressMaterials.Get(materialDoc->name.c_str(), &tempDoc)) {

		idStr name = materialDoc->name.c_str();

		//Remove this file from our in progress list
		inProgressMaterials.Remove(name.c_str());

		//Notify everyone
		NotifyViews(materialDoc, MATERIAL_SAVE);

		if(materialDoc != currentMaterial)
			delete materialDoc;
	}
}

/**
* Called when a material name has been changed and notifies all views of the change.
* @param materialDoc The material that has changed.
*/
void MaterialDocManager::MaterialNameChanged(const char* oldName, MaterialDoc* materialDoc) {

	MaterialDoc** tempDoc;
	if(inProgressMaterials.Get(oldName, &tempDoc)) {
		inProgressMaterials.Set(materialDoc->name, *tempDoc);
		inProgressMaterials.Remove(oldName);
	}

	NotifyViews(materialDoc, MATERIAL_NAME_CHANGE, oldName);
}

/**
* Called when a stage is added and notifies all views of the addition.
* @param materialDoc The material that has changed.
* @param stageNum The stage that was added.
*/
void MaterialDocManager::StageAdded(MaterialDoc* materialDoc, int stageNum) {
	//Notify everyone
	NotifyViews(materialDoc, MATERIAL_ADD_STAGE, stageNum);
}

/**
* Called when a stage has been deleted and notifies all views of the change.
* @param materialDoc The material that has changed.
* @param stageNum The stage that was deleted.
*/
void MaterialDocManager::StageDeleted(MaterialDoc* materialDoc, int stageNum) {
	//Notify everyone
	NotifyViews(materialDoc, MATERIAL_DELETE_STAGE, stageNum);
}

/**
* Called when a stage has been movied and notifies all views of the change.
* @param materialDoc The material that has changed.
* @param from The original position of the stage.
* @param to The new position of the stage.
*/
void MaterialDocManager::StageMoved(MaterialDoc* materialDoc, int from, int to) {
	//Notify everyone
	NotifyViews(materialDoc, MATERIAL_MOVE_STAGE, from, to);
}

/**
* Called when a material attribute has been edited and notifies all views of the change.
* @param materialDoc The material that has changed.
* @param stage The stage that contains the changed attribute.
* @param attribName The name of the attribute that changed.
*/
void MaterialDocManager::AttributeChanged(MaterialDoc* materialDoc, int stage, const char* attribName) {
	//Notify everyone
	NotifyViews(materialDoc, MATERIAL_ATTRIBUTE_CHANGE, stage, attribName);
}

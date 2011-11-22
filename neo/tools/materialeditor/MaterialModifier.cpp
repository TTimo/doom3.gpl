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

#include "MaterialModifier.h"
#include "MaterialDocManager.h"
#include "MaterialTreeView.h"

//////////////////////////////////////////////////////////////////////////////
//MaterialModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for MaterialModifier
*/
MaterialModifier::MaterialModifier(MaterialDocManager* manager, const char* materialName) {
	this->manager = manager;
	this->materialName = materialName;
}

//////////////////////////////////////////////////////////////////////////////
//AttributeMaterialModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for AttributeMaterialModifier
*/
AttributeMaterialModifier::AttributeMaterialModifier(MaterialDocManager* manager, const char* materialName, int stage, const char* key) 
: MaterialModifier(manager, materialName) {
	
	this->stage = stage;
	this->key = key;
	
}

//////////////////////////////////////////////////////////////////////////////
//AttributeMaterialModifierString
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for AttributeMaterialModifierString
*/
AttributeMaterialModifierString::AttributeMaterialModifierString(MaterialDocManager* manager, const char* materialName, int stage, const char* key, const char* value, const char* oldValue) 
: AttributeMaterialModifier(manager, materialName, stage, key) {

	this->value = value;
	this->oldValue = oldValue;
}

/**
* Performs an undo operation of a string attribute change.
*/
void AttributeMaterialModifierString::Undo() {
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->SetAttribute(stage, key, oldValue, false);
}

/**
* Performs a redo operation of a string attribute change.
*/
void AttributeMaterialModifierString::Redo() {
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->SetAttribute(stage, key, value, false);
}

//////////////////////////////////////////////////////////////////////////////
//AttributeMaterialModifierBool
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for AttributeMaterialModifierBool
*/
AttributeMaterialModifierBool::AttributeMaterialModifierBool(MaterialDocManager* manager, const char* materialName, int stage, const char* key, bool value, bool oldValue) 
: AttributeMaterialModifier(manager, materialName, stage, key) {

	this->value = value;
	this->oldValue = oldValue;
}

/**
* Performs an undo operation of a boolean attribute change.
*/
void AttributeMaterialModifierBool::Undo() {
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->SetAttributeBool(stage, key, oldValue, false);
}

/**
* Performs a redo operation of a boolean attribute change.
*/
void AttributeMaterialModifierBool::Redo() {
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->SetAttributeBool(stage, key, value, false);
}

//////////////////////////////////////////////////////////////////////////////
//StageMoveModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for StageMoveModifier
*/
StageMoveModifier::StageMoveModifier(MaterialDocManager* manager, const char* materialName, int from, int to)
: MaterialModifier(manager, materialName) {
	this->from = from;
	this->to = to;
}

/**
* Performs an undo operation of a stage move.
*/
void StageMoveModifier::Undo() {
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->MoveStage(to, from, false);
}

/**
* Performs a redo operation of a moved stage.
*/
void StageMoveModifier::Redo() {
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->MoveStage(from, to, false);
}

//////////////////////////////////////////////////////////////////////////////
//StageDeleteModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for StageDeleteModifier
*/
StageDeleteModifier::StageDeleteModifier(MaterialDocManager* manager, const char* materialName, int stageNum, idDict stageData) 
: MaterialModifier(manager, materialName) {
	this->stageNum = stageNum;
	this->stageData = stageData;
}

/**
* Performs an undo operation of a deleted stage.
*/
void StageDeleteModifier::Undo() {

	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->InsertStage(stageNum, stageData.GetInt("stagetype"), stageData.GetString("name"), false);
	material->SetData(stageNum, &stageData);

}

/**
* Performs a redo operation of a deleted stage.
*/
void StageDeleteModifier::Redo() {
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->RemoveStage(stageNum, false);
}

//////////////////////////////////////////////////////////////////////////////
//StageInsertModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for StageInsertModifier
*/
StageInsertModifier::StageInsertModifier(MaterialDocManager* manager, const char* materialName, int stageNum, int stageType, const char* stageName)
: MaterialModifier(manager, materialName) {
	this->stageNum = stageNum;
	this->stageType = stageType;
	this->stageName = stageName;
}

/**
* Performs an undo operation of an inserted stage.
*/
void StageInsertModifier::Undo() {

	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->RemoveStage(stageNum, false);
}

/**
* Performs a redo operation of an inserted stage.
*/
void StageInsertModifier::Redo() {
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->InsertStage(stageNum, stageType, stageName, false);
}

//////////////////////////////////////////////////////////////////////////////
//AddMaterialModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for AddMaterialModifier
*/
AddMaterialModifier::AddMaterialModifier(MaterialDocManager* manager, const char* materialName, const char* materialFile)
: MaterialModifier(manager, materialName) {
	this->materialFile = materialFile;
}

/**
* Performs an undo operation of an added material.
*/
void AddMaterialModifier::Undo() {
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	manager->DeleteMaterial(material, false);
}

/**
* Performs a redo operation of an added material.
*/
void AddMaterialModifier::Redo() {
	manager->RedoAddMaterial(materialName);
}

//////////////////////////////////////////////////////////////////////////////
//DeleteMaterialModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for DeleteMaterialModifier
*/
DeleteMaterialModifier::DeleteMaterialModifier(MaterialDocManager* manager, const char* materialName)
: MaterialModifier(manager, materialName) {
}

/**
* Performs an undo operation of a deleted material.
*/
void DeleteMaterialModifier::Undo() {
	
	manager->RedoAddMaterial(materialName, false);	
}

/**
* Performs a redo operation of a deleted material.
*/
void DeleteMaterialModifier::Redo() {
	
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	manager->DeleteMaterial(material, false);
}


//////////////////////////////////////////////////////////////////////////////
//MoveMaterialModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for MoveMaterialModifier
*/
MoveMaterialModifier::MoveMaterialModifier(MaterialDocManager* manager, const char* materialName, const char* materialFile, const char* copyMaterial)
: MaterialModifier(manager, materialName) {
	this->materialFile = materialFile;
	this->copyMaterial = copyMaterial;
}

/**
* Performs an undo operation of a moved material
*/
void MoveMaterialModifier::Undo() {
	
	//Delete New Material
	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	manager->DeleteMaterial(material, false);

	//RedoAdd Old Material
	manager->RedoAddMaterial(copyMaterial, false);
}

/**
* Performs a redo operation of a moved material.
*/
void MoveMaterialModifier::Redo() {
	//Delete Old Material
	MaterialDoc* material = manager->CreateMaterialDoc(copyMaterial);
	manager->DeleteMaterial(material, false);

	//Redo Add New Material
	manager->RedoAddMaterial(materialName, false);
}

//////////////////////////////////////////////////////////////////////////////
//RenameMaterialModifier
//////////////////////////////////////////////////////////////////////////////
/**
* Constructor for RenameMaterialModifier
*/
RenameMaterialModifier::RenameMaterialModifier(MaterialDocManager* manager, const char* materialName, const char* oldName)
: MaterialModifier(manager, materialName) {
	this->oldName = oldName;
}

/**
* Performs an undo operation of a renamed material
*/
void RenameMaterialModifier::Undo() {

	MaterialDoc* material = manager->CreateMaterialDoc(materialName);
	material->SetMaterialName(oldName, false);
}

/**
* Performs a redo operation of a renamed material.
*/
void RenameMaterialModifier::Redo() {
	
	MaterialDoc* material = manager->CreateMaterialDoc(oldName);
	material->SetMaterialName(materialName, false);
}

//////////////////////////////////////////////////////////////////////////////
//AddMaterialFolderModifier
//////////////////////////////////////////////////////////////////////////////
/**
* Constructor for AddMaterialFolderModifier
*/
AddMaterialFolderModifier::AddMaterialFolderModifier(MaterialDocManager* manager, const char* materialName, MaterialTreeView* view, HTREEITEM item, HTREEITEM parent)
: MaterialModifier(manager, materialName) {
	this->view = view;
	this->item = item;
	this->parent = parent;
}

/**
* Performs an undo operation of an added material folder.
*/
void AddMaterialFolderModifier::Undo() {
	view->DeleteFolder(item, false);
}

/**
* Performs a redo operation of an added material folder.
*/
void AddMaterialFolderModifier::Redo() {
	view->AddFolder(materialName, parent);
}

//////////////////////////////////////////////////////////////////////////////
//RenameMaterialFolderModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for RenameMaterialFolderModifier
*/
RenameMaterialFolderModifier::RenameMaterialFolderModifier(MaterialDocManager* manager, const char* materialName, MaterialTreeView* view, HTREEITEM item, const char* oldName)
: MaterialModifier(manager, materialName) {
	this->view = view;
	this->item = item;
	this->oldName = oldName;
}

/**
* Performs an undo operation of a renamed material folder.
*/
void RenameMaterialFolderModifier::Undo() {
	view->RenameFolder(item, oldName);
}

/**
* Performs a redo operation of a renamed material folder.
*/
void RenameMaterialFolderModifier::Redo() {
	view->RenameFolder(item, materialName);
}


//////////////////////////////////////////////////////////////////////////////
//DeleteMaterialFolderModifier
//////////////////////////////////////////////////////////////////////////////

/**
* Constructor for DeleteMaterialFolderModifier
*/
DeleteMaterialFolderModifier::DeleteMaterialFolderModifier(MaterialDocManager* manager, const char* materialName, MaterialTreeView* view, HTREEITEM parent, idStrList* affectedMaterials)
: MaterialModifier(manager, materialName) {
	this->view = view;
	this->parent = parent;
	this->affectedMaterials = *affectedMaterials;
}

/**
* Performs an undo operation of a deleted material folder.
*/
void DeleteMaterialFolderModifier::Undo() {
	
	//Add the folder back and save the folder position for the redo
	item = view->AddFolder(materialName, parent);

	//Add each of the children back
	for(int i = 0; i < affectedMaterials.Num(); i++) {
		manager->RedoAddMaterial(affectedMaterials[i], false);
	}
}

/**
* Performs a redo operation of a deleted material folder.
*/
void DeleteMaterialFolderModifier::Redo() {

	view->DeleteFolder(item, false);
}


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

#include "MEOptions.h"

/**
* Constructor for MEOptions.
*/
MEOptions::MEOptions ( ) {
	
	registry.Init("Software\\id Software\\DOOM3\\Tools\\MaterialEditor");

	materialTreeWidth = 0;
	stageWidth = 0;
	previewPropertiesWidth = 0;
	materialEditHeight = 0;
	materialPropHeadingWidth = 0;
	previewPropHeadingWidth = 0;

}

/**
* Destructor for MEOptions.
*/
MEOptions::~MEOptions() {
}

/**
* Saves the material editor options to the registry.
*/
bool MEOptions::Save (void) {

	registry.SetFloat("materialTreeWidth", materialTreeWidth);
	registry.SetFloat("stageWidth", stageWidth);
	registry.SetFloat("previewPropertiesWidth", previewPropertiesWidth);
	registry.SetFloat("materialEditHeight", materialEditHeight);
	registry.SetFloat("materialPropHeadingWidth", materialPropHeadingWidth);
	registry.SetFloat("previewPropHeadingWidth", previewPropHeadingWidth);

	return registry.Save();
}

/**
* Loads the material editor options from the registry.
*/
bool MEOptions::Load (void) {
	
	if(!registry.Load()) {
		return false;
	}
	
	materialTreeWidth = (int)registry.GetFloat("materialTreeWidth");
	stageWidth = (int)registry.GetFloat("stageWidth");
	previewPropertiesWidth = (int)registry.GetFloat("previewPropertiesWidth");
	materialEditHeight = (int)registry.GetFloat("materialEditHeight");
	materialPropHeadingWidth = (int)registry.GetFloat("materialPropHeadingWidth");
	previewPropHeadingWidth = (int)registry.GetFloat("previewPropHeadingWidth");

	return true;
	
}
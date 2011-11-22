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

#include "../common/registryoptions.h"

/**
* Wrapper class that is responsible for reading and writing Material Editor
* settings to the registry. Settings are written to 
* Software\\id Software\\DOOM3\\Tools\\MaterialEditor
*/
class MEOptions {

public:
	MEOptions();
	~MEOptions();

	bool				Save (void);
	bool				Load (void);

	/**
	* Sets the flag that determines if the settings need to be saved because
	* they where modified.
	*/
	void				SetModified(bool mod = true) { modified = mod; };
	/**
	* Get the flag that determines if the settings need to be saved because
	* they where modified.
	*/
	bool				GetModified() { return modified; };

	void				SetWindowPlacement		( const char* name, HWND hwnd );
	bool				GetWindowPlacement		( const char* name, HWND hwnd );

	void				SetMaterialTreeWidth(int width);
	int					GetMaterialTreeWidth();

	void				SetStageWidth(int width);
	int					GetStageWidth();

	void				SetPreviewPropertiesWidth(int width);
	int					GetPreviewPropertiesWidth();

	void				SetMaterialEditHeight(int height);
	int					GetMaterialEditHeight();

	void				SetMaterialPropHeadingWidth(int width);
	int					GetMaterialPropHeadingWidth();

	void				SetPreviewPropHeadingWidth(int width);
	int					GetPreviewPropHeadingWidth();

protected:
	rvRegistryOptions	registry;

	bool				modified;

	int					materialTreeWidth;
	int					stageWidth;
	int					previewPropertiesWidth;
	int					materialEditHeight;
	int					materialPropHeadingWidth;
	int					previewPropHeadingWidth;
};


ID_INLINE void MEOptions::SetWindowPlacement ( const char* name, HWND hwnd ) {
	registry.SetWindowPlacement ( name, hwnd );
}

ID_INLINE bool MEOptions::GetWindowPlacement ( const char* name, HWND hwnd ) {
	return registry.GetWindowPlacement ( name, hwnd );
}

ID_INLINE void MEOptions::SetMaterialTreeWidth(int width) {
	materialTreeWidth = width;
	SetModified(true);
}

ID_INLINE int MEOptions::GetMaterialTreeWidth() {
	return materialTreeWidth;
}

ID_INLINE void MEOptions::SetStageWidth(int width) {
	stageWidth = width;
	SetModified(true);
}

ID_INLINE int MEOptions::GetStageWidth() {
	return stageWidth;
}

ID_INLINE void MEOptions::SetPreviewPropertiesWidth(int width) {
	previewPropertiesWidth = width;
	SetModified(true);
}

ID_INLINE int MEOptions::GetPreviewPropertiesWidth() {
	return previewPropertiesWidth;
}

ID_INLINE void MEOptions::SetMaterialEditHeight(int height) {
	materialEditHeight = height;
	SetModified(true);
}

ID_INLINE int MEOptions::GetMaterialEditHeight() {
	return materialEditHeight;
}

ID_INLINE void MEOptions::SetMaterialPropHeadingWidth(int width) {
	materialPropHeadingWidth = width;
	SetModified(true);
}

ID_INLINE int MEOptions::GetMaterialPropHeadingWidth() {
	return materialPropHeadingWidth;
}

ID_INLINE void MEOptions::SetPreviewPropHeadingWidth(int width) {
	previewPropHeadingWidth = width;
	SetModified(true);
}

ID_INLINE int MEOptions::GetPreviewPropHeadingWidth() {
	return previewPropHeadingWidth;
}
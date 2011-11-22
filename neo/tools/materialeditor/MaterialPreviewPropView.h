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

#include "../common/PropTree/PropTreeView.h"
#include "MaterialPreviewView.h"

// MaterialPreviewPropView view

class MaterialPreviewPropView : public CPropTreeView
{
	DECLARE_DYNCREATE(MaterialPreviewPropView)

protected:
	MaterialPreviewPropView();           // protected constructor used by dynamic creation
	virtual ~MaterialPreviewPropView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view

	afx_msg void OnPropertyChangeNotification( NMHDR *nmhdr, LRESULT *lresult );
	afx_msg void OnPropertyButtonClick( NMHDR *nmhdr, LRESULT *lresult );

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void AddLight( void );
	void InitializePropTree( void );

	void RegisterPreviewView( MaterialPreviewView *view );

protected:

	int		numLights;

	MaterialPreviewView	*materialPreview;

	DECLARE_MESSAGE_MAP()
};



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

#include <afxcview.h>
#include <afxole.h>

#include "MaterialEditor.h"
#include "ToggleListView.h"
#include "MaterialView.h"
#include "MaterialPropTreeView.h"

/**
* View that handles managing the material stages.
*/
class StageView : public ToggleListView, public MaterialView
{

public:
	virtual ~StageView();

	/** 
	* Defines the type of stages
	*/
	enum {
		STAGE_TYPE_MATERIAL,
		STAGE_TYPE_STAGE,
		STAGE_TYPE_SPECIAL_MAP_STAGE
	};

	//Associates a property view with this stage view
	void					SetMaterialPropertyView(MaterialPropTreeView* propView) { m_propView = propView; };

	//MaterialView Interface
	virtual void			MV_OnMaterialSelectionChange(MaterialDoc* pMaterial);
	virtual void			MV_OnMaterialStageAdd(MaterialDoc* pMaterial, int stageNum);
	virtual void			MV_OnMaterialStageDelete(MaterialDoc* pMaterial, int stageNum);
	virtual void			MV_OnMaterialStageMove(MaterialDoc* pMaterial, int from, int to);
	virtual void			MV_OnMaterialAttributeChanged(MaterialDoc* pMaterial, int stage, const char* attribName);
	virtual void			MV_OnMaterialSaved(MaterialDoc* pMaterial);

	//Edit Operation Tests
	bool					CanCopy();
	bool					CanPaste();
	bool					CanCut();
	bool					CanDelete();
	bool					CanRename();

	//Refresh the stage list
	void					RefreshStageList();

protected:
	StageView();
	DECLARE_DYNCREATE(StageView)

	afx_msg int				OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void 			OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void 			OnLvnDeleteallitems(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void 			OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void 			OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void 			OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void 			OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void 			OnRenameStage();
	afx_msg void 			OnDeleteStage();
	afx_msg void 			OnDeleteAllStages();
	afx_msg void 			OnAddStage();
	afx_msg void 			OnAddBumpmapStage();
	afx_msg void 			OnAddDiffuseStage();
	afx_msg void 			OnAddSpecualarStage();

	afx_msg void 			OnCopy();
	afx_msg void 			OnPaste();
	
	afx_msg void 			OnLvnBeginlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void 			OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void 			OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
	
	//Overrides
	virtual BOOL			PreTranslateMessage(MSG* pMsg);
	virtual BOOL			PreCreateWindow(CREATESTRUCT& cs);

	//Toggle List View Interface
	virtual void			OnStateChanged(int index, int toggleState);

	void					PopupMenu(CPoint* pt);

	void					DropItemOnList();

protected:

	MaterialPropTreeView*	m_propView;
	MaterialDoc*			currentMaterial;
	
	//Manual handing of the row dragging
	CImageList*				dragImage;
	bool					bDragging;
	int						dragIndex;
	int						dropIndex;
	CWnd*					dropWnd;
	CPoint					dropPoint;

	bool					internalChange;
};



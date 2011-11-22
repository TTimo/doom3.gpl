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
#include "MaterialPropTreeView.h"
#include "StageView.h"

#include "../comafx/CSyntaxRichEditCtrl.h"

/**
* View that contains the material edit controls. These controls include
* the stage view, the properties view and the source view.
*/
class MaterialEditView : public CFormView, public MaterialView, SourceModifyOwner {

public:
	enum{ IDD = IDD_MATERIALEDIT_FORM };

	CEdit						m_nameEdit;
	CSplitterWnd				m_editSplitter;

	StageView*					m_stageView;
	MaterialPropTreeView*		m_materialPropertyView;
	CTabCtrl					m_tabs;
	CSyntaxRichEditCtrl			m_textView;
	
public:
	virtual			~MaterialEditView();
	
	//MaterialView Interface
	virtual void	MV_OnMaterialSelectionChange(MaterialDoc* pMaterial);
	virtual void	MV_OnMaterialNameChanged(MaterialDoc* pMaterial, const char* oldName);

	//SourceModifyOwner Interface
	virtual idStr GetSourceText();
	
protected:
	MaterialEditView();
	DECLARE_DYNCREATE(MaterialEditView)

	void			GetMaterialSource();
	void			ApplyMaterialSource();

	//CFormView Overrides
	virtual void	DoDataExchange(CDataExchange* pDX);
	virtual void	OnInitialUpdate();
	
	//Message Handlers
	afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void 	OnTcnSelChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnEnChangeEdit( NMHDR *pNMHDR, LRESULT *pResult );
	DECLARE_MESSAGE_MAP()	

protected:
	bool initHack;
	bool sourceInit;

	bool	sourceChanged;
	idStr	currentMaterialName;
};

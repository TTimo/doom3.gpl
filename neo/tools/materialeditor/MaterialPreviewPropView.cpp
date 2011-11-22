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


#include "MaterialPreviewPropView.h"


// MaterialPropTreeView

IMPLEMENT_DYNCREATE(MaterialPreviewPropView, CPropTreeView)


MaterialPreviewPropView::MaterialPreviewPropView() {
	numLights = 0;
	materialPreview = NULL;
}

MaterialPreviewPropView::~MaterialPreviewPropView() {
}

BEGIN_MESSAGE_MAP(MaterialPreviewPropView, CPropTreeView)
	ON_NOTIFY( PTN_ITEMCHANGED, IDC_PROPERTYTREE, OnPropertyChangeNotification )
	ON_NOTIFY( PTN_ITEMBUTTONCLICK, IDC_PROPERTYTREE, OnPropertyButtonClick )
END_MESSAGE_MAP()


void MaterialPreviewPropView::AddLight( void ) {
	int i, count, lightShaderIndex = 0;
	const idMaterial *mat;

	CPropTreeItemButton*	pRoot;
	CPropTreeItemCombo*		pCombo;
	CPropTreeItemColor*		pColor;
	CPropTreeItemCheck*		pCheck;
	CPropTreeItemEdit*		pEdit;

	//Increase the number of lights
	numLights++;

	pRoot = (CPropTreeItemButton*)m_Tree.InsertItem(new CPropTreeItemButton());
	pRoot->SetLabelText(_T(va("Light #%d", numLights)));
	pRoot->SetInfoText(_T(va("Parameters for light number %d.", numLights)));
	pRoot->SetButtonText( "Remove" );
	pRoot->SetCtrlID( numLights - 1 );
	pRoot->Expand();

	pCombo = (CPropTreeItemCombo*)m_Tree.InsertItem(new CPropTreeItemCombo(), pRoot);
	pCombo->SetLabelText( _T("Shader") );
	pCombo->SetInfoText( _T("Set the light shader.") );
	pCombo->SetDropDownHeight( 200 );
	pCombo->CreateComboBox();
	// Add all light shaders to the combo box
	count = declManager->GetNumDecls( DECL_MATERIAL );
	for (i = 0; i < count; i++) {
		mat = declManager->MaterialByIndex(i, false);

		idStr materialName = mat->GetName();
		materialName.ToLower();

		if ( materialName.Left(7) == "lights/" || materialName.Left(5) == "fogs/" ) {
			pCombo->InsertString( lightShaderIndex, materialName );
			pCombo->SetItemData( lightShaderIndex, lightShaderIndex );

			if ( materialName == "lights/defaultpointlight" ) {
				pCombo->SetCurSel( lightShaderIndex );
			}

			lightShaderIndex++;
		}
	}

	pColor = (CPropTreeItemColor*)m_Tree.InsertItem(new CPropTreeItemColor(), pRoot);
	pColor->SetLabelText(_T("Color"));
	pColor->SetInfoText(_T("Color of the light."));
	pColor->SetItemValue((LPARAM)RGB(0xff, 0xff, 0xff)); // default as color white

	pEdit = (CPropTreeItemEdit*)m_Tree.InsertItem( new CPropTreeItemEdit(), pRoot);
	pEdit->SetLabelText(_T("Radius"));
	pEdit->SetInfoText(_T("Radius of the light."));
	pEdit->SetItemValue( (LPARAM)_T("300.0") );

	pCheck = (CPropTreeItemCheck*)m_Tree.InsertItem(new CPropTreeItemCheck(), pRoot);
	pCheck->SetLabelText(_T("Move light"));
	pCheck->SetInfoText(_T("When checked, allow light to move."));
	pCheck->CreateCheckBox();
	pCheck->SetCheckState( BST_CHECKED );

	if ( materialPreview ) {
		materialPreview->OnAddLight();
	}
}

//Create sample data for the preview properties
void MaterialPreviewPropView::InitializePropTree( void ) {
	int i;
	CPropTreeItem		*pRoot;
	CPropTreeItem		*pParmRoot;
	CPropTreeItemCheck	*pCheck;
	CPropTreeItemEdit	*pEdit;

	pRoot = m_Tree.InsertItem(new CPropTreeItem());
	pRoot->SetLabelText(_T("Preview Properties"));
	pRoot->SetInfoText(_T("Properties for the preview window."));
	pRoot->Expand(); // have this item expanded by default

	CPropTreeItemCombo* pCombo;
	pCombo = (CPropTreeItemCombo*)m_Tree.InsertItem(new CPropTreeItemCombo(), pRoot);
	pCombo->SetLabelText(_T("Model Type"));
	pCombo->SetInfoText(_T("Select the type of model on which to preview the material."));
	pCombo->CreateComboBox();
	pCombo->InsertString( 0, "Cube" );
	pCombo->InsertString( 1, "Box - 2:1");
	pCombo->InsertString( 2, "Box - 4:1");
	pCombo->InsertString( 3, "Box - 1:2");
	pCombo->InsertString( 4, "Box - 1:4");
	pCombo->InsertString( 5, "Cylinder - V");
	pCombo->InsertString( 6, "Cylinder - H");
	pCombo->InsertString( 7, "Sphere");
	pCombo->SetItemData( 0, 0 );
	pCombo->SetItemData( 1, 1 );
	pCombo->SetItemData( 2, 2 );
	pCombo->SetItemData( 3, 3 );
	pCombo->SetItemData( 4, 4 );
	pCombo->SetItemData( 5, 5 );
	pCombo->SetItemData( 6, 6 );
	pCombo->SetItemData( 7, 7 );
	
	pCombo->SetCurSel( 0 );

	// Custom model entry
	/*pEdit = (CPropTreeItemEdit*)m_Tree.InsertItem( new CPropTreeItemEdit(), pRoot );
	pEdit->SetLabelText(_T("Custom Model"));
	pEdit->SetInfoText(_T("Specify any model to display the current material."));
	pEdit->SetItemValue((LPARAM)_T(""));*/
	CPropTreeItemEditButton *pCutomButton;
	pCutomButton = (CPropTreeItemEditButton*)m_Tree.InsertItem(new CPropTreeItemEditButton(), pRoot );
	pCutomButton->SetButtonText(_T("..."));
	pCutomButton->SetLabelText(_T("Custom Model"));
	pCutomButton->SetInfoText(_T("Specify any model to display the current material."));
	pCutomButton->SetItemValue((LPARAM)_T(""));

	// Checkbox for showing debug light spheres
	pCheck = (CPropTreeItemCheck*)m_Tree.InsertItem( new CPropTreeItemCheck(), pRoot );
	pCheck->SetLabelText(_T("Show Lights"));
	pCheck->SetInfoText(_T("Show the light origin sphere and number in the preview."));
	pCheck->CreateCheckBox();
	pCheck->SetCheckState( BST_CHECKED );

	// Local and Global shader parms
	pParmRoot = m_Tree.InsertItem(new CPropTreeItem());
	pParmRoot->SetLabelText(_T("Local Parms"));
	pParmRoot->SetInfoText(_T("Local shaderparms for the model being displayed."));
	pParmRoot->Expand( FALSE ); // have this item NOT expanded by default

	for( i = 0; i < MAX_ENTITY_SHADER_PARMS; i++ ) {
		pEdit = (CPropTreeItemEdit*)m_Tree.InsertItem( new CPropTreeItemEdit(), pParmRoot );
		pEdit->SetLabelText(_T(va("parm%d", i)));
		pEdit->SetInfoText(_T("Set the local shaderparm for the model"));
		if ( i < 4 ) {
			pEdit->SetItemValue((LPARAM)_T("1"));
		} else {
			pEdit->SetItemValue((LPARAM)_T("0"));
		}
	}

	pParmRoot = m_Tree.InsertItem(new CPropTreeItem());
	pParmRoot->SetLabelText(_T("Global Parms"));
	pParmRoot->SetInfoText(_T("Global shaderparms for the renderworld being displayed."));
	pParmRoot->Expand( FALSE ); // have this item NOT expanded by default

	for( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ ) {
		pEdit = (CPropTreeItemEdit*)m_Tree.InsertItem( new CPropTreeItemEdit(), pParmRoot );
		pEdit->SetLabelText(_T(va("global%d", i)));
		pEdit->SetInfoText(_T("Set the global shaderparm for the renderworld"));
		if ( i < 4 ) {
			pEdit->SetItemValue((LPARAM)_T("1"));
		} else {
			pEdit->SetItemValue((LPARAM)_T("0"));
		}
	}

	// Lights

	pRoot = m_Tree.InsertItem(new CPropTreeItem());
	pRoot->SetLabelText(_T(""));
	pRoot->SetInfoText(_T(""));

	CPropTreeItemButton *pButton;
	pButton = (CPropTreeItemButton*)m_Tree.InsertItem(new CPropTreeItemButton());
	pButton->SetButtonText(_T(" Add Light "));
	pButton->SetLabelText(_T("Preview Lights"));
	pButton->SetInfoText(_T("Test the button."));

	pRoot = m_Tree.InsertItem(new CPropTreeItem());
	pRoot->SetLabelText(_T(""));
	pRoot->SetInfoText(_T(""));

	AddLight();
}

// MaterialPreviewPropView drawing

void MaterialPreviewPropView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
}

// MaterialPreviewPropView diagnostics

#ifdef _DEBUG
void MaterialPreviewPropView::AssertValid() const
{
	CPropTreeView::AssertValid();
}

void MaterialPreviewPropView::Dump(CDumpContext& dc) const
{
	CPropTreeView::Dump(dc);
}
#endif //_DEBUG


void MaterialPreviewPropView::RegisterPreviewView( MaterialPreviewView *view ) {
	materialPreview = view;
}

// MaterialPreviewPropView message handlers

void MaterialPreviewPropView::OnPropertyChangeNotification( NMHDR *nmhdr, LRESULT *lresult ) {
	idVec3			testColor;
	int				lightId = 0;
	COLORREF		color;
	NMPROPTREE		*nmProp;
	CPropTreeItem	*item;
	CPropTreeItem	*parent;

	nmProp = (NMPROPTREE *)nmhdr;
	item = nmProp->pItem;

	// Determine which light this item modifies
	parent = item->GetParent();
	if ( parent ) {
		lightId = parent->GetCtrlID();
	}

	idStr	itemLabel = item->GetLabelText();
	
	if ( itemLabel == "Model Type" ) {
		materialPreview->OnModelChange( item->GetItemValue() );

	} else if ( itemLabel == "Custom Model" ) {
		materialPreview->OnCustomModelChange( (const char *)item->GetItemValue() );

	} else if ( itemLabel == "Show Lights" ) {
		materialPreview->OnShowLightsChange( item->GetItemValue() ? true : false );

	} else if ( itemLabel == "Shader" ) {
		CPropTreeItemCombo	*combo = (CPropTreeItemCombo *)item;
		CString materialName;

		combo->GetLBText( combo->GetCurSel(), materialName );

		materialPreview->OnLightShaderChange( lightId, materialName.GetBuffer() );

	} else if ( itemLabel == "Radius" ) {
		materialPreview->OnLightRadiusChange( lightId, atof( (char *)item->GetItemValue() ) );

	} else if ( itemLabel == "Color" ) {
		color = item->GetItemValue();

		testColor.x = (float)GetRValue( color ) * (float)( 1.f/255.f );
		testColor.y = (float)GetGValue( color ) * (float)( 1.f/255.f );
		testColor.z = (float)GetBValue( color ) * (float)( 1.f/255.f );

		materialPreview->OnLightColorChange( lightId, testColor );

	} else if ( itemLabel == "Move light" ) {
		materialPreview->OnLightAllowMoveChange( lightId, item->GetItemValue() ? true : false );

	} else if ( itemLabel.Left(4) == "parm" ) {
		int index;

		itemLabel.Strip( "parm" );
		index = atoi( itemLabel.c_str() );

		materialPreview->OnLocalParmChange( index, atof( (char *)item->GetItemValue() ) );

	} else if ( itemLabel.Left(6) == "global" ) {
		int index;

		itemLabel.Strip( "global" );
		index = atoi( itemLabel.c_str() );

		materialPreview->OnGlobalParmChange( index, atof( (char *)item->GetItemValue() ) );
	}
}

void MaterialPreviewPropView::OnPropertyButtonClick( NMHDR *nmhdr, LRESULT *lresult ) {
	NMPROPTREE		*nmProp;
	CPropTreeItem	*item;

	nmProp = (NMPROPTREE *)nmhdr;
	item = nmProp->pItem;

	idStr	itemLabel = item->GetLabelText();

	if ( itemLabel == "Preview Lights" ) {
		AddLight();

	} else if ( itemLabel.Left(5) == "Light" ) {
		CPropTreeItem	*light;
		int lightId = item->GetCtrlID();
		int testLightNum = 0;

		m_Tree.DeleteItem( item );

		for( light = m_Tree.GetRootItem()->GetChild(); light != NULL; light = light->GetSibling() ) {
			idStr label = light->GetLabelText();

			if ( label.Left(5) == "Light" ) {
				testLightNum++;
				light->SetLabelText(_T(va("Light #%d", testLightNum)));
				light->SetInfoText(_T(va("Parameters for light number %d.", testLightNum)));
				light->SetCtrlID( testLightNum - 1 );
			}
		}

		materialPreview->OnDeleteLight( lightId );

		numLights--;
	} else if ( itemLabel == "Custom Model" ) {
		CFileDialog dlg(TRUE);
		dlg.m_ofn.Flags |= OFN_FILEMUSTEXIST;
		item->Check(FALSE);
		if( dlg.DoModal()== IDOK) {
			item->Check(FALSE);
			item->SetItemValue((LPARAM)fileSystem->OSPathToRelativePath(dlg.m_ofn.lpstrFile));
			m_Tree.SendNotify(PTN_ITEMCHANGED, item);
		}
	}
}

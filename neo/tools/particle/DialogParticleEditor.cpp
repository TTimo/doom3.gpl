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

#include "../../game/game.h"
#include "../../sys/win32/win_local.h"
#include "../../sys/win32/rc/common_resource.h"
#include "../../sys/win32/rc/Radiant_resource.h"
#include "../../sys/win32/rc/ParticleEditor_resource.h"
#include "../comafx/DialogName.h"
#include "../comafx/VectorCtl.h"
#include "../comafx/DialogColorPicker.h"
#include "../radiant/GLWidget.h"
#include "../radiant/PreviewDlg.h"

#include "DialogParticleEditor.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

const int StageEnableID[] = {
	IDC_EDIT_MATERIAL,
	IDC_BUTTON_BROWSEMATERIAL,
	IDC_EDIT_ANIMFRAMES,
	IDC_EDIT_ANIMRATE,
	IDC_EDIT_COLOR,
	IDC_BUTTON_BROWSECOLOR,
	IDC_EDIT_FADECOLOR,
	IDC_BUTTON_BROWSEFADECOLOR,
	IDC_EDIT_FADEIN,
	IDC_SLIDER_FADEIN,
	IDC_EDIT_FADEOUT,
	IDC_EDIT_FADEFRACTION,
	IDC_SLIDER_FADEOUT,
	IDC_SLIDER_FADEFRACTION,
	IDC_EDIT_BUNCHING,
	IDC_SLIDER_BUNCHING,
	IDC_EDIT_COUNT,
	IDC_SLIDER_COUNT,
	IDC_EDIT_TIME,
	IDC_SLIDER_TIME,
	IDC_EDIT_CYCLES,
	IDC_EDIT_TIMEOFFSET,
	IDC_EDIT_DEADTIME,
	IDC_CHECK_WORLDGRAVITY,
	IDC_EDIT_GRAVITY,
	IDC_SLIDER_GRAVITY,
	IDC_RADIO_RECT,
	IDC_RADIO_CYLINDER,
	IDC_RADIO_SPHERE,
	IDC_EDIT_OFFSET,
	IDC_EDIT_XSIZE,
	IDC_EDIT_YSIZE,
	IDC_EDIT_ZSIZE,
	IDC_EDIT_RINGOFFSET,
	IDC_RADIO_CONE,
	IDC_RADIO_OUTWARD,
	IDC_EDIT_DIRECTIONPARM,
	IDC_RADIO_AIMED,
	IDC_RADIO_VIEW,
	IDC_RADIO_X,
	IDC_RADIO_Y,
	IDC_RADIO_Z,
	IDC_EDIT_ORIENTATIONPARM1,
	IDC_EDIT_ORIENTATIONPARM2,
	IDC_SLIDER_SPEEDFROM,
	IDC_EDIT_SPEEDFROM,
	IDC_EDIT_SPEEDTO,
	IDC_SLIDER_SPEEDTO,
	IDC_SLIDER_ROTATIONFROM,
	IDC_EDIT_ROTATIONFROM,
	IDC_EDIT_ROTATIONTO,
	IDC_SLIDER_ROTATIONTO,
	IDC_SLIDER_SIZEFROM,
	IDC_EDIT_SIZEFROM,
	IDC_EDIT_SIZETO,
	IDC_SLIDER_SIZETO,
	IDC_SLIDER_ASPECTFROM,
	IDC_EDIT_ASPECTFROM,
	IDC_EDIT_ASPECTTO,
	IDC_SLIDER_ASPECTTO,
	IDC_COMBO_CUSTOMPATH,
	IDC_CHECK_ENTITYCOLOR,
};

const int StageIDCount = sizeof( StageEnableID ) / sizeof ( const int );

const int EditEnableID[] = {
	IDC_BUTTON_XDN,
	IDC_BUTTON_XUP,
	IDC_BUTTON_YUP,
	IDC_BUTTON_YDN,
	IDC_BUTTON_ZUP,
	IDC_BUTTON_ZDN,
	IDC_BUTTON_DROPEMITTER,
	IDC_BUTTON_VECTOR,
	IDC_BUTTON_BROWSECOLOR_ENTITY
};

const int EditIDCount = sizeof ( EditEnableID ) / sizeof ( const int );


CDialogParticleEditor *g_ParticleDialog = NULL;


/*
================
ParticleEditorInit
================
*/
void ParticleEditorInit( const idDict *spawnArgs ) {

	if ( renderSystem->IsFullScreen() ) {
		common->Printf( "Cannot run the particle editor in fullscreen mode.\n"
			"Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

	if ( g_ParticleDialog == NULL ) {
		InitAfx();
		g_ParticleDialog = new CDialogParticleEditor();
	}

	if ( g_ParticleDialog->GetSafeHwnd() == NULL) {
		g_ParticleDialog->Create( IDD_DIALOG_PARTICLE_EDITOR );
		/*
		// FIXME: restore position
		CRect rct;
		g_AFDialog->SetWindowPos( NULL, rct.left, rct.top, 0, 0, SWP_NOSIZE );
		*/
	}

	idKeyInput::ClearStates();

	g_ParticleDialog->ShowWindow( SW_SHOW );
	g_ParticleDialog->SetFocus();

	if ( spawnArgs ) {
		idStr str = spawnArgs->GetString( "model" );
		str.StripFileExtension();
		g_ParticleDialog->SelectParticle( str );
		g_ParticleDialog->SetParticleVisualization( static_cast<int>( CDialogParticleEditor::SELECTED ) );
	}

	cvarSystem->SetCVarBool( "r_useCachedDynamicModels", false );
}


/*
================
ParticleEditorRun
================
*/
void ParticleEditorRun( void ) {
#if _MSC_VER >= 1300
	MSG *msg = AfxGetCurrentMessage();			// TODO Robert fix me!!
#else
	MSG *msg = &m_msgCur;
#endif

	while( ::PeekMessage(msg, NULL, NULL, NULL, PM_NOREMOVE) ) {
		// pump message
		if ( !AfxGetApp()->PumpMessage() ) {
		}
	}
}

/*
================
ParticleEditorShutdown
================
*/
void ParticleEditorShutdown( void ) {
	delete g_ParticleDialog;
	g_ParticleDialog = NULL;
}


// CDialogParticleEditor dialog

IMPLEMENT_DYNAMIC(CDialogParticleEditor, CDialog)
CDialogParticleEditor::CDialogParticleEditor(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogParticleEditor::IDD, pParent)
	, matName(_T(""))
	, animFrames(_T(""))
	, animRate(_T(""))
	, color(_T(""))
	, fadeColor(_T(""))
	, fadeIn(_T(""))
	, fadeOut(_T(""))
	, fadeFraction(_T(""))
	, count(_T(""))
	, time(_T(""))
	, timeOffset(_T(""))
	, deadTime(_T(""))
	, gravity(_T(""))
	, bunching(_T(""))
	, offset(_T(""))
	, xSize(_T(""))
	, ySize(_T(""))
	, zSize(_T(""))
	, ringOffset(_T(""))
	, directionParm(_T(""))
	, direction(0)
	, orientation(0)
	, distribution(0)
	, viewOrigin(_T(""))
	, speedFrom(_T(""))
	, speedTo(_T(""))
	, rotationFrom(_T(""))
	, rotationTo(_T(""))
	, sizeFrom(_T(""))
	, sizeTo(_T(""))
	, aspectFrom(_T(""))
	, aspectTo(_T(""))
	, customPath(_T(""))
	, customParms(_T(""))
	, trails(_T(""))
	, trailTime(_T(""))
	, worldGravity(TRUE)
	, entityColor(TRUE)
	, randomDistribution(TRUE)
	, initialAngle(_T(""))
	, boundsExpansion(_T(""))
	, customDesc(_T(""))
	, particleMode(FALSE)
{
	visualization = TESTMODEL;
	mapModified = false;
}

CDialogParticleEditor::~CDialogParticleEditor() {
}

void CDialogParticleEditor::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DEPTHHACK, depthHack);
	DDX_Check(pDX, IDC_CHECK_WORLDGRAVITY, worldGravity);
	DDX_Check(pDX, IDC_CHECK_ENTITYCOLOR, entityColor);
	DDX_Control(pDX, IDC_COMBO_PARTICLES, comboParticle);
	DDX_Control(pDX, IDC_LIST_STAGES, listStages);
	DDX_Text(pDX, IDC_COMBO_CUSTOMPATH, customPath );
	DDX_Text(pDX, IDC_EDIT_CUSTOMPARMS, customParms );
	DDX_Text(pDX, IDC_EDIT_MATERIAL, matName);
	DDX_Text(pDX, IDC_EDIT_ANIMFRAMES, animFrames);
	DDX_Text(pDX, IDC_EDIT_ANIMRATE, animRate);
	DDX_Text(pDX, IDC_EDIT_COLOR, color);
	DDX_Text(pDX, IDC_EDIT_FADECOLOR, fadeColor);
	DDX_Text(pDX, IDC_EDIT_FADEIN, fadeIn);
	DDX_Text(pDX, IDC_EDIT_FADEOUT, fadeOut);
	DDX_Text(pDX, IDC_EDIT_FADEFRACTION, fadeFraction);
	DDX_Text(pDX, IDC_EDIT_COUNT, count);
	DDX_Text(pDX, IDC_EDIT_TIME, time);
	DDX_Text(pDX, IDC_EDIT_TIMEOFFSET, timeOffset);
	DDX_Text(pDX, IDC_EDIT_DEADTIME, deadTime);
	DDX_Text(pDX, IDC_EDIT_GRAVITY, gravity);
	DDX_Text(pDX, IDC_EDIT_BUNCHING, bunching);
	DDX_Text(pDX, IDC_EDIT_OFFSET, offset);
	DDX_Text(pDX, IDC_EDIT_XSIZE, xSize);
	DDX_Text(pDX, IDC_EDIT_YSIZE, ySize);
	DDX_Text(pDX, IDC_EDIT_ZSIZE, zSize);
	DDX_Text(pDX, IDC_EDIT_RINGOFFSET, ringOffset);
	DDX_Text(pDX, IDC_EDIT_CYCLES, cycles);
	DDX_Control(pDX, IDC_STATIC_DIRPARM, staticDirectionParm);
	DDX_Text(pDX, IDC_EDIT_DIRECTIONPARM, directionParm);
	DDX_Text(pDX, IDC_EDIT_SPEEDFROM, speedFrom);
	DDX_Text(pDX, IDC_EDIT_SPEEDTO, speedTo);
	DDX_Text(pDX, IDC_EDIT_ROTATIONFROM, rotationFrom);
	DDX_Text(pDX, IDC_EDIT_ROTATIONTO, rotationTo);
	DDX_Text(pDX, IDC_EDIT_SIZEFROM, sizeFrom);
	DDX_Text(pDX, IDC_EDIT_SIZETO, sizeTo);
	DDX_Text(pDX, IDC_EDIT_ASPECTFROM, aspectFrom);
	DDX_Text(pDX, IDC_EDIT_ASPECTTO, aspectTo);
	DDX_Text(pDX, IDC_EDIT_ORIENTATIONPARM1, trails);
	DDX_Text(pDX, IDC_EDIT_ORIENTATIONPARM2, trailTime);
	DDX_Check(pDX, IDC_CHECK_RANDOMDISTRIBUTION, randomDistribution);
	DDX_Text(pDX, IDC_EDIT_INITIALANGLE, initialAngle);
	DDX_Text(pDX, IDC_EDIT_BOUNDSEXPANSION, boundsExpansion);
	DDX_Text(pDX, IDC_STATIC_DESC, customDesc);
	DDX_Control(pDX, IDC_EDIT_RINGOFFSET, editRingOffset);
	DDX_Radio(pDX, IDC_RADIO_RECT, distribution);
	DDX_Radio(pDX, IDC_RADIO_CONE, direction);
	DDX_Radio(pDX, IDC_RADIO_VIEW, orientation);
	DDX_Control(pDX, IDC_SLIDER_BUNCHING, sliderBunching);
	DDX_Control(pDX, IDC_SLIDER_FADEIN, sliderFadeIn);
	DDX_Control(pDX, IDC_SLIDER_FADEOUT, sliderFadeOut);
	DDX_Control(pDX, IDC_SLIDER_FADEFRACTION, sliderFadeFraction);
	DDX_Control(pDX, IDC_SLIDER_COUNT, sliderCount);
	DDX_Control(pDX, IDC_SLIDER_TIME, sliderTime);
	DDX_Control(pDX, IDC_SLIDER_GRAVITY, sliderGravity);
	DDX_Control(pDX, IDC_SLIDER_SPEEDFROM, sliderSpeedFrom);
	DDX_Control(pDX, IDC_SLIDER_SPEEDTO, sliderSpeedTo);
	DDX_Control(pDX, IDC_SLIDER_ROTATIONFROM, sliderRotationFrom);
	DDX_Control(pDX, IDC_SLIDER_ROTATIONTO, sliderRotationTo);
	DDX_Control(pDX, IDC_SLIDER_SIZEFROM, sliderSizeFrom);
	DDX_Control(pDX, IDC_SLIDER_SIZETO, sliderSizeTo);
	DDX_Control(pDX, IDC_SLIDER_ASPECTFROM, sliderAspectFrom);
	DDX_Control(pDX, IDC_SLIDER_ASPECTTO, sliderAspectTo);
	DDX_Control(pDX, IDC_BUTTON_VECTOR, vectorControl);
}


BEGIN_MESSAGE_MAP(CDialogParticleEditor, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ADDSTAGE, OnBnClickedButtonAddstage)
	ON_BN_CLICKED(IDC_BUTTON_REMOVESTAGE, OnBnClickedButtonRemovestage)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEMATERIAL, OnBnClickedButtonBrowsematerial)
	ON_BN_CLICKED(IDC_BUTTON_BROWSECOLOR, OnBnClickedButtonBrowsecolor)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEFADECOLOR, OnBnClickedButtonBrowsefadecolor)
	ON_BN_CLICKED(IDC_BUTTON_BROWSECOLOR_ENTITY, OnBnClickedButtonBrowseEntitycolor)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, OnBnClickedButtonUpdate)
	ON_CBN_SELCHANGE(IDC_COMBO_PARTICLES, OnCbnSelchangeComboParticles)
	ON_CBN_SELCHANGE(IDC_COMBO_CUSTOMPATH, OnCbnSelchangeComboPath)
	ON_BN_CLICKED(IDC_RADIO_RECT, OnBnClickedRadioRect)
	ON_BN_CLICKED(IDC_RADIO_SPHERE, OnBnClickedRadioSphere)
	ON_BN_CLICKED(IDC_RADIO_CYLINDER, OnBnClickedRadioCylinder)
	ON_BN_CLICKED(IDC_RADIO_CONE, OnBnClickedRadioCone)
	ON_BN_CLICKED(IDC_RADIO_OUTWARD, OnBnClickedRadioOutward)
	ON_BN_CLICKED(IDC_RADIO_VIEW, OnBnClickedRadioView)
	ON_BN_CLICKED(IDC_RADIO_AIMED, OnBnClickedRadioAimed)
	ON_BN_CLICKED(IDC_RADIO_X, OnBnClickedRadioX)
	ON_BN_CLICKED(IDC_RADIO_Y, OnBnClickedRadioY)
	ON_BN_CLICKED(IDC_RADIO_Z, OnBnClickedRadioZ)
	ON_BN_CLICKED(IDC_BUTTON_HIDESTAGE, OnBnClickedButtonHidestage)
	ON_BN_CLICKED(IDC_BUTTON_SHOWSTAGE, OnBnClickedButtonShowstage)
	ON_BN_CLICKED(IDC_CHECK_WORLDGRAVITY, OnBnClickedWorldGravity)
	ON_BN_CLICKED(IDC_CHECK_ENTITYCOLOR, OnBnClickedEntityColor)
	ON_LBN_SELCHANGE(IDC_LIST_STAGES, OnLbnSelchangeListStages)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_PARTICLE, OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_PARTICLE_AS, OnBnClickedButtonSaveAs)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_PARTICLEENTITIES, OnBnClickedButtonSaveParticles)
	ON_BN_CLICKED(IDC_BUTTON_TESTMODEL, OnBnClickedTestModel)
	ON_BN_CLICKED(IDC_BUTTON_IMPACT, OnBnClickedImpact)
	ON_BN_CLICKED(IDC_BUTTON_MUZZLE, OnBnClickedMuzzle)
	ON_BN_CLICKED(IDC_BUTTON_FLIGHT, OnBnClickedFlight)
	ON_BN_CLICKED(IDC_BUTTON_SELECTED, OnBnClickedSelected)
	ON_BN_CLICKED(IDC_BUTTON_DOOM, OnBnClickedDoom)
	ON_BN_CLICKED(IDC_CHECK_EDITPARTICLEMODE, OnBnClickedParticleMode)
	ON_BN_CLICKED(IDC_BUTTON_DROPEMITTER, OnBtnDrop)
	ON_BN_CLICKED(IDC_BUTTON_YUP, OnBtnYup)
	ON_BN_CLICKED(IDC_BUTTON_YDN, OnBtnYdn)
	ON_BN_CLICKED(IDC_BUTTON_XDN, OnBtnXdn)
	ON_BN_CLICKED(IDC_BUTTON_XUP, OnBtnXup)
	ON_BN_CLICKED(IDC_BUTTON_ZUP, OnBtnZup)
	ON_BN_CLICKED(IDC_BUTTON_ZDN, OnBtnZdn)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CDialogParticleEditor message handlers

void CDialogParticleEditor::OnBnClickedParticleMode() {
	particleMode = !particleMode;
	cvarSystem->SetCVarInteger( "g_editEntityMode", ( particleMode ) ? 4 : 0 );
	EnableEditControls();
}


void CDialogParticleEditor::OnBnClickedButtonSaveAs() {
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}
	DialogName dlg("New Particle");
	if (dlg.DoModal() == IDOK) {
		if ( declManager->FindType( DECL_PARTICLE, dlg.m_strName, false ) ) {
			MessageBox( "Particle already exists!", "Particle exists", MB_OK );
			return;
		}
		CFileDialog dlgSave( TRUE, "prt", NULL, OFN_CREATEPROMPT, "Particle Files (*.prt)|*.prt||All Files (*.*)|*.*||", AfxGetMainWnd() );
		if ( dlgSave.DoModal() == IDOK ) {
			idStr fileName;
			fileName = fileSystem->OSPathToRelativePath( dlgSave.m_ofn.lpstrFile );
			idDeclParticle *decl = dynamic_cast<idDeclParticle*>( declManager->CreateNewDecl( DECL_PARTICLE, dlg.m_strName, fileName ) );
			if ( decl ) {
				decl->stages.DeleteContents( true );
				decl->depthHack = idp->depthHack;
				for ( int i = 0; i < idp->stages.Num(); i++ ) {
					idParticleStage *stage = new idParticleStage();
					*stage = *idp->stages[i];
					decl->stages.Append( stage );
				}
				EnumParticles();
				int index = comboParticle.FindStringExact( -1, dlg.m_strName );
				if ( index >= 0 ) {
					comboParticle.SetCurSel( index );
				}
				OnBnClickedButtonSave();
				OnCbnSelchangeComboParticles();
				OnBnClickedButtonUpdate();
			}
		}
	}
}


void CDialogParticleEditor::OnBnClickedButtonSaveParticles() {
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, "saveParticles" );
	CWnd *wnd = GetDlgItem( IDC_BUTTON_SAVE_PARTICLEENTITIES );
	if ( wnd ) {
		wnd->EnableWindow( FALSE );
	}
}

void CDialogParticleEditor::OnBnClickedButtonAddstage() {
	AddStage();
}

void CDialogParticleEditor::OnBnClickedButtonRemovestage() {
	RemoveStage();
}

void CDialogParticleEditor::OnBnClickedButtonBrowsematerial() {
	CPreviewDlg matDlg( this );
	matDlg.SetMode(CPreviewDlg::MATERIALS, "particles" );
	matDlg.SetDisablePreview( true );
	if ( matDlg.DoModal() == IDOK ) {
		matName = matDlg.mediaName;
		DlgVarsToCurStage();
		CurStageToDlgVars();
	}
}

void CDialogParticleEditor::OnBnClickedButtonBrowsecolor() {
	int r, g, b;
	float ob;
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	r = ps->color.x * 255.0f;
	g = ps->color.y * 255.0f;
	b = ps->color.z * 255.0f;
	ob = 1.0f;
	if ( DoNewColor( &r, &g, &b, &ob ) ) {
		color.Format( "%f %f %f %f", (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	}
}

void CDialogParticleEditor::OnBnClickedButtonBrowseEntitycolor() {
	int r, g, b;
	float ob;
	idList<idEntity*> list;
	idDict dict2;
	idStr str;
	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		const idDict *dict = gameEdit->EntityGetSpawnArgs( list[0] );
		if ( dict ) {
			idVec3 clr = dict->GetVector( "_color", "1 1 1" );
			r = clr.x * 255.0f;
			g = clr.y * 255.0f;
			b = clr.z * 255.0f;
			ob = 1.0f;
			if ( DoNewColor( &r, &g, &b, &ob ) ) {
				for ( int i = 0; i < count; i++ ) {
					dict = gameEdit->EntityGetSpawnArgs( list[i] );
					const char *name = dict->GetString( "name" );
					idEntity *ent = gameEdit->FindEntity( name );
					if ( ent ) {
						gameEdit->EntitySetColor( ent, idVec3( (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f ) );
						str = va( "%f %f %f", (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f );
						dict2.Clear();
						dict2.Set( "_color", str );
						gameEdit->EntityChangeSpawnArgs( ent, &dict2 );
						gameEdit->MapSetEntityKeyVal( name, "_color",  str );
					}
				}
			}
		}
		CWnd *wnd = GetDlgItem( IDC_BUTTON_SAVE_PARTICLEENTITIES );
		if ( wnd ) {
			wnd->EnableWindow( TRUE );
		}
	}

}

void CDialogParticleEditor::OnBnClickedButtonBrowsefadecolor() {
	int r, g, b;
	float ob;
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	r = ps->fadeColor.x * 255.0f;
	g = ps->fadeColor.y * 255.0f;
	b = ps->fadeColor.z * 255.0f;
	ob = 1.0f;
	if ( DoNewColor( &r, &g, &b, &ob ) ) {
		fadeColor.Format( "%f %f %f %f", (float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	}
}

void CDialogParticleEditor::OnBnClickedButtonUpdate() {
	UpdateData( TRUE );
	DlgVarsToCurStage();
	CurStageToDlgVars();
}

void CDialogParticleEditor::SelectParticle( const char *name ) {
	int index = comboParticle.FindString( 0, name );
	if ( index >= 0 ) {
		comboParticle.SetCurSel( index );
		UpdateParticleData();
	}
}

idDeclParticle *CDialogParticleEditor::GetCurParticle() {
	int sel = comboParticle.GetCurSel();
	if ( sel == CB_ERR ) {
		return NULL;
	}
	int index = comboParticle.GetItemData( sel );
	return static_cast<idDeclParticle *>( const_cast<idDecl *>( declManager->DeclByIndex( DECL_PARTICLE, index ) ) );
}

void CDialogParticleEditor::UpdateParticleData() {
	
	listStages.ResetContent();
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}
	for ( int i = 0; i < idp->stages.Num(); i++ ) {
		int index = listStages.AddString( va( "stage %i", i ) );
		if ( index >= 0 ) {
			listStages.SetItemData( index, i );
		}
	}
	listStages.SetCurSel( 0 );
	OnLbnSelchangeListStages();
	CWnd *wnd = GetDlgItem( IDC_STATIC_INFILE );
	if ( wnd ) {
		wnd->SetWindowText( va( "Particle file: %s", idp->GetFileName() ) );
	}

	SetParticleView();	
}

void CDialogParticleEditor::OnCbnSelchangeComboParticles() {
	UpdateParticleData();
}


void CDialogParticleEditor::OnCbnSelchangeComboPath() {
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::UpdateControlInfo() {
	CWnd *wnd = GetDlgItem( IDC_EDIT_RINGOFFSET );
	if ( wnd ) {
		wnd->EnableWindow( distribution == 2 );
	}
	wnd = GetDlgItem( IDC_STATIC_DIRPARM );
	if ( wnd ) {
		wnd->SetWindowText( (direction == 0 ) ? "Angle" : "Upward Bias" );
	}
	wnd = GetDlgItem( IDC_EDIT_ORIENTATIONPARM1 );
	if ( wnd ) {
		wnd->EnableWindow( orientation == 1 );
	}
	wnd = GetDlgItem( IDC_EDIT_ORIENTATIONPARM2 );
	if ( wnd ) {
		wnd->EnableWindow( orientation == 1 );
	}

	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	sliderBunching.SetValuePos( ps->spawnBunching );
	sliderFadeIn.SetValuePos( ps->fadeInFraction );
	sliderFadeOut.SetValuePos( ps->fadeOutFraction );
	sliderFadeFraction.SetValuePos( ps->fadeIndexFraction );
	sliderCount.SetValuePos( ps->totalParticles );
	sliderTime.SetValuePos( ps->particleLife );
	sliderGravity.SetValuePos( ps->gravity );
	sliderSpeedFrom.SetValuePos( ps->speed.from );
	sliderSpeedTo.SetValuePos( ps->speed.to );
	sliderRotationFrom.SetValuePos( ps->rotationSpeed.from );
	sliderRotationTo.SetValuePos( ps->rotationSpeed.to );
	sliderSizeFrom.SetValuePos( ps->size.from );
	sliderSizeTo.SetValuePos( ps->size.to );
	sliderAspectFrom.SetValuePos( ps->aspect.from );
	sliderAspectTo.SetValuePos( ps->aspect.to );
}

void CDialogParticleEditor::OnBnClickedRadioRect() {
	distribution = 0;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedRadioSphere() {
	distribution = 2;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedRadioCylinder() {
	distribution = 1;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedRadioCone() {
	direction = 0;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedRadioOutward() {
	direction = 1;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedRadioView() {
	orientation = 0;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedRadioAimed() {
	orientation = 1;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedRadioX() {
	orientation = 2;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedRadioY() {
	orientation = 3;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedRadioZ() {
	orientation = 4;
	DlgVarsToCurStage();
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnBnClickedDoom() {
	::SetFocus(win32.hWnd);
}


void CDialogParticleEditor::OnBnClickedTestModel() {
	visualization = TESTMODEL;
	SetParticleView();
}

void CDialogParticleEditor::OnBnClickedImpact() {
	visualization = IMPACT;
	SetParticleView();
}

void CDialogParticleEditor::OnBnClickedMuzzle(){ 
	visualization = MUZZLE;
	SetParticleView();
}

void CDialogParticleEditor::OnBnClickedFlight() {
	visualization = FLIGHT;
	SetParticleView();
}

void CDialogParticleEditor::OnBnClickedSelected() {
	visualization = SELECTED;
	SetParticleView();
}

void CDialogParticleEditor::SetParticleVisualization( int i ) { 
	visualization = i; 
	SetParticleView();
}

void CDialogParticleEditor::SetParticleView() {
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, "testmodel" );
	idStr str;
	switch ( visualization ) {
		case TESTMODEL : 
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, va("testmodel %s\n", str.c_str() ) );
		break;
		case IMPACT :
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cvarSystem->SetCVarInteger( "g_testParticle", TEST_PARTICLE_IMPACT );
			cvarSystem->SetCVarString( "g_testParticleName", str );
		break;
		case MUZZLE :
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cvarSystem->SetCVarInteger( "g_testParticle", TEST_PARTICLE_MUZZLE );
			cvarSystem->SetCVarString( "g_testParticleName", str );
		break;
		case FLIGHT :
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cvarSystem->SetCVarInteger( "g_testParticle", TEST_PARTICLE_FLIGHT );
			cvarSystem->SetCVarString( "g_testParticleName", str );
		break;
		case SELECTED :
			str = idp->GetName();
			str.SetFileExtension( ".prt" );
			cvarSystem->SetCVarInteger( "g_testParticle", TEST_PARTICLE_FLIGHT );
			SetSelectedModel( str );
		break;
	}
}

void CDialogParticleEditor::SetSelectedModel( const char *val ) {
	idList<idEntity*> list;
	idMat3 axis;

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		for ( int i = 0; i < count; i++ ) {
			const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
			if ( dict == NULL ) {
				continue;
			}
			const char *name = dict->GetString( "name" );
			gameEdit->EntitySetModel( list[i], val );
			gameEdit->EntityUpdateVisuals( list[i] );
			gameEdit->EntityGetAxis( list[i], axis );
			vectorControl.SetidAxis( axis );
			gameEdit->MapSetEntityKeyVal( name, "model", val );
		}
		CWnd *wnd = GetDlgItem( IDC_BUTTON_SAVE_PARTICLEENTITIES );
		if ( wnd ) {
			wnd->EnableWindow( TRUE );
		}
	}
}


void CDialogParticleEditor::OnBnClickedButtonHidestage() {
	HideStage();
}

void CDialogParticleEditor::OnBnClickedButtonShowstage() {
	ShowStage();
}


void CDialogParticleEditor::OnBnClickedWorldGravity() {
	worldGravity = !worldGravity;
	DlgVarsToCurStage();
	CurStageToDlgVars();
}

void CDialogParticleEditor::OnBnClickedEntityColor() {
	entityColor = !entityColor;
	DlgVarsToCurStage();
	CurStageToDlgVars();
}


void CDialogParticleEditor::AddStage() {
	
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	idParticleStage *stage = new idParticleStage;
	
	if ((GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
		idParticleStage *source = GetCurStage();
		if ( source == NULL ) {
			delete stage;
			return;
		}
		*stage = *source;
	} else {
		stage->Default();
	}
	int newIndex = idp->stages.Append( stage );
	int index = listStages.AddString( va( "stage %i", newIndex ) );
	listStages.SetCurSel( index );
	listStages.SetItemData( index, newIndex );
	ShowCurrentStage();
	EnableStageControls();
}

void CDialogParticleEditor::RemoveStage() {
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	if ( MessageBox( "Are you sure you want to remove this stage?", "Remove Stage", MB_YESNO | MB_ICONQUESTION ) != IDYES ) {
		return;
	}

	int index = listStages.GetCurSel();
	if ( index >= 0 ) {
		int newIndex = listStages.GetItemData( index );
		if ( newIndex >= 0 && newIndex < idp->stages.Num() ) {
			idp->stages.RemoveIndex( newIndex );
			index += ( index >= 1 ) ? -1 : 1;
			newIndex = comboParticle.FindStringExact( -1, idp->GetName() );
			EnumParticles();
			if ( newIndex >= 0 ) {
				comboParticle.SetCurSel( newIndex );
			}
			OnCbnSelchangeComboParticles();
			listStages.SetCurSel( index );
			ShowCurrentStage();
		}
	}
	EnableStageControls();
}

void CDialogParticleEditor::ShowStage() {
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	ps->hidden = false;
	int index = listStages.GetCurSel();
	int newIndex = listStages.GetItemData( index );
	listStages.DeleteString ( index );
	listStages.InsertString( index, va("stage %i", index ) );
	listStages.SetItemData( index, newIndex );
	listStages.SetCurSel( index );
	EnableStageControls();
}

void CDialogParticleEditor::HideStage() {
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	ps->hidden = true;
	int index = listStages.GetCurSel();
	int newIndex = listStages.GetItemData( index );
	listStages.DeleteString ( index );
	listStages.InsertString( index, va("stage %i (H) ", index ) );
	listStages.SetItemData( index, newIndex );
	listStages.SetCurSel( index );
	EnableStageControls();
}

idParticleStage *CDialogParticleEditor::GetCurStage() {
	idDeclParticle *idp = GetCurParticle();
	int sel = listStages.GetCurSel();
	int index = listStages.GetItemData( sel );
	if ( idp == NULL || sel == LB_ERR || index >= idp->stages.Num() ) {
		return NULL;
	}
	return idp->stages[index];
}

void CDialogParticleEditor::ClearDlgVars() {
	matName = "";
	animFrames = "";
	animRate = "";
	color = "";
	fadeColor = "";
	fadeIn = "";
	fadeOut = "";
	fadeFraction = "";
	count = "";
	time = "";
	timeOffset = "";
	deadTime = "";
	gravity = "";
	bunching = "";
	offset = "";
	xSize = "";
	ySize = "";
	zSize = "";
	ringOffset = "";
	directionParm = "";
	direction = 1;
	orientation = 1;
	distribution = 1;
	speedFrom = "";
	speedTo = "";
	rotationFrom = "";
	rotationTo = "";
	sizeFrom = "";
	sizeTo = "";
	aspectFrom = "";
	aspectTo = "";
	customPath = "";
	customParms = "";
	trails = "";
	trailTime = "";
	worldGravity = FALSE;
	entityColor = FALSE;
	randomDistribution = TRUE;
	initialAngle = "";
	customDesc = "";
	UpdateData( FALSE );
}

void CDialogParticleEditor::CurStageToDlgVars() {

	// go ahead and get the two system vars too
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	depthHack = va( "%.3f", idp->depthHack );

	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	matName = ps->material->GetName();
	animFrames = va( "%i", ps->animationFrames );
	animRate = va( "%i", ps->animationRate );
	color = ps->color.ToString();
	fadeColor = ps->fadeColor.ToString();
	fadeIn = va( "%.3f", ps->fadeInFraction );
	fadeOut = va( "%.3f", ps->fadeOutFraction );
	fadeFraction = va( "%.3f", ps->fadeIndexFraction );
	count = va( "%i", ps->totalParticles );
	time = va( "%.3f", ps->particleLife );
	timeOffset = va( "%.3f", ps->timeOffset );
	deadTime = va( "%.3f", ps->deadTime );
	gravity = va( "%.3f", ps->gravity );
	bunching = va( "%.3f", ps->spawnBunching );
	offset = ps->offset.ToString( 0 );
	xSize = va( "%.3f", ps->distributionParms[0] );
	ySize = va( "%.3f", ps->distributionParms[1] );
	zSize = va( "%.3f", ps->distributionParms[2] );
	ringOffset = va( "%.3f", ps->distributionParms[3] );
	directionParm = va( "%.3f", ps->directionParms[0] );
	direction = ps->directionType;
	orientation = ps->orientation;
	distribution = ps->distributionType;
	speedFrom = va( "%.3f", ps->speed.from );
	speedTo = va( "%.3f", ps->speed.to );
	rotationFrom = va( "%.3f", ps->rotationSpeed.from );
	rotationTo = va( "%.3f", ps->rotationSpeed.to );
	sizeFrom = va( "%.3f", ps->size.from );
	sizeTo = va( "%.3f", ps->size.to );
	aspectFrom = va( "%.3f", ps->aspect.from );
	aspectTo = va( "%.3f", ps->aspect.to );
	trails = va( "%f", ps->orientationParms[0] );
	trailTime = va( "%.3f", ps->orientationParms[1] );
	cycles = va( "%.3f", ps->cycles );
	customPath = ps->GetCustomPathName();
	customParms = "";
	customDesc = ps->GetCustomPathDesc();
	if ( ps->customPathType != PPATH_STANDARD ) {
		for ( int i = 0; i < ps->NumCustomPathParms(); i++ ) {
			customParms += va( "%.1f ", ps->customPathParms[i] );
		}
	}
	worldGravity = ps->worldGravity;
	initialAngle = va( "%.3f", ps->initialAngle );
	boundsExpansion = va( "%.3f", ps->boundsExpansion );
	randomDistribution = ps->randomDistribution;
	entityColor = ps->entityColor;	
	UpdateData( FALSE );
}

void CDialogParticleEditor::DlgVarsToCurStage() {

	// go ahead and set the two system vars too
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}
	idp->depthHack = atof( depthHack );

	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	ps->material = declManager->FindMaterial( matName );
	ps->animationFrames = atoi( animFrames );
	ps->animationRate = atoi( animRate );
	sscanf( color, "%f %f %f %f", &ps->color.x, &ps->color.y, &ps->color.z, &ps->color.w );
	sscanf( fadeColor, "%f %f %f %f", &ps->fadeColor.x, &ps->fadeColor.y, &ps->fadeColor.z, &ps->fadeColor.w );
	ps->fadeInFraction = atof( fadeIn );
	ps->fadeOutFraction = atof( fadeOut );
	ps->fadeIndexFraction = atof( fadeFraction );
	ps->totalParticles = atoi( count );
	ps->particleLife = atof( time );
	ps->timeOffset = atof( timeOffset );
	ps->deadTime = atof( deadTime );
	ps->gravity = atof( gravity );
	ps->spawnBunching = atof( bunching );
	sscanf( offset, "%f %f %f", &ps->offset.x, &ps->offset.y, &ps->offset.z );
	ps->distributionParms[0] = atof( xSize );
	ps->distributionParms[1] = atof( ySize );
	ps->distributionParms[2] = atof( zSize );
	ps->distributionParms[3] = atof( ringOffset );
	ps->directionParms[0] = atof( directionParm );
	ps->directionType = static_cast<prtDirection_t>( direction );
	ps->orientation = static_cast<prtOrientation_t>( orientation );
	ps->distributionType = static_cast<prtDistribution_t>( distribution );
	ps->speed.from = atof( speedFrom );
	ps->speed.to = atof( speedTo );
	ps->rotationSpeed.from = atof( rotationFrom );
	ps->rotationSpeed.to = atof( rotationTo );
	ps->size.from = atof( sizeFrom );
	ps->size.to = atof( sizeTo );
	ps->aspect.from = atof( aspectFrom );
	ps->aspect.to = atof( aspectTo );
	ps->orientationParms[0] = atof( trails );
	ps->orientationParms[1] = atof( trailTime );
	ps->worldGravity = ( worldGravity == TRUE );
	ps->cycles = atof( cycles );
	ps->cycleMsec = ( ps->particleLife + ps->deadTime ) * 1000;

	sscanf( customParms, "%f %f %f %f %f %f %f %f", &ps->customPathParms[0], &ps->customPathParms[1], &ps->customPathParms[2], 
		   &ps->customPathParms[3], &ps->customPathParms[4], &ps->customPathParms[5],
		   &ps->customPathParms[6], &ps->customPathParms[7] );

	ps->SetCustomPathType( customPath );

	ps->initialAngle = atof( initialAngle );
	ps->boundsExpansion = atof( boundsExpansion );
	ps->randomDistribution = ( randomDistribution != FALSE );
	ps->entityColor = ( entityColor == TRUE );

}

void CDialogParticleEditor::ShowCurrentStage() {
	ClearDlgVars();
	idParticleStage *ps = GetCurStage();
	if ( ps == NULL ) {
		return;
	}
	CurStageToDlgVars();
	UpdateControlInfo();
}

void CDialogParticleEditor::OnLbnSelchangeListStages() {
	ShowCurrentStage();
	EnableStageControls();
}

void CDialogParticleEditor::OnBnClickedButtonNew() {
	DialogName dlg("New Particle");
	if (dlg.DoModal() == IDOK) {
		CFileDialog dlgSave( TRUE, "prt", NULL, OFN_CREATEPROMPT, "Particle Files (*.prt)|*.prt||All Files (*.*)|*.*||", AfxGetMainWnd() );
		if ( dlgSave.DoModal() == IDOK ) {
			if ( declManager->FindType( DECL_PARTICLE, dlg.m_strName, false ) ) {
				MessageBox( "Particle already exists!", "Particle exists", MB_OK );
				return;
			}
			idStr fileName;
			fileName = fileSystem->OSPathToRelativePath( dlgSave.m_ofn.lpstrFile );
			idDecl *decl = declManager->CreateNewDecl( DECL_PARTICLE, dlg.m_strName, fileName );
			if ( decl ) {
				if ( MessageBox( "Copy current particle?", "Copy current", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
					MessageBox( "Copy current particle not implemented yet.. Stay tuned" );
				}
				EnumParticles();
				int index = comboParticle.FindStringExact( -1, dlg.m_strName );
				if ( index >= 0 ) {
					comboParticle.SetCurSel( index );
				}
				OnBnClickedButtonSave();
				OnCbnSelchangeComboParticles();
			}
		}
	}
}

void CDialogParticleEditor::OnBnClickedButtonSave() {
	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}

	if ( strstr( idp->GetFileName(), "implicit" ) ) {
		// defaulted, need to choose a file 
		CFileDialog dlgSave( FALSE, "prt", NULL, OFN_OVERWRITEPROMPT, "Particle Files (*.prt)|*.prt||All Files (*.*)|*.*||", AfxGetMainWnd() );
		if ( dlgSave.DoModal() == IDOK ) {
			idStr fileName;
			fileName = fileSystem->OSPathToRelativePath( dlgSave.m_ofn.lpstrFile );
			idp->Save( fileName );
			EnumParticles();
		}
	} else {
		idp->Save();
	}

}

void CDialogParticleEditor::EnumParticles() {
	CWaitCursor cursor;
	comboParticle.ResetContent();
	for ( int i = 0; i < declManager->GetNumDecls( DECL_PARTICLE ); i++ ) {
		const idDecl *idp = declManager->DeclByIndex( DECL_PARTICLE, i );
		int index = comboParticle.AddString( idp->GetName() );
		if ( index >= 0 ) {
			comboParticle.SetItemData( index, i );
		}
	}
	comboParticle.SetCurSel( 0 );
	OnCbnSelchangeComboParticles();
}

void CDialogParticleEditor::OnDestroy() {
	com_editors &= ~EDITOR_PARTICLE;
	return CDialog::OnDestroy();
}

void VectorCallBack( idQuat rotation ) {
	if ( g_ParticleDialog && g_ParticleDialog->GetSafeHwnd() ) {
		g_ParticleDialog->SetVectorControlUpdate( rotation );
	}
}

void CDialogParticleEditor::SetVectorControlUpdate( idQuat rotation ) {
	if ( particleMode ) {
		idList<idEntity*> list;

		list.SetNum( 128 );
		int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
		list.SetNum( count );

		if ( count ) {
			for ( int i = 0; i < count; i++ ) {
				const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
				if ( dict == NULL ) {
					continue;
				}
				const char *name = dict->GetString( "name" );
				gameEdit->EntitySetAxis( list[i], rotation.ToMat3() );
				gameEdit->EntityUpdateVisuals( list[i] );
				gameEdit->MapSetEntityKeyVal( name, "rotation", rotation.ToMat3().ToString() );
			}
			CWnd *wnd = GetDlgItem( IDC_BUTTON_SAVE_PARTICLEENTITIES );
			if ( wnd ) {
				wnd->EnableWindow( TRUE );
			}
		}
	}
}

BOOL CDialogParticleEditor::OnInitDialog() {
	
	com_editors |= EDITOR_PARTICLE;

	particleMode = ( cvarSystem->GetCVarInteger( "g_editEntityMode" ) == 4 );
	mapModified = false;

	CDialog::OnInitDialog();
	
	sliderBunching.SetRange( 0, 20 );
	sliderBunching.SetValueRange( 0.0f, 1.0f );
	sliderFadeIn.SetRange( 0, 20 );
	sliderFadeIn.SetValueRange( 0.0f, 1.0f );
	sliderFadeOut.SetRange( 0, 20 );
	sliderFadeOut.SetValueRange( 0.0f, 1.0f );
	sliderCount.SetRange( 0, 1024 );
	sliderCount.SetValueRange( 0, 4096 );
	sliderTime.SetRange( 0, 200 );
	sliderTime.SetValueRange( 0.0f, 10.0f );
	sliderGravity.SetRange( 0, 600 );
	sliderGravity.SetValueRange( -300.0f, 300.0f );
	sliderSpeedFrom.SetRange( 0, 600 );
	sliderSpeedFrom.SetValueRange( -300.0f, 300.0f );
	sliderSpeedTo.SetRange( 0, 600 );
	sliderSpeedTo.SetValueRange( -300.0f, 300.0f );
	sliderRotationFrom.SetRange( 0, 100 );
	sliderRotationFrom.SetValueRange( 0.0f, 100.0f );
	sliderRotationTo.SetRange( 0, 100 );
	sliderRotationTo.SetValueRange( 0.0f, 100.0f );
	sliderSizeFrom.SetRange( 0, 256 );
	sliderSizeFrom.SetValueRange( 0.0f, 128.0f );
	sliderSizeTo.SetRange( 0, 256 );
	sliderSizeTo.SetValueRange( 0.0f, 128.0f );
	sliderAspectFrom.SetRange( 0, 256 );
	sliderAspectFrom.SetValueRange( 0.0f, 128.0f );
	sliderAspectTo.SetRange( 0, 256 );
	sliderAspectTo.SetValueRange( 0.0f, 128.0f );
	sliderFadeFraction.SetRange( 0, 20 );
	sliderFadeFraction.SetValueRange( 0.0f, 1.0f );
	
	EnumParticles();
	SetParticleView();

	toolTipCtrl.Create( this );
	toolTipCtrl.Activate( TRUE );

	CWnd* wnd = GetWindow( GW_CHILD );
	CString str;
	while ( wnd ) {
		if ( str.LoadString( wnd->GetDlgCtrlID() ) ) {
			toolTipCtrl.AddTool( wnd, str );
		}
		wnd = wnd->GetWindow( GW_HWNDNEXT );
	}
	
	wnd = GetDlgItem( IDC_BUTTON_SAVE_PARTICLEENTITIES );
	if ( wnd ) {
		wnd->EnableWindow( FALSE );
	}
	EnableEditControls();

	vectorControl.SetVectorChangingCallback( VectorCallBack );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogParticleEditor::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) {
	CDialog::OnHScroll( nSBCode, nPos, pScrollBar );
	CSliderCtrl *ctrl = dynamic_cast< CSliderCtrl* >( pScrollBar );
	if ( !ctrl ) {
		return;
	}
	if ( ctrl == &sliderBunching ) {
		// handle bunching
		bunching = va( "%.3f", sliderBunching.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderFadeIn ) {
		fadeIn = va( "%.3f", sliderFadeIn.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderFadeOut ) {
		fadeOut = va( "%.3f", sliderFadeOut.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderFadeFraction ) {
		fadeFraction = va( "%.3f", sliderFadeFraction.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderCount ) {
		count = va( "%i", (int)sliderCount.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderTime ) {
		time = va( "%.3f", sliderTime.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderGravity ) {
		gravity = va( "%.3f", sliderGravity.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderSpeedFrom ) {
		speedFrom = va( "%.3f", sliderSpeedFrom.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderSpeedTo ) {
		speedTo = va( "%.3f", sliderSpeedTo.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderRotationFrom ) {
		rotationFrom = va( "%.3f", sliderRotationFrom.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderRotationTo ) {
		rotationTo = va( "%.3f", sliderRotationTo.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderSizeFrom ) {
		sizeFrom = va( "%.3f", sliderSizeFrom.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderSizeTo ) {
		sizeTo = va( "%.3f", sliderSizeTo.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderAspectFrom ) {
		aspectFrom = va( "%.3f", sliderAspectFrom.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	} else if ( ctrl == &sliderAspectTo ) {
		aspectTo = va( "%.3f", sliderAspectTo.GetValue() );
		DlgVarsToCurStage();
		CurStageToDlgVars();
	}
}


BOOL CDialogParticleEditor::PreTranslateMessage(MSG *pMsg) {
	if ( pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST ) {
		toolTipCtrl.RelayEvent( pMsg );
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CDialogParticleEditor::EnableStageControls() {
	idParticleStage *stage = GetCurStage();
	bool b = ( stage && stage->hidden ) ? false : true;
	for ( int i = 0; i < StageIDCount; i++ ) {
		CWnd *wnd = GetDlgItem( StageEnableID[ i ] );
		if ( wnd ) {
			wnd->EnableWindow( b );
		}
	}
}

void CDialogParticleEditor::EnableEditControls() {
	for ( int i = 0; i < EditIDCount; i++ ) {
		CWnd *wnd = GetDlgItem( EditEnableID[ i ] );
		if ( wnd ) {
			wnd->EnableWindow( particleMode );
		}
	}
}

void CDialogParticleEditor::UpdateSelectedOrigin( float x, float y, float z ) {
	idList<idEntity*> list;
	idVec3 origin;
	idVec3 vec(x, y, z);

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		for ( int i = 0; i < count; i++ ) {
			const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
			if ( dict == NULL ) {
				continue;
			}
			const char *name = dict->GetString( "name" );
			gameEdit->EntityTranslate( list[i], vec );
			gameEdit->EntityUpdateVisuals( list[i] );
			gameEdit->MapEntityTranslate( name, vec );
		}
		CWnd *wnd = GetDlgItem( IDC_BUTTON_SAVE_PARTICLEENTITIES );
		if ( wnd ) {
			wnd->EnableWindow( TRUE );
		}
	}
}

void CDialogParticleEditor::OnBtnYup() 
{
	UpdateSelectedOrigin(0, 8, 0);
}

void CDialogParticleEditor::OnBtnYdn() 
{
	UpdateSelectedOrigin(0, -8, 0);
}

void CDialogParticleEditor::OnBtnXdn() 
{
	UpdateSelectedOrigin(-8, 0, 0);
}

void CDialogParticleEditor::OnBtnXup() 
{
	UpdateSelectedOrigin(8, 0, 0);
}

void CDialogParticleEditor::OnBtnZup() 
{
	UpdateSelectedOrigin(0, 0, 8);
}

void CDialogParticleEditor::OnBtnZdn() 
{
	UpdateSelectedOrigin(0, 0, -8);
}

void CDialogParticleEditor::OnBtnDrop() 
{
	idStr		classname;
	idStr		key;
	idStr		value;
	idVec3		org;
	idDict		args;
	idAngles	viewAngles;

	if ( !gameEdit->PlayerIsValid() ) {
		return;
	}
	
	gameEdit->PlayerGetViewAngles( viewAngles );
	gameEdit->PlayerGetEyePosition( org );

	org += idAngles( 0, viewAngles.yaw, 0 ).ToForward() * 80 + idVec3( 0, 0, 1 );
	args.Set("origin", org.ToString());
	args.Set("classname", "func_emitter");
	args.Set("angle", va( "%f", viewAngles.yaw + 180 ));

	idDeclParticle *idp = GetCurParticle();
	if ( idp == NULL ) {
		return;
	}
	idStr str = idp->GetName();
	str.SetFileExtension( ".prt" );

	args.Set("model", str);

	idStr name = gameEdit->GetUniqueEntityName( "func_emitter" );
	bool nameValid = false;
	while (!nameValid) {
		DialogName dlg("Name Particle", this);
		dlg.m_strName = name;
		if (dlg.DoModal() == IDOK) {
			idEntity *gameEnt = gameEdit->FindEntity( dlg.m_strName );
			if (gameEnt) {
				if (MessageBox("Please choose another name", "Duplicate Entity Name!", MB_OKCANCEL) == IDCANCEL) {
					return;
				}
			} else {
				nameValid = true;
				name = dlg.m_strName;
			}
		}
	}

	args.Set("name", name.c_str());

	idEntity *ent = NULL;
	gameEdit->SpawnEntityDef( args, &ent );
	if (ent) {
		gameEdit->EntityUpdateChangeableSpawnArgs( ent, NULL );
		gameEdit->ClearEntitySelection();
		gameEdit->AddSelectedEntity( ent );
	}

	gameEdit->MapAddEntity( &args );
}

void CDialogParticleEditor::OnOK()
{
	// never return on OK as windows will map this at times when you don't want
	// ENTER closing the dialog
	// CDialog::OnOK();
}

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

#include "qe3.h"
#include "Radiant.h"
#include "../../game/game.h"
#include "../comafx/DialogColorPicker.h"
#include "LightDlg.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif


void CLightInfo::Defaults() {
	pointLight = true;
	fallOff = 1;
	strTexture = "";
	equalRadius = true;
	explicitStartEnd = false;
	lightRadius.Zero();
	lightTarget.Zero();
	lightRight.Zero();
	lightUp.Zero();
	lightStart.Zero();
	lightEnd.Zero();
	lightCenter.Zero();
	hasCenter = false;
	isParallel = false;
	castShadows = true;
	castSpecular = true;
	castDiffuse = true;
	rotate = false;
	strobe = false;
	rotateSpeed = 0;
	strobeSpeed = 0;
	color[0] = color[1] = color[2] = 255;
	fogDensity[0] = fogDensity[1] = fogDensity[2] = 0;
	fog = false;
	lightRadius[0] = lightRadius[1] = lightRadius[2] = 300;
}


void CLightInfo::DefaultPoint() {
	idVec3 oldColor = color;
	Defaults();
	color = oldColor;
	pointLight = true;
}

void CLightInfo::DefaultProjected() {
	idVec3 oldColor = color;
	Defaults();
	color = oldColor;
	pointLight = false;
	lightTarget[2] = -256;
	lightUp[1] = -128;
	lightRight[0] = -128;
}

void CLightInfo::FromDict( const idDict *e ) {

	lightRadius.Zero();
	lightTarget.Zero();
	lightRight.Zero();
	lightUp.Zero();
	lightStart.Zero();
	lightEnd.Zero();
	lightCenter.Zero();

	castShadows = !e->GetBool("noshadows");
	castSpecular = !e->GetBool("nospecular");
	castDiffuse = !e->GetBool("nodiffuse");
	fallOff = e->GetFloat("falloff");
	strTexture = e->GetString("texture");

	isParallel = e->GetBool("parallel");

	if (!e->GetVector("_color", "", color)) {
		color[0] = color[1] = color[2] = 1;
	}
	// windows needs 0-255 scale
	color[0] *= 255;
	color[1] *= 255;
	color[2] *= 255;

	if (e->GetVec4("fog", "", fogDensity)) {
		fog = true;
	} else {
		fog = false;
	}

	if (e->GetVector("light_right","", lightRight)) {
		// projected light
		pointLight = false;
		e->GetVector("light_target", "", lightTarget);
		e->GetVector("light_up", "", lightUp);
		if (e->GetVector("light_start", "", lightStart)) {
			// explicit start and end points
			explicitStartEnd = true;
			if (!e->GetVector("light_end", "", lightEnd)) {
				// no end, use target
				VectorCopy(lightTarget, lightEnd);
			}
		} else {
			explicitStartEnd = false;
			// create a start a quarter of the way to the target
			lightStart = lightTarget * 0.25;
			VectorCopy(lightTarget, lightEnd);
		}
	} else {
		pointLight = true;
		if (e->GetVector("light_radius", "", lightRadius)) {
			equalRadius = false;
		} else {
			float radius = e->GetFloat("light");
			if (radius == 0) {
				radius = 300;
			}
			lightRadius[0] = lightRadius[1] = lightRadius[2] = radius;
			equalRadius = true;
		}
		if (e->GetVector("light_center", "", lightCenter)) {
			hasCenter = true;
		}
	}
}

void CLightInfo::ToDictFromDifferences ( idDict *e, const idDict *differences ) {
    for ( int i = 0 ; i < differences->GetNumKeyVals () ; i ++ ) {
        const idKeyValue *kv = differences->GetKeyVal( i );
        
        if ( kv->GetValue().Length() > 0 ) {
            e->Set ( kv->GetKey() ,kv->GetValue() );
		} else {
	        e->Delete ( kv->GetKey() );
		}

        common->Printf( "Applied difference: %s %s\n" , kv->GetKey().c_str() , kv->GetValue().c_str() );
    }
}

//write all info to a dict, regardless of light type
void CLightInfo::ToDictWriteAllInfo( idDict *e ) {
    e->Set("noshadows", (!castShadows) ? "1" : "0");
	e->Set("nospecular", (!castSpecular) ? "1" : "0");
	e->Set("nodiffuse", (!castDiffuse) ? "1" : "0");

	e->SetFloat("falloff",fallOff);
	
	if (strTexture.GetLength() > 0 ) {
		e->Set("texture", strTexture);
	}

	idVec3 temp = color;
	temp /= 255;
	e->SetVector("_color", temp);

	if (!equalRadius) {
		e->Set("light_radius", va("%g %g %g", lightRadius[0], lightRadius[1], lightRadius[2]));
	} else {
		e->Set("light_radius", va("%g %g %g", lightRadius[0], lightRadius[0], lightRadius[0]));
	}

	e->Set("light_center", va("%g %g %g", lightCenter[0], lightCenter[1], lightCenter[2]));
    e->Set("parallel", isParallel?"1":"0");

    e->Set("light_target", va("%g %g %g", lightTarget[0], lightTarget[1], lightTarget[2]));
	e->Set("light_up", va("%g %g %g", lightUp[0], lightUp[1], lightUp[2]));
	e->Set("light_right", va("%g %g %g", lightRight[0], lightRight[1], lightRight[2]));
    e->Set("light_start", va("%g %g %g", lightStart[0], lightStart[1], lightStart[2]));
	e->Set("light_end", va("%g %g %g", lightEnd[0], lightEnd[1], lightEnd[2]));
}

void CLightInfo::ToDict( idDict *e ) {

	e->Delete("noshadows");
	e->Delete("nospecular");
	e->Delete("nodiffuse");
	e->Delete("falloff");
	e->Delete("parallel");
	e->Delete("texture");
	e->Delete("_color");
	e->Delete("fog");
	e->Delete("light_target");
	e->Delete("light_right");
	e->Delete("light_up");
	e->Delete("light_start");
	e->Delete("light_end");
	e->Delete("light_radius");
	e->Delete("light_center");
	e->Delete("light");

	e->Set("noshadows", (!castShadows) ? "1" : "0");
	e->Set("nospecular", (!castSpecular) ? "1" : "0");
	e->Set("nodiffuse", (!castDiffuse) ? "1" : "0");

	e->SetFloat("falloff",fallOff);
	
	if (strTexture.GetLength() > 0) {
		e->Set("texture", strTexture);
	}

	idVec3 temp = color;
	temp /= 255;
	e->SetVector("_color", temp);

	if (fog) {
		e->Set("fog", va("%g %g %g %g", fogDensity[0]/255.0, fogDensity[1]/255.0, fogDensity[2]/255.0, fogDensity[3]/255.0));
	}

	if (pointLight) {
		if (!equalRadius) {
			e->Set("light_radius", va("%g %g %g", lightRadius[0], lightRadius[1], lightRadius[2]));
		} else {
			e->Set("light_radius", va("%g %g %g", lightRadius[0], lightRadius[0], lightRadius[0]));
		}

		if (hasCenter) {
			e->Set("light_center", va("%g %g %g", lightCenter[0], lightCenter[1], lightCenter[2]));
		}

		if (isParallel) {
			e->Set("parallel", "1");
		}
	} else {
		e->Set("light_target", va("%g %g %g", lightTarget[0], lightTarget[1], lightTarget[2]));
		e->Set("light_up", va("%g %g %g", lightUp[0], lightUp[1], lightUp[2]));
		e->Set("light_right", va("%g %g %g", lightRight[0], lightRight[1], lightRight[2]));
		if (explicitStartEnd) {
			e->Set("light_start", va("%g %g %g", lightStart[0], lightStart[1], lightStart[2]));
			e->Set("light_end", va("%g %g %g", lightEnd[0], lightEnd[1], lightEnd[2]));
		}
	}
}

CLightInfo::CLightInfo() {
	Defaults();
}



/////////////////////////////////////////////////////////////////////////////
// CLightDlg dialog

CLightDlg *g_LightDialog = NULL;


CLightDlg::CLightDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLightDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLightDlg)
	m_bEqualRadius = FALSE;
	m_bExplicitFalloff = FALSE;
	m_bPointLight = FALSE;
	m_bCheckProjected = FALSE;
	m_fFallloff = 0.0f;
	m_nFalloff = -1;
	m_bRotate = FALSE;
	m_bShadows = FALSE;
	m_bSpecular = FALSE;
	m_bDiffuse = FALSE;
	m_fEndX = 0.0f;
	m_fEndY = 0.0f;
	m_fEndZ = 0.0f;
	m_fRadiusX = 0.0f;
	m_fRadiusY = 0.0f;
	m_fRadiusZ = 0.0f;
	m_fRightX = 0.0f;
	m_fRightY = 0.0f;
	m_fRightZ = 0.0f;
	m_fRotate = 0.0f;
	m_fStartX = 0.0f;
	m_fStartY = 0.0f;
	m_fStartZ = 0.0f;
	m_fTargetX = 0.0f;
	m_fTargetY = 0.0f;
	m_fTargetZ = 0.0f;
	m_fUpX = 0.0f;
	m_fUpY = 0.0f;
	m_fUpZ = 0.0f;
	m_hasCenter = FALSE;
	m_centerX = 0.0f;
	m_centerY = 0.0f;
	m_centerZ = 0.0f;
    m_bIsParallel = FALSE;
	//}}AFX_DATA_INIT
	m_drawMaterial = new idGLDrawableMaterial();
}

CLightDlg::~CLightDlg() {
	delete m_drawMaterial;
}

void CLightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightDlg)
	if ( com_editorActive ) {
		DDX_Control(pDX, IDC_LIGHTPREVIEW, m_wndPreview);
	}
	DDX_Control(pDX, IDC_COMBO_TEXTURE, m_wndLights);
	DDX_Check(pDX, IDC_CHECK_EQUALRADIUS, m_bEqualRadius);
	DDX_Check(pDX, IDC_CHECK_EXPLICITFALLOFF, m_bExplicitFalloff);
	DDX_Check(pDX, IDC_CHECK_POINT, m_bPointLight);
	DDX_Check(pDX, IDC_CHECK_PROJECTED, m_bCheckProjected);
	DDX_Radio(pDX, IDC_RADIO_FALLOFF, m_nFalloff);
	DDX_Check(pDX, IDC_CHECK_SHADOWS, m_bShadows);
	DDX_Check(pDX, IDC_CHECK_SPECULAR, m_bSpecular);
	DDX_Check(pDX, IDC_CHECK_DIFFUSE, m_bDiffuse);
    DDX_Check(pDX , IDC_CHECK_PARALLEL , m_bIsParallel );
	DDX_Text(pDX, IDC_EDIT_ENDX, m_fEndX);
	DDX_Text(pDX, IDC_EDIT_ENDY, m_fEndY);
	DDX_Text(pDX, IDC_EDIT_ENDZ, m_fEndZ);
	DDX_Text(pDX, IDC_EDIT_RADIUSX, m_fRadiusX);
	DDX_Text(pDX, IDC_EDIT_RADIUSY, m_fRadiusY);
	DDX_Text(pDX, IDC_EDIT_RADIUSZ, m_fRadiusZ);
	DDX_Text(pDX, IDC_EDIT_RIGHTX, m_fRightX);
	DDX_Text(pDX, IDC_EDIT_RIGHTY, m_fRightY);
	DDX_Text(pDX, IDC_EDIT_RIGHTZ, m_fRightZ);
	DDX_Text(pDX, IDC_EDIT_STARTX, m_fStartX);
	DDX_Text(pDX, IDC_EDIT_STARTY, m_fStartY);
	DDX_Text(pDX, IDC_EDIT_STARTZ, m_fStartZ);
	DDX_Text(pDX, IDC_EDIT_TARGETX, m_fTargetX);
	DDX_Text(pDX, IDC_EDIT_TARGETY, m_fTargetY);
	DDX_Text(pDX, IDC_EDIT_TARGETZ, m_fTargetZ);
	DDX_Text(pDX, IDC_EDIT_UPX, m_fUpX);
	DDX_Text(pDX, IDC_EDIT_UPY, m_fUpY);
	DDX_Text(pDX, IDC_EDIT_UPZ, m_fUpZ);
	DDX_Check(pDX, IDC_CHECK_CENTER, m_hasCenter);
	DDX_Text(pDX, IDC_EDIT_CENTERX, m_centerX);
	DDX_Text(pDX, IDC_EDIT_CENTERY, m_centerY);
	DDX_Text(pDX, IDC_EDIT_CENTERZ, m_centerZ);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightDlg, CDialog)
	//{{AFX_MSG_MAP(CLightDlg)
	ON_BN_CLICKED(IDC_BTN_TEXTURE, OnBtnTexture)
	ON_BN_CLICKED(IDC_CHECK_EQUALRADIUS, OnCheckEqualradius)
	ON_BN_CLICKED(IDC_CHECK_EXPLICITFALLOFF, OnCheckExplicitfalloff)
	ON_BN_CLICKED(IDC_CHECK_POINT, OnCheckPoint)
	ON_BN_CLICKED(IDC_CHECK_PROJECTED, OnCheckProjected)
	ON_BN_CLICKED(IDC_RADIO_FALLOFF, OnRadioFalloff)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
    ON_BN_CLICKED(IDC_APPLY_DIFFERENT, OnApplyDifferences)
	ON_BN_CLICKED(IDC_BTN_COLOR, OnBtnColor)
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE(IDC_COMBO_TEXTURE, OnSelchangeComboTexture)
	ON_BN_CLICKED(IDC_CHECK_CENTER, OnCheckCenter)
	ON_BN_CLICKED(IDC_CHECK_PARALLEL, OnCheckParallel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightDlg message handlers

void CLightDlg::SetSpecifics() {
	if (lightInfo.pointLight) {
		GetDlgItem(IDC_EDIT_RADIUSY)->EnableWindow(!lightInfo.equalRadius);
		GetDlgItem(IDC_EDIT_RADIUSZ)->EnableWindow(!lightInfo.equalRadius);
		GetDlgItem(IDC_EDIT_CENTERX)->EnableWindow(lightInfo.hasCenter);
		GetDlgItem(IDC_EDIT_CENTERY)->EnableWindow(lightInfo.hasCenter);
		GetDlgItem(IDC_EDIT_CENTERZ)->EnableWindow(lightInfo.hasCenter);
	} else {
		GetDlgItem(IDC_EDIT_STARTX)->EnableWindow(lightInfo.explicitStartEnd);
		GetDlgItem(IDC_EDIT_STARTY)->EnableWindow(lightInfo.explicitStartEnd);
		GetDlgItem(IDC_EDIT_STARTZ)->EnableWindow(lightInfo.explicitStartEnd);
		GetDlgItem(IDC_EDIT_ENDX)->EnableWindow(lightInfo.explicitStartEnd);
		GetDlgItem(IDC_EDIT_ENDY)->EnableWindow(lightInfo.explicitStartEnd);
		GetDlgItem(IDC_EDIT_ENDZ)->EnableWindow(lightInfo.explicitStartEnd);
	}
}

void CLightDlg::EnableControls() {
	GetDlgItem(IDC_CHECK_EQUALRADIUS)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_RADIUSX)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_RADIUSY)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_RADIUSZ)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_RADIO_FALLOFF)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_RADIO_FALLOFF2)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_RADIO_FALLOFF3)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_TARGETX)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_TARGETY)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_TARGETZ)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_RIGHTX)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_RIGHTY)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_RIGHTZ)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_UPX)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_UPY)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_UPZ)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_STARTX)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_STARTY)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_STARTZ)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_ENDX)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_ENDY)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_ENDZ)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_CHECK_EXPLICITFALLOFF)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_CHECK_POINT)->EnableWindow(!lightInfo.pointLight);
	GetDlgItem(IDC_CHECK_PROJECTED)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_CENTERX)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_CENTERY)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_EDIT_CENTERZ)->EnableWindow(lightInfo.pointLight);
	GetDlgItem(IDC_CHECK_CENTER)->EnableWindow(lightInfo.pointLight);

	reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_PROJECTED))->SetCheck(!lightInfo.pointLight);
	reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_POINT))->SetCheck(lightInfo.pointLight);

	SetSpecifics();
}

void CLightDlg::UpdateDialogFromLightInfo( void ) {
	m_hasCenter = lightInfo.hasCenter;
	m_bEqualRadius = lightInfo.equalRadius;
	m_bExplicitFalloff = lightInfo.explicitStartEnd;
	m_bPointLight = lightInfo.pointLight;
	m_bCheckProjected = !lightInfo.pointLight;
	m_fFallloff = lightInfo.fallOff;
	if (lightInfo.fallOff < 0.35) {
		m_nFalloff = 0;
	} else if (lightInfo.fallOff < 0.70) {
		m_nFalloff = 1;
	} else {
		m_nFalloff = 2;
	}
	//m_bFog = lightInfo.fog;
	m_bRotate = lightInfo.rotate;
	m_bShadows = lightInfo.castShadows;
	m_bSpecular = lightInfo.castSpecular;
	//m_bStrobe = lightInfo.strobe;
	//m_fStrobe = lightInfo.strobeSpeed;
	int sel = m_wndLights.FindStringExact(-1, lightInfo.strTexture);
	m_wndLights.SetCurSel(sel);
	if (sel >= 0) {
		m_drawMaterial->setMedia(lightInfo.strTexture);
	} else {
		m_drawMaterial->setMedia(lightInfo.strTexture);
	}

	m_bDiffuse = lightInfo.castDiffuse;
	m_fEndX = lightInfo.lightEnd[0];
	m_fEndY = lightInfo.lightEnd[1];
	m_fEndZ = lightInfo.lightEnd[2];
	m_fRadiusX = lightInfo.lightRadius[0];
	m_fRadiusY = lightInfo.lightRadius[1];
	m_fRadiusZ = lightInfo.lightRadius[2];
	m_fRightX = lightInfo.lightRight[0];
	m_fRightY = lightInfo.lightRight[1];
	m_fRightZ = lightInfo.lightRight[2];
	//m_bRotate = lightInfo.rotate;
	//m_fRotate = lightInfo.rotateSpeed;
	m_fStartX = lightInfo.lightStart[0];
	m_fStartY = lightInfo.lightStart[1];
	m_fStartZ = lightInfo.lightStart[2];
	m_fTargetX = lightInfo.lightTarget[0];
	m_fTargetY = lightInfo.lightTarget[1];
	m_fTargetZ = lightInfo.lightTarget[2];
	m_fUpX = lightInfo.lightUp[0];
	m_fUpY = lightInfo.lightUp[1];
	m_fUpZ = lightInfo.lightUp[2];
	VectorCopy(lightInfo.color, color);
	//m_fFogAlpha = lightInfo.fogDensity[3];
	m_centerX = lightInfo.lightCenter[0];
	m_centerY = lightInfo.lightCenter[1];
	m_centerZ = lightInfo.lightCenter[2];

    //jhefty - added parallel light updating
    m_bIsParallel = lightInfo.isParallel;

	UpdateData(FALSE);
}

void CLightDlg::UpdateLightInfoFromDialog( void ) {
	UpdateData( TRUE );
	
	lightInfo.pointLight = ( m_bPointLight != FALSE );
	lightInfo.equalRadius = ( m_bEqualRadius != FALSE );
	lightInfo.explicitStartEnd = ( m_bExplicitFalloff != FALSE );
	
	if (lightInfo.pointLight) {
		if (m_nFalloff == 0) {
			m_fFallloff = 0.0;
		} else if (m_nFalloff == 1) {
			m_fFallloff = 0.5;
		} else {
			m_fFallloff = 1.0;
		}
	}

	lightInfo.fallOff = m_fFallloff;

	//lightInfo.fog = m_bFog;
	lightInfo.rotate = ( m_bRotate != FALSE );
	lightInfo.castShadows = ( m_bShadows != FALSE );
	lightInfo.castSpecular = ( m_bSpecular != FALSE );

	VectorCopy(color, lightInfo.color);
    lightInfo.isParallel = (m_bIsParallel == TRUE);

	//lightInfo.fogDensity[3] = m_fFogAlpha;

	//lightInfo.strobe = m_bStrobe;
	//lightInfo.strobeSpeed = m_fStrobe;
	//lightInfo.rotate = m_bRotate; 
	//lightInfo.rotateSpeed = m_fRotate;

	int sel = m_wndLights.GetCurSel();
	CString str("");
	if (sel >= 0) {
		m_wndLights.GetLBText(sel, str);
	}
	lightInfo.strTexture = str;

	lightInfo.castDiffuse = ( m_bDiffuse != FALSE );
	lightInfo.lightEnd[0] = m_fEndX;
	lightInfo.lightEnd[1] = m_fEndY;
	lightInfo.lightEnd[2] = m_fEndZ;
	lightInfo.lightRadius[0] = m_fRadiusX;
	lightInfo.lightRadius[1] = m_fRadiusY; 
	lightInfo.lightRadius[2] = m_fRadiusZ; 
	lightInfo.lightRight[0] = m_fRightX;
	lightInfo.lightRight[1] = m_fRightY;
	lightInfo.lightRight[2] = m_fRightZ;
	lightInfo.lightStart[0] = m_fStartX;
	lightInfo.lightStart[1] = m_fStartY;
	lightInfo.lightStart[2] = m_fStartZ;
	lightInfo.lightTarget[0] = m_fTargetX;
	lightInfo.lightTarget[1] = m_fTargetY; 
	lightInfo.lightTarget[2] = m_fTargetZ;
	lightInfo.lightUp[0] = m_fUpX;
	lightInfo.lightUp[1] = m_fUpY; 
	lightInfo.lightUp[2] = m_fUpZ;

	lightInfo.hasCenter = ( m_hasCenter != FALSE );
	lightInfo.lightCenter[0] = m_centerX;
	lightInfo.lightCenter[1] = m_centerY;
	lightInfo.lightCenter[2] = m_centerZ;
}

void CLightDlg::SaveLightInfo( const idDict *differences ) {

	if ( com_editorActive ) {

		// used from Radiant
		for ( brush_t *b = selected_brushes.next; b && b != &selected_brushes; b = b->next ) {
			if ( ( b->owner->eclass->nShowFlags & ECLASS_LIGHT ) && !b->entityModel ) {
				if ( differences ) {
					lightInfo.ToDictFromDifferences( &b->owner->epairs, differences );
				} else {
					lightInfo.ToDict( &b->owner->epairs );
				}
				Brush_Build( b );
			}
		}

	} else {

		// used in-game
		idList<idEntity *> list;

		list.SetNum( 128 );
		int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
		list.SetNum( count );

		for ( int i = 0; i < count; i++ ) {
			if ( differences ) {
				gameEdit->EntityChangeSpawnArgs( list[i], differences );
				gameEdit->EntityUpdateChangeableSpawnArgs( list[i], NULL );
			} else {
				idDict newArgs;
				lightInfo.ToDict( &newArgs );
				gameEdit->EntityChangeSpawnArgs( list[i], &newArgs );
				gameEdit->EntityUpdateChangeableSpawnArgs( list[i], NULL );
			}
			gameEdit->EntityUpdateVisuals( list[i] );
		}
    }
}

void CLightDlg::ColorButtons() {
	CRect r;

	CClientDC dc(this);
	
	CButton *pBtn = (CButton *)GetDlgItem(IDC_BTN_COLOR);
	pBtn->GetClientRect(&r);
	colorBitmap.DeleteObject();
	colorBitmap.CreateCompatibleBitmap(&dc, r.Width(), r.Height());
	CDC MemDC;
	MemDC.CreateCompatibleDC(&dc);
	CBitmap *pOldBmp = MemDC.SelectObject(&colorBitmap);
	{
		CBrush br(RGB(color[0], color[1], color[2])); 
		MemDC.FillRect(r,&br);
	}
	dc.SelectObject(pOldBmp);
	pBtn->SetBitmap(HBITMAP(colorBitmap)); 
}


void CLightDlg::LoadLightTextures() {
	int count = declManager->GetNumDecls( DECL_MATERIAL );
	int i;
	const idMaterial *mat;
	for (i = 0; i < count; i++) {
		mat = declManager->MaterialByIndex(i, false);
		idStr str = mat->GetName();
		str.ToLower();
		if (str.Icmpn("lights/", strlen("lights/")) == 0 || str.Icmpn("fogs/", strlen("fogs/")) == 0) {
			m_wndLights.AddString(mat->GetName());
		}
	}
}

BOOL CLightDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	com_editors |= EDITOR_LIGHT;

	UpdateDialog( true );

	LoadLightTextures();

	if ( com_editorActive ) {
		m_wndPreview.setDrawable(m_drawMaterial);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLightDlg::OnDestroy() {

	com_editors &= ~EDITOR_LIGHT;

	return CDialog::OnDestroy();
}

void CLightDlg::OnBtnTexture() 
{
	// TODO: Add your control notification handler code here
	
}

void CLightDlg::OnCheckEqualradius() 
{
	lightInfo.equalRadius = ( reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_EQUALRADIUS))->GetCheck() != 0 );
	SetSpecifics();
}

void CLightDlg::OnCheckExplicitfalloff() 
{
	lightInfo.explicitStartEnd = ( reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_EXPLICITFALLOFF))->GetCheck() != 0 );
	SetSpecifics();
}

void CLightDlg::OnCheckPoint() 
{
	lightInfo.DefaultPoint();
	UpdateDialogFromLightInfo();
	EnableControls();
}

void CLightDlg::OnCheckProjected() 
{
	lightInfo.DefaultProjected();
	UpdateDialogFromLightInfo();
	EnableControls();
}

void CLightDlg::OnRadioFalloff() 
{
}

void CLightDlg::OnOK() {
	UpdateLightInfoFromDialog();
	SaveLightInfo( NULL );
	Sys_UpdateWindows(W_ALL);
	CDialog::OnOK();
}

entity_t *SingleLightSelected() {
	if ( QE_SingleBrush( true, true ) ) {
		brush_t *b = selected_brushes.next;
		if ( ( b->owner->eclass->nShowFlags & ECLASS_LIGHT ) && !b->entityModel ) {
			return b->owner;
		}
	}
	return NULL;
}

void CLightDlg::UpdateDialog( bool updateChecks )
{	
	CString title;

	lightInfo.Defaults();
	lightInfoOriginal.Defaults ();

	if ( com_editorActive ) {
		// used from Radiant
		entity_t *e = SingleLightSelected();
		if ( e ) {
			lightInfo.FromDict(&e->epairs);
			lightInfoOriginal.FromDict(&e->epairs); //our original copy of the values that we compare against for apply differences
			title = "Light Editor";
		} else {   
			//find the last brush belonging to the last entity selected and use that as the source
			e = NULL;
			for ( brush_t *b = selected_brushes.next ; b != &selected_brushes ; b = b->next ) {
				if ( ( b->owner->eclass->nShowFlags & ECLASS_LIGHT ) && !b->entityModel ) {
					e = b->owner;
					break;
				}
			}

			if ( e ) {
				lightInfo.FromDict( &e->epairs );
				lightInfoOriginal.FromDict(&e->epairs); //our original copy of the values that we compaer against for apply differences
				title = "Light Editor - (Multiple lights selected)";
			} else {
				title = "Light Editor - (No lights selected)";
			}
		}
	} else {
		// used in-game
		idList<idEntity *> list;

		list.SetNum( 128 );
		int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
		list.SetNum( count );

		if ( count > 0 ) {
			lightInfo.FromDict( gameEdit->EntityGetSpawnArgs( list[count-1] ) );
			title = "Light Editor";
		} else {
			title = "Light Editor - (No entities selected)";
		}
	}

	SetWindowText( title );

	UpdateDialogFromLightInfo();
	ColorButtons();

	if ( updateChecks ) {
		EnableControls();
	}
}

void LightEditorInit( const idDict *spawnArgs ) {
	if ( renderSystem->IsFullScreen() ) {
		common->Printf( "Cannot run the light editor in fullscreen mode.\n"
					"Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

	if ( g_LightDialog == NULL ) {
		InitAfx();
		g_LightDialog = new CLightDlg();
	}

	if ( g_LightDialog->GetSafeHwnd() == NULL ) {
		g_LightDialog->Create( IDD_DIALOG_LIGHT );
		CRect rct;
		LONG lSize = sizeof( rct );
		if ( LoadRegistryInfo( "Radiant::LightWindow", &rct, &lSize ) ) {
			g_LightDialog->SetWindowPos(NULL, rct.left, rct.top, 0,0, SWP_NOSIZE);
		}
	}

	idKeyInput::ClearStates();

	g_LightDialog->ShowWindow( SW_SHOW );
	g_LightDialog->SetFocus();
	g_LightDialog->UpdateDialog( true );

	if ( spawnArgs ) {
		// FIXME: select light based on spawn args
	}
}

void LightEditorRun( void ) {
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

void LightEditorShutdown( void ) {
	delete g_LightDialog;
	g_LightDialog = NULL;
}

void UpdateLightInspector() {
	if ( g_LightDialog && g_LightDialog->GetSafeHwnd() != NULL ) {
		g_LightDialog->UpdateDialog(true);   //jhefty - update ALL info about the light, including check boxes
	}
}

void CLightDlg::OnApply() {
	UpdateLightInfoFromDialog();
	SaveLightInfo( NULL );
	Sys_UpdateWindows( W_ALL );
}

void UpdateLightDialog( float r, float g, float b, float a ) {
	UpdateRadiantColor( 0.0f, 0.0f, 0.0f, 0.0f );
	g_LightDialog->UpdateColor( r, g, b, a );
}

void CLightDlg::UpdateColor( float r, float g, float b, float a ) {
	color[0] = a * r;
	color[1] = a * g;
	color[2] = a * b;
	ColorButtons();
	UpdateLightInfoFromDialog();
	SaveLightInfo( NULL );
	Sys_UpdateWindows( W_CAMERA );
}

void CLightDlg::OnBtnColor() {
	int r, g, b;
	float ob;
	r = color[0];
	g = color[1];
	b = color[2];
	if ( DoNewColor( &r, &g, &b, &ob, UpdateLightDialog ) ) {
		color[0] = ob * r;
		color[1] = ob * g;
		color[2] = ob * b;
		ColorButtons();
	}
}

void CLightDlg::OnCancel() {
	CDialog::OnCancel();
}

HBRUSH CLightDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

BOOL CLightDlg::DestroyWindow() 
{
	if (GetSafeHwnd())
	{
		CRect rct;
		GetWindowRect(rct);
		SaveRegistryInfo("Radiant::LightWindow", &rct, sizeof(rct));
	}
	return CDialog::DestroyWindow();
}

void CLightDlg::OnSelchangeComboTexture() 
{
	UpdateData(TRUE);
	int sel = m_wndLights.GetCurSel();
	CString str;
	if (sel >= 0) {
		m_wndLights.GetLBText(sel, str);
		m_drawMaterial->setMedia(str);
		if ( com_editorActive ) {
			m_wndPreview.RedrawWindow();
		}
	}
	Sys_UpdateWindows(W_ALL);
}

void CLightDlg::OnCheckCenter() 
{
	if (reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_CENTER))->GetCheck()) {
		lightInfo.hasCenter = true;
		lightInfo.lightCenter.x = 0;
		lightInfo.lightCenter.y = 0;
		lightInfo.lightCenter.z = 32;
	} else {
		lightInfo.hasCenter = false;
		lightInfo.lightCenter.Zero();
	}
	UpdateDialogFromLightInfo();
	SetSpecifics();
}

void CLightDlg::OnCheckParallel() {
	if ( reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_PARALLEL))->GetCheck() ) {
		lightInfo.hasCenter = true;
		lightInfo.isParallel = true;
		lightInfo.lightCenter.x = 0;
		lightInfo.lightCenter.y = 0;
		lightInfo.lightCenter.z = 32;
	} else {
		lightInfo.isParallel = false;
		lightInfo.hasCenter = false;
	}

	UpdateDialogFromLightInfo();
	SetSpecifics();
}

//jhefty - only apply settings that are different
void CLightDlg::OnApplyDifferences () {
	idDict differences, modified, original;

	UpdateLightInfoFromDialog();

	lightInfo.ToDict( &modified );
	lightInfoOriginal.ToDictWriteAllInfo( &original );

	differences = modified;

	// jhefty - compile a set of modified values to apply
	for ( int i = 0; i < modified.GetNumKeyVals (); i ++ ) {
		const idKeyValue* valModified = modified.GetKeyVal ( i );
		const idKeyValue* valOriginal = original.FindKey ( valModified->GetKey() );

		//if it hasn't changed, remove it from the list of values to apply
		if ( !valOriginal || ( valModified->GetValue() == valOriginal->GetValue() ) ) {
			differences.Delete ( valModified->GetKey() );
		}
	}

	SaveLightInfo( &differences );

	lightInfoOriginal.FromDict( &modified );

	Sys_UpdateWindows( W_ALL );
}

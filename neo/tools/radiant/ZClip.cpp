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

#include "zclip.h"


CZClip::CZClip()
{	
	LONG 
	lSize = sizeof(m_bEnabled);
	if (!LoadRegistryInfo("radiant_ZClipEnabled",	&m_bEnabled, &lSize))
		m_bEnabled = false;

	lSize = sizeof(m_iZClipTop);
	if (!LoadRegistryInfo("radiant_ZClipTop",		&m_iZClipTop, &lSize))
		m_iZClipTop = 64;

	lSize = sizeof(m_iZClipBottom);
	if (!LoadRegistryInfo("radiant_ZClipBottom",	&m_iZClipBottom, &lSize))
		m_iZClipBottom = -64;

	Legalise();
}

CZClip::~CZClip()
{
	// TODO: registry save

	SaveRegistryInfo("radiant_ZClipEnabled", &m_bEnabled,		sizeof(m_bEnabled));
	SaveRegistryInfo("radiant_ZClipTop",     &m_iZClipTop,		sizeof(m_iZClipTop));
	SaveRegistryInfo("radiant_ZClipBottom",  &m_iZClipBottom,	sizeof(m_iZClipBottom));
}

void CZClip::Reset(void)
{
	m_iZClipTop		= 64;		// arb. starting values, but must be at least 64 apart
	m_iZClipBottom	= -64;
	m_bEnabled		= false;

	Legalise();
}


int	CZClip::GetTop(void)
{
	return m_iZClipTop;
}

int CZClip::GetBottom(void)
{
	return m_iZClipBottom;
}

void CZClip::Legalise(void)
{
	// need swapping?
	//
	if (m_iZClipTop < m_iZClipBottom)
	{
		int iTemp = m_iZClipTop;
					m_iZClipTop = m_iZClipBottom;
								  m_iZClipBottom = iTemp;
	}

	// too close together?
	//
#define ZCLIP_MIN_SPACING 64

	if (abs(m_iZClipTop - m_iZClipBottom) < ZCLIP_MIN_SPACING)
		m_iZClipBottom = m_iZClipTop - ZCLIP_MIN_SPACING;
}


void CZClip::SetTop(int iNewZ)
{
	m_iZClipTop = iNewZ;

	Legalise();		
}

void CZClip::SetBottom(int iNewZ)
{
	m_iZClipBottom = iNewZ;

	Legalise();
}

bool CZClip::IsEnabled(void)
{
	return m_bEnabled;
}


bool CZClip::Enable(bool bOnOff)
{
	m_bEnabled = !m_bEnabled;
	return IsEnabled();
}

#define ZCLIP_BAR_THICKNESS 8
#define ZCLIP_ARROWHEIGHT (ZCLIP_BAR_THICKNESS*8)

void CZClip::Paint(void)
{
	float	x, y;
	int	xCam = z.width/4;	// hmmm, a rather unpleasant and obscure global name, but it was already called that so...

	qglColor3f (ZCLIP_COLOUR);//1.0, 0.0, 1.0);

	// draw TOP marker...
	//
	x = 0;
	y = m_iZClipTop;

	if (m_bEnabled)	
		qglBegin(GL_QUADS);
	else
		qglBegin(GL_LINE_LOOP);

	qglVertex3f (x-xCam,y,0);
	qglVertex3f (x-xCam,y+ZCLIP_BAR_THICKNESS,0);
	qglVertex3f (x+xCam,y+ZCLIP_BAR_THICKNESS,0);
	qglVertex3f (x+xCam,y,0);
	qglEnd ();

	qglColor3f (ZCLIP_COLOUR_DIM);//0.8, 0.0, 0.8);

	if (m_bEnabled)
		qglBegin(GL_TRIANGLES);
	else
		qglBegin(GL_LINE_LOOP);	
	qglVertex3f (x,(y+ZCLIP_BAR_THICKNESS),0);
	qglVertex3f (x-xCam,(y+ZCLIP_BAR_THICKNESS)+(ZCLIP_ARROWHEIGHT/2),0);
	qglVertex3f (x+xCam,(y+ZCLIP_BAR_THICKNESS)+(ZCLIP_ARROWHEIGHT/2),0);
	qglEnd ();

	// draw bottom marker...
	//
	qglColor3f (ZCLIP_COLOUR);//1.0, 0.0, 1.0);
	x = 0;
	y = m_iZClipBottom;

	if (m_bEnabled)	
		qglBegin(GL_QUADS);
	else
		qglBegin(GL_LINE_LOOP);
	qglVertex3f (x-xCam,y,0);
	qglVertex3f (x-xCam,y-ZCLIP_BAR_THICKNESS,0);
	qglVertex3f (x+xCam,y-ZCLIP_BAR_THICKNESS,0);
	qglVertex3f (x+xCam,y,0);
	qglEnd ();

	qglColor3f (ZCLIP_COLOUR_DIM);//0.8, 0.0, 0.8);

	if (m_bEnabled)
		qglBegin(GL_TRIANGLES);
	else
		qglBegin(GL_LINE_LOOP);	
	qglVertex3f (x,(y-ZCLIP_BAR_THICKNESS),0);
	qglVertex3f (x-xCam,(y-ZCLIP_BAR_THICKNESS)-(ZCLIP_ARROWHEIGHT/2),0);
	qglVertex3f (x+xCam,(y-ZCLIP_BAR_THICKNESS)-(ZCLIP_ARROWHEIGHT/2),0);
	qglEnd ();
}


///////////////// eof ///////////////////



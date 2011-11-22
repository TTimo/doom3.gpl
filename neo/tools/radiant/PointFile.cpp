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

#define	MAX_POINTFILE	8192
static idVec3	s_pointvecs[MAX_POINTFILE];
static int		s_num_points, s_check_point;

void Pointfile_Delete (void)
{
	char	name[1024];

	strcpy (name, currentmap);
	StripExtension (name);
	strcat (name, ".lin");

	remove(name);
}

// advance camera to next point
void Pointfile_Next (void)
{
	idVec3	dir;

	if (s_check_point >= s_num_points-2)
	{
		Sys_Status ("End of pointfile", 0);
		return;
	}
	s_check_point++;
	VectorCopy (s_pointvecs[s_check_point], g_pParentWnd->GetCamera()->Camera().origin);
	VectorCopy (s_pointvecs[s_check_point], g_pParentWnd->GetXYWnd()->GetOrigin());
	VectorSubtract (s_pointvecs[s_check_point+1], g_pParentWnd->GetCamera()->Camera().origin, dir);
	dir.Normalize();
	g_pParentWnd->GetCamera()->Camera().angles[1] = atan2 (dir[1], dir[0])*180/3.14159;
	g_pParentWnd->GetCamera()->Camera().angles[0] = asin (dir[2])*180/3.14159;

	Sys_UpdateWindows (W_ALL);
}

// advance camera to previous point
void Pointfile_Prev (void)
{
	idVec3	dir;

	if ( s_check_point == 0)
	{
		Sys_Status ("Start of pointfile", 0);
		return;
	}
	s_check_point--;
	VectorCopy (s_pointvecs[s_check_point], g_pParentWnd->GetCamera()->Camera().origin);
	VectorCopy (s_pointvecs[s_check_point], g_pParentWnd->GetXYWnd()->GetOrigin());
	VectorSubtract (s_pointvecs[s_check_point+1], g_pParentWnd->GetCamera()->Camera().origin, dir);
	dir.Normalize();
	g_pParentWnd->GetCamera()->Camera().angles[1] = atan2 (dir[1], dir[0])*180/3.14159;
	g_pParentWnd->GetCamera()->Camera().angles[0] = asin (dir[2])*180/3.14159;

	Sys_UpdateWindows (W_ALL);
}

void WINAPI Pointfile_Check (void)
{
	char	name[1024];
	FILE	*f;
	idVec3	v;

	strcpy (name, currentmap);
	StripExtension (name);
	strcat (name, ".lin");

	f = fopen (name, "r");
	if (!f)
		return;

	common->Printf ("Reading pointfile %s\n", name);

	if (!g_qeglobals.d_pointfile_display_list)
		g_qeglobals.d_pointfile_display_list = qglGenLists(1);

	s_num_points = 0;
  qglNewList (g_qeglobals.d_pointfile_display_list,  GL_COMPILE);
	qglColor3f (1, 0, 0);
	qglDisable(GL_TEXTURE_2D);
	qglDisable(GL_TEXTURE_1D);
	qglLineWidth (2);
	qglBegin(GL_LINE_STRIP);
	do
	{
		if (fscanf (f, "%f %f %f\n", &v[0], &v[1], &v[2]) != 3)
			break;
		if (s_num_points < MAX_POINTFILE)
		{
			VectorCopy (v, s_pointvecs[s_num_points]);
			s_num_points++;
		}
		qglVertex3fv( v.ToFloatPtr() );
	} while (1);
	qglEnd();
	qglLineWidth (0.5);
	qglEndList ();

	s_check_point = 0;
	fclose (f);
	//Pointfile_Next ();
}

void Pointfile_Draw( void )
{
	int i;

	qglColor3f( 1.0F, 0.0F, 0.0F );
	qglDisable(GL_TEXTURE_2D);
	qglDisable(GL_TEXTURE_1D);
	qglLineWidth (2);
	qglBegin(GL_LINE_STRIP);
	for ( i = 0; i < s_num_points; i++ )
	{
		qglVertex3fv( s_pointvecs[i].ToFloatPtr() );
	}
	qglEnd();
	qglLineWidth( 0.5 );
}

void Pointfile_Clear (void)
{
	if (!g_qeglobals.d_pointfile_display_list)
		return;

	qglDeleteLists (g_qeglobals.d_pointfile_display_list, 1);
	g_qeglobals.d_pointfile_display_list = 0;
	Sys_UpdateWindows (W_ALL);
}


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

#include "../../sys/win32/rc/AFEditor_resource.h"

#include "DialogAF.h"
#include "DialogAFConstraint.h"
#include "DialogAFConstraintFixed.h"


// DialogAFConstraintFixed dialog

toolTip_t DialogAFConstraintFixed::toolTips[] = {
	{ 0, NULL }
};

IMPLEMENT_DYNAMIC(DialogAFConstraintFixed, CDialog)

/*
================
DialogAFConstraintFixed::DialogAFConstraintFixed
================
*/
DialogAFConstraintFixed::DialogAFConstraintFixed(CWnd* pParent /*=NULL*/)
	: CDialog(DialogAFConstraintFixed::IDD, pParent)
	, constraint(NULL)
	, file(NULL)
{
	Create( IDD_DIALOG_AF_CONSTRAINT_FIXED, pParent );
	EnableToolTips( TRUE );
}

/*
================
DialogAFConstraintFixed::~DialogAFConstraintFixed
================
*/
DialogAFConstraintFixed::~DialogAFConstraintFixed() {
}

/*
================
DialogAFConstraintFixed::DoDataExchange
================
*/
void DialogAFConstraintFixed::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAFConstraintHinge)
	//}}AFX_DATA_MAP
}

/*
================
DialogAFConstraintFixed::InitJointLists
================
*/
void DialogAFConstraintFixed::InitJointLists( void ) {
}

/*
================
DialogAFConstraintFixed::LoadFile
================
*/
void DialogAFConstraintFixed::LoadFile( idDeclAF *af ) {
	file = af;
	constraint = NULL;
	InitJointLists();
}

/*
================
DialogAFConstraintFixed::SaveFile
================
*/
void DialogAFConstraintFixed::SaveFile( void ) {
	SaveConstraint();
}

/*
================
DialogAFConstraintFixed::LoadConstraint
================
*/
void DialogAFConstraintFixed::LoadConstraint( idDeclAF_Constraint *c ) {

	constraint = c;

	// update displayed values
	UpdateData( FALSE );
}

/*
================
DialogAFConstraintFixed::SaveConstraint
================
*/
void DialogAFConstraintFixed::SaveConstraint( void ) {

	if ( !file || !constraint ) {
		return;
	}
	UpdateData( TRUE );

	AFDialogSetFileModified();
}

/*
================
DialogAFConstraintFixed::UpdateFile
================
*/
void DialogAFConstraintFixed::UpdateFile( void ) {
	SaveConstraint();
	if ( file ) {
		gameEdit->AF_UpdateEntities( file->GetName() );
	}
}

/*
================
DialogAFConstraintFixed::OnToolHitTest
================
*/
int DialogAFConstraintFixed::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CDialog::OnToolHitTest( point, pTI );
	return DefaultOnToolHitTest( toolTips, this, point, pTI );
}


BEGIN_MESSAGE_MAP(DialogAFConstraintFixed, CDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()


// DialogAFConstraintFixed message handlers

BOOL DialogAFConstraintFixed::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	return DefaultOnToolTipNotify( toolTips, id, pNMHDR, pResult );
}

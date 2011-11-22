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

#include "GEModifierGroup.h"

rvGEModifierGroup::rvGEModifierGroup ( ) :
	rvGEModifier ( "Group", NULL )
{	
}

rvGEModifierGroup::~rvGEModifierGroup ( )
{
	int i;
	
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		delete mModifiers[i];
	}
	
	mModifiers.Clear ( );
}

bool rvGEModifierGroup::Append ( rvGEModifier* mod )
{
	// All modifiers must be the same type
	assert ( !mModifiers.Num() || !idStr::Icmp ( mod->GetName ( ), mModifiers[0]->GetName ( ) ) );

	if ( !mModifiers.Num ( ) )
	{
		mName = mod->GetName ( );
	}

	mModifiers.Append ( mod );
	return true;
}

bool rvGEModifierGroup::IsValid ( void )
{
	int i;
	
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		if ( !mModifiers[i]->IsValid ( ) )
		{
			return false;
		}
	}
	
	return true;
}

bool rvGEModifierGroup::Apply ( void )
{
	int i;
	
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		mModifiers[i]->Apply ( );
	}
	
	return true;
}

bool rvGEModifierGroup::Undo ( void )
{
	int i;
	
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		mModifiers[i]->Undo ( );
	}
	
	return true;
}

bool rvGEModifierGroup::CanMerge ( rvGEModifier* mergebase )
{
	rvGEModifierGroup*	merge = (rvGEModifierGroup*) mergebase;
	int					i;
			
	if ( mModifiers.Num() != merge->mModifiers.Num ( ) )
	{
		return false;
	}
			
	// Double check the merge is possible
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		if ( mModifiers[i]->GetWindow() != merge->mModifiers[i]->GetWindow() )
		{
			return false;
		}
		
		if ( idStr::Icmp ( mModifiers[i]->GetName ( ), merge->mModifiers[i]->GetName ( ) ) )
		{
			return false;
		}

		if ( !mModifiers[i]->CanMerge ( merge->mModifiers[i] ) )
		{
			return false;
		}
	}

	return true;
}

bool rvGEModifierGroup::Merge ( rvGEModifier* mergebase )
{
	rvGEModifierGroup*	merge = (rvGEModifierGroup*) mergebase;
	int					i;
	
	// Double check the merge is possible
	for ( i = 0; i < mModifiers.Num(); i ++ )
	{
		mModifiers[i]->Merge ( merge->mModifiers[i] );
	}
	
	return true;
}

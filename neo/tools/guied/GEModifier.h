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
#ifndef GEMODIFIER_H_
#define GEMODIFIER_H_

class idWindow;
class rvGEWindowWrapper;

class rvGEModifier
{
public:

	rvGEModifier ( const char* name, idWindow* window );
	virtual ~rvGEModifier ( ) { }

	virtual bool		Apply		( void ) = 0;
	virtual bool		Undo		( void ) = 0;	
	virtual const char*	GetName		( void );
	virtual bool		CanMerge	( rvGEModifier* merge );
	
	virtual bool		IsValid		( void );
	
	virtual bool		Merge		( rvGEModifier* merge );
	
	idWindow*			GetWindow	( void );
	
	
protected:
	
	idWindow*			mWindow;
	rvGEWindowWrapper*	mWrapper;
	idStr				mName;
};

ID_INLINE bool rvGEModifier::IsValid ( void )
{
	return true;
}

ID_INLINE idWindow* rvGEModifier::GetWindow ( void )
{
	return mWindow;
}

ID_INLINE const char* rvGEModifier::GetName ( void )
{
	return mName;
}

ID_INLINE bool rvGEModifier::CanMerge ( rvGEModifier* merge )
{
	return false;
}

ID_INLINE bool rvGEModifier::Merge ( rvGEModifier* merge )
{
	return false;
}

#endif // GEMODIFIER_H_

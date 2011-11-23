/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").  

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
#ifndef GEKEYVALUEMODIFIER_H_
#define GEKEYVALUEMODIFIER_H_

#ifndef GEMODIFIER_H_
#include "GEModifier.h"
#endif

class rvGEKeyValueModifier : public rvGEModifier
{
public:

	rvGEKeyValueModifier ( const char* name, idWindow* window, const char* key, const char* value );
	
	virtual bool		Apply		( void );
	virtual bool		Undo		( void );

	virtual bool		CanMerge	( rvGEModifier* merge );
	virtual bool		Merge		( rvGEModifier* merge );
			
protected:
	
	idStr		mKey;
	idStr		mValue;
	idStr		mUndoValue;
}; 

ID_INLINE bool rvGEKeyValueModifier::CanMerge ( rvGEModifier* merge )
{
	return !((rvGEKeyValueModifier*)merge)->mKey.Icmp ( mKey );
}

#endif // GEKEYVALUEMODIFIER_H_
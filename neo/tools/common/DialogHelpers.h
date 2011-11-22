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

#ifndef DIALOGHELPERS_H_
#define DIALOGHELPERS_H_

class rvDialogItem
{
public:

	HWND	mWindow;
	int		mID;
	
	rvDialogItem ( int id ) { mID = id; }
	
	void Cache ( HWND parent )
	{
		mWindow = GetDlgItem ( parent, mID );
	}
	
	void Check ( bool checked )
	{
		SendMessage ( mWindow, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0 );
	}
	
	void Enable ( bool enable )
	{
		EnableWindow ( mWindow, enable );
	}
	
	bool IsChecked ( void )
	{
		return SendMessage ( mWindow, BM_GETCHECK, 0, 0 ) == BST_CHECKED ? true : false;
	}
	
	void SetText ( const char* text )
	{
		SetWindowText ( mWindow, text );
	}
	
	void GetText ( idStr& out )
	{
		char text[4096];
		GetWindowText ( mWindow, text, 4095 );
		out = text;
	}
	
	float GetFloat ( void )
	{
		idStr text;
		GetText ( text );
		return atof( text );
	}
	
	void SetFloat ( float f )
	{
		SetText ( va("%g", f ) );
	}
	
	operator HWND( void ) const { return mWindow; }
};

class rvDialogItemContainer
{
protected:
	
	void Cache ( HWND parent, int count )
	{
		int				i;
		unsigned char*	ptr;
		
		ptr = (unsigned char*)this;
		for ( i = 0; i < count; i ++, ptr += sizeof(rvDialogItem) )
		{
			((rvDialogItem*)ptr)->Cache ( parent );			
		}
	}
};

#define DIALOGITEM_BEGIN(name)									\
class name : public rvDialogItemContainer						\
{																\
public:															\
	name ( void ) { }											\
	name ( HWND hwnd ) { Cache ( hwnd ); }						\
	void Cache ( HWND parent )									\
	{															\
		rvDialogItemContainer::Cache ( parent, sizeof(*this)/sizeof(rvDialogItem) );	\
	}


#define DIALOGITEM(id,name)										\
class c##name : public rvDialogItem								\
{																\
public:															\
	c##name(int localid=id) : rvDialogItem ( localid ) { }		\
} name;															

#define DIALOGITEM_END()										\
};


#endif // DIALOGHELPERS_H_

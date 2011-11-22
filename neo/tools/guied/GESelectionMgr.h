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

#ifndef GESELECTIONMGR_H_
#define GESELECTIONMGR_H_

class rvGEWorkspace;

class rvGESelectionMgr
{
public:

	enum EHitTest
	{
		HT_NONE,
		HT_SELECT,
		HT_MOVE,
		HT_SIZE_TOPLEFT,
		HT_SIZE_TOP,
		HT_SIZE_TOPRIGHT,
		HT_SIZE_RIGHT,
		HT_SIZE_BOTTOMRIGHT,
		HT_SIZE_BOTTOM,
		HT_SIZE_BOTTOMLEFT,
		HT_SIZE_LEFT
	};

	rvGESelectionMgr ( );

	void			SetWorkspace		( rvGEWorkspace* workspace );

	void			Set					( idWindow* );
	void			Add					( idWindow* window, bool expand = true );
	void			Remove				( idWindow* );
	void			Clear				( void );
	
	int				Num					( void );
	
	void			Render				( void );
	
	EHitTest		HitTest				( float x, float y );
	
	bool			IsSelected			( idWindow* window );
	bool			IsExpression		( void );
	
	idRectangle&	GetRect				( void );
	idWindow*		GetBottomMost		( void );

	idWindow*&		operator[]( int index );

protected:

	void		UpdateRectangle		( void );
	void		UpdateExpression	( void );

	idList<idWindow*>	mSelections;
	idRectangle			mRect;
	rvGEWorkspace*		mWorkspace;
	bool				mExpression;
};

ID_INLINE int rvGESelectionMgr::Num ( void )
{
	return mSelections.Num ( );
}

ID_INLINE idWindow*& rvGESelectionMgr::operator[]( int index ) 
{
	assert( index >= 0 );
	assert( index < mSelections.Num() );

	return mSelections[ index ];
}

ID_INLINE void rvGESelectionMgr::SetWorkspace ( rvGEWorkspace* workspace )
{
	mWorkspace = workspace;
}

ID_INLINE idRectangle& rvGESelectionMgr::GetRect ( void )
{
	UpdateRectangle ( );
	return mRect;
}

ID_INLINE bool rvGESelectionMgr::IsSelected ( idWindow* window )
{
	return mSelections.FindIndex ( window ) != -1 ? true : false;
}

ID_INLINE bool rvGESelectionMgr::IsExpression ( void )
{
	return mExpression;
}

#endif // GESELECTIONMGR_H_

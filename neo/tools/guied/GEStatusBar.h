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
#ifndef GESTATUSBAR_H_
#define GESTATUSBAR_H_

class rvGEStatusBar
{
public:

	rvGEStatusBar ( );

	bool	Create			( HWND parent, UINT id, bool visible = true );
	void	Resize			( int width, int height );	
	
	HWND	GetWindow		( void );

	void	SetZoom			( int zoom );
	void	SetTriangles	( int tris );
	void	SetSimple		( bool simple );
	
	void	Show			( bool state );
	void	Update			( void );
		
protected:

	HWND	mWnd;
	bool	mSimple;
	int		mZoom;
	int		mTriangles;
};

ID_INLINE HWND rvGEStatusBar::GetWindow ( void )
{
	return mWnd;
}

ID_INLINE void rvGEStatusBar::SetZoom ( int zoom )
{
	if ( mZoom != zoom )
	{
		mZoom = zoom;
		Update ( );
	}
}

ID_INLINE void rvGEStatusBar::SetTriangles ( int triangles )
{
	if ( triangles != mTriangles )
	{
		mTriangles = triangles;
		Update ( );
	}
}

ID_INLINE void rvGEStatusBar::SetSimple ( bool simple )
{
	if ( mSimple != simple )
	{
		mSimple = simple;
		Update ( );
	}
}

#endif // GESTATUSBAR_H_
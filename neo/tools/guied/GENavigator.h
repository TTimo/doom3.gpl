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
#ifndef GENAVIGATOR_H_
#define GENAVIGATOR_H_

class rvGEWorkspace;
class idWindow;

class rvGENavigator
{
public:

	rvGENavigator ( );
	
	bool	Create				( HWND parent, bool visible );
	void	Show				( bool visibile );

	void	Refresh				( void );

	void	SetWorkspace		( rvGEWorkspace* workspace );
	
	void	Update				( void );	
	void	UpdateSelections	( void );
	
	HWND	GetWindow			( void );

protected:

	void	AddWindow			( idWindow* window );

	static LRESULT CALLBACK WndProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK ListWndProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	static LRESULT FAR PASCAL GetMsgProc ( int nCode, WPARAM wParam, LPARAM lParam );

	HWND			mWnd;
	HWND			mTree;
	HICON			mVisibleIcon;
	HICON			mVisibleIconDisabled;
	HICON			mScriptsIcon;
	HICON			mScriptsLightIcon;
	HICON			mCollapseIcon;
	HICON			mExpandIcon;
	rvGEWorkspace*	mWorkspace;
	WNDPROC			mListWndProc;
};

ID_INLINE HWND rvGENavigator::GetWindow ( void )
{
	return mWnd;
}

#endif // GENAVIGATOR_H_
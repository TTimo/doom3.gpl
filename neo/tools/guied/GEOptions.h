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

#ifndef GEOPTIONS_H_
#define GEOPTIONS_H_

#ifndef REGISTRYOPTIONS_H_
#include "../common/registryoptions.h"
#endif

class idRectangle;

class rvGEOptions
{
public:

	static const int MAX_MRU_SIZE = rvRegistryOptions::MAX_MRU_SIZE;

	rvGEOptions();

	void			Init( void );

	// Write the options to the registery
	bool			Save				( void );
	
	// Read the options from the registry
	bool			Load					( void );
	
	void			SetSelectionColor		( idVec4& color );
	void			SetSelectionColor		( COLORREF color );
	void			SetGridColor			( idVec4& color );
	void			SetGridColor			( COLORREF color );
	void			SetGridWidth			( int width );
	void			SetGridHeight			( int height );
	void			SetGridVisible			( bool vis );
	void			SetGridSnap				( bool snap );
	void			SetLastOptionsPage		( int page );
	void			SetNavigatorVisible		( bool vis );
	void			SetPropertiesVisible	( bool vis );
	void			SetTransformerVisible	( bool vis );
	void			SetIgnoreDesktopSelect	( bool ignore );
	void			SetStatusBarVisible		( bool vis );

	void			AddRecentFile			( const char* filename );
	int				GetRecentFileCount		( void );
	const char*		GetRecentFile			( int index );
	
	idVec4&			GetGridColor			( void );
	int				GetGridWidth			( void );
	int				GetGridHeight			( void );
	bool			GetGridVisible			( void );	
	bool			GetGridSnap				( void );
	int				GetLastOptionsPage		( void );
	idVec4&			GetWorkspaceColor		( void );
	bool			GetNavigatorVisible		( void );
	bool			GetTransformerVisible	( void );
	bool			GetPropertiesVisible	( void );
	idVec4&			GetSelectionColor		( void );
	COLORREF*		GetCustomColors			( void );
	bool			GetIgnoreDesktopSelect	( void );
	bool			GetStatusBarVisible		( void );

	void			SetWindowPlacement		( const char* name, HWND hwnd );
	bool			GetWindowPlacement		( const char* name, HWND hwnd );

	void			SnapRectToGrid			( idRectangle& rect, bool snapLeft = true, bool snapTop = true, bool snapWidth = true, bool snapHeight = true );
	
protected:

	void				ConvertColor		( COLORREF src, idVec4& dest );
	void				SetModified			( bool mod );

	bool				mModified;
	int					mLastOptionsPage;

	idVec4				mGridColor;
	int					mGridWidth;
	int					mGridHeight;
	bool				mGridSnap;
	bool				mGridVisible;
	
	idVec4				mWorkspaceColor;
	idVec4				mSelectionColor;
	
	bool				mNavigatorVisible;
	bool				mPropertiesVisible;
	bool				mTransformerVisible;
	bool				mStatusBarVisible;
	bool				mIgnoreDesktopSelect;
	
	idList<idStr>		mRecentFiles;
	
	COLORREF			mCustomColors[16];	
	
	rvRegistryOptions	mRegistry;
};

ID_INLINE void rvGEOptions::SetModified ( bool mod )
{
	mModified = mod;
}

ID_INLINE void rvGEOptions::ConvertColor ( COLORREF src, idVec4& dest )
{
	dest[0] = (float)GetRValue ( src ) / 255.0f;
	dest[1] = (float)GetGValue ( src ) / 255.0f;
	dest[2] = (float)GetBValue ( src ) / 255.0f;
	dest[3] = 1.0f;
}

ID_INLINE void rvGEOptions::SetGridWidth ( int width )
{
	mGridWidth = width;
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetGridHeight ( int height )
{
	mGridHeight = height;
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetGridSnap ( bool snap )
{
	mGridSnap = snap;
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetGridVisible ( bool vis )
{
	mGridVisible = vis;
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetStatusBarVisible ( bool vis )
{
	mStatusBarVisible = vis;
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetGridColor ( COLORREF color )
{
	ConvertColor ( color, mGridColor );
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetSelectionColor ( idVec4& color )
{
	VectorCopy ( color, mSelectionColor );
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetSelectionColor ( COLORREF color )
{
	ConvertColor ( color, mSelectionColor );
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetGridColor ( idVec4& color )
{
	VectorCopy ( color, mGridColor );
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetNavigatorVisible ( bool vis )
{
	mNavigatorVisible = vis;
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetPropertiesVisible ( bool vis )
{
	mPropertiesVisible = vis;
	SetModified ( true );
}

ID_INLINE void rvGEOptions::SetTransformerVisible ( bool vis )
{
	mTransformerVisible = vis;
	SetModified ( true );
}

ID_INLINE idVec4& rvGEOptions::GetGridColor ( void )
{
	return mGridColor;
}

ID_INLINE int rvGEOptions::GetGridWidth ( void )
{
	return mGridWidth;
}

ID_INLINE int rvGEOptions::GetGridHeight ( void )
{
	return mGridHeight;
}

ID_INLINE bool rvGEOptions::GetGridVisible ( void )
{
	return mGridVisible;
}

ID_INLINE bool rvGEOptions::GetGridSnap ( void )
{
	return mGridSnap;
}

ID_INLINE idVec4& rvGEOptions::GetWorkspaceColor ( void )
{
	return mWorkspaceColor;
}

ID_INLINE int rvGEOptions::GetLastOptionsPage ( void )
{
	return mLastOptionsPage;
}

ID_INLINE void rvGEOptions::SetLastOptionsPage ( int page )
{
	mLastOptionsPage = page;
}

ID_INLINE bool rvGEOptions::GetNavigatorVisible ( void )
{
	return mNavigatorVisible;
}

ID_INLINE bool rvGEOptions::GetPropertiesVisible ( void )
{
	return mPropertiesVisible;
}

ID_INLINE bool rvGEOptions::GetTransformerVisible ( void )
{
	return mTransformerVisible;
}

ID_INLINE bool rvGEOptions::GetStatusBarVisible ( void )
{
	return mStatusBarVisible;
}

ID_INLINE idVec4& rvGEOptions::GetSelectionColor ( void )
{
	return mSelectionColor;
}

ID_INLINE COLORREF* rvGEOptions::GetCustomColors ( void )
{
	return mCustomColors;
}

ID_INLINE void rvGEOptions::SetIgnoreDesktopSelect ( bool ignore )
{
	mIgnoreDesktopSelect = ignore;
}

ID_INLINE bool rvGEOptions::GetIgnoreDesktopSelect ( void )
{
	return mIgnoreDesktopSelect;
}

ID_INLINE void rvGEOptions::SetWindowPlacement ( const char* name, HWND hwnd )
{
	mRegistry.SetWindowPlacement ( name, hwnd );
}

ID_INLINE bool rvGEOptions::GetWindowPlacement ( const char* name, HWND hwnd )
{
	return mRegistry.GetWindowPlacement ( name, hwnd );
}

ID_INLINE void rvGEOptions::AddRecentFile ( const char* filename )
{
	mRegistry.AddRecentFile ( filename );
}

ID_INLINE int rvGEOptions::GetRecentFileCount ( void )
{
	return mRegistry.GetRecentFileCount ( );
}

ID_INLINE const char* rvGEOptions::GetRecentFile ( int index )
{
	return mRegistry.GetRecentFile ( index );
}

#endif // _GEOPTIONS_H_


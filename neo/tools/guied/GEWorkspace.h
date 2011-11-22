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

#ifndef _GEWORKSPACE_H_
#define _GEWORKSPACE_H_

#ifndef GESELECTIONMGR_H_
#include "GESelectionMgr.h"
#endif // GESELECTIONMGR_H_

#ifndef GEMODIFIERSTACK_H_
#include "GEModifierStack.h"
#endif // GEMODIFIERSTACK_H_

class rvGEApp;

class rvGEClipboardItem
{
public:

	idDict	mStateDict;
	idDict	mScriptDict;
	idDict	mVarDict;
};

class rvGEWorkspace
{
public:

	enum ESourceControlState
	{
		SCS_NONE,
		SCS_CHECKEDOUT,
		SCS_CHECKEDIN,
	};

	enum EZoomLevel
	{
		ZOOM_MIN,
		ZOOM_25,
		ZOOM_33,
		ZOOM_50,
		ZOOM_66,
		ZOOM_100,
		ZOOM_150,
		ZOOM_200,
		ZOOM_300,
		ZOOM_MAX
	};
	
	enum EItemAlign
	{
		ALIGN_LEFTS,
		ALIGN_CENTERS,
		ALIGN_RIGHTS,
		ALIGN_TOPS,
		ALIGN_MIDDLES,
		ALIGN_BOTTOMS,
	};

	rvGEWorkspace ( rvGEApp* app );
	~rvGEWorkspace ( );
	
	// Attach the workspace to a win32 window
	bool					Attach					( HWND wnd );
	
	// Detach the workspace from the current win32 window
	void					Detach					( void );
		
	bool					NewFile					( void );		
	bool					LoadFile				( const char* filename, idStr* error = NULL );
	bool					SaveFile				( const char* filename );
	const char*				GetFilename				( void );
	
	// Source control methods
	bool					CheckOut				( void );
	bool					CheckIn					( void );	
	bool					UndoCheckout			( void );
	ESourceControlState		GetSourceControlState	( void );	
	
	void					Render					( HDC hDC );

	rvGEApp*				GetApplication			( void );

	void					HideSelected			( void );
	void					UnhideSelected			( void );
	void					DeleteSelected			( void );
	bool					EditSelectedProperties	( void );
	bool					EditSelectedScripts		( void );
	void					BringSelectedForward	( void );
	void					BringSelectedToFront	( void );
	void					SendSelectedBackward	( void );
	void					SendSelectedToBack		( void );
	void					AlignSelected			( EItemAlign align );
	void					MakeSelectedSameSize	( bool width, bool height );
	void					MakeSelectedAChild		( void );

	bool					CanHide					( void );
	void					ShowHidden				( void );
	void					HideWindow				( idWindow* window );
	void					UnhideWindow			( idWindow* window );

	EZoomLevel				ZoomIn					( void );	
	EZoomLevel				ZoomOut					( void );	
	EZoomLevel				GetZoom					( void );
	float					GetZoomScale			( void );
	
	static rvGEWorkspace*		GetWorkspace			( HWND wnd );	
	rvGEModifierStack&			GetModifierStack		( void );
	idUserInterfaceLocal*		GetInterface			( void );
	rvGESelectionMgr&			GetSelectionMgr			( void );
	idList<rvGEClipboardItem*>	GetClipboard			( void );
	HWND						GetWindow				( void );

	void					HandleMessage			( UINT msg, WPARAM wParam, LPARAM lParam );
	
	idVec2&					WindowToWorkspace		( idVec2& point );
	idVec2&					WorkspaceToWindow		( idVec2& point );
	idRectangle&			WindowToWorkspace		( idRectangle& rect );
	idRectangle&			WorkspaceToWindow		( idRectangle& rect );
	
	bool					IsModified				( void );
	bool					IsNew					( void );

	idWindow*				AddWindow				( rvGEWindowWrapper::EWindowType type );
	
//	void					Cut						( void );
	void					Copy					( void );
	void					Paste					( void );

	void					AddModifierMove			( const char* modName, float x, float y, bool snap );
	void					AddModifierSize			( const char* modName, float l, float t, float r, float b, bool snap );

	void					SetModified				( bool mod );

protected:

	enum EModifierType
	{
		MOD_UNKNOWN,
		MOD_DELETE,
		MOD_HIDE,
		MOD_UNHIDE,
		MOD_SHOWHIDDEN,
		MOD_SEND_BACKWARD,
		MOD_BRING_FORWARD,
		MOD_SEND_BACK,
		MOD_BRING_FRONT,
	};

	bool					SetupPixelFormat	( void );
	void					UpdateSelections	( void );

	// Additional rendering routines
	void					RenderGrid			( void );

	// File related methods
	void					WriteTabs			( idFile* file, int depth );
	bool					WriteWindow			( idFile* file, int depth, idWindow* window );

	// Message handlers
	int						HandleRButtonDown	( WPARAM wParam, LPARAM lParam );
	int						HandleLButtonDown	( WPARAM wParam, LPARAM lParam );
	int						HandleLButtonUp		( WPARAM wParam, LPARAM lParam );
	int						HandleLButtonDblClk	( WPARAM wParam, LPARAM lParam );
	int						HandleMButtonDown	( WPARAM wParam, LPARAM lParam );
	int						HandleMButtonUp		( WPARAM wParam, LPARAM lParam );
	int						HandleMouseMove		( WPARAM wParam, LPARAM lParam );
	int						HandleKeyDown		( WPARAM wParam, LPARAM lParam );
	int						HandleScroll		( int scrollbar, WPARAM wParam, LPARAM lParam );
	int						HandleCommand		( WPARAM wParam, LPARAM lParam );

	// General protected functions
	void					UpdateScrollbars	( void );
	void					UpdateRectangle		( bool useScroll = true );
	void					UpdateCursor		( void );
	void					UpdateCursor		( float x, float y );
	void					UpdateCursor		( rvGESelectionMgr::EHitTest type );
	void					UpdateTitle			( void );
	idWindow*				NewWindow			( idDict* state, rvGEWindowWrapper::EWindowType type );
	void					Scroll				( int scrollbar, int offset );

	// Modifier methods
	void					AddModifierMoveNudge( float x, float y, bool snap );
	void					AddModifierSizeNudge( float w, float h, bool snap );
	void					AddModifierShowAll	( void );
	
	void					AddModifiers		( EModifierType type, ... );
	void					AddModifiers		( idWindow* window, EModifierType type, ... );
	rvGEModifier*			CreateModifier		( EModifierType type, idWindow* window, va_list args );

	idUserInterfaceLocal*		mInterface;
	HWND						mWnd;	
	
	int							mZoom;
	idRectangle					mRect;
	
	bool						mScrollHorz;
	bool						mScrollVert;
	
	int							mWindowWidth;
	int							mWindowHeight;
	
	idStr						mFilename;
		
	rvGEModifierStack			mModifiers;
	rvGESelectionMgr			mSelections;
	
	rvGESelectionMgr::EHitTest	mDragType;
	idVec2						mDragPoint;
	int							mDragTime;
	bool						mDragX;
	bool						mDragY;
	bool						mDragScroll;
	
	rvGEApp*					mApplication;
	
	static idList<rvGEClipboardItem*>	mClipboard;
	idList<idWindow*>					mSelectMenu;
	idVec2								mSelectMenuPos;

private:

	static bool				CleanupEnumProc			( rvGEWindowWrapper* wrapper, void* data );
	static bool				ShowAllEnumProc			( rvGEWindowWrapper* wrapper, void* data );
	static bool				BuildSelectMenuEnumProc	( rvGEWindowWrapper* wrapper, void* data );

	// States
	bool					mModified;
	bool					mNew;
	bool					mDontAdd;
	ESourceControlState		mSourceControlState;
	
	// Resources
	HCURSOR					mHandCursor;
};

ID_INLINE rvGEWorkspace::EZoomLevel rvGEWorkspace::GetZoom ( void )
{
	return (EZoomLevel)mZoom;
}

ID_INLINE rvGEWorkspace* rvGEWorkspace::GetWorkspace ( HWND wnd )
{
	return (rvGEWorkspace*) GetWindowLong ( wnd, GWL_USERDATA );
}

ID_INLINE const char* rvGEWorkspace::GetFilename ( void )
{
	return mFilename;
}

ID_INLINE bool rvGEWorkspace::IsModified ( void )
{
	return mModified;
}

ID_INLINE bool rvGEWorkspace::IsNew ( void )
{
	return mNew;
}

ID_INLINE rvGEModifierStack& rvGEWorkspace::GetModifierStack ( void )
{
	return mModifiers;
}

ID_INLINE rvGESelectionMgr& rvGEWorkspace::GetSelectionMgr ( void )
{
	return mSelections;
}

ID_INLINE void rvGEWorkspace::ShowHidden ( void )
{
	AddModifierShowAll ( );
}

ID_INLINE void rvGEWorkspace::AddModifierMoveNudge ( float x, float y, bool snap )
{
	AddModifierMove ( "Nudge Move", x, y, snap );
}

ID_INLINE void rvGEWorkspace::AddModifierSizeNudge ( float w, float h, bool snap )
{
	AddModifierSize ( "Nudge Size", 0, 0, w, h, snap );
}

ID_INLINE idUserInterfaceLocal* rvGEWorkspace::GetInterface ( void )
{
	return mInterface;
}

ID_INLINE rvGEApp* rvGEWorkspace::GetApplication ( void )
{
	return mApplication;
}

ID_INLINE HWND rvGEWorkspace::GetWindow ( void )
{
	return mWnd;
}

ID_INLINE idList<rvGEClipboardItem*> rvGEWorkspace::GetClipboard ( void )
{
	return mClipboard;
}

ID_INLINE rvGEWorkspace::ESourceControlState rvGEWorkspace::GetSourceControlState ( void )
{
	return mSourceControlState;
}

#endif // _GEWORKSPACE_H_

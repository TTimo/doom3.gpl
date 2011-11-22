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
#ifndef OPENFILEDIALOG_H_
#define OPENFILEDIALOG_H_

#define OFD_MUSTEXIST	0x00000001

class rvOpenFileDialog
{
public:

	rvOpenFileDialog ( void );
	~rvOpenFileDialog ( void );

	bool			DoModal		( HWND parent );
	const char*		GetFilename	( void );

	void			SetFilter		( const char* filter );
	void			SetTitle		( const char* title );
	void			SetOKTitle		( const char* title );
	void			SetInitialPath	( const char* path );
	void			SetFlags		( int flags );

	const char*		GetInitialPath  ( void );

protected:

	void			UpdateFileList	( void );
	void			UpdateLookIn	( void );

	HWND			mWnd;
	HWND			mWndFileList;
	HWND			mWndLookin;
	
	HINSTANCE		mInstance;
	
	HIMAGELIST		mImageList;
	HBITMAP			mBackBitmap;
	
	static char		mLookin[ MAX_OSPATH ];
	idStr			mFilename;
	idStr			mTitle;
	idStr			mOKTitle;
	idStrList		mFilters;
	
	int				mFlags;

private:
	
	void	HandleCommandOK			( void );
	void	HandleLookInChange		( void );
	void	HandleInitDialog		( void );

	static INT_PTR CALLBACK DlgProc ( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
};

ID_INLINE const char* rvOpenFileDialog::GetFilename ( void )
{
	return mFilename.c_str ( );
}

ID_INLINE void rvOpenFileDialog::SetTitle ( const char* title )
{
	mTitle = title;
}

ID_INLINE void rvOpenFileDialog::SetOKTitle ( const char* title )
{
	mOKTitle = title;
}

ID_INLINE void rvOpenFileDialog::SetInitialPath ( const char* path )
{
	if ( !idStr::Cmpn( mLookin, path, strlen( path ) ) )
	{
		return;
	}

	idStr::Copynz( mLookin, path, sizeof( mLookin ) );
}

ID_INLINE void rvOpenFileDialog::SetFlags ( int flags )
{
	mFlags = flags;
}

ID_INLINE const char* rvOpenFileDialog::GetInitialPath ( void )
{
	return mLookin;
}

#endif // OPENFILEDIALOG_H_

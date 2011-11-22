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

#ifndef REGISTRYOPTIONS_H_
#define REGISTRYOPTIONS_H_

class rvRegistryOptions
{
public:

	static const int MAX_MRU_SIZE = 4;

	rvRegistryOptions();

	void			Init( const char *key );

	// Write the options to the registery
	bool			Save				( void );
	
	// Read the options from the registry
	bool			Load				( void );

	// Window placement routines
	void			SetWindowPlacement		( const char* name, HWND hwnd );
	bool			GetWindowPlacement		( const char* name, HWND hwnd );

	// List view column sizes
	void			SetColumnWidths			( const char* name, HWND list );
	void			GetColumnWidths			( const char* name, HWND list );

	// Set routines
	void			SetFloat				( const char* name, float v );
	void			SetLong					( const char* name, long v );
	void			SetBool					( const char* name, bool v );
	void			SetString				( const char* name, const char* v );
	void			SetVec4					( const char* name, idVec4& v );
	void			SetBinary				( const char* name, const unsigned char* data, int size );

	// Get routines
	float			GetFloat				( const char* name );
	long			GetLong					( const char* name );
	bool			GetBool					( const char* name );
	const char*		GetString				( const char* name );
	idVec4			GetVec4					( const char* name );
	void			GetBinary				( const char* name, unsigned char* data, int size );

	// MRU related methods
	void			AddRecentFile			( const char* filename );
	const char*		GetRecentFile			( int index );	
	int				GetRecentFileCount		( void );

private:

	idList<idStr>	mRecentFiles;
	idDict			mValues;
	idStr			mBaseKey;
};

ID_INLINE void rvRegistryOptions::SetFloat ( const char* name, float v )
{
	mValues.SetFloat ( name, v );
}

ID_INLINE void rvRegistryOptions::SetLong ( const char* name, long v )
{
	mValues.SetInt ( name, v );
}

ID_INLINE void rvRegistryOptions::SetBool ( const char* name, bool v )
{
	mValues.SetBool ( name, v );
}

ID_INLINE void rvRegistryOptions::SetString ( const char* name, const char* v )
{
	mValues.Set ( name, v );
}

ID_INLINE void rvRegistryOptions::SetVec4 ( const char* name, idVec4& v )
{
	mValues.SetVec4 ( name, v );
}

ID_INLINE float rvRegistryOptions::GetFloat ( const char* name )
{
	return mValues.GetFloat ( name );
}

ID_INLINE long rvRegistryOptions::GetLong ( const char* name )
{
	return mValues.GetInt ( name );
}

ID_INLINE bool rvRegistryOptions::GetBool ( const char* name )
{
	return mValues.GetBool ( name );
}

ID_INLINE const char* rvRegistryOptions::GetString ( const char* name )
{
	return mValues.GetString ( name );
}

ID_INLINE idVec4 rvRegistryOptions::GetVec4 ( const char* name )
{
	return mValues.GetVec4 ( name );
}

ID_INLINE int rvRegistryOptions::GetRecentFileCount ( void )
{
	return mRecentFiles.Num ( );
}

ID_INLINE const char* rvRegistryOptions::GetRecentFile ( int index )
{
	return mRecentFiles[index].c_str ( );
}

#endif // REGISTRYOPTIONS_H_

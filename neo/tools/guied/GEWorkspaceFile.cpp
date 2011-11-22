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

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "GEApp.h"

bool GECheckInDlg_DoModal ( HWND parent, const char* filename, idStr* comment );

/*
================
rvGEWorkspace::SaveFile

Writes the contents of the open gui file to disk
================
*/
bool rvGEWorkspace::SaveFile ( const char* filename )
{
	idFile*		file;
	idWindow*	window;

	SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_WAIT ) ) );
	
	mFilename = filename;
	
	// Since quake can only write to its path we will write a temp file then copy it over	
	idStr tempfile;
	idStr ospath;
	
	tempfile = "guis/temp.guied";
	ospath = fileSystem->RelativePathToOSPath ( tempfile, "fs_basepath" );
	
	// Open the output file for write
	if ( !(file = fileSystem->OpenFileWrite ( tempfile ) ) )
	{
		int error = GetLastError ( );
		SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_ARROW ) ) );
		return false;
	}
	
	window = mInterface->GetDesktop ( );
	
	WriteWindow ( file, 1, window );
	
	fileSystem->CloseFile ( file );
	
	if ( !CopyFile ( ospath, filename, FALSE ) )
	{
		DeleteFile ( ospath );
		SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_ARROW ) ) );
		return false;
	}
	
	DeleteFile ( ospath );

	mFilename = filename;
	mModified = false;
	mNew      = false;
	UpdateTitle ( );	

	SetCursor ( LoadCursor ( NULL, MAKEINTRESOURCE(IDC_ARROW ) ) );
	
	return true;
}

/*
================
rvGEWorkspace::WriteTabs

Writes the given number of tabs to the given file
================
*/
void rvGEWorkspace::WriteTabs ( idFile* file, int depth  )
{
	int i;
	
	for ( i = 0; i < depth; i ++ )
	{
		file->Write ( "\t", 1 );
	}
}

/*
================
rvGEWorkspace::WriteWindow

Writes the contents of the given window to the file
================
*/
bool rvGEWorkspace::WriteWindow ( idFile* file, int depth, idWindow* window )
{
	idStr				out;
	rvGEWindowWrapper*	wrapper;
	int					i;

	wrapper = rvGEWindowWrapper::GetWrapper ( window );
	if ( !wrapper )
	{
		return true;
	}

	if ( wrapper->IsDeleted ( ) )
	{
		return true;
	}

	// Window def header	
	WriteTabs ( file, depth - 1 );
	
	out = wrapper->WindowTypeToString ( wrapper->GetWindowType ( ) );			
	out.Append ( " " );
	file->Write ( out, out.Length() );	

	out = window->GetName ( );
	file->Write ( out, out.Length() );
	file->Write ( "\r\n", 2 );
	
	WriteTabs ( file, depth - 1 );
	
	out = "{\r\n";
	file->Write ( out, out.Length() );
	file->ForceFlush ( );

	for ( i = 0; i < wrapper->GetStateDict().GetNumKeyVals(); i ++ )
	{
		const idKeyValue* key = wrapper->GetStateDict().GetKeyVal ( i );		

		// Dont write name to the files
		if ( !key->GetKey().Icmp ( "name" ) )
		{
			continue;
		}
					
		WriteTabs ( file, depth );
			
		out = key->GetKey();
		out.Append ( "\t" );
		file->Write ( out, out.Length() );

		const char* p;
		for ( p = key->GetValue().c_str(); *p; p ++ )
		{			
			switch ( *p )
			{
				case '\n':
					file->Write ( "\\n", 2 );
					break;
				
				default:
					file->Write ( p, 1 );
					break;
			}
		}
		
		file->Write ( "\r\n", 2 );
	}

	for ( i = 0; i < wrapper->GetVariableDict().GetNumKeyVals(); i ++ )
	{
		const idKeyValue* key = wrapper->GetVariableDict().GetKeyVal ( i );

		WriteTabs ( file, depth );

		out = key->GetKey();
		out.Append ( "\t" );
		out.Append ( key->GetValue() );
		out.Append ( "\r\n" );
		
		file->Write ( out, out.Length() );
	}

	if ( wrapper->GetScriptDict().GetNumKeyVals() )
	{
		file->Write ( "\r\n", 2 );
	}

	for ( i = 0; i < wrapper->GetScriptDict().GetNumKeyVals(); i ++ )
	{
		const idKeyValue* key = wrapper->GetScriptDict().GetKeyVal ( i );

		WriteTabs ( file, depth );
				
		file->Write ( key->GetKey(), key->GetKey().Length() );
		file->Write ( " ", 1 );

		idLexer src( key->GetValue(), key->GetValue().Length(), "", LEXFL_ALLOWMULTICHARLITERALS | LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWBACKSLASHSTRINGCONCAT );
		src.ParseBracedSectionExact ( out, depth + 1);		
		
		file->Write ( out, out.Length() );
		file->Write ( "\r\n", 2 );
		file->Write ( "\r\n", 2 );
	}

	for ( i = 0; i < wrapper->GetChildCount(); i ++ )
	{
		idWindow* child = wrapper->GetChild ( i );
	
		WriteWindow ( file, depth + 1, child );	
	}

	// Window def footer
	WriteTabs ( file, depth - 1 );
	
	out = "}\r\n";
	file->Write ( out, out.Length() );
	file->ForceFlush ( );
	
	return true;
}

/*
================
rvGEWorkspace::NewFile

Opens a new file for editing
================
*/
bool rvGEWorkspace::NewFile ( void )
{
	idStr	empty;
	idStr	ospath;	
	idFile*	file;

	// Make a temporary file with nothing in it so we can just use 
	// load to do all the work
	ospath = fileSystem->RelativePathToOSPath ( "guis/Untitled.guiednew", "fs_basepath" );
	DeleteFile ( ospath );
	
	file = fileSystem->OpenFileWrite ( "guis/Untitled.guiednew" );
	if ( NULL == file )
	{
		return false;
	}
	
	empty = "windowDef Desktop { rect 0,0,640,480 }";
	file->Write ( empty, empty.Length() );
	fileSystem->CloseFile ( file );

	// Load the temporary file
	if ( !LoadFile ( ospath, NULL ) )
	{
		// Ensure the temp file doesnt hang around
		DeleteFile ( ospath );
		return false;
	}
	
	mNew = true;
	
	// Ensure the temp file doesnt hang around
	DeleteFile ( ospath );
	
	// Go back to using a .gui extensions
	ospath.StripFileExtension ( );
	ospath.Append ( ".gui" );
	
	mFilename = ospath;
			
	return true;
}

/*
================
rvGEWorkspace::LoadFile

Loads the given gui file.
================
*/
bool rvGEWorkspace::LoadFile ( const char* filename, idStr* error )
{
	delete mInterface;

	idStr tempfile;
	idStr ospath;
	bool  result;
	
	tempfile = "guis/temp.guied";
	ospath = fileSystem->RelativePathToOSPath ( tempfile, "fs_basepath" );

	// Make sure the gui directory exists
	idStr createDir = ospath;
	createDir.StripFilename ( );
	CreateDirectory ( createDir, NULL );

	SetFileAttributes ( ospath, FILE_ATTRIBUTE_NORMAL );
	DeleteFile ( ospath );
	if ( !CopyFile ( filename, ospath, FALSE ) )
	{
		if ( error )
		{
			*error = "File not found";
		}
		return false;
	}
		
	SetFileAttributes ( ospath, FILE_ATTRIBUTE_NORMAL );

	mFilename = filename;
	UpdateTitle ( );
	
	// Let the real window system parse it first
	mInterface = NULL;
	result     = true;
	try 
	{	
		mInterface = reinterpret_cast< idUserInterfaceLocal* >( uiManager->FindGui( tempfile, true, true ) );
		if ( !mInterface && error )
		{
			*error = "File not found";
		}
	}
	catch ( idException& e )
	{
		result = false;
		if ( error )
		{
			*error = e.error;
		}
		return false;
	}

	if ( result )
	{
		rvGEWindowWrapper::GetWrapper ( mInterface->GetDesktop ( ) )->Expand ( );
	}
	else
	{
		DeleteFile ( ospath );
	}	

	return result;
}

/*
================
rvGEWorkspace::CheckIn

Checks in the current workspace file into source control
================
*/
bool rvGEWorkspace::CheckIn ( void )
{
	return false;

}

/*
================
rvGEWorkspace::CheckOut

Checks out the current workspace file from source control
================
*/
bool rvGEWorkspace::CheckOut ( void )
{
		return false;
}

/*
================
rvGEWorkspace::UndoCheckout

Undoes the checkout of the current file
================
*/
bool rvGEWorkspace::UndoCheckout ( void )
{
	return false;
}


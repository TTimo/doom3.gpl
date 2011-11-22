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

#include "../Game_local.h"
#include "../../MayaImport/maya_main.h"

/***********************************************************************

	Maya conversion functions

***********************************************************************/

static idStr				Maya_Error;

static exporterInterface_t	Maya_ConvertModel = NULL;
static exporterShutdown_t	Maya_Shutdown = NULL;
static int					importDLL = 0;

bool idModelExport::initialized = false;

/*
====================
idModelExport::idModelExport
====================
*/
idModelExport::idModelExport() {
	Reset();
}

/*
====================
idModelExport::Shutdown
====================
*/
void idModelExport::Shutdown( void ) {
	if ( Maya_Shutdown ) {
		Maya_Shutdown();
	}

	if ( importDLL ) {
		sys->DLL_Unload( importDLL );
	}

	importDLL = 0;
	Maya_Shutdown = NULL;
	Maya_ConvertModel = NULL;
	Maya_Error.Clear();
	initialized = false;
}

/*
=====================
idModelExport::CheckMayaInstall

Determines if Maya is installed on the user's machine
=====================
*/
bool idModelExport::CheckMayaInstall( void ) {
#ifndef _WIN32
	return false;
#elif 0
	HKEY	hKey;
	long	lres, lType;

	lres = RegOpenKey( HKEY_LOCAL_MACHINE, "SOFTWARE\\Alias|Wavefront\\Maya\\4.5\\Setup\\InstallPath", &hKey );

	if ( lres != ERROR_SUCCESS ) {
		return false;
	}

	lres = RegQueryValueEx( hKey, "MAYA_INSTALL_LOCATION", NULL, (unsigned long*)&lType, (unsigned char*)NULL, (unsigned long*)NULL );

	RegCloseKey( hKey );

	if ( lres != ERROR_SUCCESS ) {
		return false;
	}
	return true;
#else
	HKEY	hKey;
	long	lres;

	// only check the non-version specific key so that we only have to update the maya dll when new versions are released
	lres = RegOpenKey( HKEY_LOCAL_MACHINE, "SOFTWARE\\Alias|Wavefront\\Maya", &hKey );
	RegCloseKey( hKey );

	if ( lres != ERROR_SUCCESS ) {
		return false;
	}
	return true;
#endif
}

/*
=====================
idModelExport::LoadMayaDll

Checks to see if we can load the Maya export dll
=====================
*/
void idModelExport::LoadMayaDll( void ) {
	exporterDLLEntry_t	dllEntry;
	char				dllPath[ MAX_OSPATH ];

	fileSystem->FindDLL( "MayaImport", dllPath, false );
	if ( !dllPath[ 0 ] ) {
		return;
	}
	importDLL = sys->DLL_Load( dllPath );
	if ( !importDLL ) {
		return;
	}

	// look up the dll interface functions
	dllEntry = ( exporterDLLEntry_t )sys->DLL_GetProcAddress( importDLL, "dllEntry" ); 
	Maya_ConvertModel = ( exporterInterface_t )sys->DLL_GetProcAddress( importDLL, "Maya_ConvertModel" );
	Maya_Shutdown = ( exporterShutdown_t )sys->DLL_GetProcAddress( importDLL, "Maya_Shutdown" );
	if ( !Maya_ConvertModel || !dllEntry || !Maya_Shutdown ) {
		Maya_ConvertModel = NULL;
		Maya_Shutdown = NULL;
		sys->DLL_Unload( importDLL );
		importDLL = 0;
		gameLocal.Error( "Invalid interface on export DLL." );
		return;
	}

	// initialize the DLL
	if ( !dllEntry( MD5_VERSION, common, sys ) ) {
		// init failed
		Maya_ConvertModel = NULL;
		Maya_Shutdown = NULL;
		sys->DLL_Unload( importDLL );
		importDLL = 0;
		gameLocal.Error( "Export DLL init failed." );
		return;
	}
}

/*
=====================
idModelExport::ConvertMayaToMD5

Checks if a Maya model should be converted to an MD5, and converts if if the time/date or 
version number has changed.
=====================
*/
bool idModelExport::ConvertMayaToMD5( void ) {
	ID_TIME_T		
		sourceTime;
	ID_TIME_T		destTime;
	int			version;
	idToken		cmdLine;
	idStr		path;

	// check if our DLL got loaded
	if ( initialized && !Maya_ConvertModel ) {
		Maya_Error = "MayaImport dll not loaded.";
		return false;
	}

	// if idAnimManager::forceExport is set then we always reexport Maya models
	if ( idAnimManager::forceExport ) {
		force = true;
	}

	// get the source file's time
	if ( fileSystem->ReadFile( src, NULL, &sourceTime ) < 0 ) {
		// source file doesn't exist
		return true;
	}

	// get the destination file's time
	if ( !force && ( fileSystem->ReadFile( dest, NULL, &destTime ) >= 0 ) ) {
		idParser parser( LEXFL_ALLOWPATHNAMES | LEXFL_NOSTRINGESCAPECHARS );

		parser.LoadFile( dest );

		// read the file version
		if ( parser.CheckTokenString( MD5_VERSION_STRING ) ) {
			version = parser.ParseInt();

			// check the command line
			if ( parser.CheckTokenString( "commandline" ) ) {
				parser.ReadToken( &cmdLine );

				// check the file time, scale, and version
				if ( ( destTime >= sourceTime ) && ( version == MD5_VERSION ) && ( cmdLine == commandLine ) ) {
					// don't convert it
					return true;
				}
			}
		}
	}

	// if this is the first time we've been run, check if Maya is installed and load our DLL
	if ( !initialized ) {
		initialized = true;

		if ( !CheckMayaInstall() ) {
			Maya_Error = "Maya not installed in registry.";
			return false;
		}

		LoadMayaDll();

		// check if our DLL got loaded
		if ( !Maya_ConvertModel ) {
			Maya_Error = "Could not load MayaImport dll.";
			return false;
		}
	}

	// we need to make sure we have a full path, so convert the filename to an OS path
	src = fileSystem->RelativePathToOSPath( src );
	dest = fileSystem->RelativePathToOSPath( dest );

	dest.ExtractFilePath( path );
	if ( path.Length() ) {
		fileSystem->CreateOSPath( path );
	}

	// get the os path in case it needs to create one
	path = fileSystem->RelativePathToOSPath( "" );

	common->SetRefreshOnPrint( true );
	Maya_Error = Maya_ConvertModel( path, commandLine );
	common->SetRefreshOnPrint( false );
	if ( Maya_Error != "Ok" ) {
		return false;
	}
	
	// conversion succeded
	return true;
}

/*
====================
idModelExport::Reset
====================
*/
void idModelExport::Reset( void ) {
	force		= false;
	commandLine = "";
	src			= "";
	dest		= "";
}

/*
====================
idModelExport::ExportModel
====================
*/
bool idModelExport::ExportModel( const char *model ) {
	const char *game = cvarSystem->GetCVarString( "fs_game" );
	if ( strlen(game) == 0 ) {
		game = BASE_GAMEDIR;
	}

	Reset();
	src  = model;
	dest = model;
	dest.SetFileExtension( MD5_MESH_EXT );

	sprintf( commandLine, "mesh %s -dest %s -game %s", src.c_str(), dest.c_str(), game );
	if ( !ConvertMayaToMD5() ) {
		gameLocal.Printf( "Failed to export '%s' : %s", src.c_str(), Maya_Error.c_str() );
		return false;
	}

	return true;
}

/*
====================
idModelExport::ExportAnim
====================
*/
bool idModelExport::ExportAnim( const char *anim ) {
	const char *game = cvarSystem->GetCVarString( "fs_game" );
	if ( strlen(game) == 0 ) {
		game = BASE_GAMEDIR;
	}

	Reset();
	src  = anim;
	dest = anim;
	dest.SetFileExtension( MD5_ANIM_EXT );

	sprintf( commandLine, "anim %s -dest %s -game %s", src.c_str(), dest.c_str(), game );
	if ( !ConvertMayaToMD5() ) {
		gameLocal.Printf( "Failed to export '%s' : %s", src.c_str(), Maya_Error.c_str() );
		return false;
	}

	return true;
}

/*
====================
idModelExport::ParseOptions
====================
*/
bool idModelExport::ParseOptions( idLexer &lex ) {
	idToken	token;
	idStr	destdir;
	idStr	sourcedir;

	if ( !lex.ReadToken( &token ) ) {
		lex.Error( "Expected filename" );
		return false;
	}

	src = token;
	dest = token;

	while( lex.ReadToken( &token ) ) {
		if ( token == "-" ) {
			if ( !lex.ReadToken( &token ) ) {
				lex.Error( "Expecting option" );
				return false;
			}
			if ( token == "sourcedir" ) {
				if ( !lex.ReadToken( &token ) ) {
					lex.Error( "Missing pathname after -sourcedir" );
					return false;
				}
				sourcedir = token;
			} else if ( token == "destdir" ) {
				if ( !lex.ReadToken( &token ) ) {
					lex.Error( "Missing pathname after -destdir" );
					return false;
				}
				destdir = token;
			} else if ( token == "dest" ) {
				if ( !lex.ReadToken( &token ) ) {
					lex.Error( "Missing filename after -dest" );
					return false;
				}
				dest = token;
			} else {
				commandLine += va( " -%s", token.c_str() );
			}
		} else {
			commandLine += va( " %s", token.c_str() );
		}
	}

	if ( sourcedir.Length() ) {
		src.StripPath();
		sourcedir.BackSlashesToSlashes();
		sprintf( src, "%s/%s", sourcedir.c_str(), src.c_str() );
	}

	if ( destdir.Length() ) {
		dest.StripPath();
		destdir.BackSlashesToSlashes();
		sprintf( dest, "%s/%s", destdir.c_str(), dest.c_str() );
	}

	return true;
}

/*
====================
idModelExport::ParseExportSection
====================
*/
int idModelExport::ParseExportSection( idParser &parser ) {
	idToken	command;
	idToken	token;
	idStr	defaultCommands;
	idLexer lex;
	idStr	temp;
	idStr	parms;
	int		count;

	// only export sections that match our export mask
	if ( g_exportMask.GetString()[ 0 ] ) {
		if ( parser.CheckTokenString( "{" ) ) {
			parser.SkipBracedSection( false );
			return 0;
		}

        parser.ReadToken( &token );
		if ( token.Icmp( g_exportMask.GetString() ) ) {
			parser.SkipBracedSection();
			return 0;
		}
		parser.ExpectTokenString( "{" );
	} else if ( !parser.CheckTokenString( "{" ) ) {
		// skip the export mask
		parser.ReadToken( &token );
		parser.ExpectTokenString( "{" );
	}

	count = 0;

	lex.SetFlags( LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT );

	while( 1 ) {

		if ( !parser.ReadToken( &command ) ) {
			parser.Error( "Unexpoected end-of-file" );
			break;
		}

		if ( command == "}" ) {
			break;
		}

		if ( command == "options" ) {
			parser.ParseRestOfLine( defaultCommands );
		} else if ( command == "addoptions" ) {
			parser.ParseRestOfLine( temp );
			defaultCommands += " ";
			defaultCommands += temp;
		} else if ( ( command == "mesh" ) || ( command == "anim" ) || ( command == "camera" ) ) {
			if ( !parser.ReadToken( &token ) ) {
				parser.Error( "Expected filename" );
			}

			temp = token;
			parser.ParseRestOfLine( parms );

			if ( defaultCommands.Length() ) {
				sprintf( temp, "%s %s", temp.c_str(), defaultCommands.c_str() );
			}

			if ( parms.Length() ) {
				sprintf( temp, "%s %s", temp.c_str(), parms.c_str() );
			}

			lex.LoadMemory( temp, temp.Length(), parser.GetFileName() );

			Reset();
			if ( ParseOptions( lex ) ) {
				const char *game = cvarSystem->GetCVarString( "fs_game" );
				if ( strlen(game) == 0 ) {
					game = BASE_GAMEDIR;
				}

				if ( command == "mesh" ) {
					dest.SetFileExtension( MD5_MESH_EXT );
				} else if ( command == "anim" ) {
					dest.SetFileExtension( MD5_ANIM_EXT );
				} else if ( command == "camera" ) {
					dest.SetFileExtension( MD5_CAMERA_EXT );
				} else {
					dest.SetFileExtension( command );
				}
				idStr back = commandLine;
				sprintf( commandLine, "%s %s -dest %s -game %s%s", command.c_str(), src.c_str(), dest.c_str(), game, commandLine.c_str() );
				if ( ConvertMayaToMD5() ) {
					count++;
				} else {
					parser.Warning( "Failed to export '%s' : %s", src.c_str(), Maya_Error.c_str() );
				}
			}
			lex.FreeSource();
		} else {
			parser.Error( "Unknown token: %s", command.c_str() );
			parser.SkipBracedSection( false );
			break;
		}
	}

	return count;
}

/*
================
idModelExport::ExportDefFile
================
*/
int idModelExport::ExportDefFile( const char *filename ) {
	idParser	parser( LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT );
	idToken		token;
	int			count;

	count = 0;

	if ( !parser.LoadFile( filename ) ) {
		gameLocal.Printf( "Could not load '%s'\n", filename );
		return 0;
	}

	while( parser.ReadToken( &token ) ) {
		if ( token == "export" ) {
			count += ParseExportSection( parser );
		} else {
			parser.ReadToken( &token );
			parser.SkipBracedSection();
		}
	}

	return count;
}

/*
================
idModelExport::ExportModels
================
*/
int idModelExport::ExportModels( const char *pathname, const char *extension ) {
	int	count;

	count = 0;

	idFileList *files;
	int			i;

	if ( !CheckMayaInstall() ) {
		// if Maya isn't installed, don't bother checking if we have anims to export
		return 0;
	}

	gameLocal.Printf( "--------- Exporting models --------\n" );
	if ( !g_exportMask.GetString()[ 0 ] ) {
		gameLocal.Printf( "  Export mask: '%s'\n", g_exportMask.GetString() );
	}

	count = 0;

	files = fileSystem->ListFiles( pathname, extension );
	for( i = 0; i < files->GetNumFiles(); i++ ) {
		count += ExportDefFile( va( "%s/%s", pathname, files->GetFile( i ) ) );
	}
	fileSystem->FreeFileList( files );

	gameLocal.Printf( "...%d models exported.\n", count );
	gameLocal.Printf( "-----------------------------------\n" );

	return count;
}

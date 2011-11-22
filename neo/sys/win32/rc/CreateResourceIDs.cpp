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

#include "../../../idlib/precompiled.h"
#pragma hdrstop


/*
================
CreateResourceIDs_f
================
*/
void CreateResourceIDs_f( const idCmdArgs &args ) {
	int i, j;
	idStr path, fileName;
	idStrList resourceFiles;
	idLexer src;
	idToken token;
	idStrList dialogs;
	idStrList resources;
	idStrList bitmaps;
	idStrList icons;
	idStrList strings;
	idStrList controls;
	idStrList commands;

	if ( args.Argc() > 1 ) {
		path = args.Argv(1);
	} else {
		path = SOURCE_CODE_BASE_FOLDER"/";
		path.Append( __FILE__ );
		path.StripFilename();
		path.BackSlashesToSlashes();
	}

	common->Printf( "%s\n", path.c_str() );
	Sys_ListFiles( path, "_resource.h", resourceFiles );

	for ( i = 0; i < resourceFiles.Num(); i++ ) {

		fileName = path + "/" + resourceFiles[i];

		common->Printf( "creating IDs for %s...\n", fileName.c_str() );

		if ( !src.LoadFile( fileName, true ) ) {
			common->Warning( "couldn't load %s", fileName.c_str() );
			continue;
		}

		dialogs.Clear();
		resources.Clear();
		bitmaps.Clear();
		icons.Clear();
		strings.Clear();
		controls.Clear();
		commands.Clear();

		while( src.ReadToken( &token ) ) {
			if ( token == "#" ) {
				src.ExpectAnyToken( &token );
				if ( token == "ifdef" || token == "ifndef" ) {
					src.SkipRestOfLine();
				} else if ( token == "define" ) {
					src.ExpectTokenType( TT_NAME, 0, &token );

					if ( token.Icmpn( "_APS_", 5 ) == 0 ) {
						continue;
					}

					if ( token.Icmpn( "IDD_", 4 ) == 0 ) {
						dialogs.AddUnique( token );
					} else if ( token.Icmpn( "IDR_", 4 ) == 0 ) {
						resources.AddUnique( token );
					} else if ( token.Icmpn( "IDB_", 4 ) == 0 ) {
						bitmaps.AddUnique( token );
					} else if ( token.Icmpn( "IDI_", 4 ) == 0 ) {
						icons.AddUnique( token );
					} else if ( token.Icmpn( "IDS_", 4 ) == 0 ||
								token.Icmpn( "IDP_", 4 ) == 0 ) {
						strings.AddUnique( token );
					} else if ( token.Icmpn( "IDC_", 4 ) == 0 ) {
						controls.AddUnique( token );
					} else {
						commands.AddUnique( token );
					}
				}
			}
		}

		src.FreeSource();

		idFile *f;
		int curResource, curControl, curCommand;

		curResource = i ? i * 1000 : 100;
		curCommand = 20000 + i * 1000;
		curControl = i * 1000 + 200;

		f = fileSystem->OpenExplicitFileWrite( fileName );
		if ( !f ) {
			common->Warning( "couldn't write %s", fileName.c_str() );
			continue;
		}

		f->WriteFloatString(	"//{{NO_DEPENDENCIES}}\n"
								"// Microsoft Visual C++ generated include file.\n"
								"// Used by .rc\n"
								"//\n\n" );

		for ( j = 0; j < dialogs.Num(); j++ ) {
			f->WriteFloatString( "#define %-40s %d\n", dialogs[j].c_str(), curResource++ );
		}
		for ( j = 0; j < resources.Num(); j++ ) {
			f->WriteFloatString( "#define %-40s %d\n", resources[j].c_str(), curResource++ );
		}
		for ( j = 0; j < bitmaps.Num(); j++ ) {
			f->WriteFloatString( "#define %-40s %d\n", bitmaps[j].c_str(), curResource++ );
		}
		for ( j = 0; j < icons.Num(); j++ ) {
			f->WriteFloatString( "#define %-40s %d\n", icons[j].c_str(), curResource++ );
		}
		for ( j = 0; j < strings.Num(); j++ ) {
			f->WriteFloatString( "#define %-40s %d\n", strings[j].c_str(), curResource++ );
		}

		f->WriteFloatString( "\n" );

		for ( j = 0; j < controls.Num(); j++ ) {
			f->WriteFloatString( "#define %-40s %d\n", controls[j].c_str(), curControl++ );
		}

		f->WriteFloatString( "\n" );

		for ( j = 0; j < commands.Num(); j++ ) {

			// NOTE: special hack for Radiant
			if ( commands[j].Cmp( "ID_ENTITY_START" ) == 0 ) {
				f->WriteFloatString( "#define %-40s %d\n", commands[j].c_str(), 40000 );
				continue;
			}
			if ( commands[j].Cmp( "ID_ENTITY_END" ) == 0 ) {
				f->WriteFloatString( "#define %-40s %d\n", commands[j].c_str(), 45000 );
				continue;
			}

			f->WriteFloatString( "#define %-40s %d\n", commands[j].c_str(), curCommand++ );
		}


		f->WriteFloatString(	"\n// Next default values for new objects\n"
								"// \n"
								"#ifdef APSTUDIO_INVOKED\n"
								"#ifndef APSTUDIO_READONLY_SYMBOLS\n"
								"#define _APS_3D_CONTROLS                1\n"
								"#define _APS_NEXT_RESOURCE_VALUE        %d\n"
								"#define _APS_NEXT_COMMAND_VALUE         %d\n"
								"#define _APS_NEXT_CONTROL_VALUE         %d\n"
								"#define _APS_NEXT_SYMED_VALUE           %d\n"
								"#endif\n"
								"#endif\n", curResource, curCommand, curControl, curResource );

		fileSystem->CloseFile( f );
	}
}

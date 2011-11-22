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

#include "../idlib/precompiled.h"
#pragma hdrstop

/*
=================
idDeclPDA::Size
=================
*/
size_t idDeclPDA::Size( void ) const {
	return sizeof( idDeclPDA );
}

/*
===============
idDeclPDA::Print
===============
*/
void idDeclPDA::Print( void ) const {
	common->Printf( "Implement me\n" );
}

/*
===============
idDeclPDA::List
===============
*/
void idDeclPDA::List( void ) const {
	common->Printf( "Implement me\n" );
}

/*
================
idDeclPDA::Parse
================
*/
bool idDeclPDA::Parse( const char *text, const int textLength ) {
	idLexer src;
	idToken token;

	src.LoadMemory( text, textLength, GetFileName(), GetLineNum() );
	src.SetFlags( DECL_LEXER_FLAGS );
	src.SkipUntilString( "{" );

	// scan through, identifying each individual parameter
	while( 1 ) {
		
		if ( !src.ReadToken( &token ) ) {
			break;
		}

		if ( token == "}" ) {
			break;
		}

		if ( !token.Icmp( "name") ) {
			src.ReadToken( &token );
			pdaName = token;
			continue;
		}

		if ( !token.Icmp( "fullname") ) {
			src.ReadToken( &token );
			fullName = token;
			continue;
		}

		if ( !token.Icmp( "icon") ) {
			src.ReadToken( &token );
			icon = token;
			continue;
		}

		if ( !token.Icmp( "id") ) {
			src.ReadToken( &token );
			id = token;
			continue;
		}

		if ( !token.Icmp( "post") ) {
			src.ReadToken( &token );
			post = token;
			continue;
		}

		if ( !token.Icmp( "title") ) {
			src.ReadToken( &token );
			title = token;
			continue;
		}

		if ( !token.Icmp( "security") ) {
			src.ReadToken( &token );
			security = token;
			continue;
		}

		if ( !token.Icmp( "pda_email") ) {
			src.ReadToken( &token );
			emails.Append( token );
			declManager->FindType(DECL_EMAIL, token);
			continue;
		}

		if ( !token.Icmp( "pda_audio") ) {
			src.ReadToken( &token );
			audios.Append( token );
			declManager->FindType(DECL_AUDIO, token);
			continue;
		}

		if ( !token.Icmp( "pda_video") ) {
			src.ReadToken( &token );
			videos.Append( token );
			declManager->FindType(DECL_VIDEO, token);
			continue;
		}

	}

	if ( src.HadError() ) {
		src.Warning( "PDA decl '%s' had a parse error", GetName() );
		return false;
	}

	originalVideos = videos.Num();
	originalEmails = emails.Num();
	return true;
}

/*
===================
idDeclPDA::DefaultDefinition
===================
*/
const char *idDeclPDA::DefaultDefinition( void ) const {
	return
		"{\n"
		"\t"		"name  \"default pda\"\n"
		"}"; 
}

/*
===================
idDeclPDA::FreeData
===================
*/
void idDeclPDA::FreeData( void ) {
	videos.Clear();
	audios.Clear();
	emails.Clear();
	originalEmails = 0;
	originalVideos = 0;
}

/*
=================
idDeclPDA::AddVideo
=================
*/
void idDeclPDA::AddVideo( const char *name, bool unique ) const {
	if ( unique && ( videos.Find( name ) != NULL ) ) {
		return;
	}
	if ( declManager->FindType( DECL_VIDEO, name, false ) == NULL ) {
		common->Printf( "Video %s not found\n", name );
		return;
	}
	videos.Append( name );
}

/*
=================
idDeclPDA::AddAudio
=================
*/
void idDeclPDA::AddAudio( const char *name, bool unique ) const {
	if ( unique && ( audios.Find( name ) != NULL ) ) {
		return;
	}
	if ( declManager->FindType( DECL_AUDIO, name, false ) == NULL ) {
		common->Printf( "Audio log %s not found\n", name );
		return;
	}
	audios.Append( name );
}

/*
=================
idDeclPDA::AddEmail
=================
*/
void idDeclPDA::AddEmail( const char *name, bool unique ) const {
	if ( unique && ( emails.Find( name ) != NULL ) ) {
		return;
	}
	if ( declManager->FindType( DECL_EMAIL, name, false ) == NULL ) {
		common->Printf( "Email %s not found\n", name );
		return;
	}
	emails.Append( name );
}

/*
=================
idDeclPDA::RemoveAddedEmailsAndVideos
=================
*/
void idDeclPDA::RemoveAddedEmailsAndVideos() const {
	int num = emails.Num();
	if ( originalEmails < num ) {
		while ( num && num > originalEmails ) {
			emails.RemoveIndex( --num );
		}
	}
	num = videos.Num();
	if ( originalVideos < num ) {
		while ( num && num > originalVideos ) {
			videos.RemoveIndex( --num );
		}
	}
}

/*
=================
idDeclPDA::SetSecurity
=================
*/
void idDeclPDA::SetSecurity( const char *sec ) const {
	security = sec;
}

/*
=================
idDeclPDA::GetNumVideos
=================
*/
const int idDeclPDA::GetNumVideos() const {
	return videos.Num();
}

/*
=================
idDeclPDA::GetNumAudios
=================
*/
const int idDeclPDA::GetNumAudios() const {
	return audios.Num();
}

/*
=================
idDeclPDA::GetNumEmails
=================
*/
const int idDeclPDA::GetNumEmails() const {
	return emails.Num();
}

/*
=================
idDeclPDA::GetVideoByIndex
=================
*/
const idDeclVideo* idDeclPDA::GetVideoByIndex( int index ) const {
	if ( index >= 0 && index < videos.Num() ) {
		return static_cast< const idDeclVideo* >( declManager->FindType( DECL_VIDEO, videos[index], false ) );
	}
	return NULL;
}

/*
=================
idDeclPDA::GetAudioByIndex
=================
*/
const idDeclAudio* idDeclPDA::GetAudioByIndex( int index ) const {
	if ( index >= 0 && index < audios.Num() ) {
		return static_cast< const idDeclAudio* >( declManager->FindType( DECL_AUDIO, audios[index], false ) );
	}
	return NULL;
}

/*
=================
idDeclPDA::GetEmailByIndex
=================
*/
const idDeclEmail* idDeclPDA::GetEmailByIndex( int index ) const {
	if ( index >= 0 && index < emails.Num() ) {
		return static_cast< const idDeclEmail* >( declManager->FindType( DECL_EMAIL, emails[index], false ) );
	}
	return NULL;
}

/*
=================
idDeclEmail::Size
=================
*/
size_t idDeclEmail::Size( void ) const {
	return sizeof( idDeclEmail );
}

/*
===============
idDeclEmail::Print
===============
*/
void idDeclEmail::Print( void ) const {
	common->Printf( "Implement me\n" );
}

/*
===============
idDeclEmail::List
===============
*/
void idDeclEmail::List( void ) const {
	common->Printf( "Implement me\n" );
}

/*
================
idDeclEmail::Parse
================
*/
bool idDeclEmail::Parse( const char *_text, const int textLength ) {
	idLexer src;
	idToken token;

	src.LoadMemory( _text, textLength, GetFileName(), GetLineNum() );
	src.SetFlags( LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES |	LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT | LEXFL_NOFATALERRORS );
	src.SkipUntilString( "{" );

	text = "";
	// scan through, identifying each individual parameter
	while( 1 ) {

		if ( !src.ReadToken( &token ) ) {
			break;
		}

		if ( token == "}" ) {
			break;
		}

		if ( !token.Icmp( "subject") ) {
			src.ReadToken( &token );
			subject = token;
			continue;
		}

		if ( !token.Icmp( "to") ) {
			src.ReadToken( &token );
			to = token;
			continue;
		}

		if ( !token.Icmp( "from") ) {
			src.ReadToken( &token );
			from = token;
			continue;
		}

		if ( !token.Icmp( "date") ) {
			src.ReadToken( &token );
			date = token;
			continue;
		}

		if ( !token.Icmp( "text") ) {
			src.ReadToken( &token );
			if ( token != "{" ) {
				src.Warning( "Email decl '%s' had a parse error", GetName() );
				return false;
			}
			while ( src.ReadToken( &token ) && token != "}" ) {
				text += token;
			}
			continue;
		}

		if ( !token.Icmp( "image") ) {
			src.ReadToken( &token );
			image = token;
			continue;
		}
	}

	if ( src.HadError() ) {
		src.Warning( "Email decl '%s' had a parse error", GetName() );
		return false;
	}
	return true;
}

/*
===================
idDeclEmail::DefaultDefinition
===================
*/
const char *idDeclEmail::DefaultDefinition( void ) const {
	return
		"{\n"
		"\t"	"{\n"
		"\t\t"		"to\t5Mail recipient\n"
		"\t\t"		"subject\t5Nothing\n"
		"\t\t"		"from\t5No one\n"
		"\t"	"}\n"
		"}"; 
}

/*
===================
idDeclEmail::FreeData
===================
*/
void idDeclEmail::FreeData( void ) {
}

/*
=================
idDeclVideo::Size
=================
*/
size_t idDeclVideo::Size( void ) const {
	return sizeof( idDeclVideo );
}

/*
===============
idDeclVideo::Print
===============
*/
void idDeclVideo::Print( void ) const {
	common->Printf( "Implement me\n" );
}

/*
===============
idDeclVideo::List
===============
*/
void idDeclVideo::List( void ) const {
	common->Printf( "Implement me\n" );
}

/*
================
idDeclVideo::Parse
================
*/
bool idDeclVideo::Parse( const char *text, const int textLength ) {
	idLexer src;
	idToken token;

	src.LoadMemory( text, textLength, GetFileName(), GetLineNum() );
	src.SetFlags( LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES |	LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT | LEXFL_NOFATALERRORS );
	src.SkipUntilString( "{" );

	// scan through, identifying each individual parameter
	while( 1 ) {

		if ( !src.ReadToken( &token ) ) {
			break;
		}

		if ( token == "}" ) {
			break;
		}

		if ( !token.Icmp( "name") ) {
			src.ReadToken( &token );
			videoName = token;
			continue;
		}

		if ( !token.Icmp( "preview") ) {
			src.ReadToken( &token );
			preview = token;
			continue;
		}

		if ( !token.Icmp( "video") ) {
			src.ReadToken( &token );
			video = token;
			declManager->FindMaterial( video );			
			continue;
		}

		if ( !token.Icmp( "info") ) {
			src.ReadToken( &token );
			info = token;
			continue;
		}

		if ( !token.Icmp( "audio") ) {
			src.ReadToken( &token );
			audio = token;
			declManager->FindSound(audio);
			continue;
		}

	}

	if ( src.HadError() ) {
		src.Warning( "Video decl '%s' had a parse error", GetName() );
		return false;
	}
	return true;
}

/*
===================
idDeclVideo::DefaultDefinition
===================
*/
const char *idDeclVideo::DefaultDefinition( void ) const {
	return
		"{\n"
		"\t"	"{\n"
		"\t\t"		"name\t5Default Video\n"
		"\t"	"}\n"
		"}"; 
}

/*
===================
idDeclVideo::FreeData
===================
*/
void idDeclVideo::FreeData( void ) {
}

/*
=================
idDeclAudio::Size
=================
*/
size_t idDeclAudio::Size( void ) const {
	return sizeof( idDeclAudio );
}

/*
===============
idDeclAudio::Print
===============
*/
void idDeclAudio::Print( void ) const {
	common->Printf( "Implement me\n" );
}

/*
===============
idDeclAudio::List
===============
*/
void idDeclAudio::List( void ) const {
	common->Printf( "Implement me\n" );
}

/*
================
idDeclAudio::Parse
================
*/
bool idDeclAudio::Parse( const char *text, const int textLength ) {
	idLexer src;
	idToken token;

	src.LoadMemory( text, textLength, GetFileName(), GetLineNum() );
	src.SetFlags( LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWPATHNAMES |	LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT | LEXFL_NOFATALERRORS );
	src.SkipUntilString( "{" );

	// scan through, identifying each individual parameter
	while( 1 ) {

		if ( !src.ReadToken( &token ) ) {
			break;
		}

		if ( token == "}" ) {
			break;
		}

		if ( !token.Icmp( "name") ) {
			src.ReadToken( &token );
			audioName = token;
			continue;
		}

		if ( !token.Icmp( "audio") ) {
			src.ReadToken( &token );
			audio = token;
			declManager->FindSound(audio);
			continue;
		}

		if ( !token.Icmp( "info") ) {
			src.ReadToken( &token );
			info = token;
			continue;
		}

		if ( !token.Icmp( "preview") ) {
			src.ReadToken( &token );
			preview = token;
			continue;
		}

	}

	if ( src.HadError() ) {
		src.Warning( "Audio decl '%s' had a parse error", GetName() );
		return false;
	}
	return true;
}

/*
===================
idDeclAudio::DefaultDefinition
===================
*/
const char *idDeclAudio::DefaultDefinition( void ) const {
	return
		"{\n"
		"\t"	"{\n"
		"\t\t"		"name\t5Default Audio\n"
		"\t"	"}\n"
		"}"; 
}

/*
===================
idDeclAudio::FreeData
===================
*/
void idDeclAudio::FreeData( void ) {
}

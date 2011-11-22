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

#include "snd_local.h"


/*
===============
idSoundShader::Init
===============
*/
void idSoundShader::Init( void ) {
	desc = "<no description>";
	errorDuringParse = false;
	onDemand = false;
	numEntries = 0;
	numLeadins = 0;
	leadinVolume = 0;
	altSound = NULL;
}

/*
===============
idSoundShader::idSoundShader
===============
*/
idSoundShader::idSoundShader( void ) {
	Init();
}

/*
===============
idSoundShader::~idSoundShader
===============
*/
idSoundShader::~idSoundShader( void ) {
}

/*
=================
idSoundShader::Size
=================
*/
size_t idSoundShader::Size( void ) const {
	return sizeof( idSoundShader );
}

/*
===============
idSoundShader::idSoundShader::FreeData
===============
*/
void idSoundShader::FreeData() {
	numEntries = 0;
	numLeadins = 0;
}

/*
===================
idSoundShader::SetDefaultText
===================
*/
bool idSoundShader::SetDefaultText( void ) {
	idStr wavname;

	wavname = GetName();
	wavname.DefaultFileExtension( ".wav" );		// if the name has .ogg in it, that will stay

	// if there exists a wav file with the same name
	if ( 1 ) { //fileSystem->ReadFile( wavname, NULL ) != -1 ) {
		char generated[2048];
		idStr::snPrintf( generated, sizeof( generated ), 
						"sound %s // IMPLICITLY GENERATED\n"
						"{\n"
						"%s\n"
						"}\n", GetName(), wavname.c_str() );
		SetText( generated );
		return true;
	} else {
		return false;
	}
}

/*
===================
DefaultDefinition
===================
*/
const char *idSoundShader::DefaultDefinition() const {
	return
		"{\n"
	"\t"	"_default.wav\n"
		"}";
}

/*
===============
idSoundShader::Parse

  this is called by the declManager
===============
*/
bool idSoundShader::Parse( const char *text, const int textLength ) {
	idLexer	src;

	src.LoadMemory( text, textLength, GetFileName(), GetLineNum() );
	src.SetFlags( DECL_LEXER_FLAGS );
	src.SkipUntilString( "{" );

	// deeper functions can set this, which will cause MakeDefault() to be called at the end
	errorDuringParse = false;

	if ( !ParseShader( src ) || errorDuringParse ) {
		MakeDefault();
		return false;
	}
	return true;
}

/*
===============
idSoundShader::ParseShader
===============
*/
bool idSoundShader::ParseShader( idLexer &src ) {
	int			i;
	idToken		token;

	parms.minDistance = 1;
	parms.maxDistance = 10;
	parms.volume = 1;
	parms.shakes = 0;
	parms.soundShaderFlags = 0;
	parms.soundClass = 0;

	speakerMask = 0;
	altSound = NULL;

	for( i = 0; i < SOUND_MAX_LIST_WAVS; i++ ) {
		leadins[i] = NULL;
		entries[i] = NULL;
	}
	numEntries = 0;
	numLeadins = 0;

	int	maxSamples = idSoundSystemLocal::s_maxSoundsPerShader.GetInteger();
	if ( com_makingBuild.GetBool() || maxSamples <= 0 || maxSamples > SOUND_MAX_LIST_WAVS ) {
		maxSamples = SOUND_MAX_LIST_WAVS;
	}

	while ( 1 ) {
		if ( !src.ExpectAnyToken( &token ) ) {
			return false;
		}
		// end of definition
		else if ( token == "}" ) {
			break;
		}
		// minimum number of sounds
		else if ( !token.Icmp( "minSamples" ) ) {
			maxSamples = idMath::ClampInt( src.ParseInt(), SOUND_MAX_LIST_WAVS, maxSamples );
		}
		// description
		else if ( !token.Icmp( "description" ) ) {
			src.ReadTokenOnLine( &token );
			desc = token.c_str();
		}
		// mindistance
		else if ( !token.Icmp( "mindistance" ) ) {
			parms.minDistance = src.ParseFloat();
		}
		// maxdistance
		else if ( !token.Icmp( "maxdistance" ) ) {
			parms.maxDistance = src.ParseFloat();
		}
		// shakes screen
		else if ( !token.Icmp( "shakes" ) ) {
			src.ExpectAnyToken( &token );
			if ( token.type == TT_NUMBER ) {
				parms.shakes = token.GetFloatValue();
			} else {
				src.UnreadToken( &token );
				parms.shakes = 1.0f;
			}
		}
		// reverb
		else if ( !token.Icmp( "reverb" ) ) {
			int reg0 = src.ParseFloat();
			if ( !src.ExpectTokenString( "," ) ) {
				src.FreeSource();
				return false;
			}
			int reg1 = src.ParseFloat();
			// no longer supported
		}
		// volume
		else if ( !token.Icmp( "volume" ) ) {
			parms.volume = src.ParseFloat();
		}
		// leadinVolume is used to allow light breaking leadin sounds to be much louder than the broken loop
		else if ( !token.Icmp( "leadinVolume" ) ) {
			leadinVolume = src.ParseFloat();
		}
		// speaker mask
		else if ( !token.Icmp( "mask_center" ) ) {
			speakerMask |= 1<<SPEAKER_CENTER;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_left" ) ) {
			speakerMask |= 1<<SPEAKER_LEFT;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_right" ) ) {
			speakerMask |= 1<<SPEAKER_RIGHT;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_backright" ) ) {
			speakerMask |= 1<<SPEAKER_BACKRIGHT;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_backleft" ) ) {
			speakerMask |= 1<<SPEAKER_BACKLEFT;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_lfe" ) ) {
			speakerMask |= 1<<SPEAKER_LFE;
		}
		// soundClass
		else if ( !token.Icmp( "soundClass" ) ) {
			parms.soundClass = src.ParseInt();
			if ( parms.soundClass < 0 || parms.soundClass >= SOUND_MAX_CLASSES ) {
				src.Warning( "SoundClass out of range" );
				return false;
			}
		}
		// altSound
		else if ( !token.Icmp( "altSound" ) ) {
			if ( !src.ExpectAnyToken( &token ) ) {
				return false;
			}
			altSound = declManager->FindSound( token.c_str() );
		}
		// ordered
		else if ( !token.Icmp( "ordered" ) ) {
			// no longer supported
		}
		// no_dups
		else if ( !token.Icmp( "no_dups" ) ) {
			parms.soundShaderFlags |= SSF_NO_DUPS;
		}
		// no_flicker
		else if ( !token.Icmp( "no_flicker" ) ) {
			parms.soundShaderFlags |= SSF_NO_FLICKER;
		}
		// plain
		else if ( !token.Icmp( "plain" ) ) {	
			// no longer supported
		}
		// looping
		else if ( !token.Icmp( "looping" ) ) {
			parms.soundShaderFlags |= SSF_LOOPING;
		}
		// no occlusion
		else if ( !token.Icmp( "no_occlusion" ) ) {
			parms.soundShaderFlags |= SSF_NO_OCCLUSION;
		}
		// private
		else if ( !token.Icmp( "private" ) ) {
			parms.soundShaderFlags |= SSF_PRIVATE_SOUND;
		}
		// antiPrivate
		else if ( !token.Icmp( "antiPrivate" ) ) {
			parms.soundShaderFlags |= SSF_ANTI_PRIVATE_SOUND;
		}
		// once
		else if ( !token.Icmp( "playonce" ) ) {
			parms.soundShaderFlags |= SSF_PLAY_ONCE;
		}
		// global
		else if ( !token.Icmp( "global" ) ) {
			parms.soundShaderFlags |= SSF_GLOBAL;
		}
		// unclamped
		else if ( !token.Icmp( "unclamped" ) ) {
			parms.soundShaderFlags |= SSF_UNCLAMPED;
		}
		// omnidirectional
		else if ( !token.Icmp( "omnidirectional" ) ) {
			parms.soundShaderFlags |= SSF_OMNIDIRECTIONAL;
		}
		// onDemand can't be a parms, because we must track all references and overrides would confuse it
		else if ( !token.Icmp( "onDemand" ) ) {
			// no longer loading sounds on demand
			//onDemand = true;
		}

		// the wave files
		else if ( !token.Icmp( "leadin" ) ) {
			// add to the leadin list
			if ( !src.ReadToken( &token ) ) {
				src.Warning( "Expected sound after leadin" );
				return false;
			}
			if ( soundSystemLocal.soundCache && numLeadins < maxSamples ) {
				leadins[ numLeadins ] = soundSystemLocal.soundCache->FindSound( token.c_str(), onDemand );
				numLeadins++;
			}
		} else if ( token.Find( ".wav", false ) != -1 || token.Find( ".ogg", false ) != -1 ) {
			// add to the wav list
			if ( soundSystemLocal.soundCache && numEntries < maxSamples ) {
				token.BackSlashesToSlashes();
				idStr lang = cvarSystem->GetCVarString( "sys_lang" );
				if ( lang.Icmp( "english" ) != 0 && token.Find( "sound/vo/", false ) >= 0 ) {
					idStr work = token;
					work.ToLower();
					work.StripLeading( "sound/vo/" );
					work = va( "sound/vo/%s/%s", lang.c_str(), work.c_str() );
					if ( fileSystem->ReadFile( work, NULL, NULL ) > 0 ) {
						token = work;
					} else {
						// also try to find it with the .ogg extension
						work.SetFileExtension( ".ogg" );
						if ( fileSystem->ReadFile( work, NULL, NULL ) > 0 ) {
							token = work;
						}
					}
				} 					
				entries[ numEntries ] = soundSystemLocal.soundCache->FindSound( token.c_str(), onDemand );
				numEntries++;
			}
		} else {
			src.Warning( "unknown token '%s'", token.c_str() );
			return false;
		}
	}

	if ( parms.shakes > 0.0f ) {
		CheckShakesAndOgg();
	}

	return true;
}

/*
===============
idSoundShader::CheckShakesAndOgg
===============
*/
bool idSoundShader::CheckShakesAndOgg( void ) const {
	int i;
	bool ret = false;

	for ( i = 0; i < numLeadins; i++ ) {
		if ( leadins[ i ]->objectInfo.wFormatTag == WAVE_FORMAT_TAG_OGG ) {
			common->Warning( "sound shader '%s' has shakes and uses OGG file '%s'",
								GetName(), leadins[ i ]->name.c_str() );
			ret = true;
		}
	}
	for ( i = 0; i < numEntries; i++ ) {
		if ( entries[ i ]->objectInfo.wFormatTag == WAVE_FORMAT_TAG_OGG ) {
			common->Warning( "sound shader '%s' has shakes and uses OGG file '%s'",
								GetName(), entries[ i ]->name.c_str() );
			ret = true;
		}
	}
	return ret;
}

/*
===============
idSoundShader::List
===============
*/
void idSoundShader::List() const {
	idStrList	shaders;

	common->Printf( "%4i: %s\n", Index(), GetName() );
	if ( idStr::Icmp( GetDescription(), "<no description>" ) != 0 ) {
		common->Printf( "      description: %s\n", GetDescription() );
	}
	for( int k = 0; k < numLeadins ; k++ ) {
		const idSoundSample *objectp = leadins[k];
		if ( objectp ) {
			common->Printf( "      %5dms %4dKb %s (LEADIN)\n", soundSystemLocal.SamplesToMilliseconds(objectp->LengthIn44kHzSamples()), (objectp->objectMemSize/1024)
				,objectp->name.c_str() );
		}
	}
	for( int k = 0; k < numEntries; k++ ) {
		const idSoundSample *objectp = entries[k];
		if ( objectp ) {
			common->Printf( "      %5dms %4dKb %s\n", soundSystemLocal.SamplesToMilliseconds(objectp->LengthIn44kHzSamples()), (objectp->objectMemSize/1024)
				,objectp->name.c_str() );
		}
	}
}

/*
===============
idSoundShader::GetAltSound
===============
*/
const idSoundShader *idSoundShader::GetAltSound( void ) const {
	return altSound;
}

/*
===============
idSoundShader::GetMinDistance
===============
*/
float idSoundShader::GetMinDistance() const {
	return parms.minDistance;
}

/*
===============
idSoundShader::GetMaxDistance
===============
*/
float idSoundShader::GetMaxDistance() const {
	return parms.maxDistance;
}

/*
===============
idSoundShader::GetDescription
===============
*/
const char *idSoundShader::GetDescription() const {
	return desc;
}

/*
===============
idSoundShader::HasDefaultSound
===============
*/
bool idSoundShader::HasDefaultSound() const {
	for ( int i = 0; i < numEntries; i++ ) {
		if ( entries[i] && entries[i]->defaultSound ) {
			return true;
		}
	}
	return false;
}

/*
===============
idSoundShader::GetParms
===============
*/
const soundShaderParms_t *idSoundShader::GetParms() const {
	return &parms;
}

/*
===============
idSoundShader::GetNumSounds
===============
*/
int idSoundShader::GetNumSounds() const {
	return numLeadins + numEntries;
}

/*
===============
idSoundShader::GetSound
===============
*/
const char *idSoundShader::GetSound( int index ) const {
	if ( index >= 0 ) {
		if ( index < numLeadins ) {
			return leadins[index]->name.c_str();
		}
		index -= numLeadins;
		if ( index < numEntries ) {
			return entries[index]->name.c_str();
		}
	}
	return "";
}

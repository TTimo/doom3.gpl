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

// This is real evil but allows the code to inspect arbitrary class variables.
#define private		public
#define protected	public

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

#ifdef ID_DEBUG_MEMORY
#include "GameTypeInfo.h"				// Make sure this is up to date!
#else
#include "NoGameTypeInfo.h"
#endif

// disabled because it's adds about 64MB to state dumps and takes a really long time
//#define DUMP_GAMELOCAL


typedef void (*WriteVariableType_t)( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize );


class idTypeInfoTools {
public:
	static const classTypeInfo_t *	FindClassInfo( const char *typeName );
	static const enumTypeInfo_t *	FindEnumInfo( const char *typeName );
	static bool						IsSubclassOf( const char *typeName, const char *superType );
	static void						PrintType( const void *typePtr, const char *typeName );
	static void						WriteTypeToFile( idFile *fp, const void *typePtr, const char *typeName );
	static void						InitTypeVariables( const void *typePtr, const char *typeName, int value );
	static void						WriteGameState( const char *fileName );
	static void						CompareGameState( const char *fileName );

private:
	static idFile *					fp;
	static int						initValue;
	static WriteVariableType_t		Write;
	static idLexer *				src;
	static bool						typeError;

	static const char *				OutputString( const char *string );
	static bool						ParseTemplateArguments( idLexer &src, idStr &arguments );
	static void						PrintVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize );
	static void						WriteVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize );
	static void						WriteGameStateVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize );
	static void						InitVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize );
	static void						VerifyVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize );
	static int						WriteVariable_r( const void *varPtr, const char *varName, const char *varType, const char *scope, const char *prefix, const int pointerDepth );
	static void						WriteClass_r( const void *classPtr, const char *className, const char *classType, const char *scope, const char *prefix, const int pointerDepth );
};

idFile *							idTypeInfoTools::fp			= NULL;
int									idTypeInfoTools::initValue	= 0;
WriteVariableType_t					idTypeInfoTools::Write		= NULL;
idLexer *							idTypeInfoTools::src		= NULL;
bool								idTypeInfoTools::typeError	= false;


/*
================
GetTypeVariableName
================
*/
const char *GetTypeVariableName( const char *typeName, int offset ) {
	static char varName[1024];
	int i;

	for ( i = 0; classTypeInfo[i].typeName != NULL; i++ ) {
		if ( idStr::Cmp( typeName, classTypeInfo[i].typeName ) == 0 ) {
			if ( classTypeInfo[i].variables[0].name != NULL && offset >= classTypeInfo[i].variables[0].offset ) {
				break;
			}
			typeName = classTypeInfo[i].superType;
			if ( *typeName == '\0' ) {
				return "<unknown>";
			}
			i = -1;
		}
	}

	const classTypeInfo_t &classInfo = classTypeInfo[i];

	for ( i = 0; classInfo.variables[i].name != NULL; i++ ) {
		if ( offset <= classInfo.variables[i].offset ) {
			break;
		}
	}
	if ( i == 0 ) {
		idStr::snPrintf( varName, sizeof( varName ), "%s::<unknown>", classInfo.typeName );
	} else {
		idStr::snPrintf( varName, sizeof( varName ), "%s::%s", classInfo.typeName, classInfo.variables[i-1].name );
	}
	return varName;
}

/*
================
idTypeInfoTools::FindClassInfo
================
*/
const classTypeInfo_t *idTypeInfoTools::FindClassInfo( const char *typeName ) {
	int i;

	for ( i = 0; classTypeInfo[i].typeName != NULL; i++ ) {
		if ( idStr::Cmp( typeName, classTypeInfo[i].typeName ) == 0 ) {
			return &classTypeInfo[i];
		}
	}
	return NULL;
}

/*
================
idTypeInfoTools::FindEnumInfo
================
*/
const enumTypeInfo_t *idTypeInfoTools::FindEnumInfo( const char *typeName ) {
	int i;

	for ( i = 0; enumTypeInfo[i].typeName != NULL; i++ ) {
		if ( idStr::Cmp( typeName, enumTypeInfo[i].typeName ) == 0 ) {
			return &enumTypeInfo[i];
		}
	}
	return NULL;
}

/*
================
idTypeInfoTools::IsSubclassOf
================
*/
bool idTypeInfoTools::IsSubclassOf( const char *typeName, const char *superType ) {
	int i;

	while( *typeName != '\0' ) {
		if ( idStr::Cmp( typeName, superType ) == 0 ) {
			return true;
		}
		for ( i = 0; classTypeInfo[i].typeName != NULL; i++ ) {
			if ( idStr::Cmp( typeName, classTypeInfo[i].typeName ) == 0 ) {
				typeName = classTypeInfo[i].superType;
				break;
			}
		}
		if ( classTypeInfo[i].typeName == NULL ) {
			common->Warning( "super class %s not found", typeName );
			break;
		}
	}
	return false;
}

/*
================
idTypeInfoTools::OutputString
================
*/
const char *idTypeInfoTools::OutputString( const char *string ) {
	static int index = 0;
	static char buffers[4][16384];
	char *out;
	int i, c;

	out = buffers[index];
	index = ( index + 1 ) & 3;

	if ( string == NULL ) {
		return NULL;
	}

	for ( i = 0; i < sizeof( buffers[0] ) - 2; i++ ) {
		c = *string++;
		switch( c ) {
			case '\0': out[i] = '\0'; return out;
			case '\\': out[i++] = '\\'; out[i] = '\\'; break;
			case '\n': out[i++] = '\\'; out[i] = 'n'; break;
			case '\r': out[i++] = '\\'; out[i] = 'r'; break;
			case '\t': out[i++] = '\\'; out[i] = 't'; break;
			case '\v': out[i++] = '\\'; out[i] = 'v'; break;
			default: out[i] = c; break;
		}
	}
	out[i] = '\0';
	return out;
}

/*
================
idTypeInfoTools::ParseTemplateArguments
================
*/
bool idTypeInfoTools::ParseTemplateArguments( idLexer &src, idStr &arguments ) {
	int indent;
	idToken token;

	arguments = "";

	if ( !src.ExpectTokenString( "<" ) ) {
		return false;
	}

	indent = 1;
	while( indent ) {
		if ( !src.ReadToken( &token ) ) {
			break;
		}
		if ( token == "<" ) {
			indent++;
		} else if ( token == ">" ) {
			indent--;
		} else {
			if ( arguments.Length() ) {
				arguments += " ";
			}
			arguments += token;
		}
	}
	return true;
}

/*
================
idTypeInfoTools::PrintType
================
*/
void idTypeInfoTools::PrintType( const void *typePtr, const char *typeName ) {
	idTypeInfoTools::fp = NULL;
	idTypeInfoTools::initValue = 0;
	idTypeInfoTools::Write = PrintVariable;
	WriteClass_r( typePtr, "", typeName, "", "", 0 );
}

/*
================
idTypeInfoTools::WriteTypeToFile
================
*/
void idTypeInfoTools::WriteTypeToFile( idFile *fp, const void *typePtr, const char *typeName ) {
	idTypeInfoTools::fp = fp;
	idTypeInfoTools::initValue = 0;
	idTypeInfoTools::Write = WriteVariable;
	WriteClass_r( typePtr, "", typeName, "", "", 0 );
}

/*
================
idTypeInfoTools::InitTypeVariables
================
*/
void idTypeInfoTools::InitTypeVariables( const void *typePtr, const char *typeName, int value ) {
	idTypeInfoTools::fp = NULL;
	idTypeInfoTools::initValue = value;
	idTypeInfoTools::Write = InitVariable;
	WriteClass_r( typePtr, "", typeName, "", "", 0 );
}

/*
================
IsAllowedToChangedFromSaveGames
================
*/
bool IsAllowedToChangedFromSaveGames( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value ) {
	if ( idStr::Icmp( scope, "idAnimator" ) == 0 ) {
		if ( idStr::Icmp( varName, "forceUpdate" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "lastTransformTime" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "AFPoseTime" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "frameBounds" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idClipModel" ) == 0 ) {
		if ( idStr::Icmp( varName, "touchCount" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idEntity" ) == 0 ) {
		if ( idStr::Icmp( varName, "numPVSAreas" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "renderView" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idBrittleFracture" ) == 0 ) {
		if ( idStr::Icmp( varName, "changed" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idPhysics_AF" ) == 0 ) {
		return true;
	} else if ( idStr::Icmp( scope, "renderEntity_t" ) == 0 ) {
		// These get fixed up when UpdateVisuals is called
		if ( idStr::Icmp( varName, "origin" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "axis" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "bounds" ) == 0 ) {
			return true;
		}
	}

	if ( idStr::Icmpn( prefix, "idAFEntity_Base::af.idAF::physicsObj.idPhysics_AF", 49) == 0 ) {
		return true;
	}

	return false;
}

/*
================
IsRenderHandleVariable
================
*/
bool IsRenderHandleVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value ) {
	if ( idStr::Icmp( scope, "idClipModel" ) == 0 ) {
		if ( idStr::Icmp( varName, "renderModelHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idFXLocalAction" ) == 0 ) {
		if ( idStr::Icmp( varName, "lightDefHandle" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "modelDefHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idEntity" ) == 0 ) {
		if ( idStr::Icmp( varName, "modelDefHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idLight" ) == 0 ) {
		if ( idStr::Icmp( varName, "lightDefHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idAFEntity_Gibbable" ) == 0 ) {
		if ( idStr::Icmp( varName, "skeletonModelDefHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idAFEntity_SteamPipe" ) == 0 ) {
		if ( idStr::Icmp( varName, "steamModelHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idItem" ) == 0 ) {
		if ( idStr::Icmp( varName, "itemShellHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idExplodingBarrel" ) == 0 ) {
		if ( idStr::Icmp( varName, "particleModelDefHandle" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "lightDefHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idProjectile" ) == 0 ) {
		if ( idStr::Icmp( varName, "lightDefHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idBFGProjectile" ) == 0 ) {
		if ( idStr::Icmp( varName, "secondModelDefHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idSmokeParticles" ) == 0 ) {
		if ( idStr::Icmp( varName, "renderEntityHandle" ) == 0 ) {
			return true;
		}
	} else if ( idStr::Icmp( scope, "idWeapon" ) == 0 ) {
		if ( idStr::Icmp( varName, "muzzleFlashHandle" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "worldMuzzleFlashHandle" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "guiLightHandle" ) == 0 ) {
			return true;
		}
		if ( idStr::Icmp( varName, "nozzleGlowHandle" ) == 0 ) {
			return true;
		}
	}
	return false;
}

/*
================
idTypeInfoTools::PrintVariable
================
*/
void idTypeInfoTools::PrintVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize ) {
	common->Printf( "%s%s::%s%s = \"%s\"\n", prefix, scope, varName, postfix, value );
}

/*
================
idTypeInfoTools::WriteVariable
================
*/
void idTypeInfoTools::WriteVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize ) {

	for ( int i = idStr::FindChar( value, '#', 0 ); i >= 0; i = idStr::FindChar( value, '#', i+1 ) ) {
		if (	idStr::Icmpn( value+i+1, "INF", 3 ) == 0 ||
				idStr::Icmpn( value+i+1, "IND", 3 ) == 0 ||
				idStr::Icmpn( value+i+1, "NAN", 3 ) == 0 ||
				idStr::Icmpn( value+i+1, "QNAN", 4 ) == 0 ||
				idStr::Icmpn( value+i+1, "SNAN", 4 ) == 0 ) {
			common->Warning( "%s%s::%s%s = \"%s\"", prefix, scope, varName, postfix, value );
			break;
		}
	}
	fp->WriteFloatString( "%s%s::%s%s = \"%s\"\n", prefix, scope, varName, postfix, value );
}

/*
================
idTypeInfoTools::WriteGameStateVariable
================
*/
void idTypeInfoTools::WriteGameStateVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize ) {

	for ( int i = idStr::FindChar( value, '#', 0 ); i >= 0; i = idStr::FindChar( value, '#', i+1 ) ) {
		if (	idStr::Icmpn( value+i+1, "INF", 3 ) == 0 ||
				idStr::Icmpn( value+i+1, "IND", 3 ) == 0 ||
				idStr::Icmpn( value+i+1, "NAN", 3 ) == 0 ||
				idStr::Icmpn( value+i+1, "QNAN", 4 ) == 0 ||
				idStr::Icmpn( value+i+1, "SNAN", 4 ) == 0 ) {
			common->Warning( "%s%s::%s%s = \"%s\"", prefix, scope, varName, postfix, value );
			break;
		}
	}

	if ( IsRenderHandleVariable( varName, varType, scope, prefix, postfix, value ) ) {
		return;
	}

	if ( IsAllowedToChangedFromSaveGames( varName, varType, scope, prefix, postfix, value ) ) {
		return;
	}

	fp->WriteFloatString( "%s%s::%s%s = \"%s\"\n", prefix, scope, varName, postfix, value );
}

/*
================
idTypeInfoTools::InitVariable
================
*/
void idTypeInfoTools::InitVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize ) {
	if ( varPtr != NULL && varSize > 0 ) {
		// NOTE: skip renderer handles
		if ( IsRenderHandleVariable( varName, varType, scope, prefix, postfix, value ) ) {
			return;
		}
		memset( const_cast<void*>(varPtr), initValue, varSize );
	}
}

/*
================
idTypeInfoTools::VerifyVariable
================
*/
void idTypeInfoTools::VerifyVariable( const char *varName, const char *varType, const char *scope, const char *prefix, const char *postfix, const char *value, const void *varPtr, int varSize ) {
	idToken token;

	if ( typeError ) {
		return;
	}

	src->SkipUntilString( "=" );
	src->ExpectTokenType( TT_STRING, 0, &token );
	if ( token.Cmp( value ) != 0 ) {

		// NOTE: skip several things

		if ( IsRenderHandleVariable( varName, varType, scope, prefix, postfix, value ) ) {
			return;
		}

		if ( IsAllowedToChangedFromSaveGames( varName, varType, scope, prefix, postfix, value ) ) {
			return;
		}

		src->Warning( "state diff for %s%s::%s%s\n%s\n%s", prefix, scope, varName, postfix, token.c_str(), value );
		typeError = true;
	}
}

/*
================
idTypeInfoTools::WriteVariable_r
================
*/
int idTypeInfoTools::WriteVariable_r( const void *varPtr, const char *varName, const char *varType, const char *scope, const char *prefix, const int pointerDepth ) {
	int i, isPointer, typeSize;
	idLexer typeSrc;
	idToken token;
	idStr typeString, templateArgs;

	isPointer = 0;
	typeSize = -1;

	// create a type string without 'const', 'mutable', 'class', 'struct', 'union'
	typeSrc.LoadMemory( varType, idStr::Length( varType ), varName );
	while( typeSrc.ReadToken( &token ) ) {
		if ( token != "const" && token != "mutable" && token != "class" && token != "struct" && token != "union" ) {
			typeString += token + " ";
		}
	}
	typeString.StripTrailing( ' ' );
	typeSrc.FreeSource();

	// if this is an array
	if ( typeString[typeString.Length() - 1] == ']' ) {
		for ( i = typeString.Length(); i > 0 && typeString[i - 1] != '['; i-- ) {
		}
		int num = atoi( &typeString[i] );
		idStr listVarType = typeString;
		listVarType.CapLength( i - 1 );
		typeSize = 0;
		for ( i = 0; i < num; i++ ) {
			idStr listVarName = va( "%s[%d]", varName, i );
			int size = WriteVariable_r( varPtr, listVarName, listVarType, scope, prefix, pointerDepth );
			typeSize += size;
			if ( size == -1 ) {
				break;
			}
			varPtr = (void *)( ( (byte *) varPtr ) + size );
		}
		return typeSize;
	}

	// if this is a pointer
	isPointer = 0;
	for ( i = typeString.Length(); i > 0 && typeString[i - 1] == '*'; i -= 2 ) {
		if ( varPtr == (void *)0xcdcdcdcd || ( varPtr != NULL && *((unsigned long *)varPtr) == 0xcdcdcdcd ) ) {
			common->Warning( "%s%s::%s%s references uninitialized memory", prefix, scope, varName, "" );
			return typeSize;
		}
		if ( varPtr != NULL  ) {
			varPtr = *((void **)varPtr);
		}
		isPointer++;
	}

	if ( varPtr == NULL ) {
		Write( varName, varType, scope, prefix, "", "<NULL>", varPtr, 0 );
		return sizeof( void * );
	}

	typeSrc.LoadMemory( typeString, typeString.Length(), varName );

	if ( !typeSrc.ReadToken( &token ) ) {
		Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
		return -1;
	}

	// get full type
	while( typeSrc.CheckTokenString( "::" ) ) {
		idToken newToken;
		typeSrc.ExpectTokenType( TT_NAME, 0, &newToken );
		token += "::" + newToken;
	}

	if ( token == "signed" ) {

		if ( !typeSrc.ReadToken( &token ) ) {
			Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
			return -1;
		}
		if ( token == "char" ) {

			typeSize = sizeof( signed char );
			Write( varName, varType, scope, prefix, "", va( "%d", *((signed char *)varPtr) ), varPtr, typeSize );

		} else if ( token == "short" ) {

			typeSize = sizeof( signed short );
			Write( varName, varType, scope, prefix, "", va( "%d", *((signed short *)varPtr) ), varPtr, typeSize );

		} else if ( token == "int" ) {

			typeSize = sizeof( signed int );
			Write( varName, varType, scope, prefix, "", va( "%d", *((signed int *)varPtr) ), varPtr, typeSize );

		} else if ( token == "long" ) {

			typeSize = sizeof( signed long );
			Write( varName, varType, scope, prefix, "", va( "%ld", *((signed long *)varPtr) ), varPtr, typeSize );

		} else {

			Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
			return -1;
		}

	} else if ( token == "unsigned" ) {

		if ( !typeSrc.ReadToken( &token ) ) {
			Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
			return -1;
		}
		if ( token == "char" ) {

			typeSize = sizeof( unsigned char );
			Write( varName, varType, scope, prefix, "", va( "%d", *((unsigned char *)varPtr) ), varPtr, typeSize );

		} else if ( token == "short" ) {

			typeSize = sizeof( unsigned short );
			Write( varName, varType, scope, prefix, "", va( "%d", *((unsigned short *)varPtr) ), varPtr, typeSize );

		} else if ( token == "int" ) {

			typeSize = sizeof( unsigned int );
			Write( varName, varType, scope, prefix, "", va( "%d", *((unsigned int *)varPtr) ), varPtr, typeSize );

		} else if ( token == "long" ) {

			typeSize = sizeof( unsigned long );
			Write( varName, varType, scope, prefix, "", va( "%lu", *((unsigned long *)varPtr) ), varPtr, typeSize );

		} else {

			Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), varPtr, 0 );
			return -1;
		}

	} else if ( token == "byte" ) {

		typeSize = sizeof( byte );
		Write( varName, varType, scope, prefix, "", va( "%d", *((byte *)varPtr) ), varPtr, typeSize );

	} else if ( token == "word" ) {

		typeSize = sizeof( word );
		Write( varName, varType, scope, prefix, "", va( "%d", *((word *)varPtr) ), varPtr, typeSize );

	} else if ( token == "dword" ) {

		typeSize = sizeof( dword );
		Write( varName, varType, scope, prefix, "", va( "%d", *((dword *)varPtr) ), varPtr, typeSize );

	} else if ( token == "bool" ) {

		typeSize = sizeof( bool );
		Write( varName, varType, scope, prefix, "", va( "%d", *((bool *)varPtr) ), varPtr, typeSize );

	} else if ( token == "char" ) {

		typeSize = sizeof( char );
		Write( varName, varType, scope, prefix, "", va( "%d", *((char *)varPtr) ), varPtr, typeSize );

	} else if ( token == "short" ) {

		typeSize = sizeof( short );
		Write( varName, varType, scope, prefix, "", va( "%d", *((short *)varPtr) ), varPtr, typeSize );

	} else if ( token == "int" ) {

		typeSize = sizeof( int );
		Write( varName, varType, scope, prefix, "", va( "%d", *((int *)varPtr) ), varPtr, typeSize );

	} else if ( token == "long" ) {

		typeSize = sizeof( long );
		Write( varName, varType, scope, prefix, "", va( "%ld", *((long *)varPtr) ), varPtr, typeSize );

	} else if ( token == "float" ) {

		typeSize = sizeof( float );
		Write( varName, varType, scope, prefix, "", idStr( *((float *)varPtr) ).c_str(), varPtr, typeSize );

	} else if ( token == "double" ) {

		typeSize = sizeof( double );
		Write( varName, varType, scope, prefix, "", idStr( (float)*((double *)varPtr) ).c_str(), varPtr, typeSize );

	} else if ( token == "idVec2" ) {

		typeSize = sizeof( idVec2 );
		Write( varName, varType, scope, prefix, "", ((idVec2 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idVec3" ) {

		typeSize = sizeof( idVec3 );
		Write( varName, varType, scope, prefix, "", ((idVec3 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idVec4" ) {

		typeSize = sizeof( idVec4 );
		Write( varName, varType, scope, prefix, "", ((idVec4 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idVec5" ) {

		typeSize = sizeof( idVec5 );
		Write( varName, varType, scope, prefix, "", ((idVec5 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idVec6" ) {

		typeSize = sizeof( idVec6 );
		Write( varName, varType, scope, prefix, "", ((idVec6 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idVecX" ) {

		const idVecX *vec = ((idVecX *)varPtr);
		if ( vec->ToFloatPtr() != NULL ) {
			Write( varName, varType, scope, prefix, "", vec->ToString( 8 ), vec->ToFloatPtr(), vec->GetSize() * sizeof( float ) );
		} else {
			Write( varName, varType, scope, prefix, "", "<NULL>", varPtr, 0 );
		}
		typeSize = sizeof( idVecX );

	} else if ( token == "idMat2" ) {

		typeSize = sizeof( idMat2 );
		Write( varName, varType, scope, prefix, "", ((idMat2 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idMat3" ) {

		typeSize = sizeof( idMat3 );
		Write( varName, varType, scope, prefix, "", ((idMat3 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idMat4" ) {

		typeSize = sizeof( idMat4 );
		Write( varName, varType, scope, prefix, "", ((idMat4 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idMat5" ) {

		typeSize = sizeof( idMat5 );
		Write( varName, varType, scope, prefix, "", ((idMat5 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idMat6" ) {

		typeSize = sizeof( idMat6 );
		Write( varName, varType, scope, prefix, "", ((idMat6 *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idMatX" ) {

		typeSize = sizeof( idMatX );
		const idMatX *mat = ((idMatX *)varPtr);
		if ( mat->ToFloatPtr() != NULL ) {
			Write( varName, varType, scope, prefix, "", mat->ToString( 8 ), mat->ToFloatPtr(), mat->GetNumColumns() * mat->GetNumRows() * sizeof( float ) );
		} else {
			Write( varName, varType, scope, prefix, "", "<NULL>", NULL, 0 );
		}

	} else if ( token == "idAngles" ) {

		typeSize = sizeof( idAngles );
		Write( varName, varType, scope, prefix, "", ((idAngles *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idQuat" ) {

		typeSize = sizeof( idQuat );
		Write( varName, varType, scope, prefix, "", ((idQuat *)varPtr)->ToString( 8 ), varPtr, typeSize );

	} else if ( token == "idBounds" ) {

		typeSize = sizeof( idBounds );
		const idBounds *bounds = ((idBounds *)varPtr);
		if ( bounds->IsCleared() ) {
			Write( varName, varType, scope, prefix, "", "<cleared>", varPtr, typeSize );
		} else {
			Write( varName, varType, scope, prefix, "", va( "(%s)-(%s)", (*bounds)[0].ToString( 8 ), (*bounds)[1].ToString( 8 ) ), varPtr, typeSize );
		}

	} else if ( token == "idList" ) {

		idList<int> *list = ((idList<int> *)varPtr);
		Write( varName, varType, scope, prefix, ".num", va( "%d", list->Num() ), NULL, 0 );
		// NOTE: we don't care about the amount of memory allocated
		//Write( varName, varType, scope, prefix, ".size", va( "%d", list->Size() ), NULL, 0 );
		Write( varName, varType, scope, prefix, ".granularity", va( "%d", list->GetGranularity() ), NULL, 0 );

		if ( list->Num() && ParseTemplateArguments( typeSrc, templateArgs ) ) {
			void *listVarPtr = list->Ptr();
			for ( i = 0; i < list->Num(); i++ ) {
				idStr listVarName = va( "%s[%d]", varName, i );
				int size = WriteVariable_r( listVarPtr, listVarName, templateArgs, scope, prefix, pointerDepth );
				if ( size == -1 ) {
					break;
				}
				listVarPtr = (void *)( ( (byte *) listVarPtr ) + size );
			}
		}

		typeSize = sizeof( idList<int> );

	} else if ( token == "idStaticList" ) {

		idStaticList<int, 1> *list = ((idStaticList<int, 1> *)varPtr);
		Write( varName, varType, scope, prefix, ".num", va( "%d", list->Num() ), NULL, 0 );

		int totalSize = 0;
		if ( list->Num() && ParseTemplateArguments( typeSrc, templateArgs ) ) {
			void *listVarPtr = list->Ptr();
			for ( i = 0; i < list->Num(); i++ ) {
				idStr listVarName = va( "%s[%d]", varName, i );
				int size = WriteVariable_r( listVarPtr, listVarName, templateArgs, scope, prefix, pointerDepth );
				if ( size == -1 ) {
					break;
				}
				totalSize += size;
				listVarPtr = (void *)( ( (byte *) listVarPtr ) + size );
			}
		}

		typeSize = sizeof( int ) + totalSize;

	} else if ( token == "idLinkList" ) {

		// FIXME: implement
		typeSize = sizeof( idLinkList<idEntity> );
		Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), NULL, 0 );

	} else if ( token == "idStr" ) {

		typeSize = sizeof( idStr );

		const idStr *str = ((idStr *)varPtr);
		Write( varName, varType, scope, prefix, "", OutputString( str->c_str() ), str->c_str(), str->Length() );

	} else if ( token == "idStrList" ) {

		typeSize = sizeof( idStrList );

		const idStrList *list = ((idStrList *)varPtr);
		if ( list->Num() ) {
			for ( i = 0; i < list->Num(); i++ ) {
				Write( varName, varType, scope, prefix, va("[%d]", i ), OutputString( (*list)[i].c_str() ), (*list)[i].c_str(), (*list)[i].Length() );
			}
		} else {
			Write( varName, varType, scope, prefix, "", "<empty>", NULL, 0 );
		}

	} else if ( token == "idDict" ) {

		typeSize = sizeof( idDict );

		const idDict *dict = ((idDict *)varPtr);
		if ( dict->GetNumKeyVals() ) {
			for ( i = 0; i < dict->GetNumKeyVals(); i++ ) {
				const idKeyValue *kv = dict->GetKeyVal( i );
				Write( varName, varType, scope, prefix, va("[%d]", i ), va( "\'%s\'  \'%s\'", OutputString( kv->GetKey().c_str() ), OutputString( kv->GetValue().c_str() ) ), NULL, 0 );
			}
		} else {
			Write( varName, varType, scope, prefix, "", "<empty>", NULL, 0 );
		}

	} else if ( token == "idExtrapolate" ) {

		const idExtrapolate<float> *interpolate = ((idExtrapolate<float> *)varPtr);
		Write( varName, varType, scope, prefix, ".extrapolationType", idStr( interpolate->GetExtrapolationType() ).c_str(), &interpolate->extrapolationType, sizeof( interpolate->extrapolationType ) );
		Write( varName, varType, scope, prefix, ".startTime", idStr( interpolate->GetStartTime() ).c_str(), &interpolate->startTime, sizeof( interpolate->startTime ) );
		Write( varName, varType, scope, prefix, ".duration", idStr( interpolate->GetDuration() ).c_str(), &interpolate->duration, sizeof( interpolate->duration ) );

		if ( ParseTemplateArguments( typeSrc, templateArgs ) ) {
			if ( templateArgs == "int" ) {
				const idExtrapolate<int> *interpolate = ((idExtrapolate<int> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", idStr( interpolate->GetStartValue() ).c_str(), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".baseSpeed", idStr( interpolate->GetBaseSpeed() ).c_str(), &interpolate->baseSpeed, sizeof( interpolate->baseSpeed ) );
				Write( varName, varType, scope, prefix, ".speed", idStr( interpolate->GetSpeed() ).c_str(), &interpolate->speed, sizeof( interpolate->speed ) );
				typeSize = sizeof( idExtrapolate<int> );
			} else if ( templateArgs == "float" ) {
				const idExtrapolate<float> *interpolate = ((idExtrapolate<float> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", idStr( interpolate->GetStartValue() ).c_str(), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".baseSpeed", idStr( interpolate->GetBaseSpeed() ).c_str(), &interpolate->baseSpeed, sizeof( interpolate->baseSpeed ) );
				Write( varName, varType, scope, prefix, ".speed", idStr( interpolate->GetSpeed() ).c_str(), &interpolate->speed, sizeof( interpolate->speed ) );
				typeSize = sizeof( idExtrapolate<float> );
			} else if ( templateArgs == "idVec3" ) {
				const idExtrapolate<idVec3> *interpolate = ((idExtrapolate<idVec3> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", interpolate->GetStartValue().ToString( 8 ), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".baseSpeed", interpolate->GetBaseSpeed().ToString( 8 ), &interpolate->baseSpeed, sizeof( interpolate->baseSpeed ) );
				Write( varName, varType, scope, prefix, ".speed", interpolate->GetSpeed().ToString( 8 ), &interpolate->speed, sizeof( interpolate->speed ) );
				typeSize = sizeof( idExtrapolate<idVec3> );
			} else if ( templateArgs == "idAngles" ) {
				const idExtrapolate<idAngles> *interpolate = ((idExtrapolate<idAngles> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", interpolate->GetStartValue().ToString( 8 ), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".baseSpeed", interpolate->GetBaseSpeed().ToString( 8 ), &interpolate->baseSpeed, sizeof( interpolate->baseSpeed ) );
				Write( varName, varType, scope, prefix, ".speed", interpolate->GetSpeed().ToString( 8 ), &interpolate->speed, sizeof( interpolate->speed ) );
				typeSize = sizeof( idExtrapolate<idAngles> );
			} else {
				Write( varName, varType, scope, prefix, "", va( "<unknown template argument type '%s' for idExtrapolate>", templateArgs.c_str() ), NULL, 0 );
			}
		}

	} else if ( token == "idInterpolate" ) {

		const idInterpolate<float> *interpolate = ((idInterpolate<float> *)varPtr);
		Write( varName, varType, scope, prefix, ".startTime", idStr( interpolate->GetStartTime() ).c_str(), &interpolate->startTime, sizeof( interpolate->startTime ) );
		Write( varName, varType, scope, prefix, ".duration", idStr( interpolate->GetDuration() ).c_str(), &interpolate->duration, sizeof( interpolate->duration ) );

		if ( ParseTemplateArguments( typeSrc, templateArgs ) ) {
			if ( templateArgs == "int" ) {
				const idInterpolate<int> *interpolate = ((idInterpolate<int> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", idStr( interpolate->GetStartValue() ).c_str(), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".endValue", idStr( interpolate->GetEndValue() ).c_str(), &interpolate->endValue, sizeof( interpolate->endValue ) );
				typeSize = sizeof( idInterpolate<int> );
			} else if ( templateArgs == "float" ) {
				const idInterpolate<float> *interpolate = ((idInterpolate<float> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", idStr( interpolate->GetStartValue() ).c_str(), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".endValue", idStr( interpolate->GetEndValue() ).c_str(), &interpolate->endValue, sizeof( interpolate->endValue ) );
				typeSize = sizeof( idInterpolate<float> );
			} else {
				Write( varName, varType, scope, prefix, "", va( "<unknown template argument type '%s' for idInterpolate>", templateArgs.c_str() ), NULL, 0 );
			}
		}

	} else if ( token == "idInterpolateAccelDecelLinear" ) {

		const idInterpolateAccelDecelLinear<float> *interpolate = ((idInterpolateAccelDecelLinear<float> *)varPtr);
		Write( varName, varType, scope, prefix, ".startTime", idStr( interpolate->GetStartTime() ).c_str(), &interpolate->startTime, sizeof( interpolate->startTime ) );
		Write( varName, varType, scope, prefix, ".accelTime", idStr( interpolate->GetAcceleration() ).c_str(), &interpolate->accelTime, sizeof( interpolate->accelTime ) );
		Write( varName, varType, scope, prefix, ".linearTime", idStr( interpolate->linearTime ).c_str(), &interpolate->linearTime, sizeof( interpolate->linearTime ) );
		Write( varName, varType, scope, prefix, ".decelTime", idStr( interpolate->GetDeceleration() ).c_str(), &interpolate->decelTime, sizeof( interpolate->decelTime ) );

		if ( ParseTemplateArguments( typeSrc, templateArgs ) ) {
			if ( templateArgs == "int" ) {
				const idInterpolateAccelDecelLinear<int> *interpolate = ((idInterpolateAccelDecelLinear<int> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", idStr( interpolate->GetStartValue() ).c_str(), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".endValue", idStr( interpolate->GetEndValue() ).c_str(), &interpolate->endValue, sizeof( interpolate->endValue ) );
				typeSize = sizeof( idInterpolateAccelDecelLinear<int> );
			} else if ( templateArgs == "float" ) {
				const idInterpolateAccelDecelLinear<float> *interpolate = ((idInterpolateAccelDecelLinear<float> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", idStr( interpolate->GetStartValue() ).c_str(), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".endValue", idStr( interpolate->GetEndValue() ).c_str(), &interpolate->endValue, sizeof( interpolate->endValue ) );
				typeSize = sizeof( idInterpolateAccelDecelLinear<float> );
			} else {
				Write( varName, varType, scope, prefix, "", va( "<unknown template argument type '%s' for idInterpolateAccelDecelLinear>", templateArgs.c_str() ), NULL, 0 );
			}
		}

	} else if ( token == "idInterpolateAccelDecelSine" ) {

		const idInterpolateAccelDecelSine<float> *interpolate = ((idInterpolateAccelDecelSine<float> *)varPtr);
		Write( varName, varType, scope, prefix, ".startTime", idStr( interpolate->GetStartTime() ).c_str(), &interpolate->startTime, sizeof( interpolate->startTime ) );
		Write( varName, varType, scope, prefix, ".accelTime", idStr( interpolate->GetAcceleration() ).c_str(), &interpolate->accelTime, sizeof( interpolate->accelTime ) );
		Write( varName, varType, scope, prefix, ".linearTime", idStr( interpolate->linearTime ).c_str(), &interpolate->linearTime, sizeof( interpolate->linearTime ) );
		Write( varName, varType, scope, prefix, ".decelTime", idStr( interpolate->GetDeceleration() ).c_str(), &interpolate->decelTime, sizeof( interpolate->decelTime ) );

		if ( ParseTemplateArguments( typeSrc, templateArgs ) ) {
			if ( templateArgs == "int" ) {
				const idInterpolateAccelDecelSine<int> *interpolate = ((idInterpolateAccelDecelSine<int> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", idStr( interpolate->GetStartValue() ).c_str(), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".endValue", idStr( interpolate->GetEndValue() ).c_str(), &interpolate->endValue, sizeof( interpolate->endValue ) );
				typeSize = sizeof( idInterpolateAccelDecelSine<int> );
			} else if ( templateArgs == "float" ) {
				const idInterpolateAccelDecelSine<float> *interpolate = ((idInterpolateAccelDecelSine<float> *)varPtr);
				Write( varName, varType, scope, prefix, ".startValue", idStr( interpolate->GetStartValue() ).c_str(), &interpolate->startValue, sizeof( interpolate->startValue ) );
				Write( varName, varType, scope, prefix, ".endValue", idStr( interpolate->GetEndValue() ).c_str(), &interpolate->endValue, sizeof( interpolate->endValue ) );
				typeSize = sizeof( idInterpolateAccelDecelSine<float> );
			} else {
				Write( varName, varType, scope, prefix, "", va( "<unknown template argument type '%s' for idInterpolateAccelDecelSine>", templateArgs.c_str() ), NULL, 0 );
			}
		}

	} else if ( token == "idUserInterface" ) {

		typeSize = sizeof( idUserInterface );
		const idUserInterface *gui = ((idUserInterface *)varPtr);
		Write( varName, varType, scope, prefix, "", gui->Name(), varPtr, sizeof( varPtr ) );

	} else if ( token == "idRenderModel" ) {

		typeSize = sizeof( idRenderModel );
		const idRenderModel *model = ((idRenderModel *)varPtr);
		Write( varName, varType, scope, prefix, "", model->Name(), varPtr, sizeof( varPtr ) );

	} else if ( token == "qhandle_t" ) {

		typeSize = sizeof( int );
		Write( varName, varType, scope, prefix, "", va( "%d", *((int *)varPtr) ), varPtr, typeSize );

	} else if ( token == "cmHandle_t" ) {

		typeSize = sizeof( int );
		Write( varName, varType, scope, prefix, "", va( "%d", *((int *)varPtr) ), varPtr, typeSize );

	} else if ( token == "idEntityPtr" ) {

		typeSize = sizeof( idEntityPtr<idEntity> );

		const idEntityPtr<idEntity> *entPtr = ((idEntityPtr<idEntity> *)varPtr);
		if ( entPtr->GetEntity() ) {
			idEntity *entity = entPtr->GetEntity();
			Write( varName, varType, scope, prefix, ".", va( "entity %d: \'%s\'", entity->entityNumber, entity->name.c_str() ), varPtr, typeSize );
		} else {
			Write( varName, varType, scope, prefix, "", "<NULL>", varPtr, typeSize );
		}

	} else if ( token == "idEntity::entityFlags_s" ) {

		const idEntity::entityFlags_s *flags = ((idEntity::entityFlags_s *)varPtr);
		Write( varName, varType, scope, prefix, ".notarget", flags->notarget ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".noknockback", flags->noknockback ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".takedamage", flags->takedamage ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".hidden", flags->hidden ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".bindOrientated", flags->bindOrientated ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".solidForTeam", flags->solidForTeam ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".forcePhysicsUpdate", flags->forcePhysicsUpdate ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".selected", flags->selected ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".neverDormant", flags->neverDormant ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".isDormant", flags->isDormant ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".hasAwakened", flags->hasAwakened ? "true" : "false", NULL, 0 );
		Write( varName, varType, scope, prefix, ".networkSync", flags->networkSync ? "true" : "false", NULL, 0 );
		typeSize = sizeof( idEntity::entityFlags_s );

	} else if ( token == "idScriptBool" ) {

		typeSize = sizeof( idScriptBool );

		const idScriptBool *scriptBool = ((idScriptBool *)varPtr);
		if ( scriptBool->IsLinked() ) {
			Write( varName, varType, scope, prefix, "", ( *scriptBool != 0 ) ? "true" : "false", varPtr, typeSize );
		} else {
			Write( varName, varType, scope, prefix, "", "<not linked>", varPtr, typeSize );
		}

	} else {

		const classTypeInfo_t *classTypeInfo = FindClassInfo( scope + ( "::" + token ) );
		if ( classTypeInfo == NULL ) {
			classTypeInfo = FindClassInfo( token );
		}
		if ( classTypeInfo != NULL ) {

			typeSize = classTypeInfo->size;

			if ( !isPointer ) {

				char newPrefix[1024];
				idStr::snPrintf( newPrefix, sizeof( newPrefix ), "%s%s::%s.", prefix, scope, varName );
				WriteClass_r( varPtr, "", token, token, newPrefix, pointerDepth );

			} else if ( token == "idAnim" ) {

				const idAnim *anim = ((idAnim*)varPtr);
				Write( varName, varType, scope, prefix, "", anim->Name(), NULL, 0 );

			} else if ( token == "idPhysics" ) {

				const idPhysics *physics = ((idPhysics*)varPtr);
				Write( varName, varType, scope, prefix, "", physics->GetType()->classname, NULL, 0 );

			} else if ( IsSubclassOf( token, "idEntity" ) ) {

				const idEntity *entity = ((idEntity*)varPtr);
				Write( varName, varType, scope, prefix, "", va( "entity %d: \'%s\'", entity->entityNumber, entity->name.c_str() ), NULL, 0 );

			} else if ( IsSubclassOf( token, "idDecl" ) ) {

				const idDecl *decl = ((idDecl *)varPtr);
				Write( varName, varType, scope, prefix, "", decl->GetName(), NULL, 0 );

			} else if ( pointerDepth == 0 && (
						token == "idAFBody" ||
						token == "idAFTree" ||
						token == "idClipModel" ||
						IsSubclassOf( token, "idAFConstraint" )
						) ) {

				char newPrefix[1024];
				idStr::snPrintf( newPrefix, sizeof( newPrefix ), "%s%s::%s->", prefix, scope, varName );
				WriteClass_r( varPtr, "", token, token, newPrefix, pointerDepth + 1 );

			} else {

				Write( varName, varType, scope, prefix, "", va( "<pointer type '%s' not listed>", varType ), NULL, 0 );
				return -1;
			}
		} else {
			const enumTypeInfo_t *enumTypeInfo = FindEnumInfo( scope + ( "::" + token ) );
			if ( enumTypeInfo == NULL ) {
				enumTypeInfo = FindEnumInfo( token );
			}
			if ( enumTypeInfo != NULL ) {

				typeSize = sizeof( int );	// NOTE: assuming sizeof( enum ) is sizeof( int )

				for ( i = 0; enumTypeInfo->values[i].name != NULL; i++ ) {
					if ( *((int *)varPtr) == enumTypeInfo->values[i].value ) {
						break;
					}
				}
				if ( enumTypeInfo->values[i].name != NULL ) {
					Write( varName, varType, scope, prefix, "", enumTypeInfo->values[i].name, NULL, 0 );
				} else {
					Write( varName, varType, scope, prefix, "", va( "%d", *((int *)varPtr) ), NULL, 0 );
				}

			} else {
				Write( varName, varType, scope, prefix, "", va( "<unknown type '%s'>", varType ), NULL, 0 );
				return -1;
			}
		}
	}

	i = 0;
	do {
		if ( *((unsigned long *)varPtr) == 0xcdcdcdcd ) {
			common->Warning( "%s%s::%s%s uses uninitialized memory", prefix, scope, varName, "" );
			break;
		}
	} while( ++i < typeSize );

	if ( isPointer ) {
		return sizeof( void * );
	}
	return typeSize;
}

/*
================
idTypeInfoTools::WriteClass_r
================
*/
void idTypeInfoTools::WriteClass_r( const void *classPtr, const char *className, const char *classType, const char *scope, const char *prefix, const int pointerDepth ) {
	int i;

	const classTypeInfo_t *classInfo = FindClassInfo( classType );
	if ( !classInfo ) {
		return;
	}
	if ( *classInfo->superType != '\0' ) {
		WriteClass_r( classPtr, className, classInfo->superType, scope, prefix, pointerDepth );
	}

	for ( i = 0; classInfo->variables[i].name != NULL; i++ ) {
		const classVariableInfo_t &classVar = classInfo->variables[i];

		void *varPtr = (void *) (((byte *)classPtr) + classVar.offset);

		WriteVariable_r( varPtr, classVar.name, classVar.type, classType, prefix, pointerDepth );
	}
}

/*
================
idTypeInfoTools::WriteGameState
================
*/
void idTypeInfoTools::WriteGameState( const char *fileName ) {
	int i, num;
	idFile *file;

	file = fileSystem->OpenFileWrite( fileName );
	if ( !file ) {
		common->Warning( "couldn't open %s", fileName );
		return;
	}

	fp = file;
	Write = WriteGameStateVariable; //WriteVariable;

#ifdef DUMP_GAMELOCAL

	file->WriteFloatString( "\ngameLocal {\n" );
	WriteClass_r( (void *)&gameLocal, "", "idGameLocal", "idGameLocal", "", 0 );
	file->WriteFloatString( "}\n" );

#endif

	for ( num = i = 0; i < gameLocal.num_entities; i++ ) {
		idEntity *ent = gameLocal.entities[i];
		if ( ent == NULL ) {
			continue;
		}
		file->WriteFloatString( "\nentity %d %s {\n", i, ent->GetType()->classname );
		WriteClass_r( (void *)ent, "", ent->GetType()->classname, ent->GetType()->classname, "", 0 );
		file->WriteFloatString( "}\n" );
		num++;
	}

	fileSystem->CloseFile( file );

	common->Printf( "%d entities written\n", num );
}

/*
================
idTypeInfoTools::CompareGameState
================
*/
void idTypeInfoTools::CompareGameState( const char *fileName ) {
	int entityNum;
	idToken token;

	src = new idLexer();
	src->SetFlags( LEXFL_NOSTRINGESCAPECHARS );

	if ( !src->LoadFile( fileName ) ) {
		common->Warning( "couldn't load %s", fileName );
		delete src;
		src = NULL;
		return;
	}

	fp = NULL;
	Write = VerifyVariable;

#ifdef DUMP_GAMELOCAL

	if ( !src->ExpectTokenString( "gameLocal" ) || !src->ExpectTokenString( "{" ) ) {
		delete src;
		src = NULL;
		return;
	}

	WriteClass_r( (void *)&gameLocal, "", "idGameLocal", "idGameLocal", "", 0 );

	if ( !src->ExpectTokenString( "}" ) ) {
		delete src;
		src = NULL;
		return;
	}

#endif

	while( src->ReadToken( &token ) ) {
		if ( token != "entity" ) {
			break;
		}
		if ( !src->ExpectTokenType( TT_NUMBER, TT_INTEGER, &token ) ) {
			break;
		}

		entityNum = token.GetIntValue();

		if ( entityNum < 0 || entityNum >= gameLocal.num_entities ) {
			src->Warning( "entity number %d out of range", entityNum );
			break;
		}

		typeError = false;

		idEntity *ent = gameLocal.entities[entityNum];
		if ( !ent ) {
			src->Warning( "entity %d is not spawned", entityNum );
			src->SkipBracedSection( true );
			continue;
		}

		if ( !src->ExpectTokenType( TT_NAME, 0, &token ) ) {
			break;
		}

		if ( token.Cmp( ent->GetType()->classname ) != 0 ) {
			src->Warning( "entity %d has wrong type", entityNum );
			src->SkipBracedSection( true );
			continue;
		}

		if ( !src->ExpectTokenString( "{" ) ) {
			src->Warning( "entity %d missing leading {", entityNum );
			break;
		}

		WriteClass_r( (void *)ent, "", ent->GetType()->classname, ent->GetType()->classname, "", 0 );

		if ( !src->SkipBracedSection( false ) ) {
			src->Warning( "entity %d missing trailing }", entityNum );
			break;
		}
	}

	delete src;
	src = NULL;
}

/*
================
WriteGameState_f
================
*/
void WriteGameState_f( const idCmdArgs &args ) {
	idStr fileName;

	if ( args.Argc() > 1 ) {
		fileName = args.Argv(1);
	} else {
		fileName = "GameState.txt";
	}
	fileName.SetFileExtension( "gameState.txt" );

	idTypeInfoTools::WriteGameState( fileName );
}

/*
================
CompareGameState_f
================
*/
void CompareGameState_f( const idCmdArgs &args ) {
	idStr fileName;

	if ( args.Argc() > 1 ) {
		fileName = args.Argv(1);
	} else {
		fileName = "GameState.txt";
	}
	fileName.SetFileExtension( "gameState.txt" );

	idTypeInfoTools::CompareGameState( fileName );
}

/*
================
TestSaveGame_f
================
*/
void TestSaveGame_f( const idCmdArgs &args ) {
	idStr name;

	if ( args.Argc() <= 1 ) {
		gameLocal.Printf( "testSaveGame <mapName>\n" );
		return;
	}

	name = args.Argv( 1 );

	try {
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "map %s", name.c_str() ) );
		name.Replace( "\\", "_" );
		name.Replace( "/", "_" );
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "saveGame test_%s", name.c_str() ) );
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "loadGame test_%s", name.c_str() ) );
	}
	catch( idException & ) {
		// an ERR_DROP was thrown
	}
	cmdSystem->BufferCommandText( CMD_EXEC_NOW, "quit" );
}

/*
================
WriteTypeToFile
================
*/
void WriteTypeToFile( idFile *fp, const void *typePtr, const char *typeName ) {
	idTypeInfoTools::WriteTypeToFile( fp, typePtr, typeName );
}

/*
================
PrintType
================
*/
void PrintType( const void *typePtr, const char *typeName ) {
	idTypeInfoTools::PrintType( typePtr, typeName );
}

/*
================
InitTypeVariables
================
*/
void InitTypeVariables( const void *typePtr, const char *typeName, int value ) {
	idTypeInfoTools::InitTypeVariables( typePtr, typeName, value );
}

/*
================
ListTypeInfo_f
================
*/
int SortTypeInfoByName( const int *a, const int *b ) {
	return idStr::Icmp( classTypeInfo[*a].typeName, classTypeInfo[*b].typeName );
}

int SortTypeInfoBySize( const int *a, const int *b ) {
	if ( classTypeInfo[*a].size < classTypeInfo[*b].size ) {
		return -1;
	}
	if ( classTypeInfo[*a].size > classTypeInfo[*b].size ) {
		return 1;
	}
	return 0;
}

void ListTypeInfo_f( const idCmdArgs &args ) {
	int i, j;
	idList<int> index;

	common->Printf( "%-32s : %-32s size (B)\n", "type name", "super type name" );
	for ( i = 0; classTypeInfo[i].typeName != NULL; i++ ) {
		index.Append( i );
	}

	if ( args.Argc() > 1 && idStr::Icmp( args.Argv( 1 ), "size" ) == 0 ) {
		index.Sort( SortTypeInfoBySize );
	} else {
		index.Sort( SortTypeInfoByName );
	}

	for ( i = 0; classTypeInfo[i].typeName != NULL; i++ ) {
		j = index[i];
		common->Printf( "%-32s : %-32s %d\n", classTypeInfo[j].typeName, classTypeInfo[j].superType, classTypeInfo[j].size );
	}
}

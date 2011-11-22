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

#include "Maya5.0/maya.h"
//#include "Maya6.0/maya.h"			// must also change include directory in project from "MayaImport\Maya4.5\include" to "MayaImport\Maya6.0\include" (requires MSDev 7.1)
#include "exporter.h"
#include "maya_main.h"

idStr	errorMessage;
bool	initialized = false;

#define DEFAULT_ANIM_EPSILON	0.125f
#define DEFAULT_QUAT_EPSILON	( 1.0f / 8192.0f )

#define SLOP_VERTEX				0.01f			// merge xyz coordinates this far apart
#define	SLOP_TEXCOORD			0.001f			// merge texture coordinates this far apart

const char *componentNames[ 6 ] = { "Tx", "Ty", "Tz", "Qx", "Qy", "Qz" };

idSys *			sys = NULL;
idCommon *		common = NULL;
idCVarSystem *	cvarSystem = NULL;

idCVar *		idCVar::staticVars = NULL;

/*
=================
MayaError
=================
*/
void MayaError( const char *fmt, ... ) {
	va_list	argptr;
	char	text[ 8192 ];

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	throw idException( text );
}

/*
=================
FS_WriteFloatString
=================
*/
#define	MAX_PRINT_MSG	4096
static int WriteFloatString( FILE *file, const char *fmt, ... ) {
	long i;
	unsigned long u;
	double f;
	char *str;
	int index;
	idStr tmp, format;
	va_list argPtr;

	va_start( argPtr, fmt );

	index = 0;

	while( *fmt ) {
		switch( *fmt ) {
			case '%':
				format = "";
				format += *fmt++;
				while ( (*fmt >= '0' && *fmt <= '9') ||
						*fmt == '.' || *fmt == '-' || *fmt == '+' || *fmt == '#') {
					format += *fmt++;
				}
				format += *fmt;
				switch( *fmt ) {
					case 'f':
					case 'e':
					case 'E':
					case 'g':
					case 'G':
						f = va_arg( argPtr, double );
						if ( format.Length() <= 2 ) {
							// high precision floating point number without trailing zeros
							sprintf( tmp, "%1.10f", f );
							tmp.StripTrailing( '0' );
							tmp.StripTrailing( '.' );
							index += fprintf( file, "%s", tmp.c_str() );
						}
						else {
							index += fprintf( file, format.c_str(), f );
						}
						break;
					case 'd':
					case 'i':
						i = va_arg( argPtr, long );
						index += fprintf( file, format.c_str(), i );
						break;
					case 'u':
						u = va_arg( argPtr, unsigned long );
						index += fprintf( file, format.c_str(), u );
						break;
					case 'o':
						u = va_arg( argPtr, unsigned long );
						index += fprintf( file, format.c_str(), u );
						break;
					case 'x':
						u = va_arg( argPtr, unsigned long );
						index += fprintf( file, format.c_str(), u );
						break;
					case 'X':
						u = va_arg( argPtr, unsigned long );
						index += fprintf( file, format.c_str(), u );
						break;
					case 'c':
						i = va_arg( argPtr, long );
						index += fprintf( file, format.c_str(), (char) i );
						break;
					case 's':
						str = va_arg( argPtr, char * );
						index += fprintf( file, format.c_str(), str );
						break;
					case '%':
						index += fprintf( file, format.c_str() );
						break;
					default:
						MayaError( "WriteFloatString: invalid format %s", format.c_str() );
						break;
				}
				fmt++;
				break;
			case '\\':
				fmt++;
				switch( *fmt ) {
					case 't':
						index += fprintf( file, "\t" );
						break;
					case 'n':
						index += fprintf( file, "\n" );
					default:
						MayaError( "WriteFloatString: unknown escape character \'%c\'", *fmt );
						break;
				}
				fmt++;
				break;
			default:
				index += fprintf( file, "%c", *fmt );
				fmt++;
				break;
		}
	}

	va_end( argPtr );

	return index;
}

/*
================
OSPathToRelativePath

takes a full OS path, as might be found in data from a media creation
program, and converts it to a qpath by stripping off directories

Returns false if the osPath tree doesn't match any of the existing
search paths.
================
*/
bool OSPathToRelativePath( const char *osPath, idStr &qpath, const char *game ) {
	char *s, *base;

	// skip a drive letter?

	// search for anything with BASE_GAMEDIR in it
	// Ase files from max may have the form of:
	// "//Purgatory/purgatory/doom/base/models/mapobjects/bitch/hologirl.tga"
	// which won't match any of our drive letter based search paths
	base = (char *)strstr( osPath, BASE_GAMEDIR );

	// _D3XP added mod support
	if ( base == NULL && strlen(game) > 0 ) {

		base = s = (char *)strstr( osPath, game );

		while( s = strstr( s, game ) ) {
			s += strlen( game );
			if ( s[0] == '/' || s[0] == '\\' ) {
				base = s;
			}
		}
	} 

	if ( base ) {
		s = strstr( base, "/" );
		if ( !s ) {
			s = strstr( base, "\\" );
		}
		if ( s ) {
			qpath = s + 1;
			return true;
		}
	}

	common->Printf( "OSPathToRelativePath failed on %s\n", osPath );
	qpath = osPath;

	return false;
}

/*
===============
ConvertFromIdSpace
===============
*/
idMat3 ConvertFromIdSpace( const idMat3 &idmat ) {
	idMat3 mat;

	mat[ 0 ][ 0 ] = idmat[ 0 ][ 0 ];
	mat[ 0 ][ 2 ] = -idmat[ 0 ][ 1 ];
	mat[ 0 ][ 1 ] = idmat[ 0 ][ 2 ];
						
	mat[ 1 ][ 0 ] = idmat[ 1 ][ 0 ];
	mat[ 1 ][ 2 ] = -idmat[ 1 ][ 1 ];
	mat[ 1 ][ 1 ] = idmat[ 1 ][ 2 ];
						
	mat[ 2 ][ 0 ] = idmat[ 2 ][ 0 ];
	mat[ 2 ][ 2 ] = -idmat[ 2 ][ 1 ];
	mat[ 2 ][ 1 ] = idmat[ 2 ][ 2 ];

	return mat;
}

/*
===============
ConvertFromIdSpace
===============
*/
idVec3 ConvertFromIdSpace( const idVec3 &idpos ) {
	idVec3 pos;

	pos.x = idpos.x;
	pos.z = -idpos.y;
	pos.y = idpos.z;

	return pos;
}

/*
===============
ConvertToIdSpace
===============
*/
idMat3 ConvertToIdSpace( const idMat3 &mat ) {
	idMat3 idmat;

	idmat[ 0 ][ 0 ] = mat[ 0 ][ 0 ];
	idmat[ 0 ][ 1 ] = -mat[ 0 ][ 2 ];
	idmat[ 0 ][ 2 ] = mat[ 0 ][ 1 ];

	idmat[ 1 ][ 0 ] = mat[ 1 ][ 0 ];
	idmat[ 1 ][ 1 ] = -mat[ 1 ][ 2 ];
	idmat[ 1 ][ 2 ] = mat[ 1 ][ 1 ];

	idmat[ 2 ][ 0 ] = mat[ 2 ][ 0 ];
	idmat[ 2 ][ 1 ] = -mat[ 2 ][ 2 ];
	idmat[ 2 ][ 2 ] = mat[ 2 ][ 1 ];

	return idmat;
}

/*
===============
ConvertToIdSpace
===============
*/
idVec3 ConvertToIdSpace( const idVec3 &pos ) {
	idVec3 idpos;

	idpos.x = pos.x;
	idpos.y = -pos.z;
	idpos.z = pos.y;

	return idpos;
}

/*
===============
idVec
===============
*/
idVec3 idVec( const MFloatPoint &point ) {
	return idVec3( point[ 0 ], point[ 1 ], point[ 2 ] );
}

/*
===============
idVec
===============
*/
idVec3 idVec( const MMatrix &matrix ) {
	return idVec3( matrix[ 3 ][ 0 ], matrix[ 3 ][ 1 ], matrix[ 3 ][ 2 ] );
}

/*
===============
idMat
===============
*/
idMat3 idMat( const MMatrix &matrix ) {
	int		j, k;
	idMat3	mat;

	for( j = 0; j < 3; j++ ) {
		for( k = 0; k < 3; k++ ) {
			mat[ j ][ k ] = matrix[ j ][ k ];
		}
	}

	return mat;
}

/*
===============
GetParent
===============
*/
MFnDagNode *GetParent( MFnDagNode *joint ) {
	MStatus		status;
	MObject		parentObject;

	parentObject = joint->parent( 0, &status );
	if ( !status && status.statusCode() == MStatus::kInvalidParameter ) {
		return NULL;
	}

	while( !parentObject.hasFn( MFn::kTransform ) ) {
		MFnDagNode parentNode( parentObject, &status );
		if ( !status ) {
			return NULL;
		}

		parentObject = parentNode.parent( 0, &status );
		if ( !status && status.statusCode() == MStatus::kInvalidParameter ) {
			return NULL;
		}
	}

	MFnDagNode *parentNode;

	parentNode = new MFnDagNode( parentObject, &status );
	if ( !status ) {
		delete parentNode;
		return NULL;
	}

	return parentNode;
}

/*
==============================================================================================

	idTokenizer

==============================================================================================
*/

/*
====================
idTokenizer::SetTokens
====================
*/
int idTokenizer::SetTokens( const char *buffer ) {
	const char *cmd;

	Clear();

	// tokenize commandline
	cmd = buffer;
	while ( *cmd ) {
		// skip whitespace
		while( *cmd && isspace( *cmd ) ) {
			cmd++;
		}

		if ( !*cmd ) {
			break;
		}
		
		idStr &current = tokens.Alloc();
		while( *cmd && !isspace( *cmd ) ) {
			current += *cmd;
			cmd++;
		}
	}

	return tokens.Num();
}

/*
====================
idTokenizer::NextToken
====================
*/
const char *idTokenizer::NextToken( const char *errorstring ) {
	if ( currentToken < tokens.Num() ) {
		return tokens[ currentToken++ ];
	}

	if ( errorstring ) {
		MayaError( "Error: %s", errorstring );
	}

	return NULL;
}

/*
==============================================================================================

	idExportOptions

==============================================================================================
*/

/*
====================
idExportOptions::Reset
====================
*/
void idExportOptions::Reset( const char *commandline ) {
	scale			= 1.0f;
	type			= WRITE_MESH;
	startframe		= -1;
	endframe		= -1;
	ignoreMeshes	= false;
	clearOrigin		= false;
	clearOriginAxis	= false;
	framerate		= 24;
	align			= "";
	rotate			= 0.0f;
	commandLine		= commandline;
	prefix			= "";
	jointThreshold	= 0.05f;
	ignoreScale		= false;
	xyzPrecision	= DEFAULT_ANIM_EPSILON;
	quatPrecision	= DEFAULT_QUAT_EPSILON;
	cycleStart		= -1;

	src.Clear();
	dest.Clear();

	tokens.SetTokens( commandline );

	keepjoints.Clear();
	renamejoints.Clear();
	remapjoints.Clear();
	exportgroups.Clear();
	skipmeshes.Clear();
	keepmeshes.Clear();
	groups.Clear();
}

/*
====================
idExportOptions::idExportOptions
====================
*/
idExportOptions::idExportOptions( const char *commandline, const char *ospath ) {
	idStr		token;
	idNamePair	joints;
	int			i;
	idAnimGroup	*group;
	idStr		sourceDir;
	idStr		destDir;

	Reset( commandline );

	token = tokens.NextToken( "Missing export command" );
	if ( token == "mesh" ) {
		type = WRITE_MESH;
	} else if ( token == "anim" ) {
		type = WRITE_ANIM;
	} else if ( token == "camera" ) {
		type = WRITE_CAMERA;
	} else {
		MayaError( "Unknown export command '%s'", token.c_str() );
	}

	src = tokens.NextToken( "Missing source filename" );
	dest = src;

	for( token = tokens.NextToken(); token != ""; token = tokens.NextToken() ) {
		if ( token == "-force" ) {
			// skip
		} else if ( token == "-game" ) {
			// parse game name
			game = tokens.NextToken( "Expecting game name after -game" );

		} else if ( token == "-rename" ) {
			// parse joint to rename
			joints.from = tokens.NextToken( "Missing joint name for -rename.  Usage: -rename [joint name] [new name]" );
			joints.to	= tokens.NextToken( "Missing new name for -rename.  Usage: -rename [joint name] [new name]" );
			renamejoints.Append( joints );

		} else if ( token == "-prefix" ) {
			prefix = tokens.NextToken( "Missing name for -prefix.  Usage: -prefix [joint prefix]" );

		} else if ( token == "-parent" ) {
			// parse joint to reparent
			joints.from = tokens.NextToken( "Missing joint name for -parent.  Usage: -parent [joint name] [new parent]" );
			joints.to	= tokens.NextToken( "Missing new parent for -parent.  Usage: -parent [joint name] [new parent]" );
			remapjoints.Append( joints );

		} else if ( !token.Icmp( "-sourcedir" ) ) {
			// parse source directory
			sourceDir = tokens.NextToken( "Missing filename for -sourcedir.  Usage: -sourcedir [directory]" );

		} else if ( !token.Icmp( "-destdir" ) ) {
			// parse destination directory
			destDir = tokens.NextToken( "Missing filename for -destdir.  Usage: -destdir [directory]" );

		} else if ( token == "-dest" ) {
			// parse destination filename
			dest = tokens.NextToken( "Missing filename for -dest.  Usage: -dest [filename]" );

		} else if ( token == "-range" ) {
			// parse frame range to export
			token		= tokens.NextToken( "Missing start frame for -range.  Usage: -range [start frame] [end frame]" );
			startframe	= atoi( token );
			token		= tokens.NextToken( "Missing end frame for -range.  Usage: -range [start frame] [end frame]" );
			endframe	= atoi( token );

			if ( startframe > endframe ) {
				MayaError( "Start frame is greater than end frame." );
			}

		} else if ( !token.Icmp( "-cycleStart" ) ) {
			// parse start frame of cycle
			token		= tokens.NextToken( "Missing cycle start frame for -cycleStart.  Usage: -cycleStart [first frame of cycle]" );
			cycleStart	= atoi( token );

		} else if ( token == "-scale" ) {
			// parse scale
			token	= tokens.NextToken( "Missing scale amount for -scale.  Usage: -scale [scale amount]" );
			scale	= atof( token );

		} else if ( token == "-align" ) {
			// parse align joint
			align = tokens.NextToken( "Missing joint name for -align.  Usage: -align [joint name]" );

		} else if ( token == "-rotate" ) {
			// parse angle rotation
			token	= tokens.NextToken( "Missing value for -rotate.  Usage: -rotate [yaw]" );
			rotate	= -atof( token );

		} else if ( token == "-nomesh" ) {
			ignoreMeshes = true;

		} else if ( token == "-clearorigin" ) {
			clearOrigin = true;
			clearOriginAxis = true;

		} else if ( token == "-clearoriginaxis" ) {
			clearOriginAxis = true;

		} else if ( token == "-ignorescale" ) {
			ignoreScale = true;

		} else if ( token == "-xyzprecision" ) {
			// parse quaternion precision
			token = tokens.NextToken( "Missing value for -xyzprecision.  Usage: -xyzprecision [precision]" );
			xyzPrecision = atof( token );
			if ( xyzPrecision < 0.0f ) {
				MayaError( "Invalid value for -xyzprecision.  Must be >= 0" );
			}

		} else if ( token == "-quatprecision" ) {
			// parse quaternion precision
			token = tokens.NextToken( "Missing value for -quatprecision.  Usage: -quatprecision [precision]" );
			quatPrecision = atof( token );
			if ( quatPrecision < 0.0f ) {
				MayaError( "Invalid value for -quatprecision.  Must be >= 0" );
			}
		
		} else if ( token == "-jointthreshold" ) {
			// parse joint threshold
			token			= tokens.NextToken( "Missing weight for -jointthreshold.  Usage: -jointthreshold [minimum joint weight]" );
			jointThreshold	= atof( token );

		} else if ( token == "-skipmesh" ) {
			token = tokens.NextToken( "Missing name for -skipmesh.  Usage: -skipmesh [name of mesh to skip]" );
			skipmeshes.AddUnique( token );

		} else if ( token == "-keepmesh" ) {
			token = tokens.NextToken( "Missing name for -keepmesh.  Usage: -keepmesh [name of mesh to keep]" );
			keepmeshes.AddUnique( token );

		} else if ( token == "-jointgroup" ) {
			token	= tokens.NextToken( "Missing name for -jointgroup.  Usage: -jointgroup [group name] [joint1] [joint2]...[joint n]" );
			group = groups.Ptr();
			for( i = 0; i < groups.Num(); i++, group++ ) {
				if ( group->name == token ) {
					break;
				}
			}

			if ( i >= groups.Num() ) {
				// create a new group
				group = &groups.Alloc();
				group->name = token;
			}

			while( tokens.TokenAvailable() ) {
				token = tokens.NextToken();
				if ( token[ 0 ] == '-' ) {
					tokens.UnGetToken();
					break;
				}

				group->joints.AddUnique( token );
			}
		} else if ( token == "-group" ) {
			// add the list of groups to export (these don't affect the hierarchy)
			while( tokens.TokenAvailable() ) {
				token = tokens.NextToken();
				if ( token[ 0 ] == '-' ) {
					tokens.UnGetToken();
					break;
				}

				group = groups.Ptr();
				for( i = 0; i < groups.Num(); i++, group++ ) {
					if ( group->name == token ) {
						break;
					}
				}

				if ( i >= groups.Num() ) {
					MayaError( "Unknown group '%s'", token.c_str() );
				}

				exportgroups.AddUnique( group );
			}
		} else if ( token == "-keep" ) {
			// add joints that are kept whether they're used by a mesh or not
			while( tokens.TokenAvailable() ) {
				token = tokens.NextToken();
				if ( token[ 0 ] == '-' ) {
					tokens.UnGetToken();
					break;
				}
				keepjoints.AddUnique( token );
			}
		} else {
			MayaError( "Unknown option '%s'", token.c_str() );
		}
	}

	token = src;
	src = ospath;
	src.BackSlashesToSlashes();
	src.AppendPath( sourceDir );
	src.AppendPath( token );

	token = dest;
	dest = ospath;
	dest.BackSlashesToSlashes();
	dest.AppendPath( destDir );
	dest.AppendPath( token );

	// Maya only accepts unix style path separators
	src.BackSlashesToSlashes();
	dest.BackSlashesToSlashes();

	if ( skipmeshes.Num() && keepmeshes.Num() ) {
		MayaError( "Can't use -keepmesh and -skipmesh together." );
	}
}

/*
====================
idExportOptions::jointInExportGroup
====================
*/
bool idExportOptions::jointInExportGroup( const char *jointname ) {
	int			i;
	int			j;
	idAnimGroup	*group;

	if ( !exportgroups.Num() ) {
		// if we don't have any groups specified as export then export every joint
		return true;
	}

	// search through all exported groups to see if this joint is exported
	for( i = 0; i < exportgroups.Num(); i++ ) {
		group = exportgroups[ i ];
		for( j = 0; j < group->joints.Num(); j++ ) {
			if ( group->joints[ j ] == jointname ) {
				return true;
			}
		}
	}

	return false;
}

/*
==============================================================================

idExportJoint

==============================================================================
*/

idExportJoint::idExportJoint() {
	index				= 0;
	exportNum			= 0;

	mayaNode.SetOwner( this );
	exportNode.SetOwner( this );

	dagnode				= NULL;
						
	t					= vec3_zero;
	wm					= mat3_default;
	bindpos				= vec3_zero;
	bindmat				= mat3_default;
	keep				= false;
	scale				= 1.0f;
	invscale			= 1.0f;
	animBits			= 0;
	firstComponent		= 0;
	baseFrame.q.Set( 0.0f, 0.0f, 0.0f );
	baseFrame.t.Zero();
}

idExportJoint &idExportJoint::operator=( const idExportJoint &other ) {
	name		= other.name;
	realname	= other.realname;
	longname	= other.longname;
	index		= other.index;
	exportNum	= other.exportNum;
	keep		= other.keep;
	
	scale		= other.scale;
	invscale	= other.invscale;
	
	dagnode		= other.dagnode;

	mayaNode	= other.mayaNode;
	exportNode	= other.exportNode;
	
	t			= other.t;
	idt			= other.idt;
	wm			= other.wm;
	idwm		= other.idwm;
	bindpos		= other.bindpos;
	bindmat		= other.bindmat;

	animBits	= other.animBits;
	firstComponent = other.firstComponent;
	baseFrame	= other.baseFrame;

	mayaNode.SetOwner( this );
	exportNode.SetOwner( this );

	return *this;
}

/*
==============================================================================

idExportMesh

==============================================================================
*/

void idExportMesh::ShareVerts( void ) {
	int i, j, k;
	exportVertex_t vert;
	idList<exportVertex_t> v;

	v = verts;
	verts.Clear();
	for( i = 0; i < tris.Num(); i++ ) {
		for( j = 0; j < 3; j++ ) {
			vert = v[ tris[ i ].indexes[ j ] ];
			vert.texCoords[ 0 ] = uv[ i ].uv[ j ][ 0 ];
			vert.texCoords[ 1 ] = 1.0f - uv[ i ].uv[ j ][ 1 ];

			for( k = 0; k < verts.Num(); k++ ) {
				if ( vert.numWeights != verts[ k ].numWeights ) {
					continue;
				}
				if ( vert.startweight != verts[ k ].startweight ) {
					continue;
				}
				if ( !vert.pos.Compare( verts[ k ].pos, SLOP_VERTEX ) ) {
					continue;
				}
				if ( !vert.texCoords.Compare( verts[ k ].texCoords, SLOP_TEXCOORD ) ) {
					continue;
				}

				break;
			}

			if ( k < verts.Num() ) {
				tris[ i ].indexes[ j ] = k;
			} else {
				tris[ i ].indexes[ j ] = verts.Append( vert );
			}
		}
	}
}

void idExportMesh::Merge( idExportMesh *mesh ) {
	int i;
	int numverts;
	int numtris;
	int numweights;
	int numuvs;

	// merge name
	sprintf( name, "%s, %s", name.c_str(), mesh->name.c_str() );

	// merge verts
	numverts = verts.Num();
	verts.SetNum( numverts + mesh->verts.Num() );
	for( i = 0; i < mesh->verts.Num(); i++ ) {
		verts[ numverts + i ] = mesh->verts[ i ];
		verts[ numverts + i ].startweight += weights.Num();
	}

	// merge triangles
	numtris = tris.Num();
	tris.SetNum( numtris + mesh->tris.Num() );
	for( i = 0; i < mesh->tris.Num(); i++ ) {
		tris[ numtris + i ].indexes[ 0 ] = mesh->tris[ i ].indexes[ 0 ] + numverts;
		tris[ numtris + i ].indexes[ 1 ] = mesh->tris[ i ].indexes[ 1 ] + numverts;
		tris[ numtris + i ].indexes[ 2 ] = mesh->tris[ i ].indexes[ 2 ] + numverts;
	}

	// merge weights
	numweights = weights.Num();
	weights.SetNum( numweights + mesh->weights.Num() );
	for( i = 0; i < mesh->weights.Num(); i++ ) {
		weights[ numweights + i ] = mesh->weights[ i ];
	}

	// merge uvs
	numuvs = uv.Num();
	uv .SetNum( numuvs + mesh->uv.Num() );
	for( i = 0; i < mesh->uv.Num(); i++ ) {
		uv[ numuvs + i ] = mesh->uv[ i ];
	}
}

void idExportMesh::GetBounds( idBounds &bounds ) const {
	int						i;
	int						j;
	idVec3					pos;
	const exportWeight_t	*weight;
	const exportVertex_t	*vert;

	bounds.Clear();

	weight = weights.Ptr();
	vert = verts.Ptr();
	for( i = 0; i < verts.Num(); i++, vert++ ) {
		pos.Zero();
		weight = &weights[ vert->startweight ];
		for( j = 0; j < vert->numWeights; j++, weight++ ) {
			pos += weight->jointWeight * ( weight->joint->idwm * weight->offset + weight->joint->idt );
		}
		bounds.AddPoint( pos );
	}
}

/*
==============================================================================

idExportModel

==============================================================================
*/

/*
====================
idExportModel::idExportModel
====================
*/
ID_INLINE idExportModel::idExportModel() {
	export_joints		= 0;
	skipjoints			= 0;
	frameRate			= 24;
	numFrames			= 0;
	exportOrigin		= NULL;
}

/*
====================
idExportModel::~idExportModel
====================
*/
ID_INLINE idExportModel::~idExportModel() {
	meshes.DeleteContents( true );
}

idExportJoint *idExportModel::FindJointReal( const char *name ) {
	idExportJoint	*joint;
	int				i;

	joint = joints.Ptr();
	for( i = 0; i < joints.Num(); i++, joint++ ) {
		if ( joint->realname == name ) {
			return joint;
		}
	}

	return NULL;
}

idExportJoint *idExportModel::FindJoint( const char *name ) {
	idExportJoint	*joint;
	int				i;

	joint = joints.Ptr();
	for( i = 0; i < joints.Num(); i++, joint++ ) {
		if ( joint->name == name ) {
			return joint;
		}
	}

	return NULL;
}

bool idExportModel::WriteMesh( const char *filename, idExportOptions &options ) {
	int				i, j;
	int				numMeshes;
	idExportMesh	*mesh;
	idExportJoint	*joint;
	idExportJoint	*parent;
	idExportJoint	*sibling;
	FILE			*file;
	const char		*parentName;
	int				parentNum;
	idList<idExportJoint *> jointList;

	file = fopen( filename, "w" );
	if ( !file ) {
		return false;
	}

	for( joint = exportHead.GetNext(); joint != NULL; joint = joint->exportNode.GetNext() ) {
		jointList.Append( joint );
	}

	for( i = 0; i < jointList.Num(); i++ ) {
		joint = jointList[ i ];
		sibling = joint->exportNode.GetSibling();
		while( sibling ) {
			if ( idStr::Cmp( joint->name, sibling->name ) > 0 ) {
				joint->exportNode.MakeSiblingAfter( sibling->exportNode );
				sibling = joint->exportNode.GetSibling();
			} else {
				sibling = sibling->exportNode.GetSibling();
			}
		}
	}

	jointList.Clear();
	for( joint = exportHead.GetNext(); joint != NULL; joint = joint->exportNode.GetNext() ) {
		joint->exportNum = jointList.Append( joint );
	}

	numMeshes = 0;
	if ( !options.ignoreMeshes ) {
		for( i = 0; i < meshes.Num(); i++ ) {
			if ( meshes[ i ]->keep ) {
				numMeshes++;
			}
		}
	}

	// write version info
	WriteFloatString( file, MD5_VERSION_STRING " %d\n", MD5_VERSION );
	WriteFloatString( file, "commandline \"%s\"\n\n", options.commandLine.c_str() );

	// write joints
	WriteFloatString( file, "numJoints %d\n", jointList.Num() );
	WriteFloatString( file, "numMeshes %d\n\n", numMeshes );

	WriteFloatString( file, "joints {\n" );
	for( i = 0; i < jointList.Num(); i++ ) {
		joint = jointList[ i ];
		parent = joint->exportNode.GetParent();
		if ( parent ) {
			parentNum = parent->exportNum;
			parentName = parent->name.c_str();
		} else {
			parentNum = -1;
			parentName = "";
		}

		idCQuat	bindQuat = joint->bindmat.ToQuat().ToCQuat();
		WriteFloatString( file, "\t\"%s\"\t%d ( %f %f %f ) ( %f %f %f )\t\t// %s\n", joint->name.c_str(), parentNum, 
			joint->bindpos.x, joint->bindpos.y, joint->bindpos.z, bindQuat[ 0 ], bindQuat[ 1 ], bindQuat[ 2 ], parentName );
	}
	WriteFloatString( file, "}\n" );

	// write meshes
	for( i = 0; i < meshes.Num(); i++ ) {
		mesh = meshes[ i ];
		if ( !mesh->keep ) {
			continue;
		}

		WriteFloatString( file, "\nmesh {\n" );
		WriteFloatString( file, "\t// meshes: %s\n", mesh->name.c_str() );
		WriteFloatString( file, "\tshader \"%s\"\n", mesh->shader.c_str() );
		
		WriteFloatString( file, "\n\tnumverts %d\n", mesh->verts.Num() );
		for( j = 0; j < mesh->verts.Num(); j++ ) {
			WriteFloatString( file, "\tvert %d ( %f %f ) %d %d\n", j, mesh->verts[ j ].texCoords[ 0 ], mesh->verts[ j ].texCoords[ 1 ],
				mesh->verts[ j ].startweight, mesh->verts[ j ].numWeights ); 
		}

		WriteFloatString( file, "\n\tnumtris %d\n", mesh->tris.Num() );
		for( j = 0; j < mesh->tris.Num(); j++ ) {
			WriteFloatString( file, "\ttri %d %d %d %d\n", j, mesh->tris[ j ].indexes[ 2 ], mesh->tris[ j ].indexes[ 1 ], mesh->tris[ j ].indexes[ 0 ] );
		}

		WriteFloatString( file, "\n\tnumweights %d\n", mesh->weights.Num() );
		for( j = 0; j < mesh->weights.Num(); j++ ) {
			exportWeight_t *weight;

			weight = &mesh->weights[ j ];
			WriteFloatString( file, "\tweight %d %d %f ( %f %f %f )\n", j, 
				weight->joint->exportNum, weight->jointWeight, weight->offset.x, weight->offset.y, weight->offset.z );
		}

		WriteFloatString( file, "}\n" );
	}

	fclose( file );

	return true;
}

bool idExportModel::WriteAnim( const char *filename, idExportOptions &options ) {
	int				i, j;
	idExportJoint	*joint;
	idExportJoint	*parent;
	idExportJoint	*sibling;
	jointFrame_t	*frame;
	FILE			*file;
	int				numAnimatedComponents;
	idList<idExportJoint *> jointList;

	file = fopen( filename, "w" );
	if ( !file ) {
		return false;
	}

	for( joint = exportHead.GetNext(); joint != NULL; joint = joint->exportNode.GetNext() ) {
		jointList.Append( joint );
	}

	for( i = 0; i < jointList.Num(); i++ ) {
		joint = jointList[ i ];
		sibling = joint->exportNode.GetSibling();
		while( sibling ) {
			if ( idStr::Cmp( joint->name, sibling->name ) > 0 ) {
				joint->exportNode.MakeSiblingAfter( sibling->exportNode );
				sibling = joint->exportNode.GetSibling();
			} else {
				sibling = sibling->exportNode.GetSibling();
			}
		}
	}

	jointList.Clear();
	for( joint = exportHead.GetNext(); joint != NULL; joint = joint->exportNode.GetNext() ) {
		joint->exportNum = jointList.Append( joint );
	}

	numAnimatedComponents = 0;
	for( i = 0; i < jointList.Num(); i++ ) {
		joint = jointList[ i ];
		joint->exportNum = i;
		joint->baseFrame = frames[ 0 ][ joint->index ];
		joint->animBits = 0;
		for( j = 1; j < numFrames; j++ ) {
			frame = &frames[ j ][ joint->index ];
			if ( fabs( frame->t[ 0 ] - joint->baseFrame.t[ 0 ] ) > options.xyzPrecision ) {
				joint->animBits |= ANIM_TX;
			}
			if ( fabs( frame->t[ 1 ] - joint->baseFrame.t[ 1 ] ) > options.xyzPrecision ) {
				joint->animBits |= ANIM_TY;
			}
			if ( fabs( frame->t[ 2 ] - joint->baseFrame.t[ 2 ] ) > options.xyzPrecision ) {
				joint->animBits |= ANIM_TZ;
			}
			if ( fabs( frame->q[ 0 ] - joint->baseFrame.q[ 0 ] ) > options.quatPrecision ) {
				joint->animBits |= ANIM_QX;
			}
			if ( fabs( frame->q[ 1 ] - joint->baseFrame.q[ 1 ] ) > options.quatPrecision ) {
				joint->animBits |= ANIM_QY;
			}
			if ( fabs( frame->q[ 2 ] - joint->baseFrame.q[ 2 ] ) > options.quatPrecision ) {
				joint->animBits |= ANIM_QZ;
			}
			if ( ( joint->animBits & 63 ) == 63 ) {
				break;
			}
		}
		if ( joint->animBits ) {
			joint->firstComponent = numAnimatedComponents;
			for( j = 0; j < 6; j++ ) {
				if ( joint->animBits & BIT( j ) ) {
					numAnimatedComponents++;
				}
			}
		}
	}

	// write version info
	WriteFloatString( file, MD5_VERSION_STRING " %d\n", MD5_VERSION );
	WriteFloatString( file, "commandline \"%s\"\n\n", options.commandLine.c_str() );

	WriteFloatString( file, "numFrames %d\n", numFrames );
	WriteFloatString( file, "numJoints %d\n", jointList.Num() );
	WriteFloatString( file, "frameRate %d\n", frameRate );
	WriteFloatString( file, "numAnimatedComponents %d\n", numAnimatedComponents );
		
	// write out the hierarchy
	WriteFloatString( file, "\nhierarchy {\n" );
	for( i = 0; i < jointList.Num(); i++ ) {
		joint = jointList[ i ];
		parent = joint->exportNode.GetParent();
		if ( parent ) {
			WriteFloatString( file, "\t\"%s\"\t%d %d %d\t// %s", joint->name.c_str(), parent->exportNum, joint->animBits, joint->firstComponent, parent->name.c_str() );
		} else {
			WriteFloatString( file, "\t\"%s\"\t-1 %d %d\t//", joint->name.c_str(), joint->animBits, joint->firstComponent );
		}

		if ( !joint->animBits ) {
			WriteFloatString( file, "\n" );
		} else {
			WriteFloatString( file, " ( " );
			for( j = 0; j < 6; j++ ) {
				if ( joint->animBits & BIT( j ) ) {
					WriteFloatString( file, "%s ", componentNames[ j ] );
				}
			}
			WriteFloatString( file, ")\n" );
		}
	}
	WriteFloatString( file, "}\n" );

	// write the frame bounds
	WriteFloatString( file, "\nbounds {\n" );
	for( i = 0; i < numFrames; i++ ) {
		WriteFloatString( file, "\t( %f %f %f ) ( %f %f %f )\n", bounds[ i ][ 0 ].x, bounds[ i ][ 0 ].y, bounds[ i ][ 0 ].z, bounds[ i ][ 1 ].x, bounds[ i ][ 1 ].y, bounds[ i ][ 1 ].z );
	}
	WriteFloatString( file, "}\n" );

	// write the base frame
	WriteFloatString( file, "\nbaseframe {\n" );
	for( i = 0; i < jointList.Num(); i++ ) {
		joint = jointList[ i ];
		WriteFloatString( file, "\t( %f %f %f ) ( %f %f %f )\n", joint->baseFrame.t[ 0 ], joint->baseFrame.t[ 1 ], joint->baseFrame.t[ 2 ],
			joint->baseFrame.q[ 0 ], joint->baseFrame.q[ 1 ], joint->baseFrame.q[ 2 ] );
	}
	WriteFloatString( file, "}\n" );

	// write the frames
	for( i = 0; i < numFrames; i++ ) {
		WriteFloatString( file, "\nframe %d {\n", i );
		for( j = 0; j < jointList.Num(); j++ ) {
			joint = jointList[ j ];
			frame = &frames[ i ][ joint->index ];
			if ( joint->animBits ) {
				WriteFloatString( file, "\t" );
				if ( joint->animBits & ANIM_TX ) {
					WriteFloatString( file, " %f", frame->t[ 0 ] );
				}
				if ( joint->animBits & ANIM_TY ) {
					WriteFloatString( file, " %f", frame->t[ 1 ] );
				}
				if ( joint->animBits & ANIM_TZ ) {
					WriteFloatString( file, " %f", frame->t[ 2 ] );
				}
				if ( joint->animBits & ANIM_QX ) {
					WriteFloatString( file, " %f", frame->q[ 0 ] );
				}
				if ( joint->animBits & ANIM_QY ) {
					WriteFloatString( file, " %f", frame->q[ 1 ] );
				}
				if ( joint->animBits & ANIM_QZ ) {
					WriteFloatString( file, " %f", frame->q[ 2 ] );
				}
				WriteFloatString( file, "\n" );
			}
		}
		WriteFloatString( file, "}\n" );
	}

	fclose( file );

	return true;
}

bool idExportModel::WriteCamera( const char *filename, idExportOptions &options ) {
	int		i;
	FILE	*file;

	file = fopen( filename, "w" );
	if ( !file ) {
		return false;
	}

	// write version info
	WriteFloatString( file, MD5_VERSION_STRING " %d\n", MD5_VERSION );
	WriteFloatString( file, "commandline \"%s\"\n\n", options.commandLine.c_str() );

	WriteFloatString( file, "numFrames %d\n", camera.Num() );
	WriteFloatString( file, "frameRate %d\n", frameRate );
	WriteFloatString( file, "numCuts %d\n", cameraCuts.Num() );
	
	// write out the cuts
	WriteFloatString( file, "\ncuts {\n" );
	for( i = 0; i < cameraCuts.Num(); i++ ) {
		WriteFloatString( file, "\t%d\n", cameraCuts[ i ] );
	}
	WriteFloatString( file, "}\n" );

	// write out the frames
	WriteFloatString( file, "\ncamera {\n" );
	cameraFrame_t *frame = camera.Ptr();
	for( i = 0; i < camera.Num(); i++, frame++ ) {
		WriteFloatString( file, "\t( %f %f %f ) ( %f %f %f ) %f\n", frame->t.x, frame->t.y, frame->t.z, frame->q[ 0 ], frame->q[ 1 ], frame->q[ 2 ], frame->fov );
	}
	WriteFloatString( file, "}\n" );

	fclose( file );

	return true;
}

/*
==============================================================================

Maya

==============================================================================
*/

/*
===============
idMayaExport::~idMayaExport

===============
*/
idMayaExport::~idMayaExport() {
	FreeDagNodes();

	// free up the file in Maya
	MFileIO::newFile( true );
}

/*
===============
idMayaExport::TimeForFrame
===============
*/
float idMayaExport::TimeForFrame( int num ) const {
	MTime	time;

	// set time unit to 24 frames per second
	time.setUnit( MTime::kFilm );
	time.setValue( num );
	return time.as( MTime::kSeconds );
}

/*
===============
idMayaExport::GetMayaFrameNum
===============
*/
int idMayaExport::GetMayaFrameNum( int num ) const {
	int	frameNum;

	if ( options.cycleStart > options.startframe ) {
		// in cycles, the last frame is a duplicate of the first frame, so with cycleStart we need to
		// duplicate one of the interior frames instead and chop off the first frame.
		frameNum = options.cycleStart + num;
		if ( frameNum > options.endframe ) {
			frameNum -= options.endframe - options.startframe;
		}
		if ( frameNum < options.startframe ) {
			frameNum  = options.startframe + 1;
		}
	} else {
		frameNum = options.startframe + num;
		if ( frameNum > options.endframe ) {
			frameNum -= options.endframe + 1 - options.startframe;
		}
		if ( frameNum < options.startframe ) {
			frameNum  = options.startframe;
		}
	}

	return frameNum;
}

/*
===============
idMayaExport::SetFrame
===============
*/
void idMayaExport::SetFrame( int num ) {
	MTime	time;
	int		frameNum;

	frameNum = GetMayaFrameNum( num );

	// set time unit to 24 frames per second
	time.setUnit( MTime::kFilm );
	time.setValue( frameNum );
	MGlobal::viewFrame( time );
}

/*
===============
idMayaExport::PruneJoints
===============
*/
void idMayaExport::PruneJoints( idStrList &keepjoints, idStr &prefix ) {
	int				i;
	int				j;
	idExportMesh	*mesh;
	idExportJoint	*joint;
	idExportJoint	*joint2;
	idExportJoint	*parent;
	int				num_weights;

	// if we don't have any joints specified to keep, mark the ones used by the meshes as keep
	if ( !keepjoints.Num() && !prefix.Length() ) {
		if ( !model.meshes.Num() || options.ignoreMeshes ) {
			// export all joints
			joint = model.joints.Ptr();
			for( i = 0; i < model.joints.Num(); i++, joint++ ) {
				joint->keep = true;
			}
		} else {
			for( i = 0; i < model.meshes.Num(); i++, mesh++ ) {
				mesh = model.meshes[ i ];
				for( j = 0; j < mesh->weights.Num(); j++ ) {
					mesh->weights[ j ].joint->keep = true;
				}
			}
		}
	} else {
		// mark the joints to keep
		for( i = 0; i < keepjoints.Num(); i++ ) {
			joint = model.FindJoint( keepjoints[ i ] );
			if ( joint ) {
				joint->keep = true;
			}
		}

		// count valid meshes
		for( i = 0; i < model.meshes.Num(); i++ ) {
			mesh = model.meshes[ i ];
			num_weights = 0;
			for( j = 0; j < mesh->weights.Num(); j++ ) {
				if ( mesh->weights[ j ].joint->keep ) {
					num_weights++;
				} else if ( prefix.Length() && !mesh->weights[ j ].joint->realname.Cmpn( prefix, prefix.Length() ) ) {
					// keep the joint if it's used by the mesh and it has the right prefix
					mesh->weights[ j ].joint->keep = true;
					num_weights++;
				}
			}

			if ( num_weights != mesh->weights.Num() ) {
				mesh->keep = false;
			}
		}
	}

	// find all joints aren't exported and reparent joint's children
	model.export_joints = 0;
	joint = model.joints.Ptr();
	for( i = 0; i < model.joints.Num(); i++, joint++ ) {
		if ( !joint->keep ) {
			joint->exportNode.RemoveFromHierarchy();
		} else {
			joint->index = model.export_joints;
			model.export_joints++;

			// make sure we are parented to an exported joint
			for( parent = joint->exportNode.GetParent(); parent != NULL; parent = parent->exportNode.GetParent() ) {
				if ( parent->keep ) {
					break;
				}
			}

			if ( parent != NULL ) {
				joint->exportNode.ParentTo( parent->exportNode );
			} else {
				joint->exportNode.ParentTo( model.exportHead );
			}
		}
	}

	// check if we have any duplicate joint names
	for( joint = model.exportHead.GetNext(); joint != NULL; joint = joint->exportNode.GetNext() ) {
		if ( !joint->keep ) {
			MayaError( "Non-kept joint in export tree ('%s')", joint->name.c_str() );
		}

		for( joint2 = model.exportHead.GetNext(); joint2 != NULL; joint2 = joint2->exportNode.GetNext() ) {
			if ( ( joint2 != joint ) && ( joint2->name == joint->name ) ) {
				MayaError( "Two joints found with the same name ('%s')", joint->name.c_str() );
			}
		}
	}
}

/*
===============
idMayaExport::FreeDagNodes
===============
*/
void idMayaExport::FreeDagNodes( void ) {
	int i;

	for( i = 0; i < model.joints.Num(); i++ ) {
		delete model.joints[ i ].dagnode;
		model.joints[ i ].dagnode = NULL;
	}
}

/*
===============
idMayaExport::GetBindPose
===============
*/
void idMayaExport::GetBindPose( MObject &jointNode, idExportJoint *joint, float scale ) {
	MStatus				status;
	MFnDependencyNode	fnJoint( jointNode );
	MObject				aBindPose = fnJoint.attribute( "bindPose", &status );

	joint->bindpos = vec3_zero;
	joint->bindmat = mat3_default;

	if ( MS::kSuccess == status ) {
		unsigned	ii;
		unsigned	jointIndex;
		unsigned	connLength;
		MPlugArray	connPlugs;
		MPlug		pBindPose( jointNode, aBindPose );

		pBindPose.connectedTo( connPlugs, false, true );
		connLength = connPlugs.length();
		for( ii = 0; ii < connLength; ++ii ) {
			if ( connPlugs[ ii ].node().apiType() == MFn::kDagPose ) {
				MObject			aMember = connPlugs[ ii ].attribute();
				MFnAttribute	fnAttr( aMember );

				if ( fnAttr.name() == "worldMatrix" ) {
					jointIndex = connPlugs[ ii ].logicalIndex();

					MFnDependencyNode nDagPose( connPlugs[ ii ].node() );

					// construct plugs for this joint's world matrix
					MObject aWorldMatrix = nDagPose.attribute( "worldMatrix" );
					MPlug	pWorldMatrix( connPlugs[ ii ].node(), aWorldMatrix );

					pWorldMatrix.selectAncestorLogicalIndex( jointIndex, aWorldMatrix );

					// get the world matrix data
					MObject worldMatrix;
					MStatus status = pWorldMatrix.getValue( worldMatrix );
					if ( MS::kSuccess != status ) {
						// Problem retrieving world matrix
						return;
					} 

					MFnMatrixData	dMatrix( worldMatrix );
					MMatrix			wMatrix = dMatrix.matrix( &status );

					joint->bindmat = ConvertToIdSpace( idMat( wMatrix ) );
					joint->bindpos = ConvertToIdSpace( idVec( wMatrix ) ) * scale;
					if ( !options.ignoreScale ) {
						joint->bindpos *= joint->scale;
					} else {
						joint->bindmat[ 0 ].Normalize();
						joint->bindmat[ 1 ].Normalize();
						joint->bindmat[ 2 ].Normalize();
					}

					return;
				}
			}
		}
	}
}

/*
===============
idMayaExport::GetLocalTransform
===============
*/
void idMayaExport::GetLocalTransform( idExportJoint *joint, idVec3 &pos, idMat3 &mat ) {
	MStatus		status;
	MDagPath	dagPath;

	pos.Zero();
	mat.Identity();

	if ( !joint->dagnode ) {
		return;
	}

	status = joint->dagnode->getPath( dagPath );
	if ( !status ) {
		return;
	}

	MObject	transformNode = dagPath.transform( &status );
	if ( !status && ( status.statusCode () == MStatus::kInvalidParameter ) ) {
		return;
	}

	MFnDagNode transform( transformNode, &status );
	if ( !status ) {
		return;
	}

	pos = idVec( transform.transformationMatrix() );
	mat = idMat( transform.transformationMatrix() );
}

/*
===============
idMayaExport::GetWorldTransform
===============
*/
void idMayaExport::GetWorldTransform( idExportJoint *joint, idVec3 &pos, idMat3 &mat, float scale ) {
	idExportJoint *parent;

	GetLocalTransform( joint, pos, mat );
	mat.OrthoNormalizeSelf();
	pos *= scale;

	parent = joint->mayaNode.GetParent();
	if ( parent ) {
		idVec3 parentpos;
		idMat3 parentmat;

		GetWorldTransform( parent, parentpos, parentmat, scale );

		pos = parentpos + ( parentmat * ( pos * parent->scale ) );
		mat = mat * parentmat;
	}
}

/*
===============
idMayaExport::CreateJoints
===============
*/
void idMayaExport::CreateJoints( float scale ) {
	int				i;
	int				j;
	idExportJoint	*joint;
	idExportJoint	*parent;
	MStatus			status;
	MDagPath		dagPath;
	MFnDagNode		*parentNode;

	SetFrame( 0 );

	// create an initial list with all of the transformable nodes in the scene
	MItDag dagIterator( MItDag::kDepthFirst, MFn::kTransform, &status );
	for ( ; !dagIterator.isDone(); dagIterator.next() ) {
		status = dagIterator.getPath( dagPath );
		if ( !status ) {
			MayaError( "CreateJoints: MItDag::getPath failed (%s)", status.errorString().asChar() );
			continue;
		}

		joint = &model.joints.Alloc();
		joint->index	= model.joints.Num() - 1;
		joint->dagnode	= new MFnDagNode( dagPath, &status );
		if ( !status ) {
			MayaError( "CreateJoints: MFnDagNode constructor failed (%s)", status.errorString().asChar() );
			continue;
		}

		joint->name		= joint->dagnode->name().asChar();
		joint->realname = joint->name;
	}

	// allocate an extra joint in case we need to add an origin later
	model.exportOrigin = &model.joints.Alloc();
	model.exportOrigin->index	= model.joints.Num() - 1;

	// create scene hierarchy
	joint = model.joints.Ptr();
	for( i = 0; i < model.joints.Num(); i++, joint++ ) {
		if ( !joint->dagnode ) {
			continue;
		}
		joint->mayaNode.ParentTo( model.mayaHead );
		joint->exportNode.ParentTo( model.exportHead );

		parentNode = GetParent( joint->dagnode );
		if ( parentNode ) {
			// find the parent joint in our jointlist
			for( j = 0; j < model.joints.Num(); j++ ) {
				if ( !model.joints[ j ].dagnode ) {
					continue;
				}
				if ( model.joints[ j ].dagnode->name() == parentNode->name() ) {
					joint->mayaNode.ParentTo( model.joints[ j ].mayaNode );
					joint->exportNode.ParentTo( model.joints[ j ].exportNode );
					break;
				}
			}

			delete parentNode;
		}
		
		// create long name
		parent = joint->mayaNode.GetParent();
		if ( parent ) {
			joint->longname = parent->longname + "/" + joint->name;
		} else {
			joint->longname = joint->name;
		}

		// get the joint's scale
		GetLocalTransform( &model.joints[ i ], joint->t, joint->wm );
		joint->scale = joint->wm[ 0 ].Length();

		if ( parent ) {
			joint->scale *= parent->scale;
			if ( joint->scale != 0 ) {
				joint->invscale = 1.0f / joint->scale;
			} else {
				joint->invscale = 0;
			}
		}

		joint->dagnode->getPath( dagPath );
 		GetBindPose( dagPath.node( &status ), joint, scale );
	}
}

/*
===============
idMayaExport::RenameJoints
===============
*/
void idMayaExport::RenameJoints( idList<idNamePair> &renamejoints, idStr &prefix ) {
	int				i;
	idExportJoint	*joint;

	// rename joints that match the prefix
	if ( prefix.Length() ) {
		joint = model.joints.Ptr();
		for( i = 0; i < model.joints.Num(); i++, joint++ ) {
			if ( !joint->name.Cmpn( prefix, prefix.Length() ) ) {
				// remove the prefix from the name
				joint->name = joint->name.Right( joint->name.Length() - prefix.Length() );
			}
		}
	}

	// rename joints if necessary
	for( i = 0; i < renamejoints.Num(); i++ ) {
		joint = model.FindJoint( renamejoints[ i ].from );
		if ( joint ) {
			joint->name = renamejoints[ i ].to;
		}
	}
}

/*
===============
idMayaExport::RemapParents
===============
*/
bool idMayaExport::RemapParents( idList<idNamePair> &remapjoints ) {
	int				i;
	idExportJoint	*joint;
	idExportJoint	*parent;
	idExportJoint	*origin;
	idExportJoint	*sibling;

	for( i = 0; i < remapjoints.Num(); i++ ) {
		// find joint to reparent
		joint = model.FindJoint( remapjoints[ i ].from );
		if ( !joint ) {
			// couldn't find joint, fail
			MayaError( "Couldn't find joint '%s' to reparent\n", remapjoints[ i ].from.c_str() );
		}

		// find new parent joint
		parent = model.FindJoint( remapjoints[ i ].to );
		if ( !parent ) {
			// couldn't find parent, fail
			MayaError( "Couldn't find joint '%s' to be new parent for '%s'\n", remapjoints[ i ].to.c_str(), remapjoints[ i ].from.c_str() );
		}

		if ( parent->exportNode.ParentedBy( joint->exportNode ) ) {
			MayaError( "Joint '%s' is a child of joint '%s' and can't become the parent.", joint->name.c_str(), parent->name.c_str() );
		}
		joint->exportNode.ParentTo( parent->exportNode );
	}

	// if we have an origin, make it the first node in the export list, otherwise add one
	origin = model.FindJoint( "origin" );
	if ( !origin ) {
		origin = model.exportOrigin;
		origin->dagnode	= NULL;
		origin->name	= "origin";
		origin->realname = "origin";
		origin->bindmat.Identity();
		origin->bindpos.Zero();
	}

	origin->exportNode.ParentTo( model.exportHead );

	// force the joint to be kept
	origin->keep = true;
	
	// make all root joints children of the origin joint
	joint = model.exportHead.GetChild();
	while( joint ) {
		sibling = joint->exportNode.GetSibling();
		if ( joint != origin ) {
			joint->exportNode.ParentTo( origin->exportNode );
		}
		joint = sibling;
	}

	return true;
}

/*
===============
idMayaExport::FindShader

Find the shading node for the given shading group set node.
===============
*/
MObject idMayaExport::FindShader( MObject& setNode ) {
	MStatus				status;
	MFnDependencyNode	fnNode( setNode );
	MPlug				shaderPlug;
	
	shaderPlug = fnNode.findPlug( "surfaceShader" );			
	if ( !shaderPlug.isNull() ) {
		MPlugArray connectedPlugs;
		bool asSrc = false;
		bool asDst = true;
		shaderPlug.connectedTo( connectedPlugs, asDst, asSrc, &status );

		if ( connectedPlugs.length() != 1 ) {
			MayaError( "FindShader: Error getting shader (%s)", status.errorString().asChar() );
		} else {
			return connectedPlugs[ 0 ].node();
		}
	}			
	
	return MObject::kNullObj;
}

/*
===============
idMayaExport::GetTextureForMesh

Find the texture files that apply to the color of each polygon of
a selected shape if the shape has its polygons organized into sets.
===============
*/
void idMayaExport::GetTextureForMesh( idExportMesh *mesh, MFnDagNode &dagNode ) {
	MStatus		status;
	MDagPath	path;
	int			i;
	int			instanceNum;
	
	status = dagNode.getPath( path );
	if ( !status ) {
		return;
	}

	path.extendToShape();

	// If the shape is instanced then we need to determine which
	// instance this path refers to.
	//
	instanceNum = 0;
	if ( path.isInstanced() ) {
		instanceNum = path.instanceNumber();
	}

    // Get a list of all sets pertaining to the selected shape and the
    // members of those sets.
    //
	MFnMesh fnMesh( path );
	MObjectArray sets;
	MObjectArray comps;
	status = fnMesh.getConnectedSetsAndMembers( instanceNum, sets, comps, true );
	if ( !status ) {
		MayaError( "GetTextureForMesh: MFnMesh::getConnectedSetsAndMembers failed (%s)", status.errorString().asChar() );
	}

	// Loop through all the sets.  If the set is a polygonal set, find the
    // shader attached to the and print out the texture file name for the
    // set along with the polygons in the set.
	//
	for ( i = 0; i < ( int )sets.length(); i++ ) {
		MObject set = sets[i];
		MObject comp = comps[i];

		MFnSet fnSet( set, &status );
		if ( status == MS::kFailure ) {
			MayaError( "GetTextureForMesh: MFnSet constructor failed (%s)", status.errorString().asChar() );
            continue;
        }

        // Make sure the set is a polygonal set.  If not, continue.
		MItMeshPolygon piter(path, comp, &status);
		if (status == MS::kFailure) {
            continue;
		}

		// Find the texture that is applied to this set.  First, get the
		// shading node connected to the set.  Then, if there is an input
		// attribute called "color", search upstream from it for a texture
		// file node.
        //
		MObject shaderNode = FindShader( set );
		if ( shaderNode == MObject::kNullObj ) {
			continue;
		}

		MPlug colorPlug = MFnDependencyNode(shaderNode).findPlug("color", &status);
		if ( status == MS::kFailure ) {
			continue;
		}

		MItDependencyGraph dgIt(colorPlug, MFn::kFileTexture,
						   MItDependencyGraph::kUpstream, 
						   MItDependencyGraph::kBreadthFirst,
						   MItDependencyGraph::kNodeLevel, 
						   &status);

		if ( status == MS::kFailure ) {
			continue;
		}
		
		dgIt.disablePruningOnFilter();

        // If no texture file node was found, just continue.
        //
		if ( dgIt.isDone() ) {
			continue;
		}
		  
        // Print out the texture node name and texture file that it references.
        //
		MObject textureNode = dgIt.thisNode();
        MPlug filenamePlug = MFnDependencyNode( textureNode ).findPlug( "fileTextureName" );
        MString textureName;
        filenamePlug.getValue( textureName );

		// remove the OS path and save it in the mesh
		OSPathToRelativePath( textureName.asChar(), mesh->shader, options.game );
		mesh->shader.StripFileExtension();

		return;
	}
}

/*
===============
idMayaExport::CopyMesh
===============
*/
idExportMesh *idMayaExport::CopyMesh( MFnSkinCluster &skinCluster, float scale ) {
	MStatus			status;
	MObjectArray	objarray;
	MObjectArray	outputarray;
	int				nGeom;
	int				i, j, k;
	idExportMesh	*mesh;
	float			uv_u, uv_v;
	idStr			name, altname;
	int				pos;

	status = skinCluster.getInputGeometry( objarray );
	if ( !status ) {
		MayaError( "CopyMesh: Error getting input geometry (%s)", status.errorString().asChar() );
		return NULL;
	}

	nGeom = objarray.length();
	for( i = 0; i < nGeom; i++ ) {
		MFnDagNode dagNode( objarray[ i ], &status );
		if ( !status ) {
			common->Printf( "CopyMesh: MFnDagNode Constructor failed (%s)", status.errorString().asChar() );
			continue;
		}

		MFnMesh fnmesh( objarray[ i ], &status );
		if ( !status ) {
			// object isn't an MFnMesh
			continue;
		}

		status = skinCluster.getOutputGeometry( outputarray );
		if ( !status ) {
			common->Printf( "CopyMesh: Error getting output geometry (%s)", status.errorString().asChar() );
			return NULL;
		}

		if ( outputarray.length() < 1 ) {
			return NULL;
		}

		name = fnmesh.name().asChar();
		if ( options.prefix.Length() ) {
			if ( !name.Cmpn( options.prefix, options.prefix.Length() ) ) {
				// remove the prefix from the name
				name = name.Right( name.Length() - options.prefix.Length() );
			} else {
				// name doesn't match prefix, so don't use this mesh
				//return NULL;
			}
		}

		pos = name.Find( "ShapeOrig" );
		if ( pos >= 0 ) {
			name.CapLength( pos );
		}

		MFnDagNode dagNode2( outputarray[ 0 ], &status );
		if ( !status ) {
			common->Printf( "CopyMesh: MFnDagNode Constructor failed (%s)", status.errorString().asChar() );
			continue;
		}

		altname = name;
		MObject	parent = fnmesh.parent( 0, &status );
		if ( status ) {
			MFnDagNode parentNode( parent, &status );
			if ( status ) {
				altname = parentNode.name().asChar();
			}
		}

		name.StripLeadingOnce( options.prefix );
		altname.StripLeadingOnce( options.prefix );
		if ( options.keepmeshes.Num() ) {
			if ( !options.keepmeshes.Find( name ) && !options.keepmeshes.Find( altname ) ) {
				if ( altname != name ) {
					common->Printf( "Skipping mesh '%s' ('%s')\n", name.c_str(), altname.c_str() );
				} else {
					common->Printf( "Skipping mesh '%s'\n", name.c_str() );
				}
				return NULL;
			}
		}
			
		if ( options.skipmeshes.Find( name ) || options.skipmeshes.Find( altname ) ) {
			common->Printf( "Skipping mesh '%s' ('%s')\n", name.c_str(), altname.c_str() );
			return NULL;
		}

		mesh = new idExportMesh();
		model.meshes.Append( mesh );

		if ( altname.Length() ) {
			mesh->name = altname;
		} else {
			mesh->name = name;
		}
		GetTextureForMesh( mesh, dagNode2 );

		int v = fnmesh.numVertices( &status );
		mesh->verts.SetNum( v );

		MFloatPointArray vertexArray;

		fnmesh.getPoints( vertexArray, MSpace::kPreTransform );

		for( j = 0; j < v; j++ ) {
			memset( &mesh->verts[ j ], 0, sizeof( mesh->verts[ j ] ) );
			mesh->verts[ j ].pos = ConvertToIdSpace( idVec( vertexArray[ j ] ) ) * scale;
		}

		MIntArray vertexList;
		int p;
		
		p = fnmesh.numPolygons( &status );
		mesh->tris.SetNum( p );
		mesh->uv.SetNum( p );

		MString setName;
		
		status = fnmesh.getCurrentUVSetName( setName );
		if ( !status ) {
			MayaError( "CopyMesh: MFnMesh::getCurrentUVSetName failed (%s)", status.errorString().asChar() );
		}

		for( j = 0; j < p; j++ ) {
			fnmesh.getPolygonVertices( j, vertexList );
			if ( vertexList.length() != 3 ) {
				MayaError( "CopyMesh: Too many vertices on a face (%d)\n", vertexList.length() );
			}

			for( k = 0; k < 3; k++ ) {
				mesh->tris[ j ].indexes[ k ] = vertexList[ k ];
			
				status = fnmesh.getPolygonUV( j, k, uv_u, uv_v, &setName );
				if ( !status ) {
					MayaError( "CopyMesh: MFnMesh::getPolygonUV failed (%s)", status.errorString().asChar() );
				}
				
				mesh->uv[ j ].uv[ k ][ 0 ] = uv_u;
				mesh->uv[ j ].uv[ k ][ 1 ] = uv_v;
			}
		}

		return mesh;
	}

	return NULL;
}

/*
===============
idMayaExport::CreateMesh
===============
*/
void idMayaExport::CreateMesh( float scale ) {
	size_t			count;
	idExportMesh	*mesh;
	MStatus			status;
	exportWeight_t	weight;
	unsigned int	nGeoms;

	// Iterate through graph and search for skinCluster nodes
	MItDependencyNodes iter( MFn::kSkinClusterFilter );
	count = 0;
	for ( ; !iter.isDone(); iter.next() ) {
		MObject object = iter.item();
		
		count++;
		
		// For each skinCluster node, get the list of influence objects
		MFnSkinCluster skinCluster( object, &status );
		if ( !status ) {
			MayaError( "%s: Error getting skin cluster (%s)", object.apiTypeStr(), status.errorString().asChar() );
		} 

		mesh = CopyMesh( skinCluster, scale );
		if ( !mesh ) {
			continue;
		}

		MDagPathArray infs;
		unsigned int nInfs = skinCluster.influenceObjects(infs, &status);
		if ( !status ) {
			MayaError( "Mesh '%s': Error getting influence objects (%s)", mesh->name.c_str(), status.errorString().asChar() );
		}

		if ( 0 == nInfs ) {
			MayaError( "Mesh '%s': No influence objects found", mesh->name.c_str() );
		}
		
		// loop through the geometries affected by this cluster
		nGeoms = skinCluster.numOutputConnections();
		for (size_t ii = 0; ii < nGeoms; ++ii) {
			unsigned int index = skinCluster.indexForOutputConnection(ii,&status);

			if ( !status ) {
				MayaError( "Mesh '%s': Error getting geometry index (%s)", mesh->name.c_str(), status.errorString().asChar() );
			}

			// get the dag path of the ii'th geometry
			MDagPath skinPath;
			status = skinCluster.getPathAtIndex(index,skinPath);
			if ( !status ) {
				MayaError( "Mesh '%s': Error getting geometry path (%s)", mesh->name.c_str(), status.errorString().asChar() );
			}

			// iterate through the components of this geometry
			MItGeometry gIter( skinPath );

			// print out the influence objects
			idList<idExportJoint *> joints;
			idExportJoint			*joint;
			exportVertex_t			*vert;

			joints.Resize( nInfs );
			for (size_t kk = 0; kk < nInfs; ++kk) {
				const char *c;
				MString s;

				s = infs[kk].partialPathName();
				c = s.asChar();
				joint = model.FindJointReal( c );
				if ( !joint ) {
					MayaError( "Mesh '%s': joint %s not found", mesh->name.c_str(), c );
				}

				joints.Append( joint );
			}

			for ( /* nothing */ ; !gIter.isDone(); gIter.next() ) {
				MObject comp = gIter.component( &status );
				if ( !status ) {
					MayaError( "Mesh '%s': Error getting component (%s)", mesh->name.c_str(), status.errorString().asChar() );
				}

				// Get the weights for this vertex (one per influence object)
				MFloatArray wts;
				unsigned infCount;
				status = skinCluster.getWeights(skinPath,comp,wts,infCount);
				if ( !status ) {
					MayaError( "Mesh '%s': Error getting weights (%s)", mesh->name.c_str(), status.errorString().asChar() );
				}
				if (0 == infCount) {
					MayaError( "Mesh '%s': Error: 0 influence objects.", mesh->name.c_str() );
				}

				int num = gIter.index();
				vert = &mesh->verts[ num ];
				vert->startweight = mesh->weights.Num();

				float totalweight = 0.0f;

				// copy the weight data for this vertex
				int numNonZeroWeights = 0;
				int jj;
				for ( jj = 0; jj < (int)infCount ; ++jj ) {
					float w = ( float )wts[ jj ];
					if ( w > 0.0f ) {
						numNonZeroWeights++;
					}
					if ( w > options.jointThreshold ) {
						weight.joint = joints[ jj ];
						weight.jointWeight = wts[ jj ];
						
						if ( !options.ignoreScale ) {
							weight.joint->bindmat.ProjectVector( vert->pos - ( weight.joint->bindpos * weight.joint->invscale ), weight.offset );
							weight.offset *= weight.joint->scale;
						} else {
							weight.joint->bindmat.ProjectVector( vert->pos - weight.joint->bindpos, weight.offset );
						}
						mesh->weights.Append( weight );
						totalweight += weight.jointWeight;
					}
				}

				vert->numWeights = mesh->weights.Num() - vert->startweight;
				if ( !vert->numWeights ) {
					if ( numNonZeroWeights ) {
						MayaError( "Error on mesh '%s': Vertex %d doesn't have any joint weights exceeding jointThreshold (%f).", mesh->name.c_str(), num, options.jointThreshold );
					} else {
						MayaError( "Error on mesh '%s': Vertex %d doesn't have any joint weights.", mesh->name.c_str(), num );
					}
				} else if ( !totalweight ) {
					MayaError( "Error on mesh '%s': Combined weight of 0 on vertex %d.", mesh->name.c_str(), num );
				}
				//if ( numNonZeroWeights ) {
				//	common->Printf( "Mesh '%s': skipped %d out of %d weights on vertex %d\n", mesh->name.c_str(), numNonZeroWeights, numNonZeroWeights + vert->numWeights, num );
				//}

				// normalize the joint weights
				for( jj = 0; jj < vert->numWeights; jj++ ) {
					mesh->weights[ vert->startweight + jj ].jointWeight /= totalweight;
				}
			}
			break;
		}
	}

	if ( !count && !options.ignoreMeshes ) {
		MayaError( "CreateMesh: No skinClusters found in this scene.\n" );
	}
}

/*
===============
idMayaExport::CombineMeshes

combine surfaces with the same shader.
===============
*/
void idMayaExport::CombineMeshes( void ) {
	int						i, j;
	int						count;
	idExportMesh			*mesh;
	idExportMesh			*combine;
	idList<idExportMesh *>	oldmeshes;

	oldmeshes = model.meshes;
	model.meshes.Clear();

	count = 0;
	for( i = 0; i < oldmeshes.Num(); i++ ) {
		mesh = oldmeshes[ i ];
		if ( !mesh->keep ) {
			delete mesh;
			continue;
		}

		combine = NULL;
		for( j = 0; j < model.meshes.Num(); j++ ) {
			if ( model.meshes[ j ]->shader == mesh->shader ) {
				combine = model.meshes[ j ];
				break;
			}
		}

		if ( combine ) {
			combine->Merge( mesh );
			delete mesh;
			count++;
		} else {
			model.meshes.Append( mesh );
		}
	}

	// share verts
	for( i = 0; i < model.meshes.Num(); i++ ) {
		model.meshes[ i ]->ShareVerts();
	}

	common->Printf( "Merged %d meshes\n", count );
}

/*
===============
idMayaExport::GetAlignment
===============
*/
void idMayaExport::GetAlignment( idStr &alignName, idMat3 &align, float rotate, int startframe ) {
	idVec3			pos;
	idExportJoint	*joint;
	idAngles		ang( 0, rotate, 0 );
	idMat3			mat;

	align.Identity();

	if ( alignName.Length() ) {
		SetFrame( 0 );

		joint = model.FindJoint( alignName );
		if ( !joint ) {
			MayaError( "could not find joint '%s' to align model to.\n", alignName.c_str() );
		}

		// found it
		GetWorldTransform( joint, pos, mat, 1.0f );
		align[ 0 ][ 0 ] = mat[ 2 ][ 0 ];
		align[ 0 ][ 1 ] = -mat[ 2 ][ 2 ];
		align[ 0 ][ 2 ] = mat[ 2 ][ 1 ];

		align[ 1 ][ 0 ] = mat[ 0 ][ 0 ];
		align[ 1 ][ 1 ] = -mat[ 0 ][ 2 ];
		align[ 1 ][ 2 ] = mat[ 0 ][ 1 ];

		align[ 2 ][ 0 ] = mat[ 1 ][ 0 ];
		align[ 2 ][ 1 ] = -mat[ 1 ][ 2 ];
		align[ 2 ][ 2 ] = mat[ 1 ][ 1 ];

		if ( rotate ) {
			align *= ang.ToMat3();
		}
	} else if ( rotate ) {
		align = ang.ToMat3();
	}

	align.TransposeSelf();
}

/*
===============
idMayaExport::GetObjectType

return the type of the object
===============
*/
const char *idMayaExport::GetObjectType( MObject object ) {
	if( object.isNull() ) {
		return "(Null)";
	}

	MStatus				stat;
	MFnDependencyNode	dgNode;
	MString				typeName;

	stat		= dgNode.setObject( object );
	typeName	= dgNode.typeName( &stat );
	if( MS::kSuccess != stat ) {
		// can not get the type name of this object
		return "(Unknown)";
	}

	return typeName.asChar();
}

/*
===============
idMayaExport::GetCameraFov
===============
*/
float idMayaExport::GetCameraFov( idExportJoint *joint ) {
	int		childCount;
	int		j;
	double	horiz;
	double	focal;
	MStatus	status;
	const char *n1, *n2;
	MFnDagNode *dagnode;
	float	fov;

	dagnode = joint->dagnode;

	MObject cameraNode = dagnode->object();
	childCount = dagnode->childCount();

	fov = 90.0f;
	for( j = 0; j < childCount; j++ ) {
		MObject childNode = dagnode->child( j );
		
		n1 = GetObjectType( cameraNode );
		n2 = GetObjectType( childNode );
		if ( ( !strcmp( "transform", n1 ) ) && ( !strcmp( "camera", n2 ) ) ) {
			MFnCamera camera( childNode );
			focal = camera.focalLength();
			horiz = camera.horizontalFilmAperture();
			fov = RAD2DEG( 2 * atan( ( horiz * 0.5 ) / ( focal / 25.4 ) ) );
			break;
		}
	}

	return fov;
}

/*
===============
idMayaExport::GetCameraFrame
===============
*/
void idMayaExport::GetCameraFrame( idExportJoint *camera, idMat3 &align, cameraFrame_t *cam ) {
	idMat3 mat;
	idMat3 axis;
	idVec3 pos;

	// get the worldspace positions of the joint
	GetWorldTransform( camera, pos, axis, 1.0f );
	
	// convert to id coordinates
	cam->t	= ConvertToIdSpace( pos ) * align;

	// correct the orientation for the game
	axis	= ConvertToIdSpace( axis ) * align;
	mat[ 0 ] = -axis[ 2 ];
	mat[ 1 ] = -axis[ 0 ];
	mat[ 2 ] = axis[ 1 ];
	cam->q = mat.ToQuat().ToCQuat();

	// get it's fov
	cam->fov = GetCameraFov( camera );
}

/*
===============
idMayaExport::CreateCameraAnim
===============
*/
void idMayaExport::CreateCameraAnim( idMat3 &align ) {
	float				start, end;
	MDagPath			dagPath;
	int					frameNum;
	short				v;
	MStatus				status;
	cameraFrame_t		cam;
	idExportJoint		*refCam;
	idExportJoint		*camJoint;
	idStr				currentCam;
	idStr				newCam;
	MPlug				plug;
	MFnEnumAttribute	cameraAttribute;

	start = TimeForFrame( options.startframe );
	end   = TimeForFrame( options.endframe );

#if 0
	options.framerate = 60;
	model.numFrames = ( int )( ( end - start ) * ( float )options.framerate ) + 1;
	model.frameRate = options.framerate;
#else
	model.numFrames = options.endframe + 1 - options.startframe;
	model.frameRate = options.framerate;
#endif

	common->Printf( "start frame = %d\n  end frame = %d\n", options.startframe, options.endframe );
	common->Printf( " start time = %f\n   end time = %f\n total time = %f\n", start, end, end - start );

	if ( start > end ) {
		MayaError( "Start frame is greater than end frame." );
	}

	refCam = model.FindJoint( "refcam" );
	if ( refCam == NULL ) {
		currentCam = MAYA_DEFAULT_CAMERA;
	} else {
		MObject cameraNode = refCam->dagnode->object();
		MFnDependencyNode cameraDG( cameraNode, &status );
		if( MS::kSuccess != status ) {
			MayaError( "Can't find 'refcam' dependency node." );
			return;
		}

		MObject attr = cameraDG.attribute( MString( "Camera" ), &status );
		if( MS::kSuccess != status ) {
			MayaError( "Can't find 'Camera' attribute on 'refcam'." );
			return;
		}

		plug = MPlug( cameraNode, attr );
		status = cameraAttribute.setObject( attr );
		if( MS::kSuccess != status ) {
			MayaError( "Bad 'Camera' attribute on 'refcam'." );
			return;
		}

		model.camera.Clear();
		model.cameraCuts.Clear();

		SetFrame( 0 );
		status = plug.getValue( v );
		currentCam = cameraAttribute.fieldName( v, &status ).asChar();
		if( MS::kSuccess != status ) {
			MayaError( "Error getting camera name on frame %d", GetMayaFrameNum( 0 ) );
		}
	}

    camJoint = model.FindJoint( currentCam );
	if ( !camJoint ) {
		MayaError( "Couldn't find camera '%s'", currentCam.c_str() );
	}

	for( frameNum = 0; frameNum < model.numFrames; frameNum++ ) {
		common->Printf( "\rFrame %d/%d...", options.startframe + frameNum, options.endframe );

#if 0
		MTime	time;
		time.setUnit( MTime::kSeconds );
		time.setValue( start + ( ( float )frameNum / ( float )options.framerate ) );
		MGlobal::viewFrame( time );
#else
		SetFrame( frameNum );
#endif

		// get the position for this frame
		GetCameraFrame( camJoint, align, &model.camera.Alloc() );

		if ( refCam != NULL ) {
			status = plug.getValue( v );
			newCam = cameraAttribute.fieldName( v, &status ).asChar();
			if( MS::kSuccess != status ) {
				MayaError( "Error getting camera name on frame %d", GetMayaFrameNum( frameNum ) );
			}

			if ( newCam != currentCam ) {
				// place a cut at our last frame
				model.cameraCuts.Append( model.camera.Num() - 1 );

				currentCam = newCam;
				camJoint = model.FindJoint( currentCam );
				if ( !camJoint ) {
					MayaError( "Couldn't find camera '%s'", currentCam.c_str() );
				}

				// get the position for this frame
				GetCameraFrame( camJoint, align, &model.camera.Alloc() );
			}
		}
	}

	common->Printf( "\n" );
}

/*
===============
idMayaExport::GetDefaultPose
===============
*/
void idMayaExport::GetDefaultPose( idMat3 &align ) {
	float			start;
	MDagPath		dagPath;
	idMat3			jointaxis;
	idVec3			jointpos;
	idExportJoint	*joint, *parent;
	idBounds		bnds;
	idBounds		meshBounds;
	idList<jointFrame_t> frame;

	start = TimeForFrame( options.startframe );

	common->Printf( "default pose frame = %d\n", options.startframe );
	common->Printf( " default pose time = %f\n", start );

	frame.SetNum( model.joints.Num() );
	SetFrame( 0 );

	// convert joints into local coordinates and save in channels
	for( joint = model.exportHead.GetNext(); joint != NULL; joint = joint->exportNode.GetNext() ) {
		if ( !joint->dagnode ) {
			// custom origin joint
			joint->idwm.Identity();
			joint->idt.Zero();
			frame[ joint->index ].t.Zero();
			frame[ joint->index ].q.Set( 0.0f, 0.0f, 0.0f );
			continue;
		}

		// get the worldspace positions of the joint
		GetWorldTransform( joint, jointpos, jointaxis, options.scale );
		
		// convert to id coordinates
		jointaxis	= ConvertToIdSpace( jointaxis ) * align;
		jointpos	= ConvertToIdSpace( jointpos ) * align;

		// save worldspace position of joint for children
		joint->idwm	= jointaxis;
		joint->idt	= jointpos;

		parent = joint->exportNode.GetParent();
		if ( parent ) {
			// convert to local coordinates
			jointpos = ( jointpos - parent->idt ) * parent->idwm.Transpose();
			jointaxis = jointaxis * parent->idwm.Transpose();
		} else if ( joint->name == "origin" ) {
			if ( options.clearOrigin ) {
				jointpos.Zero();
			}
			if ( options.clearOriginAxis ) {
				jointaxis.Identity();
			}
		}

		frame[ joint->index ].t = jointpos;
		frame[ joint->index ].q = jointaxis.ToQuat().ToCQuat();
	}

	// relocate origin to start at 0, 0, 0 for first frame
	joint = model.FindJoint( "origin" );
	if ( joint ) {
		frame[ joint->index ].t.Zero();
	}

	// transform the hierarchy
	for( joint = model.exportHead.GetNext(); joint != NULL; joint = joint->exportNode.GetNext() ) {
		jointpos	= frame[ joint->index ].t;
		jointaxis	= frame[ joint->index ].q.ToQuat().ToMat3();

		parent = joint->exportNode.GetParent();
		if ( parent ) {
			joint->idwm = jointaxis * parent->idwm;
			joint->idt = parent->idt + jointpos * parent->idwm;
		} else {
			joint->idwm = jointaxis;
			joint->idt = jointpos;
		}

		joint->bindmat = joint->idwm;
		joint->bindpos = joint->idt;
	}

	common->Printf( "\n" );
}

/*
===============
idMayaExport::CreateAnimation
===============
*/
void idMayaExport::CreateAnimation( idMat3 &align ) {
	int				i;
	float			start, end;
	MDagPath		dagPath;
	idMat3			jointaxis;
	idVec3			jointpos;
	int				frameNum;
	idExportJoint	*joint, *parent;
	idBounds		bnds;
	idBounds		meshBounds;
	jointFrame_t	*frame;
	int				cycleStart;
	idVec3			totalDelta;
	idList<jointFrame_t>	copyFrames;

	start = TimeForFrame( options.startframe );
	end   = TimeForFrame( options.endframe );

	model.numFrames = options.endframe + 1 - options.startframe;
	model.frameRate = options.framerate;

	common->Printf( "start frame = %d\n  end frame = %d\n", options.startframe, options.endframe );
	common->Printf( " start time = %f\n   end time = %f\n total time = %f\n", start, end, end - start );

	if ( start > end ) {
		MayaError( "Start frame is greater than end frame." );
	}

	model.bounds.SetNum( model.numFrames );
	model.jointFrames.SetNum( model.numFrames * model.joints.Num() );
	model.frames.SetNum( model.numFrames );
	for( i = 0; i < model.numFrames; i++ ) {
		model.frames[ i ] = &model.jointFrames[ model.joints.Num() * i ];
	}

	// *sigh*.  cyclestart doesn't work nicely with the anims.
	// may just want to not do it in SetTime anymore.
	cycleStart = options.cycleStart;
	options.cycleStart = options.startframe;

	for( frameNum = 0; frameNum < model.numFrames; frameNum++ ) {
		common->Printf( "\rFrame %d/%d...", options.startframe + frameNum, options.endframe );

		frame = model.frames[ frameNum ];
		SetFrame( frameNum );

		// convert joints into local coordinates and save in channels
		for( joint = model.exportHead.GetNext(); joint != NULL; joint = joint->exportNode.GetNext() ) {
			if ( !joint->dagnode ) {
				// custom origin joint
				joint->idwm.Identity();
				joint->idt.Zero();
				frame[ joint->index ].t.Zero();
				frame[ joint->index ].q.Set( 0.0f, 0.0f, 0.0f );
				continue;
			}

			// get the worldspace positions of the joint
			GetWorldTransform( joint, jointpos, jointaxis, options.scale );
			
			// convert to id coordinates
			jointaxis	= ConvertToIdSpace( jointaxis ) * align;
			jointpos	= ConvertToIdSpace( jointpos ) * align;

			// save worldspace position of joint for children
			joint->idwm	= jointaxis;
			joint->idt	= jointpos;

			parent = joint->exportNode.GetParent();
			if ( parent ) {
				// convert to local coordinates
				jointpos = ( jointpos - parent->idt ) * parent->idwm.Transpose();
				jointaxis = jointaxis * parent->idwm.Transpose();
			} else if ( joint->name == "origin" ) {
				if ( options.clearOrigin ) {
					jointpos.Zero();
				}
				if ( options.clearOriginAxis ) {
					jointaxis.Identity();
				}
			}

			frame[ joint->index ].t = jointpos;
			frame[ joint->index ].q = jointaxis.ToQuat().ToCQuat();
		}
	}

	options.cycleStart = cycleStart;
	totalDelta.Zero();

	joint = model.FindJoint( "origin" );
	if ( joint ) {
		frame = model.frames[ 0 ];
		idVec3 origin = frame[ joint->index ].t;

		frame = model.frames[ model.numFrames - 1 ];
		totalDelta = frame[ joint->index ].t - origin;
	}

	// shift the frames when cycleStart is used
	if ( options.cycleStart > options.startframe ) {
		copyFrames = model.jointFrames;
		for( i = 0; i < model.numFrames; i++ ) {
			bool shiftorigin = false;
			frameNum = i + ( options.cycleStart - options.startframe );
			if ( frameNum >= model.numFrames ) {
				// wrap around, skipping the first frame, since it's a dupe of the last frame
				frameNum -= model.numFrames - 1;
				shiftorigin = true;
			}

			memcpy( &model.jointFrames[ model.joints.Num() * i ], &copyFrames[ model.joints.Num() * frameNum ], model.joints.Num() * sizeof( copyFrames[ 0 ] ) );

			if ( joint && shiftorigin ) {
				model.frames[ i ][ joint->index ].t += totalDelta;
			}
		}
	}

	if ( joint ) {
		// relocate origin to start at 0, 0, 0 for first frame
		frame = model.frames[ 0 ];
		idVec3 origin = frame[ joint->index ].t;
		for( i = 0; i < model.numFrames; i++ ) {
			frame = model.frames[ i ];
			frame[ joint->index ].t -= origin;
		}
	}

	// get the bounds for each frame
	for( frameNum = 0; frameNum < model.numFrames; frameNum++ ) {
		frame = model.frames[ frameNum ];

		// transform the hierarchy
		for( joint = model.exportHead.GetNext(); joint != NULL; joint = joint->exportNode.GetNext() ) {
			jointpos	= frame[ joint->index ].t;
			jointaxis	= frame[ joint->index ].q.ToQuat().ToMat3();

			parent = joint->exportNode.GetParent();
			if ( parent ) {
				joint->idwm = jointaxis * parent->idwm;
				joint->idt = parent->idt + jointpos * parent->idwm;
			} else {
				joint->idwm = jointaxis;
				joint->idt = jointpos;
			}
		}

		// get bounds for this frame
		bnds.Clear();
		for( i = 0; i < model.meshes.Num(); i++ ) {
			if ( model.meshes[ i ]->keep ) {
				model.meshes[ i ]->GetBounds( meshBounds );
				bnds.AddBounds( meshBounds );
			}
		}
		model.bounds[ frameNum ][ 0 ] = bnds[ 0 ];
		model.bounds[ frameNum ][ 1 ] = bnds[ 1 ];
	}

	common->Printf( "\n" );
}

/*
===============
idMayaExport::ConvertModel
===============
*/
void idMayaExport::ConvertModel( void ) {
	MStatus	status;
	idMat3	align;

	common->Printf( "Converting %s to %s...\n", options.src.c_str(), options.dest.c_str() );

	// see if the destination file exists
	FILE *file = fopen( options.dest, "r" );
	if ( file ) {
		fclose( file );

		// make sure we can write to the destination
		FILE *file = fopen( options.dest, "r+" );
		if ( !file ) {
			MayaError( "Unable to write to the file '%s'", options.dest.c_str() );
		}
		fclose( file );
	}

	MString	filename( options.src );
	MFileIO::newFile( true );

	// Load the file into Maya
	common->Printf( "Loading file...\n" );
	status = MFileIO::open( filename, NULL, true );
	if ( !status ) {
		MayaError( "Error loading '%s': '%s'\n", filename.asChar(), status.errorString().asChar() );
	}

	// force Maya to update the frame.  When using references, sometimes
	// the model is posed the way it is in the source.  Since Maya only
	// updates it when the frame time changes to a value other than the
	// current, just setting the time to the min time doesn't guarantee
	// that the model gets updated.
	MGlobal::viewFrame( MAnimControl::maxTime() );
	MGlobal::viewFrame( MAnimControl::minTime() );

	if ( options.startframe < 0 ) {
		options.startframe = MAnimControl::minTime().as( MTime::kFilm );
	}

	if ( options.endframe < 0 ) {
		options.endframe = MAnimControl::maxTime().as( MTime::kFilm );
	}
	if ( options.cycleStart < 0 ) {
		options.cycleStart = options.startframe;
	} else if ( ( options.cycleStart < options.startframe ) || ( options.cycleStart > options.endframe ) ) {
		MayaError( "cycleStart (%d) out of frame range (%d to %d)\n", options.cycleStart, options.startframe, options.endframe );
	} else if ( options.cycleStart == options.endframe ) {
		// end frame is a duplicate of the first frame in cycles, so just disable cycleStart
		options.cycleStart = options.startframe;
	}

	// create a list of the transform nodes that will make up our heirarchy
	common->Printf( "Creating joints...\n" );
	CreateJoints( options.scale );
	if ( options.type != WRITE_CAMERA ) {
		common->Printf( "Creating meshes...\n" );
		CreateMesh( options.scale );
		common->Printf( "Renaming joints...\n" );
		RenameJoints( options.renamejoints, options.prefix );
		common->Printf( "Remapping parents...\n" );
		RemapParents( options.remapjoints );
		common->Printf( "Pruning joints...\n" );
		PruneJoints( options.keepjoints, options.prefix );
		common->Printf( "Combining meshes...\n" );
		CombineMeshes();
	}

	common->Printf( "Align model...\n" );
	GetAlignment( options.align, align, options.rotate, 0 );

	switch( options.type ) {
	case WRITE_MESH :
        common->Printf( "Grabbing default pose:\n" );
		GetDefaultPose( align );
		common->Printf( "Writing file...\n" );
		if ( !model.WriteMesh( options.dest, options ) ) {
			MayaError( "error writing to '%s'", options.dest.c_str() );
		}
		break;

	case WRITE_ANIM :
        common->Printf( "Creating animation frames:\n" );
		CreateAnimation( align );
		common->Printf( "Writing file...\n" );
		if ( !model.WriteAnim( options.dest, options ) ) {
			MayaError( "error writing to '%s'", options.dest.c_str() );
		}
		break;

	case WRITE_CAMERA :
		common->Printf( "Creating camera frames:\n" );
		CreateCameraAnim( align );

		common->Printf( "Writing file...\n" );
		if ( !model.WriteCamera( options.dest, options ) ) {
			MayaError( "error writing to '%s'", options.dest.c_str() );
		}
		break;
	}

	common->Printf( "done\n\n" );
}

/*
===============
idMayaExport::ConvertToMD3
===============
*/
void idMayaExport::ConvertToMD3( void ) {
#if 0
	int					i, j;
	md3Header_t			*pinmodel;
    md3Frame_t			*frame;
	md3Surface_t		*surf;
	md3Shader_t			*shader;
	md3Triangle_t		*tri;
	md3St_t				*st;
	md3XyzNormal_t		*xyz;
	md3Tag_t			*tag;
	int					version;
	int					size;

	//model_t *mod, int lod, void *buffer, const char *mod_name

	pinmodel = (md3Header_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != MD3_VERSION) {
		common->Printf( "R_LoadMD3: %s has wrong version (%i should be %i)\n",
				 mod_name, version, MD3_VERSION);
		return qfalse;
	}

	mod->type = MOD_MESH;
	size = LittleLong(pinmodel->ofsEnd);
	mod->dataSize += size;
	mod->md3[lod] = ri.Hunk_Alloc( size );

	memcpy (mod->md3[lod], buffer, LittleLong(pinmodel->ofsEnd) );

    LL(mod->md3[lod]->ident);
    LL(mod->md3[lod]->version);
    LL(mod->md3[lod]->numFrames);
    LL(mod->md3[lod]->numTags);
    LL(mod->md3[lod]->numSurfaces);
    LL(mod->md3[lod]->ofsFrames);
    LL(mod->md3[lod]->ofsTags);
    LL(mod->md3[lod]->ofsSurfaces);
    LL(mod->md3[lod]->ofsEnd);

	if ( mod->md3[lod]->numFrames < 1 ) {
		common->Printf( "R_LoadMD3: %s has no frames\n", mod_name );
		return qfalse;
	}
    
	// swap all the frames
    frame = (md3Frame_t *) ( (byte *)mod->md3[lod] + mod->md3[lod]->ofsFrames );
    for ( i = 0 ; i < mod->md3[lod]->numFrames ; i++, frame++) {
    	frame->radius = LittleFloat( frame->radius );
        for ( j = 0 ; j < 3 ; j++ ) {
            frame->bounds[0][j] = LittleFloat( frame->bounds[0][j] );
            frame->bounds[1][j] = LittleFloat( frame->bounds[1][j] );
	    	frame->localOrigin[j] = LittleFloat( frame->localOrigin[j] );
        }
	}

	// swap all the tags
    tag = (md3Tag_t *) ( (byte *)mod->md3[lod] + mod->md3[lod]->ofsTags );
    for ( i = 0 ; i < mod->md3[lod]->numTags * mod->md3[lod]->numFrames ; i++, tag++) {
        for ( j = 0 ; j < 3 ; j++ ) {
			tag->origin[j] = LittleFloat( tag->origin[j] );
			tag->axis[0][j] = LittleFloat( tag->axis[0][j] );
			tag->axis[1][j] = LittleFloat( tag->axis[1][j] );
			tag->axis[2][j] = LittleFloat( tag->axis[2][j] );
        }
	}

	// swap all the surfaces
	surf = (md3Surface_t *) ( (byte *)mod->md3[lod] + mod->md3[lod]->ofsSurfaces );
	for ( i = 0 ; i < mod->md3[lod]->numSurfaces ; i++) {

        LL(surf->ident);
        LL(surf->flags);
        LL(surf->numFrames);
        LL(surf->numShaders);
        LL(surf->numTriangles);
        LL(surf->ofsTriangles);
        LL(surf->numVerts);
        LL(surf->ofsShaders);
        LL(surf->ofsSt);
        LL(surf->ofsXyzNormals);
        LL(surf->ofsEnd);
		
		if ( surf->numVerts > SHADER_MAX_VERTEXES ) {
			ri.Error (ERR_DROP, "R_LoadMD3: %s has more than %i verts on a surface (%i)",
				mod_name, SHADER_MAX_VERTEXES, surf->numVerts );
		}
		if ( surf->numTriangles*3 > SHADER_MAX_INDEXES ) {
			ri.Error (ERR_DROP, "R_LoadMD3: %s has more than %i triangles on a surface (%i)",
				mod_name, SHADER_MAX_INDEXES / 3, surf->numTriangles );
		}
	
		// change to surface identifier
		surf->ident = SF_MD3;

		// lowercase the surface name so skin compares are faster
		Q_strlwr( surf->name );

		// strip off a trailing _1 or _2
		// this is a crutch for q3data being a mess
		j = strlen( surf->name );
		if ( j > 2 && surf->name[j-2] == '_' ) {
			surf->name[j-2] = 0;
		}

        // register the shaders
        shader = (md3Shader_t *) ( (byte *)surf + surf->ofsShaders );
        for ( j = 0 ; j < surf->numShaders ; j++, shader++ ) {
            shader_t	*sh;

            sh = R_FindShader( shader->name, LIGHTMAP_NONE, qtrue );
			if ( sh->defaultShader ) {
				shader->shaderIndex = 0;
			} else {
				shader->shaderIndex = sh->index;
			}
        }

		// swap all the triangles
		tri = (md3Triangle_t *) ( (byte *)surf + surf->ofsTriangles );
		for ( j = 0 ; j < surf->numTriangles ; j++, tri++ ) {
			LL(tri->indexes[0]);
			LL(tri->indexes[1]);
			LL(tri->indexes[2]);
		}

		// swap all the ST
        st = (md3St_t *) ( (byte *)surf + surf->ofsSt );
        for ( j = 0 ; j < surf->numVerts ; j++, st++ ) {
            st->st[0] = LittleFloat( st->st[0] );
            st->st[1] = LittleFloat( st->st[1] );
        }

		// swap all the XyzNormals
        xyz = (md3XyzNormal_t *) ( (byte *)surf + surf->ofsXyzNormals );
        for ( j = 0 ; j < surf->numVerts * surf->numFrames ; j++, xyz++ ) 
		{
            xyz->xyz[0] = LittleShort( xyz->xyz[0] );
            xyz->xyz[1] = LittleShort( xyz->xyz[1] );
            xyz->xyz[2] = LittleShort( xyz->xyz[2] );

            xyz->normal = LittleShort( xyz->normal );
        }


		// find the next surface
		surf = (md3Surface_t *)( (byte *)surf + surf->ofsEnd );
	}
	return true;
#endif
}

/*
==============================================================================

dll setup

==============================================================================
*/

/*
===============
Maya_Shutdown
===============
*/
void Maya_Shutdown( void ) {
	if ( initialized ) {
		errorMessage.Clear();
		initialized = false;

		// This shuts down the entire app somehow, so just ignore it.
		//MLibrary::cleanup();
	}
}

/*
===============
Maya_ConvertModel
===============
*/
const char *Maya_ConvertModel( const char *ospath, const char *commandline ) {
	
	errorMessage = "Ok";
 
	try {
		idExportOptions options( commandline, ospath );
		idMayaExport	exportM( options );

		exportM.ConvertModel();
	}
	
	catch( idException &exception ) {
		errorMessage = exception.error;
	}

	return errorMessage;
}

/*
===============
dllEntry
===============
*/
bool dllEntry( int version, idCommon *common, idSys *sys ) {

	if ( !common || !sys ) {
		return false;
	}

	::common = common;
	::sys = sys;
	::cvarSystem = NULL;

	idLib::sys			= sys;
	idLib::common		= common;
	idLib::cvarSystem	= NULL;
	idLib::fileSystem	= NULL;

	idLib::Init();

	if ( version != MD5_VERSION ) {
		common->Printf( "Error initializing Maya exporter: DLL version %d different from .exe version %d\n", MD5_VERSION, version );
		return false;
	}

	if ( !initialized ) {
		MStatus	status;

		status = MLibrary::initialize( GAME_NAME, true );
		if ( !status ) {
			common->Printf( "Error calling MLibrary::initialize (%s)\n", status.errorString().asChar() );
			return false;
		}

		initialized = true;
	}

	return true;
}

// Force type checking on the interface functions to help ensure that they match the ones in the .exe
const exporterDLLEntry_t	ValidateEntry = &dllEntry;
const exporterInterface_t	ValidateConvert = &Maya_ConvertModel;
const exporterShutdown_t	ValidateShutdown = &Maya_Shutdown;

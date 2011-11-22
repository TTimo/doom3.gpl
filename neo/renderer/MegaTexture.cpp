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

#include "tr_local.h"

idCVar idMegaTexture::r_megaTextureLevel( "r_megaTextureLevel", "0", CVAR_RENDERER | CVAR_INTEGER, "draw only a specific level" );
idCVar idMegaTexture::r_showMegaTexture( "r_showMegaTexture", "0", CVAR_RENDERER | CVAR_BOOL, "display all the level images" );
idCVar idMegaTexture::r_showMegaTextureLabels( "r_showMegaTextureLabels", "0", CVAR_RENDERER | CVAR_BOOL, "draw colored blocks in each tile" );
idCVar idMegaTexture::r_skipMegaTexture( "r_skipMegaTexture", "0", CVAR_RENDERER | CVAR_INTEGER, "only use the lowest level image" );
idCVar idMegaTexture::r_terrainScale( "r_terrainScale", "3", CVAR_RENDERER | CVAR_INTEGER, "vertically scale USGS data" );

/*

allow sparse population of the upper detail tiles

*/

int RoundDownToPowerOfTwo( int num ) {
	int		pot;
	for (pot = 1 ; (pot*2) <= num ; pot<<=1) {
	}
	return pot;
}

static union {
	int		intVal;
	byte	color[4];
} fillColor;

static byte	colors[8][4] = {
	{ 0, 0, 0, 255 },
	{ 255, 0, 0, 255 },
	{ 0, 255, 0, 255 },
	{ 255, 255, 0, 255 },
	{ 0, 0, 255, 255 },
	{ 255, 0, 255, 255 },
	{ 0, 255, 255, 255 },
	{ 255, 255, 255, 255 }
};

static void R_EmptyLevelImage( idImage *image ) {
	int	c = MAX_LEVEL_WIDTH * MAX_LEVEL_WIDTH;
	byte	*data = (byte *)_alloca( c*4 );

	for ( int i = 0 ; i < c ; i++ ) {
		((int *)data)[i] = fillColor.intVal;
	}

	// FIXME: this won't live past vid mode changes
	image->GenerateImage( data, MAX_LEVEL_WIDTH, MAX_LEVEL_WIDTH, 
		TF_DEFAULT, false, TR_REPEAT, TD_HIGH_QUALITY );
}


/*
====================
InitFromMegaFile
====================
*/
bool idMegaTexture::InitFromMegaFile( const char *fileBase ) {
	idStr	name = "megaTextures/";
	name += fileBase;
	name.StripFileExtension();
	name += ".mega";

	int		width, height;

	fileHandle = fileSystem->OpenFileRead( name.c_str() );
	if ( !fileHandle ) {
		common->Printf( "idMegaTexture: failed to open %s\n", name.c_str() );
		return false;
	}

	fileHandle->Read( &header, sizeof( header ) );
	if ( header.tileSize < 64 || header.tilesWide < 1 || header.tilesHigh < 1 ) {
		common->Printf( "idMegaTexture: bad header on %s\n", name.c_str() );
		return false;
	}

	currentTriMapping = NULL;

	numLevels = 0;
	width = header.tilesWide;
	height = header.tilesHigh;

	int	tileOffset = 1;					// just past the header

	memset( levels, 0, sizeof( levels ) );
	while( 1 ) {
		idTextureLevel *level = &levels[numLevels];

		level->mega = this;
		level->tileOffset = tileOffset;
		level->tilesWide = width;
		level->tilesHigh = height;
		level->parms[0] = -1;		// initially mask everything
		level->parms[1] = 0;
		level->parms[2] = 0;
		level->parms[3] = (float)width / TILE_PER_LEVEL;
		level->Invalidate();

		tileOffset += level->tilesWide * level->tilesHigh;

		char	str[1024];
		sprintf( str, "MEGA_%s_%i", fileBase, numLevels );

		// give each level a default fill color
		for (int i = 0 ; i < 4 ; i++ ) {
			fillColor.color[i] = colors[numLevels+1][i];
		}

		levels[numLevels].image = globalImages->ImageFromFunction( str, R_EmptyLevelImage );
		numLevels++;
		
		if ( width <= TILE_PER_LEVEL && height <= TILE_PER_LEVEL ) {
			break;
		}
		width = ( width + 1 ) >> 1;
		height = ( height + 1 ) >> 1;
	}

	// force first bind to load everything
	currentViewOrigin[0] = -99999999.0f;
	currentViewOrigin[1] = -99999999.0f;
	currentViewOrigin[2] = -99999999.0f;

	return true;
}

/*
====================
SetMappingForSurface

analyzes xyz and st to create a mapping
This is not very robust, but works for rectangular grids
====================
*/
void	idMegaTexture::SetMappingForSurface( const srfTriangles_t *tri ) {
	if ( tri == currentTriMapping ) {
		return;
	}
	currentTriMapping = tri;

	if ( !tri->verts ) {
		return;
	}

	idDrawVert	origin, axis[2];

	origin.st[0] = 1.0;
	origin.st[1] = 1.0;

	axis[0].st[0] = 0;
	axis[0].st[1] = 1;

	axis[1].st[0] = 1;
	axis[1].st[1] = 0;

	for ( int i = 0 ; i < tri->numVerts ; i++ ) {
		idDrawVert	*v = &tri->verts[i];

		if ( v->st[0] <= origin.st[0] && v->st[1] <= origin.st[1] ) {
			origin = *v;
		}
		if ( v->st[0] >= axis[0].st[0] && v->st[1] <= axis[0].st[1] ) {
			axis[0] = *v;
		}
		if ( v->st[0] <= axis[1].st[0] && v->st[1] >= axis[1].st[1] ) {
			axis[1] = *v;
		}
	}

	for ( int i = 0 ; i < 2 ; i++ ) {
		idVec3	dir = axis[i].xyz - origin.xyz;
		float	texLen = axis[i].st[i] - origin.st[i];
		float	spaceLen = (axis[i].xyz - origin.xyz).Length();

		float scale = texLen / (spaceLen*spaceLen);
		dir *= scale;

		float	c = origin.xyz * dir - origin.st[i];

		localViewToTextureCenter[i][0] = dir[0];
		localViewToTextureCenter[i][1] = dir[1];
		localViewToTextureCenter[i][2] = dir[2];
		localViewToTextureCenter[i][3] = -c;
	}
}

/*
====================
BindForViewOrigin
====================
*/
void idMegaTexture::BindForViewOrigin( const idVec3 viewOrigin ) {

	SetViewOrigin( viewOrigin );

	// borderClamp image goes in texture 0
	GL_SelectTexture( 0 );
	globalImages->borderClampImage->Bind();

	// level images in higher textures, blurriest first
	for ( int i = 0 ; i < 7 ; i++ ) {
		GL_SelectTexture( 1+i );

		if ( i >= numLevels ) {
			globalImages->whiteImage->Bind();

			static float	parms[4] = { -2, -2, 0, 1 };	// no contribution
			qglProgramLocalParameter4fvARB( GL_VERTEX_PROGRAM_ARB, i, parms );
		} else {
			idTextureLevel	*level = &levels[ numLevels-1-i ];
			
			if ( r_showMegaTexture.GetBool() ) {
				if ( i & 1 ) {
					globalImages->blackImage->Bind();
				} else {
					globalImages->whiteImage->Bind();
				}
			} else {
				level->image->Bind();
			}
			qglProgramLocalParameter4fvARB( GL_VERTEX_PROGRAM_ARB, i, level->parms );
		}
	}

	float	parms[4];
	parms[0] = 0;
	parms[1] = 0;
	parms[2] = 0;
	parms[3] = 1;
	qglProgramLocalParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 7, parms );

	parms[0] = 1;
	parms[1] = 1;
	parms[2] = r_terrainScale.GetFloat();
	parms[3] = 1;
	qglProgramLocalParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 8, parms );
}

/*
====================
Unbind

This can go away once everything uses fragment programs so the enable states don't
need tracking
====================
*/
void idMegaTexture::Unbind( void ) {
	for ( int i = 0 ; i < numLevels ; i++ ) {
		GL_SelectTexture( 1+i );
		globalImages->BindNull();
	}
}


/*
====================
SetViewOrigin
====================
*/
void idMegaTexture::SetViewOrigin( const idVec3 viewOrigin ) {
	if ( r_showMegaTextureLabels.IsModified() ) {
		r_showMegaTextureLabels.ClearModified();
		currentViewOrigin[0] = viewOrigin[0] + 0.1;	// force a change
		for ( int i = 0 ; i < numLevels ; i++ ) {
			levels[i].Invalidate();
		}
	}

	if ( viewOrigin == currentViewOrigin ) {
		return;
	}
	if ( r_skipMegaTexture.GetBool() ) {
		return;
	}

	currentViewOrigin = viewOrigin;

	float	texCenter[2];

	// convert the viewOrigin to a texture center, which will
	// be a different conversion for each megaTexture
	for ( int i = 0 ; i < 2 ; i++ ) {
		texCenter[i] = 
			viewOrigin[0] * localViewToTextureCenter[i][0] +
			viewOrigin[1] * localViewToTextureCenter[i][1] +
			viewOrigin[2] * localViewToTextureCenter[i][2] +
			localViewToTextureCenter[i][3];
	}

	for ( int i = 0 ; i < numLevels ; i++ ) {
		levels[i].UpdateForCenter( texCenter );
	}
}


/*
====================
UpdateTile

A local tile will only be mapped to globalTile[ localTile + X * TILE_PER_LEVEL ] for some x
====================
*/
void idTextureLevel::UpdateTile( int localX, int localY, int globalX, int globalY ) {
	idTextureTile	*tile = &tileMap[localX][localY];

	if ( tile->x == globalX && tile->y == globalY ) {
		return;
	}
	if ( (globalX & (TILE_PER_LEVEL-1)) != localX || (globalY & (TILE_PER_LEVEL-1)) != localY ) {
		common->Error( "idTextureLevel::UpdateTile: bad coordinate mod" );
	}

	tile->x = globalX;
	tile->y = globalY;

	byte	data[ TILE_SIZE * TILE_SIZE * 4 ];

	if ( globalX >= tilesWide || globalX < 0 || globalY >= tilesHigh || globalY < 0 ) {
		// off the map
		memset( data, 0, sizeof( data ) );
	} else {
		// extract the data from the full image (FIXME: background load from disk)
		int		tileNum = tileOffset + tile->y * tilesWide + tile->x;

		int		tileSize = TILE_SIZE * TILE_SIZE * 4;

		mega->fileHandle->Seek( tileNum * tileSize, FS_SEEK_SET );
		memset( data, 128, sizeof( data ) );
		mega->fileHandle->Read( data, tileSize );
	}

	if ( idMegaTexture::r_showMegaTextureLabels.GetBool() ) {
		// put a color marker in it
		byte	color[4] = { 255 * localX / TILE_PER_LEVEL, 255 * localY / TILE_PER_LEVEL, 0, 0 };
		for ( int x = 0 ; x < 8 ; x++ ) {
			for ( int y = 0 ; y < 8 ; y++ ) {
				*(int *)&data[ ( ( y + TILE_SIZE/2 - 4 ) * TILE_SIZE + x + TILE_SIZE/2 - 4 ) * 4 ] = *(int *)color;
			}
		}
	}

	// upload all the mip-map levels
	int	level = 0;
	int size = TILE_SIZE;
	while ( 1 ) {
		qglTexSubImage2D( GL_TEXTURE_2D, level, localX * size, localY * size, size, size, GL_RGBA, GL_UNSIGNED_BYTE, data );
		size >>= 1;
		level++;

		if ( size == 0 ) {
			break;
		}

		int	byteSize = size * 4;
		// mip-map in place
		for ( int y = 0 ; y < size ; y++ ) {
			byte	*in, *in2, *out;
			in = data + y * size * 16;
			in2 = in + size * 8;
			out = data + y * size * 4;
			for ( int x = 0 ; x < size ; x++ ) {
				out[x*4+0] = ( in[x*8+0] + in[x*8+4+0] + in2[x*8+0] + in2[x*8+4+0] ) >> 2;
				out[x*4+1] = ( in[x*8+1] + in[x*8+4+1] + in2[x*8+1] + in2[x*8+4+1] ) >> 2;
				out[x*4+2] = ( in[x*8+2] + in[x*8+4+2] + in2[x*8+2] + in2[x*8+4+2] ) >> 2;
				out[x*4+3] = ( in[x*8+3] + in[x*8+4+3] + in2[x*8+3] + in2[x*8+4+3] ) >> 2;
			}
		}
	}
}

/*
====================
UpdateForCenter

Center is in the 0.0 to 1.0 range
====================
*/
void idTextureLevel::UpdateForCenter( float center[2] ) {
	int		globalTileCorner[2];
	int		localTileOffset[2];

	if ( tilesWide <= TILE_PER_LEVEL && tilesHigh <= TILE_PER_LEVEL ) {
		globalTileCorner[0] = 0;
		globalTileCorner[1] = 0;
		localTileOffset[0] = 0;
		localTileOffset[1] = 0;
		// orient the mask so that it doesn't mask anything at all
		parms[0] = 0.25;
		parms[1] = 0.25;
		parms[3] = 0.25;
	} else {
		for ( int i = 0 ; i < 2 ; i++ ) {
			float	global[2];

			// this value will be outside the 0.0 to 1.0 range unless
			// we are in the corner of the megaTexture
			global[i] = ( center[i] * parms[3] - 0.5 ) * TILE_PER_LEVEL;

			globalTileCorner[i] = (int)( global[i] + 0.5 );

			localTileOffset[i] = globalTileCorner[i] & (TILE_PER_LEVEL-1);

			// scaling for the mask texture to only allow the proper window
			// of tiles to show through
			parms[i] = -globalTileCorner[i] / (float)TILE_PER_LEVEL;
		}
	}

	image->Bind();

	for ( int x = 0 ; x < TILE_PER_LEVEL ; x++ ) {
		for ( int y = 0 ; y < TILE_PER_LEVEL ; y++ ) {
			int		globalTile[2];

			globalTile[0] = globalTileCorner[0] + ( ( x - localTileOffset[0] ) & (TILE_PER_LEVEL-1) );
			globalTile[1] = globalTileCorner[1] + ( ( y - localTileOffset[1] ) & (TILE_PER_LEVEL-1) );

			UpdateTile( x, y, globalTile[0], globalTile[1] );
		}
	}
}

/*
=====================
Invalidate

Forces all tiles to be regenerated
=====================
*/
void idTextureLevel::Invalidate() {
	for ( int x = 0 ; x < TILE_PER_LEVEL ; x++ ) {
		for ( int y = 0 ; y < TILE_PER_LEVEL ; y++ ) {
			tileMap[x][y].x =
			tileMap[x][y].y = -99999;
		}
	}
}

//===================================================================================================


typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


static byte ReadByte( idFile *f ) {
	byte	b;

	f->Read( &b, 1 );
	return b;
}

static short ReadShort( idFile *f ) {
	byte	b[2];

	f->Read( &b, 2 );

	return b[0] + ( b[1] << 8 );
}


/*
====================
GenerateMegaMipMaps
====================
*/
void	idMegaTexture::GenerateMegaMipMaps( megaTextureHeader_t *header, idFile *outFile ) {
	outFile->Flush();

	// out fileSystem doesn't allow read / write access...
	idFile	*inFile = fileSystem->OpenFileRead( outFile->GetName() );

	int	tileOffset = 1;
	int	width = header->tilesWide;
	int	height = header->tilesHigh;

	int		tileSize = header->tileSize * header->tileSize * 4;
	byte	*oldBlock = (byte *)_alloca( tileSize );
	byte	*newBlock = (byte *)_alloca( tileSize );

	while ( width > 1 || height > 1 ) {
		int	newHeight = (height+1) >> 1;
		if ( newHeight < 1 ) {
			newHeight = 1;
		}
		int	newWidth = (width+1) >> 1;
		if ( width < 1 ) {
			width = 1;
		}
		common->Printf( "generating %i x %i block mip level\n", newWidth, newHeight );

		int		tileNum;

		for ( int y = 0 ; y < newHeight ; y++ ) {
			common->Printf( "row %i\n", y );
			session->UpdateScreen();

			for ( int x = 0 ; x < newWidth ; x++ ) {
				// mip map four original blocks down into a single new block
				for ( int yy = 0 ; yy < 2 ; yy++ ) {
					for ( int xx = 0 ; xx< 2 ; xx++ ) {
						int	tx = x*2 + xx;
						int ty = y*2 + yy;

						if ( tx > width || ty > height ) {
							// off edge, zero fill
							memset( newBlock, 0, sizeof( newBlock ) );
						} else {
							tileNum = tileOffset + ty * width + tx;
							inFile->Seek( tileNum * tileSize, FS_SEEK_SET );
							inFile->Read( oldBlock, tileSize );
						}
						// mip map the new pixels
						for ( int yyy = 0 ; yyy < TILE_SIZE / 2 ; yyy++ ) {
							for ( int xxx = 0 ; xxx < TILE_SIZE / 2 ; xxx++ ) {
								byte *in = &oldBlock[ ( yyy * 2 * TILE_SIZE + xxx * 2 ) * 4 ];
								byte *out = &newBlock[ ( ( ( TILE_SIZE/2 * yy ) + yyy ) * TILE_SIZE + ( TILE_SIZE/2 * xx ) + xxx ) * 4 ];
								out[0] = ( in[0] + in[4] + in[0+TILE_SIZE*4] + in[4+TILE_SIZE*4] ) >> 2;
								out[1] = ( in[1] + in[5] + in[1+TILE_SIZE*4] + in[5+TILE_SIZE*4] ) >> 2;
								out[2] = ( in[2] + in[6] + in[2+TILE_SIZE*4] + in[6+TILE_SIZE*4] ) >> 2;
								out[3] = ( in[3] + in[7] + in[3+TILE_SIZE*4] + in[7+TILE_SIZE*4] ) >> 2;
							}
						}

						// write the block out
						tileNum = tileOffset + width * height + y * newWidth + x;
						outFile->Seek( tileNum * tileSize, FS_SEEK_SET );
						outFile->Write( newBlock, tileSize );

					}
				}
			}
		}
		tileOffset += width * height;
		width = newWidth;
		height = newHeight;
	}

	delete inFile;
}

/*
====================
GenerateMegaPreview

Make a 2k x 2k preview image for a mega texture that can be used in modeling programs
====================
*/
void	idMegaTexture::GenerateMegaPreview( const char *fileName ) {
	idFile	*fileHandle = fileSystem->OpenFileRead( fileName );
	if ( !fileHandle ) {
		common->Printf( "idMegaTexture: failed to open %s\n", fileName );
		return;
	}

	idStr	outName = fileName;
	outName.StripFileExtension();
	outName += "_preview.tga";

	common->Printf( "Creating %s.\n", outName.c_str() );

	megaTextureHeader_t header;

	fileHandle->Read( &header, sizeof( header ) );
	if ( header.tileSize < 64 || header.tilesWide < 1 || header.tilesHigh < 1 ) {
		common->Printf( "idMegaTexture: bad header on %s\n", fileName );
		return;
	}

	int	tileSize = header.tileSize;
	int	width = header.tilesWide;
	int	height = header.tilesHigh;
	int	tileOffset = 1;
	int	tileBytes = tileSize * tileSize * 4;
	// find the level that fits
	while ( width * tileSize > 2048 || height * tileSize > 2048 ) {
		tileOffset += width * height;
		width >>= 1;
		if ( width < 1 ) {
			width = 1;
		}
		height >>= 1;
		if ( height < 1 ) {
			height = 1;
		}
	}

	byte *pic = (byte *)R_StaticAlloc( width * height * tileBytes );
	byte	*oldBlock = (byte *)_alloca( tileBytes );
	for ( int y = 0 ; y < height ; y++ ) {
		for ( int x = 0 ; x < width ; x++ ) {
			int tileNum = tileOffset + y * width + x;
			fileHandle->Seek( tileNum * tileBytes, FS_SEEK_SET );
			fileHandle->Read( oldBlock, tileBytes );

			for ( int yy = 0 ; yy < tileSize ; yy++ ) {
				memcpy( pic + ( ( y * tileSize + yy ) * width * tileSize + x * tileSize  ) * 4,
					oldBlock + yy * tileSize * 4, tileSize * 4 );
			}
		}
	}

	R_WriteTGA( outName.c_str(), pic, width * tileSize, height * tileSize, false );

	R_StaticFree( pic );

	delete fileHandle;
}


/*
====================
MakeMegaTexture_f

Incrementally load a giant tga file and process into the mega texture block format
====================
*/
void idMegaTexture::MakeMegaTexture_f( const idCmdArgs &args ) {
	int		columns, rows, fileSize, numBytes;
	byte	*pixbuf;
	int		row, column;
	TargaHeader	targa_header;

	if ( args.Argc() != 2 ) {
		common->Printf( "USAGE: makeMegaTexture <filebase>\n" );
		return;
	}

	idStr	name_s = "megaTextures/";
	name_s += args.Argv(1);
	name_s.StripFileExtension();
	name_s += ".tga";

	const char	*name = name_s.c_str();

	//
	// open the file
	//
	common->Printf( "Opening %s.\n", name );
	fileSize = fileSystem->ReadFile( name, NULL, NULL );
	idFile	*file = fileSystem->OpenFileRead( name );

	if ( !file ) {
		common->Printf( "Couldn't open %s\n", name );
		return;
	}

	targa_header.id_length = ReadByte( file );
	targa_header.colormap_type = ReadByte( file );
	targa_header.image_type = ReadByte( file );
	
	targa_header.colormap_index = ReadShort( file );
	targa_header.colormap_length = ReadShort( file );
	targa_header.colormap_size = ReadByte( file );
	targa_header.x_origin = ReadShort( file );
	targa_header.y_origin = ReadShort( file );
	targa_header.width = ReadShort( file );
	targa_header.height = ReadShort( file );
	targa_header.pixel_size = ReadByte( file );
	targa_header.attributes = ReadByte( file );

	if ( targa_header.image_type != 2 && targa_header.image_type != 10 && targa_header.image_type != 3 ) {
		common->Error( "LoadTGA( %s ): Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n", name );
	}

	if ( targa_header.colormap_type != 0 ) {
		common->Error( "LoadTGA( %s ): colormaps not supported\n", name );
	}

	if ( ( targa_header.pixel_size != 32 && targa_header.pixel_size != 24 ) && targa_header.image_type != 3 ) {
		common->Error( "LoadTGA( %s ): Only 32 or 24 bit images supported (no colormaps)\n", name );
	}

	if ( targa_header.image_type == 2 || targa_header.image_type == 3 ) {
		numBytes = targa_header.width * targa_header.height * ( targa_header.pixel_size >> 3 );
		if ( numBytes > fileSize - 18 - targa_header.id_length ) {
			common->Error( "LoadTGA( %s ): incomplete file\n", name );
		}
	}

	columns = targa_header.width;
	rows = targa_header.height;

	// skip TARGA image comment
	if ( targa_header.id_length != 0 ) {
		file->Seek( targa_header.id_length, FS_SEEK_CUR );
	}
	
	megaTextureHeader_t		mtHeader;

	mtHeader.tileSize = TILE_SIZE;
	mtHeader.tilesWide = RoundDownToPowerOfTwo( targa_header.width ) / TILE_SIZE;
	mtHeader.tilesHigh = RoundDownToPowerOfTwo( targa_header.height ) / TILE_SIZE;

	idStr	outName = name;
	outName.StripFileExtension();
	outName += ".mega";

	common->Printf( "Writing %i x %i size %i tiles to %s.\n", 
		mtHeader.tilesWide, mtHeader.tilesHigh, mtHeader.tileSize, outName.c_str() );

	// open the output megatexture file
	idFile	*out = fileSystem->OpenFileWrite( outName.c_str() );

	out->Write( &mtHeader, sizeof( mtHeader ) );
	out->Seek( TILE_SIZE * TILE_SIZE * 4, FS_SEEK_SET );

	// we will process this one row of tiles at a time, since the entire thing
	// won't fit in memory
	byte	*targa_rgba = (byte *)R_StaticAlloc( TILE_SIZE * targa_header.width * 4 );

	int blockRowsRemaining = mtHeader.tilesHigh;
	while ( blockRowsRemaining-- ) {
		common->Printf( "%i blockRowsRemaining\n", blockRowsRemaining );
		session->UpdateScreen();

		if ( targa_header.image_type == 2 || targa_header.image_type == 3 ) 	{ 
			// Uncompressed RGB or gray scale image
			for( row = 0 ; row < TILE_SIZE ; row++ ) {
				pixbuf = targa_rgba + row*columns*4;
				for( column = 0; column < columns; column++) {
					unsigned char red,green,blue,alphabyte;
					switch( targa_header.pixel_size ) {
					case 8:
						blue = ReadByte( file );
						green = blue;
						red = blue;
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = 255;
						break;

					case 24:
						blue = ReadByte( file );
						green = ReadByte( file );
						red = ReadByte( file );
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = 255;
						break;
					case 32:
						blue = ReadByte( file );
						green = ReadByte( file );
						red = ReadByte( file );
						alphabyte = ReadByte( file );
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						break;
					default:
						common->Error( "LoadTGA( %s ): illegal pixel_size '%d'\n", name, targa_header.pixel_size );
						break;
					}
				}
			}
		} else if ( targa_header.image_type == 10 ) {   // Runlength encoded RGB images
			unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;

			red = 0;
			green = 0;
			blue = 0;
			alphabyte = 0xff;

			for( row = 0 ; row < TILE_SIZE ; row++ ) {
				pixbuf = targa_rgba + row*columns*4;
				for( column = 0; column < columns; ) {
					packetHeader= ReadByte( file );
					packetSize = 1 + (packetHeader & 0x7f);
					if ( packetHeader & 0x80 ) {        // run-length packet
						switch( targa_header.pixel_size ) {
							case 24:
									blue = ReadByte( file );
									green = ReadByte( file );
									red = ReadByte( file );
									alphabyte = 255;
									break;
							case 32:
									blue = ReadByte( file );
									green = ReadByte( file );
									red = ReadByte( file );
									alphabyte = ReadByte( file );
									break;
							default:
								common->Error( "LoadTGA( %s ): illegal pixel_size '%d'\n", name, targa_header.pixel_size );
								break;
						}
		
						for( j = 0; j < packetSize; j++ ) {
							*pixbuf++=red;
							*pixbuf++=green;
							*pixbuf++=blue;
							*pixbuf++=alphabyte;
							column++;
							if ( column == columns ) { // run spans across rows
								common->Error( "TGA had RLE across columns, probably breaks block" );
								column = 0;
								if ( row > 0) {
									row--;
								}
								else {
									goto breakOut;
								}
								pixbuf = targa_rgba + row*columns*4;
							}
						}
					} else {                            // non run-length packet
						for( j = 0; j < packetSize; j++ ) {
							switch( targa_header.pixel_size ) {
								case 24:
										blue = ReadByte( file );
										green = ReadByte( file );
										red = ReadByte( file );
										*pixbuf++ = red;
										*pixbuf++ = green;
										*pixbuf++ = blue;
										*pixbuf++ = 255;
										break;
								case 32:
										blue = ReadByte( file );
										green = ReadByte( file );
										red = ReadByte( file );
										alphabyte = ReadByte( file );
										*pixbuf++ = red;
										*pixbuf++ = green;
										*pixbuf++ = blue;
										*pixbuf++ = alphabyte;
										break;
								default:
									common->Error( "LoadTGA( %s ): illegal pixel_size '%d'\n", name, targa_header.pixel_size );
									break;
							}
							column++;
							if ( column == columns ) { // pixel packet run spans across rows
								column = 0;
								if ( row > 0 ) {
									row--;
								}
								else {
									goto breakOut;
								}
								pixbuf = targa_rgba + row*columns*4;
							}						
						}
					}
				}
				breakOut: ;
			}
		}

		//
		// write out individual blocks from the full row block buffer
		//
		for ( int rowBlock = 0 ; rowBlock < mtHeader.tilesWide ; rowBlock++ ) {
			for ( int y = 0 ; y < TILE_SIZE ; y++ ) {
				out->Write( targa_rgba + ( y * targa_header.width + rowBlock * TILE_SIZE ) * 4, TILE_SIZE * 4 );
			}
		}
	}

	R_StaticFree( targa_rgba );

	GenerateMegaMipMaps( &mtHeader, out );

	delete out;
	delete file;

	GenerateMegaPreview( outName.c_str() );
#if 0
	if ( (targa_header.attributes & (1<<5)) ) {			// image flp bit
		R_VerticalFlip( *pic, *width, *height );
	}
#endif
}



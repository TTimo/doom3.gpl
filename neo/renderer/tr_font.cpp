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

#ifdef BUILD_FREETYPE
#include "../ft2/fterrors.h"
#include "../ft2/ftsystem.h"
#include "../ft2/ftimage.h"
#include "../ft2/freetype.h"
#include "../ft2/ftoutln.h"

#define _FLOOR(x)  ((x) & -64)
#define _CEIL(x)   (((x)+63) & -64)
#define _TRUNC(x)  ((x) >> 6)

FT_Library ftLibrary = NULL;  
#endif


#ifdef BUILD_FREETYPE

/*
============
R_GetGlyphInfo
============
*/
void R_GetGlyphInfo(FT_GlyphSlot glyph, int *left, int *right, int *width, int *top, int *bottom, int *height, int *pitch) {

	*left  = _FLOOR( glyph->metrics.horiBearingX );
	*right = _CEIL( glyph->metrics.horiBearingX + glyph->metrics.width );
	*width = _TRUNC(*right - *left);

	*top    = _CEIL( glyph->metrics.horiBearingY );
	*bottom = _FLOOR( glyph->metrics.horiBearingY - glyph->metrics.height );
	*height = _TRUNC( *top - *bottom );
	*pitch  = ( qtrue ? (*width+3) & -4 : (*width+7) >> 3 );
}

/*
============
R_RenderGlyph
============
*/
FT_Bitmap *R_RenderGlyph(FT_GlyphSlot glyph, glyphInfo_t* glyphOut) {
	FT_Bitmap  *bit2;
	int left, right, width, top, bottom, height, pitch, size;

	R_GetGlyphInfo(glyph, &left, &right, &width, &top, &bottom, &height, &pitch);

	if ( glyph->format == ft_glyph_format_outline ) {
		size   = pitch*height; 

		bit2 = Mem_Alloc(sizeof(FT_Bitmap));

		bit2->width      = width;
		bit2->rows       = height;
		bit2->pitch      = pitch;
		bit2->pixel_mode = ft_pixel_mode_grays;
		//bit2->pixel_mode = ft_pixel_mode_mono;
		bit2->buffer     = Mem_Alloc(pitch*height);
		bit2->num_grays = 256;

		memset( bit2->buffer, 0, size );

		FT_Outline_Translate( &glyph->outline, -left, -bottom );

		FT_Outline_Get_Bitmap( ftLibrary, &glyph->outline, bit2 );

		glyphOut->height = height;
		glyphOut->pitch = pitch;
		glyphOut->top = (glyph->metrics.horiBearingY >> 6) + 1;
		glyphOut->bottom = bottom;

		return bit2;
	}
	else {
		common->Printf( "Non-outline fonts are not supported\n" );
	}
	return NULL;
}

/*
============
RE_ConstructGlyphInfo
============
*/
glyphInfo_t *RE_ConstructGlyphInfo( unsigned char *imageOut, int *xOut, int *yOut, int *maxHeight, FT_Face face, const unsigned char c, qboolean calcHeight ) {
	int i;
	static glyphInfo_t glyph;
	unsigned char *src, *dst;
	float scaled_width, scaled_height;
	FT_Bitmap *bitmap = NULL;

	memset(&glyph, 0, sizeof(glyphInfo_t));
	// make sure everything is here
	if (face != NULL) {
		FT_Load_Glyph(face, FT_Get_Char_Index( face, c), FT_LOAD_DEFAULT );
		bitmap = R_RenderGlyph(face->glyph, &glyph);
		if (bitmap) {
			glyph.xSkip = (face->glyph->metrics.horiAdvance >> 6) + 1;
		} else {
			return &glyph;
		}

		if (glyph.height > *maxHeight) {
			*maxHeight = glyph.height;
		}

		if (calcHeight) {
			Mem_Free(bitmap->buffer);
			Mem_Free(bitmap);
			return &glyph;
		}

/*
		// need to convert to power of 2 sizes so we do not get 
		// any scaling from the gl upload
		for (scaled_width = 1 ; scaled_width < glyph.pitch ; scaled_width<<=1)
			;
		for (scaled_height = 1 ; scaled_height < glyph.height ; scaled_height<<=1)
			;
*/

		scaled_width = glyph.pitch;
		scaled_height = glyph.height;

		// we need to make sure we fit
		if (*xOut + scaled_width + 1 >= 255) {
			if (*yOut + *maxHeight + 1 >= 255) {
				*yOut = -1;
				*xOut = -1;
				Mem_Free(bitmap->buffer);
				Mem_Free(bitmap);
				return &glyph;
			} else {
			*xOut = 0;
			*yOut += *maxHeight + 1;
			}
		} else if (*yOut + *maxHeight + 1 >= 255) {
			*yOut = -1;
			*xOut = -1;
			Mem_Free(bitmap->buffer);
			Mem_Free(bitmap);
			return &glyph;
		}

		src = bitmap->buffer;
		dst = imageOut + (*yOut * 256) + *xOut;

		if (bitmap->pixel_mode == ft_pixel_mode_mono) {
			for (i = 0; i < glyph.height; i++) {
				int j;
				unsigned char *_src = src;
				unsigned char *_dst = dst;
				unsigned char mask = 0x80;
				unsigned char val = *_src;
				for (j = 0; j < glyph.pitch; j++) {
					if (mask == 0x80) {
						val = *_src++;
					}
					if (val & mask) {
						*_dst = 0xff;
					}
					mask >>= 1;
        
					if ( mask == 0 ) {
						mask = 0x80;
					}
					_dst++;
				}

				src += glyph.pitch;
				dst += 256;

			}
		} else {
			for (i = 0; i < glyph.height; i++) {
				memcpy( dst, src, glyph.pitch );
				src += glyph.pitch;
				dst += 256;
			}
		}

		// we now have an 8 bit per pixel grey scale bitmap 
		// that is width wide and pf->ftSize->metrics.y_ppem tall

		glyph.imageHeight = scaled_height;
		glyph.imageWidth = scaled_width;
		glyph.s = (float)*xOut / 256;
		glyph.t = (float)*yOut / 256;
		glyph.s2 = glyph.s + (float)scaled_width / 256;
		glyph.t2 = glyph.t + (float)scaled_height / 256;

		*xOut += scaled_width + 1;
	}

	Mem_Free(bitmap->buffer);
	Mem_Free(bitmap);

	return &glyph;
}

#endif

static int fdOffset;
static byte	*fdFile;

/*
============
readInt
============
*/
int readInt( void ) {
	int i = fdFile[fdOffset]+(fdFile[fdOffset+1]<<8)+(fdFile[fdOffset+2]<<16)+(fdFile[fdOffset+3]<<24);
	fdOffset += 4;
	return i;
}

typedef union {
	byte	fred[4];
	float	ffred;
} poor;

/*
============
readFloat
============
*/
float readFloat( void ) {
	poor	me;
#ifdef __ppc__
	me.fred[0] = fdFile[fdOffset+3];
	me.fred[1] = fdFile[fdOffset+2];
	me.fred[2] = fdFile[fdOffset+1];
	me.fred[3] = fdFile[fdOffset+0];
#else
	me.fred[0] = fdFile[fdOffset+0];
	me.fred[1] = fdFile[fdOffset+1];
	me.fred[2] = fdFile[fdOffset+2];
	me.fred[3] = fdFile[fdOffset+3];
#endif
	fdOffset += 4;
	return me.ffred;
}

/*
============
RegisterFont

Loads 3 point sizes, 12, 24, and 48
============
*/
bool idRenderSystemLocal::RegisterFont( const char *fontName, fontInfoEx_t &font ) {
#ifdef BUILD_FREETYPE
	FT_Face face;
	int j, k, xOut, yOut, lastStart, imageNumber;
	int scaledSize, newSize, maxHeight, left, satLevels;
	unsigned char *out, *imageBuff;
	glyphInfo_t *glyph;
	idImage *image;
	idMaterial *h;
	float max;
#endif
	void *faceData;
	ID_TIME_T ftime;
	int i, len, fontCount;
	char name[1024];

	int pointSize = 12;
/*
	if ( registeredFontCount >= MAX_FONTS ) {
		common->Warning( "RegisterFont: Too many fonts registered already." );
		return false;
	}

	int pointSize = 12;
	idStr::snPrintf( name, sizeof(name), "%s/fontImage_%i.dat", fontName, pointSize );
	for ( i = 0; i < registeredFontCount; i++ ) {
		if ( idStr::Icmp(name, registeredFont[i].fontInfoSmall.name) == 0 ) {
			memcpy( &font, &registeredFont[i], sizeof( fontInfoEx_t ) );
			return true;
		}
	}
*/

	memset( &font, 0, sizeof( font ) );

	for ( fontCount = 0; fontCount < 3; fontCount++ ) {

		if ( fontCount == 0) {
			pointSize = 12;
		} else if ( fontCount == 1 ) {
			pointSize = 24;
		} else {
			pointSize = 48;
		}
		// we also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font
		float glyphScale = 1.0f; 		// change the scale to be relative to 1 based on 72 dpi ( so dpi of 144 means a scale of .5 )
		glyphScale *= 48.0f / pointSize;

		idStr::snPrintf( name, sizeof(name), "%s/fontImage_%i.dat", fontName, pointSize );

		fontInfo_t *outFont;
		if ( fontCount == 0 ) {
			outFont = &font.fontInfoSmall;
		}
		else if ( fontCount == 1 ) {
			outFont = &font.fontInfoMedium;
		}
		else {
			outFont = &font.fontInfoLarge;
		}

		idStr::Copynz( outFont->name, name, sizeof( outFont->name ) );

		len = fileSystem->ReadFile( name, NULL, &ftime );
		if ( len != sizeof( fontInfo_t ) ) {
			common->Warning( "RegisterFont: couldn't find font: '%s'", name );
			return false;
		}

		fileSystem->ReadFile( name, &faceData, &ftime );
		fdOffset = 0;
		fdFile = reinterpret_cast<unsigned char*>(faceData);
		for( i = 0; i < GLYPHS_PER_FONT; i++ ) {
			outFont->glyphs[i].height		= readInt();
			outFont->glyphs[i].top			= readInt();
			outFont->glyphs[i].bottom		= readInt();
			outFont->glyphs[i].pitch		= readInt();
			outFont->glyphs[i].xSkip		= readInt();
			outFont->glyphs[i].imageWidth	= readInt();
			outFont->glyphs[i].imageHeight	= readInt();
			outFont->glyphs[i].s			= readFloat();
			outFont->glyphs[i].t			= readFloat();
			outFont->glyphs[i].s2			= readFloat();
			outFont->glyphs[i].t2			= readFloat();
			int junk /* font.glyphs[i].glyph */		= readInt();
			//FIXME: the +6, -6 skips the embedded fonts/ 
			memcpy( outFont->glyphs[i].shaderName, &fdFile[fdOffset + 6], 32 - 6 );
			fdOffset += 32;
		}
		outFont->glyphScale = readFloat();

		int mw = 0;
		int mh = 0;
		for (i = GLYPH_START; i < GLYPH_END; i++) {
			idStr::snPrintf(name, sizeof(name), "%s/%s", fontName, outFont->glyphs[i].shaderName);
			outFont->glyphs[i].glyph = declManager->FindMaterial(name);
			outFont->glyphs[i].glyph->SetSort( SS_GUI );
			if (mh < outFont->glyphs[i].height) {
				mh = outFont->glyphs[i].height;
			}
			if (mw < outFont->glyphs[i].xSkip) {
				mw = outFont->glyphs[i].xSkip;
			}
		}
		if (fontCount == 0) {
			font.maxWidthSmall = mw;
			font.maxHeightSmall = mh;
		} else if (fontCount == 1) {
			font.maxWidthMedium = mw;
			font.maxHeightMedium = mh;
		} else {
			font.maxWidthLarge = mw;
			font.maxHeightLarge = mh;
		}
		fileSystem->FreeFile( faceData );
	}

	//memcpy( &registeredFont[registeredFontCount++], &font, sizeof( fontInfoEx_t ) );

	return true ;

#ifndef BUILD_FREETYPE
    common->Warning( "RegisterFont: couldn't load FreeType code %s", name );
#else

	if (ftLibrary == NULL) {
		common->Warning( "RegisterFont: FreeType not initialized." );
		return;
	}

	len = fileSystem->ReadFile(fontName, &faceData, &ftime);
	if ( len <= 0 ) {
		common->Warning( "RegisterFont: Unable to read font file" );
		return;
	}

	// allocate on the stack first in case we fail
	if ( FT_New_Memory_Face( ftLibrary, faceData, len, 0, &face ) ) {
		common->Warning( "RegisterFont: FreeType2, unable to allocate new face." );
		return;
	}


	if ( FT_Set_Char_Size( face, pointSize << 6, pointSize << 6, dpi, dpi) ) {
		common->Warning( "RegisterFont: FreeType2, Unable to set face char size." );
		return;
	}

	// font = registeredFonts[registeredFontCount++];

	// make a 256x256 image buffer, once it is full, register it, clean it and keep going 
	// until all glyphs are rendered

	out = Mem_Alloc( 1024*1024 );
	if ( out == NULL ) {
		common->Warning( "RegisterFont: Mem_Alloc failure during output image creation." );
		return;
	}
	memset( out, 0, 1024*1024 );

	maxHeight = 0;

	for (i = GLYPH_START; i < GLYPH_END; i++) {
		glyph = RE_ConstructGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qtrue);
	}

	xOut = 0;
	yOut = 0;
	i = GLYPH_START;
	lastStart = i;
	imageNumber = 0;

	while ( i <= GLYPH_END ) {

		glyph = RE_ConstructGlyphInfo(out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qfalse);

		if (xOut == -1 || yOut == -1 || i == GLYPH_END)  {
			// ran out of room
			// we need to create an image from the bitmap, set all the handles in the glyphs to this point
			// 

			scaledSize = 256*256;
			newSize = scaledSize * 4;
			imageBuff = Mem_Alloc(newSize);
			left = 0;
			max = 0;
			satLevels = 255;
			for ( k = 0; k < (scaledSize) ; k++ ) {
				if (max < out[k]) {
					max = out[k];
				}
			}

			if (max > 0) {
				max = 255/max;
			}

			for ( k = 0; k < (scaledSize) ; k++ ) {
				imageBuff[left++] = 255;
				imageBuff[left++] = 255;
				imageBuff[left++] = 255;
				imageBuff[left++] = ((float)out[k] * max);
			}

			idStr::snprintf( name, sizeof(name), "fonts/fontImage_%i_%i.tga", imageNumber++, pointSize );
			if (r_saveFontData->integer) { 
				R_WriteTGA(name, imageBuff, 256, 256);
			}

			//idStr::snprintf( name, sizeof(name), "fonts/fontImage_%i_%i", imageNumber++, pointSize );
			image = R_CreateImage(name, imageBuff, 256, 256, qfalse, qfalse, GL_CLAMP);
			h = RE_RegisterShaderFromImage(name, LIGHTMAP_2D, image, qfalse);
			for (j = lastStart; j < i; j++) {
				font.glyphs[j].glyph = h;
				idStr::Copynz( font.glyphs[j].shaderName, name, sizeof( font.glyphs[j].shaderName ) );
			}
			lastStart = i;
			memset( out, 0, 1024*1024 );
			xOut = 0;
			yOut = 0;
			Mem_Free( imageBuff );
			i++;
		} else {
			memcpy( &font.glyphs[i], glyph, sizeof( glyphInfo_t ) );
			i++;
		}
	}

	registeredFont[registeredFontCount].glyphScale = glyphScale;
	font.glyphScale = glyphScale;
	memcpy( &registeredFont[registeredFontCount++], &font, sizeof( fontInfo_t ) );

	if ( r_saveFontData->integer ) { 
		fileSystem->WriteFile( va( "fonts/fontImage_%i.dat", pointSize), &font, sizeof( fontInfo_t ) );
	}

	Mem_Free( out );

	fileSystem->FreeFile( faceData );
#endif
	return true;
}

/*
============
R_InitFreeType
============
*/
void R_InitFreeType( void ) {
#ifdef BUILD_FREETYPE
	if ( FT_Init_FreeType( &ftLibrary ) ) {
		common->Printf( "R_InitFreeType: Unable to initialize FreeType.\n" );
	}
#endif
//	registeredFontCount = 0;
}

/*
============
R_DoneFreeType
============
*/
void R_DoneFreeType( void ) {
#ifdef BUILD_FREETYPE
	if ( ftLibrary ) {
		FT_Done_FreeType( ftLibrary );
		ftLibrary = NULL;
	}
#endif
//	registeredFontCount = 0;
}

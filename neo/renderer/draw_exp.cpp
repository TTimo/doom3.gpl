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
#include "../sys/win32/win_local.h"

/*

strictly experimental / research codepaths

!!!if we use front facing occluders, we can portal flow from light centers

try depth_component_16 rendering

do we care about portals from light perspective? back / front face issues.

how do we do weapon depth hacks with shadow buffers?
	distort their world space vertexes instead of offsetting their depth?

jittering off the side of a projection will give wrong shadows

really huge lights, like sunlight, are going to be problematic with fixed projections
	we could tile the projections and let the auto-resize cut them down as necessary

It sucks that depth buffers are non-linear, because the bias and compares change with distance

polygon offset factor causes occasional texture holes from highly angled textures

*/

static	bool		initialized;

static	int lightBufferSize = 1024;
static	int	maxLightBufferSize = 1024;
static float lightBufferSizeFraction = 0.5;

static	int viewBufferSize = 1024;
static	int	viewBufferHeight = 768;
static	int	maxViewBufferSize = 1024;
static float viewBufferSizeFraction = 0.5;
static float viewBufferHeightFraction = 0.5;
static	bool	nativeViewBuffer = false;		// true if viewBufferSize is the viewport width

static	HPBUFFERARB	floatPbuffer;
static	HDC			floatPbufferDC;
static	idImage		*floatPbufferImage;

static	HPBUFFERARB	floatPbuffer2;
static	HDC			floatPbuffer2DC;
static	idImage		*floatPbuffer2Image;

static	HPBUFFERARB	floatPbufferQuarter;
static	HDC			floatPbufferQuarterDC;
static	idImage		*floatPbufferQuarterImage;

static	HGLRC		floatContext;

static	HPBUFFERARB	shadowPbuffer;
static	HDC			shadowPbufferDC;

static	HPBUFFERARB	viewPbuffer;
static	HDC			viewPbufferDC;

static	idImage		*shadowImage[3];

static	idImage		*viewDepthImage;
static	idImage		*viewAlphaImage;

static	idImage		*viewShadowImage;

static	idImage		*jitterImage16;
static	idImage		*jitterImage4;
static	idImage		*jitterImage1;

static	idImage		*random256Image;

static	int			shadowVertexProgram;
static	int			shadowFragmentProgram16;
static	int			shadowFragmentProgram4;
static	int			shadowFragmentProgram1;
static	int			shadowFragmentProgram0;

static	int			screenSpaceShadowVertexProgram;
static	int			screenSpaceShadowFragmentProgram16;
static	int			screenSpaceShadowFragmentProgram4;
static	int			screenSpaceShadowFragmentProgram1;
static	int			screenSpaceShadowFragmentProgram0;

static	int			depthMidpointVertexProgram;
static	int			depthMidpointFragmentProgram;

static	int			shadowResampleVertexProgram;
static	int			shadowResampleFragmentProgram;

static	int			gammaDitherVertexProgram;
static	int			gammaDitherFragmentProgram;

static	int			downSampleVertexProgram;
static	int			downSampleFragmentProgram;

static	int			bloomVertexProgram;
static	int			bloomFragmentProgram;

static	float		viewLightAxialSize;

idCVar r_sb_lightResolution( "r_sb_lightResolution", "1024", CVAR_RENDERER | CVAR_INTEGER, "Pixel dimensions for each shadow buffer, 64 - 2048" );
idCVar r_sb_viewResolution( "r_sb_viewResolution", "1024", CVAR_RENDERER | CVAR_INTEGER, "Width of screen space shadow sampling" );
idCVar r_sb_noShadows( "r_sb_noShadows", "0", CVAR_RENDERER | CVAR_BOOL, "don't draw any occluders" );
idCVar r_sb_usePbuffer( "r_sb_usePbuffer", "1", CVAR_RENDERER | CVAR_BOOL, "draw offscreen" );
idCVar r_sb_jitterScale( "r_sb_jitterScale", "0.006", CVAR_RENDERER | CVAR_FLOAT, "scale factor for jitter offset" );
idCVar r_sb_biasScale( "r_sb_biasScale", "0.0001", CVAR_RENDERER | CVAR_FLOAT, "scale factor for jitter bias" );
idCVar r_sb_samples( "r_sb_samples", "4", CVAR_RENDERER | CVAR_INTEGER, "0, 1, 4, or 16" );
idCVar r_sb_randomize( "r_sb_randomize", "1", CVAR_RENDERER | CVAR_BOOL, "randomly offset jitter texture each draw" );
// polyOfsFactor causes holes in low res images
idCVar r_sb_polyOfsFactor( "r_sb_polyOfsFactor", "2", CVAR_RENDERER | CVAR_FLOAT, "polygonOffset factor for drawing shadow buffer" );
idCVar r_sb_polyOfsUnits( "r_sb_polyOfsUnits", "3000", CVAR_RENDERER | CVAR_FLOAT, "polygonOffset units for drawing shadow buffer" );
idCVar r_sb_occluderFacing( "r_sb_occluderFacing", "0", CVAR_RENDERER | CVAR_INTEGER, "0 = front faces, 1 = back faces, 2 = midway between" );
// r_sb_randomizeBufferOrientation?

idCVar r_sb_frustomFOV( "r_sb_frustomFOV", "92", CVAR_RENDERER | CVAR_FLOAT, "oversize FOV for point light side matching" );
idCVar r_sb_showFrustumPixels( "r_sb_showFrustumPixels", "0", CVAR_RENDERER | CVAR_BOOL, "color the pixels contained in the frustum" );
idCVar r_sb_singleSide( "r_sb_singleSide", "-1", CVAR_RENDERER | CVAR_INTEGER, "only draw a single side (0-5) of point lights" );
idCVar r_sb_useCulling( "r_sb_useCulling", "1", CVAR_RENDERER | CVAR_BOOL, "cull geometry to individual side frustums" );
idCVar r_sb_linearFilter( "r_sb_linearFilter", "1", CVAR_RENDERER | CVAR_BOOL, "use GL_LINEAR instead of GL_NEAREST on shadow maps" );

idCVar r_sb_screenSpaceShadow( "r_sb_screenSpaceShadow", "1", CVAR_RENDERER | CVAR_BOOL, "build shadows in screen space instead of on surfaces" );

idCVar r_hdr_useFloats( "r_hdr_useFloats", "0", CVAR_RENDERER | CVAR_BOOL, "use a floating point rendering buffer" );
idCVar r_hdr_exposure( "r_hdr_exposure", "1.0", CVAR_RENDERER | CVAR_FLOAT, "maximum light scale" );
idCVar r_hdr_bloomFraction( "r_hdr_bloomFraction", "0.1", CVAR_RENDERER | CVAR_FLOAT, "fraction to smear across neighbors" );
idCVar r_hdr_gamma( "r_hdr_gamma", "1", CVAR_RENDERER | CVAR_FLOAT, "monitor gamma power" );
idCVar r_hdr_monitorDither( "r_hdr_monitorDither", "0.01", CVAR_RENDERER | CVAR_FLOAT, "random dither in monitor space" );

// from world space to light origin, looking down the X axis
static float	unflippedLightMatrix[16];

// from world space to OpenGL view space, looking down the negative Z axis
static float	lightMatrix[16];

// from OpenGL view space to OpenGL NDC ( -1 : 1 in XYZ )
static float	lightProjectionMatrix[16];


void	RB_ARB2_DrawInteraction( const drawInteraction_t *din );

typedef struct {
	const char	*name;
	int			num;
} wglString_t;

wglString_t	wglString[] = {
{ "WGL_NUMBER_PIXEL_FORMATS_ARB",		0x2000 },
{ "WGL_DRAW_TO_WINDOW_ARB",			0x2001 },
{ "WGL_DRAW_TO_BITMAP_ARB",			0x2002 },
{ "WGL_ACCELERATION_ARB",			0x2003 },
{ "WGL_NEED_PALETTE_ARB",			0x2004 },
{ "WGL_NEED_SYSTEM_PALETTE_ARB",		0x2005 },
{ "WGL_SWAP_LAYER_BUFFERS_ARB",		0x2006 },
{ "WGL_SWAP_METHOD_ARB",			0x2007 },
{ "WGL_NUMBER_OVERLAYS_ARB",			0x2008 },
{ "WGL_NUMBER_UNDERLAYS_ARB",		0x2009 },
{ "WGL_TRANSPARENT_ARB",			0x200A },
{ "WGL_TRANSPARENT_RED_VALUE_ARB",		0x2037 },
{ "WGL_TRANSPARENT_GREEN_VALUE_ARB",		0x2038 },
{ "WGL_TRANSPARENT_BLUE_VALUE_ARB",		0x2039 },
{ "WGL_TRANSPARENT_ALPHA_VALUE_ARB",		0x203A },
{ "WGL_TRANSPARENT_INDEX_VALUE_ARB",		0x203B },
{ "WGL_SHARE_DEPTH_ARB",			0x200C },
{ "WGL_SHARE_STENCIL_ARB",			0x200D },
{ "WGL_SHARE_ACCUM_ARB",			0x200E },
{ "WGL_SUPPORT_GDI_ARB",			0x200F },
{ "WGL_SUPPORT_OPENGL_ARB",			0x2010 },
{ "WGL_DOUBLE_BUFFER_ARB",			0x2011 },
{ "WGL_STEREO_ARB",				0x2012 },
{ "WGL_PIXEL_TYPE_ARB",			0x2013 },
{ "WGL_COLOR_BITS_ARB",			0x2014 },
{ "WGL_RED_BITS_ARB",			0x2015 },
{ "WGL_RED_SHIFT_ARB",			0x2016 },
{ "WGL_GREEN_BITS_ARB",			0x2017 },
{ "WGL_GREEN_SHIFT_ARB",			0x2018 },
{ "WGL_BLUE_BITS_ARB",			0x2019 },
{ "WGL_BLUE_SHIFT_ARB",			0x201A },
{ "WGL_ALPHA_BITS_ARB",			0x201B },
{ "WGL_ALPHA_SHIFT_ARB",			0x201C },
{ "WGL_ACCUM_BITS_ARB",			0x201D },
{ "WGL_ACCUM_RED_BITS_ARB",			0x201E },
{ "WGL_ACCUM_GREEN_BITS_ARB",		0x201F },
{ "WGL_ACCUM_BLUE_BITS_ARB",			0x2020 },
{ "WGL_ACCUM_ALPHA_BITS_ARB",		0x2021 },
{ "WGL_DEPTH_BITS_ARB",			0x2022 },
{ "WGL_STENCIL_BITS_ARB",			0x2023 },
{ "WGL_AUX_BUFFERS_ARB",			0x2024 },

{ "WGL_NO_ACCELERATION_ARB",			0x2025 },
{ "WGL_GENERIC_ACCELERATION_ARB",		0x2026 },
{ "WGL_FULL_ACCELERATION_ARB",		0x2027 },

{ "WGL_SWAP_EXCHANGE_ARB",			0x2028 },
{ "WGL_SWAP_COPY_ARB",			0x2029 },
{ "WGL_SWAP_UNDEFINED_ARB",			0x202A },

{ "WGL_TYPE_RGBA_ARB",			0x202B },
{ "WGL_TYPE_COLORINDEX_ARB",			0x202C },
};

static const int NUM_WGL_STRINGS = sizeof( wglString ) / sizeof( wglString[0] );

static void R_CheckWglErrors( void ) {
	int	err = GetLastError();
	char	*name;

#if 0
	LPVOID lpMsgBuf;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					err,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
					);
#endif
	err &= 0xffff;
	switch ( err ) {
	case 13: name = "ERROR_INVALID_DATA"; break;
	case 6: name = "ERROR_INVALID_HANDLE"; break;
	case 4317: name = "ERROR_INVALID_OPERATION"; break;
	default: name = va( "code %i", err ); break;
	}

	common->Printf( "GetLastError: %s\n", name );
}

static void R_MakeCurrent( HDC dc, HGLRC context, HPBUFFERARB pbuffer ) {
	if ( pbuffer ) {
		if ( !wglReleaseTexImageARB( pbuffer, WGL_FRONT_LEFT_ARB ) ) {
			R_CheckWglErrors();
			common->Error( "wglReleaseTexImageARB failed" );
		}
	}
	if ( !qwglMakeCurrent( dc, context ) ) {
		R_CheckWglErrors();
		common->FatalError( "qwglMakeCurrent failed" );
	}
}

static void R_BindTexImage( HPBUFFERARB pbuffer ) {
	if ( !wglReleaseTexImageARB( pbuffer, WGL_FRONT_LEFT_ARB ) ) {
		R_CheckWglErrors();
		common->Error( "wglReleaseTexImageARB failed" );
	}
	if ( !wglBindTexImageARB( pbuffer, WGL_FRONT_LEFT_ARB ) ) {
		R_CheckWglErrors();
		common->Error( "failed wglBindTexImageARB" );
	}
}

static void R_ReportTextureParms( void ) {
	int	parms[8];

//	q glGetTexParameteriv( GL_TEXTURE_RECTANGLE_NV, 
	qglGetIntegerv( GL_TEXTURE_BINDING_RECTANGLE_NV, parms );

}

/*
====================
RB_CreateBloomTable
====================
*/
static const int	BLOOM_RADIUS = 8;
static void RB_CreateBloomTable( void ) {
	float	bloom[BLOOM_RADIUS];
	float	total = 0;

	// gaussian
	float	stdDev = 2.0;
	for ( int i = 0 ; i < BLOOM_RADIUS ; i++ ) {
		float	f = (float)i / stdDev;
		bloom[i] = exp( -0.5 * f * f );
		total += bloom[i];
	}

	total = ( total - bloom[0] ) * 2 + bloom[0];

	// normalize to 1.0 contribution, so a full row or column will equal 1.0
	for ( int i = 0 ; i < BLOOM_RADIUS ; i++ ) {
		bloom[i] *= 1.0 / total;
		common->Printf( "PARAM bloom%i = { %f };\n", i, bloom[i] );
	}

}

/*
====================
GL_SelectTextureNoClient
====================
*/
static void GL_SelectTextureNoClient( int unit ) {
	backEnd.glState.currenttmu = unit;
	qglActiveTextureARB( GL_TEXTURE0_ARB + unit );
	RB_LogComment( "glActiveTextureARB( %i )\n", unit );
}


/*
================
R_CreateShadowBufferImage

================
*/
static void R_CreateShadowBufferImage( idImage *image ) {
	byte	*data = (byte *)Mem_Alloc( lightBufferSize*lightBufferSize );

	memset( data, 0, lightBufferSize*lightBufferSize );

	image->GenerateImage( (byte *)data, 4, 4, 
		TF_LINEAR, false, TR_CLAMP_TO_BORDER, TD_HIGH_QUALITY );

	// now reset it to a shadow depth image
	GL_CheckErrors();
	image->uploadWidth = image->uploadHeight = lightBufferSize; 
	qglTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24_ARB, lightBufferSize, lightBufferSize, 
		0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data );

	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE );
//	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL );

	// explicit zero depth border
	float	color[4];
	color[0] = color[1] = color[2] = color[3] = 0;
	qglTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color );

	GL_CheckErrors();

	Mem_Free( data );
}

static void R_CreateViewAlphaImage( idImage *image ) {
	int		c = viewBufferSize*viewBufferSize*4;
	byte	*data = (byte *)Mem_Alloc( c );

	// don't let it pick an intensity format
	for ( int i = 0 ; i < c ; i++ ) {
		data[i] = i;
	}
	memset( data, 0, viewBufferSize*viewBufferSize );

	image->GenerateImage( (byte *)data, viewBufferSize, viewBufferSize, 
		TF_LINEAR, false, TR_CLAMP, TD_HIGH_QUALITY );
}

static void R_CreateStubImage( idImage *image ) {
	float	data[3][4][4];

	// generate the texture number
	qglGenTextures( 1, &image->texnum );
	qglBindTexture( GL_TEXTURE_RECTANGLE_NV, image->texnum );
	memset( data, 0, sizeof( data ) );
	glTexImage2D( GL_TEXTURE_RECTANGLE_NV, 0, GL_FLOAT_RGBA16_NV, 4, 3, 0, GL_RGBA, GL_FLOAT, &data );
}

/*
================
R_CreateJitterImage

================
*/
const static	int JITTER_SIZE = 128;
static void R_CreateJitterImage16( idImage *image ) {
	byte	data[JITTER_SIZE][JITTER_SIZE*16][4];

	for ( int i = 0 ; i < JITTER_SIZE ; i++ ) {
		for ( int s = 0 ; s < 16 ; s++ ) {
			int sOfs = 64 * ( s & 3 );
			int tOfs = 64 * ( ( s >> 2 ) & 3 );

			for ( int j = 0 ; j < JITTER_SIZE ; j++ ) {
				data[i][s*JITTER_SIZE+j][0] = (rand() & 63 ) | sOfs;
				data[i][s*JITTER_SIZE+j][1] = (rand() & 63 ) | tOfs;
				data[i][s*JITTER_SIZE+j][2] = rand();
				data[i][s*JITTER_SIZE+j][3] = 0;
			}
		}
	}

	image->GenerateImage( (byte *)data, JITTER_SIZE*16, JITTER_SIZE, 
		TF_NEAREST, false, TR_REPEAT, TD_HIGH_QUALITY );
}

static void R_CreateJitterImage4( idImage *image ) {
	byte	data[JITTER_SIZE][JITTER_SIZE*4][4];

	for ( int i = 0 ; i < JITTER_SIZE ; i++ ) {
		for ( int s = 0 ; s < 4 ; s++ ) {
			int sOfs = 128 * ( s & 1 );
			int tOfs = 128 * ( ( s >> 1 ) & 1 );

			for ( int j = 0 ; j < JITTER_SIZE ; j++ ) {
				data[i][s*JITTER_SIZE+j][0] = (rand() & 127 ) | sOfs;
				data[i][s*JITTER_SIZE+j][1] = (rand() & 127 ) | tOfs;
				data[i][s*JITTER_SIZE+j][2] = rand();
				data[i][s*JITTER_SIZE+j][3] = 0;
			}
		}
	}

	image->GenerateImage( (byte *)data, JITTER_SIZE*4, JITTER_SIZE, 
		TF_NEAREST, false, TR_REPEAT, TD_HIGH_QUALITY );
}

static void R_CreateJitterImage1( idImage *image ) {
	byte	data[JITTER_SIZE][JITTER_SIZE][4];

	for ( int i = 0 ; i < JITTER_SIZE ; i++ ) {
		for ( int j = 0 ; j < JITTER_SIZE ; j++ ) {
			data[i][j][0] = rand();
			data[i][j][1] = rand();
			data[i][j][2] = rand();
			data[i][j][3] = 0;
		}
	}

	image->GenerateImage( (byte *)data, JITTER_SIZE, JITTER_SIZE, 
		TF_NEAREST, false, TR_REPEAT, TD_HIGH_QUALITY );
}

static void R_CreateRandom256Image( idImage *image ) {
	byte	data[256][256][4];

	for ( int i = 0 ; i < 256 ; i++ ) {
		for ( int j = 0 ; j < 256 ; j++ ) {
			data[i][j][0] = rand();
			data[i][j][1] = rand();
			data[i][j][2] = rand();
			data[i][j][3] = rand();
		}
	}

	image->GenerateImage( (byte *)data, 256, 256, 
		TF_NEAREST, false, TR_REPEAT, TD_HIGH_QUALITY );
}


/*
==================
R_PrintPixelFormat
==================
*/
void R_PrintPixelFormat( int pixelFormat ) {
	int		res;
	int		iAttribute;
	int	iValue;

	common->Printf( "----- pixelFormat %i -----\n", pixelFormat );

	for ( int i = 1 ; i < NUM_WGL_STRINGS ; i++ ) {
		iAttribute = wglString[i].num;
		res = wglGetPixelFormatAttribivARB( win32.hDC, pixelFormat, 0, 1, &iAttribute, &iValue );
		if ( res && iValue ) {
			common->Printf( "%s : %i\n", wglString[i].name, iValue );
		}
	}
}


/*
==================
R_Exp_Allocate
==================
*/
void R_Exp_Allocate( void ) {
	// find a pixel format for our floating point pbuffer
	int		iAttributes[NUM_WGL_STRINGS*2], *atr_p;
	FLOAT	fAttributes[] = {0, 0};
	UINT	numFormats;
	int		pixelformats[1024];
	int		ret;
	int	pbiAttributes[] = {0, 0};

	initialized = true;

#if 1
	//
	// allocate the floating point rendering buffer
	//
	atr_p = iAttributes;

	*atr_p++ = WGL_DRAW_TO_PBUFFER_ARB;
	*atr_p++ = TRUE;
	*atr_p++ = WGL_FLOAT_COMPONENTS_NV;
	*atr_p++ = TRUE;
	*atr_p++ = WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_RGBA_NV;
	*atr_p++ = TRUE;
//	*atr_p++ = WGL_BIND_TO_TEXTURE_RGBA_ARB;
//	*atr_p++ = TRUE;
	*atr_p++ = WGL_DEPTH_BITS_ARB;
	*atr_p++ = 24;
	*atr_p++ = WGL_STENCIL_BITS_ARB;
	*atr_p++ = 8;
	*atr_p++ = 0;
	*atr_p++ = 0;

	ret = wglChoosePixelFormatARB( win32.hDC, iAttributes, fAttributes, 
		sizeof( pixelformats ) / sizeof( pixelformats[0] ), pixelformats, &numFormats );

#if 0
	for ( int i = 0 ; i < (int)numFormats ; i++ ) {
		R_PrintPixelFormat( pixelformats[i] );
	}
#endif
	common->Printf( "\nfloatPbuffer:\n" );
	R_PrintPixelFormat( pixelformats[0] );

	// allocate a pbuffer with this pixel format
	int	pbiAttributesTexture[] = {
		WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_FLOAT_RGBA_NV, 
		WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_RECTANGLE_NV, // WGL_TEXTURE_2D_ARB,
			0, 0};

	floatPbuffer = wglCreatePbufferARB( win32.hDC, pixelformats[0], glConfig.vidWidth,
		glConfig.vidHeight, pbiAttributesTexture );
	if ( !floatPbuffer ) {
		common->Printf( "failed to create floatPbuffer.\n" );
		GL_CheckErrors();
	}
	floatPbufferDC = wglGetPbufferDCARB( floatPbuffer );
	floatPbufferImage = globalImages->ImageFromFunction( "_floatPbuffer", R_CreateStubImage );

	// create a second buffer for ping-pong operations
	floatPbuffer2 = wglCreatePbufferARB( win32.hDC, pixelformats[0], glConfig.vidWidth,
		glConfig.vidHeight, pbiAttributesTexture );
	if ( !floatPbuffer2 ) {
		common->Printf( "failed to create floatPbuffer.\n" );
		GL_CheckErrors();
	}
	floatPbuffer2DC = wglGetPbufferDCARB( floatPbuffer2 );
	floatPbuffer2Image = globalImages->ImageFromFunction( "_floatPbuffer2", R_CreateStubImage );

	// create a third buffer for down sampling operations
	floatPbufferQuarter = wglCreatePbufferARB( win32.hDC, pixelformats[0], glConfig.vidWidth / 4,
		glConfig.vidHeight / 4, pbiAttributesTexture );
	if ( !floatPbufferQuarter ) {
		common->Printf( "failed to create floatPbuffer.\n" );
		GL_CheckErrors();
	}
	floatPbufferQuarterDC = wglGetPbufferDCARB( floatPbufferQuarter );
	floatPbufferQuarterImage = globalImages->ImageFromFunction( "floatPbufferQuarter", R_CreateStubImage );

	// create a new GL context for this pixel format and share textures
	floatContext = wglCreateContext( floatPbufferDC ); 
	if ( !floatContext ) {
		common->Printf( "failed to create context for floatPbufferDC.\n" );
		GL_CheckErrors();
	}

	if ( !wglShareLists( floatContext, win32.hGLRC ) ) {
		common->Printf( "failed to share lists.\n" );
	}

	// create a rendering context for this pixel format and share textures

	// allocate a texture for the rendering 

#endif

	//=================================================================================

	//
	// allocate the shadow pbuffer
	//
	atr_p = iAttributes;

	*atr_p++ = WGL_DRAW_TO_PBUFFER_ARB;
	*atr_p++ = TRUE;
	*atr_p++ = WGL_RED_BITS_ARB;
	*atr_p++ = 8;
	*atr_p++ = WGL_GREEN_BITS_ARB;
	*atr_p++ = 8;
	*atr_p++ = WGL_BLUE_BITS_ARB;
	*atr_p++ = 8;
	*atr_p++ = WGL_ALPHA_BITS_ARB;
	*atr_p++ = 8;
	*atr_p++ = WGL_DEPTH_BITS_ARB;
	*atr_p++ = 24;
	*atr_p++ = WGL_STENCIL_BITS_ARB;
	*atr_p++ = 8;
	*atr_p++ = 0;
	*atr_p++ = 0;

	ret = wglChoosePixelFormatARB( win32.hDC, iAttributes, fAttributes, 
		sizeof( pixelformats ) / sizeof( pixelformats[0] ), pixelformats, &numFormats );
#if 0
	for ( int i = 0 ; i < (int)numFormats ; i++ ) {
		R_PrintPixelFormat( pixelformats[i] );	
	}
#endif
	common->Printf( "\nshadowPbuffer:\n" );
	R_PrintPixelFormat( pixelformats[0] );

pixelformats[0] = win32.pixelformat;	// forced to do this by wgl...

	//-----------------------------------

	lightBufferSize = maxLightBufferSize;

	// allocate a pbuffer with this pixel format
	shadowPbuffer = wglCreatePbufferARB( win32.hDC, pixelformats[0], lightBufferSize,
		lightBufferSize, pbiAttributes );

	// allocate a rendering context for the pbuffer
	shadowPbufferDC = wglGetPbufferDCARB( shadowPbuffer );

	// generate the texture number
	shadowImage[0] = globalImages->ImageFromFunction( va("_shadowBuffer%i_0",lightBufferSize), R_CreateShadowBufferImage );
	shadowImage[1] = globalImages->ImageFromFunction( va("_shadowBuffer%i_1",lightBufferSize), R_CreateShadowBufferImage );
	shadowImage[2] = globalImages->ImageFromFunction( va("_shadowBuffer%i_2",lightBufferSize), R_CreateShadowBufferImage );

	//-----------------------------------

	lightBufferSize = maxViewBufferSize;

	// allocate a pbuffer with this pixel format
	viewPbuffer = wglCreatePbufferARB( win32.hDC, pixelformats[0], maxViewBufferSize,
		maxViewBufferSize, pbiAttributes );

	// allocate a rendering context for the pbuffer
	viewPbufferDC = wglGetPbufferDCARB( viewPbuffer );

	// create the image space depth buffer for image-space shadow trnasforms
	viewDepthImage = globalImages->ImageFromFunction("_viewDepth", R_CreateShadowBufferImage );

	// create the image space shadow alpha buffer for subsampling the shadow calculation
	viewAlphaImage = globalImages->ImageFromFunction("_viewAlpha", R_CreateViewAlphaImage );

	//-----------------------------------

	// generate the jitter image
	jitterImage16 = globalImages->ImageFromFunction( "_jitter16", R_CreateJitterImage16 );
	jitterImage4 = globalImages->ImageFromFunction( "_jitter4", R_CreateJitterImage4 );
	jitterImage1 = globalImages->ImageFromFunction( "_jitter1", R_CreateJitterImage1 );

	depthMidpointVertexProgram = R_FindARBProgram( GL_VERTEX_PROGRAM_ARB, "depthMidpoint.vfp" );
	depthMidpointFragmentProgram = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "depthMidpoint.vfp" );

	shadowResampleVertexProgram = R_FindARBProgram( GL_VERTEX_PROGRAM_ARB, "shadowResample.vfp" );
	shadowResampleFragmentProgram = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "shadowResample.vfp" );

	screenSpaceShadowVertexProgram = R_FindARBProgram( GL_VERTEX_PROGRAM_ARB, "screenSpaceShadow1.vfp" );

	screenSpaceShadowFragmentProgram0 = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "screenSpaceShadow0.vfp" );
	screenSpaceShadowFragmentProgram1 = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "screenSpaceShadow1.vfp" );
	screenSpaceShadowFragmentProgram4 = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "screenSpaceShadow4.vfp" );
	screenSpaceShadowFragmentProgram16 = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "screenSpaceShadow16.vfp" );

	shadowVertexProgram = R_FindARBProgram( GL_VERTEX_PROGRAM_ARB, "shadowBufferInteraction1.vfp" );

	shadowFragmentProgram0 = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "shadowBufferInteraction0.vfp" );
	shadowFragmentProgram1 = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "shadowBufferInteraction1.vfp" );
	shadowFragmentProgram4 = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "shadowBufferInteraction4.vfp" );
	shadowFragmentProgram16 = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "shadowBufferInteraction16.vfp" );

	gammaDitherVertexProgram = R_FindARBProgram( GL_VERTEX_PROGRAM_ARB, "gammaDither.vfp" );
	gammaDitherFragmentProgram = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "gammaDither.vfp" );

	downSampleVertexProgram = R_FindARBProgram( GL_VERTEX_PROGRAM_ARB, "downSample.vfp" );
	downSampleFragmentProgram = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "downSample.vfp" );

	bloomVertexProgram = R_FindARBProgram( GL_VERTEX_PROGRAM_ARB, "bloom.vfp" );
	bloomFragmentProgram = R_FindARBProgram( GL_FRAGMENT_PROGRAM_ARB, "bloom.vfp" );

	random256Image = globalImages->ImageFromFunction( "_random256", R_CreateRandom256Image );
}

//===========================================================================================

static const int CULL_RECEIVER = 1;	// still draw occluder, but it is out of the view
static const int CULL_OCCLUDER_AND_RECEIVER = 2;	// the surface doesn't effect the view at all

/*
==================
RB_EXP_CullInteractions

Sets surfaceInteraction_t->cullBits
==================
*/
void RB_EXP_CullInteractions( viewLight_t *vLight, idPlane frustumPlanes[6] ) {
	for ( idInteraction *inter = vLight->lightDef->firstInteraction ; inter ; inter = inter->lightNext ) {
		const idRenderEntityLocal *entityDef = inter->entityDef;
		if ( !entityDef ) {
			continue;
		}
		if ( inter->numSurfaces < 1 ) {
			continue;
		}

		int	culled = 0;

		if ( r_sb_useCulling.GetBool() ) {
			// transform light frustum into object space, positive side points outside the light
			idPlane	localPlanes[6];
			int		plane;
			for ( plane = 0 ; plane < 6 ; plane++ ) {
				R_GlobalPlaneToLocal( entityDef->modelMatrix, frustumPlanes[plane], localPlanes[plane] );
			}

			// cull the entire entity bounding box
			// has referenceBounds been tightened to the actual model bounds?
			idVec3	corners[8];
			for ( int i = 0 ; i < 8 ; i++ ) {
				corners[i][0] = entityDef->referenceBounds[i&1][0];
				corners[i][1] = entityDef->referenceBounds[(i>>1)&1][1];
				corners[i][2] = entityDef->referenceBounds[(i>>2)&1][2];
			}

			for ( plane = 0 ; plane < 6 ; plane++ ) {
				int		j;
				for ( j = 0 ; j < 8 ; j++ ) {
					// if a corner is on the negative side (inside) of the frustum, the surface is not culled
					// by this plane
					if ( corners[j] * localPlanes[plane].ToVec4().ToVec3() + localPlanes[plane][3] < 0 ) {
						break;
					}
				}
				if ( j == 8 ) {
					break;			// all points outside the light
				}
			}
			if ( plane < 6 ) {
				culled = CULL_OCCLUDER_AND_RECEIVER;
			}
		}

		for ( int i = 0 ; i < inter->numSurfaces ; i++ ) {
			surfaceInteraction_t	*surfInt = &inter->surfaces[i];

			if ( !surfInt->ambientTris ) {
				continue;
			}
			surfInt->expCulled = culled;
		}

	}
}

/*
==================
RB_EXP_RenderOccluders
==================
*/
void RB_EXP_RenderOccluders( viewLight_t *vLight ) {
	for ( idInteraction *inter = vLight->lightDef->firstInteraction ; inter ; inter = inter->lightNext ) {
		const idRenderEntityLocal *entityDef = inter->entityDef;
		if ( !entityDef ) {
			continue;
		}
		if ( inter->numSurfaces < 1 ) {
			continue;
		}

		// no need to check for current on this, because each interaction is always
		// a different space
		float	matrix[16];
		myGlMultMatrix( inter->entityDef->modelMatrix, lightMatrix, matrix );
		qglLoadMatrixf( matrix );

		// draw each surface
		for ( int i = 0 ; i < inter->numSurfaces ; i++ ) {
			surfaceInteraction_t	*surfInt = &inter->surfaces[i];

			if ( !surfInt->ambientTris ) {
				continue;
			}
			if ( surfInt->shader && !surfInt->shader->SurfaceCastsShadow() ) {
				continue;
			}

			// cull it
			if ( surfInt->expCulled == CULL_OCCLUDER_AND_RECEIVER ) {
				continue;
			}

			// render it
			const srfTriangles_t *tri = surfInt->ambientTris;
			if ( !tri->ambientCache ) {
				R_CreateAmbientCache( const_cast<srfTriangles_t *>(tri), false ); 
			}
			idDrawVert *ac = (idDrawVert *)vertexCache.Position( tri->ambientCache );
			qglVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
	qglTexCoordPointer( 2, GL_FLOAT, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
	if ( surfInt->shader ) {
		surfInt->shader->GetEditorImage()->Bind();
	}
			RB_DrawElementsWithCounters( tri );
		}
	}
}

/*
==================
RB_RenderShadowBuffer
==================
*/
void    RB_RenderShadowBuffer( viewLight_t	*vLight, int side ) {
	float	xmin, xmax, ymin, ymax;
	float	width, height;
	float	zNear;

	float	fov = r_sb_frustomFOV.GetFloat();

	//
	// set up 90 degree projection matrix
	//
	zNear	= 4;

	ymax = zNear * tan( fov * idMath::PI / 360.0f );
	ymin = -ymax;

	xmax = zNear * tan( fov * idMath::PI / 360.0f );
	xmin = -xmax;

	width = xmax - xmin;
	height = ymax - ymin;

	lightProjectionMatrix[0] = 2 * zNear / width;
	lightProjectionMatrix[4] = 0;
	lightProjectionMatrix[8] = 0;
	lightProjectionMatrix[12] = 0;

	lightProjectionMatrix[1] = 0;
	lightProjectionMatrix[5] = 2 * zNear / height;
	lightProjectionMatrix[9] = 0;
	lightProjectionMatrix[13] = 0;

	// this is the far-plane-at-infinity formulation, and
	// crunches the Z range slightly so w=0 vertexes do not
	// rasterize right at the wraparound point
	lightProjectionMatrix[2] = 0;
	lightProjectionMatrix[6] = 0;
	lightProjectionMatrix[10] = -0.999f;
	lightProjectionMatrix[14] = -2.0f * zNear;

	lightProjectionMatrix[3] = 0;
	lightProjectionMatrix[7] = 0;
	lightProjectionMatrix[11] = -1;
	lightProjectionMatrix[15] = 0;


	if ( r_sb_usePbuffer.GetBool() ) {
		// set the current openGL drawable to the shadow buffer
		R_MakeCurrent( shadowPbufferDC, win32.hGLRC, NULL /* !@# shadowPbuffer */ );
	}

	qglMatrixMode( GL_PROJECTION );
	qglLoadMatrixf( lightProjectionMatrix );
	qglMatrixMode( GL_MODELVIEW );

	qglViewport( 0, 0, lightBufferSize, lightBufferSize );
	qglScissor( 0, 0, lightBufferSize, lightBufferSize );
	qglStencilFunc( GL_ALWAYS, 0, 255 );

qglClearColor( 0, 1, 0, 0 );
GL_State( GLS_DEPTHFUNC_LESS | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );	// make sure depth mask is off before clear
qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

// draw all the occluders
qglColor3f( 1, 1, 1 );
GL_SelectTexture( 0 );
qglEnableClientState( GL_TEXTURE_COORD_ARRAY );

	backEnd.currentSpace = NULL;

	static float	s_flipMatrix[16] = {
		// convert from our coordinate system (looking down X)
		// to OpenGL's coordinate system (looking down -Z)
		0, 0, -1, 0,
		-1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	};

	float	viewMatrix[16];

	idVec3	vec;
	idVec3	origin = vLight->lightDef->globalLightOrigin;

	if ( side == -1 ) {
		// projected light
		vec = vLight->lightDef->parms.target;
		vec.Normalize();
		viewMatrix[0] = vec[0];
		viewMatrix[4] = vec[1];
		viewMatrix[8] = vec[2];

		vec = vLight->lightDef->parms.right;
		vec.Normalize();
		viewMatrix[1] = -vec[0];
		viewMatrix[5] = -vec[1];
		viewMatrix[9] = -vec[2];

		vec = vLight->lightDef->parms.up;
		vec.Normalize();
		viewMatrix[2] = vec[0];
		viewMatrix[6] = vec[1];
		viewMatrix[10] = vec[2];
	} else {
		// side of a point light
		memset( viewMatrix, 0, sizeof( viewMatrix ) );
		switch ( side ) {
		case 0:
			viewMatrix[0] = 1;
			viewMatrix[9] = 1;
			viewMatrix[6] = -1;
			break;
		case 1:
			viewMatrix[0] = -1;
			viewMatrix[9] = -1;
			viewMatrix[6] = -1;
			break;
		case 2:
			viewMatrix[4] = 1;
			viewMatrix[1] = -1;
			viewMatrix[10] = 1;
			break;
		case 3:
			viewMatrix[4] = -1;
			viewMatrix[1] = -1;
			viewMatrix[10] = -1;
			break;
		case 4:
			viewMatrix[8] = 1;
			viewMatrix[1] = -1;
			viewMatrix[6] = -1;
			break;
		case 5:
			viewMatrix[8] = -1;
			viewMatrix[1] = 1;
			viewMatrix[6] = -1;
			break;
		}
	}

	viewMatrix[12] = -origin[0] * viewMatrix[0] + -origin[1] * viewMatrix[4] + -origin[2] * viewMatrix[8];
	viewMatrix[13] = -origin[0] * viewMatrix[1] + -origin[1] * viewMatrix[5] + -origin[2] * viewMatrix[9];
	viewMatrix[14] = -origin[0] * viewMatrix[2] + -origin[1] * viewMatrix[6] + -origin[2] * viewMatrix[10];

	viewMatrix[3] = 0;
	viewMatrix[7] = 0;
	viewMatrix[11] = 0;
	viewMatrix[15] = 1;

	memcpy( unflippedLightMatrix, viewMatrix, sizeof( unflippedLightMatrix ) );
	myGlMultMatrix( viewMatrix, s_flipMatrix,lightMatrix);

	// create frustum planes
	idPlane	globalFrustum[6];

	// near clip
	globalFrustum[0][0] = -viewMatrix[0];
	globalFrustum[0][1] = -viewMatrix[4];
	globalFrustum[0][2] = -viewMatrix[8];
	globalFrustum[0][3] = -(origin[0] * globalFrustum[0][0] + origin[1] * globalFrustum[0][1] + origin[2] * globalFrustum[0][2]);

	// far clip
	globalFrustum[1][0] = viewMatrix[0];
	globalFrustum[1][1] = viewMatrix[4];
	globalFrustum[1][2] = viewMatrix[8];
	globalFrustum[1][3] = -globalFrustum[0][3] - viewLightAxialSize;

	// side clips
	globalFrustum[2][0] = -viewMatrix[0] + viewMatrix[1];
	globalFrustum[2][1] = -viewMatrix[4] + viewMatrix[5];
	globalFrustum[2][2] = -viewMatrix[8] + viewMatrix[9];

	globalFrustum[3][0] = -viewMatrix[0] - viewMatrix[1];
	globalFrustum[3][1] = -viewMatrix[4] - viewMatrix[5];
	globalFrustum[3][2] = -viewMatrix[8] - viewMatrix[9];

	globalFrustum[4][0] = -viewMatrix[0] + viewMatrix[2];
	globalFrustum[4][1] = -viewMatrix[4] + viewMatrix[6];
	globalFrustum[4][2] = -viewMatrix[8] + viewMatrix[10];

	globalFrustum[5][0] = -viewMatrix[0] - viewMatrix[2];
	globalFrustum[5][1] = -viewMatrix[4] - viewMatrix[6];
	globalFrustum[5][2] = -viewMatrix[8] - viewMatrix[10];

	// is this nromalization necessary?
	for ( int i = 0 ; i < 6 ; i++ ) {
		globalFrustum[i].ToVec4().ToVec3().Normalize();
	}

	for ( int i = 2 ; i < 6 ; i++ ) {
		globalFrustum[i][3] = - (origin * globalFrustum[i].ToVec4().ToVec3() );
	}

	RB_EXP_CullInteractions( vLight, globalFrustum );


	// FIXME: we want to skip the sampling as well as the generation when not casting shadows
	if ( !r_sb_noShadows.GetBool() && vLight->lightShader->LightCastsShadows() ) {
		//
		// set polygon offset for the rendering
		//
		switch ( r_sb_occluderFacing.GetInteger() ) {
		case 0:		// front sides
			qglPolygonOffset( r_sb_polyOfsFactor.GetFloat(), r_sb_polyOfsUnits.GetFloat() );
			qglEnable( GL_POLYGON_OFFSET_FILL );
			RB_EXP_RenderOccluders( vLight );
			qglDisable( GL_POLYGON_OFFSET_FILL );
			break;
		case 1:		// back sides
			qglPolygonOffset( -r_sb_polyOfsFactor.GetFloat(), -r_sb_polyOfsUnits.GetFloat() );
			qglEnable( GL_POLYGON_OFFSET_FILL );
			GL_Cull( CT_BACK_SIDED );
			RB_EXP_RenderOccluders( vLight );
			GL_Cull( CT_FRONT_SIDED );
			qglDisable( GL_POLYGON_OFFSET_FILL );
			break;
		case 2:		// both sides
			GL_Cull( CT_BACK_SIDED );
			RB_EXP_RenderOccluders( vLight );
			GL_Cull( CT_FRONT_SIDED );
			shadowImage[2]->Bind();
			qglCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, lightBufferSize, lightBufferSize );

			RB_EXP_RenderOccluders( vLight );
			shadowImage[1]->Bind();
			qglCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, lightBufferSize, lightBufferSize );

			// fragment program to combine the two depth images
			qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, depthMidpointVertexProgram );
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, depthMidpointFragmentProgram );
			qglEnable(GL_VERTEX_PROGRAM_ARB);
			qglEnable(GL_FRAGMENT_PROGRAM_ARB);

			GL_SelectTextureNoClient( 1 );
			shadowImage[1]->Bind();
			qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

			GL_SelectTextureNoClient( 0 );
			shadowImage[2]->Bind();
			qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

			// draw a full screen quad
			qglMatrixMode( GL_PROJECTION );
			qglLoadIdentity(); 
			qglOrtho( 0, 1, 0, 1, -1, 1 );
			qglMatrixMode( GL_MODELVIEW );
			qglLoadIdentity();

			GL_State( GLS_DEPTHFUNC_ALWAYS );

			qglBegin( GL_TRIANGLE_FAN );
			qglTexCoord2f( 0, 0 );
			qglVertex2f( 0, 0 );
			qglTexCoord2f( 0, lightBufferSizeFraction );
			qglVertex2f( 0, 1 );
			qglTexCoord2f( lightBufferSizeFraction, lightBufferSizeFraction );
			qglVertex2f( 1, 1 );
			qglTexCoord2f( lightBufferSizeFraction, 0 );
			qglVertex2f( 1, 0 );
			qglEnd();

			qglDisable( GL_VERTEX_PROGRAM_ARB );
			qglDisable( GL_FRAGMENT_PROGRAM_ARB );

			break;
		}
	}

	// copy to the texture
	shadowImage[0]->Bind();
	qglCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, lightBufferSize, lightBufferSize );

qglDisableClientState( GL_TEXTURE_COORD_ARRAY );


	// reset the normal view matrix

	qglMatrixMode( GL_PROJECTION );
	qglLoadMatrixf( backEnd.viewDef->projectionMatrix );
	qglMatrixMode( GL_MODELVIEW );

	// the current modelView matrix is not valid
	backEnd.currentSpace = NULL;
}

/*
==================
RB_EXP_DrawInteraction
==================
*/
void	RB_EXP_DrawInteraction( const drawInteraction_t *din ) {
	// load all the vertex program parameters
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_ORIGIN, din->localLightOrigin.ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_VIEW_ORIGIN, din->localViewOrigin.ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_PROJECT_S, din->lightProjection[0].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_PROJECT_T, din->lightProjection[1].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_PROJECT_Q, din->lightProjection[2].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_LIGHT_FALLOFF_S, din->lightProjection[3].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_BUMP_MATRIX_S, din->bumpMatrix[0].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_BUMP_MATRIX_T, din->bumpMatrix[1].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_DIFFUSE_MATRIX_S, din->diffuseMatrix[0].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_DIFFUSE_MATRIX_T, din->diffuseMatrix[1].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_SPECULAR_MATRIX_S, din->specularMatrix[0].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_SPECULAR_MATRIX_T, din->specularMatrix[1].ToFloatPtr() );


// calculate depth projection for shadow buffer
float	sRow[4];
float	tRow[4];
float	rRow[4];
float	qRow[4];
float	matrix[16];
float	matrix2[16];
myGlMultMatrix( din->surf->space->modelMatrix, lightMatrix, matrix );
myGlMultMatrix( matrix, lightProjectionMatrix, matrix2 );

// the final values need to be in 0.0 : 1.0 range instead of -1 : 1
sRow[0] = 0.5 * lightBufferSizeFraction * ( matrix2[0] + matrix2[3] );
sRow[1] = 0.5 * lightBufferSizeFraction * ( matrix2[4] + matrix2[7] );
sRow[2] = 0.5 * lightBufferSizeFraction * ( matrix2[8] + matrix2[11] );
sRow[3] = 0.5 * lightBufferSizeFraction * ( matrix2[12] + matrix2[15] );
qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 18, sRow );
tRow[0] = 0.5 * lightBufferSizeFraction * ( matrix2[1] + matrix2[3] );
tRow[1] = 0.5 * lightBufferSizeFraction * ( matrix2[5] + matrix2[7] );
tRow[2] = 0.5 * lightBufferSizeFraction * ( matrix2[9] + matrix2[11] );
tRow[3] = 0.5 * lightBufferSizeFraction * ( matrix2[13] + matrix2[15] );
qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 19, tRow );
rRow[0] = 0.5 * ( matrix2[2] + matrix2[3] );
rRow[1] = 0.5 * ( matrix2[6] + matrix2[7] );
rRow[2] = 0.5 * ( matrix2[10] + matrix2[11] );
rRow[3] = 0.5 * ( matrix2[14] + matrix2[15] );
qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 20, rRow );
qRow[0] = matrix2[3];
qRow[1] = matrix2[7];
qRow[2] = matrix2[11];
qRow[3] = matrix2[15];
qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 21, qRow );


	// testing fragment based normal mapping
	if ( r_testARBProgram.GetBool() ) {
		qglProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 2, din->localLightOrigin.ToFloatPtr() );
		qglProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 3, din->localViewOrigin.ToFloatPtr() );
	}

	static const float zero[4] = { 0, 0, 0, 0 };
	static const float one[4] = { 1, 1, 1, 1 };
	static const float negOne[4] = { -1, -1, -1, -1 };

	switch ( din->vertexColor ) {
	case SVC_IGNORE:
		qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_MODULATE, zero );
		qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_ADD, one );
		break;
	case SVC_MODULATE:
		qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_MODULATE, one );
		qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_ADD, zero );
		break;
	case SVC_INVERSE_MODULATE:
		qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_MODULATE, negOne );
		qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_COLOR_ADD, one );
		break;
	}

	// set the constant colors
	qglProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, din->diffuseColor.ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 1, din->specularColor.ToFloatPtr() );

	//-----------------------------------------------------
	// screen power of two correction factor

	float	parm[4];
	parm[0] = 1.0 / ( JITTER_SIZE * r_sb_samples.GetInteger() ) ;
	parm[1] = 1.0 / JITTER_SIZE;
	parm[2] = 0;
	parm[3] = 1;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 2, parm );

	// jitter tex scale
	parm[0] = 
	parm[1] = r_sb_jitterScale.GetFloat() * lightBufferSizeFraction;
	parm[2] = -r_sb_biasScale.GetFloat();
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 3, parm );

	// jitter tex offset
	if ( r_sb_randomize.GetBool() ) {
		parm[0] = (rand()&255) / 255.0;
		parm[1] = (rand()&255) / 255.0;
	} else {
		parm[0] = parm[1] = 0;
	}
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 4, parm );
	//-----------------------------------------------------

	// set the textures

	// texture 1 will be the per-surface bump map
	GL_SelectTextureNoClient( 1 );
	din->bumpImage->Bind();

	// texture 2 will be the light falloff texture
	GL_SelectTextureNoClient( 2 );
	din->lightFalloffImage->Bind();

	// texture 3 will be the light projection texture
	GL_SelectTextureNoClient( 3 );
	din->lightImage->Bind();

	// texture 4 is the per-surface diffuse map
	GL_SelectTextureNoClient( 4 );
	din->diffuseImage->Bind();

	// texture 5 is the per-surface specular map
	GL_SelectTextureNoClient( 5 );
	din->specularImage->Bind();

	// draw it
	RB_DrawElementsWithCounters( din->surf->geo );
}

/*
=============
RB_EXP_CreateDrawInteractions

=============
*/
void RB_EXP_CreateDrawInteractions( const drawSurf_t *surf ) {
	if ( !surf ) {
		return;
	}
	if ( r_sb_screenSpaceShadow.GetBool() ) {
		// perform setup here that will be constant for all interactions
		GL_State( GLS_SRCBLEND_DST_ALPHA | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc );

		if ( r_testARBProgram.GetBool() ) {
			qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_TEST );
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_TEST );
		} else {
			qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_INTERACTION );
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_INTERACTION );
		}
	} else {
		// perform setup here that will be constant for all interactions
		GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc );
GL_State( GLS_DEPTHMASK | GLS_DEPTHFUNC_ALWAYS );//!@#

		// bind the vertex program
		qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, shadowVertexProgram );
		if ( r_sb_samples.GetInteger() == 16 ) {
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowFragmentProgram16 );
		} else if ( r_sb_samples.GetInteger() == 4 ) {
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowFragmentProgram4 );
		} else if ( r_sb_samples.GetInteger() == 1 ) {
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowFragmentProgram1 );
		} else {
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowFragmentProgram0 );
		}
	}

	qglEnable(GL_VERTEX_PROGRAM_ARB);
	qglEnable(GL_FRAGMENT_PROGRAM_ARB);

	// enable the vertex arrays
	qglEnableVertexAttribArrayARB( 8 );
	qglEnableVertexAttribArrayARB( 9 );
	qglEnableVertexAttribArrayARB( 10 );
	qglEnableVertexAttribArrayARB( 11 );
	qglEnableClientState( GL_COLOR_ARRAY );

	// texture 0 is the normalization cube map for the vector towards the light
	GL_SelectTextureNoClient( 0 );
	if ( backEnd.vLight->lightShader->IsAmbientLight() ) {
		globalImages->normalCubeMapImage->Bind();
		qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_AMBIENT);
		qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, FPROG_AMBIENT);
	} else {
		globalImages->normalCubeMapImage->Bind();
	}

	// texture 6 is the specular lookup table
	GL_SelectTextureNoClient( 6 );
	if ( r_testARBProgram.GetBool() ) {
		globalImages->specular2DTableImage->Bind();	// variable specularity in alpha channel
	} else {
		globalImages->specularTableImage->Bind();
	}


	for ( ; surf ; surf=surf->nextOnLight ) {
		// perform setup here that will not change over multiple interaction passes
		if ( backEnd.vLight->lightShader->IsAmbientLight() ) {
			float	parm[4];

			parm[0] = surf->space->modelMatrix[0];
			parm[1] = surf->space->modelMatrix[4];
			parm[2] = surf->space->modelMatrix[8];
			parm[3] = 0;
			qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 20, parm );
			parm[0] = surf->space->modelMatrix[1];
			parm[1] = surf->space->modelMatrix[5];
			parm[2] = surf->space->modelMatrix[9];
			parm[3] = 0;
			qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 21, parm );
			parm[0] = surf->space->modelMatrix[2];
			parm[1] = surf->space->modelMatrix[6];
			parm[2] = surf->space->modelMatrix[10];
			parm[3] = 0;
			qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, 22, parm );

			GL_SelectTextureNoClient( 0 );
			const shaderStage_t *stage = backEnd.vLight->lightShader->GetStage( 0 );
			if ( stage->newStage ) {
				stage->newStage->fragmentProgramImages[7]->BindFragment();
			}
		}

		// set the vertex pointers
		idDrawVert	*ac = (idDrawVert *)vertexCache.Position( surf->geo->ambientCache );
		qglColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ac->color );
		qglVertexAttribPointerARB( 11, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
		qglVertexAttribPointerARB( 10, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
		qglVertexAttribPointerARB( 9, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
		qglVertexAttribPointerARB( 8, 2, GL_FLOAT, false, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
		qglVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );

		// this may cause RB_ARB2_DrawInteraction to be exacuted multiple
		// times with different colors and images if the surface or light have multiple layers
		if ( r_sb_screenSpaceShadow.GetBool() ) {
			RB_CreateSingleDrawInteractions( surf, RB_ARB2_DrawInteraction );
		} else {
			RB_CreateSingleDrawInteractions( surf, RB_EXP_DrawInteraction );
		}
	}

	qglDisableVertexAttribArrayARB( 8 );
	qglDisableVertexAttribArrayARB( 9 );
	qglDisableVertexAttribArrayARB( 10 );
	qglDisableVertexAttribArrayARB( 11 );
	qglDisableClientState( GL_COLOR_ARRAY );

	// disable features
	GL_SelectTextureNoClient( 6 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 5 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 4 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 3 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 2 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 1 );
	globalImages->BindNull();

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture( 0 );

	qglDisable(GL_VERTEX_PROGRAM_ARB);
	qglDisable(GL_FRAGMENT_PROGRAM_ARB);
}

void InvertByTranspose( const float a[16], float r[16] ) {
    r[ 0] = a[ 0];
    r[ 1] = a[ 4];
    r[ 2] = a[ 8];
	r[ 3] = 0;
    r[ 4] = a[ 1];
    r[ 5] = a[ 5];
    r[ 6] = a[ 9];
	r[ 7] = 0;
    r[ 8] = a[ 2];
    r[ 9] = a[ 6];
    r[10] = a[10];
	r[11] = 0;
    r[12] = -(r[ 0]*a[12] + r[ 4]*a[13] + r[ 8]*a[14]);
    r[13] = -(r[ 1]*a[12] + r[ 5]*a[13] + r[ 9]*a[14]);
    r[14] = -(r[ 2]*a[12] + r[ 6]*a[13] + r[10]*a[14]);
	r[15] = 1;
}

void FullInvert( const float a[16], float r[16] ) {
	idMat4	am;

	for ( int i = 0 ; i < 4 ; i++ ) {
		for ( int j = 0 ; j < 4 ; j++ ) {
			am[i][j] = a[j*4+i];
		}
	}

//	idVec4 test( 100, 100, 100, 1 );
//	idVec4	transformed, inverted;
//	transformed = test * am;

	if ( !am.InverseSelf() ) {
		common->Error( "Invert failed" );
	}
//	inverted = transformed * am;

	for ( int i = 0 ; i < 4 ; i++ ) {
		for ( int j = 0 ; j < 4 ; j++ ) {
			r[j*4+i] = am[i][j];
		}
	}
}

/*
==================
RB_Exp_TrianglesForFrustum
==================
*/
const srfTriangles_t	*RB_Exp_TrianglesForFrustum( viewLight_t *vLight, int side ) {
	const srfTriangles_t *tri;

	static srfTriangles_t	frustumTri;
	static idDrawVert		verts[5];
	static glIndex_t		indexes[18] = { 0, 1, 2,	0, 2, 3,	0, 3, 4,	0, 4, 1,	2, 1, 4,	2, 4, 3 };

	if ( side == -1 ) {
		tri = vLight->frustumTris;
	} else {
		memset( verts, 0, sizeof( verts ) );

		for ( int i = 0 ; i < 5 ; i++ ) {
			verts[i].xyz = vLight->globalLightOrigin;
		}

		memset( &frustumTri, 0, sizeof( frustumTri ) );
		frustumTri.indexes = indexes;
		frustumTri.verts = verts;
		frustumTri.numIndexes = 18;
		frustumTri.numVerts = 5;

		tri = &frustumTri;

		float	size = viewLightAxialSize;

		switch ( side ) {
		case 0:
			verts[1].xyz[0] += size;
			verts[2].xyz[0] += size;
			verts[3].xyz[0] += size;
			verts[4].xyz[0] += size;
			verts[1].xyz[1] += size;
			verts[1].xyz[2] += size;
			verts[2].xyz[1] -= size;
			verts[2].xyz[2] += size;
			verts[3].xyz[1] -= size;
			verts[3].xyz[2] -= size;
			verts[4].xyz[1] += size;
			verts[4].xyz[2] -= size;
			break;
		case 1:
			verts[1].xyz[0] -= size;
			verts[2].xyz[0] -= size;
			verts[3].xyz[0] -= size;
			verts[4].xyz[0] -= size;
			verts[1].xyz[1] -= size;
			verts[1].xyz[2] += size;
			verts[2].xyz[1] += size;
			verts[2].xyz[2] += size;
			verts[3].xyz[1] += size;
			verts[3].xyz[2] -= size;
			verts[4].xyz[1] -= size;
			verts[4].xyz[2] -= size;
			break;
		case 2:
			verts[1].xyz[1] += size;
			verts[2].xyz[1] += size;
			verts[3].xyz[1] += size;
			verts[4].xyz[1] += size;
			verts[1].xyz[0] -= size;
			verts[1].xyz[2] += size;
			verts[2].xyz[0] += size;
			verts[2].xyz[2] += size;
			verts[3].xyz[0] += size;
			verts[3].xyz[2] -= size;
			verts[4].xyz[0] -= size;
			verts[4].xyz[2] -= size;
			break;
		case 3:
			verts[1].xyz[1] -= size;
			verts[2].xyz[1] -= size;
			verts[3].xyz[1] -= size;
			verts[4].xyz[1] -= size;
			verts[1].xyz[0] += size;
			verts[1].xyz[2] += size;
			verts[2].xyz[0] -= size;
			verts[2].xyz[2] += size;
			verts[3].xyz[0] -= size;
			verts[3].xyz[2] -= size;
			verts[4].xyz[0] += size;
			verts[4].xyz[2] -= size;
			break;
		case 4:
			verts[1].xyz[2] += size;
			verts[2].xyz[2] += size;
			verts[3].xyz[2] += size;
			verts[4].xyz[2] += size;
			verts[1].xyz[0] += size;
			verts[1].xyz[1] += size;
			verts[2].xyz[0] -= size;
			verts[2].xyz[1] += size;
			verts[3].xyz[0] -= size;
			verts[3].xyz[1] -= size;
			verts[4].xyz[0] += size;
			verts[4].xyz[1] -= size;
			break;
		case 5:
			verts[1].xyz[2] -= size;
			verts[2].xyz[2] -= size;
			verts[3].xyz[2] -= size;
			verts[4].xyz[2] -= size;
			verts[1].xyz[0] -= size;
			verts[1].xyz[1] += size;
			verts[2].xyz[0] += size;
			verts[2].xyz[1] += size;
			verts[3].xyz[0] += size;
			verts[3].xyz[1] -= size;
			verts[4].xyz[0] -= size;
			verts[4].xyz[1] -= size;
			break;
		}

		frustumTri.ambientCache = vertexCache.AllocFrameTemp( verts, sizeof( verts ) );
	}

	return tri;
}


/*
==================
RB_Exp_SelectFrustum
==================
*/
void	RB_Exp_SelectFrustum( viewLight_t *vLight, int side ) {
	qglLoadMatrixf( backEnd.viewDef->worldSpace.modelViewMatrix );

	const srfTriangles_t *tri = RB_Exp_TrianglesForFrustum( vLight, side );

	idDrawVert *ac = (idDrawVert *)vertexCache.Position( tri->ambientCache );
	qglVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );

	qglDisable( GL_TEXTURE_2D );
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	// clear stencil buffer
	qglEnable( GL_SCISSOR_TEST );
	qglEnable( GL_STENCIL_TEST );
	qglClearStencil( 1 );
	qglClear( GL_STENCIL_BUFFER_BIT );

	// draw front faces of the light frustum, incrementing the stencil buffer on depth fail
	// so we can't draw on those pixels
	GL_State( GLS_COLORMASK | GLS_ALPHAMASK | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
	qglStencilFunc( GL_ALWAYS, 0, 255 );
	qglStencilOp( GL_KEEP, GL_INCR, GL_KEEP );
	GL_Cull( CT_FRONT_SIDED );

	RB_DrawElementsWithCounters( tri );
	
	// draw back faces of the light frustum with 
	// depth test greater
	// stencil test of equal 1
	// zero stencil stencil when depth test passes, so subsequent surface drawing
	// can occur on those pixels

	// this pass does all the shadow filtering
	qglStencilFunc( GL_EQUAL, 1, 255 );
	qglStencilOp( GL_KEEP, GL_KEEP, GL_ZERO );

	GL_Cull( CT_BACK_SIDED );
	qglDepthFunc( GL_GREATER );

	// write to destination alpha
	if ( r_sb_showFrustumPixels.GetBool() ) {
		GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
		qglDisable( GL_TEXTURE_2D );
		qglColor4f( 0, 0.25, 0, 1 );
	} else {
		GL_State( GLS_COLORMASK | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
		qglEnable(GL_VERTEX_PROGRAM_ARB);
		qglEnable(GL_FRAGMENT_PROGRAM_ARB);
		qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, screenSpaceShadowVertexProgram );
		switch ( r_sb_samples.GetInteger() ) {
		case 0:
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, screenSpaceShadowFragmentProgram0 );
			break;
		case 1:
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, screenSpaceShadowFragmentProgram1 );
			break;
		case 4:
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, screenSpaceShadowFragmentProgram4 );
			break;
		case 16:
			qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, screenSpaceShadowFragmentProgram16 );
			break;
		}
	}

/*
texture[0] = view depth texture
texture[1] = jitter texture
texture[2] = light depth texture
*/
	GL_SelectTextureNoClient( 2 );
	shadowImage[0]->Bind();

	GL_SelectTextureNoClient( 1 );
	if ( r_sb_samples.GetInteger() == 16 ) {
		jitterImage16->Bind();
	} else if ( r_sb_samples.GetInteger() == 4 ) {
		jitterImage4->Bind();
	} else {
		jitterImage1->Bind();
	}

	GL_SelectTextureNoClient( 0 );
	viewDepthImage->Bind();

	/*
PARAM	positionToDepthTexScale		= program.local[0];	# fragment.position to screen depth texture transformation
PARAM	zProject					= program.local[1];	# projection[10], projection[14], 0, 0
PARAM	positionToViewSpace			= program.local[2];	# X add, Y add, X mul, Y mul
PARAM	viewToLightS				= program.local[3];
PARAM	viewToLightT				= program.local[4];
PARAM	viewToLightR				= program.local[5];
PARAM	viewToLightQ				= program.local[6];
PARAM	positionToJitterTexScale	= program.local[7];	# fragment.position to jitter texture
PARAM	jitterTexScale				= program.local[8];
PARAM	jitterTexOffset				= program.local[9];
*/
	float	parm[4];
	int		pot;

	// calculate depth projection for shadow buffer
	float	sRow[4];
	float	tRow[4];
	float	rRow[4];
	float	qRow[4];
	float	invertedView[16];
	float	invertedProjection[16];
	float	matrix[16];
	float	matrix2[16];

	// we need the inverse of the projection matrix to go from NDC to view
	FullInvert( backEnd.viewDef->projectionMatrix, invertedProjection );

	/*
	from window to NDC:
		( x - xMid ) * 1.0 / xMid
		( y - yMid ) * 1.0 / yMid
		( z - 0.5 ) * 2

	from NDC to clip coordinates:
		rcp(1/w)

	*/

	// we need the inverse of the viewMatrix to go from view (looking down negative Z) to world
	InvertByTranspose( backEnd.viewDef->worldSpace.modelViewMatrix, invertedView );

	// then we go from world to light view space (looking down negative Z)
	myGlMultMatrix( invertedView, lightMatrix, matrix );

	// then to light projection, giving X/w, Y/w, Z/w in the -1 : 1 range
	myGlMultMatrix( matrix, lightProjectionMatrix, matrix2 );

	// the final values need to be in 0.0 : 1.0 range instead of -1 : 1
	sRow[0] = 0.5 * ( matrix2[0] + matrix2[3] ) * lightBufferSizeFraction;
	sRow[1] = 0.5 * ( matrix2[4] + matrix2[7] ) * lightBufferSizeFraction;
	sRow[2] = 0.5 * ( matrix2[8] + matrix2[11] ) * lightBufferSizeFraction;
	sRow[3] = 0.5 * ( matrix2[12] + matrix2[15] ) * lightBufferSizeFraction;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 3, sRow );
	tRow[0] = 0.5 * ( matrix2[1] + matrix2[3] ) * lightBufferSizeFraction;
	tRow[1] = 0.5 * ( matrix2[5] + matrix2[7] ) * lightBufferSizeFraction;
	tRow[2] = 0.5 * ( matrix2[9] + matrix2[11] ) * lightBufferSizeFraction;
	tRow[3] = 0.5 * ( matrix2[13] + matrix2[15] ) * lightBufferSizeFraction;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 4, tRow );
	rRow[0] = 0.5 * ( matrix2[2] + matrix2[3] );
	rRow[1] = 0.5 * ( matrix2[6] + matrix2[7] );
	rRow[2] = 0.5 * ( matrix2[10] + matrix2[11] );
	rRow[3] = 0.5 * ( matrix2[14] + matrix2[15] );
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 5, rRow );
	qRow[0] = matrix2[3];
	qRow[1] = matrix2[7];
	qRow[2] = matrix2[11];
	qRow[3] = matrix2[15];
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 6, qRow );

	//-----------------------------------------------------
	// these should be constant for the entire frame

	// convert 0..viewport-1 sizes to fractions inside the POT screen depth texture
	int	 w = viewBufferSize;
	pot = MakePowerOfTwo( w );
	parm[0] = 1.0 / maxViewBufferSize;	//  * ( (float)viewBufferSize / w );
	int	 h = viewBufferHeight;
	pot = MakePowerOfTwo( h );
	parm[1] = parm[0]; // 1.0 / pot;
	parm[2] = 0;
	parm[3] = 1;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, parm );

	// zProject values
	parm[0] = backEnd.viewDef->projectionMatrix[10];
	parm[1] = backEnd.viewDef->projectionMatrix[14];
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 1, parm );

	// positionToViewSpace
	parm[0] = -1.0 / backEnd.viewDef->projectionMatrix[0];
	parm[1] = -1.0 / backEnd.viewDef->projectionMatrix[5];
	parm[2] = 2.0/viewBufferSize;
	parm[3] = 2.0/viewBufferSize;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 2, parm );

	// positionToJitterTexScale
	parm[0] = 1.0 / ( JITTER_SIZE * r_sb_samples.GetInteger() ) ;
	parm[1] = 1.0 / JITTER_SIZE;
	parm[2] = 0;
	parm[3] = 1;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 7, parm );

	// jitter tex scale
	parm[0] = 
	parm[1] = r_sb_jitterScale.GetFloat() * lightBufferSizeFraction;
	parm[2] = -r_sb_biasScale.GetFloat();
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 8, parm );

	// jitter tex offset
	if ( r_sb_randomize.GetBool() ) {
		parm[0] = (rand()&255) / 255.0;
		parm[1] = (rand()&255) / 255.0;
	} else {
		parm[0] = parm[1] = 0;
	}
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 9, parm );
	//-----------------------------------------------------



	RB_DrawElementsWithCounters( tri );

	qglDisable(GL_VERTEX_PROGRAM_ARB);
	qglDisable(GL_FRAGMENT_PROGRAM_ARB);

	GL_Cull( CT_FRONT_SIDED );
//	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );

	qglDepthFunc( GL_LEQUAL );
	if ( r_sb_showFrustumPixels.GetBool() ) {
		qglEnable( GL_TEXTURE_2D );
		qglColor3f( 1, 1, 1 );
	}

	// after all the frustums have been drawn, the surfaces that have been drawn on will get interactions
	// scissor may still be a win even with the stencil test for very fast rejects
	qglStencilFunc( GL_EQUAL, 0, 255 );
	qglStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

	// we can avoid clearing the stencil buffer by changing the hasLight value for each light
}

/*
==================
R_EXP_CalcLightAxialSize

all light side projections must currently match, so non-centered
and non-cubic lights must take the largest length
==================
*/
float	R_EXP_CalcLightAxialSize( viewLight_t *vLight ) {
	float	max = 0;

	if ( !vLight->lightDef->parms.pointLight ) {
		idVec3	dir = vLight->lightDef->parms.target - vLight->lightDef->parms.origin;
		max = dir.Length();
		return max;
	}

	for ( int i = 0 ; i < 3 ; i++ ) {
		float	dist = fabs(vLight->lightDef->parms.lightCenter[i] );
		dist += vLight->lightDef->parms.lightRadius[i];
		if ( dist > max ) {
			max = dist;
		}
	}
	return max;
}

/*
==================
R_EXP_RenderViewDepthImage

This could be avoided by drop sampling the native view depth buffer with render to texture
Bilerp might even be aprorpiate, although it would cause issues at edges
==================
*/
void RB_T_FillDepthBuffer( const drawSurf_t *surf );

void R_EXP_RenderViewDepthImage( void ) {
	if ( !r_sb_screenSpaceShadow.GetBool() ) {
		return;
	}

	// if the screen resolution is exactly the window width, we can
	// use the depth buffer we already have
	if ( 0 ) { // nativeViewBuffer ) {
		viewDepthImage->CopyDepthbuffer( backEnd.viewDef->viewport.x1,
			backEnd.viewDef->viewport.y1,  backEnd.viewDef->viewport.x2 -  backEnd.viewDef->viewport.x1 + 1,
			backEnd.viewDef->viewport.y2 -  backEnd.viewDef->viewport.y1 + 1 );
	} else {
		RB_LogComment( "---------- R_EXP_RenderViewDepthImage ----------\n" );

		if ( r_sb_usePbuffer.GetBool() ) {
			GL_CheckErrors();
			// set the current openGL drawable to the shadow buffer
			R_MakeCurrent( viewPbufferDC, win32.hGLRC, NULL /* !@# viewPbuffer */ );
		}

		// render the depth to the new size
		qglViewport( 0, 0, viewBufferSize, viewBufferHeight );
		qglScissor( 0, 0, viewBufferSize, viewBufferHeight );
		qglClear( GL_DEPTH_BUFFER_BIT );
		qglStencilFunc( GL_ALWAYS, 0, 255 );

		// the first texture will be used for alpha tested surfaces
		GL_SelectTexture( 0 );
		qglEnableClientState( GL_TEXTURE_COORD_ARRAY );

		GL_State( GLS_DEPTHFUNC_LESS );

		RB_RenderDrawSurfListWithFunction( backEnd.viewDef->drawSurfs, backEnd.viewDef->numDrawSurfs, RB_T_FillDepthBuffer );

		//
		// copy it to a texture
		//
		viewDepthImage->Bind();
		qglCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, viewBufferSize, viewBufferHeight );

		if ( r_sb_usePbuffer.GetBool() ) {
			// set the normal screen drawable current
			R_MakeCurrent( win32.hDC, win32.hGLRC, NULL );
		}

		// reset the window clipping
		qglMatrixMode( GL_PROJECTION );
		qglLoadMatrixf( backEnd.viewDef->projectionMatrix );
		qglMatrixMode( GL_MODELVIEW );

		qglViewport( tr.viewportOffset[0] + backEnd.viewDef->viewport.x1, 
			tr.viewportOffset[1] + backEnd.viewDef->viewport.y1, 
			backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
			backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1 );
		qglScissor( tr.viewportOffset[0] + backEnd.viewDef->viewport.x1, 
			tr.viewportOffset[1] + backEnd.viewDef->viewport.y1, 
			backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
			backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1 );

		// the current modelView matrix is not valid
		backEnd.currentSpace = NULL;
	}

	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}

/*
==================
RB_EXP_SetNativeBuffer

This is always the back buffer, and scissor is set full screen
==================
*/
void RB_EXP_SetNativeBuffer( void ) {
	// set the normal screen drawable current
	R_MakeCurrent( win32.hDC, win32.hGLRC, NULL );

	qglViewport( tr.viewportOffset[0] + backEnd.viewDef->viewport.x1, 
		tr.viewportOffset[1] + backEnd.viewDef->viewport.y1, 
		backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
		backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1 );

	backEnd.currentScissor = backEnd.viewDef->viewport;
	if ( r_useScissor.GetBool() ) {
		qglScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1, 
			backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
			backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
			backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
	}
}

/*
==================
RB_EXP_SetRenderBuffer

This may be to a float pBuffer, and scissor is set to cover only the light
==================
*/
void RB_EXP_SetRenderBuffer( viewLight_t *vLight ) {
	if ( r_hdr_useFloats.GetBool() ) {
		R_MakeCurrent( floatPbufferDC, floatContext, floatPbuffer );
	} else {
		if ( !qwglMakeCurrent( win32.hDC, win32.hGLRC ) ) {
			GL_CheckErrors();
			common->FatalError( "Couldn't return to normal drawing context" );
		}
	}

	qglViewport( tr.viewportOffset[0] + backEnd.viewDef->viewport.x1, 
		tr.viewportOffset[1] + backEnd.viewDef->viewport.y1, 
		backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
		backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1 );

	if ( !vLight ) {
		backEnd.currentScissor = backEnd.viewDef->viewport;
	} else {
		backEnd.currentScissor = vLight->scissorRect;
	}
	if ( r_useScissor.GetBool() ) {
		qglScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1, 
			backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
			backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
			backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
	}
}

/*
==================
RB_shadowResampleAlpha

==================
*/
void	RB_shadowResampleAlpha( void ) {
	viewAlphaImage->Bind();
	// we could make this a subimage, but it isn't relevent once we have render-to-texture
	qglCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, viewBufferSize, viewBufferHeight );

	RB_EXP_SetRenderBuffer( backEnd.vLight );

//=====================

	qglLoadMatrixf( backEnd.viewDef->worldSpace.modelViewMatrix );

	// this uses the full light, not side frustums
	const srfTriangles_t *tri = backEnd.vLight->frustumTris;

	idDrawVert *ac = (idDrawVert *)vertexCache.Position( tri->ambientCache );
	qglVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );

	// clear stencil buffer
	qglEnable( GL_SCISSOR_TEST );
	qglEnable( GL_STENCIL_TEST );
	qglClearStencil( 1 );
	qglClear( GL_STENCIL_BUFFER_BIT );

	// draw front faces of the light frustum, incrementing the stencil buffer on depth fail
	// so we can't draw on those pixels
	GL_State( GLS_COLORMASK | GLS_ALPHAMASK | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
	qglStencilFunc( GL_ALWAYS, 0, 255 );
	qglStencilOp( GL_KEEP, GL_INCR, GL_KEEP );
	GL_Cull( CT_FRONT_SIDED );

	// set fragment / vertex program?

	RB_DrawElementsWithCounters( tri );
	
	// draw back faces of the light frustum with 
	// depth test greater
	// stencil test of equal 1
	// zero stencil stencil when depth test passes, so subsequent interaction drawing
	// can occur on those pixels

	// this pass does all the shadow filtering
	qglStencilFunc( GL_EQUAL, 1, 255 );
	qglStencilOp( GL_KEEP, GL_KEEP, GL_ZERO );

	// write to destination alpha
	if ( r_sb_showFrustumPixels.GetBool() ) {
		GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
		qglDisable( GL_TEXTURE_2D );
		qglColor4f( 0, 0.25, 0, 1 );
	} else {
		GL_State( GLS_COLORMASK | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
		qglEnable(GL_VERTEX_PROGRAM_ARB);
		qglEnable(GL_FRAGMENT_PROGRAM_ARB);
		qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, shadowResampleVertexProgram );
		qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, shadowResampleFragmentProgram );

		// convert 0..viewport-1 sizes to fractions inside the POT screen depth texture
		// shrink by one unit for bilerp
		float	parm[4];
		parm[0] = 1.0 / (maxViewBufferSize+1) * viewBufferSize / maxViewBufferSize;
		parm[1] = parm[0];
		parm[2] = 0;
		parm[3] = 1;
		qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, parm );
	}

	GL_Cull( CT_BACK_SIDED );
	qglDepthFunc( GL_GREATER );

	RB_DrawElementsWithCounters( tri );

	qglDisable(GL_VERTEX_PROGRAM_ARB);
	qglDisable(GL_FRAGMENT_PROGRAM_ARB);

	GL_Cull( CT_FRONT_SIDED );

	qglDepthFunc( GL_LEQUAL );
	if ( r_sb_showFrustumPixels.GetBool() ) {
		qglEnable( GL_TEXTURE_2D );
		qglColor3f( 1, 1, 1 );
	}

	// after all the frustums have been drawn, the surfaces that have been drawn on will get interactions
	// scissor may still be a win even with the stencil test for very fast rejects
	qglStencilFunc( GL_EQUAL, 0, 255 );
	qglStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
}


/*
==================
RB_EXP_CoverScreen
==================
*/
void RB_EXP_CoverScreen( void ) {
	// draw a full screen quad
	qglMatrixMode( GL_PROJECTION );
	qglLoadIdentity(); 
	qglOrtho( 0, 1, 0, 1, -1, 1 );
	qglMatrixMode( GL_MODELVIEW );
	qglLoadIdentity();

	qglBegin( GL_TRIANGLE_FAN );
	qglVertex2f( 0, 0 );
	qglVertex2f( 0, 1 );
	qglVertex2f( 1, 1 );
	qglVertex2f( 1, 0 );
	qglEnd();
}

/*
==================
RB_EXP_ReadFloatBuffer
==================
*/
void RB_EXP_ReadFloatBuffer( void ) {
	int		pixels = glConfig.vidWidth * glConfig.vidHeight;
	float	*buf = (float *)R_StaticAlloc( pixels * 4 * sizeof( float ) );

	qglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_RGBA, GL_FLOAT, buf );

	float	mins[4] = { 9999, 9999, 9999, 9999 };
	float	maxs[4] = { -9999, -9999, -9999, -9999 };
	for ( int i = 0 ; i < pixels ; i++ ) {
		for ( int j = 0 ; j < 4 ; j++ ) {
			float	v = buf[ i*4 + j ];
			if ( v < mins[j] ) {
				mins[j] = v;
			}
			if ( v > maxs[j] ) {
				maxs[j] = v;
			}
		}
	}

	RB_EXP_SetNativeBuffer();

	qglLoadIdentity();
	qglMatrixMode( GL_PROJECTION );
	GL_State( GLS_DEPTHFUNC_ALWAYS );
	qglColor3f( 1, 1, 1 );
	qglPushMatrix();
	qglLoadIdentity(); 
	qglDisable( GL_TEXTURE_2D );
    qglOrtho( 0, 1, 0, 1, -1, 1 );
	qglRasterPos2f( 0.01f, 0.01f );
	qglDrawPixels( glConfig.vidWidth, glConfig.vidHeight, GL_RGBA, GL_FLOAT, buf );
	qglPopMatrix();
	qglEnable( GL_TEXTURE_2D );
	qglMatrixMode( GL_MODELVIEW );

	R_StaticFree( buf );
}


void RB_TestGamma( void );

/*
==================
RB_EXP_GammaDither
==================
*/
void	RB_EXP_GammaDither( void ) {
	if ( !r_hdr_useFloats.GetBool() ) {
		return;
	}

#if 0
r_testGamma.SetBool( true );
RB_TestGamma();
r_testGamma.SetBool( false );
#endif

	RB_EXP_SetNativeBuffer();

	/*
# texture 0 is the high dynamic range buffer
# texture 1 is the random dither texture
# texture 2 is the light bloom texture

# writes result.color as the 32 bit dithered and gamma corrected values

PARAM	exposure =			program.local[0];		# multiply HDR value by this to get screen pixels
PARAM	gammaPower =		program.local[1];
PARAM	monitorDither =		program.local[2];
PARAM	positionToDitherScale =		program.local[3];
PARAM	bloomFraction =		program.local[4];
PARAM	positionToBloomScale =		program.local[5];

	*/

	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB,gammaDitherVertexProgram );
	qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, gammaDitherFragmentProgram );
	qglEnable(GL_VERTEX_PROGRAM_ARB);
	qglEnable(GL_FRAGMENT_PROGRAM_ARB);

	qglActiveTextureARB( GL_TEXTURE2_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbufferQuarterImage->texnum );
	R_BindTexImage( floatPbufferQuarter );

	qglActiveTextureARB( GL_TEXTURE1_ARB );
	random256Image->BindFragment();

	qglActiveTextureARB( GL_TEXTURE0_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbufferImage->texnum );
	R_BindTexImage( floatPbuffer );

	float	parm[4];

	parm[0] = r_hdr_exposure.GetFloat();
	parm[1] = 0;
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, parm );

	parm[0] = r_hdr_gamma.GetFloat();
	parm[1] = 0;
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 1, parm );

	parm[0] = r_hdr_monitorDither.GetFloat();
	parm[1] = 0;
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 2, parm );

	parm[0] = 1.0 / 256;
	parm[1] = parm[0];
	parm[2] = rand()/65535.0;
	parm[3] = rand()/65535.0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 3, parm );

	parm[0] = 1.0 - r_hdr_bloomFraction.GetFloat();
	parm[1] = r_hdr_bloomFraction.GetFloat();
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 4, parm );

	parm[0] = 0.25;
	parm[1] = 0.25;
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 5, parm );

	qglDisable( GL_STENCIL_TEST );
	qglDisable( GL_SCISSOR_TEST );
	qglDisable( GL_DEPTH_TEST );

	RB_EXP_CoverScreen();

	qglEnable( GL_DEPTH_TEST );

	qglDisable(GL_VERTEX_PROGRAM_ARB);
	qglDisable(GL_FRAGMENT_PROGRAM_ARB);
}

/*
==================
RB_EXP_Bloom
==================
*/
void	RB_EXP_Bloom( void ) {
	if ( !r_hdr_useFloats.GetBool() ) {
		return;
	}

	if ( r_hdr_bloomFraction.GetFloat() == 0 ) {
		return;
	}

	GL_CheckErrors();

	//
	// mip map
	//

	// draw to the second floatPbuffer
	R_MakeCurrent( floatPbuffer2DC, floatContext, floatPbuffer2 );

	GL_State( 0 );
	qglDisable( GL_DEPTH_TEST );
	qglDisable( GL_SCISSOR_TEST );

	qglEnable(GL_VERTEX_PROGRAM_ARB);
	qglEnable(GL_FRAGMENT_PROGRAM_ARB);

	qglClearColor( 1.0, 0.5, 0, 0 );
	qglClear( GL_COLOR_BUFFER_BIT );
	qglViewport( 0, 0, glConfig.vidWidth>>1, glConfig.vidHeight>>1 );

	// read from the original floatPbuffer
	qglActiveTextureARB( GL_TEXTURE0_ARB );
	qglBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbufferImage->texnum );
	R_BindTexImage( floatPbuffer );

	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, downSampleVertexProgram );
	qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, downSampleFragmentProgram );

	RB_EXP_CoverScreen();

	//
	// mip map again
	//
	qglViewport( 0, 0, glConfig.vidWidth>>2, glConfig.vidHeight>>2 );

	// draw to the second floatPbuffer
	R_MakeCurrent( floatPbufferQuarterDC, floatContext, floatPbufferQuarter );

	// read from the original floatPbuffer
	qglBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbuffer2Image->texnum );
	R_BindTexImage( floatPbuffer2 );

	RB_EXP_CoverScreen();

	//
	// blur horizontally
	//
	/*
# texture 0 is the high dynamic range buffer
# writes result.color as the fp16 result of a smeared bloom

PARAM	step =		program.local[0];		# { 1, 0 } or { 0, 1 } for horizontal / vertical separation
	*/

	// draw to the second floatPbuffer
	R_MakeCurrent( floatPbuffer2DC, floatContext, floatPbuffer2 );

	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, bloomVertexProgram );
	qglBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, bloomFragmentProgram );
	qglEnable(GL_VERTEX_PROGRAM_ARB);
	qglEnable(GL_FRAGMENT_PROGRAM_ARB);

	GL_SelectTextureNoClient( 0 );

	// blur horizontally first to the second floatPbuffer
	qglBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbufferQuarterImage->texnum );
	R_BindTexImage( floatPbufferQuarter );

	float	parm[4];

	parm[0] = 1;
	parm[1] = 0;
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, parm );

	RB_EXP_CoverScreen();

	//
	// now blur vertically back to the quarter pbuffer
	//
	R_MakeCurrent( floatPbufferQuarterDC, floatContext, floatPbufferQuarter );

	qglBindTexture( GL_TEXTURE_RECTANGLE_NV, floatPbuffer2Image->texnum );
	R_BindTexImage( floatPbuffer2 );

	parm[0] = 0;
	parm[1] = 1;
	parm[2] = 0;
	parm[3] = 0;
	qglProgramLocalParameter4fvARB( GL_FRAGMENT_PROGRAM_ARB, 0, parm );

	RB_EXP_CoverScreen();

	//========================

	qglEnable( GL_DEPTH_TEST );
	qglEnable( GL_SCISSOR_TEST );

	qglDisable(GL_VERTEX_PROGRAM_ARB);
	qglDisable(GL_FRAGMENT_PROGRAM_ARB);

	GL_CheckErrors();
}

/*
==================
RB_Exp_DrawInteractions
==================
*/
void    RB_Exp_DrawInteractions( void ) {
	if ( !initialized ) {
		R_Exp_Allocate();
	}

	if ( !backEnd.viewDef->viewLights ) {
		return;
	}

	// validate the samples
	if ( r_sb_samples.GetInteger() != 16 && r_sb_samples.GetInteger() != 4 && r_sb_samples.GetInteger() != 1 ) {
		r_sb_samples.SetInteger( 0 );
	}

	// validate the light resolution
	if ( r_sb_lightResolution.GetInteger() < 64 ) {
		r_sb_lightResolution.SetInteger( 64 );
	} else if ( r_sb_lightResolution.GetInteger() > maxLightBufferSize ) {
		r_sb_lightResolution.SetInteger( maxLightBufferSize );
	}
	lightBufferSize = r_sb_lightResolution.GetInteger();
	lightBufferSizeFraction = (float)lightBufferSize / maxLightBufferSize;

	// validate the view resolution
	if ( r_sb_viewResolution.GetInteger() < 64 ) {
		r_sb_viewResolution.SetInteger( 64 );
	} else if ( r_sb_viewResolution.GetInteger() > maxViewBufferSize ) {
		r_sb_viewResolution.SetInteger( maxViewBufferSize );
	}
	viewBufferSize = r_sb_viewResolution.GetInteger();
	viewBufferHeight = viewBufferSize * 3 / 4;
	viewBufferSizeFraction = (float)viewBufferSize / maxViewBufferSize;
	viewBufferHeightFraction = (float)viewBufferHeight / maxViewBufferSize;
	if (  viewBufferSize == backEnd.viewDef->viewport.x2 -  backEnd.viewDef->viewport.x1 + 1 ) {
		nativeViewBuffer = true;
	} else {
		nativeViewBuffer = false;
	}
	
	// set up for either point sampled or percentage-closer filtering for the shadow sampling
	shadowImage[0]->BindFragment();
	if ( r_sb_linearFilter.GetBool() ) {
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	} else {
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE );

	globalImages->BindNull();

	// copy the current depth buffer to a texture for image-space shadowing,
	// or re-render at a lower resolution
	R_EXP_RenderViewDepthImage();

	GL_SelectTexture( 0 );
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );

	// disable stencil shadow test
	qglStencilFunc( GL_ALWAYS, 128, 255 );

	// the jitter image will be used to offset sample centers
	GL_SelectTextureNoClient( 8 );
	if ( r_sb_samples.GetInteger() == 16 ) {
		jitterImage16->BindFragment();
	} else if ( r_sb_samples.GetInteger() == 4 ) {
		jitterImage4->BindFragment();
	} else {
		jitterImage1->BindFragment();
	}

	// if we are using a float buffer, clear it now
	if ( r_hdr_useFloats.GetBool() ) {
		RB_EXP_SetRenderBuffer( NULL );
		// we need to set a lot of things, because this is a completely different context
		RB_SetDefaultGLState();
		qglClearColor( 0.001f, 1.0f, 0.01f, 0.1f );
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		// clear the z buffer, set the projection matrix, etc
		RB_BeginDrawingView();
		RB_STD_FillDepthBuffer( (drawSurf_t **)&backEnd.viewDef->drawSurfs[0], backEnd.viewDef->numDrawSurfs );
	}


	//
	// for each light, perform adding and shadowing
	//
	for ( viewLight_t *vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next ) {
		backEnd.vLight = vLight;

		const idMaterial	*lightShader = vLight->lightShader;

		// do fogging later
		if ( lightShader->IsFogLight() ) {
			continue;
		}
		if ( lightShader->IsBlendLight() ) {
			continue;
		}

		if ( !vLight->localInteractions && !vLight->globalInteractions
			&& !vLight->translucentInteractions ) {
			continue;
		}

		if ( !vLight->frustumTris->ambientCache ) {
			R_CreateAmbientCache( const_cast<srfTriangles_t *>(vLight->frustumTris), false ); 
		}

		// all light side projections must currently match, so non-centered
		// and non-cubic lights must take the largest length
		viewLightAxialSize = R_EXP_CalcLightAxialSize( vLight );

		int	side, sideStop;

		if ( vLight->lightDef->parms.pointLight ) {
			if ( r_sb_singleSide.GetInteger() != -1 ) {
				side = r_sb_singleSide.GetInteger();
				sideStop = side+1;
			} else {
				side = 0;
				sideStop = 6;
			}
		} else {
			side = -1;
			sideStop = 0;
		}

		for (  ; side < sideStop ; side++ ) {
			// FIXME: check for frustums completely off the screen

			// render a shadow buffer
			RB_RenderShadowBuffer( vLight, side );

			// back to view rendering, possibly in the off-screen buffer
			if ( nativeViewBuffer || !r_sb_screenSpaceShadow.GetBool() ) {
				// directly to screen
				RB_EXP_SetRenderBuffer( vLight );
			} else {
				// to off screen buffer
				if ( r_sb_usePbuffer.GetBool() ) {
					GL_CheckErrors();
					// set the current openGL drawable to the shadow buffer
					R_MakeCurrent( viewPbufferDC, win32.hGLRC, viewPbuffer );
				}
				qglViewport( 0, 0, viewBufferSize, viewBufferHeight );
				qglScissor( 0, 0, viewBufferSize, viewBufferHeight );	// !@# FIXME: scale light scissor
			}

			// render the shadows into destination alpha on the included pixels
			RB_Exp_SelectFrustum( vLight, side );

			if ( !r_sb_screenSpaceShadow.GetBool() ) {
				// bind shadow buffer to texture
				GL_SelectTextureNoClient( 7 );
				shadowImage[0]->BindFragment();

				RB_EXP_CreateDrawInteractions( vLight->localInteractions );
				RB_EXP_CreateDrawInteractions( vLight->globalInteractions );
				backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
				RB_EXP_CreateDrawInteractions( vLight->translucentInteractions );
				backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
			}
		}

		// render the native window coordinates interactions
		if ( r_sb_screenSpaceShadow.GetBool() ) {
			if ( !nativeViewBuffer ) {
				RB_shadowResampleAlpha();
				qglEnable( GL_STENCIL_TEST );
			} else {
				RB_EXP_SetRenderBuffer( vLight );
if ( r_ignore.GetBool() ) {
qglEnable( GL_STENCIL_TEST );	//!@#
} else {
qglDisable( GL_STENCIL_TEST );	//!@#
}
			}
			RB_EXP_CreateDrawInteractions( vLight->localInteractions );
			RB_EXP_CreateDrawInteractions( vLight->globalInteractions );
			backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
			RB_EXP_CreateDrawInteractions( vLight->translucentInteractions );
			backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
		}
	}

	GL_SelectTexture( 0 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );

	// experimental transfer function
	for ( int i = 7 ; i >= 0 ; i-- ) {
		GL_SelectTextureNoClient( i );
		globalImages->BindNull();
	}
	GL_State( 0 );

	RB_EXP_Bloom();
	RB_EXP_GammaDither();

	// these haven't been state saved
	for ( int i = 0 ; i < 8 ; i++ ) {
		backEnd.glState.tmu[i].current2DMap = -1;
	}

	// take it out of texture compare mode so I can testImage it for debugging
	shadowImage[0]->BindFragment();
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE );


}



/*
==================
R_Exp_Init
==================
*/
void R_Exp_Init( void ) {
	glConfig.allowExpPath = false;

	common->Printf( "---------- R_Exp_Init ----------\n" );

	if ( !glConfig.ARBVertexProgramAvailable || !glConfig.ARBFragmentProgramAvailable ) {
		common->Printf( "Not available.\n" );
		return;
	}
RB_CreateBloomTable();
#if 0
	if ( !R_CheckExtension( "GL_NV_float_buffer" ) ) {
		common->Printf( "Not available.\n" );
		return;
	}
	if ( !R_CheckExtension( "GL_NV_texture_rectangle" ) ) {
		common->Printf( "Not available.\n" );
		return;
	}
#endif

#if 0
	qglCombinerParameterfvNV = (void (APIENTRY *)( GLenum pname, const GLfloat *params ))
		GLimp_ExtensionPointer( "glCombinerParameterfvNV" );
#endif

	common->Printf( "Available.\n" );

	if ( !idStr::Icmp( r_renderer.GetString(), "exp" ) ) {
		R_Exp_Allocate();
	}

	common->Printf( "--------------------------------------------\n" );

	glConfig.allowExpPath = true;
}

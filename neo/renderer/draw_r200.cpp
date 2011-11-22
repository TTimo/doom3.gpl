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

/*

  There are not enough vertex program texture coordinate outputs
  to have unique texture coordinates for bump, specular, and diffuse,
  so diffuse and specular are assumed to be the same mapping.

  To handle properly, those cases with different diffuse and specular
  mapping will need to be run as two passes.

*/

// changed from 1 to 255 to not conflict with ARB2 program names
static int	FPROG_FAST_PATH = 255;

typedef struct {
	GLint	numFragmentRegisters;	// 6
	GLint	numFragmentConstants;	// 8
	GLint	numPasses;				// 2
	GLint	numInstructionsPerPass;	// 8
	GLint	numInstructionsTotal;	// 16
	GLint	colorAlphaPairing;		// 1
	GLint	numLoopbackComponenets;	// 3
	GLint	numInputInterpolatorComponents;	// 3
} atiFragmentShaderInfo_t;

static atiFragmentShaderInfo_t	fsi;

typedef struct {
	// vertex shader invariants
	int	lightPos;		// light position in object coordinates
	int	viewerPos;		// viewer position in object coordinates
	int	lightProjectS;	// projected light s texgen
	int	lightProjectT;	// projected light t texgen
	int	lightProjectQ;	// projected light q texgen
	int	lightFalloffS;	// projected light falloff s texgen
	int	bumpTransformS;	// bump TEX0 S transformation
	int	bumpTransformT;	// bump TEX0 T transformation
	int	colorTransformS;	// diffuse/specular texture matrix
	int	colorTransformT;	// diffuse/specular texture matrix

	// vertex shader variants
	int	texCoords;
	int	vertexColors;
	int	normals;
	int	tangents;
	int	biTangents;

} atiVertexShaderInfo_t;

static atiVertexShaderInfo_t	vsi;

/*
===================
RB_R200_ARB_DrawInteraction

===================
*/
static void RB_R200_ARB_DrawInteraction( const drawInteraction_t *din ) {
	// check for the case we can't handle in a single pass (we could calculate this at shader parse time to optimize)
	if ( din->diffuseImage != globalImages->blackImage && din->specularImage != globalImages->blackImage
		&& memcmp( din->specularMatrix, din->diffuseMatrix, sizeof( din->diffuseMatrix ) ) ) {
//		common->Printf( "Note: Shader %s drawn as two pass on R200\n", din->surf->shader->getName() );

		// draw the specular as a separate pass with a black diffuse map
		drawInteraction_t	d;
		d = *din;
		d.diffuseImage = globalImages->blackImage;
		memcpy( d.diffuseMatrix, d.specularMatrix, sizeof( d.diffuseMatrix ) );
		RB_R200_ARB_DrawInteraction( &d );

		// now fall through and draw the diffuse pass with a black specular map
		d = *din;
		din = &d;
		d.specularImage = globalImages->blackImage;
	}

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
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_SPECULAR_MATRIX_S, din->diffuseMatrix[0].ToFloatPtr() );
	qglProgramEnvParameter4fvARB( GL_VERTEX_PROGRAM_ARB, PP_SPECULAR_MATRIX_T, din->diffuseMatrix[1].ToFloatPtr() );

	const srfTriangles_t	*tri = din->surf->geo;
	idDrawVert	*ac = (idDrawVert *)vertexCache.Position( tri->ambientCache );
	qglVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), (void *)&ac->xyz );

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


	// texture 0 = light projection
	// texture 1 = light falloff
	// texture 2 = surface diffuse
	// texture 3 = surface specular
	// texture 4 = surface bump
	// texture 5 = normalization cube map

	GL_SelectTexture( 5 );
	if ( din->ambientLight ) {
		globalImages->ambientNormalMap->Bind();
	} else {
		globalImages->normalCubeMapImage->Bind();
	}

	GL_SelectTexture( 4 );
	din->bumpImage->Bind();

	GL_SelectTexture( 3 );
	din->specularImage->Bind();
	qglTexCoordPointer( 3, GL_FLOAT, sizeof( idDrawVert ), (void *)&ac->normal );

	GL_SelectTexture( 2 );
	din->diffuseImage->Bind();
	qglTexCoordPointer( 3, GL_FLOAT, sizeof( idDrawVert ), (void *)&ac->tangents[1][0] );

	GL_SelectTexture( 1 );
	din->lightFalloffImage->Bind();
	qglTexCoordPointer( 3, GL_FLOAT, sizeof( idDrawVert ), (void *)&ac->tangents[0][0] );

	GL_SelectTexture( 0 );
	din->lightImage->Bind();
	qglTexCoordPointer( 2, GL_FLOAT, sizeof( idDrawVert ), (void *)&ac->st[0] );

	qglSetFragmentShaderConstantATI( GL_CON_0_ATI, din->diffuseColor.ToFloatPtr() );
	qglSetFragmentShaderConstantATI( GL_CON_1_ATI, din->specularColor.ToFloatPtr() );

	if ( din->vertexColor != SVC_IGNORE ) {
		qglColorPointer( 4, GL_UNSIGNED_BYTE, sizeof(idDrawVert), (void *)&ac->color );
		qglEnableClientState( GL_COLOR_ARRAY );

		RB_DrawElementsWithCounters( tri );

		qglDisableClientState( GL_COLOR_ARRAY );
		qglColor4f( 1, 1, 1, 1 );
	} else {
		RB_DrawElementsWithCounters( tri );
	}
}

/*
==================
RB_R200_ARB_CreateDrawInteractions
==================
*/
static void RB_R200_ARB_CreateDrawInteractions( const drawSurf_t *surf ) {
	if ( !surf ) {
		return;
	}

	// force a space calculation for light vectors
	backEnd.currentSpace = NULL;

	// set the depth test
	if ( surf->material->Coverage() == MC_TRANSLUCENT /* != C_PERFORATED */ ) {
		GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | GLS_DEPTHFUNC_LESS );
	} else {
		// only draw on the alpha tested pixels that made it to the depth buffer
		GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | GLS_DEPTHFUNC_EQUAL );
	}

	// start the vertex shader
	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_R200_INTERACTION );
	qglEnable(GL_VERTEX_PROGRAM_ARB);

	// start the fragment shader
	qglBindFragmentShaderATI( FPROG_FAST_PATH );
#if defined( MACOS_X )
	qglEnable( GL_TEXT_FRAGMENT_SHADER_ATI );
#else
	qglEnable( GL_FRAGMENT_SHADER_ATI );
#endif

	qglColor4f( 1, 1, 1, 1 );

	GL_SelectTexture( 1 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
	GL_SelectTexture( 2 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
	GL_SelectTexture( 3 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );

	for ( ; surf ; surf=surf->nextOnLight ) {
		RB_CreateSingleDrawInteractions( surf, RB_R200_ARB_DrawInteraction );
	}

	GL_SelectTexture( 5 );
	globalImages->BindNull();

	GL_SelectTexture( 4 );
	globalImages->BindNull();

	GL_SelectTexture( 3 );
	globalImages->BindNull();
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );

	GL_SelectTexture( 2 );
	globalImages->BindNull();
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );

	GL_SelectTexture( 1 );
	globalImages->BindNull();
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );

	GL_SelectTexture( 0 );

	qglDisable( GL_VERTEX_PROGRAM_ARB );
#if defined( MACOS_X )
	qglDisable( GL_TEXT_FRAGMENT_SHADER_ATI );
#else
	qglDisable( GL_FRAGMENT_SHADER_ATI );
#endif
}



/*
==================
RB_R200_DrawInteractions

==================
*/
void RB_R200_DrawInteractions( void ) {
	qglEnable( GL_STENCIL_TEST );

	for ( viewLight_t *vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next ) {
		// do fogging later
		if ( vLight->lightShader->IsFogLight() ) {
			continue;
		}
		if ( vLight->lightShader->IsBlendLight() ) {
			continue;
		}

		backEnd.vLight = vLight;

		RB_LogComment( "---------- RB_RenderViewLight 0x%p ----------\n", vLight );

		// clear the stencil buffer if needed
		if ( vLight->globalShadows || vLight->localShadows ) {
			backEnd.currentScissor = vLight->scissorRect;
			if ( r_useScissor.GetBool() ) {
				qglScissor( backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1, 
					backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
					backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
					backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1 );
			}
			qglClear( GL_STENCIL_BUFFER_BIT );
		} else {
			// no shadows, so no need to read or write the stencil buffer
			// we might in theory want to use GL_ALWAYS instead of disabling
			// completely, to satisfy the invarience rules
			qglStencilFunc( GL_ALWAYS, 128, 255 );
		}

		if ( r_useShadowVertexProgram.GetBool() ) {
			qglEnable( GL_VERTEX_PROGRAM_ARB );
			qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
			RB_StencilShadowPass( vLight->globalShadows );

			RB_R200_ARB_CreateDrawInteractions( vLight->localInteractions );

			qglEnable( GL_VERTEX_PROGRAM_ARB );
			qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
			RB_StencilShadowPass( vLight->localShadows );

			RB_R200_ARB_CreateDrawInteractions( vLight->globalInteractions );

			qglDisable( GL_VERTEX_PROGRAM_ARB );	// if there weren't any globalInteractions, it would have stayed on
		} else {
			RB_StencilShadowPass( vLight->globalShadows );
			RB_R200_ARB_CreateDrawInteractions( vLight->localInteractions );

			RB_StencilShadowPass( vLight->localShadows );
			RB_R200_ARB_CreateDrawInteractions( vLight->globalInteractions );
		}

		if ( r_skipTranslucent.GetBool() ) {
			continue;
		}

		// disable stencil testing for translucent interactions, because
		// the shadow isn't calculated at their point, and the shadow
		// behind them may be depth fighting with a back side, so there
		// isn't any reasonable thing to do
		qglStencilFunc( GL_ALWAYS, 128, 255 );
		RB_R200_ARB_CreateDrawInteractions( vLight->translucentInteractions );
	}
}


/*
=================
R_BuildSurfaceFragmentProgram
=================
*/
static void R_BuildSurfaceFragmentProgram( int programNum ) {
	qglBindFragmentShaderATI( programNum );

	qglBeginFragmentShaderATI();

	// texture 0 = light projection
	// texture 1 = light falloff
	// texture 2 = surface diffuse
	// texture 3 = surface specular
	// texture 4 = surface bump
	// texture 5 = normalization cube map

	// texcoord 0 = light projection texGen
	// texcoord 1 = light falloff texGen
	// texcoord 2 = bumpmap texCoords
	// texcoord 3 = specular / diffuse texCoords
	// texcoord 4 = halfangle vector in local tangent space
	// texcoord 5 = vector to light in local tangent space

	// constant 0 = diffuse modulate
	// constant 1 = specular modulate
	// constant 5 = internal use for 0.75 constant

	qglSampleMapATI( GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STQ_DQ_ATI );
	qglSampleMapATI( GL_REG_1_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI );
	qglSampleMapATI( GL_REG_4_ATI, GL_TEXTURE2_ARB, GL_SWIZZLE_STR_ATI );
	qglSampleMapATI( GL_REG_5_ATI, GL_TEXTURE5_ARB, GL_SWIZZLE_STR_ATI );

	// move the alpha component to the red channel to support rxgb normal map compression
	if ( globalImages->image_useNormalCompression.GetInteger() == 2 ) {
		qglColorFragmentOp1ATI( GL_MOV_ATI, GL_REG_4_ATI, GL_RED_BIT_ATI, GL_NONE,
			GL_REG_4_ATI, GL_ALPHA, GL_NONE );
	}

	// light projection * light falloff
	qglColorFragmentOp2ATI( GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
		GL_REG_0_ATI, GL_NONE, GL_NONE,
		GL_REG_1_ATI, GL_NONE, GL_NONE );

	// vectorToLight dot bumpMap
	qglColorFragmentOp2ATI( GL_DOT3_ATI, GL_REG_1_ATI, GL_NONE, GL_SATURATE_BIT_ATI,
		GL_REG_4_ATI, GL_NONE, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI,
		GL_REG_5_ATI, GL_NONE, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI );

	// bump * light
	qglColorFragmentOp2ATI( GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
		GL_REG_0_ATI, GL_NONE, GL_NONE,
		GL_REG_1_ATI, GL_NONE, GL_NONE );

	//-------------------

	// carry over the incomingLight calculation
	qglPassTexCoordATI( GL_REG_0_ATI, GL_REG_0_ATI, GL_SWIZZLE_STR_ATI );

	// sample the diffuse surface map
	qglSampleMapATI( GL_REG_2_ATI, GL_TEXTURE3_ARB, GL_SWIZZLE_STR_ATI );

	// sample the specular surface map
	qglSampleMapATI( GL_REG_3_ATI, GL_TEXTURE3_ARB, GL_SWIZZLE_STR_ATI );

	// we will use the surface bump map again
	qglPassTexCoordATI( GL_REG_4_ATI, GL_REG_4_ATI, GL_SWIZZLE_STR_ATI );

	// normalize the specular halfangle
	qglSampleMapATI( GL_REG_5_ATI, GL_TEXTURE4_ARB, GL_SWIZZLE_STR_ATI );
	
	// R1 = halfangle dot surfaceNormal
	qglColorFragmentOp2ATI( GL_DOT3_ATI, GL_REG_1_ATI, GL_NONE, GL_SATURATE_BIT_ATI,
		GL_REG_4_ATI, GL_NONE, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI,
		GL_REG_5_ATI, GL_NONE, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI );

	// R1 = 4 * ( R1 - 0.75 )
	// subtract 0.75 and quadruple to tighten the specular spot
	float data[4] = { 0.75, 0.75,  0.75,  0.75 };
	qglSetFragmentShaderConstantATI( GL_CON_5_ATI, data );
	qglColorFragmentOp2ATI( GL_SUB_ATI, GL_REG_1_ATI, GL_NONE, GL_4X_BIT_ATI | GL_SATURATE_BIT_ATI,
		GL_REG_1_ATI, GL_NONE, GL_NONE,
		GL_CON_5_ATI, GL_NONE, GL_NONE );

	// R1 = R1 * R1
	// sqare the stretched specular result
	qglColorFragmentOp2ATI( GL_MUL_ATI, GL_REG_1_ATI, GL_NONE, GL_SATURATE_BIT_ATI,
		GL_REG_1_ATI, GL_NONE, GL_NONE,
		GL_REG_1_ATI, GL_NONE, GL_NONE );

	// R1 = R1 * R3
	// R1 = specular power * specular texture * 2
	qglColorFragmentOp2ATI( GL_MUL_ATI, GL_REG_1_ATI, GL_NONE, GL_2X_BIT_ATI | GL_SATURATE_BIT_ATI,
		GL_REG_1_ATI, GL_NONE, GL_NONE,
		GL_REG_3_ATI, GL_NONE, GL_NONE );

	// R2 = R2 * CONST0
	// down modulate the diffuse map
	qglColorFragmentOp2ATI( GL_MUL_ATI, GL_REG_2_ATI, GL_NONE, GL_SATURATE_BIT_ATI,
		GL_REG_2_ATI, GL_NONE, GL_NONE,
		GL_CON_0_ATI, GL_NONE, GL_NONE );

	// R2 = R2 + R1 * CONST1
	// diffuse + specular * specular color
	qglColorFragmentOp3ATI( GL_MAD_ATI, GL_REG_2_ATI, GL_NONE, GL_SATURATE_BIT_ATI,
		GL_REG_1_ATI, GL_NONE, GL_NONE,
		GL_CON_1_ATI, GL_NONE, GL_NONE,
		GL_REG_2_ATI, GL_NONE, GL_NONE );

	// out = reflectance * incoming light
	qglColorFragmentOp2ATI( GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_SATURATE_BIT_ATI,
		GL_REG_0_ATI, GL_NONE, GL_NONE,
		GL_REG_2_ATI, GL_NONE, GL_NONE );

	// out * vertex color
	qglColorFragmentOp2ATI( GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
		GL_REG_0_ATI, GL_NONE, GL_NONE,
		GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE );

	// out alpha = 0 to allow blending optimization
	qglAlphaFragmentOp1ATI( GL_MOV_ATI, GL_REG_0_ATI, GL_NONE,
		GL_ZERO, GL_NONE, GL_NONE );

	qglEndFragmentShaderATI();

	GL_CheckErrors();
}

/*
=================
R_R200_Init
=================
*/
void R_R200_Init( void ) {
	glConfig.allowR200Path = false;

	common->Printf( "----------- R200_Init -----------\n" );

	if ( !glConfig.atiFragmentShaderAvailable || !glConfig.ARBVertexProgramAvailable || !glConfig.ARBVertexBufferObjectAvailable ) {
		common->Printf( "Not available.\n" );
		return;
	}

	GL_CheckErrors();

	qglGetIntegerv( GL_NUM_FRAGMENT_REGISTERS_ATI, &fsi.numFragmentRegisters );
	qglGetIntegerv( GL_NUM_FRAGMENT_CONSTANTS_ATI, &fsi.numFragmentConstants );
	qglGetIntegerv( GL_NUM_PASSES_ATI, &fsi.numPasses );
	qglGetIntegerv( GL_NUM_INSTRUCTIONS_PER_PASS_ATI, &fsi.numInstructionsPerPass );
	qglGetIntegerv( GL_NUM_INSTRUCTIONS_TOTAL_ATI, &fsi.numInstructionsTotal );
	qglGetIntegerv( GL_COLOR_ALPHA_PAIRING_ATI, &fsi.colorAlphaPairing );
	qglGetIntegerv( GL_NUM_LOOPBACK_COMPONENTS_ATI, &fsi.numLoopbackComponenets );
	qglGetIntegerv( GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI, &fsi.numInputInterpolatorComponents );

	common->Printf( "GL_NUM_FRAGMENT_REGISTERS_ATI: %i\n", fsi.numFragmentRegisters );
	common->Printf( "GL_NUM_FRAGMENT_CONSTANTS_ATI: %i\n", fsi.numFragmentConstants );
	common->Printf( "GL_NUM_PASSES_ATI: %i\n", fsi.numPasses );
	common->Printf( "GL_NUM_INSTRUCTIONS_PER_PASS_ATI: %i\n", fsi.numInstructionsPerPass );
	common->Printf( "GL_NUM_INSTRUCTIONS_TOTAL_ATI: %i\n", fsi.numInstructionsTotal );
	common->Printf( "GL_COLOR_ALPHA_PAIRING_ATI: %i\n", fsi.colorAlphaPairing );
	common->Printf( "GL_NUM_LOOPBACK_COMPONENTS_ATI: %i\n", fsi.numLoopbackComponenets );
	common->Printf( "GL_NUM_INPUT_INTERPOLATOR_COMPONENTS_ATI: %i\n", fsi.numInputInterpolatorComponents );

	common->Printf( "FPROG_FAST_PATH\n" );
	R_BuildSurfaceFragmentProgram( FPROG_FAST_PATH );

	common->Printf( "---------------------\n" );

	glConfig.allowR200Path = true;
}

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

typedef enum {
	FPROG_BUMP_AND_LIGHT,
	FPROG_DIFFUSE_COLOR,
	FPROG_SPECULAR_COLOR,
	FPROG_DIFFUSE_AND_SPECULAR_COLOR,

	FPROG_NUM_FRAGMENT_PROGRAMS
} fragmentProgram_t;

GLuint	fragmentDisplayListBase;	// FPROG_NUM_FRAGMENT_PROGRAMS lists

void RB_NV20_DependentSpecularPass( const drawInteraction_t *din );
void RB_NV20_DependentAmbientPass( void );

/*
=========================================================================================

GENERAL INTERACTION RENDERING

=========================================================================================
*/

/*
====================
GL_SelectTextureNoClient
====================
*/
void GL_SelectTextureNoClient( int unit ) {
	backEnd.glState.currenttmu = unit;
	qglActiveTextureARB( GL_TEXTURE0_ARB + unit );
	RB_LogComment( "glActiveTextureARB( %i )\n", unit );
}

/*
==================
RB_NV20_BumpAndLightFragment
==================
*/
static void RB_NV20_BumpAndLightFragment( void ) {
	if ( r_useCombinerDisplayLists.GetBool() ) {
		qglCallList( fragmentDisplayListBase + FPROG_BUMP_AND_LIGHT );
		return;
	}

	// program the nvidia register combiners
	qglCombinerParameteriNV( GL_NUM_GENERAL_COMBINERS_NV, 3 );

	// stage 0 rgb performs the dot product
	// SPARE0 = TEXTURE0 dot TEXTURE1
	qglCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_TEXTURE1_ARB, GL_EXPAND_NORMAL_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_TEXTURE0_ARB, GL_EXPAND_NORMAL_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER0_NV, GL_RGB, 
		GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_NONE, GL_NONE, GL_TRUE, GL_FALSE, GL_FALSE );


	// stage 1 rgb multiplies texture 2 and 3 together
	// SPARE1 = TEXTURE2 * TEXTURE3
	qglCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_TEXTURE2_ARB, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_TEXTURE3_ARB, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER1_NV, GL_RGB, 
		GL_SPARE1_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// stage 1 alpha does nohing

	// stage 2 color multiplies spare0 * spare 1 just for debugging
	// SPARE0 = SPARE0 * SPARE1
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER2_NV, GL_RGB, 
		GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// stage 2 alpha multiples spare0 * spare 1
	// SPARE0 = SPARE0 * SPARE1
	qglCombinerInputNV( GL_COMBINER2_NV, GL_ALPHA, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_BLUE );
	qglCombinerInputNV( GL_COMBINER2_NV, GL_ALPHA, GL_VARIABLE_B_NV, 
		GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_BLUE );
	qglCombinerOutputNV( GL_COMBINER2_NV, GL_ALPHA, 
		GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// final combiner
	qglFinalCombinerInputNV( GL_VARIABLE_D_NV, GL_SPARE0_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_A_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_B_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_C_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_G_NV, GL_SPARE0_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_ALPHA );
}

/*
==================
RB_NV20_DI_BumpAndLightPass

We are going to write alpha as light falloff * ( bump dot light ) * lightProjection
If the light isn't a monoLightShader, the lightProjection will be skipped, because
it will have to be done on an itterated basis
==================
*/
static void RB_NV20_DI_BumpAndLightPass( const drawInteraction_t *din, bool monoLightShader ) {
	RB_LogComment( "---------- RB_NV_BumpAndLightPass ----------\n" );

	GL_State( GLS_COLORMASK | GLS_DEPTHMASK | backEnd.depthFunc );

	// texture 0 is the normalization cube map
	// GL_TEXTURE0_ARB will be the normalized vector
	// towards the light source
#ifdef MACOS_X
	GL_SelectTexture( 0 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 0 );
#endif
	if ( din->ambientLight ) {
		globalImages->ambientNormalMap->Bind();
	} else {
		globalImages->normalCubeMapImage->Bind();
	}

	// texture 1 will be the per-surface bump map
#ifdef MACOS_X
	GL_SelectTexture( 1 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 1 );
#endif
	din->bumpImage->Bind();

	// texture 2 will be the light falloff texture
#ifdef MACOS_X
	GL_SelectTexture( 2 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 2 );
#endif
	din->lightFalloffImage->Bind();

	// texture 3 will be the light projection texture
#ifdef MACOS_X
	GL_SelectTexture( 3 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 3 );
#endif
	if ( monoLightShader ) {
		din->lightImage->Bind();
	} else {
		// if the projected texture is multi-colored, we
		// will need to do it in subsequent passes
		globalImages->whiteImage->Bind();
	}

	// bind our "fragment program"
	RB_NV20_BumpAndLightFragment();

	// draw it
	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_NV20_BUMP_AND_LIGHT );
	RB_DrawElementsWithCounters( din->surf->geo );
}


/*
==================
RB_NV20_DiffuseColorFragment
==================
*/
static void RB_NV20_DiffuseColorFragment( void ) {
	if ( r_useCombinerDisplayLists.GetBool() ) {
		qglCallList( fragmentDisplayListBase + FPROG_DIFFUSE_COLOR );
		return;
	}

	// program the nvidia register combiners
	qglCombinerParameteriNV( GL_NUM_GENERAL_COMBINERS_NV, 1 );

	// stage 0 is free, so we always do the multiply of the vertex color
	// when the vertex color is inverted, qglCombinerInputNV(GL_VARIABLE_B_NV) will be changed
	qglCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_TEXTURE0_ARB, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_PRIMARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER0_NV, GL_RGB, 
		GL_TEXTURE0_ARB, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	qglCombinerOutputNV( GL_COMBINER0_NV, GL_ALPHA, 
		GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );


	// for GL_CONSTANT_COLOR0_NV * TEXTURE0 * TEXTURE1
	qglFinalCombinerInputNV( GL_VARIABLE_A_NV, GL_CONSTANT_COLOR0_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_B_NV, GL_E_TIMES_F_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_C_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_D_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_E_NV, GL_TEXTURE0_ARB,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_F_NV, GL_TEXTURE1_ARB,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_G_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_ALPHA );

}

/*
==================
RB_NV20_DI_DiffuseColorPass

==================
*/
static void RB_NV20_DI_DiffuseColorPass( const drawInteraction_t *din ) {
	RB_LogComment( "---------- RB_NV20_DiffuseColorPass ----------\n" );

	GL_State( GLS_SRCBLEND_DST_ALPHA | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | GLS_ALPHAMASK 
		| backEnd.depthFunc );

	// texture 0 will be the per-surface diffuse map
#ifdef MACOS_X
	GL_SelectTexture( 0 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 0 );
#endif
	din->diffuseImage->Bind();

	// texture 1 will be the light projected texture
#ifdef MACOS_X
	GL_SelectTexture( 1 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 1 );
#endif
	din->lightImage->Bind();

	// texture 2 is disabled
#ifdef MACOS_X
	GL_SelectTexture( 2 );
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 2 );
#endif
	globalImages->BindNull();

	// texture 3 is disabled
#ifdef MACOS_X
	GL_SelectTexture( 3 );
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 3 );
#endif
	globalImages->BindNull();

	// bind our "fragment program"
	RB_NV20_DiffuseColorFragment();

	// override one parameter for inverted vertex color
	if ( din->vertexColor == SVC_INVERSE_MODULATE ) {
		qglCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, 
			GL_PRIMARY_COLOR_NV, GL_UNSIGNED_INVERT_NV, GL_RGB );
	}

	// draw it
	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_NV20_DIFFUSE_COLOR );
	RB_DrawElementsWithCounters( din->surf->geo );
}


/*
==================
RB_NV20_SpecularColorFragment
==================
*/
static void RB_NV20_SpecularColorFragment( void ) {
	if ( r_useCombinerDisplayLists.GetBool() ) {
		qglCallList( fragmentDisplayListBase + FPROG_SPECULAR_COLOR );
		return;
	}

	// program the nvidia register combiners
	qglCombinerParameteriNV( GL_NUM_GENERAL_COMBINERS_NV, 4 );

	// we want GL_CONSTANT_COLOR1_NV * PRIMARY_COLOR * TEXTURE2 * TEXTURE3 * specular( TEXTURE0 * TEXTURE1 )

	// stage 0 rgb performs the dot product
	// GL_SPARE0_NV = ( TEXTURE0 dot TEXTURE1 - 0.5 ) * 2
	// TEXTURE2 = TEXTURE2 * PRIMARY_COLOR
	// the scale and bias steepen the specular curve
	qglCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_TEXTURE1_ARB, GL_EXPAND_NORMAL_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_TEXTURE0_ARB, GL_EXPAND_NORMAL_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER0_NV, GL_RGB, 
		GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_SCALE_BY_TWO_NV, GL_BIAS_BY_NEGATIVE_ONE_HALF_NV, GL_TRUE, GL_FALSE, GL_FALSE );

	// stage 0 alpha does nothing

	// stage 1 color takes bump * bump
	// GL_SPARE0_NV = ( GL_SPARE0_NV * GL_SPARE0_NV - 0.5 ) * 2
	// the scale and bias steepen the specular curve
	qglCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER1_NV, GL_RGB, 
		GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_SCALE_BY_TWO_NV, GL_BIAS_BY_NEGATIVE_ONE_HALF_NV, GL_FALSE, GL_FALSE, GL_FALSE );

	// stage 1 alpha does nothing

	// stage 2 color
	// GL_SPARE0_NV = GL_SPARE0_NV * TEXTURE3
	// SECONDARY_COLOR = CONSTANT_COLOR * TEXTURE2
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_TEXTURE3_ARB, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_CONSTANT_COLOR1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_D_NV, 
		GL_TEXTURE2_ARB, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER2_NV, GL_RGB, 
		GL_SPARE0_NV, GL_SECONDARY_COLOR_NV, GL_DISCARD_NV,
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// stage 2 alpha does nothing


	// stage 3 scales the texture by the vertex color
	qglCombinerInputNV( GL_COMBINER3_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SECONDARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER3_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_PRIMARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER3_NV, GL_RGB, 
		GL_SECONDARY_COLOR_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// stage 3 alpha does nothing

	// final combiner = GL_SPARE0_NV * SECONDARY_COLOR + PRIMARY_COLOR * SECONDARY_COLOR
	qglFinalCombinerInputNV( GL_VARIABLE_A_NV, GL_SPARE0_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_B_NV, GL_SECONDARY_COLOR_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_C_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_D_NV, GL_E_TIMES_F_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_E_NV, GL_SPARE0_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_F_NV, GL_SECONDARY_COLOR_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_G_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_ALPHA );
}


/*
==================
RB_NV20_DI_SpecularColorPass

==================
*/
static void RB_NV20_DI_SpecularColorPass( const drawInteraction_t *din ) {
	RB_LogComment( "---------- RB_NV20_SpecularColorPass ----------\n" );

	GL_State( GLS_SRCBLEND_DST_ALPHA | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | GLS_ALPHAMASK
		| backEnd.depthFunc );

	// texture 0 is the normalization cube map for the half angle
#ifdef MACOS_X
	GL_SelectTexture( 0 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 0 );
#endif
	globalImages->normalCubeMapImage->Bind();

	// texture 1 will be the per-surface bump map
#ifdef MACOS_X
	GL_SelectTexture( 1 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 1 );
#endif
	din->bumpImage->Bind();

	// texture 2 will be the per-surface specular map
#ifdef MACOS_X
	GL_SelectTexture( 2 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 2 );
#endif
	din->specularImage->Bind();

	// texture 3 will be the light projected texture
#ifdef MACOS_X
	GL_SelectTexture( 3 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 3 );
#endif
	din->lightImage->Bind();

	// bind our "fragment program"
	RB_NV20_SpecularColorFragment();

	// override one parameter for inverted vertex color
	if ( din->vertexColor == SVC_INVERSE_MODULATE ) {
		qglCombinerInputNV( GL_COMBINER3_NV, GL_RGB, GL_VARIABLE_B_NV, 
			GL_PRIMARY_COLOR_NV, GL_UNSIGNED_INVERT_NV, GL_RGB );
	}

	// draw it
	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_NV20_SPECULAR_COLOR );
	RB_DrawElementsWithCounters( din->surf->geo );
}



/*
==================
RB_NV20_DiffuseAndSpecularColorFragment
==================
*/
static void RB_NV20_DiffuseAndSpecularColorFragment( void ) {
	if ( r_useCombinerDisplayLists.GetBool() ) {
		qglCallList( fragmentDisplayListBase + FPROG_DIFFUSE_AND_SPECULAR_COLOR );
		return;
	}

	// program the nvidia register combiners
	qglCombinerParameteriNV( GL_NUM_GENERAL_COMBINERS_NV, 3 );

	// GL_CONSTANT_COLOR0_NV will be the diffuse color
	// GL_CONSTANT_COLOR1_NV will be the specular color

	// stage 0 rgb performs the dot product
	// GL_SECONDARY_COLOR_NV = ( TEXTURE0 dot TEXTURE1 - 0.5 ) * 2
	// the scale and bias steepen the specular curve
	qglCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_TEXTURE1_ARB, GL_EXPAND_NORMAL_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_TEXTURE0_ARB, GL_EXPAND_NORMAL_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER0_NV, GL_RGB, 
		GL_SECONDARY_COLOR_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_SCALE_BY_TWO_NV, GL_BIAS_BY_NEGATIVE_ONE_HALF_NV, GL_TRUE, GL_FALSE, GL_FALSE );

	// stage 0 alpha does nothing

	// stage 1 color takes bump * bump
	// PRIMARY_COLOR = ( GL_SECONDARY_COLOR_NV * GL_SECONDARY_COLOR_NV - 0.5 ) * 2
	// the scale and bias steepen the specular curve
	qglCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SECONDARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_SECONDARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER1_NV, GL_RGB, 
		GL_SECONDARY_COLOR_NV, GL_DISCARD_NV, GL_DISCARD_NV,
		GL_SCALE_BY_TWO_NV, GL_BIAS_BY_NEGATIVE_ONE_HALF_NV, GL_FALSE, GL_FALSE, GL_FALSE );

	// stage 1 alpha does nothing

	// stage 2 color
	// PRIMARY_COLOR = ( PRIMARY_COLOR * TEXTURE3 ) * 2
	// SPARE0 = 1.0 * 1.0 (needed for final combiner)
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_A_NV, 
		GL_SECONDARY_COLOR_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_B_NV, 
		GL_TEXTURE3_ARB, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_C_NV, 
		GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB );
	qglCombinerInputNV( GL_COMBINER2_NV, GL_RGB, GL_VARIABLE_D_NV, 
		GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB );
	qglCombinerOutputNV( GL_COMBINER2_NV, GL_RGB, 
		GL_SECONDARY_COLOR_NV, GL_SPARE0_NV, GL_DISCARD_NV,
		GL_SCALE_BY_TWO_NV, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

	// stage 2 alpha does nothing

	// final combiner = TEXTURE2_ARB * CONSTANT_COLOR0_NV + PRIMARY_COLOR_NV * CONSTANT_COLOR1_NV
	// alpha = GL_ZERO
	qglFinalCombinerInputNV( GL_VARIABLE_A_NV, GL_CONSTANT_COLOR1_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_B_NV, GL_SECONDARY_COLOR_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_C_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_D_NV, GL_E_TIMES_F_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_E_NV, GL_TEXTURE2_ARB,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_F_NV, GL_CONSTANT_COLOR0_NV,
		GL_UNSIGNED_IDENTITY_NV, GL_RGB );
	qglFinalCombinerInputNV( GL_VARIABLE_G_NV, GL_ZERO,
		GL_UNSIGNED_IDENTITY_NV, GL_ALPHA );
}


/*
==================
RB_NV20_DI_DiffuseAndSpecularColorPass

==================
*/
static void RB_NV20_DI_DiffuseAndSpecularColorPass( const drawInteraction_t *din ) {
	RB_LogComment( "---------- RB_NV20_DI_DiffuseAndSpecularColorPass ----------\n" );

	GL_State( GLS_SRCBLEND_DST_ALPHA | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc );

	// texture 0 is the normalization cube map for the half angle
// still bound from RB_NV_BumpAndLightPass
//	GL_SelectTextureNoClient( 0 );
//	GL_Bind( tr.normalCubeMapImage );

	// texture 1 is the per-surface bump map
// still bound from RB_NV_BumpAndLightPass
//	GL_SelectTextureNoClient( 1 );
//	GL_Bind( din->bumpImage );

	// texture 2 is the per-surface diffuse map
#ifdef MACOS_X
	GL_SelectTexture( 2 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 2 );
#endif
	din->diffuseImage->Bind();

	// texture 3 is the per-surface specular map
#ifdef MACOS_X
	GL_SelectTexture( 3 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	GL_SelectTextureNoClient( 3 );
#endif
	din->specularImage->Bind();

	// bind our "fragment program"
	RB_NV20_DiffuseAndSpecularColorFragment();

	// draw it
	qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_NV20_DIFFUSE_AND_SPECULAR_COLOR );
	RB_DrawElementsWithCounters( din->surf->geo );
}


/*
==================
RB_NV20_DrawInteraction
==================
*/
static void	RB_NV20_DrawInteraction( const drawInteraction_t *din ) {
	const drawSurf_t *surf = din->surf;

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

	// set the constant colors
	qglCombinerParameterfvNV( GL_CONSTANT_COLOR0_NV, din->diffuseColor.ToFloatPtr() );
	qglCombinerParameterfvNV( GL_CONSTANT_COLOR1_NV, din->specularColor.ToFloatPtr() );

	// vertex color passes should be pretty rare (cross-faded bump map surfaces), so always
	// run them down as three-passes
	if ( din->vertexColor != SVC_IGNORE ) {
		qglEnableClientState( GL_COLOR_ARRAY );
		RB_NV20_DI_BumpAndLightPass( din, false );
		RB_NV20_DI_DiffuseColorPass( din );
		RB_NV20_DI_SpecularColorPass( din );
		qglDisableClientState( GL_COLOR_ARRAY );
		return;
	}

	qglColor3f( 1, 1, 1 );

	// on an ideal card, we would now just bind the textures and call a
	// single pass vertex / fragment program, but
	// on NV20, we need to decide which single / dual / tripple pass set of programs to use

	// ambient light could be done as a single pass if we want to optimize for it

	// monochrome light is two passes
	int		internalFormat = din->lightImage->internalFormat;
	if ( ( r_useNV20MonoLights.GetInteger() == 2 ) || 
		( din->lightImage->isMonochrome && r_useNV20MonoLights.GetInteger() ) ) {
		// do a two-pass rendering
		RB_NV20_DI_BumpAndLightPass( din, true );
		RB_NV20_DI_DiffuseAndSpecularColorPass( din );
	} else {
		// general case is three passes
		// ( bump dot lightDir ) * lightFalloff
		// diffuse * lightProject
		// specular * ( bump dot halfAngle extended ) * lightProject
		RB_NV20_DI_BumpAndLightPass( din, false );
		RB_NV20_DI_DiffuseColorPass( din );
		RB_NV20_DI_SpecularColorPass( din );
	}
}


/*
=============
RB_NV20_CreateDrawInteractions

=============
*/
static void RB_NV20_CreateDrawInteractions( const drawSurf_t *surf ) {
	if ( !surf ) {
		return;
	}

	qglEnable( GL_VERTEX_PROGRAM_ARB );
	qglEnable( GL_REGISTER_COMBINERS_NV );

#ifdef MACOS_X
	GL_SelectTexture(0);
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
#else
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );

	qglEnableVertexAttribArrayARB( 8 );
	qglEnableVertexAttribArrayARB( 9 );
	qglEnableVertexAttribArrayARB( 10 );
	qglEnableVertexAttribArrayARB( 11 );
#endif

	for ( ; surf ; surf=surf->nextOnLight ) {
		// set the vertex pointers
		idDrawVert	*ac = (idDrawVert *)vertexCache.Position( surf->geo->ambientCache );
		qglColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), ac->color );
#ifdef MACOS_X
		GL_SelectTexture( 0 );
		qglTexCoordPointer( 2, GL_FLOAT, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
		GL_SelectTexture( 1 );
		qglTexCoordPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
		GL_SelectTexture( 2 );
		qglTexCoordPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
		GL_SelectTexture( 3 );
		qglTexCoordPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
		GL_SelectTexture( 0 );
#else
		qglVertexAttribPointerARB( 11, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->normal.ToFloatPtr() );
		qglVertexAttribPointerARB( 10, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[1].ToFloatPtr() );
		qglVertexAttribPointerARB( 9, 3, GL_FLOAT, false, sizeof( idDrawVert ), ac->tangents[0].ToFloatPtr() );
		qglVertexAttribPointerARB( 8, 2, GL_FLOAT, false, sizeof( idDrawVert ), ac->st.ToFloatPtr() );
#endif
		qglVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );

		RB_CreateSingleDrawInteractions( surf, RB_NV20_DrawInteraction );
	}

#ifndef MACOS_X
	qglDisableVertexAttribArrayARB( 8 );
	qglDisableVertexAttribArrayARB( 9 );
	qglDisableVertexAttribArrayARB( 10 );
	qglDisableVertexAttribArrayARB( 11 );
#endif

	// disable features
#ifdef MACOS_X
	GL_SelectTexture( 3 );
	globalImages->BindNull();
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
	GL_SelectTexture( 2 );
	globalImages->BindNull();
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	
	GL_SelectTexture( 1 );
	globalImages->BindNull();
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
#else	
	GL_SelectTextureNoClient( 3 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 2 );
	globalImages->BindNull();

	GL_SelectTextureNoClient( 1 );
	globalImages->BindNull();
#endif

	backEnd.glState.currenttmu = -1;
	GL_SelectTexture( 0 );

	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );

	qglDisable( GL_VERTEX_PROGRAM_ARB );
	qglDisable( GL_REGISTER_COMBINERS_NV );
}


//======================================================================================


/*
==================
RB_NV20_DrawInteractions
==================
*/
void RB_NV20_DrawInteractions( void ) {
	viewLight_t		*vLight;

	//
	// for each light, perform adding and shadowing
	//
	for ( vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next ) {
		// do fogging later
		if ( vLight->lightShader->IsFogLight() ) {
			continue;
		}
		if ( vLight->lightShader->IsBlendLight() ) {
			continue;
		}
		if ( !vLight->localInteractions && !vLight->globalInteractions
			&& !vLight->translucentInteractions ) {
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

		backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;

		if ( r_useShadowVertexProgram.GetBool() ) {
			qglEnable( GL_VERTEX_PROGRAM_ARB );
			qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
			RB_StencilShadowPass( vLight->globalShadows );
			RB_NV20_CreateDrawInteractions( vLight->localInteractions );
			qglEnable( GL_VERTEX_PROGRAM_ARB );
			qglBindProgramARB( GL_VERTEX_PROGRAM_ARB, VPROG_STENCIL_SHADOW );
			RB_StencilShadowPass( vLight->localShadows );
			RB_NV20_CreateDrawInteractions( vLight->globalInteractions );
			qglDisable( GL_VERTEX_PROGRAM_ARB );	// if there weren't any globalInteractions, it would have stayed on
		} else {
			RB_StencilShadowPass( vLight->globalShadows );
			RB_NV20_CreateDrawInteractions( vLight->localInteractions );
			RB_StencilShadowPass( vLight->localShadows );
			RB_NV20_CreateDrawInteractions( vLight->globalInteractions );
		}

		// translucent surfaces never get stencil shadowed
		if ( r_skipTranslucent.GetBool() ) {
			continue;
		}

		qglStencilFunc( GL_ALWAYS, 128, 255 );

		backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
		RB_NV20_CreateDrawInteractions( vLight->translucentInteractions );

		backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;
	}
}

//=======================================================================

/*
==================
R_NV20_Init

==================
*/
void R_NV20_Init( void ) {
	glConfig.allowNV20Path = false;

	common->Printf( "---------- R_NV20_Init ----------\n" );

	if ( !glConfig.registerCombinersAvailable || !glConfig.ARBVertexProgramAvailable || glConfig.maxTextureUnits < 4 ) {
		common->Printf( "Not available.\n" );
		return;
	}

	GL_CheckErrors();

	// create our "fragment program" display lists
	fragmentDisplayListBase = qglGenLists( FPROG_NUM_FRAGMENT_PROGRAMS );

	// force them to issue commands to build the list
	bool temp = r_useCombinerDisplayLists.GetBool();
	r_useCombinerDisplayLists.SetBool( false );

	qglNewList( fragmentDisplayListBase + FPROG_BUMP_AND_LIGHT, GL_COMPILE );
	RB_NV20_BumpAndLightFragment();
	qglEndList();

	qglNewList( fragmentDisplayListBase + FPROG_DIFFUSE_COLOR, GL_COMPILE );
	RB_NV20_DiffuseColorFragment();
	qglEndList();

	qglNewList( fragmentDisplayListBase + FPROG_SPECULAR_COLOR, GL_COMPILE );
	RB_NV20_SpecularColorFragment();
	qglEndList();

	qglNewList( fragmentDisplayListBase + FPROG_DIFFUSE_AND_SPECULAR_COLOR, GL_COMPILE );
	RB_NV20_DiffuseAndSpecularColorFragment();
	qglEndList();

	r_useCombinerDisplayLists.SetBool( temp );

	common->Printf( "---------------------------------\n" );

	glConfig.allowNV20Path = true;
}


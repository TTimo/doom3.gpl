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

  with standard calls, we can't do bump mapping or vertex colors with
  shader colors

  2 texture units:

falloff
--
light cube
bump
--
light projection
diffuse


	3 texture units:

light cube
bump
--
falloff
light projection
diffuse


	5 texture units:

light cube
bump
falloff
light projection
diffuse

*/

/*
==================
RB_ARB_DrawInteraction

backEnd.vLight

backEnd.depthFunc must be equal for alpha tested surfaces to work right,
it is set to lessThan for blended transparent surfaces

==================
*/
static void RB_ARB_DrawInteraction( const drawInteraction_t *din ) {
	const drawSurf_t *surf = din->surf;
	const srfTriangles_t	*tri = din->surf->geo;

	// set the vertex arrays, which may not all be enabled on a given pass
	idDrawVert *ac = (idDrawVert *)vertexCache.Position( tri->ambientCache );
	qglVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
	GL_SelectTexture( 0 );
	qglTexCoordPointer( 2, GL_FLOAT, sizeof( idDrawVert ), (void *)&ac->st );

	//-----------------------------------------------------
	//
	// bump / falloff
	//
	//-----------------------------------------------------
	// render light falloff * bumpmap lighting

	//
	// draw light falloff to the alpha channel
	//
	GL_State( GLS_COLORMASK | GLS_DEPTHMASK | backEnd.depthFunc );

	qglColor3f( 1, 1, 1 );
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	qglEnable( GL_TEXTURE_GEN_S );
	qglTexGenfv( GL_S, GL_OBJECT_PLANE, din->lightProjection[3].ToFloatPtr() );
	qglTexCoord2f( 0, 0.5 );

// ATI R100 can't do partial texgens
#define	NO_MIXED_TEXGEN

#ifdef NO_MIXED_TEXGEN
idVec4	plane;
plane[0] = 0;
plane[1] = 0;
plane[2] = 0;
plane[3] = 0.5;
qglEnable( GL_TEXTURE_GEN_T );
qglTexGenfv( GL_T, GL_OBJECT_PLANE, plane.ToFloatPtr() );

plane[0] = 0;
plane[1] = 0;
plane[2] = 0;
plane[3] = 1;
qglEnable( GL_TEXTURE_GEN_Q );
qglTexGenfv( GL_Q, GL_OBJECT_PLANE, plane.ToFloatPtr() );

#endif

	din->lightFalloffImage->Bind();

	// draw it
	RB_DrawElementsWithCounters( tri );

	qglDisable( GL_TEXTURE_GEN_S );
#ifdef NO_MIXED_TEXGEN
qglDisable( GL_TEXTURE_GEN_T );
qglDisable( GL_TEXTURE_GEN_Q );
#endif

#if 0
GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_DEPTHMASK 
			| backEnd.depthFunc );
// the texccords are the non-normalized vector towards the light origin
GL_SelectTexture( 0 );
globalImages->normalCubeMapImage->Bind();
qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
qglTexCoordPointer( 3, GL_FLOAT, sizeof( lightingCache_t ), ((lightingCache_t *)vertexCache.Position(tri->lightingCache))->localLightVector.ToFloatPtr() );
// draw it
RB_DrawElementsWithCounters( tri );
return;
#endif

	// we can't do bump mapping with standard calls, so skip it
	if ( glConfig.envDot3Available && glConfig.cubeMapAvailable ) {
		//
		// draw the bump map result onto the alpha channel
		//
		GL_State( GLS_SRCBLEND_DST_ALPHA | GLS_DSTBLEND_ZERO | GLS_COLORMASK | GLS_DEPTHMASK 
			| backEnd.depthFunc );

		// texture 0 will be the per-surface bump map
		GL_SelectTexture( 0 );
		qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
//	FIXME: matrix work!	RB_BindStageTexture( surfaceRegs, &surfaceStage->texture, surf );
		din->bumpImage->Bind();

		// texture 1 is the normalization cube map
		// the texccords are the non-normalized vector towards the light origin
		GL_SelectTexture( 1 );
		if ( din->ambientLight ) {
			globalImages->ambientNormalMap->Bind();	// fixed value
		} else {
			globalImages->normalCubeMapImage->Bind();
		}
		qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
		qglTexCoordPointer( 3, GL_FLOAT, sizeof( lightingCache_t ), ((lightingCache_t *)vertexCache.Position(tri->lightingCache))->localLightVector.ToFloatPtr() );

		// I just want alpha = Dot( texture0, texture1 )
		GL_TexEnv( GL_COMBINE_ARB );

		qglTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGBA_ARB );
		qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
		qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB );
		qglTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR );
		qglTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR );
		qglTexEnvi( GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1 );
		qglTexEnvi( GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1 );

		// draw it
		RB_DrawElementsWithCounters( tri );

		GL_TexEnv( GL_MODULATE );

		globalImages->BindNull();
		qglDisableClientState( GL_TEXTURE_COORD_ARRAY );

		GL_SelectTexture( 0 );
//		RB_FinishStageTexture( &surfaceStage->texture, surf );
	}

	//-----------------------------------------------------
	//
	// projected light / surface color for diffuse maps
	//
	//-----------------------------------------------------
	// don't trash alpha
	GL_State( GLS_SRCBLEND_DST_ALPHA | GLS_DSTBLEND_ONE | GLS_ALPHAMASK | GLS_DEPTHMASK 
	| backEnd.depthFunc );

	// texture 0 will get the surface color texture
	GL_SelectTexture( 0 );

	// select the vertex color source
	if ( din->vertexColor == SVC_IGNORE ) {
		qglColor4fv( din->diffuseColor.ToFloatPtr() );
	} else {
		// FIXME: does this not get diffuseColor blended in?
		qglColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), (void *)&ac->color );
		qglEnableClientState( GL_COLOR_ARRAY );

		if ( din->vertexColor == SVC_INVERSE_MODULATE ) {
			GL_TexEnv( GL_COMBINE_ARB );
			qglTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE );
			qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
			qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB );
			qglTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR );
			qglTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_ONE_MINUS_SRC_COLOR );
			qglTexEnvi( GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1 );
		}
	}

	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
	// FIXME: does this not get the texture matrix?
//	RB_BindStageTexture( surfaceRegs, &surfaceStage->texture, surf );
	din->diffuseImage->Bind();

	// texture 1 will get the light projected texture
	GL_SelectTexture( 1 );
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	qglEnable( GL_TEXTURE_GEN_S );
	qglEnable( GL_TEXTURE_GEN_T );
	qglEnable( GL_TEXTURE_GEN_Q );
	qglTexGenfv( GL_S, GL_OBJECT_PLANE, din->lightProjection[0].ToFloatPtr() );
	qglTexGenfv( GL_T, GL_OBJECT_PLANE, din->lightProjection[1].ToFloatPtr() );
	qglTexGenfv( GL_Q, GL_OBJECT_PLANE, din->lightProjection[2].ToFloatPtr() );

	din->lightImage->Bind();

	// draw it
	RB_DrawElementsWithCounters( tri );

	qglDisable( GL_TEXTURE_GEN_S );
	qglDisable( GL_TEXTURE_GEN_T );
	qglDisable( GL_TEXTURE_GEN_Q );

	globalImages->BindNull();
	GL_SelectTexture( 0 );

	if ( din->vertexColor != SVC_IGNORE ) {
		qglDisableClientState( GL_COLOR_ARRAY );
		GL_TexEnv( GL_MODULATE );
	}

//	RB_FinishStageTexture( &surfaceStage->texture, surf );
}

/*
==================
RB_ARB_DrawThreeTextureInteraction

Used by radeon R100 and Intel graphics parts

backEnd.vLight

backEnd.depthFunc must be equal for alpha tested surfaces to work right,
it is set to lessThan for blended transparent surfaces

==================
*/
static void RB_ARB_DrawThreeTextureInteraction( const drawInteraction_t *din ) {
	const drawSurf_t *surf = din->surf;
	const srfTriangles_t	*tri = din->surf->geo;

	// set the vertex arrays, which may not all be enabled on a given pass
	idDrawVert *ac = (idDrawVert *)vertexCache.Position( tri->ambientCache );
	qglVertexPointer( 3, GL_FLOAT, sizeof( idDrawVert ), ac->xyz.ToFloatPtr() );
	GL_SelectTexture( 0 );
	qglTexCoordPointer( 2, GL_FLOAT, sizeof( idDrawVert ), (void *)&ac->st );
	qglColor3f( 1, 1, 1 );

	//
	// bump map dot cubeMap into the alpha channel
	//
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO | GLS_COLORMASK | GLS_DEPTHMASK 
		| backEnd.depthFunc );

	// texture 0 will be the per-surface bump map
	GL_SelectTexture( 0 );
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
//	FIXME: matrix work!	RB_BindStageTexture( surfaceRegs, &surfaceStage->texture, surf );
	din->bumpImage->Bind();

	// texture 1 is the normalization cube map
	// the texccords are the non-normalized vector towards the light origin
	GL_SelectTexture( 1 );
	if ( din->ambientLight ) {
		globalImages->ambientNormalMap->Bind();	// fixed value
	} else {
		globalImages->normalCubeMapImage->Bind();
	}
	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
	qglTexCoordPointer( 3, GL_FLOAT, sizeof( lightingCache_t ), ((lightingCache_t *)vertexCache.Position(tri->lightingCache))->localLightVector.ToFloatPtr() );

	// I just want alpha = Dot( texture0, texture1 )
	GL_TexEnv( GL_COMBINE_ARB );

	qglTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGBA_ARB );
	qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
	qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB );
	qglTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR );
	qglTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR );
	qglTexEnvi( GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1 );
	qglTexEnvi( GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1 );

	// draw it
	RB_DrawElementsWithCounters( tri );

	GL_TexEnv( GL_MODULATE );

	globalImages->BindNull();
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );

	GL_SelectTexture( 0 );
//		RB_FinishStageTexture( &surfaceStage->texture, surf );


	//-----------------------------------------------------
	//
	// light falloff / projected light / surface color for diffuse maps
	//
	//-----------------------------------------------------
	// multiply result by alpha, but don't trash alpha
	GL_State( GLS_SRCBLEND_DST_ALPHA | GLS_DSTBLEND_ONE | GLS_ALPHAMASK | GLS_DEPTHMASK 
	| backEnd.depthFunc );

	// texture 0 will get the surface color texture
	GL_SelectTexture( 0 );

	// select the vertex color source
	if ( din->vertexColor == SVC_IGNORE ) {
		qglColor4fv( din->diffuseColor.ToFloatPtr() );
	} else {
		// FIXME: does this not get diffuseColor blended in?
		qglColorPointer( 4, GL_UNSIGNED_BYTE, sizeof( idDrawVert ), (void *)&ac->color );
		qglEnableClientState( GL_COLOR_ARRAY );

		if ( din->vertexColor == SVC_INVERSE_MODULATE ) {
			GL_TexEnv( GL_COMBINE_ARB );
			qglTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE );
			qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
			qglTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB );
			qglTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR );
			qglTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_ONE_MINUS_SRC_COLOR );
			qglTexEnvi( GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1 );
		}
	}

	qglEnableClientState( GL_TEXTURE_COORD_ARRAY );
	// FIXME: does this not get the texture matrix?
//	RB_BindStageTexture( surfaceRegs, &surfaceStage->texture, surf );
	din->diffuseImage->Bind();

	// texture 1 will get the light projected texture
	GL_SelectTexture( 1 );
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	qglEnable( GL_TEXTURE_GEN_S );
	qglEnable( GL_TEXTURE_GEN_T );
	qglEnable( GL_TEXTURE_GEN_Q );
	qglTexGenfv( GL_S, GL_OBJECT_PLANE, din->lightProjection[0].ToFloatPtr() );
	qglTexGenfv( GL_T, GL_OBJECT_PLANE, din->lightProjection[1].ToFloatPtr() );
	qglTexGenfv( GL_Q, GL_OBJECT_PLANE, din->lightProjection[2].ToFloatPtr() );
	din->lightImage->Bind();

	// texture 2 will get the light falloff texture
	GL_SelectTexture( 2 );
	qglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	qglEnable( GL_TEXTURE_GEN_S );
	qglEnable( GL_TEXTURE_GEN_T );
	qglEnable( GL_TEXTURE_GEN_Q );

	qglTexGenfv( GL_S, GL_OBJECT_PLANE, din->lightProjection[3].ToFloatPtr() );

	idVec4	plane;
	plane[0] = 0;
	plane[1] = 0;
	plane[2] = 0;
	plane[3] = 0.5;
	qglTexGenfv( GL_T, GL_OBJECT_PLANE, plane.ToFloatPtr() );

	plane[0] = 0;
	plane[1] = 0;
	plane[2] = 0;
	plane[3] = 1;
	qglTexGenfv( GL_Q, GL_OBJECT_PLANE, plane.ToFloatPtr() );

	din->lightFalloffImage->Bind();

	// draw it
	RB_DrawElementsWithCounters( tri );

	qglDisable( GL_TEXTURE_GEN_S );
	qglDisable( GL_TEXTURE_GEN_T );
	qglDisable( GL_TEXTURE_GEN_Q );
	globalImages->BindNull();

	GL_SelectTexture( 1 );
	qglDisable( GL_TEXTURE_GEN_S );
	qglDisable( GL_TEXTURE_GEN_T );
	qglDisable( GL_TEXTURE_GEN_Q );
	globalImages->BindNull();

	GL_SelectTexture( 0 );

	if ( din->vertexColor != SVC_IGNORE ) {
		qglDisableClientState( GL_COLOR_ARRAY );
		GL_TexEnv( GL_MODULATE );
	}

//	RB_FinishStageTexture( &surfaceStage->texture, surf );
}


/*
==================
RB_CreateDrawInteractions
==================
*/
static void RB_CreateDrawInteractions( const drawSurf_t *surf ) {
	if ( !surf ) {
		return;
	}

	// force a space calculation
	backEnd.currentSpace = NULL;

	if ( r_useTripleTextureARB.GetBool() && glConfig.maxTextureUnits >= 3 ) {
		for ( ; surf ; surf = surf->nextOnLight ) {
			// break it up into multiple primitive draw interactions if necessary
			RB_CreateSingleDrawInteractions( surf, RB_ARB_DrawThreeTextureInteraction );
		}
	} else {
		for ( ; surf ; surf = surf->nextOnLight ) {
			// break it up into multiple primitive draw interactions if necessary
			RB_CreateSingleDrawInteractions( surf, RB_ARB_DrawInteraction );
		}
	}
}



/*
==================
RB_RenderViewLight

==================
*/
static void RB_RenderViewLight( viewLight_t *vLight ) {
	backEnd.vLight = vLight;

	// do fogging later
	if ( vLight->lightShader->IsFogLight() ) {
		return;
	}
	if ( vLight->lightShader->IsBlendLight() ) {
		return;
	}

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
	RB_StencilShadowPass( vLight->globalShadows );
	RB_CreateDrawInteractions( vLight->localInteractions );
	RB_StencilShadowPass( vLight->localShadows );
	RB_CreateDrawInteractions( vLight->globalInteractions );

	if ( r_skipTranslucent.GetBool() ) {
		return;
	}

	// disable stencil testing for translucent interactions, because
	// the shadow isn't calculated at their point, and the shadow
	// behind them may be depth fighting with a back side, so there
	// isn't any reasonable thing to do
	qglStencilFunc( GL_ALWAYS, 128, 255 );
	backEnd.depthFunc = GLS_DEPTHFUNC_LESS;
	RB_CreateDrawInteractions( vLight->translucentInteractions );
}


/*
==================
RB_ARB_DrawInteractions
==================
*/
void RB_ARB_DrawInteractions( void ) {
	qglEnable( GL_STENCIL_TEST );

	for ( viewLight_t *vLight = backEnd.viewDef->viewLights ; vLight ; vLight = vLight->next ) {
		RB_RenderViewLight( vLight );
	}
}


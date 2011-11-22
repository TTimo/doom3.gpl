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

#ifndef __MODELOVERLAY_H__
#define __MODELOVERLAY_H__

/*
===============================================================================

	Render model overlay for adding decals on top of dynamic models.

===============================================================================
*/

const int MAX_OVERLAY_SURFACES	= 16;

typedef struct overlayVertex_s {
	int							vertexNum;
	float						st[2];
} overlayVertex_t;

typedef struct overlaySurface_s {
	int							surfaceNum;
	int							surfaceId;
	int							numIndexes;
	glIndex_t *					indexes;
	int							numVerts;
	overlayVertex_t *			verts;
} overlaySurface_t;

typedef struct overlayMaterial_s {
	const idMaterial *			material;
	idList<overlaySurface_t *>	surfaces;
} overlayMaterial_t;


class idRenderModelOverlay {
public:
								idRenderModelOverlay();
								~idRenderModelOverlay();

	static idRenderModelOverlay *Alloc( void );
	static void					Free( idRenderModelOverlay *overlay );

	// Projects an overlay onto deformable geometry and can be added to
	// a render entity to allow decals on top of dynamic models.
	// This does not generate tangent vectors, so it can't be used with
	// light interaction shaders. Materials for overlays should always
	// be clamped, because the projected texcoords can run well off the
	// texture since no new clip vertexes are generated.
	void						CreateOverlay( const idRenderModel *model, const idPlane localTextureAxis[2], const idMaterial *material );

	// Creates new model surfaces for baseModel, which should be a static instantiation of a dynamic model.
	void						AddOverlaySurfacesToModel( idRenderModel *baseModel );

	// Removes overlay surfaces from the model.
	static void					RemoveOverlaySurfacesFromModel( idRenderModel *baseModel );

	void						ReadFromDemoFile( class idDemoFile *f );
	void						WriteToDemoFile( class idDemoFile *f ) const;

private:
	idList<overlayMaterial_t *>	materials;

	void						FreeSurface( overlaySurface_t *surface );
};

#endif /* !__MODELOVERLAY_H__ */

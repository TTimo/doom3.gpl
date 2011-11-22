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

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "qe3.h"

const float PLANE_EPSILON = 0.0001f;

/*
=============
CSG_MakeHollow
=============
*/
void CSG_MakeHollow (void)
{
	brush_t		*b, *front, *back, *next;
	face_t		*f;
	face_t		split;
	idVec3		move;
	int			i;

	for (b = selected_brushes.next ; b != &selected_brushes ; b=next)
	{
		next = b->next;

    if (b->owner->eclass->fixedsize || b->pPatch || b->hiddenBrush || b->modelHandle > 0)
		  continue;

		for ( f = b->brush_faces; f; f = f->next ) {
			split = *f;
			VectorScale (f->plane, g_qeglobals.d_gridsize, move);
			for (i=0 ; i<3 ; i++)
				VectorSubtract (split.planepts[i], move, split.planepts[i]);

			Brush_SplitBrushByFace (b, &split, &front, &back);
			if (back)
				Brush_Free (back);
			if (front)
				Brush_AddToList (front, &selected_brushes);
		}
		Brush_Free (b);
	}
	Sys_UpdateWindows (W_ALL);
}

/*
=============
Brush_Merge

 Returns a new brush that is created by merging brush1 and brush2.
 May return NULL if brush1 and brush2 do not create a convex brush when merged.
 The input brushes brush1 and brush2 stay intact.

 if onlyshape is true then the merge is allowed based on the shape only
 otherwise the texture/shader references of faces in the same plane have to
 be the same as well.
=============
*/
brush_t *Brush_Merge(brush_t *brush1, brush_t *brush2, int onlyshape)
{
	int i, shared;
	brush_t *newbrush;
	face_t *face1, *face2, *newface, *f;

	// check for bounding box overlapp
	for (i = 0; i < 3; i++)
	{
		if (brush1->mins[i] > brush2->maxs[i] + ON_EPSILON
				|| brush1->maxs[i] < brush2->mins[i] - ON_EPSILON)
		{
			// never merge if the brushes overlap
			return NULL;
		}
	}
	//
	shared = 0;
	// check if the new brush would be convex... flipped planes make a brush non-convex
	for (face1 = brush1->brush_faces; face1; face1 = face1->next)
	{
		// don't check the faces of brush 1 and 2 touching each other
		for (face2 = brush2->brush_faces; face2; face2 = face2->next)
		{
			if ( face1->plane.Compare( -face2->plane, PLANE_EPSILON ) )
			{
				shared++;
				// there may only be ONE shared side
				if (shared > 1)
					return NULL;
				break;
			}
		}
		// if this face plane is shared
		if (face2) continue;
		//
		for (face2 = brush2->brush_faces; face2; face2 = face2->next)
		{
			// don't check the faces of brush 1 and 2 touching each other
			for ( f = brush1->brush_faces; f; f = f->next ) {
				if ( face2->plane.Compare( -f->plane, PLANE_EPSILON ) ) {
					break;
				}
			}
			if ( f ) {
				continue;
			}

			if ( face1->plane.Compare( face2->plane, PLANE_EPSILON ) )
			{
				//if the texture/shader references should be the same but are not
				if (!onlyshape && stricmp(face1->texdef.name, face2->texdef.name) != 0)
					return NULL;
				continue;
			}
			//
			if ( face1->face_winding->PlanesConcave( *face2->face_winding,
							face1->plane.Normal(), face2->plane.Normal(), -face1->plane[3], -face2->plane[3]))
			{
				return NULL;
			} //end if
		} //end for
	} //end for
	//
	newbrush = Brush_Alloc();
	//
	for (face1 = brush1->brush_faces; face1; face1 = face1->next)
	{
		// don't add the faces of brush 1 and 2 touching each other
		for (face2 = brush2->brush_faces; face2; face2 = face2->next)
		{
			if ( face1->plane.Compare( -face2->plane, PLANE_EPSILON ) ) {
				break;
			}
		}
		if ( face2 )
			continue;
		// don't add faces with the same plane twice
		for (f = newbrush->brush_faces; f; f = f->next)
		{
			if ( face1->plane.Compare( f->plane, PLANE_EPSILON ) )
				break;
			if ( face1->plane.Compare( -f->plane, PLANE_EPSILON ) )
				break;
		}
		if ( f ) {
			continue;
		}

		newface = Face_Alloc();
		newface->texdef = face1->texdef;
		VectorCopy(face1->planepts[0], newface->planepts[0]);
		VectorCopy(face1->planepts[1], newface->planepts[1]);
		VectorCopy(face1->planepts[2], newface->planepts[2]);
		newface->plane = face1->plane;
		newface->next = newbrush->brush_faces;
		newbrush->brush_faces = newface;
	}

	for (face2 = brush2->brush_faces; face2; face2 = face2->next) {
		// don't add the faces of brush 1 and 2 touching each other
		for (face1 = brush1->brush_faces; face1; face1 = face1->next)
		{
			if ( face2->plane.Compare( -face1->plane, PLANE_EPSILON ) ) {
				break;
			}
		}
		if (face1)
			continue;
		// don't add faces with the same plane twice
		for (f = newbrush->brush_faces; f; f = f->next)
		{
			if ( face2->plane.Compare( f->plane, PLANE_EPSILON ) )
				break;
			if ( face2->plane.Compare( -f->plane, PLANE_EPSILON ) )
				break;
		}
		if ( f ) {
			continue;
		}
		//
		newface = Face_Alloc();
		newface->texdef = face2->texdef;
		VectorCopy(face2->planepts[0], newface->planepts[0]);
		VectorCopy(face2->planepts[1], newface->planepts[1]);
		VectorCopy(face2->planepts[2], newface->planepts[2]);
		newface->plane = face2->plane;
		newface->next = newbrush->brush_faces;
		newbrush->brush_faces = newface;
	}
	// link the new brush to an entity
	Entity_LinkBrush (brush1->owner, newbrush);
	// build windings for the faces
	Brush_BuildWindings( newbrush, false);
	return newbrush;
}

/*
=============
Brush_MergeListPairs

  Returns a list with merged brushes.
  Tries to merge brushes pair wise.
  The input list is destroyed.
  Input and output should be a single linked list using .next
=============
*/
brush_t *Brush_MergeListPairs(brush_t *brushlist, int onlyshape)
{
	int nummerges, merged;
	brush_t *b1, *b2, *tail, *newbrush, *newbrushlist;
	brush_t *lastb2;

	if (!brushlist) return NULL;

	nummerges = 0;
	do
	{
		for (tail = brushlist; tail; tail = tail->next)
		{
			if (!tail->next) break;
		}
		merged = 0;
		newbrushlist = NULL;
		for (b1 = brushlist; b1; b1 = brushlist)
		{
			lastb2 = b1;
			for (b2 = b1->next; b2; b2 = b2->next)
			{
				newbrush = Brush_Merge(b1, b2, onlyshape);
				if (newbrush)
				{
					tail->next = newbrush;
					lastb2->next = b2->next;
					brushlist = brushlist->next;
					b1->next = b1->prev = NULL;
					b2->next = b2->prev = NULL;
					Brush_Free(b1);
					Brush_Free(b2);
					for (tail = brushlist; tail; tail = tail->next)
					{
						if (!tail->next) break;
					} //end for
					merged++;
					nummerges++;
					break;
				}
				lastb2 = b2;
			}
			//if b1 can't be merged with any of the other brushes
			if (!b2)
			{
				brushlist = brushlist->next;
				//keep b1
				b1->next = newbrushlist;
				newbrushlist = b1;
			}
		}
		brushlist = newbrushlist;
	} while(merged);
	return newbrushlist;
}

/*
=============
Brush_MergeList

 Tries to merge all brushes in the list into one new brush.
 The input brush list stays intact.
 Returns NULL if no merged brush can be created.
 To create a new brush the brushes in the list may not overlap and
 the outer faces of the brushes together should make a new convex brush.

 if onlyshape is true then the merge is allowed based on the shape only
 otherwise the texture/shader references of faces in the same plane have to
 be the same as well.
=============
*/
brush_t *Brush_MergeList(brush_t *brushlist, int onlyshape)
{
	brush_t *brush1, *brush2, *brush3, *newbrush;
	face_t *face1, *face2, *face3, *newface, *f;

	if (!brushlist) return NULL;
	for (brush1 = brushlist; brush1; brush1 = brush1->next)
	{
		// check if the new brush would be convex... flipped planes make a brush concave
		for (face1 = brush1->brush_faces; face1; face1 = face1->next)
		{
			// don't check face1 if it touches another brush
			for (brush2 = brushlist; brush2; brush2 = brush2->next)
			{
				if (brush2 == brush1) continue;
				for (face2 = brush2->brush_faces; face2; face2 = face2->next)
				{
					if ( face1->plane.Compare( -face2->plane, PLANE_EPSILON ) ) {
						break;
					}
				}
				if (face2)
					break;
			}
			// if face1 touches another brush
			if (brush2)
				continue;
			//
			for (brush2 = brush1->next; brush2; brush2 = brush2->next)
			{
				// don't check the faces of brush 2 touching another brush
				for (face2 = brush2->brush_faces; face2; face2 = face2->next)
				{
					for (brush3 = brushlist; brush3; brush3 = brush3->next)
					{
						if (brush3 == brush2) continue;
						for (face3 = brush3->brush_faces; face3; face3 = face3->next)
						{
							if ( face2->plane.Compare( -face3->plane, PLANE_EPSILON ) )
								break;
						}
						if (face3)
							break;
					}
					// if face2 touches another brush
					if (brush3)
						continue;
					//
					if ( face1->plane.Compare( face2->plane, PLANE_EPSILON ) )
					{
						//if the texture/shader references should be the same but are not
						if (!onlyshape && stricmp(face1->texdef.name, face2->texdef.name) != 0)
							return NULL;
						continue;
					}
					//
					if ( face1->face_winding->PlanesConcave( *face2->face_winding,
							face1->plane.Normal(), face2->plane.Normal(), -face1->plane[3], -face2->plane[3]))
					{
						return NULL;
					}
				}
			}
		}
	}
	//
	newbrush = Brush_Alloc();
	//
	for (brush1 = brushlist; brush1; brush1 = brush1->next)
	{
		for (face1 = brush1->brush_faces; face1; face1 = face1->next)
		{
			// don't add face1 to the new brush if it touches another brush
			for (brush2 = brushlist; brush2; brush2 = brush2->next)
			{
				if (brush2 == brush1) continue;
				for (face2 = brush2->brush_faces; face2; face2 = face2->next)
				{
					if ( face1->plane.Compare( -face2->plane, PLANE_EPSILON ) ) {
						break;
					}
				}
				if (face2)
					break;
			}
			if (brush2)
				continue;
			// don't add faces with the same plane twice
			for (f = newbrush->brush_faces; f; f = f->next)
			{
				if ( face1->plane.Compare( f->plane, PLANE_EPSILON ) )
					break;
				if ( face1->plane.Compare( -f->plane, PLANE_EPSILON ) )
					break;
			}
			if (f)
				continue;
			//
			newface = Face_Alloc();
			newface->texdef = face1->texdef;
			VectorCopy(face1->planepts[0], newface->planepts[0]);
			VectorCopy(face1->planepts[1], newface->planepts[1]);
			VectorCopy(face1->planepts[2], newface->planepts[2]);
			newface->plane = face1->plane;
			newface->next = newbrush->brush_faces;
			newbrush->brush_faces = newface;
		}
	}
	// link the new brush to an entity
	Entity_LinkBrush (brushlist->owner, newbrush);
	// build windings for the faces
	Brush_BuildWindings( newbrush, false);
	return newbrush;
}

/*
=============
Brush_Subtract

 Returns a list of brushes that remain after B is subtracted from A.
 May by empty if A is contained inside B.
 The originals are undisturbed.
=============
*/
brush_t *Brush_Subtract(brush_t *a, brush_t *b)
{
	// a - b = out (list)
	brush_t *front, *back;
	brush_t *in, *out, *next;
	face_t *f;

	in = a;
	out = NULL;
	for (f = b->brush_faces; f && in; f = f->next)
	{
		Brush_SplitBrushByFace(in, f, &front, &back);
		if (in != a) Brush_Free(in);
		if (front)
		{	// add to list
			front->next = out;
			out = front;
		}
		in = back;
	}
	//NOTE: in != a just in case brush b has no faces
	if (in && in != a)
	{
		Brush_Free(in);
	}
	else
	{	//didn't really intersect
		for (b = out; b; b = next)
		{
			next = b->next;
			b->next = b->prev = NULL;
			Brush_Free(b);
		}
		return a;
	}
	return out;
}

/*
=============
CSG_Subtract
=============
*/
void CSG_Subtract (void)
{
	brush_t		*b, *s, *fragments, *nextfragment, *frag, *next, *snext;
	brush_t		fragmentlist;
	int			i, numfragments, numbrushes;

	Sys_Status ("Subtracting...\n");

	if (selected_brushes.next == &selected_brushes)
	{
		Sys_Status("No brushes selected.\n");
		return;
	}

	fragmentlist.next = &fragmentlist;
	fragmentlist.prev = &fragmentlist;

	numfragments = 0;
	numbrushes = 0;
	for (b = selected_brushes.next ; b != &selected_brushes ; b=next)
	{
		next = b->next;

		if (b->owner->eclass->fixedsize || b->modelHandle > 0)
			continue;	// can't use texture from a fixed entity, so don't subtract

		// chop all fragments further up
		for (s = fragmentlist.next; s != &fragmentlist; s = snext)
		{
			snext = s->next;

			for (i=0 ; i<3 ; i++)
				if (b->mins[i] >= s->maxs[i] - ON_EPSILON 
				|| b->maxs[i] <= s->mins[i] + ON_EPSILON)
					break;
			if (i != 3)
				continue;	// definately don't touch
			fragments = Brush_Subtract(s, b);
			// if the brushes did not really intersect
			if (fragments == s)
				continue;
			// try to merge fragments
			fragments = Brush_MergeListPairs(fragments, true);
			// add the fragments to the list
			for (frag = fragments; frag; frag = nextfragment)
			{
				nextfragment = frag->next;
				frag->next = NULL;
				frag->owner = s->owner;
				Brush_AddToList(frag, &fragmentlist);
			}
			// free the original brush
			Brush_Free(s);
		}

		// chop any active brushes up
		for (s = active_brushes.next; s != &active_brushes; s = snext)
		{
			snext = s->next;

			if (s->owner->eclass->fixedsize || s->pPatch || s->hiddenBrush || s->modelHandle > 0)
				continue;

			//face_t *pFace = s->brush_faces;
			if ( s->brush_faces->d_texture && ( s->brush_faces->d_texture->GetContentFlags()& CONTENTS_NOCSG ) )
			{
				continue;
			}

			for (i=0 ; i<3 ; i++)
				if (b->mins[i] >= s->maxs[i] - ON_EPSILON 
				|| b->maxs[i] <= s->mins[i] + ON_EPSILON)
					break;
			if (i != 3)
				continue;	// definately don't touch

			fragments = Brush_Subtract(s, b);
			// if the brushes did not really intersect
			if (fragments == s)
				continue;
			//
			Undo_AddBrush(s);
			// one extra brush chopped up
			numbrushes++;
			// try to merge fragments
			fragments = Brush_MergeListPairs(fragments, true);
			// add the fragments to the list
			for (frag = fragments; frag; frag = nextfragment)
			{
				nextfragment = frag->next;
				frag->next = NULL;
				frag->owner = s->owner;
				Brush_AddToList(frag, &fragmentlist);
			}
			// free the original brush
			Brush_Free(s);
		}
	}

	// move all fragments to the active brush list
	for (frag = fragmentlist.next; frag != &fragmentlist; frag = nextfragment)
	{
		nextfragment = frag->next;
		numfragments++;
		Brush_RemoveFromList(frag);
		Brush_AddToList(frag, &active_brushes);
		Undo_EndBrush(frag);
	}

	if (numfragments == 0)
	{
		common->Printf("Selected brush%s did not intersect with any other brushes.\n",
					(selected_brushes.next->next == &selected_brushes) ? "":"es");
		return;
	}
	Sys_Status("done.");
	common->Printf(" (created %d fragment%s out of %d brush%s)\n", numfragments, (numfragments == 1)?"":"s",
							numbrushes, (numbrushes == 1)?"":"es");
	Sys_UpdateWindows(W_ALL);
}

/*
=============
CSG_Merge
=============
*/
void CSG_Merge(void)
{
	brush_t *b, *next, *newlist, *newbrush;
	struct entity_s	*owner;

	Sys_Status("Merging...\n");

	if (selected_brushes.next == &selected_brushes)
	{
		Sys_Status("No brushes selected.\n");
		return;
	}

	if (selected_brushes.next->next == &selected_brushes)
	{
		Sys_Status("At least two brushes have to be selected.\n");
		return;
	}

	owner = selected_brushes.next->owner;

	for (b = selected_brushes.next; b != &selected_brushes; b = next)
	{
		next = b->next;

		if (b->owner->eclass->fixedsize || b->modelHandle > 0)
		{
			// can't use texture from a fixed entity, so don't subtract
			Sys_Status("Cannot add fixed size entities.\n");
			return;
		}

		if (b->pPatch)
		{
			Sys_Status("Cannot add patches.\n");
			return;
		}

		if ( b->brush_faces->d_texture && ( b->brush_faces->d_texture->GetContentFlags() & CONTENTS_NOCSG ) )
		{
			Sys_Status("Cannot add brushes using shaders that don't allows CSG operations.\n");
			return;
		}

		if (b->owner != owner)
		{
			Sys_Status("Cannot add brushes from different entities.\n");
			return;
		}

	}

	newlist = NULL;
	for (b = selected_brushes.next; b != &selected_brushes; b = next)
	{
		next = b->next;

		Brush_RemoveFromList(b);
		b->next = newlist;
		b->prev = NULL;
		newlist = b;
	}

	newbrush = Brush_MergeList(newlist, true);
	// if the new brush would not be convex
	if (!newbrush)
	{
		// add the brushes back into the selection
		for (b = newlist; b; b = next)
		{
			next = b->next;
			b->next = NULL;
			b->prev = NULL;
			Brush_AddToList(b, &selected_brushes);
		}
		Sys_Status("Cannot add a set of brushes with a concave hull.\n");
		return;
	}
	// free the original brushes
	for (b = newlist; b; b = next)
	{
		next = b->next;
		b->next = NULL;
		b->prev = NULL;
		Brush_Free(b);
	}
	Brush_AddToList(newbrush, &selected_brushes);

	Sys_Status ("done.\n");
	Sys_UpdateWindows (W_ALL);
}

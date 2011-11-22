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
#include "Radiant.h"
#include "autocaulk.h"

// Note: the code in here looks pretty goofy in places, and probably doesn't use the new Q4 class stuff fully, 
//   but I just got it in and compiling from the JK2/SOF2 Radiants via some ugly code replaces, and it works, so there.
// Also, a bunch of Radiant fields no longer exist in this codebase, likewise the whole point of passing in the bool
//	to this code, but I've just left it as-is. A designer tested it and pronounced it fine.
 
//#pragma warning( disable : 4786)
//#include <list>
//using namespace std;
//#pragma warning( disable : 4786)

#undef strnicmp
#define strnicmp		idStr::Icmpn

#if 1


//extern void ClearBounds (idVec3 mins, idVec3 maxs);
//extern void AddPointToBounds (const idVec3 v, idVec3 mins, idVec3 maxs);
void ClearBounds (idVec3 &mins, idVec3 &maxs)
{
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void AddPointToBounds( const idVec3 &v, idVec3 &mins, idVec3 &maxs ) 
{
	int		i;
	float	val;

	for (i=0 ; i<3 ; i++)
	{
		val = v[i];
		if (val < mins[i])
			mins[i] = val;
		if (val > maxs[i])
			maxs[i] = val;
	}
}


static void FloorBounds(idVec3 &mins, idVec3 &maxs)
{
	for (int i=0 ; i<3 ; i++)
	{
		mins[i] = floor(mins[i] + 0.5);
		maxs[i] = floor(maxs[i] + 0.5);
	}
}


static LPCSTR vtos(idVec3 &v3)
{
	return va("%.3ff,%.3f,%.3f",v3[0],v3[1],v3[2]);
}
struct PairBrushFace_t
{
	face_t*		pFace;
	brush_t*	pBrush;
};
idList < PairBrushFace_t > FacesToCaulk;
void Select_AutoCaulk()
{
	/*Sys_Printf*/common->Printf("Caulking...\n");	

	FacesToCaulk.Clear();

	int iSystemBrushesSkipped = 0;
	face_t *pSelectedFace;

	brush_t *next;
	for (brush_t *pSelectedBrush = selected_brushes.next ; pSelectedBrush != &selected_brushes ; pSelectedBrush = next)
	{
		next = pSelectedBrush->next;
		
		if (pSelectedBrush->owner->eclass->fixedsize)
			continue;	// apparently this means it's a model, so skip it...

		// new check, we can't caulk a brush that has any "system/" faces...
		//
		bool bSystemFacePresent = false;
		for ( pSelectedFace = pSelectedBrush->brush_faces; pSelectedFace; pSelectedFace = pSelectedFace->next)
		{	
			if (!strnicmp(pSelectedFace->d_texture->GetName(),"system/",7))
			{
				bSystemFacePresent = true;
				break;
			}
		}
		if (bSystemFacePresent)
		{
			iSystemBrushesSkipped++;
			continue;	// verboten to caulk this.
		}		

		for (int iBrushListToScan = 0; iBrushListToScan<2; iBrushListToScan++)
		{		
			brush_t	*snext;
			for (brush_t *pScannedBrush = (iBrushListToScan?active_brushes.next:selected_brushes.next); pScannedBrush != (iBrushListToScan?&active_brushes:&selected_brushes) ; pScannedBrush = snext)
			{
				snext = pScannedBrush->next;

				if ( pScannedBrush == pSelectedBrush)
					continue;
				
				if (pScannedBrush->owner->eclass->fixedsize || pScannedBrush->pPatch || pScannedBrush->hiddenBrush)
					continue;

		  		if (FilterBrush(pScannedBrush))
					continue;

// idMaterial stuff no longer support this, not sure what else to do. 
//   Searching for other occurences of QER_NOCARVE just shows people REMing the code and ignoring ths issue...
//				
//				if (pScannedBrush->brush_faces->d_texture->bFromShader && (pScannedBrush->brush_faces->d_texture->TestMaterialFlag(QER_NOCARVE)))
//					continue;
				
				// basic-reject first to see if brushes can even possibly touch (coplanar counts as touching)
				//
				int i;
				for (i=0 ; i<3 ; i++)
				{
					if (pSelectedBrush->mins[i] > pScannedBrush->maxs[i] ||
						pSelectedBrush->maxs[i] < pScannedBrush->mins[i])
					{
						break;
					}
				}
				if (i != 3)
					continue;	// can't be touching

				// ok, now for the clever stuff, we need to detect only those faces that are both coplanar and smaller
				//	or equal to the face they're coplanar with...
				//
				for (pSelectedFace = pSelectedBrush->brush_faces; pSelectedFace; pSelectedFace = pSelectedFace->next)
				{
					idWinding *pSelectedWinding = pSelectedFace->face_winding;

					if (!pSelectedWinding)
						continue;	// freed face, probably won't happen here, but who knows with this program?

	//				SquaredFace_t SelectedSquaredFace;
	//				WindingToSquaredFace( &SelectedSquaredFace, pSelectedWinding);

					for (face_t *pScannedFace = pScannedBrush->brush_faces; pScannedFace; pScannedFace = pScannedFace->next)
					{
						// don't even try caulking against a system face, because these are often transparent and will leave holes
						//
						if (!strnicmp(pScannedFace->d_texture->GetName(),"system/",7))
							continue;

						// and don't try caulking against something inherently transparent...
						//
						if (pScannedFace->d_texture->TestMaterialFlag(QER_TRANS))
							continue;

						idWinding *pScannedWinding = pScannedFace->face_winding;

						if (!pScannedWinding)
							continue;	// freed face, probably won't happen here, but who knows with this program?

	//					SquaredFace_t ScannedSquaredFace;
	//					WindingToSquaredFace( &ScannedSquaredFace, pScannedWinding);

	/*					if (VectorCompare(ScannedSquaredFace.v3NormalisedRotationVector, SelectedSquaredFace.v3NormalisedRotationVector)
							&& 
							VectorCompare(ScannedSquaredFace.v3NormalisedElevationVector, SelectedSquaredFace.v3NormalisedElevationVector)
							)
	*/
						{
							// brush faces are in parallel planes to each other, so check that their normals
							//	are opposite, by adding them together and testing for zero...
							// (if normals are opposite, then faces can be against/touching each other?)						
							//
							idVec3 v3ZeroTest;							
							idVec3 v3Zero;v3Zero.Zero();	//static idVec3 v3Zero={0,0,0};
							
							VectorAdd(pSelectedFace->plane.Normal(),pScannedFace->plane.Normal(),v3ZeroTest);
							if (v3ZeroTest == v3Zero)
							{
								// planes are facing each other...
								//
								// coplanar? (this is some maths of Gil's, which I don't even pretend to understand)
								//
								float fTotalDist = 0;
								for (int _i=0; _i<3; _i++)
								{
									fTotalDist += fabs(	DotProduct(pSelectedFace->plane.Normal(),(*pSelectedWinding)[0]) 
														- 
														DotProduct(pSelectedFace->plane.Normal(),(*pScannedWinding)[i])
														);
								}
								//OutputDebugString(va("Dist = %g\n",fTotalDist));
								
								if (fTotalDist > 0.01)
									continue;

								// every point in the selected face must be within (or equal to) the bounds of the
								//	scanned face...
								//
								// work out the bounds first...
								//
								idVec3 v3ScannedBoundsMins, v3ScannedBoundsMaxs;
								ClearBounds (v3ScannedBoundsMins, v3ScannedBoundsMaxs);
								int iPoint;
								for (iPoint=0; iPoint<pScannedWinding->GetNumPoints(); iPoint++)
								{
									AddPointToBounds( (*pScannedWinding)[iPoint].ToVec3(), v3ScannedBoundsMins, v3ScannedBoundsMaxs);
								}
								// floor 'em... (or .001 differences mess things up...
								//
								FloorBounds(v3ScannedBoundsMins, v3ScannedBoundsMaxs);

								
								// now check points from selected face...
								//
								bool bWithin = true;
								for (iPoint=0; iPoint < pSelectedWinding->GetNumPoints(); iPoint++)
								{
									for (int iXYZ=0; iXYZ<3; iXYZ++)
									{
										float f = floor((*pSelectedWinding)[iPoint][iXYZ] + 0.5);
										if (!
												(
												f >= v3ScannedBoundsMins[iXYZ] 
												&&
												f <= v3ScannedBoundsMaxs[iXYZ]
												)
											 )
										{
											bWithin = false;
										}
									}
								}

								if (bWithin)
								{
									PairBrushFace_t PairBrushFace;
													PairBrushFace.pFace = pSelectedFace;
													PairBrushFace.pBrush= pSelectedBrush;
									FacesToCaulk.Append(PairBrushFace);									
								}
							}
						}
					}
				}			
			}
		}
	}

	
	// apply caulk...
	//
	int iFacesCaulked = 0;
	if (FacesToCaulk.Num())
	{
		LPCSTR psCaulkName = "textures/common/caulk";
		const idMaterial *pCaulk = Texture_ForName(psCaulkName);

		if (pCaulk)
		{
			//
			// and call some other junk that Radiant wants so so we can use it later...
			//
			texdef_t tex;
			memset (&tex, 0, sizeof(tex));
			tex.scale[0] = 1;
			tex.scale[1] = 1;
			//tex.flags = pCaulk->flags;	// field missing in Q4
			//tex.value = pCaulk->value;	// ditto
			//tex.contents = pCaulk->contents;	// ditto
			tex.SetName( pCaulk->GetName() );

			//Texture_SetTexture (&tex);

			for (int iListEntry = 0; iListEntry < FacesToCaulk.Num(); iListEntry++)
			{
				PairBrushFace_t &PairBrushFace = FacesToCaulk[iListEntry];
				face_t *pFace = PairBrushFace.pFace;
				brush_t*pBrush= PairBrushFace.pBrush;

				pFace->d_texture = pCaulk;				
				pFace->texdef = tex;

				Face_FitTexture(pFace, 1, 1);	// this doesn't work here for some reason... duh.
				Brush_Build(pBrush);

				iFacesCaulked++;
			}		
		}
		else
		{
			/*Sys_Printf*/common->Printf(" Unable to locate caulk texture at: \"%s\"!\n",psCaulkName);
		}
	}

	/*Sys_Printf*/common->Printf("( %d faces caulked )\n",iFacesCaulked);

	if (iSystemBrushesSkipped)
	{
		/*Sys_Printf*/common->Printf("( %d system-faced brushes skipped )\n",iSystemBrushesSkipped);
	}

	Sys_UpdateWindows (W_ALL);
}
#endif
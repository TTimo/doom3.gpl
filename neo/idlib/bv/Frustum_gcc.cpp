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
#include "../precompiled.h"

void BoxToPoints( const idVec3 &center, const idVec3 &extents, const idMat3 &axis, idVec3 points[8] );

/*
============
idFrustum::ProjectionBounds
============
*/
bool idFrustum::ProjectionBounds( const idBox &box, idBounds &projectionBounds ) const {
	int i, p1, p2, pointCull[8], culled, outside;
	float scale1, scale2;
	idFrustum localFrustum;
	idVec3 points[8], localOrigin;
	idMat3 localAxis, localScaled;
	idBounds bounds( -box.GetExtents(), box.GetExtents() );

	// if the frustum origin is inside the bounds
	if ( bounds.ContainsPoint( ( origin - box.GetCenter() ) * box.GetAxis().Transpose() ) ) {
		// bounds that cover the whole frustum
		float boxMin, boxMax, base;

		base = origin * axis[0];
		box.AxisProjection( axis[0], boxMin, boxMax );

		projectionBounds[0].x = boxMin - base;
		projectionBounds[1].x = boxMax - base;
		projectionBounds[0].y = projectionBounds[0].z = -1.0f;
		projectionBounds[1].y = projectionBounds[1].z = 1.0f;

		return true;
	}

	projectionBounds.Clear();

	// transform the bounds into the space of this frustum
	localOrigin = ( box.GetCenter() - origin ) * axis.Transpose();
	localAxis = box.GetAxis() * axis.Transpose();
	BoxToPoints( localOrigin, box.GetExtents(), localAxis, points );

	// test outer four edges of the bounds
	culled = -1;
	outside = 0;
	for ( i = 0; i < 4; i++ ) {
		p1 = i;
		p2 = 4 + i;
		AddLocalLineToProjectionBoundsSetCull( points[p1], points[p2], pointCull[p1], pointCull[p2], projectionBounds );
		culled &= pointCull[p1] & pointCull[p2];
		outside |= pointCull[p1] | pointCull[p2];
	}

	// if the bounds are completely outside this frustum
	if ( culled ) {
		return false;
	}

	// if the bounds are completely inside this frustum
	if ( !outside ) {
		return true;
	}

	// test the remaining edges of the bounds
	for ( i = 0; i < 4; i++ ) {
		p1 = i;
		p2 = (i+1)&3;
		AddLocalLineToProjectionBoundsUseCull( points[p1], points[p2], pointCull[p1], pointCull[p2], projectionBounds );
	}

	for ( i = 0; i < 4; i++ ) {
		p1 = 4 + i;
		p2 = 4 + ((i+1)&3);
		AddLocalLineToProjectionBoundsUseCull( points[p1], points[p2], pointCull[p1], pointCull[p2], projectionBounds );
	}

	// if the bounds extend beyond two or more boundaries of this frustum
	if ( outside != 1 && outside != 2 && outside != 4 && outside != 8 ) {

		localOrigin = ( origin - box.GetCenter() ) * box.GetAxis().Transpose();
		localScaled = axis * box.GetAxis().Transpose();
		localScaled[0] *= dFar;
		localScaled[1] *= dLeft;
		localScaled[2] *= dUp;

		// test the outer edges of this frustum for intersection with the bounds
		if ( (outside & 2) && (outside & 8) ) {
			BoundsRayIntersection( bounds, localOrigin, localScaled[0] - localScaled[1] - localScaled[2], scale1, scale2 );
			if ( scale1 <= scale2 && scale1 >= 0.0f ) {
				projectionBounds.AddPoint( idVec3( scale1 * dFar, -1.0f, -1.0f ) );
				projectionBounds.AddPoint( idVec3( scale2 * dFar, -1.0f, -1.0f ) );
			}
		}
		if ( (outside & 2) && (outside & 4) ) {
			BoundsRayIntersection( bounds, localOrigin, localScaled[0] - localScaled[1] + localScaled[2], scale1, scale2 );
			if ( scale1 <= scale2 && scale1 >= 0.0f  ) {
				projectionBounds.AddPoint( idVec3( scale1 * dFar, -1.0f, 1.0f ) );
				projectionBounds.AddPoint( idVec3( scale2 * dFar, -1.0f, 1.0f ) );
			}
		}
		if ( (outside & 1) && (outside & 8) ) {
			BoundsRayIntersection( bounds, localOrigin, localScaled[0] + localScaled[1] - localScaled[2], scale1, scale2 );
			if ( scale1 <= scale2 && scale1 >= 0.0f  ) {
				projectionBounds.AddPoint( idVec3( scale1 * dFar, 1.0f, -1.0f ) );
				projectionBounds.AddPoint( idVec3( scale2 * dFar, 1.0f, -1.0f ) );
			}
		}
		if ( (outside & 1) && (outside & 2) ) {
			BoundsRayIntersection( bounds, localOrigin, localScaled[0] + localScaled[1] + localScaled[2], scale1, scale2 );
			if ( scale1 <= scale2 && scale1 >= 0.0f  ) {
				projectionBounds.AddPoint( idVec3( scale1 * dFar, 1.0f, 1.0f ) );
				projectionBounds.AddPoint( idVec3( scale2 * dFar, 1.0f, 1.0f ) );
			}
		}
	}

	return true;
}

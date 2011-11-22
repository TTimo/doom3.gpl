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

#ifndef __BV_FRUSTUM_H__
#define __BV_FRUSTUM_H__

/*
===============================================================================

	Orthogonal Frustum

===============================================================================
*/

class idFrustum {
public:
					idFrustum( void );

	void			SetOrigin( const idVec3 &origin );
	void			SetAxis( const idMat3 &axis );
	void			SetSize( float dNear, float dFar, float dLeft, float dUp );
	void			SetPyramid( float dNear, float dFar );
	void			MoveNearDistance( float dNear );
	void			MoveFarDistance( float dFar );

	const idVec3 &	GetOrigin( void ) const;						// returns frustum origin
	const idMat3 &	GetAxis( void ) const;							// returns frustum orientation
	idVec3			GetCenter( void ) const;						// returns center of frustum

	bool			IsValid( void ) const;							// returns true if the frustum is valid
	float			GetNearDistance( void ) const;					// returns distance to near plane
	float			GetFarDistance( void ) const;					// returns distance to far plane
	float			GetLeft( void ) const;							// returns left vector length
	float			GetUp( void ) const;							// returns up vector length

	idFrustum		Expand( const float d ) const;					// returns frustum expanded in all directions with the given value
	idFrustum &		ExpandSelf( const float d );					// expands frustum in all directions with the given value
	idFrustum		Translate( const idVec3 &translation ) const;	// returns translated frustum
	idFrustum &		TranslateSelf( const idVec3 &translation );		// translates frustum
	idFrustum		Rotate( const idMat3 &rotation ) const;			// returns rotated frustum
	idFrustum &		RotateSelf( const idMat3 &rotation );			// rotates frustum

	float			PlaneDistance( const idPlane &plane ) const;
	int				PlaneSide( const idPlane &plane, const float epsilon = ON_EPSILON ) const;

					// fast culling but might not cull everything outside the frustum
	bool			CullPoint( const idVec3 &point ) const;
	bool			CullBounds( const idBounds &bounds ) const;
	bool			CullBox( const idBox &box ) const;
	bool			CullSphere( const idSphere &sphere ) const;
	bool			CullFrustum( const idFrustum &frustum ) const;
	bool			CullWinding( const class idWinding &winding ) const;

					// exact intersection tests
	bool			ContainsPoint( const idVec3 &point ) const;
	bool			IntersectsBounds( const idBounds &bounds ) const;
	bool			IntersectsBox( const idBox &box ) const;
	bool			IntersectsSphere( const idSphere &sphere ) const;
	bool			IntersectsFrustum( const idFrustum &frustum ) const;
	bool			IntersectsWinding( const idWinding &winding ) const;
	bool			LineIntersection( const idVec3 &start, const idVec3 &end ) const;
	bool			RayIntersection( const idVec3 &start, const idVec3 &dir, float &scale1, float &scale2 ) const;

					// returns true if the projection origin is far enough away from the bounding volume to create a valid frustum
	bool			FromProjection( const idBounds &bounds, const idVec3 &projectionOrigin, const float dFar );
	bool			FromProjection( const idBox &box, const idVec3 &projectionOrigin, const float dFar );
	bool			FromProjection( const idSphere &sphere, const idVec3 &projectionOrigin, const float dFar );

					// moves the far plane so it extends just beyond the bounding volume
	bool			ConstrainToBounds( const idBounds &bounds );
	bool			ConstrainToBox( const idBox &box );
	bool			ConstrainToSphere( const idSphere &sphere );
	bool			ConstrainToFrustum( const idFrustum &frustum );

	void			ToPlanes( idPlane planes[6] ) const;			// planes point outwards
	void			ToPoints( idVec3 points[8] ) const;				// 8 corners of the frustum

					// calculates the projection of this frustum onto the given axis
	void			AxisProjection( const idVec3 &dir, float &min, float &max ) const;
	void			AxisProjection( const idMat3 &ax, idBounds &bounds ) const;

					// calculates the bounds for the projection in this frustum
	bool			ProjectionBounds( const idBounds &bounds, idBounds &projectionBounds ) const;
	bool			ProjectionBounds( const idBox &box, idBounds &projectionBounds ) const;
	bool			ProjectionBounds( const idSphere &sphere, idBounds &projectionBounds ) const;
	bool			ProjectionBounds( const idFrustum &frustum, idBounds &projectionBounds ) const;
	bool			ProjectionBounds( const idWinding &winding, idBounds &projectionBounds ) const;

					// calculates the bounds for the projection in this frustum of the given frustum clipped to the given box
	bool			ClippedProjectionBounds( const idFrustum &frustum, const idBox &clipBox, idBounds &projectionBounds ) const;

private:
	idVec3			origin;		// frustum origin
	idMat3			axis;		// frustum orientation
	float			dNear;		// distance of near plane, dNear >= 0.0f
	float			dFar;		// distance of far plane, dFar > dNear
	float			dLeft;		// half the width at the far plane
	float			dUp;		// half the height at the far plane
	float			invFar;		// 1.0f / dFar

private:
	bool			CullLocalBox( const idVec3 &localOrigin, const idVec3 &extents, const idMat3 &localAxis ) const;
	bool			CullLocalFrustum( const idFrustum &localFrustum, const idVec3 indexPoints[8], const idVec3 cornerVecs[4] ) const;
	bool			CullLocalWinding( const idVec3 *points, const int numPoints, int *pointCull ) const;
	bool			BoundsCullLocalFrustum( const idBounds &bounds, const idFrustum &localFrustum, const idVec3 indexPoints[8], const idVec3 cornerVecs[4] ) const;
	bool			LocalLineIntersection( const idVec3 &start, const idVec3 &end ) const;
	bool			LocalRayIntersection( const idVec3 &start, const idVec3 &dir, float &scale1, float &scale2 ) const;
	bool			LocalFrustumIntersectsFrustum( const idVec3 points[8], const bool testFirstSide ) const;
	bool			LocalFrustumIntersectsBounds( const idVec3 points[8], const idBounds &bounds ) const;
	void			ToClippedPoints( const float fractions[4], idVec3 points[8] ) const;
	void			ToIndexPoints( idVec3 indexPoints[8] ) const;
	void			ToIndexPointsAndCornerVecs( idVec3 indexPoints[8], idVec3 cornerVecs[4] ) const;
	void			AxisProjection( const idVec3 indexPoints[8], const idVec3 cornerVecs[4], const idVec3 &dir, float &min, float &max ) const;
	void			AddLocalLineToProjectionBoundsSetCull( const idVec3 &start, const idVec3 &end, int &startCull, int &endCull, idBounds &bounds ) const;
	void			AddLocalLineToProjectionBoundsUseCull( const idVec3 &start, const idVec3 &end, int startCull, int endCull, idBounds &bounds ) const;
	bool			AddLocalCapsToProjectionBounds( const idVec3 endPoints[4], const int endPointCull[4], const idVec3 &point, int pointCull, int pointClip, idBounds &projectionBounds ) const;
	bool			BoundsRayIntersection( const idBounds &bounds, const idVec3 &start, const idVec3 &dir, float &scale1, float &scale2 ) const;
	void			ClipFrustumToBox( const idBox &box, float clipFractions[4], int clipPlanes[4] ) const;
	bool			ClipLine( const idVec3 localPoints[8], const idVec3 points[8], int startIndex, int endIndex, idVec3 &start, idVec3 &end, int &startClip, int &endClip ) const;
};


ID_INLINE idFrustum::idFrustum( void ) {
	dNear = dFar = 0.0f;
}

ID_INLINE void idFrustum::SetOrigin( const idVec3 &origin ) {
	this->origin = origin;
}

ID_INLINE void idFrustum::SetAxis( const idMat3 &axis ) {
	this->axis = axis;
}

ID_INLINE void idFrustum::SetSize( float dNear, float dFar, float dLeft, float dUp ) {
	assert( dNear >= 0.0f && dFar > dNear && dLeft > 0.0f && dUp > 0.0f );
	this->dNear = dNear;
	this->dFar = dFar;
	this->dLeft = dLeft;
	this->dUp = dUp;
	this->invFar = 1.0f / dFar;
}

ID_INLINE void idFrustum::SetPyramid( float dNear, float dFar ) {
	assert( dNear >= 0.0f && dFar > dNear );
	this->dNear = dNear;
	this->dFar = dFar;
	this->dLeft = dFar;
	this->dUp = dFar;
	this->invFar = 1.0f / dFar;
}

ID_INLINE void idFrustum::MoveNearDistance( float dNear ) {
	assert( dNear >= 0.0f );
	this->dNear = dNear;
}

ID_INLINE void idFrustum::MoveFarDistance( float dFar ) {
	assert( dFar > this->dNear );
	float scale = dFar / this->dFar;
	this->dFar = dFar;
	this->dLeft *= scale;
	this->dUp *= scale;
	this->invFar = 1.0f / dFar;
}

ID_INLINE const idVec3 &idFrustum::GetOrigin( void ) const {
	return origin;
}

ID_INLINE const idMat3 &idFrustum::GetAxis( void ) const {
	return axis;
}

ID_INLINE idVec3 idFrustum::GetCenter( void ) const {
	return ( origin + axis[0] * ( ( dFar - dNear ) * 0.5f ) );
}

ID_INLINE bool idFrustum::IsValid( void ) const {
	return ( dFar > dNear );
}

ID_INLINE float idFrustum::GetNearDistance( void ) const {
	return dNear;
}

ID_INLINE float idFrustum::GetFarDistance( void ) const {
	return dFar;
}

ID_INLINE float idFrustum::GetLeft( void ) const {
	return dLeft;
}

ID_INLINE float idFrustum::GetUp( void ) const {
	return dUp;
}

ID_INLINE idFrustum idFrustum::Expand( const float d ) const {
	idFrustum f = *this;
	f.origin -= d * f.axis[0];
	f.dFar += 2.0f * d;
	f.dLeft = f.dFar * dLeft * invFar;
	f.dUp = f.dFar * dUp * invFar;
	f.invFar = 1.0f / dFar;
	return f;
}

ID_INLINE idFrustum &idFrustum::ExpandSelf( const float d ) {
	origin -= d * axis[0];
	dFar += 2.0f * d;
	dLeft = dFar * dLeft * invFar;
	dUp = dFar * dUp * invFar;
	invFar = 1.0f / dFar;
	return *this;
}

ID_INLINE idFrustum idFrustum::Translate( const idVec3 &translation ) const {
	idFrustum f = *this;
	f.origin += translation;
	return f;
}

ID_INLINE idFrustum &idFrustum::TranslateSelf( const idVec3 &translation ) {
	origin += translation;
	return *this;
}

ID_INLINE idFrustum idFrustum::Rotate( const idMat3 &rotation ) const {
	idFrustum f = *this;
	f.axis *= rotation;
	return f;
}

ID_INLINE idFrustum &idFrustum::RotateSelf( const idMat3 &rotation ) {
	axis *= rotation;
	return *this;
}

#endif /* !__BV_FRUSTUM_H__ */

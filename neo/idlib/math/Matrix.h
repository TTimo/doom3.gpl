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

#ifndef __MATH_MATRIX_H__
#define __MATH_MATRIX_H__

/*
===============================================================================

  Matrix classes, all matrices are row-major except idMat3

===============================================================================
*/

#define MATRIX_INVERSE_EPSILON		1e-14
#define MATRIX_EPSILON				1e-6

class idAngles;
class idQuat;
class idCQuat;
class idRotation;
class idMat4;

//===============================================================
//
//	idMat2 - 2x2 matrix
//
//===============================================================

class idMat2 {
public:
					idMat2( void );
					explicit idMat2( const idVec2 &x, const idVec2 &y );
					explicit idMat2( const float xx, const float xy, const float yx, const float yy );
					explicit idMat2( const float src[ 2 ][ 2 ] );

	const idVec2 &	operator[]( int index ) const;
	idVec2 &		operator[]( int index );
	idMat2			operator-() const;
	idMat2			operator*( const float a ) const;
	idVec2			operator*( const idVec2 &vec ) const;
	idMat2			operator*( const idMat2 &a ) const;
	idMat2			operator+( const idMat2 &a ) const;
	idMat2			operator-( const idMat2 &a ) const;
	idMat2 &		operator*=( const float a );
	idMat2 &		operator*=( const idMat2 &a );
	idMat2 &		operator+=( const idMat2 &a );
	idMat2 &		operator-=( const idMat2 &a );

	friend idMat2	operator*( const float a, const idMat2 &mat );
	friend idVec2	operator*( const idVec2 &vec, const idMat2 &mat );
	friend idVec2 &	operator*=( idVec2 &vec, const idMat2 &mat );

	bool			Compare( const idMat2 &a ) const;						// exact compare, no epsilon
	bool			Compare( const idMat2 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const idMat2 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const idMat2 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;

	float			Trace( void ) const;
	float			Determinant( void ) const;
	idMat2			Transpose( void ) const;	// returns transpose
	idMat2 &		TransposeSelf( void );
	idMat2			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	idMat2			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	idVec2			mat[ 2 ];
};

extern idMat2 mat2_zero;
extern idMat2 mat2_identity;
#define mat2_default	mat2_identity

ID_INLINE idMat2::idMat2( void ) {
}

ID_INLINE idMat2::idMat2( const idVec2 &x, const idVec2 &y ) {
	mat[ 0 ].x = x.x; mat[ 0 ].y = x.y;
	mat[ 1 ].x = y.x; mat[ 1 ].y = y.y;
}

ID_INLINE idMat2::idMat2( const float xx, const float xy, const float yx, const float yy ) {
	mat[ 0 ].x = xx; mat[ 0 ].y = xy;
	mat[ 1 ].x = yx; mat[ 1 ].y = yy;
}

ID_INLINE idMat2::idMat2( const float src[ 2 ][ 2 ] ) {
	memcpy( mat, src, 2 * 2 * sizeof( float ) );
}

ID_INLINE const idVec2 &idMat2::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 2 ) );
	return mat[ index ];
}

ID_INLINE idVec2 &idMat2::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 2 ) );
	return mat[ index ];
}

ID_INLINE idMat2 idMat2::operator-() const {
	return idMat2(	-mat[0][0], -mat[0][1],
					-mat[1][0], -mat[1][1] );
}

ID_INLINE idVec2 idMat2::operator*( const idVec2 &vec ) const {
	return idVec2(
		mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y,
		mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y );
}

ID_INLINE idMat2 idMat2::operator*( const idMat2 &a ) const {
	return idMat2(
		mat[0].x * a[0].x + mat[0].y * a[1].x,
		mat[0].x * a[0].y + mat[0].y * a[1].y,
		mat[1].x * a[0].x + mat[1].y * a[1].x,
		mat[1].x * a[0].y + mat[1].y * a[1].y );
}

ID_INLINE idMat2 idMat2::operator*( const float a ) const {
	return idMat2(
		mat[0].x * a, mat[0].y * a, 
		mat[1].x * a, mat[1].y * a );
}

ID_INLINE idMat2 idMat2::operator+( const idMat2 &a ) const {
	return idMat2(
		mat[0].x + a[0].x, mat[0].y + a[0].y, 
		mat[1].x + a[1].x, mat[1].y + a[1].y );
}
    
ID_INLINE idMat2 idMat2::operator-( const idMat2 &a ) const {
	return idMat2(
		mat[0].x - a[0].x, mat[0].y - a[0].y,
		mat[1].x - a[1].x, mat[1].y - a[1].y );
}

ID_INLINE idMat2 &idMat2::operator*=( const float a ) {
	mat[0].x *= a; mat[0].y *= a;
	mat[1].x *= a; mat[1].y *= a;

    return *this;
}

ID_INLINE idMat2 &idMat2::operator*=( const idMat2 &a ) {
	float x, y;
	x = mat[0].x; y = mat[0].y;
	mat[0].x = x * a[0].x + y * a[1].x;
	mat[0].y = x * a[0].y + y * a[1].y;
	x = mat[1].x; y = mat[1].y;
	mat[1].x = x * a[0].x + y * a[1].x;
	mat[1].y = x * a[0].y + y * a[1].y;
	return *this;
}

ID_INLINE idMat2 &idMat2::operator+=( const idMat2 &a ) {
	mat[0].x += a[0].x; mat[0].y += a[0].y;
	mat[1].x += a[1].x; mat[1].y += a[1].y;

    return *this;
}

ID_INLINE idMat2 &idMat2::operator-=( const idMat2 &a ) {
	mat[0].x -= a[0].x; mat[0].y -= a[0].y;
	mat[1].x -= a[1].x; mat[1].y -= a[1].y;

    return *this;
}

ID_INLINE idVec2 operator*( const idVec2 &vec, const idMat2 &mat ) {
	return mat * vec;
}

ID_INLINE idMat2 operator*( const float a, idMat2 const &mat ) {
	return mat * a;
}

ID_INLINE idVec2 &operator*=( idVec2 &vec, const idMat2 &mat ) {
	vec = mat * vec;
	return vec;
}

ID_INLINE bool idMat2::Compare( const idMat2 &a ) const {
	if ( mat[0].Compare( a[0] ) &&
		mat[1].Compare( a[1] ) ) {
		return true;
	}
	return false;
}

ID_INLINE bool idMat2::Compare( const idMat2 &a, const float epsilon ) const {
	if ( mat[0].Compare( a[0], epsilon ) &&
		mat[1].Compare( a[1], epsilon ) ) {
		return true;
	}
	return false;
}

ID_INLINE bool idMat2::operator==( const idMat2 &a ) const {
	return Compare( a );
}

ID_INLINE bool idMat2::operator!=( const idMat2 &a ) const {
	return !Compare( a );
}

ID_INLINE void idMat2::Zero( void ) {
	mat[0].Zero();
	mat[1].Zero();
}

ID_INLINE void idMat2::Identity( void ) {
	*this = mat2_identity;
}

ID_INLINE bool idMat2::IsIdentity( const float epsilon ) const {
	return Compare( mat2_identity, epsilon );
}

ID_INLINE bool idMat2::IsSymmetric( const float epsilon ) const {
	return ( idMath::Fabs( mat[0][1] - mat[1][0] ) < epsilon );
}

ID_INLINE bool idMat2::IsDiagonal( const float epsilon ) const {
	if ( idMath::Fabs( mat[0][1] ) > epsilon ||
		idMath::Fabs( mat[1][0] ) > epsilon ) {
		return false;
	}
	return true;
}

ID_INLINE float idMat2::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] );
}

ID_INLINE float idMat2::Determinant( void ) const {
	return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
}

ID_INLINE idMat2 idMat2::Transpose( void ) const {
	return idMat2(	mat[0][0], mat[1][0],
					mat[0][1], mat[1][1] );
}

ID_INLINE idMat2 &idMat2::TransposeSelf( void ) {
	float tmp;

	tmp = mat[0][1];
	mat[0][1] = mat[1][0];
	mat[1][0] = tmp;

	return *this;
}

ID_INLINE idMat2 idMat2::Inverse( void ) const {
	idMat2 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

ID_INLINE idMat2 idMat2::InverseFast( void ) const {
	idMat2 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

ID_INLINE int idMat2::GetDimension( void ) const {
	return 4;
}

ID_INLINE const float *idMat2::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

ID_INLINE float *idMat2::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}


//===============================================================
//
//	idMat3 - 3x3 matrix
//
//	NOTE:	matrix is column-major
//
//===============================================================

class idMat3 {
public:
					idMat3( void );
					explicit idMat3( const idVec3 &x, const idVec3 &y, const idVec3 &z );
					explicit idMat3( const float xx, const float xy, const float xz, const float yx, const float yy, const float yz, const float zx, const float zy, const float zz );
					explicit idMat3( const float src[ 3 ][ 3 ] );

	const idVec3 &	operator[]( int index ) const;
	idVec3 &		operator[]( int index );
	idMat3			operator-() const;
	idMat3			operator*( const float a ) const;
	idVec3			operator*( const idVec3 &vec ) const;
	idMat3			operator*( const idMat3 &a ) const;
	idMat3			operator+( const idMat3 &a ) const;
	idMat3			operator-( const idMat3 &a ) const;
	idMat3 &		operator*=( const float a );
	idMat3 &		operator*=( const idMat3 &a );
	idMat3 &		operator+=( const idMat3 &a );
	idMat3 &		operator-=( const idMat3 &a );

	friend idMat3	operator*( const float a, const idMat3 &mat );
	friend idVec3	operator*( const idVec3 &vec, const idMat3 &mat );
	friend idVec3 &	operator*=( idVec3 &vec, const idMat3 &mat );

	bool			Compare( const idMat3 &a ) const;						// exact compare, no epsilon
	bool			Compare( const idMat3 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const idMat3 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const idMat3 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsRotated( void ) const;

	void			ProjectVector( const idVec3 &src, idVec3 &dst ) const;
	void			UnprojectVector( const idVec3 &src, idVec3 &dst ) const;

	bool			FixDegeneracies( void );	// fix degenerate axial cases
	bool			FixDenormals( void );		// change tiny numbers to zero

	float			Trace( void ) const;
	float			Determinant( void ) const;
	idMat3			OrthoNormalize( void ) const;
	idMat3 &		OrthoNormalizeSelf( void );
	idMat3			Transpose( void ) const;	// returns transpose
	idMat3 &		TransposeSelf( void );
	idMat3			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	idMat3			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero
	idMat3			TransposeMultiply( const idMat3 &b ) const;

	idMat3			InertiaTranslate( const float mass, const idVec3 &centerOfMass, const idVec3 &translation ) const;
	idMat3 &		InertiaTranslateSelf( const float mass, const idVec3 &centerOfMass, const idVec3 &translation );
	idMat3			InertiaRotate( const idMat3 &rotation ) const;
	idMat3 &		InertiaRotateSelf( const idMat3 &rotation );

	int				GetDimension( void ) const;

	idAngles		ToAngles( void ) const;
	idQuat			ToQuat( void ) const;
	idCQuat			ToCQuat( void ) const;
	idRotation		ToRotation( void ) const;
	idMat4			ToMat4( void ) const;
	idVec3			ToAngularVelocity( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

	friend void		TransposeMultiply( const idMat3 &inv, const idMat3 &b, idMat3 &dst );
	friend idMat3	SkewSymmetric( idVec3 const &src );

private:
	idVec3			mat[ 3 ];
};

extern idMat3 mat3_zero;
extern idMat3 mat3_identity;
#define mat3_default	mat3_identity

ID_INLINE idMat3::idMat3( void ) {
}

ID_INLINE idMat3::idMat3( const idVec3 &x, const idVec3 &y, const idVec3 &z ) {
	mat[ 0 ].x = x.x; mat[ 0 ].y = x.y; mat[ 0 ].z = x.z;
	mat[ 1 ].x = y.x; mat[ 1 ].y = y.y; mat[ 1 ].z = y.z;
	mat[ 2 ].x = z.x; mat[ 2 ].y = z.y; mat[ 2 ].z = z.z;
}

ID_INLINE idMat3::idMat3( const float xx, const float xy, const float xz, const float yx, const float yy, const float yz, const float zx, const float zy, const float zz ) {
	mat[ 0 ].x = xx; mat[ 0 ].y = xy; mat[ 0 ].z = xz;
	mat[ 1 ].x = yx; mat[ 1 ].y = yy; mat[ 1 ].z = yz;
	mat[ 2 ].x = zx; mat[ 2 ].y = zy; mat[ 2 ].z = zz;
}

ID_INLINE idMat3::idMat3( const float src[ 3 ][ 3 ] ) {
	memcpy( mat, src, 3 * 3 * sizeof( float ) );
}

ID_INLINE const idVec3 &idMat3::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 3 ) );
	return mat[ index ];
}

ID_INLINE idVec3 &idMat3::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 3 ) );
	return mat[ index ];
}

ID_INLINE idMat3 idMat3::operator-() const {
	return idMat3(	-mat[0][0], -mat[0][1], -mat[0][2],
					-mat[1][0], -mat[1][1], -mat[1][2],
					-mat[2][0], -mat[2][1], -mat[2][2] );
}

ID_INLINE idVec3 idMat3::operator*( const idVec3 &vec ) const {
	return idVec3(
		mat[ 0 ].x * vec.x + mat[ 1 ].x * vec.y + mat[ 2 ].x * vec.z,
		mat[ 0 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 2 ].y * vec.z,
		mat[ 0 ].z * vec.x + mat[ 1 ].z * vec.y + mat[ 2 ].z * vec.z );
}

ID_INLINE idMat3 idMat3::operator*( const idMat3 &a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	idMat3 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 3 + j ]
					+ m1Ptr[1] * m2Ptr[ 1 * 3 + j ]
					+ m1Ptr[2] * m2Ptr[ 2 * 3 + j ];
			dstPtr++;
		}
		m1Ptr += 3;
	}
	return dst;
}

ID_INLINE idMat3 idMat3::operator*( const float a ) const {
	return idMat3(
		mat[0].x * a, mat[0].y * a, mat[0].z * a,
		mat[1].x * a, mat[1].y * a, mat[1].z * a,
		mat[2].x * a, mat[2].y * a, mat[2].z * a );
}

ID_INLINE idMat3 idMat3::operator+( const idMat3 &a ) const {
	return idMat3(
		mat[0].x + a[0].x, mat[0].y + a[0].y, mat[0].z + a[0].z,
		mat[1].x + a[1].x, mat[1].y + a[1].y, mat[1].z + a[1].z,
		mat[2].x + a[2].x, mat[2].y + a[2].y, mat[2].z + a[2].z );
}
    
ID_INLINE idMat3 idMat3::operator-( const idMat3 &a ) const {
	return idMat3(
		mat[0].x - a[0].x, mat[0].y - a[0].y, mat[0].z - a[0].z,
		mat[1].x - a[1].x, mat[1].y - a[1].y, mat[1].z - a[1].z,
		mat[2].x - a[2].x, mat[2].y - a[2].y, mat[2].z - a[2].z );
}

ID_INLINE idMat3 &idMat3::operator*=( const float a ) {
	mat[0].x *= a; mat[0].y *= a; mat[0].z *= a;
	mat[1].x *= a; mat[1].y *= a; mat[1].z *= a; 
	mat[2].x *= a; mat[2].y *= a; mat[2].z *= a;

    return *this;
}

ID_INLINE idMat3 &idMat3::operator*=( const idMat3 &a ) {
	int i, j;
	const float *m2Ptr;
	float *m1Ptr, dst[3];

	m1Ptr = reinterpret_cast<float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			dst[j]  = m1Ptr[0] * m2Ptr[ 0 * 3 + j ]
					+ m1Ptr[1] * m2Ptr[ 1 * 3 + j ]
					+ m1Ptr[2] * m2Ptr[ 2 * 3 + j ];
		}
		m1Ptr[0] = dst[0]; m1Ptr[1] = dst[1]; m1Ptr[2] = dst[2];
		m1Ptr += 3;
	}
	return *this;
}

ID_INLINE idMat3 &idMat3::operator+=( const idMat3 &a ) {
	mat[0].x += a[0].x; mat[0].y += a[0].y; mat[0].z += a[0].z;
	mat[1].x += a[1].x; mat[1].y += a[1].y; mat[1].z += a[1].z;
	mat[2].x += a[2].x; mat[2].y += a[2].y; mat[2].z += a[2].z;

    return *this;
}

ID_INLINE idMat3 &idMat3::operator-=( const idMat3 &a ) {
	mat[0].x -= a[0].x; mat[0].y -= a[0].y; mat[0].z -= a[0].z;
	mat[1].x -= a[1].x; mat[1].y -= a[1].y; mat[1].z -= a[1].z;
	mat[2].x -= a[2].x; mat[2].y -= a[2].y; mat[2].z -= a[2].z;

    return *this;
}

ID_INLINE idVec3 operator*( const idVec3 &vec, const idMat3 &mat ) {
	return mat * vec;
}

ID_INLINE idMat3 operator*( const float a, const idMat3 &mat ) {
	return mat * a;
}

ID_INLINE idVec3 &operator*=( idVec3 &vec, const idMat3 &mat ) {
	float x = mat[ 0 ].x * vec.x + mat[ 1 ].x * vec.y + mat[ 2 ].x * vec.z;
	float y = mat[ 0 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 2 ].y * vec.z;
	vec.z = mat[ 0 ].z * vec.x + mat[ 1 ].z * vec.y + mat[ 2 ].z * vec.z;
	vec.x = x;
	vec.y = y;
	return vec;
}

ID_INLINE bool idMat3::Compare( const idMat3 &a ) const {
	if ( mat[0].Compare( a[0] ) &&
		mat[1].Compare( a[1] ) &&
		mat[2].Compare( a[2] ) ) {
		return true;
	}
	return false;
}

ID_INLINE bool idMat3::Compare( const idMat3 &a, const float epsilon ) const {
	if ( mat[0].Compare( a[0], epsilon ) &&
		mat[1].Compare( a[1], epsilon ) &&
		mat[2].Compare( a[2], epsilon ) ) {
		return true;
	}
	return false;
}

ID_INLINE bool idMat3::operator==( const idMat3 &a ) const {
	return Compare( a );
}

ID_INLINE bool idMat3::operator!=( const idMat3 &a ) const {
	return !Compare( a );
}

ID_INLINE void idMat3::Zero( void ) {
	memset( mat, 0, sizeof( idMat3 ) );
}

ID_INLINE void idMat3::Identity( void ) {
	*this = mat3_identity;
}

ID_INLINE bool idMat3::IsIdentity( const float epsilon ) const {
	return Compare( mat3_identity, epsilon );
}

ID_INLINE bool idMat3::IsSymmetric( const float epsilon ) const {
	if ( idMath::Fabs( mat[0][1] - mat[1][0] ) > epsilon ) {
		return false;
	}
	if ( idMath::Fabs( mat[0][2] - mat[2][0] ) > epsilon ) {
		return false;
	}
	if ( idMath::Fabs( mat[1][2] - mat[2][1] ) > epsilon ) {
		return false;
	}
	return true;
}

ID_INLINE bool idMat3::IsDiagonal( const float epsilon ) const {
	if ( idMath::Fabs( mat[0][1] ) > epsilon ||
		idMath::Fabs( mat[0][2] ) > epsilon ||
		idMath::Fabs( mat[1][0] ) > epsilon ||
		idMath::Fabs( mat[1][2] ) > epsilon ||
		idMath::Fabs( mat[2][0] ) > epsilon ||
		idMath::Fabs( mat[2][1] ) > epsilon ) {
		return false;
	}
	return true;
}

ID_INLINE bool idMat3::IsRotated( void ) const {
	return !Compare( mat3_identity );
}

ID_INLINE void idMat3::ProjectVector( const idVec3 &src, idVec3 &dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
}

ID_INLINE void idMat3::UnprojectVector( const idVec3 &src, idVec3 &dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z;
}

ID_INLINE bool idMat3::FixDegeneracies( void ) {
	bool r = mat[0].FixDegenerateNormal();
	r |= mat[1].FixDegenerateNormal();
	r |= mat[2].FixDegenerateNormal();
	return r;
}

ID_INLINE bool idMat3::FixDenormals( void ) {
	bool r = mat[0].FixDenormals();
	r |= mat[1].FixDenormals();
	r |= mat[2].FixDenormals();
	return r;
}

ID_INLINE float idMat3::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] );
}

ID_INLINE idMat3 idMat3::OrthoNormalize( void ) const {
	idMat3 ortho;

	ortho = *this;
	ortho[ 0 ].Normalize();
	ortho[ 2 ].Cross( mat[ 0 ], mat[ 1 ] );
	ortho[ 2 ].Normalize();
	ortho[ 1 ].Cross( mat[ 2 ], mat[ 0 ] );
	ortho[ 1 ].Normalize();
	return ortho;
}

ID_INLINE idMat3 &idMat3::OrthoNormalizeSelf( void ) {
	mat[ 0 ].Normalize();
	mat[ 2 ].Cross( mat[ 0 ], mat[ 1 ] );
	mat[ 2 ].Normalize();
	mat[ 1 ].Cross( mat[ 2 ], mat[ 0 ] );
	mat[ 1 ].Normalize();
	return *this;
}

ID_INLINE idMat3 idMat3::Transpose( void ) const {
	return idMat3(	mat[0][0], mat[1][0], mat[2][0],
					mat[0][1], mat[1][1], mat[2][1],
					mat[0][2], mat[1][2], mat[2][2] );
}

ID_INLINE idMat3 &idMat3::TransposeSelf( void ) {
	float tmp0, tmp1, tmp2;

	tmp0 = mat[0][1];
	mat[0][1] = mat[1][0];
	mat[1][0] = tmp0;
	tmp1 = mat[0][2];
	mat[0][2] = mat[2][0];
	mat[2][0] = tmp1;
	tmp2 = mat[1][2];
	mat[1][2] = mat[2][1];
	mat[2][1] = tmp2;

	return *this;
}

ID_INLINE idMat3 idMat3::Inverse( void ) const {
	idMat3 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

ID_INLINE idMat3 idMat3::InverseFast( void ) const {
	idMat3 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

ID_INLINE idMat3 idMat3::TransposeMultiply( const idMat3 &b ) const {
	return idMat3(	mat[0].x * b[0].x + mat[1].x * b[1].x + mat[2].x * b[2].x,
					mat[0].x * b[0].y + mat[1].x * b[1].y + mat[2].x * b[2].y,
					mat[0].x * b[0].z + mat[1].x * b[1].z + mat[2].x * b[2].z,
					mat[0].y * b[0].x + mat[1].y * b[1].x + mat[2].y * b[2].x,
					mat[0].y * b[0].y + mat[1].y * b[1].y + mat[2].y * b[2].y,
					mat[0].y * b[0].z + mat[1].y * b[1].z + mat[2].y * b[2].z,
					mat[0].z * b[0].x + mat[1].z * b[1].x + mat[2].z * b[2].x,
					mat[0].z * b[0].y + mat[1].z * b[1].y + mat[2].z * b[2].y,
					mat[0].z * b[0].z + mat[1].z * b[1].z + mat[2].z * b[2].z );
}

ID_INLINE void TransposeMultiply( const idMat3 &transpose, const idMat3 &b, idMat3 &dst ) {
	dst[0].x = transpose[0].x * b[0].x + transpose[1].x * b[1].x + transpose[2].x * b[2].x;
	dst[0].y = transpose[0].x * b[0].y + transpose[1].x * b[1].y + transpose[2].x * b[2].y;
	dst[0].z = transpose[0].x * b[0].z + transpose[1].x * b[1].z + transpose[2].x * b[2].z;
	dst[1].x = transpose[0].y * b[0].x + transpose[1].y * b[1].x + transpose[2].y * b[2].x;
	dst[1].y = transpose[0].y * b[0].y + transpose[1].y * b[1].y + transpose[2].y * b[2].y;
	dst[1].z = transpose[0].y * b[0].z + transpose[1].y * b[1].z + transpose[2].y * b[2].z;
	dst[2].x = transpose[0].z * b[0].x + transpose[1].z * b[1].x + transpose[2].z * b[2].x;
	dst[2].y = transpose[0].z * b[0].y + transpose[1].z * b[1].y + transpose[2].z * b[2].y;
	dst[2].z = transpose[0].z * b[0].z + transpose[1].z * b[1].z + transpose[2].z * b[2].z;
}

ID_INLINE idMat3 SkewSymmetric( idVec3 const &src ) {
	return idMat3( 0.0f, -src.z,  src.y, src.z,   0.0f, -src.x, -src.y,  src.x,   0.0f );
}

ID_INLINE int idMat3::GetDimension( void ) const {
	return 9;
}

ID_INLINE const float *idMat3::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

ID_INLINE float *idMat3::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}


//===============================================================
//
//	idMat4 - 4x4 matrix
//
//===============================================================

class idMat4 {
public:
					idMat4( void );
					explicit idMat4( const idVec4 &x, const idVec4 &y, const idVec4 &z, const idVec4 &w );
					explicit idMat4(const float xx, const float xy, const float xz, const float xw,
									const float yx, const float yy, const float yz, const float yw,
									const float zx, const float zy, const float zz, const float zw,
									const float wx, const float wy, const float wz, const float ww );
					explicit idMat4( const idMat3 &rotation, const idVec3 &translation );
					explicit idMat4( const float src[ 4 ][ 4 ] );

	const idVec4 &	operator[]( int index ) const;
	idVec4 &		operator[]( int index );
	idMat4			operator*( const float a ) const;
	idVec4			operator*( const idVec4 &vec ) const;
	idVec3			operator*( const idVec3 &vec ) const;
	idMat4			operator*( const idMat4 &a ) const;
	idMat4			operator+( const idMat4 &a ) const;
	idMat4			operator-( const idMat4 &a ) const;
	idMat4 &		operator*=( const float a );
	idMat4 &		operator*=( const idMat4 &a );
	idMat4 &		operator+=( const idMat4 &a );
	idMat4 &		operator-=( const idMat4 &a );

	friend idMat4	operator*( const float a, const idMat4 &mat );
	friend idVec4	operator*( const idVec4 &vec, const idMat4 &mat );
	friend idVec3	operator*( const idVec3 &vec, const idMat4 &mat );
	friend idVec4 &	operator*=( idVec4 &vec, const idMat4 &mat );
	friend idVec3 &	operator*=( idVec3 &vec, const idMat4 &mat );

	bool			Compare( const idMat4 &a ) const;						// exact compare, no epsilon
	bool			Compare( const idMat4 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const idMat4 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const idMat4 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsRotated( void ) const;

	void			ProjectVector( const idVec4 &src, idVec4 &dst ) const;
	void			UnprojectVector( const idVec4 &src, idVec4 &dst ) const;

	float			Trace( void ) const;
	float			Determinant( void ) const;
	idMat4			Transpose( void ) const;	// returns transpose
	idMat4 &		TransposeSelf( void );
	idMat4			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	idMat4			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero
	idMat4			TransposeMultiply( const idMat4 &b ) const;

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	idVec4			mat[ 4 ];
};

extern idMat4 mat4_zero;
extern idMat4 mat4_identity;
#define mat4_default	mat4_identity

ID_INLINE idMat4::idMat4( void ) {
}

ID_INLINE idMat4::idMat4( const idVec4 &x, const idVec4 &y, const idVec4 &z, const idVec4 &w ) {
	mat[ 0 ] = x;
	mat[ 1 ] = y;
	mat[ 2 ] = z;
	mat[ 3 ] = w;
}

ID_INLINE idMat4::idMat4( const float xx, const float xy, const float xz, const float xw,
							const float yx, const float yy, const float yz, const float yw,
							const float zx, const float zy, const float zz, const float zw,
							const float wx, const float wy, const float wz, const float ww ) {
	mat[0][0] = xx; mat[0][1] = xy; mat[0][2] = xz; mat[0][3] = xw;
	mat[1][0] = yx; mat[1][1] = yy; mat[1][2] = yz; mat[1][3] = yw;
	mat[2][0] = zx; mat[2][1] = zy; mat[2][2] = zz; mat[2][3] = zw;
	mat[3][0] = wx; mat[3][1] = wy; mat[3][2] = wz; mat[3][3] = ww;
}

ID_INLINE idMat4::idMat4( const idMat3 &rotation, const idVec3 &translation ) {
	// NOTE: idMat3 is transposed because it is column-major
	mat[ 0 ][ 0 ] = rotation[0][0];
	mat[ 0 ][ 1 ] = rotation[1][0];
	mat[ 0 ][ 2 ] = rotation[2][0];
	mat[ 0 ][ 3 ] = translation[0];
	mat[ 1 ][ 0 ] = rotation[0][1];
	mat[ 1 ][ 1 ] = rotation[1][1];
	mat[ 1 ][ 2 ] = rotation[2][1];
	mat[ 1 ][ 3 ] = translation[1];
	mat[ 2 ][ 0 ] = rotation[0][2];
	mat[ 2 ][ 1 ] = rotation[1][2];
	mat[ 2 ][ 2 ] = rotation[2][2];
	mat[ 2 ][ 3 ] = translation[2];
	mat[ 3 ][ 0 ] = 0.0f;
	mat[ 3 ][ 1 ] = 0.0f;
	mat[ 3 ][ 2 ] = 0.0f;
	mat[ 3 ][ 3 ] = 1.0f;
}

ID_INLINE idMat4::idMat4( const float src[ 4 ][ 4 ] ) {
	memcpy( mat, src, 4 * 4 * sizeof( float ) );
}

ID_INLINE const idVec4 &idMat4::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 4 ) );
	return mat[ index ];
}

ID_INLINE idVec4 &idMat4::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 4 ) );
	return mat[ index ];
}

ID_INLINE idMat4 idMat4::operator*( const float a ) const {
	return idMat4(
		mat[0].x * a, mat[0].y * a, mat[0].z * a, mat[0].w * a,
		mat[1].x * a, mat[1].y * a, mat[1].z * a, mat[1].w * a,
		mat[2].x * a, mat[2].y * a, mat[2].z * a, mat[2].w * a,
		mat[3].x * a, mat[3].y * a, mat[3].z * a, mat[3].w * a );
}

ID_INLINE idVec4 idMat4::operator*( const idVec4 &vec ) const {
	return idVec4(
		mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w * vec.w,
		mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w * vec.w,
		mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w * vec.w,
		mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].w * vec.w );
}

ID_INLINE idVec3 idMat4::operator*( const idVec3 &vec ) const {
	float s = mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].w;
	if ( s == 0.0f ) {
		return idVec3( 0.0f, 0.0f, 0.0f );
	}
	if ( s == 1.0f ) {
		return idVec3(
			mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w,
			mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w,
			mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w );
	}
	else {
		float invS = 1.0f / s;
		return idVec3(
			(mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w) * invS,
			(mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w) * invS,
			(mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w) * invS );
	}
}

ID_INLINE idMat4 idMat4::operator*( const idMat4 &a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	idMat4 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 4; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 4 + j ]
					+ m1Ptr[1] * m2Ptr[ 1 * 4 + j ]
					+ m1Ptr[2] * m2Ptr[ 2 * 4 + j ]
					+ m1Ptr[3] * m2Ptr[ 3 * 4 + j ];
			dstPtr++;
		}
		m1Ptr += 4;
	}
	return dst;
}

ID_INLINE idMat4 idMat4::operator+( const idMat4 &a ) const {
	return idMat4( 
		mat[0].x + a[0].x, mat[0].y + a[0].y, mat[0].z + a[0].z, mat[0].w + a[0].w,
		mat[1].x + a[1].x, mat[1].y + a[1].y, mat[1].z + a[1].z, mat[1].w + a[1].w,
		mat[2].x + a[2].x, mat[2].y + a[2].y, mat[2].z + a[2].z, mat[2].w + a[2].w,
		mat[3].x + a[3].x, mat[3].y + a[3].y, mat[3].z + a[3].z, mat[3].w + a[3].w );
}
    
ID_INLINE idMat4 idMat4::operator-( const idMat4 &a ) const {
	return idMat4( 
		mat[0].x - a[0].x, mat[0].y - a[0].y, mat[0].z - a[0].z, mat[0].w - a[0].w,
		mat[1].x - a[1].x, mat[1].y - a[1].y, mat[1].z - a[1].z, mat[1].w - a[1].w,
		mat[2].x - a[2].x, mat[2].y - a[2].y, mat[2].z - a[2].z, mat[2].w - a[2].w,
		mat[3].x - a[3].x, mat[3].y - a[3].y, mat[3].z - a[3].z, mat[3].w - a[3].w );
}

ID_INLINE idMat4 &idMat4::operator*=( const float a ) {
	mat[0].x *= a; mat[0].y *= a; mat[0].z *= a; mat[0].w *= a;
	mat[1].x *= a; mat[1].y *= a; mat[1].z *= a; mat[1].w *= a;
	mat[2].x *= a; mat[2].y *= a; mat[2].z *= a; mat[2].w *= a;
	mat[3].x *= a; mat[3].y *= a; mat[3].z *= a; mat[3].w *= a;
    return *this;
}

ID_INLINE idMat4 &idMat4::operator*=( const idMat4 &a ) {
	*this = (*this) * a;
	return *this;
}

ID_INLINE idMat4 &idMat4::operator+=( const idMat4 &a ) {
	mat[0].x += a[0].x; mat[0].y += a[0].y; mat[0].z += a[0].z; mat[0].w += a[0].w;
	mat[1].x += a[1].x; mat[1].y += a[1].y; mat[1].z += a[1].z; mat[1].w += a[1].w;
	mat[2].x += a[2].x; mat[2].y += a[2].y; mat[2].z += a[2].z; mat[2].w += a[2].w;
	mat[3].x += a[3].x; mat[3].y += a[3].y; mat[3].z += a[3].z; mat[3].w += a[3].w;
    return *this;
}

ID_INLINE idMat4 &idMat4::operator-=( const idMat4 &a ) {
	mat[0].x -= a[0].x; mat[0].y -= a[0].y; mat[0].z -= a[0].z; mat[0].w -= a[0].w;
	mat[1].x -= a[1].x; mat[1].y -= a[1].y; mat[1].z -= a[1].z; mat[1].w -= a[1].w;
	mat[2].x -= a[2].x; mat[2].y -= a[2].y; mat[2].z -= a[2].z; mat[2].w -= a[2].w;
	mat[3].x -= a[3].x; mat[3].y -= a[3].y; mat[3].z -= a[3].z; mat[3].w -= a[3].w;
    return *this;
}

ID_INLINE idMat4 operator*( const float a, const idMat4 &mat ) {
	return mat * a;
}

ID_INLINE idVec4 operator*( const idVec4 &vec, const idMat4 &mat ) {
	return mat * vec;
}

ID_INLINE idVec3 operator*( const idVec3 &vec, const idMat4 &mat ) {
	return mat * vec;
}

ID_INLINE idVec4 &operator*=( idVec4 &vec, const idMat4 &mat ) {
	vec = mat * vec;
	return vec;
}

ID_INLINE idVec3 &operator*=( idVec3 &vec, const idMat4 &mat ) {
	vec = mat * vec;
	return vec;
}

ID_INLINE bool idMat4::Compare( const idMat4 &a ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 4*4; i++ ) {
		if ( ptr1[i] != ptr2[i] ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat4::Compare( const idMat4 &a, const float epsilon ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 4*4; i++ ) {
		if ( idMath::Fabs( ptr1[i] - ptr2[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat4::operator==( const idMat4 &a ) const {
	return Compare( a );
}

ID_INLINE bool idMat4::operator!=( const idMat4 &a ) const {
	return !Compare( a );
}

ID_INLINE void idMat4::Zero( void ) {
	memset( mat, 0, sizeof( idMat4 ) );
}

ID_INLINE void idMat4::Identity( void ) {
	*this = mat4_identity;
}

ID_INLINE bool idMat4::IsIdentity( const float epsilon ) const {
	return Compare( mat4_identity, epsilon );
}

ID_INLINE bool idMat4::IsSymmetric( const float epsilon ) const {
	for ( int i = 1; i < 4; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			if ( idMath::Fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMat4::IsDiagonal( const float epsilon ) const {
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			if ( i != j && idMath::Fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMat4::IsRotated( void ) const {
	if ( !mat[ 0 ][ 1 ] && !mat[ 0 ][ 2 ] &&
		!mat[ 1 ][ 0 ] && !mat[ 1 ][ 2 ] &&
		!mat[ 2 ][ 0 ] && !mat[ 2 ][ 1 ] ) {
		return false;
	}
	return true;
}

ID_INLINE void idMat4::ProjectVector( const idVec4 &src, idVec4 &dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
	dst.w = src * mat[ 3 ];
}

ID_INLINE void idMat4::UnprojectVector( const idVec4 &src, idVec4 &dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z + mat[ 3 ] * src.w;
}

ID_INLINE float idMat4::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] + mat[3][3] );
}

ID_INLINE idMat4 idMat4::Inverse( void ) const {
	idMat4 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

ID_INLINE idMat4 idMat4::InverseFast( void ) const {
	idMat4 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

ID_INLINE idMat4 idMat3::ToMat4( void ) const {
	// NOTE: idMat3 is transposed because it is column-major
	return idMat4(	mat[0][0],	mat[1][0],	mat[2][0],	0.0f,
					mat[0][1],	mat[1][1],	mat[2][1],	0.0f,
					mat[0][2],	mat[1][2],	mat[2][2],	0.0f,
					0.0f,		0.0f,		0.0f,		1.0f );
}

ID_INLINE int idMat4::GetDimension( void ) const {
	return 16;
}

ID_INLINE const float *idMat4::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

ID_INLINE float *idMat4::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}


//===============================================================
//
//	idMat5 - 5x5 matrix
//
//===============================================================

class idMat5 {
public:
					idMat5( void );
					explicit idMat5( const idVec5 &v0, const idVec5 &v1, const idVec5 &v2, const idVec5 &v3, const idVec5 &v4 );
					explicit idMat5( const float src[ 5 ][ 5 ] );

	const idVec5 &	operator[]( int index ) const;
	idVec5 &		operator[]( int index );
	idMat5			operator*( const float a ) const;
	idVec5			operator*( const idVec5 &vec ) const;
	idMat5			operator*( const idMat5 &a ) const;
	idMat5			operator+( const idMat5 &a ) const;
	idMat5			operator-( const idMat5 &a ) const;
	idMat5 &		operator*=( const float a );
	idMat5 &		operator*=( const idMat5 &a );
	idMat5 &		operator+=( const idMat5 &a );
	idMat5 &		operator-=( const idMat5 &a );

	friend idMat5	operator*( const float a, const idMat5 &mat );
	friend idVec5	operator*( const idVec5 &vec, const idMat5 &mat );
	friend idVec5 &	operator*=( idVec5 &vec, const idMat5 &mat );

	bool			Compare( const idMat5 &a ) const;						// exact compare, no epsilon
	bool			Compare( const idMat5 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const idMat5 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const idMat5 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;

	float			Trace( void ) const;
	float			Determinant( void ) const;
	idMat5			Transpose( void ) const;	// returns transpose
	idMat5 &		TransposeSelf( void );
	idMat5			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	idMat5			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	idVec5			mat[ 5 ];
};

extern idMat5 mat5_zero;
extern idMat5 mat5_identity;
#define mat5_default	mat5_identity

ID_INLINE idMat5::idMat5( void ) {
}

ID_INLINE idMat5::idMat5( const float src[ 5 ][ 5 ] ) {
	memcpy( mat, src, 5 * 5 * sizeof( float ) );
}

ID_INLINE idMat5::idMat5( const idVec5 &v0, const idVec5 &v1, const idVec5 &v2, const idVec5 &v3, const idVec5 &v4 ) {
	mat[0] = v0;
	mat[1] = v1;
	mat[2] = v2;
	mat[3] = v3;
	mat[4] = v4;
}

ID_INLINE const idVec5 &idMat5::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 5 ) );
	return mat[ index ];
}

ID_INLINE idVec5 &idMat5::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 5 ) );
	return mat[ index ];
}

ID_INLINE idMat5 idMat5::operator*( const idMat5 &a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	idMat5 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for ( i = 0; i < 5; i++ ) {
		for ( j = 0; j < 5; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 5 + j ]
					+ m1Ptr[1] * m2Ptr[ 1 * 5 + j ]
					+ m1Ptr[2] * m2Ptr[ 2 * 5 + j ]
					+ m1Ptr[3] * m2Ptr[ 3 * 5 + j ]
					+ m1Ptr[4] * m2Ptr[ 4 * 5 + j ];
			dstPtr++;
		}
		m1Ptr += 5;
	}
	return dst;
}

ID_INLINE idMat5 idMat5::operator*( const float a ) const {
	return idMat5(
		idVec5( mat[0][0] * a, mat[0][1] * a, mat[0][2] * a, mat[0][3] * a, mat[0][4] * a ),
		idVec5( mat[1][0] * a, mat[1][1] * a, mat[1][2] * a, mat[1][3] * a, mat[1][4] * a ),
		idVec5( mat[2][0] * a, mat[2][1] * a, mat[2][2] * a, mat[2][3] * a, mat[2][4] * a ),
		idVec5( mat[3][0] * a, mat[3][1] * a, mat[3][2] * a, mat[3][3] * a, mat[3][4] * a ),
		idVec5( mat[4][0] * a, mat[4][1] * a, mat[4][2] * a, mat[4][3] * a, mat[4][4] * a ) );
}

ID_INLINE idVec5 idMat5::operator*( const idVec5 &vec ) const {
	return idVec5(
		mat[0][0] * vec[0] + mat[0][1] * vec[1] + mat[0][2] * vec[2] + mat[0][3] * vec[3] + mat[0][4] * vec[4],
		mat[1][0] * vec[0] + mat[1][1] * vec[1] + mat[1][2] * vec[2] + mat[1][3] * vec[3] + mat[1][4] * vec[4],
		mat[2][0] * vec[0] + mat[2][1] * vec[1] + mat[2][2] * vec[2] + mat[2][3] * vec[3] + mat[2][4] * vec[4],
		mat[3][0] * vec[0] + mat[3][1] * vec[1] + mat[3][2] * vec[2] + mat[3][3] * vec[3] + mat[3][4] * vec[4],
		mat[4][0] * vec[0] + mat[4][1] * vec[1] + mat[4][2] * vec[2] + mat[4][3] * vec[3] + mat[4][4] * vec[4] );
}

ID_INLINE idMat5 idMat5::operator+( const idMat5 &a ) const {
	return idMat5(
		idVec5( mat[0][0] + a[0][0], mat[0][1] + a[0][1], mat[0][2] + a[0][2], mat[0][3] + a[0][3], mat[0][4] + a[0][4] ),
		idVec5( mat[1][0] + a[1][0], mat[1][1] + a[1][1], mat[1][2] + a[1][2], mat[1][3] + a[1][3], mat[1][4] + a[1][4] ),
		idVec5( mat[2][0] + a[2][0], mat[2][1] + a[2][1], mat[2][2] + a[2][2], mat[2][3] + a[2][3], mat[2][4] + a[2][4] ),
		idVec5( mat[3][0] + a[3][0], mat[3][1] + a[3][1], mat[3][2] + a[3][2], mat[3][3] + a[3][3], mat[3][4] + a[3][4] ),
		idVec5( mat[4][0] + a[4][0], mat[4][1] + a[4][1], mat[4][2] + a[4][2], mat[4][3] + a[4][3], mat[4][4] + a[4][4] ) );
}

ID_INLINE idMat5 idMat5::operator-( const idMat5 &a ) const {
	return idMat5(
		idVec5( mat[0][0] - a[0][0], mat[0][1] - a[0][1], mat[0][2] - a[0][2], mat[0][3] - a[0][3], mat[0][4] - a[0][4] ),
		idVec5( mat[1][0] - a[1][0], mat[1][1] - a[1][1], mat[1][2] - a[1][2], mat[1][3] - a[1][3], mat[1][4] - a[1][4] ),
		idVec5( mat[2][0] - a[2][0], mat[2][1] - a[2][1], mat[2][2] - a[2][2], mat[2][3] - a[2][3], mat[2][4] - a[2][4] ),
		idVec5( mat[3][0] - a[3][0], mat[3][1] - a[3][1], mat[3][2] - a[3][2], mat[3][3] - a[3][3], mat[3][4] - a[3][4] ),
		idVec5( mat[4][0] - a[4][0], mat[4][1] - a[4][1], mat[4][2] - a[4][2], mat[4][3] - a[4][3], mat[4][4] - a[4][4] ) );
}

ID_INLINE idMat5 &idMat5::operator*=( const float a ) {
	mat[0][0] *= a; mat[0][1] *= a; mat[0][2] *= a; mat[0][3] *= a; mat[0][4] *= a;
	mat[1][0] *= a; mat[1][1] *= a; mat[1][2] *= a; mat[1][3] *= a; mat[1][4] *= a;
	mat[2][0] *= a; mat[2][1] *= a; mat[2][2] *= a; mat[2][3] *= a; mat[2][4] *= a;
	mat[3][0] *= a; mat[3][1] *= a; mat[3][2] *= a; mat[3][3] *= a; mat[3][4] *= a;
	mat[4][0] *= a; mat[4][1] *= a; mat[4][2] *= a; mat[4][3] *= a; mat[4][4] *= a;
	return *this;
}

ID_INLINE idMat5 &idMat5::operator*=( const idMat5 &a ) {
	*this = *this * a;
	return *this;
}

ID_INLINE idMat5 &idMat5::operator+=( const idMat5 &a ) {
	mat[0][0] += a[0][0]; mat[0][1] += a[0][1]; mat[0][2] += a[0][2]; mat[0][3] += a[0][3]; mat[0][4] += a[0][4];
	mat[1][0] += a[1][0]; mat[1][1] += a[1][1]; mat[1][2] += a[1][2]; mat[1][3] += a[1][3]; mat[1][4] += a[1][4];
	mat[2][0] += a[2][0]; mat[2][1] += a[2][1]; mat[2][2] += a[2][2]; mat[2][3] += a[2][3]; mat[2][4] += a[2][4];
	mat[3][0] += a[3][0]; mat[3][1] += a[3][1]; mat[3][2] += a[3][2]; mat[3][3] += a[3][3]; mat[3][4] += a[3][4];
	mat[4][0] += a[4][0]; mat[4][1] += a[4][1]; mat[4][2] += a[4][2]; mat[4][3] += a[4][3]; mat[4][4] += a[4][4];
	return *this;
}

ID_INLINE idMat5 &idMat5::operator-=( const idMat5 &a ) {
	mat[0][0] -= a[0][0]; mat[0][1] -= a[0][1]; mat[0][2] -= a[0][2]; mat[0][3] -= a[0][3]; mat[0][4] -= a[0][4];
	mat[1][0] -= a[1][0]; mat[1][1] -= a[1][1]; mat[1][2] -= a[1][2]; mat[1][3] -= a[1][3]; mat[1][4] -= a[1][4];
	mat[2][0] -= a[2][0]; mat[2][1] -= a[2][1]; mat[2][2] -= a[2][2]; mat[2][3] -= a[2][3]; mat[2][4] -= a[2][4];
	mat[3][0] -= a[3][0]; mat[3][1] -= a[3][1]; mat[3][2] -= a[3][2]; mat[3][3] -= a[3][3]; mat[3][4] -= a[3][4];
	mat[4][0] -= a[4][0]; mat[4][1] -= a[4][1]; mat[4][2] -= a[4][2]; mat[4][3] -= a[4][3]; mat[4][4] -= a[4][4];
	return *this;
}

ID_INLINE idVec5 operator*( const idVec5 &vec, const idMat5 &mat ) {
	return mat * vec;
}

ID_INLINE idMat5 operator*( const float a, idMat5 const &mat ) {
	return mat * a;
}

ID_INLINE idVec5 &operator*=( idVec5 &vec, const idMat5 &mat ) {
	vec = mat * vec;
	return vec;
}

ID_INLINE bool idMat5::Compare( const idMat5 &a ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 5*5; i++ ) {
		if ( ptr1[i] != ptr2[i] ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat5::Compare( const idMat5 &a, const float epsilon ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 5*5; i++ ) {
		if ( idMath::Fabs( ptr1[i] - ptr2[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat5::operator==( const idMat5 &a ) const {
	return Compare( a );
}

ID_INLINE bool idMat5::operator!=( const idMat5 &a ) const {
	return !Compare( a );
}

ID_INLINE void idMat5::Zero( void ) {
	memset( mat, 0, sizeof( idMat5 ) );
}

ID_INLINE void idMat5::Identity( void ) {
	*this = mat5_identity;
}

ID_INLINE bool idMat5::IsIdentity( const float epsilon ) const {
	return Compare( mat5_identity, epsilon );
}

ID_INLINE bool idMat5::IsSymmetric( const float epsilon ) const {
	for ( int i = 1; i < 5; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			if ( idMath::Fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMat5::IsDiagonal( const float epsilon ) const {
	for ( int i = 0; i < 5; i++ ) {
		for ( int j = 0; j < 5; j++ ) {
			if ( i != j && idMath::Fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE float idMat5::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] + mat[3][3] + mat[4][4] );
}

ID_INLINE idMat5 idMat5::Inverse( void ) const {
	idMat5 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

ID_INLINE idMat5 idMat5::InverseFast( void ) const {
	idMat5 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

ID_INLINE int idMat5::GetDimension( void ) const {
	return 25;
}

ID_INLINE const float *idMat5::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

ID_INLINE float *idMat5::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}


//===============================================================
//
//	idMat6 - 6x6 matrix
//
//===============================================================

class idMat6 {
public:
					idMat6( void );
					explicit idMat6( const idVec6 &v0, const idVec6 &v1, const idVec6 &v2, const idVec6 &v3, const idVec6 &v4, const idVec6 &v5 );
					explicit idMat6( const idMat3 &m0, const idMat3 &m1, const idMat3 &m2, const idMat3 &m3 );
					explicit idMat6( const float src[ 6 ][ 6 ] );

	const idVec6 &	operator[]( int index ) const;
	idVec6 &		operator[]( int index );
	idMat6			operator*( const float a ) const;
	idVec6			operator*( const idVec6 &vec ) const;
	idMat6			operator*( const idMat6 &a ) const;
	idMat6			operator+( const idMat6 &a ) const;
	idMat6			operator-( const idMat6 &a ) const;
	idMat6 &		operator*=( const float a );
	idMat6 &		operator*=( const idMat6 &a );
	idMat6 &		operator+=( const idMat6 &a );
	idMat6 &		operator-=( const idMat6 &a );

	friend idMat6	operator*( const float a, const idMat6 &mat );
	friend idVec6	operator*( const idVec6 &vec, const idMat6 &mat );
	friend idVec6 &	operator*=( idVec6 &vec, const idMat6 &mat );

	bool			Compare( const idMat6 &a ) const;						// exact compare, no epsilon
	bool			Compare( const idMat6 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const idMat6 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const idMat6 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;

	idMat3			SubMat3( int n ) const;
	float			Trace( void ) const;
	float			Determinant( void ) const;
	idMat6			Transpose( void ) const;	// returns transpose
	idMat6 &		TransposeSelf( void );
	idMat6			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	idMat6			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	idVec6			mat[ 6 ];
};

extern idMat6 mat6_zero;
extern idMat6 mat6_identity;
#define mat6_default	mat6_identity

ID_INLINE idMat6::idMat6( void ) {
}

ID_INLINE idMat6::idMat6( const idMat3 &m0, const idMat3 &m1, const idMat3 &m2, const idMat3 &m3 ) {
	mat[0] = idVec6( m0[0][0], m0[0][1], m0[0][2], m1[0][0], m1[0][1], m1[0][2] );
	mat[1] = idVec6( m0[1][0], m0[1][1], m0[1][2], m1[1][0], m1[1][1], m1[1][2] );
	mat[2] = idVec6( m0[2][0], m0[2][1], m0[2][2], m1[2][0], m1[2][1], m1[2][2] );
	mat[3] = idVec6( m2[0][0], m2[0][1], m2[0][2], m3[0][0], m3[0][1], m3[0][2] );
	mat[4] = idVec6( m2[1][0], m2[1][1], m2[1][2], m3[1][0], m3[1][1], m3[1][2] );
	mat[5] = idVec6( m2[2][0], m2[2][1], m2[2][2], m3[2][0], m3[2][1], m3[2][2] );
}

ID_INLINE idMat6::idMat6( const idVec6 &v0, const idVec6 &v1, const idVec6 &v2, const idVec6 &v3, const idVec6 &v4, const idVec6 &v5 ) {
	mat[0] = v0;
	mat[1] = v1;
	mat[2] = v2;
	mat[3] = v3;
	mat[4] = v4;
	mat[5] = v5;
}

ID_INLINE idMat6::idMat6( const float src[ 6 ][ 6 ] ) {
	memcpy( mat, src, 6 * 6 * sizeof( float ) );
}

ID_INLINE const idVec6 &idMat6::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 6 ) );
	return mat[ index ];
}

ID_INLINE idVec6 &idMat6::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 6 ) );
	return mat[ index ];
}

ID_INLINE idMat6 idMat6::operator*( const idMat6 &a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	idMat6 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for ( i = 0; i < 6; i++ ) {
		for ( j = 0; j < 6; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 6 + j ]
					+ m1Ptr[1] * m2Ptr[ 1 * 6 + j ]
					+ m1Ptr[2] * m2Ptr[ 2 * 6 + j ]
					+ m1Ptr[3] * m2Ptr[ 3 * 6 + j ]
					+ m1Ptr[4] * m2Ptr[ 4 * 6 + j ]
					+ m1Ptr[5] * m2Ptr[ 5 * 6 + j ];
			dstPtr++;
		}
		m1Ptr += 6;
	}
	return dst;
}

ID_INLINE idMat6 idMat6::operator*( const float a ) const {
	return idMat6(
		idVec6( mat[0][0] * a, mat[0][1] * a, mat[0][2] * a, mat[0][3] * a, mat[0][4] * a, mat[0][5] * a ),
		idVec6( mat[1][0] * a, mat[1][1] * a, mat[1][2] * a, mat[1][3] * a, mat[1][4] * a, mat[1][5] * a ),
		idVec6( mat[2][0] * a, mat[2][1] * a, mat[2][2] * a, mat[2][3] * a, mat[2][4] * a, mat[2][5] * a ),
		idVec6( mat[3][0] * a, mat[3][1] * a, mat[3][2] * a, mat[3][3] * a, mat[3][4] * a, mat[3][5] * a ),
		idVec6( mat[4][0] * a, mat[4][1] * a, mat[4][2] * a, mat[4][3] * a, mat[4][4] * a, mat[4][5] * a ),
		idVec6( mat[5][0] * a, mat[5][1] * a, mat[5][2] * a, mat[5][3] * a, mat[5][4] * a, mat[5][5] * a ) );
}

ID_INLINE idVec6 idMat6::operator*( const idVec6 &vec ) const {
	return idVec6(
		mat[0][0] * vec[0] + mat[0][1] * vec[1] + mat[0][2] * vec[2] + mat[0][3] * vec[3] + mat[0][4] * vec[4] + mat[0][5] * vec[5],
		mat[1][0] * vec[0] + mat[1][1] * vec[1] + mat[1][2] * vec[2] + mat[1][3] * vec[3] + mat[1][4] * vec[4] + mat[1][5] * vec[5],
		mat[2][0] * vec[0] + mat[2][1] * vec[1] + mat[2][2] * vec[2] + mat[2][3] * vec[3] + mat[2][4] * vec[4] + mat[2][5] * vec[5],
		mat[3][0] * vec[0] + mat[3][1] * vec[1] + mat[3][2] * vec[2] + mat[3][3] * vec[3] + mat[3][4] * vec[4] + mat[3][5] * vec[5],
		mat[4][0] * vec[0] + mat[4][1] * vec[1] + mat[4][2] * vec[2] + mat[4][3] * vec[3] + mat[4][4] * vec[4] + mat[4][5] * vec[5],
		mat[5][0] * vec[0] + mat[5][1] * vec[1] + mat[5][2] * vec[2] + mat[5][3] * vec[3] + mat[5][4] * vec[4] + mat[5][5] * vec[5] );
}

ID_INLINE idMat6 idMat6::operator+( const idMat6 &a ) const {
	return idMat6(
		idVec6( mat[0][0] + a[0][0], mat[0][1] + a[0][1], mat[0][2] + a[0][2], mat[0][3] + a[0][3], mat[0][4] + a[0][4], mat[0][5] + a[0][5] ),
		idVec6( mat[1][0] + a[1][0], mat[1][1] + a[1][1], mat[1][2] + a[1][2], mat[1][3] + a[1][3], mat[1][4] + a[1][4], mat[1][5] + a[1][5] ),
		idVec6( mat[2][0] + a[2][0], mat[2][1] + a[2][1], mat[2][2] + a[2][2], mat[2][3] + a[2][3], mat[2][4] + a[2][4], mat[2][5] + a[2][5] ),
		idVec6( mat[3][0] + a[3][0], mat[3][1] + a[3][1], mat[3][2] + a[3][2], mat[3][3] + a[3][3], mat[3][4] + a[3][4], mat[3][5] + a[3][5] ),
		idVec6( mat[4][0] + a[4][0], mat[4][1] + a[4][1], mat[4][2] + a[4][2], mat[4][3] + a[4][3], mat[4][4] + a[4][4], mat[4][5] + a[4][5] ),
		idVec6( mat[5][0] + a[5][0], mat[5][1] + a[5][1], mat[5][2] + a[5][2], mat[5][3] + a[5][3], mat[5][4] + a[5][4], mat[5][5] + a[5][5] ) );
}

ID_INLINE idMat6 idMat6::operator-( const idMat6 &a ) const {
	return idMat6(
		idVec6( mat[0][0] - a[0][0], mat[0][1] - a[0][1], mat[0][2] - a[0][2], mat[0][3] - a[0][3], mat[0][4] - a[0][4], mat[0][5] - a[0][5] ),
		idVec6( mat[1][0] - a[1][0], mat[1][1] - a[1][1], mat[1][2] - a[1][2], mat[1][3] - a[1][3], mat[1][4] - a[1][4], mat[1][5] - a[1][5] ),
		idVec6( mat[2][0] - a[2][0], mat[2][1] - a[2][1], mat[2][2] - a[2][2], mat[2][3] - a[2][3], mat[2][4] - a[2][4], mat[2][5] - a[2][5] ),
		idVec6( mat[3][0] - a[3][0], mat[3][1] - a[3][1], mat[3][2] - a[3][2], mat[3][3] - a[3][3], mat[3][4] - a[3][4], mat[3][5] - a[3][5] ),
		idVec6( mat[4][0] - a[4][0], mat[4][1] - a[4][1], mat[4][2] - a[4][2], mat[4][3] - a[4][3], mat[4][4] - a[4][4], mat[4][5] - a[4][5] ),
		idVec6( mat[5][0] - a[5][0], mat[5][1] - a[5][1], mat[5][2] - a[5][2], mat[5][3] - a[5][3], mat[5][4] - a[5][4], mat[5][5] - a[5][5] ) );
}

ID_INLINE idMat6 &idMat6::operator*=( const float a ) {
	mat[0][0] *= a; mat[0][1] *= a; mat[0][2] *= a; mat[0][3] *= a; mat[0][4] *= a; mat[0][5] *= a;
	mat[1][0] *= a; mat[1][1] *= a; mat[1][2] *= a; mat[1][3] *= a; mat[1][4] *= a; mat[1][5] *= a;
	mat[2][0] *= a; mat[2][1] *= a; mat[2][2] *= a; mat[2][3] *= a; mat[2][4] *= a; mat[2][5] *= a;
	mat[3][0] *= a; mat[3][1] *= a; mat[3][2] *= a; mat[3][3] *= a; mat[3][4] *= a; mat[3][5] *= a;
	mat[4][0] *= a; mat[4][1] *= a; mat[4][2] *= a; mat[4][3] *= a; mat[4][4] *= a; mat[4][5] *= a;
	mat[5][0] *= a; mat[5][1] *= a; mat[5][2] *= a; mat[5][3] *= a; mat[5][4] *= a; mat[5][5] *= a;
	return *this;
}

ID_INLINE idMat6 &idMat6::operator*=( const idMat6 &a ) {
	*this = *this * a;
	return *this;
}

ID_INLINE idMat6 &idMat6::operator+=( const idMat6 &a ) {
	mat[0][0] += a[0][0]; mat[0][1] += a[0][1]; mat[0][2] += a[0][2]; mat[0][3] += a[0][3]; mat[0][4] += a[0][4]; mat[0][5] += a[0][5];
	mat[1][0] += a[1][0]; mat[1][1] += a[1][1]; mat[1][2] += a[1][2]; mat[1][3] += a[1][3]; mat[1][4] += a[1][4]; mat[1][5] += a[1][5];
	mat[2][0] += a[2][0]; mat[2][1] += a[2][1]; mat[2][2] += a[2][2]; mat[2][3] += a[2][3]; mat[2][4] += a[2][4]; mat[2][5] += a[2][5];
	mat[3][0] += a[3][0]; mat[3][1] += a[3][1]; mat[3][2] += a[3][2]; mat[3][3] += a[3][3]; mat[3][4] += a[3][4]; mat[3][5] += a[3][5];
	mat[4][0] += a[4][0]; mat[4][1] += a[4][1]; mat[4][2] += a[4][2]; mat[4][3] += a[4][3]; mat[4][4] += a[4][4]; mat[4][5] += a[4][5];
	mat[5][0] += a[5][0]; mat[5][1] += a[5][1]; mat[5][2] += a[5][2]; mat[5][3] += a[5][3]; mat[5][4] += a[5][4]; mat[5][5] += a[5][5];
	return *this;
}

ID_INLINE idMat6 &idMat6::operator-=( const idMat6 &a ) {
	mat[0][0] -= a[0][0]; mat[0][1] -= a[0][1]; mat[0][2] -= a[0][2]; mat[0][3] -= a[0][3]; mat[0][4] -= a[0][4]; mat[0][5] -= a[0][5];
	mat[1][0] -= a[1][0]; mat[1][1] -= a[1][1]; mat[1][2] -= a[1][2]; mat[1][3] -= a[1][3]; mat[1][4] -= a[1][4]; mat[1][5] -= a[1][5];
	mat[2][0] -= a[2][0]; mat[2][1] -= a[2][1]; mat[2][2] -= a[2][2]; mat[2][3] -= a[2][3]; mat[2][4] -= a[2][4]; mat[2][5] -= a[2][5];
	mat[3][0] -= a[3][0]; mat[3][1] -= a[3][1]; mat[3][2] -= a[3][2]; mat[3][3] -= a[3][3]; mat[3][4] -= a[3][4]; mat[3][5] -= a[3][5];
	mat[4][0] -= a[4][0]; mat[4][1] -= a[4][1]; mat[4][2] -= a[4][2]; mat[4][3] -= a[4][3]; mat[4][4] -= a[4][4]; mat[4][5] -= a[4][5];
	mat[5][0] -= a[5][0]; mat[5][1] -= a[5][1]; mat[5][2] -= a[5][2]; mat[5][3] -= a[5][3]; mat[5][4] -= a[5][4]; mat[5][5] -= a[5][5];
	return *this;
}

ID_INLINE idVec6 operator*( const idVec6 &vec, const idMat6 &mat ) {
	return mat * vec;
}

ID_INLINE idMat6 operator*( const float a, idMat6 const &mat ) {
	return mat * a;
}

ID_INLINE idVec6 &operator*=( idVec6 &vec, const idMat6 &mat ) {
	vec = mat * vec;
	return vec;
}

ID_INLINE bool idMat6::Compare( const idMat6 &a ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 6*6; i++ ) {
		if ( ptr1[i] != ptr2[i] ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat6::Compare( const idMat6 &a, const float epsilon ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 6*6; i++ ) {
		if ( idMath::Fabs( ptr1[i] - ptr2[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat6::operator==( const idMat6 &a ) const {
	return Compare( a );
}

ID_INLINE bool idMat6::operator!=( const idMat6 &a ) const {
	return !Compare( a );
}

ID_INLINE void idMat6::Zero( void ) {
	memset( mat, 0, sizeof( idMat6 ) );
}

ID_INLINE void idMat6::Identity( void ) {
	*this = mat6_identity;
}

ID_INLINE bool idMat6::IsIdentity( const float epsilon ) const {
	return Compare( mat6_identity, epsilon );
}

ID_INLINE bool idMat6::IsSymmetric( const float epsilon ) const {
	for ( int i = 1; i < 6; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			if ( idMath::Fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMat6::IsDiagonal( const float epsilon ) const {
	for ( int i = 0; i < 6; i++ ) {
		for ( int j = 0; j < 6; j++ ) {
			if ( i != j && idMath::Fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE idMat3 idMat6::SubMat3( int n ) const {
	assert( n >= 0 && n < 4 );
	int b0 = ((n & 2) >> 1) * 3;
	int b1 = (n & 1) * 3;
	return idMat3(
		mat[b0 + 0][b1 + 0], mat[b0 + 0][b1 + 1], mat[b0 + 0][b1 + 2],
		mat[b0 + 1][b1 + 0], mat[b0 + 1][b1 + 1], mat[b0 + 1][b1 + 2],
		mat[b0 + 2][b1 + 0], mat[b0 + 2][b1 + 1], mat[b0 + 2][b1 + 2] );
}

ID_INLINE float idMat6::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] + mat[3][3] + mat[4][4] + mat[5][5] );
}

ID_INLINE idMat6 idMat6::Inverse( void ) const {
	idMat6 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

ID_INLINE idMat6 idMat6::InverseFast( void ) const {
	idMat6 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

ID_INLINE int idMat6::GetDimension( void ) const {
	return 36;
}

ID_INLINE const float *idMat6::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

ID_INLINE float *idMat6::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}


//===============================================================
//
//	idMatX - arbitrary sized dense real matrix
//
//  The matrix lives on 16 byte aligned and 16 byte padded memory.
//
//	NOTE: due to the temporary memory pool idMatX cannot be used by multiple threads.
//
//===============================================================

#define MATX_MAX_TEMP		1024
#define MATX_QUAD( x )		( ( ( ( x ) + 3 ) & ~3 ) * sizeof( float ) )
#define MATX_CLEAREND()		int s = numRows * numColumns; while( s < ( ( s + 3 ) & ~3 ) ) { mat[s++] = 0.0f; }
#define MATX_ALLOCA( n )	( (float *) _alloca16( MATX_QUAD( n ) ) )
#define MATX_SIMD

class idMatX {
public:
					idMatX( void );
					explicit idMatX( int rows, int columns );
					explicit idMatX( int rows, int columns, float *src );
					~idMatX( void );

	void			Set( int rows, int columns, const float *src );
	void			Set( const idMat3 &m1, const idMat3 &m2 );
	void			Set( const idMat3 &m1, const idMat3 &m2, const idMat3 &m3, const idMat3 &m4 );

	const float *	operator[]( int index ) const;
	float *			operator[]( int index );
	idMatX &		operator=( const idMatX &a );
	idMatX			operator*( const float a ) const;
	idVecX			operator*( const idVecX &vec ) const;
	idMatX			operator*( const idMatX &a ) const;
	idMatX			operator+( const idMatX &a ) const;
	idMatX			operator-( const idMatX &a ) const;
	idMatX &		operator*=( const float a );
	idMatX &		operator*=( const idMatX &a );
	idMatX &		operator+=( const idMatX &a );
	idMatX &		operator-=( const idMatX &a );

	friend idMatX	operator*( const float a, const idMatX &m );
	friend idVecX	operator*( const idVecX &vec, const idMatX &m );
	friend idVecX &	operator*=( idVecX &vec, const idMatX &m );

	bool			Compare( const idMatX &a ) const;								// exact compare, no epsilon
	bool			Compare( const idMatX &a, const float epsilon ) const;			// compare with epsilon
	bool			operator==( const idMatX &a ) const;							// exact compare, no epsilon
	bool			operator!=( const idMatX &a ) const;							// exact compare, no epsilon

	void			SetSize( int rows, int columns );								// set the number of rows/columns
	void			ChangeSize( int rows, int columns, bool makeZero = false );		// change the size keeping data intact where possible
	int				GetNumRows( void ) const { return numRows; }					// get the number of rows
	int				GetNumColumns( void ) const { return numColumns; }				// get the number of columns
	void			SetData( int rows, int columns, float *data );					// set float array pointer
	void			Zero( void );													// clear matrix
	void			Zero( int rows, int columns );									// set size and clear matrix
	void			Identity( void );												// clear to identity matrix
	void			Identity( int rows, int columns );								// set size and clear to identity matrix
	void			Diag( const idVecX &v );										// create diagonal matrix from vector
	void			Random( int seed, float l = 0.0f, float u = 1.0f );				// fill matrix with random values
	void			Random( int rows, int columns, int seed, float l = 0.0f, float u = 1.0f );
	void			Negate( void );													// (*this) = - (*this)
	void			Clamp( float min, float max );									// clamp all values
	idMatX &		SwapRows( int r1, int r2 );										// swap rows
	idMatX &		SwapColumns( int r1, int r2 );									// swap columns
	idMatX &		SwapRowsColumns( int r1, int r2 );								// swap rows and columns
	idMatX &		RemoveRow( int r );												// remove a row
	idMatX &		RemoveColumn( int r );											// remove a column
	idMatX &		RemoveRowColumn( int r );										// remove a row and column
	void			ClearUpperTriangle( void );										// clear the upper triangle
	void			ClearLowerTriangle( void );										// clear the lower triangle
	void			SquareSubMatrix( const idMatX &m, int size );					// get square sub-matrix from 0,0 to size,size
	float			MaxDifference( const idMatX &m ) const;							// return maximum element difference between this and m

	bool			IsSquare( void ) const { return ( numRows == numColumns ); }
	bool			IsZero( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsTriDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsOrthogonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsOrthonormal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPMatrix( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsZMatrix( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPositiveDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetricPositiveDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPositiveSemiDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetricPositiveSemiDefinite( const float epsilon = MATRIX_EPSILON ) const;

	float			Trace( void ) const;											// returns product of diagonal elements
	float			Determinant( void ) const;										// returns determinant of matrix
	idMatX			Transpose( void ) const;										// returns transpose
	idMatX &		TransposeSelf( void );											// transposes the matrix itself
	idMatX			Inverse( void ) const;											// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );											// returns false if determinant is zero
	idMatX			InverseFast( void ) const;										// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );										// returns false if determinant is zero

	bool			LowerTriangularInverse( void );									// in-place inversion, returns false if determinant is zero
	bool			UpperTriangularInverse( void );									// in-place inversion, returns false if determinant is zero

	idVecX			Multiply( const idVecX &vec ) const;							// (*this) * vec
	idVecX			TransposeMultiply( const idVecX &vec ) const;					// this->Transpose() * vec

	idMatX			Multiply( const idMatX &a ) const;								// (*this) * a
	idMatX			TransposeMultiply( const idMatX &a ) const;						// this->Transpose() * a

	void			Multiply( idVecX &dst, const idVecX &vec ) const;				// dst = (*this) * vec
	void			MultiplyAdd( idVecX &dst, const idVecX &vec ) const;			// dst += (*this) * vec
	void			MultiplySub( idVecX &dst, const idVecX &vec ) const;			// dst -= (*this) * vec
	void			TransposeMultiply( idVecX &dst, const idVecX &vec ) const;		// dst = this->Transpose() * vec
	void			TransposeMultiplyAdd( idVecX &dst, const idVecX &vec ) const;	// dst += this->Transpose() * vec
	void			TransposeMultiplySub( idVecX &dst, const idVecX &vec ) const;	// dst -= this->Transpose() * vec

	void			Multiply( idMatX &dst, const idMatX &a ) const;					// dst = (*this) * a
	void			TransposeMultiply( idMatX &dst, const idMatX &a ) const;		// dst = this->Transpose() * a

	int				GetDimension( void ) const;										// returns total number of values in matrix

	const idVec6 &	SubVec6( int row ) const;										// interpret beginning of row as a const idVec6
	idVec6 &		SubVec6( int row );												// interpret beginning of row as an idVec6
	const idVecX	SubVecX( int row ) const;										// interpret complete row as a const idVecX
	idVecX			SubVecX( int row );												// interpret complete row as an idVecX
	const float *	ToFloatPtr( void ) const;										// pointer to const matrix float array
	float *			ToFloatPtr( void );												// pointer to matrix float array
	const char *	ToString( int precision = 2 ) const;

	void			Update_RankOne( const idVecX &v, const idVecX &w, float alpha );
	void			Update_RankOneSymmetric( const idVecX &v, float alpha );
	void			Update_RowColumn( const idVecX &v, const idVecX &w, int r );
	void			Update_RowColumnSymmetric( const idVecX &v, int r );
	void			Update_Increment( const idVecX &v, const idVecX &w );
	void			Update_IncrementSymmetric( const idVecX &v );
	void			Update_Decrement( int r );

	bool			Inverse_GaussJordan( void );					// invert in-place with Gauss-Jordan elimination
	bool			Inverse_UpdateRankOne( const idVecX &v, const idVecX &w, float alpha );
	bool			Inverse_UpdateRowColumn( const idVecX &v, const idVecX &w, int r );
	bool			Inverse_UpdateIncrement( const idVecX &v, const idVecX &w );
	bool			Inverse_UpdateDecrement( const idVecX &v, const idVecX &w, int r );
	void			Inverse_Solve( idVecX &x, const idVecX &b ) const;

	bool			LU_Factor( int *index, float *det = NULL );		// factor in-place: L * U
	bool			LU_UpdateRankOne( const idVecX &v, const idVecX &w, float alpha, int *index );
	bool			LU_UpdateRowColumn( const idVecX &v, const idVecX &w, int r, int *index );
	bool			LU_UpdateIncrement( const idVecX &v, const idVecX &w, int *index );
	bool			LU_UpdateDecrement( const idVecX &v, const idVecX &w, const idVecX &u, int r, int *index );
	void			LU_Solve( idVecX &x, const idVecX &b, const int *index ) const;
	void			LU_Inverse( idMatX &inv, const int *index ) const;
	void			LU_UnpackFactors( idMatX &L, idMatX &U ) const;
	void			LU_MultiplyFactors( idMatX &m, const int *index ) const;

	bool			QR_Factor( idVecX &c, idVecX &d );				// factor in-place: Q * R
	bool			QR_UpdateRankOne( idMatX &R, const idVecX &v, const idVecX &w, float alpha );
	bool			QR_UpdateRowColumn( idMatX &R, const idVecX &v, const idVecX &w, int r );
	bool			QR_UpdateIncrement( idMatX &R, const idVecX &v, const idVecX &w );
	bool			QR_UpdateDecrement( idMatX &R, const idVecX &v, const idVecX &w, int r );
	void			QR_Solve( idVecX &x, const idVecX &b, const idVecX &c, const idVecX &d ) const;
	void			QR_Solve( idVecX &x, const idVecX &b, const idMatX &R ) const;
	void			QR_Inverse( idMatX &inv, const idVecX &c, const idVecX &d ) const;
	void			QR_UnpackFactors( idMatX &Q, idMatX &R, const idVecX &c, const idVecX &d ) const;
	void			QR_MultiplyFactors( idMatX &m, const idVecX &c, const idVecX &d ) const;

	bool			SVD_Factor( idVecX &w, idMatX &V );				// factor in-place: U * Diag(w) * V.Transpose()
	void			SVD_Solve( idVecX &x, const idVecX &b, const idVecX &w, const idMatX &V ) const;
	void			SVD_Inverse( idMatX &inv, const idVecX &w, const idMatX &V ) const;
	void			SVD_MultiplyFactors( idMatX &m, const idVecX &w, const idMatX &V ) const;

	bool			Cholesky_Factor( void );						// factor in-place: L * L.Transpose()
	bool			Cholesky_UpdateRankOne( const idVecX &v, float alpha, int offset = 0 );
	bool			Cholesky_UpdateRowColumn( const idVecX &v, int r );
	bool			Cholesky_UpdateIncrement( const idVecX &v );
	bool			Cholesky_UpdateDecrement( const idVecX &v, int r );
	void			Cholesky_Solve( idVecX &x, const idVecX &b ) const;
	void			Cholesky_Inverse( idMatX &inv ) const;
	void			Cholesky_MultiplyFactors( idMatX &m ) const;

	bool			LDLT_Factor( void );							// factor in-place: L * D * L.Transpose()
	bool			LDLT_UpdateRankOne( const idVecX &v, float alpha, int offset = 0 );
	bool			LDLT_UpdateRowColumn( const idVecX &v, int r );
	bool			LDLT_UpdateIncrement( const idVecX &v );
	bool			LDLT_UpdateDecrement( const idVecX &v, int r );
	void			LDLT_Solve( idVecX &x, const idVecX &b ) const;
	void			LDLT_Inverse( idMatX &inv ) const;
	void			LDLT_UnpackFactors( idMatX &L, idMatX &D ) const;
	void			LDLT_MultiplyFactors( idMatX &m ) const;

	void			TriDiagonal_ClearTriangles( void );
	bool			TriDiagonal_Solve( idVecX &x, const idVecX &b ) const;
	void			TriDiagonal_Inverse( idMatX &inv ) const;

	bool			Eigen_SolveSymmetricTriDiagonal( idVecX &eigenValues );
	bool			Eigen_SolveSymmetric( idVecX &eigenValues );
	bool			Eigen_Solve( idVecX &realEigenValues, idVecX &imaginaryEigenValues );
	void			Eigen_SortIncreasing( idVecX &eigenValues );
	void			Eigen_SortDecreasing( idVecX &eigenValues );

	static void		Test( void );

private:
	int				numRows;				// number of rows
	int				numColumns;				// number of columns
	int				alloced;				// floats allocated, if -1 then mat points to data set with SetData
	float *			mat;					// memory the matrix is stored

	static float	temp[MATX_MAX_TEMP+4];	// used to store intermediate results
	static float *	tempPtr;				// pointer to 16 byte aligned temporary memory
	static int		tempIndex;				// index into memory pool, wraps around

private:
	void			SetTempSize( int rows, int columns );
	float			DeterminantGeneric( void ) const;
	bool			InverseSelfGeneric( void );
	void			QR_Rotate( idMatX &R, int i, float a, float b );
	float			Pythag( float a, float b ) const;
	void			SVD_BiDiag( idVecX &w, idVecX &rv1, float &anorm );
	void			SVD_InitialWV( idVecX &w, idMatX &V, idVecX &rv1 );
	void			HouseholderReduction( idVecX &diag, idVecX &subd );
	bool			QL( idVecX &diag, idVecX &subd );
	void			HessenbergReduction( idMatX &H );
	void			ComplexDivision( float xr, float xi, float yr, float yi, float &cdivr, float &cdivi );
	bool			HessenbergToRealSchur( idMatX &H, idVecX &realEigenValues, idVecX &imaginaryEigenValues );
};

ID_INLINE idMatX::idMatX( void ) {
	numRows = numColumns = alloced = 0;
	mat = NULL;
}

ID_INLINE idMatX::~idMatX( void ) {
	// if not temp memory
	if ( mat != NULL && ( mat < idMatX::tempPtr || mat > idMatX::tempPtr + MATX_MAX_TEMP ) && alloced != -1 ) {
		Mem_Free16( mat );
	}
}

ID_INLINE idMatX::idMatX( int rows, int columns ) {
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetSize( rows, columns );
}

ID_INLINE idMatX::idMatX( int rows, int columns, float *src ) {
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetData( rows, columns, src );
}

ID_INLINE void idMatX::Set( int rows, int columns, const float *src ) {
	SetSize( rows, columns );
	memcpy( this->mat, src, rows * columns * sizeof( float ) );
}

ID_INLINE void idMatX::Set( const idMat3 &m1, const idMat3 &m2 ) {
	int i, j;

	SetSize( 3, 6 );
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			mat[(i+0) * numColumns + (j+0)] = m1[i][j];
			mat[(i+0) * numColumns + (j+3)] = m2[i][j];
		}
	}
}

ID_INLINE void idMatX::Set( const idMat3 &m1, const idMat3 &m2, const idMat3 &m3, const idMat3 &m4 ) {
	int i, j;

	SetSize( 6, 6 );
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			mat[(i+0) * numColumns + (j+0)] = m1[i][j];
			mat[(i+0) * numColumns + (j+3)] = m2[i][j];
			mat[(i+3) * numColumns + (j+0)] = m3[i][j];
			mat[(i+3) * numColumns + (j+3)] = m4[i][j];
		}
	}
}

ID_INLINE const float *idMatX::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < numRows ) );
	return mat + index * numColumns;
}

ID_INLINE float *idMatX::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < numRows ) );
	return mat + index * numColumns;
}

ID_INLINE idMatX &idMatX::operator=( const idMatX &a ) {
	SetSize( a.numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Copy16( mat, a.mat, a.numRows * a.numColumns );
#else
	memcpy( mat, a.mat, a.numRows * a.numColumns * sizeof( float ) );
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX idMatX::operator*( const float a ) const {
	idMatX m;

	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Mul16( m.mat, mat, a, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		m.mat[i] = mat[i] * a;
	}
#endif
	return m;
}

ID_INLINE idVecX idMatX::operator*( const idVecX &vec ) const {
	idVecX dst;

	assert( numColumns == vec.GetSize() );

	dst.SetTempSize( numRows );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	Multiply( dst, vec );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::operator*( const idMatX &a ) const {
	idMatX dst;

	assert( numColumns == a.numRows );

	dst.SetTempSize( numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	Multiply( dst, a );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::operator+( const idMatX &a ) const {
	idMatX m;

	assert( numRows == a.numRows && numColumns == a.numColumns );
	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Add16( m.mat, mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		m.mat[i] = mat[i] + a.mat[i];
	}
#endif
	return m;
}

ID_INLINE idMatX idMatX::operator-( const idMatX &a ) const {
	idMatX m;

	assert( numRows == a.numRows && numColumns == a.numColumns );
	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Sub16( m.mat, mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		m.mat[i] = mat[i] - a.mat[i];
	}
#endif
	return m;
}

ID_INLINE idMatX &idMatX::operator*=( const float a ) {
#ifdef MATX_SIMD
	SIMDProcessor->MulAssign16( mat, a, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] *= a;
	}
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX &idMatX::operator*=( const idMatX &a ) {
	*this = *this * a;
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX &idMatX::operator+=( const idMatX &a ) {
	assert( numRows == a.numRows && numColumns == a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->AddAssign16( mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] += a.mat[i];
	}
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX &idMatX::operator-=( const idMatX &a ) {
	assert( numRows == a.numRows && numColumns == a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->SubAssign16( mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] -= a.mat[i];
	}
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX operator*( const float a, idMatX const &m ) {
	return m * a;
}

ID_INLINE idVecX operator*( const idVecX &vec, const idMatX &m ) {
	return m * vec;
}

ID_INLINE idVecX &operator*=( idVecX &vec, const idMatX &m ) {
	vec = m * vec;
	return vec;
}

ID_INLINE bool idMatX::Compare( const idMatX &a ) const {
	int i, s;

	assert( numRows == a.numRows && numColumns == a.numColumns );

	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		if ( mat[i] != a.mat[i] ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMatX::Compare( const idMatX &a, const float epsilon ) const {
	int i, s;

	assert( numRows == a.numRows && numColumns == a.numColumns );

	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		if ( idMath::Fabs( mat[i] - a.mat[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMatX::operator==( const idMatX &a ) const {
	return Compare( a );
}

ID_INLINE bool idMatX::operator!=( const idMatX &a ) const {
	return !Compare( a );
}

ID_INLINE void idMatX::SetSize( int rows, int columns ) {
	assert( mat < idMatX::tempPtr || mat > idMatX::tempPtr + MATX_MAX_TEMP );
	int alloc = ( rows * columns + 3 ) & ~3;
	if ( alloc > alloced && alloced != -1 ) {
		if ( mat != NULL ) {
			Mem_Free16( mat );
		}
		mat = (float *) Mem_Alloc16( alloc * sizeof( float ) );
		alloced = alloc;
	}
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

ID_INLINE void idMatX::SetTempSize( int rows, int columns ) {
	int newSize;

	newSize = ( rows * columns + 3 ) & ~3;
	assert( newSize < MATX_MAX_TEMP );
	if ( idMatX::tempIndex + newSize > MATX_MAX_TEMP ) {
		idMatX::tempIndex = 0;
	}
	mat = idMatX::tempPtr + idMatX::tempIndex;
	idMatX::tempIndex += newSize;
	alloced = newSize;
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

ID_INLINE void idMatX::SetData( int rows, int columns, float *data ) {
	assert( mat < idMatX::tempPtr || mat > idMatX::tempPtr + MATX_MAX_TEMP );
	if ( mat != NULL && alloced != -1 ) {
		Mem_Free16( mat );
	}
	assert( ( ( (int) data ) & 15 ) == 0 ); // data must be 16 byte aligned
	mat = data;
	alloced = -1;
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

ID_INLINE void idMatX::Zero( void ) {
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	memset( mat, 0, numRows * numColumns * sizeof( float ) );
#endif
}

ID_INLINE void idMatX::Zero( int rows, int columns ) {
	SetSize( rows, columns );
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	memset( mat, 0, rows * columns * sizeof( float ) );
#endif
}

ID_INLINE void idMatX::Identity( void ) {
	assert( numRows == numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	memset( mat, 0, numRows * numColumns * sizeof( float ) );
#endif
	for ( int i = 0; i < numRows; i++ ) {
		mat[i * numColumns + i] = 1.0f;
	}
}

ID_INLINE void idMatX::Identity( int rows, int columns ) {
	assert( rows == columns );
	SetSize( rows, columns );
	idMatX::Identity();
}

ID_INLINE void idMatX::Diag( const idVecX &v ) {
	Zero( v.GetSize(), v.GetSize() );
	for ( int i = 0; i < v.GetSize(); i++ ) {
		mat[i * numColumns + i] = v[i];
	}
}

ID_INLINE void idMatX::Random( int seed, float l, float u ) {
	int i, s;
	float c;
	idRandom rnd(seed);

	c = u - l;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idMatX::Random( int rows, int columns, int seed, float l, float u ) {
	int i, s;
	float c;
	idRandom rnd(seed);

	SetSize( rows, columns );
	c = u - l;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idMatX::Negate( void ) {
#ifdef MATX_SIMD
	SIMDProcessor->Negate16( mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] = -mat[i];
	}
#endif
}

ID_INLINE void idMatX::Clamp( float min, float max ) {
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		if ( mat[i] < min ) {
			mat[i] = min;
		} else if ( mat[i] > max ) {
			mat[i] = max;
		}
	}
}

ID_INLINE idMatX &idMatX::SwapRows( int r1, int r2 ) {
	float *ptr;

	ptr = (float *) _alloca16( numColumns * sizeof( float ) );
	memcpy( ptr, mat + r1 * numColumns, numColumns * sizeof( float ) );
	memcpy( mat + r1 * numColumns, mat + r2 * numColumns, numColumns * sizeof( float ) );
	memcpy( mat + r2 * numColumns, ptr, numColumns * sizeof( float ) );

	return *this;
}

ID_INLINE idMatX &idMatX::SwapColumns( int r1, int r2 ) {
	int i;
	float tmp, *ptr;

	for ( i = 0; i < numRows; i++ ) {
		ptr = mat + i * numColumns;
		tmp = ptr[r1];
		ptr[r1] = ptr[r2];
		ptr[r2] = tmp;
	}

	return *this;
}

ID_INLINE idMatX &idMatX::SwapRowsColumns( int r1, int r2 ) {

	SwapRows( r1, r2 );
	SwapColumns( r1, r2 );
	return *this;
}

ID_INLINE void idMatX::ClearUpperTriangle( void ) {
	assert( numRows == numColumns );
	for ( int i = numRows-2; i >= 0; i-- ) {
		memset( mat + i * numColumns + i + 1, 0, (numColumns - 1 - i) * sizeof(float) );
	}
}

ID_INLINE void idMatX::ClearLowerTriangle( void ) {
	assert( numRows == numColumns );
	for ( int i = 1; i < numRows; i++ ) {
		memset( mat + i * numColumns, 0, i * sizeof(float) );
	}
}

ID_INLINE void idMatX::SquareSubMatrix( const idMatX &m, int size ) {
	int i;
	assert( size <= m.numRows && size <= m.numColumns );
	SetSize( size, size );
	for ( i = 0; i < size; i++ ) {
		memcpy( mat + i * numColumns, m.mat + i * m.numColumns, size * sizeof( float ) );
	}
}

ID_INLINE float idMatX::MaxDifference( const idMatX &m ) const {
	int i, j;
	float diff, maxDiff;

	assert( numRows == m.numRows && numColumns == m.numColumns );

	maxDiff = -1.0f;
	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < numColumns; j++ ) {
			diff = idMath::Fabs( mat[ i * numColumns + j ] - m[i][j] );
			if ( maxDiff < 0.0f || diff > maxDiff ) {
				maxDiff = diff;
			}
		}
	}
	return maxDiff;
}

ID_INLINE bool idMatX::IsZero( const float epsilon ) const {
	// returns true if (*this) == Zero
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			if ( idMath::Fabs( mat[i * numColumns + j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsIdentity( const float epsilon ) const {
	// returns true if (*this) == Identity
	assert( numRows == numColumns );
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			if ( idMath::Fabs( mat[i * numColumns + j] - (float)( i == j ) ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsDiagonal( const float epsilon ) const {
	// returns true if all elements are zero except for the elements on the diagonal
	assert( numRows == numColumns );
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			if ( i != j && idMath::Fabs( mat[i * numColumns + j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsTriDiagonal( const float epsilon ) const {
	// returns true if all elements are zero except for the elements on the diagonal plus or minus one column

	if ( numRows != numColumns ) {
		return false;
	}
	for ( int i = 0; i < numRows-2; i++ ) {
		for ( int j = i+2; j < numColumns; j++ ) {
			if ( idMath::Fabs( (*this)[i][j] ) > epsilon ) {
				return false;
			}
			if ( idMath::Fabs( (*this)[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsSymmetric( const float epsilon ) const {
	// (*this)[i][j] == (*this)[j][i]
	if ( numRows != numColumns ) {
		return false;
	}
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			if ( idMath::Fabs( mat[ i * numColumns + j ] - mat[ j * numColumns + i ] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE float idMatX::Trace( void ) const {
	float trace = 0.0f;

	assert( numRows == numColumns );

	// sum of elements on the diagonal
	for ( int i = 0; i < numRows; i++ ) {
		trace += mat[i * numRows + i];
	}
	return trace;
}

ID_INLINE float idMatX::Determinant( void ) const {

	assert( numRows == numColumns );

	switch( numRows ) {
		case 1:
			return mat[0];
		case 2:
			return reinterpret_cast<const idMat2 *>(mat)->Determinant();
		case 3:
			return reinterpret_cast<const idMat3 *>(mat)->Determinant();
		case 4:
			return reinterpret_cast<const idMat4 *>(mat)->Determinant();
		case 5:
			return reinterpret_cast<const idMat5 *>(mat)->Determinant();
		case 6:
			return reinterpret_cast<const idMat6 *>(mat)->Determinant();
		default:
			return DeterminantGeneric();
	}
	return 0.0f;
}

ID_INLINE idMatX idMatX::Transpose( void ) const {
	idMatX transpose;
	int i, j;

	transpose.SetTempSize( numColumns, numRows );

	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < numColumns; j++ ) {
			transpose.mat[j * transpose.numColumns + i] = mat[i * numColumns + j];
		}
	}

	return transpose;
}

ID_INLINE idMatX &idMatX::TransposeSelf( void ) {
	*this = Transpose();
	return *this;
}

ID_INLINE idMatX idMatX::Inverse( void ) const {
	idMatX invMat;

	invMat.SetTempSize( numRows, numColumns );
	memcpy( invMat.mat, mat, numRows * numColumns * sizeof( float ) );
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

ID_INLINE bool idMatX::InverseSelf( void ) {

	assert( numRows == numColumns );

	switch( numRows ) {
		case 1:
			if ( idMath::Fabs( mat[0] ) < MATRIX_INVERSE_EPSILON ) {
				return false;
			}
			mat[0] = 1.0f / mat[0];
			return true;
		case 2:
			return reinterpret_cast<idMat2 *>(mat)->InverseSelf();
		case 3:
			return reinterpret_cast<idMat3 *>(mat)->InverseSelf();
		case 4:
			return reinterpret_cast<idMat4 *>(mat)->InverseSelf();
		case 5:
			return reinterpret_cast<idMat5 *>(mat)->InverseSelf();
		case 6:
			return reinterpret_cast<idMat6 *>(mat)->InverseSelf();
		default:
			return InverseSelfGeneric();
	}
}

ID_INLINE idMatX idMatX::InverseFast( void ) const {
	idMatX invMat;

	invMat.SetTempSize( numRows, numColumns );
	memcpy( invMat.mat, mat, numRows * numColumns * sizeof( float ) );
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

ID_INLINE bool idMatX::InverseFastSelf( void ) {

	assert( numRows == numColumns );

	switch( numRows ) {
		case 1:
			if ( idMath::Fabs( mat[0] ) < MATRIX_INVERSE_EPSILON ) {
				return false;
			}
			mat[0] = 1.0f / mat[0];
			return true;
		case 2:
			return reinterpret_cast<idMat2 *>(mat)->InverseFastSelf();
		case 3:
			return reinterpret_cast<idMat3 *>(mat)->InverseFastSelf();
		case 4:
			return reinterpret_cast<idMat4 *>(mat)->InverseFastSelf();
		case 5:
			return reinterpret_cast<idMat5 *>(mat)->InverseFastSelf();
		case 6:
			return reinterpret_cast<idMat6 *>(mat)->InverseFastSelf();
		default:
			return InverseSelfGeneric();
	}
	return false;
}

ID_INLINE idVecX idMatX::Multiply( const idVecX &vec ) const {
	idVecX dst;

	assert( numColumns == vec.GetSize() );

	dst.SetTempSize( numRows );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	Multiply( dst, vec );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::Multiply( const idMatX &a ) const {
	idMatX dst;

	assert( numColumns == a.numRows );

	dst.SetTempSize( numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	Multiply( dst, a );
#endif
	return dst;
}

ID_INLINE idVecX idMatX::TransposeMultiply( const idVecX &vec ) const {
	idVecX dst;

	assert( numRows == vec.GetSize() );

	dst.SetTempSize( numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyVecX( dst, *this, vec );
#else
	TransposeMultiply( dst, vec );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::TransposeMultiply( const idMatX &a ) const {
	idMatX dst;

	assert( numRows == a.numRows );

	dst.SetTempSize( numColumns, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyMatX( dst, *this, a );
#else
	TransposeMultiply( dst, a );
#endif
	return dst;
}

ID_INLINE void idMatX::Multiply( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	mPtr = mat;
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numRows; i++ ) {
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numColumns; j++ ) {
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] = sum;
		mPtr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::MultiplyAdd( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyAddVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	mPtr = mat;
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numRows; i++ ) {
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numColumns; j++ ) {
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] += sum;
		mPtr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::MultiplySub( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplySubVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	mPtr = mat;
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numRows; i++ ) {
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numColumns; j++ ) {
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] -= sum;
		mPtr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiply( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numColumns; i++ ) {
		mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numRows; j++ ) {
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		dstPtr[i] = sum;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiplyAdd( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyAddVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numColumns; i++ ) {
		mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numRows; j++ ) {
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		dstPtr[i] += sum;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiplySub( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplySubVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numColumns; i++ ) {
		mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numRows; j++ ) {
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		dstPtr[i] -= sum;
	}
#endif
}

ID_INLINE void idMatX::Multiply( idMatX &dst, const idMatX &a ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	int i, j, k, l, n;
	float *dstPtr;
	const float *m1Ptr, *m2Ptr;
	double sum;

	assert( numColumns == a.numRows );

	dstPtr = dst.ToFloatPtr();
	m1Ptr = ToFloatPtr();
	m2Ptr = a.ToFloatPtr();
	k = numRows;
	l = a.GetNumColumns();

	for ( i = 0; i < k; i++ ) {
		for ( j = 0; j < l; j++ ) {
			m2Ptr = a.ToFloatPtr() + j;
			sum = m1Ptr[0] * m2Ptr[0];
			for ( n = 1; n < numColumns; n++ ) {
				m2Ptr += l;
				sum += m1Ptr[n] * m2Ptr[0];
			}
			*dstPtr++ = sum;
		}
		m1Ptr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiply( idMatX &dst, const idMatX &a ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyMatX( dst, *this, a );
#else
	int i, j, k, l, n;
	float *dstPtr;
	const float *m1Ptr, *m2Ptr;
	double sum;

	assert( numRows == a.numRows );

	dstPtr = dst.ToFloatPtr();
	m1Ptr = ToFloatPtr();
	k = numColumns;
	l = a.numColumns;

	for ( i = 0; i < k; i++ ) {
		for ( j = 0; j < l; j++ ) {
			m1Ptr = ToFloatPtr() + i;
			m2Ptr = a.ToFloatPtr() + j;
			sum = m1Ptr[0] * m2Ptr[0];
			for ( n = 1; n < numRows; n++ ) {
				m1Ptr += numColumns;
				m2Ptr += a.numColumns;
				sum += m1Ptr[0] * m2Ptr[0];
			}
			*dstPtr++ = sum;
		}
	}
#endif
}

ID_INLINE int idMatX::GetDimension( void ) const {
	return numRows * numColumns;
}

ID_INLINE const idVec6 &idMatX::SubVec6( int row ) const {
	assert( numColumns >= 6 && row >= 0 && row < numRows );
	return *reinterpret_cast<const idVec6 *>(mat + row * numColumns);
}

ID_INLINE idVec6 &idMatX::SubVec6( int row ) {
	assert( numColumns >= 6 && row >= 0 && row < numRows );
	return *reinterpret_cast<idVec6 *>(mat + row * numColumns);
}

ID_INLINE const idVecX idMatX::SubVecX( int row ) const {
	idVecX v;
	assert( row >= 0 && row < numRows );
	v.SetData( numColumns, mat + row * numColumns );
	return v;
}

ID_INLINE idVecX idMatX::SubVecX( int row ) {
	idVecX v;
	assert( row >= 0 && row < numRows );
	v.SetData( numColumns, mat + row * numColumns );
	return v;
}

ID_INLINE const float *idMatX::ToFloatPtr( void ) const {
	return mat;
}

ID_INLINE float *idMatX::ToFloatPtr( void ) {
	return mat;
}

#endif /* !__MATH_MATRIX_H__ */

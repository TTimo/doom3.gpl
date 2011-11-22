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
#pragma hdrstop

static idCVar lcp_showFailures( "lcp_showFailures", "0", CVAR_SYSTEM | CVAR_BOOL, "show LCP solver failures" );

const float LCP_BOUND_EPSILON			= 1e-5f;
const float LCP_ACCEL_EPSILON			= 1e-5f;
const float LCP_DELTA_ACCEL_EPSILON		= 1e-9f;
const float LCP_DELTA_FORCE_EPSILON		= 1e-9f;

#define IGNORE_UNSATISFIABLE_VARIABLES

//===============================================================
//                                                        M
//  idLCP_Square                                         MrE
//                                                        E
//===============================================================

class idLCP_Square : public idLCP {
public:
	virtual bool	Solve( const idMatX &o_m, idVecX &o_x, const idVecX &o_b, const idVecX &o_lo, const idVecX &o_hi, const int *o_boxIndex );

private:
	idMatX			m;					// original matrix
	idVecX			b;					// right hand side
	idVecX			lo, hi;				// low and high bounds
	idVecX			f, a;				// force and acceleration
	idVecX			delta_f, delta_a;	// delta force and delta acceleration
	idMatX			clamped;			// LU factored sub matrix for clamped variables
	idVecX			diagonal;			// reciprocal of diagonal of U of the LU factored sub matrix for clamped variables
	int				numUnbounded;		// number of unbounded variables
	int				numClamped;			// number of clamped variables
	float **		rowPtrs;			// pointers to the rows of m
	int *			boxIndex;			// box index
	int *			side;				// tells if a variable is at the low boundary = -1, high boundary = 1 or inbetween = 0
	int *			permuted;			// index to keep track of the permutation
	bool			padded;				// set to true if the rows of the initial matrix are 16 byte padded

private:
	bool			FactorClamped( void );
	void			SolveClamped( idVecX &x, const float *b );
	void			Swap( int i, int j );
	void			AddClamped( int r );
	void			RemoveClamped( int r );
	void			CalcForceDelta( int d, float dir );
	void			CalcAccelDelta( int d );
	void			ChangeForce( int d, float step );
	void			ChangeAccel( int d, float step );
	void			GetMaxStep( int d, float dir, float &maxStep, int &limit, int &limitSide ) const;
};

/*
============
idLCP_Square::FactorClamped
============
*/
bool idLCP_Square::FactorClamped( void ) {
	int i, j, k;
	float s, d;

	for ( i = 0; i < numClamped; i++ ) {
		memcpy( clamped[i], rowPtrs[i], numClamped * sizeof( float ) );
	}

	for ( i = 0; i < numClamped; i++ ) {

		s = idMath::Fabs( clamped[i][i] );

		if ( s == 0.0f ) {
			return false;
		}

		diagonal[i] = d = 1.0f / clamped[i][i];
		for ( j = i + 1; j < numClamped; j++ ) {
			clamped[j][i] *= d;
		}

		for ( j = i + 1; j < numClamped; j++ ) {
			d = clamped[j][i];
			for ( k = i + 1; k < numClamped; k++ ) {
				clamped[j][k] -= d * clamped[i][k];
			}
		}
	}

	return true;
}

/*
============
idLCP_Square::SolveClamped
============
*/
void idLCP_Square::SolveClamped( idVecX &x, const float *b ) {
	int i, j;
	float sum;

	// solve L
	for ( i = 0; i < numClamped; i++ ) {
		sum = b[i];
		for ( j = 0; j < i; j++ ) {
			sum -= clamped[i][j] * x[j];
		}
		x[i] = sum;
	}

	// solve U
	for ( i = numClamped - 1; i >= 0; i-- ) {
		sum = x[i];
		for ( j = i + 1; j < numClamped; j++ ) {
			sum -= clamped[i][j] * x[j];
		}
		x[i] = sum * diagonal[i];
	}
}

/*
============
idLCP_Square::Swap
============
*/
void idLCP_Square::Swap( int i, int j ) {

	if ( i == j ) {
		return;
	}

	idSwap( rowPtrs[i], rowPtrs[j] );
	m.SwapColumns( i, j );
	b.SwapElements( i, j );
	lo.SwapElements( i, j );
	hi.SwapElements( i, j );
	a.SwapElements( i, j );
	f.SwapElements( i, j );
	if ( boxIndex ) {
		idSwap( boxIndex[i], boxIndex[j] );
	}
	idSwap( side[i], side[j] );
	idSwap( permuted[i], permuted[j] );
}

/*
============
idLCP_Square::AddClamped
============
*/
void idLCP_Square::AddClamped( int r ) {
	int i, j;
	float sum;

	assert( r >= numClamped );

	// add a row at the bottom and a column at the right of the factored
	// matrix for the clamped variables

	Swap( numClamped, r );

	// add row to L
	for ( i = 0; i < numClamped; i++ ) {
		sum = rowPtrs[numClamped][i];
		for ( j = 0; j < i; j++ ) {
			sum -= clamped[numClamped][j] * clamped[j][i];
		}
		clamped[numClamped][i] = sum * diagonal[i];
	}

	// add column to U
	for ( i = 0; i <= numClamped; i++ ) {
		sum = rowPtrs[i][numClamped];
		for ( j = 0; j < i; j++ ) {
			sum -= clamped[i][j] * clamped[j][numClamped];
		}
		clamped[i][numClamped] = sum;
	}

	diagonal[numClamped] = 1.0f / clamped[numClamped][numClamped];

	numClamped++;
}

/*
============
idLCP_Square::RemoveClamped
============
*/
void idLCP_Square::RemoveClamped( int r ) {
	int i, j;
	float *y0, *y1, *z0, *z1;
	double diag, beta0, beta1, p0, p1, q0, q1, d;

	assert( r < numClamped );

	numClamped--;

	// no need to swap and update the factored matrix when the last row and column are removed
	if ( r == numClamped ) {
		return;
	}

	y0 = (float *) _alloca16( numClamped * sizeof( float ) );
	z0 = (float *) _alloca16( numClamped * sizeof( float ) );
	y1 = (float *) _alloca16( numClamped * sizeof( float ) );
	z1 = (float *) _alloca16( numClamped * sizeof( float ) );

	// the row/column need to be subtracted from the factorization
	for ( i = 0; i < numClamped; i++ ) {
		y0[i] = -rowPtrs[i][r];
	}

	memset( y1, 0, numClamped * sizeof( float ) );
	y1[r] = 1.0f;

	memset( z0, 0, numClamped * sizeof( float ) );
	z0[r] = 1.0f;

	for ( i = 0; i < numClamped; i++ ) {
		z1[i] = -rowPtrs[r][i];
	}

	// swap the to be removed row/column with the last row/column
	Swap( r, numClamped );

	// the swapped last row/column need to be added to the factorization
	for ( i = 0; i < numClamped; i++ ) {
		y0[i] += rowPtrs[i][r];
	}

	for ( i = 0; i < numClamped; i++ ) {
		z1[i] += rowPtrs[r][i];
	}
	z1[r] = 0.0f;

	// update the beginning of the to be updated row and column
	for ( i = 0; i < r; i++ ) {
		p0 = y0[i];
		beta1 = z1[i] * diagonal[i];

		clamped[i][r] += p0;
		for ( j = i+1; j < numClamped; j++ ) {
			z1[j] -= beta1 * clamped[i][j];
		}
		for ( j = i+1; j < numClamped; j++ ) {
			y0[j] -= p0 * clamped[j][i];
		}
		clamped[r][i] += beta1;
	}

	// update the lower right corner starting at r,r
	for ( i = r; i < numClamped; i++ ) {
		diag = clamped[i][i];

		p0 = y0[i];
		p1 = z0[i];
		diag += p0 * p1;

		if ( diag == 0.0f ) {
			idLib::common->Printf( "idLCP_Square::RemoveClamped: updating factorization failed\n" );
			return;
		}

		beta0 = p1 / diag;

		q0 = y1[i];
		q1 = z1[i];
		diag += q0 * q1;

		if ( diag == 0.0f ) {
			idLib::common->Printf( "idLCP_Square::RemoveClamped: updating factorization failed\n" );
			return;
		}

		d = 1.0f / diag;
		beta1 = q1 * d;

		clamped[i][i] = diag;
		diagonal[i] = d;

		for ( j = i+1; j < numClamped; j++ ) {

			d = clamped[i][j];

			d += p0 * z0[j];
			z0[j] -= beta0 * d;

			d += q0 * z1[j];
			z1[j] -= beta1 * d;

			clamped[i][j] = d;
		}

		for ( j = i+1; j < numClamped; j++ ) {

			d = clamped[j][i];

			y0[j] -= p0 * d;
			d += beta0 * y0[j];

			y1[j] -= q0 * d;
			d += beta1 * y1[j];

			clamped[j][i] = d;
		}
	}
	return;
}

/*
============
idLCP_Square::CalcForceDelta

  modifies this->delta_f
============
*/
ID_INLINE void idLCP_Square::CalcForceDelta( int d, float dir ) {
	int i;
	float *ptr;

	delta_f[d] = dir;

	if ( numClamped == 0 ) {
		return;
	}

	// get column d of matrix
	ptr = (float *) _alloca16( numClamped * sizeof( float ) );
	for ( i = 0; i < numClamped; i++ ) {
		ptr[i] = rowPtrs[i][d];
	}

	// solve force delta
	SolveClamped( delta_f, ptr );

	// flip force delta based on direction
	if ( dir > 0.0f ) {
		ptr = delta_f.ToFloatPtr();
		for ( i = 0; i < numClamped; i++ ) {
			ptr[i] = - ptr[i];
		}
	}
}

/*
============
idLCP_Square::CalcAccelDelta

  modifies this->delta_a and uses this->delta_f
============
*/
ID_INLINE void idLCP_Square::CalcAccelDelta( int d ) {
	int j;
	float dot;

	// only the not clamped variables, including the current variable, can have a change in acceleration
	for ( j = numClamped; j <= d; j++ ) {
		// only the clamped variables and the current variable have a force delta unequal zero
		SIMDProcessor->Dot( dot, rowPtrs[j], delta_f.ToFloatPtr(), numClamped );
		delta_a[j] = dot + rowPtrs[j][d] * delta_f[d];
	}
}

/*
============
idLCP_Square::ChangeForce

  modifies this->f and uses this->delta_f
============
*/
ID_INLINE void idLCP_Square::ChangeForce( int d, float step ) {
	// only the clamped variables and current variable have a force delta unequal zero
	SIMDProcessor->MulAdd( f.ToFloatPtr(), step, delta_f.ToFloatPtr(), numClamped );
	f[d] += step * delta_f[d];
}

/*
============
idLCP_Square::ChangeAccel

  modifies this->a and uses this->delta_a
============
*/
ID_INLINE void idLCP_Square::ChangeAccel( int d, float step ) {
	// only the not clamped variables, including the current variable, can have an acceleration unequal zero
	SIMDProcessor->MulAdd( a.ToFloatPtr() + numClamped, step, delta_a.ToFloatPtr() + numClamped, d - numClamped + 1 );
}

/*
============
idLCP_Square::GetMaxStep
============
*/
void idLCP_Square::GetMaxStep( int d, float dir, float &maxStep, int &limit, int &limitSide ) const {
	int i;
	float s;

	// default to a full step for the current variable
	if ( idMath::Fabs( delta_a[d] ) > LCP_DELTA_ACCEL_EPSILON ) {
		maxStep = -a[d] / delta_a[d];
	} else {
		maxStep = 0.0f;
	}
	limit = d;
	limitSide = 0;

	// test the current variable
	if ( dir < 0.0f ) {
		if ( lo[d] != -idMath::INFINITY ) {
			s = ( lo[d] - f[d] ) / dir;
			if ( s < maxStep ) {
				maxStep = s;
				limitSide = -1;
			}
		}
	} else {
		if ( hi[d] != idMath::INFINITY ) {
			s = ( hi[d] - f[d] ) / dir;
			if ( s < maxStep ) {
				maxStep = s;
				limitSide = 1;
			}
		}
	}

	// test the clamped bounded variables
	for ( i = numUnbounded; i < numClamped; i++ ) {
		if ( delta_f[i] < -LCP_DELTA_FORCE_EPSILON ) {
			// if there is a low boundary
			if ( lo[i] != -idMath::INFINITY ) {
				s = ( lo[i] - f[i] ) / delta_f[i];
				if ( s < maxStep ) {
					maxStep = s;
					limit = i;
					limitSide = -1;
				}
			}
		} else if ( delta_f[i] > LCP_DELTA_FORCE_EPSILON ) {
			// if there is a high boundary
			if ( hi[i] != idMath::INFINITY ) {
				s = ( hi[i] - f[i] ) / delta_f[i];
				if ( s < maxStep ) {
					maxStep = s;
					limit = i;
					limitSide = 1;
				}
			}
		}
	}

	// test the not clamped bounded variables
	for ( i = numClamped; i < d; i++ ) {
		if ( side[i] == -1 ) {
			if ( delta_a[i] >= -LCP_DELTA_ACCEL_EPSILON ) {
				continue;
			}
		} else if ( side[i] == 1 ) {
			if ( delta_a[i] <= LCP_DELTA_ACCEL_EPSILON ) {
				continue;
			}
		} else {
			continue;
		}
		// ignore variables for which the force is not allowed to take any substantial value
		if ( lo[i] >= -LCP_BOUND_EPSILON && hi[i] <= LCP_BOUND_EPSILON ) {
			continue;
		}
		s = -a[i] / delta_a[i];
		if ( s < maxStep ) {
			maxStep = s;
			limit = i;
			limitSide = 0;
		}
	}
}

/*
============
idLCP_Square::Solve
============
*/
bool idLCP_Square::Solve( const idMatX &o_m, idVecX &o_x, const idVecX &o_b, const idVecX &o_lo, const idVecX &o_hi, const int *o_boxIndex ) {
	int i, j, n, limit, limitSide, boxStartIndex;
	float dir, maxStep, dot, s;
	char *failed;

	// true when the matrix rows are 16 byte padded
	padded = ((o_m.GetNumRows()+3)&~3) == o_m.GetNumColumns();

	assert( padded || o_m.GetNumRows() == o_m.GetNumColumns() );
	assert( o_x.GetSize() == o_m.GetNumRows() );
	assert( o_b.GetSize() == o_m.GetNumRows() );
	assert( o_lo.GetSize() == o_m.GetNumRows() );
	assert( o_hi.GetSize() == o_m.GetNumRows() );

	// allocate memory for permuted input
	f.SetData( o_m.GetNumRows(), VECX_ALLOCA( o_m.GetNumRows() ) );
	a.SetData( o_b.GetSize(), VECX_ALLOCA( o_b.GetSize() ) );
	b.SetData( o_b.GetSize(), VECX_ALLOCA( o_b.GetSize() ) );
	lo.SetData( o_lo.GetSize(), VECX_ALLOCA( o_lo.GetSize() ) );
	hi.SetData( o_hi.GetSize(), VECX_ALLOCA( o_hi.GetSize() ) );
	if ( o_boxIndex ) {
		boxIndex = (int *)_alloca16( o_x.GetSize() * sizeof( int ) );
		memcpy( boxIndex, o_boxIndex, o_x.GetSize() * sizeof( int ) );
	} else {
		boxIndex = NULL;
	}

	// we override the const on o_m here but on exit the matrix is unchanged
	m.SetData( o_m.GetNumRows(), o_m.GetNumColumns(), const_cast<float *>(o_m[0]) );
	f.Zero();
	a.Zero();
	b = o_b;
	lo = o_lo;
	hi = o_hi;

	// pointers to the rows of m
	rowPtrs = (float **) _alloca16( m.GetNumRows() * sizeof( float * ) );
	for ( i = 0; i < m.GetNumRows(); i++ ) {
		rowPtrs[i] = m[i];
	}

	// tells if a variable is at the low boundary, high boundary or inbetween
	side = (int *) _alloca16( m.GetNumRows() * sizeof( int ) );

	// index to keep track of the permutation
	permuted = (int *) _alloca16( m.GetNumRows() * sizeof( int ) );
	for ( i = 0; i < m.GetNumRows(); i++ ) {
		permuted[i] = i;
	}

	// permute input so all unbounded variables come first
	numUnbounded = 0;
	for ( i = 0; i < m.GetNumRows(); i++ ) {
		if ( lo[i] == -idMath::INFINITY && hi[i] == idMath::INFINITY ) {
			if ( numUnbounded != i ) {
				Swap( numUnbounded, i );
			}
			numUnbounded++;
		}
	}

	// permute input so all variables using the boxIndex come last
	boxStartIndex = m.GetNumRows();
	if ( boxIndex ) {
		for ( i = m.GetNumRows() - 1; i >= numUnbounded; i-- ) {
			if ( boxIndex[i] >= 0 && ( lo[i] != -idMath::INFINITY || hi[i] != idMath::INFINITY ) ) {
				boxStartIndex--;
				if ( boxStartIndex != i ) {
					Swap( boxStartIndex, i );
				}
			}
		}
	}

	// sub matrix for factorization 
	clamped.SetData( m.GetNumRows(), m.GetNumColumns(), MATX_ALLOCA( m.GetNumRows() * m.GetNumColumns() ) );
	diagonal.SetData( m.GetNumRows(), VECX_ALLOCA( m.GetNumRows() ) );

	// all unbounded variables are clamped
	numClamped = numUnbounded;

	// if there are unbounded variables
	if ( numUnbounded ) {

		// factor and solve for unbounded variables
		if ( !FactorClamped() ) {
			idLib::common->Printf( "idLCP_Square::Solve: unbounded factorization failed\n" );
			return false;
		}
		SolveClamped( f, b.ToFloatPtr() );

		// if there are no bounded variables we are done
		if ( numUnbounded == m.GetNumRows() ) {
			o_x = f;	// the vector is not permuted
			return true;
		}
	}

#ifdef IGNORE_UNSATISFIABLE_VARIABLES
	int numIgnored = 0;
#endif

	// allocate for delta force and delta acceleration
	delta_f.SetData( m.GetNumRows(), VECX_ALLOCA( m.GetNumRows() ) );
	delta_a.SetData( m.GetNumRows(), VECX_ALLOCA( m.GetNumRows() ) );

	// solve for bounded variables
	failed = NULL;
	for ( i = numUnbounded; i < m.GetNumRows(); i++ ) {

		// once we hit the box start index we can initialize the low and high boundaries of the variables using the box index
		if ( i == boxStartIndex ) {
			for ( j = 0; j < boxStartIndex; j++ ) {
				o_x[permuted[j]] = f[j];
			}
			for ( j = boxStartIndex; j < m.GetNumRows(); j++ ) {
				s = o_x[boxIndex[j]];
				if ( lo[j] != -idMath::INFINITY ) {
					lo[j] = - idMath::Fabs( lo[j] * s );
				}
				if ( hi[j] != idMath::INFINITY ) {
					hi[j] = idMath::Fabs( hi[j] * s );
				}
			}
		}

		// calculate acceleration for current variable
		SIMDProcessor->Dot( dot, rowPtrs[i], f.ToFloatPtr(), i );
		a[i] = dot - b[i];

		// if already at the low boundary
		if ( lo[i] >= -LCP_BOUND_EPSILON && a[i] >= -LCP_ACCEL_EPSILON ) {
			side[i] = -1;
			continue;
		}

		// if already at the high boundary
		if ( hi[i] <= LCP_BOUND_EPSILON && a[i] <= LCP_ACCEL_EPSILON ) {
			side[i] = 1;
			continue;
		}

		// if inside the clamped region
		if ( idMath::Fabs( a[i] ) <= LCP_ACCEL_EPSILON ) {
			side[i] = 0;
			AddClamped( i );
			continue;
		}

		// drive the current variable into a valid region
		for ( n = 0; n < maxIterations; n++ ) {

			// direction to move
			if ( a[i] <= 0.0f ) {
				dir = 1.0f;
			} else {
				dir = -1.0f;
			}

			// calculate force delta
			CalcForceDelta( i, dir );

			// calculate acceleration delta: delta_a = m * delta_f;
			CalcAccelDelta( i );

			// maximum step we can take
			GetMaxStep( i, dir, maxStep, limit, limitSide );

			if ( maxStep <= 0.0f ) {
#ifdef IGNORE_UNSATISFIABLE_VARIABLES
				// ignore the current variable completely
				lo[i] = hi[i] = 0.0f;
				f[i] = 0.0f;
				side[i] = -1;
				numIgnored++;
#else
				failed = va( "invalid step size %.4f", maxStep );
#endif
				break;
			}

			// change force
			ChangeForce( i, maxStep );

			// change acceleration
			ChangeAccel( i, maxStep );

			// clamp/unclamp the variable that limited this step
			side[limit] = limitSide;
			switch( limitSide ) {
				case 0: {
					a[limit] = 0.0f;
					AddClamped( limit );
					break;
				}
				case -1: {
					f[limit] = lo[limit];
					if ( limit != i ) {
						RemoveClamped( limit );
					}
					break;
				}
				case 1: {
					f[limit] = hi[limit];
					if ( limit != i ) {
						RemoveClamped( limit );
					}
					break;
				}
			}

			// if the current variable limited the step we can continue with the next variable
			if ( limit == i ) {
				break;
			}
		}

		if ( n >= maxIterations ) {
			failed = va( "max iterations %d", maxIterations );
			break;
		}

		if ( failed ) {
			break;
		}
	}

#ifdef IGNORE_UNSATISFIABLE_VARIABLES
	if ( numIgnored ) {
		if ( lcp_showFailures.GetBool() ) {
			idLib::common->Printf( "idLCP_Symmetric::Solve: %d of %d bounded variables ignored\n", numIgnored, m.GetNumRows() - numUnbounded );
		}
	}
#endif

	// if failed clear remaining forces
	if ( failed ) {
		if ( lcp_showFailures.GetBool() ) {
			idLib::common->Printf( "idLCP_Square::Solve: %s (%d of %d bounded variables ignored)\n", failed, m.GetNumRows() - i, m.GetNumRows() - numUnbounded );
		}
		for ( j = i; j < m.GetNumRows(); j++ ) {
			f[j] = 0.0f;
		}
	}

#if defined(_DEBUG) && 0
	if ( !failed ) {
		// test whether or not the solution satisfies the complementarity conditions
		for ( i = 0; i < m.GetNumRows(); i++ ) {
			a[i] = -b[i];
			for ( j = 0; j < m.GetNumRows(); j++ ) {
				a[i] += rowPtrs[i][j] * f[j];
			}

			if ( f[i] == lo[i] ) {
				if ( lo[i] != hi[i] && a[i] < -LCP_ACCEL_EPSILON ) {
					int bah1 = 1;
				}
			} else if ( f[i] == hi[i] ) {
				if ( lo[i] != hi[i] && a[i] > LCP_ACCEL_EPSILON ) {
					int bah2 = 1;
				}
			} else if ( f[i] < lo[i] || f[i] > hi[i] || idMath::Fabs( a[i] ) > 1.0f ) {
				int bah3 = 1;
			}
		}
	}
#endif

	// unpermute result
	for ( i = 0; i < f.GetSize(); i++ ) {
		o_x[permuted[i]] = f[i];
	}

	// unpermute original matrix
	for ( i = 0; i < m.GetNumRows(); i++ ) {
		for ( j = 0; j < m.GetNumRows(); j++ ) {
			if ( permuted[j] == i ) {
				break;
			}
		}
		if ( i != j ) {
			m.SwapColumns( i, j );
			idSwap( permuted[i], permuted[j] );
		}
	}

	return true;
}


//===============================================================
//                                                        M
//  idLCP_Symmetric                                      MrE
//                                                        E
//===============================================================

class idLCP_Symmetric : public idLCP {
public:
	virtual bool	Solve( const idMatX &o_m, idVecX &o_x, const idVecX &o_b, const idVecX &o_lo, const idVecX &o_hi, const int *o_boxIndex );

private:
	idMatX			m;					// original matrix
	idVecX			b;					// right hand side
	idVecX			lo, hi;				// low and high bounds
	idVecX			f, a;				// force and acceleration
	idVecX			delta_f, delta_a;	// delta force and delta acceleration
	idMatX			clamped;			// LDLt factored sub matrix for clamped variables
	idVecX			diagonal;			// reciprocal of diagonal of LDLt factored sub matrix for clamped variables
	idVecX			solveCache1;		// intermediate result cached in SolveClamped
	idVecX			solveCache2;		// "
	int				numUnbounded;		// number of unbounded variables
	int				numClamped;			// number of clamped variables
	int				clampedChangeStart;	// lowest row/column changed in the clamped matrix during an iteration
	float **		rowPtrs;			// pointers to the rows of m
	int *			boxIndex;			// box index
	int *			side;				// tells if a variable is at the low boundary = -1, high boundary = 1 or inbetween = 0
	int *			permuted;			// index to keep track of the permutation
	bool			padded;				// set to true if the rows of the initial matrix are 16 byte padded

private:
	bool			FactorClamped( void );
	void			SolveClamped( idVecX &x, const float *b );
	void			Swap( int i, int j );
	void			AddClamped( int r, bool useSolveCache );
	void			RemoveClamped( int r );
	void			CalcForceDelta( int d, float dir );
	void			CalcAccelDelta( int d );
	void			ChangeForce( int d, float step );
	void			ChangeAccel( int d, float step );
	void			GetMaxStep( int d, float dir, float &maxStep, int &limit, int &limitSide ) const;
};

/*
============
idLCP_Symmetric::FactorClamped
============
*/
bool idLCP_Symmetric::FactorClamped( void ) {

	clampedChangeStart = 0;

	for ( int i = 0; i < numClamped; i++ ) {
		memcpy( clamped[i], rowPtrs[i], numClamped * sizeof( float ) );
	}
	return SIMDProcessor->MatX_LDLTFactor( clamped, diagonal, numClamped );
}

/*
============
idLCP_Symmetric::SolveClamped
============
*/
void idLCP_Symmetric::SolveClamped( idVecX &x, const float *b ) {

	// solve L
	SIMDProcessor->MatX_LowerTriangularSolve( clamped, solveCache1.ToFloatPtr(), b, numClamped, clampedChangeStart );

	// solve D
	SIMDProcessor->Mul( solveCache2.ToFloatPtr(), solveCache1.ToFloatPtr(), diagonal.ToFloatPtr(), numClamped );

	// solve Lt
	SIMDProcessor->MatX_LowerTriangularSolveTranspose( clamped, x.ToFloatPtr(), solveCache2.ToFloatPtr(), numClamped );

	clampedChangeStart = numClamped;
}

/*
============
idLCP_Symmetric::Swap
============
*/
void idLCP_Symmetric::Swap( int i, int j ) {

	if ( i == j ) {
		return;
	}

	idSwap( rowPtrs[i], rowPtrs[j] );
	m.SwapColumns( i, j );
	b.SwapElements( i, j );
	lo.SwapElements( i, j );
	hi.SwapElements( i, j );
	a.SwapElements( i, j );
	f.SwapElements( i, j );
	if ( boxIndex ) {
		idSwap( boxIndex[i], boxIndex[j] );
	}
	idSwap( side[i], side[j] );
	idSwap( permuted[i], permuted[j] );
}

/*
============
idLCP_Symmetric::AddClamped
============
*/
void idLCP_Symmetric::AddClamped( int r, bool useSolveCache ) {
	float d, dot;

	assert( r >= numClamped );

	if ( numClamped < clampedChangeStart ) {
		clampedChangeStart = numClamped;
	}

	// add a row at the bottom and a column at the right of the factored
	// matrix for the clamped variables

	Swap( numClamped, r );

	// solve for v in L * v = rowPtr[numClamped]
	if ( useSolveCache ) {

		// the lower triangular solve was cached in SolveClamped called by CalcForceDelta
		memcpy( clamped[numClamped], solveCache2.ToFloatPtr(), numClamped * sizeof( float ) );
		// calculate row dot product
		SIMDProcessor->Dot( dot, solveCache2.ToFloatPtr(), solveCache1.ToFloatPtr(), numClamped );

	} else {

		float *v = (float *) _alloca16( numClamped * sizeof( float ) );

		SIMDProcessor->MatX_LowerTriangularSolve( clamped, v, rowPtrs[numClamped], numClamped );
		// add bottom row to L
		SIMDProcessor->Mul( clamped[numClamped], v, diagonal.ToFloatPtr(), numClamped );
		// calculate row dot product
		SIMDProcessor->Dot( dot, clamped[numClamped], v, numClamped );
	}

	// update diagonal[numClamped]
	d = rowPtrs[numClamped][numClamped] - dot;

	if ( d == 0.0f ) {
		idLib::common->Printf( "idLCP_Symmetric::AddClamped: updating factorization failed\n" );
		numClamped++;
		return;
	}

	clamped[numClamped][numClamped] = d;
	diagonal[numClamped] = 1.0f / d;

	numClamped++;
}

/*
============
idLCP_Symmetric::RemoveClamped
============
*/
void idLCP_Symmetric::RemoveClamped( int r ) {
	int i, j, n;
	float *addSub, *original, *v, *ptr, *v1, *v2, dot;
	double sum, diag, newDiag, invNewDiag, p1, p2, alpha1, alpha2, beta1, beta2;

	assert( r < numClamped );

	if ( r < clampedChangeStart ) {
		clampedChangeStart = r;
	}

	numClamped--;

	// no need to swap and update the factored matrix when the last row and column are removed
	if ( r == numClamped ) {
		return;
	}

	// swap the to be removed row/column with the last row/column
	Swap( r, numClamped );

	// update the factored matrix
	addSub = (float *) _alloca16( numClamped * sizeof( float ) );

	if ( r == 0 ) {

		if ( numClamped == 1 ) {
			diag = rowPtrs[0][0];
			if ( diag == 0.0f ) {
				idLib::common->Printf( "idLCP_Symmetric::RemoveClamped: updating factorization failed\n" );
				return;
			}
			clamped[0][0] = diag;
			diagonal[0] = 1.0f / diag;
			return;
		}

		// calculate the row/column to be added to the lower right sub matrix starting at (r, r)
		original = rowPtrs[numClamped];
		ptr = rowPtrs[r];
		addSub[0] = ptr[0] - original[numClamped];
		for ( i = 1; i < numClamped; i++ ) {
			addSub[i] = ptr[i] - original[i];
		}

	} else {

		v = (float *) _alloca16( numClamped * sizeof( float ) );

		// solve for v in L * v = rowPtr[r]
		SIMDProcessor->MatX_LowerTriangularSolve( clamped, v, rowPtrs[r], r );

		// update removed row
		SIMDProcessor->Mul( clamped[r], v, diagonal.ToFloatPtr(), r );

		// if the last row/column of the matrix is updated
		if ( r == numClamped - 1 ) {
			// only calculate new diagonal
			SIMDProcessor->Dot( dot, clamped[r], v, r );
			diag = rowPtrs[r][r] - dot;
			if ( diag == 0.0f ) {
				idLib::common->Printf( "idLCP_Symmetric::RemoveClamped: updating factorization failed\n" );
				return;
			}
			clamped[r][r] = diag;
			diagonal[r] = 1.0f / diag;
			return;
		}

		// calculate the row/column to be added to the lower right sub matrix starting at (r, r)
		for ( i = 0; i < r; i++ ) {
			v[i] = clamped[r][i] * clamped[i][i];
		}
		for ( i = r; i < numClamped; i++ ) {
			if ( i == r ) {
				sum = clamped[r][r];
			} else {
				sum = clamped[r][r] * clamped[i][r];
			}
			ptr = clamped[i];
			for ( j = 0; j < r; j++ ) {
				sum += ptr[j] * v[j];
			}
			addSub[i] = rowPtrs[r][i] - sum;
		}
	}

	// add row/column to the lower right sub matrix starting at (r, r)

	v1 = (float *) _alloca16( numClamped * sizeof( float ) );
	v2 = (float *) _alloca16( numClamped * sizeof( float ) );

	diag = idMath::SQRT_1OVER2;
	v1[r] = ( 0.5f * addSub[r] + 1.0f ) * diag;
	v2[r] = ( 0.5f * addSub[r] - 1.0f ) * diag;
	for ( i = r+1; i < numClamped; i++ ) {
		v1[i] = v2[i] = addSub[i] * diag;
	}

	alpha1 = 1.0f;
	alpha2 = -1.0f;

	// simultaneous update/downdate of the sub matrix starting at (r, r)
	n = clamped.GetNumColumns();
	for ( i = r; i < numClamped; i++ ) {

		diag = clamped[i][i];
		p1 = v1[i];
		newDiag = diag + alpha1 * p1 * p1;

		if ( newDiag == 0.0f ) {
			idLib::common->Printf( "idLCP_Symmetric::RemoveClamped: updating factorization failed\n" );
			return;
		}

		alpha1 /= newDiag;
		beta1 = p1 * alpha1;
		alpha1 *= diag;

		diag = newDiag;
		p2 = v2[i];
		newDiag = diag + alpha2 * p2 * p2;

		if ( newDiag == 0.0f ) {
			idLib::common->Printf( "idLCP_Symmetric::RemoveClamped: updating factorization failed\n" );
			return;
		}

		clamped[i][i] = newDiag;
		diagonal[i] = invNewDiag = 1.0f / newDiag;

		alpha2 *= invNewDiag;
		beta2 = p2 * alpha2;
		alpha2 *= diag;

		// update column below diagonal (i,i)
		ptr = clamped.ToFloatPtr() + i;

		for ( j = i+1; j < numClamped - 1; j += 2 ) {

			float sum0 = ptr[(j+0)*n];
			float sum1 = ptr[(j+1)*n];

			v1[j+0] -= p1 * sum0;
			v1[j+1] -= p1 * sum1;

			sum0 += beta1 * v1[j+0];
			sum1 += beta1 * v1[j+1];

			v2[j+0] -= p2 * sum0;
			v2[j+1] -= p2 * sum1;

			sum0 += beta2 * v2[j+0];
			sum1 += beta2 * v2[j+1];

			ptr[(j+0)*n] = sum0;
			ptr[(j+1)*n] = sum1;
		}

		for ( ; j < numClamped; j++ ) {

			sum = ptr[j*n];

			v1[j] -= p1 * sum;
			sum += beta1 * v1[j];

			v2[j] -= p2 * sum;
			sum += beta2 * v2[j];

			ptr[j*n] = sum;
		}
	}
}

/*
============
idLCP_Symmetric::CalcForceDelta

  modifies this->delta_f
============
*/
ID_INLINE void idLCP_Symmetric::CalcForceDelta( int d, float dir ) {
	int i;
	float *ptr;

	delta_f[d] = dir;

	if ( numClamped == 0 ) {
		return;
	}

	// solve force delta
	SolveClamped( delta_f, rowPtrs[d] );

	// flip force delta based on direction
	if ( dir > 0.0f ) {
		ptr = delta_f.ToFloatPtr();
		for ( i = 0; i < numClamped; i++ ) {
			ptr[i] = - ptr[i];
		}
	}
}

/*
============
idLCP_Symmetric::CalcAccelDelta

  modifies this->delta_a and uses this->delta_f
============
*/
ID_INLINE void idLCP_Symmetric::CalcAccelDelta( int d ) {
	int j;
	float dot;

	// only the not clamped variables, including the current variable, can have a change in acceleration
	for ( j = numClamped; j <= d; j++ ) {
		// only the clamped variables and the current variable have a force delta unequal zero
		SIMDProcessor->Dot( dot, rowPtrs[j], delta_f.ToFloatPtr(), numClamped );
		delta_a[j] = dot + rowPtrs[j][d] * delta_f[d];
	}
}

/*
============
idLCP_Symmetric::ChangeForce

  modifies this->f and uses this->delta_f
============
*/
ID_INLINE void idLCP_Symmetric::ChangeForce( int d, float step ) {
	// only the clamped variables and current variable have a force delta unequal zero
	SIMDProcessor->MulAdd( f.ToFloatPtr(), step, delta_f.ToFloatPtr(), numClamped );
	f[d] += step * delta_f[d];
}

/*
============
idLCP_Symmetric::ChangeAccel

  modifies this->a and uses this->delta_a
============
*/
ID_INLINE void idLCP_Symmetric::ChangeAccel( int d, float step ) {
	// only the not clamped variables, including the current variable, can have an acceleration unequal zero
	SIMDProcessor->MulAdd( a.ToFloatPtr() + numClamped, step, delta_a.ToFloatPtr() + numClamped, d - numClamped + 1 );
}

/*
============
idLCP_Symmetric::GetMaxStep
============
*/
void idLCP_Symmetric::GetMaxStep( int d, float dir, float &maxStep, int &limit, int &limitSide ) const {
	int i;
	float s;

	// default to a full step for the current variable
	if ( idMath::Fabs( delta_a[d] ) > LCP_DELTA_ACCEL_EPSILON ) {
		maxStep = -a[d] / delta_a[d];
	} else {
		maxStep = 0.0f;
	}
	limit = d;
	limitSide = 0;

	// test the current variable
	if ( dir < 0.0f ) {
		if ( lo[d] != -idMath::INFINITY ) {
			s = ( lo[d] - f[d] ) / dir;
			if ( s < maxStep ) {
				maxStep = s;
				limitSide = -1;
			}
		}
	} else {
		if ( hi[d] != idMath::INFINITY ) {
			s = ( hi[d] - f[d] ) / dir;
			if ( s < maxStep ) {
				maxStep = s;
				limitSide = 1;
			}
		}
	}

	// test the clamped bounded variables
	for ( i = numUnbounded; i < numClamped; i++ ) {
		if ( delta_f[i] < -LCP_DELTA_FORCE_EPSILON ) {
			// if there is a low boundary
			if ( lo[i] != -idMath::INFINITY ) {
				s = ( lo[i] - f[i] ) / delta_f[i];
				if ( s < maxStep ) {
					maxStep = s;
					limit = i;
					limitSide = -1;
				}
			}
		} else if ( delta_f[i] > LCP_DELTA_FORCE_EPSILON ) {
			// if there is a high boundary
			if ( hi[i] != idMath::INFINITY ) {
				s = ( hi[i] - f[i] ) / delta_f[i];
				if ( s < maxStep ) {
					maxStep = s;
					limit = i;
					limitSide = 1;
				}
			}
		}
	}

	// test the not clamped bounded variables
	for ( i = numClamped; i < d; i++ ) {
		if ( side[i] == -1 ) {
			if ( delta_a[i] >= -LCP_DELTA_ACCEL_EPSILON ) {
				continue;
			}
		} else if ( side[i] == 1 ) {
			if ( delta_a[i] <= LCP_DELTA_ACCEL_EPSILON ) {
				continue;
			}
		} else {
			continue;
		}
		// ignore variables for which the force is not allowed to take any substantial value
		if ( lo[i] >= -LCP_BOUND_EPSILON && hi[i] <= LCP_BOUND_EPSILON ) {
			continue;
		}
		s = -a[i] / delta_a[i];
		if ( s < maxStep ) {
			maxStep = s;
			limit = i;
			limitSide = 0;
		}
	}
}

/*
============
idLCP_Symmetric::Solve
============
*/
bool idLCP_Symmetric::Solve( const idMatX &o_m, idVecX &o_x, const idVecX &o_b, const idVecX &o_lo, const idVecX &o_hi, const int *o_boxIndex ) {
	int i, j, n, limit, limitSide, boxStartIndex;
	float dir, maxStep, dot, s;
	char *failed;

	// true when the matrix rows are 16 byte padded
	padded = ((o_m.GetNumRows()+3)&~3) == o_m.GetNumColumns();

	assert( padded || o_m.GetNumRows() == o_m.GetNumColumns() );
	assert( o_x.GetSize() == o_m.GetNumRows() );
	assert( o_b.GetSize() == o_m.GetNumRows() );
	assert( o_lo.GetSize() == o_m.GetNumRows() );
	assert( o_hi.GetSize() == o_m.GetNumRows() );

	// allocate memory for permuted input
	f.SetData( o_m.GetNumRows(), VECX_ALLOCA( o_m.GetNumRows() ) );
	a.SetData( o_b.GetSize(), VECX_ALLOCA( o_b.GetSize() ) );
	b.SetData( o_b.GetSize(), VECX_ALLOCA( o_b.GetSize() ) );
	lo.SetData( o_lo.GetSize(), VECX_ALLOCA( o_lo.GetSize() ) );
	hi.SetData( o_hi.GetSize(), VECX_ALLOCA( o_hi.GetSize() ) );
	if ( o_boxIndex ) {
		boxIndex = (int *)_alloca16( o_x.GetSize() * sizeof( int ) );
		memcpy( boxIndex, o_boxIndex, o_x.GetSize() * sizeof( int ) );
	} else {
		boxIndex = NULL;
	}

	// we override the const on o_m here but on exit the matrix is unchanged
	m.SetData( o_m.GetNumRows(), o_m.GetNumColumns(), const_cast<float *>(o_m[0]) );
	f.Zero();
	a.Zero();
	b = o_b;
	lo = o_lo;
	hi = o_hi;

	// pointers to the rows of m
	rowPtrs = (float **) _alloca16( m.GetNumRows() * sizeof( float * ) );
	for ( i = 0; i < m.GetNumRows(); i++ ) {
		rowPtrs[i] = m[i];
	}

	// tells if a variable is at the low boundary, high boundary or inbetween
	side = (int *) _alloca16( m.GetNumRows() * sizeof( int ) );

	// index to keep track of the permutation
	permuted = (int *) _alloca16( m.GetNumRows() * sizeof( int ) );
	for ( i = 0; i < m.GetNumRows(); i++ ) {
		permuted[i] = i;
	}

	// permute input so all unbounded variables come first
	numUnbounded = 0;
	for ( i = 0; i < m.GetNumRows(); i++ ) {
		if ( lo[i] == -idMath::INFINITY && hi[i] == idMath::INFINITY ) {
			if ( numUnbounded != i ) {
				Swap( numUnbounded, i );
			}
			numUnbounded++;
		}
	}

	// permute input so all variables using the boxIndex come last
	boxStartIndex = m.GetNumRows();
	if ( boxIndex ) {
		for ( i = m.GetNumRows() - 1; i >= numUnbounded; i-- ) {
			if ( boxIndex[i] >= 0 && ( lo[i] != -idMath::INFINITY || hi[i] != idMath::INFINITY ) ) {
				boxStartIndex--;
				if ( boxStartIndex != i ) {
					Swap( boxStartIndex, i );
				}
			}
		}
	}

	// sub matrix for factorization 
	clamped.SetData( m.GetNumRows(), m.GetNumColumns(), MATX_ALLOCA( m.GetNumRows() * m.GetNumColumns() ) );
	diagonal.SetData( m.GetNumRows(), VECX_ALLOCA( m.GetNumRows() ) );
	solveCache1.SetData( m.GetNumRows(), VECX_ALLOCA( m.GetNumRows() ) );
	solveCache2.SetData( m.GetNumRows(), VECX_ALLOCA( m.GetNumRows() ) );

	// all unbounded variables are clamped
	numClamped = numUnbounded;

	// if there are unbounded variables
	if ( numUnbounded ) {

		// factor and solve for unbounded variables
		if ( !FactorClamped() ) {
			idLib::common->Printf( "idLCP_Symmetric::Solve: unbounded factorization failed\n" );
			return false;
		}
		SolveClamped( f, b.ToFloatPtr() );

		// if there are no bounded variables we are done
		if ( numUnbounded == m.GetNumRows() ) {
			o_x = f;	// the vector is not permuted
			return true;
		}
	}

#ifdef IGNORE_UNSATISFIABLE_VARIABLES
	int numIgnored = 0;
#endif

	// allocate for delta force and delta acceleration
	delta_f.SetData( m.GetNumRows(), VECX_ALLOCA( m.GetNumRows() ) );
	delta_a.SetData( m.GetNumRows(), VECX_ALLOCA( m.GetNumRows() ) );

	// solve for bounded variables
	failed = NULL;
	for ( i = numUnbounded; i < m.GetNumRows(); i++ ) {

		clampedChangeStart = 0;

		// once we hit the box start index we can initialize the low and high boundaries of the variables using the box index
		if ( i == boxStartIndex ) {
			for ( j = 0; j < boxStartIndex; j++ ) {
				o_x[permuted[j]] = f[j];
			}
			for ( j = boxStartIndex; j < m.GetNumRows(); j++ ) {
				s = o_x[boxIndex[j]];
				if ( lo[j] != -idMath::INFINITY ) {
					lo[j] = - idMath::Fabs( lo[j] * s );
				}
				if ( hi[j] != idMath::INFINITY ) {
					hi[j] = idMath::Fabs( hi[j] * s );
				}
			}
		}

		// calculate acceleration for current variable
		SIMDProcessor->Dot( dot, rowPtrs[i], f.ToFloatPtr(), i );
		a[i] = dot - b[i];

		// if already at the low boundary
		if ( lo[i] >= -LCP_BOUND_EPSILON && a[i] >= -LCP_ACCEL_EPSILON ) {
			side[i] = -1;
			continue;
		}

		// if already at the high boundary
		if ( hi[i] <= LCP_BOUND_EPSILON && a[i] <= LCP_ACCEL_EPSILON ) {
			side[i] = 1;
			continue;
		}

		// if inside the clamped region
		if ( idMath::Fabs( a[i] ) <= LCP_ACCEL_EPSILON ) {
			side[i] = 0;
			AddClamped( i, false );
			continue;
		}

		// drive the current variable into a valid region
		for ( n = 0; n < maxIterations; n++ ) {

			// direction to move
			if ( a[i] <= 0.0f ) {
				dir = 1.0f;
			} else {
				dir = -1.0f;
			}

			// calculate force delta
			CalcForceDelta( i, dir );

			// calculate acceleration delta: delta_a = m * delta_f;
			CalcAccelDelta( i );

			// maximum step we can take
			GetMaxStep( i, dir, maxStep, limit, limitSide );

			if ( maxStep <= 0.0f ) {
#ifdef IGNORE_UNSATISFIABLE_VARIABLES
				// ignore the current variable completely
				lo[i] = hi[i] = 0.0f;
				f[i] = 0.0f;
				side[i] = -1;
				numIgnored++;
#else
				failed = va( "invalid step size %.4f", maxStep );
#endif
				break;
			}

			// change force
			ChangeForce( i, maxStep );

			// change acceleration
			ChangeAccel( i, maxStep );

			// clamp/unclamp the variable that limited this step
			side[limit] = limitSide;
			switch( limitSide ) {
				case 0: {
					a[limit] = 0.0f;
					AddClamped( limit, ( limit == i ) );
					break;
				}
				case -1: {
					f[limit] = lo[limit];
					if ( limit != i ) {
						RemoveClamped( limit );
					}
					break;
				}
				case 1: {
					f[limit] = hi[limit];
					if ( limit != i ) {
						RemoveClamped( limit );
					}
					break;
				}
			}

			// if the current variable limited the step we can continue with the next variable
			if ( limit == i ) {
				break;
			}
		}

		if ( n >= maxIterations ) {
			failed = va( "max iterations %d", maxIterations );
			break;
		}

		if ( failed ) {
			break;
		}
	}

#ifdef IGNORE_UNSATISFIABLE_VARIABLES
	if ( numIgnored ) {
		if ( lcp_showFailures.GetBool() ) {
			idLib::common->Printf( "idLCP_Symmetric::Solve: %d of %d bounded variables ignored\n", numIgnored, m.GetNumRows() - numUnbounded );
		}
	}
#endif

	// if failed clear remaining forces
	if ( failed ) {
		if ( lcp_showFailures.GetBool() ) {
			idLib::common->Printf( "idLCP_Symmetric::Solve: %s (%d of %d bounded variables ignored)\n", failed, m.GetNumRows() - i, m.GetNumRows() - numUnbounded );
		}
		for ( j = i; j < m.GetNumRows(); j++ ) {
			f[j] = 0.0f;
		}
	}

#if defined(_DEBUG) && 0
	if ( !failed ) {
		// test whether or not the solution satisfies the complementarity conditions
		for ( i = 0; i < m.GetNumRows(); i++ ) {
			a[i] = -b[i];
			for ( j = 0; j < m.GetNumRows(); j++ ) {
				a[i] += rowPtrs[i][j] * f[j];
			}

			if ( f[i] == lo[i] ) {
				if ( lo[i] != hi[i] && a[i] < -LCP_ACCEL_EPSILON ) {
					int bah1 = 1;
				}
			} else if ( f[i] == hi[i] ) {
				if ( lo[i] != hi[i] && a[i] > LCP_ACCEL_EPSILON ) {
					int bah2 = 1;
				}
			} else if ( f[i] < lo[i] || f[i] > hi[i] || idMath::Fabs( a[i] ) > 1.0f ) {
				int bah3 = 1;
			}
		}
	}
#endif

	// unpermute result
	for ( i = 0; i < f.GetSize(); i++ ) {
		o_x[permuted[i]] = f[i];
	}

	// unpermute original matrix
	for ( i = 0; i < m.GetNumRows(); i++ ) {
		for ( j = 0; j < m.GetNumRows(); j++ ) {
			if ( permuted[j] == i ) {
				break;
			}
		}
		if ( i != j ) {
			m.SwapColumns( i, j );
			idSwap( permuted[i], permuted[j] );
		}
	}

	return true;
}


//===============================================================
//
//	idLCP
//
//===============================================================

/*
============
idLCP::AllocSquare
============
*/
idLCP *idLCP::AllocSquare( void ) {
	idLCP *lcp = new idLCP_Square;
	lcp->SetMaxIterations( 32 );
	return lcp;
}

/*
============
idLCP::AllocSymmetric
============
*/
idLCP *idLCP::AllocSymmetric( void ) {
	idLCP *lcp = new idLCP_Symmetric;
	lcp->SetMaxIterations( 32 );
	return lcp;
}

/*
============
idLCP::~idLCP
============
*/
idLCP::~idLCP( void ) {
}

/*
============
idLCP::SetMaxIterations
============
*/
void idLCP::SetMaxIterations( int max ) {
	maxIterations = max;
}

/*
============
idLCP::GetMaxIterations
============
*/
int idLCP::GetMaxIterations( void ) {
	return maxIterations;
}

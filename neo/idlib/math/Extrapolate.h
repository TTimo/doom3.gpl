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

#ifndef __MATH_EXTRAPOLATE_H__
#define __MATH_EXTRAPOLATE_H__

/*
==============================================================================================

	Extrapolation

==============================================================================================
*/

typedef enum {
	EXTRAPOLATION_NONE			= 0x01,	// no extrapolation, covered distance = duration * 0.001 * ( baseSpeed )
	EXTRAPOLATION_LINEAR		= 0x02,	// linear extrapolation, covered distance = duration * 0.001 * ( baseSpeed + speed )
	EXTRAPOLATION_ACCELLINEAR	= 0x04,	// linear acceleration, covered distance = duration * 0.001 * ( baseSpeed + 0.5 * speed )
	EXTRAPOLATION_DECELLINEAR	= 0x08,	// linear deceleration, covered distance = duration * 0.001 * ( baseSpeed + 0.5 * speed )
	EXTRAPOLATION_ACCELSINE		= 0x10,	// sinusoidal acceleration, covered distance = duration * 0.001 * ( baseSpeed + sqrt( 0.5 ) * speed )
	EXTRAPOLATION_DECELSINE		= 0x20,	// sinusoidal deceleration, covered distance = duration * 0.001 * ( baseSpeed + sqrt( 0.5 ) * speed )
	EXTRAPOLATION_NOSTOP		= 0x40	// do not stop at startTime + duration
} extrapolation_t;

template< class type >
class idExtrapolate {
public:
						idExtrapolate();

	void				Init( const float startTime, const float duration, const type &startValue, const type &baseSpeed, const type &speed, const extrapolation_t extrapolationType );
	type				GetCurrentValue( float time ) const;
	type				GetCurrentSpeed( float time ) const;
	bool				IsDone( float time ) const { return ( !( extrapolationType & EXTRAPOLATION_NOSTOP ) && time >= startTime + duration ); }
	void				SetStartTime( float time ) { startTime = time; currentTime = -1; }
	float				GetStartTime( void ) const { return startTime; }
	float				GetEndTime( void ) const { return ( !( extrapolationType & EXTRAPOLATION_NOSTOP ) && duration > 0 ) ? startTime + duration : 0; }
	float				GetDuration( void ) const { return duration; }
	void				SetStartValue( const type &value ) { startValue = value; currentTime = -1; }
	const type &		GetStartValue( void ) const { return startValue; }
	const type &		GetBaseSpeed( void ) const { return baseSpeed; }
	const type &		GetSpeed( void ) const { return speed; }
	extrapolation_t		GetExtrapolationType( void ) const { return extrapolationType; }

private:
	extrapolation_t		extrapolationType;
	float				startTime;
	float				duration;
	type				startValue;
	type				baseSpeed;
	type				speed;
	mutable float		currentTime;
	mutable type		currentValue;
};

/*
====================
idExtrapolate::idExtrapolate
====================
*/
template< class type >
ID_INLINE idExtrapolate<type>::idExtrapolate() {
	extrapolationType = EXTRAPOLATION_NONE;
	startTime = duration = 0.0f;
	memset( &startValue, 0, sizeof( startValue ) );
	memset( &baseSpeed, 0, sizeof( baseSpeed ) );
	memset( &speed, 0, sizeof( speed ) );
	currentTime = -1;
	currentValue = startValue;
}

/*
====================
idExtrapolate::Init
====================
*/
template< class type >
ID_INLINE void idExtrapolate<type>::Init( const float startTime, const float duration, const type &startValue, const type &baseSpeed, const type &speed, const extrapolation_t extrapolationType ) {
	this->extrapolationType = extrapolationType;
	this->startTime = startTime;
	this->duration = duration;
	this->startValue = startValue;
	this->baseSpeed = baseSpeed;
	this->speed = speed;
	currentTime = -1;
	currentValue = startValue;
}

/*
====================
idExtrapolate::GetCurrentValue
====================
*/
template< class type >
ID_INLINE type idExtrapolate<type>::GetCurrentValue( float time ) const {
	float deltaTime, s;

	if ( time == currentTime ) {
		return currentValue;
	}

	currentTime = time;

	if ( time < startTime ) {
		return startValue;
	}

	if ( !( extrapolationType &	EXTRAPOLATION_NOSTOP ) && ( time > startTime + duration ) ) {
		time = startTime + duration;
	}

	switch( extrapolationType & ~EXTRAPOLATION_NOSTOP ) {
		case EXTRAPOLATION_NONE: {
			deltaTime = ( time - startTime ) * 0.001f;
			currentValue = startValue + deltaTime * baseSpeed;
			break;
		}
		case EXTRAPOLATION_LINEAR: {
			deltaTime = ( time - startTime ) * 0.001f;
			currentValue = startValue + deltaTime * ( baseSpeed + speed );
			break;
		}
		case EXTRAPOLATION_ACCELLINEAR: {
			if ( !duration ) {
				currentValue = startValue;
			} else {
				deltaTime = ( time - startTime ) / duration;
				s = ( 0.5f * deltaTime * deltaTime ) * ( duration * 0.001f );
				currentValue = startValue + deltaTime * baseSpeed + s * speed;
			}
			break;
		}
		case EXTRAPOLATION_DECELLINEAR: {
			if ( !duration ) {
				currentValue = startValue;
			} else {
				deltaTime = ( time - startTime ) / duration;
				s = ( deltaTime - ( 0.5f * deltaTime * deltaTime ) ) * ( duration * 0.001f );
				currentValue = startValue + deltaTime * baseSpeed + s * speed;
			}
			break;
		}
		case EXTRAPOLATION_ACCELSINE: {
			if ( !duration ) {
				currentValue = startValue;
			} else {
				deltaTime = ( time - startTime ) / duration;
				s = ( 1.0f - idMath::Cos( deltaTime * idMath::HALF_PI ) ) * duration * 0.001f * idMath::SQRT_1OVER2;
				currentValue = startValue + deltaTime * baseSpeed + s * speed;
			}
			break;
		}
		case EXTRAPOLATION_DECELSINE: {
			if ( !duration ) {
				currentValue = startValue;
			} else {
				deltaTime = ( time - startTime ) / duration;
				s = idMath::Sin( deltaTime * idMath::HALF_PI ) * duration * 0.001f * idMath::SQRT_1OVER2;
				currentValue = startValue + deltaTime * baseSpeed + s * speed;
			}
			break;
		}
	}
	return currentValue;
}

/*
====================
idExtrapolate::GetCurrentSpeed
====================
*/
template< class type >
ID_INLINE type idExtrapolate<type>::GetCurrentSpeed( float time ) const {
	float deltaTime, s;

	if ( time < startTime || !duration ) {
		return ( startValue - startValue );
	}

	if ( !( extrapolationType &	EXTRAPOLATION_NOSTOP ) && ( time > startTime + duration ) ) {
		return ( startValue - startValue );
	}

	switch( extrapolationType & ~EXTRAPOLATION_NOSTOP ) {
		case EXTRAPOLATION_NONE: {
			return baseSpeed;
		}
		case EXTRAPOLATION_LINEAR: {
			return baseSpeed + speed;
		}
		case EXTRAPOLATION_ACCELLINEAR: {
			deltaTime = ( time - startTime ) / duration;
			s = deltaTime;
			return baseSpeed + s * speed;
		}
		case EXTRAPOLATION_DECELLINEAR: {
			deltaTime = ( time - startTime ) / duration;
			s = 1.0f - deltaTime;
			return baseSpeed + s * speed;
		}
		case EXTRAPOLATION_ACCELSINE: {
			deltaTime = ( time - startTime ) / duration;
			s = idMath::Sin( deltaTime * idMath::HALF_PI );
			return baseSpeed + s * speed;
		}
		case EXTRAPOLATION_DECELSINE: {
			deltaTime = ( time - startTime ) / duration;
			s = idMath::Cos( deltaTime * idMath::HALF_PI );
			return baseSpeed + s * speed;
		}
		default: {
			return baseSpeed;
		}
	}
}

#endif /* !__MATH_EXTRAPOLATE_H__ */

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

#include "../Game_local.h"

/*
================
idDebugGraph::idDebugGraph
================
*/
idDebugGraph::idDebugGraph() {
	index = 0;
}

/*
================
idDebugGraph::SetNumSamples
================
*/
void idDebugGraph::SetNumSamples( int num ) {
	index = 0;
	samples.Clear();
	samples.SetNum( num );
	memset( samples.Ptr(), 0, samples.MemoryUsed() );
}

/*
================
idDebugGraph::AddValue
================
*/
void idDebugGraph::AddValue( float value ) {
	samples[ index ] = value;
	index++;
	if ( index >= samples.Num() ) {
		index = 0;
	}
}

/*
================
idDebugGraph::Draw
================
*/
void idDebugGraph::Draw( const idVec4 &color, float scale ) const {
	int i;
	float value1;
	float value2;
	idVec3 vec1;
	idVec3 vec2;

	const idMat3 &axis = gameLocal.GetLocalPlayer()->viewAxis;
	const idVec3 pos = gameLocal.GetLocalPlayer()->GetPhysics()->GetOrigin() + axis[ 1 ] * samples.Num() * 0.5f;

	value1 = samples[ index ] * scale;
	for( i = 1; i < samples.Num(); i++ ) {
		value2 = samples[ ( i + index ) % samples.Num() ] * scale;

		vec1 = pos + axis[ 2 ] * value1 - axis[ 1 ] * ( i - 1 ) + axis[ 0 ] * samples.Num();
		vec2 = pos + axis[ 2 ] * value2 - axis[ 1 ] * i + axis[ 0 ] * samples.Num();

		gameRenderWorld->DebugLine( color, vec1, vec2, gameLocal.msec, false );
		value1 = value2;
	}
}

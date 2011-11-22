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

#include "splines.h"

idCameraDef splineList;
idCameraDef *g_splineList = &splineList;

/*
================
glLabeledPoint
================
*/
void glLabeledPoint(idVec4 &color, idVec3 &point, float size, const char *label) {
	qglColor3fv( color.ToFloatPtr() );
	qglPointSize( size );
	qglBegin( GL_POINTS );
	qglVertex3fv( point.ToFloatPtr() );
	qglEnd();
	idVec3 v = point;
	v.x += 1;
	v.y += 1;
	v.z += 1;
	qglRasterPos3fv( v.ToFloatPtr() );
	qglCallLists( strlen(label), GL_UNSIGNED_BYTE, label );
}

/*
================
glBox
================
*/
void glBox(idVec4 &color, idVec3 &point, float size) {
	idVec3 mins(point);
	idVec3 maxs(point);
	mins[0] -= size;
	mins[1] += size;
	mins[2] -= size;
	maxs[0] += size;
	maxs[1] -= size;
	maxs[2] += size;
	idVec4	saveColor;
	qglGetFloatv(GL_CURRENT_COLOR, saveColor.ToFloatPtr());
	qglColor3fv( color.ToFloatPtr() );
	qglBegin(GL_LINE_LOOP);
	qglVertex3f(mins[0],mins[1],mins[2]);
	qglVertex3f(maxs[0],mins[1],mins[2]);
	qglVertex3f(maxs[0],maxs[1],mins[2]);
	qglVertex3f(mins[0],maxs[1],mins[2]);
	qglEnd();
	qglBegin(GL_LINE_LOOP);
	qglVertex3f(mins[0],mins[1],maxs[2]);
	qglVertex3f(maxs[0],mins[1],maxs[2]);
	qglVertex3f(maxs[0],maxs[1],maxs[2]);
	qglVertex3f(mins[0],maxs[1],maxs[2]);
	qglEnd();

	qglBegin(GL_LINES);
  	qglVertex3f(mins[0],mins[1],mins[2]);
	qglVertex3f(mins[0],mins[1],maxs[2]);
	qglVertex3f(mins[0],maxs[1],maxs[2]);
	qglVertex3f(mins[0],maxs[1],mins[2]);
	qglVertex3f(maxs[0],mins[1],mins[2]);
	qglVertex3f(maxs[0],mins[1],maxs[2]);
	qglVertex3f(maxs[0],maxs[1],maxs[2]);
	qglVertex3f(maxs[0],maxs[1],mins[2]);
	qglEnd();
	qglColor4fv(saveColor.ToFloatPtr());

}

/*
================
splineTest
================
*/
void splineTest() {
	//g_splineList->load("p:/doom/base/maps/test_base1.camera");
}

/*
================
splineDraw
================
*/
void splineDraw() {
	//g_splineList->addToRenderer();
}

/*
================
debugLine
================
*/
void debugLine(idVec4 &color, float x, float y, float z, float x2, float y2, float z2) {
	idVec3 from(x, y, z);
	idVec3 to(x2, y2, z2);
	session->rw->DebugLine(color, from, to);
}


/*
=================================================================================

idPointListInterface

=================================================================================
*/

/*
================
idPointListInterface::selectPointByRay
================
*/
int idPointListInterface::selectPointByRay(const idVec3 &origin, const idVec3 &direction, bool single) {
	int		i, besti, count;
	float	d, bestd;
	idVec3	temp, temp2;

	// find the point closest to the ray
	besti = -1;
	bestd = 8;
	count = numPoints();

	for (i=0; i < count; i++) {
		temp = *getPoint(i);
		temp2 = temp;
		temp -= origin;
		d = DotProduct(temp, direction);
		VectorMA (origin, d, direction, temp);
		temp2 -= temp;
		d = temp2.Length();
		if (d <= bestd) {
			bestd = d;
			besti = i;
		}
	}

	if (besti >= 0) {
		selectPoint(besti, single);
	}

	return besti;
}

/*
================
idPointListInterface::isPointSelected
================
*/
int idPointListInterface::isPointSelected(int index) {
	int count = selectedPoints.Num();
	for (int i = 0; i < count; i++) {
		if (selectedPoints[i] == index) {
			return i;
		}
	}
	return -1;
}

/*
================
idPointListInterface::selectPoint
================
*/
int idPointListInterface::selectPoint(int index, bool single) {
	if (index >= 0 && index < numPoints()) {
		if (single) {
			deselectAll();
		} else {
			if (isPointSelected(index) >= 0) {
				selectedPoints.Remove(index);
			}
		}
		return selectedPoints.Append(index);
	}
	return -1;
}

/*
================
idPointListInterface::selectAll
================
*/
void idPointListInterface::selectAll() {
	selectedPoints.Clear();
	for (int i = 0; i < numPoints(); i++) {
		selectedPoints.Append(i);
	}
}

/*
================
idPointListInterface::deselectAll
================
*/
void idPointListInterface::deselectAll() {
	selectedPoints.Clear();
}

/*
================
idPointListInterface::getSelectedPoint
================
*/
idVec3 *idPointListInterface::getSelectedPoint( int index ) {
	assert(index >= 0 && index < numSelectedPoints());
	return getPoint(selectedPoints[index]);
}

/*
================
idPointListInterface::updateSelection
================
*/
void idPointListInterface::updateSelection(const idVec3 &move) {
	int count = selectedPoints.Num();
	for (int i = 0; i < count; i++) {
		*getPoint(selectedPoints[i]) += move;
	}
}

/*
================
idPointListInterface::drawSelection
================
*/
void idPointListInterface::drawSelection() {
	int count = selectedPoints.Num();
	for (int i = 0; i < count; i++) {
		glBox(colorRed, *getPoint(selectedPoints[i]), 4);
	}
}

/*
=================================================================================

idSplineList

=================================================================================
*/

/*
================
idSplineList::clearControl
================
*/
void idSplineList::clearControl() {
	for (int i = 0; i < controlPoints.Num(); i++) {
		delete controlPoints[i];
	}
	controlPoints.Clear();
}

/*
================
idSplineList::clearSpline
================
*/
void idSplineList::clearSpline() {
	for (int i = 0; i < splinePoints.Num(); i++) {
		delete splinePoints[i];
	}
	splinePoints.Clear();
}

/*
================
idSplineList::clear
================
*/
void idSplineList::clear() {
	clearControl();
	clearSpline();
	splineTime.Clear();
	selected = NULL;
	dirty = true;
	activeSegment = 0;
	granularity = 0.025f;
	pathColor = idVec4(1.0f, 0.5f, 0.0f, 1.0f);
	controlColor = idVec4(0.7f, 0.0f, 1.0f, 1.0f);
	segmentColor = idVec4(0.0f, 0.0f, 1.0f, 1.0);
	activeColor = idVec4(1.0f, 0.0f, 0.0f, 1.0f);
}

/*
================
idSplineList::setColors
================
*/
void idSplineList::setColors(idVec4 &path, idVec4 &segment, idVec4 &control, idVec4 &active) {
	pathColor = path;
	segmentColor = segment;
	controlColor = control;
	activeColor = active;
}

/*
================
idSplineList::validTime
================
*/
bool idSplineList::validTime() {
	if (dirty) {
		buildSpline();
	}
	// gcc doesn't allow static casting away from bools
	// why?  I've no idea...
	return (bool)(splineTime.Num() > 0 && splineTime.Num() == splinePoints.Num());
}

/*
================
idSplineList::addToRenderer
================
*/
void idSplineList::addToRenderer() {
	int i;
	idVec3 mins, maxs;

	if (controlPoints.Num() == 0) {
		return;
	}
        
	for(i = 0; i < controlPoints.Num(); i++) {
		VectorCopy(*controlPoints[i], mins);
		VectorCopy(mins, maxs);
		mins[0] -= 8;
		mins[1] += 8;
		mins[2] -= 8;
		maxs[0] += 8;
		maxs[1] -= 8;
		maxs[2] += 8;
		debugLine( colorYellow, mins[0], mins[1], mins[2], maxs[0], mins[1], mins[2]);
		debugLine( colorYellow, maxs[0], mins[1], mins[2], maxs[0], maxs[1], mins[2]);
		debugLine( colorYellow, maxs[0], maxs[1], mins[2], mins[0], maxs[1], mins[2]);
		debugLine( colorYellow, mins[0], maxs[1], mins[2], mins[0], mins[1], mins[2]);
		
		debugLine( colorYellow, mins[0], mins[1], maxs[2], maxs[0], mins[1], maxs[2]);
		debugLine( colorYellow, maxs[0], mins[1], maxs[2], maxs[0], maxs[1], maxs[2]);
		debugLine( colorYellow, maxs[0], maxs[1], maxs[2], mins[0], maxs[1], maxs[2]);
		debugLine( colorYellow, mins[0], maxs[1], maxs[2], mins[0], mins[1], maxs[2]);
	    
	}

	int step = 0;
	idVec3 step1;
	for(i = 3; i < controlPoints.Num(); i++) {
		for (float tension = 0.0f; tension < 1.001f; tension += 0.1f) {
			float x = 0;
			float y = 0;
			float z = 0;
			for (int j = 0; j < 4; j++) {
				x += controlPoints[i - (3 - j)]->x * calcSpline(j, tension);
				y += controlPoints[i - (3 - j)]->y * calcSpline(j, tension);
				z += controlPoints[i - (3 - j)]->z * calcSpline(j, tension);
			}
			if (step == 0) {
				step1[0] = x;
				step1[1] = y;
				step1[2] = z;
				step = 1;
			} else {
				debugLine( colorWhite, step1[0], step1[1], step1[2], x, y, z);
				step = 0;
			}

		}
	}
}

/*
================
idSplineList::buildSpline
================
*/
void idSplineList::buildSpline() {
	int start = Sys_Milliseconds();
	clearSpline();
	for(int i = 3; i < controlPoints.Num(); i++) {
		for (float tension = 0.0f; tension < 1.001f; tension += granularity) {
			float x = 0;
			float y = 0;
			float z = 0;
			for (int j = 0; j < 4; j++) {
				x += controlPoints[i - (3 - j)]->x * calcSpline(j, tension);
				y += controlPoints[i - (3 - j)]->y * calcSpline(j, tension);
				z += controlPoints[i - (3 - j)]->z * calcSpline(j, tension);
			}
			splinePoints.Append(new idVec3(x, y, z));
		}
	}
	dirty = false;
	//common->Printf("Spline build took %f seconds\n", (float)(Sys_Milliseconds() - start) / 1000);
}

/*
================
idSplineList::draw
================
*/
void idSplineList::draw(bool editMode) {
        int i;
        
	if (controlPoints.Num() == 0) {
		return;
	}

	if (dirty) {
		buildSpline();
	}


	qglColor3fv( controlColor.ToFloatPtr() );
	qglPointSize( 5 );
	
	qglBegin(GL_POINTS);
	for (i = 0; i < controlPoints.Num(); i++) {
		qglVertex3fv( (*controlPoints[i]).ToFloatPtr() );
	}
	qglEnd();
	
	if (editMode) {
		for(i = 0; i < controlPoints.Num(); i++) {
			glBox(activeColor, *controlPoints[i], 4);
		}
	}

	//Draw the curve
	qglColor3fv( pathColor.ToFloatPtr() );
	qglBegin(GL_LINE_STRIP);
	int count = splinePoints.Num();
	for (i = 0; i < count; i++) {
		qglVertex3fv( (*splinePoints[i]).ToFloatPtr() );
	}
	qglEnd();

	if (editMode) {
		qglColor3fv( segmentColor.ToFloatPtr() );
		qglPointSize(3);
		qglBegin(GL_POINTS);
		for (i = 0; i < count; i++) {
			qglVertex3fv( (*splinePoints[i]).ToFloatPtr() );
		}
		qglEnd();
	}
	if (count > 0) {
		//assert(activeSegment >=0 && activeSegment < count);
		if (activeSegment >=0 && activeSegment < count) {
			glBox(activeColor, *splinePoints[activeSegment], 6);
			glBox(colorYellow, *splinePoints[activeSegment], 8);
		}
	}

}

/*
================
idSplineList::totalDistance
================
*/
float idSplineList::totalDistance() {

	// FIXME: save dist and return
	// 
	if (controlPoints.Num() == 0) {
		return 0.0f;
	}

	if (dirty) {
		buildSpline();
	}

	float dist = 0.0f;
	idVec3 temp;
	int count = splinePoints.Num();
	for(int i = 1; i < count; i++) {
		temp = *splinePoints[i-1];
		temp -= *splinePoints[i];
		dist += temp.Length();
	}
	return dist;
}

/*
================
idSplineList::initPosition
================
*/
void idSplineList::initPosition(long bt, long totalTime) {

	if (dirty) {
		buildSpline();
	}

	if (splinePoints.Num() == 0) {
		return;
	}

	baseTime = bt;
	time = totalTime;

	// calc distance to travel ( this will soon be broken into time segments )
	splineTime.Clear();
	splineTime.Append(bt);
	double dist = totalDistance();
	double distSoFar = 0.0;
	idVec3 temp;
	int count = splinePoints.Num();
	//for(int i = 2; i < count - 1; i++) {
	for(int i = 1; i < count; i++) {
		temp = *splinePoints[i-1];
		temp -= *splinePoints[i];
		distSoFar += temp.Length();
		double percent = distSoFar / dist;
		percent *= totalTime;
		splineTime.Append(percent + bt);
	}
	assert(splineTime.Num() == splinePoints.Num());
	activeSegment = 0;
}

/*
================
idSplineList::calcSpline
================
*/
float idSplineList::calcSpline(int step, float tension) {
	switch(step) {
		case 0:	return (pow(1 - tension, 3)) / 6;
		case 1:	return (3 * pow(tension, 3) - 6 * pow(tension, 2) + 4) / 6;
		case 2:	return (-3 * pow(tension, 3) + 3 * pow(tension, 2) + 3 * tension + 1) / 6;
		case 3:	return pow(tension, 3) / 6;
	}
	return 0.0f;
}

/*
================
idSplineList::updateSelection
================
*/
void idSplineList::updateSelection(const idVec3 &move) {
	if (selected) {
		dirty = true;
		VectorAdd(*selected, move, *selected);
	}
}

/*
================
idSplineList::setSelectedPoint
================
*/
void idSplineList::setSelectedPoint(idVec3 *p) {
	if (p) {
		p->SnapInt();
		for(int i = 0; i < controlPoints.Num(); i++) {
			if ( (*p).Compare( *controlPoints[i], VECTOR_EPSILON ) ) {
				selected = controlPoints[i];
			}
		}
	} else {
		selected = NULL;
	}
}

/*
================
idSplineList::getPosition
================
*/
const idVec3 *idSplineList::getPosition(long t) {
	static idVec3 interpolatedPos;

	int count = splineTime.Num();
	if (count == 0) {
		return &vec3_zero;
	}

	assert(splineTime.Num() == splinePoints.Num());

#if 0
	float velocity = getVelocity(t);
	float timePassed = t - lastTime;
	lastTime = t;

	// convert to seconds	
	timePassed /= 1000;

	float distToTravel = timePassed * velocity;

	distSoFar += distToTravel;
	float tempDistance = 0;

	idVec3 temp;
	int count = splinePoints.Num();
	//for(int i = 2; i < count - 1; i++) {
	for(int i = 1; i < count; i++) {
		temp = *splinePoints[i-1];
		temp -= *splinePoints[i];
		tempDistance += temp.Length();
		if (tempDistance >= distSoFar) {
			break;
		}
	}

	if (i == count) {
		interpolatedPos = splinePoints[i-1];
	} else {
		double timeHi = splineTime[i + 1];
		double timeLo = splineTime[i - 1];
		double percent = (timeHi - t) / (timeHi - timeLo); 
		idVec3 v1 = *splinePoints[i - 1];
		idVec3 v2 = *splinePoints[i + 1];
		v2 *= (1.0f - percent);
		v1 *= percent;
		v2 += v1;
		interpolatedPos = v2;
	}
	return &interpolatedPos;

#else
	while (activeSegment < count) {
		if (splineTime[activeSegment] >= t) {
			if (activeSegment > 0 && activeSegment < count - 1) {
				double timeHi = splineTime[activeSegment + 1];
				double timeLo = splineTime[activeSegment - 1];
				//float percent = (float)(baseTime + time - t) / time;
				double percent = (timeHi - t) / (timeHi - timeLo); 
				// pick two bounding points
				idVec3 v1 = *splinePoints[activeSegment-1];
				idVec3 v2 = *splinePoints[activeSegment+1];
				v2 *= (1.0f - percent);
				v1 *= percent;
				v2 += v1;
				interpolatedPos = v2;
				return &interpolatedPos;
			}
			return splinePoints[activeSegment];
		} else {
			activeSegment++;
		}
	}
	return splinePoints[count-1];
#endif
}

/*
================
idSplineList::parse
================
*/
void idSplineList::parse( idParser *src ) {
	idToken token;
	idStr key;

	src->ExpectTokenString( "{" );

	while ( 1 ) {
		if ( !src->ExpectAnyToken( &token ) ) {
			break;
		}
		if ( token == "}" ) {
			break;
		}
		// if token is not a brace, it is a key for a key/value pair
		if ( token == "(" ) {
			src->UnreadToken( &token );
			// read the control point
			idVec3 point;
			src->Parse1DMatrix( 3, point.ToFloatPtr() );
			addPoint(point.x, point.y, point.z);
		}
		else {
			key = token;
			src->ReadTokenOnLine( &token );
			if ( !key.Icmp( "granularity" ) ) {
				granularity = atof(token.c_str());
			}
			else if ( !key.Icmp( "name" ) ) {
				name = token;
			}
			else {
				src->Error( "unknown spline list key: %s", key.c_str() );
				break;
			}
		}
	}
	dirty = true;
}

/*
================
idSplineList::write
================
*/
void idSplineList::write( idFile *f, const char *p) {
	f->Printf( "\t\t%s {\n", p );

	//f->Printf( "\t\tname %s\n", name.c_str() );
	f->Printf( "\t\t\tgranularity %f\n", granularity );
	int count = controlPoints.Num();
	for (int i = 0; i < count; i++) {
		f->Printf( "\t\t\t( %f %f %f )\n", controlPoints[i]->x, controlPoints[i]->y, controlPoints[i]->z );
	}
	f->Printf( "\t\t}\n" );
}

/*
=================================================================================

idCamaraDef

=================================================================================
*/

/*
================
idCameraDef::clear
================
*/
void idCameraDef::clear() {
	currentCameraPosition = 0;
	cameraRunning = false;
	lastDirection.Zero();
	baseTime = 30;
	activeTarget = 0;
	name = "camera01";
	fov.SetFOV(90);
	int i;
	for (i = 0; i < targetPositions.Num(); i++) {
		delete targetPositions[i];
	}
	for (i = 0; i < events.Num(); i++) {
		delete events[i];
	}
	delete cameraPosition;
	cameraPosition = NULL;
	events.Clear();
	targetPositions.Clear();
}

/*
================
idCameraDef::startNewCamera
================
*/
idCameraPosition *idCameraDef::startNewCamera( idCameraPosition::positionType type ) {
	clear();
	if (type == idCameraPosition::SPLINE) {
		cameraPosition = new idSplinePosition();
	} else if (type == idCameraPosition::INTERPOLATED) {
		cameraPosition = new idInterpolatedPosition();
	} else {
		cameraPosition = new idFixedPosition();
	}
	return cameraPosition;
}

/*
================
idCameraDef::addTarget
================
*/
void idCameraDef::addTarget(const char *name, idCameraPosition::positionType type) {
	const char *text = (name == NULL) ? va("target0%d", numTargets()+1) : name;
	idCameraPosition *pos = newFromType(type);
	if (pos) {
		pos->setName(name);
		targetPositions.Append(pos);
		activeTarget = numTargets()-1;
		if (activeTarget == 0) {
			// first one
			addEvent(idCameraEvent::EVENT_TARGET, name, 0);
		}
	}
}

/*
================
idCameraDef::getActiveTarget
================
*/
idCameraPosition *idCameraDef::getActiveTarget() {
	if (targetPositions.Num() == 0) {
		addTarget(NULL, idCameraPosition::FIXED);
	}
	return targetPositions[activeTarget];
}

/*
================
idCameraDef::getActiveTarget
================
*/
idCameraPosition *idCameraDef::getActiveTarget(int index) {
	if (targetPositions.Num() == 0) {
		addTarget(NULL, idCameraPosition::FIXED);
		return targetPositions[0];
	}
	return targetPositions[index];
}

/*
================
idCameraDef::setActiveTargetByName
================
*/
void idCameraDef::setActiveTargetByName( const char *name ) {
	for (int i = 0; i < targetPositions.Num(); i++) {
		if (idStr::Icmp(name, targetPositions[i]->getName()) == 0) {
			setActiveTarget(i);
			return;
		}
	}
}

/*
================
idCameraDef::setActiveTarget
================
*/
void idCameraDef::setActiveTarget( int index ) {
	assert(index >= 0 && index < targetPositions.Num());
	activeTarget = index;
}

/*
================
idCameraDef::draw
================
*/
void idCameraDef::draw( bool editMode ) {
            // gcc doesn't allow casting away from bools
            // why?  I've no idea...
	if (cameraPosition) {
		cameraPosition->draw((bool)((editMode || cameraRunning) && cameraEdit));
		int count = targetPositions.Num();
		for (int i = 0; i < count; i++) {
			targetPositions[i]->draw((bool)((editMode || cameraRunning) && i == activeTarget && !cameraEdit));
		}
	}
}

/*
================
idCameraDef::numPoints
================
*/
int idCameraDef::numPoints() {
	if (cameraEdit) {
		return cameraPosition->numPoints();
	}
	return getActiveTarget()->numPoints();
}

/*
================
idCameraDef::getPoint
================
*/
const idVec3 *idCameraDef::getPoint(int index) {
	if (cameraEdit) {
		return cameraPosition->getPoint(index);
	}
	return getActiveTarget()->getPoint(index);
}

/*
================
idCameraDef::stopEdit
================
*/
void idCameraDef::stopEdit() {
	editMode = false;
	if (cameraEdit) {
		cameraPosition->stopEdit();
	} else {
		getActiveTarget()->stopEdit();
	}
}

/*
================
idCameraDef::startEdit
================
*/
void idCameraDef::startEdit(bool camera) {
	cameraEdit = camera;
	if (camera) {
		cameraPosition->startEdit();
		for (int i = 0; i < targetPositions.Num(); i++) {
			targetPositions[i]->stopEdit();
		}
	} else {
		getActiveTarget()->startEdit();
		cameraPosition->stopEdit();
	}
	editMode = true;
}

/*
================
idCameraDef::getPositionObj
================
*/
idCameraPosition *idCameraDef::getPositionObj() {
	if (cameraPosition == NULL) {
		cameraPosition = new idFixedPosition();
	}
	return cameraPosition;
}

/*
================
idCameraDef::getActiveSegmentInfo
================
*/
void idCameraDef::getActiveSegmentInfo(int segment, idVec3 &origin, idVec3 &direction, float *fov) {
#if 0
	if (!cameraSpline.validTime()) {
		buildCamera();
	}
	double d = (double)segment / numSegments();
	getCameraInfo(d * totalTime * 1000, origin, direction, fov);
#endif
/*
	if (!cameraSpline.validTime()) {
		buildCamera();
	}
	origin = *cameraSpline.getSegmentPoint(segment);
	

	idVec3 temp;

	int numTargets = getTargetSpline()->controlPoints.Num();
	int count = cameraSpline.splineTime.Num();
	if (numTargets == 0) {
		// follow the path
		if (cameraSpline.getActiveSegment() < count - 1) {
			temp = *cameraSpline.splinePoints[cameraSpline.getActiveSegment()+1];
		}
	} else if (numTargets == 1) {
		temp = *getTargetSpline()->controlPoints[0];
	} else {
		temp = *getTargetSpline()->getSegmentPoint(segment);
	}

	temp -= origin;
	temp.Normalize();
	direction = temp;
*/
}

/*
================
idCameraDef::getCameraInfo
================
*/
bool idCameraDef::getCameraInfo(long time, idVec3 &origin, idVec3 &direction, float *fv) {
	char	buff[ 1024 ];
	int		i;

	if ((time - startTime) / 1000 <= totalTime) {

		for( i = 0; i < events.Num(); i++ ) {
			if (time >= startTime + events[i]->getTime() && !events[i]->getTriggered()) {
				events[i]->setTriggered(true);
				if (events[i]->getType() == idCameraEvent::EVENT_TARGET) {
					setActiveTargetByName(events[i]->getParam());
					getActiveTarget()->start(startTime + events[i]->getTime());
					//common->Printf("Triggered event switch to target: %s\n",events[i]->getParam());
				} else if (events[i]->getType() == idCameraEvent::EVENT_TRIGGER) {
#if 0
//FIXME: seperate game and editor spline code
					idEntity *ent;
					ent = gameLocal.FindEntity( events[i]->getParam() );
					if (ent) {
						ent->Signal( SIG_TRIGGER );
						ent->ProcessEvent( &EV_Activate, gameLocal.world );
					}
#endif
				} else if (events[i]->getType() == idCameraEvent::EVENT_FOV) {
					memset(buff, 0, sizeof(buff));
					strcpy(buff, events[i]->getParam());
					const char *param1 = strtok(buff, " \t,\0");
					const char *param2 = strtok(NULL, " \t,\0");
					fov.reset(fov.GetFOV(time), atof(param1), time, atoi(param2)); 
					//*fv = fov = atof(events[i]->getParam());
				} else if (events[i]->getType() == idCameraEvent::EVENT_CAMERA) {
				} else if (events[i]->getType() == idCameraEvent::EVENT_STOP) {
					return false;
				}
			}
		}
	} else {
	}

	origin = *cameraPosition->getPosition(time);
	
	*fv = fov.GetFOV(time);

	idVec3 temp = origin;

	int numTargets = targetPositions.Num();
	if (numTargets == 0) {
/*
		// follow the path
		if (cameraSpline.getActiveSegment() < count - 1) {
			temp = *cameraSpline.splinePoints[cameraSpline.getActiveSegment()+1];
			if (temp == origin) {
				int index = cameraSpline.getActiveSegment() + 2;
				while (temp == origin && index < count - 1) {
					temp = *cameraSpline.splinePoints[index++];
				}
			}
		}
*/
	} else {
		temp = *getActiveTarget()->getPosition(time);
	}
	
	temp -= origin;
	temp.Normalize();
	direction = temp;

	return true;
}

/*
================
idCameraDef::waitEvent
================
*/
bool idCameraDef::waitEvent(int index) {
	//for (int i = 0; i < events.Num(); i++) {
	//	if (events[i]->getSegment() == index && events[i]->getType() == idCameraEvent::EVENT_WAIT) {
	//		return true;
	//	}
    //}
	return false;
}

/*
================
idCameraDef::buildCamera
================
*/
#define NUM_CCELERATION_SEGS 10
#define CELL_AMT 5

void idCameraDef::buildCamera() {
	int i;
	int lastSwitch = 0;
	idList<float> waits;
	idList<int> targets;

	totalTime = baseTime;
	cameraPosition->setTime(totalTime * 1000);
	// we have a base time layout for the path and the target path
	// now we need to layer on any wait or speed changes
	for (i = 0; i < events.Num(); i++) {
		idCameraEvent *ev = events[i];
		events[i]->setTriggered(false);
		switch (events[i]->getType()) {
			case idCameraEvent::EVENT_TARGET : {
				targets.Append(i);
				break;
			}
			case idCameraEvent::EVENT_FEATHER : {
				long startTime = 0;
				float speed = 0;
				long loopTime = 10;
				float stepGoal = cameraPosition->getBaseVelocity() / (1000 / loopTime);
				while (startTime <= 1000) {
					cameraPosition->addVelocity(startTime, loopTime, speed);
					speed += stepGoal;
					if (speed > cameraPosition->getBaseVelocity()) {
						speed = cameraPosition->getBaseVelocity();
					}
					startTime += loopTime;
				}

				startTime = totalTime * 1000 - 1000;
				long endTime = startTime + 1000;
				speed = cameraPosition->getBaseVelocity();
				while (startTime < endTime) {
					speed -= stepGoal;
					if (speed < 0) {
						speed = 0;
					}
					cameraPosition->addVelocity(startTime, loopTime, speed);
					startTime += loopTime;
				}
				break;

			}
			case idCameraEvent::EVENT_WAIT : {
				waits.Append(atof(events[i]->getParam()));

				//FIXME: this is quite hacky for Wolf E3, accel and decel needs
				// do be parameter based etc.. 
				long startTime = events[i]->getTime() - 1000;
				if (startTime < 0) {
					startTime = 0;
				}
				float speed = cameraPosition->getBaseVelocity();
				long loopTime = 10;
				float steps = speed / ((events[i]->getTime() - startTime) / loopTime);
				while (startTime <= events[i]->getTime() - loopTime) {
					cameraPosition->addVelocity(startTime, loopTime, speed);
					speed -= steps;
					startTime += loopTime;
				}
				cameraPosition->addVelocity(events[i]->getTime(), atof(events[i]->getParam()) * 1000, 0);

				startTime = events[i]->getTime() + atof(events[i]->getParam()) * 1000;
				long endTime = startTime + 1000;
				speed = 0;
				while (startTime <= endTime) {
					cameraPosition->addVelocity(startTime, loopTime, speed);
					speed += steps;
					startTime += loopTime;
				}
				break;
			}
			case idCameraEvent::EVENT_TARGETWAIT : {
				//targetWaits.Append(i);
				break;
			}
			case idCameraEvent::EVENT_SPEED : {
/*
				// take the average delay between up to the next five segments
				float adjust = atof(events[i]->getParam());
				int index = events[i]->getSegment();
				total = 0;
				count = 0;

				// get total amount of time over the remainder of the segment
				for (j = index; j < cameraSpline.numSegments() - 1; j++) {
					total += cameraSpline.getSegmentTime(j + 1) - cameraSpline.getSegmentTime(j);
					count++;
				}

				// multiply that by the adjustment
				double newTotal = total * adjust;
				// what is the difference.. 
				newTotal -= total;
				totalTime += newTotal / 1000;

				// per segment difference
				newTotal /= count;
				int additive = newTotal;

				// now propogate that difference out to each segment
				for (j = index; j < cameraSpline.numSegments(); j++) {
					cameraSpline.addSegmentTime(j, additive);
					additive += newTotal;
				}
				break;
*/
			}
		}
	}


	for (i = 0; i < waits.Num(); i++) {
		totalTime += waits[i];
	}

	// on a new target switch, we need to take time to this point ( since last target switch ) 
	// and allocate it across the active target, then reset time to this point
	long timeSoFar = 0;
	long total = totalTime * 1000;
	for (i = 0; i < targets.Num(); i++) {
		long t;
		if (i < targets.Num() - 1) {
			t = events[targets[i+1]]->getTime();
		} else {
			t = total - timeSoFar;
		}
		// t is how much time to use for this target
		setActiveTargetByName(events[targets[i]]->getParam());
		getActiveTarget()->setTime(t);
		timeSoFar += t;
	}
}

/*
================
idCameraDef::startCamera
================
*/
void idCameraDef::startCamera(long t) {
	cameraPosition->clearVelocities();
	cameraPosition->start(t);
	buildCamera();
	//for (int i = 0; i < targetPositions.Num(); i++) {
	//	targetPositions[i]->
	//}
	startTime = t;
	cameraRunning = true;
}

/*
================
idCameraDef::parse
================
*/
void idCameraDef::parse( idParser *src  ) {
	idToken token;

	src->ReadToken(&token);
	src->ExpectTokenString( "{" );
	while ( 1 ) {

		src->ExpectAnyToken( &token );

		if ( token == "}" ) {
			break;
		}
		else if ( !token.Icmp( "time" ) ) {
			baseTime = src->ParseFloat();
		}
		else if ( !token.Icmp( "camera_fixed") ) {
			cameraPosition = new idFixedPosition();
			cameraPosition->parse( src );
		}
		else if ( !token.Icmp( "camera_interpolated") ) {
			cameraPosition = new idInterpolatedPosition();
			cameraPosition->parse( src );
		}
		else if ( !token.Icmp( "camera_spline") ) {
			cameraPosition = new idSplinePosition();
			cameraPosition->parse( src );
		}
		else if ( !token.Icmp( "target_fixed") ) {
			idFixedPosition *pos = new idFixedPosition();
			pos->parse( src );
			targetPositions.Append(pos);
		}
		else if ( !token.Icmp( "target_interpolated") ) {
			idInterpolatedPosition *pos = new idInterpolatedPosition();
			pos->parse( src );
			targetPositions.Append(pos);
		}
		else if ( !token.Icmp( "target_spline") ) {
			idSplinePosition *pos = new idSplinePosition();
			pos->parse( src );
			targetPositions.Append(pos);
		}
		else if ( !token.Icmp( "fov") ) {
			fov.parse( src );
		}
		else if ( !token.Icmp( "event") ) {
			idCameraEvent *event = new idCameraEvent();
			event->parse( src );
			addEvent(event);
		}
		else {
			src->Error( "unknown camera def: %s", token.c_str() );
			break;
		}
	}

	if ( !cameraPosition ) {
		common->Printf( "no camera position specified\n" );
		// prevent a crash later on
		cameraPosition = new idFixedPosition();
	}
}

/*
================
idCameraDef::load
================
*/
bool idCameraDef::load( const char *filename ) {
	idParser *src;

	src = new idParser( filename, LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );
	if ( !src->IsLoaded() ) {
		common->Printf( "couldn't load %s\n", filename );
		delete src;
		return false;
	}

	clear();
	parse( src );

	delete src;

	return true;
}

/*
================
idCameraDef::save
================
*/
void idCameraDef::save(const char *filename) {
	idFile *f = fileSystem->OpenFileWrite( filename, "fs_devpath" );
	if ( f ) {
		int i;
		f->Printf( "cameraPathDef { \n" );
		f->Printf( "\ttime %f\n", baseTime );

		cameraPosition->write( f, va("camera_%s",cameraPosition->typeStr()) );

		for (i = 0; i < numTargets(); i++) {
			targetPositions[i]->write( f, va("target_%s", targetPositions[i]->typeStr()) );
		}

		for (i = 0; i < events.Num(); i++) {
			events[i]->write( f, "event" );
		}

		fov.write( f, "fov" );

		f->Printf( "}\n" );
	}
	fileSystem->CloseFile( f );
}

/*
================
idCameraDef::sortEvents
================
*/
int idCameraDef::sortEvents(const void *p1, const void *p2) {
	idCameraEvent *ev1 = (idCameraEvent*)(p1);
	idCameraEvent *ev2 = (idCameraEvent*)(p2);

	if (ev1->getTime() > ev2->getTime()) {
		return -1;
	}
	if (ev1->getTime() < ev2->getTime()) {
		return 1;
	}
	return 0; 
}

/*
================
idCameraDef::addEvent
================
*/
void idCameraDef::addEvent(idCameraEvent *event) {
	events.Append(event);
	//events.Sort(&sortEvents);

}

/*
================
idCameraDef::addEvent
================
*/
void idCameraDef::addEvent(idCameraEvent::eventType t, const char *param, long time) {
	addEvent(new idCameraEvent(t, param, time));
	buildCamera();
}

/*
================
idCameraDef::newFromType
================
*/
idCameraPosition *idCameraDef::newFromType( idCameraPosition::positionType t ) {
	switch (t) {
		case idCameraPosition::FIXED : return new idFixedPosition();
		case idCameraPosition::INTERPOLATED : return new idInterpolatedPosition();
		case idCameraPosition::SPLINE : return new idSplinePosition();
	};
	return NULL;
}


/*
=================================================================================

idCamaraEvent

=================================================================================
*/

/*
================
idCameraEvent::eventStr
================
*/
const char *idCameraEvent::eventStr[] = {
	"NA",
	"WAIT",
	"TARGETWAIT",
	"SPEED",
	"TARGET",
	"SNAPTARGET",
	"FOV",
	"CMD",
	"TRIGGER",
	"STOP",
	"CAMERA",
	"FADEOUT",
	"FADEIN",
	"FEATHER"
};

/*
================
idCameraEvent::parse
================
*/
void idCameraEvent::parse( idParser *src ) {
	idToken token;
	idStr key;

	src->ExpectTokenString( "{" );

	while ( 1 ) {

		if ( !src->ExpectAnyToken( &token ) ) {
			break;
		}
		if ( token == "}" ) {
			break;
		}

		key = token;
		src->ReadTokenOnLine( &token );
		if ( !key.Icmp( "type" ) ) {
			type = static_cast<idCameraEvent::eventType>(atoi(token.c_str()));
		}
		else if ( !key.Icmp( "param" ) ) {
			paramStr = token;
		}
		else if ( !key.Icmp( "time" ) ) {
			time = atoi(token.c_str());
		}
		else {
			src->Error( "unknown camera event key: %s", key.c_str() );
			break;
		}
	}
}

/*
================
idCameraEvent::write
================
*/
void idCameraEvent::write( idFile *f, const char *name) {
	f->Printf( "\t%s {\n", name );
	f->Printf( "\t\ttype %d\n", static_cast<int>(type) );
	f->Printf( "\t\tparam \"%s\"\n", paramStr.c_str() );
	f->Printf( "\t\ttime %d\n", time );
	f->Printf( "\t}\n" );
}

/*
=================================================================================

idCamaraPosition

=================================================================================
*/

/*
================
idCameraPosition::positionStr
================
*/
const char *idCameraPosition::positionStr[] = {
	"Fixed",
	"Interpolated",
	"Spline",
};

/*
================
idCameraPosition::positionStr
================
*/
void idCameraPosition::clearVelocities() {
	for (int i = 0; i < velocities.Num(); i++) {
		delete velocities[i];
		velocities[i] = NULL;
	}
	velocities.Clear();
}

/*
================
idCameraPosition::positionStr
================
*/
float idCameraPosition::getVelocity( long t ) {
	long check = t - startTime;
	for ( int i = 0; i < velocities.Num(); i++ ) {
		if (check >= velocities[i]->startTime && check <= velocities[i]->startTime + velocities[i]->time) {
			return velocities[i]->speed;
		}
	}
	return baseVelocity;
}

/*
================
idCameraPosition::parseToken
================
*/
bool idCameraPosition::parseToken( const idStr &key, idParser *src ) {
	idToken token;

	if ( !key.Icmp( "time" ) ) {
		time = src->ParseInt();
		return true;
	}
	else if ( !key.Icmp( "type" ) ) {
		type = static_cast<idCameraPosition::positionType> ( src->ParseInt() );
		return true;
	}
	else if ( !key.Icmp( "velocity" ) ) {
		long t = atol(token);
		long d = src->ParseInt();
		float s = src->ParseFloat();
		addVelocity(t, d, s);
		return true;
	}
	else if ( !key.Icmp( "baseVelocity" ) ) {
		baseVelocity = src->ParseFloat();
		return true;
	}
	else if ( !key.Icmp( "name" ) ) {
		src->ReadToken( &token );
		name = token;
		return true;
	}
	else if ( !key.Icmp( "time" ) ) {
		time = src->ParseInt();
		return true;
	}
	else {
		src->Error( "unknown camera position key: %s", key.c_str() );
		return false;
	}
}

/*
================
idCameraPosition::write
================
*/
void idCameraPosition::write( idFile *f, const char *p ) {
	f->Printf( "\t\ttime %i\n", time );
	f->Printf( "\t\ttype %i\n", static_cast<int>(type) );
	f->Printf( "\t\tname %s\n", name.c_str() );
	f->Printf( "\t\tbaseVelocity %f\n", baseVelocity );
	for (int i = 0; i < velocities.Num(); i++) {
		f->Printf( "\t\tvelocity %i %i %f\n", velocities[i]->startTime, velocities[i]->time, velocities[i]->speed );
	}
}

/*
=================================================================================

idInterpolatedPosition

=================================================================================
*/

/*
================
idInterpolatedPosition::getPoint
================
*/
idVec3 *idInterpolatedPosition::getPoint( int index ) {
	assert( index >= 0 && index < 2 );
	if ( index == 0 ) {
		return &startPos;
	}
	return &endPos;
}

/*
================
idInterpolatedPosition::addPoint
================
*/
void idInterpolatedPosition::addPoint( const float x, const float y, const float z ) {
	if (first) {
		startPos.Set(x, y, z);
		first = false;
	} else {
		endPos.Set(x, y, z);
		first = true;
	}
}

/*
================
idInterpolatedPosition::addPoint
================
*/
void idInterpolatedPosition::addPoint( const idVec3 &v ) {
	if (first) {
		startPos = v;
		first = false;
	}
	else {
		endPos = v;
		first = true;
	}
}

/*
================
idInterpolatedPosition::draw
================
*/
void idInterpolatedPosition::draw( bool editMode ) {
	glLabeledPoint(colorBlue, startPos, (editMode) ? 5 : 3, "Start interpolated");
	glLabeledPoint(colorBlue, endPos, (editMode) ? 5 : 3, "End interpolated");
	qglBegin(GL_LINES);
	qglVertex3fv( startPos.ToFloatPtr() );
	qglVertex3fv( endPos.ToFloatPtr() );
	qglEnd();
}

/*
================
idInterpolatedPosition::start
================
*/
void idInterpolatedPosition::start( long t ) {
	idCameraPosition::start(t);
	lastTime = startTime;
	distSoFar = 0.0f;
	idVec3 temp = startPos;
	temp -= endPos;
	calcVelocity(temp.Length());
}

/*
================
idInterpolatedPosition::getPosition
================
*/
const idVec3 *idInterpolatedPosition::getPosition( long t ) { 
	static idVec3 interpolatedPos;

	if (t - startTime > 6000) {
		int i = 0;
	}

	float velocity = getVelocity(t);
	float timePassed = t - lastTime;
	lastTime = t;

	// convert to seconds	
	timePassed /= 1000;

	if (velocity != getBaseVelocity()) {
		int i = 0;
	}

	float distToTravel = timePassed * velocity;

	idVec3 temp = startPos;
	temp -= endPos;
	float distance = temp.Length();

	distSoFar += distToTravel;
	float percent = (float)(distSoFar) / distance;

	if ( percent > 1.0f ) {
		percent = 1.0f;
	} else if ( percent < 0.0f ) {
		percent = 0.0f;
	}

	// the following line does a straigt calc on percentage of time
	// float percent = (float)(startTime + time - t) / time;

	idVec3 v1 = startPos;
	idVec3 v2 = endPos;
	v1 *= (1.0f - percent);
	v2 *= percent;
	v1 += v2;
	interpolatedPos = v1;
	return &interpolatedPos;
}

/*
================
idInterpolatedPosition::parse
================
*/
void idInterpolatedPosition::parse( idParser *src ) {
	idToken token;

	src->ExpectTokenString( "{" );
	while ( 1 ) {
		if ( !src->ExpectAnyToken( &token ) ) {
			break;
		}
		if ( token == "}" ) {
			break;
		}

		if ( !token.Icmp( "startPos" ) ) {
			src->Parse1DMatrix( 3, startPos.ToFloatPtr() );
		}
		else if ( !token.Icmp( "endPos" ) ) {
			src->Parse1DMatrix( 3, endPos.ToFloatPtr() );
		}
		else {
			idCameraPosition::parseToken( token, src);
		}
	}
}

/*
================
idInterpolatedPosition::write
================
*/
void idInterpolatedPosition::write( idFile *f, const char *p ) {
	f->Printf( "\t%s {\n", p );
	idCameraPosition::write( f, p );
	f->Printf( "\t\tstartPos ( %f %f %f )\n", startPos.x, startPos.y, startPos.z );
	f->Printf( "\t\tendPos ( %f %f %f )\n", endPos.x, endPos.y, endPos.z );
	f->Printf( "\t}\n" );
}

/*
=================================================================================

idCameraFOV

=================================================================================
*/

/*
================
idCameraFOV::GetFOV
================
*/
float idCameraFOV::GetFOV( long t ) {
	if (time) {
		assert(startTime);
		float percent = (t - startTime) / length;
		if ( percent < 0.0f ) {
			percent = 0.0f;
		} else if ( percent > 1.0f ) {
			percent = 1.0f;
		}
		float temp = endFOV - startFOV;
		temp *= percent;
		fov = startFOV + temp;
	}
	return fov;
}

/*
================
idCameraFOV::reset
================
*/
void idCameraFOV::reset( float startfov, float endfov, int start, int len ) {
	startFOV = startfov;
	endFOV = endfov;
	startTime = start;
	length = len;
}

/*
================
idCameraFOV::parse
================
*/
void idCameraFOV::parse( idParser *src ) {
	idToken token;

	src->ExpectTokenString( "{" );
	while ( 1 ) {
		if ( !src->ExpectAnyToken( &token ) ) {
			break;
		}
		if ( token == "}" ) {
			break;
		}

		if ( !token.Icmp( "fov" ) ) {
			fov = src->ParseFloat();
		}
		else if ( !token.Icmp( "startFOV" ) ) {
			startFOV = src->ParseFloat();
		}
		else if ( !token.Icmp( "endFOV" ) ) {
			endFOV = src->ParseFloat();
		}
		else if ( !token.Icmp( "time" ) ) {
			time = src->ParseInt();
		}
		else {
			src->Error( "unknown camera FOV key: %s", token.c_str() );
			break;
		}
	}
}

/*
================
idCameraFOV::write
================
*/
void idCameraFOV::write( idFile *f, const char *p ) {
	f->Printf( "\t%s {\n", p );
	f->Printf( "\t\tfov %f\n", fov );
	f->Printf( "\t\tstartFOV %f\n", startFOV );
	f->Printf( "\t\tendFOV %f\n", endFOV );
	f->Printf( "\t\ttime %i\n", time );
	f->Printf( "\t}\n" );
}

/*
=================================================================================

idFixedPosition

=================================================================================
*/

/*
================
idFixedPosition::parse
================
*/
void idFixedPosition::parse( idParser *src ) {
	idToken token;

	src->ExpectTokenString( "{" );
	while ( 1 ) {
		if ( !src->ExpectAnyToken( &token ) ) {
			break;
		}
		if ( token == "}" ) {
			break;
		}

		if ( !token.Icmp( "pos" ) ) {
			src->Parse1DMatrix( 3, pos.ToFloatPtr() );
		}
		else {
			idCameraPosition::parseToken( token, src );
		}
	}
}

/*
================
idFixedPosition::write
================
*/
void idFixedPosition::write( idFile *f, const char *p ) {
	f->Printf( "\t%s {\n", p );
	idCameraPosition::write( f, p );
	f->Printf( "\t\tpos ( %f %f %f )\n", pos.x, pos.y, pos.z );
	f->Printf( "\t}\n" );
}

/*
=================================================================================

idSplinePosition

=================================================================================
*/

/*
================
idSplinePosition::start
================
*/
void idSplinePosition::start( long t ) {
	idCameraPosition::start( t );
	target.initPosition(t, time);
	lastTime = startTime;
	distSoFar = 0.0f;
	calcVelocity(target.totalDistance());
}

/*
================
idSplinePosition::parse
================
*/
void idSplinePosition::parse( idParser *src ) {
	idToken token;

	src->ExpectTokenString( "{" );
	while ( 1 ) {
		if ( !src->ExpectAnyToken( &token ) ) {
			break;
		}
		if ( token == "}" ) {
			break;
		}
		if ( !token.Icmp( "target" ) ) {
			target.parse( src );
		}
		else {
			idCameraPosition::parseToken( token, src );	
		}
	}
}

/*
================
idSplinePosition::write
================
*/
void idSplinePosition::write( idFile *f, const char *p ) {
	f->Printf( "\t%s {\n", p );
	idCameraPosition::write( f, p );
	target.write( f, "target" );
	f->Printf( "\t}\n" );
}

/*
================
idSplinePosition::getPosition
================
*/
const idVec3 *idSplinePosition::getPosition(long t) {
	static idVec3 interpolatedPos;

	float velocity = getVelocity(t);
	float timePassed = t - lastTime;
	lastTime = t;

	// convert to seconds	
	timePassed /= 1000;

	float distToTravel = timePassed * velocity;

	distSoFar += distToTravel;
	double tempDistance = target.totalDistance();

	double percent = (double)(distSoFar) / tempDistance;

	double targetDistance = percent * tempDistance;
	tempDistance = 0;

	double lastDistance1,lastDistance2;
	lastDistance1 = lastDistance2 = 0;
	//FIXME: calc distances on spline build
	idVec3 temp;
	int count = target.numSegments();
	//for(int i = 2; i < count - 1; i++) {
	int i;
	for( i = 1; i < count; i++) {
		temp = *target.getSegmentPoint(i-1);
		temp -= *target.getSegmentPoint(i);
		tempDistance += temp.Length();
		if (i & 1) {
			lastDistance1 = tempDistance;
		} else {
			lastDistance2 = tempDistance;
		}
		if (tempDistance >= targetDistance) {
			break;
		}
	}

	if (i >= count - 1) {
		interpolatedPos = *target.getSegmentPoint(i-1);
	} else {
#if 0
		double timeHi = target.getSegmentTime(i + 1);
		double timeLo = target.getSegmentTime(i - 1);
		double percent = (timeHi - t) / (timeHi - timeLo); 
		idVec3 v1 = *target.getSegmentPoint(i - 1);
		idVec3 v2 = *target.getSegmentPoint(i + 1);
		v2 *= (1.0f - percent);
		v1 *= percent;
		v2 += v1;
		interpolatedPos = v2;
#else
		if (lastDistance1 > lastDistance2) {
			double d = lastDistance2;
			lastDistance2 = lastDistance1;
			lastDistance1 = d;
		}

		idVec3 v1 = *target.getSegmentPoint(i - 1);
		idVec3 v2 = *target.getSegmentPoint(i);
		double percent = (lastDistance2 - targetDistance) / (lastDistance2 - lastDistance1); 
		v2 *= (1.0f - percent);
		v1 *= percent;
		v2 += v1;
		interpolatedPos = v2;
#endif
	}
	return &interpolatedPos;

}

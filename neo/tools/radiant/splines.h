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

#ifndef __SPLINES_H__
#define __SPLINES_H__

extern void glBox(idVec4 &color, idVec3 &point, float size);
extern void glLabeledPoint(idVec4 &color, idVec3 &point, float size, const char *label);


class idPointListInterface {
public:
						idPointListInterface() { selectedPoints.Clear(); };
						~idPointListInterface() {};
	
	virtual int			numPoints() { return 0; }
	virtual void		addPoint( const float x, const float y, const float z ) {}
	virtual void		addPoint( const idVec3 &v ) {}
	virtual void		removePoint( int index ) {}
	virtual idVec3 *	getPoint( int index ) { return NULL; }
	
	int					numSelectedPoints() { return selectedPoints.Num(); }
	idVec3 *			getSelectedPoint( int index );
	int					selectPointByRay( const idVec3 &origin, const idVec3 &direction, bool single );
	int					isPointSelected( int index );
	int					selectPoint( int index, bool single );
	void				selectAll();
	void				deselectAll();
	virtual void		updateSelection( const idVec3 &move );
	void				drawSelection();

protected:
	idList<int>			selectedPoints;
};


class idSplineList {
	friend class		idCamera;

public:

						idSplineList() { clear(); }
						idSplineList( const char *p ) { clear(); name = p; }
						~idSplineList() { clear(); }

	void				clearControl();
	void				clearSpline();
	void				parse( idParser *src );
	void				write( idFile *f, const char *name );

	void				clear();
	void				initPosition( long startTime, long totalTime );
	const idVec3 *		getPosition( long time );

	void				draw( bool editMode );
	void				addToRenderer();

	void				setSelectedPoint( idVec3 *p );
	idVec3 *			getSelectedPoint() { return selected; }

	void				addPoint( const idVec3 &v ) { controlPoints.Append(new idVec3(v) ); dirty = true; }
	void				addPoint( float x, float y, float z ) { controlPoints.Append(new idVec3(x, y, z)); dirty = true; }

	void				updateSelection(const idVec3 &move);
	void				startEdit() { editMode = true; }
	void				stopEdit() { editMode = false; }
	void				buildSpline();
	void				setGranularity( float f ) { granularity = f; }
	float				getGranularity() { return granularity; }

	int					numPoints() { return controlPoints.Num(); }
	idVec3 *			getPoint(int index) { assert(index >= 0 && index < controlPoints.Num()); return controlPoints[index]; }
	idVec3 *			getSegmentPoint(int index) { assert(index >= 0 && index < splinePoints.Num()); return splinePoints[index]; }
	void				setSegmentTime(int index, int time) { assert(index >= 0 && index < splinePoints.Num()); splineTime[index] = time; }
	int					getSegmentTime(int index) { assert(index >= 0 && index < splinePoints.Num()); return splineTime[index]; }
	void				addSegmentTime(int index, int time) { assert(index >= 0 && index < splinePoints.Num()); splineTime[index] += time; }
	float				totalDistance();

	int					getActiveSegment() { return activeSegment; }
	void				setActiveSegment( int i ) { /* assert(i >= 0 && (splinePoints.Num() > 0 && i < splinePoints.Num())); */ activeSegment = i; }
	int					numSegments() { return splinePoints.Num(); }

	void				setColors(idVec4 &path, idVec4 &segment, idVec4 &control, idVec4 &active);

	const char *		getName() { return name.c_str(); }
	void				setName( const char *p ) { name = p; }

	bool				validTime();
	void				setTime( long t ) { time = t; }
	void				setBaseTime( long t ) { baseTime = t; }

protected:
	idStr				name;
	float				calcSpline(int step, float tension);
	idList<idVec3*>		controlPoints;
	idList<idVec3*>		splinePoints;
	idList<double>		splineTime;
	idVec3 *			selected;
	idVec4				pathColor, segmentColor, controlColor, activeColor;
	float				granularity;
	bool				editMode;
	bool				dirty;
	int					activeSegment;
	long				baseTime;
	long				time;
};

// time in milliseconds 
// velocity where 1.0 equal rough walking speed
struct idVelocity {
						idVelocity( long start, long duration, float s ) { startTime = start; time = duration; speed = s; }
	long				startTime;
	long				time;
	float				speed;
};

// can either be a look at or origin position for a camera
class idCameraPosition : public idPointListInterface {
public:
	
						idCameraPosition() { time = 0; name = "position"; }
						idCameraPosition( const char *p ) { name = p; }
						idCameraPosition( long t ) { time = t; }
	virtual				~idCameraPosition() { clear(); }

	// this can be done with RTTI syntax but i like the derived classes setting a type
	// makes serialization a bit easier to see
	//
	enum				positionType {
							FIXED = 0x00,
							INTERPOLATED,
							SPLINE,
							POSITION_COUNT
						};

	virtual void		clearVelocities();
	virtual void		clear() { editMode = false; time = 5000; clearVelocities(); }
	virtual void		start( long t ) { startTime = t; }
	long				getTime() { return time; }
	virtual void		setTime(long t) { time = t; }
	float				getVelocity( long t );
	float				getBaseVelocity() { return baseVelocity; }
	void				addVelocity( long start, long duration, float speed ) { velocities.Append(new idVelocity(start, duration, speed)); }
	virtual const idVec3 *getPosition( long t ) { return NULL; }
	virtual void		draw( bool editMode ) {};
	virtual void		parse( idParser *src ) {};
	virtual void		write( idFile *f, const char *name);
	virtual bool		parseToken( const idStr &key, idParser *src );
	const char *		getName() { return name.c_str(); }
	void				setName( const char *p ) { name = p; }
	virtual void		startEdit() { editMode = true; }
	virtual void		stopEdit() { editMode = false; }
	virtual void		draw() {};
	const char *		typeStr() { return positionStr[static_cast<int>(type)]; }
	void				calcVelocity( float distance ) { float secs = (float)time / 1000; baseVelocity = distance / secs; }

protected:
	static const char *	positionStr[POSITION_COUNT];
	long				startTime;
	long				time;
	positionType		type;
	idStr				name;
	bool				editMode;
	idList<idVelocity*> velocities;
	float				baseVelocity;
};

class idFixedPosition : public idCameraPosition {
public:

						idFixedPosition() : idCameraPosition() { init(); }
						idFixedPosition(idVec3 p) : idCameraPosition() { init(); pos = p; }
						~idFixedPosition() { }

	void				init() { pos.Zero(); type = idCameraPosition::FIXED; }

	virtual void		addPoint( const idVec3 &v ) { pos = v; }
	virtual void		addPoint( const float x, const float y, const float z ) { pos.Set(x, y, z); }
	virtual const idVec3 *getPosition( long t ) { return &pos; }
	void				parse( idParser *src );
	void				write( idFile *f, const char *name );
	virtual int			numPoints() { return 1; }
	virtual idVec3 *	getPoint( int index ) { assert( index == 0 ); return &pos; }
	virtual void		draw( bool editMode ) { glLabeledPoint(colorBlue, pos, (editMode) ? 5 : 3, "Fixed point"); }

protected:
	idVec3				pos;
};

class idInterpolatedPosition : public idCameraPosition {
public:
						idInterpolatedPosition() : idCameraPosition() { init(); }
						idInterpolatedPosition( idVec3 start, idVec3 end, long time ) : idCameraPosition(time) { init(); startPos = start; endPos = end; }
						~idInterpolatedPosition() { }

	void				init() { type = idCameraPosition::INTERPOLATED; first = true; startPos.Zero(); endPos.Zero(); }

	virtual const idVec3 *getPosition(long t);
	void				parse( idParser *src );
	void				write( idFile *f, const char *name );
	virtual int			numPoints() { return 2; }
	virtual idVec3 *	getPoint( int index );
	virtual void		addPoint( const float x, const float y, const float z );
	virtual void		addPoint( const idVec3 &v );
	virtual void		draw( bool editMode );
	virtual void		start( long t );

protected:
	bool				first;
	idVec3				startPos;
	idVec3				endPos;
	long				lastTime;
	float				distSoFar;
};

class idSplinePosition : public idCameraPosition {
public:

						idSplinePosition() : idCameraPosition() { init(); }
						idSplinePosition( long time ) : idCameraPosition( time ) { init(); }
						~idSplinePosition() { }

	void				init() { type = idCameraPosition::SPLINE; }
	virtual void		start( long t );
	virtual const idVec3 *getPosition( long t );
	void				addControlPoint( idVec3 &v ) { target.addPoint(v); }
	void				parse( idParser *src );
	void				write( idFile *f, const char *name );
	virtual int			numPoints() { return target.numPoints(); }
	virtual idVec3 *	getPoint( int index ) { return target.getPoint(index); }
	virtual void		addPoint( const idVec3 &v ) { target.addPoint( v ); }
	virtual void		draw( bool editMode ) { target.draw( editMode ); }
	virtual void		updateSelection( const idVec3 &move ) { idCameraPosition::updateSelection(move); target.buildSpline(); }

protected:
	idSplineList		target;
	long				lastTime;
	float				distSoFar;
};

class idCameraFOV {
public:
						idCameraFOV() { time = 0; fov = 90; }
						idCameraFOV( int v ) { time = 0; fov = v; }
						idCameraFOV( int s, int e, long t ) { startFOV = s; endFOV = e; time = t; }
						~idCameraFOV() { }

	void				SetFOV( float f ) { fov = f; }
	float				GetFOV( long t );
	void				start( long t ) { startTime = t; }
	void				reset( float startfov, float endfov, int start, int len );
	void				parse( idParser *src );
	void				write( idFile *f, const char *name );

protected:
	float				fov;
	float				startFOV;
	float				endFOV;
	int					startTime;
	int					time;
	int					length;
};

class idCameraEvent {
public:
	enum				eventType {
							EVENT_NA = 0x00,
							EVENT_WAIT,
							EVENT_TARGETWAIT,
							EVENT_SPEED,
							EVENT_TARGET,
							EVENT_SNAPTARGET,
							EVENT_FOV,
							EVENT_CMD,
							EVENT_TRIGGER,
							EVENT_STOP,
							EVENT_CAMERA,
							EVENT_FADEOUT,
							EVENT_FADEIN,
							EVENT_FEATHER,
							EVENT_COUNT
						};

						idCameraEvent() { paramStr = ""; type = EVENT_NA; time = 0; }
						idCameraEvent( eventType t, const char *param, long n ) { type = t; paramStr = param; time = n; }
						~idCameraEvent() { }

	eventType			getType() { return type; }
	const char *		typeStr() { return eventStr[static_cast<int>(type)]; }
	const char *		getParam() { return paramStr.c_str(); }
	long				getTime() { return time; }
	void				setTime(long n) { time = n; }
	void				parse( idParser *src );
	void				write( idFile *f, const char *name );
	void				setTriggered( bool b ) { triggered = b; }
	bool				getTriggered() { return triggered; }

	static const char *	eventStr[EVENT_COUNT];

protected:
	eventType			type;
	idStr				paramStr;
	long				time;
	bool				triggered;

};

class idCameraDef {
public:
						idCameraDef() { cameraPosition = NULL; clear(); }
						~idCameraDef() { clear(); }

	void				clear();
	idCameraPosition *	startNewCamera(idCameraPosition::positionType type);
	void				addEvent( idCameraEvent::eventType t, const char *param, long time );
	void				addEvent( idCameraEvent *event );
	static int			sortEvents( const void *p1, const void *p2 );
	int					numEvents() { return events.Num(); }
	idCameraEvent *		getEvent(int index) { assert(index >= 0 && index < events.Num()); return events[index]; }
	void				parse( idParser *src );
	bool				load( const char *filename );
	void				save( const char *filename );
	void				buildCamera();

	void				addTarget( const char *name, idCameraPosition::positionType type );

	idCameraPosition *	getActiveTarget();
	idCameraPosition *	getActiveTarget( int index );
	int					numTargets() { return targetPositions.Num(); }
	void				setActiveTargetByName(const char *name);
	void				setActiveTarget( int index );
	void				setRunning( bool b ) { cameraRunning = b; }
	void				setBaseTime( float f ) { baseTime = f; }
	float				getBaseTime() { return baseTime; }
	float				getTotalTime() { return totalTime; }
	void				startCamera( long t );
	void				stopCamera() { cameraRunning = true; }
	void				getActiveSegmentInfo(int segment, idVec3 &origin, idVec3 &direction, float *fv);
	bool				getCameraInfo(long time, idVec3 &origin, idVec3 &direction, float *fv);
	void				draw( bool editMode );
	int					numPoints();
	const idVec3 *		getPoint( int index );
	void				stopEdit();
	void				startEdit( bool camera );
	bool				waitEvent( int index );
	const char *		getName() { return name.c_str(); }
	void				setName( const char *p ) { name = p; }
	idCameraPosition *	getPositionObj();

	static idCameraPosition *newFromType( idCameraPosition::positionType t );

protected:
	idStr				name;
	int					currentCameraPosition;
	idVec3				lastDirection;
	bool				cameraRunning;
	idCameraPosition *	cameraPosition;
	idList<idCameraPosition*> targetPositions;
	idList<idCameraEvent*> events;
	idCameraFOV			fov;
	int					activeTarget;
	float				totalTime;
	float				baseTime;
	long				startTime;

	bool				cameraEdit;
	bool				editMode;
};

extern bool g_splineMode;

extern idCameraDef *g_splineList;

#endif /* !__SPLINES_H__ */

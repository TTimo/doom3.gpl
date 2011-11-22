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
#ifndef DEBUGGERSERVER_H_
#define DEBUGGERSERVER_H_

#ifndef DEBUGGERMESSAGES_H_
#include "DebuggerMessages.h"
#endif

#ifndef DEBUGGERBREAKPOINT_H_
#include "DebuggerBreakpoint.h"
#endif

#ifndef __GAME_LOCAL_H__
#include "../../game/Game.h"
#endif

class idInterpreter;
class idProgram;

class rvDebuggerServer
{
public:

	rvDebuggerServer ( );
	~rvDebuggerServer ( );

	bool		Initialize			( void );
	void		Shutdown			( void );
	
	bool		ProcessMessages		( void );
	
	bool		IsConnected			( void );
	
	void		CheckBreakpoints	( idInterpreter* interpreter, idProgram* program, int instructionPointer );
	
	void		Print				( const char* text );

	void		OSPathToRelativePath( const char *osPath, idStr &qpath );
		
protected:

	// protected member variables
	bool							mConnected;
	netadr_t						mClientAdr;
	idPort							mPort;
	idList<rvDebuggerBreakpoint*>	mBreakpoints;
	CRITICAL_SECTION				mCriticalSection;
	
	HANDLE							mGameThread;
	
	bool							mBreak;
	bool							mBreakNext;
	bool							mBreakStepOver;
	bool							mBreakStepInto;
	int								mBreakStepOverDepth;
	const function_t*				mBreakStepOverFunc1;
	const function_t*				mBreakStepOverFunc2;
	idProgram*						mBreakProgram;
	int								mBreakInstructionPointer;
	idInterpreter*					mBreakInterpreter;
	
	idStr							mLastStatementFile;
	int								mLastStatementLine;	
	
private:

	void		ClearBreakpoints				( void );

	void		Break							( idInterpreter* interpreter, idProgram* program, int instructionPointer );
	void		Resume							( void );

	void		SendMessage						( EDebuggerMessage dbmsg );
	void		SendPacket						( void* data, int datasize );

	// Message handlers
	void		HandleAddBreakpoint				( msg_t* msg );
	void		HandleRemoveBreakpoint			( msg_t* msg );
	void		HandleResume					( msg_t* msg );
	void		HandleInspectVariable			( msg_t* msg );
	void		HandleInspectCallstack			( msg_t* msg );
	void		HandleInspectThreads			( msg_t* msg );
	
	// MSG helper routines
	void		MSG_WriteCallstackFunc			( msg_t* msg, const prstack_t* stack );
};

/*
================
rvDebuggerServer::IsConnected
================
*/
ID_INLINE bool rvDebuggerServer::IsConnected ( void )
{
	return mConnected;
}

/*
================
rvDebuggerServer::SendPacket
================
*/
ID_INLINE void rvDebuggerServer::SendPacket ( void* data, int size )
{
	mPort.SendPacket ( mClientAdr, data, size );
}

#endif // DEBUGGERSERVER_H_

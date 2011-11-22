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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <pwd.h>
#include <pthread.h>

#include "../../idlib/precompiled.h"
#include "posix_public.h"

#if defined(_DEBUG)
// #define ID_VERBOSE_PTHREADS 
#endif

/*
======================================================
locks
======================================================
*/

// we use an extra lock for the local stuff
const int MAX_LOCAL_CRITICAL_SECTIONS = MAX_CRITICAL_SECTIONS + 1;
static pthread_mutex_t global_lock[ MAX_LOCAL_CRITICAL_SECTIONS ];

/*
==================
Sys_EnterCriticalSection
==================
*/
void Sys_EnterCriticalSection( int index ) {
	assert( index >= 0 && index < MAX_LOCAL_CRITICAL_SECTIONS );
#ifdef ID_VERBOSE_PTHREADS	
	if ( pthread_mutex_trylock( &global_lock[index] ) == EBUSY ) {
		Sys_Printf( "busy lock %d in thread '%s'\n", index, Sys_GetThreadName() );
		if ( pthread_mutex_lock( &global_lock[index] ) == EDEADLK ) {
			Sys_Printf( "FATAL: DEADLOCK %d, in thread '%s'\n", index, Sys_GetThreadName() );
		}
	}	
#else
	pthread_mutex_lock( &global_lock[index] );
#endif
}

/*
==================
Sys_LeaveCriticalSection
==================
*/
void Sys_LeaveCriticalSection( int index ) {
	assert( index >= 0 && index < MAX_LOCAL_CRITICAL_SECTIONS );
#ifdef ID_VERBOSE_PTHREADS
	if ( pthread_mutex_unlock( &global_lock[index] ) == EPERM ) {
		Sys_Printf( "FATAL: NOT LOCKED %d, in thread '%s'\n", index, Sys_GetThreadName() );
	}
#else
	pthread_mutex_unlock( &global_lock[index] );
#endif
}

/*
======================================================
wait and trigger events
we use a single lock to manipulate the conditions, MAX_LOCAL_CRITICAL_SECTIONS-1

the semantics match the win32 version. signals raised while no one is waiting stay raised until a wait happens (which then does a simple pass-through)

NOTE: we use the same mutex for all the events. I don't think this would become much of a problem
cond_wait unlocks atomically with setting the wait condition, and locks it back before exiting the function
the potential for time wasting lock waits is very low
======================================================
*/

pthread_cond_t	event_cond[ MAX_TRIGGER_EVENTS ];
bool			signaled[ MAX_TRIGGER_EVENTS ];
bool			waiting[ MAX_TRIGGER_EVENTS ];

/*
==================
Sys_WaitForEvent
==================
*/
void Sys_WaitForEvent( int index ) {
	assert( index >= 0 && index < MAX_TRIGGER_EVENTS );
	Sys_EnterCriticalSection( MAX_LOCAL_CRITICAL_SECTIONS - 1 );
	assert( !waiting[ index ] );	// WaitForEvent from multiple threads? that wouldn't be good
	if ( signaled[ index ] ) {
		// emulate windows behaviour: signal has been raised already. clear and keep going
		signaled[ index ] = false;
	} else {
		waiting[ index ] = true;
		pthread_cond_wait( &event_cond[ index ], &global_lock[ MAX_LOCAL_CRITICAL_SECTIONS - 1 ] );
		waiting[ index ] = false;
	}
	Sys_LeaveCriticalSection( MAX_LOCAL_CRITICAL_SECTIONS - 1 );
}

/*
==================
Sys_TriggerEvent
==================
*/
void Sys_TriggerEvent( int index ) {
	assert( index >= 0 && index < MAX_TRIGGER_EVENTS );
	Sys_EnterCriticalSection( MAX_LOCAL_CRITICAL_SECTIONS - 1 );
	if ( waiting[ index ] ) {		
		pthread_cond_signal( &event_cond[ index ] );
	} else {
		// emulate windows behaviour: if no thread is waiting, leave the signal on so next wait keeps going
		signaled[ index ] = true;
	}
	Sys_LeaveCriticalSection( MAX_LOCAL_CRITICAL_SECTIONS - 1 );
}

/*
======================================================
thread create and destroy
======================================================
*/

// not a hard limit, just what we keep track of for debugging
#define MAX_THREADS 10
xthreadInfo *g_threads[MAX_THREADS];

int g_thread_count = 0;

typedef void *(*pthread_function_t) (void *);

/*
==================
Sys_CreateThread
==================
*/
void Sys_CreateThread( xthread_t function, void *parms, xthreadPriority priority, xthreadInfo& info, const char *name, xthreadInfo **threads, int *thread_count ) {
	Sys_EnterCriticalSection( );		
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	if ( pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE ) != 0 ) {
		common->Error( "ERROR: pthread_attr_setdetachstate %s failed\n", name );
	}
	if ( pthread_create( ( pthread_t* )&info.threadHandle, &attr, ( pthread_function_t )function, parms ) != 0 ) {
		common->Error( "ERROR: pthread_create %s failed\n", name );
	}
	pthread_attr_destroy( &attr );
	info.name = name;
	if ( *thread_count < MAX_THREADS ) {
		threads[ ( *thread_count )++ ] = &info;
	} else {
		common->DPrintf( "WARNING: MAX_THREADS reached\n" );
	}
	Sys_LeaveCriticalSection( );
}

/*
==================
Sys_DestroyThread
==================
*/
void Sys_DestroyThread( xthreadInfo& info ) {
	// the target thread must have a cancelation point, otherwise pthread_cancel is useless
	assert( info.threadHandle );
	if ( pthread_cancel( ( pthread_t )info.threadHandle ) != 0 ) {
		common->Error( "ERROR: pthread_cancel %s failed\n", info.name );
	}
	if ( pthread_join( ( pthread_t )info.threadHandle, NULL ) != 0 ) {
		common->Error( "ERROR: pthread_join %s failed\n", info.name );
	}
	info.threadHandle = 0;
	Sys_EnterCriticalSection( );
	for( int i = 0 ; i < g_thread_count ; i++ ) {
		if ( &info == g_threads[ i ] ) {
			g_threads[ i ] = NULL;
			int j;
			for( j = i+1 ; j < g_thread_count ; j++ ) {
				g_threads[ j-1 ] = g_threads[ j ];
			}
			g_threads[ j-1 ] = NULL;
			g_thread_count--;
			Sys_LeaveCriticalSection( );
			return;
		}
	}
	Sys_LeaveCriticalSection( );
}

/*
==================
Sys_GetThreadName
find the name of the calling thread
==================
*/
const char* Sys_GetThreadName( int *index ) {
	Sys_EnterCriticalSection( );
	pthread_t thread = pthread_self();
	for( int i = 0 ; i < g_thread_count ; i++ ) {
		if ( thread == (pthread_t)g_threads[ i ]->threadHandle ) {
			if ( index ) {
				*index = i;
			}
			Sys_LeaveCriticalSection( );
			return g_threads[ i ]->name;
		}
	}
	if ( index ) {
		*index = -1;
	}
	Sys_LeaveCriticalSection( );
	return "main";
}

/*
=========================================================
Async Thread
=========================================================
*/

xthreadInfo asyncThread;

/*
=================
Posix_StartAsyncThread
=================
*/
void Posix_StartAsyncThread() {
	if ( asyncThread.threadHandle == 0 ) {
		Sys_CreateThread( (xthread_t)Sys_AsyncThread, NULL, THREAD_NORMAL, asyncThread, "Async", g_threads, &g_thread_count );
	} else {
		common->Printf( "Async thread already running\n" );
	}
	common->Printf( "Async thread started\n" );
}

/*
==================
Posix_InitPThreads
==================
*/
void Posix_InitPThreads( ) {
	int i;
	pthread_mutexattr_t attr;

	// init critical sections
	for ( i = 0; i < MAX_LOCAL_CRITICAL_SECTIONS; i++ ) {
		pthread_mutexattr_init( &attr );
		pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
		pthread_mutex_init( &global_lock[i], &attr );
		pthread_mutexattr_destroy( &attr );
	}

	// init event sleep/triggers
	for ( i = 0; i < MAX_TRIGGER_EVENTS; i++ ) {
		pthread_cond_init( &event_cond[ i ], NULL );
		signaled[i] = false;
		waiting[i] = false;
	}

	// init threads table
	for ( i = 0; i < MAX_THREADS; i++ ) {
		g_threads[ i ] = NULL;
	}	
}


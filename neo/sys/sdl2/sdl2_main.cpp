#include "../../idlib/precompiled.h"
#pragma hdrstop

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include "../sys_local.h"
#include "SDL.h"

void Platform_Init();
void Platform_Quit();

#include <stdio.h>
#include <stdarg.h>

int       g_Quit = 0;

static SDL_mutex     *s_CritSect[MAX_CRITICAL_SECTIONS];
static xthreadInfo    s_AsyncThread;

static SDL_mutex *s_EventCS;
static SDL_cond	 *s_event_cond[ MAX_TRIGGER_EVENTS ];
static bool       s_signaled[ MAX_TRIGGER_EVENTS ];
static bool       s_waiting[ MAX_TRIGGER_EVENTS ];

void Sys_DebugPrintf( const char *fmt, ... )
{
	common->DPrintf("TODO: Sys_DebugPrintf\n");
}

void Sys_Printf( const char *fmt, ... )
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

int Sys_DLL_Load( const char *dllName )
{
	return (intptr_t)SDL_LoadObject( dllName );
}

void Sys_DLL_Unload( int dllHandle )
{
	SDL_UnloadObject( (void *)dllHandle );
}

void *Sys_DLL_GetProcAddress( int dllHandle, const char *procName )
{
	return SDL_LoadFunction( (void *)dllHandle, procName );
}

void Sys_Init( void )
{
	Platform_Init();
	// we have 4 critical sections MAX_CRITICAL_SECTIONS
	for( int i = 0; i < MAX_CRITICAL_SECTIONS; ++i )
		s_CritSect[i] = SDL_CreateMutex();
	s_EventCS = SDL_CreateMutex();
	for( int i = 0; i < MAX_TRIGGER_EVENTS; ++i )
	{
		s_event_cond[i] = SDL_CreateCond();
		s_signaled[i] = false;
		s_waiting[i] = false;
	}
}

void Sys_Quit( void )
{
	Platform_Quit();
	for( int i = 0; i < MAX_CRITICAL_SECTIONS; ++i )
		SDL_DestroyMutex(s_CritSect[i]);
	SDL_DestroyMutex( s_EventCS );
	for( int i = 0; i < MAX_TRIGGER_EVENTS; ++i )
		SDL_DestroyCond(s_event_cond[i]);
	exit( EXIT_SUCCESS );
}

void Sys_Shutdown( void )
{
	common->DPrintf("TODO: Sys_Shutdown\n");
}

void Sys_Sleep( int msec )
{
	common->DPrintf("TODO: Sys_Sleep\n");
}

void Sys_Mkdir( const char *path )
{
	mkdir(path, 0777);
}

int Sys_ListFiles( const char *directory, const char *extension, idStrList &list )
{
	struct dirent *d;
	DIR *fdir;
	bool dironly = false;
	char search[MAX_OSPATH];
	struct stat st;
	bool debug;
	
	list.Clear();
	
	debug = cvarSystem->GetCVarBool( "fs_debug" );
	
	if (!extension)
		extension = "";
	
	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == 0) {
		extension = "";
		dironly = true;
	}
	
	// search
	// NOTE: case sensitivity of directory path can screw us up here
	if ((fdir = opendir(directory)) == NULL) {
		if (debug) {
			common->Printf("Sys_ListFiles: opendir %s failed\n", directory);
		}
		return -1;
	}
	
	while ((d = readdir(fdir)) != NULL) {
		idStr::snPrintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if (!dironly) {
			idStr look(search);
			idStr ext;
			look.ExtractFileExtension(ext);
			if (extension[0] != '\0' && ext.Icmp(&extension[1]) != 0) {
				continue;
			}
		}
		if ((dironly && !(st.st_mode & S_IFDIR)) ||
			(!dironly && (st.st_mode & S_IFDIR)))
			continue;
		
		list.Append(d->d_name);
	}
	
	closedir(fdir);
	
	if ( debug ) {
		common->Printf( "Sys_ListFiles: %d entries in %s\n", list.Num(), directory );
	}
	
	return list.Num();
}

void Sys_Error( const char *error, ... )
{
	common->DPrintf("TODO: Sys_Error\n");
}

void Sys_EnterCriticalSection( int index )
{
	SDL_LockMutex(s_CritSect[index]);
}

void Sys_LeaveCriticalSection( int index )
{
	SDL_UnlockMutex(s_CritSect[index]);
}

/*
 ==================
 Sys_WaitForEvent
 ==================
 */
void Sys_WaitForEvent( int index ) {
	assert( index >= 0 && index < MAX_TRIGGER_EVENTS );
	SDL_LockMutex( s_EventCS );
	assert( !s_waiting[ index ] );	// WaitForEvent from multiple threads? that wouldn't be good
	if ( s_signaled[ index ] ) {
		// emulate windows behaviour: signal has been raised already. clear and keep going
		s_signaled[ index ] = false;
	} else {
		s_waiting[ index ] = true;
		SDL_CondWait( s_event_cond[index], s_EventCS );
		s_waiting[ index ] = false;
	}
	SDL_UnlockMutex( s_EventCS );
}

/*
 ==================
 Sys_TriggerEvent
 ==================
 */
void Sys_TriggerEvent( int index ) {
	assert( index >= 0 && index < MAX_TRIGGER_EVENTS );
	SDL_LockMutex( s_EventCS );
	if ( s_waiting[ index ] ) {
		SDL_CondSignal( s_event_cond[index] );
	} else {
		// emulate windows behaviour: if no thread is waiting, leave the signal on so next wait keeps going
		s_signaled[ index ] = true;
	}
	SDL_UnlockMutex( s_EventCS );
}

ID_TIME_T Sys_FileTimeStamp( FILE *fp )
{
	struct stat st;
	fstat(fileno(fp), &st);
	return st.st_mtime;
}

void Sys_DoPreferences( void )
{
	// this is the function that brings up a dialog to choose fullscreen/window etc
	// it pushes these preferences into cvars
	common->DPrintf("__Sys_DoPreferences\n");
	
	cvarSystem->SetCVarBool( "r_fullscreen",  false );
}

/*
================
Sys_GetProcessorId
================
*/
cpuid_t Sys_GetProcessorId( void )
{
	// implemented
	int cpuid = CPUID_GENERIC;
#if defined(__i386__)
	cpuid |= CPUID_INTEL | CPUID_MMX | CPUID_SSE | CPUID_SSE2 | CPUID_SSE3 | CPUID_HTT | CPUID_CMOV | CPUID_FTZ | CPUID_DAZ;
#else
	#error "Unsupported Platform"
#endif
	return static_cast<cpuid_t>(cpuid);
}

/*
================
Sys_GetProcessorString
================
*/
const char *Sys_GetProcessorString( void )
{
#if defined(__i386__)
	return "x86 CPU with MMX/SSE/SSE2/SSE3 extensions";
#else
	#error "Unsupported Platform"
#endif
	return NULL;
}

double Sys_ClockTicksPerSecond(void)
{
	return (double)SDL_GetPerformanceFrequency();
}

double Sys_GetClockTicks( void )
{
	return (double)SDL_GetPerformanceCounter();
}

void Sys_DebugVPrintf( const char *fmt, va_list arg )
{
	common->DPrintf("TODO: Sys_DebugVPrintf\n");
}

/*
 ==================
 Sys_Createthread
 ==================
 */
void Sys_CreateThread(  xthread_t function, void *parms, xthreadPriority priority, xthreadInfo &info, const char *name, xthreadInfo *threads[MAX_THREADS], int *thread_count )
{
	common->DPrintf("Sys_CreateThread %s : note priority etc not implemented\n", name);
	SDL_Thread *thread = SDL_CreateThread( (SDL_ThreadFunction)function, name, parms );
	info.name = name;
}

void Sys_SetFatalError( const char *error )
{
	common->DPrintf("TODO: Sys_SetFatalError\n");
}

bool Sys_AlreadyRunning( void )
{
	// implemented, we just don't care (same as in OSX)
	return false;
}

/*
 ==================
 idSysLocal::OpenURL
 ==================
 */
void idSysLocal::OpenURL( const char *url, bool doexit )
{
	common->DPrintf("TODO: idSysLocal::OpenURL\n");
}

/*
 ==================
 idSysLocal::StartProcess
 ==================
 */
void idSysLocal::StartProcess( const char *exePath, bool doexit )
{
	common->DPrintf("TODO: idSysLocal::StartProcess\n");
}

/*
 ================
 Sys_GetClipboardData
 ================
 */
char *Sys_GetClipboardData( void )
{
	common->DPrintf("TODO: Sys_GetClipboardData\n");
	return "";
}

/*
 ================
 Sys_SetClipboardData
 ================
 */
void Sys_SetClipboardData( const char *string )
{
	common->DPrintf("TODO: Sys_SetClipboardData\n");
}

/*
 =================
 Sys_AsyncThread
 =================
 */
void Sys_AsyncThread( void ) {
	while ( 1 ) {
		SDL_Delay( 16 );
		common->Async();
		Sys_TriggerEvent( TRIGGER_EVENT_ONE );
	}
}

static void Sys_StartAsyncThread()
{
	Sys_CreateThread( (xthread_t)Sys_AsyncThread, NULL, THREAD_NORMAL, s_AsyncThread, "Async", g_threads, &g_thread_count );
}

int main( int argc, const char *argv[] )
{
//	if( !Sys_CheckOS() )
//		return EXIT_FAILURE;

	// Initialize SDL2
	if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_EVENTS) < 0 )
		return EXIT_FAILURE;

	atexit(SDL_Quit);
	
	// FIXME - APPLE specific code
	
	// Finder passes the process serial number as only argument after the program path
	// nuke it if we see it
	if ( argc > 1 && idStr::Cmpn( argv[ 1 ], "-psn", 4 ) ) {
		common->Init( argc-1, &argv[1], NULL );
	} else {
		common->Init( 0, NULL, NULL );
	}
	
	// get the initial time base
	Sys_Milliseconds();

	// start our timer thread
	Sys_StartAsyncThread();
	
	while( !g_Quit )
	{
		common->Frame();
	}
    SDL_Quit();
}
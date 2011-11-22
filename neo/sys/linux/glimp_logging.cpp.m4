#include "idlib/precompiled.h"
#include "renderer/tr_local.h"
#include "sys/linux/local.h"
#include "glimp_local.h"
#pragma hdrstop

#include <unistd.h>
#define ID_LOG_TO_STDOUT 0

dnl =====================================================
dnl utils
dnl =====================================================

define(`forloop', 
	`pushdef(`$1', `$2')_forloop(`$1', `$2', `$3', `$4')popdef(`$1')')
define(`_forloop',
	`$4`'ifelse($1, `$3', ,
	`define(`$1', incr($1))_forloop(`$1', `$2', `$3', `$4')')')
	
dnl =====================================================
dnl the gl wgl glX definitions
dnl =====================================================
include(../gllog/gl_def.m4)

dnl =====================================================
dnl logging functions
dnl =====================================================

typedef struct {
	GLenum	e;
	const char *name;
} glEnumName_t;

#define	DEF(x) { x, #x },

glEnumName_t	glEnumNames[] = {
#include "sys/linux/glimp_glenum.h"
	{ 0, NULL }
};

/*
======================
EnumString
======================
*/
static const char *EnumString( GLenum t )
{
	static char buffer[8][1024];
	static int index = 0;

	for ( glEnumName_t *n = glEnumNames ; n->name ; n++ ) {
		if ( t == n->e ) {
			return n->name;
		}
	}

	int oldIndex = index;
	index = ( index + 1 ) & 7;
	sprintf( buffer[oldIndex], "0x%x", t );

	return buffer[oldIndex];
}

/*
======================
FloatData
======================
*/
static const char *FloatData( const GLfloat *v, int count ) {
	static char buffer[8][1024];
	static int index = 0;
	char *name;

	name = buffer[index&7];
	sprintf( name, "f%i", index );
	index++;

	fprintf( tr.logFile, "static float %s[%i] = {", name, count );
	for( int i = 0 ; i < count ; i++ ) {
		if ( i < count - 1 ) {
			fprintf( tr.logFile, "%f,", v[i] );
		} else {
			fprintf( tr.logFile, "%f};\n", v[i] );
		}
	}

	return name;
}

#include "glimp_logfuncs.cpp"

dnl define(`log_func', `static `$1' APIENTRY log`$2'(`$3') {
dnl }')
dnl forloop(`i', gl_start, gl_end, `log_func(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
dnl ')
dnl forloop(`i', glX_start, glX_end, `log_func(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
dnl ')

/*
======================
GLimp_BindLogging
======================
*/
void GLimp_BindLogging() {
define(`assign_funcptr', `qgl`$1' = log`$1';')
forloop(`i', gl_start, gl_end, `assign_funcptr(indir(`f'i`_name'))
')

define(`assign_funcptr', `qglX`$1' = log`$1';')
forloop(`i', glX_start, glX_end, `assign_funcptr(indir(`f'i`_name'))
')
}

/*
======================
GLimp_EnableLogging
======================
*/
void GLimp_EnableLogging(bool enable) {
	static bool		isEnabled = false;
	static idStr	ospath;
	static int		initialFrames;

	// return if we're already active
	if ( isEnabled && enable ) {
		// decrement log counter and stop if it has reached 0
		r_logFile.SetInteger( r_logFile.GetInteger() - 1 );
		if ( r_logFile.GetInteger() ) {
			return;
		}
#if ID_LOG_TO_STDOUT
		common->Printf( "end stdout GL loggging after %i frames.\n", initialFrames );
#else
		common->Printf( "closing GL logfile '%s' after %i frames.\n", ospath.c_str(), initialFrames );

		fclose( tr.logFile );
#endif
		enable = false;
		tr.logFile = NULL;
	}

	// return if we're already disabled
	if ( !enable && !isEnabled ) {
		return;
	}

	isEnabled = enable;

	if ( enable ) {
		if ( !tr.logFile ) {
			struct tm		*newtime;
			time_t			aclock;
			idStr			qpath;
			int				i;

			initialFrames = r_logFile.GetInteger();

#if ID_LOG_TO_STDOUT
			tr.logFile = fdopen( STDOUT_FILENO, "w" );
#else
			// scan for an unused filename
			for ( i = 0 ; i < 9999 ; i++ ) {
				sprintf( qpath, "renderlog_%i.txt", i ); 
				if ( fileSystem->ReadFile( qpath, NULL, NULL ) == -1 ) {
					break;		// use this name
				}
			}

			ospath = fileSystem->RelativePathToOSPath( qpath );
			tr.logFile = fopen( ospath, "wt" );
#endif

			// write the time out to the top of the file
			time( &aclock );
			newtime = localtime( &aclock );
			fprintf( tr.logFile, "// %s", asctime( newtime ) );
			fprintf( tr.logFile, "// %s\n\n", com_version.GetString() );
		}

		GLimp_BindLogging();
	} else {

		GLimp_BindNative();
	}
}

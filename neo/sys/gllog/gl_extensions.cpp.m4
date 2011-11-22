#include "idlib/precompiled.h"
#pragma hdrstop

dnl =====================================================
dnl utils
dnl =====================================================

define(`forloop', 
	`pushdef(`$1', `$2')_forloop(`$1', `$2', `$3', `$4')popdef(`$1')')
define(`_forloop',
	`$4`'ifelse($1, `$3', ,
	`define(`$1', incr($1))_forloop(`$1', `$2', `$3', `$4')')')
	
dnl =====================================================
dnl GL extensions
dnl =====================================================

typedef struct {
	const char *ext_name;
} glExtName_t;

glExtName_t glExtNames[] = {
	NULL
};

static void StubFunction( void ) { }

GLExtension_t GLimp_ExtensionPointer( const char *name ) {
	if ( strstr( name, "wgl" ) == name ) {
		common->DPrintf( "WARNING: GLimp_ExtensionPointer for '%s'\n", name );
	}
#ifdef ID_DEDICATED
	common->Printf("GLimp_ExtensionPointer %s\n", name);
	return StubFunction;
#else
	#if 0
	glExtName_t *n;
	for ( n = glExtNames ; n->ext_name ; n++ ) {
		if ( !strcmp( name, n->ext_name ) ) {
			common->DPrintf("matched GL extension: %s\n", name );
			break;
		}
	}
	if ( ! n->ext_name ) {
		common->DPrintf("unmatched GL extension name: %s\n", name );
	}
	#endif
	GLExtension_t ret;
	#if defined(__linux__)    
	// for some reason glXGetProcAddressARB doesn't work on RH9?
	ret = qglXGetProcAddressARB((const GLubyte *) name);
	if ( !ret ) {
		common->Printf("glXGetProcAddressARB failed: \"%s\"\n", name);
		return StubFunction;
	}
	#else
    #error Need OS define
	#endif
	return ret;
#endif
}

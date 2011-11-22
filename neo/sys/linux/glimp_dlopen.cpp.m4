#include "idlib/precompiled.h"
#include "renderer/tr_local.h"
#include "sys/linux/local.h"
#include "glimp_local.h"

#include <dlfcn.h>

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
dnl qgl function ptrs
dnl =====================================================

define(`instance_funcptr', ``$1' ( APIENTRY * qgl`$2' )(`$3');')
forloop(`i', gl_start, gl_end, `instance_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')

dnl =====================================================
dnl glX function ptrs
dnl =====================================================

define(`instance_funcptr', ``$1' ( * qglX`$2' )(`$3');')
forloop(`i', glX_start, glX_end, `instance_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')

dnl =====================================================
dnl dll ptrs
dnl those are the actual dlsym'ed pointers
dnl logging configuration redirects qgl / qglX to either log or dll versions
dnl =====================================================

define(`instance_funcptr', ``$1' ( * dll`$2' )(`$3');')
forloop(`i', gl_start, gl_end, `instance_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')
forloop(`i', glX_start, glX_end, `instance_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')

dnl =====================================================
dnl code
dnl =====================================================

/*
======================
GLimp_BindNative
======================
*/
void GLimp_BindNative() {
define(`assign_funcptr', `qgl`$1' = dll`$1';')
forloop(`i', gl_start, gl_end, `assign_funcptr(indir(`f'i`_name'))
')

define(`assign_funcptr', `qglX`$1' = dll`$1';')
forloop(`i', glX_start, glX_end, `assign_funcptr(indir(`f'i`_name'))
')
}

static void *glHandle = NULL;

/*
======================
GLimp_dlsym_failed
======================
*/
void GLimp_dlsym_failed(const char *name) {
	common->DPrintf("dlsym(%s) failed: %s\n", name, dlerror());
}

/*
======================
GLimp_dlopen
======================
*/
bool GLimp_dlopen() {
	const char *driverName = r_glDriver.GetString()[0] ? r_glDriver.GetString() : "libGL.so.1";
	common->Printf("dlopen(%s)\n", driverName);
	if ( !( glHandle = dlopen( driverName, RTLD_NOW | RTLD_GLOBAL ) ) ) {
		common->DPrintf("dlopen(%s) failed: %s\n", driverName, dlerror());
		return false;
	}
	
	// dlsym the symbols
	
define(`dlsym_funcptr', `dll`$2' = ( `$1' ( APIENTRY *)(`$3') ) dlsym( glHandle, "gl`$2'" );')
define(`safe_dlsym_funcptr', `dlsym_funcptr(`$1', `$2', `$3')
if (!dll`$2') { GLimp_dlsym_failed("gl`$2'"); return false; }')
forloop(`i', gl_start, gl_end, `safe_dlsym_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')

define(`dlsym_funcptr', `dll`$2' = ( `$1' ( APIENTRY *)(`$3') ) dlsym( glHandle, "glX`$2'" );')
define(`safe_dlsym_funcptr', `dlsym_funcptr(`$1', `$2', `$3')
if (!dll`$2') { GLimp_dlsym_failed("glX`$2'"); return false; }')
forloop(`i', glX_start, glX_end, `safe_dlsym_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')

	// make the initial binding
	GLimp_BindNative();

	return true;
}

/*
======================
GLimp_dlclose
======================
*/
void GLimp_dlclose() {
	if ( !glHandle ) {
		common->DPrintf("dlclose: GL handle is NULL\n");
	} else {	
		dlclose( glHandle );
		glHandle = NULL;
	}
	
define(`reset_funcptr', `qgl`$1' = NULL;')
forloop(`i', gl_start, gl_end, `reset_funcptr(indir(`f'i`_name'))
')

define(`reset_funcptr', `qglX`$1' = NULL;')
forloop(`i', glX_start, glX_end, `reset_funcptr(indir(`f'i`_name'))
')

}

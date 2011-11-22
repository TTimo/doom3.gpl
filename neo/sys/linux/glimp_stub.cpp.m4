// glimp_stub.cpp.m4
// stub gl/glX APIs

#include "idlib/precompiled.h"
#include "renderer/tr_local.h"
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
dnl the gl wgl glX definitions
dnl =====================================================
include(../gllog/gl_def.m4)

dnl =====================================================
dnl qgl stubs
dnl there is a number of functions for which we have special case code
dnl =====================================================

define(`override_GetError', `')
define(`override_GenLists', `')
define(`override_GetIntegerv', `')
define(`override_GetString', `')

define(`instance_funcptr', ``$1' gl`$2'(`$3'){}')
define(`try_instance_funcptr', `ifdef(`override_'$2, ,`instance_funcptr(`$1', `$2', `$3')')')
forloop(`i', gl_start, gl_end, `try_instance_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')

dnl =====================================================
dnl glX stubs
dnl =====================================================

define(`override_GetProcAddressARB', `')

define(`instance_funcptr', ``$1' glX`$2'(`$3'){}')
define(`try_instance_funcptr', `ifdef(`override_'$2, ,`instance_funcptr(`$1', `$2', `$3')')')
forloop(`i', glX_start, glX_end, `try_instance_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')

GLenum glGetError(void){return 0;}

GLuint glGenLists(GLsizei range){return 0;}

void glGetIntegerv(GLenum pname, GLint *params){
	switch( pname ) {
		case GL_MAX_TEXTURE_SIZE: *params = 1024; break;
		case GL_MAX_TEXTURE_UNITS_ARB: *params = 2; break;
		default: *params = 0; break;
	}
}

const GLubyte * glGetString(GLenum name){
	switch( name ) {
		case GL_EXTENSIONS: return (GLubyte *)"GL_ARB_multitexture GL_ARB_texture_env_combine GL_ARB_texture_cube_map GL_ARB_texture_env_dot3";
	}
	return (const GLubyte *)"";
}

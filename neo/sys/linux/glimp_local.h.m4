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
dnl dll funcs declare
dnl =====================================================

define(`declare_funcptr', `extern `$1' ( * dll`$2' )(`$3');')
forloop(`i', gl_start, gl_end, `declare_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')
forloop(`i', glX_start, glX_end, `declare_funcptr(indir(`f'i`_ret'), indir(`f'i`_name'), indir(`f'i`_params'))
')

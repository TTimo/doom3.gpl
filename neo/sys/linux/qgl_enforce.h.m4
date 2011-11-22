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
dnl issue the defines to lock out gl usage
dnl =====================================================
define(`define_out', `#define gl$1 use_qgl$1')
forloop(`i', gl_start, gl_end, `define_out(indir(`f'i`_name'))
')
forloop(`i', glX_start, glX_end, `define_out(indir(`f'i`_name'))
')


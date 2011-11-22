#!/usr/bin/env python
# process stdin into an M4 macro definition file
# recognize three keyword qgl qwgl qglX
#
# output
# global macros keep track of the three intervals for function types:
# gl_start gl_end wgl_start wgl_end glX_start glX_end
# NOTE: will we need similar thing for extensions?
#
# each function:
# f<id>_ret: return type
# f<id>_name: function name
# f<id>_params: complete params
#
# ex:
# define(`f1_ret', `void')
# define(`f1_name', `Accum')
# define(`f1_params', ``GLenum op, GLfloat value'')
#

import sys, string
from read import read_gl

(gl, wgl, glX) = read_gl(sys.stdin)

sys.stdout.write('define(`gl_start\', `0\')\n')
sys.stdout.write('define(`gl_end\', `%d\')\n' % int(len(gl)-1))
sys.stdout.write('define(`wgl_start\', `%d\')\n' % int(len(gl)))
sys.stdout.write('define(`wgl_end\', `%d\')\n' % int(len(gl)+len(wgl)-1))
sys.stdout.write('define(`glX_start\', `%d\')\n' % int(len(gl)+len(wgl)))
sys.stdout.write('define(`glX_end\', `%d\')\n' % int(len(gl)+len(wgl)+len(glX)-1))

i = 0
for l in (gl, wgl, glX):
	for t in l:
		# process ret type to strip trailing spaces
		t[0] = string.strip(t[0])
		sys.stdout.write('define(`f%d_ret\', `%s\')\n' % (i, t[0]))
		sys.stdout.write('define(`f%d_name\', `%s\')\n' % (i, t[2]))
		sys.stdout.write('define(`f%d_params\', ``%s\'\')\n' % (i, t[3]))
		i += 1

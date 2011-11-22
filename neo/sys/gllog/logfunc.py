#!/usr/bin/env python
# generate logging code
# this requires an analysis of the parameters for verbose and do actual call

import sys, string, re
from read import read_gl

def do_logfunc(f_in, f_out):

	(gl, wgl, glX) = read_gl(f_in)

	for l in (gl, glX):
		for t in l:
			# process ret type to strip trailing spaces
			t[0] = string.strip(t[0])
			f_out.write('static %s APIENTRY log%s(%s) {\n' % ( t[0], t[2], t[3] ))
			# work on parameters
			base_params = string.split(t[3], ',')
			#f_out.write('// %s\n' % repr(base_params))
			# init format string and parameter list
			params = []
			format = t[1][1:] + t[2]
			# a general help list
			types = []
			names = []
			for i in base_params:
				regex = re.compile('([a-zA-Z0-9]*)$')
				name = regex.search(i).group(1)
				type = string.strip(i[0:len(i)-len(name)])				
				# catch type with no name
				if (len(type) == 0):
					type = name
					name = ''
				#f_out.write('// type: "%s" name: "%s"\n' % (type, name))
				types.append(type)
				names.append(name)
				# verbose the types
				if (type == 'GLenum'):
					format += ' %s'
					params.append( 'EnumString(' + name + ')' )
				elif (type == 'GLfloat' or type == 'GLclampf' or type == 'GLdouble'):
					format += ' %g'
					params.append( name )
				elif (type == 'GLint' or type == 'GLuint' or type == 'GLsizei' or type == 'GLbyte' or type == 'GLshort'
					or type == 'GLubyte' or type == 'GLushort'):
					format += ' %d'
					params.append( name )
				elif (type == 'GLboolean'):
					format += ' %s'
					params.append( name + ' ? "Y" : "N"' )
				elif (type == 'void'):
					pass
				else:
					f_out.write('// unknown type: "%s" name: "%s"\n' % (type, name))
					format += ' \'' + type + ' ' + name + '\''
			f_out.write('\tfprintf( tr.logFile, "' + format + '\\n"')
			for par in params:
				f_out.write(', ' + par)
			f_out.write(' );\n')
			if (t[0] != 'void'):
				f_out.write('\treturn dll%s(' % t[2])
			else:
				f_out.write('\tdll%s(' % t[2])
			started = 0
			for i in names:
				if (started):
					f_out.write(', ')
				else:
					started = 1
				f_out.write(i)
			f_out.write(');\n')
			f_out.write('}\n\n')
			
if __name__ == '__main__':
	do_logfunc(sys.stdin, sys.stdout)

# utility module to process incoming GL description

import sys, string

def read_gl(f_in):
	buffer = f_in.read()
	lines = string.split(buffer, '\n')

	gl = []
	wgl = []
	glX = []

	for line in lines:
		if ( len(line) ): # drop empty lines
			tokens = string.split(line, ';')
			if ( tokens[1] == 'qgl' ):
				gl.append(tokens)
			elif ( tokens[1] == 'qwgl' ):
				wgl.append(tokens)
			elif ( tokens[1] == 'qglX' ):
				glX.append(tokens)
			else:
				sys.stderr.write('ERROR: unknown type %s\n' % tokens[1])
				raise "abort"
	
	return (gl, wgl, glX)

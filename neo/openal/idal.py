#!/usr/bin/env python
# create win32 source for OpenAL on-demand loading from AL API definitions
# a set of defines al* -> idal*
# typedefs and code pointer functions definition

# 1: get linking with no OpenAL DLL link in anymore
# i.e. have the defines and the code pointer working
# 2: do the load code

import time

funcs = [
    [ 'ALenum', 'alGetError', 'ALvoid' ],
    [ 'ALvoid', 'alGenBuffers', 'ALsizei', 'ALuint *' ],
    [ 'ALboolean', 'alIsSource', 'ALuint' ],
    [ 'ALvoid', 'alSourceStop', 'ALuint' ],
    [ 'ALvoid', 'alGetSourcei', 'ALuint', 'ALenum', 'ALint *' ],
    [ 'ALint', 'alGetInteger', 'ALenum' ],
    [ 'ALCvoid', 'alcSuspendContext', 'ALCcontext *' ],
    [ 'ALCboolean', 'alcMakeContextCurrent', 'ALCcontext *' ],
    [ 'ALCvoid', 'alcProcessContext', 'ALCcontext *' ],
    [ 'ALCvoid', 'alcDestroyContext', 'ALCcontext *' ],
    [ 'ALCubyte *', 'alcGetString', 'ALCdevice *', 'ALCenum' ],
    [ 'ALvoid', 'alBufferData', 'ALuint', 'ALenum', 'ALvoid *', 'ALsizei', 'ALsizei' ],
    [ 'ALvoid', 'alDeleteBuffers', 'ALsizei', 'ALuint *' ],
    [ 'ALboolean', 'alIsExtensionPresent', 'ALubyte *' ],
    [ 'ALvoid', 'alDeleteSources', 'ALsizei', 'ALuint *' ],
    [ 'ALenum', 'alGetEnumValue', 'ALubyte *' ],
    [ 'ALvoid *', 'alGetProcAddress', 'ALubyte *' ],
    [ 'ALCcontext *', 'alcCreateContext', 'ALCdevice *', 'ALCint *' ],
    [ 'ALCdevice *', 'alcOpenDevice', 'ALubyte *' ],
    [ 'ALvoid', 'alListenerfv', 'ALenum', 'ALfloat*' ],
    [ 'ALvoid', 'alSourceQueueBuffers', 'ALuint', 'ALsizei', 'ALuint *' ],
    [ 'ALvoid', 'alSourcei', 'ALuint', 'ALenum', 'ALint' ],
    [ 'ALvoid', 'alListenerf', 'ALenum', 'ALfloat' ],
    [ 'ALCvoid', 'alcCloseDevice', 'ALCdevice *' ],
    [ 'ALboolean', 'alIsBuffer', 'ALuint' ],
    [ 'ALvoid', 'alSource3f', 'ALuint', 'ALenum', 'ALfloat', 'ALfloat', 'ALfloat' ],
    [ 'ALvoid', 'alGenSources', 'ALsizei', 'ALuint *' ],
    [ 'ALvoid', 'alSourcef', 'ALuint', 'ALenum', 'ALfloat' ],
    [ 'ALvoid', 'alSourceUnqueueBuffers', 'ALuint', 'ALsizei', 'ALuint *' ],
    [ 'ALvoid', 'alSourcePlay', 'ALuint' ],
    ]

def warningHeader( f ):
    f.write( '// generated header. do not edit\n' )
    f.write( '// ' + __file__ + '\n' )
    f.write( '// ' + time.asctime() + '\n\n' )

def genIDALFunc( f, declare ):
    if ( declare ):
        extern = 'extern '
    else:
        extern = ''
    for func in funcs:
        f.write( extern + func[0] + ' ( ALAPIENTRY * id' + func[1] + ' )( ' )
        i = 2
        while ( i < len( func ) ):
            if ( i != 2 ):
                f.write( ', ' )
            f.write( func[i] )
            i += 1
        if ( declare ):
            f.write( ' );\n' )
        else:
            f.write( ' ) = NULL;\n' )

def genDefineMapping( f ):
    for func in funcs:
        fname = func[1]
        f.write( '#define %s id%s\n' % ( fname, fname ) )

def genIDALInit( f ):
    for func in funcs:
        # annoying casting
        cast = func[0] + ' ( ALAPIENTRY * ) ( '
        i = 2
        while ( i < len( func ) ):
            if ( i != 2 ):
                cast += ', '
            cast += func[i]
            i += 1
        cast += ' )'
        # function
        f.write( 'id' + func[1] + ' = ( ' + cast + ' )GetProcAddress( h, "' + func[1] + '" );\n' )
        f.write( 'if ( !id' + func[1] + ') {\n  return "' + func[1] + '";\n}\n' )

if __name__ == '__main__':
    f = open( 'idal.h', 'w' )
    warningHeader( f )
    genIDALFunc( f, True )
    f.write( '\n' )
    genDefineMapping( f )
    f.close()
    f = open( 'idal.cpp', 'w' )
    warningHeader( f )
    genIDALFunc( f, False )
    f.write( '\n' );
    f.write( 'const char* InitializeIDAL( HMODULE h ) {\n' )
    genIDALInit( f )
    f.write( 'return NULL;\n' );
    f.write( '};\n' )
    f.close()

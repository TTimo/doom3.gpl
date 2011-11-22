#!/usr/bin/env python
# prepare content for SDK

import shutil, os, stat

media = '../../../../../media-sdk'
media = os.path.abspath( media )

try:
    shutil.rmtree( 'Doom3_SDK' )
except:
    print 'Could not remove Doom3_SDK'
    pass

# copy source from list
f = open( 'source.list' )
l = [ s[:-1] for s in f.readlines() ]
f.close()
for p in l:
    sp = os.path.join( '../../..', p )
    dp = os.path.join( 'Doom3_SDK/src', p )
    try:
        os.makedirs( os.path.dirname( dp ) )
    except:
        pass
    print 'cp ' + sp + ' -> ' + dp
    shutil.copy( sp, dp )

# copy explicit media content over
for root, dirs, files in os.walk( media ):
    if '.svn' in dirs:
        dirs.remove( '.svn' )
    for f in files:
        sp = os.path.join( root, f )
        dp = os.path.join( 'Doom3_SDK', sp[ len( media ) + 1: ] )
        try:
            os.makedirs( os.path.dirname( dp ) )
        except:
            pass
        print 'cp ' + sp + ' -> ' + dp
        shutil.copy( sp, dp )

def makewritable( path ):
    for root, dirs, files in os.walk( path ):
        for f in files:
            os.chmod( os.path.join( root, f ), stat.S_IWRITE )

# cleanup '.svn'
for root, dirs, files in os.walk( 'Doom3_SDK' ):
    if '.svn' in dirs:
        print 'remove ' + os.path.join( root, '.svn' )
        # SVN sets readonly on some files, which causes rmtree failure on win32
        makewritable( os.path.join( root, '.svn' ) )
        shutil.rmtree( os.path.join( root, '.svn' ) )
        dirs.remove( '.svn' )

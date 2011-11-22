# -*- mode: python -*-
import string

from twisted.application import service, internet

from runner_lib import doomService

maps_file = open( 'maps.list.full' )
multiplayer = 0

# maps_file = open( 'maps.list.mp' )
# multiplayer = 1

# maps_file = open( 'maps.list.game' )
# multiplayer = 0

blank_run = 1

maps = maps_file.readlines()
maps_file.close()
for i in range( 0, len(maps) ):
	maps[i] = string.strip( maps[i], '\n' )

application = service.Application( "doomRunner" )
doomService( '/home/timo/runner/doom.x86', '+set r_fullscreen 0 +set in_nograb 1 +set si_pure 0 +set com_makingBuild 1 +set s_forceWav 1 +set s_maxSoundsPerShader 0 +set s_constantAmplitude 1 +set fs_devpath /home/timo/Id/DoomPure +set fs_basepath /home/timo/Id/DoomBase +set fs_cdpath /home/timo/Id/DoomBase.smbfs +set fs_copyfiles 3', maps, sort = 1, multiplayer = multiplayer, blank_run = blank_run ).setServiceParent( application )

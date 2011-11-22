import os, sys

import scons_utils

class idSDK( scons_utils.idSetupBase ):
	
	def PreBuildSDK( self, build_path ):
		self.build_path = build_path
		print 'PreBuildSDK: ' + repr( build_path )
		for p in build_path:
			self.SimpleCommand( 'rm -rf ' + p )

	def Visit( self, arg, dirname, names ):
		#print 'visit: %s %s' % ( dirname, repr( names ) )
		for i in names:
			if ( i[len(i)-2:] == '.h' or i[len(i)-4:] == '.cpp' ):
				self.file_list.append( os.path.join( dirname, i ) )

	def BuildSDK( self, target = None, source = None, env = None ):
		print 'Building SDK release'
		# extract the file list
		self.file_list = []
		for p in self.build_path:
			os.path.walk( p, self.Visit, None )
		main_version = self.ExtractEngineVersion()
		version = self.ExtractBuildVersion()
		sdk_dirname = 'doom3-linux-%s.%s-sdk' % ( main_version, version )
		sdk_srcdir = os.path.join( sdk_dirname, 'src' )
		if ( os.path.exists( sdk_dirname ) ):
			self.SimpleCommand( 'rm -rf ' + sdk_dirname )
		self.SimpleCommand( 'mkdir -p ' + sdk_srcdir )
		for i in self.file_list:
			# NOTE: same len on all paths game/d3xp. probably breaks for anything else
			short = i[ len( self.build_path[0] ) + 1: ]
			target = os.path.join( sdk_srcdir, short )
			dir = os.path.dirname( target )
			if ( not os.path.exists( dir ) ):
				self.SimpleCommand( 'mkdir -p ' + dir )
			self.SimpleCommand( 'cp ' + i + ' ' + target )
		# remove a bunch of files from hardcoded list
		delete = [ 'framework/Compressor.h', 'framework/Console.h', 'framework/DemoChecksum.h', 'framework/DemoFile.h',
				   'framework/EditField.h', 'framework/EventLoop.h', 'framework/KeyInput.h', 'framework/Session.h',
				   'framework/async/AsyncClient.h', 'framework/async/AsyncNetwork.h', 'framework/async/AsyncServer.h',
				   'framework/async/MsgChannel.h', 'framework/async/ServerScan.h',
				   'mssdk', 'punkbuster', 'sys/osx',
				   'tools/comafx/StdAfx.h', 'tools/compilers/compiler_public.h', 'tools/edit_public.h' ]
		for i in delete:
			target = os.path.join( sdk_srcdir, i )
			self.SimpleCommand( 'rm -rf ' + target )
		# copy files from a hardcoded list
		force_copy = [ 'SConstruct', 'sys/scons/SConscript.game', 'sys/scons/SConscript.idlib', 'sys/scons/scons_utils.py',
					   'game/Game.def', 'd3xp/Game.def',
					   'idlib/geometry/Surface_Polytope.cpp', 'idlib/hashing/CRC8.cpp', 'idlib/math/Complex.cpp',
					   'idlib/math/Simd_3DNow.cpp', 'idlib/math/Simd_AltiVec.cpp', 'idlib/math/Simd_MMX.cpp', 'idlib/math/Simd_SSE.cpp',
					   'idlib/math/Simd_SSE2.cpp', 'idlib/math/Simd_SSE3.cpp',
					   'MayaImport/exporter.h', 'MayaImport/maya_main.cpp', 'MayaImport/maya_main.h',
					   'MayaImport/mayaimport.def', 'MayaImport/Maya4.5/maya.h', 'MayaImport/maya5.0/maya.h',
					   'MayaImport/Maya6.0/maya.h',
					   'd3xp/EndLevel.cpp', 'd3xp/EndLevel.h'
					   ]
		for i in force_copy:
			target = os.path.join( sdk_srcdir, i )
			dir = os.path.dirname( target )
			if ( not os.path.exists( dir ) ):
				self.SimpleCommand( 'mkdir -p ' + dir )
			self.SimpleCommand( 'cp ' + i + ' ' + target )
		# copy sdk media
		if ( not os.path.exists( 'sys/linux/setup/media-sdk' ) ):
			print 'sdk media is missing (sys/linux/setup/media-sdk)'
			sys.exit( 1 )
		self.SimpleCommand( 'cp -R sys/linux/setup/media-sdk/* ' + sdk_dirname )
		# .zip files are auto-expanded by lokisetup, and there's no disable feature
		# zip up the maya toplevel stuff
		self.SimpleCommand( 'cd ' + sdk_dirname + ' && zip MayaSetupStuff.zip MayaImportx86* && rm MayaImportx86*' )
		# put the setup in
		self.SimpleCommand( 'cp -R -f sys/linux/setup/image-base/* ' + sdk_dirname )
		self.SimpleCommand( 'cp -R -f sys/linux/setup/image-sdk/* ' + sdk_dirname )
		# M4
		m4_dict = { 'M4_VERSION' : main_version }
		self.M4Processing( sdk_dirname + '/setup.data/setup.xml.in', m4_dict )
		# create the FreeBSD symlinks
		self.SimpleCommand( 'cd ' + sdk_dirname + '/setup.data/bin ; ln -s Linux FreeBSD' )
		# create amd64 symlinks
		self.SimpleCommand( 'cd ' + sdk_dirname + '/setup.data/bin/Linux ; ln -s x86 amd64' )
		# remove .svn entries
		self.SimpleCommand( 'find ' + sdk_dirname + ' -name \'.svn\' -type d | xargs rm -rf' )
		# put it together
		self.SimpleCommand( 'sys/linux/setup/makeself/makeself.sh ' + sdk_dirname + ' ' + sdk_dirname + '.x86.run \'DOOM III SDK\' ./setup.sh' )
		print 'TODO: do a build check in SDK directory'

import sys, os, string, time, commands, re, pickle, StringIO, popen2, commands, pdb, zipfile, tempfile

import scons_utils

class idSetup( scons_utils.idSetupBase ):

	# do not alter the sources, specially with strip and brandelfing
	def BuildSetup( self, target = None, source = None, env = None ):
		brandelf_path = source[0].abspath
		if ( target[0].path == 'setup-demo' ):
			print 'Building demo setup'
			demo_build = True
			core_path = source[1].abspath
			game_path = source[2].abspath
		else:
			print 'Building setup'
			demo_build = False
			core_path = source[1].abspath
			ded_path = source[2].abspath
			game_path = source[3].abspath
			d3xp_path = source[4].abspath
		# identify dynamic dependencies that we bundle with the binary
		ldd_deps = []
		ldd_output = self.SimpleCommand( 'ldd -r ' + core_path )
		pat = re.compile( '.*lib(stdc\+\+|gcc_s).* => (.*) \(.*\)' )
		for i in string.split( ldd_output, '\n' ):
			if ( pat.match( i ) ):
				ldd_deps.append( pat.split( i )[ 2 ] )
		# prep the binaries and update the paths
		temp_dir = tempfile.mkdtemp( prefix = 'doomsetup' )
		if ( demo_build ):
			self.SimpleCommand( 'cp %s %s/doom.x86' % ( core_path, temp_dir ) )
			core_path = '%s/doom.x86' % temp_dir
			self.SimpleCommand( 'cp %s %s/gamex86.so' % ( game_path, temp_dir ) )
			game_path = '%s/gamex86.so' % temp_dir
			self.SimpleCommand( 'strip ' + core_path )
			self.SimpleCommand( 'strip ' + game_path )
			self.SimpleCommand( brandelf_path + ' -t Linux ' + core_path )
		else:
			self.SimpleCommand( 'cp %s %s/doom.x86' % ( core_path, temp_dir ) )
			core_path = '%s/doom.x86' % temp_dir
			self.SimpleCommand( 'cp %s %s/doomded.x86' % ( ded_path, temp_dir ) )
			ded_path = '%s/doomded.x86' % temp_dir
			self.SimpleCommand( 'cp %s %s/gamex86-base.so' % ( game_path, temp_dir ) )
			game_path = '%s/gamex86-base.so' % temp_dir
			self.SimpleCommand( 'cp %s %s/gamex86-d3xp.so' % ( d3xp_path, temp_dir ) )
			d3xp_path = '%s/gamex86-d3xp.so' % temp_dir
			self.SimpleCommand( 'strip ' + core_path )
			self.SimpleCommand( 'strip ' + ded_path )
			self.SimpleCommand( 'strip ' + game_path )
			self.SimpleCommand( 'strip ' + d3xp_path )
			self.SimpleCommand( brandelf_path + ' -t Linux ' + core_path )
			self.SimpleCommand( brandelf_path + ' -t Linux ' + ded_path )
		# main version tag - ENGINE_VERSION in Licensee.h
		main_version = self.ExtractEngineVersion( )
		# build number
		version = self.ExtractBuildVersion( )
		if ( demo_build ):
			base_dirname = 'doom3-linux-%s.%s-demo' % ( main_version, version )
		else:
			base_dirname = 'doom3-linux-%s.%s' % ( main_version, version )
		if ( os.path.exists( base_dirname ) ):
			self.SimpleCommand( 'rm -rf %s' % base_dirname )
		self.SimpleCommand( 'mkdir %s' % base_dirname )
		self.SimpleCommand( 'cp -R sys/linux/setup/image-base/* ' + base_dirname )
		if ( demo_build ):
			self.SimpleCommand( 'cp -R -f sys/linux/setup/image-demo/* ' + base_dirname )
		else:
			self.SimpleCommand( 'cp -R -f sys/linux/setup/image/* ' + base_dirname )			
		# process M4 stuff
		if ( demo_build ):
			m4_dict = { 'M4_PRODUCT' : 'doom3-demo', 'M4_DESC' : 'DOOM III demo', 'M4_VERSION' : '%s.%s' % ( main_version, version ) }
		else:
			m4_dict = { 'M4_PRODUCT' : 'doom3', 'M4_DESC' : 'DOOM III', 'M4_VERSION' : '%s.%s' % ( main_version, version ) }
		M4_LDD = ''
		for i in ldd_deps:
			if ( len( M4_LDD ) ):
				M4_LDD += '\n'
			M4_LDD += os.path.basename( i )
		m4_dict[ 'M4_LDD' ] = M4_LDD
		self.M4Processing( base_dirname + '/setup.data/setup.xml.in', m4_dict )
		# build the game pak
		if ( demo_build ):
			# the demo doesn't use a game pak
			self.SimpleCommand( 'cp ' + game_path + ' ' + base_dirname )
		else:
			# comment out this part to stick to game paks already provided in the media tree
#			print 'zipping together base game01.pk4'
#			game_zip = zipfile.ZipFile( 'sys/linux/setup/media/base/game01.pk4', 'w', zipfile.ZIP_DEFLATED )
#			game_zip.write( game_path, 'gamex86.so' )
#			game_zip.write( 'sys/linux/setup/binary.conf', 'binary.conf' )
#			game_zip.printdir()
#			game_zip.close()			
#			print 'zipping together d3xp game01.pk4'
#			game_zip = zipfile.ZipFile( 'sys/linux/setup/media/d3xp/game01.pk4', 'w', zipfile.ZIP_DEFLATED )
#			game_zip.write( d3xp_path, 'gamex86.so' )
#			game_zip.write( 'sys/linux/setup/binary.conf', 'binary.conf' )
#			game_zip.printdir()
#			game_zip.close()
			pass
		# copy media
		if ( demo_build ):
			# we use a different repository path for large binary data
			# extract or symlink from media-demo
			if ( not os.path.exists( 'sys/linux/setup/media-demo' ) ):
				print 'demo media is missing (sys/linux/setup/media-demo)'
				sys.exit( 1 )
			# check the md5 of the demo pack to be sure
			md5sum = self.SimpleCommand( 'md5sum sys/linux/setup/media-demo/demo/demo00.pk4' )
			if ( md5sum != '70c2c63ef1190158f1ebd6c255b22d8e  sys/linux/setup/media-demo/demo/demo00.pk4' ):
				print 'demo media has invalid checksum'
				sys.exit( 1 )
			self.SimpleCommand( 'cp -R sys/linux/setup/media-demo/* ' + base_dirname )
		else:
			if ( not os.path.exists( 'sys/linux/setup/media' ) ):
				print 'media is missing (sys/linux/setup/media)'
				sys.exit( 1 )
			# copy the CHANGES file
			self.SimpleCommand( 'cp -v sys/linux/setup/media/CHANGES ' + base_dirname )
			# copy out the pk4 files from the main media tree
			self.SimpleCommand( 'mkdir ' + base_dirname + '/base' )
			self.SimpleCommand( 'mkdir ' + base_dirname + '/d3xp' )
			self.SimpleCommand( 'find sys/linux/setup/media/ -name "*.pk4" | grep -v zpak | cut -b 23- | while read i ; do cp -v sys/linux/setup/media/$i ' + base_dirname + '/$i ; done' )
		# copy
		self.SimpleCommand( 'cp ' + core_path + ' ' + base_dirname + '/bin/Linux/x86' )
		if ( not demo_build ):
			self.SimpleCommand( 'cp ' + ded_path + ' ' + base_dirname + '/bin/Linux/x86' )
		for i in ldd_deps:
			self.SimpleCommand( 'cp ' + i + ' ' + base_dirname + '/' + os.path.basename( i ) )
		# punkbuster
		if ( not demo_build ):
			self.SimpleCommand( 'cp -R punkbuster/setup/pb ' + base_dirname )
			self.SimpleCommand( 'cp -Rf punkbuster/setup/linux/pb ' + base_dirname )
			self.SimpleCommand( 'cp sys/linux/setup/media/PB_EULA.txt ' + base_dirname + '/pb' )
		# put a version tag, xqf request
		f = open( base_dirname + '/version.info', 'w' )
		f.write( main_version + '\n' )
		f.write( self.ExtractProtocolVersion() + '\n' )
		f.close()
		# create the FreeBSD symlinks
		self.SimpleCommand( 'cd ' + base_dirname + '/bin ; ln -s Linux FreeBSD' )
		self.SimpleCommand( 'cd ' + base_dirname + '/setup.data/bin ; ln -s Linux FreeBSD' )
		# create amd64 symlinks
		self.SimpleCommand( 'cd ' + base_dirname + '/bin/Linux ; ln -s x86 amd64' )
		self.SimpleCommand( 'cd ' + base_dirname + '/setup.data/bin/Linux ; ln -s x86 amd64' )
		# remove .svn entries
		self.SimpleCommand( 'find ' + base_dirname + ' -name \'.svn\' -type d | xargs rm -rf' )
		# remove D3XP related stuff until final release
		#self.SimpleCommand( 'rm -rf ' + base_dirname + '/d3xp/*' )
		# package it
		target_setup = base_dirname + '.x86.run'
		if ( demo_build ):
			self.SimpleCommand( 'sys/linux/setup/makeself/makeself.sh ' + base_dirname + ' ' + target_setup + ' \'DOOM III demo\' ./setup.sh' )
		else:
			self.SimpleCommand( 'sys/linux/setup/makeself/makeself.sh ' + base_dirname + ' ' + target_setup + ' \'DOOM III\' ./setup.sh' )
		# take out the temp dir
		self.SimpleCommand( 'rm -rf %s' % temp_dir )
		# success
		return None


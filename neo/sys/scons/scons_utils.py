# -*- mode: python -*-
import sys, os, string, time, commands, re, pickle, StringIO, popen2, commands, pdb, zipfile, tempfile
import SCons

# need an Environment and a matching buffered_spawn API .. encapsulate
class idBuffering:
	silent = False

	def buffered_spawn( self, sh, escape, cmd, args, env ):
		stderr = StringIO.StringIO()
		stdout = StringIO.StringIO()
		command_string = ''
		for i in args:
			if ( len( command_string ) ):
				command_string += ' '
			command_string += i
		try:
			retval = self.env['PSPAWN']( sh, escape, cmd, args, env, stdout, stderr )
		except OSError, x:
			if x.errno != 10:
				raise x
			print 'OSError ignored on command: %s' % command_string
			retval = 0
		print command_string
		if ( retval != 0 or not self.silent ):
			sys.stdout.write( stdout.getvalue() )
			sys.stderr.write( stderr.getvalue() )
		return retval		

class idSetupBase:
	
	def SimpleCommand( self, cmd ):
		print cmd
		ret = commands.getstatusoutput( cmd )
		if ( len( ret[ 1 ] ) ):
			sys.stdout.write( ret[ 1 ] )
			sys.stdout.write( '\n' )
		if ( ret[ 0 ] != 0 ):
			raise 'command failed'
		return ret[ 1 ]

	def TrySimpleCommand( self, cmd ):
		print cmd
		ret = commands.getstatusoutput( cmd )
		sys.stdout.write( ret[ 1 ] )

	def M4Processing( self, file, d ):
		file_out = file[:-3]
		cmd = 'm4 '
		for ( key, val ) in d.items():
			cmd += '--define=%s="%s" ' % ( key, val )
		cmd += '%s > %s' % ( file, file_out )
		self.SimpleCommand( cmd )	

	def ExtractProtocolVersion( self ):
		f = open( 'framework/Licensee.h' )
		l = f.readlines()
		f.close()

		major = 'X'
		p = re.compile( '^#define ASYNC_PROTOCOL_MAJOR\t*(.*)' )
		for i in l:
			if ( p.match( i ) ):
				major = p.match( i ).group(1)
				break

		f = open( 'framework/async/AsyncNetwork.h' )
		l = f.readlines()
		f.close()

		minor = 'X'
		p = re.compile( '^const int ASYNC_PROTOCOL_MINOR\t*= (.*);' )
		for i in l:
			if ( p.match( i ) ):
				minor = p.match( i ).group(1)
				break	
	
		return '%s.%s' % ( major, minor )

	def ExtractEngineVersion( self ):
		f = open( 'framework/Licensee.h' )
		l = f.readlines()
		f.close()

		version = 'X'
		p = re.compile( '^#define.*ENGINE_VERSION\t*"DOOM (.*)"' )
		for i in l:
			if ( p.match( i ) ):
				version = p.match( i ).group(1)
				break
	
		return version

	def ExtractBuildVersion( self ):
		f = open( 'framework/BuildVersion.h' )
		l = f.readlines()[ 4 ]
		f.close()
		pat = re.compile( '.* = (.*);\n' )
		return pat.split( l )[ 1 ]

def checkLDD( target, source, env ):
	file = target[0]
	if (not os.path.isfile(file.abspath)):
		print('ERROR: CheckLDD: target %s not found\n' % target[0])
		Exit(1)
	( status, output ) = commands.getstatusoutput( 'ldd -r %s' % file )
	if ( status != 0 ):
		print 'ERROR: ldd command returned with exit code %d' % ldd_ret
		os.system( 'rm %s' % target[ 0 ] )
		sys.exit(1)
	lines = string.split( output, '\n' )
	have_undef = 0
	for i_line in lines:
		#print repr(i_line)
		regex = re.compile('undefined symbol: (.*)\t\\((.*)\\)')
		if ( regex.match(i_line) ):
			symbol = regex.sub('\\1', i_line)
			try:
				env['ALLOWED_SYMBOLS'].index(symbol)
			except:
				have_undef = 1
	if ( have_undef ):
		print output
		print "ERROR: undefined symbols"
		os.system('rm %s' % target[0])
		sys.exit(1)

def SharedLibrarySafe( env, target, source ):
	ret = env.SharedLibrary( target, source )
	env.AddPostAction( ret, checkLDD )
	return ret

def NotImplementedStub( *whatever ):
	print 'Not Implemented'
	sys.exit( 1 )

# --------------------------------------------------------------------

class idGamePaks( idSetupBase ):

	def BuildGamePak( self, target = None, source = None, env = None ):
		# NOTE: ew should have done with zipfile module
		temp_dir = tempfile.mkdtemp( prefix = 'gamepak' )
		self.SimpleCommand( 'cp %s %s' % ( source[0].abspath, os.path.join( temp_dir, 'gamex86.so' ) ) )
		self.SimpleCommand( 'strip %s' % os.path.join( temp_dir, 'gamex86.so' ) )
		self.SimpleCommand( 'echo 2 > %s' % ( os.path.join( temp_dir, 'binary.conf' ) ) )
		self.SimpleCommand( 'cd %s ; zip %s gamex86.so binary.conf' % ( temp_dir, os.path.join( temp_dir, target[0].abspath ) ) )
		self.SimpleCommand( 'rm -r %s' % temp_dir )
		return None

# --------------------------------------------------------------------

# get a clean error output when running multiple jobs
def SetupBufferedOutput( env, silent ):
	buf = idBuffering()
	buf.silent = silent
	buf.env = env
	env['SPAWN'] = buf.buffered_spawn

# setup utilities on an environement
def SetupUtils( env ):
	gamepaks = idGamePaks()
	env.BuildGamePak = gamepaks.BuildGamePak
	env.SharedLibrarySafe = SharedLibrarySafe
	try:
		import SDK
		sdk = SDK.idSDK()
		env.PreBuildSDK = sdk.PreBuildSDK
		env.BuildSDK = sdk.BuildSDK
	except:
		print 'SDK.py hookup failed'
		env.PreBuildSDK = NotImplementedStub
		env.BuildSDK = NotImplementedStub
	try:
		import Setup
		setup = Setup.idSetup()
		env.BuildSetup = setup.BuildSetup
	except:
		print 'Setup.py hookup failed'
		env.BuildSetup = NotImplementedStub

def BuildList( s_prefix, s_string ):
	s_list = string.split( s_string )
	for i in range( len( s_list ) ):
		s_list[ i ] = s_prefix + '/' + s_list[ i ]
	return s_list

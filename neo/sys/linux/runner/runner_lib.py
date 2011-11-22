# run doom process on a series of maps
# can be used for regression testing, or to fetch media
# keeps a log of each run ( see getLogfile )

# currently uses a basic stdout activity timeout to decide when to move on
# using a periodic check of /proc/<pid>/status SleepAVG
# when the sleep average is reaching 0, issue a 'quit' to stdout

# keeps serialized run status in runner.pickle
# NOTE: can be used to initiate runs on failed maps only for instance etc.

# TODO: use the serialized and not the logs to sort the run order

# TODO: better logging. Use idLogger?

# TODO: configurable event when the process is found interactive
# instead of emitting a quit, perform some warning action?

import sys, os, commands, string, time, traceback, pickle

from twisted.application import internet, service
from twisted.internet import protocol, reactor, utils, defer
from twisted.internet.task import LoopingCall

class doomClientProtocol( protocol.ProcessProtocol ):

	# ProcessProtocol API

	def connectionMade( self ):
		self.logfile.write( 'connectionMade\n' )
		
	def outReceived( self, data ):
		print data
		self.logfile.write( data )

	def errReceived( self, data ):
		print 'stderr: ' + data
		self.logfile.write( 'stderr: ' + data )
		
	def inConnectionLost( self ):
		self.logfile.write( 'inConnectionLost\n' )
		
	def outConnectionLost( self ):
		self.logfile.write( 'outConnectionLost\n' )
		
	def errConnectionLost( self ):
		self.logfile.write( 'errConnectionLost\n' )
		
	def processEnded( self, status_object ):
		self.logfile.write( 'processEnded %s\n' % repr( status_object ) )
		self.logfile.write( time.strftime( '%H:%M:%S', time.localtime( time.time() ) ) + '\n' )
		self.logfile.close()
		self.deferred.callback( None )
		
	# mac management
	def __init__( self, logfilename, deferred ):
		self.logfilename = logfilename
		self.logfile = open( logfilename, 'a' )
		self.logfile.write( time.strftime( '%H:%M:%S', time.localtime( time.time() ) ) + '\n' )
		self.deferred = deferred

class doomService( service.Service ):

	# current monitoring state
	# 0: nothing running
	# 1: we have a process running, we're monitoring it's CPU usage
	# 2: we issued a 'quit' to the process's stdin
	#   either going to get a processEnded, or a timeout
	# 3: we forced a kill because of error, timeout etc.
	state = 0

	# load check period
	check_period = 10

	# pickled status file
	pickle_file = 'runner.pickle'

	# stores status indexed by filename
	# { 'mapname' : ( state, last_update ), .. }
	status = {}

	# start the maps as multiplayer server
	multiplayer = 0

	def __init__( self, bin, cmdline, maps, sort = 0, multiplayer = 0, blank_run = 0 ):
		self.p_transport = None
		self.multiplayer = multiplayer
		self.blank_run = blank_run
		if ( self.multiplayer ):
			print 'Operate in multiplayer mode'
		self.bin = os.path.abspath( bin )
		if ( type( cmdline ) is type( '' ) ):
			self.cmdline = string.split( cmdline, ' ' )
		else:
			self.cmdline = cmdline
		self.maps = maps
		if ( os.path.exists( self.pickle_file ) ):
			print 'Loading pickled status %s' % self.pickle_file
			handle = open( self.pickle_file, 'r' )
			self.status = pickle.load( handle )
			handle.close()
		if ( sort ):
			print 'Sorting maps oldest runs first'
			maps_sorted = [ ]
			for i in self.maps:
				i_log = self.getLogfile( i )
				if ( os.path.exists( i_log ) ):
					maps_sorted.append( ( i, os.path.getmtime( i_log ) ) )
				else:
					maps_sorted.append( ( i, 0 ) )
			maps_sorted.sort( lambda x,y : cmp( x[1], y[1] ) )
			self.maps = [ ]
			if ( blank_run ):
				self.maps.append( 'blankrun' )
			for i in maps_sorted:
				self.maps.append( i[ 0 ] )
			print 'Sorted as: %s\n' % repr( self.maps )

	def getLogfile( self, name ):
		return 'logs/' + string.translate( name, string.maketrans( '/', '-' ) ) + '.log'

	# deferred call when child process dies
	def processEnded( self, val ):
		print 'child has died - state %d' % self.state
		self.status[ self.maps[ self.i_map ] ] = ( self.state, time.time() )
		self.i_map += 1
		if ( self.i_map >= len( self.maps ) ):
			reactor.stop()
		else:
			self.nextMap()

	def processTimeout( self ):
		self.p_transport.signalProcess( "KILL" )

	def sleepAVGReply( self, val ):
		try:
			s = val[10:][:-2]
			print 'sleepAVGReply %s%%' % s
			if ( s == '0' ):
				# need twice in a row
				if ( self.state == 2 ):					
					print 'child process is interactive'
					self.p_transport.write( 'quit\n' )
				else:
					self.state = 2
			else:
				self.state = 1
#			else:
#				reactor.callLater( self.check_period, self.checkCPU )
		except:
			print traceback.format_tb( sys.exc_info()[2] )
			print sys.exc_info()[0]
			print 'exception raised in sleepAVGReply - killing process'
			self.state = 3
			self.p_transport.signalProcess( 'KILL' )

	def sleepAVGTimeout( self ):
		print 'sleepAVGTimeout - killing process'
		self.state = 3
		self.p_transport.signalProcess( 'KILL' )

	# called at regular intervals to monitor the sleep average of the child process
	# when sleep reaches 0, it means the map is loaded and interactive
	def checkCPU( self ):
		if ( self.state == 0 or self.p_transport is None or self.p_transport.pid is None ):
			print 'checkCPU: no child process atm'
			return
		defer = utils.getProcessOutput( '/bin/bash', [ '-c', 'cat /proc/%d/status | grep SleepAVG' % self.p_transport.pid ] )
		defer.addCallback( self.sleepAVGReply )
		defer.setTimeout( 2, self.sleepAVGTimeout )		

	def nextMap( self ):
		self.state = 0
		name = self.maps[ self.i_map ]
		print 'Starting map: ' + name
		logfile = self.getLogfile( name )
		print 'Logging to: ' + logfile
		if ( self.multiplayer ):
			cmdline = [ self.bin ] + self.cmdline + [ '+set', 'si_map', name ]
			if ( name != 'blankrun' ):
				cmdline.append( '+spawnServer' )
		else:
			cmdline = [ self.bin ] + self.cmdline
			if ( name != 'blankrun' ):
				cmdline += [ '+devmap', name ]
		print 'Command line: ' + repr( cmdline )		
		self.deferred = defer.Deferred()
		self.deferred.addCallback( self.processEnded )
		self.p_transport = reactor.spawnProcess( doomClientProtocol( logfile, self.deferred ), self.bin, cmdline , path = os.path.dirname( self.bin ), env = os.environ )
		self.state = 1
#		# setup the CPU usage loop
#		reactor.callLater( self.check_period, self.checkCPU )

	def startService( self ):
		print 'doomService startService'
		loop = LoopingCall( self.checkCPU )
		loop.start( self.check_period )
		self.i_map = 0
		self.nextMap()

	def stopService( self ):
		print 'doomService stopService'
		if ( not self.p_transport.pid is None ):			
			self.p_transport.signalProcess( 'KILL' )
		# serialize
		print 'saving status to %s' % self.pickle_file
		handle = open( self.pickle_file, 'w+' )
		pickle.dump( self.status, handle )
		handle.close()

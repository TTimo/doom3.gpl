# a collection of utility functions to manipulate pak files

import os, zipfile, md5, pdb

# sorts in reverse alphabetical order like doom does for searching
def list_paks( path ):
    files = os.listdir( path )
    for i in files:
        if ( i[-4:] != '.pk4' ):
            files.remove( i )
	files.sort()
	files.reverse()
    return files

def list_files_in_pak( pak ):
    files = []
    zippy = zipfile.ZipFile( pak )
    files += zippy.namelist()
    files.sort()
    return files

# no sorting, blunt list of everything
def list_files_in_paks( path ):
    files = []
    zippies = list_paks( path )
    for fname in zippies:
        print fname
        zippy = zipfile.ZipFile( os.path.join( path, fname ) )
        files += zippy.namelist()
    # sort and remove dupes
    dico = {}
    for f in files:
        dico[ f ] = 1
    files = dico.keys()
    files.sort()
    return files

# build a dictionary of names -> ( pak name, md5 ) from a path of pk4s
def md5_in_paks( path ):
	ret = {}
	zippies = list_paks( path )
	for fname in zippies:
		print fname
		zippy = zipfile.ZipFile( os.path.join( path, fname ) )
		for file in zippy.namelist():
			if ( ret.has_key( file ) ):
				continue
			data = zippy.read( file )
			m = md5.new()
			m.update( data )
			ret[ file ] = ( fname, m.hexdigest() )
	return ret

# find which files need to be updated in a set of paks from an expanded list
# returns ( updated, not_found, {} )
# ignores directories
# by default, no case match is done
# if case match is set, return ( updated, not_found, { zip case -> FS case } )
#   updated will contain the zip case name
def list_updated_files( pak_path, base_path, case_match = False ):
	not_found = []
	updated = []
	case_table = {}
	pak_md5 = md5_in_paks( pak_path )
	for file in pak_md5.keys():
		if ( file[-1] == '/' ):
			continue
		path = os.path.join( base_path, file )
		if ( case_match ):
			ret = ifind( base_path, file )
			if ( not ret[ 0 ] ):
				not_found.append( file )
				continue
			else:
				case_table[ path ] = ret[ 1 ]
				path = os.path.join( base_path, ret[ 1 ] )
		try:
			f = open( path )
			data = f.read()
			f.close()
		except:
			if ( case_match ):
				raise "internal error: ifind success but later read failed"
			not_found.append( file )
		else:
			m = md5.new()
			m.update( data )
			if ( m.hexdigest() != pak_md5[ file ][ 1 ] ):
				print file
				updated.append( file )
	return ( updated, not_found, case_table )

# find which files are missing in the expanded path, and extract the directories
# returns ( files, dirs, missing )
def status_files_for_path( path, infiles ):
    files = []
    dirs = []
    missing = []
    for i in infiles:
        test_path = os.path.join( path, i )
        if ( os.path.isfile( test_path ) ):
            files.append( i )
        elif ( os.path.isdir( test_path ) ):
            dirs.append( i )
        else:
            missing.append( i )
    return ( files, dirs, missing )

# build a pak from a base path and a list of files
def build_pak( pak, path, files ):
    zippy = zipfile.ZipFile( pak, 'w', zipfile.ZIP_DEFLATED )
    for i in files:
        source_path = os.path.join( path, i )
        print source_path
        zippy.write( source_path, i )
    zippy.close()

# process the list of files after a run to update media
# dds/ -> verify all the .dds are present in zip ( case insensitive )
# .wav -> verify that all .wav have a .ogg version in zip ( case insensitive )
# .tga not in dds/ -> try to find a .dds for them
# work from a list of files, and a path to the base pak files
# files: text files with files line by line
# pak_path: the path to the pak files to compare against
# returns: ( [ missing ], [ bad ] )
# bad are files the function didn't know what to do about ( bug )
# missing are lowercased of all the files that where not matched in build
# the dds/ ones are all forced to .dds extension
# missing .wav are returned in the missing list both as .wav and .ogg
# ( that's handy when you need to fetch next )
def check_files_against_build( files, pak_path ):
	pak_list = list_files_in_paks( pak_path )
	# make it lowercase
	tmp = []
	for i in pak_list:
		tmp.append( i.lower() )
	pak_list = tmp
	# read the files and make them lowercase
	f = open( files )
	check_files = f.readlines()
	f.close()
	tmp = []
	for i in check_files:
		s = i.lower()
		s = s.replace( '\n', '' )
		s = s.replace( '\r', '' )
		tmp.append( s )
	check_files = tmp
	# start processing
	bad = []
	missing = []
	for i in check_files:
		if ( i[ :4 ] == 'dds/' ):
			if ( i[ len(i)-4: ] == '.tga' ):
				i = i[ :-4 ] + '.dds'
			elif ( i[ len(i)-4: ] != '.dds' ):
				print 'File not understood: ' + i
				bad.append( i )
				continue
			try:
				pak_list.index( i )
			except:
				print 'Not found: ' + i
				missing.append( i )
		elif ( i[ len(i)-4: ] == '.wav' ):
			i = i[ :-4 ] + '.ogg'
			try:
				pak_list.index( i )
			except:
				print 'Not found: ' + i
				missing.append( i )
				missing.append( i[ :-4 ] + '.wav' )
		elif ( i[ len(i)-4: ] == '.tga' ):
			# tga, not from dds/
			try:
				pak_list.index( i )
			except:
				print 'Not found: ' + i
				missing.append( i )
				i = 'dds/' + i[ :-4 ] + '.dds'
				print 'Add dds  : ' + i
				missing.append( i )
		else:
			try:
				pak_list.index( i )
			except:
				print 'Not found: ' + i
				missing.append( i )
	return ( missing, bad )

# match a path to a file in a case insensitive way
# return ( True/False, 'walked up to' )
def ifind( base, path ):
	refpath = path
	path = os.path.normpath( path )
	path = os.path.normcase( path )
	# early out just in case
	if ( os.path.exists( path ) ):
		return ( True, path )
	head = path
	components = []
	while ( len( head ) ):
		( head, chunk ) = os.path.split( head )
		components.append( chunk )
		#print 'head: %s - components: %s' % ( head, repr( components ) )
	components.reverse()
	level = 0
	for root, dirs, files in os.walk( base, topdown = True ):
		if ( level < len( components ) - 1 ):
			#print 'filter dirs: %s' % repr( dirs )
			dirs_del = []
			for i in dirs:
				if ( not i.lower() == components[ level ].lower() ):
					dirs_del.append( i )
			for i in dirs_del:
				dirs.remove( i )
			level += 1
			# we assume there is never going to be 2 dirs with only case difference
			if ( len( dirs ) != 1 ):
				#print '%s: ifind failed dirs matching at %s - dirs: %s' % ( refpath, root, repr( dirs ) )
				return ( False, root[ len( base ) + 1: ] )
		else:
			# must find the file here
			for i in files:
				if ( i.lower() == components[-1].lower() ):
					return ( True, os.path.join( root, i )[ len( base ) + 1: ] )
			return ( False, root[ len( base ) + 1: ] )

# do case insensitive FS search on files list
# return [ cased files, not found (unmodified ) ]
def ifind_list( base, files ):
	cased = []
	notfound = []
	for i in files:
		ret = ifind( base, i )
		if ( ret[ 0 ] ):
			cased.append( ret[ 1 ] )
		else:
			notfound.append( i )
	return [ cased, notfound ]


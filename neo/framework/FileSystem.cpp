/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Unzip.h"

#ifdef WIN32
	#include <io.h>	// for _read
#else
	#if !__MACH__ && __MWERKS__
		#include <types.h>
		#include <stat.h>
	#else
		#include <sys/types.h>
		#include <sys/stat.h>
	#endif
	#include <unistd.h>
#endif

#if ID_ENABLE_CURL
	#include "../curl/include/curl/curl.h"
#endif

/*
=============================================================================

DOOM FILESYSTEM

All of Doom's data access is through a hierarchical file system, but the contents of 
the file system can be transparently merged from several sources.

A "relativePath" is a reference to game file data, which must include a terminating zero.
"..", "\\", and ":" are explicitly illegal in qpaths to prevent any references
outside the Doom directory system.

The "base path" is the path to the directory holding all the game directories and
usually the executable. It defaults to the current directory, but can be overridden
with "+set fs_basepath c:\doom" on the command line. The base path cannot be modified
at all after startup.

The "save path" is the path to the directory where game files will be saved. It defaults
to the base path, but can be overridden with a "+set fs_savepath c:\doom" on the
command line. Any files that are created during the game (demos, screenshots, etc.) will
be created reletive to the save path.

The "cd path" is the path to an alternate hierarchy that will be searched if a file
is not located in the base path. A user can do a partial install that copies some
data to a base path created on their hard drive and leave the rest on the cd. It defaults
to the current directory, but it can be overridden with "+set fs_cdpath g:\doom" on the
command line.

The "dev path" is the path to an alternate hierarchy where the editors and tools used
during development (Radiant, AF editor, dmap, runAAS) will write files to. It defaults to
the cd path, but can be overridden with a "+set fs_devpath c:\doom" on the command line.

If a user runs the game directly from a CD, the base path would be on the CD. This
should still function correctly, but all file writes will fail (harmlessly).

The "base game" is the directory under the paths where data comes from by default, and
can be either "base" or "demo".

The "current game" may be the same as the base game, or it may be the name of another
directory under the paths that should be searched for files before looking in the base
game. The game directory is set with "+set fs_game myaddon" on the command line. This is
the basis for addons.

No other directories outside of the base game and current game will ever be referenced by
filesystem functions.

To save disk space and speed up file loading, directory trees can be collapsed into zip
files. The files use a ".pk4" extension to prevent users from unzipping them accidentally,
but otherwise they are simply normal zip files. A game directory can have multiple zip
files of the form "pak0.pk4", "pak1.pk4", etc. Zip files are searched in decending order
from the highest number to the lowest, and will always take precedence over the filesystem.
This allows a pk4 distributed as a patch to override all existing data.

Because we will have updated executables freely available online, there is no point to
trying to restrict demo / oem versions of the game with code changes. Demo / oem versions
should be exactly the same executables as release versions, but with different data that
automatically restricts where game media can come from to prevent add-ons from working.

After the paths are initialized, Doom will look for the product.txt file. If not found
and verified, the game will run in restricted mode. In restricted mode, only files
contained in demo/pak0.pk4 will be available for loading, and only if the zip header is
verified to not have been modified. A single exception is made for DoomConfig.cfg. Files
can still be written out in restricted mode, so screenshots and demos are allowed.
Restricted mode can be tested by setting "+set fs_restrict 1" on the command line, even
if there is a valid product.txt under the basepath or cdpath.

If the "fs_copyfiles" cvar is set to 1, then every time a file is sourced from the cd
path, it will be copied over to the save path. This is a development aid to help build
test releases and to copy working sets of files.

If the "fs_copyfiles" cvar is set to 2, any file found in fs_cdpath that is newer than
it's fs_savepath version will be copied to fs_savepath (in addition to the fs_copyfiles 1
behaviour).

If the "fs_copyfiles" cvar is set to 3, files from both basepath and cdpath will be copied
over to the save path. This is useful when copying working sets of files mainly from base
path with an additional cd path (which can be a slower network drive for instance).

If the "fs_copyfiles" cvar is set to 4, files that exist in the cd path but NOT the base path
will be copied to the save path

NOTE: fs_copyfiles and case sensitivity. On fs_caseSensitiveOS 0 filesystems ( win32 ), the
copied files may change casing when copied over.

The relative path "sound/newstuff/test.wav" would be searched for in the following places:

for save path, dev path, base path, cd path:
	for current game, base game:
		search directory
		search zip files

downloaded files, to be written to save path + current game's directory

The filesystem can be safely shutdown and reinitialized with different
basedir / cddir / game combinations, but all other subsystems that rely on it
(sound, video) must also be forced to restart.


"fs_caseSensitiveOS":
This cvar is set on operating systems that use case sensitive filesystems (Linux and OSX)
It is a common situation to have the media reference filenames, whereas the file on disc 
only matches in a case-insensitive way. When "fs_caseSensitiveOS" is set, the filesystem
will always do a case insensitive search.
IMPORTANT: This only applies to files, and not to directories. There is no case-insensitive
matching of directories. All directory names should be lowercase, when "com_developer" is 1,
the filesystem will warn when it catches bad directory situations (regardless of the
"fs_caseSensitiveOS" setting)
When bad casing in directories happen and "fs_caseSensitiveOS" is set, BuildOSPath will
attempt to correct the situation by forcing the path to lowercase. This assumes the media
is stored all lowercase.

"additional mod path search":
fs_game_base can be used to set an additional search path
in search order, fs_game, fs_game_base, BASEGAME
for instance to base a mod of D3 + D3XP assets, fs_game mymod, fs_game_base d3xp

=============================================================================
*/



// define to fix special-cases for GetPackStatus so that files that shipped in 
// the wrong place for Doom 3 don't break pure servers.
#define DOOM3_PURE_SPECIAL_CASES	

typedef bool (*pureExclusionFunc_t)( const struct pureExclusion_s &excl, int l, const idStr &name );

typedef struct pureExclusion_s {
	int					nameLen;
	int					extLen;
	const char *		name;
	const char *		ext;
	pureExclusionFunc_t	func;
} pureExclusion_t;

bool excludeExtension( const pureExclusion_t &excl, int l, const idStr &name ) {
	if ( l > excl.extLen && !idStr::Icmp( name.c_str() + l - excl.extLen, excl.ext ) ) {
		return true;
	}
	return false;
}

bool excludePathPrefixAndExtension( const pureExclusion_t &excl, int l, const idStr &name ) {
	if ( l > excl.nameLen && !idStr::Icmp( name.c_str() + l - excl.extLen, excl.ext ) && !name.IcmpPrefixPath( excl.name ) ) {
		return true;
	}
	return false;
}

bool excludeFullName( const pureExclusion_t &excl, int l, const idStr &name ) {
	if ( l == excl.nameLen && !name.Icmp( excl.name ) ) {
		return true;
	}
	return false;
}

static pureExclusion_t pureExclusions[] = {
	{ 0,	0,	NULL,											"/",		excludeExtension },
	{ 0,	0,	NULL,											"\\",		excludeExtension },
	{ 0,	0,	NULL,											".pda",		excludeExtension },
	{ 0,	0,	NULL,											".gui",		excludeExtension },
	{ 0,	0,	NULL,											".pd",		excludeExtension },
	{ 0,	0,	NULL,											".lang",	excludeExtension },
	{ 0,	0,	"sound/VO",										".ogg",		excludePathPrefixAndExtension },
	{ 0,	0,	"sound/VO",										".wav",		excludePathPrefixAndExtension },
#if	defined DOOM3_PURE_SPECIAL_CASES	
	// add any special-case files or paths for pure servers here
	{ 0,	0,	"sound/ed/marscity/vo_intro_cutscene.ogg",		NULL,		excludeFullName },
	{ 0,	0,	"sound/weapons/soulcube/energize_01.ogg",		NULL,		excludeFullName },
	{ 0,	0,	"sound/xian/creepy/vocal_fx",					".ogg",		excludePathPrefixAndExtension },
	{ 0,	0,	"sound/xian/creepy/vocal_fx",					".wav",		excludePathPrefixAndExtension },
	{ 0,	0,	"sound/feedback",								".ogg",		excludePathPrefixAndExtension },
	{ 0,	0,	"sound/feedback",								".wav",		excludePathPrefixAndExtension },
	{ 0,	0,	"guis/assets/mainmenu/chnote.tga",				NULL,		excludeFullName },
	{ 0,	0,	"sound/levels/alphalabs2/uac_better_place.ogg",	NULL,		excludeFullName },
	{ 0,	0,	"textures/bigchars.tga",						NULL,		excludeFullName },
	{ 0,	0,	"dds/textures/bigchars.dds",					NULL,		excludeFullName },
	{ 0,	0,	"fonts",										".tga",		excludePathPrefixAndExtension },
	{ 0,	0,	"dds/fonts",									".dds",		excludePathPrefixAndExtension },
	{ 0,	0,	"default.cfg",									NULL,		excludeFullName },
	// russian zpak001.pk4
	{ 0,	0,  "fonts",										".dat",		excludePathPrefixAndExtension },
	{ 0,	0,	"guis/temp.guied",								NULL,		excludeFullName },
#endif
	{ 0,	0,	NULL,											NULL,		NULL }
};

// ensures that lengths for pure exclusions are correct
class idInitExclusions {
public:
	idInitExclusions() {
		for ( int i = 0; pureExclusions[i].func != NULL; i++ ) {
			if ( pureExclusions[i].name ) {
				pureExclusions[i].nameLen = idStr::Length( pureExclusions[i].name );
			}
			if ( pureExclusions[i].ext ) {
				pureExclusions[i].extLen = idStr::Length( pureExclusions[i].ext );
			}
		}
	}
};

static idInitExclusions	initExclusions;

#define MAX_ZIPPED_FILE_NAME	2048
#define FILE_HASH_SIZE			1024

typedef struct fileInPack_s {
	idStr				name;						// name of the file
	unsigned long		pos;						// file info position in zip
	struct fileInPack_s * next;						// next file in the hash
} fileInPack_t;

typedef enum {
	BINARY_UNKNOWN = 0,
	BINARY_YES,
	BINARY_NO
} binaryStatus_t;

typedef enum {
	PURE_UNKNOWN = 0,	// need to run the pak through GetPackStatus
	PURE_NEUTRAL,	// neutral regarding pureness. gets in the pure list if referenced
	PURE_ALWAYS,	// always referenced - for pak* named files, unless NEVER
	PURE_NEVER		// VO paks. may be referenced, won't be in the pure lists
} pureStatus_t;

typedef struct {
	idList<int>			depends;
	idList<idDict *>	mapDecls;
} addonInfo_t;

typedef struct {
	idStr				pakFilename;				// c:\doom\base\pak0.pk4
	unzFile				handle;
	int					checksum;
	int					numfiles;
	int					length;
	bool				referenced;
	binaryStatus_t		binary;
	bool				addon;						// this is an addon pack - addon_search tells if it's 'active'
	bool				addon_search;				// is in the search list
	addonInfo_t			*addon_info;
	pureStatus_t		pureStatus;
	bool				isNew;						// for downloaded paks
	fileInPack_t		*hashTable[FILE_HASH_SIZE];
	fileInPack_t		*buildBuffer;
} pack_t;

typedef struct {
	idStr				path;						// c:\doom
	idStr				gamedir;					// base
} directory_t;

typedef struct searchpath_s {
	pack_t *			pack;						// only one of pack / dir will be non NULL
	directory_t *		dir;
	struct searchpath_s *next;
} searchpath_t;

// search flags when opening a file
#define FSFLAG_SEARCH_DIRS		( 1 << 0 )
#define FSFLAG_SEARCH_PAKS		( 1 << 1 )
#define FSFLAG_PURE_NOREF		( 1 << 2 )
#define FSFLAG_BINARY_ONLY		( 1 << 3 )
#define FSFLAG_SEARCH_ADDONS	( 1 << 4 )

// 3 search path (fs_savepath fs_basepath fs_cdpath)
// + .jpg and .tga
#define MAX_CACHED_DIRS 6

// how many OSes to handle game paks for ( we don't have to know them precisely )
#define MAX_GAME_OS	6
#define BINARY_CONFIG "binary.conf"
#define ADDON_CONFIG "addon.conf"

class idDEntry : public idStrList {
public:
						idDEntry() {}
	virtual				~idDEntry() {}

	bool				Matches( const char *directory, const char *extension ) const;
	void				Init( const char *directory, const char *extension, const idStrList &list );
	void				Clear( void );

private:
	idStr				directory;
	idStr				extension;
};

class idFileSystemLocal : public idFileSystem {
public:
							idFileSystemLocal( void );

	virtual void			Init( void );
	virtual void			StartBackgroundDownloadThread( void );
	virtual void			Restart( void );
	virtual void			Shutdown( bool reloading );
	virtual bool			IsInitialized( void ) const;
	virtual bool			PerformingCopyFiles( void ) const;
	virtual idModList *		ListMods( void );
	virtual void			FreeModList( idModList *modList );
	virtual idFileList *	ListFiles( const char *relativePath, const char *extension, bool sort = false, bool fullRelativePath = false, const char* gamedir = NULL );
	virtual idFileList *	ListFilesTree( const char *relativePath, const char *extension, bool sort = false, const char* gamedir = NULL );
	virtual void			FreeFileList( idFileList *fileList );
	virtual const char *	OSPathToRelativePath( const char *OSPath );
	virtual const char *	RelativePathToOSPath( const char *relativePath, const char *basePath );
	virtual const char *	BuildOSPath( const char *base, const char *game, const char *relativePath );
	virtual void			CreateOSPath( const char *OSPath );
	virtual bool			FileIsInPAK( const char *relativePath );
	virtual void			UpdatePureServerChecksums( void );
	virtual bool			UpdateGamePakChecksums( void );
	virtual fsPureReply_t	SetPureServerChecksums( const int pureChecksums[ MAX_PURE_PAKS ], int gamePakChecksum, int missingChecksums[ MAX_PURE_PAKS ], int *missingGamePakChecksum );
	virtual void			GetPureServerChecksums( int checksums[ MAX_PURE_PAKS ], int OS, int *gamePakChecksum );
	virtual void			SetRestartChecksums( const int pureChecksums[ MAX_PURE_PAKS ], int gamePakChecksum );
	virtual	void			ClearPureChecksums( void );
	virtual int				GetOSMask( void );
	virtual int				ReadFile( const char *relativePath, void **buffer, ID_TIME_T *timestamp );
	virtual void			FreeFile( void *buffer );
	virtual int				WriteFile( const char *relativePath, const void *buffer, int size, const char *basePath = "fs_savepath" );
	virtual void			RemoveFile( const char *relativePath );	
	virtual idFile *		OpenFileReadFlags( const char *relativePath, int searchFlags, pack_t **foundInPak = NULL, bool allowCopyFiles = true, const char* gamedir = NULL );
	virtual idFile *		OpenFileRead( const char *relativePath, bool allowCopyFiles = true, const char* gamedir = NULL );
	virtual idFile *		OpenFileWrite( const char *relativePath, const char *basePath = "fs_savepath" );
	virtual idFile *		OpenFileAppend( const char *relativePath, bool sync = false, const char *basePath = "fs_basepath"   );
	virtual idFile *		OpenFileByMode( const char *relativePath, fsMode_t mode );
	virtual idFile *		OpenExplicitFileRead( const char *OSPath );
	virtual idFile *		OpenExplicitFileWrite( const char *OSPath );
	virtual void			CloseFile( idFile *f );
	virtual void			BackgroundDownload( backgroundDownload_t *bgl );
	virtual void			ResetReadCount( void ) { readCount = 0; }
	virtual void			AddToReadCount( int c ) { readCount += c; }
	virtual int				GetReadCount( void ) { return readCount; }
	virtual void			FindDLL( const char *basename, char dllPath[ MAX_OSPATH ], bool updateChecksum );
	virtual void			ClearDirCache( void );
	virtual bool			HasD3XP( void );
	virtual bool			RunningD3XP( void );
	virtual void			CopyFile( const char *fromOSPath, const char *toOSPath );
	virtual int				ValidateDownloadPakForChecksum( int checksum, char path[ MAX_STRING_CHARS ], bool isBinary );
	virtual idFile *		MakeTemporaryFile( void );
	virtual int				AddZipFile( const char *path );
	virtual findFile_t		FindFile( const char *path, bool scheduleAddons );
	virtual int				GetNumMaps();
	virtual const idDict *	GetMapDecl( int i );
	virtual void			FindMapScreenshot( const char *path, char *buf, int len );
	virtual bool			FilenameCompare( const char *s1, const char *s2 ) const;

	static void				Dir_f( const idCmdArgs &args );
	static void				DirTree_f( const idCmdArgs &args );
	static void				Path_f( const idCmdArgs &args );
	static void				TouchFile_f( const idCmdArgs &args );
	static void				TouchFileList_f( const idCmdArgs &args );

private:
	friend dword 			BackgroundDownloadThread( void *parms );

	searchpath_t *			searchPaths;
	int						readCount;			// total bytes read
	int						loadCount;			// total files read
	int						loadStack;			// total files in memory
	idStr					gameFolder;			// this will be a single name without separators

	searchpath_t			*addonPaks;			// not loaded up, but we saw them

	idDict					mapDict;			// for GetMapDecl

	static idCVar			fs_debug;
	static idCVar			fs_restrict;
	static idCVar			fs_copyfiles;
	static idCVar			fs_basepath;
	static idCVar			fs_savepath;
	static idCVar			fs_cdpath;
	static idCVar			fs_devpath;
	static idCVar			fs_game;
	static idCVar			fs_game_base;
	static idCVar			fs_caseSensitiveOS;
	static idCVar			fs_searchAddons;

	backgroundDownload_t *	backgroundDownloads;
	backgroundDownload_t	defaultBackgroundDownload;
	xthreadInfo				backgroundThread;

	idList<pack_t *>		serverPaks;
	bool					loadedFileFromDir;		// set to true once a file was loaded from a directory - can't switch to pure anymore
	idList<int>				restartChecksums;		// used during a restart to set things in right order
	idList<int>				addonChecksums;			// list of checksums that should go to the search list directly ( for restarts )
	int						restartGamePakChecksum;
	int						gameDLLChecksum;		// the checksum of the last loaded game DLL
	int						gamePakChecksum;		// the checksum of the pak holding the loaded game DLL

	int						gamePakForOS[ MAX_GAME_OS ];

	idDEntry				dir_cache[ MAX_CACHED_DIRS ]; // fifo
	int						dir_cache_index;
	int						dir_cache_count;

	int						d3xp;	// 0: didn't check, -1: not installed, 1: installed

private:
	void					ReplaceSeparators( idStr &path, char sep = PATHSEPERATOR_CHAR );
	long					HashFileName( const char *fname ) const;
	int						ListOSFiles( const char *directory, const char *extension, idStrList &list );
	FILE *					OpenOSFile( const char *name, const char *mode, idStr *caseSensitiveName = NULL );
	FILE *					OpenOSFileCorrectName( idStr &path, const char *mode );
	int						DirectFileLength( FILE *o );
	void					CopyFile( idFile *src, const char *toOSPath );
	int						AddUnique( const char *name, idStrList &list, idHashIndex &hashIndex ) const;
	void					GetExtensionList( const char *extension, idStrList &extensionList ) const;
	int						GetFileList( const char *relativePath, const idStrList &extensions, idStrList &list, idHashIndex &hashIndex, bool fullRelativePath, const char* gamedir = NULL );

	int						GetFileListTree( const char *relativePath, const idStrList &extensions, idStrList &list, idHashIndex &hashIndex, const char* gamedir = NULL );
	pack_t *				LoadZipFile( const char *zipfile );
	void					AddGameDirectory( const char *path, const char *dir );
	void					SetupGameDirectories( const char *gameName );
	void					Startup( void );
	void					SetRestrictions( void );
							// some files can be obtained from directories without compromising si_pure
	bool					FileAllowedFromDir( const char *path );
							// searches all the paks, no pure check
	pack_t *				GetPackForChecksum( int checksum, bool searchAddons = false );
							// searches all the paks, no pure check
	pack_t *				FindPakForFileChecksum( const char *relativePath, int fileChecksum, bool bReference );
	idFile_InZip *			ReadFileFromZip( pack_t *pak, fileInPack_t *pakFile, const char *relativePath );
	int						GetFileChecksum( idFile *file );
	pureStatus_t			GetPackStatus( pack_t *pak );
	addonInfo_t *			ParseAddonDef( const char *buf, const int len );
	void					FollowAddonDependencies( pack_t *pak );

	static size_t			CurlWriteFunction( void *ptr, size_t size, size_t nmemb, void *stream );
							// curl_progress_callback in curl.h
	static int				CurlProgressFunction( void *clientp, double dltotal, double dlnow, double ultotal, double ulnow );
};

idCVar	idFileSystemLocal::fs_restrict( "fs_restrict", "", CVAR_SYSTEM | CVAR_INIT | CVAR_BOOL, "" );
idCVar	idFileSystemLocal::fs_debug( "fs_debug", "0", CVAR_SYSTEM | CVAR_INTEGER, "", 0, 2, idCmdSystem::ArgCompletion_Integer<0,2> );
idCVar	idFileSystemLocal::fs_copyfiles( "fs_copyfiles", "0", CVAR_SYSTEM | CVAR_INIT | CVAR_INTEGER, "", 0, 4, idCmdSystem::ArgCompletion_Integer<0,3> );
idCVar	idFileSystemLocal::fs_basepath( "fs_basepath", "", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar	idFileSystemLocal::fs_savepath( "fs_savepath", "", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar	idFileSystemLocal::fs_cdpath( "fs_cdpath", "", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar	idFileSystemLocal::fs_devpath( "fs_devpath", "", CVAR_SYSTEM | CVAR_INIT, "" );
idCVar	idFileSystemLocal::fs_game( "fs_game", "", CVAR_SYSTEM | CVAR_INIT | CVAR_SERVERINFO, "mod path" );
idCVar  idFileSystemLocal::fs_game_base( "fs_game_base", "", CVAR_SYSTEM | CVAR_INIT | CVAR_SERVERINFO, "alternate mod path, searched after the main fs_game path, before the basedir" );
#ifdef WIN32
idCVar	idFileSystemLocal::fs_caseSensitiveOS( "fs_caseSensitiveOS", "0", CVAR_SYSTEM | CVAR_BOOL, "" );
#else
idCVar	idFileSystemLocal::fs_caseSensitiveOS( "fs_caseSensitiveOS", "1", CVAR_SYSTEM | CVAR_BOOL, "" );
#endif
idCVar	idFileSystemLocal::fs_searchAddons( "fs_searchAddons", "0", CVAR_SYSTEM | CVAR_BOOL, "search all addon pk4s ( disables addon functionality )" );

idFileSystemLocal	fileSystemLocal;
idFileSystem *		fileSystem = &fileSystemLocal;

/*
================
idFileSystemLocal::idFileSystemLocal
================
*/
idFileSystemLocal::idFileSystemLocal( void ) {
	searchPaths = NULL;
	readCount = 0;
	loadCount = 0;
	loadStack = 0;
	dir_cache_index = 0;
	dir_cache_count = 0;
	d3xp = 0;
	loadedFileFromDir = false;
	restartGamePakChecksum = 0;
	memset( &backgroundThread, 0, sizeof( backgroundThread ) );
	addonPaks = NULL;
}

/*
================
idFileSystemLocal::HashFileName

return a hash value for the filename
================
*/
long idFileSystemLocal::HashFileName( const char *fname ) const {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while( fname[i] != '\0' ) {
		letter = idStr::ToLower( fname[i] );
		if ( letter == '.' ) {
			break;				// don't include extension
		}
		if ( letter == '\\' ) {
			letter = '/';		// damn path names
		}
		hash += (long)(letter) * (i+119);
		i++;
	}
	hash &= (FILE_HASH_SIZE-1);
	return hash;
}

/*
===========
idFileSystemLocal::FilenameCompare

Ignore case and separator char distinctions
===========
*/
bool idFileSystemLocal::FilenameCompare( const char *s1, const char *s2 ) const {
	int		c1, c2;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( c1 >= 'a' && c1 <= 'z' ) {
			c1 -= ('a' - 'A');
		}
		if ( c2 >= 'a' && c2 <= 'z' ) {
			c2 -= ('a' - 'A');
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}
		
		if ( c1 != c2 ) {
			return true;		// strings not equal
		}
	} while( c1 );
	
	return false;		// strings are equal
}

/*
================
idFileSystemLocal::OpenOSFile
optional caseSensitiveName is set to case sensitive file name as found on disc (fs_caseSensitiveOS only)
================
*/
FILE *idFileSystemLocal::OpenOSFile( const char *fileName, const char *mode, idStr *caseSensitiveName ) {
	int i;
	FILE *fp;
	idStr fpath, entry;
	idStrList list;

#ifndef __MWERKS__
#ifndef WIN32 
	// some systems will let you fopen a directory
	struct stat buf;
	if ( stat( fileName, &buf ) != -1 && !S_ISREG(buf.st_mode) ) {
		return NULL;
	}
#endif
#endif
	fp = fopen( fileName, mode );
	if ( !fp && fs_caseSensitiveOS.GetBool() ) {
		fpath = fileName;
		fpath.StripFilename();
		fpath.StripTrailing( PATHSEPERATOR_CHAR );
		if ( ListOSFiles( fpath, NULL, list ) == -1 ) {
			return NULL;
		}
		
		for ( i = 0; i < list.Num(); i++ ) {
			entry = fpath + PATHSEPERATOR_CHAR + list[i];
			if ( !entry.Icmp( fileName ) ) {
				fp = fopen( entry, mode );
				if ( fp ) {
					if ( caseSensitiveName ) {
						*caseSensitiveName = entry;
						caseSensitiveName->StripPath();
					}
					if ( fs_debug.GetInteger() ) {
						common->Printf( "idFileSystemLocal::OpenFileRead: changed %s to %s\n", fileName, entry.c_str() );
					}
					break;
				} else {
					// not supposed to happen if ListOSFiles is doing it's job correctly
					common->Warning( "idFileSystemLocal::OpenFileRead: fs_caseSensitiveOS 1 could not open %s", entry.c_str() );
				}
			}
		}
	} else if ( caseSensitiveName ) {
		*caseSensitiveName = fileName;
		caseSensitiveName->StripPath();
	}
	return fp;
}

/*
================
idFileSystemLocal::OpenOSFileCorrectName
================
*/
FILE *idFileSystemLocal::OpenOSFileCorrectName( idStr &path, const char *mode ) {
	idStr caseName;
	FILE *f = OpenOSFile( path.c_str(), mode, &caseName );
	if ( f ) {
		path.StripFilename();
		path += PATHSEPERATOR_STR;
		path += caseName;
	}
	return f;
}

/*
================
idFileSystemLocal::DirectFileLength
================
*/
int idFileSystemLocal::DirectFileLength( FILE *o ) {
	int		pos;
	int		end;

	pos = ftell( o );
	fseek( o, 0, SEEK_END );
	end = ftell( o );
	fseek( o, pos, SEEK_SET );
	return end;
}

/*
============
idFileSystemLocal::CreateOSPath

Creates any directories needed to store the given filename
============
*/
void idFileSystemLocal::CreateOSPath( const char *OSPath ) {
	char	*ofs;
	
	// make absolutely sure that it can't back up the path
	// FIXME: what about c: ?
	if ( strstr( OSPath, ".." ) || strstr( OSPath, "::" ) ) {
#ifdef _DEBUG		
		common->DPrintf( "refusing to create relative path \"%s\"\n", OSPath );
#endif
		return;
	}

	idStr path( OSPath );
	for( ofs = &path[ 1 ]; *ofs ; ofs++ ) {
		if ( *ofs == PATHSEPERATOR_CHAR ) {	
			// create the directory
			*ofs = 0;
			Sys_Mkdir( path );
			*ofs = PATHSEPERATOR_CHAR;
		}
	}
}

/*
=================
idFileSystemLocal::CopyFile

Copy a fully specified file from one place to another
=================
*/
void idFileSystemLocal::CopyFile( const char *fromOSPath, const char *toOSPath ) {
	FILE	*f;
	int		len;
	byte	*buf;

	common->Printf( "copy %s to %s\n", fromOSPath, toOSPath );
	f = OpenOSFile( fromOSPath, "rb" );
	if ( !f ) {
		return;
	}
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	fseek( f, 0, SEEK_SET );

	buf = (byte *)Mem_Alloc( len );
	if ( fread( buf, 1, len, f ) != (unsigned int)len ) {
		common->FatalError( "short read in idFileSystemLocal::CopyFile()\n" );
	}
	fclose( f );

	CreateOSPath( toOSPath );
	f = OpenOSFile( toOSPath, "wb" );
	if ( !f ) {
		common->Printf( "could not create destination file\n" );
		Mem_Free( buf );
		return;
	}
	if ( fwrite( buf, 1, len, f ) != (unsigned int)len ) {
		common->FatalError( "short write in idFileSystemLocal::CopyFile()\n" );
	}
	fclose( f );
	Mem_Free( buf );
}

/*
=================
idFileSystemLocal::CopyFile
=================
*/
void idFileSystemLocal::CopyFile( idFile *src, const char *toOSPath ) {
	FILE	*f;
	int		len;
	byte	*buf;

	common->Printf( "copy %s to %s\n", src->GetName(), toOSPath );
	src->Seek( 0, FS_SEEK_END );
	len = src->Tell();
	src->Seek( 0, FS_SEEK_SET );

	buf = (byte *)Mem_Alloc( len );
	if ( src->Read( buf, len ) != len ) {
		common->FatalError( "Short read in idFileSystemLocal::CopyFile()\n" );
	}

	CreateOSPath( toOSPath );
	f = OpenOSFile( toOSPath, "wb" );
	if ( !f ) {
		common->Printf( "could not create destination file\n" );
		Mem_Free( buf );
		return;
	}
	if ( fwrite( buf, 1, len, f ) != (unsigned int)len ) {
		common->FatalError( "Short write in idFileSystemLocal::CopyFile()\n" );
	}
	fclose( f );
	Mem_Free( buf );
}

/*
====================
idFileSystemLocal::ReplaceSeparators

Fix things up differently for win/unix/mac
====================
*/
void idFileSystemLocal::ReplaceSeparators( idStr &path, char sep ) {
	char *s;

	for( s = &path[ 0 ]; *s ; s++ ) {
		if ( *s == '/' || *s == '\\' ) {
			*s = sep;
		}
	}
}

/*
===================
idFileSystemLocal::BuildOSPath
===================
*/
const char *idFileSystemLocal::BuildOSPath( const char *base, const char *game, const char *relativePath ) {
	static char OSPath[MAX_STRING_CHARS];
	idStr newPath;

	if ( fs_caseSensitiveOS.GetBool() || com_developer.GetBool() ) {
		// extract the path, make sure it's all lowercase
		idStr testPath, fileName;

		sprintf( testPath, "%s/%s", game , relativePath );
		testPath.StripFilename();

		if ( testPath.HasUpper() ) {

			common->Warning( "Non-portable: path contains uppercase characters: %s", testPath.c_str() );

			// attempt a fixup on the fly
			if ( fs_caseSensitiveOS.GetBool() ) {
				testPath.ToLower();
				fileName = relativePath;
				fileName.StripPath();
				sprintf( newPath, "%s/%s/%s", base, testPath.c_str(), fileName.c_str() );
				ReplaceSeparators( newPath );
				common->DPrintf( "Fixed up to %s\n", newPath.c_str() );
				idStr::Copynz( OSPath, newPath, sizeof( OSPath ) );
				return OSPath;
			}
		}
	}

	idStr strBase = base;
	strBase.StripTrailing( '/' );
	strBase.StripTrailing( '\\' );
	sprintf( newPath, "%s/%s/%s", strBase.c_str(), game, relativePath );
	ReplaceSeparators( newPath );
	idStr::Copynz( OSPath, newPath, sizeof( OSPath ) );
	return OSPath;
}

/*
================
idFileSystemLocal::OSPathToRelativePath

takes a full OS path, as might be found in data from a media creation
program, and converts it to a relativePath by stripping off directories

Returns false if the osPath tree doesn't match any of the existing
search paths.

================
*/
const char *idFileSystemLocal::OSPathToRelativePath( const char *OSPath ) {
	static char relativePath[MAX_STRING_CHARS];
	char *s, *base;

	// skip a drive letter?

	// search for anything with "base" in it
	// Ase files from max may have the form of:
	// "//Purgatory/purgatory/doom/base/models/mapobjects/bitch/hologirl.tga"
	// which won't match any of our drive letter based search paths
	bool ignoreWarning = false;
#ifdef ID_DEMO_BUILD
	base = strstr( OSPath, BASE_GAMEDIR );	
	idStr tempStr = OSPath;
	tempStr.ToLower();
	if ( ( strstr( tempStr, "//" ) || strstr( tempStr, "w:" ) ) && strstr( tempStr, "/doom/base/") ) {
		// will cause a warning but will load the file. ase models have
		// hard coded doom/base/ in the material names
		base = strstr( OSPath, "base" );
		ignoreWarning = true;
	}
#else
	// look for the first complete directory name
	base = (char *)strstr( OSPath, BASE_GAMEDIR );
	while ( base ) {
		char c1 = '\0', c2;
		if ( base > OSPath ) {
			c1 = *(base - 1);
		}
		c2 = *( base + strlen( BASE_GAMEDIR ) );
		if ( ( c1 == '/' || c1 == '\\' ) && ( c2 == '/' || c2 == '\\' ) ) {
			break;
		}
		base = strstr( base + 1, BASE_GAMEDIR );
	}
#endif
	// fs_game and fs_game_base support - look for first complete name with a mod path
	// ( fs_game searched before fs_game_base )
	const char *fsgame = NULL;
	int igame = 0;
	for ( igame = 0; igame < 2; igame++ ) {
		if ( igame == 0 ) {
			fsgame = fs_game.GetString();
		} else if ( igame == 1 ) {
			fsgame = fs_game_base.GetString();
		}
		if ( base == NULL && fsgame && strlen( fsgame ) ) {
			base = (char *)strstr( OSPath, fsgame );
			while ( base ) {
				char c1 = '\0', c2;
				if ( base > OSPath ) {
					c1 = *(base - 1);
				}
				c2 = *( base + strlen( fsgame ) );
				if ( ( c1 == '/' || c1 == '\\' ) && ( c2 == '/' || c2 == '\\' ) ) {
					break;
				}
				base = strstr( base + 1, fsgame );
			}
		}
	}

	if ( base ) {
		s = strstr( base, "/" );
		if ( !s ) {
			s = strstr( base, "\\" );
		}
		if ( s ) {
			strcpy( relativePath, s + 1 );
			if ( fs_debug.GetInteger() > 1 ) {
				common->Printf( "idFileSystem::OSPathToRelativePath: %s becomes %s\n", OSPath, relativePath );
			}
			return relativePath;
		}
	}

	if ( !ignoreWarning ) {
		common->Warning( "idFileSystem::OSPathToRelativePath failed on %s", OSPath );
	}
	strcpy( relativePath, "" );
	return relativePath;
}

/*
=====================
idFileSystemLocal::RelativePathToOSPath

Returns a fully qualified path that can be used with stdio libraries
=====================
*/
const char *idFileSystemLocal::RelativePathToOSPath( const char *relativePath, const char *basePath ) {
	const char *path = cvarSystem->GetCVarString( basePath );
	if ( !path[0] ) {
		path = fs_savepath.GetString();
	}
	return BuildOSPath( path, gameFolder, relativePath );
}

/*
=================
idFileSystemLocal::RemoveFile
=================
*/
void idFileSystemLocal::RemoveFile( const char *relativePath ) {
	idStr OSPath;

	if ( fs_devpath.GetString()[0] ) {
		OSPath = BuildOSPath( fs_devpath.GetString(), gameFolder, relativePath );
		remove( OSPath );
	}

	OSPath = BuildOSPath( fs_savepath.GetString(), gameFolder, relativePath );
	remove( OSPath );

	ClearDirCache();
}

/*
================
idFileSystemLocal::FileIsInPAK
================
*/
bool idFileSystemLocal::FileIsInPAK( const char *relativePath ) {
	searchpath_t	*search;
	pack_t			*pak;
	fileInPack_t	*pakFile;
	long			hash;

	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}

	if ( !relativePath ) {
		common->FatalError( "idFileSystemLocal::FileIsInPAK: NULL 'relativePath' parameter passed\n" );
	}

	// qpaths are not supposed to have a leading slash
	if ( relativePath[0] == '/' || relativePath[0] == '\\' ) {
		relativePath++;
	}

	// make absolutely sure that it can't back up the path.
	// The searchpaths do guarantee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo" 
	if ( strstr( relativePath, ".." ) || strstr( relativePath, "::" ) ) {
		return false;
	}

	//
	// search through the path, one element at a time
	//

	hash = HashFileName( relativePath );

	for ( search = searchPaths; search; search = search->next ) {
		// is the element a pak file?
		if ( search->pack && search->pack->hashTable[hash] ) {

			// disregard if it doesn't match one of the allowed pure pak files - or is a localization file
			if ( serverPaks.Num() ) {
				GetPackStatus( search->pack );
				if ( search->pack->pureStatus != PURE_NEVER && !serverPaks.Find( search->pack ) ) {
					continue; // not on the pure server pak list
				}
			}

			// look through all the pak file elements
			pak = search->pack;
			pakFile = pak->hashTable[hash];
			do {
				// case and separator insensitive comparisons
				if ( !FilenameCompare( pakFile->name, relativePath ) ) {
					return true;
				}
				pakFile = pakFile->next;
			} while( pakFile != NULL );
		}
	}
	return false;
}

/*
============
idFileSystemLocal::ReadFile

Filename are relative to the search path
a null buffer will just return the file length and time without loading
timestamp can be NULL if not required
============
*/
int idFileSystemLocal::ReadFile( const char *relativePath, void **buffer, ID_TIME_T *timestamp ) {
	idFile *	f;
	byte *		buf;
	int			len;
	bool		isConfig;

	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}

	if ( !relativePath || !relativePath[0] ) {
		common->FatalError( "idFileSystemLocal::ReadFile with empty name\n" );
	}

	if ( timestamp ) {
		*timestamp = FILE_NOT_FOUND_TIMESTAMP;
	}

	if ( buffer ) {
		*buffer = NULL;
	}

	buf = NULL;	// quiet compiler warning

	// if this is a .cfg file and we are playing back a journal, read
	// it from the journal file
	if ( strstr( relativePath, ".cfg" ) == relativePath + strlen( relativePath ) - 4 ) {
		isConfig = true;
		if ( eventLoop && eventLoop->JournalLevel() == 2 ) {
			int		r;

			loadCount++;
			loadStack++;

			common->DPrintf( "Loading %s from journal file.\n", relativePath );
			len = 0;
			r = eventLoop->com_journalDataFile->Read( &len, sizeof( len ) );
			if ( r != sizeof( len ) ) {
				*buffer = NULL;
				return -1;
			}
			buf = (byte *)Mem_ClearedAlloc(len+1);
			*buffer = buf;
			r = eventLoop->com_journalDataFile->Read( buf, len );
			if ( r != len ) {
				common->FatalError( "Read from journalDataFile failed" );
			}

			// guarantee that it will have a trailing 0 for string operations
			buf[len] = 0;

			return len;
		}
	} else {
		isConfig = false;
	}

	// look for it in the filesystem or pack files
	f = OpenFileRead( relativePath, ( buffer != NULL ) );
	if ( f == NULL ) {
		if ( buffer ) {
			*buffer = NULL;
		}
		return -1;
	}
	len = f->Length();

	if ( timestamp ) {
		*timestamp = f->Timestamp();
	}
	
	if ( !buffer ) {
		CloseFile( f );
		return len;
	}

	loadCount++;
	loadStack++;

	buf = (byte *)Mem_ClearedAlloc(len+1);
	*buffer = buf;

	f->Read( buf, len );

	// guarantee that it will have a trailing 0 for string operations
	buf[len] = 0;
	CloseFile( f );

	// if we are journalling and it is a config file, write it to the journal file
	if ( isConfig && eventLoop && eventLoop->JournalLevel() == 1 ) {
		common->DPrintf( "Writing %s to journal file.\n", relativePath );
		eventLoop->com_journalDataFile->Write( &len, sizeof( len ) );
		eventLoop->com_journalDataFile->Write( buf, len );
		eventLoop->com_journalDataFile->Flush();
	}

	return len;
}

/*
=============
idFileSystemLocal::FreeFile
=============
*/
void idFileSystemLocal::FreeFile( void *buffer ) {
	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}
	if ( !buffer ) {
		common->FatalError( "idFileSystemLocal::FreeFile( NULL )" );
	}
	loadStack--;

	Mem_Free( buffer );
}

/*
============
idFileSystemLocal::WriteFile

Filenames are relative to the search path
============
*/
int idFileSystemLocal::WriteFile( const char *relativePath, const void *buffer, int size, const char *basePath ) {
	idFile *f;

	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}

	if ( !relativePath || !buffer ) {
		common->FatalError( "idFileSystemLocal::WriteFile: NULL parameter" );
	}

	f = idFileSystemLocal::OpenFileWrite( relativePath, basePath );
	if ( !f ) {
		common->Printf( "Failed to open %s\n", relativePath );
		return -1;
	}

	size = f->Write( buffer, size );

	CloseFile( f );

	return size;
}

/*
=================
idFileSystemLocal::ParseAddonDef
=================
*/
addonInfo_t *idFileSystemLocal::ParseAddonDef( const char *buf, const int len ) {
	idLexer		src;
	idToken		token, token2;
	addonInfo_t	*info;

	src.LoadMemory( buf, len, "<addon.conf>" );
	src.SetFlags( DECL_LEXER_FLAGS );
	if ( !src.SkipUntilString( "addonDef" ) ) {
		src.Warning( "ParseAddonDef: no addonDef" );
		return NULL;
	}
	if ( !src.ReadToken( &token ) ) {
		src.Warning( "Expected {" );
		return NULL;
	}
	info = new addonInfo_t;
	// read addonDef
	while ( 1 ) {
		if ( !src.ReadToken( &token ) ) {
			delete info;
			return NULL;
		}
		if ( !token.Icmp( "}" ) ) {
			break;
		}
		if ( token.type != TT_STRING ) {
			src.Warning( "Expected quoted string, but found '%s'", token.c_str() );
			delete info;
			return NULL;
		}
		int checksum;
		if ( sscanf( token.c_str(), "0x%x", &checksum ) != 1 && sscanf( token.c_str(), "%x", &checksum ) != 1 ) {
			src.Warning( "Could not parse checksum '%s'", token.c_str() );
			delete info;
			return NULL;
		}
		info->depends.Append( checksum );
	}
	// read any number of mapDef entries
	while ( 1 ) {
		if ( !src.SkipUntilString( "mapDef" ) ) {
			return info;
		}
		if ( !src.ReadToken( &token ) ) {
			src.Warning( "Expected map path" );
			info->mapDecls.DeleteContents( true );
			delete info;
			return NULL;
		}
		idDict *dict = new idDict;
		dict->Set( "path", token.c_str() );
		if ( !src.ReadToken( &token ) ) {
			src.Warning( "Expected {" );
			info->mapDecls.DeleteContents( true );
			delete dict;
			delete info;
			return NULL;
		}
		while ( 1 ) {
			if ( !src.ReadToken( &token ) ) {
				break;
			}
			if ( !token.Icmp( "}" ) ) {
				break;
			}
			if ( token.type != TT_STRING ) {
				src.Warning( "Expected quoted string, but found '%s'", token.c_str() );
				info->mapDecls.DeleteContents( true );
				delete dict;
				delete info;
				return NULL;
			}

			if ( !src.ReadToken( &token2 ) ) {
				src.Warning( "Unexpected end of file" );
				info->mapDecls.DeleteContents( true );
				delete dict;
				delete info;
				return NULL;
			}

			if ( dict->FindKey( token ) ) {
				src.Warning( "'%s' already defined", token.c_str() );
			}
			dict->Set( token, token2 );
		}
		info->mapDecls.Append( dict );
	}
	assert( false );
	return NULL;
}

/*
=================
idFileSystemLocal::LoadZipFile
=================
*/
pack_t *idFileSystemLocal::LoadZipFile( const char *zipfile ) {
	fileInPack_t *	buildBuffer;
	pack_t *		pack;
	unzFile			uf;
	int				err;
	unz_global_info gi;
	char			filename_inzip[MAX_ZIPPED_FILE_NAME];
	unz_file_info	file_info;
	int				i;
	long			hash;
	int				fs_numHeaderLongs;
	int *			fs_headerLongs;
	FILE			*f;
	int				len;
	int				confHash;
	fileInPack_t	*pakFile;

	f = OpenOSFile( zipfile, "rb" );
	if ( !f ) {
		return NULL;
	}
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	fclose( f );

	fs_numHeaderLongs = 0;

	uf = unzOpen( zipfile );
	err = unzGetGlobalInfo( uf, &gi );

	if ( err != UNZ_OK ) {
		return NULL;
	}

	buildBuffer = new fileInPack_t[gi.number_entry];
	pack = new pack_t;
	for( i = 0; i < FILE_HASH_SIZE; i++ ) {
		pack->hashTable[i] = NULL;
	}

	pack->pakFilename = zipfile;
	pack->handle = uf;
	pack->numfiles = gi.number_entry;
	pack->buildBuffer = buildBuffer;
	pack->referenced = false;
	pack->binary = BINARY_UNKNOWN;
	pack->addon = false;
	pack->addon_search = false;
	pack->addon_info = NULL;
	pack->pureStatus = PURE_UNKNOWN;
	pack->isNew = false;

	pack->length = len;

	unzGoToFirstFile(uf);
	fs_headerLongs = (int *)Mem_ClearedAlloc( gi.number_entry * sizeof(int) );
	for ( i = 0; i < (int)gi.number_entry; i++ ) {
		err = unzGetCurrentFileInfo( uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0 );
		if ( err != UNZ_OK ) {
			break;
		}
		if ( file_info.uncompressed_size > 0 ) {
			fs_headerLongs[fs_numHeaderLongs++] = LittleLong( file_info.crc );
		}
		hash = HashFileName( filename_inzip );
		buildBuffer[i].name = filename_inzip;
		buildBuffer[i].name.ToLower();
		buildBuffer[i].name.BackSlashesToSlashes();
		// store the file position in the zip
		unzGetCurrentFileInfoPosition( uf, &buildBuffer[i].pos );
		// add the file to the hash
		buildBuffer[i].next = pack->hashTable[hash];
		pack->hashTable[hash] = &buildBuffer[i];
		// go to the next file in the zip
		unzGoToNextFile(uf);
	}

	// check if this is an addon pak
	pack->addon = false;
	confHash = HashFileName( ADDON_CONFIG );
	for ( pakFile = pack->hashTable[confHash]; pakFile; pakFile = pakFile->next ) {
		if ( !FilenameCompare( pakFile->name, ADDON_CONFIG ) ) {			
			pack->addon = true;			
			idFile_InZip *file = ReadFileFromZip( pack, pakFile, ADDON_CONFIG );
			// may be just an empty file if you don't bother about the mapDef
			if ( file && file->Length() ) {
				char *buf;
				buf = new char[ file->Length() + 1 ];
				file->Read( (void *)buf, file->Length() );
				buf[ file->Length() ] = '\0';
				pack->addon_info = ParseAddonDef( buf, file->Length() );
				delete[] buf;
			}
			if ( file ) {
				CloseFile( file );
			}
			break;
		}
	}

	pack->checksum = MD4_BlockChecksum( fs_headerLongs, 4 * fs_numHeaderLongs );
	pack->checksum = LittleLong( pack->checksum );

	Mem_Free( fs_headerLongs );

	return pack;
}

/*
===============
idFileSystemLocal::AddZipFile
adds a downloaded pak file to the list so we can work out what we have and what we still need
the isNew flag is set to true, indicating that we cannot add this pak to the search lists without a restart
===============
*/
int idFileSystemLocal::AddZipFile( const char *path ) {
	idStr			fullpath = fs_savepath.GetString();
	pack_t			*pak;
	searchpath_t	*search, *last;

	fullpath.AppendPath( path );
	pak = LoadZipFile( fullpath );
	if ( !pak ) {
		common->Warning( "AddZipFile %s failed\n", path );
		return 0;
	}
	// insert the pak at the end of the search list - temporary until we restart
	pak->isNew = true;
	search = new searchpath_t;
	search->dir = NULL;
	search->pack = pak;
	search->next = NULL;
	last = searchPaths;
	while ( last->next ) {
		last = last->next;
	}
	last->next = search;
	common->Printf( "Appended pk4 %s with checksum 0x%x\n", pak->pakFilename.c_str(), pak->checksum );
	return pak->checksum;
}

/*
===============
idFileSystemLocal::AddUnique
===============
*/
int idFileSystemLocal::AddUnique( const char *name, idStrList &list, idHashIndex &hashIndex ) const {
	int i, hashKey;

	hashKey = hashIndex.GenerateKey( name );
	for ( i = hashIndex.First( hashKey ); i >= 0; i = hashIndex.Next( i ) ) {
		if ( list[i].Icmp( name ) == 0 ) {
			return i;
		}
	}
	i = list.Append( name );
	hashIndex.Add( hashKey, i );
	return i;
}

/*
===============
idFileSystemLocal::GetExtensionList
===============
*/
void idFileSystemLocal::GetExtensionList( const char *extension, idStrList &extensionList ) const {
	int s, e, l;

	l = idStr::Length( extension );
	s = 0;
	while( 1 ) {
		e = idStr::FindChar( extension, '|', s, l );
		if ( e != -1 ) {
			extensionList.Append( idStr( extension, s, e ) );
			s = e + 1;
		} else {
			extensionList.Append( idStr( extension, s, l ) );
			break;
		}
	}
}

/*
===============
idFileSystemLocal::GetFileList

Does not clear the list first so this can be used to progressively build a file list.
When 'sort' is true only the new files added to the list are sorted.
===============
*/
int idFileSystemLocal::GetFileList( const char *relativePath, const idStrList &extensions, idStrList &list, idHashIndex &hashIndex, bool fullRelativePath, const char* gamedir ) {
	searchpath_t *	search;
	fileInPack_t *	buildBuffer;
	int				i, j;
	int				pathLength;
	int				length;
	const char *	name;
	pack_t *		pak;
	idStr			work;

	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}

	if ( !extensions.Num() ) {
		return 0;
	}

	if ( !relativePath ) {
		return 0;
	}
	pathLength = strlen( relativePath );
	if ( pathLength ) {
		pathLength++;	// for the trailing '/'
	}

	// search through the path, one element at a time, adding to list
	for( search = searchPaths; search != NULL; search = search->next ) {
		if ( search->dir ) {
			if(gamedir && strlen(gamedir)) {
				if(search->dir->gamedir != gamedir) {
					continue;
				}
			}

			idStrList	sysFiles;
			idStr		netpath;

			netpath = BuildOSPath( search->dir->path, search->dir->gamedir, relativePath );

			for ( i = 0; i < extensions.Num(); i++ ) {

				// scan for files in the filesystem
				ListOSFiles( netpath, extensions[i], sysFiles );

				// if we are searching for directories, remove . and ..
				if ( extensions[i][0] == '/' && extensions[i][1] == 0 ) {
					sysFiles.Remove( "." );
					sysFiles.Remove( ".." );
				}

				for( j = 0; j < sysFiles.Num(); j++ ) {
					// unique the match
					if ( fullRelativePath ) {
						work = relativePath;
						work += "/";
						work += sysFiles[j];
						AddUnique( work, list, hashIndex );
					}
					else {
						AddUnique( sysFiles[j], list, hashIndex );
					}
				}
			}
		} else if ( search->pack ) {
			// look through all the pak file elements

			// exclude any extra packs if we have server paks to search
			if ( serverPaks.Num() ) {
				GetPackStatus( search->pack );
				if ( search->pack->pureStatus != PURE_NEVER && !serverPaks.Find( search->pack ) ) {
					continue; // not on the pure server pak list
				}
			}

			pak = search->pack;
			buildBuffer = pak->buildBuffer;
			for( i = 0; i < pak->numfiles; i++ ) {

				length = buildBuffer[i].name.Length();

				// if the name is not long anough to at least contain the path
				if ( length <= pathLength ) {
					continue;
				}

				name = buildBuffer[i].name;


				// check for a path match without the trailing '/'
				if ( pathLength && idStr::Icmpn( name, relativePath, pathLength - 1 ) != 0 ) {
					continue;
				}
 
				// ensure we have a path, and not just a filename containing the path
				if ( name[ pathLength ] == '\0' || name[pathLength - 1] != '/' ) {
					continue;
				}
 
				// make sure the file is not in a subdirectory
				for ( j = pathLength; name[j+1] != '\0'; j++ ) {
					if ( name[j] == '/' ) {
						break;
					}
				}
				if ( name[j+1] ) {
					continue;
				}

				// check for extension match
				for ( j = 0; j < extensions.Num(); j++ ) {
					if ( length >= extensions[j].Length() && extensions[j].Icmp( name + length - extensions[j].Length() ) == 0 ) {
						break;
					}
				}
				if ( j >= extensions.Num() ) {
					continue;
				}

				// unique the match
				if ( fullRelativePath ) {
					work = relativePath;
					work += "/";
					work += name + pathLength;
					work.StripTrailing( '/' );
					AddUnique( work, list, hashIndex );
				} else {
					work = name + pathLength;
					work.StripTrailing( '/' );
					AddUnique( work, list, hashIndex );
				}
			}
		}
	}

	return list.Num();
}

/*
===============
idFileSystemLocal::ListFiles
===============
*/
idFileList *idFileSystemLocal::ListFiles( const char *relativePath, const char *extension, bool sort, bool fullRelativePath, const char* gamedir ) {
	idHashIndex hashIndex( 4096, 4096 );
	idStrList extensionList;

	idFileList *fileList = new idFileList;
	fileList->basePath = relativePath;

	GetExtensionList( extension, extensionList );

	GetFileList( relativePath, extensionList, fileList->list, hashIndex, fullRelativePath, gamedir );

	if ( sort ) {
		idStrListSortPaths( fileList->list );
	}

	return fileList;
}

/*
===============
idFileSystemLocal::GetFileListTree
===============
*/
int idFileSystemLocal::GetFileListTree( const char *relativePath, const idStrList &extensions, idStrList &list, idHashIndex &hashIndex, const char* gamedir ) {
	int i;
	idStrList slash, folders( 128 );
	idHashIndex folderHashIndex( 1024, 128 );

	// recurse through the subdirectories
	slash.Append( "/" );
	GetFileList( relativePath, slash, folders, folderHashIndex, true, gamedir );
	for ( i = 0; i < folders.Num(); i++ ) {
		if ( folders[i][0] == '.' ) {
			continue;
		}
		if ( folders[i].Icmp( relativePath ) == 0 ){
			continue;
		}
		GetFileListTree( folders[i], extensions, list, hashIndex, gamedir );
	}

	// list files in the current directory
	GetFileList( relativePath, extensions, list, hashIndex, true, gamedir );

	return list.Num();
}

/*
===============
idFileSystemLocal::ListFilesTree
===============
*/
idFileList *idFileSystemLocal::ListFilesTree( const char *relativePath, const char *extension, bool sort, const char* gamedir ) {
	idHashIndex hashIndex( 4096, 4096 );
	idStrList extensionList;

	idFileList *fileList = new idFileList();
	fileList->basePath = relativePath;
	fileList->list.SetGranularity( 4096 );

	GetExtensionList( extension, extensionList );

	GetFileListTree( relativePath, extensionList, fileList->list, hashIndex, gamedir );

	if ( sort ) {
		idStrListSortPaths( fileList->list );
	}

	return fileList;
}

/*
===============
idFileSystemLocal::FreeFileList
===============
*/
void idFileSystemLocal::FreeFileList( idFileList *fileList ) {
	delete fileList;
}

/*
===============
idFileSystemLocal::ListMods
===============
*/
idModList *idFileSystemLocal::ListMods( void ) {
	int 		i;
	const int 	MAX_DESCRIPTION = 256;
	char 		desc[ MAX_DESCRIPTION ];

	idStrList	dirs;
	idStrList	pk4s;

	idModList	*list = new idModList;

	const char	*search[ 4 ];
	int			isearch;

	search[0] = fs_savepath.GetString();
	search[1] = fs_devpath.GetString();
	search[2] = fs_basepath.GetString();
	search[3] = fs_cdpath.GetString();

	for ( isearch = 0; isearch < 4; isearch++ ) {

		dirs.Clear();
		pk4s.Clear();

		// scan for directories
		ListOSFiles( search[ isearch ], "/", dirs );

		dirs.Remove( "." );
		dirs.Remove( ".." );
		dirs.Remove( "base" );
		dirs.Remove( "pb" );

		// see if there are any pk4 files in each directory
		for( i = 0; i < dirs.Num(); i++ ) {
			idStr gamepath = BuildOSPath( search[ isearch ], dirs[ i ], "" );
			ListOSFiles( gamepath, ".pk4", pk4s );
			if ( pk4s.Num() ) {
				if ( !list->mods.Find( dirs[ i ] ) ) {
					// D3 1.3 #31, only list d3xp if the pak is present
					if ( dirs[ i ].Icmp( "d3xp" ) || HasD3XP() ) {
						list->mods.Append( dirs[ i ] );
					}
				}
			}
		}
	}
	   
	list->mods.Sort();

	// read the descriptions for each mod - search all paths
	for ( i = 0; i < list->mods.Num(); i++ ) {

		for ( isearch = 0; isearch < 4; isearch++ ) {

			idStr descfile = BuildOSPath( search[ isearch ], list->mods[ i ], "description.txt" );
			FILE *f = OpenOSFile( descfile, "r" );
			if ( f ) {
				if ( fgets( desc, MAX_DESCRIPTION, f ) ) {
					list->descriptions.Append( desc );
					fclose( f );
					break;
				} else {
					common->DWarning( "Error reading %s", descfile.c_str() );
					fclose( f );
					continue;
				}
			}
		}

		if ( isearch == 4 ) {
			list->descriptions.Append( list->mods[ i ] );
		}
	}

	list->mods.Insert( "" );
	list->descriptions.Insert( "Doom 3" );

	assert( list->mods.Num() == list->descriptions.Num() );

	return list;
}

/*
===============
idFileSystemLocal::FreeModList
===============
*/
void idFileSystemLocal::FreeModList( idModList *modList ) {
	delete modList;
}

/*
===============
idDEntry::Matches
===============
*/
bool idDEntry::Matches(const char *directory, const char *extension) const {
	if ( !idDEntry::directory.Icmp( directory ) && !idDEntry::extension.Icmp( extension ) ) {
		return true;
	}
	return false;
}

/*
===============
idDEntry::Init
===============
*/
void idDEntry::Init( const char *directory, const char *extension, const idStrList &list ) {
	idDEntry::directory = directory;
	idDEntry::extension = extension;
	idStrList::operator=(list);
}

/*
===============
idDEntry::Clear
===============
*/
void idDEntry::Clear( void ) {
	directory.Clear();
	extension.Clear();
	idStrList::Clear();
}

/*
===============
idFileSystemLocal::ListOSFiles

 call to the OS for a listing of files in an OS directory
 optionally, perform some caching of the entries
===============
*/
int	idFileSystemLocal::ListOSFiles( const char *directory, const char *extension, idStrList &list ) {
	int i, j, ret;

	if ( !extension ) {
		extension = "";
	}

	if ( !fs_caseSensitiveOS.GetBool() ) {
		return Sys_ListFiles( directory, extension, list );
	}

	// try in cache
	i = dir_cache_index - 1;
	while( i >= dir_cache_index - dir_cache_count ) {
		j = (i+MAX_CACHED_DIRS) % MAX_CACHED_DIRS;
		if ( dir_cache[j].Matches( directory, extension ) ) {
			if ( fs_debug.GetInteger() ) {
				//common->Printf( "idFileSystemLocal::ListOSFiles: cache hit: %s\n", directory );
			}
			list = dir_cache[j];
			return list.Num();
		}
		i--;
	}

	if ( fs_debug.GetInteger() ) {
		//common->Printf( "idFileSystemLocal::ListOSFiles: cache miss: %s\n", directory );
	}	

	ret = Sys_ListFiles( directory, extension, list );

	if ( ret == -1 ) {
		return -1;
	}

	// push a new entry
	dir_cache[dir_cache_index].Init( directory, extension, list );
	dir_cache_index = (++dir_cache_index) % MAX_CACHED_DIRS;
	if ( dir_cache_count < MAX_CACHED_DIRS ) {
		dir_cache_count++;
	}

	return ret;
}

/*
================
idFileSystemLocal::Dir_f
================
*/
void idFileSystemLocal::Dir_f( const idCmdArgs &args ) {
	idStr		relativePath;
	idStr		extension;
	idFileList *fileList;
	int			i;

	if ( args.Argc() < 2 || args.Argc() > 3 ) {
		common->Printf( "usage: dir <directory> [extension]\n" );
		return;
	}

	if ( args.Argc() == 2 ) {
		relativePath = args.Argv( 1 );
		extension = "";
	}
	else {
		relativePath = args.Argv( 1 );
		extension = args.Argv( 2 );
		if ( extension[0] != '.' ) {
			common->Warning( "extension should have a leading dot" );
		}
	}
	relativePath.BackSlashesToSlashes();
	relativePath.StripTrailing( '/' );

	common->Printf( "Listing of %s/*%s\n", relativePath.c_str(), extension.c_str() );
	common->Printf( "---------------\n" );

	fileList = fileSystemLocal.ListFiles( relativePath, extension );

	for ( i = 0; i < fileList->GetNumFiles(); i++ ) {
		common->Printf( "%s\n", fileList->GetFile( i ) );
	}
	common->Printf( "%d files\n", fileList->list.Num() );

	fileSystemLocal.FreeFileList( fileList );
}

/*
================
idFileSystemLocal::DirTree_f
================
*/
void idFileSystemLocal::DirTree_f( const idCmdArgs &args ) {
	idStr		relativePath;
	idStr		extension;
	idFileList *fileList;
	int			i;

	if ( args.Argc() < 2 || args.Argc() > 3 ) {
		common->Printf( "usage: dirtree <directory> [extension]\n" );
		return;
	}

	if ( args.Argc() == 2 ) {
		relativePath = args.Argv( 1 );
		extension = "";
	}
	else {
		relativePath = args.Argv( 1 );
		extension = args.Argv( 2 );
		if ( extension[0] != '.' ) {
			common->Warning( "extension should have a leading dot" );
		}
	}
	relativePath.BackSlashesToSlashes();
	relativePath.StripTrailing( '/' );

	common->Printf( "Listing of %s/*%s /s\n", relativePath.c_str(), extension.c_str() );
	common->Printf( "---------------\n" );

	fileList = fileSystemLocal.ListFilesTree( relativePath, extension );

	for ( i = 0; i < fileList->GetNumFiles(); i++ ) {
		common->Printf( "%s\n", fileList->GetFile( i ) );
	}
	common->Printf( "%d files\n", fileList->list.Num() );

	fileSystemLocal.FreeFileList( fileList );
}

/*
============
idFileSystemLocal::Path_f
============
*/
void idFileSystemLocal::Path_f( const idCmdArgs &args ) {
	searchpath_t *sp;
	int i;
	idStr status;

	common->Printf( "Current search path:\n" );
	for ( sp = fileSystemLocal.searchPaths; sp; sp = sp->next ) {
		if ( sp->pack ) {
			if ( com_developer.GetBool() ) {
				sprintf( status, "%s (%i files - 0x%x %s", sp->pack->pakFilename.c_str(), sp->pack->numfiles, sp->pack->checksum, sp->pack->referenced ? "referenced" : "not referenced" );
				if ( sp->pack->addon ) {
					status += " - addon)\n";
				} else {
					status += ")\n";
				}
				common->Printf( status.c_str() );
			} else {
				common->Printf( "%s (%i files)\n", sp->pack->pakFilename.c_str(), sp->pack->numfiles );
			}
			if ( fileSystemLocal.serverPaks.Num() ) {
				if ( fileSystemLocal.serverPaks.Find( sp->pack ) ) {
					common->Printf( "    on the pure list\n" );
				} else {
					common->Printf( "    not on the pure list\n" );
				}
			}
		} else {
			common->Printf( "%s/%s\n", sp->dir->path.c_str(), sp->dir->gamedir.c_str() );
		}
	}
	common->Printf( "game DLL: 0x%x in pak: 0x%x\n", fileSystemLocal.gameDLLChecksum, fileSystemLocal.gamePakChecksum );
#if ID_FAKE_PURE
	common->Printf( "Note: ID_FAKE_PURE is enabled\n" );
#endif
	for( i = 0; i < MAX_GAME_OS; i++ ) {
		if ( fileSystemLocal.gamePakForOS[ i ] ) {
			common->Printf( "OS %d - pak 0x%x\n", i, fileSystemLocal.gamePakForOS[ i ] );
		}
	}
	// show addon packs that are *not* in the search lists
	common->Printf( "Addon pk4s:\n" );
	for ( sp = fileSystemLocal.addonPaks; sp; sp = sp->next ) {
		if ( com_developer.GetBool() ) {
			common->Printf( "%s (%i files - 0x%x)\n", sp->pack->pakFilename.c_str(), sp->pack->numfiles, sp->pack->checksum );
		} else {
			common->Printf( "%s (%i files)\n", sp->pack->pakFilename.c_str(), sp->pack->numfiles );
		}		
	}
}

/*
============
idFileSystemLocal::GetOSMask
============
*/
int idFileSystemLocal::GetOSMask( void ) {
	int i, ret = 0;
	for( i = 0; i < MAX_GAME_OS; i++ ) {
		if ( fileSystemLocal.gamePakForOS[ i ] ) {
			ret |= ( 1 << i );
		}
	}
	if ( !ret ) {
		return -1;
	}
	return ret;
}

/*
============
idFileSystemLocal::TouchFile_f

The only purpose of this function is to allow game script files to copy
arbitrary files furing an "fs_copyfiles 1" run.
============
*/
void idFileSystemLocal::TouchFile_f( const idCmdArgs &args ) {
	idFile *f;

	if ( args.Argc() != 2 ) {
		common->Printf( "Usage: touchFile <file>\n" );
		return;
	}

	f = fileSystemLocal.OpenFileRead( args.Argv( 1 ) );
	if ( f ) {
		fileSystemLocal.CloseFile( f );
	}
}

/*
============
idFileSystemLocal::TouchFileList_f

Takes a text file and touches every file in it, use one file per line.
============
*/
void idFileSystemLocal::TouchFileList_f( const idCmdArgs &args ) {
	
	if ( args.Argc() != 2 ) {
		common->Printf( "Usage: touchFileList <filename>\n" );
		return;
	}

	const char *buffer = NULL;
	idParser src( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_ALLOWMULTICHARLITERALS | LEXFL_ALLOWBACKSLASHSTRINGCONCAT );
	if ( fileSystem->ReadFile( args.Argv( 1 ), ( void** )&buffer, NULL ) && buffer ) {
		src.LoadMemory( buffer, strlen( buffer ), args.Argv( 1 ) );
		if ( src.IsLoaded() ) {
			idToken token;
			while( src.ReadToken( &token ) ) {
				common->Printf( "%s\n", token.c_str() );
				session->UpdateScreen();
				idFile *f = fileSystemLocal.OpenFileRead( token );
				if ( f ) {
					fileSystemLocal.CloseFile( f );
				}
			}
		}
	}

}


/*
================
idFileSystemLocal::AddGameDirectory

Sets gameFolder, adds the directory to the head of the search paths, then loads any pk4 files.
================
*/
void idFileSystemLocal::AddGameDirectory( const char *path, const char *dir ) {
	int				i;
	searchpath_t *	search;
	pack_t *		pak;
	idStr			pakfile;
	idStrList		pakfiles;

	// check if the search path already exists
	for ( search = searchPaths; search; search = search->next ) {
		// if this element is a pak file
		if ( !search->dir ) {
			continue;
		}
		if ( search->dir->path.Cmp( path ) == 0 && search->dir->gamedir.Cmp( dir ) == 0 ) {
			return;
		}
	}

	gameFolder = dir;

	//
	// add the directory to the search path
	//
	search = new searchpath_t;
	search->dir = new directory_t;
	search->pack = NULL;

	search->dir->path = path;
	search->dir->gamedir = dir;
	search->next = searchPaths;
	searchPaths = search;

	// find all pak files in this directory
	pakfile = BuildOSPath( path, dir, "" );
	pakfile[ pakfile.Length() - 1 ] = 0;	// strip the trailing slash

	ListOSFiles( pakfile, ".pk4", pakfiles );

	// sort them so that later alphabetic matches override
	// earlier ones. This makes pak1.pk4 override pak0.pk4
	pakfiles.Sort();

	for ( i = 0; i < pakfiles.Num(); i++ ) {
		pakfile = BuildOSPath( path, dir, pakfiles[i] );
		pak = LoadZipFile( pakfile );
		if ( !pak ) {
			continue;
		}
		// insert the pak after the directory it comes from
		search = new searchpath_t;
		search->dir = NULL;
		search->pack = pak;
		search->next = searchPaths->next;
		searchPaths->next = search;
		common->Printf( "Loaded pk4 %s with checksum 0x%x\n", pakfile.c_str(), pak->checksum );
	}
}

/*
================
idFileSystemLocal::SetupGameDirectories

  Takes care of the correct search order.
================
*/
void idFileSystemLocal::SetupGameDirectories( const char *gameName ) {
	// setup cdpath
	if ( fs_cdpath.GetString()[0] ) {
		AddGameDirectory( fs_cdpath.GetString(), gameName );
	}

	// setup basepath
	if ( fs_basepath.GetString()[0] ) {
		AddGameDirectory( fs_basepath.GetString(), gameName );
	}

	// setup devpath
	if ( fs_devpath.GetString()[0] ) {
		AddGameDirectory( fs_devpath.GetString(), gameName );
	}

	// setup savepath
	if ( fs_savepath.GetString()[0] ) {
		AddGameDirectory( fs_savepath.GetString(), gameName );
	}
}

/*
===============
idFileSystemLocal::FollowDependencies
===============
*/
void idFileSystemLocal::FollowAddonDependencies( pack_t *pak ) {
	assert( pak );
	if ( !pak->addon_info || !pak->addon_info->depends.Num() ) {
		return;
	}
	int i, num = pak->addon_info->depends.Num();
	for ( i = 0; i < num; i++ ) {
		pack_t *deppak = GetPackForChecksum( pak->addon_info->depends[ i ], true );
		if ( deppak ) {
			// make sure it hasn't been marked for search already
			if ( !deppak->addon_search ) {
				// must clean addonChecksums as we go
				int addon_index = addonChecksums.FindIndex( deppak->checksum );
				if ( addon_index >= 0 ) {
					addonChecksums.RemoveIndex( addon_index );
				}
				deppak->addon_search = true;
				common->Printf( "Addon pk4 %s 0x%x depends on pak %s 0x%x, will be searched\n",
								pak->pakFilename.c_str(), pak->checksum,
								deppak->pakFilename.c_str(), deppak->checksum );
				FollowAddonDependencies( deppak );
			}
		} else {
			common->Printf( "Addon pk4 %s 0x%x depends on unknown pak 0x%x\n",
							pak->pakFilename.c_str(), pak->checksum, pak->addon_info->depends[ i ] );
		}
	}
}

/*
================
idFileSystemLocal::Startup
================
*/
void idFileSystemLocal::Startup( void ) {
	searchpath_t	**search;
	int				i;
	pack_t			*pak;
	int				addon_index;

	common->Printf( "------ Initializing File System ------\n" );

	if ( restartChecksums.Num() ) {
		common->Printf( "restarting in pure mode with %d pak files\n", restartChecksums.Num() );
	}
	if ( addonChecksums.Num() ) {
		common->Printf( "restarting filesystem with %d addon pak file(s) to include\n", addonChecksums.Num() );
	}

	SetupGameDirectories( BASE_GAMEDIR );

	// fs_game_base override
	if ( fs_game_base.GetString()[0] &&
		 idStr::Icmp( fs_game_base.GetString(), BASE_GAMEDIR ) ) {
		SetupGameDirectories( fs_game_base.GetString() );
	}

	// fs_game override
	if ( fs_game.GetString()[0] &&
		 idStr::Icmp( fs_game.GetString(), BASE_GAMEDIR ) &&
		 idStr::Icmp( fs_game.GetString(), fs_game_base.GetString() ) ) {
		SetupGameDirectories( fs_game.GetString() );
	}

	// currently all addons are in the search list - deal with filtering out and dependencies now
	// scan through and deal with dependencies
	search = &searchPaths;
	while ( *search ) {
		if ( !( *search )->pack || !( *search )->pack->addon ) {
			search = &( ( *search )->next );
			continue;
		}
		pak = ( *search )->pack;
		if ( fs_searchAddons.GetBool() ) {
			// when we have fs_searchAddons on we should never have addonChecksums
			assert( !addonChecksums.Num() );
			pak->addon_search = true;
			search = &( ( *search )->next );
			continue;
		}
		addon_index = addonChecksums.FindIndex( pak->checksum );
		if ( addon_index >= 0 ) {
			assert( !pak->addon_search );	// any pak getting flagged as addon_search should also have been removed from addonChecksums already
			pak->addon_search = true;
			addonChecksums.RemoveIndex( addon_index );
			FollowAddonDependencies( pak );
		}
		search = &( ( *search )->next );
	}

	// now scan to filter out addons not marked addon_search
	search = &searchPaths;
	while ( *search ) {
		if ( !( *search )->pack || !( *search )->pack->addon ) {
			search = &( ( *search )->next );
			continue;
		}
		assert( !( *search )->dir );
		pak = ( *search )->pack;
		if ( pak->addon_search ) {
			common->Printf( "Addon pk4 %s with checksum 0x%x is on the search list\n",
							pak->pakFilename.c_str(), pak->checksum );
			search = &( ( *search )->next );
		} else {
			// remove from search list, put in addons list
			searchpath_t *paksearch = *search;
			*search = ( *search )->next;
			paksearch->next = addonPaks;
			addonPaks = paksearch;
			common->Printf( "Addon pk4 %s with checksum 0x%x is on addon list\n",
							pak->pakFilename.c_str(), pak->checksum );				
		}
	}

	// all addon paks found and accounted for
	assert( !addonChecksums.Num() );
	addonChecksums.Clear();	// just in case

	if ( restartChecksums.Num() ) {
		search = &searchPaths;
		while ( *search ) {
			if ( !( *search )->pack ) {
				search = &( ( *search )->next );
				continue;
			}
			if ( ( i = restartChecksums.FindIndex( ( *search )->pack->checksum ) ) != -1 ) {
				if ( i == 0 ) {
					// this pak is the next one in the pure search order
					serverPaks.Append( ( *search )->pack );
					restartChecksums.RemoveIndex( 0 );
					if ( !restartChecksums.Num() ) {
						break; // early out, we're done
					}
					search = &( ( *search )->next );
					continue;
				} else {
					// this pak will be on the pure list, but order is not right yet
					searchpath_t	*aux;
					aux = ( *search )->next;
					if ( !aux ) {
						// last of the list can't be swapped back
						if ( fs_debug.GetBool() ) {
							common->Printf( "found pure checksum %x at index %d, but the end of search path is reached\n", ( *search )->pack->checksum, i );
							idStr checks;
							checks.Clear();
							for ( i = 0; i < serverPaks.Num(); i++ ) {
								checks += va( "%p ", serverPaks[ i ] );
							}
							common->Printf( "%d pure paks - %s \n", serverPaks.Num(), checks.c_str() );
							checks.Clear();
							for ( i = 0; i < restartChecksums.Num(); i++ ) {
								checks += va( "%x ", restartChecksums[ i ] );
							}
							common->Printf( "%d paks left - %s\n", restartChecksums.Num(), checks.c_str() );
						}
						common->FatalError( "Failed to restart with pure mode restrictions for server connect" );
					}
					// put this search path at the end of the list
					searchpath_t *search_end;
					search_end = ( *search )->next;
					while ( search_end->next ) {
						search_end = search_end->next;
					}
					search_end->next = *search;
					*search = ( *search )->next;
					search_end->next->next = NULL;
					continue;
				}
			}
			// this pak is not on the pure list
			search = &( ( *search )->next );
		}
		// the list must be empty
		if ( restartChecksums.Num() ) {
			if ( fs_debug.GetBool() ) {
				idStr checks;
				checks.Clear();
				for ( i = 0; i < serverPaks.Num(); i++ ) {
					checks += va( "%p ", serverPaks[ i ] );
				}
				common->Printf( "%d pure paks - %s \n", serverPaks.Num(), checks.c_str() );
				checks.Clear();
				for ( i = 0; i < restartChecksums.Num(); i++ ) {
					checks += va( "%x ", restartChecksums[ i ] );
				}
				common->Printf( "%d paks left - %s\n", restartChecksums.Num(), checks.c_str() );
			}
			common->FatalError( "Failed to restart with pure mode restrictions for server connect" );
		}
		// also the game pak checksum
		// we could check if the game pak is actually present, but we would not be restarting if there wasn't one @ first pure check
		gamePakChecksum = restartGamePakChecksum;
	}

	// add our commands
	cmdSystem->AddCommand( "dir", Dir_f, CMD_FL_SYSTEM, "lists a folder", idCmdSystem::ArgCompletion_FileName );
	cmdSystem->AddCommand( "dirtree", DirTree_f, CMD_FL_SYSTEM, "lists a folder with subfolders" );
	cmdSystem->AddCommand( "path", Path_f, CMD_FL_SYSTEM, "lists search paths" );
	cmdSystem->AddCommand( "touchFile", TouchFile_f, CMD_FL_SYSTEM, "touches a file" );
	cmdSystem->AddCommand( "touchFileList", TouchFileList_f, CMD_FL_SYSTEM, "touches a list of files" );

	// print the current search paths
	Path_f( idCmdArgs() );

	common->Printf( "file system initialized.\n" );
	common->Printf( "--------------------------------------\n" );
}

/*
===================
idFileSystemLocal::SetRestrictions

Looks for product keys and restricts media add on ability
if the full version is not found
===================
*/
void idFileSystemLocal::SetRestrictions( void ) {
#ifdef ID_DEMO_BUILD
	common->Printf( "\nRunning in restricted demo mode.\n\n" );
	// make sure that the pak file has the header checksum we expect
	searchpath_t	*search;
	for ( search = searchPaths; search; search = search->next ) {
		if ( search->pack ) {
			// a tiny attempt to keep the checksum from being scannable from the exe
			if ( ( search->pack->checksum ^ 0x84268436u ) != ( DEMO_PAK_CHECKSUM ^ 0x84268436u ) ) {
				common->FatalError( "Corrupted %s: 0x%x", search->pack->pakFilename.c_str(), search->pack->checksum );
			}
		}
	}
	cvarSystem->SetCVarBool( "fs_restrict", true );
#endif
}

/*
=====================
idFileSystemLocal::UpdatePureServerChecksums
=====================
*/
void idFileSystemLocal::UpdatePureServerChecksums( void ) {
	searchpath_t	*search;
	int				i;
	pureStatus_t	status;

	serverPaks.Clear();
	for ( search = searchPaths; search; search = search->next ) {
		// is the element a referenced pak file?
		if ( !search->pack ) {
			continue;
		}
		status = GetPackStatus( search->pack );
		if ( status == PURE_NEVER ) {
			continue;
		}
		if ( status == PURE_NEUTRAL && !search->pack->referenced ) {
			continue;
		}
		serverPaks.Append( search->pack );
		if ( serverPaks.Num() >= MAX_PURE_PAKS ) {
			common->FatalError( "MAX_PURE_PAKS ( %d ) exceeded\n", MAX_PURE_PAKS );
		}
	}
	if ( fs_debug.GetBool() ) {
		idStr checks;
		for ( i = 0; i < serverPaks.Num(); i++ ) {
			checks += va( "%x ", serverPaks[ i ]->checksum );
		}
		common->Printf( "set pure list - %d paks ( %s)\n", serverPaks.Num(), checks.c_str() );
	}
}

/*
=====================
idFileSystemLocal::UpdateGamePakChecksums
=====================
*/
bool idFileSystemLocal::UpdateGamePakChecksums( void ) {
	searchpath_t	*search;
	fileInPack_t	*pakFile;
	int				confHash;
	idFile			*confFile;
	char			*buf;
	idLexer			*lexConf;
	idToken			token;
	int				id;

	confHash = HashFileName( BINARY_CONFIG );

	memset( gamePakForOS, 0, sizeof( gamePakForOS ) );
	for ( search = searchPaths; search; search = search->next ) {
		if ( !search->pack ) {
			continue;
		}
		search->pack->binary = BINARY_NO;
		for ( pakFile = search->pack->hashTable[confHash]; pakFile; pakFile = pakFile->next ) {
			if ( !FilenameCompare( pakFile->name, BINARY_CONFIG ) ) {
				search->pack->binary = BINARY_YES;
				confFile = ReadFileFromZip( search->pack, pakFile, BINARY_CONFIG );
				buf = new char[ confFile->Length() + 1 ];
				confFile->Read( (void *)buf, confFile->Length() );
				buf[ confFile->Length() ] = '\0';
				lexConf = new idLexer( buf, confFile->Length(), confFile->GetFullPath() );
				while ( lexConf->ReadToken( &token ) ) {
					if ( token.IsNumeric() ) {
						id = atoi( token );
						if ( id < MAX_GAME_OS && !gamePakForOS[ id ] ) {
							if ( fs_debug.GetBool() ) {
								common->Printf( "Adding game pak checksum for OS %d: %s 0x%x\n", id, confFile->GetFullPath(), search->pack->checksum );
							}
 							gamePakForOS[ id ] = search->pack->checksum;
						}
					}
				}
				CloseFile( confFile );
				delete lexConf;
				delete[] buf;
			}
		}
	}

	// some sanity checks on the game code references
	// make sure that at least the local OS got a pure reference
	if ( !gamePakForOS[ BUILD_OS_ID ] ) {
		common->Warning( "No game code pak reference found for the local OS" );
		return false;
	}

	if ( !cvarSystem->GetCVarBool( "net_serverAllowServerMod" ) &&
		gamePakChecksum != gamePakForOS[ BUILD_OS_ID ] ) {
		common->Warning( "The current game code doesn't match pak files (net_serverAllowServerMod is off)" );
		return false;
	}

	return true;
}

/*
=====================
idFileSystemLocal::GetPackForChecksum
=====================
*/
pack_t* idFileSystemLocal::GetPackForChecksum( int checksum, bool searchAddons ) {
	searchpath_t	*search;
	for ( search = searchPaths; search; search = search->next ) {
		if ( !search->pack ) {
			continue;
		}
		if ( search->pack->checksum == checksum ) {
			return search->pack;
		}
	}
	if ( searchAddons ) {
		for ( search = addonPaks; search; search = search->next ) {
			assert( search->pack && search->pack->addon );
			if ( search->pack->checksum == checksum ) {
				return search->pack;
			}
		}
	}
	return NULL;
}

/*
===============
idFileSystemLocal::ValidateDownloadPakForChecksum
===============
*/
int idFileSystemLocal::ValidateDownloadPakForChecksum( int checksum, char path[ MAX_STRING_CHARS ], bool isBinary ) {
	int			i;
	idStrList	testList;
	idStr		name;
	idStr		relativePath;
	bool		pakBinary;
	pack_t		*pak = GetPackForChecksum( checksum );

	if ( !pak ) {
		return 0;
	}

	// validate this pak for a potential download
	// ignore pak*.pk4 for download. those are reserved to distribution and cannot be downloaded
	name = pak->pakFilename;
	name.StripPath();
	if ( strstr( name.c_str(), "pak" ) == name.c_str() ) {
		common->DPrintf( "%s is not a donwloadable pak\n", pak->pakFilename.c_str() );
		return 0;
	}
	// check the binary
	// a pure server sets the binary flag when starting the game
	assert( pak->binary != BINARY_UNKNOWN );
	pakBinary = ( pak->binary == BINARY_YES ) ? true : false;
	if ( isBinary != pakBinary ) {
		common->DPrintf( "%s binary flag mismatch\n", pak->pakFilename.c_str() );
		return 0;
	}

	// extract a path that includes the fs_game: != OSPathToRelativePath
	testList.Append( fs_savepath.GetString() );
	testList.Append( fs_devpath.GetString() );
	testList.Append( fs_basepath.GetString() );
	testList.Append( fs_cdpath.GetString() );
	for ( i = 0; i < testList.Num(); i ++ ) {
		if ( testList[ i ].Length() && !testList[ i ].Icmpn( pak->pakFilename, testList[ i ].Length() ) ) {
			relativePath = pak->pakFilename.c_str() + testList[ i ].Length() + 1;
			break;
		}
	}
	if ( i == testList.Num() ) {
		common->Warning( "idFileSystem::ValidateDownloadPak: failed to extract relative path for %s", pak->pakFilename.c_str() );
		return 0;
	}
	idStr::Copynz( path, relativePath, MAX_STRING_CHARS );
	return pak->length;
}

/*
=====================
idFileSystemLocal::ClearPureChecksums
=====================
*/
void idFileSystemLocal::ClearPureChecksums( void ) {
	common->DPrintf( "Cleared pure server lock\n" );
	serverPaks.Clear();
}

/*
=====================
idFileSystemLocal::SetPureServerChecksums
set the pure paks according to what the server asks
if that's not possible, identify why and build an answer
can be:
  loadedFileFromDir - some files were loaded from directories instead of paks (a restart in pure pak-only is required)
  missing/wrong checksums - some pak files would need to be installed/updated (downloaded for instance)
  some pak files currently referenced are not referenced by the server
  wrong order - if the pak order doesn't match, means some stuff could have been loaded from somewhere else
server referenced files are prepended to the list if possible ( that doesn't break pureness )
DLL:
  the checksum of the pak containing the DLL is maintained seperately, the server can send different replies by OS
=====================
*/
fsPureReply_t idFileSystemLocal::SetPureServerChecksums( const int pureChecksums[ MAX_PURE_PAKS ], int _gamePakChecksum, int missingChecksums[ MAX_PURE_PAKS ], int *missingGamePakChecksum ) {
	pack_t			*pack;
	int				i, j, imissing;
	bool			success = true;
	bool			canPrepend = true;
	char			dllName[MAX_OSPATH];
	int				dllHash;
	fileInPack_t *	pakFile;

	sys->DLL_GetFileName( "game", dllName, MAX_OSPATH );
	dllHash = HashFileName( dllName );

	imissing = 0;
	missingChecksums[ 0 ] = 0;
	assert( missingGamePakChecksum );
	*missingGamePakChecksum = 0;

	if ( pureChecksums[ 0 ] == 0 ) {
		ClearPureChecksums();
		return PURE_OK;
	}

	if ( !serverPaks.Num() ) {
		// there was no pure lockdown yet - lock to what we already have
		UpdatePureServerChecksums();
	}
	i = 0; j = 0;
	while ( pureChecksums[ i ] ) {
		if ( j < serverPaks.Num() && serverPaks[ j ]->checksum == pureChecksums[ i ] ) {
			canPrepend = false; // once you start matching into the list there is no prepending anymore
			i++; j++; // the pak is matched, is in the right order, continue..
		} else {
			pack = GetPackForChecksum( pureChecksums[ i ], true );
			if ( pack && pack->addon && !pack->addon_search ) {
				// this is an addon pack, and it's not on our current search list
				// setting success to false meaning that a restart including this addon is required
				if ( fs_debug.GetBool() ) {
					common->Printf( "pak %s checksumed 0x%x is on addon list. Restart required.\n", pack->pakFilename.c_str(), pack->checksum );
				}
				success = false;
			}
			if ( pack && pack->isNew ) {
				// that's a downloaded pack, we will need to restart
				if ( fs_debug.GetBool() ) {
					common->Printf( "pak %s checksumed 0x%x is a newly downloaded file. Restart required.\n", pack->pakFilename.c_str(), pack->checksum );
				}
				success = false;
			}
			if ( pack ) {
				if ( canPrepend ) {
					// we still have a chance
					if ( fs_debug.GetBool() ) {
						common->Printf( "prepend pak %s checksumed 0x%x at index %d\n", pack->pakFilename.c_str(), pack->checksum, j );
					}
					// NOTE: there is a light possibility this adds at the end of the list if UpdatePureServerChecksums didn't set anything
					serverPaks.Insert( pack, j );
					i++; j++; // continue..
				} else {
					success = false;
					if ( fs_debug.GetBool() ) {
						// verbose the situation
						if ( serverPaks.Find( pack ) ) {
							common->Printf( "pak %s checksumed 0x%x is in the pure list at wrong index. Current index is %d, found at %d\n", pack->pakFilename.c_str(), pack->checksum, j, serverPaks.FindIndex( pack ) );
						} else {
							common->Printf( "pak %s checksumed 0x%x can't be added to pure list because of search order\n", pack->pakFilename.c_str(), pack->checksum );
						}
					}
					i++; // advance server checksums only
				}
			} else {
				// didn't find a matching checksum
				success = false;
				missingChecksums[ imissing++ ] = pureChecksums[ i ];
				missingChecksums[ imissing ] = 0;
				if ( fs_debug.GetBool() ) {
					common->Printf( "checksum not found - 0x%x\n", pureChecksums[ i ] );
				}
				i++; // advance the server checksums only
			}
		}
	}
	while ( j < serverPaks.Num() ) {
		success = false; // just in case some extra pak files are referenced at the end of our local list
		if ( fs_debug.GetBool() ) {
			common->Printf( "pak %s checksumed 0x%x is an extra reference at the end of local pure list\n", serverPaks[ j ]->pakFilename.c_str(), serverPaks[ j ]->checksum );
		}
		j++;
	}

	// DLL checksuming
	if ( !_gamePakChecksum ) {
		// server doesn't have knowledge of code we can use ( OS issue )
		return PURE_NODLL;
	}
	assert( gameDLLChecksum );
#if ID_FAKE_PURE
	gamePakChecksum = _gamePakChecksum;
#endif
	if ( _gamePakChecksum != gamePakChecksum ) {
		// current DLL is wrong, search for a pak with the approriate checksum
		// ( search all paks, the pure list is not relevant here )
		pack = GetPackForChecksum( _gamePakChecksum );
		if ( !pack ) {
			if ( fs_debug.GetBool() ) {
				common->Printf( "missing the game code pak ( 0x%x )\n", _gamePakChecksum );
			}
			// if there are other paks missing they have also been marked above
			*missingGamePakChecksum = _gamePakChecksum;
			return PURE_MISSING;
		}
		// if assets paks are missing, don't try any of the DLL restart / NODLL
		if ( imissing ) {
			return PURE_MISSING;
		}
		// we have a matching pak
		if ( fs_debug.GetBool() ) {
			common->Printf( "server's game code pak candidate is '%s' ( 0x%x )\n", pack->pakFilename.c_str(), pack->checksum );
		}
		// make sure there is a valid DLL for us
		if ( pack->hashTable[ dllHash ] ) {
			for ( pakFile = pack->hashTable[ dllHash ]; pakFile; pakFile = pakFile->next ) {
				if ( !FilenameCompare( pakFile->name, dllName ) ) {
					gamePakChecksum = _gamePakChecksum;		// this will be used to extract the DLL in pure mode FindDLL
					return PURE_RESTART;
				}
			}
		}
		common->Warning( "media is misconfigured. server claims pak '%s' ( 0x%x ) has media for us, but '%s' is not found\n", pack->pakFilename.c_str(), pack->checksum, dllName );
		return PURE_NODLL;
	}

	// we reply to missing after DLL check so it can be part of the list
	if ( imissing ) {
		return PURE_MISSING;
	}

	// one last check
	if ( loadedFileFromDir ) {
		success = false;
		if ( fs_debug.GetBool() ) {
			common->Printf( "SetPureServerChecksums: there are files loaded from dir\n" );
		}
	}
	return ( success ? PURE_OK : PURE_RESTART );
}

/*
=====================
idFileSystemLocal::GetPureServerChecksums
=====================
*/
void idFileSystemLocal::GetPureServerChecksums( int checksums[ MAX_PURE_PAKS ], int OS, int *_gamePakChecksum ) {
	int i;

	for ( i = 0; i < serverPaks.Num(); i++ ) {
		checksums[ i ] = serverPaks[ i ]->checksum;
	}
	checksums[ i ] = 0;
	if ( _gamePakChecksum ) {
		if ( OS >= 0 ) {
			*_gamePakChecksum = gamePakForOS[ OS ];
		} else {
			*_gamePakChecksum = gamePakChecksum;
		}
	}
}

/*
=====================
idFileSystemLocal::SetRestartChecksums
=====================
*/
void idFileSystemLocal::SetRestartChecksums( const int pureChecksums[ MAX_PURE_PAKS ], int gamePakChecksum ) {
	int		i;
	pack_t	*pack;

	restartChecksums.Clear();
	i = 0;
	while ( pureChecksums[ i ] ) {
		pack = GetPackForChecksum( pureChecksums[ i ], true );
		if ( !pack ) {
			common->FatalError( "SetRestartChecksums failed: no pak for checksum 0x%x\n", pureChecksums[i] );
		}
		if ( pack->addon && addonChecksums.FindIndex( pack->checksum ) < 0 ) {
			// can't mark it pure if we're not even gonna search it :-)
			addonChecksums.Append( pack->checksum );
		}
		restartChecksums.Append( pureChecksums[ i ] );
		i++;
	}
	restartGamePakChecksum = gamePakChecksum;
}

/*
================
idFileSystemLocal::Init

Called only at inital startup, not when the filesystem
is resetting due to a game change
================
*/
void idFileSystemLocal::Init( void ) {
	// allow command line parms to override our defaults
	// we have to specially handle this, because normal command
	// line variable sets don't happen until after the filesystem
	// has already been initialized
	common->StartupVariable( "fs_basepath", false );
	common->StartupVariable( "fs_savepath", false );
	common->StartupVariable( "fs_cdpath", false );
	common->StartupVariable( "fs_devpath", false );
	common->StartupVariable( "fs_game", false );
	common->StartupVariable( "fs_game_base", false );
	common->StartupVariable( "fs_copyfiles", false );
	common->StartupVariable( "fs_restrict", false );
	common->StartupVariable( "fs_searchAddons", false );

#if !ID_ALLOW_D3XP
	if ( fs_game.GetString()[0] && !idStr::Icmp( fs_game.GetString(), "d3xp" ) ) {
		 fs_game.SetString( NULL );
	}
	if ( fs_game_base.GetString()[0] && !idStr::Icmp( fs_game_base.GetString(), "d3xp" ) ) {
		  fs_game_base.SetString( NULL );
	}
#endif	
	
	if ( fs_basepath.GetString()[0] == '\0' ) {
		fs_basepath.SetString( Sys_DefaultBasePath() );
	}
	if ( fs_savepath.GetString()[0] == '\0' ) {
		fs_savepath.SetString( Sys_DefaultSavePath() );
	}
	if ( fs_cdpath.GetString()[0] == '\0' ) {
		fs_cdpath.SetString( Sys_DefaultCDPath() );
	}

	if ( fs_devpath.GetString()[0] == '\0' ) {
#ifdef WIN32
		fs_devpath.SetString( fs_cdpath.GetString()[0] ? fs_cdpath.GetString() : fs_basepath.GetString() );
#else
		fs_devpath.SetString( fs_savepath.GetString() );
#endif
	}

	// try to start up normally
	Startup( );

	// see if we are going to allow add-ons
	SetRestrictions();

	// spawn a thread to handle background file reads
	StartBackgroundDownloadThread();

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	// Dedicated servers can run with no outside files at all
	if ( ReadFile( "default.cfg", NULL, NULL ) <= 0 ) {
		common->FatalError( "Couldn't load default.cfg" );
	}
}

/*
================
idFileSystemLocal::Restart
================
*/
void idFileSystemLocal::Restart( void ) {
	// free anything we currently have loaded
	Shutdown( true );

	Startup( );

	// see if we are going to allow add-ons
	SetRestrictions();

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	if ( ReadFile( "default.cfg", NULL, NULL ) <= 0 ) {
		common->FatalError( "Couldn't load default.cfg" );
	}
}

/*
================
idFileSystemLocal::Shutdown

Frees all resources and closes all files
================
*/
void idFileSystemLocal::Shutdown( bool reloading ) {
	searchpath_t *sp, *next, *loop;

	gameFolder.Clear();

	serverPaks.Clear();
	if ( !reloading ) {
		restartChecksums.Clear();
		addonChecksums.Clear();
	}
	loadedFileFromDir = false;
	gameDLLChecksum = 0;
	gamePakChecksum = 0;

	ClearDirCache();

	// free everything - loop through searchPaths and addonPaks
	for ( loop = searchPaths; loop; loop == searchPaths ? loop = addonPaks : loop = NULL ) {
		for ( sp = loop; sp; sp = next ) {
			next = sp->next;

			if ( sp->pack ) {
				unzClose( sp->pack->handle );
				delete [] sp->pack->buildBuffer;
				if ( sp->pack->addon_info ) {
					sp->pack->addon_info->mapDecls.DeleteContents( true );
					delete sp->pack->addon_info;
				}
				delete sp->pack;
			}
			if ( sp->dir ) {
				delete sp->dir;
			}
			delete sp;
		}
	}

	// any FS_ calls will now be an error until reinitialized
	searchPaths = NULL;
	addonPaks = NULL;

	cmdSystem->RemoveCommand( "path" );
	cmdSystem->RemoveCommand( "dir" );
	cmdSystem->RemoveCommand( "dirtree" );
	cmdSystem->RemoveCommand( "touchFile" );

	mapDict.Clear();
}

/*
================
idFileSystemLocal::IsInitialized
================
*/
bool idFileSystemLocal::IsInitialized( void ) const {
	return ( searchPaths != NULL );
}


/*
=================================================================================

Opening files

=================================================================================
*/

/*
===========
idFileSystemLocal::FileAllowedFromDir
===========
*/
bool idFileSystemLocal::FileAllowedFromDir( const char *path ) {
	unsigned int l;

	l = strlen( path );

	if ( !strcmp( path + l - 4, ".cfg" )		// for config files
		|| !strcmp( path + l - 4, ".dat" )		// for journal files
		|| !strcmp( path + l - 4, ".dll" )		// dynamic modules are handled a different way for pure
		|| !strcmp( path + l - 3, ".so" )
		|| ( l > 6 && !strcmp( path + l - 6, ".dylib" ) )
		|| ( l > 10 && !strcmp( path + l - 10, ".scriptcfg" ) )	// configuration script, such as map cycle
#if ID_PURE_ALLOWDDS
		 || !strcmp( path + l - 4, ".dds" )
#endif
		 ) {
		// note: cd and xp keys, as well as config.spec are opened through an explicit OS path and don't hit this
		return true;
	}
	// savegames
	if ( strstr( path, "savegames" ) == path &&
		( !strcmp( path + l - 4, ".tga" ) || !strcmp( path + l -4, ".txt" ) || !strcmp( path + l - 5, ".save" ) ) ) {
		return true;
	}
	// screen shots
	if ( strstr( path, "screenshots" ) == path && !strcmp( path + l - 4, ".tga" ) ) {
		return true;
	}
	// objective tgas
	if ( strstr( path, "maps/game" ) == path && 
		!strcmp( path + l - 4, ".tga" ) ) {
		return true;
	}
	// splash screens extracted from addons
	if ( strstr( path, "guis/assets/splash/addon" ) == path &&
		 !strcmp( path + l -4, ".tga" ) ) {
		return true;
	}

	return false;
}

/*
===========
idFileSystemLocal::GetPackStatus
===========
*/
pureStatus_t idFileSystemLocal::GetPackStatus( pack_t *pak ) {
	int				i, l, hashindex;
	fileInPack_t	*file;
	bool			abrt;
	idStr			name;

	if ( pak->pureStatus != PURE_UNKNOWN ) {
		return pak->pureStatus;
	}

	// check content for PURE_NEVER
	i = 0;
	file = pak->buildBuffer;
	for ( hashindex = 0; hashindex < FILE_HASH_SIZE; hashindex++ ) {
		abrt = false;
		file = pak->hashTable[ hashindex ];
		while ( file ) {
			abrt = true;
			l = file->name.Length();
			for ( int j = 0; pureExclusions[j].func != NULL; j++ ) {
				if ( pureExclusions[j].func( pureExclusions[j], l, file->name ) ) {
					abrt = false;
					break;
				}
			}
			if ( abrt ) {
				common->DPrintf( "pak '%s' candidate for pure: '%s'\n", pak->pakFilename.c_str(), file->name.c_str() );
				break;
			}
			file = file->next;
			i++;
		}
		if ( abrt ) {
			break;
		}
	}
	if ( i == pak->numfiles ) {
		pak->pureStatus = PURE_NEVER;
		return PURE_NEVER;
	}

	// check pak name for PURE_ALWAYS
	pak->pakFilename.ExtractFileName( name );
	if ( !name.IcmpPrefixPath( "pak" ) ) {
		pak->pureStatus = PURE_ALWAYS;
		return PURE_ALWAYS;
	}

	pak->pureStatus = PURE_NEUTRAL;
	return PURE_NEUTRAL;
}

/*
===========
idFileSystemLocal::ReadFileFromZip
===========
*/
idFile_InZip * idFileSystemLocal::ReadFileFromZip( pack_t *pak, fileInPack_t *pakFile, const char *relativePath ) {
	unz_s *			zfi;
	FILE *			fp;
	idFile_InZip *file = new idFile_InZip();

	// open a new file on the pakfile
	file->z = unzReOpen( pak->pakFilename, pak->handle );
	if ( file->z == NULL ) {
		common->FatalError( "Couldn't reopen %s", pak->pakFilename.c_str() );
	}
	file->name = relativePath;
	file->fullPath = pak->pakFilename + "/" + relativePath;
	zfi = (unz_s *)file->z;
	// in case the file was new
	fp = zfi->file;
	// set the file position in the zip file (also sets the current file info)
	unzSetCurrentFileInfoPosition( pak->handle, pakFile->pos );
	// copy the file info into the unzip structure
	memcpy( zfi, pak->handle, sizeof(unz_s) );
	// we copy this back into the structure
	zfi->file = fp;
	// open the file in the zip
	unzOpenCurrentFile( file->z );
	file->zipFilePos = pakFile->pos;
	file->fileSize = zfi->cur_file_info.uncompressed_size;
	return file;
}

/*
===========
idFileSystemLocal::OpenFileReadFlags

Finds the file in the search path, following search flag recommendations
Returns filesize and an open FILE pointer.
Used for streaming data out of either a
separate file or a ZIP file.
===========
*/
idFile *idFileSystemLocal::OpenFileReadFlags( const char *relativePath, int searchFlags, pack_t **foundInPak, bool allowCopyFiles, const char* gamedir ) {
	searchpath_t *	search;
	idStr			netpath;
	pack_t *		pak;
	fileInPack_t *	pakFile;
	directory_t *	dir;
	long			hash;
	FILE *			fp;
	
	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}

	if ( !relativePath ) {
		common->FatalError( "idFileSystemLocal::OpenFileRead: NULL 'relativePath' parameter passed\n" );
	}

	if ( foundInPak ) {
		*foundInPak = NULL;
	}

	// qpaths are not supposed to have a leading slash
	if ( relativePath[0] == '/' || relativePath[0] == '\\' ) {
		relativePath++;
	}

	// make absolutely sure that it can't back up the path.
	// The searchpaths do guarantee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo" 
	if ( strstr( relativePath, ".." ) || strstr( relativePath, "::" ) ) {
		return NULL;
	}
	
	// edge case
	if ( relativePath[0] == '\0' ) {
		return NULL;
	}

	// make sure the doomkey file is only readable by game at initialization
	// any other time the key should only be accessed in memory using the provided functions
	if( common->IsInitialized() && ( idStr::Icmp( relativePath, CDKEY_FILE ) == 0 || idStr::Icmp( relativePath, XPKEY_FILE ) == 0 ) ) {
		return NULL;
	}

	//
	// search through the path, one element at a time
	//

	hash = HashFileName( relativePath );

	for ( search = searchPaths; search; search = search->next ) {
		if ( search->dir && ( searchFlags & FSFLAG_SEARCH_DIRS ) ) {
			// check a file in the directory tree

			// if we are running restricted, the only files we
			// will allow to come from the directory are .cfg files
			if ( fs_restrict.GetBool() || serverPaks.Num() ) {
				if ( !FileAllowedFromDir( relativePath ) ) {
					continue;
				}
			}

			dir = search->dir;

			if(gamedir && strlen(gamedir)) {
				if(dir->gamedir != gamedir) {
					continue;
				}
			}
			
			netpath = BuildOSPath( dir->path, dir->gamedir, relativePath );
			fp = OpenOSFileCorrectName( netpath, "rb" );
			if ( !fp ) {
				continue;
			}

			idFile_Permanent *file = new idFile_Permanent();
			file->o = fp;
			file->name = relativePath;
			file->fullPath = netpath;
			file->mode = ( 1 << FS_READ );
			file->fileSize = DirectFileLength( file->o );
			if ( fs_debug.GetInteger() ) {
				common->Printf( "idFileSystem::OpenFileRead: %s (found in '%s/%s')\n", relativePath, dir->path.c_str(), dir->gamedir.c_str() );
			}

			if ( !loadedFileFromDir && !FileAllowedFromDir( relativePath ) ) {
				if ( restartChecksums.Num() ) {
					common->FatalError( "'%s' loaded from directory: Failed to restart with pure mode restrictions for server connect", relativePath );
				}
				common->DPrintf( "filesystem: switching to pure mode will require a restart. '%s' loaded from directory.\n", relativePath );
				loadedFileFromDir = true;
			}

			// if fs_copyfiles is set
			if ( allowCopyFiles && fs_copyfiles.GetInteger() ) {

				idStr copypath;
				idStr name;
				copypath = BuildOSPath( fs_savepath.GetString(), dir->gamedir, relativePath );
				netpath.ExtractFileName( name );
				copypath.StripFilename( );
				copypath += PATHSEPERATOR_STR;
				copypath += name;

				bool isFromCDPath = !dir->path.Cmp( fs_cdpath.GetString() );
				bool isFromSavePath = !dir->path.Cmp( fs_savepath.GetString() );
				bool isFromBasePath = !dir->path.Cmp( fs_basepath.GetString() );

				switch ( fs_copyfiles.GetInteger() ) {
					case 1:
						// copy from cd path only
						if ( isFromCDPath ) {
							CopyFile( netpath, copypath );
						}
						break;
					case 2:
						// from cd path + timestamps
						if ( isFromCDPath ) {
							CopyFile( netpath, copypath );
						} else if ( isFromSavePath || isFromBasePath ) {
							idStr sourcepath;
							sourcepath = BuildOSPath( fs_cdpath.GetString(), dir->gamedir, relativePath );
							FILE *f1 = OpenOSFile( sourcepath, "r" );
							if ( f1 ) {
								ID_TIME_T t1 = Sys_FileTimeStamp( f1 );
								fclose( f1 );
								FILE *f2 = OpenOSFile( copypath, "r" );
								if ( f2 ) {
									ID_TIME_T t2 = Sys_FileTimeStamp( f2 );
									fclose( f2 );
									if ( t1 > t2 ) {
										CopyFile( sourcepath, copypath );
									}
								}
							}
						}
						break;
					case 3:
						if ( isFromCDPath || isFromBasePath ) {
							CopyFile( netpath, copypath );
						}
						break;
					case 4:
						if ( isFromCDPath && !isFromBasePath ) {
							CopyFile( netpath, copypath );
						}
						break;
				}
			}

			return file;
		} else if ( search->pack && ( searchFlags & FSFLAG_SEARCH_PAKS ) ) {

			if ( !search->pack->hashTable[hash] ) {
				continue;
			}

			// disregard if it doesn't match one of the allowed pure pak files
			if ( serverPaks.Num() ) {
				GetPackStatus( search->pack );
				if ( search->pack->pureStatus != PURE_NEVER && !serverPaks.Find( search->pack ) ) {
					continue; // not on the pure server pak list
				}
			}

			// look through all the pak file elements
			pak = search->pack;

			if ( searchFlags & FSFLAG_BINARY_ONLY ) {
				// make sure this pak is tagged as a binary file
				if ( pak->binary == BINARY_UNKNOWN ) {
					int				confHash;
					fileInPack_t	*pakFile;
					confHash = HashFileName( BINARY_CONFIG );
					pak->binary = BINARY_NO;
					for ( pakFile = search->pack->hashTable[confHash]; pakFile; pakFile = pakFile->next ) {
						if ( !FilenameCompare( pakFile->name, BINARY_CONFIG ) ) {
							pak->binary = BINARY_YES;
							break;
						}
					}
				}
				if ( pak->binary == BINARY_NO ) {
					continue; // not a binary pak, skip
				}
			}

			for ( pakFile = pak->hashTable[hash]; pakFile; pakFile = pakFile->next ) {
				// case and separator insensitive comparisons
				if ( !FilenameCompare( pakFile->name, relativePath ) ) {
					idFile_InZip *file = ReadFileFromZip( pak, pakFile, relativePath );

					if ( foundInPak ) {
						*foundInPak = pak;
					}

					if ( !pak->referenced && !( searchFlags & FSFLAG_PURE_NOREF ) ) {
						// mark this pak referenced
						if ( fs_debug.GetInteger( ) ) {
							common->Printf( "idFileSystem::OpenFileRead: %s -> adding %s to referenced paks\n", relativePath, pak->pakFilename.c_str() );
						}
						pak->referenced = true;
					}

					if ( fs_debug.GetInteger( ) ) {
						common->Printf( "idFileSystem::OpenFileRead: %s (found in '%s')\n", relativePath, pak->pakFilename.c_str() );
					}
					return file;
				}
			}
		}
	}

	if ( searchFlags & FSFLAG_SEARCH_ADDONS ) {
		for ( search = addonPaks; search; search = search->next ) {
			assert( search->pack );
			fileInPack_t	*pakFile;
			pak = search->pack;
			for ( pakFile = pak->hashTable[hash]; pakFile; pakFile = pakFile->next ) {
				if ( !FilenameCompare( pakFile->name, relativePath ) ) {
					idFile_InZip *file = ReadFileFromZip( pak, pakFile, relativePath );
					if ( foundInPak ) {
						*foundInPak = pak;
					}
					// we don't toggle pure on paks found in addons - they can't be used without a reloadEngine anyway
					if ( fs_debug.GetInteger( ) ) {
						common->Printf( "idFileSystem::OpenFileRead: %s (found in addon pk4 '%s')\n", relativePath, search->pack->pakFilename.c_str() );
					}
					return file;
				}
			}
		}
	}
	
	if ( fs_debug.GetInteger( ) ) {
		common->Printf( "Can't find %s\n", relativePath );
	}
	
	return NULL;
}

/*
===========
idFileSystemLocal::OpenFileRead
===========
*/
idFile *idFileSystemLocal::OpenFileRead( const char *relativePath, bool allowCopyFiles, const char* gamedir ) {
	return OpenFileReadFlags( relativePath, FSFLAG_SEARCH_DIRS | FSFLAG_SEARCH_PAKS, NULL, allowCopyFiles, gamedir );
}

/*
===========
idFileSystemLocal::OpenFileWrite
===========
*/
idFile *idFileSystemLocal::OpenFileWrite( const char *relativePath, const char *basePath ) {
	const char *path;
	idStr OSpath;
	idFile_Permanent *f;

	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}

	path = cvarSystem->GetCVarString( basePath );
	if ( !path[0] ) {
		path = fs_savepath.GetString();
	}

	OSpath = BuildOSPath( path, gameFolder, relativePath );

	if ( fs_debug.GetInteger() ) {
		common->Printf( "idFileSystem::OpenFileWrite: %s\n", OSpath.c_str() );
	}

	// if the dir we are writing to is in our current list, it will be outdated
	// so just flush everything
	ClearDirCache();

	common->DPrintf( "writing to: %s\n", OSpath.c_str() );
	CreateOSPath( OSpath );

	f = new idFile_Permanent();
	f->o = OpenOSFile( OSpath, "wb" );
	if ( !f->o ) {
		delete f;
		return NULL;
	}
	f->name = relativePath;
	f->fullPath = OSpath;
	f->mode = ( 1 << FS_WRITE );
	f->handleSync = false;
	f->fileSize = 0;

	return f;
}

/*
===========
idFileSystemLocal::OpenExplicitFileRead
===========
*/
idFile *idFileSystemLocal::OpenExplicitFileRead( const char *OSPath ) {
	idFile_Permanent *f;

	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}

	if ( fs_debug.GetInteger() ) {
		common->Printf( "idFileSystem::OpenExplicitFileRead: %s\n", OSPath );
	}

	common->DPrintf( "idFileSystem::OpenExplicitFileRead - reading from: %s\n", OSPath );

	f = new idFile_Permanent();
	f->o = OpenOSFile( OSPath, "rb" );
	if ( !f->o ) {
		delete f;
		return NULL;
	}
	f->name = OSPath;
	f->fullPath = OSPath;
	f->mode = ( 1 << FS_READ );
	f->handleSync = false;
	f->fileSize = DirectFileLength( f->o );

	return f;
}

/*
===========
idFileSystemLocal::OpenExplicitFileWrite
===========
*/
idFile *idFileSystemLocal::OpenExplicitFileWrite( const char *OSPath ) {
	idFile_Permanent *f;

	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}

	if ( fs_debug.GetInteger() ) {
		common->Printf( "idFileSystem::OpenExplicitFileWrite: %s\n", OSPath );
	}

	common->DPrintf( "writing to: %s\n", OSPath );
	CreateOSPath( OSPath );

	f = new idFile_Permanent();
	f->o = OpenOSFile( OSPath, "wb" );
	if ( !f->o ) {
		delete f;
		return NULL;
	}
	f->name = OSPath;
	f->fullPath = OSPath;
	f->mode = ( 1 << FS_WRITE );
	f->handleSync = false;
	f->fileSize = 0;

	return f;
}

/*
===========
idFileSystemLocal::OpenFileAppend
===========
*/
idFile *idFileSystemLocal::OpenFileAppend( const char *relativePath, bool sync, const char *basePath ) {
	const char *path;
	idStr OSpath;
	idFile_Permanent *f;

	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}

	path = cvarSystem->GetCVarString( basePath );
	if ( !path[0] ) {
		path = fs_savepath.GetString();
	}

	OSpath = BuildOSPath( path, gameFolder, relativePath );
	CreateOSPath( OSpath );

	if ( fs_debug.GetInteger() ) {
		common->Printf( "idFileSystem::OpenFileAppend: %s\n", OSpath.c_str() );
	}

	f = new idFile_Permanent();
	f->o = OpenOSFile( OSpath, "ab" );
	if ( !f->o ) {
		delete f;
		return NULL;
	}
	f->name = relativePath;
	f->fullPath = OSpath;
	f->mode = ( 1 << FS_WRITE ) + ( 1 << FS_APPEND );
	f->handleSync = sync;
	f->fileSize = DirectFileLength( f->o );

	return f;
}

/*
================
idFileSystemLocal::OpenFileByMode
================
*/
idFile *idFileSystemLocal::OpenFileByMode( const char *relativePath, fsMode_t mode ) {
	if ( mode == FS_READ ) {
		return OpenFileRead( relativePath );
	}
	if ( mode == FS_WRITE ) {
		return OpenFileWrite( relativePath );
	}
	if ( mode == FS_APPEND ) {
		return OpenFileAppend( relativePath, true );
	}
	common->FatalError( "idFileSystemLocal::OpenFileByMode: bad mode" );
	return NULL;
}

/*
==============
idFileSystemLocal::CloseFile
==============
*/
void idFileSystemLocal::CloseFile( idFile *f ) {
	if ( !searchPaths ) {
		common->FatalError( "Filesystem call made without initialization\n" );
	}
	delete f;
}


/*
=================================================================================

back ground loading

=================================================================================
*/

/*
=================
idFileSystemLocal::CurlWriteFunction
=================
*/
size_t idFileSystemLocal::CurlWriteFunction( void *ptr, size_t size, size_t nmemb, void *stream ) {
	backgroundDownload_t *bgl = (backgroundDownload_t *)stream;
	if ( !bgl->f ) {
		return size * nmemb;
	}
	#ifdef _WIN32
		return _write( static_cast<idFile_Permanent*>(bgl->f)->GetFilePtr()->_file, ptr, size * nmemb );
	#else
		return fwrite( ptr, size, nmemb, static_cast<idFile_Permanent*>(bgl->f)->GetFilePtr() );
	#endif
}

/*
=================
idFileSystemLocal::CurlProgressFunction
=================
*/
int idFileSystemLocal::CurlProgressFunction( void *clientp, double dltotal, double dlnow, double ultotal, double ulnow ) {
	backgroundDownload_t *bgl = (backgroundDownload_t *)clientp;
	if ( bgl->url.status == DL_ABORTING ) {
		return 1;
	}
	bgl->url.dltotal = dltotal;
	bgl->url.dlnow = dlnow;
	return 0;
}

/*
===================
BackgroundDownload

Reads part of a file from a background thread.
===================
*/
dword BackgroundDownloadThread( void *parms ) {
	while( 1 ) {
		Sys_EnterCriticalSection();
		backgroundDownload_t	*bgl = fileSystemLocal.backgroundDownloads;
		if ( !bgl ) {
			Sys_LeaveCriticalSection();
			Sys_WaitForEvent();
			continue;
		}
		// remove this from the list
		fileSystemLocal.backgroundDownloads = bgl->next;
		Sys_LeaveCriticalSection();

		bgl->next = NULL;

		if ( bgl->opcode == DLTYPE_FILE ) {
			// use the low level read function, because fread may allocate memory
			#if defined(WIN32)
				_read( static_cast<idFile_Permanent*>(bgl->f)->GetFilePtr()->_file, bgl->file.buffer, bgl->file.length );
			#else
				fread(  bgl->file.buffer, bgl->file.length, 1, static_cast<idFile_Permanent*>(bgl->f)->GetFilePtr() );
			#endif
			bgl->completed = true;
		} else {
#if ID_ENABLE_CURL
			// DLTYPE_URL
			// use a local buffer for curl error since the size define is local
			char error_buf[ CURL_ERROR_SIZE ];
			bgl->url.dlerror[ 0 ] = '\0';
			CURL *session = curl_easy_init();
			CURLcode ret;
			if ( !session ) {
				bgl->url.dlstatus = CURLE_FAILED_INIT;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			ret = curl_easy_setopt( session, CURLOPT_ERRORBUFFER, error_buf );
			if ( ret ) {
				bgl->url.dlstatus = ret;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			ret = curl_easy_setopt( session, CURLOPT_URL, bgl->url.url.c_str() );
			if ( ret ) {
				bgl->url.dlstatus = ret;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			ret = curl_easy_setopt( session, CURLOPT_FAILONERROR, 1 );
			if ( ret ) {
				bgl->url.dlstatus = ret;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			ret = curl_easy_setopt( session, CURLOPT_WRITEFUNCTION, idFileSystemLocal::CurlWriteFunction );
			if ( ret ) {
				bgl->url.dlstatus = ret;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			ret = curl_easy_setopt( session, CURLOPT_WRITEDATA, bgl );
			if ( ret ) {
				bgl->url.dlstatus = ret;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			ret = curl_easy_setopt( session, CURLOPT_NOPROGRESS, 0 );
			if ( ret ) {
				bgl->url.dlstatus = ret;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			ret = curl_easy_setopt( session, CURLOPT_PROGRESSFUNCTION, idFileSystemLocal::CurlProgressFunction );
			if ( ret ) {
				bgl->url.dlstatus = ret;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			ret = curl_easy_setopt( session, CURLOPT_PROGRESSDATA, bgl );
			if ( ret ) {
				bgl->url.dlstatus = ret;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			bgl->url.dlnow = 0;
			bgl->url.dltotal = 0;
			bgl->url.status = DL_INPROGRESS;
			ret = curl_easy_perform( session );
			if ( ret ) {
				Sys_Printf( "curl_easy_perform failed: %s\n", error_buf );
				idStr::Copynz( bgl->url.dlerror, error_buf, MAX_STRING_CHARS );
				bgl->url.dlstatus = ret;
				bgl->url.status = DL_FAILED;
				bgl->completed = true;
				continue;
			}
			bgl->url.status = DL_DONE;
			bgl->completed = true;
#else
			bgl->url.status = DL_FAILED;
			bgl->completed = true;
#endif
		}
	}
	return 0;
}

/*
=================
idFileSystemLocal::StartBackgroundReadThread
=================
*/
void idFileSystemLocal::StartBackgroundDownloadThread() {
	if ( !backgroundThread.threadHandle ) {
		Sys_CreateThread( (xthread_t)BackgroundDownloadThread, NULL, THREAD_NORMAL, backgroundThread, "backgroundDownload", g_threads, &g_thread_count );
		if ( !backgroundThread.threadHandle ) {
			common->Warning( "idFileSystemLocal::StartBackgroundDownloadThread: failed" );
		}
	} else {
		common->Printf( "background thread already running\n" );
	}
}

/*
=================
idFileSystemLocal::BackgroundDownload
=================
*/
void idFileSystemLocal::BackgroundDownload( backgroundDownload_t *bgl ) {
	if ( bgl->opcode == DLTYPE_FILE ) {
		if ( dynamic_cast<idFile_Permanent *>(bgl->f) ) {
			// add the bgl to the background download list
			Sys_EnterCriticalSection();
			bgl->next = backgroundDownloads;
			backgroundDownloads = bgl;
			Sys_TriggerEvent();
			Sys_LeaveCriticalSection();
		} else {
			// read zipped file directly
			bgl->f->Seek( bgl->file.position, FS_SEEK_SET );
			bgl->f->Read( bgl->file.buffer, bgl->file.length );
			bgl->completed = true;
		}
	} else {
		Sys_EnterCriticalSection();
		bgl->next = backgroundDownloads;
		backgroundDownloads = bgl;
		Sys_TriggerEvent();
		Sys_LeaveCriticalSection();
	}
}

/*
=================
idFileSystemLocal::PerformingCopyFiles
=================
*/
bool idFileSystemLocal::PerformingCopyFiles( void ) const {
	return fs_copyfiles.GetInteger() > 0;
}

/*
=================
idFileSystemLocal::FindPakForFileChecksum
=================
*/
pack_t *idFileSystemLocal::FindPakForFileChecksum( const char *relativePath, int findChecksum, bool bReference ) {
	searchpath_t	*search;
	pack_t			*pak;
	fileInPack_t	*pakFile;
	int				hash;
	assert( !serverPaks.Num() );
	hash = HashFileName( relativePath );
	for ( search = searchPaths; search; search = search->next ) {
		if ( search->pack && search->pack->hashTable[ hash ] ) {
			pak = search->pack;
			for ( pakFile = pak->hashTable[ hash ]; pakFile; pakFile = pakFile->next ) {
				if ( !FilenameCompare( pakFile->name, relativePath ) ) {
					idFile_InZip *file = ReadFileFromZip( pak, pakFile, relativePath );
					if ( findChecksum == GetFileChecksum( file ) ) {
						if ( fs_debug.GetBool() ) {
							common->Printf( "found '%s' with checksum 0x%x in pak '%s'\n", relativePath, findChecksum, pak->pakFilename.c_str() );
						}
						if ( bReference ) {
							pak->referenced = true;
							// FIXME: use dependencies for pak references
						}
						CloseFile( file );
						return pak;
					} else if ( fs_debug.GetBool() ) {
						common->Printf( "'%s' in pak '%s' has != checksum %x\n", relativePath, pak->pakFilename.c_str(), GetFileChecksum( file ) );
					}
					CloseFile( file );
				}
			}
		}
	}
	if ( fs_debug.GetBool() ) {
		common->Printf( "no pak file found for '%s' checksumed %x\n", relativePath, findChecksum );
	}
	return NULL;
}

/*
=================
idFileSystemLocal::GetFileChecksum
=================
*/
int idFileSystemLocal::GetFileChecksum( idFile *file ) {
	int len, ret;
	byte *buf;

	file->Seek( 0, FS_SEEK_END );
	len = file->Tell();
	file->Seek( 0, FS_SEEK_SET );
	buf = (byte *)Mem_Alloc( len );
	if ( file->Read( buf, len ) != len ) {
		common->FatalError( "Short read in idFileSystemLocal::GetFileChecksum()\n" );
	}
	ret = MD4_BlockChecksum( buf, len );
	Mem_Free( buf );
	return ret;
}

/*
=================
idFileSystemLocal::FindDLL
=================
*/
void idFileSystemLocal::FindDLL( const char *name, char _dllPath[ MAX_OSPATH ], bool updateChecksum ) {
	idFile			*dllFile = NULL;
	char			dllName[MAX_OSPATH];
	idStr			dllPath;
	int				dllHash;
	pack_t			*inPak;
	pack_t			*pak;
	fileInPack_t	*pakFile;	

	sys->DLL_GetFileName( name, dllName, MAX_OSPATH );
	dllHash = HashFileName( dllName );

#if ID_FAKE_PURE
	if ( 1 ) {
#else
	if ( !serverPaks.Num() ) {
#endif
		// from executable directory first - this is handy for developement
		dllPath = Sys_EXEPath( );
		dllPath.StripFilename( );
		dllPath.AppendPath( dllName );
		dllFile = OpenExplicitFileRead( dllPath );
	}
	if ( !dllFile ) {
		if ( !serverPaks.Num() ) {
			// not running in pure mode, try to extract from a pak file first
			dllFile = OpenFileReadFlags( dllName, FSFLAG_SEARCH_PAKS | FSFLAG_PURE_NOREF | FSFLAG_BINARY_ONLY, &inPak );
			if ( dllFile ) {
				common->Printf( "found DLL in pak file: %s\n", dllFile->GetFullPath() );
				dllPath = RelativePathToOSPath( dllName, "fs_savepath" );
				CopyFile( dllFile, dllPath );
				CloseFile( dllFile );
				dllFile = OpenFileReadFlags( dllName, FSFLAG_SEARCH_DIRS );
				if ( !dllFile ) {
					common->Error( "DLL extraction to fs_savepath failed\n" );
				} else if ( updateChecksum ) {
					gameDLLChecksum = GetFileChecksum( dllFile );
					gamePakChecksum = inPak->checksum;
					updateChecksum = false;	// don't try again below
				}
			} else {
				// didn't find a source in a pak file, try in the directory
				dllFile = OpenFileReadFlags( dllName, FSFLAG_SEARCH_DIRS );
				if ( dllFile ) {
					if ( updateChecksum ) {
						gameDLLChecksum = GetFileChecksum( dllFile );
						// see if we can mark a pak file
						pak = FindPakForFileChecksum( dllName, gameDLLChecksum, false );
						pak ? gamePakChecksum = pak->checksum : gamePakChecksum = 0;
						updateChecksum = false;
					}
				}
			}
		} else {
			// we are in pure mode. this path to be reached only for game DLL situations
			// with a code pak checksum given by server
			assert( gamePakChecksum );
			assert( updateChecksum );
			pak = GetPackForChecksum( gamePakChecksum );
			if ( !pak ) {
				// not supposed to happen, bug in pure code?
				common->Warning( "FindDLL in pure mode: game pak not found ( 0x%x )\n", gamePakChecksum );
			} else {
				// extract and copy
				for ( pakFile = pak->hashTable[dllHash]; pakFile; pakFile = pakFile->next ) {
					if ( !FilenameCompare( pakFile->name, dllName ) ) {
						dllFile = ReadFileFromZip( pak, pakFile, dllName );
						common->Printf( "found DLL in game pak file: %s\n", pak->pakFilename.c_str() );
						dllPath = RelativePathToOSPath( dllName, "fs_savepath" );
						CopyFile( dllFile, dllPath );
						CloseFile( dllFile );
						dllFile = OpenFileReadFlags( dllName, FSFLAG_SEARCH_DIRS );
						if ( !dllFile ) {
							common->Error( "DLL extraction to fs_savepath failed\n" );
						} else {
							gameDLLChecksum = GetFileChecksum( dllFile );
							updateChecksum = false;	// don't try again below
						}						
					}
				}
			}
		}
	}
	if ( updateChecksum ) {
		if ( dllFile ) {
			gameDLLChecksum = GetFileChecksum( dllFile );
		} else {
			gameDLLChecksum = 0;
		}
		gamePakChecksum = 0;
	}
	if ( dllFile ) {
		dllPath = dllFile->GetFullPath( );
		CloseFile( dllFile );
		dllFile = NULL;
	} else {
		dllPath = "";
	}
	idStr::snPrintf( _dllPath, MAX_OSPATH, dllPath.c_str() );
}

/*
================
idFileSystemLocal::ClearDirCache
================
*/
void idFileSystemLocal::ClearDirCache( void ) {
	int i;

	dir_cache_index = 0;
	dir_cache_count = 0;
	for( i = 0; i < MAX_CACHED_DIRS; i++ ) {
		dir_cache[ i ].Clear();
	}
}

/*
===============
idFileSystemLocal::HasD3XP
===============
*/
bool idFileSystemLocal::HasD3XP( void ) {
	int			i;
	idStrList	dirs, pk4s;
	idStr		gamepath;

	if ( d3xp == -1 ) {
		return false;
	} else if ( d3xp == 1 ) {
		return true;
	}
	
#if 0
	// check for a d3xp directory with a pk4 file
	// copied over from ListMods - only looks in basepath
	ListOSFiles( fs_basepath.GetString(), "/", dirs );
	for ( i = 0; i < dirs.Num(); i++ ) {
		if ( dirs[i].Icmp( "d3xp" ) == 0 ) {
			gamepath = BuildOSPath( fs_basepath.GetString(), dirs[ i ], "" );
			ListOSFiles( gamepath, ".pk4", pk4s );
			if ( pk4s.Num() ) {
				d3xp = 1;
				return true;
			}
		}
	}
#elif ID_ALLOW_D3XP
	// check for d3xp's d3xp/pak000.pk4 in any search path
	// checking wether the pak is loaded by checksum wouldn't be enough:
	// we may have a different fs_game right now but still need to reply that it's installed
	const char	*search[4];
	idFile	  	*pakfile;
	search[0] = fs_savepath.GetString();
	search[1] = fs_devpath.GetString();
	search[2] = fs_basepath.GetString();
	search[3] = fs_cdpath.GetString();
	for ( i = 0; i < 4; i++ ) {
		pakfile = OpenExplicitFileRead( BuildOSPath( search[ i ], "d3xp", "pak000.pk4" ) );
		if ( pakfile ) {
			CloseFile( pakfile );
			d3xp = 1;
			return true;
		}
	}
#endif

#if ID_ALLOW_D3XP
	// if we didn't find a pk4 file then the user might have unpacked so look for default.cfg file
	// that's the old way mostly used during developement. don't think it hurts to leave it there
	ListOSFiles( fs_basepath.GetString(), "/", dirs );
	for ( i = 0; i < dirs.Num(); i++ ) {
		if ( dirs[i].Icmp( "d3xp" ) == 0 ) {
			
			gamepath = BuildOSPath( fs_savepath.GetString(), dirs[ i ], "default.cfg" );
			idFile* cfg = OpenExplicitFileRead(gamepath);
			if(cfg) {
				CloseFile(cfg);
				d3xp = 1;
				return true;
			}
		}
	}
#endif
	d3xp = -1;
	return false;
}

/*
===============
idFileSystemLocal::RunningD3XP
===============
*/
bool idFileSystemLocal::RunningD3XP( void ) {
	// TODO: mark the checksum of the gold XP and check for it being referenced ( for double mod support )
	// a simple fs_game check should be enough for now..
	if ( !idStr::Icmp( fs_game.GetString(), "d3xp" ) ||
		 !idStr::Icmp( fs_game_base.GetString(), "d3xp" ) ) {
		return true;
	}
	return false;
}

/*
===============
idFileSystemLocal::MakeTemporaryFile
===============
*/
idFile * idFileSystemLocal::MakeTemporaryFile( void ) {
	FILE *f = tmpfile();
	if ( !f ) {
		common->Warning( "idFileSystem::MakeTemporaryFile failed: %s", strerror( errno ) );
		return NULL;
	}
	idFile_Permanent *file = new idFile_Permanent();
	file->o = f;
	file->name = "<tempfile>";
	file->fullPath = "<tempfile>";
	file->mode = ( 1 << FS_READ ) + ( 1 << FS_WRITE );
	file->fileSize = 0;
	return file;
}

/*
===============
idFileSystemLocal::FindFile
===============
*/
 findFile_t idFileSystemLocal::FindFile( const char *path, bool scheduleAddons ) {
	pack_t *pak;
	idFile *f = OpenFileReadFlags( path, FSFLAG_SEARCH_DIRS | FSFLAG_SEARCH_PAKS | FSFLAG_SEARCH_ADDONS, &pak );
	if ( !f ) {
		return FIND_NO;
	}
	if ( !pak ) {
		// found in FS, not even in paks
		return FIND_YES;
	}
	// marking addons for inclusion on reload - may need to do that even when already in the search path
	if ( scheduleAddons && pak->addon && addonChecksums.FindIndex( pak->checksum ) < 0 ) {
		addonChecksums.Append( pak->checksum );			
	}
	// an addon that's not on search list yet? that will require a restart
	if ( pak->addon && !pak->addon_search ) {
		delete f;
		return FIND_ADDON;
	}
	delete f;
	return FIND_YES;
}

/*
===============
idFileSystemLocal::GetNumMaps
account for actual decls and for addon maps
===============
*/
int idFileSystemLocal::GetNumMaps() {
	int				i;
	searchpath_t	*search = NULL;
	int				ret = declManager->GetNumDecls( DECL_MAPDEF );
	
	// add to this all addon decls - coming from all addon packs ( searched or not )
	for ( i = 0; i < 2; i++ ) {
		if ( i == 0 ) {
			search = searchPaths;
		} else if ( i == 1 ) {
			search = addonPaks;
		}
		for ( ; search ; search = search->next ) {
			if ( !search->pack || !search->pack->addon || !search->pack->addon_info ) {
				continue;
			}
			ret += search->pack->addon_info->mapDecls.Num();
		}
	}
	return ret;
}

/*
===============
idFileSystemLocal::GetMapDecl
retrieve the decl dictionary, add a 'path' value
===============
*/
const idDict * idFileSystemLocal::GetMapDecl( int idecl ) {
	int 					i;
	const idDecl			*mapDecl;
	const idDeclEntityDef	*mapDef;
	int						numdecls = declManager->GetNumDecls( DECL_MAPDEF );
	searchpath_t			*search = NULL;
	
	if ( idecl < numdecls ) {
		mapDecl = declManager->DeclByIndex( DECL_MAPDEF, idecl );
		mapDef = static_cast<const idDeclEntityDef *>( mapDecl );
		if ( !mapDef ) {
			common->Error( "idFileSystemLocal::GetMapDecl %d: not found\n", idecl );
		}
		mapDict = mapDef->dict;
		mapDict.Set( "path", mapDef->GetName() );
		return &mapDict;
	}
	idecl -= numdecls;
	for ( i = 0; i < 2; i++ ) {
		if ( i == 0 ) {
			search = searchPaths;
		} else if ( i == 1 ) {
			search = addonPaks;
		}
		for ( ; search ; search = search->next ) {
			if ( !search->pack || !search->pack->addon || !search->pack->addon_info ) {
				continue;
			}
			// each addon may have a bunch of map decls
			if ( idecl < search->pack->addon_info->mapDecls.Num() ) {
				mapDict = *search->pack->addon_info->mapDecls[ idecl ];
				return &mapDict;
			}
			idecl -= search->pack->addon_info->mapDecls.Num();
			assert( idecl >= 0 );
		}
	}
	return NULL;
}

/*
===============
idFileSystemLocal::FindMapScreenshot
===============
*/
void idFileSystemLocal::FindMapScreenshot( const char *path, char *buf, int len ) {
	idFile	*file;
	idStr	mapname = path;

	mapname.StripPath();
	mapname.StripFileExtension();
	
	idStr::snPrintf( buf, len, "guis/assets/splash/%s.tga", mapname.c_str() );
	if ( ReadFile( buf, NULL, NULL ) == -1 ) {
		// try to extract from an addon
		file = OpenFileReadFlags( buf, FSFLAG_SEARCH_ADDONS );
		if ( file ) {
			// save it out to an addon splash directory
			int dlen = file->Length();
			char *data = new char[ dlen ];
			file->Read( data, dlen );
			CloseFile( file );
			idStr::snPrintf( buf, len, "guis/assets/splash/addon/%s.tga", mapname.c_str() );
			WriteFile( buf, data, dlen );
			delete[] data;
		} else {
			idStr::Copynz( buf, "guis/assets/splash/pdtempa", len );
		}
	}
}

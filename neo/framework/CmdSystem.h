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

#ifndef __CMDSYSTEM_H__
#define __CMDSYSTEM_H__

/*
===============================================================================

	Console command execution and command text buffering.

	Any number of commands can be added in a frame from several different
	sources. Most commands come from either key bindings or console line input,
	but entire text files can be execed.

	Command execution takes a null terminated string, breaks it into tokens,
	then searches for a command or variable that matches the first token.

===============================================================================
*/

// command flags
typedef enum {
	CMD_FL_ALL				= -1,
	CMD_FL_CHEAT			= BIT(0),	// command is considered a cheat
	CMD_FL_SYSTEM			= BIT(1),	// system command
	CMD_FL_RENDERER			= BIT(2),	// renderer command
	CMD_FL_SOUND			= BIT(3),	// sound command
	CMD_FL_GAME				= BIT(4),	// game command
	CMD_FL_TOOL				= BIT(5)	// tool command
} cmdFlags_t;

// parameters for command buffer stuffing
typedef enum {
	CMD_EXEC_NOW,						// don't return until completed
	CMD_EXEC_INSERT,					// insert at current position, but don't run yet
	CMD_EXEC_APPEND						// add to end of the command buffer (normal case)
} cmdExecution_t;

// command function
typedef void (*cmdFunction_t)( const idCmdArgs &args );

// argument completion function
typedef void (*argCompletion_t)( const idCmdArgs &args, void(*callback)( const char *s ) );


class idCmdSystem {
public:
	virtual				~idCmdSystem( void ) {}

	virtual void		Init( void ) = 0;
	virtual void		Shutdown( void ) = 0;

						// Registers a command and the function to call for it.
	virtual void		AddCommand( const char *cmdName, cmdFunction_t function, int flags, const char *description, argCompletion_t argCompletion = NULL ) = 0;
						// Removes a command.
	virtual void		RemoveCommand( const char *cmdName ) = 0;
						// Remove all commands with one of the flags set.
	virtual void		RemoveFlaggedCommands( int flags ) = 0;

						// Command and argument completion using callback for each valid string.
	virtual void		CommandCompletion( void(*callback)( const char *s ) ) = 0;
	virtual void		ArgCompletion( const char *cmdString, void(*callback)( const char *s ) ) = 0;

						// Adds command text to the command buffer, does not add a final \n
	virtual void		BufferCommandText( cmdExecution_t exec, const char *text ) = 0;
						// Pulls off \n \r or ; terminated lines of text from the command buffer and
						// executes the commands. Stops when the buffer is empty.
						// Normally called once per frame, but may be explicitly invoked.
	virtual void		ExecuteCommandBuffer( void ) = 0;

						// Base for path/file auto-completion.
	virtual void		ArgCompletion_FolderExtension( const idCmdArgs &args, void(*callback)( const char *s ), const char *folder, bool stripFolder, ... ) = 0;
						// Base for decl name auto-completion.
	virtual void		ArgCompletion_DeclName( const idCmdArgs &args, void(*callback)( const char *s ), int type ) = 0;

						// Adds to the command buffer in tokenized form ( CMD_EXEC_NOW or CMD_EXEC_APPEND only )
	virtual void		BufferCommandArgs( cmdExecution_t exec, const idCmdArgs &args ) = 0;

						// Setup a reloadEngine to happen on next command run, and give a command to execute after reload
	virtual void		SetupReloadEngine( const idCmdArgs &args ) = 0;
	virtual bool		PostReloadEngine( void ) = 0;

						// Default argument completion functions.
	static void			ArgCompletion_Boolean( const idCmdArgs &args, void(*callback)( const char *s ) );
	template<int min,int max>
	static void			ArgCompletion_Integer( const idCmdArgs &args, void(*callback)( const char *s ) );
	template<const char **strings>
	static void			ArgCompletion_String( const idCmdArgs &args, void(*callback)( const char *s ) );
	template<int type>
	static void			ArgCompletion_Decl( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			ArgCompletion_FileName( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			ArgCompletion_MapName( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			ArgCompletion_ModelName( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			ArgCompletion_SoundName( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			ArgCompletion_ImageName( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			ArgCompletion_VideoName( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			ArgCompletion_ConfigName( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			ArgCompletion_SaveGame( const idCmdArgs &args, void(*callback)( const char *s ) );
	static void			ArgCompletion_DemoName( const idCmdArgs &args, void(*callback)( const char *s ) );
};

extern idCmdSystem *	cmdSystem;


ID_INLINE void idCmdSystem::ArgCompletion_Boolean( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	callback( va( "%s 0", args.Argv( 0 ) ) );
	callback( va( "%s 1", args.Argv( 0 ) ) );
}

template<int min,int max> ID_STATIC_TEMPLATE ID_INLINE void idCmdSystem::ArgCompletion_Integer( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	for ( int i = min; i <= max; i++ ) {
		callback( va( "%s %d", args.Argv( 0 ), i ) );
	}
}

template<const char **strings> ID_STATIC_TEMPLATE ID_INLINE void idCmdSystem::ArgCompletion_String( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	for ( int i = 0; strings[i]; i++ ) {
		callback( va( "%s %s", args.Argv( 0 ), strings[i] ) );
	}
}

template<int type> ID_STATIC_TEMPLATE ID_INLINE void idCmdSystem::ArgCompletion_Decl( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_DeclName( args, callback, type );
}

ID_INLINE void idCmdSystem::ArgCompletion_FileName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", true, "", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_MapName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "maps/", true, ".map", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_ModelName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "models/", false, ".lwo", ".ase", ".md5mesh", ".ma", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_SoundName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "sound/", false, ".wav", ".ogg", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_ImageName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", false, ".tga", ".dds", ".jpg", ".pcx", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_VideoName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "video/", false, ".roq", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_ConfigName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "/", true, ".cfg", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_SaveGame( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "SaveGames/", true, ".save", NULL );
}

ID_INLINE void idCmdSystem::ArgCompletion_DemoName( const idCmdArgs &args, void(*callback)( const char *s ) ) {
	cmdSystem->ArgCompletion_FolderExtension( args, callback, "demos/", true, ".demo", NULL );
}

#endif /* !__CMDSYSTEM_H__ */

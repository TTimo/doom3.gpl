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

#ifndef __SESSIONLOCAL_H__
#define __SESSIONLOCAL_H__

/*

IsConnectedToServer();
IsGameLoaded();
IsGuiActive();
IsPlayingRenderDemo();

if connected to a server
	if handshaking
	if map loading
	if in game
else if a game loaded
	if in load game menu
	if main menu up
else if playing render demo
else
	if error dialog
	full console

*/

typedef struct {
	usercmd_t	cmd;
	int			consistencyHash;
} logCmd_t;

struct fileTIME_T {
	int				index;
	ID_TIME_T			timeStamp;

					operator int() const { return timeStamp; }
};

typedef struct {
	idDict			serverInfo;
	idDict			syncedCVars;
	idDict			userInfo[MAX_ASYNC_CLIENTS];
	idDict			persistentPlayerInfo[MAX_ASYNC_CLIENTS];
	usercmd_t		mapSpawnUsercmd[MAX_ASYNC_CLIENTS];		// needed for tracking delta angles
} mapSpawnData_t;

typedef enum {
	TD_NO,
	TD_YES,
	TD_YES_THEN_QUIT
} timeDemo_t;

const int USERCMD_PER_DEMO_FRAME	= 2;
const int CONNECT_TRANSMIT_TIME		= 1000;
const int MAX_LOGGED_USERCMDS		= 60*60*60;	// one hour of single player, 15 minutes of four player

class idSessionLocal : public idSession {
public:

						idSessionLocal();
	virtual				~idSessionLocal();

	virtual void		Init();

	virtual void		Shutdown();

	virtual void		Stop();

	virtual void		UpdateScreen( bool outOfSequence = true );

	virtual void		PacifierUpdate();

	virtual void		Frame();

	virtual bool		IsMultiplayer();

	virtual bool		ProcessEvent( const sysEvent_t *event );

	virtual void		StartMenu( bool playIntro = false );
	virtual void		ExitMenu();
	virtual void		GuiFrameEvents();
	virtual void		SetGUI( idUserInterface *gui, HandleGuiCommand_t handle );

	virtual const char *MessageBox( msgBoxType_t type, const char *message, const char *title = NULL, bool wait = false, const char *fire_yes = NULL, const char *fire_no = NULL, bool network = false  );
	virtual void		StopBox( void );
	virtual void		DownloadProgressBox( backgroundDownload_t *bgl, const char *title, int progress_start = 0, int progress_end = 100 );
	virtual void		SetPlayingSoundWorld();

	virtual void		TimeHitch( int msec );

	virtual void		ReadCDKey( void );
	virtual void		WriteCDKey( void );
	virtual const char *GetCDKey( bool xp );
	virtual bool		CheckKey( const char *key, bool netConnect, bool offline_valid[ 2 ] );
	virtual bool		CDKeysAreValid( bool strict );
	virtual void		ClearCDKey( bool valid[ 2 ] );
	virtual void		SetCDKeyGuiVars( void );
	virtual bool		WaitingForGameAuth( void );
	virtual void		CDKeysAuthReply( bool valid, const char *auth_msg );

	virtual int			GetSaveGameVersion( void );

	virtual const char *GetCurrentMapName();

	//=====================================

	int					GetLocalClientNum();

	void				MoveToNewMap( const char *mapName );

	// loads a map and starts a new game on it
	void				StartNewGame( const char *mapName, bool devmap = false );
	void				PlayIntroGui();

	void				LoadSession( const char *name );
	void				SaveSession( const char *name );

	// called by Draw when the scene to scene wipe is still running
	void				DrawWipeModel();
	void				StartWipe( const char *materialName, bool hold = false);
	void				CompleteWipe();
	void				ClearWipe();

	void				ShowLoadingGui();

	void				ScrubSaveGameFileName( idStr &saveFileName ) const;
	idStr				GetAutoSaveName( const char *mapName ) const;

	bool				LoadGame(const char *saveName);
	bool				SaveGame(const char *saveName, bool autosave = false);

	const char			*GetAuthMsg( void );

	//=====================================

	static idCVar		com_showAngles;
	static idCVar		com_showTics;
	static idCVar		com_minTics;
	static idCVar		com_fixedTic;
	static idCVar		com_showDemo;
	static idCVar		com_skipGameDraw;
	static idCVar		com_aviDemoWidth;
	static idCVar		com_aviDemoHeight;
	static idCVar		com_aviDemoSamples;
	static idCVar		com_aviDemoTics;
	static idCVar		com_wipeSeconds;
	static idCVar		com_guid;

	static idCVar		gui_configServerRate;

	int					timeHitch;

	bool				menuActive;
	idSoundWorld *		menuSoundWorld;			// so the game soundWorld can be muted

	bool				insideExecuteMapChange;	// draw loading screen and update
												// screen on prints
	int					bytesNeededForMapLoad;	// 

	// we don't want to redraw the loading screen for every single
	// console print that happens
	int					lastPacifierTime;

	// this is the information required to be set before ExecuteMapChange() is called,
	// which can be saved off at any time with the following commands so it can all be played back
	mapSpawnData_t		mapSpawnData;
	idStr				currentMapName;			// for checking reload on same level
	bool				mapSpawned;				// cleared on Stop()

	int					numClients;				// from serverInfo

	int					logIndex;
	logCmd_t			loggedUsercmds[MAX_LOGGED_USERCMDS];
	int					statIndex;
	logStats_t			loggedStats[MAX_LOGGED_STATS];
	int					lastSaveIndex;
	// each game tic, numClients usercmds will be added, until full

	bool				insideUpdateScreen;	// true while inside ::UpdateScreen()

	bool				loadingSaveGame;	// currently loading map from a SaveGame
	idFile *			savegameFile;		// this is the savegame file to load from
	int					savegameVersion;

	idFile *			cmdDemoFile;		// if non-zero, we are reading commands from a file

	int					latchedTicNumber;	// set to com_ticNumber each frame
	int					lastGameTic;		// while latchedTicNumber > lastGameTic, run game frames
	int					lastDemoTic;
	bool				syncNextGameFrame;


	bool				aviCaptureMode;		// if true, screenshots will be taken and sound captured
	idStr				aviDemoShortName;	// 
	float				aviDemoFrameCount;
	int					aviTicStart;

	timeDemo_t			timeDemo;
	int					timeDemoStartTime;
	int					numDemoFrames;		// for timeDemo and demoShot
	int					demoTimeOffset;
	renderView_t		currentDemoRenderView;
	// the next one will be read when 
	// com_frameTime + demoTimeOffset > currentDemoRenderView.

	// TODO: make this private (after sync networking removal and idnet tweaks)
	idUserInterface *	guiActive;
	HandleGuiCommand_t	guiHandle;

	idUserInterface *	guiInGame;
	idUserInterface *	guiMainMenu;
	idListGUI *			guiMainMenu_MapList;		// easy map list handling
	idUserInterface *	guiRestartMenu;
	idUserInterface *	guiLoading;
	idUserInterface *	guiIntro;
	idUserInterface *	guiGameOver;
	idUserInterface *	guiTest;
	idUserInterface *	guiTakeNotes;
	
	idUserInterface *	guiMsg;
	idUserInterface *	guiMsgRestore;				// store the calling GUI for restore
	idStr				msgFireBack[ 2 ];
	bool				msgRunning;
	int					msgRetIndex;
	bool				msgIgnoreButtons;
	
	bool				waitingOnBind;

	const idMaterial *	whiteMaterial;

	const idMaterial *	wipeMaterial;
	int					wipeStartTic;
	int					wipeStopTic;
	bool				wipeHold;

#if ID_CONSOLE_LOCK
	int					emptyDrawCount;				// watchdog to force the main menu to restart
#endif

	//=====================================
	void				Clear();

	void				DrawCmdGraph();
	void				Draw();

	void				WriteCmdDemo( const char *name, bool save = false);
	void				StartPlayingCmdDemo( const char *demoName);
	void				TimeCmdDemo( const char *demoName);
	void				SaveCmdDemoToFile(idFile *file);
	void				LoadCmdDemoFromFile(idFile *file);
	void				StartRecordingRenderDemo( const char *name );
	void				StopRecordingRenderDemo();
	void				StartPlayingRenderDemo( idStr name );
	void				StopPlayingRenderDemo();
	void				CompressDemoFile( const char *scheme, const char *name );
	void				TimeRenderDemo( const char *name, bool twice = false );
	void				AVIRenderDemo( const char *name );
	void				AVICmdDemo( const char *name );
	void				AVIGame( const char *name );
	void				BeginAVICapture( const char *name );
	void				EndAVICapture();

	void				AdvanceRenderDemo( bool singleFrameOnly );
	void				RunGameTic();
	
	void				FinishCmdLoad();
	void				LoadLoadingGui(const char *mapName);

	void				DemoShot( const char *name );

	void				TestGUI( const char *name );

	int					GetBytesNeededForMapLoad( const char *mapName );
	void				SetBytesNeededForMapLoad( const char *mapName, int bytesNeeded );

	void				ExecuteMapChange( bool noFadeWipe = false );
	void				UnloadMap();

	// return true if we actually waiting on an auth reply
	bool				MaybeWaitOnCDKey( void );

	//------------------
	// Session_menu.cpp

	idStrList			loadGameList;
	idStrList			modsList;

	idUserInterface *	GetActiveMenu();

	void				DispatchCommand( idUserInterface *gui, const char *menuCommand, bool doIngame = true );
	void				MenuEvent( const sysEvent_t *event );
	bool				HandleSaveGameMenuCommand( idCmdArgs &args, int &icmd );
	void				HandleInGameCommands( const char *menuCommand );
	void				HandleMainMenuCommands( const char *menuCommand );
	void				HandleChatMenuCommands( const char *menuCommand );
	void				HandleIntroMenuCommands( const char *menuCommand );
	void				HandleRestartMenuCommands( const char *menuCommand );
	void				HandleMsgCommands( const char *menuCommand );
	void				HandleNoteCommands( const char *menuCommand );
	void				GetSaveGameList( idStrList &fileList, idList<fileTIME_T> &fileTimes );
	void				TakeNotes( const char * p, bool extended = false );
	void				UpdateMPLevelShot( void );

	void				SetSaveGameGuiVars( void );
	void				SetMainMenuGuiVars( void );
	void				SetModsMenuGuiVars( void );
	void				SetMainMenuSkin( void );
	void				SetPbMenuGuiVars( void );
	
private:
	bool				BoxDialogSanityCheck( void );
	void				EmitGameAuth( void );
	
	typedef enum {
		CDKEY_UNKNOWN,	// need to perform checks on the key
		CDKEY_INVALID,	// that key is wrong
		CDKEY_OK,		// valid
		CDKEY_CHECKING, // sent a check request ( gameAuth only )
		CDKEY_NA		// does not apply, xp key when xp is not present
	} cdKeyState_t;

	static const int	CDKEY_BUF_LEN = 17;
	static const int	CDKEY_AUTH_TIMEOUT = 5000;

	char				cdkey[ CDKEY_BUF_LEN ];
	cdKeyState_t		cdkey_state;
	char				xpkey[ CDKEY_BUF_LEN ];
	cdKeyState_t		xpkey_state;
	int					authEmitTimeout;
	bool				authWaitBox;

	idStr				authMsg;
};

extern idSessionLocal	sessLocal;

#endif /* !__SESSIONLOCAL_H__ */

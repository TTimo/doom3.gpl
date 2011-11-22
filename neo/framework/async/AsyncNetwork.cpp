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

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "AsyncNetwork.h"

idAsyncServer		idAsyncNetwork::server;
idAsyncClient		idAsyncNetwork::client;

idCVar				idAsyncNetwork::verbose( "net_verbose", "0", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "1 = verbose output, 2 = even more verbose output", 0, 2, idCmdSystem::ArgCompletion_Integer<0,2> );
idCVar				idAsyncNetwork::allowCheats( "net_allowCheats", "0", CVAR_SYSTEM | CVAR_BOOL | CVAR_NETWORKSYNC, "Allow cheats in network game" );
#ifdef ID_DEDICATED
// dedicated executable can only have a value of 1 for net_serverDedicated
idCVar				idAsyncNetwork::serverDedicated( "net_serverDedicated", "1", CVAR_SERVERINFO | CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT | CVAR_ROM, "" );
#else
idCVar				idAsyncNetwork::serverDedicated( "net_serverDedicated", "0", CVAR_SERVERINFO | CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "1 = text console dedicated server, 2 = graphical dedicated server", 0, 2, idCmdSystem::ArgCompletion_Integer<0,2> );
#endif
idCVar				idAsyncNetwork::serverSnapshotDelay( "net_serverSnapshotDelay", "50", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "delay between snapshots in milliseconds" );
idCVar				idAsyncNetwork::serverMaxClientRate( "net_serverMaxClientRate", "16000", CVAR_SYSTEM | CVAR_INTEGER | CVAR_ARCHIVE | CVAR_NOCHEAT, "maximum rate to a client in bytes/sec" );
idCVar				idAsyncNetwork::clientMaxRate( "net_clientMaxRate", "16000", CVAR_SYSTEM | CVAR_INTEGER | CVAR_ARCHIVE | CVAR_NOCHEAT, "maximum rate requested by client from server in bytes/sec" );
idCVar				idAsyncNetwork::serverMaxUsercmdRelay( "net_serverMaxUsercmdRelay", "5", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "maximum number of usercmds from other clients the server relays to a client", 1, MAX_USERCMD_RELAY, idCmdSystem::ArgCompletion_Integer<1,MAX_USERCMD_RELAY> );
idCVar				idAsyncNetwork::serverZombieTimeout( "net_serverZombieTimeout", "5", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "disconnected client timeout in seconds" );
idCVar				idAsyncNetwork::serverClientTimeout( "net_serverClientTimeout", "40", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "client time out in seconds" );
idCVar				idAsyncNetwork::clientServerTimeout( "net_clientServerTimeout", "40", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "server time out in seconds" );
idCVar				idAsyncNetwork::serverDrawClient( "net_serverDrawClient", "-1", CVAR_SYSTEM | CVAR_INTEGER, "number of client for which to draw view on server" );
idCVar				idAsyncNetwork::serverRemoteConsolePassword( "net_serverRemoteConsolePassword", "", CVAR_SYSTEM | CVAR_NOCHEAT, "remote console password" );
idCVar				idAsyncNetwork::clientPrediction( "net_clientPrediction", "16", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "additional client side prediction in milliseconds" );
idCVar				idAsyncNetwork::clientMaxPrediction( "net_clientMaxPrediction", "1000", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "maximum number of milliseconds a client can predict ahead of server." );
idCVar				idAsyncNetwork::clientUsercmdBackup( "net_clientUsercmdBackup", "5", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "number of usercmds to resend" );
idCVar				idAsyncNetwork::clientRemoteConsoleAddress( "net_clientRemoteConsoleAddress", "localhost", CVAR_SYSTEM | CVAR_NOCHEAT, "remote console address" );
idCVar				idAsyncNetwork::clientRemoteConsolePassword( "net_clientRemoteConsolePassword", "", CVAR_SYSTEM | CVAR_NOCHEAT, "remote console password" );
idCVar				idAsyncNetwork::master0( "net_master0", IDNET_HOST ":" IDNET_MASTER_PORT, CVAR_SYSTEM | CVAR_ROM, "idnet master server address" );
idCVar				idAsyncNetwork::master1( "net_master1", "", CVAR_SYSTEM | CVAR_ARCHIVE, "1st master server address" );
idCVar				idAsyncNetwork::master2( "net_master2", "", CVAR_SYSTEM | CVAR_ARCHIVE, "2nd master server address" );
idCVar				idAsyncNetwork::master3( "net_master3", "", CVAR_SYSTEM | CVAR_ARCHIVE, "3rd master server address" );
idCVar				idAsyncNetwork::master4( "net_master4", "", CVAR_SYSTEM | CVAR_ARCHIVE, "4th master server address" );
idCVar				idAsyncNetwork::LANServer( "net_LANServer", "0", CVAR_SYSTEM | CVAR_BOOL | CVAR_NOCHEAT, "config LAN games only - affects clients and servers" );
idCVar				idAsyncNetwork::serverReloadEngine( "net_serverReloadEngine", "0", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT, "perform a full reload on next map restart (including flushing referenced pak files) - decreased if > 0" );
idCVar				idAsyncNetwork::serverAllowServerMod( "net_serverAllowServerMod", "0", CVAR_SYSTEM | CVAR_BOOL | CVAR_NOCHEAT, "allow server-side mods" );
idCVar				idAsyncNetwork::idleServer( "si_idleServer", "0", CVAR_SYSTEM | CVAR_BOOL | CVAR_INIT | CVAR_SERVERINFO, "game clients are idle" );
idCVar				idAsyncNetwork::clientDownload( "net_clientDownload", "1", CVAR_SYSTEM | CVAR_INTEGER | CVAR_ARCHIVE, "client pk4 downloads policy: 0 - never, 1 - ask, 2 - always (will still prompt for binary code)" );

int					idAsyncNetwork::realTime;
master_t			idAsyncNetwork::masters[ MAX_MASTER_SERVERS ];

/*
==================
idAsyncNetwork::idAsyncNetwork
==================
*/
idAsyncNetwork::idAsyncNetwork( void ) {
}

/*
==================
idAsyncNetwork::Init
==================
*/
void idAsyncNetwork::Init( void ) {

	realTime = 0;

	memset( masters, 0, sizeof( masters ) );
	masters[0].var = &master0;
	masters[1].var = &master1;
	masters[2].var = &master2;
	masters[3].var = &master3;
	masters[4].var = &master4;

#ifndef	ID_DEMO_BUILD
	cmdSystem->AddCommand( "spawnServer", SpawnServer_f, CMD_FL_SYSTEM, "spawns a server", idCmdSystem::ArgCompletion_MapName );
	cmdSystem->AddCommand( "nextMap", NextMap_f, CMD_FL_SYSTEM, "loads the next map on the server" );
	cmdSystem->AddCommand( "connect", Connect_f, CMD_FL_SYSTEM, "connects to a server" );
	cmdSystem->AddCommand( "reconnect", Reconnect_f, CMD_FL_SYSTEM, "reconnect to the last server we tried to connect to" );
	cmdSystem->AddCommand( "serverInfo", GetServerInfo_f, CMD_FL_SYSTEM, "shows server info" );
	cmdSystem->AddCommand( "LANScan", GetLANServers_f, CMD_FL_SYSTEM, "scans LAN for servers" );
	cmdSystem->AddCommand( "listServers", ListServers_f, CMD_FL_SYSTEM, "lists scanned servers" );
	cmdSystem->AddCommand( "rcon", RemoteConsole_f, CMD_FL_SYSTEM, "sends remote console command to server" );
	cmdSystem->AddCommand( "heartbeat", Heartbeat_f, CMD_FL_SYSTEM, "send a heartbeat to the the master servers" );
	cmdSystem->AddCommand( "kick", Kick_f, CMD_FL_SYSTEM, "kick a client by connection number" );
	cmdSystem->AddCommand( "checkNewVersion", CheckNewVersion_f, CMD_FL_SYSTEM, "check if a new version of the game is available" );
	cmdSystem->AddCommand( "updateUI", UpdateUI_f, CMD_FL_SYSTEM, "internal - cause a sync down of game-modified userinfo" );
#endif
}

/*
==================
idAsyncNetwork::GetMasterAddress
==================
*/
netadr_t idAsyncNetwork::GetMasterAddress( void ) {
	netadr_t ret;
	GetMasterAddress( 0, ret );
	return masters[ 0 ].address;
}

/*
==================
idAsyncNetwork::GetMasterAddress
==================
*/
bool idAsyncNetwork::GetMasterAddress( int index, netadr_t &adr ) {
	if ( !masters[ index ].var ) {
		return false;
	}	
	if ( masters[ index ].var->GetString()[0] == '\0' ) {
		return false;
	}
	if ( !masters[ index ].resolved || masters[ index ].var->IsModified() ) {
		masters[ index ].var->ClearModified();
		if ( !Sys_StringToNetAdr( masters[ index ].var->GetString(), &masters[ index ].address, true ) ) {
			common->Printf( "Failed to resolve master%d: %s\n", index, masters[ index ].var->GetString() );
			memset( &masters[ index ].address, 0, sizeof( netadr_t ) );
			masters[ index ].resolved = true;
			return false;
		}
		if ( masters[ index ].address.port == 0 ) {
			masters[ index ].address.port = atoi( IDNET_MASTER_PORT );
		}
		masters[ index ].resolved = true;
	}
	adr = masters[ index ].address;
	return true;
}

/*
==================
idAsyncNetwork::Shutdown
==================
*/
void idAsyncNetwork::Shutdown( void ) {
	client.serverList.Shutdown();
	client.DisconnectFromServer();
	client.ClearServers();
	client.ClosePort();
	server.Kill();
	server.ClosePort();
}

/*
==================
idAsyncNetwork::RunFrame
==================
*/
void idAsyncNetwork::RunFrame( void ) {
	if ( console->Active() ) {
		Sys_GrabMouseCursor( false );
		usercmdGen->InhibitUsercmd( INHIBIT_ASYNC, true );
	} else {
		Sys_GrabMouseCursor( true );
		usercmdGen->InhibitUsercmd( INHIBIT_ASYNC, false );
	}
	client.RunFrame();
	server.RunFrame();
}

/*
==================
idAsyncNetwork::WriteUserCmdDelta
==================
*/
void idAsyncNetwork::WriteUserCmdDelta( idBitMsg &msg, const usercmd_t &cmd, const usercmd_t *base ) {
	if ( base ) {
		msg.WriteDeltaLongCounter( base->gameTime, cmd.gameTime );
		msg.WriteDeltaByte( base->buttons, cmd.buttons );
		msg.WriteDeltaShort( base->mx, cmd.mx );
		msg.WriteDeltaShort( base->my, cmd.my );
		msg.WriteDeltaChar( base->forwardmove, cmd.forwardmove );
		msg.WriteDeltaChar( base->rightmove, cmd.rightmove );
		msg.WriteDeltaChar( base->upmove, cmd.upmove );
		msg.WriteDeltaShort( base->angles[0], cmd.angles[0] );
		msg.WriteDeltaShort( base->angles[1], cmd.angles[1] );
		msg.WriteDeltaShort( base->angles[2], cmd.angles[2] );
		return;
	}

	msg.WriteLong( cmd.gameTime );
	msg.WriteByte( cmd.buttons );
    msg.WriteShort( cmd.mx );
	msg.WriteShort( cmd.my );
	msg.WriteChar( cmd.forwardmove );
	msg.WriteChar( cmd.rightmove );
	msg.WriteChar( cmd.upmove );
	msg.WriteShort( cmd.angles[0] );
	msg.WriteShort( cmd.angles[1] );
	msg.WriteShort( cmd.angles[2] );
}

/*
==================
idAsyncNetwork::ReadUserCmdDelta
==================
*/
void idAsyncNetwork::ReadUserCmdDelta( const idBitMsg &msg, usercmd_t &cmd, const usercmd_t *base ) {
	memset( &cmd, 0, sizeof( cmd ) );

	if ( base ) {
		cmd.gameTime = msg.ReadDeltaLongCounter( base->gameTime );
		cmd.buttons = msg.ReadDeltaByte( base->buttons );
		cmd.mx = msg.ReadDeltaShort( base->mx );
		cmd.my = msg.ReadDeltaShort( base->my );
		cmd.forwardmove = msg.ReadDeltaChar( base->forwardmove );
		cmd.rightmove = msg.ReadDeltaChar( base->rightmove );
		cmd.upmove = msg.ReadDeltaChar( base->upmove );
		cmd.angles[0] = msg.ReadDeltaShort( base->angles[0] );
		cmd.angles[1] = msg.ReadDeltaShort( base->angles[1] );
		cmd.angles[2] = msg.ReadDeltaShort( base->angles[2] );
		return;
	}

	cmd.gameTime = msg.ReadLong();
    cmd.buttons = msg.ReadByte();
    cmd.mx = msg.ReadShort();
	cmd.my = msg.ReadShort();
	cmd.forwardmove = msg.ReadChar();
	cmd.rightmove = msg.ReadChar();
	cmd.upmove = msg.ReadChar();
	cmd.angles[0] = msg.ReadShort();
	cmd.angles[1] = msg.ReadShort();
	cmd.angles[2] = msg.ReadShort();
}

/*
==================
idAsyncNetwork::DuplicateUsercmd
==================
*/
bool idAsyncNetwork::DuplicateUsercmd( const usercmd_t &previousUserCmd, usercmd_t &currentUserCmd, int frame, int time ) {

	if ( currentUserCmd.gameTime <= previousUserCmd.gameTime ) {

		currentUserCmd = previousUserCmd;
		currentUserCmd.gameFrame = frame;
		currentUserCmd.gameTime = time;
		currentUserCmd.duplicateCount++;

		if ( currentUserCmd.duplicateCount > MAX_USERCMD_DUPLICATION ) {
			currentUserCmd.buttons &= ~BUTTON_ATTACK;
			if ( abs( currentUserCmd.forwardmove ) > 2 ) currentUserCmd.forwardmove >>= 1;
			if ( abs( currentUserCmd.rightmove ) > 2 ) currentUserCmd.rightmove >>= 1;
			if ( abs( currentUserCmd.upmove ) > 2 ) currentUserCmd.upmove >>= 1;
		}

		return true;
	}
	return false;
}

/*
==================
idAsyncNetwork::UsercmdInputChanged
==================
*/
bool idAsyncNetwork::UsercmdInputChanged( const usercmd_t &previousUserCmd, const usercmd_t &currentUserCmd ) {
	return	previousUserCmd.buttons != currentUserCmd.buttons ||
			previousUserCmd.forwardmove != currentUserCmd.forwardmove ||
			previousUserCmd.rightmove != currentUserCmd.rightmove ||
			previousUserCmd.upmove != currentUserCmd.upmove ||
			previousUserCmd.angles[0] != currentUserCmd.angles[0] ||
			previousUserCmd.angles[1] != currentUserCmd.angles[1] ||
			previousUserCmd.angles[2] != currentUserCmd.angles[2];
}

/*
==================
idAsyncNetwork::SpawnServer_f
==================
*/
void idAsyncNetwork::SpawnServer_f( const idCmdArgs &args ) {

	if(args.Argc() > 1) {
		cvarSystem->SetCVarString("si_map", args.Argv(1));
	}

	// don't let a server spawn with singleplayer game type - it will crash
	if ( idStr::Icmp( cvarSystem->GetCVarString( "si_gameType" ), "singleplayer" ) == 0 ) {
		cvarSystem->SetCVarString( "si_gameType", "deathmatch" );
	}
	com_asyncInput.SetBool( false );
	// make sure the current system state is compatible with net_serverDedicated
	switch ( cvarSystem->GetCVarInteger( "net_serverDedicated" ) ) {
		case 0:
		case 2:
			if ( !renderSystem->IsOpenGLRunning() ) {
				common->Warning( "OpenGL is not running, net_serverDedicated == %d", cvarSystem->GetCVarInteger( "net_serverDedicated" ) );
			}
			break;
		case 1:
			if ( renderSystem->IsOpenGLRunning() ) {
				Sys_ShowConsole( 1, false );
				renderSystem->ShutdownOpenGL();
			}
			soundSystem->SetMute( true );
			soundSystem->ShutdownHW();
			break;
	}
	// use serverMapRestart if we already have a running server
	if ( server.IsActive() ) {
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "serverMapRestart" );
	} else {
		server.Spawn();
	}
}

/*
==================
idAsyncNetwork::NextMap_f
==================
*/
void idAsyncNetwork::NextMap_f( const idCmdArgs &args ) {
	server.ExecuteMapChange();
}

/*
==================
idAsyncNetwork::Connect_f
==================
*/
void idAsyncNetwork::Connect_f( const idCmdArgs &args ) {
	if ( server.IsActive() ) {
		common->Printf( "already running a server\n" );
		return;
	}
	if ( args.Argc() != 2 ) {
		common->Printf( "USAGE: connect <serverName>\n" );
		return;
	}
	com_asyncInput.SetBool( false );
	client.ConnectToServer( args.Argv( 1 ) );
}

/*
==================
idAsyncNetwork::Reconnect_f
==================
*/
void idAsyncNetwork::Reconnect_f( const idCmdArgs &args ) {
	client.Reconnect();
}

/*
==================
idAsyncNetwork::GetServerInfo_f
==================
*/
void idAsyncNetwork::GetServerInfo_f( const idCmdArgs &args ) {
	client.GetServerInfo( args.Argv( 1 ) );
}

/*
==================
idAsyncNetwork::GetLANServers_f
==================
*/
void idAsyncNetwork::GetLANServers_f( const idCmdArgs &args ) {
	client.GetLANServers();
}

/*
==================
idAsyncNetwork::ListServers_f
==================
*/
void idAsyncNetwork::ListServers_f( const idCmdArgs &args ) {
	client.ListServers();
}

/*
==================
idAsyncNetwork::RemoteConsole_f
==================
*/
void idAsyncNetwork::RemoteConsole_f( const idCmdArgs &args ) {
	client.RemoteConsole( args.Args() );
}

/*
==================
idAsyncNetwork::Heartbeat_f
==================
*/
void idAsyncNetwork::Heartbeat_f( const idCmdArgs &args ) {
	if ( !server.IsActive() ) {
		common->Printf( "server is not running\n" );
		return;
	}
	server.MasterHeartbeat( true );
}

/*
==================
idAsyncNetwork::Kick_f
==================
*/
void idAsyncNetwork::Kick_f( const idCmdArgs &args ) {
	idStr clientId;
	int iclient;

	if ( !server.IsActive() ) {
		common->Printf( "server is not running\n" );
		return;
	}

	clientId = args.Argv( 1 );
	if ( !clientId.IsNumeric() ) {
		common->Printf( "usage: kick <client number>\n" );
		return;
	}
	iclient = atoi( clientId );
	
	if ( server.GetLocalClientNum() == iclient ) {
		common->Printf( "can't kick the host\n" );
		return;
	}

	server.DropClient( iclient, "#str_07134" );
}

/*
==================
idAsyncNetwork::GetNETServers
==================
*/
void idAsyncNetwork::GetNETServers( ) {
	client.GetNETServers();
}

/*
==================
idAsyncNetwork::CheckNewVersion_f
==================
*/
void idAsyncNetwork::CheckNewVersion_f( const idCmdArgs &args ) {
	client.SendVersionCheck(); 
}

/*
==================
idAsyncNetwork::ExecuteSessionCommand
==================
*/
void idAsyncNetwork::ExecuteSessionCommand( const char *sessCmd ) {
	if ( sessCmd[ 0 ] ) {
		if ( !idStr::Icmp( sessCmd, "game_startmenu" ) ) {
			session->SetGUI( game->StartMenu(), NULL );
		}
	}
}

/*
=================
idAsyncNetwork::UpdateUI_f
=================
*/
void idAsyncNetwork::UpdateUI_f( const idCmdArgs &args ) {
	if ( args.Argc() != 2 ) {
		common->Warning( "idAsyncNetwork::UpdateUI_f: wrong arguments\n" );
		return;
	}
	if ( !server.IsActive() ) {
		common->Warning( "idAsyncNetwork::UpdateUI_f: server is not active\n" );
		return;
	}
	int clientNum = atoi( args.Args( 1 ) );
	server.UpdateUI( clientNum );
}

/*
===============
idAsyncNetwork::BuildInvalidKeyMsg
===============
*/
void idAsyncNetwork::BuildInvalidKeyMsg( idStr &msg, bool valid[ 2 ] ) {
	if ( !valid[ 0 ] ) {
		msg += common->GetLanguageDict()->GetString( "#str_07194" );
	}
	if ( fileSystem->HasD3XP() && !valid[ 1 ] ) {
		if ( msg.Length() ) {
			msg += "\n";
		}
		msg += common->GetLanguageDict()->GetString( "#str_07195" );
	}
	msg += "\n";
	msg += common->GetLanguageDict()->GetString( "#str_04304" );	
}


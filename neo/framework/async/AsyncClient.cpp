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

#include "../Session_local.h"

const int SETUP_CONNECTION_RESEND_TIME	= 1000;
const int EMPTY_RESEND_TIME				= 500;
const int PREDICTION_FAST_ADJUST		= 4;


/*
==================
idAsyncClient::idAsyncClient
==================
*/
idAsyncClient::idAsyncClient( void ) {
	guiNetMenu = NULL;
	updateState = UPDATE_NONE;
	Clear();
}

/*
==================
idAsyncClient::Clear
==================
*/
void idAsyncClient::Clear( void ) {
	active = false;
	realTime = 0;
	clientTime = 0;
	clientId = 0;
	clientDataChecksum = 0;
	clientNum = 0;
	clientState = CS_DISCONNECTED;
	clientPrediction = 0;
	clientPredictTime = 0;
	serverId = 0;
	serverChallenge = 0;
	serverMessageSequence = 0;
	lastConnectTime = -9999;
	lastEmptyTime = -9999;
	lastPacketTime = -9999;
	lastSnapshotTime = -9999;
	snapshotGameFrame = 0;
	snapshotGameTime = 0;
	snapshotSequence = 0;
	gameInitId = GAME_INIT_ID_INVALID;
	gameFrame = 0;
	gameTimeResidual = 0;
	gameTime = 0;
	memset( userCmds, 0, sizeof( userCmds ) );
	backgroundDownload.completed = true;
	lastRconTime = 0;
	showUpdateMessage = false;
	lastFrameDelta = 0;

	dlRequest = -1;
	dlCount = -1;
	memset( dlChecksums, 0, sizeof( int ) * MAX_PURE_PAKS );
	currentDlSize = 0;
	totalDlSize = 0;
}

/*
==================
idAsyncClient::Shutdown
==================
*/
void idAsyncClient::Shutdown( void ) {
	guiNetMenu = NULL;
	updateMSG.Clear();
	updateURL.Clear();
	updateFile.Clear();
	updateFallback.Clear();
	backgroundDownload.url.url.Clear();
	dlList.Clear();
}

/*
==================
idAsyncClient::InitPort
==================
*/
bool idAsyncClient::InitPort( void ) {
	// if this is the first time we connect to a server, open the UDP port
	if ( !clientPort.GetPort() ) {
		if ( !clientPort.InitForPort( PORT_ANY ) ) {
			common->Printf( "Couldn't open client network port.\n" );
			return false;
		}
	}
	// maintain it valid between connects and ui manager reloads
	guiNetMenu = uiManager->FindGui( "guis/netmenu.gui", true, false, true );

	return true;
}

/*
==================
idAsyncClient::ClosePort
==================
*/
void idAsyncClient::ClosePort( void ) {
	clientPort.Close();
}

/*
==================
idAsyncClient::ClearPendingPackets
==================
*/
void idAsyncClient::ClearPendingPackets( void ) {
	int			size;
	byte		msgBuf[MAX_MESSAGE_SIZE];
	netadr_t	from;

	while( clientPort.GetPacket( from, msgBuf, size, sizeof( msgBuf ) ) ) {
	}
}

/*
==================
idAsyncClient::HandleGuiCommandInternal
==================
*/
const char* idAsyncClient::HandleGuiCommandInternal( const char *cmd ) {
	if ( !idStr::Cmp( cmd, "abort" ) || !idStr::Cmp( cmd, "pure_abort" ) ) {
		common->DPrintf( "connection aborted\n" );
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
		return "";
	} else {
		common->DWarning( "idAsyncClient::HandleGuiCommand: unknown cmd %s", cmd );
	}
	return NULL;
}

/*
==================
idAsyncClient::HandleGuiCommand
==================
*/
const char* idAsyncClient::HandleGuiCommand( const char *cmd ) {
	return idAsyncNetwork::client.HandleGuiCommandInternal( cmd );
}

/*
==================
idAsyncClient::ConnectToServer
==================
*/
void idAsyncClient::ConnectToServer( const netadr_t adr ) {
	// shutdown any current game. that includes network disconnect
	session->Stop();

	if ( !InitPort() ) {
		return;
	}

	if ( cvarSystem->GetCVarBool( "net_serverDedicated" ) ) {
		common->Printf( "Can't connect to a server as dedicated\n" );
		return;
	}

	// trash any currently pending packets
	ClearPendingPackets();
	
	serverAddress = adr;

	// clear the client state
	Clear();

	// get a pseudo random client id, but don't use the id which is reserved for connectionless packets
	clientId = Sys_Milliseconds() & CONNECTIONLESS_MESSAGE_ID_MASK;

	// calculate a checksum on some of the essential data used
	clientDataChecksum = declManager->GetChecksum();

	// start challenging the server
	clientState = CS_CHALLENGING;

	active = true;

	guiNetMenu = uiManager->FindGui( "guis/netmenu.gui", true, false, true );
	guiNetMenu->SetStateString( "status", va( common->GetLanguageDict()->GetString( "#str_06749" ), Sys_NetAdrToString( adr ) ) );
	session->SetGUI( guiNetMenu, HandleGuiCommand );
}

/*
==================
idAsyncClient::Reconnect
==================
*/
void idAsyncClient::Reconnect( void ) {
	ConnectToServer( serverAddress );
}

/*
==================
idAsyncClient::ConnectToServer
==================
*/
void idAsyncClient::ConnectToServer( const char *address ) {
	int serverNum;
	netadr_t adr;

	if ( idStr::IsNumeric( address ) ) {
		serverNum = atoi( address );
		if ( serverNum < 0 || serverNum >= serverList.Num() ) {
			session->MessageBox( MSG_OK, va( common->GetLanguageDict()->GetString( "#str_06733" ), serverNum ), common->GetLanguageDict()->GetString( "#str_06735" ), true );
			return;
		}
		adr = serverList[ serverNum ].adr;
	} else {
		if ( !Sys_StringToNetAdr( address, &adr, true ) ) {
			session->MessageBox( MSG_OK, va( common->GetLanguageDict()->GetString( "#str_06734" ), address ), common->GetLanguageDict()->GetString( "#str_06735" ), true );
			return;
		}
	}
	if ( !adr.port ) {
		adr.port = PORT_SERVER;
	}

	common->Printf( "\"%s\" resolved to %s\n", address, Sys_NetAdrToString( adr ) );

	ConnectToServer( adr );
}

/*
==================
idAsyncClient::DisconnectFromServer
==================
*/
void idAsyncClient::DisconnectFromServer( void ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	if ( clientState >= CS_CONNECTED ) {
		// if we were actually connected, clear the pure list
		fileSystem->ClearPureChecksums();

		// send reliable disconnect to server
		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteByte( CLIENT_RELIABLE_MESSAGE_DISCONNECT );
		msg.WriteString( "disconnect" );

		if ( !channel.SendReliableMessage( msg ) ) {
			common->Error( "client->server reliable messages overflow\n" );
		}

		SendEmptyToServer( true );
		SendEmptyToServer( true );
		SendEmptyToServer( true );
	}

	if ( clientState != CS_PURERESTART ) {
		channel.Shutdown();
		clientState = CS_DISCONNECTED;
	}

	active = false;
}

/*
==================
idAsyncClient::GetServerInfo
==================
*/
void idAsyncClient::GetServerInfo( const netadr_t adr ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];
	
	if ( !InitPort() ) {
		return;
	}

	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	msg.WriteString( "getInfo" );
	msg.WriteLong( serverList.GetChallenge() );	// challenge

	clientPort.SendPacket( adr, msg.GetData(), msg.GetSize() );	
}

/*
==================
idAsyncClient::GetServerInfo
==================
*/
void idAsyncClient::GetServerInfo( const char *address ) {
	netadr_t	adr;

	if ( address && *address != '\0' ) {
		if ( !Sys_StringToNetAdr( address, &adr, true ) ) {
			common->Printf( "Couldn't get server address for \"%s\"\n", address );
			return;
		}
	} else if ( active ) {
		adr = serverAddress;
	} else if ( idAsyncNetwork::server.IsActive() ) {
		// used to be a Sys_StringToNetAdr( "localhost", &adr, true ); and send a packet over loopback
		// but this breaks with net_ip ( typically, for multi-homed servers )
		idAsyncNetwork::server.PrintLocalServerInfo();
		return;
	} else {
		common->Printf( "no server found\n" );
		return;
	}

	if ( !adr.port ) {
		adr.port = PORT_SERVER;
	}

	GetServerInfo( adr );
}

/*
==================
idAsyncClient::GetLANServers
==================
*/
void idAsyncClient::GetLANServers( void ) {
	int			i;
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];
	netadr_t	broadcastAddress;

	if ( !InitPort() ) {
		return;
	}

	idAsyncNetwork::LANServer.SetBool( true );

	serverList.SetupLANScan();

	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	msg.WriteString( "getInfo" );
	msg.WriteLong( serverList.GetChallenge() );

	broadcastAddress.type = NA_BROADCAST;
	for ( i = 0; i < MAX_SERVER_PORTS; i++ ) {
		broadcastAddress.port = PORT_SERVER + i;
		clientPort.SendPacket( broadcastAddress, msg.GetData(), msg.GetSize() );
	}
}

/*
==================
idAsyncClient::GetNETServers
==================
*/
void idAsyncClient::GetNETServers( void ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	idAsyncNetwork::LANServer.SetBool( false );

	// NetScan only clears GUI and results, not the stored list
	serverList.Clear( );
	serverList.NetScan( );
	serverList.StartServers( true );

	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	msg.WriteString( "getServers" );
	msg.WriteLong( ASYNC_PROTOCOL_VERSION );
	msg.WriteString( cvarSystem->GetCVarString( "fs_game" ) );
	msg.WriteBits( cvarSystem->GetCVarInteger( "gui_filter_password" ), 2 );
	msg.WriteBits( cvarSystem->GetCVarInteger( "gui_filter_players" ), 2 );
	msg.WriteBits( cvarSystem->GetCVarInteger( "gui_filter_gameType" ), 2 );

	netadr_t adr;
	if ( idAsyncNetwork::GetMasterAddress( 0, adr ) ) {
		clientPort.SendPacket( adr, msg.GetData(), msg.GetSize() );
	}
}

/*
==================
idAsyncClient::ListServers
==================
*/
void idAsyncClient::ListServers( void ) {
	int i;

	for ( i = 0; i < serverList.Num(); i++ ) {
		common->Printf( "%3d: %s %dms (%s)\n", i, serverList[i].serverInfo.GetString( "si_name" ), serverList[ i ].ping, Sys_NetAdrToString( serverList[i].adr ) );
	}
}

/*
==================
idAsyncClient::ClearServers
==================
*/
void idAsyncClient::ClearServers( void ) {
	serverList.Clear();
}

/*
==================
idAsyncClient::RemoteConsole
==================
*/
void idAsyncClient::RemoteConsole( const char *command ) {
	netadr_t	adr;
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	if ( !InitPort() ) {
		return;
	}

	if ( active ) {
		adr = serverAddress;
	} else {
		Sys_StringToNetAdr( idAsyncNetwork::clientRemoteConsoleAddress.GetString(), &adr, true );
	}
	
	if ( !adr.port ) {
		adr.port = PORT_SERVER;
	}

	lastRconAddress = adr;
	lastRconTime = realTime;

	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	msg.WriteString( "rcon" );
	msg.WriteString( idAsyncNetwork::clientRemoteConsolePassword.GetString() );
	msg.WriteString( command );

	clientPort.SendPacket( adr, msg.GetData(), msg.GetSize() );	
}

/*
==================
idAsyncClient::GetPrediction
==================
*/
int idAsyncClient::GetPrediction( void ) const {
	if ( clientState < CS_CONNECTED ) {
		return -1;
	} else {
		return clientPrediction;
	}
}

/*
==================
idAsyncClient::GetTimeSinceLastPacket
==================
*/
int idAsyncClient::GetTimeSinceLastPacket( void ) const {
	if ( clientState < CS_CONNECTED ) {
		return -1;
	} else {
		return clientTime - lastPacketTime;
	}
}

/*
==================
idAsyncClient::GetOutgoingRate
==================
*/
int idAsyncClient::GetOutgoingRate( void ) const {
	if ( clientState < CS_CONNECTED ) {
		return -1;
	} else {
		return channel.GetOutgoingRate();
	}
}

/*
==================
idAsyncClient::GetIncomingRate
==================
*/
int idAsyncClient::GetIncomingRate( void ) const {
	if ( clientState < CS_CONNECTED ) {
		return -1;
	} else {
		return channel.GetIncomingRate();
	}
}

/*
==================
idAsyncClient::GetOutgoingCompression
==================
*/
float idAsyncClient::GetOutgoingCompression( void ) const {
	if ( clientState < CS_CONNECTED ) {
		return 0.0f;
	} else {
		return channel.GetOutgoingCompression();
	}
}

/*
==================
idAsyncClient::GetIncomingCompression
==================
*/
float idAsyncClient::GetIncomingCompression( void ) const {
	if ( clientState < CS_CONNECTED ) {
		return 0.0f;
	} else {
		return channel.GetIncomingCompression();
	}
}

/*
==================
idAsyncClient::GetIncomingPacketLoss
==================
*/
float idAsyncClient::GetIncomingPacketLoss( void ) const {
	if ( clientState < CS_CONNECTED ) {
		return 0.0f;
	} else {
		return channel.GetIncomingPacketLoss();
	}
}

/*
==================
idAsyncClient::DuplicateUsercmds
==================
*/
void idAsyncClient::DuplicateUsercmds( int frame, int time ) {
	int i, previousIndex, currentIndex;

	previousIndex = ( frame - 1 ) & ( MAX_USERCMD_BACKUP - 1 );
	currentIndex = frame & ( MAX_USERCMD_BACKUP - 1 );

	// duplicate previous user commands if no new commands are available for a client
	for ( i = 0; i < MAX_ASYNC_CLIENTS; i++ ) {
		idAsyncNetwork::DuplicateUsercmd( userCmds[previousIndex][i], userCmds[currentIndex][i], frame, time );
	}
}

/*
==================
idAsyncClient::SendUserInfoToServer
==================
*/
void idAsyncClient::SendUserInfoToServer( void ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];
	idDict		info;

	if ( clientState < CS_CONNECTED ) {
		return;
	}

	info = *cvarSystem->MoveCVarsToDict( CVAR_USERINFO );
	
	// send reliable client info to server
	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteByte( CLIENT_RELIABLE_MESSAGE_CLIENTINFO );
	msg.WriteDeltaDict( info, &sessLocal.mapSpawnData.userInfo[ clientNum ] );

	if ( !channel.SendReliableMessage( msg ) ) {
		common->Error( "client->server reliable messages overflow\n" );
	}

	sessLocal.mapSpawnData.userInfo[clientNum] = info;
}

/*
==================
idAsyncClient::SendEmptyToServer
==================
*/
void idAsyncClient::SendEmptyToServer( bool force, bool mapLoad ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	if ( lastEmptyTime > realTime ) {
		lastEmptyTime = realTime;
	}

	if ( !force && ( realTime - lastEmptyTime < EMPTY_RESEND_TIME ) ) {
		return;
	}

	if ( idAsyncNetwork::verbose.GetInteger() ) {
		common->Printf( "sending empty to server, gameInitId = %d\n", mapLoad ? GAME_INIT_ID_MAP_LOAD : gameInitId );
	}

	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteLong( serverMessageSequence );
	msg.WriteLong( mapLoad ? GAME_INIT_ID_MAP_LOAD : gameInitId );
	msg.WriteLong( snapshotSequence );
	msg.WriteByte( CLIENT_UNRELIABLE_MESSAGE_EMPTY );

	channel.SendMessage( clientPort, clientTime, msg );

	while( channel.UnsentFragmentsLeft() ) {
		channel.SendNextFragment( clientPort, clientTime );
	}

	lastEmptyTime = realTime;
}

/*
==================
idAsyncClient::SendPingResponseToServer
==================
*/
void idAsyncClient::SendPingResponseToServer( int time ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	if ( idAsyncNetwork::verbose.GetInteger() == 2 ) {
		common->Printf( "sending ping response to server, gameInitId = %d\n", gameInitId );
	}

	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteLong( serverMessageSequence );
	msg.WriteLong( gameInitId );
	msg.WriteLong( snapshotSequence );
	msg.WriteByte( CLIENT_UNRELIABLE_MESSAGE_PINGRESPONSE );
	msg.WriteLong( time );

	channel.SendMessage( clientPort, clientTime, msg );
	while( channel.UnsentFragmentsLeft() ) {
		channel.SendNextFragment( clientPort, clientTime );
	}
}

/*
==================
idAsyncClient::SendUsercmdsToServer
==================
*/
void idAsyncClient::SendUsercmdsToServer( void ) {
	int			i, numUsercmds, index;
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];
	usercmd_t *	last;

	if ( idAsyncNetwork::verbose.GetInteger() == 2 ) {
		common->Printf( "sending usercmd to server: gameInitId = %d, gameFrame = %d, gameTime = %d\n", gameInitId, gameFrame, gameTime );
	}

	// generate user command for this client
	index = gameFrame & ( MAX_USERCMD_BACKUP - 1 );
	userCmds[index][clientNum] = usercmdGen->GetDirectUsercmd();
	userCmds[index][clientNum].gameFrame = gameFrame;
	userCmds[index][clientNum].gameTime = gameTime;

	// send the user commands to the server
	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteLong( serverMessageSequence );
	msg.WriteLong( gameInitId );
	msg.WriteLong( snapshotSequence );
	msg.WriteByte( CLIENT_UNRELIABLE_MESSAGE_USERCMD );
	msg.WriteShort( clientPrediction );

	numUsercmds = idMath::ClampInt( 0, 10, idAsyncNetwork::clientUsercmdBackup.GetInteger() ) + 1;

	// write the user commands
	msg.WriteLong( gameFrame );
	msg.WriteByte( numUsercmds );
	for ( last = NULL, i = gameFrame - numUsercmds + 1; i <= gameFrame; i++ ) {
		index = i & ( MAX_USERCMD_BACKUP - 1 );
		idAsyncNetwork::WriteUserCmdDelta( msg, userCmds[index][clientNum], last );
		last = &userCmds[index][clientNum];
	}

	channel.SendMessage( clientPort, clientTime, msg );
	while( channel.UnsentFragmentsLeft() ) {
		channel.SendNextFragment( clientPort, clientTime );
	}
}

/*
==================
idAsyncClient::InitGame
==================
*/
void idAsyncClient::InitGame( int serverGameInitId, int serverGameFrame, int serverGameTime, const idDict &serverSI ) {
	gameInitId = serverGameInitId;
	gameFrame = snapshotGameFrame = serverGameFrame;
	gameTime = snapshotGameTime = serverGameTime;
	gameTimeResidual = 0;
	memset( userCmds, 0, sizeof( userCmds ) );

	for ( int i = 0; i < MAX_ASYNC_CLIENTS; i++ ) {
		sessLocal.mapSpawnData.userInfo[ i ].Clear();
	}

	sessLocal.mapSpawnData.serverInfo = serverSI;
}

/*
==================
idAsyncClient::ProcessUnreliableServerMessage
==================
*/
void idAsyncClient::ProcessUnreliableServerMessage( const idBitMsg &msg ) {
	int i, j, index, id, numDuplicatedUsercmds, aheadOfServer, numUsercmds, delta;
	int serverGameInitId, serverGameFrame, serverGameTime;
	idDict serverSI;
	usercmd_t *last;
	bool pureWait;

	serverGameInitId = msg.ReadLong();

	id = msg.ReadByte();
	switch( id ) {
		case SERVER_UNRELIABLE_MESSAGE_EMPTY: {
			if ( idAsyncNetwork::verbose.GetInteger() ) {
				common->Printf( "received empty message from server\n" );
			}
			break;
		}
		case SERVER_UNRELIABLE_MESSAGE_PING: {
			if ( idAsyncNetwork::verbose.GetInteger() == 2 ) {
				common->Printf( "received ping message from server\n" );
			}
			SendPingResponseToServer( msg.ReadLong() );
			break;
		}
		case SERVER_UNRELIABLE_MESSAGE_GAMEINIT: {
			serverGameFrame = msg.ReadLong();
			serverGameTime = msg.ReadLong();
			msg.ReadDeltaDict( serverSI, NULL );
			pureWait = serverSI.GetBool( "si_pure" );

			InitGame( serverGameInitId, serverGameFrame, serverGameTime, serverSI );

			channel.ResetRate();

			if ( idAsyncNetwork::verbose.GetInteger() ) {
				common->Printf( "received gameinit, gameInitId = %d, gameFrame = %d, gameTime = %d\n", gameInitId, gameFrame, gameTime );
			}

			// mute sound
			soundSystem->SetMute( true );

			// ensure chat icon goes away when the GUI is changed...
			//cvarSystem->SetCVarBool( "ui_chat", false );

			if ( pureWait ) {
				guiNetMenu = uiManager->FindGui( "guis/netmenu.gui", true, false, true );
				session->SetGUI( guiNetMenu, HandleGuiCommand );
				session->MessageBox( MSG_ABORT, common->GetLanguageDict()->GetString ( "#str_04317" ), common->GetLanguageDict()->GetString ( "#str_04318" ), false, "pure_abort" );
			} else {
				// load map
				session->SetGUI( NULL, NULL );
				sessLocal.ExecuteMapChange();
			}

			break;
		}
		case SERVER_UNRELIABLE_MESSAGE_SNAPSHOT: {
			// if the snapshot is from a different game
			if ( serverGameInitId != gameInitId ) {
				if ( idAsyncNetwork::verbose.GetInteger() ) {
					common->Printf( "ignoring snapshot with != gameInitId\n" );
				}
				break;
			}

			snapshotSequence = msg.ReadLong();
			snapshotGameFrame = msg.ReadLong();
			snapshotGameTime = msg.ReadLong();
			numDuplicatedUsercmds = msg.ReadByte();
			aheadOfServer = msg.ReadShort();

			// read the game snapshot
			game->ClientReadSnapshot( clientNum, snapshotSequence, snapshotGameFrame, snapshotGameTime, numDuplicatedUsercmds, aheadOfServer, msg );

			// read user commands of other clients from the snapshot
			for ( last = NULL, i = msg.ReadByte(); i < MAX_ASYNC_CLIENTS; i = msg.ReadByte() ) {
				numUsercmds = msg.ReadByte();
				if ( numUsercmds > MAX_USERCMD_RELAY ) {
					common->Error( "snapshot %d contains too many user commands for client %d", snapshotSequence, i );
					break;
				}
				for ( j = 0; j < numUsercmds; j++ ) {
					index = ( snapshotGameFrame + j ) & ( MAX_USERCMD_BACKUP - 1 );
					idAsyncNetwork::ReadUserCmdDelta( msg, userCmds[index][i], last );
					userCmds[index][i].gameFrame = snapshotGameFrame + j;
					userCmds[index][i].duplicateCount = 0;
					last = &userCmds[index][i];
				}
				// clear all user commands after the ones just read from the snapshot
				for ( j = numUsercmds; j < MAX_USERCMD_BACKUP; j++ ) {
					index = ( snapshotGameFrame + j ) & ( MAX_USERCMD_BACKUP - 1 );
					userCmds[index][i].gameFrame = 0;
					userCmds[index][i].gameTime = 0;
				}
			}

			// if this is the first snapshot after a game init was received
			if ( clientState == CS_CONNECTED ) {
				gameTimeResidual = 0;
				clientState = CS_INGAME;
				assert( !sessLocal.GetActiveMenu( ) );
				if ( idAsyncNetwork::verbose.GetInteger() ) {
					common->Printf( "received first snapshot, gameInitId = %d, gameFrame %d gameTime %d\n", gameInitId, snapshotGameFrame, snapshotGameTime );
				}
			}

			// if the snapshot is newer than the clients current game time
			if ( gameTime < snapshotGameTime || gameTime > snapshotGameTime + idAsyncNetwork::clientMaxPrediction.GetInteger() ) {
				gameFrame = snapshotGameFrame;
				gameTime = snapshotGameTime;
				gameTimeResidual = idMath::ClampInt( -idAsyncNetwork::clientMaxPrediction.GetInteger(), idAsyncNetwork::clientMaxPrediction.GetInteger(), gameTimeResidual );
				clientPredictTime = idMath::ClampInt( -idAsyncNetwork::clientMaxPrediction.GetInteger(), idAsyncNetwork::clientMaxPrediction.GetInteger(), clientPredictTime );
			}

			// adjust the client prediction time based on the snapshot time
			clientPrediction -= ( 1 - ( INTSIGNBITSET( aheadOfServer - idAsyncNetwork::clientPrediction.GetInteger() ) << 1 ) );
			clientPrediction = idMath::ClampInt( idAsyncNetwork::clientPrediction.GetInteger(), idAsyncNetwork::clientMaxPrediction.GetInteger(), clientPrediction );
			delta = gameTime - ( snapshotGameTime + clientPrediction );
			clientPredictTime -= ( delta / PREDICTION_FAST_ADJUST ) + ( 1 - ( INTSIGNBITSET( delta ) << 1 ) );

			lastSnapshotTime = clientTime;

			if ( idAsyncNetwork::verbose.GetInteger() == 2 ) {
				common->Printf( "received snapshot, gameInitId = %d, gameFrame = %d, gameTime = %d\n", gameInitId, gameFrame, gameTime );
			}

			if ( numDuplicatedUsercmds && ( idAsyncNetwork::verbose.GetInteger() == 2 ) ) {
				common->Printf( "server duplicated %d user commands before snapshot %d\n", numDuplicatedUsercmds, snapshotGameFrame );
			}
			break;
		}
		default: {
			common->Printf( "unknown unreliable server message %d\n", id );
			break;
		}
	}
}

/*
==================
idAsyncClient::ProcessReliableMessagePure
==================
*/
void idAsyncClient::ProcessReliableMessagePure( const idBitMsg &msg ) {
	idBitMsg	outMsg;
	byte		msgBuf[ MAX_MESSAGE_SIZE ];
	int			inChecksums[ MAX_PURE_PAKS ];
	int			i;
	int			gamePakChecksum;
	int			serverGameInitId;

	session->SetGUI( NULL, NULL );

	serverGameInitId = msg.ReadLong();

	if ( serverGameInitId != gameInitId ) {
		common->DPrintf( "ignoring pure server checksum from an outdated gameInitId (%d)\n", serverGameInitId );
		return;
	}

	if ( !ValidatePureServerChecksums( serverAddress, msg ) ) {
		
		return;
	}

	if ( idAsyncNetwork::verbose.GetInteger() ) {
		common->Printf( "received new pure server info. ExecuteMapChange and report back\n" );
	}

	// it is now ok to load the next map with updated pure checksums
	sessLocal.ExecuteMapChange( true );

	// upon receiving our pure list, the server will send us SCS_INGAME and we'll start getting snapshots
	fileSystem->GetPureServerChecksums( inChecksums, -1, &gamePakChecksum );
	outMsg.Init( msgBuf, sizeof( msgBuf ) );
	outMsg.WriteByte( CLIENT_RELIABLE_MESSAGE_PURE );

	outMsg.WriteLong( gameInitId );

	i = 0;
	while ( inChecksums[ i ] ) {
		outMsg.WriteLong( inChecksums[ i++ ] );
	}
	outMsg.WriteLong( 0 );
	outMsg.WriteLong( gamePakChecksum );

	if ( !channel.SendReliableMessage( outMsg ) ) {
		common->Error( "client->server reliable messages overflow\n" );
	}
}

/*
===============
idAsyncClient::ReadLocalizedServerString
===============
*/
void idAsyncClient::ReadLocalizedServerString( const idBitMsg &msg, char *out, int maxLen ) {
	msg.ReadString( out, maxLen );
	// look up localized string. if the message is not an #str_ format, we'll just get it back unchanged
	idStr::snPrintf( out, maxLen - 1, "%s", common->GetLanguageDict()->GetString( out ) );
}

/*
==================
idAsyncClient::ProcessReliableServerMessages
==================
*/
void idAsyncClient::ProcessReliableServerMessages( void ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];
	byte		id;

	msg.Init( msgBuf, sizeof( msgBuf ) );

	while ( channel.GetReliableMessage( msg ) ) {
		id = msg.ReadByte();
		switch( id ) {
			case SERVER_RELIABLE_MESSAGE_CLIENTINFO: {
				int clientNum;
				clientNum = msg.ReadByte();

				idDict &info = sessLocal.mapSpawnData.userInfo[ clientNum ];
				bool haveBase = ( msg.ReadBits( 1 ) != 0 );

#if ID_CLIENTINFO_TAGS
				int checksum = info.Checksum();
				int srv_checksum = msg.ReadLong();
				if ( checksum != srv_checksum ) {
					common->DPrintf( "SERVER_RELIABLE_MESSAGE_CLIENTINFO %d (haveBase: %s): != checksums srv: 0x%x local: 0x%x\n", clientNum, haveBase ? "true" : "false", checksum, srv_checksum );
					info.Print();
				} else {
					common->DPrintf( "SERVER_RELIABLE_MESSAGE_CLIENTINFO %d (haveBase: %s): checksums ok 0x%x\n", clientNum, haveBase ? "true" : "false", checksum );
				}
#endif

				if ( haveBase ) {
					msg.ReadDeltaDict( info, &info );
				} else {
					msg.ReadDeltaDict( info, NULL );
				}

				// server forces us to a different userinfo
				if ( clientNum == idAsyncClient::clientNum ) {
					common->DPrintf( "local user info modified by server\n" );
					cvarSystem->SetCVarsFromDict( info );
					cvarSystem->ClearModifiedFlags( CVAR_USERINFO ); // don't emit back
				}
				game->SetUserInfo( clientNum, info, true, false );
				break;
			}
			case SERVER_RELIABLE_MESSAGE_SYNCEDCVARS: {
				idDict &info = sessLocal.mapSpawnData.syncedCVars;
				msg.ReadDeltaDict( info, &info );
				cvarSystem->SetCVarsFromDict( info );
				if ( !idAsyncNetwork::allowCheats.GetBool() ) {
					cvarSystem->ResetFlaggedVariables( CVAR_CHEAT );
				}
				break;
			}
			case SERVER_RELIABLE_MESSAGE_PRINT: {
				char string[MAX_STRING_CHARS];
				msg.ReadString( string, MAX_STRING_CHARS );
				common->Printf( "%s\n", string );
				break;
			}
			case SERVER_RELIABLE_MESSAGE_DISCONNECT: {
				int clientNum;
				char string[MAX_STRING_CHARS];
				clientNum = msg.ReadLong( );
				ReadLocalizedServerString( msg, string, MAX_STRING_CHARS );
				if ( clientNum == idAsyncClient::clientNum ) {
					session->Stop();
					session->MessageBox( MSG_OK, string, common->GetLanguageDict()->GetString ( "#str_04319" ), true );
					session->StartMenu();
				} else {
					common->Printf( "client %d %s\n", clientNum, string );
					cmdSystem->BufferCommandText( CMD_EXEC_NOW, va( "addChatLine \"%s^0 %s\"", sessLocal.mapSpawnData.userInfo[ clientNum ].GetString( "ui_name" ), string ) );
					sessLocal.mapSpawnData.userInfo[ clientNum ].Clear();
				}
				break;
			}
			case SERVER_RELIABLE_MESSAGE_APPLYSNAPSHOT: {
				int sequence;
				sequence = msg.ReadLong();
				if ( !game->ClientApplySnapshot( clientNum, sequence ) ) {
					session->Stop();
					common->Error( "couldn't apply snapshot %d", sequence );
				}
				break;
			}
			case SERVER_RELIABLE_MESSAGE_PURE: {
				ProcessReliableMessagePure( msg );
				break;
			}
			case SERVER_RELIABLE_MESSAGE_RELOAD: {
				if ( idAsyncNetwork::verbose.GetBool() ) {
					common->Printf( "got MESSAGE_RELOAD from server\n" );
				}
				// simply reconnect, so that if the server restarts in pure mode we can get the right list and avoid spurious reloads
				cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "reconnect\n" );
				break;
			}
			case SERVER_RELIABLE_MESSAGE_ENTERGAME: {
				SendUserInfoToServer();
				game->SetUserInfo( clientNum, sessLocal.mapSpawnData.userInfo[ clientNum ], true, false );
				cvarSystem->ClearModifiedFlags( CVAR_USERINFO );
				break;
			}
			default: {
				// pass reliable message on to game code
				game->ClientProcessReliableMessage( clientNum, msg );
				break;
			}
		}
	}
}

/*
==================
idAsyncClient::ProcessChallengeResponseMessage
==================
*/
void idAsyncClient::ProcessChallengeResponseMessage( const netadr_t from, const idBitMsg &msg ) {	
	char serverGame[ MAX_STRING_CHARS ], serverGameBase[ MAX_STRING_CHARS ];

	if ( clientState != CS_CHALLENGING ) {
		common->Printf( "Unwanted challenge response received.\n" );
		return;
	}

	serverChallenge = msg.ReadLong();
	serverId = msg.ReadShort();
	msg.ReadString( serverGameBase, MAX_STRING_CHARS );
	msg.ReadString( serverGame, MAX_STRING_CHARS );

	// the server is running a different game... we need to reload in the correct fs_game
	// even pure pak checks would fail if we didn't, as there are files we may not even see atm
	// NOTE: we could read the pure list from the server at the same time and set it up for the restart
	// ( if the client can restart directly with the right pak order, then we avoid an extra reloadEngine later.. )
	if ( idStr::Icmp( cvarSystem->GetCVarString( "fs_game_base" ), serverGameBase ) ||
		idStr::Icmp( cvarSystem->GetCVarString( "fs_game" ), serverGame ) ) {
		// bug #189 - if the server is running ROE and ROE is not locally installed, refuse to connect or we might crash
		if ( !fileSystem->HasD3XP() && ( !idStr::Icmp( serverGameBase, "d3xp" ) || !idStr::Icmp( serverGame, "d3xp" ) ) ) {
			common->Printf( "The server is running Doom3: Resurrection of Evil expansion pack. RoE is not installed on this client. Aborting the connection..\n" );
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "disconnect\n" );
			return;
		}
		common->Printf( "The server is running a different mod (%s-%s). Restarting..\n", serverGameBase, serverGame );
		cvarSystem->SetCVarString( "fs_game_base", serverGameBase );
		cvarSystem->SetCVarString( "fs_game", serverGame );
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "reloadEngine" );
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "reconnect\n" );
		return;
	}

	common->Printf( "received challenge response 0x%x from %s\n", serverChallenge, Sys_NetAdrToString( from ) );

	// start sending connect packets instead of challenge request packets
	clientState = CS_CONNECTING;
	lastConnectTime = -9999;

	// take this address as the new server address.  This allows
	// a server proxy to hand off connections to multiple servers
	serverAddress = from;
}

/*
==================
idAsyncClient::ProcessConnectResponseMessage
==================
*/
void idAsyncClient::ProcessConnectResponseMessage( const netadr_t from, const idBitMsg &msg ) {
	int serverGameInitId, serverGameFrame, serverGameTime;
	idDict serverSI;

	if ( clientState >= CS_CONNECTED ) {
		common->Printf( "Duplicate connect received.\n" );
		return;
	}
	if ( clientState != CS_CONNECTING ) {
		common->Printf( "Connect response packet while not connecting.\n" );
		return;
	}
	if ( !Sys_CompareNetAdrBase( from, serverAddress ) ) {
		common->Printf( "Connect response from a different server.\n" );
		common->Printf( "%s should have been %s\n", Sys_NetAdrToString( from ), Sys_NetAdrToString( serverAddress ) );
		return;
	}

	common->Printf( "received connect response from %s\n", Sys_NetAdrToString( from ) );

	channel.Init( from, clientId );
	clientNum = msg.ReadLong();
	clientState = CS_CONNECTED;
	lastPacketTime = -9999;

	serverGameInitId = msg.ReadLong();
	serverGameFrame = msg.ReadLong();
	serverGameTime = msg.ReadLong();
	msg.ReadDeltaDict( serverSI, NULL );

	InitGame( serverGameInitId, serverGameFrame, serverGameTime, serverSI );

	// load map
	session->SetGUI( NULL, NULL );
	sessLocal.ExecuteMapChange();

	clientPredictTime = clientPrediction = idMath::ClampInt( 0, idAsyncNetwork::clientMaxPrediction.GetInteger(), clientTime - lastConnectTime );
}

/*
==================
idAsyncClient::ProcessDisconnectMessage
==================
*/
void idAsyncClient::ProcessDisconnectMessage( const netadr_t from, const idBitMsg &msg ) {
	if ( clientState == CS_DISCONNECTED ) {
		common->Printf( "Disconnect packet while not connected.\n" );
		return;
	}
	if ( !Sys_CompareNetAdrBase( from, serverAddress ) ) {
		common->Printf( "Disconnect packet from unknown server.\n" );
		return;
	}
	session->Stop();
	session->MessageBox( MSG_OK, common->GetLanguageDict()->GetString ( "#str_04320" ), NULL, true );
	session->StartMenu();
}

/*
==================
idAsyncClient::ProcessInfoResponseMessage
==================
*/
void idAsyncClient::ProcessInfoResponseMessage( const netadr_t from, const idBitMsg &msg ) {
	int i, protocol, index;
	networkServer_t serverInfo;
	bool verbose = false;

	if ( from.type == NA_LOOPBACK || cvarSystem->GetCVarBool( "developer" ) ) {
		verbose = true;
	}

	serverInfo.clients = 0;
	serverInfo.adr = from;
	serverInfo.challenge = msg.ReadLong();			// challenge
	protocol = msg.ReadLong();
	if ( protocol != ASYNC_PROTOCOL_VERSION ) {
		common->Printf( "server %s ignored - protocol %d.%d, expected %d.%d\n", Sys_NetAdrToString( serverInfo.adr ), protocol >> 16, protocol & 0xffff, ASYNC_PROTOCOL_MAJOR, ASYNC_PROTOCOL_MINOR );
		return;
	}
	msg.ReadDeltaDict( serverInfo.serverInfo, NULL );

	if ( verbose ) {
		common->Printf( "server IP = %s\n", Sys_NetAdrToString( serverInfo.adr ) );
		serverInfo.serverInfo.Print();
	}
	for ( i = msg.ReadByte(); i < MAX_ASYNC_CLIENTS; i = msg.ReadByte() ) {
		serverInfo.pings[ serverInfo.clients ] = msg.ReadShort();
		serverInfo.rate[ serverInfo.clients ] = msg.ReadLong();
		msg.ReadString( serverInfo.nickname[ serverInfo.clients ], MAX_NICKLEN );
		if ( verbose ) {
			common->Printf( "client %2d: %s, ping = %d, rate = %d\n", i, serverInfo.nickname[ serverInfo.clients ], serverInfo.pings[ serverInfo.clients ], serverInfo.rate[ serverInfo.clients ] );
		}
		serverInfo.clients++;
	}
	serverInfo.OSMask = msg.ReadLong();
	index = serverList.InfoResponse( serverInfo );

	common->Printf( "%d: server %s - protocol %d.%d - %s\n", index, Sys_NetAdrToString( serverInfo.adr ), protocol >> 16, protocol & 0xffff, serverInfo.serverInfo.GetString( "si_name" ) );
}

/*
==================
idAsyncClient::ProcessPrintMessage
==================
*/
void idAsyncClient::ProcessPrintMessage( const netadr_t from, const idBitMsg &msg ) {
	char		string[ MAX_STRING_CHARS ];
	int			opcode;
	int			game_opcode = ALLOW_YES;
	const char	*retpass;

	opcode = msg.ReadLong();
	if ( opcode == SERVER_PRINT_GAMEDENY ) {
		game_opcode = msg.ReadLong();
	}
	ReadLocalizedServerString( msg, string, MAX_STRING_CHARS );
	common->Printf( "%s\n", string );
	guiNetMenu->SetStateString( "status", string );
	if ( opcode == SERVER_PRINT_GAMEDENY ) {
		if ( game_opcode == ALLOW_BADPASS ) {
			retpass = session->MessageBox( MSG_PROMPT, common->GetLanguageDict()->GetString ( "#str_04321" ), string, true, "passprompt_ok" );
			ClearPendingPackets();
			guiNetMenu->SetStateString( "status",  common->GetLanguageDict()->GetString ( "#str_04322" ));
			if ( retpass ) {
				// #790
				cvarSystem->SetCVarString( "password", "" );
				cvarSystem->SetCVarString( "password", retpass );			
			} else {
				cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
			}
		} else if ( game_opcode == ALLOW_NO ) {
			session->MessageBox( MSG_OK, string, common->GetLanguageDict()->GetString ( "#str_04323" ), true );
			ClearPendingPackets();
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
		}
		// ALLOW_NOTYET just keeps running as usual. The GUI has an abort button
	} else if ( opcode == SERVER_PRINT_BADCHALLENGE && clientState >= CS_CONNECTING ) {
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "reconnect" );
	}
}

/*
==================
idAsyncClient::ProcessServersListMessage
==================
*/
void idAsyncClient::ProcessServersListMessage( const netadr_t from, const idBitMsg &msg ) {
	if ( !Sys_CompareNetAdrBase( idAsyncNetwork::GetMasterAddress(), from ) ) {
		common->DPrintf( "received a server list from %s - not a valid master\n", Sys_NetAdrToString( from ) );
		return;
	}
	while ( msg.GetRemaingData() ) {
		int a,b,c,d;
		a = msg.ReadByte(); b = msg.ReadByte(); c = msg.ReadByte(); d = msg.ReadByte();
		serverList.AddServer( serverList.Num(), va( "%i.%i.%i.%i:%i", a, b, c, d, msg.ReadShort() ) );
	}
}

/*
==================
idAsyncClient::ProcessAuthKeyMessage
==================
*/
void idAsyncClient::ProcessAuthKeyMessage( const netadr_t from, const idBitMsg &msg ) {
	authKeyMsg_t		authMsg;
	char				read_string[ MAX_STRING_CHARS ];
	const char			*retkey;
	authBadKeyStatus_t	authBadStatus;
	int					key_index;
	bool				valid[ 2 ];
	idStr				auth_msg;

	if ( clientState != CS_CONNECTING && !session->WaitingForGameAuth() ) {
		common->Printf( "clientState != CS_CONNECTING, not waiting for game auth, authKey ignored\n" );
		return;
	}

	authMsg = (authKeyMsg_t)msg.ReadByte();
	if ( authMsg == AUTHKEY_BADKEY ) {
		valid[ 0 ] = valid[ 1 ] = true;
		key_index = 0;
		authBadStatus = (authBadKeyStatus_t)msg.ReadByte();
		switch ( authBadStatus ) {
		case AUTHKEY_BAD_INVALID:
			valid[ 0 ] = ( msg.ReadByte() == 1 );
			valid[ 1 ] = ( msg.ReadByte() == 1 );
			idAsyncNetwork::BuildInvalidKeyMsg( auth_msg, valid );
			break;
		case AUTHKEY_BAD_BANNED:
			key_index = msg.ReadByte();
			auth_msg = common->GetLanguageDict()->GetString( va( "#str_0719%1d", 6 + key_index ) );
			auth_msg += "\n";
			auth_msg += common->GetLanguageDict()->GetString( "#str_04304" );
			valid[ key_index ] = false;
			break;
		case AUTHKEY_BAD_INUSE:
			key_index = msg.ReadByte();
			auth_msg = common->GetLanguageDict()->GetString( va( "#str_0719%1d", 8 + key_index ) );
			auth_msg += "\n";
			auth_msg += common->GetLanguageDict()->GetString( "#str_04304" );
			valid[ key_index ] = false;
			break;
		case AUTHKEY_BAD_MSG:
			// a general message explaining why this key is denied
			// no specific use for this atm. let's not clear the keys either
			msg.ReadString( read_string, MAX_STRING_CHARS );
			auth_msg = read_string;
			break;
		}
		common->DPrintf( "auth deny: %s\n", auth_msg.c_str() );
		
		// keys to be cleared. applies to both net connect and game auth
		session->ClearCDKey( valid );

		// get rid of the bad key - at least that's gonna annoy people who stole a fake key
		if ( clientState == CS_CONNECTING ) {
			while ( 1 ) {
				// here we use the auth status message
				retkey = session->MessageBox( MSG_CDKEY, auth_msg, common->GetLanguageDict()->GetString( "#str_04325" ), true );
				if ( retkey ) {
					if ( session->CheckKey( retkey, true, valid ) ) {
						cmdSystem->BufferCommandText( CMD_EXEC_NOW, "reconnect" );
					} else {
						// build a more precise message about the offline check failure
						idAsyncNetwork::BuildInvalidKeyMsg( auth_msg, valid );
						session->MessageBox( MSG_OK, auth_msg.c_str(), common->GetLanguageDict()->GetString( "#str_04327" ), true );
						continue;
					}
				} else {
					cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
				}
				break;
			}
		} else {
			// forward the auth status information to the session code
			session->CDKeysAuthReply( false, auth_msg );
		}
	} else {
		msg.ReadString( read_string, MAX_STRING_CHARS );
		cvarSystem->SetCVarString( "com_guid", read_string );
		common->Printf( "guid set to %s\n", read_string );
		session->CDKeysAuthReply( true, NULL );
	}
}

/*
==================
idAsyncClient::ProcessVersionMessage
==================
*/
void idAsyncClient::ProcessVersionMessage( const netadr_t from, const idBitMsg &msg ) {
	char string[ MAX_STRING_CHARS ];

	if ( updateState != UPDATE_SENT ) {
		common->Printf( "ProcessVersionMessage: version reply, != UPDATE_SENT\n" );
		return;
	}

	common->Printf( "A new version is available\n" );
	msg.ReadString( string, MAX_STRING_CHARS );
	updateMSG = string;
	updateDirectDownload = ( msg.ReadByte() != 0 );
	msg.ReadString( string, MAX_STRING_CHARS );
	updateURL = string;
	updateMime = (dlMime_t)msg.ReadByte();
	msg.ReadString( string, MAX_STRING_CHARS );
	updateFallback = string;
	updateState = UPDATE_READY;
}

/*
==================
idAsyncClient::ValidatePureServerChecksums
==================
*/
bool idAsyncClient::ValidatePureServerChecksums( const netadr_t from, const idBitMsg &msg ) {
	int			i, numChecksums, numMissingChecksums;
	int			inChecksums[ MAX_PURE_PAKS ];
	int			inGamePakChecksum;
	int			missingChecksums[ MAX_PURE_PAKS ];
	int			missingGamePakChecksum;
	idBitMsg	dlmsg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	// read checksums
	// pak checksums, in a 0-terminated list
	numChecksums = 0;
	do {
		i = msg.ReadLong( );
		inChecksums[ numChecksums++ ] = i;
		// just to make sure a broken message doesn't crash us
		if ( numChecksums >= MAX_PURE_PAKS ) {
			common->Warning( "MAX_PURE_PAKS ( %d ) exceeded in idAsyncClient::ProcessPureMessage\n", MAX_PURE_PAKS );
			return false;
		}
	} while ( i );
	inChecksums[ numChecksums ] = 0;
	inGamePakChecksum = msg.ReadLong();

	fsPureReply_t reply = fileSystem->SetPureServerChecksums( inChecksums, inGamePakChecksum, missingChecksums, &missingGamePakChecksum );
	switch ( reply ) {
		case PURE_RESTART:
			// need to restart the filesystem with a different pure configuration
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
			// restart with the right FS configuration and get back to the server
			clientState = CS_PURERESTART;	
			fileSystem->SetRestartChecksums( inChecksums, inGamePakChecksum );
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, "reloadEngine" );
			return false;
		case PURE_MISSING: {

			idStr checksums;

			i = 0;
			while ( missingChecksums[ i ] ) {
				checksums += va( "0x%x ", missingChecksums[ i++ ] );
			}
			numMissingChecksums = i;

			if ( idAsyncNetwork::clientDownload.GetInteger() == 0 ) {
				// never any downloads
				idStr message = va( common->GetLanguageDict()->GetString( "#str_07210" ), Sys_NetAdrToString( from ) );

				if ( numMissingChecksums > 0 ) {
					message += va( common->GetLanguageDict()->GetString( "#str_06751" ), numMissingChecksums, checksums.c_str() );
				}
				if ( missingGamePakChecksum ) {
					message += va( common->GetLanguageDict()->GetString( "#str_06750" ), missingGamePakChecksum );
				}

				common->Printf( message );
				cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
				session->MessageBox( MSG_OK, message, common->GetLanguageDict()->GetString( "#str_06735" ), true );
			} else {
				if ( clientState >= CS_CONNECTED ) {
					// we are already connected, reconnect to negociate the paks in connectionless mode
					cmdSystem->BufferCommandText( CMD_EXEC_NOW, "reconnect" );
					return false;
				}
				// ask the server to send back download info
				common->DPrintf( "missing %d paks: %s\n", numMissingChecksums + ( missingGamePakChecksum ? 1 : 0 ), checksums.c_str() );
				if ( missingGamePakChecksum ) {
					common->DPrintf( "game code pak: 0x%x\n", missingGamePakChecksum );
				}
				// store the requested downloads
				GetDownloadRequest( missingChecksums, numMissingChecksums, missingGamePakChecksum );
				// build the download request message
				// NOTE: in a specific function?
				dlmsg.Init( msgBuf, sizeof( msgBuf ) );
				dlmsg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
				dlmsg.WriteString( "downloadRequest" );
				dlmsg.WriteLong( serverChallenge );
				dlmsg.WriteShort( clientId );
				// used to make sure the server replies to the same download request
				dlmsg.WriteLong( dlRequest );
				// special case the code pak - if we have a 0 checksum then we don't need to download it
				dlmsg.WriteLong( missingGamePakChecksum );
				// 0-terminated list of missing paks
				i = 0;
				while ( missingChecksums[ i ] ) {
					dlmsg.WriteLong( missingChecksums[ i++ ] );
				}
				dlmsg.WriteLong( 0 );
				clientPort.SendPacket( from, dlmsg.GetData(), dlmsg.GetSize() );
			}

			return false;
		}
		case PURE_NODLL:
			common->Printf( common->GetLanguageDict()->GetString( "#str_07211" ), Sys_NetAdrToString( from ) );
			cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
			return false;
		default:
			return true;
	}
	return true;
}

/*
==================
idAsyncClient::ProcessPureMessage
==================
*/
void idAsyncClient::ProcessPureMessage( const netadr_t from, const idBitMsg &msg ) {
	idBitMsg	outMsg;
	byte		msgBuf[ MAX_MESSAGE_SIZE ];
	int			i;
	int			inChecksums[ MAX_PURE_PAKS ];
	int			gamePakChecksum;

	if ( clientState != CS_CONNECTING ) {
		common->Printf( "clientState != CS_CONNECTING, pure msg ignored\n" );
		return;
	}

	if ( !ValidatePureServerChecksums( from, msg ) ) {
		return;
	}

	fileSystem->GetPureServerChecksums( inChecksums, -1, &gamePakChecksum );
	outMsg.Init( msgBuf, sizeof( msgBuf ) );
	outMsg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	outMsg.WriteString( "pureClient" );
	outMsg.WriteLong( serverChallenge );
	outMsg.WriteShort( clientId );
	i = 0;
	while ( inChecksums[ i ] ) {
		outMsg.WriteLong( inChecksums[ i++ ] );
	}
	outMsg.WriteLong( 0 );
	outMsg.WriteLong( gamePakChecksum );
	clientPort.SendPacket( from, outMsg.GetData(), outMsg.GetSize() );
}

/*
==================
idAsyncClient::ConnectionlessMessage
==================
*/
void idAsyncClient::ConnectionlessMessage( const netadr_t from, const idBitMsg &msg ) {
	char string[MAX_STRING_CHARS*2];  // M. Quinn - Even Balance - PB packets can go beyond 1024

	msg.ReadString( string, sizeof( string ) );

	// info response from a server, are accepted from any source
	if ( idStr::Icmp( string, "infoResponse" ) == 0 ) {
		ProcessInfoResponseMessage( from, msg );
		return;
	}

	// from master server:
	if ( Sys_CompareNetAdrBase( from, idAsyncNetwork::GetMasterAddress( ) ) ) {
		// server list
		if ( idStr::Icmp( string, "servers" ) == 0 ) {
			ProcessServersListMessage( from, msg );
			return;
		}
	
		if ( idStr::Icmp( string, "authKey" ) == 0 ) {
			ProcessAuthKeyMessage( from, msg );
			return;
		}

		if ( idStr::Icmp( string, "newVersion" ) == 0 ) {
			ProcessVersionMessage( from, msg );
			return;
		}
	}

	// ignore if not from the current/last server
	if ( !Sys_CompareNetAdrBase( from, serverAddress ) && ( lastRconTime + 10000 < realTime || !Sys_CompareNetAdrBase( from, lastRconAddress ) ) ) {
		common->DPrintf( "got message '%s' from bad source: %s\n", string, Sys_NetAdrToString( from ) );
		return;
	}

	// challenge response from the server we are connecting to
	if ( idStr::Icmp( string, "challengeResponse" ) == 0 ) {
		ProcessChallengeResponseMessage( from, msg );
		return;
	}

	// connect response from the server we are connecting to
	if ( idStr::Icmp( string, "connectResponse" ) == 0 ) {
		ProcessConnectResponseMessage( from, msg );
		return;
	}

	// a disconnect message from the server, which will happen if the server
	// dropped the connection but is still getting packets from this client
	if ( idStr::Icmp( string, "disconnect" ) == 0 ) {
		ProcessDisconnectMessage( from, msg );
		return;
	}

	// print request from server
	if ( idStr::Icmp( string, "print" ) == 0 ) {
		ProcessPrintMessage( from, msg );
		return;
	}

	// server pure list
	if ( idStr::Icmp( string, "pureServer" ) == 0 ) {
		ProcessPureMessage( from, msg );
		return;
	}

	if ( idStr::Icmp( string, "downloadInfo" ) == 0 ) {
		ProcessDownloadInfoMessage( from, msg );
	}

	if ( idStr::Icmp( string, "authrequired" ) == 0 ) {
		// server telling us that he's expecting an auth mode connect, just in case we're trying to connect in LAN mode
		if ( idAsyncNetwork::LANServer.GetBool() ) {
			common->Warning( "server %s requests master authorization for this client. Turning off LAN mode\n", Sys_NetAdrToString( from ) );
			idAsyncNetwork::LANServer.SetBool( false );
		}
	}

	common->DPrintf( "ignored message from %s: %s\n", Sys_NetAdrToString( from ), string );
}

/*
=================
idAsyncClient::ProcessMessage
=================
*/
void idAsyncClient::ProcessMessage( const netadr_t from, idBitMsg &msg ) {
	int id;

	id = msg.ReadShort();

	// check for a connectionless packet
	if ( id == CONNECTIONLESS_MESSAGE_ID ) {
		ConnectionlessMessage( from, msg );
		return;
	}

	if ( clientState < CS_CONNECTED ) {
		return;		// can't be a valid sequenced packet
	}

	if ( msg.GetRemaingData() < 4 ) {
		common->DPrintf( "%s: tiny packet\n", Sys_NetAdrToString( from ) );
		return;
	}

	// is this a packet from the server
	if ( !Sys_CompareNetAdrBase( from, channel.GetRemoteAddress() ) || id != serverId ) {
		common->DPrintf( "%s: sequenced server packet without connection\n", Sys_NetAdrToString( from ) );
		return;
	}

	if ( !channel.Process( from, clientTime, msg, serverMessageSequence ) ) {
		return;		// out of order, duplicated, fragment, etc.
	}

	lastPacketTime = clientTime;
	ProcessReliableServerMessages();
	ProcessUnreliableServerMessage( msg );
}

/*
==================
idAsyncClient::SetupConnection
==================
*/
void idAsyncClient::SetupConnection( void ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	if ( clientTime - lastConnectTime < SETUP_CONNECTION_RESEND_TIME ) {
		return;
	}

	if ( clientState == CS_CHALLENGING ) {
		common->Printf( "sending challenge to %s\n", Sys_NetAdrToString( serverAddress ) );
		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
		msg.WriteString( "challenge" );
		msg.WriteLong( clientId );
		clientPort.SendPacket( serverAddress, msg.GetData(), msg.GetSize() );
	} else if ( clientState == CS_CONNECTING ) {
		common->Printf( "sending connect to %s with challenge 0x%x\n", Sys_NetAdrToString( serverAddress ), serverChallenge );
		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
		msg.WriteString( "connect" );
		msg.WriteLong( ASYNC_PROTOCOL_VERSION );
#if ID_FAKE_PURE
		// fake win32 OS - might need to adapt depending on the case
		msg.WriteShort( 0 );
#else
		msg.WriteShort( BUILD_OS_ID );
#endif
		msg.WriteLong( clientDataChecksum );
		msg.WriteLong( serverChallenge );
		msg.WriteShort( clientId );
		msg.WriteLong( cvarSystem->GetCVarInteger( "net_clientMaxRate" ) );
		msg.WriteString( cvarSystem->GetCVarString( "com_guid" ) );
		msg.WriteString( cvarSystem->GetCVarString( "password" ), -1, false );
		// do not make the protocol depend on PB
		msg.WriteShort( 0 );
		clientPort.SendPacket( serverAddress, msg.GetData(), msg.GetSize() );
		
		if ( idAsyncNetwork::LANServer.GetBool() ) {
			common->Printf( "net_LANServer is set, connecting in LAN mode\n" );
		} else {
			// emit a cd key authorization request
			// modified at protocol 1.37 for XP key addition
			msg.BeginWriting();
			msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
			msg.WriteString( "clAuth" );
			msg.WriteLong( ASYNC_PROTOCOL_VERSION );
			msg.WriteNetadr( serverAddress );
			// if we don't have a com_guid, this will request a direct reply from auth with it
			msg.WriteByte( cvarSystem->GetCVarString( "com_guid" )[0] ? 1 : 0 );
			// send the main key, and flag an extra byte to add XP key
			msg.WriteString( session->GetCDKey( false ) );
			const char *xpkey = session->GetCDKey( true );
			msg.WriteByte( xpkey ? 1 : 0 );
			if ( xpkey ) {
				msg.WriteString( xpkey );
			}
			clientPort.SendPacket( idAsyncNetwork::GetMasterAddress(), msg.GetData(), msg.GetSize() );
		}
	} else {
		return;
	}

	lastConnectTime = clientTime;
}

/*
==================
idAsyncClient::SendReliableGameMessage
==================
*/
void idAsyncClient::SendReliableGameMessage( const idBitMsg &msg ) {
	idBitMsg	outMsg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	if ( clientState < CS_INGAME ) {
		return;
	}

	outMsg.Init( msgBuf, sizeof( msgBuf ) );
	outMsg.WriteByte( CLIENT_RELIABLE_MESSAGE_GAME );
	outMsg.WriteData( msg.GetData(), msg.GetSize() );
	if ( !channel.SendReliableMessage( outMsg ) ) {
		common->Error( "client->server reliable messages overflow\n" );
	}
}

/*
==================
idAsyncClient::Idle
==================
*/
void idAsyncClient::Idle( void ) {
	// also need to read mouse for the connecting guis
	usercmdGen->GetDirectUsercmd();

	SendEmptyToServer();
}

/*
==================
idAsyncClient::UpdateTime
==================
*/
int idAsyncClient::UpdateTime( int clamp ) {
	int time, msec;

	time = Sys_Milliseconds();
	msec = idMath::ClampInt( 0, clamp, time - realTime );
	realTime = time;
	clientTime += msec;
	return msec;
}

/*
==================
idAsyncClient::RunFrame
==================
*/
void idAsyncClient::RunFrame( void ) {
	int			msec, size;
	bool		newPacket;
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];
	netadr_t	from;

	msec = UpdateTime( 100 );

	if ( !clientPort.GetPort() ) {
		return;
	}

	// handle ongoing pk4 downloads and patch downloads
	HandleDownloads();

	gameTimeResidual += msec;

	// spin in place processing incoming packets until enough time lapsed to run a new game frame
	do {

		do {

			// blocking read with game time residual timeout
			newPacket = clientPort.GetPacketBlocking( from, msgBuf, size, sizeof( msgBuf ), USERCMD_MSEC - ( gameTimeResidual + clientPredictTime ) - 1 );
			if ( newPacket ) {
				msg.Init( msgBuf, sizeof( msgBuf ) );
				msg.SetSize( size );
				msg.BeginReading();
				ProcessMessage( from, msg );
			}

			msec = UpdateTime( 100 );
			gameTimeResidual += msec;

		} while( newPacket );

	} while( gameTimeResidual + clientPredictTime < USERCMD_MSEC );

	// update server list
	serverList.RunFrame();

	if ( clientState == CS_DISCONNECTED ) {
		usercmdGen->GetDirectUsercmd();
		gameTimeResidual = USERCMD_MSEC - 1;
		clientPredictTime = 0;
		return;
	}

	if ( clientState == CS_PURERESTART ) {
		clientState = CS_DISCONNECTED;
		Reconnect();
		gameTimeResidual = USERCMD_MSEC - 1;
		clientPredictTime = 0;
		return;
	}

	// if not connected setup a connection
	if ( clientState < CS_CONNECTED ) {
		// also need to read mouse for the connecting guis
		usercmdGen->GetDirectUsercmd();
		SetupConnection();
		gameTimeResidual = USERCMD_MSEC - 1;
		clientPredictTime = 0;
		return;
	}

	if ( CheckTimeout() ) {
		return;
	}

	// if not yet in the game send empty messages to keep data flowing through the channel
	if ( clientState < CS_INGAME ) {
		Idle();
		gameTimeResidual = 0;
		return;
	}

	// check for user info changes
	if ( cvarSystem->GetModifiedFlags() & CVAR_USERINFO ) {
		game->ThrottleUserInfo( );
		SendUserInfoToServer( );
		game->SetUserInfo( clientNum, sessLocal.mapSpawnData.userInfo[ clientNum ], true, false );
		cvarSystem->ClearModifiedFlags( CVAR_USERINFO );
	}

	if ( gameTimeResidual + clientPredictTime >= USERCMD_MSEC ) {
		lastFrameDelta = 0;
	}

	// generate user commands for the predicted time
	while ( gameTimeResidual + clientPredictTime >= USERCMD_MSEC ) {

		// send the user commands of this client to the server
		SendUsercmdsToServer();

		// update time
		gameFrame++;
		gameTime += USERCMD_MSEC;
		gameTimeResidual -= USERCMD_MSEC;

		// run from the snapshot up to the local game frame
		while ( snapshotGameFrame < gameFrame ) {

			lastFrameDelta++;

			// duplicate usercmds for clients if no new ones are available
			DuplicateUsercmds( snapshotGameFrame, snapshotGameTime );

			// indicate the last prediction frame before a render
			bool lastPredictFrame = ( snapshotGameFrame + 1 >= gameFrame && gameTimeResidual + clientPredictTime < USERCMD_MSEC );

			// run client prediction
			gameReturn_t ret = game->ClientPrediction( clientNum, userCmds[ snapshotGameFrame & ( MAX_USERCMD_BACKUP - 1 ) ], lastPredictFrame );

			idAsyncNetwork::ExecuteSessionCommand( ret.sessionCommand );

			snapshotGameFrame++;
			snapshotGameTime += USERCMD_MSEC;
		}
	}
}

/*
==================
idAsyncClient::PacifierUpdate
==================
*/
void idAsyncClient::PacifierUpdate( void ) {
	if ( !IsActive() ) {
		return;
	}
	realTime = Sys_Milliseconds();
	SendEmptyToServer( false, true );
}

/*
==================
idAsyncClient::SendVersionCheck
==================
*/
void idAsyncClient::SendVersionCheck( bool fromMenu ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	if ( updateState != UPDATE_NONE && !fromMenu ) {
		common->DPrintf( "up-to-date check was already performed\n" );
		return;
	}

	InitPort();
	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	msg.WriteString( "versionCheck" );
	msg.WriteLong( ASYNC_PROTOCOL_VERSION );
	msg.WriteShort( BUILD_OS_ID );
	msg.WriteString( cvarSystem->GetCVarString( "si_version" ) );
	msg.WriteString( cvarSystem->GetCVarString( "com_guid" ) );
	clientPort.SendPacket( idAsyncNetwork::GetMasterAddress(), msg.GetData(), msg.GetSize() );

	common->DPrintf( "sent a version check request\n" );

	updateState = UPDATE_SENT;
	updateSentTime = clientTime;
	showUpdateMessage = fromMenu;
}

/*
==================
idAsyncClient::SendVersionDLUpdate

sending those packets is not strictly necessary. just a way to tell the update server
about what is going on. allows the update server to have a more precise view of the overall
network load for the updates
==================
*/
void idAsyncClient::SendVersionDLUpdate( int state ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	msg.WriteString( "versionDL" );
	msg.WriteLong( ASYNC_PROTOCOL_VERSION );
	msg.WriteShort( state );
	clientPort.SendPacket( idAsyncNetwork::GetMasterAddress(), msg.GetData(), msg.GetSize() );
}

/*
==================
idAsyncClient::HandleDownloads
==================
*/
void idAsyncClient::HandleDownloads( void ) {

	if ( updateState == UPDATE_SENT && clientTime > updateSentTime + 2000 ) {
		// timing out on no reply
		updateState = UPDATE_DONE;
		if ( showUpdateMessage ) {
			session->MessageBox( MSG_OK, common->GetLanguageDict()->GetString ( "#str_04839" ), common->GetLanguageDict()->GetString ( "#str_04837" ), true );
			showUpdateMessage = false;
		}
		common->DPrintf( "No update available\n" );
	} else if ( backgroundDownload.completed ) {
		// only enter these if the download slot is free
		if ( updateState == UPDATE_READY ) {
			// 
			if ( session->MessageBox( MSG_YESNO, updateMSG, common->GetLanguageDict()->GetString ( "#str_04330" ), true, "yes" )[0] ) {
				if ( !updateDirectDownload ) {
					sys->OpenURL( updateURL, true );
					updateState = UPDATE_DONE;
				} else {

					// we're just creating the file at toplevel inside fs_savepath
					updateURL.ExtractFileName( updateFile );
					idFile_Permanent *f = static_cast< idFile_Permanent *>( fileSystem->OpenFileWrite( updateFile ) );
					dltotal = 0;
					dlnow = 0;

					backgroundDownload.completed = false;
					backgroundDownload.opcode = DLTYPE_URL;
					backgroundDownload.f = f;
					backgroundDownload.url.status = DL_WAIT;
					backgroundDownload.url.dlnow = 0;
					backgroundDownload.url.dltotal = 0;
					backgroundDownload.url.url = updateURL;
					fileSystem->BackgroundDownload( &backgroundDownload );

					updateState = UPDATE_DLING;
					SendVersionDLUpdate( 0 );
					session->DownloadProgressBox( &backgroundDownload, va( "Downloading %s\n", updateFile.c_str() ) );
					updateState = UPDATE_DONE;
					if ( backgroundDownload.url.status == DL_DONE ) {				
						SendVersionDLUpdate( 1 );
						idStr fullPath = f->GetFullPath();
						fileSystem->CloseFile( f );
						if ( session->MessageBox( MSG_YESNO, common->GetLanguageDict()->GetString ( "#str_04331" ), common->GetLanguageDict()->GetString ( "#str_04332" ), true, "yes" )[0] ) {
							if ( updateMime == FILE_EXEC ) {
								sys->StartProcess( fullPath, true );
							} else {
								sys->OpenURL( va( "file://%s", fullPath.c_str() ), true );
							}
						} else {
							session->MessageBox( MSG_OK, va( common->GetLanguageDict()->GetString ( "#str_04333" ), fullPath.c_str() ), common->GetLanguageDict()->GetString ( "#str_04334" ), true );
						}
					} else {
						if ( backgroundDownload.url.dlerror[ 0 ] ) {
							common->Warning( "update download failed. curl error: %s", backgroundDownload.url.dlerror );
						}
						SendVersionDLUpdate( 2 );
						idStr name = f->GetName();
						fileSystem->CloseFile( f );
						fileSystem->RemoveFile( name );
						session->MessageBox( MSG_OK, common->GetLanguageDict()->GetString ( "#str_04335" ), common->GetLanguageDict()->GetString ( "#str_04336" ), true );
						if ( updateFallback.Length() ) {
							sys->OpenURL( updateFallback.c_str(), true );
						} else {
							common->Printf( "no fallback URL\n" );
						}
					}
				}
			} else {
				updateState = UPDATE_DONE;
			}
		} else if ( dlList.Num() ) {

			int numPaks = dlList.Num();
			int pakCount = 1;
			int progress_start, progress_end;
			currentDlSize = 0;

			do {

				if ( dlList[ 0 ].url[ 0 ] == '\0' ) {
					// ignore empty files
					dlList.RemoveIndex( 0 );
					continue;
				}
				common->Printf( "start download for %s\n", dlList[ 0 ].url.c_str() );

				idFile_Permanent *f = static_cast< idFile_Permanent *>( fileSystem->MakeTemporaryFile( ) );
				if ( !f ) {
					common->Warning( "could not create temporary file" );
					dlList.Clear();
					return;
				}

				backgroundDownload.completed = false;
				backgroundDownload.opcode = DLTYPE_URL;
				backgroundDownload.f = f;
				backgroundDownload.url.status = DL_WAIT;
				backgroundDownload.url.dlnow = 0;
				backgroundDownload.url.dltotal = dlList[ 0 ].size;
				backgroundDownload.url.url = dlList[ 0 ].url;
				fileSystem->BackgroundDownload( &backgroundDownload );
				idStr dltitle;
				// "Downloading %s"
				sprintf( dltitle, common->GetLanguageDict()->GetString( "#str_07213" ), dlList[ 0 ].filename.c_str() );
				if ( numPaks > 1 ) {
					dltitle += va( " (%d/%d)", pakCount, numPaks );
				}
				if ( totalDlSize ) {
					progress_start = (int)( (float)currentDlSize * 100.0f / (float)totalDlSize );
					progress_end = (int)( (float)( currentDlSize + dlList[ 0 ].size ) * 100.0f / (float)totalDlSize );
				} else {
					progress_start = 0;
					progress_end = 100;
				}
				session->DownloadProgressBox( &backgroundDownload, dltitle, progress_start, progress_end );
				if ( backgroundDownload.url.status == DL_DONE ) {				
					idFile		*saveas;
					const int	CHUNK_SIZE = 1024 * 1024;
					byte		*buf;
					int			remainlen;
					int			readlen;
					int			retlen;
					int			checksum;

					common->Printf( "file downloaded\n" );
					idStr finalPath = cvarSystem->GetCVarString( "fs_savepath" );
					finalPath.AppendPath( dlList[ 0 ].filename );
					fileSystem->CreateOSPath( finalPath );
					// do the final copy ourselves so we do by small chunks in case the file is big
					saveas = fileSystem->OpenExplicitFileWrite( finalPath );
					buf = (byte*)Mem_Alloc( CHUNK_SIZE );
					f->Seek( 0, FS_SEEK_END );
					remainlen = f->Tell();
					f->Seek( 0, FS_SEEK_SET );
					while ( remainlen ) {
						readlen = Min( remainlen, CHUNK_SIZE );
						retlen = f->Read( buf, readlen );
						if ( retlen != readlen ) {
							common->FatalError( "short read %d of %d in idFileSystem::HandleDownload", retlen, readlen );
						}
						retlen = saveas->Write( buf, readlen );
						if ( retlen != readlen ) {
							common->FatalError( "short write %d of %d in idFileSystem::HandleDownload", retlen, readlen );
						}
						remainlen -= readlen;
					}
					fileSystem->CloseFile( f );
					fileSystem->CloseFile( saveas );
					common->Printf( "saved as %s\n", finalPath.c_str() );
					Mem_Free( buf );
					
					// add that file to our paks list
					checksum = fileSystem->AddZipFile( dlList[ 0 ].filename );

					// verify the checksum to be what the server says
					if ( !checksum || checksum != dlList[ 0 ].checksum ) {
						// "pak is corrupted ( checksum 0x%x, expected 0x%x )"
						session->MessageBox( MSG_OK, va( common->GetLanguageDict()->GetString( "#str_07214" ) , checksum, dlList[0].checksum ), "Download failed", true );
						fileSystem->RemoveFile( dlList[ 0 ].filename );
						dlList.Clear();
						return;
					}

					currentDlSize += dlList[ 0 ].size;
					
				} else {
					common->Warning( "download failed: %s", dlList[ 0 ].url.c_str() );
					if ( backgroundDownload.url.dlerror[ 0 ] ) {
						common->Warning( "curl error: %s", backgroundDownload.url.dlerror );
					}
					// "The download failed or was cancelled"
					// "Download failed"
					session->MessageBox( MSG_OK, common->GetLanguageDict()->GetString( "#str_07215" ), common->GetLanguageDict()->GetString( "#str_07216" ), true );
					dlList.Clear();
					return;
				}

				pakCount++;
				dlList.RemoveIndex( 0 );			
			} while ( dlList.Num() );
			
			// all downloads successful - do the dew
			cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "reconnect\n" );
		}
	}
}

/*
===============
idAsyncClient::SendAuthCheck
===============
*/
bool idAsyncClient::SendAuthCheck( const char *cdkey, const char *xpkey ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_MESSAGE_SIZE];

	msg.Init( msgBuf, sizeof( msgBuf ) );
	msg.WriteShort( CONNECTIONLESS_MESSAGE_ID );
	msg.WriteString( "gameAuth" );
	msg.WriteLong( ASYNC_PROTOCOL_VERSION );
	msg.WriteByte( cdkey ? 1 : 0 );
	msg.WriteString( cdkey ? cdkey : "" );
	msg.WriteByte( xpkey ? 1 : 0 );
	msg.WriteString( xpkey ? xpkey : "" );
	InitPort();
	clientPort.SendPacket( idAsyncNetwork::GetMasterAddress(), msg.GetData(), msg.GetSize() );
	return true;
}

/*
===============
idAsyncClient::CheckTimeout
===============
*/
bool idAsyncClient::CheckTimeout( void ) {
	if ( lastPacketTime > 0 && ( lastPacketTime + idAsyncNetwork::clientServerTimeout.GetInteger()*1000 < clientTime ) ) {
		session->StopBox();
		session->MessageBox( MSG_OK, common->GetLanguageDict()->GetString ( "#str_04328" ), common->GetLanguageDict()->GetString ( "#str_04329" ), true );
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
		return true;
	}
	return false;
}

/*
===============
idAsyncClient::ProcessDownloadInfoMessage
===============
*/
void idAsyncClient::ProcessDownloadInfoMessage( const netadr_t from, const idBitMsg &msg ) {
	char			buf[ MAX_STRING_CHARS ];
	int				srvDlRequest = msg.ReadLong();
	int				infoType = msg.ReadByte();
	int				pakDl;
	int				pakIndex;
	
	pakDlEntry_t	entry;
	bool			gotAllFiles = true;
	idStr			sizeStr;
	bool			gotGame = false;

	if ( dlRequest == -1 || srvDlRequest != dlRequest ) {
		common->Warning( "bad download id from server, ignored" );
		return;
	}
	// mark the dlRequest as dead now whatever how we process it
	dlRequest = -1;

	if ( infoType == SERVER_DL_REDIRECT ) {
		msg.ReadString( buf, MAX_STRING_CHARS );
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
		// "You are missing required pak files to connect to this server.\nThe server gave a web page though:\n%s\nDo you want to go there now?"
		// "Missing required files"
		if ( session->MessageBox( MSG_YESNO, va( common->GetLanguageDict()->GetString( "#str_07217" ), buf ),
								  common->GetLanguageDict()->GetString( "#str_07218" ), true, "yes" )[ 0 ] ) {
			sys->OpenURL( buf, true );
		}
	} else if ( infoType == SERVER_DL_LIST ) {
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
		if ( dlList.Num() ) {
			common->Warning( "tried to process a download list while already busy downloading things" );
			return;
		}
		// read the URLs, check against what we requested, prompt for download
		pakIndex = -1;
		totalDlSize = 0;
		do {
			pakIndex++;
			pakDl = msg.ReadByte();
			if ( pakDl == SERVER_PAK_YES ) {
				if ( pakIndex == 0 ) {
					gotGame = true;
				}
				msg.ReadString( buf, MAX_STRING_CHARS );
				entry.filename = buf;
				msg.ReadString( buf, MAX_STRING_CHARS );
				entry.url = buf;
				entry.size = msg.ReadLong();
				// checksums are not transmitted, we read them from the dl request we sent
				entry.checksum = dlChecksums[ pakIndex ];
				totalDlSize += entry.size;
				dlList.Append( entry );
				common->Printf( "download %s from %s ( 0x%x )\n", entry.filename.c_str(), entry.url.c_str(), entry.checksum );
			} else if ( pakDl == SERVER_PAK_NO ) {
				msg.ReadString( buf, MAX_STRING_CHARS );
				entry.filename = buf;
				entry.url = "";
				entry.size = 0;
				entry.checksum = 0;
				dlList.Append( entry );
				// first pak is game pak, only fail it if we actually requested it
				if ( pakIndex != 0 || dlChecksums[ 0 ] != 0 ) {
					common->Printf( "no download offered for %s ( 0x%x )\n", entry.filename.c_str(), dlChecksums[ pakIndex ] );
					gotAllFiles = false;
				}
			} else {
				assert( pakDl == SERVER_PAK_END );
			}			
		} while ( pakDl != SERVER_PAK_END );
		if ( dlList.Num() < dlCount ) {
			common->Printf( "%d files were ignored by the server\n", dlCount - dlList.Num() );
			gotAllFiles = false;
		}
		sizeStr.BestUnit( "%.2f", totalDlSize, MEASURE_SIZE );
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
		if ( totalDlSize == 0 ) {
			// was no downloadable stuff for us
			// "Can't connect to the pure server: no downloads offered"
			// "Missing required files"
			dlList.Clear();
			session->MessageBox( MSG_OK, common->GetLanguageDict()->GetString( "#str_07219" ), common->GetLanguageDict()->GetString( "#str_07218" ), true );
			return;
		}
		bool asked = false;
		if ( gotGame ) {
			asked = true;
			// "You need to download game code to connect to this server. Are you sure? You should only answer yes if you trust the server administrators."
			// "Missing game binaries"
			if ( !session->MessageBox( MSG_YESNO, common->GetLanguageDict()->GetString( "#str_07220" ), common->GetLanguageDict()->GetString( "#str_07221" ), true, "yes" )[ 0 ] ) {
				dlList.Clear();
				return;
			}
		}
		if ( !gotAllFiles ) {
			asked = true;
			// "The server only offers to download some of the files required to connect ( %s ). Download anyway?"
			// "Missing required files"
			if ( !session->MessageBox( MSG_YESNO, va( common->GetLanguageDict()->GetString( "#str_07222" ), sizeStr.c_str() ),
									   common->GetLanguageDict()->GetString( "#str_07218" ), true, "yes" )[ 0 ] ) {
				dlList.Clear();
				return;
			}
		}
		if ( !asked && idAsyncNetwork::clientDownload.GetInteger() == 1 ) {
			// "You need to download some files to connect to this server ( %s ), proceed?"
			// "Missing required files"
			if ( !session->MessageBox( MSG_YESNO, va( common->GetLanguageDict()->GetString( "#str_07224" ), sizeStr.c_str() ),
									   common->GetLanguageDict()->GetString( "#str_07218" ), true, "yes" )[ 0 ] ) {
				dlList.Clear();
				return;
			}
		}
	} else {
		cmdSystem->BufferCommandText( CMD_EXEC_NOW, "disconnect" );
		// "You are missing some files to connect to this server, and the server doesn't provide downloads."
		// "Missing required files"
		session->MessageBox( MSG_OK, common->GetLanguageDict()->GetString( "#str_07223" ), common->GetLanguageDict()->GetString( "#str_07218" ), true );
	}
}

/*
===============
idAsyncClient::GetDownloadRequest
===============
*/
int idAsyncClient::GetDownloadRequest( const int checksums[ MAX_PURE_PAKS ], int count, int gamePakChecksum ) {
	assert( !checksums[ count ] ); // 0-terminated
	if ( memcmp( dlChecksums + 1, checksums, sizeof( int ) * count ) || gamePakChecksum != dlChecksums[ 0 ] ) {
		idRandom newreq;

		dlChecksums[ 0 ] = gamePakChecksum;
		memcpy( dlChecksums + 1, checksums, sizeof( int ) * MAX_PURE_PAKS );

		newreq.SetSeed( Sys_Milliseconds() );
		dlRequest = newreq.RandomInt();
		dlCount = count + ( gamePakChecksum ? 1 : 0 );
		return dlRequest;
	}
	// this is the same dlRequest, we haven't heard from the server. keep the same id
	return dlRequest;
}

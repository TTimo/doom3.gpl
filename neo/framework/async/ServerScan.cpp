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

idCVar gui_filter_password( "gui_filter_password", "0", CVAR_GUI | CVAR_INTEGER | CVAR_ARCHIVE, "Password filter" );
idCVar gui_filter_players( "gui_filter_players", "0", CVAR_GUI | CVAR_INTEGER | CVAR_ARCHIVE, "Players filter" );
idCVar gui_filter_gameType( "gui_filter_gameType", "0", CVAR_GUI | CVAR_INTEGER | CVAR_ARCHIVE, "Gametype filter" );
idCVar gui_filter_idle( "gui_filter_idle", "0", CVAR_GUI | CVAR_INTEGER | CVAR_ARCHIVE, "Idle servers filter" );
idCVar gui_filter_game( "gui_filter_game", "0", CVAR_GUI | CVAR_INTEGER | CVAR_ARCHIVE, "Game filter" );

const char* l_gameTypes[] = {
	"Deathmatch",
	"Tourney",
	"Team DM",
	"Last Man",
	"CTF",
	NULL
};

static idServerScan *l_serverScan = NULL;

/*
================
idServerScan::idServerScan
================
*/
idServerScan::idServerScan( ) {
	m_pGUI = NULL;
	m_sort = SORT_PING;
	m_sortAscending = true;
	challenge = 0;
	LocalClear();
}

/*
================
idServerScan::LocalClear
================
*/
void idServerScan::LocalClear( ) {
	scan_state = IDLE;
	incoming_net = false;
	lan_pingtime = -1;
	net_info.Clear();
	net_servers.Clear();
	cur_info = 0;
	if ( listGUI ) {
		listGUI->Clear();
	}
	incoming_useTimeout = false;
	m_sortedServers.Clear();
}

/*
================
idServerScan::Clear
================
*/
void idServerScan::Clear( ) {
	LocalClear();
	idList<networkServer_t>::Clear();
}

/*
================
idServerScan::Shutdown
================
*/
void idServerScan::Shutdown( ) {
	m_pGUI = NULL;
	if ( listGUI ) {
		listGUI->Config( NULL, NULL );
		uiManager->FreeListGUI( listGUI );
		listGUI = NULL;
	}
	screenshot.Clear();
}

/*
================
idServerScan::SetupLANScan
================
*/
void idServerScan::SetupLANScan( ) {	
	Clear();
	GUIUpdateSelected();
	scan_state = LAN_SCAN;
	challenge++;
	lan_pingtime = Sys_Milliseconds();
	common->DPrintf( "SetupLANScan with challenge %d\n", challenge );
}

/*
================
idServerScan::InfoResponse
================
*/
int idServerScan::InfoResponse( networkServer_t &server ) {
	if ( scan_state == IDLE ) {
		return false;
	}

	idStr serv = Sys_NetAdrToString( server.adr );

	if ( server.challenge != challenge ) {
		common->DPrintf( "idServerScan::InfoResponse - ignoring response from %s, wrong challenge %d.", serv.c_str(), server.challenge );
		return false;
	}

	if ( scan_state == NET_SCAN ) {	
		const idKeyValue *info = net_info.FindKey( serv.c_str() );
		if ( !info ) {
			common->DPrintf( "idServerScan::InfoResponse NET_SCAN: reply from unknown %s\n", serv.c_str() );
			return false;
		}
		int id = atoi( info->GetValue() );
		net_info.Delete( serv.c_str() );
		inServer_t iserv = net_servers[ id ];
		server.ping = Sys_Milliseconds() - iserv.time;
		server.id = iserv.id;
	} else {
		server.ping = Sys_Milliseconds() - lan_pingtime;
		server.id = 0;

		// check for duplicate servers
		for ( int i = 0; i < Num() ; i++ ) {
			if ( memcmp( &(*this)[ i ].adr, &server.adr, sizeof(netadr_t) ) == 0 ) {
				common->DPrintf( "idServerScan::InfoResponse LAN_SCAN: duplicate server %s\n", serv.c_str() );
				return true;
			}
		}
	}

	const char *si_map = server.serverInfo.GetString( "si_map" );
	const idDecl *mapDecl = declManager->FindType( DECL_MAPDEF, si_map, false );
	const idDeclEntityDef *mapDef = static_cast< const idDeclEntityDef * >( mapDecl );
	if ( mapDef ) {
		const char *mapName = common->GetLanguageDict()->GetString( mapDef->dict.GetString( "name", si_map ) );
		server.serverInfo.Set( "si_mapName", mapName );
	} else {
		server.serverInfo.Set( "si_mapName", si_map );
	}

	int index = Append( server );
	// for now, don't maintain sorting when adding new info response servers
	m_sortedServers.Append( Num()-1 );
	if ( listGUI->IsConfigured( ) && !IsFiltered( server ) ) {
		GUIAdd( Num()-1, server );
	}
	if ( listGUI->GetSelection( NULL, 0 ) == ( Num()-1 ) ) {
		GUIUpdateSelected();
	}

	return index;
}

/*
================
idServerScan::AddServer
================
*/
void idServerScan::AddServer( int id, const char *srv ) {
	inServer_t s;
	
	incoming_net = true;
	incoming_lastTime = Sys_Milliseconds() + INCOMING_TIMEOUT;
	s.id = id;
	
	// using IPs, not hosts
	if ( !Sys_StringToNetAdr( srv, &s.adr, false ) ) {
		common->DPrintf( "idServerScan::AddServer: failed to parse server %s\n", srv );
		return;
	}
	if ( !s.adr.port ) {
		s.adr.port = PORT_SERVER;
	}
	
	net_servers.Append( s );
}

/*
================
idServerScan::EndServers
================
*/
void idServerScan::EndServers( ) {
	incoming_net = false;
	l_serverScan = this;
	m_sortedServers.Sort( idServerScan::Cmp );
	ApplyFilter();
} 

/*
================
idServerScan::StartServers
================
*/
void idServerScan::StartServers( bool timeout ) {
	incoming_net = true;
	incoming_useTimeout = timeout;
	incoming_lastTime = Sys_Milliseconds() + REFRESH_START;
}

/*
================
idServerScan::EmitGetInfo
================
*/
void idServerScan::EmitGetInfo( netadr_t &serv ) {
	idAsyncNetwork::client.GetServerInfo( serv );
}

/*
===============
idServerScan::GetChallenge
===============
*/
int idServerScan::GetChallenge( ) {
	return challenge;
}

/*
================
idServerScan::NetScan
================
*/
void idServerScan::NetScan( ) {
	if ( !idAsyncNetwork::client.IsPortInitialized() ) {
		// if the port isn't open, initialize it, but wait for a short
		// time to let the OS do whatever magic things it needs to do...
		idAsyncNetwork::client.InitPort();
		// start the scan one second from now...
		scan_state = WAIT_ON_INIT;
		endWaitTime = Sys_Milliseconds() + 1000;
		return;
	}

	// make sure the client port is open
	idAsyncNetwork::client.InitPort();

	scan_state = NET_SCAN;
	challenge++;
	
	idList<networkServer_t>::Clear();
	m_sortedServers.Clear();
	cur_info = 0;
	net_info.Clear();
	listGUI->Clear();
	GUIUpdateSelected();
	common->DPrintf( "NetScan with challenge %d\n", challenge );
	
	while ( cur_info < Min( net_servers.Num(), MAX_PINGREQUESTS ) ) {
		netadr_t serv = net_servers[ cur_info ].adr;
		EmitGetInfo( serv );
		net_servers[ cur_info ].time = Sys_Milliseconds();
		net_info.SetInt( Sys_NetAdrToString( serv ), cur_info );
		cur_info++;
	}
}

/*
===============
idServerScan::ServerScanFrame
===============
*/
void idServerScan::RunFrame( ) {
	if ( scan_state == IDLE ) {
		return;
	} 
	
	if ( scan_state == WAIT_ON_INIT ) {
		if ( Sys_Milliseconds() >= endWaitTime ) {
				scan_state = IDLE;
				NetScan();
			}
		return;
	} 
	
	int timeout_limit = Sys_Milliseconds() - REPLY_TIMEOUT;
	
	if ( scan_state == LAN_SCAN ) {
		if ( timeout_limit > lan_pingtime ) {
			common->Printf( "Scanned for servers on the LAN\n" );
			scan_state = IDLE;
		}
		return;
	}
	
	// if scan_state == NET_SCAN
	
	// check for timeouts
	int i = 0;
	while ( i < net_info.GetNumKeyVals() ) {
		if ( timeout_limit > net_servers[ atoi( net_info.GetKeyVal( i )->GetValue().c_str() ) ].time ) {
			common->DPrintf( "timeout %s\n", net_info.GetKeyVal( i )->GetKey().c_str() );
			net_info.Delete( net_info.GetKeyVal( i )->GetKey().c_str() );
		} else {
			i++;
		}
	}
			
	// possibly send more queries
	while ( cur_info < net_servers.Num() && net_info.GetNumKeyVals() < MAX_PINGREQUESTS ) {
		netadr_t serv = net_servers[ cur_info ].adr;
		EmitGetInfo( serv );
		net_servers[ cur_info ].time = Sys_Milliseconds();
		net_info.SetInt( Sys_NetAdrToString( serv ), cur_info );
		cur_info++;
	}
	
	// update state
	if ( ( !incoming_net || ( incoming_useTimeout && Sys_Milliseconds() > incoming_lastTime ) ) && net_info.GetNumKeyVals() == 0 ) {
		EndServers();
		// the list is complete, we are no longer waiting for any getInfo replies
		common->Printf( "Scanned %d servers.\n", cur_info );
		scan_state = IDLE;
	}
}

/*
===============
idServerScan::GetBestPing
===============
*/
bool idServerScan::GetBestPing( networkServer_t &serv ) {
	int i, ic;
	ic = Num();
	if ( !ic ) {
		return false;
	}
	serv = (*this)[ 0 ];
	for ( i = 0 ; i < ic ; i++ ) {
		if ( (*this)[ i ].ping < serv.ping ) {
			serv = (*this)[ i ];
		}
	}
	return true;
}

/*
================
idServerScan::GUIConfig
================
*/
void idServerScan::GUIConfig( idUserInterface *pGUI, const char *name ) {
	m_pGUI = pGUI;
	if ( listGUI == NULL ) {
		listGUI = uiManager->AllocListGUI();
	}
	listGUI->Config( pGUI, name );
}

/*
================
idServerScan::GUIUpdateSelected
================
*/
void idServerScan::GUIUpdateSelected( void ) {
	char screenshot[ MAX_STRING_CHARS ];

	if ( !m_pGUI ) {
		return;
	}
	int i = listGUI->GetSelection( NULL, 0 );
	if ( i == -1 || i >= Num() ) {
		m_pGUI->SetStateString( "server_name", "" );
		m_pGUI->SetStateString( "player1", "" );
		m_pGUI->SetStateString( "player2", "" );
		m_pGUI->SetStateString( "player3", "" );
		m_pGUI->SetStateString( "player4", "" );
		m_pGUI->SetStateString( "player5", "" );
		m_pGUI->SetStateString( "player6", "" );
		m_pGUI->SetStateString( "player7", "" );
		m_pGUI->SetStateString( "player8", "" );
		m_pGUI->SetStateString( "server_map", "" );
		m_pGUI->SetStateString( "browser_levelshot", "" );
		m_pGUI->SetStateString( "server_gameType", "" );
		m_pGUI->SetStateString( "server_IP", "" );
		m_pGUI->SetStateString( "server_passworded", "" );
	} else {
		m_pGUI->SetStateString( "server_name", (*this)[ i ].serverInfo.GetString( "si_name" ) );
		for ( int j = 0; j < 8; j++ ) {
			if ( (*this)[i].clients > j ) {
				m_pGUI->SetStateString( va( "player%i", j + 1 ) , (*this)[ i ].nickname[ j ] );
			} else {
				m_pGUI->SetStateString( va( "player%i", j + 1 ) , "" );
			}
		}
		m_pGUI->SetStateString( "server_map", (*this)[ i ].serverInfo.GetString( "si_mapName" ) );
		fileSystem->FindMapScreenshot( (*this)[ i ].serverInfo.GetString( "si_map" ), screenshot, MAX_STRING_CHARS );
		m_pGUI->SetStateString( "browser_levelshot", screenshot );
		m_pGUI->SetStateString( "server_gameType", (*this)[ i ].serverInfo.GetString( "si_gameType" ) );
		m_pGUI->SetStateString( "server_IP", Sys_NetAdrToString( (*this)[ i ].adr ) );
		if ( (*this)[ i ].serverInfo.GetBool( "si_usePass" ) ) {
			m_pGUI->SetStateString( "server_passworded", "PASSWORD REQUIRED" );
		} else {
			m_pGUI->SetStateString( "server_passworded", "" );
		}
	}
}

/*
================
idServerScan::GUIAdd
================
*/
void idServerScan::GUIAdd( int id, const networkServer_t server ) {
	idStr name = server.serverInfo.GetString( "si_name", GAME_NAME " Server" );
	bool d3xp = false;
	bool mod = false;

	if ( !idStr::Icmp( server.serverInfo.GetString( "fs_game" ), "d3xp" ) ||
		 !idStr::Icmp( server.serverInfo.GetString( "fs_game_base" ), "d3xp" ) ) {
		d3xp = true;
	}
	if ( server.serverInfo.GetString( "fs_game" )[ 0 ] != '\0' ) {
		mod = true;
	}

	name += "\t";
	if ( server.serverInfo.GetString( "sv_punkbuster" )[ 0 ] == '1' ) {
		name += "mtr_PB";
	}

	name += "\t";
	if ( d3xp ) {
		// FIXME: even for a 'D3XP mod'
		// could have a specific icon for this case
		name += "mtr_doom3XPIcon";
	} else if ( mod ) {
		name += "mtr_doom3Mod";
	} else {
		name += "mtr_doom3Icon";
	}
	name += "\t";
	name += va( "%i/%i\t", server.clients, server.serverInfo.GetInt( "si_maxPlayers" ) );
	name += ( server.ping > -1 ) ? va( "%i\t", server.ping ) : "na\t";
	name += server.serverInfo.GetString( "si_gametype" );
	name += "\t";
	name += server.serverInfo.GetString( "si_mapName" );
	name += "\t";
	listGUI->Add( id, name );
}

/*
================
idServerScan::ApplyFilter
================
*/
void idServerScan::ApplyFilter( ) {
	int i;
	networkServer_t serv;
	idStr s;

	listGUI->SetStateChanges( false );
	listGUI->Clear();
	for ( i = m_sortAscending ? 0 : m_sortedServers.Num() - 1;
			m_sortAscending ? i < m_sortedServers.Num() : i >= 0;
			m_sortAscending ? i++ : i-- ) {
		serv = (*this)[ m_sortedServers[ i ] ];
		if ( !IsFiltered( serv ) ) {
			GUIAdd( m_sortedServers[ i ], serv );
		}
	}
	GUIUpdateSelected();
	listGUI->SetStateChanges( true );
}

/*
================
idServerScan::IsFiltered
================
*/
bool idServerScan::IsFiltered( const networkServer_t server ) {
	int i;
	const idKeyValue *keyval;

	// OS support filter
#if 0
	// filter out pure servers that won't provide checksumed game code for client OS
	keyval = server.serverInfo.FindKey( "si_pure" );
	if ( keyval && !idStr::Cmp( keyval->GetValue(), "1" ) ) {
		if ( ( server.OSMask & ( 1 << BUILD_OS_ID ) ) == 0 ) {
			return true;
		}
	}
#else
	if ( ( server.OSMask & ( 1 << BUILD_OS_ID ) ) == 0 ) {
		return true;
	}
#endif
	// password filter
	keyval = server.serverInfo.FindKey( "si_usePass" );
	if ( keyval && gui_filter_password.GetInteger() == 1 ) {
		// show passworded only
		if ( keyval->GetValue()[ 0 ] == '0' ) {
			return true;
		}
	} else if ( keyval && gui_filter_password.GetInteger() == 2 ) {
		// show no password only
		if ( keyval->GetValue()[ 0 ] != '0' ) {
			return true;
		}
	}
	// players filter
	keyval = server.serverInfo.FindKey( "si_maxPlayers" );
	if ( keyval ) {
		if ( gui_filter_players.GetInteger() == 1 && server.clients == atoi( keyval->GetValue() ) ) {
			return true;
		} else if ( gui_filter_players.GetInteger() == 2 && ( !server.clients || server.clients == atoi( keyval->GetValue() ) ) ) {
			return true;
		}
	}
	// gametype filter
	keyval = server.serverInfo.FindKey( "si_gameType" );
	if ( keyval && gui_filter_gameType.GetInteger() ) {
		i = 0;
		while ( l_gameTypes[ i ] ) {
			if ( !keyval->GetValue().Icmp( l_gameTypes[ i ] ) ) {
				break;
			}
			i++;
		}
		if ( l_gameTypes[ i ] && i != gui_filter_gameType.GetInteger() -1 ) {
			return true;
		}
	}
	// idle server filter
	keyval = server.serverInfo.FindKey( "si_idleServer" );
	if ( keyval && !gui_filter_idle.GetInteger() ) {
		if ( !keyval->GetValue().Icmp( "1" ) ) {
			return true;
		}
	}

	// autofilter D3XP games if the user does not has the XP installed
	if(!fileSystem->HasD3XP() && !idStr::Icmp(server.serverInfo.GetString( "fs_game" ), "d3xp")) {
		return true;
	}

	// filter based on the game doom or XP
	if(gui_filter_game.GetInteger() == 1) { //Only Doom
		if(idStr::Icmp(server.serverInfo.GetString("fs_game"), "")) {
			return true;
		}
	} else if(gui_filter_game.GetInteger() == 2) { //Only D3XP
		if(idStr::Icmp(server.serverInfo.GetString("fs_game"), "d3xp")) {
			return true;
		}
	}

	return false;
}

/*
================
idServerScan::Cmp
================
*/

int idServerScan::Cmp( const int *a, const int *b ) {
	networkServer_t serv1, serv2;
	idStr s1, s2;
	int ret;

	serv1 = (*l_serverScan)[ *a ];
	serv2 = (*l_serverScan)[ *b ];
	switch ( l_serverScan->m_sort ) {
		case SORT_PING:
			ret = serv1.ping < serv2.ping ? -1 : ( serv1.ping > serv2.ping ? 1 : 0 );
			return ret;
		case SORT_SERVERNAME:
			serv1.serverInfo.GetString( "si_name", "", s1 );
			serv2.serverInfo.GetString( "si_name", "", s2 );
			return s1.IcmpNoColor( s2 );
		case SORT_PLAYERS:
			ret = serv1.clients < serv2.clients ? -1 : ( serv1.clients > serv2.clients ? 1 : 0 );
			return ret;
		case SORT_GAMETYPE:
			serv1.serverInfo.GetString( "si_gameType", "", s1 );
			serv2.serverInfo.GetString( "si_gameType", "", s2 );
			return s1.Icmp( s2 );
		case SORT_MAP:
			serv1.serverInfo.GetString( "si_mapName", "", s1 );
			serv2.serverInfo.GetString( "si_mapName", "", s2 );
			return s1.Icmp( s2 );
		case SORT_GAME:
			serv1.serverInfo.GetString( "fs_game", "", s1 );
			serv2.serverInfo.GetString( "fs_game", "", s2 );
			return s1.Icmp( s2 );
	}
	return 0;
}

/*
================
idServerScan::SetSorting
================
*/
void idServerScan::SetSorting( serverSort_t sort ) {
	l_serverScan = this;
	if ( sort == m_sort ) {
		m_sortAscending = !m_sortAscending;
	} else {
		m_sort = sort;
		m_sortAscending = true; // is the default for any new sort
		m_sortedServers.Sort( idServerScan::Cmp );
	}
	// trigger a redraw
	ApplyFilter();
}


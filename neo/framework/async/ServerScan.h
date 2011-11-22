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

#ifndef __SERVERSCAN_H__
#define __SERVERSCAN_H__

/*
===============================================================================

	Scan for servers, on the LAN or from a list
	Update a listDef GUI through usage of idListGUI class
	When updating large lists of servers, sends out getInfo in small batches to avoid congestion

===============================================================================
*/

// storage for incoming servers / server scan
typedef struct {
	netadr_t	adr;
	int			id;
	int			time;
} inServer_t;

// the menu gui uses a hard-coded control type to display a list of network games
typedef struct {
	netadr_t	adr;
	idDict		serverInfo;
	int			ping;
	int			id;			// idnet mode sends an id for each server in list
	int			clients;
	char		nickname[ MAX_NICKLEN ][ MAX_ASYNC_CLIENTS ];
	short		pings[ MAX_ASYNC_CLIENTS ];
	int			rate[ MAX_ASYNC_CLIENTS ];
	int			OSMask;
    int			challenge;
} networkServer_t;

typedef enum {
	SORT_PING,
	SORT_SERVERNAME,
	SORT_PLAYERS,
	SORT_GAMETYPE,
	SORT_MAP,
	SORT_GAME
} serverSort_t;

class idServerScan : public idList<networkServer_t> {
public:	
						idServerScan( );
	
	int					InfoResponse( networkServer_t &server );

	// add an internet server - ( store a numeric id along with it )
	void				AddServer( int id, const char *srv );

	// we are going to feed server entries to be pinged
	// if timeout is true, use a timeout once we start AddServer to trigger EndServers and decide the scan is done
	void				StartServers( bool timeout );
	// we are done filling up the list of server entries
	void				EndServers( );

	// scan the current list of servers - used for refreshes and while receiving a fresh list
	void				NetScan( );

	// clear
	void				Clear( );

	// called each game frame. Updates the scanner state, takes care of ongoing scans
	void				RunFrame( );
	
	typedef enum {
		IDLE = 0,
		WAIT_ON_INIT,
		LAN_SCAN,
		NET_SCAN
	} scan_state_t;

	scan_state_t		GetState() { return scan_state; }
	void				SetState( scan_state_t );
	
	bool				GetBestPing( networkServer_t &serv );

						// prepare for a LAN scan. idAsyncClient does the network job (UDP broadcast), we do the storage
	void				SetupLANScan( );

	void				GUIConfig( idUserInterface *pGUI, const char *name );
						// update the GUI fields with information about the currently selected server
	void				GUIUpdateSelected( void );

	void				Shutdown( );

	void				ApplyFilter( );

						// there is an internal toggle, call twice with same sort to switch
	void				SetSorting( serverSort_t sort );

	int					GetChallenge( );

private:
	static const int	MAX_PINGREQUESTS 	= 32;		// how many servers to query at once
	static const int	REPLY_TIMEOUT 		= 999;		// how long should we wait for a reply from a game server
	static const int	INCOMING_TIMEOUT	= 1500;		// when we got an incoming server list, how long till we decide the list is done
	static const int	REFRESH_START		= 10000;	// how long to wait when sending the initial refresh request

	scan_state_t		scan_state;
	
	bool				incoming_net;	// set to true while new servers are fed through AddServer
	bool				incoming_useTimeout;
	int					incoming_lastTime;
	
	int					lan_pingtime;	// holds the time of LAN scan
	
						// servers we're waiting for a reply from
						// won't exceed MAX_PINGREQUESTS elements
						// holds index of net_servers elements, indexed by 'from' string
	idDict				net_info;		

	idList<inServer_t>	net_servers;
						// where we are in net_servers list for getInfo emissions ( NET_SCAN only )
						// we may either be waiting on MAX_PINGREQUESTS, or for net_servers to grow some more ( through AddServer )
	int					cur_info;

	idUserInterface		*m_pGUI;
	idListGUI *			listGUI;

	serverSort_t		m_sort;
	bool				m_sortAscending;
	idList<int>			m_sortedServers;	// use ascending for the walking order

	idStr				screenshot;
	int					challenge;			// challenge for current scan
	
	int					endWaitTime;		// when to stop waiting on a port init

private:
	void				LocalClear( );		// we need to clear some internal data as well

	void				EmitGetInfo( netadr_t &serv );
	void				GUIAdd( int id, const networkServer_t server );
	bool				IsFiltered( const networkServer_t server );

	static int			Cmp( const int *a, const int *b );
};

#endif /* !__SERVERSCAN_H__ */

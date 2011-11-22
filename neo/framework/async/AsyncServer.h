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

#ifndef __ASYNCSERVER_H__
#define __ASYNCSERVER_H__

/*
===============================================================================

  Network Server for asynchronous networking.

===============================================================================
*/

// MAX_CHALLENGES is made large to prevent a denial of service attack that could cycle
// all of them out before legitimate users connected
const int MAX_CHALLENGES				= 1024;

// if we don't hear from authorize server, assume it is down
const int AUTHORIZE_TIMEOUT				= 5000;

// states for the server's authorization process
typedef enum {
	CDK_WAIT = 0,	// we are waiting for a confirm/deny from auth
					// this is subject to timeout if we don't hear from auth
					// or a permanent wait if auth said so
	CDK_OK,
	CDK_ONLYLAN,
	CDK_PUREWAIT,
	CDK_PUREOK,
	CDK_MAXSTATES
} authState_t;

// states from the auth server, while the client is in CDK_WAIT
typedef enum {
	AUTH_NONE = 0,	// no reply yet
	AUTH_OK,		// this client is good
	AUTH_WAIT,		// wait - keep sending me srvAuth though
	AUTH_DENY,		// denied - don't send me anything about this client anymore
	AUTH_MAXSTATES
} authReply_t;

// message from auth to be forwarded back to the client
// some are locally hardcoded to save space, auth has the possibility to send a custom reply
typedef enum {
	AUTH_REPLY_WAITING = 0,	// waiting on an initial reply from auth
	AUTH_REPLY_UNKNOWN,		// client unknown to auth
	AUTH_REPLY_DENIED,		// access denied
	AUTH_REPLY_PRINT,		// custom message
	AUTH_REPLY_SRVWAIT,		// auth server replied and tells us he's working on it
	AUTH_REPLY_MAXSTATES
} authReplyMsg_t;

typedef struct challenge_s {
	netadr_t			address;		// client address
	int					clientId;		// client identification
	int					challenge;		// challenge code
	int					time;			// time the challenge was created
	int					pingTime;		// time the challenge response was sent to client
	bool				connected;		// true if the client is connected
	authState_t			authState;		// local state regarding the client
	authReply_t			authReply;		// cd key check replies
	authReplyMsg_t		authReplyMsg;	// default auth messages
	idStr				authReplyPrint;	// custom msg
	char				guid[12];		// guid
	int					OS;
} challenge_t;

typedef enum {
	SCS_FREE,			// can be reused for a new connection
	SCS_ZOMBIE,			// client has been disconnected, but don't reuse connection for a couple seconds
	SCS_PUREWAIT,		// client needs to update it's pure checksums before we can go further
	SCS_CONNECTED,		// client is connected
	SCS_INGAME			// client is in the game
} serverClientState_t;

typedef struct serverClient_s {
	int					OS;
	int					clientId;
	serverClientState_t	clientState;
	int					clientPrediction;
	int					clientAheadTime;
	int					clientRate;
	int					clientPing;

	int					gameInitSequence;
	int					gameFrame;
	int					gameTime;

	idMsgChannel		channel;
	int					lastConnectTime;
	int					lastEmptyTime;
	int					lastPingTime;
	int					lastSnapshotTime;
	int					lastPacketTime;
	int					lastInputTime;
	int					snapshotSequence;
	int					acknowledgeSnapshotSequence;
	int					numDuplicatedUsercmds;

	char				guid[12];  // Even Balance - M. Quinn

} serverClient_t;


class idAsyncServer {
public:
						idAsyncServer();

	bool				InitPort( void );
	void				ClosePort( void );
	void				Spawn( void );
	void				Kill( void );
	void				ExecuteMapChange( void );

	int					GetPort( void ) const;
	netadr_t			GetBoundAdr( void ) const;
	bool				IsActive( void ) const { return active; }
	int					GetDelay( void ) const { return gameTimeResidual; }
	int					GetOutgoingRate( void ) const;
	int					GetIncomingRate( void ) const;
	bool				IsClientInGame( int clientNum ) const;
	int					GetClientPing( int clientNum ) const;
	int					GetClientPrediction( int clientNum ) const;
	int					GetClientTimeSinceLastPacket( int clientNum ) const;
	int					GetClientTimeSinceLastInput( int clientNum ) const;
	int					GetClientOutgoingRate( int clientNum ) const;
	int					GetClientIncomingRate( int clientNum ) const;
	float				GetClientOutgoingCompression( int clientNum ) const;
	float				GetClientIncomingCompression( int clientNum ) const;
	float				GetClientIncomingPacketLoss( int clientNum ) const;
	int					GetNumClients( void ) const;
	int					GetNumIdleClients( void ) const;
	int					GetLocalClientNum( void ) const { return localClientNum; }

	void				RunFrame( void );
	void				ProcessConnectionLessMessages( void );
	void				RemoteConsoleOutput( const char *string );
	void				SendReliableGameMessage( int clientNum, const idBitMsg &msg );
	void				SendReliableGameMessageExcluding( int clientNum, const idBitMsg &msg );
	void				LocalClientSendReliableMessage( const idBitMsg &msg );

	void				MasterHeartbeat( bool force = false );
	void				DropClient( int clientNum, const char *reason );

	void				PacifierUpdate( void );

	void				UpdateUI( int clientNum );

	void				UpdateAsyncStatsAvg( void );
	void				GetAsyncStatsAvgMsg( idStr &msg );

	void				PrintLocalServerInfo( void );

private:
	bool				active;						// true if server is active
	int					realTime;					// absolute time

	int					serverTime;					// local server time
	idPort				serverPort;					// UDP port
	int					serverId;					// server identification
	int					serverDataChecksum;			// checksum of the data used by the server
	int					localClientNum;				// local client on listen server

	challenge_t			challenges[MAX_CHALLENGES];	// to prevent invalid IPs from connecting
	serverClient_t		clients[MAX_ASYNC_CLIENTS];	// clients
	usercmd_t			userCmds[MAX_USERCMD_BACKUP][MAX_ASYNC_CLIENTS];

	int					gameInitId;					// game initialization identification
	int					gameFrame;					// local game frame
	int					gameTime;					// local game time
	int					gameTimeResidual;			// left over time from previous frame

	netadr_t			rconAddress;
	
	int					nextHeartbeatTime;
	int					nextAsyncStatsTime;

	bool				serverReloadingEngine;		// flip-flop to not loop over when net_serverReloadEngine is on

	bool				noRconOutput;				// for default rcon response when command is silent

	int					lastAuthTime;				// global for auth server timeout

	// track the max outgoing rate over the last few secs to watch for spikes
	// dependent on net_serverSnapshotDelay. 50ms, for a 3 seconds backlog -> 60 samples
	static const int	stats_numsamples = 60;
	int					stats_outrate[ stats_numsamples ];
	int					stats_current;
	int					stats_average_sum;
	int					stats_max;
	int					stats_max_index;

	void				PrintOOB( const netadr_t to, int opcode, const char *string );
	void				DuplicateUsercmds( int frame, int time );
	void				ClearClient( int clientNum );
	void				InitClient( int clientNum, int clientId, int clientRate );
	void				InitLocalClient( int clientNum );
	void				BeginLocalClient( void );
	void				LocalClientInput( void );
	void				CheckClientTimeouts( void );
	void				SendPrintBroadcast( const char *string );
	void				SendPrintToClient( int clientNum, const char *string );
	void				SendUserInfoBroadcast( int userInfoNum, const idDict &info, bool sendToAll = false );
	void				SendUserInfoToClient( int clientNum, int userInfoNum, const idDict &info );
	void				SendSyncedCvarsBroadcast( const idDict &cvars );
	void				SendSyncedCvarsToClient( int clientNum, const idDict &cvars );
	void				SendApplySnapshotToClient( int clientNum, int sequence );
	bool				SendEmptyToClient( int clientNum, bool force = false );
	bool				SendPingToClient( int clientNum );
	void				SendGameInitToClient( int clientNum );
	bool				SendSnapshotToClient( int clientNum );
	void				ProcessUnreliableClientMessage( int clientNum, const idBitMsg &msg );
	void				ProcessReliableClientMessages( int clientNum );
	void				ProcessChallengeMessage( const netadr_t from, const idBitMsg &msg );
	void				ProcessConnectMessage( const netadr_t from, const idBitMsg &msg );
	void				ProcessRemoteConsoleMessage( const netadr_t from, const idBitMsg &msg );
	void				ProcessGetInfoMessage( const netadr_t from, const idBitMsg &msg );
	bool				ConnectionlessMessage( const netadr_t from, const idBitMsg &msg );
	bool				ProcessMessage( const netadr_t from, idBitMsg &msg );
	void				ProcessAuthMessage( const idBitMsg &msg );
	bool				SendPureServerMessage( const netadr_t to, int OS );										// returns false if no pure paks on the list
	void				ProcessPureMessage( const netadr_t from, const idBitMsg &msg );
	int					ValidateChallenge( const netadr_t from, int challenge, int clientId );	// returns -1 if validate failed
	bool				SendReliablePureToClient( int clientNum );
	void				ProcessReliablePure( int clientNum, const idBitMsg &msg );
	bool				VerifyChecksumMessage( int clientNum, const netadr_t *from, const idBitMsg &msg, idStr &reply, int OS ); // if from is NULL, clientNum is used for error messages
	void				SendReliableMessage( int clientNum, const idBitMsg &msg );				// checks for overflow and disconnects the faulty client
	int					UpdateTime( int clamp );
	void				SendEnterGameToClient( int clientNum );
	void				ProcessDownloadRequestMessage( const netadr_t from, const idBitMsg &msg );
};

#endif /* !__ASYNCSERVER_H__ */

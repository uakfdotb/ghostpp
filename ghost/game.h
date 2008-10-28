/*

   Copyright [2008] [Trevor Hogan]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   CODE PORTED FROM THE ORIGINAL GHOST PROJECT: http://ghost.pwner.org/

*/

#ifndef GAME_H
#define GAME_H

#include "gameslot.h"

//
// CBaseGame
//

class CTCPServer;
class CGameProtocol;
class CPotentialPlayer;
class CGamePlayer;
class CMap;
class CIncomingJoinPlayer;
class CIncomingAction;
class CIncomingChatPlayer;
class CIncomingMapSize;

class CBaseGame
{
public:
	CGHost *m_GHost;

protected:
	CTCPServer *m_Socket;						// listening socket
	CGameProtocol *m_Protocol;					// game protocol
	vector<CGameSlot> m_Slots;					// vector of slots
	vector<CPotentialPlayer *> m_Potentials;	// vector of potential players (connections that haven't sent a W3GS_REQJOIN packet yet)
	vector<CGamePlayer *> m_Players;			// vector of players
	queue<CIncomingAction *> m_Actions;			// queue of actions to be sent
	vector<string> m_Reserved;					// vector of player names with reserved slots (from the !hold command)
	CMap *m_Map;								// map data (this is a pointer to global data)
	bool m_Exiting;								// set to true and this class will be deleted next update
	uint16_t m_HostPort;						// the port to host games on
	unsigned char m_GameState;					// game state, public or private
	string m_GameName;							// game name
	string m_VirtualHostName;					// virtual host's name
	string m_OwnerName;							// name of the player who owns this game (should be considered an admin)
	string m_CreatorName;						// name of the player who created this game
	string m_CreatorServer;						// battle.net server the player who created this game was on
	string m_AnnounceMessage;					// a message to be sent every m_AnnounceInterval seconds
	uint32_t m_HostCounter;						// a unique game number
	uint32_t m_Latency;							// the number of ms to wait between sending action packets (we queue any received during this time)
	uint32_t m_SyncLimit;						// the maximum number of packets a player can fall out of sync before starting the lag screen
	uint32_t m_MaxSyncCounter;					// the largest number of keepalives received from any one player (for determining if anyone is lagging)
	uint32_t m_CreationTime;					// GetTime when the game was created
	uint32_t m_LastPingTime;					// GetTime when the last ping was sent
	uint32_t m_LastRefreshTime;					// GetTime when the last game refresh was sent
	uint32_t m_LastAnnounceTime;				// GetTime when the last announce message was sent
	uint32_t m_AnnounceInterval;				// how many seconds to wait between sending the m_AnnounceMessage
	uint32_t m_LastCountDownTicks;				// GetTicks when the last countdown message was sent
	uint32_t m_CountDownCounter;				// the countdown is finished when this reaches zero
	uint32_t m_StartedLoadingTicks;				// GetTicks when the game started loading
	uint32_t m_StartedLoadingTime;				// GetTime when the game started loading
	uint32_t m_StartPlayers;					// number of players when the game started
	uint32_t m_LastActionSentTicks;				// GetTicks when the last action packet was sent
	uint32_t m_LastLagScreenTime;				// GetTime when the last lag screen was active
	bool m_Locked;								// if the game owner is the only one allowed to run game commands or not
	bool m_RefreshMessages;						// if we should display "game refreshed..." messages or not
	bool m_MuteAll;								// if we should stop forwarding ingame chat messages targeted for all players or not
	bool m_MuteLobby;							// if we should stop forwarding lobby chat messages
	bool m_CountDownStarted;					// if the game start countdown has started or not
	bool m_GameLoading;							// if the game is currently loading or not
	bool m_GameLoaded;							// if the game has loaded or not
	bool m_Lagging;								// if the lag screen is active or not

public:
	CBaseGame( CGHost *nGHost, CMap *nMap, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nOwnerName, string nCreatorName, string nCreatorServer );
	virtual ~CBaseGame( );

	virtual uint16_t GetHostPort( )				{ return m_HostPort; }
	virtual unsigned char GetGameState( )		{ return m_GameState; }
	virtual string GetGameName( )				{ return m_GameName; }
	virtual string GetVirtualHostName( )		{ return m_VirtualHostName; }
	virtual string GetOwnerName( )				{ return m_OwnerName; }
	virtual string GetCreatorName( )			{ return m_CreatorName; }
	virtual string GetCreatorServer( )			{ return m_CreatorServer; }
	virtual uint32_t GetLastLagScreenTime( )	{ return m_LastLagScreenTime; }
	virtual bool GetLocked( )					{ return m_Locked; }
	virtual bool GetRefreshMessages( )			{ return m_RefreshMessages; }
	virtual bool GetCountDownStarted( )			{ return m_CountDownStarted; }
	virtual bool GetGameLoading( )				{ return m_GameLoading; }
	virtual bool GetGameLoaded( )				{ return m_GameLoaded; }
	virtual bool GetLagging( )					{ return m_Lagging; }

	virtual void SetExiting( bool nExiting )	{ m_Exiting = nExiting; }

	virtual uint32_t GetSlotsOpen( );
	virtual uint32_t GetNumPlayers( );
	virtual string GetDescription( );

	// processing functions

	virtual unsigned int SetFD( void *fd, int *nfds );
	virtual bool Update( void *fd );
	virtual void UpdatePost( );

	// generic functions to send packets to players

	virtual void Send( CGamePlayer *player, BYTEARRAY data );
	virtual void Send( unsigned char PID, BYTEARRAY data );
	virtual void Send( BYTEARRAY PIDs, BYTEARRAY data );
	virtual void SendAll( BYTEARRAY data );

	// functions to send packets to players

	virtual void SendChat( unsigned char fromPID, CGamePlayer *player, string message );
	virtual void SendChat( unsigned char fromPID, unsigned char toPID, string message );
	virtual void SendChat( CGamePlayer *player, string message );
	virtual void SendChat( unsigned char toPID, string message );
	virtual void SendAllChat( unsigned char fromPID, string message );
	virtual void SendAllChat( string message );
	virtual void SendAllSlotInfo( );
	virtual void SendWelcomeMessage( CGamePlayer *player );

	// events
	// note: these are only called while iterating through the m_Potentials or m_Players vectors
	// therefore you can't modify those vectors and must use the player's m_DeleteMe member to flag for deletion

	virtual void EventPlayerDeleted( CGamePlayer *player );
	virtual void EventPlayerJoined( CPotentialPlayer *player, CIncomingJoinPlayer *joinPlayer );
	virtual void EventPlayerLeft( CGamePlayer *player );
	virtual void EventPlayerLoaded( CGamePlayer *player );
	virtual void EventPlayerAction( CGamePlayer *player, CIncomingAction *action );
	virtual void EventPlayerChatToHost( CGamePlayer *player, CIncomingChatPlayer *chatPlayer );
	virtual void EventPlayerBotCommand( CGamePlayer *player, string command, string payload );
	virtual void EventPlayerChangeTeam( CGamePlayer *player, unsigned char team );
	virtual void EventPlayerChangeColour( CGamePlayer *player, unsigned char colour );
	virtual void EventPlayerChangeRace( CGamePlayer *player, unsigned char race );
	virtual void EventPlayerChangeHandicap( CGamePlayer *player, unsigned char handicap );
	virtual void EventPlayerDropRequest( CGamePlayer *player );
	virtual void EventPlayerMapSize( CGamePlayer *player, CIncomingMapSize *mapSize );
	virtual void EventPlayerPongToHost( CGamePlayer *player, uint32_t pong );

	// these events are called outside of any iterations

	virtual void EventGameStarted( );
	virtual void EventGameLoaded( );

	// other functions

	virtual unsigned char GetSIDFromPID( unsigned char PID );
	virtual CGamePlayer *GetPlayerFromPID( unsigned char PID );
	virtual CGamePlayer *GetPlayerFromSID( unsigned char SID );
	virtual CGamePlayer *GetPlayerFromName( string name, bool sensitive );
	virtual uint32_t GetPlayerFromNamePartial( string name, CGamePlayer **player );
	virtual CGamePlayer *GetPlayerFromColour( unsigned char colour );
	virtual unsigned char GetNewPID( );
	virtual unsigned char GetNewColour( );
	virtual BYTEARRAY GetPIDs( );
	virtual BYTEARRAY GetPIDs( unsigned char excludePID );
	virtual unsigned char GetHostPID( );
	virtual unsigned char GetEmptySlot( bool reserved );
	virtual unsigned char GetEmptySlot( unsigned char team );
	virtual unsigned char GetEmptySlot( unsigned char team, unsigned char PID );
	virtual void SwapSlots( unsigned char SID1, unsigned char SID2 );
	virtual void OpenSlot( unsigned char SID, bool kick );
	virtual void CloseSlot( unsigned char SID, bool kick );
	virtual void ComputerSlot( unsigned char SID, unsigned char skill, bool kick );
	virtual void ColourSlot( unsigned char SID, unsigned char colour );
	virtual void OpenAllSlots( );
	virtual void CloseAllSlots( );
	virtual void ShuffleSlots( );
	virtual void AddToSpoofed( string server, string name, bool sendMessage );
	virtual void AddToReserved( string name );
	virtual bool IsOwner( string name );
	virtual bool IsReserved( string name );
	virtual bool IsDownloading( );
	virtual void StartCountDown( bool force );
	virtual void StopPlayers( string reason );
	virtual void StopLaggers( string reason );
	virtual void CreateVirtualHost( );
	virtual void DeleteVirtualHost( );
};

//
// CGame
//

class CDBBan;
class CDBGame;
class CDBGamePlayer;
class CStats;

class CGame : public CBaseGame
{
protected:
	vector<CDBBan *> m_DBBans;					// vector of potential ban data for the database (see the Update function for more info, it's not as straightforward as you might think)
	CDBGame *m_DBGame;							// potential game data for the database
	vector<CDBGamePlayer *> m_DBGamePlayers;	// vector of potential gameplayer data for the database
	CStats *m_Stats;							// class to keep track of game stats such as kills/deaths/assists in dota
	uint32_t m_GameOverTime;					// GetTime when the game was over as reported by the stats class

public:
	CGame( CGHost *nGHost, CMap *nMap, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nOwnerName, string nCreatorName, string nCreatorServer );
	virtual ~CGame( );

	virtual void EventPlayerDeleted( CGamePlayer *player );
	virtual void EventPlayerAction( CGamePlayer *player, CIncomingAction *action );
	virtual void EventPlayerBotCommand( CGamePlayer *player, string command, string payload );
	virtual void EventGameStarted( );
};

//
// CAdminGame
//

typedef pair<string,uint32_t> TempBan;

class CAdminGame : public CBaseGame
{
protected:
	string m_Password;
	vector<TempBan> m_TempBans;

public:
	CAdminGame( CGHost *nGHost, CMap *nMap, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nPassword );
	virtual ~CAdminGame( );

	virtual void SendWelcomeMessage( CGamePlayer *player );
	virtual void EventPlayerJoined( CPotentialPlayer *player, CIncomingJoinPlayer *joinPlayer );
	virtual void EventPlayerBotCommand( CGamePlayer *player, string command, string payload );
};

#endif

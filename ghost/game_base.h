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

#ifndef GAME_BASE_H
#define GAME_BASE_H

#include "gameslot.h"

//
// CBaseGame
//

class CTCPServer;
class CGameProtocol;
class CPotentialPlayer;
class CGamePlayer;
class CMap;
class CSaveGame;
class CReplay;
class CIncomingJoinPlayer;
class CIncomingAction;
class CIncomingChatPlayer;
class CIncomingMapSize;
class CCallableScoreCheck;
struct QueuedSpoofAdd;

class CBaseGame
{
public:
	CGHost *m_GHost;

protected:
	CTCPServer *m_Socket;							// listening socket
	CGameProtocol *m_Protocol;						// game protocol
	std::vector<CGameSlot> m_Slots;						// std::vector of slots
	std::vector<CPotentialPlayer *> m_Potentials;		// std::vector of potential players (connections that haven't sent a W3GS_REQJOIN packet yet)
	std::vector<CGamePlayer *> m_Players;				// std::vector of players
	std::vector<CCallableScoreCheck *> m_ScoreChecks;
	std::queue<CIncomingAction *> m_Actions;				// std::queue of actions to be sent
	std::vector<std::string> m_Reserved;						// std::vector of player names with reserved slots (from the !hold command)
	std::set<std::string> m_IgnoredNames;						// set of player names to NOT print ban messages for when joining because they've already been printed
	std::set<std::string> m_IPBlackList;						// set of IP addresses to blacklist from joining (todotodo: convert to uint32's for efficiency)
	std::vector<CGameSlot> m_EnforceSlots;				// std::vector of slots to force players to use (used with saved games)
	std::vector<PIDPlayer> m_EnforcePlayers;				// std::vector of pids to force players to use (used with saved games)
	CMap *m_Map;									// map data
	CSaveGame *m_SaveGame;							// savegame data (this is a pointer to global data)
	CReplay *m_Replay;								// replay
	bool m_Exiting;									// set to true and this class will be deleted next update
	bool m_Saving;									// if we're currently saving game data to the database
	uint16_t m_HostPort;							// the port to host games on
	unsigned char m_GameState;						// game state, public or private
	unsigned char m_VirtualHostPID;					// virtual host's PID
	unsigned char m_FakePlayerPID;					// the fake player's PID (if present)
	unsigned char m_GProxyEmptyActions;
	std::string m_GameName;								// game name
	std::string m_LastGameName;							// last game name (the previous game name before it was rehosted)
	std::string m_VirtualHostName;						// virtual host's name
	std::string m_OwnerName;								// name of the player who owns this game (should be considered an admin)
	std::string m_CreatorName;							// name of the player who created this game
	std::string m_CreatorServer;							// battle.net server the player who created this game was on
	std::string m_AnnounceMessage;						// a message to be sent every m_AnnounceInterval seconds
	std::string m_StatString;							// the stat std::string when the game started (used when saving replays)
	std::string m_KickVotePlayer;						// the player to be kicked with the currently running kick vote
	std::string m_HCLCommandString;						// the "HostBot Command Library" command std::string, used to pass a limited amount of data to specially designed maps
	uint32_t m_RandomSeed;							// the random seed sent to the Warcraft III clients
	uint32_t m_HostCounter;							// a unique game number
	uint32_t m_EntryKey;							// random entry key for LAN, used to prove that a player is actually joining from LAN
	uint32_t m_Latency;								// the number of ms to wait between sending action packets (we std::queue any received during this time)
	uint32_t m_SyncLimit;							// the maximum number of packets a player can fall out of sync before starting the lag screen
	uint32_t m_SyncCounter;							// the number of actions sent so far (for determining if anyone is lagging)
	uint32_t m_GameTicks;							// ingame ticks
	uint32_t m_CreationTime;						// GetTime when the game was created
	uint32_t m_LastPingTime;						// GetTime when the last ping was sent
	uint32_t m_LastRefreshTime;						// GetTime when the last game refresh was sent
	uint32_t m_LastDownloadTicks;					// GetTicks when the last map download cycle was performed
	uint32_t m_DownloadCounter;						// # of map bytes downloaded in the last second
	uint32_t m_LastDownloadCounterResetTicks;		// GetTicks when the download counter was last reset
	uint32_t m_LastAnnounceTime;					// GetTime when the last announce message was sent
	uint32_t m_AnnounceInterval;					// how many seconds to wait between sending the m_AnnounceMessage
	uint32_t m_LastAutoStartTime;					// the last time we tried to auto start the game
	uint32_t m_AutoStartPlayers;					// auto start the game when there are this many players or more
	uint32_t m_LastCountDownTicks;					// GetTicks when the last countdown message was sent
	uint32_t m_CountDownCounter;					// the countdown is finished when this reaches zero
	uint32_t m_StartedLoadingTicks;					// GetTicks when the game started loading
	uint32_t m_StartPlayers;						// number of players when the game started
	uint32_t m_LastLagScreenResetTime;				// GetTime when the "lag" screen was last reset
	uint32_t m_LastActionSentTicks;					// GetTicks when the last action packet was sent
	uint32_t m_LastActionLateBy;					// the number of ticks we were late sending the last action packet by
	uint32_t m_StartedLaggingTime;					// GetTime when the last lag screen started
	uint32_t m_LastLagScreenTime;					// GetTime when the last lag screen was active (continuously updated)
	uint32_t m_LastReservedSeen;					// GetTime when the last reserved player was seen in the lobby
	uint32_t m_StartedKickVoteTime;					// GetTime when the kick vote was started
	uint32_t m_GameOverTime;						// GetTime when the game was over
	uint32_t m_LastPlayerLeaveTicks;				// GetTicks when the most recent player left the game
	double m_MinimumScore;							// the minimum allowed score for matchmaking mode
	double m_MaximumScore;							// the maximum allowed score for matchmaking mode
	bool m_SlotInfoChanged;							// if the slot info has changed and hasn't been sent to the players yet (optimization)
	bool m_Locked;									// if the game owner is the only one allowed to run game commands or not
	bool m_RefreshMessages;							// if we should display "game refreshed..." messages or not
	bool m_RefreshError;							// if there was an error refreshing the game
	bool m_RefreshRehosted;							// if we just rehosted and are waiting for confirmation that it was successful
	bool m_MuteAll;									// if we should stop forwarding ingame chat messages targeted for all players or not
	bool m_MuteLobby;								// if we should stop forwarding lobby chat messages
	bool m_CountDownStarted;						// if the game start countdown has started or not
	bool m_GameLoading;								// if the game is currently loading or not
	bool m_GameLoaded;								// if the game has loaded or not
	bool m_LoadInGame;								// if the load-in-game feature is enabled or not
	bool m_Lagging;									// if the lag screen is active or not
	bool m_AutoSave;								// if we should auto save the game before someone disconnects
	bool m_MatchMaking;								// if matchmaking mode is enabled
	bool m_LocalAdminMessages;						// if local admin messages should be relayed or not
	int m_DoDelete;									// notifies thread to exit
	uint32_t m_LastReconnectHandleTime;				// last time we tried to handle GProxy reconnects

public:
	std::vector<std::string> m_DoSayGames;					// std::vector of strings we should announce to the current game
	std::mutex m_SayGamesMutex;					// mutex for the above std::vector
	std::vector<QueuedSpoofAdd> m_DoSpoofAdd;			// std::vector of spoof add function call structures
	std::mutex m_SpoofAddMutex;

public:
	CBaseGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, std::string nGameName, std::string nOwnerName, std::string nCreatorName, std::string nCreatorServer );
	virtual ~CBaseGame( );

	virtual void loop( );
	virtual void doDelete( );
	virtual bool readyDelete( );

	virtual std::vector<CGameSlot> GetEnforceSlots( )	{ return m_EnforceSlots; }
	virtual std::vector<PIDPlayer> GetEnforcePlayers( )	{ return m_EnforcePlayers; }
	virtual CSaveGame *GetSaveGame( )				{ return m_SaveGame; }
	virtual uint16_t GetHostPort( )					{ return m_HostPort; }
	virtual unsigned char GetGameState( )			{ return m_GameState; }
	virtual unsigned char GetGProxyEmptyActions( )	{ return m_GProxyEmptyActions; }
	virtual std::string GetGameName( )					{ return m_GameName; }
	virtual std::string GetLastGameName( )				{ return m_LastGameName; }
	virtual std::string GetVirtualHostName( )			{ return m_VirtualHostName; }
	virtual std::string GetOwnerName( )					{ return m_OwnerName; }
	virtual std::string GetCreatorName( )				{ return m_CreatorName; }
	virtual std::string GetCreatorServer( )				{ return m_CreatorServer; }
	virtual uint32_t GetHostCounter( )				{ return m_HostCounter; }
	virtual uint32_t GetLastLagScreenTime( )		{ return m_LastLagScreenTime; }
	virtual bool GetLocked( )						{ return m_Locked; }
	virtual bool GetRefreshMessages( )				{ return m_RefreshMessages; }
	virtual bool GetCountDownStarted( )				{ return m_CountDownStarted; }
	virtual bool GetGameLoading( )					{ return m_GameLoading; }
	virtual bool GetGameLoaded( )					{ return m_GameLoaded; }
	virtual bool GetLagging( )						{ return m_Lagging; }

	virtual void SetEnforceSlots( std::vector<CGameSlot> nEnforceSlots )		{ m_EnforceSlots = nEnforceSlots; }
	virtual void SetEnforcePlayers( std::vector<PIDPlayer> nEnforcePlayers )	{ m_EnforcePlayers = nEnforcePlayers; }
	virtual void SetExiting( bool nExiting )							{ m_Exiting = nExiting; }
	virtual void SetAutoStartPlayers( uint32_t nAutoStartPlayers )		{ m_AutoStartPlayers = nAutoStartPlayers; }
	virtual void SetMinimumScore( double nMinimumScore )				{ m_MinimumScore = nMinimumScore; }
	virtual void SetMaximumScore( double nMaximumScore )				{ m_MaximumScore = nMaximumScore; }
	virtual void SetRefreshError( bool nRefreshError )					{ m_RefreshError = nRefreshError; }
	virtual void SetMatchMaking( bool nMatchMaking )					{ m_MatchMaking = nMatchMaking; }

	virtual uint32_t GetNextTimedActionTicks( );
	virtual uint32_t GetSlotsOccupied( );
	virtual uint32_t GetSlotsOpen( );
	virtual uint32_t GetNumPlayers( );
	virtual uint32_t GetNumHumanPlayers( );
	virtual std::string GetDescription( );

	virtual void SetAnnounce( uint32_t interval, std::string message );

	// processing functions

	virtual unsigned int SetFD( void *fd, void *send_fd, int *nfds );
	virtual bool Update( void *fd, void *send_fd );
	virtual void UpdatePost( void *send_fd );

	// generic functions to send packets to players

	virtual void Send( CGamePlayer *player, BYTEARRAY data );
	virtual void Send( unsigned char PID, BYTEARRAY data );
	virtual void Send( BYTEARRAY PIDs, BYTEARRAY data );
	virtual void SendAll( BYTEARRAY data );

	// functions to send packets to players

	virtual void SendChat( unsigned char fromPID, CGamePlayer *player, std::string message );
	virtual void SendChat( unsigned char fromPID, unsigned char toPID, std::string message );
	virtual void SendChat( CGamePlayer *player, std::string message );
	virtual void SendChat( unsigned char toPID, std::string message );
	virtual void SendAllChat( unsigned char fromPID, std::string message );
	virtual void SendAllChat( std::string message );
	virtual void SendLocalAdminChat( std::string message );
	virtual void SendAllSlotInfo( );
	virtual void SendVirtualHostPlayerInfo( CGamePlayer *player );
	virtual void SendFakePlayerInfo( CGamePlayer *player );
	virtual void SendAllActions( );
	virtual void SendWelcomeMessage( CGamePlayer *player );
	virtual void SendEndMessage( );

	// events
	// note: these are only called while iterating through the m_Potentials or m_Players vectors
	// therefore you can't modify those vectors and must use the player's m_DeleteMe member to flag for deletion

	virtual void EventPlayerDeleted( CGamePlayer *player );
	virtual void EventPlayerDisconnectTimedOut( CGamePlayer *player );
	virtual void EventPlayerDisconnectPlayerError( CGamePlayer *player );
	virtual void EventPlayerDisconnectSocketError( CGamePlayer *player );
	virtual void EventPlayerDisconnectConnectionClosed( CGamePlayer *player );
	virtual void EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer );
	virtual void EventPlayerJoinedWithScore( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer, double score );
	virtual void EventPlayerLeft( CGamePlayer *player, uint32_t reason );
	virtual void EventPlayerLoaded( CGamePlayer *player );
	virtual bool EventPlayerAction( CGamePlayer *player, CIncomingAction *action );
	virtual void EventPlayerKeepAlive( CGamePlayer *player, uint32_t checkSum );
	virtual void EventPlayerChatToHost( CGamePlayer *player, CIncomingChatPlayer *chatPlayer );
	virtual bool EventPlayerBotCommand( CGamePlayer *player, std::string command, std::string payload );
	virtual void EventPlayerChangeTeam( CGamePlayer *player, unsigned char team );
	virtual void EventPlayerChangeColour( CGamePlayer *player, unsigned char colour );
	virtual void EventPlayerChangeRace( CGamePlayer *player, unsigned char race );
	virtual void EventPlayerChangeHandicap( CGamePlayer *player, unsigned char handicap );
	virtual void EventPlayerDropRequest( CGamePlayer *player );
	virtual void EventPlayerMapSize( CGamePlayer *player, CIncomingMapSize *mapSize );
	virtual void EventPlayerPongToHost( CGamePlayer *player, uint32_t pong );

	// these events are called outside of any iterations

	virtual void EventGameRefreshed( std::string server );
	virtual void EventGameStarted( );
	virtual void EventGameLoaded( );

	// other functions

	virtual unsigned char GetSIDFromPID( unsigned char PID );
	virtual CGamePlayer *GetPlayerFromPID( unsigned char PID );
	virtual CGamePlayer *GetPlayerFromSID( unsigned char SID );
	virtual CGamePlayer *GetPlayerFromName( std::string name, bool sensitive );
	virtual uint32_t GetPlayerFromNamePartial( std::string name, CGamePlayer **player );
	virtual CGamePlayer *GetPlayerFromColour( unsigned char colour );
	virtual unsigned char GetNewPID( );
	virtual unsigned char GetNewColour( );
	virtual BYTEARRAY GetPIDs( );
	virtual BYTEARRAY GetPIDs( unsigned char excludePID );
	virtual unsigned char GetHostPID( );
	virtual unsigned char GetEmptySlot( bool reserved );
	virtual unsigned char GetEmptySlot( unsigned char team, unsigned char PID );
	virtual void SwapSlots( unsigned char SID1, unsigned char SID2 );
	virtual void OpenSlot( unsigned char SID, bool kick );
	virtual void CloseSlot( unsigned char SID, bool kick );
	virtual void ComputerSlot( unsigned char SID, unsigned char skill, bool kick );
	virtual void ColourSlot( unsigned char SID, unsigned char colour );
	virtual void OpenAllSlots( );
	virtual void CloseAllSlots( );
	virtual void ShuffleSlots( );
	virtual std::vector<unsigned char> BalanceSlotsRecursive( std::vector<unsigned char> PlayerIDs, unsigned char *TeamSizes, double *PlayerScores, unsigned char StartTeam );
	virtual void BalanceSlots( );
	virtual void AddToSpoofed( std::string server, std::string name, bool sendMessage );
	virtual void AddToReserved( std::string name );
	virtual bool IsOwner( std::string name );
	virtual bool IsReserved( std::string name );
	virtual bool IsDownloading( );
	virtual bool IsGameDataSaved( );
	virtual void SaveGameData( );
	virtual void StartCountDown( bool force );
	virtual void StartCountDownAuto( bool requireSpoofChecks );
	virtual void StopPlayers( std::string reason );
	virtual void StopLaggers( std::string reason );
	virtual void CreateVirtualHost( );
	virtual void DeleteVirtualHost( );
	virtual void CreateFakePlayer( );
	virtual void DeleteFakePlayer( );
};

struct QueuedSpoofAdd {
	std::string server;
	std::string name;
	bool sendMessage;
	std::string failMessage; //empty if no failure
};

#endif

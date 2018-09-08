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

#ifndef GHOST_H
#define GHOST_H

#include "includes.h"

//
// CGHost
//

class CUDPSocket;
class CTCPServer;
class CTCPSocket;
class CGPSProtocol;
class CCRC32;
class CSHA1;
class CBNET;
class CBaseGame;
class CAdminGame;
class CGHostDB;
class CBaseCallable;
class CLanguage;
class CMap;
class CSaveGame;
class CConfig;

struct GProxyReconnector {
	CTCPSocket *socket;
	unsigned char PID;
	uint32_t ReconnectKey;
	uint32_t LastPacket;
	uint32_t PostedTime;
};

class CGHost
{
public:
	CUDPSocket *m_UDPSocket;				// a UDP socket for sending broadcasts and other junk (used with !sendlan)
	CTCPServer *m_ReconnectSocket;			// listening socket for GProxy++ reliable reconnects
	std::vector<CTCPSocket *> m_ReconnectSockets;// std::vector of sockets attempting to reconnect (connected but not identified yet)
	CGPSProtocol *m_GPSProtocol;
	CCRC32 *m_CRC;							// for calculating CRC's
	CSHA1 *m_SHA;							// for calculating SHA1's
	std::vector<CBNET *> m_BNETs;				// all our battle.net connections (there can be more than one)
	CBaseGame *m_CurrentGame;				// this game is still in the lobby state
	CAdminGame *m_AdminGame;				// this "fake game" allows an admin who knows the password to control the bot from the local network
	std::vector<CBaseGame *> m_Games;			// these games are in progress
	std::mutex m_GamesMutex;
	CGHostDB *m_DB;							// database
	CGHostDB *m_DBLocal;					// local database (for temporary data)
	std::vector<CBaseCallable *> m_Callables;	// std::vector of orphaned callables waiting to die
	std::mutex m_CallablesMutex;
	std::vector<BYTEARRAY> m_LocalAddresses;		// std::vector of local IP addresses
	CLanguage *m_Language;					// language
	CMap *m_Map;							// the currently loaded map
	CMap *m_AdminMap;						// the map to use in the admin game
	CMap *m_AutoHostMap;					// the map to use when autohosting
	CSaveGame *m_SaveGame;					// the save game to use
	std::vector<PIDPlayer> m_EnforcePlayers;		// std::vector of pids to force players to use in the next game (used with saved games)
	bool m_Exiting;							// set to true to force ghost to shutdown next update (used by SignalCatcher)
	bool m_ExitingNice;						// set to true to force ghost to disconnect from all battle.net connections and wait for all games to finish before shutting down
	bool m_Enabled;							// set to false to prevent new games from being created
	std::string m_Version;						// GHost++ version std::string
	uint32_t m_HostCounter;					// the current host counter (a unique number to identify a game, incremented each time a game is created)
	std::string m_AutoHostGameName;				// the base game name to auto host with
	std::string m_AutoHostOwner;
	std::string m_AutoHostServer;
	uint32_t m_AutoHostMaximumGames;		// maximum number of games to auto host
	uint32_t m_AutoHostAutoStartPlayers;	// when using auto hosting auto start the game when this many players have joined
	uint32_t m_LastAutoHostTime;			// GetTime when the last auto host was attempted
	bool m_AutoHostMatchMaking;
	double m_AutoHostMinimumScore;
	double m_AutoHostMaximumScore;
	bool m_AllGamesFinished;				// if all games finished (used when exiting nicely)
	uint32_t m_AllGamesFinishedTime;		// GetTime when all games finished (used when exiting nicely)
	std::string m_LanguageFile;					// config value: language file
	std::string m_Warcraft3Path;					// config value: Warcraft 3 path
	bool m_TFT;								// config value: TFT enabled or not
	std::string m_BindAddress;					// config value: the address to host games on
	uint16_t m_HostPort;					// config value: the port to host games on
	bool m_Reconnect;						// config value: GProxy++ reliable reconnects enabled or not
	uint16_t m_ReconnectPort;				// config value: the port to listen for GProxy++ reliable reconnects on
	uint32_t m_ReconnectWaitTime;			// config value: the maximum number of minutes to wait for a GProxy++ reliable reconnect
	uint32_t m_MaxGames;					// config value: maximum number of games in progress
	char m_CommandTrigger;					// config value: the command trigger inside games
	std::string m_MapCFGPath;					// config value: map cfg path
	std::string m_SaveGamePath;					// config value: savegame path
	std::string m_MapPath;						// config value: map path
	bool m_SaveReplays;						// config value: save replays
	std::string m_ReplayPath;					// config value: replay path
	std::string m_VirtualHostName;				// config value: virtual host name
	bool m_HideIPAddresses;					// config value: hide IP addresses from players
	bool m_CheckMultipleIPUsage;			// config value: check for multiple IP address usage
	uint32_t m_SpoofChecks;					// config value: do automatic spoof checks or not
	bool m_RequireSpoofChecks;				// config value: require spoof checks or not
	bool m_ReserveAdmins;					// config value: consider admins to be reserved players or not
	bool m_RefreshMessages;					// config value: display refresh messages or not (by default)
	bool m_AutoLock;						// config value: auto lock games when the owner is present
	bool m_AutoSave;						// config value: auto save before someone disconnects
	uint32_t m_AllowDownloads;				// config value: allow map downloads or not
	bool m_PingDuringDownloads;				// config value: ping during map downloads or not
	uint32_t m_MaxDownloaders;				// config value: maximum number of map downloaders at the same time
	uint32_t m_MaxDownloadSpeed;			// config value: maximum total map download speed in KB/sec
	bool m_LCPings;							// config value: use LC style pings (divide actual pings by two)
	uint32_t m_AutoKickPing;				// config value: auto kick players with ping higher than this
	uint32_t m_BanMethod;					// config value: ban method (ban by name/ip/both)
	std::string m_IPBlackListFile;				// config value: IP blacklist file (ipblacklist.txt)
	uint32_t m_LobbyTimeLimit;				// config value: auto close the game lobby after this many minutes without any reserved players
	uint32_t m_Latency;						// config value: the latency (by default)
	uint32_t m_SyncLimit;					// config value: the maximum number of packets a player can fall out of sync before starting the lag screen (by default)
	bool m_VoteKickAllowed;					// config value: if votekicks are allowed or not
	uint32_t m_VoteKickPercentage;			// config value: percentage of players required to vote yes for a votekick to pass
	std::string m_DefaultMap;					// config value: default map (map.cfg)
	std::string m_MOTDFile;						// config value: motd.txt
	std::string m_GameLoadedFile;				// config value: gameloaded.txt
	std::string m_GameOverFile;					// config value: gameover.txt
	bool m_LocalAdminMessages;				// config value: send local admin messages or not
	bool m_AdminGameCreate;					// config value: create the admin game or not
	uint16_t m_AdminGamePort;				// config value: the port to host the admin game on
	std::string m_AdminGamePassword;				// config value: the admin game password
	std::string m_AdminGameMap;					// config value: the admin game map config to use
	unsigned char m_LANWar3Version;			// config value: LAN warcraft 3 version
	uint32_t m_ReplayWar3Version;			// config value: replay warcraft 3 version (for saving replays)
	uint32_t m_ReplayBuildNumber;			// config value: replay build number (for saving replays)
	bool m_TCPNoDelay;						// config value: use Nagle's algorithm or not
	uint32_t m_MatchMakingMethod;			// config value: the matchmaking method
	uint32_t m_MapGameType;					// config value: the MapGameType overwrite (aka: refresh hack)
	std::vector<GProxyReconnector *> m_PendingReconnects;
	std::mutex m_ReconnectMutex;

	CGHost( CConfig *CFG );
	~CGHost( );

	// processing functions

	bool Update( long usecBlock );

	// events

	void EventBNETConnecting( CBNET *bnet );
	void EventBNETConnected( CBNET *bnet );
	void EventBNETDisconnected( CBNET *bnet );
	void EventBNETLoggedIn( CBNET *bnet );
	void EventBNETGameRefreshed( CBNET *bnet );
	void EventBNETGameRefreshFailed( CBNET *bnet );
	void EventBNETConnectTimedOut( CBNET *bnet );
	void EventBNETWhisper( CBNET *bnet, std::string user, std::string message );
	void EventBNETChat( CBNET *bnet, std::string user, std::string message );
	void EventBNETEmote( CBNET *bnet, std::string user, std::string message );
	void EventGameDeleted( CBaseGame *game );

	// other functions

	void ReloadConfigs( );
	void SetConfigs( CConfig *CFG );
	void ExtractScripts( );
	void LoadIPToCountryData( );
	void CreateGame( CMap *map, unsigned char gameState, bool saveGame, std::string gameName, std::string ownerName, std::string creatorName, std::string creatorServer, bool whisper );
};

#endif

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

// standard integer sizes for 64 bit compatibility

#ifdef WIN32
 #include "ms_stdint.h"
#else
 #include <stdint.h>
#endif

// STL

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <queue>
#include <string>
#include <vector>

using namespace std;

typedef vector<unsigned char> BYTEARRAY;

// time

uint32_t GetTime( );		// seconds
uint32_t GetTicks( );		// milliseconds

#ifdef WIN32
 #define MILLISLEEP( x ) Sleep( x )
#else
 #define MILLISLEEP( x ) usleep( ( x ) * 1000 )
#endif

// network

#undef FD_SETSIZE
#define FD_SETSIZE 512

// output

void CONSOLE_Print( string message );
void DEBUG_Print( string message );
void DEBUG_Print( BYTEARRAY b );

//
// CGHost
//

class CUDPSocket;
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

class CGHost
{
public:
	CUDPSocket *m_UDPSocket;				// a UDP socket for sending broadcasts and other junk (used with !sendlan)
	CCRC32 *m_CRC;							// for calculating CRC's
	CSHA1 *m_SHA;							// for calculating SHA1's
	vector<CBNET *> m_BNETs;				// all our battle.net connections (there can be more than one)
	CBaseGame *m_CurrentGame;				// this game is still in the lobby state
	CAdminGame *m_AdminGame;				// this "fake game" allows an admin who knows the password to control the bot from the local network
	vector<CBaseGame *> m_Games;			// these games are in progress
	CGHostDB *m_DB;							// database
	CGHostDB *m_DBLocal;					// local database (for temporary data)
	vector<CBaseCallable *> m_Callables;	// vector of orphaned callables waiting to die
	CLanguage *m_Language;					// language
	CMap *m_Map;							// the currently loaded map (this is global data, CBaseGame just references it so don't modify it unless you know what you're doing)
	CMap *m_AdminMap;						// the map to use in the admin game
	CSaveGame *m_SaveGame;					// the save game to use
	bool m_Exiting;							// set to true to force ghost to shutdown next update (used by SignalCatcher)
	bool m_Enabled;							// set to false to prevent new games from being created
	string m_Version;						// GHost++ version string
	uint32_t m_HostCounter;					// the current host counter (a unique number to identify a game, incremented each time a game is created)
	string m_AutoHostGameName;				// the base game name to auto host with
	string m_AutoHostMapCFG;				// the map config to auto host with
	string m_AutoHostOwner;
	string m_AutoHostServer;
	uint32_t m_AutoHostMaximumGames;		// maximum number of games to auto host
	uint32_t m_AutoHostAutoStartPlayers;	// when using auto hosting auto start the game when this many players have joined
	uint32_t m_LastAutoHostTime;			// GetTime when the last auto host was attempted
	bool m_AutoHostMatchMaking;
	double m_AutoHostMinimumScore;
	double m_AutoHostMaximumScore;
	string m_LanguageFile;					// config value: language file
	string m_Warcraft3Path;					// config value: Warcraft 3 path
	string m_BindAddress;					// config value: the address to host games on
	uint16_t m_HostPort;					// config value: the port to host games on
	uint32_t m_MaxGames;					// config value: maximum number of games in progress
	char m_CommandTrigger;					// config value: the command trigger inside games
	string m_MapCFGPath;					// config value: map cfg path
	string m_SaveGamePath;					// config value: savegame path
	string m_MapPath;						// config value: map path
	bool m_SaveReplays;						// config value: save replays
	string m_ReplayPath;					// config value: replay path
	string m_VirtualHostName;				// config value: virtual host name
	bool m_HideIPAddresses;					// config value: hide IP addresses from players
	bool m_CheckMultipleIPUsage;			// config value: check for multiple IP address usage
	bool m_SpoofChecks;						// config value: do/require spoof checks or not
	bool m_RefreshMessages;					// config value: display refresh messages or not (by default)
	bool m_AutoLock;						// config value: auto lock games when the owner is present
	bool m_AutoSave;						// config value: auto save before someone disconnects
	uint32_t m_AllowDownloads;				// config value: allow map downloads or not
	bool m_PingDuringDownloads;				// config value: ping during map downloads or not
	uint32_t m_MaxDownloaders;				// config value: maximum number of map downloaders at the same time
	uint32_t m_MaxDownloadSpeed;			// config value: maximum total map download speed in KB/sec
	bool m_LCPings;							// config value: use LC style pings (divide actual pings by two)
	uint32_t m_AutoKickPing;				// config value: auto kick players with ping higher than this
	uint32_t m_LobbyTimeLimit;				// config value: auto close the game lobby after this many minutes without any reserved players
	uint32_t m_Latency;						// config value: the latency (by default)
	uint32_t m_SyncLimit;					// config value: the maximum number of packets a player can fall out of sync before starting the lag screen (by default)
	bool m_VoteKickAllowed;					// config value: if votekicks are allowed or not
	uint32_t m_VoteKickPercentage;			// config value: percentage of players required to vote yes for a votekick to pass
	string m_DefaultMap;					// config value: default map (map.cfg)
	string m_MOTDFile;						// config value: motd.txt
	string m_GameLoadedFile;				// config value: gameloaded.txt
	string m_GameOverFile;					// config value: gameover.txt
	bool m_AdminGameCreate;					// config value: create the admin game or not
	uint16_t m_AdminGamePort;				// config value: the port to host the admin game on
	string m_AdminGamePassword;				// config value: the admin game password

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
	void EventGameDeleted( CBaseGame *game );

	// other functions

	void ExtractScripts( );
	void LoadIPToCountryData( );
	void CreateGame( unsigned char gameState, bool saveGame, string gameName, string ownerName, string creatorName, string creatorServer, bool whisper );
};

#endif

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

uint32_t GetTime( );		// seconds since January 1, 1970 usually
uint32_t GetTicks( );		// milliseconds since computer startup usually, overflows after ~50 days

#ifdef WIN32
 #define MILLISLEEP( x ) Sleep( x )
#else
 #define MILLISLEEP( x ) usleep( ( x ) * 1000 )
#endif

// network

#undef FD_SETSIZE
#define FD_SETSIZE 512

// crc

class CCRC32;
extern CCRC32 *gCRC;

// output

void CONSOLE_Print( string message );
void DEBUG_Print( string message );
void DEBUG_Print( BYTEARRAY b );

//
// CGHost
//

class CUDPSocket;
class CBNET;
class CBaseGame;
class CAdminGame;
class CGHostDB;
class CLanguage;
class CMap;

class CGHost
{
public:
	CUDPSocket *m_UDPSocket;		// a UDP socket for sending broadcasts and other junk (used with !sendlan)
	vector<CBNET *> m_BNETs;		// all our battle.net connections (there can be more than one)
	CBaseGame *m_CurrentGame;		// this game is still in the lobby state
	CAdminGame *m_AdminGame;		// this "fake game" allows an admin who knows the password to control the bot from the local network
	vector<CBaseGame *> m_Games;	// these games are in progress
	CGHostDB *m_DB;					// database
	CLanguage *m_Language;			// language
	CMap *m_Map;					// the currently loaded map (this is global data, CBaseGame just references it so don't modify it unless you know what you're doing)
	CMap *m_AdminMap;				// the map to use in the admin game
	bool m_Exiting;					// set to true to force ghost to shutdown next update (used by SignalCatcher)
	string m_Version;				// GHost++ version string
	uint32_t m_HostCounter;			// the current host counter (a unique number to identify a game, incremented each time a game is created)
	string m_Warcraft3Path;			// config value: Warcraft 3 path
	uint16_t m_HostPort;			// config value: the port to host games on
	uint32_t m_MaxGames;			// config value: maximum number of games in progress
	char m_CommandTrigger;			// config value: the command trigger inside games
	string m_MapCFGPath;			// config value: map cfg path
	string m_MapPath;				// config value: map path
	bool m_SpoofChecks;				// config value: do/require spoof checks or not
	bool m_RefreshMessages;			// config value: display refresh messages or not (by default)
	bool m_AutoLock;				// config value: auto lock games when the owner is present
	uint32_t m_AllowDownloads;		// config value: allow map downloads or not
	bool m_PingDuringDownloads;		// config value: ping during map downloads or not
	bool m_LCPings;					// config value: use LC style pings (divide actual pings by two)
	uint32_t m_AutoKickPing;		// config value: auto kick players with ping higher than this
	uint32_t m_Latency;				// config value: the latency (by default)
	uint32_t m_SyncLimit;			// config value: the maximum number of packets a player can fall out of sync before starting the lag screen (by default)
	bool m_AdminGameCreate;			// config value: create the admin game or not
	uint16_t m_AdminGamePort;		// config value: the port to host the admin game on
	string m_AdminGamePassword;		// config value: the admin game password

	CGHost( );
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
	void CreateGame( unsigned char gameState, string gameName, string ownerName, string creatorName, string creatorServer, bool whisper );
};

#endif

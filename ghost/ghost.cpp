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

#include "ghost.h"
#include "util.h"
#include "crc32.h"
#include "csvparser.h"
#include "config.h"
#include "language.h"
#include "socket.h"
#include "user.h"
#include "ghostdb.h"
#include "ghostdbsqlite.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "gameprotocol.h"
#include "game.h"

#include <signal.h>
#include <stdlib.h>

#define __STORMLIB_SELF__
#include <stormlib/StormLib.h>

/*

#include "ghost.h"
#include "util.h"
#include "crc32.h"
#include "csvparser.h"
#include "config.h"
#include "language.h"
#include "socket.h"
#include "commandpacket.h"
#include "user.h"
#include "ghostdb.h"
#include "ghostdbsqlite.h"
#include "bncsutilinterface.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "replay.h"
#include "gameslot.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game.h"
#include "stats.h"
#include "statsdota.h"
#include "sqlite3.h"

*/

#ifdef WIN32
 #include <windows.h>
 #include <winsock.h>
#endif

#include <time.h>

#ifndef WIN32
 #include <sys/time.h>
#endif

time_t gStartTime;
string gLogFile;
CGHost *gGHost = NULL;

uint32_t GetTime( )
{
	return (uint32_t)( time( NULL ) - gStartTime );
}

uint32_t GetTicks( )
{
#ifdef WIN32
	return GetTickCount( );
#else
	uint32_t ticks;
	struct timeval now;
	gettimeofday( &now, NULL );
	ticks = now.tv_sec * 1000;
	ticks += now.tv_usec / 1000;
	return ticks;
#endif
}

void SignalCatcher( int signal )
{
	// signal( SIGABRT, SignalCatcher );
	// signal( SIGINT, SignalCatcher );

	CONSOLE_Print( "[!!!] caught signal, shutting down" );

	if( gGHost )
	{
		if( gGHost->m_Exiting )
			exit( 1 );
		else
			gGHost->m_Exiting = true;
	}
	else
		exit( 1 );
}

//
// main
//

int main( int argc, char **argv )
{
	string CFGFile = "ghost.cfg";

	if( argc > 1 && argv[1] )
		CFGFile = argv[1];

	// read config file

	CConfig CFG;
	CFG.Read( CFGFile );
	gLogFile = CFG.GetString( "bot_log", string( ) );

	// print something for logging purposes

	CONSOLE_Print( "[GHOST] starting up" );

	// catch SIGABRT and SIGINT

	signal( SIGABRT, SignalCatcher );
	signal( SIGINT, SignalCatcher );

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif

	// initialize the start time

	gStartTime = time( NULL );

#ifdef WIN32
	// initialize winsock

	CONSOLE_Print( "[GHOST] starting winsock" );
	WSADATA wsadata;

	if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
	{
		CONSOLE_Print( "[GHOST] error starting winsock" );
		return 1;
	}
#endif

	// initialize ghost

	gGHost = new CGHost( &CFG );

	while( 1 )
	{
		// block for 50ms on all sockets - if you intend to perform any timed actions more frequently you should change this
		// that said it's likely we'll loop more often than this due to there being data waiting on one of the sockets but there aren't any guarantees

		if( gGHost->Update( 50000 ) )
			break;
	}

	// shutdown ghost

	CONSOLE_Print( "[GHOST] shutting down" );
	delete gGHost;
	gGHost = NULL;

#ifdef WIN32
	// shutdown winsock

	CONSOLE_Print( "[GHOST] shutting down winsock" );
	WSACleanup( );
#endif

	return 0;
}

void CONSOLE_Print( string message )
{
	cout << message << endl;

	// logging

	if( !gLogFile.empty( ) )
	{
		ofstream Log;
		Log.open( gLogFile.c_str( ), ios :: app );

		if( !Log.fail( ) )
		{
			time_t Now = time( NULL );
			string Time = asctime( localtime( &Now ) );

			// erase the newline
			
			Time.erase( Time.size( ) - 1 );
			Log << "[" << Time << "] " << message << endl;
			Log.close( );
		}
	}
}

void DEBUG_Print( string message )
{
	cout << message << endl;
}

void DEBUG_Print( BYTEARRAY b )
{
	cout << "{ ";

	for( unsigned int i = 0; i < b.size( ); i++ )
		cout << hex << (int)b[i] << " ";

	cout << "}" << endl;
}

//
// CGHost
//

CGHost :: CGHost( CConfig *CFG )
{
	m_UDPSocket = new CUDPSocket( );
	m_CRC = new CCRC32( );
	m_CRC->Initialize( );
	m_CurrentGame = NULL;
	m_DB = new CGHostDBSQLite( CFG );
	m_Exiting = false;
	m_Enabled = true;
	m_Version = "11.01";
	m_HostCounter = 1;
	m_AutoHostMaximumGames = 0;
	m_AutoHostAutoStartPlayers = 0;
	m_LastAutoHostTime = 0;
	m_LanguageFile = CFG->GetString( "bot_language", "language.cfg" );
	m_Language = new CLanguage( m_LanguageFile );
	m_Warcraft3Path = CFG->GetString( "bot_war3path", "C:\\Program Files\\Warcraft III\\" );
	m_HostPort = CFG->GetInt( "bot_hostport", 6112 );
	m_MaxGames = CFG->GetInt( "bot_maxgames", 5 );
	string BotCommandTrigger = CFG->GetString( "bot_commandtrigger", "!" );

	if( BotCommandTrigger.empty( ) )
		BotCommandTrigger = "!";

	m_CommandTrigger = BotCommandTrigger[0];
	m_MapCFGPath = CFG->GetString( "bot_mapcfgpath", string( ) );
	m_SaveGamePath = CFG->GetString( "bot_savegamepath", string( ) );
	m_MapPath = CFG->GetString( "bot_mappath", string( ) );
	m_SaveReplays = CFG->GetInt( "bot_savereplays", 0 ) == 0 ? false : true;
	m_ReplayPath = CFG->GetString( "bot_replaypath", string( ) );
	m_SpoofChecks = CFG->GetInt( "bot_spoofchecks", 1 ) == 0 ? false : true;
	m_RefreshMessages = CFG->GetInt( "bot_refreshmessages", 0 ) == 0 ? false : true;
	m_AutoLock = CFG->GetInt( "bot_autolock", 0 ) == 0 ? false : true;
	m_AutoSave = CFG->GetInt( "bot_autosave", 0 ) == 0 ? false : true;
	m_AllowDownloads = CFG->GetInt( "bot_allowdownloads", 0 );
	m_PingDuringDownloads = CFG->GetInt( "bot_pingduringdownloads", 0 ) == 0 ? false : true;
	m_LCPings = CFG->GetInt( "bot_lcpings", 1 ) == 0 ? false : true;
	m_AutoKickPing = CFG->GetInt( "bot_autokickping", 400 );
	m_Latency = CFG->GetInt( "bot_latency", 100 );
	m_SyncLimit = CFG->GetInt( "bot_synclimit", 50 );
	m_AdminGameCreate = CFG->GetInt( "admingame_create", 0 ) == 0 ? false : true;
	m_AdminGamePort = CFG->GetInt( "admingame_port", 6113 );
	m_AdminGamePassword = CFG->GetString( "admingame_password", string( ) );

	// load the battle.net connections
	// we're just loading the config data and creating the CBNET classes here, the connections are established later (in the Update function)

	for( uint32_t i = 1; i < 10; i++ )
	{
		string Prefix;

		if( i == 1 )
			Prefix = "bnet_";
		else
			Prefix = "bnet" + UTIL_ToString( i ) + "_";

		string Server = CFG->GetString( Prefix + "server", string( ) );
		string CDKeyROC = CFG->GetString( Prefix + "cdkeyroc", string( ) );
		string CDKeyTFT = CFG->GetString( Prefix + "cdkeytft", string( ) );
		string UserName = CFG->GetString( Prefix + "username", string( ) );
		string UserPassword = CFG->GetString( Prefix + "password", string( ) );
		string FirstChannel = CFG->GetString( Prefix + "firstchannel", "The Void" );
		string RootAdmin = CFG->GetString( Prefix + "rootadmin", string( ) );
		string BNETCommandTrigger = CFG->GetString( Prefix + "commandtrigger", "!" );

		if( BNETCommandTrigger.empty( ) )
			BNETCommandTrigger = "!";

		bool HoldFriends = CFG->GetInt( Prefix + "holdfriends", 1 ) == 0 ? false : true;
		bool HoldClan = CFG->GetInt( Prefix + "holdclan", 1 ) == 0 ? false : true;
		unsigned char War3Version = CFG->GetInt( Prefix + "custom_war3version", 22 );
		BYTEARRAY EXEVersion = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversion", string( ) ), 4 );
		BYTEARRAY EXEVersionHash = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversionhash", string( ) ), 4 );
		string PasswordHashType = CFG->GetString( Prefix + "custom_passwordhashtype", string( ) );

		if( Server.empty( ) )
			break;

		if( CDKeyROC.empty( ) )
		{
			CONSOLE_Print( "[GHOST] missing " + Prefix + "cdkeyroc, skipping this battle.net connection" );
			continue;
		}

		if( CDKeyTFT.empty( ) )
		{
			CONSOLE_Print( "[GHOST] missing " + Prefix + "cdkeytft, skipping this battle.net connection" );
			continue;
		}

		if( UserName.empty( ) )
		{
			CONSOLE_Print( "[GHOST] missing " + Prefix + "username, skipping this battle.net connection" );
			continue;
		}

		if( UserPassword.empty( ) )
		{
			CONSOLE_Print( "[GHOST] missing " + Prefix + "password, skipping this battle.net connection" );
			continue;
		}

		CONSOLE_Print( "[GHOST] found battle.net connection #" + UTIL_ToString( i ) + " for server [" + Server + "]" );
		m_BNETs.push_back( new CBNET( this, Server, CDKeyROC, CDKeyTFT, UserName, UserPassword, FirstChannel, RootAdmin, BNETCommandTrigger[0], HoldFriends, HoldClan, War3Version, EXEVersion, EXEVersionHash, PasswordHashType ) );
	}

	if( m_BNETs.empty( ) )
		CONSOLE_Print( "[GHOST] warning - no battle.net connections found in config file" );

	// extract common.j and blizzard.j from War3Patch.mpq if we can
	// these two files are necessary for calculating "map_crc" when loading maps so we make sure to do it before loading the default map
	// see CMap :: Load for more information

	ExtractScripts( );

	// load the default maps (note: make sure to run ExtractScripts first)

	CConfig MapCFG;
	MapCFG.Read( m_MapCFGPath + "map.cfg" );
	m_Map = new CMap( this, &MapCFG, m_MapCFGPath + "map.cfg" );
	m_AdminMap = new CMap( this );
	m_SaveGame = new CSaveGame( this );

	// load the iptocountry data

	LoadIPToCountryData( );

	// create the admin game

	if( m_AdminGameCreate )
	{
		CONSOLE_Print( "[GHOST] creating admin game" );
		m_AdminGame = new CAdminGame( this, m_AdminMap, NULL, m_AdminGamePort, 0, "GHost++ Admin Game", m_AdminGamePassword );

		if( m_AdminGamePort == m_HostPort )
			CONSOLE_Print( "[GHOST] warning - admingame_port and bot_hostport are set to the same value, you won't be able to host any games" );
	}
	else
		m_AdminGame = NULL;

	if( m_BNETs.empty( ) && !m_AdminGame )
		CONSOLE_Print( "[GHOST] warning - no battle.net connections found and no admin game created" );

	CONSOLE_Print( "[GHOST] GHost++ Version " + m_Version );
}

CGHost :: ~CGHost( )
{
	delete m_UDPSocket;
	delete m_CRC;

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		delete *i;

	delete m_CurrentGame;
	delete m_AdminGame;

	for( vector<CBaseGame *> :: iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
		delete *i;

	delete m_DB;
	delete m_Language;
	delete m_Map;
	delete m_AdminMap;
	delete m_SaveGame;
}

bool CGHost :: Update( long usecBlock )
{
	// todotodo: do we really want to shutdown if there's a database error? is there any way to recover from this?

	if( m_DB->HasError( ) )
	{
		CONSOLE_Print( "[GHOST] database error - " + m_DB->GetError( ) );
		return true;
	}

	unsigned int NumFDs = 0;

	// take every socket we own and throw it in one giant select statement so we can block on all sockets

	int nfds = 0;
	fd_set fd;
	FD_ZERO( &fd );

	// 1. all battle.net sockets

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		NumFDs += (*i)->SetFD( &fd, &nfds );

	// 2. the current game's server and player sockets

	if( m_CurrentGame )
		NumFDs += m_CurrentGame->SetFD( &fd, &nfds );

	// 3. the admin game's server and player sockets

	if( m_AdminGame )
		NumFDs += m_AdminGame->SetFD( &fd, &nfds );

	// 4. all running games' player sockets

	for( vector<CBaseGame *> :: iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
		NumFDs += (*i)->SetFD( &fd, &nfds );

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usecBlock;

#ifdef WIN32
	select( 1, &fd, NULL, NULL, &tv );
#else
	select( nfds + 1, &fd, NULL, NULL, &tv );
#endif

	if( NumFDs == 0 )
	{
		// we don't have any sockets (i.e. we aren't connected to battle.net maybe due to a lost connection and there aren't any games running)
		// select will return immediately and we'll chew up the CPU if we let it loop so just sleep for 50ms to kill some time

		MILLISLEEP( 50 );
	}

	bool AdminExit = false;
	bool BNETExit = false;

	// update current game

	if( m_CurrentGame )
	{
		if( m_CurrentGame->Update( &fd ) )
		{
			CONSOLE_Print( "[GHOST] deleting current game [" + m_CurrentGame->GetGameName( ) + "]" );
			delete m_CurrentGame;
			m_CurrentGame = NULL;

			for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
			{
				(*i)->SendGameUncreate( );
				(*i)->SendEnterChat( );
			}
		}
		else if( m_CurrentGame )
			m_CurrentGame->UpdatePost( );
	}

	// update admin game

	if( m_AdminGame )
	{
		if( m_AdminGame->Update( &fd ) )
		{
			CONSOLE_Print( "[GHOST] deleting admin game" );
			delete m_AdminGame;
			m_AdminGame = NULL;
			AdminExit = true;
		}
		else if( m_AdminGame )
			m_AdminGame->UpdatePost( );
	}

	// update running games

	for( vector<CBaseGame *> :: iterator i = m_Games.begin( ); i != m_Games.end( ); )
	{
		if( (*i)->Update( &fd ) )
		{
			CONSOLE_Print( "[GHOST] deleting game [" + (*i)->GetGameName( ) + "]" );
			EventGameDeleted( *i );
			delete *i;
			i = m_Games.erase( i );
		}
		else
		{
			(*i)->UpdatePost( );
			i++;
		}
	}

	// update battle.net connections

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		if( (*i)->Update( &fd ) )
			BNETExit = true;
	}

	// autohost

	if( !m_AutoHostGameName.empty( ) && !m_AutoHostMapCFG.empty( ) && m_AutoHostMaximumGames != 0 && m_AutoHostAutoStartPlayers != 0 && GetTime( ) >= m_LastAutoHostTime + 30 )
	{
		string GameName = m_AutoHostGameName + " #" + UTIL_ToString( m_HostCounter );

		// copy all the checks from CGHost :: CreateGame here because we don't want to spam the chat when there's an error
		// instead we fail silently and try again soon

		if( m_Enabled && GameName.size( ) <= 31 && !m_CurrentGame && m_Games.size( ) < m_MaxGames && m_Games.size( ) < m_AutoHostMaximumGames )
		{
			// load the autohost map config

			CConfig MapCFG;
			MapCFG.Read( m_AutoHostMapCFG );
			m_Map->Load( &MapCFG, m_AutoHostMapCFG );

			if( m_Map->GetValid( ) )
			{
				CreateGame( GAME_PUBLIC, false, GameName, m_AutoHostOwner, m_AutoHostOwner, m_AutoHostServer, false );

				if( m_CurrentGame )
					m_CurrentGame->SetAutoStartPlayers( m_AutoHostAutoStartPlayers );
			}
			else
			{
				CONSOLE_Print( "[GHOST] stopped auto hosting, map config file [" + m_AutoHostMapCFG + "] is invalid" );
				m_AutoHostGameName.clear( );
				m_AutoHostMapCFG.clear( );
				m_AutoHostOwner.clear( );
				m_AutoHostServer.clear( );
				m_AutoHostMaximumGames = 0;
				m_AutoHostAutoStartPlayers = 0;
			}
		}

		m_LastAutoHostTime = GetTime( );
	}

	return m_Exiting || AdminExit || BNETExit;
}

void CGHost :: EventBNETConnecting( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->ConnectingToBNET( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETConnected( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->ConnectedToBNET( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETDisconnected( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->DisconnectedFromBNET( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETLoggedIn( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->LoggedInToBNET( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETGameRefreshed( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->BNETGameHostingSucceeded( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETGameRefreshFailed( CBNET *bnet )
{
	if( m_CurrentGame )
	{
		for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			(*i)->QueueChatCommand( m_Language->UnableToCreateGameTryAnotherName( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ) );

			if( (*i)->GetServer( ) == m_CurrentGame->GetCreatorServer( ) )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameTryAnotherName( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ), m_CurrentGame->GetCreatorName( ), true );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->BNETGameHostingFailed( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ) );

		m_CurrentGame->SendAllChat( m_Language->UnableToCreateGameTryAnotherName( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ) );

		// we take the easy route and simply close the lobby if a refresh fails
		// it's possible at least one refresh succeeded and therefore the game is still joinable on at least one battle.net (plus on the local network) but we don't keep track of that
		// we only close the game if it has no players since we support game rehosting (via !priv and !pub in the lobby)

		if( m_CurrentGame->GetNumPlayers( ) == 0 )
			m_CurrentGame->SetExiting( true );
	}
}

void CGHost :: EventBNETConnectTimedOut( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->ConnectingToBNETTimedOut( bnet->GetServer( ) ) );
}

void CGHost :: EventGameDeleted( CBaseGame *game )
{
	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		(*i)->QueueChatCommand( m_Language->GameIsOver( game->GetDescription( ) ) );

		if( (*i)->GetServer( ) == game->GetCreatorServer( ) )
			(*i)->QueueChatCommand( m_Language->GameIsOver( game->GetDescription( ) ), game->GetCreatorName( ), true );
	}
}

void CGHost :: HandleCommandAdminGame( CAdminGame *game, string user, string command, string payload )
{

}

void CGHost :: HandleCommandBNET( CBNET *bnet, string user, string command, string payload, bool whisper )
{
	CUser User;
	User.SetName( user );

	if( bnet->IsRootAdmin( user ) )
	{
		CONSOLE_Print( "[GHOST] bnet root admin [" + user + "] sent command [" + command + "] with payload [" + payload + "]" );
		User.SetAccess( ACCESSGROUP_ALL );
	}
	else if( m_DB->AdminCheck( bnet->GetServer( ), user ) )
	{
		CONSOLE_Print( "[GHOST] bnet admin [" + user + "] sent command [" + command + "] with payload [" + payload + "]" );
		User.SetAccess( ACCESSGROUP_ADMIN );
	}
	else
	{
		CONSOLE_Print( "[GHOST] bnet user [" + user + "] sent command [" + command + "] with payload [" + payload + "]" );
		User.SetAccess( ACCESSGROUP_USER );
	}

	string Response;

	if( command == "addadmin" && User.GetAccess( ) & ACCESS_ADMIN_MANAGEMENT )
		Response = CmdAddAdminBNET( &User, bnet, payload );
	else if( ( command == "addban" || command == "ban" ) && User.GetAccess( ) & ACCESS_BAN_MANAGEMENT )
		Response = CmdAddBanBNET( &User, bnet, payload );
	else if( command == "announce" && User.GetAccess( ) & ACCESS_ANNOUNCE )
		Response = CmdAnnounce( &User, m_CurrentGame, payload );
	else if( command == "autohost" && User.GetAccess( ) & ACCESS_AUTOHOST )
		Response = CmdAutoHost( &User, user, bnet->GetServer( ), payload );
	else if( command == "autostart" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdAutoStart( &User, m_CurrentGame, payload );
	else if( command == "channel" && User.GetAccess( ) & ACCESS_BNET_MANAGEMENT )
		Response = CmdChannel( &User, bnet, payload );
	else if( command == "checkadmin" && User.GetAccess( ) & ACCESS_ADMIN_CHECKING )
		Response = CmdCheckAdminBNET( &User, bnet, payload );
	else if( command == "checkban" && User.GetAccess( ) & ACCESS_BAN_CHECKING )
		Response = CmdCheckBanBNET( &User, bnet, payload );
	else if( command == "close" && User.GetAccess( ) & ACCESS_SLOT_MANAGEMENT )
		Response = CmdClose( &User, bnet, m_CurrentGame, payload );
	else if( command == "closeall" && User.GetAccess( ) & ACCESS_SLOT_MANAGEMENT )
		Response = CmdCloseAll( &User, m_CurrentGame );
	else if( command == "countadmins" && User.GetAccess( ) & ACCESS_ADMIN_CHECKING )
		Response = CmdCountAdmins( &User, bnet->GetServer( ) );
	else if( command == "countbans" && User.GetAccess( ) & ACCESS_BAN_CHECKING )
		Response = CmdCountBans( &User, bnet->GetServer( ) );
	else if( command == "deladmin" && User.GetAccess( ) & ACCESS_ADMIN_MANAGEMENT )
		Response = CmdDelAdminBNET( &User, bnet, payload );
	else if( ( command == "delban" || command == "unban" ) && User.GetAccess( ) & ACCESS_BAN_MANAGEMENT )
		Response = CmdDelBan( &User, payload );
	else if( command == "disable" && User.GetAccess( ) & ACCESS_BOT_MANAGEMENT )
		Response = CmdDisable( &User );
	else if( command == "enable" && User.GetAccess( ) & ACCESS_BOT_MANAGEMENT )
		Response = CmdEnable( &User );
	else if( command == "endgame" && User.GetAccess( ) & ACCESS_ENDGAME )
		Response = CmdEndGame( &User, payload );
	else if( ( command == "exit" || command == "quit" ) && User.GetAccess( ) & ACCESS_BOT_MANAGEMENT )
		Response = CmdExit( &User, payload );
	else if( command == "getclan" && User.GetAccess( ) & ACCESS_BNET_MANAGEMENT )
		Response = CmdGetClan( &User, bnet );
	else if( command == "getfriends" && User.GetAccess( ) & ACCESS_BNET_MANAGEMENT )
		Response = CmdGetFriends( &User, bnet );
	else if( command == "getgame" && User.GetAccess( ) & ( ACCESS_ENDGAME | ACCESS_SAYGAMES ) )
		Response = CmdGetGame( &User, payload );
	else if( command == "getgames" && User.GetAccess( ) & ( ACCESS_ENDGAME | ACCESS_SAYGAMES ) )
		Response = CmdGetGames( &User );
	else if( command == "hold" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdHold( &User, m_CurrentGame, payload );
	else if( command == "hostsg" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdHostSG( &User, user, bnet->GetServer( ), payload, whisper );
	else if( ( command == "load" || command == "map" ) && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdLoad( &User, payload );
	else if( command == "loadsg" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdLoadSG( &User, payload );
	else if( command == "open" && User.GetAccess( ) & ACCESS_SLOT_MANAGEMENT )
		Response = CmdOpen( &User, bnet, m_CurrentGame, payload );
	else if( command == "openall" && User.GetAccess( ) & ACCESS_SLOT_MANAGEMENT )
		Response = CmdOpenAll( &User, m_CurrentGame );
	else if( command == "priv" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdPriv( &User, user, bnet->GetServer( ), payload, whisper );
	else if( command == "privby" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdPrivBy( &User, user, bnet->GetServer( ), payload, whisper );
	else if( command == "pub" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdPub( &User, user, bnet->GetServer( ), payload, whisper );
	else if( command == "pubby" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdPubBy( &User, user, bnet->GetServer( ), payload, whisper );
	else if( command == "say" && User.GetAccess( ) & ACCESS_BNET_MANAGEMENT )
		Response = CmdSay( &User, bnet, payload );
	else if( command == "saygame" && User.GetAccess( ) & ACCESS_SAYGAMES )
		Response = CmdSayGame( &User, payload );
	else if( command == "saygames" && User.GetAccess( ) & ACCESS_SAYGAMES )
		Response = CmdSayGames( &User, payload );
	else if( command == "sp" && User.GetAccess( ) & ACCESS_SLOT_MANAGEMENT )
		Response = CmdSP( &User, m_CurrentGame );
	else if( command == "start" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdStart( &User, m_CurrentGame, payload );
	else if( command == "stats" && User.GetAccess( ) & ACCESS_STATS && bnet->GetNumChatCommands( ) < 3 )
		Response = CmdStats( &User, payload );
	else if( command == "statsdota" && User.GetAccess( ) & ACCESS_STATS && bnet->GetNumChatCommands( ) < 3 )
		Response = CmdStatsDotA( &User, payload );
	else if( command == "swap" && User.GetAccess( ) & ACCESS_SLOT_MANAGEMENT )
		Response = CmdSwap( &User, m_CurrentGame, payload );
	else if( command == "unhost" && User.GetAccess( ) & ACCESS_GAME_MANAGEMENT )
		Response = CmdUnhost( &User, m_CurrentGame );
	else if( command == "version" && User.GetAccess( ) & ACCESS_VERSION && bnet->GetNumChatCommands( ) < 3 )
		Response = CmdVersion( &User );

	// done

	if( !Response.empty( ) )
		bnet->QueueChatCommand( Response, user, whisper );
}

void CGHost :: HandleCommandGame( CBaseGame *game, string user, string command, string payload )
{

}

void CGHost :: ExtractScripts( )
{
	string PatchMPQFileName = m_Warcraft3Path + "War3Patch.mpq";
	HANDLE PatchMPQ;

	if( SFileOpenArchive( PatchMPQFileName.c_str( ), 0, 0, &PatchMPQ ) )
	{
		CONSOLE_Print( "[GHOST] loading MPQ file [" + PatchMPQFileName + "]" );
		HANDLE SubFile;

		// common.j

		if( SFileOpenFileEx( PatchMPQ, "Scripts\\common.j", 0, &SubFile ) )
		{
			uint32_t FileLength = SFileGetFileSize( SubFile, NULL );

			if( FileLength > 0 && FileLength != 0xFFFFFFFF )
			{
				char *SubFileData = new char[FileLength];
				DWORD BytesRead = 0;

				if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead ) )
				{
					CONSOLE_Print( "[GHOST] extracting Scripts\\common.j from MPQ file to [" + m_MapCFGPath + "common.j]" );
					UTIL_FileWrite( m_MapCFGPath + "common.j", (unsigned char *)SubFileData, BytesRead );
				}
				else
					CONSOLE_Print( "[GHOST] warning - unable to extract Scripts\\common.j from MPQ file" );

				delete [] SubFileData;
			}

			SFileCloseFile( SubFile );
		}
		else
			CONSOLE_Print( "[GHOST] couldn't find Scripts\\common.j in MPQ file" );

		// blizzard.j

		if( SFileOpenFileEx( PatchMPQ, "Scripts\\blizzard.j", 0, &SubFile ) )
		{
			uint32_t FileLength = SFileGetFileSize( SubFile, NULL );

			if( FileLength > 0 && FileLength != 0xFFFFFFFF )
			{
				char *SubFileData = new char[FileLength];
				DWORD BytesRead = 0;

				if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead ) )
				{
					CONSOLE_Print( "[GHOST] extracting Scripts\\blizzard.j from MPQ file to [" + m_MapCFGPath + "blizzard.j]" );
					UTIL_FileWrite( m_MapCFGPath + "blizzard.j", (unsigned char *)SubFileData, BytesRead );
				}
				else
					CONSOLE_Print( "[GHOST] warning - unable to extract Scripts\\blizzard.j from MPQ file" );

				delete [] SubFileData;
			}

			SFileCloseFile( SubFile );
		}
		else
			CONSOLE_Print( "[GHOST] couldn't find Scripts\\blizzard.j in MPQ file" );

		SFileCloseArchive( PatchMPQ );
	}
	else
		CONSOLE_Print( "[GHOST] warning - unable to load MPQ file [" + PatchMPQFileName + "]" );
}

void CGHost :: LoadIPToCountryData( )
{
	ifstream in;
	in.open( "ip-to-country.csv" );

	if( in.fail( ) )
		CONSOLE_Print( "[GHOST] warning - unable to read file [ip-to-country.csv], iptocountry data not loaded" );
	else
	{
		CONSOLE_Print( "[GHOST] started loading [ip-to-country.csv]" );

		// the begin and commit statements are optimizations
		// we're about to insert ~4 MB of data into the database so if we allow the database to treat each insert as a transaction it will take a LONG time
		// todotodo: handle begin/commit failures a bit more gracefully

		if( !m_DB->Begin( ) )
			CONSOLE_Print( "[GHOST] warning - failed to begin database transaction, iptocountry data not loaded" );
		else
		{
			unsigned char Percent = 0;
			string Line;
			string IP1;
			string IP2;
			string Country;
			CSVParser parser;

			// get length of file for the progress meter

			in.seekg( 0, ios :: end );
			uint32_t FileLength = in.tellg( );
			in.seekg( 0, ios :: beg );

			while( !in.eof( ) )
			{
				getline( in, Line );

				if( Line.empty( ) )
					continue;

				parser << Line;
				parser >> IP1;
				parser >> IP2;
				parser >> Country;
				m_DB->FromAdd( UTIL_ToUInt32( IP1 ), UTIL_ToUInt32( IP2 ), Country );

				// it's probably going to take awhile to load the iptocountry data (~10 seconds on my 3.2 GHz P4 when using SQLite3)
				// so let's print a progress meter just to keep the user from getting worried

				unsigned char NewPercent = (unsigned char)( (float)in.tellg( ) / FileLength * 100 );

				if( NewPercent != Percent && NewPercent % 10 == 0 )
				{
					Percent = NewPercent;
					CONSOLE_Print( "[GHOST] iptocountry data: " + UTIL_ToString( Percent ) + "% loaded" );
				}
			}

			if( !m_DB->Commit( ) )
				CONSOLE_Print( "[GHOST] warning - failed to commit database transaction, iptocountry data not loaded" );
			else
				CONSOLE_Print( "[GHOST] finished loading [ip-to-country.csv]" );
		}

		in.close( );
	}
}

void CGHost :: CreateGame( unsigned char gameState, bool saveGame, string gameName, string ownerName, string creatorName, string creatorServer, bool whisper )
{
	if( !m_Enabled )
	{
		for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameDisabled( gameName ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameDisabled( gameName ) );

		return;
	}

	if( gameName.size( ) > 31 )
	{
		for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameNameTooLong( gameName ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameNameTooLong( gameName ) );

		return;
	}

	if( !m_Map->GetValid( ) )
	{
		for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameInvalidMap( gameName ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameInvalidMap( gameName ) );

		return;
	}

	if( saveGame )
	{
		if( !m_SaveGame->GetValid( ) )
		{
			for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
			{
				if( (*i)->GetServer( ) == creatorServer )
					(*i)->QueueChatCommand( m_Language->UnableToCreateGameInvalidSaveGame( gameName ), creatorName, whisper );
			}

			if( m_AdminGame )
				m_AdminGame->SendAllChat( m_Language->UnableToCreateGameInvalidSaveGame( gameName ) );

			return;
		}

		string MapPath1 = m_SaveGame->GetMapPath( );
		string MapPath2 = m_Map->GetMapPath( );
		transform( MapPath1.begin( ), MapPath1.end( ), MapPath1.begin( ), (int(*)(int))tolower );
		transform( MapPath2.begin( ), MapPath2.end( ), MapPath2.begin( ), (int(*)(int))tolower );

		if( MapPath1 != MapPath2 )
		{
			for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
			{
				if( (*i)->GetServer( ) == creatorServer )
					(*i)->QueueChatCommand( m_Language->UnableToCreateGameSaveGameMapMismatch( gameName ), creatorName, whisper );
			}

			if( m_AdminGame )
				m_AdminGame->SendAllChat( m_Language->UnableToCreateGameSaveGameMapMismatch( gameName ) );

			return;
		}
	}

	if( m_CurrentGame )
	{
		for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameAnotherGameInLobby( gameName, m_CurrentGame->GetDescription( ) ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameAnotherGameInLobby( gameName, m_CurrentGame->GetDescription( ) ) );

		return;
	}

	if( m_Games.size( ) >= m_MaxGames )
	{
		for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameMaxGamesReached( gameName, UTIL_ToString( m_MaxGames ) ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameMaxGamesReached( gameName, UTIL_ToString( m_MaxGames ) ) );

		return;
	}

	CONSOLE_Print( "[GHOST] creating game [" + gameName + "]" );

	if( saveGame )
		m_CurrentGame = new CGame( this, m_Map, m_SaveGame, m_HostPort, gameState, gameName, ownerName, creatorName, creatorServer );
	else
		m_CurrentGame = new CGame( this, m_Map, NULL, m_HostPort, gameState, gameName, ownerName, creatorName, creatorServer );

	// todotodo: check if listening failed and report the error to the user

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		if( whisper && (*i)->GetServer( ) == creatorServer )
		{
			// note that we send this whisper only on the creator server

			if( gameState == GAME_PRIVATE )
				(*i)->QueueChatCommand( m_Language->CreatingPrivateGame( gameName, ownerName ), creatorName, whisper );
			else if( gameState == GAME_PUBLIC )
				(*i)->QueueChatCommand( m_Language->CreatingPublicGame( gameName, ownerName ), creatorName, whisper );
		}
		else
		{
			// try to send an immediate chat command if it's not a whisper
			// this is because if we queue the chat command it will get sent after the game creation message and battle.net will discard it
			// note that we send this on all bnet servers

			if( gameState == GAME_PRIVATE )
				(*i)->ImmediateChatCommand( m_Language->CreatingPrivateGame( gameName, ownerName ) );
			else if( gameState == GAME_PUBLIC )
				(*i)->ImmediateChatCommand( m_Language->CreatingPublicGame( gameName, ownerName ) );
		}

		if( saveGame )
			(*i)->SendGameCreate( gameState, gameName, string( ), m_Map, m_SaveGame );
		else
			(*i)->SendGameCreate( gameState, gameName, string( ), m_Map, NULL );
	}

	if( m_AdminGame )
	{
		if( gameState == GAME_PRIVATE )
			m_AdminGame->SendAllChat( m_Language->CreatingPrivateGame( gameName, ownerName ) );
		else if( gameState == GAME_PUBLIC )
			m_AdminGame->SendAllChat( m_Language->CreatingPublicGame( gameName, ownerName ) );
	}

	// if we're creating a private game we don't need to send any game refresh messages so we can rejoin the chat immediately
	// unfortunately this doesn't work on PVPGN servers because they consider an enterchat message to be a gameuncreate message when in a game
	// so don't rejoin the chat if we're using PVPGN

	if( gameState == GAME_PRIVATE )
	{
		for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetPasswordHashType( ) != "pvpgn" )
				(*i)->SendEnterChat( );
		}
	}

	// hold friends and/or clan members

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		if( (*i)->GetHoldFriends( ) )
			(*i)->HoldFriends( m_CurrentGame );

		if( (*i)->GetHoldClan( ) )
			(*i)->HoldClan( m_CurrentGame );
	}
}

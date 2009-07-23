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
#include "config.h"
#include "language.h"
#include "socket.h"
#include "ghostdb.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "replay.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game_base.h"

#include <cmath>
#include <string.h>
#include <time.h>

//
// CBaseGame
//

CBaseGame :: CBaseGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nOwnerName, string nCreatorName, string nCreatorServer )
{
	m_GHost = nGHost;
	m_Socket = new CTCPServer( );
	m_Protocol = new CGameProtocol( m_GHost );
	m_Map = nMap;
	m_SaveGame = nSaveGame;

	if( m_GHost->m_SaveReplays && !m_SaveGame )
		m_Replay = new CReplay( m_GHost );
	else
		m_Replay = NULL;

	m_Exiting = false;
	m_Saving = false;
	m_HostPort = nHostPort;
	m_GameState = nGameState;
	m_VirtualHostPID = 255;
	m_FakePlayerPID = 255;
	m_GameName = nGameName;
	m_VirtualHostName = m_GHost->m_VirtualHostName;
	m_OwnerName = nOwnerName;
	m_CreatorName = nCreatorName;
	m_CreatorServer = nCreatorServer;
	m_HCLCommandString = m_Map->GetMapDefaultHCL( );
	m_RandomSeed = GetTicks( );
	m_HostCounter = m_GHost->m_HostCounter++;
	m_Latency = m_GHost->m_Latency;
	m_SyncLimit = m_GHost->m_SyncLimit;
	m_MaxSyncCounter = 0;
	m_GameTicks = 0;
	m_CreationTime = GetTime( );
	m_LastPingTime = GetTime( );
	m_LastRefreshTime = GetTime( );
	m_LastDownloadTicks = GetTime( );
	m_LastAnnounceTime = 0;
	m_AnnounceInterval = 0;
	m_LastAutoStartTime = GetTime( );
	m_AutoStartPlayers = 0;
	m_LastCountDownTicks = 0;
	m_CountDownCounter = 0;
	m_StartedLoadingTicks = 0;
	m_StartPlayers = 0;
	m_LastLoadInGameResetTime = 0;
	m_LastActionSentTicks = 0;
	m_StartedLaggingTime = 0;
	m_LastLagScreenTime = 0;
	m_LastReservedSeen = GetTime( );
	m_StartedKickVoteTime = 0;
	m_GameOverTime = 0;
	m_MinimumScore = 0.0;
	m_MaximumScore = 0.0;
	m_Locked = false;
	m_RefreshMessages = m_GHost->m_RefreshMessages;
	m_RefreshError = false;
	m_RefreshRehosted = false;
	m_MuteAll = false;
	m_MuteLobby = false;
	m_CountDownStarted = false;
	m_GameLoading = false;
	m_GameLoaded = false;
	m_LoadInGame = m_Map->GetMapLoadInGame( );
	m_Desynced = false;
	m_Lagging = false;
	m_AutoSave = m_GHost->m_AutoSave;
	m_MatchMaking = false;

	if( m_SaveGame )
	{
		m_Slots = m_SaveGame->GetSlots( );

		// the savegame slots contain player entries
		// we really just want the open/closed/computer entries
		// so open all the player slots

		for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
		{
			if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 )
			{
				(*i).SetPID( 0 );
				(*i).SetDownloadStatus( 255 );
				(*i).SetSlotStatus( SLOTSTATUS_OPEN );
			}
		}
	}
	else
		m_Slots = m_Map->GetSlots( );

	if( !m_GHost->m_IPBlackListFile.empty( ) )
	{
		ifstream in;
		in.open( m_GHost->m_IPBlackListFile.c_str( ) );

		if( in.fail( ) )
			CONSOLE_Print( "[GAME: " + m_GameName + "] error loading IP blacklist file [" + m_GHost->m_IPBlackListFile + "]" );
		else
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] loading IP blacklist file [" + m_GHost->m_IPBlackListFile + "]" );
			string Line;

			while( !in.eof( ) )
			{
				getline( in, Line );

				// ignore blank lines and comments

				if( Line.empty( ) || Line[0] == '#' )
					continue;

				// remove newlines and partial newlines to help fix issues with Windows formatted files on Linux systems

				Line.erase( remove( Line.begin( ), Line.end( ), ' ' ), Line.end( ) );
				Line.erase( remove( Line.begin( ), Line.end( ), '\r' ), Line.end( ) );
				Line.erase( remove( Line.begin( ), Line.end( ), '\n' ), Line.end( ) );

				// ignore lines that don't look like IP addresses

				if( Line.find_first_not_of( "1234567890." ) != string :: npos )
					continue;

				m_IPBlackList.insert( Line );
			}

			in.close( );

			CONSOLE_Print( "[GAME: " + m_GameName + "] loaded " + UTIL_ToString( m_IPBlackList.size( ) ) + " lines from IP blacklist file" );
		}
	}

	// start listening for connections

	if( !m_GHost->m_BindAddress.empty( ) )
		CONSOLE_Print( "[GAME: " + m_GameName + "] attempting to bind to address [" + m_GHost->m_BindAddress + "]" );
	else
		CONSOLE_Print( "[GAME: " + m_GameName + "] attempting to bind to all available addresses" );

	if( m_Socket->Listen( m_GHost->m_BindAddress, m_HostPort ) )
		CONSOLE_Print( "[GAME: " + m_GameName + "] listening on port " + UTIL_ToString( m_HostPort ) );
	else
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] error listening on port " + UTIL_ToString( m_HostPort ) );
		m_Exiting = true;
	}
}

CBaseGame :: ~CBaseGame( )
{
	// save replay
	// todotodo: put this in a thread

	if( m_Replay && ( m_GameLoading || m_GameLoaded ) )
	{
		time_t Now = time( NULL );
		char Time[17];
		memset( Time, 0, sizeof( char ) * 17 );
		strftime( Time, sizeof( char ) * 17, "%Y-%m-%d %H-%M", localtime( &Now ) );
		string MinString = UTIL_ToString( ( m_GameTicks / 1000 ) / 60 );
		string SecString = UTIL_ToString( ( m_GameTicks / 1000 ) % 60 );

		if( MinString.size( ) == 1 )
			MinString.insert( 0, "0" );

		if( SecString.size( ) == 1 )
			SecString.insert( 0, "0" );

		m_Replay->BuildReplay( m_GameName, m_StatString );
		m_Replay->Save( m_GHost->m_ReplayPath + UTIL_FileSafeName( "GHost++ " + string( Time ) + " " + m_GameName + " (" + MinString + "m" + SecString + "s).w3g" ) );
	}

	delete m_Socket;
	delete m_Protocol;
	delete m_Replay;

	for( vector<CPotentialPlayer *> :: iterator i = m_Potentials.begin( ); i != m_Potentials.end( ); i++ )
		delete *i;

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		delete *i;

	for( vector<CCallableScoreCheck *> :: iterator i = m_ScoreChecks.begin( ); i != m_ScoreChecks.end( ); i++ )
		m_GHost->m_Callables.push_back( *i );

	while( !m_Actions.empty( ) )
	{
		delete m_Actions.front( );
		m_Actions.pop( );
	}
}

uint32_t CBaseGame :: GetSlotsOccupied( )
{
	uint32_t NumSlotsOccupied = 0;

	for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED )
			NumSlotsOccupied++;
	}

	return NumSlotsOccupied;
}

uint32_t CBaseGame :: GetSlotsOpen( )
{
	uint32_t NumSlotsOpen = 0;

	for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OPEN )
			NumSlotsOpen++;
	}

	return NumSlotsOpen;
}

uint32_t CBaseGame :: GetNumPlayers( )
{
	uint32_t NumPlayers = 0;

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
			NumPlayers++;
	}

	return NumPlayers;
}

string CBaseGame :: GetDescription( )
{
	string Description = m_GameName + " : " + m_OwnerName + " : " + UTIL_ToString( GetNumPlayers( ) ) + "/" + UTIL_ToString( m_GameLoading || m_GameLoaded ? m_StartPlayers : m_Slots.size( ) );

	if( m_GameLoading || m_GameLoaded )
		Description += " : " + UTIL_ToString( ( m_GameTicks / 1000 ) / 60 ) + "m";
	else
		Description += " : " + UTIL_ToString( ( GetTime( ) - m_CreationTime ) / 60 ) + "m";

	return Description;
}

void CBaseGame :: SetAnnounce( uint32_t interval, string message )
{
	m_AnnounceInterval = interval;
	m_AnnounceMessage = message;
	m_LastAnnounceTime = GetTime( );
}

unsigned int CBaseGame :: SetFD( void *fd, void *send_fd, int *nfds )
{
	unsigned int NumFDs = 0;

	if( m_Socket )
	{
		m_Socket->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
		NumFDs++;
	}

	for( vector<CPotentialPlayer *> :: iterator i = m_Potentials.begin( ); i != m_Potentials.end( ); i++ )
	{
		if( (*i)->GetSocket( ) )
		{
			(*i)->GetSocket( )->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
			NumFDs++;
		}
	}

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetSocket( ) )
		{
			(*i)->GetSocket( )->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
			NumFDs++;
		}
	}

	return NumFDs;
}

bool CBaseGame :: Update( void *fd, void *send_fd )
{
	// update callables

	for( vector<CCallableScoreCheck *> :: iterator i = m_ScoreChecks.begin( ); i != m_ScoreChecks.end( ); )
	{
		if( (*i)->GetReady( ) )
		{
			double Score = (*i)->GetResult( );

			for( vector<CPotentialPlayer *> :: iterator j = m_Potentials.begin( ); j != m_Potentials.end( ); j++ )
			{
				if( (*j)->GetJoinPlayer( ) && (*j)->GetJoinPlayer( )->GetName( ) == (*i)->GetName( ) )
					EventPlayerJoinedWithScore( *j, (*j)->GetJoinPlayer( ), Score );
			}

			m_GHost->m_DB->RecoverCallable( *i );
			delete *i;
			i = m_ScoreChecks.erase( i );
		}
		else
			i++;
	}

	// update players

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); )
	{
		if( (*i)->Update( fd ) )
		{
			EventPlayerDeleted( *i );
			delete *i;
			i = m_Players.erase( i );
		}
		else
			i++;
	}

	for( vector<CPotentialPlayer *> :: iterator i = m_Potentials.begin( ); i != m_Potentials.end( ); )
	{
		if( (*i)->Update( fd ) )
		{
			// flush the socket (e.g. in case a rejection message is queued)

			if( (*i)->GetSocket( ) )
				(*i)->GetSocket( )->DoSend( (fd_set *)send_fd );

			delete *i;
			i = m_Potentials.erase( i );
		}
		else
			i++;
	}

	// create the virtual host player

	if( !m_GameLoading && !m_GameLoaded && GetNumPlayers( ) < 12 )
		CreateVirtualHost( );

	// unlock the game

	if( m_Locked && !GetPlayerFromName( m_OwnerName, false ) )
	{
		SendAllChat( m_GHost->m_Language->GameUnlocked( ) );
		m_Locked = false;
	}

	// ping every 5 seconds
	// changed this to ping during game loading as well to hopefully fix some problems with people disconnecting during loading
	// changed this to ping during the game as well

	if( GetTime( ) >= m_LastPingTime + 5 )
	{
		// note: we must send pings to players who are downloading the map because Warcraft III disconnects from the lobby if it doesn't receive a ping every ~90 seconds
		// so if the player takes longer than 90 seconds to download the map they would be disconnected unless we keep sending pings
		// todotodo: ignore pings received from players who have recently finished downloading the map

		SendAll( m_Protocol->SEND_W3GS_PING_FROM_HOST( ) );

		// we also broadcast the game to the local network every 5 seconds so we hijack this timer for our nefarious purposes
		// however we only want to broadcast if the countdown hasn't started
		// see the !sendlan code later in this file for some more information about how this works
		// todotodo: should we send a game cancel message somewhere? we'll need to implement a host counter for it to work

		if( !m_CountDownStarted )
		{
			BYTEARRAY MapGameType;

			// construct the correct W3GS_GAMEINFO packet

			if( m_SaveGame )
			{
				MapGameType.push_back( 0 );
				MapGameType.push_back( 2 );
				MapGameType.push_back( 0 );
				MapGameType.push_back( 0 );
				BYTEARRAY MapWidth;
				MapWidth.push_back( 0 );
				MapWidth.push_back( 0 );
				BYTEARRAY MapHeight;
				MapHeight.push_back( 0 );
				MapHeight.push_back( 0 );
				m_GHost->m_UDPSocket->Broadcast( 6112, m_Protocol->SEND_W3GS_GAMEINFO( MapGameType, m_Map->GetMapGameFlags( ), MapWidth, MapHeight, m_GameName, "Varlock", GetTime( ) - m_CreationTime, "Save\\Multiplayer\\" + m_SaveGame->GetFileNameNoPath( ), m_SaveGame->GetMagicNumber( ), 12, 12, m_HostPort, m_HostCounter ) );
			}
			else
			{
				MapGameType.push_back( m_Map->GetMapGameType( ) );
				MapGameType.push_back( 0 );
				MapGameType.push_back( 0 );
				MapGameType.push_back( 0 );
				m_GHost->m_UDPSocket->Broadcast( 6112, m_Protocol->SEND_W3GS_GAMEINFO( MapGameType, m_Map->GetMapGameFlags( ), m_Map->GetMapWidth( ), m_Map->GetMapHeight( ), m_GameName, "Varlock", GetTime( ) - m_CreationTime, m_Map->GetMapPath( ), m_Map->GetMapCRC( ), 12, 12, m_HostPort, m_HostCounter ) );
			}
		}

		m_LastPingTime = GetTime( );
	}

	// auto rehost if there was a refresh error in autohosted games

	if( m_RefreshError && !m_CountDownStarted && m_GameState == GAME_PUBLIC && !m_GHost->m_AutoHostGameName.empty( ) && m_GHost->m_AutoHostMaximumGames != 0 && m_GHost->m_AutoHostAutoStartPlayers != 0 && m_AutoStartPlayers != 0 )
	{
		// there's a slim chance that this isn't actually an autohosted game since there is no explicit autohost flag
		// however, if autohosting is enabled and this game is public and this game is set to autostart, it's probably autohosted
		// so rehost it using the current autohost game name

		string GameName = m_GHost->m_AutoHostGameName + " #" + UTIL_ToString( m_GHost->m_HostCounter );
		CONSOLE_Print( "[GAME: " + m_GameName + "] automatically trying to rehost as public game [" + GameName + "] due to refresh failure" );
		m_GameName = GameName;
		m_HostCounter = m_GHost->m_HostCounter++;
		m_RefreshError = false;

		for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			(*i)->QueueGameUncreate( );
			(*i)->QueueEnterChat( );

			// the game creation message will be sent on the next refresh
		}

		m_CreationTime = GetTime( );
		m_LastRefreshTime = GetTime( );
	}

	// refresh every 3 seconds

	if( !m_RefreshError && !m_CountDownStarted && m_GameState == GAME_PUBLIC && GetSlotsOpen( ) > 0 && GetTime( ) >= m_LastRefreshTime + 3 )
	{
		// send a game refresh packet to each battle.net connection

		bool Refreshed = false;

		for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			// don't queue a game refresh message if the queue contains more than 1 packet because they're very low priority

			if( (*i)->GetOutPacketsQueued( ) <= 1 )
			{
				(*i)->QueueGameRefresh( m_GameState, m_GameName, string( ), m_Map, m_SaveGame, GetTime( ) - m_CreationTime, m_HostCounter );
				Refreshed = true;
			}
		}

		// only print the "game refreshed" message if we actually refreshed on at least one battle.net server

		if( m_RefreshMessages && Refreshed )
			SendAllChat( m_GHost->m_Language->GameRefreshed( ) );

		m_LastRefreshTime = GetTime( );
	}

	// send more map data

	if( !m_GameLoading && !m_GameLoaded && GetTicks( ) >= m_LastDownloadTicks + 250 )
	{
		uint32_t Downloaders = 0;
		uint32_t DownloadCounter = 0;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetDownloadStarted( ) && !(*i)->GetDownloadFinished( ) )
			{
				Downloaders++;

				if( m_GHost->m_MaxDownloaders > 0 && Downloaders > m_GHost->m_MaxDownloaders )
					break;

				// send up to 50 pieces of the map at once so that the download goes faster
				// if we wait for each MAPPART packet to be acknowledged by the client it'll take a long time to download
				// this is because we would have to wait the round trip time (the ping time) between sending every 1442 bytes of map data
				// doing it this way allows us to send at least 70 KB in each round trip interval which is much more reasonable
				// the theoretical throughput is [70 KB * 1000 / ping] in KB/sec so someone with 100 ping (round trip ping, not LC ping) could download at 700 KB/sec

				uint32_t MapSize = UTIL_ByteArrayToUInt32( m_Map->GetMapSize( ), false );

				while( (*i)->GetLastMapPartSent( ) < (*i)->GetLastMapPartAcked( ) + 1442 * 50 && (*i)->GetLastMapPartSent( ) < MapSize )
				{
					if( (*i)->GetLastMapPartSent( ) == 0 )
					{
						// overwrite the "started download ticks" since this is the first time we've sent any map data to the player
						// prior to this we've only determined if the player needs to download the map but it's possible we could have delayed sending any data due to download limits

						(*i)->SetStartedDownloadingTicks( GetTicks( ) );
					}

					// limit the download speed if we're sending too much data
					// we divide by 4 because we run this code every 250ms (i.e. four times per second)

					if( m_GHost->m_MaxDownloadSpeed > 0 && DownloadCounter > m_GHost->m_MaxDownloadSpeed * 1024 / 4 )
						break;

					Send( *i, m_Protocol->SEND_W3GS_MAPPART( GetHostPID( ), (*i)->GetPID( ), (*i)->GetLastMapPartSent( ), m_Map->GetMapData( ) ) );
					(*i)->SetLastMapPartSent( (*i)->GetLastMapPartSent( ) + 1442 );
					DownloadCounter += 1442;
				}
			}
		}

		m_LastDownloadTicks = GetTicks( );
	}

	// announce every m_AnnounceInterval seconds

	if( !m_AnnounceMessage.empty( ) && !m_CountDownStarted && GetTime( ) >= m_LastAnnounceTime + m_AnnounceInterval )
	{
		SendAllChat( m_AnnounceMessage );
		m_LastAnnounceTime = GetTime( );
	}

	// kick players who don't spoof check within 20 seconds when matchmaking is enabled

	if( !m_CountDownStarted && m_MatchMaking && m_AutoStartPlayers != 0 && !m_Map->GetMapMatchMakingCategory( ).empty( ) && m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM && m_GHost->m_BNETs.size( ) == 1 )
	{
		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( !(*i)->GetSpoofed( ) && GetTime( ) >= (*i)->GetJoinTime( ) + 20 )
			{
				(*i)->SetDeleteMe( true );
				(*i)->SetLeftReason( m_GHost->m_Language->WasKickedForNotSpoofChecking( ) );
				(*i)->SetLeftCode( PLAYERLEAVE_LOBBY );
				OpenSlot( GetSIDFromPID( (*i)->GetPID( ) ), false );
			}
		}
	}

	// try to auto start every 10 seconds

	if( m_AutoStartPlayers != 0 && GetTime( ) >= m_LastAutoStartTime + 10 )
	{
		// require spoof checks when using matchmaking

		if( m_MatchMaking && m_AutoStartPlayers != 0 && !m_Map->GetMapMatchMakingCategory( ).empty( ) && m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM && m_GHost->m_BNETs.size( ) == 1 )
		{
			uint32_t PlayersScored = 0;
			uint32_t PlayersNotScored = 0;
			double AverageScore = 0.0;
			double MinScore = 0.0;
			double MaxScore = 0.0;
			bool Found = false;

			for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			{
				if( (*i)->GetScore( ) < -99999.0 )
					PlayersNotScored++;
				else
				{
					PlayersScored++;
					AverageScore += (*i)->GetScore( );

					if( !Found || (*i)->GetScore( ) < MinScore )
						MinScore = (*i)->GetScore( );

					if( !Found || (*i)->GetScore( ) > MaxScore )
						MaxScore = (*i)->GetScore( );

					Found = true;
				}
			}

			double Spread = MaxScore - MinScore;

			// todotodo: don't start the countdown if the spread is too large

			StartCountDownAuto( true );
		}
		else
			StartCountDownAuto( false );

		m_LastAutoStartTime = GetTime( );
	}

	// countdown every 500 ms

	if( m_CountDownStarted && GetTicks( ) >= m_LastCountDownTicks + 500 )
	{
		if( m_CountDownCounter > 0 )
		{
			// we use a countdown counter rather than a "finish countdown time" here because it might alternately round up or down the count
			// this sometimes resulted in a countdown of e.g. "6 5 3 2 1" during my testing which looks pretty dumb
			// doing it this way ensures it's always "5 4 3 2 1" but each interval might not be *exactly* the same length

			SendAllChat( UTIL_ToString( m_CountDownCounter ) + ". . ." );
			m_CountDownCounter--;
		}
		else if( !m_GameLoading && !m_GameLoaded )
			EventGameStarted( );

		m_LastCountDownTicks = GetTicks( );
	}

	// check if the lobby is "abandoned" and needs to be closed since it will never start

	if( !m_GameLoading && !m_GameLoaded && m_AutoStartPlayers == 0 && m_GHost->m_LobbyTimeLimit > 0 )
	{
		// check if there's a player with reserved status in the game

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetReserved( ) )
				m_LastReservedSeen = GetTime( );
		}

		// check if we've hit the time limit

		if( GetTime( ) >= m_LastReservedSeen + m_GHost->m_LobbyTimeLimit * 60 )
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] is over (lobby time limit hit)" );
			return true;
		}
	}

	// check if the game is loaded

	if( m_GameLoading )
	{
		bool FinishedLoading = true;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			FinishedLoading = (*i)->GetFinishedLoading( );

			if( !FinishedLoading )
				break;
		}

		if( FinishedLoading )
		{
			m_LastActionSentTicks = GetTicks( );
			m_GameLoading = false;
			m_GameLoaded = true;
			EventGameLoaded( );
		}
		else
		{
			// reset the "lag" screen (the load-in-game screen) every 30 seconds

			if( m_LoadInGame && GetTime( ) >= m_LastLoadInGameResetTime + 30 )
			{
				for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
				{
					if( (*i)->GetFinishedLoading( ) )
					{
						// stop the lag screen

						for( vector<CGamePlayer *> :: iterator j = m_Players.begin( ); j != m_Players.end( ); j++ )
						{
							if( !(*j)->GetFinishedLoading( ) )
								Send( *i, m_Protocol->SEND_W3GS_STOP_LAG( *j, true ) );
						}

						// send an empty update
						// this resets the lag screen timer but creates a rather annoying problem
						// in order to prevent a desync we must make sure every player receives the exact same "desyncable game data" (updates and player leaves) in the exact same order
						// unfortunately we cannot send updates to players who are still loading the map, so we buffer the updates to those players (see the else clause a few lines down for the code)
						// in addition to this we must ensure any player leave messages are sent in the exact same position relative to these updates so those must be buffered too

						Send( *i, m_Protocol->SEND_W3GS_INCOMING_ACTION( queue<CIncomingAction *>( ), 0 ) );

						// start the lag screen

						Send( *i, m_Protocol->SEND_W3GS_START_LAG( m_Players, true ) );
					}
					else
					{
						// buffer the empty update since the player is still loading the map

						UTIL_AppendByteArray( *(*i)->GetLoadInGameData( ), m_Protocol->SEND_W3GS_INCOMING_ACTION( queue<CIncomingAction *>( ), 0 ) );
					}
				}

				if( m_Replay )
					m_Replay->AddTimeSlot( 0, queue<CIncomingAction *>( ) );

				m_LastLoadInGameResetTime = GetTime( );
			}
		}
	}

	// keep track of the largest sync counter (the number of keepalive packets received by each player)
	// if anyone falls behind by more than m_SyncLimit keepalives we start the lag screen

	if( m_GameLoaded )
	{
		// calculate the largest sync counter

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetSyncCounter( ) > m_MaxSyncCounter )
				m_MaxSyncCounter = (*i)->GetSyncCounter( );
		}

		// check if anyone has started lagging
		// we consider a player to have started lagging if they're more than m_SyncLimit keepalives behind

		if( !m_Lagging )
		{
			string LaggingString;

			for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			{
				if( m_MaxSyncCounter - (*i)->GetSyncCounter( ) > m_SyncLimit )
				{
					(*i)->SetLagging( true );
					(*i)->SetStartedLaggingTicks( GetTicks( ) );
					m_Lagging = true;
					m_StartedLaggingTime = GetTime( );

					if( LaggingString.empty( ) )
						LaggingString = (*i)->GetName( );
					else
						LaggingString += ", " + (*i)->GetName( );
				}
			}

			if( m_Lagging )
			{
				// start the lag screen

				CONSOLE_Print( "[GAME: " + m_GameName + "] started lagging on [" + LaggingString + "]" );
				SendAll( m_Protocol->SEND_W3GS_START_LAG( m_Players ) );

				// reset everyone's drop vote

				for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
					(*i)->SetDropVote( false );
			}
		}

		if( m_Lagging )
		{
			// we cannot allow the lag screen to stay up for more than ~65 seconds because Warcraft III disconnects if it doesn't receive an action packet at least this often
			// one (easy) solution is to simply drop all the laggers if they lag for more than 60 seconds, which is what we do here

			if( GetTime( ) >= m_StartedLaggingTime + 60 )
				StopLaggers( "was automatically dropped after 60 seconds" );

			// check if anyone has stopped lagging normally
			// we consider a player to have stopped lagging if they're less than half m_SyncLimit keepalives behind

			for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			{
				if( (*i)->GetLagging( ) && m_MaxSyncCounter - (*i)->GetSyncCounter( ) < m_SyncLimit / 2 )
				{
					// stop the lag screen for this player

					CONSOLE_Print( "[GAME: " + m_GameName + "] stopped lagging on [" + (*i)->GetName( ) + "]" );
					SendAll( m_Protocol->SEND_W3GS_STOP_LAG( *i ) );
					(*i)->SetLagging( false );
					(*i)->SetStartedLaggingTicks( 0 );
				}
			}

			// check if everyone has stopped lagging

			bool Lagging = false;

			for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			{
				if( (*i)->GetLagging( ) )
					Lagging = true;
			}

			m_Lagging = Lagging;

			// reset m_LastActionSentTicks because we want the game to stop running while the lag screen is up

			m_LastActionSentTicks = GetTicks( );

			// keep track of the last lag screen time so we can avoid timing out players

			m_LastLagScreenTime = GetTime( );
		}
	}

	// send actions every m_Latency milliseconds
	// actions are at the heart of every Warcraft 3 game but luckily we don't need to know their contents to relay them
	// we queue player actions in EventPlayerAction then just resend them in batches to all players here

	if( m_GameLoaded && !m_Lagging && GetTicks( ) >= m_LastActionSentTicks + m_Latency )
		SendAllActions( );

	// expire the votekick

	if( !m_KickVotePlayer.empty( ) && GetTime( ) >= m_StartedKickVoteTime + 60 )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] votekick against player [" + m_KickVotePlayer + "] expired" );
		SendAllChat( m_GHost->m_Language->VoteKickExpired( m_KickVotePlayer ) );
		m_KickVotePlayer.clear( );
		m_StartedKickVoteTime = 0;
	}

	// start the gameover timer if there's only one player left

	if( m_Players.size( ) == 1 && m_FakePlayerPID == 255 && m_GameOverTime == 0 && ( m_GameLoading || m_GameLoaded ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] gameover timer started (one player left)" );
		m_GameOverTime = GetTime( );
	}

	// finish the gameover timer

	if( m_GameOverTime != 0 && GetTime( ) >= m_GameOverTime + 60 )
	{
		bool AlreadyStopped = true;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( !(*i)->GetDeleteMe( ) )
			{
				AlreadyStopped = false;
				break;
			}
		}

		if( !AlreadyStopped )
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] is over (gameover timer finished)" );
			StopPlayers( "was disconnected (gameover timer finished)" );
		}
	}

	// end the game if there aren't any players left

	if( m_Players.empty( ) && ( m_GameLoading || m_GameLoaded ) )
	{
		if( !m_Saving )
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] is over (no players left)" );
			SaveGameData( );
			m_Saving = true;
		}
		else if( IsGameDataSaved( ) )
			return true;
	}

	// accept new connections

	if( m_Socket )
	{
		CTCPSocket *NewSocket = m_Socket->Accept( (fd_set *)fd );

		if( NewSocket )
		{
			// check the IP blacklist

			if( m_IPBlackList.find( NewSocket->GetIPString( ) ) == m_IPBlackList.end( ) )
				m_Potentials.push_back( new CPotentialPlayer( m_Protocol, this, NewSocket ) );
			else
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] rejected connection from [" + NewSocket->GetIPString( ) + "] due to blacklist" );
				delete NewSocket;
			}
		}

		if( m_Socket->HasError( ) )
			return true;
	}

	return m_Exiting;
}

void CBaseGame :: UpdatePost( void *send_fd )
{
	// we need to manually call DoSend on each player now because CGamePlayer :: Update doesn't do it
	// this is in case player 2 generates a packet for player 1 during the update but it doesn't get sent because player 1 already finished updating
	// in reality since we're queueing actions it might not make a big difference but oh well

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetSocket( ) )
			(*i)->GetSocket( )->DoSend( (fd_set *)send_fd );
	}

	for( vector<CPotentialPlayer *> :: iterator i = m_Potentials.begin( ); i != m_Potentials.end( ); i++ )
	{
		if( (*i)->GetSocket( ) )
			(*i)->GetSocket( )->DoSend( (fd_set *)send_fd );
	}
}

void CBaseGame :: Send( CGamePlayer *player, BYTEARRAY data )
{
	if( player && player->GetSocket( ) )
		player->GetSocket( )->PutBytes( data );
}

void CBaseGame :: Send( unsigned char PID, BYTEARRAY data )
{
	Send( GetPlayerFromPID( PID ), data );
}

void CBaseGame :: Send( BYTEARRAY PIDs, BYTEARRAY data )
{
	for( unsigned int i = 0; i < PIDs.size( ); i++ )
		Send( PIDs[i], data );
}

void CBaseGame :: SendAll( BYTEARRAY data )
{
	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetSocket( ) )
			(*i)->GetSocket( )->PutBytes( data );
	}
}

void CBaseGame :: SendChat( unsigned char fromPID, CGamePlayer *player, string message )
{
	// send a private message to one player - it'll be marked [Private] in Warcraft 3

	if( player )
	{
		if( !m_GameLoading && !m_GameLoaded )
		{
			if( message.size( ) > 254 )
				message = message.substr( 0, 254 );

			Send( player, m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, UTIL_CreateByteArray( player->GetPID( ) ), 16, BYTEARRAY( ), message ) );
		}
		else
		{
			unsigned char ExtraFlags[] = { 3, 0, 0, 0 };

			// based on my limited testing it seems that the extra flags' first byte contains 3 plus the recipient's colour to denote a private message

			unsigned char SID = GetSIDFromPID( player->GetPID( ) );

			if( SID < m_Slots.size( ) )
				ExtraFlags[0] = 3 + m_Slots[SID].GetColour( );

			if( message.size( ) > 127 )
				message = message.substr( 0, 127 );

			Send( player, m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, UTIL_CreateByteArray( player->GetPID( ) ), 32, UTIL_CreateByteArray( ExtraFlags, 4 ), message ) );
		}
	}
}

void CBaseGame :: SendChat( unsigned char fromPID, unsigned char toPID, string message )
{
	SendChat( fromPID, GetPlayerFromPID( toPID ), message );
}

void CBaseGame :: SendChat( CGamePlayer *player, string message )
{
	SendChat( GetHostPID( ), player, message );
}

void CBaseGame :: SendChat( unsigned char toPID, string message )
{
	SendChat( GetHostPID( ), toPID, message );
}

void CBaseGame :: SendAllChat( unsigned char fromPID, string message )
{
	// send a public message to all players - it'll be marked [All] in Warcraft 3

	if( GetNumPlayers( ) > 0 )
	{
		if( !m_GameLoading && !m_GameLoaded )
		{
			if( message.size( ) > 254 )
				message = message.substr( 0, 254 );

			// this is a lobby ghost chat message

			SendAll( m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, GetPIDs( ), 16, BYTEARRAY( ), message ) );
		}
		else
		{
			if( message.size( ) > 127 )
				message = message.substr( 0, 127 );

			// this is an ingame ghost chat message, print it to the console

			CONSOLE_Print( "[GAME: " + m_GameName + "] [Local]: " + message );
			SendAll( m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, GetPIDs( ), 32, UTIL_CreateByteArray( (uint32_t)0, false ), message ) );

			if( m_Replay )
				m_Replay->AddChatMessage( fromPID, 32, 0, message );
		}
	}
}

void CBaseGame :: SendAllChat( string message )
{
	SendAllChat( GetHostPID( ), message );
}

void CBaseGame :: SendAllSlotInfo( )
{
	if( !m_GameLoading && !m_GameLoaded )
		SendAll( m_Protocol->SEND_W3GS_SLOTINFO( m_Slots, m_RandomSeed, m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM ? 3 : 0, m_Map->GetMapNumPlayers( ) ) );
}

void CBaseGame :: SendVirtualHostPlayerInfo( CGamePlayer *player )
{
	if( m_VirtualHostPID == 255 )
		return;

	BYTEARRAY IP;
	IP.push_back( 0 );
	IP.push_back( 0 );
	IP.push_back( 0 );
	IP.push_back( 0 );
	Send( player, m_Protocol->SEND_W3GS_PLAYERINFO( m_VirtualHostPID, m_VirtualHostName, IP, IP ) );
}

void CBaseGame :: SendFakePlayerInfo( CGamePlayer *player )
{
	if( m_FakePlayerPID == 255 )
		return;

	BYTEARRAY IP;
	IP.push_back( 0 );
	IP.push_back( 0 );
	IP.push_back( 0 );
	IP.push_back( 0 );
	Send( player, m_Protocol->SEND_W3GS_PLAYERINFO( m_FakePlayerPID, "FakePlayer", IP, IP ) );
}

void CBaseGame :: SendAllActions( )
{
	uint16_t SendInterval = GetTicks( ) - m_LastActionSentTicks;
	m_GameTicks += SendInterval;

	// add actions to replay

	if( m_Replay )
		m_Replay->AddTimeSlot( SendInterval, m_Actions );

	// we aren't allowed to send more than 1460 bytes in a single packet but it's possible we might have more than that many bytes waiting in the queue

	if( !m_Actions.empty( ) )
	{
		// we use a "sub actions queue" which we keep adding actions to until we reach the size limit
		// start by adding one action to the sub actions queue

		queue<CIncomingAction *> SubActions;
		CIncomingAction *Action = m_Actions.front( );
		m_Actions.pop( );
		SubActions.push( Action );
		uint32_t SubActionsLength = Action->GetLength( );

		while( !m_Actions.empty( ) )
		{
			Action = m_Actions.front( );
			m_Actions.pop( );

			// check if adding the next action to the sub actions queue would put us over the limit (1452 because the INCOMING_ACTION and INCOMING_ACTION2 packets use an extra 8 bytes)

			if( SubActionsLength + Action->GetLength( ) > 1452 )
			{
				// we'd be over the limit if we added the next action to the sub actions queue
				// so send everything already in the queue and then clear it out
				// the W3GS_INCOMING_ACTION2 packet handles the overflow but it must be sent *before* the corresponding W3GS_INCOMING_ACTION packet

				SendAll( m_Protocol->SEND_W3GS_INCOMING_ACTION2( SubActions ) );

				while( !SubActions.empty( ) )
				{
					delete SubActions.front( );
					SubActions.pop( );
				}

				SubActionsLength = 0;
			}

			SubActions.push( Action );
			SubActionsLength += Action->GetLength( );
		}

		SendAll( m_Protocol->SEND_W3GS_INCOMING_ACTION( SubActions, SendInterval ) );

		while( !SubActions.empty( ) )
		{
			delete SubActions.front( );
			SubActions.pop( );
		}
	}
	else
		SendAll( m_Protocol->SEND_W3GS_INCOMING_ACTION( m_Actions, SendInterval ) );

	m_LastActionSentTicks = GetTicks( );
}

void CBaseGame :: SendWelcomeMessage( CGamePlayer *player )
{
	// read from motd.txt if available (thanks to zeeg for this addition)

	ifstream in;
	in.open( m_GHost->m_MOTDFile.c_str( ) );

	if( in.fail( ) )
	{
		// default welcome message

		SendChat( player, " " );
		SendChat( player, " " );
		SendChat( player, " " );
		SendChat( player, "GHost++                                        http://forum.codelain.com/" );
		SendChat( player, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" );
		SendChat( player, "     Game Name:                 " + m_GameName );
		SendChat( player, "     HCL Command String:  " + m_HCLCommandString );
	}
	else
	{
		// custom welcome message
		// don't print more than 8 lines

		uint32_t Count = 0;
		string Line;

		while( !in.eof( ) && Count < 8 )
		{
			getline( in, Line );

			if( Line.empty( ) )
			{
				if( !in.eof( ) )
					SendChat( player, " " );
			}
			else
				SendChat( player, Line );

			Count++;
		}

		in.close( );
	}
}

void CBaseGame :: SendEndMessage( )
{
	// read from gameover.txt if available

	ifstream in;
	in.open( m_GHost->m_GameOverFile.c_str( ) );

	if( !in.fail( ) )
	{
		// don't print more than 8 lines

		uint32_t Count = 0;
		string Line;

		while( !in.eof( ) && Count < 8 )
		{
			getline( in, Line );

			if( Line.empty( ) )
			{
				if( !in.eof( ) )
					SendAllChat( " " );
			}
			else
				SendAllChat( Line );

			Count++;
		}

		in.close( );
	}
}

void CBaseGame :: EventPlayerDeleted( CGamePlayer *player )
{
	CONSOLE_Print( "[GAME: " + m_GameName + "] deleting player [" + player->GetName( ) + "]: " + player->GetLeftReason( ) );

	// in some cases we're forced to send the left message early so don't send it again

	if( player->GetLeftMessageSent( ) )
		return;

	if( m_GameLoaded )
		SendAllChat( player->GetName( ) + " " + player->GetLeftReason( ) + "." );

	if( player->GetLagging( ) )
		SendAll( m_Protocol->SEND_W3GS_STOP_LAG( player ) );

	// autosave

	if( m_GameLoaded && player->GetLeftCode( ) == PLAYERLEAVE_DISCONNECT && m_AutoSave )
	{
		string SaveGameName = UTIL_FileSafeName( "GHost++ AutoSave " + m_GameName + " (" + player->GetName( ) + ").w3z" );
		CONSOLE_Print( "[GAME: " + m_GameName + "] auto saving [" + SaveGameName + "] before player drop, shortened send interval = " + UTIL_ToString( GetTicks( ) - m_LastActionSentTicks ) );
		BYTEARRAY CRC;
		BYTEARRAY Action;
		Action.push_back( 6 );
		UTIL_AppendByteArray( Action, SaveGameName );
		m_Actions.push( new CIncomingAction( player->GetPID( ), CRC, Action ) );
		SendAllActions( );
	}

	if( m_GameLoading && m_LoadInGame )
	{
		// we must buffer player leave messages when using "load in game" to prevent desyncs
		// this ensures the player leave messages are correctly interleaved with the empty updates sent to each player

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetFinishedLoading( ) )
			{
				if( !player->GetFinishedLoading( ) )
					Send( *i, m_Protocol->SEND_W3GS_STOP_LAG( player ) );

				Send( *i, m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( player->GetPID( ), player->GetLeftCode( ) ) );
			}
			else
				UTIL_AppendByteArray( *(*i)->GetLoadInGameData( ), m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( player->GetPID( ), player->GetLeftCode( ) ) );
		}
	}
	else
	{
		// tell everyone about the player leaving

		SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( player->GetPID( ), player->GetLeftCode( ) ) );
	}

	// set the replay's host PID and name to the last player to leave the game
	// this will get overwritten as each player leaves the game so it will eventually be set to the last player

	if( m_Replay && ( m_GameLoading || m_GameLoaded ) )
	{
		m_Replay->SetHostPID( player->GetPID( ) );
		m_Replay->SetHostName( player->GetName( ) );

		// add leave message to replay

		if( m_GameLoading && !m_LoadInGame )
			m_Replay->AddLeaveGameDuringLoading( 1, player->GetPID( ), player->GetLeftCode( ) );
		else
			m_Replay->AddLeaveGame( 1, player->GetPID( ), player->GetLeftCode( ) );
	}

	// abort the countdown if there was one in progress

	if( m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
	{
		SendAllChat( m_GHost->m_Language->CountDownAborted( ) );
		m_CountDownStarted = false;
	}

	// abort the votekick

	if( !m_KickVotePlayer.empty( ) )
		SendAllChat( m_GHost->m_Language->VoteKickCancelled( m_KickVotePlayer ) );

	m_KickVotePlayer.clear( );
	m_StartedKickVoteTime = 0;
}

void CBaseGame :: EventPlayerDisconnectTimedOut( CGamePlayer *player )
{
	// not only do we not do any timeouts if the game is lagging, we allow for an additional grace period of 10 seconds
	// this is because Warcraft 3 stops sending packets during the lag screen
	// so when the lag screen finishes we would immediately disconnect everyone if we didn't give them some extra time

	if( GetTime( ) >= m_LastLagScreenTime + 10 )
	{
		player->SetDeleteMe( true );
		player->SetLeftReason( m_GHost->m_Language->HasLostConnectionTimedOut( ) );
		player->SetLeftCode( PLAYERLEAVE_DISCONNECT );

		if( !m_GameLoading && !m_GameLoaded )
			OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
	}
}

void CBaseGame :: EventPlayerDisconnectPlayerError( CGamePlayer *player )
{
	// at the time of this comment there's only one player error and that's when we receive a bad packet from the player
	// since TCP has checks and balances for data corruption the chances of this are pretty slim

	player->SetDeleteMe( true );
	player->SetLeftReason( m_GHost->m_Language->HasLostConnectionPlayerError( player->GetErrorString( ) ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );

	if( !m_GameLoading && !m_GameLoaded )
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
}

void CBaseGame :: EventPlayerDisconnectSocketError( CGamePlayer *player )
{
	player->SetDeleteMe( true );
	player->SetLeftReason( m_GHost->m_Language->HasLostConnectionSocketError( player->GetSocket( )->GetErrorString( ) ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );

	if( !m_GameLoading && !m_GameLoaded )
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
}

void CBaseGame :: EventPlayerDisconnectConnectionClosed( CGamePlayer *player )
{
	player->SetDeleteMe( true );
	player->SetLeftReason( m_GHost->m_Language->HasLostConnectionClosedByRemoteHost( ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );

	if( !m_GameLoading && !m_GameLoaded )
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
}

void CBaseGame :: EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer )
{
	// check if the new player's name is empty or too long

	if( joinPlayer->GetName( ).empty( ) || joinPlayer->GetName( ).size( ) > 15 )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game with an invalid name of length " + UTIL_ToString( joinPlayer->GetName( ).size( ) ) );
		potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->SetDeleteMe( true );
		return;
	}

	// check if the new player's name is the same as the virtual host name

	if( joinPlayer->GetName( ) == m_VirtualHostName )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game with the virtual host name" );
		potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->SetDeleteMe( true );
		return;
	}

	// check if the new player's name is already taken

	if( GetPlayerFromName( joinPlayer->GetName( ), false ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but that name is already taken" );
		// SendAllChat( m_GHost->m_Language->TryingToJoinTheGameButTaken( joinPlayer->GetName( ) ) );
		potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->SetDeleteMe( true );
		return;
	}

	// check if the new player's name is banned

	for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		CDBBan *Ban = (*i)->IsBannedName( joinPlayer->GetName( ) );

		if( Ban )
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but is banned by name" );
			SendAllChat( m_GHost->m_Language->TryingToJoinTheGameButBanned( joinPlayer->GetName( ) ) );

			if( m_GHost->m_BanMethod == 1 || m_GHost->m_BanMethod == 3 )
			{
				potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
				potential->SetDeleteMe( true );
				return;
			}

			break;
		}

		Ban = (*i)->IsBannedIP( potential->GetExternalIPString( ) );

		if( Ban )
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but is banned by IP address" );
			SendAllChat( m_GHost->m_Language->TryingToJoinTheGameButBanned( joinPlayer->GetName( ) ) );

			if( m_GHost->m_BanMethod == 2 || m_GHost->m_BanMethod == 3 )
			{
				potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
				potential->SetDeleteMe( true );
				return;
			}

			break;
		}
	}

	if( m_MatchMaking && m_AutoStartPlayers != 0 && !m_Map->GetMapMatchMakingCategory( ).empty( ) && m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM && m_GHost->m_BNETs.size( ) == 1 )
	{
		// matchmaking is enabled
		// start a database query to determine the player's score
		// when the query is complete we will call EventPlayerJoinedWithScore

		m_ScoreChecks.push_back( m_GHost->m_DB->ThreadedScoreCheck( m_Map->GetMapMatchMakingCategory( ), joinPlayer->GetName( ), m_GHost->m_BNETs[0]->GetServer( ) ) );
		return;
	}

	// try to find an empty slot

	unsigned char SID = GetEmptySlot( false );

	// check if the player is an admin or root admin on any connected realm for determining reserved status
	// we can't just use the spoof checked realm like in EventPlayerBotCommand because the player hasn't spoof checked yet

	bool AnyAdminCheck = false;

	for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		if( (*i)->IsAdmin( joinPlayer->GetName( ) ) || (*i)->IsRootAdmin( joinPlayer->GetName( ) ) )
		{
			AnyAdminCheck = true;
			break;
		}
	}

	bool Reserved = IsReserved( joinPlayer->GetName( ) ) || AnyAdminCheck || IsOwner( joinPlayer->GetName( ) );

	if( SID == 255 && Reserved )
	{
		// a reserved player is trying to join the game but it's full, try to find a reserved slot

		SID = GetEmptySlot( true );

		if( SID != 255 )
		{
			CGamePlayer *KickedPlayer = GetPlayerFromSID( SID );

			if( KickedPlayer )
			{
				KickedPlayer->SetDeleteMe( true );
				KickedPlayer->SetLeftReason( m_GHost->m_Language->WasKickedForReservedPlayer( joinPlayer->GetName( ) ) );
				KickedPlayer->SetLeftCode( PLAYERLEAVE_LOBBY );

				// send a playerleave message immediately since it won't normally get sent until the player is deleted which is after we send a playerjoin message
				// we don't need to call OpenSlot here because we're about to overwrite the slot data anyway

				SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( KickedPlayer->GetPID( ), KickedPlayer->GetLeftCode( ) ) );
				KickedPlayer->SetLeftMessageSent( true );
			}
		}
	}

	if( SID == 255 && IsOwner( joinPlayer->GetName( ) ) )
	{
		// the owner player is trying to join the game but it's full and we couldn't even find a reserved slot, kick the player in the lowest numbered slot
		// updated this to try to find a player slot so that we don't end up kicking a computer

		SID = 0;

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[i].GetComputer( ) == 0 )
			{
				SID = i;
				break;
			}
		}

		CGamePlayer *KickedPlayer = GetPlayerFromSID( SID );

		if( KickedPlayer )
		{
			KickedPlayer->SetDeleteMe( true );
			KickedPlayer->SetLeftReason( m_GHost->m_Language->WasKickedForOwnerPlayer( joinPlayer->GetName( ) ) );
			KickedPlayer->SetLeftCode( PLAYERLEAVE_LOBBY );

			// send a playerleave message immediately since it won't normally get sent until the player is deleted which is after we send a playerjoin message
			// we don't need to call OpenSlot here because we're about to overwrite the slot data anyway

			SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( KickedPlayer->GetPID( ), KickedPlayer->GetLeftCode( ) ) );
			KickedPlayer->SetLeftMessageSent( true );
		}
	}

	if( SID >= m_Slots.size( ) )
	{
		potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->SetDeleteMe( true );
		return;
	}

	// we have a slot for the new player
	// make room for them by deleting the virtual host player if we have to

	if( GetNumPlayers( ) >= 11 )
		DeleteVirtualHost( );

	// turning the CPotentialPlayer into a CGamePlayer is a bit of a pain because we have to be careful not to close the socket
	// this problem is solved by setting the socket to NULL before deletion and handling the NULL case in the destructor
	// we also have to be careful to not modify the m_Potentials vector since we're currently looping through it

	CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] joined the game" );
	CGamePlayer *Player = new CGamePlayer( potential, GetNewPID( ), joinPlayer->GetName( ), joinPlayer->GetInternalIP( ), Reserved );

	// consider the owner player to have already spoof checked
	// we will still attempt to spoof check them if it's enabled but this allows owners connecting over LAN to access admin commands

	if( IsOwner( joinPlayer->GetName( ) ) )
		Player->SetSpoofed( true );

	m_Players.push_back( Player );
	potential->SetSocket( NULL );
	potential->SetDeleteMe( true );

	if( m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM )
		m_Slots[SID] = CGameSlot( Player->GetPID( ), 255, SLOTSTATUS_OCCUPIED, 0, m_Slots[SID].GetTeam( ), m_Slots[SID].GetColour( ), m_Slots[SID].GetRace( ) );
	else
	{
		m_Slots[SID] = CGameSlot( Player->GetPID( ), 255, SLOTSTATUS_OCCUPIED, 0, 12, 12, SLOTRACE_RANDOM );

		// try to pick a team and colour
		// make sure there aren't too many other players already

		unsigned char NumOtherPlayers = 0;

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[i].GetTeam( ) != 12 )
				NumOtherPlayers++;
		}

		if( NumOtherPlayers < m_Map->GetMapNumPlayers( ) )
		{
			if( SID < m_Map->GetMapNumPlayers( ) )
				m_Slots[SID].SetTeam( SID );
			else
				m_Slots[SID].SetTeam( 0 );

			m_Slots[SID].SetColour( GetNewColour( ) );
		}
	}

	// send slot info to the new player
	// the SLOTINFOJOIN packet also tells the client their assigned PID and that the join was successful

	Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_SLOTINFOJOIN( Player->GetPID( ), Player->GetSocket( )->GetPort( ), Player->GetExternalIP( ), m_Slots, m_RandomSeed, m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM ? 3 : 0, m_Map->GetMapNumPlayers( ) ) );

	// send virtual host info and fake player info (if present) to the new player

	SendVirtualHostPlayerInfo( Player );
	SendFakePlayerInfo( Player );

	BYTEARRAY BlankIP;
	BlankIP.push_back( 0 );
	BlankIP.push_back( 0 );
	BlankIP.push_back( 0 );
	BlankIP.push_back( 0 );

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && *i != Player )
		{
			// send info about the new player to every other player

			if( (*i)->GetSocket( ) )
			{
				if( m_GHost->m_HideIPAddresses )
					(*i)->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_PLAYERINFO( Player->GetPID( ), Player->GetName( ), BlankIP, BlankIP ) );
				else
					(*i)->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_PLAYERINFO( Player->GetPID( ), Player->GetName( ), Player->GetExternalIP( ), Player->GetInternalIP( ) ) );
			}

			// send info about every other player to the new player

			if( m_GHost->m_HideIPAddresses )
				Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_PLAYERINFO( (*i)->GetPID( ), (*i)->GetName( ), BlankIP, BlankIP ) );
			else
				Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_PLAYERINFO( (*i)->GetPID( ), (*i)->GetName( ), (*i)->GetExternalIP( ), (*i)->GetInternalIP( ) ) );
		}
	}

	// send a map check packet to the new player

	Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_MAPCHECK( m_Map->GetMapPath( ), m_Map->GetMapSize( ), m_Map->GetMapInfo( ), m_Map->GetMapCRC( ), m_Map->GetMapSHA1( ) ) );

	// send slot info to everyone, so the new player gets this info twice but everyone else still needs to know the new slot layout

	SendAllSlotInfo( );

	// send a welcome message

	SendWelcomeMessage( Player );

	// check for multiple IP usage

	if( m_GHost->m_CheckMultipleIPUsage )
	{
		string Others;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( Player != *i && Player->GetExternalIPString( ) == (*i)->GetExternalIPString( ) )
			{
				if( Others.empty( ) )
					Others = (*i)->GetName( );
				else
					Others += ", " + (*i)->GetName( );
			}
		}

		if( !Others.empty( ) )
			SendAllChat( m_GHost->m_Language->MultipleIPAddressUsageDetected( joinPlayer->GetName( ), Others ) );
	}

	// abort the countdown if there was one in progress

	if( m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
	{
		SendAllChat( m_GHost->m_Language->CountDownAborted( ) );
		m_CountDownStarted = false;
	}

	// auto lock the game

	if( m_GHost->m_AutoLock && !m_Locked && IsOwner( joinPlayer->GetName( ) ) )
	{
		SendAllChat( m_GHost->m_Language->GameLocked( ) );
		m_Locked = true;
	}
}

void CBaseGame :: EventPlayerJoinedWithScore( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer, double score )
{
	// this function is only called when matchmaking is enabled
	// EventPlayerJoined will be called first in all cases
	// if matchmaking is enabled EventPlayerJoined will start a database query to retrieve the player's score and keep the connection open while we wait
	// when the database query is complete EventPlayerJoinedWithScore will be called

	// check if the new player's name is the same as the virtual host name

	if( joinPlayer->GetName( ) == m_VirtualHostName )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game with the virtual host name" );
		potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->SetDeleteMe( true );
		return;
	}

	// check if the new player's name is already taken

	if( GetPlayerFromName( joinPlayer->GetName( ), false ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but that name is already taken" );
		// SendAllChat( m_GHost->m_Language->TryingToJoinTheGameButTaken( joinPlayer->GetName( ) ) );
		potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->SetDeleteMe( true );
		return;
	}

	// check if the new player's score is within the limits

	if( score > -99999.0 && ( score < m_MinimumScore || score > m_MaximumScore ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but has a rating [" + UTIL_ToString( score, 2 ) + "] outside the limits [" + UTIL_ToString( m_MinimumScore, 2 ) + "] to [" + UTIL_ToString( m_MaximumScore, 2 ) + "]" );
		potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->SetDeleteMe( true );
		return;
	}

	// try to find an empty slot

	unsigned char SID = GetEmptySlot( false );

	if( SID == 255 )
	{
		// no empty slot found, time to do some matchmaking!
		// the general idea is that we're going to compute the average score of all players in the game
		// then we kick the player with the score furthest from that average (or a player without a score)
		// this ensures that the players' scores will tend to converge as players join the game
		// note: the database code uses a score of -100000 to denote "no score"

		// calculate the average score

		double AverageScore = 0.0;
		uint32_t PlayersScored = 0;

		if( score > -99999.0 )
		{
			AverageScore = score;
			PlayersScored = 1;
		}

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetScore( ) > -99999.0 )
			{
				AverageScore += (*i)->GetScore( );
				PlayersScored++;
			}
		}

		if( PlayersScored > 0 )
			AverageScore /= PlayersScored;

		// calculate the furthest player from the average

		CGamePlayer *FurthestPlayer = NULL;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( !FurthestPlayer || (*i)->GetScore( ) < -99999.0 || abs( (*i)->GetScore( ) - AverageScore ) > abs( FurthestPlayer->GetScore( ) - AverageScore ) )
				FurthestPlayer = *i;
		}

		if( !FurthestPlayer )
		{
			// this should be impossible

			CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but no furthest player was found (this should be impossible)" );
			potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
			potential->SetDeleteMe( true );
			return;
		}

		// kick the new player if they have the furthest score

		if( score < -99999.0 || abs( score - AverageScore ) > abs( FurthestPlayer->GetScore( ) - AverageScore ) )
		{
			if( score < -99999.0 )
				CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but has the furthest rating [N/A] from the average [" + UTIL_ToString( AverageScore, 2 ) + "]" );
			else
				CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but has the furthest rating [" + UTIL_ToString( score, 2 ) + "] from the average [" + UTIL_ToString( AverageScore, 2 ) + "]" );

			potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
			potential->SetDeleteMe( true );
			return;
		}

		// kick the furthest player

		SID = GetSIDFromPID( FurthestPlayer->GetPID( ) );
		FurthestPlayer->SetDeleteMe( true );

		if( FurthestPlayer->GetScore( ) < -99999.0 )
			FurthestPlayer->SetLeftReason( m_GHost->m_Language->WasKickedForHavingFurthestScore( "N/A", UTIL_ToString( AverageScore, 2 ) ) );
		else
			FurthestPlayer->SetLeftReason( m_GHost->m_Language->WasKickedForHavingFurthestScore( UTIL_ToString( FurthestPlayer->GetScore( ), 2 ), UTIL_ToString( AverageScore, 2 ) ) );

		FurthestPlayer->SetLeftCode( PLAYERLEAVE_LOBBY );

		// send a playerleave message immediately since it won't normally get sent until the player is deleted which is after we send a playerjoin message
		// we don't need to call OpenSlot here because we're about to overwrite the slot data anyway

		SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( FurthestPlayer->GetPID( ), FurthestPlayer->GetLeftCode( ) ) );
		FurthestPlayer->SetLeftMessageSent( true );

		if( FurthestPlayer->GetScore( ) < -99999.0 )
			SendAllChat( "Player [" + FurthestPlayer->GetName( ) + "] was kicked for having the furthest rating [N/A] from the average [" + UTIL_ToString( AverageScore, 2 ) + "]" );
		else
			SendAllChat( "Player [" + FurthestPlayer->GetName( ) + "] was kicked for having the furthest rating [" + UTIL_ToString( FurthestPlayer->GetScore( ), 2 ) + "] from the average [" + UTIL_ToString( AverageScore, 2 ) + "]" );
	}

	if( SID >= m_Slots.size( ) )
	{
		potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->SetDeleteMe( true );
		return;
	}

	// we have a slot for the new player
	// make room for them by deleting the virtual host player if we have to

	if( GetNumPlayers( ) >= 11 )
		DeleteVirtualHost( );

	// turning the CPotentialPlayer into a CGamePlayer is a bit of a pain because we have to be careful not to close the socket
	// this problem is solved by setting the socket to NULL before deletion and handling the NULL case in the destructor
	// we also have to be careful to not modify the m_Potentials vector since we're currently looping through it

	CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] joined the game" );
	CGamePlayer *Player = new CGamePlayer( potential, GetNewPID( ), joinPlayer->GetName( ), joinPlayer->GetInternalIP( ), false );
	Player->SetScore( score );
	m_Players.push_back( Player );
	potential->SetSocket( NULL );
	potential->SetDeleteMe( true );
	m_Slots[SID] = CGameSlot( Player->GetPID( ), 255, SLOTSTATUS_OCCUPIED, 0, m_Slots[SID].GetTeam( ), m_Slots[SID].GetColour( ), m_Slots[SID].GetRace( ) );

	// send slot info to the new player
	// the SLOTINFOJOIN packet also tells the client their assigned PID and that the join was successful

	Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_SLOTINFOJOIN( Player->GetPID( ), Player->GetSocket( )->GetPort( ), Player->GetExternalIP( ), m_Slots, m_RandomSeed, m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM ? 3 : 0, m_Map->GetMapNumPlayers( ) ) );

	// send virtual host info and fake player info (if present) to the new player

	SendVirtualHostPlayerInfo( Player );
	SendFakePlayerInfo( Player );

	BYTEARRAY BlankIP;
	BlankIP.push_back( 0 );
	BlankIP.push_back( 0 );
	BlankIP.push_back( 0 );
	BlankIP.push_back( 0 );

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && *i != Player )
		{
			// send info about the new player to every other player

			if( (*i)->GetSocket( ) )
			{
				if( m_GHost->m_HideIPAddresses )
					(*i)->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_PLAYERINFO( Player->GetPID( ), Player->GetName( ), BlankIP, BlankIP ) );
				else
					(*i)->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_PLAYERINFO( Player->GetPID( ), Player->GetName( ), Player->GetExternalIP( ), Player->GetInternalIP( ) ) );
			}

			// send info about every other player to the new player

			if( m_GHost->m_HideIPAddresses )
				Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_PLAYERINFO( (*i)->GetPID( ), (*i)->GetName( ), BlankIP, BlankIP ) );
			else
				Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_PLAYERINFO( (*i)->GetPID( ), (*i)->GetName( ), (*i)->GetExternalIP( ), (*i)->GetInternalIP( ) ) );
		}
	}

	// send a map check packet to the new player

	Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_MAPCHECK( m_Map->GetMapPath( ), m_Map->GetMapSize( ), m_Map->GetMapInfo( ), m_Map->GetMapCRC( ), m_Map->GetMapSHA1( ) ) );

	// send slot info to everyone, so the new player gets this info twice but everyone else still needs to know the new slot layout

	SendAllSlotInfo( );

	// send a welcome message

	SendWelcomeMessage( Player );

	if( score < -99999.0 )
		SendAllChat( m_GHost->m_Language->PlayerHasScore( joinPlayer->GetName( ), "N/A" ) );
	else
		SendAllChat( m_GHost->m_Language->PlayerHasScore( joinPlayer->GetName( ), UTIL_ToString( score, 2 ) ) );

	uint32_t PlayersScored = 0;
	uint32_t PlayersNotScored = 0;
	double AverageScore = 0.0;
	double MinScore = 0.0;
	double MaxScore = 0.0;
	bool Found = false;

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
		{
			if( (*i)->GetScore( ) < -99999.0 )
				PlayersNotScored++;
			else
			{
				PlayersScored++;
				AverageScore += (*i)->GetScore( );

				if( !Found || (*i)->GetScore( ) < MinScore )
					MinScore = (*i)->GetScore( );

				if( !Found || (*i)->GetScore( ) > MaxScore )
					MaxScore = (*i)->GetScore( );

				Found = true;
			}
		}
	}

	double Spread = MaxScore - MinScore;
	SendAllChat( m_GHost->m_Language->RatedPlayersSpread( UTIL_ToString( PlayersScored ), UTIL_ToString( PlayersScored + PlayersNotScored ), UTIL_ToString( (uint32_t)Spread ) ) );

	// check for multiple IP usage

	if( m_GHost->m_CheckMultipleIPUsage )
	{
		string Others;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( Player != *i && Player->GetExternalIPString( ) == (*i)->GetExternalIPString( ) )
			{
				if( Others.empty( ) )
					Others = (*i)->GetName( );
				else
					Others += ", " + (*i)->GetName( );
			}
		}

		if( !Others.empty( ) )
			SendAllChat( m_GHost->m_Language->MultipleIPAddressUsageDetected( joinPlayer->GetName( ), Others ) );
	}

	// abort the countdown if there was one in progress

	if( m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
	{
		SendAllChat( m_GHost->m_Language->CountDownAborted( ) );
		m_CountDownStarted = false;
	}

	// auto lock the game

	if( m_GHost->m_AutoLock && !m_Locked && IsOwner( joinPlayer->GetName( ) ) )
	{
		SendAllChat( m_GHost->m_Language->GameLocked( ) );
		m_Locked = true;
	}

	// balance the slots

	if( m_AutoStartPlayers != 0 && GetNumPlayers( ) == m_AutoStartPlayers )
		BalanceSlots( );
}

void CBaseGame :: EventPlayerLeft( CGamePlayer *player )
{
	// this function is only called when a player leave packet is received, not when there's a socket error, kick, etc...

	player->SetDeleteMe( true );
	player->SetLeftReason( m_GHost->m_Language->HasLeftVoluntarily( ) );
	player->SetLeftCode( PLAYERLEAVE_LOST );

	if( !m_GameLoading && !m_GameLoaded )
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
}

void CBaseGame :: EventPlayerLoaded( CGamePlayer *player )
{
	CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + player->GetName( ) + "] finished loading in " + UTIL_ToString( (float)( player->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks ) / 1000, 2 ) + " seconds" );

	if( m_LoadInGame )
	{
		// send any buffered data to the player now
		// see the Update function for more information about why we do this
		// this includes player loaded messages, game updates, and player leave messages

		Send( player, *player->GetLoadInGameData( ) );
		player->GetLoadInGameData( )->clear( );

		// start the lag screen for the new player

		bool FinishedLoading = true;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			FinishedLoading = (*i)->GetFinishedLoading( );

			if( !FinishedLoading )
				break;
		}

		if( !FinishedLoading )
			Send( player, m_Protocol->SEND_W3GS_START_LAG( m_Players, true ) );

		// remove the new player from previously loaded players' lag screens

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( *i != player && (*i)->GetFinishedLoading( ) )
				Send( *i, m_Protocol->SEND_W3GS_STOP_LAG( player ) );
		}

		// send a chat message to previously loaded players

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( *i != player && (*i)->GetFinishedLoading( ) )
				SendChat( *i, m_GHost->m_Language->PlayerFinishedLoading( player->GetName( ) ) );
		}

		if( !FinishedLoading )
			SendChat( player, m_GHost->m_Language->PleaseWaitPlayersStillLoading( ) );
	}
	else
		SendAll( m_Protocol->SEND_W3GS_GAMELOADED_OTHERS( player->GetPID( ) ) );
}

void CBaseGame :: EventPlayerAction( CGamePlayer *player, CIncomingAction *action )
{
	m_Actions.push( action );

	// check for players saving the game and notify everyone

	if( !action->GetAction( ).empty( ) && action->GetAction( )[0] == 6 )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + player->GetName( ) + "] is saving the game" );
		SendAllChat( m_GHost->m_Language->PlayerIsSavingTheGame( player->GetName( ) ) );
	}
}

void CBaseGame :: EventPlayerKeepAlive( CGamePlayer *player, uint32_t checkSum )
{
	// check for desyncs

	uint32_t FirstCheckSum = player->GetCheckSums( )->front( );

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetCheckSums( )->empty( ) )
			return;

		if( !m_Desynced && (*i)->GetCheckSums( )->front( ) != FirstCheckSum )
		{
			m_Desynced = true;
			CONSOLE_Print( "[GAME: " + m_GameName + "] desync detected" );
			SendAllChat( m_GHost->m_Language->DesyncDetected( ) );
			SendAllChat( m_GHost->m_Language->DesyncDetected( ) );
			SendAllChat( m_GHost->m_Language->DesyncDetected( ) );
		}
	}

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		(*i)->GetCheckSums( )->pop( );

	// add checksum to replay but only if we're not desynced

	if( m_Replay && !m_Desynced )
		m_Replay->AddCheckSum( FirstCheckSum );
}

void CBaseGame :: EventPlayerChatToHost( CGamePlayer *player, CIncomingChatPlayer *chatPlayer )
{
	if( chatPlayer->GetFromPID( ) == player->GetPID( ) )
	{
		if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_MESSAGE || chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_MESSAGEEXTRA )
		{
			// relay the chat message to other players

			bool Relay = !player->GetMuted( );
			BYTEARRAY ExtraFlags = chatPlayer->GetExtraFlags( );

			// calculate timestamp

			string MinString = UTIL_ToString( ( m_GameTicks / 1000 ) / 60 );
			string SecString = UTIL_ToString( ( m_GameTicks / 1000 ) % 60 );

			if( MinString.size( ) == 1 )
				MinString.insert( 0, "0" );

			if( SecString.size( ) == 1 )
				SecString.insert( 0, "0" );

			if( !ExtraFlags.empty( ) )
			{
				if( ExtraFlags[0] == 0 )
				{
					// this is an ingame [All] message, print it to the console

					CONSOLE_Print( "[GAME: " + m_GameName + "] (" + MinString + ":" + SecString + ") [All] [" + player->GetName( ) + "]: " + chatPlayer->GetMessage( ) );

					// don't relay ingame messages targeted for all players if we're currently muting all
					// note that commands will still be processed even when muting all because we only stop relaying the messages, the rest of the function is unaffected

					if( m_MuteAll )
						Relay = false;
				}

				if( Relay )
				{
					// add chat message to replay
					// this includes allied chat and private chat from both teams as long as it was relayed

					if( m_Replay )
						m_Replay->AddChatMessage( chatPlayer->GetFromPID( ), chatPlayer->GetFlag( ), UTIL_ByteArrayToUInt32( chatPlayer->GetExtraFlags( ), false ), chatPlayer->GetMessage( ) );
				}
			}
			else
			{
				// this is a lobby message, print it to the console

				CONSOLE_Print( "[GAME: " + m_GameName + "] [Lobby] [" + player->GetName( ) + "]: " + chatPlayer->GetMessage( ) );

				if( m_MuteLobby )
					Relay = false;
			}

			if( Relay )
				Send( chatPlayer->GetToPIDs( ), m_Protocol->SEND_W3GS_CHAT_FROM_HOST( chatPlayer->GetFromPID( ), chatPlayer->GetToPIDs( ), chatPlayer->GetFlag( ), chatPlayer->GetExtraFlags( ), chatPlayer->GetMessage( ) ) );

			// handle bot commands

			string Message = chatPlayer->GetMessage( );

			if( !Message.empty( ) && Message[0] == m_GHost->m_CommandTrigger )
			{
				// extract the command trigger, the command, and the payload
				// e.g. "!say hello world" -> command: "say", payload: "hello world"

				string Command;
				string Payload;
				string :: size_type PayloadStart = Message.find( " " );

				if( PayloadStart != string :: npos )
				{
					Command = Message.substr( 1, PayloadStart - 1 );
					Payload = Message.substr( PayloadStart + 1 );
				}
				else
					Command = Message.substr( 1 );

				transform( Command.begin( ), Command.end( ), Command.begin( ), (int(*)(int))tolower );
				EventPlayerBotCommand( player, Command, Payload );
			}
		}
		else if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_TEAMCHANGE && !m_CountDownStarted )
			EventPlayerChangeTeam( player, chatPlayer->GetByte( ) );
		else if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_COLOURCHANGE && !m_CountDownStarted )
			EventPlayerChangeColour( player, chatPlayer->GetByte( ) );
		else if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_RACECHANGE && !m_CountDownStarted )
			EventPlayerChangeRace( player, chatPlayer->GetByte( ) );
		else if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_HANDICAPCHANGE && !m_CountDownStarted )
			EventPlayerChangeHandicap( player, chatPlayer->GetByte( ) );
	}
}

void CBaseGame :: EventPlayerBotCommand( CGamePlayer *player, string command, string payload )
{

}

void CBaseGame :: EventPlayerChangeTeam( CGamePlayer *player, unsigned char team )
{
	// player is requesting a team change

	if( m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM )
	{
		unsigned char oldSID = GetSIDFromPID( player->GetPID( ) );
		unsigned char newSID = GetEmptySlot( team, player->GetPID( ) );
		SwapSlots( oldSID, newSID );
	}
	else
	{
		if( team > 12 )
			return;

		if( team == 12 )
		{
			if( m_Map->GetMapObservers( ) != MAPOBS_ALLOWED && m_Map->GetMapObservers( ) != MAPOBS_REFEREES )
				return;
		}
		else
		{
			if( team >= m_Map->GetMapNumPlayers( ) )
				return;

			// make sure there aren't too many other players already

			unsigned char NumOtherPlayers = 0;

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[i].GetTeam( ) != 12 && m_Slots[i].GetPID( ) != player->GetPID( ) )
					NumOtherPlayers++;
			}

			if( NumOtherPlayers >= m_Map->GetMapNumPlayers( ) )
				return;
		}

		unsigned char SID = GetSIDFromPID( player->GetPID( ) );

		if( SID < m_Slots.size( ) )
		{
			m_Slots[SID].SetTeam( team );

			if( team == 12 )
			{
				// if they're joining the observer team give them the observer colour

				m_Slots[SID].SetColour( 12 );
			}
			else if( m_Slots[SID].GetColour( ) == 12 )
			{
				// if they're joining a regular team give them an unused colour

				m_Slots[SID].SetColour( GetNewColour( ) );
			}

			SendAllSlotInfo( );
		}
	}
}

void CBaseGame :: EventPlayerChangeColour( CGamePlayer *player, unsigned char colour )
{
	// player is requesting a colour change

	if( m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM )
		return;

	if( colour > 11 )
		return;

	unsigned char SID = GetSIDFromPID( player->GetPID( ) );

	if( SID < m_Slots.size( ) )
	{
		// make sure the player isn't an observer

		if( m_Slots[SID].GetTeam( ) == 12 )
			return;

		ColourSlot( SID, colour );
	}
}

void CBaseGame :: EventPlayerChangeRace( CGamePlayer *player, unsigned char race )
{
	// player is requesting a race change

	if( m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM )
		return;

	if( m_Map->GetMapFlags( ) & MAPFLAG_RANDOMRACES )
		return;

	if( race != SLOTRACE_HUMAN && race != SLOTRACE_ORC && race != SLOTRACE_NIGHTELF && race != SLOTRACE_UNDEAD && race != SLOTRACE_RANDOM )
		return;

	unsigned char SID = GetSIDFromPID( player->GetPID( ) );

	if( SID < m_Slots.size( ) )
	{
		m_Slots[SID].SetRace( race );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: EventPlayerChangeHandicap( CGamePlayer *player, unsigned char handicap )
{
	// player is requesting a handicap change

	if( m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM )
		return;

	if( handicap != 50 && handicap != 60 && handicap != 70 && handicap != 80 && handicap != 90 && handicap != 100 )
		return;

	unsigned char SID = GetSIDFromPID( player->GetPID( ) );

	if( SID < m_Slots.size( ) )
	{
		m_Slots[SID].SetHandicap( handicap );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: EventPlayerDropRequest( CGamePlayer *player )
{
	// todotodo: check that we've waited the full 45 seconds

	if( m_Lagging )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + player->GetName( ) + "] voted to drop laggers" );
		SendAllChat( m_GHost->m_Language->PlayerVotedToDropLaggers( player->GetName( ) ) );

		// check if at least half the players voted to drop

		uint32_t Votes = 0;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetDropVote( ) )
				Votes++;
		}

		if( (float)Votes / m_Players.size( ) > 0.49 )
			StopLaggers( m_GHost->m_Language->LaggedOutDroppedByVote( ) );
	}
}

void CBaseGame :: EventPlayerMapSize( CGamePlayer *player, CIncomingMapSize *mapSize )
{
	if( m_GameLoading || m_GameLoaded )
		return;

	// todotodo: the variable names here are confusing due to extremely poor design on my part

	uint32_t MapSize = UTIL_ByteArrayToUInt32( m_Map->GetMapSize( ), false );

	if( mapSize->GetSizeFlag( ) != 1 || mapSize->GetMapSize( ) != MapSize )
	{
		// the player doesn't have the map

		if( m_GHost->m_AllowDownloads != 0 )
		{
			string *MapData = m_Map->GetMapData( );

			if( !MapData->empty( ) )
			{
				if( m_GHost->m_AllowDownloads == 1 || ( m_GHost->m_AllowDownloads == 2 && player->GetDownloadAllowed( ) ) )
				{
					if( !player->GetDownloadStarted( ) && mapSize->GetSizeFlag( ) == 1 )
					{
						// inform the client that we are willing to send the map

						CONSOLE_Print( "[GAME: " + m_GameName + "] map download started for player [" + player->GetName( ) + "]" );
						Send( player, m_Protocol->SEND_W3GS_STARTDOWNLOAD( GetHostPID( ) ) );
						player->SetDownloadStarted( true );
						player->SetStartedDownloadingTicks( GetTicks( ) );
					}
					else
						player->SetLastMapPartAcked( mapSize->GetMapSize( ) );
				}
			}
			else
			{
				player->SetDeleteMe( true );
				player->SetLeftReason( "doesn't have the map and there is no local copy of the map to send" );
				player->SetLeftCode( PLAYERLEAVE_LOBBY );
				OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
			}
		}
		else
		{
			player->SetDeleteMe( true );
			player->SetLeftReason( "doesn't have the map and map downloads are disabled" );
			player->SetLeftCode( PLAYERLEAVE_LOBBY );
			OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
		}
	}
	else
	{
		if( player->GetDownloadStarted( ) )
		{
			// calculate download rate

			float Seconds = (float)( GetTicks( ) - player->GetStartedDownloadingTicks( ) ) / 1000;
			float Rate = (float)MapSize / 1024 / Seconds;
			CONSOLE_Print( "[GAME: " + m_GameName + "] map download finished for player [" + player->GetName( ) + "] in " + UTIL_ToString( Seconds, 1 ) + " seconds" );
			SendAllChat( m_GHost->m_Language->PlayerDownloadedTheMap( player->GetName( ), UTIL_ToString( Seconds, 1 ), UTIL_ToString( Rate, 1 ) ) );
			player->SetDownloadFinished( true );
			player->SetFinishedDownloadingTime( GetTime( ) );

			// add to database

			m_GHost->m_Callables.push_back( m_GHost->m_DB->ThreadedDownloadAdd( m_Map->GetMapPath( ), MapSize, player->GetName( ), player->GetExternalIPString( ), player->GetSpoofed( ) ? 1 : 0, player->GetSpoofedRealm( ), GetTicks( ) - player->GetStartedDownloadingTicks( ) ) );
		}
	}

	unsigned char NewDownloadStatus = (unsigned char)( (float)mapSize->GetMapSize( ) / MapSize * 100 );
	unsigned char SID = GetSIDFromPID( player->GetPID( ) );

	if( NewDownloadStatus > 100 )
		NewDownloadStatus = 100;

	if( SID < m_Slots.size( ) )
	{
		// only send the slot info if the download status changed

		if( m_Slots[SID].GetDownloadStatus( ) != NewDownloadStatus )
		{
			m_Slots[SID].SetDownloadStatus( NewDownloadStatus );
			SendAllSlotInfo( );
		}
	}
}

void CBaseGame :: EventPlayerPongToHost( CGamePlayer *player, uint32_t pong )
{
	// autokick players with excessive pings but only if they're not reserved and we've received at least 3 pings from them
	// also don't kick anyone if the game is loading or loaded - this could happen because we send pings during loading but we stop sending them after the game is loaded
	// see the Update function for where we send pings

	if( !m_GameLoading && !m_GameLoaded && !player->GetReserved( ) && player->GetNumPings( ) >= 3 && player->GetPing( m_GHost->m_LCPings ) > m_GHost->m_AutoKickPing )
	{
		// send a chat message because we don't normally do so when a player leaves the lobby

		SendAllChat( m_GHost->m_Language->AutokickingPlayerForExcessivePing( player->GetName( ), UTIL_ToString( player->GetPing( m_GHost->m_LCPings ) ) ) );
		player->SetDeleteMe( true );
		player->SetLeftReason( "was autokicked for excessive ping of " + UTIL_ToString( player->GetPing( m_GHost->m_LCPings ) ) );
		player->SetLeftCode( PLAYERLEAVE_LOBBY );
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
	}
}

void CBaseGame :: EventGameRefreshed( string server )
{
	if( m_RefreshRehosted )
	{
		// we're not actually guaranteed this refresh was for the rehosted game and not the previous one
		// but since we unqueue game refreshes when rehosting, the only way this can happen is due to network delay
		// it's a risk we're willing to take but can result in a false positive here

		SendAllChat( m_GHost->m_Language->RehostWasSuccessful( ) );
		m_RefreshRehosted = false;
	}
}

void CBaseGame :: EventGameStarted( )
{
	CONSOLE_Print( "[GAME: " + m_GameName + "] started loading with " + UTIL_ToString( GetNumPlayers( ) ) + " players" );

	// encode the HCL command string in the slot handicaps
	// here's how it works:
	//  the user inputs a command string to be sent to the map
	//  it is almost impossible to send a message from the bot to the map so we encode the command string in the slot handicaps
	//  this works because there are only 6 valid handicaps but Warcraft III allows the bot to set up to 256 handicaps
	//  we encode the original (unmodified) handicaps in the new handicaps and use the remaining space to store a short message
	//  only occupied slots deliver their handicaps to the map and we can send one character (from a list) per handicap
	//  when the map finishes loading, assuming it's designed to use the HCL system, it checks if anyone has an invalid handicap
	//  if so, it decodes the message from the handicaps and restores the original handicaps using the encoded values
	//  the meaning of the message is specific to each map and the bot doesn't need to understand it
	//  e.g. you could send game modes, # of rounds, level to start on, anything you want as long as it fits in the limited space available
	//  note: if you attempt to use the HCL system on a map that does not support HCL the bot will drastically modify the handicaps
	//  since the map won't automatically restore the original handicaps in this case your game will be ruined

	if( !m_HCLCommandString.empty( ) )
	{
		if( m_HCLCommandString.size( ) <= GetSlotsOccupied( ) )
		{
			string HCLChars = "abcdefghijklmnopqrstuvwxyz0123456789 -=,.";

			if( m_HCLCommandString.find_first_not_of( HCLChars ) == string :: npos )
			{
				unsigned char EncodingMap[256];
				unsigned char j = 0;

				for( uint32_t i = 0; i < 256; i++ )
				{
					// the following 7 handicap values are forbidden

					if( j == 0 || j == 50 || j == 60 || j == 70 || j == 80 || j == 90 || j == 100 )
						j++;

					EncodingMap[i] = j++;
				}

				unsigned char CurrentSlot = 0;

				for( string :: iterator si = m_HCLCommandString.begin( ); si != m_HCLCommandString.end( ); si++ )
				{
					while( m_Slots[CurrentSlot].GetSlotStatus( ) != SLOTSTATUS_OCCUPIED )
						CurrentSlot++;

					unsigned char HandicapIndex = ( m_Slots[CurrentSlot].GetHandicap( ) - 50 ) / 10;
					unsigned char CharIndex = HCLChars.find( *si );
					m_Slots[CurrentSlot++].SetHandicap( EncodingMap[HandicapIndex + CharIndex * 6] );
				}

				SendAllSlotInfo( );
				CONSOLE_Print( "[GAME: " + m_GameName + "] successfully encoded HCL command string [" + m_HCLCommandString + "]" );
			}
			else
				CONSOLE_Print( "[GAME: " + m_GameName + "] encoding HCL command string [" + m_HCLCommandString + "] failed because it contains invalid characters" );
		}
		else
			CONSOLE_Print( "[GAME: " + m_GameName + "] encoding HCL command string [" + m_HCLCommandString + "] failed because there aren't enough occupied slots" );
	}

	m_StartedLoadingTicks = GetTicks( );
	m_LastLoadInGameResetTime = GetTime( );
	m_GameLoading = true;

	// since we use a fake countdown to deal with leavers during countdown the COUNTDOWN_START and COUNTDOWN_END packets are sent in quick succession
	// send a start countdown packet

	SendAll( m_Protocol->SEND_W3GS_COUNTDOWN_START( ) );

	// remove the virtual host player

	DeleteVirtualHost( );

	// send an end countdown packet

	SendAll( m_Protocol->SEND_W3GS_COUNTDOWN_END( ) );

	// send a game loaded packet for the fake player (if present)

	if( m_FakePlayerPID != 255 )
		SendAll( m_Protocol->SEND_W3GS_GAMELOADED_OTHERS( m_FakePlayerPID ) );

	// record the starting number of players

	m_StartPlayers = GetNumPlayers( );

	// close the listening socket

	delete m_Socket;
	m_Socket = NULL;

	// delete any potential players that are still hanging around

	for( vector<CPotentialPlayer *> :: iterator i = m_Potentials.begin( ); i != m_Potentials.end( ); i++ )
		delete *i;

	m_Potentials.clear( );

	// set initial values for replay

	if( m_Replay )
	{
		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			m_Replay->AddPlayer( (*i)->GetPID( ), (*i)->GetName( ) );

		m_Replay->SetSlots( m_Slots );
		m_Replay->SetRandomSeed( m_RandomSeed );
		m_Replay->SetSelectMode( m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM ? 3 : 0 );
		m_Replay->SetStartSpotCount( m_Map->GetMapNumPlayers( ) );
		m_Replay->SetMapGameType( m_Map->GetMapGameType( ) );

		if( !m_Players.empty( ) )
		{
			// this might not be necessary since we're going to overwrite the replay's host PID and name everytime a player leaves

			m_Replay->SetHostPID( m_Players[0]->GetPID( ) );
			m_Replay->SetHostName( m_Players[0]->GetName( ) );
		}
	}

	// build a stat string for use when saving the replay
	// we have to build this now because the map data could change now that the game has started

	BYTEARRAY StatString;
	UTIL_AppendByteArray( StatString, m_Map->GetMapGameFlags( ) );
	StatString.push_back( 0 );
	UTIL_AppendByteArray( StatString, m_Map->GetMapWidth( ) );
	UTIL_AppendByteArray( StatString, m_Map->GetMapHeight( ) );
	UTIL_AppendByteArray( StatString, m_Map->GetMapCRC( ) );
	UTIL_AppendByteArray( StatString, m_Map->GetMapPath( ) );
	UTIL_AppendByteArray( StatString, "GHost++" );
	StatString.push_back( 0 );
	UTIL_AppendByteArray( StatString, m_Map->GetMapSHA1( ) );		// note: in replays generated by Warcraft III it stores 20 zeros for the SHA1 instead of the real thing
	StatString = UTIL_EncodeStatString( StatString );
	m_StatString = string( StatString.begin( ), StatString.end( ) );

	if( m_LoadInGame )
	{
		// buffer all the player loaded messages
		// this ensures that every player receives the same set of player loaded messages in the same order, even if someone leaves during loading
		// if someone leaves during loading we buffer the leave message to ensure it gets sent in the correct position but the player loaded message wouldn't get sent if we didn't buffer it now

		BYTEARRAY Buffer;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			UTIL_AppendByteArray( Buffer, m_Protocol->SEND_W3GS_GAMELOADED_OTHERS( (*i)->GetPID( ) ) );

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			UTIL_AppendByteArray( *(*i)->GetLoadInGameData( ), Buffer );
	}

	// move the game to the games in progress vector

	m_GHost->m_CurrentGame = NULL;
	m_GHost->m_Games.push_back( this );

	// and finally reenter battle.net chat

	for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		(*i)->QueueGameUncreate( );
		(*i)->QueueEnterChat( );
	}
}

void CBaseGame :: EventGameLoaded( )
{
	CONSOLE_Print( "[GAME: " + m_GameName + "] finished loading with " + UTIL_ToString( GetNumPlayers( ) ) + " players" );

	// send shortest, longest, and personal load times to each player

	CGamePlayer *Shortest = NULL;
	CGamePlayer *Longest = NULL;

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !Shortest || (*i)->GetFinishedLoadingTicks( ) < Shortest->GetFinishedLoadingTicks( ) )
			Shortest = *i;

		if( !Longest || (*i)->GetFinishedLoadingTicks( ) > Longest->GetFinishedLoadingTicks( ) )
			Longest = *i;
	}

	if( Shortest && Longest )
	{
		SendAllChat( m_GHost->m_Language->ShortestLoadByPlayer( Shortest->GetName( ), UTIL_ToString( (float)( Shortest->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks ) / 1000, 2 ) ) );
		SendAllChat( m_GHost->m_Language->LongestLoadByPlayer( Longest->GetName( ), UTIL_ToString( (float)( Longest->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks ) / 1000, 2 ) ) );
	}

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		SendChat( *i, m_GHost->m_Language->YourLoadingTimeWas( UTIL_ToString( (float)( (*i)->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks ) / 1000, 2 ) ) );

	// read from gameloaded.txt if available

	ifstream in;
	in.open( m_GHost->m_GameLoadedFile.c_str( ) );

	if( !in.fail( ) )
	{
		// don't print more than 8 lines

		uint32_t Count = 0;
		string Line;

		while( !in.eof( ) && Count < 8 )
		{
			getline( in, Line );

			if( Line.empty( ) )
			{
				if( !in.eof( ) )
					SendAllChat( " " );
			}
			else
				SendAllChat( Line );

			Count++;
		}

		in.close( );
	}
}

unsigned char CBaseGame :: GetSIDFromPID( unsigned char PID )
{
	if( m_Slots.size( ) > 255 )
		return 255;

	for( unsigned char i = 0; i < m_Slots.size( ); i++ )
	{
		if( m_Slots[i].GetPID( ) == PID )
			return i;
	}

	return 255;
}

CGamePlayer *CBaseGame :: GetPlayerFromPID( unsigned char PID )
{
	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && (*i)->GetPID( ) == PID )
			return *i;
	}

	return NULL;
}

CGamePlayer *CBaseGame :: GetPlayerFromSID( unsigned char SID )
{
	if( SID < m_Slots.size( ) )
		return GetPlayerFromPID( m_Slots[SID].GetPID( ) );

	return NULL;
}

CGamePlayer *CBaseGame :: GetPlayerFromName( string name, bool sensitive )
{
	if( !sensitive )
		transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
		{
			string TestName = (*i)->GetName( );

			if( !sensitive )
				transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

			if( TestName == name )
				return *i;
		}
	}

	return NULL;
}

uint32_t CBaseGame :: GetPlayerFromNamePartial( string name, CGamePlayer **player )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	uint32_t Matches = 0;
	*player = NULL;

	// try to match each player with the passed string (e.g. "Varlock" would be matched with "lock")

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
		{
			string TestName = (*i)->GetName( );
			transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

			if( TestName.find( name ) != string :: npos )
			{
				Matches++;
				*player = *i;
			}
		}
	}

	return Matches;
}

CGamePlayer *CBaseGame :: GetPlayerFromColour( unsigned char colour )
{
	for( unsigned char i = 0; i < m_Slots.size( ); i++ )
	{
		if( m_Slots[i].GetColour( ) == colour )
			return GetPlayerFromSID( i );
	}

	return NULL;
}

unsigned char CBaseGame :: GetNewPID( )
{
	// find an unused PID for a new player to use

	for( unsigned char TestPID = 1; TestPID < 255; TestPID++ )
	{
		if( TestPID == m_VirtualHostPID || TestPID == m_FakePlayerPID )
			continue;

		bool InUse = false;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( !(*i)->GetLeftMessageSent( ) && (*i)->GetPID( ) == TestPID )
			{
				InUse = true;
				break;
			}
		}

		if( !InUse )
			return TestPID;
	}

	// this should never happen

	return 255;
}

unsigned char CBaseGame :: GetNewColour( )
{
	// find an unused colour for a player to use

	for( unsigned char TestColour = 0; TestColour < 12; TestColour++ )
	{
		bool InUse = false;

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetColour( ) == TestColour )
			{
				InUse = true;
				break;
			}
		}

		if( !InUse )
			return TestColour;
	}

	// this should never happen

	return 12;
}

BYTEARRAY CBaseGame :: GetPIDs( )
{
	BYTEARRAY result;

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
			result.push_back( (*i)->GetPID( ) );
	}

	return result;
}

BYTEARRAY CBaseGame :: GetPIDs( unsigned char excludePID )
{
	BYTEARRAY result;

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && (*i)->GetPID( ) != excludePID )
			result.push_back( (*i)->GetPID( ) );
	}

	return result;
}

unsigned char CBaseGame :: GetHostPID( )
{
	// return the player to be considered the host (it can be any player) - mainly used for sending text messages from the bot
	// try to find the virtual host player first

	if( m_VirtualHostPID != 255 )
		return m_VirtualHostPID;

	// try to find the owner player next

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && (*i)->GetName( ) == m_OwnerName )
			return (*i)->GetPID( );
	}

	// okay then, just use the first available player

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
			return (*i)->GetPID( );
	}

	return 255;
}

unsigned char CBaseGame :: GetEmptySlot( bool reserved )
{
	if( m_Slots.size( ) > 255 )
		return 255;

	if( m_SaveGame )
	{
		// unfortunately we don't know which slot each player was assigned in the savegame
		// but we do know which slots were occupied and which weren't so let's at least force players to use previously occupied slots

		vector<CGameSlot> SaveGameSlots = m_SaveGame->GetSlots( );

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && SaveGameSlots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && SaveGameSlots[i].GetComputer( ) == 0 )
				return i;
		}

		// don't bother with reserved slots in savegames
	}
	else
	{
		// look for an empty slot for a new player to occupy
		// if reserved is true then we're willing to use closed or occupied slots as long as it wouldn't displace a player with a reserved slot

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN )
				return i;
		}

		if( reserved )
		{
			// no empty slots, but since player is reserved give them a closed slot

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_CLOSED )
					return i;
			}

			// no closed slots either, give them an occupied slot but not one occupied by another reserved player
			// first look for a player who is downloading the map and has the least amount downloaded so far

			unsigned char LeastDownloaded = 100;
			unsigned char LeastSID = 255;

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				CGamePlayer *Player = GetPlayerFromSID( i );

				if( Player && !Player->GetReserved( ) && m_Slots[i].GetDownloadStatus( ) < LeastDownloaded )
				{
					LeastDownloaded = m_Slots[i].GetDownloadStatus( );
					LeastSID = i;
				}
			}

			if( LeastSID != 255 )
				return LeastSID;

			// nobody who isn't reserved is downloading the map, just choose the first player who isn't reserved

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				CGamePlayer *Player = GetPlayerFromSID( i );

				if( Player && !Player->GetReserved( ) )
					return i;
			}
		}
	}

	return 255;
}

unsigned char CBaseGame :: GetEmptySlot( unsigned char team, unsigned char PID )
{
	if( m_Slots.size( ) > 255 )
		return 255;

	// find an empty slot based on player's current slot

	unsigned char StartSlot = GetSIDFromPID( PID );

	if( StartSlot < m_Slots.size( ) )
	{
		if( m_Slots[StartSlot].GetTeam( ) != team )
		{
			// player is trying to move to another team so start looking from the first slot on that team
			// we actually just start looking from the very first slot since the next few loops will check the team for us

			StartSlot = 0;
		}

		if( m_SaveGame )
		{
			vector<CGameSlot> SaveGameSlots = m_SaveGame->GetSlots( );

			for( unsigned char i = StartSlot; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && m_Slots[i].GetTeam( ) == team && SaveGameSlots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && SaveGameSlots[i].GetComputer( ) == 0 )
					return i;
			}

			for( unsigned char i = 0; i < StartSlot; i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && m_Slots[i].GetTeam( ) == team && SaveGameSlots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && SaveGameSlots[i].GetComputer( ) == 0 )
					return i;
			}
		}
		else
		{
			// find an empty slot on the correct team starting from StartSlot

			for( unsigned char i = StartSlot; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && m_Slots[i].GetTeam( ) == team )
					return i;
			}

			// didn't find an empty slot, but we could have missed one with SID < StartSlot
			// e.g. in the DotA case where I am in slot 4 (yellow), slot 5 (orange) is occupied, and slot 1 (blue) is open and I am trying to move to another slot

			for( unsigned char i = 0; i < StartSlot; i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && m_Slots[i].GetTeam( ) == team )
					return i;
			}
		}
	}

	return 255;
}

void CBaseGame :: SwapSlots( unsigned char SID1, unsigned char SID2 )
{
	if( SID1 < m_Slots.size( ) && SID2 < m_Slots.size( ) && SID1 != SID2 )
	{
		CGameSlot Slot1 = m_Slots[SID1];
		CGameSlot Slot2 = m_Slots[SID2];

		if( m_Map->GetMapGameType( ) != GAMETYPE_CUSTOM )
		{
			// regular game - swap everything

			m_Slots[SID1] = Slot2;
			m_Slots[SID2] = Slot1;
		}
		else
		{
			// custom game - don't swap the team, colour, or race

			m_Slots[SID1] = CGameSlot( Slot2.GetPID( ), Slot2.GetDownloadStatus( ), Slot2.GetSlotStatus( ), Slot2.GetComputer( ), Slot1.GetTeam( ), Slot1.GetColour( ), Slot1.GetRace( ) );
			m_Slots[SID2] = CGameSlot( Slot1.GetPID( ), Slot1.GetDownloadStatus( ), Slot1.GetSlotStatus( ), Slot1.GetComputer( ), Slot2.GetTeam( ), Slot2.GetColour( ), Slot2.GetRace( ) );
		}

		SendAllSlotInfo( );
	}
}

void CBaseGame :: OpenSlot( unsigned char SID, bool kick )
{
	if( kick )
	{
		CGamePlayer *Player = GetPlayerFromSID( SID );

		if( Player )
		{
			Player->SetDeleteMe( true );
			Player->SetLeftReason( "was kicked when opening a slot" );
			Player->SetLeftCode( PLAYERLEAVE_LOBBY );
		}
	}

	if( SID < m_Slots.size( ) )
	{
		CGameSlot Slot = m_Slots[SID];
		m_Slots[SID] = CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, Slot.GetTeam( ), Slot.GetColour( ), Slot.GetRace( ) ); 
		SendAllSlotInfo( );
	}
}

void CBaseGame :: CloseSlot( unsigned char SID, bool kick )
{
	if( kick )
	{
		CGamePlayer *Player = GetPlayerFromSID( SID );

		if( Player )
		{
			Player->SetDeleteMe( true );
			Player->SetLeftReason( "was kicked when closing a slot" );
			Player->SetLeftCode( PLAYERLEAVE_LOBBY );
		}
	}

	if( SID < m_Slots.size( ) )
	{
		CGameSlot Slot = m_Slots[SID];
		m_Slots[SID] = CGameSlot( 0, 255, SLOTSTATUS_CLOSED, 0, Slot.GetTeam( ), Slot.GetColour( ), Slot.GetRace( ) ); 
		SendAllSlotInfo( );
	}
}

void CBaseGame :: ComputerSlot( unsigned char SID, unsigned char skill, bool kick )
{
	if( kick )
	{
		CGamePlayer *Player = GetPlayerFromSID( SID );

		if( Player )
		{
			Player->SetDeleteMe( true );
			Player->SetLeftReason( "was kicked when creating a computer in a slot" );
			Player->SetLeftCode( PLAYERLEAVE_LOBBY );
		}
	}

	if( SID < m_Slots.size( ) && skill < 3 )
	{
		CGameSlot Slot = m_Slots[SID];
		m_Slots[SID] = CGameSlot( 0, 100, SLOTSTATUS_OCCUPIED, 1, Slot.GetTeam( ), Slot.GetColour( ), Slot.GetRace( ), skill );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: ColourSlot( unsigned char SID, unsigned char colour )
{
	if( SID < m_Slots.size( ) && colour < 12 )
	{
		// make sure the requested colour isn't already taken

		bool Taken = false;
		unsigned char TakenSID = 0;

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetColour( ) == colour )
			{
				TakenSID = i;
				Taken = true;
			}
		}

		if( Taken && m_Slots[TakenSID].GetSlotStatus( ) != SLOTSTATUS_OCCUPIED )
		{
			// the requested colour is currently "taken" by an unused (open or closed) slot
			// but we allow the colour to persist within a slot so if we only update the existing player's colour the unused slot will have the same colour
			// this isn't really a problem except that if someone then joins the game they'll receive the unused slot's colour resulting in a duplicate
			// one way to solve this (which we do here) is to swap the player's current colour into the unused slot

			m_Slots[TakenSID].SetColour( m_Slots[SID].GetColour( ) );
			m_Slots[SID].SetColour( colour );
			SendAllSlotInfo( );
		}
		else if( !Taken )
		{
			// the requested colour isn't used by ANY slot

			m_Slots[SID].SetColour( colour );
			SendAllSlotInfo( );
		}
	}
}

void CBaseGame :: OpenAllSlots( )
{
	bool Changed = false;

	for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_CLOSED )
		{
			(*i).SetSlotStatus( SLOTSTATUS_OPEN );
			Changed = true;
		}
	}

	if( Changed )
		SendAllSlotInfo( );
}

void CBaseGame :: CloseAllSlots( )
{
	bool Changed = false;

	for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OPEN )
		{
			(*i).SetSlotStatus( SLOTSTATUS_CLOSED );
			Changed = true;
		}
	}

	if( Changed )
		SendAllSlotInfo( );
}

void CBaseGame :: ShuffleSlots( )
{
	// we only want to shuffle the player slots
	// that means we need to prevent this function from shuffling the open/closed/computer slots too
	// so we start by copying the player slots to a temporary vector

	vector<CGameSlot> PlayerSlots;

	for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 )
			PlayerSlots.push_back( *i );
	}

	// now we shuffle PlayerSlots

	if( m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM )
	{
		// custom game
		// rather than rolling our own probably broken shuffle algorithm we use random_shuffle because it's guaranteed to do it properly
		// so in order to let random_shuffle do all the work we need a vector to operate on
		// unfortunately we can't just use PlayerSlots because the team/colour/race shouldn't be modified
		// so make a vector we can use

		vector<unsigned char> SIDs;

		for( unsigned char i = 0; i < PlayerSlots.size( ); i++ )
			SIDs.push_back( i );

		random_shuffle( SIDs.begin( ), SIDs.end( ) );

		// now put the PlayerSlots vector in the same order as the SIDs vector

		vector<CGameSlot> Slots;

		// as usual don't modify the team/colour/race

		for( unsigned char i = 0; i < SIDs.size( ); i++ )
			Slots.push_back( CGameSlot( PlayerSlots[SIDs[i]].GetPID( ), PlayerSlots[SIDs[i]].GetDownloadStatus( ), PlayerSlots[SIDs[i]].GetSlotStatus( ), PlayerSlots[SIDs[i]].GetComputer( ), PlayerSlots[i].GetTeam( ), PlayerSlots[i].GetColour( ), PlayerSlots[i].GetRace( ) ) );

		PlayerSlots = Slots;
	}
	else
	{
		// regular game
		// it's easy when we're allowed to swap the team/colour/race!

		random_shuffle( PlayerSlots.begin( ), PlayerSlots.end( ) );
	}

	// now we put m_Slots back together again

	vector<CGameSlot> :: iterator CurrentPlayer = PlayerSlots.begin( );
	vector<CGameSlot> Slots;

	for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 )
		{
			Slots.push_back( *CurrentPlayer );
			CurrentPlayer++;
		}
		else
			Slots.push_back( *i );
	}

	m_Slots = Slots;

	// and finally tell everyone about the new slot configuration

	SendAllSlotInfo( );
}

void CBaseGame :: BalanceSlots( )
{
	// todotodo: this isn't a very good balancing algorithm :)

	ShuffleSlots( );
}

void CBaseGame :: AddToSpoofed( string server, string name, bool sendMessage )
{
	CGamePlayer *Player = GetPlayerFromName( name, true );

	if( Player )
	{
		Player->SetSpoofedRealm( server );
		Player->SetSpoofed( true );

		if( sendMessage )
			SendAllChat( m_GHost->m_Language->SpoofCheckAcceptedFor( server, name ) );
	}
}

void CBaseGame :: AddToReserved( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	// check that the user is not already reserved

	for( vector<string> :: iterator i = m_Reserved.begin( ); i != m_Reserved.end( ); i++ )
	{
		if( *i == name )
			return;
	}

	m_Reserved.push_back( name );

	// upgrade the user if they're already in the game

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		string NameLower = (*i)->GetName( );
		transform( NameLower.begin( ), NameLower.end( ), NameLower.begin( ), (int(*)(int))tolower );

		if( NameLower == name )
			(*i)->SetReserved( true );
	}
}

bool CBaseGame :: IsOwner( string name )
{
	string OwnerLower = m_OwnerName;
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	transform( OwnerLower.begin( ), OwnerLower.end( ), OwnerLower.begin( ), (int(*)(int))tolower );
	return name == OwnerLower;
}

bool CBaseGame :: IsReserved( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	for( vector<string> :: iterator i = m_Reserved.begin( ); i != m_Reserved.end( ); i++ )
	{
		if( *i == name )
			return true;
	}

	return false;
}

bool CBaseGame :: IsDownloading( )
{
	// returns true if at least one player is downloading the map

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetDownloadStarted( ) && !(*i)->GetDownloadFinished( ) )
			return true;
	}

	return false;
}

bool CBaseGame :: IsGameDataSaved( )
{
	return true;
}

void CBaseGame :: SaveGameData( )
{

}

void CBaseGame :: StartCountDown( bool force )
{
	if( !m_CountDownStarted )
	{
		if( force )
		{
			m_CountDownStarted = true;
			m_CountDownCounter = 5;
		}
		else
		{
			// check if the HCL command string is short enough

			if( m_HCLCommandString.size( ) > GetSlotsOccupied( ) )
			{
				SendAllChat( m_GHost->m_Language->TheHCLIsTooLongUseForceToStart( ) );
				return;
			}

			// check if everyone has the map

			string StillDownloading;

			for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
			{
				if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 && (*i).GetDownloadStatus( ) != 100 )
				{
					CGamePlayer *Player = GetPlayerFromPID( (*i).GetPID( ) );

					if( Player )
					{
						if( StillDownloading.empty( ) )
							StillDownloading = Player->GetName( );
						else
							StillDownloading += ", " + Player->GetName( );
					}
				}
			}

			if( !StillDownloading.empty( ) )
				SendAllChat( m_GHost->m_Language->PlayersStillDownloading( StillDownloading ) );

			// check if everyone is spoof checked

			string NotSpoofChecked;

			if( m_GHost->m_SpoofChecks )
			{
				for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
				{
					if( !(*i)->GetSpoofed( ) )
					{
						if( NotSpoofChecked.empty( ) )
							NotSpoofChecked = (*i)->GetName( );
						else
							NotSpoofChecked += ", " + (*i)->GetName( );
					}
				}

				if( !NotSpoofChecked.empty( ) )
				{
					SendAllChat( m_GHost->m_Language->PlayersNotYetSpoofChecked( NotSpoofChecked ) );

					if( m_GHost->m_BNETs.size( ) == 1 )
					{
						BYTEARRAY UniqueName = m_GHost->m_BNETs[0]->GetUniqueName( );

						if( m_GameState == GAME_PUBLIC )
							SendAllChat( m_GHost->m_Language->ManuallySpoofCheckByWhispering( string( UniqueName.begin( ), UniqueName.end( ) ) ) );
						else if( m_GameState == GAME_PRIVATE )
							SendAllChat( m_GHost->m_Language->SpoofCheckByWhispering( string( UniqueName.begin( ), UniqueName.end( ) ) ) );
					}

					// todotodo: figure something out with multiple realms here
				}
			}

			// check if everyone has been pinged enough (3 times) that the autokicker would have kicked them by now
			// see function EventPlayerPongToHost for the autokicker code

			string NotPinged;

			for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			{
				if( (*i)->GetNumPings( ) < 3 )
				{
					if( NotPinged.empty( ) )
						NotPinged = (*i)->GetName( );
					else
						NotPinged += ", " + (*i)->GetName( );
				}
			}

			if( !NotPinged.empty( ) )
				SendAllChat( m_GHost->m_Language->PlayersNotYetPinged( NotPinged ) );

			// if no problems found start the game

			if( StillDownloading.empty( ) && NotSpoofChecked.empty( ) && NotPinged.empty( ) )
			{
				m_CountDownStarted = true;
				m_CountDownCounter = 5;
			}
		}
	}
}

void CBaseGame :: StartCountDownAuto( bool requireSpoofChecks )
{
	if( !m_CountDownStarted )
	{
		// check if enough players are present

		if( GetNumPlayers( ) < m_AutoStartPlayers )
		{
			SendAllChat( m_GHost->m_Language->WaitingForPlayersBeforeAutoStart( UTIL_ToString( m_AutoStartPlayers ), UTIL_ToString( m_AutoStartPlayers - GetNumPlayers( ) ) ) );
			return;
		}

		// check if everyone has the map

		string StillDownloading;

		for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
		{
			if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 && (*i).GetDownloadStatus( ) != 100 )
			{
				CGamePlayer *Player = GetPlayerFromPID( (*i).GetPID( ) );

				if( Player )
				{
					if( StillDownloading.empty( ) )
						StillDownloading = Player->GetName( );
					else
						StillDownloading += ", " + Player->GetName( );
				}
			}
		}

		if( !StillDownloading.empty( ) )
		{
			SendAllChat( m_GHost->m_Language->PlayersStillDownloading( StillDownloading ) );
			return;
		}

		string NotSpoofChecked;

		if( requireSpoofChecks )
		{
			// check if everyone is spoof checked

			if( m_GHost->m_SpoofChecks )
			{
				for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
				{
					if( !(*i)->GetSpoofed( ) )
					{
						if( NotSpoofChecked.empty( ) )
							NotSpoofChecked = (*i)->GetName( );
						else
							NotSpoofChecked += ", " + (*i)->GetName( );
					}
				}

				if( !NotSpoofChecked.empty( ) )
				{
					SendAllChat( m_GHost->m_Language->PlayersNotYetSpoofChecked( NotSpoofChecked ) );

					/*

					if( m_GHost->m_BNETs.size( ) == 1 )
					{
						BYTEARRAY UniqueName = m_GHost->m_BNETs[0]->GetUniqueName( );

						if( m_GameState == GAME_PUBLIC )
							SendAllChat( m_GHost->m_Language->ManuallySpoofCheckByWhispering( string( UniqueName.begin( ), UniqueName.end( ) ) ) );
						else if( m_GameState == GAME_PRIVATE )
							SendAllChat( m_GHost->m_Language->SpoofCheckByWhispering( string( UniqueName.begin( ), UniqueName.end( ) ) ) );
					}

					*/

					// todotodo: figure something out with multiple realms here
					// idea: we can send a different host counter to each realm and use that to determine the likely realm the user came from!
				}
			}
		}

		// check if everyone has been pinged enough (3 times) that the autokicker would have kicked them by now
		// see function EventPlayerPongToHost for the autokicker code

		string NotPinged;

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetNumPings( ) < 3 )
			{
				if( NotPinged.empty( ) )
					NotPinged = (*i)->GetName( );
				else
					NotPinged += ", " + (*i)->GetName( );
			}
		}

		if( !NotPinged.empty( ) )
		{
			SendAllChat( m_GHost->m_Language->PlayersNotYetPingedAutoStart( NotPinged ) );
			return;
		}

		// if no problems found start the game

		if( StillDownloading.empty( ) && NotSpoofChecked.empty( ) && NotPinged.empty( ) )
		{
			m_CountDownStarted = true;
			m_CountDownCounter = 10;
		}
	}
}

void CBaseGame :: StopPlayers( string reason )
{
	// disconnect every player and set their left reason to the passed string
	// we use this function when we want the code in the Update function to run before the destructor (e.g. saving players to the database)
	// therefore calling this function when m_GameLoading || m_GameLoaded is roughly equivalent to setting m_Exiting = true
	// the only difference is whether the code in the Update function is executed or not

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		(*i)->SetDeleteMe( true );
		(*i)->SetLeftReason( reason );
		(*i)->SetLeftCode( PLAYERLEAVE_LOST );
	}
}

void CBaseGame :: StopLaggers( string reason )
{
	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetLagging( ) )
		{
			(*i)->SetDeleteMe( true );
			(*i)->SetLeftReason( reason );
			(*i)->SetLeftCode( PLAYERLEAVE_DISCONNECT );
		}
	}
}

void CBaseGame :: CreateVirtualHost( )
{
	if( m_VirtualHostPID != 255 )
		return;

	m_VirtualHostPID = GetNewPID( );
	BYTEARRAY IP;
	IP.push_back( 0 );
	IP.push_back( 0 );
	IP.push_back( 0 );
	IP.push_back( 0 );
	SendAll( m_Protocol->SEND_W3GS_PLAYERINFO( m_VirtualHostPID, m_VirtualHostName, IP, IP ) );
}

void CBaseGame :: DeleteVirtualHost( )
{
	if( m_VirtualHostPID == 255 )
		return;

	SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( m_VirtualHostPID, PLAYERLEAVE_LOBBY ) );
	m_VirtualHostPID = 255;
}

void CBaseGame :: CreateFakePlayer( )
{
	if( m_FakePlayerPID != 255 )
		return;

	unsigned char SID = GetEmptySlot( false );

	if( SID < m_Slots.size( ) )
	{
		m_FakePlayerPID = GetNewPID( );
		BYTEARRAY IP;
		IP.push_back( 0 );
		IP.push_back( 0 );
		IP.push_back( 0 );
		IP.push_back( 0 );
		SendAll( m_Protocol->SEND_W3GS_PLAYERINFO( m_FakePlayerPID, "FakePlayer", IP, IP ) );
		m_Slots[SID] = CGameSlot( m_FakePlayerPID, 100, SLOTSTATUS_OCCUPIED, 0, m_Slots[SID].GetTeam( ), m_Slots[SID].GetColour( ), m_Slots[SID].GetRace( ) );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: DeleteFakePlayer( )
{
	if( m_FakePlayerPID == 255 )
		return;

	for( unsigned char i = 0; i < m_Slots.size( ); i++ )
	{
		if( m_Slots[i].GetPID( ) == m_FakePlayerPID )
			m_Slots[i] = CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, m_Slots[i].GetTeam( ), m_Slots[i].GetColour( ), m_Slots[i].GetRace( ) ); 
	}

	SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( m_FakePlayerPID, PLAYERLEAVE_LOBBY ) );
	SendAllSlotInfo( );
	m_FakePlayerPID = 255;
}

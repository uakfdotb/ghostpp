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
#include "game.h"
#include "stats.h"
#include "statsdota.h"

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
	m_HostPort = nHostPort;
	m_GameState = nGameState;
	m_VirtualHostPID = 255;
	m_GameName = nGameName;
	m_VirtualHostName = m_GHost->m_VirtualHostName;
	m_OwnerName = nOwnerName;
	m_CreatorName = nCreatorName;
	m_CreatorServer = nCreatorServer;
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
	m_StartedLoadingTime = 0;
	m_StartPlayers = 0;
	m_LastActionSentTicks = 0;
	m_StartedLaggingTime = 0;
	m_LastLagScreenTime = 0;
	m_LastReservedSeen = GetTime( );
	m_Locked = false;
	m_RefreshMessages = m_GHost->m_RefreshMessages;
	m_MuteAll = false;
	m_MuteLobby = false;
	m_CountDownStarted = false;
	m_GameLoading = false;
	m_GameLoaded = false;
	m_Desynced = false;
	m_Lagging = false;
	m_AutoSave = m_GHost->m_AutoSave;

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

	while( !m_Actions.empty( ) )
	{
		delete m_Actions.front( );
		m_Actions.pop( );
	}
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

unsigned int CBaseGame :: SetFD( void *fd, int *nfds )
{
	unsigned int NumFDs = 0;

	if( m_Socket )
	{
		m_Socket->SetFD( (fd_set *)fd, nfds );
		NumFDs++;
	}

	for( vector<CPotentialPlayer *> :: iterator i = m_Potentials.begin( ); i != m_Potentials.end( ); i++ )
	{
		if( (*i)->GetSocket( ) )
		{
			(*i)->GetSocket( )->SetFD( (fd_set *)fd, nfds );
			NumFDs++;
		}
	}

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetSocket( ) )
		{
			(*i)->GetSocket( )->SetFD( (fd_set *)fd, nfds );
			NumFDs++;
		}
	}

	return NumFDs;
}

bool CBaseGame :: Update( void *fd )
{
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
		// todotodo: ignore pings received from players who have recently finished download the map

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

	// refresh every 10 seconds

	if( !m_CountDownStarted && m_GameState == GAME_PUBLIC && GetSlotsOpen( ) > 0 && GetTime( ) >= m_LastRefreshTime + 10 )
	{
		// send a game refresh packet to each battle.net connection

		for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			// we send two game refreshes, the first one to indicate the game is full and the second one to indicate the game is open
			// this is why "refreshing the slots" in Warcraft III works because battle.net seems to advertise games that change states more than those that don't
			// therefore by doing this we're following the same procedure Warcraft III does except that we don't actually have to close and open any slots

			(*i)->SendGameRefresh( m_GameState | GAME_FULL, m_GameName, string( ), m_Map, m_SaveGame, GetTime( ) - m_CreationTime );
			(*i)->SendGameRefresh( m_GameState, m_GameName, string( ), m_Map, m_SaveGame, GetTime( ) - m_CreationTime );
		}

		if( m_RefreshMessages )
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

	// try to auto start every 10 seconds

	if( m_AutoStartPlayers != 0 && GetTime( ) >= m_LastAutoStartTime + 10 )
	{
		StartCountDownAuto( );
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
		{
			m_StartedLoadingTicks = GetTicks( );
			m_StartedLoadingTime = GetTime( );
			m_GameLoading = true;
			EventGameStarted( );
		}

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

	// if there aren't any players left and the game is loading/loaded then it's over and we can mark ourselves for deletion

	if( m_Players.empty( ) && ( m_GameLoading || m_GameLoaded ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] is over (no players left)" );
		return true;
	}

	// accept new connections

	if( m_Socket )
	{
		CTCPSocket *NewSocket = m_Socket->Accept( (fd_set *)fd );

		if( NewSocket )
			m_Potentials.push_back( new CPotentialPlayer( m_Protocol, this, NewSocket ) );

		if( m_Socket->HasError( ) )
			return true;
	}

	return m_Exiting;
}

void CBaseGame :: UpdatePost( )
{
	// we need to manually call DoSend on each player now because CGamePlayer :: Update doesn't do it
	// this is in case player 2 generates a packet for player 1 during the update but it doesn't get sent because player 1 already finished updating
	// in reality since we're queueing actions it might not make a big difference but oh well

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetSocket( ) )
			(*i)->GetSocket( )->DoSend( );
	}

	for( vector<CPotentialPlayer *> :: iterator i = m_Potentials.begin( ); i != m_Potentials.end( ); i++ )
	{
		if( (*i)->GetSocket( ) )
			(*i)->GetSocket( )->DoSend( );
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
		if( !m_GameLoaded )
		{
			if( message.size( ) > 220 )
				message = message.substr( 0, 220 );

			Send( player, m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, UTIL_CreateByteArray( player->GetPID( ) ), 16, BYTEARRAY( ), message ) );
		}
		else
		{
			unsigned char ExtraFlags[] = { 3, 0, 0, 0 };

			// based on my limited testing it seems that the extra flags' first byte contains 3 plus the recipient's colour to denote a private message

			unsigned char SID = GetSIDFromPID( player->GetPID( ) );

			if( SID < m_Slots.size( ) )
				ExtraFlags[0] = 3 + m_Slots[SID].GetColour( );

			if( message.size( ) > 120 )
				message = message.substr( 0, 120 );

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
		if( !m_GameLoaded )
		{
			if( message.size( ) > 220 )
				message = message.substr( 0, 220 );

			// this is a lobby ghost chat message

			SendAll( m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, GetPIDs( ), 16, BYTEARRAY( ), message ) );
		}
		else
		{
			if( message.size( ) > 120 )
				message = message.substr( 0, 120 );

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
	in.open( "motd.txt" );

	if( in.fail( ) )
	{
		// default welcome message

		SendChat( player, " " );
		SendChat( player, " " );
		SendChat( player, " " );
		SendChat( player, " " );
		SendChat( player, "GHost++                                        http://forum.codelain.com/" );
		SendChat( player, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" );
		SendChat( player, "          Game Name:     " + m_GameName );
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
				SendChat( player, " " );
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
	in.open( "gameover.txt" );

	if( !in.fail( ) )
	{
		// don't print more than 8 lines

		uint32_t Count = 0;
		string Line;

		while( !in.eof( ) && Count < 8 )
		{
			getline( in, Line );

			if( Line.empty( ) )
				SendAllChat( " " );
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

	if( m_AutoSave && m_GameLoaded && player->GetLeftCode( ) == PLAYERLEAVE_DISCONNECT )
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

	// tell everyone about the player leaving

	SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( player->GetPID( ), player->GetLeftCode( ) ) );

	// set the replay's host PID and name to the last player to leave the game
	// this will get overwritten as each player leaves the game so it will eventually be set to the last player

	if( m_Replay && ( m_GameLoading || m_GameLoaded ) )
	{
		m_Replay->SetHostPID( player->GetPID( ) );
		m_Replay->SetHostName( player->GetName( ) );

		// add leave message to replay

		m_Replay->AddLeaveGame( 1, player->GetPID( ), player->GetLeftCode( ) );
	}

	// abort the countdown if there was one in progress

	if( m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
	{
		SendAllChat( m_GHost->m_Language->CountDownAborted( ) );
		m_CountDownStarted = false;
	}
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
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] is trying to join the game with an invalid name of length " + UTIL_ToString( joinPlayer->GetName( ).size( ) ) );
		potential->SetDeleteMe( true );
		return;
	}

	// check if the new player's name is the same as the virtual host name

	if( joinPlayer->GetName( ) == m_VirtualHostName )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] is trying to join the game with the virtual host name" );
		potential->SetDeleteMe( true );
		return;
	}

	// check if the new player's name is already taken

	if( GetPlayerFromName( joinPlayer->GetName( ), false ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] is trying to join the game but that name is already taken" );
		// SendAllChat( m_GHost->m_Language->TryingToJoinTheGameButTaken( joinPlayer->GetName( ) ) );
		potential->SetDeleteMe( true );
		return;
	}

	// check if the new player's name is banned

	for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		CDBBan *Ban = m_GHost->m_DB->BanCheck( (*i)->GetServer( ), joinPlayer->GetName( ) );

		if( Ban )
		{
			delete Ban;
			CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] is trying to join the game but is banned" );
			SendAllChat( m_GHost->m_Language->TryingToJoinTheGameButBanned( joinPlayer->GetName( ) ) );
			potential->SetDeleteMe( true );
			return;
		}
	}

	// try to find an empty slot

	unsigned char SID = GetEmptySlot( false );

	// check if the player is an admin or root admin on any connected realm for determining reserved status
	// we can't just use the spoof checked realm like in EventPlayerBotCommand because the player hasn't spoof checked yet

	bool AdminCheck = false;

	for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		if( m_GHost->m_DB->AdminCheck( (*i)->GetServer( ), joinPlayer->GetName( ) ) || (*i)->IsRootAdmin( joinPlayer->GetName( ) ) )
		{
			AdminCheck = true;
			break;
		}
	}

	bool Reserved = IsReserved( joinPlayer->GetName( ) ) || AdminCheck || IsOwner( joinPlayer->GetName( ) );

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

	CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] joined the game" );
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
			m_Slots[SID].SetTeam( 0 );
			m_Slots[SID].SetColour( GetNewColour( ) );
		}
	}

	// send slot info to the new player
	// the SLOTINFOJOIN packet also tells the client their assigned PID and that the join was successful

	Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_SLOTINFOJOIN( Player->GetPID( ), Player->GetSocket( )->GetPort( ), Player->GetExternalIP( ), m_Slots, m_RandomSeed, m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM ? 3 : 0, m_Map->GetMapNumPlayers( ) ) );

	// send virtual host info to the new player

	SendVirtualHostPlayerInfo( Player );

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

	Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_MAPCHECK( m_Map->GetMapPath( ), m_Map->GetMapSize( ), m_Map->GetMapInfo( ), m_Map->GetMapCRC( ) ) );

	// send slot info to everyone, so the new player gets this info twice but everyone else still needs to know the new slot layout

	SendAllSlotInfo( );

	// send a welcome message

	SendWelcomeMessage( Player );

	// check for multiple IP usage

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

			if( !m_GameLoading )
			{
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
			}

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
			if( team >= m_Map->GetMapNumTeams( ) )
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

			m_GHost->m_DB->DownloadAdd( m_Map->GetMapPath( ), MapSize, player->GetName( ), player->GetExternalIPString( ), player->GetSpoofed( ) ? 1 : 0, player->GetSpoofedRealm( ), GetTicks( ) - player->GetStartedDownloadingTicks( ) );
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

void CBaseGame :: EventGameStarted( )
{
	CONSOLE_Print( "[GAME: " + m_GameName + "] started loading with " + UTIL_ToString( GetNumPlayers( ) ) + " players" );

	// since we use a fake countdown to deal with leavers during countdown the COUNTDOWN_START and COUNTDOWN_END packets are sent in quick succession
	// send a start countdown packet

	SendAll( m_Protocol->SEND_W3GS_COUNTDOWN_START( ) );

	// remove the virtual host player

	DeleteVirtualHost( );

	// send an end countdown packet

	SendAll( m_Protocol->SEND_W3GS_COUNTDOWN_END( ) );

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
	StatString = UTIL_EncodeStatString( StatString );
	m_StatString = string( StatString.begin( ), StatString.end( ) );

	// move the game to the games in progress vector

	m_GHost->m_CurrentGame = NULL;
	m_GHost->m_Games.push_back( this );

	// and finally reenter battle.net chat

	for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		(*i)->SendGameUncreate( );
		(*i)->SendEnterChat( );
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
	in.open( "gameloaded.txt" );

	if( !in.fail( ) )
	{
		// don't print more than 8 lines

		uint32_t Count = 0;
		string Line;

		while( !in.eof( ) && Count < 8 )
		{
			getline( in, Line );

			if( Line.empty( ) )
				SendAllChat( " " );
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
		if( TestPID == m_VirtualHostPID )
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

void CBaseGame :: StartCountDownAuto( )
{
	if( !m_CountDownStarted )
	{
		// check if enough players are present

		if( GetNumPlayers( ) < m_AutoStartPlayers )
		{
			SendAllChat( m_GHost->m_Language->WaitingForPlayersBeforeAutoStart( UTIL_ToString( m_AutoStartPlayers ) ) );
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
			SendAllChat( m_GHost->m_Language->PlayersNotYetPinged( NotPinged ) );
			return;
		}

		// if no problems found start the game

		if( StillDownloading.empty( ) && NotPinged.empty( ) )
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

//
// sorting classes
//

class CGamePlayerSortAscByPing
{
public:
	bool operator( ) ( CGamePlayer *Player1, CGamePlayer *Player2 ) const
	{
		return Player1->GetPing( false ) < Player2->GetPing( false );
	}
};

class CGamePlayerSortDescByPing
{
public:
	bool operator( ) ( CGamePlayer *Player1, CGamePlayer *Player2 ) const
	{
		return Player1->GetPing( false ) > Player2->GetPing( false );
	}
};

//
// CGame
//

CGame :: CGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nOwnerName, string nCreatorName, string nCreatorServer ) : CBaseGame( nGHost, nMap, nSaveGame, nHostPort, nGameState, nGameName, nOwnerName, nCreatorName, nCreatorServer )
{
	m_DBBanLast = NULL;
	m_DBGame = new CDBGame( 0, string( ), m_Map->GetMapPath( ), string( ), string( ), string( ), 0 );

	if( m_Map->GetMapType( ) == "dota" )
		m_Stats = new CStatsDOTA( this );
	else
		m_Stats = NULL;

	m_GameOverTime = 0;
}

CGame :: ~CGame( )
{
	if( m_GameLoading || m_GameLoaded )
	{
		if( !m_GHost->m_DB->Begin( ) )
			CONSOLE_Print( "[GAME: " + m_GameName + "] warning - failed to begin database transaction, game data not saved" );
		else
		{
			// store the CDBGame in the database

			uint32_t GameID = m_GHost->m_DB->GameAdd( m_GHost->m_BNETs.size( ) == 1 ? m_GHost->m_BNETs[0]->GetServer( ) : string( ), m_DBGame->GetMap( ), m_GameName, m_OwnerName, GetTime( ) - m_StartedLoadingTime, m_GameState, m_CreatorName, m_CreatorServer );

			if( GameID > 0 )
			{
				// store the CDBGamePlayers in the database

				for( vector<CDBGamePlayer *> :: iterator i = m_DBGamePlayers.begin( ); i != m_DBGamePlayers.end( ); i++ )
					m_GHost->m_DB->GamePlayerAdd( GameID, (*i)->GetName( ), (*i)->GetIP( ), (*i)->GetSpoofed( ), (*i)->GetSpoofedRealm( ), (*i)->GetReserved( ), (*i)->GetLoadingTime( ), (*i)->GetLeft( ), (*i)->GetLeftReason( ), (*i)->GetTeam( ), (*i)->GetColour( ) );

				// store the stats in the database

				if( m_Stats )
					m_Stats->Save( m_GHost->m_DB, GameID );
			}

			if( !m_GHost->m_DB->Commit( ) )
				CONSOLE_Print( "[GAME: " + m_GameName + "] warning - failed to commit database transaction, game data not saved" );
		}
	}

	for( vector<CDBBan *> :: iterator i = m_DBBans.begin( ); i != m_DBBans.end( ); i++ )
		delete *i;

	delete m_DBGame;

	for( vector<CDBGamePlayer *> :: iterator i = m_DBGamePlayers.begin( ); i != m_DBGamePlayers.end( ); i++ )
		delete *i;

	delete m_Stats;
}

void CGame :: EventPlayerDeleted( CGamePlayer *player )
{
	CBaseGame :: EventPlayerDeleted( player );

	// record everything we need to know about the player for storing in the database later
	// since we haven't stored the game yet (it's not over yet!) we can't link the gameplayer to the game
	// see the destructor for where these CDBGamePlayers are stored in the database
	// we could have inserted an incomplete record on creation and updated it later but this makes for a cleaner interface

	if( m_GameLoading || m_GameLoaded )
	{
		// todotodo: since we store players that crash during loading it's possible that the stats classes could have no information on them
		// that could result in a DBGamePlayer without a corresponding DBDotAPlayer - just be aware of the possibility

		unsigned char SID = GetSIDFromPID( player->GetPID( ) );
		unsigned char Team = 255;
		unsigned char Colour = 255;

		if( SID < m_Slots.size( ) )
		{
			Team = m_Slots[SID].GetTeam( );
			Colour = m_Slots[SID].GetColour( );
		}

		m_DBGamePlayers.push_back( new CDBGamePlayer( 0, 0, player->GetName( ), player->GetExternalIPString( ), player->GetSpoofed( ) ? 1 : 0, player->GetSpoofedRealm( ), player->GetReserved( ) ? 1 : 0, player->GetFinishedLoading( ) ? player->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks : 0, GetTime( ) - m_StartedLoadingTime, player->GetLeftReason( ), Team, Colour ) );

		// also keep track of the last player to leave for the !banlast command

		for( vector<CDBBan *> :: iterator i = m_DBBans.begin( ); i != m_DBBans.end( ); i++ )
		{
			if( (*i)->GetName( ) == player->GetName( ) )
				m_DBBanLast = *i;
		}
	}
}

void CGame :: EventPlayerAction( CGamePlayer *player, CIncomingAction *action )
{
	CBaseGame :: EventPlayerAction( player, action );

	// give the stats class a chance to process the action

	if( m_Stats && m_Stats->ProcessAction( action ) && m_GameOverTime == 0 )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] stats class reported game over" );
		SendEndMessage( );
		m_GameOverTime = GetTime( );
	}
}

void CGame :: EventPlayerBotCommand( CGamePlayer *player, string command, string payload )
{
	CBaseGame :: EventPlayerBotCommand( player, command, payload );

	// todotodo: don't be lazy

	string User = player->GetName( );
	string Command = command;
	string Payload = payload;

	bool RootAdminCheck = false;

	for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		if( (*i)->GetServer( ) == player->GetSpoofedRealm( ) && (*i)->IsRootAdmin( User ) )
		{
			RootAdminCheck = true;
			break;
		}
	}

	if( player->GetSpoofed( ) && ( m_GHost->m_DB->AdminCheck( player->GetSpoofedRealm( ), User ) || RootAdminCheck || IsOwner( User ) ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] admin [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );

		if( !m_Locked || IsOwner( User ) )
		{
			/*****************
			* ADMIN COMMANDS *
			******************/

			//
			// !ABORT (abort countdown)
			// !A
			//

			// we use "!a" as an alias for abort because you don't have much time to abort the countdown so it's useful for the abort command to be easy to type

			if( ( Command == "abort" || Command == "a" ) && m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
			{
				SendAllChat( m_GHost->m_Language->CountDownAborted( ) );
				m_CountDownStarted = false;
			}

			//
			// !ADDBAN
			// !BAN
			//

			if( ( Command == "addban" || Command == "ban" ) && !Payload.empty( ) && !m_GHost->m_BNETs.empty( ) )
			{
				// extract the victim and the reason
				// e.g. "Varlock leaver after dying" -> victim: "Varlock", reason: "leaver after dying"

				string Victim;
				string Reason;
				stringstream SS;
				SS << Payload;
				SS >> Victim;

				if( !SS.eof( ) )
				{
					getline( SS, Reason );
					string :: size_type Start = Reason.find_first_not_of( " " );

					if( Start != string :: npos )
						Reason = Reason.substr( Start );
				}

				if( m_GameLoaded )
				{
					string VictimLower = Victim;
					transform( VictimLower.begin( ), VictimLower.end( ), VictimLower.begin( ), (int(*)(int))tolower );
					uint32_t Matches = 0;
					CDBBan *LastMatch = NULL;

					// try to match each player with the passed string (e.g. "Varlock" would be matched with "lock")
					// we use the m_DBBans vector for this in case the player already left and thus isn't in the m_Players vector anymore

					for( vector<CDBBan *> :: iterator i = m_DBBans.begin( ); i != m_DBBans.end( ); i++ )
					{
						string TestName = (*i)->GetName( );
						transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

						if( TestName.find( VictimLower ) != string :: npos )
						{
							Matches++;
							LastMatch = *i;
						}
					}

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToBanNoMatchesFound( Victim ) );
					else if( Matches == 1 )
					{
						if( !LastMatch->GetServer( ).empty( ) )
						{
							// the user was spoof checked, ban only on the spoofed realm

							m_GHost->m_DB->BanAdd( LastMatch->GetServer( ), LastMatch->GetName( ), LastMatch->GetIP( ), m_GameName, User, Reason );
						}
						else
						{
							// the user wasn't spoof checked, ban on every realm

							for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
								m_GHost->m_DB->BanAdd( (*i)->GetServer( ), LastMatch->GetName( ), LastMatch->GetIP( ), m_GameName, User, Reason );
						}

						CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + LastMatch->GetName( ) + "] was banned by player [" + User + "]" );
						SendAllChat( m_GHost->m_Language->PlayerWasBannedByPlayer( LastMatch->GetName( ), User ) );
					}
					else
						SendAllChat( m_GHost->m_Language->UnableToBanFoundMoreThanOneMatch( Victim ) );
				}
				else
				{
					CGamePlayer *LastMatch = NULL;
					uint32_t Matches = GetPlayerFromNamePartial( Victim, &LastMatch );

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToBanNoMatchesFound( Victim ) );
					else if( Matches == 1 )
					{
						if( !LastMatch->GetSpoofedRealm( ).empty( ) )
						{
							// the user was spoof checked, ban only on the spoofed realm

							m_GHost->m_DB->BanAdd( LastMatch->GetSpoofedRealm( ), LastMatch->GetName( ), LastMatch->GetExternalIPString( ), m_GameName, User, Reason );
						}
						else
						{
							// the user wasn't spoof checked, ban on every realm

							for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
								m_GHost->m_DB->BanAdd( (*i)->GetServer( ), LastMatch->GetName( ), LastMatch->GetExternalIPString( ), m_GameName, User, Reason );
						}

						CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + LastMatch->GetName( ) + "] was banned by player [" + User + "]" );
						SendAllChat( m_GHost->m_Language->PlayerWasBannedByPlayer( LastMatch->GetName( ), User ) );
					}
					else
						SendAllChat( m_GHost->m_Language->UnableToBanFoundMoreThanOneMatch( Victim ) );
				}
			}

			//
			// !ANNOUNCE
			//

			if( Command == "announce" && !m_CountDownStarted )
			{
				if( Payload.empty( ) || Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->AnnounceMessageDisabled( ) );
					SetAnnounce( 0, string( ) );
				}
				else
				{
					// extract the interval and the message
					// e.g. "30 hello everyone" -> interval: "30", message: "hello everyone"

					uint32_t Interval;
					string Message;
					stringstream SS;
					SS << Payload;
					SS >> Interval;

					if( SS.fail( ) || Interval == 0 )
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to announce command" );
					else
					{
						if( SS.eof( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to announce command" );
						else
						{
							getline( SS, Message );
							string :: size_type Start = Message.find_first_not_of( " " );

							if( Start != string :: npos )
								Message = Message.substr( Start );

							SendAllChat( m_GHost->m_Language->AnnounceMessageEnabled( ) );
							SetAnnounce( Interval, Message );
						}
					}
				}
			}

			//
			// !AUTOSAVE
			//

			if( Command == "autosave" )
			{
				if( Payload == "on" )
				{
					SendAllChat( m_GHost->m_Language->AutoSaveEnabled( ) );
					m_AutoSave = true;
				}
				else if( Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->AutoSaveDisabled( ) );
					m_AutoSave = false;
				}
			}

			//
			// !AUTOSTART
			//

			if( Command == "autostart" && !m_CountDownStarted )
			{
				if( Payload.empty( ) || Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->AutoStartDisabled( ) );
					m_AutoStartPlayers = 0;
				}
				else
				{
					uint32_t AutoStartPlayers = UTIL_ToUInt32( Payload );

					if( AutoStartPlayers != 0 )
					{
						SendAllChat( m_GHost->m_Language->AutoStartEnabled( UTIL_ToString( AutoStartPlayers ) ) );
						m_AutoStartPlayers = AutoStartPlayers;
					}
				}
			}

			//
			// !BANLAST
			//

			if( Command == "banlast" && m_GameLoaded && !m_GHost->m_BNETs.empty( ) && m_DBBanLast )
			{
				if( !m_DBBanLast->GetServer( ).empty( ) )
				{
					// the user was spoof checked, ban only on the spoofed realm

					m_GHost->m_DB->BanAdd( m_DBBanLast->GetServer( ), m_DBBanLast->GetName( ), m_DBBanLast->GetIP( ), m_GameName, User, Payload );
				}
				else
				{
					// the user wasn't spoof checked, ban on every realm

					for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
						m_GHost->m_DB->BanAdd( (*i)->GetServer( ), m_DBBanLast->GetName( ), m_DBBanLast->GetIP( ), m_GameName, User, Payload );
				}

				CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + m_DBBanLast->GetName( ) + "] was banned by player [" + User + "]" );
				SendAllChat( m_GHost->m_Language->PlayerWasBannedByPlayer( m_DBBanLast->GetName( ), User ) );
			}

			//
			// !CHECK
			//

			if( Command == "check" )
			{
				if( !Payload.empty( ) )
				{
					CGamePlayer *LastMatch = NULL;
					uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToCheckPlayerNoMatchesFound( Payload ) );
					else if( Matches == 1 )
						SendAllChat( m_GHost->m_Language->CheckedPlayer( LastMatch->GetName( ), LastMatch->GetNumPings( ) > 0 ? UTIL_ToString( LastMatch->GetPing( m_GHost->m_LCPings ) ) + "ms" : "N/A", m_GHost->m_DB->FromCheck( UTIL_ByteArrayToUInt32( LastMatch->GetExternalIP( ), true ) ), m_GHost->m_DB->AdminCheck( LastMatch->GetSpoofedRealm( ), LastMatch->GetName( ) ) || RootAdminCheck ? "Yes" : "No", IsOwner( LastMatch->GetName( ) ) ? "Yes" : "No", LastMatch->GetSpoofed( ) ? "Yes" : "No", LastMatch->GetSpoofedRealm( ).empty( ) ? "N/A" : LastMatch->GetSpoofedRealm( ), LastMatch->GetReserved( ) ? "Yes" : "No" ) );
					else
						SendAllChat( m_GHost->m_Language->UnableToCheckPlayerFoundMoreThanOneMatch( Payload ) );
				}
				else
					SendAllChat( m_GHost->m_Language->CheckedPlayer( User, player->GetNumPings( ) > 0 ? UTIL_ToString( player->GetPing( m_GHost->m_LCPings ) ) + "ms" : "N/A", m_GHost->m_DB->FromCheck( UTIL_ByteArrayToUInt32( player->GetExternalIP( ), true ) ), m_GHost->m_DB->AdminCheck( player->GetSpoofedRealm( ), User ) || RootAdminCheck ? "Yes" : "No", IsOwner( User ) ? "Yes" : "No", player->GetSpoofed( ) ? "Yes" : "No", player->GetSpoofedRealm( ).empty( ) ? "N/A" : player->GetSpoofedRealm( ), player->GetReserved( ) ? "Yes" : "No" ) );
			}

			//
			// !CHECKBAN
			//

			if( Command == "checkban" && !Payload.empty( ) && !m_GHost->m_BNETs.empty( ) )
			{
				for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				{
					CDBBan *Ban = m_GHost->m_DB->BanCheck( (*i)->GetServer( ), Payload );

					if( Ban )
					{
						SendAllChat( m_GHost->m_Language->UserWasBannedOnByBecause( (*i)->GetServer( ), Payload, Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ) );
						delete Ban;
						Ban = NULL;
					}
					else
						SendAllChat( m_GHost->m_Language->UserIsNotBanned( (*i)->GetServer( ), Payload ) );
				}
			}

			//
			// !CLOSE (close slot)
			//

			if( Command == "close" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				// close as many slots as specified, e.g. "5 10" closes slots 5 and 10

				stringstream SS;
				SS << Payload;

				while( !SS.eof( ) )
				{
					uint32_t SID;
					SS >> SID;

					if( SS.fail( ) )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input to close command" );
						break;
					}
					else
						CloseSlot( (unsigned char)( SID - 1 ), true );
				}
			}

			//
			// !CLOSEALL
			//

			if( Command == "closeall" && !m_GameLoading && !m_GameLoaded )
				CloseAllSlots( );

			//
			// !COMP (computer slot)
			//

			if( Command == "comp" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the skill
				// e.g. "1 2" -> slot: "1", skill: "2"

				uint32_t Slot;
				uint32_t Skill = 1;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to comp command" );
				else
				{
					if( !SS.eof( ) )
						SS >> Skill;

					if( SS.fail( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to comp command" );
					else
						ComputerSlot( (unsigned char)( Slot - 1 ), (unsigned char)Skill, true );
				}
			}

			//
			// !COMPCOLOUR (computer colour change)
			//

			if( Command == "compcolour" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the colour
				// e.g. "1 2" -> slot: "1", colour: "2"

				uint32_t Slot;
				uint32_t Colour;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to compcolour command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to compcolour command" );
					else
					{
						SS >> Colour;

						if( SS.fail( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to compcolour command" );
						else
						{
							unsigned char SID = (unsigned char)( Slot - 1 );

							if( m_Map->GetMapGameType( ) != GAMETYPE_CUSTOM && Colour < 12 && SID < m_Slots.size( ) )
							{
								if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
									ColourSlot( SID, Colour );
							}
						}
					}
				}
			}

			//
			// !COMPHANDICAP (computer handicap change)
			//

			if( Command == "comphandicap" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the handicap
				// e.g. "1 50" -> slot: "1", handicap: "50"

				uint32_t Slot;
				uint32_t Handicap;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to comphandicap command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to comphandicap command" );
					else
					{
						SS >> Handicap;

						if( SS.fail( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to comphandicap command" );
						else
						{
							unsigned char SID = (unsigned char)( Slot - 1 );

							if( m_Map->GetMapGameType( ) != GAMETYPE_CUSTOM && ( Handicap == 50 || Handicap == 60 || Handicap == 70 || Handicap == 80 || Handicap == 90 || Handicap == 100 ) && SID < m_Slots.size( ) )
							{
								if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
								{
									m_Slots[SID].SetHandicap( (unsigned char)Handicap );
									SendAllSlotInfo( );
								}
							}
						}
					}
				}
			}

			//
			// !COMPRACE (computer race change)
			//

			if( Command == "comprace" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the race
				// e.g. "1 human" -> slot: "1", race: "human"

				uint32_t Slot;
				string Race;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to comprace command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to comprace command" );
					else
					{
						getline( SS, Race );
						string :: size_type Start = Race.find_first_not_of( " " );

						if( Start != string :: npos )
							Race = Race.substr( Start );

						transform( Race.begin( ), Race.end( ), Race.begin( ), (int(*)(int))tolower );
						unsigned char SID = (unsigned char)( Slot - 1 );

						if( m_Map->GetMapGameType( ) != GAMETYPE_CUSTOM && !( m_Map->GetMapFlags( ) & MAPFLAG_RANDOMRACES ) && SID < m_Slots.size( ) )
						{
							if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
							{
								if( Race == "human" )
								{
									m_Slots[SID].SetRace( SLOTRACE_HUMAN );
									SendAllSlotInfo( );
								}
								else if( Race == "orc" )
								{
									m_Slots[SID].SetRace( SLOTRACE_ORC );
									SendAllSlotInfo( );
								}
								else if( Race == "night elf" )
								{
									m_Slots[SID].SetRace( SLOTRACE_NIGHTELF );
									SendAllSlotInfo( );
								}
								else if( Race == "undead" )
								{
									m_Slots[SID].SetRace( SLOTRACE_UNDEAD );
									SendAllSlotInfo( );
								}
								else if( Race == "random" )
								{
									m_Slots[SID].SetRace( SLOTRACE_RANDOM );
									SendAllSlotInfo( );
								}
								else
									CONSOLE_Print( "[GAME: " + m_GameName + "] unknown race [" + Race + "] sent to comprace command" );
							}
						}
					}
				}
			}

			//
			// !COMPTEAM (computer team change)
			//

			if( Command == "compteam" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the team
				// e.g. "1 2" -> slot: "1", team: "2"

				uint32_t Slot;
				uint32_t Team;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to compteam command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to compteam command" );
					else
					{
						SS >> Team;

						if( SS.fail( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to compteam command" );
						else
						{
							unsigned char SID = (unsigned char)( Slot - 1 );

							if( m_Map->GetMapGameType( ) != GAMETYPE_CUSTOM && Team < 12 && SID < m_Slots.size( ) )
							{
								if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
								{
									m_Slots[SID].SetTeam( (unsigned char)( Team - 1 ) );
									SendAllSlotInfo( );
								}
							}
						}
					}
				}
			}

			//
			// !DOWNLOAD
			// !DL
			//

			if( ( Command == "download" || Command == "dl" ) && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				CGamePlayer *LastMatch = NULL;
				uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToStartDownloadNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					if( !LastMatch->GetDownloadStarted( ) && !LastMatch->GetDownloadFinished( ) )
					{
						unsigned char SID = GetSIDFromPID( LastMatch->GetPID( ) );

						if( SID < m_Slots.size( ) && m_Slots[SID].GetDownloadStatus( ) != 100 )
						{
							// inform the client that we are willing to send the map

							CONSOLE_Print( "[GAME: " + m_GameName + "] map download started for player [" + LastMatch->GetName( ) + "]" );
							Send( LastMatch, m_Protocol->SEND_W3GS_STARTDOWNLOAD( GetHostPID( ) ) );
							LastMatch->SetDownloadAllowed( true );
							LastMatch->SetDownloadStarted( true );
							LastMatch->SetStartedDownloadingTicks( GetTicks( ) );
						}
					}
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToStartDownloadFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !DROP
			//

			if( Command == "drop" && m_GameLoaded )
				StopLaggers( "lagged out (dropped by admin)" );

			//
			// !END
			//

			if( Command == "end" && m_GameLoaded )
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] is over (admin ended game)" );
				StopPlayers( "was disconnected (admin ended game)" );
			}

			//
			// !FROM
			//

			if( Command == "from" )
			{
				string Froms;

				for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
				{
					// we reverse the byte order on the IP because it's stored in network byte order

					Froms += (*i)->GetName( );
					Froms += ": (";
					Froms += m_GHost->m_DB->FromCheck( UTIL_ByteArrayToUInt32( (*i)->GetExternalIP( ), true ) );
					Froms += ")";

					if( i != m_Players.end( ) - 1 )
						Froms += ", ";
				}

				SendAllChat( Froms );
			}

			//
			// !HOLD (hold a slot for someone)
			//

			if( Command == "hold" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				// hold as many players as specified, e.g. "Varlock Kilranin" holds players "Varlock" and "Kilranin"

				stringstream SS;
				SS << Payload;

				while( !SS.eof( ) )
				{
					string HoldName;
					SS >> HoldName;

					if( SS.fail( ) )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input to hold command" );
						break;
					}
					else
					{
						SendAllChat( m_GHost->m_Language->AddedPlayerToTheHoldList( HoldName ) );
						AddToReserved( HoldName );
					}
				}
			}

			//
			// !KICK (kick a player)
			//

			if( Command == "kick" && !Payload.empty( ) )
			{
				CGamePlayer *LastMatch = NULL;
				uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToKickNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					LastMatch->SetDeleteMe( true );
					LastMatch->SetLeftReason( m_GHost->m_Language->WasKickedByPlayer( User ) );

					if( !m_GameLoading && !m_GameLoaded )
						LastMatch->SetLeftCode( PLAYERLEAVE_LOBBY );
					else
						LastMatch->SetLeftCode( PLAYERLEAVE_LOST );

					if( !m_GameLoading && !m_GameLoaded )
						OpenSlot( GetSIDFromPID( LastMatch->GetPID( ) ), false );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToKickFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !LATENCY (set game latency)
			//

			if( Command == "latency" )
			{
				if( Payload.empty( ) )
					SendAllChat( m_GHost->m_Language->LatencyIs( UTIL_ToString( m_Latency ) ) );
				else
				{
					m_Latency = UTIL_ToUInt32( Payload );

					if( m_Latency <= 50 )
					{
						m_Latency = 50;
						SendAllChat( m_GHost->m_Language->SettingLatencyToMinimum( "50" ) );
					}
					else if( m_Latency >= 500 )
					{
						m_Latency = 500;
						SendAllChat( m_GHost->m_Language->SettingLatencyToMaximum( "500" ) );
					}
					else
						SendAllChat( m_GHost->m_Language->SettingLatencyTo( UTIL_ToString( m_Latency ) ) );
				}
			}

			//
			// !LOCK
			//

			if( Command == "lock" && IsOwner( User ) )
			{
				SendAllChat( m_GHost->m_Language->GameLocked( ) );
				m_Locked = true;
			}

			//
			// !MUTE
			//

			if( Command == "mute" )
			{
				CGamePlayer *LastMatch = NULL;
				uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToMuteNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					SendAllChat( m_GHost->m_Language->MutedPlayer( LastMatch->GetName( ), User ) );
					LastMatch->SetMuted( true );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToMuteFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !MUTEALL
			//

			if( Command == "muteall" && m_GameLoaded )
			{
				SendAllChat( m_GHost->m_Language->GlobalChatMuted( ) );
				m_MuteAll = true;
			}

			//
			// !OPEN (open slot)
			//

			if( Command == "open" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				// open as many slots as specified, e.g. "5 10" opens slots 5 and 10

				stringstream SS;
				SS << Payload;

				while( !SS.eof( ) )
				{
					uint32_t SID;
					SS >> SID;

					if( SS.fail( ) )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input to open command" );
						break;
					}
					else
						OpenSlot( (unsigned char)( SID - 1 ), true );
				}
			}

			//
			// !OPENALL
			//

			if( Command == "openall" && !m_GameLoading && !m_GameLoaded )
				OpenAllSlots( );

			//
			// !OWNER (set game owner)
			//

			if( Command == "owner" )
			{
				if( RootAdminCheck || IsOwner( User ) || !GetPlayerFromName( m_OwnerName, false ) )
				{
					if( !Payload.empty( ) )
					{
						SendAllChat( m_GHost->m_Language->SettingGameOwnerTo( Payload ) );
						m_OwnerName = Payload;
					}
					else
					{
						SendAllChat( m_GHost->m_Language->SettingGameOwnerTo( User ) );
						m_OwnerName = User;
					}
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToSetGameOwner( m_OwnerName ) );
			}

			//
			// !PING
			//

			if( Command == "ping" )
			{
				// kick players with ping higher than payload if payload isn't empty
				// we only do this if the game hasn't started since we don't want to kick players from a game in progress

				uint32_t Kicked = 0;
				uint32_t KickPing = 0;

				if( !m_GameLoading && !m_GameLoaded && !Payload.empty( ) )
					KickPing = UTIL_ToUInt32( Payload );

				// copy the m_Players vector so we can sort by descending ping so it's easier to find players with high pings

				vector<CGamePlayer *> SortedPlayers = m_Players;
				sort( SortedPlayers.begin( ), SortedPlayers.end( ), CGamePlayerSortDescByPing( ) );
				string Pings;

				for( vector<CGamePlayer *> :: iterator i = SortedPlayers.begin( ); i != SortedPlayers.end( ); i++ )
				{
					Pings += (*i)->GetName( );
					Pings += ": ";

					if( (*i)->GetNumPings( ) > 0 )
					{
						Pings += UTIL_ToString( (*i)->GetPing( m_GHost->m_LCPings ) );

						if( !m_GameLoading && !m_GameLoaded && !(*i)->GetReserved( ) && !Payload.empty( ) && (*i)->GetPing( m_GHost->m_LCPings ) > KickPing )
						{
							(*i)->SetDeleteMe( true );
							(*i)->SetLeftReason( "was kicked for excessive ping " + UTIL_ToString( (*i)->GetPing( m_GHost->m_LCPings ) ) + " > " + UTIL_ToString( KickPing ) );
							(*i)->SetLeftCode( PLAYERLEAVE_LOBBY );
							OpenSlot( GetSIDFromPID( (*i)->GetPID( ) ), false );
							Kicked++;
						}

						Pings += "ms";
					}
					else
						Pings += "N/A";

					if( i != SortedPlayers.end( ) - 1 )
						Pings += ", ";
				}

				SendAllChat( Pings );

				if( Kicked > 0 )
					SendAllChat( m_GHost->m_Language->KickingPlayersWithPingsGreaterThan( UTIL_ToString( Kicked ), UTIL_ToString( KickPing ) ) );
			}

			//
			// !PRIV (rehost as private game)
			//

			if( Command == "priv" && !Payload.empty( ) && !m_CountDownStarted && !m_SaveGame )
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] trying to rehost as private game [" + Payload + "]" );
				m_GameState = GAME_PRIVATE;
				m_GameName = Payload;

				for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				{
					(*i)->SendGameUncreate( );
					(*i)->SendGameCreate( m_GameState, m_GameName, string( ), m_Map, NULL );
				}

				m_CreationTime = GetTime( );
				m_LastRefreshTime = GetTime( );
			}

			//
			// !PUB (rehost as public game)
			//

			if( Command == "pub" && !Payload.empty( ) && !m_CountDownStarted && !m_SaveGame )
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] trying to rehost as public game [" + Payload + "]" );
				m_GameState = GAME_PUBLIC;
				m_GameName = Payload;

				for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				{
					(*i)->SendGameUncreate( );
					(*i)->SendGameCreate( m_GameState, m_GameName, string( ), m_Map, NULL );
				}

				m_CreationTime = GetTime( );
				m_LastRefreshTime = GetTime( );
			}

			//
			// !REFRESH (turn on or off refresh messages)
			//

			if( Command == "refresh" && !m_CountDownStarted )
			{
				if( Payload == "on" )
				{
					SendAllChat( m_GHost->m_Language->RefreshMessagesEnabled( ) );
					m_RefreshMessages = true;
				}
				else if( Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->RefreshMessagesDisabled( ) );
					m_RefreshMessages = false;
				}
			}

			//
			// !SENDLAN
			//

			if( Command == "sendlan" && !Payload.empty( ) && !m_CountDownStarted )
			{
				// extract the ip and the port
				// e.g. "1.2.3.4 6112" -> ip: "1.2.3.4", port: "6112"

				string IP;
				uint32_t Port = 6112;
				stringstream SS;
				SS << Payload;
				SS >> IP;

				if( !SS.eof( ) )
					SS >> Port;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad inputs to sendlan command" );
				else
				{
					// we send 12 for SlotsTotal because this determines how many PID's Warcraft 3 allocates
					// we need to make sure Warcraft 3 allocates at least SlotsTotal + 1 but at most 12 PID's
					// this is because we need an extra PID for the virtual host player (but we always delete the virtual host player when the 12th person joins)
					// however, we can't send 13 for SlotsTotal because this causes Warcraft 3 to crash when sharing control of units
					// nor can we send SlotsTotal because then Warcraft 3 crashes when playing maps with less than 12 PID's (because of the virtual host player taking an extra PID)
					// we also send 12 for SlotsOpen because Warcraft 3 assumes there's always at least one player in the game (the host)
					// so if we try to send accurate numbers it'll always be off by one and results in Warcraft 3 assuming the game is full when it still needs one more player
					// the easiest solution is to simply send 12 for both so the game will always show up as (1/12) players

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
						m_GHost->m_UDPSocket->SendTo( IP, Port, m_Protocol->SEND_W3GS_GAMEINFO( MapGameType, m_Map->GetMapGameFlags( ), MapWidth, MapHeight, m_GameName, "Varlock", GetTime( ) - m_CreationTime, "Save\\Multiplayer\\" + m_SaveGame->GetFileNameNoPath( ), m_SaveGame->GetMagicNumber( ), 12, 12, m_HostPort, m_HostCounter ) );
					}
					else
					{
						MapGameType.push_back( m_Map->GetMapGameType( ) );
						MapGameType.push_back( 0 );
						MapGameType.push_back( 0 );
						MapGameType.push_back( 0 );
						m_GHost->m_UDPSocket->SendTo( IP, Port, m_Protocol->SEND_W3GS_GAMEINFO( MapGameType, m_Map->GetMapGameFlags( ), m_Map->GetMapWidth( ), m_Map->GetMapHeight( ), m_GameName, "Varlock", GetTime( ) - m_CreationTime, m_Map->GetMapPath( ), m_Map->GetMapCRC( ), 12, 12, m_HostPort, m_HostCounter ) );
					}
				}
			}

			//
			// !SP
			//

			if( Command == "sp" && !m_CountDownStarted )
			{
				SendAllChat( m_GHost->m_Language->ShufflingPlayers( ) );
				ShuffleSlots( );
			}

			//
			// !START
			//

			if( Command == "start" && !m_CountDownStarted )
			{
				// if the player sent "!start force" skip the checks and start the countdown
				// otherwise check that the game is ready to start

				if( Payload == "force" )
					StartCountDown( true );
				else
					StartCountDown( false );
			}

			//
			// !SWAP (swap slots)
			//

			if( Command == "swap" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				uint32_t SID1;
				uint32_t SID2;
				stringstream SS;
				SS << Payload;
				SS >> SID1;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to swap command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to swap command" );
					else
					{
						SS >> SID2;

						if( SS.fail( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to swap command" );
						else
							SwapSlots( (unsigned char)( SID1 - 1 ), (unsigned char)( SID2 - 1 ) );
					}
				}
			}

			//
			// !SYNCLIMIT
			//

			if( Command == "synclimit" )
			{
				if( Payload.empty( ) )
					SendAllChat( m_GHost->m_Language->SyncLimitIs( UTIL_ToString( m_SyncLimit ) ) );
				else
				{
					m_SyncLimit = UTIL_ToUInt32( Payload );

					if( m_SyncLimit <= 10 )
					{
						m_SyncLimit = 10;
						SendAllChat( m_GHost->m_Language->SettingSyncLimitToMinimum( "10" ) );
					}
					else if( m_SyncLimit >= 10000 )
					{
						m_SyncLimit = 10000;
						SendAllChat( m_GHost->m_Language->SettingSyncLimitToMaximum( "10000" ) );
					}
					else
						SendAllChat( m_GHost->m_Language->SettingSyncLimitTo( UTIL_ToString( m_SyncLimit ) ) );
				}
			}

			//
			// !UNHOST
			//

			if( Command == "unhost" && !m_CountDownStarted )
				m_Exiting = true;

			//
			// !UNLOCK
			//

			if( Command == "unlock" && IsOwner( User ) )
			{
				SendAllChat( m_GHost->m_Language->GameUnlocked( ) );
				m_Locked = false;
			}

			//
			// !UNMUTE
			//

			if( Command == "unmute" )
			{
				CGamePlayer *LastMatch = NULL;
				uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToMuteNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					SendAllChat( m_GHost->m_Language->UnmutedPlayer( LastMatch->GetName( ), User ) );
					LastMatch->SetMuted( false );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToMuteFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !UNMUTEALL
			//

			if( Command == "unmuteall" && m_GameLoaded )
			{
				SendAllChat( m_GHost->m_Language->GlobalChatUnmuted( ) );
				m_MuteAll = false;
			}

			//
			// !VIRTUALHOST
			//

			if( Command == "virtualhost" && !Payload.empty( ) && Payload.size( ) <= 15 && !m_CountDownStarted )
			{
				DeleteVirtualHost( );
				m_VirtualHostName = Payload;
			}
		}
		else
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] admin command ignored, the game is locked" );
			SendChat( player, m_GHost->m_Language->TheGameIsLocked( ) );
		}
	}
	else
		CONSOLE_Print( "[GAME: " + m_GameName + "] user [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );

	/*********************
	* NON ADMIN COMMANDS *
	*********************/

	//
	// !CHECKME
	//

	if( Command == "checkme" )
		SendChat( player, m_GHost->m_Language->CheckedPlayer( User, player->GetNumPings( ) > 0 ? UTIL_ToString( player->GetPing( m_GHost->m_LCPings ) ) + "ms" : "N/A", m_GHost->m_DB->FromCheck( UTIL_ByteArrayToUInt32( player->GetExternalIP( ), true ) ), m_GHost->m_DB->AdminCheck( player->GetSpoofedRealm( ), User ) || RootAdminCheck ? "Yes" : "No", IsOwner( User ) ? "Yes" : "No", player->GetSpoofed( ) ? "Yes" : "No", player->GetSpoofedRealm( ).empty( ) ? "N/A" : player->GetSpoofedRealm( ), player->GetReserved( ) ? "Yes" : "No" ) );

	//
	// !STATS
	//

	if( Command == "stats" && GetTime( ) >= player->GetStatsSentTime( ) + 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;

		CDBGamePlayerSummary *GamePlayerSummary = m_GHost->m_DB->GamePlayerSummaryCheck( StatsUser );

		if( GamePlayerSummary )
		{
			if( player->GetSpoofed( ) && ( m_GHost->m_DB->AdminCheck( player->GetSpoofedRealm( ), User ) || RootAdminCheck || IsOwner( User ) ) )
				SendAllChat( m_GHost->m_Language->HasPlayedGamesWithThisBot( StatsUser, GamePlayerSummary->GetFirstGameDateTime( ), GamePlayerSummary->GetLastGameDateTime( ), UTIL_ToString( GamePlayerSummary->GetTotalGames( ) ), UTIL_ToString( (float)GamePlayerSummary->GetAvgLoadingTime( ) / 1000, 2 ), UTIL_ToString( GamePlayerSummary->GetAvgLeftPercent( ) ) ) );
			else
				SendChat( player, m_GHost->m_Language->HasPlayedGamesWithThisBot( StatsUser, GamePlayerSummary->GetFirstGameDateTime( ), GamePlayerSummary->GetLastGameDateTime( ), UTIL_ToString( GamePlayerSummary->GetTotalGames( ) ), UTIL_ToString( (float)GamePlayerSummary->GetAvgLoadingTime( ) / 1000, 2 ), UTIL_ToString( GamePlayerSummary->GetAvgLeftPercent( ) ) ) );

			delete GamePlayerSummary;
			GamePlayerSummary = NULL;
		}
		else
		{
			if( player->GetSpoofed( ) && ( m_GHost->m_DB->AdminCheck( player->GetSpoofedRealm( ), User ) || RootAdminCheck || IsOwner( User ) ) )
				SendAllChat( m_GHost->m_Language->HasntPlayedGamesWithThisBot( StatsUser ) );
			else
				SendChat( player, m_GHost->m_Language->HasntPlayedGamesWithThisBot( StatsUser ) );
		}

		player->SetStatsSentTime( GetTime( ) );
	}

	//
	// !STATSDOTA
	//

	if( Command == "statsdota" && GetTime( ) >= player->GetStatsDotASentTime( ) + 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;

		CDBDotAPlayerSummary *DotAPlayerSummary = m_GHost->m_DB->DotAPlayerSummaryCheck( StatsUser );

		if( DotAPlayerSummary )
		{
			string Summary = m_GHost->m_Language->HasPlayedDotAGamesWithThisBot(	StatsUser,
																					UTIL_ToString( DotAPlayerSummary->GetTotalGames( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalWins( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalLosses( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalKills( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalDeaths( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalCreepKills( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalCreepDenies( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalAssists( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalNeutralKills( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalTowerKills( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalRaxKills( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetTotalCourierKills( ) ),
																					UTIL_ToString( DotAPlayerSummary->GetAvgKills( ), 2 ),
																					UTIL_ToString( DotAPlayerSummary->GetAvgDeaths( ), 2 ),
																					UTIL_ToString( DotAPlayerSummary->GetAvgCreepKills( ), 2 ),
																					UTIL_ToString( DotAPlayerSummary->GetAvgCreepDenies( ), 2 ),
																					UTIL_ToString( DotAPlayerSummary->GetAvgAssists( ), 2 ),
																					UTIL_ToString( DotAPlayerSummary->GetAvgNeutralKills( ), 2 ),
																					UTIL_ToString( DotAPlayerSummary->GetAvgTowerKills( ), 2 ),
																					UTIL_ToString( DotAPlayerSummary->GetAvgRaxKills( ), 2 ),
																					UTIL_ToString( DotAPlayerSummary->GetAvgCourierKills( ), 2 ) );

			if( player->GetSpoofed( ) && ( m_GHost->m_DB->AdminCheck( player->GetSpoofedRealm( ), User ) || RootAdminCheck || IsOwner( User ) ) )
				SendAllChat( Summary );
			else
				SendChat( player, Summary );

			delete DotAPlayerSummary;
			DotAPlayerSummary = NULL;
		}
		else
		{
			if( player->GetSpoofed( ) && ( m_GHost->m_DB->AdminCheck( player->GetSpoofedRealm( ), User ) || RootAdminCheck || IsOwner( User ) ) )
				SendAllChat( m_GHost->m_Language->HasntPlayedDotAGamesWithThisBot( StatsUser ) );
			else
				SendChat( player, m_GHost->m_Language->HasntPlayedDotAGamesWithThisBot( StatsUser ) );
		}

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !VERSION
	//

	if( Command == "version" )
	{
		if( player->GetSpoofed( ) && ( m_GHost->m_DB->AdminCheck( player->GetSpoofedRealm( ), User ) || RootAdminCheck || IsOwner( User ) ) )
			SendChat( player, m_GHost->m_Language->VersionAdmin( m_GHost->m_Version ) );
		else
			SendChat( player, m_GHost->m_Language->VersionNotAdmin( m_GHost->m_Version ) );
	}
}

void CGame :: EventGameStarted( )
{
	CBaseGame :: EventGameStarted( );

	// record everything we need to ban each player in case we decide to do so later
	// this is because when a player leaves the game an admin might want to ban that player
	// but since the player has already left the game we don't have access to their information anymore
	// so we create a "potential ban" for each player and only store it in the database if requested to by an admin

	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		m_DBBans.push_back( new CDBBan( (*i)->GetSpoofedRealm( ), (*i)->GetName( ), (*i)->GetExternalIPString( ), string( ), string( ), string( ), string( ) ) );
}

//
// CAdminGame
//

CAdminGame :: CAdminGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nPassword ) : CBaseGame( nGHost, nMap, nSaveGame, nHostPort, nGameState, nGameName, string( ), string( ), string( ) )
{
	m_VirtualHostName = "|cFFC04040Admin";
	m_MuteLobby = true;
	m_Password = nPassword;
}

CAdminGame :: ~CAdminGame( )
{

}

bool CAdminGame :: Update( void *fd )
{
	// reset the last reserved seen timer since the admin game should never be considered abandoned

	m_LastReservedSeen = GetTime( );
	return CBaseGame :: Update( fd );
}

void CAdminGame :: SendWelcomeMessage( CGamePlayer *player )
{
	SendChat( player, " " );
	SendChat( player, " " );
	SendChat( player, "GHost++ Admin Game                    http://forum.codelain.com/" );
	SendChat( player, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" );
	SendChat( player, "Commands: addadmin, autohost, checkadmin, countadmins, deladmin" );
	SendChat( player, "Commands: disable, enable, end, exit, getgame, getgames" );
	SendChat( player, "Commands: hostsg, load, loadsg, map, password, priv, privby" );
	SendChat( player, "Commands: pub, pubby, quit, saygame, saygames, unhost" );
}

void CAdminGame :: EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer )
{
	uint32_t Time = GetTime( );

	for( vector<TempBan> :: iterator i = m_TempBans.begin( ); i != m_TempBans.end( ); )
	{
		// remove old tempbans

		if( Time >= (*i).second )
			i = m_TempBans.erase( i );
		else
		{
			if( (*i).first == potential->GetExternalIPString( ) )
			{
				// tempbanned, goodbye
				// this is just going to close the socket which displays a "game not found" message to the player

				potential->SetDeleteMe( true );
				CONSOLE_Print( "[ADMINGAME] player [" + joinPlayer->GetName( ) + "] at ip [" + (*i).first + "] is trying to join the game but is tempbanned" );
				return;
			}

			i++;
		}
	}

	CBaseGame :: EventPlayerJoined( potential, joinPlayer );
}

void CAdminGame :: EventPlayerBotCommand( CGamePlayer *player, string command, string payload )
{
	CBaseGame :: EventPlayerBotCommand( player, command, payload );

	// todotodo: don't be lazy

	string User = player->GetName( );
	string Command = command;
	string Payload = payload;

	if( player->GetLoggedIn( ) )
	{
		CONSOLE_Print( "[ADMINGAME] admin [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );

		/*****************
		* ADMIN COMMANDS *
		******************/

		//
		// !ADDADMIN
		//

		if( Command == "addadmin" && !Payload.empty( ) )
		{
			// extract the name and the server
			// e.g. "Varlock useast.battle.net" -> name: "Varlock", server: "useast.battle.net"

			string Name;
			string Server;
			stringstream SS;
			SS << Payload;
			SS >> Name;

			if( SS.eof( ) )
			{
				if( m_GHost->m_BNETs.size( ) == 1 )
					Server = m_GHost->m_BNETs[0]->GetServer( );
				else
					CONSOLE_Print( "[ADMINGAME] missing input #2 to addadmin command" );
			}
			else
				SS >> Server;

			if( !Server.empty( ) )
			{
				if( m_GHost->m_DB->AdminCheck( Server, Name ) )
					SendChat( player, m_GHost->m_Language->UserIsAlreadyAnAdmin( Server, Name ) );
				else
				{
					if( m_GHost->m_DB->AdminAdd( Server, Name ) )
						SendChat( player, m_GHost->m_Language->AddedUserToAdminDatabase( Server, Name ) );
					else
						SendChat( player, m_GHost->m_Language->ErrorAddingUserToAdminDatabase( Server, Name ) );
				}
			}
		}

		//
		// !AUTOHOST
		//

		if( Command == "autohost" )
		{
			if( Payload.empty( ) || Payload == "off" )
			{
				SendChat( player, m_GHost->m_Language->AutoHostDisabled( ) );
				m_GHost->m_AutoHostGameName.clear( );
				m_GHost->m_AutoHostMapCFG.clear( );
				m_GHost->m_AutoHostOwner.clear( );
				m_GHost->m_AutoHostServer.clear( );
				m_GHost->m_AutoHostMaximumGames = 0;
				m_GHost->m_AutoHostAutoStartPlayers = 0;
				m_GHost->m_LastAutoHostTime = GetTime( );
			}
			else
			{
				// extract the maximum games, auto start players, and the game name
				// e.g. "5 10 BattleShips Pro" -> maximum games: "5", auto start players: "10", game name: "BattleShips Pro"

				uint32_t MaximumGames;
				uint32_t AutoStartPlayers;
				string GameName;
				stringstream SS;
				SS << Payload;
				SS >> MaximumGames;

				if( SS.fail( ) || MaximumGames == 0 )
					CONSOLE_Print( "[ADMINGAME] bad input #1 to autohost command" );
				else
				{
					SS >> AutoStartPlayers;

					if( SS.fail( ) || AutoStartPlayers == 0 )
						CONSOLE_Print( "[ADMINGAME] bad input #2 to autohost command" );
					else
					{
						if( SS.eof( ) )
							CONSOLE_Print( "[ADMINGAME] missing input #3 to autohost command" );
						else
						{
							getline( SS, GameName );
							string :: size_type Start = GameName.find_first_not_of( " " );

							if( Start != string :: npos )
								GameName = GameName.substr( Start );

							SendChat( player, m_GHost->m_Language->AutoHostEnabled( ) );
							m_GHost->m_AutoHostGameName = GameName;
							m_GHost->m_AutoHostMapCFG = m_GHost->m_Map->GetCFGFile( );
							m_GHost->m_AutoHostOwner = User;
							m_GHost->m_AutoHostServer.clear( );
							m_GHost->m_AutoHostMaximumGames = MaximumGames;
							m_GHost->m_AutoHostAutoStartPlayers = AutoStartPlayers;
							m_GHost->m_LastAutoHostTime = GetTime( );
						}
					}
				}
			}
		}

		//
		// !CHECKADMIN
		//

		if( Command == "checkadmin" && !Payload.empty( ) )
		{
			// extract the name and the server
			// e.g. "Varlock useast.battle.net" -> name: "Varlock", server: "useast.battle.net"

			string Name;
			string Server;
			stringstream SS;
			SS << Payload;
			SS >> Name;

			if( SS.eof( ) )
			{
				if( m_GHost->m_BNETs.size( ) == 1 )
					Server = m_GHost->m_BNETs[0]->GetServer( );
				else
					CONSOLE_Print( "[ADMINGAME] missing input #2 to checkadmin command" );
			}
			else
				SS >> Server;

			if( !Server.empty( ) )
			{
				if( m_GHost->m_DB->AdminCheck( Server, Name ) )
					SendChat( player, m_GHost->m_Language->UserIsAnAdmin( Server, Name ) );
				else
					SendChat( player, m_GHost->m_Language->UserIsNotAnAdmin( Server, Name ) );
			}
		}

		//
		// !COUNTADMINS
		//

		if( Command == "countadmins" )
		{
			string Server = Payload;

			if( Server.empty( ) && m_GHost->m_BNETs.size( ) == 1 )
				Server = m_GHost->m_BNETs[0]->GetServer( );

			if( !Server.empty( ) )
			{
				uint32_t Count = m_GHost->m_DB->AdminCount( Server );

				if( Count == 0 )
					SendChat( player, m_GHost->m_Language->ThereAreNoAdmins( Server ) );
				else if( Count == 1 )
					SendChat( player, m_GHost->m_Language->ThereIsAdmin( Server ) );
				else
					SendChat( player, m_GHost->m_Language->ThereAreAdmins( Server, UTIL_ToString( Count ) ) );
			}
		}

		//
		// !DELADMIN
		//

		if( Command == "deladmin" && !Payload.empty( ) )
		{
			// extract the name and the server
			// e.g. "Varlock useast.battle.net" -> name: "Varlock", server: "useast.battle.net"

			string Name;
			string Server;
			stringstream SS;
			SS << Payload;
			SS >> Name;

			if( SS.eof( ) )
			{
				if( m_GHost->m_BNETs.size( ) == 1 )
					Server = m_GHost->m_BNETs[0]->GetServer( );
				else
					CONSOLE_Print( "[ADMINGAME] missing input #2 to deladmin command" );
			}
			else
				SS >> Server;

			if( !Server.empty( ) )
			{
				if( !m_GHost->m_DB->AdminCheck( Server, Name ) )
					SendChat( player, m_GHost->m_Language->UserIsNotAnAdmin( Server, Name ) );
				else
				{
					if( m_GHost->m_DB->AdminRemove( Server, Name ) )
						SendChat( player, m_GHost->m_Language->DeletedUserFromAdminDatabase( Server, Name ) );
					else
						SendChat( player, m_GHost->m_Language->ErrorDeletingUserFromAdminDatabase( Server, Name ) );
				}
			}
		}

		//
		// !DISABLE
		//

		if( Command == "disable" )
		{
			SendChat( player, m_GHost->m_Language->BotDisabled( ) );
			m_GHost->m_Enabled = false;
		}

		//
		// !ENABLE
		//

		if( Command == "enable" )
		{
			SendChat( player, m_GHost->m_Language->BotEnabled( ) );
			m_GHost->m_Enabled = true;
		}

		//
		// !END
		//

		if( Command == "end" && !Payload.empty( ) )
		{
			// todotodo: what if a game ends just as you're typing this command and the numbering changes?

			uint32_t GameNumber = UTIL_ToUInt32( Payload ) - 1;

			if( GameNumber < m_GHost->m_Games.size( ) )
			{
				SendChat( player, m_GHost->m_Language->EndingGame( m_GHost->m_Games[GameNumber]->GetDescription( ) ) );
				CONSOLE_Print( "[GAME: " + m_GHost->m_Games[GameNumber]->GetGameName( ) + "] is over (admin ended game)" );
				m_GHost->m_Games[GameNumber]->StopPlayers( "was disconnected (admin ended game)" );
			}
			else
				SendChat( player, m_GHost->m_Language->GameNumberDoesntExist( Payload ) );
		}

		//
		// !EXIT
		// !QUIT
		//

		if( Command == "exit" || Command == "quit" )
		{
			if( Payload == "force" )
				m_Exiting = true;
			else
			{
				if( m_GHost->m_CurrentGame || !m_GHost->m_Games.empty( ) )
					SendChat( player, m_GHost->m_Language->AtLeastOneGameActiveUseForceToShutdown( ) );
				else
					m_Exiting = true;
			}
		}

		//
		// !GETGAME
		//

		if( Command == "getgame" && !Payload.empty( ) )
		{
			uint32_t GameNumber = UTIL_ToUInt32( Payload ) - 1;

			if( GameNumber < m_GHost->m_Games.size( ) )
				SendChat( player, m_GHost->m_Language->GameNumberIs( Payload, m_GHost->m_Games[GameNumber]->GetDescription( ) ) );
			else
				SendChat( player, m_GHost->m_Language->GameNumberDoesntExist( Payload ) );
		}

		//
		// !GETGAMES
		//

		if( Command == "getgames" )
		{
			if( m_GHost->m_CurrentGame )
				SendChat( player, m_GHost->m_Language->GameIsInTheLobby( m_GHost->m_CurrentGame->GetDescription( ), UTIL_ToString( m_GHost->m_Games.size( ) ), UTIL_ToString( m_GHost->m_MaxGames ) ) );
			else
				SendChat( player, m_GHost->m_Language->ThereIsNoGameInTheLobby( UTIL_ToString( m_GHost->m_Games.size( ) ), UTIL_ToString( m_GHost->m_MaxGames ) ) );
		}

		if( Command == "hostsg" && !Payload.empty( ) )
			m_GHost->CreateGame( GAME_PRIVATE, true, Payload, User, User, string( ), false );

		//
		// !LOAD (load config file)
		// !MAP
		//

		if( Command == "load" || Command == "map" )
		{
			if( Payload.empty( ) )
				SendChat( player, m_GHost->m_Language->CurrentlyLoadedMapCFGIs( m_GHost->m_Map->GetCFGFile( ) ) );
			else
			{
				// only load files in the current directory just to be safe

				if( Payload.find( "/" ) != string :: npos || Payload.find( "\\" ) != string :: npos )
					SendChat( player, m_GHost->m_Language->UnableToLoadConfigFilesOutside( ) );
				else
				{
					string File = m_GHost->m_MapCFGPath + Payload + ".cfg";

					if( UTIL_FileExists( File ) )
					{
						// we have to be careful here because we didn't copy the map data when creating the game (there's only one global copy)
						// therefore if we change the map data while a game is in the lobby everything will get screwed up
						// the easiest solution is to simply reject the command if a game is in the lobby

						if( m_GHost->m_CurrentGame )
							SendChat( player, m_GHost->m_Language->UnableToLoadConfigFileGameInLobby( ) );
						else
						{
							SendChat( player, m_GHost->m_Language->LoadingConfigFile( File ) );
							CConfig MapCFG;
							MapCFG.Read( File );
							m_GHost->m_Map->Load( &MapCFG, File );
						}
					}
					else
						SendChat( player, m_GHost->m_Language->UnableToLoadConfigFileDoesntExist( File ) );
				}
			}
		}

		//
		// !LOADSG
		//

		if( Command == "loadsg" && !Payload.empty( ) )
		{
			// only load files in the current directory just to be safe

			if( Payload.find( "/" ) != string :: npos || Payload.find( "\\" ) != string :: npos )
				SendChat( player, m_GHost->m_Language->UnableToLoadSaveGamesOutside( ) );
			else
			{
				string File = m_GHost->m_SaveGamePath + Payload + ".w3z";
				string FileNoPath = Payload + ".w3z";

				if( UTIL_FileExists( File ) )
				{
					if( m_GHost->m_CurrentGame )
						SendChat( player, m_GHost->m_Language->UnableToLoadSaveGameGameInLobby( ) );
					else
					{
						SendChat( player, m_GHost->m_Language->LoadingSaveGame( File ) );
						m_GHost->m_SaveGame->Load( File, false );
						m_GHost->m_SaveGame->ParseSaveGame( );
						m_GHost->m_SaveGame->SetFileName( File );
						m_GHost->m_SaveGame->SetFileNameNoPath( FileNoPath );
					}
				}
				else
					SendChat( player, m_GHost->m_Language->UnableToLoadSaveGameDoesntExist( File ) );
			}
		}

		//
		// !PRIV (host private game)
		//

		if( Command == "priv" && !Payload.empty( ) )
			m_GHost->CreateGame( GAME_PRIVATE, false, Payload, User, User, string( ), false );

		//
		// !PRIVBY (host private game by other player)
		//

		if( Command == "privby" && !Payload.empty( ) )
		{
			// extract the owner and the game name
			// e.g. "Varlock dota 6.54b arem ~~~" -> owner: "Varlock", game name: "dota 6.54b arem ~~~"

			string Owner;
			string GameName;
			string :: size_type GameNameStart = Payload.find( " " );

			if( GameNameStart != string :: npos )
			{
				Owner = Payload.substr( 0, GameNameStart );
				GameName = Payload.substr( GameNameStart + 1 );
				m_GHost->CreateGame( GAME_PRIVATE, false, GameName, Owner, User, string( ), false );
			}
		}

		//
		// !PUB (host public game)
		//

		if( Command == "pub" && !Payload.empty( ) )
			m_GHost->CreateGame( GAME_PUBLIC, false, Payload, User, User, string( ), false );

		//
		// !PUBBY (host public game by other player)
		//

		if( Command == "pubby" && !Payload.empty( ) )
		{
			// extract the owner and the game name
			// e.g. "Varlock dota 6.54b arem ~~~" -> owner: "Varlock", game name: "dota 6.54b arem ~~~"

			string Owner;
			string GameName;
			string :: size_type GameNameStart = Payload.find( " " );

			if( GameNameStart != string :: npos )
			{
				Owner = Payload.substr( 0, GameNameStart );
				GameName = Payload.substr( GameNameStart + 1 );
				m_GHost->CreateGame( GAME_PUBLIC, false, GameName, Owner, User, string( ), false );
			}
		}

		//
		// !SAYGAME
		//

		if( Command == "saygame" && !Payload.empty( ) )
		{
			// extract the game number and the message
			// e.g. "3 hello everyone" -> game number: "3", message: "hello everyone"

			uint32_t GameNumber;
			string Message;
			stringstream SS;
			SS << Payload;
			SS >> GameNumber;

			if( SS.fail( ) )
				CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to saygame command" );
			else
			{
				if( SS.eof( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to saygame command" );
				else
				{
					getline( SS, Message );
					string :: size_type Start = Message.find_first_not_of( " " );

					if( Start != string :: npos )
						Message = Message.substr( Start );

					if( GameNumber - 1 < m_GHost->m_Games.size( ) )
						m_GHost->m_Games[GameNumber - 1]->SendAllChat( "ADMIN: " + Message );
					else
						SendChat( player, m_GHost->m_Language->GameNumberDoesntExist( UTIL_ToString( GameNumber ) ) );
				}
			}
		}

		//
		// !SAYGAMES
		//

		if( Command == "saygames" && !Payload.empty( ) )
		{
			if( m_GHost->m_CurrentGame )
				m_GHost->m_CurrentGame->SendAllChat( Payload );

			for( vector<CBaseGame *> :: iterator i = m_GHost->m_Games.begin( ); i != m_GHost->m_Games.end( ); i++ )
				(*i)->SendAllChat( "ADMIN: " + Payload );
		}

		//
		// !UNHOST
		//

		if( Command == "unhost" )
		{
			if( m_GHost->m_CurrentGame )
			{
				if( m_GHost->m_CurrentGame->GetCountDownStarted( ) )
					SendChat( player, m_GHost->m_Language->UnableToUnhostGameCountdownStarted( m_GHost->m_CurrentGame->GetDescription( ) ) );
				else
				{
					SendChat( player, m_GHost->m_Language->UnhostingGame( m_GHost->m_CurrentGame->GetDescription( ) ) );
					m_GHost->m_CurrentGame->SetExiting( true );
				}
			}
			else
				SendChat( player, m_GHost->m_Language->UnableToUnhostGameNoGameInLobby( ) );
		}
	}
	else
		CONSOLE_Print( "[ADMINGAME] user [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );

	/*********************
	* NON ADMIN COMMANDS *
	*********************/

	//
	// !PASSWORD
	//

	if( Command == "password" && !player->GetLoggedIn( ) )
	{
		if( !m_Password.empty( ) && Payload == m_Password )
		{
			CONSOLE_Print( "[ADMINGAME] user [" + User + "] logged in" );
			SendChat( player, m_GHost->m_Language->AdminLoggedIn( ) );
			player->SetLoggedIn( true );
		}
		else
		{
			uint32_t LoginAttempts = player->GetLoginAttempts( ) + 1;
			player->SetLoginAttempts( LoginAttempts );
			CONSOLE_Print( "[ADMINGAME] user [" + User + "] login attempt failed" );
			SendChat( player, m_GHost->m_Language->AdminInvalidPassword( UTIL_ToString( LoginAttempts ) ) );

			if( LoginAttempts >= 1 )
			{
				player->SetDeleteMe( true );
				player->SetLeftReason( "was kicked for too many failed login attempts" );

				// tempban for 5 seconds to prevent bruteforcing

				m_TempBans.push_back( TempBan( player->GetExternalIPString( ), GetTime( ) + 5 ) );
			}
		}
	}
}

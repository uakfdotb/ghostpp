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
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game.h"
#include "game_cascaded.h"

//
// CCascadedGame
//

CCascadedGame :: CCascadedGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nOwnerName, string nCreatorName, string nCreatorServer ) : CBaseGame( nGHost, nMap, nSaveGame, nHostPort, nGameState, nGameName, nOwnerName, nCreatorName, nCreatorServer )
{

}

CCascadedGame :: ~CCascadedGame( )
{

}

bool CCascadedGame :: Update( void *fd )
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

	CreateVirtualHost( );

	// ping every 5 seconds

	if( GetTime( ) >= m_LastPingTime + 5 )
	{
		SendAll( m_Protocol->SEND_W3GS_PING_FROM_HOST( ) );

		if( !m_CountDownStarted )
		{
			BYTEARRAY MapGameType;
			MapGameType.push_back( m_Map->GetMapGameType( ) );
			MapGameType.push_back( 0 );
			MapGameType.push_back( 0 );
			MapGameType.push_back( 0 );
			m_GHost->m_UDPSocket->Broadcast( 6112, m_Protocol->SEND_W3GS_GAMEINFO( MapGameType, m_Map->GetMapGameFlags( ), m_Map->GetMapWidth( ), m_Map->GetMapHeight( ), m_GameName, "Varlock", GetTime( ) - m_CreationTime, m_Map->GetMapPath( ), m_Map->GetMapCRC( ), 12, 12, m_HostPort, m_HostCounter ) );
		}

		m_LastPingTime = GetTime( );
	}

	// refresh every 2 seconds

	if( GetTime( ) >= m_LastRefreshTime + 2 )
	{
		// send a game refresh packet to each battle.net connection

		for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
			(*i)->SendGameRefresh( m_GameState, m_GameName, string( ), m_Map, m_SaveGame, GetTime( ) - m_CreationTime, m_HostCounter );

		m_LastRefreshTime = GetTime( );
	}

	// send more map data

	if( GetTicks( ) >= m_LastDownloadTicks + 250 )
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

					Send( *i, m_Protocol->SEND_W3GS_MAPPART( m_VirtualHostPID, (*i)->GetPID( ), (*i)->GetLastMapPartSent( ), m_Map->GetMapData( ) ) );
					(*i)->SetLastMapPartSent( (*i)->GetLastMapPartSent( ) + 1442 );
					DownloadCounter += 1442;
				}
			}
		}

		m_LastDownloadTicks = GetTicks( );
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

void CCascadedGame :: EventPlayerDeleted( CGamePlayer *player )
{
	CONSOLE_Print( "[CAS_GAME: " + m_GameName + "] deleting player [" + player->GetName( ) + "]: " + player->GetLeftReason( ) );
	SendAllChat( m_GHost->m_Language->PlayerLeftTheGame( player->GetName( ) ) );
}

void CCascadedGame :: EventPlayerDisconnectTimedOut( CGamePlayer *player )
{
	player->SetDeleteMe( true );
	player->SetLeftReason( m_GHost->m_Language->HasLostConnectionTimedOut( ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );
}

void CCascadedGame :: EventPlayerDisconnectPlayerError( CGamePlayer *player )
{
	player->SetDeleteMe( true );
	player->SetLeftReason( m_GHost->m_Language->HasLostConnectionPlayerError( player->GetErrorString( ) ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );
}

void CCascadedGame :: EventPlayerDisconnectSocketError( CGamePlayer *player )
{
	player->SetDeleteMe( true );
	player->SetLeftReason( m_GHost->m_Language->HasLostConnectionSocketError( player->GetSocket( )->GetErrorString( ) ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );
}

void CCascadedGame :: EventPlayerDisconnectConnectionClosed( CGamePlayer *player )
{
	player->SetDeleteMe( true );
	player->SetLeftReason( m_GHost->m_Language->HasLostConnectionClosedByRemoteHost( ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );
}

void CCascadedGame :: EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer )
{
	if( joinPlayer->GetName( ).empty( ) || joinPlayer->GetName( ).size( ) > 15 )
	{
		CONSOLE_Print( "[CAS_GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] is trying to join the game with an invalid name of length " + UTIL_ToString( joinPlayer->GetName( ).size( ) ) );
		potential->SetDeleteMe( true );
		return;
	}

	if( joinPlayer->GetName( ) == m_VirtualHostName )
	{
		CONSOLE_Print( "[CAS_GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] is trying to join the game with the virtual host name" );
		potential->SetDeleteMe( true );
		return;
	}

	if( GetPlayerFromName( joinPlayer->GetName( ), false ) )
	{
		CONSOLE_Print( "[CAS_GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] is trying to join the game but that name is already taken" );
		potential->SetDeleteMe( true );
		return;
	}

	for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		CDBBan *Ban = m_GHost->m_DB->BanCheck( (*i)->GetServer( ), joinPlayer->GetName( ) );

		if( Ban )
		{
			delete Ban;
			CONSOLE_Print( "[CAS_GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] is trying to join the game but is banned" );
			potential->SetDeleteMe( true );
			return;
		}
	}

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

	// we don't need to look for a slot for the player because every player in a CascadedGame will be in their own individual game
	// set every client's PID to 5 just because we need to avoid using the virtual host's PID (which will end up being 0) and 5 seems like a nice enough number

	CONSOLE_Print( "[CAS_GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "] joined the game" );
	SendAllChat( m_GHost->m_Language->PlayerJoinedTheGame( joinPlayer->GetName( ) ) );
	CGamePlayer *Player = new CGamePlayer( potential, 5, joinPlayer->GetName( ), joinPlayer->GetInternalIP( ), Reserved );

	if( IsOwner( joinPlayer->GetName( ) ) )
		Player->SetSpoofed( true );

	m_Players.push_back( Player );
	potential->SetSocket( NULL );
	potential->SetDeleteMe( true );
	m_Slots[0] = CGameSlot( Player->GetPID( ), 255, SLOTSTATUS_OCCUPIED, 0, m_Slots[0].GetTeam( ), m_Slots[0].GetColour( ), m_Slots[0].GetRace( ) );
	Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_SLOTINFOJOIN( Player->GetPID( ), Player->GetSocket( )->GetPort( ), Player->GetExternalIP( ), m_Slots, m_RandomSeed, m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM ? 3 : 0, m_Map->GetMapNumPlayers( ) ) );
	SendVirtualHostPlayerInfo( Player );
	Player->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_MAPCHECK( m_Map->GetMapPath( ), m_Map->GetMapSize( ), m_Map->GetMapInfo( ), m_Map->GetMapCRC( ) ) );

	// send some game information

	SendChat( Player, "There are " + UTIL_ToString( m_Players.size( ) ) + " players in this game." );
}

void CCascadedGame :: EventPlayerLeft( CGamePlayer *player )
{
	player->SetDeleteMe( true );
	player->SetLeftReason( m_GHost->m_Language->HasLeftVoluntarily( ) );
	player->SetLeftCode( PLAYERLEAVE_LOST );
}

void CCascadedGame :: EventPlayerChatToHost( CGamePlayer *player, CIncomingChatPlayer *chatPlayer )
{
	if( chatPlayer->GetFromPID( ) == player->GetPID( ) && chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_MESSAGE )
	{
		CONSOLE_Print( "[CAS_GAME: " + m_GameName + "] [" + player->GetName( ) + "]: " + chatPlayer->GetMessage( ) );

		// relay the chat message to other players via the virtual host
		// each player has the same PID so we can use the sender's PID as the recipient's PID too!

		BYTEARRAY ToPIDs;
		ToPIDs.push_back( player->GetPID( ) );
		string NewMessage = player->GetName( ) + ": " + chatPlayer->GetMessage( );

		if( NewMessage.size( ) > 254 )
			NewMessage = NewMessage.substr( 0, 254 );

		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( *i != player )
				Send( *i, m_Protocol->SEND_W3GS_CHAT_FROM_HOST( m_VirtualHostPID, ToPIDs, 16, BYTEARRAY( ), NewMessage ) );
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
}

void CCascadedGame :: EventPlayerBotCommand( CGamePlayer *player, string command, string payload )
{

}

void CCascadedGame :: EventPlayerChangeTeam( CGamePlayer *player, unsigned char team )
{

}

void CCascadedGame :: EventPlayerChangeColour( CGamePlayer *player, unsigned char colour )
{

}

void CCascadedGame :: EventPlayerChangeRace( CGamePlayer *player, unsigned char race )
{

}

void CCascadedGame :: EventPlayerChangeHandicap( CGamePlayer *player, unsigned char handicap )
{

}

void CCascadedGame :: EventPlayerMapSize( CGamePlayer *player, CIncomingMapSize *mapSize )
{
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

						CONSOLE_Print( "[CAS_GAME: " + m_GameName + "] map download started for player [" + player->GetName( ) + "]" );
						Send( player, m_Protocol->SEND_W3GS_STARTDOWNLOAD( m_VirtualHostPID ) );
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
			}
		}
		else
		{
			player->SetDeleteMe( true );
			player->SetLeftReason( "doesn't have the map and map downloads are disabled" );
			player->SetLeftCode( PLAYERLEAVE_LOBBY );
		}
	}
	else
	{
		if( player->GetDownloadStarted( ) )
		{
			// calculate download rate

			float Seconds = (float)( GetTicks( ) - player->GetStartedDownloadingTicks( ) ) / 1000;
			float Rate = (float)MapSize / 1024 / Seconds;
			CONSOLE_Print( "[CAS_GAME: " + m_GameName + "] map download finished for player [" + player->GetName( ) + "] in " + UTIL_ToString( Seconds, 1 ) + " seconds" );
			SendAllChat( m_GHost->m_Language->PlayerDownloadedTheMap( player->GetName( ), UTIL_ToString( Seconds, 1 ), UTIL_ToString( Rate, 1 ) ) );
			player->SetDownloadFinished( true );
			player->SetFinishedDownloadingTime( GetTime( ) );

			// add to database

			m_GHost->m_DB->DownloadAdd( m_Map->GetMapPath( ), MapSize, player->GetName( ), player->GetExternalIPString( ), player->GetSpoofed( ) ? 1 : 0, player->GetSpoofedRealm( ), GetTicks( ) - player->GetStartedDownloadingTicks( ) );
		}
	}

	unsigned char OldDownloadStatus = 0;

	if( mapSize->GetMapSize( ) >= 1442 )
		OldDownloadStatus = (unsigned char)( (float)( mapSize->GetMapSize( ) - 1442 ) / MapSize * 100 );

	unsigned char NewDownloadStatus = (unsigned char)( (float)mapSize->GetMapSize( ) / MapSize * 100 );
	unsigned char SID = GetSIDFromPID( player->GetPID( ) );

	if( OldDownloadStatus > 100 )
		OldDownloadStatus = 100;

	if( NewDownloadStatus > 100 )
		NewDownloadStatus = 100;

	if( SID < m_Slots.size( ) )
	{
		// only send the slot info if the download status changed

		if( OldDownloadStatus != NewDownloadStatus )
		{
			m_Slots[SID].SetDownloadStatus( NewDownloadStatus );
			Send( player, m_Protocol->SEND_W3GS_SLOTINFO( m_Slots, m_RandomSeed, m_Map->GetMapGameType( ) == GAMETYPE_CUSTOM ? 3 : 0, m_Map->GetMapNumPlayers( ) ) );
			m_Slots[SID].SetDownloadStatus( 100 );
		}
	}
}

void CCascadedGame :: EventPlayerPongToHost( CGamePlayer *player, uint32_t pong )
{
	if( !player->GetReserved( ) && player->GetNumPings( ) >= 3 && player->GetPing( m_GHost->m_LCPings ) > m_GHost->m_AutoKickPing )
	{
		player->SetDeleteMe( true );
		player->SetLeftReason( "was autokicked for excessive ping of " + UTIL_ToString( player->GetPing( m_GHost->m_LCPings ) ) );
		player->SetLeftCode( PLAYERLEAVE_LOBBY );
	}
}

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
#include "game_staging.h"

#include <string.h>

#include <boost/filesystem.hpp>

using namespace boost :: filesystem;

//
// CStagingGame
//

CStagingGame :: CStagingGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nPassword ) : CBaseGame( nGHost, nMap, nSaveGame, nHostPort, nGameState, nGameName, string( ), string( ), string( ) )
{
        m_Password = nPassword;
        m_MuteLobby = true;
        m_VirtualSlots = true;
}

CStagingGame :: ~CStagingGame( )
{

}

bool CStagingGame :: Update( void *fd, void *send_fd )
{
	// reset the last reserved seen timer since the admin game should never be considered abandoned

	m_LastReservedSeen = GetTime( );
	
	// upgrade any players who are finished in the staging area
	// to the current game
	// here we also delete old players
	
	for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); )
	{
		if( ( (*i)->GetSpoofed( ) || !m_GHost->m_RequireSpoofChecks ) && (*i)->GetDownloadFinished( ) && ( (*i)->GetLoggedIn( ) || m_Password.empty( ) ) )
		{
			// remove the player from the current game
			// and then add the player to the other game (or create new game)
		
			if( m_FakePlayerPID != 255 )
				(*i)->Send( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( m_FakePlayerPID, PLAYERLEAVE_LOBBY ) );
		
			if( m_VirtualHostPID != 255 )
				(*i)->Send( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( m_VirtualHostPID, PLAYERLEAVE_LOBBY ) );
		
			// transfer the socket and joinPlayer to the new player
			CTCPSocket *playerSocket = (*i)->GetSocket( );
			CIncomingJoinPlayer *joinPlayer = (*i)->GetJoinPlayer( );
			(*i)->SetSocket( NULL );
			(*i)->SetJoinPlayer( NULL );

			joinPlayer->SetTransferJoin( true );
			joinPlayer->SetTransferPID( (*i)->GetPID( ) );
			
			TransferPlayer transferPlayer;
			transferPlayer.joinPlayer = joinPlayer;
			transferPlayer.playerSocket = playerSocket;
			
			boost::mutex::scoped_lock lock( m_GHost->m_TransferMutex );
			m_GHost->m_TransferPlayers.push_back( transferPlayer );
			lock.unlock( );
			
			delete (*i);
			i = m_Players.erase( i );
		}
		
		else if( GetTime( ) - (*i)->GetJoinTime( ) > 180000 || ( GetTime( ) - (*i)->GetJoinTime( ) > 20000 && (*i)->GetDownloadFinished( ) ) )
		{
			(*i)->SetDeleteMe( true );
			(*i)->SetLeftReason( "Allowable staging area time expired." );
			(*i)->SetLeftCode( PLAYERLEAVE_LOBBY );
			++i;
		}
		
		else
			++i;
	}
	
	return CBaseGame :: Update( fd, send_fd );
}

void CStagingGame :: EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer )
{
	uint32_t Time = GetTime( );

	for( vector<TempBan> :: iterator i = m_TempBans.begin( ); i != m_TempBans.end( ); )
	{
		// remove old tempbans (after 5 seconds)

		if( Time - (*i).second >= 5 )
			i = m_TempBans.erase( i );
		else
		{
			if( (*i).first == potential->GetExternalIPString( ) )
			{
				// tempbanned, goodbye

				potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_WRONGPASSWORD ) );
				potential->SetDeleteMe( true );
				CONSOLE_Print( "[STAGINGGAME] player [" + joinPlayer->GetName( ) + "] at ip [" + (*i).first + "] is trying to join the game but is tempbanned" );
				return;
			}

                        ++i;
		}
	}

	CBaseGame :: EventPlayerJoined( potential, joinPlayer );
}

void CStagingGame :: SendWelcomeMessage( CGamePlayer *player )
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
		SendChat( player, " " );
		SendChat( player, "GHost Staging Area                        http://www.codelain.com/" );
		SendChat( player, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" );
		SendChat( player, "   Please wait while pre-game checks are performed." );
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

                        ++Count;
		}

		in.close( );
	}
}

bool CStagingGame :: EventPlayerBotCommand( CGamePlayer *player, string command, string payload )
{
	CBaseGame :: EventPlayerBotCommand( player, command, payload );

	// todotodo: don't be lazy

	string User = player->GetName( );
	string Command = command;
	string Payload = payload;
	
	CONSOLE_Print( "[STAGINGGAME] user [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );

	/*********************
	* NON ADMIN COMMANDS *
	*********************/

	//
	// !PASSWORD
	//

	if( Command == "password" && !player->GetLoggedIn( ) && !m_Password.empty( ) )
	{
		if( !m_Password.empty( ) && Payload == m_Password )
		{
			CONSOLE_Print( "[STAGINGGAME] user [" + User + "] logged in" );
			SendChat( player, "You are now logged in." );
			player->SetLoggedIn( true );
		}
		else
		{
			uint32_t LoginAttempts = player->GetLoginAttempts( ) + 1;
			player->SetLoginAttempts( LoginAttempts );
			CONSOLE_Print( "[STAGINGGAME] user [" + User + "] login attempt failed" );
			SendChat( player, "Login attempt failed." );

			if( LoginAttempts >= 1 )
			{
				player->SetDeleteMe( true );
				player->SetLeftReason( "was kicked for too many failed login attempts" );
				player->SetLeftCode( PLAYERLEAVE_LOBBY );
				OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );

				// tempban for 5 seconds to prevent bruteforcing

				m_TempBans.push_back( TempBan( player->GetExternalIPString( ), GetTime( ) ) );
			}
		}
	}

	// always hide chat commands from other players in the admin game
	// note: this is actually redundant because we've already set m_MuteLobby = true so this has no effect
	// if you actually wanted to relay chat commands you would have to set m_MuteLobby = false AND return false here

	return true;
}

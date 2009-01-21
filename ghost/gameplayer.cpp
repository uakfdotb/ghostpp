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
#include "language.h"
#include "socket.h"
#include "commandpacket.h"
#include "bnet.h"
#include "map.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game.h"

//
// CPotentialPlayer
//

CPotentialPlayer :: CPotentialPlayer( CGameProtocol *nProtocol, CBaseGame *nGame, CTCPSocket *nSocket )
{
	m_Protocol = nProtocol;
	m_Game = nGame;
	m_Socket = nSocket;
	m_DeleteMe = false;
	m_Error = false;
}

CPotentialPlayer :: ~CPotentialPlayer( )
{
	if( m_Socket )
		delete m_Socket;

	while( !m_Packets.empty( ) )
	{
		delete m_Packets.front( );
		m_Packets.pop( );
	}
}

BYTEARRAY CPotentialPlayer :: GetExternalIP( )
{
	unsigned char Zeros[] = { 0, 0, 0, 0 };

	if( m_Socket )
		return m_Socket->GetIP( );

	return UTIL_CreateByteArray( Zeros, 4 );
}

string CPotentialPlayer :: GetExternalIPString( )
{
	if( m_Socket )
		return m_Socket->GetIPString( );

	return string( );
}

bool CPotentialPlayer :: Update( void *fd )
{
	if( m_DeleteMe )
		return true;

	if( !m_Socket )
		return false;

	m_Socket->DoRecv( (fd_set *)fd );
	ExtractPackets( );
	ProcessPackets( );

	// don't call DoSend here because some other players may not have updated yet and may generate a packet for this player
	// also m_Socket may have been set to NULL during ProcessPackets but we're banking on the fact that m_DeleteMe has been set to true as well so it'll short circuit before dereferencing

	return m_DeleteMe || m_Error || m_Socket->HasError( ) || !m_Socket->GetConnected( );
}

void CPotentialPlayer :: ExtractPackets( )
{
	if( !m_Socket )
		return;

	// extract as many packets as possible from the socket's receive buffer and put them in the m_Packets queue

	string *RecvBuffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( Bytes.size( ) >= 4 )
	{
		// byte 0 is always 247

		if( Bytes[0] == W3GS_HEADER_CONSTANT )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if( Bytes.size( ) >= Length )
			{
				m_Packets.push( new CCommandPacket( W3GS_HEADER_CONSTANT, Bytes[1], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ) ) );
				*RecvBuffer = RecvBuffer->substr( Length );
				Bytes  = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
			}
			else
				return;
		}
		else
		{
			m_Error = true;
			m_ErrorString = "received invalid packet from player";
			return;
		}
	}
}

void CPotentialPlayer :: ProcessPackets( )
{
	if( !m_Socket )
		return;

	CIncomingJoinPlayer *JoinPlayer = NULL;

	// process all the received packets in the m_Packets queue

	while( !m_Packets.empty( ) )
	{
		CCommandPacket *Packet = m_Packets.front( );
		m_Packets.pop( );

		if( Packet->GetPacketType( ) == W3GS_HEADER_CONSTANT )
		{
			// the only packet we care about as a potential player is W3GS_REQJOIN, ignore everything else

			switch( Packet->GetID( ) )
			{
			case CGameProtocol :: W3GS_REQJOIN:
				JoinPlayer = m_Protocol->RECEIVE_W3GS_REQJOIN( Packet->GetData( ) );

				if( JoinPlayer )
					m_Game->EventPlayerJoined( this, JoinPlayer );

				delete JoinPlayer;
				JoinPlayer = NULL;

				// don't continue looping because there may be more packets waiting and this parent class doesn't handle them
				// EventPlayerJoined creates the new player, NULLs the socket, and sets the delete flag on this object so it'll be deleted shortly
				// any unprocessed packets will be copied to the new CGamePlayer in the constructor or discarded if we get deleted because the game is full

				delete Packet;
				return;
			}
		}

		delete Packet;
	}
}

//
// CGamePlayer
//

CGamePlayer :: CGamePlayer( CGameProtocol *nProtocol, CBaseGame *nGame, CTCPSocket *nSocket, unsigned char nPID, string nName, BYTEARRAY nInternalIP, bool nReserved ) : CPotentialPlayer( nProtocol, nGame, nSocket )
{
	m_PID = nPID;
	m_Name = nName;
	m_InternalIP = nInternalIP;
	m_LeftCode = PLAYERLEAVE_LOBBY;
	m_LoginAttempts = 0;
	m_SyncCounter = 0;
	m_JoinTime = GetTime( );
	m_LastMapPartSent = 0;
	m_StartedDownloadingTicks = 0;
	m_FinishedLoadingTicks = 0;
	m_StartedLaggingTicks = 0;
	m_StatsSentTime = 0;
	m_StatsDotASentTime = 0;
	m_LoggedIn = false;
	m_Spoofed = false;
	m_Reserved = nReserved;
	m_WhoisSent = false;
	m_DownloadAllowed = false;
	m_DownloadStarted = false;
	m_DownloadFinished = false;
	m_FinishedLoading = false;
	m_Lagging = false;
	m_DropVote = false;
	m_Muted = false;
	m_LeftMessageSent = false;
}

CGamePlayer :: CGamePlayer( CPotentialPlayer *potential, unsigned char nPID, string nName, BYTEARRAY nInternalIP, bool nReserved ) : CPotentialPlayer( potential->m_Protocol, potential->m_Game, potential->GetSocket( ) )
{
	m_Packets = potential->GetPackets( );
	m_PID = nPID;
	m_Name = nName;
	m_InternalIP = nInternalIP;
	m_LeftCode = PLAYERLEAVE_LOBBY;
	m_LoginAttempts = 0;
	m_SyncCounter = 0;
	m_JoinTime = GetTime( );
	m_LastMapPartSent = 0;
	m_StartedDownloadingTicks = 0;
	m_FinishedLoadingTicks = 0;
	m_StartedLaggingTicks = 0;
	m_StatsSentTime = 0;
	m_StatsDotASentTime = 0;
	m_LoggedIn = false;
	m_Spoofed = false;
	m_Reserved = nReserved;
	m_WhoisSent = false;
	m_DownloadAllowed = false;
	m_DownloadStarted = false;
	m_DownloadFinished = false;
	m_FinishedLoading = false;
	m_Lagging = false;
	m_DropVote = false;
	m_LeftMessageSent = false;
}

CGamePlayer :: ~CGamePlayer( )
{

}

uint32_t CGamePlayer :: GetPing( bool LCPing )
{
	// just average all the pings in the vector, nothing fancy

	if( m_Pings.empty( ) )
		return 0;

	uint32_t AvgPing = 0;

	for( unsigned int i = 0; i < m_Pings.size( ); i++ )
		AvgPing += m_Pings[i];

	AvgPing /= m_Pings.size( );

	if( LCPing )
		return AvgPing / 2;
	else
		return AvgPing;
}

bool CGamePlayer :: Update( void *fd )
{
	// wait 3 seconds after joining before sending the /whois or /w
	// if we send the /whois too early battle.net may not have caught up with where the player is and return erroneous results
	// when connecting to multiple realms we send a /whois or /w on every realm

	if( m_Game->m_GHost->m_SpoofChecks && !m_WhoisSent && GetTime( ) > m_JoinTime + 3 )
	{
		// todotodo: we could get kicked from battle.net for sending a command with invalid characters, do some basic checking

		for( vector<CBNET *> :: iterator i = m_Game->m_GHost->m_BNETs.begin( ); i != m_Game->m_GHost->m_BNETs.end( ); i++ )
		{
			if( m_Game->GetGameState( ) == GAME_PUBLIC )
				(*i)->QueueChatCommand( "/whois " + m_Name );
			else if( m_Game->GetGameState( ) == GAME_PRIVATE )
				(*i)->QueueChatCommand( m_Game->m_GHost->m_Language->SpoofCheckByReplying( ), m_Name, true );
		}

		m_WhoisSent = true;
	}

	// check for socket timeouts
	// if we don't receive anything from a player for 30 seconds we can assume they've dropped
	// this works because in the lobby we send pings every 5 seconds and expect a response to each one
	// and in the game the Warcraft 3 client sends keepalives frequently (at least once per second it looks like)

	if( m_Socket && GetTime( ) > m_Socket->GetLastRecv( ) + 30 )
		m_Game->EventPlayerDisconnectTimedOut( this );

	// base class update

	bool Deleting = CPotentialPlayer :: Update( fd );

	if( Deleting )
	{
		// try to find out why we're requesting deletion
		// in cases other than the ones covered here m_LeftReason should have been set when m_DeleteMe was set

		if( m_Error )
			m_Game->EventPlayerDisconnectPlayerError( this );

		if( m_Socket )
		{
			if( m_Socket->HasError( ) )
				m_Game->EventPlayerDisconnectSocketError( this );

			if( !m_Socket->GetConnected( ) )
				m_Game->EventPlayerDisconnectConnectionClosed( this );
		}
	}

	return Deleting;
}

void CGamePlayer :: ProcessPackets( )
{
	if( !m_Socket )
		return;

	CIncomingAction *Action = NULL;
	CIncomingChatPlayer *ChatPlayer = NULL;
	CIncomingMapSize *MapSize = NULL;
	bool HasMap = false;
	uint32_t CheckSum = 0;
	uint32_t Pong = 0;

	// process all the received packets in the m_Packets queue

	while( !m_Packets.empty( ) )
	{
		CCommandPacket *Packet = m_Packets.front( );
		m_Packets.pop( );

		if( Packet->GetPacketType( ) == W3GS_HEADER_CONSTANT )
		{
			switch( Packet->GetID( ) )
			{
			case CGameProtocol :: W3GS_LEAVEGAME:
				if( m_Protocol->RECEIVE_W3GS_LEAVEGAME( Packet->GetData( ) ) )
					m_Game->EventPlayerLeft( this );

				break;

			case CGameProtocol :: W3GS_GAMELOADED_SELF:
				if( m_Protocol->RECEIVE_W3GS_GAMELOADED_SELF( Packet->GetData( ) ) )
				{
					m_FinishedLoading = true;
					m_FinishedLoadingTicks = GetTicks( );
					m_Game->EventPlayerLoaded( this );
				}

				break;

			case CGameProtocol :: W3GS_OUTGOING_ACTION:
				Action = m_Protocol->RECEIVE_W3GS_OUTGOING_ACTION( Packet->GetData( ), m_PID );

				if( Action )
					m_Game->EventPlayerAction( this, Action );

				// don't delete Action here because the game is going to store it in a queue and delete it later

				break;

			case CGameProtocol :: W3GS_OUTGOING_KEEPALIVE:
				CheckSum = m_Protocol->RECEIVE_W3GS_OUTGOING_KEEPALIVE( Packet->GetData( ) );
				m_CheckSums.push( CheckSum );
				m_SyncCounter++;
				m_Game->EventPlayerKeepAlive( this, CheckSum );
				break;

			case CGameProtocol :: W3GS_CHAT_TO_HOST:
				ChatPlayer = m_Protocol->RECEIVE_W3GS_CHAT_TO_HOST( Packet->GetData( ) );

				if( ChatPlayer )
					m_Game->EventPlayerChatToHost( this, ChatPlayer );

				delete ChatPlayer;
				ChatPlayer = NULL;
				break;

			case CGameProtocol :: W3GS_DROPREQ:
				// todotodo: no idea what's in this packet

				if( !m_DropVote )
				{
					m_DropVote = true;
					m_Game->EventPlayerDropRequest( this );
				}

				break;

			case CGameProtocol :: W3GS_MAPSIZE:
				MapSize = m_Protocol->RECEIVE_W3GS_MAPSIZE( Packet->GetData( ), m_Game->m_GHost->m_Map->GetMapSize( ) );

				if( MapSize )
					m_Game->EventPlayerMapSize( this, MapSize );

				delete MapSize;
				MapSize = NULL;
				break;

			case CGameProtocol :: W3GS_PONG_TO_HOST:
				Pong = m_Protocol->RECEIVE_W3GS_PONG_TO_HOST( Packet->GetData( ) );

				// we discard pong values of 1
				// the client sends one of these when connecting plus we return 1 on error to kill two birds with one stone

				if( Pong != 1 )
				{
					// we also discard pong values when we're downloading because they're almost certainly inaccurate

					if( !m_DownloadStarted || m_DownloadFinished )
					{
						// we also discard pong values when anyone else is downloading if we're configured to

						if( m_Game->m_GHost->m_PingDuringDownloads || !m_Game->IsDownloading( ) )
						{
							m_Pings.push_back( GetTicks( ) - Pong );

							if( m_Pings.size( ) > 20 )
								m_Pings.erase( m_Pings.begin( ) );
						}
					}
				}

				m_Game->EventPlayerPongToHost( this, Pong );
				break;
			}
		}

		delete Packet;
	}
}

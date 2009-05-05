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
#include "socket.h"
#include "commandpacket.h"
#include "bnlsprotocol.h"
#include "bnlsclient.h"

//
// CBNLSClient
//

CBNLSClient :: CBNLSClient( string nServer, uint16_t nPort, uint32_t nWardenCookie )
{
	m_Socket = new CTCPClient( );
	m_Protocol = new CBNLSProtocol( );
	m_WasConnected = false;
	m_Server = nServer;
	m_Port = nPort;
	m_LastNullTime = 0;
	m_WardenCookie = nWardenCookie;
	m_TotalWardenIn = 0;
	m_TotalWardenOut = 0;
}

CBNLSClient :: ~CBNLSClient( )
{
	delete m_Socket;
	delete m_Protocol;

	while( !m_Packets.empty( ) )
	{
		delete m_Packets.front( );
		m_Packets.pop( );
	}
}

BYTEARRAY CBNLSClient :: GetWardenResponse( )
{
	BYTEARRAY WardenResponse;

	if( !m_WardenResponses.empty( ) )
	{
		WardenResponse = m_WardenResponses.front( );
		m_WardenResponses.pop( );
		m_TotalWardenOut++;
	}

	return WardenResponse;
}

unsigned int CBNLSClient :: SetFD( void *fd, int *nfds )
{
	if( !m_Socket->HasError( ) && m_Socket->GetConnected( ) )
	{
		m_Socket->SetFD( (fd_set *)fd, nfds );
		return 1;
	}

	return 0;
}

bool CBNLSClient :: Update( void *fd )
{
	if( m_Socket->HasError( ) )
	{
		CONSOLE_Print( "[BNLSC: %s:%d:C%d] disconnected from BNLS server due to socket error", m_Server.c_str(), m_Port, m_WardenCookie );
		return true;
	}

	if( !m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && m_WasConnected )
	{
		CONSOLE_Print( "[BNLSC: %s:%d:C%d] disconnected from BNLS server due to socket not connected", m_Server.c_str(), m_Port, m_WardenCookie );
		return true;
	}

	if( m_Socket->GetConnected( ) )
	{
		m_Socket->DoRecv( (fd_set *)fd );
		ExtractPackets( );
		ProcessPackets( );

		if( GetTime( ) >= m_LastNullTime + 50 )
		{
			m_Socket->PutBytes( m_Protocol->SEND_BNLS_NULL( ) );
			m_LastNullTime = GetTime( );
		}

		while( !m_OutPackets.empty( ) )
		{
			m_Socket->PutBytes( m_OutPackets.front( ) );
			m_OutPackets.pop( );
		}

		m_Socket->DoSend( );
		return false;
	}

	if( m_Socket->GetConnecting( ) && m_Socket->CheckConnect( ) )
	{
		m_Port = 1;
		CONSOLE_Print( "[BNLSC: %s:%d:C%d] connected", m_Server.c_str(), m_Port, m_WardenCookie );
		m_WasConnected = true;
		m_LastNullTime = GetTime( );
		return false;
	}

	if( !m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && !m_WasConnected )
	{
		CONSOLE_Print( "[BNLSC: %s:%d:C%d] connecting to server [%s] on port %d", m_Server.c_str(), m_Port, m_WardenCookie, m_Server.c_str(), m_Port );
		m_Socket->Connect( string( ), m_Server, m_Port );
		return false;
	}

	return false;
}

void CBNLSClient :: ExtractPackets( )
{
	string *RecvBuffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	while( Bytes.size( ) >= 3 )
	{
		uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false );

		if( Length >= 3 )
		{
			if( Bytes.size( ) >= Length )
			{
				m_Packets.push( new CCommandPacket( 0, Bytes[2], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ) ) );
				*RecvBuffer = RecvBuffer->substr( Length );
				Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
			}
			else
				return;
		}
		else
		{
			CONSOLE_Print( "[BNLSC: %s:%d:C%d] error - received invalid packet from BNLS server (bad length), disconnecting", m_Server.c_str(), m_Port, m_WardenCookie );
			m_Socket->Disconnect( );
			return;
		}
	}
}

void CBNLSClient :: ProcessPackets( )
{
	while( !m_Packets.empty( ) )
	{
		CCommandPacket *Packet = m_Packets.front( );
		m_Packets.pop( );

		if( Packet->GetID( ) == CBNLSProtocol :: BNLS_WARDEN )
		{
			BYTEARRAY WardenResponse = m_Protocol->RECEIVE_BNLS_WARDEN( Packet->GetData( ) );

			if( !WardenResponse.empty( ) )
				m_WardenResponses.push( WardenResponse );
		}

		delete Packet;
	}
}

void CBNLSClient :: QueueWardenSeed( uint32_t seed )
{
	m_OutPackets.push( m_Protocol->SEND_BNLS_WARDEN_SEED( m_WardenCookie, seed ) );
}

void CBNLSClient :: QueueWardenRaw( BYTEARRAY wardenRaw )
{
	m_OutPackets.push( m_Protocol->SEND_BNLS_WARDEN_RAW( m_WardenCookie, wardenRaw ) );
	m_TotalWardenIn++;
}

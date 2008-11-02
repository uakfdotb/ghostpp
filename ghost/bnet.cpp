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
#include "commandpacket.h"
#include "ghostdb.h"
#include "bncsutilinterface.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include "map.h"
#include "gameprotocol.h"
#include "game.h"

//
// CBNET
//

CBNET :: CBNET( CGHost *nGHost, string nServer, string nCDKeyROC, string nCDKeyTFT, string nUserName, string nUserPassword, string nFirstChannel, string nRootAdmin, char nCommandTrigger, bool nHoldFriends, bool nHoldClan, unsigned char nWar3Version, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType )
{
	// todotodo: append path seperator to Warcraft3Path if needed

	m_GHost = nGHost;
	m_Socket = new CTCPClient( );
	m_Protocol = new CBNETProtocol( );
	m_BNCSUtil = new CBNCSUtilInterface( nUserName, nUserPassword );
	m_Exiting = false;
	m_Server = nServer;
	m_CDKeyROC = nCDKeyROC;
	m_CDKeyTFT = nCDKeyTFT;
	transform( m_CDKeyROC.begin( ), m_CDKeyROC.end( ), m_CDKeyROC.begin( ), (int(*)(int))toupper );
	transform( m_CDKeyTFT.begin( ), m_CDKeyTFT.end( ), m_CDKeyTFT.begin( ), (int(*)(int))toupper );
	m_UserName = nUserName;
	m_UserPassword = nUserPassword;
	m_FirstChannel = nFirstChannel;
	m_RootAdmin = nRootAdmin;
	transform( m_RootAdmin.begin( ), m_RootAdmin.end( ), m_RootAdmin.begin( ), (int(*)(int))tolower );
	m_CommandTrigger = nCommandTrigger;
	m_War3Version = nWar3Version;
	m_EXEVersion = nEXEVersion;
	m_EXEVersionHash = nEXEVersionHash;
	m_PasswordHashType = nPasswordHashType;
	m_NextConnectTime = GetTime( );
	m_LastNullTime = 0;
	m_LastChatCommandTime = 0;
	m_AutoHostMaximumGames = 0;
	m_AutoHostAutoStartPlayers = 0;
	m_LastAutoHostTime = 0;
	m_WaitingToConnect = true;
	m_LoggedIn = false;
	m_InChat = false;
	m_HoldFriends = nHoldFriends;
	m_HoldClan = nHoldClan;
}

CBNET :: ~CBNET( )
{
	delete m_Socket;
	delete m_Protocol;

	while( !m_Packets.empty( ) )
	{
		delete m_Packets.front( );
		m_Packets.pop( );
	}

	delete m_BNCSUtil;

	for( vector<CIncomingFriendList *> :: iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
		delete *i;

	for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
		delete *i;
}

BYTEARRAY CBNET :: GetUniqueName( )
{
	return m_Protocol->GetUniqueName( );
}

unsigned int CBNET :: SetFD( void *fd, int *nfds )
{
	if( !m_Socket->HasError( ) && m_Socket->GetConnected( ) )
	{
		m_Socket->SetFD( (fd_set *)fd, nfds );
		return 1;
	}

	return 0;
}

bool CBNET :: Update( void *fd )
{
	// we return at the end of each if statement so we don't have to deal with errors related to the order of the if statements
	// that means it might take a few ms longer to complete a task involving multiple steps (in this case, reconnecting) due to blocking or sleeping
	// but it's not a big deal at all, maybe 100ms in the worst possible case (based on a 50ms blocking time)

	if( m_Socket->HasError( ) )
	{
		// the socket has an error

		CONSOLE_Print( "[BNET: " + m_Server + "] disconnected from battle.net due to socket error" );
		CONSOLE_Print( "[BNET: " + m_Server + "] waiting 30 seconds to reconnect" );
		m_GHost->EventBNETDisconnected( this );
		m_BNCSUtil->Reset( m_UserName, m_UserPassword );
		m_Socket->Reset( );
		m_NextConnectTime = GetTime( ) + 30;
		m_LoggedIn = false;
		m_InChat = false;
		m_WaitingToConnect = true;
		return m_Exiting;
	}

	if( !m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && !m_WaitingToConnect )
	{
		// the socket was disconnected

		CONSOLE_Print( "[BNET: " + m_Server + "] disconnected from battle.net due to socket not connected" );
		CONSOLE_Print( "[BNET: " + m_Server + "] waiting 30 seconds to reconnect" );
		m_GHost->EventBNETDisconnected( this );
		m_BNCSUtil->Reset( m_UserName, m_UserPassword );
		m_Socket->Reset( );
		m_NextConnectTime = GetTime( ) + 30;
		m_LoggedIn = false;
		m_InChat = false;
		m_WaitingToConnect = true;
		return m_Exiting;
	}

	if( m_Socket->GetConnected( ) )
	{
		// the socket is connected and everything appears to be working properly

		m_Socket->DoRecv( (fd_set *)fd );
		ExtractPackets( );
		ProcessPackets( );

		// check if at least one chat command is waiting to be sent and if we've waited long enough to prevent flooding
		// the original VB source used a formula based on the message length but 2 seconds seems to work fine

		if( !m_ChatCommands.empty( ) && GetTime( ) >= m_LastChatCommandTime + 2 )
		{
			string ChatCommand = m_ChatCommands.front( );
			m_ChatCommands.pop( );
			CONSOLE_Print( "[LOCAL: " + m_Server + "] " + ChatCommand );
			SendChatCommand( ChatCommand );
			m_LastChatCommandTime = GetTime( );
		}

		// send null packets every 60 seconds to detect disconnects

		if( GetTime( ) >= m_LastNullTime + 60 )
		{
			m_Socket->PutBytes( m_Protocol->SEND_SID_NULL( ) );
			m_LastNullTime = GetTime( );
		}

		// auto host

		if( !m_AutoHostGameName.empty( ) && m_AutoHostMaximumGames != 0 && m_AutoHostAutoStartPlayers != 0 && GetTime( ) >= m_LastAutoHostTime + 30 )
		{
			string GameName = m_AutoHostGameName + " #" + UTIL_ToString( m_GHost->m_HostCounter );

			// copy all the checks from CGHost :: CreateGame here because we don't want to spam the chat when there's an error
			// instead we fail silently and try again soon

			if( m_GHost->m_Enabled && GameName.size( ) <= 31 && m_GHost->m_Map->GetValid( ) && !m_GHost->m_CurrentGame && m_GHost->m_Games.size( ) < m_GHost->m_MaxGames && m_GHost->m_Games.size( ) < m_AutoHostMaximumGames )
			{
				m_GHost->CreateGame( GAME_PUBLIC, GameName, m_RootAdmin, m_RootAdmin, m_Server, false );

				if( m_GHost->m_CurrentGame )
					m_GHost->m_CurrentGame->SetAutoStartPlayers( m_AutoHostAutoStartPlayers );
			}

			m_LastAutoHostTime = GetTime( );
		}

		m_Socket->DoSend( );
		return m_Exiting;
	}

	if( m_Socket->GetConnecting( ) )
	{
		// we are currently attempting to connect to battle.net

		if( m_Socket->CheckConnect( ) )
		{
			// the connection attempt completed

			CONSOLE_Print( "[BNET: " + m_Server + "] connected" );
			m_GHost->EventBNETConnected( this );
			m_Socket->PutBytes( m_Protocol->SEND_PROTOCOL_INITIALIZE_SELECTOR( ) );
			m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_INFO( m_War3Version ) );
			m_Socket->DoSend( );
			m_LastNullTime = GetTime( );
			m_LastChatCommandTime = GetTime( );
			return m_Exiting;
		}
		else if( GetTime( ) >= m_NextConnectTime + 15 )
		{
			// the connection attempt timed out (15 seconds)

			CONSOLE_Print( "[BNET: " + m_Server + "] connect timed out" );
			CONSOLE_Print( "[BNET: " + m_Server + "] waiting 30 seconds to reconnect" );
			m_GHost->EventBNETConnectTimedOut( this );
			m_Socket->Reset( );
			m_NextConnectTime = GetTime( ) + 30;
			m_WaitingToConnect = true;
			return m_Exiting;
		}
	}

	if( !m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && GetTime( ) >= m_NextConnectTime )
	{
		// attempt to connect to battle.net

		CONSOLE_Print( "[BNET: " + m_Server + "] connecting to server [" + m_Server + "] on port 6112" );
		m_GHost->EventBNETConnecting( this );
		m_Socket->Connect( m_Server, 6112 );
		m_WaitingToConnect = false;
		return m_Exiting;
	}

	return m_Exiting;
}

void CBNET :: ExtractPackets( )
{
	// extract as many packets as possible from the socket's receive buffer and put them in the m_Packets queue

	string *RecvBuffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( Bytes.size( ) >= 4 )
	{
		// byte 0 is always 255

		if( Bytes[0] == BNET_HEADER_CONSTANT )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if( Bytes.size( ) >= Length )
			{
				m_Packets.push( new CCommandPacket( BNET_HEADER_CONSTANT, Bytes[1], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ) ) );
				*RecvBuffer = RecvBuffer->substr( Length );
				Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
			}
			else
				return;
		}
		else
		{
			CONSOLE_Print( "[BNET: " + m_Server + "] error - received invalid packet from battle.net, disconnecting" );
			m_Socket->Disconnect( );
			return;
		}
	}
}

void CBNET :: ProcessPackets( )
{
	CIncomingGameHost *GameHost = NULL;
	CIncomingChatEvent *ChatEvent = NULL;
	vector<CIncomingFriendList *> Friends;
	vector<CIncomingClanList *> Clans;

	// process all the received packets in the m_Packets queue
	// this normally means sending some kind of response

	while( !m_Packets.empty( ) )
	{
		CCommandPacket *Packet = m_Packets.front( );
		m_Packets.pop( );

		if( Packet->GetPacketType( ) == BNET_HEADER_CONSTANT )
		{
			switch( Packet->GetID( ) )
			{
			case CBNETProtocol :: SID_NULL:
				if( m_Protocol->RECEIVE_SID_NULL( Packet->GetData( ) ) )
					m_Socket->PutBytes( m_Protocol->SEND_SID_NULL( ) );

				break;

			case CBNETProtocol :: SID_GETADVLISTEX:
				GameHost = m_Protocol->RECEIVE_SID_GETADVLISTEX( Packet->GetData( ) );

				if( GameHost )
					CONSOLE_Print( "[BNET: " + m_Server + "] joining game [" + GameHost->GetGameName( ) + "]" );

				delete GameHost;
				GameHost = NULL;

				break;

			case CBNETProtocol :: SID_ENTERCHAT:
				if( m_Protocol->RECEIVE_SID_ENTERCHAT( Packet->GetData( ) ) )
				{
					CONSOLE_Print( "[BNET: " + m_Server + "] joining channel [" + m_FirstChannel + "]" );
					m_InChat = true;
					m_Socket->PutBytes( m_Protocol->SEND_SID_JOINCHANNEL( m_FirstChannel ) );
				}

				break;

			case CBNETProtocol :: SID_CHATEVENT:
				ChatEvent = m_Protocol->RECEIVE_SID_CHATEVENT( Packet->GetData( ) );

				if( ChatEvent )
					ProcessChatEvent( ChatEvent );

				delete ChatEvent;
				ChatEvent = NULL;

				break;

			case CBNETProtocol :: SID_CHECKAD:
				m_Protocol->RECEIVE_SID_CHECKAD( Packet->GetData( ) );
				break;

			case CBNETProtocol :: SID_STARTADVEX3:
				if( m_Protocol->RECEIVE_SID_STARTADVEX3( Packet->GetData( ) ) )
				{
					m_InChat = false;
					m_GHost->EventBNETGameRefreshed( this );
				}
				else
				{
					CONSOLE_Print( "[BNET: " + m_Server + "] startadvex3 failed" );
					m_GHost->EventBNETGameRefreshFailed( this );
				}

				break;

			case CBNETProtocol :: SID_PING:
				m_Socket->PutBytes( m_Protocol->SEND_SID_PING( m_Protocol->RECEIVE_SID_PING( Packet->GetData( ) ) ) );
				break;

			case CBNETProtocol :: SID_AUTH_INFO:
				if( m_Protocol->RECEIVE_SID_AUTH_INFO( Packet->GetData( ) ) )
				{
					if( m_BNCSUtil->HELP_SID_AUTH_CHECK( m_GHost->m_Warcraft3Path, m_CDKeyROC, m_CDKeyTFT, m_Protocol->GetValueStringFormulaString( ), m_Protocol->GetIX86VerFileNameString( ), m_Protocol->GetClientToken( ), m_Protocol->GetServerToken( ) ) )
					{
						// override the exe information generated by bncsutil if specified in the config file
						// apparently this is useful for pvpgn users

						if( m_EXEVersion.size( ) == 4 )
						{
							CONSOLE_Print( "[BNET: " + m_Server + "] using custom exe version bnet_custom_exeversion = " + UTIL_ToString( m_EXEVersion[0] ) + " " + UTIL_ToString( m_EXEVersion[1] ) + " " + UTIL_ToString( m_EXEVersion[2] ) + " " + UTIL_ToString( m_EXEVersion[3] ) );
							m_BNCSUtil->SetEXEVersion( m_EXEVersion );
						}

						if( m_EXEVersionHash.size( ) == 4 )
						{
							CONSOLE_Print( "[BNET: " + m_Server + "] using custom exe version hash bnet_custom_exeversionhash = " + UTIL_ToString( m_EXEVersionHash[0] ) + " " + UTIL_ToString( m_EXEVersionHash[1] ) + " " + UTIL_ToString( m_EXEVersionHash[2] ) + " " + UTIL_ToString( m_EXEVersionHash[3] ) );
							m_BNCSUtil->SetEXEVersionHash( m_EXEVersionHash );
						}

						m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_CHECK( m_Protocol->GetClientToken( ), m_BNCSUtil->GetEXEVersion( ), m_BNCSUtil->GetEXEVersionHash( ), m_BNCSUtil->GetKeyInfoROC( ), m_BNCSUtil->GetKeyInfoTFT( ), m_BNCSUtil->GetEXEInfo( ), "GHost" ) );
					}
					else
					{
						CONSOLE_Print( "[BNET: " + m_Server + "] logon failed - bncsutil key hash failed (check your Warcraft 3 path and cd keys), disconnecting" );
						m_Socket->Disconnect( );
						delete Packet;
						return;
					}
				}

				break;

			case CBNETProtocol :: SID_AUTH_CHECK:
				if( m_Protocol->RECEIVE_SID_AUTH_CHECK( Packet->GetData( ) ) )
				{
					// cd keys accepted

					CONSOLE_Print( "[BNET: " + m_Server + "] cd keys accepted" );
					m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGON( );
					m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGON( m_BNCSUtil->GetClientKey( ), m_UserName ) );
				}
				else
				{
					// cd keys not accepted

					switch( UTIL_ByteArrayToUInt32( m_Protocol->GetKeyState( ), false ) )
					{
					case CBNETProtocol :: KR_ROC_KEY_IN_USE:
						CONSOLE_Print( "[BNET: " + m_Server + "] logon failed - ROC CD key in use by user [" + m_Protocol->GetKeyStateDescription( ) + "], disconnecting" );
						break;
					case CBNETProtocol :: KR_TFT_KEY_IN_USE:
						CONSOLE_Print( "[BNET: " + m_Server + "] logon failed - TFT CD key in use by user [" + m_Protocol->GetKeyStateDescription( ) + "], disconnecting" );
						break;
					case CBNETProtocol :: KR_OLD_GAME_VERSION:
						CONSOLE_Print( "[BNET: " + m_Server + "] logon failed - game version is too old, disconnecting" );
						break;
					case CBNETProtocol :: KR_INVALID_VERSION:
						CONSOLE_Print( "[BNET: " + m_Server + "] logon failed - game version is invalid, disconnecting" );
						break;
					default:
						CONSOLE_Print( "[BNET: " + m_Server + "] logon failed - cd keys not accepted, disconnecting" );
						break;
					}

					m_Socket->Disconnect( );
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol :: SID_AUTH_ACCOUNTLOGON:
				if( m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGON( Packet->GetData( ) ) )
				{
					CONSOLE_Print( "[BNET: " + m_Server + "] username [" + m_UserName + "] accepted" );

					if( m_PasswordHashType == "pvpgn" )
					{
						// pvpgn logon

						CONSOLE_Print( "[BNET: " + m_Server + "] using pvpgn logon type (for pvpgn servers only)" );
						m_BNCSUtil->HELP_PvPGNPasswordHash( m_UserPassword );
						m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF( m_BNCSUtil->GetPvPGNPasswordHash( ) ) );
					}
					else
					{
						// battle.net logon

						CONSOLE_Print( "[BNET: " + m_Server + "] using battle.net logon type (for official battle.net servers only)" );
						m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGONPROOF( m_Protocol->GetSalt( ), m_Protocol->GetServerPublicKey( ) );
						m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF( m_BNCSUtil->GetM1( ) ) );
					}
				}
				else
				{
					CONSOLE_Print( "[BNET: " + m_Server + "] logon failed - invalid username, disconnecting" );
					m_Socket->Disconnect( );
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol :: SID_AUTH_ACCOUNTLOGONPROOF:
				if( m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( Packet->GetData( ) ) )
				{
					// logon successful

					CONSOLE_Print( "[BNET: " + m_Server + "] logon successful" );
					m_LoggedIn = true;
					m_GHost->EventBNETLoggedIn( this );
					m_Socket->PutBytes( m_Protocol->SEND_SID_NETGAMEPORT( m_GHost->m_HostPort ) );
					m_Socket->PutBytes( m_Protocol->SEND_SID_ENTERCHAT( ) );
					m_Socket->PutBytes( m_Protocol->SEND_SID_FRIENDSLIST( ) );
					m_Socket->PutBytes( m_Protocol->SEND_SID_CLANMEMBERLIST( ) );
				}
				else
				{
					CONSOLE_Print( "[BNET: " + m_Server + "] logon failed - invalid password, disconnecting" );

					// try to figure out if the user might be using the wrong logon type since too many people are confused by this

					string Server = m_Server;
					transform( Server.begin( ), Server.end( ), Server.begin( ), (int(*)(int))tolower );

					if( m_PasswordHashType == "pvpgn" && ( Server == "useast.battle.net" || Server == "uswest.battle.net" || Server == "asia.battle.net" || Server == "europe.battle.net" ) )
						CONSOLE_Print( "[BNET: " + m_Server + "] it looks like you're trying to connect to a battle.net server using a pvpgn logon type, check your config file's \"battle.net custom data\" section" );
					else if( m_PasswordHashType != "pvpgn" && ( Server != "useast.battle.net" && Server != "uswest.battle.net" && Server != "asia.battle.net" && Server != "europe.battle.net" ) )
						CONSOLE_Print( "[BNET: " + m_Server + "] it looks like you're trying to connect to a pvpgn server using a battle.net logon type, check your config file's \"battle.net custom data\" section" );

					m_Socket->Disconnect( );
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol :: SID_FRIENDSLIST:
				Friends = m_Protocol->RECEIVE_SID_FRIENDSLIST( Packet->GetData( ) );

				for( vector<CIncomingFriendList *> :: iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
					delete *i;

				m_Friends = Friends;

				/* DEBUG_Print( "received " + UTIL_ToString( Friends.size( ) ) + " friends" );
				for( vector<CIncomingFriendList *> :: iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
					DEBUG_Print( "friend: " + (*i)->GetAccount( ) ); */

				break;

			case CBNETProtocol :: SID_CLANMEMBERLIST:
				vector<CIncomingClanList *> Clans = m_Protocol->RECEIVE_SID_CLANMEMBERLIST( Packet->GetData( ) );

				for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
					delete *i;

				m_Clans = Clans;

				/* DEBUG_Print( "received " + UTIL_ToString( Clans.size( ) ) + " clan members" );
				for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
					DEBUG_Print( "clan member: " + (*i)->GetName( ) ); */

				break;
			}
		}

		delete Packet;
	}
}

void CBNET :: ProcessChatEvent( CIncomingChatEvent *chatEvent )
{
	CBNETProtocol :: IncomingChatEvent Event = chatEvent->GetChatEvent( );
	bool Whisper = ( Event == CBNETProtocol :: EID_WHISPER );
	string User = chatEvent->GetUser( );
	string Message = chatEvent->GetMessage( );

	if( Event == CBNETProtocol :: EID_WHISPER || Event == CBNETProtocol :: EID_TALK )
	{
		if( Event == CBNETProtocol :: EID_WHISPER )
			CONSOLE_Print( "[WHISPER: " + m_Server + "] [" + User + "] " + Message );
		else
			CONSOLE_Print( "[LOCAL: " + m_Server + "] [" + User + "] " + Message );

		// handle spoof checking for current game
		// this case covers whispers - we assume that anyone who sends a whisper to the bot with message "spoofcheck" should be considered spoof checked
		// note that this means you can whisper "spoofcheck" even in a public game to manually spoofcheck if the /whois fails

		if( Event == CBNETProtocol :: EID_WHISPER && Message == "spoofcheck" && m_GHost->m_CurrentGame )
			m_GHost->m_CurrentGame->AddToSpoofed( m_Server, User, true );

		// handle bot commands

		if( !Message.empty( ) && Message[0] == m_CommandTrigger )
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

			if( m_GHost->m_DB->AdminCheck( m_Server, User ) || IsRootAdmin( User ) )
			{
				CONSOLE_Print( "[BNET: " + m_Server + "] admin [" + User + "] sent command [" + Message + "]" );

				/*****************
				* ADMIN COMMANDS *
				******************/

				//
				// !ADDADMIN
				//

				if( Command == "addadmin" && !Payload.empty( ) )
				{
					if( IsRootAdmin( User ) )
					{
						if( m_GHost->m_DB->AdminCheck( m_Server, Payload ) )
							QueueChatCommand( m_GHost->m_Language->UserIsAlreadyAnAdmin( m_Server, Payload ), User, Whisper );
						else
						{
							if( m_GHost->m_DB->AdminAdd( m_Server, Payload ) )
								QueueChatCommand( m_GHost->m_Language->AddedUserToAdminDatabase( m_Server, Payload ), User, Whisper );
							else
								QueueChatCommand( m_GHost->m_Language->ErrorAddingUserToAdminDatabase( m_Server, Payload ), User, Whisper );
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !ADDBAN
				// !BAN
				//

				if( Command == "addban" || Command == "ban" && !Payload.empty( ) )
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

					CDBBan *Ban = m_GHost->m_DB->BanCheck( m_Server, Victim );

					if( Ban )
					{
						QueueChatCommand( m_GHost->m_Language->UserIsAlreadyBanned( m_Server, Victim ), User, Whisper );
						delete Ban;
						Ban = NULL;
					}
					else
					{
						if( m_GHost->m_DB->BanAdd( m_Server, Victim, string( ), string( ), User, Reason ) )
							QueueChatCommand( m_GHost->m_Language->BannedUser( m_Server, Victim ), User, Whisper );
						else
							QueueChatCommand( m_GHost->m_Language->ErrorBanningUser( m_Server, Victim ), User, Whisper );
					}
				}

				//
				// !ANNOUNCE
				//

				if( Command == "announce" && m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetCountDownStarted( ) )
				{
					if( Payload.empty( ) || Payload == "off" )
					{
						QueueChatCommand( m_GHost->m_Language->AnnounceMessageDisabled( ), User, Whisper );
						m_GHost->m_CurrentGame->SetAnnounce( 0, string( ) );
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
							CONSOLE_Print( "[BNET: " + m_Server + "] bad input #1 to announce command" );
						else
						{
							if( SS.eof( ) )
								CONSOLE_Print( "[BNET: " + m_Server + "] missing input #2 to announce command" );
							else
							{
								getline( SS, Message );
								string :: size_type Start = Message.find_first_not_of( " " );

								if( Start != string :: npos )
									Message = Message.substr( Start );

								QueueChatCommand( m_GHost->m_Language->AnnounceMessageEnabled( ), User, Whisper );
								m_GHost->m_CurrentGame->SetAnnounce( Interval, Message );
							}
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
						QueueChatCommand( m_GHost->m_Language->AutoHostDisabled( ), User, Whisper );
						m_AutoHostGameName.clear( );
						m_AutoHostMaximumGames = 0;
						m_AutoHostAutoStartPlayers = 0;
						m_LastAutoHostTime = GetTime( );
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
							CONSOLE_Print( "[BNET: " + m_Server + "] bad input #1 to autohost command" );
						else
						{
							SS >> AutoStartPlayers;

							if( SS.fail( ) || AutoStartPlayers == 0 )
								CONSOLE_Print( "[BNET: " + m_Server + "] bad input #2 to autohost command" );
							else
							{
								if( SS.eof( ) )
									CONSOLE_Print( "[BNET: " + m_Server + "] missing input #3 to autohost command" );
								else
								{
									getline( SS, GameName );
									string :: size_type Start = GameName.find_first_not_of( " " );

									if( Start != string :: npos )
										GameName = GameName.substr( Start );

									QueueChatCommand( m_GHost->m_Language->AutoHostEnabled( ), User, Whisper );
									m_AutoHostGameName = GameName;
									m_AutoHostMaximumGames = MaximumGames;
									m_AutoHostAutoStartPlayers = AutoStartPlayers;
									m_LastAutoHostTime = GetTime( );
								}
							}
						}
					}
				}

				//
				// !AUTOSTART
				//

				if( Command == "autostart" && m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetCountDownStarted( ) )
				{
					if( Payload.empty( ) || Payload == "off" )
					{
						QueueChatCommand( m_GHost->m_Language->AutoStartDisabled( ), User, Whisper );
						m_GHost->m_CurrentGame->SetAutoStartPlayers( 0 );
					}
					else
					{
						uint32_t AutoStartPlayers = UTIL_ToUInt32( Payload );

						if( AutoStartPlayers != 0 )
						{
							QueueChatCommand( m_GHost->m_Language->AutoStartEnabled( UTIL_ToString( AutoStartPlayers ) ), User, Whisper );
							m_GHost->m_CurrentGame->SetAutoStartPlayers( AutoStartPlayers );
						}
					}
				}

				//
				// !CHANNEL (change channel)
				//

				if( Command == "channel" && !Payload.empty( ) )
					QueueChatCommand( "/join " + Payload );

				//
				// !CHECKADMIN
				//

				if( Command == "checkadmin" && !Payload.empty( ) )
				{
					if( IsRootAdmin( User ) )
					{
						if( m_GHost->m_DB->AdminCheck( m_Server, Payload ) )
							QueueChatCommand( m_GHost->m_Language->UserIsAnAdmin( m_Server, Payload ), User, Whisper );
						else
							QueueChatCommand( m_GHost->m_Language->UserIsNotAnAdmin( m_Server, Payload ), User, Whisper );
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !CHECKBAN
				//

				if( Command == "checkban" && !Payload.empty( ) )
				{
					CDBBan *Ban = m_GHost->m_DB->BanCheck( m_Server, Payload );

					if( Ban )
					{
						QueueChatCommand( m_GHost->m_Language->UserWasBannedOnByBecause( m_Server, Payload, Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ), User, Whisper );
						delete Ban;
						Ban = NULL;
					}
					else
						QueueChatCommand( m_GHost->m_Language->UserIsNotBanned( m_Server, Payload ), User, Whisper );
				}

				//
				// !CLOSE (close slot)
				//

				if( Command == "close" && !Payload.empty( ) && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
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
								CONSOLE_Print( "[BNET: " + m_Server + "] bad input to close command" );
								break;
							}
							else
								m_GHost->m_CurrentGame->CloseSlot( (unsigned char)( SID - 1 ), true );
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !CLOSEALL
				//

				if( Command == "closeall" && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
						m_GHost->m_CurrentGame->CloseAllSlots( );
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !COUNTADMINS
				//

				if( Command == "countadmins" )
				{
					uint32_t Count = m_GHost->m_DB->AdminCount( m_Server );

					if( Count == 0 )
						QueueChatCommand( m_GHost->m_Language->ThereAreNoAdmins( m_Server ), User, Whisper );
					else if( Count == 1 )
						QueueChatCommand( m_GHost->m_Language->ThereIsAdmin( m_Server ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->ThereAreAdmins( m_Server, UTIL_ToString( Count ) ), User, Whisper );
				}

				//
				// !COUNTBANS
				//

				if( Command == "countbans" )
				{
					uint32_t Count = m_GHost->m_DB->BanCount( m_Server );

					if( Count == 0 )
						QueueChatCommand( m_GHost->m_Language->ThereAreNoBannedUsers( m_Server ), User, Whisper );
					else if( Count == 1 )
						QueueChatCommand( m_GHost->m_Language->ThereIsBannedUser( m_Server ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->ThereAreBannedUsers( m_Server, UTIL_ToString( Count ) ), User, Whisper );
				}

				//
				// !DELADMIN
				//

				if( Command == "deladmin" && !Payload.empty( ) )
				{
					if( IsRootAdmin( User ) )
					{
						if( !m_GHost->m_DB->AdminCheck( m_Server, Payload ) )
							QueueChatCommand( m_GHost->m_Language->UserIsNotAnAdmin( m_Server, Payload ), User, Whisper );
						else
						{
							if( m_GHost->m_DB->AdminRemove( m_Server, Payload ) )
								QueueChatCommand( m_GHost->m_Language->DeletedUserFromAdminDatabase( m_Server, Payload ), User, Whisper );
							else
								QueueChatCommand( m_GHost->m_Language->ErrorDeletingUserFromAdminDatabase( m_Server, Payload ), User, Whisper );
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !DELBAN
				// !UNBAN
				//

				if( Command == "delban" || Command == "unban" && !Payload.empty( ) )
				{
					if( m_GHost->m_DB->BanRemove( Payload ) )
						QueueChatCommand( m_GHost->m_Language->UnbannedUser( Payload ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->ErrorUnbanningUser( Payload ), User, Whisper );
				}

				//
				// !DISABLE
				//

				if( Command == "disable" )
				{
					if( IsRootAdmin( User ) )
					{
						QueueChatCommand( m_GHost->m_Language->BotDisabled( ), User, Whisper );
						m_GHost->m_Enabled = false;
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !ENABLE
				//

				if( Command == "enable" )
				{
					if( IsRootAdmin( User ) )
					{
						QueueChatCommand( m_GHost->m_Language->BotEnabled( ), User, Whisper );
						m_GHost->m_Enabled = true;
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
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
						QueueChatCommand( m_GHost->m_Language->EndingGame( m_GHost->m_Games[GameNumber]->GetDescription( ) ), User, Whisper );
						CONSOLE_Print( "[GAME: " + m_GHost->m_Games[GameNumber]->GetGameName( ) + "] is over (admin ended game)" );
						m_GHost->m_Games[GameNumber]->StopPlayers( "was disconnected (admin ended game)" );
					}
					else
						QueueChatCommand( m_GHost->m_Language->GameNumberDoesntExist( Payload ), User, Whisper );
				}

				//
				// !EXIT
				// !QUIT
				//

				if( Command == "exit" || Command == "quit" )
				{
					if( IsRootAdmin( User ) )
					{
						if( Payload == "force" )
							m_Exiting = true;
						else
						{
							if( m_GHost->m_CurrentGame || !m_GHost->m_Games.empty( ) )
								QueueChatCommand( m_GHost->m_Language->AtLeastOneGameActiveUseForceToShutdown( ), User, Whisper );
							else
								m_Exiting = true;
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !GETCLAN
				//

				if( Command == "getclan" )
					SendGetClanList( );

				//
				// !GETFRIENDS
				//

				if( Command == "getfriends" )
					SendGetFriendsList( );

				//
				// !GETGAME
				//

				if( Command == "getgame" && !Payload.empty( ) )
				{
					uint32_t GameNumber = UTIL_ToUInt32( Payload ) - 1;

					if( GameNumber < m_GHost->m_Games.size( ) )
						QueueChatCommand( m_GHost->m_Language->GameNumberIs( Payload, m_GHost->m_Games[GameNumber]->GetDescription( ) ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->GameNumberDoesntExist( Payload ), User, Whisper );
				}

				//
				// !GETGAMES
				//

				if( Command == "getgames" )
				{
					if( m_GHost->m_CurrentGame )
						QueueChatCommand( m_GHost->m_Language->GameIsInTheLobby( m_GHost->m_CurrentGame->GetDescription( ), UTIL_ToString( m_GHost->m_Games.size( ) ), UTIL_ToString( m_GHost->m_MaxGames ) ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->ThereIsNoGameInTheLobby( UTIL_ToString( m_GHost->m_Games.size( ) ), UTIL_ToString( m_GHost->m_MaxGames ) ), User, Whisper );
				}

				//
				// !HOLD (hold a slot for someone)
				//

				if( Command == "hold" && !Payload.empty( ) && m_GHost->m_CurrentGame )
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
							CONSOLE_Print( "[BNET: " + m_Server + "] bad input to hold command" );
							break;
						}
						else
						{
							QueueChatCommand( m_GHost->m_Language->AddedPlayerToTheHoldList( HoldName ), User, Whisper );
							m_GHost->m_CurrentGame->AddToReserved( HoldName );
						}
					}
				}

				//
				// !LOAD (load config file)
				// !MAP
				//

				if( Command == "load" || Command == "map" )
				{
					if( Payload.empty( ) )
						QueueChatCommand( m_GHost->m_Language->CurrentlyLoadedMapCFGIs( m_GHost->m_Map->GetCFGFile( ) ), User, Whisper );
					else
					{
						// only load files in the current directory just to be safe

						if( Payload.find( "/" ) != string :: npos || Payload.find( "\\" ) != string :: npos )
							QueueChatCommand( m_GHost->m_Language->UnableToLoadConfigFilesOutside( ), User, Whisper );
						else
						{
							string File = m_GHost->m_MapCFGPath + Payload + ".cfg";

							if( UTIL_FileExists( File ) )
							{
								// we have to be careful here because we didn't copy the map data when creating the game (there's only one global copy)
								// therefore if we change the map data while a game is in the lobby everything will get screwed up
								// the easiest solution is to simply reject the command if a game is in the lobby

								if( m_GHost->m_CurrentGame )
									QueueChatCommand( m_GHost->m_Language->UnableToLoadConfigFileGameInLobby( ), User, Whisper );
								else
								{
									QueueChatCommand( m_GHost->m_Language->LoadingConfigFile( File ), User, Whisper );
									CConfig MapCFG;
									MapCFG.Read( File );
									m_GHost->m_Map->Load( &MapCFG, File );
								}
							}
							else
								QueueChatCommand( m_GHost->m_Language->UnableToLoadConfigFileDoesntExist( File ), User, Whisper );
						}
					}
				}

				//
				// !OPEN (open slot)
				//

				if( Command == "open" && !Payload.empty( ) && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
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
								CONSOLE_Print( "[BNET: " + m_Server + "] bad input to open command" );
								break;
							}
							else
								m_GHost->m_CurrentGame->OpenSlot( (unsigned char)( SID - 1 ), true );
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !OPENALL
				//

				if( Command == "openall" && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
						m_GHost->m_CurrentGame->OpenAllSlots( );
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !PRIV (host private game)
				//

				if( Command == "priv" && !Payload.empty( ) )
					m_GHost->CreateGame( GAME_PRIVATE, Payload, User, User, m_Server, Whisper );

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
						m_GHost->CreateGame( GAME_PRIVATE, GameName, Owner, User, m_Server, Whisper );
					}
				}

				//
				// !PUB (host public game)
				//

				if( Command == "pub" && !Payload.empty( ) )
					m_GHost->CreateGame( GAME_PUBLIC, Payload, User, User, m_Server, Whisper );

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
						m_GHost->CreateGame( GAME_PUBLIC, GameName, Owner, User, m_Server, Whisper );
					}
				}

				//
				// !SAY
				//

				if( Command == "say" && !Payload.empty( ) )
					QueueChatCommand( Payload );

				//
				// !SAYGAMES
				//

				if( Command == "saygames" && !Payload.empty( ) )
				{
					if( IsRootAdmin( User ) )
					{
						if( m_GHost->m_CurrentGame )
							m_GHost->m_CurrentGame->SendAllChat( Payload );

						for( vector<CBaseGame *> :: iterator i = m_GHost->m_Games.begin( ); i != m_GHost->m_Games.end( ); i++ )
							(*i)->SendAllChat( Payload );
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !SP
				//

				if( Command == "sp" && m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetCountDownStarted( ) )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
					{
						m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->ShufflingPlayers( ) );
						m_GHost->m_CurrentGame->ShuffleSlots( );
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !START
				//

				if( Command == "start" && m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetCountDownStarted( ) )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
					{
						// if the player sent "!start force" skip the checks and start the countdown
						// otherwise check that the game is ready to start

						if( Payload == "force" )
							m_GHost->m_CurrentGame->StartCountDown( true );
						else
							m_GHost->m_CurrentGame->StartCountDown( false );
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !SWAP (swap slots)
				//

				if( Command == "swap" && !Payload.empty( ) && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
					{
						uint32_t SID1;
						uint32_t SID2;
						stringstream SS;
						SS << Payload;
						SS >> SID1;

						if( SS.fail( ) )
							CONSOLE_Print( "[BNET: " + m_Server + "] bad input #1 to swap command" );
						else
						{
							if( SS.eof( ) )
								CONSOLE_Print( "[BNET: " + m_Server + "] missing input #2 to swap command" );
							else
							{
								SS >> SID2;

								if( SS.fail( ) )
									CONSOLE_Print( "[BNET: " + m_Server + "] bad input #2 to swap command" );
								else
									m_GHost->m_CurrentGame->SwapSlots( (unsigned char)( SID1 - 1 ), (unsigned char)( SID2 - 1 ) );
							}
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !UNHOST
				//

				if( Command == "unhost" )
				{
					if( m_GHost->m_CurrentGame )
					{
						if( m_GHost->m_CurrentGame->GetCountDownStarted( ) )
							QueueChatCommand( m_GHost->m_Language->UnableToUnhostGameCountdownStarted( m_GHost->m_CurrentGame->GetDescription( ) ), User, Whisper );
						else
						{
							QueueChatCommand( m_GHost->m_Language->UnhostingGame( m_GHost->m_CurrentGame->GetDescription( ) ), User, Whisper );
							m_GHost->m_CurrentGame->SetExiting( true );
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->UnableToUnhostGameNoGameInLobby( ), User, Whisper );
				}
			}
			else
				CONSOLE_Print( "[BNET: " + m_Server + "] user [" + User + "] sent command [" + Message + "]" );

			/*********************
			* NON ADMIN COMMANDS *
			*********************/

			// don't respond to non admins if there are more than 3 messages already in the queue
			// this prevents malicious users from filling up the bot's chat queue and crippling the bot
			// in some cases the queue may be full of legitimate messages but we don't really care if the bot ignores one of these commands once in awhile
			// e.g. when several users join a game at the same time and cause multiple /whois messages to be queued at once

			if( m_GHost->m_DB->AdminCheck( m_Server, User ) || IsRootAdmin( User ) || m_ChatCommands.size( ) <= 3 )
			{
				//
				// !STATS
				//

				if( Command == "stats" )
				{
					string StatsUser = User;

					if( !Payload.empty( ) )
						StatsUser = Payload;

					CDBGamePlayerSummary *GamePlayerSummary = m_GHost->m_DB->GamePlayerSummaryCheck( StatsUser );

					if( GamePlayerSummary )
					{
						QueueChatCommand( m_GHost->m_Language->HasPlayedGamesWithThisBot( StatsUser, GamePlayerSummary->GetFirstGameDateTime( ), GamePlayerSummary->GetLastGameDateTime( ), UTIL_ToString( GamePlayerSummary->GetTotalGames( ) ), UTIL_ToString( (float)GamePlayerSummary->GetAvgLoadingTime( ) / 1000, 2 ), UTIL_ToString( GamePlayerSummary->GetAvgLeftPercent( ) ) ), User, Whisper );
						delete GamePlayerSummary;
						GamePlayerSummary = NULL;
					}
					else
						QueueChatCommand( m_GHost->m_Language->HasntPlayedGamesWithThisBot( StatsUser ), User, Whisper );
				}

				//
				// !STATSDOTA
				//

				if( Command == "statsdota" )
				{
					string StatsUser = User;

					if( !Payload.empty( ) )
						StatsUser = Payload;

					CDBDotAPlayerSummary *DotAPlayerSummary = m_GHost->m_DB->DotAPlayerSummaryCheck( StatsUser );

					if( DotAPlayerSummary )
					{
						QueueChatCommand( m_GHost->m_Language->HasPlayedDotAGamesWithThisBot( StatsUser, UTIL_ToString( DotAPlayerSummary->GetTotalGames( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalWins( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalLosses( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalKills( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalDeaths( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalCreepKills( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalCreepDenies( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalAssists( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalNeutralKills( ) ) ), User, Whisper );
						delete DotAPlayerSummary;
						DotAPlayerSummary = NULL;
					}
					else
						QueueChatCommand( m_GHost->m_Language->HasntPlayedDotAGamesWithThisBot( StatsUser ) );
				}

				//
				// !VERSION
				//

				if( Command == "version" )
				{
					if( m_GHost->m_DB->AdminCheck( m_Server, User ) || IsRootAdmin( User ) )
						QueueChatCommand( m_GHost->m_Language->VersionAdmin( m_GHost->m_Version ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->VersionNotAdmin( m_GHost->m_Version ), User, Whisper );
				}
			}
		}
	}
	else if( Event == CBNETProtocol :: EID_CHANNEL )
	{
		// keep track of current channel so we can rejoin it after hosting a game

		CONSOLE_Print( "[BNET: " + m_Server + "] joined channel [" + Message + "]" );
		m_CurrentChannel = Message;
	}
	else if( Event == CBNETProtocol :: EID_INFO )
	{
		CONSOLE_Print( "[INFO: " + m_Server + "] " + Message );

		// extract the first word which we hope is the username
		// this is not necessarily true though since info messages also include channel MOTD's and such

		string UserName;
		string :: size_type Split = Message.find( " " );

		if( Split != string :: npos )
			UserName = Message.substr( 0, Split );
		else
			UserName = Message.substr( 0 );

		// handle spoof checking for current game
		// this case covers whois results which are used when hosting a public game (we send out a "/whois [player]" for each player)
		// at all times you can still /w the bot with "spoofcheck" to manually spoof check

		if( m_GHost->m_CurrentGame && m_GHost->m_CurrentGame->GetGameState( ) == GAME_PUBLIC )
		{
			// we don't need to check if the player is in the game before spamming the chat because the bot only sends out a /whois on players that join the game
			// however, we DO need to check that we're only connected to one realm because if we're connected to multiple realms we send a /whois on every realm for every player
			// so we're always guaranteed to get negative results with multiple realms - we're only interested in the positive ones

			if( m_GHost->m_BNETs.size( ) == 1 )
			{
				if( Message.find( "is away" ) != string :: npos )
					m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofPossibleIsAway( UserName ) );
				else if( Message.find( "is unavailable" ) != string :: npos )
					m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofPossibleIsUnavailable( UserName ) );
				else if( Message.find( "is refusing messages" ) != string :: npos )
					m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofPossibleIsRefusingMessages( UserName ) );
				else if( Message.find( "is using Warcraft III The Frozen Throne in the channel" ) != string :: npos )
					m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofDetectedIsNotInGame( UserName ) );
				else if( Message.find( "is using Warcraft III The Frozen Throne in channel" ) != string :: npos )
					m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofDetectedIsNotInGame( UserName ) );
				else if( Message.find( "is using Warcraft III The Frozen Throne in a private channel" ) != string :: npos )
					m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofDetectedIsInPrivateChannel( UserName ) );
			}

			if( Message.find( "is using Warcraft III The Frozen Throne in game" ) != string :: npos || Message.find( "is using Warcraft III Frozen Throne and is currently in  game" ) != string :: npos )
			{
				if( Message.find( m_GHost->m_CurrentGame->GetGameName( ) ) != string :: npos )
					m_GHost->m_CurrentGame->AddToSpoofed( m_Server, UserName, false );
				else if( m_GHost->m_BNETs.size( ) == 1 )
					m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofDetectedIsInAnotherGame( UserName ) );
			}
		}
	}
	else if( Event == CBNETProtocol :: EID_ERROR )
		CONSOLE_Print( "[ERROR: " + m_Server + "] " + Message );
}

void CBNET :: SendEnterChat( )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_ENTERCHAT( ) );
}

void CBNET :: SendJoinChannel( string channel )
{
	if( m_LoggedIn && m_InChat )
		m_Socket->PutBytes( m_Protocol->SEND_SID_JOINCHANNEL( channel ) );
}

void CBNET :: SendChatCommand( string chatCommand )
{
	// don't call this function directly, use QueueChatCommand instead to prevent getting kicked for flooding

	if( m_LoggedIn )
	{
		if( chatCommand.size( ) > 220 )
			chatCommand = chatCommand.substr( 0, 220 );

		m_Socket->PutBytes( m_Protocol->SEND_SID_CHATCOMMAND( chatCommand ) );
	}
}

void CBNET :: SendGameCreate( unsigned char state, string gameName, string hostName, CMap *map )
{
	if( hostName.empty( ) )
	{
		BYTEARRAY UniqueName = m_Protocol->GetUniqueName( );
		hostName = string( UniqueName.begin( ), UniqueName.end( ) );
	}

	if( m_LoggedIn && map )
	{
		if( !m_CurrentChannel.empty( ) )
			m_FirstChannel = m_CurrentChannel;

		// uptime seems to be seconds since game creation so we pass zero here since this is the first STARTADVEX3 packet
		// we pass an actual uptime value in SendGameRefresh

		m_InChat = false;
		m_Socket->PutBytes( m_Protocol->SEND_SID_STARTADVEX3( state, map->GetMapGameType( ), map->GetMapGameFlags( ), map->GetMapWidth( ), map->GetMapHeight( ), gameName, hostName, 0, map->GetMapPath( ), map->GetMapCRC( ) ) );
	}
}

void CBNET :: SendGameUncreate( )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_STOPADV( ) );
}

void CBNET :: SendGameRefresh( unsigned char state, string gameName, string hostName, CMap *map, uint32_t upTime )
{
	if( hostName.empty( ) )
	{
		BYTEARRAY UniqueName = m_Protocol->GetUniqueName( );
		hostName = string( UniqueName.begin( ), UniqueName.end( ) );
	}

	if( m_LoggedIn && map )
		m_Socket->PutBytes( m_Protocol->SEND_SID_STARTADVEX3( state, map->GetMapGameType( ), map->GetMapGameFlags( ), map->GetMapWidth( ), map->GetMapHeight( ), gameName, hostName, upTime, map->GetMapPath( ), map->GetMapCRC( ) ) );
}

void CBNET :: SendGameJoin( string gameName )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_GETADVLISTEX( gameName ) );
}

void CBNET :: SendGetFriendsList( )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_FRIENDSLIST( ) );
}

void CBNET :: SendGetClanList( )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_CLANMEMBERLIST( ) );
}

void CBNET :: QueueChatCommand( string chatCommand )
{
	if( chatCommand.empty( ) )
		return;

	m_ChatCommands.push( chatCommand );
}

void CBNET :: QueueChatCommand( string chatCommand, string user, bool whisper )
{
	if( chatCommand.empty( ) )
		return;

	// if whisper is true send the chat command as a whisper to user, otherwise just queue the chat command

	if( whisper )
		QueueChatCommand( "/w " + user + " " + chatCommand );
	else
		QueueChatCommand( chatCommand );
}

void CBNET :: ImmediateChatCommand( string chatCommand )
{
	if( chatCommand.empty( ) )
		return;

	if( GetTime( ) >= m_LastChatCommandTime + 2 )
	{
		CONSOLE_Print( "[LOCAL: " + m_Server + "] " + chatCommand );
		SendChatCommand( chatCommand );
		m_LastChatCommandTime = GetTime( );
	}
}

void CBNET :: ImmediateChatCommand( string chatCommand, string user, bool whisper )
{
	if( chatCommand.empty( ) )
		return;

	if( whisper )
		ImmediateChatCommand( "/w " + user + " " + chatCommand );
	else
		ImmediateChatCommand( chatCommand );
}

bool CBNET :: IsRootAdmin( string name )
{
	// m_RootAdmin was already transformed to lower case in the constructor

	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	return name == m_RootAdmin;
}

void CBNET :: HoldFriends( CBaseGame *game )
{
	if( game )
	{
		for( vector<CIncomingFriendList *> :: iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
			game->AddToReserved( (*i)->GetAccount( ) );
	}
}

void CBNET :: HoldClan( CBaseGame *game )
{
	if( game )
	{
		for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
			game->AddToReserved( (*i)->GetName( ) );
	}
}

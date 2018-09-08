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

#ifndef BNETPROTOCOL_H
#define BNETPROTOCOL_H

//
// CBNETProtocol
//

#define BNET_HEADER_CONSTANT 255

class CIncomingGameHost;
class CIncomingChatEvent;
class CIncomingFriendList;
class CIncomingClanList;

class CBNETProtocol
{
public:
	enum Protocol {
		SID_NULL					= 0,	// 0x0
		SID_STOPADV					= 2,	// 0x2
		SID_GETADVLISTEX			= 9,	// 0x9
		SID_ENTERCHAT				= 10,	// 0xA
		SID_JOINCHANNEL				= 12,	// 0xC
		SID_CHATCOMMAND				= 14,	// 0xE
		SID_CHATEVENT				= 15,	// 0xF
		SID_CHECKAD					= 21,	// 0x15
		SID_STARTADVEX3				= 28,	// 0x1C
		SID_DISPLAYAD				= 33,	// 0x21
		SID_NOTIFYJOIN				= 34,	// 0x22
		SID_PING					= 37,	// 0x25
		SID_LOGONRESPONSE			= 41,	// 0x29
		SID_NETGAMEPORT				= 69,	// 0x45
		SID_AUTH_INFO				= 80,	// 0x50
		SID_AUTH_CHECK				= 81,	// 0x51
		SID_AUTH_ACCOUNTLOGON		= 83,	// 0x53
		SID_AUTH_ACCOUNTLOGONPROOF	= 84,	// 0x54
		SID_WARDEN					= 94,	// 0x5E
		SID_FRIENDSLIST				= 101,	// 0x65
		SID_FRIENDSUPDATE			= 102,	// 0x66
		SID_CLANCREATIONINVITATION	= 114,	// 0x72
		SID_CLANINVITATION			= 119,  // 0x77
		SID_CLANREMOVEMEMBER    	= 120,  // 0x78
		SID_CLANINVITATIONRESPONSE	= 121,	// 0x79
		SID_CLANCHANGERANK			= 122,	// 0x7A
		SID_CLANSETMOTD 			= 123,	// 0x7B
		SID_CLANMEMBERLIST			= 125,	// 0x7D
		SID_CLANMEMBERSTATUSCHANGE	= 127	// 0x7F
	};

	enum KeyResult {
		KR_GOOD				= 0,
		KR_OLD_GAME_VERSION	= 256,
		KR_INVALID_VERSION	= 257,
		KR_ROC_KEY_IN_USE	= 513,
		KR_TFT_KEY_IN_USE	= 529
	};

	enum IncomingChatEvent {
		EID_SHOWUSER			= 1,	// received when you join a channel (includes users in the channel and their information)
		EID_JOIN				= 2,	// received when someone joins the channel you're currently in
		EID_LEAVE				= 3,	// received when someone leaves the channel you're currently in
		EID_WHISPER				= 4,	// received a whisper message
		EID_TALK				= 5,	// received when someone talks in the channel you're currently in
		EID_BROADCAST			= 6,	// server broadcast
		EID_CHANNEL				= 7,	// received when you join a channel (includes the channel's name, flags)
		EID_USERFLAGS			= 9,	// user flags updates
		EID_WHISPERSENT			= 10,	// sent a whisper message
		EID_CHANNELFULL			= 13,	// channel is full
		EID_CHANNELDOESNOTEXIST	= 14,	// channel does not exist
		EID_CHANNELRESTRICTED	= 15,	// channel is restricted
		EID_INFO				= 18,	// broadcast/information message
		EID_ERROR				= 19,	// error message
		EID_EMOTE				= 23	// emote
	};

	enum RankCode {
		CLAN_INITIATE = 0,			// 0x00 First week member
		CLAN_PARTIAL_MEMBER = 1,	// 0x01 Peon
		CLAN_MEMBER = 2,			// 0x02 Grunt
		CLAN_OFFICER = 3,			// 0x03 Shaman
		CLAN_LEADER = 4				// 0x04 Chieftain
	};

private:
	BYTEARRAY m_ClientToken;			// set in constructor
	BYTEARRAY m_LogonType;				// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_ServerToken;			// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_MPQFileTime;			// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_IX86VerFileName;		// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_ValueStringFormula;		// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_KeyState;				// set in RECEIVE_SID_AUTH_CHECK
	BYTEARRAY m_KeyStateDescription;	// set in RECEIVE_SID_AUTH_CHECK
	BYTEARRAY m_Salt;					// set in RECEIVE_SID_AUTH_ACCOUNTLOGON
	BYTEARRAY m_ServerPublicKey;		// set in RECEIVE_SID_AUTH_ACCOUNTLOGON
	BYTEARRAY m_UniqueName;				// set in RECEIVE_SID_ENTERCHAT
	BYTEARRAY m_ClanLastInviteTag;		// set in RECEIVE_SID_CLANCREATIONINVITATION, SID_CLANINVITATIONRESPONSE
	BYTEARRAY m_ClanLastInviteName;		// set in RECEIVE_SID_CLANCREATIONINVITATION, SID_CLANINVITATIONRESPONSE

public:
	CBNETProtocol( );
	~CBNETProtocol( );

	BYTEARRAY GetClientToken( )				{ return m_ClientToken; }
	BYTEARRAY GetLogonType( )				{ return m_LogonType; }
	BYTEARRAY GetServerToken( )				{ return m_ServerToken; }
	BYTEARRAY GetMPQFileTime( )				{ return m_MPQFileTime; }
	BYTEARRAY GetIX86VerFileName( )			{ return m_IX86VerFileName; }
	std::string GetIX86VerFileNameString( )		{ return std::string( m_IX86VerFileName.begin( ), m_IX86VerFileName.end( ) ); }
	BYTEARRAY GetValueStringFormula( )		{ return m_ValueStringFormula; }
	std::string GetValueStringFormulaString( )	{ return std::string( m_ValueStringFormula.begin( ), m_ValueStringFormula.end( ) ); }
	BYTEARRAY GetKeyState( )				{ return m_KeyState; }
	std::string GetKeyStateDescription( )		{ return std::string( m_KeyStateDescription.begin( ), m_KeyStateDescription.end( ) ); }
	BYTEARRAY GetSalt( )					{ return m_Salt; }
	BYTEARRAY GetServerPublicKey( )			{ return m_ServerPublicKey; }
	BYTEARRAY GetUniqueName( )				{ return m_UniqueName; }

	// receive functions

	bool RECEIVE_SID_NULL( BYTEARRAY data );
	CIncomingGameHost *RECEIVE_SID_GETADVLISTEX( BYTEARRAY data );
	bool RECEIVE_SID_ENTERCHAT( BYTEARRAY data );
	CIncomingChatEvent *RECEIVE_SID_CHATEVENT( BYTEARRAY data );
	bool RECEIVE_SID_CHECKAD( BYTEARRAY data );
	bool RECEIVE_SID_STARTADVEX3( BYTEARRAY data );
	BYTEARRAY RECEIVE_SID_PING( BYTEARRAY data );
	bool RECEIVE_SID_LOGONRESPONSE( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_INFO( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_CHECK( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_ACCOUNTLOGON( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY data );
	BYTEARRAY RECEIVE_SID_WARDEN( BYTEARRAY data );
	std::vector<CIncomingFriendList *> RECEIVE_SID_FRIENDSLIST( BYTEARRAY data );
	std::vector<CIncomingClanList *> RECEIVE_SID_CLANMEMBERLIST( BYTEARRAY data );
	CIncomingClanList *RECEIVE_SID_CLANMEMBERSTATUSCHANGE( BYTEARRAY data );
	std::string RECEIVE_SID_CLANCREATIONINVITATION( BYTEARRAY data );
	std::string RECEIVE_SID_CLANINVITATIONRESPONSE( BYTEARRAY data );

	// send functions

	BYTEARRAY SEND_PROTOCOL_INITIALIZE_SELECTOR( );
	BYTEARRAY SEND_SID_NULL( );
	BYTEARRAY SEND_SID_STOPADV( );
	BYTEARRAY SEND_SID_GETADVLISTEX( std::string gameName );
	BYTEARRAY SEND_SID_ENTERCHAT( );
	BYTEARRAY SEND_SID_JOINCHANNEL( std::string channel );
	BYTEARRAY SEND_SID_CHATCOMMAND( std::string command );
	BYTEARRAY SEND_SID_CHECKAD( );
	BYTEARRAY SEND_SID_STARTADVEX3( unsigned char state, BYTEARRAY mapGameType, BYTEARRAY mapFlags, BYTEARRAY mapWidth, BYTEARRAY mapHeight, std::string gameName, std::string hostName, uint32_t upTime, std::string mapPath, BYTEARRAY mapCRC, BYTEARRAY mapSHA1, uint32_t hostCounter );
	BYTEARRAY SEND_SID_NOTIFYJOIN( std::string gameName );
	BYTEARRAY SEND_SID_PING( BYTEARRAY pingValue );
	BYTEARRAY SEND_SID_LOGONRESPONSE( BYTEARRAY clientToken, BYTEARRAY serverToken, BYTEARRAY passwordHash, std::string accountName );
	BYTEARRAY SEND_SID_NETGAMEPORT( uint16_t serverPort );
	BYTEARRAY SEND_SID_AUTH_INFO( unsigned char ver, bool TFT, uint32_t localeID, std::string countryAbbrev, std::string country );
	BYTEARRAY SEND_SID_AUTH_CHECK( bool TFT, BYTEARRAY clientToken, BYTEARRAY exeVersion, BYTEARRAY exeVersionHash, BYTEARRAY keyInfoROC, BYTEARRAY keyInfoTFT, std::string exeInfo, std::string keyOwnerName );
	BYTEARRAY SEND_SID_AUTH_ACCOUNTLOGON( BYTEARRAY clientPublicKey, std::string accountName );
	BYTEARRAY SEND_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY clientPasswordProof );
	BYTEARRAY SEND_SID_WARDEN( BYTEARRAY wardenResponse );
	BYTEARRAY SEND_SID_FRIENDSLIST( );
	BYTEARRAY SEND_SID_CLANMEMBERLIST( );
	BYTEARRAY SEND_SID_CLANINVITATION( std::string accountName );
	BYTEARRAY SEND_SID_CLANREMOVEMEMBER( std::string accountName );
	BYTEARRAY SEND_SID_CLANCHANGERANK( std::string accountName, CBNETProtocol :: RankCode rank );
	BYTEARRAY SEND_SID_CLANSETMOTD( std::string motd );
	BYTEARRAY SEND_SID_CLANCREATIONINVITATION( bool accept );
	BYTEARRAY SEND_SID_CLANINVITATIONRESPONSE( bool accept );

	// other functions

private:
	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );
};

//
// CIncomingGameHost
//

class CIncomingGameHost
{
private:
	BYTEARRAY m_IP;
	uint16_t m_Port;
	std::string m_GameName;
	BYTEARRAY m_HostCounter;

public:
	CIncomingGameHost( BYTEARRAY &nIP, uint16_t nPort, std::string nGameName, BYTEARRAY &nHostCounter );
	~CIncomingGameHost( );

	BYTEARRAY GetIP( )			{ return m_IP; }
	std::string GetIPString( );
	uint16_t GetPort( )			{ return m_Port; }
	std::string GetGameName( )		{ return m_GameName; }
	BYTEARRAY GetHostCounter( )	{ return m_HostCounter; }
};

//
// CIncomingChatEvent
//

class CIncomingChatEvent
{
private:
	CBNETProtocol :: IncomingChatEvent m_ChatEvent;
	int32_t m_Ping;
	std::string m_User;
	std::string m_Message;

public:
	CIncomingChatEvent( CBNETProtocol :: IncomingChatEvent nChatEvent, int32_t nPing, std::string nUser, std::string nMessage );
	~CIncomingChatEvent( );

	CBNETProtocol :: IncomingChatEvent GetChatEvent( )	{ return m_ChatEvent; }
	int32_t GetPing( )									{ return m_Ping; }
	std::string GetUser( )									{ return m_User; }
	std::string GetMessage( )								{ return m_Message; }
};

//
// CIncomingFriendList
//

class CIncomingFriendList
{
private:
	std::string m_Account;
	unsigned char m_Status;
	unsigned char m_Area;
	std::string m_Location;

public:
	CIncomingFriendList( std::string nAccount, unsigned char nStatus, unsigned char nArea, std::string nLocation );
	~CIncomingFriendList( );

	std::string GetAccount( )		{ return m_Account; }
	unsigned char GetStatus( )	{ return m_Status; }
	unsigned char GetArea( )	{ return m_Area; }
	std::string GetLocation( )		{ return m_Location; }
	std::string GetDescription( );

private:
	std::string ExtractStatus( unsigned char status );
	std::string ExtractArea( unsigned char area );
	std::string ExtractLocation( std::string location );
};

//
// CIncomingClanList
//

class CIncomingClanList
{
private:
	std::string m_Name;
	unsigned char m_Rank;
	unsigned char m_Status;

public:
	CIncomingClanList( std::string nName, unsigned char nRank, unsigned char nStatus );
	~CIncomingClanList( );

	std::string GetName( )			{ return m_Name; }
	std::string GetRank( );
	std::string GetStatus( );
	std::string GetDescription( );
};

#endif

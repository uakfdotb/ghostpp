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

#ifndef BNET_H
#define BNET_H

//
// CBNET
//

class CTCPClient;
class CCommandPacket;
class CBNCSUtilInterface;
class CBNETProtocol;
class CIncomingFriendList;
class CIncomingClanList;
class CIncomingChatEvent;

class CBNET
{
public:
	CGHost *m_GHost;

private:
	CTCPClient *m_Socket;						// the connection to battle.net
	CBNETProtocol *m_Protocol;					// battle.net protocol
	queue<CCommandPacket *> m_Packets;			// queue of incoming packets
	CBNCSUtilInterface *m_BNCSUtil;				// the interface to the bncsutil library (used for logging into battle.net)
	queue<string> m_ChatCommands;				// queue of chat commands waiting to be sent (to prevent getting kicked for flooding)
	vector<CIncomingFriendList *> m_Friends;	// vector of friends
	vector<CIncomingClanList *> m_Clans;		// vector of clan members
	bool m_Exiting;								// set to true and this class will be deleted next update
	string m_Server;							// battle.net server to connect to
	string m_CDKeyROC;							// ROC CD key
	string m_CDKeyTFT;							// TFT CD key
	string m_CountryAbbrev;						// country abbreviation
	string m_Country;							// country
	string m_UserName;							// battle.net username
	string m_UserPassword;						// battle.net password
	string m_FirstChannel;						// the first chat channel to join upon entering chat (note: we hijack this to store the last channel when entering a game)
	string m_CurrentChannel;					// the current chat channel
	string m_RootAdmin;							// the root admin
	char m_CommandTrigger;						// the character prefix to identify commands
	unsigned char m_War3Version;				// custom warcraft 3 version for PvPGN users
	BYTEARRAY m_EXEVersion;						// custom exe version for PvPGN users
	BYTEARRAY m_EXEVersionHash;					// custom exe version hash for PvPGN users
	string m_PasswordHashType;					// password hash type for PvPGN users
	uint32_t m_MaxMessageLength;				// maximum message length for PvPGN users
	uint32_t m_NextConnectTime;					// GetTime when we should try connecting to battle.net next (after we get disconnected)
	uint32_t m_LastNullTime;					// GetTime when the last null packet was sent for detecting disconnects
	uint32_t m_LastChatCommandTicks;			// GetTicks when the last chat command was sent for the m_ChatCommands queue
	bool m_WaitingToConnect;					// if we're waiting to reconnect to battle.net after being disconnected
	bool m_LoggedIn;							// if we've logged into battle.net or not
	bool m_InChat;								// if we've entered chat or not (but we're not necessarily in a chat channel yet)
	bool m_HoldFriends;							// whether to auto hold friends when creating a game or not
	bool m_HoldClan;							// whether to auto hold clan members when creating a game or not

public:
	CBNET( CGHost *nGHost, string nServer, string nCDKeyROC, string nCDKeyTFT, string nCountryAbbrev, string nCountry, string nUserName, string nUserPassword, string nFirstChannel, string nRootAdmin, char nCommandTrigger, bool nHoldFriends, bool nHoldClan, unsigned char nWar3Version, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType, uint32_t nMaxMessageLength );
	~CBNET( );

	bool GetExiting( )					{ return m_Exiting; }
	string GetServer( )					{ return m_Server; }
	string GetCDKeyROC( )				{ return m_CDKeyROC; }
	string GetCDKeyTFT( )				{ return m_CDKeyTFT; }
	string GetUserName( )				{ return m_UserName; }
	string GetUserPassword( )			{ return m_UserPassword; }
	string GetFirstChannel( )			{ return m_FirstChannel; }
	string GetCurrentChannel( )			{ return m_CurrentChannel; }
	string GetRootAdmin( )				{ return m_RootAdmin; }
	char GetCommandTrigger( )			{ return m_CommandTrigger; }
	BYTEARRAY GetEXEVersion( )			{ return m_EXEVersion; }
	BYTEARRAY GetEXEVersionHash( )		{ return m_EXEVersionHash; }
	string GetPasswordHashType( )		{ return m_PasswordHashType; }
	bool GetLoggedIn( )					{ return m_LoggedIn; }
	bool GetInChat( )					{ return m_InChat; }
	bool GetHoldFriends( )				{ return m_HoldFriends; }
	bool GetHoldClan( )					{ return m_HoldClan; }
	BYTEARRAY GetUniqueName( );

	// processing functions

	unsigned int SetFD( void *fd, int *nfds );
	bool Update( void *fd );
	void ExtractPackets( );
	void ProcessPackets( );
	void ProcessChatEvent( CIncomingChatEvent *chatEvent );

	// functions to send packets to battle.net

	void SendEnterChat( );
	void SendJoinChannel( string channel );
	void SendChatCommand( string chatCommand );
	void SendGameCreate( unsigned char state, string gameName, string hostName, CMap *map, CSaveGame *saveGame, uint32_t hostCounter );
	void SendGameUncreate( );
	void SendGameRefresh( unsigned char state, string gameName, string hostName, CMap *map, CSaveGame *saveGame, uint32_t upTime, uint32_t hostCounter );
	void SendGameJoin( string gameName );
	void SendGetFriendsList( );
	void SendGetClanList( );

	// other functions

	void QueueChatCommand( string chatCommand );
	void QueueChatCommand( string chatCommand, string user, bool whisper );
	void ImmediateChatCommand( string chatCommand );
	void ImmediateChatCommand( string chatCommand, string user, bool whisper );
	bool IsRootAdmin( string name );
	void HoldFriends( CBaseGame *game );
	void HoldClan( CBaseGame *game );
};

#endif

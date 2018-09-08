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

#include "bnetprotocol.h"

//
// CBNET
//

class CTCPClient;
class CCommandPacket;
class CBNCSUtilInterface;
class CBNETProtocol;
class CBNLSClient;
class CIncomingFriendList;
class CIncomingClanList;
class CIncomingChatEvent;
class CCallableAdminCount;
class CCallableAdminAdd;
class CCallableAdminRemove;
class CCallableAdminList;
class CCallableBanCount;
class CCallableBanAdd;
class CCallableBanRemove;
class CCallableBanList;
class CCallableGamePlayerSummaryCheck;
class CCallableDotAPlayerSummaryCheck;
class CDBBan;

typedef std::pair<std::string,CCallableAdminCount *> PairedAdminCount;
typedef std::pair<std::string,CCallableAdminAdd *> PairedAdminAdd;
typedef std::pair<std::string,CCallableAdminRemove *> PairedAdminRemove;
typedef std::pair<std::string,CCallableBanCount *> PairedBanCount;
typedef std::pair<std::string,CCallableBanAdd *> PairedBanAdd;
typedef std::pair<std::string,CCallableBanRemove *> PairedBanRemove;
typedef std::pair<std::string,CCallableGamePlayerSummaryCheck *> PairedGPSCheck;
typedef std::pair<std::string,CCallableDotAPlayerSummaryCheck *> PairedDPSCheck;

class CBNET
{
public:
	CGHost *m_GHost;

private:
	CTCPClient *m_Socket;							// the connection to battle.net
	CBNETProtocol *m_Protocol;						// battle.net protocol
	CBNLSClient *m_BNLSClient;						// the BNLS client (for external warden handling)
	std::queue<CCommandPacket *> m_Packets;				// std::queue of incoming packets
	std::mutex m_PacketsMutex;					// game sometimes calls QueueChatCommand and UnqueueChatCommand; this makes the functions synchronized
	CBNCSUtilInterface *m_BNCSUtil;					// the interface to the bncsutil library (used for logging into battle.net)
	std::queue<BYTEARRAY> m_OutPackets;					// std::queue of outgoing packets to be sent (to prevent getting kicked for flooding)
	std::vector<CIncomingFriendList *> m_Friends;		// std::vector of friends
	std::vector<CIncomingClanList *> m_Clans;			// std::vector of clan members
	std::vector<PairedAdminCount> m_PairedAdminCounts;	// std::vector of paired threaded database admin counts in progress
	std::vector<PairedAdminAdd> m_PairedAdminAdds;		// std::vector of paired threaded database admin adds in progress
	std::vector<PairedAdminRemove> m_PairedAdminRemoves;	// std::vector of paired threaded database admin removes in progress
	std::vector<PairedBanCount> m_PairedBanCounts;		// std::vector of paired threaded database ban counts in progress
	std::vector<PairedBanAdd> m_PairedBanAdds;			// std::vector of paired threaded database ban adds in progress
	std::vector<PairedBanRemove> m_PairedBanRemoves;		// std::vector of paired threaded database ban removes in progress
	std::vector<PairedGPSCheck> m_PairedGPSChecks;		// std::vector of paired threaded database game player summary checks in progress
	std::vector<PairedDPSCheck> m_PairedDPSChecks;		// std::vector of paired threaded database DotA player summary checks in progress
	CCallableAdminList *m_CallableAdminList;		// threaded database admin list in progress
	CCallableBanList *m_CallableBanList;			// threaded database ban list in progress
	std::vector<std::string> m_Admins;						// std::vector of cached admins
	std::vector<CDBBan *> m_Bans;						// std::vector of cached bans
	std::mutex m_BansMutex;						// synchronizes accesses and updates to the m_Bans std::vector
	bool m_Exiting;									// set to true and this class will be deleted next update
	std::string m_Server;								// battle.net server to connect to
	std::string m_ServerIP;								// battle.net server to connect to (the IP address so we don't have to resolve it every time we connect)
	std::string m_ServerAlias;							// battle.net server alias (short name, e.g. "USEast")
	uint32_t m_ServerReconnectCount;				// counts how many times we have reconnected to the server and failed; when it's high, we resolve ServerIP again
	std::string m_BNLSServer;							// BNLS server to connect to (for warden handling)
	uint16_t m_BNLSPort;							// BNLS port
	uint32_t m_BNLSWardenCookie;					// BNLS warden cookie
	std::string m_CDKeyROC;								// ROC CD key
	std::string m_CDKeyTFT;								// TFT CD key
	std::string m_CountryAbbrev;							// country abbreviation
	std::string m_Country;								// country
	uint32_t m_LocaleID;							// see: http://msdn.microsoft.com/en-us/library/0h88fahh%28VS.85%29.aspx
	std::string m_UserName;								// battle.net username
	std::string m_UserPassword;							// battle.net password
	std::string m_FirstChannel;							// the first chat channel to join upon entering chat (note: we hijack this to store the last channel when entering a game)
	std::string m_CurrentChannel;						// the current chat channel
	std::string m_RootAdmin;								// the root admin
	char m_CommandTrigger;							// the character prefix to identify commands
	unsigned char m_War3Version;					// custom warcraft 3 version for PvPGN users
	BYTEARRAY m_EXEVersion;							// custom exe version for PvPGN users
	BYTEARRAY m_EXEVersionHash;						// custom exe version hash for PvPGN users
	std::string m_PasswordHashType;						// password hash type for PvPGN users
	std::string m_PVPGNRealmName;						// realm name for PvPGN users (for mutual friend spoofchecks)
	uint32_t m_MaxMessageLength;					// maximum message length for PvPGN users
	uint32_t m_HostCounterID;						// the host counter ID to identify players from this realm
	uint32_t m_LastDisconnectedTime;				// GetTime when we were last disconnected from battle.net
	uint32_t m_LastConnectionAttemptTime;			// GetTime when we last attempted to connect to battle.net
	uint32_t m_LastNullTime;						// GetTime when the last null packet was sent for detecting disconnects
	uint32_t m_LastOutPacketTicks;					// GetTicks when the last packet was sent for the m_OutPackets std::queue
	uint32_t m_LastOutPacketSize;
	uint32_t m_FrequencyDelayTimes;
	uint32_t m_LastAdminRefreshTime;				// GetTime when the admin list was last refreshed from the database
	uint32_t m_LastBanRefreshTime;					// GetTime when the ban list was last refreshed from the database
	bool m_FirstConnect;							// if we haven't tried to connect to battle.net yet
	bool m_WaitingToConnect;						// if we're waiting to reconnect to battle.net after being disconnected
	bool m_LoggedIn;								// if we've logged into battle.net or not
	bool m_InChat;									// if we've entered chat or not (but we're not necessarily in a chat channel yet)
	bool m_HoldFriends;								// whether to auto hold friends when creating a game or not
	bool m_HoldClan;								// whether to auto hold clan members when creating a game or not
	bool m_PublicCommands;							// whether to allow public commands or not
	bool m_LastInviteCreation;						// whether the last invite received was for a clan creation (else, it was for invitation response)

public:
	CBNET( CGHost *nGHost, std::string nServer, std::string nServerAlias, std::string nBNLSServer, uint16_t nBNLSPort, uint32_t nBNLSWardenCookie, std::string nCDKeyROC, std::string nCDKeyTFT, std::string nCountryAbbrev, std::string nCountry, uint32_t nLocaleID, std::string nUserName, std::string nUserPassword, std::string nFirstChannel, std::string nRootAdmin, char nCommandTrigger, bool nHoldFriends, bool nHoldClan, bool nPublicCommands, unsigned char nWar3Version, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, std::string nPasswordHashType, std::string nPVPGNRealmName, uint32_t nMaxMessageLength, uint32_t nHostCounterID );
	~CBNET( );

	bool GetExiting( )					{ return m_Exiting; }
	std::string GetServer( )					{ return m_Server; }
	std::string GetServerAlias( )			{ return m_ServerAlias; }
	std::string GetCDKeyROC( )				{ return m_CDKeyROC; }
	std::string GetCDKeyTFT( )				{ return m_CDKeyTFT; }
	std::string GetUserName( )				{ return m_UserName; }
	std::string GetUserPassword( )			{ return m_UserPassword; }
	std::string GetFirstChannel( )			{ return m_FirstChannel; }
	std::string GetCurrentChannel( )			{ return m_CurrentChannel; }
	std::string GetRootAdmin( )				{ return m_RootAdmin; }
	char GetCommandTrigger( )			{ return m_CommandTrigger; }
	BYTEARRAY GetEXEVersion( )			{ return m_EXEVersion; }
	BYTEARRAY GetEXEVersionHash( )		{ return m_EXEVersionHash; }
	std::string GetPasswordHashType( )		{ return m_PasswordHashType; }
	std::string GetPVPGNRealmName( )			{ return m_PVPGNRealmName; }
	uint32_t GetHostCounterID( )		{ return m_HostCounterID; }
	bool GetLoggedIn( )					{ return m_LoggedIn; }
	bool GetInChat( )					{ return m_InChat; }
	bool GetHoldFriends( )				{ return m_HoldFriends; }
	bool GetHoldClan( )					{ return m_HoldClan; }
	bool GetPublicCommands( )			{ return m_PublicCommands; }
	uint32_t GetOutPacketsQueued( )		{ return m_OutPackets.size( ); }
	BYTEARRAY GetUniqueName( );

	// processing functions

	unsigned int SetFD( void *fd, void *send_fd, int *nfds );
	bool Update( void *fd, void *send_fd );
	void ExtractPackets( );
	void ProcessPackets( );
	void ProcessChatEvent( CIncomingChatEvent *chatEvent );
	void BotCommand( std::string Message, std::string User, bool Whisper, bool ForceRoot );

	// functions to send packets to battle.net

	void SendJoinChannel( std::string channel );
	void SendGetFriendsList( );
	void SendGetClanList( );
	void SendClanInvitation( std::string accountName );
	void SendClanRemoveMember( std::string accountName );
	void SendClanChangeRank( std::string accountName, CBNETProtocol::RankCode rank );
	void SendClanSetMotd( std::string motd );
	void SendClanAcceptInvite( bool accept );
	void QueueEnterChat( );
	void QueueChatCommand( std::string chatCommand );
	void QueueChatCommand( std::string chatCommand, std::string user, bool whisper );
	void QueueGameCreate( unsigned char state, std::string gameName, std::string hostName, CMap *map, CSaveGame *saveGame, uint32_t hostCounter );
	void QueueGameRefresh( unsigned char state, std::string gameName, std::string hostName, CMap *map, CSaveGame *saveGame, uint32_t upTime, uint32_t hostCounter );
	void QueueGameUncreate( );

	void UnqueuePackets( unsigned char type );
	void UnqueueChatCommand( std::string chatCommand );
	void UnqueueGameRefreshes( );

	// other functions

	bool IsAdmin( std::string name );
	bool IsRootAdmin( std::string name );
	CDBBan *IsBannedName( std::string name );
	CDBBan *IsBannedIP( std::string ip );
	void AddAdmin( std::string name );
	void AddBan( std::string name, std::string ip, std::string gamename, std::string admin, std::string reason );
	void RemoveAdmin( std::string name );
	void RemoveBan( std::string name );
	void HoldFriends( CBaseGame *game );
	void HoldClan( CBaseGame *game );
};

#endif

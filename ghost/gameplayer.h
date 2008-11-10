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

#ifndef GAMEPLAYER_H
#define GAMEPLAYER_H

class CTCPSocket;
class CCommandPacket;
class CGameProtocol;
class CGame;

//
// CPotentialPlayer
//

class CPotentialPlayer
{
public:
	CGameProtocol *m_Protocol;
	CBaseGame *m_Game;

protected:
	// note: we permit m_Socket to be NULL in this class to allow for the virtual host player which doesn't really exist
	// it also allows us to convert CPotentialPlayers to CGamePlayers without the CPotentialPlayer's destructor closing the socket

	CTCPSocket *m_Socket;
	queue<CCommandPacket *> m_Packets;
	bool m_DeleteMe;
	bool m_Error;
	string m_ErrorString;

public:
	CPotentialPlayer( CGameProtocol *nProtocol, CBaseGame *nGame, CTCPSocket *nSocket );
	virtual ~CPotentialPlayer( );

	virtual CTCPSocket *GetSocket( )				{ return m_Socket; }
	virtual BYTEARRAY GetExternalIP( );
	virtual string GetExternalIPString( );
	virtual queue<CCommandPacket *> GetPackets( )	{ return m_Packets; }
	virtual bool GetDeleteMe( )						{ return m_DeleteMe; }
	virtual bool GetError( )						{ return m_Error; }
	virtual string GetErrorString( )				{ return m_ErrorString; }

	virtual void SetSocket( CTCPSocket *nSocket )	{ m_Socket = nSocket; }
	virtual void SetDeleteMe( bool nDeleteMe )		{ m_DeleteMe = nDeleteMe; }

	// processing functions

	virtual bool Update( void *fd );
	virtual void ExtractPackets( );
	virtual void ProcessPackets( );
};

//
// CGamePlayer
//

class CGamePlayer : public CPotentialPlayer
{
private:
	unsigned char m_PID;
	string m_Name;								// the player's name
	BYTEARRAY m_InternalIP;						// the player's internal IP address as reported by the player when connecting
	vector<uint32_t> m_Pings;					// store the last few (20) pings received so we can take an average
	queue<uint32_t> m_CheckSums;				// the last few checksums the player has sent (for detecting desyncs)
	string m_LeftReason;						// the reason the player left the game
	string m_SpoofedRealm;						// the realm the player last spoof checked on
	uint32_t m_LeftCode;						// the code to be sent in W3GS_PLAYERLEAVE_OTHERS for why this player left the game
	uint32_t m_LoginAttempts;					// the number of attempts to login (used with CAdminGame only)
	uint32_t m_SyncCounter;						// the number of keepalive packets received from this player
	uint32_t m_JoinTime;						// GetTime when the player joined the game (used to delay sending the /whois a few seconds to allow for some lag)
	uint32_t m_LastMapPartSent;					// the last mappart sent to the player (for sending more than one part at a time)
	uint32_t m_StartedDownloadingTicks;			// GetTicks when the player started downloading the map
	uint32_t m_FinishedLoadingTicks;			// GetTicks when the player finished loading the game
	uint32_t m_StartedLaggingTicks;				// GetTicks when the player started lagging
	uint32_t m_StatsSentTime;					// GetTime when we sent this player's stats to the chat (to prevent players from spamming !stats)
	uint32_t m_StatsDotASentTime;				// GetTime when we sent this player's dota stats to the chat (to prevent players from spamming !statsdota)
	bool m_LoggedIn;							// if the player has logged in or not (used with CAdminGame only)
	bool m_Spoofed;								// if the player has spoof checked or not
	bool m_Reserved;							// if the player is reserved (VIP) or not
	bool m_WhoisSent;							// if we've sent a battle.net /whois for this player yet (for spoof checking)
	bool m_DownloadAllowed;						// if we're allowed to download the map or not (used with permission based map downloads)
	bool m_DownloadStarted;						// if we've started downloading the map or not
	bool m_DownloadFinished;					// if we've finished downloading the map or not
	bool m_FinishedLoading;						// if the player has finished loading or not
	bool m_Lagging;								// if the player is lagging or not (on the lag screen)
	bool m_DropVote;							// if the player voted to drop the laggers or not (on the lag screen)
	bool m_LeftMessageSent;						// if the playerleave message has been sent or not

public:
	CGamePlayer( CGameProtocol *nProtocol, CBaseGame *nGame, CTCPSocket *nSocket, unsigned char nPID, string nName, BYTEARRAY nInternalIP, bool nReserved );
	CGamePlayer( CPotentialPlayer *potential, unsigned char nPID, string nName, BYTEARRAY nInternalIP, bool nReserved );
	virtual ~CGamePlayer( );

	unsigned char GetPID( )					{ return m_PID; }
	string GetName( )						{ return m_Name; }
	BYTEARRAY GetInternalIP( )				{ return m_InternalIP; }
	unsigned int GetNumPings( )				{ return m_Pings.size( ); }
	unsigned int GetNumCheckSums( )			{ return m_CheckSums.size( ); }
	queue<uint32_t> *GetCheckSums( )		{ return &m_CheckSums; }
	string GetLeftReason( )					{ return m_LeftReason; }
	string GetSpoofedRealm( )				{ return m_SpoofedRealm; }
	uint32_t GetLeftCode( )					{ return m_LeftCode; }
	uint32_t GetLoginAttempts( )			{ return m_LoginAttempts; }
	uint32_t GetSyncCounter( )				{ return m_SyncCounter; }
	uint32_t GetLastMapPartSent( )			{ return m_LastMapPartSent; }
	uint32_t GetStartedDownloadingTicks( )	{ return m_StartedDownloadingTicks; }
	uint32_t GetFinishedLoadingTicks( )		{ return m_FinishedLoadingTicks; }
	uint32_t GetStartedLaggingTicks( )		{ return m_StartedLaggingTicks; }
	uint32_t GetStatsSentTime( )			{ return m_StatsSentTime; }
	uint32_t GetStatsDotASentTime( )		{ return m_StatsDotASentTime; }
	bool GetLoggedIn( )						{ return m_LoggedIn; }
	bool GetSpoofed( )						{ return m_Spoofed; }
	bool GetReserved( )						{ return m_Reserved; }
	bool GetWhoisSent( )					{ return m_WhoisSent; }
	bool GetDownloadAllowed( )				{ return m_DownloadAllowed; }
	bool GetDownloadStarted( )				{ return m_DownloadStarted; }
	bool GetDownloadFinished( )				{ return m_DownloadFinished; }
	bool GetFinishedLoading( )				{ return m_FinishedLoading; }
	bool GetLagging( )						{ return m_Lagging; }
	bool GetDropVote( )						{ return m_DropVote; }
	bool GetLeftMessageSent( )				{ return m_LeftMessageSent; }

	void SetLeftReason( string nLeftReason )								{ m_LeftReason = nLeftReason; }
	void SetSpoofedRealm( string nSpoofedRealm )							{ m_SpoofedRealm = nSpoofedRealm; }
	void SetLeftCode( uint32_t nLeftCode )									{ m_LeftCode = nLeftCode; }
	void SetLoginAttempts( uint32_t nLoginAttempts )						{ m_LoginAttempts = nLoginAttempts; }
	void SetSyncCounter( uint32_t nSyncCounter )							{ m_SyncCounter = nSyncCounter; }
	void SetLastMapPartSent( uint32_t nLastMapPartSent )					{ m_LastMapPartSent = nLastMapPartSent; }
	void SetStartedDownloadingTicks( uint32_t nStartedDownloadingTicks )	{ m_StartedDownloadingTicks = nStartedDownloadingTicks; }
	void SetStartedLaggingTicks( uint32_t nStartedLaggingTicks )			{ m_StartedLaggingTicks = nStartedLaggingTicks; }
	void SetStatsSentTime( uint32_t nStatsSentTime )						{ m_StatsSentTime = nStatsSentTime; }
	void SetStatsDotASentTime( uint32_t nStatsDotASentTime )				{ m_StatsDotASentTime = nStatsDotASentTime; }
	void SetLoggedIn( bool nLoggedIn )										{ m_LoggedIn = nLoggedIn; }
	void SetSpoofed( bool nSpoofed )										{ m_Spoofed = nSpoofed; }
	void SetReserved( bool nReserved )										{ m_Reserved = nReserved; }
	void SetDownloadAllowed( bool nDownloadAllowed )						{ m_DownloadAllowed = nDownloadAllowed; }
	void SetDownloadStarted( bool nDownloadStarted )						{ m_DownloadStarted = nDownloadStarted; }
	void SetDownloadFinished( bool nDownloadFinished )						{ m_DownloadFinished = nDownloadFinished; }
	void SetLagging( bool nLagging )										{ m_Lagging = nLagging; }
	void SetDropVote( bool nDropVote )										{ m_DropVote = nDropVote; }
	void SetLeftMessageSent( bool nLeftMessageSent )						{ m_LeftMessageSent = nLeftMessageSent; }

	uint32_t GetPing( bool LCPing );

	// processing functions

	virtual bool Update( void *fd );
	virtual void ProcessPackets( );
};

#endif

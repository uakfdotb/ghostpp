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

#ifndef GAME_H
#define GAME_H

//
// CGame
//

class CDBBan;
class CDBGame;
class CDBGamePlayer;
class CStats;
class CCallableBanCheck;
class CCallableBanAdd;
class CCallableGameAdd;
class CCallableGamePlayerSummaryCheck;
class CCallableDotAPlayerSummaryCheck;

typedef std::pair<std::string,CCallableBanCheck *> PairedBanCheck;
typedef std::pair<std::string,CCallableBanAdd *> PairedBanAdd;
typedef std::pair<std::string,CCallableGamePlayerSummaryCheck *> PairedGPSCheck;
typedef std::pair<std::string,CCallableDotAPlayerSummaryCheck *> PairedDPSCheck;

class CGame : public CBaseGame
{
protected:
	CDBBan *m_DBBanLast;						// last ban for the !banlast command - this is a pointer to one of the items in m_DBBans
	std::vector<CDBBan *> m_DBBans;					// std::vector of potential ban data for the database (see the Update function for more info, it's not as straightforward as you might think)
	CDBGame *m_DBGame;							// potential game data for the database
	std::vector<CDBGamePlayer *> m_DBGamePlayers;	// std::vector of potential gameplayer data for the database
	CStats *m_Stats;							// class to keep track of game stats such as kills/deaths/assists in dota
	CCallableGameAdd *m_CallableGameAdd;		// threaded database game addition in progress
	std::vector<PairedBanCheck> m_PairedBanChecks;	// std::vector of paired threaded database ban checks in progress
	std::vector<PairedBanAdd> m_PairedBanAdds;		// std::vector of paired threaded database ban adds in progress
	std::vector<PairedGPSCheck> m_PairedGPSChecks;	// std::vector of paired threaded database game player summary checks in progress
	std::vector<PairedDPSCheck> m_PairedDPSChecks;	// std::vector of paired threaded database DotA player summary checks in progress

public:
	CGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, std::string nGameName, std::string nOwnerName, std::string nCreatorName, std::string nCreatorServer );
	virtual ~CGame( );

	virtual bool Update( void *fd, void *send_fd );
	virtual void EventPlayerDeleted( CGamePlayer *player );
	virtual bool EventPlayerAction( CGamePlayer *player, CIncomingAction *action );
	virtual bool EventPlayerBotCommand( CGamePlayer *player, std::string command, std::string payload );
	virtual void EventGameStarted( );
	virtual bool IsGameDataSaved( );
	virtual void SaveGameData( );
};

#endif

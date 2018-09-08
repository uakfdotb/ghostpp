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

#ifndef GAME_ADMIN_H
#define GAME_ADMIN_H

//
// CAdminGame
//

class CCallableAdminCount;
class CCallableAdminAdd;
class CCallableAdminRemove;
class CCallableBanCount;
// class CCallableBanAdd;
class CCallableBanRemove;

typedef std::pair<std::string,CCallableAdminCount *> PairedAdminCount;
typedef std::pair<std::string,CCallableAdminAdd *> PairedAdminAdd;
typedef std::pair<std::string,CCallableAdminRemove *> PairedAdminRemove;
typedef std::pair<std::string,CCallableBanCount *> PairedBanCount;
// typedef std::pair<std::string,CCallableBanAdd *> PairedBanAdd;
typedef std::pair<std::string,CCallableBanRemove *> PairedBanRemove;

typedef std::pair<std::string,uint32_t> TempBan;

class CAdminGame : public CBaseGame
{
protected:
	std::string m_Password;
	std::vector<TempBan> m_TempBans;
	std::vector<PairedAdminCount> m_PairedAdminCounts;	// std::vector of paired threaded database admin counts in progress
	std::vector<PairedAdminAdd> m_PairedAdminAdds;		// std::vector of paired threaded database admin adds in progress
	std::vector<PairedAdminRemove> m_PairedAdminRemoves;	// std::vector of paired threaded database admin removes in progress
	std::vector<PairedBanCount> m_PairedBanCounts;		// std::vector of paired threaded database ban counts in progress
	// std::vector<PairedBanAdd> m_PairedBanAdds;		// std::vector of paired threaded database ban adds in progress
	std::vector<PairedBanRemove> m_PairedBanRemoves;		// std::vector of paired threaded database ban removes in progress

public:
	CAdminGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, std::string nGameName, std::string nPassword );
	virtual ~CAdminGame( );

	virtual bool Update( void *fd, void *send_fd );
	virtual void SendAdminChat( std::string message );
	virtual void SendWelcomeMessage( CGamePlayer *player );
	virtual void EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer );
	virtual bool EventPlayerBotCommand( CGamePlayer *player, std::string command, std::string payload );
};

#endif

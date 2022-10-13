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

typedef pair<string,CCallableAdminCount *> PairedAdminCount;
typedef pair<string,CCallableAdminAdd *> PairedAdminAdd;
typedef pair<string,CCallableAdminRemove *> PairedAdminRemove;
typedef pair<string,CCallableBanCount *> PairedBanCount;
// typedef pair<string,CCallableBanAdd *> PairedBanAdd;
typedef pair<string,CCallableBanRemove *> PairedBanRemove;

typedef pair<string,uint32_t> TempBan;

class CAdminGame : public CBaseGame
{
protected:
	string m_Password;
	vector<TempBan> m_TempBans;
	vector<PairedAdminCount> m_PairedAdminCounts;	// vector of paired threaded database admin counts in progress
	vector<PairedAdminAdd> m_PairedAdminAdds;		// vector of paired threaded database admin adds in progress
	vector<PairedAdminRemove> m_PairedAdminRemoves;	// vector of paired threaded database admin removes in progress
	vector<PairedBanCount> m_PairedBanCounts;		// vector of paired threaded database ban counts in progress
	// vector<PairedBanAdd> m_PairedBanAdds;		// vector of paired threaded database ban adds in progress
	vector<PairedBanRemove> m_PairedBanRemoves;		// vector of paired threaded database ban removes in progress

public:
	CAdminGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nPassword );
	virtual ~CAdminGame( );

	virtual bool Update( void *fd, void *send_fd );
	virtual void SendAdminChat( string message );
	virtual void SendWelcomeMessage( CGamePlayer *player );
	virtual void EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer );
	virtual bool EventPlayerBotCommand( CGamePlayer *player, string command, string payload );
};

#endif

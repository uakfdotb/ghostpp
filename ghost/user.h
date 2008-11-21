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

#ifndef USER_H
#define USER_H

#define ORIGIN_NONE						0
#define ORIGIN_CONSOLE					1
#define ORIGIN_TELNET					2
#define ORIGIN_ADMINGAME				3
#define ORIGIN_BNET						4
#define ORIGIN_GAME						5

#define ACCESS_VERSION					( 1 )		// !version
#define ACCESS_STATS					( 1 << 1 )	// !stats !statsdota
#define ACCESS_CHECKME					( 1 << 2 )	// !checkme
#define ACCESS_CHECK					( 1 << 3 )	// !check
#define ACCESS_FROM						( 1 << 4 )	// !from
#define ACCESS_PING						( 1 << 5 )	// !ping
#define ACCESS_GAME_MANAGEMENT			( 1 << 6 )	// !a !abort !autostart !hold !hostsg !load !loadsg !map !priv !privby !pub !pubby !start !unhost
#define ACCESS_SLOT_MANAGEMENT			( 1 << 7 )	// !close !closeall !open !openall !sp !swap
#define ACCESS_COMP_MANAGEMENT			( 1 << 8 )	// !comp !compcolour !comphandicap !comprace !compteam
#define ACCESS_SENDLAN					( 1 << 9 )	// !sendlan
#define ACCESS_REFRESH					( 1 << 10 )	// !refresh
#define ACCESS_DOWNLOAD					( 1 << 11 )	// !dl !download
#define ACCESS_ANNOUNCE					( 1 << 12 )	// !announce
#define ACCESS_OWNER					( 1 << 13 )	// !owner
#define ACCESS_LATENCY					( 1 << 14 )	// !latency
#define ACCESS_SYNCLIMIT				( 1 << 15 )	// !synclimit
#define ACCESS_AUTOSAVE					( 1 << 16 )	// !autosave
#define ACCESS_MUTE						( 1 << 17 )	// !muteall !unmuteall
#define ACCESS_KICK						( 1 << 18 )	// !kick
#define ACCESS_DROP						( 1 << 19 )	// !drop
#define ACCESS_END						( 1 << 20 )	// !end
#define ACCESS_AUTOHOST					( 1 << 21 )	// !autohost
#define ACCESS_ENDGAME					( 1 << 22 )	// !endgame !getgame !getgames
#define ACCESS_BAN_MANAGEMENT			( 1 << 23 )	// !addban !ban !banlast !delban !unban
#define ACCESS_BAN_CHECKING				( 1 << 24 )	// !checkban !countbans
#define ACCESS_BNET_MANAGEMENT			( 1 << 25 )	// !channel !getclan !getfriends !say
#define ACCESS_SAYGAMES					( 1 << 26 )	// !getgame !getgames !saygame !saygames
#define ACCESS_ADMIN_MANAGEMENT			( 1 << 27 )	// !addadmin !deladmin
#define ACCESS_ADMIN_CHECKING			( 1 << 28 )	// !checkadmin !countadmins
#define ACCESS_BOT_MANAGEMENT			( 1 << 29 )	// !disable !enable !exit !quit

#define ACCESSGROUP_USER				ACCESS_VERSION | ACCESS_STATS | ACCESS_CHECKME
#define ACCESSGROUP_ADMIN				ACCESS_VERSION | ACCESS_STATS | ACCESS_CHECKME | ACCESS_CHECK | ACCESS_FROM | ACCESS_PING | ACCESS_GAME_MANAGEMENT | ACCESS_SLOT_MANAGEMENT | ACCESS_COMP_MANAGEMENT | ACCESS_SENDLAN | ACCESS_REFRESH | ACCESS_DOWNLOAD | ACCESS_ANNOUNCE | ACCESS_OWNER | ACCESS_LATENCY | ACCESS_SYNCLIMIT | ACCESS_AUTOSAVE | ACCESS_MUTE | ACCESS_KICK | ACCESS_DROP | ACCESS_END | ACCESS_ENDGAME | ACCESS_BAN_MANAGEMENT | ACCESS_BAN_CHECKING | ACCESS_BNET_MANAGEMENT
#define ACCESSGROUP_ALL					( ~0 )

/*

!lock
!unlock

*/

//
// CUser
//

class CUser
{
private:
	string m_Name;
	uint32_t m_Origin;
	uint64_t m_Access;

public:
	CUser( );
	~CUser( );

	string GetName( )			{ return m_Name; }
	uint32_t GetOrigin( )		{ return m_Origin; }
	uint64_t GetAccess( )		{ return m_Access; }

	void SetName( string nName )		{ m_Name = nName; }
	void SetOrigin( uint32_t nOrigin )	{ m_Origin = nOrigin; }
	void SetAccess( uint64_t nAccess )	{ m_Access = nAccess; }
};

#endif

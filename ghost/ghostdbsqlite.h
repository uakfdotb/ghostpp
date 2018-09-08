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

#ifndef GHOSTDBSQLITE_H
#define GHOSTDBSQLITE_H

/**************
 *** SCHEMA ***
 **************

CREATE TABLE admins (
	id INTEGER PRIMARY KEY,
	name TEXT NOT NULL,
	server TEXT NOT NULL DEFAULT ""
)

CREATE TABLE bans (
	id INTEGER PRIMARY KEY,
	server TEXT NOT NULL,
	name TEXT NOT NULL,
	ip TEXT,
	date TEXT NOT NULL,
	gamename TEXT,
	admin TEXT NOT NULL,
	reason TEXT
)

CREATE TABLE games (
	id INTEGER PRIMARY KEY,
	server TEXT NOT NULL,
	map TEXT NOT NULL,
	datetime TEXT NOT NULL,
	gamename TEXT NOT NULL,
	ownername TEXT NOT NULL,
	duration INTEGER NOT NULL,
	gamestate INTEGER NOT NULL DEFAULT 0,
	creatorname TEXT NOT NULL DEFAULT "",
	creatorserver TEXT NOT NULL DEFAULT ""
)

CREATE TABLE gameplayers (
	id INTEGER PRIMARY KEY,
	gameid INTEGER NOT NULL,
	name TEXT NOT NULL,
	ip TEXT NOT NULL,
	spoofed INTEGER NOT NULL,
	reserved INTEGER NOT NULL,
	loadingtime INTEGER NOT NULL,
	left INTEGER NOT NULL,
	leftreason TEXT NOT NULL,
	team INTEGER NOT NULL,
	colour INTEGER NOT NULL,
	spoofedrealm TEXT NOT NULL DEFAULT ""
)

CREATE TABLE dotagames (
	id INTEGER PRIMARY KEY,
	gameid INTEGER NOT NULL,
	winner INTEGER NOT NULL,
	min INTEGER NOT NULL DEFAULT 0,
	sec INTEGER NOT NULL DEFAULT 0
)

CREATE TABLE dotaplayers (
	id INTEGER PRIMARY KEY,
	gameid INTEGER NOT NULL,
	colour INTEGER NOT NULL,
	kills INTEGER NOT NULL,
	deaths INTEGER NOT NULL,
	creepkills INTEGER NOT NULL,
	creepdenies INTEGER NOT NULL,
	assists INTEGER NOT NULL,
	gold INTEGER NOT NULL,
	neutralkills INTEGER NOT NULL,
	item1 TEXT NOT NULL,
	item2 TEXT NOT NULL,
	item3 TEXT NOT NULL,
	item4 TEXT NOT NULL,
	item5 TEXT NOT NULL,
	item6 TEXT NOT NULL,
	hero TEXT NOT NULL DEFAULT "",
	newcolour NOT NULL DEFAULT 0,
	towerkills NOT NULL DEFAULT 0,
	raxkills NOT NULL DEFAULT 0,
	courierkills NOT NULL DEFAULT 0
)

CREATE TABLE config (
	name TEXT NOT NULL PRIMARY KEY,
	value TEXT NOT NULL
)

CREATE TABLE downloads (
	id INTEGER PRIMARY KEY,
	map TEXT NOT NULL,
	mapsize INTEGER NOT NULL,
	datetime TEXT NOT NULL,
	name TEXT NOT NULL,
	ip TEXT NOT NULL,
	spoofed INTEGER NOT NULL,
	spoofedrealm TEXT NOT NULL,
	downloadtime INTEGER NOT NULL
)

CREATE TABLE w3mmdplayers (
	id INTEGER PRIMARY KEY,
	category TEXT NOT NULL,
	gameid INTEGER NOT NULL,
	pid INTEGER NOT NULL,
	name TEXT NOT NULL,
	flag TEXT NOT NULL,
	leaver INTEGER NOT NULL,
	practicing INTEGER NOT NULL
)

CREATE TABLE w3mmdvars (
	id INTEGER PRIMARY KEY,
	gameid INTEGER NOT NULL,
	pid INTEGER NOT NULL,
	varname TEXT NOT NULL,
	value_int INTEGER DEFAULT NULL,
	value_real REAL DEFAULT NULL,
	value_string TEXT DEFAULT NULL
)

CREATE TEMPORARY TABLE iptocountry (
	ip1 INTEGER NOT NULL,
	ip2 INTEGER NOT NULL,
	country TEXT NOT NULL,
	PRIMARY KEY ( ip1, ip2 )
)

CREATE INDEX idx_gameid ON gameplayers ( gameid )
CREATE INDEX idx_gameid_colour ON dotaplayers ( gameid, colour )

 **************
 *** SCHEMA ***
 **************/

//
// CSQLITE3 (wrapper class)
//

class CSQLITE3
{
private:
	void *m_DB;
	bool m_Ready;
	std::vector<std::string> m_Row;

public:
	CSQLITE3( std::string filename );
	~CSQLITE3( );

	bool GetReady( )			{ return m_Ready; }
	std::vector<std::string> *GetRow( )	{ return &m_Row; }
	std::string GetError( );

	int Prepare( std::string query, void **Statement );
	int Step( void *Statement );
	int Finalize( void *Statement );
	int Reset( void *Statement );
	int ClearBindings( void *Statement );
	int Exec( std::string query );
	uint32_t LastRowID( );
};

//
// CGHostDBSQLite
//

class CGHostDBSQLite : public CGHostDB
{
private:
	std::string m_File;
	CSQLITE3 *m_DB;

	// we keep some prepared statements in memory rather than recreating them each function call
	// this is an optimization because preparing statements takes time
	// however it only pays off if you're going to be using the statement extremely often

	void *FromAddStmt;

public:
	CGHostDBSQLite( CConfig *CFG );
	virtual ~CGHostDBSQLite( );

	virtual void Upgrade1_2( );
	virtual void Upgrade2_3( );
	virtual void Upgrade3_4( );
	virtual void Upgrade4_5( );
	virtual void Upgrade5_6( );
	virtual void Upgrade6_7( );
	virtual void Upgrade7_8( );

	virtual bool Begin( );
	virtual bool Commit( );
	virtual uint32_t AdminCount( std::string server );
	virtual bool AdminCheck( std::string server, std::string user );
	virtual bool AdminAdd( std::string server, std::string user );
	virtual bool AdminRemove( std::string server, std::string user );
	virtual std::vector<std::string> AdminList( std::string server );
	virtual uint32_t BanCount( std::string server );
	virtual CDBBan *BanCheck( std::string server, std::string user, std::string ip );
	virtual bool BanAdd( std::string server, std::string user, std::string ip, std::string gamename, std::string admin, std::string reason );
	virtual bool BanRemove( std::string server, std::string user );
	virtual bool BanRemove( std::string user );
	virtual std::vector<CDBBan *> BanList( std::string server );
	virtual uint32_t GameAdd( std::string server, std::string map, std::string gamename, std::string ownername, uint32_t duration, uint32_t gamestate, std::string creatorname, std::string creatorserver );
	virtual uint32_t GamePlayerAdd( uint32_t gameid, std::string name, std::string ip, uint32_t spoofed, std::string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, std::string leftreason, uint32_t team, uint32_t colour );
	virtual uint32_t GamePlayerCount( std::string name );
	virtual CDBGamePlayerSummary *GamePlayerSummaryCheck( std::string name );
	virtual uint32_t DotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
	virtual uint32_t DotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, std::string item1, std::string item2, std::string item3, std::string item4, std::string item5, std::string item6, std::string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills );
	virtual uint32_t DotAPlayerCount( std::string name );
	virtual CDBDotAPlayerSummary *DotAPlayerSummaryCheck( std::string name );
	virtual std::string FromCheck( uint32_t ip );
	virtual bool FromAdd( uint32_t ip1, uint32_t ip2, std::string country );
	virtual bool DownloadAdd( std::string map, uint32_t mapsize, std::string name, std::string ip, uint32_t spoofed, std::string spoofedrealm, uint32_t downloadtime );
	virtual uint32_t W3MMDPlayerAdd( std::string category, uint32_t gameid, uint32_t pid, std::string name, std::string flag, uint32_t leaver, uint32_t practicing );
	virtual bool W3MMDVarAdd( uint32_t gameid, std::map<VarP,int32_t> var_ints );
	virtual bool W3MMDVarAdd( uint32_t gameid, std::map<VarP,double> var_reals );
	virtual bool W3MMDVarAdd( uint32_t gameid, std::map<VarP,std::string> var_strings );

	// threaded database functions
	// note: these are not actually implemented with threads at the moment, they WILL block until the query is complete
	// todotodo: implement threads here

	virtual CCallableAdminCount *ThreadedAdminCount( std::string server );
	virtual CCallableAdminCheck *ThreadedAdminCheck( std::string server, std::string user );
	virtual CCallableAdminAdd *ThreadedAdminAdd( std::string server, std::string user );
	virtual CCallableAdminRemove *ThreadedAdminRemove( std::string server, std::string user );
	virtual CCallableAdminList *ThreadedAdminList( std::string server );
	virtual CCallableBanCount *ThreadedBanCount( std::string server );
	virtual CCallableBanCheck *ThreadedBanCheck( std::string server, std::string user, std::string ip );
	virtual CCallableBanAdd *ThreadedBanAdd( std::string server, std::string user, std::string ip, std::string gamename, std::string admin, std::string reason );
	virtual CCallableBanRemove *ThreadedBanRemove( std::string server, std::string user );
	virtual CCallableBanRemove *ThreadedBanRemove( std::string user );
	virtual CCallableBanList *ThreadedBanList( std::string server );
	virtual CCallableGameAdd *ThreadedGameAdd( std::string server, std::string map, std::string gamename, std::string ownername, uint32_t duration, uint32_t gamestate, std::string creatorname, std::string creatorserver );
	virtual CCallableGamePlayerAdd *ThreadedGamePlayerAdd( uint32_t gameid, std::string name, std::string ip, uint32_t spoofed, std::string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, std::string leftreason, uint32_t team, uint32_t colour );
	virtual CCallableGamePlayerSummaryCheck *ThreadedGamePlayerSummaryCheck( std::string name );
	virtual CCallableDotAGameAdd *ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
	virtual CCallableDotAPlayerAdd *ThreadedDotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, std::string item1, std::string item2, std::string item3, std::string item4, std::string item5, std::string item6, std::string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills );
	virtual CCallableDotAPlayerSummaryCheck *ThreadedDotAPlayerSummaryCheck( std::string name );
	virtual CCallableDownloadAdd *ThreadedDownloadAdd( std::string map, uint32_t mapsize, std::string name, std::string ip, uint32_t spoofed, std::string spoofedrealm, uint32_t downloadtime );
	virtual CCallableW3MMDPlayerAdd *ThreadedW3MMDPlayerAdd( std::string category, uint32_t gameid, uint32_t pid, std::string name, std::string flag, uint32_t leaver, uint32_t practicing );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, std::map<VarP,int32_t> var_ints );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, std::map<VarP,double> var_reals );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, std::map<VarP,std::string> var_strings );
};

#endif

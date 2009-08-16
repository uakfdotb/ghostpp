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

#ifndef MAP_H
#define MAP_H

#define MAPSPEED_SLOW			1
#define MAPSPEED_NORMAL			2
#define MAPSPEED_FAST			3

#define MAPVIS_HIDETERRAIN		1
#define MAPVIS_EXPLORED			2
#define MAPVIS_ALWAYSVISIBLE	3
#define MAPVIS_DEFAULT			4

#define MAPOBS_NONE				1
#define MAPOBS_ONDEFEAT			2
#define MAPOBS_ALLOWED			3
#define MAPOBS_REFEREES			4

#define MAPFLAG_TEAMSTOGETHER	1
#define MAPFLAG_FIXEDTEAMS		2
#define MAPFLAG_UNITSHARE		4
#define MAPFLAG_RANDOMHERO		8
#define MAPFLAG_RANDOMRACES		16

#include "gameslot.h"

//
// CMap
//

class CMap
{
public:
	CGHost *m_GHost;

private:
	bool m_Valid;
	string m_CFGFile;
	string m_MapPath;							// config value: map path
	BYTEARRAY m_MapSize;						// config value: map size (4 bytes)
	BYTEARRAY m_MapInfo;						// config value: map info (4 bytes) -> this is the real CRC
	BYTEARRAY m_MapCRC;							// config value: map crc (4 bytes) -> this is not the real CRC, it's the "xoro" value
	BYTEARRAY m_MapSHA1;						// config value: map sha1 (20 bytes)
	unsigned char m_MapSpeed;
	unsigned char m_MapVisibility;
	unsigned char m_MapObservers;
	unsigned char m_MapFlags;
	unsigned char m_MapGameType;
	BYTEARRAY m_MapWidth;						// config value: map width (2 bytes)
	BYTEARRAY m_MapHeight;						// config value: map height (2 bytes)
	string m_MapType;							// config value: map type (for stats class)
	string m_MapMatchMakingCategory;			// config value: map matchmaking category (for matchmaking)
	string m_MapStatsW3MMDCategory;				// config value: map stats w3mmd category (for saving w3mmd stats)
	string m_MapDefaultHCL;						// config value: map default HCL to use (this should really be specified elsewhere and not part of the map config)
	uint32_t m_MapDefaultPlayerScore;			// config value: map default player score (for matchmaking)
	string m_MapLocalPath;						// config value: map local path
	bool m_MapLoadInGame;
	string m_MapData;							// the map data itself, for sending the map to players
	uint32_t m_MapNumPlayers;
	uint32_t m_MapNumTeams;
	vector<CGameSlot> m_Slots;

public:
	CMap( CGHost *nGHost );
	CMap( CGHost *nGHost, CConfig *CFG, string nCFGFile );
	~CMap( );

	bool GetValid( )						{ return m_Valid; }
	string GetCFGFile( )					{ return m_CFGFile; }
	string GetMapPath( )					{ return m_MapPath; }
	BYTEARRAY GetMapSize( )					{ return m_MapSize; }
	BYTEARRAY GetMapInfo( )					{ return m_MapInfo; }
	BYTEARRAY GetMapCRC( )					{ return m_MapCRC; }
	BYTEARRAY GetMapSHA1( )					{ return m_MapSHA1; }
	unsigned char GetMapSpeed( )			{ return m_MapSpeed; }
	unsigned char GetMapVisibility( )		{ return m_MapVisibility; }
	unsigned char GetMapObservers( )		{ return m_MapObservers; }
	unsigned char GetMapFlags( )			{ return m_MapFlags; }
	BYTEARRAY GetMapGameFlags( );
	unsigned char GetMapGameType( )			{ return m_MapGameType; }
	BYTEARRAY GetMapWidth( )				{ return m_MapWidth; }
	BYTEARRAY GetMapHeight( )				{ return m_MapHeight; }
	string GetMapType( )					{ return m_MapType; }
	string GetMapMatchMakingCategory( )		{ return m_MapMatchMakingCategory; }
	string GetMapStatsW3MMDCategory( )		{ return m_MapStatsW3MMDCategory; }
	string GetMapDefaultHCL( )				{ return m_MapDefaultHCL; }
	uint32_t GetMapDefaultPlayerScore( )	{ return m_MapDefaultPlayerScore; }
	string GetMapLocalPath( )				{ return m_MapLocalPath; }
	bool GetMapLoadInGame( )				{ return m_MapLoadInGame; }
	string *GetMapData( )					{ return &m_MapData; }
	uint32_t GetMapNumPlayers( )			{ return m_MapNumPlayers; }
	uint32_t GetMapNumTeams( )				{ return m_MapNumTeams; }
	vector<CGameSlot> GetSlots( )			{ return m_Slots; }

	void Load( CConfig *CFG, string nCFGFile );
	void CheckValid( );
	uint32_t XORRotateLeft( unsigned char *data, uint32_t length );
};

#endif

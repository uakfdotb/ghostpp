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

#include "ghost.h"
#include "util.h"
#include "crc32.h"
#include "config.h"
#include "map.h"

#define __STORMLIB_SELF__
#include <stormlib/StormLib.h>

#define ROTL(x,n) ((x)<<(n))|((x)>>(32-(n)))	// this probably won't work with signed integers
#define ROTR(x,n) ((x)>>(n))|((x)<<(32-(n)))	// this probably won't work with signed integers

//
// CMap
//

CMap :: CMap( CGHost *nGHost )
{
	m_GHost = nGHost;
	m_MapPath = "Maps\\FrozenThrone\\(12)EmeraldGardens.w3x";
	m_MapSize = UTIL_ExtractNumbers( "174 221 4 0", 4 );
	m_MapInfo = UTIL_ExtractNumbers( "251 57 68 98", 4 );
	m_MapCRC = UTIL_ExtractNumbers( "112 185 65 97", 4 );
	m_MapSpeed = MAPSPEED_FAST;
	m_MapVisibility = MAPVIS_DEFAULT;
	m_MapObservers = MAPOBS_NONE;
	m_MapFlags = MAPFLAG_TEAMSTOGETHER | MAPFLAG_FIXEDTEAMS;
	m_MapGameType = 9;
	m_MapWidth = UTIL_ExtractNumbers( "172 0", 2 );
	m_MapHeight = UTIL_ExtractNumbers( "172 0", 2 );
	m_MapNumPlayers = 12;
	m_MapNumTeams = 12;
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 0, 0, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 1, 1, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 2, 2, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 3, 3, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 4, 4, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 5, 5, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 6, 6, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 7, 7, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 8, 8, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 9, 9, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 10, 10, SLOTRACE_RANDOM ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 11, 11, SLOTRACE_RANDOM ) );
}

CMap :: CMap( CGHost *nGHost, CConfig *CFG, string nCFGFile )
{
	m_GHost = nGHost;
	Load( CFG, nCFGFile );
}

CMap :: ~CMap( )
{

}

BYTEARRAY CMap :: GetMapGameFlags( )
{
	/*

	Speed: (mask 0x00000003) cannot be combined
		0x00000000 - Slow game speed
		0x00000001 - Normal game speed
		0x00000002 - Fast game speed
	Visibility: (mask 0x00000F00) cannot be combined
		0x00000100 - Hide terrain
		0x00000200 - Map explored
		0x00000400 - Always visible (no fog of war)
		0x00000800 - Default
	Observers/Referees: (mask 0x40003000) cannot be combined
		0x00000000 - No Observers
		0x00002000 - Observers on Defeat
		0x00003000 - Additional players as observer allowed
		0x40000000 - Referees
	Teams/Units/Hero/Race: (mask 0x07064000) can be combined
		0x00004000 - Teams Together (team members are placed at neighbored starting locations)
		0x00060000 - Fixed teams
		0x01000000 - Unit share
		0x02000000 - Random hero
		0x04000000 - Random races

	*/

	uint32_t GameFlags = 0;

	// speed

	if( m_MapSpeed == MAPSPEED_SLOW )
		GameFlags = 0x00000000;
	else if( m_MapSpeed == MAPSPEED_NORMAL )
		GameFlags = 0x00000001;
	else
		GameFlags = 0x00000002;

	// visibility

	if( m_MapVisibility == MAPVIS_HIDETERRAIN )
		GameFlags |= 0x00000100;
	else if( m_MapVisibility == MAPVIS_EXPLORED )
		GameFlags |= 0x00000200;
	else if( m_MapVisibility == MAPVIS_ALWAYSVISIBLE )
		GameFlags |= 0x00000400;
	else
		GameFlags |= 0x00000800;

	// observers

	if( m_MapObservers == MAPOBS_ONDEFEAT )
		GameFlags |= 0x00002000;
	else if( m_MapObservers == MAPOBS_ALLOWED )
		GameFlags |= 0x00003000;
	else if( m_MapObservers == MAPOBS_REFEREES )
		GameFlags |= 0x40000000;

	// teams/units/hero/race

	if( m_MapFlags & MAPFLAG_TEAMSTOGETHER )
		GameFlags |= 0x00004000;
	if( m_MapFlags & MAPFLAG_FIXEDTEAMS )
		GameFlags |= 0x00060000;
	if( m_MapFlags & MAPFLAG_UNITSHARE )
		GameFlags |= 0x01000000;
	if( m_MapFlags & MAPFLAG_RANDOMHERO )
		GameFlags |= 0x02000000;
	if( m_MapFlags & MAPFLAG_RANDOMRACES )
		GameFlags |= 0x04000000;

	return UTIL_CreateByteArray( GameFlags, false );
}

void CMap :: Load( CConfig *CFG, string nCFGFile )
{
	m_CFGFile = nCFGFile;

	// load the map data

	m_MapLocalPath = CFG->GetString( "map_localpath", string( ) );
	m_MapData.clear( );

	if( !m_MapLocalPath.empty( ) )
		m_MapData = UTIL_FileRead( m_GHost->m_MapPath + m_MapLocalPath );

	// load the map MPQ

	string MapMPQFileName = m_GHost->m_MapPath + m_MapLocalPath;
	HANDLE MapMPQ;
	bool MapMPQReady = false;

	if( SFileOpenArchive( MapMPQFileName.c_str( ), 0, 0, &MapMPQ ) )
	{
		CONSOLE_Print( "[MAP] loading MPQ file [" + MapMPQFileName + "]" );
		MapMPQReady = true;
	}
	else
		CONSOLE_Print( "[MAP] warning - unable to load MPQ file [" + MapMPQFileName + "]" );

	// try to calculate map_size, map_info, map_crc

	BYTEARRAY MapSize;
	BYTEARRAY MapInfo;
	BYTEARRAY MapCRC;

	if( !m_MapData.empty( ) )
	{
		// calculate map_size

		MapSize = UTIL_CreateByteArray( (uint32_t)m_MapData.size( ), false );
		CONSOLE_Print( "[MAP] calculated map_size = " + UTIL_ToString( MapSize[0] ) + " " + UTIL_ToString( MapSize[1] ) + " " + UTIL_ToString( MapSize[2] ) + " " + UTIL_ToString( MapSize[3] ) );

		// calculate map_info (this is actually the CRC)

		MapInfo = UTIL_CreateByteArray( (uint32_t)m_GHost->m_CRC->FullCRC( (unsigned char *)m_MapData.c_str( ), m_MapData.size( ) ), false );
		CONSOLE_Print( "[MAP] calculated map_info = " + UTIL_ToString( MapInfo[0] ) + " " + UTIL_ToString( MapInfo[1] ) + " " + UTIL_ToString( MapInfo[2] ) + " " + UTIL_ToString( MapInfo[3] ) );

		// calculate map_crc (this is not the CRC)
		// a big thank you to Strilanc for figuring this out

		string CommonJ = UTIL_FileRead( m_GHost->m_MapCFGPath + "common.j" );

		if( CommonJ.empty( ) )
			CONSOLE_Print( "[MAP] unable to calculate map_crc - unable to read file [" + m_GHost->m_MapCFGPath + "common.j]" );
		else
		{
			string BlizzardJ = UTIL_FileRead( m_GHost->m_MapCFGPath + "blizzard.j" );

			if( BlizzardJ.empty( ) )
				CONSOLE_Print( "[MAP] unable to calculate map_crc - unable to read file [" + m_GHost->m_MapCFGPath + "blizzard.j]" );
			else
			{
				uint32_t Val = 0;
				Val = Val ^ XORRotateLeft( (unsigned char *)CommonJ.c_str( ), CommonJ.size( ) );
				// DEBUG_Print( UTIL_CreateByteArray( Val, false ) );
				Val = Val ^ XORRotateLeft( (unsigned char *)BlizzardJ.c_str( ), BlizzardJ.size( ) );
				// DEBUG_Print( UTIL_CreateByteArray( Val, false ) );
				Val = ROTL( Val, 3 );
				Val = ROTL( Val ^ 0x03F1379E, 3 );
				// DEBUG_Print( UTIL_CreateByteArray( Val, false ) );

				if( MapMPQReady )
				{
					vector<string> FileList;
					FileList.push_back( "war3map.j" );
					FileList.push_back( "scripts\\war3map.j" );
					FileList.push_back( "war3map.w3e" );
					FileList.push_back( "war3map.wpm" );
					FileList.push_back( "war3map.doo" );
					FileList.push_back( "war3map.w3u" );
					FileList.push_back( "war3map.w3b" );
					FileList.push_back( "war3map.w3d" );
					FileList.push_back( "war3map.w3a" );
					FileList.push_back( "war3map.w3q" );
					bool FoundScript = false;

					for( vector<string> :: iterator i = FileList.begin( ); i != FileList.end( ); i++ )
					{
						HANDLE SubFile;

						if( SFileOpenFileEx( MapMPQ, (*i).c_str( ), 0, &SubFile ) )
						{
							uint32_t FileLength = SFileGetFileSize( SubFile, NULL );

							if( FileLength > 0 && FileLength != 0xFFFFFFFF )
							{
								char *SubFileData = new char[FileLength];
								DWORD BytesRead = 0;

								if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead ) )
								{
									if( *i == "war3map.j" || *i == "scripts\\war3map.j" )
										FoundScript = true;

									Val = ROTL( Val ^ XORRotateLeft( (unsigned char *)SubFileData, BytesRead ), 3 );
									// DEBUG_Print( "*** found: " + *i );
									// DEBUG_Print( UTIL_CreateByteArray( Val, false ) );
								}

								delete [] SubFileData;
							}

							SFileCloseFile( SubFile );
						}
						else
						{
							// DEBUG_Print( "*** not found: " + *i );
						}
					}

					if( !FoundScript )
						CONSOLE_Print( "[MAP] couldn't find war3map.j or scripts\\war3map.j in MPQ file, calculated map_crc is probably wrong" );

					MapCRC = UTIL_CreateByteArray( Val, false );
					CONSOLE_Print( "[MAP] calculated map_crc = " + UTIL_ToString( MapCRC[0] ) + " " + UTIL_ToString( MapCRC[1] ) + " " + UTIL_ToString( MapCRC[2] ) + " " + UTIL_ToString( MapCRC[3] ) );
				}
				else
					CONSOLE_Print( "[MAP] unable to calculate map_crc - map MPQ file not loaded" );
			}
		}
	}
	else
		CONSOLE_Print( "[MAP] no map data available, using config file for map_size, map_info, map_crc" );

	// try to calculate map_width, map_height, map_slot<x>, map_numplayers, map_numteams

	BYTEARRAY MapWidth;
	BYTEARRAY MapHeight;
	uint32_t MapNumPlayers = 0;
	uint32_t MapNumTeams = 0;
	vector<CGameSlot> Slots;

	if( !m_MapData.empty( ) )
	{
		if( MapMPQReady )
		{
			HANDLE SubFile;

			if( SFileOpenFileEx( MapMPQ, "war3map.w3i", 0, &SubFile ) )
			{
				uint32_t FileLength = SFileGetFileSize( SubFile, NULL );

				if( FileLength > 0 && FileLength != 0xFFFFFFFF )
				{
					char *SubFileData = new char[FileLength];
					DWORD BytesRead = 0;

					if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead ) )
					{
						istringstream ISS( string( SubFileData, BytesRead ) );

						// war3map.w3i format found at http://www.wc3campaigns.net/tools/specs/index.html by Zepir/PitzerMike

						string GarbageString;
						uint32_t FileFormat;
						uint32_t RawMapWidth;
						uint32_t RawMapHeight;
						uint32_t RawMapFlags;
						uint32_t RawMapNumPlayers;
						uint32_t RawMapNumTeams;

						ISS.read( (char *)&FileFormat, 4 );				// file format (18 = ROC, 25 = TFT)

						if( FileFormat == 18 || FileFormat == 25 )
						{
							ISS.seekg( 4, ios :: cur );					// number of saves
							ISS.seekg( 4, ios :: cur );					// editor version
							getline( ISS, GarbageString, '\0' );		// map name
							getline( ISS, GarbageString, '\0' );		// map author
							getline( ISS, GarbageString, '\0' );		// map description
							getline( ISS, GarbageString, '\0' );		// players recommended
							ISS.seekg( 32, ios :: cur );				// camera bounds
							ISS.seekg( 16, ios :: cur );				// camera bounds complements
							ISS.read( (char *)&RawMapWidth, 4 );		// map width
							ISS.read( (char *)&RawMapHeight, 4 );		// map height
							ISS.read( (char *)&RawMapFlags, 4 );		// flags
							ISS.seekg( 1, ios :: cur );					// map main ground type

							if( FileFormat == 18 )
								ISS.seekg( 4, ios :: cur );				// campaign background number
							else if( FileFormat == 25 )
							{
								ISS.seekg( 4, ios :: cur );				// loading screen background number
								getline( ISS, GarbageString, '\0' );	// path of custom loading screen model
							}

							getline( ISS, GarbageString, '\0' );		// map loading screen text
							getline( ISS, GarbageString, '\0' );		// map loading screen title
							getline( ISS, GarbageString, '\0' );		// map loading screen subtitle

							if( FileFormat == 18 )
								ISS.seekg( 4, ios :: cur );				// map loading screen number
							else if( FileFormat == 25 )
							{
								ISS.seekg( 4, ios :: cur );				// used game data set
								getline( ISS, GarbageString, '\0' );	// prologue screen path
							}

							getline( ISS, GarbageString, '\0' );		// prologue screen text
							getline( ISS, GarbageString, '\0' );		// prologue screen title
							getline( ISS, GarbageString, '\0' );		// prologue screen subtitle

							if( FileFormat == 25 )
							{
								ISS.seekg( 4, ios :: cur );				// uses terrain fog
								ISS.seekg( 4, ios :: cur );				// fog start z height
								ISS.seekg( 4, ios :: cur );				// fog end z height
								ISS.seekg( 4, ios :: cur );				// fog density
								ISS.seekg( 1, ios :: cur );				// fog red value
								ISS.seekg( 1, ios :: cur );				// fog green value
								ISS.seekg( 1, ios :: cur );				// fog blue value
								ISS.seekg( 1, ios :: cur );				// fog alpha value
								ISS.seekg( 4, ios :: cur );				// global weather id
								getline( ISS, GarbageString, '\0' );	// custom sound environment
								ISS.seekg( 1, ios :: cur );				// tileset id of the used custom light environment
								ISS.seekg( 1, ios :: cur );				// custom water tinting red value
								ISS.seekg( 1, ios :: cur );				// custom water tinting green value
								ISS.seekg( 1, ios :: cur );				// custom water tinting blue value
								ISS.seekg( 1, ios :: cur );				// custom water tinting alpha value
							}

							ISS.read( (char *)&RawMapNumPlayers, 4 );	// number of players

							for( uint32_t i = 0; i < RawMapNumPlayers; i++ )
							{
								CGameSlot Slot( 0, 255, SLOTSTATUS_OPEN, 0, 0, 1, SLOTRACE_RANDOM );
								uint32_t Colour;
								uint32_t Status;
								uint32_t Race;

								ISS.read( (char *)&Colour, 4 );			// colour
								Slot.SetColour( Colour );
								ISS.read( (char *)&Status, 4 );			// status

								if( Status == 1 )
									Slot.SetSlotStatus( SLOTSTATUS_OPEN );
								else if( Status == 2 )
								{
									Slot.SetSlotStatus( SLOTSTATUS_OCCUPIED );
									Slot.SetComputer( 1 );
									Slot.SetComputerType( SLOTCOMP_NORMAL );
								}
								else
									Slot.SetSlotStatus( SLOTSTATUS_CLOSED );

								ISS.read( (char *)&Race, 4 );			// race

								if( Race == 1 )
									Slot.SetRace( SLOTRACE_HUMAN );
								else if( Race == 2 )
									Slot.SetRace( SLOTRACE_ORC );
								else if( Race == 3 )
									Slot.SetRace( SLOTRACE_UNDEAD );
								else if( Race == 4 )
									Slot.SetRace( SLOTRACE_NIGHTELF );
								else
									Slot.SetRace( SLOTRACE_RANDOM );

								ISS.seekg( 4, ios :: cur );				// fixed start position
								getline( ISS, GarbageString, '\0' );	// player name
								ISS.seekg( 4, ios :: cur );				// start position x
								ISS.seekg( 4, ios :: cur );				// start position y
								ISS.seekg( 4, ios :: cur );				// ally low priorities
								ISS.seekg( 4, ios :: cur );				// ally high priorities

								Slots.push_back( Slot );
							}

							ISS.read( (char *)&RawMapNumTeams, 4 );		// number of teams

							for( uint32_t i = 0; i < RawMapNumTeams; i++ )
							{
								uint32_t Flags;
								uint32_t PlayerMask;

								ISS.read( (char *)&Flags, 4 );			// flags
								ISS.read( (char *)&PlayerMask, 4 );		// player mask

								for( unsigned char j = 0; j < 12; j++ )
								{
									if( PlayerMask & 1 )
									{
										for( vector<CGameSlot> :: iterator k = Slots.begin( ); k != Slots.end( ); k++ )
										{
											if( (*k).GetColour( ) == j )
												(*k).SetTeam( i );
										}
									}

									PlayerMask >>= 1;
								}

								getline( ISS, GarbageString, '\0' );	// team name
							}

							MapWidth = UTIL_CreateByteArray( (uint16_t)RawMapWidth, false );
							CONSOLE_Print( "[MAP] calculated map_width = " + UTIL_ToString( MapWidth[0] ) + " " + UTIL_ToString( MapWidth[1] ) );
							MapHeight = UTIL_CreateByteArray( (uint16_t)RawMapHeight, false );
							CONSOLE_Print( "[MAP] calculated map_height = " + UTIL_ToString( MapHeight[0] ) + " " + UTIL_ToString( MapHeight[1] ) );
							MapNumPlayers = RawMapNumPlayers;
							CONSOLE_Print( "[MAP] calculated map_numplayers = " + UTIL_ToString( MapNumPlayers ) );
							MapNumTeams = RawMapNumTeams;
							CONSOLE_Print( "[MAP] calculated map_numteams = " + UTIL_ToString( MapNumTeams ) );
							CONSOLE_Print( "[MAP] found " + UTIL_ToString( Slots.size( ) ) + " slots" );

							/* for( vector<CGameSlot> :: iterator i = Slots.begin( ); i != Slots.end( ); i++ )
								DEBUG_Print( (*i).GetByteArray( ) ); */

							// if it's a melee map...

							if( RawMapFlags & 4 )
							{
								CONSOLE_Print( "[MAP] found melee map, initializing slots and setting map_numteams = map_numplayers" );

								// give each slot a different team and set the race to random

								unsigned char Team = 0;

								for( vector<CGameSlot> :: iterator i = Slots.begin( ); i != Slots.end( ); i++ )
								{
									(*i).SetTeam( Team++ );
									(*i).SetRace( SLOTRACE_RANDOM );
								}

								// and set numteams = numplayers because numteams doesn't seem to be a meaningful value in melee maps

								MapNumTeams = MapNumPlayers;
							}
						}
					}
					else
						CONSOLE_Print( "[MAP] unable to calculate map_width, map_height, map_slot<x>, map_numplayers, map_numteams - unable to extract war3map.w3i from MPQ file" );

					delete [] SubFileData;
				}

				SFileCloseFile( SubFile );
			}
			else
				CONSOLE_Print( "[MAP] unable to calculate map_width, map_height, map_slot<x>, map_numplayers, map_numteams - couldn't find war3map.w3i in MPQ file" );
		}
		else
			CONSOLE_Print( "[MAP] unable to calculate map_width, map_height, map_slot<x>, map_numplayers, map_numteams - map MPQ file not loaded" );
	}
	else
		CONSOLE_Print( "[MAP] no map data available, using config file for map_width, map_height, map_slot<x>, map_numplayers, map_numteams" );

	// close the map MPQ

	if( MapMPQReady )
		SFileCloseArchive( MapMPQ );

	m_MapPath = CFG->GetString( "map_path", "Maps\\Download\\DotA Allstars v6.54b.w3x" );

	if( MapSize.empty( ) )
		MapSize = UTIL_ExtractNumbers( CFG->GetString( "map_size", "193 236 49 0" ), 4 );
	else if( CFG->Exists( "map_size" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_size with config value map_size = " + CFG->GetString( "map_size", string( ) ) );
		MapSize = UTIL_ExtractNumbers( CFG->GetString( "map_size", string( ) ), 4 );
	}

	m_MapSize = MapSize;

	if( MapInfo.empty( ) )
		MapInfo = UTIL_ExtractNumbers( CFG->GetString( "map_info", "114 75 121 60" ), 4 );
	else if( CFG->Exists( "map_info" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_info with config value map_info = " + CFG->GetString( "map_info", string( ) ) );
		MapInfo = UTIL_ExtractNumbers( CFG->GetString( "map_info", string( ) ), 4 );
	}

	m_MapInfo = MapInfo;

	if( MapCRC.empty( ) )
		MapCRC = UTIL_ExtractNumbers( CFG->GetString( "map_crc", "120 227 167 121" ), 4 );
	else if( CFG->Exists( "map_crc" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_crc with config value map_crc = " + CFG->GetString( "map_crc", string( ) ) );
		MapCRC = UTIL_ExtractNumbers( CFG->GetString( "map_crc", string( ) ), 4 );
	}

	m_MapCRC = MapCRC;
	m_MapSpeed = CFG->GetInt( "map_speed", MAPSPEED_FAST );
	m_MapVisibility = CFG->GetInt( "map_visibility", MAPVIS_DEFAULT );
	m_MapObservers = CFG->GetInt( "map_observers", MAPOBS_NONE );
	m_MapFlags = CFG->GetInt( "map_flags", MAPFLAG_TEAMSTOGETHER | MAPFLAG_FIXEDTEAMS );
	m_MapGameType = CFG->GetInt( "map_gametype", 1 );

	if( MapWidth.empty( ) )
		MapWidth = UTIL_ExtractNumbers( CFG->GetString( "map_width", "116 0" ), 2 );
	else if( CFG->Exists( "map_width" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_width with config value map_width = " + CFG->GetString( "map_width", string( ) ) );
		MapWidth = UTIL_ExtractNumbers( CFG->GetString( "map_width", string( ) ), 2 );
	}

	m_MapWidth = MapWidth;

	if( MapHeight.empty( ) )
		MapHeight = UTIL_ExtractNumbers( CFG->GetString( "map_height", "116 0" ), 2 );
	else if( CFG->Exists( "map_height" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_height with config value map_height = " + CFG->GetString( "map_height", string( ) ) );
		MapHeight = UTIL_ExtractNumbers( CFG->GetString( "map_height", string( ) ), 2 );
	}

	m_MapHeight = MapHeight;
	m_MapType = CFG->GetString( "map_type", string( ) );

	if( MapNumPlayers == 0 )
		MapNumPlayers = CFG->GetInt( "map_numplayers", 10 );
	else if( CFG->Exists( "map_numplayers" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_numplayers with config value map_numplayers = " + CFG->GetString( "map_numplayers", string( ) ) );
		MapNumPlayers = CFG->GetInt( "map_numplayers", 0 );
	}

	m_MapNumPlayers = MapNumPlayers;

	if( MapNumTeams == 0 )
		MapNumTeams = CFG->GetInt( "map_numteams", 2 );
	else if( CFG->Exists( "map_numteams" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_numteams with config value map_numteams = " + CFG->GetString( "map_numteams", string( ) ) );
		MapNumTeams = CFG->GetInt( "map_numteams", 0 );
	}

	m_MapNumTeams = MapNumTeams;

	if( Slots.empty( ) )
	{
		for( uint32_t Slot = 1; Slot <= 12; Slot++ )
		{
			string SlotString = CFG->GetString( "map_slot" + UTIL_ToString( Slot ), string( ) );

			if( SlotString.empty( ) )
				break;

			BYTEARRAY SlotData = UTIL_ExtractNumbers( SlotString, 9 );
			Slots.push_back( CGameSlot( SlotData ) );
		}
	}
	else if( CFG->Exists( "map_slot1" ) )
	{
		CONSOLE_Print( "[MAP] overriding slots" );
		Slots.clear( );

		for( uint32_t Slot = 1; Slot <= 12; Slot++ )
		{
			string SlotString = CFG->GetString( "map_slot" + UTIL_ToString( Slot ), string( ) );

			if( SlotString.empty( ) )
				break;

			BYTEARRAY SlotData = UTIL_ExtractNumbers( SlotString, 9 );
			Slots.push_back( CGameSlot( SlotData ) );
		}
	}

	m_Slots = Slots;

	if( m_Slots.empty( ) )
	{
		// no slots found, use the DotA 6.54b default slot structure

		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 0, 1, SLOTRACE_NIGHTELF ) );
		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 0, 2, SLOTRACE_NIGHTELF ) );
		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 0, 3, SLOTRACE_NIGHTELF ) );
		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 0, 4, SLOTRACE_NIGHTELF ) );
		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 0, 5, SLOTRACE_NIGHTELF ) );
		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 1, 7, SLOTRACE_UNDEAD ) );
		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 1, 8, SLOTRACE_UNDEAD ) );
		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 1, 9, SLOTRACE_UNDEAD ) );
		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 1, 10, SLOTRACE_UNDEAD ) );
		m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 1, 11, SLOTRACE_UNDEAD ) );
	}

	// if random races is set force every slot's race to random + fixed

	if( m_MapFlags & MAPFLAG_RANDOMRACES )
	{
		CONSOLE_Print( "[MAP] forcing races to random" );

		for( vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
			(*i).SetRace( SLOTRACE_RANDOM | SLOTRACE_FIXED );
	}

	// add observer slots

	if( m_MapObservers == MAPOBS_ALLOWED || m_MapObservers == MAPOBS_REFEREES )
	{
		CONSOLE_Print( "[MAP] adding " + UTIL_ToString( 12 - m_Slots.size( ) ) + " observer slots" );

		while( m_Slots.size( ) < 12 )
			m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 12, 12, SLOTRACE_RANDOM ) );
	}
}

uint32_t CMap :: XORRotateLeft( unsigned char *data, uint32_t length )
{
	// a big thank you to Strilanc for figuring this out

	uint32_t i = 0;
	uint32_t Val = 0;

	if( length > 3 )
	{
		while( i < length - 3 )
		{
			Val = ROTL( Val ^ ( (uint32_t)data[i] + (uint32_t)( data[i + 1] << 8 ) + (uint32_t)( data[i + 2] << 16 ) + (uint32_t)( data[i + 3] << 24 ) ), 3 );
			i += 4;
		}
	}

	while( i < length )
	{
		Val = ROTL( Val ^ data[i], 3 );
		i++;
	}

	return Val;
}

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
#include "sha1.h"
#include "config.h"
#include "map.h"

#define __STORMLIB_SELF__
#include <src/StormLib.h>

#define ROTL(x,n) ((x)<<(n))|((x)>>(32-(n)))	// this won't work with signed types
#define ROTR(x,n) ((x)>>(n))|((x)<<(32-(n)))	// this won't work with signed types

//
// CMap
//

CMap :: CMap( CGHost *nGHost ) : m_GHost( nGHost ), m_Valid( true ), m_MapPath( "Maps\\FrozenThrone\\(12)EmeraldGardens.w3x" ), m_MapSize( UTIL_ExtractNumbers( "174 221 4 0", 4 ) ), m_MapInfo( UTIL_ExtractNumbers( "251 57 68 98", 4 ) ), m_MapCRC( UTIL_ExtractNumbers( "108 250 204 59", 4 ) ), m_MapSHA1( UTIL_ExtractNumbers( "35 81 104 182 223 63 204 215 1 17 87 234 220 66 3 185 82 99 6 13", 20 ) ), m_MapSpeed( MAPSPEED_FAST ), m_MapVisibility( MAPVIS_DEFAULT ), m_MapObservers( MAPOBS_NONE ), m_MapFlags( MAPFLAG_TEAMSTOGETHER | MAPFLAG_FIXEDTEAMS ), m_MapFilterMaker( MAPFILTER_MAKER_BLIZZARD ), m_MapFilterType( MAPFILTER_TYPE_MELEE ), m_MapFilterSize( MAPFILTER_SIZE_LARGE ), m_MapFilterObs( MAPFILTER_OBS_NONE ), m_MapOptions( MAPOPT_MELEE ), m_MapWidth( UTIL_ExtractNumbers( "172 0", 2 ) ), m_MapHeight( UTIL_ExtractNumbers( "172 0", 2 ) ), m_MapLoadInGame( false ), m_MapNumPlayers( 12 ), m_MapNumTeams( 12 )
{
	CONSOLE_Print( "[MAP] using hardcoded Emerald Gardens map data for Warcraft 3 version 1.24 & 1.24b" );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 0, 0, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 1, 1, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 2, 2, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 3, 3, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 4, 4, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 5, 5, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 6, 6, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 7, 7, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 8, 8, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 9, 9, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 10, 10, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
	m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, 11, 11, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE ) );
}

CMap :: CMap( CGHost *nGHost, CConfig *CFG, std::string nCFGFile ) : m_GHost( nGHost )
{
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

uint32_t CMap :: GetMapGameType( )
{
	/* spec stolen from Strilanc as follows:

    Public Enum GameTypes As UInteger
        None = 0
        Unknown0 = 1 << 0 '[always seems to be set?]

        '''<summary>Setting this bit causes wc3 to check the map and disc if it is not signed by Blizzard</summary>
        AuthenticatedMakerBlizzard = 1 << 3
        OfficialMeleeGame = 1 << 5

		SavedGame = 1 << 9
        PrivateGame = 1 << 11

        MakerUser = 1 << 13
        MakerBlizzard = 1 << 14
        TypeMelee = 1 << 15
        TypeScenario = 1 << 16
        SizeSmall = 1 << 17
        SizeMedium = 1 << 18
        SizeLarge = 1 << 19
        ObsFull = 1 << 20
        ObsOnDeath = 1 << 21
        ObsNone = 1 << 22

        MaskObs = ObsFull Or ObsOnDeath Or ObsNone
        MaskMaker = MakerBlizzard Or MakerUser
        MaskType = TypeMelee Or TypeScenario
        MaskSize = SizeLarge Or SizeMedium Or SizeSmall
        MaskFilterable = MaskObs Or MaskMaker Or MaskType Or MaskSize
    End Enum

	*/

	// note: we allow "conflicting" flags to be set at the same time (who knows if this is a good idea)
	// we also don't set any flags this class is unaware of such as Unknown0, SavedGame, and PrivateGame

	uint32_t GameType = 0;

	// maker

	if( m_MapFilterMaker & MAPFILTER_MAKER_USER )
		GameType |= MAPGAMETYPE_MAKERUSER;
	if( m_MapFilterMaker & MAPFILTER_MAKER_BLIZZARD )
		GameType |= MAPGAMETYPE_MAKERBLIZZARD;

	// type

	if( m_MapFilterType & MAPFILTER_TYPE_MELEE )
		GameType |= MAPGAMETYPE_TYPEMELEE;
	if( m_MapFilterType & MAPFILTER_TYPE_SCENARIO )
		GameType |= MAPGAMETYPE_TYPESCENARIO;

	// size

	if( m_MapFilterSize & MAPFILTER_SIZE_SMALL )
		GameType |= MAPGAMETYPE_SIZESMALL;
	if( m_MapFilterSize & MAPFILTER_SIZE_MEDIUM )
		GameType |= MAPGAMETYPE_SIZEMEDIUM;
	if( m_MapFilterSize & MAPFILTER_SIZE_LARGE )
		GameType |= MAPGAMETYPE_SIZELARGE;

	// obs

	if( m_MapFilterObs & MAPFILTER_OBS_FULL )
		GameType |= MAPGAMETYPE_OBSFULL;
	if( m_MapFilterObs & MAPFILTER_OBS_ONDEATH )
		GameType |= MAPGAMETYPE_OBSONDEATH;
	if( m_MapFilterObs & MAPFILTER_OBS_NONE )
		GameType |= MAPGAMETYPE_OBSNONE;

	return GameType;
}

unsigned char CMap :: GetMapLayoutStyle( )
{
	// 0 = melee
	// 1 = custom forces
	// 2 = fixed player settings (not possible with the Warcraft III map editor)
	// 3 = custom forces + fixed player settings

	if( !( m_MapOptions & MAPOPT_CUSTOMFORCES ) )
		return 0;

	if( !( m_MapOptions & MAPOPT_FIXEDPLAYERSETTINGS ) )
		return 1;

	return 3;
}

void CMap :: Load( CConfig *CFG, std::string nCFGFile )
{
	m_Valid = true;
	m_CFGFile = nCFGFile;

	// load the map data

	m_MapLocalPath = CFG->GetString( "map_localpath", std::string( ) );
	m_MapData.clear( );

	if( !m_MapLocalPath.empty( ) )
		m_MapData = UTIL_FileRead( m_GHost->m_MapPath + m_MapLocalPath );

	// load the map MPQ

	std::string MapMPQFileName = m_GHost->m_MapPath + m_MapLocalPath;
	HANDLE MapMPQ;
	bool MapMPQReady = false;

	if( SFileOpenArchive( MapMPQFileName.c_str( ), 0, MPQ_OPEN_FORCE_MPQ_V1, &MapMPQ ) )
	{
		CONSOLE_Print( "[MAP] loading MPQ file [" + MapMPQFileName + "]" );
		MapMPQReady = true;
	}
	else
		CONSOLE_Print( "[MAP] warning - unable to load MPQ file [" + MapMPQFileName + "]" );

	// try to calculate map_size, map_info, map_crc, map_sha1

	BYTEARRAY MapSize;
	BYTEARRAY MapInfo;
	BYTEARRAY MapCRC;
	BYTEARRAY MapSHA1;

	if( !m_MapData.empty( ) )
	{
		m_GHost->m_SHA->Reset( );

		// calculate map_size

		MapSize = UTIL_CreateByteArray( (uint32_t)m_MapData.size( ), false );
		CONSOLE_Print( "[MAP] calculated map_size = " + UTIL_ByteArrayToDecString( MapSize ) );

		// calculate map_info (this is actually the CRC)

		MapInfo = UTIL_CreateByteArray( (uint32_t)m_GHost->m_CRC->FullCRC( (unsigned char *)m_MapData.c_str( ), m_MapData.size( ) ), false );
		CONSOLE_Print( "[MAP] calculated map_info = " + UTIL_ByteArrayToDecString( MapInfo ) );

		// calculate map_crc (this is not the CRC) and map_sha1
		// a big thank you to Strilanc for figuring the map_crc algorithm out

		std::string CommonJ = UTIL_FileRead( m_GHost->m_MapCFGPath + "common.j" );

		if( CommonJ.empty( ) )
			CONSOLE_Print( "[MAP] unable to calculate map_crc/sha1 - unable to read file [" + m_GHost->m_MapCFGPath + "common.j]" );
		else
		{
			std::string BlizzardJ = UTIL_FileRead( m_GHost->m_MapCFGPath + "blizzard.j" );

			if( BlizzardJ.empty( ) )
				CONSOLE_Print( "[MAP] unable to calculate map_crc/sha1 - unable to read file [" + m_GHost->m_MapCFGPath + "blizzard.j]" );
			else
			{
				uint32_t Val = 0;

				// update: it's possible for maps to include their own copies of common.j and/or blizzard.j
				// this code now overrides the default copies if required

				bool OverrodeCommonJ = false;
				bool OverrodeBlizzardJ = false;

				if( MapMPQReady )
				{
					HANDLE SubFile;

					// override common.j

					if( SFileOpenFileEx( MapMPQ, "Scripts\\common.j", 0, &SubFile ) )
					{
						uint32_t FileLength = SFileGetFileSize( SubFile, NULL );

						if( FileLength > 0 && FileLength != 0xFFFFFFFF )
						{
							char *SubFileData = new char[FileLength];
							DWORD BytesRead = 0;

							if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead,0) )
							{
								CONSOLE_Print( "[MAP] overriding default common.j with map copy while calculating map_crc/sha1" );
								OverrodeCommonJ = true;
								Val = Val ^ XORRotateLeft( (unsigned char *)SubFileData, BytesRead );
								m_GHost->m_SHA->Update( (unsigned char *)SubFileData, BytesRead );
							}

							delete [] SubFileData;
						}

						SFileCloseFile( SubFile );
					}
				}

				if( !OverrodeCommonJ )
				{
					Val = Val ^ XORRotateLeft( (unsigned char *)CommonJ.c_str( ), CommonJ.size( ) );
					m_GHost->m_SHA->Update( (unsigned char *)CommonJ.c_str( ), CommonJ.size( ) );
				}

				if( MapMPQReady )
				{
					HANDLE SubFile;

					// override blizzard.j

					if( SFileOpenFileEx( MapMPQ, "Scripts\\blizzard.j", 0, &SubFile ) )
					{
						uint32_t FileLength = SFileGetFileSize( SubFile, NULL );

						if( FileLength > 0 && FileLength != 0xFFFFFFFF )
						{
							char *SubFileData = new char[FileLength];
							DWORD BytesRead = 0;

							if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead,0) )
							{
								CONSOLE_Print( "[MAP] overriding default blizzard.j with map copy while calculating map_crc/sha1" );
								OverrodeBlizzardJ = true;
								Val = Val ^ XORRotateLeft( (unsigned char *)SubFileData, BytesRead );
								m_GHost->m_SHA->Update( (unsigned char *)SubFileData, BytesRead );
							}

							delete [] SubFileData;
						}

						SFileCloseFile( SubFile );
					}
				}

				if( !OverrodeBlizzardJ )
				{
					Val = Val ^ XORRotateLeft( (unsigned char *)BlizzardJ.c_str( ), BlizzardJ.size( ) );
					m_GHost->m_SHA->Update( (unsigned char *)BlizzardJ.c_str( ), BlizzardJ.size( ) );
				}

				Val = ROTL( Val, 3 );
				Val = ROTL( Val ^ 0x03F1379E, 3 );
				m_GHost->m_SHA->Update( (unsigned char *)"\x9E\x37\xF1\x03", 4 );

				if( MapMPQReady )
				{
					std::vector<std::string> FileList;
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

					for( std::vector<std::string> :: iterator i = FileList.begin( ); i != FileList.end( ); ++i )
					{
						// don't use scripts\war3map.j if we've already used war3map.j (yes, some maps have both but only war3map.j is used)

						if( FoundScript && *i == "scripts\\war3map.j" )
							continue;

						HANDLE SubFile;

						if( SFileOpenFileEx( MapMPQ, (*i).c_str( ), 0, &SubFile ) )
						{
							uint32_t FileLength = SFileGetFileSize( SubFile, NULL );

							if( FileLength > 0 && FileLength != 0xFFFFFFFF )
							{
								char *SubFileData = new char[FileLength];
								DWORD BytesRead = 0;

								if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead,0 ) )
								{
									if( *i == "war3map.j" || *i == "scripts\\war3map.j" )
										FoundScript = true;

									Val = ROTL( Val ^ XORRotateLeft( (unsigned char *)SubFileData, BytesRead ), 3 );
									m_GHost->m_SHA->Update( (unsigned char *)SubFileData, BytesRead );
									// DEBUG_Print( "*** found: " + *i );
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
						CONSOLE_Print( "[MAP] couldn't find war3map.j or scripts\\war3map.j in MPQ file, calculated map_crc/sha1 is probably wrong" );

					MapCRC = UTIL_CreateByteArray( Val, false );
					CONSOLE_Print( "[MAP] calculated map_crc = " + UTIL_ByteArrayToDecString( MapCRC ) );

					m_GHost->m_SHA->Final( );
					unsigned char SHA1[20];
					memset( SHA1, 0, sizeof( unsigned char ) * 20 );
					m_GHost->m_SHA->GetHash( SHA1 );
					MapSHA1 = UTIL_CreateByteArray( SHA1, 20 );
					CONSOLE_Print( "[MAP] calculated map_sha1 = " + UTIL_ByteArrayToDecString( MapSHA1 ) );
				}
				else
					CONSOLE_Print( "[MAP] unable to calculate map_crc/sha1 - map MPQ file not loaded" );
			}
		}
	}
	else
		CONSOLE_Print( "[MAP] no map data available, using config file for map_size, map_info, map_crc, map_sha1" );

	// try to calculate map_width, map_height, map_slot<x>, map_numplayers, map_numteams

	uint32_t EditorVersion; // used to determine maximum slots when adding observers
	uint32_t MapOptions = 0;
	BYTEARRAY MapWidth;
	BYTEARRAY MapHeight;
	uint32_t MapNumPlayers = 0;
	uint32_t MapNumTeams = 0;
	uint32_t MapFilterType = MAPFILTER_TYPE_SCENARIO;
	std::vector<CGameSlot> Slots;

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

					if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead ,0) )
					{
						std::istringstream ISS( std::string( SubFileData, BytesRead ) );

						// war3map.w3i format found at http://www.wc3campaigns.net/tools/specs/index.html by Zepir/PitzerMike

						std::string GarbageString;
						uint32_t FileFormat;
						uint32_t RawMapWidth;
						uint32_t RawMapHeight;
						uint32_t RawMapFlags;
						uint32_t RawMapNumPlayers;
						uint32_t RawMapNumTeams;

						ISS.read( (char *)&FileFormat, 4 );				// file format (18 = ROC, 25 = TFT)

						if( FileFormat == 18 || FileFormat == 25 )
						{
							ISS.seekg( 4, std::ios :: cur );					// number of saves
							ISS.read( (char *)&EditorVersion, 4 );		// editor version
							std::getline( ISS, GarbageString, '\0' );		// map name
							std::getline( ISS, GarbageString, '\0' );		// map author
							std::getline( ISS, GarbageString, '\0' );		// map description
							std::getline( ISS, GarbageString, '\0' );		// players recommended
							ISS.seekg( 32, std::ios :: cur );				// camera bounds
							ISS.seekg( 16, std::ios :: cur );				// camera bounds complements
							ISS.read( (char *)&RawMapWidth, 4 );		// map width
							ISS.read( (char *)&RawMapHeight, 4 );		// map height
							ISS.read( (char *)&RawMapFlags, 4 );		// flags
							ISS.seekg( 1, std::ios :: cur );					// map main ground type

							if( FileFormat == 18 )
								ISS.seekg( 4, std::ios :: cur );				// campaign background number
							else if( FileFormat == 25 )
							{
								ISS.seekg( 4, std::ios :: cur );				// loading screen background number
								std::getline( ISS, GarbageString, '\0' );	// path of custom loading screen model
							}

							std::getline( ISS, GarbageString, '\0' );		// map loading screen text
							std::getline( ISS, GarbageString, '\0' );		// map loading screen title
							std::getline( ISS, GarbageString, '\0' );		// map loading screen subtitle

							if( FileFormat == 18 )
								ISS.seekg( 4, std::ios :: cur );				// map loading screen number
							else if( FileFormat == 25 )
							{
								ISS.seekg( 4, std::ios :: cur );				// used game data set
								std::getline( ISS, GarbageString, '\0' );	// prologue screen path
							}
							// std:: not need but added
							std::getline( ISS, GarbageString, '\0' );		// prologue screen text
							std::getline( ISS, GarbageString, '\0' );		// prologue screen title
							std::getline( ISS, GarbageString, '\0' );		// prologue screen subtitle

							if( FileFormat == 25 )
							{
								ISS.seekg( 4, std::ios :: cur );				// uses terrain fog
								ISS.seekg( 4, std::ios :: cur );				// fog start z height
								ISS.seekg( 4, std::ios :: cur );				// fog end z height
								ISS.seekg( 4, std::ios :: cur );				// fog density
								ISS.seekg( 1, std::ios :: cur );				// fog red value
								ISS.seekg( 1, std::ios :: cur );				// fog green value
								ISS.seekg( 1, std::ios :: cur );				// fog blue value
								ISS.seekg( 1, std::ios :: cur );				// fog alpha value
								ISS.seekg( 4, std::ios :: cur );				// global weather id
								std::getline( ISS, GarbageString, '\0' );	// custom sound environment
								ISS.seekg( 1, std::ios :: cur );				// tileset id of the used custom light environment
								ISS.seekg( 1, std::ios :: cur );				// custom water tinting red value
								ISS.seekg( 1, std::ios :: cur );				// custom water tinting green value
								ISS.seekg( 1, std::ios :: cur );				// custom water tinting blue value
								ISS.seekg( 1, std::ios :: cur );				// custom water tinting alpha value
							}

							ISS.read( (char *)&RawMapNumPlayers, 4 );	// number of players
							uint32_t ClosedSlots = 0;

							for( uint32_t i = 0; i < RawMapNumPlayers; ++i )
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
								{
									Slot.SetSlotStatus( SLOTSTATUS_CLOSED );
									++ClosedSlots;
								}

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

								ISS.seekg( 4, std::ios :: cur );				// fixed start position
								std::getline( ISS, GarbageString, '\0' );	// player name
								ISS.seekg( 4, std::ios :: cur );				// start position x
								ISS.seekg( 4, std::ios :: cur );				// start position y
								ISS.seekg( 4, std::ios :: cur );				// ally low priorities
								ISS.seekg( 4, std::ios :: cur );				// ally high priorities

								if( Slot.GetSlotStatus( ) != SLOTSTATUS_CLOSED )
									Slots.push_back( Slot );
							}

							ISS.read( (char *)&RawMapNumTeams, 4 );		// number of teams

							for( uint32_t i = 0; i < RawMapNumTeams; ++i )
							{
								uint32_t Flags;
								uint32_t PlayerMask;

								ISS.read( (char *)&Flags, 4 );			// flags
								ISS.read( (char *)&PlayerMask, 4 );		// player mask

								for( unsigned char j = 0; j < MAX_SLOTS; ++j )
								{
									if( PlayerMask & 1 )
									{
										for( std::vector<CGameSlot> :: iterator k = Slots.begin( ); k != Slots.end( ); ++k )
										{
											if( (*k).GetColour( ) == j )
												(*k).SetTeam( i );
										}
									}

									PlayerMask >>= 1;
								}

								std::getline( ISS, GarbageString, '\0' );	// team name
							}

							// the bot only cares about the following options: melee, fixed player settings, custom forces
							// let's not confuse the user by displaying erroneous map options so zero them out now

							MapOptions = RawMapFlags & ( MAPOPT_MELEE | MAPOPT_FIXEDPLAYERSETTINGS | MAPOPT_CUSTOMFORCES );
							CONSOLE_Print( "[MAP] calculated map_options = " + UTIL_ToString( MapOptions ) );
							MapWidth = UTIL_CreateByteArray( (uint16_t)RawMapWidth, false );
							CONSOLE_Print( "[MAP] calculated map_width = " + UTIL_ByteArrayToDecString( MapWidth ) );
							MapHeight = UTIL_CreateByteArray( (uint16_t)RawMapHeight, false );
							CONSOLE_Print( "[MAP] calculated map_height = " + UTIL_ByteArrayToDecString( MapHeight ) );
							MapNumPlayers = RawMapNumPlayers - ClosedSlots;
							CONSOLE_Print( "[MAP] calculated map_numplayers = " + UTIL_ToString( MapNumPlayers ) );
							MapNumTeams = RawMapNumTeams;
							CONSOLE_Print( "[MAP] calculated map_numteams = " + UTIL_ToString( MapNumTeams ) );

							uint32_t SlotNum = 1;

							for( std::vector<CGameSlot> :: iterator i = Slots.begin( ); i != Slots.end( ); ++i )
							{
								CONSOLE_Print( "[MAP] calculated map_slot" + UTIL_ToString( SlotNum ) + " = " + UTIL_ByteArrayToDecString( (*i).GetByteArray( ) ) );
								++SlotNum;
							}

							if( MapOptions & MAPOPT_MELEE )
							{
								CONSOLE_Print( "[MAP] found melee map, initializing slots" );

								// give each slot a different team and set the race to random

								unsigned char Team = 0;

								for( std::vector<CGameSlot> :: iterator i = Slots.begin( ); i != Slots.end( ); ++i )
								{
									(*i).SetTeam( Team++ );
									(*i).SetRace( SLOTRACE_RANDOM );
								}

								MapFilterType = MAPFILTER_TYPE_MELEE;
							}

							if( !( MapOptions & MAPOPT_FIXEDPLAYERSETTINGS ) )
							{
								// make races selectable

								for( std::vector<CGameSlot> :: iterator i = Slots.begin( ); i != Slots.end( ); ++i )
									(*i).SetRace( (*i).GetRace( ) | SLOTRACE_SELECTABLE );
							}
						}
					}
					else
						CONSOLE_Print( "[MAP] unable to calculate map_options, map_width, map_height, map_slot<x>, map_numplayers, map_numteams - unable to extract war3map.w3i from MPQ file" );

					delete [] SubFileData;
				}

				SFileCloseFile( SubFile );
			}
			else
				CONSOLE_Print( "[MAP] unable to calculate map_options, map_width, map_height, map_slot<x>, map_numplayers, map_numteams - couldn't find war3map.w3i in MPQ file" );
		}
		else
			CONSOLE_Print( "[MAP] unable to calculate map_options, map_width, map_height, map_slot<x>, map_numplayers, map_numteams - map MPQ file not loaded" );
	}
	else
		CONSOLE_Print( "[MAP] no map data available, using config file for map_options, map_width, map_height, map_slot<x>, map_numplayers, map_numteams" );

	// close the map MPQ

	if( MapMPQReady )
		SFileCloseArchive( MapMPQ );

	m_MapPath = CFG->GetString( "map_path", std::string( ) );

	if( MapSize.empty( ) )
		MapSize = UTIL_ExtractNumbers( CFG->GetString( "map_size", std::string( ) ), 4 );
	else if( CFG->Exists( "map_size" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_size with config value map_size = " + CFG->GetString( "map_size", std::string( ) ) );
		MapSize = UTIL_ExtractNumbers( CFG->GetString( "map_size", std::string( ) ), 4 );
	}

	m_MapSize = MapSize;

	if( MapInfo.empty( ) )
		MapInfo = UTIL_ExtractNumbers( CFG->GetString( "map_info", std::string( ) ), 4 );
	else if( CFG->Exists( "map_info" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_info with config value map_info = " + CFG->GetString( "map_info", std::string( ) ) );
		MapInfo = UTIL_ExtractNumbers( CFG->GetString( "map_info", std::string( ) ), 4 );
	}

	m_MapInfo = MapInfo;

	if( MapCRC.empty( ) )
		MapCRC = UTIL_ExtractNumbers( CFG->GetString( "map_crc", std::string( ) ), 4 );
	else if( CFG->Exists( "map_crc" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_crc with config value map_crc = " + CFG->GetString( "map_crc", std::string( ) ) );
		MapCRC = UTIL_ExtractNumbers( CFG->GetString( "map_crc", std::string( ) ), 4 );
	}

	m_MapCRC = MapCRC;

	if( MapSHA1.empty( ) )
		MapSHA1 = UTIL_ExtractNumbers( CFG->GetString( "map_sha1", std::string( ) ), 20 );
	else if( CFG->Exists( "map_sha1" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_sha1 with config value map_sha1 = " + CFG->GetString( "map_sha1", std::string( ) ) );
		MapSHA1 = UTIL_ExtractNumbers( CFG->GetString( "map_sha1", std::string( ) ), 20 );
	}

	m_MapSHA1 = MapSHA1;
	m_MapSpeed = CFG->GetInt( "map_speed", MAPSPEED_FAST );
	m_MapVisibility = CFG->GetInt( "map_visibility", MAPVIS_DEFAULT );
	m_MapObservers = CFG->GetInt( "map_observers", MAPOBS_NONE );
	m_MapFlags = CFG->GetInt( "map_flags", MAPFLAG_TEAMSTOGETHER | MAPFLAG_FIXEDTEAMS );
	m_MapFilterMaker = CFG->GetInt( "map_filter_maker", MAPFILTER_MAKER_USER );

	if( CFG->Exists( "map_filter_type" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_filter_type with config value map_filter_type = " + CFG->GetString( "map_filter_type", std::string( ) ) );
		MapFilterType = CFG->GetInt( "map_filter_type", MAPFILTER_TYPE_SCENARIO );
	}

	m_MapFilterType = MapFilterType;

	m_MapFilterSize = CFG->GetInt( "map_filter_size", MAPFILTER_SIZE_LARGE );
	m_MapFilterObs = CFG->GetInt( "map_filter_obs", MAPFILTER_OBS_NONE );

	// todotodo: it might be possible for MapOptions to legitimately be zero so this is not a valid way of checking if it wasn't parsed out earlier

	if( MapOptions == 0 )
		MapOptions = CFG->GetInt( "map_options", 0 );
	else if( CFG->Exists( "map_options" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_options with config value map_options = " + CFG->GetString( "map_options", std::string( ) ) );
		MapOptions = CFG->GetInt( "map_options", 0 );
	}

	m_MapOptions = MapOptions;

	if( MapWidth.empty( ) )
		MapWidth = UTIL_ExtractNumbers( CFG->GetString( "map_width", std::string( ) ), 2 );
	else if( CFG->Exists( "map_width" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_width with config value map_width = " + CFG->GetString( "map_width", std::string( ) ) );
		MapWidth = UTIL_ExtractNumbers( CFG->GetString( "map_width", std::string( ) ), 2 );
	}

	m_MapWidth = MapWidth;

	if( MapHeight.empty( ) )
		MapHeight = UTIL_ExtractNumbers( CFG->GetString( "map_height", std::string( ) ), 2 );
	else if( CFG->Exists( "map_height" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_height with config value map_height = " + CFG->GetString( "map_height", std::string( ) ) );
		MapHeight = UTIL_ExtractNumbers( CFG->GetString( "map_height", std::string( ) ), 2 );
	}

	m_MapHeight = MapHeight;
	m_MapType = CFG->GetString( "map_type", std::string( ) );
	m_MapMatchMakingCategory = CFG->GetString( "map_matchmakingcategory", std::string( ) );
	m_MapStatsW3MMDCategory = CFG->GetString( "map_statsw3mmdcategory", std::string( ) );
	m_MapDefaultHCL = CFG->GetString( "map_defaulthcl", std::string( ) );
	m_MapDefaultPlayerScore = CFG->GetInt( "map_defaultplayerscore", 1000 );
	m_MapLoadInGame = CFG->GetInt( "map_loadingame", 0 ) == 0 ? false : true;

	if( MapNumPlayers == 0 )
		MapNumPlayers = CFG->GetInt( "map_numplayers", 0 );
	else if( CFG->Exists( "map_numplayers" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_numplayers with config value map_numplayers = " + CFG->GetString( "map_numplayers", std::string( ) ) );
		MapNumPlayers = CFG->GetInt( "map_numplayers", 0 );
	}

	m_MapNumPlayers = MapNumPlayers;

	if( MapNumTeams == 0 )
		MapNumTeams = CFG->GetInt( "map_numteams", 0 );
	else if( CFG->Exists( "map_numteams" ) )
	{
		CONSOLE_Print( "[MAP] overriding calculated map_numteams with config value map_numteams = " + CFG->GetString( "map_numteams", std::string( ) ) );
		MapNumTeams = CFG->GetInt( "map_numteams", 0 );
	}

	m_MapNumTeams = MapNumTeams;

	if( Slots.empty( ) )
	{
	for( uint32_t Slot = 1; Slot <= MAX_SLOTS; ++Slot )
		{
			std::string SlotString = CFG->GetString( "map_slot" + UTIL_ToString( Slot ), std::string( ) );

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

		for( uint32_t Slot = 1; Slot <= MAX_SLOTS; ++Slot )
		{
			std::string SlotString = CFG->GetString( "map_slot" + UTIL_ToString( Slot ), std::string( ) );

			if( SlotString.empty( ) )
				break;

			BYTEARRAY SlotData = UTIL_ExtractNumbers( SlotString, 9 );
			Slots.push_back( CGameSlot( SlotData ) );
		}
	}

	m_Slots = Slots;

	// if random races is set force every slot's race to random

	if( m_MapFlags & MAPFLAG_RANDOMRACES )
	{
		CONSOLE_Print( "[MAP] forcing races to random" );

		for( std::vector<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); ++i )
			(*i).SetRace( SLOTRACE_RANDOM );
	}

	// add observer slots

	if( m_MapObservers == MAPOBS_ALLOWED || m_MapObservers == MAPOBS_REFEREES )
	{
		uint32_t DefaultMaxSlots = MAX_SLOTS;
		if( EditorVersion < 6060 )
			DefaultMaxSlots = 12;
		uint32_t MaxSlots = CFG->GetInt( "map_maxslots", DefaultMaxSlots );
		CONSOLE_Print( "[MAP] adding " + UTIL_ToString( MaxSlots - m_Slots.size( ) ) + " observer slots" );

		while( m_Slots.size( ) < MaxSlots )
			m_Slots.push_back( CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, MAX_SLOTS, MAX_SLOTS, SLOTRACE_RANDOM ) );
	}

	CheckValid( );
}

void CMap :: CheckValid( )
{
	// todotodo: should this code fix any errors it sees rather than just warning the user?

	if( m_MapPath.empty( ) || m_MapPath.length( ) > 53 )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_path detected" );
	}
	else if( m_MapPath[0] == '\\' )
		CONSOLE_Print( "[MAP] warning - map_path starts with '\\', any replays saved by GHost++ will not be playable in Warcraft III" );

	if( m_MapPath.find( '/' ) != std::string :: npos )
		CONSOLE_Print( "[MAP] warning - map_path contains forward slashes '/' but it must use Windows style back slashes '\\'" );

	if( m_MapSize.size( ) != 4 )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_size detected" );
	}
	else if( !m_MapData.empty( ) && m_MapData.size( ) != UTIL_ByteArrayToUInt32( m_MapSize, false ) )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_size detected - size mismatch with actual map data" );
	}

	if( m_MapInfo.size( ) != 4 )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_info detected" );
	}

	if( m_MapCRC.size( ) != 4 )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_crc detected" );
	}

	if( m_MapSHA1.size( ) != 20 )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_sha1 detected" );
	}

	if( m_MapSpeed != MAPSPEED_SLOW && m_MapSpeed != MAPSPEED_NORMAL && m_MapSpeed != MAPSPEED_FAST )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_speed detected" );
	}

	if( m_MapVisibility != MAPVIS_HIDETERRAIN && m_MapVisibility != MAPVIS_EXPLORED && m_MapVisibility != MAPVIS_ALWAYSVISIBLE && m_MapVisibility != MAPVIS_DEFAULT )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_visibility detected" );
	}

	if( m_MapObservers != MAPOBS_NONE && m_MapObservers != MAPOBS_ONDEFEAT && m_MapObservers != MAPOBS_ALLOWED && m_MapObservers != MAPOBS_REFEREES )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_observers detected" );
	}

	// todotodo: m_MapFlags
	// todotodo: m_MapFilterMaker, m_MapFilterType, m_MapFilterSize, m_MapFilterObs

	if( m_MapWidth.size( ) != 2 )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_width detected" );
	}

	if( m_MapHeight.size( ) != 2 )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_height detected" );
	}

	if( m_MapNumPlayers == 0 || m_MapNumPlayers > MAX_SLOTS )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_numplayers detected" );
	}

	if( m_MapNumTeams == 0 || m_MapNumTeams > MAX_SLOTS )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_numteams detected" );
	}

	if( m_Slots.empty( ) || m_Slots.size( ) > MAX_SLOTS )
	{
		m_Valid = false;
		CONSOLE_Print( "[MAP] invalid map_slot<x> detected" );
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
		++i;
	}

	return Val;
}

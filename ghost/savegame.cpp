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
#include "packed.h"
#include "savegame.h"

//
// CSaveGame
//

CSaveGame :: CSaveGame( ) : CPacked( ), m_NumSlots( 0 ), m_RandomSeed( 0 )
{

}

CSaveGame :: ~CSaveGame( )
{

}

#define READB( x, y, z )	(x).read( (char *)(y), (z) )
#define READSTR( x, y )		getline( (x), (y), '\0' )

void CSaveGame :: ParseSaveGame( )
{
	m_MapPath.clear( );
	m_GameName.clear( );
	m_NumSlots = 0;
	m_Slots.clear( );
	m_RandomSeed = 0;
	m_MagicNumber.clear( );

	if( m_Flags != 0 )
	{
		CONSOLE_Print( "[SAVEGAME] invalid replay (flags mismatch)" );
		m_Valid = false;
		return;
	}

	istringstream ISS( m_Decompressed );

	// savegame format figured out by Varlock:
	// string		-> map path
	// 0 (string?)	-> ??? (no idea what this is)
	// string		-> game name
	// 0 (string?)	-> ??? (maybe original game password)
	// string		-> stat string
	// 4 bytes		-> ??? (seems to be # of slots)
	// 4 bytes		-> ??? (seems to be 0x01 0x28 0x49 0x00 on both of the savegames examined)
	// 2 bytes		-> ??? (no idea what this is)
	// slot structure
	// 4 bytes		-> magic number

	unsigned char Garbage1;
	uint16_t Garbage2;
	uint32_t Garbage4;
	string GarbageString;
	uint32_t MagicNumber;

	READSTR( ISS, m_MapPath );				// map path
	READSTR( ISS, GarbageString );			// ???
	READSTR( ISS, m_GameName );				// game name
	READSTR( ISS, GarbageString );			// ???
	READSTR( ISS, GarbageString );			// stat string
	READB( ISS, &Garbage4, 4 );				// ???
	READB( ISS, &Garbage4, 4 );				// ???
	READB( ISS, &Garbage2, 2 );				// ???
	READB( ISS, &m_NumSlots, 1 );			// number of slots

	if( m_NumSlots > 12 )
	{
		CONSOLE_Print( "[SAVEGAME] invalid savegame (too many slots)" );
		m_Valid = false;
		return;
	}

	for( unsigned char i = 0; i < m_NumSlots; i++ )
	{
		unsigned char SlotData[9];
		READB( ISS, SlotData, 9 );			// slot data
		m_Slots.push_back( CGameSlot( SlotData[0], SlotData[1], SlotData[2], SlotData[3], SlotData[4], SlotData[5], SlotData[6], SlotData[7], SlotData[8] ) );
	}

	READB( ISS, &m_RandomSeed, 4 );			// random seed
	READB( ISS, &Garbage1, 1 );				// GameType
	READB( ISS, &Garbage1, 1 );				// number of player slots (non observer)
	READB( ISS, &MagicNumber, 4 );			// magic number

	if( ISS.eof( ) || ISS.fail( ) )
	{
		CONSOLE_Print( "[SAVEGAME] failed to parse savegame header" );
		m_Valid = false;
		return;
	}

	m_MagicNumber = UTIL_CreateByteArray( MagicNumber, false );
	m_Valid = true;
}

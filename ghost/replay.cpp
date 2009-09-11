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
#include "replay.h"
#include "gameprotocol.h"

//
// CReplay
//

CReplay :: CReplay( ) : CPacked( )
{
	m_HostPID = 0;
	m_PlayerCount = 0;
	m_MapGameType = 1;

	m_RandomSeed = 0;
	m_SelectMode = 0;
	m_StartSpotCount = 0;
}

CReplay :: ~CReplay( )
{

}

void CReplay :: AddLeaveGame( uint32_t reason, unsigned char PID, uint32_t result )
{
	BYTEARRAY Block;
	Block.push_back( REPLAY_LEAVEGAME );
	UTIL_AppendByteArray( Block, reason, false );
	Block.push_back( PID );
	UTIL_AppendByteArray( Block, result, false );
	UTIL_AppendByteArray( Block, (uint32_t)1, false );
	m_Blocks.push( Block );
}

void CReplay :: AddLeaveGameDuringLoading( uint32_t reason, unsigned char PID, uint32_t result )
{
	BYTEARRAY Block;
	Block.push_back( REPLAY_LEAVEGAME );
	UTIL_AppendByteArray( Block, reason, false );
	Block.push_back( PID );
	UTIL_AppendByteArray( Block, result, false );
	UTIL_AppendByteArray( Block, (uint32_t)1, false );
	m_LoadingBlocks.push( Block );
}

void CReplay :: AddTimeSlot( uint16_t timeIncrement, queue<CIncomingAction *> actions )
{
	BYTEARRAY Block;
	Block.push_back( REPLAY_TIMESLOT );
	UTIL_AppendByteArray( Block, (uint16_t)0, false );
	UTIL_AppendByteArray( Block, timeIncrement, false );

	while( !actions.empty( ) )
	{
		CIncomingAction *Action = actions.front( );
		actions.pop( );
		Block.push_back( Action->GetPID( ) );
		UTIL_AppendByteArray( Block, (uint16_t)Action->GetAction( )->size( ), false );
		UTIL_AppendByteArrayFast( Block, *Action->GetAction( ) );
	}

	// assign length

	BYTEARRAY LengthBytes = UTIL_CreateByteArray( (uint16_t)( Block.size( ) - 3 ), false );
	Block[1] = LengthBytes[0];
	Block[2] = LengthBytes[1];
	m_Blocks.push( Block );
}

void CReplay :: AddChatMessage( unsigned char PID, unsigned char flags, uint32_t chatMode, string message )
{
	BYTEARRAY Block;
	Block.push_back( REPLAY_CHATMESSAGE );
	Block.push_back( PID );
	UTIL_AppendByteArray( Block, (uint16_t)0, false );
	Block.push_back( flags );
	UTIL_AppendByteArray( Block, chatMode, false );
	UTIL_AppendByteArrayFast( Block, message );

	// assign length

	BYTEARRAY LengthBytes = UTIL_CreateByteArray( (uint16_t)( Block.size( ) - 4 ), false );
	Block[2] = LengthBytes[0];
	Block[3] = LengthBytes[1];
	m_Blocks.push( Block );
}

void CReplay :: AddCheckSum( uint32_t checkSum )
{
	m_CheckSums.push( checkSum );
}

void CReplay :: BuildReplay( string gameName, string statString, uint32_t war3Version, uint16_t buildNumber )
{
	m_War3Version = war3Version;
	m_BuildNumber = buildNumber;
	m_Flags = 32768;

	CONSOLE_Print( "[REPLAY] building replay" );

	uint32_t LanguageID = 0x0012F8B0;

	BYTEARRAY Replay;
	Replay.push_back( 16 );															// Unknown (4.0)
	Replay.push_back( 1 );															// Unknown (4.0)
	Replay.push_back( 0 );															// Unknown (4.0)
	Replay.push_back( 0 );															// Unknown (4.0)
	Replay.push_back( 0 );															// Host RecordID (4.1)
	Replay.push_back( m_HostPID );													// Host PlayerID (4.1)
	UTIL_AppendByteArrayFast( Replay, m_HostName );									// Host PlayerName (4.1)
	Replay.push_back( 1 );															// Host AdditionalSize (4.1)
	Replay.push_back( 0 );															// Host AdditionalData (4.1)
	UTIL_AppendByteArrayFast( Replay, gameName );									// GameName (4.2)
	Replay.push_back( 0 );															// Null (4.0)
	UTIL_AppendByteArrayFast( Replay, statString );									// StatString (4.3)
	UTIL_AppendByteArray( Replay, (uint32_t)m_Slots.size( ), false );				// PlayerCount (4.6)
	Replay.push_back( m_MapGameType );												// GameType (4.7)
	Replay.push_back( 32 );															// GameType (4.7)
	Replay.push_back( 73 );															// GameType (4.7)
	Replay.push_back( 0 );															// GameType (4.7)
	UTIL_AppendByteArray( Replay, LanguageID, false );								// LanguageID (4.8)

	// PlayerList (4.9)

	for( vector<ReplayPlayer> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i).first != m_HostPID )
		{
			Replay.push_back( 22 );													// Player RecordID (4.1)
			Replay.push_back( (*i).first );											// Player PlayerID (4.1)
			UTIL_AppendByteArrayFast( Replay, (*i).second );						// Player PlayerName (4.1)
			Replay.push_back( 1 );													// Player AdditionalSize (4.1)
			Replay.push_back( 0 );													// Player AdditionalData (4.1)
			UTIL_AppendByteArray( Replay, (uint32_t)0, false );						// Unknown
		}
	}

	// GameStartRecord (4.10)

	Replay.push_back( 25 );															// RecordID (4.10)
	UTIL_AppendByteArray( Replay, (uint16_t)( 7 + m_Slots.size( ) * 9 ), false );	// Size (4.10)
	Replay.push_back( m_Slots.size( ) );											// NumSlots (4.10)

	for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		UTIL_AppendByteArray( Replay, m_Slots[i].GetByteArray( ) );

	UTIL_AppendByteArray( Replay, m_RandomSeed, false );							// RandomSeed (4.10)
	Replay.push_back( m_SelectMode );												// SelectMode (4.10)
	Replay.push_back( m_StartSpotCount );											// StartSpotCount (4.10)

	// ReplayData (5.0)

	Replay.push_back( REPLAY_FIRSTSTARTBLOCK );
	UTIL_AppendByteArray( Replay, (uint32_t)1, false );
	Replay.push_back( REPLAY_SECONDSTARTBLOCK );
	UTIL_AppendByteArray( Replay, (uint32_t)1, false );

	// leavers during loading need to be stored between the second and third start blocks

	while( !m_LoadingBlocks.empty( ) )
	{
		UTIL_AppendByteArray( Replay, m_LoadingBlocks.front( ) );
		m_LoadingBlocks.pop( );
	}

	Replay.push_back( REPLAY_THIRDSTARTBLOCK );
	UTIL_AppendByteArray( Replay, (uint32_t)1, false );

	// initialize replay length to zero
	// we'll accumulate the replay length as we iterate through the timeslots
	// this is necessary because we might be discarding some timeslots due to not enough checksums and the replay length needs to be accurate

	m_ReplayLength = 0;
	uint32_t TimeSlotsDiscarded = 0;

	while( !m_Blocks.empty( ) )
	{
		BYTEARRAY Block = m_Blocks.front( );
		m_Blocks.pop( );

		if( Block.size( ) >= 5 && Block[0] == REPLAY_TIMESLOT )
		{
			if( m_CheckSums.empty( ) )
			{
				TimeSlotsDiscarded++;
				continue;
			}

			// append timeslot

			UTIL_AppendByteArrayFast( Replay, Block );

			// append checksum

			BYTEARRAY CheckSum;
			CheckSum.reserve( 6 );
			CheckSum.push_back( REPLAY_CHECKSUM );
			CheckSum.push_back( 4 );
			UTIL_AppendByteArray( CheckSum, m_CheckSums.front( ), false );
			m_CheckSums.pop( );
			UTIL_AppendByteArrayFast( Replay, CheckSum );

			// accumulate replay length

			m_ReplayLength += UTIL_ByteArrayToUInt16( Block, false, 3 );
		}
		else
			UTIL_AppendByteArrayFast( Replay, Block );
	}

	if( TimeSlotsDiscarded > 0 )
		CONSOLE_Print( "[REPLAY] ran out of checksums, discarded " + UTIL_ToString( TimeSlotsDiscarded ) + " timeslots" );

	// done

	m_Decompressed = string( Replay.begin( ), Replay.end( ) );
}

#define READB( x, y, z )	(x).read( (char *)(y), (z) )
#define READSTR( x, y )		getline( (x), (y), '\0' )

void CReplay :: ParseReplay( )
{
	m_HostPID = 0;
	m_HostName.clear( );
	m_GameName.clear( );
	m_StatString.clear( );
	m_PlayerCount = 0;
	m_MapGameType = 1;

	m_Players.clear( );
	m_Slots.clear( );
	m_LoadingBlocks = queue<BYTEARRAY>( );
	m_Blocks = queue<BYTEARRAY>( );
	m_CheckSums = queue<uint32_t>( );
	m_RandomSeed = 0;
	m_SelectMode = 0;
	m_StartSpotCount = 0;

	if( m_Flags != 32768 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (flags mismatch)" );
		m_Valid = false;
		return;
	}

	istringstream ISS( m_Decompressed );

	unsigned char Garbage1;
	uint16_t Garbage2;
	uint32_t Garbage4;
	string GarbageString;

	READB( ISS, &Garbage4, 4 );				// Unknown (4.0)

	if( Garbage4 != 272 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (4.0 Unknown mismatch)" );
		m_Valid = false;
		return;
	}

	READB( ISS, &Garbage1, 1 );				// Host RecordID (4.1)

	if( Garbage1 != 0 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (4.1 Host RecordID mismatch)" );
		m_Valid = false;
		return;
	}

	READB( ISS, &m_HostPID, 1 );

	if( m_HostPID > 15 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (4.1 Host PlayerID is invalid)" );
		m_Valid = false;
		return;
	}

	READSTR( ISS, m_HostName );				// Host PlayerName (4.1)
	READB( ISS, &Garbage1, 1 );				// Host AdditionalSize (4.1)

	if( Garbage1 != 1 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (4.1 Host AdditionalSize mismatch)" );
		m_Valid = false;
		return;
	}

	READB( ISS, &Garbage1, 1 );				// Host AdditionalData (4.1)

	if( Garbage1 != 0 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (4.1 Host AdditionalData mismatch)" );
		m_Valid = false;
		return;
	}

	READSTR( ISS, m_GameName );				// GameName (4.2)
	READSTR( ISS, GarbageString );			// Null (4.0)
	READSTR( ISS, m_StatString );			// StatString (4.3)
	READB( ISS, &m_PlayerCount, 4 );		// PlayerCount (4.6)

	if( m_PlayerCount > 12 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (4.6 PlayerCount is invalid)" );
		m_Valid = false;
		return;
	}

	READB( ISS, &Garbage4, 4 );				// GameType (4.7)

	if( (Garbage4 & 0xFFFFFF00) != 4792320 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (4.7 GameType mismatch)" );
		m_Valid = false;
		return;
	}

	m_MapGameType = Garbage4 & 0x000000FF;
	READB( ISS, &Garbage4, 4 );				// LanguageID (4.8)

	while( 1 )
	{
		READB( ISS, &Garbage1, 1 );			// Player RecordID (4.1)

		if( Garbage1 == 22 )
		{
			unsigned char PlayerID;
			string PlayerName;

			READB( ISS, &PlayerID, 1 );		// Player PlayerID (4.1)

			if( PlayerID > 15 )
			{
				CONSOLE_Print( "[REPLAY] invalid replay (4.9 Player PlayerID is invalid)" );
				m_Valid = false;
				return;
			}

			READSTR( ISS, PlayerName );		// Player PlayerName (4.1)
			READB( ISS, &Garbage1, 1 );		// Player AdditionalSize (4.1)

			if( Garbage1 != 1 )
			{
				CONSOLE_Print( "[REPLAY] invalid replay (4.9 Player AdditionalSize mismatch)" );
				m_Valid = false;
				return;
			}

			READB( ISS, &Garbage1, 1 );		// Player AdditionalData (4.1)

			if( Garbage1 != 0 )
			{
				CONSOLE_Print( "[REPLAY] invalid replay (4.9 Player AdditionalData mismatch)" );
				m_Valid = false;
				return;
			}

			READB( ISS, &Garbage4, 4 );		// Unknown

			if( Garbage4 != 0 )
			{
				CONSOLE_Print( "[REPLAY] invalid replay (4.9 Unknown mismatch)" );
				m_Valid = false;
				return;
			}

			AddPlayer( PlayerID, PlayerName );
		}
		else if( Garbage1 == 25 )
			break;
		else
		{
			CONSOLE_Print( "[REPLAY] invalid replay (4.9 Player RecordID mismatch)" );
			m_Valid = false;
			return;
		}
	}

	if( ISS.fail( ) )
	{
		CONSOLE_Print( "[SAVEGAME] failed to parse replay header" );
		m_Valid = false;
		return;
	}

	/*

	// GameStartRecord (4.10)

	Replay.push_back( 25 );															// RecordID (4.10)
	UTIL_AppendByteArray( Replay, (uint16_t)( 7 + m_Slots.size( ) * 9 ), false );	// Size (4.10)
	Replay.push_back( m_Slots.size( ) );											// NumSlots (4.10)

	for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		UTIL_AppendByteArray( Replay, m_Slots[i].GetByteArray( ) );

	UTIL_AppendByteArray( Replay, m_RandomSeed, false );							// RandomSeed (4.10)
	Replay.push_back( m_SelectMode );												// SelectMode (4.10)
	Replay.push_back( m_StartSpotCount );											// StartSpotCount (4.10)

	*/

	m_Valid = true;
}

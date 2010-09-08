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

CReplay :: CReplay( ) : CPacked( ), m_HostPID( 0 ), m_PlayerCount( 0 ), m_MapGameType( 0 ), m_RandomSeed( 0 ), m_SelectMode( 0 ), m_StartSpotCount( 0 )
{
	m_CompiledBlocks.reserve( 262144 );
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
	m_CompiledBlocks += string( Block.begin( ), Block.end( ) );
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

void CReplay :: AddTimeSlot2( queue<CIncomingAction *> actions )
{
	BYTEARRAY Block;
	Block.push_back( REPLAY_TIMESLOT2 );
	UTIL_AppendByteArray( Block, (uint16_t)0, false );
	UTIL_AppendByteArray( Block, (uint16_t)0, false );

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
	m_CompiledBlocks += string( Block.begin( ), Block.end( ) );
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
	m_CompiledBlocks += string( Block.begin( ), Block.end( ) );
	m_ReplayLength += timeIncrement;
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
	m_CompiledBlocks += string( Block.begin( ), Block.end( ) );
}

void CReplay :: AddLoadingBlock( BYTEARRAY &loadingBlock )
{
	m_LoadingBlocks.push( loadingBlock );
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
	UTIL_AppendByteArray( Replay, m_MapGameType, false );							// GameType (4.7)
	UTIL_AppendByteArray( Replay, LanguageID, false );								// LanguageID (4.8)

	// PlayerList (4.9)

        for( vector<PIDPlayer> :: iterator i = m_Players.begin( ); i != m_Players.end( ); ++i )
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

        for( unsigned char i = 0; i < m_Slots.size( ); ++i )
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

	// done

	m_Decompressed = string( Replay.begin( ), Replay.end( ) );
	m_Decompressed += m_CompiledBlocks;
}

#define READB( x, y, z )	(x).read( (char *)(y), (z) )
#define READSTR( x, y )		getline( (x), (y), '\0' )

void CReplay :: ParseReplay( bool parseBlocks )
{
	m_HostPID = 0;
	m_HostName.clear( );
	m_GameName.clear( );
	m_StatString.clear( );
	m_PlayerCount = 0;
	m_MapGameType = 0;
	m_Players.clear( );
	m_Slots.clear( );
	m_RandomSeed = 0;
	m_SelectMode = 0;
	m_StartSpotCount = 0;
	m_LoadingBlocks = queue<BYTEARRAY>( );
	m_Blocks = queue<BYTEARRAY>( );
	m_CheckSums = queue<uint32_t>( );

	if( m_Flags != 32768 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (flags mismatch)" );
		m_Valid = false;
		return;
	}

	istringstream ISS( m_Decompressed );

	unsigned char Garbage1;
	uint32_t Garbage4;
	string GarbageString;
	unsigned char GarbageData[65535];

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

	AddPlayer( m_HostPID, m_HostName );
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

	READB( ISS, &m_MapGameType, 4 );		// GameType (4.7)
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

	uint16_t Size;
	unsigned char NumSlots;
	READB( ISS, &Size, 2 );					// Size (4.10)
	READB( ISS, &NumSlots, 1 );				// NumSlots (4.10)

	if( Size != 7 + NumSlots * 9 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (4.10 Size is invalid)" );
		m_Valid = false;
		return;
	}

	if( NumSlots == 0 || NumSlots > 12 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (4.10 NumSlots is invalid)" );
		m_Valid = false;
		return;
	}

        for( int i = 0; i < NumSlots; ++i )
	{
		unsigned char SlotData[9];
		READB( ISS, SlotData, 9 );
		BYTEARRAY SlotDataBA = UTIL_CreateByteArray( SlotData, 9 );
		m_Slots.push_back( CGameSlot( SlotDataBA ) );
	}

	READB( ISS, &m_RandomSeed, 4 );			// RandomSeed (4.10)
	READB( ISS, &m_SelectMode, 1 );			// SelectMode (4.10)
	READB( ISS, &m_StartSpotCount, 1 );		// StartSpotCount (4.10)

	if( ISS.eof( ) || ISS.fail( ) )
	{
		CONSOLE_Print( "[SAVEGAME] failed to parse replay header" );
		m_Valid = false;
		return;
	}

	if( !parseBlocks )
		return;

	READB( ISS, &Garbage1, 1 );				// first start block ID (5.0)

	if( Garbage1 != CReplay :: REPLAY_FIRSTSTARTBLOCK )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (5.0 first start block ID mismatch)" );
		m_Valid = false;
		return;
	}

	READB( ISS, &Garbage4, 4 );				// first start block data (5.0)

	if( Garbage4 != 1 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (5.0 first start block data mismatch)" );
		m_Valid = false;
		return;
	}

	READB( ISS, &Garbage1, 1 );				// second start block ID (5.0)

	if( Garbage1 != CReplay :: REPLAY_SECONDSTARTBLOCK )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (5.0 second start block ID mismatch)" );
		m_Valid = false;
		return;
	}

	READB( ISS, &Garbage4, 4 );				// second start block data (5.0)

	if( Garbage4 != 1 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (5.0 second start block data mismatch)" );
		m_Valid = false;
		return;
	}

	while( 1 )
	{
		READB( ISS, &Garbage1, 1 );			// third start block ID *or* loading block ID (5.0)

		if( ISS.eof( ) || ISS.fail( ) )
		{
			CONSOLE_Print( "[REPLAY] invalid replay (5.0 third start block unexpected end of file found)" );
			m_Valid = false;
			return;
		}
		if( Garbage1 == CReplay :: REPLAY_LEAVEGAME )
		{
			READB( ISS, GarbageData, 13 );
			BYTEARRAY LoadingBlock;
			LoadingBlock.push_back( Garbage1 );
			UTIL_AppendByteArray( LoadingBlock, GarbageData, 13 );
			m_LoadingBlocks.push( LoadingBlock );
		}
		else if( Garbage1 == CReplay :: REPLAY_THIRDSTARTBLOCK )
			break;
		else
		{
			CONSOLE_Print( "[REPLAY] invalid replay (5.0 third start block ID mismatch)" );
			m_Valid = false;
			return;
		}
	}

	READB( ISS, &Garbage4, 4 );				// third start block data (5.0)

	if( Garbage4 != 1 )
	{
		CONSOLE_Print( "[REPLAY] invalid replay (5.0 third start block data mismatch)" );
		m_Valid = false;
		return;
	}

	if( ISS.eof( ) || ISS.fail( ) )
	{
		CONSOLE_Print( "[SAVEGAME] failed to parse replay start blocks" );
		m_Valid = false;
		return;
	}

	uint32_t ActualReplayLength = 0;

	while( 1 )
	{
		READB( ISS, &Garbage1, 1 );			// block ID (5.0)

		if( ISS.eof( ) || ISS.fail( ) )
			break;
		else if( Garbage1 == CReplay :: REPLAY_LEAVEGAME )
		{
			READB( ISS, GarbageData, 13 );

			// reconstruct the block

			BYTEARRAY Block;
			Block.push_back( CReplay :: REPLAY_LEAVEGAME );
			UTIL_AppendByteArray( Block, GarbageData, 13 );
			m_Blocks.push( Block );
		}
		else if( Garbage1 == CReplay :: REPLAY_TIMESLOT )
		{
			uint16_t BlockSize;
			READB( ISS, &BlockSize, 2 );
			READB( ISS, GarbageData, BlockSize );

			if( BlockSize >= 2 )
				ActualReplayLength += GarbageData[0] | GarbageData[1] << 8;

			// reconstruct the block

			BYTEARRAY Block;
			Block.push_back( CReplay :: REPLAY_TIMESLOT );
			UTIL_AppendByteArray( Block, BlockSize, false );
			UTIL_AppendByteArray( Block, GarbageData, BlockSize );
			m_Blocks.push( Block );
		}
		else if( Garbage1 == CReplay :: REPLAY_CHATMESSAGE )
		{
			unsigned char PID;
			uint16_t BlockSize;
			READB( ISS, &PID, 1 );

			if( PID > 15 )
			{
				CONSOLE_Print( "[REPLAY] invalid replay (5.0 chatmessage pid is invalid)" );
				m_Valid = false;
				return;
			}

			READB( ISS, &BlockSize, 2 );
			READB( ISS, GarbageData, BlockSize );

			// reconstruct the block

			BYTEARRAY Block;
			Block.push_back( CReplay :: REPLAY_CHATMESSAGE );
			Block.push_back( PID );
			UTIL_AppendByteArray( Block, BlockSize, false );
			UTIL_AppendByteArray( Block, GarbageData, BlockSize );
			m_Blocks.push( Block );
		}
		else if( Garbage1 == CReplay :: REPLAY_CHECKSUM )
		{
			READB( ISS, &Garbage1, 1 );

			if( Garbage1 != 4 )
			{
				CONSOLE_Print( "[REPLAY] invalid replay (5.0 checksum unknown mismatch)" );
				m_Valid = false;
				return;
			}

			uint32_t CheckSum;
			READB( ISS, &CheckSum, 4 );
			m_CheckSums.push( CheckSum );
		}
		else
		{
			// it's not necessarily an error if we encounter an unknown block ID since replays can contain extra data

			break;
		}
	}

	if( m_ReplayLength != ActualReplayLength )
		CONSOLE_Print( "[REPLAY] warning - replay length mismatch (" + UTIL_ToString( m_ReplayLength ) + "ms/" + UTIL_ToString( ActualReplayLength ) + "ms)" );

	m_Valid = true;
}

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
#include "savegame.h"

#include <zlib.h>

// we can't use zlib's uncompress function because it expects a complete compressed buffer
// however, we're going to be passing it chunks of incomplete data
// this custom tzuncompress function handles this case properly

int tzuncompress( Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen )
{
	z_stream stream;
	int err;

	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	/* Check for source > 64K on 16-bit machine: */
	if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

	stream.next_out = dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;

	err = inflateInit(&stream);
	if (err != Z_OK) return err;

	err = inflate(&stream, Z_SYNC_FLUSH);
	if (err != Z_STREAM_END && err != Z_OK) {
		inflateEnd(&stream);
		if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
			return Z_DATA_ERROR;
		return err;
	}
	*destLen = stream.total_out;

	err = inflateEnd(&stream);
	return err;
}

//
// CSaveGame
//

CSaveGame :: CSaveGame( CGHost *nGHost )
{
	m_GHost = nGHost;
	m_Valid = false;
}

CSaveGame :: CSaveGame( CGHost *nGHost, string nFileName, string nFileNameNoPath )
{
	m_GHost = nGHost;
	Load( nFileName, nFileNameNoPath );
}

CSaveGame :: ~CSaveGame( )
{

}

void CSaveGame :: Load( string nFileName, string nFileNameNoPath )
{
	m_Valid = true;
	m_FileName = nFileName;
	m_FileNameNoPath = nFileNameNoPath;

	// load the map data

	m_SaveGameData = UTIL_FileRead( m_FileName );
	m_SaveGameDecompressedData.empty( );

	if( !m_SaveGameData.empty( ) )
		Decompress( );

	if( !m_Valid )
	{
		m_SaveGameData.clear( );
		m_SaveGameDecompressedData.clear( );
		return;
	}

	istringstream ISS( m_SaveGameDecompressedData );

	// savegame format figured out by Varlock:
	// string		-> map path
	// 0 (string?)	-> ??? (no idea what this is)
	// string		-> original game name
	// 0 (string?)	-> ??? (maybe original game password)
	// string		-> stat string
	// 4 bytes		-> ??? (seems to be # of slots)
	// 4 bytes		-> ??? (seems to be 0x01 0x28 0x49 0x00 on both of the savegames examined)
	// 2 bytes		-> ??? (no idea what this is)
	// slot structure
	// 4 bytes		-> magic number

	string GarbageString;
	unsigned char NumSlots;
	uint32_t MagicNumber;

	getline( ISS, m_MapPath, '\0' );		// map path
	getline( ISS, GarbageString, '\0' );	// ???
	getline( ISS, GarbageString, '\0' );	// original game name
	getline( ISS, GarbageString, '\0' );	// ???
	getline( ISS, GarbageString, '\0' );	// stat string
	ISS.seekg( 4, ios :: cur );				// ???
	ISS.seekg( 4, ios :: cur );				// ???
	ISS.seekg( 2, ios :: cur );				// ???
	ISS.read( (char *)&NumSlots, 1 );		// number of slots

	if( NumSlots > 12 )
	{
		CONSOLE_Print( "[SAVEGAME] too many slots in decompressed data" );
		m_Valid = false;
		m_SaveGameData.clear( );
		m_SaveGameDecompressedData.clear( );
		return;
	}

	m_Slots.clear( );
	CONSOLE_Print( "[SAVEGAME] found " + UTIL_ToString( NumSlots ) + " slots" );

	for( unsigned char i = 0; i < NumSlots; i++ )
	{
		unsigned char SlotData[9];
		ISS.read( (char *)SlotData, 9 );	// slot data
		m_Slots.push_back( CGameSlot( SlotData[0], SlotData[1], SlotData[2], SlotData[3], SlotData[4], SlotData[5], SlotData[6], SlotData[7], SlotData[8] ) );
	}

	ISS.seekg( 4, ios :: cur );				// GetTicks
	ISS.seekg( 1, ios :: cur );				// GameType
	ISS.seekg( 1, ios :: cur );				// number of player slots (non observer)
	ISS.read( (char *)&MagicNumber, 4 );	// magic number

	if( ISS.fail( ) )
	{
		CONSOLE_Print( "[SAVEGAME] failed to parse decompressed data" );
		m_Valid = false;
		m_SaveGameData.clear( );
		m_SaveGameDecompressedData.clear( );
		return;
	}

	m_MagicNumber = UTIL_CreateByteArray( MagicNumber, false );

	CONSOLE_Print( "[SAVEGAME] found map path = " + m_MapPath );
	CONSOLE_Print( "[SAVEGAME] found magic number = " + UTIL_ToString( m_MagicNumber[0] ) + " " + UTIL_ToString( m_MagicNumber[1] ) + " " + UTIL_ToString( m_MagicNumber[2] ) + " " + UTIL_ToString( m_MagicNumber[3] ) );

	// we're done with the save game data, destroy it

	m_SaveGameData.clear( );
	m_SaveGameDecompressedData.clear( );
}

void CSaveGame :: Decompress( )
{
	istringstream ISS( m_SaveGameData );

	// w3z format found at http://www.thehelper.net/forums/showthread.php?t=42787

	string GarbageString;
	uint32_t HeaderVersion;
	uint32_t DecompressedSize;
	uint32_t NumBlocks;
	uint16_t BlockCompressed;
	uint16_t BlockDecompressed;

	// read header

	getline( ISS, GarbageString, '\0' );

	if( GarbageString != "Warcraft III recorded game\x01A" )
	{
		CONSOLE_Print( "[SAVEGAME] not a valid w3z file" );
		m_Valid = false;
		return;
	}

	ISS.seekg( 4, ios :: cur );						// header size
	ISS.seekg( 4, ios :: cur );						// compressed file size
	ISS.read( (char *)&HeaderVersion, 4 );			// header version
	ISS.read( (char *)&DecompressedSize, 4 );		// decompressed file size
	ISS.read( (char *)&NumBlocks, 4 );				// # of blocks

	if( HeaderVersion == 0 )
	{
		ISS.seekg( 2, ios :: cur );					// unknown
		ISS.seekg( 2, ios :: cur );					// version number
	}
	else
	{
		ISS.seekg( 4, ios :: cur );					// version identifier
		ISS.seekg( 4, ios :: cur );					// version number
	}

	ISS.seekg( 2, ios :: cur );						// build number
	ISS.seekg( 2, ios :: cur );						// flags
	ISS.seekg( 4, ios :: cur );						// length
	ISS.seekg( 4, ios :: cur );						// CRC

	if( ISS.fail( ) )
	{
		CONSOLE_Print( "[SAVEGAME] failed to read w3z header" );
		m_Valid = false;
		return;
	}

	CONSOLE_Print( "[SAVEGAME] reading 1/" + UTIL_ToString( NumBlocks ) + " blocks" );

	// read blocks
	// update: modified this to read only the first block because there's no reason to read every block
	// this is because we only read from the first few bytes of the decompressed data so the first block should have everything we need
	// reading every block is quite CPU and memory intensive because savegames decompress to ~50 MB of data

	for( uint32_t i = 0; i < 1; i++ )
	{
		ISS.read( (char *)&BlockCompressed, 2 );	// block compressed size
		ISS.read( (char *)&BlockDecompressed, 2 );	// block decompressed size
		ISS.seekg( 4, ios :: cur );					// hash

		if( ISS.fail( ) )
		{
			CONSOLE_Print( "[SAVEGAME] failed to read block header" );
			m_Valid = false;
		}

		// read compressed data

		uLongf BlockCompressedLong = BlockCompressed;
		uLongf BlockDecompressedLong = BlockDecompressed;
		unsigned char *CompressedData = new unsigned char[BlockCompressed];
		unsigned char *DecompressedData = new unsigned char[BlockDecompressed];
		ISS.read( (char*)CompressedData, BlockCompressed );

		if( ISS.fail( ) )
		{
			CONSOLE_Print( "[SAVEGAME] failed to read block data" );
			m_Valid = false;
		}

		// decompress data

		int Result = tzuncompress( DecompressedData, &BlockDecompressedLong, CompressedData, BlockCompressedLong );

		if( Result != Z_OK )
		{
			CONSOLE_Print( "[SAVEGAME] uncompress error " + UTIL_ToString( Result ) );
			m_Valid = false;
		}

		if( BlockDecompressedLong != (uLongf)BlockDecompressed )
		{
			CONSOLE_Print( "[SAVEGAME] block decompressed size mismatch, actual = " + UTIL_ToString( BlockDecompressedLong ) + ", expected = " + UTIL_ToString( BlockDecompressed ) );
			m_Valid = false;
		}

		m_SaveGameDecompressedData += string( (char *)DecompressedData, BlockDecompressedLong );
		delete [] DecompressedData;
		delete [] CompressedData;

		if( !m_Valid )
			return;
	}

	CONSOLE_Print( "[SAVEGAME] decompressed " + UTIL_ToString( m_SaveGameDecompressedData.size( ) ) + " bytes" );

	/*

	if( DecompressedSize > m_SaveGameDecompressedData.size( ) )
	{
		CONSOLE_Print( "[SAVEGAME] not enough decompressed data" );
		m_Valid = false;
		return;
	}

	CONSOLE_Print( "[SAVEGAME] discarding " + UTIL_ToString( m_SaveGameDecompressedData.size( ) - DecompressedSize ) + " bytes" );
	m_SaveGameDecompressedData.erase( DecompressedSize );

	*/
}

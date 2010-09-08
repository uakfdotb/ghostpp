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
#include "packed.h"

#include <zlib.h>

// we can't use zlib's uncompress function because it expects a complete compressed buffer
// however, we're going to be passing it chunks of incomplete data
// this custom tzuncompress function will do the job

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
// CPacked
//

CPacked :: CPacked( ) : m_Valid( true ), m_HeaderSize( 0 ), m_CompressedSize( 0 ), m_HeaderVersion( 0 ), m_DecompressedSize( 0 ), m_NumBlocks( 0 ), m_War3Identifier( 0 ), m_War3Version( 0 ), m_BuildNumber( 0 ), m_Flags( 0 ), m_ReplayLength( 0 )
{
	m_CRC = new CCRC32( );
	m_CRC->Initialize( );
}

CPacked :: ~CPacked( )
{
	delete m_CRC;
}

void CPacked :: Load( string fileName, bool allBlocks )
{
	m_Valid = true;
	CONSOLE_Print( "[PACKED] loading data from file [" + fileName + "]" );
	m_Compressed = UTIL_FileRead( fileName );
	Decompress( allBlocks );
}

bool CPacked :: Save( bool TFT, string fileName )
{
	Compress( TFT );

	if( m_Valid )
	{
		CONSOLE_Print( "[PACKED] saving data to file [" + fileName + "]" );
		return UTIL_FileWrite( fileName, (unsigned char *)m_Compressed.c_str( ), m_Compressed.size( ) );
	}
	else
		return false;
}

bool CPacked :: Extract( string inFileName, string outFileName )
{
	m_Valid = true;
	CONSOLE_Print( "[PACKED] extracting data from file [" + inFileName + "] to file [" + outFileName + "]" );
	m_Compressed = UTIL_FileRead( inFileName );
	Decompress( true );

	if( m_Valid )
		return UTIL_FileWrite( outFileName, (unsigned char *)m_Decompressed.c_str( ), m_Decompressed.size( ) );
	else
		return false;
}

bool CPacked :: Pack( bool TFT, string inFileName, string outFileName )
{
	m_Valid = true;
	CONSOLE_Print( "[PACKET] packing data from file [" + inFileName + "] to file [" + outFileName + "]" );
	m_Decompressed = UTIL_FileRead( inFileName );
	Compress( TFT );

	if( m_Valid )
		return UTIL_FileWrite( outFileName, (unsigned char *)m_Compressed.c_str( ), m_Compressed.size( ) );
	else
		return false;
}

void CPacked :: Decompress( bool allBlocks )
{
	CONSOLE_Print( "[PACKED] decompressing data" );

	// format found at http://www.thehelper.net/forums/showthread.php?t=42787

	m_Decompressed.clear( );
	istringstream ISS( m_Compressed );
	string GarbageString;

	// read header

	getline( ISS, GarbageString, '\0' );

	if( GarbageString != "Warcraft III recorded game\x01A" )
	{
		CONSOLE_Print( "[PACKED] not a valid packed file" );
		m_Valid = false;
		return;
	}

	ISS.read( (char *)&m_HeaderSize, 4 );			// header size
	ISS.read( (char *)&m_CompressedSize, 4 );		// compressed file size
	ISS.read( (char *)&m_HeaderVersion, 4 );		// header version
	ISS.read( (char *)&m_DecompressedSize, 4 );		// decompressed file size
	ISS.read( (char *)&m_NumBlocks, 4 );			// number of blocks

	if( m_HeaderVersion == 0 )
	{
		ISS.seekg( 2, ios :: cur );					// unknown
		ISS.seekg( 2, ios :: cur );					// version number

		CONSOLE_Print( "[PACKED] header version is too old" );
		m_Valid = false;
		return;
	}
	else
	{
		ISS.read( (char *)&m_War3Identifier, 4 );	// version identifier
		ISS.read( (char *)&m_War3Version, 4 );		// version number
	}

	ISS.read( (char *)&m_BuildNumber, 2 );			// build number
	ISS.read( (char *)&m_Flags, 2 );				// flags
	ISS.read( (char *)&m_ReplayLength, 4 );			// replay length
	ISS.seekg( 4, ios :: cur );						// CRC

	if( ISS.fail( ) )
	{
		CONSOLE_Print( "[PACKED] failed to read header" );
		m_Valid = false;
		return;
	}

	if( allBlocks )
		CONSOLE_Print( "[PACKED] reading " + UTIL_ToString( m_NumBlocks ) + " blocks" );
	else
		CONSOLE_Print( "[PACKED] reading 1/" + UTIL_ToString( m_NumBlocks ) + " blocks" );

	// read blocks

        for( uint32_t i = 0; i < m_NumBlocks; ++i )
	{
		uint16_t BlockCompressed;
		uint16_t BlockDecompressed;

		// read block header

		ISS.read( (char *)&BlockCompressed, 2 );	// block compressed size
		ISS.read( (char *)&BlockDecompressed, 2 );	// block decompressed size
		ISS.seekg( 4, ios :: cur );					// checksum

		if( ISS.fail( ) )
		{
			CONSOLE_Print( "[PACKED] failed to read block header" );
			m_Valid = false;
			return;
		}

		// read block data

		uLongf BlockCompressedLong = BlockCompressed;
		uLongf BlockDecompressedLong = BlockDecompressed;
		unsigned char *CompressedData = new unsigned char[BlockCompressed];
		unsigned char *DecompressedData = new unsigned char[BlockDecompressed];
		ISS.read( (char*)CompressedData, BlockCompressed );

		if( ISS.fail( ) )
		{
			CONSOLE_Print( "[PACKED] failed to read block data" );
			delete [] DecompressedData;
			delete [] CompressedData;
			m_Valid = false;
			return;
		}

		// decompress block data

		int Result = tzuncompress( DecompressedData, &BlockDecompressedLong, CompressedData, BlockCompressedLong );

		if( Result != Z_OK )
		{
			CONSOLE_Print( "[PACKED] tzuncompress error " + UTIL_ToString( Result ) );
			delete [] DecompressedData;
			delete [] CompressedData;
			m_Valid = false;
			return;
		}

		if( BlockDecompressedLong != (uLongf)BlockDecompressed )
		{
			CONSOLE_Print( "[PACKED] block decompressed size mismatch, actual = " + UTIL_ToString( BlockDecompressedLong ) + ", expected = " + UTIL_ToString( BlockDecompressed ) );
			delete [] DecompressedData;
			delete [] CompressedData;
			m_Valid = false;
			return;
		}

		m_Decompressed += string( (char *)DecompressedData, BlockDecompressedLong );
		delete [] DecompressedData;
		delete [] CompressedData;

		// stop after one iteration if not decompressing all blocks

		if( !allBlocks )
			break;
	}

	CONSOLE_Print( "[PACKED] decompressed " + UTIL_ToString( m_Decompressed.size( ) ) + " bytes" );

	if( allBlocks || m_NumBlocks == 1 )
	{
		if( m_DecompressedSize > m_Decompressed.size( ) )
		{
			CONSOLE_Print( "[PACKED] not enough decompressed data" );
			m_Valid = false;
			return;
		}

		// the last block is padded with zeros, discard them

		CONSOLE_Print( "[PACKED] discarding " + UTIL_ToString( m_Decompressed.size( ) - m_DecompressedSize ) + " bytes" );
		m_Decompressed.erase( m_DecompressedSize );
	}
}

void CPacked :: Compress( bool TFT )
{
	CONSOLE_Print( "[PACKED] compressing data" );

	// format found at http://www.thehelper.net/forums/showthread.php?t=42787

	m_Compressed.clear( );

	// compress data into blocks of size 8192 bytes
	// use a buffer of size 8213 bytes because in the worst case zlib will grow the data 0.1% plus 12 bytes

	uint32_t CompressedSize = 0;
	string Padded = m_Decompressed;
	Padded.append( 8192 - ( Padded.size( ) % 8192 ), 0 );
	vector<string> CompressedBlocks;
	string :: size_type Position = 0;
	unsigned char *CompressedData = new unsigned char[8213];

	while( Position < Padded.size( ) )
	{
		uLongf BlockCompressedLong = 8213;
		int Result = compress( CompressedData, &BlockCompressedLong, (const Bytef *)Padded.c_str( ) + Position, 8192 );

		if( Result != Z_OK )
		{
			CONSOLE_Print( "[PACKED] compress error " + UTIL_ToString( Result ) );
			delete [] CompressedData;
			m_Valid = false;
			return;
		}

		CompressedBlocks.push_back( string( (char *)CompressedData, BlockCompressedLong ) );
		CompressedSize += BlockCompressedLong;
		Position += 8192;
	}

	delete [] CompressedData;

	// build header

	uint32_t HeaderSize = 68;
	uint32_t HeaderCompressedSize = HeaderSize + CompressedSize + CompressedBlocks.size( ) * 8;
	uint32_t HeaderVersion = 1;
	BYTEARRAY Header;
	UTIL_AppendByteArray( Header, "Warcraft III recorded game\x01A" );
	UTIL_AppendByteArray( Header, HeaderSize, false );
	UTIL_AppendByteArray( Header, HeaderCompressedSize, false );
	UTIL_AppendByteArray( Header, HeaderVersion, false );
	UTIL_AppendByteArray( Header, (uint32_t)m_Decompressed.size( ), false );
	UTIL_AppendByteArray( Header, (uint32_t)CompressedBlocks.size( ), false );

	if( TFT )
	{
		Header.push_back( 'P' );	// "W3XP"
		Header.push_back( 'X' );
		Header.push_back( '3' );
		Header.push_back( 'W' );
	}
	else
	{
		Header.push_back( '3' );	// "WAR3"
		Header.push_back( 'R' );
		Header.push_back( 'A' );
		Header.push_back( 'W' );
	}

	UTIL_AppendByteArray( Header, m_War3Version, false );
	UTIL_AppendByteArray( Header, m_BuildNumber, false );
	UTIL_AppendByteArray( Header, m_Flags, false );
	UTIL_AppendByteArray( Header, m_ReplayLength, false );

	// append zero header CRC
	// the header CRC is calculated over the entire header with itself set to zero
	// we'll overwrite the zero header CRC after we calculate it

	UTIL_AppendByteArray( Header, (uint32_t)0, false );

	// calculate header CRC

	string HeaderString = string( Header.begin( ), Header.end( ) );
	uint32_t CRC = m_CRC->FullCRC( (unsigned char *)HeaderString.c_str( ), HeaderString.size( ) );

	// overwrite the (currently zero) header CRC with the calculated CRC

	Header.erase( Header.end( ) - 4, Header.end( ) );
	UTIL_AppendByteArray( Header, CRC, false );

	// append header

	m_Compressed += string( Header.begin( ), Header.end( ) );

	// append blocks

        for( vector<string> :: iterator i = CompressedBlocks.begin( ); i != CompressedBlocks.end( ); ++i )
	{
		BYTEARRAY BlockHeader;
		UTIL_AppendByteArray( BlockHeader, (uint16_t)(*i).size( ), false );
		UTIL_AppendByteArray( BlockHeader, (uint16_t)8192, false );

		// append zero block header CRC
		UTIL_AppendByteArray( BlockHeader, (uint32_t)0, false );

		// calculate block header CRC

		string BlockHeaderString = string( BlockHeader.begin( ), BlockHeader.end( ) );
		uint32_t CRC1 = m_CRC->FullCRC( (unsigned char *)BlockHeaderString.c_str( ), BlockHeaderString.size( ) );
		CRC1 = CRC1 ^ ( CRC1 >> 16 );
		uint32_t CRC2 = m_CRC->FullCRC( (unsigned char *)(*i).c_str( ), (*i).size( ) );
		CRC2 = CRC2 ^ ( CRC2 >> 16 );
		uint32_t BlockCRC = ( CRC1 & 0xFFFF ) | ( CRC2 << 16 );

		// overwrite the block header CRC with the calculated CRC

		BlockHeader.erase( BlockHeader.end( ) - 4, BlockHeader.end( ) );
		UTIL_AppendByteArray( BlockHeader, BlockCRC, false );

		// append block header and data

		m_Compressed += string( BlockHeader.begin( ), BlockHeader.end( ) );
		m_Compressed += *i;
	}
}

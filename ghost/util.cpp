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

#include <sys/stat.h>

BYTEARRAY UTIL_CreateByteArray( unsigned char *a, int size )
{
	if( size < 1 )
		return BYTEARRAY( );

	return BYTEARRAY( a, a + size );
}

BYTEARRAY UTIL_CreateByteArray( unsigned char c )
{
	BYTEARRAY result;
	result.push_back( c );
	return result;
}

BYTEARRAY UTIL_CreateByteArray( uint16_t i, bool reverse )
{
	BYTEARRAY result;
	result.push_back( (unsigned char)i );
	result.push_back( (unsigned char)( i >> 8 ) );

	if( reverse )
		return BYTEARRAY( result.rbegin( ), result.rend( ) );
	else
		return result;
}

BYTEARRAY UTIL_CreateByteArray( uint32_t i, bool reverse )
{
	BYTEARRAY result;
	result.push_back( (unsigned char)i );
	result.push_back( (unsigned char)( i >> 8 ) );
	result.push_back( (unsigned char)( i >> 16 ) );
	result.push_back( (unsigned char)( i >> 24 ) );

	if( reverse )
		return BYTEARRAY( result.rbegin( ), result.rend( ) );
	else
		return result;
}

uint16_t UTIL_ByteArrayToUInt16( BYTEARRAY b, bool reverse, unsigned int start )
{
	if( b.size( ) < start + 2 )
		return 0;

	BYTEARRAY temp = BYTEARRAY( b.begin( ) + start, b.begin( ) + start + 2 );

	if( reverse )
		temp = BYTEARRAY( temp.rbegin( ), temp.rend( ) );

	return (uint16_t)( temp[1] << 8 | temp[0] );
}

uint32_t UTIL_ByteArrayToUInt32( BYTEARRAY b, bool reverse, unsigned int start )
{
	if( b.size( ) < start + 4 )
		return 0;

	BYTEARRAY temp = BYTEARRAY( b.begin( ) + start, b.begin( ) + start + 4 );

	if( reverse )
		temp = BYTEARRAY( temp.rbegin( ), temp.rend( ) );

	return (uint32_t)( temp[3] << 24 | temp[2] << 16 | temp[1] << 8 | temp[0] );
}

std::string UTIL_ByteArrayToDecString( BYTEARRAY b )
{
	if( b.empty( ) )
		return std::string( );

	std::string result = UTIL_ToString( b[0] );

	for( BYTEARRAY :: iterator i = b.begin( ) + 1; i != b.end( ); ++i )
		result += " " + UTIL_ToString( *i );

	return result;
}

std::string UTIL_ByteArrayToHexString( BYTEARRAY b )
{
	if( b.empty( ) )
		return std::string( );

	std::string result = UTIL_ToHexString( b[0] );

	for( BYTEARRAY :: iterator i = b.begin( ) + 1; i != b.end( ); ++i )
	{
		if( *i < 16 )
			result += " 0" + UTIL_ToHexString( *i );
		else
			result += " " + UTIL_ToHexString( *i );
	}

	return result;
}

void UTIL_AppendByteArray( BYTEARRAY &b, BYTEARRAY append )
{
	b.insert( b.end( ), append.begin( ), append.end( ) );
}

void UTIL_AppendByteArrayFast( BYTEARRAY &b, BYTEARRAY &append )
{
	b.insert( b.end( ), append.begin( ), append.end( ) );
}

void UTIL_AppendByteArray( BYTEARRAY &b, unsigned char *a, int size )
{
	UTIL_AppendByteArray( b, UTIL_CreateByteArray( a, size ) );
}

void UTIL_AppendByteArray( BYTEARRAY &b, std::string append, bool terminator )
{
	// append the std::string plus a null terminator

	b.insert( b.end( ), append.begin( ), append.end( ) );

	if( terminator )
		b.push_back( 0 );
}

void UTIL_AppendByteArrayFast( BYTEARRAY &b, std::string &append, bool terminator )
{
	// append the std::string plus a null terminator

	b.insert( b.end( ), append.begin( ), append.end( ) );

	if( terminator )
		b.push_back( 0 );
}

void UTIL_AppendByteArray( BYTEARRAY &b, uint16_t i, bool reverse )
{
	UTIL_AppendByteArray( b, UTIL_CreateByteArray( i, reverse ) );
}

void UTIL_AppendByteArray( BYTEARRAY &b, uint32_t i, bool reverse )
{
	UTIL_AppendByteArray( b, UTIL_CreateByteArray( i, reverse ) );
}

BYTEARRAY UTIL_ExtractCString( BYTEARRAY &b, unsigned int start )
{
	// start searching the byte array at position 'start' for the first null value
	// if found, return the subarray from 'start' to the null value but not including the null value

	if( start < b.size( ) )
	{
		for( unsigned int i = start; i < b.size( ); ++i )
		{
			if( b[i] == 0 )
				return BYTEARRAY( b.begin( ) + start, b.begin( ) + i );
		}

		// no null value found, return the rest of the byte array

		return BYTEARRAY( b.begin( ) + start, b.end( ) );
	}

	return BYTEARRAY( );
}

unsigned char UTIL_ExtractHex( BYTEARRAY &b, unsigned int start, bool reverse )
{
	// consider the byte array to contain a 2 character ASCII encoded hex value at b[start] and b[start + 1] e.g. "FF"
	// extract it as a single decoded byte

	if( start + 1 < b.size( ) )
	{
		unsigned int c;
		std::string temp = std::string( b.begin( ) + start, b.begin( ) + start + 2 );

		if( reverse )
			temp = std::string( temp.rend( ), temp.rbegin( ) );

		std::stringstream SS;
		SS << temp;
		SS >> std::hex >> c;
		return c;
	}

	return 0;
}

BYTEARRAY UTIL_ExtractNumbers( std::string s, unsigned int count )
{
	// consider the std::string to contain a bytearray in dec-text form, e.g. "52 99 128 1"

	BYTEARRAY result;
	unsigned int c;
	std::stringstream SS;
	SS << s;

	for( unsigned int i = 0; i < count; ++i )
	{
		if( SS.eof( ) )
			break;

		SS >> c;

		// todotodo: if c > 255 handle the error instead of truncating

		result.push_back( (unsigned char)c );
	}

	return result;
}

BYTEARRAY UTIL_ExtractHexNumbers( std::string s )
{
	// consider the std::string to contain a bytearray in hex-text form, e.g. "4e 17 b7 e6"

	BYTEARRAY result;
	unsigned int c;
	std::stringstream SS;
	SS << s;

	while( !SS.eof( ) )
	{
		SS >> std::hex >> c;

		// todotodo: if c > 255 handle the error instead of truncating

		result.push_back( (unsigned char)c );
	}

	return result;
}

std::string UTIL_ToString( unsigned long i )
{
	std::string result;
	std::stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

std::string UTIL_ToString( unsigned short i )
{
	std::string result;
	std::stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

std::string UTIL_ToString( unsigned int i )
{
	std::string result;
	std::stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

std::string UTIL_ToString( long i )
{
	std::string result;
	std::stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

std::string UTIL_ToString( short i )
{
	std::string result;
	std::stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

std::string UTIL_ToString( int i )
{
	std::string result;
	std::stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

std::string UTIL_ToString( float f, int digits )
{
	std::string result;
	std::stringstream SS;
	SS << std :: fixed << std :: setprecision( digits ) << f;
	SS >> result;
	return result;
}

std::string UTIL_ToString( double d, int digits )
{
	std::string result;
	std::stringstream SS;
	SS << std :: fixed << std :: setprecision( digits ) << d;
	SS >> result;
	return result;
}

std::string UTIL_ToHexString( uint32_t i )
{
	std::string result;
	std::stringstream SS;
	SS << std :: hex << i;
	SS >> result;
	return result;
}

// todotodo: these UTIL_ToXXX functions don't fail gracefully, they just return garbage (in the uint case usually just -1 casted to an unsigned type it looks like)

uint16_t UTIL_ToUInt16( std::string &s )
{
	uint16_t result;
	std::stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

uint32_t UTIL_ToUInt32( std::string &s )
{
	uint32_t result;
	std::stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

int16_t UTIL_ToInt16( std::string &s )
{
	int16_t result;
	std::stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

int32_t UTIL_ToInt32( std::string &s )
{
	int32_t result;
	std::stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

double UTIL_ToDouble( std::string &s )
{
	double result;
	std::stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

std::string UTIL_MSToString( uint32_t ms )
{
	std::string MinString = UTIL_ToString( ( ms / 1000 ) / 60 );
	std::string SecString = UTIL_ToString( ( ms / 1000 ) % 60 );

	if( MinString.size( ) == 1 )
		MinString.insert( 0, "0" );

	if( SecString.size( ) == 1 )
		SecString.insert( 0, "0" );

	return MinString + "m" + SecString + "s";
}

bool UTIL_FileExists( std::string file )
{
	struct stat fileinfo;

	if( stat( file.c_str( ), &fileinfo ) == 0 )
		return true;

	return false;
}

std::string UTIL_FileRead( std::string file, uint32_t start, uint32_t length )
{
	std::ifstream IS;
	IS.open( file.c_str( ), std::ios :: binary );

	if( IS.fail( ) )
	{
		CONSOLE_Print( "[UTIL] warning - unable to read file part [" + file + "]" );
		return std::string( );
	}

	// get length of file

	IS.seekg( 0, std::ios :: end );
	uint32_t FileLength = IS.tellg( );

	if( start > FileLength )
	{
		IS.close( );
		return std::string( );
	}

	IS.seekg( start, std::ios :: beg );

	// read data

	char *Buffer = new char[length];
	IS.read( Buffer, length );
	std::string BufferString = std::string( Buffer, IS.gcount( ) );
	IS.close( );
	delete [] Buffer;
	return BufferString;
}

std::string UTIL_FileRead( std::string file )
{
	std::ifstream IS;
	IS.open( file.c_str( ), std::ios :: binary );

	if( IS.fail( ) )
	{
		CONSOLE_Print( "[UTIL] warning - unable to read file [" + file + "]" );
		return std::string( );
	}

	// get length of file

	IS.seekg( 0, std::ios :: end );
	uint32_t FileLength = IS.tellg( );
	IS.seekg( 0, std::ios :: beg );

	// read data

	char *Buffer = new char[FileLength];
	IS.read( Buffer, FileLength );
	std::string BufferString = std::string( Buffer, IS.gcount( ) );
	IS.close( );
	delete [] Buffer;

	if( BufferString.size( ) == FileLength )
		return BufferString;
	else
		return std::string( );
}

bool UTIL_FileWrite( std::string file, unsigned char *data, uint32_t length )
{
	std::ofstream OS;
	OS.open( file.c_str( ), std::ios :: binary );

	if( OS.fail( ) )
	{
		CONSOLE_Print( "[UTIL] warning - unable to write file [" + file + "]" );
		return false;
	}

	// write data

	OS.write( (const char *)data, length );
	OS.close( );
	return true;
}

std::string UTIL_FileSafeName( std::string fileName )
{
	std::string :: size_type BadStart = fileName.find_first_of( "\\/:*?<>|" );

	while( BadStart != std::string :: npos )
	{
		fileName.replace( BadStart, 1, 1, '_' );
		BadStart = fileName.find_first_of( "\\/:*?<>|" );
	}

	return fileName;
}

std::string UTIL_AddPathSeperator( std::string path )
{
	if( path.empty( ) )
		return std::string( );

#ifdef WIN32
	char Seperator = '\\';
#else
	char Seperator = '/';
#endif

	if( *(path.end( ) - 1) == Seperator )
		return path;
	else
		return path + std::string( 1, Seperator );
}

BYTEARRAY UTIL_EncodeStatString( BYTEARRAY &data )
{
	unsigned char Mask = 1;
	BYTEARRAY Result;

	for( unsigned int i = 0; i < data.size( ); ++i )
	{
		if( ( data[i] % 2 ) == 0 )
			Result.push_back( data[i] + 1 );
		else
		{
			Result.push_back( data[i] );
			Mask |= 1 << ( ( i % 7 ) + 1 );
		}

		if( i % 7 == 6 || i == data.size( ) - 1 )
		{
			Result.insert( Result.end( ) - 1 - ( i % 7 ), Mask );
			Mask = 1;
		}
	}

	return Result;
}

BYTEARRAY UTIL_DecodeStatString( BYTEARRAY &data )
{
	unsigned char Mask;
	BYTEARRAY Result;

	for( unsigned int i = 0; i < data.size( ); ++i )
	{
		if( ( i % 8 ) == 0 )
			Mask = data[i];
		else
		{
			if( ( Mask & ( 1 << ( i % 8 ) ) ) == 0 )
				Result.push_back( data[i] - 1 );
			else
				Result.push_back( data[i] );
		}
	}

	return Result;
}

bool UTIL_IsLanIP( BYTEARRAY ip )
{
	if( ip.size( ) != 4 )
		return false;

	// thanks to LuCasn for this function

	// 127.0.0.1
	if( ip[0] == 127 && ip[1] == 0 && ip[2] == 0 && ip[3] == 1 )
		return true;

	// 10.x.x.x
	if( ip[0] == 10 )
		return true;

	// 172.16.0.0-172.31.255.255
	if( ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31 )
		return true;

	// 192.168.x.x
	if( ip[0] == 192 && ip[1] == 168 )
		return true;

	// RFC 3330 and RFC 3927 automatic address range
	if( ip[0] == 169 && ip[1] == 254 )
		return true;

	return false;
}

bool UTIL_IsLocalIP( BYTEARRAY ip, std::vector<BYTEARRAY> &localIPs )
{
	if( ip.size( ) != 4 )
		return false;

	for( std::vector<BYTEARRAY> :: iterator i = localIPs.begin( ); i != localIPs.end( ); ++i )
	{
		if( (*i).size( ) != 4 )
			continue;

		if( ip[0] == (*i)[0] && ip[1] == (*i)[1] && ip[2] == (*i)[2] && ip[3] == (*i)[3] )
			return true;
	}

	return false;
}

void UTIL_Replace( std::string &Text, std::string Key, std::string Value )
{
	// don't allow any infinite loops

	if( Value.find( Key ) != std::string :: npos )
		return;

	std::string :: size_type KeyStart = Text.find( Key );

	while( KeyStart != std::string :: npos )
	{
		Text.replace( KeyStart, Key.size( ), Value );
		KeyStart = Text.find( Key );
	}
}

std::vector<std::string> UTIL_Tokenize( std::string s, char delim )
{
	std::vector<std::string> Tokens;
	std::string Token;

	for( std::string :: iterator i = s.begin( ); i != s.end( ); ++i )
	{
		if( *i == delim )
		{
			if( Token.empty( ) )
				continue;

			Tokens.push_back( Token );
			Token.clear( );
		}
		else
			Token += *i;
	}

	if( !Token.empty( ) )
		Tokens.push_back( Token );

	return Tokens;
}

uint32_t UTIL_Factorial( uint32_t x )
{
	uint32_t Factorial = 1;

	for( uint32_t i = 2; i <= x; ++i )
		Factorial *= i;

	return Factorial;
}

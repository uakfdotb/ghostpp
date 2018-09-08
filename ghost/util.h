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

#ifndef UTIL_H
#define UTIL_H

// byte arrays

BYTEARRAY UTIL_CreateByteArray( unsigned char *a, int size );
BYTEARRAY UTIL_CreateByteArray( unsigned char c );
BYTEARRAY UTIL_CreateByteArray( uint16_t i, bool reverse );
BYTEARRAY UTIL_CreateByteArray( uint32_t i, bool reverse );
uint16_t UTIL_ByteArrayToUInt16( BYTEARRAY b, bool reverse, unsigned int start = 0 );
uint32_t UTIL_ByteArrayToUInt32( BYTEARRAY b, bool reverse, unsigned int start = 0 );
std::string UTIL_ByteArrayToDecString( BYTEARRAY b );
std::string UTIL_ByteArrayToHexString( BYTEARRAY b );
void UTIL_AppendByteArray( BYTEARRAY &b, BYTEARRAY append );
void UTIL_AppendByteArrayFast( BYTEARRAY &b, BYTEARRAY &append );
void UTIL_AppendByteArray( BYTEARRAY &b, unsigned char *a, int size );
void UTIL_AppendByteArray( BYTEARRAY &b, std::string append, bool terminator = true );
void UTIL_AppendByteArrayFast( BYTEARRAY &b, std::string &append, bool terminator = true );
void UTIL_AppendByteArray( BYTEARRAY &b, uint16_t i, bool reverse );
void UTIL_AppendByteArray( BYTEARRAY &b, uint32_t i, bool reverse );
BYTEARRAY UTIL_ExtractCString( BYTEARRAY &b, unsigned int start );
unsigned char UTIL_ExtractHex( BYTEARRAY &b, unsigned int start, bool reverse );
BYTEARRAY UTIL_ExtractNumbers( std::string s, unsigned int count );
BYTEARRAY UTIL_ExtractHexNumbers( std::string s );

// conversions

std::string UTIL_ToString( unsigned long i );
std::string UTIL_ToString( unsigned short i );
std::string UTIL_ToString( unsigned int i );
std::string UTIL_ToString( long i );
std::string UTIL_ToString( short i );
std::string UTIL_ToString( int i );
std::string UTIL_ToString( float f, int digits );
std::string UTIL_ToString( double d, int digits );
std::string UTIL_ToHexString( uint32_t i );
uint16_t UTIL_ToUInt16( std::string &s );
uint32_t UTIL_ToUInt32( std::string &s );
int16_t UTIL_ToInt16( std::string &s );
int32_t UTIL_ToInt32( std::string &s );
double UTIL_ToDouble( std::string &s );
std::string UTIL_MSToString( uint32_t ms );

// files

bool UTIL_FileExists( std::string file );
std::string UTIL_FileRead( std::string file, uint32_t start, uint32_t length );
std::string UTIL_FileRead( std::string file );
bool UTIL_FileWrite( std::string file, unsigned char *data, uint32_t length );
std::string UTIL_FileSafeName( std::string fileName );
std::string UTIL_AddPathSeperator( std::string path );

// stat strings

BYTEARRAY UTIL_EncodeStatString( BYTEARRAY &data );
BYTEARRAY UTIL_DecodeStatString( BYTEARRAY &data );

// other

bool UTIL_IsLanIP( BYTEARRAY ip );
bool UTIL_IsLocalIP( BYTEARRAY ip, std::vector<BYTEARRAY> &localIPs );
void UTIL_Replace( std::string &Text, std::string Key, std::string Value );
std::vector<std::string> UTIL_Tokenize( std::string s, char delim );

// math

uint32_t UTIL_Factorial( uint32_t x );

#define nCr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)) / UTIL_Factorial(r))
#define nPr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)))

#endif

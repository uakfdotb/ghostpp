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
#include "config.h"

#include <stdlib.h>

//
// CConfig
//

CConfig :: CConfig( )
{

}

CConfig :: ~CConfig( )
{

}

void CConfig :: Read( std::string file )
{
	std::ifstream in;
	in.open( file.c_str( ) );

	if( in.fail( ) )
		CONSOLE_Print( "[CONFIG] warning - unable to read file [" + file + "]" );
	else
	{
		CONSOLE_Print( "[CONFIG] loading file [" + file + "]" );
		std::string Line;

		while( !in.eof( ) )
		{
			std::getline( in, Line );

			// ignore blank lines and comments

			if( Line.empty( ) || Line[0] == '#' )
				continue;

			// remove newlines and partial newlines to help fix issues with Windows formatted config files on Linux systems

			Line.erase( remove( Line.begin( ), Line.end( ), '\r' ), Line.end( ) );
			Line.erase( remove( Line.begin( ), Line.end( ), '\n' ), Line.end( ) );

			std::string :: size_type Split = Line.find( "=" );

			if( Split == std::string :: npos )
				continue;

			std::string :: size_type KeyStart = Line.find_first_not_of( " " );
			std::string :: size_type KeyEnd = Line.find( " ", KeyStart );
			std::string :: size_type ValueStart = Line.find_first_not_of( " ", Split + 1 );
			std::string :: size_type ValueEnd = Line.size( );

			if( ValueStart != std::string :: npos )
				m_CFG[Line.substr( KeyStart, KeyEnd - KeyStart )] = Line.substr( ValueStart, ValueEnd - ValueStart );
		}

		in.close( );
	}
}

bool CConfig :: Exists( std::string key )
{
	return m_CFG.find( key ) != m_CFG.end( );
}

int CConfig :: GetInt( std::string key, int x )
{
	if( m_CFG.find( key ) == m_CFG.end( ) )
		return x;
	else
		return atoi( m_CFG[key].c_str( ) );
}

uint32_t CConfig :: GetUInt32( std::string key, uint32_t x )
{
	if( m_CFG.find( key ) == m_CFG.end( ) )
		return x;
	else
		return UTIL_ToUInt32( m_CFG[key] );
}

std::string CConfig :: GetString( std::string key, std::string x )
{
	if( m_CFG.find( key ) == m_CFG.end( ) )
		return x;
	else
		return m_CFG[key];
}

void CConfig :: Set( std::string key, std::string x )
{
	m_CFG[key] = x;
}

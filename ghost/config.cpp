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

map<string, string> gCFG;

void CFG_Read( map<string, string> &CFG, string file )
{
	ifstream in;
	in.open( file.c_str( ) );

	if( in.fail( ) )
		CONSOLE_Print( "[CONFIG] warning - unable to read file [" + file + "]" );
	else
	{
		CONSOLE_Print( "[CONFIG] loading file [" + file + "]" );
		string Line;

		while( !in.eof( ) )
		{
			getline( in, Line );

			// ignore blank lines and comments

			if( Line.empty( ) || Line[0] == '#' )
				continue;

			string :: size_type Split = Line.find( "=" );

			if( Split == string :: npos )
				continue;

			string :: size_type KeyStart = Line.find_first_not_of( " " );
			string :: size_type KeyEnd = Line.find( " ", KeyStart );
			string :: size_type ValueStart = Line.find_first_not_of( " ", Split + 1 );
			string :: size_type ValueEnd = Line.size( );

			if( ValueStart != string :: npos )
				CFG[Line.substr( KeyStart, KeyEnd - KeyStart )] = Line.substr( ValueStart, ValueEnd - ValueStart );
		}

		in.close( );
	}
}

void CFG_SetInt( map<string, string> &CFG, string key, int x )
{
	CFG[key] = UTIL_ToString( x );
}

void CFG_SetString( map<string, string> &CFG, string key, string x )
{
	CFG[key] = x;
}

int CFG_GetInt( map<string, string> &CFG, string key, int x )
{
	if( CFG.find( key ) == CFG.end( ) )
		return x;
	else
		return atoi( CFG[key].c_str( ) );
}

string CFG_GetString( map<string, string> &CFG, string key, string x )
{
	if( CFG.find( key ) == CFG.end( ) )
		return x;
	else
		return CFG[key];
}

void CFG_Delete( map<string, string> &CFG, string key )
{
	CFG.erase( key );
}

void CFG_Write( map<string, string> &CFG, string file )
{
	ofstream out;
	out.open( file.c_str( ) );

	if( out.fail( ) )
		CONSOLE_Print( "[CONFIG] warning - unable to write file [" + file + "]" );

	for( map<string, string> :: iterator i = CFG.begin( ); i != CFG.end( ); i++ )
		out << (*i).first.c_str( ) << " = " << (*i).second.c_str( ) << endl;

	out.close( );
}

void CFG_Clear( map<string, string> &CFG )
{
	CFG.clear( );
}

void CFG_Read( string file )
{
	CFG_Read( gCFG, file );
}

void CFG_SetInt( string key, int x )
{
	CFG_SetInt( gCFG, key, x );
}

void CFG_SetString( string key, string x )
{
	CFG_SetString( gCFG, key, x );
}

int CFG_GetInt( string key, int x )
{
	return CFG_GetInt( gCFG, key, x );
}

string CFG_GetString( string key, string x )
{
	return CFG_GetString( gCFG, key, x );
}

void CFG_Delete( string key )
{
	CFG_Delete( gCFG, key );
}

void CFG_Write( string file )
{
	CFG_Write( gCFG, file );
}

void CFG_Clear( )
{
	CFG_Clear( gCFG );
}

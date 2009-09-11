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

#include "includes.h"
#include "util.h"
#include "packed.h"
#include "replay.h"
#include "savegame.h"

void CONSOLE_Print( string message )
{
	cout << message << endl;
}

void PrintP( CPacked *Packed )
{
	cout << "=======================" << endl;
	cout << "PACKED FILE INFORMATION" << endl;
	cout << "=======================" << endl;

	if( Packed->GetValid( ) )
	{
		cout << "header size      : " << Packed->GetHeaderSize( ) << " bytes" << endl;
		cout << "compressed size  : " << Packed->GetCompressedSize( ) << " bytes" << endl;
		cout << "header version   : " << Packed->GetHeaderVersion( ) << endl;
		cout << "decompressed size: " << Packed->GetDecompressedSize( ) << " bytes" << endl;
		cout << "num blocks       : " << Packed->GetNumBlocks( ) << endl;
		cout << "game identifier  : " << Packed->GetWar3Identifier( ) << " (";

		BYTEARRAY GameIdentifier = UTIL_CreateByteArray( Packed->GetWar3Identifier( ), true );

		for( int j = 0; j < 4; j++ )
		{
			if( isalpha( GameIdentifier[j] ) || isdigit( GameIdentifier[j] ) )
				cout << (char)GameIdentifier[j];
		}

		cout << ")" << endl;
		cout << "game version     : " << Packed->GetWar3Version( ) << endl;
		cout << "build number     : " << Packed->GetBuildNumber( ) << endl;
		cout << "flags            : " << Packed->GetFlags( ) << endl;
		cout << "replay length    : " << Packed->GetReplayLength( ) << " ms (" << Packed->GetReplayLength( ) / 1000 << " seconds)" << endl;
	}
	else
		cout << "packed file is not valid" << endl;
}

#define READB( x, y, z )	(x).read( (char *)(y), (z) )
#define READSTR( x, y )		getline( (x), (y), '\0' )

void PrintPR( CReplay *Replay )
{
	cout << "=======================" << endl;
	cout << "REPLAY FILE INFORMATION" << endl;
	cout << "=======================" << endl;

	if( Replay->GetValid( ) )
	{
		cout << "host info         : " << (int)Replay->GetHostPID( ) << " -> " << Replay->GetHostName( ) << endl;
		cout << "game name         : " << Replay->GetGameName( ) << endl;

		// parse stat string

		string StatString = Replay->GetStatString( );
		BYTEARRAY Temp = BYTEARRAY( StatString.begin( ), StatString.end( ) );
		Temp = UTIL_DecodeStatString( Temp );
		istringstream ISS( string( Temp.begin( ), Temp.end( ) ) );

		unsigned char Garbage1;
		uint32_t MapGameFlags;
		uint16_t MapWidth;
		uint16_t MapHeight;
		uint32_t MapXORO;
		string MapPath;
		string HostName;
		unsigned char MapSHA1[20];
		READB( ISS, &MapGameFlags, 4 );
		READB( ISS, &Garbage1, 1 );
		READB( ISS, &MapWidth, 2 );
		READB( ISS, &MapHeight, 2 );
		READB( ISS, &MapXORO, 4 );
		READSTR( ISS, MapPath );
		READSTR( ISS, HostName );
		READB( ISS, MapSHA1, 20 );

		if( ISS.fail( ) )
		{
			cout << "ss map game flags : <error>" << endl;
			cout << "ss map width      : <error>" << endl;
			cout << "ss map height     : <error>" << endl;
			cout << "ss map xoro       : <error>" << endl;
			cout << "ss map path       : <error>" << endl;
			cout << "ss host name      : <error>" << endl;
			cout << "ss map sha1       : <error>" << endl;
		}
		else
		{
			// todotodo: parse map game flags

			cout << "ss map game flags : " << MapGameFlags << endl;
			cout << "ss map width      : " << MapWidth << endl;
			cout << "ss map height     : " << MapHeight << endl;
			cout << "ss map xoro       : [" << UTIL_ByteArrayToDecString( UTIL_CreateByteArray( MapXORO, false ) ) << "]" << endl;
			cout << "ss map path       : " << MapPath << endl;
			cout << "ss host name      : " << HostName << endl;
			cout << "ss map sha1       : [" << UTIL_ByteArrayToDecString( UTIL_CreateByteArray( MapSHA1, 20 ) ) << "]" << endl;
		}

		cout << "player count      : " << Replay->GetPlayerCount( ) << endl;
		cout << "map game type     : " << (int)Replay->GetMapGameType( ) << endl;

		vector<ReplayPlayer> Players = Replay->GetPlayers( );

		for( vector<ReplayPlayer> :: iterator i = Players.begin( ); i != Players.end( ); i++ )
			cout << "player info       : " << (int)(*i).first << " -> " << (*i).second << endl;

		// todotodo: slot info

		cout << "random seed       : " << Replay->GetRandomSeed( ) << endl;
		cout << "select mode       : " << (int)Replay->GetSelectMode( ) << endl;
		cout << "start spot count  : " << (int)Replay->GetStartSpotCount( ) << endl;
		cout << "num loading blocks: " << Replay->GetLoadingBlocks( )->size( ) << endl;
		cout << "num blocks        : " << Replay->GetBlocks( )->size( ) << endl;
		cout << "num checksums     : " << Replay->GetCheckSums( )->size( ) << endl;
	}
	else
		cout << "replay file is not valid" << endl;
}

void PrintPS( CSaveGame *SaveGame )
{
	cout << "===========================" << endl;
	cout << "SAVED GAME FILE INFORMATION" << endl;
	cout << "===========================" << endl;

	if( SaveGame->GetValid( ) )
	{
		cout << "map path         : " << SaveGame->GetMapPath( ) << endl;
		cout << "game name        : " << SaveGame->GetGameName( ) << endl;
		cout << "num slots        : " << (int)SaveGame->GetNumSlots( ) << endl;
		cout << "random seed      : " << SaveGame->GetRandomSeed( ) << endl;
		cout << "magic number     : " << UTIL_ByteArrayToUInt32( SaveGame->GetMagicNumber( ), false ) << " [" + UTIL_ByteArrayToDecString( SaveGame->GetMagicNumber( ) ) << "]" << endl;
	}
	else
		cout << "saved game file is not valid" << endl;
}

void PrintUsage( )
{
	cout << "replay_stitcher: a Warcraft 3 saved game & replay utility" << endl;
	cout << "written by Trevor Hogan and included with GHost++ (forum.codelain.com)" << endl;
	// cout << "release version 1.0" << endl;
	cout << " OPTIONS:" << endl;
	cout << "  -d filename: decompress a packed file" << endl;
	cout << "  -p filename: print basic information from a packed file" << endl;
	cout << "  -pr filename: print replay information from a Warcraft 3 replay" << endl;
	cout << "  -ps filename: print saved game information from a Warcraft 3 saved game" << endl;
}

int main( int argc, char **argv )
{
	if( argc <= 1 )
	{
		PrintUsage( );
		return 1;
	}

	vector<string> Args;

	for( int i = 0; i < argc; i++ )
		Args.push_back( argv[i] );

	for( int i = 1; i < argc; i++ )
	{
		if( Args[i] == "-d" )
		{
			if( ++i < argc )
			{
				CPacked *Packed = new CPacked( );
				Packed->Extract( Args[i], Args[i] + ".unpacked" );
				delete Packed;
			}
			else
			{
				PrintUsage( );
				return 1;
			}
		}
		else if( Args[i] == "-p" )
		{
			if( ++i < argc )
			{
				CPacked *Packed = new CPacked( );
				Packed->Load( Args[i], true );
				PrintP( Packed );
				delete Packed;
			}
			else
			{
				PrintUsage( );
				return 1;
			}
		}
		else if( Args[i] == "-pr" )
		{
			if( ++i < argc )
			{
				CReplay *Replay = new CReplay( );
				Replay->Load( Args[i], true );
				PrintP( Replay );
				Replay->ParseReplay( );
				PrintPR( Replay );
				delete Replay;
			}
			else
			{
				PrintUsage( );
				return 1;
			}
		}
		else if( Args[i] == "-ps" )
		{
			if( ++i < argc )
			{
				CSaveGame *SaveGame = new CSaveGame( );
				SaveGame->Load( Args[i], true );
				PrintP( SaveGame );
				SaveGame->ParseSaveGame( );
				PrintPS( SaveGame );
				delete SaveGame;
			}
			else
			{
				PrintUsage( );
				return 1;
			}
		}
		else
		{
			PrintUsage( );
			return 1;
		}
	}

	return 0;
}

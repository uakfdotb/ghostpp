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

		if( Packed->GetFlags( ) == 32768 )
			cout << "replay length    : " << Packed->GetReplayLength( ) << " ms (" << UTIL_MSToString( Packed->GetReplayLength( ) ) << ")" << endl;
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
		{
			if( (*i).first != Replay->GetHostPID( ) )
				cout << "player info       : " << (int)(*i).first << " -> " << (*i).second << endl;
		}

		cout << "num slots         : " << Replay->GetSlots( ).size( ) << endl;

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
	cout << "  -d <filename>: decompress a packed file" << endl;
	cout << "  -p <filename>: print basic information from a packed file" << endl;
	cout << "  -pr <filename>: print replay information from a Warcraft 3 replay" << endl;
	cout << "  -ps <filename>: print saved game information from a Warcraft 3 saved game" << endl;
	cout << "  -s <filename1> <filename2> ...: stitch replays" << endl;
}

bool operator <( const ReplayPlayer &left, const ReplayPlayer &right )
{
	return left.first < right.first;
}

bool operator ==( const ReplayPlayer &left, const ReplayPlayer &right )
{
	return left.first == right.first && left.second == right.second;
}

bool operator ==( const CGameSlot &left, const CGameSlot &right )
{
	return left.GetByteArray( ) == right.GetByteArray( );
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
		else if( Args[i] == "-s" )
		{
			vector<CReplay *> Replays;

			while( ++i < argc )
			{
				CReplay *Replay = new CReplay( );
				Replay->Load( Args[i], true );
				Replay->ParseReplay( );
				Replays.push_back( Replay );
			}

			if( Replays.size( ) >= 2 )
			{
				uint32_t ReplayLength = 0;
				unsigned char HostPID = Replays[0]->GetHostPID( );
				string HostName = Replays[0]->GetHostName( );
				CReplay *Stitched = new CReplay( );
				Stitched->SetWar3Version( Replays[0]->GetWar3Version( ) );
				Stitched->SetBuildNumber( Replays[0]->GetBuildNumber( ) );
				Stitched->SetFlags( Replays[0]->GetFlags( ) );
				Stitched->SetMapGameType( Replays[0]->GetMapGameType( ) );
				vector<ReplayPlayer> Players = Replays[0]->GetPlayers( );
				sort( Players.begin( ), Players.end( ) );

				for( vector<ReplayPlayer> :: iterator i = Players.begin( ); i != Players.end( ); i++ )
					Stitched->AddPlayer( (*i).first, (*i).second );

				Stitched->SetSlots( Replays[0]->GetSlots( ) );
				Stitched->SetRandomSeed( Replays[0]->GetRandomSeed( ) );
				Stitched->SetSelectMode( Replays[0]->GetSelectMode( ) );
				Stitched->SetStartSpotCount( Replays[0]->GetStartSpotCount( ) );

				for( vector<CReplay *> :: iterator i = Replays.begin( ); i != Replays.end( ); i++ )
				{
					if( i != Replays.begin( ) )
					{
						if( (*i)->GetWar3Version( ) != Stitched->GetWar3Version( ) )
						{
							cout << "*** error: subsequent replay has different game version" << endl;
							return 1;
						}

						if( (*i)->GetBuildNumber( ) != Stitched->GetBuildNumber( ) )
						{
							cout << "*** error: subsequent replay has different build number" << endl;
							return 1;
						}

						if( (*i)->GetFlags( ) != Stitched->GetFlags( ) )
						{
							cout << "*** error: subsequent replay has different flags" << endl;
							return 1;
						}

						if( (*i)->GetMapGameType( ) != Stitched->GetMapGameType( ) )
						{
							cout << "*** error: subsequent replay has different map game type" << endl;
							return 1;
						}

						vector<ReplayPlayer> Players2 = (*i)->GetPlayers( );
						sort( Players2.begin( ), Players2.end( ) );

						if( Players != Players2 )
						{
							cout << "*** error: subsequent replay has different player layout" << endl;
							return 1;
						}

						if( (*i)->GetSlots( ) != Stitched->GetSlots( ) )
						{
							cout << "*** error: subsequent replay has different slot layout" << endl;
							return 1;
						}

						if( (*i)->GetSelectMode( ) != Stitched->GetSelectMode( ) )
						{
							cout << "*** error: subsequent replay has different select mode" << endl;
							return 1;
						}

						if( (*i)->GetStartSpotCount( ) != Stitched->GetStartSpotCount( ) )
						{
							cout << "*** error: subsequent replay has different start spot count" << endl;
							return 1;
						}

						// todotodo: compare stat strings for game settings, etc

						if( !(*i)->GetLoadingBlocks( )->empty( ) )
						{
							cout << "*** error: subsequent replay contains loading blocks" << endl;
							return 1;
						}
					}

					cout << "stitching replay..." << endl;
					bool SaveGameSelected = false;
					queue<BYTEARRAY> *Blocks = (*i)->GetBlocks( );
					queue<uint32_t> *CheckSums = (*i)->GetCheckSums( );

					while( !Blocks->empty( ) )
					{
						BYTEARRAY Block = Blocks->front( );
						Blocks->pop( );
						Stitched->AddBlock( Block );

						if( Block.size( ) >= 6 && Block[0] == CReplay :: REPLAY_LEAVEGAME )
						{
							HostPID = Block[5];

							for( vector<ReplayPlayer> :: iterator j = Players.begin( ); j != Players.end( ); j++ )
							{
								if( (*j).first == HostPID )
									HostName = (*j).second;
							}
						}
						else if( Block.size( ) >= 5 && Block[0] == CReplay :: REPLAY_TIMESLOT )
						{
							if( CheckSums->empty( ) )
							{
								cout << "*** error: ran out of checksums" << endl;
								return 1;
							}

							Stitched->AddCheckSum( CheckSums->front( ) );
							CheckSums->pop( );
							ReplayLength += UTIL_ByteArrayToUInt16( Block, false, 3 );

							// extract individual actions

							uint32_t Pos = 5;

							while( Block.size( ) >= Pos + 4 )
							{
								unsigned char ActionPID = Block[Pos];
								uint16_t ActionSize = UTIL_ByteArrayToUInt16( Block, false, Pos + 1 );

								if( Block[Pos + 3] == 6 )
								{
									string SavingPlayer;

									for( vector<ReplayPlayer> :: iterator j = Players.begin( ); j != Players.end( ); j++ )
									{
										if( (*j).first == ActionPID )
											SavingPlayer = (*j).second;
									}

									if( SavingPlayer.empty( ) )
									{
										cout << "*** error: found savegame action with invalid pid" << endl;
										return 1;
									}

									if( i + 1 != Replays.end( ) )
									{
										cout << "===> " << SavingPlayer << " saved the game at " << UTIL_MSToString( ReplayLength ) << ", is this where the next replay started?" << endl;
										string Input;
										cin >> Input;

										if( Input == "y" || Input == "Y" )
										{
											SaveGameSelected = true;
											break;
										}
									}
								}

								Pos += ActionSize + 3;
							}
						}

						if( SaveGameSelected )
							break;
					}

					if( i + 1 != Replays.end( ) && !SaveGameSelected )
					{
						cout << "*** error: end of replay found but no savegame action was found or selected" << endl;
						return 1;
					}
				}

				Stitched->SetReplayLength( ReplayLength );
				Stitched->SetHostPID( HostPID );
				Stitched->SetHostName( HostName );
				Stitched->BuildReplay( Replays[0]->GetGameName( ), Replays[0]->GetStatString( ), Replays[0]->GetWar3Version( ), Replays[0]->GetBuildNumber( ) );
				Stitched->Save( "stitched.w3g" );

				for( vector<CReplay *> :: iterator i = Replays.begin( ); i != Replays.end( ); i++ )
					delete *i;

				Replays.clear( );
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

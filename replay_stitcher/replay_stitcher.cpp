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

void PrintUsage( )
{
	cout << "replay_stitcher: a Warcraft 3 saved game & replay utility" << endl;
	cout << "written by Trevor Hogan and included with GHost++ (forum.codelain.com)" << endl;
	cout << " OPTIONS:" << endl;
	cout << "  -d filename: decompress a packed file" << endl;
	cout << "  -p filename: print basic information from a packed file" << endl;
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
				Packed->Extract( Args[i], Args[i] + ".raw" );
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
				cout << "=======================" << endl;
				cout << "PACKED FILE INFORMATION" << endl;
				cout << "=======================" << endl;
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
				delete Packed;
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

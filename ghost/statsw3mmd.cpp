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
#include "ghostdb.h"
#include "gameprotocol.h"
#include "stats.h"
#include "statsw3mmd.h"

//
// CStatsW3MMD
//

CStatsW3MMD :: CStatsW3MMD( CBaseGame *nGame ) : CStats( nGame )
{
	CONSOLE_Print( "[STATSW3MMD] using Warcraft 3 Map Meta Data stats parser version 0" );
}

CStatsW3MMD :: ~CStatsW3MMD( )
{

}

bool CStatsW3MMD :: ProcessAction( CIncomingAction *Action )
{
	unsigned int i = 0;
	BYTEARRAY ActionData = Action->GetAction( );
	BYTEARRAY MissionKey;
	BYTEARRAY Key;
	BYTEARRAY Value;

	/*

	**Message Format**
	- The sub-packets carrying messages in the game data have the format byte 0x6B, string filename, string mission_key, string key, dword value.
	- The strings are all null-terminated, and dwords are little-endian.
	- Messages can be identified by the surrounding pattern: kMMD.Dat[null]val:[decimal-number][null][message contents][null][dword]"
	- Checksum messages can be identified by the surrounding pattern: kMMD.Dat[null]chk:[decimal-number][null][decimal-number][null][dword]"
	- Message ids start at 0 and increase by 1 for each message. IDs become very important in cases where cheaters try to fake messages.
	- Messages are composed of a sequence of arguments separated by non-escaped spaces.
	- Escape sequences are '\ ' for ' ' (space), '\\' for '\' (backslash).
	- The dword value in the message is a weak checksum for the message. The parser does not need to understand this checksum, as it is only used by client to detects forgeries.
	- A message must be followed by a checksum message with the message contents replaced by the msg id.
	- The purpose of the checksum message is to allow friendly clients to detect tampering, and ultimately is only required because of the limitations of wc3 JASS.
	- An example of data containing a message with checksum:
			...
			kMMD.Dat[null]val:0[null]init version 0 0[null][0xFFFFFFFF]
			...
			kMMD.Dat[null]chk:0[null]0[null][0xFFFFFFFF]
			...

	*/

	while( ActionData.size( ) >= i + 9 )
	{
		if( ActionData[i] == 'k' &&
			ActionData[i + 1] == 'M' &&
			ActionData[i + 2] == 'M' &&
			ActionData[i + 3] == 'D' &&
			ActionData[i + 4] == '.' &&
			ActionData[i + 5] == 'D' &&
			ActionData[i + 6] == 'a' &&
			ActionData[i + 7] == 't' &&
			ActionData[i + 8] == 0x00 )
		{
			if( ActionData.size( ) >= i + 10 )
			{
				MissionKey = UTIL_ExtractCString( ActionData, i + 9 );

				if( ActionData.size( ) >= i + 11 + MissionKey.size( ) )
				{
					Key = UTIL_ExtractCString( ActionData, i + 10 + MissionKey.size( ) );

					if( ActionData.size( ) >= i + 15 + MissionKey.size( ) + Key.size( ) )
					{
						Value = BYTEARRAY( ActionData.begin( ) + i + 11 + MissionKey.size( ) + Key.size( ), ActionData.begin( ) + i + 15 + MissionKey.size( ) + Key.size( ) );
						string MissionKeyString = string( MissionKey.begin( ), MissionKey.end( ) );
						string KeyString = string( Key.begin( ), Key.end( ) );
						uint32_t ValueInt = UTIL_ByteArrayToUInt32( Value, false );

						CONSOLE_Print( "[STATSW3MMD] DEBUG: mkey [" + MissionKeyString + "], key [" + KeyString + "], value [" + UTIL_ToString( ValueInt ) + "]" );

						// todotodo: verify message order using MissionKey
						// todotodo: cheat detection

						if( MissionKeyString.size( ) >= 3 && MissionKeyString.substr( 0, 3 ) == "val" )
						{
							vector<string> Tokens = TokenizeKey( KeyString );

							if( !Tokens.empty( ) )
							{
								if( Tokens[0] == "init" && Tokens.size( ) >= 2 )
								{
									if( Tokens[1] == "version" && Tokens.size( ) == 4 )
									{
										// Tokens[2] = minimum
										// Tokens[3] = current

										CONSOLE_Print( "[STATSW3MMD] map is using Warcraft 3 Map Meta Data library version [" + Tokens[3] + "]" );

										if( UTIL_ToUInt32( Tokens[2] ) > 0 )
											CONSOLE_Print( "[STATSW3MMD] warning - parser version 0 is not compatible with this map, minimum version [" + Tokens[2] + "]" );
										else
											CONSOLE_Print( "[STATSW3MMD] parser version 0 is compatible with this map, minimum version [" + Tokens[2] + "]" );
									}
									else if( Tokens[1] == "pid" && Tokens.size( ) == 4 )
									{
										// Tokens[2] = pid
										// Tokens[3] = name

										uint32_t PID = UTIL_ToUInt32( Tokens[2] );

										if( m_PIDToName.find( PID ) != m_PIDToName.end( ) )
											CONSOLE_Print( "[STATSW3MMD] overwriting previous name [" + m_PIDToName[PID] + "] with new name [" + Tokens[3] + "] for PID [" + Tokens[2] + "]" );

										m_PIDToName[PID] = Tokens[3];
									}
								}
								else if( Tokens[0] == "DefVarP" && Tokens.size( ) == 5 )
								{
									// Tokens[1] = name
									// Tokens[2] = value type
									// Tokens[3] = goal type
									// Tokens[4] = priority
								}
								else if( Tokens[0] == "VarP" && Tokens.size( ) == 5 )
								{
									// Tokens[1] = pid
									// Tokens[2] = name
									// Tokens[3] = operation
									// Tokens[4] = value
								}
								else if( Tokens[0] == "FlagP" && Tokens.size( ) == 3 )
								{
									// Tokens[1] = pid
									// Tokens[2] = flag

									if( Tokens[2] == "winner" || Tokens[2] == "loser" || Tokens[2] == "drawer" || Tokens[2] == "leaver" || Tokens[2] == "practicing" )
									{
										uint32_t PID = UTIL_ToUInt32( Tokens[1] );

										if( Tokens[2] == "leaver" )
											m_FlagsLeaver[PID] = true;
										else if( Tokens[2] == "practicing" )
											m_FlagsPracticing[PID] = true;
										else
										{
											if( m_Flags.find( PID ) != m_Flags.end( ) )
												CONSOLE_Print( "[STATSW3MMD] overwriting previous flag [" + m_Flags[PID] + "] with new flag [" + Tokens[2] + "] for PID [" + Tokens[1] + "]" );

											m_Flags[PID] = Tokens[2];
										}
									}
									else
										CONSOLE_Print( "[STATSW3MMD] unknown flag [" + Tokens[2] + "] found, ignoring" );
								}
								else if( Tokens[0] == "DefEvent" )
								{

								}
								else if( Tokens[0] == "Event" )
								{

								}
								else if( Tokens[0] == "Custom" )
								{

								}
								else
									CONSOLE_Print( "[STATSW3MMD] unknown message type [" + Tokens[0] + "] found, ignoring" );
							}
						}
						else if( MissionKeyString.size( ) >= 3 && MissionKeyString.substr( 0, 3 ) == "chk" )
						{

						}

						i += 15 + MissionKey.size( ) + Key.size( );
					}
					else
						i++;
				}
				else
					i++;
			}
			else
				i++;
		}
		else
			i++;
	}

	return false;
}

void CStatsW3MMD :: Save( CGHost *GHost, CGHostDB *DB, uint32_t GameID )
{
	for( map<uint32_t,string> :: iterator i = m_PIDToName.begin( ); i != m_PIDToName.end( ); i++ )
	{
		CONSOLE_Print( "[STATSW3MMD] DEBUG: recorded PID [" + UTIL_ToString( i->first ) + "], name [" + i->second + "]" );

		if( m_Flags.find( i->first ) == m_Flags.end( ) )
			CONSOLE_Print( "[STATSW3MMD] no flag recorded for PID [" + UTIL_ToString( i->first ) + "]" );
		else
			CONSOLE_Print( "[STATSW3MMD] DEBUG: recorded flag [" + i->second + "]" );

		uint32_t Leaver = 0;
		uint32_t Practicing = 0;

		if( m_FlagsLeaver.find( i->first ) != m_FlagsLeaver.end( ) && m_FlagsLeaver[i->first] )
			Leaver = 1;

		if( m_FlagsPracticing.find( i->first ) != m_FlagsLeaver.end( ) && m_FlagsPracticing[i->first] )
			Practicing = 1;

		GHost->m_Callables.push_back( DB->ThreadedW3MMDPlayerAdd( "test", GameID, i->first, i->second, m_Flags[i->first], Leaver, Practicing ) );
	}
}

vector<string> CStatsW3MMD :: TokenizeKey( string key )
{
	vector<string> Tokens;
	string Token;
	bool Escaping = false;

	for( string :: iterator i = key.begin( ); i != key.end( ); i++ )
	{
		if( Escaping )
		{
			if( *i == ' ' )
				Token += ' ';
			else if( *i == '\\' )
				Token += '\\';
			else
			{
				CONSOLE_Print( "[STATSW3MMD] error tokenizing key [" + key + "], invalid escape sequence found, ignoring" );
				return vector<string>( );
			}

			Escaping = false;
		}
		else
		{
			if( *i == ' ' )
			{
				if( Token.empty( ) )
				{
					CONSOLE_Print( "[STATSW3MMD] error tokenizing key [" + key + "], empty token found, ignoring" );
					return vector<string>( );
				}

				Tokens.push_back( Token );
				Token.clear( );
			}
			else if( *i == '\\' )
				Escaping = true;
			else
				Token += *i;
		}
	}

	if( Token.empty( ) )
	{
		CONSOLE_Print( "[STATSW3MMD] error tokenizing key [" + key + "], empty token found, ignoring" );
		return vector<string>( );
	}

	Tokens.push_back( Token );
	return Tokens;
}

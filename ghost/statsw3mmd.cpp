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
#include "game_base.h"
#include "stats.h"
#include "statsw3mmd.h"

//
// CStatsW3MMD
//

CStatsW3MMD :: CStatsW3MMD( CBaseGame *nGame, string nCategory ) : CStats( nGame )
{
	CONSOLE_Print( "[STATSW3MMD] using Warcraft 3 Map Meta Data stats parser version 0" );
	CONSOLE_Print( "[STATSW3MMD] using map_statsw3mmdcategory [%s]", nCategory.c_str() );
	m_Category = nCategory;
	m_NextValueID = 0;
	m_NextCheckID = 0;
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

						// CONSOLE_Print( "[STATSW3MMD] DEBUG: mkey [" + MissionKeyString + "], key [" + KeyString + "], value [" + UTIL_ToString( ValueInt ) + "]" );

						if( MissionKeyString.size( ) > 4 && MissionKeyString.substr( 0, 4 ) == "val:" )
						{
							string ValueIDString = MissionKeyString.substr( 4 );
							uint32_t ValueID = UTIL_ToUInt32( ValueIDString );

							if( ValueID == m_NextValueID )
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

											CONSOLE_Print( "[STATSW3MMD: %s] map is using Warcraft 3 Map Meta Data library version [%s]", m_Game->GetGameName( ).c_str(), Tokens[3].c_str() );

											if( UTIL_ToUInt32( Tokens[2] ) > 0 )
												CONSOLE_Print( "[STATSW3MMD: %s] warning - parser version 0 is not compatible with this map, minimum version [%s]", m_Game->GetGameName( ).c_str(), Tokens[2].c_str() );
											else
												CONSOLE_Print( "[STATSW3MMD: %s] parser version 0 is compatible with this map, minimum version [%s]", m_Game->GetGameName( ).c_str(), Tokens[2].c_str() );
										}
										else if( Tokens[1] == "pid" && Tokens.size( ) == 4 )
										{
											// Tokens[2] = pid
											// Tokens[3] = name

											uint32_t PID = UTIL_ToUInt32( Tokens[2] );

											if( m_PIDToName.find( PID ) != m_PIDToName.end( ) )
												CONSOLE_Print( "[STATSW3MMD: %s] overwriting previous name [%s] with new name [%s] for PID [%s]", m_Game->GetGameName( ).c_str(), m_PIDToName[PID].c_str(), Tokens[3].c_str(), Tokens[2].c_str() );

											m_PIDToName[PID] = Tokens[3];
										}
									}
									else if( Tokens[0] == "DefVarP" && Tokens.size( ) == 5 )
									{
										// Tokens[1] = name
										// Tokens[2] = value type
										// Tokens[3] = goal type (ignored here)
										// Tokens[4] = suggestion (ignored here)

										if( m_DefVarPs.find( Tokens[1] ) != m_DefVarPs.end( ) )
											CONSOLE_Print( "[STATSW3MMD: %s] duplicate DefVarP [%s] found, ignoring", m_Game->GetGameName( ).c_str(), KeyString.c_str() );
										else
										{
											if( Tokens[2] == "int" || Tokens[2] == "real" || Tokens[2] == "string" )
												m_DefVarPs[Tokens[1]] = Tokens[2];
											else
												CONSOLE_Print( "[STATSW3MMD: %s] unknown DefVarP [%s] found, ignoring", m_Game->GetGameName( ).c_str(), KeyString.c_str() );
										}

									}
									else if( Tokens[0] == "VarP" && Tokens.size( ) == 5 )
									{
										// Tokens[1] = pid
										// Tokens[2] = name
										// Tokens[3] = operation
										// Tokens[4] = value

										if( m_DefVarPs.find( Tokens[2] ) == m_DefVarPs.end( ) )
											CONSOLE_Print( "[STATSW3MMD: %s] VarP [%s] found without a corresponding DefVarP, ignoring", m_Game->GetGameName( ).c_str(), KeyString.c_str() );
										else
										{
											string ValueType = m_DefVarPs[Tokens[2]];

											if( ValueType == "int" )
											{
												VarP VP = VarP( UTIL_ToUInt32( Tokens[1] ), Tokens[2] );

												if( Tokens[3] == "=" )
													m_VarPInts[VP] = UTIL_ToInt32( Tokens[4] );
												else if( Tokens[3] == "+=" )
												{
													if( m_VarPInts.find( VP ) != m_VarPInts.end( ) )
														m_VarPInts[VP] += UTIL_ToInt32( Tokens[4] );
													else
													{
														// CONSOLE_Print( "[STATSW3MMD: %s] int VarP [" + KeyString + "] found with relative operation [+=] without a previously assigned value, ignoring" );
														m_VarPInts[VP] = UTIL_ToInt32( Tokens[4] );
													}
												}
												else if( Tokens[3] == "-=" )
												{
													if( m_VarPInts.find( VP ) != m_VarPInts.end( ) )
														m_VarPInts[VP] -= UTIL_ToInt32( Tokens[4] );
													else
													{
														// CONSOLE_Print( "[STATSW3MMD: %s] int VarP [" + KeyString + "] found with relative operation [-=] without a previously assigned value, ignoring" );
														m_VarPInts[VP] = -UTIL_ToInt32( Tokens[4] );
													}
												}
												else
													CONSOLE_Print( "[STATSW3MMD: %s] unknown int VarP [%s] operation [%s] found, ignoring", m_Game->GetGameName( ).c_str(), KeyString.c_str(), Tokens[3].c_str() );
											}
											else if( ValueType == "real" )
											{
												VarP VP = VarP( UTIL_ToUInt32( Tokens[1] ), Tokens[2] );

												if( Tokens[3] == "=" )
													m_VarPReals[VP] = UTIL_ToDouble( Tokens[4] );
												else if( Tokens[3] == "+=" )
												{
													if( m_VarPReals.find( VP ) != m_VarPReals.end( ) )
														m_VarPReals[VP] += UTIL_ToDouble( Tokens[4] );
													else
													{
														// CONSOLE_Print( "[STATSW3MMD: %s] real VarP [" + KeyString + "] found with relative operation [+=] without a previously assigned value, ignoring" );
														m_VarPReals[VP] = UTIL_ToDouble( Tokens[4] );
													}
												}
												else if( Tokens[3] == "-=" )
												{
													if( m_VarPReals.find( VP ) != m_VarPReals.end( ) )
														m_VarPReals[VP] -= UTIL_ToDouble( Tokens[4] );
													else
													{
														// CONSOLE_Print( "[STATSW3MMD: %s] real VarP [" + KeyString + "] found with relative operation [-=] without a previously assigned value, ignoring" );
														m_VarPReals[VP] = -UTIL_ToDouble( Tokens[4] );
													}
												}
												else
													CONSOLE_Print( "[STATSW3MMD: %s] unknown real VarP [%s] operation [%s] found, ignoring", m_Game->GetGameName( ).c_str(), KeyString.c_str(), Tokens[3].c_str() );
											}
											else
											{
												VarP VP = VarP( UTIL_ToUInt32( Tokens[1] ), Tokens[2] );

												if( Tokens[3] == "=" )
													m_VarPStrings[VP] = Tokens[4];
												else
													CONSOLE_Print( "[STATSW3MMD: %s] unknown string VarP [%s] operation [%s] found, ignoring", m_Game->GetGameName( ).c_str(), KeyString.c_str(), Tokens[3].c_str() );
											}
										}
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
													CONSOLE_Print( "[STATSW3MMD: %s] overwriting previous flag [%s] with new flag [%s] for PID [%s]", m_Game->GetGameName( ).c_str(), m_Flags[PID].c_str(), Tokens[2].c_str(), Tokens[1].c_str() );

												m_Flags[PID] = Tokens[2];
											}
										}
										else
											CONSOLE_Print( "[STATSW3MMD: %s] unknown flag [%s] found, ignoring", m_Game->GetGameName( ).c_str(), Tokens[2].c_str() );
									}
									else if( Tokens[0] == "DefEvent" )
									{
										// todotodo: parse event definitions so event messages can be displayed in a readable format
									}
									else if( Tokens[0] == "Event" )
									{
										// todotodo: use event definitions to display event messages in a readable format

										CONSOLE_Print( "[STATSW3MMD: %s] event [%s]", m_Game->GetGameName( ).c_str(), KeyString.c_str() );
									}
									else if( Tokens[0] == "Custom" )
									{
										CONSOLE_Print( "[STATSW3MMD: %s] custom [%s]", m_Game->GetGameName( ).c_str(), KeyString.c_str() );
									}
									else
										CONSOLE_Print( "[STATSW3MMD: %s] unknown message type [%s] found, ignoring", m_Game->GetGameName( ).c_str(), Tokens[0].c_str() );
								}

								m_NextValueID++;
							}
							else
								CONSOLE_Print( "[STATSW3MMD: %s] found value id [%s] but expected %d, ignoring", m_Game->GetGameName( ).c_str(), ValueIDString.c_str(), m_NextValueID );
						}
						else if( MissionKeyString.size( ) > 4 && MissionKeyString.substr( 0, 4 ) == "chk:" )
						{
							string CheckIDString = MissionKeyString.substr( 4 );
							uint32_t CheckID = UTIL_ToUInt32( CheckIDString );

							if( CheckID == m_NextCheckID )
							{
								// todotodo: cheat detection

								m_NextCheckID++;
							}
							else
								CONSOLE_Print( "[STATSW3MMD: %s] found check id [%s] but expected %d, ignoring", m_Game->GetGameName( ).c_str(), CheckIDString.c_str(), m_NextCheckID );
						}
						else
							CONSOLE_Print( "[STATSW3MMD: %s] unknown mission key [%s] found, ignoring", m_Game->GetGameName( ).c_str(), MissionKeyString.c_str() );

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
	CONSOLE_Print( "[STATSW3MMD: %s] received %d/%d value/check messages", m_Game->GetGameName( ).c_str(), m_NextValueID, m_NextCheckID );

	if( DB->Begin( ) )
	{
		// todotodo: there's no reason to create a new callable for each entry in this map
		// rewrite ThreadedW3MMDPlayerAdd to act more like ThreadedW3MMDVarAdd

		for( map<uint32_t,string> :: iterator i = m_PIDToName.begin( ); i != m_PIDToName.end( ); i++ )
		{
			if( m_Flags.find( i->first ) == m_Flags.end( ) )
				CONSOLE_Print( "[STATSW3MMD: %s] warning - no flag recorded for PID [%d]", m_Game->GetGameName( ).c_str(), i->first );

			uint32_t Leaver = 0;
			uint32_t Practicing = 0;

			if( m_FlagsLeaver.find( i->first ) != m_FlagsLeaver.end( ) && m_FlagsLeaver[i->first] )
				Leaver = 1;

			if( m_FlagsPracticing.find( i->first ) != m_FlagsLeaver.end( ) && m_FlagsPracticing[i->first] )
				Practicing = 1;

			GHost->m_Callables.push_back( DB->ThreadedW3MMDPlayerAdd( m_Category, GameID, i->first, i->second, m_Flags[i->first], Leaver, Practicing ) );
		}

		if( !m_VarPInts.empty( ) )
			GHost->m_Callables.push_back( DB->ThreadedW3MMDVarAdd( GameID, m_VarPInts ) );

		if( !m_VarPReals.empty( ) )
			GHost->m_Callables.push_back( DB->ThreadedW3MMDVarAdd( GameID, m_VarPReals ) );

		if( !m_VarPStrings.empty( ) )
			GHost->m_Callables.push_back( DB->ThreadedW3MMDVarAdd( GameID, m_VarPStrings ) );

		if( DB->Commit( ) )
			CONSOLE_Print( "[STATSW3MMD: %s] saving data", m_Game->GetGameName( ).c_str() );
		else
			CONSOLE_Print( "[STATSW3MMD: %s] unable to commit database transaction, data not saved", m_Game->GetGameName( ).c_str() );
	}
	else
		CONSOLE_Print( "[STATSW3MMD: %s] unable to begin database transaction, data not saved", m_Game->GetGameName( ).c_str() );
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
				CONSOLE_Print( "[STATSW3MMD: %s] error tokenizing key [%s], invalid escape sequence found, ignoring", m_Game->GetGameName( ).c_str(), key.c_str() );
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
					CONSOLE_Print( "[STATSW3MMD: %s] error tokenizing key [%s], empty token found, ignoring", m_Game->GetGameName( ).c_str(), key.c_str() );
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
		CONSOLE_Print( "[STATSW3MMD: %s] error tokenizing key [%s], empty token found, ignoring", m_Game->GetGameName( ).c_str(), key.c_str() );
		return vector<string>( );
	}

	Tokens.push_back( Token );
	return Tokens;
}

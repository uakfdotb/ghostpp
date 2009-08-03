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

#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#ifdef WIN32
 #include "ms_stdint.h"
#else
 #include <stdint.h>
#endif

#include "config.h"
#include "elo.h"

#include <string.h>

#ifdef WIN32
 #include <winsock.h>
#endif

#include <mysql/mysql.h>

void CONSOLE_Print( string message )
{
	cout << message << endl;
}

string MySQLEscapeString( MYSQL *conn, string str )
{
	char *to = new char[str.size( ) * 2 + 1];
	unsigned long size = mysql_real_escape_string( conn, to, str.c_str( ), str.size( ) );
	string result( to, size );
	delete [] to;
	return result;
}

vector<string> MySQLFetchRow( MYSQL_RES *res )
{
	vector<string> Result;

	MYSQL_ROW Row = mysql_fetch_row( res );

	if( Row )
	{
		unsigned long *Lengths;
		Lengths = mysql_fetch_lengths( res );

		for( unsigned int i = 0; i < mysql_num_fields( res ); i++ )
		{
			if( Row[i] )
				Result.push_back( string( Row[i], Lengths[i] ) );
			else
				Result.push_back( string( ) );
		}
	}

	return Result;
}

string UTIL_ToString( uint32_t i )
{
	string result;
	stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

string UTIL_ToString( float f, int digits )
{
	string result;
	stringstream SS;
	SS << std :: fixed << std :: setprecision( digits ) << f;
	SS >> result;
	return result;
}

uint32_t UTIL_ToUInt32( string &s )
{
	uint32_t result;
	stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

float UTIL_ToFloat( string &s )
{
	float result;
	stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

int main( int argc, char **argv )
{
	string CFGFile = "update_w3mmd_elo.cfg";

	if( argc > 1 && argv[1] )
		CFGFile = argv[1];

	CConfig CFG;
	CFG.Read( CFGFile );
	string Server = CFG.GetString( "db_mysql_server", string( ) );
	string Database = CFG.GetString( "db_mysql_database", "ghost" );
	string User = CFG.GetString( "db_mysql_user", string( ) );
	string Password = CFG.GetString( "db_mysql_password", string( ) );
	int Port = CFG.GetInt( "db_mysql_port", 0 );
	string Category = CFG.GetString( "update_category", string( ) );

	if( Category.empty( ) )
	{
		cout << "no update_category specified in config file" << endl;
		return 1;
	}

	cout << "connecting to database server" << endl;
	MYSQL *Connection = NULL;

	if( !( Connection = mysql_init( NULL ) ) )
	{
		cout << "error: " << mysql_error( Connection ) << endl;
		return 1;
	}

	my_bool Reconnect = true;
	mysql_options( Connection, MYSQL_OPT_RECONNECT, &Reconnect );

	if( !( mysql_real_connect( Connection, Server.c_str( ), User.c_str( ), Password.c_str( ), Database.c_str( ), Port, NULL, 0 ) ) )
	{
		cout << "error: " << mysql_error( Connection ) << endl;
		return 1;
	}

	cout << "connected" << endl;
	cout << "beginning transaction" << endl;

	string QBegin = "BEGIN";

	if( mysql_real_query( Connection, QBegin.c_str( ), QBegin.size( ) ) != 0 )
	{
		cout << "error: " << mysql_error( Connection ) << endl;
		return 1;
	}

	cout << "creating tables" << endl;

	string QCreate1 = "CREATE TABLE IF NOT EXISTS w3mmd_elo_scores ( id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, category VARCHAR(25) NOT NULL, name VARCHAR(15) NOT NULL, server VARCHAR(100) NOT NULL, score REAL NOT NULL )";
	string QCreate2 = "CREATE TABLE IF NOT EXISTS w3mmd_elo_games_scored ( id INT NOT NULL AUTO_INCREMENT PRIMARY KEY, category VARCHAR(25), gameid INT NOT NULL )";

	if( mysql_real_query( Connection, QCreate1.c_str( ), QCreate1.size( ) ) != 0 )
	{
		cout << "error: " << mysql_error( Connection ) << endl;
		return 1;
	}

	if( mysql_real_query( Connection, QCreate2.c_str( ), QCreate2.size( ) ) != 0 )
	{
		cout << "error: " << mysql_error( Connection ) << endl;
		return 1;
	}

	cout << "getting unscored games" << endl;
	queue<uint32_t> UnscoredGames;

	string QSelectUnscored = "SELECT id FROM games WHERE id NOT IN ( SELECT gameid FROM w3mmd_elo_games_scored WHERE category='" + MySQLEscapeString( Connection, Category ) + "' ) ORDER BY id";

	if( mysql_real_query( Connection, QSelectUnscored.c_str( ), QSelectUnscored.size( ) ) != 0 )
	{
		cout << "error: " << mysql_error( Connection ) << endl;
		return 1;
	}
	else
	{
		MYSQL_RES *Result = mysql_store_result( Connection );

		if( Result )
		{
			vector<string> Row = MySQLFetchRow( Result );

			while( !Row.empty( ) )
			{
				UnscoredGames.push( UTIL_ToUInt32( Row[0] ) );
				Row = MySQLFetchRow( Result );
			}

			mysql_free_result( Result );
		}
		else
		{
			cout << "error: " << mysql_error( Connection ) << endl;
			return 1;
		}
	}

	cout << "found " << UnscoredGames.size( ) << " unscored games" << endl;

	while( !UnscoredGames.empty( ) )
	{
		uint32_t GameID = UnscoredGames.front( );
		UnscoredGames.pop( );

		// lowercase the name because there was a bug in GHost++ 13.3 and earlier that didn't automatically lowercase it when using MySQL

		string QSelectPlayers = "SELECT w3mmd_elo_scores.id, LOWER(gameplayers.name), spoofedrealm, flag, practicing, score FROM w3mmdplayers LEFT JOIN gameplayers ON gameplayers.gameid=w3mmdplayers.gameid AND LOWER(gameplayers.name)=LOWER(w3mmdplayers.name) LEFT JOIN w3mmd_elo_scores ON LOWER(w3mmd_elo_scores.name)=LOWER(gameplayers.name) AND server=spoofedrealm WHERE w3mmdplayers.category='" + MySQLEscapeString( Connection, Category ) + "' AND w3mmdplayers.gameid=" + UTIL_ToString( GameID );

		if( mysql_real_query( Connection, QSelectPlayers.c_str( ), QSelectPlayers.size( ) ) != 0 )
		{
			cout << "error: " << mysql_error( Connection ) << endl;
			return 1;
		}
		else
		{
			MYSQL_RES *Result = mysql_store_result( Connection );

			if( Result )
			{
				cout << "gameid " << UTIL_ToString( GameID ) << " found" << endl;

				bool ignore = false;
				bool winner = false;
				uint32_t rowids[12];
				string names[12];
				string servers[12];
				bool exists[12];
				int num_players = 0;
				float player_ratings[12];
				int player_teams[12];
				int num_teams = 0;
				float team_ratings[12];
				float team_winners[12];
				int team_numplayers[12];

				for( int i = 0; i < 12; i++ )
				{
					team_ratings[i] = 0.0;
					team_numplayers[i] = 0;
				}

				vector<string> Row = MySQLFetchRow( Result );

				while( Row.size( ) == 6 )
				{
					// Row[0] = rowid
					// Row[1] = name
					// Row[2] = server
					// Row[3] = flag
					// Row[4] = practicing
					// Row[5] = score

					if( num_players >= 12 )
					{
						cout << "gameid " << UTIL_ToString( GameID ) << " has more than 12 players, ignoring" << endl;
						ignore = true;
						break;
					}

					if( Row[3] == "drawer" )
					{
						cout << "ignoring player [" << Row[1] << "|" << Row[2] << "] because they drew" << endl;
						continue;
					}

					if( Row[4] == "1" )
					{
						cout << "ignoring player [" << Row[1] << "|" << Row[2] << "] because they were practicing" << endl;
						continue;
					}

					if( Row[3] == "winner" )
					{
						// keep track of whether at least one player won or not since we shouldn't score the game if nobody won

						winner = true;

						// note: we pretend each player is on a different team (i.e. it was a free for all)
						// this is because the ELO algorithm requires that each team either all won or all lost as a group
						// however, the MMD system stores win/loss flags on a per player basis and doesn't constrain the flags based on team
						// another option is to throw an error when this is detected and ignore the game completely
						// at this point I'm not sure which option is more correct

						team_winners[num_players] = 1.0;
					}
					else
						team_winners[num_players] = 0.0;

					if( !Row[0].empty( ) )
						rowids[num_players] = UTIL_ToUInt32( Row[0] );
					else
						rowids[num_players] = 0;

					names[num_players] = Row[1];
					servers[num_players] = Row[2];

					if( !Row[5].empty( ) )
					{
						exists[num_players] = true;
						player_ratings[num_players] = UTIL_ToFloat( Row[5] );
					}
					else
					{
						cout << "new player [" << Row[1] << "|" << Row[2] << "] found" << endl;
						exists[num_players] = false;
						player_ratings[num_players] = 1000.0;
					}

					player_teams[num_players] = num_players;
					team_ratings[num_players] = player_ratings[num_players];
					team_numplayers[num_players]++;
					num_players++;
					Row = MySQLFetchRow( Result );
				}

				num_teams = num_players;
				mysql_free_result( Result );

				if( !ignore )
				{
					if( num_players == 0 )
						cout << "gameid " << UTIL_ToString( GameID ) << " has no players or is the wrong category, ignoring" << endl;
					else if( !winner )
						cout << "gameid " << UTIL_ToString( GameID ) << " has no winner, ignoring" << endl;
					else
					{
						cout << "gameid " << UTIL_ToString( GameID ) << " is calculating" << endl;

						float old_player_ratings[12];
						memcpy( old_player_ratings, player_ratings, sizeof( float ) * 12 );
						// team_ratings[0] /= team_numplayers[0];
						// team_ratings[1] /= team_numplayers[1];
						elo_recalculate_ratings( num_players, player_ratings, player_teams, num_teams, team_ratings, team_winners );

						for( int i = 0; i < num_players; i++ )
						{
							cout << "player [" << names[i] << "|" << servers[i] << "] rating " << UTIL_ToString( (uint32_t)old_player_ratings[i] ) << " -> " << UTIL_ToString( (uint32_t)player_ratings[i] ) << endl;

							if( exists[i] )
							{
								string QUpdateScore = "UPDATE w3mmd_elo_scores SET score=" + UTIL_ToString( player_ratings[i], 2 ) + " WHERE id=" + UTIL_ToString( rowids[i] );

								if( mysql_real_query( Connection, QUpdateScore.c_str( ), QUpdateScore.size( ) ) != 0 )
								{
									cout << "error: " << mysql_error( Connection ) << endl;
									return 1;
								}
							}
							else
							{
								string EscCategory = MySQLEscapeString( Connection, Category );
								string EscName = MySQLEscapeString( Connection, names[i] );
								string EscServer = MySQLEscapeString( Connection, servers[i] );
								string QInsertScore = "INSERT INTO w3mmd_elo_scores ( category, name, server, score ) VALUES ( '" + EscCategory + "', '" + EscName + "', '" + EscServer + "', " + UTIL_ToString( player_ratings[i], 2 ) + " )";

								if( mysql_real_query( Connection, QInsertScore.c_str( ), QInsertScore.size( ) ) != 0 )
								{
									cout << "error: " << mysql_error( Connection ) << endl;
									return 1;
								}
							}
						}
					}
				}				
			}
			else
			{
				cout << "error: " << mysql_error( Connection ) << endl;
				return 1;
			}
		}

		string QInsertScored = "INSERT INTO w3mmd_elo_games_scored ( category, gameid ) VALUES ( '" + MySQLEscapeString( Connection, Category ) + "', " + UTIL_ToString( GameID ) + " )";

		if( mysql_real_query( Connection, QInsertScored.c_str( ), QInsertScored.size( ) ) != 0 )
		{
			cout << "error: " << mysql_error( Connection ) << endl;
			return 1;
		}
	}

	cout << "copying w3mmd elo scores to scores table" << endl;

	string QCopyScores1 = "DELETE FROM scores WHERE category='" + MySQLEscapeString( Connection, Category ) + "'";
	string QCopyScores2 = "INSERT INTO scores ( category, name, server, score ) SELECT category, name, server, score FROM w3mmd_elo_scores WHERE category='" + MySQLEscapeString( Connection, Category ) + "'";

	if( mysql_real_query( Connection, QCopyScores1.c_str( ), QCopyScores1.size( ) ) != 0 )
	{
		cout << "error: " << mysql_error( Connection ) << endl;
		return 1;
	}

	if( mysql_real_query( Connection, QCopyScores2.c_str( ), QCopyScores2.size( ) ) != 0 )
	{
		cout << "error: " << mysql_error( Connection ) << endl;
		return 1;
	}

	cout << "committing transaction" << endl;

	string QCommit = "COMMIT";

	if( mysql_real_query( Connection, QCommit.c_str( ), QCommit.size( ) ) != 0 )
	{
		cout << "error: " << mysql_error( Connection ) << endl;
		return 1;
	}

	cout << "done" << endl;
	return 0;
}

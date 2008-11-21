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
#include "language.h"
#include "user.h"
#include "ghostdb.h"
#include "ghostdbsqlite.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "gameprotocol.h"
#include "game.h"

string CGHost :: CmdAddAdminBNET( CUser *user, CBNET *bnet, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageAddAdminBNET( );

	if( m_DB->AdminCheck( bnet->GetServer( ), payload ) )
		return m_Language->UserIsAlreadyAnAdmin( bnet->GetServer( ), payload );
	else
	{
		if( m_DB->AdminAdd( bnet->GetServer( ), payload ) )
			return m_Language->AddedUserToAdminDatabase( bnet->GetServer( ), payload );
		else
			return m_Language->ErrorAddingUserToAdminDatabase( bnet->GetServer( ), payload );
	}
}

string CGHost :: CmdAddBanBNET( CUser *user, CBNET *bnet, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageAddBanBNET( );

	string Victim;
	string Reason;
	stringstream SS;
	SS << payload;
	SS >> Victim;

	if( !SS.eof( ) )
	{
		getline( SS, Reason );
		string :: size_type Start = Reason.find_first_not_of( " " );

		if( Start != string :: npos )
			Reason = Reason.substr( Start );
	}

	CDBBan *Ban = m_DB->BanCheck( bnet->GetServer( ), Victim );

	if( Ban )
	{
		delete Ban;
		return m_Language->UserIsAlreadyBanned( bnet->GetServer( ), Victim );
	}
	else
	{
		if( m_DB->BanAdd( bnet->GetServer( ), Victim, string( ), string( ), user->GetName( ), Reason ) )
			return m_Language->BannedUser( bnet->GetServer( ), Victim );
		else
			return m_Language->ErrorBanningUser( bnet->GetServer( ), Victim );
	}
}

string CGHost :: CmdAnnounce( CUser *user, CBaseGame *game, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageAnnounce( );

	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	if( payload == "off" )
	{
		game->SetAnnounce( 0, string( ) );
		return m_Language->AnnounceMessageDisabled( );
	}
	else
	{
		uint32_t Interval;
		string Message;
		stringstream SS;
		SS << payload;
		SS >> Interval;

		if( SS.fail( ) || Interval == 0 || SS.eof( ) )
			return m_Language->UsageAnnounce( );

		getline( SS, Message );
		string :: size_type Start = Message.find_first_not_of( " " );

		if( Start != string :: npos )
			Message = Message.substr( Start );

		game->SetAnnounce( Interval, Message );
		return m_Language->AnnounceMessageEnabled( );
	}
}

string CGHost :: CmdAutoHost( CUser *user, string owner, string server, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageAutoHost( );

	if( payload == "off" )
	{
		m_AutoHostGameName.clear( );
		m_AutoHostMapCFG.clear( );
		m_AutoHostOwner.clear( );
		m_AutoHostServer.clear( );
		m_AutoHostMaximumGames = 0;
		m_AutoHostAutoStartPlayers = 0;
		m_LastAutoHostTime = GetTime( );
		return m_Language->AutoHostDisabled( );
	}
	else
	{
		uint32_t MaximumGames;
		uint32_t AutoStartPlayers;
		string GameName;
		stringstream SS;
		SS << payload;
		SS >> MaximumGames;

		if( SS.fail( ) || MaximumGames == 0 )
			return m_Language->UsageAutoHost( );

		SS >> AutoStartPlayers;

		if( SS.fail( ) || AutoStartPlayers == 0 || SS.eof( ) )
			return m_Language->UsageAutoHost( );

		getline( SS, GameName );
		string :: size_type Start = GameName.find_first_not_of( " " );

		if( Start != string :: npos )
			GameName = GameName.substr( Start );

		m_AutoHostGameName = GameName;
		m_AutoHostMapCFG = m_Map->GetCFGFile( );
		m_AutoHostOwner = owner;
		m_AutoHostServer = server;
		m_AutoHostMaximumGames = MaximumGames;
		m_AutoHostAutoStartPlayers = AutoStartPlayers;
		m_LastAutoHostTime = GetTime( );
		return m_Language->AutoHostEnabled( );
	}
}

string CGHost :: CmdAutoStart( CUser *user, CBaseGame *game, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageAutoStart( );

	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	if( payload == "off" )
	{
		game->SetAutoStartPlayers( 0 );
		return m_Language->AutoStartDisabled( );
	}
	else
	{
		uint32_t AutoStartPlayers;
		stringstream SS;
		SS << payload;
		SS >> AutoStartPlayers;

		if( SS.fail( ) || AutoStartPlayers == 0 )
			return m_Language->UsageAutoStart( );

		game->SetAutoStartPlayers( AutoStartPlayers );
		return m_Language->AutoStartEnabled( UTIL_ToString( AutoStartPlayers ) );
	}
}

string CGHost :: CmdChannel( CUser *user, CBNET *bnet, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageChannel( );

	bnet->QueueChatCommand( "/join " + payload );
	return string( );
}

string CGHost :: CmdCheckAdminBNET( CUser *user, CBNET *bnet, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageCheckAdminBNET( );

	if( m_DB->AdminCheck( bnet->GetServer( ), payload ) )
		return m_Language->UserIsAnAdmin( bnet->GetServer( ), payload );
	else
		return m_Language->UserIsNotAnAdmin( bnet->GetServer( ), payload );
}

string CGHost :: CmdCheckBanBNET( CUser *user, CBNET *bnet, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageCheckBanBNET( );

	CDBBan *Ban = m_DB->BanCheck( bnet->GetServer( ), payload );

	if( Ban )
	{
		string Result = m_Language->UserWasBannedOnByBecause( bnet->GetServer( ), payload, Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) );
		delete Ban;
		return Result;
	}
	else
		return m_Language->UserIsNotBanned( bnet->GetServer( ), payload );
}

string CGHost :: CmdClose( CUser *user, CBNET *bnet, CBaseGame *game, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageClose( );

	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	if( game->GetLocked( ) )
		return m_Language->TheGameIsLockedBNET( );

	stringstream SS;
	SS << payload;

	while( !SS.eof( ) )
	{
		uint32_t SID;
		SS >> SID;

		if( SS.fail( ) )
			return m_Language->UsageClose( );
		else
			game->CloseSlot( (unsigned char)( SID - 1 ), true );
	}

	return string( );
}

string CGHost :: CmdCloseAll( CUser *user, CBaseGame *game )
{
	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	if( game->GetLocked( ) )
		return m_Language->TheGameIsLockedBNET( );

	game->CloseAllSlots( );
	return string( );
}

string CGHost :: CmdCountAdmins( CUser *user, string server )
{
	uint32_t Count = m_DB->AdminCount( server );

	if( Count == 0 )
		return m_Language->ThereAreNoAdmins( server );
	else if( Count == 1 )
		return m_Language->ThereIsAdmin( server );
	else
		return m_Language->ThereAreAdmins( server, UTIL_ToString( Count ) );
}

string CGHost :: CmdCountBans( CUser *user, string server )
{
	uint32_t Count = m_DB->BanCount( server );

	if( Count == 0 )
		return m_Language->ThereAreNoBannedUsers( server );
	else if( Count == 1 )
		return m_Language->ThereIsBannedUser( server );
	else
		return m_Language->ThereAreBannedUsers( server, UTIL_ToString( Count ) );
}

string CGHost :: CmdDelAdminBNET( CUser *user, CBNET *bnet, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageDelAdminBNET( );

	if( !m_DB->AdminCheck( bnet->GetServer( ), payload ) )
		return m_Language->UserIsNotAnAdmin( bnet->GetServer( ), payload );
	else
	{
		if( m_DB->AdminRemove( bnet->GetServer( ), payload ) )
			return m_Language->DeletedUserFromAdminDatabase( bnet->GetServer( ), payload );
		else
			return m_Language->ErrorDeletingUserFromAdminDatabase( bnet->GetServer( ), payload );
	}
}

string CGHost :: CmdDelBan( CUser *user, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageDelBan( );

	if( m_DB->BanRemove( payload ) )
		return m_Language->UnbannedUser( payload );
	else
		return m_Language->ErrorUnbanningUser( payload );
}

string CGHost :: CmdDisable( CUser *user )
{
	m_Enabled = false;
	return m_Language->BotDisabled( );
}

string CGHost :: CmdEnable( CUser *user )
{
	m_Enabled = true;
	return m_Language->BotEnabled( );
}

string CGHost :: CmdEndGame( CUser *user, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageEndGame( );

	uint32_t GameNumber = UTIL_ToUInt32( payload ) - 1;

	if( GameNumber < m_Games.size( ) )
	{
		m_Games[GameNumber]->StopPlayers( "was disconnected (admin ended game)" );
		return m_Language->EndingGame( m_Games[GameNumber]->GetDescription( ) );
	}
	else
		return m_Language->GameNumberDoesntExist( payload );
}

string CGHost :: CmdExit( CUser *user, string payload )
{
	if( payload == "force" )
		m_Exiting = true;
	else
	{
		if( m_CurrentGame || !m_Games.empty( ) )
			return m_Language->AtLeastOneGameActiveUseForceToShutdown( );
		else
			m_Exiting = true;
	}

	return string( );
}

string CGHost :: CmdGetClan( CUser *user, CBNET *bnet )
{
	bnet->SendGetClanList( );
	return string( );
}

string CGHost :: CmdGetFriends( CUser *user, CBNET *bnet )
{
	bnet->SendGetFriendsList( );
	return string( );
}

string CGHost :: CmdGetGame( CUser *user, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageGetGame( );

	uint32_t GameNumber = UTIL_ToUInt32( payload ) - 1;

	if( GameNumber < m_Games.size( ) )
		return m_Language->GameNumberIs( payload, m_Games[GameNumber]->GetDescription( ) );
	else
		return m_Language->GameNumberDoesntExist( payload );
}

string CGHost :: CmdGetGames( CUser *user )
{
	if( m_CurrentGame )
		return m_Language->GameIsInTheLobby( m_CurrentGame->GetDescription( ), UTIL_ToString( m_Games.size( ) ), UTIL_ToString( m_MaxGames ) );
	else
		return m_Language->ThereIsNoGameInTheLobby( UTIL_ToString( m_Games.size( ) ), UTIL_ToString( m_MaxGames ) );
}

string CGHost :: CmdHold( CUser *user, CBaseGame *game, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageHold( );

	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	stringstream SS;
	SS << payload;

	while( !SS.eof( ) )
	{
		string HoldName;
		SS >> HoldName;

		if( SS.fail( ) )
			return m_Language->UsageHold( );
		else
		{
			game->AddToReserved( HoldName );
			// return m_Language->AddedPlayerToTheHoldList( HoldName );
		}
	}
}

string CGHost :: CmdHostSG( CUser *user, string creator, string server, string payload, bool whisper )
{
	if( payload.empty( ) )
		return m_Language->UsageHostSG( );

	CreateGame( GAME_PRIVATE, true, payload, creator, creator, server, whisper );
	return string( );
}

string CGHost :: CmdLoad( CUser *user, string payload )
{
	if( payload.empty( ) )
		return m_Language->CurrentlyLoadedMapCFGIs( m_Map->GetCFGFile( ) );

	// only load files in the current directory just to be safe

	if( payload.find( "/" ) != string :: npos || payload.find( "\\" ) != string :: npos )
		return m_Language->UnableToLoadConfigFilesOutside( );

	string File = m_MapCFGPath + payload + ".cfg";

	if( UTIL_FileExists( File ) )
	{
		// we have to be careful here because we didn't copy the map data when creating the game (there's only one global copy)
		// therefore if we change the map data while a game is in the lobby everything will get screwed up
		// the easiest solution is to simply reject the command if a game is in the lobby

		if( m_CurrentGame )
			return m_Language->UnableToLoadConfigFileGameInLobby( );

		CConfig MapCFG;
		MapCFG.Read( File );
		m_Map->Load( &MapCFG, File );
		return m_Language->LoadingConfigFile( File );
	}
	else
		return m_Language->UnableToLoadConfigFileDoesntExist( File );
}

string CGHost :: CmdLoadSG( CUser *user, string payload )
{
	if( payload.empty( ) )
		return m_Language->CurrentlyLoadedSaveGameIs( m_SaveGame->GetFileName( ) );

	// only load files in the current directory just to be safe

	if( payload.find( "/" ) != string :: npos || payload.find( "\\" ) != string :: npos )
		return m_Language->UnableToLoadSaveGamesOutside( );

	string File = m_SaveGamePath + payload + ".w3z";
	string FileNoPath = payload + ".w3z";

	if( UTIL_FileExists( File ) )
	{
		if( m_CurrentGame )
			return m_Language->UnableToLoadSaveGameGameInLobby( );

		m_SaveGame->Load( File, false );
		m_SaveGame->ParseSaveGame( );
		m_SaveGame->SetFileName( File );
		m_SaveGame->SetFileNameNoPath( FileNoPath );
		return m_Language->LoadingSaveGame( File );
	}
	else
		return m_Language->UnableToLoadSaveGameDoesntExist( File );
}

string CGHost :: CmdOpen( CUser *user, CBNET *bnet, CBaseGame *game, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageOpen( );

	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	if( game->GetLocked( ) )
		return m_Language->TheGameIsLockedBNET( );

	stringstream SS;
	SS << payload;

	while( !SS.eof( ) )
	{
		uint32_t SID;
		SS >> SID;

		if( SS.fail( ) )
			return m_Language->UsageOpen( );
		else
			game->OpenSlot( (unsigned char)( SID - 1 ), true );
	}

	return string( );
}

string CGHost :: CmdOpenAll( CUser *user, CBaseGame *game )
{
	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	if( game->GetLocked( ) )
		return m_Language->TheGameIsLockedBNET( );

	game->OpenAllSlots( );
	return string( );
}

string CGHost :: CmdPriv( CUser *user, string creator, string server, string payload, bool whisper )
{
	if( payload.empty( ) )
		return m_Language->UsagePriv( );

	CreateGame( GAME_PRIVATE, false, payload, creator, creator, server, whisper );
	return string( );
}

string CGHost :: CmdPrivBy( CUser *user, string creator, string server, string payload, bool whisper )
{
	if( payload.empty( ) )
		return m_Language->UsagePrivBy( );

	string Owner;
	string GameName;
	stringstream SS;
	SS << payload;
	SS >> Owner;

	if( SS.fail( ) || SS.eof( ) )
		return m_Language->UsagePrivBy( );

	getline( SS, GameName );
	string :: size_type Start = GameName.find_first_not_of( " " );

	if( Start != string :: npos )
		GameName = GameName.substr( Start );

	CreateGame( GAME_PRIVATE, false, GameName, Owner, creator, server, whisper );
	return string( );
}

string CGHost :: CmdPub( CUser *user, string creator, string server, string payload, bool whisper )
{
	if( payload.empty( ) )
		return m_Language->UsagePub( );

	CreateGame( GAME_PUBLIC, false, payload, creator, creator, server, whisper );
	return string( );
}

string CGHost :: CmdPubBy( CUser *user, string creator, string server, string payload, bool whisper )
{
	if( payload.empty( ) )
		return m_Language->UsagePubBy( );

	string Owner;
	string GameName;
	stringstream SS;
	SS << payload;
	SS >> Owner;

	if( SS.fail( ) || SS.eof( ) )
		return m_Language->UsagePubBy( );

	getline( SS, GameName );
	string :: size_type Start = GameName.find_first_not_of( " " );

	if( Start != string :: npos )
		GameName = GameName.substr( Start );

	CreateGame( GAME_PUBLIC, false, GameName, Owner, creator, server, whisper );
	return string( );
}

string CGHost :: CmdSay( CUser *user, CBNET *bnet, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageSay( );

	bnet->QueueChatCommand( payload );
	return string( );
}

string CGHost :: CmdSayGame( CUser *user, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageSayGame( );

	uint32_t GameNumber;
	string Message;
	stringstream SS;
	SS << payload;
	SS >> GameNumber;

	if( SS.fail( ) || SS.eof( ) )
		return m_Language->UsageSayGame( );

	getline( SS, Message );
	string :: size_type Start = Message.find_first_not_of( " " );

	if( Start != string :: npos )
		Message = Message.substr( Start );

	if( GameNumber - 1 < m_Games.size( ) )
	{
		m_Games[GameNumber - 1]->SendAllChat( "ADMIN: " + Message );
		return string( );
	}
	else
		return m_Language->GameNumberDoesntExist( UTIL_ToString( GameNumber ) );
}

string CGHost :: CmdSayGames( CUser *user, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageSayGames( );

	if( m_CurrentGame )
		m_CurrentGame->SendAllChat( payload );

	for( vector<CBaseGame *> :: iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
		(*i)->SendAllChat( "ADMIN: " + payload );

	return string( );
}

string CGHost :: CmdSP( CUser *user, CBaseGame *game )
{
	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	if( game->GetLocked( ) )
		return m_Language->TheGameIsLockedBNET( );

	game->ShuffleSlots( );
	return m_Language->ShufflingPlayers( );
}

string CGHost :: CmdStart( CUser *user, CBaseGame *game, string payload )
{
	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	if( game->GetLocked( ) )
		return m_Language->TheGameIsLockedBNET( );

	// if the player sent "!start force" skip the checks and start the countdown
	// otherwise check that the game is ready to start

	if( payload == "force" )
		game->StartCountDown( true );
	else
		game->StartCountDown( false );

	return string( );
}

string CGHost :: CmdStats( CUser *user, string payload )
{
	string StatsUser = user->GetName( );

	if( !payload.empty( ) )
		StatsUser = payload;

	CDBGamePlayerSummary *GamePlayerSummary = m_DB->GamePlayerSummaryCheck( StatsUser );

	if( GamePlayerSummary )
	{
		string Result = m_Language->HasPlayedGamesWithThisBot( StatsUser, GamePlayerSummary->GetFirstGameDateTime( ), GamePlayerSummary->GetLastGameDateTime( ), UTIL_ToString( GamePlayerSummary->GetTotalGames( ) ), UTIL_ToString( (float)GamePlayerSummary->GetAvgLoadingTime( ) / 1000, 2 ), UTIL_ToString( GamePlayerSummary->GetAvgLeftPercent( ) ) );
		delete GamePlayerSummary;
		return Result;
	}
	else
		return m_Language->HasntPlayedGamesWithThisBot( StatsUser );
}

string CGHost :: CmdStatsDotA( CUser *user, string payload )
{
	string StatsUser = user->GetName( );

	if( !payload.empty( ) )
		StatsUser = payload;

	CDBDotAPlayerSummary *DotAPlayerSummary = m_DB->DotAPlayerSummaryCheck( StatsUser );

	if( DotAPlayerSummary )
	{
		string Result = m_Language->HasPlayedDotAGamesWithThisBot( StatsUser, UTIL_ToString( DotAPlayerSummary->GetTotalGames( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalWins( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalLosses( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalKills( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalDeaths( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalCreepKills( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalCreepDenies( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalAssists( ) ), UTIL_ToString( DotAPlayerSummary->GetTotalNeutralKills( ) ) );
		delete DotAPlayerSummary;
		return Result;
	}
	else
		return m_Language->HasntPlayedDotAGamesWithThisBot( StatsUser );
}

string CGHost :: CmdSwap( CUser *user, CBaseGame *game, string payload )
{
	if( payload.empty( ) )
		return m_Language->UsageSwap( );

	if( !game || game->GetCountDownStarted( ) )
		return m_Language->GameNotFoundOrAlreadyStarted( );

	if( game->GetLocked( ) )
		return m_Language->TheGameIsLockedBNET( );

	uint32_t SID1;
	uint32_t SID2;
	stringstream SS;
	SS << payload;
	SS >> SID1;

	if( SS.fail( ) || SS.eof( ) )
		return m_Language->UsageSwap( );

	SS >> SID2;

	if( SS.fail( ) )
		return m_Language->UsageSwap( );
	else
	{
		game->SwapSlots( (unsigned char)( SID1 - 1 ), (unsigned char)( SID2 - 1 ) );
		return string( );
	}
}

string CGHost :: CmdUnhost( CUser *user, CBaseGame *game )
{
	if( !game )
		return m_Language->UnableToUnhostGameNoGameInLobby( );

	if( game->GetCountDownStarted( ) )
		return m_Language->UnableToUnhostGameCountdownStarted( game->GetDescription( ) );

	game->SetExiting( true );
	return m_Language->UnhostingGame( game->GetDescription( ) );
}

string CGHost :: CmdVersion( CUser *user )
{
	return m_Language->VersionAdmin( m_Version );
}

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
#include "config.h"
#include "language.h"

//
// CLanguage
//

CLanguage :: CLanguage( string nCFGFile )
{
	m_CFG = new CConfig( );
	m_CFG->Read( nCFGFile );
}

CLanguage :: ~CLanguage( )
{
	delete m_CFG;
}

void CLanguage :: Replace( string &Text, string Key, string Value )
{
	// don't allow any infinite loops

	if( Value.find( Key ) != string :: npos )
		return;

	string :: size_type KeyStart = Text.find( Key );

	while( KeyStart != string :: npos )
	{
		Text.replace( KeyStart, Key.size( ), Value );
		KeyStart = Text.find( Key );
	}
}

string CLanguage :: UnableToCreateGameTryAnotherName( string server, string gamename )
{
	string Out = m_CFG->GetString( "lang_0001", "lang_0001" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

string CLanguage :: UserIsAlreadyAnAdmin( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0002", "lang_0002" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: AddedUserToAdminDatabase( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0003", "lang_0003" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: ErrorAddingUserToAdminDatabase( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0004", "lang_0004" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: YouDontHaveAccessToThatCommand( )
{
	return m_CFG->GetString( "lang_0005", "lang_0005" );
}

string CLanguage :: UserIsAlreadyBanned( string server, string victim )
{
	string Out = m_CFG->GetString( "lang_0006", "lang_0006" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: BannedUser( string server, string victim )
{
	string Out = m_CFG->GetString( "lang_0007", "lang_0007" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: ErrorBanningUser( string server, string victim )
{
	string Out = m_CFG->GetString( "lang_0008", "lang_0008" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: UserIsAnAdmin( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0009", "lang_0009" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UserIsNotAnAdmin( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0010", "lang_0010" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UserWasBannedOnByBecause( string server, string victim, string date, string admin, string reason )
{
	string Out = m_CFG->GetString( "lang_0011", "lang_0011" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$DATE$", date );
	Replace( Out, "$ADMIN$", admin );
	Replace( Out, "$REASON$", reason );
	return Out;
}

string CLanguage :: UserIsNotBanned( string server, string victim )
{
	string Out = m_CFG->GetString( "lang_0012", "lang_0012" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: ThereAreNoAdmins( string server )
{
	string Out = m_CFG->GetString( "lang_0013", "lang_0013" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ThereIsAdmin( string server )
{
	string Out = m_CFG->GetString( "lang_0014", "lang_0014" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ThereAreAdmins( string server, string count )
{
	string Out = m_CFG->GetString( "lang_0015", "lang_0015" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$COUNT$", count );
	return Out;
}

string CLanguage :: ThereAreNoBannedUsers( string server )
{
	string Out = m_CFG->GetString( "lang_0016", "lang_0016" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ThereIsBannedUser( string server )
{
	string Out = m_CFG->GetString( "lang_0017", "lang_0017" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ThereAreBannedUsers( string server, string count )
{
	string Out = m_CFG->GetString( "lang_0018", "lang_0018" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$COUNT$", count );
	return Out;
}

string CLanguage :: YouCantDeleteTheRootAdmin( )
{
	return m_CFG->GetString( "lang_0019", "lang_0019" );
}

string CLanguage :: DeletedUserFromAdminDatabase( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0020", "lang_0020" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: ErrorDeletingUserFromAdminDatabase( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0021", "lang_0021" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UnbannedUser( string victim )
{
	string Out = m_CFG->GetString( "lang_0022", "lang_0022" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: ErrorUnbanningUser( string victim )
{
	string Out = m_CFG->GetString( "lang_0023", "lang_0023" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: GameNumberIs( string number, string description )
{
	string Out = m_CFG->GetString( "lang_0024", "lang_0024" );
	Replace( Out, "$NUMBER$", number );
	Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

string CLanguage :: GameNumberDoesntExist( string number )
{
	string Out = m_CFG->GetString( "lang_0025", "lang_0025" );
	Replace( Out, "$NUMBER$", number );
	return Out;
}

string CLanguage :: GameIsInTheLobby( string description, string current, string max )
{
	string Out = m_CFG->GetString( "lang_0026", "lang_0026" );
	Replace( Out, "$DESCRIPTION$", description );
	Replace( Out, "$CURRENT$", current );
	Replace( Out, "$MAX$", max );
	return Out;
}

string CLanguage :: ThereIsNoGameInTheLobby( string current, string max )
{
	string Out = m_CFG->GetString( "lang_0027", "lang_0027" );
	Replace( Out, "$CURRENT$", current );
	Replace( Out, "$MAX$", max );
	return Out;
}

string CLanguage :: UnableToLoadConfigFilesOutside( )
{
	return m_CFG->GetString( "lang_0028", "lang_0028" );
}

string CLanguage :: LoadingConfigFile( string file )
{
	string Out = m_CFG->GetString( "lang_0029", "lang_0029" );
	Replace( Out, "$FILE$", file );
	return Out;
}

string CLanguage :: UnableToLoadConfigFileDoesntExist( string file )
{
	string Out = m_CFG->GetString( "lang_0030", "lang_0030" );
	Replace( Out, "$FILE$", file );
	return Out;
}

string CLanguage :: CreatingPrivateGame( string gamename, string user )
{
	string Out = m_CFG->GetString( "lang_0031", "lang_0031" );
	Replace( Out, "$GAMENAME$", gamename );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: CreatingPublicGame( string gamename, string user )
{
	string Out = m_CFG->GetString( "lang_0032", "lang_0032" );
	Replace( Out, "$GAMENAME$", gamename );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UnableToUnhostGameCountdownStarted( string description )
{
	string Out = m_CFG->GetString( "lang_0033", "lang_0033" );
	Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

string CLanguage :: UnhostingGame( string description )
{
	string Out = m_CFG->GetString( "lang_0034", "lang_0034" );
	Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

string CLanguage :: UnableToUnhostGameNoGameInLobby( )
{
	return m_CFG->GetString( "lang_0035", "lang_0035" );
}

string CLanguage :: VersionAdmin( string version )
{
	string Out = m_CFG->GetString( "lang_0036", "lang_0036" );
	Replace( Out, "$VERSION$", version );
	return Out;
}

string CLanguage :: VersionNotAdmin( string version )
{
	string Out = m_CFG->GetString( "lang_0037", "lang_0037" );
	Replace( Out, "$VERSION$", version );
	return Out;
}

string CLanguage :: UnableToCreateGameAnotherGameInLobby( string gamename, string description )
{
	string Out = m_CFG->GetString( "lang_0038", "lang_0038" );
	Replace( Out, "$GAMENAME$", gamename );
	Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

string CLanguage :: UnableToCreateGameMaxGamesReached( string gamename, string max )
{
	string Out = m_CFG->GetString( "lang_0039", "lang_0039" );
	Replace( Out, "$GAMENAME$", gamename );
	Replace( Out, "$MAX$", max );
	return Out;
}

string CLanguage :: GameIsOver( string description )
{
	string Out = m_CFG->GetString( "lang_0040", "lang_0040" );
	Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

string CLanguage :: SpoofCheckByReplying( )
{
	return m_CFG->GetString( "lang_0041", "lang_0041" );
}

string CLanguage :: GameRefreshed( )
{
	return m_CFG->GetString( "lang_0042", "lang_0042" );
}

string CLanguage :: SpoofPossibleIsAway( string user )
{
	string Out = m_CFG->GetString( "lang_0043", "lang_0043" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: SpoofPossibleIsUnavailable( string user )
{
	string Out = m_CFG->GetString( "lang_0044", "lang_0044" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: SpoofPossibleIsRefusingMessages( string user )
{
	string Out = m_CFG->GetString( "lang_0045", "lang_0045" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: SpoofDetectedIsNotInGame( string user )
{
	string Out = m_CFG->GetString( "lang_0046", "lang_0046" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: SpoofDetectedIsInPrivateChannel( string user )
{
	string Out = m_CFG->GetString( "lang_0047", "lang_0047" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: SpoofDetectedIsInAnotherGame( string user )
{
	string Out = m_CFG->GetString( "lang_0048", "lang_0048" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: CountDownAborted( )
{
	return m_CFG->GetString( "lang_0049", "lang_0049" );
}

string CLanguage :: TryingToJoinTheGameButBanned( string victim )
{
	string Out = m_CFG->GetString( "lang_0050", "lang_0050" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: UnableToBanNoMatchesFound( string victim )
{
	string Out = m_CFG->GetString( "lang_0051", "lang_0051" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: PlayerWasBannedByPlayer( string victim, string user )
{
	string Out = m_CFG->GetString( "lang_0052", "lang_0052" );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UnableToBanFoundMoreThanOneMatch( string victim )
{
	string Out = m_CFG->GetString( "lang_0053", "lang_0053" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: AddedPlayerToTheHoldList( string user )
{
	string Out = m_CFG->GetString( "lang_0054", "lang_0054" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UnableToKickNoMatchesFound( string victim )
{
	string Out = m_CFG->GetString( "lang_0055", "lang_0055" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: UnableToKickFoundMoreThanOneMatch( string victim )
{
	string Out = m_CFG->GetString( "lang_0056", "lang_0056" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: SettingLatencyToMinimum( string min )
{
	string Out = m_CFG->GetString( "lang_0057", "lang_0057" );
	Replace( Out, "$MIN$", min );
	return Out;
}

string CLanguage :: SettingLatencyToMaximum( string max )
{
	string Out = m_CFG->GetString( "lang_0058", "lang_0058" );
	Replace( Out, "$MAX$", max );
	return Out;
}

string CLanguage :: SettingLatencyTo( string latency )
{
	string Out = m_CFG->GetString( "lang_0059", "lang_0059" );
	Replace( Out, "$LATENCY$", latency );
	return Out;
}

string CLanguage :: KickingPlayersWithPingsGreaterThan( string total, string ping )
{
	string Out = m_CFG->GetString( "lang_0060", "lang_0060" );
	Replace( Out, "$TOTAL$", total );
	Replace( Out, "$PING$", ping );
	return Out;
}

string CLanguage :: HasPlayedGamesWithThisBot( string user, string firstgame, string lastgame, string totalgames, string avgloadingtime, string avgstay )
{
	string Out = m_CFG->GetString( "lang_0061", "lang_0061" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$FIRSTGAME$", firstgame );
	Replace( Out, "$LASTGAME$", lastgame );
	Replace( Out, "$TOTALGAMES$", totalgames );
	Replace( Out, "$AVGLOADINGTIME$", avgloadingtime );
	Replace( Out, "$AVGSTAY$", avgstay );
	return Out;
}

string CLanguage :: HasntPlayedGamesWithThisBot( string user )
{
	string Out = m_CFG->GetString( "lang_0062", "lang_0062" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: AutokickingPlayerForExcessivePing( string victim, string ping )
{
	string Out = m_CFG->GetString( "lang_0063", "lang_0063" );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$PING$", ping );
	return Out;
}

string CLanguage :: SpoofCheckAcceptedFor( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0064", "lang_0064" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: PlayersNotYetSpoofChecked( string notspoofchecked )
{
	string Out = m_CFG->GetString( "lang_0065", "lang_0065" );
	Replace( Out, "$NOTSPOOFCHECKED$", notspoofchecked );
	return Out;
}

string CLanguage :: ManuallySpoofCheckByWhispering( string hostname )
{
	string Out = m_CFG->GetString( "lang_0066", "lang_0066" );
	Replace( Out, "$HOSTNAME$", hostname );
	return Out;
}

string CLanguage :: SpoofCheckByWhispering( string hostname )
{
	string Out = m_CFG->GetString( "lang_0067", "lang_0067" );
	Replace( Out, "$HOSTNAME$", hostname );
	return Out;
}

string CLanguage :: EveryoneHasBeenSpoofChecked( )
{
	return m_CFG->GetString( "lang_0068", "lang_0068" );
}

string CLanguage :: PlayersNotYetPinged( string notpinged )
{
	string Out = m_CFG->GetString( "lang_0069", "lang_0069" );
	Replace( Out, "$NOTPINGED$", notpinged );
	return Out;
}

string CLanguage :: EveryoneHasBeenPinged( )
{
	return m_CFG->GetString( "lang_0070", "lang_0070" );
}

string CLanguage :: ShortestLoadByPlayer( string user, string loadingtime )
{
	string Out = m_CFG->GetString( "lang_0071", "lang_0071" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$LOADINGTIME$", loadingtime );
	return Out;
}

string CLanguage :: LongestLoadByPlayer( string user, string loadingtime )
{
	string Out = m_CFG->GetString( "lang_0072", "lang_0072" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$LOADINGTIME$", loadingtime );
	return Out;
}

string CLanguage :: YourLoadingTimeWas( string loadingtime )
{
	string Out = m_CFG->GetString( "lang_0073", "lang_0073" );
	Replace( Out, "$LOADINGTIME$", loadingtime );
	return Out;
}

string CLanguage :: HasPlayedDotAGamesWithThisBot( string user, string totalgames, string totalwins, string totallosses, string totalkills, string totaldeaths, string totalcreepkills, string totalcreepdenies, string totalassists, string totalneutralkills, string totaltowerkills, string totalraxkills, string totalcourierkills, string avgkills, string avgdeaths, string avgcreepkills, string avgcreepdenies, string avgassists, string avgneutralkills, string avgtowerkills, string avgraxkills, string avgcourierkills )
{
	string Out = m_CFG->GetString( "lang_0074", "lang_0074" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$TOTALGAMES$", totalgames );
	Replace( Out, "$TOTALWINS$", totalwins );
	Replace( Out, "$TOTALLOSSES$", totallosses );
	Replace( Out, "$TOTALKILLS$", totalkills );
	Replace( Out, "$TOTALDEATHS$", totaldeaths );
	Replace( Out, "$TOTALCREEPKILLS$", totalcreepkills );
	Replace( Out, "$TOTALCREEPDENIES$", totalcreepdenies );
	Replace( Out, "$TOTALASSISTS$", totalassists );
	Replace( Out, "$TOTALNEUTRALKILLS$", totalneutralkills );
	Replace( Out, "$TOTALTOWERKILLS$", totaltowerkills );
	Replace( Out, "$TOTALRAXKILLS$", totalraxkills );
	Replace( Out, "$TOTALCOURIERKILLS$", totalcourierkills );
	Replace( Out, "$AVGKILLS$", avgkills );
	Replace( Out, "$AVGDEATHS$", avgdeaths );
	Replace( Out, "$AVGCREEPKILLS$", avgcreepkills );
	Replace( Out, "$AVGCREEPDENIES$", avgcreepdenies );
	Replace( Out, "$AVGASSISTS$", avgassists );
	Replace( Out, "$AVGNEUTRALKILLS$", avgneutralkills );
	Replace( Out, "$AVGTOWERKILLS$", avgtowerkills );
	Replace( Out, "$AVGRAXKILLS$", avgraxkills );
	Replace( Out, "$AVGCOURIERKILLS$", avgcourierkills );
	return Out;
}

string CLanguage :: HasntPlayedDotAGamesWithThisBot( string user )
{
	string Out = m_CFG->GetString( "lang_0075", "lang_0075" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: WasKickedForReservedPlayer( string reserved )
{
	string Out = m_CFG->GetString( "lang_0076", "lang_0076" );
	Replace( Out, "$RESERVED$", reserved );
	return Out;
}

string CLanguage :: WasKickedForOwnerPlayer( string owner )
{
	string Out = m_CFG->GetString( "lang_0077", "lang_0077" );
	Replace( Out, "$OWNER$", owner );
	return Out;
}

string CLanguage :: WasKickedByPlayer( string user )
{
	string Out = m_CFG->GetString( "lang_0078", "lang_0078" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: HasLostConnectionPlayerError( string error )
{
	string Out = m_CFG->GetString( "lang_0079", "lang_0079" );
	Replace( Out, "$ERROR$", error );
	return Out;
}

string CLanguage :: HasLostConnectionSocketError( string error )
{
	string Out = m_CFG->GetString( "lang_0080", "lang_0080" );
	Replace( Out, "$ERROR$", error );
	return Out;
}

string CLanguage :: HasLostConnectionClosedByRemoteHost( )
{
	return m_CFG->GetString( "lang_0081", "lang_0081" );
}

string CLanguage :: HasLeftVoluntarily( )
{
	return m_CFG->GetString( "lang_0082", "lang_0082" );
}

string CLanguage :: EndingGame( string description )
{
	string Out = m_CFG->GetString( "lang_0083", "lang_0083" );
	Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

string CLanguage :: HasLostConnectionTimedOut( )
{
	return m_CFG->GetString( "lang_0084", "lang_0084" );
}

string CLanguage :: GlobalChatMuted( )
{
	return m_CFG->GetString( "lang_0085", "lang_0085" );
}

string CLanguage :: GlobalChatUnmuted( )
{
	return m_CFG->GetString( "lang_0086", "lang_0086" );
}

string CLanguage :: ShufflingPlayers( )
{
	return m_CFG->GetString( "lang_0087", "lang_0087" );
}

string CLanguage :: UnableToLoadConfigFileGameInLobby( )
{
	return m_CFG->GetString( "lang_0088", "lang_0088" );
}

string CLanguage :: PlayersStillDownloading( string stilldownloading )
{
	string Out = m_CFG->GetString( "lang_0089", "lang_0089" );
	Replace( Out, "$STILLDOWNLOADING$", stilldownloading );
	return Out;
}

string CLanguage :: RefreshMessagesEnabled( )
{
	return m_CFG->GetString( "lang_0090", "lang_0090" );
}

string CLanguage :: RefreshMessagesDisabled( )
{
	return m_CFG->GetString( "lang_0091", "lang_0091" );
}

string CLanguage :: AtLeastOneGameActiveUseForceToShutdown( )
{
	return m_CFG->GetString( "lang_0092", "lang_0092" );
}

string CLanguage :: CurrentlyLoadedMapCFGIs( string mapcfg )
{
	string Out = m_CFG->GetString( "lang_0093", "lang_0093" );
	Replace( Out, "$MAPCFG$", mapcfg );
	return Out;
}

string CLanguage :: LaggedOutDroppedByAdmin( )
{
	return m_CFG->GetString( "lang_0094", "lang_0094" );
}

string CLanguage :: LaggedOutDroppedByVote( )
{
	return m_CFG->GetString( "lang_0095", "lang_0095" );
}

string CLanguage :: PlayerVotedToDropLaggers( string user )
{
	string Out = m_CFG->GetString( "lang_0096", "lang_0096" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: LatencyIs( string latency )
{
	string Out = m_CFG->GetString( "lang_0097", "lang_0097" );
	Replace( Out, "$LATENCY$", latency );
	return Out;
}

string CLanguage :: SyncLimitIs( string synclimit )
{
	string Out = m_CFG->GetString( "lang_0098", "lang_0098" );
	Replace( Out, "$SYNCLIMIT$", synclimit );
	return Out;
}

string CLanguage :: SettingSyncLimitToMinimum( string min )
{
	string Out = m_CFG->GetString( "lang_0099", "lang_0099" );
	Replace( Out, "$MIN$", min );
	return Out;
}

string CLanguage :: SettingSyncLimitToMaximum( string max )
{
	string Out = m_CFG->GetString( "lang_0100", "lang_0100" );
	Replace( Out, "$MAX$", max );
	return Out;
}

string CLanguage :: SettingSyncLimitTo( string synclimit )
{
	string Out = m_CFG->GetString( "lang_0101", "lang_0101" );
	Replace( Out, "$SYNCLIMIT$", synclimit );
	return Out;
}

string CLanguage :: UnableToCreateGameNotLoggedIn( string gamename )
{
	string Out = m_CFG->GetString( "lang_0102", "lang_0102" );
	Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

string CLanguage :: AdminLoggedIn( )
{
	return m_CFG->GetString( "lang_0103", "lang_0103" );
}

string CLanguage :: AdminInvalidPassword( string attempt )
{
	string Out = m_CFG->GetString( "lang_0104", "lang_0104" );
	Replace( Out, "$ATTEMPT$", attempt );
	return Out;
}

string CLanguage :: ConnectingToBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0105", "lang_0105" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ConnectedToBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0106", "lang_0106" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: DisconnectedFromBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0107", "lang_0107" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: LoggedInToBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0108", "lang_0108" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: BNETGameHostingSucceeded( string server )
{
	string Out = m_CFG->GetString( "lang_0109", "lang_0109" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: BNETGameHostingFailed( string server, string gamename )
{
	string Out = m_CFG->GetString( "lang_0110", "lang_0110" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

string CLanguage :: ConnectingToBNETTimedOut( string server )
{
	string Out = m_CFG->GetString( "lang_0111", "lang_0111" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: PlayerDownloadedTheMap( string user, string seconds, string rate )
{
	string Out = m_CFG->GetString( "lang_0112", "lang_0112" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$SECONDS$", seconds );
	Replace( Out, "$RATE$", rate );
	return Out;
}

string CLanguage :: UnableToCreateGameNameTooLong( string gamename )
{
	string Out = m_CFG->GetString( "lang_0113", "lang_0113" );
	Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

string CLanguage :: SettingGameOwnerTo( string owner )
{
	string Out = m_CFG->GetString( "lang_0114", "lang_0114" );
	Replace( Out, "$OWNER$", owner );
	return Out;
}

string CLanguage :: TheGameIsLocked( )
{
	return m_CFG->GetString( "lang_0115", "lang_0115" );
}

string CLanguage :: GameLocked( )
{
	return m_CFG->GetString( "lang_0116", "lang_0116" );
}

string CLanguage :: GameUnlocked( )
{
	return m_CFG->GetString( "lang_0117", "lang_0117" );
}

string CLanguage :: UnableToStartDownloadNoMatchesFound( string victim )
{
	string Out = m_CFG->GetString( "lang_0118", "lang_0118" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: UnableToStartDownloadFoundMoreThanOneMatch( string victim )
{
	string Out = m_CFG->GetString( "lang_0119", "lang_0119" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: UnableToSetGameOwner( string owner )
{
	string Out = m_CFG->GetString( "lang_0120", "lang_0120" );
	Replace( Out, "$OWNER$", owner );
	return Out;
}

string CLanguage :: UnableToCheckPlayerNoMatchesFound( string victim )
{
	string Out = m_CFG->GetString( "lang_0121", "lang_0121" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: CheckedPlayer( string victim, string ping, string from, string admin, string owner, string spoofed, string spoofedrealm, string reserved )
{
	string Out = m_CFG->GetString( "lang_0122", "lang_0122" );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$PING$", ping );
	Replace( Out, "$FROM$", from );
	Replace( Out, "$ADMIN$", admin );
	Replace( Out, "$OWNER$", owner );
	Replace( Out, "$SPOOFED$", spoofed );
	Replace( Out, "$SPOOFEDREALM$", spoofedrealm );
	Replace( Out, "$RESERVED$", reserved );
	return Out;
}

string CLanguage :: UnableToCheckPlayerFoundMoreThanOneMatch( string victim )
{
	string Out = m_CFG->GetString( "lang_0123", "lang_0123" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: TheGameIsLockedBNET( )
{
	return m_CFG->GetString( "lang_0124", "lang_0124" );
}

string CLanguage :: UnableToCreateGameDisabled( string gamename )
{
	string Out = m_CFG->GetString( "lang_0125", "lang_0125" );
	Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

string CLanguage :: BotDisabled( )
{
	return m_CFG->GetString( "lang_0126", "lang_0126" );
}

string CLanguage :: BotEnabled( )
{
	return m_CFG->GetString( "lang_0127", "lang_0127" );
}

string CLanguage :: UnableToCreateGameInvalidMap( string gamename )
{
	string Out = m_CFG->GetString( "lang_0128", "lang_0128" );
	Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

string CLanguage :: WaitingForPlayersBeforeAutoStart( string players, string playersleft )
{
	string Out = m_CFG->GetString( "lang_0129", "lang_0129" );
	Replace( Out, "$PLAYERS$", players );
	Replace( Out, "$PLAYERSLEFT$", playersleft );
	return Out;
}

string CLanguage :: AutoStartDisabled( )
{
	return m_CFG->GetString( "lang_0130", "lang_0130" );
}

string CLanguage :: AutoStartEnabled( string players )
{
	string Out = m_CFG->GetString( "lang_0131", "lang_0131" );
	Replace( Out, "$PLAYERS$", players );
	return Out;
}

string CLanguage :: AnnounceMessageEnabled( )
{
	return m_CFG->GetString( "lang_0132", "lang_0132" );
}

string CLanguage :: AnnounceMessageDisabled( )
{
	return m_CFG->GetString( "lang_0133", "lang_0133" );
}

string CLanguage :: AutoHostEnabled( )
{
	return m_CFG->GetString( "lang_0134", "lang_0134" );
}

string CLanguage :: AutoHostDisabled( )
{
	return m_CFG->GetString( "lang_0135", "lang_0135" );
}

string CLanguage :: UnableToLoadSaveGamesOutside( )
{
	return m_CFG->GetString( "lang_0136", "lang_0136" );
}

string CLanguage :: UnableToLoadSaveGameGameInLobby( )
{
	return m_CFG->GetString( "lang_0137", "lang_0137" );
}

string CLanguage :: LoadingSaveGame( string file )
{
	string Out = m_CFG->GetString( "lang_0138", "lang_0138" );
	Replace( Out, "$FILE$", file );
	return Out;
}

string CLanguage :: UnableToLoadSaveGameDoesntExist( string file )
{
	string Out = m_CFG->GetString( "lang_0139", "lang_0139" );
	Replace( Out, "$FILE$", file );
	return Out;
}

string CLanguage :: UnableToCreateGameInvalidSaveGame( string gamename )
{
	string Out = m_CFG->GetString( "lang_0140", "lang_0140" );
	Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

string CLanguage :: UnableToCreateGameSaveGameMapMismatch( string gamename )
{
	string Out = m_CFG->GetString( "lang_0141", "lang_0141" );
	Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

string CLanguage :: AutoSaveEnabled( )
{
	return m_CFG->GetString( "lang_0142", "lang_0142" );
}

string CLanguage :: AutoSaveDisabled( )
{
	return m_CFG->GetString( "lang_0143", "lang_0143" );
}

string CLanguage :: DesyncDetected( )
{
	return m_CFG->GetString( "lang_0144", "lang_0144" );
}

string CLanguage :: UnableToMuteNoMatchesFound( string victim )
{
	string Out = m_CFG->GetString( "lang_0145", "lang_0145" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: MutedPlayer( string victim, string user )
{
	string Out = m_CFG->GetString( "lang_0146", "lang_0146" );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UnmutedPlayer( string victim, string user )
{
	string Out = m_CFG->GetString( "lang_0147", "lang_0147" );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UnableToMuteFoundMoreThanOneMatch( string victim )
{
	string Out = m_CFG->GetString( "lang_0148", "lang_0148" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: PlayerIsSavingTheGame( string player )
{
	string Out = m_CFG->GetString( "lang_0149", "lang_0149" );
	Replace( Out, "$PLAYER$", player );
	return Out;
}

string CLanguage :: UpdatingClanList( )
{
	return m_CFG->GetString( "lang_0150", "lang_0150" );
}

string CLanguage :: UpdatingFriendsList( )
{
	return m_CFG->GetString( "lang_0151", "lang_0151" );
}

string CLanguage :: MultipleIPAddressUsageDetected( string player, string others )
{
	string Out = m_CFG->GetString( "lang_0152", "lang_0152" );
	Replace( Out, "$PLAYER$", player );
	Replace( Out, "$OTHERS$", others );
	return Out;
}

string CLanguage :: UnableToVoteKickAlreadyInProgress( )
{
	return m_CFG->GetString( "lang_0153", "lang_0153" );
}

string CLanguage :: UnableToVoteKickNotEnoughPlayers( )
{
	return m_CFG->GetString( "lang_0154", "lang_0154" );
}

string CLanguage :: UnableToVoteKickNoMatchesFound( string victim )
{
	string Out = m_CFG->GetString( "lang_0155", "lang_0155" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: UnableToVoteKickPlayerIsReserved( string victim )
{
	string Out = m_CFG->GetString( "lang_0156", "lang_0156" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: StartedVoteKick( string victim, string user, string votesneeded )
{
	string Out = m_CFG->GetString( "lang_0157", "lang_0157" );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$USER$", user );
	Replace( Out, "$VOTESNEEDED$", votesneeded );
	return Out;
}

string CLanguage :: UnableToVoteKickFoundMoreThanOneMatch( string victim )
{
	string Out = m_CFG->GetString( "lang_0158", "lang_0158" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: VoteKickPassed( string victim )
{
	string Out = m_CFG->GetString( "lang_0159", "lang_0159" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: ErrorVoteKickingPlayer( string victim )
{
	string Out = m_CFG->GetString( "lang_0160", "lang_0160" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: VoteKickAcceptedNeedMoreVotes( string victim, string user, string votes )
{
	string Out = m_CFG->GetString( "lang_0161", "lang_0161" );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$USER$", user );
	Replace( Out, "$VOTES$", votes );
	return Out;
}

string CLanguage :: VoteKickCancelled( string victim )
{
	string Out = m_CFG->GetString( "lang_0162", "lang_0162" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: VoteKickExpired( string victim )
{
	string Out = m_CFG->GetString( "lang_0163", "lang_0163" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: WasKickedByVote( )
{
	return m_CFG->GetString( "lang_0164", "lang_0164" );
}

string CLanguage :: TypeYesToVote( string commandtrigger )
{
	string Out = m_CFG->GetString( "lang_0165", "lang_0165" );
	Replace( Out, "$COMMANDTRIGGER$", commandtrigger );
	return Out;
}

string CLanguage :: PlayersNotYetPingedAutoStart( string notpinged )
{
	string Out = m_CFG->GetString( "lang_0166", "lang_0166" );
	Replace( Out, "$NOTPINGED$", notpinged );
	return Out;
}

string CLanguage :: CreatingCascadedGame( string gamename, string user )
{
	string Out = m_CFG->GetString( "lang_0167", "lang_0167" );
	Replace( Out, "$GAMENAME$", gamename );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: PlayerJoinedTheGame( string user )
{
	string Out = m_CFG->GetString( "lang_0168", "lang_0168" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: PlayerLeftTheGame( string user )
{
	string Out = m_CFG->GetString( "lang_0169", "lang_0169" );
	Replace( Out, "$USER$", user );
	return Out;
}

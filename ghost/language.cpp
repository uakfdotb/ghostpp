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

//
// CLanguage
//

CLanguage :: CLanguage( std::string nCFGFile )
{
	m_CFG = new CConfig( );
	m_CFG->Read( nCFGFile );
}

CLanguage :: ~CLanguage( )
{
	delete m_CFG;
}

std::string CLanguage :: UnableToCreateGameTryAnotherName( std::string server, std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0001", "lang_0001" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: UserIsAlreadyAnAdmin( std::string server, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0002", "lang_0002" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: AddedUserToAdminDatabase( std::string server, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0003", "lang_0003" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: ErrorAddingUserToAdminDatabase( std::string server, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0004", "lang_0004" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: YouDontHaveAccessToThatCommand( )
{
	return m_CFG->GetString( "lang_0005", "lang_0005" );
}

std::string CLanguage :: UserIsAlreadyBanned( std::string server, std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0006", "lang_0006" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: BannedUser( std::string server, std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0007", "lang_0007" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: ErrorBanningUser( std::string server, std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0008", "lang_0008" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: UserIsAnAdmin( std::string server, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0009", "lang_0009" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: UserIsNotAnAdmin( std::string server, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0010", "lang_0010" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: UserWasBannedOnByBecause( std::string server, std::string victim, std::string date, std::string admin, std::string reason )
{
	std::string Out = m_CFG->GetString( "lang_0011", "lang_0011" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$DATE$", date );
	UTIL_Replace( Out, "$ADMIN$", admin );
	UTIL_Replace( Out, "$REASON$", reason );
	return Out;
}

std::string CLanguage :: UserIsNotBanned( std::string server, std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0012", "lang_0012" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: ThereAreNoAdmins( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0013", "lang_0013" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: ThereIsAdmin( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0014", "lang_0014" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: ThereAreAdmins( std::string server, std::string count )
{
	std::string Out = m_CFG->GetString( "lang_0015", "lang_0015" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$COUNT$", count );
	return Out;
}

std::string CLanguage :: ThereAreNoBannedUsers( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0016", "lang_0016" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: ThereIsBannedUser( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0017", "lang_0017" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: ThereAreBannedUsers( std::string server, std::string count )
{
	std::string Out = m_CFG->GetString( "lang_0018", "lang_0018" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$COUNT$", count );
	return Out;
}

std::string CLanguage :: YouCantDeleteTheRootAdmin( )
{
	return m_CFG->GetString( "lang_0019", "lang_0019" );
}

std::string CLanguage :: DeletedUserFromAdminDatabase( std::string server, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0020", "lang_0020" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: ErrorDeletingUserFromAdminDatabase( std::string server, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0021", "lang_0021" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: UnbannedUser( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0022", "lang_0022" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: ErrorUnbanningUser( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0023", "lang_0023" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: GameNumberIs( std::string number, std::string description )
{
	std::string Out = m_CFG->GetString( "lang_0024", "lang_0024" );
	UTIL_Replace( Out, "$NUMBER$", number );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

std::string CLanguage :: GameNumberDoesntExist( std::string number )
{
	std::string Out = m_CFG->GetString( "lang_0025", "lang_0025" );
	UTIL_Replace( Out, "$NUMBER$", number );
	return Out;
}

std::string CLanguage :: GameIsInTheLobby( std::string description, std::string current, std::string max )
{
	std::string Out = m_CFG->GetString( "lang_0026", "lang_0026" );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	UTIL_Replace( Out, "$CURRENT$", current );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

std::string CLanguage :: ThereIsNoGameInTheLobby( std::string current, std::string max )
{
	std::string Out = m_CFG->GetString( "lang_0027", "lang_0027" );
	UTIL_Replace( Out, "$CURRENT$", current );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

std::string CLanguage :: UnableToLoadConfigFilesOutside( )
{
	return m_CFG->GetString( "lang_0028", "lang_0028" );
}

std::string CLanguage :: LoadingConfigFile( std::string file )
{
	std::string Out = m_CFG->GetString( "lang_0029", "lang_0029" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

std::string CLanguage :: UnableToLoadConfigFileDoesntExist( std::string file )
{
	std::string Out = m_CFG->GetString( "lang_0030", "lang_0030" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

std::string CLanguage :: CreatingPrivateGame( std::string gamename, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0031", "lang_0031" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: CreatingPublicGame( std::string gamename, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0032", "lang_0032" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: UnableToUnhostGameCountdownStarted( std::string description )
{
	std::string Out = m_CFG->GetString( "lang_0033", "lang_0033" );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

std::string CLanguage :: UnhostingGame( std::string description )
{
	std::string Out = m_CFG->GetString( "lang_0034", "lang_0034" );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

std::string CLanguage :: UnableToUnhostGameNoGameInLobby( )
{
	return m_CFG->GetString( "lang_0035", "lang_0035" );
}

std::string CLanguage :: VersionAdmin( std::string version )
{
	std::string Out = m_CFG->GetString( "lang_0036", "lang_0036" );
	UTIL_Replace( Out, "$VERSION$", version );
	return Out;
}

std::string CLanguage :: VersionNotAdmin( std::string version )
{
	std::string Out = m_CFG->GetString( "lang_0037", "lang_0037" );
	UTIL_Replace( Out, "$VERSION$", version );
	return Out;
}

std::string CLanguage :: UnableToCreateGameAnotherGameInLobby( std::string gamename, std::string description )
{
	std::string Out = m_CFG->GetString( "lang_0038", "lang_0038" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

std::string CLanguage :: UnableToCreateGameMaxGamesReached( std::string gamename, std::string max )
{
	std::string Out = m_CFG->GetString( "lang_0039", "lang_0039" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

std::string CLanguage :: GameIsOver( std::string description )
{
	std::string Out = m_CFG->GetString( "lang_0040", "lang_0040" );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

std::string CLanguage :: SpoofCheckByReplying( )
{
	return m_CFG->GetString( "lang_0041", "lang_0041" );
}

std::string CLanguage :: GameRefreshed( )
{
	return m_CFG->GetString( "lang_0042", "lang_0042" );
}

std::string CLanguage :: SpoofPossibleIsAway( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0043", "lang_0043" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: SpoofPossibleIsUnavailable( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0044", "lang_0044" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: SpoofPossibleIsRefusingMessages( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0045", "lang_0045" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: SpoofDetectedIsNotInGame( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0046", "lang_0046" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: SpoofDetectedIsInPrivateChannel( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0047", "lang_0047" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: SpoofDetectedIsInAnotherGame( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0048", "lang_0048" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: CountDownAborted( )
{
	return m_CFG->GetString( "lang_0049", "lang_0049" );
}

std::string CLanguage :: TryingToJoinTheGameButBanned( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0050", "lang_0050" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: UnableToBanNoMatchesFound( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0051", "lang_0051" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: PlayerWasBannedByPlayer( std::string server, std::string victim, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0052", "lang_0052" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: UnableToBanFoundMoreThanOneMatch( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0053", "lang_0053" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: AddedPlayerToTheHoldList( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0054", "lang_0054" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: UnableToKickNoMatchesFound( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0055", "lang_0055" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: UnableToKickFoundMoreThanOneMatch( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0056", "lang_0056" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: SettingLatencyToMinimum( std::string min )
{
	std::string Out = m_CFG->GetString( "lang_0057", "lang_0057" );
	UTIL_Replace( Out, "$MIN$", min );
	return Out;
}

std::string CLanguage :: SettingLatencyToMaximum( std::string max )
{
	std::string Out = m_CFG->GetString( "lang_0058", "lang_0058" );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

std::string CLanguage :: SettingLatencyTo( std::string latency )
{
	std::string Out = m_CFG->GetString( "lang_0059", "lang_0059" );
	UTIL_Replace( Out, "$LATENCY$", latency );
	return Out;
}

std::string CLanguage :: KickingPlayersWithPingsGreaterThan( std::string total, std::string ping )
{
	std::string Out = m_CFG->GetString( "lang_0060", "lang_0060" );
	UTIL_Replace( Out, "$TOTAL$", total );
	UTIL_Replace( Out, "$PING$", ping );
	return Out;
}

std::string CLanguage :: HasPlayedGamesWithThisBot( std::string user, std::string firstgame, std::string lastgame, std::string totalgames, std::string avgloadingtime, std::string avgstay )
{
	std::string Out = m_CFG->GetString( "lang_0061", "lang_0061" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$FIRSTGAME$", firstgame );
	UTIL_Replace( Out, "$LASTGAME$", lastgame );
	UTIL_Replace( Out, "$TOTALGAMES$", totalgames );
	UTIL_Replace( Out, "$AVGLOADINGTIME$", avgloadingtime );
	UTIL_Replace( Out, "$AVGSTAY$", avgstay );
	return Out;
}

std::string CLanguage :: HasntPlayedGamesWithThisBot( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0062", "lang_0062" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: AutokickingPlayerForExcessivePing( std::string victim, std::string ping )
{
	std::string Out = m_CFG->GetString( "lang_0063", "lang_0063" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$PING$", ping );
	return Out;
}

std::string CLanguage :: SpoofCheckAcceptedFor( std::string server, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0064", "lang_0064" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: PlayersNotYetSpoofChecked( std::string notspoofchecked )
{
	std::string Out = m_CFG->GetString( "lang_0065", "lang_0065" );
	UTIL_Replace( Out, "$NOTSPOOFCHECKED$", notspoofchecked );
	return Out;
}

std::string CLanguage :: ManuallySpoofCheckByWhispering( std::string hostname )
{
	std::string Out = m_CFG->GetString( "lang_0066", "lang_0066" );
	UTIL_Replace( Out, "$HOSTNAME$", hostname );
	return Out;
}

std::string CLanguage :: SpoofCheckByWhispering( std::string hostname )
{
	std::string Out = m_CFG->GetString( "lang_0067", "lang_0067" );
	UTIL_Replace( Out, "$HOSTNAME$", hostname );
	return Out;
}

std::string CLanguage :: EveryoneHasBeenSpoofChecked( )
{
	return m_CFG->GetString( "lang_0068", "lang_0068" );
}

std::string CLanguage :: PlayersNotYetPinged( std::string notpinged )
{
	std::string Out = m_CFG->GetString( "lang_0069", "lang_0069" );
	UTIL_Replace( Out, "$NOTPINGED$", notpinged );
	return Out;
}

std::string CLanguage :: EveryoneHasBeenPinged( )
{
	return m_CFG->GetString( "lang_0070", "lang_0070" );
}

std::string CLanguage :: ShortestLoadByPlayer( std::string user, std::string loadingtime )
{
	std::string Out = m_CFG->GetString( "lang_0071", "lang_0071" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$LOADINGTIME$", loadingtime );
	return Out;
}

std::string CLanguage :: LongestLoadByPlayer( std::string user, std::string loadingtime )
{
	std::string Out = m_CFG->GetString( "lang_0072", "lang_0072" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$LOADINGTIME$", loadingtime );
	return Out;
}

std::string CLanguage :: YourLoadingTimeWas( std::string loadingtime )
{
	std::string Out = m_CFG->GetString( "lang_0073", "lang_0073" );
	UTIL_Replace( Out, "$LOADINGTIME$", loadingtime );
	return Out;
}

std::string CLanguage :: HasPlayedDotAGamesWithThisBot( std::string user, std::string totalgames, std::string totalwins, std::string totallosses, std::string totalkills, std::string totaldeaths, std::string totalcreepkills, std::string totalcreepdenies, std::string totalassists, std::string totalneutralkills, std::string totaltowerkills, std::string totalraxkills, std::string totalcourierkills, std::string avgkills, std::string avgdeaths, std::string avgcreepkills, std::string avgcreepdenies, std::string avgassists, std::string avgneutralkills, std::string avgtowerkills, std::string avgraxkills, std::string avgcourierkills )
{
	std::string Out = m_CFG->GetString( "lang_0074", "lang_0074" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$TOTALGAMES$", totalgames );
	UTIL_Replace( Out, "$TOTALWINS$", totalwins );
	UTIL_Replace( Out, "$TOTALLOSSES$", totallosses );
	UTIL_Replace( Out, "$TOTALKILLS$", totalkills );
	UTIL_Replace( Out, "$TOTALDEATHS$", totaldeaths );
	UTIL_Replace( Out, "$TOTALCREEPKILLS$", totalcreepkills );
	UTIL_Replace( Out, "$TOTALCREEPDENIES$", totalcreepdenies );
	UTIL_Replace( Out, "$TOTALASSISTS$", totalassists );
	UTIL_Replace( Out, "$TOTALNEUTRALKILLS$", totalneutralkills );
	UTIL_Replace( Out, "$TOTALTOWERKILLS$", totaltowerkills );
	UTIL_Replace( Out, "$TOTALRAXKILLS$", totalraxkills );
	UTIL_Replace( Out, "$TOTALCOURIERKILLS$", totalcourierkills );
	UTIL_Replace( Out, "$AVGKILLS$", avgkills );
	UTIL_Replace( Out, "$AVGDEATHS$", avgdeaths );
	UTIL_Replace( Out, "$AVGCREEPKILLS$", avgcreepkills );
	UTIL_Replace( Out, "$AVGCREEPDENIES$", avgcreepdenies );
	UTIL_Replace( Out, "$AVGASSISTS$", avgassists );
	UTIL_Replace( Out, "$AVGNEUTRALKILLS$", avgneutralkills );
	UTIL_Replace( Out, "$AVGTOWERKILLS$", avgtowerkills );
	UTIL_Replace( Out, "$AVGRAXKILLS$", avgraxkills );
	UTIL_Replace( Out, "$AVGCOURIERKILLS$", avgcourierkills );
	return Out;
}

std::string CLanguage :: HasntPlayedDotAGamesWithThisBot( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0075", "lang_0075" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: WasKickedForReservedPlayer( std::string reserved )
{
	std::string Out = m_CFG->GetString( "lang_0076", "lang_0076" );
	UTIL_Replace( Out, "$RESERVED$", reserved );
	return Out;
}

std::string CLanguage :: WasKickedForOwnerPlayer( std::string owner )
{
	std::string Out = m_CFG->GetString( "lang_0077", "lang_0077" );
	UTIL_Replace( Out, "$OWNER$", owner );
	return Out;
}

std::string CLanguage :: WasKickedByPlayer( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0078", "lang_0078" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: HasLostConnectionPlayerError( std::string error )
{
	std::string Out = m_CFG->GetString( "lang_0079", "lang_0079" );
	UTIL_Replace( Out, "$ERROR$", error );
	return Out;
}

std::string CLanguage :: HasLostConnectionSocketError( std::string error )
{
	std::string Out = m_CFG->GetString( "lang_0080", "lang_0080" );
	UTIL_Replace( Out, "$ERROR$", error );
	return Out;
}

std::string CLanguage :: HasLostConnectionClosedByRemoteHost( )
{
	return m_CFG->GetString( "lang_0081", "lang_0081" );
}

std::string CLanguage :: HasLeftVoluntarily( )
{
	return m_CFG->GetString( "lang_0082", "lang_0082" );
}

std::string CLanguage :: EndingGame( std::string description )
{
	std::string Out = m_CFG->GetString( "lang_0083", "lang_0083" );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

std::string CLanguage :: HasLostConnectionTimedOut( )
{
	return m_CFG->GetString( "lang_0084", "lang_0084" );
}

std::string CLanguage :: GlobalChatMuted( )
{
	return m_CFG->GetString( "lang_0085", "lang_0085" );
}

std::string CLanguage :: GlobalChatUnmuted( )
{
	return m_CFG->GetString( "lang_0086", "lang_0086" );
}

std::string CLanguage :: ShufflingPlayers( )
{
	return m_CFG->GetString( "lang_0087", "lang_0087" );
}

std::string CLanguage :: UnableToLoadConfigFileGameInLobby( )
{
	return m_CFG->GetString( "lang_0088", "lang_0088" );
}

std::string CLanguage :: PlayersStillDownloading( std::string stilldownloading )
{
	std::string Out = m_CFG->GetString( "lang_0089", "lang_0089" );
	UTIL_Replace( Out, "$STILLDOWNLOADING$", stilldownloading );
	return Out;
}

std::string CLanguage :: RefreshMessagesEnabled( )
{
	return m_CFG->GetString( "lang_0090", "lang_0090" );
}

std::string CLanguage :: RefreshMessagesDisabled( )
{
	return m_CFG->GetString( "lang_0091", "lang_0091" );
}

std::string CLanguage :: AtLeastOneGameActiveUseForceToShutdown( )
{
	return m_CFG->GetString( "lang_0092", "lang_0092" );
}

std::string CLanguage :: CurrentlyLoadedMapCFGIs( std::string mapcfg )
{
	std::string Out = m_CFG->GetString( "lang_0093", "lang_0093" );
	UTIL_Replace( Out, "$MAPCFG$", mapcfg );
	return Out;
}

std::string CLanguage :: LaggedOutDroppedByAdmin( )
{
	return m_CFG->GetString( "lang_0094", "lang_0094" );
}

std::string CLanguage :: LaggedOutDroppedByVote( )
{
	return m_CFG->GetString( "lang_0095", "lang_0095" );
}

std::string CLanguage :: PlayerVotedToDropLaggers( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0096", "lang_0096" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: LatencyIs( std::string latency )
{
	std::string Out = m_CFG->GetString( "lang_0097", "lang_0097" );
	UTIL_Replace( Out, "$LATENCY$", latency );
	return Out;
}

std::string CLanguage :: SyncLimitIs( std::string synclimit )
{
	std::string Out = m_CFG->GetString( "lang_0098", "lang_0098" );
	UTIL_Replace( Out, "$SYNCLIMIT$", synclimit );
	return Out;
}

std::string CLanguage :: SettingSyncLimitToMinimum( std::string min )
{
	std::string Out = m_CFG->GetString( "lang_0099", "lang_0099" );
	UTIL_Replace( Out, "$MIN$", min );
	return Out;
}

std::string CLanguage :: SettingSyncLimitToMaximum( std::string max )
{
	std::string Out = m_CFG->GetString( "lang_0100", "lang_0100" );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

std::string CLanguage :: SettingSyncLimitTo( std::string synclimit )
{
	std::string Out = m_CFG->GetString( "lang_0101", "lang_0101" );
	UTIL_Replace( Out, "$SYNCLIMIT$", synclimit );
	return Out;
}

std::string CLanguage :: UnableToCreateGameNotLoggedIn( std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0102", "lang_0102" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: AdminLoggedIn( )
{
	return m_CFG->GetString( "lang_0103", "lang_0103" );
}

std::string CLanguage :: AdminInvalidPassword( std::string attempt )
{
	std::string Out = m_CFG->GetString( "lang_0104", "lang_0104" );
	UTIL_Replace( Out, "$ATTEMPT$", attempt );
	return Out;
}

std::string CLanguage :: ConnectingToBNET( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0105", "lang_0105" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: ConnectedToBNET( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0106", "lang_0106" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: DisconnectedFromBNET( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0107", "lang_0107" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: LoggedInToBNET( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0108", "lang_0108" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: BNETGameHostingSucceeded( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0109", "lang_0109" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: BNETGameHostingFailed( std::string server, std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0110", "lang_0110" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: ConnectingToBNETTimedOut( std::string server )
{
	std::string Out = m_CFG->GetString( "lang_0111", "lang_0111" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

std::string CLanguage :: PlayerDownloadedTheMap( std::string user, std::string seconds, std::string rate )
{
	std::string Out = m_CFG->GetString( "lang_0112", "lang_0112" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$SECONDS$", seconds );
	UTIL_Replace( Out, "$RATE$", rate );
	return Out;
}

std::string CLanguage :: UnableToCreateGameNameTooLong( std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0113", "lang_0113" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: SettingGameOwnerTo( std::string owner )
{
	std::string Out = m_CFG->GetString( "lang_0114", "lang_0114" );
	UTIL_Replace( Out, "$OWNER$", owner );
	return Out;
}

std::string CLanguage :: TheGameIsLocked( )
{
	return m_CFG->GetString( "lang_0115", "lang_0115" );
}

std::string CLanguage :: GameLocked( )
{
	return m_CFG->GetString( "lang_0116", "lang_0116" );
}

std::string CLanguage :: GameUnlocked( )
{
	return m_CFG->GetString( "lang_0117", "lang_0117" );
}

std::string CLanguage :: UnableToStartDownloadNoMatchesFound( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0118", "lang_0118" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: UnableToStartDownloadFoundMoreThanOneMatch( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0119", "lang_0119" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: UnableToSetGameOwner( std::string owner )
{
	std::string Out = m_CFG->GetString( "lang_0120", "lang_0120" );
	UTIL_Replace( Out, "$OWNER$", owner );
	return Out;
}

std::string CLanguage :: UnableToCheckPlayerNoMatchesFound( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0121", "lang_0121" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: CheckedPlayer( std::string victim, std::string ping, std::string from, std::string admin, std::string owner, std::string spoofed, std::string spoofedrealm, std::string reserved )
{
	std::string Out = m_CFG->GetString( "lang_0122", "lang_0122" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$PING$", ping );
	UTIL_Replace( Out, "$FROM$", from );
	UTIL_Replace( Out, "$ADMIN$", admin );
	UTIL_Replace( Out, "$OWNER$", owner );
	UTIL_Replace( Out, "$SPOOFED$", spoofed );
	UTIL_Replace( Out, "$SPOOFEDREALM$", spoofedrealm );
	UTIL_Replace( Out, "$RESERVED$", reserved );
	return Out;
}

std::string CLanguage :: UnableToCheckPlayerFoundMoreThanOneMatch( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0123", "lang_0123" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: TheGameIsLockedBNET( )
{
	return m_CFG->GetString( "lang_0124", "lang_0124" );
}

std::string CLanguage :: UnableToCreateGameDisabled( std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0125", "lang_0125" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: BotDisabled( )
{
	return m_CFG->GetString( "lang_0126", "lang_0126" );
}

std::string CLanguage :: BotEnabled( )
{
	return m_CFG->GetString( "lang_0127", "lang_0127" );
}

std::string CLanguage :: UnableToCreateGameInvalidMap( std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0128", "lang_0128" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: WaitingForPlayersBeforeAutoStart( std::string players, std::string playersleft )
{
	std::string Out = m_CFG->GetString( "lang_0129", "lang_0129" );
	UTIL_Replace( Out, "$PLAYERS$", players );
	UTIL_Replace( Out, "$PLAYERSLEFT$", playersleft );
	return Out;
}

std::string CLanguage :: AutoStartDisabled( )
{
	return m_CFG->GetString( "lang_0130", "lang_0130" );
}

std::string CLanguage :: AutoStartEnabled( std::string players )
{
	std::string Out = m_CFG->GetString( "lang_0131", "lang_0131" );
	UTIL_Replace( Out, "$PLAYERS$", players );
	return Out;
}

std::string CLanguage :: AnnounceMessageEnabled( )
{
	return m_CFG->GetString( "lang_0132", "lang_0132" );
}

std::string CLanguage :: AnnounceMessageDisabled( )
{
	return m_CFG->GetString( "lang_0133", "lang_0133" );
}

std::string CLanguage :: AutoHostEnabled( )
{
	return m_CFG->GetString( "lang_0134", "lang_0134" );
}

std::string CLanguage :: AutoHostDisabled( )
{
	return m_CFG->GetString( "lang_0135", "lang_0135" );
}

std::string CLanguage :: UnableToLoadSaveGamesOutside( )
{
	return m_CFG->GetString( "lang_0136", "lang_0136" );
}

std::string CLanguage :: UnableToLoadSaveGameGameInLobby( )
{
	return m_CFG->GetString( "lang_0137", "lang_0137" );
}

std::string CLanguage :: LoadingSaveGame( std::string file )
{
	std::string Out = m_CFG->GetString( "lang_0138", "lang_0138" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

std::string CLanguage :: UnableToLoadSaveGameDoesntExist( std::string file )
{
	std::string Out = m_CFG->GetString( "lang_0139", "lang_0139" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

std::string CLanguage :: UnableToCreateGameInvalidSaveGame( std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0140", "lang_0140" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: UnableToCreateGameSaveGameMapMismatch( std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0141", "lang_0141" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: AutoSaveEnabled( )
{
	return m_CFG->GetString( "lang_0142", "lang_0142" );
}

std::string CLanguage :: AutoSaveDisabled( )
{
	return m_CFG->GetString( "lang_0143", "lang_0143" );
}

std::string CLanguage :: DesyncDetected( )
{
	return m_CFG->GetString( "lang_0144", "lang_0144" );
}

std::string CLanguage :: UnableToMuteNoMatchesFound( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0145", "lang_0145" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: MutedPlayer( std::string victim, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0146", "lang_0146" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: UnmutedPlayer( std::string victim, std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0147", "lang_0147" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: UnableToMuteFoundMoreThanOneMatch( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0148", "lang_0148" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: PlayerIsSavingTheGame( std::string player )
{
	std::string Out = m_CFG->GetString( "lang_0149", "lang_0149" );
	UTIL_Replace( Out, "$PLAYER$", player );
	return Out;
}

std::string CLanguage :: UpdatingClanList( )
{
	return m_CFG->GetString( "lang_0150", "lang_0150" );
}

std::string CLanguage :: UpdatingFriendsList( )
{
	return m_CFG->GetString( "lang_0151", "lang_0151" );
}

std::string CLanguage :: MultipleIPAddressUsageDetected( std::string player, std::string others )
{
	std::string Out = m_CFG->GetString( "lang_0152", "lang_0152" );
	UTIL_Replace( Out, "$PLAYER$", player );
	UTIL_Replace( Out, "$OTHERS$", others );
	return Out;
}

std::string CLanguage :: UnableToVoteKickAlreadyInProgress( )
{
	return m_CFG->GetString( "lang_0153", "lang_0153" );
}

std::string CLanguage :: UnableToVoteKickNotEnoughPlayers( )
{
	return m_CFG->GetString( "lang_0154", "lang_0154" );
}

std::string CLanguage :: UnableToVoteKickNoMatchesFound( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0155", "lang_0155" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: UnableToVoteKickPlayerIsReserved( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0156", "lang_0156" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: StartedVoteKick( std::string victim, std::string user, std::string votesneeded )
{
	std::string Out = m_CFG->GetString( "lang_0157", "lang_0157" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$VOTESNEEDED$", votesneeded );
	return Out;
}

std::string CLanguage :: UnableToVoteKickFoundMoreThanOneMatch( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0158", "lang_0158" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: VoteKickPassed( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0159", "lang_0159" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: ErrorVoteKickingPlayer( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0160", "lang_0160" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: VoteKickAcceptedNeedMoreVotes( std::string victim, std::string user, std::string votes )
{
	std::string Out = m_CFG->GetString( "lang_0161", "lang_0161" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$VOTES$", votes );
	return Out;
}

std::string CLanguage :: VoteKickCancelled( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0162", "lang_0162" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: VoteKickExpired( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0163", "lang_0163" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: WasKickedByVote( )
{
	return m_CFG->GetString( "lang_0164", "lang_0164" );
}

std::string CLanguage :: TypeYesToVote( std::string commandtrigger )
{
	std::string Out = m_CFG->GetString( "lang_0165", "lang_0165" );
	UTIL_Replace( Out, "$COMMANDTRIGGER$", commandtrigger );
	return Out;
}

std::string CLanguage :: PlayersNotYetPingedAutoStart( std::string notpinged )
{
	std::string Out = m_CFG->GetString( "lang_0166", "lang_0166" );
	UTIL_Replace( Out, "$NOTPINGED$", notpinged );
	return Out;
}

std::string CLanguage :: WasKickedForNotSpoofChecking( )
{
	return m_CFG->GetString( "lang_0167", "lang_0167" );
}

std::string CLanguage :: WasKickedForHavingFurthestScore( std::string score, std::string average )
{
	std::string Out = m_CFG->GetString( "lang_0168", "lang_0168" );
	UTIL_Replace( Out, "$SCORE$", score );
	UTIL_Replace( Out, "$AVERAGE$", average );
	return Out;
}

std::string CLanguage :: PlayerHasScore( std::string player, std::string score )
{
	std::string Out = m_CFG->GetString( "lang_0169", "lang_0169" );
	UTIL_Replace( Out, "$PLAYER$", player );
	UTIL_Replace( Out, "$SCORE$", score );
	return Out;
}

std::string CLanguage :: RatedPlayersSpread( std::string rated, std::string total, std::string spread )
{
	std::string Out = m_CFG->GetString( "lang_0170", "lang_0170" );
	UTIL_Replace( Out, "$RATED$", rated );
	UTIL_Replace( Out, "$TOTAL$", total );
	UTIL_Replace( Out, "$SPREAD$", spread );
	return Out;
}

std::string CLanguage :: ErrorListingMaps( )
{
	return m_CFG->GetString( "lang_0171", "lang_0171" );
}

std::string CLanguage :: FoundMaps( std::string maps )
{
	std::string Out = m_CFG->GetString( "lang_0172", "lang_0172" );
	UTIL_Replace( Out, "$MAPS$", maps );
	return Out;
}

std::string CLanguage :: NoMapsFound( )
{
	return m_CFG->GetString( "lang_0173", "lang_0173" );
}

std::string CLanguage :: ErrorListingMapConfigs( )
{
	return m_CFG->GetString( "lang_0174", "lang_0174" );
}

std::string CLanguage :: FoundMapConfigs( std::string mapconfigs )
{
	std::string Out = m_CFG->GetString( "lang_0175", "lang_0175" );
	UTIL_Replace( Out, "$MAPCONFIGS$", mapconfigs );
	return Out;
}

std::string CLanguage :: NoMapConfigsFound( )
{
	return m_CFG->GetString( "lang_0176", "lang_0176" );
}

std::string CLanguage :: PlayerFinishedLoading( std::string user )
{
	std::string Out = m_CFG->GetString( "lang_0177", "lang_0177" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

std::string CLanguage :: PleaseWaitPlayersStillLoading( )
{
	return m_CFG->GetString( "lang_0178", "lang_0178" );
}

std::string CLanguage :: MapDownloadsDisabled( )
{
	return m_CFG->GetString( "lang_0179", "lang_0179" );
}

std::string CLanguage :: MapDownloadsEnabled( )
{
	return m_CFG->GetString( "lang_0180", "lang_0180" );
}

std::string CLanguage :: MapDownloadsConditional( )
{
	return m_CFG->GetString( "lang_0181", "lang_0181" );
}

std::string CLanguage :: SettingHCL( std::string HCL )
{
	std::string Out = m_CFG->GetString( "lang_0182", "lang_0182" );
	UTIL_Replace( Out, "$HCL$", HCL );
	return Out;
}

std::string CLanguage :: UnableToSetHCLInvalid( )
{
	return m_CFG->GetString( "lang_0183", "lang_0183" );
}

std::string CLanguage :: UnableToSetHCLTooLong( )
{
	return m_CFG->GetString( "lang_0184", "lang_0184" );
}

std::string CLanguage :: TheHCLIs( std::string HCL )
{
	std::string Out = m_CFG->GetString( "lang_0185", "lang_0185" );
	UTIL_Replace( Out, "$HCL$", HCL );
	return Out;
}

std::string CLanguage :: TheHCLIsTooLongUseForceToStart( )
{
	return m_CFG->GetString( "lang_0186", "lang_0186" );
}

std::string CLanguage :: ClearingHCL( )
{
	return m_CFG->GetString( "lang_0187", "lang_0187" );
}

std::string CLanguage :: TryingToRehostAsPrivateGame( std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0188", "lang_0188" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: TryingToRehostAsPublicGame( std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0189", "lang_0189" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: RehostWasSuccessful( )
{
	return m_CFG->GetString( "lang_0190", "lang_0190" );
}

std::string CLanguage :: TryingToJoinTheGameButBannedByName( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0191", "lang_0191" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: TryingToJoinTheGameButBannedByIP( std::string victim, std::string ip, std::string bannedname )
{
	std::string Out = m_CFG->GetString( "lang_0192", "lang_0192" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$IP$", ip );
	UTIL_Replace( Out, "$BANNEDNAME$", bannedname );
	return Out;
}

std::string CLanguage :: HasBannedName( std::string victim )
{
	std::string Out = m_CFG->GetString( "lang_0193", "lang_0193" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

std::string CLanguage :: HasBannedIP( std::string victim, std::string ip, std::string bannedname )
{
	std::string Out = m_CFG->GetString( "lang_0194", "lang_0194" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$IP$", ip );
	UTIL_Replace( Out, "$BANNEDNAME$", bannedname );
	return Out;
}

std::string CLanguage :: PlayersInGameState( std::string number, std::string players )
{
	std::string Out = m_CFG->GetString( "lang_0195", "lang_0195" );
	UTIL_Replace( Out, "$NUMBER$", number );
	UTIL_Replace( Out, "$PLAYERS$", players );
	return Out;
}

std::string CLanguage :: ValidServers( std::string servers )
{
	std::string Out = m_CFG->GetString( "lang_0196", "lang_0196" );
	UTIL_Replace( Out, "$SERVERS$", servers );
	return Out;
}

std::string CLanguage :: TeamCombinedScore( std::string team, std::string score )
{
	std::string Out = m_CFG->GetString( "lang_0197", "lang_0197" );
	UTIL_Replace( Out, "$TEAM$", team );
	UTIL_Replace( Out, "$SCORE$", score );
	return Out;
}

std::string CLanguage :: BalancingSlotsCompleted( )
{
	return m_CFG->GetString( "lang_0198", "lang_0198" );
}

std::string CLanguage :: PlayerWasKickedForFurthestScore( std::string name, std::string score, std::string average )
{
	std::string Out = m_CFG->GetString( "lang_0199", "lang_0199" );
	UTIL_Replace( Out, "$NAME$", name );
	UTIL_Replace( Out, "$SCORE$", score );
	UTIL_Replace( Out, "$AVERAGE$", average );
	return Out;
}

std::string CLanguage :: LocalAdminMessagesEnabled( )
{
	return m_CFG->GetString( "lang_0200", "lang_0200" );
}

std::string CLanguage :: LocalAdminMessagesDisabled( )
{
	return m_CFG->GetString( "lang_0201", "lang_0201" );
}

std::string CLanguage :: WasDroppedDesync( )
{
	return m_CFG->GetString( "lang_0202", "lang_0202" );
}

std::string CLanguage :: WasKickedForHavingLowestScore( std::string score )
{
	std::string Out = m_CFG->GetString( "lang_0203", "lang_0203" );
	UTIL_Replace( Out, "$SCORE$", score );
	return Out;
}

std::string CLanguage :: PlayerWasKickedForLowestScore( std::string name, std::string score )
{
	std::string Out = m_CFG->GetString( "lang_0204", "lang_0204" );
	UTIL_Replace( Out, "$NAME$", name );
	UTIL_Replace( Out, "$SCORE$", score );
	return Out;
}

std::string CLanguage :: ReloadingConfigurationFiles( )
{
	return m_CFG->GetString( "lang_0205", "lang_0205" );
}

std::string CLanguage :: CountDownAbortedSomeoneLeftRecently( )
{
	return m_CFG->GetString( "lang_0206", "lang_0206" );
}

std::string CLanguage :: UnableToCreateGameMustEnforceFirst( std::string gamename )
{
	std::string Out = m_CFG->GetString( "lang_0207", "lang_0207" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

std::string CLanguage :: UnableToLoadReplaysOutside( )
{
	return m_CFG->GetString( "lang_0208", "lang_0208" );
}

std::string CLanguage :: LoadingReplay( std::string file )
{
	std::string Out = m_CFG->GetString( "lang_0209", "lang_0209" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

std::string CLanguage :: UnableToLoadReplayDoesntExist( std::string file )
{
	std::string Out = m_CFG->GetString( "lang_0210", "lang_0210" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

std::string CLanguage :: CommandTrigger( std::string trigger )
{
	std::string Out = m_CFG->GetString( "lang_0211", "lang_0211" );
	UTIL_Replace( Out, "$TRIGGER$", trigger );
	return Out;
}

std::string CLanguage :: CantEndGameOwnerIsStillPlaying( std::string owner )
{
	std::string Out = m_CFG->GetString( "lang_0212", "lang_0212" );
	UTIL_Replace( Out, "$OWNER$", owner );
	return Out;
}

std::string CLanguage :: CantUnhostGameOwnerIsPresent( std::string owner )
{
	std::string Out = m_CFG->GetString( "lang_0213", "lang_0213" );
	UTIL_Replace( Out, "$OWNER$", owner );
	return Out;
}

std::string CLanguage :: WasAutomaticallyDroppedAfterSeconds( std::string seconds )
{
	std::string Out = m_CFG->GetString( "lang_0214", "lang_0214" );
	UTIL_Replace( Out, "$SECONDS$", seconds );
	return Out;
}

std::string CLanguage :: HasLostConnectionTimedOutGProxy( )
{
	return m_CFG->GetString( "lang_0215", "lang_0215" );
}

std::string CLanguage :: HasLostConnectionSocketErrorGProxy( std::string error )
{
	std::string Out = m_CFG->GetString( "lang_0216", "lang_0216" );
	UTIL_Replace( Out, "$ERROR$", error );
	return Out;
}

std::string CLanguage :: HasLostConnectionClosedByRemoteHostGProxy( )
{
	return m_CFG->GetString( "lang_0217", "lang_0217" );
}

std::string CLanguage :: WaitForReconnectSecondsRemain( std::string seconds )
{
	std::string Out = m_CFG->GetString( "lang_0218", "lang_0218" );
	UTIL_Replace( Out, "$SECONDS$", seconds );
	return Out;
}

std::string CLanguage :: WasUnrecoverablyDroppedFromGProxy( )
{
	return m_CFG->GetString( "lang_0219", "lang_0219" );
}

std::string CLanguage :: PlayerReconnectedWithGProxy( std::string name )
{
	std::string Out = m_CFG->GetString( "lang_0220", "lang_0220" );
	UTIL_Replace( Out, "$NAME$", name );
	return Out;
}

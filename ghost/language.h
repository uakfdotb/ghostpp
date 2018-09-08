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

#ifndef LANGUAGE_H
#define LANGUAGE_H

//
// CLanguage
//

class CLanguage
{
private:
	CConfig *m_CFG;

public:
	CLanguage( std::string nCFGFile );
	~CLanguage( );

	std::string UnableToCreateGameTryAnotherName( std::string server, std::string gamename );
	std::string UserIsAlreadyAnAdmin( std::string server, std::string user );
	std::string AddedUserToAdminDatabase( std::string server, std::string user );
	std::string ErrorAddingUserToAdminDatabase( std::string server, std::string user );
	std::string YouDontHaveAccessToThatCommand( );
	std::string UserIsAlreadyBanned( std::string server, std::string victim );
	std::string BannedUser( std::string server, std::string victim );
	std::string ErrorBanningUser( std::string server, std::string victim );
	std::string UserIsAnAdmin( std::string server, std::string user );
	std::string UserIsNotAnAdmin( std::string server, std::string user );
	std::string UserWasBannedOnByBecause( std::string server, std::string victim, std::string date, std::string admin, std::string reason );
	std::string UserIsNotBanned( std::string server, std::string victim );
	std::string ThereAreNoAdmins( std::string server );
	std::string ThereIsAdmin( std::string server );
	std::string ThereAreAdmins( std::string server, std::string count );
	std::string ThereAreNoBannedUsers( std::string server );
	std::string ThereIsBannedUser( std::string server );
	std::string ThereAreBannedUsers( std::string server, std::string count );
	std::string YouCantDeleteTheRootAdmin( );
	std::string DeletedUserFromAdminDatabase( std::string server, std::string user );
	std::string ErrorDeletingUserFromAdminDatabase( std::string server, std::string user );
	std::string UnbannedUser( std::string victim );
	std::string ErrorUnbanningUser( std::string victim );
	std::string GameNumberIs( std::string number, std::string description );
	std::string GameNumberDoesntExist( std::string number );
	std::string GameIsInTheLobby( std::string description, std::string current, std::string max );
	std::string ThereIsNoGameInTheLobby( std::string current, std::string max );
	std::string UnableToLoadConfigFilesOutside( );
	std::string LoadingConfigFile( std::string file );
	std::string UnableToLoadConfigFileDoesntExist( std::string file );
	std::string CreatingPrivateGame( std::string gamename, std::string user );
	std::string CreatingPublicGame( std::string gamename, std::string user );
	std::string UnableToUnhostGameCountdownStarted( std::string description );
	std::string UnhostingGame( std::string description );
	std::string UnableToUnhostGameNoGameInLobby( );
	std::string VersionAdmin( std::string version );
	std::string VersionNotAdmin( std::string version );
	std::string UnableToCreateGameAnotherGameInLobby( std::string gamename, std::string description );
	std::string UnableToCreateGameMaxGamesReached( std::string gamename, std::string max );
	std::string GameIsOver( std::string description );
	std::string SpoofCheckByReplying( );
	std::string GameRefreshed( );
	std::string SpoofPossibleIsAway( std::string user );
	std::string SpoofPossibleIsUnavailable( std::string user );
	std::string SpoofPossibleIsRefusingMessages( std::string user );
	std::string SpoofDetectedIsNotInGame( std::string user );
	std::string SpoofDetectedIsInPrivateChannel( std::string user );
	std::string SpoofDetectedIsInAnotherGame( std::string user );
	std::string CountDownAborted( );
	std::string TryingToJoinTheGameButBanned( std::string victim );
	std::string UnableToBanNoMatchesFound( std::string victim );
	std::string PlayerWasBannedByPlayer( std::string server, std::string victim, std::string user );
	std::string UnableToBanFoundMoreThanOneMatch( std::string victim );
	std::string AddedPlayerToTheHoldList( std::string user );
	std::string UnableToKickNoMatchesFound( std::string victim );
	std::string UnableToKickFoundMoreThanOneMatch( std::string victim );
	std::string SettingLatencyToMinimum( std::string min );
	std::string SettingLatencyToMaximum( std::string max );
	std::string SettingLatencyTo( std::string latency );
	std::string KickingPlayersWithPingsGreaterThan( std::string total, std::string ping );
	std::string HasPlayedGamesWithThisBot( std::string user, std::string firstgame, std::string lastgame, std::string totalgames, std::string avgloadingtime, std::string avgstay );
	std::string HasntPlayedGamesWithThisBot( std::string user );
	std::string AutokickingPlayerForExcessivePing( std::string victim, std::string ping );
	std::string SpoofCheckAcceptedFor( std::string server, std::string user );
	std::string PlayersNotYetSpoofChecked( std::string notspoofchecked );
	std::string ManuallySpoofCheckByWhispering( std::string hostname );
	std::string SpoofCheckByWhispering( std::string hostname );
	std::string EveryoneHasBeenSpoofChecked( );
	std::string PlayersNotYetPinged( std::string notpinged );
	std::string EveryoneHasBeenPinged( );
	std::string ShortestLoadByPlayer( std::string user, std::string loadingtime );
	std::string LongestLoadByPlayer( std::string user, std::string loadingtime );
	std::string YourLoadingTimeWas( std::string loadingtime );
	std::string HasPlayedDotAGamesWithThisBot( std::string user, std::string totalgames, std::string totalwins, std::string totallosses, std::string totalkills, std::string totaldeaths, std::string totalcreepkills, std::string totalcreepdenies, std::string totalassists, std::string totalneutralkills, std::string totaltowerkills, std::string totalraxkills, std::string totalcourierkills, std::string avgkills, std::string avgdeaths, std::string avgcreepkills, std::string avgcreepdenies, std::string avgassists, std::string avgneutralkills, std::string avgtowerkills, std::string avgraxkills, std::string avgcourierkills );
	std::string HasntPlayedDotAGamesWithThisBot( std::string user );
	std::string WasKickedForReservedPlayer( std::string reserved );
	std::string WasKickedForOwnerPlayer( std::string owner );
	std::string WasKickedByPlayer( std::string user );
	std::string HasLostConnectionPlayerError( std::string error );
	std::string HasLostConnectionSocketError( std::string error );
	std::string HasLostConnectionClosedByRemoteHost( );
	std::string HasLeftVoluntarily( );
	std::string EndingGame( std::string description );
	std::string HasLostConnectionTimedOut( );
	std::string GlobalChatMuted( );
	std::string GlobalChatUnmuted( );
	std::string ShufflingPlayers( );
	std::string UnableToLoadConfigFileGameInLobby( );
	std::string PlayersStillDownloading( std::string stilldownloading );
	std::string RefreshMessagesEnabled( );
	std::string RefreshMessagesDisabled( );
	std::string AtLeastOneGameActiveUseForceToShutdown( );
	std::string CurrentlyLoadedMapCFGIs( std::string mapcfg );
	std::string LaggedOutDroppedByAdmin( );
	std::string LaggedOutDroppedByVote( );
	std::string PlayerVotedToDropLaggers( std::string user );
	std::string LatencyIs( std::string latency );
	std::string SyncLimitIs( std::string synclimit );
	std::string SettingSyncLimitToMinimum( std::string min );
	std::string SettingSyncLimitToMaximum( std::string max );
	std::string SettingSyncLimitTo( std::string synclimit );
	std::string UnableToCreateGameNotLoggedIn( std::string gamename );
	std::string AdminLoggedIn( );
	std::string AdminInvalidPassword( std::string attempt );
	std::string ConnectingToBNET( std::string server );
	std::string ConnectedToBNET( std::string server );
	std::string DisconnectedFromBNET( std::string server );
	std::string LoggedInToBNET( std::string server );
	std::string BNETGameHostingSucceeded( std::string server );
	std::string BNETGameHostingFailed( std::string server, std::string gamename );
	std::string ConnectingToBNETTimedOut( std::string server );
	std::string PlayerDownloadedTheMap( std::string user, std::string seconds, std::string rate );
	std::string UnableToCreateGameNameTooLong( std::string gamename );
	std::string SettingGameOwnerTo( std::string owner );
	std::string TheGameIsLocked( );
	std::string GameLocked( );
	std::string GameUnlocked( );
	std::string UnableToStartDownloadNoMatchesFound( std::string victim );
	std::string UnableToStartDownloadFoundMoreThanOneMatch( std::string victim );
	std::string UnableToSetGameOwner( std::string owner );
	std::string UnableToCheckPlayerNoMatchesFound( std::string victim );
	std::string CheckedPlayer( std::string victim, std::string ping, std::string from, std::string admin, std::string owner, std::string spoofed, std::string spoofedrealm, std::string reserved );
	std::string UnableToCheckPlayerFoundMoreThanOneMatch( std::string victim );
	std::string TheGameIsLockedBNET( );
	std::string UnableToCreateGameDisabled( std::string gamename );
	std::string BotDisabled( );
	std::string BotEnabled( );
	std::string UnableToCreateGameInvalidMap( std::string gamename );
	std::string WaitingForPlayersBeforeAutoStart( std::string players, std::string playersleft );
	std::string AutoStartDisabled( );
	std::string AutoStartEnabled( std::string players );
	std::string AnnounceMessageEnabled( );
	std::string AnnounceMessageDisabled( );
	std::string AutoHostEnabled( );
	std::string AutoHostDisabled( );
	std::string UnableToLoadSaveGamesOutside( );
	std::string UnableToLoadSaveGameGameInLobby( );
	std::string LoadingSaveGame( std::string file );
	std::string UnableToLoadSaveGameDoesntExist( std::string file );
	std::string UnableToCreateGameInvalidSaveGame( std::string gamename );
	std::string UnableToCreateGameSaveGameMapMismatch( std::string gamename );
	std::string AutoSaveEnabled( );
	std::string AutoSaveDisabled( );
	std::string DesyncDetected( );
	std::string UnableToMuteNoMatchesFound( std::string victim );
	std::string MutedPlayer( std::string victim, std::string user );
	std::string UnmutedPlayer( std::string victim, std::string user );
	std::string UnableToMuteFoundMoreThanOneMatch( std::string victim );
	std::string PlayerIsSavingTheGame( std::string player );
	std::string UpdatingClanList( );
	std::string UpdatingFriendsList( );
	std::string MultipleIPAddressUsageDetected( std::string player, std::string others );
	std::string UnableToVoteKickAlreadyInProgress( );
	std::string UnableToVoteKickNotEnoughPlayers( );
	std::string UnableToVoteKickNoMatchesFound( std::string victim );
	std::string UnableToVoteKickPlayerIsReserved( std::string victim );
	std::string StartedVoteKick( std::string victim, std::string user, std::string votesneeded );
	std::string UnableToVoteKickFoundMoreThanOneMatch( std::string victim );
	std::string VoteKickPassed( std::string victim );
	std::string ErrorVoteKickingPlayer( std::string victim );
	std::string VoteKickAcceptedNeedMoreVotes( std::string victim, std::string user, std::string votes );
	std::string VoteKickCancelled( std::string victim );
	std::string VoteKickExpired( std::string victim );
	std::string WasKickedByVote( );
	std::string TypeYesToVote( std::string commandtrigger );
	std::string PlayersNotYetPingedAutoStart( std::string notpinged );
	std::string WasKickedForNotSpoofChecking( );
	std::string WasKickedForHavingFurthestScore( std::string score, std::string average );
	std::string PlayerHasScore( std::string player, std::string score );
	std::string RatedPlayersSpread( std::string rated, std::string total, std::string spread );
	std::string ErrorListingMaps( );
	std::string FoundMaps( std::string maps );
	std::string NoMapsFound( );
	std::string ErrorListingMapConfigs( );
	std::string FoundMapConfigs( std::string mapconfigs );
	std::string NoMapConfigsFound( );
	std::string PlayerFinishedLoading( std::string user );
	std::string PleaseWaitPlayersStillLoading( );
	std::string MapDownloadsDisabled( );
	std::string MapDownloadsEnabled( );
	std::string MapDownloadsConditional( );
	std::string SettingHCL( std::string HCL );
	std::string UnableToSetHCLInvalid( );
	std::string UnableToSetHCLTooLong( );
	std::string TheHCLIs( std::string HCL );
	std::string TheHCLIsTooLongUseForceToStart( );
	std::string ClearingHCL( );
	std::string TryingToRehostAsPrivateGame( std::string gamename );
	std::string TryingToRehostAsPublicGame( std::string gamename );
	std::string RehostWasSuccessful( );
	std::string TryingToJoinTheGameButBannedByName( std::string victim );
	std::string TryingToJoinTheGameButBannedByIP( std::string victim, std::string ip, std::string bannedname );
	std::string HasBannedName( std::string victim );
	std::string HasBannedIP( std::string victim, std::string ip, std::string bannedname );
	std::string PlayersInGameState( std::string number, std::string players );
	std::string ValidServers( std::string servers );
	std::string TeamCombinedScore( std::string team, std::string score );
	std::string BalancingSlotsCompleted( );
	std::string PlayerWasKickedForFurthestScore( std::string name, std::string score, std::string average );
	std::string LocalAdminMessagesEnabled( );
	std::string LocalAdminMessagesDisabled( );
	std::string WasDroppedDesync( );
	std::string WasKickedForHavingLowestScore( std::string score );
	std::string PlayerWasKickedForLowestScore( std::string name, std::string score );
	std::string ReloadingConfigurationFiles( );
	std::string CountDownAbortedSomeoneLeftRecently( );
	std::string UnableToCreateGameMustEnforceFirst( std::string gamename );
	std::string UnableToLoadReplaysOutside( );
	std::string LoadingReplay( std::string file );
	std::string UnableToLoadReplayDoesntExist( std::string file );
	std::string CommandTrigger( std::string trigger );
	std::string CantEndGameOwnerIsStillPlaying( std::string owner );
	std::string CantUnhostGameOwnerIsPresent( std::string owner );
	std::string WasAutomaticallyDroppedAfterSeconds( std::string seconds );
	std::string HasLostConnectionTimedOutGProxy( );
	std::string HasLostConnectionSocketErrorGProxy( std::string error );
	std::string HasLostConnectionClosedByRemoteHostGProxy( );
	std::string WaitForReconnectSecondsRemain( std::string seconds );
	std::string WasUnrecoverablyDroppedFromGProxy( );
	std::string PlayerReconnectedWithGProxy( std::string name );
};

#endif

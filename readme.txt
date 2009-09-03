====================
GHost++ Version 14.7
====================

GHost++ is a port of the original GHost project to C++. It was ported by Trevor Hogan.
It contains many enhancements and fixes that were not present in the original GHost.
You can compile and run GHost++ on Windows, Linux, and OS X with this release although the code should be mostly portable to other operating systems with only very minor changes.
The official GHost++ forums are currently located at http://forum.codelain.com
The official GHost++ SVN repository is currently located at http://code.google.com/p/ghostplusplus/

========
Overview
========

GHost++ is a Warcraft III: The Frozen Throne game hosting bot.
It can host Warcraft III: The Frozen Throne games on LAN, on battle.net, on PVPGN, and on any combination of these networks at the same time.
Since GHost++ is a bot it must have its own CD keys, username, and password for each battle.net server.
Note that you can use the same set of CD keys on each battle.net server (East, West, Europe, Asia) at the same time.
This means that to play on your own GHost++ bot you will need one set of CD keys for yourself and one set for your bot.
It is possible to work around this limitation using the "Admin Game" feature of GHost++ (search this readme for more information).

=============
Configuration
=============

GHost++ is configured via the plain text configuration file ghost.cfg.
The program itself runs in console mode and does not take any console input (it outputs messages to the console for information purposes only).

***You need to edit ghost.cfg before running GHost++***
***You need to edit ghost.cfg before running GHost++***
***You need to edit ghost.cfg before running GHost++***

GHost++ takes one command line argument, the name of the main config file. It defaults to ghost.cfg if no arguments are provided.
Note: If you are using Windows and ghost.cfg looks like an unreadable mess you should try to open it with Wordpad instead of Notepad (or some other text editor).

==============
Required Files
==============

If you want to be able to connect to battle.net:

-> put "game.dll" in your bot_war3path
-> put "Storm.dll" in your bot_war3path
-> put "war3.exe" in your bot_war3path

If you want GHost++ to automatically extract blizzard.j and common.j on startup (used when automatically calculating map values):

-> put "War3Patch.mpq" in your bot_war3path

If you want GHost++ to automatically calculate map values:

-> put "blizzard.j" in your bot_mapcfgpath
-> put "common.j" in your bot_mapcfgpath

Note that blizzard.j and common.j will be automatically extracted from War3Patch.mpq if you provide GHost++ with your War3Patch.mpq file (as mentioned above).

===================
Optimizing Your Bot
===================

*** General Tips:

1.) The most common reason for lag in GHost++ games on Windows is due to the way Windows allocates CPU time to programs.
If you are experiencing extreme lag on Windows, open the task manager (Ctrl+Alt+Delete), find ghost.exe in the process list, and increase the priority by one level.
Do not set the priority to "realtime" as this does not benefit GHost++ and can cause system instability.
Note: GHost++ Version 14.3 and newer automatically sets the process priority to "above normal" on startup so this tip is no longer applicable.

2.) The second most common reason is due to the local SQLite database GHost++ uses.
The local SQLite database GHost++ uses is not intended to be used with large scale bots.
If you are experiencing lag when adding admins and bans and when games end you should either delete your ghost.dbs and start over, or clean it up manually, or use MySQL instead.
Cleaning up the database manually requires using a 3rd party tool not included in GHost++ (e.g. the SQLite Manager addon for Firefox) and is not described here.
Using MySQL requires setting up a MySQL database server and is only recommended for advanced users.

3.) If you are experiencing lag when using the !stats and !statsdota commands, these commands are not optimized for large databases whether SQLite or MySQL.
You can disable anonymous access to !stats and !statsdota by setting the bnet*_publiccommands config value to 0 for each realm you want to disable these commands on.

4.) Another reason for lag on Windows is that Windows does not handle very large log files efficiently.
If your ghost.log is too large (several MB) you should delete or rename it. You can do this while the bot is running.

*** Network Tips:

1.) If you are experiencing spikes when the bot is reconnecting to battle.net the most likely reason is due to the DNS resolver.
GHost++ resolves battle.net server addresses and BNLS addresses when connecting.
Since Version 13.3 GHost++ automatically caches all battle.net server addresses after the first connection but does NOT cache BNLS addresses.
To avoid calling the DNS resolver when reconnecting you should ensure that the BNLS addresses in ghost.cfg are in "dots and numbers" format (e.g. "1.2.3.4").
It is not necessary to do the same for the battle.net server addresses as they are cached and changing these addresses will affect your admins and bans.

2.) If you are experiencing lag when players are downloading the map, try decreasing bot_maxdownloaders and bot_maxdownloadspeed in ghost.cfg.

3.) If you want to minimize the latency in your games and you have a fast internet connection, try setting tcp_nodelay = 1 in ghost.cfg.
This may reduce game latency but will also slightly increase the bandwidth required to run each game.

===============
How Admins Work
===============

There are three types of admins:

1.) Root Admins.

Each battle.net server has a root admin defined in ghost.cfg.
Root admins have access to every command both in battle.net and in the lobby and ingame.
In particular this includes !addadmin, !checkadmin, !countadmins, !deladmin, !exit, and !quit among others.
Root admins are also exempt from command restrictions in locked games and can change the owner of a game using !owner even when the game owner is present.

2.) Game Owners.

Each game has an owner defined as the user who ran the !priv or !pub command or the user specified with the !privby or !pubby command.
Game owners have access to every command in the lobby and ingame but NO commands in battle.net.
You can think of the game owner as a temporary admin for one game only - it doesn't have to be a root admin or a regular admin.
The game owner is also the only user other than root admins who can use commands inside a game which is locked (see the !lock and !unlock commands for more information).
The game owner for a particular game can be changed after the game is created with the !owner command.

3.) Admins.

Each battle.net server has a list of admins.
Admins are added and deleted by the root admin only.
Admins have access to most commands in battle.net and all commands in the lobby and ingame.

So, how does GHost++ determine whether a user has admin access in the lobby and ingame? The user must meet several criteria:

1.) The user must be spoof checked.
 a.) If spoof checking is disabled they must still manually spoof check by whispering the bot.
 b.) GHost++ treats players joining from the LAN screen as spoof checked because they cannot spoof check since they aren't connected to battle.net.
2.) The user must be either a root admin on the realm they spoof checked on, or they must be the game owner, or they must be an admin on the realm they spoof checked on.

If the bot is ignoring you in the lobby and ingame it's most likely because you haven't spoof checked.

=============
How Bans Work
=============

Each ban is attached to a specific realm.
When you run the !addban or !ban commands on battle.net, the ban will be attached to the realm you ran the command on.
When you run the !addban or !ban commands in a game, the ban will be attached to the realm the player joined from.
When you run the !delban or !unban commands all bans for that username will be deleted regardless of realm.

Update:

Since Version 13.1 GHost++ supports banning of users by IP address as well as by name.
This feature is controlled by the bot_banmethod configuration value.
If bot_banmethod = 1, GHost++ will automatically reject players using a banned name.
If bot_banmethod = 2, GHost++ will automatically reject players using a banned IP address.
If bot_banmethod = 3, GHost++ will automatically reject players using a banned name or IP address.
If bot_banmethod is anything else GHost++ will print a message when a banned player joins but will not automatically reject them.

=======================
How Reserved Slots Work
=======================

Each game has a list of reserved players.
The list always starts out empty when starting a new game.
Note: You can configure GHost++ to automatically add the bot's battle.net friends and/or clan members to the reserved list when configuring your battle.net connections.
You can use the !hold command to add players to the list.
When a player joins the game the bot considers them to be a reserved player if any one of the following is true:

1.) If it finds the player in the list.
2.) If the player is a root admin on any defined realm.
3.) If the player is an admin on any defined realm.
4.) If the player is the game owner.

Note that you do not need to be spoof checked to be considered a reserved player because you can't spoof check before joining the game.
If the player is considered a reserved player they will be given preference when choosing a slot in the following ways:

1.) If an open slot is found they will join that slot.
2.) If a closed slot is found they will join that slot.
3.) If a slot occupied by a non-reserved player is found and that player is downloading the map and has the least amount downloaded so far that player will be kicked and the reserved player will join that slot.
4.) If a slot occupied by a non-reserved player is found that player will be kicked and the reserved player will join that slot.

Additionally, if the player is the game owner they will be guaranteed a slot in the following way:

4.) The player in the lowest numbered slot will be kicked and the game owner will join that slot.
5.) The computer in slot 0 (the first slot) will be kicked and the game owner will join that slot.

===============================================================
The Admin Game (or: how to use GHost++ with one set of CD keys)
===============================================================

Since Version 9.0 GHost++ can be used with only one set of CD keys.
This feature can also be used to host LAN games without connecting to battle.net at all.
Here's how:

1.) You need to set admingame_create to 1 in ghost.cfg to enable the admin game.
2.) You need to set admingame_port in ghost.cfg to the port you want to host the admin game on. It *must* be different from your bnet_hostport *and* from your Warcraft 3 port.
3.) You need to set admingame_password in ghost.cfg to a password which you will need to login to the admin game.

Now, when you start GHost++:

1.) GHost++ automatically hosts a Warcraft 3 game (the "Admin Game") and broadcasts it to the local network.
 a.) If you aren't on the local network you won't be able to join the Admin Game.
 b.) The Admin Game is not created on battle.net. It is a local network (LAN) game only.
2.) If configured to do so GHost++ logs into battle.net as usual.
 a.) This means you need to edit ghost.cfg if you haven't already done so and enter your CD keys, battle.net username, battle.net password, etc...
 b.) If you only want to host LAN games and don't want to connect to battle.net at all you can skip this step.
3.) Open Warcraft 3 and go to the LAN screen. After a few seconds you will see a game called "GHost++ Admin Game" appear. Join it.
4.) Use the password command to enter your Admin Game password (e.g. "!password your_password_here").
 a.) If you enter the wrong password you will be kicked from the game and temporarily banned for 5 seconds.
5.) Use the map or load command to load a map.
6.) Use the priv/privby/pub/pubby commands to create a game. If connecting to battle.net you should wait until it says "Battle.net game hosting succeeded" for each battle.net server before continuing.
7.) Leave the game and return to the LAN screen. After a few seconds you will see your newly created game appear. Join it.
8.) You are now ready to play. Your game has been created on the local network and, if configured to do so, on each battle.net server.
9.) If you make a mistake and want to unhost the game you can use !unhost in either game (the Admin Game or your newly created game).

Note: MySQL support in the Admin Game was added in GHost++ Version 14.3. Previous versions do not support MySQL database commands in the admin game.

Since Version 14.3 GHost++ can automatically relay battle.net whispers, chat messages, and emotes to you when you use the admin game.
This feature will only be activated when the admin game is enabled.
When the bot receives a battle.net whisper it will send it to all logged in users in the admin game as "[W: Realm] [User] Message".
When the bot receives a battle.net chat message it will send it to all logged in users in the admin game as "[L: Realm] [User] Message".
When the bot receives a battle.net emote it will send it to all logged in users in the admin game as "[E: Realm] [User] Message".
If you find these messages too annoying you can turn them off in a regular game by using "!messages off" and back on by using "!messages on".
Additionally, if you are the game owner and you are connecting to the bot from a local or LAN IP address the bot will send you the same messages in game lobbies and in games.
You can use the bot's battle.net account to respond to messages with the !w command or the !say command.
In the admin game, in game lobbies, and in games, you can type "!w <name> <message>" to force the bot to send a whisper to <name> on ALL connected battle.net realms.
This means that it's possible someone using the same name as your friend but on a different realm will see your message.
Unfortunately the bot must send your message to all realms because it doesn't know which realm you want the message to be sent to.
You can avoid problems with this by configuring your bot to only connect to a single battle.net realm.
Note that when you use the !w command the bot will hide your message from the other players in the game lobby and in the game.
Be careful not to mistype the command or it will be relayed to other players!
The !say command works the same as the !w command except it doesn't require a player name (it's used for sending chat messages instead of whispers).

================================
Using GHost++ on Multiple Realms
================================

Since Version 10.0 GHost++ can connect to multiple realms at the same time.
Here's how:

1.) When GHost++ starts up it reads up to 9 sets of battle.net connection information from ghost.cfg.
2.) A set of battle.net connection information contains the following keys:
 a.) *server (required)
 b.) *serveralias
 c.) *cdkeyroc (required)
 d.) *cdkeytft (required)
 e.) *username (required)
 f.) *password (required)
 g.) *firstchannel
 h.) *rootadmin
 i.) *commandtrigger
 j.) *holdfriends
 k.) *holdclan
 l.) *publiccommands
 m.) *bnlsserver
 n.) *bnlsport
 o.) *bnlswardencookie
 p.) *custom_war3version
 q.) *custom_exeversion
 r.) *custom_exeversionhash
 s.) *custom_passwordhashtype
 t.) *custom_maxmessagelength
 u.) *custom_countryabbrev
 v.) *custom_country
3.) GHost++ will search for battle.net connection information by replacing the "*" in each key above with "bnet_" then "bnet2_" then "bnet3_" and so on until "bnet9_".
 a.) Note that GHost++ doesn't search for "bnet1_" for backwards compatibility reasons.
4.) If GHost++ doesn't find a *server key it stops searching for any further battle.net connection information.
5.) If any of the required keys are missing GHost++ will skip that set of battle.net connection information and start searching for the next one.

============
Auto Hosting
============

Since Version 10.4 GHost++ can automatically host and start public games without admin intervention.
Here's how:

1.) Load the map you want to auto host with the !load or !map commands.
2.) Use the !autohost command to start the auto hosting process.
3.) The bot will keep track of which map file was loaded when the !autohost command was used and it will use that map file when auto hosting each game.

The autohost command takes three arguments: !autohost <m> <p> <n>

1.) <m> is the maximum number of games to auto host at the same time.
 a.) This can be less than bot_maxgames, especially if you want to leave some games open for your own use.
 b.) The bot will not auto host any new games while the number of running games is <m> or greater. The running games do not have to be auto hosted games.
2.) <p> is the number of players required to auto start the game.
 a.) When <p> players have joined the game it will ensure that all players have the map and have been ping checked before starting.
 b.) If spoof checks are required with bot_requirespoofchecks then it will ensure all players have spoof checked before starting.
3.) <n> is the game name.
 a.) The bot will automatically add a unique game number to the end of <n> before auto hosting each game.
 b.) For example if you specify "BattleShips Auto" it will create games like "BattleShips Auto #1" and "BattleShips Auto #2" and so on.

It will take up to 30 seconds before each game is created.
Note that if the map is changed while the bot is auto hosting it will continue creating games using the original map file.
To change the auto hosted map you must stop the auto hoster, change the map, and restart it using the !autohost command again.

Example usage:

!autohost 4 10 BattleShips!!!

This will auto host up to 4 games at a time, auto starting when 10 players have joined, with names like "BattleShips!!! #1" and "BattleShips!!! #2" and so on.
To turn off the auto hoster use "!autohost off" or simply "!autohost".

=================
Using Saved Games
=================

Since Version 11.0 GHost++ can autosave games and load saved games.
GHost++ will never create saved games itself, you must provide GHost++ with saved games which were created by Warcraft III.
To load a saved game:

1.) You need to set bot_savegamepath in ghost.cfg to the path where you put your saved games.

Now, when you start GHost++:

1.) Use the map or load command to load the correct map (the map the saved game was created on).
2.) Use the loadsg command to load the saved game.
3.) Use the hostsg command to host the saved game. If the saved game was created on a different map it will refuse to host the game.
 a.) Saved games are always created as private games and cannot be rehosted.
4.) GHost++ is not smart enough to know which player belongs in which slot. It is your responsibility to ensure each player is in the correct slot before starting the game.
 a.) The slot numbers are not adjusted to match the number of players.
 b.) This means the slot commands such as close, open, and swap use the original slot numbers.
 c.) It's possible to swap someone into an invisible slot if you aren't careful.
 d.) If you start the game with any players in the wrong slot you may experience glitches in the game.
5.) Use the start command to start the saved game.

GHost++ also supports autosaving the game just before someone disconnects.
This is an experimental feature and is not recommended for competitive play as it causes glitches in the game.
Here's how:

1.) You need to set bot_autosave to 1 in ghost.cfg to enable autosaving.

When a player unexpectedly disconnects from the game GHost++ will forge a savegame request from that player.
This causes all remaining players to save the game.
It is important to realize that GHost++ does not create a saved game itself.
The player who disconnected from the game will also not save the game because they have already disconnected.
Since saved games are unique to the player who created them it is impossible to obtain the correct saved game for the disconnected player at this point.
Therefore, one of the remaining players must send their saved game to the disconnected player.
The saved game must come from a player on the same team as the disconnected player otherwise the fog of war is reversed for that player.
This procedure causes additional glitches in the game due to the incorrect saved game even if it comes from a player on the same team.

***A QUICK NOTE FOR COMPETITIVE PLAYERS***

Although the autosave feature is experimental and glitchy the load feature is not.
As long as each player has the correct saved game and each player is in the correct slot before starting no glitches have been observed.
This means you will still need to manually save the game on a periodic basis in order to ensure that each player has the correct saved game after a disconnect.
It also means you will need to be careful to ensure that each player is in the correct slot when loading the game.

==============
Saving Replays
==============

Since Version 11.0 GHost++ can automatically create replays for hosted games.
Here's how:

1.) You need to set bot_savereplays to 1 in ghost.cfg to enable automatic creation of replays.
2.) You need to set bot_replaypath in ghost.cfg to the path where you want GHost++ to create replays.

GHost++ will create a replay for each game in the bot_replaypath directory after the game has finished.
These replays will contain chat messages from all players (including allied and private chat from players on every team).
These replays will also be complete replays regardless of the order in which players leave the game.
As a slight amendment to the above due to a technical problem the last few frames (normally less than a second) of the game will be missing.
When GHost++ is configured to save replays it will store replay data for each game in memory as the game progresses.
This means you will see a noticeable increase in GHost++'s memory usage as the replay data grows. When the game is finished the memory will be released.

================
Map Config Files
================

GHost++ uses map config files to describe each map and set some game options.
Since Version 7.0 GHost++ can automatically calculate some of the map config values if it has access to the actual map file. It can calculate the following:

a.) map_size (the size of the map).
b.) map_info (the CRC32 of the map).
c.) map_crc (this is generated by a custom algorithm - note that due to a naming mixup this is NOT actually the CRC).
d.) map_sha1 (the SHA1 hash of a subset of the map data).
e.) map_width (the playable width of the map).
f.) map_height (the playable height of the map).
g.) map_numplayers (the number of players - used with melee maps only).
h.) map_numteams (the number of teams - used with melee maps only).
i.) map_slot<x> (the slot structure).

Credit goes to Strilanc for figuring out the algorithm for calculating map_crc.
GHost++ performs the following actions at startup:

1.) The bot tries to open the file War3Patch.mpq located in the directory specified by bnet_war3path.
2.) If successful it tries to extract "Scripts\common.j" and "Scripts\blizzard.j" and write them to the directory specified by bot_mapcfgpath.

Note that it is NOT an error if the above process fails.
e.g. if you already have a copy of common.j and blizzard.j in your bot_mapcfgpath then this is not a problem.
e.g. if you don't care about GHost++ automatically calculating map_crc/sha1 then this is not a problem.
GHost++ performs the following actions when loading a map:

1.) The bot tries to open the specified map config file.
2.) The bot tries to open the actual map file as specified by [bot_mappath + map_localpath].
3.) If successful it automatically calculates map_size and map_info.
4.) The bot tries to open "common.j" in the directory specified by bot_mapcfgpath. If it fails it stops trying to calculate map_crc/sha1.
5.) The bot tries to open "blizzard.j" in the directory specified by bot_mapcfgpath. If it fails it stops trying to calculate map_crc/sha1.
6.) The bot tries to open the actual map file as an MPQ archive.
7.) If successful it automatically calculates map_crc/sha1. If it fails it stops trying to calculate map_crc/sha1.
8.) If the bot is able to open the MPQ archive it tries to extract "war3map.w3i" (it keeps this file in memory and does not make a disk copy).
9.) If successful it automatically calculates map_width, map_height, map_slot<x>, map_numplayers, and map_numteams.
10.) If any map values were specified in the map config file the bot uses those values INSTEAD of the automatically calculated ones.

You may wonder why I haven't just included a copy of common.j and blizzard.j instead of going through all this effort to extract it.
This is because common.j and blizzard.j are official Blizzard files and are presumably copyrighted.
If you don't want GHost++ to extract common.j and blizzard.j itself you can simply copy those files to your bot_mapcfgpath yourself and forget about the rest.
This is particularly useful for Linux users who don't want to copy a 25 MB patch file over to their GHost++ server.
So, in summary:

If you want players to be able to download maps:
 - make sure you have a copy of the actual map file on the server and that [bot_mappath + map_localpath] is set to the correct location.
 - make sure bot_allowdownloads = 1

If you want GHost++ to automatically calculate map_size and map_info:
 - make sure you have a copy of the actual map file on the server and that [bot_mappath + map_localpath] is set to the correct location.

If you want GHost++ to automatically calculate map_crc/sha1:
 - make sure you have a copy of common.j and blizzard.j as extracted from War3Patch.mpq in bot_mapcfgpath.
 - make sure you have a copy of the actual map file on the server and that [bot_mappath + map_localpath] is set to the correct location.

If you want GHost++ to automatically calculate map_width, map_height, map_slot<x>, map_numplayers, and map_numteams:
 - make sure you have a copy of the actual map file on the server and that [bot_mappath + map_localpath] is set to the correct location.

If you want GHost++ to automatically extract common.j and blizzard.j from War3Patch.mpq:
 - make sure you have a copy of War3Patch.mpq in bnet_war3path on startup.

Note that some map files are "protected" in such a way that StormLib is unable to read them. In this case the bot might calculate incorrect values.

=======================
Using the "map" Command
=======================

Since Version 13.2 GHost++ supports loading Warcraft III maps without corresponding map config files.
These maps will always be loaded with default options, more specifically:

1.) The map speed will be fast.
2.) The map visibility will be default.
3.) The map observers will be none.
4.) The map flags will be teams together + fixed teams.
5.) The map game type will be custom.

This means you cannot use the !map command to load non-custom (e.g. blizzard or melee) maps at this time.
You will need to create config files for any maps that you want to change these settings for and use the !load command to load them.

===================
Regular Expressions
===================

Since Version 13.2 GHost++ supports regular expressions when loading map config files and map files.
You can control this behaviour with the bot_useregexes config value.
When regexes are disabled, GHost++ will perform a simple partial match on the given pattern. GHost++ does NOT support wildcards such as "*" or "?".
When regexes are enabled, GHost++ will perform a regular expression match on the given pattern.

Examples (regexes disabled):

!load dota -> this matches names such as "dota6.59d.cfg" or "mydota.cfg"
!load war -> this matches names such as "warlock.cfg" or "wormwar.cfg"
!load dota* -> this does NOT match "dota6.59d.cfg" because GHost++ does not support wildcards
!load 59d -> this matches names such as "dota6.59d.cfg"

Examples (regexes enabled):

!load dota -> this matches the single name "dota"
!load dota* -> this does NOT match "dota6.59d.cfg" because regular expressions do not work this way, it matches "dotaaa" and "dotaaaa" instead
!load dota.* -> this matches names such as "dota6.59d.cfg" or "dota6.60.cfg" but NOT "mydota.cfg"
!load .*war.* -> this matches names such as "warlock.cfg" or "wormwar.cfg"

You can search the internet for more information on regular expressions if you are interested in learning how they work as they are quite common.

===========
Using MySQL
===========

Since Version 12.0 GHost++ supports MySQL databases for storing admins/bans/stats/etc...
To configure GHost++ to connect to a MySQL database you just need to specify the following config values in ghost.cfg:

db_type = mysql
db_mysql_server = localhost
db_mysql_database = YOUR_DB
db_mysql_user = YOUR_USERNAME
db_mysql_password = YOUR_PASSWORD
db_mysql_port = 0
db_mysql_botid = 1

You can use a remote MySQL server if you wish, just specify the server and port above (the default MySQL port is 3306).
Please be aware that GHost++ does not cache and retry failed queries so it is possible for GHost++ to lose data when using a remote (or even local) MySQL server.
Create a new database on your MySQL server then run the most recent "mysql_create_tables.sql" file on it.
GHost++ won't create or modify your MySQL database schema like it does with SQLite so you are responsible for making sure your database schema is accurate.
This means you need to keep track of what schema you're using and run the appropriate "mysql_upgrade.sql" file(s) on your database as necessary.
Note that with MySQL you can configure multiple bots to use the same database.
It is recommended that you set db_mysql_botid to a unique value on each bot connecting to the same database but it is not necessary.
The bot ID number is just to help you keep track of which bot the data came from and can be set to the same value on each bot if you wish.

=====================
Automatic Matchmaking
=====================

Since Version 13.0 GHost++ supports automatic matchmaking.
This is an advanced feature and is not recommended unless you have considerable experience with GHost++ and MySQL databases.
Automatic matchmaking attempts to match players of similar skill together for a more enjoyable gaming experience.
To use automatic matchmaking there are several requirements:

1.) You can only use automatic matchmaking with a MySQL database. SQLite databases are not supported.
2.) You can only use automatic matchmaking with custom (e.g. non-melee) maps.
3.) You must specify the "map_matchmakingcategory" value in your map config file.
4.) You must specify the "map_defaultplayerscore" value in your map config file (this is used when balancing the teams).

Here's how it works:

1.) Automatic matchmaking only works with autohosted games. You will need to use the "!autohostmm" command to enable matchmaking.
2.) When a player joins the game, GHost++ checks the MySQL database's "scores" table using the map's "map_matchmakingcategory" for the player's score.
 a.) GHost++ will NEVER write to the scores table. GHost++ does NOT contain a scoring algorithm. It is YOUR responsibility to generate the scores table.
 b.) Scores can be any number, positive or negative, but they should always be greater than -99999 as GHost++ assumes any value less than this represents "no score".

What happens next depends on what your bot_matchmakingmethod config value is set to.
If bot_matchmakingmethod is 0 (no matchmaking):

3.) No matchmaking is performed. Players are allowed to join the game in a first come first served fashion.

If bot_matchmakingmethod is 1 (furthest score matchmaking):

3.) If the game is full the player with the "furthest" score from the average (computed by absolute value) is kicked from the game.
 a.) The kicked player can be the new player if they have the furthest score.
 b.) A player without a score is considered to have the furthest score and will always be kicked in favour of a player with a score.

If bot_matchmakingmethod is 2 (lowest score matchmaking):

3.) If the game is full the player with the lowest score is kicked from the game.
 a.) The kicked player can be the new player if they have the lowest score.
 b.) A player without a score is considered to have the lowest score and will always be kicked in favour of a player with a score.

Then, regardless of bot_matchmakingmethod:

4.) The teams will be automatically rebalanced. For the purpose of balancing, players with no score are considered to have the map_defaultplayerscore.
 a.) Note: The team balancing algorithm cannot be used for 4 teams of 3 players as it is too slow in this case.

Note that GHost++ does not contain a default scoring algorithm.
This means that automatic matchmaking DOES NOT work "out of the box".
You can use the included "update_dota_elo" and "update_w3mmd_elo" projects to generate ELO scores for DotA maps and maps using the W3MMD standard.

===============================================
The W3MMD (Warcraft III Map Meta Data) Standard
===============================================

Since Version 13.0 GHost++ supports saving arbitrary statistics to the database from maps which support the W3MMD standard.
The data that is saved depends on how the map is written.
This will not work with every map, only with maps that specifically support the W3MMD standard.

Here's how to use it:

1.) Obtain a map that supports the W3MMD standard.
2.) Choose a category for the map. The category is used to allow multiple versions of the same map to share the same statistics.
 a.) For example, you might use the category "civwars" for Civ Wars 2.30 and also for Civ Wars 2.31 and future versions.
 b.) You can change the category at any time although only future statistics will be affected, it's just an arbitrary name used to identify where the statistics came from.
3.) Open the map config file.
 a.) Add "map_type = w3mmd".
 b.) Add "map_statsw3mmdcategory = <your category>".

GHost++ will now save map statistics to the database under the specified category.
There is no way to display these stats from within GHost++.
The intention is that you will display the stats externally, e.g. on a website, or you will use the update_w3mmd_elo project to generate scores for use with matchmaking.

==========================================
The HCL (HostBot Command Library) Standard
==========================================

Since Version 14.0 GHost++ supports passing a very limited amount of arbitrary data to maps which support the HCL standard.
The passed data is interpreted by each map individually and does not necessarily have the same meaning for different maps.
It is intended that maps will use the HCL system to allow the bot to set game modes or other "startup" data which should not be specified by the players themselves.
For example, when autohosting DotA games it is desirable for the bot to set the game mode rather than the players so that it will be set correctly.

Here's how to use it:

1.) Obtain a map that supports the HCL standard.
2.) Set the "HCL Command String" (the data to be passed to the map) by using the !hcl command in the lobby.
 a.) The HCL Command String can only contain one character per player and/or computer player when the game starts.
 b.) It can also only contain characters from a limited set (lowercase letters, numbers, and a small number of special characters).
 c.) For example, with 4 players (or 3 players and 1 computer, and so on) the HCL Command String can be at most 4 characters long.
 d.) The HCL Command String has a different meaning for different maps so you will need to find out what the HCL means for each map before you use it.
 e.) Use the !clearhcl command to clear the HCL Command String if you change your mind and don't wish to use it.
3.) Start the game.

The HCL system works by embedding information in the player handicaps in such a way that the original handicaps can be restored after the map finishes loading.
This means that if you try to set the HCL command String on a map that doesn't support the standard, the bot will drastically modify the player handicaps.
However, maps that do not support the standard will not restore the handicaps to their original values and your game will be ruined.
Do not attempt to set the HCL Command String on a map that doesn't support the HCL standard.

If you want to set the HCL for every game automatically (e.g. when autohosting):

1.) Create a map config file for the map you wish to autohost with.
2.) Set "map_defaulthcl = <something>" in your map config file.
3.) Load your map config file with the !load command.
4.) Start autohosting with the !autohost command. The bot will use the default HCL specified in your map config file.
 a.) Note that this is a somewhat clumsy method of setting the default HCL and may be modified in future versions of GHost++.

Map makers take note: You may wish to include the HCL system in your map even if you don't intend to use the HCL Command String anywhere.
This is because including the HCL system immunizes your map from accidental disruption of the player handicaps due to setting the HCL Command String on an unsupported map.

======
Warden
======

On April 14th, 2009 Blizzard enabled the Warden anti cheat system when connecting to official battle.net servers. This does not affect PVPGN servers and LAN games.
Unfortunately there is currently no portable method to handle the Warden requests sent by battle.net.
To get around this, GHost++ requires an external "Warden server".
The Warden server is a type of BNLS server running on a Windows computer which GHost++ connects to. The Warden server helps GHost++ generate the correct Warden responses.
GHost++ does not send any sensitive information to the BNLS server. It is NOT possible for the BNLS server operator to steal your CD keys or username or password.
However, GHost++ assumes that the BNLS server is sending correct responses to the Warden requests.
You will need to trust the BNLS server author and the operator as it is possible they could forge fake Warden responses which could indicate to battle.net that you are cheating.
This would result in the banning of your CD keys.
It's even possible that Blizzard could update their Warden code in such a way that the Warden server would generate incorrect responses resulting in your CD keys getting banned.
It's entirely at your own risk that you connect to battle.net now that Warden is active (although it was always at your own risk it's just riskier now).
Your CD keys may be banned at any time.
To configure GHost++ to use an external Warden server:

1.) This is only necessary for official battle.net servers. You do not need to do this for PVPGN servers.
2.) For each battle.net connection, set the following config values in your main config file (e.g. ghost.cfg):
 a.) bnet<x>_bnlsserver = <BNLS server address>
 b.) bnet<x>_bnlsport = <BNLS server port> (the default is 9367)
 c.) bnet<x>_bnlswardencookie = <number> (the default is 0)

The "bnlswardencookie" is used to identify bots connecting to the same BNLS server from the same IP address.
If you are running multiple bots on the same computer or from the same NAT (e.g. behind the same router) then you need to set the bnlswardencookie to a UNIQUE number on each bot.
You will also need to do this if you are connecting to multiple realms from the same bot and using the same BNLS server for each realm.
For example, set it to "1" on the first bot, and "2" on the second bot, etc...
If two bots on the same IP address are using the same bnlswardencookie the Warden server may generate incorrect responses which may result in your CD keys getting banned.
If GHost++ loses connection to the BNLS server, it will stop responding to the Warden requests sent by battle.net. A message will also be printed to your console.
In this case battle.net will kick you from the server after 2 minutes of not responding to Warden requests.
When GHost++ automatically reconnects to battle.net it will try to connect to the BNLS server again. If successful it will continue operating as normal.

==========================
The "Load In Game" Feature
==========================

Since Version 13.3 GHost++ supports a new "load in game" feature. Credit goes to Strilanc for the idea.
This feature is disabled by default although it is possible it will become enabled by default in a future version after enough testing.
To enable loading in game for a particular map, simply open the map config file and add "map_loadingame = 1".
When loading in game is enabled, as each player finishes loading the map they will immediately start the game rather than waiting at the loading screen.
However, since other players are still loading the map, the lag screen will appear and every player still loading the map will be listed.
As players finish loading the map they will be removed from the lag screen and a chat message will be printed.
Since it is possible it will take more than 45 seconds for everyone to load, the lag screen timer will be reset every 30 seconds.
This feature allows players to see who is still loading the map and allows them to chat while waiting.
Please note that some maps with intro cutscenes do not permit chatting in the first few seconds and therefore it is not possible to chat while waiting.

========
Commands
========

Parameters in angled brackets <like this> are required and parameters in square brackets [like this] are optional.

*** In battle.net (via local chat or whisper at any time):

!addadmin <name>                add a new admin to the database for this realm
!addban <name> [reason]         add a new ban to the database for this realm
!announce <sec> <msg>           set the announce message (the bot will print <msg> every <sec> seconds in the lobby), use "off" to disable the announce message
!autohost <m> <p> <n>           auto host up to <m> games, auto starting when <p> players have joined, with name <n>, use "off" to disable auto hosting
!autohostmm <m> <p> <a> <b> <n> auto host up to <m> games, auto starting when <p> players have joined, with name <n>, with matchmaking enabled and min score <a>, max score <b>
!autostart <players>            auto start the game when the specified number of players have joined, use "off" to disable auto start
!ban                            alias to !addban
!channel <name>                 change battle.net channel
!checkadmin <name>              check if a user is an admin on this realm
!checkban <name>                check if a user is banned on this realm
!close <number> ...             close slot
!closeall                       close all open slots
!countadmins                    display the total number of admins for this realm
!countbans                      display the total number of bans for this realm
!dbstatus                       show database status information
!deladmin <name>                remove an admin from the database for this realm
!delban <name>                  remove a ban from the database for all realms
!disable                        disable creation of new games
!downloads <0|1|2>              disable/enable/conditional map downloads
!enable                         enable creation of new games
!end <number>                   end the specified game in progress (disconnect everyone)
!exit [force|nice]              shutdown ghost++, optionally add [force] to skip checks or [nice] to allow running games to finish first
!getclan                        refresh the internal copy of the clan members list
!getfriends                     refresh the internal copy of the friends list
!getgame <number>               display information about a game in progress
!getgames                       display information about all games
!hold <name> ...                hold a slot for someone
!hostsg <name>                  host a saved game
!load <pattern>                 load a map config file (".cfg" files), leave blank to see current map
!loadsg <filename>              load a saved game
!map <pattern>                  load a map file (".w3m" and ".w3x" files), leave blank to see current map
!open <number> ...              open slot
!openall                        open all closed slots
!priv <name>                    host private game
!privby <owner> <name>          host private game by another player (gives <owner> access to admin commands in the game lobby and in the game)
!pub <name>                     host public game
!pubby <owner> <name>           host public game by another player (gives <owner> access to admin commands in the game lobby and in the game)
!quit [force|nice]              alias to !exit
!reload                         reload the main configuration files
!say <text>                     send <text> to battle.net as a chat command
!saygame <number> <text>        send <text> to the specified game in progress
!saygames <text>                send <text> to all games
!sp                             shuffle players
!start [force]                  start game, optionally add [force] to skip checks
!stats [name]                   display basic player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!statsdota [name]               display DotA player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!swap <n1> <n2>                 swap slots
!unban <name>                   alias to !delban
!unhost                         unhost game in lobby
!version                        display version information (can be used by non admins)
!wardenstatus                   show warden status information

*** In game lobby:

!a                      alias to !abort
!abort                  abort countdown
!addban <name> <reason> add a new ban to the database (it tries to do a partial match)
!announce <sec> <msg>   set the announce message (the bot will print <msg> every <sec> seconds), leave blank or "off" to disable the announce message
!autostart <players>    auto start the game when the specified number of players have joined, leave blank or "off" to disable auto start
!autosave <on/off>      enable or disable autosaving
!ban                    alias to !addban
!check <name>           check a user's status (leave blank to check your own status)
!checkban <name>        check if a user is banned on any realm
!checkme                check your own status (can be used by non admins, sends a private message visible only to the user)
!close <number> ...     close slot
!closeall               close all open slots
!comp <slot> <skill>    create a computer in slot <slot> of skill <skill> (skill is 0 for easy, 1 for normal, 2 for insane)
!compcolour <s> <c>     change a computer's colour in slot <s> to <c> (c goes from 1 to 12)
!comphandicap <s> <h>   change a computer's handicap in slot <s> to <h> (h is 50, 60, 70, 80, 90, or 100)
!comprace <s> <r>       change a computer's race in slot <s> to <r> (r is "human", "orc", "night elf", "undead", or "random")
!compteam <s> <t>       change a computer's team in slot <s> to <t> (t goes from 1 to # of teams)
!dl <name>              alias to !download
!download <name>        allow a user to start downloading the map (only used with conditional map downloads, it tries to do a partial match)
!fakeplayer             create or delete a fake player to occupy a slot during the game (the player will not do anything except stay AFK)
!from                   display the country each player is from
!hold <name> ...        hold a slot for someone
!kick <name>            kick a player (it tries to do a partial match)
!latency <number>       set game latency (50-500), leave blank to see current latency
!lock                   lock the game so only the game owner can run commands
!messages <on/off>      enable or disable local admin messages for this game (battle.net messages relayed to local admins in game)
!mute <name>            mute a player (it tries to do a partial match)
!open <number> ...      open slot
!openall                open all closed slots
!owner [name]           set game owner to yourself, optionally add [name] to set game owner to someone else
!ping [number]          ping players, optionally add [number] to kick players with ping above [number]
!priv <name>            rehost as private game
!pub <name>             rehost as public game
!refresh <on/off>       enable or disable refresh messages
!say <text>             send <text> to all connected battle.net realms as a chat command (this command is HIDDEN from other players)
!sendlan <ip> [port]    send a fake LAN message to IP address <ip> and port [port], default port is 6112 if not specified
!sp                     shuffle players
!start [force]          start game, optionally add [force] to skip checks
!stats [name]           display basic player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!statsdota [name]       display DotA player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!swap <n1> <n2>         swap slots
!synclimit <number>     set sync limit for the lag screen (10-10000), leave blank to see current sync limit
!unhost                 unhost game
!unlock                 unlock the game
!unmute <name>          unmute a player (it tries to do a partial match)
!version                display version information (can be used by non admins, sends a private message visible only to the user)
!virtualhost <name>     change the virtual host name
!votecancel             cancel a votekick
!votekick <name>        start a votekick (it tries to do a partial match, can be used by non admins)
!w <name> <message>     send a whisper on every connected battle.net realm from the bot's account to the player called <name> (this command is HIDDEN from other players)
!yes                    register a vote in the votekick (can be used by non admins)

*** In game:

!addban <name> <reason> add a new ban to the database (it tries to do a partial match)
!autosave <on/off>      enable or disable autosaving
!ban                    alias to !addban
!banlast <reason>       ban the last leaver
!check <name>           check a user's status (leave blank to check your own status)
!checkban <name>        check if a user is banned on any realm
!checkme                check your own status (can be used by non admins, sends a private message visible only to the user)
!drop                   drop all lagging players
!end                    end the game (disconnect everyone)
!from                   display the country each player is from
!kick <name>            kick a player (it tries to do a partial match)
!latency <number>       set game latency (50-500), leave blank to see current latency
!lock                   lock the game so only the game owner can run commands
!messages <on/off>      enable or disable local admin messages for this game (battle.net messages relayed to local admins in game)
!mute <name>            mute a player (it tries to do a partial match)
!muteall                mute global chat (allied and private chat still works)
!owner [name]           set game owner to yourself, optionally add [name] to set game owner to someone else
!ping                   ping players
!say <text>             send <text> to all connected battle.net realms as a chat command (this command is HIDDEN from other players)
!stats [name]           display basic player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!statsdota [name]       display DotA player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!synclimit <number>     set sync limit for the lag screen (10-10000), leave blank to see current sync limit
!unlock                 unlock the game
!unmute <name>          unmute a player (it tries to do a partial match)
!unmuteall              unmute global chat
!version                display version information (can be used by non admins, sends a private message visible only to the user)
!votecancel             cancel a votekick
!votekick <name>        start a votekick (it tries to do a partial match, can be used by non admins)
!w <name> <message>     send a whisper on every connected battle.net realm from the bot's account to the player called <name> (this command is HIDDEN from other players)
!yes                    register a vote in the votekick (can be used by non admins)

*** In admin game lobby:

!addadmin <name> [realm]        add a new admin to the database for the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!autohost <m> <p> <n>           auto host up to <m> games, auto starting when <p> players have joined, with name <n>, use "off" to disable auto hosting
!autohostmm <m> <p> <a> <b> <n> auto host up to <m> games, auto starting when <p> players have joined, with name <n>, with matchmaking enabled and min score <a>, max score <b>
!checkadmin <name> [realm]      check if a user is an admin for the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!checkban <name> [realm]        check if a user is banned on the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!countadmins [realm]            display the total number of admins for the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!countbans [realm]              display the total number of bans on the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!deladmin <name> [realm]        remove an admin from the database for the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!delban <name>                  remove a ban from the database for all realms
!disable                        disable creation of new games
!downloads <0|1|2>              disable/enable/conditional map downloads
!enable                         enable creation of new games
!end <number>                   end a game in progress (disconnect everyone)
!exit [force|nice]              shutdown ghost++, optionally add [force] to skip checks or [nice] to allow running games to finish first
!getgame <number>               display information on a game in progress
!getgames                       display information on all games
!hostsg <name>                  host a saved game
!load <pattern>                 load a map config file (".cfg" files), leave blank to see current map
!loadsg <filename>              load a saved game
!map <pattern>                  load a map file (".w3m" and ".w3x" files), leave blank to see current map
!password <p>                   login (the password is set in ghost.cfg with admingame_password)
!priv <name>                    host private game
!privby <owner> <name>          host private game by another player (gives <owner> access to admin commands in the game lobby and in the game)
!pub <name>                     host public game
!pubby <owner> <name>           host public game by another player (gives <owner> access to admin commands in the game lobby and in the game)
!quit [force|nice]              alias to !exit
!reload                         reload the main configuration files
!say <text>                     send <text> to all connected battle.net realms as a chat command
!saygame <number> <text>        send <text> to the specified game in progress
!saygames <text>                send <text> to all games
!unban <name>                   alias to !delban
!unhost                         unhost game in lobby (not the admin game)
!w <name> <message>             send a whisper on every connected battle.net realm from the bot's account to the player called <name>

============================
Compiling GHost++ on Windows
============================

1. Download Visual C++ 2008 Express Edition at http://www.microsoft.com/express/
2. Download Boost 1.38.0 and configure Visual C++ to use the Boost headers.
 a. Go to http://www.boost.org/ and download and extract Boost 1.38.0.
 b. Open Visual C++ and click Tools -> Options -> Projects and Solutions -> VC++ Directories -> Show directories for: Include files.
 c. Select the path to wherever you downloaded Boost (e.g. for me it's "D:\boost_1_38_0").
3. Download the precompiled GHost++ boost libraries for Windows from http://code.google.com/p/ghostplusplus/ and extract them to the "boost\lib\" subdirectory.
4. Open ghost.sln
5. Choose a Debug or Release version with or without MySQL support
6. Build

Notes:

The ghost.sln includes four projects: bncsutil, ghost, update_dota_elo, and update_w3mmd_elo.
bncsutil was not written by me and can be found here: http://code.google.com/p/bncsutil/
bncsutil requires gmp (GNU Multiple Precision Arithmetic Library) but I have included a prebuilt gmp.lib so you don't need to build one yourself unless you want to.
gmp can be found here: http://gmplib.org/
The included gmp.lib is a Release version built with my own copy of Visual C++ 2008 Express Edition.
GHost++ requires StormLib but I have included a prebuilt StormLibRAS.lib so you don't need to build one yourself unless you want to.
StormLib can be found here: http://www.zezula.net/en/mpq/stormlib.html
The included StormLibRAS.lib is a Static Release version built with my own copy of Visual C++ 2008 Express Edition.
I made some modifications to StormLib to make it compile on Linux. The modified project can be found in the included StormLib directory if you want to compile it yourself.

==========================
Compiling GHost++ on Linux
==========================

You'll need GMP (GNU Multiple Precision Arithmetic Library) to build bncsutil.
If you have yum you can install it with the following command:

1. yum install gmp-devel

Then, assuming you extracted this archive to your home dir (~):

2. cd ~/ghost/bncsutil/src/bncsutil/
3. make

You'll need StormLib to build GHost++.
I made some modifications to StormLib to make it compile on Linux. The modified project can be found in the included StormLib directory.
StormLib requires libbz2 and zlib (try running "yum install bzip2-devel" and/or "yum install zlib-devel" if it refuses to compile).

4. cd ~/ghost/StormLib/stormlib/
5. make

You'll need the MySQL development libraries to build GHost++. You can probably use whatever version your package manager has available as they are usually kept up to date.

6. yum install mysql-devel

You'll need Boost version 1.38.0 (or potentially any newer versions). If your package manager has this version available please use it instead of installing it manually.
To install Boost manually:

7. Download and extract Boost 1.38.0 from http://www.boost.org/
8. su to root.
9. ./configure --prefix=/usr --with-libraries=date_time,thread,system,filesystem,regex
10. Edit the newly created Makefile with your favourite text editor. The second line should be "BJAM_CONFIG=". Replace it with "BJAM_CONFIG= --layout=system".
11. make install

Once it's built you can continue:

12. cd ~/ghost/ghost/
13. make

========================
Running GHost++ on Linux
========================

You will need to copy ~/ghost/bncsutil/src/bncsutil/libbncutil.so to /usr/local/lib/ or otherwise set LD_LIBRARY_PATH so it can find the bncsutil library.
You will need to copy ~/ghost/StormLib/stormlib/libStorm.so to /usr/local/lib/ or otherwise set LD_LIBRARY_PATH so it can file the StormLib library.
You will also need to copy game.dll, Storm.dll, and war3.exe (and possibly War3Patch.mpq) from a valid Warcraft III installation to the location specified in your ghost.cfg.

Note that some systems do not automatically load libraries from /usr/local/lib/
If you are having trouble loading libbncsutil and libStorm, you can either copy them to /usr/lib/ instead (not recommended) or set LD_LIBRARY_PATH.
To set LD_LIBRARY_PATH type "export LD_LIBRARY_PATH=/usr/local/lib/" without the quotes.
If you are a Linux expert and you know a better solution to this problem please feel free to join the GHost++ forums and let us know.

=========================
Compiling GHost++ on OS X
=========================

Nothing here yet.

=======================
Running GHost++ on OS X
=======================

Nothing here yet.

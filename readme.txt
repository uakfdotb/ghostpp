=====================
GHost++ Version 11.01
=====================

GHost++ is a port of the original GHost project to C++ (ported by Trevor Hogan).
The original GHost project can be found here: http://ghost.pwner.org/
You can compile and run GHost++ on Windows or Linux with this release although the code should be mostly portable to other operating systems such as OS X with only very minor changes.
Since it is written in native code you do not need to install the .NET framework on Windows or Mono on Linux.

=============
Configuration
=============

GHost++ is configured via the three configuration files ghost.cfg, map.cfg, and language.cfg.
The program itself runs in console mode and does not take any console input (it outputs messages to the console for information purposes only).

***You need to edit ghost.cfg before running GHost++***
***You need to edit ghost.cfg before running GHost++***
***You need to edit ghost.cfg before running GHost++***

GHost++ takes one command line argument, the name of the main config file. It defaults to ghost.cfg if no arguments are provided.

==============
Required Files
==============

If you want to be able to connect to battle.net:

-> "game.dll" in your bot_war3path
-> "storm.dll" in your bot_war3path (Linux users note that GHost++ searches for this file in all lowercase)
-> "war3.exe" in your bot_war3path

If you want GHost++ to automatically extract blizzard.j and common.j on startup (used when automatically calculating map values):

-> "War3Patch.mpq" in your bot_war3path

If you want GHost++ to automatically calculate map values:

-> "blizzard.j" in your bot_mapcfgpath
-> "common.j" in your bot_mapcfgpath

Note that blizzard.j and common.j will be automatically extracted from War3Patch.mpq if you provide GHost++ with your War3Patch.mpq file (as mentioned above).

===============
How Admins Work
===============

There are three types of admins:

1.) Root Admins.

Each battle.net server has a root admin defined in ghost.cfg.
Root admins have access to every command both in battle.net and in the lobby and ingame.
In particular this includes !addadmin, !checkadmin, !countadmins, !deladmin, !exit, and !quit among others.

2.) Game Owners.

Each game has an owner defined as the user who ran the !priv or !pub command or the user specified with the !privby or !pubby command.
Game owners have access to every command in the lobby and ingame but NO commands in battle.net.
You can think of the game owner as a temporary admin for one game only - it doesn't have to be a root admin or a regular admin.
The game owner is also the only user who can use commands inside a game which is locked (see the !lock and !unlock commands for more information).
The game owner for a particular can be changed after the game is created with the !owner command.

3.) Admins.

Each battle.net server has a list of admins.
Admins are added and deleted by the root admin only.
Admins have access to most commands in battle.net and all commands in the lobby and ingame.

So, how does GHost++ determine whether a user has admin access in the lobby and ingame? The user must meet several criteria:

1.) The user must be spoof checked.
 a.) If spoof checking is disabled they must still manually spoof check by whispering the bot.
 b.) GHost++ treats the game owner as spoof checked even if they aren't. This is because it's impossible to spoof check in LAN games.
2.) The user must be either a root admin on the realm they spoof checked on, or they must be the game owner, or they must be an admin on the realm they spoof checked on.

If the bot is ignoring you in the lobby and ingame it's most likely because you haven't spoof checked.

=============
How Bans Work
=============

Each ban is attached to a specific realm.
When you run the !addban or !ban commands on battle.net, the ban will be attached to the realm you ran the command on.
When you run the !addban or !ban commands in a game, it works like this:

1.) If the banned user was spoof checked, the ban will be attached to the realm they spoof checked on.
2.) If the banned user was not spoof checked, a new ban will be created for each defined realm and attached to that realm.

When you run the !delban or !unban commands all bans for that username will be deleted regardless of realm.
GHost++ considers a user to be banned if it finds a ban for that user on ANY of the defined realms.
This is because GHost++ doesn't wait for the user to spoof check before kicking them, instead it immediately kicks thems and assumes they came from the banned realm.

=======================
How Reserved Slots Work
=======================

Each game has a list of reserved players.
The list always starts out empty when starting a new game.
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
3.) If a slot occupied by a non-reserved player is found that player will be kicked and the reserved player will join that slot.

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

================================
Using GHost++ on Multiple Realms
================================

Since Version 10.0 GHost++ can connect to multiple realms at the same time.
Here's how:

1.) When GHost++ starts up it reads up to 9 sets of battle.net connection information from ghost.cfg.
2.) A set of battle.net connection information contains the following keys:
 a.) *_server (required)
 b.) *_cdkeyroc (required)
 c.) *_cdkeytft (required)
 d.) *_username (required)
 e.) *_password (required)
 f.) *_firstchannel
 g.) *_rootadmin
 h.) *_commandtrigger
 i.) *_custom_war3version
 j.) *_custom_exeversion
 k.) *_custom_exeversionhash
 l.) *_custom_passwordhashtype
3.) GHost++ will search for battle.net connection information by replacing the "*" in each key above with "bnet_" then "bnet2_" then "bnet3_" and so on until "bnet9_".
 a.) Note that GHost++ doesn't search for "bnet1_" for backwards compatibility reasons.
4.) If GHost++ doesn't find a *_server key it stops searching for any further battle.net connection information.
5.) If any of the required keys are missing GHost++ will skip that set of battle.net connection information and start searching for the next one.

============
Auto Hosting
============

Since Version 10.4 GHost++ can automatically host and start public games without admin intervention.
Here's how:

1.) Load the map you want to auto host with the !load or !map commands.
2.) Use the !autohost command to start the auto hosting process.
3.) The bot will keep track of which map config file was loaded when the !autohost command was used and it will reload that map config file before auto hosting each game.

The autohost command takes three arguments: !autohost <m> <p> <n>

1.) <m> is the maximum number of games to auto host.
 a.) This can be less than bot_maxgames, especially if you want to leave some games open for your own use.
 b.) The bot will not auto host any new games while the number of running games is <m> or greater. The running games do not have to be auto hosted games.
2.) <p> is the number of players required to auto start the game.
 a.) When <p> players have joined the game it will ensure that all players have the map and have been ping checked before starting.
 b.) Spoof checks are not required when auto starting games.
3.) <n> is the game name.
 a.) The bot will automatically add a unique game number to the end of <n> before auto hosting each game.
 b.) For example if you specify "BattleShips Auto" it will create games like "BattleShips Auto #1" and "BattleShips Auto #2" and so on.

It will take up to 30 seconds before each game is created.
The previously loaded map config file will not be reloaded after the bot auto hosts each game.

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
d.) map_width (the playable width of the map).
e.) map_height (the playable height of the map).
f.) map_numplayers (the number of players - used with melee maps only).
g.) map_numteams (the number of teams - used with melee maps only).
h.) map_slot<x> (the slot structure).

Credit goes to Strilanc for figuring out the algorithm for calculating map_crc.
GHost++ performs the following actions at startup:

1.) The bot tries to open the file War3Patch.mpq located in the directory specified by bnet_war3path.
2.) If successful it tries to extract "Scripts\common.j" and "Scripts\blizzard.j" and write them to the directory specified by bot_mapcfgpath.

Note that it is NOT an error if the above process fails.
e.g. if you already have a copy of common.j and blizzard.j in your bot_mapcfgpath then this is not a problem.
e.g. if you don't care about GHost++ automatically calculating map_crc then this is not a problem.
GHost++ performs the following actions when loading a map:

1.) The bot tries to open the specified map config file.
2.) The bot tries to open the actual map file as specified by [bot_mappath + map_localpath].
3.) If successful it automatically calculates map_size and map_info.
4.) The bot tries to open "common.j" in the directory specified by bot_mapcfgpath. If it fails it stops trying to calculate map_crc.
5.) The bot tries to open "blizzard.j" in the directory specified by bot_mapcfgpath. If it fails it stops trying to calculate map_crc.
6.) The bot tries to open the actual map file as an MPQ archive.
7.) If successful it automatically calculates map_crc. If it fails it stops trying to calculate map_crc.
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

If you want GHost++ to automatically calculate map_crc:
 - make sure you have a copy of common.j and blizzard.j as extracted from War3Patch.mpq in bot_mapcfgpath.
 - make sure you have a copy of the actual map file on the server and that [bot_mappath + map_localpath] is set to the correct location.

If you want GHost++ to automatically calculate map_width, map_height, map_slot<x>, map_numplayers, and map_numteams:
 - make sure you have a copy of the actual map file on the server and that [bot_mappath + map_localpath] is set to the correct location.

If you want GHost++ to automatically extract common.j and blizzard.j from War3Patch.mpq:
 - make sure you have a copy of War3Patch.mpq in bnet_war3path on startup.

Note that some map files are "protected" in such a way that StormLib is unable to read them. In this case the bot might calculate incorrect values.

========
Commands
========

In battle.net (via local chat or whisper at any time):

!addadmin <name>         add a new admin to the database for this realm
!addban <name> <reason>  add a new ban to the database for this realm
!announce <sec> <msg>    set the announce message (the bot will print <msg> every <sec> seconds in the game lobby), leave blank or "off" to disable the announce message
!autohost <m> <p> <n>    auto host up to <m> games, auto starting when <p> players have joined, with name <n>, leave blank or "off" to disable auto hosting
!autostart <players>     auto start the game when the specified number of players have joined, leave blank or "off" to disable auto start
!ban                     alias to !addban
!channel <name>          change channel
!checkadmin <name>       check if a user is an admin on this realm
!checkban <name>         check if a user is banned on this realm
!close <number> ...      close slot
!closeall                close all open slots
!countadmins             display the total number of admins for this realm
!countbans               display the total number of bans for this realm
!deladmin <name>         remove an admin from the database for this realm
!delban <name>           remove a ban from the database for this realm
!disable                 disable creation of new games
!enable                  enable creation of new games
!end <number>            end a game in progress (disconnect everyone)
!exit [force]            shutdown ghost++, optionally add [force] to skip checks
!getclan                 refresh the internal copy of the clan members list
!getfriends              refresh the internal copy of the friends list
!getgame <number>        display information on a game in progress
!getgames                display information on all games
!hold <name> ...         hold a slot for someone
!hostsg <name>           host a saved game
!load <filename>         load a config file (for changing maps), leave blank to see current map - the ".cfg" is automatically appended to the filename
!loadsg <filename>       load a saved game
!map <filename>          alias to !load
!open <number> ...       open slot
!openall                 open all closed slots
!priv <name>             host private game
!privby <owner> <name>   host private game by another player (gives <owner> access to admin commands in the game lobby and in the game)
!pub <name>              host public game
!pubby <owner> <name>    host public game by another player (gives <owner> access to admin commands in the game lobby and in the game)
!quit [force]            alias to !exit
!say <text>              send <text> to battle.net as a chat command
!saygame <number> <text> send <text> to the specified game in progress
!saygames <text>         send <text> to all games
!sp                      shuffle players
!start [force]           start game, optionally add [force] to skip checks
!stats [name]            display basic player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!statsdota [name]        display DotA player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!swap <n1> <n2>          swap slots
!unban                   alias to !delban
!unhost                  unhost game in lobby
!version                 display version information (can be used by non admins)

In game lobby:

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
!from                   display the country each player is from
!hold <name> ...        hold a slot for someone
!kick <name>            kick a player (it tries to do a partial match)
!latency <number>       set game latency (50-500), leave blank to see current latency
!lock                   lock the game so only the game owner can run commands
!open <number> ...      open slot
!openall                open all closed slots
!owner [name]           set game owner to yourself, optionally add [name] to set game owner to someone else
!ping [number]          ping players, optionally add [number] to kick players with ping above [number]
!priv <name>            rehost as private game
!pub <name>             rehost as public game
!refresh <on/off>       enable or disable refresh messages
!sendlan <ip> [port]    send a fake LAN message to IP address <ip> and port [port], default port is 6112 if not specified
!sp                     shuffle players
!start [force]          start game, optionally add [force] to skip checks
!stats [name]           display basic player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!statsdota [name]       display DotA player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!swap <n1> <n2>         swap slots
!synclimit <number>     set sync limit for the lag screen (10-10000), leave blank to see current sync limit
!unhost                 unhost game
!unlock                 unlock the game
!version                display version information (can be used by non admins)

In game:

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
!muteall                mute global chat (allied and private chat still works)
!owner [name]           set game owner to yourself, optionally add [name] to set game owner to someone else
!ping                   ping players
!stats [name]           display basic player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!statsdota [name]       display DotA player statistics, optionally add [name] to display statistics for another player (can be used by non admins)
!synclimit <number>     set sync limit for the lag screen (10-10000), leave blank to see current sync limit
!unlock                 unlock the game
!unmuteall              unmute global chat
!version                display version information (can be used by non admins)

In admin game lobby:

!addadmin <name> <realm>   add a new admin to the database for the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!checkadmin <name> <realm> check if a user is an admin for the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!countadmins <realm>       display the total number of admins for the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!deladmin <name> <realm>   remove an admin from the database for the specified realm (if only one realm is defined in ghost.cfg it uses that realm instead)
!disable                   disable creation of new games
!enable                    enable creation of new games
!end <number>              end a game in progress (disconnect everyone)
!exit [force]              shutdown ghost++, optionally add [force] to skip checks
!getgame <number>          display information on a game in progress
!getgames                  display information on all games
!hostsg <name>             host a saved game
!load <filename>           load a config file (for changing maps), leave blank to see current map - the ".cfg" is automatically appended to the filename
!loadsg <filename>         load a saved game
!map <filename>            alias to !load
!password <p>              login (the password is set in ghost.cfg with admingame_password)
!priv <name>               host private game
!privby <owner> <name>     host private game by another player (gives <owner> access to admin commands in the game lobby and in the game)
!pub <name>                host public game
!pubby <owner> <name>      host public game by another player (gives <owner> access to admin commands in the game lobby and in the game)
!quit [force]              alias to !exit
!saygame <number> <text>   send <text> to the specified game in progress
!saygames <text>           send <text> to all games
!unhost                    unhost game

============================
Compiling GHost++ on Windows
============================

1. Download Visual C++ 2008 Express Edition at http://www.microsoft.com/express/
2. Open ghost.sln
3. Choose a Debug or Release version
4. Build

Notes:

The ghost.sln includes two projects, bncsutil and ghost.
bncsutil was not written by me and can be found here: http://code.google.com/p/bncsutil/
bncsutil requires gmp (GNU Multiple Precision Arithmetic Library) but I have included a prebuilt gmp.lib so you don't need to build one yourself unless you want to.
gmp can be found here: http://gmplib.org/
The included gmp.lib is a Release version built with my own copy of Visual C++ 2008 Express Edition.
GHost++ requires StormLib but I have included a prebuilt StormLibRAS.lib so you don't need to build one yourself unless you want to.
StormLib can be found here: http://www.zezula.net/en/mpq/stormlib.html
The included StormLibRAS.lib is a Static Release version built with my own copy of Visual C++ 2008 Express Edition.
I made some modifications to StormLib to make it compile on Linux. The modified project can be found in the included StormLib directory if you want to compile it yourself.
When you build bncsutil the compiler will ouput some warnings about Wp64 deprecation and unsafe functions but you can ignore these.
When you build a Debug version of bncsutil the linker will output a lot of warnings about missing debug information due to gmp.lib being a Release version but you can ignore these.
When you build GHost++ the compiler will output a lot of warnings when compiling sqlite3.c but you can ignore these.
When you build a Debug version of GHost++ the linker will out a warning about conflicting default libs due to StormLibRAS.lib being a Release version but you can ignore this.

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

Once it's built you can continue:

6. cd ~/ghost/ghost/
7. make

========================
Running GHost++ on Linux
========================

You will need to copy ~/ghost/bncsutil/src/bncsutil/libbncutil.so to /usr/local/lib/ or otherwise set LD_LIBRARY_PATH so it can find the bncsutil library.
You will need to copy ~/ghost/StormLib/stormlib/libStorm.so to /usr/local/lib/ or otherwise set LD_LIBRARY_PATH so it can file the StormLib library.
You will also need to copy game.dll, storm.dll, and war3.exe from a valid Warcraft III installation to the location specified in your ghost.cfg
GHost++ searches for "storm.dll" in all lowercase not "Storm.dll" so you may need to rename this file if yours is capitalized.

=========
CHANGELOG
=========

Version 11.01
 - bot_savereplays now defaults to 0
 - added new map configs for DotA 6.57
 - modified GHost++ to prevent excessive traffic when connecting to PVPGN servers

Version 11.0
 - updated SQLite to SQLite 3.6.4
 - added zlib to the project
 - modified the StormLib Visual C++ project file to fix a build confliction with zlib on Windows
 - added support for autosaving games and loading games
  * see the "Using Saved Games" section of this readme for more information
 - added support for automatically saving replays
  * see the "Saving Replays" section of this readme for more information
 - added new config value bot_savegamepath to specify the directory where saved games will be loaded from
 - added new config value bot_autosave to enable or disable autosaving by default
 - added new config value bot_savereplays to enable or disable automatic saving of replays
 - added new config value bot_replaypath to specify the directory where replays will be saved to
 - added new command !loadsg to load a saved game
 - added new command !hostsg to host a saved game
 - added new command !autosave to enable or disable autosaving for a particular game
 - added new command !saygame to send a message to a specified game in progress
 - added new command !checkme for non admins to use (the response to this command is sent privately)
 - the "game is locked" response is now sent privately
 - the "game refreshed" message is now printed only once per refresh rather than once per server per refresh
 - the !ping command can now be used ingame
 - the !from command can now be used ingame
 - pings are now sent to players ingame in order to continue tracking player pings after the game has started
 - pings are no longer sent to players who are downloading the map
 - when using the !hold command if the player is already in the game they will now be immediately upgraded to reserved status
 - the !saygame and !saygames commands now prefix your message with "ADMIN: "
 - the !check and !checkme commands now display ping and from fields in addition to the other fields
 - the !addban and !ban commands can now be used in the game lobby
 - game timestamps are now calculated based on actual ingame time instead of real time since the game started loading
 - when the owner player joins the game it now attempts to kick a real player instead of the entity in slot 0
 - added automatic detection of desyncs (a warning message will be printed to chat if a desync is detected)
 - fixed a bug where commands with aliases could be executed when they shouldn't be
 - updated some outdated information in this readme
 - many changes and additions to language.cfg

Version 10.4
 - added support for auto starting games
 - added support for auto hosting public games
  * see the "Auto Hosting" section of this readme for more information
 - bot_log now defaults to empty (if it isn't specified no log file will be generated)
 - bot_refreshmessages now defaults to 0
 - games are now refreshed every 5 seconds instead of 10 seconds
 - before creating a game the bot now checks that the currently loaded map config file is valid
 - a timestamp is now printed to the console with ingame chat messages
 - fixed a typo with the map config file for BattleShips Pro 1.189
 - added new config value bot_language to specify the language file
 - added new command !autostart to auto start the current game
 - added new command !autohost to auto host public games
 - you can now use the !announce command in battle.net (it still affects the current game lobby only)
 - added responses to the !announce command
 - added new map config for BattleShips Pro 1.197
 - added new map config for Warlock 083
 - added new map config for Worm War
 - fixed a bug where a reserved player joining the game and kicking someone would cause everyone to disconnect
 - fixed a bug where a player joining over LAN would be kicked when the game filled up when the game had less than 12 slots
 - added some new entries to language.cfg

Version 10.3
 - added support for specifying the config file on the command line (e.g. "ghost.exe mycfg.cfg" or "ghost++ mycfg.cfg")
  * it will default to ghost.cfg if no config file is specified
 - added support for logging console output
 - added new config value bot_log to specify the log file
 - added new config value bot_autolock to automatically lock the game when the owner joins
 - added new config value bnet*_holdfriends to automatically add the bot's friends to the reserved list when creating a game
 - added new config value bnet*_holdclan to automatically add the bot's clan members to the reserved list when creating a game
 - added new command !banlast to ban the last leaver
 - added new command !getclan to refresh the clan members list
 - added new command !getfriends to refresh the friends list
 - added new command !disable to prevent new games from being created
 - added new command !enable to allow new games to be created
 - added new command !saygames to send a chat message to all games
 - fixed a bug where Warcraft 3 would crash when sharing control of units when the map has 12 slots and when connecting via LAN
 - fixed some bugs where long chat messages would be incorrectly truncated
 - fixed a crash bug when there was an error opening the sqlite3 database
 - fixed a potential crash bug when checking game player summaries with a corrupt database
 - fixed a bug where the bot would sometimes not detect a dropped battle.net connection
 - added 3 new entries to language.cfg

Version 10.2
 - added new command !closeall to close all open slots
 - added new command !openall to open all closed slots
 - fixed a minor bug where GHost++ would substitute the creator's name for the owner's name in the "game created" chat message when using !privby and !pubby
 - fixed a bug where GHost++ would sometimes use map_width instead of map_numplayers when loading a map config file

Version 10.1
 - the database schema has been updated to version number 4
  * GHost++ will automatically update your database schema if it's out of date
  * map downloads are now recorded to the database
  * the game creator's name and battle.net server are now recorded to the database
 - fixed some minor chat issues when using multiple realms
 - chat messages about game status are now sent to the game creator rather than the game owner since the owner can be changed and isn't tied to a realm
 - "spoof check accepted" messages are no longer printed when using multiple realms
 - the bot now attempts to spoof check the game owner in order to determine what realm they came from (this is not mandatory - they are still considered spoof checked automatically)
 - duplicate player names are no longer allowed in the same game
 - added new command !announce to send a repeating message to the game lobby
 - fixed a bug that would sometimes result in all players leaving the game at the end of the countdown when starting a melee map with observers
  * as a result of this, you must make sure map_numplayers is correct even in custom maps now

Version 10.01
 - fixed a bug where GHost++ would use an incorrect database schema when creating a new database
  * if you created a database file with Version 10.0 your database schema is incorrect and you should delete your database file and let Version 10.01 create a new one

Version 10.0
 - added support for connecting to multiple realms at the same time
  * see the "Using Multiple Realms" section of this readme for more information
  * added new config value *_rootadmin (e.g. bnet_rootadmin, bnet2_rootadmin, etc...)
  * removed config value db_rootadmin
  * added new config value *_commandtrigger (e.g. bnet_commandtrigger, bnet2_commandtrigger, etc...)
  * renamed config value bnet_hostport to bot_hostport
  * renamed config value bnet_war3path to bot_war3path
  * changed the way admins work slightly to accomodate multiple realms (see the "How Admins Work" section of this readme for more information)
  * changed the way bans work slightly to accomodate multiple realms (see the "How Bans Work" section of this readme for more information)
 - the database schema has been updated to version number 3
  * GHost++ will automatically update your database schema if it's out of date
 - updated iptocountry.csv to the latest version from October 9, 2008
 - added new command !check to check a user's status (leave blank to check your own status)
 - added new command !lock to lock a game and prevent anyone but the game owner from running commands
 - added new command !unlock to unlock a game
 - added new command !owner to set the game owner
 - the !hold command can now be accessed through battle.net
 - the !hold command can now take more than one input
 - the !version command can now be accessed in the lobby and ingame
 - the !stats command can now be accessed ingame
 - the !statsdota command can now be accessed ingame
 - the !addadmin command can now be accessed in the admin game (the realm must be specified unless only one realm is defined)
 - the !checkadmin command can now be accessed in the admin game (the realm must be specified unless only one realm is defined)
 - the !countadmins command can now be accessed in the admin game (the realm must be specified unless only one realm is defined)
 - the !deladmin command can now be accessed in the admin game (the realm must be specified unless only one realm is defined)
 - added new config value bot_commandtrigger for specifying the command trigger ingame
 - added new config value bot_spoofchecks to enable or disable spoof checks
 - added support for conditional map downloads
  * if you set bot_allowdownloads to 2 conditional map downloads will be enabled
  * players without the map will remain at 0% map downloaded until an admin permits them to download the map
  * an admin can use the !download or !dl commands to permit a player to download the map
 - fixed a bug where loading times were not printed after the game loaded
 - fixed the welcome messages to display properly on all screen resolutions
 - the countdown is now aborted if someone joins the game during the countdown
 - GHost++ now checks that your game names are 31 characters or less
 - the game owner checks are now case insensitive (so you don't need to type the owner name with correct capitalization when using !privby and !pubby)
 - included a fix for spoof checking on PVPGN
 - you don't need to have an active battle.net connection to host a LAN game with the admin game anymore
 - the game state (public or private) is now saved to the database
 - download times and speeds are now printed when a map download finishes
 - added 13 new entries to language.cfg and modified many other entries

Version 9.2
 - GHost++ now creates all sockets in non blocking mode
  * this fixes the bug where too many players downloading the map would cause the bot to temporarily freeze up
  * this might improve game performance a little, especially when playing with people on slow connections such as dialup
 - fixed a bug where the bot would sometimes send action packets that were too large which would cause Warcraft 3 to disconnect
  * this bug appeared in games where lots of actions were used (e.g. tower defense maps, Uther Party, even sometimes in DotA)
  * this fixes the bug where the server console would print "connection closed by remote host" for every player
 - added a 15 second timeout when connecting to battle.net rather than waiting indefinitely for the connection to complete
 - the virtual host player now automatically leaves the game when the 12th player joins
  * this might help prevent Warcraft 3 from disconnecting due to there being too many players in the game but it's untested
  * if the player count drops below 12 again the virtual host player will automatically rejoin the game
 - changed the virtual host name to a red "Admin" in the Admin Game
 - when playing DotA the bot no longer kicks everyone before displaying the summary screen
  * the game will remain open until the last player leaves (you can use !end to force the game to end)
  * the bot considers the game time to be the time when the last player left so you won't be considered as playing to 100% when using the !stats command unless you're the last to leave
 - added new config value bot_latency to set the default latency (you can still change this for a particular game with the !latency command)
 - fixed a typo with lang_0039 in language.cfg
 - added one new entry to language.cfg

Version 9.1
 - fixed a crash bug with maps other than DotA

Version 9.0
 - added support for the admin game
  * this allows you to use GHost++ with only one set of CD keys
  * see the "Admin Game" section of this readme for more information
 - added support for new replay data in DotA 6.55
  * this required some changes to the database schema
  * GHost++ will try to determine whether your database schema needs to be upgraded or not when it starts
  * if your database schema is out of data GHost++ will automatically upgrade it
 - completely rewrote game.cpp to allow for admin games
 - did some spring cleaning on some classes to make for cleaner and neater code
 - bot_synclimit defaults to 30 now instead of 15
 - added new config value admingame_create to control whether to create the admin game or not
 - added new config value admingame_port to control the admin game port
 - added new config value admingame_password to control the admin game password
 - fixed a bug where GHost++ sent the map width instead of the map height when broadcasting a game to the local network
 - added 10 new entries to language.cfg
 - modified lang_0093 slightly

Version 8.1
 - updated StormLib to version 6.23
  * backported all my previous modifications to StormLib so it should stil compile and run properly on Linux systems
  * made a few more modifications to StormLib to make my changes cleaner and more understandable
  * as a result of this StormLib once again uses the system copy of zlib under Linux instead of a local copy
  * StormLib should now correctly load the Warlock 083 map
 - finally wrote some code to check all commands for correct input
  * e.g. if you try to run the command "!swap 1 x" or "!swap 1"
  * if bad input is detected a warning will be printed to the server console
 - the !close command can now take more than one input (e.g. "!close 6 7 8")
 - the !open command can now take more than one input (e.g. "!open 6 7 8")
 - the bot now attempts to send the "game created" chat message to the channel *before* creating the game
  * if it can't because of flood protection it won't send the chat message at all
  * this doesn't apply if you whispered the bot since whispers work even when in a game
 - fixed a ton of problems related to observers in melee maps
  * GHost++ now enforces player limits when observers are permitted
  * GHost++ does not enforce player limits with computer slots
 - the !compteam command now uses correct team numbers (previously you had to use team 0 for team 1 and so on)
 - added new config value map_numplayers to control the number of players in melee maps (no effect in custom maps)
 - added new config value map_numteams to control the number of teams in melee maps (no effect in custom maps)
  * in most (all?) cases this should be the same as map_numplayers
 - GHost++ now automatically adds observer slots to maps when observers are permitted (including custom maps)

Version 8.01
 - modified StormLib so that it should compile and run properly on more Linux systems now (including 64 bit systems)
 - GHost++ now uses the StormLib headers from the StormLib directory rather than seperate copies

Version 8.0
 - switched from libmpq to StormLib for MPQ loading
   * libmpq was unable to deal with several map protection schemes and would even segfault (crash) when loading some maps
   * StormLib can handle more map protection schemes (but not all - for example, Warlock 083 doesn't load properly)
 - added support for automatically calculating map_width, map_height, and map_slot<x> (the slot structure)
 - added support for game rehosting via the !priv and !pub commands in the game lobby
   * this doesn't work very well with automatic spoof checking in public games at the moment
 - added support for the lag screen
   * the lag screen is controlled by the sync limit
   * if a player falls behind by more than the sync limit number of packets the lag screen will be started
   * when the player catches up to within half the sync limit number of packets they will be removed from the lag screen
 - added new config value bot_pingduringdownloads to control whether the bot should stop pinging *all* players when at least one player is downloading
 - added new config value bot_refreshmessages to control whether refresh messages are displayed by default (use !refresh to change this for a particular game)
 - added new config value bot_synclimit to control the default sync limit (use !synclimit to change this for a particular game)
 - added new command !drop to drop all lagging players (players listed on the lag screen)
 - added new command !synclimit to change the sync limit for a particular game
 - game descriptions now include the game time in minutes (when using !getgame/!getgames)
 - the !channel command now uses "/join" instead of "/j" so it should work on PVPGN servers now
 - the !exit and !quit commands can now only be accessed by the root admin
 - the !exit and !quit commands will now alert you if there is a game in the lobby or in progress (add force to skip this check)
 - the !latency command now returns the current latency setting if no latency is specified
 - the !sp command now shuffles players only (it leaves open/closed/computer slots in place)
 - added some additional console messages during startup to help identify problems when connecting to PVPGN servers
 - added 10 new entries to language.cfg

Version 7.0
 - optimized loading of the iptocountry data, it should load somewhat faster now
 - added support for automatically calculating map_size, map_info, and map_crc
  * a big thank you to Strilanc for figuring out the map_crc algorithm
  * see the new "Map Config Files" section of this readme for much more information on this
 - GHost++ now requires libmpq
  * libmpq requires zlib
  * libmpq requires libbz2
  * Windows users never fear - I have included all the necessary include files and prebuilt libraries
  * Linux users are in for a bit more effort - you'll need to download and compile libmpq and possibly zlib and libbz2 yourself
  * note that I had to modify libmpq slightly to make it compile with Visual C++ (the modified project is found in the included libmpq-0.4.2-win.zip if you want to compile it yourself)
 - added ms_stdint.h for standard types across all these libraries with Visual C++
  * bncsutil now uses ms_stdint.h when compiling with Visual C++

Version 6.2

- updated SQLite to SQLite 3.6.3
- slightly modified the welcome message sent when a player joins the game
- added new command !from to display the country each player is from
- the iptocountry data is loaded from the file "ip-to-country.csv" on startup
 * a big thank you to tjado for help with the iptocountry feature
 * the included ip-to-country.csv is the September 1, 2008 data from http://ip-to-country.webhosting.info/
 * it may take a few seconds to load the iptocountry data when you start ghost++ (it takes ~10 seconds on my 3.2 GHz P4)
- fixed a bug where config files with very long lines could cause ghost++ to fail
- fixed some crash bugs where ghost++ could crash if the database schema is not what ghost++ expects

Version 6.1

- stats are now displayed for players who didn't spoof check when using !stats and !statsdota
- the bot now automatically broadcasts the game to the local network on port 6112 every 5 seconds when hosting a game
- when joining a game from the LAN screen after receiving a broadcast or a !sendlan packet you should no longer be disconnected after the game fills up
- player leave codes are now sent when a player leaves the game (only "player lost" and "player disconnected" codes)
- fixed a bug with printing ingame chat messages to the console
- fixed a bug where players downloading the map could still be kicked for excessive pings

Version 6.0

- rewrote the internal map handling code
  * map.cfg and all other map configuration files are now stored in another directory
  * you can't load maps while a game is in the lobby anymore
  * unfortunately the configuration files are still necessary because the bot doesn't parse the maps (yet)
- added support for non custom games
  * this is only barely tested so there may be some bugs
  * added support for changing your team/colour/race/handicap
  * if you allow observers/referees the bot doesn't ensure the correct player totals so it's possible to have more players than a map supports
  * if you choose random races the race box isn't greyed out but the bot won't allow you to change your race anyway
- added new config value bot_mapcfgpath
- added new config value map_speed
- added new config value map_visibility
- added new config value map_observers
- added new config value map_flags
- added new config value map_gametype
- removed config value map_custom (it's replaced by map_observers)
- when creating a private game on a PVPGN server the bot stays in the game rather than returning to the chat channel immediately
- the bot now discards pings from players who are downloading the map (since those pings are probably inaccurate)
- ingame chat messages are now printed to the server console (only messages sent to all players, allied and private chat is not printed)
- the bot now checks that everyone has the map before starting the game when using !start (use !start force to skip this check)
- removed some useless messages when using !start
- when playing a map with dota stats player deaths will be printed to the console just for fun
- added UDP socket support as a first step to supporting LAN functions
- added new command !refresh to enable or disable refresh messages (every game starts with refresh messages enabled by default)
- added new command !comp to add a computer player to the game
- added new command !compcolour to change a computer's colour
- added new command !comphandicap to change a computer's handicap
- added new command !comprace to change a computer's race
- added new command !compteam to change a computer's team
- added new command !sendlan to send a fake LAN message to another computer
  * in order to use this the player needs to be running Warcraft 3 and waiting at the LAN screen
  * when you type !sendlan <ip> [port] the game will appear on the player's LAN screen and can be joined directly
  * the player needs to have forwarded the correct port on their router for this to work
  * upon joining the game the bot will reveal that you are using a name spoofer which is more or less true and thus will not be disabled
- added 4 new entries to language.cfg

Version 5.1

- added includes for stdlib.h, string.h, and algorithm to fix some compile issues on some platforms
- GHost++ now pipelines up to 70 KB of map data to the player for even faster map downloads (the theoretical throughput is [70 KB * 1000 / ping] in KB/sec)
- GHost++ no longer autokicks reserved players for excessive pings
- added new command !privby to host a private game by a player other than yourself
- added new command !pubby to host a public game by a player other than yourself
- the !delban and !unban commands no longer work in the game lobby or in the game (they can still be used in battle.net via local chat or by whispering the bot)
- when using !privby and !pubby the specified owner is granted access to the admin commands available in the lobby and in the game but NOT in battle.net unless they're an admin
- when creating a private game the bot now returns to the battle.net chat channel immediately rather than waiting until the game starts
- added support for printing total wins and total losses to the !statsdota command
- modified language.cfg to include total wins and total losses when printing dota stats

Version 5.0 Beta 1

- fixed a potential infinite loop with the language code
- added support for map downloads
- GHost++ does not use map files for anything except sending the raw data to the player so the map config files are still necessary although this may change in the future
- GHost++ pipelines up to 28 KB of map data to the player before waiting for confirmation packets so map downloads should go very quickly if you and the receiving player have fast connections
- GHost++ sends map data to every player that requests it simulataneously (there is no limit of 1 or 2 players at a time like in Warcraft 3)
- the virtual host "GHost" in the lobby will now be created even when the map has 12 slots
 * this is necessary because the bot can't send maps to players without a valid PID and we can't be guaranteed the lobby will have another player (or that Warcraft 3 will accept a forged PID)
 * this is untested and may cause Warcraft 3 to do something weird when a map with 12 slots fills up with real players which is why this is a beta release
- added new config value bot_mappath to specify the directory where GHost++ will look for map files (it tries to open [bot_mappath + map_localpath] when looking for the map)
- added new config value bot_allowdownloads to specify whether to allow map downloads or not
- added new config value map_localpath
- added new command !sp to shuffle all the players in the game lobby
- added new command !muteall to mute global chat in game
- added new command !unmuteall to unmute global chat in game
- you can now access the !stats and !statsdota commands in battle.net via local chat or by whispering the bot (as with !version the bot will only respond if there are few messages already in the chat queue)
- added 3 new entries to language.cfg

Version 4.3

- added a socket timeout function to automatically kick players who stop responding for 30 seconds (this doesn't include truly AFK players as their computer will send keepalive packets)

Version 4.2

- fixed a bug where some players would sometimes not be saved to the database when a DotA game ended
- if you were using version 4 or version 4.1 and you played at least one DotA game your database may be missing some players
- modified the !version command to only respond to non admins when there are few messages already in the chat queue to prevent malicious users from abusing the bot
- the !end command no longer works in the game lobby - it's intended to be used to end a currently running game (use !unhost to end a game in the lobby)
- you can now access the !stats and !statsdota commands more than once per game but only once every 5 seconds to prevent spamming
- you can now add a player name after the !stats and !statsdota commands to get statistics about another player (leave it blank to get your own statistics)

Version 4.1

- fixed a crash bug when using !stats and !statsdota
- two minor cosmetic changes to language.cfg

Version 4

- updated SQLite to SQLite 3.6.1
- included the SQLite source code to help eliminate some linking errors, no more SQLite shared objects required on Linux
- added languages support (console messages are still hardcoded in english)
- included an english language.cfg
- GHost++ now sends pings during map loading to help prevent players from timing out during map loading
- admins are now automatically treated as reserved players when joining a game
- the !version command now sends the version number to admins
- the !version command can now be accessed by non admins, however it does not send the version number to non admins (this can be changed in language.cfg)
- added support for basic game/player statistics such as total games played (these stats are collected no matter which map is played)
- added support for map specific game/player statistics (these stats are collected only on some maps)
- included a DotA statistics class to record DotA specific statistics using the new real time replay data in dota 6.54
- added new config value map_type to specify which statistics class handles the map statistics (currently "dota" is the only supported type, set it to anything else to disable)
- added new command !stats which any player can access which displays basic player statistics
- added new command !statsdota which any player can access which displays DotA player statistics
- added new command !end
- added some more console messages to help with troubleshooting (some of the old error messages were pretty vague)
- a console message will now be printed when a player is kicked because they don't have the map

Version 3

- ghost now works on systems with 64 bit processors
- if you have a 64 bit processor and you want to compile ghost yourself you will need to recompile bncsutil because one of the 64 bit fixes was in bncsutil
- ghost now works with maps other than DotA
- the default map is now DotA 6.54b if no map.cfg file is found
- added new config value map_custom
- added new config value map_width to allow for maps other than DotA
- added new config value map_height to allow for maps other than DotA
- added new config values map_slot<x> where x is a number from 1 to 12 to allow for maps other than DotA
- added new command !load and !map
- added new command !a (as an alias to !abort)
- removed command !privobs (observers are now specified in the map config file)
- removed command !pubobs (observers are now specified in the map config file)
- added some more console messages to help understand what the bot is doing
- fixed a bug where changing your team or slot wouldn't work properly for maps other than DotA
- fixed a bug where maps with more or less than 10 player (non observer) slots wouldn't load
- fixed a bug where an extra comma would sometimes be added to the ping text when using !ping
- the config files are now in <LF> format instead of <CR/LF>
- included a DotA 6.54b map file
- included a DotA 6.54b with observers map file
- included a Warlock 081b map file

Version 2

- the makefile now creates a "ghost++" binary instead of "ghost" so you can copy it to the parent directory without any trouble
- ghost now tries to shutdown properly when you press Ctrl-C, press it again to force it to exit immediately
- changed config key from bot_rootadmin to db_rootadmin
- added new config key bnet_custom_war3version (set it to 21 for version 1.21 or 22 for version 1.22)
- added SQLite 3 database support
- added new command !addadmin
- added new command !checkadmin
- added new command !countadmins
- added new command !deladmin
- added new command !addban and !ban
- added new command !checkban
- added new command !countbans
- added new command !delban and !unban
- fixed a bug where a player crashing during loading could prevent the game from starting
- fixed a bug where a reserved player joining a full game would take another player's slot but the previous player wouldn't be kicked
- fixed a bug where the owner player joining a full game with no reserved slots available would cause the blue player to be kicked but the owner would still not be allowed to join the game
- assorted minor bug fixes

Version 1

- initial release

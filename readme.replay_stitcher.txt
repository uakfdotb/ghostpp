===================================
GHost++ Replay Stitcher Version 1.0
===================================

The GHost++ Replay Stitcher is a Warcraft 3 "packed file" utility.
Packed files include Warcraft 3 replays and Warcraft 3 saved games (among others).
The main purpose of the replay stitcher is to "stitch" together replays from before and after a saved game.
You can also use it to decompress packed files, print information about them, and change their version and build number.

============================
Replay Stitching (-s option)
============================

The stitching function is used to stitch together replays from before and after a saved game.
This means you must provide the program with a list of source replays saved from consecutive sessions of the same game.
But Warcraft 3 does NOT save replays from a new session of the same game, so how do you get any subsequent replays?
GHost++ supports saving partial replays when hosting from a saved game, so you must use GHost++ to host subsequent sessions.
Note: The replay stitcher has not been tested with an initial replay saved by Warcraft 3 and it may not work properly in this case.
It has only been tested exclusively with replays saved by GHost++ therefore it's recommended that you use GHost++ for all sessions.
When you have your consecutive replays, run the program like in the following example:

D:\svn\ghost++v3>replay_stitcher -s "GHost++ 2009-09-13 15-39 testtest2 (02m10s).w3g" "Partial GHost++ 2009-09-13 15-46 testtest3 (02m02s).w3g" stitched.w3g
[PACKED] loading data from file [GHost++ 2009-09-13 15-39 testtest2 (02m10s).w3g]
[PACKED] decompressing data
[PACKED] reading 3 blocks
[PACKED] decompressed 24576 bytes
[PACKED] discarding 5190 bytes
[PACKED] loading data from file [Partial GHost++ 2009-09-13 15-46 testtest3 (02m02s).w3g]
[PACKED] decompressing data
[PACKED] reading 3 blocks
[PACKED] decompressed 24576 bytes
[PACKED] discarding 5800 bytes
stitching replay...
===> Varlock saved the game at 01m57s, is this where the next replay started (0 remain)? [y/n] y
stitching replay...
Varlock left the game at 04m00s.
Strilanc left the game at 04m02s.
[REPLAY] building replay
[PACKED] compressing data
[PACKED] saving data to file [stitched.w3g]

The program will first load each of the source replays, then it will begin the stitching process.
If it encounters any mismatches between the replays it will print an error and exit.
Every time a player saved the game in any of the replays a message will be printed like so:

===> Varlock saved the game at 01m57s, is this where the next replay started (0 remain)? [y/n]

The "0 remain" part tells you how many more times the game was saved in the current replay.
If it says "0 remain" and you answer "n" to the prompt the program will exit because subsequent replays must start at a save point.
If you answer "y" to the prompt the program will begin searching the next consecutive replay for save points.
When it gets to the last replay all the remaining data will be added to the final replay regardless of further save points.
Assuming everything went well the final replay should be a complete and viewable replay of the entire game.

=========================================
Changing Packed File Versions (-v option)
=========================================

The version function is used to change versions and build numbers of packed files.
For example, you can use this to change the version of a Warcraft 3 replay.
Note: You *cannot* use this function to change replay versions between major game versions (e.g. Warcraft 3 patch 1.23 -> 1.24)
It is only useful for fixing incorrectly saved replays.
Example:

D:\svn\ghost++v3>replay_stitcher -v 24 6059 stitched.w3g
[PACKED] loading data from file [stitched.w3g]
[PACKED] decompressing data
[PACKED] reading 5 blocks
[PACKED] decompressed 40960 bytes
[PACKED] discarding 4500 bytes
changing version to 24 and build number to 6059
[PACKED] compressing data
[PACKED] saving data to file [stitched.w3g]

=========
Changelog
=========

Version 1.0
 - initial release

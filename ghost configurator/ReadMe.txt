License:
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Copyright 2009 Benjamin J. Schmid 

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at 

	http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Installation:
Place this application in your ghost++ directory.

Requirements:
-Windows XP and above
-dotNet framework 3.5

Info:
release: 16.4.2010
auto: HardwareBug
version: 1.2b
written with: Virtual Basic 2008 Express Edition

Programmer info:
At first opening of the configurator project you may get an error when opening the SettingsDialog form,
to solve this error just start the debugging, after that all will be fine.

Change log:
16.4.2010 - v1.2b
-fix a bug where the configurator wouldn't save the bnet#_server if it's value was the default server (useast.battle.net)

11.4.2010 - v1.2a
-if only a default.cfg is aviable the SettingsDialog will choose the create new ghost.cfg option
-for the create new ghost.cfg option no ghost.cfg file path is needed
-fixed some small bugs

5.4.2010 - v1.2
-added support for 2 cfg files (ghost.cfg & default.cfg)
-added settings dialog with syntax checks
-added a button (in the settings dialog) to delete the configurator settings
-added a shortcut to reset the selected value (ctrl + z)
-added a shortcut to get the selected value from (bnet) server 1 tab (ctrl + shift)
-the ghost.exe path is now customizable
-removed the cfg updater tab (it isn't needed any more)

27.9.2009 - v1.05
-the ghost.cfg path is now customizable
-added a button to remove a bnet server
-added checkboxes to "run ghost after closing" or "don't save settings on exit"
-if the settings can not be saved to the cfg file the settings will be saved to a backup file
-fixed some small bugs

19.9.2009 - v1.0 
-initial release
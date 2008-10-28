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

#ifndef CONFIG_H
#define CONFIG_H

extern map<string, string> gCFG;

void CFG_Read( map<string, string> &CFG, string file );
void CFG_SetInt( map<string, string> &CFG, string key, int x );
void CFG_SetString( map<string, string> &CFG, string key, string x );
int CFG_GetInt( map<string, string> &CFG, string key, int x );
string CFG_GetString( map<string, string> &CFG, string key, string x );
void CFG_Delete( map<string, string> &CFG, string key );
void CFG_Write( map<string, string> &CFG, string file );
void CFG_Clear( map<string, string> &CFG );

void CFG_Read( string file );
void CFG_SetInt( string key, int x );
void CFG_SetString( string key, string x );
int CFG_GetInt( string key, int x );
string CFG_GetString( string key, string x );
void CFG_Delete( string key );
void CFG_Write( string file );
void CFG_Clear( );

#endif

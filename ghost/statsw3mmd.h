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

#ifndef STATSW3MMD_H
#define STATSW3MMD_H

//
// CStatsW3MMD
//

typedef pair<uint32_t,string> VarP;

class CStatsW3MMD : public CStats
{
private:
	string m_Category;
	uint32_t m_NextValueID;
	uint32_t m_NextCheckID;
	map<uint32_t,string> m_PIDToName;			// pid -> player name (e.g. 0 -> "Varlock") --- note: will not be automatically converted to lower case
	map<uint32_t,string> m_Flags;				// pid -> flag (e.g. 0 -> "winner")
	map<uint32_t,bool> m_FlagsLeaver;			// pid -> leaver flag (e.g. 0 -> true) --- note: will only be present if true
	map<uint32_t,bool> m_FlagsPracticing;		// pid -> practice flag (e.g. 0 -> true) --- note: will only be present if true
	map<string,string> m_DefVarPs;				// varname -> value type (e.g. "kills" -> "int")
	map<VarP,int32_t> m_VarPInts;				// pid,varname -> value (e.g. 0,"kills" -> 5)
	map<VarP,double> m_VarPReals;				// pid,varname -> value (e.g. 0,"x" -> 0.8)
	map<VarP,string> m_VarPStrings;				// pid,varname -> value (e.g. 0,"hero" -> "heroname")
	map<string, vector<string> > m_DefEvents;	// event -> vector of arguments + format

public:
	CStatsW3MMD( CBaseGame *nGame, string nCategory );
	virtual ~CStatsW3MMD( );

	virtual bool ProcessAction( CIncomingAction *Action );
	virtual void Save( CGHost *GHost, CGHostDB *DB, uint32_t GameID );
	virtual vector<string> TokenizeKey( string key );
};

#endif

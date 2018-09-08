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

#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "gameslot.h"

//
// CSaveGame
//

class CSaveGame : public CPacked
{
private:
	std::string m_FileName;
	std::string m_FileNameNoPath;
	std::string m_MapPath;
	std::string m_GameName;
	unsigned char m_NumSlots;
	std::vector<CGameSlot> m_Slots;
	uint32_t m_RandomSeed;
	BYTEARRAY m_MagicNumber;

public:
	CSaveGame( );
	virtual ~CSaveGame( );

	std::string GetFileName( )				{ return m_FileName; }
	std::string GetFileNameNoPath( )			{ return m_FileNameNoPath; }
	std::string GetMapPath( )				{ return m_MapPath; }
	std::string GetGameName( )				{ return m_GameName; }
	unsigned char GetNumSlots( )		{ return m_NumSlots; }
	std::vector<CGameSlot> GetSlots( )		{ return m_Slots; }
	uint32_t GetRandomSeed( )			{ return m_RandomSeed; }
	BYTEARRAY GetMagicNumber( )			{ return m_MagicNumber; }

	void SetFileName( std::string nFileName )				{ m_FileName = nFileName; }
	void SetFileNameNoPath( std::string nFileNameNoPath )	{ m_FileNameNoPath = nFileNameNoPath; }

	void ParseSaveGame( );
};

#endif

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

#ifndef REPLAY_H
#define REPLAY_H

#include "gameslot.h"

//
// CReplay
//

typedef pair<unsigned char,string> ReplayPlayer;

class CIncomingAction;

class CReplay : public CPacked
{
public:
	enum BlockID {
		REPLAY_LEAVEGAME		= 0x17,
		REPLAY_FIRSTSTARTBLOCK	= 0x1A,
		REPLAY_SECONDSTARTBLOCK	= 0x1B,
		REPLAY_THIRDSTARTBLOCK	= 0x1C,
		REPLAY_TIMESLOT			= 0x1F,
		REPLAY_CHATMESSAGE		= 0x20,
		REPLAY_CHECKSUM			= 0x22
	};

private:
	vector<ReplayPlayer> m_Players;
	vector<CGameSlot> m_Slots;
	queue<BYTEARRAY> m_Blocks;
	queue<uint32_t> m_CheckSums;
	uint32_t m_RandomSeed;
	unsigned char m_SelectMode;
	unsigned char m_StartSpotCount;
	unsigned char m_HostPID;
	string m_HostName;

public:
	CReplay( CGHost *nGHost );
	virtual ~CReplay( );

	void AddPlayer( unsigned char nPID, string nName )		{ m_Players.push_back( ReplayPlayer( nPID, nName ) ); }
	void SetSlots( vector<CGameSlot> nSlots )				{ m_Slots = nSlots; }
	void SetRandomSeed( uint32_t nRandomSeed )				{ m_RandomSeed = nRandomSeed; }
	void SetSelectMode( unsigned char nSelectMode )			{ m_SelectMode = nSelectMode; }
	void SetStartSpotCount( unsigned char nStartSpotCount )	{ m_StartSpotCount = nStartSpotCount; }
	void SetHostPID( unsigned char nHostPID )				{ m_HostPID = nHostPID; }
	void SetHostName( string nHostName )					{ m_HostName = nHostName; }

	void AddLeaveGame( uint32_t reason, unsigned char PID, uint32_t result );
	void AddTimeSlot( uint16_t timeIncrement, queue<CIncomingAction *> actions );
	void AddChatMessage( unsigned char PID, unsigned char flags, uint32_t chatMode, string message );
	void AddCheckSum( uint32_t checkSum );
	void BuildReplay( string gameName, string statString );
};

#endif

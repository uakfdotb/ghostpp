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

#ifndef BNLSCLIENT_H
#define BNLSCLIENT_H

//
// CBNLSClient
//

class CTCPClient;
class CBNLSProtocol;
class CCommandPacket;

class CBNLSClient
{
private:
	CTCPClient *m_Socket;							// the connection to the BNLS server
	CBNLSProtocol *m_Protocol;						// battle.net protocol
	queue<CCommandPacket *> m_Packets;				// queue of incoming packets
	bool m_WasConnected;
	string m_Server;
	uint16_t m_Port;
	uint32_t m_LastNullTime;
	uint32_t m_WardenCookie;						// the warden cookie
	queue<BYTEARRAY> m_OutPackets;					// queue of outgoing packets to be sent
	queue<BYTEARRAY> m_WardenResponses;				// the warden responses to be sent to battle.net
	uint32_t m_TotalWardenIn;
	uint32_t m_TotalWardenOut;

public:
	CBNLSClient( string nServer, uint16_t nPort, uint32_t nWardenCookie );
	~CBNLSClient( );

	BYTEARRAY GetWardenResponse( );
	uint32_t GetTotalWardenIn( )		{ return m_TotalWardenIn; }
	uint32_t GetTotalWardenOut( )		{ return m_TotalWardenOut; }

	// processing functions

	unsigned int SetFD( void *fd, void *send_fd, int *nfds );
	bool Update( void *fd, void *send_fd );
	void ExtractPackets( );
	void ProcessPackets( );

	// other functions

	void QueueWardenSeed( uint32_t seed );
	void QueueWardenRaw( BYTEARRAY wardenRaw );
};

#endif

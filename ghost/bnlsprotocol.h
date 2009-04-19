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

#ifndef BNLSPROTOCOL_H
#define BNLSPROTOCOL_H

//
// CBNLSProtocol
//

class CBNLSProtocol
{
public:
	enum Protocol {
		BNLS_NULL					= 0x00,
		BNLS_CDKEY					= 0x01,
		BNLS_LOGONCHALLENGE			= 0x02,
		BNLS_LOGONPROOF				= 0x03,
		BNLS_CREATEACCOUNT			= 0x04,
		BNLS_CHANGECHALLENGE		= 0x05,
		BNLS_CHANGEPROOF			= 0x06,
		BNLS_UPGRADECHALLENGE		= 0x07,
		BNLS_UPGRADEPROOF			= 0x08,
		BNLS_VERSIONCHECK			= 0x09,
		BNLS_CONFIRMLOGON			= 0x0a,
		BNLS_HASHDATA				= 0x0b,
		BNLS_CDKEY_EX				= 0x0c,
		BNLS_CHOOSENLSREVISION		= 0x0d,
		BNLS_AUTHORIZE				= 0x0e,
		BNLS_AUTHORIZEPROOF			= 0x0f,
		BNLS_REQUESTVERSIONBYTE		= 0x10,
		BNLS_VERIFYSERVER			= 0x11,
		BNLS_RESERVESERVERSLOTS		= 0x12,
		BNLS_SERVERLOGONCHALLENGE	= 0x13,
		BNLS_SERVERLOGONPROOF		= 0x14,
		BNLS_RESERVED0				= 0x15,
		BNLS_RESERVED1				= 0x16,
		BNLS_RESERVED2				= 0x17,
		BNLS_VERSIONCHECKEX			= 0x18,
		BNLS_RESERVED3				= 0x19,
		BNLS_VERSIONCHECKEX2		= 0x1a,
		BNLS_WARDEN					= 0x7d
	};

public:
	CBNLSProtocol( );
	~CBNLSProtocol( );

	// receive functions

	BYTEARRAY RECEIVE_BNLS_WARDEN( BYTEARRAY data );

	// send functions

	BYTEARRAY SEND_BNLS_NULL( );
	BYTEARRAY SEND_BNLS_WARDEN_SEED( uint32_t cookie, uint32_t seed );
	BYTEARRAY SEND_BNLS_WARDEN_RAW( uint32_t cookie, BYTEARRAY raw );
	BYTEARRAY SEND_BNLS_WARDEN_RUNMODULE( uint32_t cookie );

	// other functions

private:
	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );
};

#endif

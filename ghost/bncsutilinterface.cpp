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

#include "ghost.h"
#include "util.h"
#include "bncsutilinterface.h"

#include <bncsutil/bncsutil.h>

//
// CBNCSUtilInterface
//

CBNCSUtilInterface :: CBNCSUtilInterface( std::string userName, std::string userPassword )
{
	// m_nls = (void *)nls_init( userName.c_str( ), userPassword.c_str( ) );
	m_NLS = new NLS( userName, userPassword );
}

CBNCSUtilInterface :: ~CBNCSUtilInterface( )
{
	// nls_free( (nls_t *)m_nls );
	delete (NLS *)m_NLS;
}

void CBNCSUtilInterface :: Reset( std::string userName, std::string userPassword )
{
	// nls_free( (nls_t *)m_nls );
	// m_nls = (void *)nls_init( userName.c_str( ), userPassword.c_str( ) );
	delete (NLS *)m_NLS;
	m_NLS = new NLS( userName, userPassword );
}

bool CBNCSUtilInterface :: HELP_SID_AUTH_CHECK( bool TFT, uint32_t war3Version, std::string war3Path, std::string keyROC, std::string keyTFT, std::string valueStringFormula, std::string mpqFileName, BYTEARRAY clientToken, BYTEARRAY serverToken )
{
	// set m_EXEVersion, m_EXEVersionHash, m_EXEInfo, m_InfoROC, m_InfoTFT

	std::string FileWar3EXE = war3Path + "Warcraft III.exe";

	if( !UTIL_FileExists( FileWar3EXE ) )
		FileWar3EXE = war3Path + "warcraft.exe";

	bool MissingFile = false;

	if( !UTIL_FileExists( FileWar3EXE ) )
	{
		CONSOLE_Print( "[BNCSUI] unable to open [" + FileWar3EXE + "]" );
		MissingFile = true;
	}

	std::string FileStormDLL, FileGameDLL;

	if( war3Version <= 28 )
	{
		FileStormDLL = war3Path + "Storm.dll";

		if( !UTIL_FileExists( FileStormDLL ) )
			FileStormDLL = war3Path + "storm.dll";

		FileGameDLL = war3Path + "game.dll";

		if( !UTIL_FileExists( FileStormDLL ) )
		{
			CONSOLE_Print( "[BNCSUI] unable to open [" + FileStormDLL + "]" );
			MissingFile = true;
		}

		if( !UTIL_FileExists( FileGameDLL ) )
		{
			CONSOLE_Print( "[BNCSUI] unable to open [" + FileGameDLL + "]" );
			MissingFile = true;
		}
	}

	if( MissingFile )
		return false;

	// todotodo: check getExeInfo return value to ensure 1024 bytes was enough

	char buf[1024];
	uint32_t EXEVersion;
	getExeInfo( FileWar3EXE.c_str( ), (char *)&buf, 1024, (uint32_t *)&EXEVersion, BNCSUTIL_PLATFORM_X86 );
	m_EXEInfo = buf;
	m_EXEVersion = UTIL_CreateByteArray( EXEVersion, false );
	unsigned long EXEVersionHash;

	// for war3version <= 28, we use war3.exe, storm.dll, and game.dll
	// for war3version == 29, we use Warcraft III.exe only
	if( war3Version <= 28 )
	{
		checkRevisionFlat( valueStringFormula.c_str( ), FileWar3EXE.c_str( ), FileStormDLL.c_str( ), FileGameDLL.c_str( ), extractMPQNumber( mpqFileName.c_str( ) ), (unsigned long *)&EXEVersionHash );
	}
	else
	{
		const char* files[] = { FileWar3EXE.c_str( ) };
		checkRevision( valueStringFormula.c_str( ), files, 1, extractMPQNumber( mpqFileName.c_str( ) ), (unsigned long *)&EXEVersionHash );
	}

	m_EXEVersionHash = UTIL_CreateByteArray( (uint32_t) EXEVersionHash, false );
	m_KeyInfoROC = CreateKeyInfo( keyROC, UTIL_ByteArrayToUInt32( clientToken, false ), UTIL_ByteArrayToUInt32( serverToken, false ) );

	if( TFT )
		m_KeyInfoTFT = CreateKeyInfo( keyTFT, UTIL_ByteArrayToUInt32( clientToken, false ), UTIL_ByteArrayToUInt32( serverToken, false ) );

	if( m_KeyInfoROC.size( ) == 36 && ( !TFT || m_KeyInfoTFT.size( ) == 36 ) )
		return true;
	else
	{
		if( m_KeyInfoROC.size( ) != 36 )
			CONSOLE_Print( "[BNCSUI] unable to create ROC key info - invalid ROC key" );

		if( TFT && m_KeyInfoTFT.size( ) != 36 )
			CONSOLE_Print( "[BNCSUI] unable to create TFT key info - invalid TFT key" );
	}

	return false;
}

bool CBNCSUtilInterface :: HELP_SID_AUTH_ACCOUNTLOGON( )
{
	// set m_ClientKey

	char buf[32];
	// nls_get_A( (nls_t *)m_nls, buf );
	( (NLS *)m_NLS )->getPublicKey( buf );
	m_ClientKey = UTIL_CreateByteArray( (unsigned char *)buf, 32 );
	return true;
}

bool CBNCSUtilInterface :: HELP_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY salt, BYTEARRAY serverKey )
{
	// set m_M1

	char buf[20];
	// nls_get_M1( (nls_t *)m_nls, buf, std::string( serverKey.begin( ), serverKey.end( ) ).c_str( ), std::string( salt.begin( ), salt.end( ) ).c_str( ) );
	( (NLS *)m_NLS )->getClientSessionKey( buf, std::string( salt.begin( ), salt.end( ) ).c_str( ), std::string( serverKey.begin( ), serverKey.end( ) ).c_str( ) );
	m_M1 = UTIL_CreateByteArray( (unsigned char *)buf, 20 );
	return true;
}

bool CBNCSUtilInterface :: HELP_PvPGNPasswordHash( std::string userPassword )
{
	// set m_PvPGNPasswordHash

	char buf[20];
	hashPassword( userPassword.c_str( ), buf );
	m_PvPGNPasswordHash = UTIL_CreateByteArray( (unsigned char *)buf, 20 );
	return true;
}

BYTEARRAY CBNCSUtilInterface :: CreateKeyInfo( std::string key, uint32_t clientToken, uint32_t serverToken )
{
	unsigned char Zeros[] = { 0, 0, 0, 0 };
	BYTEARRAY KeyInfo;
	CDKeyDecoder Decoder( key.c_str( ), key.size( ) );

	if( Decoder.isKeyValid( ) )
	{
		UTIL_AppendByteArray( KeyInfo, UTIL_CreateByteArray( (uint32_t)key.size( ), false ) );
		UTIL_AppendByteArray( KeyInfo, UTIL_CreateByteArray( Decoder.getProduct( ), false ) );
		UTIL_AppendByteArray( KeyInfo, UTIL_CreateByteArray( Decoder.getVal1( ), false ) );
		UTIL_AppendByteArray( KeyInfo, UTIL_CreateByteArray( Zeros, 4 ) );
		size_t Length = Decoder.calculateHash( clientToken, serverToken );
		char *buf = new char[Length];
		Length = Decoder.getHash( buf );
		UTIL_AppendByteArray( KeyInfo, UTIL_CreateByteArray( (unsigned char *)buf, Length ) );
		delete [] buf;
	}

	return KeyInfo;
}

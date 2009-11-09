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

#ifndef PACKED_H
#define PACKED_H

//
// CPacked
//

class CCRC32;

class CPacked
{
public:
	CCRC32 *m_CRC;

protected:
	bool m_Valid;
	string m_Compressed;
	string m_Decompressed;
	uint32_t m_HeaderSize;
	uint32_t m_CompressedSize;
	uint32_t m_HeaderVersion;
	uint32_t m_DecompressedSize;
	uint32_t m_NumBlocks;
	uint32_t m_War3Identifier;
	uint32_t m_War3Version;
	uint16_t m_BuildNumber;
	uint16_t m_Flags;
	uint32_t m_ReplayLength;

public:
	CPacked( );
	virtual ~CPacked( );

	virtual bool GetValid( )				{ return m_Valid; }
	virtual uint32_t GetHeaderSize( )		{ return m_HeaderSize; }
	virtual uint32_t GetCompressedSize( )	{ return m_CompressedSize; }
	virtual uint32_t GetHeaderVersion( )	{ return m_HeaderVersion; }
	virtual uint32_t GetDecompressedSize( )	{ return m_DecompressedSize; }
	virtual uint32_t GetNumBlocks( )		{ return m_NumBlocks; }
	virtual uint32_t GetWar3Identifier( )	{ return m_War3Identifier; }
	virtual uint32_t GetWar3Version( )		{ return m_War3Version; }
	virtual uint16_t GetBuildNumber( )		{ return m_BuildNumber; }
	virtual uint16_t GetFlags( )			{ return m_Flags; }
	virtual uint32_t GetReplayLength( )		{ return m_ReplayLength; }

	virtual void SetWar3Version( uint32_t nWar3Version )			{ m_War3Version = nWar3Version; }
	virtual void SetBuildNumber( uint16_t nBuildNumber )			{ m_BuildNumber = nBuildNumber; }
	virtual void SetFlags( uint16_t nFlags )						{ m_Flags = nFlags; }
	virtual void SetReplayLength( uint32_t nReplayLength )			{ m_ReplayLength = nReplayLength; }

	virtual void Load( string fileName, bool allBlocks );
	virtual bool Save( bool TFT, string fileName );
	virtual bool Extract( string inFileName, string outFileName );
	virtual bool Pack( bool TFT, string inFileName, string outFileName );
	virtual void Decompress( bool allBlocks );
	virtual void Compress( bool TFT );
};

#endif

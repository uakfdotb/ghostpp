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

//
// CConfig
//

class CConfig
{
private:
	std::map<std::string, std::string> m_CFG;

public:
	CConfig( );
	~CConfig( );

	void Read( std::string file );
	bool Exists( std::string key );
	int GetInt( std::string key, int x );
	uint32_t GetUInt32( std::string key, uint32_t x );
	std::string GetString( std::string key, std::string x );
	void Set( std::string key, std::string x );
};

#endif

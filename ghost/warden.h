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

#ifndef WARDEN_H
#define WARDEN_H

//
// CWardenSHA1
//

typedef struct
{
	int bitlen[2];
	int state[32];
} WARDENSHA1_CTX;

class CWardenSHA1
{
public:
	CWardenSHA1( );
	~CWardenSHA1( );

	void reverse_endian( int *val, int *buffer );
	void warden_sha1_tweedle( int *ptr_rotator, int bitwise, int bitwise2, int bitwise3, int *ptr_adder, int *ptr_ret );
	void warden_sha1_twitter( int *ptr_rotator, int bitwise, int rotator2, int bitwise2, int *ptr_rotator3, int *ptr_ret );
	void warden_sha1_hash( int buffer[5], unsigned char *data, int length );
	void warden_sha1_init( WARDENSHA1_CTX *ctx );
	void warden_sha1_update( WARDENSHA1_CTX *ctx, char *data, int len );
	void warden_sha1_final( WARDENSHA1_CTX *ctx, int *hash );
	void warden_sha1_transform( int *data, int *state );
};

//
// CWarden
//

typedef struct
{
	int current_position;
	unsigned char random_data[0x14];
	unsigned char random_source_1[0x14];
	unsigned char random_source_2[0x14];
} t_random_data;

class CWarden
{
public:
	CGHost *m_GHost;

private:
	CWardenSHA1 *m_SHA1;
	uint32_t m_Seed;
	t_random_data m_RandomData;
	unsigned char m_KeyOut[258];
	unsigned char m_KeyIn[258];

public:
	CWarden( CGHost *nGHost, uint32_t nSeed );
	~CWarden( );

	// randomdata

	void random_data_update( );
	void random_data_initialize( char *seed, int length );
	unsigned char random_data_get_byte( );
	void random_data_get_bytes( char *buffer, int bytes );

	// rc4

	void generate_key( unsigned char *key_buffer, unsigned char *base, unsigned int base_length );
	void do_crypt( unsigned char *key, unsigned char *data, int length );

	// warden

	void HandleWarden( BYTEARRAY packet );
	void HandleWarden0x00( unsigned char *data, uint32_t length );
	void HandleWarden0x01( unsigned char *data, uint32_t length );
	void HandleWarden0x02( unsigned char *data, uint32_t length );
	void HandleWarden0x03( unsigned char *data, uint32_t length );
	void HandleWarden0x04( unsigned char *data, uint32_t length );
	void HandleWarden0x05( unsigned char *data, uint32_t length );
};

#endif

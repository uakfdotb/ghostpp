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
#include "warden.h"

//
// CWardenSHA1
//

#define RotateLeft32(value, bits) (int)(((uint32_t)(value)<<(bits))|((uint32_t)(value)>>(32-(bits))))

CWardenSHA1 :: CWardenSHA1( )
{

}

CWardenSHA1 :: ~CWardenSHA1( )
{

}

void CWardenSHA1 :: reverse_endian( int *val, int *buffer )
{
	*buffer = ((*val & 0x000000FF) << 24) | ((*val & 0x0000FF00) << 8) | ((*val & 0x00FF0000) >> 8) | ((*val & 0xFF000000) >> 24);
}

void CWardenSHA1 :: warden_sha1_tweedle( int *ptr_rotator, int bitwise, int bitwise2, int bitwise3, int *ptr_adder, int *ptr_ret )
{
	*ptr_ret = *ptr_ret + (((RotateLeft32(bitwise3, 5)) + ( (~(*ptr_rotator) & bitwise2) | (*ptr_rotator & bitwise))) + *ptr_adder + 0x5A827999);
	*ptr_adder = 0;
	*ptr_rotator = RotateLeft32(*ptr_rotator, 0x1e);
}

void CWardenSHA1 :: warden_sha1_twitter( int *ptr_rotator, int bitwise, int rotator2, int bitwise2, int *ptr_rotator3, int *ptr_ret )
{
	*ptr_ret = *ptr_ret + ((((bitwise2 | bitwise) & *(ptr_rotator)) | (bitwise2 & bitwise)) + ((RotateLeft32(rotator2, 5)) + *ptr_rotator3) - 0x70e44324);
	*ptr_rotator3 = 0;
	*ptr_rotator = RotateLeft32(*ptr_rotator, 0x1e);
}

void CWardenSHA1 :: warden_sha1_hash( int buffer[5], unsigned char *data, int length )
{
	WARDENSHA1_CTX ctx;
	warden_sha1_init( &ctx );
	warden_sha1_update( &ctx, (char *)data, length );
	warden_sha1_final( &ctx, buffer );
}

void CWardenSHA1 :: warden_sha1_init( WARDENSHA1_CTX *ctx )
{
	ctx->bitlen[0] = 0;
	ctx->bitlen[1] = 0;
	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
	ctx->state[4] = 0xC3D2E1F0;
}

void CWardenSHA1 :: warden_sha1_update( WARDENSHA1_CTX *ctx, char *data, int len )
{
	int *bitlen = ctx->bitlen;
	char *state = (char *) ctx->state;
	int a;
	int b;
	int c;
	int i;

	/** This is a hack because this function doesn't work with 64-byte strings or longer.
	 * So just split up those strings. */
	if(len >= 0x40) /* CHANGED */
	{ /* CHANGED */
		for(i = 0; i < len; i += 0x3F) /* CHANGED */
		{ /* CHANGED */
			warden_sha1_update(ctx, data + i, min(0x3F, len - i)); /* CHANGED */
		} /* CHANGED */
	} /* CHANGED */
	else /* CHANGED */
	{ /* CHANGED */
		/* The next two lines multiply len by 8. */
		c = len >> 29;
		b = len << 3;

		a = (bitlen[0] / 8) & 0x3F;

		/* Check for overflow. */
		if(bitlen[0] + b < bitlen[0] || bitlen[0] + b < b)
			bitlen[1]++;
		bitlen[0] = bitlen[0] + b;
		bitlen[1] = bitlen[1] + c;

		len = len + a;
		data = data - a;

		if(len >= 0x40)
		{
			if(a)
			{
				while(a < 0x40)
				{
					state[0x14 + a] = data[a];
					a++;
				}

				warden_sha1_transform((int *) (state + 0x14), (int *) state);
				len -= 0x40;
				data += 0x40;
				a = 0;
			}

			if(len >= 0x40)
			{
				b = len;
				for(i = 0; i < b / 0x40; i++)
				{
					warden_sha1_transform((int *) data, (int *) state);
					len -= 0x40;
					data += 0x40;
				}
			}
		}

		for(; a < len; a++)
			state[a + 0x1c - 8] = data[a];
	} /* CHANGED */
}

void CWardenSHA1 :: warden_sha1_final( WARDENSHA1_CTX *ctx, int *hash )
{
	int i;
	int vars[2]; /* CHANGED */
	char *MysteryBuffer = "\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	int len; /* CHANGED */

	reverse_endian(&ctx->bitlen[1], &vars[0]); /* CHANGED */
	reverse_endian(&ctx->bitlen[0], &vars[1]); /* CHANGED */

	len = ((-9 - (ctx->bitlen[0] >> 3)) & 0x3F) + 1; /* CHANGED */
	warden_sha1_update(ctx, MysteryBuffer, len);
	warden_sha1_update(ctx, (char *)vars, 8);

	for(i = 0; i < 5; i++)
		reverse_endian(&ctx->state[i], &hash[i]); /* CHANGED */
}

void CWardenSHA1 :: warden_sha1_transform( int *data, int *state )
{
	int a, b, c, d, e, f, g, h, m, n;
	int i;

	int buf[80];

	for(i = 0; i < 0x10; i++) /** CHANGED */
		reverse_endian(&data[i], &data[i]); /* CHANGED */

	memcpy(buf, data, 0x40);

	for(i = 0; i < 0x40; i++)
		buf[i + 16] = RotateLeft32(buf[i + 13] ^ buf[i + 8] ^ buf[i + 0] ^ buf[i + 2], 1);

	m = state[0];
	b = state[1];
	c = state[2];
	n = state[3];
	e = state[4];

	for(i = 0; i < 20; i += 5)
	{
		warden_sha1_tweedle(&b, c, n, m, &buf[0 + i], &e);
		warden_sha1_tweedle(&m, b, c, e, &buf[1 + i], &n);
		warden_sha1_tweedle(&e, m, b, n, &buf[2 + i], &c);
		warden_sha1_tweedle(&n, e, m, c, &buf[3 + i], &b);
		warden_sha1_tweedle(&c, n, e, b, &buf[4 + i], &m);
	}

	f = m;
	d = n;

	for(i = 0x14; i < 0x28; i += 5)
	{
		g =  buf[i] + RotateLeft32(f, 5) + (d ^ c ^ b);
		d = d + RotateLeft32(g + e + 0x6ed9eba1, 5) + (c ^ RotateLeft32(b, 0x1e) ^ f) + buf[i + 1] + 0x6ed9eba1;
		c = c + RotateLeft32(d, 5) + ((g + e + 0x6ed9eba1) ^ RotateLeft32(b, 0x1e) ^ RotateLeft32(f, 0x1e)) + buf[i + 2] + 0x6ed9eba1;
		e = RotateLeft32(g + e + 0x6ed9eba1, 0x1e);
		b = RotateLeft32(b, 0x1e) + RotateLeft32(c, 5) + (e ^ d ^ RotateLeft32(f, 0x1e)) + buf[i + 3] + 0x6ed9eba1;
		d = RotateLeft32(d, 0x1e);
		f = RotateLeft32(f, 0x1e) + RotateLeft32(b, 5) + (e ^ d ^ c) + buf[i + 4] + 0x6ed9eba1;
		c = RotateLeft32(c, 0x1e);

		memset(buf, 0, 20);

	} while(i < 0x28);

	m = f;
	n = d;

	for(i = 0x28; i < 0x3c; i += 5)
	{
		warden_sha1_twitter(&b, n, m, c, &buf[i + 0], &e);
		warden_sha1_twitter(&m, c, e, b, &buf[i + 1], &n);
		warden_sha1_twitter(&e, b, n, m, &buf[i + 2], &c);
		warden_sha1_twitter(&n, m, c, e, &buf[i + 3], &b);
		warden_sha1_twitter(&c, e, b, n, &buf[i + 4], &m);
	}

	f = m;
	a = m;
	d = n;

	for(i = 0x3c; i < 0x50; i += 5)
	{
		g = RotateLeft32(a, 5) + (d ^ c ^ b) + buf[i + 0] + e - 0x359d3e2a;
		b = RotateLeft32(b, 0x1e);
		e = g;
		d = (c ^ b ^ a) + buf[i + 1] + d + RotateLeft32(g, 5) - 0x359d3e2a;
		a = RotateLeft32(a, 0x1e);
		g = RotateLeft32(d, 5);
		g = (e ^ b ^ a) + buf[i + 2] + c + g - 0x359d3e2a;
		e = RotateLeft32(e, 0x1e);
		c = g;
		g = RotateLeft32(g, 5) + (e ^ d ^ a) + buf[i + 3] + b - 0x359d3e2a;
		d = RotateLeft32(d, 0x1e);
		h = (e ^ d ^ c) + buf[i + 4];
		b = g;
		g = RotateLeft32(g, 5);
		c = RotateLeft32(c, 0x1e);
		a = (h + a) + g - 0x359d3e2a;

		buf[i + 0] = 0;
		buf[i + 1] = 0;
		buf[i + 2] = 0;
		buf[i + 3] = 0;
		buf[i + 4] = 0;
	} while(i < 0x50);

	state[0] = state[0] + a;
	state[1] = state[1] + b;
	state[2] = state[2] + c;
	state[3] = state[3] + d;
	state[4] = state[4] + e;
}

//
// CWarden
//

#define SWAP(a,b) {if((a)!=(b)){(a)^=(b);(b)^=(a);(a)^=(b);}}

CWarden :: CWarden( CGHost *nGHost, uint32_t nSeed )
{
	m_GHost = nGHost;
	m_SHA1 = new CWardenSHA1( );
	m_Seed = nSeed;
	random_data_initialize( (char *)&m_Seed, 4 );
	unsigned char KeyOutGenerator[16];
	unsigned char KeyInGenerator[16];
	random_data_get_bytes( (char *)KeyOutGenerator, 16 );
	random_data_get_bytes( (char *)KeyInGenerator, 16 );
	generate_key( m_KeyOut, KeyOutGenerator, 16 );
	generate_key( m_KeyIn, KeyInGenerator, 16 );

	/*

    m_Parse(0) = Addr2Ptr(AddressOf HW0x00)
    m_Parse(1) = Addr2Ptr(AddressOf HW0x01)
    m_Parse(2) = Addr2Ptr(AddressOf HW0x02)
    m_Parse(3) = Addr2Ptr(AddressOf HW0x03)
    m_Parse(4) = Addr2Ptr(AddressOf HW0x04)
    m_Parse(5) = Addr2Ptr(AddressOf HW0x05)
    m_ModFolder = App.Path & "\Warden\"
    m_WardenIniPath = strINIPath

	*/
}

CWarden :: ~CWarden( )
{
	delete m_SHA1;
}

void CWarden :: random_data_update( )
{
	WARDENSHA1_CTX ctx;
	m_SHA1->warden_sha1_init( &ctx );
	m_SHA1->warden_sha1_update( &ctx, (char *)m_RandomData.random_source_1, 0x14 );
	m_SHA1->warden_sha1_update( &ctx, (char *)m_RandomData.random_data,     0x14 );
	m_SHA1->warden_sha1_update( &ctx, (char *)m_RandomData.random_source_2, 0x14 );
	m_SHA1->warden_sha1_final( &ctx, (int *)m_RandomData.random_data );
}

void CWarden :: random_data_initialize( char *seed, int length )
{
	int length1 = length >> 1;
	int length2 = length - length1;
	char *seed1 = seed;
	char *seed2 = seed + length1;
	memset( &m_RandomData, 0, sizeof( t_random_data ) );
	m_SHA1->warden_sha1_hash( (int *)m_RandomData.random_source_1, (unsigned char *)seed1, length1 );
	m_SHA1->warden_sha1_hash( (int *)m_RandomData.random_source_2, (unsigned char *)seed2, length2 );
	random_data_update( );
	m_RandomData.current_position = 0;
}

unsigned char CWarden :: random_data_get_byte( )
{
	int i = m_RandomData.current_position;
	unsigned char value = m_RandomData.random_data[i];

	i++;
	if( i >= 0x14 )
	{
		i = 0;
		random_data_update( );
	}

	m_RandomData.current_position = i;
	return value;
}

void CWarden :: random_data_get_bytes( char *buffer, int bytes )
{
	for( int i = 0; i < bytes; i++ )
		buffer[i] = random_data_get_byte( );
}

void CWarden :: generate_key( unsigned char *key_buffer, unsigned char *base, unsigned int base_length )
{
	unsigned char val = 0;
	unsigned int i;
	unsigned int position = 0;

	for( i = 0; i < 0x100; i++ )
		key_buffer[i] = i;

	key_buffer[0x100] = 0;
	key_buffer[0x101] = 0;

	for( i = 1; i <= 0x40; i++ )
	{
		val += key_buffer[(i * 4) - 4] + base[position++ % base_length];
		SWAP(key_buffer[(i * 4) - 4], key_buffer[val & 0x0FF]);

		val += key_buffer[(i * 4) - 3] + base[position++ % base_length];
		SWAP(key_buffer[(i * 4) - 3], key_buffer[val & 0x0FF]);

		val += key_buffer[(i * 4) - 2] + base[position++ % base_length];
		SWAP(key_buffer[(i * 4) - 2], key_buffer[val & 0x0FF]);

		val += key_buffer[(i * 4) - 1] + base[position++ % base_length];
		SWAP(key_buffer[(i * 4) - 1], key_buffer[val & 0x0FF]);
	}
}

void CWarden :: do_crypt( unsigned char *key, unsigned char *data, int length )
{
	for( int i = 0; i < length; i++ )
	{
		key[0x100]++;
		key[0x101] += key[key[0x100]];
		SWAP(key[key[0x101]], key[key[0x100]]);
		data[i] = data[i] ^ key[(key[key[0x101]] + key[key[0x100]]) & 0x0FF];
	}
}

void CWarden :: HandleWarden( BYTEARRAY packet )
{
	unsigned char *data = new unsigned char[packet.size( )];
	memcpy( data, string( packet.begin( ), packet.end( ) ).c_str( ), packet.size( ) );
	do_crypt( m_KeyIn, data, packet.size( ) );

	if( data[0] == 0x00 )
		HandleWarden0x00( data + 1, packet.size( ) - 1 );
	else if( data[0] == 0x01 )
		HandleWarden0x01( data + 1, packet.size( ) - 1 );
	else if( data[0] == 0x02 )
		HandleWarden0x02( data + 1, packet.size( ) - 1 );
	else if( data[0] == 0x03 )
		HandleWarden0x03( data + 1, packet.size( ) - 1 );
	else if( data[0] == 0x04 )
		HandleWarden0x04( data + 1, packet.size( ) - 1 );
	else if( data[0] == 0x05 )
		HandleWarden0x05( data + 1, packet.size( ) - 1 );
	else
		CONSOLE_Print( "[WARDEN] received unknown packet ID " + UTIL_ToString( data[0] ) );

	delete [] data;
}

void CWarden :: HandleWarden0x00( unsigned char *data, uint32_t length )
{
	/*

    Dim S           As String
    Call WardenCleanUp
    If (wLen < 37) Then Exit Function
    Call CopyMemory(ByVal m_ModName, ByVal hData + 1, 16)
    Call CopyMemory(m_ModKey(0), ByVal hData + 17, 16)
    Call CopyMemory(m_ModLen, ByVal hData + 33, 4)
    S = m_ModFolder & StrToHex(m_ModName, vbNullString) & ".bin"
    If (Len(Dir$(S)) = 0) Then
        If (m_ModLen < 50) Or (m_ModLen > 5000000) Then
            m_ModLen = 0
            Exit Function
        End If
        S = vbNullChar
        ReDim m_ModData(m_ModLen - 1)
        m_ModState = 1
    Else
        If (LoadModule(0, S) = 0) Then Exit Function
        S = Chr$(&H1)
        m_ModLen = 0
        m_ModState = 2
    End If
    m_ModPos = 0
    Call RC4CryptStr(S, m_KeyOut(), 1)
    Call OnSendPacket(S)
    HW0x00 = 1

	*/
}

void CWarden :: HandleWarden0x01( unsigned char *data, uint32_t length )
{
	/*

    If (Not m_ModState = 1) Then Exit Function
    If (m_ModLen = 0) Then Exit Function
    If (wLen < 4) Then Exit Function
    Call CopyMemory(m_ModData(m_ModPos), ByVal hData + 3, wLen - 3)
    m_ModPos = m_ModPos + (wLen - 3)
    'Debug.Print m_ModPos & " Of " & m_ModLen
    If (m_ModPos >= m_ModLen) Then
        m_ModState = 2
        HW0x01 = HW0x01Ex()
    Else
        HW0x01 = 1
    End If

	*/
}

void CWarden :: HandleWarden0x02( unsigned char *data, uint32_t length )
{
	/*

    On Error GoTo HW0x02Err
    'eww, yep
    Dim S               As String
    Dim strData         As String
    Dim P               As Long
    Dim strOut          As String
    Dim PosOut          As Long
    Dim bHeader(6)      As Byte
    'string table
    Dim B           As Long 'lengh of string
    Dim sCount      As Byte
    Dim sTable(20)  As String
    If (Not m_ModState = 2) Then GoTo HW0x02Err
    If (wLen < 2) Then GoTo HW0x02Err
    S = Space(wLen)
    Call CopyMemory(ByVal S, ByVal hData, wLen)
    P = 2
    Do Until (P >= wLen)
        B = Asc(Mid$(S, P, 1)): P = P + 1
        If (B = 0) Then Exit Do
        sCount = sCount + 1
        sTable(sCount) = Mid$(S, P, B): P = P + B
    Loop
    PosOut = 8
    strOut = Space(512) 'max size are are send buffer
    Do Until (P >= wLen)
        strData = Get0x02Data(S, P, sTable(), sCount)
        If (Len(strData) = 0) Then GoTo HW0x02Err
        Mid$(strOut, PosOut, Len(strData)) = strData
        PosOut = PosOut + Len(strData)
    Loop
    strOut = Left$(strOut, (PosOut - 1))
    bHeader(0) = &H2
    Call CopyMemory(bHeader(1), CInt(PosOut - 8), 2)
    Call CopyMemory(bHeader(3), WardenChecksum(Mid$(strOut, 8)), 4)
    Call CopyMemory(ByVal strOut, bHeader(0), 7)
    Call RC4CryptStr(strOut, m_KeyOut(), 1)
    Call OnSendPacket(strOut)
    HW0x02 = 1
    Exit Function
HW0x02Err:
    Debug.Print "HW0x02() Error: " & Err.Description

	*/
}

/*

Private Function Get0x02Data(ByRef S As String, ByRef P As Long, ByRef sTable() As String, ByVal sCount As Byte) As String
    Dim R           As String
    Dim bTest       As Boolean
    Dim A           As Long
    Dim L           As Byte
    If ((P + 6) >= Len(S)) Then Exit Function
    bTest = (Asc(Mid(S, P + 1, 1)) <= sCount)
    bTest = bTest And (Asc(Mid(S, P + 6, 1)) < &H40)
    If bTest Then
        Call CopyMemory(A, ByVal Mid$(S, P + 2, 4), 4)
        L = Asc(Mid$(S, P + 6, 1))
        R = GetINI("MEMORY", CStr(sTable(Asc(Mid(S, P + 1, 1))) & "&H" & Hex(A) & "_" & L), m_WardenIniPath, vbNullString)
        If Len(R) Then
            P = P + 7
            Get0x02Data = vbNullChar & HexToStr(R)
            Exit Function
        End If
    End If
    If ((P + 29) >= Len(S)) Then Exit Function
    bTest = (Asc(Mid$(S, P + 29, 1)) < &H80)
    bTest = bTest And (Asc(Mid$(S, P + 28, 1)) = 0)
    bTest = bTest And (Asc(Mid$(S, P + 27, 1)) < &H40)
    If (bTest = False) Then Exit Function
    Call CopyMemory(A, ByVal Mid$(S, P + 26, 4), 4)
    If Len(GetINI("PAGEA", CStr("&H" & Hex(A)), m_WardenIniPath, vbNullString)) = 0 Then Exit Function
    P = P + 30
    Get0x02Data = vbNullChar
End Function

*/

void CWarden :: HandleWarden0x03( unsigned char *data, uint32_t length )
{
	// ignore this
}

void CWarden :: HandleWarden0x04( unsigned char *data, uint32_t length )
{
	// ignore this
}

void CWarden :: HandleWarden0x05( unsigned char *data, uint32_t length )
{
	/*

    Dim bKey(257)       As Byte
    Dim bData()         As Byte
    Dim lngRecv         As Long
    Dim bCode(39)       As Byte
    If (Not m_ModState = 2) Then Exit Function
    m_RC4 = 0
    m_ModState = 0
    'PUSH   Size
    'PUSH   &Seed
    'MOV    ECX, Param
    'XOR    EDX, EDX
    'MOV    EAX, Address
    'CALL   EAX
    'RETN   16
    Call ExecuteCode(bCode(), &H68, &H4&, _
                              &H68, VarPtr(m_Seed), _
                              &HB9, m_ModMem, _
                              &HD233, _
                              &HB8, m_Func(0), _
                              &HD0FF, _
                              &HC2, 16, 0)
    If (m_RC4 = 0) Then Exit Function
    ReDim bData(wLen - 1)
    Call CopyMemory(bData(0), ByVal hData, wLen)
    Call CopyMemory(bKey(0), ByVal m_RC4 + 258, 258)
    Call RC4Crypt(bData(), bKey(), wLen)
    Call CopyMemory(bKey(0), ByVal m_RC4, 258)
    m_PKT = vbNullString
    'PUSH   &BytesRead
    'PUSH   Lengh
    'PUSH   &bData
    'MOV    ECX, Mem
    'XOR    EDX, EDX
    'MOV    EAX, Address
    'CALL   EAX
    'RETN   16
    Call ExecuteCode(bCode(), &H68, VarPtr(lngRecv), _
                              &H68, wLen, _
                              &H68, VarPtr(bData(0)), _
                              &HB9, m_ModMem, _
                              &HD233, _
                              &HB8, m_Func(2), _
                              &HD0FF, _
                              &HC2, 16, 0)
    If (Len(m_PKT) = 0) Then Exit Function
    Call RC4CryptStr(m_PKT, bKey(), 1)
    Call RC4CryptStr(m_PKT, m_KeyOut(), 1)
    Call CopyMemory(m_KeyOut(0), ByVal m_RC4, 258)
    Call CopyMemory(m_KeyIn(0), ByVal m_RC4 + 258, 258)
    m_ModState = 2
    Call OnSendPacket(m_PKT)
    m_RC4 = 0
    m_PKT = vbNullString

	*/
}

/*

Private Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" (ByRef Destination As Any, ByRef Source As Any, ByVal numbytes As Long)
Private Declare Sub ZeroMemory Lib "kernel32" Alias "RtlZeroMemory" (ByRef Destination As Any, ByVal numbytes As Long)
Private Declare Function GetPrivateProfileStringA Lib "kernel32" (ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpDefault As String, ByVal lpReturnedString As String, ByVal nSize As Long, ByVal lpFileName As String) As Long
Private Declare Function WritePrivateProfileStringA Lib "kernel32" (ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpString As Any, ByVal lpFileName As String) As Long
Private Declare Function LoadLibraryA Lib "kernel32" (ByVal strFilePath As String) As Long
Private Declare Function GetProcAddress Lib "kernel32" (ByVal hModule As Long, ByVal lpProcName As String) As Long
Private Declare Function GlobalAlloc Lib "kernel32" (ByVal wFlags As Long, ByVal dwBytes As Long) As Long
Private Declare Function GlobalFree Lib "kernel32" (ByVal hMem As Long) As Long
Private Declare Function GlobalLock Lib "kernel32" (ByVal hMem As Long) As Long
Private Declare Function GlobalUnlock Lib "kernel32" (ByVal hMem As Long) As Long
Private Declare Function CallWindowProcA Lib "user32" (ByVal lpPrevWndFunc As Long, ByVal hWnd As Long, ByVal uMsg As Long, ByVal wParam As Long, ByVal lParam As Long) As Long
Private Declare Function uncompress Lib "zlib" (ByRef dest As Any, ByRef destLen As Long, ByRef src As Any, ByRef srcLen As Long) As Long
Private Declare Function htonl Lib "ws2_32" (ByVal hostlong As Long) As Long
Private Declare Function send Lib "ws2_32" (ByVal sckHandle As Long, ByRef InBuf As Any, ByVal buflen As Long, ByVal Flags As Long) As Long

'//Does the job of Maiev.mod

Private m_Parse(5)          As Long
Private m_CallBack(7)       As Long 'callback function list, for warden
Private m_Func(2)           As Long 'wardens exports
Private m_KeyOut(257)       As Byte
Private m_KeyIn(257)        As Byte
Private m_Seed              As Long
Private m_Mod               As Long 'pointer to the module
Private m_ModMem            As Long 'pointer to wardens memory block
Private m_ModState          As Byte '0=idle,1=downloading,2=hackyhacky
Private m_RC4               As Long
Private m_PKT               As String
Private m_WardenIniPath     As String 'the warden folder
Private m_SocketHandle      As Long

'//Warden download stuff
Private m_ModName           As String * 16 'the modules name
Private m_ModFolder         As String 'the warden folder
Private m_ModKey(15)        As Byte 'key to crypt module with
Private m_ModLen            As Long 'lengh of downloading module
Private m_ModPos            As Long 'position in write data for downloading
Private m_ModData()         As Byte 'module download buffer

Public Sub WardenCleanUp()
    '//Unload any existing module
    If m_Mod Then
        Call UnloadModule
        Call free(m_Mod)
        Call ZeroMemory(m_Func(0), 12)
        m_Mod = 0
        m_ModMem = 0
    End If
    m_ModState = 0
    '//Clear download variables
    Call ZeroMemory(ByVal m_ModName, 16)
    Call ZeroMemory(m_ModKey(0), 16)
    m_ModLen = 0
    m_ModPos = 0
    Erase m_ModData()
End Sub
Private Function LoadModule(ByVal lngMod As Long, ByRef strPath As String) As Long
    Dim bData()         As Byte
    Dim i               As Long
    If (lngMod = 0) Then
        i = FreeFile
        Open strPath For Binary Lock Read As #i
            If (LOF(i) < 1) Then
                Close #i
                Exit Function
            End If
            ReDim bData(LOF(i))
            Get #i, 1, bData()
        Close #i
        lngMod = VarPtr(bData(0))
    End If
    If m_Mod Then
        Call UnloadModule
        Call free(m_Mod)
        Call ZeroMemory(m_Func(0), 12)
    End If
    m_ModMem = 0
    m_Mod = PrepareModule(lngMod)
    If (m_Mod = 0) Then Exit Function
    Call InitModule
    If (m_ModMem = 0) Then
        Call free(m_Mod)
        Exit Function
    End If
    Call CopyMemory(i, ByVal m_ModMem, 4)
    Call CopyMemory(m_Func(0), ByVal i, 12)
    LoadModule = 1
End Function


Private Function HW0x01Ex() As Long
    On Error GoTo HW0x01ExErr
    Dim bData()         As Byte
    Dim i               As Long
    Dim S               As String
    ReDim bData(257)
    Call RC4Key(m_ModKey(), bData(), 16)
    Call RC4Crypt(m_ModData(), bData(), m_ModLen)
    Call CopyMemory(i, m_ModData(0), 4)
    If (i < &H120) Or (i > 5000000) Then GoTo HW0x01ExErr
    ReDim bData(i - 1)
    If (Not uncompress(bData(0), i, m_ModData(4), CLng(m_ModLen - &H108)) = 0) Then GoTo HW0x01ExErr
    m_ModLen = 0
    m_ModPos = 0
    Erase m_ModData()
    S = m_ModFolder & StrToHex(m_ModName, vbNullString) & ".bin"
    i = FreeFile
    Open S For Binary Lock Write As #i
        Put #i, 1, bData()
    Close #i
    If (LoadModule(VarPtr(bData(0)), S) = 0) Then GoTo HW0x01ExErr
    m_ModState = 2
    bData(0) = 1
    Call RC4Crypt(bData(), m_KeyOut(), 1)
    Call OnSendPacket(Chr$(bData(0)))
    Erase bData()
    HW0x01Ex = 1
    Exit Function
HW0x01ExErr:
    Erase m_ModData()
    m_ModLen = 0
    m_ModPos = 0
    m_ModState = 0
    Debug.Print "HW0x01Ex() Error: " & Err.Description
End Function

Private Function PrepareModule(ByRef pModule As Long) As Long
    '//carbon copy port from iagos code
    Debug.Print "PrepareModule()"
    Dim dwModuleSize        As Long
    Dim pNewModule          As Long
    dwModuleSize = getInteger(pModule, &H0)
    pNewModule = malloc(dwModuleSize)
    Call ZeroMemory(ByVal pNewModule, dwModuleSize)
    Debug.Print "   Allocated " & dwModuleSize & " (0x" & Hex(dwModuleSize) & ") bytes for new module"
    Call CopyMemory(ByVal pNewModule, ByVal pModule, 40)
    Dim dwSrcLocation       As Long
    Dim dwDestLocation      As Long
    Dim dwLimit             As Long
    dwSrcLocation = &H28 + (getInteger(pNewModule, &H24) * 12)
    dwDestLocation = getInteger(pModule, &H28)
    dwLimit = getInteger(pModule, &H0)
    Dim bSkip               As Boolean
    Debug.Print "   Copying code sections to module."
    While (dwDestLocation < dwLimit)
        Dim dwCount         As Long
        Call CopyMemory(ByVal VarPtr(dwCount), ByVal pModule + dwSrcLocation, 1)
        Call CopyMemory(ByVal VarPtr(dwCount) + 1, ByVal pModule + dwSrcLocation + 1, 1)
        dwSrcLocation = dwSrcLocation + 2
        If (bSkip = False) Then
            Call CopyMemory(ByVal pNewModule + dwDestLocation, ByVal pModule + dwSrcLocation, dwCount)
            dwSrcLocation = dwSrcLocation + dwCount
        End If
        bSkip = Not bSkip
        dwDestLocation = dwDestLocation + dwCount
    Wend
    Debug.Print "   Adjusting references to global variables..."
    dwSrcLocation = getInteger(pModule, 8)
    dwDestLocation = 0
    Dim i                       As Long
    Dim lng0x0C                 As Long
    Dim lngTest                 As Long
    Call CopyMemory(lng0x0C, ByVal pNewModule + &HC, 4)
    While (i < lng0x0C)
        Call CopyMemory(lngTest, ByVal pNewModule + dwSrcLocation, 1)
        lngTest = lngTest And &HFF&
        Call CopyMemory(ByVal VarPtr(lngTest) + 0, ByVal pNewModule + dwSrcLocation + 1, 1)
        Call CopyMemory(ByVal VarPtr(lngTest) + 1, ByVal pNewModule + dwSrcLocation, 1)
        dwDestLocation = dwDestLocation + lngTest
        dwSrcLocation = dwSrcLocation + 2
        Call insertInteger(pNewModule, dwDestLocation, getInteger(pNewModule, dwDestLocation) + pNewModule)
        i = i + 1
    Wend
    Debug.Print "   Updating API library references.."
    dwLimit = getInteger(pNewModule, &H20)
    Dim dwProcStart             As Long
    Dim szLib                   As String
    Dim dwProcOffset            As Long
    Dim hModule                 As Long
    Dim dwProc                  As Long
    Dim szFunc                  As String
    For i = 0 To dwLimit - 1
        dwProcStart = getInteger(pNewModule, &H1C) + (i * 8)
        szLib = GetSTR(pNewModule + getInteger(pNewModule, dwProcStart))
        dwProcOffset = getInteger(pNewModule, dwProcStart + 4)
        Debug.Print "   Lib: " & szLib
        hModule = LoadLibraryA(szLib)
        dwProc = getInteger(pNewModule, dwProcOffset)
        While dwProc
            If (dwProc > 0) Then
                szFunc = GetSTR(pNewModule + dwProc)
                Debug.Print "       Function: " & szFunc
                Call insertInteger(pNewModule, dwProcOffset, GetProcAddress(hModule, szFunc))
            Else
                dwProc = dwProc And &H7FFFFFFF
                Debug.Print "       Ordinary: 0x" & Hex(dwProc)
            End If
            dwProcOffset = dwProcOffset + 4
            dwProc = getInteger(pNewModule, dwProcOffset)
        Wend
    Next i
    Debug.Print "   Successfully mapped Warden Module to 0x" & Hex(pNewModule)
    PrepareModule = pNewModule
End Function
Private Sub InitModule()
    Debug.Print "InitModule()"
    Dim A               As Long
    Dim B               As Long
    Dim C               As Long
    Dim bCode(15)       As Byte
    C = getInteger(m_Mod, &H18)
    B = 1 - C
    If (B > getInteger(m_Mod, &H14)) Then Exit Sub
    A = getInteger(m_Mod, &H10)
    A = getInteger(m_Mod, A + (B * 4)) + m_Mod
    Debug.Print "   Initialize Function is mapped at 0x" & Hex(A)
    m_CallBack(0) = Addr2Ptr(AddressOf SendPacket)
    m_CallBack(1) = Addr2Ptr(AddressOf CheckModule)
    m_CallBack(2) = Addr2Ptr(AddressOf ModuleLoad)
    m_CallBack(3) = Addr2Ptr(AddressOf AllocateMem)
    m_CallBack(4) = Addr2Ptr(AddressOf FreeMemory)
    m_CallBack(5) = Addr2Ptr(AddressOf SetRC4Data)
    m_CallBack(6) = Addr2Ptr(AddressOf GetRC4Data)
    m_CallBack(7) = VarPtr(m_CallBack(0))
    'MOV    ECX, &Param
    'CALL   Address
    'RETN   16
    m_ModMem = ExecuteCode(bCode(), &HB9, VarPtr(m_CallBack(7)), _
                                    &H15FF, VarPtr(A), _
                                    &HC2, 16, 0)
End Sub
Private Sub UnloadModule()
    Dim bCode(15)   As Byte
    'MOV    ECX, Param
    'CALL   Address
    'RETN   16
    Call ExecuteCode(bCode(), &HB9, m_ModMem, _
                              &H15FF, VarPtr(m_Func(1)), _
                              &HC2, 16, 0)
End Sub



Private Sub SendPacket(ByVal ptrPacket As Long, ByVal dwSize As Long)
    If (dwSize < 1) Then Exit Sub
    If (dwSize > 5000) Then Exit Sub
    m_PKT = Space(dwSize)
    Call CopyMemory(ByVal m_PKT, ByVal ptrPacket, dwSize)
    'Debug.Print "Warden.SendPacket() pkt=0x" & Hex(ptrPacket) & ", size=" & dwSize & vbCrLf & GetLog(m_PKT)
End Sub
Private Function CheckModule(ByVal ptrMod As Long, ByVal ptrKey As Long) As Long
    'Debug.Print "Warden.CheckModule() " & ptrMod & "/" & ptrKey
    'CheckModule = 0 '//Need to download
    'CheckModule = 1 '//Don't need to download
    CheckModule = 1
End Function
Private Function ModuleLoad(ByVal ptrRC4Key As Long, ByVal pModule As Long, ByVal dwModSize As Long) As Long
    'Debug.Print "Warden.ModuleLoad() " & ptrMod & "/" & ptrKey
    'ModuleLoad = 0 '//Need to download
    'ModuleLoad = 1 '//Don't need to download
    ModuleLoad = 1
End Function
Private Function AllocateMem(ByVal dwSize As Long) As Long
    AllocateMem = malloc(dwSize)
End Function
Private Sub FreeMemory(ByVal dwMemory As Long)
    Call free(dwMemory)
    'Debug.Print "Warden.FreeMemory() 0x" & Hex(dwMemory)
End Sub
Private Function SetRC4Data(ByVal lpKeys As Long, ByVal dwSize As Long) As Long
    'Debug.Print "Warden.SetRC4Data() 0x" & Hex(lpKeys) & "/0x" & Hex(dwSize)
End Function
Private Function GetRC4Data(ByVal lpBuffer As Long, ByRef dwSize As Long) As Long
    'Debug.Print "Warden.GetRC4Data() 0x" & Hex(lpBuffer) & "/0x" & Hex(dwSize)
    'GetRC4Data = 1 'got the keys already
    'GetRC4Data = 0 'generate new keys
    GetRC4Data = m_RC4
    m_RC4 = lpBuffer
End Function



Private Function getInteger(ByRef bArray As Long, ByVal dwLocation As Long) As Long
    Call CopyMemory(getInteger, ByVal bArray + dwLocation, 4)
End Function
Private Sub insertInteger(ByRef bArray As Long, ByVal dwLocation As Long, ByVal dwValue As Long)
    Call CopyMemory(ByVal bArray + dwLocation, dwValue, 4)
End Sub
Private Function GetSTR(ByRef bArray As Long) As String
    Dim bTest           As Byte
    Dim i               As Long
    Do
        Call CopyMemory(bTest, ByVal bArray + i, 1)
        If (bTest = 0) Then
            If (i = 0) Then Exit Function
            GetSTR = String(i, 0)
            Call CopyMemory(ByVal GetSTR, ByVal bArray, i)
            Exit Function
        End If
        i = i + 1
    Loop
End Function
Private Function Addr2Ptr(ByVal lngAddr As Long) As Long
    Addr2Ptr = lngAddr
End Function

Private Function WardenChecksum(ByRef S As String) As Long
    Dim lngData(4)  As Long
    Call CopyMemory(lngData(0), ByVal BSHA1(S, True, True), 20)
    WardenChecksum = lngData(0) Xor lngData(1) Xor lngData(2) Xor lngData(3) Xor lngData(4)
End Function




'###################################################################################
'###################################################################################
'############################## Other Functions ####################################
'###################################################################################
'###################################################################################
Private Function malloc(ByVal dwSize As Long) As Long
    Dim lngHandle   As Long
    lngHandle = GlobalAlloc(0, dwSize + 4)
    malloc = GlobalLock(lngHandle) + 4
    Call CopyMemory(ByVal malloc - 4, lngHandle, 4)
End Function
Private Sub free(ByVal dwPtr As Long)
    Dim lngHandle   As Long
    Call CopyMemory(lngHandle, ByVal dwPtr - 4, 4)
    Call GlobalUnlock(lngHandle)
    Call GlobalFree(lngHandle)
End Sub

Private Function GetINI(ByVal strHeader As String, ByVal strValueName As String, ByVal strFileName As String, Optional ByVal strDefalt As String = vbNullString) As String
    Dim S       As String
    S = String(512, vbNullChar)
    Call GetPrivateProfileStringA(strHeader, strValueName, strDefalt, S, Len(S), strFileName)
    GetINI = GetSTRING(S)
End Function
Private Function SaveINI(ByVal strHeader As String, ByVal strValueName As String, ByVal strValue As String, ByVal strFileName As String)
    On Error GoTo SaveINIErr
    Call WritePrivateProfileStringA(strHeader, strValueName, strValue, strFileName)
SaveINIErr:
End Function
Private Function GetSTRING(ByVal S As String, Optional ByVal T As String = vbNullChar) As String
    Dim i       As Integer
    i = InStr(1, S, T)
    If (i = 0) Then
        GetSTRING = S
    Else
        GetSTRING = Mid(S, 1, (i - 1))
    End If
End Function
Private Function StrToHex(ByVal strData As String, Optional Splitter As String = " ") As String
    Dim aLen As Long: aLen = Len(strData)
    Dim eLen As Long: eLen = 2 + Len(Splitter)
    Dim iPos As Long
    StrToHex = Space((aLen * eLen))
    For i = 1 To (aLen * eLen) Step eLen
        iPos = iPos + 1
        Mid(StrToHex, i, eLen) = Right("0" & Hex(Asc(Mid(strData, iPos, 1))), 2) & Splitter
    Next i
    StrToHex = Trim(StrToHex)
End Function
Private Function HexToStr(ByVal strData As String) As String
    HexToStr = String(Len(strData) / 3, 0)
    Dim iPos As Long
    For i = 1 To Len(strData) Step 3
        iPos = iPos + 1
        Mid$(HexToStr, iPos, 1) = Chr("&H" & Mid$(strData, i, 2))
    Next i
End Function
Private Function ExecuteCode(ByRef bBuf() As Byte, ParamArray Code() As Variant) As Long
    Dim i       As Long
    Dim Pos     As Long
    For i = 0 To UBound(Code)
        If (VarType(Code(i)) = vbLong) Then
            Call CopyMemory(bBuf(Pos), CLng(Code(i)), 4): Pos = Pos + 4
        ElseIf (Code(i) >= 0) And (Code(i) <= &HFF) Then
            bBuf(Pos) = Code(i): Pos = Pos + 1
        ElseIf (Code(i) >= &H8000) And (Code(i) <= &H7FFF) Then
            Call CopyMemory(bBuf(Pos), CInt(Code(i)), 2): Pos = Pos + 2
        Else
            Call CopyMemory(bBuf(Pos), CLng(Code(i)), 4): Pos = Pos + 4
        End If
    Next i
    ExecuteCode = CallWindowProcA(VarPtr(bBuf(0)), 0, 0, 0, 0)
End Function
'~~~~~~ Extra functions ~~~~~~~
Private Function LSC(ByVal N As Long, ByVal S As Long) As Long
    'left shift circle
    LSC = (LS(N, S) Or RS(N, (32 - S)))
End Function
Private Function RS(ByVal N As Long, ByVal S As Long) As Long
    'right shift bits
    If (S < 0) Or (S > 31) Then
        RS = 0
    ElseIf (S = 0) Then
        RS = N
    Else
        If ((N And &H80000000) = &H80000000) Then
            N = (N And &H7FFFFFFF)
            If (S = 31) Then 'stop over flow when shifting 31bits
                N = N / 2147483648#
            Else
                N = N \ (2 ^ S)
            End If
            RS = N Or (2 ^ (31 - S))
        Else
           RS = Int(CDbl(N) / CDbl(2 ^ S))
        End If
    End If
End Function
Private Function LS(ByVal N As Long, ByVal S As Long) As Long
    'left shift bits
    If (S < 0) Or (S > 31) Then
        LS = 0
    ElseIf S = 0 Then
        LS = N
    Else
        N = N And (2 ^ (32 - S) - 1)
        LS = WDbl(CDbl(N) * CDbl(WDbl(2 ^ S)))
    End If
End Function
Private Function WDbl(ByVal N As Double) As Long
    'wrap a double back to a long
    If N > &H7FFFFFFF Then
        N = N - 4294967296#
    ElseIf N < &H80000000 Then
        N = N + 4294967296#
    End If
    WDbl = N
End Function
Private Function Add(ByVal N1 As Long, ByVal N2 As Long, Optional ByVal D As Double) As Long
    'add 2 longs to a double, then wrap round
    D = N1
    D = D + N2
    Add = WDbl(D)
End Function
Private Function MakeDWORD(ByVal lngValue As Long) As String
    MakeDWORD = String(4, vbNullChar)
    Call CopyMemory(ByVal MakeDWORD, lngValue, 4)
End Function
Private Sub OnSendPacket(ByRef Data As String)
    Dim bHeader(3)  As Byte
    bHeader(0) = &HFF
    bHeader(1) = &H5E
    Call CopyMemory(bHeader(2), CInt(Len(Data) + 4), 2)
    Call send(m_SocketHandle, bHeader(3), 4, 0&)
    Call send(m_SocketHandle, ByVal Data, Len(Data), 0&)
    Debug.Print "SEND: " & StrToHex(Data)
End Sub

*/

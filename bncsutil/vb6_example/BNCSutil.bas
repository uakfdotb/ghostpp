Attribute VB_Name = "BNCSutil"
Option Explicit

'------------------------------------------------------------------------------
'  BNCSutil
'  Battle.Net Utility Library
'
'  Copyright © 2004-2005 Eric Naeseth
'------------------------------------------------------------------------------
'  Visual Basic Declarations
'  November 20, 2004
'------------------------------------------------------------------------------
'  This library is free software; you can redistribute it and/or
'  modify it under the terms of the GNU Lesser General Public
'  License as published by the Free Software Foundation; either
'  version 2.1 of the License, or (at your option) any later version.
'
'  This library is distributed in the hope that it will be useful,
'  but WITHOUT ANY WARRANTY; without even the implied warranty of
'  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
'  Lesser General Public License for more details.
'
'  A copy of the GNU Lesser General Public License is included in the BNCSutil
'  distribution in the file COPYING.  If you did not receive this copy,
'  write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
'  Boston, MA  02111-1307  USA
'------------------------------------------------------------------------------

'  DLL Imports
'---------------------------

' Library Information
Public Declare Function bncsutil_getVersion Lib "bncsutil.dll" () As Long
Public Declare Function bncsutil_getVersionString_Raw Lib "bncsutil.dll" _
    Alias "bncsutil_getVersionString" (ByVal outBuf As String) As Long

' CheckRevision
Public Declare Function extractMPQNumber Lib "bncsutil.dll" _
    (ByVal mpqName As String) As Long
' [!] You should use checkRevision and getExeInfo (see below) instead of their
'     _Raw counterparts.
Public Declare Function checkRevision_Raw Lib "bncsutil.dll" Alias "checkRevisionFlat" _
    (ByVal ValueString As String, ByVal File1 As String, ByVal File2 As String, _
     ByVal File3 As String, ByVal mpqNumber As Long, ByRef Checksum As Long) As Long
Public Declare Function getExeInfo_Raw Lib "bncsutil.dll" Alias "getExeInfo" _
    (ByVal Filename As String, ByVal exeInfoString As String, _
    ByVal infoBufferSize As Long, Version As Long, ByVal Platform As Long) As Long

' Old Logon System
' [!] You should use doubleHashPassword and hashPassword instead of their
'     _Raw counterparts.  (See below for those functions.)
Public Declare Sub doubleHashPassword_Raw Lib "bncsutil.dll" Alias "doubleHashPassword" _
    (ByVal Password As String, ByVal ClientToken As Long, ByVal ServerToken As Long, _
    ByVal outBuffer As String)
Public Declare Sub hashPassword_Raw Lib "bncsutil.dll" Alias "hashPassword" _
    (ByVal Password As String, ByVal outBuffer As String)

' Broken SHA-1
Public Declare Sub calcHashBuf Lib "bncsutil.dll" _
    (ByVal Data As String, ByVal Length As Long, ByVal Hash As String)

' CD-Key Decoding

' Call kd_init() first to set up the decoding system, unless you are only using kd_quick().
' Then call kd_create() to create a key decoder "handle" each time you want to
' decode a CD-key.  It will return the handle or -1 on failure.  The handle
' should then be passed as the "decoder" argument to all the other kd_ functions.
' Call kd_free() on the handle when finished with the decoder to free the
' memory it is using.

Public Declare Function kd_quick Lib "bncsutil.dll" _
    (ByVal CDKey As String, ByVal ClientToken As Long, ByVal ServerToken As Long, _
    PublicValue As Long, Product As Long, ByVal HashBuffer As String, ByVal BufferLen As Long) As Long
Public Declare Function kd_init Lib "bncsutil.dll" () As Long
Public Declare Function kd_create Lib "bncsutil.dll" _
    (ByVal CDKey As String, ByVal keyLength As Long) As Long
Public Declare Function kd_free Lib "bncsutil.dll" _
    (ByVal decoder As Long) As Long
Public Declare Function kd_val2Length Lib "bncsutil.dll" _
    (ByVal decoder As Long) As Long
Public Declare Function kd_product Lib "bncsutil.dll" _
    (ByVal decoder As Long) As Long
Public Declare Function kd_val1 Lib "bncsutil.dll" _
    (ByVal decoder As Long) As Long
Public Declare Function kd_val2 Lib "bncsutil.dll" _
    (ByVal decoder As Long) As Long
Public Declare Function kd_longVal2 Lib "bncsutil.dll" _
    (ByVal decoder As Long, ByVal Out As String) As Long
Public Declare Function kd_calculateHash Lib "bncsutil.dll" _
    (ByVal decoder As Long, ByVal ClientToken As Long, ByVal ServerToken As Long) As Long
Public Declare Function kd_getHash Lib "bncsutil.dll" _
    (ByVal decoder As Long, ByVal Out As String) As Long
Public Declare Function kd_isValid Lib "bncsutil.dll" _
    (ByVal decoder As Long) As Long
    
'New Logon System

' Call nls_init() to get a "handle" to an NLS object (nls_init will return 0
' if it encounters an error).  This "handle" should be passed as the "NLS"
' argument to all the other nls_* functions.  You do not need to change the
' username and password to upper-case as nls_init() will do this for you.
' Call nls_free() on the handle to free the memory it's using.
' nls_account_create() and nls_account_logon() generate the bodies of
' SID_AUTH_ACCOUNTCREATE and SID_AUTH_ACCOUNTLOGIN packets, respectively.

Public Declare Function nls_init Lib "bncsutil.dll" _
    (ByVal Username As String, ByVal Password As String) As Long 'really returns a POINTER!
Public Declare Function nls_init_l Lib "bncsutil.dll" _
    (ByVal Username As String, ByVal Username_Length As Long, _
    ByVal Password As String, ByVal Password_Length As Long) As Long
Public Declare Function nls_reinit Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal Username As String, ByVal Password As String) As Long
Public Declare Function nls_reinit_l Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal Username As String, ByVal Username_Length As Long, _
    ByVal Password As String, ByVal Password_Length As Long) As Long
Public Declare Sub nls_free Lib "bncsutil.dll" _
    (ByVal NLS As Long)
Public Declare Function nls_account_create Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal Buffer As String, ByVal BufLen As Long) As Long
Public Declare Function nls_account_logon Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal Buffer As String, ByVal BufLen As Long) As Long
Public Declare Sub nls_get_A Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal Out As String)
Public Declare Sub nls_get_M1 Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal Out As String, ByVal B As String, ByVal Salt As String)
Public Declare Function nls_check_M2 Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal M2 As String, ByVal B As String, ByVal Salt As String) As Long
Public Declare Function nls_check_signature Lib "bncsutil.dll" _
    (ByVal Address As Long, ByVal Signature As String) As Long
Public Declare Function nls_account_change_proof Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal Buffer As String, ByVal NewPassword As String, _
    ByVal B As String, ByVal Salt As String) As Long 'returns a new NLS pointer for the new password
Public Declare Sub nls_get_S Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal Out As String, ByVal B As String, ByVal Salt As String)
Public Declare Sub nls_get_K Lib "bncsutil.dll" _
    (ByVal NLS As Long, ByVal Out As String, ByVal S As String)
    
Private Declare Function bncsutil_debug_status Lib "bncsutil.dll" () As Long
Private Declare Function bncsutil_set_debug_status Lib "bncsutil.dll" (ByVal NewStatus As Long) As Long
Private Declare Sub bncsutil_debug_message Lib "bncsutil.dll" (ByVal Message As String)
Private Declare Sub bncsutil_debug_dump Lib "bncsutil.dll" (ByVal Message As String, ByVal Length As Long)
Private Declare Function bncsutil_internal_debug_messages Lib "bncsutil.dll" () As Long
    
'  Constants
'---------------------------
Public Const BNCSUTIL_PLATFORM_X86& = &H1
Public Const BNCSUTIL_PLATFORM_WINDOWS& = &H1
Public Const BNCSUTIL_PLATFORM_WIN& = &H1

Public Const BNCSUTIL_PLATFORM_PPC& = &H2
Public Const BNCSUTIL_PLATFORM_MAC& = &H2

Public Const BNCSUTIL_PLATFORM_OSX& = &H3

'  Winsock
'---------------------------
Private Type sockaddr_in
    Family As Integer
    Port As Integer
    Address As Long
    Filler As String * 8
End Type

Private Declare Function getsockname Lib "ws2_32.dll" (ByVal S As Long, Name As sockaddr_in, NameLen As Long) As Long


'---------------------------------------'
'  VB-Specifc Functions and Properties  '
'---------------------------------------'

Public Property Get Version() As String
    Version = bncsutil_getVersionString()
End Property

Public Property Get DebugMode() As Boolean
    DebugMode = (bncsutil_debug_status() <> 0)
End Property

Public Property Let DebugMode(ByVal NewValue As Boolean)
    If (NewValue) Then
        bncsutil_set_debug_status 1
    Else
        bncsutil_set_debug_status 0
    End If
End Property

Public Property Get InternalDebugMessages() As Boolean
    InternalDebugMessages = (bncsutil_internal_debug_messages() <> 0)
End Property

Public Sub DebugMessage(Message As String)
    bncsutil_debug_message Message
End Sub

Public Sub DebugHexDump(Data As String, Optional ByVal Length As Long = -1)
    If (Length = -1) Then
        Length = Len(Data)
    End If
    
    bncsutil_debug_dump Data, Length
End Sub


' RequiredVersion must be a version as a.b.c
' Returns True if the current BNCSutil version is sufficent, False if not.
Public Function bncsutil_checkVersion(ByVal RequiredVersion As String) As Boolean
    Dim i&, j&
    Dim Frag() As String
    Dim Req As Long, Check As Long
    bncsutil_checkVersion = False
    Frag = Split(RequiredVersion, ".")
    j = 0
    For i = UBound(Frag) To 0 Step -1
        Check = Check + (CLng(Val(Frag(i))) * (100 ^ j))
        j = j + 1
    Next i
    Check = bncsutil_getVersion()
    If (Check >= Req) Then
        bncsutil_checkVersion = True
    End If
End Function

Public Function bncsutil_getVersionString() As String
    Dim S As String * 12
    Dim Length As Long
    Length = bncsutil_getVersionString_Raw(S)
    bncsutil_getVersionString = Left$(S, Length)
End Function

'CheckRevision
Public Function checkRevision(ValueString As String, File1$, File2$, File3$, mpqNumber As Long, Checksum As Long) As Boolean
    checkRevision = (checkRevision_Raw(ValueString, File1, File2, File3, mpqNumber, Checksum) > 0)
End Function

Public Function checkRevisionA(ValueString As String, Files() As String, mpqNumber As Long, Checksum As Long) As Boolean
    checkRevisionA = (checkRevision_Raw(ValueString, Files(0), Files(1), Files(2), mpqNumber, Checksum) > 0)
End Function

'EXE Information
'Information string (file name, date, time, and size) will be placed in InfoString.
'InfoString does NOT need to be initialized (e.g. InfoString = String$(255, vbNullChar))
'Returns the file version or 0 on failure.
Public Function getExeInfo(EXEFile As String, InfoString As String, Optional ByVal Platform As Long = BNCSUTIL_PLATFORM_WINDOWS) As Long
    Dim Version As Long, InfoSize As Long, Result As Long
    Dim i&
    InfoSize = 256
    InfoString = String$(256, vbNullChar)
    Result = getExeInfo_Raw(EXEFile, InfoString, InfoSize, Version, Platform)
    If Result = 0 Then
        getExeInfo = 0
        Exit Function
    End If
    While Result > InfoSize
        If InfoSize > 1024 Then
            getExeInfo = 0
            Exit Function
        End If
        InfoSize = InfoSize + 256
        InfoString = String$(InfoSize, vbNullChar)
        Result = getExeInfo_Raw(EXEFile, InfoString, InfoSize, Version, Platform)
    Wend
    getExeInfo = Version
    i = InStr(InfoString, vbNullChar)
    If i = 0 Then Exit Function
    InfoString = Left$(InfoString, i - 1)
End Function

'OLS Password Hashing
Public Function doubleHashPassword(Password As String, ByVal ClientToken&, ByVal ServerToken&) As String
    Dim Hash As String * 20
    doubleHashPassword_Raw Password, ClientToken, ServerToken, Hash
    doubleHashPassword = Hash
End Function

Public Function hashPassword(Password As String) As String
    Dim Hash As String * 20
    hashPassword_Raw Password, Hash
    hashPassword = Hash
End Function

Public Function nls_check_socket_signature(ByVal SocketHandle As Long, Signature As String) As Boolean
    Dim NameLen As Long, Name As sockaddr_in
    
    NameLen = 16
    getsockname SocketHandle, Name, NameLen
    
    nls_check_socket_signature = (nls_check_signature(Name.Address, Signature) <> 0)
End Function

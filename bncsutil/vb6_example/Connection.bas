Attribute VB_Name = "Connection"
Option Explicit

'-------------------------------------------------
'  Packet ID Constants
'-------------------------------------------------
Public Const SID_NULL& = &H0
Public Const SID_ENTERCHAT& = &HA
Public Const SID_JOINCHANNEL& = &HC
Public Const SID_CHATCOMMAND& = &HE
Public Const SID_CHATEVENT& = &HF
Public Const SID_UDPPINGRESPONSE& = &H14
Public Const SID_PING& = &H25
Public Const SID_LOGONRESPONSE& = &H29
Public Const SID_CREATEACCOUNT& = &H2A
Public Const SID_CHANGEPASSWORD& = &H31
Public Const SID_LOGONRESPONSE2& = &H3A
Public Const SID_CREATEACCOUNT2& = &H3D
Public Const SID_AUTH_INFO& = &H50
Public Const SID_AUTH_CHECK& = &H51
Public Const SID_AUTH_ACCOUNTCREATE& = &H52
Public Const SID_AUTH_ACCOUNTLOGON& = &H53
Public Const SID_AUTH_ACCOUNTLOGONPROOF& = &H54
Public Const SID_AUTH_ACCOUNTCHANGE& = &H55
Public Const SID_AUTH_ACCOUNTCHANGEPROOF& = &H56

'-------------------------------------------------
'  Product Constants
'-------------------------------------------------
Public Enum BNCSProducts
    STAR = &H53544152 ' in C: 'STAR'
    SEXP = &H53455850 ' in C: 'SEXP'
    W2BN = &H5732424E ' ... you get the idea
    D2DV = &H44324456
    D2XP = &H44325850
    WAR3 = &H57415233
    W3XP = &H57335850
    UNKN = &HFFFFFFFF ' unknown product
End Enum

Public Const ARCH_IX86 As Long = &H49583836

'-------------------------------------------------
'  Status
'-------------------------------------------------
Public IsConnected As Boolean

'-------------------------------------------------
'  Connection Variables
'-------------------------------------------------
Private ClientToken As Long, ServerToken As Long, UseNLS As Boolean, _
    NLS As Long, OldNLS As Long

'-------------------------------------------------
'  Misc. Windows API Declarations
'-------------------------------------------------
Public Declare Function GetTickCount Lib "kernel32.dll" () As Long


'-------------------------------------------------
'  Connector
'-------------------------------------------------
Public Sub Connect()
    ClientToken = GetTickCount()
    With frmMain
        .WS.Close
        .WS.RemoteHost = .txtServer
        .WS.RemotePort = 6112
        .WS.Connect
    End With
End Sub

'-------------------------------------------------
'  Disconnect
'-------------------------------------------------
Public Sub Disconnect()
    frmMain.WS.Close
    IsConnected = False
    
    ' Free NLS memory if still allocated.
    FreeNLS
    
    UpdateStatus "Disconnected."
    frmMain.cmdConnect.Caption = "Connect"
End Sub

Private Function HexByte(ByVal Number As Long) As String
    HexByte = Hex$(Number And &HFF)
    If Len(HexByte) = 1 Then _
        HexByte = "0" & HexByte
End Function

'-------------------------------------------------
'  First-Pass Data Handler
'-------------------------------------------------
'  Extracts all the packets from a particular
'  chunk of data, and then passes each packet to
'  the packet handler.
'-------------------------------------------------
Public Sub HandleData(Data As String, Length As Long)
    Dim Position As Long
    Dim P As Packet
    Dim PacketID As Byte
    Dim PacketLength As Long
    
    ' Initialize the packet buffer
    Set P = New Packet
    P.SetData Data
    
    Do While Position < Length
        ' All Battle.Net packets begin with a byte with a value
        ' of 0xFF.  Make sure that the first byte we read is
        ' 0xFF, because if it's not, it's a sign that something
        ' went wrong.
        If (P.GetByte() <> &HFF) Then
            UpdateStatus "Error: Invalid Battle.Net packet received.", stError
            Disconnect
            Exit Sub
        End If
        
        PacketID = P.GetByte()
        PacketLength = P.GetWORD()
        
        BNCSutil.DebugMessage "Incoming Packet 0x" & HexByte(PacketID) & " (Length = " & PacketLength & ")"
        BNCSutil.DebugHexDump Data
        
        Select Case PacketID
            Case SID_AUTH_INFO
                HandleAuthInfo P
            Case SID_PING
                HandlePing P
            Case SID_AUTH_CHECK
                HandleAuthCheck P
            Case SID_AUTH_ACCOUNTLOGON
                HandleAuthLogon P
            Case SID_AUTH_ACCOUNTLOGONPROOF
                HandleAuthLogonProof P
            Case SID_LOGONRESPONSE2
                HandleLogonResponse P
            Case SID_CHANGEPASSWORD
                HandleChangePassword P
            Case SID_CREATEACCOUNT2
                HandleCreateAccount P
            Case SID_AUTH_ACCOUNTCHANGE
                HandleAuthChange P
            Case SID_AUTH_ACCOUNTCHANGEPROOF
                HandleAuthChangeProof P
        End Select
        
        ' Advance position.
        Position = Position + PacketLength
    Loop
End Sub

'-------------------------------------------------
'  SID_AUTH_INFO (0x50) Sender
'-------------------------------------------------
Public Sub SendAuthInfo()
    Dim P As Packet, VerByte As Long, Client As Long
    Set P = New Packet
    
    ' Get client and check for success/failure.
    Client = GetClient()
    If (Client = 0) Then
        ' Failure.
        UpdateStatus "Invalid client.", stError
        Disconnect
        Exit Sub
    End If
    
    ' Determine version byte.
    Select Case Client
        Case STAR, SEXP
            VerByte = &HCD
        Case W2BN
            VerByte = &H4F
        Case D2DV, D2XP
            VerByte = &HB
        Case WAR3, W3XP
            VerByte = &H14
        Case Else
            ' Just in case.
            UpdateStatus "Unknown client (when determining version byte).", stError
            Disconnect
            Exit Sub
    End Select
    
    ' Build and send packet.
    With P
        .InsertDWORD &H0                'Protocol ID (always zero)
        .InsertDWORD ARCH_IX86          'Platform ID (Intel x86)
        .InsertDWORD Client             'Game Client
        .InsertDWORD VerByte            'Version byte
        .InsertDWORD &H0                'Product Language
        .InsertDWORD &H0                'Local IP for Network Address Translation
        .InsertDWORD &H0                'Time zone bias
        .InsertDWORD &H0                'Locale ID
        .InsertDWORD &H0                'Language ID
        .InsertString "USA"             'Country abbreviation
        .InsertString "United States"   'Country
        
        .Send frmMain.WS, SID_AUTH_INFO
    End With
End Sub

Public Function KeyCount(ByVal Product As BNCSProducts) As Long
    Select Case Product
        Case D2XP, W3XP: KeyCount = 2
        Case Else: KeyCount = 1
    End Select
End Function

'-------------------------------------------------
'  SID_AUTH_INFO (0x50) Handler
'-------------------------------------------------
Private Sub HandleAuthInfo(P As Packet)
    Dim mpqNumber As Long, ChecksumFormula As String
    Dim EXEVersion As Long, Checksum As Long, EXEInfo As String
    Dim HashFiles(2) As String
    Dim PublicValue As Long, ProductValue As Long
    Dim KeyHash As String * 20
    Dim ServerSignature As String * 128
    Dim NumberOfKeys As Long
    Dim i As Long
    
    Dim O As Packet
    Set O = New Packet

    Select Case P.GetDWORD()
        ' Logon type
        Case 0
            'Old Logon System
            UseNLS = False
        Case 1
            'W3 Beta NLS
            Disconnect
            MsgBox "The NLS revision that the server has requested " & _
                "is not supported.", vbExclamation, "Example Bot"
            Exit Sub
        Case 2
            'New Logon System
            UseNLS = True
        Case Else
            'Who knows?
            Disconnect
            MsgBox "Unsupported logon system.", vbExclamation, "Example Bot"
            Exit Sub
    End Select
    
    ServerToken = P.GetDWORD()
    P.Skip 12 'Skip UDPvalue and MPQ filetime.
    mpqNumber = extractMPQNumber(P.GetString())
    If (mpqNumber < 0) Then
        Disconnect
        MsgBox "Unrecognized MPQ number.", vbExclamation, "Example Bot"
        Exit Sub
    End If
    
    ChecksumFormula = P.GetString()
    
    ' Check server signature.
    ' (Appears broken?)
    'If (UseNLS) Then
    '    ServerSignature = P.GetFixedString(128)
    '    If (Not nls_check_socket_signature(frmMain.WS.SocketHandle, ServerSignature)) Then
    '        Disconnect
    '        MsgBox "Server signature check failed.", vbExclamation, "Example Bot"
    '    End If
    'End If
    
    ' Get hash file paths.
    GetHashFiles HashFiles
    
    ' Perform revision check operations.
    If (checkRevision(ChecksumFormula, HashFiles(0), HashFiles(1), _
    HashFiles(2), mpqNumber, Checksum) = False) Then
        Disconnect
        MsgBox "CheckRevision failed.", vbExclamation, "Example Bot"
        Exit Sub
    End If
    
    ' Get EXE version info.
    EXEVersion = getExeInfo(HashFiles(0), EXEInfo)
    If (EXEVersion = 0) Then
        Disconnect
        MsgBox "Failed to get executable hash file information.", _
            vbExclamation, "Example Bot"
        Exit Sub
    End If
    
    ' Build SID_AUTH_CHECK (0x51) packet.
    ' (We'll do the CD-key stuff along the way.)
    With O
        .InsertDWORD ClientToken
        .InsertDWORD EXEVersion
        .InsertDWORD Checksum
        
        ' Number of keys in packet:
        NumberOfKeys = KeyCount(GetClient())
        .InsertDWORD NumberOfKeys

        .InsertDWORD 0 ' Not using spawn.
        
        ' Decode the CD-keys and calculate their hashes.
        For i = 0 To (NumberOfKeys - 1)
            If kd_quick(frmMain.txtCDKey(i).Text, ClientToken, ServerToken, PublicValue, ProductValue, KeyHash, 20) = 0 Then
                Disconnect
                MsgBox "Failed to decode CD key #" & (i + 1) & ".", vbExclamation, "Example Bot"
                Exit Sub
            End If
            
            .InsertDWORD Len(frmMain.txtCDKey(i).Text)
            .InsertDWORD ProductValue
            .InsertDWORD PublicValue
            .InsertDWORD 0
            .InsertNonNTString KeyHash
        Next i
        
        ' Finishing touches.
        .InsertString EXEInfo
        .InsertString "BNCSutil Example" ' Key owner.
        
        .Send frmMain.WS, SID_AUTH_CHECK
    End With
End Sub

'-------------------------------------------------
'  SID_PING (0x25) Handler
'-------------------------------------------------
Public Sub HandlePing(P As Packet)
    Dim O As Packet
    Set O = New Packet
    
    O.InsertDWORD P.GetDWORD()
    O.Send frmMain.WS, SID_PING
End Sub

'-------------------------------------------------
'  SID_AUTH_CHECK (0x51) Handler
'-------------------------------------------------
Public Sub HandleAuthCheck(P As Packet)
    Dim strTmp As String
    
    Select Case P.GetDWORD() 'Result
        Case &H0
            ' OK.
        Case &H100
            Disconnect
            MsgBox "Game version is out of date.  Wait for a BNLS update.", vbExclamation, "Example Bot"
            Exit Sub
        Case &H101
            Disconnect
            MsgBox "Invalid game version.", vbExclamation, "Example Bot"
            Exit Sub
        Case &H200
            Disconnect
            MsgBox "Invalid CD-Key.", vbExclamation, "Example Bot"
            Exit Sub
        Case &H201
            Disconnect
            strTmp = P.GetString()
            If LenB(strTmp) > 0 Then
                MsgBox "CD-Key in use by " & strTmp & ".", vbExclamation, "Example Bot"
            Else
                MsgBox "CD-Key in use.", vbExclamation, "Example Bot"
            End If
            Exit Sub
        Case &H202
            Disconnect
            MsgBox "CD-Key is banned from Battle.Net.", vbExclamation, "Example Bot"
            Exit Sub
        Case &H203
            Disconnect
            MsgBox "CD-Key is for another game.", vbExclamation, "Example Bot"
            Exit Sub
        Case &H210
            Disconnect
            MsgBox "Invalid Expansion CD-Key.", vbExclamation, "Example Bot"
            Exit Sub
        Case &H211
            Disconnect
            strTmp = P.GetString()
            If LenB(strTmp) > 0 Then
                MsgBox "Expansion CD-Key in use by " & strTmp & ".", vbExclamation, "Example Bot"
            Else
                MsgBox "Expansion CD-Key in use.", vbExclamation, "Example Bot"
            End If
            Exit Sub
        Case &H212
            Disconnect
            MsgBox "Expansion CD-Key is banned from Battle.Net.", vbExclamation, "Example Bot"
            Exit Sub
        Case &H213
            Disconnect
            MsgBox "Expansion CD-Key is for another game.", vbExclamation, "Example Bot"
            Exit Sub
        Case Else
            Disconnect
            MsgBox "SID_AUTH_CHECK failed for an unknown reason.", vbExclamation, "Example Bot"
    End Select
    
    If frmMain.chkChangePassword.Value = vbChecked Then
        If UseNLS Then
            SendAuthChange
        Else
            SendChangePassword
        End If
    Else
        If UseNLS Then
            SendAuthLogon
        Else
            SendLogonResponse
        End If
    End If
End Sub

'-------------------------------------------------
'  SID_AUTH_ACCOUNTLOGON (0x53) Sender
'-------------------------------------------------
Public Sub SendAuthLogon()
    Dim P As Packet
    Dim Var_A As String * 32 'note the * 32, this creates a
                             'fixed-length string 32 chars long
    Set P = New Packet
    
    ' The NLS variable is declared at the top of this module.
    ' Its value **MUST** remain available throughout all of the
    ' SID_AUTH_ACCOUNT* packets, which is why it's not declared
    ' in this sub.  If you don't understatnd this, look into
    ' the concept of "variable scope".
    '
    ' Note that the initialization is only done if NLS doesn't already have
    ' a value.  This is because it will otherwise already have been created
    ' in the password change process.
    If (NLS = 0) Then
        NLS = nls_init(frmMain.txtUsername.Text, frmMain.txtPassword.Text)
        If (NLS = 0) Then
            Disconnect
            MsgBox "Failed to initialize NLS.", vbExclamation, "Example Bot"
            Exit Sub
        End If
    End If
    
    ' Retrieve "public ephermeral value" (A).
    Call nls_get_A(NLS, Var_A)
    
    ' Build packet.
    With P
        .InsertNonNTString Var_A
        .InsertString frmMain.txtUsername.Text
        
        ' Send away!
        .Send frmMain.WS, SID_AUTH_ACCOUNTLOGON
    End With
End Sub

'-------------------------------------------------
'  SID_AUTH_ACCOUNTLOGON (0x53) Handler
'-------------------------------------------------
Public Sub HandleAuthLogon(P As Packet)
    Dim M1 As String * 20
    Dim Salt As String, Var_B As String
    Dim O As Packet

    ' Examine result code.
    Select Case P.GetDWORD()
        Case 0
            ' Logon accepted
        Case 1
            ' Account doesn't exist.
            ShowAccountCreate
            'If MsgBox("Account doesn't exist.  Create account?", vbQuestion Or vbYesNo, "Example Bot") = vbYes Then
            '    SendAuthCreate
            'Else
            '    Disconnect
            'End If
            Exit Sub
        Case 5
            ' Account requires upgrade.
            Disconnect
            MsgBox "Account requires upgrade.", vbExclamation, "Example Bot"
            Exit Sub
        Case Else
            ' ??
            Disconnect
            MsgBox "Unknown logon result.", vbExclamation, "Example Bot"
            Exit Sub
    End Select
    
    ' Build SID_AUTH_ACCOUNTLOGONPROOF (0x54) packet.
    Set O = New Packet
    With P
        Salt = .GetFixedString(32)
        Var_B = .GetFixedString(32)
    End With
    
    ' Get password proof.
    Call nls_get_M1(NLS, M1, Var_B, Salt)
    
    With O
        .InsertNonNTString M1
        .Send frmMain.WS, SID_AUTH_ACCOUNTLOGONPROOF
    End With
End Sub

'-------------------------------------------------
'  SID_AUTH_ACCOUNTLOGONPROOF (0x54) Handler
'-------------------------------------------------
Public Sub HandleAuthLogonProof(P As Packet)
    ' Examine result.
    Dim M2 As String
    Select Case P.GetDWORD()
        Case 0
            ' Check server password proof.
            M2 = P.GetFixedString(20)
            If (nls_check_M2(NLS, M2, vbNullString, vbNullString) = 0) Then
                Disconnect
                MsgBox "Server password proof check failed!", vbExclamation, "Example Bot"
                Exit Sub
            End If
            
            FreeNLS ' free NLS memory
            UpdateStatus "Logged in!"
        Case 2
            Disconnect
            MsgBox "Incorrect password.", vbExclamation, "Example Bot"
            Exit Sub
        Case &HE
            Disconnect
            MsgBox "E-mail registration required.", vbExclamation, "Example Bot"
            Exit Sub
        Case Else
            Disconnect
            MsgBox "Unknown logon proof error.", vbExclamation, "Example Bot"
            Exit Sub
    End Select
End Sub

'-------------------------------------------------
'  SID_AUTH_ACCOUNTCREATE (0x52) Sender
'-------------------------------------------------
Public Sub SendAuthCreate()
    Dim Buffer As String, BufLen As Long
    Dim P As Packet
    Set P = New Packet
    
    ' This one is pretty easy.
    BufLen = 65 + Len(frmMain.txtUsername.Text)
    Buffer = String$(BufLen, vbNullChar) ' Initialize buffer
    If (nls_account_create(NLS, Buffer, BufLen) = 0) Then
        Disconnect
        MsgBox "Failed to generate account create packet."
        Exit Sub
    End If
    
    With P
        .InsertNonNTString Buffer ' Buffer contains entire packet body.
        .Send frmMain.WS, SID_AUTH_ACCOUNTCREATE
    End With
End Sub

'-------------------------------------------------
'  SID_AUTH_ACCOUNTCREATE (0x52) Handler
'-------------------------------------------------
Public Sub HandleAuthCreate(P As Packet)
    ' Examine result.
    Select Case P.GetDWORD()
        Case 0
            UpdateStatus "Created account."
        Case 7, 8, 9, &HA, &HB, &HC
            Disconnect
            MsgBox "Invalid username.", vbExclamation, "Example Bot"
            Exit Sub
        Case Else
            Disconnect
            MsgBox "Account already exists.", vbExclamation, "Example Bot"
            Exit Sub
    End Select
    
    ' Now try logging in again.
    SendAuthLogon
End Sub

'-------------------------------------------------
'  SID_LOGONRESPONSE2 (0x3A) Sender
'-------------------------------------------------
Public Sub SendLogonResponse()
    Dim P As Packet
    Set P = New Packet
    
    With P
        .InsertDWORD ClientToken
        .InsertDWORD ServerToken
        .InsertNonNTString doubleHashPassword(frmMain.txtPassword.Text, ClientToken, ServerToken)
        .InsertString frmMain.txtUsername.Text
        
        .Send frmMain.WS, SID_LOGONRESPONSE2
    End With
End Sub

'-------------------------------------------------
'  SID_LOGONRESPONSE2 (0x3A) Handler
'-------------------------------------------------
Public Sub HandleLogonResponse(P As Packet)
    ' Examine result.
    Select Case P.GetDWORD()
        Case 0
            ' Success
            UpdateStatus "Logged in!"
        Case 1
            ShowAccountCreate
            'If (MsgBox("Account does not exist.  Create account?", vbQuestion Or vbYesNo, "Example Bot") = vbYes) Then
            '    ' Create account.
            '    SendCreateAccount
            'Else
            '    Disconnect
            'End If
            Exit Sub
        Case 2
            Disconnect
            MsgBox "Incorrect password.", vbExclamation, "Example Bot"
            Exit Sub
    End Select
End Sub

'-------------------------------------------------
'  SID_CREATEACCOUNT2 (0x3D) Sender
'-------------------------------------------------
Public Sub SendCreateAccount()
    Dim P As Packet
    Set P = New Packet
    
    With P
        .InsertNonNTString hashPassword(frmMain.txtPassword.Text)
        .InsertString frmMain.txtUsername.Text
        
        .Send frmMain.WS, SID_CREATEACCOUNT2
    End With
End Sub

'-------------------------------------------------
'  SID_CREATEACCOUNT2 (0x3D) Handler
'-------------------------------------------------
Public Sub HandleCreateAccount(P As Packet)
    ' Examine result.
    Select Case P.GetDWORD()
        Case 0
            ' Success
            UpdateStatus "Created account."
        Case 2
            Disconnect
            MsgBox "Username contains invalid characters.", vbExclamation, "Example Bot"
            Exit Sub
        Case 3
            Disconnect
            MsgBox "Username contains banned word(s).", vbExclamation, "Example Bot"
            Exit Sub
        Case 4
            Disconnect
            MsgBox "Account already exists.", vbExclamation, "Example Bot"
            Exit Sub
        Case 6
            Disconnect
            MsgBox "Username is too short.", vbExclamation, "Example Bot"
            Exit Sub
        Case Else
            Disconnect
            MsgBox "Unknown account creation result.", vbExclamation, "Example Bot"
            Exit Sub
    End Select
    
    ' Try logging in again, now that the account was created.
    SendLogonResponse
End Sub

'-------------------------------------------------
'  SID_CHANGEPASSWORD (0x31) Sender
'-------------------------------------------------
Public Sub SendChangePassword()
    Dim P As Packet
    Set P = New Packet
    
    With P
        .InsertDWORD ClientToken
        .InsertDWORD ServerToken
        .InsertNonNTString doubleHashPassword(frmMain.txtPassword.Text, ClientToken, ServerToken)
        .InsertNonNTString hashPassword(frmMain.txtNewPassword.Text)
        .InsertString frmMain.txtUsername.Text
        
        .Send frmMain.WS, SID_CHANGEPASSWORD
    End With
End Sub

'-------------------------------------------------
'  SID_CHANGEPASSWORD (0x31) Handler
'-------------------------------------------------
Public Sub HandleChangePassword(P As Packet)
    If (P.GetDWORD()) Then
        MsgBox "Password changed.", vbInformation, "BNCSutil Example Bot"
        Disconnect
    Else
        Disconnect
        UpdateStatus "Failed to change password.", stError
    End If
End Sub

'-------------------------------------------------
'  SID_AUTH_ACCOUNTCHANGE (0x55) Sender
'-------------------------------------------------
Public Sub SendAuthChange()
    Dim P As Packet
    Set P = New Packet
    Dim A As String * 32
    
    NLS = nls_init(frmMain.txtUsername.Text, frmMain.txtPassword.Text)
    If (NLS = 0) Then
        Disconnect
        MsgBox "Failed to initialize NLS.", vbExclamation, "Example Bot"
        Exit Sub
    End If
    
    With P
        Call nls_get_A(NLS, A)
        .InsertNonNTString A
        .InsertString frmMain.txtUsername.Text
        .Send frmMain.WS, SID_AUTH_ACCOUNTCHANGE
    End With
End Sub

'-------------------------------------------------
'  SID_AUTH_ACCOUNTCHANGE (0x55) Handler
'-------------------------------------------------
Public Sub HandleAuthChange(P As Packet)
    Dim ServerKey As String, Salt As String, NewPW As String
    Dim NewNLS As Long, OutputBuffer As String * 84
    Dim O As Packet
    
    Select Case P.GetDWORD()
        Case 0
            'ok
        Case 1
            Disconnect
            MsgBox "Account does not exist!", vbExclamation, "Example Bot"
            Exit Sub
        Case 5
            Disconnect
            MsgBox "Account requires upgrade.", vbExclamation, "Example Bot"
            Exit Sub
        Case Else
            Disconnect
            MsgBox "Unknown SID_AUTH_ACCOUNTCHANGE response.", vbExclamation, "Example Bot"
            Exit Sub
    End Select
    
    Salt = P.GetFixedString(32)
    ServerKey = P.GetFixedString(32)
    
    NewPW = frmMain.txtNewPassword.Text
    NewNLS = nls_account_change_proof(NLS, OutputBuffer, NewPW, ServerKey, Salt)
    
    If (NewNLS = 0) Then
        Disconnect
        MsgBox "Failed to generate SID_AUTH_ACCOUNTCHANGEPROOF packet.", vbExclamation, "Example Bot"
        Exit Sub
    End If
    
    OldNLS = NLS
    NLS = NewNLS
        
    Set O = New Packet
    O.InsertNonNTString OutputBuffer
    O.Send frmMain.WS, SID_AUTH_ACCOUNTCHANGEPROOF
End Sub

'-------------------------------------------------
'  SID_AUTH_ACCOUNTCHANGEPROOF (0x56) Handler
'-------------------------------------------------
Public Sub HandleAuthChangeProof(P As Packet)
    Dim M2 As String
    Select Case P.GetDWORD()
        Case 0
            ' Check server password proof.
            M2 = P.GetFixedString(20)
            If (nls_check_M2(OldNLS, M2, vbNullString, vbNullString) = 0) Then
                '            note the OldNLS being used
                Disconnect
                MsgBox "Server password proof check failed!", vbExclamation, "Example Bot"
            End If
            
            ' Free NLS memory for old password.
            Call nls_free(OldNLS)
            OldNLS = 0
            
            UpdateStatus "Password changed."
            SendAuthLogon ' log in
        Case 2
            Disconnect
            MsgBox "Incorrect password!", vbExclamation, "Example Bot"
        Case Else
            Disconnect
            MsgBox "Unknown SID_AUTH_ACCOUNTCHANGEPROOF result code received.", vbExclamation, "Example Bot"
    End Select
End Sub

'-------------------------------------------------
'  Account Create Request Accepted
'-------------------------------------------------
'  Called by frmNewAccount when the user presses
'  the "Create Account" button and the confirmed
'  password matches.
'-------------------------------------------------
Public Sub AccountCreateAccepted()
    If UseNLS Then
        SendAuthCreate
    Else
        SendCreateAccount
    End If
End Sub

'-------------------------------------------------
'  Hash File Paths
'-------------------------------------------------
Public Sub GetHashFiles(Files() As String)
    Select Case GetClient()
        Case STAR, SEXP
            Files(0) = frmMain.txtHashFolder.Text & "\starcraft.exe"
            Files(1) = frmMain.txtHashFolder.Text & "\storm.dll"
            Files(2) = frmMain.txtHashFolder.Text & "\battle.snp"
        Case W2BN
            Files(0) = frmMain.txtHashFolder.Text & "\WarCraft II BNE.exe"
            Files(1) = frmMain.txtHashFolder.Text & "\storm.dll"
            Files(2) = frmMain.txtHashFolder.Text & "\battle.snp"
        Case D2DV, D2XP
            Files(0) = frmMain.txtHashFolder.Text & "\Game.exe"
            Files(1) = frmMain.txtHashFolder.Text & "\Bnclient.dll"
            Files(2) = frmMain.txtHashFolder.Text & "\D2Client.dll"
        Case WAR3, W3XP
            Files(0) = frmMain.txtHashFolder.Text & "\war3.exe"
            Files(1) = frmMain.txtHashFolder.Text & "\Storm.dll"
            Files(2) = frmMain.txtHashFolder.Text & "\Game.dll"
    End Select
End Sub

Private Sub FreeNLS()
    If (NLS <> 0) Then
        Call nls_free(NLS)
        NLS = 0
    End If
    
    If (OldNLS <> 0) Then
        Call nls_free(OldNLS)
        OldNLS = 0
    End If
End Sub

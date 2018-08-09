VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "Packet"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit
Private Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" _
    (ByRef Destination As Any, ByRef Source As Any, ByVal numbytes As Long)

Private Enum PacketType
    pIncoming
    pOutgoing
    pNull
End Enum

Public Enum PacketFormat
    pBnet
    pBNLS
    pMCP
    pNone
    pMatrix
End Enum

Private Const EOP_ERROR& = vbObjectError + 1201

Private RawData As String
Private Mode As PacketType
Public Position As Long
Public RaiseOverflowErrors As Boolean
Private Const RAISE_OVERFLOW_ERRORS_DEFAULT As Boolean = False

Public Property Get EOPError() As Long
    EOPError = EOP_ERROR
End Property

Public Sub Reset()
    Mode = pNull
    RawData = ""
    Position = 1
End Sub

Public Function SetData(Data As String)
    RawData = Data
    Mode = pIncoming
End Function

Public Function GetData() As String
    GetData = RawData
    Reset
End Function

Public Sub Skip(Bytes As Long)
    Position = Position + Bytes
End Sub

Public Function GetString() As String
    Dim NTPos&, strTemp$
    GetString = ""
    NTPos = InStr(Position, RawData, vbNullChar)
    If NTPos = 0 Then
        If RaiseOverflowErrors Then _
            Err.Raise EOP_ERROR, , "Trying to read past end of packet."
        Exit Function
    End If
    GetString = Mid$(RawData, Position, NTPos - Position)
    Position = NTPos + 1
End Function

Public Function GetFixedString(Length As Long) As String
    Dim rVal&
    If LenData() - Position + 1 >= Length Then
        GetFixedString = Mid$(RawData, Position, Length)
        Position = Position + Length
    Else
        If RaiseOverflowErrors Then _
            Err.Raise EOP_ERROR, , "Trying to read past end of packet."
        Exit Function
    End If
End Function

Public Function GetBoolean(Optional ByVal Bytes As Long = 4) As Boolean
    Dim rVal&
    If LenData() - Position + 1 >= Bytes Then
        CopyMemory rVal, ByVal Mid$(RawData, Position, Bytes), Bytes
        Position = Position + Bytes
    Else
        If RaiseOverflowErrors Then _
            Err.Raise EOP_ERROR, , "Trying to read past end of packet."
        Exit Function
    End If
    If rVal = 0 Then
        GetBoolean = False
    Else
        GetBoolean = True
    End If
End Function

Public Function GetDWORD() As Long
    Dim rVal&
    If LenData() - Position + 1 >= 4 Then
        CopyMemory rVal, ByVal Mid$(RawData, Position, 4), 4
        Position = Position + 4
        GetDWORD = rVal
        'BotEvent "", evDbg, Position & ":" & StrToHex(Mid$(RawData, Position))
    Else
        If RaiseOverflowErrors Then _
            Err.Raise EOP_ERROR, , "Trying to read past end of packet."
        Exit Function
    End If
End Function

Public Function GetDWORDArray(Length As Long) As Long()
    If Length = 0 Then Exit Function
    Dim oS() As Long, i&
    ReDim oS(Length) As Long
    For i = 0 To Length - 1
        oS(i) = GetDWORD()
    Next i
    GetDWORDArray = oS
End Function

Public Function GetWORD() As Integer
    Dim rVal%
    If LenData() - Position + 1 >= 2 Then
        CopyMemory rVal, ByVal Mid$(RawData, Position, 2), 2
        Position = Position + 2
        GetWORD = rVal
    Else
        If RaiseOverflowErrors Then _
            Err.Raise EOP_ERROR, , "Trying to read past end of packet."
        Exit Function
    End If
End Function

Public Function GetWORDArray(Length As Long) As Integer()
    If Length = 0 Then Exit Function
    Dim oS() As Integer, i&
    ReDim oS(Length) As Integer
    For i = 0 To Length - 1
        oS(i) = GetWORD()
    Next i
    GetWORDArray = oS
End Function

Public Function GetByte() As Byte
    Dim rVal As Byte
    rVal = 0
    If LenData() - Position + 1 >= 1 Then
        CopyMemory rVal, ByVal Mid$(RawData, Position, 1), 1
        Position = Position + 1
        GetByte = rVal
    Else
        If RaiseOverflowErrors Then _
            Err.Raise EOP_ERROR, , "Trying to read past end of packet."
        GetByte = 0
    End If
End Function

Public Function GetStringArray(ByVal Length As Long) As String()
    If Length = 0 Then Exit Function
    Dim oS() As String, i&
    ReDim oS(Length) As String
    For i = 0 To Length - 1
        oS(i) = GetString()
    Next i
    GetStringArray = oS
End Function

Public Function LenData() As Long
    LenData = Len(RawData)
End Function

'Used with Winsock.bas
'Public Function Send(Socket As Long, Optional PacketID As Byte, Optional ByVal Format As PacketFormat = pBnet)
'    If Mode = pIncoming Then Exit Function
'    Select Case Format
'        Case pBnet
'            SendData Socket, Chr$(&HFF) & MakeByte(PacketID) & _
'                MakeWORD(Len(RawData) + 4) & RawData
'        Case pBNLS
'            SendData Socket, MakeWORD(Len(RawData) + 3) & MakeByte(PacketID) & RawData
'        Case pMCP
'            SendData Socket, MakeWORD(Len(RawData) + 3) & MakeByte(PacketID) & RawData
'        Case Else
'            SendData Socket, RawData
'    End Select
'    Reset
'End Function

Private Function HexByte(ByVal Number As Long) As String
    HexByte = Hex$(Number And &HFF)
    If Len(HexByte) = 1 Then _
        HexByte = "0" & HexByte
End Function

'Used with Winsock controls
Public Sub Send(Socket As Winsock, Optional ByVal PacketID As Byte, Optional ByVal Format As PacketFormat = pBnet)
On Error GoTo Trap
    Dim Data As String
    
    If Mode = pIncoming Then Exit Sub
    Select Case Format
        Case pBnet
            Data = Chr$(&HFF) & Chr$(PacketID) & _
                MakeWORD(Len(RawData) + 4) & RawData
        Case pBNLS, pMCP
            Data = MakeWORD(Len(RawData) + 3) & Chr$(PacketID) & RawData
        Case pMatrix
            Data = MakeByte(PacketID) & Chr$(Len(RawData) + 3) & RawData
        Case Else
            Data = RawData
    End Select
    
    BNCSutil.DebugMessage "Outgoing Packet 0x" & HexByte(PacketID) & " (Length = " & Len(Data) & ")"
    BNCSutil.DebugHexDump Data
    
    Socket.SendData Data
    Reset
    Exit Sub
Trap:
On Error GoTo Trap2
    Disconnect
Trap2:
End Sub

Public Sub InsertString(Data As String)
    If Mode = pIncoming Then Exit Sub
    Mode = pOutgoing
    RawData = RawData & Data & Chr$(0)
End Sub

Public Sub InsertStringList(Data() As String)
'<RAGE>On Error Resume Next
    If Mode = pIncoming Then Exit Sub
    Mode = pOutgoing
    Dim i As Integer
    For i = LBound(Data) To UBound(Data) Step 1
        RawData = RawData & Data(i) & Chr$(0)
    Next i
    RawData = RawData & Chr$(0) 'extra null byte to end stringlist
End Sub

Public Sub InsertStringArray(Data() As String)
'<RAGE>On Error Resume Next
    If Mode = pIncoming Then Exit Sub
    Mode = pOutgoing
    Dim i As Integer
    For i = LBound(Data) To UBound(Data) Step 1
        RawData = RawData & Data(i) & Chr$(0)
    Next i
    'no extra null byte in string arrays
End Sub

Public Sub InsertNTString(Data As String)
    InsertString Data
End Sub

Public Sub InsertNonNTString(Data As String)
    If Mode = pIncoming Then Exit Sub
    Mode = pOutgoing
    RawData = RawData & Data
End Sub

Public Sub InsertByte(Data As Byte)
    If Mode = pIncoming Then Exit Sub
    Mode = pOutgoing
    
    RawData = RawData & MakeByte(Data)
End Sub

Public Sub InsertWORD(Data As Integer)
    If Mode = pIncoming Then Exit Sub
    Mode = pOutgoing
    
    RawData = RawData & MakeWORD(Data)
End Sub

Public Sub InsertDWORD(Data As Long)
    If Mode = pIncoming Then Exit Sub
    Mode = pOutgoing
    
    RawData = RawData & MakeDWORD(Data)
End Sub

Public Sub InsertDWORDArray(Data() As Long)
    If Mode = pIncoming Then Exit Sub
    Mode = pOutgoing
    
    Dim i As Integer
    For i = LBound(Data) To UBound(Data) Step 1
        RawData = RawData & MakeDWORD(Data(i))
    Next i
End Sub

Private Function MakeByte(Data As Byte) As String
    Dim Result As String * 1
    CopyMemory ByVal Result, Data, 1
    MakeByte = Result
End Function

Private Function MakeWORD(Data As Integer) As String
    Dim Result As String * 2
    CopyMemory ByVal Result, Data, 2
    MakeWORD = Result
End Function

Public Function MakeDWORD(Data As Long) As String
    Dim Result As String * 4
    CopyMemory ByVal Result, Data, 4
    MakeDWORD = Result
End Function

Private Sub Class_Initialize()
    Position = 1
    Mode = pNull
    RaiseOverflowErrors = RAISE_OVERFLOW_ERRORS_DEFAULT
End Sub

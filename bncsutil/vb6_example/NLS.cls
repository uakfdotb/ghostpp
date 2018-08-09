VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "NLS"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Attribute VB_Ext_KEY = "SavedWithClassBuilder6" ,"Yes"
Attribute VB_Ext_KEY = "Top_Level" ,"Yes"
Option Explicit

' NLS Wrapper Class

Private m_Username As String
Private m_Password As String
Private m_NLS As Long

Private m_ServerKey As String
Private m_Salt As String

Private Sub Class_Terminate()
On Error GoTo Trap
    If (m_NLS) Then _
        nls_free m_NLS
Trap:
End Sub

Public Property Get Username() As String
    Username = m_Username
End Property

Public Property Let Username(NewValue As String)
    m_Username = NewValue
End Property

Public Property Get Password() As String
    Password = m_Password
End Property

Public Property Let Password(NewValue As String)
    m_Password = NewValue
End Property

Public Property Get ServerKey() As String
    ServerKey = m_ServerKey
End Property

Public Property Let ServerKey(NewValue As String)
    m_ServerKey = NewValue
End Property

Public Property Get Salt() As String
    Salt = m_Salt
End Property

Public Property Let Salt(NewValue As String)
    m_Salt = NewValue
End Property

Public Property Get Handle() As Long
    If (m_NLS = 0 And LenB(m_Username) > 0 And LenB(m_Password) > 0) Then
        m_NLS = nls_init(m_Username, m_Password)
    End If
    Handle = m_NLS
End Property

Public Property Let Handle(ByVal NewValue As Long)
    If (m_NLS) Then _
        nls_free m_NLS
    m_NLS = NewValue
End Property

Public Property Get PublicKey() As String
    PublicKey = String$(32, vbNullChar)
    nls_get_A Handle, PublicKey
End Property

Public Property Get ClientPasswordProof() As String
    ClientPasswordProof = String$(20, vbNullChar)
    nls_get_M1 Handle, ClientPasswordProof, m_ServerKey, m_Salt
End Property

Public Function VerifyServerProof(ServerProof As String) As Boolean
    If (nls_check_M2(Handle, ServerProof, vbNullString, vbNullString)) Then
        VerifyServerProof = True
    Else
        VerifyServerProof = False
    End If
End Function

Public Property Get CreateAccount() As Buffer
    Dim CreateBuf As String, DataLen As Long
    
    DataLen = Len(m_Username) + 65
    CreateBuf = String$(DataLen, vbNullChar)
    Set CreateAccount = New Buffer
    With CreateAccount
        nls_account_create Handle, CreateBuf, DataLen
        .AddRaw CreateBuf
    End With
End Property

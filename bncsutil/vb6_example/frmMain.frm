VERSION 5.00
Object = "{248DD890-BB45-11CF-9ABC-0080C7E7B78D}#1.0#0"; "MSWINSCK.OCX"
Begin VB.Form frmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "BNCSutil Example"
   ClientHeight    =   6330
   ClientLeft      =   45
   ClientTop       =   360
   ClientWidth     =   5070
   BeginProperty Font 
      Name            =   "Tahoma"
      Size            =   8.25
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   6330
   ScaleWidth      =   5070
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame fraStatus 
      Caption         =   "Status"
      Height          =   855
      Left            =   120
      TabIndex        =   20
      Top             =   4200
      Width           =   4815
      Begin VB.Label lblStatus 
         Alignment       =   2  'Center
         Height          =   255
         Left            =   120
         TabIndex        =   21
         Top             =   360
         Width           =   4575
      End
   End
   Begin VB.Frame fraConfig 
      Caption         =   "Configuration"
      Height          =   3375
      Left            =   120
      TabIndex        =   4
      Top             =   600
      Width           =   4815
      Begin VB.TextBox txtNewPassword2 
         Height          =   285
         IMEMode         =   3  'DISABLE
         Left            =   1440
         PasswordChar    =   "*"
         TabIndex        =   25
         Top             =   3720
         Width           =   2535
      End
      Begin VB.TextBox txtNewPassword 
         Height          =   285
         IMEMode         =   3  'DISABLE
         Left            =   1440
         PasswordChar    =   "*"
         TabIndex        =   23
         Top             =   3360
         Width           =   2535
      End
      Begin VB.CheckBox chkChangePassword 
         Caption         =   "Change password when connected."
         Height          =   255
         Left            =   120
         TabIndex        =   22
         Top             =   3000
         Width           =   3735
      End
      Begin VB.CommandButton cmdBrowseHashes 
         Caption         =   "Browse"
         Height          =   255
         Left            =   3960
         TabIndex        =   19
         Top             =   2520
         Width           =   735
      End
      Begin VB.TextBox txtHashFolder 
         Height          =   285
         Left            =   1320
         TabIndex        =   18
         Top             =   2520
         Width           =   2535
      End
      Begin VB.TextBox txtServer 
         Height          =   285
         Left            =   1320
         TabIndex        =   14
         Top             =   1800
         Width           =   2535
      End
      Begin VB.ComboBox cmbClient 
         Height          =   315
         Left            =   1320
         Style           =   2  'Dropdown List
         TabIndex        =   16
         Top             =   2160
         Width           =   2535
      End
      Begin VB.TextBox txtCDKey 
         Height          =   285
         Index           =   1
         Left            =   1320
         TabIndex        =   12
         Top             =   1440
         Width           =   2535
      End
      Begin VB.TextBox txtCDKey 
         Height          =   285
         Index           =   0
         Left            =   1320
         TabIndex        =   10
         Top             =   1080
         Width           =   2535
      End
      Begin VB.TextBox txtPassword 
         Height          =   285
         IMEMode         =   3  'DISABLE
         Left            =   1320
         PasswordChar    =   "*"
         TabIndex        =   8
         Top             =   720
         Width           =   2535
      End
      Begin VB.TextBox txtUsername 
         Height          =   285
         Left            =   1320
         TabIndex        =   6
         Top             =   360
         Width           =   2535
      End
      Begin VB.Label lblNewPassword2 
         Caption         =   "Confirm:"
         Height          =   255
         Left            =   120
         TabIndex        =   26
         Top             =   3750
         Width           =   975
      End
      Begin VB.Label lblNewPassword 
         Caption         =   "New Password:"
         Height          =   255
         Left            =   120
         TabIndex        =   24
         Top             =   3390
         Width           =   1215
      End
      Begin VB.Label Label9 
         Caption         =   "Hash Folder:"
         Height          =   255
         Left            =   120
         TabIndex        =   17
         Top             =   2550
         Width           =   975
      End
      Begin VB.Label Label8 
         Caption         =   "Server:"
         Height          =   255
         Left            =   120
         TabIndex        =   13
         Top             =   1830
         Width           =   975
      End
      Begin VB.Label Label7 
         Caption         =   "Client:"
         Height          =   255
         Left            =   120
         TabIndex        =   15
         Top             =   2220
         Width           =   975
      End
      Begin VB.Label Label5 
         Caption         =   "Expansion Key:"
         Height          =   255
         Left            =   120
         TabIndex        =   11
         Top             =   1470
         Width           =   1215
      End
      Begin VB.Label Label4 
         Caption         =   "CD-Key:"
         Height          =   255
         Left            =   120
         TabIndex        =   9
         Top             =   1110
         Width           =   975
      End
      Begin VB.Label Label3 
         Caption         =   "Password:"
         Height          =   255
         Left            =   120
         TabIndex        =   7
         Top             =   750
         Width           =   975
      End
      Begin VB.Label Label2 
         Caption         =   "Username:"
         Height          =   255
         Left            =   120
         TabIndex        =   5
         Top             =   390
         Width           =   975
      End
   End
   Begin VB.Frame fraButtons 
      Height          =   1095
      Left            =   0
      TabIndex        =   1
      Top             =   5280
      Width           =   8775
      Begin VB.CheckBox chkDebug 
         Caption         =   "Show BNCSutil debug messages."
         Height          =   255
         Left            =   120
         TabIndex        =   27
         Top             =   720
         Width           =   2775
      End
      Begin MSWinsockLib.Winsock WS 
         Left            =   2280
         Top             =   240
         _ExtentX        =   741
         _ExtentY        =   741
         _Version        =   393216
      End
      Begin VB.CommandButton cmdConnect 
         Caption         =   "Connect"
         Height          =   375
         Left            =   120
         TabIndex        =   3
         Top             =   240
         Width           =   1215
      End
      Begin VB.CommandButton cmdExit 
         Caption         =   "Exit"
         Height          =   375
         Left            =   3720
         TabIndex        =   2
         Top             =   240
         Width           =   1215
      End
   End
   Begin VB.Label Label1 
      Caption         =   "A simple example of how to connect to Battle.Net using BNCSutil."
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   6135
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private ProductMap() As BNCSProducts
Public Product As BNCSProducts

Private Sub chkChangePassword_Click()
    If chkChangePassword.Value Then
        Height = 7605
        fraConfig.Height = 4215
        fraStatus.Top = 5040
        fraButtons.Top = 6120
        lblNewPassword.Visible = True
        lblNewPassword2.Visible = True
        txtNewPassword.Visible = True
        txtNewPassword2.Visible = True
    Else
        Height = 6765
        fraConfig.Height = 3375
        fraStatus.Top = 4200
        fraButtons.Top = 5280
        lblNewPassword.Visible = False
        lblNewPassword2.Visible = False
        txtNewPassword.Visible = False
        txtNewPassword2.Visible = False
    End If
    VerifyConfig
End Sub

Private Sub cmbChange()
    VerifyConfig
End Sub

Private Sub cmbClick()
    VerifyConfig
End Sub

Private Sub chkDebug_Click()
    BNCSutil.DebugMode = (chkDebug.Value = vbChecked)
End Sub

Private Sub cmbClient_Change()
    Dim Index As Long
    
    Index = cmbClient.ListIndex
    
    If Index < LBound(ProductMap) Or Index > UBound(ProductMap) Then
        ' sanity check
        Product = UNKN
        Exit Sub
    End If
    
    Product = ProductMap(Index)
    VerifyConfig
End Sub

Private Sub cmbClient_Click()
    cmbClient_Change
End Sub

Private Sub cmbClient_KeyPress(KeyAscii As Integer)
    cmbClient_Change
End Sub

Private Sub cmdExit_Click()
    Unload Me
End Sub

Private Sub Form_Load()
On Error GoTo DLL_Error
    ' First, check that the BNCSutil library is
    ' available and recent enough to use.
    If (Not bncsutil_checkVersion("1.2.0")) Then
        MsgBox "The available BNCSutil library is too old to use.  " & _
            "BNCSutil 1.2.0 and up is required.  (You have " & BNCSutil.Version & ".)", vbCritical, "Example Bot"
        End
    End If
On Error GoTo Startup_Error
    ' Note: It is no longer necessary to call kd_init().
    
    ' Add the list of clients.
    With frmMain.cmbClient
        .AddItem "StarCraft"
        .AddItem "Brood War"
        .AddItem "WarCraft II BNE"
        .AddItem "Diablo II"
        .AddItem "Lord of Destruction"
        .AddItem "WarCraft III"
        .AddItem "Frozen Throne"
    End With
    
    ' Set up combo box -> product code mapping
    ReDim ProductMap(6) As BNCSProducts
    ProductMap(0) = STAR
    ProductMap(1) = SEXP
    ProductMap(2) = W2BN
    ProductMap(3) = D2DV
    ProductMap(4) = D2XP
    ProductMap(5) = WAR3
    ProductMap(6) = W3XP
    
    Product = UNKN
    
    chkChangePassword_Click
    
    ' "Verify" the fields (which will of course fail),
    ' setting up the Status label in the process.
    VerifyConfig
    
    ' If InternalDebugMessages is False, bncsutil.dll was
    ' not compiled with debugging messages.
    chkDebug.Enabled = BNCSutil.InternalDebugMessages
    If (BNCSutil.DebugMode) Then
        chkDebug.Value = vbChecked
    Else
        chkDebug.Value = vbUnchecked
    End If
    Exit Sub
Startup_Error:
    ' Generic error.
    MsgBox "The bot failed to start.  (" & Err.Description & " - #" & _
        Err.Number & ")", vbCritical, "Example Bot"
    End
    Exit Sub
DLL_Error:
    ' This code will be run if there was a problem
    ' calling bncsutil_checkVersion, indicating
    ' a problem with finding or loading the DLL.
    MsgBox "The BNCSutil library could not be loaded, and the bot must close.", _
        vbCritical, "Example Bot"
    End
End Sub

Private Sub cmdBrowseHashes_Click()
    Dim Folder As String
    Folder = BrowseForFolder()
    If (Folder <> vbNullString) Then
        txtHashFolder.Text = Folder
    End If
End Sub

Private Sub cmdConnect_Click()
    If Not IsConnected Then
        UpdateStatus "Connecting..."
        cmdConnect.Caption = "Disconnect"
        Connect
    Else
        Disconnect
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    If (IsConnected) Then
        Disconnect
    End If
    
    If (BNCSutil.DebugMode) Then
        BNCSutil.DebugMode = False
    End If
End Sub

Private Sub txtCDKey_Change(Index As Integer)
    VerifyConfig
End Sub

Private Sub txtHashFolder_Change()
    VerifyConfig
End Sub

Private Sub txtHome_Change()
    VerifyConfig
End Sub

Private Sub txtNewPassword_Change()
    VerifyConfig
End Sub

Private Sub txtNewPassword2_Change()
    VerifyConfig
End Sub

Private Sub txtPassword_Change()
    VerifyConfig
End Sub

Private Sub txtServer_Change()
    VerifyConfig
End Sub

Private Sub txtUsername_Change()
    VerifyConfig
End Sub

Private Sub WS_Close()
    IsConnected = False
    UpdateStatus "Disconnected."
    cmdConnect.Caption = "Connect"
End Sub

Private Sub WS_Connect()
    IsConnected = True
    
    UpdateStatus "Connected."
    
    ' Let Battle.Net know that we're using the binary protocol.
    WS.SendData Chr$(1)
    
    ' Send SID_AUTH_INFO (0x50)
    SendAuthInfo
End Sub

Private Sub WS_DataArrival(ByVal bytesTotal As Long)
    Dim Data As String
    
    ' Get the data that was just received.
    WS.GetData Data, vbString
    
    ' Pass it to the first-pass parser.
    HandleData Data, bytesTotal
End Sub

Private Sub WS_Error(ByVal Number As Integer, Description As String, ByVal Scode As Long, ByVal Source As String, ByVal HelpFile As String, ByVal HelpContext As Long, CancelDisplay As Boolean)
    UpdateStatus "Socket Error: " & Description & " (#" & Number & ")", stError
    IsConnected = False
    cmdConnect.Caption = "Connect"
End Sub

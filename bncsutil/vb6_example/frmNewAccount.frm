VERSION 5.00
Begin VB.Form frmNewAccount 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Account Does Not Exist"
   ClientHeight    =   2535
   ClientLeft      =   45
   ClientTop       =   360
   ClientWidth     =   5535
   BeginProperty Font 
      Name            =   "Tahoma"
      Size            =   8.25
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   Icon            =   "frmNewAccount.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2535
   ScaleWidth      =   5535
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtConfirm 
      Height          =   285
      IMEMode         =   3  'DISABLE
      Left            =   1800
      PasswordChar    =   "*"
      TabIndex        =   6
      Top             =   1410
      Width           =   3015
   End
   Begin VB.Frame fraButtons 
      Height          =   975
      Left            =   -240
      TabIndex        =   1
      Top             =   1800
      Width           =   6135
      Begin VB.CommandButton cmdAbort 
         Cancel          =   -1  'True
         Caption         =   "Abort"
         Height          =   375
         Left            =   2760
         TabIndex        =   3
         Top             =   240
         Width           =   1215
      End
      Begin VB.CommandButton cmdCreate 
         Caption         =   "Create Account"
         Default         =   -1  'True
         Height          =   375
         Left            =   4080
         TabIndex        =   2
         Top             =   240
         Width           =   1575
      End
   End
   Begin VB.Label Label1 
      Caption         =   "Confirm Password:"
      Height          =   255
      Left            =   120
      TabIndex        =   5
      Top             =   1440
      Width           =   1575
   End
   Begin VB.Label lblStatus 
      Caption         =   "If you would like to create the account, you must confirm your password."
      Height          =   495
      Left            =   120
      TabIndex        =   4
      Top             =   720
      Width           =   5415
   End
   Begin VB.Label lblQuestion 
      Caption         =   "The account you have specified (""MMMMMMMMMMMMMMMM"") does not exist.  What would you like to do?"
      Height          =   495
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   5295
   End
End
Attribute VB_Name = "frmNewAccount"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub cmdAbort_Click()
    Disconnect
    Unload Me
End Sub

Private Sub cmdCreate_Click()
    If LenB(txtConfirm.Text) = 0 Then
        lblStatus.ForeColor = vbRed
        lblStatus.Caption = "If you would like to create the account, you must confirm your password."
        Exit Sub
    End If
    
    If (frmMain.txtPassword.Text <> txtConfirm.Text) Then
        lblStatus.ForeColor = vbRed
        lblStatus.Caption = "The confirmation you entered does not match the password you previously entered."
        Exit Sub
    End If
    
    AccountCreateAccepted
    Unload Me
End Sub

Private Sub Form_Load()
    lblQuestion.Caption = "The account you have specified (""" & _
        frmMain.txtUsername.Text & """) does not exist.  What would you like to do?"
    lblStatus.ForeColor = vbButtonText
    lblStatus.Caption = "If you would like to create the account, you must confirm your password."
End Sub

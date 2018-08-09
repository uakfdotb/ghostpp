Attribute VB_Name = "Interface"
Option Explicit

'-------------------------------------------------
'  Status Types
'-------------------------------------------------
Public Enum StatusTypes
    stNormal = 0
    stError
End Enum

'-------------------------------------------------
'  Folder Browsing API Declarations
'-------------------------------------------------
Private Type BrowseInfo
    hWndOwner As Long
    pidlRoot As Long
    sDisplayName As String
    sTitle As String
    ulFlags As Long
    lpfn As Long
    lParam As Long
    iImage As Long
End Type

Private Declare Function SHBrowseForFolder Lib "Shell32.dll" (bBrowse As BrowseInfo) As Long
Private Declare Function SHGetPathFromIDList Lib "Shell32.dll" (ByVal lItem As Long, ByVal sDir As String) As Long


'-------------------------------------------------
'  Update Status
'-------------------------------------------------
Public Sub UpdateStatus(NewStatus As String, Optional ByVal sType As StatusTypes = stNormal)
    frmMain.lblStatus.Caption = NewStatus
    
    Select Case sType
        Case stNormal
            frmMain.lblStatus.ForeColor = vbButtonText
        Case stError
            frmMain.lblStatus.ForeColor = vbRed
    End Select
End Sub

'-------------------------------------------------
'  Configuration Verifier
'-------------------------------------------------
'  Checks the fields on the form to make sure
'  they have all been filled in properly.
'-------------------------------------------------
Public Function VerifyConfig() As Boolean
    VerifyConfig = False
    
    With frmMain
        If (.WS.State = sckConnected Or .WS.State = sckConnecting) Then
            Disconnect
        End If
    
        .cmdConnect.Enabled = False
        
        ' First, determine whether to enable
        ' the expansion CD-key field.
        If (KeyCount(GetClient()) = 2) Then
            .txtCDKey(1).Locked = False
            .txtCDKey(1).BackColor = vbWindowBackground
            .txtCDKey(1).ForeColor = vbWindowText
        Else
            .txtCDKey(1).Locked = True
            .txtCDKey(1).BackColor = vbButtonFace
            .txtCDKey(1).ForeColor = vbGrayText
        End If
        
        ' Check the config fields to see if something
        ' has been entered.
        
        If LenB(.txtUsername.Text) = 0 Then
            UpdateStatus "You must enter a username.", stError
            Exit Function
        End If
        
        If LenB(.txtPassword.Text) = 0 Then
            UpdateStatus "You must enter a password.", stError
            Exit Function
        End If
        
        If LenB(.txtCDKey(0).Text) = 0 Then
            UpdateStatus "You must enter a CD-key.", stError
            Exit Function
        End If
        
        If .txtCDKey(1).Locked = False And LenB(.txtCDKey(1).Text) = 0 Then
            UpdateStatus "You must enter an expansion CD-key.", stError
            Exit Function
        End If
        
        If LenB(.txtServer.Text) = 0 Then
            UpdateStatus "You must enter a server.", stError
            Exit Function
        End If
        
        If GetClient() = UNKN Then
            UpdateStatus "You must pick a client.", stError
            Exit Function
        End If
        
        If LenB(.txtHashFolder.Text) = 0 Then
            UpdateStatus "You must enter a hash folder.", stError
            Exit Function
        End If
        
        If .chkChangePassword.Value = vbChecked Then
            If LenB(.txtNewPassword.Text) = 0 Then
                UpdateStatus "You must enter a new password.", stError
                Exit Function
            End If
            
            If LenB(.txtNewPassword2.Text) = 0 Then
                UpdateStatus "You must confirm your password.", stError
                Exit Function
            End If
            
            If (.txtNewPassword.Text <> .txtNewPassword2.Text) Then
                UpdateStatus "The new password and its confirmation do not match.", stError
                Exit Function
            End If
        End If
        
        ' OK
        UpdateStatus "Ready to connect."
        .cmdConnect.Enabled = True
        VerifyConfig = True
        
    End With
End Function

'-------------------------------------------------
'  Get Client Code
'-------------------------------------------------
'  Returns the Battle.Net product code (e.g. SEXP)
'  for the client chosen on the form.
'-------------------------------------------------
Public Function GetClient() As BNCSProducts
    GetClient = frmMain.Product
End Function

'-------------------------------------------------
'  Folder Browsing
'-------------------------------------------------
'  Let the user browse for a folder.  Returns the
'  selected folder or vbNullString on cancel.
'-------------------------------------------------
Public Function BrowseForFolder() As String
    Dim browse_info As BrowseInfo
    Dim item As Long
    Dim dir_name As String

    BrowseForFolder = vbNullString
   
    browse_info.hWndOwner = frmMain.hWnd
    browse_info.pidlRoot = 0
    browse_info.sDisplayName = String$(260, vbNullChar)
    browse_info.sTitle = "Select Hash Directory"
    browse_info.ulFlags = 1 ' Return directory name.
    browse_info.lpfn = 0
    browse_info.lParam = 0
    browse_info.iImage = 0
    
    item = SHBrowseForFolder(browse_info)
    If item Then
        dir_name = String$(260, vbNullChar)
        If SHGetPathFromIDList(item, dir_name) Then
            BrowseForFolder = Left(dir_name, InStr(dir_name, Chr$(0)) - 1)
        Else
            BrowseForFolder = vbNullString
        End If
    End If
End Function

'-------------------------------------------------
'  Ask to Create Account
'-------------------------------------------------
Public Sub ShowAccountCreate()
    frmNewAccount.Show
End Sub

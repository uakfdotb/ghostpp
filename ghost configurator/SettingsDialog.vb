'Copyright 2009 Benjamin J. Schmid 

'Licensed under the Apache License, Version 2.0 (the "License");
'you may not use this file except in compliance with the License.
'You may obtain a copy of the License at 

'	http://www.apache.org/licenses/LICENSE-2.0 

'Unless required by applicable law or agreed to in writing, software
'distributed under the License is distributed on an "AS IS" BASIS,
'WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
'See the License for the specific language governing permissions and
'limitations under the License. 

Imports System.Windows.Forms

Public Class SettingsDialog

    Structure TextBoxen
        Dim TBGhostCfg As Boolean
        Dim TBDefaultCfg As Boolean
    End Structure

    'true für benötigte texteingaben
    Dim FileNeeded As TextBoxen

    'meldungen einstellen
    Dim File_exists As String = "using the file in the application's directroy"
    Dim File_doesnt_exist As String = "file not found, please choose one"
    Dim File_create As String = "create a ghost.cfg in the application's directory"

    Private Sub OK_Button_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles OK_Button.Click

        If rBtnCreateCfg.Checked Then                   'warnung falls eine ghost.cfg schon existiert und überschrieben wird
            If TBGhostCfgPath.Text = "" And My.Computer.FileSystem.FileExists(MainWindow.cAppDirPath & MainWindow.cGhostCfg) Then
                Select Case MsgBox("In the application's directroy is already a ghost.cfg file." & vbCr & _
                                   "Do you want to overwrite it with an empty one?", MsgBoxStyle.YesNo, "Warning")
                    Case MsgBoxResult.No
                        Exit Sub
                End Select
            End If
        End If

        Me.DialogResult = System.Windows.Forms.DialogResult.OK
        Me.Close()
    End Sub

    Private Sub Cancel_Button_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Cancel_Button.Click
        Me.DialogResult = System.Windows.Forms.DialogResult.Cancel
        Me.Close()
    End Sub

    Private Sub BtnResetAll_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles BtnResetAll.Click
        Me.DialogResult = Windows.Forms.DialogResult.Ignore 'um einstellungen zurückzusetzen
        Me.Close()
    End Sub

    Private Sub rBnt_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles rBntUseMonoCfg.Click, _
                                                                                        rBtnUseDualCfg.Click, rBtnCreateCfg.Click
        'default cfg path aktivieren bzw. deaktivieren
        If rBntUseMonoCfg.Checked Then
            Me.TBDeafaultCfgPath.Enabled = False
            Me.BtnBrowseDCfg.Enabled = False
            Me.LblDcfg.Enabled = False
        Else
            Me.TBDeafaultCfgPath.Enabled = True
            Me.BtnBrowseDCfg.Enabled = True
            Me.LblDcfg.Enabled = True
        End If

        'meldung für create new ändern
        If rBtnCreateCfg.Checked Then
            TBGhostCfgPath.TextInfo = File_create
        Else
            If My.Computer.FileSystem.FileExists(MainWindow.cAppDirPath + MainWindow.cGhostCfg) Then
                Me.TBGhostCfgPath.TextInfo = File_exists
            Else
                Me.TBGhostCfgPath.TextInfo = File_doesnt_exist
            End If
        End If

        me_Validated(sender, e)
    End Sub

    'beim anzeigen den Info text richtig einstellen
    Private Sub SettingsDialog_Shown(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.Shown

        'einstellungen in setting dialog übernehmen
        TBGhostCfgPath.Text = ""
        TBDeafaultCfgPath.Text = ""
        TBGhostExePath.Text = ""

        If My.Settings.MyCfgPath <> Nothing Then TBGhostCfgPath.Text = My.Settings.MyCfgPath
        If My.Settings.DefaultCfgPath <> Nothing Then TBDeafaultCfgPath.Text = My.Settings.DefaultCfgPath
        If My.Settings.GhostExePath <> Nothing Then TBGhostExePath.Text = My.Settings.GhostExePath

        'rButtons setzen
        If My.Settings.DualCfgMode Then
            Dim tmpDefaultCfgIsInAppDir As Boolean = If(My.Settings.DefaultCfgPath = "", True, _
                                        (My.Computer.FileSystem.GetParentPath(My.Settings.DefaultCfgPath) + "\") = MainWindow.cAppDirPath)
            Dim tmpDefaultCfgPath As String = If(My.Settings.DefaultCfgPath = "", MainWindow.cAppDirPath & MainWindow.cDefaultCfg, _
                                              My.Settings.DefaultCfgPath)
            
            'falls (default.cfg existiert) aber (ghost.cfg nicht) und (create new nicht ausgegraut) und (default.cfg in app dir) ist
            If My.Computer.FileSystem.FileExists(tmpDefaultCfgPath) And _
            Not My.Computer.FileSystem.FileExists(MainWindow.cAppDirPath & MainWindow.cGhostCfg) And _
             rBtnCreateCfg.Enabled And tmpDefaultCfgIsInAppDir Then
                rBtnCreateCfg.Checked = True
            Else
                rBtnUseDualCfg.Checked = True
            End If
        Else
            rBntUseMonoCfg.Checked = True
        End If

        'info für TBghostCfg
        If My.Computer.FileSystem.FileExists(MainWindow.cAppDirPath + MainWindow.cGhostCfg) Then
            Me.TBGhostCfgPath.TextInfo = File_exists
            FileNeeded.TBGhostCfg = False
        Else
            Me.TBGhostCfgPath.TextInfo = File_doesnt_exist
            FileNeeded.TBGhostCfg = True
        End If

        'info für TBdefaultCfg
        If My.Computer.FileSystem.FileExists(MainWindow.cAppDirPath + MainWindow.cDefaultCfg) Then
            Me.TBDeafaultCfgPath.TextInfo = File_exists
            FileNeeded.TBDefaultCfg = False
        Else
            Me.TBDeafaultCfgPath.TextInfo = File_doesnt_exist
            FileNeeded.TBDefaultCfg = True
        End If

        'info für ghost.exe
        If My.Computer.FileSystem.FileExists(MainWindow.cAppDirPath + MainWindow.cGhostExe) Then
            Me.TBGhostExePath.TextInfo = File_exists
        Else
            Me.TBGhostExePath.TextInfo = File_doesnt_exist & " (optional)"
        End If

        rBnt_Click(sender, e) 'erst am ende w.g. sonst wird aktualisierte info nochmal zurückgesetzt
        'me_Validated(sender, e)    'w.g. wird indirecht von rBtn_klick ausgeführt
    End Sub

    Private Sub BtnBrowseGCfg_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles BtnBrowseGCfg.Click
        If Not rBtnCreateCfg.Checked Then
            With OpenFileDialog
                .FileName = Nothing
                .Title = "Please choose a ghost.cfg file"
                .InitialDirectory = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath)
                .FilterIndex = 1
            End With

            If OpenFileDialog.ShowDialog = Windows.Forms.DialogResult.OK Then
                TBGhostCfgPath.Text = OpenFileDialog.FileName
            End If
        Else
            CreateGhostCfgDialog.InitialDirectory = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath)

            If CreateGhostCfgDialog.ShowDialog = Windows.Forms.DialogResult.OK Then
                TBGhostCfgPath.Text = CreateGhostCfgDialog.FileName
            End If
        End If
        me_Validated(sender, e)
    End Sub

    Private Sub BtnBrowseDCfg_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles BtnBrowseDCfg.Click
        With OpenFileDialog
            .FileName = Nothing
            .Title = "Please choose a default.cfg file"
            .InitialDirectory = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath)
            .FilterIndex = 1
        End With

        If OpenFileDialog.ShowDialog = Windows.Forms.DialogResult.OK Then
            TBDeafaultCfgPath.Text = OpenFileDialog.FileName
        End If
        me_Validated(sender, e)
    End Sub

    Private Sub BtnBrowseGExe_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles BtnBrowseGExe.Click
        With OpenFileDialog
            .FileName = Nothing
            .Title = "Please choose the ghost.exe"
            .InitialDirectory = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath)
            .FilterIndex = 2
        End With

        If OpenFileDialog.ShowDialog = Windows.Forms.DialogResult.OK Then
            TBGhostExePath.Text = OpenFileDialog.FileName
        End If
        me_Validated(sender, e)
    End Sub

    'werte überprüfen und je nach dem einen error provider anzeigen
    Private Sub me_Validated(ByVal sender As Object, ByVal e As EventArgs) _
    Handles Me.MouseMove, TBGhostCfgPath.UserChangedText, TBDeafaultCfgPath.UserChangedText, TBGhostExePath.UserChangedText
        Dim DisableOK As Boolean

        'nur durch mause.move den errorprovieder auslösen
        Dim SenderIsNotTB As Boolean = Not (sender.name = TBDeafaultCfgPath.Name Or sender.name = TBGhostCfgPath.Name Or _
        sender.name = TBGhostExePath.Name)

        'GhostCfg - bedingungs variablen
        Dim tmpBadDir As Boolean
        Dim tmpBadFile As Boolean = Not My.Computer.FileSystem.GetName(TBGhostCfgPath.Text) Like "*.*"

        If Len(TBGhostCfgPath.Text) > 3 Then 'len w.g. c:\
            tmpBadDir = Not My.Computer.FileSystem.DirectoryExists( _
            My.Computer.FileSystem.GetParentPath(TBGhostCfgPath.Text))
        End If

        'GhostCfg - Textbox check
        If TBGhostCfgPath.Text = "" Then
            'wenn datei nicht existiert und nicht neu erstellen eingestellt ist
            If FileNeeded.TBGhostCfg And Not rBtnCreateCfg.Checked Then DisableOK = True

            ErrorProvider.SetError(BtnBrowseGCfg, "")

        ElseIf Not My.Computer.FileSystem.FileExists(TBGhostCfgPath.Text) And _
        Not rBtnCreateCfg.Checked Then
            If SenderIsNotTB Then ErrorProvider.SetError(BtnBrowseGCfg, "File doesn't exists!")

            'button nur deaktivieren wenn errorprovieder schon ausgelöst wurde
            If ErrorProvider.GetError(BtnBrowseGCfg) <> "" Then DisableOK = True

        ElseIf rBtnCreateCfg.Checked And (tmpBadDir Or tmpBadFile) Then
            If SenderIsNotTB Then
                If tmpBadDir Then
                    ErrorProvider.SetError(BtnBrowseGCfg, "Directory doesn't exists!")
                ElseIf tmpBadFile Then
                    ErrorProvider.SetError(BtnBrowseGCfg, "File name invalid!")
                End If
            End If
            If ErrorProvider.GetError(BtnBrowseGCfg) <> "" Then DisableOK = True
        Else
            ErrorProvider.SetError(BtnBrowseGCfg, "")

        End If

        'DefaultCfg
        If TBDeafaultCfgPath.Text = "" Then
            If FileNeeded.TBDefaultCfg And Not rBntUseMonoCfg.Checked Then DisableOK = True
            ErrorProvider.SetError(BtnBrowseDCfg, "")

        ElseIf Not My.Computer.FileSystem.FileExists(TBDeafaultCfgPath.Text) And _
        Not rBntUseMonoCfg.Checked Then
            If SenderIsNotTB Then ErrorProvider.SetError(BtnBrowseDCfg, "File doesn't exists!")

            If ErrorProvider.GetError(BtnBrowseDCfg) <> "" Then DisableOK = True
        Else
            ErrorProvider.SetError(BtnBrowseDCfg, "")
        End If

        'GhostExe
        If TBGhostExePath.Text <> "" And Not My.Computer.FileSystem.FileExists(TBGhostExePath.Text) Then
            If SenderIsNotTB Then ErrorProvider.SetError(BtnBrowseGExe, "File doesn't exists!")
        Else
            ErrorProvider.SetError(BtnBrowseGExe, "")
        End If

        'ok button ausgrauen falls ein pfad nicht stimmt
        If DisableOK Then
            OK_Button.Enabled = False
        Else
            OK_Button.Enabled = True
        End If
    End Sub
End Class

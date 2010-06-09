<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class SettingsDialog
    Inherits System.Windows.Forms.Form

    'Das Formular überschreibt den Löschvorgang, um die Komponentenliste zu bereinigen.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Wird vom Windows Form-Designer benötigt.
    Private components As System.ComponentModel.IContainer

    'Hinweis: Die folgende Prozedur ist für den Windows Form-Designer erforderlich.
    'Das Bearbeiten ist mit dem Windows Form-Designer möglich.  
    'Das Bearbeiten mit dem Code-Editor ist nicht möglich.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container
        Me.TableLayoutPanel1 = New System.Windows.Forms.TableLayoutPanel
        Me.OK_Button = New System.Windows.Forms.Button
        Me.Cancel_Button = New System.Windows.Forms.Button
        Me.rBntUseMonoCfg = New System.Windows.Forms.RadioButton
        Me.rBtnUseDualCfg = New System.Windows.Forms.RadioButton
        Me.rBtnCreateCfg = New System.Windows.Forms.RadioButton
        Me.BtnBrowseGCfg = New System.Windows.Forms.Button
        Me.BtnBrowseDCfg = New System.Windows.Forms.Button
        Me.BtnBrowseGExe = New System.Windows.Forms.Button
        Me.BtnResetAll = New System.Windows.Forms.Button
        Me.LblGCfg = New System.Windows.Forms.Label
        Me.LblDcfg = New System.Windows.Forms.Label
        Me.LblGExe = New System.Windows.Forms.Label
        Me.OpenFileDialog = New System.Windows.Forms.OpenFileDialog
        Me.CreateGhostCfgDialog = New System.Windows.Forms.SaveFileDialog
        Me.ErrorProvider = New System.Windows.Forms.ErrorProvider(Me.components)
        Me.TBGhostExePath = New ghost_configurator.myTextBox
        Me.TBDeafaultCfgPath = New ghost_configurator.myTextBox
        Me.TBGhostCfgPath = New ghost_configurator.myTextBox
        Me.TableLayoutPanel1.SuspendLayout()
        CType(Me.ErrorProvider, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.SuspendLayout()
        '
        'TableLayoutPanel1
        '
        Me.TableLayoutPanel1.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.TableLayoutPanel1.ColumnCount = 2
        Me.TableLayoutPanel1.ColumnStyles.Add(New System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50.0!))
        Me.TableLayoutPanel1.ColumnStyles.Add(New System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50.0!))
        Me.TableLayoutPanel1.Controls.Add(Me.OK_Button, 0, 0)
        Me.TableLayoutPanel1.Controls.Add(Me.Cancel_Button, 1, 0)
        Me.TableLayoutPanel1.Location = New System.Drawing.Point(277, 173)
        Me.TableLayoutPanel1.Name = "TableLayoutPanel1"
        Me.TableLayoutPanel1.RowCount = 1
        Me.TableLayoutPanel1.RowStyles.Add(New System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50.0!))
        Me.TableLayoutPanel1.Size = New System.Drawing.Size(146, 29)
        Me.TableLayoutPanel1.TabIndex = 0
        '
        'OK_Button
        '
        Me.OK_Button.Anchor = System.Windows.Forms.AnchorStyles.None
        Me.OK_Button.Location = New System.Drawing.Point(3, 3)
        Me.OK_Button.Name = "OK_Button"
        Me.OK_Button.Size = New System.Drawing.Size(67, 23)
        Me.OK_Button.TabIndex = 0
        Me.OK_Button.Text = "OK"
        '
        'Cancel_Button
        '
        Me.Cancel_Button.Anchor = System.Windows.Forms.AnchorStyles.None
        Me.Cancel_Button.DialogResult = System.Windows.Forms.DialogResult.Cancel
        Me.Cancel_Button.Location = New System.Drawing.Point(76, 3)
        Me.Cancel_Button.Name = "Cancel_Button"
        Me.Cancel_Button.Size = New System.Drawing.Size(67, 23)
        Me.Cancel_Button.TabIndex = 1
        Me.Cancel_Button.Text = "Cancel"
        '
        'rBntUseMonoCfg
        '
        Me.rBntUseMonoCfg.AutoSize = True
        Me.rBntUseMonoCfg.Location = New System.Drawing.Point(12, 12)
        Me.rBntUseMonoCfg.Name = "rBntUseMonoCfg"
        Me.rBntUseMonoCfg.Size = New System.Drawing.Size(147, 17)
        Me.rBntUseMonoCfg.TabIndex = 1
        Me.rBntUseMonoCfg.TabStop = True
        Me.rBntUseMonoCfg.Text = "Use only the ghost.cfg file"
        Me.rBntUseMonoCfg.UseVisualStyleBackColor = True
        '
        'rBtnUseDualCfg
        '
        Me.rBtnUseDualCfg.AutoSize = True
        Me.rBtnUseDualCfg.Location = New System.Drawing.Point(12, 35)
        Me.rBtnUseDualCfg.Name = "rBtnUseDualCfg"
        Me.rBtnUseDualCfg.Size = New System.Drawing.Size(228, 17)
        Me.rBtnUseDualCfg.TabIndex = 2
        Me.rBtnUseDualCfg.TabStop = True
        Me.rBtnUseDualCfg.Text = "Use bouth cfg files (default.cfg + ghost.cfg)"
        Me.rBtnUseDualCfg.UseVisualStyleBackColor = True
        '
        'rBtnCreateCfg
        '
        Me.rBtnCreateCfg.AutoSize = True
        Me.rBtnCreateCfg.Location = New System.Drawing.Point(12, 58)
        Me.rBtnCreateCfg.Name = "rBtnCreateCfg"
        Me.rBtnCreateCfg.Size = New System.Drawing.Size(299, 17)
        Me.rBtnCreateCfg.TabIndex = 3
        Me.rBtnCreateCfg.TabStop = True
        Me.rBtnCreateCfg.Text = "Use default.cfg file and create a new (empty) ghost.cfg file"
        Me.rBtnCreateCfg.UseVisualStyleBackColor = True
        '
        'BtnBrowseGCfg
        '
        Me.BtnBrowseGCfg.Location = New System.Drawing.Point(380, 78)
        Me.BtnBrowseGCfg.Name = "BtnBrowseGCfg"
        Me.BtnBrowseGCfg.Size = New System.Drawing.Size(38, 22)
        Me.BtnBrowseGCfg.TabIndex = 7
        Me.BtnBrowseGCfg.Text = "..."
        Me.BtnBrowseGCfg.UseVisualStyleBackColor = True
        '
        'BtnBrowseDCfg
        '
        Me.BtnBrowseDCfg.Location = New System.Drawing.Point(380, 104)
        Me.BtnBrowseDCfg.Name = "BtnBrowseDCfg"
        Me.BtnBrowseDCfg.Size = New System.Drawing.Size(38, 22)
        Me.BtnBrowseDCfg.TabIndex = 8
        Me.BtnBrowseDCfg.Text = "..."
        Me.BtnBrowseDCfg.UseVisualStyleBackColor = True
        '
        'BtnBrowseGExe
        '
        Me.BtnBrowseGExe.Location = New System.Drawing.Point(380, 130)
        Me.BtnBrowseGExe.Name = "BtnBrowseGExe"
        Me.BtnBrowseGExe.Size = New System.Drawing.Size(38, 22)
        Me.BtnBrowseGExe.TabIndex = 9
        Me.BtnBrowseGExe.Text = "..."
        Me.BtnBrowseGExe.UseVisualStyleBackColor = True
        '
        'BtnResetAll
        '
        Me.BtnResetAll.Enabled = False
        Me.BtnResetAll.Location = New System.Drawing.Point(6, 177)
        Me.BtnResetAll.Name = "BtnResetAll"
        Me.BtnResetAll.Size = New System.Drawing.Size(130, 22)
        Me.BtnResetAll.TabIndex = 10
        Me.BtnResetAll.Text = "delete settings file"
        Me.BtnResetAll.UseVisualStyleBackColor = True
        '
        'LblGCfg
        '
        Me.LblGCfg.AutoSize = True
        Me.LblGCfg.Location = New System.Drawing.Point(9, 83)
        Me.LblGCfg.Name = "LblGCfg"
        Me.LblGCfg.Size = New System.Drawing.Size(78, 13)
        Me.LblGCfg.TabIndex = 11
        Me.LblGCfg.Text = "ghost.cfg path:"
        '
        'LblDcfg
        '
        Me.LblDcfg.AutoSize = True
        Me.LblDcfg.Location = New System.Drawing.Point(3, 109)
        Me.LblDcfg.Name = "LblDcfg"
        Me.LblDcfg.Size = New System.Drawing.Size(84, 13)
        Me.LblDcfg.TabIndex = 12
        Me.LblDcfg.Text = "default.cfg path:"
        '
        'LblGExe
        '
        Me.LblGExe.AutoSize = True
        Me.LblGExe.Location = New System.Drawing.Point(7, 135)
        Me.LblGExe.Name = "LblGExe"
        Me.LblGExe.Size = New System.Drawing.Size(80, 13)
        Me.LblGExe.TabIndex = 13
        Me.LblGExe.Text = "ghost.exe path:"
        '
        'OpenFileDialog
        '
        Me.OpenFileDialog.FileName = "OpenFileDialog1"
        Me.OpenFileDialog.Filter = "Ghost Cfg's|*.cfg|Applications|*.exe|All files|*.*"
        '
        'CreateGhostCfgDialog
        '
        Me.CreateGhostCfgDialog.Filter = "Ghost Cfg's|*.cfg|All files|*.*"
        Me.CreateGhostCfgDialog.Title = "Please choose a location and the name for the ghost.cfg file"
        '
        'ErrorProvider
        '
        Me.ErrorProvider.ContainerControl = Me
        '
        'TBGhostExePath
        '
        Me.TBGhostExePath.ForeColor = System.Drawing.Color.SlateGray
        Me.TBGhostExePath.Location = New System.Drawing.Point(95, 132)
        Me.TBGhostExePath.Name = "TBGhostExePath"
        Me.TBGhostExePath.Size = New System.Drawing.Size(279, 20)
        Me.TBGhostExePath.TabIndex = 6
        Me.TBGhostExePath.TextInfo = Nothing
        '
        'TBDeafaultCfgPath
        '
        Me.TBDeafaultCfgPath.ForeColor = System.Drawing.Color.SlateGray
        Me.TBDeafaultCfgPath.Location = New System.Drawing.Point(95, 106)
        Me.TBDeafaultCfgPath.Name = "TBDeafaultCfgPath"
        Me.TBDeafaultCfgPath.Size = New System.Drawing.Size(279, 20)
        Me.TBDeafaultCfgPath.TabIndex = 5
        Me.TBDeafaultCfgPath.TextInfo = Nothing
        '
        'TBGhostCfgPath
        '
        Me.TBGhostCfgPath.ForeColor = System.Drawing.Color.SlateGray
        Me.TBGhostCfgPath.Location = New System.Drawing.Point(95, 80)
        Me.TBGhostCfgPath.Name = "TBGhostCfgPath"
        Me.TBGhostCfgPath.Size = New System.Drawing.Size(279, 20)
        Me.TBGhostCfgPath.TabIndex = 4
        Me.TBGhostCfgPath.TextInfo = Nothing
        '
        'SettingsDialog
        '
        Me.AcceptButton = Me.OK_Button
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.CancelButton = Me.Cancel_Button
        Me.ClientSize = New System.Drawing.Size(435, 209)
        Me.Controls.Add(Me.LblGExe)
        Me.Controls.Add(Me.LblDcfg)
        Me.Controls.Add(Me.LblGCfg)
        Me.Controls.Add(Me.BtnResetAll)
        Me.Controls.Add(Me.BtnBrowseGExe)
        Me.Controls.Add(Me.BtnBrowseDCfg)
        Me.Controls.Add(Me.BtnBrowseGCfg)
        Me.Controls.Add(Me.TBGhostExePath)
        Me.Controls.Add(Me.TBDeafaultCfgPath)
        Me.Controls.Add(Me.TBGhostCfgPath)
        Me.Controls.Add(Me.rBtnCreateCfg)
        Me.Controls.Add(Me.rBtnUseDualCfg)
        Me.Controls.Add(Me.rBntUseMonoCfg)
        Me.Controls.Add(Me.TableLayoutPanel1)
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog
        Me.MaximizeBox = False
        Me.MinimizeBox = False
        Me.Name = "SettingsDialog"
        Me.ShowInTaskbar = False
        Me.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen
        Me.Text = "Settings"
        Me.TableLayoutPanel1.ResumeLayout(False)
        CType(Me.ErrorProvider, System.ComponentModel.ISupportInitialize).EndInit()
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents TableLayoutPanel1 As System.Windows.Forms.TableLayoutPanel
    Friend WithEvents OK_Button As System.Windows.Forms.Button
    Friend WithEvents Cancel_Button As System.Windows.Forms.Button
    Friend WithEvents rBntUseMonoCfg As System.Windows.Forms.RadioButton
    Friend WithEvents rBtnUseDualCfg As System.Windows.Forms.RadioButton
    Friend WithEvents rBtnCreateCfg As System.Windows.Forms.RadioButton
    Friend WithEvents TBGhostCfgPath As ghost_configurator.myTextBox
    Friend WithEvents TBDeafaultCfgPath As ghost_configurator.myTextBox
    Friend WithEvents TBGhostExePath As ghost_configurator.myTextBox
    Friend WithEvents BtnBrowseGCfg As System.Windows.Forms.Button
    Friend WithEvents BtnBrowseDCfg As System.Windows.Forms.Button
    Friend WithEvents BtnBrowseGExe As System.Windows.Forms.Button
    Friend WithEvents BtnResetAll As System.Windows.Forms.Button
    Friend WithEvents LblGCfg As System.Windows.Forms.Label
    Friend WithEvents LblDcfg As System.Windows.Forms.Label
    Friend WithEvents LblGExe As System.Windows.Forms.Label
    Friend WithEvents OpenFileDialog As System.Windows.Forms.OpenFileDialog
    Friend WithEvents CreateGhostCfgDialog As System.Windows.Forms.SaveFileDialog
    Friend WithEvents ErrorProvider As System.Windows.Forms.ErrorProvider

End Class

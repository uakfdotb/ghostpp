<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class MainWindow
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
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(MainWindow))
        Me.TabControl1 = New System.Windows.Forms.TabControl
        Me.TabBot = New System.Windows.Forms.TabPage
        Me.TabBnet = New System.Windows.Forms.TabPage
        Me.TabControl2 = New System.Windows.Forms.TabControl
        Me.SubTab1 = New System.Windows.Forms.TabPage
        Me.addServer = New System.Windows.Forms.Button
        Me.TabAgame = New System.Windows.Forms.TabPage
        Me.GroupReplay = New System.Windows.Forms.GroupBox
        Me.GroupLAN = New System.Windows.Forms.GroupBox
        Me.GroupAHost = New System.Windows.Forms.GroupBox
        Me.GroupUdpTcp = New System.Windows.Forms.GroupBox
        Me.GroupAGame = New System.Windows.Forms.GroupBox
        Me.Tabdb = New System.Windows.Forms.TabPage
        Me.TabHelp = New System.Windows.Forms.TabPage
        Me.RichTextBox1 = New System.Windows.Forms.RichTextBox
        Me.InfoLabel = New System.Windows.Forms.Label
        Me.CBoxInfoTimer = New System.Windows.Forms.Timer(Me.components)
        Me.dontSaveCB = New System.Windows.Forms.CheckBox
        Me.runGhostCB = New System.Windows.Forms.CheckBox
        Me.Btn_openSettings = New System.Windows.Forms.Button
        Me.DefaultTip = New System.Windows.Forms.ToolTip(Me.components)
        Me.TabControl1.SuspendLayout()
        Me.TabBnet.SuspendLayout()
        Me.TabControl2.SuspendLayout()
        Me.SubTab1.SuspendLayout()
        Me.TabAgame.SuspendLayout()
        Me.TabHelp.SuspendLayout()
        Me.SuspendLayout()
        '
        'TabControl1
        '
        Me.TabControl1.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.TabControl1.Controls.Add(Me.TabBot)
        Me.TabControl1.Controls.Add(Me.TabBnet)
        Me.TabControl1.Controls.Add(Me.TabAgame)
        Me.TabControl1.Controls.Add(Me.Tabdb)
        Me.TabControl1.Controls.Add(Me.TabHelp)
        Me.TabControl1.Location = New System.Drawing.Point(12, 22)
        Me.TabControl1.MinimumSize = New System.Drawing.Size(0, 200)
        Me.TabControl1.Name = "TabControl1"
        Me.TabControl1.SelectedIndex = 0
        Me.TabControl1.Size = New System.Drawing.Size(426, 390)
        Me.TabControl1.TabIndex = 0
        '
        'TabBot
        '
        Me.TabBot.AutoScroll = True
        Me.TabBot.Location = New System.Drawing.Point(4, 22)
        Me.TabBot.Name = "TabBot"
        Me.TabBot.Padding = New System.Windows.Forms.Padding(3)
        Me.TabBot.Size = New System.Drawing.Size(418, 364)
        Me.TabBot.TabIndex = 0
        Me.TabBot.Text = "Bot"
        Me.TabBot.UseVisualStyleBackColor = True
        '
        'TabBnet
        '
        Me.TabBnet.Controls.Add(Me.TabControl2)
        Me.TabBnet.Location = New System.Drawing.Point(4, 22)
        Me.TabBnet.Name = "TabBnet"
        Me.TabBnet.Padding = New System.Windows.Forms.Padding(3)
        Me.TabBnet.Size = New System.Drawing.Size(418, 364)
        Me.TabBnet.TabIndex = 1
        Me.TabBnet.Text = "Battle.net"
        Me.TabBnet.UseVisualStyleBackColor = True
        '
        'TabControl2
        '
        Me.TabControl2.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.TabControl2.Controls.Add(Me.SubTab1)
        Me.TabControl2.Location = New System.Drawing.Point(6, 6)
        Me.TabControl2.Name = "TabControl2"
        Me.TabControl2.SelectedIndex = 0
        Me.TabControl2.Size = New System.Drawing.Size(406, 214)
        Me.TabControl2.TabIndex = 0
        '
        'SubTab1
        '
        Me.SubTab1.BackColor = System.Drawing.Color.WhiteSmoke
        Me.SubTab1.Controls.Add(Me.addServer)
        Me.SubTab1.Location = New System.Drawing.Point(4, 22)
        Me.SubTab1.Name = "SubTab1"
        Me.SubTab1.Padding = New System.Windows.Forms.Padding(3)
        Me.SubTab1.Size = New System.Drawing.Size(398, 188)
        Me.SubTab1.TabIndex = 0
        Me.SubTab1.Text = "Server 1"
        '
        'addServer
        '
        Me.addServer.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.addServer.Location = New System.Drawing.Point(322, 8)
        Me.addServer.Name = "addServer"
        Me.addServer.Size = New System.Drawing.Size(74, 20)
        Me.addServer.TabIndex = 0
        Me.addServer.Text = "add server"
        Me.addServer.UseVisualStyleBackColor = True
        '
        'TabAgame
        '
        Me.TabAgame.Controls.Add(Me.GroupReplay)
        Me.TabAgame.Controls.Add(Me.GroupLAN)
        Me.TabAgame.Controls.Add(Me.GroupAHost)
        Me.TabAgame.Controls.Add(Me.GroupUdpTcp)
        Me.TabAgame.Controls.Add(Me.GroupAGame)
        Me.TabAgame.Location = New System.Drawing.Point(4, 22)
        Me.TabAgame.Name = "TabAgame"
        Me.TabAgame.Size = New System.Drawing.Size(418, 364)
        Me.TabAgame.TabIndex = 2
        Me.TabAgame.Text = "Admin Game & more"
        Me.TabAgame.UseVisualStyleBackColor = True
        '
        'GroupReplay
        '
        Me.GroupReplay.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.GroupReplay.Location = New System.Drawing.Point(3, 195)
        Me.GroupReplay.Name = "GroupReplay"
        Me.GroupReplay.Size = New System.Drawing.Size(412, 59)
        Me.GroupReplay.TabIndex = 0
        Me.GroupReplay.TabStop = False
        Me.GroupReplay.Text = "Replay"
        '
        'GroupLAN
        '
        Me.GroupLAN.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.GroupLAN.Location = New System.Drawing.Point(3, 260)
        Me.GroupLAN.Name = "GroupLAN"
        Me.GroupLAN.Size = New System.Drawing.Size(412, 59)
        Me.GroupLAN.TabIndex = 0
        Me.GroupLAN.TabStop = False
        Me.GroupLAN.Text = "LAN"
        '
        'GroupAHost
        '
        Me.GroupAHost.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.GroupAHost.Location = New System.Drawing.Point(3, 130)
        Me.GroupAHost.Name = "GroupAHost"
        Me.GroupAHost.Size = New System.Drawing.Size(412, 59)
        Me.GroupAHost.TabIndex = 0
        Me.GroupAHost.TabStop = False
        Me.GroupAHost.Text = "AutoHost"
        '
        'GroupUdpTcp
        '
        Me.GroupUdpTcp.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.GroupUdpTcp.Location = New System.Drawing.Point(3, 74)
        Me.GroupUdpTcp.Name = "GroupUdpTcp"
        Me.GroupUdpTcp.Size = New System.Drawing.Size(412, 50)
        Me.GroupUdpTcp.TabIndex = 0
        Me.GroupUdpTcp.TabStop = False
        Me.GroupUdpTcp.Text = "UDP && TCP"
        '
        'GroupAGame
        '
        Me.GroupAGame.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.GroupAGame.Location = New System.Drawing.Point(3, 3)
        Me.GroupAGame.Name = "GroupAGame"
        Me.GroupAGame.Size = New System.Drawing.Size(412, 65)
        Me.GroupAGame.TabIndex = 0
        Me.GroupAGame.TabStop = False
        Me.GroupAGame.Text = "Admin Game"
        '
        'Tabdb
        '
        Me.Tabdb.Location = New System.Drawing.Point(4, 22)
        Me.Tabdb.Name = "Tabdb"
        Me.Tabdb.Size = New System.Drawing.Size(418, 364)
        Me.Tabdb.TabIndex = 3
        Me.Tabdb.Text = "Database"
        Me.Tabdb.UseVisualStyleBackColor = True
        '
        'TabHelp
        '
        Me.TabHelp.Controls.Add(Me.RichTextBox1)
        Me.TabHelp.Location = New System.Drawing.Point(4, 22)
        Me.TabHelp.Name = "TabHelp"
        Me.TabHelp.Size = New System.Drawing.Size(418, 364)
        Me.TabHelp.TabIndex = 5
        Me.TabHelp.Text = "Help"
        Me.TabHelp.UseVisualStyleBackColor = True
        '
        'RichTextBox1
        '
        Me.RichTextBox1.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
                    Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.RichTextBox1.Location = New System.Drawing.Point(10, 10)
        Me.RichTextBox1.Name = "RichTextBox1"
        Me.RichTextBox1.ReadOnly = True
        Me.RichTextBox1.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.ForcedVertical
        Me.RichTextBox1.Size = New System.Drawing.Size(395, 351)
        Me.RichTextBox1.TabIndex = 0
        Me.RichTextBox1.Text = resources.GetString("RichTextBox1.Text")
        '
        'InfoLabel
        '
        Me.InfoLabel.Anchor = CType(((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Left) _
                    Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.InfoLabel.AutoSize = True
        Me.InfoLabel.Font = New System.Drawing.Font("Microsoft Sans Serif", 8.25!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.InfoLabel.Location = New System.Drawing.Point(9, 415)
        Me.InfoLabel.MaximumSize = New System.Drawing.Size(434, 0)
        Me.InfoLabel.Name = "InfoLabel"
        Me.InfoLabel.Size = New System.Drawing.Size(393, 13)
        Me.InfoLabel.TabIndex = 1
        Me.InfoLabel.Text = "Click at a Textbox or hover over a checkbox to get informatione about this settin" & _
            "g!"
        '
        'CBoxInfoTimer
        '
        Me.CBoxInfoTimer.Interval = 200
        '
        'dontSaveCB
        '
        Me.dontSaveCB.AutoSize = True
        Me.dontSaveCB.Location = New System.Drawing.Point(148, 3)
        Me.dontSaveCB.Name = "dontSaveCB"
        Me.dontSaveCB.Size = New System.Drawing.Size(148, 17)
        Me.dontSaveCB.TabIndex = 7
        Me.dontSaveCB.Text = "don't save settings on exit"
        Me.dontSaveCB.UseVisualStyleBackColor = True
        '
        'runGhostCB
        '
        Me.runGhostCB.AutoSize = True
        Me.runGhostCB.Location = New System.Drawing.Point(12, 3)
        Me.runGhostCB.Name = "runGhostCB"
        Me.runGhostCB.Size = New System.Drawing.Size(130, 17)
        Me.runGhostCB.TabIndex = 8
        Me.runGhostCB.Text = "run ghost after closing"
        Me.runGhostCB.UseVisualStyleBackColor = True
        '
        'Btn_openSettings
        '
        Me.Btn_openSettings.Location = New System.Drawing.Point(387, 1)
        Me.Btn_openSettings.Name = "Btn_openSettings"
        Me.Btn_openSettings.Size = New System.Drawing.Size(62, 22)
        Me.Btn_openSettings.TabIndex = 9
        Me.Btn_openSettings.Text = "Settings"
        Me.Btn_openSettings.UseVisualStyleBackColor = True
        '
        'DefaultTip
        '
        Me.DefaultTip.AutoPopDelay = 5000
        Me.DefaultTip.InitialDelay = 2000
        Me.DefaultTip.ReshowDelay = 100
        Me.DefaultTip.ToolTipTitle = "Hint:"
        '
        'MainWindow
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(450, 448)
        Me.Controls.Add(Me.Btn_openSettings)
        Me.Controls.Add(Me.runGhostCB)
        Me.Controls.Add(Me.dontSaveCB)
        Me.Controls.Add(Me.InfoLabel)
        Me.Controls.Add(Me.TabControl1)
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D
        Me.MaximizeBox = False
        Me.MinimizeBox = False
        Me.Name = "MainWindow"
        Me.Text = "Loading..."
        Me.TabControl1.ResumeLayout(False)
        Me.TabBnet.ResumeLayout(False)
        Me.TabControl2.ResumeLayout(False)
        Me.SubTab1.ResumeLayout(False)
        Me.TabAgame.ResumeLayout(False)
        Me.TabHelp.ResumeLayout(False)
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents TabControl1 As System.Windows.Forms.TabControl
    Friend WithEvents TabBot As System.Windows.Forms.TabPage
    Friend WithEvents TabAgame As System.Windows.Forms.TabPage
    Friend WithEvents Tabdb As System.Windows.Forms.TabPage
    Friend WithEvents InfoLabel As System.Windows.Forms.Label
    Friend WithEvents CBoxInfoTimer As System.Windows.Forms.Timer
    Friend WithEvents TabBnet As System.Windows.Forms.TabPage
    Friend WithEvents TabControl2 As System.Windows.Forms.TabControl
    Friend WithEvents SubTab1 As System.Windows.Forms.TabPage
    Friend WithEvents GroupLAN As System.Windows.Forms.GroupBox
    Friend WithEvents GroupAHost As System.Windows.Forms.GroupBox
    Friend WithEvents GroupUdpTcp As System.Windows.Forms.GroupBox
    Friend WithEvents GroupAGame As System.Windows.Forms.GroupBox
    Friend WithEvents GroupReplay As System.Windows.Forms.GroupBox
    Friend WithEvents addServer As System.Windows.Forms.Button
    Friend WithEvents TabHelp As System.Windows.Forms.TabPage
    Friend WithEvents RichTextBox1 As System.Windows.Forms.RichTextBox
    Friend WithEvents dontSaveCB As System.Windows.Forms.CheckBox
    Friend WithEvents runGhostCB As System.Windows.Forms.CheckBox
    Friend WithEvents Btn_openSettings As System.Windows.Forms.Button
    Friend WithEvents DefaultTip As System.Windows.Forms.ToolTip

End Class

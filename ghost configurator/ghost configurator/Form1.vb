Public Class Form1
    Dim Label(300) As Label
    Dim TextBox(300) As TextBox
    Dim Numeric(300) As NumericUpDown
    Dim CheckBox(300), ColorCBox(300) As CheckBox
    Dim Button(300) As Button
    Dim DirBrowser(300) As FolderBrowserDialog
    Dim ColorWin(300) As ColorDialog
    Dim SubTab(10) As TabPage
    Dim pfad, version, keyStr, zedStr, ErrorStr(10), key(300), schluessel(300), wert(300), info(300), textlines() As String
    Dim InfoNum, InfoCount, zed, HeigthAdd, ErrorN(10), BNetIndex(50), line(300), type(300), tab(300), LTool(2, 300) As Integer
    'bugs:!?

    Private Sub Form1_Load(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.Load
        'start konfiguration
        Me.Icon = My.Resources.parametres_3D
        version = "v1.05"
        'fenster initalisieren
        Dim teile(), templine As String
        Dim count(40) As Integer            '30 für anzahl tabs(1-4, 21-40)
        Dim de As Integer = 1
        pfad = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath) + "\ghost.cfg"

        'pfad zur cfg datei einstellen
        If My.Settings.Update Then
            My.Settings.Upgrade()
            My.Settings.Update = False
        End If

        If My.Settings.CfgPath = Nothing Then
            If Not My.Computer.FileSystem.FileExists(pfad) Then
                If openCfgBrowse() Then
                    ErrorN(0) = 2
                    AppError()
                    Exit Sub
                End If
            End If
        Else
            If My.Computer.FileSystem.FileExists(My.Settings.CfgPath) Then
                pfad = My.Settings.CfgPath
                Me.CfgPathBox.Text = My.Settings.CfgPath
            Else
                If openCfgBrowse() Then
                    ErrorN(0) = 2
                    AppError()
                    Exit Sub
                End If
            End If
        End If

        'falls i was schief läuft error #001
        Try
            textlines = IO.File.ReadAllLines(pfad)
            For i As Integer = 0 To UBound(textlines)
                templine = LTrim(textlines(i))

                'sortieren von infos und keys
                If templine = "" Then
                    'nichts unternehmen (debug)
                ElseIf templine.StartsWith("#") Then
                    templine = LTrim(templine.Remove(0, templine.LastIndexOf("#") + 1))
                    If Not (templine.Contains("CONFIGURATION") Or IsSubfix(templine) Or templine.StartsWith("example")) And templine <> "" Then
                        info(de) += LTrim(templine)
                        If info(de) <> "" Then
                            info(de) += Chr(13)
                        End If
                    End If
                ElseIf templine.Contains("=") And IsSubfix(templine) Then
                    line(de) = i
                    'schluessel & wert trennen
                    teile = Split(textlines(i), "=")
                    key(de) = Trim(teile(0))
                    schluessel(de) = Trim(teile(0))
                    'schluessel prefix entfernen
                    If schluessel(de).StartsWith("bot_") Then
                        schluessel(de) = schluessel(de).Remove(0, 4)
                    ElseIf schluessel(de).StartsWith("admingame_") Then
                        schluessel(de) = schluessel(de).Remove(0, 10)
                    ElseIf schluessel(de).StartsWith("db_") Then
                        schluessel(de) = schluessel(de).Remove(0, 3)
                    ElseIf schluessel(de).StartsWith("bnet_") Then
                        schluessel(de) = schluessel(de).Remove(0, 5)
                    ElseIf schluessel(de).StartsWith("bnet") Then
                        If IsNumeric(Mid(schluessel(de), 5, (Mid(schluessel(de), 5).IndexOf("_")))) Then
                            If Mid(schluessel(de), 5, (Mid(schluessel(de), 5).IndexOf("_"))) <= 10 Then
                                schluessel(de) = schluessel(de).Remove(0, 5 + Mid(templine, 5).IndexOf("_"))
                            Else
                                'bereits gesetzte variablen zurückgsetzten
                                key(de) = ""
                                schluessel(de) = ""
                                line(de) = 0
                                ErrorN(3) = 1                                   'error: syntax fehler("bnet11_")
                                ErrorStr(3) += "  " & templine & Chr(13)
                            End If
                        Else
                            'bereits gesetzte variablen zurückgsetzten
                            key(de) = ""
                            schluessel(de) = ""
                            line(de) = 0
                            ErrorN(4) = 1                                       'error: syntax fehler("bnetABC_")
                            ErrorStr(4) += "  " & templine & Chr(13)
                        End If
                    ElseIf schluessel(de).StartsWith("udp_") Or schluessel(de).StartsWith("tcp_") Or _
                    schluessel(de).StartsWith("lan_") Then
                        schluessel(de) = schluessel(de).Remove(0, 4)
                    ElseIf schluessel(de).StartsWith("autohost_") Then
                        schluessel(de) = schluessel(de).Remove(0, 9)
                    ElseIf schluessel(de).StartsWith("replay_") Then
                        schluessel(de) = schluessel(de).Remove(0, 7)
                    End If
                    wert(de) = LTrim(teile(1))
                    'wert einordnen
                    If info(de) <> "" Then
                        If info(de).Contains("port") Or info(de).Contains("max") Or info(de).Contains("Numeric") Or _
                        info(de).Contains("method") Or info(de).Contains("higher") Or info(de).Contains("set to") Or _
                        info(de).Contains("percent") Or info(de).Contains("number") Or _
                        schluessel(de).Contains("max") Or schluessel(de).Contains("port") Then
                            'NummericUpDown
                            type(de) = 2
                        ElseIf wert(de) = "0" Or wert(de) = "1" Then
                            'checkbox
                            type(de) = 3
                        ElseIf info(de).Contains("directory") Or info(de).Contains("path") Then
                            'textbox mit dir browser
                            type(de) = 11
                        ElseIf info(de).Contains("colour") Or info(de).Contains("color") Then
                            'textbox mit farb auswahl
                            type(de) = 12
                        Else
                            'für alle nicht erkannte -> normale textbox
                            type(de) = 1
                        End If
                    Else
                        info(de) += "NO description available!"
                        If schluessel(de) <> "" Then
                            If schluessel(de).Contains("max") Or schluessel(de).Contains("port") Then
                                'NummericUpDown
                                type(de) = 2
                            ElseIf schluessel(de).Contains("path") Then
                                'textbox mit dir browser
                                type(de) = 11
                            Else
                                'für alle nicht erkannten -> normale textbox
                                type(de) = 1
                            End If
                        End If
                    End If
                    de += 1
                    If de = UBound(key) + 1 Then                    'error: zu viele werte
                        ErrorN(1) = 1
                        Exit For
                    End If
                ElseIf Not templine.Contains("=") Then
                    If IsSubfix(templine) Then
                        ErrorN(2) = 1                               'error: syntax fehler(kein "=")
                        ErrorStr(2) += "  " & templine & Chr(13)
                    Else
                        ErrorN(6) = 1
                        ErrorStr(6) += "  " & templine & Chr(13)    'error: syntaxfehler (kein "#")
                    End If
                End If
            Next
            If de = 1 Then                                          'error: keine einstellungen gefunden
                ErrorN(0) = 3
                AppError()
                Exit Sub
            End If
        Catch ex As Exception                                       'error: lade fehler
            ErrorN(0) = 1
            AppError()
            Exit Sub
        End Try

        'unvollständige BNetKey's rausfilltern
        Dim aNum, runs As Integer
        For Me.zed = 2 To 10
            zedStr = zed
            If Array.FindIndex(key, AddressOf FindBNetKey) <> -1 Then
                If Array.FindIndex(key, AddressOf FindPasswordKey) = -1 Then
                    For Each Me.keyStr In Array.FindAll(key, AddressOf FindBNetKey)
                        aNum = Array.FindIndex(key, AddressOf FindKey)
                        type(aNum) = 0
                        For be As Integer = line(aNum) To UBound(textlines) - 1
                            textlines(be) = textlines(be + 1)
                        Next
                        For da As Integer = 1 To UBound(line)
                            If line(da) > line(aNum) Then line(da) -= 1
                        Next
                        runs += 1
                        line(aNum) = 0
                    Next
                End If
            End If
        Next
        'map_loadInGame schüssel entfernen
        If Array.FindIndex(textlines, AddressOf IsKeyLoadInGame) <> -1 Then
            Dim index As Integer = Array.FindIndex(textlines, AddressOf IsKeyLoadInGame)
            For be As Integer = index To UBound(textlines) - 1
                textlines(be) = textlines(be + 1)
            Next
            For da As Integer = 1 To UBound(line)
                If line(da) > index Then line(da) -= 1
            Next
            runs += 1
        End If
        Array.Resize(textlines, textlines.Length - runs)

        'Tools anpassen
        Dim b As Integer
        'setzen der tools
        For a As Integer = 1 To UBound(key)
            If key(a) <> "" Then
                If Not key(a).StartsWith("#") Then
                    If type(a) = 1 Or type(a) = 11 Or type(a) = 12 Then
                        'Texetbox + Lable erstellen
                        Label(a) = New Label
                        Label(a).Name = "Label" & a
                        Label(a).AutoSize = True
                        TextBox(a) = New TextBox
                        TextBox(a).Name = "TextBox" & a
                        TextBox(a).Size = New Size(212, 20)
                        Label(a).Text = schluessel(a)
                        TextBox(a).Text = wert(a)
                        AddHandler TextBox(a).Click, AddressOf TextBox_click
                        If type(a) = 11 Then
                            Button(a) = New Button
                            With Button(a)
                                .Width = 30
                                .Height = 20
                                .Name = "Button" & a
                                .Text = "..."
                                AddHandler Button(a).Click, AddressOf ButtonFolder_click
                            End With
                            DirBrowser(a) = New FolderBrowserDialog
                            DirBrowser(a).Description = "Please select the " & Label(a).Text & "!"
                        ElseIf type(a) = 12 Then
                            Button(a) = New Button
                            ColorCBox(a) = New CheckBox
                            With Button(a)
                                .Width = 30
                                .Height = 20
                                .Name = "Button" & a
                                .Text = ""
                                AddHandler .Click, AddressOf ButtonColor_click
                            End With
                            With ColorCBox(a)
                                .Name = "ColorCBox" & a
                                .Text = ""
                                .AutoSize = True
                                AddHandler .Click, AddressOf ColorCheckBox_Click
                            End With
                            If wert(a).StartsWith("|cFF") And IsHex(Mid(wert(a), 5, 2)) And IsHex(Mid(wert(a), 7, 2)) And _
                            IsHex(Mid(wert(a), 9, 2)) Then
                                Button(a).BackColor = Color.FromArgb(CInt(CHex(False, Mid(wert(a), 5, 2))), _
                                                            CInt(CHex(False, Mid(wert(a), 7, 2))), _
                                                            CInt(CHex(False, Mid(wert(a), 9, 2))))
                                TextBox(a).Text = Mid(wert(a), 11)
                                ColorCBox(a).Checked = True
                            Else
                                Button(a).Enabled = False
                            End If
                            ColorWin(a) = New ColorDialog
                        End If
                    ElseIf type(a) = 2 Then
                        'NummericUpDown + Lable erstellen
                        Label(a) = New Label
                        Label(a).Name = "Label" & b
                        Label(a).AutoSize = True
                        Numeric(a) = New NumericUpDown
                        With Numeric(a)
                            .Name = "NumericUpDown" & a
                            .Size = New Size(90, 20)
                            .Maximum = 9999
                        End With
                        Label(a).Text = schluessel(a)
                        Numeric(a).Text = wert(a)
                        AddHandler Numeric(a).Click, AddressOf Numeric_click
                    ElseIf type(a) = 3 Then
                        'nur checkbox erstellen
                        CheckBox(a) = New CheckBox
                        With CheckBox(a)
                            .Name = "CheckBox" & a
                            .AutoSize = True
                            .Text = schluessel(a)
                            .Checked = CBool(wert(a))
                            AddHandler .MouseMove, AddressOf CheckBox_MouseMove
                        End With
                    End If
                End If
            End If
        Next
        'tabs den tools zuordnen
        Dim UTab As Integer
        For a As Integer = 1 To UBound(LTool, 2)
            If key(a) <> "" And type(a) <> 0 Then
                If Not key(a).StartsWith("#") Then
                    If key(a).StartsWith("bot_") Then
                        tab(a) = 1
                    ElseIf key(a).StartsWith("bnet") Then
                        If key(a).StartsWith("bnet_") Then
                            tab(a) = 21
                        Else
                            tab(a) = 20 + CInt(Mid(key(a), 5, Mid(key(a), 5).IndexOf("_")))     'legt die tabanzahl zwischen 22-30 fest!
                            If tab(a) > UTab Then
                                UTab = tab(a)
                            End If
                        End If
                    ElseIf key(a).StartsWith("admingame_") Then
                        tab(a) = 31
                    ElseIf key(a).StartsWith("db_") Then
                        tab(a) = 4
                    ElseIf key(a).StartsWith("udp_") Or key(a).StartsWith("tcp_") Then
                        tab(a) = 32
                    ElseIf key(a).StartsWith("autohost_") Then
                        tab(a) = 33
                    ElseIf key(a).StartsWith("replay_") Then
                        tab(a) = 34
                    ElseIf key(a).StartsWith("lan_") Then
                        tab(a) = 35
                    End If
                End If
            End If
        Next

        'bnet subtabs erstellen
        SubTab(1) = SubTab1
        AddHandler SubTab(1).MouseMove, AddressOf Any_MouseMove
        For i As Integer = 2 To (UTab - 20)
            SubTab(i) = New TabPage
            With SubTab(i)
                .Text = "Server " & i
                .Name = "SubTab" & i
                .BackColor = Color.WhiteSmoke
                .Parent = Me.TabControl2
                AddHandler .MouseMove, AddressOf Any_MouseMove
            End With
            If i = UTab - 20 Then                                   'letztem tab remove Button hinzufügen
                Dim rmvServer As New Button
                With rmvServer
                    .Top = 8
                    .Left = 322
                    .Height = 20
                    .Width = 74
                    .Name = "rmvServer" & UTab - 20
                    .Text = "del. server"
                    .UseVisualStyleBackColor = True
                    .Parent = SubTab(i)
                    AddHandler .Click, AddressOf removeServer_click
                End With
            End If
        Next
        'höhe des sub tabs & gruppen ermitteln
        Dim SubToolNum(30) As Double            '30 für anzahl tabs(untertabs von bnet) von 21-30
        Dim GroupToolNum(40) As Double          '40 für anzahl tabs(gruppen auf tab 3) von 31-35
        For i As Integer = 1 To UBound(type)
            If tab(i) > 20 And tab(i) <= 30 Then
                If type(i) = 1 Or type(i) = 11 Or type(i) = 12 Then
                    SubToolNum(tab(i)) += 1
                ElseIf type(i) = 2 Or type(i) = 3 Then
                    SubToolNum(tab(i)) += 0.5
                End If
            ElseIf tab(i) > 30 Then
                If type(i) = 1 Or type(i) = 11 Or type(i) = 12 Then
                    GroupToolNum(tab(i)) += 1
                ElseIf type(i) = 2 Or type(i) = 3 Then
                    GroupToolNum(tab(i)) += 0.5
                End If
            End If
        Next
        'höhe mit der anzahl der tools ausrechnen 
        Me.TabControl2.Height = Math.Ceiling(SubToolNum.Max) * 23 + 33

        'höhe der Gruppen setzen
        Me.GroupAGame.Height = Math.Ceiling(GroupToolNum(31)) * 23 + 12
        Me.GroupUdpTcp.Height = Math.Ceiling(GroupToolNum(32)) * 23 + 12
        Me.GroupAHost.Height = Math.Ceiling(GroupToolNum(33)) * 23 + 12
        Me.GroupReplay.Height = Math.Ceiling(GroupToolNum(34)) * 23 + 12
        Me.GroupLAN.Height = Math.Ceiling(GroupToolNum(35)) * 23 + 12
        'position der Gruppen setzen
        Me.GroupUdpTcp.Top = Me.GroupAGame.Top + Me.GroupAGame.Height + 3
        Me.GroupAHost.Top = Me.GroupUdpTcp.Top + Me.GroupUdpTcp.Height + 3
        Me.GroupReplay.Top = Me.GroupAHost.Top + Me.GroupAHost.Height + 3
        Me.GroupLAN.Top = Me.GroupReplay.Top + Me.GroupReplay.Height + 3

        'sortieren: textboxen zuerst
        For a As Integer = 1 To UBound(LTool, 2)
            If type(a) = 1 Or type(a) = 11 Or type(a) = 12 Then
                If tab(a) <> 0 Then
                    If LTool(0, tab(a)) <> 0 Then
                        TextBox(a).Top = TextBox(LTool(0, tab(a))).Top + TextBox(LTool(0, tab(a))).Height + 3
                    Else
                        If tab(a) <> 2 And tab(a) < 31 Then
                            TextBox(a).Top = 8
                        ElseIf tab(a) = 2 Then
                            TextBox(a).Top = TabControl2.Height + 12
                        ElseIf tab(a) > 30 Then
                            TextBox(a).Top = 10
                        End If
                    End If
                    Label(a).Top = TextBox(a).Top + 2
                    If Len(schluessel(a)) > 18 Then                                 'wenn schluessel(a) zu lang ist
                        TextBox(a).Left = 148
                    Else
                        TextBox(a).Left = 108
                    End If
                    If type(a) = 11 Or type(a) = 12 Then Button(a).Top = TextBox(a).Top
                    If type(a) = 12 Then ColorCBox(a).Top = TextBox(a).Top + 4

                    'neu erstellte tools zu tabs zuweisen
                    If tab(a) = 1 Then
                        Label(a).Parent = Me.TabBot
                        TextBox(a).Parent = Me.TabBot
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = Me.TabBot
                        If type(a) = 12 Then ColorCBox(a).Parent = Me.TabBot
                    ElseIf tab(a) = 2 Then
                        Label(a).Parent = Me.TabBnet
                        TextBox(a).Parent = Me.TabBnet
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = Me.TabBnet
                        If type(a) = 12 Then ColorCBox(a).Parent = Me.TabBnet
                    ElseIf tab(a) = 3 Then
                        Label(a).Parent = Me.TabAgame
                        TextBox(a).Parent = Me.TabAgame
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = Me.TabAgame
                        If type(a) = 12 Then ColorCBox(a).Parent = Me.TabAgame
                    ElseIf tab(a) = 4 Then
                        Label(a).Parent = Me.Tabdb
                        TextBox(a).Parent = Me.Tabdb
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = Me.Tabdb
                        If type(a) = 12 Then ColorCBox(a).Parent = Me.Tabdb
                    ElseIf tab(a) > 20 And tab(a) <= 30 Then
                        Label(a).Parent = SubTab(tab(a) - 20)
                        TextBox(a).Parent = SubTab(tab(a) - 20)
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = SubTab(tab(a) - 20)
                        If type(a) = 12 Then ColorCBox(a).Parent = SubTab(tab(a) - 20)
                    ElseIf tab(a) = 31 Then
                        Label(a).Parent = Me.GroupAGame
                        TextBox(a).Parent = Me.GroupAGame
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = Me.GroupAGame
                        If type(a) = 12 Then ColorCBox(a).Parent = Me.GroupAGame
                    ElseIf tab(a) = 32 Then
                        Label(a).Parent = Me.GroupUdpTcp
                        TextBox(a).Parent = Me.GroupUdpTcp
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = Me.GroupUdpTcp
                        If type(a) = 12 Then ColorCBox(a).Parent = Me.GroupUdpTcp
                    ElseIf tab(a) = 33 Then
                        Label(a).Parent = Me.GroupAHost
                        TextBox(a).Parent = Me.GroupAHost
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = Me.GroupAHost
                        If type(a) = 12 Then ColorCBox(a).Parent = Me.GroupAHost
                    ElseIf tab(a) = 34 Then
                        Label(a).Parent = Me.GroupReplay
                        TextBox(a).Parent = Me.GroupReplay
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = Me.GroupReplay
                        If type(a) = 12 Then ColorCBox(a).Parent = Me.GroupReplay
                    ElseIf tab(a) = 35 Then
                        Label(a).Parent = Me.GroupLAN
                        TextBox(a).Parent = Me.GroupLAN
                        If type(a) = 11 Or type(a) = 12 Then Button(a).Parent = Me.GroupLAN
                        If type(a) = 12 Then ColorCBox(a).Parent = Me.GroupLAN
                    End If
                    Label(a).Left = TextBox(a).Left - (Label(a).Width + 2)
                    If type(a) = 11 Or type(a) = 12 Then Button(a).Left = TextBox(a).Left + TextBox(a).Width + 3
                    If type(a) = 12 Then ColorCBox(a).Left = Button(a).Left + Button(a).Width + 3
                    LTool(0, tab(a)) = a
                End If
            End If
        Next
        'NummericUpDown & Checkboxen zählen
        For a As Integer = 1 To UBound(LTool, 2)
            If (type(a) = 2 Or type(a) = 3) And tab(a) <> 0 Then
                count(tab(a)) += 1
            End If
        Next
        For a As Integer = 0 To UBound(count)
            count(a) = Math.Ceiling(count(a) / 2)
        Next
        'letzte textbox merken
        For a As Integer = 1 To UBound(LTool, 2)
            LTool(1, a) = LTool(0, a)
        Next
        'sortieren: Nummeric unter textboxen
        For a As Integer = 1 To UBound(LTool, 2)
            If type(a) = 2 Then
                If tab(a) <> 0 Then
                    If count(tab(a)) > 0 Then                          'erste oda zweite reihe
                        Numeric(a).Left = 108
                    Else
                        If count(tab(a)) = 0 Then
                            LTool(0, tab(a)) = LTool(1, tab(a))
                        End If
                        Numeric(a).Left = 308
                    End If
                    count(tab(a)) -= 1
                    If LTool(0, tab(a)) <> 0 Then                       'oberster platz?
                        If LTool(1, tab(a)) = LTool(0, tab(a)) Then     'unter textbox?
                            Numeric(a).Top = TextBox(LTool(0, tab(a))).Top + TextBox(LTool(0, tab(a))).Height + 3
                        Else
                            Numeric(a).Top = Numeric(LTool(0, tab(a))).Top + Numeric(LTool(0, tab(a))).Height + 3
                        End If
                    Else
                        If tab(a) <> 2 And tab(a) < 31 Then
                            Numeric(a).Top = 8
                        ElseIf tab(a) = 2 Then
                            Numeric(a).Top = TabControl2.Height + 12
                        ElseIf tab(a) > 30 Then
                            Numeric(a).Top = 10
                        End If
                    End If
                    Label(a).Top = Numeric(a).Top + 2
                    If tab(a) = 1 Then                                 'neu erstellte tools zu tabs zuweisen
                        Label(a).Parent = Me.TabBot
                        Numeric(a).Parent = Me.TabBot
                    ElseIf tab(a) = 2 Then
                        Label(a).Parent = Me.TabBnet
                        Numeric(a).Parent = Me.TabBnet
                    ElseIf tab(a) = 3 Then
                        Label(a).Parent = Me.TabAgame
                        Numeric(a).Parent = Me.TabAgame
                    ElseIf tab(a) = 4 Then
                        Label(a).Parent = Me.Tabdb
                        Numeric(a).Parent = Me.Tabdb
                    ElseIf tab(a) > 20 And tab(a) <= 30 Then
                        Label(a).Parent = SubTab(tab(a) - 20)
                        Numeric(a).Parent = SubTab(tab(a) - 20)
                    ElseIf tab(a) = 31 Then
                        Label(a).Parent = Me.GroupAGame
                        Numeric(a).Parent = Me.GroupAGame
                    ElseIf tab(a) = 32 Then
                        Label(a).Parent = Me.GroupUdpTcp
                        Numeric(a).Parent = Me.GroupUdpTcp
                    ElseIf tab(a) = 33 Then
                        Label(a).Parent = Me.GroupAHost
                        Numeric(a).Parent = Me.GroupAHost
                    ElseIf tab(a) = 34 Then
                        Label(a).Parent = Me.GroupReplay
                        Numeric(a).Parent = Me.GroupReplay
                    ElseIf tab(a) = 35 Then
                        Label(a).Parent = Me.GroupLAN
                        Numeric(a).Parent = Me.GroupLAN
                    End If
                    Label(a).Left = Numeric(a).Left - (Label(a).Width + 2)
                    LTool(0, tab(a)) = a
                End If
            End If
        Next
        'letzte NumericUpDown merken
        For a As Integer = 1 To UBound(LTool, 2)
            LTool(2, a) = LTool(0, a)
        Next
        'sortieren: nach NummericUpDown die Checkboxen
        For a As Integer = 1 To UBound(LTool, 2)
            If type(a) = 3 Then
                If tab(a) <> 0 Then
                    If count(tab(a)) > 0 Then                              'erste oda zweite reihe
                        CheckBox(a).Left = 108
                    Else
                        If count(tab(a)) = 0 Then
                            LTool(0, tab(a)) = LTool(1, tab(a))
                        End If
                        CheckBox(a).Left = 258
                    End If
                    count(tab(a)) -= 1
                    If LTool(0, tab(a)) <> 0 Then                           'oberster platz?
                        If LTool(1, tab(a)) = LTool(0, tab(a)) Then         'unter textbox?
                            CheckBox(a).Top = TextBox(LTool(0, tab(a))).Top + TextBox(LTool(0, tab(a))).Height + 5
                        ElseIf LTool(2, tab(a)) = LTool(0, tab(a)) Then     'unter numeric?
                            CheckBox(a).Top = Numeric(LTool(0, tab(a))).Top + Numeric(LTool(0, tab(a))).Height + 5
                        Else
                            CheckBox(a).Top = CheckBox(LTool(0, tab(a))).Top + CheckBox(LTool(0, tab(a))).Height + 6
                        End If
                    Else
                        If tab(a) <> 2 And tab(a) < 31 Then
                            CheckBox(a).Top = 8
                        ElseIf tab(a) = 2 Then
                            CheckBox(a).Top = TabControl2.Height + 12
                        ElseIf tab(a) > 30 Then
                            CheckBox(a).Top = 10
                        End If
                    End If
                    If tab(a) = 1 Then                                     'neu erstellte tools zu tabs zuweisen
                        CheckBox(a).Parent = Me.TabBot
                    ElseIf tab(a) = 2 Then
                        CheckBox(a).Parent = Me.TabBnet
                    ElseIf tab(a) = 3 Then
                        CheckBox(a).Parent = Me.TabAgame
                    ElseIf tab(a) = 4 Then
                        CheckBox(a).Parent = Me.Tabdb
                    ElseIf tab(a) > 20 And tab(a) <= 30 Then
                        CheckBox(a).Parent = SubTab(tab(a) - 20)
                    ElseIf tab(a) = 31 Then
                        CheckBox(a).Parent = Me.GroupAGame
                    ElseIf tab(a) = 32 Then
                        CheckBox(a).Parent = Me.GroupUdpTcp
                    ElseIf tab(a) = 33 Then
                        CheckBox(a).Parent = Me.GroupAHost
                    ElseIf tab(a) = 34 Then
                        CheckBox(a).Parent = Me.GroupReplay
                    ElseIf tab(a) = 35 Then
                        CheckBox(a).Parent = Me.GroupLAN
                    End If
                    LTool(0, tab(a)) = a
                End If
            End If
        Next

        'fenster anschaulicher machen
        Me.Width = 460
        Dim Tall(300) As Integer                            'MAX
        Tall(0) = Me.GroupLAN.Top + Me.GroupLAN.Height - 20
        For a As Integer = 1 To UBound(Tall)
            If tab(a) <> 0 Then
                If type(a) = 1 Or type(a) = 11 Or type(a) = 12 Then
                    Tall(a) = TextBox(a).Top
                ElseIf type(a) = 2 Then
                    Tall(a) = Numeric(a).Top
                ElseIf type(a) = 3 Then
                    Tall(a) = CheckBox(a).Top
                End If
            End If
        Next
        'bnet subtabs & gruppen auch berücksichtigen
        If (TabControl2.Height + TabControl2.Top) > (GroupLAN.Top + GroupLAN.Height) Then
            Tall(0) = TabControl2.Height - 10
        Else
            Tall(0) = GroupLAN.Top + GroupLAN.Height - 20
        End If

        'kleine desktop größen berücksichtigen
        Dim TallMax As Integer = Tall.Max
        If Tall.Max > My.Computer.Screen.Bounds.Height - 300 Then
            TallMax = My.Computer.Screen.Bounds.Height - 300
        End If
        TabControl1.Height = TallMax + 47
        Me.Height = TabControl1.Height + 107
        InfoLabel.Top = TabControl1.Top + TabControl1.Height + 1
        InfoLabel.Height = 89
        Me.Text = "Bot Configuration - " & version
        Me.CenterToScreen()
    End Sub

    Private Sub Form1_Shown(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.Shown
        AppError()

        'MouseMove events objekten zuordnen
        AddHandler TabBot.MouseMove, AddressOf Any_MouseMove
        AddHandler TabBnet.MouseMove, AddressOf Any_MouseMove
        AddHandler TabAgame.MouseMove, AddressOf Any_MouseMove
        AddHandler Tabdb.MouseMove, AddressOf Any_MouseMove
        AddHandler GroupAGame.MouseMove, AddressOf Any_MouseMove
        AddHandler GroupAHost.MouseMove, AddressOf Any_MouseMove
        AddHandler GroupLAN.MouseMove, AddressOf Any_MouseMove
        AddHandler GroupReplay.MouseMove, AddressOf Any_MouseMove
        AddHandler GroupUdpTcp.MouseMove, AddressOf Any_MouseMove

        'systemkompatibilität
        If My.Computer.Info.OSFullName.Contains("Vista") Then
            HeigthAdd = 0
        Else
            HeigthAdd = 5
        End If
    End Sub

    Private Sub Form1_FormClosing(ByVal sender As Object, ByVal e As System.Windows.Forms.FormClosingEventArgs) Handles Me.FormClosing
        'werte von tools auslesen
        For i As Integer = 1 To UBound(wert)
            If tab(i) <> 0 Then
                If type(i) = 1 Or type(i) = 11 Then
                    wert(i) = TextBox(i).Text
                ElseIf type(i) = 2 Then
                    wert(i) = Numeric(i).Text
                ElseIf type(i) = 3 Then
                    wert(i) = -CInt(CheckBox(i).Checked)
                ElseIf type(i) = 12 Then
                    If wert(i).StartsWith("|cFF") And IsHex(Mid(wert(i), 5, 2)) And IsHex(Mid(wert(i), 7, 2)) And _
                    IsHex(Mid(wert(i), 9, 2)) And ColorCBox(i).Checked Then
                        wert(i) = Mid(wert(i), 1, 10) & TextBox(i).Text
                    Else
                        wert(i) = TextBox(i).Text
                    End If
                End If
            End If
        Next

        'werte von variablen in "textlines" übernehmen
        For i As Integer = 1 To UBound(line)
            If line(i) <> 0 Then
                textlines(line(i)) = key(i) & " = " & wert(i)
            End If
        Next

        'in cfg schreiben & cfg pfad speichern
        If (ErrorN(0) > 3 Or ErrorN(0) = 0) And Not Me.dontSaveCB.Checked Then
            Try
                IO.File.WriteAllLines(pfad, textlines)

                'ghost nach speichern starten
                If Me.runGhostCB.Checked Then
                    Dim Ghost As New System.Diagnostics.Process()
                    Ghost.StartInfo.WorkingDirectory = My.Computer.FileSystem.GetParentPath(pfad)
                    Ghost.StartInfo.FileName = My.Computer.FileSystem.GetParentPath(pfad) & "\ghost.exe "
                    Ghost.Start()
                End If

                'my setting verwaltung
                If Not My.Settings.CfgPath = "" Or My.Settings.CfgPath <> Me.CfgPathBox.Text Then
                    My.Settings.CfgPath = Me.CfgPathBox.Text
                    If My.Settings.CfgPath = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath) & "\ghost.cfg" Then
                        My.Settings.CfgPath = ""
                    End If
                    My.Settings.Save()
                End If
            Catch ex As Exception
                If Not My.Computer.FileSystem.FileExists(pfad & ".bak") Then
                    IO.File.WriteAllLines(pfad & ".bak", textlines)
                    ErrorN(0) = 9
                Else
                    ErrorN(0) = 10
                End If
                AppError()
            End Try
        ElseIf ErrorN(0) = 1 Or ErrorN(0) = 3 Then
            If My.Settings.CfgPath <> "" Then
                My.Settings.CfgPath = ""
                My.Settings.Save()
            End If
        ElseIf Me.dontSaveCB.Checked And Me.runGhostCB.Checked Then
            'wenn run ghost + dont save aktive ist
            Dim Ghost As New System.Diagnostics.Process()
            Ghost.StartInfo.WorkingDirectory = My.Computer.FileSystem.GetParentPath(pfad)
            Ghost.StartInfo.FileName = My.Computer.FileSystem.GetParentPath(pfad) & "\ghost.exe "
            Ghost.Start()
        End If
    End Sub

    Private Sub Update_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles BtnUpdate.Click
        OpenACfg.FileName = ""
        OpenACfg.InitialDirectory = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath)
        If OpenACfg.ShowDialog = Windows.Forms.DialogResult.OK Then
            'aktuelle werte in fariablen speichern
            For i As Integer = 1 To UBound(wert)
                If tab(i) <> 0 Then
                    If type(i) = 1 Or type(i) = 11 Then
                        wert(i) = TextBox(i).Text
                    ElseIf type(i) = 2 Then
                        wert(i) = Numeric(i).Text
                    ElseIf type(i) = 3 Then
                        wert(i) = -CInt(CheckBox(i).Checked)
                    ElseIf type(i) = 12 Then
                        If wert(i).StartsWith("|cFF") And IsHex(Mid(wert(i), 5, 2)) And IsHex(Mid(wert(i), 7, 2)) And _
                        IsHex(Mid(wert(i), 9, 2)) And ColorCBox(i).Checked Then
                            wert(i) = Mid(wert(i), 1, 10) & TextBox(i).Text
                        Else
                            wert(i) = TextBox(i).Text
                        End If
                    End If
                End If
            Next

            'update variablen erstellen
            Dim oldPfad As String = OpenACfg.FileName
            Dim oldTextlines() As String = IO.File.ReadAllLines(oldPfad)
            Dim oldtempline, oldKey(300), oldWert(300), Teile() As String       'MAX
            Dim d As Integer = 1
            Dim iWerte As Integer
            ErrorN(8) = 0
            For i As Integer = 0 To UBound(oldTextlines)
                oldtempline = LTrim(oldTextlines(i))
                If oldtempline = "" Then
                    'nichts unternehmen (debug)
                ElseIf Not IsSubfix(oldtempline) Then
                    'nichts unternehmen (aussortieren der infos)
                ElseIf oldtempline.Contains("=") Then
                    'oldKey & oldWert trennen
                    Teile = Split(oldTextlines(i), "=")
                    oldKey(d) = Trim(Teile(0))
                    oldWert(d) = LTrim(Teile(1))
                    d += 1
                End If
            Next
            If d = 1 Then ErrorN(8) = 1 '                                   error: keine Werte gefunden!

            'werte von alter cfg übernehmen
            For t As Integer = 1 To UBound(oldKey)
                For i As Integer = 1 To UBound(key)
                    If key(i) = oldKey(t) And Not oldWert(t) = "" And Not wert(i) = oldWert(t) Then
                        wert(i) = oldWert(t)
                        iWerte += 1
                    End If
                Next
            Next

            'werte im interface aktualiesieren
            For a As Integer = 1 To UBound(key)
                If key(a) <> "" Then
                    If Not key(a).StartsWith("#") Then
                        If type(a) = 1 Or type(a) = 11 Or type(a) = 12 Then
                            TextBox(a).Text = wert(a)
                            If type(a) = 12 Then
                                If wert(a).StartsWith("|cFF") And IsHex(Mid(wert(a), 5, 2)) And IsHex(Mid(wert(a), 7, 2)) And _
                                        IsHex(Mid(wert(a), 9, 2)) Then
                                    Button(a).BackColor = Color.FromArgb(CInt(CHex(False, Mid(wert(a), 5, 2))), _
                                                                CInt(CHex(False, Mid(wert(a), 7, 2))), _
                                                                CInt(CHex(False, Mid(wert(a), 9, 2))))
                                    TextBox(a).Text = Mid(wert(a), 11)
                                    ColorCBox(a).Checked = True
                                    Button(a).Enabled = True
                                Else
                                    Button(a).Enabled = False
                                    Button(a).BackColor = Color.WhiteSmoke
                                    ColorCBox(a).Checked = False
                                End If
                            End If
                        ElseIf type(a) = 2 Then
                            Numeric(a).Text = wert(a)
                        ElseIf type(a) = 3 Then
                            CheckBox(a).Checked = CBool(wert(a))
                        End If
                    End If
                End If
            Next
            If iWerte > 0 Then 'Meldung: Erfolg!
                MsgBox("Success, " & iWerte & " settings were updated!", MsgBoxStyle.Information, "Update finished.")
                Me.InfoLabel.Text = "Settings updated, they are ready to be checked or edited!"
            ElseIf Not ErrorN(8) = 1 Then
                ErrorN(8) = 2                                           'error: keine übereinstimmungen gefunden!
            End If
            AppError()
        End If
    End Sub

    Private Sub addServer_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles addServer.Click
        'error überprüfen
        If Not TabControl2.TabCount < 10 Then
            ErrorN(9) = 1
            AppError()
            Exit Sub
        End If
        'Bnet schlüssel listen
        Dim runden As Integer = 1
        Dim TextZeilen As Integer
        zedStr = "_"
        For Each Me.keyStr In Array.FindAll(key, AddressOf FindBNetKey)
            BNetIndex(runden) = Array.FindIndex(key, AddressOf FindKey)
            runden += 1
        Next
        Array.Resize(BNetIndex, runden)
        'zeilen zählen
        For i As Integer = 1 To UBound(BNetIndex)
            For Each zeile In info(BNetIndex(i)).Split(Chr(13))
                If zeile <> "NO description available!" Then
                    TextZeilen += 1
                Else
                    TextZeilen -= 1
                End If
            Next
        Next
        'textlines länge anpassen
        TextZeilen += runden * 2 - 3                    'array länge kompensieren + keine zeile am ende
        Dim AktZeile As Integer = textlines.Length + 1  'eine zeile freilassen
        Array.Resize(textlines, AktZeile + TextZeilen)

        'neue bnet werte in array schreiben
        Dim BnetNo As Integer = TabControl2.TabCount + 1
        For i As Integer = 1 To UBound(BNetIndex)
            Dim infoZeile As String() = info(BNetIndex(i)).Split(Chr(13))
            For Each zeile In infoZeile
                If zeile <> "NO description available!" Then
                    textlines(AktZeile) = "### " & zeile
                    AktZeile += 1
                Else
                    AktZeile -= 1
                End If
            Next
            If wert(BNetIndex(i)) = "1" Or wert(BNetIndex(i)) = "0" Then
                textlines(AktZeile) = "bnet" & BnetNo & "_" & schluessel(BNetIndex(i)) & " = " & wert(BNetIndex(i))
            Else
                textlines(AktZeile) = "bnet" & BnetNo & "_" & schluessel(BNetIndex(i)) & " = "
            End If
            AktZeile += 2
        Next

        'app neustart
        Application.Restart()
    End Sub

    Private Sub removeServer_click(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim temp As Button = CType(sender, Button)
        zedStr = Mid(temp.Name, 10)
        Dim infoIndex(150), Runs, IRuns, zahler As Integer

        'bnet key identifizieren
        For Each Me.keyStr In Array.FindAll(key, AddressOf FindBNetKey)
            Runs += 1
            BNetIndex(Runs) = Array.FindIndex(key, AddressOf FindKey)
        Next

        'infos suchen
        For i As Integer = 1 To UBound(BNetIndex)
            zahler = (line(BNetIndex(i))) - 1
            'If zahler > 0 Then
            'If Not (textlines(zahler).StartsWith("#") And textlines(zahler) = "") And _
            'textlines(zahler - 1).StartsWith("#") And textlines(zahler - 1) = "" Then
            '    zahler -= 1
            'End If
            'End If

            While zahler > 0
                If (textlines(zahler).StartsWith("#") Or textlines(zahler) = "") And IRuns <= 150 Then
                    IRuns += 1
                    infoIndex(IRuns) = zahler
                    zahler -= 1
                Else
                    zahler = 0
                End If
            End While
        Next

        'keys zurücksetzen
        For i As Integer = 1 To UBound(BNetIndex)
            If BNetIndex(i) > 0 Then
                type(BNetIndex(i)) = 0
                For be As Integer = line(BNetIndex(i)) To UBound(textlines) - 1
                    textlines(be) = textlines(be + 1)
                Next
                For da As Integer = 1 To UBound(line)
                    If line(da) > line(BNetIndex(i)) Then line(da) -= 1
                Next
                For hu As Integer = 1 To UBound(infoIndex)
                    If infoIndex(hu) > line(BNetIndex(i)) Then infoIndex(hu) -= 1
                Next
                line(BNetIndex(i)) = 0
            End If
        Next

        'infos entfernen
        For i As Integer = 1 To UBound(infoIndex)
            If infoIndex(i) > 0 Then
                For be As Integer = infoIndex(i) To UBound(textlines) - 1
                    textlines(be) = textlines(be + 1)
                Next
                For da As Integer = 1 To UBound(line)
                    If line(da) > infoIndex(i) Then line(da) -= 1
                Next
                For ha As Integer = 1 To UBound(infoIndex)
                    If infoIndex(ha) > infoIndex(i) Then infoIndex(ha) -= 1
                Next
            End If
        Next
        'array größe setzen
        Array.Resize(textlines, textlines.Length - (Runs + IRuns))

        Application.Restart()
    End Sub

    Private Sub TabControl1_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles TabControl1.Click
        'standardeigenschaften setzen
        Me.CfgPathBox.Visible = False
        Me.CfgBrowserBtn.Visible = False
        Me.InfoLabel.Top = TabControl1.Top + TabControl1.Height + 1
        Me.Height = TabControl1.Height + 107

        'überschrift dem aktuellen tab anpassen
        If TabControl1.SelectedIndex = 0 Then
            Me.Text = TabBot.Text & " Configuration - " & version
        ElseIf TabControl1.SelectedIndex = 1 Then
            Me.Text = TabBnet.Text & " Configuration - " & version
        ElseIf TabControl1.SelectedIndex = 2 Then
            Me.Text = TabAgame.Text & " Configuration - " & version
        ElseIf TabControl1.SelectedIndex = 3 Then
            Me.Text = Tabdb.Text & " Configuration - " & version
        ElseIf TabControl1.SelectedIndex = 4 Then
            Me.Text = TabCfgUp.Text & " - " & version
            Me.InfoLabel.Text = "Ready to update settings!"
        ElseIf TabControl1.SelectedIndex = 5 Then
            Me.Text = "Configurator Help" & " - " & version
            Me.InfoLabel.Text = "The cfg file path:" & Chr(13) & Chr(13) & "Hint: clear this textbox to use the ghost.cfg located " & _
            "in the application's directory!"
            Me.CfgPathBox.Visible = True
            Me.CfgBrowserBtn.Visible = True
            Me.InfoLabel.Top = TabControl1.Top + TabControl1.Height + 3
            Me.CfgPathBox.Top = TabControl1.Top + TabControl1.Height + 1
            Me.CfgBrowserBtn.Top = TabControl1.Top + TabControl1.Height + 1
        End If

        If TabControl1.SelectedIndex < 4 Then
            Me.InfoLabel.Text = "Click at a Textbox or hover over a checkbox to get information about the setting!"
        End If
    End Sub

    Private Sub CfgBrowserBtn_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles CfgBrowserBtn.Click
        OpenACfg.FileName = ""
        OpenACfg.InitialDirectory = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath)
        If OpenACfg.ShowDialog = Windows.Forms.DialogResult.OK Then
            Me.CfgPathBox.Text = OpenACfg.FileName
        End If
    End Sub

    Private Sub runGhostCB_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles runGhostCB.Click
        If runGhostCB.Checked = True And _
        Not My.Computer.FileSystem.FileExists(My.Computer.FileSystem.GetParentPath(pfad) & "\ghost.exe") Then
            MsgBox("Ghost.exe can't be found, it won't be launched after closing!" & Chr(13) & _
                   "(You may start it manually.)", MsgBoxStyle.Information, "Ghost not found!")
            Me.runGhostCB.Checked = False
        End If
    End Sub

    Private Sub TextBox_click(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim TBox As TextBox = CType(sender, TextBox)
        InfoLabel.Text = info(CInt(Mid(TBox.Name, 8)))                          'zu auslöser passende info setzen(für Textboxen)
        If InfoLabel.Height > 55 - HeigthAdd Then                               'wenn info länger ist
            Me.Height = TabControl1.Height + InfoLabel.Height + 55 + HeigthAdd
        Else
            Me.Height = TabControl1.Height + 107
        End If
    End Sub

    Private Sub Numeric_click(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim Num As NumericUpDown = CType(sender, NumericUpDown)
        InfoLabel.Text = info(CInt(Mid(Num.Name, 14)))                          'zu auslöser passende info setzen(für NumericUpDown)
        If InfoLabel.Height > 55 - HeigthAdd Then                               'wenn info länger ist
            Me.Height = TabControl1.Height + InfoLabel.Height + 55 + HeigthAdd
        Else
            Me.Height = TabControl1.Height + 107
        End If
    End Sub

    Private Sub CheckBox_MouseMove(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim CBox As CheckBox = CType(sender, CheckBox)
        InfoNum = CInt(Mid(CBox.Name, 9))
        InfoCount = 3
        CBoxInfoTimer.Enabled = True                                             'info verzögerung aktivieren vor info änderung
    End Sub

    Private Sub ColorCheckBox_Click(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim temp As CheckBox = CType(sender, CheckBox)
        Dim a As Integer = Mid(temp.Name, 10)
        If temp.Checked Then
            Button(a).Enabled = True
            If wert(a).StartsWith("|cFF") And IsHex(Mid(wert(a), 5, 2)) And IsHex(Mid(wert(a), 7, 2)) And _
                IsHex(Mid(wert(a), 9, 2)) Then
                Button(a).BackColor = Color.FromArgb(CInt(CHex(False, Mid(wert(a), 5, 2))), _
                                            CInt(CHex(False, Mid(wert(a), 7, 2))), _
                                            CInt(CHex(False, Mid(wert(a), 9, 2))))
            End If
        Else
            Button(a).Enabled = False
            Button(a).BackColor = Color.WhiteSmoke
        End If
    End Sub

    Private Sub CBoxInfo_Tick(ByVal sender As Object, ByVal e As System.EventArgs) Handles CBoxInfoTimer.Tick
        InfoCount -= 1
        If InfoCount = 0 Then
            CBoxInfoTimer.Enabled = False
            InfoLabel.Text = info(InfoNum)                                      'zu auslöser passende info setzen(für CheckBoxen)
            If InfoLabel.Height > 55 - HeigthAdd Then                           'wenn info länger ist
                Me.Height = TabControl1.Height + InfoLabel.Height + 55 + HeigthAdd
            Else
                Me.Height = TabControl1.Height + 107
            End If
        End If
    End Sub

    Public Sub Any_MouseMove()
        CBoxInfoTimer.Enabled = False
    End Sub

    Private Sub ButtonFolder_click(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim temp As Button = CType(sender, Button)
        Dim TNum As Integer = Mid(temp.Name, 7)
        If DirBrowser(TNum).ShowDialog() = Windows.Forms.DialogResult.OK Then
            If Not DirBrowser(TNum).SelectedPath.EndsWith("\") Then
                TextBox(TNum).Text = DirBrowser(TNum).SelectedPath & "\"
            Else
                TextBox(TNum).Text = DirBrowser(TNum).SelectedPath
            End If
        End If
    End Sub

    Private Sub ButtonColor_click(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim temp As Button = CType(sender, Button)
        Dim l As Integer = Mid(temp.Name, 7)
        If wert(l).StartsWith("|cFF") And IsHex(Mid(wert(l), 5, 2)) And IsHex(Mid(wert(l), 7, 2)) And _
        IsHex(Mid(wert(l), 9, 2)) Then
            ColorWin(l).Color = Color.FromArgb(CInt(CHex(False, Mid(wert(l), 5, 2))), CInt(CHex(False, Mid(wert(l), 7, 2))), _
                                                                       CInt(CHex(False, Mid(wert(l), 9, 2))))
        End If
        If ColorWin(l).ShowDialog = Windows.Forms.DialogResult.OK Then
            wert(l) = "|cFF" & CHex(True, ColorWin(l).Color.R) & CHex(True, ColorWin(l).Color.G) & CHex(True, ColorWin(l).Color.B) & _
                                    Mid(wert(l), 11)
            Button(l).BackColor = Color.FromArgb(CInt(CHex(False, Mid(wert(l), 5, 2))), _
                                                    CInt(CHex(False, Mid(wert(l), 7, 2))), _
                                                    CInt(CHex(False, Mid(wert(l), 9, 2))))
        End If
    End Sub

    Private Function IsSubfix(ByVal text As String) As Boolean
        If text.StartsWith("bot_") Or text.StartsWith("admingame_") Or text.StartsWith("db_") Or text.StartsWith("bnet") Or _
           text.StartsWith("udp_") Or text.StartsWith("tcp_") Or text.StartsWith("lan_") Or text.StartsWith("autohost_") Or _
           text.StartsWith("replay_") Then
            Return True
        Else
            Return False
        End If
    End Function

    Private Function openCfgBrowse() As Boolean
        'info file not found!
        Dim msg As MsgBoxResult = MsgBox("Unable to load the ghost.cfg!" & Chr(13) & Chr(13) & _
                                         "Click " & Chr(34) & "ok" & Chr(34) & " to browse for the cfg file OR click " & Chr(34) & _
                                         "abort" & Chr(34) & " to close the application!" & Chr(13) & _
                                         "(You may move this application into the ghost++ directory.)", MsgBoxStyle.OkCancel, _
                                         "Cfg not loaded!")
        If msg = MsgBoxResult.Ok Then
            OpenACfg.InitialDirectory = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath)
            If OpenACfg.ShowDialog = Windows.Forms.DialogResult.OK Then
                pfad = OpenACfg.FileName
                My.Settings.CfgPath = OpenACfg.FileName
                Me.CfgPathBox.Text = OpenACfg.FileName
                Return False
            Else
                Return True
            End If
        Else
            Return True
        End If
    End Function

    Private Function CHex(ByVal way As Boolean, ByVal num As String) As String
        Dim temp(1) As String
        If way Then                                         'dezimalzahlen -> hexadezimalzahlen
            temp(0) = Math.Floor(num / 16)
            temp(1) = num Mod 16
            For i As Integer = 0 To 1
                If temp(i) >= 0 And temp(i) <= 9 Then
                    'nichst ändern
                ElseIf temp(i) = 10 Then
                    temp(i) = "A"
                ElseIf temp(i) = 11 Then
                    temp(i) = "B"
                ElseIf temp(i) = 12 Then
                    temp(i) = "C"
                ElseIf temp(i) = 13 Then
                    temp(i) = "D"
                ElseIf temp(i) = 14 Then
                    temp(i) = "E"
                ElseIf temp(i) = 15 Then
                    temp(i) = "F"
                End If
            Next
            num = temp(0) & temp(1)
        Else                                                'hexadezimalzahlen -> dezimalzahlen
            temp(0) = Mid(num, 1, 1)
            temp(1) = Mid(num, 2, 1)
            For i As Integer = 0 To 1
                If UCase(temp(i)) = "A" Then
                    temp(i) = 10
                ElseIf UCase(temp(i)) = "B" Then
                    temp(i) = 11
                ElseIf UCase(temp(i)) = "C" Then
                    temp(i) = 12
                ElseIf UCase(temp(i)) = "D" Then
                    temp(i) = 13
                ElseIf UCase(temp(i)) = "E" Then
                    temp(i) = 14
                ElseIf UCase(temp(i)) = "F" Then
                    temp(i) = 15
                ElseIf temp(i) >= 0 And temp(i) <= 9 Then
                    'nichts ändern
                End If
            Next
            num = CInt(temp(0)) * 16 + CInt(temp(1))
        End If
        Return num
    End Function

    Private Function IsHex(ByVal hex As String) As Boolean
        Dim chr() As String = {Mid(hex, 1, 1), Mid(hex, 2, 1)}
        Dim isTrue(1) As Integer
        For i As Integer = 0 To 1
            If IsNumeric(chr(i)) Then
                isTrue(i) = 1
            ElseIf UCase(chr(i)) = "A" Then
                isTrue(i) = 1
            ElseIf UCase(chr(i)) = "B" Then
                isTrue(i) = 1
            ElseIf UCase(chr(i)) = "C" Then
                isTrue(i) = 1
            ElseIf UCase(chr(i)) = "D" Then
                isTrue(i) = 1
            ElseIf UCase(chr(i)) = "E" Then
                isTrue(i) = 1
            ElseIf UCase(chr(i)) = "F" Then
                isTrue(i) = 1
            End If
        Next
        Return CBool(isTrue(0) * isTrue(1))
    End Function

    Private Function FindBNetKey(ByVal str As String) As Boolean
        If Not str = Nothing Then
            If str.StartsWith("bnet" & zedStr) Then
                Return True
            Else
                Return False
            End If
        Else
            Return False
        End If
    End Function

    Private Function FindPasswordKey(ByVal str As String) As Boolean
        If str = "bnet" & zed & "_password" Then Return True Else Return False
    End Function

    Private Function FindKey(ByVal str As String) As Boolean
        If str = keyStr Then Return True Else Return False
    End Function

    Private Function IsKeyLoadInGame(ByVal str As String) As Boolean
        If str.StartsWith("map_loadingame") Then Return True Else Return False
    End Function

    Public Sub AppError()
        'configurator (start) fehler
        If ErrorN(0) = 1 Then
            MsgBox("Error during the data processing, try to change the cfg file path at restart " & _
                   "and respectively replace the ghost.cfg in the application's directory with another one!" _
                   , MsgBoxStyle.Critical, "Error #001")
            Me.Close()
        ElseIf ErrorN(0) = 2 Then
            'wenn keine cfg gefunden wurde & abbrechen
            Me.Close()
        ElseIf ErrorN(0) = 3 Then
            'anpassen: 
            MsgBox("There are no setting in the ghost.cfg," & Chr(13) & "make sure that the used cfg file is a ghost++ cfg!" & _
                   Chr(13) & Chr(13) & "The cfg file path have been reseted (to prevent bugs).", _
                   MsgBoxStyle.Critical, "Error #011")
            Me.Close()
        ElseIf ErrorN(0) = 9 Then
            MsgBox("Unable to write settings to the cfg file, make sure that the ghost.cfg file isn't write protected or " & _
                   "in use by another application!" & Chr(13) & Chr(13) & "The settings has been saved to " & Chr(34) & _
                   "ghost.cfg.bak" & Chr(34) & " (you have to rename this file to " & Chr(34) & "ghost.cfg" & Chr(34) & _
                   ", to use this settings).", MsgBoxStyle.Critical, "Error #010")
        ElseIf ErrorN(0) = 10 Then
            MsgBox("Unable to write settings to the cfg file, make sure that the ghost.cfg file isn't write protected or " & _
                   "in use by another application!", MsgBoxStyle.Critical, "Error #010")
        End If
        'bei error(0) restliche meldungen ignorieren
        If ErrorN(0) <> 0 Then
            Exit Sub
        End If

        'configurator meldungen
        If ErrorN(1) = 1 Then
            MsgBox("To many settings in the cfg file, maximum is " & UBound(type) & "!", MsgBoxStyle.Exclamation, "Error #002")
        End If
        If ErrorN(2) = 1 Then
            MsgBox("Syntax error in (the ghost.cfg in) following lines:" & Chr(13) & ErrorStr(2) & " a " & Chr(34) & "=" & Chr(34) & _
                   " is missing!", MsgBoxStyle.Exclamation, "Error #003")
        End If
        If ErrorN(3) = 1 Then
            MsgBox("Syntax error in (the ghost.cfg in) following lines:" & Chr(13) & ErrorStr(3) & "only 10 bnet servers are " & _
                   "supported!", MsgBoxStyle.Exclamation, "Error #004")
        End If
        If ErrorN(4) = 1 Then
            MsgBox("Syntax error in (the ghost.cfg in) following lines:" & Chr(13) & ErrorStr(4) & "only numbers after " & Chr(34) _
                   & "bnet" & Chr(34) & " are supported!", MsgBoxStyle.Exclamation, "Error #005")
        End If
        If ErrorN(6) = 1 Then
            MsgBox("Syntax error in (the ghost.cfg in) following lines:" & Chr(13) & ErrorStr(6) & " a " & Chr(34) & "#" & Chr(34) & _
                   " is missing!", MsgBoxStyle.Exclamation, "Error #006")
        End If
        'Updater fehler
        If ErrorN(8) = 1 Then
            MsgBox("Could not find any setting in the selected cfg file!", MsgBoxStyle.Exclamation, "Error #008")
            Me.InfoLabel.Text = "Error, no settings found, only files named " & Chr(34) & "ghost.cfg" & Chr(34) & " are supported!"
        ElseIf ErrorN(8) = 2 Then
            MsgBox("No differences found between the old cfg and the new one!", MsgBoxStyle.Exclamation, "Error #009")
            Me.InfoLabel.Text = "Error, no differences found between the two files!"
        End If
        'add Server fehler
        If ErrorN(9) = 1 Then
            MsgBox("Could not create more server settings, only 10 servers are supported!", MsgBoxStyle.Exclamation, "Error #015")
        End If
        Array.Clear(ErrorN, 0, UBound(ErrorN))
    End Sub
End Class

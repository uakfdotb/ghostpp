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

Public Class MainWindow
    Dim Label(300) As Label
    Dim TextBox(300) As TextBox
    Dim Numeric(300) As NumericUpDown
    Dim CheckBox(300), ColorCBox(300) As CheckBox
    Dim Button(300) As Button
    Dim DirBrowser(300) As FolderBrowserDialog
    Dim ColorWin(300) As ColorDialog
    Dim SubTab(10) As TabPage
    Dim ApplySettings As Boolean
    Dim DefaultCfgPath, MyCfgPath, version, KeyStr, BnetNoStr, ErrorStr(20), key(300), Schluessel(300), defWert(300), myWert(300), _
        Info(300), DefaultCfgFile(), myCfgFile() As String
    Dim InfoNum, InfoCount, BnetNo, HeigthAdd, ErrorN(20), BNetIndex(50), line(300), type(300), tab(300), LTool(2, 300) As Integer

    Public cAppDirPath, cGhostCfg, cDefaultCfg, cGhostExe As String

    'bugs:

    'features:


#Region "change log:"
    '-ab server 2 werden die folgenden einstellungen immer gespeichert: _server, _serveralias, _bnlsserver z338
    '-releas datum aktualiesieren
    '-beim startdialog wirklich in den papierkorb löschen
    '-create new was ist bei eigenen pfaden?
    '-auch beim start up dialog auf default pfade überprüfen? z1445
    '-für create new: wenn ghostCfgPath leer ist app dir mit ghost.cfg nehmen
    '-wenn default.cfg vorhanden & ghost.cfg nicht settings dialog auf creat new setzen
    '-meldung aktualisieren, bedinung auf file exists umstellen (dialog ok_ckick z43)
    '-ausnahme für create new (dialog me_validate z209)
    '-Bedingung für create new hinzufügen in der textInfo von TBghostCfg geändert wird. (dialog z 72)
    '-variablen für diese form globalisieren + neue variable für create new (dialog z28)
    '-bedingung für create new hinzufügen (dialog z82)
    '-readme eintrag der auf vb2008 bug mit custom control hinweist hinzufügen
    '-alle hauptfunktionen noch einmal durchgehen und testen ob diese in beinden modi richtig funktionieren
    '-shortcuts mit beschreibung hinzufügen
    '-hilfe aktualiesieren
    '-alten converter entfernen?
    '-bnet_locale = 0    (textbox)
    'tool select system überpüfen
    '-tooltips nur anzeigen wenn dual cfg aktive
    '-help page info anpassen
    '-fehlermeldungen für dual_cfg
    '-restliche fehlermeldungen nach appError auslagern
    '-für create new: ghost.cfg einfach so überschreiben lassen?
    '-sind die dateiüberprüfenen von startUpsettingsDialog noch nötig?
    '-server tab: button einstellung "von server 1" übernehmen
    '-option um werte auf standart zurückzusetzen hinzufügen(über tastatur zugreifbar?)
    '-beim umschalten von mono auf dual_cfg aufpassen am besten nur setting setzen und änderungen nach neustart übernehmen
    '-eintellungen zurücksetzbar (über button)
    '-create new ist zur laufteit noch nit unterstützt
    '-optionen auch während der laufzeit aufrufbar
    '-öffentliche variablen für das programm arbeitsverzeichnis + ghostCfg, defaultCfg, ghostExe eingerichtet
    '-verschiedene startsituationen durchgehen(ohne ghost, ohne default; ohne ghost, mit default;  mit ghost, ohne default)
    '-versuche myTextBox so abzuändern das textChanced nicht beim focuswegsel aufgerufen wird!
    '-startupdialog: rBtnCreateCfg.Checked ausahme hinzufügen für verzeichnis existiert nicht
    '-in settingsdialog ok_button enable moglichst von mouse move entkoppel
    '-start ghost after closeing aktualiesiert: in form1_closing, in CBrunGhost_click
    '-settingDialog button auch ausgrauen wenn erforderliche angabe fehlt( spetzielles handling für new ghost.cfg erforderlich)
    '-mytextbox bug nach eingabe wird dialogbox wert überschrieben
    '-remove_server funtz für mono_cfg nichtmehr(array wurde auf falsche länge gesetzt z1055
    '-server tab wieder auf 9 server begrenzen (ist sonst verwirrent)
    '-für bnet#_ werden alle einträge gespeichert die nicht gleich nichts sind
    '-configuration für bnet#_ werden im moment noch probleme machen
    '-in add_server handling für dual_cfg eingebaut z 1027
    '-in remove_server tab handling für dual_cfg hinzugefügt z1093
    '-in remove_server wird geprüft ob der entsprechende wert überhaupt ind der cfg steht z1043
    '-start rutiene hinzugefügt um mehrere bnet server zu ermöglichen z225
    '-dual config system [eine "default.cfg"(read only) und eine ghost.cfg in der nur die änderungen stehen]
    '-einzelne prozeduren fragen erst nach ob defwert oder mywert aktuell ist!
    '-settings beim start abklopfen z54
    '-speichervorgang auf myWert umgestellt
    '-ghost.cfg laden & werte auslesen z198
    'code optimierung "tempIndex" z217
    'Update -> UpToDate z43
    'lambda funktionen eingebaut bei:z210,z195,z193 & unter such funktionen
    'systemkompatibilität fragt jetzt nach OS Major Version
#End Region


#Region "Form1 Prozeduren"
    Private Sub Form1_Load(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.Load
        'start konfiguration
        Me.Icon = My.Resources.parametres_3D
        version = "v1.2b"
        'fenster initalisieren
        Dim CfgLine(), templine As String
        Dim count(40) As Integer            '40 für anzahl tabs(1-4, 21-40)
        Dim de As Integer = 1

        'benötigte variablen setzen
        cAppDirPath = My.Computer.FileSystem.GetParentPath(Application.ExecutablePath) + "\"
        cGhostCfg = "ghost.cfg"
        cDefaultCfg = "default.cfg"
        cGhostExe = "ghost.exe"

        DefaultCfgPath = cAppDirPath + cDefaultCfg
        MyCfgPath = cAppDirPath + cGhostCfg


        'falls nötig settings Upgrade
        If Not My.Settings.UpToDate Then
            My.Settings.Upgrade()
            My.Settings.UpToDate = True
        End If

        'prüfen ob erforderliche cfgs vorhanden sind
        If My.Settings.DualCfgMode Then

            If My.Settings.MyCfgPath <> Nothing AndAlso My.Computer.FileSystem.FileExists(My.Settings.MyCfgPath) Then
                MyCfgPath = My.Settings.MyCfgPath

            ElseIf My.Settings.MyCfgPath = Nothing AndAlso My.Computer.FileSystem.FileExists(MyCfgPath) Then
                'tu nichts
            Else
                If Not StartUpSettingsDialog() Then
                    ErrorN(0) = 2
                    Me.Close()
                    Exit Sub
                End If
            End If

            If My.Settings.DefaultCfgPath <> Nothing AndAlso My.Computer.FileSystem.FileExists(My.Settings.DefaultCfgPath) Then
                DefaultCfgPath = My.Settings.DefaultCfgPath

            ElseIf My.Settings.DefaultCfgPath = Nothing AndAlso My.Computer.FileSystem.FileExists(DefaultCfgPath) Then
                'tu nichts
            Else
                If Not StartUpSettingsDialog() Then
                    ErrorN(0) = 2
                    Me.Close()
                    Exit Sub
                End If
            End If
        Else

            If My.Settings.MyCfgPath <> Nothing AndAlso My.Computer.FileSystem.FileExists(My.Settings.MyCfgPath) Then
                MyCfgPath = My.Settings.MyCfgPath

            ElseIf My.Settings.MyCfgPath = Nothing AndAlso My.Computer.FileSystem.FileExists(MyCfgPath) Then
                'tu nichts
            Else
                If Not StartUpSettingsDialog() Then
                    ErrorN(0) = 2
                    Me.Close()
                    Exit Sub
                End If
            End If
            DefaultCfgPath = MyCfgPath
        End If

        'falls i was schief läuft error #001
        Try
            'defaul.cfg laden
            DefaultCfgFile = IO.File.ReadAllLines(DefaultCfgPath)

            For i As Integer = 0 To UBound(DefaultCfgFile)
                templine = LTrim(DefaultCfgFile(i))

                'sortieren von infos und keys
                If templine = "" Then
                    'nichts unternehmen (debug)
                ElseIf templine.StartsWith("#") Then
                    templine = LTrim(templine.Remove(0, templine.LastIndexOf("#") + 1))
                    If Not (templine.Contains("CONFIGURATION") Or IsSubfix(templine) Or templine.StartsWith("example")) And templine <> "" Then
                        Info(de) += LTrim(templine)
                        If Info(de) <> "" Then
                            Info(de) += Chr(13)
                        End If
                    End If
                ElseIf templine.Contains("=") And IsSubfix(templine) Then
                    line(de) = i
                    'schluessel & wert trennen
                    CfgLine = Split(DefaultCfgFile(i), "=")
                    key(de) = Trim(CfgLine(0))
                    Schluessel(de) = Trim(CfgLine(0))
                    'schluessel prefix entfernen
                    If Schluessel(de).StartsWith("bot_") Then
                        Schluessel(de) = Schluessel(de).Remove(0, 4)
                    ElseIf Schluessel(de).StartsWith("admingame_") Then
                        Schluessel(de) = Schluessel(de).Remove(0, 10)
                    ElseIf Schluessel(de).StartsWith("db_") Then
                        Schluessel(de) = Schluessel(de).Remove(0, 3)
                    ElseIf Schluessel(de).StartsWith("bnet_") Then
                        Schluessel(de) = Schluessel(de).Remove(0, 5)
                    ElseIf Schluessel(de).StartsWith("bnet") Then
                        If IsNumeric(Mid(Schluessel(de), 5, 1)) Then
                            If Mid(Schluessel(de), 5, 1) <= 9 Then      'max 9 bnet server
                                Schluessel(de) = Schluessel(de).Remove(0, 6)
                            Else
                                'bereits gesetzte variablen zurückgsetzten
                                key(de) = ""
                                Schluessel(de) = ""
                                line(de) = 0
                                ErrorN(3) = 1                                   'error: syntax fehler("bnet11_")
                                ErrorStr(3) += "  " & templine & Chr(13)
                            End If
                        Else
                            'bereits gesetzte variablen zurückgsetzten
                            key(de) = ""
                            Schluessel(de) = ""
                            line(de) = 0
                            ErrorN(4) = 1                                       'error: syntax fehler("bnetABC_")
                            ErrorStr(4) += "  " & templine & Chr(13)
                        End If
                    ElseIf Schluessel(de).StartsWith("udp_") Or Schluessel(de).StartsWith("tcp_") Or _
                    Schluessel(de).StartsWith("lan_") Then
                        Schluessel(de) = Schluessel(de).Remove(0, 4)
                    ElseIf Schluessel(de).StartsWith("autohost_") Then
                        Schluessel(de) = Schluessel(de).Remove(0, 9)
                    ElseIf Schluessel(de).StartsWith("replay_") Then
                        Schluessel(de) = Schluessel(de).Remove(0, 7)
                    End If
                    defWert(de) = LTrim(CfgLine(1))
                    'wert einordnen
                    If Info(de) <> "" Then
                        If (Info(de).Contains("port") Or Info(de).Contains("max") Or Info(de).Contains("Numeric") Or _
                        Info(de).Contains("method") Or Info(de).Contains("higher") Or Info(de).Contains("set to") Or _
                        Info(de).Contains("percent") Or Info(de).Contains("number") Or _
                        Schluessel(de).Contains("max") Or Schluessel(de).Contains("port")) _
                        And Not Info(de).Contains("locale") Then
                            'NummericUpDown
                            type(de) = 2
                        ElseIf defWert(de) = "0" Or defWert(de) = "1" Then
                            'checkbox
                            type(de) = 3
                        ElseIf Info(de).Contains("directory") Or Info(de).Contains("path") Then
                            'textbox mit dir browser
                            type(de) = 11
                        ElseIf Info(de).Contains("colour") Or Info(de).Contains("color") Then
                            'textbox mit farb auswahl
                            type(de) = 12
                        Else
                            'für alle nicht erkannte -> normale textbox
                            type(de) = 1
                        End If
                    Else
                        Info(de) += "NO description available!"
                        If Schluessel(de) <> "" Then
                            If Schluessel(de).Contains("max") Or Schluessel(de).Contains("port") Then
                                'NummericUpDown
                                type(de) = 2
                            ElseIf Schluessel(de).Contains("path") Then
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
            ErrorStr(0) = Strings.Left(ex.ToString, Strings.InStr(ex.ToString, ":") - 1)
            AppError()
            Exit Sub
        End Try

        'myCfg landen(nur bei bual cfg ausführen)
        Try
            If My.Settings.DualCfgMode = False Then Exit Try

            myCfgFile = IO.File.ReadAllLines(MyCfgPath)

            'suche nach mehreren bnet servern
            Dim BnetServers As Integer
            For i As Integer = 2 To 9   'max 9 bnet server
                BnetNoStr = i
                If Array.FindIndex(myCfgFile, FindBnetKey) <> -1 Then BnetServers = i
            Next

            'wenn mehrere bnet server vorhanden
            If BnetServers <> Nothing Then
                Dim runden As Integer
                BnetNoStr = "_"

                'bnet keys suchen
                For Each Me.KeyStr In Array.FindAll(key, FindBnetKey)
                    BNetIndex(runden) = Array.FindIndex(key, FindKey)
                    runden += 1
                Next
                runden -= 1
                ReDim Preserve BNetIndex(runden)

                'neue bnet keys schreiben
                Dim tmpAktKey As Integer = Array.IndexOf(key, Nothing, 1)
                For i As Integer = 2 To BnetServers

                    If tmpAktKey + If(i = 2, runden + 1, runden) > 300 Or tmpAktKey = -1 Then
                        ErrorN(1) = 1                                           'error: zu viele werte
                        Exit For
                    End If

                    For c As Integer = 0 To runden
                        key(tmpAktKey + c) = "bnet" & i & "_" & Schluessel(BNetIndex(c))
                        Schluessel(tmpAktKey + c) = Schluessel(BNetIndex(c))

                        'Debug: für bnet# einstellungen um bestimmte werte auf den ghost++ quellcode wert zu setzen
                        If Schluessel(BNetIndex(c)) <> "server" And Schluessel(BNetIndex(c)) <> "bnlsserver" And _
                           Schluessel(BNetIndex(c)) <> "serveralias" Then
                            defWert(tmpAktKey + c) = defWert(BNetIndex(c))
                        End If
                        type(tmpAktKey + c) = type(BNetIndex(c))
                        Info(tmpAktKey + c) = Info(BNetIndex(c))
                    Next
                    tmpAktKey += runden + 1
                Next
            End If

            For i As Integer = 0 To UBound(myCfgFile)
                templine = LTrim(myCfgFile(i))

                'werte auslesen
                If IsSubfix(templine) And Not templine = "" And templine.Contains("=") Then
                    CfgLine = Split(templine, "=")
                    Dim tempKey As String = Trim(CfgLine(0))
                    Dim tempValue As String = Trim(CfgLine(1))

                    'schlüssel in key suchen
                    Dim keyIndex As Integer = Array.IndexOf(key, tempKey)

                    'falls gefunden wert setzen
                    If keyIndex <> -1 Then myWert(keyIndex) = tempValue

                ElseIf Not templine.Contains("=") And IsSubfix(templine) Then
                    ErrorN(15) = 1                                              'error: syntax fehler(kein "=")
                    ErrorStr(15) &= "  " & templine & vbCr
                End If
            Next
        Catch ex As IndexOutOfRangeException
            ErrorN(16) = 1                                                      'error: zu viele bnet keys
        Catch ex As Exception
            ErrorN(0) = 11                                                      'error: unerwarteter fehler
            ErrorStr(0) = Strings.Left(ex.ToString, Strings.InStr(ex.ToString, ":") - 1)
            AppError()
            Exit Sub
        End Try

        'unvollständige BNetKey's rausfilltern
        Dim aNum, runs As Integer
        For Me.BnetNo = 2 To 9      'max 9 Bnet server
            BnetNoStr = BnetNo
            If Array.FindIndex(key, FindBnetKey) <> -1 Then
                If Array.FindIndex(key, Function(i) If(i = "bnet" & BnetNo & "_password", True, False)) = -1 Then
                    For Each Me.KeyStr In Array.FindAll(key, FindBnetKey)
                        aNum = Array.FindIndex(key, FindKey)
                        type(aNum) = 0
                        For be As Integer = line(aNum) To UBound(DefaultCfgFile) - 1
                            DefaultCfgFile(be) = DefaultCfgFile(be + 1)
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
        Dim tempIndex As Integer = Array.FindIndex(DefaultCfgFile, Function(i) If(i.StartsWith("map_loadingame"), True, False))

        If tempIndex <> -1 Then
            For be As Integer = tempIndex To UBound(DefaultCfgFile) - 1
                DefaultCfgFile(be) = DefaultCfgFile(be + 1)
            Next
            For da As Integer = 1 To UBound(line)
                If line(da) > tempIndex Then line(da) -= 1
            Next
            runs += 1
        End If
        If runs > 0 Then Array.Resize(DefaultCfgFile, DefaultCfgFile.Length - runs) 'wurden einträge geändert dann array.resize

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
                        Label(a).Text = Schluessel(a)
                        TextBox(a) = New TextBox
                        TextBox(a).Name = "TextBox" & a
                        TextBox(a).Size = New Size(212, 20)
                        TextBox(a).Text = If(Not myWert(a) Is Nothing, myWert(a), defWert(a))
                        'das "is" in der Bedingung ist hier ganz wichtig!!!

                        'toolTips erstellen
                        If My.Settings.DualCfgMode Then
                            If key(a).StartsWith("bnet") And Not key(a).StartsWith("bnet_") Then
                                DefaultTip.SetToolTip(TextBox(a), "Press ctrl + z to reset this value!" & vbCr & _
                                                      "Press ctrl + shift to get the value from server 1!")
                            Else
                                DefaultTip.SetToolTip(TextBox(a), "Press ctrl + z to reset this value!")
                            End If
                        End If

                        AddHandler TextBox(a).Click, AddressOf TextBox_click
                        AddHandler TextBox(a).KeyDown, AddressOf Any_Hotkey
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
                            If myWert(a) Is Nothing Then myWert(a) = defWert(a)

                            If myWert(a).StartsWith("|cFF") And IsHex(Mid(myWert(a), 5, 2)) And IsHex(Mid(myWert(a), 7, 2)) And _
                            IsHex(Mid(myWert(a), 9, 2)) Then
                                Button(a).BackColor = Color.FromArgb(CInt(CHex(False, Mid(myWert(a), 5, 2))), _
                                                            CInt(CHex(False, Mid(myWert(a), 7, 2))), _
                                                            CInt(CHex(False, Mid(myWert(a), 9, 2))))
                                TextBox(a).Text = Mid(myWert(a), 11)
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
                            .Maximum = 999999
                        End With
                        Label(a).Text = Schluessel(a)
                        Numeric(a).Text = If(Not myWert(a) Is Nothing, myWert(a), defWert(a))

                        'toolTips erstellen
                        If My.Settings.DualCfgMode Then
                            If key(a).StartsWith("bnet") And Not key(a).StartsWith("bnet_") Then
                                DefaultTip.SetToolTip(Numeric(a), "Press ctrl + z to reset this value!" & vbCr & _
                                                      "Press ctrl + shift to get the value from server 1!")
                            Else
                                DefaultTip.SetToolTip(Numeric(a), "Press ctrl + z to reset this value!")
                            End If
                        End If

                        AddHandler Numeric(a).Click, AddressOf Numeric_click
                        AddHandler Numeric(a).KeyDown, AddressOf Any_Hotkey
                        ElseIf type(a) = 3 Then
                            'nur checkbox erstellen
                            CheckBox(a) = New CheckBox
                            With CheckBox(a)
                                .Name = "CheckBox" & a
                                .AutoSize = True
                                .Text = Schluessel(a)

                                Dim tempValue As String = If(Not myWert(a) Is Nothing, myWert(a), defWert(a))
                                .Checked = CBool(If(tempValue = Nothing, 0, CInt(tempValue)))
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
                            tab(a) = 20 + CInt(Mid(key(a), 5, 1))     'legt die tabanzahl zwischen 22-30 fest!
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
        Dim SubToolNum(30) As Double            '30 w.g. anzahl tabs(untertabs von bnet) von 21-30
        Dim GroupToolNum(40) As Double          '40 w.g. anzahl tabs(gruppen auf admingame & more) von 31-35
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
                    If Len(Schluessel(a)) > 18 Then                                 'wenn schluessel(a) zu lang ist
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

        'systemkompatibilität
        If Environment.OSVersion.Version.Major >= 6 Then 'ist vista oder neuer?
            HeigthAdd = 0
        Else
            HeigthAdd = 5
        End If


    End Sub

    Private Sub Form1_FormClosing(ByVal sender As Object, ByVal e As System.Windows.Forms.FormClosingEventArgs) Handles Me.FormClosing
        'werte von tools auslesen
        For i As Integer = 1 To UBound(myWert)
            If tab(i) <> 0 Then
                If type(i) = 1 Or type(i) = 11 Then
                    myWert(i) = TextBox(i).Text
                ElseIf type(i) = 2 Then
                    myWert(i) = Numeric(i).Text
                ElseIf type(i) = 3 Then
                    myWert(i) = -CInt(CheckBox(i).Checked)
                ElseIf type(i) = 12 Then
                    If myWert(i).StartsWith("|cFF") And IsHex(Mid(myWert(i), 5, 2)) And IsHex(Mid(myWert(i), 7, 2)) And _
                    IsHex(Mid(myWert(i), 9, 2)) And ColorCBox(i).Checked Then
                        myWert(i) = Mid(myWert(i), 1, 10) & TextBox(i).Text
                    Else
                        myWert(i) = TextBox(i).Text
                    End If
                End If
            End If
        Next

        If My.Settings.DualCfgMode Then
            'werte vergleichen und änderungen zu myCfgFile hinzufügen
            ReDim myCfgFile(300)

            Dim lauf As Integer
            For i As Integer = 1 To UBound(defWert)
                If defWert(i) <> myWert(i) Then
                    myCfgFile(lauf) = key(i) & " = " & myWert(i)
                    lauf += 1
                End If
            Next

            ReDim Preserve myCfgFile(lauf - 1)
        Else
            'werte von variablen in "DefaultCfgFile" übernehmen
            For i As Integer = 1 To UBound(line)
                If line(i) <> 0 Then
                    DefaultCfgFile(line(i)) = key(i) & " = " & myWert(i)
                End If
            Next
        End If

        'in cfg schreiben & cfg pfad speichern
        If (ErrorN(0) > 3 Or ErrorN(0) = 0) And Not ErrorN(0) = 11 And Not Me.dontSaveCB.Checked Then
            Try
                'für dual_cfg nur änderungen in ghost.cfg speichern, ansonsten alles speichern
                If My.Settings.DualCfgMode Then
                    IO.File.WriteAllLines(MyCfgPath, myCfgFile)
                Else
                    IO.File.WriteAllLines(MyCfgPath, DefaultCfgFile)
                End If

                'ghost nach speichern starten
                If Me.runGhostCB.Checked Then
                    Dim tmpFile As String
                    If My.Computer.FileSystem.FileExists(My.Settings.GhostExePath) Then
                        tmpFile = My.Settings.GhostExePath
                    Else
                        tmpFile = cAppDirPath & cGhostExe
                    End If

                    Dim Ghost As New System.Diagnostics.Process()
                    Ghost.StartInfo.WorkingDirectory = My.Computer.FileSystem.GetParentPath(tmpFile)
                    Ghost.StartInfo.FileName = tmpFile
                    Ghost.Start()
                End If

                'my setting verwaltung
                If ApplySettings Then
                    Dim wurde_geändert As Boolean

                    With My.Settings
                        If .DefaultCfgPath <> "" Or .DefaultCfgPath <> SettingsDialog.TBDeafaultCfgPath.Text Then

                            .DefaultCfgPath = SettingsDialog.TBDeafaultCfgPath.Text
                            wurde_geändert = True
                            If .DefaultCfgPath = cAppDirPath + cDefaultCfg Then
                                .DefaultCfgPath = ""
                            End If
                        End If

                        If .MyCfgPath <> "" Or .MyCfgPath <> SettingsDialog.TBGhostCfgPath.Text Then

                            .MyCfgPath = SettingsDialog.TBGhostCfgPath.Text
                            wurde_geändert = True
                            If .MyCfgPath = cAppDirPath + cGhostCfg Then
                                .MyCfgPath = ""
                            End If
                        End If

                        If .GhostExePath <> "" Or .GhostExePath <> SettingsDialog.TBGhostExePath.Text Then

                            .GhostExePath = SettingsDialog.TBGhostExePath.Text
                            wurde_geändert = True
                            If .GhostExePath = cAppDirPath + cGhostExe Then
                                .GhostExePath = ""
                            End If
                        End If

                        If Not (.DualCfgMode = True And (SettingsDialog.rBtnUseDualCfg.Checked Or _
                                                         SettingsDialog.rBtnCreateCfg.Checked)) Then
                            If SettingsDialog.rBntUseMonoCfg.Checked Then
                                .DualCfgMode = False
                            Else
                                .DualCfgMode = True
                            End If
                            wurde_geändert = True
                        End If

                        If wurde_geändert Then
                            .Save()
                        End If
                    End With
                End If

            Catch ex As Exception
                Dim tmpPath As String = If(My.Settings.DualCfgMode, MyCfgPath, DefaultCfgPath)

                If Not My.Computer.FileSystem.FileExists(tmpPath & ".bak") Then
                    IO.File.WriteAllLines(tmpPath & ".bak", If(My.Settings.DualCfgMode, myCfgFile, DefaultCfgFile))
                    ErrorN(0) = 9
                Else
                    ErrorN(0) = 10
                End If
                AppError()
            End Try
        ElseIf ErrorN(0) = 1 Or ErrorN(0) = 3 Or ErrorN(0) = 11 Then
            'bei schwerem ausnahme fehler einstellungen zurücksetzen
            If My.Settings.DefaultCfgPath <> "" Then My.Settings.DefaultCfgPath = ""

            If My.Settings.MyCfgPath <> "" Then My.Settings.MyCfgPath = ""

            If My.Settings.DualCfgMode = False Then My.Settings.DualCfgMode = True
            My.Settings.Save()

        ElseIf Me.dontSaveCB.Checked And Me.runGhostCB.Checked Then
            'wenn run ghost + dont save aktive ist
            Dim tmpFile As String
            If My.Computer.FileSystem.FileExists(My.Settings.GhostExePath) Then
                tmpFile = My.Settings.GhostExePath
            Else
                tmpFile = cAppDirPath & cGhostExe
            End If

            Dim Ghost As New System.Diagnostics.Process()
            Ghost.StartInfo.WorkingDirectory = My.Computer.FileSystem.GetParentPath(tmpFile)
            Ghost.StartInfo.FileName = tmpFile
            Ghost.Start()
        End If
    End Sub
#End Region

#Region "Allemeine click events"

    Private Sub addServer_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles addServer.Click
        'error überprüfen
        If Not TabControl2.TabCount < 9 Then           'Falls Tab kleiner 9
            ErrorN(9) = 1
            AppError()
            Exit Sub
        End If
        'Bnet schlüssel listen
        Dim runden As Integer = 1
        Dim TextZeilen As Integer
        ReDim BNetIndex(50)

        BnetNoStr = "_"
        For Each Me.KeyStr In Array.FindAll(key, FindBnetKey)
            BNetIndex(runden) = Array.FindIndex(key, FindKey)
            runden += 1
        Next
        Array.Resize(BNetIndex, runden)
        'zeilen zählen
        For i As Integer = 1 To UBound(BNetIndex)
            For Each zeile In Info(BNetIndex(i)).Split(Chr(13))
                If zeile <> "NO description available!" Then
                    TextZeilen += 1
                Else
                    TextZeilen -= 1
                End If
            Next
        Next
        'DefaultCfgFile länge anpassen
        TextZeilen += runden * 2 - 3                    'array länge kompensieren + keine zeile am ende
        Dim AktZeile As Integer = DefaultCfgFile.Length + 1  'eine zeile freilassen
        Array.Resize(DefaultCfgFile, AktZeile + TextZeilen)

        'neue bnet werte in array schreiben
        Dim BnetNo As Integer = TabControl2.TabCount + 1
        For i As Integer = 1 To UBound(BNetIndex)
            Dim infoZeile As String() = Info(BNetIndex(i)).Split(Chr(13))
            For Each zeile In infoZeile
                If zeile <> "NO description available!" Then
                    DefaultCfgFile(AktZeile) = "### " & zeile
                    AktZeile += 1
                Else
                    AktZeile -= 1
                End If
            Next
            If defWert(BNetIndex(i)) = "1" Or defWert(BNetIndex(i)) = "0" Then          'e.g. checkbox(die benötigen einen wert)
                DefaultCfgFile(AktZeile) = "bnet" & BnetNo & "_" & Schluessel(BNetIndex(i)) & " = " & defWert(BNetIndex(i))
            Else
                DefaultCfgFile(AktZeile) = "bnet" & BnetNo & "_" & Schluessel(BNetIndex(i)) & " = "
            End If

            AktZeile += 2
        Next

        'für dual_cfg neue keys hinzufügen
        Dim tmpAktZeile As Integer = Array.IndexOf(key, Nothing, 1)
        key(tmpAktZeile) = "bnet" & BnetNo & "_server"
        defWert(tmpAktZeile) = "tmpString"

        'app neustart
        Application.Restart()
    End Sub

    Private Sub removeServer_click(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim temp As Button = CType(sender, Button)
        BnetNoStr = Mid(temp.Name, 10)
        Dim infoIndex(150), Runs, IRuns, zahler As Integer
        ReDim BNetIndex(50)

        'bnet key identifizieren
        For Each Me.KeyStr In Array.FindAll(key, FindBnetKey)
            Runs += 1
            BNetIndex(Runs) = Array.FindIndex(key, FindKey)
        Next
        ReDim Preserve BNetIndex(Runs)

        If DefaultCfgPath <> MyCfgPath Then Runs = 0

        'infos suchen
        For i As Integer = 1 To UBound(BNetIndex)
            zahler = (line(BNetIndex(i))) - 1

            While zahler > 0
                If (DefaultCfgFile(zahler).StartsWith("#") Or DefaultCfgFile(zahler) = "") And IRuns <= 150 Then
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
            If BNetIndex(i) > 0 AndAlso line(BNetIndex(i)) > 0 Then
                type(BNetIndex(i)) = 0
                For be As Integer = line(BNetIndex(i)) To UBound(DefaultCfgFile) - 1
                    DefaultCfgFile(be) = DefaultCfgFile(be + 1)
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
                For be As Integer = infoIndex(i) To UBound(DefaultCfgFile) - 1
                    DefaultCfgFile(be) = DefaultCfgFile(be + 1)
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
        Array.Resize(DefaultCfgFile, DefaultCfgFile.Length - (Runs + IRuns))

        'die werte gleichsetzen damit die entsprechenden bnet keys nichtmehr gespeichert werden
        For i As Integer = 0 To UBound(BNetIndex)
            myWert(BNetIndex(i)) = defWert(BNetIndex(i))
            tab(BNetIndex(i)) = 0
        Next

        Application.Restart()
    End Sub

    Private Sub TabControl1_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles TabControl1.Click
        'standardeigenschaften setzen
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
            Me.Text = "Configurator Help" & " - " & version
            Me.InfoLabel.Text = ""
            Me.InfoLabel.Top = TabControl1.Top + TabControl1.Height + 3
        End If

        If TabControl1.SelectedIndex < 4 Then
            Me.InfoLabel.Text = "Click at a Textbox or hover over a checkbox to get information about the setting!"
        End If
    End Sub

    Private Sub CBrunGhost_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles runGhostCB.Click
        If runGhostCB.Checked = True And _
        Not My.Computer.FileSystem.FileExists(cAppDirPath & cGhostExe) And _
        Not My.Computer.FileSystem.FileExists(My.Settings.GhostExePath) Then
            ErrorN(10) = 1
            AppError()
            Me.runGhostCB.Checked = False
        End If
    End Sub

    Private Sub Btn_openSettings_Click(ByVal sender As Object, ByVal e As System.EventArgs) Handles Btn_openSettings.Click

        SettingsDialog.rBtnCreateCfg.Enabled = False
        SettingsDialog.BtnResetAll.Enabled = True

        Select Case SettingsDialog.ShowDialog
            Case Windows.Forms.DialogResult.OK
                'neue einstellungen übernehmen
                ApplySettings = True
                Application.Restart()

            Case Windows.Forms.DialogResult.Ignore
                'einstellungsdatei löschen
                Dim SettingDir As String = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData) & _
                "\ghost_configurator"

                If My.Computer.FileSystem.DirectoryExists(SettingDir) Then
                    My.Computer.FileSystem.DeleteDirectory(SettingDir, FileIO.UIOption.OnlyErrorDialogs, _
                                                           FileIO.RecycleOption.SendToRecycleBin)
                    Application.Restart()
                Else
                    ErrorN(11) = 1
                    AppError()
                End If
        End Select
    End Sub
#End Region

#Region "Dynamische Prozeduren"

    Private Sub TextBox_click(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim TBox As TextBox = CType(sender, TextBox)
        InfoLabel.Text = Info(CInt(Mid(TBox.Name, 8)))                          'zu auslöser passende info setzen(für Textboxen)
        If InfoLabel.Height > 55 - HeigthAdd Then                               'wenn info länger ist
            Me.Height = TabControl1.Height + InfoLabel.Height + 55 + HeigthAdd
        Else
            Me.Height = TabControl1.Height + 107
        End If
    End Sub

    Private Sub Numeric_click(ByVal sender As Object, ByVal e As System.EventArgs)
        Dim Num As NumericUpDown = CType(sender, NumericUpDown)
        InfoLabel.Text = Info(CInt(Mid(Num.Name, 14)))                          'zu auslöser passende info setzen(für NumericUpDown)
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
        Dim tempWert As String = If(Not myWert(a) Is Nothing, myWert(a), defWert(a))

        If temp.Checked Then
            Button(a).Enabled = True
            If tempWert.StartsWith("|cFF") And IsHex(Mid(tempWert, 5, 2)) And IsHex(Mid(tempWert, 7, 2)) And _
                IsHex(Mid(tempWert, 9, 2)) Then
                Button(a).BackColor = Color.FromArgb(CInt(CHex(False, Mid(tempWert, 5, 2))), _
                                            CInt(CHex(False, Mid(tempWert, 7, 2))), _
                                            CInt(CHex(False, Mid(tempWert, 9, 2))))
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
            InfoLabel.Text = Info(InfoNum)                                      'zu auslöser passende info setzen(für CheckBoxen)
            If InfoLabel.Height > 55 - HeigthAdd Then                           'wenn info länger ist
                Me.Height = TabControl1.Height + InfoLabel.Height + 55 + HeigthAdd
            Else
                Me.Height = TabControl1.Height + 107
            End If
        End If
    End Sub

    Public Sub Any_MouseMove() Handles TabBot.MouseMove, TabBnet.MouseMove, TabAgame.MouseMove, Tabdb.MouseMove, GroupAGame.MouseMove, _
                                       GroupAHost.MouseMove, GroupLAN.MouseMove, GroupReplay.MouseMove, GroupUdpTcp.MouseMove
        CBoxInfoTimer.Enabled = False
    End Sub

    Private Sub Any_Hotkey(ByVal sender As Object, ByVal e As KeyEventArgs)
        Dim ToolNo As Integer
        Dim isTBox As Boolean

        If Len(sender.name) >= 14 Then
            ToolNo = CInt(Mid(sender.name, 14))
        Else
            ToolNo = CInt(Mid(sender.name, 8))
            isTBox = True
        End If

        If My.Settings.DualCfgMode Then
            'wert auf standart zurücksetzen

            If e.Control And Chr(e.KeyCode) = "Z" Then
                If Not type(ToolNo) = 12 Then
                    sender.text = defWert(ToolNo)

                ElseIf defWert(ToolNo).StartsWith("|cFF") Then      'ausnahmebehandlung für die color box =P
                    myWert(ToolNo) = defWert(ToolNo)
                    sender.text = Mid(defWert(ToolNo), 11)
                    ColorCBox(ToolNo).Checked = True
                    ColorCheckBox_Click(sender:=ColorCBox(ToolNo), e:=EventArgs.Empty)
                Else
                    myWert(ToolNo) = defWert(ToolNo)
                    sender.text = defWert(ToolNo)
                    ColorCBox(ToolNo).Checked = False
                    ColorCheckBox_Click(sender:=ColorCBox(ToolNo), e:=EventArgs.Empty)
                End If
            End If

            'wert von server 1 tab holen
            If tab(ToolNo) > 21 And tab(ToolNo) < 30 Then
                If e.Control And e.Shift Then
                    Dim BnetNo As Integer = Array.IndexOf(key, "bnet_" + Schluessel(ToolNo))

                    sender.text = If(isTBox, TextBox(BnetNo).Text, CStr(Numeric(BnetNo).Value))
                End If
            End If
        End If
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
        Dim tempWert As String = If(Not myWert(l) Is Nothing, myWert(l), defWert(l))

        If tempWert.StartsWith("|cFF") And IsHex(Mid(tempWert, 5, 2)) And IsHex(Mid(tempWert, 7, 2)) And _
        IsHex(Mid(tempWert, 9, 2)) Then
            ColorWin(l).Color = Color.FromArgb(CInt(CHex(False, Mid(tempWert, 5, 2))), CInt(CHex(False, Mid(tempWert, 7, 2))), _
                                                                       CInt(CHex(False, Mid(tempWert, 9, 2))))
        End If
        If ColorWin(l).ShowDialog = Windows.Forms.DialogResult.OK Then
            myWert(l) = "|cFF" & CHex(True, ColorWin(l).Color.R) & CHex(True, ColorWin(l).Color.G) & CHex(True, ColorWin(l).Color.B) & _
                                    Mid(myWert(l), 11)
            Button(l).BackColor = Color.FromArgb(CInt(CHex(False, Mid(myWert(l), 5, 2))), _
                                                    CInt(CHex(False, Mid(myWert(l), 7, 2))), _
                                                    CInt(CHex(False, Mid(myWert(l), 9, 2))))
        End If
    End Sub
#End Region

    Private Function IsSubfix(ByVal text As String) As Boolean
        If text.StartsWith("bot_") Or text.StartsWith("admingame_") Or text.StartsWith("db_") Or text.StartsWith("bnet") Or _
           text.StartsWith("udp_") Or text.StartsWith("tcp_") Or text.StartsWith("lan_") Or text.StartsWith("autohost_") Or _
           text.StartsWith("replay_") Then
            Return True
        Else
            Return False
        End If
    End Function

    Private Function StartUpSettingsDialog() As Boolean

        Select Case SettingsDialog.ShowDialog
            Case Windows.Forms.DialogResult.OK

                'falls creat new nicht ausgefählt ist
                If Not SettingsDialog.rBtnCreateCfg.Checked Then
                    'GhostCfgPath übernehmen
                    If My.Computer.FileSystem.FileExists(SettingsDialog.TBGhostCfgPath.Text) Then
                        MyCfgPath = SettingsDialog.TBGhostCfgPath.Text
                        My.Settings.MyCfgPath = MyCfgPath
                    ElseIf SettingsDialog.TBGhostCfgPath.Text = "" Then
                        My.Settings.MyCfgPath = Nothing
                    End If
                End If

                'falls nicht mono cfg aktive
                If Not SettingsDialog.rBntUseMonoCfg.Checked Then
                    'defaultCfgPath übernehmen
                    If My.Computer.FileSystem.FileExists(SettingsDialog.TBDeafaultCfgPath.Text) Then
                        DefaultCfgPath = SettingsDialog.TBDeafaultCfgPath.Text
                        My.Settings.DefaultCfgPath = DefaultCfgPath
                    ElseIf SettingsDialog.TBDeafaultCfgPath.Text = "" Then
                        My.Settings.DefaultCfgPath = Nothing
                    End If
                    My.Settings.DualCfgMode = True

                Else
                    DefaultCfgPath = MyCfgPath
                    My.Settings.DualCfgMode = False
                    My.Settings.DefaultCfgPath = Nothing
                End If

                'wenn create new ausgewählt ist
                If SettingsDialog.rBtnCreateCfg.Checked Then
                    'wert aus TB übernehmen
                    If SettingsDialog.TBGhostCfgPath.Text <> "" Then
                        MyCfgPath = SettingsDialog.TBGhostCfgPath.Text
                    End If

                    Dim tmpString As String = My.Computer.FileSystem.GetParentPath(MyCfgPath)
                    Dim tmpArray(0) As String

                    'neue datei erstellen
                    If My.Computer.FileSystem.DirectoryExists(tmpString) Then
                        IO.File.WriteAllLines(MyCfgPath, tmpArray)
                        My.Settings.MyCfgPath = MyCfgPath
                    End If
                End If

                If SettingsDialog.TBGhostExePath.Text <> "" AndAlso _
                My.Computer.FileSystem.FileExists(SettingsDialog.TBGhostExePath.Text) Then
                    My.Settings.GhostExePath = SettingsDialog.TBGhostExePath.Text
                End If

                With My.Settings
                    'Standart pfade nicht speichern
                    If .MyCfgPath = cAppDirPath + cGhostCfg Then
                        .MyCfgPath = ""
                    End If
                    If .DefaultCfgPath = cAppDirPath + cDefaultCfg Then
                        .DefaultCfgPath = ""
                    End If
                    If .GhostExePath = cAppDirPath + cGhostExe Then
                        .GhostExePath = ""
                    End If

                    'in jedem fall einstellungen speichern
                    .Save()

                    'If .DualCfgMode <> True Or .MyCfgPath <> "" Or .DefaultCfgPath <> "" Or .GhostExePath <> "" Then
                    '    .Save()
                    'Else
                    '    'einstellungsdatei löschen
                    '    Dim SettingDir As String = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData) & _
                    '                               "\ghost_configurator"

                    '    If My.Computer.FileSystem.DirectoryExists(SettingDir) Then
                    '        My.Computer.FileSystem.DeleteDirectory(SettingDir, FileIO.UIOption.OnlyErrorDialogs, _
                    '                                               FileIO.RecycleOption.DeletePermanently)
                    '    End If
                    'End If
                End With

                Return True
            Case Windows.Forms.DialogResult.Cancel
                Return False
        End Select
    End Function

#Region "Hex converter"
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
#End Region

#Region "Such Funktionen"
    Private FindBnetKey As Predicate(Of String) = Function(i) If(i <> Nothing AndAlso _
                                                                 i.StartsWith(LTrim("bnet" & BnetNoStr)), True, False)

    Private FindKey As Predicate(Of String) = Function(i) If(i = KeyStr, True, False) 'predicate: spetzieller type für suchanfragen
#End Region

    Public Sub AppError()
        Dim CfgName As String = If(My.Settings.DualCfgMode, cDefaultCfg, cGhostCfg)
        Dim saveFileName As String = My.Computer.FileSystem.GetName(If(My.Settings.DualCfgMode, MyCfgPath, DefaultCfgPath))

        'kritische fehler (beim start)
        If ErrorN(0) = 1 Then
            MsgBox("Error during the data processing, try to change the " & CfgName & " file path at restart " & _
                   "and respectively replace the this cfg file with another one!" _
                   & vbCr & vbCr & "Error code: " & ErrorStr(0), MsgBoxStyle.Critical, "Error #001")
            Me.Close()
        ElseIf ErrorN(0) = 2 Then
            'wenn keine cfg gefunden wurde & abbrechen
            Me.Close()
        ElseIf ErrorN(0) = 3 Then
            MsgBox("There are no setting in the " & CfgName & "," & Chr(13) & "make sure that the used cfg file is a ghost++ cfg!" & _
                   Chr(13) & Chr(13) & "The cfg file path have been reseted (to prevent bugs).", _
                   MsgBoxStyle.Critical, "Error #011")
            Me.Close()
        ElseIf ErrorN(0) = 9 Then
            MsgBox("Unable to write settings to the cfg file, make sure that the " & saveFileName & " file isn't write protected or " & _
                   "in use by another application!" & Chr(13) & Chr(13) & "The settings has been saved to " & Chr(34) & _
                   saveFileName & ".bak" & Chr(34) & " (you have to rename this file to " & Chr(34) & saveFileName & Chr(34) & _
                   ", to use this settings).", MsgBoxStyle.Critical, "Error #010")
        ElseIf ErrorN(0) = 10 Then
            MsgBox("Unable to write settings to the cfg file, make sure that the " & saveFileName & " file isn't write protected or " & _
                   "in use by another application!", MsgBoxStyle.Critical, "Error #010")
        ElseIf ErrorN(0) = 11 Then
            MsgBox("Error during the data processing, try to change the ghost.cfg file path at restart " & _
                  "and respectively replace the this cfg file with another one!" _
                  & vbCr & vbCr & "Error code: " & ErrorStr(0), MsgBoxStyle.Critical, "Error #002")
            Me.Close()
        End If
        'bei error(0) restliche meldungen ignorieren
        If ErrorN(0) <> 0 Then
            Exit Sub
        End If

        'configurator meldungen
        If ErrorN(1) = 1 Then
            MsgBox("To many settings in the cfg file(s), maximum is " & UBound(type) & "!", MsgBoxStyle.Exclamation, "Error #002")
        End If
        If ErrorN(2) = 1 Then
            MsgBox("Syntax error in (the " & CfgName & " in) following lines:" & Chr(13) & ErrorStr(2) & " a " & Chr(34) & "=" & Chr(34) & _
                   " is missing!", MsgBoxStyle.Exclamation, "Error #003")
        End If
        If ErrorN(3) = 1 Then
            MsgBox("Syntax error in (the " & CfgName & " in) following lines:" & Chr(13) & ErrorStr(3) & "only 9 bnet servers are " & _
                   "supported!", MsgBoxStyle.Exclamation, "Error #004")
        End If
        If ErrorN(4) = 1 Then
            MsgBox("Syntax error in (the " & CfgName & " in) following lines:" & Chr(13) & ErrorStr(4) & "only numbers after " & Chr(34) _
                   & "bnet" & Chr(34) & " are supported!", MsgBoxStyle.Exclamation, "Error #005")
        End If
        If ErrorN(6) = 1 Then
            MsgBox("Syntax error in (the " & CfgName & " in) following lines:" & Chr(13) & ErrorStr(6) & " a " & Chr(34) & "#" & Chr(34) & _
                   " is missing!", MsgBoxStyle.Exclamation, "Error #006")
        End If
        'Updater fehler
        'If ErrorN(8) = 1 Then
        '    MsgBox("Could not find any setting in the selected cfg file!", MsgBoxStyle.Exclamation, "Error #008")
        '    Me.InfoLabel.Text = "Error, no settings found, only files named " & Chr(34) & "ghost.cfg" & Chr(34) & " are supported!"
        'ElseIf ErrorN(8) = 2 Then
        '    MsgBox("No differences found between the old cfg and the new one!", MsgBoxStyle.Exclamation, "Error #009")
        '    Me.InfoLabel.Text = "Error, no differences found between the two files!"
        'End If

        'add Server fehler
        If ErrorN(9) = 1 Then
            MsgBox("Could not create more server settings, only 10 servers are supported!", MsgBoxStyle.Exclamation, "Error #015")
        End If
        'ghosts.exe nicht gefunden
        If ErrorN(10) = 1 Then
            MsgBox("Ghost.exe can't be found, it won't be launched after closing!" & Chr(13) & _
                  "(You may specify the ghost.exe path at the settings.)", MsgBoxStyle.Information, "Error #017")
        End If
        'settings fehler
        If ErrorN(11) = 1 Then
            MsgBox("Could not find the configurator setting file," & vbCr & _
                   "it seems that no settings were saved until yet!", MsgBoxStyle.Information, "Error #018")
        End If
        'dual cfg mode fehler (ghsot.cfg)
        If ErrorN(15) = 1 Then
            'ToDo: neue error nummer geben
            MsgBox("Syntax error in (the ghost.cfg in) following lines:" & Chr(13) & ErrorStr(15) & " a " & Chr(34) & "=" & Chr(34) & _
                   " is missing!", MsgBoxStyle.Exclamation, "Error #020")
        End If
        If ErrorN(16) = 1 Then
            'ToDo: neue error nummer geben
            MsgBox("To many battle.net settings (" & Chr(34) & "bnet_" & Chr(34) & ") in the cfg file(s), maximum is 50!", _
                   MsgBoxStyle.Exclamation, "Error #021")
        End If
        Array.Clear(ErrorN, 0, UBound(ErrorN))
    End Sub
End Class

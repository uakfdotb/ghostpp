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

Public Class myTextBox
    'in MeinButton.Desinger.vb muss "Inherits" auf "System.Windows.Forms.TextBox" gestellt sein
    Private _TextInfo As String
    Private _aText As String
    Private FirstRun As Boolean = True
    Private MeIsFocused As Boolean


    Public Property TextInfo() As String
        Get
            Return _TextInfo
        End Get
        Set(ByVal value As String)
            _TextInfo = value
            RaiseEvent newTextInfo()
        End Set
    End Property

    Public Overrides Property Text() As String
        Get
            If Me.Focused Then _aText = MyBase.Text

            Return _aText
        End Get
        Set(ByVal value As String)
            MyBase.Text = value
            If Not Me.Focused Then
                Me.ForeColor = Drawing.SystemColors.WindowText
                _aText = value
            End If
        End Set
    End Property

    Private Event newTextInfo()
    Public Event UserChangedText(ByVal sender, ByVal e)

    Private Sub MeinButton_GotFocus(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.GotFocus

        Me.ForeColor = Drawing.SystemColors.WindowText
        MyBase.Text = _aText

        MeIsFocused = True
    End Sub

    Private Sub MeinButton_LostFocus(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.LostFocus
        MeIsFocused = False

        _aText = MyBase.Text
        If _aText = "" Then
            Me.ForeColor = Drawing.Color.SlateGray

            MyBase.Text = _TextInfo
        End If
    End Sub

    Private Sub TextInfoChanged() Handles Me.newTextInfo
        If FirstRun Then

            FirstRun = False
            _aText = MyBase.Text
        End If
        If _aText = "" Then

            Me.ForeColor = Drawing.Color.SlateGray
            MyBase.Text = _TextInfo
        End If
    End Sub

    'Soll das noramle event text_changed ersetzen( wird aber nicht von den eingenen triggers augeslöst
    Private Sub MeineTextBox_TextChanged(ByVal sender As Object, ByVal e As System.EventArgs) Handles Me.TextChanged
        If MeIsFocused Then
            RaiseEvent UserChangedText(sender, e)
        End If
    End Sub
End Class

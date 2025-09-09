import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    implicitWidth: 300
    property int mode: 0 // 0: Solid, 1: Sketch
    readonly property Theme theme: Theme {}
    background: Rectangle { color: theme.panelBg; border.color: theme.border; radius: 0 }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Label {
            text: root.mode === 0 ? qsTr("Solid Parameters") : qsTr("Sketch Parameters")
            font.bold: true
            color: theme.text
        }

        StackLayout {
            id: paramsStack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.mode

            // Solid parameters
            ColumnLayout {
                spacing: 6
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    background: Rectangle { color: theme.panelMuted; border.color: theme.border; radius: 0 }
                    Label { anchors.centerIn: parent; text: qsTr("Select object(s) and set Extrude params"); color: theme.textMuted }
                }
                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: 6
                    Button {
                        text: qsTr("Cancel")
                        background: Rectangle { color: theme.panelMuted; radius: 0 }
                        contentItem: Label { text: parent.text; color: theme.text }
                    }
                    Button {
                        text: qsTr("OK")
                        background: Rectangle { color: theme.accent; radius: 0 }
                        contentItem: Label { text: parent.text; color: "white" }
                    }
                }
            }

            // Sketch parameters
            ColumnLayout {
                spacing: 6
                CheckBox {
                    text: qsTr("Show sketch grid")
                    contentItem: Label { text: parent.text; color: theme.text }
                    indicator: Rectangle { implicitWidth: 16; implicitHeight: 16; color: theme.panelMuted; border.color: theme.border }
                }
                CheckBox {
                    text: qsTr("Snap to grid")
                    contentItem: Label { text: parent.text; color: theme.text }
                    indicator: Rectangle { implicitWidth: 16; implicitHeight: 16; color: theme.panelMuted; border.color: theme.border }
                }
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    background: Rectangle { color: theme.panelMuted; border.color: theme.border; radius: 0 }
                    Label { anchors.centerIn: parent; text: qsTr("Select tool: point/line, edit constraints"); color: theme.textMuted }
                }
                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: 6
                    Button {
                        text: qsTr("Cancel")
                        background: Rectangle { color: theme.panelMuted; radius: 0 }
                        contentItem: Label { text: parent.text; color: theme.text }
                    }
                    Button {
                        text: qsTr("OK")
                        background: Rectangle { color: theme.accent; radius: 0 }
                        contentItem: Label { text: parent.text; color: "white" }
                    }
                }
            }
        }
    }
}

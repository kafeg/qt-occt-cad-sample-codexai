import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    implicitWidth: 300
    property int mode: 0 // 0: Solid, 1: Sketch

    background: Rectangle { color: "#25272a"; border.color: "#2f3236" }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: root.mode === 0 ? qsTr("Solid Parameters") : qsTr("Sketch Parameters")
            font.bold: true
            color: "#e6e6e6"
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
                    background: Rectangle { color: "#2a2c2f"; border.color: "#3a3d42"; radius: 6 }
                    Label { anchors.centerIn: parent; text: qsTr("Select object(s) and set Extrude params"); color: "#bbb" }
                }
                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: 8
                    Button { text: qsTr("Cancel"); background: Rectangle { color: "#3a3d42"; radius: 6 } }
                    Button { text: qsTr("OK"); background: Rectangle { color: "#4c8bf5"; radius: 6 } }
                }
            }

            // Sketch parameters
            ColumnLayout {
                spacing: 6
                CheckBox { text: qsTr("Show sketch grid") }
                CheckBox { text: qsTr("Snap to grid") }
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    background: Rectangle { color: "#2a2c2f"; border.color: "#3a3d42"; radius: 6 }
                    Label { anchors.centerIn: parent; text: qsTr("Select tool: point/line, edit constraints"); color: "#bbb" }
                }
                RowLayout {
                    Layout.alignment: Qt.AlignRight
                    spacing: 8
                    Button { text: qsTr("Cancel"); background: Rectangle { color: "#3a3d42"; radius: 6 } }
                    Button { text: qsTr("OK"); background: Rectangle { color: "#4c8bf5"; radius: 6 } }
                }
            }
        }
    }
}

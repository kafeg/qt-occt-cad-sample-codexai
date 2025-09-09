import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    implicitHeight: 80
    background: Rectangle { color: "#202225"; border.color: "#2f3236" }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Label { text: qsTr("Timeline"); font.bold: true; color: "#e6e6e6" }

        ListView {
            id: timelineView
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: ListView.Horizontal
            spacing: 8
            model: 8
            delegate: Rectangle {
                width: 140
                height: 36
                radius: 6
                color: "#2a2c2f"
                border.color: "#3a3d42"
                Label {
                    anchors.centerIn: parent
                    text: "Feature " + (index+1)
                    color: "#cfd3d6"
                }
            }
            clip: true
        }
    }
}


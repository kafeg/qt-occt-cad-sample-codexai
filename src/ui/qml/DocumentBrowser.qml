import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    implicitWidth: 240
    implicitHeight: 280

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Label {
            text: qsTr("Document Browser")
            font.bold: true
        }

        // Placeholder content
        Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Label {
                anchors.centerIn: parent
                text: qsTr("No items")
                color: "#666"
            }
        }
    }
}


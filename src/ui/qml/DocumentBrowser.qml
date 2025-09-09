import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    implicitWidth: 260
    implicitHeight: 320
    background: Rectangle { color: "#25272a"; border.color: "#2f3236" }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label { text: qsTr("Browser"); font.bold: true; color: "#e6e6e6" }

        // Sections: Origin, Bodies, Sketches
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Label { text: qsTr("Origin"); color: "#9aa0a6" }
            Repeater {
                model: 1
                Label { text: "• XY, XZ, YZ planes"; color: "#cfd3d6" }
            }
        }

        Rectangle { height: 1; color: "#2f3236"; Layout.fillWidth: true }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Label { text: qsTr("Bodies"); color: "#9aa0a6" }
            Repeater {
                model: 3
                Label { text: "Body " + (index+1); color: "#cfd3d6" }
            }
        }

        Rectangle { height: 1; color: "#2f3236"; Layout.fillWidth: true }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Label { text: qsTr("Sketches"); color: "#9aa0a6" }
            Repeater {
                model: 2
                Label { text: "Sketch " + (index+1); color: "#cfd3d6" }
            }
        }

        Item { Layout.fillHeight: true }
    }
}

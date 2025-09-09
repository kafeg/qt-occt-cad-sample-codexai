import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    implicitWidth: 260
    implicitHeight: 320
    readonly property Theme theme: Theme {}
    background: Rectangle { color: theme.panelBg; border.color: "transparent"; radius: 0 }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        // Header with separator
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            
            Label { 
                text: qsTr("Browser"); 
                font.bold: true; 
                color: theme.textMuted;
                Layout.fillWidth: true
            }
            
            Rectangle { 
                height: 1; 
                color: theme.divider; 
                Layout.fillWidth: true 
            }
        }

        // Sections: Origin, Bodies, Sketches
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Label { text: qsTr("Origin"); color: theme.textMuted }
            Repeater {
                model: 1
                Label { text: "• XY, XZ, YZ planes"; color: theme.text }
            }
        }

        Rectangle { height: 1; color: theme.divider; Layout.fillWidth: true }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Label { text: qsTr("Bodies"); color: theme.textMuted }
            Repeater {
                model: 3
                Label { text: "Body " + (index+1); color: theme.text }
            }
        }

        Rectangle { height: 1; color: theme.divider; Layout.fillWidth: true }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            Label { text: qsTr("Sketches"); color: theme.textMuted }
            Repeater {
                model: 2
                Label { text: "Sketch " + (index+1); color: theme.text }
            }
        }

        Item { Layout.fillHeight: true }
    }
}

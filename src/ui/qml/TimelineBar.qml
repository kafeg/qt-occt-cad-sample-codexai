import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    implicitHeight: 80
    readonly property Theme theme: Theme {}
    background: Rectangle { color: theme.panelBg; border.color: theme.border }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Label { text: qsTr("Timeline"); font.bold: true; color: theme.text }

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
                radius: 0
                color: theme.chipBg
                border.color: theme.border
                Label {
                    anchors.centerIn: parent
                    text: "Feature " + (index+1)
                    color: theme.text
                }
            }
            clip: true
        }
    }
}

import QtQuick
import QtQuick.Controls

Item {
    id: root
    readonly property Theme theme: Theme {}
    Rectangle {
        anchors.fill: parent
        color: theme.viewBg
        border.color: theme.divider
        border.width: 1

        Label {
            anchors.centerIn: parent
            text: qsTr("OCCT View")
            color: theme.textMuted
        }
    }
}

import QtQuick
import QtQuick.Controls

Item {
    id: root
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.06, 0.07, 0.09, 1)
        border.color: "#333"
        border.width: 1

        Label {
            anchors.centerIn: parent
            text: qsTr("OCCT View")
            color: "#bbb"
        }
    }
}


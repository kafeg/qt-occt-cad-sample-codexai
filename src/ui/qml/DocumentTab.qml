import QtQuick
import QtQuick.Controls

Item {
    id: root
    anchors.fill: parent

    // Placeholder content for a document tab.
    // Hook up actual viewer/document UI here later.
    Label {
        anchors.centerIn: parent
        text: qsTr("Document View")
        opacity: 0.7
    }
}

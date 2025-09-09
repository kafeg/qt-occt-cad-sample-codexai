import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ToolBar {
    id: root

    RowLayout {
        anchors.fill: parent
        spacing: 6

        // Minimal placeholder actions; actual wiring happens in C++/QML later
        ToolButton { text: qsTr("Add Box") }
        ToolButton { text: qsTr("Add Cylinder") }
        ToolButton { text: qsTr("Add Extrude") }
        ToolSeparator {}
        ToolButton { text: qsTr("Move") }
        Item { Layout.fillWidth: true }
    }
}


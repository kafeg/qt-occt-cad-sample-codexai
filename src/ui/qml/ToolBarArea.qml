import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ToolBar {
    id: root
    property int mode: 0 // 0: Solid, 1: Sketch

    background: Rectangle { color: "#2a2c2f"; border.color: "#2f3236"; height: 56 }

    RowLayout {
        anchors.fill: parent
        spacing: 8
        // Mode tabs
        TabBar {
            id: modeTabs
            implicitHeight: 40
            currentIndex: root.mode
            onCurrentIndexChanged: root.mode = currentIndex
            TabButton {
                text: qsTr("Solid")
                implicitHeight: 40
                background: Rectangle { color: modeTabs.currentIndex === 0 ? "#1e1f22" : "#2a2c2f"; radius: 6; border.color: "#3a3d42" }
                contentItem: Label { text: parent.text; color: "#e6e6e6"; verticalAlignment: Text.AlignVCenter }
            }
            TabButton {
                text: qsTr("Sketch")
                implicitHeight: 40
                background: Rectangle { color: modeTabs.currentIndex === 1 ? "#1e1f22" : "#2a2c2f"; radius: 6; border.color: "#3a3d42" }
                contentItem: Label { text: parent.text; color: "#e6e6e6"; verticalAlignment: Text.AlignVCenter }
            }
        }

        // Tool groups per mode
        StackLayout {
            id: toolGroups
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            currentIndex: modeTabs.currentIndex

            // Solid tools
            RowLayout {
                spacing: 6
                ToolButton {
                    text: qsTr("New Sketch")
                    icon.source: "qrc:/ui/icons/box.svg" // placeholder icon
                    background: Rectangle { color: "#3a3d42"; radius: 6 }
                }
                ToolButton {
                    text: qsTr("Extrude")
                    icon.source: "qrc:/ui/icons/cylinder.svg" // placeholder icon
                    background: Rectangle { color: "#3a3d42"; radius: 6 }
                }
            }

            // Sketch tools
            RowLayout {
                spacing: 6
                ToolButton {
                    text: qsTr("Point")
                    background: Rectangle { color: "#3a3d42"; radius: 6 }
                }
                ToolButton {
                    text: qsTr("Line")
                    background: Rectangle { color: "#3a3d42"; radius: 6 }
                }
            }
        }

        // Spacer and future right-aligned quick actions
        Item { Layout.fillWidth: true }
    }
}

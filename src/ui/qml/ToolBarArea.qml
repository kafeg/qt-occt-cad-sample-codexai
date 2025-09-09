import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ToolBar {
    id: root
    property int mode: 0 // 0: Solid, 1: Sketch

    // Local theme instance (flat palette)
    readonly property Theme theme: Theme {}

    background: Rectangle { color: theme.toolbarBg; border.color: theme.border; height: 56; radius: 0 }

    RowLayout {
        anchors.fill: parent
        spacing: 4
        // Mode tabs
        TabBar {
            id: modeTabs
            implicitHeight: 40
            currentIndex: root.mode
            onCurrentIndexChanged: root.mode = currentIndex
            background: Rectangle { color: "transparent" }
            TabButton {
                text: qsTr("Solid")
                implicitHeight: 40
                background: Rectangle { color: modeTabs.currentIndex === 0 ? theme.toolbarTabOn : theme.toolbarTabBg; radius: 0; border.color: theme.border }
                contentItem: Label { text: parent.text; color: theme.text; verticalAlignment: Text.AlignVCenter; padding: 12 }
            }
            TabButton {
                text: qsTr("Sketch")
                implicitHeight: 40
                background: Rectangle { color: modeTabs.currentIndex === 1 ? theme.toolbarTabOn : theme.toolbarTabBg; radius: 0; border.color: theme.border }
                contentItem: Label { text: parent.text; color: theme.text; verticalAlignment: Text.AlignVCenter; padding: 12 }
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
                spacing: 4
                ToolButton {
                    text: qsTr("New Sketch")
                    icon.source: "qrc:/ui/icons/box.svg" // placeholder icon
                    background: Rectangle { color: "transparent"; radius: 0 }
                    contentItem: Label { text: parent.text; color: theme.text; padding: 8 }
                }
                ToolButton {
                    text: qsTr("Extrude")
                    icon.source: "qrc:/ui/icons/cylinder.svg" // placeholder icon
                    background: Rectangle { color: "transparent"; radius: 0 }
                    contentItem: Label { text: parent.text; color: theme.text; padding: 8 }
                }
            }

            // Sketch tools
            RowLayout {
                spacing: 4
                ToolButton {
                    text: qsTr("Point")
                    background: Rectangle { color: "transparent"; radius: 0 }
                    contentItem: Label { text: parent.text; color: theme.text; padding: 8 }
                }
                ToolButton {
                    text: qsTr("Line")
                    background: Rectangle { color: "transparent"; radius: 0 }
                    contentItem: Label { text: parent.text; color: theme.text; padding: 8 }
                }
            }
        }

        // Spacer and future right-aligned quick actions
        Item { Layout.fillWidth: true }
    }
}

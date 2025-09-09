import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts
import QtQuick.Shapes

// ApplicationWindow {
//     width: 800
//     height: 120
//     visible: true
//     color: "#1a1d22"

//     ToolBar {
//         id: toolbar
//         anchors.fill: parent
//         background: Rectangle { color: "#121418" }

//         RowLayout {
//             anchors.fill: parent
//             spacing: 0

//             // --- Left block with arrow ---
//             Item {
//                 Layout.preferredWidth: 300
//                 Layout.fillHeight: true

//                 Rectangle {
//                     id: leftBlock
//                     anchors.fill: parent
//                     anchors.rightMargin: 20
//                     color: "#232830"
//                 }

//                 // Arrow
//             Canvas {
//                 anchors.fill: parent
//                 onPaint: {
//                     var ctx = getContext("2d");
//                     ctx.reset();
//                     ctx.fillStyle = "#232830";
//                     ctx.beginPath();
//                     ctx.moveTo(0, 0);
//                     ctx.lineTo(width - 20, 0);
//                     ctx.lineTo(width, height/2);
//                     ctx.lineTo(width - 20, height);
//                     ctx.lineTo(0, height);
//                     ctx.closePath();
//                     ctx.fill();
//                 }
//             }

//                 Row {
//                     anchors.centerIn: parent
//                     spacing: 8

//                     Button { text: "Solid" }
//                     Button { text: "Sketch" }
//                 }
//             }

//             // --- Right block ---
//             Item {
//                 Layout.fillWidth: true
//                 Layout.fillHeight: true
//                 Rectangle {
//                     anchors.fill: parent
//                     color: "#121418"
//                 }

//                 Row {
//                     anchors.centerIn: parent
//                     spacing: 8

//                     Button { text: "Tool A" }
//                     Button { text: "Tool B" }
//                 }
//             }
//         }
//     }
// }

ApplicationWindow {
    id: root
    visible: true
    width: 1200
    height: 760
    title: qsTr("VibeCAD")
    // Local theme instance and window base color
    readonly property Theme theme: Theme {}
    color: theme.windowBg

    property int tabCounter: 0

    // No header; tabs start immediately at the top

    function addDocumentTab() {
        tabCounter += 1
        tabsModel.append({ title: qsTr("Document ") + tabCounter })
        tabBar.currentIndex = tabsModel.count - 1
    }

    function closeDocumentTab(idx) {
        if (tabsModel.count <= 1)
            return // keep at least one tab open
        // Adjust current index before removal
        var wasCurrent = (tabBar.currentIndex === idx)
        tabsModel.remove(idx)
        if (wasCurrent) {
            tabBar.currentIndex = Math.max(0, Math.min(idx, tabsModel.count - 1))
        } else if (idx < tabBar.currentIndex) {
            tabBar.currentIndex = tabBar.currentIndex - 1
        }
    }

    Component.onCompleted: {
        // Open one default tab at startup
        addDocumentTab()
    }

    ListModel { id: tabsModel }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Tabs row with flat background
        Rectangle {
            id: tabsRow
            Layout.fillWidth: true
            height: 44
            color: theme.tabBg
            border.color: theme.border

            RowLayout {
                anchors.fill: parent
                spacing: 2
                TabBar {
                    id: tabBar
                    implicitHeight: 44
                    Layout.fillWidth: true
                    background: Rectangle { color: "transparent" }
                    Repeater {
                        model: tabsModel
                        TabButton {
                            id: tab
                            text: model.title
                            implicitHeight: 44
                            padding: 0
                            background: Rectangle {
                                color: tabBar.currentIndex === index ? theme.tabActiveBg : theme.tabBg
                                border.color: theme.border
                                radius: 0
                            }
                            contentItem: Item {
                                anchors.fill: parent
                                readonly property int sideMargin: 10

                                // Close button pinned to the right, centered vertically
                                ToolButton {
                                    id: closeBtn
                                    Accessible.name: qsTr("Close Tab")
                                    text: "✕"
                                    implicitWidth: 24
                                    implicitHeight: 24
                                    padding: 0
                                    anchors.right: parent.right
                                    anchors.rightMargin: parent.sideMargin
                                    anchors.verticalCenter: parent.verticalCenter
                                    onClicked: closeDocumentTab(index)
                                    enabled: tabsModel.count > 1
                                    background: Rectangle {
                                        radius: 0
                                        color: closeBtn.down ? theme.panelMuted : "transparent"
                                        border.color: enabled ? theme.border : theme.divider
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: enabled ? theme.textMuted : theme.textWeak
                                        font.pixelSize: theme.tabIconSize
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                // Centered tab title within remaining space (excludes close button)
                                Text {
                                    text: tab.text
                                    color: theme.text
                                    font.pixelSize: theme.tabFontSize
                                    elide: Text.ElideRight
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left
                                    anchors.right: closeBtn.left
                                    anchors.leftMargin: parent.sideMargin
                                    anchors.rightMargin: parent.sideMargin
                                }
                            }
                        }
                    }
                }
                ToolButton {
                    id: addTabButton
                    text: "+"
                    Accessible.name: qsTr("Add Tab")
                    onClicked: addDocumentTab()
                    implicitWidth: 36
                    background: Rectangle { color: theme.tabBg; border.color: theme.border; radius: 0 }
                    contentItem: Label { text: parent.text; color: theme.text; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
            }
        }

        StackLayout {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            Repeater {
                model: tabsModel
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    DocumentTab { anchors.fill: parent }
                }
            }
        }
    }
}

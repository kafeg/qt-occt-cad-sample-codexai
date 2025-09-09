import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 1200
    height: 760
    title: qsTr("VibeCAD")

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

        RowLayout {
            id: tabsRow
            Layout.fillWidth: true
            spacing: 6
            TabBar {
                id: tabBar
                implicitHeight: 44 // taller tabs
                Layout.fillWidth: true
                Repeater {
                    model: tabsModel
                    TabButton {
                        text: model.title
                        implicitHeight: 44
                        padding: 12
                        font.bold: tabBar.currentIndex === index
                        background: Rectangle {
                            color: tabBar.currentIndex === index ? "#1e1f22" : "#2a2c2f"
                            radius: 6
                            border.color: tabBar.currentIndex === index ? "#3a3d42" : "#2f3236"
                        }
                        contentItem: RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 8
                            Label {
                                Layout.fillWidth: true
                                text: parent.parent.text
                                color: "#e6e6e6"
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }
                            ToolButton {
                                id: closeBtn
                                Accessible.name: qsTr("Close Tab")
                                text: "✕"
                                implicitWidth: 22
                                implicitHeight: 22
                                padding: 0
                                onClicked: closeDocumentTab(index)
                                enabled: tabsModel.count > 1
                                background: Rectangle {
                                    radius: 4
                                    color: closeBtn.down ? "#3a3d42" : "transparent"
                                    border.color: enabled ? "#3a3d42" : "#2f3236"
                                }
                                contentItem: Label {
                                    text: parent.text
                                    color: enabled ? "#cfd3d6" : "#6b6f75"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
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
                background: Rectangle { color: "#2a2c2f"; radius: 6; border.color: "#2f3236" }
                contentItem: Label { text: parent.text; color: "#e6e6e6"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
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

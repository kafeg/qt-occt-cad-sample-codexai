import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
                            text: model.title
                            implicitHeight: 44
                            padding: 0
                            background: Rectangle {
                                color: tabBar.currentIndex === index ? theme.tabActiveBg : theme.tabBg
                                border.color: theme.border
                                radius: 0
                            }
                            contentItem: RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 8
                                Label {
                                    Layout.fillWidth: true
                                    text: parent.parent.text
                                    color: theme.text
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                                ToolButton {
                                    id: closeBtn
                                    Accessible.name: qsTr("Close Tab")
                                    text: "✕"
                                    implicitWidth: 20
                                    implicitHeight: 20
                                    padding: 0
                                    onClicked: closeDocumentTab(index)
                                    enabled: tabsModel.count > 1
                                    background: Rectangle {
                                        radius: 0
                                        color: closeBtn.down ? theme.panelMuted : "transparent"
                                        border.color: enabled ? theme.border : theme.divider
                                    }
                                    contentItem: Label {
                                        text: parent.text
                                        color: enabled ? theme.textMuted : theme.textWeak
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

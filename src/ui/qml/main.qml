import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 960
    height: 600
    title: qsTr("vibecad")

    property int tabCounter: 0

    // No header; tabs start immediately at the top

    function addDocumentTab() {
        tabCounter += 1
        tabsModel.append({ title: qsTr("Document ") + tabCounter })
        tabBar.currentIndex = tabsModel.count - 1
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
            spacing: 4
            TabBar {
                id: tabBar
                Layout.fillWidth: true
                Repeater {
                    model: tabsModel
                    TabButton { text: model.title }
                }
            }
            ToolButton {
                id: addTabButton
                text: "+"
                Accessible.name: qsTr("Add Tab")
                onClicked: addDocumentTab()
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

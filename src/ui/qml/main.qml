import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    id: root
    visible: true
    width: 960
    height: 600
    title: qsTr("vibecad")

    Column {
        anchors.centerIn: parent
        spacing: 12
        Label { text: "Welcome to vibecad (stub UI)" }
        Row {
            spacing: 8
            Image { source: "qrc:/ui/icons/box.svg"; width: 32; height: 32; fillMode: Image.PreserveAspectFit }
            Image { source: "qrc:/ui/icons/cylinder.svg"; width: 32; height: 32; fillMode: Image.PreserveAspectFit }
        }
        Button { text: "OK"; onClicked: Qt.quit() }
    }
}

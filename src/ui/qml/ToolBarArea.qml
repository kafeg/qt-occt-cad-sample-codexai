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
        // Mode segmented control (icons + tooltips)
        Rectangle {
            id: modeSegment
            implicitHeight: 40
            implicitWidth: segRow.implicitWidth
            color: theme.toolbarTabOn // slightly lighter than toolbar for emphasis
            border.color: theme.border
            radius: 0
            Row {
                id: segRow
                spacing: 0
                anchors.fill: parent

                // Solid
                ToolButton {
                    id: solidBtn
                    checkable: true
                    checked: root.mode === 0
                    onClicked: root.mode = 0
                    hoverEnabled: true
                    Accessible.name: qsTr("Solid")
                    implicitHeight: parent.height
                    implicitWidth: 48
                    background: Rectangle {
                        color: solidBtn.checked ? theme.toolbarTabOn : theme.toolbarTabBg
                        radius: 0
                        border.color: "transparent"
                    }
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: "qrc:/ui/icons/mode-solid.svg"
                        sourceSize.width: 20
                        sourceSize.height: 20
                        fillMode: Image.PreserveAspectFit
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Solid")
                }

                // Divider between segments
                Rectangle { width: 1; color: theme.border; anchors.top: parent.top; anchors.bottom: parent.bottom }

                // Sketch
                ToolButton {
                    id: sketchBtn
                    checkable: true
                    checked: root.mode === 1
                    onClicked: root.mode = 1
                    hoverEnabled: true
                    Accessible.name: qsTr("Sketch")
                    implicitHeight: parent.height
                    implicitWidth: 48
                    background: Rectangle {
                        color: sketchBtn.checked ? theme.toolbarTabOn : theme.toolbarTabBg
                        radius: 0
                        border.color: "transparent"
                    }
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: "qrc:/ui/icons/mode-sketch.svg"
                        sourceSize.width: 20
                        sourceSize.height: 20
                        fillMode: Image.PreserveAspectFit
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Sketch")
                }
            }
        }

        // Arrow pointer indicating linked tool group
        Item {
            id: segArrow
            width: 12
            height: modeSegment.implicitHeight
            Layout.alignment: Qt.AlignVCenter
            Canvas {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                width: 12
                height: 16
                onPaint: {
                    var ctx = getContext('2d');
                    ctx.clearRect(0, 0, width, height);
                    ctx.beginPath();
                    ctx.moveTo(0, 0);
                    ctx.lineTo(0, height);
                    ctx.lineTo(width, height/2);
                    ctx.closePath();
                    ctx.fillStyle = modeSegment.color;
                    ctx.fill();
                    // subtle edge line
                    ctx.beginPath();
                    ctx.moveTo(0, 0);
                    ctx.lineTo(0, height);
                    ctx.strokeStyle = theme.border;
                    ctx.lineWidth = 1;
                    ctx.stroke();
                }
            }
        }

        // Tool groups per mode
        StackLayout {
            id: toolGroups
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            currentIndex: root.mode

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

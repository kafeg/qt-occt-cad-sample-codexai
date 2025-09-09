import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ToolBar {
    id: root
    height: 56
    property int mode: 0 // 0: Solid, 1: Sketch

    // Local theme instance (flat palette)
    readonly property Theme theme: Theme {}

    // remove built-in paddings of ToolBar
    padding: 0
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    background: Rectangle { color: theme.toolbarBg; border.color: theme.border; height: 56; radius: 0 }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // --- Left block with arrow ---
        Item {
            Layout.preferredWidth: height * 2.3
            Layout.fillHeight: true

            // Rectangle {
            //     id: leftBlock
            //     anchors.fill: parent
            //     anchors.rightMargin: 20
            //     color: theme.toolbarTabOn
            // }

            // Arrow
            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.reset();
                    ctx.fillStyle = theme.toolbarTabOn
                    ctx.beginPath();
                    ctx.moveTo(0, 0);
                    ctx.lineTo(width - 20, 0);
                    ctx.lineTo(width, height/2);
                    ctx.lineTo(width - 20, height);
                    ctx.lineTo(0, height);
                    ctx.closePath();
                    ctx.fill();
                }
            }

            Row {
                anchors.left: parent.left
                height: parent.height
                spacing: 0
                leftPadding: root.height * 0.2

                ButtonGroup { id: modeGroup }

                // Solid
                ToolButton {
                    id: solidBtn
                    checkable: true
                    checked: root.mode === 0
                    onClicked: root.mode = 0
                    hoverEnabled: true
                    ButtonGroup.group: modeGroup
                    Accessible.name: qsTr("Solid")
                    implicitHeight: parent.height * 0.8
                    implicitWidth: implicitHeight
                    anchors.verticalCenter: parent.verticalCenter
                    background: Rectangle {
                        color: solidBtn.checked ? theme.toolbarTabBg : theme.toolbarTabOn
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
                    ButtonGroup.group: modeGroup
                    Accessible.name: qsTr("Sketch")
                    implicitHeight: parent.height * 0.8
                    implicitWidth: implicitHeight
                    anchors.verticalCenter: parent.verticalCenter
                    background: Rectangle {
                        color: sketchBtn.checked ? theme.toolbarTabBg : theme.toolbarTabOn
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

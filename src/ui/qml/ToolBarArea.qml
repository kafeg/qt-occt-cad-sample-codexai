import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * ToolBarArea - Main toolbar with mode selection and tools
 * 
 * Features:
 * - Switch between Solid/Sketch modes
 * - Display corresponding tools for each mode
 * - Visual arrow to highlight active mode
 */
ToolBar {
    id: root
    
    // === Public properties ===
    property int mode: 0 // 0: Solid, 1: Sketch
    
    // === Constants ===
    readonly property int toolbarHeight: 56
    readonly property int modeButtonSize: 40
    readonly property int arrowWidth: 20
    readonly property int modeSelectorWidth: toolbarHeight * 2.3
    
    // === Theme ===
    readonly property Theme theme: Theme {}
    
    // === ToolBar settings ===
    height: toolbarHeight
    padding: 0
    
    background: Rectangle { 
        color: theme.toolbarBg
        border.color: theme.border
        height: toolbarHeight
        radius: 0 
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // === Mode selector with visual arrow ===
        Item {
            id: modeSelector
            Layout.preferredWidth: root.modeSelectorWidth
            Layout.fillHeight: true
            
            // === Visual arrow ===
            Canvas {
                id: arrowCanvas
                anchors.fill: parent
                
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    ctx.fillStyle = root.theme.toolbarTabOn
                    ctx.beginPath()
                    ctx.moveTo(0, 0)
                    ctx.lineTo(width - root.arrowWidth, 0)
                    ctx.lineTo(width, height / 2)
                    ctx.lineTo(width - root.arrowWidth, height)
                    ctx.lineTo(0, height)
                    ctx.closePath()
                    ctx.fill()
                }
            }
            
            // === Mode buttons ===
            Row {
                id: modeButtons
                anchors.left: parent.left
                anchors.leftMargin: 10
                height: parent.height
                spacing: 0
                
                ButtonGroup { id: modeGroup }
                
                // Solid mode button
                ToolButton {
                    id: solidBtn
                    checkable: true
                    checked: root.mode === 0
                    onClicked: root.mode = 0
                    hoverEnabled: true
                    ButtonGroup.group: modeGroup
                    Accessible.name: qsTr("Solid")
                    
                    height: 40
                    width: 40
                    anchors.verticalCenter: parent.verticalCenter
                    
                    background: Rectangle { color: solidBtn.checked ? root.theme.toolbarTabBg : root.theme.toolbarTabOn; radius: 0; border.color: "transparent" }
                    
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
                
                // Divider
                Rectangle { width: 1; color: root.theme.border; anchors.top: parent.top; anchors.bottom: parent.bottom }
                
                // Sketch mode button
                ToolButton {
                    id: sketchBtn
                    checkable: true
                    checked: root.mode === 1
                    onClicked: root.mode = 1
                    hoverEnabled: true
                    ButtonGroup.group: modeGroup
                    Accessible.name: qsTr("Sketch")
                    
                    height: 40
                    width: 40
                    anchors.verticalCenter: parent.verticalCenter
                    
                    background: Rectangle { color: sketchBtn.checked ? root.theme.toolbarTabBg : root.theme.toolbarTabOn; radius: 0; border.color: "transparent" }
                    
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

        // === Tool groups ===
        StackLayout {
            id: toolGroups
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            currentIndex: root.mode
            
            // === Solid mode tools ===
            RowLayout {
                id: solidTools
                spacing: 4
                
                Repeater {
                    model: [
                        { name: qsTr("New Sketch"), icon: "qrc:/ui/icons/box.svg" },
                        { name: qsTr("Extrude"), icon: "qrc:/ui/icons/cylinder.svg" }
                    ]
                    
                    ToolButton {
                        text: modelData.name
                        icon.source: modelData.icon
                        background: Rectangle { 
                            color: "transparent"
                            radius: 0 
                        }
                        contentItem: Label { 
                            text: parent.text
                            color: root.theme.text
                            padding: 8
                        }
                    }
                }
            }
            
            // === Sketch mode tools ===
            RowLayout {
                id: sketchTools
                spacing: 4
                
                Repeater {
                    model: [
                        { name: qsTr("Point") },
                        { name: qsTr("Line") }
                    ]
                    
                    ToolButton {
                        text: modelData.name
                        background: Rectangle { 
                            color: "transparent"
                            radius: 0 
                        }
                        contentItem: Label { 
                            text: parent.text
                            color: root.theme.text
                            padding: 8
                        }
                    }
                }
            }
        }

        // === Future quick actions on the right ===
        Item { Layout.fillWidth: true }
    }
}
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
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    spacing: 0
    
    background: Rectangle { 
        color: theme.toolbarBg
        border.color: theme.border
        border.width: 0
        height: toolbarHeight
        radius: 0 
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        // === Mode selector with visual arrow ===
        Item {
            id: modeSelector
            Layout.preferredWidth: root.modeSelectorWidth
            Layout.fillHeight: true
            Layout.leftMargin: 0
            Layout.topMargin: 0
            Layout.bottomMargin: 0
            
            // === Visual arrow ===
            Rectangle {
                anchors.fill: parent
                color: root.theme.toolbarBg
            }
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
                height: parent.height
                spacing: 0
                
                ButtonGroup { id: modeGroup }
                
                // Solid mode button
                ToolButton {
                    id: solidBtn
                    checkable: false // acts as an indicator only
                    checked: root.mode === 0
                    hoverEnabled: true
                    ButtonGroup.group: modeGroup
                    Accessible.name: qsTr("Solid")
                    
                    height: parent.height
                    width: parent.height
                    
                    background: Rectangle { color: root.mode === 0 ? root.theme.toolbarTabBg : root.theme.toolbarTabOn; radius: 0; border.color: "transparent" }
                    
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: "qrc:/ui/icons/mode-solid.svg"
                        sourceSize.width: 32
                        sourceSize.height: 32
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
                    checkable: false // acts as an indicator only
                    checked: root.mode === 1
                    hoverEnabled: true
                    ButtonGroup.group: modeGroup
                    Accessible.name: qsTr("Sketch")
                    
                    height: parent.height
                    width: parent.height
                    
                    background: Rectangle { color: root.mode === 1 ? root.theme.toolbarTabBg : root.theme.toolbarTabOn; radius: 0; border.color: "transparent" }
                    
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: "qrc:/ui/icons/mode-sketch.svg"
                        sourceSize.width: 32
                        sourceSize.height: 32
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
            Layout.fillHeight: true
            Layout.topMargin: 0
            Layout.bottomMargin: 0
            currentIndex: root.mode
            
            // === Solid mode tools ===
            RowLayout {
                id: solidTools
                spacing: 4
                
                Repeater {
                    model: [
                        { name: qsTr("New Sketch"), icon: "qrc:/ui/icons/new-sketch.svg" },
                        { name: qsTr("Extrude"), icon: "qrc:/ui/icons/extrude.svg" },
                        { name: qsTr("Revolve"), icon: "qrc:/ui/icons/revolve.svg" }
                    ]
                    
                    ToolButton {
                        text: modelData.name
                        width: parent.height
                        height: parent.height
                        background: Rectangle { 
                            color: "transparent"
                            radius: 0 
                        }
                        contentItem: Image {
                            anchors.centerIn: parent
                            source: modelData.icon
                            sourceSize.width: 40
                            sourceSize.height: 40
                            fillMode: Image.PreserveAspectFit
                        }
                        ToolTip.visible: hovered
                        ToolTip.text: modelData.name
                        onClicked: {
                            if (modelData.name === qsTr("New Sketch")) {
                                root.mode = 1
                            }
                        }
                    }
                }
            }
            
            // === Sketch mode tools ===
            RowLayout {
                id: sketchTools
                spacing: 4
                
                // Finish Sketch button
                ToolButton {
                    text: qsTr("Finish Sketch")
                    width: parent.height
                    height: parent.height
                    background: Rectangle {
                        color: "transparent"
                        radius: 0
                    }
                    contentItem: Image {
                        anchors.centerIn: parent
                        source: "qrc:/ui/icons/finish-sketch.svg"
                        sourceSize.width: 40
                        sourceSize.height: 40
                        fillMode: Image.PreserveAspectFit
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Finish Sketch")
                    onClicked: root.mode = 0
                }

                Repeater {
                    model: [
                        { name: qsTr("Point"), icon: "qrc:/ui/icons/sketch-point.svg" },
                        { name: qsTr("Line"), icon: "qrc:/ui/icons/sketch-line.svg" }
                    ]
                    
                    ToolButton {
                        text: modelData.name
                        width: parent.height
                        height: parent.height
                        background: Rectangle { 
                            color: "transparent"
                            radius: 0 
                        }
                        contentItem: Image {
                            anchors.centerIn: parent
                            source: modelData.icon
                            sourceSize.width: 40
                            sourceSize.height: 40
                            fillMode: Image.PreserveAspectFit
                        }
                        ToolTip.visible: hovered
                        ToolTip.text: modelData.name
                    }
                }
            }
        }

        // === Future quick actions on the right ===
        Item { Layout.fillWidth: true }
    }
}
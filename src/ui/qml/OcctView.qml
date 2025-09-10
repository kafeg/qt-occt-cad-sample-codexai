import QtQuick
import QtQuick.Controls
import VibeCAD.Viewer 1.0

Item {
    id: root
    readonly property Theme theme: Theme {}
    
    // OpenCascade 3D viewer
    OcctViewer {
        id: occtViewer
        anchors.fill: parent
        
        focus: true
        
        // When the viewer is ready and has a size, reset camera to origin
        Component.onCompleted: {
            if (width > 0 && height > 0) {
                resetViewToOrigin(1.2)
            }
        }
        onWidthChanged: if (width > 0 && height > 0) resetViewToOrigin(1.2)
        onHeightChanged: if (width > 0 && height > 0) resetViewToOrigin(1.2)
        
        onSelectionChanged: {
            console.log("Selection changed")
        }
    }
    
    // ViewCube controls overlay
    Rectangle {
        id: viewCubeControls
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        width: 200
        height: 160
        color: theme.panelBg
        border.color: theme.border
        border.width: 1
        radius: 4
        opacity: 0.9
        
        Column {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 4
            
            Label {
                text: qsTr("View Controls")
                font.bold: true
                color: theme.text
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Row {
                spacing: 4
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: qsTr("Iso")
                    width: 40
                    height: 24
                    onClicked: occtViewer.resetViewToOrigin(1.2)
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: qsTr("Front")
                    width: 40
                    height: 24
                    onClicked: occtViewer.resetViewToOrigin(1.0)
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: qsTr("Right")
                    width: 40
                    height: 24
                    onClicked: occtViewer.resetViewToOrigin(1.0)
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: qsTr("Top")
                    width: 40
                    height: 24
                    onClicked: occtViewer.resetViewToOrigin(1.0)
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            
            Row {
                spacing: 4
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: qsTr("Fit")
                    width: 40
                    height: 24
                    onClicked: occtViewer.resetViewToOrigin(1.2)
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: qsTr("Reset")
                    width: 40
                    height: 24
                    onClicked: occtViewer.resetViewToOrigin(1.2)
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: qsTr("Shade")
                    width: 40
                    height: 24
                    onClicked: occtViewer.resetViewToOrigin(1.2)
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: qsTr("ViewCube")
                    width: 40
                    height: 24
                    onClicked: occtViewer.resetViewToOrigin(1.2)
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            
            // Test shapes row
            Row {
                spacing: 4
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: qsTr("Box")
                    width: 40
                    height: 24
                    onClicked: occtViewer.addTestBox()
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: qsTr("Cyl")
                    width: 40
                    height: 24
                    onClicked: occtViewer.addTestCylinder()
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: qsTr("Sphere")
                    width: 40
                    height: 24
                    onClicked: occtViewer.addTestSphere()
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                Button {
                    text: qsTr("Clear")
                    width: 40
                    height: 24
                    onClicked: occtViewer.clearBodies()
                    background: Rectangle { 
                        color: parent.pressed ? theme.accent : theme.panelMuted
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.text
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
    
    // Status bar at bottom
    Rectangle {
        id: statusBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 24
        color: theme.panelBg
        border.color: theme.border
        border.width: 1
        
        Row {
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            spacing: 16
            
            Text {
                text: qsTr("Ready")
                color: theme.textMuted
                font.pixelSize: 12
            }
            
            Text {
                text: qsTr("Mouse: Rotate (LMB), Pan (MMB), Zoom (Wheel)")
                color: theme.textMuted
                font.pixelSize: 12
            }
        }
    }
}

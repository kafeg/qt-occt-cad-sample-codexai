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
        viewCubeVisible: true
        viewCubeSize: 100
        viewMode: 1 // Shaded
        viewOrientation: 0 // Isometric
        
        focus: true
        
        onViewChanged: {
            console.log("View changed")
        }
        
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
                    onClicked: occtViewer.viewOrientation = 0
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
                    onClicked: occtViewer.viewOrientation = 1
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
                    onClicked: occtViewer.viewOrientation = 2
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
                    onClicked: occtViewer.viewOrientation = 3
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
                    onClicked: occtViewer.fitAll()
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
                    onClicked: occtViewer.resetView()
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
                    text: occtViewer.viewMode === 0 ? qsTr("Wire") : qsTr("Shade")
                    width: 40
                    height: 24
                    onClicked: occtViewer.viewMode = occtViewer.viewMode === 0 ? 1 : 0
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
                    text: occtViewer.viewCubeVisible ? qsTr("Hide") : qsTr("Show")
                    width: 40
                    height: 24
                    onClicked: occtViewer.viewCubeVisible = !occtViewer.viewCubeVisible
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
                    onClicked: occtViewer.displayTestBox()
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
                    onClicked: occtViewer.displayTestCylinder()
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
                    onClicked: occtViewer.displayTestSphere()
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
                    onClicked: occtViewer.clearScene()
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    implicitWidth: 260
    property bool collapsed: false
    readonly property Theme theme: Theme {}
    background: Rectangle { color: theme.panelBg; border.color: "transparent"; radius: 0 }

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        // Header with collapse button and separator
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            
            // Header row with title and collapse button
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                
                Label { 
                    text: qsTr("Browser"); 
                    font.bold: true; 
                    color: theme.textMuted;
                    Layout.fillWidth: true
                }
                
                ToolButton {
                    id: collapseButton
                    text: root.collapsed ? "▶" : "▼"
                    implicitWidth: 20
                    implicitHeight: 20
                    onClicked: root.collapsed = !root.collapsed
                    background: Rectangle { 
                        color: collapseButton.down ? theme.panelMuted : "transparent"
                        radius: 2
                    }
                    contentItem: Text {
                        text: parent.text
                        color: theme.textMuted
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            
            Rectangle { 
                height: 1; 
                color: theme.divider; 
                Layout.fillWidth: true 
            }
        }

        // Collapsible content
        ColumnLayout {
            Layout.fillWidth: true
            visible: !root.collapsed
            spacing: 4
            
            // Sections: Origin, Bodies, Sketches
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Label { text: qsTr("Origin"); color: theme.textMuted }
                Repeater {
                    model: 1
                    Rectangle {
                        Layout.fillWidth: true
                        height: 20
                        color: "transparent"
                        Text { 
                            text: "• XY, XZ, YZ planes"; 
                            color: theme.text 
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                        }
                    }
                }
            }

            Rectangle { height: 1; color: theme.divider; Layout.fillWidth: true }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Label { text: qsTr("Bodies"); color: theme.textMuted }
                Repeater {
                    model: 3
                    Rectangle {
                        Layout.fillWidth: true
                        height: 24
                        color: mouseArea.containsMouse ? theme.panelMuted : "transparent"
                        radius: 2
                        
                        Text { 
                            text: "Body " + (index+1); 
                            color: theme.text 
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                        }
                        
                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                console.log("Body " + (index+1) + " selected")
                                // В будущем здесь будет логика выбора объекта
                            }
                        }
                    }
                }
            }

            Rectangle { height: 1; color: theme.divider; Layout.fillWidth: true }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Label { text: qsTr("Sketches"); color: theme.textMuted }
                Repeater {
                    model: 2
                    Rectangle {
                        Layout.fillWidth: true
                        height: 24
                        color: mouseArea2.containsMouse ? theme.panelMuted : "transparent"
                        radius: 2
                        
                        Text { 
                            text: "Sketch " + (index+1); 
                            color: theme.text 
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                        }
                        
                        MouseArea {
                            id: mouseArea2
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                console.log("Sketch " + (index+1) + " selected")
                                // В будущем здесь будет логика выбора эскиза
                            }
                        }
                    }
                }
            }
        }
    }
}

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    implicitWidth: 300
    property int mode: 0 // 0: Solid, 1: Sketch
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
                    text: root.mode === 0 ? qsTr("Solid Parameters") : qsTr("Sketch Parameters")
                    font.bold: true
                    color: theme.textMuted
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
            
            // Mode-specific content using StackLayout
            StackLayout {
                id: paramsStack
                Layout.fillWidth: true
                currentIndex: root.mode

                // Solid parameters
                ColumnLayout {
                    spacing: 8
                    
                    // Status message
                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        color: theme.panelMuted
                        border.color: theme.border
                        border.width: 1
                        radius: 4
                        
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("Select object(s) and set Extrude params")
                            color: theme.textMuted
                            font.pixelSize: 12
                        }
                    }
                    
                    // Parameter controls (placeholder for future implementation)
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        
                        // Distance parameter
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            
                            Label {
                                text: qsTr("Distance:")
                                color: theme.text
                                Layout.preferredWidth: 80
                            }
                            
                            SpinBox {
                                id: distanceSpinBox
                                from: 0
                                to: 1000
                                value: 10
                                stepSize: 1
                                editable: true
                                Layout.fillWidth: true
                                
                                background: Rectangle {
                                    color: theme.panelBg
                                    border.color: theme.border
                                    border.width: 1
                                    radius: 2
                                }
                                
                                textFromValue: function(value, locale) {
                                    return value + " mm"
                                }
                                
                                valueFromText: function(text, locale) {
                                    return parseInt(text.replace(" mm", "")) || 0
                                }
                            }
                        }
                        
                        // Direction parameter
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            
                            Label {
                                text: qsTr("Direction:")
                                color: theme.text
                                Layout.preferredWidth: 80
                            }
                            
                            ComboBox {
                                id: directionComboBox
                                model: ["Normal", "Reverse", "Both"]
                                currentIndex: 0
                                Layout.fillWidth: true
                                
                                background: Rectangle {
                                    color: theme.panelBg
                                    border.color: theme.border
                                    border.width: 1
                                    radius: 2
                                }
                            }
                        }
                    }
                    
                    // Action buttons
                    RowLayout {
                        Layout.alignment: Qt.AlignRight
                        spacing: 6
                        Layout.topMargin: 8
                        
                        Button {
                            text: qsTr("Cancel")
                            Layout.preferredWidth: 80
                            background: Rectangle { 
                                color: parent.pressed ? theme.panelMuted : theme.panelBg
                                border.color: theme.border
                                border.width: 1
                                radius: 2
                            }
                            contentItem: Label { 
                                text: parent.text
                                color: theme.text
                                horizontalAlignment: Text.AlignHCenter
                            }
                            onClicked: {
                                console.log("Extrude operation cancelled")
                                // Сброс параметров
                                distanceSpinBox.value = 10
                                directionComboBox.currentIndex = 0
                            }
                        }
                        
                        Button {
                            text: qsTr("OK")
                            Layout.preferredWidth: 80
                            background: Rectangle { 
                                color: parent.pressed ? Qt.darker(theme.accent, 1.2) : theme.accent
                                radius: 2
                            }
                            contentItem: Label { 
                                text: parent.text
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                            }
                            onClicked: {
                                console.log("Extrude operation applied - Distance:", distanceSpinBox.value, "Direction:", directionComboBox.currentText)
                                // В будущем здесь будет применение операции выдавливания
                            }
                        }
                    }
                }

                // Sketch parameters
                ColumnLayout {
                    spacing: 6
                    CheckBox {
                        text: qsTr("Show sketch grid")
                        contentItem: Label { text: parent.text; color: theme.text }
                        indicator: Rectangle { implicitWidth: 16; implicitHeight: 16; color: theme.panelMuted; border.color: theme.border }
                    }
                    CheckBox {
                        text: qsTr("Snap to grid")
                        contentItem: Label { text: parent.text; color: theme.text }
                        indicator: Rectangle { implicitWidth: 16; implicitHeight: 16; color: theme.panelMuted; border.color: theme.border }
                    }
                    Frame {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        background: Rectangle { color: theme.panelMuted; border.color: theme.border; radius: 0 }
                        Label { anchors.centerIn: parent; text: qsTr("Select tool: point/line, edit constraints"); color: theme.textMuted }
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignRight
                        spacing: 6
                        Button {
                            text: qsTr("Cancel")
                            background: Rectangle { color: theme.panelMuted; radius: 0 }
                            contentItem: Label { text: parent.text; color: theme.text }
                        }
                        Button {
                            text: qsTr("OK")
                            background: Rectangle { color: theme.accent; radius: 0 }
                            contentItem: Label { text: parent.text; color: "white" }
                        }
                    }
                }
            }
        }
    }
}

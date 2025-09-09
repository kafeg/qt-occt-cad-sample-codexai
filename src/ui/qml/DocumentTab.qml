import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent
    property int mode: 0 // 0: Solid, 1: Sketch
    property bool leftPanelCollapsed: false
    property bool rightPanelCollapsed: false
    readonly property Theme theme: Theme {}

    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: 0

        // Top toolbar area
        ToolBarArea {
            id: toolbar
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight
            onModeChanged: root.mode = toolbar.mode
        }

        // Main content area with three zones: left, center, right
        RowLayout {
            id: contentRow
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Left: Document browser + future panels container
            Rectangle {
                id: leftPaneContainer
                Layout.preferredWidth: root.leftPanelCollapsed ? 40 : 260
                Layout.minimumWidth: root.leftPanelCollapsed ? 40 : 220
                Layout.maximumWidth: root.leftPanelCollapsed ? 40 : 420
                Layout.fillHeight: true
                color: theme.panelBg
                border.color: theme.border
                border.width: 1
                
                ColumnLayout {
                    id: leftPane
                    anchors.fill: parent
                    anchors.margins: 1
                    spacing: 2

                    // Left panel header with collapse button
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        color: theme.panelMuted
                        border.color: theme.divider
                        border.width: 1
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 4
                            spacing: 8
                            
                            Label {
                                text: qsTr("Left Panel")
                                font.bold: true
                                color: theme.textMuted
                                Layout.fillWidth: true
                                visible: !root.leftPanelCollapsed
                            }
                            
                            ToolButton {
                                id: leftCollapseButton
                                text: root.leftPanelCollapsed ? "▶" : "▼"
                                implicitWidth: 20
                                implicitHeight: 20
                                onClicked: root.leftPanelCollapsed = !root.leftPanelCollapsed
                                background: Rectangle { 
                                    color: leftCollapseButton.down ? theme.panelBg : "transparent"
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
                    }

                    // Collapsible content
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: !root.leftPanelCollapsed
                        spacing: 2

                        DocumentBrowser {
                            id: documentBrowser
                            Layout.fillWidth: true
                            Layout.preferredHeight: 280
                        }

                        // Empty container reserved for future panels (e.g., Comments)
                        Pane {
                            id: extraPanelsContainer
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Accessible.name: qsTr("Extra Panels Container")
                            background: Rectangle { color: theme.panelBg; border.color: "transparent"; radius: 0 }
                        }
                    }
                }
            }

            // Vertical divider
            Rectangle { width: 1; color: theme.divider; Layout.fillHeight: true }

            // Center: OCCT view placeholder (main viewport)
            OcctView {
                id: occtView
                Layout.fillWidth: true
                Layout.fillHeight: true
                focus: true
            }

            // Vertical divider
            Rectangle { width: 1; color: theme.divider; Layout.fillHeight: true }

            // Right: Parameters pane
            Rectangle {
                id: rightPaneContainer
                Layout.preferredWidth: root.rightPanelCollapsed ? 40 : 300
                Layout.minimumWidth: root.rightPanelCollapsed ? 40 : 220
                Layout.maximumWidth: root.rightPanelCollapsed ? 40 : 480
                Layout.fillHeight: true
                color: theme.panelBg
                border.color: theme.border
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 1
                    spacing: 2

                    // Right panel header with collapse button
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        color: theme.panelMuted
                        border.color: theme.divider
                        border.width: 1
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 4
                            spacing: 8
                            
                            Label {
                                text: qsTr("Right Panel")
                                font.bold: true
                                color: theme.textMuted
                                Layout.fillWidth: true
                                visible: !root.rightPanelCollapsed
                            }
                            
                            ToolButton {
                                id: rightCollapseButton
                                text: root.rightPanelCollapsed ? "▶" : "▼"
                                implicitWidth: 20
                                implicitHeight: 20
                                onClicked: root.rightPanelCollapsed = !root.rightPanelCollapsed
                                background: Rectangle { 
                                    color: rightCollapseButton.down ? theme.panelBg : "transparent"
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
                    }

                    // Collapsible content
                    ParametersPane {
                        id: parametersPane
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: !root.rightPanelCollapsed
                        mode: root.mode
                    }
                }
            }
        }

        // Bottom timeline bar
        TimelineBar {
            id: timeline
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            // top divider
            topPadding: 0
        }
    }
}

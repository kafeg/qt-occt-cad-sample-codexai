import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent
    property int mode: 0 // 0: Solid, 1: Sketch
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
                Layout.preferredWidth: 260
                Layout.minimumWidth: 220
                Layout.maximumWidth: 420
                Layout.fillHeight: true
                color: theme.panelBg
                border.color: theme.border
                border.width: 1
                
                ColumnLayout {
                    id: leftPane
                    anchors.fill: parent
                    anchors.margins: 1
                    spacing: 2

                    // Content
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        DocumentBrowser {
                            id: documentBrowser
                            Layout.fillWidth: true
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
                Layout.preferredWidth: 300
                Layout.minimumWidth: 220
                Layout.maximumWidth: 480
                Layout.fillHeight: true
                color: theme.panelBg
                border.color: theme.border
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 1
                    spacing: 2

                    // Content
                    ParametersPane {
                        id: parametersPane
                        Layout.fillWidth: true
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

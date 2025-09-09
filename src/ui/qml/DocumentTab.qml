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
            ColumnLayout {
                id: leftPane
                Layout.preferredWidth: 260
                Layout.minimumWidth: 220
                Layout.maximumWidth: 420
                Layout.fillHeight: true
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
                    background: Rectangle { color: theme.panelBg; border.color: theme.border; radius: 0 }
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
            ParametersPane {
                id: parametersPane
                Layout.preferredWidth: 300
                Layout.minimumWidth: 220
                Layout.maximumWidth: 480
                Layout.fillHeight: true
                mode: root.mode
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

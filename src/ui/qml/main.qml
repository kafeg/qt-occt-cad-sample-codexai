import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Occt.Viewer 1.0

// Minimal QML UI scaffold inspired by the sample screenshot.
ApplicationWindow {
    id: win
    width: 1280
    height: 800
    visible: true
    title: qsTr("Parametric CAD (QML)")

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 8

            // App icon placeholder
            ToolButton { text: "≡" }

            // Top TabBar similar to SOLID / SURFACE / SHEET METAL
            TabBar {
                id: mainTabs
                Layout.fillWidth: true
                currentIndex: 0
                TabButton { text: "SOLID" }
                TabButton { text: "SURFACE" }
                TabButton { text: "SHEET METAL" }
            }

            // Right side actions placeholder
            ToolButton { text: qsTr("⋯") }
        }
    }

    // Main layout: left browser, center viewer, right panels, bottom timeline
    RowLayout {
        anchors.fill: parent
        spacing: 8

            // Left panel (Browser)
            Frame {
                id: browser
                Layout.preferredWidth: 260
                Layout.fillHeight: true
                padding: 8
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6
                    Label { text: qsTr("Browser"); font.bold: true }
                    Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; color: Qt.darker("#2b2d30", 1.2); radius: 4; opacity: 0.6 }
                }
            }

            // Center column: viewer + bottom timeline
            ColumnLayout {
                id: centerCol
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                // 3D View
                Frame {
                    id: viewFrame
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    padding: 0
                    OcctViewer {
                        id: occt
                        anchors.fill: parent
                        focus: true
                    }
                }

                // Bottom timeline placeholder
                Frame {
                    id: timeline
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    padding: 8
                    Label { text: qsTr("Timeline"); anchors.left: parent.left; anchors.top: parent.top }
                }
            }

            // Right stacked tools (Sketch / Constraints / Parameters)
            Frame {
                id: rightTools
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                padding: 8
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    GroupBox { title: qsTr("Sketch"); Layout.fillWidth: true; Layout.fillHeight: true; }
                    GroupBox { title: qsTr("Constraints"); Layout.fillWidth: true; Layout.preferredHeight: 180; }
                    GroupBox { title: qsTr("Parameters"); Layout.fillWidth: true; Layout.preferredHeight: 140; }
                }
            }
    }

    footer: ToolBar {
        RowLayout { anchors.fill: parent; spacing: 8
            Label { text: occt.glInfo; elide: Label.ElideRight; Layout.fillWidth: true }
        }
    }
}

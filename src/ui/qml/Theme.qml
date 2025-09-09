import QtQuick

QtObject {
    // Base palette (dark, flat)
    readonly property color windowBg:     "#171a1d"
    readonly property color viewBg:       "#111316"
    readonly property color toolbarBg:    "#2b2e31"
    readonly property color toolbarTabBg: "#2b2e31"
    readonly property color toolbarTabOn: "#32363a"
    readonly property color tabBg:        "#2a2d31"
    readonly property color tabActiveBg:  "#222529"
    readonly property color panelBg:      "#202225"
    readonly property color panelMuted:   "#26292d"
    readonly property color chipBg:       "#2a2d31"
    readonly property color border:       "#34393f"
    readonly property color divider:      "#2a2e33"

    readonly property color text:         "#e6e6e6"
    readonly property color textMuted:    "#9aa0a6"
    readonly property color textWeak:     "#6b6f75"
    readonly property color accent:       "#4c8bf5"

    // Typography
    readonly property int tabFontSize: 14
    readonly property int tabIconSize: 16

    // Toolbar segmentation
    readonly property color toolbarSegmentBg: "#393e44"   // lighter to stand out
    readonly property color toolbarToolsBg:   "#30343a"   // sits between segment and toolbar bg
}

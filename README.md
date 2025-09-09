# VibeCAD — QML CAD UI Skeleton

Basic, flat QML user interface for a CAD app built on Open CASCADE (OCCT) and Qt 6. The goal is to prototype the shell only (no real modeling yet) with a Fusion 360–like layout and multi‑document tabs.

## Build & Run

- Configure: `cmake --preset default`
- Build: `cmake --build --preset default`
- Run: `./build/src/cad-app`
- Tests: `ctest --preset default`

Dependencies are provided via `vcpkg.json` (Qt 6, OCCT, GTest). Presets for Linux/Windows are included in `CMakePresets.json`.

## UI Overview (Flat)

- Tabs: Top‑level TabBar hosts independent documents. A plus button adds a new tab. Tab height is increased for better hit targets.
- Toolbar: Inside each tab, a toolbar with two modes:
  - Solid: tools — New Sketch, Extrude (placeholders with dummy icons)
  - Sketch: tools — Point, Line
- Left: Document Browser placeholder with sections: Origin, Bodies, Sketches. Room is reserved below for future panels (e.g., Comments).
- Center: OCCT view placeholder (dark rectangle with label) to be replaced with an actual QOpenGLWidget/QQuick integration later.
- Right:
  - Solid mode: Parameters panel with OK/Cancel (placeholder content like Extrude inputs).
  - Sketch mode: Sketch settings (grid on/off, snap), plus OK/Cancel.
- Bottom: Timeline bar with horizontally scrolling feature chips (visual only).

Everything is implemented as separable QML components and assembled in `main.qml`.

## Project Structure (Essentials)

- `src/core`: Thin wrappers over OCCT primitives/booleans (e.g., `makeBox`, `makeCylinder`, `fuse`). No Qt deps.
- `src/doc`: `DocumentItem` base and minimal serialization API; registry for cross‑references.
- `src/model`: `Feature`, `Document`, primitives (`BoxFeature`, `CylinderFeature`), features (`ExtrudeFeature`, `MoveFeature`).
- `src/viewer`: Planned OCCT viewer (`OcctQOpenGLWidgetViewer`) and helpers.
- `src/ui/qml`: QML components; `main.qml` assembles the UI.
- `src/main.cpp`: App entry (executable `cad-app`).
- `tests/`: GoogleTest suites.

See `docs/architecture.md` for a short module overview and data flow.

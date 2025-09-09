# Architecture (Short)

This repository implements a minimal, layered skeleton for a CAD application based on Open CASCADE (OCCT) and Qt 6. The emphasis is on a clean separation of modules and a flat QML shell.

## Modules

- Core (`src/core`): Thin wrappers over OCCT primitives/booleans. Example APIs: `makeBox`, `makeCylinder`, `fuse`. No Qt deps.
- Document (`src/doc`): `DocumentItem` with ids and simple string‑blob serialization; registry for cross‑references.
- Model (`src/model`): `Feature` base + `Document` ordered items and `recompute()`. Provided features: Box, Cylinder, Extrude, Move.
- Viewer (`src/viewer`): Planned OCCT viewer (`OcctQOpenGLWidgetViewer`) and helpers; integrated later with QML or Widgets.
- UI (`src/ui/qml`): QML components and `main.qml` composing the shell; flat visual style.
- Sketch (`src/sketch`): Sketch data/serialization; consumed by `ExtrudeFeature` by id.
- Entry (`src/main.cpp`): Qt app bootstrap. Executable: `vibecad`.

## UI Layout (per Tab)

- Toolbar with two modes: Solid (New Sketch, Extrude), Sketch (Point, Line).
- Left: Document Browser with Origin, Bodies, Sketches; space reserved for future panels.
- Center: OCCT view placeholder (to be replaced with real viewer widget).
- Right: Parameters pane; mode‑specific content with OK/Cancel.
- Bottom: Timeline bar listing features (visual only).

## Data Flow (planned)

UI action → Command → Update `Document` → `recompute()` → Viewer reflects shapes. Wiring to QML shell is not implemented yet; most UI is placeholder.

# Repository Guidelines (Short)

## Project Structure & Module Organization
- `src/core`: Kernel API wrappers over OCCT primitives/booleans (no Qt deps). Exposed: `makeBox`, `makeCylinder`, `fuse`.
- `src/doc`: `DocumentItem` base with ids and minimal serialization API (string blob); registry support in `Document` for cross‑references.
- `src/model`: `Feature` (parameter map, serialization), `Document` (ordered items, recompute, registry), primitives/features: `BoxFeature`, `CylinderFeature`, `ExtrudeFeature`, `MoveFeature`.
- `src/viewer`: `OcctQOpenGLWidgetViewer` planned viewer and helpers (to be integrated with the QML shell later).
- `src/ui`: QML UI shell and components (tabs, toolbar, browser, parameters, timeline). Commands/dialogs are placeholders in the QML variant.
- `src/sketch`: Sketch data/serialization and viewer integration; used by `ExtrudeFeature` by id.
- `src/main.cpp`: Qt app entry. Executable name: `vibecad`.
- `tests/`: GoogleTest unit tests, including model, sketch, serialization, and UI integration.
- `CMakeLists.txt`, `CMakePresets.json`: Top-level build and presets; tests via `CTest`.
- `vcpkg/`, `vcpkg.json`: Manifest dependencies (`qtbase`, `opencascade`, `gtest`).
- `.clang-format`: Enforced C++ style (OCCT-leaning, Microsoft base).

## Build, Test, and Run
- Configure: `cmake --preset default` (Ninja, `build/`, default triplet)
- Build: `cmake --build --preset default`
- Run: `./build/src/vibecad`
- Tests: `ctest --preset default`
- Presets: `--preset linux` or `--preset windows` as needed

## Coding Style & Naming Conventions
- C++17, 2-space indent, 120-column limit, no tabs; pointer/reference alignment left.
- Run `clang-format -i <files>` (config in `.clang-format`).
- Naming: Classes/Qt types PascalCase (e.g., `OcctQOpenGLWidgetViewer`); functions/methods lowerCamelCase; constants UPPER_SNAKE; files PascalCase or lowercase with `.cpp/.h` consistent with peers.

## Testing Guidelines
- Framework: GoogleTest via `GTest::gtest(_main)`; registered with `add_test`.
- Location: place tests in `tests/`.
- Naming: end files with `_test.cpp` (e.g., `viewer_init_test.cpp`).
- Run locally: `ctest -V --test-dir build` for verbose output; keep tests fast and deterministic (no GUI requirement unless guarded).

## Commit & Pull Request Guidelines
- Commits: imperative mood, scoped and focused (e.g., `build: add linux preset`, `viewer: fix context init on macOS`).
- PRs: include summary, rationale, steps to test, screenshots if UI-visible, and link related issues.
- Checks: ensure build passes all presets you touched and `ctest` is green.

## Security & Configuration Tips
- Dependencies resolve via vcpkg manifest; adjust `VCPKG_TARGET_TRIPLET` in presets as needed.
- Cross-platform: prefer Qt/OCCT abstractions over platform APIs; avoid hardcoded paths; keep OpenGL usage within the provided `QOpenGLWidget` viewer.

## Quick Feature Map
- `BoxFeature`: params `Dx/Dy/Dz`; shape from `KernelAPI::makeBox`.
- `CylinderFeature`: params `Radius/Height`; shape from `KernelAPI::makeCylinder`.
- `ExtrudeFeature`: params `SketchId/Distance`; consumes Sketch from document registry; produces prism body.
- `MoveFeature`: params `SourceId`, exact `gp_Trsf` (+ decomposed Tx/Ty/Tz and Rx/Ry/Rz for readability); moves source and typically suppresses it.
- New features follow the same pattern: add params in `Feature::ParamKey` if common, implement `execute()` via `KernelAPI`/OCCT, add UI command+dialog if interactive.

## QML UI (Flat) — Brief
- Tabs at top: independent documents; plus to add new tab.
- Toolbar tabs: Solid (New Sketch, Extrude), Sketch (Point, Line).
- Left: Document Browser (Origin, Bodies, Sketches) with space for future panels.
- Center: OCCT view placeholder (to be replaced with integrated viewer).
- Right: Parameters pane with OK/Cancel; switches by mode (Solid/Sketch).
- Bottom: Timeline bar with feature chips (visual only).

## External Learning Resources

- Open CASCADE (OCCT)
  - OCCT Overview & Docs: https://dev.opencascade.org/doc/overview/html/index.html — Entry point to modeling data, algorithms, and visualization docs used by `KernelAPI` and AIS viewer.
  - OCCT Resources & Tutorials: https://dev.opencascade.org/resources — Official tutorials/samples to understand primitives, booleans, and viewer basics mirrored by our core/viewer layers.

- Qt 6
  - Qt 6 Documentation: https://doc.qt.io/qt-6/ — Main docs for widgets, events, and rendering; relevant for `MainWindow`, dialogs, and integration.
  - QOpenGLWidget: https://doc.qt.io/qt-6/qopenglwidget.html — Rendering surface used by `OcctQOpenGLWidgetViewer` to host OCCT’s 3D viewer.
  - Qt Widgets Module: https://doc.qt.io/qt-6/qtwidgets-index.html — Menus, dialogs, and layout patterns used throughout the UI layer.

- Autodesk Fusion 360 (Conceptual Reference)
  - Fusion 360 Help: https://help.autodesk.com/view/fusion360/ENU/ — Official reference for sketch/solid workflows that inspire our feature/timeline model.
  - Learning Paths & Tutorials: https://www.autodesk.com/learn/paths/learn-fusion-360 — Guided lessons on sketches, constraints, and solid features analogous to our planned sketch module and primitives.

## Notes for Agents
- Prefer local headers from `build/vcpkg_installed/<triplet>/include/` to confirm OCCT/Qt APIs.
- Baseline: OCCT 7.9.1, Qt 6.9.1.
- No network assumptions; keep changes minimal and respect code style.

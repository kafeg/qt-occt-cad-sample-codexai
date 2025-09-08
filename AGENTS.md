# Repository Guidelines

## Project Structure & Module Organization
- `src/core`: Kernel API wrappers over OCCT primitives/booleans (no Qt deps). Exposed: `makeBox`, `makeCylinder`, `fuse`.
- `src/doc`: `DocumentItem` base with ids and minimal serialization API (string blob); registry support in `Document` for cross‑references.
- `src/model`: `Feature` (parameter map, serialization), `Document` (ordered items, recompute, registry), primitives/features: `BoxFeature`, `CylinderFeature`, `ExtrudeFeature`, `MoveFeature`.
- `src/viewer`: `OcctQOpenGLWidgetViewer` (rendering, input, grid auto‑step, view cube, axes/trihedron, split views, manipulator, sketch overlay mode).
- `src/ui`: `MainWindow`, `TabPage`, panels (`FeatureHistoryPanel`, `DocumentTreePanel`), commands (`CreateBoxCommand`, `CreateCylinderCommand`, `CreateExtrudeCommand`, `MoveCommand`) and dialogs.
- `src/sketch`: Sketch data/serialization and viewer integration; used by `ExtrudeFeature` by id.
- `src/main.cpp`: Qt app entry. Executable name: `cad-app`.
- `tests/`: GoogleTest unit tests, including model, sketch, serialization, and UI integration.
- `CMakeLists.txt`, `CMakePresets.json`: Top-level build and presets; tests via `CTest`.
- `vcpkg/`, `vcpkg.json`: Manifest dependencies (`qtbase`, `opencascade`, `gtest`).
- `.clang-format`: Enforced C++ style (OCCT-leaning, Microsoft base).

## Build, Test, and Development Commands
- Configure: `cmake --preset default` (uses Ninja, `build/`, `arm64-osx` triplet by default).
- Build: `cmake --build --preset default` (or `cmake --build build`).
- Run: `./build/src/cad-app`.
- Tests: `ctest --preset default` (or `ctest --test-dir build`).
- Presets: use `--preset linux` or `--preset windows` to select triplets and out-of-source dirs.

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

## UI Actions
- File/Toolbar: `Add Box`, `Add Cylinder`, `Add Extrude` open dialogs and push features, then `Document::recompute()` and viewer sync.
- Move tool: `Move` activates an `AIS_Manipulator` on the selected body; Confirm commits a `MoveFeature`, Cancel discards temporary transforms.
- Split Views: `Split Views` toggles two subviews; viewer tracks active subview and updates input routing.
- Sketch overlay: viewer supports toggling a sketch into an edit overlay (Topmost w/o depth) for editing contexts.
- Test toolbar: `Clear All`, `Add Sample` (adds 3 boxes and 3 cylinders; cylinders row offset along +Y). Local transforms applied via AIS for layout only.

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
- Prefer real, local versions over guesses. Use the headers installed by vcpkg in `build/vcpkg_installed/<triplet>/include/opencascade/` to confirm OCCT APIs and enums. Also `build/vcpkg_installed/<triplet>/include` for Qt and other libs related sources.
- Baseline in this repo: OpenCascade 7.9.1, Qt 6.9.1 (Qt Declarative, Qt Quick Controls 2) — target these directly; avoid version guards unless explicitly requested.
- No network assumptions; rely on local docs/code. Keep changes minimal and respect the code style.

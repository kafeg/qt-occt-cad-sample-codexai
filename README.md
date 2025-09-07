# OCCT + Qt6 Parametric CAD Skeleton

An extensible, layered skeleton for a parametric CAD application built on Open CASCADE Technology (OCCT) and Qt6. The design and UX goals are inspired by modern parametric CAD workflows (browser with features/history, sketch → solid operations, command-driven UI), while keeping rendering and input decoupled from the geometric kernel.

![sample screenshot](/docs/sample-screenshot.png)

Initially inspired by https://github.com/gkv311/occt-samples-qopenglwidget

Mostly this is just playground for try to work with GitHub Codex AI CLI Agent. 99% code written automatically by Codex by text prompts and manual review. This is not ready project, just simple skeleton which tried to be extandable for future investigations if needed deeper realization of parametric CAD.

## Architecture

For a deeper overview and Fusion 360 concept mapping, see `docs/architecture.md`.

## Docs

- Architecture Overview: `docs/architecture.md`

- Viewer: Reusable OCCT `QOpenGLWidget` (`OcctQOpenGLWidgetViewer`) with input mapped via `AIS_ViewController`. Rendering and input are decoupled from the model and commands. Supports view cube, axes/trihedron, auto grid step, split views, and an interactive `AIS_Manipulator` for transform edits.
- Core (Kernel): Thin wrappers over OCCT (e.g., `BRepPrimAPI_*`, `BRepAlgoAPI_*`) in `KernelAPI` to isolate OCCT usage. Currently: `makeBox`, `makeCylinder`, `fuse`.
- Model: `Feature` base class with typed parameter map and resulting `TopoDS_Shape`; `Document` is an ordered list of items with recompute logic and a registry for cross‑references. Includes primitives (`BoxFeature`, `CylinderFeature`), `ExtrudeFeature` (profile‑based via Sketch id), and `MoveFeature` (rigid transform of a source feature; integrates with manipulator and serialization).
- UI: Command pattern + dialogs. Commands: Create Box, Create Cylinder, Create Extrude, and Move. Menu/toolbar actions open parameter dialogs and push features into the document. Panels: Feature History and Document Tree.
- Sketch: Implemented module with basic storage, constraints scaffolding, rendering in the viewer, edit overlay mode, and serialization. Used as a profile source for `ExtrudeFeature` by id.

## Repository Structure

- `src/`
  - `core/`: Kernel API (`KernelAPI.h/.cpp`): `makeBox`, `makeCylinder`, `fuse`.
  - `model/`: `Feature`, `Document`, primitives: `BoxFeature`, `CylinderFeature`, features: `ExtrudeFeature`, `MoveFeature`.
  - `viewer/`: `OcctQOpenGLWidgetViewer` (rendering, input, grid, axes/trihedron, view cube, split views, manipulator, sketch overlay).
  - `ui/`: Main window, tabs, history/tree panels, commands and dialogs: Create Box/Cylinder/Extrude, Move.
  - `sketch/`: Data structures, serialization and viewer integration for 2D sketches (used by `ExtrudeFeature`).
- `tests/`: GoogleTest unit tests and the test runner target.
- `vcpkg/`, `vcpkg.json`: Manifest-based dependencies (`qtbase`, `opencascade`, `gtest`).
- `CMakeLists.txt`, `CMakePresets.json`: Top-level build and presets; tests via `CTest`.
- `.clang-format`: Enforced C++ style (OCCT-leaning, Microsoft base).

## Current Status

- App target: `src/cad-app` (Qt6 GUI).
- Core wrappers: `makeBox`, `makeCylinder`, `fuse`.
- Features: `BoxFeature`, `CylinderFeature`, `ExtrudeFeature` (from Sketch id + distance), `MoveFeature` (stores exact `gp_Trsf` plus decomposed Tx/Ty/Tz and Rx/Ry/Rz for readability).
- UI commands: Create Box, Create Cylinder, Create Extrude, Move; “Add Sample” creates 3 boxes and 3 cylinders arranged in rows.
- Viewer: background gradient control, view cube, axes, origin trihedron, auto grid step, split views, overlay Z‑layers, manipulator integration to move/rotate and commit a `MoveFeature`.
- Sketch: in‑document registry with timeline mirror entries; viewer support to display and toggle edit overlay; serialization covered by tests.

## Building

The project uses [vcpkg](https://github.com/microsoft/vcpkg) and CMake presets.

```bash
git clone --recurse-submodules <repo>
cmake --preset default
cmake --build --preset default
```

- Run (Unix): `./build/src/cad-app`
- Tests: `ctest --preset default` (or `ctest --test-dir build`)

The `default` preset builds for the `arm64-osx` triplet and relies on `vcpkg.json` to provide `qtbase`, `opencascade`, and `gtest`. Use `--preset linux` or `--preset windows` to select triplets and out-of-source dirs.

## Usage

- File → Add Box: opens a dialog for Dx, Dy, Dz and appends a Box feature.
- File → Add Cylinder: opens a dialog for Radius, Height and appends a Cylinder feature.
- File → Add Extrude: selects/creates a Sketch and distance, appends an Extrude feature linked by Sketch id.
- File → Move: activates an interactive manipulator for the selected body; on confirm, commits a Move feature; on cancel, discards.
- View → Split Views (menu action “Split Views”): toggles two subviews side‑by‑side.
- Toolbar: Add Box, Add Cylinder, Add Extrude, Move, background slider; Test toolbar with Clear All and Add Sample.
- Add Sample: inserts 3 boxes and 3 cylinders; cylinders are offset in +Y; layout uses transient AIS local transforms only.

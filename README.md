# OCCT + Qt6 Parametric CAD Skeleton

An extensible, layered skeleton for a parametric CAD application built on Open CASCADE Technology (OCCT) and Qt6. The design and UX goals are inspired by modern parametric CAD workflows (browser with features/history, sketch → solid operations, command-driven UI), while keeping rendering and input decoupled from the geometric kernel.

![sample screenshot](/docs/sample-screenshot.png)

Initially inspired by https://github.com/gkv311/occt-samples-qopenglwidget

Mostly this is just playground for try to work with GitHub Codex AI CLI Agent. 99% code written automatically by Codex by text prompts and manual review. This is not ready project, just simple skeleton which tried to be extandable for future investigations if needed deeper realization of parametric CAD.

## Architecture

For a deeper overview and Fusion 360 concept mapping, see `docs/architecture.md`.

## Docs

- Architecture Overview: `docs/architecture.md`

- Viewer: Reusable OCCT `QOpenGLWidget` (`OcctQOpenGLWidgetViewer`) with input mapped via `AIS_ViewController`. Rendering and input are decoupled from commands.
- Core (Kernel): Thin wrappers over OCCT (e.g., `BRepPrimAPI_*`, `BRepAlgoAPI_*`) in `KernelAPI` to isolate OCCT usage. Currently: box, cylinder, fuse.
- Model: `Feature` base class with typed parameter map and resulting `TopoDS_Shape`; `Document` is an ordered list of features and recompute logic. Includes primitives (`BoxFeature`, `CylinderFeature`), `ExtrudeFeature`, and `MoveFeature` (rigid transform of an upstream feature).
- UI: Command pattern + dialogs. Example commands: Create Box, Create Cylinder. Menu/toolbar actions open parameter dialogs and push features into the document.
- Sketch: Placeholder module for future sketch/constraints integration.

## Repository Structure

- `src/`
  - `core/`: Kernel API (`KernelAPI.h/.cpp`): `makeBox`, `makeCylinder`, `fuse`.
  - `model/`: `Feature`, `Document`, primitives: `BoxFeature`, `CylinderFeature`.
  - `viewer/`: `OcctQOpenGLWidgetViewer` (rendering, input, grid, axes/trihedron).
  - `ui/`: Main window, tabs, commands and dialogs: Create Box/Cylinder.
  - `sketch/`: Placeholder interface for future sketcher.
- `tests/`: GoogleTest unit tests and the test runner target.
- `vcpkg/`, `vcpkg.json`: Manifest-based dependencies (`qtbase`, `opencascade`, `gtest`).
- `CMakeLists.txt`, `CMakePresets.json`: Top-level build and presets; tests via `CTest`.
- `.clang-format`: Enforced C++ style (OCCT-leaning, Microsoft base).

## Current Status

- App target: `src/cad-app` (Qt6 GUI).
- Core wrappers: box, cylinder, fuse.
- Features: `BoxFeature`, `CylinderFeature`, `ExtrudeFeature`, `MoveFeature` (stores Tx/Ty/Tz and Rx/Ry/Rz in degrees).
- UI commands: Create Box, Create Cylinder; “Add Sample” creates 3 boxes and 3 cylinders arranged in a grid.
- Viewer: background gradient control, view cube, axes, origin trihedron, auto grid step, basic `AIS_Manipulator` integration to move/rotate selected shape and commit a `MoveFeature`.

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

- File → Add Box: opens a dialog for dx, dy, dz and adds a box feature.
- File → Add Cylinder: opens a dialog for radius, height and adds a cylinder feature.
- Toolbar: Add Box, Add Cylinder, background slider, and a Test toolbar with Clear All and Add Sample.
- Add Sample: inserts 3 boxes in a row (X-axis) and 3 cylinders in a parallel row offset in +Y; transforms applied via AIS local transforms.

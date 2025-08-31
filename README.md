# OCCT + Qt6 Parametric CAD Skeleton

An extensible, layered skeleton for a parametric CAD application built on Open CASCADE Technology (OCCT) and Qt6. The design and UX goals are inspired by modern parametric CAD workflows (browser with features/history, sketch → solid operations, command-driven UI), while keeping rendering and input decoupled from the geometric kernel.

![sample screenshot](/images/occt-qopenglwidget-sample-wnt.png)

## Architecture

- Visualization Layer: Qt `QOpenGLWidget` viewer based on OCCT. Rendering (Viewer) and input mapping (Controller) are separated so UI commands stay independent from the 3D kernel implementation.
- Geometry Kernel: Thin wrappers over OCCT (e.g., BRepPrimAPI_, BRepAlgoAPI_) unified behind a Kernel API to isolate OCCT details and enable swapping the implementation later.
- Document/Data Model: `Document` holds features (e.g., sketch, extrude, boolean operations) and the build history. Each operation is an object with parameters and references to prior elements; changing parameters triggers recompute and scene update.
- Sketcher & Constraints: Dedicated module for 2D sketches using OCCT curves and an internal/external constraint solver. Sketch results feed volumetric operations.
- Command System & UI: Commands are decoupled from UI (Command pattern). Each command operates on the `Document` and invokes kernel operations. UI provides toolbars, menus, and parameter dialogs, wired via signals/slots.
- I/O & Project Format: Import/export STEP/IGES/other static formats and serialization of a native project format (e.g., JSON).

## Repository Structure

- `src/`
  - `core/`: Kernel API wrappers over OCCT primitives/booleans (no Qt dependencies).
  - `viewer/`: Reusable OCCT `QOpenGLWidget` viewer for all view modes; input translation helpers.
  - `ui/`: UI layer (toolbars/menus/commands). Placeholder for future modules.
  - `model/`: `Document`, `Feature` and build-history basics.
  - `sketch/`: Sketcher + constraints module (bootstrap placeholder).
- `occt-qopenglwidget/`: Sample Qt app wiring the viewer and basic UI for demonstration.
- `tests/`: GoogleTest unit tests and CMake test target.
- `vcpkg/`, `vcpkg.json`: Manifest-based dependencies (`qtbase`, `opencascade`, `gtest`).
- `CMakeLists.txt`, `CMakePresets.json`: Top-level build and presets; tests via `CTest`.
- `.clang-format`: Enforced C++ style (OCCT-leaning, Microsoft base).

## Current Status

- Project skeleton initialized under `src/` with `core/`, `viewer/`, `model/`, `sketch/` (and placeholder for `ui/`).
- Existing OCCT viewer migrated into `src/viewer/` and adapted for reuse across modes.
- Minimal kernel wrappers (box, fuse) in `core/` with no Qt dependency.
- Basic `Document`/`Feature` model in `model/`.
- Demo app shows a toolbar (bodies toggle, background slider), red/green guides, and an origin trihedron.

## Building

The project uses [vcpkg](https://github.com/microsoft/vcpkg) and CMake presets.

```bash
git clone --recurse-submodules <repo>
cmake --preset default
cmake --build --preset default
```

- Run (Unix): `./build/src/cad-app`
- Tests: `ctest --preset default` (or `ctest --test-dir build`)

The `default` preset builds for the `arm64-osx` triplet and relies on `vcpkg.json` to provide `qtbase` and `opencascade`. Use `--preset linux` or `--preset windows` to select triplets and out-of-source dirs.

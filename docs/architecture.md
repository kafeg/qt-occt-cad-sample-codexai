# Architecture Overview

This repository implements a layered, modular skeleton for a parametric CAD application using Open CASCADE Technology (OCCT) for geometry and Qt6 for UI and rendering. The design mirrors modern CAD patterns (e.g., Fusion 360) with a clean separation between kernel, model, viewer, and UI command layers.

## Modules

- Core (KernelAPI)
  - Purpose: Thin, testable wrappers over OCCT primitives and booleans to isolate kernel usage from the rest of the app.
  - Key APIs: `makeBox(dx, dy, dz)`, `makeCylinder(radius, height)`, `fuse(a, b)` returning `TopoDS_Shape`.
  - Notes: Encapsulates `BRepPrimAPI_*` and `BRepAlgoAPI_*` usage. No Qt dependencies.

- Document Items (`src/doc`)
  - `DocumentItem`: Base class with unique ids and a minimal serialization interface (`serialize()`/`deserialize()` string blob) for persistence and cross‑reference stability.
  - Used by `Feature` (handle‑based) and `Sketch` (std::shared_ptr in registry).

- Model
  - Feature: Base type with a typed parameter map and a resulting `TopoDS_Shape`; supports serialization.
  - Document: Ordered list of items with `recompute()`; iterates features, calls `Feature::execute()`, and holds results. Maintains a registry (`id` → `DocumentItem`) and an ordered sketch list for dependency resolution and timeline mirroring.
  - Primitives/Features:
    - `BoxFeature`, `CylinderFeature` → call `KernelAPI` to produce solids.
    - `ExtrudeFeature` → references a `Sketch` by id and distance to create a prism body.
    - `MoveFeature` → applies a rigid `gp_Trsf` to a source feature by id (exact transform stored + decomposed params for readability). Typically suppresses the source in the view.

- Viewer
  - `OcctQOpenGLWidgetViewer`: A reusable `QOpenGLWidget` that integrates OCCT viewer/contexts with `AIS_ViewController` for input. Provides grid with auto step, view cube, axes/trihedron, background controls, and split views.
  - Manipulator: Integrates `AIS_Manipulator` to interactively transform a selected body; emits a signal to commit transforms as a `MoveFeature`.
  - Sketch Rendering: Displays sketches as colored wire compounds and supports an edit overlay mode (Topmost Z‑layer, no depth test) for 2D editing contexts.
  - Decoupling: Rendering and input are independent of model logic and UI commands. The viewer consumes shapes from the model and creates AIS objects for display. Local AIS transforms used for layout are not persisted back to the model.

- UI
  - MainWindow/TabPage: Application shell and per‑document view. Synchronizes viewer contents from the `Document` after model changes.
  - Panels: `FeatureHistoryPanel` (ordered features/timeline), `DocumentTreePanel` (items overview and selection).
  - Commands: Command pattern to modify the `Document`. Commands include `CreateBoxCommand`, `CreateCylinderCommand`, `CreateExtrudeCommand`, and `MoveCommand`. Dialogs collect parameters.
  - Entry Point: `src/main.cpp` builds the Qt app and launches the main window (binary name: `cad-app`).

- Sketch (`src/sketch`)
  - Active module: basic sketch data structures, storage, serialization, and viewer integration. Used as a profile source for `ExtrudeFeature` by id.
  - Future: constraints solver and dimensioning can be expanded behind the same interfaces.

## Fusion 360 Concept Mapping

- Browser & Timeline → Model/Document
  - `Document` is an ordered feature list analogous to a timeline. Each `Feature` mirrors a Fusion feature node with parameters and a resulting body/shape.
- Parameters → Feature Param Map
  - `Feature::ParamMap` plays the role of per-feature parameters. A future global parameters table can complement this, similar to Fusion’s Change Parameters.
- Commands → UI Commands
  - UI commands create and edit features, similar to Fusion commands (e.g., Box, Cylinder, Extrude, Move). Dialogs collect inputs; manipulator‑driven edits become timeline features.
- Bodies & Display → Viewer/AIS
  - Resulting `TopoDS_Shape` instances are turned into AIS interactive objects for display. Local transforms used in the sample layout correspond to temporary positioning, not persistent model transforms.
- Combine/Boolean → KernelAPI::fuse
  - `KernelAPI::fuse` maps to Fusion’s Combine → Join.
- Sketch Environment → Sketch Module
  - A sketch workspace with constraints/dimensions provides profiles for downstream solid features (extrude, revolve), akin to Fusion’s sketch‑driven workflow. The viewer supports an edit overlay for sketch editing.

## Data Flow

- UI action → Command executes → `Document::addItem()`/`addFeature()`
- Sketch usage → `Document::addSketch()` registers a shared sketch by id; features reference by id (extrude/move source).
- `Document::recompute()` → `Feature::execute()` uses `KernelAPI`/OCCT to produce shapes; downstream dependencies resolved by id.
- Viewer sync → Build AIS objects from `Document` shapes → Render; apply transient local transforms for UI layout only.

## Extending the System

- Add a new feature
  - Define needed parameter keys (extend `Feature::ParamKey` if common).
  - Implement a `Feature` subclass with `execute()` calling `KernelAPI` and storing a `TopoDS_Shape`.
  - Add a UI command and dialog to create/edit the feature.
  - Add unit tests in `tests/` to validate model execution and command integration.

## References

- Open CASCADE (OCCT)
  - Overview and documentation: https://dev.opencascade.org/doc/overview/html/index.html
  - Tutorial and samples: https://dev.opencascade.org/resources
- Qt 6
  - Qt 6 documentation: https://doc.qt.io/qt-6/
  - QOpenGLWidget: https://doc.qt.io/qt-6/qopenglwidget.html
- Fusion 360
  - Fusion 360 Help: https://help.autodesk.com/view/fusion360/ENU/
  - Learning paths and tutorials: https://www.autodesk.com/learn/paths/learn-fusion-360

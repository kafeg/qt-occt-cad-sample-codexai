# Architecture Overview

This repository implements a layered, modular skeleton for a parametric CAD application using Open CASCADE Technology (OCCT) for geometry and Qt6 for UI and rendering. The design mirrors modern CAD patterns (e.g., Fusion 360) with a clean separation between kernel, model, viewer, and UI command layers.

## Modules

- Core (KernelAPI)
  - Purpose: Thin, testable wrappers over OCCT primitives and booleans to isolate kernel usage from the rest of the app.
  - Key APIs: `makeBox(dx, dy, dz)`, `makeCylinder(radius, height)`, `fuse(a, b)` returning `TopoDS_Shape`.
  - Notes: Encapsulates `BRepPrimAPI_*` and `BRepAlgoAPI_*` usage. No Qt dependencies.

- Model
  - Feature: Base type with a typed parameter map and a resulting `TopoDS_Shape`. Parameters include `Dx/Dy/Dz`, `Radius/Height` for built-in primitives.
  - Document: Ordered list of features with `recompute()`; iterates features, calls `Feature::execute()`, and holds results. Serves as the source of truth for geometry.
  - Primitives: `BoxFeature`, `CylinderFeature` implement `execute()` by calling `KernelAPI` and storing the resulting shape.

- Viewer
  - `OcctQOpenGLWidgetViewer`: A reusable `QOpenGLWidget` that integrates OCCT viewer/contexts with `AIS_ViewController` for input. Provides grid with auto step, view cube, axes/trihedron, and background controls.
  - Decoupling: Rendering and input are independent of model logic and UI commands. The viewer consumes shapes from the model and creates AIS objects for display. Local AIS transforms used for layout are not persisted back to the model.

- UI
  - MainWindow/TabPage: Application shell and per-document view. Synchronizes viewer contents from the `Document` after model changes.
  - Commands: Command pattern to modify the `Document`. `CreateBoxCommand` and `CreateCylinderCommand` open dialogs, push features, trigger `Document::recompute()`, then the viewer syncs.
  - Entry Point: `src/main.cpp` builds the Qt app and launches the main window (binary name: `cad-app`).

- Planned: Sketch
  - Scope: 2D sketcher with constraints and dimensions (profiles, geometric constraints, driving dimensions), feeding solid features (e.g., extrude, revolve) in the model.
  - Integration: Sketch objects live in a sketch module, produce profile wires/edges consumed by model features. Constraints solver remains encapsulated; model features reference sketch results via stable IDs.

## Fusion 360 Concept Mapping

- Browser & Timeline → Model/Document
  - `Document` is an ordered feature list analogous to a timeline. Each `Feature` mirrors a Fusion feature node with parameters and a resulting body/shape.
- Parameters → Feature Param Map
  - `Feature::ParamMap` plays the role of per-feature parameters. A future global parameters table can complement this, similar to Fusion’s Change Parameters.
- Commands → UI Commands
  - UI commands create and edit features, similar to Fusion commands (e.g., Box, Cylinder). Dialogs collect inputs, commands apply them to the `Document`.
- Bodies & Display → Viewer/AIS
  - Resulting `TopoDS_Shape` instances are turned into AIS interactive objects for display. Local transforms used in the sample layout correspond to temporary positioning, not persistent model transforms.
- Combine/Boolean → KernelAPI::fuse
  - `KernelAPI::fuse` maps to Fusion’s Combine → Join.
- Sketch Environment → Planned Sketch Module
  - A sketch workspace with constraints/dimensions will provide profiles for downstream solid features (extrude, revolve), akin to Fusion’s sketch-driven workflow.

## Data Flow

- UI action → Command executes → `Document::addFeature()`
- `Document::recompute()` → `Feature::execute()` uses `KernelAPI` to produce shapes
- Viewer sync → Build AIS objects from `Document` shapes → Render

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


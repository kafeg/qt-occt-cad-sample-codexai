# Repository Guidelines

## Project Structure & Module Organization
- `occt-qopenglwidget/`: Qt6 + OCCT app sources (`.cpp/.h`) and entrypoint `main.cpp`.
- `tests/`: GoogleTest unit tests and CMake test target.
- `CMakeLists.txt`, `CMakePresets.json`: Top-level build and presets; enables tests via `CTest`.
- `vcpkg/`, `vcpkg.json`: Manifest-based dependencies (`qtbase`, `opencascade`, `gtest`).
- `.clang-format`: Enforced C++ style (OCCT-leaning, Microsoft base).

## Build, Test, and Development Commands
- Configure: `cmake --preset default` (uses Ninja, `build/`, `arm64-osx` triplet by default).
- Build: `cmake --build --preset default` (or `cmake --build build`).
- Run (Unix): `./build/occt-qopenglwidget/occt-qopenglwidget`.
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

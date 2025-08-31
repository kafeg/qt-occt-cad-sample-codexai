# OCCT 3D Viewer with Qt6

This sample demonstrates how to embed an Open CASCADE Technology (OCCT) 3D viewer in a Qt6 `QOpenGLWidget` application.

![sample screenshot](/images/occt-qopenglwidget-sample-wnt.png)

## Building

The project uses [vcpkg](https://github.com/microsoft/vcpkg) and CMake presets to fetch dependencies.
The `vcpkg` submodule is bootstrapped automatically when configuring the
project, so simply running CMake triggers installation of the libraries
listed in `vcpkg.json`.

```bash
git clone --recurse-submodules <repo>
cmake --preset default   # or plain `cmake -S . -B build`
cmake --build --preset default
```

The `default` preset builds for the `arm64-osx` triplet and relies on the manifest `vcpkg.json` to provide `qtbase` and `opencascade`.
Adjust `VCPKG_TARGET_TRIPLET` in `CMakePresets.json` for other platforms.

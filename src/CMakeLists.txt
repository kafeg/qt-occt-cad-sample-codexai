cmake_minimum_required(VERSION 3.20)

# Decompose into subdirectories (independent CMake units)
add_subdirectory(core)
add_subdirectory(doc)
add_subdirectory(model)
add_subdirectory(sketch)
## UI and viewer
add_subdirectory(viewer)
# UI is kept as a subdirectory for sources/resources structure, but executable is defined here at src level
add_subdirectory(ui)

# UI executable: vibecad (placed at src level)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml QuickControls2)

add_executable(vibecad
    main.cpp
    ui/qml.qrc
)

target_link_libraries(vibecad PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::QuickControls2
    Qt6::OpenGL
    ${OpenCASCADE_LIBRARIES}
    viewer
)

target_compile_features(vibecad PRIVATE cxx_std_17)

# Ensure QML plugins are discoverable in dev runs
if(COMMAND qt_import_qml_plugins)
  qt_import_qml_plugins(vibecad)
endif()

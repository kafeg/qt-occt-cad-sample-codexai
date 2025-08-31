cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Subsystems as static libraries

# Core: OCCT wrappers, no Qt dependencies
add_library(core STATIC
    core/KernelAPI.cpp
    core/KernelAPI.h
)
target_link_libraries(core PUBLIC ${OpenCASCADE_LIBRARIES})
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core)

# Model: Document/Feature
add_library(model STATIC
    model/Document.cpp
    model/Document.h
    model/Feature.h
)
target_link_libraries(model PUBLIC core)
target_include_directories(model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/model)

# Sketch: placeholder for sketcher/constraints (header-only for now)
add_library(sketch INTERFACE)
target_link_libraries(sketch INTERFACE core)
target_include_directories(sketch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sketch)

# Viewer: QOpenGLWidget-based OCCT viewer
add_library(viewer STATIC
    viewer/OcctQOpenGLWidgetViewer.cpp
    viewer/OcctQOpenGLWidgetViewer.h
    viewer/OcctGlTools.cpp
    viewer/OcctGlTools.h
    viewer/OcctQtTools.h
)
target_link_libraries(viewer PUBLIC Qt6::Widgets Qt6::OpenGLWidgets ${OpenCASCADE_LIBRARIES})
target_include_directories(viewer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/viewer)

# UI executable (prototype app)
add_executable(cad-app
    main.cpp
    ui/MainWindow.cpp
    ui/MainWindow.h
)
target_include_directories(cad-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui)
target_link_libraries(cad-app PRIVATE viewer model core Qt6::Widgets Qt6::OpenGLWidgets ${OpenCASCADE_LIBRARIES})

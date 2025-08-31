cmake_minimum_required(VERSION 3.20)

project(occt-qopenglwidget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(OpenCASCADE REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    OcctQOpenGLWidgetViewer.cpp
    OcctQOpenGLWidgetViewer.h
    OcctGlTools.cpp
    OcctGlTools.h
    OcctQtTools.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets ${OpenCASCADE_LIBRARIES})


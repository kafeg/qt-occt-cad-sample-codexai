cmake_minimum_required(VERSION 3.20)

project(occt-qopenglwidget-sample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets Qml Quick QuickControls2)
find_package(OpenCASCADE REQUIRED)

add_subdirectory(src)

# Testing setup
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

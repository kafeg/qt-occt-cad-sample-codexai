cmake_minimum_required(VERSION 3.20)

project(occt-qopenglwidget-sample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt Widgets/QML removed; no Qt required at top-level
find_package(OpenCASCADE REQUIRED)

add_subdirectory(src)

# Testing setup
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

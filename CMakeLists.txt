cmake_minimum_required(VERSION 3.20)

project(occt-qopenglwidget-sample LANGUAGES CXX)

find_package(Qt6 COMPONENTS Widgets OpenGLWidgets REQUIRED)
find_package(OpenCASCADE REQUIRED)

add_subdirectory(src)

# Testing setup
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

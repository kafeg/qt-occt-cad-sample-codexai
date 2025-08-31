cmake_minimum_required(VERSION 3.20)

project(occt-qopenglwidget-sample LANGUAGES CXX)

add_subdirectory(occt-qopenglwidget)

# Testing setup
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

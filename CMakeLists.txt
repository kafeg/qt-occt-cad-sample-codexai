cmake_minimum_required(VERSION 3.20)

# Enable vcpkg manifest mode automatically if the submodule exists. This
# allows running plain `cmake` without specifying the toolchain file; vcpkg
# will bootstrap itself and install dependencies listed in `vcpkg.json`.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "" FORCE)
endif()

if(NOT DEFINED VCPKG_MANIFEST_DIR)
  set(VCPKG_MANIFEST_DIR "${CMAKE_SOURCE_DIR}" CACHE STRING "" FORCE)
endif()

if(NOT DEFINED VCPKG_TARGET_TRIPLET AND CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
  set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "" FORCE)
endif()

project(occt-qopenglwidget-sample LANGUAGES CXX)

add_subdirectory(occt-qopenglwidget)


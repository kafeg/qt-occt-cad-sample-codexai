cmake_minimum_required(VERSION 3.20)

find_package(GTest CONFIG REQUIRED)
find_package(OpenCASCADE REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

add_executable(vibecad-tests
  common/sanity_test.cpp
  common/occt_test.cpp
  features/box_feature_test.cpp
  features/cylinder_feature_test.cpp
  features/extrude_feature_test.cpp
  features/move_feature_test.cpp
  features/move_feature_rotation_test.cpp
  features/move_feature_stress_test.cpp
  model/document_timeline_test.cpp
  sketch/sketch_storage_test.cpp
  sketch/sketch_constraints_test.cpp
  sketch/sketch_order_export_test.cpp
  sketch/sketch_spatial_index_test.cpp
  serialization/serialization_test.cpp
  document_initializer_test.cpp
  viewer_integration_test.cpp
)

# Allow tests to include headers within tests/ (e.g. common/test_utils.h)
target_include_directories(vibecad-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(vibecad-tests PRIVATE
  GTest::gtest
  GTest::gtest_main
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  sketch
  model
  doc
  core
  viewer
  ${OpenCASCADE_LIBRARIES}
)

add_test(NAME all_tests COMMAND vibecad-tests)

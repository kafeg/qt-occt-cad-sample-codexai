cmake_minimum_required(VERSION 3.20)

find_package(GTest CONFIG REQUIRED)
find_package(OpenCASCADE REQUIRED)

add_executable(occt-qopenglwidget-tests
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
)

# Allow tests to include headers within tests/ (e.g. common/test_utils.h)
target_include_directories(occt-qopenglwidget-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(occt-qopenglwidget-tests PRIVATE
  GTest::gtest
  GTest::gtest_main
  sketch
  model
  doc
  core
  ${OpenCASCADE_LIBRARIES}
)

add_test(NAME all_tests COMMAND occt-qopenglwidget-tests)

cmake_minimum_required(VERSION 3.20)

find_package(GTest CONFIG REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
find_package(OpenCASCADE REQUIRED)

add_executable(occt-qopenglwidget-tests
  common/sanity_test.cpp
  common/occt_test.cpp
  common/qt_test.cpp
  features/box_feature_test.cpp
  features/cylinder_feature_test.cpp
  ui/command_integration_test.cpp
  ui/document_test.cpp
)

target_link_libraries(occt-qopenglwidget-tests PRIVATE
  GTest::gtest
  GTest::gtest_main
  Qt6::Core
  Qt6::Widgets
  ui
  model
  ${OpenCASCADE_LIBRARIES}
)

add_test(NAME all_tests COMMAND occt-qopenglwidget-tests)

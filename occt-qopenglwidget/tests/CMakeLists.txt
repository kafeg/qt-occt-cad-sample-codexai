cmake_minimum_required(VERSION 3.20)

find_package(GTest CONFIG REQUIRED)

add_executable(occt-qopenglwidget-tests
  sanity_test.cpp
)

target_link_libraries(occt-qopenglwidget-tests PRIVATE GTest::gtest GTest::gtest_main)

add_test(NAME sanity COMMAND occt-qopenglwidget-tests)


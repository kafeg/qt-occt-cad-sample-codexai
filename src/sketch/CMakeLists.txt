find_package(OpenCASCADE REQUIRED)

add_library(sketch STATIC
  Sketch.cpp
  Sketch.h
)
target_link_libraries(sketch PUBLIC ${OpenCASCADE_LIBRARIES} doc)
target_include_directories(sketch PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

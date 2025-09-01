add_library(doc STATIC
  DocumentItem.cpp
  DocumentItem.h
)
target_include_directories(doc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCASCADE_INCLUDE_DIR})
target_link_libraries(doc PUBLIC ${OpenCASCADE_LIBRARIES})

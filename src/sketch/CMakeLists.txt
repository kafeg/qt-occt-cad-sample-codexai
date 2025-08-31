add_library(sketch INTERFACE)
target_link_libraries(sketch INTERFACE core)
target_include_directories(sketch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


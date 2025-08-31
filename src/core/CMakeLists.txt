add_library(core STATIC
    KernelAPI.cpp
    KernelAPI.h
)
target_link_libraries(core PUBLIC ${OpenCASCADE_LIBRARIES})
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


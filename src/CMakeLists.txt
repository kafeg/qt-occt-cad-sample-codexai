cmake_minimum_required(VERSION 3.20)

# Decompose into subdirectories (independent CMake units)
add_subdirectory(core)
add_subdirectory(doc)
add_subdirectory(model)
add_subdirectory(sketch)
add_subdirectory(viewer)
add_subdirectory(ui)

# UI executable (prototype app)
add_executable(cad-app
    main.cpp
)
target_link_libraries(cad-app PRIVATE ui)

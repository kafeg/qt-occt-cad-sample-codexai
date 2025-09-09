cmake_minimum_required(VERSION 3.20)

# Decompose into subdirectories (independent CMake units)
add_subdirectory(core)
add_subdirectory(doc)
add_subdirectory(model)
add_subdirectory(sketch)
## UI and viewer removed; only core libraries are built for now

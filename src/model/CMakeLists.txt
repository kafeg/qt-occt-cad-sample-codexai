add_library(model STATIC
    Feature.cpp
    Feature.h
    Document.cpp
    Document.h
    BoxFeature.cpp
    BoxFeature.h
    CylinderFeature.cpp
    CylinderFeature.h
    ExtrudeFeature.cpp
    ExtrudeFeature.h
)
target_link_libraries(model PUBLIC core sketch doc)
target_include_directories(model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

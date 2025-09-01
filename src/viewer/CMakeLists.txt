add_library(viewer STATIC
    OcctQOpenGLWidgetViewer.cpp
    OcctQOpenGLWidgetViewer.h
    OcctGlTools.cpp
    OcctGlTools.h
    OcctQtTools.h
)
target_link_libraries(viewer PUBLIC Qt6::Widgets Qt6::OpenGLWidgets ${OpenCASCADE_LIBRARIES} sketch)
target_include_directories(viewer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

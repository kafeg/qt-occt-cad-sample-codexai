add_library(viewer STATIC
  OcctQOpenGLWidgetViewer.cpp
  CustomManipulator.cpp
    OcctQOpenGLWidgetViewer.h
    OcctGlTools.cpp
    OcctGlTools.h
    OcctQtTools.h
    FiniteGrid.cpp
    FiniteGrid.h
    SceneGizmos.cpp
    SceneGizmos.h
    viewer.qrc
)
target_link_libraries(viewer PUBLIC Qt6::Widgets Qt6::OpenGLWidgets Qt6::Qml Qt6::Quick Qt6::QuickControls2 ${OpenCASCADE_LIBRARIES} sketch model)
target_include_directories(viewer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

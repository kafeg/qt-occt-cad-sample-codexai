# OpenCascade viewer library
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick OpenGL OpenGLWidgets)

add_library(viewer STATIC
  OcctQmlViewer.cpp
  OcctQmlViewer.h
  OcctGlTools.h
  FiniteGrid.cpp
  FiniteGrid.h
  SceneGizmos.cpp
  SceneGizmos.h
  OcctQtTools.h
  ViewerTypes.cpp
  ViewerTypes.h
  viewer.qrc
)

target_link_libraries(viewer PRIVATE
  ${OpenCASCADE_LIBRARIES}
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  Qt6::OpenGL
  Qt6::OpenGLWidgets
  core
)

target_compile_features(viewer PRIVATE cxx_std_17)


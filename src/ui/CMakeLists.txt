cmake_minimum_required(VERSION 3.20)

# UI executable: cad-app (VibeCAD)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml QuickControls2)

add_executable(cad-app
    ../main.cpp
    qml.qrc
)

target_link_libraries(cad-app PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::QuickControls2
    ${OpenCASCADE_LIBRARIES}
)

target_compile_features(cad-app PRIVATE cxx_std_17)

# Ensure QML plugins are discoverable in dev runs
if(COMMAND qt_import_qml_plugins)
  qt_import_qml_plugins(cad-app)
endif()

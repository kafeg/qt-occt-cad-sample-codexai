add_library(ui STATIC
    MainWindow.cpp
    MainWindow.h
    TabPage.cpp
    TabPage.h
    DocumentTreePanel.cpp
    DocumentTreePanel.h
    FeatureHistoryPanel.cpp
    FeatureHistoryPanel.h
    StatsPanel.cpp
    StatsPanel.h
    AppSettings.h
    command/AbstractCommand.h
    command/CreateBoxCommand.cpp
    command/CreateBoxCommand.h
    command/CreateCylinderCommand.cpp
    command/CreateCylinderCommand.h
    command/CreateExtrudeCommand.cpp
    command/CreateExtrudeCommand.h
    command/CreateSketchCommand.cpp
    command/CreateSketchCommand.h
    command/MoveCommand.cpp
    command/MoveCommand.h
    dialog/CreateBoxDialog.cpp
    dialog/CreateBoxDialog.h
    dialog/CreateCylinderDialog.cpp
    dialog/CreateCylinderDialog.h
    dialog/CreateExtrudeDialog.cpp
    dialog/CreateExtrudeDialog.h
    dialog/CreateSketchDialog.cpp
    dialog/CreateSketchDialog.h
    dialog/SettingsDialog.cpp
    dialog/SettingsDialog.h
)
target_link_libraries(ui PUBLIC viewer model sketch Qt6::Widgets Qt6::OpenGLWidgets Qt6::Qml Qt6::Quick Qt6::QuickControls2)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ui STATIC
    MainWindow.cpp
    MainWindow.h
    TabPage.cpp
    TabPage.h
    FeatureHistoryPanel.cpp
    FeatureHistoryPanel.h
    command/AbstractCommand.h
    command/CreateBoxCommand.cpp
    command/CreateBoxCommand.h
    command/CreateCylinderCommand.cpp
    command/CreateCylinderCommand.h
    command/CreateExtrudeCommand.cpp
    command/CreateExtrudeCommand.h
    dialog/CreateBoxDialog.cpp
    dialog/CreateBoxDialog.h
    dialog/CreateCylinderDialog.cpp
    dialog/CreateCylinderDialog.h
    dialog/CreateExtrudeDialog.cpp
    dialog/CreateExtrudeDialog.h
)
target_link_libraries(ui PUBLIC viewer model sketch Qt6::Widgets Qt6::OpenGLWidgets)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

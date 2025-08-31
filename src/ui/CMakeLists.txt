add_library(ui STATIC
    MainWindow.cpp
    MainWindow.h
    TabPage.cpp
    TabPage.h
    AbstractCommand.h
    CreateBoxDialog.cpp
    CreateBoxDialog.h
    CreateBoxCommand.cpp
    CreateBoxCommand.h
)
target_link_libraries(ui PUBLIC viewer model Qt6::Widgets Qt6::OpenGLWidgets)
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

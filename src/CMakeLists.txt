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
    main_qml.cpp
)
target_link_libraries(cad-app PRIVATE ui)

# Link required QML plugins when using vcpkg's static QML packages
find_package(Qt6Qml REQUIRED)
# The QML plugin packages are pulled in via Qt6Qml; the targets exist when using
# vcpkg's Qt builds (static QML plugins). Linking them ensures plugins are embedded.
foreach(qmlplug
        qtquick2plugin
        qquicklayoutsplugin
        quickwindow
        qtquicktemplates2plugin
        qtquickcontrols2plugin
        qtquickcontrols2fusionstyleplugin)
  if (TARGET Qt6::${qmlplug})
    target_link_libraries(cad-app PRIVATE Qt6::${qmlplug})
  endif()
endforeach()

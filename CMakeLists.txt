cmake_minimum_required(VERSION 3.16)
project(qt-floppy VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/floppydiskwidget.cpp
    src/floppydiskwidget.h
    src/fdccontrollerwidget.cpp
    src/fdccontrollerwidget.h
)

add_executable(qt-floppy
    ${PROJECT_SOURCES}
)

target_link_libraries(qt-floppy PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(qt-floppy PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
) 
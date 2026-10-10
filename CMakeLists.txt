cmake_minimum_required(VERSION 3.16)

project(TableHistory VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

set(PROJECT_SOURCES
    main.cpp
    historyitem.h
    historyitem.cpp
    historytablemodel.h
    historytablemodel.cpp
    ganttwidget.h
    ganttwidget.cpp
    itemeditdialog.h
    itemeditdialog.cpp
    mainwindow.h
    mainwindow.cpp
    notestextedit.h
    aboutdialog.h
    aboutdialog.cpp
)

# Set the icon file name for the Info.plist
set(MACOSX_BUNDLE_ICON_FILE icon.icns)

# Define the absolute path to your .icns file
set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.icns")

# Tell CMake to install the icon into the bundle's Resources folder (fixed variable name)
set_source_files_properties(${app_icon_macos} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

if(APPLE)
    # Added ${app_icon_macos} to the executable target sources
    add_executable(TableHistory MACOSX_BUNDLE
        ${PROJECT_SOURCES}
        ${app_icon_macos}
    )
    # Explicitly set the bundle icon target property
    set_target_properties(TableHistory PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.tablehistory.app"
        MACOSX_BUNDLE_BUNDLE_NAME "TableHistory"
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2026 Massimo Nardello"
    )

    ##[[
    # Automatically bundle Qt libraries using macdeployqt
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

    add_custom_command(TARGET TableHistory POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.png"
            "${CMAKE_BINARY_DIR}/TableHistory.app/Contents/Resources/icon.png"
        COMMAND ${MACDEPLOYQT_EXECUTABLE}
        ARGS "${CMAKE_BINARY_DIR}/TableHistory.app" -always-overwrite
        COMMENT "Copying icon and running macdeployqt..."
    )
    #]]

    add_custom_command(TARGET TableHistory POST_BUILD
    COMMAND ${MACDEPLOYQT_EXECUTABLE}
    ARGS "$<TARGET_BUNDLE_DIR:TableHistory>" -always-overwrite

    COMMENT "Running macdeployqt to bundle dependencies..."
)
else()
    add_executable(TableHistory ${PROJECT_SOURCES})
endif()

target_link_libraries(TableHistory PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)



cmake_minimum_required(VERSION 3.16)

project(TableHistory VERSION 1.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Svg Gui)

# Define all project sources including application files and embedded Hunspell sources
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
    resources.qrc
    # Embedded Hunspell source files
    hunspell_src/affentry.cxx
    hunspell_src/affentry.hxx
    hunspell_src/affixmgr.cxx
    hunspell_src/affixmgr.hxx
    hunspell_src/atypes.hxx
    hunspell_src/baseaffix.hxx
    hunspell_src/csutil.cxx
    hunspell_src/csutil.hxx
    hunspell_src/filemgr.cxx
    hunspell_src/filemgr.hxx
    hunspell_src/hashmgr.cxx
    hunspell_src/hashmgr.hxx
    hunspell_src/htypes.hxx
    hunspell_src/hunspell.cxx
    hunspell_src/hunspell.h
    hunspell_src/hunspell.hxx
    hunspell_src/hunspelltrace.cxx
    hunspell_src/hunspelltrace.hxx
    hunspell_src/hunversion.h
    hunspell_src/hunvisapi.h
    hunspell_src/hunzip.cxx
    hunspell_src/hunzip.hxx
    hunspell_src/langnum.hxx
    hunspell_src/phonet.cxx
    hunspell_src/phonet.hxx
    hunspell_src/replist.cxx
    hunspell_src/replist.hxx
    hunspell_src/suggestmgr.cxx
    hunspell_src/suggestmgr.hxx
    hunspell_src/utf_info.hxx
    hunspell_src/w_char.hxx
)

# Set the icon file name for the Info.plist
set(MACOSX_BUNDLE_ICON_FILE icon.icns)

# Define the absolute path to your .icns file
set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon.icns")

# Tell CMake to install the icon into the bundle's Resources folder
set_source_files_properties(${app_icon_macos} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)

if(APPLE)
    add_executable(TableHistory MACOSX_BUNDLE
        ${PROJECT_SOURCES}
        ${app_icon_macos}
    )
    # Explicitly set the bundle icon target properties
    set_target_properties(TableHistory PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.tablehistory.app"
        MACOSX_BUNDLE_BUNDLE_NAME "TableHistory"
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2026 Massimo Nardello"
    )
else()
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon.rc")
    add_executable(TableHistory WIN32 ${PROJECT_SOURCES} ${APP_ICON_RESOURCE_WINDOWS})
endif()

# Link Qt libraries and include local Hunspell headers AFTER target creation
target_link_libraries(TableHistory PRIVATE Qt6::Widgets Qt6::Core Qt6::Svg Qt6::Gui)
target_include_directories(TableHistory PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/hunspell_src)
target_compile_definitions(TableHistory PRIVATE HUNSPELL_STATIC)

# macOS Post-Build Deployment (macdeployqt)
if(APPLE)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

    add_custom_command(TARGET TableHistory POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon.png"
            "$<TARGET_BUNDLE_DIR:TableHistory>/Contents/Resources/icon.png"
        COMMAND ${MACDEPLOYQT_EXECUTABLE}
        ARGS "$<TARGET_BUNDLE_DIR:TableHistory>" -always-overwrite
        COMMENT "Running macdeployqt to bundle dependencies and resources..."
    )
endif()

#
# Copyright (c) 2020-2023, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

cmake_minimum_required(VERSION "3.16")

project(ImageMosaicWall)

option(ENABLE_DPLUGIN "Build digiKam plugin only of Image Mosaic Wall tool (default=OFF)" OFF)
option(BUILD_WITH_QT6 "Build with Qt6, else Qt5"                                          OFF)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

if (ENABLE_DPLUGIN)
    include(MacroInstall)
endif()

if(NOT ENABLE_DPLUGIN AND APPLE)
  set(MACOSX_BUNDLE_ICON_FILE photils.icns)
  set(OSX_APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/assets/imw-logo.icns)
  set_source_files_properties(${OSX_APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

include(FeatureSummary)

if(MSVC)
    add_compile_options(/utf-8)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
endif()

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(BUILD_WITH_QT6)

    find_package(Qt6 "6.5.0" REQUIRED
                 NO_MODULE COMPONENTS
                 Core
                 Widgets
                 Gui
                 Concurrent
    )

    set(QT_VERSION_MAJOR 6)

else()

    find_package(Qt5 "5.6.0" REQUIRED
                 NO_MODULE COMPONENTS
                 Core
                 Widgets
                 Gui
                 Concurrent
    )

    set(QT_VERSION_MAJOR 5)

endif()

find_package(Threads)

find_package(exiv2lib REQUIRED CONFIG NAMES exiv2)

set_package_properties(exiv2lib PROPERTIES
                       URL         "https://www.exiv2.org"
                       TYPE        RECOMMENDED
                       PURPOSE     "Library to manage image metadata"
)

include(i18nUtils)

include_directories($<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt${QT_VERSION_MAJOR}::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}
)

set(imagemosaicwall_core_SRCS ${CMAKE_SOURCE_DIR}/mainwindow.cpp
                              ${CMAKE_SOURCE_DIR}/imageviewer.cpp
                              ${CMAKE_SOURCE_DIR}/imageprocessing.cpp
                              ${CMAKE_SOURCE_DIR}/progressbutton.cpp
                              ${CMAKE_SOURCE_DIR}/asyncdirectoryscanner.cpp
                              ${i18n_QRC_SRCS}
                              ${i18n_QM}
)

if(Qt6_FOUND)

    qt6_wrap_ui(imagemosaicwall_core_SRCS ${CMAKE_SOURCE_DIR}/mainwindow.ui
                                          ${CMAKE_SOURCE_DIR}/imageviewer.ui
    )

    qt6_add_resources(imagemosaicwall_core_QRC_SRCS ${CMAKE_SOURCE_DIR}/assets.qrc)

else()

    qt5_wrap_ui(imagemosaicwall_core_SRCS ${CMAKE_SOURCE_DIR}/mainwindow.ui
                                          ${CMAKE_SOURCE_DIR}/imageviewer.ui
    )

    qt5_add_resources(imagemosaicwall_core_QRC_SRCS ${CMAKE_SOURCE_DIR}/assets.qrc)

endif()

set(libimagemosaicwall_core_SRCS
    ${imagemosaicwall_core_SRCS}
    ${imagemosaicwall_core_QRC_SRCS}
)

add_library(imagemosaicwallcore STATIC ${libimagemosaicwall_core_SRCS})

target_link_libraries(imagemosaicwallcore
                      PUBLIC
                      Exiv2::exiv2lib
)

if(NOT MSVC)
    target_compile_options(imagemosaicwallcore PRIVATE -fPIC)
endif()

if(NOT ENABLE_DPLUGIN)

    if(APPLE)
        add_executable(ImageMosaicWall MACOSX_BUNDLE ${OSX_APP_ICON} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
    else()
        add_executable(ImageMosaicWall ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
    endif()

    target_link_libraries(ImageMosaicWall

                          imagemosaicwallcore

                          Qt${QT_VERSION_MAJOR}::Core
                          Qt${QT_VERSION_MAJOR}::Widgets
                          Qt${QT_VERSION_MAJOR}::Gui
                          Qt${QT_VERSION_MAJOR}::Concurrent

                          ${CMAKE_THREAD_LIBS_INIT}
    )

    if(APPLE)
        set(APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")
        set(DIRS "\${CMAKE_INSTALL_PREFIX}/share")

        install(TARGETS ${PROJECT_NAME}
                BUNDLE DESTINATION . COMPONENT Runtime
                RUNTIME DESTINATION bin COMPONENT Runtime)

        install(CODE "
            include(BundleUtilities)
            fixup_bundle(\"${APPS}\"  \"\"  \"${DIRS}\")
            " COMPONENT Runtime)
    else()
        install(TARGETS ImageMosaicWall RUNTIME DESTINATION bin)
    endif()

else()

    add_subdirectory(dplugin)
    MACRO_ADD_UNINSTALL_TARGET()

endif()

#
# Copyright (c) 2020, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

project(ImageMosaicWall)

cmake_minimum_required(VERSION "3.1.0")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
if (ENABLE_DPLUGIN)
    include(MacroInstall)
endif()

include(FeatureSummary)

set(CMAKE_CXX_STANDARD 14)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(ENABLE_DPLUGIN "Build digiKam plugin version of Image Mosaic Wall tool (default=OFF)" OFF)

find_package(Qt5 "5.6.0" REQUIRED
             NO_MODULE COMPONENTS
             Core
             Widgets
             Gui
             Concurrent
)

find_package(Threads)

find_package(exiv2 REQUIRED)

set_package_properties("exiv2"     PROPERTIES
                       DESCRIPTION "Required to build digiKam"
                       URL         "https://www.exiv2.org"
                       TYPE        RECOMMENDED
                       PURPOSE     "Library to manage image metadata"
)

include_directories($<TARGET_PROPERTY:exiv2lib,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    ${CMAKE_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}
)

set(imagemosaicwall_core_SRCS ${CMAKE_SOURCE_DIR}/mainwindow.cpp
                              ${CMAKE_SOURCE_DIR}/imageviewer.cpp
                              ${CMAKE_SOURCE_DIR}/imageprocessing.cpp
                              ${CMAKE_SOURCE_DIR}/progressbutton.cpp
)

qt5_wrap_ui(imagemosaicwall_core_SRCS ${CMAKE_SOURCE_DIR}/mainwindow.ui
                                      ${CMAKE_SOURCE_DIR}/imageviewer.ui
)

qt5_add_resources(imagemosaicwall_core_QRC_SRCS ${CMAKE_SOURCE_DIR}/assets.qrc)

set(libimagemosaicwall_core_SRCS    
    ${imagemosaicwall_core_SRCS}
    ${imagemosaicwall_core_QRC_SRCS}
)

add_library(imagemosaicwallcore STATIC ${libimagemosaicwall_core_SRCS})

if(NOT MSVC)
    target_compile_options(imagemosaicwallcore PRIVATE -fPIC)
endif()

add_executable(ImageMosaicWall ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_link_libraries(ImageMosaicWall

                      imagemosaicwallcore

                      Qt5::Core
                      Qt5::Widgets
                      Qt5::Gui
                      Qt5::Concurrent

                      exiv2lib
                      ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS ImageMosaicWall RUNTIME DESTINATION bin)

if (ENABLE_DPLUGIN)
    add_subdirectory(dplugin)
    MACRO_ADD_UNINSTALL_TARGET()
endif()

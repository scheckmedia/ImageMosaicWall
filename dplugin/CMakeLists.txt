#
# Copyright (c) 2020, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(CMAKE_AUTOMOC ON)

include(FeatureSummary)

find_package(Qt5 "5.6.0" REQUIRED
             NO_MODULE COMPONENTS
             Core
             Widgets
             Gui
             Concurrent
)

find_package(exiv2 REQUIRED)

set_package_properties("exiv2"     PROPERTIES
                       DESCRIPTION "Required to build digiKam"
                       URL         "https://www.exiv2.org"
                       TYPE        RECOMMENDED
                       PURPOSE     "Library to manage image metadata"
)

find_package(DigikamCore
             CONFIG REQUIRED)

set_package_properties(DigikamCore PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam core library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamcore,INTERFACE_INCLUDE_DIRECTORIES>/digikam
                    $<TARGET_PROPERTY:exiv2lib,INTERFACE_INCLUDE_DIRECTORIES>
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}
)

set(imagemosaicwall_generic_SRCS ${CMAKE_SOURCE_DIR}/mainwindow.cpp
                                 ${CMAKE_SOURCE_DIR}/imageviewer.cpp
                                 ${CMAKE_SOURCE_DIR}/imageprocessing.cpp
                                 ${CMAKE_CURRENT_SOURCE_DIR}/imagemosaicwallgenericplugin.cpp
)

qt5_wrap_ui(imagemosaicwall_generic_SRCS ${CMAKE_SOURCE_DIR}/mainwindow.ui
                                         ${CMAKE_SOURCE_DIR}/imageviewer.ui
)

qt5_add_resources(imagemosaicwall_generic_QRC_SRCS ${CMAKE_SOURCE_DIR}/assets.qrc)

add_library(Generic_ImageMosaicWall_Plugin
            MODULE ${imagemosaicwall_generic_SRCS}
                   ${imagemosaicwall_generic_QRC_SRCS}
)

set_target_properties(Generic_ImageMosaicWall_Plugin PROPERTIES PREFIX "")

target_link_libraries(Generic_ImageMosaicWall_Plugin
                      Digikam::digikamcore

                      Qt5::Core
                      Qt5::Widgets
                      Qt5::Gui
                      Qt5::Concurrent

                      exiv2lib
)

MACRO_ADD_PLUGIN_INSTALL_TARGET(Generic_ImageMosaicWall_Plugin generic)

# ---

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

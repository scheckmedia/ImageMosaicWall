#
# Copyright (c) 2020-2023, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_package(DigikamCore
             CONFIG REQUIRED)

set_package_properties(DigikamCore PROPERTIES
                       URL "http://www.digikam.org"
                       DESCRIPTION "digiKam core library"
)

include_directories($<TARGET_PROPERTY:Digikam::digikamcore,INTERFACE_INCLUDE_DIRECTORIES>/digikam
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}
)

add_library(Generic_ImageMosaicWall_Plugin
            MODULE ${CMAKE_CURRENT_SOURCE_DIR}/imagemosaicwallgenericplugin.cpp
)

set_target_properties(Generic_ImageMosaicWall_Plugin PROPERTIES PREFIX "")

target_link_libraries(Generic_ImageMosaicWall_Plugin

                      imagemosaicwallcore

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

#
# Copyright (c) 2019-2020, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(GNUInstallDirs)
include(${CMAKE_INSTALL_FULL_LIBDIR}/cmake/DigikamPlugin/DigikamPluginConfig.cmake)

# MACRO_ADD_PLUGIN_INSTALL_TARGET
# Install plugin files on the system processed with "make install" or "make install/fast".
# TARGET_NAME is the name of the plugin target to install (aka "Generic_HelloWorld_Plugin").
# TARGET_TYPE is the plugin type. Possible value: "generic", "editor", "rawimport", or "bqm".
#
macro(MACRO_ADD_PLUGIN_INSTALL_TARGET TARGET_NAME TARGET_TYPE)

    if    (${TARGET_TYPE} STREQUAL "generic")

        set (_pluginInstallPath ${DPluginGenericInstallPath})

    elseif(${TARGET_TYPE} STREQUAL "editor")

        set (_pluginInstallPath ${DPluginEditorInstallPath})

    elseif(${TARGET_TYPE} STREQUAL "rawimport")

        set (_pluginInstallPath ${DPluginRawImportInstallPath})

    elseif(${TARGET_TYPE} STREQUAL "bqm")

        set (_pluginInstallPath ${DPluginBqmInstallPath})

    else()

        message(FATAL "Unrecognized plugin type ${TARGET_TYPE} for ${TARGET_NAME}")

    endif()

    install(TARGETS ${TARGET_NAME}
            DESTINATION ${_pluginInstallPath}/)
endmacro()


# MACRO_ADD_UNINSTALL_TARGET
# Remove installed all files from the system processed with "make install" or "make install/fast".
# This macro add a target named "make uninstall" to your project.
#
macro(MACRO_ADD_UNINSTALL_TARGET)

    configure_file("${CMAKE_SOURCE_DIR}/cmake/templates/target.uninstall.cmake.in_cmake"
                   "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
                    COPYONLY
    )

    add_custom_target(uninstall
                      "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
    )

endmacro()

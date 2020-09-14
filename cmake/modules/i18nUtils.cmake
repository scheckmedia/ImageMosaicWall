#
# Copyright (c) 2020 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# --- Translations rules (i18n)

find_package(Qt5 "5.6.0" REQUIRED
             NO_MODULE COMPONENTS
             Core
             LinguistTools              # for i18n
)

include(${CMAKE_CURRENT_SOURCE_DIR}/i18n/i18n_list.cmake)

set_source_files_properties(${translation_files} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/i18n)

foreach(file ${translation_files})

    if(NOT EXISTS "${file}")

       message(STATUS "Translation file ${file} do not exists yet.")
       message(STATUS "It will be created in ${CMAKE_CURRENT_SOURCE_DIR}/i18n")
       message(STATUS "Don't forget to add this new file on git repository.")

       qt5_create_translation(missing_i18n_QM
                              ${CMAKE_CURRENT_SOURCE_DIR}
                              ${file}
       )

    endif()

endforeach()

qt5_add_translation(i18n_QM
                    ${translation_files}
)

foreach(file ${i18n_QM})

    get_filename_component(directory ${file} DIRECTORY)
    get_filename_component(basename  ${file} NAME)

    set(QM_XML "${QM_XML}<file>${directory}/${basename}</file>")

endforeach()

configure_file(${CMAKE_SOURCE_DIR}/cmake/templates/i18n.qrc.in_cmake ${CMAKE_CURRENT_BINARY_DIR}/i18n.qrc)

qt5_add_resources(i18n_QRC_SRCS
                  ${CMAKE_CURRENT_BINARY_DIR}/i18n.qrc
)

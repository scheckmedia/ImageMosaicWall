#
# Copyright (c) 2020 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# --- List of translations files to process

# Append new file on list here, cmake will create missing files automatically.
# Dont forget to add new created file on git repository.
# Use ISO639-1 2 letter language codes as file name.
# https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes

set(translation_files
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/de.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/fr.ts
)

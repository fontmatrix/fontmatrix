# SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
FindHyphen
----------

Finds libhyphen, the hyphenation library of the hunspell project.

It installs neither a CMake package nor a pkg-config file and carries no
version macro, so this looks for the header and the library and reports no
version. The API Fontmatrix uses has not changed since 2.4 (2008).

Imported targets
^^^^^^^^^^^^^^^^

Hyphen::Hyphen

Result variables
^^^^^^^^^^^^^^^^

Hyphen_FOUND, Hyphen_INCLUDE_DIRS, Hyphen_LIBRARIES

Cache variables
^^^^^^^^^^^^^^^

Hyphen_INCLUDE_DIR, Hyphen_LIBRARY
#]=======================================================================]

find_path(Hyphen_INCLUDE_DIR
    NAMES hyphen.h
    DOC "Directory holding hyphen.h of libhyphen")

find_library(Hyphen_LIBRARY
    NAMES hyphen libhyphen
    DOC "The libhyphen library")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Hyphen
    REQUIRED_VARS Hyphen_LIBRARY Hyphen_INCLUDE_DIR)

if(Hyphen_FOUND)
    set(Hyphen_INCLUDE_DIRS "${Hyphen_INCLUDE_DIR}")
    set(Hyphen_LIBRARIES "${Hyphen_LIBRARY}")
    if(NOT TARGET Hyphen::Hyphen)
        add_library(Hyphen::Hyphen UNKNOWN IMPORTED)
        set_target_properties(Hyphen::Hyphen PROPERTIES
            IMPORTED_LOCATION "${Hyphen_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Hyphen_INCLUDE_DIR}")
    endif()
endif()

mark_as_advanced(Hyphen_INCLUDE_DIR Hyphen_LIBRARY)

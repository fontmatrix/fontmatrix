# SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
# SPDX-License-Identifier: BSD-3-Clause

#[=======================================================================[.rst:
FindHarfBuzz
------------

Finds the HarfBuzz text shaping library.

HarfBuzz installs a CMake package of its own, but it carries no version and
not every build installs it, so this looks for the header and the library,
with pkg-config as a hint.

Fontmatrix hands HarfBuzz a FreeType face, which needs ``hb-ft.h``: a HarfBuzz
built without FreeType is reported as not found.

Imported targets
^^^^^^^^^^^^^^^^

``HarfBuzz::HarfBuzz``

Result variables
^^^^^^^^^^^^^^^^

``HarfBuzz_FOUND``, ``HarfBuzz_VERSION``, ``HarfBuzz_INCLUDE_DIRS``,
``HarfBuzz_LIBRARIES``
#]=======================================================================]

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(PC_HARFBUZZ QUIET harfbuzz)
endif()

find_path(HarfBuzz_INCLUDE_DIR
    NAMES hb.h
    HINTS ${PC_HARFBUZZ_INCLUDEDIR} ${PC_HARFBUZZ_INCLUDE_DIRS}
    PATH_SUFFIXES harfbuzz
)

find_library(HarfBuzz_LIBRARY
    NAMES harfbuzz
    HINTS ${PC_HARFBUZZ_LIBDIR} ${PC_HARFBUZZ_LIBRARY_DIRS}
)

# The FreeType integration is part of the library, its header tells if it is in
find_file(HarfBuzz_FT_HEADER
    NAMES hb-ft.h
    HINTS ${HarfBuzz_INCLUDE_DIR}
    NO_DEFAULT_PATH
)

if(HarfBuzz_INCLUDE_DIR AND EXISTS "${HarfBuzz_INCLUDE_DIR}/hb-version.h")
    file(STRINGS "${HarfBuzz_INCLUDE_DIR}/hb-version.h" _harfbuzz_version_line
         REGEX "^#define[ \t]+HB_VERSION_STRING[ \t]+\"[^\"]+\"")
    string(REGEX REPLACE ".*\"([^\"]+)\".*" "\\1" HarfBuzz_VERSION "${_harfbuzz_version_line}")
    unset(_harfbuzz_version_line)
endif()

# Only said when that is what is missing, not when HarfBuzz is not there at all
set(_harfbuzz_failure_reason)
if(HarfBuzz_LIBRARY AND HarfBuzz_INCLUDE_DIR AND NOT HarfBuzz_FT_HEADER)
    set(_harfbuzz_failure_reason "this HarfBuzz was built without FreeType support, hb-ft.h is missing")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HarfBuzz
    REQUIRED_VARS HarfBuzz_LIBRARY HarfBuzz_INCLUDE_DIR HarfBuzz_FT_HEADER
    VERSION_VAR HarfBuzz_VERSION
    REASON_FAILURE_MESSAGE "${_harfbuzz_failure_reason}"
)
unset(_harfbuzz_failure_reason)

if(HarfBuzz_FOUND)
    set(HarfBuzz_INCLUDE_DIRS "${HarfBuzz_INCLUDE_DIR}")
    set(HarfBuzz_LIBRARIES "${HarfBuzz_LIBRARY}")
    if(NOT TARGET HarfBuzz::HarfBuzz)
        add_library(HarfBuzz::HarfBuzz UNKNOWN IMPORTED)
        set_target_properties(HarfBuzz::HarfBuzz PROPERTIES
            IMPORTED_LOCATION "${HarfBuzz_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${HarfBuzz_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(HarfBuzz_INCLUDE_DIR HarfBuzz_LIBRARY HarfBuzz_FT_HEADER)

include(FeatureSummary)
set_package_properties(HarfBuzz PROPERTIES
    DESCRIPTION "Text shaping library"
    URL "https://harfbuzz.github.io/"
)

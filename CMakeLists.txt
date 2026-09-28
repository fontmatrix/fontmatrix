# Fontmatrix ./CMakeLists.txt v1

CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(fontmatrix)

if(POLICY CMP0177)
  cmake_policy(SET CMP0177 NEW)
endif()

SET(VERSION_MAJOR "0" CACHE STRING "")
SET(VERSION_MINOR "11" CACHE STRING "")
SET(VERSION_PATCH "0" CACHE STRING "")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
ADD_DEFINITIONS(-DFONTMATRIX_VERSION_MAJOR=${VERSION_MAJOR} -DFONTMATRIX_VERSION_MINOR=${VERSION_MINOR} -DFONTMATRIX_VERSION_PATCH=${VERSION_PATCH})
MESSAGE(STATUS "Building Fontmatrix ${VERSION}")


IF(UNIX AND NOT APPLE)
	SET (CMAKE_INSTALL_PREFIX /usr/local CACHE PATH "Installation path")
ENDIF(UNIX AND NOT APPLE)

IF(APPLE)
# 	SET (CMAKE_INSTALL_PREFIX /Applications CACHE PATH "Installation path")
# 	SET (FONTMATRIX_BIN bin "")
	set( CMAKE_OSX_ARCHITECTURES i386 )
ENDIF(APPLE)

IF(WIN32)
	# Only a default; Craft passes -DCMAKE_INSTALL_PREFIX=<CraftRoot>.
	# FONTMATRIX_BIN is not set to "." here: the KDE layout puts the
	# executable in ${KDE_INSTALL_BINDIR} ("bin") on Windows too, with the
	# data files in "bin/data" beside it.
	SET (CMAKE_INSTALL_PREFIX c:/fontmatrix CACHE PATH "Installation path")
ENDIF(WIN32)

ADD_DEFINITIONS(-DPREFIX="${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "Fontmatrix will be installed under ${CMAKE_INSTALL_PREFIX}")



# ECM and KDE Frameworks are required on every platform. The application
# code is uniform across Linux, macOS, and Windows; only the Windows CI
# pipeline (packaging, vcpkg-based KF6 acquisition) is deferred.
# The oldest versions the project is built and run against: Debian 13 (Qt 6.8.2, KDE
# Frameworks 6.13) and Ubuntu 25.04 (Qt 6.8.3, KDE Frameworks 6.12), both in a container, with
# warnings as errors and a start of the application. CI covers the Flatpak runtime 6.10,
# Arch and KDE Craft, all newer. Nothing older has been tried.
set(QT_MIN_VERSION "6.8.0")
set(KF_MIN_VERSION "6.12.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

# Set before KDECompilerSettings so it doesn't override us.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
# KDECompilerSettings turns on the Qt strictness definitions. One of them still
# asks for changes across the code base and stays off for now:
#   QT_NO_CAST_FROM_ASCII  → per file, see src/CMakeLists.txt; the rest with the i18n work
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
)

# Nothing deprecated up to the minimum versions is used; keep it that way.
# A Linux build does not see the _WIN32 and PLATFORM_APPLE blocks: read them before raising this.
include(ECMDeprecationSettings)
ecm_set_disabled_deprecation_versions(
    QT ${QT_MIN_VERSION}
    KF ${KF_MIN_VERSION}
)

include(FeatureSummary)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Config
    XmlGui
    ConfigWidgets
    WidgetsAddons
    StatusNotifierItem
    DBusAddons
    Crash
)
# KDocTools is optional — if absent, the handbook is skipped but the app still builds.
find_package(KF6DocTools ${KF_MIN_VERSION})
set_package_properties(KF6DocTools PROPERTIES
    DESCRIPTION "Tools to generate documentation in various formats from DocBook"
    TYPE OPTIONAL
    PURPOSE "Required to build the Fontmatrix handbook")
# KAuth serves one feature on Linux: activating fonts for all users of the machine, through a
# root helper that polkit authorizes. Optional, and only where it can work: a distribution
# package, whose D-Bus service, policy and helper land on the host. A Flatpak cannot install
# any of them and passes -DCMAKE_DISABLE_FIND_PACKAGE_KF6Auth=ON.
set(FONTMATRIX_HAVE_KAUTH FALSE)
if(UNIX AND NOT APPLE)
    find_package(KF6Auth ${KF_MIN_VERSION})
    set_package_properties(KF6Auth PROPERTIES
        DESCRIPTION "Execute actions as privileged user"
        TYPE OPTIONAL
        PURPOSE "Activating fonts for all users of the computer (polkit)")
    if(KF6Auth_FOUND AND KAUTH_BACKEND_NAME STREQUAL "POLKITQT6-1")
        set(FONTMATRIX_HAVE_KAUTH TRUE)
        add_definitions(-DHAVE_KAUTH)
    endif()
endif()
add_feature_info("Font activation for all users" FONTMATRIX_HAVE_KAUTH "a root helper and polkit action, KF6Auth with its polkit backend")
SET(FONTMATRIX_BIN ${KDE_INSTALL_BINDIR})

# Very generic definition, mainly to sort out bugs reported by mrdocs
IF(APPLE)
	ADD_DEFINITIONS( -DPLATFORM_APPLE )
ENDIF(APPLE)



# PoDoFo serves one feature: Tools > Extract fonts, which takes the fonts embedded in a PDF
# file. It is optional, because Debian and Ubuntu still ship 0.9, whose API is another one.
# Without it the application is complete except for that menu entry. To build without it on
# purpose: -DCMAKE_DISABLE_FIND_PACKAGE_podofo=ON -DCMAKE_DISABLE_FIND_PACKAGE_LIBPODOFO=ON
set(FONTMATRIX_HAVE_PODOFO FALSE)
find_package(podofo CONFIG QUIET)
IF(podofo_FOUND)
	MESSAGE(STATUS "PoDoFo Library Found OK (CMake config)")
	set(FONTMATRIX_HAVE_PODOFO TRUE)
ELSEIF(NOT CMAKE_DISABLE_FIND_PACKAGE_LIBPODOFO)
	FIND_PACKAGE(LIBPODOFO)
	IF(LIBPODOFO_INCLUDE_DIR AND LIBPODOFO_LIBRARY)
		# 0.9 would be found and then fail as a wall of compiler errors
		IF(LIBPODOFO_VERSION AND LIBPODOFO_VERSION VERSION_LESS "0.10.0")
			MESSAGE(STATUS "PoDoFo ${LIBPODOFO_VERSION} found, 0.10 or newer is needed: building without the extraction of fonts from PDF files")
		ELSE()
			MESSAGE(STATUS "PoDoFo Library Found OK")
			set(FONTMATRIX_HAVE_PODOFO TRUE)
		ENDIF()
	ENDIF()
ENDIF()
IF(FONTMATRIX_HAVE_PODOFO)
	ADD_DEFINITIONS(-DHAVE_PODOFO)
	SET(PODOFO 1)
ENDIF()
add_feature_info("Font extraction from PDF files" FONTMATRIX_HAVE_PODOFO "Tools > Extract fonts; needs PoDoFo 0.10 or newer")


# I can’t remember what it does :|
ADD_DEFINITIONS( -DHIGH_PERF )


IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	MESSAGE(STATUS "Debug build")
	ADD_DEFINITIONS(-DBUILD_TYPE_DEBUG)
ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")
	ADD_DEFINITIONS(-DQT_NO_DEBUG_OUTPUT)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

# A lot from Scribus
#<< Qt
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Widgets Svg SvgWidgets Sql Xml PrintSupport Network DBus)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
#>> Qt

# HarfBuzz reads the OpenType tables and shapes the samples. It comes from the
# platform like Qt, which links it too: a distribution package, the Flatpak
# runtime, libs/harfbuzz in Craft. 2.6.8 has the call that lists alternates.
find_package(HarfBuzz 2.6.8 REQUIRED)
set_package_properties(HarfBuzz PROPERTIES
    TYPE REQUIRED
    PURPOSE "OpenType scripts, languages and features, text shaping"
)

# SET(PUZZLE_DIR libpuzzle/src)
# ADD_SUBDIRECTORY(${PUZZLE_DIR})
# MESSAGE(STATUS "Adding "${CMAKE_SOURCE_DIR}/${PUZZLE_DIR})
# SET(PUZZLE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${PUZZLE_DIR})

#<< FreeType2
# We give a try to the built-in cmake Freetype.cmake
# 2.10: variable fonts (FT_Get_MM_Var on OpenType, FT_Get_Var_Axis_Flags) and
# colour fonts (FT_LOAD_COLOR rendering the layers of COLR, FT_Get_Color_Glyph_Layer)
FIND_PACKAGE(Freetype 2.10 REQUIRED)
IF (FREETYPE_FOUND)
  MESSAGE(STATUS "FreeType2 Library Found OK")
ELSE (FREETYPE_FOUND)
  MESSAGE(FATAL_ERROR "No Freetype found")  
ENDIF(FREETYPE_FOUND)
#>> FreeType2

# The documents of OpenType-SVG colour glyphs are gzip-compressed (FMSvgGlyphs); zlib is
# underneath Qt and FreeType on every platform already.
find_package(ZLIB REQUIRED)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "Compression library"
    URL "https://zlib.net"
    TYPE REQUIRED
    PURPOSE "The glyphs of OpenType-SVG colour fonts, which are stored compressed")


# Optional: without it the fonts are managed, but not switched on and off for other programs.
# CMake's own module (3.14) and its imported target; a missing Fontconfig used to end up as
# "FONTCONFIG_INCLUDE_DIR-NOTFOUND" on the include path and a failure at generation time.
if(UNIX AND NOT APPLE)
	find_package(Fontconfig)
	set_package_properties(Fontconfig PROPERTIES
		TYPE RECOMMENDED
		PURPOSE "Activates and deactivates fonts for the other applications")
	if(Fontconfig_FOUND)
		add_definitions(-DHAVE_FONTCONFIG)
	endif()
endif()


SET(FM_TEMPLATES_FILES
${CMAKE_SOURCE_DIR}/src/graphic-resources/template_oneline.xml
${CMAKE_SOURCE_DIR}/src/graphic-resources/template_oneline.png
${CMAKE_SOURCE_DIR}/src/graphic-resources/template_default.xml
${CMAKE_SOURCE_DIR}/src/graphic-resources/template_default.png
)
# tell cmake to process CMakeLists.txt in that subdirectory
# libhyphen (hunspell) hyphenates the sample text when a dictionary is set.
# It comes with every platform: Arch "hyphen", Debian "libhyphen-dev", the
# KDE Flatpak runtime, and craft-blueprints/libs/hyphen for Windows.
find_package(Hyphen REQUIRED)
set_package_properties(Hyphen PROPERTIES
    DESCRIPTION "Hyphenation library of the hunspell project"
    URL "https://github.com/hunspell/hyphen"
    TYPE REQUIRED
    PURPOSE "Hyphenation of the sample texts")



ADD_SUBDIRECTORY(src)

# Handbook (DocBook → khelpcenter). Skipped silently when KDocTools is absent.
IF(KF6DocTools_FOUND)
    ADD_SUBDIRECTORY(doc)
ENDIF()

# dodict
# ADD_SUBDIRECTORY(src/langs)


######## Installation ##################

# The application's data files install through the KDE_INSTALL_* variables on
# every platform, so the layout is whatever ECM says it is. On Windows ECM
# resolves KDE_INSTALL_DATAROOTDIR to `bin/data`, one of the directories Qt's
# QStandardPaths searches next to the executable, so KXmlGui
# (fontmatrixui.rc), KIconLoader (theme icons) and KLocalizedString (.mo
# catalogues) resolve with no per-platform path code, and
# FMPaths::ResourcesDir() resolves to <appdir>/data/fontmatrix/resources.
#
# Only platform-specific things stay in the IF() branches below: the XDG
# desktop entry, the man page, the MSVC runtime.
SET(RESOURCESDIR "${KDE_INSTALL_DATADIR}/fontmatrix/resources")

INSTALL(FILES
${FM_TEMPLATES_FILES}
DESTINATION "${RESOURCESDIR}"
)

# Multi-size hicolor app icon + symbolic systray icon, installed via ECM.
# Source files in src/icons/ follow the ECM naming convention
# (<size>-<context>-<name>.<ext>), so ecm_install_icons resolves the
# destination automatically. Required on Windows too: the window icon, the
# toolbar and the KStatusNotifierItem all resolve through QIcon::fromTheme().
ecm_install_icons(ICONS
	src/icons/16-apps-fontmatrix.png
	src/icons/22-apps-fontmatrix.png
	src/icons/32-apps-fontmatrix.png
	src/icons/48-apps-fontmatrix.png
	src/icons/64-apps-fontmatrix.png
	src/icons/128-apps-fontmatrix.png
	src/icons/sc-apps-fontmatrix.svg
	src/icons/22-status-fontmatrix-tray.png
	src/icons/sc-status-fontmatrix-tray.svg
	DESTINATION "${KDE_INSTALL_ICONDIR}"
	THEME hicolor
)

# Compile po/<lang>/fontmatrix.po into <lang>/LC_MESSAGES/fontmatrix.mo at
# build time and install under ${KDE_INSTALL_LOCALEDIR}. KLocalizedString
# reads from ${KDE_INSTALL_LOCALEDIR}/<lang>/LC_MESSAGES/ at runtime.
# find_package(KF6 COMPONENTS I18n) already makes Gettext a hard requirement
# via KF6I18nMacros.cmake, so msgfmt is available on Windows too (Craft's
# ki18n pulls libs/gettext) and this needs no platform guard.
ki18n_install(po)

IF(UNIX AND NOT APPLE)
	# Desktop file and AppStream metadata share the application ID, which is also what
	# KAboutData::setDesktopFileName() tells the compositor: that is how a window finds
	# its launcher and icon on Wayland, and how a software centre finds the launcher.
	INSTALL(FILES
	com.github.fontmatrix.Fontmatrix.desktop
	DESTINATION ${KDE_INSTALL_APPDIR}
	)

	INSTALL(FILES
	com.github.fontmatrix.Fontmatrix.metainfo.xml
	DESTINATION ${KDE_INSTALL_METAINFODIR}
	)

	INSTALL(FILES
	fontmatrix.1
	DESTINATION "${KDE_INSTALL_MANDIR}/man1"
	)
ENDIF(UNIX AND NOT APPLE)

IF(APPLE)
# we do not "install" on apple’s OS
ENDIF(APPLE)

IF(WIN32)
	# No windeployqt: Craft's qtbase image dir already contains Qt DLLs and
	# plugins, and Craft's NullsoftInstallerPackager flattens plugins/<type>/
	# into bin/<type>/ at packaging time (movePluginsToBin=True on Windows).
	# Running windeployqt on top would deploy the same plugins twice and the
	# packager's mergeTree would fail on the duplicates. Same pattern as
	# Kate's craft-blueprints-kde blueprint.

	# MSVC runtime DLLs (vcruntime140.dll, msvcp140.dll, …) for non-Craft
	# local Windows builds only. Under Craft this is inert: Craft configures
	# every CMake project with -DCMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP=ON
	# and ships the redistributable through its own libs/runtime package,
	# which reaches us via the virtual/base runtime dependency declared in
	# craft-blueprints/fontmatrix/fontmatrix.py. Do not rely on the block
	# below for the packaged installer.
	set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${KDE_INSTALL_BINDIR}")
	include(InstallRequiredSystemLibraries)

	# FreeType + transitive DLLs are bundled by Craft's libs/freetype
	# blueprint and Craft's `--package` step on Windows. Earlier vcpkg-
	# specific copy logic was removed in Phase 8 along with the vcpkg CI
	# path. For non-Craft local Windows builds (e.g. Qt+vcpkg + manual
	# CMake), the developer is expected to make Qt/FreeType/PoDoFo DLLs
	# available on PATH.
ENDIF(WIN32)

ADD_SUBDIRECTORY(samples)

SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_PACKAGE_NAME "Fontmatrix")
SET(CPACK_PACKAGE_VENDOR "Fontmatrix")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Font management application")

IF(WIN32)
	# MSI installer via CPack WiX generator using WiX Toolset 6 (wix.exe).
	# CPACK_WIX_VERSION=4 selects the WiX 4+ code-path in CMake's CPack generator,
	# which invokes `wix.exe` (WiX 4/5/6) instead of the legacy candle.exe/light.exe
	# (WiX 3). WiX 6 uses UTF-8 throughout and avoids the LGHT0311 code-page error
	# that WiX 3 raises for file names containing non-Latin-1 characters.
	SET(CPACK_WIX_VERSION 4)
	SET(CPACK_GENERATOR "WIX")
	SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Fontmatrix")
	# UPGRADE_GUID must remain constant across all versions of this product.
	SET(CPACK_WIX_UPGRADE_GUID "5D8B1A2F-C3E4-4F56-8901-2B3C4D5E6F78")
	SET(CPACK_PACKAGE_EXECUTABLES "fontmatrix" "Fontmatrix")
	SET(CPACK_WIX_PROGRAM_MENU_FOLDER "Fontmatrix")
	SET(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://github.com/fontmatrix/fontmatrix/")
	# Build a combined license file: copyright header + GPL text + VC++ runtime notice.
	# file(READ) + file(WRITE) runs at configure time; the original COPYING is not touched.
	file(READ "${CMAKE_SOURCE_DIR}/COPYING" _gpl_text)
	file(WRITE "${CMAKE_BINARY_DIR}/LICENSE.txt"
"Fontmatrix ${VERSION} - Font Management Application\n\
Copyright (C) 2007 Pierre Marchand and contributors\n\
https://github.com/fontmatrix/fontmatrix/\n\
\n\
This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published below.\n\
\n\
Third-party components included in this installer:\n\
  Microsoft Visual C++ Runtime Libraries\n\
    Copyright (C) Microsoft Corporation. All rights reserved.\n\
    Redistributed under the Microsoft Visual C++ Runtime Library License.\n\
    See: https://learn.microsoft.com/en-us/legal/vc-redist-license-terms\n\
\n\
------------------------------------------------------------------------\n\
GNU GENERAL PUBLIC LICENSE\n\
------------------------------------------------------------------------\n\
\n\
${_gpl_text}")
	SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_BINARY_DIR}/LICENSE.txt")
ENDIF(WIN32)

# The KDE coding style: "ninja clang-format" formats everything, the pre-commit hook checks
# the lines a commit changes.
file(GLOB_RECURSE FONTMATRIX_CLANG_FORMAT_SOURCES src/*.cpp src/*.h dodict/*.cpp)
kde_clang_format(${FONTMATRIX_CLANG_FORMAT_SOURCES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

INCLUDE(CPack)

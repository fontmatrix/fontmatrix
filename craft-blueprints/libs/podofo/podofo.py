# SPDX-License-Identifier: GPL-2.0-or-later
#
# KDE Craft blueprint for PoDoFo (https://github.com/podofo/podofo).
#
# Why this is in the Fontmatrix repo and not upstream `craft-blueprints-kde`:
# Fontmatrix needs PoDoFo for its PDF font-extraction feature, but
# craft-blueprints-kde does not ship a PoDoFo blueprint as of master at
# the time of writing. We carry it in-tree alongside our fontmatrix
# blueprint; if the KDE ecosystem starts needing PoDoFo more broadly
# (Skanpage, Okular's PDF backends, etc.), this is a candidate for
# upstream contribution.
#
# Version: 1.1.2, matching the Flatpak manifest so Windows and Linux build
# against the same API. `src/fmpdffontextractor.cpp` was ported to the
# 0.10/1.x API and compiles unchanged against 1.1.2. Distro builds on
# 0.10.x keep working — that remains the documented minimum.
#
# CMake options (from PoDoFo 1.1.2's CMakeLists.txt):
#   REQUIRED   ZLIB, OpenSSL, Freetype, LibXml2
#   OPTIONAL   Libidn, JPEG, TIFF, PNG
#   Fontconfig REQUIRED, but only when PODOFO_WITH_FONTMANAGER is on
#
# We keep the optional deps minimal: PNG (already a small dep), no
# JPEG/TIFF (not used by Fontmatrix's font-extraction code), no Libidn
# (niche).

import info
from Package.CMakePackageBase import CMakePackageBase
from Utils import CraftHash


class subinfo(info.infoclass):
    def setTargets(self):
        self.targets["1.1.2"] = "https://github.com/podofo/podofo/archive/refs/tags/1.1.2.tar.gz"
        self.targetInstSrc["1.1.2"] = "podofo-1.1.2"
        self.targetDigests["1.1.2"] = (
            ["d6ffe6fc173ac6d6e5b00f5cb9db01990cab1bdf7cc03bdeffce3013bc9ec63a"],
            CraftHash.HashAlgorithm.SHA256,
        )
        self.defaultTarget = "1.1.2"
        self.description = "C++ PDF parsing/manipulation library"
        self.webpage = "https://github.com/podofo/podofo"

    def setDependencies(self):
        # Mandatory PoDoFo deps (from its CMakeLists `find_package(... REQUIRED)`)
        self.runtimeDependencies["virtual/base"] = None
        self.runtimeDependencies["libs/zlib"] = None
        self.runtimeDependencies["libs/openssl"] = None
        self.runtimeDependencies["libs/freetype"] = None
        self.runtimeDependencies["libs/libxml2"] = None
        # Optional but small — gives PNG embedding support, no harm.
        self.runtimeDependencies["libs/libpng"] = None


class Package(CMakePackageBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.subinfo.options.configure.args += [
            "-DPODOFO_BUILD_STATIC=FALSE",
            # PoDoFo 1.x's font manager is its system-font lookup, which
            # pulls in a REQUIRED Fontconfig on every platform unless
            # PODOFO_WITH_WIN32GDI_FONT_SEARCH is also on. Fontmatrix only
            # reads embedded font streams out of PDF objects and never asks
            # PoDoFo to find a font, so turn the whole thing off rather than
            # add a Fontconfig dependency for nothing.
            "-DPODOFO_WITH_FONTMANAGER=OFF",
            # Skip optional deps Fontmatrix doesn't need.
            "-DCMAKE_DISABLE_FIND_PACKAGE_Libidn=TRUE",
            "-DCMAKE_DISABLE_FIND_PACKAGE_JPEG=TRUE",
            "-DCMAKE_DISABLE_FIND_PACKAGE_TIFF=TRUE",
            # PoDoFo's tests need a writable test-data directory and
            # are not useful in CI; skip them entirely.
            "-DPODOFO_BUILD_TEST=FALSE",
            "-DPODOFO_BUILD_EXAMPLES=FALSE",
        ]

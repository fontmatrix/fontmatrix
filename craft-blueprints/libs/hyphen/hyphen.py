# SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# KDE Craft blueprint for libhyphen (https://github.com/hunspell/hyphen), the
# hyphenation library of the hunspell project. Every other platform Fontmatrix
# builds on has it already (Arch, Debian, the KDE Flatpak runtime); Craft has
# hunspell but no hyphen, so the blueprint lives here, next to libs/podofo.
#
# Upstream builds with autotools only, which is no good on MSVC, so a patch adds
# a CMakeLists.txt of two lines of substance: a static library of hyphen.c and
# hnjalloc.c, and the header. src/fmhyphenator.cpp uses hnj_hyphen_load(),
# hnj_hyphen_hyphenate2() and hnj_hyphen_free(), unchanged since 2.4.

import info
from Package.CMakePackageBase import CMakePackageBase
from Utils import CraftHash


class subinfo(info.infoclass):
    def setTargets(self):
        self.targets["2.8.9"] = "https://github.com/hunspell/hyphen/archive/refs/tags/v2.8.9.tar.gz"
        self.targetInstSrc["2.8.9"] = "hyphen-2.8.9"
        self.targetDigests["2.8.9"] = (
            ["d6fc75d8500094903b119a45667fed91fe665e5f6f106029a4d3b4e26f3d8d6f"],
            CraftHash.HashAlgorithm.SHA256,
        )
        self.patchToApply["2.8.9"] = [("hyphen-2.8.9-cmake.diff", 1)]
        self.patchLevel["2.8.9"] = 1
        self.defaultTarget = "2.8.9"
        self.description = "Hyphenation library of the hunspell project"
        self.webpage = "https://github.com/hunspell/hyphen"

    def setDependencies(self):
        self.runtimeDependencies["virtual/base"] = None


class Package(CMakePackageBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

# SPDX-License-Identifier: GPL-2.0-or-later
#
# KDE Craft blueprint for Fontmatrix.
#
# KDE Craft builds Fontmatrix with it for the Windows installer and for the
# Linux AppImage; distribution packages and the Flatpak do not use it.
#
# Blueprint deployment in CI: copied at install time to
#   <CraftRoot>/etc/blueprints/locations/fontmatrix/fontmatrix/fontmatrix.py
# See .github/workflows/build.yml and .github/scripts/build-appimage.sh for the
# exact wiring, and BUILDING-WINDOWS.md for local-developer setup.
#
# Pattern derived from the upstream Kirigami tutorial blueprint:
#   https://develop.kde.org/docs/getting-started/building/craft/

import os

import info
from CraftCore import CraftCore
from Package.CMakePackageBase import CMakePackageBase


class subinfo(info.infoclass):
    def setTargets(self):
        self.displayName = "Fontmatrix"
        self.description = "Cross-platform font management application built on Qt6 / KF6"
        self.webpage = "https://github.com/fontmatrix/fontmatrix"

        # CI passes --options fontmatrix.srcDir=<checkout>, so the URL is
        # never fetched — but Craft requires svnTargets to have at least
        # one entry. CRAFT_FONTMATRIX_VERSION_FULL is set by the workflow.
        version = os.environ.get("CRAFT_FONTMATRIX_VERSION_FULL", "master")
        self.svnTargets[version] = "https://github.com/fontmatrix/fontmatrix.git"
        self.defaultTarget = version

    def setDependencies(self):
        # Pulls libs/runtime, which installs the MSVC redistributable DLLs
        # (vcruntime140.dll, msvcp140.dll, …) into bin/. Without it the
        # package has no C runtime. Every upstream KDE blueprint declares it.
        self.runtimeDependencies["virtual/base"] = None
        self.buildDependencies["kde/frameworks/extra-cmake-modules"] = None
        # meinproc6 turns the DocBook handbook into the HTML FMHandbookWindow reads,
        # the only handbook there is on Windows (no KHelpCenter). Build time only:
        # without it CMake skips doc/ and F1 says the handbook is not installed.
        self.buildDependencies["kde/frameworks/tier2/kdoctools"] = None

        # qttools / qtdeclarative are NOT runtime deps. qtdeclarative pulls
        # libs/llvm (~1 GB of clang tools); we don't ship Linguist/Designer
        # and translations flow through ki18n_install(po), not lrelease.
        self.runtimeDependencies["libs/qt/qtbase"] = None
        self.runtimeDependencies["libs/qt/qtsvg"] = None

        self.runtimeDependencies["kde/frameworks/tier1/kcoreaddons"] = None
        self.runtimeDependencies["kde/frameworks/tier1/ki18n"] = None
        self.runtimeDependencies["kde/frameworks/tier1/kconfig"] = None
        self.runtimeDependencies["kde/frameworks/tier1/kdbusaddons"] = None
        self.runtimeDependencies["kde/frameworks/tier1/kwidgetsaddons"] = None
        self.runtimeDependencies["kde/frameworks/tier2/kcrash"] = None
        self.runtimeDependencies["kde/frameworks/tier2/kstatusnotifieritem"] = None
        self.runtimeDependencies["kde/frameworks/tier3/kxmlgui"] = None
        self.runtimeDependencies["kde/frameworks/tier3/kconfigwidgets"] = None

        # The UI loads its icons through QIcon::fromTheme(). Both arrive
        # transitively via kxmlgui -> kiconthemes -> breeze-icons; named here
        # so a dependency trim cannot drop them silently.
        self.runtimeDependencies["kde/frameworks/tier3/kiconthemes"] = None
        self.runtimeDependencies["kde/frameworks/tier1/breeze-icons"] = None

        self.runtimeDependencies["libs/freetype"] = None
        # Qt brings it along already, since Craft builds Qt against the system
        # HarfBuzz. Fontmatrix links it itself, so it is named.
        self.runtimeDependencies["libs/harfbuzz"] = None
        self.runtimeDependencies["libs/podofo"] = None
        self.runtimeDependencies["libs/hyphen"] = None
        # the gzip-compressed documents of OpenType-SVG fonts
        self.runtimeDependencies["libs/zlib"] = None
        if CraftCore.compiler.isLinux:
            # switching fonts on and off for the other applications
            self.runtimeDependencies["libs/fontconfig"] = None


class Package(CMakePackageBase):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        # No -DCMAKE_BUILD_TYPE: Craft appends its own after the blueprint's
        # args, so one set here has no effect. It comes from Compile/BuildType
        # in .github/craft/CraftConfig.ini.
        # The polkit helper for activation for all users would have to be installed on the
        # host, which an AppImage cannot do.
        cmake_args = [
            "-DCMAKE_DISABLE_FIND_PACKAGE_KF6Auth=ON",
        ]
        for component in ("MAJOR", "MINOR", "PATCH"):
            value = os.environ.get(f"CRAFT_FONTMATRIX_VERSION_{component}")
            if value:
                cmake_args.append(f"-DVERSION_{component}={value}")
        self.subinfo.options.configure.args = " ".join(cmake_args)

        # Layer a custom blacklist on top of Craft's default
        # applications_blacklist.txt. Skip silently if the file isn't
        # alongside the blueprint (Craft's toRegExp() raises on missing
        # paths, which would abort packaging).
        blueprint_dir = os.path.dirname(os.path.abspath(__file__))
        blacklist_path = os.path.join(blueprint_dir, "blacklist.txt")
        if os.path.isfile(blacklist_path):
            self.blacklist_file.append(blacklist_path)

    def formatVersion(self, includeRevision, includeTimeStamp):
        # Default uses [git rev-parse --abbrev-ref HEAD, self.version],
        # which on a detached CI checkout becomes "HEAD-<version>". Collapse
        # it to a single label when the workflow exports a version.
        env_version = os.environ.get("CRAFT_FONTMATRIX_VERSION_FULL")
        if env_version:
            return env_version
        return super().formatVersion(includeRevision, includeTimeStamp)

    def createPackage(self):
        self.defines["appname"] = "fontmatrix"
        self.defines["company"] = "Fontmatrix"
        self.defines["website"] = "https://github.com/fontmatrix/fontmatrix"
        # The AppImage packager looks for share/applications/*<desktopFile>.desktop,
        # <appname> by default; the file is named after the application ID.
        self.defines["desktopFile"] = "com.github.fontmatrix.Fontmatrix"
        # NSIS template references @{version}; Craft only auto-fills it
        # when a concrete svnTarget version is set, so set it defensively.
        self.defines["version"] = os.environ.get("CRAFT_FONTMATRIX_VERSION_FULL", "0.0.0")
        self.defines["shortcuts"] = [
            {"name": "Fontmatrix", "target": "bin/fontmatrix.exe"},
        ]

        # Without these the NSIS packager uses Craft's own defaults: its
        # mascot icon, and no licence page. Guarded so packaging cannot fail
        # over a cosmetic define.
        try:
            src = self.sourceDir()
        except Exception as exc:  # noqa: BLE001 - cosmetic defines only
            CraftCore.log.warning(f"fontmatrix: no source dir for installer assets: {exc}")
            src = None
        if src:
            icon = src / "src/icons/fontmatrix.ico"
            if icon.exists():
                self.defines["icon"] = icon
            licence = src / "COPYING"
            if licence.exists():
                self.defines["license"] = licence

        # Drop random executables that came along with build deps but aren't
        # ours (e.g. qmldom.exe, androiddeployqt.exe, openssl.exe). Same
        # pattern as kate.py; bin/fontmatrix on Linux, bin/fontmatrix.exe on Windows.
        self.addExecutableFilter(r"(bin|libexec)/(?!fontmatrix(\.exe)?$).*")

        self.ignoredPackages.append("binary/mysql")
        # qtdeclarative is NOT ignored, even though Fontmatrix is a pure
        # QtWidgets app that loads no QML itself. kconfig, ki18n and
        # kguiaddons declare it as a runtime dependency and ship QML wrappers
        # (KF6ConfigQml, KLocalizedQmlContext, ki18n's Transcript plugin, …)
        # inside their own image dirs. Excluding it dropped Qt6Qml.dll while
        # those wrappers were still collected, so the package carried
        # libraries that could not load, and each one had to be hunted down
        # and blacklisted separately.
        #
        # qttools and libs/llvm stay ignored: qttools declares llvm as a
        # *runtime* dependency, which would add clang to the installer. It is
        # only a build dependency of qtdeclarative, so this is insurance
        # against a future runtime path to it rather than something currently
        # in the graph.
        self.ignoredPackages.extend([
            "libs/qt6/qttools",
            "libs/llvm",
        ])
        # D-Bus is Linux-only for our use case; KDBusAddons gracefully no-ops
        # when D-Bus isn't installed on Windows. Mirrors kate.py.
        if not CraftCore.compiler.isLinux:
            self.ignoredPackages.append("libs/dbus")

        return super().createPackage()

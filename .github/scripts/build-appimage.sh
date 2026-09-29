#!/bin/bash
# SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Builds the Fontmatrix AppImage with KDE Craft, the way KDE builds its own:
# sysadmin/craft-ci gitlab-templates/blocks/linux-base.yml, on the AlmaLinux 9
# image of sysadmin/ci-images craft-appimage-alma9. KDE's binary cache for
# linux-gcc-x86_64 is built there, so this has to run on AlmaLinux 9 as well;
# anything else misses the cache, and Qt and KDE Frameworks are compiled from
# source. glibc 2.34 is also the oldest one the AppImage then runs on.
#
#   build-appimage.sh [output directory]
#
# Run as root in an almalinux:9 container, with the source tree as the working
# directory. CI does that in the appimage job; locally:
#
#   podman run --rm -v "$PWD":/src:Z -v fontmatrix-craft:/craft -w /src \
#       almalinux:9.8 .github/scripts/build-appimage.sh
#
# The Craft root lives in /craft (the volume above keeps it between runs), and
# the AppImage lands in appimage-out/. CRAFT_FONTMATRIX_VERSION_{FULL,MAJOR,
# MINOR,PATCH} name the version, as for the Windows build; without them the
# package is called after the checked-out commit.
set -euo pipefail

SRC="$(pwd)"
OUT="${1:-$SRC/appimage-out}"
CRAFT_TARGET=linux-64-gcc
CRAFT_ROOT_DIR=/craft
CRAFTMASTER=/opt/craftmaster

# The packages of the craft-appimage-alma9 image that a Qt Widgets application
# needs: the compiler, Python for Craft, and the X11, Wayland and OpenGL
# headers the Qt of the binary cache was built against. Its CMake packages ask
# for them again when Fontmatrix links to Qt.
#
# fontconfig as well: the AppImage uses the host's, since Craft's copy is older
# than the configuration in a current /etc/fonts and warns about every file it
# does not understand. craft-blueprints/fontmatrix/blacklist.txt keeps Craft's
# out of the package, and linuxdeploy, which has libfontconfig.so.1 on its
# exclude list, still has to find one before it skips it.
dnf install -y 'dnf-command(config-manager)'
dnf config-manager --set-enabled crb
dnf install -y \
    git-core cmake gcc-toolset-14 python3.11 python3.11-pip sqlite zlib fuse file patch \
    at-spi2-atk-devel wayland-devel mesa-libGL-devel mesa-libEGL-devel \
    libX11-devel libX11-xcb libxcb-devel libxkbcommon-devel libxkbcommon-x11-devel \
    libXi-devel libXcursor-devel libXrandr-devel \
    xcb-util-devel xcb-util-cursor-devel xcb-util-keysyms-devel xcb-util-renderutil-devel \
    xcb-util-wm-devel xcb-util-image-devel xorg-x11-util-macros \
    flex bison gperf systemd-devel libmount-devel appstream fontconfig

PKG_CONFIG_PATH="$(/usr/bin/pkg-config --variable pc_path pkg-config)"
export PKG_CONFIG_PATH
set +u
# shellcheck disable=SC1091
source /opt/rh/gcc-toolset-14/enable
set -u
export LANG=C.UTF-8

# linuxdeploy and its plugins are AppImages themselves, and a container has no
# FUSE. Craft sets this for Docker only; podman needs it too.
export APPIMAGE_EXTRACT_AND_RUN=1
# As KDE's appimage-qt6.yml: appimagetool would validate the metainfo with
# appstream-util, which does not know the current format.
export NO_APPSTREAM=1

if [ -z "${CRAFT_FONTMATRIX_VERSION_FULL:-}" ]; then
    CRAFT_FONTMATRIX_VERSION_FULL="git-$(git -C "$SRC" rev-parse --short=7 HEAD 2>/dev/null || echo local)"
    export CRAFT_FONTMATRIX_VERSION_FULL
fi

[ -f "$CRAFTMASTER/CraftMaster.py" ] ||
    git clone --depth 1 https://invent.kde.org/packaging/craftmaster.git "$CRAFTMASTER"

craftmaster() {
    python3.11 -u "$CRAFTMASTER/CraftMaster.py" --config "$SRC/.github/craft/CraftConfig.ini" \
        --variables "Root=$CRAFT_ROOT_DIR" "Python=" --targets "$CRAFT_TARGET" "$@"
}

craftmaster --setup
craftmaster -c -i --options virtual.ignored=True --update craft

# The in-tree blueprints, where Craft looks for them. See the Windows job.
BLUEPRINTS="$CRAFT_ROOT_DIR/$CRAFT_TARGET/etc/blueprints/locations/fontmatrix"
rm -rf "$BLUEPRINTS"
mkdir -p "$BLUEPRINTS"
cp -r "$SRC"/craft-blueprints/* "$BLUEPRINTS/"

craftmaster -c --install-deps fontmatrix
craftmaster -c --no-cache --ignoreInstalled --options "fontmatrix.srcDir=$SRC" fontmatrix
craftmaster -c -i --update linuxdeploy

# Packages of earlier runs stay there when /craft is kept between runs.
packageDir="$(craftmaster -c -q --get 'packageDestinationDir()' virtual/base | tail -n 1)"
rm -f "$packageDir"/fontmatrix-*.AppImage "$packageDir"/fontmatrix-*.AppImage.sha256
craftmaster -c --package --options "fontmatrix.srcDir=$SRC" fontmatrix

mkdir -p "$OUT"
cp -v "$packageDir"/fontmatrix-*.AppImage "$OUT/"
cp -v "$packageDir"/fontmatrix-*.AppImage.sha256 "$OUT/" 2>/dev/null || true

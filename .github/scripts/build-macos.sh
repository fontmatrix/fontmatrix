#!/bin/bash
# SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Builds the Fontmatrix disk image for macOS with KDE Craft, the way KDE builds
# its own: sysadmin/craft-ci gitlab-templates/blocks/macos-base.yml, with the
# macos-arm-clang and macos-64-clang targets of .github/craft/CraftConfig.ini.
# KDE publishes a RelWithDebInfo binary cache for both, so Qt and KDE
# Frameworks are downloaded rather than compiled.
#
#   build-macos.sh <Craft target> [output directory]
#
# Run from the source tree, on a Mac of the target's architecture (the Intel
# target also runs under Rosetta 2, as on KDE's builders). The Craft root is
# $CRAFT_ROOT_DIR, ~/craft by default, and the .dmg lands in macos-out/.
# CRAFT_FONTMATRIX_VERSION_{FULL,MAJOR,MINOR,PATCH} name the version, as for
# the Windows build and the AppImage.
#
# The disk image is not signed: CodeSigning/Enabled is off in CraftConfig.ini.
set -euo pipefail

CRAFT_TARGET="${1:?usage: build-macos.sh <macos-arm-clang|macos-64-clang> [output directory]}"
SRC="$(pwd)"
OUT="${2:-$SRC/macos-out}"
CRAFT_ROOT_DIR="${CRAFT_ROOT_DIR:-$HOME/craft}"
CRAFTMASTER="${CRAFTMASTER:-$CRAFT_ROOT_DIR/craftmaster}"
PYTHON="${PYTHON:-python3}"

export LANG=en_US.UTF-8

if [ -z "${CRAFT_FONTMATRIX_VERSION_FULL:-}" ]; then
    CRAFT_FONTMATRIX_VERSION_FULL="git-$(git -C "$SRC" rev-parse --short=7 HEAD 2>/dev/null || echo local)"
    export CRAFT_FONTMATRIX_VERSION_FULL
fi

mkdir -p "$CRAFT_ROOT_DIR"
[ -f "$CRAFTMASTER/CraftMaster.py" ] ||
    git clone --depth 1 https://invent.kde.org/packaging/craftmaster.git "$CRAFTMASTER"

craftmaster() {
    "$PYTHON" -u "$CRAFTMASTER/CraftMaster.py" --config "$SRC/.github/craft/CraftConfig.ini" \
        --variables "Root=$CRAFT_ROOT_DIR" "Python=" --targets "$CRAFT_TARGET" "$@"
}

craftmaster --setup
craftmaster -c -i --options virtual.ignored=True --update craft

# The in-tree blueprints, where Craft looks for them. See the Windows job.
BLUEPRINTS="$CRAFT_ROOT_DIR/$CRAFT_TARGET/etc/blueprints/locations/fontmatrix"
rm -rf "$BLUEPRINTS"
mkdir -p "$BLUEPRINTS"
cp -R "$SRC"/craft-blueprints/* "$BLUEPRINTS/"

craftmaster -c --install-deps fontmatrix
craftmaster -c --no-cache --ignoreInstalled --options "fontmatrix.srcDir=$SRC" fontmatrix

# Disk images of earlier runs stay there when the Craft root is kept.
packageDir="$(craftmaster -c -q --get 'packageDestinationDir()' virtual/base | tail -n 1)"
rm -f "$packageDir"/fontmatrix-*.dmg "$packageDir"/fontmatrix-*.dmg.sha256
craftmaster -c --package --options "fontmatrix.srcDir=$SRC" fontmatrix

mkdir -p "$OUT"
cp -v "$packageDir"/fontmatrix-*.dmg "$OUT/"
cp -v "$packageDir"/fontmatrix-*.dmg.sha256 "$OUT/" 2>/dev/null || true

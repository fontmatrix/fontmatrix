#! /usr/bin/env bash
# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: none
#
# Extracts the translatable strings into $podir/fontmatrix.pot.
# KDE's scripty sets EXTRACTRC, XGETTEXT and podir. This project is not hosted
# there, so the same defaults are provided here and the script also runs by hand:
#     ./Messages.sh && for f in po/*/fontmatrix.po; do msgmerge -U --backup=none "$f" po/fontmatrix.pot; done

: "${EXTRACTRC:=extractrc}"
: "${podir:=po}"
: "${XGETTEXT:=xgettext --from-code=UTF-8 -C --kde -ci18n \
 -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 \
 -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 \
 -kxi18n:1 -kxi18nc:1c,2 -kxi18np:1,2 -kxi18ncp:1c,2,3 \
 -kkxi18n:1 -kkxi18nc:1c,2 -kkxi18np:1,2 -kkxi18ncp:1c,2,3 \
 -kI18N_NOOP:1 -kI18NC_NOOP:1c,2 -ktr2i18n:1 \
 --msgid-bugs-address=https://github.com/fontmatrix/fontmatrix/issues}"

# src/fontmatrix.rc is the Windows resource script, not an XMLGUI file
$EXTRACTRC $(find src \( -name '*.ui' -o -name '*.rc' -o -name '*.kcfg' \) ! -name 'fontmatrix.rc' | sort) >> rc.cpp
$XGETTEXT rc.cpp $(find src -maxdepth 1 \( -name '*.cpp' -o -name '*.h' \) | sort) -o "$podir/fontmatrix.pot"
rm -f rc.cpp

/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Thanks to jghali
#ifdef _WIN32

// clang-format off: windows.h has to come first
#include <windows.h>
#include <shlobj.h>
// clang-format on

QString getWin32SystemFontDir()
{
    QString qstr;
    WCHAR dir[512];
    if (SHGetSpecialFolderPathW(nullptr, dir, CSIDL_FONTS, false)) {
        qstr = QString::fromWCharArray(dir);
        if (!qstr.endsWith(QLatin1Char('\\')))
            qstr += QLatin1Char('\\');
        qstr.replace(QLatin1Char('\\'), QLatin1Char('/'));
    }
    return qstr;
}

#endif

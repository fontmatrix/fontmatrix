/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* fmpath.h */
/* Get all useful paths in one place with the hope that */
/* finally we will not need resource.qrc anymore */
#ifndef FMPATHS_H
#define FMPATHS_H

#include <QDir>
#include <QLocale>
#include <QMap>
#include <QString>

class FMPaths
{
    QMap<QString, QString> FMPathsDB;
    FMPaths() = default;
    // Meyers singleton — thread-safe by C++11 static-local guarantee
    static FMPaths *getThis();

public:
    static QString TranslationsDir();

    static QString ResourcesDir();

    static QString HelpDir();

    /**
     * The first page of the handbook as HTML, for the desktops where
     * KHelpCenter cannot show the DocBook one: a Flatpak, Windows, macOS.
     * Empty when the build had no KDocTools and nothing was generated.
     */
    static QString HandbookFile();

    static QString SamplesDir();

    static QString FiltersDir();

    static QString LocalizedDirPath(const QString &base, const QString &fallback = QStringLiteral("en"));

    static QString LocalizedFilePath(const QString &base, const QString &ext, const QString &fallback = QStringLiteral("en"));
    /**
     * The hyphenation dictionary for @p locale among those installed on the system
     * (the hyph_xx_YY.dic files of the hunspell "hyphen-*" packages, in every
     * <data dir>/hyphen: /usr/share/hyphen, the Flatpak runtime, <appdir>/data on
     * Windows). "bg_BG" is tried, then any "bg_*", then any "bg"; an empty string when
     * there is none.
     */
    static QString HyphenationDictionary(const QLocale &locale = QLocale::system());

    /**
     * Linux: the directory the activated fonts are copied into,
     * ~/.local/share/fonts/fontmatrix. Fontconfig scans it by itself, Flatpak
     * gives every sandbox ~/.local/share/fonts, GNOME watches it.
     */
    static QString UserFontsDir();
    /// Linux: the fontconfig file that hides the system fonts switched off, in ~/.config/fontconfig/conf.d
    static QString FontconfigRejectsFile();
    /// Linux: the user's own fontconfig file, ~/.config/fontconfig/fonts.conf, which older versions edited
    static QString FontconfigUserFile();
};
#endif

/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMLANGCOVERAGE_H
#define FMLANGCOVERAGE_H

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

class FontItem;

/**
 * Which languages a font can set, the question a designer asks first.
 *
 * Fontconfig answers it: it holds one orthography per language, the characters
 * a text in it needs, and says of every font which of them it covers — the
 * same set as a ":lang=bg" query on the command line. The list of the fonts it
 * knows comes from its cache in one call; a font outside its directories, in a
 * collection of the user's own, is read from its file.
 *
 * Without fontconfig (Windows, macOS) nothing is known and the filter is not
 * offered.
 */
class FMLangCoverage
{
public:
    /// whether the coverage can be told at all, i.e. the build has fontconfig
    static bool isAvailable();

    /// the languages of the font, as fontconfig names them ("bg", "az-ir"), sorted
    static QStringList languagesOf(FontItem *fit);
    /// every language at least one font of the database covers, sorted by name
    static QStringList languages();
    /// the fonts that cover the language
    static QList<FontItem *> fonts(const QString &language);

    /// "Bulgarian (bg)", or the code alone when Qt does not know it
    static QString name(const QString &language);
    /// the language in its own words, for a tooltip; empty when Qt does not know it
    static QString nativeName(const QString &language);

    /// the fonts of the database are read again: after an import, or a change of the collection
    static void invalidate();

private:
    static QStringList read(const QString &file);
    static void fillFromCache();

    static QHash<QString, QStringList> m_byFile; ///< the fonts fontconfig knows, and those read one by one
    static bool m_cacheRead;
};

#endif

/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMLICENSE_H
#define FMLICENSE_H

#include <QHash>
#include <QList>
#include <QString>

class FontItem;

/**
 * Under which licence a font may be used, which decides whether it can go into
 * a client's work at all.
 *
 * The name table of an OpenType font has two fields for it, a description
 * (name ID 13) and a URL (ID 14), and they are free text: every foundry words
 * them its own way. They are read into the database at import and recognised
 * here by the sentences and addresses the well-known licences are stated with.
 * What is left is reported as it stands: a licence of its own, or none at all.
 */
class FMLicense
{
public:
    enum Family {
        None = 0, ///< the font says nothing about its licence
        Other, ///< a licence of its own, not one of those below
        OFL,
        Apache,
        GPL,
        LGPL,
        MIT,
        BSD,
        CC0,
        CCBY,
        CCBYSA,
        CCBYND,
        UFL, ///< the Ubuntu font licence
        PublicDomain,
        Freeware, ///< free to use, not to change: "freeware", "free for personal use"
        Proprietary ///< reserved, restricted, "all rights reserved" and nothing permitted
    };

    /// the licence of the font, from what the database read out of its name table
    static Family of(FontItem *fit);
    /// the licence, named for the interface
    static QString name(Family family);
    /// the licences the fonts of the database are under, the commonest first
    static QList<Family> families();
    /// the fonts under the licence
    static QList<FontItem *> fonts(Family family);

    /// the database was read again: after an import
    static void invalidate();

private:
    static Family classify(const QString &text);
    static void fill();

    static QHash<FontItem *, Family> m_byFont;
    static bool m_read;
};

#endif

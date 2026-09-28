/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMFONTKINDS_H
#define FMFONTKINDS_H

#include <QHash>
#include <QList>

class FontItem;

/**
 * The fonts that are more than one set of outlines: variable fonts, whose
 * design moves along axes (OpenType Font Variations, the fvar table), and colour
 * fonts, whose glyphs carry their own colours (COLR/CPAL layers or paint,
 * CBDT and sbix bitmaps, SVG documents).
 *
 * Telling needs the face open once per font, which FontItem remembers; the
 * lists are kept until the collection changes. A remote font whose file is not
 * here yet is not opened for this.
 */
class FMFontKinds
{
public:
    enum Kind {
        Variable = 0,
        Colour
    };

    /// the fonts of that kind in the collection
    static QList<FontItem *> fonts(Kind kind);
    static int count(Kind kind);

    /// the fonts of the database changed: everything is worked out again
    static void invalidate();

private:
    static QHash<int, QList<FontItem *>> m_fonts;
};

#endif

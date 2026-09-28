/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMSVGGLYPHS_H
#define FMSVGGLYPHS_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QStringList>

#include <ft2build.h>
#include FT_FREETYPE_H

class QImage;

/**
 * The glyphs of an OpenType-SVG colour font (the 'SVG ' table), drawn with
 * Qt SVG.
 *
 * FreeType reads such a table only through rendering hooks an application
 * registers for the whole library; the table is read here instead, the way the
 * specification describes it: the document for a glyph from the document list,
 * gzip or plain, the glyph being the element with the id "glyph<N>" rendered
 * as though it were used from the defs; SVG units are font units with y
 * pointing down and the origin at the glyph origin, and a viewBox maps onto
 * the em square. Colour variables (var(--colorN, fallback)) take the first
 * palette of the CPAL table, or their fallback.
 *
 * The table is copied once, while the face is open; it does not need the face
 * after that.
 */
class FMSvgGlyphs
{
public:
    /// the SVG glyphs of the face, nullptr when it has no usable SVG table
    static FMSvgGlyphs *create(FT_Face face);

    [[nodiscard]] bool hasGlyph(unsigned int glyph) const;

    /**
     * The glyph drawn at `ppem` pixels per em: the picture, trimmed to its ink,
     * with the position of its left edge and of its top edge above the baseline,
     * in pixels from the glyph origin — as FMColorPainter::paint() gives them.
     */
    bool paint(unsigned int glyph, double ppem, QImage &img, double &left, double &top);

private:
    FMSvgGlyphs() = default;

    struct Record {
        unsigned int first = 0;
        unsigned int last = 0;
        quint32 offset = 0;
        quint32 length = 0;
    };

    /// the document of the record, decompressed; cached, several glyphs share one
    QByteArray document(const Record &record);
    /// the document the glyph is drawn from, as the specification defines it
    QByteArray glyphDocument(unsigned int glyph);

    QByteArray m_table;
    quint32 m_listOffset = 0;
    QList<Record> m_records;
    int m_unitsPerEm = 1000;
    QStringList m_palette; ///< "#rrggbb" of the first CPAL palette, by entry
    QHash<quint32, QByteArray> m_documents;
    QHash<unsigned int, QByteArray> m_glyphDocuments;
};

#endif

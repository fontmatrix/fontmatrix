/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMCOLORPAINTER_H
#define FMCOLORPAINTER_H

#include <QColor>
#include <QImage>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>

/**
 * Paints the glyphs of a COLR version 1 font, the ones FreeType only lists:
 * gradients, transforms, groups blended together. HarfBuzz walks the paint
 * graph of the glyph and calls back for every operation, and the callbacks
 * here are a QPainter on an image.
 *
 * Needs HarfBuzz 7.0; with an older one there is no painter, and FreeType's
 * rendering of the base glyph stays.
 */
namespace FMColorPainter
{
/// a HarfBuzz font on the face, in font units, for hasPaint() and paint(); nullptr when the face has no COLR version 1
hb_font_t *paintFont(FT_Face face);

/// the font has COLR version 1 paint for the glyph
bool hasPaint(hb_font_t *font, unsigned int glyph);

/**
 * Paints the glyph at ppem pixels per em. The image holds the glyph in its
 * clip box; left and top say where the image sits relative to the pen, in
 * pixels, the way FreeType's bitmap_left and bitmap_top do. False when the
 * glyph has no paint or it cannot be painted.
 */
bool paint(hb_font_t *font, unsigned int glyph, double ppem, const QColor &foreground, QImage &image, double &left, double &top);
}

#endif

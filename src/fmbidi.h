/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMBIDI_H
#define FMBIDI_H

#include "fmsharestruct.h"

/**
 * Right-to-left text in the layout: the Unicode Bidirectional Algorithm
 * (UAX #9) on shaped glyphs.
 *
 * The layout breaks lines in logical order, the order the text is written in,
 * and draws them from left to right. A line of Hebrew or Arabic has to be
 * turned around before it is drawn, and inside it a number or a Latin word has
 * to be turned back. That is what this does, from the character each glyph
 * comes from (RenderedGlyph::lChar).
 *
 * The rules applied are those of a paragraph without explicit embeddings,
 * which is what a sample text is: the paragraph direction from its first
 * strong character (P2–P3), the weak types (W1–W7), the neutrals (N1–N2), the
 * implicit levels (I1–I2), and the reordering of each line (L2) with mirroring
 * (L4). A mark goes with the character it sits on, before it in a right-to-left run.
 */
namespace FMBidi
{
/// whether the first strong character of the paragraph is right-to-left
bool isRightToLeft(const GlyphList &paragraph);

/// turns a line from logical into visual order, left to right
void toVisualOrder(GlyphList &line, bool rightToLeftParagraph);
}

#endif

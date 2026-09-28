/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmbidi.h"

#include <QChar>

#include <algorithm>

namespace
{
// The bidi classes of UAX #9 that the implicit algorithm tells apart; the
// explicit formatting characters count as boundary neutrals, there being no
// embeddings to open.
enum Type {
    L,
    R,
    AL,
    EN,
    ES,
    ET,
    AN,
    CS,
    NSM,
    N
};

Type typeOf(uint c)
{
    switch (QChar::direction(c)) {
    case QChar::DirL:
    case QChar::DirLRE:
    case QChar::DirLRO:
        return L;
    case QChar::DirR:
    case QChar::DirRLE:
    case QChar::DirRLO:
        return R;
    case QChar::DirAL:
        return AL;
    case QChar::DirEN:
        return EN;
    case QChar::DirES:
        return ES;
    case QChar::DirET:
        return ET;
    case QChar::DirAN:
        return AN;
    case QChar::DirCS:
        return CS;
    case QChar::DirNSM:
        return NSM;
    default:
        return N;
    }
}

bool isMark(uint c)
{
    switch (QChar::category(c)) {
    case QChar::Mark_NonSpacing:
    case QChar::Mark_SpacingCombining:
    case QChar::Mark_Enclosing:
        return true;
    default:
        return false;
    }
}

/// a character and the glyphs that go with it: its marks, the rest of its cluster
struct Unit {
    int first = 0;
    int count = 0;
    Type type = N;
    int level = 0;
};
}

bool FMBidi::isRightToLeft(const GlyphList &paragraph)
{
    for (const RenderedGlyph &g : paragraph) {
        const Type t(typeOf(g.lChar));
        if (t == L)
            return false;
        if (t == R || t == AL)
            return true;
    }
    return false;
}

void FMBidi::toVisualOrder(GlyphList &line, bool rightToLeftParagraph)
{
    if (line.isEmpty())
        return;

    QList<Unit> units;
    for (int i(0); i < line.count(); ++i) {
        const RenderedGlyph &g(line.at(i));
        const bool continues(i > 0 && (isMark(g.lChar) || (g.lChar != 0 && g.lChar == line.at(i - 1).lChar && g.log == line.at(i - 1).log)));
        if (continues && !units.isEmpty()) {
            ++units.last().count;
            continue;
        }
        Unit u;
        u.first = i;
        u.count = 1;
        u.type = typeOf(g.lChar);
        units << u;
    }

    // nothing right-to-left in a left-to-right paragraph: nothing to do
    if (!rightToLeftParagraph && std::none_of(units.cbegin(), units.cend(), [](const Unit &u) {
            return u.type == R || u.type == AL || u.type == AN;
        }))
        return;

    const Type sos(rightToLeftParagraph ? R : L);
    const int n(units.count());
    const auto type = [&units](int i) -> Type & {
        return units[i].type;
    };

    // W1: a mark with nothing to sit on takes the direction of the paragraph
    for (int i(0); i < n; ++i)
        if (type(i) == NSM)
            type(i) = i ? type(i - 1) : sos;
    // W2: a European number after Arabic letters is an Arabic number
    for (int i(0); i < n; ++i) {
        if (type(i) != EN)
            continue;
        for (int j(i - 1); j >= 0; --j) {
            if (type(j) == R || type(j) == L || type(j) == AL) {
                if (type(j) == AL)
                    type(i) = AN;
                break;
            }
        }
    }
    // W3
    for (int i(0); i < n; ++i)
        if (type(i) == AL)
            type(i) = R;
    // W4: one separator between two numbers of a kind belongs to them
    for (int i(1); i + 1 < n; ++i) {
        if (type(i) == ES && type(i - 1) == EN && type(i + 1) == EN)
            type(i) = EN;
        else if (type(i) == CS && type(i - 1) == type(i + 1) && (type(i - 1) == EN || type(i - 1) == AN))
            type(i) = type(i - 1);
    }
    // W5: terminators ("%", "€") next to a European number are part of it
    for (int i(0); i < n; ++i) {
        if (type(i) != ET)
            continue;
        int j(i);
        while (j < n && type(j) == ET)
            ++j;
        const bool nextToNumber((i > 0 && type(i - 1) == EN) || (j < n && type(j) == EN));
        for (int k(i); k < j; ++k)
            type(k) = nextToNumber ? EN : ET;
        i = j - 1;
    }
    // W6
    for (int i(0); i < n; ++i)
        if (type(i) == ES || type(i) == ET || type(i) == CS)
            type(i) = N;
    // W7: a European number in left-to-right text is left-to-right
    for (int i(0); i < n; ++i) {
        if (type(i) != EN)
            continue;
        Type strong(sos);
        for (int j(i - 1); j >= 0; --j) {
            if (type(j) == L || type(j) == R) {
                strong = type(j);
                break;
            }
        }
        if (strong == L)
            type(i) = L;
    }
    // N1–N2: neutrals take the direction around them when it is one, else the paragraph's
    const auto strongOf = [](Type t) {
        return (t == EN || t == AN) ? R : t;
    };
    for (int i(0); i < n; ++i) {
        if (type(i) != N)
            continue;
        int j(i);
        while (j < n && type(j) == N)
            ++j;
        const Type before(i > 0 ? strongOf(type(i - 1)) : sos);
        const Type after(j < n ? strongOf(type(j)) : sos);
        const Type resolved(before == after ? before : sos);
        for (int k(i); k < j; ++k)
            type(k) = resolved;
        i = j - 1;
    }
    // I1–I2
    const int base(rightToLeftParagraph ? 1 : 0);
    int maxLevel(base);
    for (Unit &u : units) {
        if (base == 0)
            u.level = u.type == R ? 1 : (u.type == AN || u.type == EN) ? 2 : 0;
        else
            u.level = u.type == R ? 1 : 2;
        maxLevel = std::max(maxLevel, u.level);
    }

    // L2: from the highest level down to the lowest odd one, turn around every run at least that high
    QList<int> order(n);
    for (int i(0); i < n; ++i)
        order[i] = i;
    for (int level(maxLevel); level >= 1; --level) {
        for (int i(0); i < n; ++i) {
            if (units.at(order.at(i)).level < level)
                continue;
            int j(i);
            while (j < n && units.at(order.at(j)).level >= level)
                ++j;
            std::reverse(order.begin() + i, order.begin() + j);
            i = j - 1;
        }
    }

    GlyphList visual;
    visual.reserve(line.count());
    for (int ui : std::as_const(order)) {
        const Unit &u(units.at(ui));
        // Inside a right-to-left run a cluster turns around too: its marks come before
        // the letter they sit on, which is the order right-to-left fonts are drawn for
        // (a Hebrew point has no advance and lies right of the pen) and the one
        // HarfBuzz positions them for.
        const bool rtl(u.level % 2);
        for (int k(0); k < u.count; ++k) {
            RenderedGlyph glyph(line.at(rtl ? u.first + u.count - 1 - k : u.first + k));
            // L4: in a right-to-left run a bracket faces the other way
            if (rtl && glyph.mirrorGlyph)
                glyph.glyph = glyph.mirrorGlyph;
            visual << glyph;
        }
    }
    line = visual;
}

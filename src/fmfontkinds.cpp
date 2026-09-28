/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmfontkinds.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"

#include <QElapsedTimer>

QHash<int, QList<FontItem *>> FMFontKinds::m_fonts;

void FMFontKinds::invalidate()
{
    m_fonts.clear();
}

QList<FontItem *> FMFontKinds::fonts(Kind kind)
{
    auto it = m_fonts.constFind(int(kind));
    if (it != m_fonts.constEnd())
        return it.value();

    QElapsedTimer timer;
    timer.start();
    // both answers come from one opening of the face, so both lists are made at once
    QList<FontItem *> variable;
    QList<FontItem *> colour;
    const QList<FontItem *> all(FMFontDb::DB()->AllFonts());
    for (FontItem *fit : all) {
        if (fit->isRemote() && !fit->isCached())
            continue;
        if (fit->isVariable())
            variable << fit;
        if (fit->hasColor())
            colour << fit;
    }
    qCDebug(FONTMATRIX_LOG) << "font kinds:" << variable.size() << "variable," << colour.size() << "colour, of" << all.size() << "in" << timer.elapsed()
                            << "ms";
    m_fonts.insert(int(Variable), variable);
    m_fonts.insert(int(Colour), colour);
    return m_fonts.value(int(kind));
}

int FMFontKinds::count(Kind kind)
{
    return int(fonts(kind).size());
}

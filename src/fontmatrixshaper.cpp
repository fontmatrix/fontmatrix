/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fontmatrixshaper.h"

FontmatrixShaper::FontmatrixShaper(FMOtf *o, QString s)
    : FMBaseShaper(o, s)
{
    fmos = new FMOwnShaper(script);
}

FontmatrixShaper::~FontmatrixShaper()
{
    if (fmos)
        delete fmos;
}

GlyphList FontmatrixShaper::doShape(const QString &s)
{
    fmos->fillIn(s);
    QList<Character> shaped(fmos->GetShaped());
    return otf->procstring(shaped, script);
}

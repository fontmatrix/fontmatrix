/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "harfbuzzqtshaper.h"

HarfbuzzShaper::HarfbuzzShaper(FMOtf *o, QString s)
    : FMBaseShaper(o, s)
{
    hbqtsh = new FMShaper(otf);
    hbqtsh->setScript(script);
}

HarfbuzzShaper::~HarfbuzzShaper()
{
    if (hbqtsh)
        delete hbqtsh;
}

GlyphList HarfbuzzShaper::doShape(const QString &s)
{
    return hbqtsh->doShape(s, true);
}

/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FONTMATRIXSHAPER_H
#define FONTMATRIXSHAPER_H

#include "fmbaseshaper.h"
#include "fmshaper_own.h"

class FontmatrixShaper : public FMBaseShaper
{
public:
    FontmatrixShaper(FMOtf *o, QString s);
    ~FontmatrixShaper() override;

    GlyphList doShape(const QString &s) override;

private:
    Q_DISABLE_COPY(FontmatrixShaper)
    FMOwnShaper *fmos = nullptr;
};

#endif

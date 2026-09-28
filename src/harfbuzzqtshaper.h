/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HARFBUZZSHAPER_H
#define HARFBUZZSHAPER_H

#include "fmbaseshaper.h"
#include "fmshaper.h"

class HarfbuzzShaper : public FMBaseShaper
{
public:
    HarfbuzzShaper(FMOtf *o, QString s);
    ~HarfbuzzShaper() override;

    GlyphList doShape(const QString &s) override;

private:
    Q_DISABLE_COPY(HarfbuzzShaper)
    FMShaper *hbqtsh = nullptr;
};

#endif

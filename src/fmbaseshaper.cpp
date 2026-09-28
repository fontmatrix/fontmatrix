/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmbaseshaper.h"
#include "fontmatrix_debug.h"

#include "fontmatrixshaper.h"

#include "harfbuzzqtshaper.h"

#include <QDebug>

FMShaperFactory::FMShaperFactory(FMOtf *o, QString s, SHAPER_TYPE st)
    : shaperType(st)
    , otf(o)
    , script(s)
    , shaperImpl(nullptr)
{
    switch (shaperType) {
    case FONTMATRIX:
        qCDebug(FONTMATRIX_LOG) << "NEW FontmatrixShaper";
        shaperImpl = new FontmatrixShaper(otf, script);
        break;
    case HARFBUZZ:
        qCDebug(FONTMATRIX_LOG) << "NEW HarfbuzzShaper";
        shaperImpl = new HarfbuzzShaper(otf, script);
        break;
    default:
        break;
    }
}

FMShaperFactory::~FMShaperFactory()
{
    if (shaperImpl)
        delete shaperImpl;
}

// void FMShaperFactory::resetShaperType ( SHAPER_TYPE st )
// {
// 	if ( shaperType == st )
// 		return;
//
// 	if ( shaperImpl )
// 	{
// 		delete shaperImpl;
// 		shaperImpl = 0;
// 	}
//
// 	shaperType = st;
// }

GlyphList FMShaperFactory::doShape(const QString &aString)
{
    return shaperImpl->doShape(aString);
}

FMBaseShaper::FMBaseShaper(FMOtf *o, QString s)
    : otf(o)
    , script(s)
{
}

FMBaseShaper::~FMBaseShaper() = default;

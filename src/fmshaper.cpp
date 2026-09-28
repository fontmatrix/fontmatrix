/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmshaper.h"
#include "fontmatrix_debug.h"

FMShaper::FMShaper(FMOtf *anchor)
    : anchorOTF(anchor)
{
    qCDebug(FONTMATRIX_LOG) << "FMShaper " << this << " created";
}

FMShaper::~FMShaper()
{
    qCDebug(FONTMATRIX_LOG) << "FMShaper " << this << " destroyed";
}

bool FMShaper::setScript(QString script)
{
    m_script = script;
    return !m_script.isEmpty();
}

QList<RenderedGlyph> FMShaper::doShape(QString string, bool ltr)
{
    qCDebug(FONTMATRIX_LOG) << "FMShaper::doShape(" << string << "," << ltr << ")";
    return anchorOTF->shape(string, m_script, ltr);
}

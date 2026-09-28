/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filtertag.h"
#include "filteritem.h"
#include "fmfontdb.h"

FilterTag::FilterTag()

{
}

QString FilterTag::type() const
{
    return QStringLiteral("Tag");
}

void FilterTag::operate()
{
    QString key(vData.value(Key).toString());
    QString tag(vData.value(Tag).toString());

    if (key == QLatin1String("TAG")) // regular tag
    {
        operateFilter(FMFontDb::DB()->Fonts(tag, FMFontDb::Tags));
    } else if (key == QLatin1String("ALL_ACTIVATED")) {
        operateFilter(FMFontDb::DB()->Fonts(1, FMFontDb::Activation));
    }
}

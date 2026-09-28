/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filterkind.h"
#include "fmfontkinds.h"

FilterKind::FilterKind()
{
}

QString FilterKind::type() const
{
    return QStringLiteral("Kind");
}

void FilterKind::operate()
{
    operateFilter(FMFontKinds::fonts(static_cast<FMFontKinds::Kind>(vData.value(Kind).toInt())));
}

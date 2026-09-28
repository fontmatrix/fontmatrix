/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filterlang.h"
#include "fmlangcoverage.h"

FilterLang::FilterLang()
{
}

QString FilterLang::type() const
{
    return QStringLiteral("Lang");
}

void FilterLang::operate()
{
    operateFilter(FMLangCoverage::fonts(vData.value(Language).toString()));
}

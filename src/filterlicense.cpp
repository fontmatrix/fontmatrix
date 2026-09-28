/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filterlicense.h"
#include "fmlicense.h"

FilterLicense::FilterLicense()
{
}

QString FilterLicense::type() const
{
    return QStringLiteral("License");
}

void FilterLicense::operate()
{
    operateFilter(FMLicense::fonts(static_cast<FMLicense::Family>(vData.value(License).toInt())));
}

/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filterduplicate.h"
#include "fmduplicates.h"

FilterDuplicate::FilterDuplicate()
{
}

QString FilterDuplicate::type() const
{
    return QStringLiteral("Duplicate");
}

void FilterDuplicate::operate()
{
    operateFilter(FMDuplicates::fonts(static_cast<FMDuplicates::Kind>(vData.value(Kind).toInt())));
}

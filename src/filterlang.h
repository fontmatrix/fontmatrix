/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERLANG_H
#define FILTERLANG_H

#include "filterdata.h"

/// keeps the fonts that can set a language (FMLangCoverage)
class FilterLang : public FilterData
{
public:
    FilterLang();

    enum LangIndex {
        Language = FilterData::UserIndex
    };

    [[nodiscard]] QString type() const override;
    void operate() override;
};

#endif // FILTERLANG_H

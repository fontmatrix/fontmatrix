/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERLICENSE_H
#define FILTERLICENSE_H

#include "filterdata.h"

/// keeps the fonts under one licence (FMLicense)
class FilterLicense : public FilterData
{
public:
    FilterLicense();

    enum LicenseIndex {
        License = FilterData::UserIndex
    };

    [[nodiscard]] QString type() const override;
    void operate() override;
};

#endif // FILTERLICENSE_H

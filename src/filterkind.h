/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERKIND_H
#define FILTERKIND_H

#include "filterdata.h"

/// keeps the variable fonts or the colour fonts (FMFontKinds)
class FilterKind : public FilterData
{
public:
    FilterKind();

    enum KindIndex {
        Kind = FilterData::UserIndex
    };

    [[nodiscard]] QString type() const override;
    void operate() override;
};

#endif // FILTERKIND_H

/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERDUPLICATE_H
#define FILTERDUPLICATE_H

#include "filterdata.h"

/// keeps the fonts the collection holds more than once (FMDuplicates)
class FilterDuplicate : public FilterData
{
public:
    FilterDuplicate();

    enum DuplicateIndex {
        Kind = FilterData::UserIndex
    };

    [[nodiscard]] QString type() const override;
    void operate() override;
};

#endif // FILTERDUPLICATE_H

/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERTAG_H
#define FILTERTAG_H

#include "filterdata.h"

class FilterTag : public FilterData
{
public:
    FilterTag();

    enum TagIndex {
        Key = FilterData::UserIndex,
        Tag
    };

    [[nodiscard]] QString type() const override;
    void operate() override;
};

#endif // FILTERTAG_H

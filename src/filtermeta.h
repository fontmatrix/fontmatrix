/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERMETA_H
#define FILTERMETA_H

#include "filterdata.h"

constexpr int FILTER_FIELD_SPECIAL_UNICODE = 2001;

class FilterMeta : public FilterData
{
public:
    FilterMeta();

    enum TagIndex {
        Field = FilterData::UserIndex,
        Value
    };

    [[nodiscard]] QString type() const override;
    void operate() override;
};

#endif // FILTERMETA_H

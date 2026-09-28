/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILTERPANOSE_H
#define FILTERPANOSE_H

#include "filterdata.h"

class FilterPanose : public FilterData
{
public:
    FilterPanose();

    enum TagIndex {
        Param = FilterData::UserIndex,
        Value
    };

    [[nodiscard]] QString type() const override;
    void operate() override;
};

#endif // FILTERPANOSE_H

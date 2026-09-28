/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filterpanose.h"
#include "fmfontdb.h"

FilterPanose::FilterPanose()

{
}

QString FilterPanose::type() const
{
    return QStringLiteral("Panose");
}

void FilterPanose::operate()
{
    QList<FontDBResult> dbresult(FMFontDb::DB()->getValues(FMFontDb::Panose));
    QList<FontItem *> fil;
    int paramIdx(vData.value(Param).toInt());
    int val(vData.value(Value).toInt());
    int fv(0);
    for (int i(0); i < dbresult.count(); ++i) {
        QStringList pl(dbresult[i].second.split(QStringLiteral(":")));
        fv = pl[paramIdx].toInt();
        if (fv == val)
            fil << dbresult[i].first;
    }
    operateFilter(fil);
}

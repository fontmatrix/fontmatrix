/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filtermeta.h"
#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "fontitem.h"

FilterMeta::FilterMeta()

{
}

QString FilterMeta::type() const
{
    return QStringLiteral("Meta");
}

void FilterMeta::operate()
{
    QString fs(vData.value(Value).toString());
    int ff(vData.value(Field).toInt());

    if (ff == FILTER_FIELD_SPECIAL_UNICODE) // Unicode
    {
        QList<FontItem *> allList = FMFontDb::DB()->AllFonts();
        QList<FontItem *> tl;
        int startC(0xFFFFFFFF);
        int endC(0);
        int patCount(fs.size());
        for (int a(0); a < patCount; ++a) {
            int ca(fs[a].unicode());
            if (ca < startC)
                startC = ca;
            if (ca > endC)
                endC = ca;
        }

        int superSetCount(allList.size());
        for (int i = 0; i < superSetCount; ++i) {
            int cc(allList[i]->countCoverage(startC, endC));
            if (cc >= patCount) {
                tl.append(allList[i]);
            }
        }

        operateFilter(tl);
    } else if (ff == FMFontDb::AllInfo) {
        FMFontDb::InfoItem k;
        QList<FontItem *> tmpList;
        tmpList.clear();
        const auto infoNames(FontStrings::Names().keys());
        for (int gIdx(0); gIdx < infoNames.size(); ++gIdx) {
            k = infoNames.at(gIdx);
            if (k != FMFontDb::AllInfo) {
                tmpList += FMFontDb::DB()->Fonts(fs, k);
            }
        }

        operateFilter(tmpList);

    } else {
        operateFilter(FMFontDb::DB()->Fonts(fs, FMFontDb::InfoItem(ff)));
    }
};

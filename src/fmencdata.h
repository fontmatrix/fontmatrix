/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMENCDATA_H
#define FMENCDATA_H

#include <QMap>
#include <QPair>
#include <QString>

class FMEncData
{
    static FMEncData *instance;
    FMEncData();
    static FMEncData *that();

    QMap<int, QString> langIdMap;
    void fillLangIdMap();

    QMap<int, QPair<int, int>> os2URangeMap;
    void fillOs2URAnges();

public:
    static const QMap<int, QString> &LangIdMap();
    static const QMap<int, QPair<int, int>> &Os2URanges();
};

#endif // FMENCDATA_H

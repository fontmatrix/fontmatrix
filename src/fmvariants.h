/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMVARIANTS_H
#define FMVARIANTS_H

#include <QList>
#include <QStringList>

class FontItem;

class FMVariants
{
    static FMVariants *instance;
    FMVariants();

    QList<QStringList> variants;
    QStringList priorList;
    void appendVariants(const QString &w, const QString &s, const QString &wi, const QString &o);
    inline bool compareVariants(const QStringList &a, const QStringList &b);

public:
    static QList<FontItem *> Order(QList<FontItem *> ul);
    static FontItem *Preferred(QList<FontItem *> ul);
};

#endif // FMVARIANTS_H

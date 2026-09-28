/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmvariants.h"

#include "fontitem.h"

FMVariants *FMVariants::instance = nullptr;

FMVariants::FMVariants()
{
    // init sorted variant names
    variants.clear();
    QStringList weight;
    weight << QStringLiteral("Hairline") << QStringLiteral("Thin") << QStringLiteral("UltraLight") << QStringLiteral("ExtraLight") << QStringLiteral("Light")
           << QStringLiteral("Book") << QStringLiteral("Normal") << QStringLiteral("Regular") << QStringLiteral("Roman") << QStringLiteral("Plain")
           << QStringLiteral("Medium") << QString() << QStringLiteral("Demi") << QStringLiteral("DemiBold") << QStringLiteral("SemiBold")
           << QStringLiteral("Bold") << QStringLiteral("ExtraBold") << QStringLiteral("Extra") << QStringLiteral("Heavy") << QStringLiteral("Black")
           << QStringLiteral("ExtraBlack") << QStringLiteral("UltraBlack") << QStringLiteral("Ultra");

    QStringList slope;
    slope << QString() << QStringLiteral("Italic") << QStringLiteral("Oblique") << QStringLiteral("Slanted");

    QStringList width;
    width << QString() << QStringLiteral("UltraCompressed") << QStringLiteral("Compressed") << QStringLiteral("UltraCondensed") << QStringLiteral("Condensed")
          << QStringLiteral("SemiCondensed") << QStringLiteral("Narrow") << QStringLiteral("SemiExtended") << QStringLiteral("SemiExpanded")
          << QStringLiteral("Extended") << QStringLiteral("Expanded") << QStringLiteral("ExtraExtended") << QStringLiteral("ExtraExpanded");

    QStringList optical;
    optical << QString() << QStringLiteral("Poster") << QStringLiteral("Display") << QStringLiteral("SubHead") << QStringLiteral("SmallText")
            << QStringLiteral("Caption");

    for (const auto &w : std::as_const(weight)) {
        for (const auto &s : std::as_const(slope)) {
            for (const auto &wi : std::as_const(width)) {
                for (const auto &o : std::as_const(optical)) {
                    appendVariants(w, s, wi, o);
                }
            }
        }
    }
    //	for (const auto& v : variants)
    //		qDebug()<<v.join(" ");

    priorList << QStringLiteral("Book") << QStringLiteral("Normal") << QStringLiteral("Regular") << QStringLiteral("Roman") << QStringLiteral("Plain")
              << QStringLiteral("Medium");
}

void FMVariants::appendVariants(const QString &w, const QString &s, const QString &wi, const QString &o)
{
    QStringList p;
    p << w << s << wi << o;
    QStringList l;
    for (const auto &s : std::as_const(p)) {
        if (!s.isEmpty())
            l << s;
    }
    variants << l;
}

bool FMVariants::compareVariants(const QStringList &a, const QStringList &b)
{
    for (const auto &va : a) {
        if (!b.contains(va, Qt::CaseInsensitive))
            return false;
    }
    for (const auto &vb : b) {
        if (!a.contains(vb, Qt::CaseInsensitive))
            return false;
    }
    return true;
}

QList<FontItem *> FMVariants::Order(QList<FontItem *> ul)
{
    if (instance == nullptr)
        instance = new FMVariants;

    FMVariants *vs(instance);

    QList<FontItem *> ret;
    // Not a QHash: fonts of the same variant come out in the order of this
    // map, and a hash is seeded differently on each run.
    QMap<FontItem *, QStringList> fl;
    for (auto *f : ul) {
        fl.insert(f, f->variant().split(QStringLiteral(" ")));
    }
    for (const auto &v : std::as_const(vs->variants)) {
        for (const auto flKeys = fl.keys(); auto *f : flKeys) {
            if (vs->compareVariants(v, fl[f])) {
                ret.append(f);
                fl.remove(f);
            }
        }
    }
    if (fl.count() > 0) {
        // for Univers-like fonts, we get the number key
        QMap<int, QMap<QString, FontItem *>> ulikeFonts;
        bool intOK(false);
        for (const auto flKeysList = fl.keys(); auto *f : flKeysList) {
            intOK = false;
            QString fs(fl[f].first());
            int idx(fs.toInt(&intOK, 10));
            if (intOK) {
                ulikeFonts[idx][f->variant()] = f;
                fl.remove(f);
            }
        }
        for (const auto ulikeFontsKeys = ulikeFonts.keys(); const auto &k : ulikeFontsKeys) {
            for (const auto ulikeFontsKeysList = ulikeFonts[k].keys(); const auto &v : ulikeFontsKeysList)
                ret << ulikeFonts[k][v];
        }

        // still fonts unsorted;
        if (fl.count() > 0) {
            QMap<QString, FontItem *> lastChance;
            for (const auto loopFlKeys = fl.keys(); auto *f : loopFlKeys) {
                lastChance[f->variant()] = f;
            }
            for (const auto lastChanceKeys = lastChance.keys(); const auto &v : lastChanceKeys)
                ret << lastChance[v];
        }
    }
    return ret;
}

FontItem *FMVariants::Preferred(QList<FontItem *> ul)
{
    if (ul.isEmpty())
        return nullptr;
    if (instance == nullptr)
        instance = new FMVariants;
    for (auto *it : ul) {
        if (instance->priorList.contains(it->variant(), Qt::CaseInsensitive)) {
            return it;
        }
    }
    return ul.first();
};

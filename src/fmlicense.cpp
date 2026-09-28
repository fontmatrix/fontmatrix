/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmlicense.h"
#include "fmfontdb.h"
#include "fontitem.h"

#include <KLocalizedString>

#include <QMap>

QHash<FontItem *, FMLicense::Family> FMLicense::m_byFont;
bool FMLicense::m_read = false;

void FMLicense::invalidate()
{
    m_byFont.clear();
    m_read = false;
}

FMLicense::Family FMLicense::classify(const QString &text)
{
    if (text.isEmpty())
        return None;

    struct Marker {
        Family family;
        const char *needle;
    };
    // the order matters: what is looked for first wins, so the narrower
    // sentence of a licence comes before the word it shares with another
    static const Marker markers[] = {
        {OFL, "open font license"},
        {OFL, "openfontlicense.org"},
        {OFL, "scripts.sil.org/ofl"},
        {OFL, "sil open font"},
        {UFL, "ubuntu font licence"},
        {UFL, "ubuntu font license"},
        {Apache, "apache license"},
        {Apache, "apache.org/licenses"},
        {LGPL, "lesser general public license"},
        {LGPL, "gnu lgpl"},
        {LGPL, "lgpl"},
        {GPL, "general public license"},
        {GPL, "gnu.org/licenses/gpl"},
        {GPL, "gnu gpl"},
        {CC0, "creativecommons.org/publicdomain/zero"},
        {CC0, "cc0"},
        {CCBYSA, "creativecommons.org/licenses/by-sa"},
        {CCBYSA, "attribution-sharealike"},
        {CCBYSA, "cc-by-sa"},
        {CCBYND, "creativecommons.org/licenses/by-nd"},
        {CCBYND, "attribution-noderivs"},
        {CCBY, "creativecommons.org/licenses/by"},
        {CCBY, "cc-by"},
        {CCBY, "creative commons attribution"},
        {MIT, "mit license"},
        {MIT, "opensource.org/licenses/mit"},
        {MIT, "permission is hereby granted, free of charge"},
        {BSD, "bsd license"},
        {BSD, "redistribution and use in source and binary forms"},
        {PublicDomain, "public domain"},
        {PublicDomain, "unlicense"},
        {Freeware, "freeware"},
        {Freeware, "free for personal use"},
        {Freeware, "free for non-commercial"},
    };
    for (const Marker &marker : markers) {
        if (text.contains(QLatin1String(marker.needle)))
            return marker.family;
    }

    // nothing granted, everything kept: the usual wording of a font that is sold
    static const char *const reserved[] = {
        "all rights reserved",
        "may not be copied",
        "may not be modified",
        "may not be reproduced",
        "proprietary",
        "unauthorized use",
        "unauthorised use",
        "is protected under",
        "is the property of",
        "without written permission",
        "without the prior written",
        "end user license agreement",
        "eula",
    };
    for (const char *needle : reserved) {
        if (text.contains(QLatin1String(needle)))
            return Proprietary;
    }
    return Other;
}

void FMLicense::fill()
{
    if (m_read)
        return;
    m_read = true;

    // what the name table said, whichever platform and language it said it in
    QMap<FontItem *, QString> texts;
    for (const FMFontDb::InfoItem key : {FMFontDb::LicenseDescription, FMFontDb::LicenseInfoURL}) {
        const QList<FontDBResult> rows(FMFontDb::DB()->getInfo(QList<FontItem *>(), key, -1));
        for (const FontDBResult &row : rows) {
            if (!row.first || row.second.trimmed().isEmpty())
                continue;
            QString &text = texts[row.first];
            text += QLatin1Char(' ') + row.second.toLower();
        }
    }

    const QList<FontItem *> fonts(FMFontDb::DB()->AllFonts());
    for (FontItem *fit : fonts)
        m_byFont.insert(fit, classify(texts.value(fit).simplified()));
}

FMLicense::Family FMLicense::of(FontItem *fit)
{
    if (!fit)
        return None;
    fill();
    auto it = m_byFont.constFind(fit);
    if (it != m_byFont.constEnd())
        return it.value();
    // imported since the database was read: its own name table answers
    const FontInfoMap info(fit->moreInfo());
    QString text;
    for (auto lang = info.constBegin(); lang != info.constEnd(); ++lang) {
        text += QLatin1Char(' ') + lang.value().value(FMFontDb::LicenseDescription).toLower();
        text += QLatin1Char(' ') + lang.value().value(FMFontDb::LicenseInfoURL).toLower();
    }
    const Family family = classify(text.simplified());
    m_byFont.insert(fit, family);
    return family;
}

QString FMLicense::name(Family family)
{
    switch (family) {
    case None:
        return i18nc("@item:inlistbox a font that says nothing about its licence", "No licence information");
    case Other:
        return i18nc("@item:inlistbox a licence that is not one of those Fontmatrix knows", "Another licence");
    case OFL:
        return i18nc("@item:inlistbox font licence", "SIL Open Font License");
    case Apache:
        return i18nc("@item:inlistbox font licence", "Apache License");
    case GPL:
        return i18nc("@item:inlistbox font licence", "GNU General Public License");
    case LGPL:
        return i18nc("@item:inlistbox font licence", "GNU Lesser General Public License");
    case MIT:
        return i18nc("@item:inlistbox font licence", "MIT License");
    case BSD:
        return i18nc("@item:inlistbox font licence", "BSD License");
    case CC0:
        return i18nc("@item:inlistbox font licence", "Creative Commons Zero");
    case CCBY:
        return i18nc("@item:inlistbox font licence", "Creative Commons Attribution");
    case CCBYSA:
        return i18nc("@item:inlistbox font licence", "Creative Commons Attribution-ShareAlike");
    case CCBYND:
        return i18nc("@item:inlistbox font licence", "Creative Commons Attribution-NoDerivatives");
    case UFL:
        return i18nc("@item:inlistbox font licence", "Ubuntu Font Licence");
    case PublicDomain:
        return i18nc("@item:inlistbox font licence", "Public domain");
    case Freeware:
        return i18nc("@item:inlistbox font licence: free to use, not to change or sell", "Free of charge");
    case Proprietary:
        return i18nc("@item:inlistbox font licence: all rights reserved, nothing granted", "All rights reserved");
    }
    return QString();
}

QList<FMLicense::Family> FMLicense::families()
{
    fill();
    QMap<Family, int> counted;
    for (auto it = m_byFont.constBegin(); it != m_byFont.constEnd(); ++it)
        counted[it.value()] += 1;
    QList<Family> ret(counted.keyBegin(), counted.keyEnd());
    // the commonest licence of the collection first, so the list opens on what matters
    std::sort(ret.begin(), ret.end(), [&counted](Family a, Family b) {
        return counted.value(a) != counted.value(b) ? counted.value(a) > counted.value(b) : a < b;
    });
    return ret;
}

QList<FontItem *> FMLicense::fonts(Family family)
{
    fill();
    QList<FontItem *> ret;
    const QList<FontItem *> fonts(FMFontDb::DB()->AllFonts());
    for (FontItem *fit : fonts) {
        if (of(fit) == family)
            ret << fit;
    }
    return ret;
}

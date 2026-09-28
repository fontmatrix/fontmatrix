/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmdumpinfo.h"

#include "fmfontdb.h"
#include "fontitem.h"

#include <QFile>
#include <QTextStream>

FMDumpInfo::FMDumpInfo(FontItem *font, const QString &model)
    : m_font(font)
    , m_model(model)
{
    m_info[QStringLiteral("Family")] = m_font->family();
    m_info[QStringLiteral("Variant")] = m_font->variant();

    QMap<int, QString> m_name;
    m_name[FMFontDb::Copyright] = QLatin1String("Copyright");
    m_name[FMFontDb::FontFamily] = QLatin1String("Font_Family");
    m_name[FMFontDb::FontSubfamily] = QLatin1String("Font_Subfamily");
    m_name[FMFontDb::UniqueFontIdentifier] = QLatin1String("Unique_font_identifier");
    m_name[FMFontDb::FullFontName] = QLatin1String("Full_font_name");
    m_name[FMFontDb::VersionString] = QLatin1String("Version_string");
    m_name[FMFontDb::PostscriptName] = QLatin1String("Postscript_name");
    m_name[FMFontDb::Trademark] = QLatin1String("Trademark");
    m_name[FMFontDb::ManufacturerName] = QLatin1String("Manufacturer");
    m_name[FMFontDb::Designer] = QLatin1String("Designer");
    m_name[FMFontDb::Description] = QLatin1String("Description");
    m_name[FMFontDb::URLVendor] = QLatin1String("URL_Vendor");
    m_name[FMFontDb::URLDesigner] = QLatin1String("URL_Designer");
    m_name[FMFontDb::LicenseDescription] = QLatin1String("License_Description");
    m_name[FMFontDb::LicenseInfoURL] = QLatin1String("License_Info_URL");
    m_name[FMFontDb::PreferredFamily] = QLatin1String("Preferred_Family");
    m_name[FMFontDb::PreferredSubfamily] = QLatin1String("Preferred_Subfamily");
    m_name[FMFontDb::CompatibleMacintosh] = QLatin1String("Compatible_Full");
    m_name[FMFontDb::SampleText] = QLatin1String("Sample_text");
    m_name[FMFontDb::PostScriptCIDName] = QLatin1String("PostScript_CID");
    FontInfoMap fim(m_font->rawInfo());

    /*
    hierarchy ;-)
    langIdMap[0x0000]="DEFAULT";
    langIdMap[0x0009]="ENGLISH_GENERAL";
    langIdMap[0x0409]="ENGLISH_UNITED_STATES";
    langIdMap[0x0809]="ENGLISH_UNITED_KINGDOM";
    */
    QList<int> llist;
    llist << 0x0000 << 0x0009 << 0x0409 << 0x0809;
    for (const auto &langid : std::as_const(llist)) {
        if (fim.contains(langid)) // DEFAULT - generally means english in fact, which is good for our purpose.
        {
            for (const auto fimKeys = fim[langid].keys(); const auto &key : fimKeys) {
                if (!m_info.contains(m_name[key]))
                    m_info[m_name[key]] = fim[langid][key];
            }
        }
    }
}

FMDumpInfo::~FMDumpInfo() = default;

bool FMDumpInfo::dumpInfo(const QString &filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QTextStream ts(&file);
    QString re(m_model);

    for (const auto m_infoKeys = m_info.keys(); const auto &key : m_infoKeys) {
        re.replace(QLatin1String("${") + key + QLatin1String("}"), m_info[key]);
    }
    ts << re;
    file.close();
    return true;
}

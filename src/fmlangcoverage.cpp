/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmlangcoverage.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"

#include <QFileInfo>
#include <QLocale>
#include <QSet>

#ifdef HAVE_FONTCONFIG
// clang-format off: fontconfig.h has to come first
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
// clang-format on
#endif

QHash<QString, QStringList> FMLangCoverage::m_byFile;
bool FMLangCoverage::m_cacheRead = false;

bool FMLangCoverage::isAvailable()
{
#ifdef HAVE_FONTCONFIG
    return true;
#else
    return false;
#endif
}

void FMLangCoverage::invalidate()
{
    m_byFile.clear();
    m_cacheRead = false;
}

#ifdef HAVE_FONTCONFIG

namespace
{
/// the languages of a fontconfig pattern
QStringList langsOfPattern(FcPattern *pattern)
{
    QStringList ret;
    FcLangSet *langSet = nullptr;
    if (FcPatternGetLangSet(pattern, FC_LANG, 0, &langSet) != FcResultMatch || !langSet)
        return ret;
    FcStrSet *langs = FcLangSetGetLangs(langSet);
    if (!langs)
        return ret;
    FcStrList *list = FcStrListCreate(langs);
    if (list) {
        while (FcChar8 *lang = FcStrListNext(list))
            ret << QString::fromUtf8(reinterpret_cast<const char *>(lang));
        FcStrListDone(list);
    }
    FcStrSetDestroy(langs);
    ret.sort();
    return ret;
}
}

void FMLangCoverage::fillFromCache()
{
    if (m_cacheRead)
        return;
    m_cacheRead = true;
    // every font fontconfig has in its cache, with its languages: one call, no file read
    FcPattern *pattern = FcPatternCreate();
    FcObjectSet *objects = FcObjectSetBuild(FC_FILE, FC_LANG, nullptr);
    FcFontSet *fonts = (pattern && objects) ? FcFontList(nullptr, pattern, objects) : nullptr;
    if (fonts) {
        for (int i = 0; i < fonts->nfont; ++i) {
            FcChar8 *file = nullptr;
            if (FcPatternGetString(fonts->fonts[i], FC_FILE, 0, &file) != FcResultMatch || !file)
                continue;
            const QString path(QString::fromUtf8(reinterpret_cast<const char *>(file)));
            if (!m_byFile.contains(path))
                m_byFile.insert(path, langsOfPattern(fonts->fonts[i]));
        }
        qCDebug(FONTMATRIX_LOG) << "language coverage:" << fonts->nfont << "fonts from the fontconfig cache";
        FcFontSetDestroy(fonts);
    }
    if (objects)
        FcObjectSetDestroy(objects);
    if (pattern)
        FcPatternDestroy(pattern);
}

QStringList FMLangCoverage::read(const QString &file)
{
    // a font of a directory fontconfig does not scan: its own file answers
    int count = 0;
    FcPattern *pattern = FcFreeTypeQuery(reinterpret_cast<const FcChar8 *>(QFile::encodeName(file).constData()), 0, nullptr, &count);
    if (!pattern)
        return QStringList();
    const QStringList ret(langsOfPattern(pattern));
    FcPatternDestroy(pattern);
    return ret;
}

QStringList FMLangCoverage::languagesOf(FontItem *fit)
{
    if (!fit)
        return QStringList();
    // a remote font whose file is not here yet has nothing to read
    if (fit->isRemote() && !fit->isCached())
        return QStringList();
    fillFromCache();
    const QString file(fit->localPath());
    auto it = m_byFile.constFind(file);
    if (it != m_byFile.constEnd())
        return it.value();
    const QStringList langs(read(file));
    m_byFile.insert(file, langs);
    return langs;
}

#else // no fontconfig: nothing is known

void FMLangCoverage::fillFromCache()
{
}

QStringList FMLangCoverage::read(const QString &)
{
    return QStringList();
}

QStringList FMLangCoverage::languagesOf(FontItem *)
{
    return QStringList();
}

#endif

QStringList FMLangCoverage::languages()
{
    QSet<QString> all;
    const QList<FontItem *> fonts(FMFontDb::DB()->AllFonts());
    for (FontItem *fit : fonts) {
        const QStringList langs(languagesOf(fit));
        for (const QString &lang : langs)
            all.insert(lang);
    }
    QStringList ret(all.constBegin(), all.constEnd());
    std::sort(ret.begin(), ret.end(), [](const QString &a, const QString &b) {
        const int byName = name(a).localeAwareCompare(name(b));
        return byName != 0 ? byName < 0 : a < b;
    });
    return ret;
}

QList<FontItem *> FMLangCoverage::fonts(const QString &language)
{
    QList<FontItem *> ret;
    if (language.isEmpty())
        return ret;
    const QList<FontItem *> fonts(FMFontDb::DB()->AllFonts());
    for (FontItem *fit : fonts) {
        if (languagesOf(fit).contains(language))
            ret << fit;
    }
    return ret;
}

namespace
{
/// the Qt locale of a fontconfig language code ("az-ir"), invalid when Qt does not know it
QLocale localeOf(const QString &language)
{
    QString code(language);
    code.replace(QLatin1Char('-'), QLatin1Char('_'));
    const QLocale locale(code);
    if (locale.language() == QLocale::C || locale.language() == QLocale::AnyLanguage) {
        // "ber-dz" and the like: the part before the dash may still be a language
        const QString bare(code.section(QLatin1Char('_'), 0, 0));
        const QLocale plain(bare);
        if (plain.language() != QLocale::C && plain.language() != QLocale::AnyLanguage)
            return plain;
        return QLocale(QLocale::C);
    }
    return locale;
}
}

QString FMLangCoverage::name(const QString &language)
{
    const QLocale locale(localeOf(language));
    if (locale.language() == QLocale::C)
        return language;
    return QStringLiteral("%1 (%2)").arg(QLocale::languageToString(locale.language()), language);
}

QString FMLangCoverage::nativeName(const QString &language)
{
    const QLocale locale(localeOf(language));
    if (locale.language() == QLocale::C)
        return QString();
    return locale.nativeLanguageName();
}

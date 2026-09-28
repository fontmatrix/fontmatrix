/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmpaths.h"
#include "typotek.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include <array>
#include <span>

FMPaths *FMPaths::getThis()
{
    // Static-local initialisation is thread-safe under C++11 and later.
    static FMPaths inst;
    return &inst;
}

QString FMPaths::TranslationsDir()
{
    QString cached = getThis()->FMPathsDB.value(QStringLiteral("TranslationsDir"));
    if (!cached.isEmpty())
        return cached;

    const QString dirsep(QDir::separator());
    QString dir;
#ifdef PLATFORM_APPLE
    dir =
        QApplication::applicationDirPath() + dirsep + QStringLiteral("..") + dirsep + QStringLiteral("Resources") + dirsep + QStringLiteral("Locales") + dirsep;
#elif defined(_WIN32)
    dir = QApplication::applicationDirPath() + dirsep + QStringLiteral("share") + dirsep + QStringLiteral("qm") + dirsep;
#else
    dir = QApplication::applicationDirPath() + dirsep + QStringLiteral("..") + dirsep + QStringLiteral("share") + dirsep + QStringLiteral("fontmatrix") + dirsep
        + QStringLiteral("qm") + dirsep;
#endif
    getThis()->FMPathsDB[QStringLiteral("TranslationsDir")] = dir;
    return dir;
}

QString FMPaths::HelpDir()
{
    QString cached = getThis()->FMPathsDB.value(QStringLiteral("HelpDir"));
    if (!cached.isEmpty())
        return cached;

    const QString dirsep(QDir::separator());
    QString hf;
#ifdef PLATFORM_APPLE
    hf = LocalizedDirPath(QApplication::applicationDirPath() + dirsep + QStringLiteral("help") + dirsep);
#elif defined(_WIN32)
    hf = LocalizedDirPath(QApplication::applicationDirPath() + dirsep + QStringLiteral("help") + dirsep);
#else
    hf = LocalizedDirPath(QApplication::applicationDirPath() + dirsep + QStringLiteral("..") + dirsep + QStringLiteral("share") + dirsep
                          + QStringLiteral("fontmatrix") + dirsep + QStringLiteral("help") + dirsep);
#endif
    getThis()->FMPathsDB[QStringLiteral("HelpDir")] = hf;
    return hf;
}

QString FMPaths::HandbookFile()
{
    const QString dirsep(QDir::separator());
    QString base;
#ifdef PLATFORM_APPLE
    base = QApplication::applicationDirPath() + dirsep + QStringLiteral("..") + dirsep + QStringLiteral("Resources") + dirsep + QStringLiteral("handbook")
        + dirsep;
#elif defined(_WIN32)
    // ECM puts the application's data in <appdir>/data on Windows; see ResourcesDir().
    base = QApplication::applicationDirPath() + dirsep + QStringLiteral("data") + dirsep + QStringLiteral("fontmatrix") + dirsep + QStringLiteral("handbook")
        + dirsep;
#else
    base = QApplication::applicationDirPath() + dirsep + QStringLiteral("..") + dirsep + QStringLiteral("share") + dirsep + QStringLiteral("fontmatrix")
        + dirsep + QStringLiteral("handbook") + dirsep;
#endif
    const QString dir(LocalizedDirPath(base));
    if (dir.isEmpty())
        return QString();
    const QString file(dir + QStringLiteral("index.html"));
    return QFile::exists(file) ? file : QString();
}

QString FMPaths::ResourcesDir()
{
    QString cached = getThis()->FMPathsDB.value(QStringLiteral("ResourcesDir"));
    if (!cached.isEmpty())
        return cached;

    const QString dirsep(QDir::separator());
    QString dir;
#ifdef PLATFORM_APPLE
    dir = QApplication::applicationDirPath() + dirsep + QStringLiteral("..") + dirsep + QStringLiteral("Resources") + dirsep;
#elif defined(_WIN32)
    // ECM resolves KDE_INSTALL_DATAROOTDIR to "bin/data" on Windows, i.e.
    // <appdir>/data. The top-level CMakeLists.txt installs the resources to
    // ${KDE_INSTALL_DATADIR}/fontmatrix/resources, so this must match.
    dir = QApplication::applicationDirPath() + dirsep + QStringLiteral("data") + dirsep + QStringLiteral("fontmatrix") + dirsep + QStringLiteral("resources")
        + dirsep;
#else
    // Relative to the executable so the app works both installed and from
    // a build directory (build/bin/fontmatrix → build/share/fontmatrix/resources/).
    dir = QApplication::applicationDirPath() + dirsep + QStringLiteral("..") + dirsep + QStringLiteral("share") + dirsep + QStringLiteral("fontmatrix") + dirsep
        + QStringLiteral("resources") + dirsep;
#endif
    getThis()->FMPathsDB[QStringLiteral("ResourcesDir")] = dir;
    return dir;
}

QString FMPaths::SamplesDir()
{
    const QString sep(QDir::separator());
    return typotek::getInstance()->getOwnDir().absolutePath() + sep + QStringLiteral("Samples") + sep;
}

QString FMPaths::FiltersDir()
{
    const QString sep(QDir::separator());
    QString dir = typotek::getInstance()->getConfigDir().absolutePath() + sep + QStringLiteral("Filters") + sep;
    QDir().mkpath(dir);
    return dir;
}

QString FMPaths::HyphenationDictionary(const QLocale &locale)
{
    const QString language(QLocale::languageToCode(locale.language()));
    if (language.isEmpty() || locale.language() == QLocale::C)
        return QString();
    const QString territory(QLocale::territoryToCode(locale.territory()));

    // in the order of preference: exact, same language any territory, language alone
    QStringList wanted;
    if (!territory.isEmpty())
        wanted << QStringLiteral("hyph_%1_%2.dic").arg(language, territory);
    wanted << QStringLiteral("hyph_%1_*.dic").arg(language) << QStringLiteral("hyph_%1.dic").arg(language);

    const QStringList dirs(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("hyphen"), QStandardPaths::LocateDirectory));
    for (const QString &pattern : std::as_const(wanted)) {
        for (const QString &d : dirs) {
            const QStringList found(QDir(d).entryList({pattern}, QDir::Files | QDir::Readable, QDir::Name));
            if (!found.isEmpty())
                return QDir(d).absoluteFilePath(found.first());
        }
    }
    return QString();
}

QString FMPaths::LocalizedDirPath(const QString &base, const QString &fallback)
{
    const QString sep(QStringLiteral("_"));
    const QStringList l_c(QLocale::system().name().split(sep));
    const QString langcode(l_c.first());
    const QString countrycode(l_c.last());

    std::array<QString, 4> candidates;
    int count = 0;
    if (!langcode.isEmpty() || !countrycode.isEmpty()) {
        candidates[count++] = base + langcode + sep + countrycode;
        candidates[count++] = base + langcode;
    }
    candidates[count++] = base + fallback;
    candidates[count++] = base;

    for (const QString &candidate : std::span(candidates.data(), count)) {
        QDir d(candidate);
        if (d.exists())
            return d.absolutePath() + QString(QDir::separator());
    }

    return QString();
}

QString FMPaths::LocalizedFilePath(const QString &base, const QString &ext, const QString &fallback)
{
    const QString sep(QStringLiteral("_"));
    const QStringList l_c(QLocale::system().name().split(sep));
    const QString langcode(l_c.first());
    const QString countrycode(l_c.last());

    std::array<QString, 4> candidates;
    int count = 0;
    if (!langcode.isEmpty() || !countrycode.isEmpty()) {
        candidates[count++] = base + langcode + sep + countrycode + ext;
        candidates[count++] = base + langcode + ext;
    }
    candidates[count++] = base + fallback + ext;
    candidates[count++] = base + ext;

    for (const QString &candidate : std::span(candidates.data(), count)) {
        if (QFile::exists(candidate))
            return candidate;
    }

    return QString();
}

namespace
{
/**
 * The host's XDG directory, not the sandbox's: inside a Flatpak, XDG_DATA_HOME
 * and XDG_CONFIG_HOME name ~/.var/app/<id>/..., while the fontconfig of the
 * other applications reads ~/.local/share/fonts and ~/.config/fontconfig.
 */
QString hostXdgHome(const char *variable, const QString &fallback)
{
    if (!QFileInfo::exists(QStringLiteral("/.flatpak-info"))) {
        const QString value(qEnvironmentVariable(variable));
        if (!value.isEmpty())
            return value;
    }
    return QDir::homePath() + fallback;
}
}

QString FMPaths::UserFontsDir()
{
    return hostXdgHome("XDG_DATA_HOME", QStringLiteral("/.local/share")) + QStringLiteral("/fonts/fontmatrix");
}

QString FMPaths::FontconfigRejectsFile()
{
    return hostXdgHome("XDG_CONFIG_HOME", QStringLiteral("/.config")) + QStringLiteral("/fontconfig/conf.d/60-fontmatrix-rejects.conf");
}

QString FMPaths::FontconfigUserFile()
{
    return hostXdgHome("XDG_CONFIG_HOME", QStringLiteral("/.config")) + QStringLiteral("/fontconfig/fonts.conf");
}

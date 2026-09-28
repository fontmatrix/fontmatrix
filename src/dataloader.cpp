/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dataloader.h"
#include "fmpaths.h"

#include <QDir>
#include <QFile>
#include <QLocale>

DataLoader::DataLoader()
{
    load();
}

void DataLoader::reload()
{
    load();
}

void DataLoader::load()
{
    sm.clear();
    pm.clear();
    lm.clear();

    // System samples — skip silently if the directory is absent (e.g. not yet installed)
    QDir samplesDir(FMPaths::ResourcesDir() + QLatin1String("Samples"));
    if (samplesDir.exists()) {
        for (const auto entries = samplesDir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs); const auto &ld : entries) {
            QDir lang(samplesDir.absoluteFilePath(ld));
            QLocale locale(ld);
            // Qt does not know every language of the samples (Udmurt): such a group keeps
            // the name of its directory rather than being called "C"
            const QString loclang(locale.language() == QLocale::C ? ld : QLocale::languageToString(locale.language()));
            lm.insert(loclang, locale);
            for (const auto entriesList = lang.entryList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files); const auto &st : entriesList) {
                QFile fp(lang.absoluteFilePath(st));
                if (fp.open(QIODevice::ReadOnly)) {
                    sm[loclang][st] = QString::fromUtf8(fp.readAll());
                }
            }
        }
    }

    // User samples
    QDir uDir(FMPaths::SamplesDir());
    if (!uDir.exists()) {
        uDir.mkpath(uDir.absolutePath());
    } else {
        for (const auto loopEntries = uDir.entryList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files); const auto &ld : loopEntries) {
            QFile fp(uDir.absoluteFilePath(ld));
            if (fp.open(QIODevice::ReadOnly)) {
                pm[ld] = QString::fromUtf8(fp.readAll());
            }
        }
    }

    // Fallback — keeps the UI functional when no samples are installed
    if (sm.isEmpty() && pm.isEmpty()) {
        sm[QStringLiteral("Emergency")][QStringLiteral("Text")] = QStringLiteral("Emergency Text");
    }
}

bool DataLoader::update(const QString &name, const QString &sample)
{
    QDir uDir(FMPaths::SamplesDir());
    QFile fp(uDir.absoluteFilePath(name));
    if (fp.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QByteArray utf8 = sample.toUtf8();
        if (fp.write(utf8) == utf8.size()) {
            pm[name] = sample;
            return true;
        }
    }
    return false;
}

bool DataLoader::remove(const QString &name)
{
    QDir uDir(FMPaths::SamplesDir());
    QFile fp(uDir.absoluteFilePath(name));
    if (fp.exists()) {
        if (fp.remove()) {
            pm.remove(name);
            return true;
        }
    }
    return false;
}

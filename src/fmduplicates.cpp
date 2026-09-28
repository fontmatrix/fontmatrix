/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmduplicates.h"
#include "fmfontdb.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"
#include "typotek.h"

#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QMap>

QHash<int, QList<QList<FontItem *>>> FMDuplicates::m_groups;

void FMDuplicates::invalidate()
{
    m_groups.clear();
}

QByteArray FMDuplicates::digest(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    QCryptographicHash hash(QCryptographicHash::Sha1);
    if (!hash.addData(&file))
        return QByteArray();
    return hash.result();
}

QList<QList<FontItem *>> FMDuplicates::identicalFiles()
{
    // two files of different lengths cannot be the same, and a length is free
    // to ask for: only what shares a length is ever read
    QMap<qint64, QList<FontItem *>> bySize;
    const QList<FontItem *> fonts(FMFontDb::DB()->AllFonts());
    for (FontItem *fit : fonts) {
        if (fit->isRemote() && !fit->isCached())
            continue;
        const QFileInfo info(fit->localPath());
        if (info.isFile() && info.size() > 0)
            bySize[info.size()] << fit;
    }

    QElapsedTimer timer;
    timer.start();
    int read = 0;
    QList<QList<FontItem *>> ret;
    for (auto it = bySize.constBegin(); it != bySize.constEnd(); ++it) {
        if (it.value().size() < 2)
            continue;
        QMap<QByteArray, QList<FontItem *>> byDigest;
        for (FontItem *fit : it.value()) {
            const QByteArray sum(digest(fit->localPath()));
            ++read;
            if (!sum.isEmpty())
                byDigest[sum] << fit;
        }
        for (auto group = byDigest.constBegin(); group != byDigest.constEnd(); ++group) {
            if (group.value().size() > 1)
                ret << group.value();
        }
    }
    qCDebug(FONTMATRIX_LOG) << "duplicates:" << read << "files read in" << timer.elapsed() << "ms," << ret.size() << "groups of identical files";
    return ret;
}

bool FMDuplicates::sameBytes(const QList<FontItem *> &fonts)
{
    // a length first, which costs nothing; only files of one length are read
    qint64 size(-1);
    for (FontItem *fit : fonts) {
        if (fit->isRemote() && !fit->isCached())
            return false;
        const qint64 s(QFileInfo(fit->localPath()).size());
        if (size >= 0 && s != size)
            return false;
        size = s;
    }
    QByteArray first;
    for (FontItem *fit : fonts) {
        const QByteArray sum(digest(fit->localPath()));
        if (sum.isEmpty() || (!first.isEmpty() && sum != first))
            return false;
        first = sum;
    }
    return true;
}

QList<QList<FontItem *>> FMDuplicates::sameFont()
{
    // what the font says it is: one family, one style, one version. The version
    // comes from the name table, which the database read at import
    QHash<FontItem *, QString> versions;
    const QList<FontDBResult> rows(FMFontDb::DB()->getInfo(QList<FontItem *>(), FMFontDb::VersionString, -1));
    for (const FontDBResult &row : rows) {
        if (row.first && !versions.contains(row.first))
            versions.insert(row.first, row.second.simplified());
    }

    QMap<QString, QList<FontItem *>> byFont;
    const QList<FontItem *> fonts(FMFontDb::DB()->AllFonts());
    for (FontItem *fit : fonts) {
        const QString key(fit->family() + QLatin1Char('\n') + fit->variant() + QLatin1Char('\n') + versions.value(fit));
        byFont[key] << fit;
    }

    QList<QList<FontItem *>> ret;
    for (auto it = byFont.constBegin(); it != byFont.constEnd(); ++it) {
        if (it.value().size() > 1 && !sameBytes(it.value()))
            ret << it.value();
    }
    qCDebug(FONTMATRIX_LOG) << "duplicates:" << ret.size() << "groups of one family, style and version";
    return ret;
}

QList<QList<FontItem *>> FMDuplicates::groups(Kind kind)
{
    auto it = m_groups.constFind(int(kind));
    if (it != m_groups.constEnd())
        return it.value();
    const QList<QList<FontItem *>> found(kind == IdenticalFiles ? identicalFiles() : sameFont());
    m_groups.insert(int(kind), found);
    return found;
}

QList<FontItem *> FMDuplicates::fonts(Kind kind)
{
    QList<FontItem *> ret;
    const QList<QList<FontItem *>> found(groups(kind));
    for (const QList<FontItem *> &group : found)
        ret += group;
    return ret;
}

int FMDuplicates::count(Kind kind)
{
    return int(fonts(kind).size());
}

/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmconfig.h"

#include <KConfigGroup>
#include <KSharedConfig>

namespace
{

// Split "Group/Key" → ("Group", "Key").  No slash → ("", fullKey) for global group.
std::pair<QString, QString> splitKey(const QString &fullKey)
{
    const int slash = fullKey.indexOf(QLatin1Char('/'));
    if (slash == -1)
        return {QString{}, fullKey};
    return {fullKey.left(slash), fullKey.mid(slash + 1)};
}

} // namespace

QVariant FMConfig::value(const QString &fullKey, const QVariant &def)
{
    auto [grp, key] = splitKey(fullKey);
    const KConfigGroup group = KSharedConfig::openConfig()->group(grp);
    // KConfigGroup::readEntry() converts the stored text using the type of the
    // default, and for an invalid default it returns an invalid QVariant even
    // when the key exists. QSettings returned the stored value, so callers
    // that pass no default expect one back: read those as text.
    if (!def.isValid())
        return group.hasKey(key) ? QVariant(group.readEntry(key, QString())) : QVariant();
    return group.readEntry(key, def);
}

void FMConfig::setValue(const QString &fullKey, const QVariant &val)
{
    auto [grp, key] = splitKey(fullKey);
    KSharedConfig::openConfig()->group(grp).writeEntry(key, val);
}

bool FMConfig::contains(const QString &fullKey)
{
    auto [grp, key] = splitKey(fullKey);
    return KSharedConfig::openConfig()->group(grp).hasKey(key);
}

void FMConfig::remove(const QString &fullKey)
{
    auto [grp, key] = splitKey(fullKey);
    KSharedConfig::openConfig()->group(grp).deleteEntry(key);
}

void FMConfig::sync()
{
    KSharedConfig::openConfig()->sync();
}

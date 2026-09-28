/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QString>
#include <QVariant>

// Cross-platform configuration wrapper.
// On Linux with KF6: uses KSharedConfig (stores in ~/.config/fontmatrixrc).
// On Windows/macOS: falls back to QSettings.
//
// Keys use "Group/Key" notation matching the existing QSettings convention.
// Root-level keys (no slash) go in the global config group.
class FMConfig
{
public:
    static QVariant value(const QString &fullKey, const QVariant &def = QVariant{});
    static void setValue(const QString &fullKey, const QVariant &val);
    static bool contains(const QString &fullKey);
    static void remove(const QString &fullKey);
    static void sync();
};

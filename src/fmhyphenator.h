/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMHYPHENATOR_H
#define FMHYPHENATOR_H

#include <hyphen.h>

#include "fmsharestruct.h"

#include <QList>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringDecoder>
#include <QStringEncoder>

typedef QMap<int, QPair<QString, QString>> HyphList;

class FMHyphenator : public QObject
{
public:
    FMHyphenator();
    ~FMHyphenator() override;

    bool loadDict(const QString &dictPath, int leftMin = 2, int rightMin = 3);
    HyphList hyphenate(const QString &word) const;
    /// no dictionary: words are left whole
    void unload();
    [[nodiscard]] QString dictPath() const
    {
        return currentDictPath;
    }

private:
    QString currentDictPath;
    HyphenDict *dict;
    QStringEncoder *textEncoder;
    QStringDecoder *textDecoder;
};

#endif

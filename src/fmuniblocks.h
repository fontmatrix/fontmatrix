/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMUNIBLOCKS_H
#define FMUNIBLOCKS_H

#include <QMap>
#include <QPair>
#include <QString>

class FMUniBlocks
{
    static FMUniBlocks *instance;
    static FMUniBlocks *that();

    FMUniBlocks();
    void loadBlocks();
    void recordLine(const QString &line);
    using bKey = QPair<int, int>;
    QMap<bKey, QString> p;
    bKey c; // current
    bKey f; // first
    bKey l; // last

public:
    static QString firstBlock(int &start, int &end);
    static QString lastBlock(int &start, int &end);
    static QString nextBlock(int &start, int &end);
    static QString currentBlock(int &start, int &end);

    static int start(const int &codepoint);
    static int end(const int &codepoint);

    static QString block(bKey key);
    static bKey interval(const QString &blockName);
    static QStringList blocks();
};

#endif // FMUNIBLOCKS_H

/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMDUPLICATES_H
#define FMDUPLICATES_H

#include <QHash>
#include <QList>
#include <QString>

class FontItem;

/**
 * The same font twice in the collection, which every collection of a few
 * thousand files has.
 *
 * Two kinds are told apart, because what to do about them differs. Files that
 * are byte for byte the same are simply copies, and one of them can go. Files
 * that hold the same font — one family, one style, one version — but differ in
 * their bytes are the same design in another build, hinted differently or
 * carrying another set of tables, and which one to keep is a decision.
 *
 * Nothing is written to the database. A file is read only when another file of
 * exactly its size exists, which in a collection of thousands leaves a handful.
 */
class FMDuplicates
{
public:
    enum Kind {
        IdenticalFiles = 0, ///< the same bytes
        SameFont ///< one family, style and version, in files that differ
    };

    /// the fonts that share their kind with at least one other, in groups
    static QList<QList<FontItem *>> groups(Kind kind);
    /// the same, flattened: what a filter keeps
    static QList<FontItem *> fonts(Kind kind);
    /// how many fonts are in those groups, without listing them twice
    static int count(Kind kind);

    /// the fonts of the database changed: everything is worked out again
    static void invalidate();

private:
    static QByteArray digest(const QString &path);
    /// whether the files are all byte for byte the same: identical files, not another build
    static bool sameBytes(const QList<FontItem *> &fonts);
    static QList<QList<FontItem *>> identicalFiles();
    static QList<QList<FontItem *>> sameFont();

    static QHash<int, QList<QList<FontItem *>>> m_groups;
};

#endif

/*
    SPDX-FileCopyrightText: 2008 Riku Leino <riku.leino@gmail.com>
    SPDX-FileCopyrightText: 2002 David L. Wagner, International Business Machines Corp.

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PANOSEMATCH_H
#define PANOSEMATCH_H

#include <QList>
#include <QString>

class PanoseMatch
{
public:
    /** Init the matcher with the Panose string for the selected font */
    explicit PanoseMatch(const QString &selected);

    PanoseMatch() = default;
    ~PanoseMatch() = default;

    void setAttributes(const QString &attrs);

    /** Determine the difference between the selected font and the other font. */
    [[nodiscard]] int diff(const QString &other) const;

private:
    QList<int> attributes;

    [[nodiscard]] QList<int> parse(const QString &panoseString) const;

    /** Calculate the square of the difference between two Panose values. */
    static int calcdiffm(int, int, int);

    /** Calculate the difference between two discrete Panose values. */
    static int calcdiffd(int, int);

    /** Calculate the difference between two Serif Style values. */
    static int calcdiffss(int, int);

    /** Calculate the difference between two Letterform values. */
    static int calcdifflf(int, int);

    enum PanoseKey {
        FamilyType = 0,
        SerifStyle,
        Weight,
        Proportion,
        Contrast,
        StrokeVariation,
        ArmStyle,
        Letterform,
        Midline,
        XHeight,
        InvalidPK = 9999999
    };
};

class FontItem;

/// An helper class for Panose matching
class PanoseMatchFont : private PanoseMatch
{
    PanoseMatchFont() = default;
    ~PanoseMatchFont() = default;

public:
    static QList<FontItem *> similar(FontItem *ref, int treshold);
};

#endif // PANOSEMATCH_H

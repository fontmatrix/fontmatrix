/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PUZZLEVIEWIMP_H
#define PUZZLEVIEWIMP_H

#include <QImage>
#include <QList>
#include <QString>

class SVect
{
public:
    SVect()
        : x(0.0)
        , y(0.0)
    {
    }
    SVect(double xx, double yy)
        : x(xx)
        , y(yy)
    {
    }

    [[nodiscard]] inline double squareDistance(const SVect &other) const
    {
        double C1(other.x - x);
        double C2(other.y - y);

        return (C1 * C1) + (C2 * C2);
    }

private:
    double x;
    double y;
};

class PuzzleViewImp : public QList<SVect>
{
    PuzzleViewImp() = default;

public:
    PuzzleViewImp(const QString &iPath, QRgb patternColor);
    PuzzleViewImp(const QImage &qimg, QRgb patternColor);
    ~PuzzleViewImp();

    QList<double> CompList(const PuzzleViewImp &other);
    double CompSum(const PuzzleViewImp &other);
    double CompMean(const PuzzleViewImp &other);

private:
    QRgb pc = 0U;
    void init(const QImage &qimg);
};

#endif // PUZZLEVIEWIMP_H

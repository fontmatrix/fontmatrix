/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GLYPHTOSVGHELPER_H
#define GLYPHTOSVGHELPER_H

#include <QString>

#include <QPainterPath>
#include <QRectF>
#include <QTransform>

class GlyphToSVGHelper
{
    QPainterPath m_path;
    QTransform m_transform;
    QString m_svg;

public:
    explicit GlyphToSVGHelper(QPainterPath path, QTransform tf = QTransform());

    QString getSVG(int pSize = 1000) const;
    QString getSVGPath() const;
    QRectF getRect() const;
};

#endif // GLYPHTOSVGHELPER_H

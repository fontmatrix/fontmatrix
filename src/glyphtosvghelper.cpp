/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "glyphtosvghelper.h"
#include "fontmatrix_debug.h"

#include <QPointF>
#include <QStringList>

GlyphToSVGHelper::GlyphToSVGHelper(QPainterPath path, QTransform tf)
    : m_path(path)
    , m_transform(tf)
{
    QStringList data;
    QPointF curPos;
    for (int i = 0; i < path.elementCount(); ++i) {
        QPainterPath::Element cur = path.elementAt(i);
        QPointF curPoint(tf.map(cur));
        if (cur.isMoveTo()) {
            curPos = curPoint;
            data << QStringLiteral("M %1 %2").arg(curPos.x()).arg(curPos.y());
        } else if (cur.isLineTo()) {
            curPos = curPoint;
            data << QStringLiteral("L %1 %2").arg(curPos.x()).arg(curPos.y());
        } else if (cur.isCurveTo()) {
            QPointF c1 = tf.map(path.elementAt(i + 1));
            QPointF c2 = tf.map(path.elementAt(i + 2));
            data << QStringLiteral("C %1 %2 %3 %4 %5 %6").arg(curPoint.x()).arg(curPoint.y()).arg(c1.x()).arg(c1.y()).arg(c2.x()).arg(c2.y());

            //             qDebug(data.last().toUtf8());

            i += 2;
            curPos = c2;
        } else
            qCDebug(FONTMATRIX_LOG, "Unknown point type");
    }

    m_svg += QStringLiteral("<path d=\"%1\" fill=\"%2\" />").arg(data.join(QStringLiteral(" ")), QStringLiteral("black"));
}

QString GlyphToSVGHelper::getSVG(int pSize) const
{
    QRectF r(m_transform.mapRect(m_path.boundingRect()));
    QString bbS(QStringLiteral("%1 %2 %3 %4"));
    QString bb(bbS.arg(r.top()).arg(r.left()).arg(r.width()).arg(r.height()));
    QString openElem(QStringLiteral("<svg width=\"%1\" height=\"%1\" viewBox=\"%2\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">").arg(pSize).arg(bb));
    QString closeElem(QStringLiteral("</svg>"));
    return openElem + m_svg + closeElem;
}

QString GlyphToSVGHelper::getSVGPath() const
{
    return m_svg;
}

QRectF GlyphToSVGHelper::getRect() const
{
    QRectF r(m_transform.mapRect(m_path.boundingRect()));
    return r;
}

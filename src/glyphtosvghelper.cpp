/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "glyphtosvghelper.h"

#include <QStringList>
#include <QPointF>

GlyphToSVGHelper::GlyphToSVGHelper(QPainterPath path, QTransform tf)
    :m_path(path), m_transform(tf)
{
    QStringList data;
    QPointF curPos;
    for (int i = 0; i < path.elementCount(); ++i)
    {
        QPainterPath::Element cur = path.elementAt(i);
        QPointF curPoint(tf.map(cur));
        if(cur.isMoveTo())
        {
            curPos = curPoint;
            data << QString("M %1 %2").arg(curPos.x()).arg(curPos.y());
        }
        else if(cur.isLineTo())
        {
            curPos = curPoint;
            data << QString("L %1 %2").arg(curPos.x()).arg(curPos.y());
        }
        else if(cur.isCurveTo())
        {
            QPointF c1 = tf.map(path.elementAt(i + 1));
            QPointF c2 = tf.map(path.elementAt(i + 2));
            data << QString("C %1 %2 %3 %4 %5 %6")
                    .arg(curPoint.x()).arg(curPoint.y())
                    .arg(c1.x()).arg(c1.y())
                    .arg(c2.x()).arg(c2.y());

//             qDebug(data.last().toUtf8());

            i += 2;
            curPos = c2;
        }
        else
            qDebug("Unknown point type");
    }

    m_svg += QString("<path d=\"%1\" fill=\"%2\" />").arg(data.join(" ")).arg("black");

}

QString GlyphToSVGHelper::getSVG(int pSize) const
{
    QRectF r(m_transform.mapRect( m_path.boundingRect() ));
    QString bbS("%1 %2 %3 %4");
    QString bb(bbS
            .arg(r.top())
            .arg(r.left())
            .arg(r.width())
            .arg(r.height()));
    QString openElem(QString("<svg width=\"%1\" height=\"%1\" viewBox=\"%2\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">").arg(pSize).arg (bb));
    QString closeElem("</svg>");
    return openElem + m_svg + closeElem;
}

QString GlyphToSVGHelper::getSVGPath() const
{
    return m_svg;
}

QRectF  GlyphToSVGHelper::getRect() const
{
    QRectF r(m_transform.mapRect( m_path.boundingRect() ));
    return r;
}

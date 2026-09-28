/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMGLYPHHIGHLIGHT_H
#define FMGLYPHHIGHLIGHT_H

#include <QObject>
#include <QPointF>
#include <QRectF>

class QGraphicsScene;
class QGraphicsRectItem;
class QTimeLine;

class FMGlyphHighlight : public QObject
{
    Q_OBJECT
public:
    FMGlyphHighlight(QGraphicsScene *scene, const QRectF &rect, int time = 300, int frames = 12);
    ~FMGlyphHighlight() override;

private:
    QGraphicsRectItem *m_rect = nullptr;
    QTimeLine *m_timeline = nullptr;
    QPointF initialPos;
    int maxFrame;

    void lastFrame();

private Q_SLOTS:
    void animate(int);
};

#endif

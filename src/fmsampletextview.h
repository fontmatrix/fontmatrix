/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMSAMPLETEXTVIEW_H
#define FMSAMPLETEXTVIEW_H

#include <QGraphicsView>
#include <QPointF>

/**
    @author Pierre Marchand <pierre@oep-h.com>
*/
class FMSampleTextView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit FMSampleTextView(QWidget *parent);

    ~FMSampleTextView() override;

    void fakePage();
    void sheduleUpdate();
    void unSheduleUpdate();
    bool locker = false;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void showEvent(QShowEvent *event) override;

Q_SIGNALS:
    void refit();
    void pleaseUpdateMe();
    void pleaseZoom(int);

private:
    QPointF mouseStartPoint;
    QGraphicsRectItem *theRect = nullptr;
    QGraphicsRectItem *fPage = nullptr;
    bool isSelecting;
    bool isPanning;
    bool hasPendingUpdate;

    void ensureTheRect();
};

#endif

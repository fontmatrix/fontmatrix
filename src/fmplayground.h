/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMPLAYGROUND_H
#define FMPLAYGROUND_H

#include <QGraphicsView>
#include <QPointF>
#include <QRectF>
#include <QTimer>

class FontItem;

class FMPlayGround : public QGraphicsView
{
    Q_OBJECT
public:
    explicit FMPlayGround(QWidget *parent);
    ~FMPlayGround() override;

    QStringList fontnameList();
    QList<QGraphicsItemGroup *> getLines();
    QRectF getMaxRect();

    void updateLine();
    void closeLine();
    void deselectAll();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

    void keyReleaseEvent(QKeyEvent *e) override;

    void leaveEvent(QEvent *e) override;

private:
    void displayGlyphs(const QString &spec, FontItem *fontI, double fontS);
    QPointF mouseStartPoint;
    bool isPanning;
    QList<QGraphicsItemGroup *> glyphLines;
    QList<QGraphicsItem *> curLine;
    QString curString;
    QRectF curSelRect;

    void removeLine();

    // this cursor is at the begining of a line
    QPointF CursorPos;
    // this one at the pen position
    QPointF BlinkPos;
    QTimer *CursorTimer = nullptr;

Q_SIGNALS:
    void pleaseZoom(int);

private Q_SLOTS:
    void blinkCursor();
};

#endif

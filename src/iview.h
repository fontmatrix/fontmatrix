/*
    SPDX-FileCopyrightText: 2009 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IVIEW_H
#define IVIEW_H

#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsView>
#include <QPixmap>
#include <QRect>

class IView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit IView(QWidget *parent);
    ~IView() override = default;

    void setImage(const QString &path);
    void setImage(const QPixmap &pixmap);
    QPixmap getPixmap();

    // control if the rect can be tweak by user or not
    void setControlRect(bool u);

Q_SIGNALS:
    void rectChange(QRect);
    void selColorChanged(QRgb);

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QGraphicsPixmapItem *curImage = nullptr;

    QGraphicsPolygonItem *curSel = nullptr;
    QGraphicsRectItem *curRect = nullptr;
    QGraphicsEllipseItem *curTL = nullptr;
    QGraphicsEllipseItem *curTR = nullptr;
    QGraphicsEllipseItem *curBL = nullptr;
    QGraphicsEllipseItem *curBR = nullptr;

    QPointF mouseStartPoint;
    QRect theRect;
    bool isSelecting;
    bool isPanning;

    void fitImage();

    void selectGlyph(const QPointF &scenepos);

    bool m_controlRect;

private Q_SLOTS:
    void drawSelRect(QRect);
};

#endif

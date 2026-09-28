/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMFONTCOMPAREVIEW_H
#define FMFONTCOMPAREVIEW_H

#include <QGraphicsView>
#include <QMap>
#include <QUuid>

class QGraphicsLineItem;
class QGraphicsRectItem;
class QGraphicsEllipseItem;
class QGraphicsSimpleTextItem;
class FontItem;

class FMFontCompareItem
{
public:
    enum GElement {
        Nothing = 0,
        Contour = 0x1,
        // 			Fill		= 0x2,
        Points = 0x4,
        Controls = 0x8,
        Metrics = 0x10
    };
    Q_DECLARE_FLAGS(GElements, GElement)
    FMFontCompareItem();
    FMFontCompareItem(QGraphicsScene *s, FontItem *f, int z);
    ~FMFontCompareItem();
    void show(GElements elems, QColor color, double offset = 0.0);
    void setChar(uint c)
    {
        char_code = c;
    }
    QRectF boundingRect();
    // 		QColor getColor() {return color;}
    void setIndex(int i)
    {
        zindex = i;
    }

private:
    const QUuid uuid;
    QGraphicsScene *scene = nullptr;
    FontItem *font = nullptr;
    int zindex;
    uint char_code;
    double m_offset = 0.0;
    // 		QColor color;
    QGraphicsPathItem *path = nullptr;
    QList<QGraphicsLineItem *> lines_controls;
    QList<QGraphicsLineItem *> lines_metrics;
    QList<QGraphicsEllipseItem *> points;
    QList<QGraphicsSimpleTextItem *> text_metrics;

    void clear();
    void drawPoint(QPointF point, bool control);
    void toScreen();

    static const QString toolTipModel;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(FMFontCompareItem::GElements)

class FMFontCompareView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit FMFontCompareView(QWidget *parent);
    ~FMFontCompareView() override;

    void changeFont(int level, FontItem *font);
    void removeFont(int level);
    void changeChar(uint ccode);
    void changeChar(int level, uint ccode);
    void setElements(int level, FMFontCompareItem::GElements elems);
    FMFontCompareItem::GElements getElements(int level);
    void setColor(int level, QColor color);
    QColor getColor(int level);
    void setOffset(int level, double offset);
    double getOffset(int level);
    void fitGlyphsView();

    static QMap<QString, QPen> pens;
    static QMap<QString, QBrush> brushes;

public:
    void updateGlyphs();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QMap<int, FMFontCompareItem *> glyphs; // < Z-index, glyph >
    QMap<int, FMFontCompareItem::GElements> elements; // what to show
    QMap<int, QColor> colors;
    QMap<int, double> offsets;

    void initPensAndBrushes();
    uint thechar = 0U;

    QPointF mouseStartPoint;
    QGraphicsRectItem *theRect = nullptr;
    QGraphicsRectItem *fPage = nullptr;
    bool isSelecting;
    bool isPanning;
};

#endif

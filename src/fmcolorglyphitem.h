/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FMCOLORGLYPHITEM_H
#define FMCOLORGLYPHITEM_H

#include <QColor>
#include <QGraphicsPathItem>
#include <QImage>
#include <QList>
#include <QPainterPath>
#include <QRectF>

/// one layer of a COLR glyph: an outline and its colour from the palette
struct GlyphLayer {
    QPainterPath path;
    QColor color; ///< invalid for the text colour, which is the brush of the item
};

/**
 * A glyph of a colour font where the scenes expect a path item. The path is
 * what the layout, the chart and the exports measure: the outline of the
 * layers, or the box of the bitmap. What is painted is the colour: the layers
 * each with their colour, or the bitmap into its box.
 */
class FMColorGlyphItem : public QGraphicsPathItem
{
public:
    explicit FMColorGlyphItem(QGraphicsItem *parent = nullptr);

    /// the glyph is layers of outlines (COLR version 0)
    void setLayers(const QList<GlyphLayer> &layers);
    /// the glyph is a bitmap (CBDT, sbix) that sits in the rectangle, in the units of the item
    void setImage(const QImage &image, const QRectF &rect);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QList<GlyphLayer> m_layers;
    QImage m_image;
    QRectF m_imageRect;
};

#endif

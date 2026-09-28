/*
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmcolorglyphitem.h"

#include <QPainter>

FMColorGlyphItem::FMColorGlyphItem(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    setPen(Qt::NoPen);
}

void FMColorGlyphItem::setLayers(const QList<GlyphLayer> &layers)
{
    m_layers = layers;
    m_image = QImage();
    QPainterPath all;
    for (const GlyphLayer &layer : layers)
        all.addPath(layer.path);
    setPath(all);
}

void FMColorGlyphItem::setImage(const QImage &image, const QRectF &rect)
{
    m_layers.clear();
    m_image = image;
    m_imageRect = rect;
    QPainterPath box;
    box.addRect(rect);
    setPath(box);
}

void FMColorGlyphItem::paint(QPainter *painter, [[maybe_unused]] const QStyleOptionGraphicsItem *option, [[maybe_unused]] QWidget *widget)
{
    if (!m_image.isNull()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter->drawImage(m_imageRect, m_image);
        return;
    }
    painter->setPen(Qt::NoPen);
    for (const GlyphLayer &layer : std::as_const(m_layers))
        painter->fillPath(layer.path, layer.color.isValid() ? layer.color : brush().color());
}

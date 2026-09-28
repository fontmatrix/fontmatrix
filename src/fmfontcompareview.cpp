/*
    SPDX-FileCopyrightText: 2008 Pierre Marchand <pierremarc@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmfontcompareview.h"
#include "fontmatrix_debug.h"

#include "fontitem.h"

#include <QApplication>
#include <QDebug>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QScrollBar>
#include <QWheelEvent>

const QString FMFontCompareItem::toolTipModel = QStringLiteral("<strong>x</strong>: %1 <strong>y</strong>: %2");

/// Item
FMFontCompareItem::FMFontCompareItem()
    : uuid(QUuid::createUuid())
    , scene(nullptr)
    , font(nullptr)
    , zindex(0)
    , char_code(0)
    , path(nullptr)
{
    // Nothing special :)
    // 	qDebug()<< "Create" <<uuid.toString();
}

FMFontCompareItem::FMFontCompareItem(QGraphicsScene *s, FontItem *f, int z)
    : uuid(QUuid::createUuid())
    , scene(s)
    , font(f)
    , zindex(z)
    , char_code(0)
    , path(nullptr)
{
    // 	qDebug()<< "Create" <<uuid.toString();
    // 	int ml(159);
    // 	color.setRgb(qrand() % ml,qrand() % ml, qrand() % ml, 255);
}

FMFontCompareItem::~FMFontCompareItem()
{
    clear();
}

void FMFontCompareItem::clear()
{
    // 	qDebug()<< "Clearing" <<uuid.toString();

    for (auto *li : std::as_const(lines_controls)) {
        delete li;
    }
    lines_controls.clear();
    for (auto *li : std::as_const(lines_metrics)) {
        delete li;
    }
    lines_metrics.clear();
    for (auto *ri : std::as_const(points)) {
        delete ri;
    }
    points.clear();
    for (auto *ti : std::as_const(text_metrics)) {
        delete ti;
    }
    text_metrics.clear();
    if (path) {
        delete path;
        path = nullptr;
    }
}

void FMFontCompareItem::toScreen()
{
    if (!scene)
        return;
    QRectF itemsBB(scene->sceneRect());
    for (auto *li : std::as_const(lines_controls)) {
        scene->addItem(li);
        itemsBB = itemsBB.united(li->boundingRect());
        li->setZValue(zindex);
    }
    for (auto *li : std::as_const(lines_metrics)) {
        scene->addItem(li);
        itemsBB = itemsBB.united(li->boundingRect());
        li->setZValue(zindex);
    }
    for (auto *ri : std::as_const(points)) {
        scene->addItem(ri);
        itemsBB = itemsBB.united(ri->boundingRect());
        ri->setZValue(zindex);
    }
    for (auto *ti : std::as_const(text_metrics)) {
        scene->addItem(ti);
        ti->setZValue(zindex);
    }
    if (path) {
        scene->addItem(path);
        itemsBB = itemsBB.united(path->boundingRect());
        path->setZValue(zindex);
    }

    scene->setSceneRect(itemsBB);
    // 	qDebug()<<"R"<<scene->sceneRect();
}

QRectF FMFontCompareItem::boundingRect()
{
    if (!path)
        return QRectF();

    return path->path().boundingRect();
}

void FMFontCompareItem::drawPoint(QPointF point, bool control)
{
    double u = control ? 1.0 : 2.0;

    QRectF r(point.x() - u, point.y() - u, 2 * u, 2 * u);
    auto ri = new QGraphicsEllipseItem(r);
    ri->setBrush(Qt::NoBrush);
    if (control)
        ri->setPen(FMFontCompareView::pens[QStringLiteral("control-point")]);
    else
        ri->setPen(FMFontCompareView::pens[QStringLiteral("point")]);

    QString label(toolTipModel.arg(point.x() - m_offset).arg(-point.y()));
    ri->setToolTip(label.replace(QLatin1String("-"), QStringLiteral("−")));
    points << ri;
}

void FMFontCompareItem::show(FMFontCompareItem::GElements elems, QColor color, double offset)
{
    // 	qDebug()<<"FMFontCompareItem::show"<<char_code<<font->fancyName();
    // As it’s lightweight graphic, no need to be too much circonvoluted
    clear();
    if (!font)
        return;

    if (!elems.testFlag(Contour))
        return;

    m_offset = offset;
    double fsize(1000.0);
    double sf(fsize / font->getUnitPerEm());
    path = font->itemFromChar(char_code, fsize);
    if (!path) {
        qCDebug(FONTMATRIX_LOG) << "Unable to load char" << char_code << "from font" << font->fancyName();
        return;
    }
    path->moveBy(offset, 0.0);
    path->setPen(QPen(color));

    QColor brushColor(color);
    QColor penColor(color);

    if (brushColor.alpha() > 0) {
        brushColor.setAlpha(255 / qMax(zindex + 1, 2));
    }
    penColor.setAlpha(176);

    path->setBrush(brushColor);
    path->setPen(penColor);

    if (elems.testFlag(Points)) {
        QPointF curPos;
        for (int i = 0; i < path->path().elementCount(); ++i) {
            QPainterPath::Element cur = path->path().elementAt(i);
            cur.x += offset;
            if (cur.isMoveTo()) {
                curPos = cur;
            } else if (cur.isLineTo()) {
                drawPoint(curPos, false);
                curPos = cur;
            } else if (cur.isCurveTo()) {
                QPainterPath::Element c1 = path->path().elementAt(i + 1);
                QPainterPath::Element c2 = path->path().elementAt(i + 2);
                c1.x += offset;
                c2.x += offset;
                drawPoint(curPos, false);
                drawPoint(c2, false);
                if (elems.testFlag(Controls)) {
                    drawPoint(cur, true);
                    drawPoint(c1, true);
                    QLineF l1(curPos, cur);
                    QLineF l2(c2, c1);
                    lines_controls << new QGraphicsLineItem(l1);
                    lines_controls.constLast()->setPen(FMFontCompareView::pens[QStringLiteral("control-line")]);
                    lines_controls << new QGraphicsLineItem(l2);
                    lines_controls.constLast()->setPen(FMFontCompareView::pens[QStringLiteral("control-line")]);
                }

                i += 2;
                curPos = c2;
            } else
                qCDebug(FONTMATRIX_LOG) << "Unknown point type" << cur.type;
        }
    }

    if (elems.testFlag(Metrics)) {
        double xadvance((path->data(GLYPH_DATA_HADVANCE).toDouble() * sf) + offset);
        QPointF XY(scene->views().constFirst()->mapToScene(0, 0));
        QPointF WH(scene->views().constFirst()->mapToScene(scene->views().constFirst()->width(), scene->views().constFirst()->height()));
        double minx(XY.x() + offset);
        double maxx(WH.x() + offset);
        double miny(XY.y());
        double maxy(WH.y());

        QLineF leftL(offset, miny, offset, maxy);
        QLineF rightL(xadvance, miny, xadvance, maxy);
        QLineF bottomL(minx, 0.0, maxx, 0.0);
        QPen mPen(penColor, 1.0);
        // 		mPen.setCosmetic(true);
        lines_controls << new QGraphicsLineItem(leftL);
        lines_controls.constLast()->setPen(mPen);
        lines_controls << new QGraphicsLineItem(rightL);
        lines_controls.constLast()->setPen(mPen);
        lines_controls << new QGraphicsLineItem(bottomL);
        lines_controls.constLast()->setPen(mPen);

        QString advanceString(QStringLiteral("%1/%2"));
        text_metrics << new QGraphicsSimpleTextItem(advanceString.arg(path->data(GLYPH_DATA_HADVANCE).toDouble()).arg(font->getUnitPerEm()));
        double th(text_metrics.constLast()->boundingRect().height());
        text_metrics.constLast()->setPos(xadvance, th * zindex);
        text_metrics.constLast()->setBrush(color);
    }

    toScreen();
}

/// View
QMap<QString, QPen> FMFontCompareView::pens;
QMap<QString, QBrush> FMFontCompareView::brushes;

FMFontCompareView::FMFontCompareView(QWidget *parent)
    : QGraphicsView(parent)
{
    setScene(new QGraphicsScene(0, -1000, 1000, 1000, this));
    scene()->setBackgroundBrush(Qt::white);
    setRenderHint(QPainter::Antialiasing, true);
    theRect = scene()->addRect(QRectF(), QPen(QColor(10, 10, 200)), QColor(10, 10, 200, 100));
    theRect->setZValue(1000.0);
    initPensAndBrushes();
    setInteractive(true);
    isSelecting = false;
    isPanning = false;
    setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    setTransformationAnchor(QGraphicsView::NoAnchor);
}

FMFontCompareView::~FMFontCompareView() = default;

void FMFontCompareView::changeFont(int level, FontItem *font)
{
    glyphs[level] = new FMFontCompareItem(scene(), font, level);
    elements[level] |= FMFontCompareItem::Contour;
    offsets[level] = 0.0;
    updateGlyphs();
}

void FMFontCompareView::removeFont(int level)
{
    if (glyphs.contains(level)) {
        if (glyphs.value(level))
            delete glyphs.value(level);
        glyphs.remove(level);
    }
    elements.remove(level);
    offsets.remove(level);
    int maxLevel(0);
    for (const auto glyphsKeys = glyphs.keys(); const auto &l : glyphsKeys) {
        maxLevel = qMax(maxLevel, l);
    }
    for (int i(level + 1); i <= maxLevel; ++i) {
        if (glyphs.contains(i)) {
            glyphs[i - 1] = glyphs[i];
            glyphs.remove(i);
            glyphs.value(i - 1)->setIndex(i - 1);

            elements[i - 1] = elements[i];
            elements.remove(i);

            offsets[i - 1] = offsets[i];
            offsets.remove(i);
        }
    }
    updateGlyphs();
}

void FMFontCompareView::changeChar(uint ccode)
{
    thechar = ccode;
    for (const auto glyphsKeysList = glyphs.keys(); const auto &l : glyphsKeysList) {
        glyphs.value(l)->setChar(thechar);
    }
    updateGlyphs();
}

void FMFontCompareView::changeChar(int level, uint ccode)
{
    if (glyphs.contains(level))
        glyphs.value(level)->setChar(ccode);
    updateGlyphs();
}

void FMFontCompareView::setElements(int level, FMFontCompareItem::GElements elems)
{
    // 	qDebug()<<"FMFontCompareView::setElements"<<level<<elems;
    elements[level] = elems;
    updateGlyphs();
}

FMFontCompareItem::GElements FMFontCompareView::getElements(int level)
{
    return elements.value(level);
}

void FMFontCompareView::setColor(int level, QColor color)
{
    colors[level] = color;
    updateGlyphs();
}

void FMFontCompareView::setOffset(int level, double offset)
{
    offsets[level] = offset;
    updateGlyphs();
}

double FMFontCompareView::getOffset(int level)
{
    return offsets.value(level);
}

void FMFontCompareView::initPensAndBrushes()
{
    // Controls
    pens[QStringLiteral("control-line")] = QPen(QColor(20, 20, 20), 1.0, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin);
    pens[QStringLiteral("control-point")] = QPen(Qt::black);
    pens[QStringLiteral("point")] = QPen(Qt::red);

    // Fill
    brushes[QStringLiteral("fill")] = QBrush(QColor(0, 0, 0, 32));
    pens[QStringLiteral("contour")] = QPen(QColor(20, 0, 200), 1.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);

    // 	QString cat("Compare/%1");
    // 	QSettings settings;
    // 	for (const auto& attr : pens.keys())
    // 	{
    // 		pens[attr].setColor( QColor(settings.value(cat.arg(attr),pens[attr].color().name()).toString()) );
    // 		settings.setValue(cat.arg(attr),pens[attr].color().name());
    // 	}
    // 	for (const auto& attr : brushes.keys())
    // 	{
    // 		brushes[attr].setColor( QColor(settings.value(cat.arg(attr),brushes[attr].color().name()).toString()) );
    // 		settings.setValue(cat.arg(attr),pens[attr].color().name());
    // 	}
}

void FMFontCompareView::updateGlyphs()
{
    for (const auto loopGlyphsKeys = glyphs.keys(); const auto &l : loopGlyphsKeys) {
        glyphs.value(l)->show(elements.value(l), colors.value(l), offsets.value(l));
    }
}

QColor FMFontCompareView::getColor(int level)
{
    // 	if(glyphs.contains(level))
    // 		return glyphs[level]->getColor();
    // 	return QColor();
    return colors.value(level);
}

void FMFontCompareView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MiddleButton) {
        mouseStartPoint = e->pos();
        isPanning = true;
        QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));
    } else {
        mouseStartPoint = mapToScene(e->pos());
        isSelecting = true;
    }
}

void FMFontCompareView::mouseReleaseEvent(QMouseEvent *e)
{
    if (isPanning) {
        isPanning = false;
        QApplication::restoreOverrideCursor();
        updateGlyphs();
        return;
    }
    if (!isSelecting)
        return;
    if (mouseStartPoint.toPoint() == mapToScene(e->pos()).toPoint()) {
        fitGlyphsView();
        isSelecting = false;
        theRect->setRect(QRectF());
        return;
    }

    QRect zoomRect(mouseStartPoint.toPoint(), mapToScene(e->pos()).toPoint());
    ensureVisible(zoomRect);
    isSelecting = false;
    fitInView(theRect->sceneBoundingRect(), Qt::KeepAspectRatio);
    theRect->setRect(QRectF());
}

void FMFontCompareView::mouseMoveEvent(QMouseEvent *e)
{
    if (isPanning) {
        QPointF pos(e->pos());
        int vDelta(qRound(mouseStartPoint.y() - pos.y()));
        int hDelta(qRound(mouseStartPoint.x() - pos.x()));
        verticalScrollBar()->setValue(verticalScrollBar()->value() + vDelta);
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + hDelta);
        mouseStartPoint = pos;
        updateGlyphs();
        return;
    }
    if (!isSelecting)
        return;

    QRectF r(mouseStartPoint, mapToScene(e->pos()));
    theRect->setRect(r);
}

void FMFontCompareView::wheelEvent(QWheelEvent *e)
{
    QPoint angleDelta = e->angleDelta();
    if (e->modifiers().testFlag(Qt::ControlModifier) && angleDelta.y() != 0) {
        double d(1.0 + (static_cast<double>(angleDelta.y()) / 1000.0));
        QTransform trans;
        trans.scale(d, d);
        setTransform(trans, true);
    } else {
        if (angleDelta.y() != 0)
            verticalScrollBar()->setValue(verticalScrollBar()->value() - angleDelta.y());
        if (angleDelta.x() != 0)
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - angleDelta.x());
    }
    updateGlyphs();
}

void FMFontCompareView::resizeEvent(QResizeEvent *)
{
    updateGlyphs();
}

void FMFontCompareView::fitGlyphsView()
{
    QRectF maxrect;
    for (const auto glyphLayers = glyphs.keys(); const auto &l : glyphLayers) {
        maxrect = maxrect.united(glyphs[l]->boundingRect());
    }
    double hratio(static_cast<double>(width()) / maxrect.width());
    double vratio(static_cast<double>(height()) / maxrect.height());
    double shape_ratio(qMin(hratio, vratio) * 0.9);
    double view_ratio(qMin(hratio, vratio) * 1.5);

    setTransform(QTransform());
    scale(shape_ratio, shape_ratio);

    QTransform m;
    m.scale(view_ratio, view_ratio);
    QRectF vr(m.mapRect(maxrect));
    ensureVisible(vr);
}

#include "moc_fmfontcompareview.cpp"

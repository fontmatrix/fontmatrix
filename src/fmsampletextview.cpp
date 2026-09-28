/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmsampletextview.h"
#include "fontmatrix_debug.h"
#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QGraphicsRectItem>
#include <QMouseEvent>
#include <QScrollBar>

#ifdef HAVE_QTOPENGL
#include <QGLWidget>
#endif

FMSampleTextView::FMSampleTextView(QWidget *parent)
    : QGraphicsView(parent)
    , hasPendingUpdate(false)
{
#if 0
	QGLFormat glfmt;
	glfmt.setSampleBuffers ( true );
	QGLWidget *glwgt = new QGLWidget ( glfmt );
// 	qDebug()<<"GL:: A DR S"<<glwgt->format().alpha()<<glwgt->format().directRendering()<<glwgt->format().sampleBuffers();
// 	setViewport(glwgt);
	if ( glwgt->format().sampleBuffers() )
	{
		setViewport ( glwgt );
		qCDebug(FONTMATRIX_LOG) <<"opengl enabled - DirectRendering("<< glwgt->format().directRendering() <<") - SampleBuffers("<< glwgt->format().sampleBuffers() <<")";
	}
	else
	{
		qCDebug(FONTMATRIX_LOG) <<"opengl disabled - DirectRendering("<< glwgt->format().directRendering() <<") - SampleBuffers("<< glwgt->format().sampleBuffers() <<")";
		delete glwgt;
	}
#endif

    setInteractive(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    theRect = nullptr;
    fPage = nullptr;
    isSelecting = false;
    isPanning = false;
    setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setRenderHint(QPainter::Antialiasing, true);
    setBackgroundBrush(Qt::white);
}

FMSampleTextView::~FMSampleTextView() = default;

void FMSampleTextView::resizeEvent(QResizeEvent *)
{
    Q_EMIT refit();
}

void FMSampleTextView::mousePressEvent(QMouseEvent *e)
{
    if (!scene())
        return;
    if (locker)
        return;

    if (e->button() == Qt::MiddleButton) {
        mouseStartPoint = e->pos();
        isPanning = true;
        QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));
    } else {
        ensureTheRect();
        mouseStartPoint = mapToScene(e->pos());
        isSelecting = true;
        // 		QRectF arect(mouseStartPoint, QSizeF());
        // 		theRect->setRect(arect);
    }
}

void FMSampleTextView::mouseReleaseEvent(QMouseEvent *e)
{
    if (isPanning) {
        isPanning = false;
        QApplication::restoreOverrideCursor();
        return;
    }
    if (!isSelecting)
        return;
    // 	qDebug()<<"End mouse is "<< mapToScene( e->pos()).toPoint();
    if (mouseStartPoint.toPoint() == mapToScene(e->pos()).toPoint()) {
        // scale(1,1)
        // 		qDebug() << "Re-init transformation";
        Q_EMIT pleaseZoom(0);
        isSelecting = false;
        theRect->setRect(QRectF());
        return;
    }

    QRect zoomRect(mouseStartPoint.toPoint(), mapToScene(e->pos()).toPoint());
    ensureVisible(zoomRect);
    isSelecting = false;
    // 	qDebug() << "release " << theRect->scenePos();
    fitInView(theRect->sceneBoundingRect(), Qt::KeepAspectRatio);
    theRect->setRect(QRectF());
}

void FMSampleTextView::mouseMoveEvent(QMouseEvent *e)
{
    if (isPanning) {
        QPointF pos(e->pos());
        int vDelta(qRound(mouseStartPoint.y() - pos.y()));
        int hDelta(qRound(mouseStartPoint.x() - pos.x()));
        verticalScrollBar()->setValue(verticalScrollBar()->value() + vDelta);
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + hDelta);
        mouseStartPoint = pos;
        return;
    }
    if (!isSelecting)
        return;

    QRectF r(mouseStartPoint, mapToScene(e->pos()));
    theRect->setRect(r);
}

void FMSampleTextView::ensureTheRect()
{
    if (theRect)
        return;
    theRect = scene()->addRect(QRectF(), QPen(QColor(10, 10, 200)), QColor(10, 10, 200, 100));
    theRect->setZValue(1000.0);
}

void FMSampleTextView::wheelEvent(QWheelEvent *e)
{
    // 	qDebug() << "log wheel event " << e->delta();
    // 	QGraphicsView::wheelEvent(e);
    if (locker)
        return;

    QPoint angleDelta = e->angleDelta();
    if (e->modifiers().testFlag(Qt::ControlModifier) && angleDelta.y() != 0) {
        Q_EMIT pleaseZoom(angleDelta.y());
    } else {
        if (angleDelta.y() != 0)
            verticalScrollBar()->setValue(verticalScrollBar()->value() - angleDelta.y());
        if (angleDelta.x() != 0)
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - angleDelta.x());
    }
}

void FMSampleTextView::showEvent(QShowEvent *event)
{
    if (hasPendingUpdate) {
        hasPendingUpdate = false;
        Q_EMIT pleaseUpdateMe();
    }
    QGraphicsView::showEvent(event);
}

void FMSampleTextView::sheduleUpdate()
{
    hasPendingUpdate = true;
}

void FMSampleTextView::unSheduleUpdate()
{
    hasPendingUpdate = false;
}

void FMSampleTextView::fakePage()
{
    if (fPage)
        return;

    fPage = scene()->addRect(sceneRect(), QPen(QColor(Qt::black)), QColor(Qt::white));
    setBackgroundBrush(Qt::lightGray);
}

#include "moc_fmsampletextview.cpp"

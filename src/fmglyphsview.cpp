/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fmglyphsview.h"
#include "fontmatrix_debug.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QScrollBar>

#ifdef HAVE_QTOPENGL
#include <QGLWidget>
#endif

FMGlyphsView::FMGlyphsView(QWidget *parent)
    : QGraphicsView(parent)
{
    // There is just one instance and we want to identify it
    setObjectName("theglyphsview");

#ifdef HAVE_QTOPENGL
    QGLFormat glfmt;
    glfmt.setSampleBuffers(true);
    QGLWidget *glwgt = new QGLWidget(glfmt);
    if (glwgt->format().sampleBuffers()) {
        setViewport(glwgt);
        qCDebug(FONTMATRIX_LOG) << "opengl enabled - DirectRendering(" << glwgt->format().directRendering() << ") - SampleBuffers("
                                << glwgt->format().sampleBuffers() << ")";
    } else {
        qCDebug(FONTMATRIX_LOG) << "opengl disabled - DirectRendering(" << glwgt->format().directRendering() << ") - SampleBuffers("
                                << glwgt->format().sampleBuffers() << ")";
        delete glwgt;
    }
#endif

    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(Qt::white);
    m_state = AllView;
    m_lock = false;
    m_oper = false;

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &FMGlyphsView::slotViewMoved);
}

FMGlyphsView::~FMGlyphsView() = default;

void FMGlyphsView::resizeEvent(QResizeEvent *)
{
    if (m_state == SingleView)
        Q_EMIT pleaseUpdateSingle();

    Q_EMIT pleaseUpdateMe();
}

void FMGlyphsView::showEvent(QShowEvent *)
{
    Q_EMIT pleaseUpdateMe();
}

void FMGlyphsView::mouseReleaseEvent(QMouseEvent *e)
{
    // 	Basically, we just do the job, but legacy implementation
    // 	does something I can’t figure out that leads to segfault ??
    if (e->button() == Qt::LeftButton) {
        QList<QGraphicsItem *> gg = scene()->items(mapToScene(e->pos()));
        for (auto *ii : std::as_const(gg)) {
            if (ii->data(1).toString() == QLatin1String("select") && m_state == AllView)
                ii->setSelected(true);
        }

        if (m_state == AllView)
            Q_EMIT pleaseShowSelected();
        else if (m_state == SingleView)
            Q_EMIT pleaseShowAll();
    }
}

void FMGlyphsView::mousePressEvent(QMouseEvent *)
{
    // We just catch it to avoid a waeird segfault ... we’ll see later for a plain fix
    // 	if(e->button() == Qt::LeftButton)
    // 		QGraphicsView::mouseReleaseEvent(e);
}

void FMGlyphsView::setState(ViewState s)
{
    if (s == SingleView) {
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setFocusPolicy(Qt::NoFocus);

    } else if (s == AllView) {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setFocusPolicy(Qt::WheelFocus);
    }
    m_state = s;
}

void FMGlyphsView::hideEvent(QHideEvent *)
{
    //	if ( m_state == SingleView )
    //		emit pleaseShowAll();
}

void FMGlyphsView::wheelEvent(QWheelEvent *e)
{
    if (m_state == AllView) {
        QGraphicsView::wheelEvent(e);
    }
}

QRectF FMGlyphsView::visibleSceneRect()
{
    QRectF rr(mapToScene(0.0, 0.0, static_cast<double>(width()), static_cast<double>(height())).boundingRect());
    return rr;
}

void FMGlyphsView::slotViewMoved(int)
{
    if (m_state == AllView)
        Q_EMIT pleaseUpdateMe();
}

void FMGlyphsView::keyPressEvent(QKeyEvent *e)
{
    // QGraphicsView's handler is skipped on purpose: no item of this scene
    // takes key focus. Keys scroll the grid and do nothing on a single glyph.
    if (m_state == AllView)
        QAbstractScrollArea::keyPressEvent(e); // NOLINT(bugprone-parent-virtual-call)
}

bool FMGlyphsView::lock()
{
    if (m_lock)
        return false;
    m_lock = true;
    return true;
}

void FMGlyphsView::unlock()
{
    m_lock = false;
}

#include "moc_fmglyphsview.cpp"

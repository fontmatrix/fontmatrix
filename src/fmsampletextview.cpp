/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "fmsampletextview.h"
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QScrollBar>
#include <QApplication>
#include <QCursor>
#include <QDebug>

#ifdef HAVE_QTOPENGL
#include <QGLWidget>
#endif

FMSampleTextView::FMSampleTextView ( QWidget* parent )
		: QGraphicsView ( parent ),
		hasPendingUpdate ( false )
{
#ifdef HAVE_QTOPENGL
	QGLFormat glfmt;
	glfmt.setSampleBuffers ( true );
	QGLWidget *glwgt = new QGLWidget ( glfmt );
// 	qDebug()<<"GL:: A DR S"<<glwgt->format().alpha()<<glwgt->format().directRendering()<<glwgt->format().sampleBuffers();
// 	setViewport(glwgt);
	if ( glwgt->format().sampleBuffers() )
	{
		setViewport ( glwgt );
		qDebug() <<"opengl enabled - DirectRendering("<< glwgt->format().directRendering() <<") - SampleBuffers("<< glwgt->format().sampleBuffers() <<")";
	}
	else
	{
		qDebug() <<"opengl disabled - DirectRendering("<< glwgt->format().directRendering() <<") - SampleBuffers("<< glwgt->format().sampleBuffers() <<")";
		delete glwgt;
	}
#endif

	setInteractive ( true );
	theRect = 0;
	fPage = 0;
	isSelecting = false;
	isPanning = false;
	setAlignment ( Qt::AlignTop | Qt::AlignHCenter );
	setTransformationAnchor ( QGraphicsView::NoAnchor );
	setRenderHint ( QPainter::Antialiasing, true );
}


FMSampleTextView::~FMSampleTextView()
{
}

void FMSampleTextView::resizeEvent ( QResizeEvent * event )
{
	emit refit();
}

void FMSampleTextView::mousePressEvent ( QMouseEvent * e )
{
	if ( !scene() )
		return;
	if ( locker )
		return;

	if ( e->button() == Qt::MidButton )
	{
		mouseStartPoint =  e->pos() ;
		isPanning = true;
		QApplication::setOverrideCursor (QCursor(Qt::ClosedHandCursor));
	}
	else
	{
		ensureTheRect();
		mouseStartPoint = mapToScene ( e->pos() );
		isSelecting = true;
// 		QRectF arect(mouseStartPoint, QSizeF());
// 		theRect->setRect(arect);
	}

}

void FMSampleTextView::mouseReleaseEvent ( QMouseEvent * e )
{
	if ( isPanning )
	{
		isPanning = false;
		QApplication::restoreOverrideCursor();
		return;
	}
	if ( !isSelecting )
		return;
// 	qDebug()<<"End mouse is "<< mapToScene( e->pos()).toPoint();
	if ( mouseStartPoint.toPoint() == mapToScene ( e->pos() ).toPoint() )
	{
		// scale(1,1)
// 		qDebug() << "Re-init transformation";
		emit pleaseZoom ( 0 );
		isSelecting = false;
		theRect->setRect ( QRectF() );
		return;
	}

	QRect zoomRect ( mouseStartPoint.toPoint(),mapToScene ( e->pos() ).toPoint() );
	ensureVisible ( zoomRect );
	isSelecting = false;
// 	qDebug() << "release " << theRect->scenePos();
	fitInView ( theRect->sceneBoundingRect(), Qt::KeepAspectRatio );
	theRect->setRect ( QRectF() );



}

void FMSampleTextView::mouseMoveEvent ( QMouseEvent * e )
{
	if ( isPanning )
	{
		QPointF pos ( e->pos() );
		int vDelta ( mouseStartPoint.y() - pos.y() );
		int hDelta ( mouseStartPoint.x() - pos.x() );
		verticalScrollBar()->setValue ( verticalScrollBar()->value() + vDelta );
		horizontalScrollBar()->setValue ( horizontalScrollBar()->value() + hDelta );
		mouseStartPoint = pos;
		return;
	}
	if ( !isSelecting )
		return;

	QRectF r ( mouseStartPoint,mapToScene ( e->pos() ) );
	theRect->setRect ( r );
}

void FMSampleTextView::ensureTheRect()
{
	if ( theRect )
		return;
	theRect = scene()->addRect ( QRectF(),QPen ( QColor ( 10,10,200 ) ), QColor ( 10,10,200,100 ) );
	theRect->setZValue ( 1000.0 );
}




void FMSampleTextView::wheelEvent ( QWheelEvent * e )
{
// 	qDebug() << "log wheel event " << e->delta();
// 	QGraphicsView::wheelEvent(e);
	if ( locker )
		return;

	if ( e->modifiers().testFlag ( Qt::ControlModifier ) && e->orientation() == Qt::Vertical )
	{
		emit pleaseZoom ( e->delta() );
	}
	else
	{
		if ( e->orientation() == Qt::Vertical )
			verticalScrollBar()->setValue ( verticalScrollBar()->value() - e->delta() );
		if ( e->orientation() == Qt::Horizontal )
			horizontalScrollBar()->setValue ( horizontalScrollBar()->value() - e->delta() );
	}
}

void FMSampleTextView::showEvent ( QShowEvent * event )
{
	if ( hasPendingUpdate )
	{
		hasPendingUpdate = false;
		emit pleaseUpdateMe();
	}
	QGraphicsView::showEvent ( event );
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
	if(fPage)
		return;
	
	fPage = scene()->addRect ( sceneRect() ,QPen ( QColor ( Qt::black ) ), QColor ( Qt::white ) );
	setBackgroundBrush(Qt::lightGray);
}

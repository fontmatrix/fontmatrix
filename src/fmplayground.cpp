//
// C++ Implementation: fmplayground
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmplayground.h"

#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>

FMPlayGround::FMPlayGround(QWidget *parent)
	:QGraphicsView(parent)
{
	setInteractive(true);
	setDragMode(RubberBandDrag);
	setRenderHint(QPainter::Antialiasing);
	
	isPanning = false;
}

FMPlayGround::~ FMPlayGround()
{
}



void FMPlayGround::mousePressEvent(QMouseEvent * e)
{
	if( e->button() == Qt::MidButton )
	{
		mouseStartPoint =  e->pos() ;
		isPanning = true;
	}
	QGraphicsView::mousePressEvent(e);
}

void FMPlayGround::mouseReleaseEvent(QMouseEvent * e)
{
	if(isPanning)
	{
		isPanning = false;
		return;
	}
	QGraphicsView::mouseReleaseEvent(e);
}

void FMPlayGround::mouseMoveEvent(QMouseEvent * e)
{
	if(isPanning)
	{
		QPointF pos(e->pos());
		int vDelta( mouseStartPoint.y() - pos.y() );
		int hDelta( mouseStartPoint.x() - pos.x() );
		verticalScrollBar()->setValue( verticalScrollBar()->value() + vDelta );
		horizontalScrollBar()->setValue( horizontalScrollBar()->value() + hDelta);
		mouseStartPoint = pos;
		return;
	}
	QGraphicsView::mouseMoveEvent(e);
}

void FMPlayGround::wheelEvent(QWheelEvent * e)
{
	if(e->modifiers().testFlag(Qt::ControlModifier) && e->orientation() == Qt::Vertical  )
	{
		emit pleaseZoom(e->delta());
	}
	else
	{
		if(e->orientation() == Qt::Vertical )
			verticalScrollBar()->setValue(verticalScrollBar()->value() - e->delta());
		if(e->orientation() == Qt::Horizontal)
			horizontalScrollBar()->setValue(horizontalScrollBar()->value() - e->delta());
	}
}
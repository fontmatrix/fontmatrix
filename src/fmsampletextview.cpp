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
#include <QDebug>

FMSampleTextView::FMSampleTextView(QWidget* parent)
 : QGraphicsView(parent)
{
	setInteractive(true);
	theRect = 0;
	isSelecting = false;
}


FMSampleTextView::~FMSampleTextView()
{
}

void FMSampleTextView::resizeEvent(QResizeEvent * event)
{
	emit refit();
}

void FMSampleTextView::mousePressEvent(QMouseEvent * e)
{
	if(!scene())
		return;
	if(locker)
		return;
	
	ensureTheRect();
	mouseStartPoint = mapToScene( e->pos() );
	qDebug() << "start mouse "<< mouseStartPoint;
	isSelecting = true;
	QRectF arect(mouseStartPoint, QSizeF());
	theRect->setRect(arect);
	
}

void FMSampleTextView::mouseReleaseEvent(QMouseEvent * e)
{
	if(!isSelecting)
		return;
	QRect zoomRect(mouseStartPoint.toPoint(),mapToScene( e->pos()).toPoint());
	ensureVisible(zoomRect);
	isSelecting = false;
// 	qDebug() << "release " << theRect->scenePos();
	fitInView(theRect->sceneBoundingRect(), Qt::KeepAspectRatio);
	theRect->setRect(QRectF());
	
	
	
}

void FMSampleTextView::mouseMoveEvent(QMouseEvent * e)
{
	if(!isSelecting)
		return;
	
	QRect r(mouseStartPoint.toPoint(),mapToScene(e->pos()).toPoint());
	theRect->setRect(r);
}

void FMSampleTextView::ensureTheRect()
{
	if(theRect)
		return;
	theRect = scene()->addRect(QRectF(),QPen ( QColor(10,10,200)), QColor(10,10,200,100));
	theRect->setZValue(1000.0);
}




void FMSampleTextView::wheelEvent(QWheelEvent * e)
{
// 	qDebug() << "log wheel event " << e->delta();
// 	QGraphicsView::wheelEvent(e);
	if(e->orientation() == Qt::Vertical )
		verticalScrollBar()->setValue(verticalScrollBar()->value() - e->delta());
	if(e->orientation() == Qt::Horizontal)
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() - e->delta());
}
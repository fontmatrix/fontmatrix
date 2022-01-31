//
// C++ Implementation: iview
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "iview.h"

#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>
#include <QPolygonF>
#include <QDebug>

IView::IView ( QWidget * parent )
		:QGraphicsView ( parent )
{
	curImage = 0;
	m_controlRect = false;
	setScene ( new QGraphicsScene );
	setInteractive ( true );
	theRect = QRect();
	isSelecting = false;
	isPanning = false;
	setAlignment ( Qt::AlignTop | Qt::AlignHCenter );
	setTransformationAnchor ( QGraphicsView::NoAnchor );
	setRenderHint ( QPainter::Antialiasing, true );


	curSel = scene()->addPolygon ( QPolygonF() ,QPen ( Qt::NoPen ), QColor ( 10,10,10,100 ) );
	curSel->setZValue ( 1.0 );
	
	QPen rctPen(Qt::blue, 2);
	rctPen.setCosmetic(true);
	curRect = scene()->addRect(QRect(),rctPen, QBrush(Qt::NoBrush));
	curRect->setZValue(10.0);
	
	QPen edgP(Qt::NoPen);
	edgP.setCosmetic(true);
	curTL = scene()->addEllipse(QRect(), edgP, Qt::red);
	curTR = scene()->addEllipse(QRect(), edgP, Qt::red);
	curBL = scene()->addEllipse(QRect(), edgP, Qt::red);
	curBR = scene()->addEllipse(QRect(), edgP, Qt::red);
	
	curTL->setZValue(100.0);
	curTR->setZValue(100.0);
	curBL->setZValue(100.0);
	curBR->setZValue(100.0);
	
	connect(this, SIGNAL(rectChange(QRect)), this, SLOT(drawSelRect(QRect)));
}

void IView::mouseMoveEvent ( QMouseEvent * e )
{
	////qDebug()<<"IView::mouseMoveEvent";
	if ( isPanning )
	{
		QPointF pos ( e->pos() );
		int vDelta ( qRound ( mouseStartPoint.y() - pos.y() ) );
		int hDelta ( qRound ( mouseStartPoint.x() - pos.x() ) );
		verticalScrollBar()->setValue ( verticalScrollBar()->value() + vDelta );
		horizontalScrollBar()->setValue ( horizontalScrollBar()->value() + hDelta );
		mouseStartPoint = pos;
		return;
	}
	else
	{
		QPointF pressPoint(mapToScene ( e->pos() ));
		
		QRectF rectTL(curTL->rect());
		rectTL.translate(curRect->rect().topLeft());
		
		QRectF rectTR(curTR->rect());
		rectTR.translate(curRect->rect().topRight());
		
		QRectF rectBR(curBR->rect());
		rectBR.translate(curRect->rect().bottomRight());
		
		QRectF rectBL(curBL->rect());
		rectBL.translate(curRect->rect().bottomLeft());
		
		if(rectTL.contains( pressPoint ))
		{
			QApplication::setOverrideCursor (Qt::SizeFDiagCursor);
		}
		else if(rectTR.contains( pressPoint ))
		{
			QApplication::setOverrideCursor (Qt::SizeBDiagCursor);
		}
		else if(rectBR.contains( pressPoint ))
		{
			QApplication::setOverrideCursor (Qt::SizeFDiagCursor);
		}
		else if(rectBL.contains( pressPoint ))
		{
			QApplication::setOverrideCursor (Qt::SizeBDiagCursor);
		}
		else
			QApplication::restoreOverrideCursor();
	}
	if ( !isSelecting )
		return;

	if(m_controlRect)
	{
		QPointF mp(mapToScene ( e->pos() ));
		{
			QRectF r ( mouseStartPoint, mp );
			theRect = r.normalized().toRect() ;
		}
		emit rectChange(theRect);
	}
}

void IView::mousePressEvent ( QMouseEvent * e )
{
	//qDebug()<<"IView::mousePressEvent";
	if ( !scene() )
		return;

	if ( e->button() == Qt::MiddleButton )
	{
		mouseStartPoint =  e->pos() ;
		isPanning = true;
		QApplication::setOverrideCursor ( QCursor ( Qt::ClosedHandCursor ) );
	}
	else
	{
		QPointF pressPoint(mapToScene ( e->pos() ));
		if(!m_controlRect)
			selectGlyph(pressPoint);
		else
		{
			QRectF rectTL(curTL->rect());
			rectTL.translate(curRect->rect().topLeft());

			QRectF rectTR(curTR->rect());
			rectTR.translate(curRect->rect().topRight());

			QRectF rectBR(curBR->rect());
			rectBR.translate(curRect->rect().bottomRight());

			QRectF rectBL(curBL->rect());
			rectBL.translate(curRect->rect().bottomLeft());

			if(rectTL.contains( pressPoint ))
			{
				mouseStartPoint = curRect->rect().bottomRight();
			}
			else if(rectTR.contains( pressPoint ))
			{
				mouseStartPoint = curRect->rect().bottomLeft();
			}
			else if(rectBR.contains( pressPoint ))
			{
				mouseStartPoint = curRect->rect().topLeft();
			}
			else if(rectBL.contains( pressPoint ))
			{
				mouseStartPoint = curRect->rect().topRight();
			}
			else
				mouseStartPoint = pressPoint;
		}
		isSelecting = true;
	}
}

void IView::mouseReleaseEvent ( QMouseEvent * e )
{
	//qDebug()<<"IView::mouseReleaseEvent";
	if ( isPanning )
	{
		isPanning = false;
		QApplication::restoreOverrideCursor();
		return;
	}
	if ( !isSelecting )
		return;

	isSelecting = false;
//	theRect = QRect();

}

void IView::setImage ( const QString & path )
{
	//qDebug()<<"IView::setImage";
	if ( curImage )
	{
		delete curImage;
		curImage = 0;
	}
	

	QImage i ( path );
	if ( i.isNull() )
		return;

	curImage = scene()->addPixmap ( QPixmap::fromImage ( i ) );
	fitImage();
	
	drawSelRect(QRect());
}

void IView::setImage(const QPixmap & pixmap)
{
	//qDebug()<<"IView::setImage";
	if ( curImage )
	{
		delete curImage;
		curImage = 0;
	}
	
	curImage = scene()->addPixmap ( pixmap );
	fitImage();
	
	drawSelRect(QRect());
}

QPixmap IView::getPixmap()
{
	//qDebug()<<"IView::getPixmap";
	if ( curImage )
	{
		return  curImage->pixmap();
	}
	return QPixmap();
}


void IView::drawSelRect(QRect r)
{
	//qDebug()<<"IView::drawSelRect";
	if(r.isNull())
	{
		curSel->setPolygon ( QPolygonF() );
		
		curRect->setRect(QRect());
		
		curTL->setRect(QRect());
		curTR->setRect(QRect());
		curBR->setRect(QRect());
		curBL->setRect(QRect());
		
		return;
		
	}
	
	// mask
	QPolygonF p;
	QRect ir(curImage->boundingRect().toRect());
	p << ir.topLeft() << ir.topRight() << ir.bottomRight() << ir.bottomLeft() << ir.topLeft() ;
	p << r.topLeft() << r.bottomLeft() << r.bottomRight() << r.topRight()<< r.topLeft() ;
	curSel->setPolygon (p);
	
	// rect
	curRect->setRect(r);
	
	
	// edges
	if(m_controlRect)
	{
		QRectF baseR(-4,-4,8,8);
		QTransform t;
		double hs(transform().m11());
		double vs(transform().m22());
		t.scale(1.0 / hs, 1.0 / vs);
		baseR = t.mapRect(baseR);

		curTL->setRect(baseR);
		curTR->setRect(baseR);
		curBR->setRect(baseR);
		curBL->setRect(baseR);

		curTL->setPos(r.topLeft());
		curTR->setPos(r.topRight());
		curBL->setPos(r.bottomLeft());
		curBR->setPos(r.bottomRight());
	}
	
	
}


void IView::fitImage()
{
	//qDebug()<<"IView::fitImage";
	if(!curImage)
		return;
	
	double wR = width() /curImage->boundingRect().width()  ;
	double hR =  height()/ curImage->boundingRect().height();
	
	double R = (wR > hR) ? hR : wR;
	QTransform T;
	T.scale(R,R);
	setTransform( T  , false);
	
}

void IView::resizeEvent(QResizeEvent * event)
{
	//qDebug()<<"View::resizeEvent";
	fitImage();
}

void IView::selectGlyph(const QPointF & scenepos)
{
	//qDebug()<<"IView::selectGlyph";
// 	const unsigned int treshold (5); 
	if(!curImage)
		return;
	
	QPoint imgp(curImage->mapFromScene(scenepos).toPoint());
	QImage cImg(curImage->pixmap().toImage());
	QRgb ref(cImg.pixel(imgp));
	
	
	// crop it
	QRect r;
	const int cw = cImg.width();
	const int ch = cImg.height();
	bool topReached(false);
	for(int y(0); y < ch; ++y)
	{
					
		for(int x(0);x <  cw; ++x)
		{
			if(cImg.pixel(x,y) == ref)
			{
				topReached = true;
				r.setTop(y);
				break;
			}
		}
		if(topReached)
			break;
	}
	bool bottomReached(false);
	for(int y(ch - 1); y >= 0; --y)
	{
					
		for(int x(cw-1);x >=0; --x)
		{
			if(cImg.pixel(x,y) == ref)
			{
				bottomReached = true;
				r.setBottom(y);
				break;
			}
		}
		if(bottomReached)
			break;
	}
	r.setLeft(cw);
	for(int y(0); y < ch; ++y)
	{
					
		for(int x(0);x <  cw; ++x)
		{
			if(cImg.pixel(x,y) == ref)
			{
				r.setLeft(qMin(x,r.left()));
				break;
			}
		}
	}
	r.setRight(0);
	for(int y(0); y < ch; ++y)
	{
					
		for(int x(cw -1);x >= 0; --x)
		{
			if(cImg.pixel(x,y) == ref)
			{
				r.setRight(qMax(x,r.right()));
				break;
			}
		}
	}
	//qDebug()<<"R"<<r.top()<<r.right()<<r.bottom()<<r.left();
	theRect = r;
	emit selColorChanged(ref);
	emit rectChange(theRect);
// 	drawSelRect(r);
}

void IView::setControlRect(bool u)
{
	//qDebug()<<"IView::setControlRect";
	m_controlRect = u;
	if (u)
	{
		//qDebug()<<"CR"<<theRect;
		emit rectChange(theRect);
	}
	else
	{
		curTL->setRect(QRect());
		curTR->setRect(QRect());
		curBR->setRect(QRect());
		curBL->setRect(QRect());
	}
}


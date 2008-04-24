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
#include "fontitem.h"

#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

FMPlayGround::FMPlayGround ( QWidget *parent )
		:QGraphicsView ( parent )
{
	setInteractive ( true );
	setDragMode ( RubberBandDrag );
	setRenderHint ( QPainter::Antialiasing );

	isPanning = false;
	CursorPos.rx() = 100;
	CursorPos.ry() = 100;
}

FMPlayGround::~ FMPlayGround()
{
}



void FMPlayGround::mousePressEvent ( QMouseEvent * e )
{
	if ( e->button() == Qt::MidButton )
	{
		mouseStartPoint =  e->pos() ;
		isPanning = true;
		return;
	}
	QGraphicsView::mousePressEvent ( e );
}

void FMPlayGround::mouseReleaseEvent ( QMouseEvent * e )
{
	if ( isPanning )
	{
		isPanning = false;
		return;
	}
	QGraphicsView::mouseReleaseEvent ( e );
}

void FMPlayGround::mouseMoveEvent ( QMouseEvent * e )
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
	QGraphicsView::mouseMoveEvent ( e );
}

void FMPlayGround::wheelEvent ( QWheelEvent * e )
{
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

#define DATA_PLAYGROUND 999999
void FMPlayGround::displayGlyphs ( const QString & spec, FontItem * fontI, double fontS )
{

	ensureVisible ( CursorPos.x(), CursorPos.y(), spec.count(), fontS * 1.5 );

	bool backedR ( fontI->rasterFreetype() );
	fontI->setFTRaster ( false );
	fontI->renderLine ( scene() , spec, CursorPos, spec.count() * fontS * 2.0, fontS, 1 ,false );
	fontI->setFTRaster ( backedR );

	QList< QGraphicsItem* > itemList ( scene()->items() );
	QList< QGraphicsItem* > tmpList;
	for ( int i ( 0 ); i < itemList.count(); ++i )
	{
		if ( itemList[i]->data( DATA_PLAYGROUND ).toString() == "playitem")
		{
			qDebug()<<"Item("<<i<<") is already a playground item";
		}
		else
		{
			qDebug()<<"Item("<<i<<") added to playground";
			tmpList << itemList[i];
			itemList[i]->setData ( DATA_PLAYGROUND, "playitem" );
		}
	}

	QGraphicsItemGroup *git ( scene()->createItemGroup ( tmpList ) );
	git->setData ( GLYPH_DATA_FONTNAME , fontI->fancyName() );
	git->setData ( DATA_PLAYGROUND, "playitem" );
	git->setToolTip(fontI->fancyName());
	git->setFlags ( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable );
	git->setCursor(QCursor(	Qt::OpenHandCursor ) );
	glyphLines << git;
	
	CursorPos.ry() += fontS * 1.5;

}

QStringList FMPlayGround::fontnameList()
{
	QStringList ret;
	QList< QGraphicsItem* > itemList ( scene()->items() );
	for ( int i ( 0 ); i < itemList.count(); ++i )
	{
		if ( itemList[i]->data ( GLYPH_DATA_GLYPH ).toString() == "glyph" )
		{
			QString s ( itemList[i]->data ( GLYPH_DATA_FONTNAME ).toString() );
			if ( !ret.contains ( s ) )
				ret << s;
		}
	}

	return ret;
}

QList<QGraphicsItemGroup* > FMPlayGround::getLines()
{
	return glyphLines;
}

QRectF FMPlayGround::getMaxRect()
{
	QRectF allrect(0,0,0,0);
	QList<QGraphicsItemGroup*> lit = glyphLines;
	for ( int i = 0 ; i <lit.count() ; ++i )
	{
// 		qDebug()<< lit.at(i)->data(GLYPH_DATA_FONTNAME).toString();
// 		
// 			if ( lit[i]->sceneBoundingRect().bottomRight().y() > allrect.bottomRight().y()
// 						  || lit[i]->sceneBoundingRect().bottomRight().x() > allrect.bottomRight().x()
// 						  || lit[i]->sceneBoundingRect().topLeft().y() > allrect.topLeft().y()
// 						  || lit[i]->sceneBoundingRect().topRight().y() > allrect.topRight().y() 
// 			   )
				allrect = allrect.united ( lit[i]->sceneBoundingRect() );
		

	}
	qDebug()<<"FMPlayGround::getMaxRect = "<< allrect;
	return allrect;
}


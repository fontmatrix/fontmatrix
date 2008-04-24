//
// C++ Interface: fmplayground
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMPLAYGROUND_H
#define FMPLAYGROUND_H

#include <QGraphicsView>
#include <QPointF>

class FontItem;

class FMPlayGround : public QGraphicsView
{
	Q_OBJECT
	public:
		FMPlayGround(QWidget *parent);
		~FMPlayGround();
		
		void displayGlyphs(const QString& spec, FontItem* fontI, double fontS);
		QStringList fontnameList();
		QList< QGraphicsItemGroup* > getLines();
		QRectF getMaxRect();
		
	protected:
		void mousePressEvent ( QMouseEvent * e ) ;
		void mouseReleaseEvent ( QMouseEvent * e )  ;
		void mouseMoveEvent ( QMouseEvent * e ) ;
		void wheelEvent ( QWheelEvent * e );
		
	private:
		QPointF mouseStartPoint;
		bool isPanning;
		QList<QGraphicsItemGroup*> glyphLines;
		
		QPointF CursorPos;
		
	signals:
		void pleaseZoom(int);
};


#endif

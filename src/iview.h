//
// C++ Interface: iview
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef IVIEW_H
#define IVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QRect>
#include <QPixmap>

class IView: public QGraphicsView
{
	Q_OBJECT
	public:
		IView(QWidget* parent);
		~IView(){}
		
		void setImage(const QString& path);
		void setImage(const QPixmap& pixmap);
		QPixmap getPixmap();

		// control if the rect can be tweak by user or not
		void setControlRect(bool u);

	signals:
		void rectChange(QRect);
		void selColorChanged(QRgb);
		
	protected:
		void mouseMoveEvent ( QMouseEvent * e );
		void mousePressEvent ( QMouseEvent * e );
		void mouseReleaseEvent ( QMouseEvent * e );
		void resizeEvent ( QResizeEvent * event );
		
	private:
		QGraphicsPixmapItem * curImage;
		
		QGraphicsPolygonItem * curSel;
		QGraphicsRectItem * curRect;
		QGraphicsEllipseItem * curTL;
		QGraphicsEllipseItem * curTR;
		QGraphicsEllipseItem * curBL;
		QGraphicsEllipseItem * curBR;
		
		QPointF mouseStartPoint;
		QRect theRect;
		bool isSelecting;
		bool isPanning;
		
		void fitImage();
		
		void selectGlyph(const QPointF& scenepos);

		bool m_controlRect;
		
	private slots:
		void drawSelRect(QRect);
		
		
};

#endif

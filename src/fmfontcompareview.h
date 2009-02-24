//
// C++ Interface: fmfontcompareview
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMFONTCOMPAREVIEW_H
#define FMFONTCOMPAREVIEW_H

#include <QGraphicsView>
#include <QMap>
#include <QUuid>

class QGraphicsLineItem;
class QGraphicsRectItem;
class QGraphicsEllipseItem;
class QGraphicsSimpleTextItem;
class FontItem;

class FMFontCompareItem
{
	public:
		enum GElement
		{
			Nothing		= 0,
			Contour 	= 0x1,
// 			Fill		= 0x2,
			Points		= 0x4,
			Controls	= 0x8,
			Metrics 	= 0x10
		};
		Q_DECLARE_FLAGS ( GElements,GElement )
		FMFontCompareItem();
		FMFontCompareItem ( QGraphicsScene * s, FontItem * f, int z );
		~FMFontCompareItem();
		void show ( GElements elems, QColor color, double offset = 0.0 );
		void setChar ( uint c ) {char_code = c;}
		QRectF boundingRect();
// 		QColor getColor() {return color;}
		void setIndex(int i){zindex = i;}
	private:
		const QUuid uuid;
		QGraphicsScene *scene;
		FontItem* font;
		int zindex;
		uint char_code;
// 		QColor color;
		QGraphicsPathItem* path;
		QList<QGraphicsLineItem*> lines_controls;
		QList<QGraphicsLineItem*> lines_metrics;
		QList<QGraphicsEllipseItem*> points;
		QList<QGraphicsSimpleTextItem*> text_metrics;

		void clear();
		void drawPoint ( QPointF point , bool control );
		void toScreen();
		
		static const QString toolTipModel;

};
Q_DECLARE_OPERATORS_FOR_FLAGS ( FMFontCompareItem::GElements )


class FMFontCompareView : public QGraphicsView
{
		Q_OBJECT
	public:
		FMFontCompareView ( QWidget * parent );
		~FMFontCompareView();

		void changeFont ( int level, FontItem* font );
		void removeFont ( int level );
		void changeChar ( uint ccode );
		void changeChar ( int level, uint ccode );
		void setElements ( int level, FMFontCompareItem::GElements elems );
		FMFontCompareItem::GElements getElements ( int level );
		void setColor ( int level, QColor color);
		QColor getColor ( int level );
		void setOffset (int level, double offset);
		double getOffset(int level);
		void fitGlyphsView();

		static QMap<QString, QPen> pens;
		static QMap<QString, QBrush> brushes;

	public slots:
		void updateGlyphs();

	protected:
		void mousePressEvent ( QMouseEvent * e ) ;
		void mouseReleaseEvent ( QMouseEvent * e )  ;
		void mouseMoveEvent ( QMouseEvent * e ) ;
		void wheelEvent ( QWheelEvent * e );
		void resizeEvent ( QResizeEvent * event );

	private:
		QMap<int, FMFontCompareItem*> glyphs; // < Z-index, glyph >
		QMap<int, FMFontCompareItem::GElements> elements; // what to show
		QMap<int, QColor> colors;
		QMap<int, double> offsets;

		void initPensAndBrushes();
		uint thechar;



		QPointF mouseStartPoint;
		QGraphicsRectItem *theRect;
		QGraphicsRectItem *fPage;
		bool isSelecting;
		bool isPanning;
};

#endif

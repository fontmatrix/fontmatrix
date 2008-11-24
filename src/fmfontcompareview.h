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
class FontItem;

class FMFontCompareItem
{
	public:
		enum GElement{Fill, Controls, Metrics};
		Q_DECLARE_FLAGS ( GElements,GElement )
		FMFontCompareItem();
		FMFontCompareItem ( QGraphicsScene * s, FontItem * f, int z );
		~FMFontCompareItem();
		void show ( GElements elems );
		void setChar(uint c){char_code = c;}
		QRectF boundingRect();
		QColor getColor(){return color;}
	private:
		const QUuid uuid;
		QGraphicsScene *scene;
		FontItem* font;
		int zindex;
		uint char_code;
		QColor color;
		QGraphicsPathItem* path;
		QList<QGraphicsLineItem*> lines_controls;
		QList<QGraphicsLineItem*> lines_metrics;
		QList<QGraphicsRectItem*> points;

		void clear();
		void drawPoint ( QPointF point );
		void toScreen();
		
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
		void show ( int level, FMFontCompareItem::GElements elems );
		QColor getColor(int level);

		static QMap<QString, QPen> pens;
		static QMap<QString, QBrush> brushes;

	public slots:
		void updateGlyphs();

	private:
		QMap<int, FMFontCompareItem*> glyphs; // < Z-index, glyph >
		QMap<int, FMFontCompareItem::GElements> elements; // what to show

		void initPensAndBrushes();
		uint thechar;
};

#endif

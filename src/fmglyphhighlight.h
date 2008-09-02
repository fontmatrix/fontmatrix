//
// C++ Interface: FMGlyphHighlight
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef FMGLYPHHIGHLIGHT_H
#define FMGLYPHHIGHLIGHT_H

#include <QObject>
#include <QPointF>

class QGraphicsRectItem;
class QTimeLine;

class FMGlyphHighlight : QObject
{
	Q_OBJECT
	public:
		FMGlyphHighlight(QGraphicsRectItem* rect);
		~FMGlyphHighlight();
	private:
		QGraphicsRectItem *m_rect;
		QTimeLine *m_timeline;
		QPointF initialPos;
		int maxFrame;
		
		void lastFrame();
		
	private slots:
		void animate(int);
};

#endif

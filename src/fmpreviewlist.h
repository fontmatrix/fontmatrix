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
#ifndef FMPREVIEWLIST_H
#define FMPREVIEWLIST_H

#include <QGraphicsView>

class QGraphicsPixmapItem;
class QGraphicsScene;
class FontItem;
class MainViewWidget;
class QGraphicsRectItem;
class QGraphicsItem;

struct FontPreviewItem
{
	QString name;
	QPointF pos;
	bool visible;
	QGraphicsPixmapItem* item;
	FontPreviewItem(QString n, QPointF p, bool v, QGraphicsPixmapItem* i)
	:name(n), pos(p), visible(v), item(i) {};
};

/**
	@author Pierre Marchand <pierre@oep-h.com>
	@brief FMPreviewList, as QGraphicsView descendant, is a facility to manage the preview list with QtDesigner
*/
class FMPreviewList : public QGraphicsView
{
		Q_OBJECT
	public:
		FMPreviewList ( QWidget* parent);

		~FMPreviewList();
		void setRefWidget(MainViewWidget* m){mvw = m;};
		void searchAndSelect(QString fname);
		
	public slots:
		void slotRefill ( QList<FontItem*> fonts , bool setChanged );
		void slotSelect ( QGraphicsItem* it );
		void slotClearSelect();
	private slots:
		void slotChanged();
		
	private:
		
		
		QGraphicsScene *m_scene;
		QList<FontPreviewItem> m_pixItemList;
		QGraphicsRectItem* m_select;
		MainViewWidget *mvw;
		QGraphicsItem *m_currentItem;
		
	protected:
		void showEvent ( QShowEvent * event ) ;
		void mousePressEvent ( QMouseEvent * e );

};

#endif

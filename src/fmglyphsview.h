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
#ifndef FMGLYPHSVIEW_H
#define FMGLYPHSVIEW_H

#include <QGraphicsView>
#include <QRectF>

/**
	@author Pierre Marchand <pierre@oep-h.com>
*/
class FMGlyphsView : public QGraphicsView
{
		Q_OBJECT
	public:
		
		enum ViewState{AllView,SingleView};
		
		FMGlyphsView ( QWidget *parent );
		~FMGlyphsView();
		
		void setState(ViewState s);
		ViewState state(){return m_state;}
		
		QRectF visibleSceneRect();
		
	private:
		ViewState m_state;
		
	private slots:
		void slotViewMoved();

	signals:
		/**
			forward new width, allowing FontItem::renderAll() to adjust the number of columns
		*/
		void refit ( int );
		void pleaseShowSelected();
		void pleaseShowAll();
		void pleaseUpdateMe();
		
	protected:

		void resizeEvent ( QResizeEvent * event );
		void showEvent ( QShowEvent * event ) ;
		void hideEvent ( QHideEvent * event );
		void mouseReleaseEvent ( QMouseEvent * e );
		void mousePressEvent ( QMouseEvent * e ) ;
		void wheelEvent ( QWheelEvent * e );
		void scrollContentsBy ( int dx, int dy );
		void keyPressEvent ( QKeyEvent * e );
};

#endif

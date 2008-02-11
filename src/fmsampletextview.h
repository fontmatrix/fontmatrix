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
#ifndef FMSAMPLETEXTVIEW_H
#define FMSAMPLETEXTVIEW_H

#include <QGraphicsView>
#include <QPointF>


/**
	@author Pierre Marchand <pierre@oep-h.com>
*/
class FMSampleTextView : public QGraphicsView
{
		Q_OBJECT

	public:
		FMSampleTextView ( QWidget* parent );

		~FMSampleTextView();
		
		bool locker;

	protected:
		void resizeEvent ( QResizeEvent * event );
		void mousePressEvent ( QMouseEvent * e ) ;
		void mouseReleaseEvent ( QMouseEvent * e )  ;
		void mouseMoveEvent ( QMouseEvent * e ) ;
		void wheelEvent ( QWheelEvent * e );
		void showEvent ( QShowEvent * event ) ;

	signals:
		void refit();
		void pleaseUpdateMe();
		void pleaseZoom(int);
		
	private:
		QPointF mouseStartPoint;
		QGraphicsRectItem *theRect;
		bool isSelecting;
		bool isPanning;
		
		void ensureTheRect();

};

#endif

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

/**
	@author Pierre Marchand <pierre@oep-h.com>
*/
class FMGlyphsView : public QGraphicsView
{
		Q_OBJECT
	public:
		FMGlyphsView ( QWidget *parent );

		~FMGlyphsView();

	signals:
		/**
			forward new width, allowing FontItem::renderAll() to adjust the number of columns
		*/
		void refit ( int );
	protected:

		void resizeEvent ( QResizeEvent * event );
		void showEvent ( QShowEvent * event ) ;
};

#endif

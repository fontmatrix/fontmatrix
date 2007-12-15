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
#include "fmglyphsview.h"

#include <QDebug>
#include <QMouseEvent>

FMGlyphsView::FMGlyphsView(QWidget *parent)
 : QGraphicsView(parent)
{
	// There is just one instance and we want to identify it
	setObjectName("theglyphsview");
	
	setOptimizationFlags ( QGraphicsView::DontClipPainter | QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);
	
}


FMGlyphsView::~FMGlyphsView()
{
}

void FMGlyphsView::resizeEvent(QResizeEvent * event)
{
	emit refit(width());
}

void FMGlyphsView::showEvent(QShowEvent * event)
{
	emit refit(width());
}

void FMGlyphsView::mouseReleaseEvent(QMouseEvent * e)
{
// 	qDebug() << "Catch release event";
	if(e->button() == Qt::LeftButton)
		QGraphicsView::mouseReleaseEvent(e);
}





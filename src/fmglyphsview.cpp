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
#include <QGraphicsItem>
#include <QScrollBar>

FMGlyphsView::FMGlyphsView(QWidget *parent)
 : QGraphicsView(parent)
{
	// There is just one instance and we want to identify it
	setObjectName("theglyphsview");
	setAlignment (Qt::AlignLeft | Qt::AlignTop);
	m_state = AllView;
	
}


FMGlyphsView::~FMGlyphsView()
{
}

void FMGlyphsView::resizeEvent(QResizeEvent * event)
{
	emit pleaseUpdateMe();
}

void FMGlyphsView::showEvent(QShowEvent * event)
{
	emit pleaseUpdateMe();
}

void FMGlyphsView::mouseReleaseEvent(QMouseEvent * e)
{
// 	Basically, we just do the job, but legacy implementation
// 	does something I can’t figure out that leads to segfault ??
	if(e->button() == Qt::LeftButton)
	{
		QList<QGraphicsItem*> gg = scene()->items(mapToScene(e->pos()));
		foreach(QGraphicsItem* ii, gg)
		{
			if(ii->data(1).toString() == "select" && m_state == AllView)
				ii->setSelected(true);
		}
		
		if(m_state == AllView)
			emit pleaseShowSelected();
		else if( m_state == SingleView)
			emit pleaseShowAll();
	}
}

void FMGlyphsView::mousePressEvent(QMouseEvent * e)
{
	// We just catch it to avoid a waeird segfault ... we’ll see later for a plain fix
// 	if(e->button() == Qt::LeftButton)
// 		QGraphicsView::mouseReleaseEvent(e);
}

void FMGlyphsView::setState(ViewState s)
{
	if(s == SingleView)
	{
		setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
		setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
		setFocusPolicy(Qt::NoFocus);
		
	}
	else if(s == AllView)
	{
		
		setVerticalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
		setHorizontalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
		setFocusPolicy(Qt::WheelFocus);
		
	}
	m_state = s;
}

void FMGlyphsView::hideEvent(QHideEvent * event)
{
	if(m_state == SingleView)
		emit pleaseShowAll();
}

void FMGlyphsView::wheelEvent(QWheelEvent * e)
{
	if(m_state == AllView)
		QGraphicsView::wheelEvent(e);
}

QRectF FMGlyphsView::visibleSceneRect()
{
	QRectF rr(mapToScene(0.0, 0.0, width(), height()).boundingRect());
	return rr;
}

void FMGlyphsView::slotViewMoved()
{
	if(m_state == AllView)
		emit pleaseUpdateMe();
}

void FMGlyphsView::scrollContentsBy(int dx, int dy)
{
// 	qDebug() << "FMGlyphsView::scrollContentsBy(int "<<dx<<", int "<<dy<<")";
	QAbstractScrollArea::scrollContentsBy ( dx,  dy );
	int pos = verticalScrollBar()->value();
	if(pos != 0 || m_state == AllView)
		emit pleaseUpdateMe();
// 	qDebug() << "bang";
}

void FMGlyphsView::keyPressEvent(QKeyEvent * e)
{
	if(m_state == AllView)
		QAbstractScrollArea::keyPressEvent(e);
}





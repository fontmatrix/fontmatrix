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
#include "fmpreviewlist.h"

#include "typotek.h"
#include "fontitem.h"
#include "mainviewwidget.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QGraphicsRectItem>


FMPreviewList::FMPreviewList(QWidget* parent)
 : QGraphicsView(parent)
{
	m_scene = new QGraphicsScene;
	setScene(m_scene);
	m_select = m_scene->addRect(QRectF(), QPen(Qt::blue ), QColor(0,0,120,60));
	m_select->setZValue(100.0);
	
	connect( (QObject*)verticalScrollBar(), SIGNAL(valueChanged( int )),
		this, SLOT(slotChanged(int)) );
	
}


FMPreviewList::~FMPreviewList()
{
}

void FMPreviewList::slotRefill(QList<FontItem*> fonts)
{
	
// 	typotek *typo = typotek::getInstance();
	if(!isVisible())
		return;
	for(int i= 0;i<m_pixItemList.count(); ++i)
	{
		QGraphicsPixmapItem *pit = m_pixItemList.at(i);
		if(pit)
		{
			m_scene->removeItem(pit);
			delete pit;
		}
	}
	m_pixItemList.clear();
	QRect vvrect(visibleRegion().boundingRect());
	QRect vrect = mapToScene( vvrect ).boundingRect().toRect();
	int beginPos= vrect.top();
	int endPos = vrect.bottom();
// 	qDebug()<< beginPos <<beginPos / 32 << endPos;
	if(beginPos <0)
	{
		endPos += beginPos * -1;
		beginPos = 0;
// 		qDebug()<< beginPos <<beginPos / 32 << endPos;
	}
	
	QPixmap padPix(1,1);
	QGraphicsPixmapItem *padPixItem = m_scene->addPixmap(padPix);
	m_pixItemList.append(padPixItem);
	for(int i= 0 ; i < fonts.count() ; ++i)
	{
		if((i + 1)*32 >= beginPos && i*32 <= endPos)
		{ 
			FontItem *fit = fonts.at(i);
			if(fit)
			{
				QGraphicsPixmapItem *pit = m_scene->addPixmap(fit->oneLinePreviewPixmap());
				pit->setPos(10,32*i);
				pit->setData(1,fit->name());
				pit->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	// 			pit->setZValue(10);
				m_pixItemList.append(pit);
				
			}
		}
		else
		{
			padPixItem->setPos(10,32*i);
		}
	}
}

void FMPreviewList::showEvent(QShowEvent * event)
{
	slotRefill(mvw->curFonts());
}

void FMPreviewList::slotChanged(int )
{
	slotRefill(mvw->curFonts());
}

void FMPreviewList::mousePressEvent(QMouseEvent * e)
{
	QGraphicsItem *item = m_scene->itemAt(mapToScene( e->pos() ));
	if(!item)
		return;
	
	QString fname(item->data(1).toString());
	item->setSelected(true);
	m_select->setRect(item->boundingRect());
	m_select->setPos(item->pos());
	
	qDebug() << fname;
	mvw->slotFontSelectedByName(fname);
}



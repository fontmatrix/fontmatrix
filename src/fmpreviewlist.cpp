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
#include <QScrollBar>


FMPreviewList::FMPreviewList(QWidget* parent)
 : QGraphicsView(parent)
{
	m_scene = new QGraphicsScene;
	setScene(m_scene);
// 	m_scene->setBackgroundBrush(Qt::lightGray);
	m_select = m_scene->addRect(QRectF(), QPen(Qt::blue ), QColor(0,0,120,60));
	m_select->setZValue(100.0);
	
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
	
	connect( verticalScrollBar(), SIGNAL(valueChanged( int )), this, SLOT(slotChanged()) );
	connect( verticalScrollBar(), SIGNAL(sliderReleased()),this,SLOT(slotChanged()));
	
}


FMPreviewList::~FMPreviewList()
{
}

void FMPreviewList::slotRefill(QList<FontItem*> fonts, bool setChanged)
{
// 	qDebug() << "FMPreviewList::slotRefill(QList<FontItem*> "<<&fonts<<", bool "<<setChanged<<")";
	if(setChanged)
	{
		slotClearSelect();
		trackedFonts = fonts;
	}
	
	for(int i = 0; i < m_pixItemList.count(); ++i)
	{
		
		if(m_pixItemList[i].item && m_pixItemList[i].visible)
		{
			m_scene->removeItem(m_pixItemList[i].item);
			delete m_pixItemList[i].item;
		}
	}
	m_pixItemList.clear();
	
	QRect vvrect(visibleRegion().boundingRect());
	QRect vrect = mapToScene( vvrect ).boundingRect().toRect();
	int beginPos= vrect.top();
	int endPos = vrect.bottom();
	if(beginPos <0)
	{
		endPos += beginPos * -1;
		beginPos = 0;
	}
	
	QPixmap padPix(1,1);
	QGraphicsPixmapItem *padPixItem = m_scene->addPixmap(padPix);
// 	m_pixItemList.append(FontPreviewItem("NONE",QPointF(),false,padPixItem));
	for(int i= 0 ; i < fonts.count() ; ++i)
	{
		if((i + 1)*32 >= beginPos && i*32 <= endPos)
		{ 
			FontItem *fit = fonts.at(i);
			if(fit)
			{
				QGraphicsPixmapItem *pit = m_scene->addPixmap(fit->oneLinePreviewPixmap("hamburgefonstiv"));
				pit->setPos(50,32*i);
				pit->setData(1,fit->name());
				pit->setData(2,"preview");
// 				pit->setZValue(1000);
				pit->setToolTip(fit->fancyName());
				pit->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	// 			pit->setZValue(10);
				m_pixItemList.append(FontPreviewItem(fit->name(), pit->pos(), true, pit));
				
			}
		}
		else
		{
			padPixItem->setPos(10,32*i);
			m_pixItemList.append(FontPreviewItem(fonts.at(i)->name(),padPixItem->pos(),false,padPixItem));
		}
	}
}

void FMPreviewList::showEvent(QShowEvent * event)
{
	slotRefill(mvw->curFonts(), false);
}

void FMPreviewList::slotChanged( )
{
	if(verticalScrollBar()->isSliderDown())
		return;
	slotRefill(mvw->curFonts(), false);
}

void FMPreviewList::mousePressEvent(QMouseEvent * e)
{
	qDebug() << "FMPreviewList::mousePressEvent(QMouseEvent * "<<e<<")";
	
	QPointF inListPos = mapToScene( e->pos() );
	inListPos.rx() = 100;
	QList<QGraphicsItem*> items = m_scene->items(inListPos);
	
	if(items.isEmpty())
	{
		qDebug() << "\t No item under  "<<e->pos()<<")";
		return;
	}
	QGraphicsItem* it = 0;;
	for(int i = 0; i < items.count();++i)
	{
		if(items[i]->data(2).toString() == "preview")
		{
			it = items[i];
			break;
		}
	}
	if(it == 0)
		return;
	
	if(it == m_currentItem)
		return;

	
	
	
	slotSelect(it);
	
}

void FMPreviewList::slotSelect(QGraphicsItem * it)
{
	QString fontname = it->data(1).toString();
	qDebug() << "FMPreviewList::slotSelect(QGraphicsItem * "<<fontname<<")";
	if(!it)
		return;
	it->setSelected(true);
	m_currentItem = it;
	m_select->setRect(it->boundingRect());
	m_select->setPos(it->pos());
	ensureVisible(it);
	if(isVisible())
		mvw->slotFontSelectedByName(fontname);
}

void FMPreviewList::slotClearSelect()
{
	m_currentItem = 0;
	m_select->setRect(QRectF());
	verticalScrollBar()->setValue(0);
}

void FMPreviewList::searchAndSelect(QString fname)
{
	qDebug() << "FMPreviewList::searchAndSelect(QString "<<fname<<")";
	QGraphicsItem *it = 0;
	for(int i = 0 ; i < m_pixItemList.count() ; ++i)
	{
// 		qDebug() << m_pixItemList[i].name;
		if(m_pixItemList[i].name == fname)
		{
			if(m_pixItemList[i].visible)
			{
				it = m_pixItemList[i].item;
				break;
			}
			else
			{
				FontItem *fit = typotek::getInstance()->getFont(fname);
				if(fit)
				{
					QGraphicsPixmapItem *pit = m_scene->addPixmap(fit->oneLinePreviewPixmap("hamburgefonstiv"));
					pit->setPos(m_pixItemList[i].pos);
					pit->setData(1,fit->name());
					pit->setData(2,"preview");
					pit->setToolTip(fit->fancyName());
					pit->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
					m_pixItemList[i].visible = true;
					m_pixItemList[i].item = pit;
				
				}
				it = m_pixItemList[i].item;
				break;
			}
		}
	}
	if(!it)
		return;
	slotSelect(it);
}

void FMPreviewList::resizeEvent(QResizeEvent * event)
{
	slotRefill(trackedFonts, false);
}



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
#include <QDesktopWidget>


FMPreviewList::FMPreviewList(QWidget* parent)
 : QGraphicsView(parent)
{
	m_scene = new QGraphicsScene;
	setScene(m_scene);
	setAlignment (Qt::AlignLeft | Qt::AlignTop);
// 	m_scene->setBackgroundBrush(Qt::lightGray);
	m_select = m_scene->addRect(QRectF(), QPen(Qt::blue ), QColor(0,0,120,60));
	m_select->setZValue(100.0);
	
	m_currentItem = 0;
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
	
	theWord = typotek::getInstance()->word();
	
	connect( verticalScrollBar(), SIGNAL(valueChanged( int )), this, SLOT(slotChanged()) );
	connect( verticalScrollBar(), SIGNAL(sliderReleased()),this,SLOT(slotChanged()));
	
}


FMPreviewList::~FMPreviewList()
{
}

void FMPreviewList::slotRefill(QList<FontItem*> fonts, bool setChanged)
{
	theWord = typotek::getInstance()->word();
// 	qDebug() << "FMPreviewList::slotRefill(QList<FontItem*> "<<&fonts<<", bool "<<setChanged<<")";
	if(setChanged)
	{
		slotClearSelect();
		trackedFonts.clear();
		QMap<QString, FontItem*> ordFonts;
		for(int i=0;i<fonts.count();++i)
			ordFonts[fonts[i]->fancyName()]=fonts[i];
		for(QMap<QString, FontItem*>::const_iterator fit = ordFonts.begin() ; fit != ordFonts.end(); ++fit)
			trackedFonts << fit.value();
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
	
	double theSize = typotek::getInstance()->getPreviewSize();
	double theLine = 1.3 * theSize * QApplication::desktop()->physicalDpiY() / 72.0;
	double indent = 50.0;
	
	QPixmap padPix(1,1);
	QGraphicsPixmapItem *padPixItem = m_scene->addPixmap(padPix);
// 	m_pixItemList.append(FontPreviewItem("NONE",QPointF(),false,padPixItem));
	for(int i= 0 ; i < trackedFonts.count() ; ++i)
	{
		if((i + 1)*theLine >= beginPos && i*theLine <= endPos)
		{ 
			FontItem *fit = trackedFonts.at(i);
			if(fit)
			{
				bool oldRaster = fit->rasterFreetype();
				fit->setFTRaster("true");
				QGraphicsPixmapItem *pit = m_scene->addPixmap(fit->oneLinePreviewPixmap(theWord));
				fit->setFTRaster(oldRaster);
				pit->setPos(indent ,theLine*i);
				pit->setData(1,fit->path());
				pit->setData(2,"preview");
// 				pit->setZValue(1000);
				pit->setToolTip(fit->fancyName());
				pit->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	// 			pit->setZValue(10);
				m_pixItemList.append(FontPreviewItem(fit->path(), pit->pos(), true, pit));
				
			}
		}
		else
		{
			padPixItem->setPos(10,theLine*i);
			m_pixItemList.append(FontPreviewItem(fonts.at(i)->path(),padPixItem->pos(),false,padPixItem));
		}
	}
	horizontalScrollBar()->setValue(0);
}

void FMPreviewList::showEvent(QShowEvent * event)
{
	slotRefill(mvw->curFonts(), false);
}

void FMPreviewList::slotChanged( )
{
#ifndef HIGH_PERF
	// It waits you release the slider to draw items
	if(verticalScrollBar()->isSliderDown())
		return;
#endif
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
	theWord = typotek::getInstance()->word();
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
					bool oldRaster = fit->rasterFreetype();
					fit->setFTRaster("true");
					QGraphicsPixmapItem *pit = m_scene->addPixmap(fit->oneLinePreviewPixmap(theWord));
					fit->setFTRaster(oldRaster);
					pit->setPos(m_pixItemList[i].pos);
					pit->setData(1,fit->path());
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

void FMPreviewList::keyPressEvent(QKeyEvent * e)
{
	QString ref;
	if( mvw->selectedFont())
		ref = mvw->selectedFont()->path() ;
	else
	{
		searchAndSelect(trackedFonts[0]->path());
		return;
	}
	
	if(e->key() == Qt::Key_Up)
	{
// 		verticalScrollBar()->setValue(verticalScrollBar()->value() - 32);
		
		QString target;
		for(int i = 1; i < trackedFonts.count(); ++i)
		{
			if(trackedFonts[i]->path() == ref)
			{
				target = trackedFonts[i-1]->path();
				break;
			}
		}
		if(!target.isEmpty())
			searchAndSelect(target);
		
	}
	else if(e->key() == Qt::Key_Down)
	{
// 		verticalScrollBar()->setValue(verticalScrollBar()->value() + 32);	
		QString target;
		for(int i = 0; i < trackedFonts.count() - 1; ++i)
		{
			if(trackedFonts[i]->path() == ref)
			{
				target = trackedFonts[i+1]->path();
				break;
			}
		}
		if(!target.isEmpty())
			searchAndSelect(target);
	}
}



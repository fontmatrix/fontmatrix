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
	m_select = m_scene->addRect(QRectF(), QPen(Qt::transparent ), QColor(255,216,158,100));
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
// 	qDebug()<<"FMPreviewList::slotRefill("<< fonts.count() <<","<< setChanged <<")";
	theWord = typotek::getInstance()->word();
	horizontalScrollBar()->setValue(0);
	if(setChanged)
	{
		QString rescueId = " %1";
		int rescueInt = 0;
		slotClearSelect();
		trackedFonts.clear();
		QMap<QString, FontItem*> ordFonts;
		for(int i=0;i<fonts.count();++i)
		{
			QString fId = fonts[i]->fancyName();
			if(ordFonts.contains(fId))
			{
				fId += rescueId.arg(++rescueInt);
// 				qDebug()<<"un doublon" << fId;
			}
			ordFonts[fId] = fonts[i];
		}
// 		qDebug()<< "ordFonts.count()->"<<ordFonts.count();
		
		for(QMap<QString, FontItem*>::const_iterator fit = ordFonts.begin() ; fit != ordFonts.end(); ++fit)
		{
			trackedFonts << fit.value();
		}
	}
	
	for(QMap<QString, FontPreviewItem>::const_iterator pit = m_pixItemList.begin() ; pit!= m_pixItemList.end(); ++pit)
	{
		
		if(pit.value().item && pit.value().visible)
		{
			m_scene->removeItem(pit.value().item);
			delete pit.value().item;
// 			pit.value().item = 0;
// 			pit.value().visible = false;
		}
	}
	m_pixItemList.clear();
	
	double theSize = typotek::getInstance()->getPreviewSize();
	double theLine = 1.3 * theSize * QApplication::desktop()->physicalDpiY() / 72.0;
	double indent = 50.0;
	
	QRect vvrect(visibleRegion().boundingRect());
	QRect vrect = mapToScene( vvrect ).boundingRect().toRect();
	int beginPos= vrect.top();
	int endPos = vrect.bottom();
	if(beginPos <0)
	{
		endPos += beginPos * -1;
		beginPos = 0;
	}
	
	m_scene->setSceneRect(0,0, width(), trackedFonts.count() * theLine);
	double visibilityAdjustment = theLine / 2.0;
	for(int i = 0 ; i < trackedFonts.count() ; ++i)
	{
		if( ((i + 1)*theLine) > (beginPos + visibilityAdjustment) && (i*theLine)  < (endPos - visibilityAdjustment))
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
				pit->setToolTip(fit->fancyName()/*+" - "+ QString::number(theLine*i)*/);
				pit->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
				m_pixItemList[fit->path()] = FontPreviewItem(fit->path(), QPointF(indent , theLine*i), true, pit) ;
// 				if(fit->path() == curFontName)
// 					qDebug()<< i << m_pixItemList[fit->path()].dump();
				
			}
		}
		else
		{
			m_pixItemList[trackedFonts.at(i)->path()] = FontPreviewItem(trackedFonts.at(i)->path(),QPointF(0,theLine*i),false,0);
// 			if( fonts.at(i)->path() == curFontName)
// 				qDebug()<< i << m_pixItemList[fonts.at(i)->path()].dump();
		}
	}
// 	qDebug()<< "END OF reFill()";
	
}

void FMPreviewList::showEvent(QShowEvent * event)
{
	slotRefill(mvw->curFonts(), false);
}

void FMPreviewList::slotChanged( )
{
// 	qDebug()<<"FMPreviewList::slotChanged( )";
#ifndef HIGH_PERF
	// It waits you release the slider to draw items
	if(verticalScrollBar()->isSliderDown())
		return;
#endif
	slotRefill(trackedFonts, false);
}

void FMPreviewList::mousePressEvent(QMouseEvent * e)
{
// 	qDebug() << "FMPreviewList::mousePressEvent(QMouseEvent * "<<e<<")";
	
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
	
	slotSelect(it->data(1).toString());
	
}

void FMPreviewList::slotSelect(QString fontname)
{
// 	qDebug() << "FMPreviewList::slotSelect(QGraphicsItem * "<<fontname<<")";
	if(fontname.isEmpty())
		return;
	curFontName = fontname;
	FontPreviewItem it = m_pixItemList[fontname];
// 	qDebug() << it.dump();
	
	if(!it.visible)
	{
		double topShift = 0.0;
		verticalScrollBar()->setValue(it.pos.y() + topShift);
	}
// 	ensureVisible(QRectF(0,it.pos.y(), width(), height()));
// 	qDebug() << "scroll to " << it.pos.y() << "and it was " <<(it.pos.x() == 0 ? "invisible" : "visible");
	
	if(!m_pixItemList.contains(fontname))
	{
		qDebug()<< "m_pixItemList does not contain " << fontname;
		return;
	}
	FontPreviewItem nit = m_pixItemList[fontname];
// 	qDebug()<< nit.dump();
	if(!nit.item)
	{
		qDebug()<<"OOPS, the graphic item has not been properly instanciated "<<nit.item;
		return;
	}
	nit.item->setSelected(true);
	m_currentItem = nit.item;
// 	double theSize = typotek::getInstance()->getPreviewSize();
// 	double theLine = 1.4 * theSize * QApplication::desktop()->physicalDpiY() / 72.0;
// 	
	QRectF itRect;
	QPointF itPos ( nit.pos );
	itRect.setWidth(width());
	itRect.setHeight(nit.item->boundingRect().height());
	itPos.rx() = 0.0;
	
	m_select->setRect(itRect);
	m_select->setPos(itPos);
	
	if(isVisible())
		mvw->slotFontSelectedByName(fontname);
}

void FMPreviewList::slotClearSelect()
{
// 	qDebug()<<" FMPreviewList::slotClearSelect()";
	m_currentItem = 0;
	m_select->setRect(QRectF());
	verticalScrollBar()->setValue(0);
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
		slotSelect(trackedFonts[0]->path());
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
			slotSelect(target);
		
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
			slotSelect(target);
	}
}



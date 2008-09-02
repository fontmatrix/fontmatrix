//
// C++ Implementation: FMGlyphHighlight
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmglyphhighlight.h"

#include <QGraphicsRectItem>
#include <QTimeLine>
#include <QBrush>
#include <QPen>
#include <QDebug>	

FMGlyphHighlight::FMGlyphHighlight(QGraphicsRectItem * rect)
{
// 	qDebug()<<"Create an HighLight";
	m_rect = new QGraphicsRectItem(rect->rect(), 0, rect->scene());
	m_rect->setZValue(10000);
	initialPos = m_rect->pos();
	m_timeline = new QTimeLine(300);
	maxFrame = 12;
	m_timeline->setFrameRange(0,maxFrame);
	
	connect( m_timeline,SIGNAL(frameChanged(int)), this, SLOT(animate (int)) );
	m_timeline->start();
}

FMGlyphHighlight::~ FMGlyphHighlight()
{
	delete m_timeline;
	delete m_rect;
}

void FMGlyphHighlight::animate(int frame)
{
// 	qDebug()<<"ANIM"<<frame;
	if(frame == maxFrame)
	{
		lastFrame();
		return;
	}
	// pos
// 	double shift((double)frame/20.0);
// 	m_rect->setPos(initialPos.x() + shift, initialPos.y()+shift);
// 	
// 	// scalelyphhighlight.h
// 	double scale(frame / maxFrame);
// 	QMatrix mat;
// 	mat.scale(scale,scale);
// 	m_rect->setMatrix(mat);
// 	
	// color
	QColor c(0,0,0, 255 - ( (frame * 255)/maxFrame ) );
// 	QColor c(255,0,0,255);
	m_rect->setBrush(QBrush(c));
	m_rect->setPen(QPen(c));
}

void FMGlyphHighlight::lastFrame()
{
// 	qDebug("LAST FRAME");
	deleteLater();
}

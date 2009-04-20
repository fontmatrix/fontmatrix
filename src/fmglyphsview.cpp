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

#ifdef HAVE_QTOPENGL
#include <QGLWidget>
#endif

FMGlyphsView::FMGlyphsView ( QWidget *parent )
		: QGraphicsView ( parent )
{
	// There is just one instance and we want to identify it
	setObjectName ( "theglyphsview" );

#ifdef HAVE_QTOPENGL
	QGLFormat glfmt;
	glfmt.setSampleBuffers ( true );
	QGLWidget *glwgt = new QGLWidget ( glfmt );
	if ( glwgt->format().sampleBuffers() )
	{
		setViewport ( glwgt );
		qDebug() <<"opengl enabled - DirectRendering("<< glwgt->format().directRendering() <<") - SampleBuffers("<< glwgt->format().sampleBuffers() <<")";
	}
	else
	{
		qDebug() <<"opengl disabled - DirectRendering("<< glwgt->format().directRendering() <<") - SampleBuffers("<< glwgt->format().sampleBuffers() <<")";
		delete glwgt;
	}
#endif

	setAlignment ( Qt::AlignLeft | Qt::AlignTop );
	setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
	setBackgroundBrush ( Qt::white );
	m_state = AllView;
	m_lock = false;
	m_oper = false;

	connect ( verticalScrollBar() , SIGNAL ( valueChanged ( int ) ), this, SLOT ( slotViewMoved ( int ) ) );

}


FMGlyphsView::~FMGlyphsView()
{
}

void FMGlyphsView::resizeEvent ( QResizeEvent * event )
{
	if ( m_state == SingleView )
		emit pleaseUpdateSingle();

	emit pleaseUpdateMe();

}

void FMGlyphsView::showEvent ( QShowEvent * event )
{
	emit pleaseUpdateMe();
}

void FMGlyphsView::mouseReleaseEvent ( QMouseEvent * e )
{
// 	Basically, we just do the job, but legacy implementation
// 	does something I can’t figure out that leads to segfault ??
	if ( e->button() == Qt::LeftButton )
	{
		QList<QGraphicsItem*> gg = scene()->items ( mapToScene ( e->pos() ) );
		foreach ( QGraphicsItem* ii, gg )
		{
			if ( ii->data ( 1 ).toString() == "select" && m_state == AllView )
				ii->setSelected ( true );
		}

		if ( m_state == AllView )
			emit pleaseShowSelected();
		else if ( m_state == SingleView )
			emit pleaseShowAll();
	}
}

void FMGlyphsView::mousePressEvent ( QMouseEvent * e )
{
	// We just catch it to avoid a waeird segfault ... we’ll see later for a plain fix
// 	if(e->button() == Qt::LeftButton)
// 		QGraphicsView::mouseReleaseEvent(e);
}

void FMGlyphsView::setState ( ViewState s )
{
	if ( s == SingleView )
	{
		setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
		setFocusPolicy ( Qt::NoFocus );

	}
	else if ( s == AllView )
	{

		setVerticalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
		setFocusPolicy ( Qt::WheelFocus );

	}
	m_state = s;
}

void FMGlyphsView::hideEvent ( QHideEvent * event )
{
	if ( m_state == SingleView )
		emit pleaseShowAll();
}

void FMGlyphsView::wheelEvent ( QWheelEvent * e )
{
	if ( m_state == AllView )
	{
		QGraphicsView::wheelEvent ( e );
	}
}

QRectF FMGlyphsView::visibleSceneRect()
{
	QRectF rr ( mapToScene ( 0.0, 0.0, static_cast<double> ( width() ), static_cast<double> ( height() ) ).boundingRect() );
	return rr;
}

void FMGlyphsView::slotViewMoved ( int v )
{
	if ( m_state == AllView )
		emit pleaseUpdateMe();
}


void FMGlyphsView::keyPressEvent ( QKeyEvent * e )
{
	if ( m_state == AllView )
		QAbstractScrollArea::keyPressEvent ( e );
}

bool FMGlyphsView::lock()
{
	if ( m_lock )
		return false;
	m_lock = true;
	return true;
}

void FMGlyphsView::unlock()
{
	m_lock = false;
}





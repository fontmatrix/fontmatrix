//
// C++ Implementation: fmplayground
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmplayground.h"
#include "fontitem.h"
#include "fmglyphhighlight.h"
#include "typotek.h"
#include "mainviewwidget.h"
#include "playwidget.h"

#include <QApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

#ifdef HAVE_QTOPENGL
#include <QGLWidget>
#endif

FMPlayGround::FMPlayGround ( QWidget *parent )
	:QGraphicsView ( parent )
{

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

	setInteractive ( true );
	setDragMode ( RubberBandDrag );
	setRenderHint ( QPainter::Antialiasing );
	setBackgroundBrush(Qt::white);

	isPanning = false;
	CursorPos.rx() = 100;
	CursorPos.ry() = 100;
	BlinkPos = CursorPos;
	CursorTimer = new QTimer(this);
	CursorTimer->setInterval(1000);
	CursorTimer->stop();
	connect(CursorTimer, SIGNAL(timeout()), this, SLOT(blinkCursor()));
}

FMPlayGround::~ FMPlayGround()
{
}



void FMPlayGround::mousePressEvent ( QMouseEvent * e )
{
	closeLine();
	mouseStartPoint =  e->pos() ;
	if ( e->button() == Qt::MiddleButton )
	{
		isPanning = true;
		return;
	}
	QGraphicsView::mousePressEvent ( e );
	QList<QGraphicsItem*> sel(scene()->selectedItems());
	if(sel.isEmpty())
	{
		curSelRect = QRectF();
	}
	else
	{
		foreach(const QGraphicsItem *i, sel)
		{
			curSelRect = curSelRect.united( i->boundingRect() );
		}
		curSelRect = mapFromScene(curSelRect).boundingRect();
	}
}

void FMPlayGround::mouseReleaseEvent ( QMouseEvent * e )
{
	if ( isPanning )
	{
		isPanning = false;
		return;
	}
	else if((e->button() == Qt::LeftButton))
	{
		QRect cursorArea(mouseStartPoint.x() - 2, mouseStartPoint.y() - 2, 4, 4);

		if(!curSelRect.contains(e->pos()))
		{
			if(cursorArea.contains(e->pos()))
			{
				BlinkPos = CursorPos = mapToScene( e->pos() );
				blinkCursor();
				CursorTimer->start();
			}
		}
	}
	QGraphicsView::mouseReleaseEvent ( e );
}

void FMPlayGround::mouseMoveEvent ( QMouseEvent * e )
{
	if ( isPanning )
	{
		QPointF pos ( e->pos() );
		int vDelta ( qRound(mouseStartPoint.y() - pos.y()) );
		int hDelta ( qRound(mouseStartPoint.x() - pos.x()) );
		verticalScrollBar()->setValue ( verticalScrollBar()->value() + vDelta );
		horizontalScrollBar()->setValue ( horizontalScrollBar()->value() + hDelta );
		mouseStartPoint = pos;
		return;
	}
	QGraphicsView::mouseMoveEvent ( e );
}

void FMPlayGround::wheelEvent ( QWheelEvent * e )
{
	if ( e->modifiers().testFlag ( Qt::ControlModifier ) && e->orientation() == Qt::Vertical )
	{
		emit pleaseZoom ( e->delta() );
	}
	else
	{
		if ( e->orientation() == Qt::Vertical )
			verticalScrollBar()->setValue ( verticalScrollBar()->value() - e->delta() );
		if ( e->orientation() == Qt::Horizontal )
			horizontalScrollBar()->setValue ( horizontalScrollBar()->value() - e->delta() );
	}
}


void FMPlayGround::keyReleaseEvent(QKeyEvent * e)
{
	if(e->key() == Qt::Key_Delete)
		removeLine();
	else if(e->key() == Qt::Key_Escape)
	{
		deselectAll();
		closeLine();
	}
	else if((e->key() == Qt::Key_Enter)
		|| (e->key() == Qt::Key_Return))
		{
		closeLine();
		blinkCursor();
		CursorTimer->start();
	}
	else if(e->key() == Qt::Key_Backspace)
	{
		if(curString.count() > 0)
		{
			curString.chop(1);
			updateLine();
		}
		else
			removeLine();
	}
	else if(e->modifiers().testFlag(Qt::ControlModifier))
	{
		if(Qt::Key_A == e->key())
		{
			closeLine();
			foreach(QGraphicsItemGroup *gi, glyphLines)
			{
				gi->setSelected(true);
			}
		}
		else if(Qt::Key_V == e->key())
		{
			QString subtype("plain");
			QString clipText( QApplication::clipboard()->text(subtype, QClipboard::Clipboard) );
			if(!clipText.isEmpty())
			{
				QStringList cs(clipText.split(QString("\n")));
				bool first(true);
				foreach(QString s, cs)
				{
					if(first)
					{
						first = false;
						curString += s;
					}
					else
						curString = s;
					updateLine();
					closeLine();
				}
			}
		}

	}
	else
	{
		if(!e->text().isEmpty())
		{
			curString += e->text();
			updateLine();
		}
	}
}

void FMPlayGround::leaveEvent(QEvent *e)
{
	closeLine();
}

void FMPlayGround::displayGlyphs ( const QString & spec, FontItem * fontI, double fontS )
{

	ensureVisible ( CursorPos.x(), CursorPos.y(), spec.count(), fontS * 1.5 );
	bool backedR ( fontI->rasterFreetype() );
	fontI->setFTRaster ( false );
	// We deactivate "non-latin" layout atm
	//	TextProgression *tp = TextProgression::getInstance();
	QPointF pen(CursorPos);

	foreach(RenderedGlyph g, fontI->glyphs( spec , fontS ) )
	{
		QGraphicsPathItem* glyph(fontI->itemFromGindex(g.glyph, fontS));
		if(!glyph)
			continue;
		curLine << glyph;
		//		if ( tp->inLine() == TextProgression::INLINE_RTL )
		//		{
		//			pen.rx() -= g.xadvance ;
		//		}
		//		else if ( tp->inLine() == TextProgression::INLINE_BTT )
		//		{
		//			pen.ry() -= g.yadvance ;
		//		}
		glyph->setPen(Qt::NoPen);
		scene()->addItem(glyph);
		glyph->setPos ( pen.x() + ( g.xoffset ),
				pen.y() + ( g.yoffset ) );
		//		if ( tp->inLine() == TextProgression::INLINE_LTR )
		pen.rx() += g.xadvance ;
		//		else if ( tp->inLine() == TextProgression::INLINE_TTB )
		//			pen.ry() += g.yadvance;

	}
	fontI->setFTRaster ( backedR );
	BlinkPos = pen;

}

void FMPlayGround::updateLine()
{
	CursorTimer->stop();
	FontItem * fi(typotek::getInstance()->getTheMainView()->selectedFont());
	if(fi)
	{
		foreach(QGraphicsItem * item, curLine)
			delete item;
		curLine.clear();
		displayGlyphs(curString, fi, PlayWidget::getInstance()->playFontSize());
		blinkCursor();
	}
	CursorTimer->start();
}

void FMPlayGround::closeLine()
{
	CursorTimer->stop();
	if(curLine.count() > 0)
	{
		QGraphicsItemGroup *git(scene()->createItemGroup(curLine));
		CursorPos.ry() += PlayWidget::getInstance()->playFontSize() * 1.5;
		curLine.clear();
		curString.clear();
		git->setFlags ( QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable );
		git->setCursor(QCursor(	Qt::OpenHandCursor ) );
		FontItem * fi(typotek::getInstance()->getTheMainView()->selectedFont());
		if(fi)
			git->setToolTip(QString("<strong>%1</strong><br/><em>%2<em/>").arg(fi->fancyName()).arg(fi->path()));
		glyphLines << git;
	}
	BlinkPos = CursorPos;
}


void FMPlayGround::deselectAll()
{
	foreach(QGraphicsItemGroup *gi, glyphLines)
	{
		gi->setSelected(false);
	}
}

QStringList FMPlayGround::fontnameList()
{
	QStringList ret;
	QList< QGraphicsItem* > itemList ( scene()->items() );
	for ( int i ( 0 ); i < itemList.count(); ++i )
	{
		if ( itemList[i]->data ( GLYPH_DATA_GLYPH ).toString() == "glyph" )
		{
			QString s ( itemList[i]->data ( GLYPH_DATA_FONTNAME ).toString() );
			if ( !ret.contains ( s ) )
				ret << s;
		}
	}

	return ret;
}

QList<QGraphicsItemGroup* > FMPlayGround::getLines()
{
	return glyphLines;
}

QRectF FMPlayGround::getMaxRect()
{
	QRectF allrect(0,0,0,0);
	QList<QGraphicsItemGroup*> lit = glyphLines;
	for ( int i = 0 ; i <lit.count() ; ++i )
	{
		// 		qDebug()<< lit.at(i)->data(GLYPH_DATA_FONTNAME).toString();
		//
		// 			if ( lit[i]->sceneBoundingRect().bottomRight().y() > allrect.bottomRight().y()
		// 						  || lit[i]->sceneBoundingRect().bottomRight().x() > allrect.bottomRight().x()
		// 						  || lit[i]->sceneBoundingRect().topLeft().y() > allrect.topLeft().y()
		// 						  || lit[i]->sceneBoundingRect().topRight().y() > allrect.topRight().y()
		// 			   )
		allrect = allrect.united ( lit[i]->sceneBoundingRect() );
		

	}
	qDebug()<<"FMPlayGround::getMaxRect = "<< allrect;
	return allrect;
}

void FMPlayGround::removeLine()
{
	QList<QGraphicsItemGroup*> tmpL(glyphLines);
	foreach(QGraphicsItemGroup* ig,  tmpL)
	{
		if(ig->isSelected())
		{
			scene()->removeItem(ig);
			glyphLines.removeAll( ig );
			delete ig;
		}
	}
}


void FMPlayGround::blinkCursor()
{
	double h(PlayWidget::getInstance()->playFontSize());
	new FMGlyphHighlight(scene(), QRectF(BlinkPos.x() -1, BlinkPos.y() - h , 1, h));
}



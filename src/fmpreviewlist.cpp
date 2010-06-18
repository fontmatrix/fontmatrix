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
#include "fmfloatingpreview.h"
#include "fmfontdb.h"

#include <QImage>
#include <QDebug>
#include <QSettings>
#include <QScrollBar>
//#include <QDrag>
//#include <QMimeData>
#include <QApplication>
#include <QLabel>

QVector<QRgb> FMPreviewIconEngine::m_selPalette;

FMPreviewIconEngine::FMPreviewIconEngine()
		:QIconEngineV2()
{
	if(m_selPalette.isEmpty())
	{
		QColor sColor( QApplication::palette().color(QPalette::Highlight) );
		QColor tColor( QApplication::palette().color(QPalette::HighlightedText) );
		m_selPalette.clear();
		int sr(sColor.red());
		int sg(sColor.green());
		int sb(sColor.blue());
		int tr(tColor.red());
		int tg(tColor.green());
		int tb(tColor.blue());
		int cpal(256);
		for ( int aa = 0; aa < cpal ; ++aa )
		{
			int sn(cpal - aa);
			int tn(aa);
			m_selPalette << qRgb (((sr*sn) + (tr*tn)) /cpal,
					      ((sg*sn) + (tg*tn)) /cpal,
					      ((sb*sn) + (tb*tn)) /cpal );
		}
	}
}

QVector<QRgb> FMPreviewIconEngine::actualSelPalette(const QVector<QRgb>& orig)
{
	QRgb r(QApplication::palette().color(QPalette::Highlight).rgb());
	QVector<QRgb> ret;
	for(int i(0); i<256; ++i)
		ret << r;
	QColor bgColor(QApplication::palette().color(QPalette::Base));
	QColor fgColor(QApplication::palette().color(QPalette::Text));

	// In m_selPalette, background is at the begining of the vector

	bool DarkOnLight(fgColor.rgb() < bgColor.rgb());
	// order is dark first, light last
	QMap<QRgb, int> order;
	for(int i(0); i < orig.count(); ++i)
		order[orig[i]] = i;
	QList<int> oIdx(order.values());
	if(DarkOnLight)
	{
		// oIdx has background values at the end
		int v(0);
		for(int c(oIdx.count() - 1); c >= 0 ; --c)
		{
			ret [oIdx[c]] =  m_selPalette[v];
			++v;
		}
	}
	else
	{
		int v(0);
		for(int c(0); c < oIdx.count() ; c++)
		{
			ret [oIdx[c]] =  m_selPalette[v];
			++v;
		}
	}
	return ret;
}


FMPreviewIconEngine::~FMPreviewIconEngine()
{
//	if(m_p)
//		delete m_p;
}

void FMPreviewIconEngine::paint ( QPainter * painter, const QRect & rect, QIcon::Mode mode, QIcon::State state )
{
	if(!m_p.isNull())
	{
		painter->save();
		painter->translate(rect.x(),rect.y());
		QRect r(0 , 0 , rect.width(), rect.height());
		if(mode == QIcon::Selected)
		{
			QImage hm(m_p.toImage().convertToFormat(QImage::Format_Indexed8));
			hm.setColorTable(actualSelPalette(hm.colorTable()));
			painter->drawPixmap(r, QPixmap::fromImage(hm) , r);
		}
		else
			painter->drawPixmap(r, m_p , r);
		painter->restore();
	}
}

void FMPreviewIconEngine::addPixmap ( const QPixmap & pixmap, QIcon::Mode mode, QIcon::State state )
{
	m_p = pixmap;
}


FMPreviewModel::FMPreviewModel( QObject * pa , FMPreviewView * wPa,  QList<FontItem*> db )
	: QAbstractListModel(pa) , m_view(wPa), base(db)
{
	QSettings settings;
	styleTooltipName = settings.value("Preview/StyleTooltipName","font-weight:bold;").toString();
	styleTooltipPath = settings.value("Preview/StyleTooltipPath","font-weight:normal;font-size:small;").toString();
	
	settings.setValue("Preview/StyleTooltipName", styleTooltipName);
	settings.setValue("Preview/StyleTooltipPath", styleTooltipPath);
}

QVariant FMPreviewModel::data(const QModelIndex & index, int role) const
{	
	if(!index.isValid())
		return QVariant();

	int row = index.row();
// 	qDebug()<<"D"<<row;
	FontItem *fit;
	if(base.isEmpty())
		fit = FMFontDb::DB()->getFilteredFonts(true).at(row);
	else
		fit = base.at(row);
	if(!fit)
		return QVariant();

	qDebug()<<"Data"<< fit->fancyName();
	
	QColor bgColor(QApplication::palette().color(QPalette::Base));
	QColor fgColor(QApplication::palette().color(QPalette::Text));
// 	int borders( 2*(m_view->frameWidth() + m_view->lineWidth() + m_view->midLineWidth()) ); 
// 	int scrollbar(m_view->verticalScrollBar()->width());
// 	int width( m_view->width() - (borders + scrollbar) );
// 	qDebug()<<"W"<<width<<borders<<scrollbar;
	// quite strange but I cannot determine accuratly the size of the visible part of the inner frame :/
// 	int width( qRound(m_view->width() * 0.92) );
	int width(m_view->getUsedWidth());
	
	if(role == Qt::DisplayRole)
	{
		if( typotek::getInstance()->getPreviewSubtitled() )
			return fit->fancyName() ;
		else
			return QVariant();
	}
	else if(role == Qt::DecorationRole)
	{
		QString word;
		if(specString.isEmpty())
			word = typotek::getInstance()->word(fit);
		else
			word = typotek::getInstance()->word(fit, specString);
		QPixmap im(fit->oneLinePreviewPixmap(word,fgColor, bgColor, width ) );
		QIcon ic( new FMPreviewIconEngine  );
		ic.addPixmap(im);
//		if(fit->path() == QString("/home/pierre/fontes/ttf/A028-Ext.ttf"))
//			im.save("im.png");
		return ic;
	}
	else if(role == Qt::ToolTipRole)
	{
		if(typotek::getInstance()->getPreviewSubtitled())
		{
			return QString("<div style=\"" + styleTooltipPath + "\">" + fit->path() + "</div>");
		}
		else
		{
			QString complete;
			complete += "<div style=\"" + styleTooltipName + "\">" + fit->fancyName() + "</div>";
			complete += "\n";
			complete += "<div style=\"" + styleTooltipPath + "\">" + fit->path() + "</div>";
			return complete;
		}
	}
	else if(role == PathRole)
	{
		return fit->path();
	}
	
	// fall back
	return QVariant();
	
}

Qt::ItemFlags FMPreviewModel::flags(const QModelIndex & index) const
{
	return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

int FMPreviewModel::rowCount(const QModelIndex & parent) const
{
	if(parent.isValid() || !typotek::getInstance()->getTheMainView())
		return 0;
	int cl(0);
	if(base.isEmpty())
		cl = FMFontDb::DB()->getFilteredFonts(true).count();
	else
		cl = base.count();
	return cl;
}

void FMPreviewModel::dataChanged()
{
	QAbstractItemModel::dataChanged(index(0),index(rowCount(QModelIndex()) - 1));
	m_view->updateLayout();
	emit layoutChanged ();
}

void FMPreviewModel::resetBase(QList<FontItem *>db)
{
	base = db;
	dataChanged();
}

QList<FontItem *> FMPreviewModel::getBase()
{
	if(base.isEmpty())
		return FMFontDb::DB()->getFilteredFonts(true);
	else
		return base;
}


FMPreviewView::FMPreviewView(QWidget * parent)
	:QListView(parent), columns(1)
{
	dragFlag = false;
	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::DragDrop);
}

void FMPreviewView::resizeEvent(QResizeEvent * event)
{
	int borders( 2*(frameWidth() + lineWidth() + midLineWidth()) ); 
	int scrollbar(verticalScrollBar()->width());
//	usedWidth = qRound((this->width() - (borders + scrollbar)) / columns);
	usedWidth = qRound(double(this->width())  / columns);
//	emit widthChanged(usedWidth);
	setIconSize(QSize(qRound(usedWidth), 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
}

void FMPreviewView::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		startDragPoint = event->pos();
		dragFlag = false;
//		const QModelIndex idx ( indexAt(startDragPoint) );
//		if(idx.isValid())
//			emit pressed(idx);
	}
	QListView::mousePressEvent(event);
}

void FMPreviewView::mouseMoveEvent(QMouseEvent * event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if ((event->pos() - startDragPoint).manhattanLength() < QApplication::startDragDistance())
		return;

	// Create a window with the current preview
	if(currentIndex().isValid() && (!dragFlag))
	{
		dragFlag = true;
//		FontItem * sf(typotek::getInstance()->getSelectedFont());
		QModelIndex idx = indexAt(startDragPoint);
		QString fname = idx.data(FMPreviewModel::PathRole).toString();
		if(!fname.isEmpty())
		{
			FontItem * sf(FMFontDb::DB()->Font(fname));
			if(sf)
				FMFloatingPreview::create(sf, QRect(event->globalPos(), QSize(width() ,1) ));
		}
	}

}


void FMPreviewView::updateLayout()
{
	// Nothing wants to work! ###
}

void FMPreviewView::setCurrentFont(const QString & name)
{
	QList<FontItem*> fl(reinterpret_cast<FMPreviewModel*>(model())->getBase());

	const int fl_count(fl.count());
	int rFont(fl_count);
	for(int i(0); i < fl_count ; ++i)
	{
		if(fl[i]->path() == name)
		{
			rFont = i;
			break;
		}
	}
	
	if(rFont != fl_count)
	{
		QAbstractListModel *mod(reinterpret_cast<QAbstractListModel*>(model()));
		QModelIndex mi(mod->index(rFont));
		if(mi.isValid())
		{
			selectionModel()->setCurrentIndex(mi, QItemSelectionModel::ClearAndSelect);
			// scrollTo ( mi );
		}
	}
}

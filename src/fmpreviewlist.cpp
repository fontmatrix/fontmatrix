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
#include <QPainter>
#include <QBrush>

#define FM_MINIMUM_PREVIEW_WIDTH 280

bool FMPreviewIconEngine::initState = false;
QPen FMPreviewIconEngine::pen = QPen();
QVector<QRgb> FMPreviewIconEngine::m_selPalette;
QRgb FMPreviewIconEngine::activatedColor =  qRgb (9,223,11);
QRgb FMPreviewIconEngine::deactivatedColor =  qRgb (190,190,190);
QRgb FMPreviewIconEngine::partlyActivatedColor =  qRgb (166,220,220);


FMPreviewIconEngine::FMPreviewIconEngine()
	:QIconEngine(),
	activatedFont(NotActivated)
{
	if(!initState)
	{
		// setup palette
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

		// setup "writing" pen
//		pen.setColor(QColor(m_selPalette.at(128)));
		pen.setColor(QColor(220,220,220));
		pen.setWidth(1);

		initState = true;
	}

}

QIconEngine *FMPreviewIconEngine::clone() const
{
	// TODO Implement this function
	return 0;
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
		QPainterPath pp;
		double rr(double(r.height()) / 5.0);
		pp.addRoundedRect(r,rr,rr);
		painter->setRenderHint(QPainter::Antialiasing, true);
		// draw background
		painter->save();
		painter->setPen(Qt::NoPen);
		if(mode == QIcon::Selected)
			painter->setBrush(QApplication::palette().color(QPalette::Highlight));
		else
			painter->setBrush(QApplication::palette().color(QPalette::Base));
		painter->drawPath(pp);
		painter->restore();
		// end of bg
		painter->setPen(pen);
		QRect tr(r);
		tr.translate(0, (r.height() - m_p.height()) / 2);
		if(mode == QIcon::Selected)
		{
			QImage hm(m_p.toImage().convertToFormat(QImage::Format_Indexed8));
			hm.setColorTable(actualSelPalette(hm.colorTable()));
			painter->setClipPath(pp);
			painter->drawPixmap(tr, QPixmap::fromImage(hm) , r);
			painter->drawPath(pp);
		}
		else
		{
			painter->drawPixmap(tr, m_p , r);
			painter->drawPath(pp);

		}
		if(activatedFont != NotActivated)
		{
			painter->setPen(Qt::NoPen);
			QPainterPath activationPath;
			double rr2(rr/2.0);
			activationPath.moveTo(rr, 0);
			activationPath.cubicTo(rr2,0,
					       0,rr2,
					       0,rr);
			activationPath.lineTo(0,rect.height() - rr);
			activationPath.cubicTo(0,rect.height() -rr2,
					       rr2,rect.height(),
					       rr,rect.height());
			activationPath.closeSubpath();
			if(activatedFont == Activated)
				painter->setBrush(QBrush(activatedColor));
			else if(activatedFont == PartlyActivated)
				painter->setBrush(QBrush(partlyActivatedColor));
			painter->drawPath(activationPath);
		}
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
	familyMode = false;
	QSettings settings;
	styleTooltipName = settings.value("Preview/StyleTooltipName","font-weight:bold;").toString();
	styleTooltipPath = settings.value("Preview/StyleTooltipPath","font-weight:normal;font-size:small;").toString();
	styleTooltipTags = settings.value("Preview/StyleTooltipTags","text-align:right;font-weight:normal;font-size:small;font-style:italic;").toString();
	
	settings.setValue("Preview/StyleTooltipName", styleTooltipName);
	settings.setValue("Preview/StyleTooltipPath", styleTooltipPath);
	settings.setValue("Preview/StyleTooltipTags", styleTooltipTags);
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
	
	QColor bgColor(QApplication::palette().color(QPalette::Base));
	QColor fgColor(QApplication::palette().color(QPalette::Text));

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
		FMPreviewIconEngine * pie(new FMPreviewIconEngine);
		if(!familyMode)
			pie->setActivation(fit->isActivated() ? FMPreviewIconEngine::Activated : FMPreviewIconEngine::NotActivated);
		else
		{
			bool hasActive(false);
			bool hasNotActive(false);
			foreach(FontItem * f, FMFontDb::DB()->FamilySet(fit->family()))
			{
				if(f->isActivated())
					hasActive = true;
				else
					hasNotActive = true;
				if(hasActive && hasNotActive)
					break;
			}
			if(hasNotActive && hasActive)
				pie->setActivation(FMPreviewIconEngine::PartlyActivated);
			else
			{
				pie->setActivation(hasActive ? FMPreviewIconEngine::Activated : FMPreviewIconEngine::NotActivated);
			}
		}
		QIcon ic( pie );
		ic.addPixmap(im);

		return ic;
	}
	else if(role == Qt::ToolTipRole)
	{
		if(familyMode)
		{
			QList<FontItem*> fam(FMFontDb::DB()->FamilySet(fit->family()));
			QString sRet;
			sRet+= "<div style=\"" + styleTooltipName + "\">" + fit->family() + " ("+QString::number(fam.count())+")</div>";
			sRet+= "<div style=\"" + styleTooltipTags + "\">" + fit->tags().join(QString(", ")) + "</div>";

			foreach(FontItem* ffi, fam)
			{
				sRet += "<div style=\"" + styleTooltipPath + "\">" + ffi->variant() + "</div>";
			}
			return sRet;
		}
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


FMPreviewView::FMPreviewView(QWidget * parent):
		QListView(parent),
		columns(1)
{
	dragFlag = false;
	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::DragDrop);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setSelectionRectVisible(false);

}

bool FMPreviewView::moveTo(const QString &fname)
{
	QList<FontItem*> fl(reinterpret_cast<FMPreviewModel*>(model())->getBase());

	QString uname(fname.toUpper());
	const int fl_count(fl.count());
	int rFont(fl_count);
	for(int i(0); i < fl_count ; ++i)
	{
		QString pname(fl[i]->fancyName().toUpper());
		pname.truncate(uname.count());
		if(uname == pname)
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
			return true;
		}
	}

	return false;
}

void FMPreviewView::resizeEvent(QResizeEvent * event)
{
        int actualWidth(width() - 20); // if we use the viewport size, it becomes funny when a resize shows/hides the scrollbar
        setSpacing(0);
        int gHeight(2.0 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0);
        qDebug()<< "VW" << actualWidth<<verticalScrollBar()->width()<< "S" <<spacing();
        double cNr(1);

	if(columns == 1)
                usedWidth = qRound((double(actualWidth)  / columns));
	else
        {
            int minCellWidth(FM_MINIMUM_PREVIEW_WIDTH + 6);
            cNr = qRound(double(actualWidth) / minCellWidth);
            minCellWidth =  qRound((double(actualWidth)  / cNr) - 6);
            qDebug()<< "C" << cNr << "U" << minCellWidth ;
            setGridSize(QSize(minCellWidth, gHeight + 12));
            usedWidth = minCellWidth - 6;
        }
        setIconSize(QSize(usedWidth, gHeight + 6));
	QListView::resizeEvent(event);
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
	if(!(event->modifiers().testFlag( Qt::ControlModifier )))
		return;
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if ((event->pos() - startDragPoint).manhattanLength() < QApplication::startDragDistance())
		return;

	FMPreviewModel * m(reinterpret_cast<FMPreviewModel*>(model()));
	if(m && m->getFamilyMode())
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

void FMPreviewView::keyPressEvent(QKeyEvent *event)
{
	qDebug()<<"FMPreviewView::keyPressEvent"<<event;
	if((!event->text().isEmpty()) && (event->text().at(0).isLetterOrNumber()))
		emit keyPressed(event->text());
	else
		QListView::keyPressEvent(event);
}

void FMPreviewView::updateLayout()
{
	update();
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

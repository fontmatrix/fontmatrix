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
#include <QDesktopWidget>
#include <QSettings>
#include <QScrollBar>




FMPreviewModel::FMPreviewModel( QObject * pa , QListView * wPa )
	: QAbstractListModel(pa) , m_view(wPa)
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
	FontItem *fit(typotek::getInstance()->getCurrentFonts().at(row));
	if(!fit)
		return QVariant();
	
	QColor bgColor(255,255,255);
// 	int borders( 2*(m_view->frameWidth() + m_view->lineWidth() + m_view->midLineWidth()) ); 
// 	int scrollbar(m_view->verticalScrollBar()->width());
// 	int width( m_view->width() - (borders + scrollbar) );
// 	qDebug()<<"W"<<width<<borders<<scrollbar;
	// quite strange but I cannot determine accuratly the size of the visible part of the inner frame :/
	int width( qRound(m_view->width() * 0.92) );
	
	if(role == Qt::DisplayRole)
	{
		if( typotek::getInstance()->getPreviewSubtitled() )
			return fit->fancyName() ;
		else
			return QVariant();
	}
	else if(role == Qt::DecorationRole)
	{
		QString word(typotek::getInstance()->word());
		word.replace("<name>", fit->fancyName());
		word.replace("<family>", fit->family());
		word.replace("<variant>", fit->variant());
		return QIcon( fit->oneLinePreviewPixmap(word, bgColor, width ) );
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
	QList< FontItem * > cl(typotek::getInstance()->getCurrentFonts());
	return cl.count();
}

void FMPreviewModel::dataChanged()
{
	emit layoutChanged();
}

FMPreviewView::FMPreviewView(QWidget * parent)
	:QListView(parent)
{
}

void FMPreviewView::resizeEvent(QResizeEvent * event)
{
	emit widthChanged(this->width());
}

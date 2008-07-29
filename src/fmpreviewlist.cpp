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




FMPreviewModel::FMPreviewModel( QObject * pa , QListView * wPa )
	: QAbstractListModel(pa) , m_view(wPa)
{
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
	int width( m_view->width() );
	
	if(role == Qt::DisplayRole)
	{
		if( typotek::getInstance()->getPreviewSubtitled() )
			return fit->fancyName() ;
		else
			return QVariant();
	}
	else if(role == Qt::DecorationRole)
	{
			return QIcon( fit->oneLinePreviewPixmap(typotek::getInstance()->word(), bgColor, width ) );
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
	if(parent.isValid())
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

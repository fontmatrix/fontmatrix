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
#ifndef FMPREVIEWLIST_H
#define FMPREVIEWLIST_H


#include <QListView>
#include <QAbstractListModel>

class FontItem;
class MainViewWidget;
class QListView;

class FMPreviewModel : public QAbstractListModel
{
	public:
		FMPreviewModel ( QObject * pa , QListView * wPa);
		//returns a preview
		QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
		//returns flags for items
		Qt::ItemFlags flags ( const QModelIndex &index ) const;
		//returns the number of items
		int rowCount ( const QModelIndex &parent ) const;
		
		void dataChanged();
		
	private:
		QListView *m_view;
		
		QString styleTooltipName;
		QString styleTooltipPath;
};

class FMPreviewView : public QListView
{
	Q_OBJECT
	public:
		FMPreviewView(QWidget * parent = 0);
		~FMPreviewView(){};
		
	protected:
		void resizeEvent ( QResizeEvent * event );
	signals:
		void widthChanged(int);
};

#endif

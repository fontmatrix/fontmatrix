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
#ifndef LISTDOCKWIDGET_H
#define LISTDOCKWIDGET_H

#include <QWidget>
#include <ui_listsdock.h>

class QListWidgetItem;
class QFontItem;
class QDirModel;

/**
	@author Pierre Marchand <pierre@oep-h.com>
*/
class ListDockWidget : public QWidget, public Ui::ListDock
{
		Q_OBJECT
		ListDockWidget();
	public:
		~ListDockWidget();
		static ListDockWidget* getInstance();
		static ListDockWidget* instance;
		
		void savePosition();
		void restorePosition();
		
		bool nameItemIsVisible(QTreeWidgetItem *item);
		
		void forcePreviewRefill();
	private:
		int m_position;
		QDirModel *theDirModel;
		
	public slots:
		void unlockFilter();
		
	private slots:
		void slotFolderItemclicked(QModelIndex mIdx);
		
	signals:
		void folderSelectFont(const QString&);

};

#endif

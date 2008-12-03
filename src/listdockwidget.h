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
#include <QMap>
#include <QMenu>
#include <QFileInfo>
#include <ui_listsdock.h>

class QListWidgetItem;
class FMPreviewModel;
class QFontItem;
class QDirModel;
class QMenu;
class QAction;
class QActionGroup;
class QCompleter;
class QPoint;
class FolderViewMenu;

/**
	@author Pierre Marchand <pierre@oep-h.com>
*/
class ListDockWidget : public QWidget, public Ui::ListDock
{
		Q_OBJECT
		ListDockWidget();
		~ListDockWidget();
		static ListDockWidget* instance;
	public:
		static ListDockWidget* getInstance();

		void refreshTree();
		void savePosition();
		void restorePosition();

		bool nameItemIsVisible(QTreeWidgetItem *item);

		QModelIndex getFolderCurrentIndex(){return currentFIndex;}

		QString getCurrentField() const
		{
			return currentField;
		}

		QAction* getCurrentFieldAction() const
		{
			return currentFieldAction;
		}
	

	private:
		int m_position;
		QDirModel *theDirModel;
		QStringList ffilter;
		QModelIndex currentFIndex;
		QMenu *theFilterMenu;
		QActionGroup *filterActGroup;
		QString currentField;
		QAction *currentFieldAction;
		QString allFieldName;
		
		QMap<QString, QCompleter*> completers;

		FolderViewMenu *folderViewContextMenu;

		QIcon tagsetIcon;

		void initTagCombo();
		void settingsDir(const QString &path);
		
		QString fieldString(const QString& f);
		int maxFieldStringWidth;
		
		FMPreviewModel *previewModel;
				
	public slots:
		void unlockFilter();
		void reloadTagsCombo();
		void slotPreviewUpdate();

	private slots:
		void slotFolderItemclicked(QModelIndex mIdx);
		void slotFolderPressed(QModelIndex mIdx);
		void slotFieldChanged(QAction * action);
		void slotFeedTheCompleter();
		void slotFolderViewContextMenu(const QPoint&);
		void slotTabChanged(int i);
// 		void slotPanoseChecked(bool checked);
		
		// Concerns the width of the preview widget
		void slotPreviewUpdateSize(int);
		void slotPreviewSelected(const QModelIndex & index);
		
		// text & size of the preview
		void slotPreviewText(const QString& p);
		void slotPreviewSize(double d);

	signals:
		void folderSelectFont(const QString&);

};

class FolderViewMenu : public QMenu
{
	Q_OBJECT
public:
	FolderViewMenu();
	~FolderViewMenu();

	void exec(const QFileInfo &fi, const QPoint &p);

private:
	QAction *dirReload;
	QAction *dirAction;
	QAction *dirRecursiveAction;
	QAction *fileAction;

	QFileInfo selectedFileOrDir;

private slots:
	void slotReloadTree();
	void slotImportDir();
	void slotImportDirRecursively();
	void slotImportFile();
	
};

#endif

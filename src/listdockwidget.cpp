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
#include "listdockwidget.h"
#include "typotek.h"
#include "fontitem.h"

#include <QDebug>
#include <QScrollBar>
#include <QDirModel>
#include <QStringListModel>
#include <QCompleter>
#include <QStatusBar>

extern QStringList name_meaning;

ListDockWidget* ListDockWidget::instance = 0;

ListDockWidget * ListDockWidget::getInstance()
{
	if(!instance)
	{
		instance = new ListDockWidget;
	}
	return instance;
}

ListDockWidget::ListDockWidget()
 : QWidget()
{
	setupUi(this);
	fontTree->setIconSize(QSize(32,32));
// 	tagsetCombo->addItems (typotek::getInstance()->tagsets() );

	QStringList tl_tmp = typotek::tagsList;
	qDebug() << "TAGLIST\n" << typotek::tagsList.join ( "\n" );
	tl_tmp.removeAll ( "Activated_On" );
	tl_tmp.removeAll ( "Activated_Off" );

	tagsCombo->addItems ( tl_tmp );
	tagsCombo->addItems ( typotek::getInstance()->tagsets() );

	// Folders tree
	ffilter << "*.otf" << "*.ttf" << "*.pfb";
	theDirModel = new QDirModel(ffilter, QDir::AllDirs | QDir::Files | QDir::Drives | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name);
	folderView->setModel(theDirModel);
	folderView->hideColumn(1);
	folderView->hideColumn(2);
	folderView->hideColumn(3);
	folderView->setContextMenuPolicy(Qt::CustomContextMenu);
	folderViewContextMenu = 0;

	theFilterMenu = new QMenu;
	filterActGroup = new QActionGroup(theFilterMenu);

	if(name_meaning.isEmpty())
		FontItem::fillNamesMeaning();

	currentField = tr("All fields");
	QAction *actn = new QAction(currentField, filterActGroup);
	actn->setCheckable(true);
	actn->setChecked(true);
	theFilterMenu->addAction(actn);

	QStringListModel *lModel = new QStringListModel;
	completers[currentField] = new QCompleter(this);
	completers[currentField]->setModel(lModel);

	for(int gIdx(0); gIdx < name_meaning.count() ; ++gIdx)
	{
		qDebug()<<"ADD Name action"<< name_meaning[gIdx];
		actn = new QAction(name_meaning[gIdx], filterActGroup);
		actn->setCheckable(true);
		theFilterMenu->addAction(actn);
		lModel = new QStringListModel;
		completers[name_meaning[gIdx]] = new QCompleter(this);
		completers[name_meaning[gIdx]]->setModel(lModel);
	}

	fieldButton->setMenu(theFilterMenu);
	fieldButton->setToolTip(currentField);

	searchString->setCompleter(completers.value(currentField));

	folderView->setDragDropMode(QAbstractItemView::DragDrop);

	connect( filterActGroup,SIGNAL(triggered( QAction* )),this,SLOT(slotFieldChanged(QAction*)));
	connect( searchString,SIGNAL(textEdited( const QString& )),this,SLOT(slotFeedTheCompleter(const QString&)));

	connect(folderView, SIGNAL(clicked( const QModelIndex& )), this, SLOT(slotFolderItemclicked(QModelIndex)));
	connect(folderView,SIGNAL(pressed( const QModelIndex& )),this,SLOT(slotFolderPressed(QModelIndex)));
	connect(folderView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotFolderViewContextMenu(const QPoint &)));
}


ListDockWidget::~ListDockWidget()
{
}

void ListDockWidget::savePosition()
{
	m_position = fontTree->verticalScrollBar()->value();
}

void ListDockWidget::restorePosition()
{
	fontTree->verticalScrollBar()->setValue(m_position);
}

bool ListDockWidget::nameItemIsVisible(QTreeWidgetItem * item)
{
	int center = fontTree->viewport()->size().width() / 2;
	int begin = fontTree->verticalScrollBar()->value();
	int end = begin + fontTree->viewport()->size().height();

	for(int i(begin); i < end; ++i)
	{
		if(fontTree->itemAt(center,i) == item)
			return true;
	}
	return false;
}

void ListDockWidget::forcePreviewRefill()
{
	previewList->slotRefill(QList<FontItem*>(),false);
}

void ListDockWidget::unlockFilter()
{
	searchString->setEnabled(true);
}

void ListDockWidget::slotFolderItemclicked(QModelIndex mIdx)
{
	QString path(theDirModel->data(mIdx,QDirModel::FilePathRole).toString());
	if(typotek::getInstance()->insertTemporaryFont(path))
	{
		QFileInfo pf(path);
		emit folderSelectFont(pf.absoluteFilePath());
	}
}
/*
void ListDockWidget::slotFolderItemDoubleclicked(QModelIndex mIdx)
{
	qDebug()<<"DOUBLE_ClIC"<<theDirModel->data(mIdx,QDirModel::FilePathRole).toString();
	// Want to import?
	if(theDirModel->isDir(mIdx) && theDirModel->hasChildren(mIdx))
	{
		QDir dir(theDirModel->data(mIdx,QDirModel::FilePathRole).toString());
		QStringList tmplist(dir.entryList(ffilter));
		QStringList flist;
		for(int i(0); i < tmplist.count();++i)
			flist << dir.absoluteFilePath( tmplist[i] );

		qDebug()<<"WANT"<<flist.join("::");
		if(!flist.isEmpty())
			typotek::getInstance()->open(flist);
	}
	else
	{
		QStringList flist(theDirModel->data(mIdx,QDirModel::FilePathRole).toString());
		qDebug()<<"WANT"<<flist.join("::");
		if(!flist.isEmpty())
			typotek::getInstance()->open(flist);
	}

}
*/

void ListDockWidget::slotFolderPressed(QModelIndex mIdx)
{
	currentFIndex = mIdx;
}

void ListDockWidget::slotFieldChanged(QAction * action)
{
	currentField = action->text();
	fieldButton->setToolTip(currentField);
	searchString->setCompleter(completers.value(currentField));
}


void ListDockWidget::slotFeedTheCompleter(const QString & w)
{
	QStringListModel *m = reinterpret_cast<QStringListModel*>( completers.value(currentField)->model() );
	QStringList l(m->stringList ());
	if(!l.contains(w))
	{
		l << w;
		m->setStringList(l);
	}
}

void ListDockWidget::slotFolderViewContextMenu(const QPoint& p)
{
	QDirModel *dm = static_cast<QDirModel*>(folderView->model());
	if (!dm)
		return;

	QModelIndex mi = folderView->currentIndex();
	if (!mi.isValid())
		return;

	slotFolderItemclicked(mi); // make sure the font in question is loaded
	                           // with a direct right click it would crash without this

	if (!folderViewContextMenu)
		folderViewContextMenu = new FolderViewMenu();

	folderViewContextMenu->exec(dm->fileInfo(mi), tabWidget->pos() + mapToGlobal(p));
}


FolderViewMenu::FolderViewMenu() : QMenu()
{
	dirAction = new QAction(tr("Import"), 0);
	dirRecursiveAction = new QAction(tr("Import recursively"), 0);
	fileAction = new QAction("Import", 0);

	addAction(dirAction);
	addAction(dirRecursiveAction);
	addAction(fileAction);

	connect(dirAction, SIGNAL(triggered()), this, SLOT(slotImportDir()));
	connect(dirRecursiveAction, SIGNAL(triggered()), this, SLOT(slotImportDirRecursively()));
	connect(fileAction, SIGNAL(triggered()), this, SLOT(slotImportFile()));
}

void FolderViewMenu::exec(const QFileInfo &fi, const QPoint &p)
{
	if (fi.isDir()) {
		dirAction->setVisible(true);
		dirRecursiveAction->setVisible(true);
		fileAction->setVisible(false);
	} else if (fi.isFile()) {
		dirAction->setVisible(false);
		dirRecursiveAction->setVisible(false);
		fileAction->setVisible(true);
	} else
		return; // not a file or a directory

	selectedFileOrDir = fi;

	QMenu::exec(p);
}

void FolderViewMenu::slotImportDir()
{
	QDir dir(selectedFileOrDir.absoluteFilePath());
	QStringList ffilter;
	ffilter << "*.otf" << "*.ttf" << "*.pfb";
	QStringList fontList = dir.entryList(ffilter);
	QString tmpFontPath;
	foreach(tmpFontPath, fontList) {
		typotek::getInstance()->open(tmpFontPath);
	}
}

void FolderViewMenu::slotImportDirRecursively()
{
	typotek::getInstance()->open(selectedFileOrDir.absoluteFilePath());
}

void FolderViewMenu::slotImportFile()
{
	typotek::getInstance()->open(selectedFileOrDir.absoluteFilePath());
}

FolderViewMenu::~FolderViewMenu()
{

}


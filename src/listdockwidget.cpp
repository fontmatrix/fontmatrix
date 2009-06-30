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
#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "fmpreviewlist.h"
#include "mainviewwidget.h"
#include "shortcuts.h"

#include <QDebug>
#include <QScrollBar>
#include <QDirModel>
#include <QStringListModel>
#include <QCompleter>
#include <QStatusBar>
#include <QSettings>
#include <QFileSystemWatcher>

extern QStringList name_meaning;
extern QMap< QString, QMap<int, QString> > panoseMap;

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
	
	// Hide save filter button until it’s implemented
	saveFilterButton->setVisible(false);

	
	listPreview->setModelColumn(1);
	listPreview->setViewMode(QListView::IconMode);
	listPreview->setIconSize(QSize(listPreview->width(), 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
	listPreview->setUniformItemSizes(true);
	listPreview->setMovement(QListView::Static);
	
	previewModel = new FMPreviewModel( this, listPreview );
	listPreview->setModel(previewModel);
	previewText->setText(typotek::getInstance()->word());
	previewText->setToolTip(tr("You can use the following keywords to be replaced by data from fonts: <strong>&#60;name&#62;</strong> ; <strong>&#60;family&#62;</strong> ; <strong>&#60;variant&#62;</strong>"));
	previewSize->setValue(typotek::getInstance()->getPreviewSize());

	// Folders tree
	ffilter << "*.otf" << "*.ttf" << "*.pfb";
	theDirModel = new QDirModel(ffilter, QDir::AllDirs | QDir::Files | QDir::Drives | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name);
	theDirModel->setLazyChildCount(true);
	folderView->setModel(theDirModel);
	folderView->hideColumn(1);
	folderView->hideColumn(2);
	folderView->hideColumn(3);
	folderView->setContextMenuPolicy(Qt::CustomContextMenu);
	folderViewContextMenu = 0;

	QSettings settings;
	QString lastUsedDir = settings.value("Places/LastUsedFolder", QDir::homePath()).toString();
	QDir d(lastUsedDir);
	if (!d.exists())
		lastUsedDir = QDir::homePath();
	QModelIndex luIdx(theDirModel->index(lastUsedDir, 0));
	folderView->setCurrentIndex(luIdx);
	QModelIndexList hierarchy;
	while(luIdx.isValid())
	{
		hierarchy.prepend(luIdx);
		luIdx = luIdx.parent();
	}
	foreach(QModelIndex idx, hierarchy)
		folderView->expand(idx);

	dirWatcher = new QFileSystemWatcher(this);
	initWatcher(theDirModel->index(0,0));


	theFilterMenu = new QMenu;
	filterActGroup = new QActionGroup(theFilterMenu);
	
	QString tagName(tr("Tags"));
	QAction *actn = new QAction(tagName, filterActGroup);
	actn->setData(FILTER_FIELD_SPECIAL_TAG);
	actn->setCheckable(true);
	actn->setChecked(true);
	theFilterMenu->addAction(actn);
	
	// set defaults to Tag
	currentField = tagName;
	currentFieldAction = actn;
	
	// Filter All
	allFieldName = FontStrings::Names()[FMFontDb::AllInfo];
	maxFieldStringWidth = allFieldName.count();
	actn = new QAction(allFieldName, filterActGroup);
	actn->setData(FMFontDb::AllInfo);
	actn->setCheckable(true);
	theFilterMenu->addAction(actn);
	
	QStringListModel *lModel = new QStringListModel;
	completers[allFieldName] = new QCompleter(this);
	completers[allFieldName]->setModel(lModel);
	
	// Filter Unicode
	QString uniFName(tr("Unicode character"));
	actn = new QAction(uniFName, filterActGroup);
	actn->setData(FILTER_FIELD_SPECIAL_UNICODE);
	actn->setCheckable(true);
	theFilterMenu->addAction(actn);
	
	QStringListModel *uModel = new QStringListModel;
	completers[uniFName] = new QCompleter(this);
	completers[uniFName]->setModel(uModel);


	// Filters meta-data
	for(int gIdx(0); gIdx < FontStrings::Names().keys().count() ; ++gIdx)
	{
		FMFontDb::InfoItem k(FontStrings::Names().keys()[gIdx]);
		if((k !=  FMFontDb::AllInfo))
		{
			actn = new QAction(FontStrings::Names()[k], filterActGroup);
			actn->setData( k );
			actn->setCheckable(true);
			
			theFilterMenu->addAction(actn);
			lModel = new QStringListModel;
			completers[FontStrings::Names()[k]] = new QCompleter(this);
			completers[FontStrings::Names()[k]]->setModel(lModel);
		}
	}
	

	fieldButton->setMenu(theFilterMenu);
	fieldButton->setToolTip(currentField);
	fieldButton->setText( currentField );
	
	collapseAlphaAction = new QAction(tr("Collapse Top Level"), this);
	Shortcuts::getInstance()->add(collapseAlphaAction);
	collapseFamiliesAction = new QAction(tr("Collapse Families"), this);
	Shortcuts::getInstance()->add(collapseFamiliesAction);
	
	expandAlphaAction = new QAction(tr("Expand Top Level"), this);
	Shortcuts::getInstance()->add(expandAlphaAction);
	expandFamiliesAction = new QAction(tr("Expand Families"), this);
	Shortcuts::getInstance()->add(expandFamiliesAction);
	
	QMenu * collapseMenu = new QMenu(this);
	collapseMenu->addAction(collapseFamiliesAction);
	collapseMenu->addAction(collapseAlphaAction);
	collapseMenu->addAction(expandFamiliesAction);
	collapseMenu->addAction(expandAlphaAction);
	collapseButton->setMenu(collapseMenu);

	searchString->setCompleter(completers.value(currentField));

	folderView->setDragDropMode(QAbstractItemView::DragDrop);

	initTagCombo();
	
// 	connect( panoseButton, SIGNAL(clicked( bool )), this, SLOT(slotPanoseChecked(bool)));
	
	connect(collapseAlphaAction, SIGNAL(triggered()),this,SLOT(slotCollapseAlpha()));
	connect(collapseFamiliesAction, SIGNAL(triggered()),this,SLOT(slotCollapseFamilies()));
	connect(expandAlphaAction,SIGNAL(triggered()), this, SLOT(slotExpandAlpha()));
	connect(expandFamiliesAction,SIGNAL(triggered()), this, SLOT(slotExpandFamilies()));
	
	connect( filterActGroup,SIGNAL(triggered( QAction* )),this,SLOT(slotFieldChanged(QAction*)));
	connect( searchString,SIGNAL(returnPressed()),this,SLOT(slotFeedTheCompleter()));

	connect(folderView, SIGNAL(activated( const QModelIndex& )), this, SLOT(slotFolderItemclicked(QModelIndex)));
	connect(folderView, SIGNAL(clicked( const QModelIndex& )), this, SLOT(slotFolderItemclicked(QModelIndex)));
	connect(folderView,SIGNAL(pressed( const QModelIndex& )),this,SLOT(slotFolderPressed(QModelIndex)));
	connect(folderView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotFolderViewContextMenu(const QPoint &)));
	connect(folderView, SIGNAL(expanded(QModelIndex)),this, SLOT(slotFolderAddToWatcher(QModelIndex)));
	connect(folderView, SIGNAL(collapsed(QModelIndex)), this, SLOT(slotFolderRemoveFromWatcher(QModelIndex)));
	connect(dirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(slotFolderRefresh(QString)));

	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));
	
	connect(typotek::getInstance(),SIGNAL(previewHasChanged()),this,SLOT(slotPreviewUpdate()));
	connect(listPreview, SIGNAL(widthChanged(int)),this,SLOT(slotPreviewUpdateSize(int)));
	connect(listPreview,SIGNAL(activated ( const QModelIndex&)),this,SLOT( slotPreviewSelected(const QModelIndex& )));
	connect(listPreview,SIGNAL(clicked ( const QModelIndex&)),this,SLOT( slotPreviewSelected(const QModelIndex& )));
	connect(previewText, SIGNAL(textChanged( const QString& )), this, SLOT(slotPreviewText( const QString&)));
	connect(previewSize, SIGNAL(valueChanged( double )), this, SLOT(slotPreviewSize(double)));
}


ListDockWidget::~ListDockWidget()
{
	delete dirWatcher;
}

void ListDockWidget::initWatcher(QModelIndex parent)
{
//	qDebug()<<"initWatcher"<<theDirModel->filePath(parent);
	for(int fIdx(0); fIdx < theDirModel->rowCount(parent); ++fIdx)
	{
		QModelIndex mIdx(theDirModel->index(fIdx,0, parent));
//		qDebug()<<"\t"<<theDirModel->filePath(mIdx)<<folderView->isExpanded(mIdx);
		if(folderView->isExpanded(mIdx))
		{
			QString fp(theDirModel->filePath(mIdx));
			dirWatcher->addPath(fp);
			qDebug()<<"***Watch"<<fp;
			initWatcher(mIdx);
		}
	}
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


void ListDockWidget::unlockFilter()
{
	searchString->setEnabled(true);
}

void ListDockWidget::slotFolderItemclicked(QModelIndex mIdx)
{
	QString path(theDirModel->data(mIdx,QDirModel::FilePathRole).toString());
	QFileInfo pf(path);
	if(!pf.isDir())
	{
		if(FMFontDb::DB()->insertTemporaryFont(path))
		{
			emit folderSelectFont(pf.absoluteFilePath());
		}
	}
	settingsDir(path);
}

void ListDockWidget::slotFolderPressed(QModelIndex mIdx)
{
	currentFIndex = mIdx;
}

void ListDockWidget::slotFolderAddToWatcher(QModelIndex mIdx)
{
	qDebug()<<"Add to watcher"<<theDirModel->filePath(mIdx);
	dirWatcher->addPath(theDirModel->filePath(mIdx));
}

void ListDockWidget::slotFolderRemoveFromWatcher(QModelIndex mIdx)
{
	qDebug()<<"Remove from watcher"<<theDirModel->filePath(mIdx);
	dirWatcher->removePath(theDirModel->filePath(mIdx));
}

/**
 * Reload the file and folder list in the folder tab
 */
void ListDockWidget::slotFolderRefresh(const QString& dirPath)
{
	qDebug()<<"Refresh"<<dirPath;
	theDirModel->refresh(theDirModel->index(dirPath, 0 ));
}

void ListDockWidget::slotFieldChanged(QAction * action)
{
	currentField = action->text();
	currentFieldAction = action;
	fieldButton->setToolTip(currentField);
	fieldButton->setText( fieldString( currentField ) );
	searchString->setCompleter(completers.value(currentField));
	
	if(currentFieldAction->data().toInt() == FILTER_FIELD_SPECIAL_TAG)
	{
		filterValueStack->setCurrentIndex(0);
	}
	else
	{
		filterValueStack->setCurrentIndex(1);
	}
}


void ListDockWidget::slotFeedTheCompleter()
{
	QString w(searchString->text());
	if(w.isEmpty())
		return;

	QStringListModel *m = reinterpret_cast<QStringListModel*>( completers.value(currentField)->model() );
	QStringList l(m->stringList ());
	if(!l.contains(w))
	{
		l << w;
		m->setStringList(l);
	}
	// we want "all fields" completer to have all strings completed
	if(currentField != allFieldName )
	{
		m = reinterpret_cast<QStringListModel*>( completers.value(allFieldName)->model() );
		l = m->stringList ();
		if(!l.contains(w))
		{
			l << w;
			m->setStringList(l);
		}
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
	dirAction = new QAction(tr("Import Directory"), 0);
	dirRecursiveAction = new QAction(tr("Import recursively"), 0);
	fileAction = new QAction(tr("Import File"), 0);

	addAction(dirAction);
	addAction(dirRecursiveAction);
	addAction(fileAction);

	connect(dirAction, SIGNAL(triggered()), this, SLOT(slotImportDir()));
	connect(dirRecursiveAction, SIGNAL(triggered()), this, SLOT(slotImportDirRecursively()));
	connect(fileAction, SIGNAL(triggered()), this, SLOT(slotImportFile()));
}// we want "all fields" completer to have all strings completed

void FolderViewMenu::exec(const QFileInfo &fi, const QPoint &p)
{
	if (fi.isDir()) {
		dirAction->setEnabled(true);
		dirRecursiveAction->setEnabled(true);
		fileAction->setEnabled(false);
	} else if (fi.isFile()) {
		dirAction->setEnabled(false);
		dirRecursiveAction->setEnabled(false);
		fileAction->setEnabled(true);
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
	if (fontList.count() < 1)
		return;
	QString lastItem = fontList.at(fontList.count() - 1);
	fontList.removeAt(fontList.count() - 1);
	foreach(QString tmpFontPath, fontList) {
		QString absPath = dir.absolutePath() + "/" + tmpFontPath;
		typotek::getInstance()->open(absPath, false, true);
	}
	typotek::getInstance()->open(dir.absolutePath() + "/" + lastItem, true); // import the last font with the announce flag set to true
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

void ListDockWidget::initTagCombo()
{
	tagsCombo->clear();
	tagsetIcon = QIcon(":/fontmatrix_tagseteditor.png");

	tagsCombo->addItem(tr("All activated"),"ALL_ACTIVATED");
	tagsCombo->addItem(tr("Similar to current"),"SIMILAR");

// 	QStringList ts_tmp = typotek::getInstance()->tagsets();
// 	foreach(QString tagset, ts_tmp)
// 	{
// 		tagsCombo->addItem(tagsetIcon,tagset,"TAGSET");
// 	}
	
	QStringList tl_tmp = FMFontDb::DB()->getTags();
	qDebug()<<"RELOAD"<<tl_tmp.join("|");
	tl_tmp.sort();
	foreach(QString tag, tl_tmp )
	{
		tagsCombo->addItem(tag, "TAG");
	}

}

void ListDockWidget::reloadTagsCombo()
{
	initTagCombo();
}

void ListDockWidget::settingsDir(const QString &path)
{
	static QString s;
	if (s == path)
		return;

	QFileInfo fi(path);
	QString dirPath = fi.absoluteFilePath();
	if (fi.isFile())
		dirPath = fi.absoluteDir().absolutePath();

	QSettings settings;
	settings.setValue("Places/LastUsedFolder", dirPath);

	s = path;
}

void ListDockWidget::slotTabChanged(int i)
{
	if (i == 2) {  // if the new tab is the folder view
		QSettings settings;
		QString lastUsedDir = settings.value("Places/LastUsedFolder", QDir::homePath()).toString();
		QDir d(lastUsedDir);
		if (!d.exists())
			lastUsedDir = QDir::homePath();

		folderView->setCurrentIndex(theDirModel->index(lastUsedDir));
	}
}

// void ListDockWidget::slotPanoseChecked(bool checked)
// {
// 	if(checked)
// 	{
// 		tagsCombo->clear();
// 		QMap< FontStrings::PanoseKey , QMap<int, QString> >::const_iterator cip;
// 		QMap<int, QString>::const_iterator cip2;
// 		for(cip = FontStrings::Panose().constBegin(); cip != FontStrings::Panose().constEnd(); ++cip)
// 		{
// 			for(cip2 = cip->constBegin() + 2 ;cip2 != cip->constEnd(); ++cip2)
// 			{
// 				tagsCombo->addItem(FontStrings::PanoseKeyName( cip.key() ) + "-" + cip2.value(), "TAG_IS_PANOSE");
// 			}
// 		}
// 		
// 	}
// 	else
// 	{
// 		initTagCombo();
// 	}
// }

QString ListDockWidget::fieldString(const QString & f)
{
	if(f.count() <= allFieldName.count())
		return f;
	if(f.mid(0,maxFieldStringWidth - 1).at(maxFieldStringWidth -2) == QChar(0x20) )
	{
		return f.mid(0,maxFieldStringWidth - 1);
	}
	return f.mid(0,maxFieldStringWidth - 1) + ".";
}

void ListDockWidget::slotPreviewUpdate()
{
	previewModel->dataChanged();
}

void ListDockWidget::slotPreviewUpdateSize(int w)
{
	listPreview->setIconSize(QSize(w, 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
// 	previewModel->dataChanged();
}

void ListDockWidget::slotPreviewSelected(const QModelIndex & index)
{
// 	qDebug()<<"slotPreviewSelected("<<index<<")";
	FontItem * fItem(typotek::getInstance()->getCurrentFonts().at(index.row()));
	if(!fItem)
	{
		qDebug()<<"\t-FontItme invalid";
		return;
	}
	QString fName(fItem->path());
	qDebug()<<"\t+"<< fName;
	if(!fName.isEmpty())
		typotek::getInstance()->getTheMainView()->slotFontSelectedByName(fName);
}


void ListDockWidget::slotPreviewText(const QString & p)
{
	typotek::getInstance()->setWord( p , true  );
}

void ListDockWidget::slotPreviewSize(double d)
{
	typotek::getInstance()->setPreviewSize( d );
}

QStringList ListDockWidget::getOperation() const
{
	QStringList ret;
// 	foreach(QAction* action, theOperationMenu->actions())
// 	{
// 		if(action->isChecked())
// 			ret << action->data().toString();
// 	}
	if(operationAND->isChecked())
		ret << "AND";
	if(operationNOT->isChecked())
		ret << "NOT";
	return ret;
}

void ListDockWidget::clearOperation()
{
// 	foreach(QAction* action, theOperationMenu->actions())
// 	{
// 		action->setChecked(false);
// 	}
	operationAND->setCheckState(Qt::Unchecked);
	operationNOT->setCheckState(Qt::Unchecked);
}




void ListDockWidget::slotCollapseFamilies()
{
	const int tliCount(fontTree->topLevelItemCount());
	for(int i(0); i < tliCount; ++i)
	{
		int fCount(fontTree->topLevelItem(i)->childCount());
		for(int f(0); f < fCount; ++f)
		{
			fontTree->topLevelItem(i)->child(f)->setExpanded(false);
		}
	}
}

void ListDockWidget::slotCollapseAlpha()
{
	const int tliCount(fontTree->topLevelItemCount());
	for(int i(0); i < tliCount; ++i)
	{
		fontTree->topLevelItem(i)->setExpanded(false);
	}
}

void ListDockWidget::slotExpandFamilies()
{
	const int tliCount(fontTree->topLevelItemCount());
	for(int i(0); i < tliCount; ++i)
	{
		int fCount(fontTree->topLevelItem(i)->childCount());
		for(int f(0); f < fCount; ++f)
		{
			fontTree->topLevelItem(i)->child(f)->setExpanded(true);
		}
	}
}

void ListDockWidget::slotExpandAlpha()
{
	const int tliCount(fontTree->topLevelItemCount());
	for(int i(0); i < tliCount; ++i)
	{
		fontTree->topLevelItem(i)->setExpanded(true);
	}
}

/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
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

#include "browserwidget.h"
#include "ui_browserwidget.h"

#include "fmfontdb.h"

#include <QDirModel>
#include <QDir>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QDebug>

BrowserWidget::BrowserWidget(QWidget *parent) :
		QWidget(parent),
		ui(new Ui::BrowserWidget)
{
	ui->setupUi(this);

	ffilter << "*.otf" << "*.ttf" << "*.pfb";
	theDirModel = new QDirModel(ffilter, QDir::AllDirs | QDir::Files | QDir::Drives | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name);
	theDirModel->setLazyChildCount(true);
	ui->browserView->setModel(theDirModel);
	ui->browserView->hideColumn(1);
	ui->browserView->hideColumn(2);
	ui->browserView->hideColumn(3);

	QSettings settings;
	QString lastUsedDir = settings.value("Places/LastUsedFolder", QDir::homePath()).toString();
	QDir d(lastUsedDir);
	if (!d.exists())
		lastUsedDir = QDir::homePath();
	QModelIndex luIdx(theDirModel->index(lastUsedDir, 0));
	ui->browserView->setCurrentIndex(luIdx);
	QModelIndexList hierarchy;
	while(luIdx.isValid())
	{
		hierarchy.prepend(luIdx);
		luIdx = luIdx.parent();
	}
	foreach(QModelIndex idx, hierarchy)
		ui->browserView->expand(idx);

	dirWatcher = new QFileSystemWatcher(this);
	initWatcher(theDirModel->index(0,0));

}

BrowserWidget::~BrowserWidget()
{
	delete ui;
}

void BrowserWidget::initWatcher(QModelIndex parent)
{
	//	qDebug()<<"initWatcher"<<theDirModel->filePath(parent);
	for(int fIdx(0); fIdx < theDirModel->rowCount(parent); ++fIdx)
	{
		QModelIndex mIdx(theDirModel->index(fIdx,0, parent));
		//		qDebug()<<"\t"<<theDirModel->filePath(mIdx)<<folderView->isExpanded(mIdx);
		if(ui->browserView->isExpanded(mIdx))
		{
			QString fp(theDirModel->filePath(mIdx));
			dirWatcher->addPath(fp);
			qDebug()<<"***Watch"<<fp;
			initWatcher(mIdx);
		}
	}
}

void BrowserWidget::slotFolderAddToWatcher(QModelIndex mIdx)
{
	qDebug()<<"Add to watcher"<<theDirModel->filePath(mIdx);
	dirWatcher->addPath(theDirModel->filePath(mIdx));
}

void BrowserWidget::slotFolderItemclicked(QModelIndex mIdx)
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

void BrowserWidget::slotFolderPressed(QModelIndex mIdx)
{
	currentFIndex = mIdx;
}

void BrowserWidget::slotFolderRefresh(const QString &dirPath)
{
	if(ui->browserView->isVisible())
	{
		qDebug()<<"Refresh"<<dirPath;
		theDirModel->refresh(theDirModel->index(dirPath, 0 ));
	}
}

void BrowserWidget::slotFolderRemoveFromWatcher(QModelIndex mIdx)
{
	qDebug()<<"Remove from watcher"<<theDirModel->filePath(mIdx);
	dirWatcher->removePath(theDirModel->filePath(mIdx));
}

void BrowserWidget::settingsDir(const QString &path)
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

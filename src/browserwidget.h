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

#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QModelIndex>

#define BROWSER_VIEW_INFO	0
#define BROWSER_VIEW_SAMPLE	1
#define BROWSER_VIEW_CHART	2

class QDirModel;
class QFileSystemWatcher;
class FloatingWidget;

namespace Ui {
	class BrowserWidget;
}

class BrowserWidget : public QWidget
{
	Q_OBJECT

public:
	explicit BrowserWidget(QWidget *parent = 0);
	~BrowserWidget();

private:
	Ui::BrowserWidget *ui;

	QString curVariant;

	FloatingWidget *sample;
	FloatingWidget *chart;
	FloatingWidget *activation;

	unsigned int currentIndex;
	unsigned int currentPage;
	QString uniBlock;

	QDirModel *theDirModel;
	QStringList ffilter;
	QFileSystemWatcher *dirWatcher;
	QModelIndex currentFIndex;
	void initWatcher(QModelIndex parent);
	void settingsDir(const QString& path);

private slots:
	void slotFolderItemclicked(QModelIndex mIdx);
	void slotFolderPressed(QModelIndex mIdx);
	void slotFolderAddToWatcher(QModelIndex mIdx);
	void slotFolderRemoveFromWatcher(QModelIndex mIdx);
	void slotFolderRefresh(const QString& dirPath);

	void slotShowInfo();
	void slotShowSample();
	void slotShowChart();

signals:
	void folderSelectFont(QString);
};

#endif // BROWSERWIDGET_H

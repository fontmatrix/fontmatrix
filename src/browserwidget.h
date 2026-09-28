/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BROWSERWIDGET_H
#define BROWSERWIDGET_H

#include <QFileInfo>
#include <QMenu>
#include <QModelIndex>
#include <QPoint>
#include <QStringList>
#include <QWidget>

constexpr int BROWSER_VIEW_INFO = 0;
constexpr int BROWSER_VIEW_SAMPLE = 1;
constexpr int BROWSER_VIEW_CHART = 2;

class QFileSystemModel;
class QFileSystemWatcher;
class FloatingWidget;
class FolderViewMenu;

namespace Ui
{
class BrowserWidget;
}

class BrowserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BrowserWidget(QWidget *parent = nullptr);
    ~BrowserWidget() override;

private:
    Ui::BrowserWidget *const ui;

    QString curVariant;

    FloatingWidget *sample = nullptr;
    FloatingWidget *chart = nullptr;
    FloatingWidget *activation = nullptr;

    unsigned int currentIndex = 0U;
    unsigned int currentPage;
    QString uniBlock;

    QFileSystemModel *theDirModel = nullptr;
    QStringList ffilter;
    QFileSystemWatcher *dirWatcher = nullptr;
    QModelIndex currentFIndex;

    FolderViewMenu *folderViewContextMenu = nullptr;

    void initWatcher(QModelIndex parent);
    void settingsDir(const QString &path);

    void updateButtons();

private Q_SLOTS:
    void slotFolderItemclicked(QModelIndex mIdx);
    void slotFolderPressed(QModelIndex mIdx);
    void slotFolderRefresh(const QString &dirPath);

    void slotShowInfo();
    void slotShowSample();
    void slotShowChart();

    void slotImport();

    void slotDetachChart();
    void slotDetachSample();

    void slotFolderViewContextMenu(const QPoint &);

private:
    void slotFolderAddToWatcher(QModelIndex mIdx);
    void slotFolderRemoveFromWatcher(QModelIndex mIdx);

Q_SIGNALS:
    void folderSelectFont(QString);
};

class FolderViewMenu : public QMenu
{
    Q_OBJECT
public:
    FolderViewMenu();
    ~FolderViewMenu() override;

    void exec(const QFileInfo &fi, const QPoint &p);

private:
    QAction *dirAction = nullptr;
    QAction *dirRecursiveAction = nullptr;
    QAction *fileAction = nullptr;

    QFileInfo selectedFileOrDir;

private Q_SLOTS:
    void slotImportDir();
    void slotImportDirRecursively();
    void slotImportFile();
};

#endif // BROWSERWIDGET_H

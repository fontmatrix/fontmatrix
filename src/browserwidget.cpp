/*
    SPDX-FileCopyrightText: 2010 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "browserwidget.h"
#include "fontmatrix_debug.h"
#include "ui_browserwidget.h"

#include "chartwidget.h"
#include "floatingwidgetsregister.h"
#include "fmfontdb.h"
#include "fminfodisplay.h"
#include "fontitem.h"
#include "samplewidget.h"
#include "typotek.h"

#include "fmconfig.h"
#include <KLocalizedString>
#include <QDebug>
#include <QDir>
#include <QFileSystemModel>
#include <QFileSystemWatcher>

BrowserWidget::BrowserWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BrowserWidget)
{
    ui->setupUi(this);

    ui->infoButton->setEnabled(false);
    ui->sampleButton->setEnabled(false);
    ui->chartButton->setEnabled(false);

    folderViewContextMenu = nullptr;
    currentPage = BROWSER_VIEW_SAMPLE;
    sample = chart = nullptr;
    ffilter << "*.otf" << "*.ttf" << "*.ttc" << "*.pfb";
    theDirModel = new QFileSystemModel(this);
    theDirModel->setNameFilters(ffilter);
    theDirModel->setNameFilterDisables(false);
    theDirModel->setFilter(QDir::AllDirs | QDir::Files | QDir::Drives | QDir::NoDotAndDotDot);
    theDirModel->setRootPath(QString());
    ui->browserView->setModel(theDirModel);
    ui->browserView->hideColumn(1);
    ui->browserView->hideColumn(2);
    ui->browserView->hideColumn(3);
    ui->browserView->setContextMenuPolicy(Qt::CustomContextMenu);

    QString lastUsedDir = FMConfig::value(QStringLiteral("Places/LastUsedFolder"), QDir::homePath()).toString();
    QDir d(lastUsedDir);
    if (!d.exists())
        lastUsedDir = QDir::homePath();
    QModelIndex luIdx(theDirModel->index(lastUsedDir));
    ui->browserView->setCurrentIndex(luIdx);
    QModelIndexList hierarchy;
    while (luIdx.isValid()) {
        hierarchy.prepend(luIdx);
        luIdx = luIdx.parent();
    }
    for (const auto &idx : hierarchy)
        ui->browserView->expand(idx);

    dirWatcher = new QFileSystemWatcher(this);
    initWatcher(theDirModel->index(0, 0));

    connect(ui->infoButton, &QToolButton::clicked, this, &BrowserWidget::slotShowInfo);
    connect(ui->sampleButton, &QToolButton::clicked, this, &BrowserWidget::slotShowSample);
    connect(ui->chartButton, &QToolButton::clicked, this, &BrowserWidget::slotShowChart);

    connect(ui->importButton, &QPushButton::clicked, this, &BrowserWidget::slotImport);

    connect(ui->browserView, &QTreeView::activated, this, &BrowserWidget::slotFolderItemclicked);
    connect(ui->browserView, &QTreeView::clicked, this, &BrowserWidget::slotFolderItemclicked);
    connect(ui->browserView, &QTreeView::pressed, this, &BrowserWidget::slotFolderPressed);

    connect(ui->browserView, &QTreeView::customContextMenuRequested, this, &BrowserWidget::slotFolderViewContextMenu);

    connect(dirWatcher, &QFileSystemWatcher::directoryChanged, this, &BrowserWidget::slotFolderRefresh);
}

BrowserWidget::~BrowserWidget()
{
    delete ui;
}

void BrowserWidget::initWatcher(QModelIndex parent)
{
    //	qDebug()<<"initWatcher"<<theDirModel->filePath(parent);
    for (int fIdx(0); fIdx < theDirModel->rowCount(parent); ++fIdx) {
        QModelIndex mIdx(theDirModel->index(fIdx, 0, parent));
        //		qDebug()<<"\t"<<theDirModel->filePath(mIdx)<<folderView->isExpanded(mIdx);
        if (ui->browserView->isExpanded(mIdx)) {
            QString fp(theDirModel->filePath(mIdx));
            dirWatcher->addPath(fp);
            qCDebug(FONTMATRIX_LOG) << "***Watch" << fp;
            initWatcher(mIdx);
        }
    }
}

void BrowserWidget::slotFolderAddToWatcher(QModelIndex mIdx)
{
    qCDebug(FONTMATRIX_LOG) << "Add to watcher" << theDirModel->filePath(mIdx);
    dirWatcher->addPath(theDirModel->filePath(mIdx));
}

void BrowserWidget::slotFolderItemclicked(QModelIndex mIdx)
{
    if (curVariant.isEmpty()) {
        ui->infoButton->setEnabled(true);
        ui->sampleButton->setEnabled(true);
        ui->chartButton->setEnabled(true);
    }
    QString path(theDirModel->filePath(mIdx));
    QFileInfo pf(path);
    if (!pf.isDir()) {
        if (FMFontDb::DB()->insertTemporaryFont(path)) {
            //			emit folderSelectFont(pf.absoluteFilePath());
            QString fid(pf.absoluteFilePath());
            if (fid != curVariant) {
                if (chart != nullptr)
                    uniBlock = reinterpret_cast<ChartWidget *>(chart)->currentBlock();
                delete sample;
                delete chart;
                sample = chart = nullptr;
                curVariant = fid;
                //				currentIndex = index.row();
                switch (currentPage) {
                case BROWSER_VIEW_INFO:
                    slotShowInfo();
                    break;
                case BROWSER_VIEW_SAMPLE:
                    slotShowSample();
                    break;
                case BROWSER_VIEW_CHART:
                    slotShowChart();
                    break;
                default:
                    break;
                }

                //				emit fontSelected(curVariant);
            }
        }
    }
    settingsDir(path);
}

void BrowserWidget::slotFolderPressed(QModelIndex mIdx)
{
    currentFIndex = mIdx;
}

void BrowserWidget::slotFolderRefresh([[maybe_unused]] const QString &dirPath)
{
    if (ui->browserView->isVisible()) {
        // QFileSystemModel refreshes automatically via file system monitoring
        qCDebug(FONTMATRIX_LOG) << "Refresh" << dirPath;
    }
}

void BrowserWidget::slotFolderRemoveFromWatcher(QModelIndex mIdx)
{
    qCDebug(FONTMATRIX_LOG) << "Remove from watcher" << theDirModel->filePath(mIdx);
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

    FMConfig::setValue(QStringLiteral("Places/LastUsedFolder"), dirPath);

    s = path;
}

void BrowserWidget::slotShowInfo()
{
    FMInfoDisplay fid(FMFontDb::DB()->Font(curVariant));
    ui->webView->setHtml(fid.getHtml());
    ui->displayStack->setCurrentIndex(BROWSER_VIEW_INFO);
    currentPage = BROWSER_VIEW_INFO;
    updateButtons();
}

void BrowserWidget::slotShowChart()
{
    FloatingWidget *fw(FloatingWidgetsRegister::Widget(curVariant, ChartWidget::Name));
    if (fw == nullptr) {
        if (nullptr == chart) {
            auto cw(new ChartWidget(curVariant, uniBlock, ui->pageChart));
            ui->displayStack->insertWidget(BROWSER_VIEW_CHART, cw);
            chart = cw;
            connect(chart, &FloatingWidget::detached, this, &BrowserWidget::slotDetachChart);
        }
        ui->displayStack->setCurrentWidget(chart);
    } else {
        fw->show();
    }
    currentPage = BROWSER_VIEW_CHART;
    updateButtons();
}

void BrowserWidget::slotShowSample()
{
    FloatingWidget *fw(FloatingWidgetsRegister::Widget(curVariant, SampleWidget::Name));
    if (fw == nullptr) {
        if (nullptr == sample) {
            auto sw(new SampleWidget(curVariant, ui->pageSample));
            ui->displayStack->insertWidget(BROWSER_VIEW_SAMPLE, sw);
            sample = sw;
            connect(sample, &FloatingWidget::detached, this, &BrowserWidget::slotDetachSample);
        }
        ui->displayStack->setCurrentWidget(sample);
    } else {
        fw->show();
    }
    currentPage = BROWSER_VIEW_SAMPLE;
    updateButtons();
}

void BrowserWidget::slotDetachChart()
{
    disconnect(chart, &FloatingWidget::detached, this, &BrowserWidget::slotDetachChart);
    chart = nullptr;
    slotShowInfo();
}

void BrowserWidget::slotDetachSample()
{
    disconnect(sample, &FloatingWidget::detached, this, &BrowserWidget::slotDetachSample);
    sample = nullptr;
    slotShowInfo();
}

void BrowserWidget::slotFolderViewContextMenu(const QPoint &p)
{
    qCDebug(FONTMATRIX_LOG) << "P" << p;
    auto dm = static_cast<QFileSystemModel *>(ui->browserView->model());
    if (!dm)
        return;

    QModelIndex mi = ui->browserView->currentIndex();
    if (!mi.isValid())
        return;

    slotFolderItemclicked(mi); // make sure the font in question is loaded
                               // with a direct right click it would crash without this

    if (!folderViewContextMenu)
        folderViewContextMenu = new FolderViewMenu();

    folderViewContextMenu->exec(dm->fileInfo(mi), mapToGlobal(p));
}

void BrowserWidget::updateButtons()
{
    static QList<QToolButton *> buttons;
    if (buttons.isEmpty()) {
        buttons << ui->sampleButton << ui->infoButton << ui->chartButton;
        for (auto *b : std::as_const(buttons)) {
            b->setCheckable(true);
        }
    }
    for (auto *b : std::as_const(buttons)) {
        b->setChecked(false);
    }
    switch (currentPage) {
    case BROWSER_VIEW_SAMPLE:
        ui->sampleButton->setChecked(true);
        break;
    case BROWSER_VIEW_CHART:
        ui->chartButton->setChecked(true);
        break;
    case BROWSER_VIEW_INFO:
        ui->infoButton->setChecked(true);
        break;
    default:
        break;
    }
}

void BrowserWidget::slotImport()
{
    if (!curVariant.isEmpty())
        typotek::getInstance()->open(curVariant, false, true);
}

FolderViewMenu::FolderViewMenu()
{
    dirAction = new QAction(i18nc("@action:inmenu", "Import Directory"), nullptr);
    dirRecursiveAction = new QAction(i18nc("@action:inmenu", "Import recursively"), nullptr);
    fileAction = new QAction(i18nc("@action:inmenu", "Import File"), nullptr);

    addAction(dirAction);
    addAction(dirRecursiveAction);
    addAction(fileAction);

    connect(dirAction, &QAction::triggered, this, &FolderViewMenu::slotImportDir);
    connect(dirRecursiveAction, &QAction::triggered, this, &FolderViewMenu::slotImportDirRecursively);
    connect(fileAction, &QAction::triggered, this, &FolderViewMenu::slotImportFile);
}

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
    //	QDir dir(selectedFileOrDir.absoluteFilePath());
    //	QStringList ffilter;
    //	ffilter << "*.otf" << "*.ttf" << "*.pfb";
    //	QStringList fontList = dir.entryList(ffilter);
    //	if (fontList.count() < 1)
    //		return;
    //	QString lastItem = fontList.at(fontList.count() - 1);
    //	fontList.removeAt(fontList.count() - 1);
    //	for (const auto& tmpFontPath : fontList) {
    //		QString absPath = dir.absolutePath() + "/" + tmpFontPath;
    //		typotek::getInstance()->open(absPath, false, true);
    //	}
    typotek::getInstance()->open(selectedFileOrDir.absoluteFilePath(), false, true); // import the last font with the announce flag set to true
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

#include "moc_browserwidget.cpp"

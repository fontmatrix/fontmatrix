/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "typotek.h"
#include "browserwidget.h"
#include "dataexport.h"
#include "dataloader.h"
#include "dumpdialog.h"
#include "floatingwidget.h"
#include "floatingwidgetsregister.h"
#include "fmactivate.h"
#include "fmconfig.h"
#include "fmduplicatesdialog.h"
#include "fmfontdb.h"
#include "fmfontextract.h"
#include "fmhandbook.h"
#include "fmhyphenator.h"
#include "fmlayout.h"
#include "fmmatchraster.h"
#include "fmpaths.h"
#include "fmportal.h"
#include "fmrepair.h"
#include "fontbook.h"
#include "fontcomparewidget.h"
#include "fontitem.h"
#include "fontmatrix_debug.h"
#include "importedfontsdialog.h"
#include "importtags.h"
#include "mainviewwidget.h"
#include "panosedialog.h"
#include "panosewidget.h"
#include "playwidget.h"
#include "prefspaneldialog.h"
#include "remotedir.h"
#include "shortcuts.h"
#include "systray.h"
#include "tagswidget.h"
#include "tttableview.h"
#include "winutils.h"

#include <KLocalizedString>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QProcess>
#include <QProgressBar>
#include <QProgressDialog>
#include <QScreen>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextEdit>
#include <QTextStream>
#include <QUrl>
#include <cstdio>
#include <memory>

#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

#include <KAboutData>
#include <KActionCollection>
#include <KConfigGroup>
#include <KHamburgerMenu>
#include <KHelpClient>
#include <KMessageBox>
#include <KSandbox>
#include <KSharedConfig>
#include <KStandardAction>
#include <KToolBar>

#ifdef Q_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#endif

typotek *typotek::instance = nullptr;
bool typotek::matrix = false;
QString typotek::fonteditorPath = "/usr/bin/fontforge";
extern bool __FM_SHOW_FONTLOADED;
extern int fm_num_face_opened;

/// LazyInit *********************************************
void LazyInit::run()
{
    /// We keep this for further needs
    Q_EMIT endOfRun();
}
///******************************************************

/// a bit of globalness *******************************************
namespace fontmatrix
{
QStringList exploreDirs(const QDir &dir, int deep)
{
    static QStringList retDirList;
    if (deep > 10)
        return QStringList();
    if (deep == 0)
        retDirList.clear();
    retDirList << dir.absolutePath();
    QStringList localEntries(dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot));
    for (const auto &dirEntry : std::as_const(localEntries)) {
        // 			qDebug() << "[exploreDirs] - " + dir.absolutePath() + "/" + dirEntry;
        QDir d(dir.absolutePath() + "/" + dirEntry);
        exploreDirs(d, deep + 1);
        if (!retDirList.contains(d.absolutePath()))
            retDirList << d.absolutePath();
    }

    return retDirList;
}

QMap<QString, Qt::DockWidgetArea> DockPosition;

void fillDockPos()
{
    DockPosition["Float"] = Qt::LeftDockWidgetArea;
    DockPosition["Left"] = Qt::LeftDockWidgetArea;
    DockPosition["Right"] = Qt::RightDockWidgetArea;
    DockPosition["Top"] = Qt::TopDockWidgetArea;
    DockPosition["Bottom"] = Qt::BottomDockWidgetArea;
}
}

/// *****************************************************

typotek *typotek::getInstance()
{
    if (!instance) {
        instance = new typotek;
        Q_ASSERT(instance);
    }
    return instance;
}

typotek::typotek()
{
    setWindowTitle("Fontmatrix");
    setupDrop();
    theMainView = nullptr;
    hyphenator = nullptr;
    dataLoader = nullptr;
    playVisible = false;

    m_dpiX = (double)QApplication::primaryScreen()->physicalDotsPerInchX();
    m_dpiY = (double)QApplication::primaryScreen()->physicalDotsPerInchY();
#ifdef Q_OS_MAC
    CGDirectDisplayID macDId = CGMainDisplayID();
    CGRect macDRect = CGDisplayBounds(macDId);
    CGSize macDSize = CGDisplayScreenSize(macDId);

    double macDisplayPxWidth(macDRect.size.width);
    double macDisplayPxHeight(macDRect.size.height);
    double macDisplayPhysicalWidth(double(macDSize.width) / 25.4);
    double macDisplayPhysicalHeight(double(macDSize.height) / 25.4);

    m_dpiX = macDisplayPxWidth / macDisplayPhysicalWidth;
    m_dpiY = macDisplayPxHeight / macDisplayPhysicalHeight;
#endif

    qCDebug(FONTMATRIX_LOG) << m_dpiX << m_dpiY;
}

void typotek::initMatrix()
{
    qCDebug(FONTMATRIX_LOG) << "Main Thread:" << thread();
    if (matrix)
        return;
    matrix = true;
    fontmatrix::fillDockPos();

    checkOwnDir();
    readSettings();
    initDir();

    theMainView = new MainViewWidget(this);
    theBrowser = new BrowserWidget(this);
    mainStack = new QStackedWidget(this);

    mainStack->addWidget(theMainView);
    mainStack->addWidget(theBrowser);
    setCentralWidget(mainStack);

    systray = new Systray();

    setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::ForceTabbedDocks);

    //	installDock("Tags", i18n( "Tags" ), TagsWidget::getInstance() ,  i18n( "Show/hide tags list sidebar" ) );
    //	installDock("Panose", i18n( "Panose"), PanoseWidget::getInstance(), i18n( "Browse fonts by means of Panose attributes" ) );

    // force tabifyication
    //	QStringList dl;
    //	dl << "Tags" << "Panose" << "Main";
    //	for(int i(1); i < dl.count(); ++i)
    //	{
    //		if(dockArea[dl[i]] != "Float")
    //		{
    //			for(int j(i-1); j >=0 ; --j)
    //			{
    //				if(dockArea[dl[j]] == dockArea[dl[i]])
    //					tabifyDockWidget(dockWidget[dl[j]], dockWidget[dl[i]]);
    //			}
    //		}
    //	}

    createActions();
    createStatusBar();

    // Inspired by Kate: enable the standard toolbar menu, set the RC, build the GUI.
    setStandardToolBarMenuEnabled(true);
    setXMLFile(QStringLiteral("fontmatrixui.rc"));
    createGUI(xmlFile());

    // KHelpMenu connects help_contents to KHelpClient::invokeHelp(), which
    // opens a help: URL and does nothing where no handler for that scheme is
    // installed.
    if (QAction *helpAction = actionCollection()->action(QStringLiteral("help_contents"))) {
        disconnect(helpAction, &QAction::triggered, nullptr, nullptr);
        connect(helpAction, &QAction::triggered, this, &typotek::slotHelpContents);
    }

    // Restore Show Menu Bar from the General group; must run after createGUI().
    {
        KConfigGroup generalGroup(KSharedConfig::openConfig(), QStringLiteral("General"));
        m_paShowMenuBar->setChecked(generalGroup.readEntry("Show Menu Bar", true));
        toggleShowMenuBar(false);
        connect(m_paShowMenuBar, &QAction::toggled, this, [this] {
            KConfigGroup g(KSharedConfig::openConfig(), QStringLiteral("General"));
            g.writeEntry("Show Menu Bar", m_paShowMenuBar->isChecked());
        });
    }

    // Exclude toolbar actions from hamburger menu to avoid duplicates
    if (auto *hm = qobject_cast<KHamburgerMenu *>(actionCollection()->action(QStringLiteral("hamburger_menu"))))
        hm->hideActionsOf(toolBar());
    // Wire the dynamic View menu for floating-panel entries
    for (const auto actionsList = menuBar()->actions(); auto *menuAction : actionsList) {
        if (QMenu *menu = menuAction->menu(); menu && menu->objectName() == QStringLiteral("view")) {
            viewMenu = menu;
            connect(viewMenu, &QMenu::aboutToShow, this, &typotek::updateFloatingStatus);
            break;
        }
    }
    doConnect();

    showToltalFilteredFonts();

    updateHyphenation();
}

void typotek::updateHyphenation()
{
    if (!hyphenator)
        hyphenator = new FMHyphenator();
    // A dictionary chosen in Preferences comes first; otherwise the one the system has
    // for the language of the sample shown, and none at all when it has none: a text
    // hyphenated by the rules of another language is worse than one left whole.
    QString dP(FMConfig::value(QStringLiteral("Sample/HyphenationDict"), QString()).toString());
    if (!dP.isEmpty() && !QFileInfo::exists(dP)) {
        qCWarning(FONTMATRIX_LOG) << "The hyphenation dictionary of the preferences is gone:" << dP;
        FMConfig::remove(QStringLiteral("Sample/HyphenationDict"));
        dP.clear();
    }
    const QLocale language(namedSampleLocale(currentNamedSample));
    if (dP.isEmpty())
        dP = FMPaths::HyphenationDictionary(language);
    if (dP.isEmpty()) {
        if (!hyphenator->dictPath().isEmpty())
            qCDebug(FONTMATRIX_LOG) << "No hyphenation dictionary for" << language.name() << "- the sample is not hyphenated";
        hyphenator->unload();
        return;
    }
    if (dP == hyphenator->dictPath())
        return;
    if (hyphenator->loadDict(dP, FMConfig::value(QStringLiteral("Sample/HyphLeft"), 2).toInt(), FMConfig::value(QStringLiteral("Sample/HyphRight"), 3).toInt()))
        qCDebug(FONTMATRIX_LOG) << "Hyphenation dictionary for" << language.name() << ":" << dP;
    else
        qCWarning(FONTMATRIX_LOG) << "Cannot load the hyphenation dictionary" << dP;
}

QLocale typotek::namedSampleLocale(const QString &name)
{
    // "Bulgarian::Българският език" names its language; a sample of the user's own does
    // not, and is taken to be in the language of the interface
    const QString group(name.section(QStringLiteral("::"), 0, 0));
    if (group.isEmpty() || group == QLatin1String("User") || !dataLoader)
        return QLocale::system();
    const QLocale locale(dataLoader->sampleLocale(group));
    return locale.language() == QLocale::C ? QLocale::system() : locale;
}

void typotek::installDock(const QString &id, const QString &name, QWidget *w, const QString &tip)
{
    auto dw = new QDockWidget(name);
    dw->setObjectName(id);
    dockWidget[id] = dw;
    dw->setWidget(w);
    dw->setStatusTip(tip);
    addDockWidget(fontmatrix::DockPosition[dockArea[id]], dw);
    qCDebug(FONTMATRIX_LOG) << "I" << id << dockArea[id] << dockVisible[id];
    if (dockArea[id] == QString("Float"))
        dw->setFloating(true);
    if (!dockGeometry[id].isNull())
        dw->setGeometry(dockGeometry[id]);

    connect(dw, &QDockWidget::dockLocationChanged, this, &typotek::slotDockAreaChanged);
}

void typotek::postInit()
{
}

void typotek::doConnect()
{
    if (getSystray())
        connect(FMActivate::getInstance(), &FMActivate::activationEvent, getSystray(), &Systray::updateTagMenu);

    //	connect(FMLayout::getLayout()->optionDialog,SIGNAL(finished( int )),this,SLOT(slotUpdateLayOptStatus()));
    connect(toggleMainViewButton, &QToolButton::toggled, this, &typotek::toggleMainView);
    connect(this, &typotek::newFontsArrived, theMainView, &MainViewWidget::slotFontDbChanged);
}

void typotek::closeEvent(QCloseEvent *event)
{
    if (systray && !m_forceQuit) {
        if (systray->isVisible() && FMConfig::value(QStringLiteral("Systray/CloseToTray"), true).toBool()) {
            if (!FMConfig::value(QStringLiteral("Systray/CloseNoteShown"), false).toBool()) {
                KMessageBox::information(this,
                                         i18nc("@info",
                                               "The program will keep running in the "
                                               "system tray. To terminate the program, "
                                               "choose <b>Exit</b> in the context menu "
                                               "of the system tray entry."));
                FMConfig::setValue(QStringLiteral("Systray/CloseNoteShown"), true);
            }
            hide();
            event->ignore();
            return;
        }
    }

    m_closing = true;
    for (const auto allWidgets = FloatingWidgetsRegister::AllWidgets(); auto *f : allWidgets) {
        f->close();
    }
    writeSettings();

    delete PlayWidget::getInstance();
    delete FontCompareWidget::getInstance();
    delete theMainView;

    event->accept();

    // KStatusNotifierItem holds a D-Bus service registration that keeps the
    // process alive past lastWindowClosed when the tray is not visible to the
    // user. Quit explicitly so closing the X button always terminates the
    // process unless the user opted into "close to tray" above.
    qApp->quit();
}

void typotek::slotQuit()
{
    // File → Quit (and the systray "Exit" action) must terminate even when
    // "close to tray" is on. Set the flag so closeEvent skips its hide branch.
    m_forceQuit = true;
    close();
}

/**
 * Whether the path is in the folder Fontmatrix activates fonts into. What it
 * holds is a copy of a font the database has already (on Windows and macOS,
 * also fonts the user installed there), so nothing in it is imported.
 */
bool typotek::isInUserFontFolder(const QString &path) const
{
    const QString folder(managedDir.absolutePath() + QLatin1Char('/'));
    const QString p(QDir::cleanPath(path) + QLatin1Char('/'));
#ifdef _WIN32
    return p.startsWith(folder, Qt::CaseInsensitive);
#else
    return p.startsWith(folder);
#endif
}

/// IMPORT
// if announce == true user will be shown a dialog of imported fonts
// if announce == false and collect == true all fonts imported will be
// collected and announced next time announce == true
void typotek::open(QString path, bool recursive, bool announce, bool collect)
{
    static QStringList nameList;
    static QStringList tali; // tali gets reseted when announce = true then the shouldAskTali is also set to true
    static bool shouldAskTali = true; // initial tags is only asked once if collect == true
    QStringList pathList;

    QFileInfo finfo(path);
    if (finfo.isDir() || path.isEmpty()) // importing a directory
    {
        QString dir = FMConfig::value(QStringLiteral("Places/LastUsedFolder"), QDir::homePath())
                          .toString(); // first time use the home path then remember the last used dir
        QDir d(dir);
        if (!d.exists())
            dir = QDir::homePath();

        QString tmpdir;

        if (!path.isEmpty())
            tmpdir = path;
        else
            tmpdir = QFileDialog::getExistingDirectory(this, i18nc("@title:window", "Add Directory"), dir, QFileDialog::ShowDirsOnly);

        if (tmpdir.isEmpty())
            return; // user choose to cancel the import process

        dir = tmpdir; // only set dir if importing wasn't cancelled
        FMConfig::setValue(QStringLiteral("Places/LastUsedFolder"), dir);

        QStringList dirList;
        if (recursive)
            dirList = fontmatrix::exploreDirs(dir, 0);
        else
            dirList.append(dir);
        // 	qDebug() << dirList.join ( "\n" );

        QStringList yetHereFonts;
        // 		for(int i=0;i < fontMap.count() ; ++i)
        // 			yetHereFonts << fontMap[i]->path();
        yetHereFonts = FMFontDb::DB()->AllFontNames();

        QStringList filters;
        filters << "*.otf" << "*.pfb" << "*.ttf" << "*.ttc";
        for (const auto &dr : std::as_const(dirList)) {
            if (isInUserFontFolder(dr))
                continue;
            QDir d(dr);
            QFileInfoList fil = d.entryInfoList(filters);
            for (const auto &fp : std::as_const(fil)) {
                if ((!yetHereFonts.contains(fp.absoluteFilePath()))) {
                    if (fp.isSymLink()) // #12232
                    {
                        QFileInfo fsym(fp.symLinkTarget());
                        if ((!fsym.isSymLink()) // hey, donnot try to fool us with nested symlinks :)
                            && (fsym.exists()) && (!yetHereFonts.contains(fsym.absoluteFilePath())))
                            pathList << fsym.absoluteFilePath();

                    } else
                        pathList << fp.absoluteFilePath();
                }
            }
        }
    } else if (finfo.isFile() && !isInUserFontFolder(finfo.absoluteFilePath()))
        pathList << finfo.absoluteFilePath();

    // It can happen that you wrongly select a dir, it is time to let the user cancel the import.
    // I want it :) - pm
    if (/*( pathList.count() > 1 )
       &&*/
        (KMessageBox::questionTwoActions(
             this,
             i18ncp("@info", "Do you confirm you want to import %1 font?", "Do you confirm you want to import %1 fonts?", pathList.count()),
             i18nc("@title:window", "Confirmation"),
             KGuiItem(i18nc("@action:button", "Import"), QStringLiteral("document-import")),
             KStandardGuiItem::cancel(),
             QString(),
             KMessageBox::Options(KMessageBox::Notify | KMessageBox::Dangerous))
         != KMessageBox::PrimaryAction)) {
        return;
    }

    /* Everybody say it’s useless...
        NO IT'S NOT. I'm a keen fan of this feature. Let's make it optional */
    QStringList tagsList(FMFontDb::DB()->getTags());
    if (useInitialTags && shouldAskTali) {
        ImportTags imp(this, tagsList);
        imp.exec();
        tali = imp.tags();
        shouldAskTali = false;
    }

    QProgressDialog progress(i18nc("@info:progress", "Importing font files... "), i18nc("@action:button", "cancel"), 0, pathList.count(), this);
    bool showProgress = pathList.count() > 1;
    if (showProgress) // show progress bar only if there's more than one font
    {
        progress.setWindowModality(Qt::WindowModal);
        progress.setAutoReset(false);
        progress.setValue(0);
        progress.show();
    }
    FMFontDb::DB()->TransactionBegin();
    QString importstring(i18nc("@info:progress", "Import") + " %1");
    FMFontDb *DB(FMFontDb::DB());
    QList<FontItem *> nf;
    for (int i = 0; i < pathList.count(); ++i) {
        QString pathCur(pathList.at(i));
        if (showProgress) {
            progress.setLabelText(importstring.arg(pathCur));
            progress.setValue(i);
            if (progress.wasCanceled())
                break;
        }

        {
            QFile ff(pathCur);
            QFileInfo fi(pathCur);
            {
                FontItem *fitem(DB->Font(fi.absoluteFilePath(), true));
                if (fitem) {
                    nf << fitem;
                    fitem->setActivated(false);
                    if (announce || collect)
                        nameList << fitem->fancyName();
                } else {
                    QString errorFont(i18nc("@info", "Cannot import this font because it is broken:") + " " + fi.fileName());
                    statusBar()->showMessage(errorFont);
                    if (announce || collect)
                        nameList << "__FAILEDTOLOAD__" + fi.fileName();
                }
            }
        }
    }

    QStringList tl;
    for (const auto &tag : std::as_const(tali)) {
        tl.clear();
        for (auto *f : std::as_const(nf)) {
            tl << f->path();
        }
        DB->addTag(tl, tag);
    }
    DB->TransactionEnd();
    progress.close();

    if (announce) {
        if (showFontListDialog) {
            // The User needs and deserves to know what fonts hve been imported
            ImportedFontsDialog ifd(this, nameList);
            ifd.exec();
        } else // show info in the statusbar
        {
            statusBar()->showMessage(i18nc("@info:status", "Fonts imported: %1", nameList.count()), 3000);
        }
        nameList.clear();
        tali.clear();
        shouldAskTali = true;
    }
    Q_EMIT newFontsArrived();
}

void typotek::importFiles()
{
    QStringList flist = QFileDialog::getOpenFileNames(this,
                                                      i18nc("@title:window", "Select Files to Import"),
                                                      QDir::homePath(),
                                                      QString("%1 (*.otf *.ttf *.ttc *.pfb)").arg(i18nc("@item:inlistbox file type filter", "Font Files")));
    if (!flist.isEmpty())
        openList(flist);
}

/// Import files in a drop event.
void typotek::openList(QStringList files)
{
    QStringList pathList;
    QStringList nameList;
    QStringList tali;
    QStringList tagsList(FMFontDb::DB()->getTags());
    if (useInitialTags) {
        ImportTags imp(this, tagsList);
        imp.exec();
        tali = imp.tags();
    }

    FMFontDb *DB(FMFontDb::DB());
    QStringList fontMap(DB->AllFontNames());
    for (const auto &file : files) {
        QFileInfo fp(file);
        if ((!fontMap.contains(fp.absoluteFilePath()))) {
            if (fp.isSymLink()) // #12232
            {
                QFileInfo fsym(fp.symLinkTarget());
                if ((!fsym.isSymLink()) && (fsym.exists()) && (!fontMap.contains(fsym.absoluteFilePath())))
                    pathList << fsym.absoluteFilePath();

            } else
                pathList << fp.absoluteFilePath();
        }
    }

    DB->TransactionBegin();
    QProgressDialog progress(i18nc("@info:progress", "Importing font files... "), i18nc("@action:button", "cancel"), 0, pathList.count(), this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoReset(false);

    QList<FontItem *> nf;
    QString importstring(i18nc("@info:progress", "Import") + " %1");
    for (int i = 0; i < pathList.count(); ++i) {
        progress.setLabelText(importstring.arg(pathList.at(i)));
        progress.setValue(i);
        if (progress.wasCanceled())
            break;

        QFile ff(pathList.at(i));
        QFileInfo fi(pathList.at(i));

        FontItem *fitem = DB->Font(fi.absoluteFilePath(), true);
        if (fitem) {
            nf << fitem;
            nameList << fitem->fancyName();
        } else {
            nameList << "__FAILEDTOLOAD__" + fi.fileName();
        }
    }
    QStringList tl;
    for (const auto &tag : std::as_const(tali)) {
        tl.clear();
        for (auto *f : std::as_const(nf)) {
            tl << f->path();
        }
        DB->addTag(tl, tag);
    }
    DB->TransactionEnd();
    DB->announceTagsChanged();
    progress.close();

    // The User needs and deserves to know what fonts hve been imported
    if (showFontListDialog) {
        // The User needs and deserves to know what fonts hve been imported
        ImportedFontsDialog ifd(this, nameList);
        ifd.exec();
    } else // show info in the statusbar
    {
        statusBar()->showMessage(i18nc("@info:status", "Fonts imported: %1", nameList.count()), 3000);
    }
    Q_EMIT newFontsArrived();
}

/// EXPORT
void typotek::slotExportFontSet()
{
    //	QStringList tagsList(FMFontDb::DB()->getTags());
    //	QStringList items ( tagsList );
    //	bool ok;
    //	QString item = QInputDialog::getItem ( this, i18n( "Fontmatrix Tags" ),
    //	                                       i18n( "Choose the tag for filter exported fonts" ), items, 0, false, &ok );
    //	if ( ok && !item.isEmpty() )
    //	{

    //		QString dir( QDir::homePath() );
    //		dir = QFileDialog::getExistingDirectory ( this, i18n( "Choose Directory" ), dir  ,  QFileDialog::ShowDirsOnly );
    //		if ( dir.isEmpty() )
    //			return;

    //		DataExport dx(dir,item);
    //		dx.doExport();
    //	}
    new DataExport(this);
}

void typotek::createActions()
{
    Shortcuts *scuts = Shortcuts::getInstance();

    openAct = new QAction(i18nc("@action:inmenu", "&Import Directory..."), this);
    openAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    openAct->setToolTip(i18nc("@info:tooltip", "Import a directory"));
    scuts->add(openAct);
    connect(openAct, &QAction::triggered, this, [this]() {
        open();
    });

    importFilesAction = new QAction(i18nc("@action:inmenu", "Import &Files..."), this);
    importFilesAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    importFilesAction->setToolTip(i18nc("@info:tooltip", "Import Files"));
    scuts->add(importFilesAction);
    connect(importFilesAction, &QAction::triggered, this, &typotek::importFiles);

    exportFontSetAct = new QAction(i18nc("@action:inmenu", "Export &fonts"), this);
    exportFontSetAct->setStatusTip(i18nc("@info:status", "Export a fontset"));
    scuts->add(exportFontSetAct);
    connect(exportFontSetAct, &QAction::triggered, this, &typotek::slotExportFontSet);

    fontBookAct = new QAction(i18nc("@action:inmenu", "Export font book..."), this);
    fontBookAct->setStatusTip(i18nc("@info:status", "Export a PDF document that shows selected fonts"));
    scuts->add(fontBookAct);
    connect(fontBookAct, &QAction::triggered, this, &typotek::fontBook);

    dumpInfoAct = new QAction(i18nc("@action:inmenu", "Export font info for packaging..."), this);
    dumpInfoAct->setStatusTip(i18nc("@info:status", "Fill a template file with metadata for packaging currently selected font to a Linux distribution"));
    connect(dumpInfoAct, &QAction::triggered, this, &typotek::slotDumpInfo);

    // 	tagsetAct = new QAction ( i18n( "&Tag Sets" ),this );
    // 	tagsetAct->setIcon ( QIcon ( ":/fontmatrix_tagseteditor_icon.png" ) );
    // 	scuts->add(tagsetAct);
    // 	connect ( tagsetAct,SIGNAL ( triggered( ) ),this,SLOT ( popupTagsetEditor() ) );

    activCurAct = new QAction(i18nc("@action:inmenu", "Activate all current"), this);
    activCurAct->setStatusTip(i18nc("@info:status", "Activate all currently visible fonts"));
    scuts->add(activCurAct);
    connect(activCurAct, &QAction::triggered, this, &typotek::slotActivateCurrents);

    deactivCurAct = new QAction(i18nc("@action:inmenu", "Deactivate all current"), this);
    deactivCurAct->setStatusTip(i18nc("@info:status", "Deactivate all currently visible fonts"));
    scuts->add(deactivCurAct);
    connect(deactivCurAct, &QAction::triggered, this, &typotek::slotDeactivateCurrents);

    fonteditorAct = new QAction(i18nc("@action:inmenu", "Edit current font"), this);
    scuts->add(fonteditorAct);
    connect(fonteditorAct, &QAction::triggered, this, &typotek::slotEditFont);
    updateFontEditorAction();

    reloadAct = new QAction(i18nc("@action:inmenu", "Reload Filtered"), this);
    reloadAct->setStatusTip(i18nc("@info:status", "Reload informations for filtered fonts from the font files they belong to"));
    scuts->add(reloadAct);
    connect(reloadAct, &QAction::triggered, this, &typotek::slotReloadFiltered);

    reloadSingleAct = new QAction(i18nc("@action:inmenu", "Reload Selected"), this);
    reloadSingleAct->setStatusTip(i18nc("@info:status", "Reload informations for selected font from the font file"));
    scuts->add(reloadSingleAct);
    connect(reloadSingleAct, &QAction::triggered, this, &typotek::slotReloadSingle);

    repairAct = new QAction(i18nc("@action:inmenu", "Check Database"), this);
    repairAct->setStatusTip(i18nc("@info:status", "Check Fontmatrix database for dead links to font files"));
    scuts->add(repairAct);
    connect(repairAct, &QAction::triggered, this, &typotek::slotRepair);

    //	if ( systray )
    //		connect ( theMainView, SIGNAL ( newTag ( QString ) ), systray, SLOT ( newTag ( QString ) ) );

    tagAll = new QAction(i18nc("@action:inmenu", "Tag All Filtered..."), this);
    tagAll->setStatusTip(i18nc("@info:status", "Tag all currently visible files"));
    scuts->add(tagAll);
    connect(tagAll, &QAction::triggered, this, &typotek::slotTagAll);

    showTTTAct = new QAction(i18nc("@action:inmenu", "Show TrueType tables"), this);
    showTTTAct->setStatusTip(i18nc("@info:status", "View hexadecimal values of TrueType tables for currently selected font file"));
    scuts->add(showTTTAct);
    connect(showTTTAct, &QAction::triggered, this, &typotek::slotShowTTTables);

    editPanoseAct = new QAction(i18nc("@action:inmenu", "Edit PANOSE metadata"), this);
    editPanoseAct->setStatusTip(i18nc("@info:status", "Edit PANOSE metadata without saving changes to font files"));
    scuts->add(editPanoseAct);
    connect(editPanoseAct, &QAction::triggered, this, &typotek::slotEditPanose);

    // The playground and the comparison are palettes over the font list: fonts are picked
    // in the main window while they are open. As tool windows of the main window they stay
    // above it; as windows of their own they went behind it at the first click in the list.
    PlayWidget::getInstance()->setParent(this, Qt::Tool);
    FontCompareWidget::getInstance()->setParent(this, Qt::Tool);

    playAction = new QAction(i18nc("@action:inmenu", "Playground"), this);
    playAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    playAction->setToolTip(i18nc("@info:tooltip", "Show/Hide Playground"));
    playAction->setCheckable(true);
    playAction->setChecked(false);
    scuts->add(playAction);
    connect(playAction, &QAction::triggered, PlayWidget::getInstance(), &PlayWidget::setVisible);

    compareAction = new QAction(i18nc("@action:inmenu", "Compare"), this);
    compareAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    compareAction->setToolTip(i18nc("@info:tooltip", "Show/Hide Compare glyphs"));
    compareAction->setCheckable(true);
    compareAction->setChecked(false);
    scuts->add(compareAction);
    connect(compareAction, &QAction::triggered, FontCompareWidget::getInstance(), &FontCompareWidget::setVisible);

    closeAllFloat = new QAction(i18nc("@action:inmenu", "Close All"), this);
    closeAllFloat->setToolTip(i18nc("@info:tooltip", "Close all floating windows"));
    scuts->add(closeAllFloat);
    connect(closeAllFloat, &QAction::triggered, this, &typotek::closeAllFloatings);

    showAllFloat = new QAction(i18nc("@action:inmenu", "Show All"), this);
    showAllFloat->setToolTip(i18nc("@info:tooltip", "Show all floating windows"));
    scuts->add(showAllFloat);
    connect(showAllFloat, &QAction::triggered, this, &typotek::showAllFloatings);

    hideAllFloat = new QAction(i18nc("@action:inmenu", "Hide All"), this);
    hideAllFloat->setToolTip(i18nc("@info:tooltip", "Hide all floating windows"));
    scuts->add(hideAllFloat);
    connect(hideAllFloat, &QAction::triggered, this, &typotek::hideAllFloatings);

    floatSep = new QAction(this);
    floatSep->setSeparator(true);

    // Connected only now: updateFloatingStatus() dereferences closeAllFloat,
    // showAllFloat, hideAllFloat and floatSep, which are created just above.
    //
    // Neither window routes a close through its action — the user can shut
    // them from their own title bar — so without these the Playground and
    // Compare buttons stay checked until the View menu is next opened, which
    // is what refreshed them before.
    connect(PlayWidget::getInstance(), &PlayWidget::visibilityChanged, this, &typotek::updateFloatingStatus);
    connect(FontCompareWidget::getInstance(), &FontCompareWidget::visibilityChanged, this, &typotek::updateFloatingStatus);

#ifdef HAVE_PODOFO
    // PDF is the only kind of document there is an extractor for; without PoDoFo the dialog
    // could not open anything, so the action is not offered.
    extractFontAction = new QAction(i18nc("@action:inmenu", "Extract fonts..."), this);
    extractFontAction->setStatusTip(i18nc("@info:status", "Extract fonts from documents like PDF to PFM file format"));
    scuts->add(extractFontAction);
    connect(extractFontAction, &QAction::triggered, this, &typotek::slotExtractFont);
#endif

    duplicatesAct = new QAction(i18nc("@action:inmenu", "Duplicates..."), this);
    duplicatesAct->setStatusTip(i18nc("@info:status", "The fonts the collection holds more than once, and a way to remove the extra files"));
    scuts->add(duplicatesAct);
    connect(duplicatesAct, &QAction::triggered, this, &typotek::slotShowDuplicates);

    matchRasterAct = new QAction(i18nc("@action:inmenu", "Find a font using raster sample..."), this); // FIXME find a name for it
    matchRasterAct->setStatusTip(i18nc("@info:status", "Find a font using a raster sample of a letter"));
    scuts->add(matchRasterAct);
    connect(matchRasterAct, &QAction::triggered, this, &typotek::slotMatchRaster);

    exportXeTeXAct = new QAction(i18nc("@action:inmenu", "Export font list as XeTeX..."), this);
    exportXeTeXAct->setStatusTip(i18nc("@info:status", "Export the current filtered font list as a XeTeX source file"));
    scuts->add(exportXeTeXAct);
    connect(exportXeTeXAct, &QAction::triggered, this, &typotek::slotExportXeTeX);

    auto *ac = actionCollection();

    ac->addAction(QStringLiteral("file_import_dir"), openAct);
    ac->setDefaultShortcut(openAct, QKeySequence(Qt::CTRL | Qt::Key_O));
    ac->addAction(QStringLiteral("file_import_files"), importFilesAction);
    ac->setDefaultShortcut(importFilesAction, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    ac->addAction(QStringLiteral("file_export_fontset"), exportFontSetAct);
    ac->addAction(QStringLiteral("file_font_book"), fontBookAct);
    ac->addAction(QStringLiteral("file_dump_info"), dumpInfoAct);

    ac->addAction(QStringLiteral("edit_tag_all"), tagAll);
    ac->addAction(QStringLiteral("edit_activate_current"), activCurAct);
    ac->addAction(QStringLiteral("edit_deactivate_current"), deactivCurAct);
    ac->addAction(QStringLiteral("edit_font_editor"), fonteditorAct);
    ac->addAction(QStringLiteral("edit_panose"), editPanoseAct);
    ac->addAction(QStringLiteral("edit_reload_filtered"), reloadAct);
    ac->addAction(QStringLiteral("edit_reload_single"), reloadSingleAct);

    ac->addAction(QStringLiteral("view_playground"), playAction);
    ac->setDefaultShortcut(playAction, QKeySequence(Qt::CTRL | Qt::Key_G));
    ac->addAction(QStringLiteral("view_compare"), compareAction);
    ac->setDefaultShortcut(compareAction, QKeySequence(Qt::CTRL | Qt::Key_R));
    ac->addAction(QStringLiteral("view_close_all"), closeAllFloat);
    ac->addAction(QStringLiteral("view_show_all"), showAllFloat);
    ac->addAction(QStringLiteral("view_hide_all"), hideAllFloat);

#ifdef HAVE_PODOFO
    ac->addAction(QStringLiteral("tools_extract_font"), extractFontAction);
#endif
    ac->addAction(QStringLiteral("tools_duplicates"), duplicatesAct);
    ac->addAction(QStringLiteral("tools_match_raster"), matchRasterAct);
    ac->addAction(QStringLiteral("tools_export_xetex"), exportXeTeXAct);
    ac->addAction(QStringLiteral("tools_repair"), repairAct);
    ac->addAction(QStringLiteral("tools_tt_tables"), showTTTAct);

    KStandardAction::quit(this, &typotek::slotQuit, ac);
    KStandardAction::preferences(this, &typotek::slotPrefsPanelDefault, ac);

    // KDE theme icons (XDG standard names) for menu and toolbar actions.
    openAct->setIcon(QIcon::fromTheme(QStringLiteral("folder-open")));
    importFilesAction->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    fontBookAct->setIcon(QIcon::fromTheme(QStringLiteral("document-print-preview")));
    activCurAct->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    deactivCurAct->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-stop")));
    compareAction->setIcon(QIcon::fromTheme(QStringLiteral("view-split-left-right")));
    playAction->setIcon(QIcon::fromTheme(QStringLiteral("applications-games")));
    // Other menu actions
#ifdef HAVE_PODOFO
    extractFontAction->setIcon(QIcon::fromTheme(QStringLiteral("package-x-generic")));
#endif
    exportXeTeXAct->setIcon(QIcon::fromTheme(QStringLiteral("document-export")));
    repairAct->setIcon(QIcon::fromTheme(QStringLiteral("tools-check-spelling")));
    duplicatesAct->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
    showTTTAct->setIcon(QIcon::fromTheme(QStringLiteral("document-properties")));
    tagAll->setIcon(QIcon::fromTheme(QStringLiteral("tag")));
    fonteditorAct->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
    editPanoseAct->setIcon(QIcon::fromTheme(QStringLiteral("format-list-unordered")));
    exportFontSetAct->setIcon(QIcon::fromTheme(QStringLiteral("document-save-as")));
    reloadAct->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    reloadSingleAct->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    matchRasterAct->setIcon(QIcon::fromTheme(QStringLiteral("zoom-fit-best")));

    // "Show Menu Bar" toggle and hamburger menu — the pattern comes from Kate.
    // The hamburger goes through KStandardAction::HamburgerMenu so XMLGUI
    // builds a real KHamburgerMenu container.
    m_paShowMenuBar = KStandardAction::showMenubar(this, &typotek::toggleShowMenuBar, ac);

    auto *hamburgerMenu = static_cast<KHamburgerMenu *>(ac->addAction(KStandardAction::HamburgerMenu, QStringLiteral("hamburger_menu")));
    hamburgerMenu->setMenuBar(menuBar());
    hamburgerMenu->setShowMenuBarAction(m_paShowMenuBar);
}

void typotek::createStatusBar()
{
    //	statusBar()->showMessage ( i18n( "Ready" ) );

    statusProgressBar = new QProgressBar(this);
    statusProgressBar->setMaximumSize(200, 20);
    statusBar()->addPermanentWidget(statusProgressBar);
    statusProgressBar->hide();

    QFont statusFontFont("sans-serif", 10);
    curFontPresentation = new QLabel("");
    curFontPresentation->setFrameShape(QFrame::StyledPanel);
    curFontPresentation->setAlignment(Qt::AlignRight);
    curFontPresentation->setFont(statusFontFont);
    statusBar()->insertWidget(0, curFontPresentation);

    countFilteredFonts = new QLabel("");
    countFilteredFonts->setFrameShape(QFrame::StyledPanel);
    countFilteredFonts->setAlignment(Qt::AlignRight);
    countFilteredFonts->setFont(statusFontFont);
    statusBar()->addPermanentWidget(countFilteredFonts);

    toggleMainViewButton = new QToolButton(this);
    toggleMainViewButton->setText(i18nc("@action:button", "Browse Directories"));
    toggleMainViewButton->setCheckable(true);
    toggleMainViewButton->setToolTip(i18nc("@info:tooltip", "Toggle Files/Collection view"));
    statusBar()->addPermanentWidget(toggleMainViewButton);
}

void typotek::readSettings()
{
    relayStartingStepIn(i18nc("@info:progress", "Load settings"));
    // KMainWindow restores window geometry via the autoSaveConfigGroup() mechanism
    // (see writeSettings()), so no explicit pos/size restore is needed here.

    fonteditorPath = FMConfig::value(QStringLiteral("FontEditor"), "/usr/bin/fontforge").toString();
    useInitialTags = FMConfig::value(QStringLiteral("UseInitialTags"), false).toBool();
    showFontListDialog = FMConfig::value(QStringLiteral("ShowImportedFonts"), true).toBool();
    previewSize = FMConfig::value(QStringLiteral("Preview/Size"), 28.0).toDouble();
    previewRTL = FMConfig::value(QStringLiteral("Preview/RTL"), false).toBool();
    previewSubtitled = FMConfig::value(QStringLiteral("Preview/Subtitled"), false).toBool();
    m_theWord = FMConfig::value(QStringLiteral("Preview/Word"), "<name>").toString();

    //	QStringList dl;
    //	dl << "Main" << "Tags" << "Panose";
    //	for (const auto& ds : dl)
    //	{
    //		dockArea[ds] =  FMConfig::value(QStringLiteral("Docks/"+ds+"Pos"), "Left").toString();
    //		dockVisible[ds] = FMConfig::value(QStringLiteral("Docks/"+ds+"Visible"), true).toBool();
    //		dockGeometry[ds] = FMConfig::value(QStringLiteral("Docks/"+ds+"Geometry"), QRect()).toRect();
    //		qDebug()<<ds<< dockArea[ds] << dockVisible[ds] <<dockGeometry[ds];
    //	}

    panoseMatchTreshold = FMConfig::value(QStringLiteral("Panose/MatchTreshold"), 1000).toInt();

    previewInfoFontSize = FMConfig::value(QStringLiteral("Info/PreviewSize"), 20.0).toDouble();

    templatesDir = FMConfig::value(QStringLiteral("Places/TemplatesDir"), "./").toString();
    // the downloaded files of remote fonts: a cache, kept between sessions
    const QString remoteCache(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/remote"));
    m_remoteTmpDir = FMConfig::value(QStringLiteral("Places/RemoteTmpDir"), remoteCache).toString();

    defaultOTFScript = FMConfig::value(QStringLiteral("OTF/Script")).toString();
    defaultOTFLang = FMConfig::value(QStringLiteral("OTF/Lang")).toString();
    defaultOTFGPOS = FMConfig::value(QStringLiteral("OTF/GPOS")).toString().split(";", Qt::SkipEmptyParts);
    defaultOTFGSUB = FMConfig::value(QStringLiteral("OTF/GSUB")).toString().split(";", Qt::SkipEmptyParts);
    chartInfoFontSize = FMConfig::value(QStringLiteral("ChartInfoFontSize"), 8).toInt();
    chartInfoFontName = FMConfig::value(QStringLiteral("ChartInfoFontFamily"), QFont().family()).toString();

    databaseDriver = FMConfig::value(QStringLiteral("Database/Driver"), "QSQLITE").toString();
    databaseHostname = FMConfig::value(QStringLiteral("Database/Hostname"), "").toString();
    {
        QString sep(QDir::separator());
        QString newDefault = ownDir.absolutePath() + sep + "Data.sql";
#if !defined(PLATFORM_APPLE) && !defined(_WIN32)
        // If config still holds any known pre-migration default, clear it so
        // the database is found at the current location.
        QString xdgData = qEnvironmentVariable("XDG_DATA_HOME", QDir::homePath() + "/.local/share");
        QStringList oldDefaults;
        oldDefaults << QDir::homePath() + "/.Fontmatrix/Data.sql" << xdgData + "/Undertype/fontmatrix/Data.sql";
        if (oldDefaults.contains(FMConfig::value(QStringLiteral("Database/DbName")).toString()))
            FMConfig::remove(QStringLiteral("Database/DbName"));
#endif
        databaseDbName = FMConfig::value(QStringLiteral("Database/DbName"), newDefault).toString();
    }
    databaseUser = FMConfig::value(QStringLiteral("Database/User"), "").toString();
    databasePassword = FMConfig::value(QStringLiteral("Database/Password"), "").toString();
    if (!QSqlDatabase::drivers().contains(databaseDriver)) {
        qCDebug(FONTMATRIX_LOG) << "The SQL driver you request is not available(" << databaseDriver << ")";
    }
}

void typotek::writeSettings()
{
    // Window geometry is persisted by KMainWindow via setAutoSaveSettings().
    theMainView->saveSplitterState();

    //	QStringList dl;
    //	dl << "Main" << "Tags" << "Panose";
    //	for (const auto& ds : dl)
    //	{
    //		if(dockWidget[ds]->isFloating())
    //		{
    //			dockArea[ds] = "Float";
    //		}
    //		FMConfig::setValue(QStringLiteral("Docks/"+ds+"Pos"), dockArea[ds]);
    //		FMConfig::setValue(QStringLiteral("Docks/"+ds+"Visible"), dockWidget[ds]->isVisible());
    //		FMConfig::setValue(QStringLiteral("Docks/"+ds+"Geometry"), dockWidget[ds]->geometry());
    //	}

    FMConfig::setValue(QStringLiteral("Info/PreviewSize"), previewInfoFontSize);

    FMConfig::setValue(QStringLiteral("Preview/Word"), m_theWord);

    FMConfig::setValue(QStringLiteral("Panose/MatchTreshold"), panoseMatchTreshold);

    FMConfig::setValue(QStringLiteral("Database/Driver"), databaseDriver);
    FMConfig::setValue(QStringLiteral("Database/Hostname"), databaseHostname);
    FMConfig::setValue(QStringLiteral("Database/DbName"), databaseDbName);
    FMConfig::setValue(QStringLiteral("Database/User"), databaseUser);
    FMConfig::setValue(QStringLiteral("Database/Password"), databasePassword);

    if (theMainView->selectedFont())
        FMConfig::setValue(QStringLiteral("CurrentFont"), theMainView->selectedFont()->path());
}

bool typotek::maybeSave()
{
    return true;
}

typotek::~typotek() = default;

void typotek::fillTagsList()
{
}

// Move a file or directory from src to dst.
// Skips if src does not exist or dst already exists.
// Returns true on success or when there is nothing to do.
static bool migrateItem(const QString &src, const QString &dst)
{
    if (!QFileInfo::exists(src))
        return true;
    if (QFileInfo::exists(dst))
        return true;
    QDir().mkpath(QFileInfo(dst).absolutePath());
    if (QFileInfo(src).isDir()) {
#ifdef _WIN32
        // rename() does not move directories on Windows
        qWarning() << "fontmatrix: cannot auto-migrate directory" << src << "on Windows; please move it manually to" << dst;
        return false;
#else
        // rename(2) moves directories atomically on the same filesystem
        if (::rename(src.toLocal8Bit().constData(), dst.toLocal8Bit().constData()) == 0)
            return true;
        qWarning() << "fontmatrix: could not migrate" << src << "->" << dst << "(different filesystems? please move it manually)";
        return false;
#endif
    }
    return QFile::rename(src, dst);
}

void typotek::checkOwnDir()
{
    relayStartingStepIn(i18nc("@info:progress", "Check for Fontmatrix own dir"));
    QString sep(QDir::separator());

#ifdef PLATFORM_APPLE
    {
        // ~/Library/Application Support/fontmatrix  (QStandardPaths on macOS ignores org name)
        QString newDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString oldRootPath = QDir::homePath() + sep + "Library" + sep + "Fontmatrix";

        ownDir.setPath(newDataPath);
        configDir = ownDir; // macOS has no config/data split
        // ~/Library/Fonts is the standard per-user font directory on macOS
        managedDir.setPath(QDir::homePath() + sep + "Library" + sep + "Fonts");

        QDir().mkpath(newDataPath);
        if (!managedDir.exists())
            managedDir.mkpath(managedDir.absolutePath());

        // Migrate from old ~/Library/Fontmatrix/
        QDir oldDir(oldRootPath);
        if (oldDir.exists()) {
            migrateItem(oldRootPath + sep + "Data.sql", newDataPath + sep + "Data.sql");
            migrateItem(oldRootPath + sep + "Scripts", newDataPath + sep + "Scripts");
            migrateItem(oldRootPath + sep + "Samples", newDataPath + sep + "Samples");
            migrateItem(oldRootPath + sep + "Filters", newDataPath + sep + "Filters");
            migrateItem(oldRootPath + sep + "HelpBookmarks.xml", newDataPath + sep + "HelpBookmarks.xml");
            migrateItem(oldRootPath + sep + "HelpHistory.xml", newDataPath + sep + "HelpHistory.xml");
            migrateItem(oldRootPath + sep + "Resource.xml", newDataPath + sep + "Resource.xml");
            oldDir.rmdir(oldRootPath);
        }

        ResourceFile.setFileName(newDataPath + sep + "Resource.xml");
    }
#elif defined(_WIN32)
    {
        // %LOCALAPPDATA%\Fontmatrix\fontmatrix
        QString newDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QString oldRootPath = QDir::homePath() + sep + "fontmatrix";

        ownDir.setPath(newDataPath);
        configDir = ownDir;
        // On Windows 10/11 fonts can be installed per-user without admin privileges
        QString localAppData = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        managedDir.setPath(localAppData + sep + "Microsoft" + sep + "Windows" + sep + "Fonts");

        QDir().mkpath(newDataPath);
        if (!managedDir.exists())
            managedDir.mkpath(managedDir.absolutePath());

        // Migrate from old ~/fontmatrix/
        QDir oldDir(oldRootPath);
        if (oldDir.exists()) {
            migrateItem(oldRootPath + sep + "Data.sql", newDataPath + sep + "Data.sql");
            migrateItem(oldRootPath + sep + "HelpBookmarks.xml", newDataPath + sep + "HelpBookmarks.xml");
            migrateItem(oldRootPath + sep + "HelpHistory.xml", newDataPath + sep + "HelpHistory.xml");
            oldDir.rmdir(oldRootPath);
        }

        ResourceFile.setFileName(newDataPath + sep + "Resource.xml");
    }
#else
    // XDG Base Directory compliant paths
    QString newDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString newConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString oldRootPath = QDir::homePath() + sep + ".Fontmatrix";

    ownDir.setPath(newDataPath);
    configDir.setPath(newConfigPath);
    // where fontconfig, Flatpak and GNOME all look by themselves
    managedDir.setPath(FMPaths::UserFontsDir());

    QDir().mkpath(newDataPath);
    QDir().mkpath(newConfigPath);

    // Backward compatibility: migrate from old locations before creating
    // subdirectories, so migrateItem can detect non-existent destinations.

    // Order matters: process the most-recent intermediate location first so
    // that the newest data wins when both old sources exist simultaneously.

    // 2nd step: intermediate location from when org was still "Undertype"
    QString xdgData = qEnvironmentVariable("XDG_DATA_HOME", QDir::homePath() + "/.local/share");
    QString xdgConfig = qEnvironmentVariable("XDG_CONFIG_HOME", QDir::homePath() + "/.config");
    QString oldUndertypeData = xdgData + "/Undertype/fontmatrix";
    QString oldUndertypeConfig = xdgConfig + "/Undertype/fontmatrix";
    {
        QDir d(oldUndertypeData);
        if (d.exists()) {
            migrateItem(oldUndertypeData + sep + "Activated", newDataPath + sep + "Activated");
            migrateItem(oldUndertypeData + sep + "Scripts", newDataPath + sep + "Scripts");
            migrateItem(oldUndertypeData + sep + "Samples", newDataPath + sep + "Samples");
            migrateItem(oldUndertypeData + sep + "Data.sql", newDataPath + sep + "Data.sql");
            migrateItem(oldUndertypeData + sep + "HelpBookmarks.xml", newDataPath + sep + "HelpBookmarks.xml");
            migrateItem(oldUndertypeData + sep + "HelpHistory.xml", newDataPath + sep + "HelpHistory.xml");
            migrateItem(oldUndertypeData + sep + "Filters", newConfigPath + sep + "Filters");
            d.rmdir(oldUndertypeData);
        }
    }
    {
        QDir d(oldUndertypeConfig);
        if (d.exists()) {
            migrateItem(oldUndertypeConfig + sep + "Filters", newConfigPath + sep + "Filters");
            d.rmdir(oldUndertypeConfig);
        }
    }

    // 1st step: original ~/.Fontmatrix location
    QDir oldDir(oldRootPath);
    if (oldDir.exists()) {
        migrateItem(oldRootPath + sep + "Activated", newDataPath + sep + "Activated");
        migrateItem(oldRootPath + sep + "Scripts", newDataPath + sep + "Scripts");
        migrateItem(oldRootPath + sep + "Samples", newDataPath + sep + "Samples");
        migrateItem(oldRootPath + sep + "Data.sql", newDataPath + sep + "Data.sql");
        migrateItem(oldRootPath + sep + "HelpBookmarks.xml", newDataPath + sep + "HelpBookmarks.xml");
        migrateItem(oldRootPath + sep + "HelpHistory.xml", newDataPath + sep + "HelpHistory.xml");
        migrateItem(oldRootPath + sep + "Filters", newConfigPath + sep + "Filters");
        migrateItem(oldRootPath + sep + "Resource.xml", newConfigPath + sep + "Resource.xml");
        // Remove old root only if it is now empty
        oldDir.rmdir(oldRootPath);
    }

    if (!managedDir.exists())
        managedDir.mkpath(managedDir.absolutePath());

    // the links of the versions before: FMActivate::migrateActivated() moves them once the database is up
    if (QDir(newDataPath + sep + "Activated").exists())
        m_oldActivatedDir = newDataPath + sep + "Activated";

    ResourceFile.setFileName(newConfigPath + sep + "Resource.xml");
#endif
}

QStringList typotek::getSystemFontDirs()
{
    QStringList retList;
#ifdef HAVE_FONTCONFIG // For Unices (OSX excluded)
    QStringList tmpList;
    FcConfig *FcInitLoadConfig();
    FcStrList *sysDirList = FcConfigGetFontDirs(nullptr);
    QString sysDir((char *)FcStrListNext(sysDirList));
    while (!sysDir.isEmpty()) {
        tmpList << sysDir;
        sysDir = ((char *)FcStrListNext(sysDirList));
    }
    // Because we will go recursivly through these directories, we just want to list the top most ones.
    for (const auto &path : std::as_const(tmpList)) {
        // 		qDebug()<< "PATH"<<path;
        bool root(true);
        for (const auto &ref : std::as_const(tmpList)) {
            if (path != ref) {
                if (path.startsWith(ref)) {
                    root = false;
                    break;
                }
                // 				qDebug()<<"\tREF"<<ref<<root;
            }
        }
        if (root)
            retList << path;
    }

#endif // HAVE_FONTCONFIG
#ifdef PLATFORM_APPLE
    retList << "/Library/Fonts";
    retList << "/System/Library/Fonts";
#endif // PLATFORM_APPLE
#if defined(_WIN32)
    retList << getWin32SystemFontDir();
#endif // _WIN32

    qCDebug(FONTMATRIX_LOG) << retList.join("\n");
    return retList;
}

bool typotek::isSysFont(FontItem *f)
{
    if (f) {
        if (sysFontList.contains(f->path()))
            return true;
    }
    return false;
}

void typotek::initDir()
{
    /// let’s load system fonts
    constexpr int SYSTEM_FONTS = 1;
    if (SYSTEM_FONTS) {
        relayStartingStepIn(i18nc("@info:progress", "Loading System Fonts"));
        m_sysTagName = i18nc("@item:inlistbox name of the tag given to system fonts", "System Fonts");

        QList<FontItem *> sysFontPtrs;

        QStringList sysDir(getSystemFontDirs());

        QStringList yetHereFonts;
        QList<FontItem *> fontMap(FMFontDb::DB()->AllFonts());
        for (int i = 0; i < fontMap.count(); ++i)
            yetHereFonts << fontMap[i]->path();

        for (int sIdx(0); sIdx < sysDir.count(); ++sIdx) {
            QDir theDir(sysDir[sIdx]);
            QStringList syspathList;

            QStringList dirList(fontmatrix::exploreDirs(theDir, 0));
            QStringList filters;
            filters << "*.otf" << "*.pfb" << "*.ttf" << "*.ttc";
            for (const auto &dr : std::as_const(dirList)) {
                // our own copies live under a fontconfig directory
                if (isInUserFontFolder(dr))
                    continue;
                QDir d(dr);
                QFileInfoList fil = d.entryInfoList(filters);
                for (const auto &fp : std::as_const(fil)) {
                    if (!yetHereFonts.contains(fp.absoluteFilePath()))
                        syspathList << fp.absoluteFilePath();
                    sysFontList << fp.absoluteFilePath();
                }
            }

            int sysFontCount(syspathList.count());
            if (sysFontCount > 0) {
                relayStartingStepIn(i18nc("@info:progress", "Adding") + " " + QString::number(sysFontCount) + " "
                                    + i18nc("followed by a directory name", "fonts from") + " " + sysDir[sIdx]);
                qCDebug(FONTMATRIX_LOG) << (i18n("Adding") + " " + QString::number(sysFontCount) + " " + i18nc("followed by a directory name", "fonts from")
                                            + " " + sysDir[sIdx]);
                FMFontDb::DB()->TransactionBegin();
                for (int i = 0; i < sysFontCount; ++i) {
                    QFile ff(syspathList.at(i));
                    QFileInfo fi(syspathList.at(i));
                    {
                        FontItem *fitem = FMFontDb::DB()->Font(fi.absoluteFilePath(), false);
                        if (fitem) {
                            // 							qDebug()<<"\t"<<fitem <<fitem->path();
                            fitem->setActivated(true);
                            // 							fitem->addTag ( m_sysTagName );
                            sysFontPtrs << fitem;
                        } else {
                            qCWarning(FONTMATRIX_LOG) << "Cannot open this font because its broken: " << fi.fileName();
                        }
                    }
                }
                FMFontDb::DB()->TransactionEnd();
            }
        }

        if (sysFontPtrs.count() > 0)
            relayStartingStepIn(QString::number(sysFontPtrs.count()) + " " + i18nc("@info:progress", "system fonts added."));

        // So much complicated only because otherwise, tags were added twice with SQLite ???
        QStringList tl;
        for (auto *sfp : std::as_const(sysFontPtrs)) {
            tl << sfp->path();
        }
        FMFontDb::DB()->addTag(tl, m_sysTagName);
    }

#ifdef _WIN32
    // fonts removed in Settings > Fonts since the last run, and copies that were in use at deactivation
    relayStartingStepIn(i18nc("@info:progress", "Checking the activated fonts"));
    FMActivate::getInstance()->reconcileUserFonts();
#elif !defined(PLATFORM_APPLE)
    relayStartingStepIn(i18nc("@info:progress", "Checking the activated fonts"));
    if (!m_oldActivatedDir.isEmpty()) {
        FMActivate::getInstance()->migrateActivated(m_oldActivatedDir);
        m_oldActivatedDir.clear();
    }
    FMActivate::getInstance()->reconcileActivated();
#endif

    // 	qDebug()<<"TIME(fonts) : "<<fontsTime.elapsed();
    /// Remote directories: their catalogues are read again at every start
    fetchRemoteDirectories(FMConfig::value(QStringLiteral("RemoteDirectories"), QStringList()).toStringList());
}

QNetworkAccessManager *typotek::network()
{
    if (!m_network)
        m_network = new QNetworkAccessManager(this);
    return m_network;
}

void typotek::fetchRemoteDirectories(const QStringList &urls)
{
    if (urls.isEmpty())
        return;
    // one fetch at a time; a second request waits for the first to finish
    if (remoteDir) {
        const QStringList later(urls);
        connect(
            remoteDir,
            &RemoteDir::listIsReady,
            this,
            [this, later]() {
                fetchRemoteDirectories(later);
            },
            Qt::SingleShotConnection);
        return;
    }
    relayStartingStepIn(
        i18ncp("@info:progress", "Reading the catalogue of %1 remote directory", "Reading the catalogues of %1 remote directories", urls.count()));
    remoteDir = new RemoteDir(urls, this);
    connect(remoteDir, &RemoteDir::listIsReady, this, &typotek::slotRemoteIsReady);
    remoteDir->run();
}

void typotek::slotRemoteIsReady()
{
    if (!remoteDir)
        return;
    const QList<RemoteDir::FontInfo> listInfo(remoteDir->rFonts());
    remoteDir->deleteLater();
    remoteDir = nullptr;

    FMFontDb *db(FMFontDb::DB());
    int added(0);
    db->TransactionBegin();
    for (const RemoteDir::FontInfo &info : listInfo) {
        FontItem *fi = db->Knows(info.file) ? db->Font(info.file) : nullptr;
        if (!fi) {
            // a URL cannot be opened as a file: the item is built from the index
            fi = new FontItem(info.file, true);
            fi->fileRemote(info.family, info.variant, info.type, info.info, info.pix);
            fi = db->AddFont(fi);
            if (!fi)
                continue;
            ++added;
        } else if (fi->isRemote())
            fi->fileRemote(info.family, info.variant, info.type, info.info, info.pix);
        else
            continue;
        // the tags of the directory, and the directory itself as a tag
        QStringList tags(info.tags);
        const QString host(QUrl(info.file).host());
        if (!host.isEmpty() && !tags.contains(host))
            tags << host;
        for (const QString &tag : std::as_const(tags))
            db->addTag(info.file, tag);
    }
    db->TransactionEnd();
    db->announceTagsChanged();
    if (added > 0) {
        showStatusMessage(i18ncp("@info:status", "%1 font added from the network", "%1 fonts added from the network", added));
        Q_EMIT newFontsArrived();
    }
}

void typotek::fontBook()
{
    FontBook fontbook;
    fontbook.doBook(FontBook::Full);
}

FontItem *typotek::getSelectedFont()
{
    return theMainView->selectedFont();
}

void typotek::keyPressEvent(QKeyEvent *event)
{
    // 	qDebug() << "typotek::keyPressEvent(QKeyEvent * "<<event<<")";
    if (/*event->modifiers().testFlag(Qt::ControlModifier) &&*/ event->key() == Qt::Key_J)
        qCDebug(FONTMATRIX_LOG) << "NUM FACES OPENED:" << fm_num_face_opened;
}

void typotek::slotActivateCurrents()
{
    if (KMessageBox::warningContinueCancel(this,
                                           i18nc("@info", "You are about to activate a bunch of fonts,\nit is time to cancel if it was not your intent"),
                                           i18nc("@title:window", "Activate Fonts"),
                                           KGuiItem(i18nc("@action:button", "Activate")),
                                           KStandardGuiItem::cancel(),
                                           QString(),
                                           KMessageBox::Options(KMessageBox::Notify | KMessageBox::Dangerous))
        == KMessageBox::Continue)
        theMainView->slotActivateAll();
}

void typotek::slotDeactivateCurrents()
{
    if (KMessageBox::warningContinueCancel(this,
                                           i18nc("@info", "You are about to deactivate a bunch of fonts,\nit is time to cancel if it was not your intent"),
                                           i18nc("@title:window", "Deactivate Fonts"),
                                           KGuiItem(i18nc("@action:button", "Deactivate")),
                                           KStandardGuiItem::cancel(),
                                           QString(),
                                           KMessageBox::Options(KMessageBox::Notify | KMessageBox::Dangerous))
        == KMessageBox::Continue)
        theMainView->slotDesactivateAll();
}

void typotek::toggleShowMenuBar(bool showMessage)
{
    // Inspired by Kate. The hamburger menu in the toolbar is the user's escape
    // hatch back to the menu when the bar is hidden.
    if (m_paShowMenuBar->isChecked()) {
        menuBar()->show();
    } else {
        if (showMessage && toolBar()->isHidden()) {
            const QString accel = m_paShowMenuBar->shortcut().toString(QKeySequence::NativeText);
            KMessageBox::information(this,
                                     i18nc("@info", "This will hide the menu bar completely. You can show it again by typing %1.", accel),
                                     i18nc("@title:window", "Hide menu bar"));
        }
        menuBar()->hide();
    }
}

void typotek::slotEditFont()
{
    // a tile highlighted in the list counts as the font to edit, as it does elsewhere
    FontItem *item = fontForAction();
    if (!item)
        return;

    // a remote font is edited in its downloaded copy, like everything else that opens it
    const QString path(item->localPath());
    if (path.isEmpty()) {
        statusBar()->showMessage(i18nc("@info:status", "This font has not been downloaded yet"), 3000);
        return;
    }

    if (fontEditorIsDesktop()) {
        // the desktop is asked which application should open the file, and it is
        // given the right to write it, so that an editor can save what it changes
        if (!FMPortal::openWith(path, this))
            statusBar()->showMessage(i18nc("@info:status", "The desktop could not be asked to open this font"), 3000);
        return;
    }

    QStringList arguments;
    arguments << "-nosplash" << path;

    auto myProcess = new QProcess(this);
    myProcess->start(fonteditorPath, arguments);
}

void typotek::setupDrop()
{
    setAcceptDrops(true);
    // was pretty hard!
}

void typotek::dropEvent(QDropEvent *event)
{
    qCDebug(FONTMATRIX_LOG) << "typotek::dropEvent (" << event->mimeData()->text() << ")";
    // 	qDebug()<<"F: "<<event->mimeData()->formats().join(";");

    // 	event->acceptProposedAction();
    QStringList uris = event->mimeData()->text().split("\n");
    QStringList ret;

    for (int i = 0; i < uris.count(); ++i) {
        qCDebug(FONTMATRIX_LOG) << "dropped uri[" << i << "] -> " << uris[i];
        QUrl url(uris[i].trimmed());
        qCDebug(FONTMATRIX_LOG) << "\tURL -> " << url.toLocalFile();
        if (url.scheme() == "file") {
            if (url.toLocalFile().endsWith("ttf", Qt::CaseInsensitive)) {
                ret << url.toLocalFile();
            } else if (url.toLocalFile().endsWith("ttc", Qt::CaseInsensitive)) {
                ret << url.toLocalFile();
            } else if (url.toLocalFile().endsWith("otf", Qt::CaseInsensitive)) {
                ret << url.toLocalFile();
            } else if (url.toLocalFile().endsWith("pfb", Qt::CaseInsensitive)) {
                ret << url.toLocalFile();
            } else {
                qCDebug(FONTMATRIX_LOG) << url.toLocalFile() << "is not a supported font file";
            }
        } else if (url.scheme() == "http") {
            // TODO Get fonts over http
            qCDebug(FONTMATRIX_LOG) << "Support of DragNDrop over http is sheduled";
            statusBar()->showMessage(i18nc("@info:status", "Support of DragNDrop over http is sheduled but not yet effective"), 3000);
        } else {
            qCDebug(FONTMATRIX_LOG) << "Protocol not supported";
        }
    }

    // 	qDebug() << ret.join("||");
    if (ret.count())
        openList(ret);
}

void typotek::dragEnterEvent(QDragEnterEvent *event)
{
    qCDebug(FONTMATRIX_LOG) << event->mimeData()->formats().join("|");
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
        qCDebug(FONTMATRIX_LOG) << "dragEnterEvent accepted ";
    } else {
        qCDebug(FONTMATRIX_LOG) << "dragEnterEvent refused";
        statusBar()->showMessage(i18nc("@info:status", "You bring something over me I can’t handle"), 3000);
    }
}

void typotek::slotPrefsPanel(PrefsPanelDialog::PAGE page)
{
    PrefsPanelDialog pp(this);

    pp.initSystrayPrefs(true, systray->isVisible(), systray->hasActivateAll(), systray->allConfirmation(), systray->tagsConfirmation());
    pp.initSampleTextPrefs();
    pp.initFilesAndFolders();
    pp.initShortcuts();
    pp.showPage(page);
    pp.exec();
}

void typotek::slotPrefsPanelDefault()
{
    slotPrefsPanel(PrefsPanelDialog::PAGE_GENERAL);
}

void typotek::forwardUpdateView()
{
    //	theMainView->slotView ( true );
}

void typotek::setSystrayVisible(bool isVisible)
{
    systray->slotSetVisible(isVisible);
}

void typotek::showActivateAllSystray(bool isVisible)
{
    systray->slotSetActivateAll(isVisible);
}

void typotek::systrayAllConfirmation(bool isEnabled)
{
    systray->requireAllConfirmation(isEnabled);
}

void typotek::systrayTagsConfirmation(bool isEnabled)
{
    systray->requireTagsConfirmation(isEnabled);
}

void typotek::slotCloseToSystray(bool isEnabled)
{
    FMConfig::setValue(QStringLiteral("Systray/CloseToTray"), isEnabled);
    FMConfig::setValue(QStringLiteral("Systray/CloseNoteShown"), false);
}

void typotek::slotSystrayStart(bool isEnabled)
{
    FMConfig::setValue(QStringLiteral("Systray/StartToTray"), isEnabled);
}

QString typotek::namedSample(QString name)
{
    QString cn(name);
    if (cn == QString("NEW_SAMPLE"))
        return i18nc("@info default content of a new sample text", "Edit me!");
    if (cn.isEmpty())
        cn = currentNamedSample;
    else if (cn != currentNamedSample) {
        currentNamedSample = cn;
        updateHyphenation();
    }

    if (!dataLoader)
        dataLoader = new DataLoader();

    const QMap<QString, QString> &us(dataLoader->userSamples());
    for (const auto usKeys = us.keys(); const auto &k : usKeys) {
        QString id(QString("User::") + k);
        //		qDebug()<<"\t"<<id;
        if (id == cn) {
            return us[k];
        }
    }

    const QMap<QString, QMap<QString, QString>> &ss(dataLoader->systemSamples());
    for (const auto ssKeys = ss.keys(); const auto &pk : ssKeys) {
        for (const auto ssKeysList = ss[pk].keys(); const auto &sk : ssKeysList) {
            QString id(pk + QString("::") + sk);
            //			qDebug()<<"\t"<<id;
            if (id == cn) {
                return ss.value(pk).value(sk);
            }
        }
    }
    return namedSample(defaultSampleName());
}

QMap<QString, QList<QString>> typotek::namedSamplesNames()
{
    if (!dataLoader)
        dataLoader = new DataLoader();

    QMap<QString, QList<QString>> ret;
    const QMap<QString, QString> &us(dataLoader->userSamples());
    const QMap<QString, QMap<QString, QString>> &ss(dataLoader->systemSamples());
    for (const auto loopSsKeys = ss.keys(); const auto &key : loopSsKeys) {
        ret[key] << ss[key].keys();
    }
    ret[QString("User")] << us.keys();
    return ret;
}

void typotek::addNamedSample(QString name, QString sample)
{
    if (!dataLoader)
        dataLoader = new DataLoader();

    dataLoader->update(name, sample);
    //	theMainView->refillSampleList();
}

void typotek::removeNamedSample(const QString &key)
{
    if (!dataLoader)
        dataLoader = new DataLoader();

    dataLoader->remove(key);
    //	theMainView->refillSampleList();
}

void typotek::changeSample(QString name, QString text)
{
    if (!dataLoader)
        dataLoader = new DataLoader();

    dataLoader->update(name, text);
    //	theMainView->refillSampleList();
}

QString typotek::defaultSampleName()
{
    if (!dataLoader)
        dataLoader = new DataLoader();

    const QMap<QString, QString> &us(dataLoader->userSamples());
    if (us.contains(i18n("default")))
        return QString("User::") + QString("default");
    else if (us.count() > 0)
        return QString("User::") + us.firstKey();
    else {
        const QMap<QString, QMap<QString, QString>> &ss(dataLoader->systemSamples());
        // DataLoader names the groups with QLocale::languageToString(), "German"
        // and not "de"; a directory Qt has no language for keeps its own name,
        // so a C locale matches no group.
        const auto groupOf = [](const QLocale &loc) {
            return QLocale::languageToString(loc.language());
        };
        const QLocale sysLocale(QLocale::system());
        QString l(groupOf(sysLocale));
        if ((sysLocale.language() != QLocale::C) && (ss.contains(l)) && (ss[l].count() > 0))
            return l + QString("::") + ss.value(l).firstKey();
        else {
            // Prefer Latin-script samples as fallback so the widget shows something on first use
            static const QStringList fallbackCodes{"de", "fr", "ru"};
            for (const QString &code : fallbackCodes) {
                const QString preferred(groupOf(QLocale(code)));
                if (ss.contains(preferred) && ss[preferred].count() > 0)
                    return preferred + QString("::") + ss.value(preferred).firstKey();
            }
            for (const auto sampleGroups = ss.keys(); const auto &k : sampleGroups) {
                if (ss[k].count() > 0)
                    return k + QString("::") + ss.value(k).firstKey();
            }
        }
    }
    return QString();
}

void typotek::setWord(QString s, bool)
{
    if (s == m_theWord)
        return;
    m_theWord = s;
    QList<FontItem *> fontMap(FMFontDb::DB()->AllFonts());
    for (int i(0); i < fontMap.count(); ++i)
        fontMap[i]->clearPreview();

    Q_EMIT previewHasChanged();
}

void typotek::setPreviewSize(double d)
{
    if (previewSize == d)
        return;

    //	previewSize = d;
    QList<FontItem *> fontMap(FMFontDb::DB()->AllFonts());
    for (int i(0); i < fontMap.count(); ++i)
        fontMap[i]->clearPreview();
    Q_EMIT previewHasChanged();
}

void typotek::setPreviewRTL(bool d)
{
    if (previewRTL == d)
        return;
    previewRTL = d;
    QList<FontItem *> fontMap(FMFontDb::DB()->AllFonts());
    for (int i(0); i < fontMap.count(); ++i)
        fontMap[i]->clearPreview();
    Q_EMIT previewHasChanged();
}

void typotek::setPreviewSubtitled(bool d)
{
    if (previewSubtitled == d)
        return;
    previewSubtitled = d;
    QList<FontItem *> fontMap(FMFontDb::DB()->AllFonts());
    for (int i(0); i < fontMap.count(); ++i)
        fontMap[i]->clearPreview();
    Q_EMIT previewHasChanged();
}

void typotek::setFontEditorPath(const QString &path)
{
    fonteditorPath = path;
    updateFontEditorAction();
    FMConfig::setValue(QStringLiteral("FontEditor"), fonteditorPath);
}

bool typotek::fontEditorIsDesktop() const
{
    // in a sandbox no program of the system is ours to run, and without an editor
    // of its own the desktop still knows what can open a font
    return (KSandbox::isInside() || !QFile::exists(fonteditorPath)) && FMPortal::isAvailable();
}

void typotek::updateFontEditorAction()
{
    if (!fonteditorAct)
        return;
    if (fontEditorIsDesktop()) {
        fonteditorAct->setEnabled(true);
        fonteditorAct->setStatusTip(i18nc("@info:status", "Open the selected font in the application the desktop asks you to choose"));
    } else if (QFile::exists(fonteditorPath)) {
        fonteditorAct->setEnabled(true);
        fonteditorAct->setStatusTip(i18nc("@info:status", "Edit currently selected font in a font editor of your choice"));
    } else {
        fonteditorAct->setEnabled(false);
        fonteditorAct->setStatusTip(
            i18nc("@info:status", "You don't seem to have a font editor installed. Path to font editor can be set in Preferences dialog."));
    }
}

void typotek::slotUseInitialTags(bool isEnabled)
{
    useInitialTags = isEnabled;
    FMConfig::setValue(QStringLiteral("UseInitialTags"), isEnabled);
}

void typotek::setImportedFontsHidden(bool hidden) // the "do not show this again" box of the dialog
{
    showFontListDialog = !hidden;
    FMConfig::setValue(QStringLiteral("ShowImportedFonts"), showFontListDialog);
}

bool typotek::showImportedFonts()
{
    return showFontListDialog;
}

void typotek::setTemplatesDir(const QString &dir)
{
    templatesDir = dir;
    FMConfig::setValue(QStringLiteral("Places/TemplatesDir"), templatesDir);
}

void typotek::changeFontSizeSettings(double fSize, double lSize)
{
    FMConfig::setValue(QStringLiteral("Sample/FontSize"), fSize);
    FMConfig::setValue(QStringLiteral("Sample/Interline"), lSize);
    //	theMainView->reSize(fSize,lSize);
}

void typotek::relayStartingStepIn(QString s)
{
    int i(Qt::AlignRight | Qt::AlignBottom);
    QColor c(Qt::white);
    Q_EMIT relayStartingStepOut(s, i, c);
}

void typotek::removeFontItem(QString key)
{
    // 	FontItem *fit = realFontMap.value(key);
    // 	if(!fit)
    // 		return;
    // 	fontMap.removeAll(fit);
    // 	delete fit;
    // 	realFontMap.remove(key);
    // 	qDebug()<< key << "has been removed";
    FMFontDb::DB()->Remove(key);
}

void typotek::removeFontItem(QStringList keyList)
{
    for (const auto &key : keyList) {
        removeFontItem(key);
    }
}

void typotek::showStatusMessage(const QString &message)
{
    statusBar()->showMessage(message, 3000);
}

void typotek::setRemoteTmpDir(const QString &s)
{
    if (s.isEmpty())
        m_remoteTmpDir = QDir::temp().path();
    else
        m_remoteTmpDir = s;

    FMConfig::setValue(QStringLiteral("Places/RemoteTmpDir"), m_remoteTmpDir);
}

void typotek::slotRepair()
{
    FmRepair repair(this);
    repair.exec();
}

void typotek::slotTagAll()
{
    QStringList tagsList(FMFontDb::DB()->getTags());
    ImportTags imp(this, tagsList);
    imp.exec();
    QStringList tali = imp.tags();

    if (tali.isEmpty())
        return;
    for (int t(0); t < tali.count(); ++t) {
        if (!tagsList.contains(tali[t])) {
            tagsList.append(tali[t]);
        }
    }

    FMFontDb::DB()->TransactionBegin();
    QList<FontItem *> curfonts = theMainView->curFonts();
    for (int i(0); i < curfonts.count(); ++i) {
        for (int t(0); t < tali.count(); ++t) {
            curfonts[i]->addTag(tali[t]);
        }
    }
    FMFontDb::DB()->TransactionEnd();
    //	TagsWidget::getInstance()->newTag();
}

// void typotek::printFamily()
//{
//	FontItem * font(theMainView->selectedFont());
//	if(!font)
//		return;
//	QPrinter thePrinter ( QPrinter::HighResolution );
//	QPrintDialog dialog(&thePrinter, this);
//	dialog.setWindowTitle("Fontmatrix - " + i18n("Print Family") +" - " + font->family());

//	if ( dialog.exec() != QDialog::Accepted )
//		return;

//	thePrinter.setFullPage ( true );
//	QPainter aPainter ( &thePrinter );

//	QGraphicsScene tmpScene(thePrinter.paperRect());
//	QGraphicsScene pScene(thePrinter.paperRect());

//	qDebug()<<thePrinter.paperRect();

//	QMap<int , double> logWidth;
//	QMap<int , double> logAscend;
//	QMap<int , double> logDescend;
//	QMap<int , QString> sampleString;
//	QMap<int , FontItem*> sampleFont;

//	QStringList stl1(namedSample ( theMainView->sampleName() ).split ( QRegExp("\\W") ));
//	if(stl1.count() < 10 )
//	{
//		QMessageBox::information(this,"Fontmatrix",i18n("Not enough text to make a sample"));
//		return;
//	}
//	QStringList stl;
//	int idxS(0);
//	int idxE( qrand() % 9 );
//	while((idxS + idxE) < stl1.count())
//	{
//		QString t(QStringList(stl1.mid(idxS,idxE)).join( " " ));
//		qDebug()<<"s e T"<<idxS<<idxE<<t;
//		if(!t.isEmpty())
//			stl << t;

//		idxS += idxE;
//		idxE = qrand() % 9;
//	}
//	QList<FontItem*> familyFonts(FMFontDb::DB()->Fonts(theMainView->selectedFont()->family(), FMFontDb::Family ));

//// 	if(familyFonts.count() > stl.count())
//	{
//		int diff ( familyFonts.count()  );
//		for (int i(0); i < diff; ++i)
//		{
//			sampleString[i] = stl[ i % stl.count() ];
//		}
//	}

//	// first we’ll get widths for font size 1000
//	for(int fidx(0); fidx < familyFonts.count(); ++fidx)
//	{
//		sampleFont[fidx] = familyFonts[fidx];
//		bool rasterState(sampleFont[fidx]->rasterFreetype());
//		sampleFont[fidx]->setFTRaster(false);
//		sampleFont[fidx]->setRenderReturnWidth(true);
//		logWidth[fidx] =  familyFonts[fidx]->renderLine(&tmpScene, sampleString[fidx], QPointF(0.0, 1000.0) , 999999.0, 1000.0, 1) ;
//		sampleFont[fidx]->setRenderReturnWidth(false);
//		sampleFont[fidx]->setFTRaster(rasterState);
//		logAscend[fidx] = 1000.0 - tmpScene.itemsBoundingRect().top();
//		logDescend[fidx] = tmpScene.itemsBoundingRect().bottom() - 1000.0;
//		qDebug()<< sampleString[fidx] << logWidth[fidx];
//		QList<QGraphicsItem*> lgit(tmpScene.items());
//		for (auto* git : lgit)
//		{
//			tmpScene.removeItem(git);
//			delete git;
//		}
//	}
//	double defWidth(0.8 * pScene.width() );
//	double defHeight(0.9 * pScene.height() );
//	double xOff( 0.1 * pScene.width() );
//	double yPos(0.1 * pScene.height() );

//	QFont nameFont;
//	nameFont.setPointSizeF(100.0);
//	nameFont.setItalic(true);

//	for(int fidx(0); fidx < familyFonts.count(); ++fidx)
//	{
//		double scaleFactor(1000.0 / logWidth[fidx] );
//		double fSize( defWidth *  scaleFactor );
//		double fAscend(logAscend[fidx] * fSize / 1000.0);
//		double fDescend(logDescend[fidx] * fSize  / 1000.0 );
//		if( yPos + fAscend + fDescend > defHeight)
//		{
//			pScene.render(&aPainter);
//			thePrinter.newPage();
//			QList<QGraphicsItem*> lgit(pScene.items());
//			for (auto* git : lgit)
//			{
//				pScene.removeItem(git);
//				delete git;
//			}
//			yPos = 0.1 * pScene.height();

//		}

//		yPos +=  fAscend;
//		QPointF origine(xOff,  yPos );

//		qDebug()<< sampleString[fidx] << fSize;

//		bool rasterState(sampleFont[fidx]->rasterFreetype());
//		sampleFont[fidx]->setFTRaster(false);
//		sampleFont[fidx]->renderLine(&pScene, sampleString[fidx], origine, pScene.width(), fSize, 100);
//		pScene.addLine(QLineF(origine, QPointF(xOff + defWidth, yPos)));
//		sampleFont[fidx]->setFTRaster(rasterState);

//		yPos +=  fDescend ;

//		QGraphicsSimpleTextItem * nameText = pScene.addSimpleText( familyFonts[fidx]->fancyName(), nameFont) ;
//		nameText->setPos(xOff, yPos);
//// 		nameText->setBrush(Qt::gray);
//		yPos += nameText->boundingRect().height();
//	}

//	pScene.render(&aPainter);
//}

void typotek::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
}

void typotek::slotDockAreaChanged(Qt::DockWidgetArea area)
{
    auto dw(reinterpret_cast<QDockWidget *>(sender()));

    if (dw) {
        QString wId(dw->objectName());
        if (area == Qt::LeftDockWidgetArea)
            dockArea[wId] = "Left";
        else if (area == Qt::RightDockWidgetArea)
            dockArea[wId] = "Right";
        else if (area == Qt::TopDockWidgetArea)
            dockArea[wId] = "Top";
        else if (area == Qt::BottomDockWidgetArea)
            dockArea[wId] = "Bottom";
    }
}

FMHyphenator *typotek::getHyphenator() const
{
    return hyphenator;
}

// void typotek::slotSwitchLayOptVisible()
//{
//	if(FMLayout::getLayout()->optionDialog->isVisible())
//		FMLayout::getLayout()->optionDialog->setVisible(false);
//	else
//		FMLayout::getLayout()->optionDialog->setVisible(true);
//	slotUpdateLayOptStatus();
// }

// void typotek::slotUpdateLayOptStatus()
//{
//	if(FMLayout::getLayout()->optionDialog->isVisible())
//		layOptAct->setChecked(true);
//	else
//		layOptAct->setChecked(false);
// }

void typotek::slotExportXeTeX()
{
    const QList<FontItem *> fonts = FMFontDb::DB()->getFilteredFonts();
    if (fonts.isEmpty())
        return;

    QString path = QFileDialog::getSaveFileName(this,
                                                i18nc("@title:window", "Export font list as XeTeX"),
                                                QDir::homePath(),
                                                i18nc("@item:inlistbox file type filter", "TeX files (*.tex)"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "\\hsize=360pt\n\\vsize=580pt\n";
    out << "\\baselineskip=14.5pt\n\\parindent=8pt\n";
    out << "\\frenchspacing\n\\overfullrule=6pt\n\n";
    for (FontItem *fi : fonts) {
        out << "\\font\\lafonte=\"[" << fi->path() << "]\" at 11pt\n";
        out << "\\lafonte\n";
        out << fi->family() << " " << fi->variant() << "\n\n";
    }
    out << "\\end\n";
}

QString typotek::getDefaultOTFScript() const
{
    return defaultOTFScript;
}

void typotek::setDefaultOTFScript(const QString &theValue)
{
    if (theValue != defaultOTFScript) {
        FMConfig::setValue(QStringLiteral("OTF/Script"), theValue);
    }
    defaultOTFScript = theValue;
}

QString typotek::getDefaultOTFLang() const
{
    return defaultOTFLang;
}

void typotek::setDefaultOTFLang(const QString &theValue)
{
    if (theValue != defaultOTFLang) {
        FMConfig::setValue(QStringLiteral("OTF/Lang"), theValue);
    }
    defaultOTFLang = theValue;
}

QStringList typotek::getDefaultOTFGPOS() const
{
    return defaultOTFGPOS;
}

void typotek::setDefaultOTFGPOS(const QStringList &theValue)
{
    if (theValue != defaultOTFGPOS) {
        FMConfig::setValue(QStringLiteral("OTF/GPOS"), theValue.join(";"));
    }
    defaultOTFGPOS = theValue;
}

QStringList typotek::getDefaultOTFGSUB() const
{
    return defaultOTFGSUB;
}

void typotek::setDefaultOTFGSUB(const QStringList &theValue)
{
    if (theValue != defaultOTFGSUB) {
        FMConfig::setValue(QStringLiteral("OTF/GSUB"), theValue.join(";"));
    }
    defaultOTFGSUB = theValue;
}

void typotek::startProgressJob(int max)
{
    statusProgressBar->reset();
    statusProgressBar->setRange(0, max);
    statusProgressBar->show();
}

void typotek::runProgressJob(int i)
{
    if (i) {
        statusProgressBar->setValue(i);
    } else {
        int v(statusProgressBar->value());
        statusProgressBar->setValue(++v);
    }
}

void typotek::endProgressJob()
{
    statusProgressBar->hide();
    statusProgressBar->reset();
}

FontItem *typotek::fontForAction()
{
    FontItem *item(theMainView->selectedOrCurrentFont());
    if (!item)
        statusBar()->showMessage(i18nc("@info:status", "There is no font selected"), 3000);
    return item;
}

void typotek::slotShowTTTables()
{
    FontItem *font(fontForAction());
    if (font) {
        QDialog dia(this);
        QGridLayout glayout(&dia);
        TTTableView tv(font, &dia);
        QPushButton pbutton(i18nc("@action:button", "Close"), &dia);
        glayout.addWidget(&tv, 0, 0, 3, 3);
        glayout.addWidget(&pbutton, 3, 2);
        pbutton.setDefault(true);

        QRect drect(dia.rect());
        drect.setX(this->geometry().x() + 32);
        drect.setY(this->geometry().y() + 32);
        drect.setWidth(tv.rect().width() * 1.2);
        drect.setHeight(tv.rect().height() * 1.2);
        dia.setGeometry(drect);
        connect(&pbutton, &QPushButton::released, &dia, &QDialog::close);
        dia.exec();
    }
}

void typotek::slotEditPanose()
{
    FontItem *font(fontForAction());
    if (font) {
        FMPanoseDialog dia(font, this);
        dia.exec();
        if (dia.getOk() && (dia.getSourcePanose() != dia.getTargetPanose())) {
            // 			qDebug()<< "Update Panose"<<font->path();
            FMFontDb::DB()->setValue(font->path(), FMFontDb::Panose, dia.getTargetPanose());
            //			theMainView->slotInfoFont();
        }
    }
}

void typotek::slotDumpInfo()
{
    FontItem *font(fontForAction());
    if (font) {
        FMDumpDialog dia(font, this);
        if (dia.exec() != QDialog::Accepted) {
            qCDebug(FONTMATRIX_LOG) << "Dump not saved";
        }
    }
}

void typotek::slotReloadFiltered()
{
    FontItem *cf(theMainView->selectedFont());
    QString cfName;
    if (cf)
        cfName = cf->path();

    QStringList toReload;
    QApplication::changeOverrideCursor(Qt::WaitCursor);
    QMap<QString, QStringList> tagsRec;
    FMFontDb *db(FMFontDb::DB());
    for (const auto curFontsList = theMainView->curFonts(); auto *f : curFontsList) {
        toReload << f->path();
        tagsRec[f->path()] = f->tags();
        db->Remove(f->path());
    }
    QList<FontItem *> renewedFonts;
    for (const auto &p : std::as_const(toReload)) {
        FontItem *it(db->Font(p, true));
        if (it) {
            renewedFonts << it;
        }
    }
    db->TransactionBegin();
    for (auto *it : std::as_const(renewedFonts)) {
        it->setTags(tagsRec[it->path()]);
    }
    db->TransactionEnd();

    if (toReload.count() > renewedFonts.count()) {
        //! Number of fonts we failed to reload
        showStatusMessage(i18ncp("@info:status", "Failed to reload %1 font", "Failed to reload %1 fonts", toReload.count() - renewedFonts.count()));
        //		theMainView->slotReloadFontList();
    }
    if (!cfName.isEmpty()) {
        //			theMainView->forceReloadSelection();
        theMainView->slotFontSelectedByName(cfName);
    }

    QApplication::restoreOverrideCursor();
}

void typotek::slotReloadSingle()
{
    FontItem *cf(fontForAction());
    if (cf) {
        QString curName(cf->path());
        QStringList t(cf->tags());
        FMFontDb::DB()->Remove(curName);
        cf = FMFontDb::DB()->Font(curName, true);
        if (cf) {
            cf->setTags(t);
            //			theMainView->forceReloadSelection();
            theMainView->slotFontSelectedByName(curName);
        }
    }
}

void typotek::slotExtractFont()
{
    FMFontExtract ex(this);
    ex.exec();
}

void typotek::slotShowDuplicates()
{
    // one window, not modal: "Show in List" is for looking at the main list beside it
    if (!m_duplicates)
        m_duplicates = new FMDuplicatesDialog(this);
    m_duplicates->show();
    m_duplicates->raise();
    m_duplicates->activateWindow();
}

void typotek::slotMatchRaster()
{
    FMMatchRaster mr(this);
    mr.exec();
}

void typotek::showToltalFilteredFonts()
{
    countFilteredFonts->setText(
        i18ncp("number of filtererd fonts showed in status bar", "Filtered Font: %1", "Filtered Fonts: %1", FMFontDb::DB()->countFilteredFonts()));
}

void typotek::presentFontName(QString s)
{
    curFontPresentation->setText(QString("%1 : <b>%2</b>").arg(i18nc("followed by currently selected font name (in status bar)", "Current Font"), s));
}

int typotek::getPanoseMatchTreshold() const
{
    return panoseMatchTreshold;
}

void typotek::setPanoseMatchTreshold(int theValue)
{
    panoseMatchTreshold = theValue;
}

void typotek::slotHelpContents()
{
    // The handbook is a DocBook document shown by KDE Help Center where there is
    // one. A sandbox never has it — and could not show it either, since nothing
    // of the application is visible to the host — so there the same handbook,
    // installed as HTML, is read in a window of our own.
    if (!KSandbox::isFlatpak() && !QStandardPaths::findExecutable(QStringLiteral("khelpcenter")).isEmpty()) {
        KHelpClient::invokeHelp();
        return;
    }
    if (!FMPaths::HandbookFile().isEmpty()) {
        FMHandbookWindow::showHandbook(this);
        return;
    }
    KMessageBox::information(this,
                             xi18nc("@info",
                                    "The Fontmatrix handbook could not be opened: neither <application>KDE Help Center</application> nor a copy to read in a "
                                    "browser is installed.<nl/>The project page is at <link>%1</link>.",
                                    KAboutData::applicationData().homepage()),
                             i18nc("@title:window", "Handbook Not Available"));
}

void typotek::setVisible(bool visible)
{
    // Qt also comes here for a window that is already in the requested
    // state; the docks and floating windows only follow a real change.
    if (m_closing || visible == isVisible()) {
        KXmlGuiWindow::setVisible(visible);
        return;
    }

    if (!visible) {
        for (auto it(dockWidget.constBegin()); it != dockWidget.constEnd(); ++it) {
            dockVisible[it.key()] = it.value()->isVisible();
            it.value()->hide();
        }
        visibleFloatingWidgets.clear();
        for (const auto allWidgetsList = FloatingWidgetsRegister::AllWidgets(); auto *f : allWidgetsList) {
            visibleFloatingWidgets[f] = f->isVisible();
            f->setVisible(false);
        }

        // Neither is a FloatingWidget, the loop above does not see them.
        // Their actions in the View menu follow through visibilityChanged().
        playVisible = PlayWidget::getInstance()->isVisible();
        PlayWidget::getInstance()->hide();
        compareVisible = FontCompareWidget::getInstance()->isVisible();
        FontCompareWidget::getInstance()->hide();
    } else {
        for (auto it(dockWidget.constBegin()); it != dockWidget.constEnd(); ++it) {
            // a dock that was never recorded keeps the state it has
            if (dockVisible.contains(it.key()))
                it.value()->setVisible(dockVisible.value(it.key()));
        }
        for (auto it(visibleFloatingWidgets.constBegin()); it != visibleFloatingWidgets.constEnd(); ++it) {
            it.key()->setVisible(it.value());
        }

        PlayWidget::getInstance()->setVisible(playVisible);
        FontCompareWidget::getInstance()->setVisible(compareVisible);
    }

    KXmlGuiWindow::setVisible(visible);
}

QString typotek::word(FontItem *item, const QString &alt)
{
    if (item) {
        QString word(m_theWord);
        if (word.isEmpty() && !alt.isEmpty())
            word = alt;
        word.replace("<name>", item->fancyName());
        word.replace("<family>", item->family());
        word.replace("<variant>", item->variant());
        return word;
    }

    return m_theWord;
}

void typotek::updateFloatingStatus()
{
    playAction->setChecked(PlayWidget::getInstance()->isVisible());
    compareAction->setChecked(FontCompareWidget::getInstance()->isVisible());

    viewMenu->removeAction(closeAllFloat);
    viewMenu->removeAction(showAllFloat);
    viewMenu->removeAction(hideAllFloat);
    viewMenu->removeAction(floatSep);

    QList<FloatingWidget *> fwl(FloatingWidgetsRegister::AllWidgets());
    for (const auto floatingWidgetsKeys = floatingWidgets.keys(); auto *f : floatingWidgetsKeys) {
        if (!fwl.contains(f)) {
            viewMenu->removeAction(floatingWidgets.value(f));
            floatingWidgets.remove(f);
        }
    }

    for (auto *f : std::as_const(fwl)) {
        if (floatingWidgets.contains(f)) {
            floatingWidgets.value(f)->setChecked(f->isVisible());
        } else {
            auto wa(new QAction(f->getActionName(), this));
            wa->setCheckable(true);
            connect(f, &FloatingWidget::visibilityChange, this, &typotek::updateFloatingStatus);
            connect(wa, &QAction::triggered, f, &FloatingWidget::activate);
            floatingWidgets.insert(f, wa);
            floatingWidgets.value(f)->setChecked(f->isVisible());
            viewMenu->addAction(wa);
        }
    }
    if (floatingWidgets.count() > 0) {
        viewMenu->addAction(floatSep);
        viewMenu->addAction(closeAllFloat);
    }
    if (floatingWidgets.count() > 1) {
        viewMenu->addAction(showAllFloat);
        viewMenu->addAction(hideAllFloat);
    }
}

void typotek::closeAllFloatings()
{
    for (const auto floatingWidgetsKeysList = floatingWidgets.keys(); auto *f : floatingWidgetsKeysList) {
        f->close();
    }
}

void typotek::showAllFloatings()
{
    for (const auto loopFloatingWidgetsKeys = floatingWidgets.keys(); auto *f : loopFloatingWidgetsKeys) {
        f->setVisible(true);
    }
}

void typotek::hideAllFloatings()
{
    for (const auto floatingList = floatingWidgets.keys(); auto *f : floatingList) {
        f->setVisible(false);
    }
}

void typotek::toggleMainView(bool v)
{
    if (v)
        mainStack->setCurrentWidget(theBrowser);
    else
        mainStack->setCurrentWidget(theMainView);
}

void typotek::pushObject(QObject *o)
{
    o->moveToThread(sender()->thread());
}

#include "moc_typotek.cpp"

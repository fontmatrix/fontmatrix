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



#include "typotek.h"
#include "mainviewwidget.h"
#include "fontitem.h"
#include "hyphenate/fmhyphenator.h"
// #include "typotekadaptator.h"
#include "fmlayout.h"
#include "dataloader.h"
// #include "tagseteditor.h"
#include "savedata.h"
#include "aboutwidget.h"
#include "helpwidget.h"
#include "importedfontsdialog.h"
#include "listdockwidget.h"
#include "shortcuts.h"
#include "systray.h"
#include "prefspaneldialog.h"
#include "fontbook.h"
#include "importtags.h"
#include "dataexport.h"
#include "remotedir.h"
#include "tttableview.h"
#include "fmrepair.h"
// #include "fmprintdialog.h"
#include "fmactivate.h"
#include "fmlayout.h"
#include "fmfontdb.h"
#include "tagswidget.h"

#include "winutils.h"


#include <QtGui>
#include <QTextEdit>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDir>
// #include <QDBusConnection>
#include <QProgressDialog>
#include <QProgressBar>
#include <QDomDocument>
#include <QProcess>
#include <QDockWidget>
// #include <QMutex>
// #include <QWaitCondition>

#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif


typotek* typotek::instance = 0;
bool typotek::matrix = false;
QString typotek::fonteditorPath = "/usr/bin/fontforge";
extern bool __FM_SHOW_FONTLOADED;
extern int fm_num_face_opened;


/// LazyInit *********************************************
void LazyInit::run()
{
	/// We keep this for further needs
	emit endOfRun();
}
///******************************************************

/// a bit of globalness *******************************************
namespace fontmatrix
{
	QStringList exploreDirs ( const QDir &dir, int deep )
	{
		static QStringList retDirList;
		if ( deep > 10 )
			return QStringList();
		if ( deep == 0 )
			retDirList.clear();
		retDirList << dir.absolutePath();
		QStringList localEntries ( dir.entryList ( QDir::AllDirs | QDir::NoDotAndDotDot ) );
		foreach ( QString dirEntry, localEntries )
		{
// 			qDebug() << "[exploreDirs] - " + dir.absolutePath() + "/" + dirEntry;
			QDir d ( dir.absolutePath() + "/" + dirEntry );
			exploreDirs ( d, deep + 1 );
			if ( !retDirList.contains ( d.absolutePath() ) )
				retDirList << d.absolutePath();
		}

		return retDirList;
	}


	QMap<QString , Qt::DockWidgetArea> DockPosition;

	void fillDockPos()
	{
		DockPosition["Float"] = Qt::LeftDockWidgetArea;
		DockPosition["Left"] = Qt::LeftDockWidgetArea;
		DockPosition["Right"]= Qt::RightDockWidgetArea;
		DockPosition["Top"]= Qt::TopDockWidgetArea;
		DockPosition["Bottom"]= Qt::BottomDockWidgetArea;
	}
}

/// *****************************************************

typotek * typotek::getInstance()
{
	if(!instance)
	{
		instance = new typotek;
		Q_ASSERT(instance);
	}
	return instance;
}

typotek::typotek()
{
	setWindowTitle ( "Fontmatrix" );
	setupDrop();
	theMainView = 0;
	hyphenator = 0;
	theHelp = 0;
}

void typotek::initMatrix()
{
	if(matrix)
		return;
	matrix = true;
	m_defaultSampleName = tr("default") ;
	fontmatrix::fillDockPos();

	checkOwnDir();
	readSettings();
	initDir();

	theMainView = new MainViewWidget ( this );
	setCentralWidget ( theMainView );

	if ( QSystemTrayIcon::isSystemTrayAvailable() )
		systray = new Systray();
	else
		systray = 0;

	mainDock = new QDockWidget ( tr ( "Browse Fonts" ) );
	mainDock->setWidget ( ListDockWidget::getInstance() );
	addDockWidget ( fontmatrix::DockPosition[mainDockArea], mainDock );
	if(mainDockArea == QString("Float"))
		mainDock->setFloating(true);
	if(!mainDockGeometry.isNull())
		mainDock->setGeometry(mainDockGeometry);
	
	tagsDock = new QDockWidget ( tr("Tags") );
	tagsDock->setWidget( TagsWidget::getInstance() );
	addDockWidget(fontmatrix::DockPosition[tagsDockArea], tagsDock);
	if(tagsDockArea == QString("Float"))
		tagsDock->setFloating(true);
	if(!tagsDockGeometry.isNull())
		tagsDock->setGeometry(tagsDockGeometry);


	createActions();
	createMenus();
	createStatusBar();
	doConnect();

	theMainView->resetCrumb();

	if(!hyphenator)
	{
		QSettings st;
		QString dP( st.value("Sample/HyphenationDict", "hyph.dic").toString() );
		if(QFileInfo(dP).exists())
		{
			hyphenator = new FMHyphenator();
			if (!hyphenator->loadDict(dP, st.value("Sample/HyphLeft", 2).toInt(), st.value("Sample/HyphRight", 3).toInt())) {
				st.setValue("Sample/HyphenationDict", "");
				st.setValue("Sample/HyphLeft", 2);
				st.setValue("Sample/HyphRight", 3);
			}
		}
		else
		{
			hyphenator = new FMHyphenator(); // init the hyphenator anyway for the prefs
			st.setValue("Sample/HyphenationDict", "");
			st.setValue("Sample/HyphLeft", 2);
			st.setValue("Sample/HyphRight", 3);
			qDebug()<<"Err H"<<dP;
		}
	}
}

void typotek::doConnect()
{
	if(getSystray())
		connect ( FMActivate::getInstance() ,SIGNAL ( activationEvent ( QString ) ), getSystray(),SLOT ( updateTagMenu ( QString ) ) );

	connect(mainDock,SIGNAL(dockLocationChanged( Qt::DockWidgetArea )),this,SLOT(slotMainDockAreaChanged(Qt::DockWidgetArea )));
	connect(tagsDock,SIGNAL(dockLocationChanged( Qt::DockWidgetArea )),this,SLOT(slotTagsDockAreaChanged(Qt::DockWidgetArea )));
	connect(FMLayout::getLayout()->optionDialog,SIGNAL(finished( int )),this,SLOT(slotUpdateLayOptStatus()));



}

void typotek::closeEvent ( QCloseEvent *event )
{
	QSettings settings ;
	if ( systray )
	{
		if ( systray->isVisible() && settings.value ( "Systray/CloseToTray", true ).toBool() )
		{
			if ( !settings.value ( "Systray/CloseNoteShown", false ).toBool() )
			{
				QMessageBox::information ( this, tr ( "Fontmatrix" ),
				                           tr ( "The program will keep running in the "
				                                "system tray. To terminate the program, "
				                                "choose <b>Exit</b> in the context menu "
				                                "of the system tray entry." ) );
				settings.setValue ( "Systray/CloseNoteShown", true );
			}
			hide();
			event->ignore();
			return;
		}
	}
	
// 	save();
	writeSettings();
	event->accept();
	
}

/// IMPORT
// if announce == true user will be shown a dialog of imported fonts
// if announce == false and collect == true all fonts imported will be
// collected and announced next time announce == true
void typotek::open ( QString path, bool announce, bool collect )
{
	static QStringList nameList;
	static QStringList tali; // tali gets reseted when announce = true then the shouldAskTali is also set to true
	static bool shouldAskTali = true; // initial tags is only asked once if collect == true
	QStringList pathList;

	QFileInfo finfo ( path );
	if ( finfo.isDir() || path.isEmpty() ) // importing a directory
	{
		static QSettings settings;
		static QString dir = settings.value ( "Places/LastUsedFolder", QDir::homePath() ).toString(); // first time use the home path then remember the last used dir
		QDir d ( dir );
		if ( !d.exists() )
			dir = QDir::homePath();

		QString tmpdir;

		if ( !path.isEmpty() )
			tmpdir = path;
		else
			tmpdir = QFileDialog::getExistingDirectory ( this, tr ( "Add Directory" ), dir  ,  QFileDialog::ShowDirsOnly );

		if ( tmpdir.isEmpty() )
			return; // user choose to cancel the import process

		dir = tmpdir; // only set dir if importing wasn't cancelled
		settings.setValue ( "Places/LastUsedFolder", dir );

		QDir theDir ( dir );
		// 	addFcDirItem(theDir.absolutePath());

		QStringList dirList ( fontmatrix::exploreDirs ( dir,0 ) );
		// 	qDebug() << dirList.join ( "\n" );

		QStringList yetHereFonts;
// 		for(int i=0;i < fontMap.count() ; ++i)
// 			yetHereFonts << fontMap[i]->path();
		yetHereFonts = FMFontDb::DB()->AllFontNames();

		QStringList filters;
		filters << "*.otf" << "*.pfb" << "*.ttf" ;
		foreach ( QString dr, dirList )
		{
			QDir d ( dr );
			QFileInfoList fil= d.entryInfoList ( filters );
			foreach ( QFileInfo fp, fil )
			{
				if ( ( !yetHereFonts.contains ( fp.absoluteFilePath() ) ) )
				{
					if ( fp.isSymLink() ) // #12232
					{
						QFileInfo fsym ( fp.symLinkTarget() );
						if ( ( !fsym.isSymLink() ) // hey, donnot try to fool us with nested symlinks :)
						        && ( fsym.exists() )
						        && ( !yetHereFonts.contains ( fsym.absoluteFilePath() ) ) )
							pathList <<  fsym.absoluteFilePath();

					}
					else
						pathList <<  fp.absoluteFilePath();
				}
			}
		}
	}
	else if ( finfo.isFile() )
		pathList <<  finfo.absoluteFilePath();

	/* Everybody say it’s useless...
		NO IT'S NOT. I'm a keen fan of this feature. Let's make it optional */
	QStringList tagsList ( FMFontDb::DB()->getTags() );
	if ( useInitialTags && shouldAskTali )
	{
		ImportTags imp ( this,tagsList );
		imp.exec();
		tali = imp.tags();
		shouldAskTali = false;
	}

	QProgressDialog progress ( tr ( "Importing font files... " ), tr ( "cancel" ), 0, pathList.count(), this );
	bool showProgress = pathList.count() > 1;
	if ( showProgress ) // show progress bar only if there's more than one font
	{
		progress.setWindowModality ( Qt::WindowModal );
		progress.setAutoReset ( false );
		progress.setValue ( 0 );
		progress.show();
	}
	FMFontDb::DB()->TransactionBegin();
	QString importstring ( tr ( "Import" ) +  " %1" );
	FMFontDb *DB ( FMFontDb::DB() );
	QList<FontItem*> nf;
	for ( int i = 0 ; i < pathList.count(); ++i )
	{
		QString pathCur ( pathList.at ( i ) );
		if ( showProgress )
		{
			progress.setLabelText ( importstring.arg ( pathCur ) );
			progress.setValue ( i );
			if ( progress.wasCanceled() )
				break;
		}



		{
			QFile ff ( pathCur );
			QFileInfo fi ( pathCur );
			{
				FontItem *fitem ( DB->Font ( fi.absoluteFilePath(), true ) );
				if ( fitem )
				{
					nf << fitem;
					fitem->setActivated ( false );
					if ( announce || collect )
						nameList << fitem->fancyName();
				}
				else
				{
					QString errorFont ( tr ( "Can’t import this font because it’s broken :" ) +" "+fi.fileName() );
					statusBar()->showMessage ( errorFont );
					if ( announce || collect )
						nameList << "__FAILEDTOLOAD__" + fi.fileName();
				}
			}
		}
	}

	QStringList tl;
	foreach ( QString tag, tali )
	{
		tl.clear();
		foreach ( FontItem* f, nf )
		{
			tl << f->path();
		}
		DB->addTag ( tl, tag );
	}
	DB->TransactionEnd();
	progress.close();

	if ( announce )
	{
		if ( showFontListDialog )
		{
			// The User needs and deserves to know what fonts hve been imported
			ImportedFontsDialog ifd ( this, nameList );
			ifd.exec();
		}
		else   // show info in the statusbar
		{
			statusBar()->showMessage ( tr ( "Fonts imported: %1" ).arg ( nameList.count() ), 3000 );
		}
		nameList.clear();
		tali.clear();
		shouldAskTali = true;
	}
	theMainView->slotReloadFontList();
	ListDockWidget::getInstance()->reloadTagsCombo();

}

void typotek::openList ( QStringList files )
{
	QStringList pathList;
	QStringList nameList;
	QStringList tali;
	QStringList tagsList ( FMFontDb::DB()->getTags() );
	if ( useInitialTags )
	{
		ImportTags imp ( this,tagsList );
		imp.exec();
		tali = imp.tags();
	}

	FMFontDb *DB ( FMFontDb::DB() );
	QStringList fontMap ( DB->AllFontNames() );
	foreach ( QString file, files )
	{
		QFileInfo fp ( file );
		if ( ( !fontMap.contains ( fp.absoluteFilePath() ) ) )
		{
			if ( fp.isSymLink() ) // #12232
			{
				QFileInfo fsym ( fp.symLinkTarget() );
				if ( ( !fsym.isSymLink() )
				        && ( fsym.exists() )
				        && ( !fontMap.contains ( fsym.absoluteFilePath() ) ) )
					pathList <<  fsym.absoluteFilePath();

			}
			else
				pathList <<  fp.absoluteFilePath();
		}
	}

	DB->TransactionBegin();
	QProgressDialog progress ( tr ( "Importing font files... " ),tr ( "cancel" ), 0, pathList.count(), this );
	progress.setWindowModality ( Qt::WindowModal );
	progress.setAutoReset ( false );

	QList<FontItem*> nf;
	QString importstring ( tr ( "Import" ) +" %1" );
	for ( int i = 0 ; i < pathList.count(); ++i )
	{
		progress.setLabelText ( importstring.arg ( pathList.at ( i ) ) );
		progress.setValue ( i );
		if ( progress.wasCanceled() )
			break;

		QFile ff ( pathList.at ( i ) );
		QFileInfo fi ( pathList.at ( i ) );

		FontItem *fitem = DB->Font ( fi.absoluteFilePath() ,true );
		if ( fitem )
		{
			nf << fitem;
			nameList << fitem->fancyName();
		}
		else
		{
			QString errorFont ( tr ( "Can’t import this font because it’s broken :" ) +" "+fi.fileName() );
			statusBar()->showMessage ( errorFont );
			nameList << "__FAILEDTOLOAD__" + fi.fileName();
		}
	}
	QStringList tl;
	foreach ( QString tag, tali )
	{
		tl.clear();
		foreach ( FontItem* f, nf )
		{
			tl << f->path();
		}
		DB->addTag ( tl, tag );
	}
	DB->TransactionEnd();
	progress.close();

	// The User needs and deserves to know what fonts hve been imported
	if ( showFontListDialog )
	{
		// The User needs and deserves to know what fonts hve been imported
		ImportedFontsDialog ifd ( this, nameList );
		ifd.exec();
	}
	else   // show info in the statusbar
	{
		statusBar()->showMessage ( tr ( "Fonts imported: %1" ).arg ( nameList.count() ), 3000 );
	}

	theMainView->slotReloadFontList();
	ListDockWidget::getInstance()->reloadTagsCombo();

}

/// EXPORT
void typotek::slotExportFontSet()
{
	QStringList tagsList(FMFontDb::DB()->getTags());
	QStringList items ( tagsList );
// 	items.removeAll ( "Activated_On" );
// 	items.removeAll ( "Activated_Off" );
	bool ok;
	QString item = QInputDialog::getItem ( this, "Fontmatrix Tags",
	                                       tr ( "Choose the tag for filter exported fonts" ), items, 0, false, &ok );
	if ( ok && !item.isEmpty() )
	{


		QString dir( QDir::homePath() );
		dir = QFileDialog::getExistingDirectory ( this, tr ( "Choose Directory" ), dir  ,  QFileDialog::ShowDirsOnly );
		if ( dir.isEmpty() )
			return;

		DataExport dx(dir,item);
		dx.doExport();
	}

}

bool typotek::save()
{
	SaveData saver ( &ResourceFile, this );
	return true;

}

void typotek::about()
{
	AboutWidget aabout(this);
	aabout.exec();
}

void typotek::createActions()
{
	Shortcuts *scuts = Shortcuts::getInstance();

	openAct = new QAction ( QIcon ( ":/fontmatrix_import_icon" ), tr ( "&Import..." ), this );
	openAct->setShortcut ( tr ( "Ctrl+O" ) );
	openAct->setStatusTip ( tr ( "Import a directory" ) );
	scuts->add(openAct);
	connect ( openAct, SIGNAL ( triggered() ), this, SLOT ( open() ) );

	saveAct = new QAction ( tr ( "&Sync" ), this );
	saveAct->setShortcut ( tr ( "Ctrl+S" ) );
	saveAct->setStatusTip ( tr ( "Sync with the DB file" ) );
	scuts->add(saveAct);
	connect ( saveAct, SIGNAL ( triggered() ), this, SLOT ( save()) );

	exportFontSetAct = new QAction(tr("Export &Fonts"),this);
	exportFontSetAct->setStatusTip(tr("Export a fontset"));
	scuts->add(exportFontSetAct);
	connect( exportFontSetAct,SIGNAL(triggered( )),this,SLOT(slotExportFontSet()));

	printInfoAct = new QAction ( tr ( "Print Info..." ),this );
	printInfoAct->setStatusTip ( tr ( "Print informations about the current font" ) );
	scuts->add(printInfoAct);
	connect ( printInfoAct, SIGNAL ( triggered() ), this, SLOT ( printInfo() ) );

	printSampleAct = new QAction ( tr ( "Print Sample..." ),this );
	printSampleAct->setStatusTip( tr("Print the sample as a specimen"));
	scuts->add(printSampleAct);
	connect (printSampleAct,SIGNAL( triggered() ), this, SLOT ( printSample()) );

	printChartAct = new QAction ( tr ( "Print Chart..." ),this );
	printChartAct->setStatusTip( tr("Print a chart of the current font"));
	scuts->add(printChartAct);
	connect (printChartAct,SIGNAL( triggered() ), this, SLOT ( printChart()) );

	printPlaygroundAct = new QAction ( tr ( "Print Playground..." ),this );
	printPlaygroundAct->setStatusTip( tr("Print the playground"));
	scuts->add(printPlaygroundAct);
	connect (printPlaygroundAct,SIGNAL( triggered() ), this, SLOT ( printPlayground()) );

	printFamilyAct = new QAction ( tr ( "Print Family..." ),this );
	printFamilyAct->setStatusTip( tr("Print a specimen of the whole family the current face belongs to"));
	scuts->add(printFamilyAct);
	connect (printFamilyAct,SIGNAL( triggered() ), this, SLOT ( printFamily()) );

	fontBookAct = new QAction ( QIcon ( ":/fontmatrix_fontbookexport_icon.png" ), tr ( "Export font book..." ),this );
	fontBookAct->setStatusTip ( tr ( "Export a pdf that show selected fonts" ) );
	scuts->add(fontBookAct);
	connect ( fontBookAct, SIGNAL ( triggered() ), this, SLOT ( fontBook() ) );

	exitAct = new QAction ( tr ( "E&xit" ), this );
	exitAct->setShortcut ( tr ( "Ctrl+Q" ) );
	exitAct->setStatusTip ( tr ( "Exit the application" ) );
	scuts->add(exitAct);
	connect ( exitAct, SIGNAL ( triggered() ), this, SLOT ( close() ) );


	aboutAct = new QAction ( tr ( "&About" ), this );
	aboutAct->setStatusTip ( tr ( "Show the Typotek's About box" ) );
	scuts->add(aboutAct);
	connect ( aboutAct, SIGNAL ( triggered() ), this, SLOT ( about() ) );

	aboutQtAct = new QAction ( tr ( "About &Qt" ), this );
	scuts->add(aboutQtAct);
	connect (aboutQtAct,SIGNAL(triggered()), QApplication::instance(),SLOT(aboutQt()));

	helpAct = new QAction ( tr ( "Help" ), this );
	helpAct->setCheckable(true);
	helpAct->setChecked(false);
	scuts->add(helpAct);
	connect ( helpAct,SIGNAL ( triggered( ) ),this,SLOT ( helpBegin() ) );

// 	tagsetAct = new QAction ( tr ( "&Tag Sets" ),this );
// 	tagsetAct->setIcon ( QIcon ( ":/fontmatrix_tagseteditor_icon.png" ) );
// 	scuts->add(tagsetAct);
// 	connect ( tagsetAct,SIGNAL ( triggered( ) ),this,SLOT ( popupTagsetEditor() ) );

	activCurAct = new QAction ( tr ( "Activate all current" ),this );
	scuts->add(activCurAct);
	connect ( activCurAct,SIGNAL ( triggered( ) ),this,SLOT ( slotActivateCurrents() ) );

	deactivCurAct = new QAction ( tr ( "Deactivate all current" ),this );
	scuts->add(deactivCurAct);
	connect ( deactivCurAct,SIGNAL ( triggered( ) ),this,SLOT ( slotDeactivateCurrents() ) );

	fonteditorAct = new QAction ( tr ( "Edit current font" ),this );
	scuts->add(fonteditorAct);
	connect ( fonteditorAct,SIGNAL ( triggered( ) ),this,SLOT ( slotEditFont() ) );
	if ( QFile::exists ( fonteditorPath ) )
	{
		fonteditorAct->setStatusTip ( tr ( "Try to run font editor with the selected font as argument" ) );
	}
	else
	{
		fonteditorAct->setEnabled ( false );
		fonteditorAct->setStatusTip ( tr ( "You don't seem to have font editor installed. Path to font editor can be set in preferences." ) );
	}

	prefsAct = new QAction ( tr ( "Preferences" ),this );
	scuts->add(prefsAct);
	connect ( prefsAct,SIGNAL ( triggered() ),this,SLOT ( slotPrefsPanelDefault() ) );

	repairAct = new QAction ( tr("Check Database"), this);
	scuts->add(repairAct);
	connect( repairAct, SIGNAL ( triggered() ),this,SLOT (slotRepair()));

	if ( systray )
		connect ( theMainView, SIGNAL ( newTag ( QString ) ), systray, SLOT ( newTag ( QString ) ) );

	tagAll = new QAction(tr("Tag All..."), this);
	scuts->add(tagAll);
	connect(tagAll,SIGNAL(triggered()),this,SLOT(slotTagAll()));
	
	showTTTAct = new QAction(tr("Show TrueType tables"),this);
	scuts->add(showTTTAct);
	connect(showTTTAct,SIGNAL(triggered( )),this,SLOT(slotShowTTTables()));

	nextFamily = new QAction(tr("Next Family"), this);
	nextFamily->setShortcut(Qt::Key_PageDown);
	scuts->add(nextFamily);
	connect(nextFamily, SIGNAL(triggered()), ListDockWidget::getInstance()->fontTree, SLOT(slotNextFamily()));

	nextFont = new QAction(tr("Next Face"), this);
	nextFont->setShortcut(Qt::Key_Down);
	scuts->add(nextFont);
	connect(nextFont, SIGNAL(triggered()), ListDockWidget::getInstance()->fontTree, SLOT(slotNextFont()));

	previousFamily = new QAction(tr("Previous Family"), this);
	previousFamily->setShortcut(Qt::Key_PageUp);
	scuts->add(previousFamily);
	connect(previousFamily, SIGNAL(triggered()), ListDockWidget::getInstance()->fontTree, SLOT(slotPreviousFamily()));


	previousFont = new QAction(tr("Previous Face"), this);
	previousFont->setShortcut(Qt::Key_Up);
	scuts->add(previousFont);
	connect(previousFont, SIGNAL(triggered()), ListDockWidget::getInstance()->fontTree, SLOT(slotPreviousFont()));

	layOptAct = new QAction(tr("Layout Options"),this);
	layOptAct->setCheckable(true);
	scuts->add(layOptAct);
	connect(layOptAct,SIGNAL(triggered()),this,SLOT(slotSwitchLayOptVisible()));
}

void typotek::createMenus()
{
	fileMenu = menuBar()->addMenu ( tr ( "&File" ) );

	fileMenu->addAction ( openAct );
	fileMenu->addAction ( saveAct );
	fileMenu->addAction ( exportFontSetAct );
	fileMenu->addSeparator();

	printMenu = fileMenu->addMenu(tr("Print"));
	printMenu->addAction(printInfoAct);
	printMenu->addAction(printSampleAct);
	printMenu->addAction(printChartAct);
	printMenu->addAction(printPlaygroundAct);
	printMenu->addAction(printFamilyAct);

	fileMenu->addAction ( fontBookAct );
	fileMenu->addSeparator();
	fileMenu->addAction ( exitAct );

	editMenu = menuBar()->addMenu ( tr ( "&Edit" ) );
// 	editMenu->addAction ( tagsetAct );
	editMenu->addSeparator();
	editMenu->addAction( tagAll );
	editMenu->addAction ( activCurAct );
	editMenu->addAction ( deactivCurAct );
	editMenu->addSeparator();
	editMenu->addAction ( fonteditorAct );
#ifdef PLATFORM_APPLE
#elif _WIN32
#else
	editMenu->addAction( repairAct );
#endif
	editMenu->addSeparator();
	editMenu->addAction ( prefsAct );

	editMenu->addSeparator();
	editMenu->addAction(layOptAct);
	editMenu->addAction(showTTTAct);

	browseMenu = menuBar()->addMenu(tr("&Browse"));
	browseMenu->addAction(nextFamily);
	browseMenu->addAction(previousFamily);
	browseMenu->addAction(nextFont);
	browseMenu->addAction(previousFont);
	
	viewMenu = createPopupMenu();
	viewMenu->setTitle(tr("&View"));
	menuBar()->addMenu(viewMenu);
			
	helpMenu = menuBar()->addMenu ( tr ( "&Help" ) );
	helpMenu->addAction ( helpAct );
	helpMenu->addAction ( aboutAct );
	helpMenu->addAction ( aboutQtAct );

}

void typotek::createStatusBar()
{
	statusBar()->showMessage ( tr ( "Ready" ) );
			
	statusProgressBar = new QProgressBar(this);
	statusProgressBar->setMaximumSize(200,20);
	statusBar()->addPermanentWidget(statusProgressBar);
	statusProgressBar->hide();

	QFont statusFontFont ( "sans-serif", 10 );
	curFontPresentation = new QLabel ( "" );
	curFontPresentation->setFrameShape(QFrame::StyledPanel);
	curFontPresentation->setAlignment ( Qt::AlignRight );
	curFontPresentation->setFont ( statusFontFont );
	statusBar()->addPermanentWidget ( curFontPresentation );
	
	countFilteredFonts = new QLabel ( "" );
	countFilteredFonts->setFrameShape(QFrame::StyledPanel);
	countFilteredFonts->setAlignment ( Qt::AlignRight );
	countFilteredFonts->setFont ( statusFontFont );
	statusBar()->addPermanentWidget ( countFilteredFonts );
}

void typotek::readSettings()
{
	relayStartingStepIn(tr("Load settings"));
	QSettings settings;
	QPoint pos = settings.value ( "WState/pos", QPoint ( 200, 200 ) ).toPoint();
	QSize size = settings.value ( "WState/size", QSize ( 400, 400 ) ).toSize();
	resize ( size );
	move ( pos );
	
	fonteditorPath = settings.value ( "FontEditor", "/usr/bin/fontforge" ).toString();
	useInitialTags = settings.value ( "UseInitialTags", false ).toBool();
	showFontListDialog = settings.value("ShowImportedFonts", true).toBool();
	previewSize = settings.value("Preview/Size", 15.0).toDouble();
	previewRTL = settings.value("Preview/RTL", false).toBool();
	previewSubtitled = settings.value("Preview/Subtitled", false).toBool();
	
	mainDockArea = settings.value("Docks/ToolPos", "Left").toString();
	tagsDockArea = settings.value("Docks/TagsPos", "Right").toString();
	mainDockGeometry = settings.value("Docks/ToolGeometry", QRect()).toRect();
	tagsDockGeometry = settings.value("Docks/TagsGeometry", QRect()).toRect();
	
// 	m_familySchemeFreetype = settings.value("FamilyPreferred", true).toBool();
	
	templatesDir = settings.value ( "Places/TemplatesDir", "./").toString();
	m_welcomeURL = settings.value("Places/WelcomeURL").toString();
	m_remoteTmpDir = settings.value("Places/RemoteTmpDir", QDir::tempPath()).toString();
	
	defaultOTFScript = settings.value("OTF/Script").toString();
	defaultOTFLang = settings.value("OTF/Lang").toString();
	defaultOTFGPOS = settings.value("OTF/GPOS").toString().split(";",QString::SkipEmptyParts);
	defaultOTFGSUB = settings.value("OTF/GSUB").toString().split(";",QString::SkipEmptyParts);
	chartInfoFontSize = settings.value("ChartInfoFontSize", 8).toInt();
	chartInfoFontName = settings.value("ChartInfoFontFamily", QFont().family() ).toString();
	
	
	databaseDriver = settings.value("Database/Driver","QSQLITE").toString();
	databaseHostname = settings.value("Database/Hostname","").toString();
	databaseDbName = settings.value("Database/DbName", ownDir.absolutePath()+ QDir::separator() + "Data.sql").toString();
	databaseUser = settings.value("Database/User","").toString();
	databasePassword = settings.value("Database/Password","").toString();
	if( !QSqlDatabase::drivers().contains(databaseDriver) )
		qDebug()<<"The SQL driver you request is not available("<< databaseDriver <<")";
	
}

void typotek::writeSettings()
{
	QSettings settings;
	settings.setValue( "WState/pos", pos() );
	settings.setValue( "WState/size", size() );
	
	if(mainDock->isFloating())
		mainDockArea = "Float";
	if(tagsDock->isFloating())
		tagsDockArea = "Float";
	mainDockGeometry = mainDock->geometry();
	tagsDockGeometry = tagsDock->geometry();
	settings.setValue( "Docks/ToolPos", mainDockArea );
	settings.setValue( "Docks/TagsPos", tagsDockArea );
	settings.setValue( "Docks/ToolGeometry", mainDockGeometry);
	settings.setValue( "Docks/TagsGeometry", tagsDockGeometry);
	
	settings.setValue( "WState/SplitterViewState", theMainView->splitterState(SPLITTER_VIEW_1));
	
	settings.setValue( "Database/Driver",databaseDriver);
	settings.setValue( "Database/Hostname",databaseHostname);
	settings.setValue( "Database/DbName",databaseDbName);
	settings.setValue( "Database/User",databaseUser);
	settings.setValue( "Database/Password",databasePassword);
	
	save();

}

bool typotek::maybeSave()
{

	return true;
}

typotek::~typotek()
{

}

void typotek::fillTagsList()
{

}

void typotek::checkOwnDir()
{
	relayStartingStepIn(tr("Check for Fontmatrix own dir"));
	QString sep(QDir::separator());
	
#ifdef PLATFORM_APPLE

	QString rootDir(QDir::homePath() + sep + "Library" + sep + "Fontmatrix" + sep);
	managedDir.setPath(QDir::homePath() + sep + "Library" + sep + "Fonts");
	ownDir.setPath(rootDir);
	if ( !managedDir.exists() )
		managedDir.mkpath ( QDir::homePath() + sep + "Library" + sep + "Fonts" );
	if(!ownDir.exists())
		ownDir.mkpath (rootDir);
	
	ResourceFile.setFileName ( rootDir + "Resource.xml" );

#elif _WIN32
	// For win we do not hide things because it does
	// not work yet. So if you need to debug it will be simpler.
	QString fontmanaged ( sep + "fontmatrix" );
	managedDir.setPath ( QDir::homePath() + fontmanaged );
	if ( !managedDir.exists() )
		managedDir.mkpath ( QDir::homePath() + fontmanaged );
	ownDir = managedDir;
	ResourceFile.setFileName ( QDir::homePath() + sep +"fontmatrix.data" );
#else
	QString rootDir(QDir::homePath() + sep + ".Fontmatrix" + sep);  
	ownDir.setPath(rootDir);
	// Where activated fonts are sym-linked
	managedDir.setPath ( rootDir + "Activated" );
	if ( !managedDir.exists() )
		managedDir.mkpath ( rootDir + "Activated"  );

	addFcDirItem( managedDir.absolutePath() );
	
	ResourceFile.setFileName ( rootDir + "Resource.xml" );
#endif
}

void typotek::addFcDirItem(const QString & dirPath)
{
#ifdef HAVE_FONTCONFIG
	QFile fcfile ( QDir::homePath() + "/.fonts.conf" );
	if ( !fcfile.open ( QFile::ReadWrite ) )
	{
		return;
	}
	else
	{
		QDomDocument fc ( "fontconfig" );

		// .fonts.conf is empty, it seems that we just created it.
		// Wed have to populate it a bit
		if ( fcfile.size() == 0 )
		{
			QString ds ( "<?xml version='1.0'?><!DOCTYPE fontconfig SYSTEM 'fonts.dtd'><fontconfig></fontconfig>" );
			fc.setContent ( ds );
		}
		else
		{
			fc.setContent ( &fcfile );
		}

		bool isconfigured = false;
		QDomNodeList dirlist = fc.elementsByTagName ( "dir" );
		for ( int i=0;i < dirlist.count();++i )
		{
			if ( dirlist.at ( i ).toElement().text() == dirPath )
				isconfigured = true;
		}
		if ( !isconfigured )
		{
			QDomElement root = fc.documentElement();
			QDomElement direlem = fc.createElement ( "dir" );
			QDomText textelem = fc.createTextNode ( dirPath );
			direlem.appendChild ( textelem );
			root.appendChild ( direlem );
			fcfile.resize ( 0 );

			QTextStream ts ( &fcfile );
			fc.save ( ts,4 );

		}
		fcfile.close();
	}
#endif
}

QStringList typotek::getSystemFontDirs()
{
	QStringList retList;
#ifdef HAVE_FONTCONFIG // For Unices (OSX excluded)
	QStringList tmpList;
	FcConfig* FcInitLoadConfig();
	FcStrList *sysDirList = FcConfigGetFontDirs(0);
	QString sysDir( (char*)FcStrListNext(sysDirList) );
	while(!sysDir.isEmpty())
	{
		if(!sysDir.contains("Fontmatrix"))
		{
			tmpList << sysDir;
		}
		sysDir = ( (char*)FcStrListNext(sysDirList) );
	}
	foreach(QString path, tmpList)
	{
		bool root(true);
		foreach(QString ref, tmpList)
		{
			if(path != ref)
			{
				if(ref.startsWith(path))
					root = false;
			}
		}
		if(root)
			retList << path;
	}
	
#endif //HAVE_FONTCONFIG
#ifdef PLATFORM_APPLE
	retList << "/Library/Fonts";
	retList << "/System/Library/Fonts";
#endif // PLATFORM_APPLE
#if _WIN32
	retList << getWin32SystemFontDir();
#endif // _WIN32
	return retList;
}

bool typotek::isSysFont(FontItem * f)
{
	if(f)
	{
		if(sysFontList.contains(f->path()))
			return true;
	}
	return false;
}

void typotek::initDir()
{
	DataLoader loader ( &ResourceFile );
	int lRes(loader.load());

	/// let’s load system fonts
#define SYSTEM_FONTS 1
	if(SYSTEM_FONTS)
	{
		QString SysColFon = tr ( "System Fonts" );
		QStringList tagsList(FMFontDb::DB()->getTags());
			
		QList<FontItem*> sysFontPtrs;
	
		QStringList sysDir ( getSystemFontDirs() );
		qDebug()<<sysDir.join("\n");
		
		QStringList yetHereFonts;
		QList<FontItem*> fontMap(FMFontDb::DB()->AllFonts());
		for ( int i=0;i < fontMap.count() ; ++i )
			yetHereFonts << fontMap[i]->path();
		
		for ( int sIdx ( 0 ); sIdx < sysDir.count(); ++sIdx )
		{
			QDir theDir ( sysDir[sIdx] );
			QStringList syspathList;
			QStringList nameList;
	
			QStringList dirList ( fontmatrix::exploreDirs ( theDir,0 ) );	
			QStringList filters;
			filters << "*.otf" << "*.pfb" << "*.ttf" ;
			foreach ( QString dr, dirList )
			{
				QDir d ( dr );
				QFileInfoList fil= d.entryInfoList ( filters );
				foreach ( QFileInfo fp, fil )
				{
					if ( !yetHereFonts.contains ( fp.absoluteFilePath() ) )
						syspathList <<  fp.absoluteFilePath();
					sysFontList << fp.absoluteFilePath();
				}
			}
	
			int sysFontCount ( syspathList.count() );
			if(sysFontCount > 0)
			{
				relayStartingStepIn ( tr ( "Adding" ) +" "+ QString::number ( sysFontCount ) +" "+tr ( "fonts from","followed by a directory name" )  +" "+sysDir[sIdx]);
				qDebug()<< ( tr ( "Adding" ) +" "+ QString::number ( sysFontCount ) +" "+tr ( "fonts from","followed by a directory name" )  +" "+sysDir[sIdx]);
				FMFontDb::DB()->TransactionBegin();
				for ( int i = 0 ; i < sysFontCount; ++i )
				{
					QFile ff ( syspathList.at ( i ) );
					QFileInfo fi ( syspathList.at ( i ) );
					{
						FontItem *fitem = FMFontDb::DB()->Font( fi.absoluteFilePath(), false );
						if ( fitem )
						{
// 							qDebug()<<"\t"<<fitem <<fitem->path();
							fitem->setActivated ( true );
// 							fitem->addTag ( SysColFon );
							sysFontPtrs << fitem;
						}
						else
						{
							qDebug() << "Cannot open this font because its broken : " << fi.fileName() ;
						}
					}
				}
				FMFontDb::DB()->TransactionEnd();
			}
		}
		
		relayStartingStepIn ( QString::number ( sysFontPtrs.count() ) + " " + tr ( "system fonts added." ) );
		
		// So much complicated only because otherwise, tags were added twice with SQLite ???
		QStringList tl;
		foreach(FontItem* sfp, sysFontPtrs)
		{
			tl << sfp->path();
		}
		FMFontDb::DB()->addTag(tl, SysColFon);
	}
	

// 	qDebug()<<"TIME(fonts) : "<<fontsTime.elapsed();
	/// Remote dirs
	//TODO
// 	QSettings settings;
// 	QStringList remoteDirV ( settings.value ( "RemoteDirectories" ).toStringList() );
// 	if ( !remoteDirV.isEmpty() )
// 	{
// 		relayStartingStepIn ( tr ( "Catching" ) +" "+ QString::number ( remoteDirV.count() ) +" "+tr ( "font descriptions from network" ) );
// 		remoteDir = new RemoteDir ( remoteDirV );
// 		connect ( remoteDir,SIGNAL ( listIsReady() ),this,SLOT ( slotRemoteIsReady() ) );
// 		remoteDir->run();
// 	}
}

static bool slotRemoteIsReadyRunOnce = false;
void typotek::slotRemoteIsReady()
{
	if(!slotRemoteIsReadyRunOnce)
		slotRemoteIsReadyRunOnce = true;
	else
		return;
	QStringList tagsList(FMFontDb::DB()->getTags());

// 	qDebug()<<"typotek::slotRemoteIsReady()";
	QList<FontInfo> listInfo(remoteDir->rFonts());
// 	qDebug()<< "Have got "<< listInfo.count() <<"remote font descriptions";
	for(int rf(0) ;rf < listInfo.count(); ++rf)
	{
// 		qDebug()<< rf <<" : " <<listInfo[rf].dump();
		FontItem *fi = new FontItem ( listInfo[rf].file , true );
		if(!fi->isValid())
		{
			qDebug() << "ERROR loading : " << listInfo[rf].file;
			continue;
		}
		fi->fileRemote(listInfo[rf].family,listInfo[rf].variant,listInfo[rf].type, listInfo[rf].info, listInfo[rf].pix);
// 		fontMap.append ( fi );
// 		realFontMap[fi->path() ] = fi;
		fi->setTags ( listInfo[rf].tags );
		foreach(QString tag, listInfo[rf].tags)
		{
			if(!tag.isEmpty() && !tagsList.contains(tag))
			{
				tagsList << tag;
				theMainView->slotAppendTag(tag);
			}
		}
	}
	theMainView->slotReloadFontList();
	showStatusMessage( QString::number(listInfo.count())+ " " +  tr("font descriptions imported from network"));
// 	qDebug()<<"END OF slotRemoteIsReady()";
}

void typotek::resetFilter()
{
	theMainView->resetCrumb();
}


void typotek::fontBook()
{
	FontBook fontbook;
	fontbook.doBook();
}

QList<FontItem*> typotek::getCurrentFonts()
{
	return theMainView->curFonts();
}

FontItem * typotek::getSelectedFont()
{
	return theMainView->selectedFont();
}

void typotek::keyPressEvent ( QKeyEvent * event )
{
// 	qDebug() << "typotek::keyPressEvent(QKeyEvent * "<<event<<")";
	if(/*event->modifiers().testFlag(Qt::ControlModifier) &&*/ event->key() == Qt::Key_J)
		qDebug()<<"NUM FACES OPENED:"<<fm_num_face_opened;
}

void typotek::slotActivateCurrents()
{
	if ( QMessageBox::question ( this,tr ( "Fontmatrix care" ),tr ( "You are about to activate a bunch of fonts,\nit is time to cancel if it was not your intent" ), QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel ) == QMessageBox::Ok )
		theMainView->slotActivateAll();
}

void typotek::slotDeactivateCurrents()
{
	if ( QMessageBox::question ( this,tr ( "Fontmatrix care" ),tr ( "You are about to deactivate a bunch of fonts,\nit is time to cancel if it was not your intent" ),QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel ) == QMessageBox::Ok )
		theMainView->slotDesactivateAll();
}

void typotek::helpBegin()
{
	theHelp = new HelpWidget(this);
	helpAct->setChecked(true);

	connect( theHelp, SIGNAL( end() ), this, SLOT(helpEnd()) );

	disconnect ( helpAct,SIGNAL ( triggered( ) ),this,SLOT ( helpBegin() ) );
	connect ( helpAct,SIGNAL ( triggered( ) ),this,SLOT ( helpEnd() ) );

	theHelp->show();
}

void typotek::helpEnd()
{
	helpAct->setChecked(false);

	disconnect ( helpAct,SIGNAL ( triggered( ) ),this,SLOT ( helpEnd() ) );
	connect ( helpAct,SIGNAL ( triggered( ) ),this,SLOT ( helpBegin() ) );

	theHelp->deleteLater();
}

void typotek::slotEditFont()
{
	FontItem *item = theMainView->selectedFont();
	if ( !item )
	{
		statusBar()->showMessage ( tr ( "There is no font selected" ), 5000 );
		return;
	}

	QStringList arguments;
	arguments << "-nosplash" << item->path() ;

	QProcess *myProcess = new QProcess ( this );
	myProcess->start ( fonteditorPath, arguments );
}

void typotek::setupDrop()
{
	setAcceptDrops ( true );
	// was pretty hard!
}

void typotek::dropEvent ( QDropEvent * event )
{

	qDebug() << "typotek::dropEvent ("<< event->mimeData()->text() <<")";
// 	qDebug()<<"F: "<<event->mimeData()->formats().join(";");

// 	event->acceptProposedAction();
	QStringList uris = event->mimeData()->text().split ( "\n" );
	QStringList ret;

	// Internal drag & drop
	if(event->source() && event->source()->objectName() == "folderView" )
	{
		QString fP(ListDockWidget::getInstance()->getFolderCurrentIndex().data(QDirModel::FilePathRole).toString());

		if(QFileInfo(fP).isDir())
		{
			open(fP);
			return;
		}
		else
		{
			{
				uris << "file://" + fP;
			}
		}

	}

	for ( int i = 0; i < uris.count() ; ++i )
	{
		qDebug() << "dropped uri["<< i <<"] -> "<< uris[i];
		QUrl url ( uris[i].trimmed() );
		qDebug() << "\tURL -> " << url.toLocalFile();
		if ( url.scheme() == "file" )
		{
			if ( url.toLocalFile().endsWith ( "ttf",Qt::CaseInsensitive ) )
			{
				ret << url.toLocalFile();
			}
			else if ( url.toLocalFile().endsWith ( "otf",Qt::CaseInsensitive ) )
			{
				ret << url.toLocalFile();
			}
			else if ( url.toLocalFile().endsWith ( "pfb",Qt::CaseInsensitive ) )
			{
				ret << url.toLocalFile();
			}
			else
			{
				qDebug() << url.toLocalFile ()  << "is not a supported font file";
			}
		}
		else if ( url.scheme() == "http" )
		{
			// TODO Get fonts over http
			qDebug() << "Support of DragNDrop over http is sheduled";
			statusBar()->showMessage ( tr ( "Support of DragNDrop over http is sheduled but not yet effective" ), 5000 );
		}
		else
		{
			qDebug() << "Protocol not supported";
		}
	}

// 	qDebug() << ret.join("||");
	if ( ret.count() )
		openList( ret );

}

void typotek::dragEnterEvent ( QDragEnterEvent * event )
{
	qDebug() << event->mimeData()->formats().join ( "|" );
	if ( event->mimeData()->hasFormat ( "text/uri-list" ) )
	{
		event->acceptProposedAction();
		qDebug() << "dragEnterEvent accepted " ;
	}
	else
	{
		qDebug() << "dragEnterEvent refused";
		statusBar()->showMessage ( tr ( "You bring something over me I can’t handle" ), 2000 );
	}
}

void typotek::slotPrefsPanel(PrefsPanelDialog::PAGE page)
{
	PrefsPanelDialog pp ( this );

	// FIXME if Systray is not available, systray->whatever() will segault
	if ( QSystemTrayIcon::isSystemTrayAvailable() )
		pp.initSystrayPrefs ( QSystemTrayIcon::isSystemTrayAvailable(),
		                      systray->isVisible(),
		                      systray->hasActivateAll(),
		                      systray->allConfirmation(),
		                      systray->tagsConfirmation() );
	else
		pp.initSystrayPrefs ( false,false,false,false,false );
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
	theMainView->slotView ( true );
}

void typotek::addNamedSample ( QString name, QString sample )
{
	if ( name.isEmpty() || sample.isEmpty() )
	{
		statusBar()->showMessage ( tr ( "You provided an empty string, it’s not fair" ), 3000 );
		return;
	}

	if ( name == defaultSampleName() )
	{
		statusBar()->showMessage ( tr ( "\"default\" is a reserved" ), 3000 );
		return;
	}
	m_namedSamples[name] = sample;
	theMainView->refillSampleList();
}

void typotek::removeNamedSample(const QString& key)
{
	m_namedSamples.remove(key);
	theMainView->refillSampleList();
}

void typotek::setSystrayVisible ( bool isVisible )
{
	systray->slotSetVisible ( isVisible );
}

void typotek::showActivateAllSystray ( bool isVisible )
{
	systray->slotSetActivateAll ( isVisible );
}

void typotek::systrayAllConfirmation ( bool isEnabled )
{
	systray->requireAllConfirmation ( isEnabled );
}

void typotek::systrayTagsConfirmation ( bool isEnabled )
{
	systray->requireTagsConfirmation ( isEnabled );
}

void typotek::slotCloseToSystray ( bool isEnabled )
{
	QSettings settings ;
	settings.setValue ( "Systray/CloseToTray", isEnabled );
	settings.setValue ( "Systray/CloseNoteShown", false );
}

void typotek::slotSystrayStart( bool isEnabled )
{
	QSettings settings ;
	settings.setValue ( "Systray/StartToTray", isEnabled );
}

void typotek::addNamedSampleFragment ( QString name, QString sampleFragment )
{
	// No need to check, it’s just for the loader
	QStringList lines = m_namedSamples[name].split ( "\n" );
	if ( lines[0].isEmpty() )
		lines.removeAt ( 0 );
	lines << sampleFragment;
	m_namedSamples[name] = lines.join ( "\n" );
}

QString typotek::namedSample ( QString name )
{
	if ( name.isEmpty() )
		return m_namedSamples[defaultSampleName()];
	return m_namedSamples[name];
}

void typotek::setSampleText ( QString s )
{
	m_namedSamples[defaultSampleName()] += s;
}

void typotek::changeSample ( QString name, QString text )
{
	if ( !m_namedSamples.contains ( name ) )
		return;
	m_namedSamples[name] = text;
}

void typotek::setWord ( QString s, bool updateView )
{
	if(s == m_theWord)
		return;
	m_theWord = s;
	QList<FontItem*> fontMap(FMFontDb::DB()->AllFonts());
	for(int i(0); i < fontMap.count(); ++i)
		fontMap[i]->clearPreview() ;
	if ( updateView )
	{
		ListDockWidget::getInstance()->forcePreviewRefill();
		theMainView->slotView ( true );
	}
}

void typotek::setPreviewRTL(bool d)
{
	if(previewRTL == d)
		return;
	previewRTL = d;
	QList<FontItem*> fontMap(FMFontDb::DB()->AllFonts());
	for(int i(0); i < fontMap.count(); ++i)
		fontMap[i]->clearPreview() ;
	emit previewHasChanged();
}

void typotek::setPreviewSubtitled(bool d)
{
	if(previewSubtitled == d)
		return;
	previewSubtitled = d;
	QList<FontItem*> fontMap(FMFontDb::DB()->AllFonts());
	for(int i(0); i < fontMap.count(); ++i)
		fontMap[i]->clearPreview() ;
	emit previewHasChanged();
}

void typotek::setFontEditorPath ( const QString &path )
{
	fonteditorPath = path;
	if ( QFile::exists ( fonteditorPath ) )
	{
		fonteditorAct->setEnabled ( true );
		fonteditorAct->setStatusTip ( tr ( "Try to run font editor with the selected font as argument" ) );
	}
	else
	{
		fonteditorAct->setEnabled ( false );
		fonteditorAct->setStatusTip ( tr ( "You don't seem to have font editor installed. Path to font editor can be set in preferences." ) );
	}
	QSettings settings ;
	settings.setValue ( "FontEditor", fonteditorPath );
}

void typotek::slotUseInitialTags ( bool isEnabled )
{
	useInitialTags = isEnabled;
	QSettings settings ;
	settings.setValue ( "UseInitialTags", isEnabled );
}

void typotek::showImportedFonts(int show) // 0 == show dialog, 2 == do not show
{
	bool doShow = true;
	if (show == Qt::Checked)
		doShow = false;
	showFontListDialog = doShow;
	QSettings settings;
	settings.setValue("ShowImportedFonts", doShow);
}

bool typotek::showImportedFonts()
{
	return showFontListDialog;
}

void typotek::setTemplatesDir(const QString & dir)
{
	templatesDir = dir;
	QSettings settings;
	settings.setValue("Places/TemplatesDir", templatesDir);

}

void typotek::changeFontSizeSettings(double fSize, double lSize)
{
	QSettings settings;
	settings.setValue("Sample/FontSize", fSize);
	settings.setValue("Sample/Interline", lSize);
	theMainView->reSize(fSize,lSize);
}

void typotek::relayStartingStepIn(QString s)
{
	int i(Qt::AlignCenter);
	QColor c(Qt::white);
	emit relayStartingStepOut( s, i , c );
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
	foreach(QString key, keyList)
	{
		removeFontItem(key);
	}
}

void typotek::showStatusMessage(const QString & message)
{
	statusBar()->showMessage(message, 2500);
}

void typotek::setRemoteTmpDir(const QString & s)
{
	if(s.isEmpty())
		m_remoteTmpDir = QDir::temp().path();
	else
		m_remoteTmpDir = s;

	QSettings settings;
	settings.setValue("Places/RemoteTmpDir", m_remoteTmpDir);
}

void typotek::slotRepair()
{
	FmRepair repair(this);
	repair.exec();
}

void typotek::slotTagAll()
{
	QStringList tagsList(FMFontDb::DB()->getTags());
	ImportTags imp(this,tagsList);
	imp.exec();
	QStringList tali = imp.tags();

	if(tali.isEmpty())
		return;
	for(int t(0); t < tali.count(); ++t)
	{
		if(!tagsList.contains(tali[t]))
		{
			tagsList.append(tali[t]);
			emit tagAdded(tali[t]);
		}
	}

	QList<FontItem*> curfonts = theMainView->curFonts();
	for(int i(0) ; i < curfonts.count(); ++i)
	{
		for(int t(0); t < tali.count(); ++t)
		{
			curfonts[i]->addTag(tali[t]);
		}

	}
	TagsWidget::getInstance()->newTag();
}

void typotek::printInfo()
{
	FontItem * font(theMainView->selectedFont());
	QString fontname(tr("Welcome maessage"));
	if(font)
		fontname =font->fancyName();

	QPrinter thePrinter ( QPrinter::HighResolution );
	QPrintDialog dialog(&thePrinter, this);
	dialog.setWindowTitle("Fontmatrix - " + tr("Print Infos") +" - " + fontname );

	if ( dialog.exec() != QDialog::Accepted )
		return;
	thePrinter.setFullPage ( true );
	theMainView->info()->print(&thePrinter);
}

void typotek::printSample()
{
	FontItem * font(theMainView->selectedFont());
	if(!font)
		return;
	QGraphicsScene *ls (theMainView->currentSampleScene());
	if( FMLayout::getLayout()->isRunning() )
	{
		connect(FMLayout::getLayout(), SIGNAL(paintFinished()), this,SLOT(printSample()));
// 		waitLayoutForPrint = false;
		return;
	}
	else
	{
		disconnect(FMLayout::getLayout(), SIGNAL(paintFinished()), this,SLOT(printSample()));
// 		waitLayoutForPrint = true;
	}

	QPrinter thePrinter ( QPrinter::HighResolution );
	QPrintDialog dialog(&thePrinter, this);
	dialog.setWindowTitle("Fontmatrix - " + tr("Print Sample") +" - " + font->fancyName() );

	if ( dialog.exec() != QDialog::Accepted )
		return;
	thePrinter.setFullPage ( true );
	QPainter aPainter ( &thePrinter );

	ls->render(&aPainter);

}

void typotek::printChart()
{
	FontItem * font(theMainView->selectedFont());
	if(!font)
		return;

	QPrinter thePrinter ( QPrinter::HighResolution );
	QPrintDialog dialog(&thePrinter, this);
	dialog.setWindowTitle("Fontmatrix - " + tr("Print Chart") +" - " + font->fancyName() );

	if ( dialog.exec() != QDialog::Accepted )
		return;
	thePrinter.setFullPage ( true );
	QPainter aPainter ( &thePrinter );


	double pWidth(thePrinter.paperRect().width());
	double pHeight(thePrinter.paperRect().height());
	double pFactor( thePrinter.resolution() );

	qDebug()<<"Paper :"<<pWidth<<pHeight;
	qDebug()<<"Resolution :"<<pFactor;
	qDebug()<<"P/R*72:"<<pWidth / pFactor * 72.0<< pHeight / pFactor * 72.0;

	QRectF targetR( pWidth * 0.1, pHeight * 0.1, pWidth * 0.8, pHeight * 0.8 );


	QRectF sourceR( 0, 0, pWidth / pFactor * 72.0, pHeight / pFactor * 72.0);
	QGraphicsScene pScene(sourceR);

	int maxCharcode(0x10FFFF);
	int beginCharcode(0);
	int numP(0);
	bool first(true);
	while(beginCharcode < maxCharcode)
	{
		qDebug() << "Chart("<< ++numP <<") ->"<<beginCharcode<<maxCharcode;
		QList<QGraphicsItem*> lgit(pScene.items());
		foreach(QGraphicsItem* git, lgit)
		{
			pScene.removeItem(git);
			delete git;
		}

		int controlN(maxCharcode - beginCharcode);
		int stopAtCode( font->renderChart(&pScene, beginCharcode, maxCharcode, sourceR.width(),sourceR.height() ) );
		qDebug()<< "Control"<<beginCharcode<<stopAtCode;

		if(stopAtCode == beginCharcode)
			break;

		if(first)
		{
			first = false;
		}
		else
		{
			thePrinter.newPage();
		}
		aPainter.drawText(targetR.bottomLeft(), font->fancyName()+"[U"+QString::number(beginCharcode  ,16).toUpper()+", U"+QString::number(stopAtCode ,16).toUpper()+"]");
		pScene.render(&aPainter,targetR, sourceR, Qt::KeepAspectRatio);

		beginCharcode = stopAtCode;
	}


}

void typotek::printPlayground()
{
	FontItem * font(theMainView->selectedFont());
	QPrinter thePrinter ( QPrinter::HighResolution );
	QPrintDialog dialog(&thePrinter, this);
	dialog.setWindowTitle("Fontmatrix - " + tr("Print Playground")  );

	if ( dialog.exec() != QDialog::Accepted )
		return;
	thePrinter.setFullPage ( true );
	QPainter aPainter ( &thePrinter );

	double pWidth(thePrinter.paperRect().width());
	double pHeight(thePrinter.paperRect().height());

	QRectF targetR( pWidth * 0.1, pHeight * 0.1, pWidth * 0.8, pHeight * 0.8 );
	QRectF sourceR( theMainView->getPlayground()->getMaxRect() );

	theMainView->getPlayground()->scene()->render(&aPainter, targetR ,sourceR, Qt::KeepAspectRatio );
}

void typotek::printFamily()
{
	FontItem * font(theMainView->selectedFont());
	if(!font)
		return;
	QPrinter thePrinter ( QPrinter::HighResolution );
	QPrintDialog dialog(&thePrinter, this);
	dialog.setWindowTitle("Fontmatrix - " + tr("Print Family") +" - " + font->family());

	if ( dialog.exec() != QDialog::Accepted )
		return;

	thePrinter.setFullPage ( true );
	QPainter aPainter ( &thePrinter );

	QGraphicsScene tmpScene(thePrinter.paperRect());
	QGraphicsScene pScene(thePrinter.paperRect());

	qDebug()<<thePrinter.paperRect();

	QMap<int , double> logWidth;
	QMap<int , double> logAscend;
	QMap<int , double> logDescend;
	QMap<int , QString> sampleString;
	QMap<int , FontItem*> sampleFont;

	QStringList stl1(namedSample ( theMainView->sampleName() ).split ( QRegExp("\\W") ));
	if(stl1.count() < 10 )
	{
		QMessageBox::information(this,"Fontmatrix",tr("Not enough text to make a sample"));
		return;
	}
	QStringList stl;
	int idxS(0);
	int idxE( qrand() % 9 );
	while((idxS + idxE) < stl1.count())
	{
		QString t(QStringList(stl1.mid(idxS,idxE)).join( " " ));
		qDebug()<<"s e T"<<idxS<<idxE<<t;
		if(!t.isEmpty())
			stl << t;

		idxS += idxE;
		idxE = qrand() % 9;
	}
	QList<FontItem*> familyFonts(FMFontDb::DB()->Fonts(theMainView->selectedFont()->family(), FMFontDb::Family ));

// 	if(familyFonts.count() > stl.count())
	{
		int diff ( familyFonts.count()  );
		for (int i(0); i < diff; ++i)
		{
			sampleString[i] = stl[ i % stl.count() ];
		}
	}

	// first we’ll get widths for font size 1000
	for(int fidx(0); fidx < familyFonts.count(); ++fidx)
	{
		sampleFont[fidx] = familyFonts[fidx];
		bool rasterState(sampleFont[fidx]->rasterFreetype());
		sampleFont[fidx]->setFTRaster(false);
		sampleFont[fidx]->setRenderReturnWidth(true);
		logWidth[fidx] =  familyFonts[fidx]->renderLine(&tmpScene, sampleString[fidx], QPointF(0.0, 1000.0) , 999999.0, 1000.0, 1, false) ;
		sampleFont[fidx]->setRenderReturnWidth(false);
		sampleFont[fidx]->setFTRaster(rasterState);
		logAscend[fidx] = 1000.0 - tmpScene.itemsBoundingRect().top();
		logDescend[fidx] = tmpScene.itemsBoundingRect().bottom() - 1000.0;
		qDebug()<< sampleString[fidx] << logWidth[fidx];
		QList<QGraphicsItem*> lgit(tmpScene.items());
		foreach(QGraphicsItem* git, lgit)
		{
			tmpScene.removeItem(git);
			delete git;
		}
	}
	double defWidth(0.8 * pScene.width() );
	double defHeight(0.9 * pScene.height() );
	double xOff( 0.1 * pScene.width() );
	double yPos(0.1 * pScene.height() );

	QFont nameFont;
	nameFont.setPointSizeF(100.0);
	nameFont.setItalic(true);

	for(int fidx(0); fidx < familyFonts.count(); ++fidx)
	{
		double scaleFactor(1000.0 / logWidth[fidx] );
		double fSize( defWidth *  scaleFactor );
		double fAscend(logAscend[fidx] * fSize / 1000.0);
		double fDescend(logDescend[fidx] * fSize  / 1000.0 );
		if( yPos + fAscend + fDescend > defHeight)
		{
			pScene.render(&aPainter);
			thePrinter.newPage();
			QList<QGraphicsItem*> lgit(pScene.items());
			foreach(QGraphicsItem* git, lgit)
			{
				pScene.removeItem(git);
				delete git;
			}
			yPos = 0.1 * pScene.height();

		}

		yPos +=  fAscend;
		QPointF origine(xOff,  yPos );

		qDebug()<< sampleString[fidx] << fSize;

		bool rasterState(sampleFont[fidx]->rasterFreetype());
		sampleFont[fidx]->setFTRaster(false);
		sampleFont[fidx]->renderLine(&pScene, sampleString[fidx], origine, pScene.width(), fSize, 100, false);
		pScene.addLine(QLineF(origine, QPointF(xOff + defWidth, yPos)));
		sampleFont[fidx]->setFTRaster(rasterState);

		yPos +=  fDescend ;

		QGraphicsSimpleTextItem * nameText = pScene.addSimpleText( familyFonts[fidx]->fancyName(), nameFont) ;
		nameText->setPos(xOff, yPos);
// 		nameText->setBrush(Qt::gray);
		yPos += nameText->boundingRect().height();
	}

	pScene.render(&aPainter);
}

void typotek::showEvent(QShowEvent * event)
{
	QMainWindow::showEvent(event);

	if(!theMainView->selectedFont())
		theMainView->displayWelcomeMessage();
}

void typotek::slotMainDockAreaChanged(Qt::DockWidgetArea area)
{
	if(area == Qt::LeftDockWidgetArea)
		mainDockArea = "Left";
	else if(area ==Qt::RightDockWidgetArea)
		mainDockArea ="Right";
	else if(area == Qt::TopDockWidgetArea)
		mainDockArea ="Top";
	else if(area == Qt::BottomDockWidgetArea)
		mainDockArea = "Bottom";
}

void typotek::slotTagsDockAreaChanged(Qt::DockWidgetArea area)
{
	if(area == Qt::LeftDockWidgetArea)
		tagsDockArea = "Left";
	else if(area ==Qt::RightDockWidgetArea)
		tagsDockArea ="Right";
	else if(area == Qt::TopDockWidgetArea)
		tagsDockArea ="Top";
	else if(area == Qt::BottomDockWidgetArea)
		tagsDockArea = "Bottom";
}

FMHyphenator* typotek::getHyphenator() const
{
	return hyphenator;
}

void typotek::slotSwitchLayOptVisible()
{
	if(FMLayout::getLayout()->optionDialog->isVisible())
	{
		FMLayout::getLayout()->optionDialog->setVisible(false);
	}
	else
	{
		FMLayout::getLayout()->optionDialog->setVisible(true);
	}
	slotUpdateLayOptStatus();
}

void typotek::slotUpdateLayOptStatus()
{
	if(FMLayout::getLayout()->optionDialog->isVisible())
	{
		layOptAct->setChecked(true);
	}
	else
	{
		layOptAct->setChecked(false);
	}
}

QString typotek::getDefaultOTFScript() const
{
	return defaultOTFScript;
}

void typotek::setDefaultOTFScript ( const QString& theValue )
{
	if(theValue != defaultOTFScript)
	{
		QSettings st;
		st.setValue("OTF/Script" , theValue);
	}
	defaultOTFScript = theValue;

}

QString typotek::getDefaultOTFLang() const
{
	return defaultOTFLang;
}

void typotek::setDefaultOTFLang ( const QString& theValue )
{
	if(theValue != defaultOTFLang)
	{
		QSettings st;
		st.setValue("OTF/Lang" , theValue);
	}
	defaultOTFLang = theValue;
}

QStringList typotek::getDefaultOTFGPOS() const
{
	return defaultOTFGPOS;
}

void typotek::setDefaultOTFGPOS ( const QStringList& theValue )
{
	if(theValue != defaultOTFGPOS)
	{
		QSettings st;
		st.setValue("OTF/GPOS" , theValue.join(";"));
	}
	defaultOTFGPOS = theValue;
}

QStringList typotek::getDefaultOTFGSUB() const
{
	return defaultOTFGSUB;
}

void typotek::setDefaultOTFGSUB ( const QStringList& theValue )
{
	if(theValue != defaultOTFGSUB)
	{
		QSettings st;
		st.setValue("OTF/GSUB" , theValue.join(";"));
	}
	defaultOTFGSUB = theValue;
}

void typotek::startProgressJob(int max)
{
	statusProgressBar->reset();
	statusProgressBar->setRange(0,max);
	statusProgressBar->show();
}

void typotek::runProgressJob(int i)
{
	if(i)
	{
		statusProgressBar->setValue(i);
	}
	else
	{
		int v(statusProgressBar->value());
		statusProgressBar->setValue( ++v );
	}
}

void typotek::endProgressJob()
{
	statusProgressBar->hide();
	statusProgressBar->reset();
}

void typotek::slotShowTTTables()
{
	if(theMainView->selectedFont())
	{
		QDialog dia(this);
		QGridLayout glayout(&dia);
		TTTableView tv(theMainView->selectedFont(),&dia);
		QPushButton pbutton(tr("Close"),&dia);
		glayout.addWidget(&tv,0,0,3,3);
		glayout.addWidget(&pbutton,3,2);
		pbutton.setDefault(true);
		
		QRect drect(dia.rect());
		drect.setX(this->geometry().x() + 32);
		drect.setY(this->geometry().y() + 32);
		drect.setWidth(tv.rect().width() * 1.2);
		drect.setHeight(tv.rect().height() * 1.2);
		dia.setGeometry(drect);
		connect(&pbutton,SIGNAL(released()),&dia,SLOT(close()));
		dia.exec();
	}
}

void typotek::showToltalFilteredFonts()
{
	countFilteredFonts->setText( tr( "Filtered Font(s) : %n", "number of filtererd fonts showed in status bar", theMainView->curFonts().count() ) );
}

void typotek::presentFontName(QString s)
{
	curFontPresentation->setText(tr("Current Font :", "followed by currently selected font name (in status bar)") +s);
}








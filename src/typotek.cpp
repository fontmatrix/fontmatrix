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



#include "aboutwidget.h"
#include "browserwidget.h"
#include "dataexport.h"
#include "dataloader.h"
#include "dumpdialog.h"
#include "floatingwidget.h"
#include "floatingwidgetsregister.h"
#include "fmactivate.h"
#include "fmfontdb.h"
#include "fmfontextract.h"
#include "fmmatchraster.h"
#include "fmlayout.h"
#include "fmpaths.h"
#include "fmrepair.h"
#include "fontbook.h"
#include "fontcomparewidget.h"
#include "fontitem.h"
// #include "helpwidget.h"
#include "helpbrowser.h"
#include "hyphenate/fmhyphenator.h"
#include "importedfontsdialog.h"
#include "importtags.h"
//#include "listdockwidget.h"
#include "mainviewwidget.h"
#include "panosedialog.h"
#include "panosewidget.h"
#include "playwidget.h"
#include "prefspaneldialog.h"
#include "remotedir.h"
//#include "savedata.h"
#include "shortcuts.h"
#include "systray.h"
#include "tagswidget.h"
#include "tttableview.h"
#include "typotek.h"
#include "winutils.h"

#include <QDesktopWidget>
#include <QtGui>
#include <QTextEdit>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDir>
#include <QProgressDialog>
#include <QProgressBar>
#include <QDomDocument>
#include <QProcess>
#include <QDockWidget>
#include <QStackedWidget>
#include <QMessageBox>

#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif


#ifdef HAVE_PYTHONQT
#include "fmpython_w.h"
#include "fmscriptconsole.h"
#define MAX_RECENT_PYSCRIPTS 10
#endif // HAVE_PYTHONQT

#ifdef Q_WS_MAC
#include <ApplicationServices/ApplicationServices.h>
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
	dataLoader = 0;
	playVisible = false;

	m_dpiX = ( double ) QApplication::desktop()->physicalDpiX();
	m_dpiY = ( double ) QApplication::desktop()->physicalDpiY();
#ifdef Q_WS_MAC
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

	qDebug()<< m_dpiX << m_dpiY;
}

void typotek::initMatrix()
{
	qDebug()<<"Main Thread:"<<thread();
	if(matrix)
		return;
	matrix = true;
	fontmatrix::fillDockPos();

	checkOwnDir();
	readSettings();
	initDir();
	

	theMainView = new MainViewWidget ( this );
	theBrowser = new BrowserWidget(this);
	mainStack = new QStackedWidget(this);

	mainStack->addWidget(theMainView);
	mainStack->addWidget(theBrowser);
	setCentralWidget ( mainStack );

	if ( QSystemTrayIcon::isSystemTrayAvailable() )
		systray = new Systray();
	else
		systray = 0;

	setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::ForceTabbedDocks);

//	installDock("Main", tr ( "Browse Fonts" ), ListDockWidget::getInstance() , tr ( "Show/hide fonts browsing sidebar" ));
//	installDock("Tags", tr ( "Tags" ), TagsWidget::getInstance() ,  tr ( "Show/hide tags list sidebar" ) );
//	installDock("Panose", tr ( "Panose"), PanoseWidget::getInstance(), tr ( "Browse fonts by means of Panose attributes" ) );

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
	createMenus();
	createStatusBar();
	doConnect();

	showToltalFilteredFonts();

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

void typotek::installDock(const QString& id, const QString& name, QWidget * w, const QString& tip)
{
	QDockWidget * dw = new QDockWidget(name);
	dw->setObjectName(id);
	dockWidget[id] = dw;
	dw->setWidget( w );
	dw->setStatusTip ( tip );
	addDockWidget(fontmatrix::DockPosition[dockArea[id]], dw);
	qDebug()<<"I"<<id<<dockArea[id]<<dockVisible[id];
	if(dockArea[id] == QString("Float"))
		dw->setFloating(true);
	if(!dockGeometry[id].isNull())
		dw->setGeometry(dockGeometry[id]);

	connect(dw , SIGNAL(dockLocationChanged( Qt::DockWidgetArea )),
		this, SLOT(slotDockAreaChanged(Qt::DockWidgetArea )));
}

void typotek::postInit()
{
	// TODO restore last filter
//	theMainView->slotViewAll();
	
	QSettings st;
	QString cname(st.value("CurrentFont", QString()).toString());
//	if(!cname.isEmpty())
//	{
//		if(!ListDockWidget::getInstance()->fontTree->slotSetCurrent(cname))
//			theMainView->displayWelcomeMessage();
//	}
//	else
//		theMainView->displayWelcomeMessage();
}

void typotek::doConnect()
{
	if(getSystray())
		connect ( FMActivate::getInstance() ,SIGNAL ( activationEvent ( const QStringList& ) ), getSystray(),SLOT ( updateTagMenu ( const QStringList& ) ) );

//	connect(FMLayout::getLayout()->optionDialog,SIGNAL(finished( int )),this,SLOT(slotUpdateLayOptStatus()));
#ifdef HAVE_PYTHONQT
	connect(FMScriptConsole::getInstance(),SIGNAL(finished()), this, SLOT(slotUpdateScriptConsoleStatus()));
#endif
	connect(toggleMainViewButton, SIGNAL(toggled(bool)), this, SLOT(toggleMainView(bool)));
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

	foreach(FloatingWidget *f, FloatingWidgetsRegister::AllWidgets())
	{
		f->close();
	}
	writeSettings();

	delete PlayWidget::getInstance();
	delete FontCompareWidget::getInstance();
	delete theMainView;

	event->accept();

}

/// IMPORT
// if announce == true user will be shown a dialog of imported fonts
// if announce == false and collect == true all fonts imported will be
// collected and announced next time announce == true
void typotek::open ( QString path, bool recursive, bool announce, bool collect )
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


		QStringList dirList;
		if(recursive)
			dirList =  fontmatrix::exploreDirs ( dir,0 ) ;
		else
			dirList.append(dir);
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
	
	// It can happen that you wrongly select a dir, it is time to let the user cancel the import.
	// I want it :) - pm
	if ( /*( pathList.count() > 1 )
		&&*/ ( QMessageBox::question ( this,
	                                     QString ( "Fontmatrix - %1" ).arg ( tr ( "Confirmation" ) ) ,
	                                     tr ( "Do you confirm you want to import %n font(s)?", "", pathList.count()),
	                                     QMessageBox::Yes | QMessageBox::No,
	                                     QMessageBox::No )
	             != QMessageBox::Yes ) )
	{
		return;
	}

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
					QString errorFont ( tr ( "Cannot import this font because it is broken:" ) +" "+fi.fileName() );
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
	emit newFontsArrived();
}

void typotek::importFiles()
{
	QStringList flist = QFileDialog::getOpenFileNames(this,
							  tr("Select Files to Import"),
							  QDir::homePath(),
							  QString("%1 (*.otf *.ttf *.pfb)").arg(tr("Font Files")));
	if(!flist.isEmpty())
		openList(flist);
}

/// Import files in a drop event.
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
			QString errorFont ( tr ( "Cannot import this font because it is broken: " ) +" "+fi.fileName() );
//			statusBar()->showMessage ( errorFont );
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
	emit newFontsArrived();

}

/// EXPORT
void typotek::slotExportFontSet()
{
//	QStringList tagsList(FMFontDb::DB()->getTags());
//	QStringList items ( tagsList );
//	bool ok;
//	QString item = QInputDialog::getItem ( this, tr ( "Fontmatrix Tags" ),
//	                                       tr ( "Choose the tag for filter exported fonts" ), items, 0, false, &ok );
//	if ( ok && !item.isEmpty() )
//	{


//		QString dir( QDir::homePath() );
//		dir = QFileDialog::getExistingDirectory ( this, tr ( "Choose Directory" ), dir  ,  QFileDialog::ShowDirsOnly );
//		if ( dir.isEmpty() )
//			return;

//		DataExport dx(dir,item);
//		dx.doExport();
//	}
	new DataExport(this);
}


void typotek::about()
{
	AboutWidget aabout(this);
	aabout.exec();
}

void typotek::createActions()
{
	Shortcuts *scuts = Shortcuts::getInstance();

	openAct = new QAction ( QIcon ( ":/fontmatrix_import_icon" ), tr ( "&Import Directory..." ), this );
	openAct->setShortcut ( Qt::CTRL + Qt::Key_O );
	openAct->setToolTip( tr ( "Import a directory" ) );
	scuts->add(openAct);
	connect ( openAct, SIGNAL ( triggered() ), this, SLOT ( open() ) );

	importFilesAction = new QAction(QIcon ( ":/fontmatrix_import_icon" ), tr ( "Import &Files..." ), this );
	importFilesAction->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_O );
	importFilesAction->setToolTip(tr("Import Files"));
	scuts->add(importFilesAction);
	connect(importFilesAction, SIGNAL(triggered()), this, SLOT(importFiles()));

	exportFontSetAct = new QAction(tr("Export &fonts"),this);
	exportFontSetAct->setStatusTip(tr("Export a fontset"));
	scuts->add(exportFontSetAct);
	connect( exportFontSetAct,SIGNAL(triggered( )),this,SLOT(slotExportFontSet()));


	fontBookAct = new QAction ( QIcon ( ":/fontmatrix_fontbookexport_icon.png" ), tr ( "Export font book..." ),this );
	fontBookAct->setStatusTip ( tr ( "Export a PDF document that shows selected fonts" ) );
	scuts->add(fontBookAct);
	connect ( fontBookAct, SIGNAL ( triggered() ), this, SLOT ( fontBook() ) );

	dumpInfoAct = new QAction(tr("Export font info for packaging..."), this);
	dumpInfoAct->setStatusTip ( tr ( "Fill a template file with metadata for packaging currently selected font to a Linux distribution" ) );
	connect(dumpInfoAct, SIGNAL(triggered()), this, SLOT(slotDumpInfo()));

	exitAct = new QAction ( tr ( "E&xit" ), this );
	exitAct->setShortcut ( Qt::CTRL + Qt::Key_Q );
	exitAct->setStatusTip ( tr ( "Exit the application" ) );
        exitAct->setMenuRole(QAction::QuitRole);
	scuts->add(exitAct);
	connect ( exitAct, SIGNAL ( triggered() ), this, SLOT ( close() ) );


	aboutAct = new QAction ( tr ( "&About" ), this );
	aboutAct->setStatusTip ( tr ( "Show information about Fontmatrix" ) );
        aboutAct->setMenuRole(QAction::AboutRole);
	scuts->add(aboutAct);
	connect ( aboutAct, SIGNAL ( triggered() ), this, SLOT ( about() ) );

	aboutQtAct = new QAction ( tr ( "About &Qt" ), this );
	aboutQtAct->setStatusTip ( tr ( "Show information about Qt" ) );
        aboutQtAct->setMenuRole(QAction::AboutQtRole);
	scuts->add(aboutQtAct);
	connect (aboutQtAct,SIGNAL(triggered()), QApplication::instance(),SLOT(aboutQt()));

	helpAct = new QAction ( tr ( "Help" ), this );
	helpAct->setShortcut ( Qt::Key_F1 );
	helpAct->setStatusTip ( tr ( "Read documentation on Fontmatrix" ) );
	helpAct->setCheckable(true);
	helpAct->setChecked(false);
	scuts->add(helpAct);
	connect ( helpAct,SIGNAL ( triggered( ) ),this,SLOT ( helpBegin() ) );

// 	tagsetAct = new QAction ( tr ( "&Tag Sets" ),this );
// 	tagsetAct->setIcon ( QIcon ( ":/fontmatrix_tagseteditor_icon.png" ) );
// 	scuts->add(tagsetAct);
// 	connect ( tagsetAct,SIGNAL ( triggered( ) ),this,SLOT ( popupTagsetEditor() ) );

	activCurAct = new QAction ( tr ( "Activate all current" ),this );
	activCurAct->setStatusTip ( tr ( "Activate all currently visible fonts" ) );
	scuts->add(activCurAct);
	connect ( activCurAct,SIGNAL ( triggered( ) ),this,SLOT ( slotActivateCurrents() ) );

	deactivCurAct = new QAction ( tr ( "Deactivate all current" ),this );
	deactivCurAct->setStatusTip ( tr ( "Deactivate all currently visible fonts" ) );
	scuts->add(deactivCurAct);
	connect ( deactivCurAct,SIGNAL ( triggered( ) ),this,SLOT ( slotDeactivateCurrents() ) );

	fonteditorAct = new QAction ( tr ( "Edit current font" ),this );
	scuts->add(fonteditorAct);
	connect ( fonteditorAct,SIGNAL ( triggered( ) ),this,SLOT ( slotEditFont() ) );
	if ( QFile::exists ( fonteditorPath ) )
	{
		fonteditorAct->setStatusTip ( tr ( "Edit currently selected font in a font editor of your choice" ) );
	}
	else
	{
		fonteditorAct->setEnabled ( false );
		fonteditorAct->setStatusTip ( tr ( "You don't seem to have a font editor installed. Path to font editor can be set in Preferences dialog." ) );
	}

	reloadAct = new QAction( tr ("Reload Filtered"), this );
	reloadAct->setStatusTip(tr ("Reload informations for filtered fonts from the font files they belong to"));
	scuts->add(reloadAct);
	connect(reloadAct, SIGNAL(triggered()), this, SLOT(slotReloadFiltered()));

	reloadSingleAct = new QAction(tr("Reload Selected"), this);
	reloadSingleAct->setStatusTip(tr ("Reload informations for selected font from the font file"));
	scuts->add(reloadSingleAct);
	connect(reloadSingleAct, SIGNAL(triggered()), this, SLOT(slotReloadSingle()));

	prefsAct = new QAction ( tr ( "Preferences" ),this );
	prefsAct->setStatusTip ( tr ( "Setup Fontmatrix" ) );
        prefsAct->setMenuRole(QAction::PreferencesRole);
	scuts->add(prefsAct);
	connect ( prefsAct,SIGNAL ( triggered() ),this,SLOT ( slotPrefsPanelDefault() ) );

	repairAct = new QAction ( tr("Check Database"), this);
	repairAct->setStatusTip ( tr ( "Check Fontmatrix database for dead links to font files" ) );
	scuts->add(repairAct);
	connect( repairAct, SIGNAL ( triggered() ),this,SLOT (slotRepair()));

//	if ( systray )
//		connect ( theMainView, SIGNAL ( newTag ( QString ) ), systray, SLOT ( newTag ( QString ) ) );

	tagAll = new QAction(tr("Tag All Filtered..."), this);
	tagAll->setStatusTip ( tr ( "Tag all currently visible files" ) );
	scuts->add(tagAll);
	connect(tagAll,SIGNAL(triggered()),this,SLOT(slotTagAll()));

	showTTTAct = new QAction(tr("Show TrueType tables"),this);
	showTTTAct->setStatusTip ( tr ( "View hexadecimal values of TrueType tables for currently selected font file" ) );
	scuts->add(showTTTAct);
	connect(showTTTAct,SIGNAL(triggered( )),this,SLOT(slotShowTTTables()));

	editPanoseAct = new QAction(tr("Edit PANOSE metadata"), this);
	editPanoseAct->setStatusTip ( tr ( "Edit PANOSE metadata without saving changes to font files" ) );
	scuts->add(editPanoseAct);
	connect(editPanoseAct, SIGNAL(triggered()), this, SLOT(slotEditPanose()));


	playAction = new QAction(tr("Playground"), this);
	playAction->setShortcut(Qt::CTRL + Qt::Key_G);
	playAction->setToolTip(tr("Show/Hide Playground"));
	playAction->setCheckable(true);
	playAction->setChecked(false);
	scuts->add(playAction);
	connect(playAction, SIGNAL(triggered(bool)), PlayWidget::getInstance(), SLOT(setVisible(bool)));

	compareAction = new QAction(tr("Compare"), this);
	compareAction->setShortcut(Qt::CTRL + Qt::Key_R);
	compareAction->setToolTip(tr("Show/Hide Compare glyphs"));
	compareAction->setCheckable(true);
	compareAction->setChecked(false);
	scuts->add(compareAction);
	connect(compareAction, SIGNAL(triggered(bool)), FontCompareWidget::getInstance(), SLOT(setVisible(bool)));

	closeAllFloat = new QAction(tr("Close All"), this);
	closeAllFloat->setToolTip(tr("Close all floating windows"));
	scuts->add(closeAllFloat);
	connect(closeAllFloat, SIGNAL(triggered()), this, SLOT(closeAllFloatings()));

	showAllFloat = new QAction(tr("Show All"), this);
	showAllFloat->setToolTip(tr("Show all floating windows"));
	scuts->add(showAllFloat);
	connect(showAllFloat, SIGNAL(triggered()), this, SLOT(showAllFloatings()));

	hideAllFloat = new QAction(tr("Hide All"), this);
	hideAllFloat->setToolTip(tr("Hide all floating windows"));
	scuts->add(hideAllFloat);
	connect(hideAllFloat, SIGNAL(triggered()), this, SLOT(hideAllFloatings()));

	floatSep = new QAction(this);
	floatSep->setSeparator(true);

	
	extractFontAction = new QAction(tr("Extract fonts..."),this);
	extractFontAction->setStatusTip ( tr ( "Extract fonts from documents like PDF to PFM file format" ) );
	scuts->add(extractFontAction);
	connect(extractFontAction,SIGNAL(triggered()),this,SLOT(slotExtractFont()));
	
	matchRasterAct = new QAction(tr("Find a font using raster sample..."), this); // FIXME find a name for it
	matchRasterAct->setStatusTip ( tr ( "Find a font using a raster sample of a letter" ) );
	scuts->add(matchRasterAct);
	connect(matchRasterAct,SIGNAL(triggered()),this,SLOT(slotMatchRaster()));
	
	
#ifdef HAVE_PYTHONQT
	execScriptAct = new QAction(tr("Execute Script..."),this);
	execScriptAct->setStatusTip ( tr ( "Execute a Python script" ) );
	scuts->add(execScriptAct);
	connect(execScriptAct,SIGNAL(triggered()),this,SLOT(slotExecScript()));
	
	execLastScriptAct = new QAction(tr("Execute Last Script"),this);
	execLastScriptAct->setStatusTip ( tr ( "Execute the last chosen Python script" ) );
	scuts->add(execLastScriptAct);
	connect(execLastScriptAct,SIGNAL(triggered()),this,SLOT(slotExecLastScript()));
	
	scriptConsoleAct = new QAction(tr("Script Console..."), this);
	scriptConsoleAct->setStatusTip ( tr ( "Open Python scripting console" ) );
	scriptConsoleAct->setCheckable(true);
	scuts->add(scriptConsoleAct);
	connect(scriptConsoleAct, SIGNAL(triggered()), this, SLOT(slotSwitchScriptConsole()));

#endif
}

void typotek::createMenus()
{
	fileMenu = menuBar()->addMenu ( tr ( "&File" ) );

	fileMenu->addAction ( openAct );
	fileMenu->addAction ( importFilesAction );
	fileMenu->addAction ( exportFontSetAct );
	fileMenu->addSeparator();

	fileMenu->addAction ( fontBookAct );
	fileMenu->addAction ( dumpInfoAct );
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
	editMenu->addAction ( editPanoseAct );
	editMenu->addSeparator();
	editMenu->addAction(reloadSingleAct);
	editMenu->addAction(reloadAct);
	editMenu->addSeparator();
	editMenu->addAction ( prefsAct );

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(playAction);
	viewMenu->addAction(compareAction);
	viewMenu->addSeparator();
	connect(viewMenu, SIGNAL(aboutToShow()), this,SLOT(updateFloatingStatus()));

#ifdef HAVE_PYTHONQT
	scriptMenu = menuBar()->addMenu ( tr ( "&Scripts" ) );;
	scriptMenu->addAction(execScriptAct);
	scriptMenu->addAction(scriptConsoleAct);
	scriptMenu->addAction(execLastScriptAct);
#endif
		
	servicesMenu =  menuBar()->addMenu ( tr ( "&Service" ) );
	servicesMenu->addAction(extractFontAction);
	servicesMenu->addAction(matchRasterAct);
#ifdef PLATFORM_APPLE
	// TODO
#elif _WIN32
	// TODO
#else
	servicesMenu->addAction( repairAct );
#endif
	servicesMenu->addAction(showTTTAct);
	servicesMenu->addSeparator();
//	servicesMenu->addAction(layOptAct);
	
	helpMenu = menuBar()->addMenu ( tr ( "&Help" ) );
	helpMenu->addAction ( helpAct );
	helpMenu->addAction ( aboutAct );
	helpMenu->addAction ( aboutQtAct );

}

void typotek::createStatusBar()
{
//	statusBar()->showMessage ( tr ( "Ready" ) );

	statusProgressBar = new QProgressBar(this);
	statusProgressBar->setMaximumSize(200,20);
	statusBar()->addPermanentWidget(statusProgressBar);
	statusProgressBar->hide();

	QFont statusFontFont ( "sans-serif", 10 );
	curFontPresentation = new QLabel ( "" );
	curFontPresentation->setFrameShape(QFrame::StyledPanel);
	curFontPresentation->setAlignment ( Qt::AlignRight );
	curFontPresentation->setFont ( statusFontFont );
	statusBar()->insertWidget ( 0, curFontPresentation );

	countFilteredFonts = new QLabel ( "" );
	countFilteredFonts->setFrameShape(QFrame::StyledPanel);
	countFilteredFonts->setAlignment ( Qt::AlignRight );
	countFilteredFonts->setFont ( statusFontFont );
	statusBar()->addPermanentWidget ( countFilteredFonts );

	toggleMainViewButton = new QToolButton(this);
	toggleMainViewButton->setText(tr("Browse Directories"));
	toggleMainViewButton->setCheckable(true);
	toggleMainViewButton->setToolTip(tr("Toggle Files/Collection view"));
	statusBar()->addPermanentWidget(toggleMainViewButton);
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
	previewSize = settings.value("Preview/Size", 28.0).toDouble();
	previewRTL = settings.value("Preview/RTL", false).toBool();
	previewSubtitled = settings.value("Preview/Subtitled", false).toBool();
	m_theWord = settings.value("Preview/Word", "<name>" ).toString();

//	QStringList dl;
//	dl << "Main" << "Tags" << "Panose";
//	foreach(const QString & ds, dl)
//	{
//		dockArea[ds] =  settings.value("Docks/"+ds+"Pos", "Left").toString();
//		dockVisible[ds] = settings.value("Docks/"+ds+"Visible", true).toBool();
//		dockGeometry[ds] = settings.value("Docks/"+ds+"Geometry", QRect()).toRect();
//		qDebug()<<ds<< dockArea[ds] << dockVisible[ds] <<dockGeometry[ds];
//	}

	panoseMatchTreshold = settings.value("Panose/MatchTreshold" , 1000 ).toInt();

	webBrowser = settings.value("Info/Browser", "Fontmatrix").toString();
	webBrowserOptions = settings.value("Info/BrowserOptions", "").toString();
	previewInfoFontSize = settings.value("Info/PreviewSize", 20.0).toDouble();
	
	infoStyle = settings.value("Info/Style", FMPaths::ResourcesDir() + "info.css").toString();

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
	{
		qDebug()<<"The SQL driver you request is not available("<< databaseDriver <<")";
	}

}

void typotek::writeSettings()
{
	QSettings settings;
	settings.setValue( "WState/pos", pos() );
	settings.setValue( "WState/size", size() );
	theMainView->saveSplitterState();

//	QStringList dl;
//	dl << "Main" << "Tags" << "Panose";
//	foreach(const QString & ds, dl)
//	{
//		if(dockWidget[ds]->isFloating())
//		{
//			dockArea[ds] = "Float";
//		}
//		settings.setValue( "Docks/"+ds+"Pos", dockArea[ds] );
//		settings.setValue( "Docks/"+ds+"Visible", dockWidget[ds]->isVisible());
//		settings.setValue( "Docks/"+ds+"Geometry", dockWidget[ds]->geometry());
//	}


	settings.setValue("Info/PreviewSize", previewInfoFontSize );
	settings.setValue("Info/Style", infoStyle );

	settings.setValue("Preview/Word", m_theWord);

	settings.setValue( "Panose/MatchTreshold", panoseMatchTreshold);

	settings.setValue( "Database/Driver",databaseDriver);
	settings.setValue( "Database/Hostname",databaseHostname);
	settings.setValue( "Database/DbName",databaseDbName);
	settings.setValue( "Database/User",databaseUser);
	settings.setValue( "Database/Password",databasePassword);
	
	if(theMainView->selectedFont())
		settings.setValue("CurrentFont", theMainView->selectedFont()->path());


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
	QFile fcfile ( QDir::homePath() + "/.config/fontconfig/fonts.conf" );
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
	// Because we will go recursivly through these directories, we just want to list the top most ones.
	foreach(QString path, tmpList)
	{
// 		qDebug()<< "PATH"<<path;
		bool root(true);
		foreach(QString ref, tmpList)
		{
			if(path != ref)
			{
				if(path.startsWith(ref))
				{
					root = false;
					break;
				}
// 				qDebug()<<"\tREF"<<ref<<root;
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
	
	qDebug()<<retList.join("\n");
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
	/// let’s load system fonts
#define SYSTEM_FONTS 1
	if(SYSTEM_FONTS)
	{
		relayStartingStepIn ( tr ( "Loading System Fonts") );
		m_sysTagName = tr ( "System Fonts" );
		QStringList tagsList(FMFontDb::DB()->getTags());

		QList<FontItem*> sysFontPtrs;

		QStringList sysDir ( getSystemFontDirs() );

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
// 							fitem->addTag ( m_sysTagName );
							sysFontPtrs << fitem;
						}
						else
						{
							qDebug() << "Cannot open this font because its broken: " << fi.fileName() ;
						}
					}
				}
				FMFontDb::DB()->TransactionEnd();
			}
		}

		if(sysFontPtrs.count() > 0)
			relayStartingStepIn ( QString::number ( sysFontPtrs.count() ) + " " + tr ( "system fonts added." ) );

		// So much complicated only because otherwise, tags were added twice with SQLite ???
		QStringList tl;
		foreach(FontItem* sfp, sysFontPtrs)
		{
			tl << sfp->path();
		}
		FMFontDb::DB()->addTag(tl, m_sysTagName);
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
	QList<RemoteDir::FontInfo> listInfo(remoteDir->rFonts());
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
			}
		}
	}
//	theMainView->slotReloadFontList();
	showStatusMessage( QString::number(listInfo.count())+ " " +  tr("font descriptions imported from network"));
// 	qDebug()<<"END OF slotRemoteIsReady()";
}


void typotek::fontBook()
{
	FontBook fontbook;
	fontbook.doBook(FontBook::Full);
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
	theHelp = new HelpBrowser(this,tr("Fontmatrix Help"));
	helpAct->setChecked(true);

	connect( theHelp, SIGNAL( closed() ), this, SLOT(helpEnd()) );

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
		statusBar()->showMessage ( tr ( "There is no font selected" ), 3000 );
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
			statusBar()->showMessage ( tr ( "Support of DragNDrop over http is sheduled but not yet effective" ), 3000 );
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
		statusBar()->showMessage ( tr ( "You bring something over me I can’t handle" ), 3000 );
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
//	theMainView->slotView ( true );
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


QString typotek::namedSample ( QString name )
{
	QString cn(name);
	if(cn == QString("NEW_SAMPLE"))
		return tr("Edit me!");
	if(cn.isEmpty())
		cn = currentNamedSample;
	else
		currentNamedSample = cn;

	if(!dataLoader)
		dataLoader = new DataLoader();

	const QMap<QString,QString>& us(dataLoader->userSamples());
	foreach(const QString& k, us.keys())
	{
		QString id(QString("User::") + k);
//		qDebug()<<"\t"<<id;
		if(id == cn)
		{
			return us[k];
		}
	}

	const QMap<QString, QMap<QString,QString> >& ss(dataLoader->systemSamples());
	foreach(const QString& pk, ss.keys())
	{
		foreach(const QString& sk, ss[pk].keys())
		{
			QString id(pk + QString("::") + sk);
//			qDebug()<<"\t"<<id;
			if(id == cn)
			{
				return ss[pk][sk];
			}
		}
	}
	return namedSample(defaultSampleName());
}

QMap<QString,QList<QString> > typotek::namedSamplesNames()
{
	if(!dataLoader)
		dataLoader = new DataLoader();

	QMap<QString,QList<QString> > ret;
	const QMap<QString,QString>& us(dataLoader->userSamples());
	const QMap<QString, QMap<QString,QString> >& ss(dataLoader->systemSamples());
	foreach(const QString& key, ss.keys())
	{
		ret[key] << ss[key].keys();
	}
	ret[QString("User")] << us.keys();
	return ret;
}

void typotek::addNamedSample ( QString name, QString sample )
{
	if(!dataLoader)
		dataLoader = new DataLoader();

	dataLoader->update(name, sample);
//	theMainView->refillSampleList();
}

void typotek::removeNamedSample(const QString& key)
{
	if(!dataLoader)
		dataLoader = new DataLoader();

	dataLoader->remove(key);
//	theMainView->refillSampleList();
}

void typotek::changeSample ( QString name, QString text )
{
	if(!dataLoader)
		dataLoader = new DataLoader();

	dataLoader->update(name, text);
//	theMainView->refillSampleList();
}

QString typotek::defaultSampleName()
{
	if(!dataLoader)
		dataLoader = new DataLoader();

	const QMap<QString,QString>& us(dataLoader->userSamples());
	if(us.contains(tr("default")))
		return QString("User::") + QString("default");
	else if(us.count() > 0)
		return QString("User::") + us.keys().first();
	else
	{
		const QMap<QString, QMap<QString,QString> >& ss(dataLoader->systemSamples());
		QString l(QLocale::system().language());
		if((ss.contains(l)) && (ss[l].count() > 0))
			return l + QString("::") + ss[l].keys().first();
		else
		{
			foreach(QString k, ss.keys())
			{
				if(ss[k].count() > 0)
					return k +  QString("::") +ss[k].keys().first();
			}
		}
	}
	return QString();
}

void typotek::setWord ( QString s, bool updateView )
{
	if(s == m_theWord)
		return;
	m_theWord = s;
	QList<FontItem*> fontMap(FMFontDb::DB()->AllFonts());
	for(int i(0); i < fontMap.count(); ++i)
		fontMap[i]->clearPreview() ;

	emit previewHasChanged();
}

void typotek::setPreviewSize(double d)
{
	if(previewSize == d)
		return;

//	previewSize = d;
//	if(previewSize != ListDockWidget::getInstance()->previewSize->value())
//		ListDockWidget::getInstance()->previewSize->setValue(previewSize);
	QList<FontItem*> fontMap(FMFontDb::DB()->AllFonts());
	for(int i(0); i < fontMap.count(); ++i)
		fontMap[i]->clearPreview() ;
	emit previewHasChanged();
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
//	theMainView->reSize(fSize,lSize);
}

void typotek::relayStartingStepIn(QString s)
{
	int i( Qt::AlignRight | Qt::AlignBottom );
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
	statusBar()->showMessage(message, 3000);
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
		}
	}
	
	FMFontDb::DB()->TransactionBegin();
	QList<FontItem*> curfonts = theMainView->curFonts();
	for(int i(0) ; i < curfonts.count(); ++i)
	{
		for(int t(0); t < tali.count(); ++t)
		{
			curfonts[i]->addTag(tali[t]);
		}

	}
	FMFontDb::DB()->TransactionEnd();
//	TagsWidget::getInstance()->newTag();
}


//void typotek::printFamily()
//{
//	FontItem * font(theMainView->selectedFont());
//	if(!font)
//		return;
//	QPrinter thePrinter ( QPrinter::HighResolution );
//	QPrintDialog dialog(&thePrinter, this);
//	dialog.setWindowTitle("Fontmatrix - " + tr("Print Family") +" - " + font->family());

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
//		QMessageBox::information(this,"Fontmatrix",tr("Not enough text to make a sample"));
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
//		foreach(QGraphicsItem* git, lgit)
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
//			foreach(QGraphicsItem* git, lgit)
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

void typotek::showEvent(QShowEvent * event)
{
	QMainWindow::showEvent(event);
}

void typotek::slotDockAreaChanged(Qt::DockWidgetArea area)
{
	QDockWidget * dw(reinterpret_cast<QDockWidget*>(sender()));

	if(dw)
	{
		QString wId(dw->objectName());
		if(area == Qt::LeftDockWidgetArea)
			dockArea[wId] = "Left";
		else if(area ==Qt::RightDockWidgetArea)
			dockArea[wId] ="Right";
		else if(area == Qt::TopDockWidgetArea)
			dockArea[wId] ="Top";
		else if(area == Qt::BottomDockWidgetArea)
			dockArea[wId] = "Bottom";
	}
}


FMHyphenator* typotek::getHyphenator() const
{
	return hyphenator;
}

//void typotek::slotSwitchLayOptVisible()
//{
//	if(FMLayout::getLayout()->optionDialog->isVisible())
//		FMLayout::getLayout()->optionDialog->setVisible(false);
//	else
//		FMLayout::getLayout()->optionDialog->setVisible(true);
//	slotUpdateLayOptStatus();
//}

//void typotek::slotUpdateLayOptStatus()
//{
//	if(FMLayout::getLayout()->optionDialog->isVisible())
//		layOptAct->setChecked(true);
//	else
//		layOptAct->setChecked(false);
//}

#ifdef HAVE_PYTHONQT
void typotek::slotSwitchScriptConsole()
{
	if(FMScriptConsole::getInstance()->isVisible())
		FMScriptConsole::getInstance()->setVisible(false);
	else
		FMScriptConsole::getInstance()->setVisible(true);
	slotUpdateScriptConsoleStatus();
}

void typotek::slotUpdateScriptConsoleStatus()
{
	if(FMScriptConsole::getInstance()->isVisible())
		scriptConsoleAct->setChecked(true);
	else
		scriptConsoleAct->setChecked(false);
}
#else
void typotek::slotSwitchScriptConsole(){}
void typotek::slotUpdateScriptConsoleStatus(){}
#endif

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

void typotek::slotEditPanose()
{
	if(theMainView->selectedFont())
	{
		FMPanoseDialog dia(theMainView->selectedFont(), this);
		dia.exec();
		if(dia.getOk() && ( dia.getSourcePanose() != dia.getTargetPanose() ))
		{
// 			qDebug()<< "Update Panose"<<theMainView->selectedFont()->path();
			FMFontDb::DB()->setValue(theMainView->selectedFont()->path(), FMFontDb::Panose, dia.getTargetPanose());
//			theMainView->slotInfoFont();
		}
	}
}

void typotek::slotDumpInfo()
{
	if(theMainView->selectedFont())
	{
		FMDumpDialog dia(theMainView->selectedFont(), this);
		if(dia.exec() != QDialog::Accepted)
		{
			qDebug()<< "Dump not saved";
		}
	}

}

void typotek::slotReloadFiltered()
{
	FontItem * cf(theMainView->selectedFont());
	QString cfName;
	if(cf)
		cf->path();

	QStringList toReload;
	QApplication::changeOverrideCursor(Qt::WaitCursor);
	QMap<QString, QStringList> tagsRec;
	FMFontDb *db(FMFontDb::DB());
	foreach(FontItem* f, theMainView->curFonts())
	{
		toReload << f->path();
		tagsRec[f->path()] = f->tags();
		db->Remove(f->path());
	}
	QList<FontItem*> renewedFonts;
	foreach(QString p, toReload)
	{
		FontItem * it(db->Font(p, true));
		if(it)
		{
			renewedFonts << it;
		}
	}
	db->TransactionBegin();
	foreach(FontItem* it, renewedFonts)
	{
		it->setTags(tagsRec[it->path()]);
	}
	db->TransactionEnd();

	if(toReload.count() > renewedFonts.count())
	{
		//! Number of fonts we failed to reload
		showStatusMessage(tr("Failed to reload %n fonts", "", toReload.count() - renewedFonts.count()));
//		theMainView->slotReloadFontList();
	}
	if(!cfName.isEmpty())
	{
//			theMainView->forceReloadSelection();
			theMainView->slotFontSelectedByName(cfName);
	}

	QApplication::restoreOverrideCursor();

}

void typotek::slotReloadSingle()
{
	FontItem * cf(theMainView->selectedFont());
	if(cf)
	{
		QString curName(cf->path());
		QStringList t(cf->tags());
		FMFontDb::DB()->Remove(curName);
		cf = FMFontDb::DB()->Font(curName, true);
		if(cf)
		{
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

void typotek::slotMatchRaster()
{
	FMMatchRaster mr(this);
	mr.exec();
}


#ifdef HAVE_PYTHONQT
void typotek::slotExecScript()
{
	lastScript = QFileDialog::getOpenFileName(this,"Fontmatrix",QDir::homePath(),tr("Python scripts (*.py)"));
	if(!lastScript.isEmpty())
	{
		if((recentScripts.count() < MAX_RECENT_PYSCRIPTS) && (!recentScripts.values().contains(lastScript)))
		{
			QFileInfo fInfo(lastScript);
			QAction * sca (new QAction(fInfo.baseName(), this));
			recentScripts[sca] = lastScript;
			connect(sca, SIGNAL(triggered()), this, SLOT(slotExecRecentScript()));
			scriptMenu->addAction(sca);
		}
		FMPythonW::getInstance()->runFile(lastScript);
	}
	else
		qDebug()<<"Error: Script path empty";
}
void typotek::slotExecLastScript()
{
	if(!lastScript.isEmpty())
	{
		FMPythonW::getInstance()->runFile(lastScript);
	}
	else
		qDebug()<<"Error: Script path empty";
}
void typotek::slotExecRecentScript()
{
	if(sender())
	{
		QAction * sca = reinterpret_cast<QAction*>(sender());
		if(sca)
		{
			if(recentScripts.contains(sca))
			{
				lastScript = recentScripts[sca];
				FMPythonW::getInstance()->runFile(lastScript);
			}
		}
	}
}
#else
void typotek::slotExecScript(){}
void typotek::slotExecLastScript(){}
void typotek::slotExecRecentScript(){}
#endif

void typotek::showToltalFilteredFonts()
{
	countFilteredFonts->setText( tr( "Filtered Font(s): %n", "number of filtererd fonts showed in status bar", FMFontDb::DB()->countFilteredFonts() ) );
}

void typotek::presentFontName(QString s)
{
	curFontPresentation->setText(QString("%1 : <b>%2</b>")
				     .arg(tr("Current Font", "followed by currently selected font name (in status bar)"))
				     .arg(s));
}


int typotek::getPanoseMatchTreshold() const
{
	return panoseMatchTreshold;
}


void typotek::setPanoseMatchTreshold ( int theValue )
{
	panoseMatchTreshold = theValue;
}


QString typotek::getWebBrowser() const
{
	return webBrowser;
}


void typotek::setWebBrowser ( const QString& theValue )
{
	webBrowser = theValue;
}


QString typotek::getWebBrowserOptions() const
{
	return webBrowserOptions;
}


void typotek::setWebBrowserOptions ( const QString& theValue )
{
	webBrowserOptions = theValue;
}

void typotek::hide()
{
	foreach(const QString& k, dockWidget.keys())
	{
		dockVisible[k] = dockWidget[k]->isVisible();
		dockWidget[k]->hide();
	}
	visibleFloatingWidgets.clear();
	foreach(FloatingWidget* f, FloatingWidgetsRegister::AllWidgets())
	{
		visibleFloatingWidgets[f] = f->isVisible();
		f->setVisible(false);
	}

	playVisible = PlayWidget::getInstance()->isVisible();

	QMainWindow::hide();
}

void typotek::show()
{
	foreach(const QString& k, dockWidget.keys())
	{
		dockWidget[k]->setVisible(dockVisible[k]);
	}
	foreach(FloatingWidget *f, visibleFloatingWidgets.keys())
	{
		f->setVisible(visibleFloatingWidgets[f]);
	}

	PlayWidget::getInstance()->setVisible(playVisible);

	QMainWindow::show();
}

void typotek::setInfoStyle ( const QString& theValue )
{
	infoStyle = theValue;
}

QString typotek::word(FontItem * item, const QString& alt)
{
	if(item)
	{
		QString word(m_theWord);
		if(word.isEmpty() && !alt.isEmpty())
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
	playAction->setChecked( PlayWidget::getInstance()->isVisible() );
	compareAction->setChecked(FontCompareWidget::getInstance()->isVisible());

	viewMenu->removeAction(closeAllFloat);
	viewMenu->removeAction(showAllFloat);
	viewMenu->removeAction(hideAllFloat);
	viewMenu->removeAction(floatSep);

	QList<FloatingWidget*> fwl(FloatingWidgetsRegister::AllWidgets());
	foreach(FloatingWidget* f, floatingWidgets.keys())
	{
		if(!fwl.contains(f))
		{
			viewMenu->removeAction(floatingWidgets[f]);
			floatingWidgets.remove(f);
		}
	}

	foreach(FloatingWidget* f, fwl)
	{
		if(floatingWidgets.contains(f))
		{
			floatingWidgets[f]->setChecked(f->isVisible());
		}
		else
		{
			QAction *wa(new QAction(f->getActionName(),this));
			wa->setCheckable(true);
			connect(f, SIGNAL(visibilityChange()), this, SLOT(updateFloatingStatus()));
			connect(wa, SIGNAL(triggered(bool)), f, SLOT(activate(bool)));
			floatingWidgets.insert(f,  wa);
			floatingWidgets[f]->setChecked(f->isVisible());
			viewMenu->addAction(wa);
		}
	}
	if(floatingWidgets.count() > 0)
	{
		viewMenu->addAction(floatSep);
		viewMenu->addAction(closeAllFloat);
	}
	if(floatingWidgets.count() > 1)
	{
		viewMenu->addAction(showAllFloat);
		viewMenu->addAction(hideAllFloat);
	}
}


void typotek::closeAllFloatings()
{
	foreach(FloatingWidget* f, floatingWidgets.keys())
	{
		f->close();
	}
}

void typotek::showAllFloatings()
{
	foreach(FloatingWidget* f, floatingWidgets.keys())
	{
		f->setVisible(true);
	}
}

void typotek::hideAllFloatings()
{
	foreach(FloatingWidget* f, floatingWidgets.keys())
	{
		f->setVisible(false);
	}
}


void typotek::toggleMainView(bool v)
{
	if(v)
		mainStack->setCurrentWidget(theBrowser);
	else
		mainStack->setCurrentWidget(theMainView);
}

void typotek::pushObject(QObject *o)
{
	o->moveToThread(sender()->thread());
}

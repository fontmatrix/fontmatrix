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
#include "tagseteditor.h"
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
#include "fmrepair.h"
// #include "fmprintdialog.h"
#include "fmactivate.h"
#include "fmlayout.h"

#include "winutils.h"


#include <QtGui>
#include <QTextEdit>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDir>
// #include <QDBusConnection>
#include <QProgressDialog>
#include <QDomDocument>
#include <QProcess>
#include <QDockWidget>
// #include <QMutex>
// #include <QWaitCondition>

#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif


QStringList typotek::tagsList;
typotek* typotek::instance = 0;
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
			qDebug() << "[exploreDirs] - " + dir.absolutePath() + "/" + dirEntry;
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
		DockPosition["Left"] = Qt::LeftDockWidgetArea;
		DockPosition["Right"]= Qt::RightDockWidgetArea;
		DockPosition["Top"]= Qt::TopDockWidgetArea;
		DockPosition["Bottom"]= Qt::BottomDockWidgetArea;
	}
}

/// *****************************************************

typotek::typotek()
{
	instance = this;
	setWindowTitle ( "Fontmatrix" );
	setupDrop();
	qDebug()<<"Policy"<<sizePolicy().horizontalPolicy();

	hyphenator = 0;
}

void typotek::initMatrix()
{
	qDebug()<<"initMatrix()";
	m_defaultSampleName = tr("default") ;
	fontmatrix::fillDockPos();

	readSettings();
	checkOwnDir();
	initDir();

	theMainView = new MainViewWidget ( this );
	setCentralWidget ( theMainView );

	if ( QSystemTrayIcon::isSystemTrayAvailable() )
		systray = new Systray();
	else
		systray = 0;


	QFont statusFontFont ( "sans-serif",8,QFont::Bold,false );
	curFontPresentation = new QLabel ( tr ( "Nothing Selected" ) );
	curFontPresentation->setAlignment ( Qt::AlignRight );
	curFontPresentation->setFont ( statusFontFont );
	statusBar()->addPermanentWidget ( curFontPresentation );

	mainDock = new QDockWidget ( tr ( "Browse Fonts" ) );
	mainDock->setWidget ( ListDockWidget::getInstance() );
	addDockWidget ( fontmatrix::DockPosition[mainDockArea], mainDock );


	createActions();
	createMenus();
	createStatusBar();
	doConnect();

	theMainView->resetCrumb();

	if(!hyphenator)
	{
		QSettings st;
		QString dP( st.value("HyphenationDict", "hyph.dic").toString() );
		if(QFileInfo(dP).exists())
		{
			hyphenator = new FMHyphenator();
			if (!hyphenator->loadDict(dP, st.value("HyphLeft", 2).toInt(), st.value("HyphRight", 3).toInt())) {
				st.setValue("HyphenationDict", "");
				st.setValue("HyphLeft", 2);
				st.setValue("HyphRight", 3);
			}
		}
		else
		{
			hyphenator = new FMHyphenator(); // init the hyphenator anyway for the prefs
			st.setValue("HyphenationDict", "");
			st.setValue("HyphLeft", 2);
			st.setValue("HyphRight", 3);
			qDebug()<<"Err H"<<dP;
		}
	}

}




void typotek::doConnect()
{
	// later ?

	if(getSystray())
		connect ( FMActivate::getInstance() ,SIGNAL ( activationEvent ( QString ) ), getSystray(),SLOT ( updateTagMenu ( QString ) ) );

	connect(mainDock,SIGNAL(dockLocationChanged( Qt::DockWidgetArea )),this,SLOT(slotMainDockAreaChanged(Qt::DockWidgetArea )));

}


void typotek::closeEvent ( QCloseEvent *event )
{
	QSettings settings ;
	if ( systray )
	{
		if ( systray->isVisible() && settings.value ( "SystrayCloseToTray", true ).toBool() )
		{
			if ( !settings.value ( "SystrayCloseNoteShown", false ).toBool() )
			{
				QMessageBox::information ( this, tr ( "Fontmatrix" ),
				                           tr ( "The program will keep running in the "
				                                "system tray. To terminate the program, "
				                                "choose <b>Exit</b> in the context menu "
				                                "of the system tray entry." ) );
				settings.setValue ( "SystrayCloseNoteShown", true );
			}
			hide();
			event->ignore();
			return;
		}
	}
	if ( maybeSave() )
	{
		save();
		writeSettings();
		event->accept();
	}
	else
	{
		event->ignore();
	}
}


/// IMPORT
// if announce == true user will be shown a dialog of imported fonts
// if announce == false and collect == true all fonts imported will be
// collected and announced next time announce == true
void typotek::open(QString path, bool announce, bool collect)
{
	static QStringList nameList;
	static QStringList tali; // tali gets reseted when announce = true then the shouldAskTali is also set to true
	static bool shouldAskTali = true; // initial tags is only asked once if collect == true
	QStringList pathList;

	QFileInfo finfo(path);
	if (finfo.isDir() || path.isEmpty()) { // importing a directory
		static QSettings settings;
		static QString dir = settings.value("LastUsedFolder", QDir::homePath()).toString(); // first time use the home path then remember the last used dir
		QDir d(dir);
		if (!d.exists())
			dir = QDir::homePath();

		QString tmpdir;

		if(!path.isEmpty())
			tmpdir = path;
		else
			tmpdir = QFileDialog::getExistingDirectory ( this, tr ( "Add Directory" ), dir  ,  QFileDialog::ShowDirsOnly );

		if ( tmpdir.isEmpty() )
			return; // user choose to cancel the import process

		dir = tmpdir; // only set dir if importing wasn't cancelled
		settings.setValue("LastUsedFolder", dir);

		QDir theDir ( dir );
	// 	addFcDirItem(theDir.absolutePath());

		QStringList dirList( fontmatrix::exploreDirs(dir,0) );
	// 	qDebug() << dirList.join ( "\n" );

		QStringList yetHereFonts;
		for(int i=0;i < fontMap.count() ; ++i)
			yetHereFonts << fontMap[i]->path();

		QStringList filters;
		filters << "*.otf" << "*.pfb" << "*.ttf" ;
		foreach ( QString dr, dirList )
		{
			QDir d ( dr );
			QFileInfoList fil= d.entryInfoList ( filters );
			foreach ( QFileInfo fp, fil )
			{
				if(!yetHereFonts.contains(fp.absoluteFilePath()))
					pathList <<  fp.absoluteFilePath();
			}
		}
	} else if (finfo.isFile())
		pathList <<  finfo.absoluteFilePath();

	/* Everybody say it’s useless...
		NO IT'S NOT. I'm a keen fan of this feature. Let's make it optional */
	if ( useInitialTags && shouldAskTali )
	{
		ImportTags imp(this,tagsList);
		imp.exec();
		tali = imp.tags();
		shouldAskTali = false;
	}


	for(int i = 0; i < tali.count();++i )
	{
		if ( !tagsList.contains ( tali[i] ) )
		{
			tagsList.append ( tali[i] );
			emit tagAdded ( tali[i] );
		}
	}

	QProgressDialog progress ( tr ( "Importing font files... " ), tr ( "cancel" ), 0, pathList.count(), this );
	bool showProgress = pathList.count() > 1;
	if (showProgress) { // show progress bar only if there's more than one font
		progress.setWindowModality ( Qt::WindowModal );
		progress.setAutoReset ( false );
		progress.setValue ( 0 );
		progress.show();
	}
	QString importstring ( tr ( "Import" ) +  " %1" );
	for ( int i = 0 ; i < pathList.count(); ++i )
	{
		QString pathCur(pathList.at ( i ));
		if (showProgress) {
			progress.setLabelText ( importstring.arg ( pathCur ) );
			progress.setValue ( i );
			if ( progress.wasCanceled() )
				break;
		}

		if(temporaryFonts.contains(pathCur))
		{
			FontItem *fitem(temporaryFonts.value(pathCur));
			fitem->unLock();
			fitem->setTags(tali);
			temporaryFonts.remove(pathCur);
			if (announce || collect)
				nameList << fitem->fancyName();
			continue;
		}

		QFile ff ( pathCur);
		QFileInfo fi ( pathCur );
		{
			FontItem *fitem = new FontItem ( fi.absoluteFilePath() );
			if ( fitem->isValid() )
			{
				fitem->setTags ( tali );
				fitem->setActivated(false);
				fontMap.append ( fitem );
				realFontMap[fitem->path() ] = fitem;
				if (announce || collect)
					nameList << fitem->fancyName();
			}
			else
			{
				QString errorFont ( tr ( "Can’t import this font because it’s broken :" ) +" "+fi.fileName() );
				statusBar()->showMessage ( errorFont );
				if (announce || collect)
					nameList << "__FAILEDTOLOAD__" + fi.fileName();
			}
		}
	}

	progress.close();

	if (announce) {
		if (showFontListDialog) {
			// The User needs and deserves to know what fonts hve been imported
			ImportedFontsDialog ifd ( this, nameList );
			ifd.exec();
		} else { // show info in the statusbar
			statusBar()->showMessage(tr("Fonts imported: %1").arg(nameList.count()), 3000);
		}
		nameList.clear();
		tali.clear();
		shouldAskTali = true;
	}
	theMainView->slotReloadFontList();
	
	save();
}

void typotek::open ( QStringList files )
{
	QStringList pathList = files;
	QStringList nameList;
	QStringList tali;
	if ( useInitialTags )
	{
		ImportTags imp(this,tagsList);
		imp.exec();
		tali = imp.tags();
		tali << "Activated_Off" ;
	}
	else
		tali << "Activated_Off" ;

	foreach ( QString tas, tali )
	{
		if ( !tagsList.contains ( tas ) )
		{
			tagsList.append ( tas );
			emit tagAdded ( tas );
		}
	}


	for(int i=0;i < fontMap.count() ; ++i)
	{
		if(pathList.contains( fontMap[i]->path()))
			pathList.removeAll(fontMap[i]->path());

	}

	QProgressDialog progress ( tr ( "Importing font files... " ),tr ( "cancel" ), 0, pathList.count(), this );
	progress.setWindowModality ( Qt::WindowModal );
	progress.setAutoReset ( false );

	QString importstring ( tr ( "Import" ) +" %1" );
	for ( int i = 0 ; i < pathList.count(); ++i )
	{
		progress.setLabelText ( importstring.arg ( pathList.at ( i ) ) );
		progress.setValue ( i );
		if ( progress.wasCanceled() )
			break;

		QFile ff ( pathList.at ( i ) );
		QFileInfo fi ( pathList.at ( i ) );

		FontItem *fitem = new FontItem ( fi.absoluteFilePath() );
		if ( fitem->isValid() )
		{
			fitem->setTags ( tali );
			fontMap.append ( fitem );
			realFontMap[fitem->path() ] = fitem;
			nameList << fitem->fancyName();
		}
		else
		{
			QString errorFont ( tr ( "Can’t import this font because it’s broken :" ) +" "+fi.fileName() );
			statusBar()->showMessage ( errorFont );
			nameList << "__FAILEDTOLOAD__" + fi.fileName();
		}
	}

	progress.close();

	// The User needs and deserves to know what fonts hve been imported
	if (showFontListDialog) {
		// The User needs and deserves to know what fonts hve been imported
		ImportedFontsDialog ifd ( this, nameList );
		ifd.exec();
	} else { // show info in the statusbar
		statusBar()->showMessage(tr("Fonts imported: %1").arg(nameList.count()), 3000);
	}

	theMainView->slotReloadFontList();
	save();
}

/// Neede at least for the "Browse Font Dirs" feature
// It will be a regular import, as for system fonts the item will be locked.
bool typotek::insertTemporaryFont(const QString & path)
{
	// basic check
	if(path.isEmpty())
		return false;

	QFileInfo fi ( path );
	QString absPath ( fi.absoluteFilePath() );
	// check if we have it yet
	for(int i=0;i < fontMap.count() ; ++i)
	{
		if(fontMap[i]->path() == absPath)
			return true;
	}

	// Build an item
	FontItem *item(new FontItem(absPath));
	if(!item->isValid())
	{
		delete item;
		return false;
	}
	fontMap.append ( item );
	realFontMap[ item->path() ] = item;
	temporaryFonts[ item->path() ] = item;
	item->lock();

	return true;
}


/// EXPORT
void typotek::slotExportFontSet()
{
	QStringList items ( tagsList );
	items.removeAll ( "Activated_On" );
	items.removeAll ( "Activated_Off" );
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
	SaveData saver ( &fontsdata, this );
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

	saveAct = new QAction ( tr ( "&Save" ), this );
	saveAct->setShortcut ( tr ( "Ctrl+S" ) );
	saveAct->setStatusTip ( tr ( "Save the document to disk" ) );
	scuts->add(saveAct);
	connect ( saveAct, SIGNAL ( triggered() ), this, SLOT ( save() ) );

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
	scuts->add(helpAct);
	connect ( helpAct,SIGNAL ( triggered( ) ),this,SLOT ( help() ) );

	tagsetAct = new QAction ( tr ( "&Tag Sets" ),this );
	tagsetAct->setIcon ( QIcon ( ":/fontmatrix_tagseteditor_icon.png" ) );
	scuts->add(tagsetAct);
	connect ( tagsetAct,SIGNAL ( triggered( ) ),this,SLOT ( popupTagsetEditor() ) );

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
	editMenu->addAction ( tagsetAct );
	editMenu->addSeparator();
	editMenu->addAction( tagAll );
	editMenu->addAction ( activCurAct );
	editMenu->addAction ( deactivCurAct );
	editMenu->addSeparator();
	editMenu->addAction ( fonteditorAct );
	editMenu->addAction( repairAct );
	editMenu->addSeparator();
	editMenu->addAction ( prefsAct );

	editMenu->addSeparator();
	editMenu->addAction(layOptAct);

	browseMenu = menuBar()->addMenu(tr("&Browse"));
	browseMenu->addAction(nextFamily);
	browseMenu->addAction(previousFamily);
	browseMenu->addAction(nextFont);
	browseMenu->addAction(previousFont);

	helpMenu = menuBar()->addMenu ( tr ( "&Help" ) );
	helpMenu->addAction ( helpAct );
	helpMenu->addAction ( aboutAct );
	helpMenu->addAction ( aboutQtAct );

}


void typotek::createStatusBar()
{
	statusBar()->showMessage ( tr ( "Ready" ) );
}

void typotek::readSettings()
{
	relayStartingStepIn(tr("Load settings"));
	QSettings settings;
	QPoint pos = settings.value ( "pos", QPoint ( 200, 200 ) ).toPoint();
	QSize size = settings.value ( "size", QSize ( 400, 400 ) ).toSize();
	resize ( size );
	move ( pos );
	fonteditorPath = settings.value ( "FontEditor", "/usr/bin/fontforge" ).toString();
	useInitialTags = settings.value ( "UseInitialTags", false ).toBool();
	showFontListDialog = settings.value("ShowImportedFonts", true).toBool();
	templatesDir = settings.value ( "TemplatesDir", "./").toString();
	previewSize = settings.value("PreviewSize", 15.0).toDouble();
	previewRTL = settings.value("PreviewRTL", false).toBool();
	mainDockArea = settings.value("ToolPos", "Left").toString();
	m_familySchemeFreetype = settings.value("FamilyPreferred", true).toBool();
	m_welcomeURL = settings.value("WelcomeURL").toString();
	defaultOTFScript = settings.value("OTFScript").toString();
	defaultOTFLang = settings.value("OTFLang").toString();
	defaultOTFGPOS = settings.value("OTFGPOS").toString().split(";",QString::SkipEmptyParts);
	defaultOTFGSUB = settings.value("OTFGSUB").toString().split(";",QString::SkipEmptyParts);
}

void typotek::writeSettings()
{
	QSettings settings ;
	settings.setValue ( "pos", pos() );
	settings.setValue ( "size", size() );
	settings.setValue( "ToolPos", mainDockArea );
	settings.setValue( "SplitterViewState", theMainView->splitterState(SPLITTER_VIEW_1));

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
	qDebug()<<"checkOwnDir()";
	relayStartingStepIn(tr("Check for Fontmatrix own dir"));
	QString fontmanaged ( "/.fontmatrix" ); // Where activated fonts are sym-linked
	managedDir.setPath ( QDir::homePath() + fontmanaged );
	if ( !managedDir.exists() )
		managedDir.mkpath ( QDir::homePath() + fontmanaged );

	addFcDirItem( managedDir.absolutePath() );
	fontsdata.setFileName ( QDir::homePath() + "/.fontmatrix.data" );
	QSettings settings;
	m_remoteTmpDir = settings.value("RemoteTmpDir", QDir::tempPath()).toString();
}

void typotek::addFcDirItem(const QString & dirPath)
{
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
}

QStringList typotek::getSystemFontDirs()
{
	QStringList retList;
#ifdef HAVE_FONTCONFIG // For Unices (OSX included)
	FcConfig* FcInitLoadConfig();
	FcStrList *sysDirList = FcConfigGetFontDirs(0);
	QString sysDir( (char*)FcStrListNext(sysDirList) );
	while(!sysDir.isEmpty())
	{
		if(!sysDir.contains("fontmatrix"))
		{
			qDebug()<< "SYSDIR"<<sysDir;
			retList << sysDir;
		}
		sysDir = ( (char*)FcStrListNext(sysDirList) );
	}
#endif //HAVE_FONTCONFIG
#if _WIN32
	retList << getWin32SystemFontDir();
#endif // _WIN32
	return retList;
}

void typotek::initDir()
{
	qDebug() <<"initDir()";
	DataLoader loader ( &fontsdata );
	int lRes(loader.load());
	if( lRes == FONTDATA_VERSION_MISMATCH )
	{
		QMessageBox::warning ( this, tr("Fontmatrix - data warning"),
				       tr("Your database has not been loaded because of a version mismatch.\nIt is not a problem, you just lost fonts references, tags & sample texts. If you badly need to keep these datas, do not quit Fontmatrix before you have copied the database*. At this point and with a minimum of XML skill, you should be able to get old database into new format.\n\n*database is") + QString(" " +QDir::homePath()+ QString(QDir::separator()) + ".fontmatrix.data"),
	     QMessageBox::Ok, QMessageBox::NoButton );
	}
	/// load font files
	qDebug() <<"load font files";
// 	QStringList pathList = loader.fontList();
	QMap<QString,FontLocalInfo> pathList = loader.fastList();
	int fontnr = pathList.count();

	relayStartingStepIn ( tr ( "Loading" ) +" "+ QString::number ( fontnr ) +" "+tr ( "fonts present in database" ) );

	QMap<QString,FontLocalInfo>::const_iterator pit;
	for ( pit = pathList.begin(); pit != pathList.end(); ++ pit )
	{
		FontItem *fi = new FontItem ( pit.value().file, false, true );
		if ( !fi->isValid() )
		{
			qDebug() << "ERROR loading : " << pit.value().file ;
			continue;
		}
		fi->fileLocal ( pit.value() );
		fi->unLock();
		if ( tagsMap.value ( fi->path() ).contains ( "Activated_On" ) )
			fi->setActivated ( true );
		fontMap.append ( fi );
		realFontMap[fi->path() ] = fi;
		fi->setTags ( tagsMap.value ( fi->path() ) );
// 			relayStartingStepIn(zigouigoui.at( i % 8 ) );
// 			relayStartingStepIn( QString::number( fontnr - i ) );
	}
// 	}
// 	qDebug() <<  fontMap.count() << " font files loaded.";


	/// let’s load system fonts

	QString SysColFon = tr ( "Collected System Font" );
	if ( !tagsList.contains ( SysColFon ) )
		tagsList << SysColFon;

	int sysCounter ( 0 );

	QStringList sysDir ( getSystemFontDirs() );
	for ( int sIdx ( 0 ); sIdx < sysDir.count(); ++sIdx )
	{
		QDir theDir ( sysDir[sIdx] );
		QStringList syspathList;
		QStringList nameList;

		QStringList dirList ( fontmatrix::exploreDirs ( theDir,0 ) );

		QStringList yetHereFonts;
		for ( int i=0;i < fontMap.count() ; ++i )
			yetHereFonts << fontMap[i]->path();

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
			}
		}

		int sysFontCount ( syspathList.count() );
		relayStartingStepIn ( tr ( "Adding" ) +" "+ QString::number ( sysFontCount ) +" "+tr ( "fonts from system directories" ) );
		for ( int i = 0 ; i < sysFontCount; ++i )
		{
			QFile ff ( syspathList.at ( i ) );
			QFileInfo fi ( syspathList.at ( i ) );
			{
				FontItem *fitem = new FontItem ( fi.absoluteFilePath(), false, false );
				if ( fitem->isValid() )
				{
					fitem->lock();
					fitem->setActivated ( true );
					fitem->addTag ( SysColFon );
					fontMap.append ( fitem );
					realFontMap[fitem->path() ] = fitem;
					++sysCounter;
				}
				else
				{
					qDebug() << "Can’t open this font because it’s broken : " << fi.fileName() ;
				}
			}
		}
	}
	relayStartingStepIn ( QString::number ( sysCounter ) + " " + tr ( "fonts available from system" ) );


// 	qDebug()<<"TIME(fonts) : "<<fontsTime.elapsed();
	/// Remote dirs
	//TODO
	QSettings settings;
	QStringList remoteDirV ( settings.value ( "RemoteDirectories" ).toStringList() );
	if ( !remoteDirV.isEmpty() )
	{
		relayStartingStepIn ( tr ( "Catching" ) +" "+ QString::number ( remoteDirV.count() ) +" "+tr ( "font descriptions from network" ) );
		remoteDir = new RemoteDir ( remoteDirV );
		connect ( remoteDir,SIGNAL ( listIsReady() ),this,SLOT ( slotRemoteIsReady() ) );
		remoteDir->run();
	}
}

static bool slotRemoteIsReadyRunOnce = false;
void typotek::slotRemoteIsReady()
{
	if(!slotRemoteIsReadyRunOnce)
		slotRemoteIsReadyRunOnce = true;
	else
		return;

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
		fontMap.append ( fi );
		realFontMap[fi->path() ] = fi;
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



QList< FontItem * > typotek::getFonts ( QString pattern, QString field )
{

	if ( pattern.isEmpty() )
	{
		theMainView->resetCrumb();
		return fontMap;
	}

	bool negate ( false );
	QString rPattern ( pattern );
	if ( pattern.startsWith ( "!" ) )
	{
		negate = true;
		rPattern = pattern.mid ( 1 );
	}

	QList< FontItem * > ret;
	ret.clear();
	QList< FontItem * > superSet ( theMainView->curFonts() ) ;

	if ( superSet.isEmpty() )
	{
		theMainView->resetCrumb();
		superSet = fontMap;
	}

	theMainView->addFilterToCrumb ( pattern );
	int superSetCount ( superSet.count() );

	qDebug() <<"PATERN ="<< rPattern<<": FIELD ="<< field<<":"<< superSetCount;

	if ( field == "tag" )
	{
		for ( int i =0; i < superSetCount; ++i )
		{
			if ( superSet[i]->tags().contains ( rPattern ) )
			{
// 				qDebug()<< "TAG MATCH"<<superSet[i]->family();
				ret.append ( superSet[i] );
			}
		}
	}
	else if( field.startsWith("Panose/") )
	{
		QString sf(field.mid(7));
		qDebug()<<"PANOSE"<< sf<< rPattern;
		for ( int i =0; i < superSetCount; ++i )
		{
			if ( superSet[i]->panose(sf) == rPattern )
			{
				ret.append ( superSet[i] );
			}
		}
	}
	else if(field == tr ( "Unicode character" ))
	{
		/// WARNING - Unicode fields does not support negation.
		int startC(0xFFFFFFFF);
		int endC(0);
		int patCount(rPattern.count());
		for(int a(0); a < patCount; ++a)
		{
			unsigned int ca(rPattern[a].unicode());
			if( ca < startC)
				startC = ca;
			if(ca > endC)
				endC = ca;
		}
		for ( int i =0; i < superSetCount; ++i )
		{
			int cc(superSet[i]->countCoverage ( startC, endC ) );
			if ( cc >= patCount )
			{
				qDebug()<<"U U+ fam r"<< startC<< endC<<superSet[i]->family()<<cc;
				ret.append ( superSet[i] );
			}
		}
	}
	else if ( field == tr ( "All fields" ) )
	{
		for ( int i =0; i < superSetCount; ++i )
		{
			if ( negate ?
			        ( !superSet[i]->infoText().contains ( rPattern,Qt::CaseInsensitive ) ) :
						  ( superSet[i]->infoText().contains (rPattern,Qt::CaseInsensitive ) ) )
			{
				ret.append ( superSet[i] );
			}
		}
	}
	else
	{
		for ( int i =0; i < superSetCount; ++i )
		{
			if ( negate ?
			        ( !superSet[i]->value ( field ).contains ( rPattern , Qt::CaseInsensitive ) ) :
						  ( superSet[i]->value ( field ).contains ( rPattern , Qt::CaseInsensitive ) ) )
			{
				ret.append ( superSet[i] );
			}
		}
	}

	qDebug() <<"RET"<< ret.count();
	return ret;
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

void typotek::popupTagsetEditor()
{
	TagSetEditor ed(this);
	connect ( &ed,SIGNAL ( signalNewTagset() ),theMainView,SLOT ( slotReloadTagsetList() ) );
	ed.exec();
	disconnect ( &ed,SIGNAL ( signalNewTagset() ),theMainView,SLOT ( slotReloadTagsetList() ) );
	theMainView->slotReloadTagsetList();
}

void typotek::keyPressEvent ( QKeyEvent * event )
{
	qDebug() << "typotek::keyPressEvent(QKeyEvent * "<<event<<")";
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

void typotek::help()
{
	HelpWidget theHelp ( this );
	theHelp.exec();
}

FontItem* typotek::getFont ( QString s )
{
	if ( realFontMap.contains ( s ) )
	{
		return realFontMap.value ( s );
	}
	return 0;
}

FontItem* typotek::getFont ( int i )
{
	if ( i < fontMap.count() && i >= 0 )
	{
		return fontMap.at ( i );
	}
	else
	{
		return 0;
	}
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
// 			// We remove all temporary fonts
// 			// a bit rough, but it should work well
// 			QMap<QString,FontItem*>::const_iterator fIt;
// 			for(fIt = temporaryFonts.constBegin(); fIt != temporaryFonts.constEnd() ; ++fIt)
// 			{
// 				realFontMap.remove(fIt.key());
// 				fontMap.removeAll(fIt.value());
// 				delete fIt.value();
// 			}
// 			temporaryFonts.clear();
			open(fP);
			return;
		}
		else
		{
			if(temporaryFonts.contains(fP))
			{
				open(fP);
				temporaryFonts.remove(fP);
			}
			else
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
		open ( ret );

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
	settings.setValue ( "SystrayCloseToTray", isEnabled );
	settings.setValue ( "SystrayCloseNoteShown", false );
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
	for(int i(0); i < fontMap.count(); ++i)
		fontMap[i]->clearPreview() ;
	emit previewDirectionHasChanged();
	ListDockWidget::getInstance()->forcePreviewRefill();
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
	settings.setValue("TemplatesDir", templatesDir);

}

void typotek::changeFontSizeSettings(double fSize, double lSize)
{
	QSettings settings;
	settings.setValue("SampleFontSize", fSize);
	settings.setValue("SampleInterline", lSize);
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
	FontItem *fit = realFontMap.value(key);
	if(!fit)
		return;
	fontMap.removeAll(fit);
	delete fit;
	realFontMap.remove(key);
	qDebug()<< key << "has been removed";
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
	settings.setValue("RemoteTmpDir", m_remoteTmpDir);
}

void typotek::slotRepair()
{
	FmRepair repair(this);
	repair.exec();
}

void typotek::slotTagAll()
{
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
	theMainView->slotNewTag();
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
	dialog.setWindowTitle("Fontmatrix - " + tr("Print Infos") +" - " + font->fancyName() );

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

	QStringList stl(namedSample ( theMainView->sampleName() ).split ( '\n' ));
	QList<FontItem*> familyFonts( getFonts(theMainView->selectedFont()->family(), "family"));

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

FMHyphenator* typotek::getHyphenator() const
{
	return hyphenator;
}

void typotek::slotSwitchLayOptVisible()
{
	if(FMLayout::getLayout()->optionDialog->isVisible())
	{
		FMLayout::getLayout()->optionDialog->setVisible(false);
		layOptAct->setChecked(false);
	}
	else
	{
		FMLayout::getLayout()->optionDialog->setVisible(true);
		layOptAct->setChecked(true);
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
		st.setValue("OTFScript" , theValue);
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
		st.setValue("OTFLang" , theValue);
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
		st.setValue("OTFGPOS" , theValue.join(";"));
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
		st.setValue("OTFGSUB" , theValue.join(";"));
	}
	defaultOTFGSUB = theValue;
}

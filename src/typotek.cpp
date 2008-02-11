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
// #include "typotekadaptator.h"

#include "dataloader.h"
#include "tagseteditor.h"
#include "savedata.h"
#include "aboutwidget.h"
#include "helpwidget.h"
#include "importedfontsdialog.h"
#include "listdockwidget.h"
#include "systray.h"
#include "prefspaneldialog.h"
#include "fontbook.h"
#include "importtags.h"
#include "dataexport.h"


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

#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif


QStringList typotek::tagsList;
typotek* typotek::instance = 0;
QString typotek::fonteditorPath = "/usr/bin/fontforge";
extern bool __FM_SHOW_FONTLOADED;

typotek::typotek()
{
	instance = this;
	setWindowTitle ( "Fontmatrix" );
	setupDrop();

}

void typotek::initMatrix()
{
// 	QTime initTime;
// 	initTime.start();
	
	checkOwnDir();
	readSettings();
// 	fillTagsList();
	initDir();
	

	if ( QSystemTrayIcon::isSystemTrayAvailable() )
		systray = new Systray();
	else
		systray = 0;
	
	QSettings settings;
	previewSize = settings.value("PreviewSize", 15.0).toDouble();
	
	
	theMainView = new MainViewWidget ( this );
	setCentralWidget ( theMainView );

	mainDock = new QDockWidget ( tr ( "Lists" ) );
	mainDock->setWidget ( ListDockWidget::getInstance() );
	addDockWidget ( Qt::LeftDockWidgetArea, mainDock );

	QFont statusFontFont ( "sans-serif",8,QFont::Bold,false );
	curFontPresentation = new QLabel ( tr ( "Nothing Selected" ) );
	curFontPresentation->setAlignment ( Qt::AlignRight );
	curFontPresentation->setFont ( statusFontFont );
	statusBar()->addPermanentWidget ( curFontPresentation );



	createActions();
	createMenus();
	createStatusBar();
// 	qDebug() << "TIME(initiMatrix) : "<<initTime.elapsed();
}




void typotek::doConnect()
{
	// later ?
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

namespace fontmatrix
{
QStringList exploreDirs(const QDir &dir, int deep)
{
	static QStringList retDirList;
	if(deep > 10)
		return QStringList();	
	if(deep == 0)
		retDirList.clear();
// 	qDebug() << dir.absolutePath();
	
	retDirList << dir.absolutePath();
	QStringList localEntries(dir.entryList ( QDir::AllDirs | QDir::NoDotAndDotDot));
	foreach ( QString dirEntry, localEntries )
	{
		qDebug() << "[exploreDirs] - " + dir.absolutePath() + "/" + dirEntry;
		QDir d ( dir.absolutePath() + "/" + dirEntry );
		exploreDirs(d, deep + 1);
		if(!retDirList.contains(d.absolutePath()))
			retDirList << d.absolutePath();
	}
	
	return retDirList;
} 
}

/// IMPORT
void typotek::open()
{
	static QString dir = QDir::homePath(); // first time use the home path then remember the last used dir
	QString tmpdir = QFileDialog::getExistingDirectory ( this, tr ( "Add Directory" ), dir  ,  QFileDialog::ShowDirsOnly );

	if ( tmpdir.isEmpty() )
		return; // user choose to cancel the import process

	dir = tmpdir; // only set dir if importing wasn't cancelled
	
	QDir theDir ( dir );
// 	addFcDirItem(theDir.absolutePath());

	QStringList pathList;
	QStringList nameList;
	
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
	QStringList tali;
	/* Everybody say it’s useless...
		NO IT'S NOT. I'm a keen fan of this feature. Let's make it optional */
	if ( useInitialTags )
	{
		ImportTags imp(this,tagsList);
		imp.exec();
		tali = imp.tags();
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
	progress.setWindowModality ( Qt::WindowModal );
	progress.setAutoReset ( false );
	progress.setValue ( 0 );
	progress.show();
	QString importstring ( tr ( "Import" ) +  " %1" );
	for ( int i = 0 ; i < pathList.count(); ++i )
	{
		progress.setLabelText ( importstring.arg ( pathList.at ( i ) ) );
		progress.setValue ( i );
		if ( progress.wasCanceled() )
			break;

		QFile ff ( pathList.at ( i ) );
		QFileInfo fi ( pathList.at ( i ) );
		{
			FontItem *fitem = new FontItem ( fi.absoluteFilePath() );
			if ( fitem->isValid() )
			{
				fitem->setTags ( tali );
				fitem->setActivated(false);
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
	}

	progress.close();

	// The User needs and deserves to know what fonts hve been imported
	ImportedFontsDialog ifd ( this, nameList );
	ifd.exec();
	theMainView->slotReloadFontList();
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
	ImportedFontsDialog ifd ( this, nameList );
	ifd.exec();

	theMainView->slotReloadFontList();

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


	openAct = new QAction ( QIcon ( ":/fontmatrix_import_icon" ), tr ( "&Import..." ), this );
	openAct->setShortcut ( tr ( "Ctrl+O" ) );
	openAct->setStatusTip ( tr ( "Import a directory" ) );
	connect ( openAct, SIGNAL ( triggered() ), this, SLOT ( open() ) );

	saveAct = new QAction ( tr ( "&Save" ), this );
	saveAct->setShortcut ( tr ( "Ctrl+S" ) );
	saveAct->setStatusTip ( tr ( "Save the document to disk" ) );
	connect ( saveAct, SIGNAL ( triggered() ), this, SLOT ( save() ) );
	
	exportFontSetAct = new QAction(tr("Export &Fonts"),this);
	exportFontSetAct->setStatusTip(tr("Export a fontset"));
	connect( exportFontSetAct,SIGNAL(triggered( )),this,SLOT(slotExportFontSet()));

	printAct = new QAction ( tr ( "Print..." ),this );
	printAct->setStatusTip ( tr ( "Print a specimen of the current font" ) );
	connect ( printAct, SIGNAL ( triggered() ), this, SLOT ( print() ) );

	fontBookAct = new QAction ( QIcon ( ":/fontmatrix_fontbookexport_icon.png" ), tr ( "Export font book..." ),this );
	fontBookAct->setStatusTip ( tr ( "Export a pdf that show selected fonts" ) );
	connect ( fontBookAct, SIGNAL ( triggered() ), this, SLOT ( fontBook() ) );

	exitAct = new QAction ( tr ( "E&xit" ), this );
	exitAct->setShortcut ( tr ( "Ctrl+Q" ) );
	exitAct->setStatusTip ( tr ( "Exit the application" ) );
	connect ( exitAct, SIGNAL ( triggered() ), this, SLOT ( close() ) );


	aboutAct = new QAction ( tr ( "&About" ), this );
	aboutAct->setStatusTip ( tr ( "Show the Typotek's About box" ) );
	connect ( aboutAct, SIGNAL ( triggered() ), this, SLOT ( about() ) );

	helpAct = new QAction ( tr ( "Help" ), this );
	connect ( helpAct,SIGNAL ( triggered( ) ),this,SLOT ( help() ) );

	tagsetAct = new QAction ( tr ( "&Tag Sets" ),this );
	tagsetAct->setIcon ( QIcon ( ":/fontmatrix_tagseteditor_icon.png" ) );
	connect ( tagsetAct,SIGNAL ( triggered( ) ),this,SLOT ( popupTagsetEditor() ) );

	activCurAct = new QAction ( tr ( "Activate all current" ),this );
	connect ( activCurAct,SIGNAL ( triggered( ) ),this,SLOT ( slotActivateCurrents() ) );

	deactivCurAct = new QAction ( tr ( "Deactivate all current" ),this );
	connect ( deactivCurAct,SIGNAL ( triggered( ) ),this,SLOT ( slotDeactivateCurrents() ) );

	fonteditorAct = new QAction ( tr ( "Edit current font" ),this );
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
	connect ( prefsAct,SIGNAL ( triggered() ),this,SLOT ( slotPrefsPanelDefault() ) );

	if ( systray )
		connect ( theMainView, SIGNAL ( newTag ( QString ) ), systray, SLOT ( newTag ( QString ) ) );
}

void typotek::createMenus()
{
	fileMenu = menuBar()->addMenu ( tr ( "&File" ) );

	fileMenu->addAction ( openAct );
	fileMenu->addAction ( saveAct );
	fileMenu->addAction ( exportFontSetAct );
	fileMenu->addSeparator();
	fileMenu->addAction ( printAct );
	fileMenu->addAction ( fontBookAct );
	fileMenu->addSeparator();
	fileMenu->addAction ( exitAct );

	editMenu = menuBar()->addMenu ( tr ( "Edit" ) );
	editMenu->addAction ( tagsetAct );
	editMenu->addSeparator();
	editMenu->addAction ( activCurAct );
	editMenu->addAction ( deactivCurAct );
	editMenu->addSeparator();
	editMenu->addAction ( fonteditorAct );
	editMenu->addSeparator();
	editMenu->addAction ( prefsAct );

	helpMenu = menuBar()->addMenu ( tr ( "&Help" ) );
	helpMenu->addAction ( aboutAct );
	helpMenu->addAction ( helpAct );

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
	templatesDir = settings.value ( "TemplatesDir", "./").toString();
}

void typotek::writeSettings()
{
	QSettings settings ;
	settings.setValue ( "pos", pos() );
	settings.setValue ( "size", size() );

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
	QString fontmanaged ( "/.fontmatrix" ); // Where activated fonts are sym-linked
	managedDir.setPath ( QDir::homePath() + fontmanaged );
	if ( !managedDir.exists() )
		managedDir.mkpath ( QDir::homePath() + fontmanaged );

	addFcDirItem( managedDir.absolutePath() );
	fontsdata.setFileName ( QDir::homePath() + "/.fontmatrix.data" );
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
// 			QDomElement selectFont = root.namedItem ( "selectfont" ).toElement();
// 			if(selectFont.isNull())
// 			{
// 				QDomElement e = fc.createElement ( "selectfont" );
// 				root.appendChild ( e );
// 				selectFont = root.namedItem ( "selectfont" ).toElement();
// 			}
// 			QDomElement rejectFont = selectFont.namedItem("rejectfont").toElement();
// 			if(rejectFont.isNull())
// 			{
// 				QDomElement e = fc.createElement ( "rejectfont" );
// 				selectFont.appendChild(e);
// 				rejectFont = selectFont.namedItem("rejectfont").toElement();
// 			}
// 			QDomNodeList rejlist = rejectFont.elementsByTagName ( "glob" );
// 			bool yetRejected = false;
// 			for ( int i=0;i < rejlist.count();++i )
// 			{
// 				if ( rejlist.at ( i ).toElement().text() == dirPath )
// 					yetRejected = true;
// 			}
// 			if(!yetRejected)
// 			{
// 				QDomElement e = fc.createElement ( "glob" );
// 				QDomText t = fc.createTextNode ( dirPath + "*" );
// 				e.appendChild(t);
// 				rejectFont.appendChild(e);
// 			}
			fcfile.resize ( 0 );

			QTextStream ts ( &fcfile );
			fc.save ( ts,4 );

		}
		fcfile.close();
	}
}


void typotek::initDir()
{
	
// 	QTime loadTime;
// 	loadTime.start();
	//load data file
	DataLoader loader ( &fontsdata );
	loader.load();
// 	qDebug()<<"TIME(loader) : "<<loadTime.elapsed();
	// load font files

	QStringList pathList = loader.fontList();
// 	QTime fontsTime;
// 	fontsTime.start();
	int fontnr = pathList.count();
	if ( __FM_SHOW_FONTLOADED )
	{
		for ( int i = 0 ; i < fontnr ; ++i )
		{
		
			qDebug() << "About to load : " << pathList.at ( i );
			FontItem *fi = new FontItem (  pathList.at ( i )  );
			if(!fi->isValid())
			{
				qDebug() << "ERROR loading : " << pathList.at ( i );
				continue;
			}
			fontMap.append ( fi );
			realFontMap[fi->path() ] = fi;
			fi->setTags ( tagsMap.value ( fi->path() ) );
// 			qDebug() << fi->fancyName() << " loaded.";
		}	
	}
	else
	{
		relayStartingStepIn(tr("Loading")+" "+ QString::number(fontnr) +" "+tr("fonts present in database"));
		for ( int i = 0 ; i < fontnr ; ++i )
		{
			FontItem *fi = new FontItem ( pathList.at ( i )  );
			if(!fi->isValid())
			{
				qDebug() << "ERROR loading : " << pathList.at ( i );
				continue;
			}
			fontMap.append ( fi );
			realFontMap[fi->path() ] = fi;
			fi->setTags ( tagsMap.value ( fi->path() ) );
		}
// 	theMainView->slotOrderingChanged ( theMainView->defaultOrd() );
	}
	qDebug() <<  fontMap.count() << " font files loaded.";
	
#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>

	/// let’s load system fonts
	{
		QString SysColFon = tr("Collected System Font");
// 		QTime sysTime;
// 		sysTime.start();
		FcConfig* FcInitLoadConfig();
		FcStrList *sysDirList = FcConfigGetFontDirs(0);
		QString sysDir((char*)FcStrListNext(sysDirList));
		int sysCounter(0);
		while(!sysDir.isEmpty())
		{
			if(sysDir.contains("fontmatrix"))
			{
				sysDir = (char*)FcStrListNext(sysDirList);
				continue;
			}
			QDir theDir ( sysDir );
		
			QStringList pathList;
			QStringList nameList;
			
			QStringList dirList( fontmatrix::exploreDirs(theDir,0) );
// 			qDebug() << dirList.join ( "\n" );
			
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
			
			int sysFontCount(pathList.count());
			relayStartingStepIn(tr("Adding")+" "+ QString::number(sysFontCount) +" "+tr("fonts from system directories"));
			for ( int i = 0 ; i < sysFontCount; ++i )
			{
				QFile ff ( pathList.at ( i ) );
				QFileInfo fi ( pathList.at ( i ) );
				{
					FontItem *fitem = new FontItem ( fi.absoluteFilePath());
					if ( fitem->isValid() )
					{
						fitem->lock();
						fitem->setActivated(true);
						fitem->addTag(SysColFon);
						fontMap.append ( fitem );
						realFontMap[fitem->path() ] = fitem;
						++sysCounter;
					}
					else
					{
						qDebug()<< "Can’t open this font because it’s broken : " << fi.fileName() ;
					}
				}
			}
			
			sysDir = (char*)FcStrListNext(sysDirList);
		}
		relayStartingStepIn(QString::number(sysCounter) + " " + tr("fonts available from system"));
		if(!tagsList.contains(SysColFon))
			tagsList << SysColFon;
// 		qDebug()<<"TIME(sysfonts) : "<<sysTime.elapsed();
	}
#endif //HAVE_FONTCONFIG
// 	qDebug()<<"TIME(fonts) : "<<fontsTime.elapsed();
}

QList< FontItem * > typotek::getFonts ( QString pattern, QString field )
{
	QList< FontItem * > ret;

	for ( int i =0; i < fontMap.count(); ++i )
	{
		if ( field == "tag" )
		{
			QStringList tl =  fontMap[i]->tags();
			for ( int ii=0; ii<tl.count(); ++ii )
			{
				if ( tl[ii].contains ( pattern,Qt::CaseInsensitive ) )
					ret.append ( fontMap[i] );
			}
		}
		else if ( field == "search_INSENS" )
		{
			if ( fontMap[i]->infoText().contains ( pattern,Qt::CaseInsensitive ) )
				ret.append ( fontMap[i] );
		}
		else if ( field == "search_SENS" )
		{
			if ( fontMap[i]->infoText().contains ( pattern,Qt::CaseSensitive ) )
				ret.append ( fontMap[i] );
		}
		else
		{
			if ( fontMap[i]->value ( field ).contains ( pattern , Qt::CaseInsensitive ) )
				ret.append ( fontMap[i] );
		}
	}
	return ret;
}

void typotek::print()
{
	// TODO Provide a decent preview sample, what’s here is just useless.
	QPrinter thePrinter ( QPrinter::HighResolution );
	QPrintDialog *dialog = new QPrintDialog ( &thePrinter, this );
	dialog->setWindowTitle ( tr ( "Print specimen" ) );

	if ( dialog->exec() != QDialog::Accepted )
		return;
	thePrinter.setFullPage ( true );
	QPainter aPainter ( &thePrinter );
	theMainView->textScene()->render ( &aPainter );
// 	int maxPages = theMainView->glyphsScene()->sceneRect().height() / 600;
// 	QRectF prect = aPainter.viewport();
// 	for(int i = 0; i < maxPages ; i++)
// 	{
// 		qDebug() << "Print page " << i;
// 		QRectF prect(0, i * 600 , 300, 600);
// 		thePrinter.newPage();
// 		theMainView->glyphsScene()->render(&aPainter,prect,prect);
//
// 	}
//
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

	if ( name == "default" )
	{
		statusBar()->showMessage ( tr ( "\"default\" is a reserved keyword" ), 3000 );
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
		return m_namedSamples["default"];
	return m_namedSamples[name];
}

void typotek::setSampleText ( QString s )
{
	m_namedSamples["default"] += s;
}


void typotek::changeSample ( QString name, QString text )
{
	if ( !m_namedSamples.contains ( name ) )
		return;
	m_namedSamples[name] = text;
}

void typotek::setWord ( QString s, bool updateView )
{
	m_theWord = s;
	if ( updateView )
		theMainView->slotView ( true );
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


/// LazyInit *********************************************
void LazyInit::run()
{
	qDebug() << "LazyInit::run()";
	typotek* t = typotek::getInstance();
	QList<FontItem*> fonts = t->getAllFonts();
	foreach(FontItem *fit, fonts)
	{
		fit->infoText();
	}
	qDebug() << "END OF LazyInit";
}




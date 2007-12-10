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
#include "fontbookdialog.h"
#include "dataloader.h"
#include "tagseteditor.h"
#include "savedata.h"
#include "aboutwidget.h"
#include "helpwidget.h"
#include "importedfontsdialog.h"
#include "listdockwidget.h"
#include "systray.h"
#include "prefspaneldialog.h"


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


QStringList typotek::tagsList;
typotek* typotek::instance = 0;
extern bool __FM_SHOW_FONTLOADED;

typotek::typotek()
{
	instance = this;
	setWindowTitle ( "Fontmatrix" );
	setupDrop();
}

void typotek::initMatrix()
{
	checkOwnDir();
	readSettings();
	fillTagsList();
	initDir();


	theMainView = new MainViewWidget ( this );
	setCentralWidget ( theMainView );
	
	mainDock = new QDockWidget("Lists");
	mainDock->setWidget(ListDockWidget::getInstance());
	addDockWidget(Qt::LeftDockWidgetArea, mainDock);
	
	QFont statusFontFont("sans-serif",8,QFont::Bold,false);
	curFontPresentation = new QLabel("nothing selected");
	curFontPresentation->setAlignment(Qt::AlignRight);
	curFontPresentation->setFont(statusFontFont);
	statusBar()->addPermanentWidget(curFontPresentation);

	if (QSystemTrayIcon::isSystemTrayAvailable())
		systray = new Systray();
	else
		systray = 0;

	createActions();
	createMenus();
	createStatusBar();


// 	{
// 		actAdaptator = new TypotekAdaptator ( this );
// 		if ( !QDBusConnection::sessionBus().registerService ( "com.fontmatrix.fonts" ) )
// 			qDebug() << "unable to register to DBUS";
// 		if ( !QDBusConnection::sessionBus().registerObject ( "/FontActivation", actAdaptator, QDBusConnection::ExportAllContents ) )
// 			qDebug() << "unable to register to DBUS";
// 	}
}




void typotek::doConnect()
{
	// later ?
}


void typotek::closeEvent ( QCloseEvent *event )
{
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


void typotek::open()
{

	QStringList pathList;
	QStringList nameList;
	QStringList dirList;
	QString dir = QFileDialog::getExistingDirectory ( this, tr ( "Add Directory" ), QDir::homePath ()  ,  QFileDialog::ShowDirsOnly );
	QDir theDir ( dir );
	QStringList tmpDirL = theDir.entryList ( QDir::AllDirs );
	foreach ( QString dirEntry, tmpDirL )
	{
		QDir d ( theDir.absolutePath() + "/"+dirEntry );
		dirList << d.absolutePath();
	}
	dirList << theDir.absolutePath();
// 	qDebug() << dirList.join ( "\n" );
	QStringList filters;
	filters << "*.otf" << "*.pfb" << "*.ttf" ;
	foreach ( QString dr, dirList )
	{
		QDir d ( dr );
		QFileInfoList fil= d.entryInfoList ( filters );
		foreach ( QFileInfo fp, fil )
		{
			pathList <<  fp.absoluteFilePath();
		}
	}
	QStringList tali;
	/* Everybody say it’s useless...
		QString inputTags = QInputDialog::getText ( this,"Import tags","Initial tags.\nThe string you type will be split by \"#\" to obtain a tags list." );

		if(!inputTags.isEmpty())
			tali = inputTags.split ( "#" );*/
	tali << "Activated_Off" ;

	foreach ( QString tas, tali )
	{
		if ( !tagsList.contains ( tas ) )
		{
			tagsList.append ( tas );
			emit tagAdded ( tas );
		}
	}

	QProgressDialog progress ( "Importing font files... ", "cancel", 0, pathList.count(), this );
	progress.setWindowModality ( Qt::WindowModal );
	progress.setAutoReset(false);

	QString importstring ( "Import %1" );
	for ( int i = 0 ; i < pathList.count(); ++i )
	{
		progress.setLabelText ( importstring.arg ( pathList.at ( i ) ) );
		progress.setValue ( i );
		if ( progress.wasCanceled() )
			break;

		QFile ff ( pathList.at ( i ) );
		QFileInfo fi ( pathList.at ( i ) );


		if ( ff.copy ( ownDir.absolutePath() + "/" + fi.fileName() ) )
		{

			if ( fi.suffix() == "pfb" )
			{
				QFile fafm ( QString ( pathList.at ( i ) ).replace ( ".pfb",".afm" ) );
				QFileInfo iafm ( QString ( pathList.at ( i ) ).replace ( ".pfb",".afm" ) );
				if ( fafm.exists() )
					fafm.copy ( ownDir.absolutePath() + "/" + iafm.fileName() );
			}
			FontItem *fitem = new FontItem ( ownDir.absolutePath() + "/" + fi.fileName() );

			fitem->setTags ( tali );
			fontMap.append ( fitem );
			realFontMap[fitem->name() ] = fitem;
			nameList << fitem->fancyName();
		}
	}
	
	progress.close();
	
	// The User needs and deserves to know what fonts hve been imported
	ImportedFontsDialog ifd(nameList);
	ifd.exec();
	
	theMainView->slotReloadFontList();
	
// 	QMessageBox::information(this,"Fontmatrix info","List of imported fonts :\n" + nameList.join("\n"));
// 	theMainView->slotOrderingChanged ( theMainView->defaultOrd() );
}

void typotek::open(QStringList files)
{
	QStringList pathList = files;
	QStringList nameList;
	QStringList tali;
	tali << "Activated_Off" ;

	foreach ( QString tas, tali )
	{
		if ( !tagsList.contains ( tas ) )
		{
			tagsList.append ( tas );
			emit tagAdded ( tas );
		}
	}

	QProgressDialog progress ( "Importing font files... ", "cancel", 0, pathList.count(), this );
	progress.setWindowModality ( Qt::WindowModal );
	progress.setAutoReset(false);

	QString importstring ( "Import %1" );
	for ( int i = 0 ; i < pathList.count(); ++i )
	{
		progress.setLabelText ( importstring.arg ( pathList.at ( i ) ) );
		progress.setValue ( i );
		if ( progress.wasCanceled() )
			break;

		QFile ff ( pathList.at ( i ) );
		QFileInfo fi ( pathList.at ( i ) );


		if ( ff.copy ( ownDir.absolutePath() + "/" + fi.fileName() ) )
		{

			if ( fi.suffix() == "pfb" )
			{
				QFile fafm ( QString ( pathList.at ( i ) ).replace ( ".pfb",".afm" ) );
				QFileInfo iafm ( QString ( pathList.at ( i ) ).replace ( ".pfb",".afm" ) );
				if ( fafm.exists() )
					fafm.copy ( ownDir.absolutePath() + "/" + iafm.fileName() );
			}
			FontItem *fitem = new FontItem ( ownDir.absolutePath() + "/" + fi.fileName() );

			fitem->setTags ( tali );
			fontMap.append ( fitem );
			realFontMap[fitem->name() ] = fitem;
			nameList << fitem->fancyName();
		}
	}
	
	progress.close();
	
	// The User needs and deserves to know what fonts hve been imported
	ImportedFontsDialog ifd(nameList);
	ifd.exec();
	
	theMainView->slotReloadFontList();
	
}


bool typotek::save()
{
	SaveData saver ( &fontsdata, this );
	return true;

}


void typotek::about()
{
	AboutWidget aabout;
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

	activCurAct = new QAction ( tr ( "Activate all currents" ),this );
	connect ( activCurAct,SIGNAL ( triggered( ) ),this,SLOT ( slotActivateCurrents() ) );

	deactivCurAct = new QAction ( tr ( "Deactivate all currents" ),this );
	connect ( deactivCurAct,SIGNAL ( triggered( ) ),this,SLOT ( slotDeactivateCurrents() ) );
	
	fonteditorAct = new QAction( tr ( "Edit current font" ),this );
	fonteditorAct->setStatusTip ( tr ( "Try to run Fontforge with the selected font as argument" ) );
	connect (fonteditorAct,SIGNAL ( triggered( ) ),this,SLOT ( slotEditFont()) );
	
	prefsAction = new QAction( tr ( "Preferences" ),this );
	connect(prefsAction,SIGNAL(triggered()),this,SLOT(slotPrefsPanel()));
}

void typotek::createMenus()
{
	fileMenu = menuBar()->addMenu ( tr ( "&File" ) );

	fileMenu->addAction ( openAct );
	fileMenu->addAction ( saveAct );
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
	editMenu->addAction(prefsAction);

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
	QSettings settings ( "Undertype", "fontmatrix" );
	QPoint pos = settings.value ( "pos", QPoint ( 200, 200 ) ).toPoint();
	QSize size = settings.value ( "size", QSize ( 400, 400 ) ).toSize();
	resize ( size );
	move ( pos );


}

void typotek::writeSettings()
{
	QSettings settings ( "Undertype", "fontmatrix" );
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
	//DONE parse ~/.fonts.conf to see if there is the ~/.managed-fonts dir entry
	// and create it if it does not exist and setup a QDir("~/.managed-fonts") private member
	QString fontmanaged ( "/.fonts-managed" ); // Where activated fonts are sym-linked
	QString fontreserved ( "/.fonts-reserved" );// Where aknoweldged fonts are stored.

	managedDir.setPath ( QDir::homePath() + fontmanaged );
	if ( !managedDir.exists() )
		managedDir.mkpath ( QDir::homePath() + fontmanaged );

	QFile fcfile ( QDir::homePath() + "/.fonts.conf" );
	if ( !fcfile.open ( QFile::ReadWrite ) )
	{
// 		QMessageBox::warning ( 0, QString ( "fontmatrix" ),
// 		                       QString ( "Cannot read file %1:\n%2.\nBEFORE ANYTHING, YOU SHOULD CHECK IF FONTCONFIG IS IN USE." )
// 		                       .arg ( fcfile.fileName() )
// 		                       .arg ( fcfile.errorString() ) );
	}
	else
	{
		QDomDocument fc ( "fontconfig" );
		
		// .fonts.conf is empty, it seems that we just created it.
		// Wed have to populate it a bit
		if ( fcfile.size() == 0 )
		{
			QString ds( "<?xml version='1.0'?><!DOCTYPE fontconfig SYSTEM 'fonts.dtd'><fontconfig></fontconfig>");
			fc.setContent(ds);
		}
		else
		{
			fc.setContent ( &fcfile );
		}
		
		bool isconfigured = false;
		QDomNodeList dirlist = fc.elementsByTagName ( "dir" );
		for ( int i=0;i < dirlist.count();++i )
		{
			if ( dirlist.at ( i ).toElement().text() == managedDir.absolutePath() )
				isconfigured = true;
		}
		if ( !isconfigured )
		{
			QDomElement root = fc.documentElement();
			QDomElement direlem = fc.createElement ( "dir" );
			QDomText textelem = fc.createTextNode ( managedDir.absolutePath() );
			direlem.appendChild ( textelem );
			root.appendChild ( direlem );

			fcfile.resize ( 0 );
			
			QTextStream ts ( &fcfile );
			fc.save ( ts,4 );

		}
		fcfile.close();
	}

	ownDir.setPath ( QDir::homePath() +  fontreserved );
	if ( !ownDir.exists() )
		ownDir.mkpath ( QDir::homePath() + fontreserved );

	fontsdata.setFileName ( ownDir.absolutePath() + "/fonts.data" );


}

void typotek::initDir()
{

	//load data file
	DataLoader loader ( &fontsdata );
	loader.load();

	// load font files
	QStringList filters;
	filters << "*.otf" << "*.pfb"  << "*.ttf" ;
	ownDir.setNameFilters ( filters );

	QStringList pathList = ownDir.entryList();
	if ( __FM_SHOW_FONTLOADED )
		qDebug() << pathList.join ( "\n" );
	int fontnr = pathList.count();
	QChar fl;//A
	for ( int i = 0 ; i < fontnr ; ++i )
	{

		FontItem *fi = new FontItem ( ownDir.absoluteFilePath ( pathList.at ( i ) ) );
		fontMap.append ( fi );
		realFontMap[fi->name() ] = fi;
		fi->setTags ( tagsMap.value ( fi->name() ) );

		QChar vl ( fi->family().at ( 0 ).toUpper() );
		if ( vl != fl )
		{
			fl = vl;
			QString stars ( fl );
			emit relayStartingStep ( stars , Qt::AlignCenter, Qt::black );
		}
	}
// 	theMainView->slotOrderingChanged ( theMainView->defaultOrd() );


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

// 	QString theFile = QFileDialog::getSaveFileName ( this, "Save fontBook", QDir::homePath() , "Portable Document Format (*.pdf)");

	FontBookDialog bookOption ( this );
	bookOption.exec();

	if ( !bookOption.isOk )
		return;

	double pageHeight = bookOption.getPageSize().height();
	double pageWidth = bookOption.getPageSize().width();
	QString theFile = bookOption.getFileName();
	double familySize = bookOption.getFontSize ( "family" );
	double headSize = bookOption.getFontSize ( "headline" );
	double bodySize = bookOption.getFontSize ( "body" );
	double styleSize = bookOption.getFontSize ( "style" );
	double familynameTab = bookOption.getTabFamily();
	double variantnameTab = bookOption.getTabStyle();
	double sampletextTab = bookOption.getTabSampleText();
	double topMargin =  bookOption.getPageSize().height() / 10.0;
	QStringList loremlist = bookOption.getSampleText().split ( '\n' );
	QString headline = bookOption.getSampleHeadline();
	QPrinter::PageSize printedPageSize = bookOption.getPageSizeConstant();
	double parSize = familySize * 3.0 + styleSize * 1.2 + headSize * 1.2 + static_cast<double> ( loremlist.count() ) * bodySize * 1.2;

	QPrinter thePrinter ( QPrinter::HighResolution );
	thePrinter.setOutputFormat ( QPrinter::PdfFormat );
	thePrinter.setOutputFileName ( theFile );
	thePrinter.setPageSize ( printedPageSize );
	thePrinter.setFullPage ( true );
	QGraphicsScene theScene;
	QRectF pageRect ( 0,0,pageWidth,  pageHeight );
	theScene.setSceneRect ( pageRect );
	QPainter thePainter ( &thePrinter );


	QString styleString ( QString ( "color:white;background-color:black;font-family:Helvetica;font-size:%1pt" ).arg ( familySize ) );

	QFont theFont;// the font for all that is not collected fonts
	theFont.setPointSize ( familySize );
	theFont.setFamily ( "Helvetica" );
	theFont.setBold ( true );

	QFont myLittleFont ( theFont );
	myLittleFont.setPointSize ( 10 );
	myLittleFont.setBold ( false );
	myLittleFont.setItalic ( true );


	QPen abigwhitepen;
	abigwhitepen.setWidth ( 10 );
	abigwhitepen.setColor ( Qt::white );


	QList<FontItem*> localFontMap = theMainView->curFonts();
	QMap<QString, QList<FontItem*> > keyList;
	for ( int i=0; i < localFontMap.count();++i )
	{
		keyList[localFontMap[i]->value ( "family" ) ].append ( localFontMap[i] );
// 		qDebug() << localFontMap[i]->value ( "family" ) ;
	}

	QMap<QString, QList<FontItem*> >::const_iterator kit;
	QProgressDialog progress ( "Creating font book... ", "cancel", 0, keyList.count(), this );
	progress.setWindowModality ( Qt::WindowModal );
	int progressindex=0;

	QList<FontItem*> renderedFont;
	QPointF pen ( 0,0 );
	QGraphicsTextItem *title;
	QGraphicsTextItem *folio;
	QGraphicsTextItem *ABC;
	QGraphicsTextItem *teteChap;
	QGraphicsRectItem *titleCartouche;
	QGraphicsRectItem *edgeCartouche;
	QString firstLetter;
	QString pageNumStr;
	int pageNumber = 0;


	bool firstKey = true;
	for ( kit = keyList.begin(); kit != keyList.end(); ++kit )
	{
// 		qDebug() << "\t" << kit.key();
		progress.setLabelText ( kit.key() );
		progress.setValue ( ++progressindex );
		if ( progress.wasCanceled() )
			break;

		pen.rx() = familynameTab;
		pen.ry() += topMargin;

		firstLetter.clear();
// 		firstLetter.append ( kit.key().at ( 0 ).toUpper() );
		firstLetter.append ( kit.key().toLower() );

		if ( firstKey )
		{
			pageNumStr.setNum ( 1 );
			folio = theScene.addText ( pageNumStr,theFont );
			folio->setPos ( pageWidth * 0.9, pageHeight * 0.9 );
			folio->setZValue ( 9999000.0 );
			ABC = theScene.addText ( firstLetter.at ( 0 ).toUpper() ,theFont );
			ABC->setPos ( pageWidth *0.9,pageHeight * 0.05 );
// 			ABC->rotate(90);
			edgeCartouche = theScene.addRect ( pageWidth * 0.85 + 10.0 , 0.0 - 10.0,  pageWidth * 0.15, pageHeight + 20.0 ,abigwhitepen, Qt::lightGray );

			edgeCartouche->setZValue ( 101.0 );

			ABC->setZValue ( 10000.0 );
			ABC->setDefaultTextColor ( Qt::black );
			firstKey = false;
		}

		if ( ( pen.y() + parSize ) > pageHeight * 0.9 )
		{
			pageNumStr.setNum ( ++pageNumber );
			folio = theScene.addText ( pageNumStr,theFont );
			folio->setPos ( pageWidth * 0.9, pageHeight * 0.9 );
			folio->setZValue ( 9999000.0 );
			folio->setDefaultTextColor ( Qt::black );
			theScene.render ( &thePainter );
			thePrinter.newPage();
			pen.ry() = topMargin;
			for ( int  n = 0; n < renderedFont.count(); ++n )
			{
				renderedFont[n]->deRenderAll();

			}
			renderedFont.clear();
			theScene.removeItem ( theScene.createItemGroup ( theScene.items() ) );
			ABC = theScene.addText ( firstLetter.at ( 0 ).toUpper() ,theFont );
			ABC->setPos ( pageWidth *0.9,pageHeight * 0.05 );
// 			ABC->rotate(90);
			edgeCartouche = theScene.addRect ( pageWidth * 0.85 + 10.0 , 0.0 - 10.0,  pageWidth * 0.15, pageHeight + 20.0 ,abigwhitepen, Qt::lightGray );

			edgeCartouche->setZValue ( 101.0 );

			ABC->setZValue ( 10000.0 );
			ABC->setDefaultTextColor ( Qt::black );

		}

		title = theScene.addText ( QString ( "%1" ).arg ( kit.key().toUpper() ), theFont );
		title->setPos ( pen );
		title->setDefaultTextColor ( Qt::white );
		title->setZValue ( 100.0 );
		QRectF cartrect ( 0,pen.y(),title->sceneBoundingRect().right(), title->sceneBoundingRect().height() );
		titleCartouche = theScene.addRect ( cartrect,QPen ( Qt::transparent ) ,Qt::black );
// 		titleCartouche->setPos(pen);
		pen.ry() += 4.0  * familySize;

		for ( int  n = 0; n < kit.value().count(); ++n )
		{
// 			qDebug() << "\t\t" << kit.value()[n]->variant();

			if ( ( pen.y() + ( parSize - 4.0 * familySize ) ) > pageHeight * 0.9 )
			{
				pageNumStr.setNum ( ++pageNumber );
				folio = theScene.addText ( pageNumStr,theFont );
				folio->setPos ( pageWidth * 0.9, pageHeight * 0.9 );
				folio->setDefaultTextColor ( Qt::black );
				folio->setZValue ( 1000.0 );
				theScene.render ( &thePainter );
				thePrinter.newPage();
				pen.ry() = topMargin;
				for ( int  d = 0; d <  renderedFont.count() ; ++d )
				{
					renderedFont[d]->deRenderAll();

				}
				renderedFont.clear();
				theScene.removeItem ( theScene.createItemGroup ( theScene.items() ) );
				ABC = theScene.addText ( firstLetter.at ( 0 ).toUpper() ,theFont );
				ABC->setPos ( pageWidth *0.9,pageHeight * 0.05 );
// 				ABC->rotate(90);

				teteChap = theScene.addText ( firstLetter, myLittleFont );
				teteChap->setPos ( pageWidth * 0.66, pageHeight * 0.03 );
				teteChap->setDefaultTextColor ( Qt::gray );


				edgeCartouche = theScene.addRect ( pageWidth * 0.85 + 10.0 , 0.0 - 10.0,  pageWidth * 0.15, pageHeight + 20.0 ,abigwhitepen, Qt::lightGray );
				edgeCartouche->setZValue ( 101.0 );

				ABC->setZValue ( 10000.0 );
				ABC->setDefaultTextColor ( Qt::black );
			}
			pen.rx() =variantnameTab;
			FontItem* curfi = kit.value() [n];
			qDebug() << "\tRENDER" << kit.key() << curfi->variant();
			renderedFont.append ( curfi );
			curfi->renderLine ( &theScene,curfi->variant(), pen ,styleSize );
			pen.rx() = sampletextTab;
			pen.ry() +=  2.0 * styleSize;
			curfi->renderLine ( &theScene, headline,pen, headSize );
			pen.ry() +=  headSize * 0.5;
			for ( int l=0; l < loremlist.count(); ++l )
			{
				curfi->renderLine ( &theScene, loremlist[l],pen, bodySize );
				pen.ry() +=  bodySize * 1.2;
			}
			pen.ry() +=styleSize * 2.0;

		}
	}
	if ( renderedFont.count() )
	{
		theScene.render ( &thePainter );
		for ( int  d = 0; d <  renderedFont.count() ; ++d )
		{
			renderedFont[d]->deRenderAll();

		}

	}
}

QList<FontItem*> typotek::getCurrentFonts()
{
	return theMainView->curFonts();
}

void typotek::popupTagsetEditor()
{
	TagSetEditor ed;
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
	if ( QMessageBox::question ( this,"Fontmatrix care","You are about to activate a bunch of fonts,\nit is time to cancel if it was not your intent", QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel ) == QMessageBox::Ok )
		theMainView->slotActivateAll();
}

void typotek::slotDeactivateCurrents()
{
	if ( QMessageBox::question ( this,"Fontmatrix care","You are about to deactivate a bunch of fonts,\nit is time to cancel if it was not your intent",QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel ) == QMessageBox::Ok )
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
	if ( i < fontMap.count() && i >= 0)
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
	if(!item)
	{
		statusBar()->showMessage ( tr ( "There is no font selected" ), 5000 );
		return;
	}
	
	QString program = "/usr/bin/fontforge";
	QStringList arguments;
	arguments << "-nosplash" << item->path() ;

	QProcess *myProcess = new QProcess(this);
	myProcess->start(program, arguments);
}

void typotek::setupDrop()
{
	setAcceptDrops(true);
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
// 		qDebug() << "typotek::dropEvent -> "<< uris[i];
		QUrl url(uris[i]);
		if(url.scheme() == "file")
		{
			if ( uris[i].endsWith ( "ttf",Qt::CaseInsensitive ) )
			{
				ret << url.path();
			}
			else if ( uris[i].endsWith ( "otf",Qt::CaseInsensitive ) )
			{
				ret << url.path();
			}
			else if( uris[i].endsWith ( "pfb",Qt::CaseInsensitive ) )
			{
				ret << url.path();
			}
		}
		else if(url.scheme() == "http")
		{
			// TODO Get fonts over http
			statusBar()->showMessage ( tr ( "Support of DragNDrop over http is sheduled but not yet effective" ), 5000 );
		}
	}
	
// 	qDebug() << ret.join("||");
	if(ret.count())
		open(ret);

}

void typotek::dragEnterEvent(QDragEnterEvent * event)
{
	qDebug() << event->mimeData()->formats().join("|");
	if (event->mimeData()->hasFormat("text/uri-list"))
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


void typotek::slotPrefsPanel()
{
	PrefsPanelDialog pp(this);
	pp.initSystrayPrefs(QSystemTrayIcon::isSystemTrayAvailable(),
                        systray->isVisible(),
                        systray->hasActivateAll(),
                        systray->allConfirmation(),
                        systray->tagsConfirmation());
	pp.initSampleTextPrefs();
	pp.exec();
}

void typotek::forwardUpdateView()
{
	theMainView->slotView(true);
}

void typotek::addNamedSample(QString name, QString sample)
{
	if(name.isEmpty() || sample.isEmpty())
	{
		statusBar()->showMessage(tr("You provided an empty string, it’s not fair"), 3000);
		return;
	}
	
	if(name == "default" )
	{
		statusBar()->showMessage(tr("\"default\" is a reserved keyword"), 3000);
		return;
	}
	m_namedSamples[name] = sample;
	theMainView->refillSampleList();
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

void typotek::addNamedSampleFragment(QString name, QString sampleFragment)
{
	// No need to check, it’s just for the loader
	QStringList lines = m_namedSamples[name].split("\n");
	lines << sampleFragment;
	m_namedSamples[name] = lines.join("\n");
}


QString typotek::namedSample(QString name)
{
	if(name.isEmpty())
		return m_namedSamples["default"];
	return m_namedSamples[name];
}

void typotek::setSampleText(QString s)
{
	m_namedSamples["default"] += s;
}


void typotek::changeSample(QString name, QString text)
{
	if(!m_namedSamples.contains(name))
		return;
	m_namedSamples[name] = text;
}

void typotek::setWord(QString s, bool updateView)
{
	m_theWord = s;
	if (updateView)
		theMainView->slotView(true);
}



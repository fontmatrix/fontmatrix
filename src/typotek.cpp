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
#include "typotekadaptator.h"
#include "fontbookdialog.h"

#include <QtGui>
#include <QTextEdit>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDir>
#include <QDBusConnection>
#include <QProgressDialog>

QStringList typotek::tagsList;
typotek* typotek::instance = 0;

typotek::typotek()
{
	instance = this;

	checkOwnDir();
	readSettings();
	fillTagsList();
	initDir();


	theMainView = new MainViewWidget ( this );
	setCentralWidget ( theMainView );

	createActions();
	createMenus();
	createStatusBar();


	{
		actAdaptator = new TypotekAdaptator ( this );
		if ( !QDBusConnection::sessionBus().registerService ( "com.typotek.fonts" ) )
			qDebug() << "unable to register to DBUS";
		if ( !QDBusConnection::sessionBus().registerObject ( "/FontActivation", actAdaptator, QDBusConnection::ExportAllContents ) )
			qDebug() << "unable to register to DBUS";
	}
}

void typotek::closeEvent ( QCloseEvent *event )
{
	if ( maybeSave() )
	{
		writeSettings();
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void typotek::newFile()
{
	if ( maybeSave() )
	{
		textEdit->clear();
		setCurrentFile ( "" );
	}
}

void typotek::open()
{

	QStringList pathList;
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
	tali << "Activated_Off";

	QProgressDialog progress ( "Importing font files... ", "cancel", 0, pathList.count(), this );
	progress.setWindowModality ( Qt::WindowModal );

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


			FontItem *fi = new FontItem ( pathList.at ( i ) );

			fi->setTags ( tali );
			fontMap.append ( fi );
			realFontMap[fi->name() ] = fi;
		}
	}
	theMainView->slotReloadFontList();
// 	theMainView->slotOrderingChanged ( theMainView->defaultOrd() );
}

bool typotek::save()
{
	if ( !fontsdata.open ( QFile::WriteOnly | QIODevice::Truncate| QFile::Text ) )
	{
		qDebug() << "re-oops";
	}
	QTextStream ts ( &fontsdata );
	for ( int i=0;i<fontMap.count();++i )
	{
		ts << fontMap[i]->name() << "#" << fontMap[i]->tags().join ( "#" ) << '\n';
	}
	fontsdata.close();
}

bool typotek::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName ( this );
	if ( fileName.isEmpty() )
		return false;

	return saveFile ( fileName );
}

void typotek::about()
{
	QMessageBox::about ( this, tr ( "About Typotek" ),
	                     tr ( "Typotek is what we all hoped for long time : a font manager for linux" ) );
}

void typotek::documentWasModified()
{
	setWindowModified ( true );
}

void typotek::createActions()
{


	openAct = new QAction ( QIcon ( ":/fileopen.xpm" ), tr ( "&Import..." ), this );
	openAct->setShortcut ( tr ( "Ctrl+O" ) );
	openAct->setStatusTip ( tr ( "Import a directory" ) );
	connect ( openAct, SIGNAL ( triggered() ), this, SLOT ( open() ) );

	saveAct = new QAction ( QIcon ( ":/filesave.xpm" ), tr ( "&Save" ), this );
	saveAct->setShortcut ( tr ( "Ctrl+S" ) );
	saveAct->setStatusTip ( tr ( "Save the document to disk" ) );
	connect ( saveAct, SIGNAL ( triggered() ), this, SLOT ( save() ) );

	printAct = new QAction ( tr ( "Print..." ),this );
	printAct->setStatusTip ( tr ( "Print a specimen of the current font" ) );
	connect ( printAct, SIGNAL ( triggered() ), this, SLOT ( print() ) );

	fontBookAct = new QAction ( tr ( "Export font book..." ),this );
	fontBookAct->setStatusTip ( tr ( "Export a pdf that show selected fonts" ) );
	connect ( fontBookAct, SIGNAL ( triggered() ), this, SLOT ( fontBook() ) );

	exitAct = new QAction ( tr ( "E&xit" ), this );
	exitAct->setShortcut ( tr ( "Ctrl+Q" ) );
	exitAct->setStatusTip ( tr ( "Exit the application" ) );
	connect ( exitAct, SIGNAL ( triggered() ), this, SLOT ( close() ) );


	aboutAct = new QAction ( tr ( "&About" ), this );
	aboutAct->setStatusTip ( tr ( "Show the Typotek's About box" ) );
	connect ( aboutAct, SIGNAL ( triggered() ), this, SLOT ( about() ) );

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

	helpMenu = menuBar()->addMenu ( tr ( "&Help" ) );
	helpMenu->addAction ( aboutAct );

}

void typotek::createToolBars()
{
	fileToolBar = addToolBar ( tr ( "File" ) );
	fileToolBar->addAction ( newAct );
	fileToolBar->addAction ( openAct );
	fileToolBar->addAction ( saveAct );

	editToolBar = addToolBar ( tr ( "Edit" ) );
	editToolBar->addAction ( cutAct );
	editToolBar->addAction ( copyAct );
	editToolBar->addAction ( pasteAct );
}

void typotek::createStatusBar()
{
	statusBar()->showMessage ( tr ( "Ready" ) );
}

void typotek::readSettings()
{
	QSettings settings ( "Trolltech", "Application Example" );
	QPoint pos = settings.value ( "pos", QPoint ( 200, 200 ) ).toPoint();
	QSize size = settings.value ( "size", QSize ( 400, 400 ) ).toSize();
	resize ( size );
	move ( pos );
}

void typotek::writeSettings()
{
	QSettings settings ( "Trolltech", "Application Example" );
	settings.setValue ( "pos", pos() );
	settings.setValue ( "size", size() );
}

bool typotek::maybeSave()
{

	return true;
}

void typotek::loadFile ( const QString &fileName )
{
	QFile file ( fileName );
	if ( !file.open ( QFile::ReadOnly | QFile::Text ) )
	{
		QMessageBox::warning ( this, tr ( "Application" ),
		                       tr ( "Cannot read file %1:\n%2." )
		                       .arg ( fileName )
		                       .arg ( file.errorString() ) );
		return;
	}

	QTextStream in ( &file );
	QApplication::setOverrideCursor ( Qt::WaitCursor );
	textEdit->setPlainText ( in.readAll() );
	QApplication::restoreOverrideCursor();

	setCurrentFile ( fileName );
	statusBar()->showMessage ( tr ( "File loaded" ), 2000 );
}

bool typotek::saveFile ( const QString &fileName )
{
	QFile file ( fileName );
	if ( !file.open ( QFile::WriteOnly | QFile::Text ) )
	{
		QMessageBox::warning ( this, tr ( "Application" ),
		                       tr ( "Cannot write file %1:\n%2." )
		                       .arg ( fileName )
		                       .arg ( file.errorString() ) );
		return false;
	}

	QTextStream out ( &file );
	QApplication::setOverrideCursor ( Qt::WaitCursor );
	out << textEdit->toPlainText();
	QApplication::restoreOverrideCursor();

	setCurrentFile ( fileName );
	statusBar()->showMessage ( tr ( "File saved" ), 2000 );
	return true;
}

void typotek::setCurrentFile ( const QString &fileName )
{
	curFile = fileName;
	textEdit->document()->setModified ( false );
	setWindowModified ( false );

	QString shownName;
	if ( curFile.isEmpty() )
		shownName = "untitled.txt";
	else
		shownName = strippedName ( curFile );

	setWindowTitle ( tr ( "%1[*] - %2" ).arg ( shownName ).arg ( tr ( "Application" ) ) );
}

QString typotek::strippedName ( const QString &fullFileName )
{
	return QFileInfo ( fullFileName ).fileName();
}

typotek::~typotek()
{

}

void typotek::fillTagsList()
{

}

void typotek::checkOwnDir()
{
	ownDir.setPath ( QDir::homePath() + "/.typotek" );
	if ( !ownDir.exists() )
		ownDir.mkpath ( QDir::homePath() + "/.typotek" );

	fontsdata.setFileName ( ownDir.absolutePath() + "/fonts.data" );


}

void typotek::initDir()
{

	//load data file
	if ( !fontsdata.open ( QFile::ReadOnly | QFile::Text ) )
	{
		qDebug() << "oops";
	}
	QTextStream ts ( &fontsdata );
	while ( !ts.atEnd() )
	{
		QStringList line = ts.readLine().split ( '#' );
		tagsMap[line[0]] = line.mid ( 1 );
		for ( int i = 1; i< line.count();++i )
		{
			if ( !tagsList.contains ( line[i] ) )
				tagsList.append ( line[i] );
		}

	}
	fontsdata.close();

	// load font files
	QStringList filters;
	filters << "*.otf" << "*.pfb"  << "*.ttf" ;
	ownDir.setNameFilters ( filters );

	QStringList pathList = ownDir.entryList();
	for ( int i = 0 ; i < pathList.count(); ++i )
	{
		FontItem *fi = new FontItem ( ownDir.absoluteFilePath ( pathList.at ( i ) ) );
		fontMap.append ( fi );
		realFontMap[fi->name() ] = fi;
		fi->setTags ( tagsMap.value ( fi->name() ) );
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
	
	FontBookDialog bookOption(this);
	bookOption.exec();
	
	if(!bookOption.isOk)
		return;
	
	double pageHeight = bookOption.getPageSize().height();
	double pageWidth = bookOption.getPageSize().width();
	QString theFile = bookOption.getFileName();
	double familySize = bookOption.getFontSize("family");
	double headSize = bookOption.getFontSize("headline");
	double bodySize = bookOption.getFontSize("body");
	double styleSize = bookOption.getFontSize("style");
	double familynameTab = bookOption.getTabFamily();
	double variantnameTab = bookOption.getTabStyle();
	double sampletextTab = bookOption.getTabSampleText();
	double topMargin =  bookOption.getPageSize().height() / 10.0;
	QStringList loremlist = bookOption.getSampleText().split ( '\n' );
	QString headline = bookOption.getSampleHeadline();
	QPrinter::PageSize printedPageSize = bookOption.getPageSizeConstant();
	double parSize = familySize * 3.0 + styleSize * 1.2 + headSize * 1.2 + static_cast<double>(loremlist.count()) * bodySize * 1.2;
	
	QPrinter thePrinter ( QPrinter::HighResolution );
	thePrinter.setOutputFormat ( QPrinter::PdfFormat );
	thePrinter.setOutputFileName ( theFile );
	thePrinter.setPageSize ( printedPageSize );
	thePrinter.setFullPage ( true );
	QGraphicsScene theScene;
	QRectF pageRect ( 0,0,pageWidth,  pageHeight);
	theScene.setSceneRect ( pageRect );
	QPainter thePainter ( &thePrinter );
	
	
	QString styleString ( QString("color:white;background-color:black;font-family:Helvetica;font-size:%1pt" ).arg(familySize));
	
	
	
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
	QPointF pen(0,0);
	QGraphicsTextItem *title;
	QGraphicsTextItem *folio;
	int pageNumber = 0;
	
	
	for ( kit = keyList.begin(); kit != keyList.end(); ++kit )
	{
// 		qDebug() << "\t" << kit.key();
		progress.setLabelText ( kit.key() );
		progress.setValue ( ++progressindex );
		if ( progress.wasCanceled() )
			break;
		
		pen.rx() = familynameTab;
		pen.ry() += topMargin;

		if ( ( pen.y() + parSize ) > pageHeight * 0.9 )
		{
			folio = theScene.addText ( "" );
			folio->setHtml(QString("<span style=\"font-family:Helvetica;font-size:6pt\">%1</span>").arg(++pageNumber));
			folio->setPos(theScene.width() / 2.0, theScene.height() - 15.0);
			theScene.render ( &thePainter );
			thePrinter.newPage();
			pen.ry() = topMargin;
			for ( int  n = 0; n < renderedFont.count(); ++n )
			{
				renderedFont[n]->deRenderAll();
				
			}
				renderedFont.clear();
				theScene.removeItem(theScene.createItemGroup(theScene.items()));
		}
		
		title = theScene.addText ( "" );
		title->setHtml ( QString ( "<span style=\"%2\">%1</span>" ).arg ( kit.key() ).arg ( styleString ) );
		title->setPos ( pen );
		title->setZValue(100.0);
		pen.ry() += 4.0  * familySize;
		
		for ( int  n = 0; n < kit.value().count(); ++n )
		{
// 			qDebug() << "\t\t" << kit.value()[n]->variant();

			if ( ( pen.y() + (parSize - 4.0 * familySize) ) > pageHeight * 0.9 )
			{
				folio = theScene.addText ( "" );
				folio->setHtml(QString("<span style=\"font-family:Helvetica;font-size:6pt\">%1</span>").arg(++pageNumber));
				folio->setPos(theScene.width() / 2.0, theScene.height() - 15.0);
				theScene.render ( &thePainter );
				thePrinter.newPage();
				pen.ry() = topMargin;
				for ( int  d = 0; d <  renderedFont.count() ; ++d )
				{
					renderedFont[d]->deRenderAll();
					
				}
				renderedFont.clear();
				theScene.removeItem(theScene.createItemGroup(theScene.items()));
				
			}
			pen.rx()=variantnameTab;
			FontItem* curfi = kit.value()[n];
			qDebug() << "\tRENDER" << kit.key() << curfi->variant();
			renderedFont.append(curfi);
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
	if(renderedFont.count())
	{
		folio = theScene.addText ( "" );
		folio->setHtml(QString("<span style=\"font-family:Helvetica;font-size:6pt\">%1</span>").arg(++pageNumber));
		folio->setPos(theScene.width() / 2.0, theScene.height() - 15.0);
		theScene.render(&thePainter);
		
	}
}

QList<FontItem*> typotek::getCurrentFonts()
{
	return theMainView->curFonts();
}


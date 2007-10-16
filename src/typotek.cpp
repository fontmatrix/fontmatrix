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

#include <QtGui>
#include <QTextEdit>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>
#include <QDir>
#include <QDBusConnection>

QStringList typotek::tagsList;
typotek* typotek::instance = 0;

typotek::typotek()
{
	instance = this;
	textEdit = new QTextEdit;
	checkOwnDir();
	theMainView = new MainViewWidget ( this );

	setCentralWidget ( theMainView );

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	readSettings();

	connect ( textEdit->document(), SIGNAL ( contentsChanged() ),
	          this, SLOT ( documentWasModified() ) );

	setCurrentFile ( "" );
	fillTagsList();
	initDir();
	actAdaptator = new TypotekAdaptator(this);
	QDBusConnection::sessionBus () .registerObject("/FontActivation", this);
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
	QString dir = QFileDialog::getExistingDirectory ( this, tr ( "Add Directory" ), QDir::homePath ()  ,  QFileDialog::ShowDirsOnly );
	QDir theDir ( dir );
	QStringList filters;
	filters << "*.otf" << "*.pfb" << "*.ttf" ;
	theDir.setNameFilters ( filters );
	// We'll recurse another day
	pathList = theDir.entryList();
	for ( int i = 0 ; i < pathList.count(); ++i )
	{
		QFile ff ( theDir.absoluteFilePath ( pathList.at ( i ) ) );
		if ( ff.copy ( ownDir.absolutePath() + "/" + pathList.at ( i ) ) )
		{
			FontItem *fi = new FontItem ( ownDir.absoluteFilePath ( pathList.at ( i ) ) );
			fontMap.append ( fi );
			realFontMap[fi->name()] = fi;
		}
	}
	theMainView->slotOrderingChanged ( theMainView->defaultOrd() );
}

bool typotek::save()
{
	if(!fontsdata.open(QFile::WriteOnly | QIODevice::Truncate| QFile::Text))
	{
		qDebug() << "re-oops";
	}
	QTextStream ts(&fontsdata);
	for(int i=0;i<fontMap.count();++i)
	{
		ts << fontMap[i]->name() << "#" << fontMap[i]->tags().join("#") << '\n';
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
	QMessageBox::about ( this, tr ( "About Application" ),
	                     tr ( "The <b>Application</b> example demonstrates how to "
	                          "write modern GUI applications using Qt, with a menu bar, "
	                          "toolbars, and a status bar." ) );
}

void typotek::documentWasModified()
{
	setWindowModified ( true );
}

void typotek::createActions()
{
	newAct = new QAction ( QIcon ( ":/filenew.xpm" ), tr ( "&New" ), this );
	newAct->setShortcut ( tr ( "Ctrl+N" ) );
	newAct->setStatusTip ( tr ( "Create a new file" ) );
	connect ( newAct, SIGNAL ( triggered() ), this, SLOT ( newFile() ) );

	openAct = new QAction ( QIcon ( ":/fileopen.xpm" ), tr ( "&Import..." ), this );
	openAct->setShortcut ( tr ( "Ctrl+O" ) );
	openAct->setStatusTip ( tr ( "Import a directory" ) );
	connect ( openAct, SIGNAL ( triggered() ), this, SLOT ( open() ) );

	saveAct = new QAction ( QIcon ( ":/filesave.xpm" ), tr ( "&Save" ), this );
	saveAct->setShortcut ( tr ( "Ctrl+S" ) );
	saveAct->setStatusTip ( tr ( "Save the document to disk" ) );
	connect ( saveAct, SIGNAL ( triggered() ), this, SLOT ( save() ) );

	saveAsAct = new QAction ( tr ( "Save &As..." ), this );
	saveAsAct->setStatusTip ( tr ( "Save the document under a new name" ) );
	connect ( saveAsAct, SIGNAL ( triggered() ), this, SLOT ( saveAs() ) );

	exitAct = new QAction ( tr ( "E&xit" ), this );
	exitAct->setShortcut ( tr ( "Ctrl+Q" ) );
	exitAct->setStatusTip ( tr ( "Exit the application" ) );
	connect ( exitAct, SIGNAL ( triggered() ), this, SLOT ( close() ) );

	cutAct = new QAction ( QIcon ( ":/editcut.xpm" ), tr ( "Cu&t" ), this );
	cutAct->setShortcut ( tr ( "Ctrl+X" ) );
	cutAct->setStatusTip ( tr ( "Cut the current selection's contents to the "
	                            "clipboard" ) );
	connect ( cutAct, SIGNAL ( triggered() ), textEdit, SLOT ( cut() ) );

	copyAct = new QAction ( QIcon ( ":/editcopy.xpm" ), tr ( "&Copy" ), this );
	copyAct->setShortcut ( tr ( "Ctrl+C" ) );
	copyAct->setStatusTip ( tr ( "Copy the current selection's contents to the "
	                             "clipboard" ) );
	connect ( copyAct, SIGNAL ( triggered() ), textEdit, SLOT ( copy() ) );

	pasteAct = new QAction ( QIcon ( ":/editpaste.xpm" ), tr ( "&Paste" ), this );
	pasteAct->setShortcut ( tr ( "Ctrl+V" ) );
	pasteAct->setStatusTip ( tr ( "Paste the clipboard's contents into the current "
	                              "selection" ) );
	connect ( pasteAct, SIGNAL ( triggered() ), textEdit, SLOT ( paste() ) );

	aboutAct = new QAction ( tr ( "&About" ), this );
	aboutAct->setStatusTip ( tr ( "Show the application's About box" ) );
	connect ( aboutAct, SIGNAL ( triggered() ), this, SLOT ( about() ) );

	aboutQtAct = new QAction ( tr ( "About &Qt" ), this );
	aboutQtAct->setStatusTip ( tr ( "Show the Qt library's About box" ) );
	connect ( aboutQtAct, SIGNAL ( triggered() ), qApp, SLOT ( aboutQt() ) );

	cutAct->setEnabled ( false );
	copyAct->setEnabled ( false );
	connect ( textEdit, SIGNAL ( copyAvailable ( bool ) ),
	          cutAct, SLOT ( setEnabled ( bool ) ) );
	connect ( textEdit, SIGNAL ( copyAvailable ( bool ) ),
	          copyAct, SLOT ( setEnabled ( bool ) ) );
}

void typotek::createMenus()
{
	fileMenu = menuBar()->addMenu ( tr ( "&File" ) );
	fileMenu->addAction ( newAct );
	fileMenu->addAction ( openAct );
	fileMenu->addAction ( saveAct );
	fileMenu->addAction ( saveAsAct );
	fileMenu->addSeparator();
	fileMenu->addAction ( exitAct );

	editMenu = menuBar()->addMenu ( tr ( "&Edit" ) );
	editMenu->addAction ( cutAct );
	editMenu->addAction ( copyAct );
	editMenu->addAction ( pasteAct );

	menuBar()->addSeparator();

	helpMenu = menuBar()->addMenu ( tr ( "&Help" ) );
	helpMenu->addAction ( aboutAct );
	helpMenu->addAction ( aboutQtAct );
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
	if ( textEdit->document()->isModified() )
	{
		int ret = QMessageBox::warning ( this, tr ( "Application" ),
		                                 tr ( "The document has been modified.\n"
		                                      "Do you want to save your changes?" ),
		                                 QMessageBox::Yes | QMessageBox::Default,
		                                 QMessageBox::No,
		                                 QMessageBox::Cancel | QMessageBox::Escape );
		if ( ret == QMessageBox::Yes )
			return save();
		else if ( ret == QMessageBox::Cancel )
			return false;
	}
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
	tagsList <<  "notag";
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
	if(!fontsdata.open( QFile::ReadOnly | QFile::Text))
	{
		qDebug() << "oops";
	}
	QTextStream ts(&fontsdata);
	while(!ts.atEnd())
	{
		QStringList line = ts.readLine().split('#');
		tagsMap[line[0]] = line.mid(1);
		for(int i = 1; i< line.count();++i)
		{
			if(!tagsList.contains(line[i]))
				tagsList.append(line[i]);
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
		realFontMap[fi->name()] = fi;
		fi->setTags(tagsMap.value(fi->name()));
	}
	theMainView->slotOrderingChanged ( theMainView->defaultOrd() );
	
	
}

QList< FontItem * > typotek::getFonts ( QString pattern, QString field )
{
	QList< FontItem * > ret;

	for ( int i =0; i < fontMap.count(); ++i )
	{
		if(field == "tag")
		{
			QStringList tl =  fontMap[i]->tags();
			for(int ii=0; ii<tl.count(); ++ii)
			{
				if(tl[ii].contains(pattern,Qt::CaseInsensitive))
					ret.append(fontMap[i]);
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


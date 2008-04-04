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


#include <QApplication>
#include <QDesktopWidget>

#include <QIcon>
#include <QSplashScreen>
#include <QPixmap>
#include <QBitmap>
#include <QDebug>
#include <QLocale>
#include <QTranslator>
#include <QSettings>

#include "typotek.h"
#include "listdockwidget.h"

bool __FM_SHOW_FONTLOADED;

/**
 *
 * @param argc
 * @param argv[]
 * @return
 */
int main ( int argc, char *argv[] )
{
	QCoreApplication::setOrganizationName("Undertype");
	QCoreApplication::setApplicationName("fontmatrix");
	
	Q_INIT_RESOURCE ( application );
	QApplication app ( argc, argv );
	app.setWindowIcon ( QIcon ( ":/fontmatrix_icon.png" ) );
	
	QTranslator translator;
	if(translator.load(":/texts/fontmatrix"))
	{
		app.installTranslator(&translator);
	}
	else
	{
		QFile f(":/texts/fontmatrix");
		qDebug()<< "Unable to load"<< f.fileName() ;
	}


	if ( app.arguments().contains ( "listfonts" ) )
	{
		__FM_SHOW_FONTLOADED = true;
	}
	else
	{
		__FM_SHOW_FONTLOADED = false;
	}
	
	typotek * mw = new typotek;
	
	
	QSettings settings;
	
	QSplashScreen theSplash;
	QPixmap theSplashPix ( ":/fontmatrix_splash.png" );
	bool splash = settings.value("SplashScreen", false).toBool();
	if( app.arguments().contains ( "splash" ) || splash )
	{
		QImage rootW = QPixmap::grabWindow ( QApplication::desktop()->winId(),
				( QApplication::desktop()->rect().width()-theSplashPix.rect().width() ) /2,
				  ( QApplication::desktop()->rect().height()-theSplashPix.rect().height() ) /2,
				    theSplashPix.rect().width(),
						    theSplashPix.rect().height() ).toImage();
		QImage splashImg = theSplashPix.toImage();
		
		for(int posx = 0; posx < rootW.width() ;++posx)
		{
			for(int posy =0;posy < rootW.height();++posy)
			{
				QRgb splashC(splashImg.pixel(posx,posy));
				QRgb rootC(rootW.pixel(posx,posy));
				uint Salpha(qAlpha(splashC));
				uint resRed ((qRed(splashC) * Salpha / 255) +
						(qRed(rootC) * (255 - Salpha) / 255));
				uint resGreen((qGreen(splashC) * Salpha / 255) +
						(qGreen(rootC) * (255 - Salpha) / 255));
				uint resBlue((qBlue(splashC) * Salpha / 255) +
						(qBlue(rootC) * (255 - Salpha) / 255));
				uint resRGB(qRgb ( resRed, resGreen, resBlue ));
				splashImg.setPixel(posx,posy,resRGB);
			}
		}
		theSplash.setPixmap(QPixmap::fromImage(splashImg));
		QFont spFont;
		spFont.setPointSize(14);/*
		spFont.setBold(true);*/
		theSplash.setFont(spFont);
		if(/*__FM_SHOW_FONTLOADED*/ 1)
		{
			QObject::connect ( mw,SIGNAL ( relayStartingStepOut ( QString, int, QColor ) ),&theSplash,SLOT ( showMessage ( const QString&, int, const QColor& ) ) );
		}
	}
	
	if(splash)
		theSplash.show();
	
	mw->initMatrix();
	mw->show();
	
	if(splash)
		theSplash.finish ( mw );

	LazyInit lazyInit;
	// Now we should have A running ListDockWidget in main thread
	QObject::connect(&lazyInit, SIGNAL(endOfRun()), ListDockWidget::getInstance(), SLOT(unlockFilter()) );
	
	lazyInit.start(QThread::LowestPriority);
	
	return app.exec();
}


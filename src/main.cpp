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
#include <QIcon>
#include <QSplashScreen>
#include <QPixmap>
#include <QBitmap>
#include "typotek.h"

bool __FM_SHOW_FONTLOADED;

/**
 * 
 * @param argc 
 * @param argv[] 
 * @return 
 */
int main(int argc, char *argv[])
{
      Q_INIT_RESOURCE(application);
      QApplication app(argc, argv);
      app.setWindowIcon (QIcon(":/fontmatrix_icon.png") );
      
      if(app.arguments().contains("debugfonts"))
      {
	      __FM_SHOW_FONTLOADED = true;
      }
      else
      {
	      __FM_SHOW_FONTLOADED = false;
      }
      typotek * mw;
      QPixmap theSplashPix(":/fontmatrix_splash.png");
      QSplashScreen theSplash(theSplashPix);
        mw = new typotek;
	QObject::connect(mw,SIGNAL(relayStartingStep(QString, int, QColor)),&theSplash,SLOT(showMessage( const QString&, int, const QColor& )));
      // Many splash transparency tests
      theSplash.setMask(theSplashPix.mask());
//       theSplash.setAttribute(Qt::WA_NoBackground);
//       QPalette spalette;
//       spalette.setBrush ( QPalette::Window, Qt::transparent );
//       theSplash.setPalette(spalette);
//       theSplash.setAutoFillBackground(true);
      theSplash.show();
	mw->initMatrix();	
      mw->show();
      theSplash.finish(mw);
      
      
      return app.exec();
}


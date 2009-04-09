//
// C++ Implementation: fmmatchraster
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include "puzzleviewimp.h"
#include "fmmatchraster.h"
#include "typotek.h"
#include "mainviewwidget.h"
#include "fontitem.h"

#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QImage>
#include <QString>
#include <QSettings>

#include <QDebug>
#include <QMessageBox>


FMMatchRaster::FMMatchRaster ( QWidget * parent )
		:QDialog ( parent )
{
	setupUi ( this );
	QSettings settings;
	m_compsize = settings.value ( "MatchRaster/Size", 120 ).toInt();
	m_matchLimit = settings.value ( "MatchRaster/Limit", 800.0 ).toDouble();

	m_progressValue = 0;
	m_waitingForButton = false;
	waitingFont = 0;


	connect ( browseButton,SIGNAL ( clicked() ),this, SLOT ( browseImage() ) );
	connect ( loadButton, SIGNAL ( clicked() ),this, SLOT ( loadImage() ) );
	connect ( letter,SIGNAL ( textChanged ( const QString & ) ),this,SLOT ( addImage ( const QString & ) ) );
	connect ( searchButton,SIGNAL ( clicked() ),this,SLOT ( search() ) );

	connect ( iView,SIGNAL ( rectChange ( QRect ) ),this,SLOT ( recordCurrentRect ( QRect ) ) );
	connect ( iView,SIGNAL ( selColorChanged ( QRgb ) ),this,SLOT ( recordCurrentColor ( QRgb ) ) );

	connect ( buttonBox,SIGNAL ( rejected() ),this,SLOT ( slotRefuseFont() ) );
	connect ( buttonBox,SIGNAL ( accepted() ),this,SLOT ( slotAcceptFont() ) );
}

FMMatchRaster::~ FMMatchRaster()
{
}

void FMMatchRaster::browseImage()
{
	QString ifile ( QFileDialog::getOpenFileName ( this, "Fontmatrix - Browse Image", QDir::homePath() ) );
	imagePath->setText ( ifile );
}

void FMMatchRaster::loadImage()
{
	QString ifile ( imagePath->text() );
	if ( QFile::exists ( ifile ) )
		iView->setImage ( ifile );
}

void FMMatchRaster::addImage ( const QString & text )
{
	if ( letter->text().isEmpty() )
		return;

	bool ok;
	refCodepoint = ( letter->text().count() != 4 ) ? letter->text().at ( 0 ).unicode() : letter->text().toUInt ( &ok, 16 );
	refImage = iView->getPixmap().toImage().copy ( curRect );

}

void FMMatchRaster::search()
{
	buttonBox->setEnabled ( false );
	if ( !m_waitingForButton )
	{
		remainFonts = compFonts = typotek::getInstance()->getCurrentFonts() ;
		progressBar->setRange ( 0, compFonts.count() );
		stackedWidget->setCurrentIndex ( 1 );
	}
	else
	{
		compFonts = remainFonts;
	}

	PuzzleViewImp ref ( refImage , curCol );

	foreach ( FontItem* fit, compFonts )
	{
		progressBar->setValue ( ++m_progressValue );
		remainFonts.removeAll ( fit );

		fontName->setText ( fit->fancyName() );
		QImage cImg ( fit->charImage ( refCodepoint , m_compsize ) );
		const int cw = cImg.width();
		const int ch = cImg.height();
		if ( ( !cImg.isNull() ) && ( cw > 0 ) && ( ch > 0 ) )
		{
			// crop it
			QRect r;
			const QRgb wp = cImg.pixel ( 0,0 );
			bool topReached ( false );
			for ( int y ( 0 ); y < ch; ++y )
			{

				for ( int x ( 0 );x <  cw; ++x )
				{
					if ( cImg.pixel ( x,y ) != wp )
					{
						topReached = true;
						r.setTop ( y );
						break;
					}
				}
				if ( topReached )
					break;
			}
			bool bottomReached ( false );
			for ( int y ( ch - 1 ); y >= 0; --y )
			{

				for ( int x ( cw-1 );x >=0; --x )
				{
					if ( cImg.pixel ( x,y ) != wp )
					{
						bottomReached = true;
						r.setBottom ( y );
						break;
					}
				}
				if ( bottomReached )
					break;
			}
			r.setLeft ( cw );
			for ( int y ( 0 ); y < ch; ++y )
			{

				for ( int x ( 0 );x <  cw; ++x )
				{
					if ( cImg.pixel ( x,y ) != wp )
					{
						r.setLeft ( qMin ( x,r.left() ) );
						break;
					}
				}
			}
			r.setRight ( 0 );
			for ( int y ( 0 ); y < ch; ++y )
			{

				for ( int x ( cw -1 );x >= 0; --x )
				{
					if ( cImg.pixel ( x,y ) != wp )
					{
						r.setRight ( qMax ( x,r.right() ) );
						break;
					}
				}
			}
			QImage adjustedImg ( cImg.copy ( r ).scaled ( refImage.width(),refImage.height() ) );
			PuzzleViewImp comp ( adjustedImg , QColor ( Qt::black ).rgb() );
			double compResult ( ref.CompMean ( comp ) );
// 				qDebug() <<((result > 1000.0)?"\t":"*")<< result << fit->fancyName();
			if ( compResult >= 0.0 )
			{
				if ( compResult < m_matchLimit )
				{
					if ( !filteredFonts.contains ( fit ) )
					{
						compView->setEnabled ( true );
						compView->setImage ( QPixmap::fromImage ( adjustedImg ) );
						compView->setEnabled ( false );
						scoreLabel->setText ( tr ( "The font %1 scores %2.\nDo you want to add it to the filtered fonts?" )
						                      .arg ( fit->fancyName() )
						                      .arg ( compResult ) );
						buttonBox->setEnabled ( true );
						waitingFont = fit;
						m_waitingForButton = true;
						return;
					}
				}
			}
		}
	}


	if ( filteredFonts.count() > 0 )
	{
		typotek::getInstance()->getTheMainView()->setCurFonts ( filteredFonts );
	}
	else
	{
		QMessageBox::information ( this, "Fontmatrix", tr ( "No font matches the submitted image" ) );
	}
	close();
}

void FMMatchRaster::recordCurrentRect ( QRect r )
{
	curRect = r;
}

void FMMatchRaster::recordCurrentColor ( QRgb c )
{
	curCol = c;
}

void FMMatchRaster::slotAcceptFont()
{
	if(!filteredFonts.contains(waitingFont))
		filteredFonts << waitingFont;
	waitingFont = 0;
	search();
}

void FMMatchRaster::slotRefuseFont()
{
	waitingFont = 0;
	search();
}

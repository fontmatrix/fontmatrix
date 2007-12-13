//
// C++ Implementation: fontbook
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "fontbook.h"
#include "fontbookdialog.h"
#include "typotek.h"
#include "fontitem.h"

#include <QDebug>
#include <QProgressDialog>

FontBook::FontBook()
{
}


FontBook::~FontBook()
{
}




void FontBook::doBook()
{
	FontBookDialog bookOption ( typotek::getInstance() );
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


	QList<FontItem*> localFontMap = typotek::getInstance()->getCurrentFonts();
	QMap<QString, QList<FontItem*> > keyList;
	for ( int i=0; i < localFontMap.count();++i )
	{
		keyList[localFontMap[i]->value ( "family" ) ].append ( localFontMap[i] );
// 		qDebug() << localFontMap[i]->value ( "family" ) ;
	}

	QMap<QString, QList<FontItem*> >::const_iterator kit;
	QProgressDialog progress ( "Creating font book... ", "cancel", 0, keyList.count(), typotek::getInstance() );
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


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
#include <QObject>
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

	if ( bookOption.getFileName().isEmpty() )
		return;

	outputFilePath = bookOption.getFileName();

	if ( bookOption.isTemplate() )
	{
		doBookFromTemplate ( bookOption.getTemplate() );
		return;
	}

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
	QProgressDialog progress ( QObject::tr ( "Creating font book... " ), QObject::tr ( "cancel" ), 0, keyList.count(), typotek::getInstance() );
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
			edgeCartouche = theScene.addRect ( QRectF ( QPointF ( pageWidth * 0.85 , 0.0 ),  QSizeF ( pageWidth * 0.15, pageHeight ) ) ,abigwhitepen, Qt::lightGray );

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
// 			edgeCartouche = theScene.addRect ( pageWidth * 0.85 + 10.0 , 0.0 - 10.0,  pageWidth * 0.15, pageHeight + 20.0 ,abigwhitepen, Qt::lightGray );
			edgeCartouche = theScene.addRect ( QRectF ( QPointF ( pageWidth * 0.85 , 0.0 ),  QSizeF ( pageWidth * 0.15, pageHeight ) ) ,abigwhitepen, Qt::lightGray );
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


// 				edgeCartouche = theScene.addRect ( pageWidth * 0.85 + 10.0 , 0.0 - 10.0,  pageWidth * 0.15, pageHeight + 20.0 ,abigwhitepen, Qt::lightGray );
				edgeCartouche = theScene.addRect ( QRectF ( QPointF ( pageWidth * 0.85 , 0.0 ),  QSizeF ( pageWidth * 0.15, pageHeight ) ) ,abigwhitepen, Qt::lightGray );
				edgeCartouche->setZValue ( 101.0 );

				ABC->setZValue ( 10000.0 );
				ABC->setDefaultTextColor ( Qt::black );
			}
			pen.rx() =variantnameTab;
			FontItem* curfi = kit.value() [n];
// 			qDebug() << "\tRENDER" << kit.key() << curfi->variant();
			renderedFont.append ( curfi );
			bool oldRast = curfi->rasterFreetype();
			curfi->setFTRaster ( false );
			curfi->setRTL ( false );
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
			curfi->setFTRaster ( oldRast );
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

void FontBook::doBookFromTemplate (const QDomDocument &aTemplate )
{
	// TODO

	/**
	We build lists of contexts
	*/
	QList<FontBookContext> conPage;
	QList<FontBookContext> conFamily;
	QList<FontBookContext> conSubfamily;

	QDomNodeList conList = aTemplate.elementsByTagName ( "context" );
	if ( conList.length() == 0 )
	{
		qDebug ( ) << "ERROR: "<< conList.length() <<" context in template, see yourself :" ;
		qDebug() << aTemplate.toString(1);
		return;
	}
	
	QMap<QString, QFont> qfontCache; // abit of optim.
	for ( uint i = 0; i < conList.length(); ++i )
	{
		FontBookContext fbc;
		QDomNode context = conList.item ( i );
		QString levelString = context.toElement().attributeNode ( "level" ).value();

		fbc.textElement.e = context.namedItem ( "text" ).toElement().text();
		QString textInternalString = context.namedItem ( "text" ).toElement().attributeNode ( "internal" ).value();
		fbc.textElement.internal = ( textInternalString == "true" ) ? true : false;

		QDomNode tStyle =  context.namedItem ( "textstyle" );
		fbc.textStyle.name = tStyle.toElement().attributeNode ( "name" ).value();
		fbc.textStyle.font = tStyle.namedItem ( "font" ).toElement().text();
		fbc.textStyle.fontsize = QString ( tStyle.namedItem ( "fontsize" ).toElement().text() ).toDouble() ;
		
		qfontCache[fbc.textStyle.name] = QFont(fbc.textStyle.font,fbc.textStyle.fontsize );
		
		fbc.textStyle.lineheight = QString ( tStyle.namedItem ( "lineheight" ).toElement().text() ).toDouble() ;
		fbc.textStyle.margin_top = QString ( tStyle.namedItem ( "margintop" ).toElement().text() ).toDouble() ;
		fbc.textStyle.margin_left = QString ( tStyle.namedItem ( "marginleft" ).toElement().text() ).toDouble() ;
		fbc.textStyle.margin_bottom = QString ( tStyle.namedItem ( "marginbottom" ).toElement().text() ).toDouble() ;
		fbc.textStyle.margin_right = QString ( tStyle.namedItem ( "marginright" ).toElement().text() ).toDouble() ;

		if ( levelString == "page" )
			conPage << fbc;
		else if ( levelString == "family" )
			conFamily << fbc;
		else if ( levelString == "subfamily" )
			conSubfamily << fbc;
	}

	double paperWidth  = QString ( aTemplate.documentElement () .namedItem ( "paperwidth" ).toElement().text() ).toDouble();
	double paperHeight = QString ( aTemplate.documentElement () .namedItem ( "paperheight" ).toElement().text() ).toDouble();
	
	QPrinter thePrinter ( QPrinter::HighResolution );
	thePrinter.setOutputFormat ( QPrinter::PdfFormat );
	thePrinter.setOutputFileName ( outputFilePath );
	QGraphicsScene theScene;
	QRectF pageRect ( 0,0,paperWidth,  paperHeight );
	theScene.setSceneRect ( pageRect );
	QPainter thePainter ( &thePrinter );
	QPointF thePos ( 0,0 );
	QList<FontItem*> renderedFont;
	
	

	QList<FontItem*> localFontMap = typotek::getInstance()->getCurrentFonts();
	QMap<QString, QList<FontItem*> > keyList;
	for ( int i=0; i < localFontMap.count();++i )
	{
		keyList[localFontMap[i]->value ( "family" ) ].append ( localFontMap[i] );
	}

	QMap<QString, QList<FontItem*> >::const_iterator kit;
	QProgressDialog progress ( QObject::tr ( "Creating font book... " ), QObject::tr ( "cancel" ), 0, keyList.count(), typotek::getInstance() );
	progress.setWindowModality ( Qt::WindowModal );
	int progressindex=0;

	{
		QString pageNumStr;
		int pageNumber = 0;
		///We begin in a PAGE context
// 		qDebug() << "PAGE";
		pageNumStr.setNum ( ++pageNumber );
		for ( int pIndex = 0; pIndex < conPage.count(); ++pIndex )
		{
			QStringList plines;
			if ( !conPage[pIndex].textElement.internal )
				plines = conPage[pIndex].textElement.e.split ( "\n" );
			else
			{
				QString place = conPage[pIndex].textElement.e;
				if ( place == "PageNumber" )
					plines << pageNumStr;
			}
// 			QFont pFont ( conPage[pIndex].textStyle.font, conPage[pIndex].textStyle.fontsize );
			for ( int l = 0; l < plines.count(); ++l )
			{
				QGraphicsTextItem * ti = theScene.addText ( plines[l], qfontCache[conPage[pIndex].textStyle.name] );
				ti->setPos ( conPage[pIndex].textStyle.margin_left, conPage[pIndex].textStyle.margin_top + ( l * conPage[pIndex].textStyle.lineheight ) );
			}
		}
		
		for ( kit = keyList.begin(); kit != keyList.end(); ++kit )
		{
			/// We are in a FAMILY context
// 			qDebug() << "FAMILY";
			{
				if ( progress.wasCanceled() )
					break;
				progress.setLabelText ( kit.key() );
				progress.setValue ( ++progressindex );
			}

			for ( int elemIndex = 0; elemIndex < conFamily.count() ; ++elemIndex )
			{
				// First, is there enough room for this element
				QStringList lines;
				if ( !conFamily[elemIndex].textElement.internal )
					lines = conFamily[elemIndex].textElement.e.split ( "\n" );
				else
				{
					QString place = conFamily[elemIndex].textElement.e;
					if ( place == "Family" )
						lines << kit.key();
					// TODO give access to more infos about family
				}

				double available = paperHeight - thePos.y();
				double needed = ( lines.count() * conFamily[elemIndex].textStyle.lineheight )
				                + conFamily[elemIndex].textStyle.margin_top
				                + conFamily[elemIndex].textStyle.margin_bottom;
				
				if ( needed > available )
				{
					/// We are in a PAGE context
// 					qDebug() << "PAGE";
					// close, clean and create
					theScene.render ( &thePainter );

					thePos.ry() = 0;
					for ( int  d = 0; d <  renderedFont.count() ; ++d )
						renderedFont[d]->deRenderAll();
					renderedFont.clear();
					theScene.removeItem ( theScene.createItemGroup ( theScene.items() ) );

					thePrinter.newPage();
					pageNumStr.setNum ( ++pageNumber );

					//
					for ( int pIndex = 0; pIndex < conPage.count(); ++pIndex )
					{
						QStringList plines;
						if ( !conPage[pIndex].textElement.internal )
							plines = conPage[pIndex].textElement.e.split ( "\n" );
						else
						{
							QString place = conPage[pIndex].textElement.e;
							if ( place == "PageNumber" )
								plines << pageNumStr;
						}
// 						QFont pFont ( conPage[pIndex].textStyle.font, conPage[pIndex].textStyle.fontsize );
						for ( int l = 0; l < plines.count(); ++l )
						{
							QGraphicsTextItem * ti = theScene.addText ( plines[l], qfontCache[conPage[pIndex].textStyle.name] );
							ti->setPos ( conPage[pIndex].textStyle.margin_left, conPage[pIndex].textStyle.margin_top + ( l * conPage[pIndex].textStyle.lineheight ) );
						}
					}
				}

// 				QFont aFont ( conFamily[elemIndex].textStyle.font,conFamily[elemIndex].textStyle.fontsize );
				for ( int l = 0; l < lines.count(); ++l )
				{
					QGraphicsTextItem * ti = theScene.addText ( lines[l], qfontCache[conFamily[elemIndex].textStyle.name] );
					ti->setPos ( conFamily[elemIndex].textStyle.margin_left, thePos.y() +
							( conFamily[elemIndex].textStyle.margin_top + ( l * conFamily[elemIndex].textStyle.lineheight ) ) );
				}

				thePos.ry() += needed;
			} // end of FAMILY level elements

			for ( int fontIndex = 0;fontIndex < kit.value().count(); ++fontIndex )
			{
				FontItem * theFont = kit.value() [fontIndex];
				renderedFont.append ( theFont );
				bool oldRast = theFont->rasterFreetype();
				theFont->setFTRaster ( false );
				theFont->setRTL ( false );
				/// We are in a SUBFAMILY context
// 				qDebug() << "SUBFAMILY";
				for ( int elemIndex = 0; elemIndex < conSubfamily.count() ; ++elemIndex )
				{
					// First, is there enough room for this element
					QStringList lines;
					if ( !conSubfamily[elemIndex].textElement.internal )
					{
						lines = conSubfamily[elemIndex].textElement.e.split ( "\n" , QString::SkipEmptyParts );
					}
					else
					{
						QString place = conSubfamily[elemIndex].textElement.e;
						if ( place == "Family" )
							lines << theFont->family();
						else if ( place == "SubFamily" )
							lines << theFont->variant();
						else if( place == "FullName")
							lines << theFont->fancyName();
						// more to come
					}
					
					double available = paperHeight - thePos.y();
					double needed = ( lines.count() * conSubfamily[elemIndex].textStyle.lineheight )
					                + conSubfamily[elemIndex].textStyle.margin_top
					                + conSubfamily[elemIndex].textStyle.margin_bottom;
					if ( needed > available )
					{
						/// We are in a PAGE context
// 						qDebug() << "PAGE";
						// close, clean and create
						theScene.render ( &thePainter );

						thePos.ry() = 0;
						for ( int  d = 0; d <  renderedFont.count() ; ++d )
							renderedFont[d]->deRenderAll();
						renderedFont.clear();
						theScene.removeItem ( theScene.createItemGroup ( theScene.items() ) );

						thePrinter.newPage();
						pageNumStr.setNum ( ++pageNumber );

						//
						for ( int pIndex = 0; pIndex < conPage.count(); ++pIndex )
						{
							QStringList plines;
							if ( !conPage[pIndex].textElement.internal )
								plines = conPage[pIndex].textElement.e.split ( "\n" );
							else
							{
								QString place = conPage[pIndex].textElement.e;
								if ( place == "PageNumber" )
									plines << pageNumStr;
							}
// 							QFont pFont ( conPage[pIndex].textStyle.font, conPage[pIndex].textStyle.fontsize );
							for ( int l = 0; l < plines.count(); ++l )
							{
								QGraphicsTextItem * ti = theScene.addText ( plines[l], qfontCache[conPage[pIndex].textStyle.name]);
								ti->setPos ( conPage[pIndex].textStyle.margin_left, conPage[pIndex].textStyle.margin_top + ( l * conPage[pIndex].textStyle.lineheight ) );
							}
						}
					}
					
					if ( conSubfamily[elemIndex].textStyle.font == "_FONTMATRIX_" ) // We’ll use the current font
					{
						for ( int l = 0; l < lines.count(); ++l )
						{
							QPointF pen ( conSubfamily[elemIndex].textStyle.margin_left,
									thePos.y() + 
							              (conSubfamily[elemIndex].textStyle.margin_top + ( ( l + 1 ) * conSubfamily[elemIndex].textStyle.lineheight ) ));
							theFont->renderLine ( &theScene,
							                      lines[l],
							                      pen ,
							                      conSubfamily[elemIndex].textStyle.fontsize );
						}
					}
					else
					{
// 						QFont aFont ( conSubfamily[elemIndex].textStyle.font,conSubfamily[elemIndex].textStyle.fontsize );
						for ( int l = 0; l < lines.count(); ++l )
						{
							QGraphicsTextItem * ti = theScene.addText ( lines[l], qfontCache[ conSubfamily[elemIndex].textStyle.name] );
							ti->setPos ( conSubfamily[elemIndex].textStyle.margin_left, thePos.y() + (conSubfamily[elemIndex].textStyle.margin_top + ( l * conSubfamily[elemIndex].textStyle.lineheight )) );
						}
					}

					theFont->setFTRaster ( oldRast );
					thePos.ry() += needed;
				} // end of SUBFAMILY level elements

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





}


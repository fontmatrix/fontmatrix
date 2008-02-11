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
#include <QSvgRenderer>
#include <QGraphicsSvgItem>

FontBook::FontBook()
{
	mapPSize[ "A0" ] = QPrinter::A0 ;
	mapPSize[ "A1" ] = QPrinter::A1 ;
	mapPSize[ "A2" ] = QPrinter::A2 ;
	mapPSize[ "A3" ] = QPrinter::A3 ;
	mapPSize[ "A4" ] = QPrinter::A4 ;
	mapPSize[ "A5" ] = QPrinter::A5 ;
	mapPSize[ "A6" ] = QPrinter::A6 ;
	mapPSize[ "A7" ] = QPrinter::A7 ;
	mapPSize[ "A8" ] = QPrinter::A8 ;
	mapPSize[ "A9" ] = QPrinter::A9 ;
	mapPSize[ "B0" ] = QPrinter::B0 ;
	mapPSize[ "B1" ] = QPrinter::B1 ;
	mapPSize[ "B10" ] = QPrinter::B10 ;
	mapPSize[ "B2" ] = QPrinter::B2 ;
	mapPSize[ "B3" ] = QPrinter::B3 ;
	mapPSize[ "B4" ] = QPrinter::B4 ;
	mapPSize[ "B5" ] = QPrinter::B5 ;
	mapPSize[ "B6" ] = QPrinter::B6 ;
	mapPSize[ "B7" ] = QPrinter::B7 ;
	mapPSize[ "B8" ] = QPrinter::B8 ;
	mapPSize[ "B9" ] = QPrinter::B9 ;
	mapPSize[ "Letter" ] = QPrinter::Letter ;
	mapPSize[ "Tabloid" ] = QPrinter::Tabloid ;
	mapPSize[ "Custom" ] = QPrinter::Custom ;


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

}

void FontBook::doBookFromTemplate ( const QDomDocument &aTemplate )
{
	

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
		qDebug() << aTemplate.toString ( 1 );
		return;
	}

	QMap<QString, QSvgRenderer*> svgRendered;
	QMap<QString, QFont> qfontCache; // abit of optim.
	for ( uint i = 0; i < conList.length(); ++i )
	{
		FontBookContext fbc;
		QDomNode context = conList.item ( i );
		QString levelString = context.toElement().attributeNode ( "level" ).value();

		fbc.textElement.e = context.namedItem ( "text" ).toElement().text();
		if(!fbc.textElement.e.isEmpty())
		{
	// 		QString textInternalString = context.namedItem ( "text" ).toElement().attributeNode ( "internal" ).value();
	// 		fbc.textElement.internal = ( textInternalString == "true" ) ? true : false;
			fbc.textElement.valid = true;
					
			QDomNode tStyle =  context.namedItem ( "textstyle" );
			fbc.textStyle.name = tStyle.toElement().attributeNode ( "name" ).value();
			fbc.textStyle.font = tStyle.namedItem ( "font" ).toElement().text();
			fbc.textStyle.fontsize = QString ( tStyle.namedItem ( "fontsize" ).toElement().text() ).toDouble() ;
			fbc.textStyle.color = QColor( tStyle.namedItem ( "color" ).toElement().text() );
			
			bool ital = false;
			QFont::Weight bold = QFont::Normal;
			if(fbc.textStyle.font.contains("italic", Qt::CaseInsensitive))
				ital = true;
			if(fbc.textStyle.font.contains("bold", Qt::CaseInsensitive))
				bold = QFont::Bold;
			qfontCache[fbc.textStyle.name] = QFont ( fbc.textStyle.font,fbc.textStyle.fontsize, bold, ital );
	
			fbc.textStyle.lineheight = QString ( tStyle.namedItem ( "lineheight" ).toElement().text() ).toDouble() ;
			fbc.textStyle.margin_top = QString ( tStyle.namedItem ( "margintop" ).toElement().text() ).toDouble() ;
			fbc.textStyle.margin_left = QString ( tStyle.namedItem ( "marginleft" ).toElement().text() ).toDouble() ;
			fbc.textStyle.margin_bottom = QString ( tStyle.namedItem ( "marginbottom" ).toElement().text() ).toDouble() ;
			fbc.textStyle.margin_right = QString ( tStyle.namedItem ( "marginright" ).toElement().text() ).toDouble() ;
		}
		
		QDomNode graphicNode = context.namedItem ( "graphic" );
		if(graphicNode.isElement())
		{
// 			QDomDocumentFragment svgFrag = aTemplate.createDocumentFragment();
			QDomNode svgNode = graphicNode.toElement().namedItem("svg").cloneNode(true);
// 			svgFrag.appendChild(svgNode);
// 			if(svgNode.isElement())
			{
				fbc.graphic.name =  graphicNode.toElement().attributeNode("name").value();
				fbc.graphic.x = QString ( graphicNode.toElement().attributeNode("xpos").value()).toDouble();
				fbc.graphic.y = QString ( graphicNode.toElement().attributeNode("ypos").value()).toDouble();
				
				QDomDocument svgDoc;
				QDomNode svg = svgDoc.importNode(svgNode,true);
				svgDoc.appendChild(svg);
				QString svgString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" + svgDoc.toString(0));
				QSvgRenderer *doc = new QSvgRenderer( svgString.toUtf8());
				svgRendered[fbc.graphic.name] = doc;
				fbc.graphic.valid = true;
// 				qDebug() << fbc.graphic.svg;
			}
			
		}
		

		if ( levelString == "page" )
			conPage << fbc;
		else if ( levelString == "family" )
			conFamily << fbc;
		else if ( levelString == "subfamily" )
			conSubfamily << fbc;
	}
	
	
	QString paperSize = QString (aTemplate.documentElement().namedItem( "papersize" ).toElement().text()).toUpper();
	double prectx =  QString (aTemplate.documentElement().namedItem( "papersize" ).toElement().attributeNode("bboxx").value()).toDouble();
	double precty =  QString (aTemplate.documentElement().namedItem( "papersize" ).toElement().attributeNode("bboxy").value()).toDouble();
	double prectw =  QString (aTemplate.documentElement().namedItem( "papersize" ).toElement().attributeNode("bboxw").value()).toDouble();
	double precth =  QString (aTemplate.documentElement().namedItem( "papersize" ).toElement().attributeNode("bboxh").value()).toDouble();
	
	QPrinter thePrinter ( QPrinter::HighResolution );
	thePrinter.setOutputFormat ( QPrinter::PdfFormat );
	thePrinter.setCreator("Fontmatrix " + QString::number(FONTMATRIX_VERSION_MAJOR) + "." + QString::number(FONTMATRIX_VERSION_MINOR));
	thePrinter.setDocName("A font book");
	thePrinter.setOutputFileName ( outputFilePath );
	thePrinter.setPageSize ( mapPSize[paperSize] );
	thePrinter.setFullPage ( true );
// 	qDebug() << thePrinter.pageSize() << thePrinter.pageRect() << thePrinter.paperRect() << thePrinter.resolution() ;
	double paperWidth  =  thePrinter.pageRect().width() / thePrinter.resolution() * 72;
	double paperHeight =  thePrinter.pageRect().height() / thePrinter.resolution() * 72;
// 	qDebug()<< paperSize << paperWidth << paperHeight;
	QGraphicsScene theScene;
	theScene.setSceneRect ( 0,0,paperWidth,paperHeight );
	QPainter thePainter ( &thePrinter );
	QPointF thePos ( prectx,precty );
	QList<FontItem*> renderedFont;
	QList<QGraphicsSvgItem*> renderedGraphic;



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


	QString pageNumStr;
	int pageNumber = 0;
	///We begin in a PAGE context
// 	qDebug() << "PAGE";
	pageNumStr.setNum ( ++pageNumber );
	QMap<QString,QString> pageReplace;
	QMap<QString,QString> familyReplace;
	QMap<QString,QString> subfamilyReplace;
	
	QString currentFamily;
	QString currentSubfamily;
	
	/** Z policy is
		PAGE_SVG 1
		PAGE_TEXT 10
		FAMILY_SVG 100
		FAMILY_TEXT 1000
		SUBFAMILY_SVG 10000
		SUBFAMILY_TEXT 100000
	*/

	for ( int pIndex = 0; pIndex < conPage.count(); ++pIndex )
	{
		if(conPage[pIndex].textElement.valid)
		{
			QStringList pagelines ;
			QStringList tmplines = conPage[pIndex].textElement.e.split ( "\n" );
			pageReplace["##PAGENUMBER##"] = pageNumStr;
			pageReplace["##FAMILY##"] = currentFamily;
			pageReplace["##SUBFAMILY##"] = currentSubfamily;
			for ( int t = 0; t < tmplines.count(); ++t )
			{
	
				QString place = tmplines[t];
				for ( QMap<QString,QString>::const_iterator repIt = pageReplace.begin(); repIt != pageReplace.end();   ++repIt )
					place.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
				pagelines << place;
			}
	// 			QFont pFont ( conPage[pIndex].textStyle.font, conPage[pIndex].textStyle.fontsize );
			for ( int pl = 0; pl < pagelines.count(); ++pl )
			{
				QGraphicsTextItem * ti = theScene.addText ( pagelines[pl], qfontCache[conPage[pIndex].textStyle.name] );
				ti->setPos ( conPage[pIndex].textStyle.margin_left , conPage[pIndex].textStyle.margin_top + ( pl * conPage[pIndex].textStyle.lineheight ) );
				ti->setZValue(10);
				ti->setDefaultTextColor ( conPage[pIndex].textStyle.color );
			}
		}
		if(conPage[pIndex].graphic.valid)
		{
				QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
				svgIt->setSharedRenderer(svgRendered[conPage[pIndex].graphic.name]);
				theScene.addItem(svgIt);
				svgIt->setPos(conPage[pIndex].graphic.x, conPage[pIndex].graphic.y);
				renderedGraphic << svgIt;
				svgIt->setZValue(1);
		}
	}

	/// Beginning of the big loop
	for ( kit = keyList.begin(); kit != keyList.end(); ++kit )
	{
		/// We are in a FAMILY context
// 		qDebug() << "FAMILY";
		{
			if ( progress.wasCanceled() )
				break;
			progress.setLabelText ( kit.key() );
			progress.setValue ( ++progressindex );
		}
		currentFamily = kit.key();
		for ( int elemIndex = 0; elemIndex < conFamily.count() ; ++elemIndex )
		{
			QStringList familylines;
			QStringList tmplines = conFamily[elemIndex].textElement.e.split ( "\n" );
			familyReplace["##FAMILY##"] = kit.key();
			for ( int t = 0; t < tmplines.count(); ++t )
			{

				QString place = tmplines[t];
				for ( QMap<QString,QString>::const_iterator repIt = familyReplace.begin(); repIt != familyReplace.end();   ++repIt )
					place.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
				if(!place.isEmpty())
					familylines << place;
			}

			double available = (precty + precth) - thePos.y();
			double needed = ( familylines.count() * conFamily[elemIndex].textStyle.lineheight )
			                + conFamily[elemIndex].textStyle.margin_top
			                + conFamily[elemIndex].textStyle.margin_bottom;

			if ( needed > available )
			{
				/// We are in a PAGE context
// 				qDebug() << "NFPAGE";
				// close, clean and create
				theScene.render ( &thePainter );

				thePos.ry() = precty;
				for ( int  d = 0; d <  renderedFont.count() ; ++d )
					renderedFont[d]->deRenderAll();
				for ( int  d = 0; d < renderedGraphic.count(); ++d)
					delete renderedGraphic[d];
				renderedFont.clear();
				renderedGraphic.clear();
				theScene.removeItem ( theScene.createItemGroup ( theScene.items() ) );

				thePrinter.newPage();
				pageNumStr.setNum ( ++pageNumber );

				//
				for ( int pIndex = 0; pIndex < conPage.count(); ++pIndex )
				{
					QStringList pagelines ;
					QStringList tmplines = conPage[pIndex].textElement.e.split ( "\n" );
					pageReplace["##PAGENUMBER##"] = pageNumStr;
					pageReplace["##FAMILY##"] = currentFamily;
					pageReplace["##SUBFAMILY##"] = currentSubfamily;
					for ( int t = 0; t < tmplines.count(); ++t )
					{

						QString pageplace = tmplines[t];
						for ( QMap<QString,QString>::const_iterator repIt = pageReplace.begin(); repIt != pageReplace.end();   ++repIt )
							pageplace.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
						pagelines << pageplace;
					}
// 						QFont pFont ( conPage[pIndex].textStyle.font, conPage[pIndex].textStyle.fontsize );
					for ( int pl = 0; pl < pagelines.count(); ++pl )
					{
						QGraphicsTextItem * ti = theScene.addText ( pagelines[pl], qfontCache[conPage[pIndex].textStyle.name] );
						ti->setPos ( conPage[pIndex].textStyle.margin_left + prectx, conPage[pIndex].textStyle.margin_top + ( pl * conPage[pIndex].textStyle.lineheight ) );
						ti->setZValue(10);
						ti->setDefaultTextColor( conPage[pIndex].textStyle.color);
					}
					if(conPage[pIndex].graphic.valid)
					{
						QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
						svgIt->setSharedRenderer(svgRendered[conPage[pIndex].graphic.name]);
						theScene.addItem(svgIt);
						svgIt->setPos(conPage[pIndex].graphic.x + prectx, conPage[pIndex].graphic.y);
						renderedGraphic << svgIt;
						svgIt->setZValue(1);
					}
				}
			}

// 				QFont aFont ( conFamily[elemIndex].textStyle.font,conFamily[elemIndex].textStyle.fontsize );
			for ( int fl = 0; fl < familylines.count(); ++fl )
			{
				QGraphicsTextItem * ti = theScene.addText ( familylines[fl], qfontCache[conFamily[elemIndex].textStyle.name] );
				ti->setPos ( conFamily[elemIndex].textStyle.margin_left + prectx, thePos.y() +  ( conFamily[elemIndex].textStyle.margin_top + ( fl * conFamily[elemIndex].textStyle.lineheight ) ) );
				ti->setZValue(1000);
				ti->setDefaultTextColor(conFamily[elemIndex].textStyle.color);
			}
			if(conFamily[elemIndex].graphic.valid)
			{
				QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
				svgIt->setSharedRenderer(svgRendered[conFamily[elemIndex].graphic.name]);
				theScene.addItem(svgIt);
				svgIt->setPos(conFamily[elemIndex].graphic.x + prectx, conFamily[elemIndex].graphic.y + thePos.y());
				renderedGraphic << svgIt;
				svgIt->setZValue(100);
			}

			thePos.ry() += needed;
		} // end of FAMILY level elements

		for ( int fontIndex = 0;fontIndex < kit.value().count(); ++fontIndex )
		{
// 			qDebug() << fontIndex << "/" << kit.value().count();
			FontItem * theFont = kit.value() [fontIndex];
			bool oldRast = theFont->rasterFreetype();
			theFont->setFTRaster ( false );
			theFont->setRTL ( false );
			/// We are in a SUBFAMILY context
			currentSubfamily = theFont->variant();
			for ( int elemIndex = 0; elemIndex < conSubfamily.count() ; ++elemIndex )
			{
				// First, is there enough room for this element
				QStringList sublines;
				QStringList tmplines = conSubfamily[elemIndex].textElement.e.split ( "\n" );
// 				qDebug() <<"A";
				subfamilyReplace["##FAMILY##"] = theFont->family();
				subfamilyReplace["##SUBFAMILY##"] = theFont->variant();
				subfamilyReplace["##FILE##"]= theFont->path();
				subfamilyReplace["##TAGS##"]= theFont->tags().join(";") ; 
				subfamilyReplace["##COUNT##"]= QString::number(theFont->glyphsCount());
				subfamilyReplace["##TYPE##"]= theFont->type();
				subfamilyReplace["##CHARSETS##"]=theFont->charmaps().join(";");
// 				qDebug() <<"B";
				for ( int t = 0; t < tmplines.count(); ++t )
				{

					QString subplace = tmplines[t];
					for ( QMap<QString,QString>::const_iterator repIt = subfamilyReplace.begin(); repIt != subfamilyReplace.end();   ++repIt )
						subplace.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
					if(!subplace.isEmpty())
						sublines << subplace;
				}
// 				qDebug() <<"C";
				double available =  (precty + precth) - thePos.y();
				double needed = ( sublines.count() * conSubfamily[elemIndex].textStyle.lineheight )
				                + conSubfamily[elemIndex].textStyle.margin_top
				                + conSubfamily[elemIndex].textStyle.margin_bottom;
// 				qDebug() <<"D";
				if ( needed > available )
				{
					/// We are in a PAGE context
// 					qDebug() << "NSPAGE";
					// close, clean and create
					theScene.render ( &thePainter );

					thePos.ry() = precty;
					for ( int  d = 0; d <  renderedFont.count() ; ++d )
						renderedFont[d]->deRenderAll();
					for ( int  d = 0; d < renderedGraphic.count(); ++d)
						delete renderedGraphic[d];
					renderedFont.clear();
					renderedGraphic.clear();
					theScene.removeItem ( theScene.createItemGroup ( theScene.items() ) );

					thePrinter.newPage();
					pageNumStr.setNum ( ++pageNumber );

					//
					for ( int pIndex = 0; pIndex < conPage.count(); ++pIndex )
					{
						QStringList pagelines ;
						QStringList tmplines = conPage[pIndex].textElement.e.split ( "\n" );
						pageReplace["##PAGENUMBER##"] = pageNumStr;
						pageReplace["##FAMILY##"] = currentFamily;
						pageReplace["##SUBFAMILY##"] = currentSubfamily;
						for ( int t = 0; t < tmplines.count(); ++t )
						{

							QString pageplace = tmplines[t];
							for ( QMap<QString,QString>::const_iterator repIt = pageReplace.begin(); repIt != pageReplace.end();   ++repIt )
								pageplace.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
							pagelines << pageplace;
						}
// 							QFont pFont ( conPage[pIndex].textStyle.font, conPage[pIndex].textStyle.fontsize );
						for ( int pl = 0; pl < pagelines.count(); ++pl )
						{
							QGraphicsTextItem * ti = theScene.addText ( pagelines[pl], qfontCache[conPage[pIndex].textStyle.name] );
							ti->setPos ( conPage[pIndex].textStyle.margin_left + prectx, conPage[pIndex].textStyle.margin_top + ( pl * conPage[pIndex].textStyle.lineheight ) );
							ti->setZValue(10);
							ti->setDefaultTextColor(conPage[pIndex].textStyle.color);
						}
						if(conPage[pIndex].graphic.valid)
						{
							QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
							svgIt->setSharedRenderer(svgRendered[conPage[pIndex].graphic.name]);
							theScene.addItem(svgIt);
							svgIt->setPos(conPage[pIndex].graphic.x + prectx, conPage[pIndex].graphic.y);
							renderedGraphic << svgIt;
							svgIt->setZValue(1);
						}
					}
				}
// 				else
// 					qDebug() << "NO_NSPAGE";

				if ( conSubfamily[elemIndex].textStyle.font == "_FONTMATRIX_" ) // We’ll use the current font
				{
					for ( int sl = 0; sl < sublines.count(); ++sl )
					{
						QPointF pen ( conSubfamily[elemIndex].textStyle.margin_left + prectx,
						              thePos.y() + conSubfamily[elemIndex].textStyle.margin_top +  ( (sl + 1) * conSubfamily[elemIndex].textStyle.lineheight )  );
						theFont->renderLine ( &theScene,
						                      sublines[sl],
						                      pen ,
						                      conSubfamily[elemIndex].textStyle.fontsize, 10000 );
						//TODO adding color support for text sample
					}
					renderedFont.append ( theFont );
				}
				else
				{
// 						QFont aFont ( conSubfamily[elemIndex].textStyle.font,conSubfamily[elemIndex].textStyle.fontsize );
					for ( int sl = 0; sl < sublines.count(); ++sl )
					{
						QGraphicsTextItem * ti = theScene.addText ( sublines[sl], qfontCache[ conSubfamily[elemIndex].textStyle.name] );
						ti->setPos ( conSubfamily[elemIndex].textStyle.margin_left + prectx, thePos.y() + ( conSubfamily[elemIndex].textStyle.margin_top + ( sl * conSubfamily[elemIndex].textStyle.lineheight ) ) );
						ti->setZValue(10000);
						ti->setDefaultTextColor(conSubfamily[elemIndex].textStyle.color);
					}
				}

				if(conSubfamily[elemIndex].graphic.valid)
				{
					QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
					svgIt->setSharedRenderer(svgRendered[conSubfamily[elemIndex].graphic.name]);
					theScene.addItem(svgIt);
					svgIt->setPos(conSubfamily[elemIndex].graphic.x + prectx, conSubfamily[elemIndex].graphic.y + thePos.y());
					renderedGraphic << svgIt;
					svgIt->setZValue(100000);
				}
				theFont->setFTRaster ( oldRast );
				thePos.ry() += needed;
				
			} // end of SUBFAMILY level elements
// 			qDebug() << "ENDOF_SUBFAMILY";
		}

		
	}
	if ( renderedFont.count() )
	{
		theScene.render ( &thePainter );
		for ( int  d = 0; d <  renderedFont.count() ; ++d )
		{
			renderedFont[d]->deRenderAll();

		}
		for ( int  d = 0; d < renderedGraphic.count(); ++d)
			delete renderedGraphic[d];
		renderedFont.clear();
		renderedGraphic.clear();

	}
}


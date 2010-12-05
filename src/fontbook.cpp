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
#include "fmlayout.h"
#include "fmfontdb.h"
#include "fmpaths.h"
#include "progressbarduo.h"
#include "fmvariants.h"

#include <QDebug>
#include <QImage>
#include <QObject>
#include <QProgressDialog>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QPrintDialog>
#include <QFile>
#include <QTime>
#include <QGraphicsTextItem>

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



	QFile sf(FMPaths::ResourcesDir()+ QString("fontbook.strings"));
	if(sf.open(QIODevice::ReadOnly))
	{
		QString ss(QString::fromUtf8(sf.readAll()));
		stringList = ss.split("\n", QString::SkipEmptyParts);
	}
	else
	{
		stringList << QString("Call me Ishmael. Some years ago —never mind how long precisely— having little or no money in my purse, and nothing particular to interest me on shore, I thought I would sail about a little and see the watery part of the world.");
		stringList << QString("It is a way I have of driving off the spleen and regulating the circulation.");
		stringList << QString("Whenever I find myself growing grim about the mouth; whenever it is a damp, drizzly November in my soul; whenever I find myself involuntarily pausing before coffin warehouses, and bringing up the rear of every funeral I meet; and especially whenever my hypos get such an upper hand of me, that it requires a strong moral principle to prevent me from deliberately stepping into the street, and methodically knocking people's hats off —then, I account it high time to get to sea as soon as I can.");
		stringList << QString("This is my substitute for pistol and ball.");
		stringList << QString("With a philosophical flourish Cato throws himself upon his sword; I quietly take to the ship.");
		stringList << QString("There is nothing surprising in this. If they but knew it, almost all men in their degree, some time or other, cherish very nearly the same feelings towards the ocean with me.");
	}
}


FontBook::~FontBook()
{
}




void FontBook::doBook(FontBook::Style s)
{
	printer = new QPrinter( QPrinter::HighResolution );
	printerRect = printer->paperRect(QPrinter::Point);
	QPrintDialog dialog(printer);
	dialog.setWindowTitle("Fontmatrix - " + tr("Print Fontbook"));

	if ( dialog.exec() != QDialog::Accepted )
		return;

	printer->setFullPage ( true );
	painter = new QPainter( printer );
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setRenderHint(QPainter::TextAntialiasing, true);

	switch(s)
	{
	case Full: doFullBook();
		break;
	case OneLiner: doOneLinerBook();
		break;
	default: break;
	 }

	delete painter;
	delete printer;
}

void FontBook::doFullBook()
{
	// Full book
	// <img>

	progress = new ProgressBarDuo(typotek::getInstance());

	doFullBookCover();

	progress->setLabel(QString("-"), 0);
	progress->setLabel(QString("-"), 1);
	progress->setMax(FMFontDb::DB()->getFilteredFonts(true).count(), 0);
	progress->setMax(0, 1);
	progress->show();

	int familyCounter(0);
	foreach(FontItem * family, FMFontDb::DB()->getFilteredFonts(true))
	{
		progress->setLabel(family->family(), 0);
		progress->setValue(++familyCounter, 0);
		printer->newPage();
		if(doFullBookPageLeft(family->family()))
		{
			printer->newPage();
			doFullBookPageRight(family->family());
		}
	}

	delete progress;
}

void FontBook::doFullBookCover()
{
	QGraphicsScene pScene(printerRect);

	QFont aFont;
	aFont.setPointSizeF(26.0);
	QGraphicsSimpleTextItem * title(pScene.addSimpleText(QString("Fontmatrix"),aFont));
	title->setPos(60, printerRect.height()*.9);

	int module(250);
	double x(0);
	double y(0);
	double fsize(qrand() % module);
	int gray(qrand() % 160);
	foreach(FontItem * f, FMFontDb::DB()->getFilteredFonts())
	{
		int lc(f->lastChar());
		int charcode(qrand() % lc);
		while(!f->hasCharcode(charcode))
			charcode = qrand() % lc;

		QGraphicsPathItem *p(f->itemFromChar(charcode, fsize));
		if(p->data(GLYPH_DATA_ERROR).toBool())
		{
			delete p;
			continue;
		}
		pScene.addItem(p);
		p->setPos(x + (qrand() % qRound(printerRect.width())), y + (qrand() % qRound(printerRect.height())));
		p->setBrush(QColor(gray,  gray,  gray));
		p->setPen(Qt::NoPen);
		gray = qrand() % 160;
		fsize = qrand() % module;
	}

	pScene.render(painter, printer->paperRect(), printerRect);
}

void FontBook::doFullBookPageRight(const QString &family)
{
	qDebug()<<"=>"<<family;
	QList<FontItem*> familyFonts = FMFontDb::DB()->FamilySet(family);

	QRectF halfPage(printerRect);
	halfPage.setHeight(halfPage.height() * 0.7);
	QGraphicsScene tmpScene(halfPage);
	QGraphicsScene pScene(halfPage);

	/** BEGINING OF decorative multi sized samples */

	QMap<int , double> logWidth;
	QMap<int , double> logAscend;
	QMap<int , double> logDescend;
	QMap<int , QString> sampleString;
	QMap<int , FontItem*> sampleFont;

	QString iString(stringList.join(" "));
	QStringList stl;
	QList<int> sizes;
	if(familyFonts.count() > 1)
	{
		int module(familyFonts.count() * 2);
		int idxS( qrand() %  qMax(1, iString.count() / 3) );
		int idxE( qMax( qMax(2,familyFonts.count()), qrand() % module) );
		while(stl.count() < familyFonts.count())
		{
			if((idxS + idxE) < iString.count())
			{
				QString t(iString.mid(idxS,idxE).simplified());

				if((t.count() >= 2)
					&& !stl.contains(t)
					&& !sizes.contains(t.count()))
					{
					qDebug()<<"\t"<<t;
					sizes << t.count();
					stl << t;
				}
				idxS = idxE;
				idxE = qMax( familyFonts.count(), qrand() % module);
			}
			else
			{
				idxS = qrand() %  (qMax(1, iString.count() / 3));
				idxE = qMax( familyFonts.count(), qrand() % module);
			}
		}
	}
	else // mono variant families can make the algo above to loop
	{
		QStringList monoList;
		monoList << "Ab" << "Cd" << "Ef" << "Gh" << "Ij" << "Kl" << "Mn" << "Op" << "Qr" << "St" << "Uv" << "Xy" << "Za";
		stl << monoList.at(qrand() % monoList.count());
	}
	int diff ( familyFonts.count()  );
	for (int i(0); i < diff; ++i)
	{
		sampleString[i] = stl[ i/* % stl.count() */];
	}

	// first we’ll get widths for font size 1000
	for(int fidx(0); fidx < familyFonts.count(); ++fidx)
	{
		sampleFont[fidx] = familyFonts[fidx];
		bool rasterState(sampleFont[fidx]->rasterFreetype());
		sampleFont[fidx]->setFTRaster(false);
		sampleFont[fidx]->setRenderReturnWidth(true);
		logWidth[fidx] =  familyFonts[fidx]->renderLine(&tmpScene, sampleString[fidx], QPointF(0.0, 1000.0) , 999999.0, 1000.0, 1) ;
		sampleFont[fidx]->setRenderReturnWidth(false);
		sampleFont[fidx]->setFTRaster(rasterState);
		logAscend[fidx] = 1000.0 - tmpScene.itemsBoundingRect().top();
		logDescend[fidx] = tmpScene.itemsBoundingRect().bottom() - 1000.0;
//		qDebug()<< sampleString[fidx] << logWidth[fidx];
		QList<QGraphicsItem*> lgit(tmpScene.items());
		foreach(QGraphicsItem* git, lgit)
		{
			tmpScene.removeItem(git);
			delete git;
		}
	}
	const double defWidth( 480.0 );
	const double defHeight( 570.0 );
	const double xOff( 0.5 * (printerRect.width() - defWidth) );
	double yPos( 33.0 );

	QFont nameFont;
	nameFont.setPointSizeF(5.0);

	for(int fidx(0); fidx < familyFonts.count(); ++fidx)
	{
		double fSize( (defWidth * 1000.0) /  logWidth[fidx]);
		double fAscend(logAscend[fidx] * fSize / 1000.0);
		double fDescend(logDescend[fidx] * fSize  / 1000.0 );
		if( (fidx > 0)  && ((yPos + fAscend + fDescend) > defHeight))
			break;

		yPos +=  fAscend;
		QPointF origine(xOff,  yPos );

//		qDebug()<< sampleString[fidx] << fSize;

		bool rasterState(sampleFont[fidx]->rasterFreetype());
		sampleFont[fidx]->setFTRaster(false);
		sampleFont[fidx]->renderLine(&pScene,
					     sampleString[fidx],
					     origine,
					     printerRect.width() ,
					     fSize,
					     0);
//		pScene.addLine(QLineF(origine, QPointF(xOff + defWidth, yPos)));
		sampleFont[fidx]->setFTRaster(rasterState);

		yPos += 4.0;
		QGraphicsSimpleTextItem * nameText = pScene.addSimpleText( QString("%1 %2pt").arg(familyFonts[fidx]->variant())
									   .arg((fSize > 16.0) ? qRound(fSize) : (fSize, 0, 'f', 1)),
									   nameFont) ;
		nameText->setPos(xOff, yPos);
		nameText->setBrush(Qt::gray);

		yPos += fDescend ;
		yPos += nameText->boundingRect().height();
	}
	pScene.addLine(QLineF(QPointF(xOff, defHeight + 30.0), QPointF(xOff + defWidth,  defHeight + 30.0)));
	pScene.render(painter, printer->paperRect(), printerRect);

	/** END OF decorative multi sized samples */

	/** BEGINING of paragraph preview */
	//lets setup a double column layout
	QRectF colRectLayoutLeft(printerRect);
	colRectLayoutLeft.setHeight(142.0);
	colRectLayoutLeft.setWidth(200.0);
	QRectF colRectLayoutRight(printerRect);
	colRectLayoutRight.setHeight(142.0);
	colRectLayoutRight.setWidth(260.0);
	QRectF colLeftRect(colRectLayoutLeft);
	colLeftRect.translate(xOff, 630.0);
	QRectF colRightRect(colRectLayoutRight);
	colRightRect.translate(xOff + 220.0, 630.0);

	// Select a font, regular preferred
	FontItem * rFont(FMVariants::Preferred(familyFonts));

	// Lets layout !

	double littleSize(7.5);
	double bigSize(10.0);
	QGraphicsScene layoutLeftScene(printerRect);
	QGraphicsScene layoutRightScene(printerRect);
	FMLayout *layoutLeft = new  FMLayout(&layoutLeftScene , rFont, colLeftRect);
	FMLayout *layoutRight = new  FMLayout(&layoutRightScene , rFont, colRightRect);
	layoutLeft->setDeviceIndy(true);
	layoutLeft->setAdjustedSampleInter( littleSize*1.2 );
//	layoutLeft->setRect(colLeftRect);
	layoutRight->setDeviceIndy(true);
	layoutRight->setAdjustedSampleInter( bigSize*1.2 );
//	layoutRight->setRect(colRightRect);

	bool rasterState(rFont->rasterFreetype());
	rFont->setFTRaster(false);
	QList<GlyphList> lgl;
	foreach(QString s, stringList)
	{
		lgl << rFont->glyphs(s, littleSize);
	}
	layoutLeft->doLayout(lgl, littleSize);
	lgl.clear();
	foreach(QString s, stringList)
	{
		lgl << rFont->glyphs(s, bigSize);
	}
	layoutRight->doLayout(lgl, bigSize);

	rFont->setFTRaster(rasterState);

//	qDebug()<<"R"<<colRectLayout<<colLeftRect<<layoutLeft->getRect()<<printerRect;
	layoutLeftScene.render(painter, printer->paperRect(), printerRect);
	layoutRightScene.render(painter, printer->paperRect(), printerRect);
	/** END of paragraph preview */


	// DEBUG
//	QImage image(printer->paperRect(QPrinter::Point).size().toSize(), QImage::Format_ARGB32);
//	QPainter debugPainter(&image);
//	pScene.render(&debugPainter);
//	layoutLeftScene.render(&debugPainter);
//	layoutRightScene.render(&debugPainter);
//	image.save(QString("/home/pierre/debug_%1.png").arg(QTime::currentTime().msec()));
	////////

	delete layoutLeft;
	delete layoutRight;
}

bool FontBook::doFullBookPageLeft(const QString &family)
{
	// TODOs
	QList<FontItem*> familyFonts = FMVariants::Order(FMFontDb::DB()->FamilySet(family));
	QGraphicsScene pScene(printerRect);
	QFont nameFont;
	nameFont.setPointSizeF(18.0);
	QGraphicsSimpleTextItem * familyText( pScene.addSimpleText( QString("%1").arg(family), nameFont) );
	familyText->setPos(printerRect.width() * 0.1 , printerRect.height() * 0.1);

	double xPos(printerRect.width() * 0.1);
	double yPos(printerRect.height() * 0.20);
	double maxYPos(0);
	int colBreak(0);
	if(familyFonts.count() <= 60)
	{
		if((familyFonts.count() % 4) == 0)
			colBreak = familyFonts.count() / 4;
		else
			colBreak = qMax( 1, familyFonts.count() / 3);
		double colunit(100);
		for(int fidx(0); fidx < familyFonts.count(); ++fidx)
		{
			if((fidx >= colBreak) && ((fidx % colBreak) == 0))
			{
				xPos += colunit + 20 ;
				yPos = printerRect.height() * 0.20;
			}

			nameFont.setPointSizeF(4.0);
			QGraphicsSimpleTextItem * varText( pScene.addSimpleText(familyFonts[fidx]->variant() , nameFont) );
			varText->setBrush(Qt::gray);
			varText->setPos(xPos, yPos);
			yPos += varText->boundingRect().height() * 2.2;
			QPointF origine(xPos,  yPos );

			bool rasterState(familyFonts[fidx]->rasterFreetype());
			familyFonts[fidx]->setFTRaster(false);
			familyFonts[fidx]->renderLine(&pScene, QString("foxy brown fox trot"), origine, printerRect.width() * 0.35 , 12.0);
			familyFonts[fidx]->setFTRaster(rasterState);

			yPos +=  12.0 ;
			maxYPos = qMax(yPos, maxYPos);
		}

		// Characters;
		FontItem * pf(FMVariants::Preferred(familyFonts));
		int cCount(pf->countChars());
		int charcode(pf->firstChar());
		double ccX(printerRect.width() * 0.1);
		double ccY(maxYPos + 30.0);
		const double ccW(printerRect.width() * .9);
		for(int i = 0; i < cCount; ++i)
		{
			if(ccY > 560)
				break;
			QGraphicsPathItem *p(pf->itemFromChar(charcode, 15.0));
			if(p->data(GLYPH_DATA_ERROR).toBool())
			{
				charcode = pf->nextChar(charcode);
				delete p;
				continue;
			}
			pScene.addItem(p);
			p->setPen(Qt::NoPen);
			double advance(p->data(GLYPH_DATA_HADVANCE_SCALED).toDouble());
			double fakeAdvance(advance * 1.5);
			if((ccX + fakeAdvance) > ccW)
			{
				ccY += 22.0;
				if(ccY > 560)
					break;
				ccX = printerRect.width() * 0.1;
			}
			p->setPos(ccX, ccY);

			ccX += fakeAdvance;
			charcode = pf->nextChar(charcode);

		}

		// Unicode Coverage
		QStringList llist;
		foreach(FontItem * fi, familyFonts)
		{
			foreach(const QString& sl, fi->supportedLangDeclaration())
			{
				if(!llist.contains(sl))
					llist << sl;
			}
		}

		if(llist.count() > 0)
		{
			nameFont.setPointSizeF(6.0);
			QGraphicsSimpleTextItem * uniText( pScene.addSimpleText(tr("Unicode coverage") , nameFont) );
			uniText->setPos(printerRect.width() * 0.5, 600);
			nameFont.setPointSizeF(4.0);
			QGraphicsTextItem *  uniList(pScene.addText(llist.join(", ") + QString("."), nameFont));
			uniList->setTextWidth(printerRect.width() * .4);
			uniList->setPos(printerRect.width() * 0.5, 630);
		}

		// OpenType


		pScene.render(painter, printer->paperRect(), printerRect);
		return true;
	}
	else // think Kepler :)
	{
		colBreak = 20;
		double colunit(100);
		bool more(false);
		for(int fidx(0); fidx < familyFonts.count(); ++fidx)
		{
			if((fidx >= colBreak))
			{
				if(fidx >= (4 * colBreak))
				{
					if(fidx == (4 * colBreak))
					{
						pScene.render(painter, printer->paperRect(), printerRect);
						pScene.clear();
						printer->newPage();
						more = true;
						xPos = printerRect.width() * 0.1;
						yPos = printerRect.height() * 0.08;
					}
					else if(((fidx - (4 * colBreak)) % (colBreak + 4)) == 0)
					{
						xPos += colunit + 20 ;
						yPos = printerRect.height() * 0.08;
					}
				}
				else if((fidx % colBreak) == 0)
				{
					xPos += colunit + 20 ;
					yPos = printerRect.height() * 0.20;
				}
			}

			nameFont.setPointSizeF(4.0);
			QGraphicsSimpleTextItem * varText( pScene.addSimpleText(familyFonts[fidx]->variant() , nameFont) );
			varText->setBrush(Qt::gray);
			varText->setPos(xPos, yPos);
			yPos += varText->boundingRect().height() * 2.2;
			QPointF origine(xPos,  yPos );

			bool rasterState(familyFonts[fidx]->rasterFreetype());
			familyFonts[fidx]->setFTRaster(false);
			familyFonts[fidx]->renderLine(&pScene, QString("foxy brown fox trot"), origine, printerRect.width() * 0.35 , 12.0);
			familyFonts[fidx]->setFTRaster(rasterState);

			yPos +=  12.0 ;
		}
		pScene.render(painter, printer->paperRect(), printerRect);
		return (!more);
	}

}


void FontBook::doOneLinerBook()
{

}


// OBSOLETE
void FontBook::doBookFromTemplate ( const QDomDocument &aTemplate )
{
//	/**
//	We build lists of contexts
//	*/
//	QList<FontBookContext> conPage;
//	QList<FontBookContext> conFamily;
//	QList<FontBookContext> conSubfamily;

//	QDomNodeList conList = aTemplate.elementsByTagName ( "context" );
//	if ( conList.length() == 0 )
//	{
//		qDebug ( ) << "ERROR: "<< conList.length() <<" context in template, see yourself :" ;
//		qDebug() << aTemplate.toString ( 1 );
//		return;
//	}

//	QMap<QString, QSvgRenderer*> svgRendered;
//	QMap<QString, QFont> qfontCache; // abit of optim.
//	for ( uint i = 0; i < conList.length(); ++i )
//	{
//		FontBookContext fbc;
//		QDomNode context = conList.item ( i );
//		QString levelString = context.toElement().attributeNode ( "level" ).value();

//		fbc.textElement.e = context.namedItem ( "text" ).toElement().text();
//		if ( !fbc.textElement.e.isEmpty() )
//		{
//			// 		QString textInternalString = context.namedItem ( "text" ).toElement().attributeNode ( "internal" ).value();
//			// 		fbc.textElement.internal = ( textInternalString == "true" ) ? true : false;
//			fbc.textElement.valid = true;

//			QDomNode tStyle =  context.namedItem ( "textstyle" );
//			fbc.textStyle.name = tStyle.toElement().attributeNode ( "name" ).value();
//			fbc.textStyle.font = tStyle.namedItem ( "font" ).toElement().text();
//			fbc.textStyle.fontsize = QString ( tStyle.namedItem ( "fontsize" ).toElement().text() ).toDouble() ;
//			fbc.textStyle.color = QColor ( tStyle.namedItem ( "color" ).toElement().text() );

//			bool ital = false;
//			QFont::Weight bold = QFont::Normal;
//			if ( fbc.textStyle.font.contains ( "italic", Qt::CaseInsensitive ) )
//				ital = true;
//			if ( fbc.textStyle.font.contains ( "bold", Qt::CaseInsensitive ) )
//				bold = QFont::Bold;
//			qfontCache[fbc.textStyle.name] = QFont ( fbc.textStyle.font,10, bold, ital );
//			qfontCache[fbc.textStyle.name].setPointSizeF(fbc.textStyle.fontsize );

//			fbc.textStyle.lineheight = QString ( tStyle.namedItem ( "lineheight" ).toElement().text() ).toDouble() ;
//			fbc.textStyle.margin_top = QString ( tStyle.namedItem ( "margintop" ).toElement().text() ).toDouble() ;
//			fbc.textStyle.margin_left = QString ( tStyle.namedItem ( "marginleft" ).toElement().text() ).toDouble() ;
//			fbc.textStyle.margin_bottom = QString ( tStyle.namedItem ( "marginbottom" ).toElement().text() ).toDouble() ;
//			fbc.textStyle.margin_right = QString ( tStyle.namedItem ( "marginright" ).toElement().text() ).toDouble() ;
//		}

//		QDomNode graphicNode = context.namedItem ( "graphic" );
//		if ( graphicNode.isElement() )
//		{
//			// 			QDomDocumentFragment svgFrag = aTemplate.createDocumentFragment();
//			QDomNode svgNode = graphicNode.toElement().namedItem ( "svg" ).cloneNode ( true );
//			// 			svgFrag.appendChild(svgNode);
//			// 			if(svgNode.isElement())
//			{
//				fbc.graphic.name =  graphicNode.toElement().attributeNode ( "name" ).value();
//				fbc.graphic.x = QString ( graphicNode.toElement().attributeNode ( "xpos" ).value() ).toDouble();
//				fbc.graphic.y = QString ( graphicNode.toElement().attributeNode ( "ypos" ).value() ).toDouble();

//				QDomDocument svgDoc;
//				QDomNode svg = svgDoc.importNode ( svgNode,true );
//				svgDoc.appendChild ( svg );
//				QString svgString ( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" + svgDoc.toString ( 0 ) );
//				QSvgRenderer *doc = new QSvgRenderer ( svgString.toUtf8() );
//				svgRendered[fbc.graphic.name] = doc;
//				fbc.graphic.valid = true;
//				// 				qDebug() << fbc.graphic.svg;
//			}

//		}


//		if ( levelString == "page" )
//			conPage << fbc;
//		else if ( levelString == "family" )
//			conFamily << fbc;
//		else if ( levelString == "subfamily" )
//			conSubfamily << fbc;
//	}


//	QString paperSize = QString ( aTemplate.documentElement().namedItem ( "papersize" ).toElement().text() ).toUpper();
//	double prectx =  QString ( aTemplate.documentElement().namedItem ( "papersize" ).toElement().attributeNode ( "bboxx" ).value() ).toDouble();
//	double precty =  QString ( aTemplate.documentElement().namedItem ( "papersize" ).toElement().attributeNode ( "bboxy" ).value() ).toDouble();
//	double prectw =  QString ( aTemplate.documentElement().namedItem ( "papersize" ).toElement().attributeNode ( "bboxw" ).value() ).toDouble();
//	double precth =  QString ( aTemplate.documentElement().namedItem ( "papersize" ).toElement().attributeNode ( "bboxh" ).value() ).toDouble();

//	QPrinter thePrinter ( QPrinter::HighResolution );
//	thePrinter.setOutputFormat ( QPrinter::PdfFormat );
//	thePrinter.setCreator ( "Fontmatrix " + QString::number ( FONTMATRIX_VERSION_MAJOR ) + "." + QString::number ( FONTMATRIX_VERSION_MINOR ) );
//	thePrinter.setDocName ( "A font book" );
//	thePrinter.setOutputFileName ( outputFilePath );
//	thePrinter.setPageSize ( mapPSize[paperSize] );
//	thePrinter.setFullPage ( true );
//	// 	qDebug() << thePrinter.pageSize() << thePrinter.pageRect() << thePrinter.paperRect() << thePrinter.resolution() ;
//	double paperWidth  =  thePrinter.pageRect().width() / thePrinter.resolution() * 72.0;
//	double paperHeight =  thePrinter.pageRect().height() / thePrinter.resolution() * 72.0;
//	// 	qDebug()<< paperSize << paperWidth << paperHeight;
//	QGraphicsScene theScene;
//	QGraphicsScene measurementScene;
//	measurementScene.setSceneRect( 0,0,paperWidth,paperHeight );
//	theScene.setSceneRect ( 0,0,paperWidth,paperHeight );
//	QPainter thePainter ( &thePrinter );
//	QPointF thePos ( prectx,precty );
//	QList<FontItem*> renderedFont;
//	QList<QGraphicsSvgItem*> renderedGraphic;
//	QList<QGraphicsTextItem*> renderedText;



//	QList<FontItem*> localFontMap = FMFontDb::DB()->getFilteredFonts();
//	QMap<QString, QList<FontItem*> > keyList;
//	for ( int i=0; i < localFontMap.count();++i )
//	{
//		keyList[localFontMap[i]->family() ].append ( localFontMap[i] );
//	}

//	QMap<QString, QList<FontItem*> >::const_iterator kit;
//	QProgressDialog progress ( QObject::tr ( "Creating font book... " ), QObject::tr ( "cancel" ), 0, keyList.count(), typotek::getInstance() );
//	progress.setWindowModality ( Qt::WindowModal );
//	int progressindex=0;


//	QString pageNumStr;
//	int pageNumber = 0;
//	///We begin in a PAGE context
//	// 	qDebug() << "PAGE";
//	pageNumStr.setNum ( ++pageNumber );
//	QMap<QString,QString> pageReplace;
//	QMap<QString,QString> familyReplace;
//	QMap<QString,QString> subfamilyReplace;

//	QString currentFamily;
//	QString currentSubfamily;

//	/** Z policy is
//		PAGE_SVG 1
//		PAGE_TEXT 10
//		FAMILY_SVG 100
//		FAMILY_TEXT 1000
//		SUBFAMILY_SVG 10000
//		SUBFAMILY_TEXT 100000
//	*/

//	for ( int pIndex = 0; pIndex < conPage.count(); ++pIndex )
//	{
//		// 		qDebug()<<"PI"<<pIndex;
//		if ( conPage[pIndex].textElement.valid )
//		{
//			QStringList pagelines ;
//			QStringList tmplines = conPage[pIndex].textElement.e.split ( "\n" );
//			pageReplace["##PAGENUMBER##"] = pageNumStr;
//			pageReplace["##FAMILY##"] = currentFamily;
//			pageReplace["##SUBFAMILY##"] = currentSubfamily;
//			for ( int t = 0; t < tmplines.count(); ++t )
//			{

//				QString place = tmplines[t];
//				for ( QMap<QString,QString>::const_iterator repIt = pageReplace.begin(); repIt != pageReplace.end();   ++repIt )
//					place.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
//				pagelines << place;
//			}
//			// 			QFont pFont ( conPage[pIndex].textStyle.font, conPage[pIndex].textStyle.fontsize );
//			for ( int pl = 0; pl < pagelines.count(); ++pl )
//			{
//				QGraphicsTextItem * ti = theScene.addText ( pagelines[pl], qfontCache[conPage[pIndex].textStyle.name] );
//				renderedText << ti;
//				ti->setPos ( conPage[pIndex].textStyle.margin_left , conPage[pIndex].textStyle.margin_top + ( pl * conPage[pIndex].textStyle.lineheight ) );
//				ti->setZValue ( 10 );
//				ti->setDefaultTextColor ( conPage[pIndex].textStyle.color );
//			}
//		}
//		if ( conPage[pIndex].graphic.valid )
//		{
//			QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
//			svgIt->setSharedRenderer ( svgRendered[conPage[pIndex].graphic.name] );
//			theScene.addItem ( svgIt );
//			svgIt->setPos ( conPage[pIndex].graphic.x, conPage[pIndex].graphic.y );
//			renderedGraphic << svgIt;
//			svgIt->setZValue ( 1 );
//		}
//	}

//	/// Beginning of the big loop
//	for ( kit = keyList.begin(); kit != keyList.end(); ++kit )
//	{
//		/// We are in a FAMILY context
//		// 		qDebug() << "FAMILY";
//		{
//			if ( progress.wasCanceled() )
//				break;
//			progress.setLabelText ( kit.key() );
//			progress.setValue ( ++progressindex );
//		}
//		currentFamily = kit.key();
//		for ( int elemIndex = 0; elemIndex < conFamily.count() ; ++elemIndex )
//		{
//			QStringList familylines;
//			QStringList tmplines = conFamily[elemIndex].textElement.e.split ( "\n" );
//			familyReplace["##FAMILY##"] = kit.key();
//			for ( int t = 0; t < tmplines.count(); ++t )
//			{

//				QString place = tmplines[t];
//				for ( QMap<QString,QString>::const_iterator repIt = familyReplace.begin(); repIt != familyReplace.end();   ++repIt )
//					place.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
//				if ( !place.isEmpty() )
//					familylines << place;
//			}

//			double available = ( precty + precth ) - thePos.y();
//			double needed = ( familylines.count() * conFamily[elemIndex].textStyle.lineheight )
//			                + conFamily[elemIndex].textStyle.margin_top
//			                + conFamily[elemIndex].textStyle.margin_bottom;

//			if ( needed > available )
//			{
//				/// We are in a PAGE context
//				// 				qDebug() << "NFPAGE";
//				// close, clean and create
//				theScene.render ( &thePainter );

//				thePos.ry() = precty;
//				// 				for ( int  d = 0; d <  renderedFont.count() ; ++d )
//				// 					renderedFont[d]->deRenderAll();
//				for ( int  d = 0; d < renderedGraphic.count(); ++d )
//					delete renderedGraphic[d];
//				for ( int  d = 0; d < renderedText.count(); ++d )
//					delete renderedText[d];
//				renderedFont.clear();
//				renderedGraphic.clear();
//				renderedText.clear();
//				// 				theScene.removeItem ( theScene.createItemGroup ( theScene.items() ) );

//				thePrinter.newPage();
//				pageNumStr.setNum ( ++pageNumber );

//				//
//				for ( int pIndex = 0; pIndex < conPage.count(); ++pIndex )
//				{
//					QStringList pagelines ;
//					QStringList tmplines = conPage[pIndex].textElement.e.split ( "\n" );
//					pageReplace["##PAGENUMBER##"] = pageNumStr;
//					pageReplace["##FAMILY##"] = currentFamily;
//					pageReplace["##SUBFAMILY##"] = currentSubfamily;
//					for ( int t = 0; t < tmplines.count(); ++t )
//					{

//						QString pageplace = tmplines[t];
//						for ( QMap<QString,QString>::const_iterator repIt = pageReplace.begin(); repIt != pageReplace.end();   ++repIt )
//							pageplace.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
//						pagelines << pageplace;
//					}
//					// 						QFont pFont ( conPage[pIndex].textStyle.font, conPage[pIndex].textStyle.fontsize );
//					for ( int pl = 0; pl < pagelines.count(); ++pl )
//					{
//						QGraphicsTextItem * ti = theScene.addText ( pagelines[pl], qfontCache[conPage[pIndex].textStyle.name] );
//						renderedText << ti;
//						ti->setPos ( conPage[pIndex].textStyle.margin_left + prectx, conPage[pIndex].textStyle.margin_top + ( pl * conPage[pIndex].textStyle.lineheight ) );
//						ti->setZValue ( 10 );
//						ti->setDefaultTextColor ( conPage[pIndex].textStyle.color );
//					}
//					if ( conPage[pIndex].graphic.valid )
//					{
//						QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
//						svgIt->setSharedRenderer ( svgRendered[conPage[pIndex].graphic.name] );
//						theScene.addItem ( svgIt );
//						svgIt->setPos ( conPage[pIndex].graphic.x + prectx, conPage[pIndex].graphic.y );
//						renderedGraphic << svgIt;
//						svgIt->setZValue ( 1 );
//					}
//				}
//			}

			
//			for ( int fl = 0; fl < familylines.count(); ++fl )
//			{
//				QGraphicsTextItem * ti = theScene.addText ( familylines[fl], qfontCache[conFamily[elemIndex].textStyle.name] );
//				renderedText << ti;
//				ti->setPos ( conFamily[elemIndex].textStyle.margin_left + prectx, thePos.y() + ( conFamily[elemIndex].textStyle.margin_top + ( fl * conFamily[elemIndex].textStyle.lineheight ) ) );
//				ti->setZValue ( 1000 );
//				ti->setDefaultTextColor ( conFamily[elemIndex].textStyle.color );
//			}
//			if ( conFamily[elemIndex].graphic.valid )
//			{
//				QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
//				svgIt->setSharedRenderer ( svgRendered[conFamily[elemIndex].graphic.name] );
//				theScene.addItem ( svgIt );
//				svgIt->setPos ( conFamily[elemIndex].graphic.x + prectx, conFamily[elemIndex].graphic.y + thePos.y() );
//				renderedGraphic << svgIt;
//				svgIt->setZValue ( 100 );
//			}

//			thePos.ry() += needed;
//		} // end of FAMILY level elements
		
//		/// Looping through all faces for the current family
//		for ( int fontIndex = 0;fontIndex < kit.value().count(); ++fontIndex )
//		{
//			FontItem * theFont = kit.value() [fontIndex];
//			FMLayout *alay = new  FMLayout(&theScene , theFont);
//			/// We are in a SUBFAMILY context
//			currentSubfamily = theFont->variant();
//			for ( int elemIndex = 0; elemIndex < conSubfamily.count() ; ++elemIndex )
//			{
//				// First, is there enough room for this element
//				QStringList sublines;
//				QStringList tmplines = conSubfamily[elemIndex].textElement.e.split ( "\n" );

//				subfamilyReplace["##FAMILY##"] = theFont->family();
//				subfamilyReplace["##SUBFAMILY##"] = theFont->variant();
//				subfamilyReplace["##FILE##"]= theFont->path();
//				subfamilyReplace["##TAGS##"]= theFont->tags().join ( ";" ) ;
//				subfamilyReplace["##COUNT##"]= QString::number ( theFont->glyphsCount() );
//				subfamilyReplace["##TYPE##"]= theFont->type();
//				subfamilyReplace["##CHARSETS##"]=theFont->charmaps().join ( ";" );

//				for ( int t = 0; t < tmplines.count(); ++t )
//				{

//					QString subplace = tmplines[t];
//					for ( QMap<QString,QString>::const_iterator repIt = subfamilyReplace.begin(); repIt != subfamilyReplace.end();   ++repIt )
//						subplace.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
//					if ( !subplace.isEmpty() )
//						sublines << subplace;
//				}

//				double available = ( precty + precth ) - thePos.y();
//				double needed (0);/*= ( sublines.count() * conSubfamily[elemIndex].textStyle.lineheight )
//				                + conSubfamily[elemIndex].textStyle.margin_top
//				                + conSubfamily[elemIndex].textStyle.margin_bottom;*/
//				double mwidth( conSubfamily[elemIndex].textStyle.margin_right  - (conSubfamily[elemIndex].textStyle.margin_left + prectx) );

//				/// Let’s see how much room we need
//				// For that we’ll render all elements on a dedicated scene if needed.
//				if(conSubfamily[elemIndex].graphic.valid)
//				{
//					needed = svgRendered[conSubfamily[elemIndex].graphic.name]->defaultSize().height();
//				}
//				else
//				{
//					if ( conSubfamily[elemIndex].textStyle.font == "_FONTMATRIX_" ) // We’ll use the current font
//					{
//						QList<GlyphList> gl;
//						for ( int sl = 0; sl < sublines.count(); ++sl )
//						{
//							gl << theFont->glyphs ( sublines[sl].trimmed(), conSubfamily[elemIndex].textStyle.fontsize );
//						}
//						QRectF rf( measurementScene.sceneRect() );
//						rf.setWidth(mwidth);
//						FMLayout *tlay = new  FMLayout( &measurementScene ,theFont, rf );
//						tlay->setPersistentScene(false);
//						tlay->setAdjustedSampleInter ( conSubfamily[elemIndex].textStyle.lineheight );
//						tlay->setDeviceIndy ( true );
						
//						tlay->doLayout ( gl , conSubfamily[elemIndex].textStyle.fontsize );
//						//						tlay->run();
						
//						needed = tlay->drawnLines * conSubfamily[elemIndex].textStyle.lineheight;
						
//						delete tlay;
						
//					}
//					else
//					{
//						for ( int sl = 0; sl < sublines.count(); ++sl )
//						{
							
//							QGraphicsTextItem gti( sublines[sl]);
//							gti.setFont( qfontCache[ conSubfamily[elemIndex].textStyle.name] );
//							gti.setTextWidth( mwidth );
//							needed = gti.document()->size().height();
//						}
//					}
//				}
//				if ( needed > available )
//				{
//					/// We are in a PAGE context
//					// 					qDebug() << "NSPAGE";
//					// close, clean and create
//					theScene.render ( &thePainter );

//					thePos.ry() = precty;
//					// 					for ( int  d = 0; d <  renderedFont.count() ; ++d )
//					// 						renderedFont[d]->deRenderAll();
//					for ( int  d = 0; d < renderedGraphic.count(); ++d )
//						delete renderedGraphic[d];
//					for ( int  d = 0; d < renderedText.count(); ++d )
//						delete renderedText[d];
//					renderedFont.clear();
//					renderedGraphic.clear();
//					renderedText.clear();
//					// 					theScene.removeItem ( theScene.createItemGroup ( theScene.items() ) );

//					thePrinter.newPage();
//					pageNumStr.setNum ( ++pageNumber );

//					//
//					for ( int pIndex = 0; pIndex < conPage.count(); ++pIndex )
//					{
//						QStringList pagelines ;
//						QStringList tmplines = conPage[pIndex].textElement.e.split ( "\n" );
//						pageReplace["##PAGENUMBER##"] = pageNumStr;
//						pageReplace["##FAMILY##"] = currentFamily;
//						pageReplace["##SUBFAMILY##"] = currentSubfamily;
//						for ( int t = 0; t < tmplines.count(); ++t )
//						{

//							QString pageplace = tmplines[t];
//							for ( QMap<QString,QString>::const_iterator repIt = pageReplace.begin(); repIt != pageReplace.end();   ++repIt )
//								pageplace.replace ( repIt.key(),repIt.value(),Qt::CaseSensitive );
//							pagelines << pageplace;
//						}
//						// 							QFont pFont ( conPage[pIndex].textStyle.font, conPage[pIndex].textStyle.fontsize );
//						for ( int pl = 0; pl < pagelines.count(); ++pl )
//						{
//							QGraphicsTextItem * ti = theScene.addText ( pagelines[pl], qfontCache[conPage[pIndex].textStyle.name] );
//							renderedText << ti;
//							ti->setPos ( conPage[pIndex].textStyle.margin_left + prectx, conPage[pIndex].textStyle.margin_top + ( pl * conPage[pIndex].textStyle.lineheight ) );
//							ti->setZValue ( 10 );
//							ti->setDefaultTextColor ( conPage[pIndex].textStyle.color );
//						}
//						if ( conPage[pIndex].graphic.valid )
//						{
//							QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
//							svgIt->setSharedRenderer ( svgRendered[conPage[pIndex].graphic.name] );
//							theScene.addItem ( svgIt );
//							svgIt->setPos ( conPage[pIndex].graphic.x + prectx, conPage[pIndex].graphic.y );
//							renderedGraphic << svgIt;
//							svgIt->setZValue ( 1 );
//						}
//					}
//				}

				
//				if ( conSubfamily[elemIndex].graphic.valid )
//				{
//					QGraphicsSvgItem *svgIt = new QGraphicsSvgItem();
//					renderedGraphic << svgIt;
//					svgIt->setSharedRenderer ( svgRendered[conSubfamily[elemIndex].graphic.name] );
//					theScene.addItem ( svgIt );
//					svgIt->setPos ( conSubfamily[elemIndex].graphic.x + prectx, conSubfamily[elemIndex].graphic.y + thePos.y() );
//					svgIt->setZValue ( 100000 );
//					thePos.ry() += svgRendered[conSubfamily[elemIndex].graphic.name]->defaultSize().height();
//				}
//				else
//				{
//					if ( conSubfamily[elemIndex].textStyle.font == "_FONTMATRIX_" ) // We’ll use the current font
//					{

//						QList<GlyphList> gl;
//						for ( int sl = 0; sl < sublines.count(); ++sl )
//						{
//							gl << theFont->glyphs ( sublines[sl].trimmed(), conSubfamily[elemIndex].textStyle.fontsize );
//						}
						
//						QRectF parRect ( conSubfamily[elemIndex].textStyle.margin_left + prectx,
//								 thePos.y() + conSubfamily[elemIndex].textStyle.margin_top,
//								 conSubfamily[elemIndex].textStyle.margin_right,
//								 precth - thePos.y() );
//						alay->setRect(parRect);
//						if(renderedFont.count() > 0)
//						{
//							alay->setPersistentScene(true);
//							// 							FMLayout::getLayout()->resetScene();
//						}
//						else
//							alay->setPersistentScene(false);

//						//						qDebug()<<"PAR("+theFont->fancyName()+")("<< gl.count() <<")"<<parRect ;
//						//						FMLayout::getLayout()->setTheScene ( );
//						//						FMLayout::getLayout()->setTheFont ( theFont );
//						alay->setAdjustedSampleInter ( conSubfamily[elemIndex].textStyle.lineheight );
//						alay->setDeviceIndy ( true );
						
//						alay->doLayout ( gl , conSubfamily[elemIndex].textStyle.fontsize );
//						//						alay->run();
						
//						thePos.ry() += alay->drawnLines * conSubfamily[elemIndex].textStyle.lineheight;
//						renderedFont.append ( theFont );
//					}
//					else
//					{
//						// 						QFont aFont ( conSubfamily[elemIndex].textStyle.font,conSubfamily[elemIndex].textStyle.fontsize );
//						for ( int sl = 0; sl < sublines.count(); ++sl )
//						{
//							QGraphicsTextItem * ti = theScene.addText ( sublines[sl], qfontCache[ conSubfamily[elemIndex].textStyle.name] );
//							renderedText << ti;
//							ti->setTextWidth( mwidth );
//							ti->setPos ( conSubfamily[elemIndex].textStyle.margin_left + prectx, thePos.y() + ( conSubfamily[elemIndex].textStyle.margin_top + ( sl * conSubfamily[elemIndex].textStyle.lineheight ) ) );
//							ti->setZValue ( 10000 );
//							ti->setDefaultTextColor ( conSubfamily[elemIndex].textStyle.color );
							
//							thePos.ry() += ti->document()->size().height();
//						}
//					}
//				}
//			} // end of SUBFAMILY level elements
//			// 			qDebug() << "ENDOF_SUBFAMILY";
//			delete alay;
//		}
//	}
//	if ( renderedFont.count() )
//	{
//		theScene.render ( &thePainter );
//		for ( int  d = 0; d <  renderedFont.count() ; ++d )
//		{
//			renderedFont[d]->deRenderAll();

//		}
//		for ( int  d = 0; d < renderedGraphic.count(); ++d )
//			delete renderedGraphic[d];
//		for ( int  d = 0; d < renderedText.count(); ++d )
//			delete renderedText[d];
//		renderedFont.clear();
//		renderedGraphic.clear();
//		renderedText.clear();

//	}
//	for ( QMap<QString,QSvgRenderer*>::iterator sit ( svgRendered.begin() ); sit != svgRendered.end(); ++sit )
//		delete sit.value();

}


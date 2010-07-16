/***************************************************************************
 *   Copyright (C) 2010 by Pierre Marchand   *
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

#include "chartwidget.h"
#include "ui_chartwidget.h"

#include "fmglyphhighlight.h"
#include "fontitem.h"
#include "fmfontdb.h"
#include "fmuniblocks.h"

#include <QStringListModel>
#include <QCompleter>
#include <QGraphicsRectItem>
#include <QApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QDebug>
#include <QPrinter>
#include <QPrintDialog>

const QString ChartWidget::Name = QObject::tr("Chart");

ChartWidget::ChartWidget(const QString& fid, QWidget *parent) :
		FloatingWidget(fid, Name, parent),
		ui(new Ui::ChartWidget),
		fontIdentifier(fid)
{
	FontItem * theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
	ui->setupUi(this);
	ui->uniLine->setEnabled(false);
	abcScene = new QGraphicsScene;
	ui->abcView->setScene ( abcScene );
	ui->abcView->setRenderHint ( QPainter::Antialiasing, true );
	QStringListModel* cslModel(new QStringListModel);
	cslModel->setStringList(theVeryFont->getNames());
	QCompleter* cslCompleter(new QCompleter(ui->charSearchLine));
	cslCompleter->setModel(cslModel);
	ui->charSearchLine->setCompleter(cslCompleter);
	unMapGlyphName = tr("Un-Mapped Glyphs");
	allMappedGlyphName = tr("View all mapped glyphs");
	uRangeIsNotEmpty = false;
	fillUniPlanesCombo(theVeryFont);
	curGlyph = 0;
	fancyGlyphInUse = -1;

	createConnections();

}

ChartWidget::~ChartWidget()
{
	// As we're about to delete the scene, we must tell FontItem to clear its cache
	FMFontDb::DB()->Font(fontIdentifier)->deRenderAll();
	removeConnections();
	delete ui;
	delete abcScene;
	delete curGlyph;
}

void ChartWidget::createConnections()
{
	connect ( ui->abcView,SIGNAL ( pleaseShowSelected() ),this,SLOT ( slotShowOneGlyph() ) );
	connect ( ui->abcView,SIGNAL ( pleaseShowAll() ),this,SLOT ( slotShowAllGlyph() ) );
	connect ( ui->abcView,SIGNAL ( refit ( int ) ),this,SLOT ( slotAdjustGlyphView ( int ) ) );
	connect ( ui->abcView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateGView()));
	connect ( ui->abcView, SIGNAL(pleaseUpdateSingle()), this, SLOT(slotUpdateGViewSingle()));
	connect ( ui->uniPlaneCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotPlaneSelected ( int ) ) );
	connect ( ui->clipboardCheck, SIGNAL (toggled ( bool )),this,SLOT(slotShowULine(bool)));
	connect ( ui->charSearchLine, SIGNAL(returnPressed()), this, SLOT(slotSearchCharName()));

	connect(ui->toolbar, SIGNAL(Close()), this, SLOT(close()));
	connect(ui->toolbar, SIGNAL(Hide()), this, SLOT(hide()));
	connect(ui->toolbar, SIGNAL(Print()), this, SLOT(slotPrint()));
	connect(ui->toolbar, SIGNAL(Detach()), this, SLOT(ddetach()));
}

void ChartWidget::removeConnections()
{
	disconnect ( ui->abcView,SIGNAL ( pleaseShowSelected() ),this,SLOT ( slotShowOneGlyph() ) );
	disconnect ( ui->abcView,SIGNAL ( pleaseShowAll() ),this,SLOT ( slotShowAllGlyph() ) );
	disconnect ( ui->abcView,SIGNAL ( refit ( int ) ),this,SLOT ( slotAdjustGlyphView ( int ) ) );
	disconnect ( ui->abcView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateGView()));
	disconnect ( ui->abcView, SIGNAL(pleaseUpdateSingle()), this, SLOT(slotUpdateGViewSingle()));
	disconnect ( ui->uniPlaneCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotPlaneSelected ( int ) ) );
	disconnect ( ui->clipboardCheck, SIGNAL (toggled ( bool )),this,SLOT(slotShowULine(bool)));
	disconnect ( ui->charSearchLine, SIGNAL(returnPressed()), this, SLOT(slotSearchCharName()));

	disconnect(ui->toolbar, SIGNAL(Close()), this, SLOT(close()));
	disconnect(ui->toolbar, SIGNAL(Hide()), this, SLOT(hide()));
	disconnect(ui->toolbar, SIGNAL(Print()), this, SLOT(slotPrint()));
}

void ChartWidget::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}


void ChartWidget::slotShowOneGlyph()
{
	//	qDebug() <<"slotShowOneGlyph()"<<abcScene->selectedItems().count();
	if ( abcScene->selectedItems().isEmpty() )
		return;
	if ( ui->abcView->lock() )
	{
		QGraphicsRectItem* curGlyph = reinterpret_cast<QGraphicsRectItem*> ( abcScene->selectedItems().first() );
		curGlyph->setSelected ( false );
		if ( fancyGlyphInUse < 0 )
		{
			if ( curGlyph->data ( 3 ).toInt() > 0 ) // Is a codepoint
			{
				fancyGlyphData = curGlyph->data ( 3 ).toInt();
				if(ui->clipboardCheck->isChecked())
				{
					new FMGlyphHighlight(abcScene, curGlyph->rect());
					QString simpleC;
					simpleC += QChar(fancyGlyphData);
					QApplication::clipboard()->setText(simpleC, QClipboard::Clipboard);
					ui->uniLine->setText(ui->uniLine->text() + simpleC);
				}
				else
					fancyGlyphInUse = FMFontDb::DB()->Font(fontIdentifier)->showFancyGlyph ( ui->abcView, fancyGlyphData );
			}
			else // Is a glyph index
			{
				fancyGlyphData = curGlyph->data ( 2 ).toInt();
				fancyGlyphInUse = FMFontDb::DB()->Font(fontIdentifier)->showFancyGlyph ( ui->abcView, fancyGlyphData , true );
			}
			if ( fancyGlyphInUse < 0 )
			{
				ui->abcView->unlock();
				return;
			}
			ui->abcView->setState ( FMGlyphsView::SingleView );
		}
		ui->abcView->unlock();
	}
	else
		qDebug("cannot lock ABCview");
}


void ChartWidget::slotShowAllGlyph()
{
	// 	qDebug() <<"slotShowAllGlyph()";
	if ( fancyGlyphInUse < 0 )
		return;
	if ( ui->abcView->lock() )
	{
		// 		qDebug()<<"View Locked";
		FMFontDb::DB()->Font(fontIdentifier)->hideFancyGlyph ( fancyGlyphInUse );
		fancyGlyphInUse = -1;
		ui->abcView->setState ( FMGlyphsView::AllView );

		ui->abcView->unlock();
	}
	// 	qDebug() <<"ENDOF slotShowAllGlyph()";
}

void ChartWidget::slotUpdateGView()
{
	FontItem * theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
	if(theVeryFont && ui->abcView->lock())
	{
		QPair<int,int> uniPair;
		QString curBlockText(ui->uniPlaneCombo->currentText());
		if(curBlockText == unMapGlyphName)
			uniPair = qMakePair<int,int>(-1,100);
		else if(curBlockText == allMappedGlyphName)
			uniPair = qMakePair<int,int>(0, 0x10FFFF);
		else
			uniPair = FMUniBlocks::interval( curBlockText );

		int coverage = theVeryFont->countCoverage ( uniPair.first, uniPair.second );
		int interval = uniPair.second - uniPair.first;
		coverage = coverage * 100 / ( interval + 1 );// against /0 exception

		QString statstring(tr("Block (%1):").arg( QString::number ( coverage ) + "\%"));
		ui->unicodeCoverageStat->setText ( statstring );

		theVeryFont->renderAll ( abcScene , uniPair.first, uniPair.second );
		ui->abcView->unlock();
	}
}


void ChartWidget::slotAdjustGlyphView ( int width )
{
//	if ( !theVeryFont )
//		return;

	// 	theVeryFont->adjustGlyphsPerRow ( width );
	//	slotView ( true );
}




void ChartWidget::slotUpdateGViewSingle()
{
	FontItem * theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
	if ( theVeryFont && ui->abcView->lock())
	{
		// 			qDebug() <<"1.FGI"<<fancyGlyphInUse;
		theVeryFont->hideFancyGlyph ( fancyGlyphInUse );
		if ( fancyGlyphData > 0 ) // Is a codepoint
		{
			fancyGlyphInUse = theVeryFont->showFancyGlyph ( ui->abcView, fancyGlyphData );
			// 				qDebug() <<"2.FGI"<<fancyGlyphInUse;
		}
		else // Is a glyph index
		{
			fancyGlyphInUse = theVeryFont->showFancyGlyph ( ui->abcView, fancyGlyphData , true );
			// 				qDebug() <<"3.FGI"<<fancyGlyphInUse;
		}
		ui->abcView->unlock();

	}

}


void ChartWidget::slotPlaneSelected ( int i )
{
	//	qDebug()<<"slotPlaneSelected"<<i<<uniPlaneCombo->currentIndex();
	FontItem * theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
	if(i != ui->uniPlaneCombo->currentIndex())
		ui->uniPlaneCombo->setCurrentIndex(i);

	bool stickState = uRangeIsNotEmpty;
	uRangeIsNotEmpty = true;
	slotShowAllGlyph();
	slotUpdateGView();
	if( (stickState == false) && theVeryFont)
	{
		fillUniPlanesCombo(theVeryFont);
	}
	ui->abcView->verticalScrollBar()->setValue ( 0 );
}

void ChartWidget::slotShowULine(bool checked)
{
	if(checked)
	{
		ui->uniLine->setText("");
		ui->uniLine->setEnabled(true);
	}
	else
	{
		ui->uniLine->setEnabled(false);
	}
}

void ChartWidget::slotSearchCharName()
{
	FontItem * theVeryFont(FMFontDb::DB()->Font(fontIdentifier));
	if(!theVeryFont)
		return;
	QString name(ui->charSearchLine->text());
	unsigned short cc(0);
	bool searchCodepoint(false);
	if(name.startsWith("U+")
		|| name.startsWith("u+")
		|| name.startsWith("+"))
		{
		QString vString(name.mid(name.indexOf("+")));
		bool ok(false);
		cc = vString.toInt(&ok, 16);
		if(!ok)
			cc = 0;
		searchCodepoint = true;
	}
	else
		cc = theVeryFont->getNamedChar(name);
	// 	qDebug()<<"CS"<<name<<cc;
	if(!cc)
	{
		// TODO display a usefull message
		// 		charSearchLine->clear();
		return;
	}

	foreach(const QString& key, FMUniBlocks::blocks() )
	{
		QPair<int,int> p(FMUniBlocks::interval(key));
		if((cc >= p.first)
			&& (cc <= p.second))
			{
			int idx(ui->uniPlaneCombo->findText(key));
			slotPlaneSelected(idx);
			int sv(0);
			bool first(true);
			do{
				if(first)
					first = false;
				else
				{
					ui->abcView->verticalScrollBar()->setValue(sv + ui->abcView->height());
					sv = ui->abcView->verticalScrollBar()->value();
				}
				foreach(QGraphicsItem* sit, abcScene->items())
				{
					if((sit->data(1).toString() == "select")
						&& (sit->data(3).toInt() == cc))
						{
						QGraphicsRectItem* ms(reinterpret_cast<QGraphicsRectItem*> (sit));
						if(ms)
						{
							QRectF rf(ms->rect());
							new FMGlyphHighlight(abcScene, rf, 2000, 160);
						}
						else
							qDebug()<<"ERROR: An select item not being a QRect?";
						return;

					}
				}
			}while(sv < ui->abcView->verticalScrollBar()->maximum());
			return;
		}
	}

	// if user was looking for a name and we did not find it in
	// Unicode blocks, it must be unmapped.
	if(!searchCodepoint)
	{
		int idx(ui->uniPlaneCombo->findText(unMapGlyphName));
		slotPlaneSelected(idx);
		int sv(0);
		bool first(true);
		do{
			if(first)
				first = false;
			else
			{
				ui->abcView->verticalScrollBar()->setValue(sv + ui->abcView->height());
				sv = ui->abcView->verticalScrollBar()->value();
			}
			foreach(QGraphicsItem* sit, abcScene->items())
			{
				if((sit->data(1).toString() == "select")
					&& (sit->data(3).toInt() == cc))
					{
					QGraphicsRectItem* ms(reinterpret_cast<QGraphicsRectItem*> (sit));
					if(ms)
					{
						QRectF rf(ms->rect());
						new FMGlyphHighlight(abcScene, rf, 2000, 160);
					}
					else
						qDebug()<<"ERROR: An select item not being a QRect?";
					return;

				}
			}
		}while(sv < ui->abcView->verticalScrollBar()->maximum());
		return;
	}

}


void ChartWidget::fillUniPlanesCombo ( FontItem* item )
{
	QString stickyRange(ui->uniPlaneCombo->currentText());
	// 	qDebug()<<"STiCKyRaNGe :: "<<stickyRange;
	int stickyIndex(0);

	ui->uniPlaneCombo->clear();

	int begin(0);
	int end(0);
	QString lastBlock(FMUniBlocks::lastBlock(begin, end));
	QString block(FMUniBlocks::firstBlock( begin, end ));
	bool first(true);
	do
	{
		if(first)
			first = false;
		else
			block = FMUniBlocks::nextBlock(begin, end);

		int codecount ( item->countCoverage ( begin , end ) );
		if ( codecount > 0 )
		{
			// 			qDebug() << p << codecount;
			ui->uniPlaneCombo->addItem ( block );
			if(block == stickyRange)
			{
				stickyIndex = ui->uniPlaneCombo->count() - 1;
				uRangeIsNotEmpty = true;
			}
		}
		else
		{
			if(block == stickyRange)
			{
				stickyIndex = ui->uniPlaneCombo->count() - 1;
				uRangeIsNotEmpty = true;
			}
		}

	} while(lastBlock != block);
	if(item->countCoverage ( -1 , 100 ) > 0)
	{
		ui->uniPlaneCombo->addItem( unMapGlyphName );
		if(unMapGlyphName == stickyRange)
		{
			stickyIndex = ui->uniPlaneCombo->count() - 1;
			uRangeIsNotEmpty = true;
		}
	}
	ui->uniPlaneCombo->addItem( allMappedGlyphName );
	if(allMappedGlyphName == stickyRange)
	{
		stickyIndex = ui->uniPlaneCombo->count() - 1;
		uRangeIsNotEmpty = true;
	}

	ui->uniPlaneCombo->setCurrentIndex ( stickyIndex );

}

void ChartWidget::slotPrint()
{
	FontItem *font(FMFontDb::DB()->Font(fontIdentifier));
	if(font == 0)
		return;
	QPrinter thePrinter ( QPrinter::HighResolution );
	QPrintDialog dialog(&thePrinter, this);
	dialog.setWindowTitle("Fontmatrix - " + tr("Print Chart") +" - " + font->fancyName() );

	if ( dialog.exec() != QDialog::Accepted )
		return;
	thePrinter.setFullPage ( true );
	QPainter aPainter ( &thePrinter );


	double pWidth(thePrinter.paperRect().width());
	double pHeight(thePrinter.paperRect().height());
	double pFactor( thePrinter.resolution() );

	qDebug()<<"Paper :"<<pWidth<<pHeight;
	qDebug()<<"Resolution :"<<pFactor;
	qDebug()<<"P/R*72:"<<pWidth / pFactor * 72.0<< pHeight / pFactor * 72.0;

	QRectF targetR( pWidth * 0.1, pHeight * 0.1, pWidth * 0.8, pHeight * 0.8 );


	QRectF sourceR( 0, 0, pWidth / pFactor * 72.0, pHeight / pFactor * 72.0);
	QGraphicsScene pScene(sourceR);

	int maxCharcode(0x10FFFF);
	int beginCharcode(0);
	int numP(0);
	bool first(true);
	while(beginCharcode < maxCharcode)
	{
		qDebug() << "Chart("<< ++numP <<") ->"<<beginCharcode<<maxCharcode;
		QList<QGraphicsItem*> lgit(pScene.items());
		foreach(QGraphicsItem* git, lgit)
		{
			pScene.removeItem(git);
			delete git;
		}

		int controlN(maxCharcode - beginCharcode);
		int stopAtCode( font->renderChart(&pScene, beginCharcode, maxCharcode, sourceR.width(),sourceR.height() ) );
		qDebug()<< "Control"<<beginCharcode<<stopAtCode;

		if(stopAtCode == beginCharcode)
			break;

		if(first)
		{
			first = false;
		}
		else
		{
			thePrinter.newPage();
		}
		aPainter.drawText(targetR.bottomLeft(), font->fancyName()+"[U"+QString::number(beginCharcode  ,16).toUpper()+", U"+QString::number(stopAtCode ,16).toUpper()+"]");
		pScene.render(&aPainter,targetR, sourceR, Qt::KeepAspectRatio);

		beginCharcode = stopAtCode;
	}

}

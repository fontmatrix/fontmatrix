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

ChartWidget::ChartWidget(const QString& fid, FloatingWidget *parent) :
    FloatingWidget(parent),
    ui(new Ui::ChartWidget),
    theVeryFont(FMFontDb::DB()->Font(fid))
{
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

    setWindowTitleAndType(theVeryFont->fancyName(), tr("Chart"));

    connect ( ui->abcView,SIGNAL ( pleaseShowSelected() ),this,SLOT ( slotShowOneGlyph() ) );
    connect ( ui->abcView,SIGNAL ( pleaseShowAll() ),this,SLOT ( slotShowAllGlyph() ) );
    connect ( ui->abcView,SIGNAL ( refit ( int ) ),this,SLOT ( slotAdjustGlyphView ( int ) ) );
    connect ( ui->abcView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateGView()));
    connect ( ui->abcView, SIGNAL(pleaseUpdateSingle()), this, SLOT(slotUpdateGViewSingle()));
    connect ( ui->uniPlaneCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotPlaneSelected ( int ) ) );
    connect ( ui->clipboardCheck, SIGNAL (toggled ( bool )),this,SLOT(slotShowULine(bool)));
    connect ( ui->charSearchLine, SIGNAL(returnPressed()), this, SLOT(slotSearchCharName()));

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

ChartWidget::~ChartWidget()
{
    delete ui;
    delete abcScene;
    delete curGlyph;
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
					fancyGlyphInUse = theVeryFont->showFancyGlyph ( ui->abcView, fancyGlyphData );
			}
			else // Is a glyph index
			{
				fancyGlyphData = curGlyph->data ( 2 ).toInt();
				fancyGlyphInUse = theVeryFont->showFancyGlyph ( ui->abcView, fancyGlyphData , true );
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
		theVeryFont->hideFancyGlyph ( fancyGlyphInUse );
		fancyGlyphInUse = -1;
		ui->abcView->setState ( FMGlyphsView::AllView );

		ui->abcView->unlock();
	}
// 	qDebug() <<"ENDOF slotShowAllGlyph()";
}

void ChartWidget::slotUpdateGView()
{
// 	qDebug()<<"slotUpdateGView"<<uniPlaneCombo->currentText();
// 	printBacktrace(32);
	// If all is how I think it must be, we don’t need to check anything here :)
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
	if ( !theVeryFont )
		return;

// 	theVeryFont->adjustGlyphsPerRow ( width );
//	slotView ( true );
}




void ChartWidget::slotUpdateGViewSingle()
{
// 	qDebug()<<"slotUpdateGViewSingle";
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



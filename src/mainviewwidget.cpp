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
#include "mainviewwidget.h"
#include "typotek.h"
#include "fontitem.h"
#include "fontactionwidget.h"
#include "typotekadaptator.h"

#include <QString>
#include <QDebug>
#include <QGraphicsItem>
#include <QTransform>
#include <QDialog>
#include <QGridLayout>
#include <QGraphicsRectItem>
#include <QDoubleSpinBox>
#include <QLabel>



MainViewWidget::MainViewWidget ( QWidget *parent )
		: QWidget ( parent )
{
	setupUi ( this );

	typo = reinterpret_cast<typotek*> ( parent );

	currentFonts = typo->getAllFonts();
	currentFaction =0;
	
	

	tagLayout = new QGridLayout ( tagPage );

	abcScene = new QGraphicsScene;
	loremScene = new QGraphicsScene;
	QRectF pageRect(0,0,597.6,842.4); //TODO find means to smartly decide of page size (here, iso A4)
	loremScene->setSceneRect(pageRect);
	QGraphicsRectItem *backp = loremScene->addRect(pageRect,QPen(),Qt::white);

	abcView->setScene ( abcScene );
	loremView->setScene ( loremScene );
	loremView->setRenderHint ( QPainter::Antialiasing, true );
	loremView->setBackgroundBrush(Qt::gray);
	loremView->scroll(0,-10000);
// 	loremView->scale(0.5,0.5);

	
	sampleText= "A font is a set of glyphs (images) representing the characters from a particular \ncharacter set in a particular typeface. In professional typography the term typeface is not \ninterchangeable with the word font, which is defined as\n a given alphabet and its associated characters\nin a single size. For example, 8-point Caslon is one font, and 10-point \nCaslon is another. Historically, fonts came in specific sizes determining \nthe size of characters, and in quantities of sorts or number of each letter \nprovided. The design of characters in a font took into account all \nthese factors. As the range of typeface designs increased and requirements \nof publishers broadened over the centuries, fonts of specific \nweight (blackness or lightness) and stylistic variants-most commonly regular \nor roman as distinct to italic, as well as condensed \n -- have led to font families, collections of \nclosely-related typeface designs that can include hundreds of styles. \nA font family is typically a group of related fonts which \nvary only in weight, orientation, width, etc, but not design. For example, Times is a font \nfamily, whereas Times Roman, Times Italic and Times \nBold are individual fonts making up the Times family. Font families \ntypically include several fonts, though some, such as Helvetica, may \nconsist of dozens of fonts. Helvetica, Century Schoolbook, and Courier \nare examples of three widely distributed typefaces."; // from http://en.wikipedia.org/wiki/Typeface
	sampleFontSize = 20;
	sampleInterSize = 26;
	
	ord << "family" << "variant";
	orderingCombo->addItems ( ord );

	fields << "family" << "variant";
	searchField->addItems ( fields );

	QStringList tl_tmp = typotek::tagsList;
	qDebug() << "TAGLIST\n" << typotek::tagsList.join ( "\n" );
// 	tl_tmp.removeAll ( "Activated_On" );
// 	tl_tmp.removeAll ( "Activated_Off" );

	tagsCombo->addItems ( tl_tmp );

	connect ( orderingCombo,SIGNAL ( activated ( const QString ) ),this,SLOT ( slotOrderingChanged ( QString ) ) );

	connect ( fontTree,SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ),this,SLOT ( slotfontSelected ( QTreeWidgetItem*, int ) ) );

	connect ( editAllButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotEditAll() ) );
	
	connect ( this,SIGNAL ( faceChanged() ),this,SLOT ( slotView() ) );
// 	connect ( this,SIGNAL ( faceChanged() ),this,SLOT ( slotInfoFont() ) );
	

	connect ( abcScene,SIGNAL ( selectionChanged() ),this,SLOT ( slotglyphInfo() ) );
	connect ( searchButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotSearch() ) );


	connect ( renderZoom,SIGNAL ( valueChanged ( int ) ),this,SLOT ( slotZoom ( int ) ) );
	connect ( allZoom,SIGNAL ( valueChanged ( int ) ),this,SLOT ( slotZoom ( int ) ) );

	connect ( tagsCombo,SIGNAL ( activated ( const QString& ) ),this,SLOT ( slotFilterTag ( QString ) ) );

	connect ( activateAllButton,SIGNAL ( released() ),this,SLOT ( slotActivateAll() ) );
	connect ( desactivateAllButton,SIGNAL ( released() ),this,SLOT ( slotDesactivateAll() ) );

	connect ( textButton,SIGNAL ( released() ),this,SLOT ( slotSetSampleText() ) );


	slotOrderingChanged(ord[0]);
}


MainViewWidget::~MainViewWidget()
{
}


void MainViewWidget::fillTree()
{
	QTreeWidgetItem *curItem = 0;
	openKeys.clear();
	for(int i=0; i < fontTree->topLevelItemCount();++i)
	{
		if(fontTree->topLevelItem(i)->isExpanded())
			openKeys << fontTree->topLevelItem(i)->text(0);
	}
	
	fontTree->clear();
	QMap<QString, QList<FontItem*> > keyList;
	for ( int i=0; i < currentFonts.count();++i )
	{
		keyList[currentFonts[i]->value ( currentOrdering ) ].append ( currentFonts[i] );
	}
	
	QMap<QString, QList<FontItem*> >::const_iterator kit;
	for ( kit = keyList.begin(); kit != keyList.end(); ++kit )
	{
		QTreeWidgetItem *ord = new QTreeWidgetItem ( fontTree );
		ord->setText ( 0, kit.key() );
		if(openKeys.contains(kit.key()))
		{
				 ord->setExpanded(true);
		}
		for ( int  n = 0; n < kit.value().count(); ++n )
		{
			QTreeWidgetItem *entry = new QTreeWidgetItem ( ord );
			entry->setText(0, kit.value()[n]->variant());
			entry->setText ( 1, kit.value() [n]->name() );
			bool act = kit.value() [n]->tags().contains ( "Activated_On" );
			entry->setCheckState ( 1, act ?  Qt::Checked : Qt::Unchecked );
			if(entry->text(1) == curItemName )
				curItem = entry;
		}
		fontTree->addTopLevelItem ( ord );
	}
	if(curItem)
		fontTree->scrollToItem(curItem,QAbstractItemView::PositionAtTop);

}


void MainViewWidget::slotOrderingChanged ( QString s )
{
	//Update "fontTree"
	

// 	currentFonts = typo->getAllFonts();
	currentOrdering = s;
	fillTree();

}

void MainViewWidget::slotfontSelected ( QTreeWidgetItem * item, int column )
{
// 	qDebug() << "font select";

	curItemName = item->text(0).isNull() ? item->text(1) : item->text(0);
	if ( item->childCount() > 0)
		return;

	lastIndex = faceIndex;
	faceIndex = item->text ( 1 );

	if ( faceIndex.count() && faceIndex != lastIndex )
	{
		slotFontAction ( item,column );
		emit faceChanged();
	}
	
	if ( item->checkState ( 1 ) == Qt::Checked )
		slotActivate ( true, item, column );
	else
		slotActivate ( false, item, column );

}

void MainViewWidget::slotInfoFont()
{
	FontItem *f = typo->getFont ( faceIndex );
	fontInfoText->clear();
	//QString t(QString("Family : %1\nStyle : %2\nFlags : \n%3").arg(f->family()).arg(f->variant()).arg(f->faceFlags()));
	fontInfoText->setText ( f->infoText() );

}

void MainViewWidget::slotView()
{
	FontItem *l = typo->getFont ( lastIndex );
	FontItem *f = typo->getFont ( faceIndex );
	if ( !f )
		return;
	if ( l )
		l->deRenderAll();
	f->deRenderAll();

	QApplication::setOverrideCursor ( Qt::WaitCursor );
	f->renderAll ( abcScene ); // can be rather long depending of the number of glyphs
	QApplication::restoreOverrideCursor();

	QStringList stl = sampleText.split ( '\n' );
	QPointF pen(100,80);
	for ( int i=0; i< stl.count(); ++i )
	{
		pen.ry() = 100 + sampleInterSize * i;
		f->renderLine ( loremScene,stl[i],pen, sampleFontSize );
	}
	
	slotInfoFont();
}

void MainViewWidget::slotglyphInfo()
{
	if ( abcScene->selectedItems().isEmpty() )
		return;
	glyphInfo->clear();
	QString is = typo->getFont ( faceIndex )->infoGlyph ( abcScene->selectedItems() [0]->data ( 1 ).toInt(), abcScene->selectedItems() [0]->data ( 2 ).toInt() );
	glyphInfo->setText ( is );
}

void MainViewWidget::slotSearch()
{
	fontTree->clear();

	QString fs ( searchString->text() );
	QString ff ( searchField->currentText() );

	currentFonts = typo->getFonts ( fs,ff ) ;
	currentOrdering = orderingCombo->currentText();
	fillTree();
}

void MainViewWidget::slotFilterTag ( QString tag )
{
	fontTree->clear();

	QString fs ( tag );
	QString ff ( "tag" );

	currentFonts = typo->getFonts ( fs,ff ) ;
	currentOrdering = orderingCombo->currentText();
	fillTree();
}


void MainViewWidget::slotFontAction ( QTreeWidgetItem * item, int column )
{
	if ( !currentFaction )
	{
		currentFaction = new FontActionWidget ( typo->adaptator() );
		tagLayout->addWidget ( currentFaction );
		connect ( currentFaction,SIGNAL ( cleanMe() ),this,SLOT ( slotCleanFontAction() ) );
		connect ( currentFaction,SIGNAL ( tagAdded ( QString ) ),this,SLOT ( slotAppendTag ( QString ) ) );
		currentFaction->show();
	}

	FontItem * FoIt = typo->getFont ( item->text ( 1 ) );
	if ( FoIt/* && (!FoIt->isLocked())*/ )
	{
// 		currentFaction->slotFinalize();
		QList<FontItem*> fl;
		fl.append ( FoIt );
		currentFaction->prepare ( fl );


	}
}

void MainViewWidget::slotEditAll()
{
	if ( !currentFaction )
	{
		currentFaction = new FontActionWidget ( typo->adaptator() );
		tagLayout->addWidget ( currentFaction );
		connect ( currentFaction,SIGNAL ( cleanMe() ),this,SLOT ( slotCleanFontAction() ) );
		connect ( currentFaction,SIGNAL ( tagAdded ( QString ) ),this,SLOT ( slotAppendTag ( QString ) ) );
		currentFaction->show();
	}


	QList<FontItem*> fl;
	for ( int i =0; i< currentFonts.count(); ++i )
	{
// 		if(!currentFonts[i]->isLocked())
// 		{
		fl.append ( currentFonts[i] );
// 		}
	}
	if ( fl.isEmpty() )
		return;

	currentFaction->prepare ( fl );
}

void MainViewWidget::slotCleanFontAction()
{
	typo->save();
	qDebug() << " FontActionWidget  saved";
}

void MainViewWidget::slotZoom ( int z )
{
	QGraphicsView * concernedView;
	if ( sender()->objectName().contains ( "render" ) )
		concernedView = loremView;
	else
		concernedView = abcView;

	QTransform trans;
	double delta = ( double ) z / 100.0;
	trans.scale ( delta,delta );
	concernedView->setTransform ( trans, false );

}

void MainViewWidget::slotAppendTag ( QString tag )
{
	qDebug() << "add tag to combo " << tag;
	tagsCombo->addItem ( tag );
}

void MainViewWidget::activation ( FontItem* fit , bool act )
{
	qDebug() << "Activation of " << fit->name() << act;
	if ( act )
	{

		if ( !fit->isLocked() )
		{
			QStringList tl = fit->tags();
			if ( !tl.contains ( "Activated_On" ) )
			{
				tl.removeAll ( "Activated_Off" );
				tl << "Activated_On";
				fit->setTags ( tl );

				QFileInfo fofi ( fit->path() );
				if ( !QFile::link ( fit->path() , QDir::home().absolutePath() + "/.fonts/" + fofi.fileName() ) )
				{
					qDebug() << "unable to link " << fofi.fileName();
				}
				else
				{
					typo->adaptator()->private_signal ( 1, fofi.fileName() );
				}
			}
			else
			{
				qDebug() << "\tYet activated";
			}

		}
		else
		{
			qDebug() << "\tIs Locked";
		}

	}
	else
	{

		if ( !fit->isLocked() )
		{
			QStringList tl = fit->tags();
			if ( !tl.contains ( "Activated_Off" ) )
			{
				tl.removeAll ( "Activated_On" );
				tl << "Activated_Off";
				fit->setTags ( tl );

				QFileInfo fofi ( fit->path() );
				if ( !QFile::remove ( QDir::home().absolutePath() + "/.fonts/" + fofi.fileName() ) )
				{
					qDebug() << "unable to remove " << fofi.fileName();
				}
				else
				{
					typo->adaptator()->private_signal ( 0, fofi.fileName() );
				}
			}

		}
	}
	fillTree();
	typo->save();
}


void MainViewWidget::allActivation ( bool act )
{

	foreach ( FontItem* fit, currentFonts )
	{
		activation ( fit,act );
	}

}

void MainViewWidget::slotDesactivateAll()
{
	allActivation ( false );
}

void MainViewWidget::slotActivateAll()
{
	allActivation ( true );
}

void MainViewWidget::slotSetSampleText()
{
	QDialog dial ( this );
	QGridLayout lay ( &dial );
	QTextEdit ted  ( sampleText.replace("\n","<br/>") );
	QPushButton okButton  ( "Ok" );
	
	QLabel labfs("size");
	QLabel labls("interline");
	
	QDoubleSpinBox boxfs;
	boxfs.setRange(1,999);
	boxfs.setValue(sampleFontSize);
	
	QDoubleSpinBox boxls;
	boxls.setRange(1,999);
	boxls.setValue(sampleInterSize);
	
	lay.addWidget ( &ted, 0,0,1,-1 );
	lay.addWidget(&labfs, 1,0);
	lay.addWidget(&labls, 2,0);
	lay.addWidget(&boxfs,1,1);
	lay.addWidget(&boxls,2,1);
	lay.addWidget ( &okButton,3,1 );
	connect ( &okButton,SIGNAL ( released() ),&dial,SLOT ( close() ) );

	dial.exec();
	sampleText = ted.toPlainText () ;
	sampleInterSize = boxls.value();
	sampleFontSize = boxfs.value();

	slotView();

}

void MainViewWidget::slotActivate ( bool act, QTreeWidgetItem * item, int column )
{
	FontItem * FoIt = typo->getFont ( item->text ( 1 ) );
	if ( FoIt )
	{
		activation ( FoIt, act );
	}
}

void MainViewWidget::slotReloadFontList()
{
	currentFonts.clear();
	currentFonts = typo->getAllFonts();
	fillTree();
}








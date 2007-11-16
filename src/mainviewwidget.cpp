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
	theVeryFont = 0;

	typo = typotek::getInstance();

	currentFonts = typo->getAllFonts();
	currentFaction =0;

// 	renderZoomString = "%1 \%";

	tagLayout = new QGridLayout ( tagPage );

	abcScene = new QGraphicsScene;
	loremScene = new QGraphicsScene;
	QRectF pageRect ( 0,0,597.6,842.4 ); //TODO find means to smartly decide of page size (here, iso A4)
	loremScene->setSceneRect ( pageRect );
	QGraphicsRectItem *backp = loremScene->addRect ( pageRect,QPen(),Qt::white );
	backp->setEnabled ( false );

	abcView->setScene ( abcScene );
	abcView->setRenderHint ( QPainter::Antialiasing, true );

	loremView->setScene ( loremScene );
	loremView->setRenderHint ( QPainter::Antialiasing, true );
	loremView->setBackgroundBrush ( Qt::lightGray );
	loremView->locker = true;
// 	loremView->ensureVisible ( loremScene->sceneRect() );

	sampleText= "ABCDEFGHIJKLMNOPQRSTUVXYZ\n\n  abcdefghijklmnopqrstuvxyz\n  0123456789   ,;:!?.";
	sampleFontSize = 18;
	sampleInterSize = 20;

// 	ord << "family" << "variant";
// 	orderingCombo->addItems ( ord );
	tagsetCombo->addItems ( typo->tagsets() );

// 	fields << "family" << "variant";
// 	searchField->addItems ( fields );

	QStringList tl_tmp = typotek::tagsList;
// 	qDebug() << "TAGLIST\n" << typotek::tagsList.join ( "\n" );
	tl_tmp.removeAll ( "Activated_On" );
	tl_tmp.removeAll ( "Activated_Off" );

	tagsCombo->addItems ( tl_tmp );

	//CONNECT

	connect ( tagsetCombo,SIGNAL ( activated ( const QString ) ),this,SLOT ( slotFilterTagset ( QString ) ) );

	connect ( fontTree,SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ),this,SLOT ( slotfontSelected ( QTreeWidgetItem*, int ) ) );

// 	connect ( editAllButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotEditAll() ) );

// 	connect ( this,SIGNAL ( faceChanged() ),this,SLOT ( slotView() ) );


	connect ( abcScene,SIGNAL ( selectionChanged() ),this,SLOT ( slotglyphInfo() ) );

	connect ( searchButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotSearch() ) );
	connect ( searchString,SIGNAL ( returnPressed() ),this,SLOT ( slotSearch() ) );
	connect (viewAllButton,SIGNAL(released()),this,SLOT(slotViewAll()));
	connect(viewActivatedButton,SIGNAL(released()),this,SLOT(slotViewActivated()));

	connect ( renderZoom,SIGNAL ( valueChanged ( int ) ),this,SLOT ( slotZoom ( int ) ) );
	connect ( allZoom,SIGNAL ( valueChanged ( int ) ),this,SLOT ( slotZoom ( int ) ) );

	connect ( tagsCombo,SIGNAL ( activated ( const QString& ) ),this,SLOT ( slotFilterTag ( QString ) ) );

// 	connect ( activateAllButton,SIGNAL ( released() ),this,SLOT ( slotActivateAll() ) );
// 	connect ( desactivateAllButton,SIGNAL ( released() ),this,SLOT ( slotDesactivateAll() ) );

	connect ( textButton,SIGNAL ( released() ),this,SLOT ( slotSetSampleText() ) );

	connect ( typo,SIGNAL ( tagAdded ( QString ) ),this,SLOT ( slotAppendTag ( QString ) ) );

	connect ( codepointSelectText,SIGNAL ( returnPressed() ),this,SLOT ( slotShowCodePoint() ) );

	connect ( antiAliasButton,SIGNAL ( toggled ( bool ) ),this,SLOT ( slotSwitchAntiAlias ( bool ) ) );
	
	connect (fitViewCheck,SIGNAL(stateChanged( int )),this,SLOT(slotFitChanged(int)));
	connect (loremView, SIGNAL(refit()),this,SLOT(slotRefitSample()));

	connect(fontTree,SIGNAL(itemExpanded( QTreeWidgetItem* )),this,SLOT(slotItemOpened(QTreeWidgetItem*)));
	// END CONNECT



	slotOrderingChanged ( "family" );


}


MainViewWidget::~MainViewWidget()
{
}


void MainViewWidget::fillTree()
{
	qDebug() << "curitemname = " << curItemName;
// 	if(curItemName.isEmpty() && currentFonts.count())
// 		curItemName = currentFonts.first()->name();
	QTreeWidgetItem *curItem = 0;
	openKeys.clear();
	for ( int i=0; i < fontTree->topLevelItemCount();++i )
	{
		QTreeWidgetItem *topit = fontTree->topLevelItem ( i );
		for(int j=0;j < topit->childCount();++j)
		if ( topit->child(j)->isExpanded() )
				openKeys << topit->child(j)->text ( 0 );
	}
// qDebug() << "openjey : " << openKeys.join("/");
	
	fontTree->clear();
	QMap<QString, QList<FontItem*> > keyList;
	for ( int i=0; i < currentFonts.count();++i )
	{
		keyList[currentFonts[i]->value ( currentOrdering ) ].append ( currentFonts[i] );
	}

	QMap<QString, QList<FontItem*> >::const_iterator kit;
	for ( int i = 'A'; i <= 'Z'; ++i )
	{
		QChar firstChar ( i );
		QTreeWidgetItem *alpha = new QTreeWidgetItem ( fontTree );
		alpha->setText ( 0, firstChar );
		alpha->setData(0,100,"alpha");
		bool alphaIsUsed = false;
		
		for ( kit = keyList.begin(); kit != keyList.end(); ++kit )
		{
			bool isExpanded = false;
			if ( kit.key().at ( 0 ).toUpper() == firstChar )
			{
				QTreeWidgetItem *ord = new QTreeWidgetItem ( alpha );
				ord->setText ( 0, kit.key() );
				ord->setData(0,100,"family");
				ord->setCheckState(0,Qt::Unchecked);
				bool chekno = false;
				bool checkyes = false;
				if ( openKeys.contains ( kit.key() ) )
				{
					ord->setExpanded ( true );
					isExpanded = true;
				}
				if(kit.value().count())
				{
					ord->setIcon(2,kit.value()[0]->oneLinePreviewIcon());
				}
				for ( int  n = 0; n < kit.value().count(); ++n )
				{
					QTreeWidgetItem *entry = new QTreeWidgetItem ( ord );
					entry->setText ( 0, kit.value() [n]->variant() );
					entry->setText ( 1, kit.value() [n]->name() );
					entry->setData ( 0, 100, "fontfile");
					entry->setData(0,200,entry->checkState(1));
// 					entry->setIcon ( 2, kit.value() [n]->oneLinePreviewIcon());
					if(isExpanded)
						entry->setBackground(2,QBrush(kit.value()[n]->oneLinePreviewPixmap()));
					
					bool act = kit.value() [n]->tags().contains ( "Activated_On" );
					if(act)
					{
						checkyes = true;
					}
					else
					{
						chekno = true;
					}
					entry->setCheckState ( 1, act ?  Qt::Checked : Qt::Unchecked );
					if ( entry->text ( 1 ) == curItemName )
						curItem = entry;
				}
				if(checkyes && chekno)
					ord->setCheckState(0,Qt::PartiallyChecked);
				else if(checkyes)
					ord->setCheckState(0,Qt::Checked);
				// track checkState
				ord->setData(0,200,ord->checkState(0));
				ord->setText(1,QString::number( ord->childCount() ));
				

				alphaIsUsed = true;
			}
		}
		if ( alphaIsUsed )
		{
			fontTree->addTopLevelItem ( alpha );
			alpha->setExpanded ( true );
		}
		else
		{
			delete alpha;
		}
	}
	if ( curItem )
	{
		qDebug() << "get curitem : " << curItem->text ( 0 ) << curItem->text ( 1 );
		fontTree->scrollToItem ( curItem, QAbstractItemView::PositionAtCenter );
		QColor scol(Qt::blue);
		scol.setAlpha(30);
		curItem->parent()->setBackgroundColor(0,scol);
		curItem->parent()->setBackgroundColor(1,scol);
		curItem->setBackgroundColor(0,scol);
		curItem->setBackgroundColor(1,scol);
	}
	else
	{
		qDebug() << "NO CURITEM";
	}
	fontTree->resizeColumnToContents ( 0 )  ;
// 	fontTree->resizeColumnToContents ( 1 ) ;
// 	fontTree->setColumnWidth(0,200);
}

void MainViewWidget::slotItemOpened(QTreeWidgetItem * item)
{
	if(item->data(0,100).toString() == "family")
	{
		for(int i=0; i<item->childCount(); ++i)
		{
			QString font= item->child(i)->text(1);
			if(typo->getFont(font))
			{
				item->child(i)->setBackground(2, QBrush(typo->getFont(font)->oneLinePreviewPixmap()));
			}
		}	
	}
	
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

// 	curItemName = item->text ( 1 ).isNull() ? item->text ( 0 ) : item->text ( 1 );
	
	
	if ( item->data(0,100).toString() == "alpha" )
	{
		qDebug() << "Item is an alpha";
		return;
		fillTree();
	}
	
	if(item->data(0,100).toString() == "family")
	{
		qDebug() << "Item is a family";
		curItemName = item->child(0)->text(1);
// 		qDebug() << curItemName;
		int oldc = item->data(0,200).toInt();
		if(oldc == item->checkState(0))
		{
			fillTree();
			return;
			
		}
		if(item->checkState(0) != Qt::PartiallyChecked)
		{
			
			bool cs = item->checkState(0) == Qt::Checked ? true : false;
			
			QList<FontItem*> todo;
			for(int i=0; i<item->childCount(); ++i)
			{
				todo << typo->getFont(item->child(i)->text(1));		
			}
			foreach(FontItem* afont, todo)
			{
				activation(afont, cs);
			}
			
		}
		else
		{
			qDebug() << "Something wrong djhgjksdhgjdsh_grepstring";
			
		}
		fillTree();
		return;
	}
	
	if(item->data(0,100).toString() == "fontfile")
	{
		qDebug() << "Item is a fontfile";
		lastIndex = faceIndex;
		faceIndex = item->text ( 1 );
		curItemName = faceIndex;
	
		if ( faceIndex.count() && faceIndex != lastIndex )
		{
			qDebug() << "\tFont has changed";
			slotFontAction ( item,column );
// 			emit faceChanged();
			slotView(true);
			theVeryFont = typo->getFont ( faceIndex );
			typo->setWindowTitle(theVeryFont->fancyName());
		}
	
		if(item->data(0,200).toInt() != item->checkState(1))
		{
			if ( item->checkState ( 1 ) == Qt::Checked )
				slotActivate ( true, item, column );
			else
				slotActivate ( false, item, column );
		}
		fillTree();
		return;
	}
	return;

}

void MainViewWidget::slotInfoFont()
{
	FontItem *f = typo->getFont ( faceIndex );
	fontInfoText->clear();
	//QString t(QString("Family : %1\nStyle : %2\nFlags : \n%3").arg(f->family()).arg(f->variant()).arg(f->faceFlags()));
	fontInfoText->setText ( f->infoText() );

}

void MainViewWidget::slotView(bool needDeRendering)
{
	FontItem *l = typo->getFont ( lastIndex );
	FontItem *f = typo->getFont ( faceIndex );
	if ( !f )
		return;
	if(needDeRendering)
	{
		if ( l )
			l->deRenderAll();
		f->deRenderAll();
	}

	QApplication::setOverrideCursor ( Qt::WaitCursor );
	f->renderAll ( abcScene ); // can be rather long depending of the number of glyphs
	QApplication::restoreOverrideCursor();

	QStringList stl = sampleText.split ( '\n' );
	QPointF pen ( 100,80 );
	QApplication::setOverrideCursor ( Qt::WaitCursor );
	for ( int i=0; i< stl.count(); ++i )
	{
		pen.ry() = 100 + sampleInterSize * i;
		f->renderLine ( loremScene,stl[i],pen, sampleFontSize );
	}
	QApplication::restoreOverrideCursor();
	slotInfoFont();
	if(fitViewCheck->isChecked())
	{
		QRectF allrect, firstrect;
		bool first = true;
		QList<QGraphicsItem*> lit = loremScene->items();
		for(int i = 0 ; i <lit.count() ; ++i )
		{
			if(lit[i]->data(1).toString() == "glyph")
			{
				if(first)
				{
					firstrect = lit[i]->sceneBoundingRect();
					first = false;
					
				}
				if(lit[i]->sceneBoundingRect().bottomRight().y() > allrect.bottomRight().y())
					allrect = allrect.united(lit[i]->sceneBoundingRect());
				if(lit[i]->sceneBoundingRect().bottomRight().x() > allrect.bottomRight().x())
					allrect = allrect.united(lit[i]->sceneBoundingRect());
		
			}
			
				
		}
		
		qDebug() << allrect;
		loremView->fitInView(allrect, Qt::KeepAspectRatio);
	}


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
	QString ff ( "search_%1" );
	QString sensitivity("INSENS");
	if(sensitivityCheck->isChecked())
	{
		sensitivity = "SENS";
	}

	currentFonts = typo->getFonts ( fs,ff.arg(sensitivity) ) ;
	currentOrdering = "family";
	fillTree();
	searchString->clear();
}

void MainViewWidget::slotFilterTag ( QString tag )
{
	fontTree->clear();

	QString fs ( tag );
	QString ff ( "tag" );

	currentFonts = typo->getFonts ( fs,ff ) ;
	currentOrdering = "family";
	fillTree();
}

void MainViewWidget::slotFilterTagset ( QString set )
{
	fontTree->clear();
	currentFonts.clear();
	QStringList tags = typo->tagsOfSet ( set );
	if ( !tags.count() )
		return;
	
	for(int i = 0;i < tags.count(); ++i)
	{
		currentFonts += typo->getFonts ( tags[i],"tag" );
	}
	int count_req = tags.count();
	QSet<FontItem*> setOfTags(currentFonts.toSet());
	foreach(FontItem * it, setOfTags)
	{
// 		qDebug() << it->name();
		if(currentFonts.count(it) != count_req)
			currentFonts.removeAll(it);
		else
			qDebug() << count_req<<currentFonts.count(it);
	}
	

	
	currentOrdering = "family";
	currentFonts = currentFonts.toSet().toList();
	fillTree();
}


void MainViewWidget::slotFontAction ( QTreeWidgetItem * item, int column )
{
	if(column >2 )return;
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
// 	typo->save();
// 	qDebug() << " FontActionWidget  saved";
}

void MainViewWidget::slotZoom ( int z )
{
	QGraphicsView * concernedView;
	if ( sender()->objectName().contains ( "render" ) )
	{
		concernedView = loremView;
// 		renderZoomLabel->setText(renderZoomString.arg(z));
	}
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

				if ( !QFile::link ( fit->path() , typo->getManagedDir() + "/" + fofi.fileName() ) )
				{
					qDebug() << "unable to link " << fofi.fileName();
				}
				else
				{
					if ( !fit->afm().isEmpty() )
					{
						QFileInfo afm ( fit->afm() );
						if ( !QFile::link ( fit->afm(), typo->getManagedDir() + "/" + afm.fileName() ) )
						{
							qDebug() << "unable to link " << afm.fileName();
						}
					}
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
				if ( !QFile::remove ( typo->getManagedDir() + "/" + fofi.fileName() ) )
				{
					qDebug() << "unable to remove " << fofi.fileName();
				}
				else
				{
					if ( !fit->afm().isEmpty() )
					{
						QFileInfo afm ( fit->afm() );
						if ( !QFile::remove ( typo->getManagedDir() + "/" + afm.fileName() ) )
						{
							qDebug() << "unable to remove " << afm.fileName();
						}
					}
					typo->adaptator()->private_signal ( 0, fofi.fileName() );
				}
			}

		}
	}
	fillTree();
// 	typo->save();
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
	QTextEdit ted ( sampleText.replace ( "\n","<br/>" ) );
	QPushButton okButton ( "Ok" );

	QLabel labfs ( "size" );
	QLabel labls ( "interline" );

	QDoubleSpinBox boxfs;
	boxfs.setRange ( 1,999 );
	boxfs.setValue ( sampleFontSize );

	QDoubleSpinBox boxls;
	boxls.setRange ( 1,999 );
	boxls.setValue ( sampleInterSize );

	lay.addWidget ( &ted, 0,0,1,-1 );
	lay.addWidget ( &labfs, 1,0 );
	lay.addWidget ( &labls, 2,0 );
	lay.addWidget ( &boxfs,1,1 );
	lay.addWidget ( &boxls,2,1 );
	lay.addWidget ( &okButton,3,1 );
	connect ( &okButton,SIGNAL ( released() ),&dial,SLOT ( close() ) );

	dial.exec();
	sampleText = ted.toPlainText () ;
	sampleInterSize = boxls.value();
	sampleFontSize = boxfs.value();

	slotView(true);

}

void MainViewWidget::slotActivate ( bool act, QTreeWidgetItem * item, int column )
{
	if(column >2 )return;
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

void MainViewWidget::slotReloadTagsetList()
{
	tagsetCombo->clear();
	tagsetCombo->addItems ( typo->tagsets() );
}

void MainViewWidget::slotShowCodePoint()
{
	QString codetext = codepointSelectText->text();
	bool ok;
	int codepoint = codetext.toInt ( &ok, 16 );
	if ( !ok )
		return;
	if ( !theVeryFont )
		return;
	QGraphicsPathItem *pit = theVeryFont->hasCodepoint ( codepoint );
	if ( !pit )
		return;

	abcView->fitInView ( pit, Qt::KeepAspectRatio );



}

void MainViewWidget::slotSwitchAntiAlias ( bool aa )
{
	loremView->setRenderHint ( QPainter::Antialiasing, aa );
}

void MainViewWidget::slotFitChanged(int i)
{
	if(i == Qt::Unchecked)
	{
		renderZoom->setDisabled(false);
		renderZoom->setStatusTip("zoom is enabled");
		loremView->setTransform(QTransform(1,0,0,1,0,0),false);
		loremView->locker = false;
	}
	else
	{
		renderZoom->setDisabled(true);
		renderZoom->setStatusTip("zoom is disabled, uncheck fit to view to enable zoom");
		loremView->locker = true;
	}
	slotView();
}

void MainViewWidget::slotRefitSample()
{
	if(fitViewCheck->isChecked())
		slotView();
}

void MainViewWidget::slotViewAll()
{
	currentFonts = typo->getAllFonts();
	fillTree();
}

void MainViewWidget::slotViewActivated()
{
	slotFilterTag ( "Activated_On" );
}









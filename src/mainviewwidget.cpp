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
	
	fontTree->setIconSize(QSize(32,32));

	fillUniPlanes();
// 	uniPlaneCombo->addItems(uniPlanes.keys());
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
	connect ( uniPlaneCombo,SIGNAL(activated(int)),this,SLOT(slotPlaneSelected(int)));

	connect ( antiAliasButton,SIGNAL ( toggled ( bool ) ),this,SLOT ( slotSwitchAntiAlias ( bool ) ) );
	
	connect (fitViewCheck,SIGNAL(stateChanged( int )),this,SLOT(slotFitChanged(int)));
	connect (loremView, SIGNAL(refit()),this,SLOT(slotRefitSample()));

// 	connect(fontTree,SIGNAL(itemExpanded( QTreeWidgetItem* )),this,SLOT(slotItemOpened(QTreeWidgetItem*)));
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
// 					if(isExpanded)
// 						entry->setBackground(2,QBrush(kit.value()[n]->oneLinePreviewPixmap()));
					
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
// 		QColor scol(Qt::blue);
// 		scol.setAlpha(30);
// 		curItem->parent()->setBackgroundColor(0,scol);
// 		curItem->parent()->setBackgroundColor(1,scol);
// 		curItem->setBackgroundColor(0,scol);
// 		curItem->setBackgroundColor(1,scol);
		curItem->setSelected(true);
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
			theVeryFont = typo->getFont ( faceIndex );
			fillUniPlanesCombo(theVeryFont); // has to be called before view, may I should come back to the faceChanged signal idea
			slotView(true);
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
	
	QString pkey = uniPlaneCombo->itemData( uniPlaneCombo->currentIndex() ).toString();
	QPair<int,int> uniPair(uniPlanes[pkey + uniPlaneCombo->currentText()]);
	qDebug() <<  pkey << uniPlaneCombo->currentText() <<  uniPair.first << uniPair.second;
	f->renderAll ( abcScene , uniPair.first, uniPair.second); 
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

void MainViewWidget::slotPlaneSelected(int i)
{
	slotView(true);
}

void MainViewWidget::fillUniPlanes()
{	
// 	uniPlanes[ "Basic Multilingual Plane (BMP)"] = qMakePair(0x0000,0xFFFF) ;
	// BMP is huge, we split it into langs
	uniPlanes[ "000Basic Latin" ] = qMakePair(0x0000,0x007F);
	uniPlanes[ "001Latin-1 Supplement" ] = qMakePair(0x0080,0x00FF);
	uniPlanes[ "002Latin Extended-A" ] = qMakePair(0x0100,0x017F);
	uniPlanes[ "003Latin Extended-B" ] = qMakePair(0x0180,0x024F);
	uniPlanes[ "004IPA Extensions" ] = qMakePair(0x0250,0x02AF);
	uniPlanes[ "005Spacing Modifier Letters" ] = qMakePair(0x02B0,0x02FF);
	uniPlanes[ "006Combining Diacritical Marks" ] = qMakePair(0x0300,0x036F);
	uniPlanes[ "007Greek and Coptic" ] = qMakePair(0x0370,0x03FF);
	uniPlanes[ "008Cyrillic" ] = qMakePair(0x0400,0x04FF);
	uniPlanes[ "009Cyrillic Supplement" ] = qMakePair(0x0500,0x052F);
	uniPlanes[ "010Armenian" ] = qMakePair(0x0530,0x058F);
	uniPlanes[ "011Hebrew" ] = qMakePair(0x0590,0x05FF);
	uniPlanes[ "012Arabic" ] = qMakePair(0x0600,0x06FF);
	uniPlanes[ "013Syriac" ] = qMakePair(0x0700,0x074F);
	uniPlanes[ "014Arabic Supplement" ] = qMakePair(0x0750,0x077F);
	uniPlanes[ "015Thaana" ] = qMakePair(0x0780,0x07BF);
	uniPlanes[ "016N'Ko" ] = qMakePair(0x07C0,0x07FF);
	uniPlanes[ "017Devanagari" ] = qMakePair(0x0900,0x097F);
	uniPlanes[ "018Bengali" ] = qMakePair(0x0980,0x09FF);
	uniPlanes[ "019Gurmukhi" ] = qMakePair(0x0A00,0x0A7F);
	uniPlanes[ "020Gujarati" ] = qMakePair(0x0A80,0x0AFF);
	uniPlanes[ "021Oriya" ] = qMakePair(0x0B00,0x0B7F);
	uniPlanes[ "022Tamil" ] = qMakePair(0x0B80,0x0BFF);
	uniPlanes[ "023Telugu" ] = qMakePair(0x0C00,0x0C7F);
	uniPlanes[ "024Kannada" ] = qMakePair(0x0C80,0x0CFF);
	uniPlanes[ "025Malayalam" ] = qMakePair(0x0D00,0x0D7F);
	uniPlanes[ "026Sinhala" ] = qMakePair(0x0D80,0x0DFF);
	uniPlanes[ "027Thai" ] = qMakePair(0x0E00,0x0E7F);
	uniPlanes[ "028Lao" ] = qMakePair(0x0E80,0x0EFF);
	uniPlanes[ "029Tibetan" ] = qMakePair(0x0F00,0x0FFF);
	uniPlanes[ "030Burmese" ] = qMakePair(0x1000,0x109F);
	uniPlanes[ "031Georgian" ] = qMakePair(0x10A0,0x10FF);
	uniPlanes[ "032Hangul Jamo" ] = qMakePair(0x1100,0x11FF);
	uniPlanes[ "033Ethiopic" ] = qMakePair(0x1200,0x137F);
	uniPlanes[ "034Ethiopic Supplement" ] = qMakePair(0x1380,0x139F);
	uniPlanes[ "035Cherokee" ] = qMakePair(0x13A0,0x13FF);
	uniPlanes[ "036Unified Canadian Aboriginal Syllabics" ] = qMakePair(0x1400,0x167F);
	uniPlanes[ "037Ogham" ] = qMakePair(0x1680,0x169F);
	uniPlanes[ "à38Runic" ] = qMakePair(0x16A0,0x16FF);
	uniPlanes[ "039Tagalog" ] = qMakePair(0x1700,0x171F);
	uniPlanes[ "040Hanunóo" ] = qMakePair(0x1720,0x173F);
	uniPlanes[ "041Buhid" ] = qMakePair(0x1740,0x175F);
	uniPlanes[ "042Tagbanwa" ] = qMakePair(0x1760,0x177F);
	uniPlanes[ "043Khmer" ] = qMakePair(0x1780,0x17FF);
	uniPlanes[ "044Mongolian" ] = qMakePair(0x1800,0x18AF);
	uniPlanes[ "045Limbu" ] = qMakePair(0x1900,0x194F);
	uniPlanes[ "046Tai Le" ] = qMakePair(0x1950,0x197F);
	uniPlanes[ "047New Tai Lue" ] = qMakePair(0x1980,0x19DF);
	uniPlanes[ "048Khmer Symbols" ] = qMakePair(0x19E0,0x19FF);
	uniPlanes[ "049Buginese" ] = qMakePair(0x1A00,0x1A1F);
	uniPlanes[ "050Balinese" ] = qMakePair(0x1B00,0x1B7F);
	uniPlanes[ "051Lepcha" ] =  qMakePair(0x1C00,0x1C4F);
	uniPlanes[ "052Phonetic Extensions" ] = qMakePair(0x1D00,0x1D7F);
	uniPlanes[ "053Phonetic Extensions Supplement" ] = qMakePair(0x1D80,0x1DBF);
	uniPlanes[ "054Combining Diacritical Marks Supplement" ] = qMakePair(0x1DC0,0x1DFF);
	uniPlanes[ "055Latin Extended Additional" ] = qMakePair(0x1E00,0x1EFF);
	uniPlanes[ "056Greek Extended" ] = qMakePair(0x1F00,0x1FFF);
	uniPlanes[ "057General Punctuation" ] = qMakePair(0x2000,0x206F);
	uniPlanes[ "058Superscripts and Subscripts" ] = qMakePair(0x2070,0x209F);
	uniPlanes[ "059Currency Symbols" ] = qMakePair(0x20A0,0x20CF);
	uniPlanes[ "060Combining Diacritical Marks for Symbols" ] = qMakePair(0x20D0,0x20FF);
	uniPlanes[ "061Letterlike Symbols" ] = qMakePair(0x2100,0x214F);
	uniPlanes[ "062Number Forms" ] = qMakePair(0x2150,0x218F);
	uniPlanes[ "063Arrows" ] = qMakePair(0x2190,0x21FF);
	uniPlanes[ "064Mathematical Operators" ] = qMakePair(0x2200,0x22FF);
	uniPlanes[ "065Miscellaneous Technical" ] = qMakePair(0x2300,0x23FF);
	uniPlanes[ "066Control Pictures" ] = qMakePair(0x2400,0x243F);
	uniPlanes[ "067Optical Character Recognition" ] = qMakePair(0x2440,0x245F);
	uniPlanes[ "068Enclosed Alphanumerics" ] = qMakePair(0x2460,0x24FF);
	uniPlanes[ "069Box Drawing" ] = qMakePair(0x2500,0x257F);
	uniPlanes[ "070Block Elements" ] = qMakePair(0x2580,0x259F);
	uniPlanes[ "071Geometric Shapes" ] = qMakePair(0x25A0,0x25FF);
	uniPlanes[ "072Miscellaneous Symbols" ] = qMakePair(0x2600,0x26FF);
	uniPlanes[ "073Dingbats" ] = qMakePair(0x2700,0x27BF);
	uniPlanes[ "074Miscellaneous Mathematical Symbols-A" ] = qMakePair(0x27C0,0x27EF);
	uniPlanes[ "075Supplemental Arrows-A" ] = qMakePair(0x27F0,0x27FF);
	uniPlanes[ "076Braille Patterns" ] = qMakePair(0x2800,0x28FF);
	uniPlanes[ "077Supplemental Arrows-B" ] = qMakePair(0x2900,0x297F);
	uniPlanes[ "078Miscellaneous Mathematical Symbols-B" ] = qMakePair(0x2980,0x29FF);
	uniPlanes[ "079Supplemental Mathematical Operators" ] = qMakePair(0x2A00,0x2AFF);
	uniPlanes[ "080Miscellaneous Symbols and Arrows" ] = qMakePair(0x2B00,0x2BFF);
	uniPlanes[ "081Glagolitic" ] = qMakePair(0x2C00,0x2C5F);
	uniPlanes[ "082Latin Extended-C" ] = qMakePair(0x2C60,0x2C7F);
	uniPlanes[ "083Coptic" ] = qMakePair(0x2C80,0x2CFF);
	uniPlanes[ "084Georgian Supplement" ] = qMakePair(0x2D00,0x2D2F);
	uniPlanes[ "085Tifinagh" ] = qMakePair(0x2D30,0x2D7F);
	uniPlanes[ "086Ethiopic Extended" ] = qMakePair(0x2D80,0x2DDF);
	uniPlanes[ "087Supplemental Punctuation" ] = qMakePair(0x2E00,0x2E7F);
	uniPlanes[ "088CJK Radicals Supplement" ] = qMakePair(0x2E80,0x2EFF);
	uniPlanes[ "089Kangxi Radicals" ] = qMakePair(0x2F00,0x2FDF);
	uniPlanes[ "090Ideographic Description Characters" ] = qMakePair(0x2FF0,0x2FFF);
	uniPlanes[ "091CJK Symbols and Punctuation" ] = qMakePair(0x3000,0x303F);
	uniPlanes[ "092Hiragana" ] = qMakePair(0x3040,0x309F);
	uniPlanes[ "093Katakana" ] = qMakePair(0x30A0,0x30FF);
	uniPlanes[ "094Bopomofo" ] = qMakePair(0x3100,0x312F);
	uniPlanes[ "095Hangul Compatibility Jamo" ] = qMakePair(0x3130,0x318F);
	uniPlanes[ "096Kanbun" ] = qMakePair(0x3190,0x319F);
	uniPlanes[ "097Bopomofo Extended" ] = qMakePair(0x31A0,0x31BF);
	uniPlanes[ "098CJK Strokes" ] = qMakePair(0x31C0,0x31EF);
	uniPlanes[ "099Katakana Phonetic Extensions" ] = qMakePair(0x31F0,0x31FF);
	uniPlanes[ "100Enclosed CJK Letters and Months" ] = qMakePair(0x3200,0x32FF);
	uniPlanes[ "101CJK Compatibility" ] = qMakePair(0x3300,0x33FF);
	uniPlanes[ "102CJK Unified Ideographs Extension A" ] = qMakePair(0x3400,0x4DBF);
	uniPlanes[ "103Yijing Hexagram Symbols" ] = qMakePair(0x4DC0,0x4DFF);
	uniPlanes[ "104CJK Unified Ideographs" ] = qMakePair(0x4E00,0x9FFF);
	uniPlanes[ "105Yi Syllables" ] = qMakePair(0xA000,0xA48F);
	uniPlanes[ "106Yi Radicals" ] = qMakePair(0xA490,0xA4CF);
	uniPlanes[ "107Modifier Tone Letters" ] = qMakePair(0xA700,0xA71F);
	uniPlanes[ "108Latin Extended-D" ] = qMakePair(0xA720,0xA7FF);
	uniPlanes[ "109Syloti Nagri" ] = qMakePair(0xA800,0xA82F);
	uniPlanes[ "110Phags-pa" ] = qMakePair(0xA840,0xA87F);
	uniPlanes[ "111Hangul Syllables" ] = qMakePair(0xAC00,0xD7AF);
	uniPlanes[ "112High Surrogates" ] = qMakePair(0xD800,0xDB7F);
	uniPlanes[ "113High Private Use Surrogates" ] = qMakePair(0xDB80,0xDBFF);
	uniPlanes[ "114Low Surrogates" ] = qMakePair(0xDC00,0xDFFF);
	uniPlanes[ "115Private Use Area" ] = qMakePair(0xE000,0xF8FF);
	uniPlanes[ "116CJK Compatibility Ideographs" ] = qMakePair(0xF900,0xFAFF);
	uniPlanes[ "117Alphabetic Presentation Forms" ] = qMakePair(0xFB00,0xFB4F);
	uniPlanes[ "118Arabic Presentation Forms-A" ] = qMakePair(0xFB50,0xFDFF);
	uniPlanes[ "119Variation Selectors" ] = qMakePair(0xFE00,0xFE0F);
	uniPlanes[ "120Vertical Forms" ] = qMakePair(0xFE10,0xFE1F);
	uniPlanes[ "121Combining Half Marks" ] = qMakePair(0xFE20,0xFE2F);
	uniPlanes[ "122CJK Compatibility Forms" ] = qMakePair(0xFE30,0xFE4F);
	uniPlanes[ "123Small Form Variants" ] = qMakePair(0xFE50,0xFE6F);
	uniPlanes[ "124Arabic Presentation Forms-B" ] = qMakePair(0xFE70,0xFEFF);
	uniPlanes[ "125Halfwidth and Fullwidth Forms" ] = qMakePair(0xFF00,0xFFEF);
	uniPlanes[ "126Specials" ] = qMakePair(0xFFF0,0xFFFF);

	uniPlanes["127Supplementary Multilingual Plane (SMP)"] = qMakePair(0x10000,0x1FFFF) ;
	uniPlanes["128Supplementary Ideographic Plane (SIP)"] = qMakePair(0x20000,0x2FFFF) ;
	uniPlanes["129unassigned"] = qMakePair(0x30000,0xDFFFF) ;
	uniPlanes["130Supplementary Special-purpose Plane (SSP)"] = qMakePair(0xE0000,0xEFFFF) ;
	uniPlanes["131Private Use Area 1 (PUA)"] = qMakePair(0xF0000,0xFFFFF) ;
	uniPlanes["132Private Use Area 2 (PUA)"] = qMakePair(0x100000,0x10FFFF) ;
}

void MainViewWidget::fillUniPlanesCombo(FontItem* item)
{
	uniPlaneCombo->clear();
	QStringList plist= uniPlanes.keys();
	for(int i= 0;i<plist.count();++i)
	{
		QString p=plist.at(i);
		if(p.isEmpty())
			continue;
		int begin = uniPlanes[p].first;
		int end = uniPlanes[p].second;
		int codecount = item->countCoverage(begin,end);
		if( codecount > 0)
		{
			qDebug() << p << codecount;
			uniPlaneCombo->addItem(p.mid(3), p.mid(0,3));
		}
	}
	uniPlaneCombo->setCurrentIndex(0);
	
}



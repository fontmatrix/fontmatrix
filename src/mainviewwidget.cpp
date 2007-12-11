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
// #include "typotekadaptator.h"
#include "fmpreviewlist.h"
#include "fmglyphsview.h"
#include "listdockwidget.h"
#include "fmotf.h"
#include "opentypetags.h"


#include <QString>
#include <QDebug>
#include <QGraphicsItem>
#include <QTransform>
#include <QDialog>
#include <QGridLayout>
#include <QGraphicsRectItem>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QScrollBar>
#include <QGraphicsRectItem>
// #include <QTimeLine>
// #include <QGraphicsItemAnimation>



MainViewWidget::MainViewWidget ( QWidget *parent )
		: QWidget ( parent )
{
	setupUi ( this );
	theVeryFont = 0;
	typo = typotek::getInstance();
	m_lists = ListDockWidget::getInstance();
	currentFonts = typo->getAllFonts();
	currentFaction =0;
	fontsetHasChanged = true;
	curGlyph = 0;
	fontInfoText->setSource(QUrl("qrc:/texts/welcome"));
	fillUniPlanes();
	sampleTextCombo->addItems(typo->namedSamplesNames());
	
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
	loremView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	loremView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	sampleText= typo->namedSample("default");
	sampleFontSize = 18;
	sampleInterSize = 20;
	m_lists->previewList->setRefWidget(this);

	//CONNECT

	connect ( m_lists->tagsetCombo,SIGNAL ( activated ( const QString ) ),this,SLOT ( slotFilterTagset ( QString ) ) );
	connect (  m_lists->fontTree,SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ),this,SLOT ( slotFontSelected ( QTreeWidgetItem*, int ) ) );
	connect (  m_lists->searchButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotSearch() ) );
	connect (  m_lists->searchString,SIGNAL ( returnPressed() ),this,SLOT ( slotSearch() ) );
	connect ( m_lists->viewAllButton,SIGNAL(released()),this,SLOT(slotViewAll()));
	connect ( m_lists->viewActivatedButton,SIGNAL(released()),this,SLOT(slotViewActivated()));
	connect( m_lists->fontTree,SIGNAL(itemExpanded( QTreeWidgetItem* )),this,SLOT(slotItemOpened(QTreeWidgetItem*)));
		
	connect ( abcScene,SIGNAL ( selectionChanged() ),this,SLOT ( slotglyphInfo() ) );
	connect ( renderZoom,SIGNAL ( valueChanged ( int ) ),this,SLOT ( slotZoom ( int ) ) );
// 	connect ( allZoom,SIGNAL ( valueChanged ( int ) ),this,SLOT ( slotZoom ( int ) ) );
	connect (  m_lists->tagsCombo,SIGNAL ( activated ( const QString& ) ),this,SLOT ( slotFilterTag ( QString ) ) );
// 	connect ( activateAllButton,SIGNAL ( released() ),this,SLOT ( slotActivateAll() ) );
// 	connect ( desactivateAllButton,SIGNAL ( released() ),this,SLOT ( slotDesactivateAll() ) );
// 	connect ( textButton,SIGNAL ( released() ),this,SLOT ( slotSetSampleText() ) );
	connect ( typo,SIGNAL ( tagAdded ( QString ) ),this,SLOT ( slotAppendTag ( QString ) ) );
// 	connect ( codepointSelectText,SIGNAL ( returnPressed() ),this,SLOT ( slotShowCodePoint() ) );
	connect ( uniPlaneCombo,SIGNAL(activated(int)),this,SLOT(slotPlaneSelected(int)));
	connect ( antiAliasButton,SIGNAL ( toggled ( bool ) ),this,SLOT ( slotSwitchAntiAlias ( bool ) ) );
	connect (fitViewCheck,SIGNAL(stateChanged( int )),this,SLOT(slotFitChanged(int)));
	connect (loremView, SIGNAL(refit()),this,SLOT(slotRefitSample()));
	connect(abcView,SIGNAL(refit(int)),this,SLOT(slotAdjustGlyphView(int)));
	connect(OTFeaturesButton, SIGNAL(clicked()), this, SLOT(slotFeatureChanged()));
	connect(sampleTextCombo,SIGNAL(activated( int )),this,SLOT(slotSampleChanged()));
	connect(freetypeButton,SIGNAL(released()),this,SLOT(slotFTRasterChanged()));
	// END CONNECT

	currentOrdering = "family" ;
	fillTree();


}


MainViewWidget::~MainViewWidget()
{
}


void MainViewWidget::fillTree()
{

	QTreeWidgetItem *curItem = 0;
	openKeys.clear();
	for ( int i=0; i < m_lists->fontTree->topLevelItemCount();++i )
	{
		QTreeWidgetItem *topit = m_lists->fontTree->topLevelItem ( i );
		for ( int j=0;j < topit->childCount();++j )
			if ( topit->child ( j )->isExpanded() )
				openKeys << topit->child ( j )->text ( 0 );
	}
// qDebug() << "openjey : " << openKeys.join("/");
	
	QFont alphaFont("helvetica",14,QFont::Bold,false);

	m_lists->fontTree->clear();
	QMap<QString, QList<FontItem*> > keyList;
	for ( int i=0; i < currentFonts.count();++i )
	{
		keyList[currentFonts[i]->value ( currentOrdering ) ].append ( currentFonts[i] );
	}

	QMap<QString, QList<FontItem*> >::const_iterator kit;
	for ( int i = 0x21 /* ! */; i <= 0x7e /* ~ */; ++i )
	{
		QChar firstChar ( i );
		QTreeWidgetItem *alpha = new QTreeWidgetItem ( m_lists->fontTree );
		alpha->setText ( 0, firstChar );
		alpha->setFont(0,alphaFont);
		alpha->setData ( 0,100,"alpha" );
		alpha->setBackgroundColor(0,Qt::lightGray);
		alpha->setBackgroundColor(1,Qt::lightGray);
		bool alphaIsUsed = false;

		for ( kit = keyList.begin(); kit != keyList.end(); ++kit )
		{
			bool isExpanded = false;
			if ( kit.key().at ( 0 ).toUpper() == firstChar )
			{
				QTreeWidgetItem *ord = new QTreeWidgetItem ( alpha );
				ord->setText ( 0, kit.key() );
				ord->setData ( 0,100,"family" );
				ord->setCheckState ( 0,Qt::Unchecked );
				bool chekno = false;
				bool checkyes = false;
				if ( openKeys.contains ( kit.key() ) )
				{
					ord->setExpanded ( true );
					isExpanded = true;
				}

// 				QMap<QString,int> variantMap;
				for ( int  n = 0; n < kit.value().count(); ++n )
				{
					QTreeWidgetItem *entry = new QTreeWidgetItem ( ord );
					QString variant = kit.value() [n]->variant();
// 					variantMap[variant] = n;
					entry->setText ( 0,  variant );
					entry->setText ( 1, kit.value() [n]->name() );
					entry->setData ( 0, 100, "fontfile" );
					

					if ( isExpanded )
					{
// 						QFont fakeFont;
// 						fakeFont.setPointSizeF(100);
// 						entry->setFont(2, fakeFont);
// 						entry->setText(2, "A");
// 						entry->setBackground ( 2,QBrush ( kit.value() [n]->oneLinePreviewPixmap() ) );
						
					}


					bool act = kit.value() [n]->isActivated();
					if ( act )
					{
						checkyes = true;
					}
					else
					{
						chekno = true;
					}
					entry->setCheckState ( 1, act ?  Qt::Checked : Qt::Unchecked );
					entry->setData ( 0,200,entry->checkState ( 1 ) );
					
					if ( entry->text ( 1 ) == curItemName )
						curItem = entry;
				}

				// try to give the most sensitive icon
// 				if ( variantMap.contains ( "Regular" ) )
// 					ord->setIcon ( 2,kit.value() [ variantMap["Regular"] ]->oneLinePreviewIcon ( "a" ) );
// 				else if ( variantMap.contains ( "Roman" ) )
// 					ord->setIcon ( 2,kit.value() [ variantMap["Roman"] ]->oneLinePreviewIcon ( "a" ) );
// 				else if ( variantMap.contains ( "Medium" ) )
// 					ord->setIcon ( 2,kit.value() [ variantMap["Medium"] ]->oneLinePreviewIcon ( "a" ) );
// 				else if ( variantMap.contains ( "Book" ) )
// 					ord->setIcon ( 2,kit.value() [ variantMap["Book"] ]->oneLinePreviewIcon ( "a" ) );
// 				else
// 					ord->setIcon ( 2,kit.value() [0]->oneLinePreviewIcon("a") );

				if ( checkyes && chekno )
					ord->setCheckState ( 0,Qt::PartiallyChecked );
				else if ( checkyes )
					ord->setCheckState ( 0,Qt::Checked );
				// track checkState
				ord->setData ( 0,200,ord->checkState ( 0 ) );
				ord->setText ( 1,QString::number ( ord->childCount() ) );


				alphaIsUsed = true;
			}
		}
		if ( alphaIsUsed )
		{
			m_lists->fontTree->addTopLevelItem ( alpha );
			alpha->setExpanded ( true );
		}
		else
		{
			delete alpha;
		}
	}
	
	m_lists->previewList->slotRefill(currentFonts, fontsetHasChanged);
	if ( curItem )
	{
// 		qDebug() << "get curitem : " << curItem->text ( 0 ) << curItem->text ( 1 );
		m_lists->fontTree->scrollToItem ( curItem, QAbstractItemView::PositionAtCenter );
		QColor scol(Qt::blue);
		scol.setAlpha(30);
		curItem->parent()->setBackgroundColor(0,scol);
		curItem->parent()->setBackgroundColor(1,scol);
		curItem->setBackgroundColor(0,scol);
		curItem->setBackgroundColor(1,scol);
		
		
// 		curItem->setSelected ( true );
	}
	else
	{
		qDebug() << "NO CURITEM";
	}
	m_lists->fontTree->resizeColumnToContents ( 0 )  ;
// 	m_lists->fontTree->resizeColumnToContents ( 1 ) ;
// 	m_lists->fontTree->setColumnWidth(0,200);
	
	fontsetHasChanged = false;
}

void MainViewWidget::slotItemOpened(QTreeWidgetItem * item)
{
// 	if(item->data(0,100).toString() == "family")
// 	{
// 		slotFontSelected (item, 0);
// 	}
// 	
}


void MainViewWidget::slotOrderingChanged ( QString s )
{
	//Update "m_lists->fontTree"


// 	currentFonts = typo->getAllFonts();
	currentOrdering = s;
	fillTree();

}

/// Should be renamed in slotNameItemSelected
void MainViewWidget::slotFontSelected ( QTreeWidgetItem * item, int column )
{
// 	qDebug() << "font select";	
	if ( item->data(0,100).toString() == "alpha" )
	{
// 		qDebug() << "Item is an alpha";
		return;
		fillTree();
	}
	
	if(item->data(0,100).toString() == "family")
	{
// 		qDebug() << "Item is a family";
		bool wantView = true;
		bool hasChild = false;
		QStringList names;
		QMap<QString, QString> variantMap;
		for(int i=0; i < item->childCount(); ++i)
		{
			hasChild = true;
			if(item->child(i)->text(1) == curItemName)
				wantView = false; 
			names << item->child(i)->text(1) ;
			variantMap[item->child(i)->text(0)] = item->child(i)->text(1) ;
		}
		slotFontActionByNames(names);
		
		if(wantView && hasChild)
		{
			lastIndex = faceIndex;

				if ( variantMap.contains ( "Regular" ) )
					faceIndex =  variantMap["Regular"];
				else if ( variantMap.contains ( "Roman" ) )
					faceIndex =  variantMap["Roman"];
				else if ( variantMap.contains ( "Medium" ) )
					faceIndex =  variantMap["Medium"];
				else if ( variantMap.contains ( "Book" ) )
					faceIndex =  variantMap["Book"];
				else
					faceIndex =  *(variantMap.begin());

// 			faceIndex = item->child(0)->text(1);
			curItemName = faceIndex;
	
			if ( faceIndex.count() && faceIndex != lastIndex )
			{
// 				slotFontActionByName( faceIndex );
				theVeryFont = typo->getFont ( faceIndex );
				fillOTTree();
				fillUniPlanesCombo(theVeryFont); 
				slotView(true);
				typo->setWindowTitle(theVeryFont->fancyName()+ " - Fontmatrix");
				typo->presentFontName(theVeryFont->fancyName());
				m_lists->previewList->searchAndSelect(theVeryFont->name());
			}
		}
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
			qDebug() << "Something wrong, Qt::PartiallyChecked should not be reached" ;
			
		}
		fillTree();
		return;
	}
	
	if(item->data(0,100).toString() == "fontfile")
	{
// 		qDebug() << "Item is a fontfile";
		lastIndex = faceIndex;
		faceIndex = item->text ( 1 );
		curItemName = faceIndex;
	
		if ( faceIndex.count() && faceIndex != lastIndex )
		{
// 			qDebug() << "Font has changed \n\tOLD : "<<lastIndex<<"\n\tNEW : " << faceIndex ;
			slotFontAction ( item,column );
// 			emit faceChanged();
			theVeryFont = typo->getFont ( faceIndex );
			fillOTTree();
			fillUniPlanesCombo(theVeryFont); // has to be called before view, may I should come back to the faceChanged signal idea
			slotView(true);
			typo->setWindowTitle(theVeryFont->fancyName() + " - Fontmatrix");
			typo->presentFontName(theVeryFont->fancyName());
			m_lists->previewList->searchAndSelect(theVeryFont->name());
		}
		if(item->data(0,200).toInt() != item->checkState(1))
		{
			if ( item->checkState ( 1 ) == Qt::Checked )
				slotActivate ( true, item, column );
			else
				slotActivate ( false, item, column );
		}
		fillTree();
		abcView->verticalScrollBar()->setValue(0);
		return;
	}
	return;

}

void MainViewWidget::slotFontSelectedByName(QString fname)
{
	qDebug() << "MainViewWidget::slotFontSelectedByName("<<fname<<")";
	if(fname.isEmpty())
		return;
	lastIndex = faceIndex;
	faceIndex = fname;
	curItemName = faceIndex;
	
	if ( faceIndex.count() && faceIndex != lastIndex )
	{
// 		qDebug() << "Font has changed \n\tOLD : "<<lastIndex<<"\n\tNEW : " << faceIndex ;
		slotFontActionByName(fname);
		theVeryFont = typo->getFont ( faceIndex );
		fillOTTree();
		fillUniPlanesCombo(theVeryFont); 
		slotView(true);
		typo->setWindowTitle(theVeryFont->fancyName()+ " - Fontmatrix");
		typo->presentFontName(theVeryFont->fancyName());
	}
	
	fillTree();
	abcView->verticalScrollBar()->setValue(0);
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
	if(needDeRendering )
	{
// 		qDebug() << "neeedDerender (faceIndex = "<< faceIndex <<")";
		if ( l )
		{
// 			qDebug() << "last to derender (lastindex = "<< lastIndex <<")";
			l->deRenderAll();
		}
// 		else
// 		{
// 			qDebug() << "NO last to derender (lastindex = "<< lastIndex <<")";
// 		}
		f->deRenderAll();
		
		curGlyph = 0;
	}
// 	else
// 	{
// 		qDebug() << "dontNeedDerender (faceIndex = "<< faceIndex <<")";
// 	}

// 	if(renderingLock == true)
// 		return;
// 	renderingLock = true;
	
	theVeryFont->setFTRaster(freetypeButton->isChecked());
	
	QApplication::setOverrideCursor ( Qt::WaitCursor );
	
	QString pkey = uniPlaneCombo->itemData( uniPlaneCombo->currentIndex() ).toString();
	QPair<int,int> uniPair(uniPlanes[pkey + uniPlaneCombo->currentText()]);
	int coverage = theVeryFont->countCoverage(uniPair.first, uniPair.second);
	int interval = uniPair.second - uniPair.first;
	coverage = coverage * 100 / interval;
	unicodeCoverageStat->setText( QString::number(coverage) + "\%");
	f->renderAll ( abcScene , uniPair.first, uniPair.second); 
	QApplication::restoreOverrideCursor();

	QStringList stl = typo->namedSample(sampleTextCombo->currentText()).split ( '\n' );
	QPointF pen ( 100,80 );
	QApplication::setOverrideCursor ( Qt::WaitCursor );
	for ( int i=0; i< stl.count(); ++i )
	{
		pen.ry() = 100 + sampleInterSize * i;
		bool processFeatures = f->isOpenType() &&  !deFillOTTree().isEmpty();
		if( processFeatures )
		{
			OTFSet aSet = deFillOTTree();
			qDebug() << aSet.dump();
			f->renderLine(aSet, loremScene,stl[i],pen, sampleFontSize );
		}
		else
		{
			f->renderLine ( loremScene,stl[i],pen, sampleFontSize );
		}
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
		loremView->fitInView(allrect, Qt::KeepAspectRatio);
	}

// 	renderingLock = false;

}

void MainViewWidget::slotglyphInfo()
{
	if ( abcScene->selectedItems().isEmpty() )
		return;
	if(curGlyph)
	{
		curGlyph->setBrush(QColor(255,255,255,0));
	}
	curGlyph = reinterpret_cast<QGraphicsRectItem*>(abcScene->selectedItems().first());
	curGlyph->setBrush(QColor(0,0,0,60));
// 	QGraphicsPathItem * gitem = theVeryFont->hasCodepoint(curGlyph->data(3).toInt());
// 	gitem->setBrush( QColor(200,200,200) );
	
// 	QTransform transform;
// 	transform.scale(2.0,2.0);
// 	abcView->setTransform(transform);
	if(abcView->transform().isIdentity())
	{
		abcView->fitInView(curGlyph->rect().toRect(), Qt::KeepAspectRatio);
		abcView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		abcView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
	else
	{
		abcView->setTransform(QTransform());
		abcView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		loremView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}
}

void MainViewWidget::slotSearch()
{
	m_lists->fontTree->clear();
	fontsetHasChanged = true;

	QString fs ( m_lists->searchString->text() );
	QString ff ( "search_%1" );
	QString sensitivity("INSENS");
	if(m_lists->sensitivityCheck->isChecked())
	{
		sensitivity = "SENS";
	}

	currentFonts = typo->getFonts ( fs,ff.arg(sensitivity) ) ;
	currentOrdering = "family";
	fillTree();
	m_lists->searchString->clear();
}

void MainViewWidget::slotFilterTag ( QString tag )
{
	m_lists->fontTree->clear();
	fontsetHasChanged = true;
	QString fs ( tag );
	QString ff ( "tag" );

	currentFonts = typo->getFonts ( fs,ff ) ;
	currentOrdering = "family";
	fillTree();
}

void MainViewWidget::slotFilterTagset ( QString set )
{
	m_lists->fontTree->clear();
	fontsetHasChanged = true;
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
		currentFaction = new FontActionWidget (/* typo->adaptator()*/ );
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

void MainViewWidget::slotFontActionByName(QString fname)
{
	if ( !currentFaction )
	{
		currentFaction = new FontActionWidget ( /*typo->adaptator()*/ );
		tagLayout->addWidget ( currentFaction );
		connect ( currentFaction,SIGNAL ( cleanMe() ),this,SLOT ( slotCleanFontAction() ) );
		connect ( currentFaction,SIGNAL ( tagAdded ( QString ) ),this,SLOT ( slotAppendTag ( QString ) ) );
		currentFaction->show();
	}

	FontItem * FoIt = typo->getFont ( fname);
	if ( FoIt/* && (!FoIt->isLocked())*/ )
	{
// 		currentFaction->slotFinalize();
		QList<FontItem*> fl;
		fl.append ( FoIt );
		currentFaction->prepare ( fl );


	}
}

void MainViewWidget::slotFontActionByNames(QStringList fnames)
{
	if ( !currentFaction )
	{
		currentFaction = new FontActionWidget ( /*typo->adaptator()*/ );
		tagLayout->addWidget ( currentFaction );
		connect ( currentFaction,SIGNAL ( cleanMe() ),this,SLOT ( slotCleanFontAction() ) );
		connect ( currentFaction,SIGNAL ( tagAdded ( QString ) ),this,SLOT ( slotAppendTag ( QString ) ) );
		currentFaction->show();
	}

	QList<FontItem*> FoIt;// = typo->getFont ( fname);
	for(int i= 0; i < fnames.count() ; ++i)
	{
		FoIt.append(typo->getFont(fnames[i]));
	}
	if ( FoIt.count() )
		currentFaction->prepare ( FoIt );
}


void MainViewWidget::slotEditAll()
{
	if ( !currentFaction )
	{
		currentFaction = new FontActionWidget (/* typo->adaptator()*/ );
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
// 	qDebug() << "add tag to combo " << tag;
	m_lists->tagsCombo->addItem ( tag );
	emit newTag(tag);
}

void MainViewWidget::activation ( FontItem* fit , bool act )
{
// 	qDebug() << "Activation of " << fit->name() << act;
	if ( act )
	{

		if ( !fit->isLocked() )
		{
			if ( !fit->isActivated() )
			{
				fit->setActivated(true);

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
// 					typo->adaptator()->private_signal ( 1, fofi.fileName() );
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
			if ( fit->isActivated() )
			{
				fit->setActivated(false);
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
// 					typo->adaptator()->private_signal ( 0, fofi.fileName() );
				}
			}

		}
		else
		{
			qDebug() << "\tIs Locked";
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

void MainViewWidget::slotSetSampleText(QString s)
{
	sampleText = s ;
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
	m_lists->tagsetCombo->clear();
	m_lists->tagsetCombo->addItems ( typo->tagsets() );
}

// void MainViewWidget::slotShowCodePoint()
// {
// // 	QString codetext = codepointSelectText->text();
// 	bool ok;
// // 	int codepoint = codetext.toInt ( &ok, 16 );
// 	if ( !ok )
// 		return;
// 	if ( !theVeryFont )
// 		return;
// // 	QGraphicsPathItem *pit = theVeryFont->hasCodepoint ( codepoint );
// 	if ( !pit )
// 		return;
// 
// 	abcView->fitInView ( pit, Qt::KeepAspectRatio );
// 
// 
// 
// }

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
		loremView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		loremView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}
	else
	{
		renderZoom->setDisabled(true);
		renderZoom->setStatusTip("zoom is disabled, uncheck fit to view to enable zoom");
		loremView->locker = true;
		loremView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		loremView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
	fontsetHasChanged = true;
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
	abcView->verticalScrollBar()->setValue(0);
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
	// TODO split planes into at least scripts
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
// 			qDebug() << p << codecount;
			uniPlaneCombo->addItem(p.mid(3), p.mid(0,3));
		}
	}
	uniPlaneCombo->setCurrentIndex(0);
	
}

void MainViewWidget::keyPressEvent(QKeyEvent * event)
{
// 	qDebug() << " MainViewWidget::keyPressEvent(QKeyEvent * "<<event<<")";
	if(event->key() == Qt::Key_Space &&  event->modifiers().testFlag ( Qt::ControlModifier ))
	{
		// Switch list view
		if(m_lists->fontlistTab->currentIndex() == 0)
			m_lists->fontlistTab->setCurrentIndex(1);
		else
			m_lists->fontlistTab->setCurrentIndex(0);
	}
}

void MainViewWidget::slotAdjustGlyphView(int width)
{
	if(!theVeryFont)
		return;
		
	theVeryFont->adjustGlyphsPerRow(width);
	slotView(true);
}

void MainViewWidget::fillOTTree()
{
	OpenTypeTree->clear();
	langCombo->setEnabled(false);
	QStringList scripts;
	if(theVeryFont && theVeryFont->isOpenType())
	{
		FmOtf * otf = theVeryFont->takeOTFInstance();
		foreach(QString table, otf->get_tables())
		{
			otf->set_table(table);
			QTreeWidgetItem *tab_item = new QTreeWidgetItem(OpenTypeTree,QStringList(table));
			tab_item->setExpanded(true);
			foreach(QString script, otf->get_scripts())
			{
				scripts << script;
				otf->set_script(script);
				QTreeWidgetItem *script_item = new QTreeWidgetItem(tab_item, QStringList(script));
				script_item->setExpanded(true);
				foreach(QString lang, otf->get_langs())
				{
					otf->set_lang(lang);
					QTreeWidgetItem *lang_item = new QTreeWidgetItem(script_item, QStringList(lang));
					lang_item->setExpanded(true);
					foreach(QString feature, otf->get_features())
					{
						QStringList f(feature);
						f << OTTagMeans(feature);
						QTreeWidgetItem *feature_item = new QTreeWidgetItem(lang_item, f);
						feature_item->setCheckState(0, Qt::Unchecked);
					}
				}
			}
		}
		OpenTypeTree->resizeColumnToContents ( 0 ) ;
		theVeryFont->releaseOTFInstance(otf);
	}
	scripts = scripts.toSet().toList();
// 	scripts.removeAll("latn");
	if(scripts.count() > 1)
	{
		langCombo->setEnabled(true);
		langCombo->addItems(scripts);
	}
}

OTFSet MainViewWidget::deFillOTTree()
{
// 	qDebug() << "MainViewWidget::deFillOTTree()";
	OTFSet ret;
// 	qDebug() << OpenTypeTree->topLevelItemCount();
	for(int table_index = 0; table_index < OpenTypeTree->topLevelItemCount(); ++table_index)//tables
	{
// 		qDebug() << "table_index = " << table_index;
		QTreeWidgetItem * table_item = OpenTypeTree->topLevelItem ( table_index ) ;
// 		qDebug() <<  table_item->text(0);
		for(int script_index = 0; script_index < table_item->childCount();++script_index)//scripts
		{
			QTreeWidgetItem * script_item = table_item->child(script_index);
// 			qDebug() << "\tscript_index = " <<  script_index << script_item->text(0);
			for(int lang_index = 0; lang_index < script_item->childCount(); ++lang_index)//langs
			{
				QTreeWidgetItem * lang_item = script_item->child(lang_index);
// 				qDebug() << "\t\tlang_index = "<< lang_index << lang_item->text(0);
				for(int feature_index = 0; feature_index < lang_item->childCount(); ++feature_index)//features
				{
// 					qDebug() << lang_item->childCount() <<" / "<<  feature_index;
					QTreeWidgetItem * feature_item = lang_item->child(feature_index);
// 					qDebug() << "\t\t\tfeature_item -> "<< feature_item->text(0);
					if(feature_item->checkState(0) == Qt::Checked)
					{
						if(table_item->text(0) == "GPOS")
						{
							ret.script = script_item->text(0);
							ret.lang = lang_item->text(0);
							ret.gpos_features.append(feature_item->text(0));
						}
						if(table_item->text(0) == "GSUB")
						{
							ret.script = script_item->text(0);
							ret.lang = lang_item->text(0);
							ret.gsub_features.append(feature_item->text(0));
						}
					}
				}
			}
		}
	}
// 	qDebug() << "endOf";
	return ret;
	
}

void MainViewWidget::slotFeatureChanged()
{
// 	OTFSet ret = deFillOTTree();
	slotView(true); 
}

void MainViewWidget::slotSampleChanged()
{
	slotView(true); 
}

void MainViewWidget::refillSampleList()
{
	sampleTextCombo->clear();
	sampleTextCombo->addItems(typo->namedSamplesNames());
}

void MainViewWidget::slotFTRasterChanged()
{
// 	if(theVeryFont)
// 	{
// 		theVeryFont->setFTRaster(freetypeButton->isChecked());
		slotView(true);
// 	}

}

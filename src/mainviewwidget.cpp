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
// #include "fontactionwidget.h"
// #include "typotekadaptator.h"
#include "fmpreviewlist.h"
#include "fmglyphsview.h"
#include "listdockwidget.h"
#include "fmotf.h"
#include "opentypetags.h"
#include "systray.h"


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
#include <QProgressDialog>
#include <QMenu>
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
	fontsetHasChanged = true;
	curGlyph = 0;
	fancyGlyphInUse = -1;
	fontInfoText->setSource ( QUrl ( "qrc:/texts/welcome" ) );
	fillUniPlanes();
	refillSampleList();

// 	tagLayout = new QGridLayout ( tagPage );
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
	loremView->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
	loremView->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

	sampleText= typo->namedSample ( "default" );
	sampleFontSize = 18;
	sampleInterSize = 20;
	m_lists->previewList->setRefWidget ( this );

	//CONNECT
	connect ( m_lists->tagsetCombo,SIGNAL ( activated ( const QString ) ),this,SLOT ( slotFilterTagset ( QString ) ) );
	connect ( m_lists->fontTree,SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ),this,SLOT ( slotFontSelected ( QTreeWidgetItem*, int ) ) );
	connect ( m_lists->searchButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotSearch() ) );
	connect ( m_lists->searchString,SIGNAL ( returnPressed() ),this,SLOT ( slotSearch() ) );
	connect ( m_lists->viewAllButton,SIGNAL ( released() ),this,SLOT ( slotViewAll() ) );
	connect ( m_lists->viewActivatedButton,SIGNAL ( released() ),this,SLOT ( slotViewActivated() ) );
	connect ( m_lists->fontTree,SIGNAL ( itemExpanded ( QTreeWidgetItem* ) ),this,SLOT ( slotItemOpened ( QTreeWidgetItem* ) ) );
	connect ( m_lists->tagsCombo,SIGNAL ( activated ( const QString& ) ),this,SLOT ( slotFilterTag ( QString ) ) );

	connect ( abcView,SIGNAL ( pleaseShowSelected() ),this,SLOT ( slotShowOneGlyph() ) );
	connect ( abcView,SIGNAL ( pleaseShowAll() ),this,SLOT ( slotShowAllGlyph() ) );
	connect ( renderZoom,SIGNAL ( valueChanged ( int ) ),this,SLOT ( slotZoom ( int ) ) );
	connect ( typo,SIGNAL ( tagAdded ( QString ) ),this,SLOT ( slotAppendTag ( QString ) ) );
	connect ( uniPlaneCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotPlaneSelected ( int ) ) );
	connect ( antiAliasButton,SIGNAL ( toggled ( bool ) ),this,SLOT ( slotSwitchAntiAlias ( bool ) ) );
	connect ( fitViewCheck,SIGNAL ( stateChanged ( int ) ),this,SLOT ( slotFitChanged ( int ) ) );
	connect ( loremView, SIGNAL ( refit() ),this,SLOT ( slotRefitSample() ) );
	connect ( abcView,SIGNAL ( refit ( int ) ),this,SLOT ( slotAdjustGlyphView ( int ) ) );
	connect ( OpenTypeTree, SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ), this, SLOT ( slotFeatureChanged() ) );
	connect ( langCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotChangeScript() ) );
	connect ( rtlCheck,SIGNAL ( stateChanged ( int ) ),this,SLOT ( slotSwitchRTL() ) );
	connect ( useShaperCheck,SIGNAL ( stateChanged ( int ) ),this,SLOT ( slotWantShape() ) );
	connect ( sampleTextCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotSampleChanged() ) );
	connect ( freetypeButton,SIGNAL ( released() ),this,SLOT ( slotFTRasterChanged() ) );
	connect ( abcView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateGView()));
	connect ( loremView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateSView()));
	
	connect ( tagsListWidget,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
	connect ( tagsListWidget,SIGNAL ( itemClicked ( QListWidgetItem* ) ),this,SLOT ( slotSwitchCheckState ( QListWidgetItem* ) ) );
	connect ( newTagButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotNewTag() ) );
// 	connect ( this ,SIGNAL ( cleanMe() ),this,SLOT ( slotCleanFontAction() ) );
	connect ( this ,SIGNAL ( tagAdded ( QString ) ),this,SLOT ( slotAppendTag ( QString ) ) );

	connect ( this,SIGNAL ( activationEvent ( QString ) ),typo->getSystray(),SLOT ( updateTagMenu ( QString ) ) );
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

	QFont alphaFont ( "helvetica",14,QFont::Bold,false );

	m_lists->fontTree->clear();
	QMap<QString, QList<FontItem*> > keyList;
	QList<int> initChars;
	for ( int i=0; i < currentFonts.count();++i )
	{
		keyList[currentFonts[i]->value ( currentOrdering ) ].append ( currentFonts[i] );
		initChars << currentFonts[i]->family()[0].unicode();
	}
	initChars = initChars.toSet().toList();
	
	QMap<QString, QList<FontItem*> >::const_iterator kit;
	for ( int i = 0 ; i < initChars.count() ; ++i )
	{
		QChar firstChar ( initChars[i] );
// 		qDebug() << "First char is " <<firstChar;
		QTreeWidgetItem *alpha = new QTreeWidgetItem ( m_lists->fontTree );
		alpha->setText ( 0, firstChar );
		alpha->setFont ( 0,alphaFont );
		alpha->setData ( 0,100,"alpha" );
		alpha->setBackgroundColor ( 0,Qt::lightGray );
		alpha->setBackgroundColor ( 1,Qt::lightGray );
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

	m_lists->previewList->slotRefill ( currentFonts, fontsetHasChanged );
	if ( curItem )
	{
// 		qDebug() << "get curitem : " << curItem->text ( 0 ) << curItem->text ( 1 );
		m_lists->fontTree->scrollToItem ( curItem, QAbstractItemView::PositionAtCenter );
		QColor scol ( Qt::blue );
		scol.setAlpha ( 30 );
		curItem->parent()->setBackgroundColor ( 0,scol );
		curItem->parent()->setBackgroundColor ( 1,scol );
		curItem->setBackgroundColor ( 0,scol );
		curItem->setBackgroundColor ( 1,scol );


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

void MainViewWidget::slotItemOpened ( QTreeWidgetItem * item )
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
	if ( item->data ( 0,100 ).toString() == "alpha" )
	{
// 		qDebug() << "Item is an alpha";
		return;
		fillTree();
	}

	if ( item->data ( 0,100 ).toString() == "family" )
	{
		qDebug() << "Item is a family";
		bool wantView = true;
		bool hasChild = false;
		QStringList names;
		QMap<QString, QString> variantMap;
		for ( int i=0; i < item->childCount(); ++i )
		{
			hasChild = true;
			if ( item->child ( i )->text ( 1 ) == curItemName )
				wantView = false;
			names << item->child ( i )->text ( 1 ) ;
			variantMap[item->child ( i )->text ( 0 ) ] = item->child ( i )->text ( 1 ) ;
		}
		slotFontActionByNames ( names );

		if ( wantView && hasChild )
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
				faceIndex =  * ( variantMap.begin() );

// 			faceIndex = item->child(0)->text(1);
			curItemName = faceIndex;

			if ( faceIndex.count() && faceIndex != lastIndex )
			{
				if(abcView->state() == FMGlyphsView::SingleView)
					slotShowAllGlyph();
// 				slotFontActionByName( faceIndex );
				theVeryFont = typo->getFont ( faceIndex );
				fillOTTree();
				fillUniPlanesCombo ( theVeryFont );
				slotView ( true );
				typo->setWindowTitle ( theVeryFont->fancyName() + " - Fontmatrix" );
				typo->presentFontName ( theVeryFont->fancyName() );
				m_lists->previewList->searchAndSelect ( theVeryFont->name() );
			}
		}
		qDebug() << curItemName;
		int oldc = item->data ( 0,200 ).toInt();
		if ( oldc == item->checkState ( 0 ) )
		{
			fillTree();
			return;

		}
		if ( item->checkState ( 0 ) != Qt::PartiallyChecked )
		{

			bool cs = item->checkState ( 0 ) == Qt::Checked ? true : false;

			QList<FontItem*> todo;
			for ( int i=0; i<item->childCount(); ++i )
			{
				todo << typo->getFont ( item->child ( i )->text ( 1 ) );
			}
			foreach ( FontItem* afont, todo )
			{
				activation ( afont, cs );
			}

		}
		else
		{
			qDebug() << "Something wrong, Qt::PartiallyChecked should not be reached" ;

		}
		fillTree();
		return;
	}

	if ( item->data ( 0,100 ).toString() == "fontfile" )
	{
// 		qDebug() << "Item is a fontfile";
		lastIndex = faceIndex;
		faceIndex = item->text ( 1 );
		curItemName = faceIndex;

		if ( faceIndex.count() && faceIndex != lastIndex )
		{
			if(abcView->state() == FMGlyphsView::SingleView)
				slotShowAllGlyph();
// 			qDebug() << "Font has changed \n\tOLD : "<<lastIndex<<"\n\tNEW : " << faceIndex ;
			slotFontAction ( item,column );
// 			emit faceChanged();
			theVeryFont = typo->getFont ( faceIndex );
			fillOTTree();
			fillUniPlanesCombo ( theVeryFont ); // has to be called before view, may I should come back to the faceChanged signal idea
			slotView ( true );
			typo->setWindowTitle ( theVeryFont->fancyName() + " - Fontmatrix" );
			typo->presentFontName ( theVeryFont->fancyName() );
			m_lists->previewList->searchAndSelect ( theVeryFont->name() );
		}
		if ( item->data ( 0,200 ).toInt() != item->checkState ( 1 ) )
		{
			if ( item->checkState ( 1 ) == Qt::Checked )
				slotActivate ( true, item, column );
			else
				slotActivate ( false, item, column );
		}
		fillTree();
		abcView->verticalScrollBar()->setValue ( 0 );
		return;
	}
	return;

}

void MainViewWidget::slotFontSelectedByName ( QString fname )
{
	qDebug() << "MainViewWidget::slotFontSelectedByName("<<fname<<")";
	if ( fname.isEmpty() || fname ==  faceIndex )
		return;
	lastIndex = faceIndex;
	faceIndex = fname;
	curItemName = faceIndex;
	
	if ( faceIndex.count() && faceIndex != lastIndex )
	{
		qDebug() << "Font has changed \n\tOLD : "<<lastIndex<<"\n\tNEW : " << faceIndex ;
		if(abcView->state() == FMGlyphsView::SingleView)
			slotShowAllGlyph();
		slotFontActionByName ( fname );
		theVeryFont = typo->getFont ( faceIndex );
		fillOTTree();
		fillUniPlanesCombo ( theVeryFont );
		slotView ( true );
		typo->setWindowTitle ( theVeryFont->fancyName() + " - Fontmatrix" );
		typo->presentFontName ( theVeryFont->fancyName() );
		fillTree();
		abcView->verticalScrollBar()->setValue ( 0 );
	}
	
	
}


void MainViewWidget::slotInfoFont()
{
	FontItem *f = typo->getFont ( faceIndex );
	fontInfoText->clear();
	//QString t(QString("Family : %1\nStyle : %2\nFlags : \n%3").arg(f->family()).arg(f->variant()).arg(f->faceFlags()));
	fontInfoText->setText ( f->infoText() );

}

void MainViewWidget::slotView ( bool needDeRendering )
{
	FontItem *l = typo->getFont ( lastIndex );
	FontItem *f = typo->getFont ( faceIndex );
	if ( !f )
		return;
	if ( needDeRendering )
	{
		if ( l )
		{
			l->deRenderAll();
		}
		f->deRenderAll();

		curGlyph = 0;
	}

	theVeryFont->setRTL ( rtlCheck->isChecked() );
	theVeryFont->setFTRaster ( freetypeButton->isChecked() );

	QString pkey = uniPlaneCombo->itemData ( uniPlaneCombo->currentIndex() ).toString();
	QPair<int,int> uniPair ( uniPlanes[pkey + uniPlaneCombo->currentText() ] );
	int coverage = theVeryFont->countCoverage ( uniPair.first, uniPair.second );
	int interval = uniPair.second - uniPair.first;
	coverage = coverage * 100 / ( interval + 1 );// against /0 exception
	unicodeCoverageStat->setText ( QString::number ( coverage ) + "\%" );

	if ( abcView->isVisible() )
	{
		// Used to be slow,old times
// 		QApplication::setOverrideCursor ( Qt::WaitCursor );
		f->renderAll ( abcScene , uniPair.first, uniPair.second );
// 		QApplication::restoreOverrideCursor();
	}

	if ( loremView->isVisible() )
	{
		QStringList stl = typo->namedSample ( sampleTextCombo->currentText() ).split ( '\n' );
		QPointF pen ( ( rtlCheck->isChecked() ) ? 500 : 100,80 );
		bool processFeatures = f->isOpenType() &&  !deFillOTTree().isEmpty();
		QString script = langCombo->currentText();
		bool processScript =  f->isOpenType() && ( useShaperCheck->checkState() == Qt::Checked ) && ( !script.isEmpty() );

		QApplication::setOverrideCursor ( Qt::WaitCursor );
		for ( int i=0; i< stl.count(); ++i )
		{
			pen.ry() = 100 + sampleInterSize * i;
			if ( processScript )
			{
				qDebug() << "render " << stl[i] << " as " << script;
				f->renderLine ( script ,loremScene,stl[i],pen, sampleFontSize );
			}
			else if ( processFeatures )
			{
				OTFSet aSet = deFillOTTree();
				qDebug() << aSet.dump();
				f->renderLine ( aSet, loremScene,stl[i],pen, sampleFontSize );
			}
			else
			{
				f->renderLine ( loremScene,stl[i],pen, sampleFontSize );
			}
		}
		QApplication::restoreOverrideCursor();

		if ( fitViewCheck->isChecked() )
		{
			QRectF allrect, firstrect;
			bool first = true;
			QList<QGraphicsItem*> lit = loremScene->items();
			for ( int i = 0 ; i <lit.count() ; ++i )
			{
				if ( lit[i]->data ( 1 ).toString() == "glyph" )
				{
					if ( first )
					{
						firstrect = lit[i]->sceneBoundingRect();
						first = false;

					}
					if ( lit[i]->sceneBoundingRect().bottomRight().y() > allrect.bottomRight().y() )
						allrect = allrect.united ( lit[i]->sceneBoundingRect() );
					if ( lit[i]->sceneBoundingRect().bottomRight().x() > allrect.bottomRight().x() )
						allrect = allrect.united ( lit[i]->sceneBoundingRect() );

				}


			}
			loremView->fitInView ( allrect, Qt::KeepAspectRatio );
		}
	}

	slotInfoFont();
// 	renderingLock = false;

}


void MainViewWidget::slotSearch()
{
	m_lists->fontTree->clear();
	fontsetHasChanged = true;

	QString fs ( m_lists->searchString->text() );
	QString ff ( "search_%1" );
	QString sensitivity ( "INSENS" );
	if ( m_lists->sensitivityCheck->isChecked() )
	{
		sensitivity = "SENS";
	}

	currentFonts = typo->getFonts ( fs,ff.arg ( sensitivity ) ) ;
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

	for ( int i = 0;i < tags.count(); ++i )
	{
		currentFonts += typo->getFonts ( tags[i],"tag" );
	}
	int count_req = tags.count();
	QSet<FontItem*> setOfTags ( currentFonts.toSet() );
	foreach ( FontItem * it, setOfTags )
	{
// 		qDebug() << it->name();
		if ( currentFonts.count ( it ) != count_req )
			currentFonts.removeAll ( it );
		else
			qDebug() << count_req<<currentFonts.count ( it );
	}



	currentOrdering = "family";
	currentFonts = currentFonts.toSet().toList();
	fillTree();
}


void MainViewWidget::slotFontAction ( QTreeWidgetItem * item, int column )
{
	if ( column >2 ) return;

	FontItem * FoIt = typo->getFont ( item->text ( 1 ) );
	if ( FoIt/* && (!FoIt->isLocked())*/ )
	{
		QList<FontItem*> fl;
		fl.append ( FoIt );
		prepare ( fl );


	}
}

void MainViewWidget::slotFontActionByName ( QString fname )
{
	FontItem * FoIt = typo->getFont ( fname );
	if ( FoIt/* && (!FoIt->isLocked())*/ )
	{
		QList<FontItem*> fl;
		fl.append ( FoIt );
		prepare ( fl );


	}
}

void MainViewWidget::slotFontActionByNames ( QStringList fnames )
{

	QList<FontItem*> FoIt;
	for ( int i= 0; i < fnames.count() ; ++i )
	{
		FoIt.append ( typo->getFont ( fnames[i] ) );
	}
	if ( FoIt.count() )
		prepare ( FoIt );
}


void MainViewWidget::slotEditAll()
{
	QList<FontItem*> fl;
	for ( int i =0; i< currentFonts.count(); ++i )
	{
		fl.append ( currentFonts[i] );
	}
	if ( fl.isEmpty() )
		return;

	prepare ( fl );
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
	emit newTag ( tag );
}

void MainViewWidget::activation ( FontItem* fit , bool act , bool updateTree )
{
// 	qDebug() << "Activation of " << fit->name() << act;
	if ( act )
	{

		if ( !fit->isLocked() )
		{
			if ( !fit->isActivated() )
			{
				fit->setActivated ( true );

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
				fit->setActivated ( false );
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
	if ( updateTree )
		fillTree();
	emit activationEvent ( fit->name() );
// 	typo->save();
}


void MainViewWidget::allActivation ( bool act )
{

	QProgressDialog progress(tr("Activation event"),tr("Cancel"),1,currentFonts.count(),this);
	progress.setWindowModality ( Qt::WindowModal );
	progress.setAutoReset(false);
	QString activString = act ? tr("Activation of :") : tr("Deactivation of :");
	int i =1;
	foreach ( FontItem* fit, currentFonts )
	{
		activation ( fit,act, false );// false here prevents to refill TreeView eachtime.
		progress.setLabelText ( activString + " " +( fit->name() ) );
		progress.setValue ( ++i );
		if ( progress.wasCanceled() )
			break;
	}
	fillTree();
}

void MainViewWidget::slotDesactivateAll()
{
	allActivation ( false );
}

void MainViewWidget::slotActivateAll()
{
	allActivation ( true );
}

void MainViewWidget::slotSetSampleText ( QString s )
{
	sampleText = s ;
	slotView ( true );

}

void MainViewWidget::slotActivate ( bool act, QTreeWidgetItem * item, int column )
{
	if ( column >2 ) return;
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

void MainViewWidget::slotFitChanged ( int i )
{
	if ( i == Qt::Unchecked )
	{
		renderZoom->setDisabled ( false );
		renderZoom->setStatusTip ( tr("zoom is enabled") );
		loremView->setTransform ( QTransform ( 1,0,0,1,0,0 ),false );
		loremView->locker = false;
		loremView->setVerticalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
		loremView->setHorizontalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
	}
	else
	{
		renderZoom->setDisabled ( true );
		renderZoom->setStatusTip ( tr("zoom is disabled, uncheck fit to view to enable zoom") );
		loremView->locker = true;
		loremView->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
		loremView->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
	}
	slotView();
}

void MainViewWidget::slotRefitSample()
{
	if ( fitViewCheck->isChecked() )
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


void MainViewWidget::fillUniPlanes()
{
// 	uniPlanes[ "Basic Multilingual Plane (BMP)"] = qMakePair(0x0000,0xFFFF) ;
	// BMP is huge, we split it into langs
	uniPlanes[ tr( "000Basic Latin" ) ] = qMakePair ( 0x0000,0x007F );
	uniPlanes[ tr( "001Latin-1 Supplement" ) ] = qMakePair ( 0x0080,0x00FF );
	uniPlanes[ tr( "002Latin Extended-A" ) ] = qMakePair ( 0x0100,0x017F );
	uniPlanes[ tr( "003Latin Extended-B" ) ] = qMakePair ( 0x0180,0x024F );
	uniPlanes[ tr( "004IPA Extensions" ) ] = qMakePair ( 0x0250,0x02AF );
	uniPlanes[ tr( "005Spacing Modifier Letters" ) ] = qMakePair ( 0x02B0,0x02FF );
	uniPlanes[ tr( "006Combining Diacritical Marks" ) ] = qMakePair ( 0x0300,0x036F );
	uniPlanes[ tr( "007Greek and Coptic" ) ] = qMakePair ( 0x0370,0x03FF );
	uniPlanes[ tr( "008Cyrillic" ) ] = qMakePair ( 0x0400,0x04FF );
	uniPlanes[ tr( "009Cyrillic Supplement" ) ] = qMakePair ( 0x0500,0x052F );
	uniPlanes[ tr( "010Armenian" ) ] = qMakePair ( 0x0530,0x058F );
	uniPlanes[ tr( "011Hebrew" ) ] = qMakePair ( 0x0590,0x05FF );
	uniPlanes[ tr( "012Arabic" ) ] = qMakePair ( 0x0600,0x06FF );
	uniPlanes[ tr( "013Syriac" ) ] = qMakePair ( 0x0700,0x074F );
	uniPlanes[ tr( "014Arabic Supplement" ) ] = qMakePair ( 0x0750,0x077F );
	uniPlanes[ tr( "015Thaana" ) ] = qMakePair ( 0x0780,0x07BF );
	uniPlanes[ tr( "016N'Ko" ) ] = qMakePair ( 0x07C0,0x07FF );
	uniPlanes[ tr( "017Devanagari" ) ] = qMakePair ( 0x0900,0x097F );
	uniPlanes[ tr( "018Bengali" ) ] = qMakePair ( 0x0980,0x09FF );
	uniPlanes[ tr( "019Gurmukhi" ) ] = qMakePair ( 0x0A00,0x0A7F );
	uniPlanes[ tr( "020Gujarati" ) ] = qMakePair ( 0x0A80,0x0AFF );
	uniPlanes[ tr( "021Oriya" ) ] = qMakePair ( 0x0B00,0x0B7F );
	uniPlanes[ tr( "022Tamil" ) ] = qMakePair ( 0x0B80,0x0BFF );
	uniPlanes[ tr( "023Telugu" ) ] = qMakePair ( 0x0C00,0x0C7F );
	uniPlanes[ tr( "024Kannada" ) ] = qMakePair ( 0x0C80,0x0CFF );
	uniPlanes[ tr( "025Malayalam" ) ] = qMakePair ( 0x0D00,0x0D7F );
	uniPlanes[ tr( "026Sinhala" ) ] = qMakePair ( 0x0D80,0x0DFF );
	uniPlanes[ tr( "027Thai" ) ] = qMakePair ( 0x0E00,0x0E7F );
	uniPlanes[ tr( "028Lao" ) ] = qMakePair ( 0x0E80,0x0EFF );
	uniPlanes[ tr( "029Tibetan" ) ] = qMakePair ( 0x0F00,0x0FFF );
	uniPlanes[ tr( "030Burmese" ) ] = qMakePair ( 0x1000,0x109F );
	uniPlanes[ tr( "031Georgian" ) ] = qMakePair ( 0x10A0,0x10FF );
	uniPlanes[ tr( "032Hangul Jamo" ) ] = qMakePair ( 0x1100,0x11FF );
	uniPlanes[ tr( "033Ethiopic" ) ] = qMakePair ( 0x1200,0x137F );
	uniPlanes[ tr( "034Ethiopic Supplement" ) ] = qMakePair ( 0x1380,0x139F );
	uniPlanes[ tr( "035Cherokee" ) ] = qMakePair ( 0x13A0,0x13FF );
	uniPlanes[ tr( "036Unified Canadian Aboriginal Syllabics" ) ] = qMakePair ( 0x1400,0x167F );
	uniPlanes[ tr( "037Ogham" ) ] = qMakePair ( 0x1680,0x169F );
	uniPlanes[ tr( "038Runic" ) ] = qMakePair ( 0x16A0,0x16FF );
	uniPlanes[ tr( "039Tagalog" ) ] = qMakePair ( 0x1700,0x171F );
	uniPlanes[ tr( "040Hanunóo" ) ] = qMakePair ( 0x1720,0x173F );
	uniPlanes[ tr( "041Buhid" ) ] = qMakePair ( 0x1740,0x175F );
	uniPlanes[ tr( "042Tagbanwa" ) ] = qMakePair ( 0x1760,0x177F );
	uniPlanes[ tr( "043Khmer" ) ] = qMakePair ( 0x1780,0x17FF );
	uniPlanes[ tr( "044Mongolian" ) ] = qMakePair ( 0x1800,0x18AF );
	uniPlanes[ tr( "045Limbu" ) ] = qMakePair ( 0x1900,0x194F );
	uniPlanes[ tr( "046Tai Le" ) ] = qMakePair ( 0x1950,0x197F );
	uniPlanes[ tr( "047New Tai Lue" ) ] = qMakePair ( 0x1980,0x19DF );
	uniPlanes[ tr( "048Khmer Symbols" ) ] = qMakePair ( 0x19E0,0x19FF );
	uniPlanes[ tr( "049Buginese" ) ] = qMakePair ( 0x1A00,0x1A1F );
	uniPlanes[ tr( "050Balinese" ) ] = qMakePair ( 0x1B00,0x1B7F );
	uniPlanes[ tr( "051Lepcha" ) ] =  qMakePair ( 0x1C00,0x1C4F );
	uniPlanes[ tr( "052Phonetic Extensions" ) ] = qMakePair ( 0x1D00,0x1D7F );
	uniPlanes[ tr( "053Phonetic Extensions Supplement" ) ] = qMakePair ( 0x1D80,0x1DBF );
	uniPlanes[ tr( "054Combining Diacritical Marks Supplement" ) ] = qMakePair ( 0x1DC0,0x1DFF );
	uniPlanes[ tr( "055Latin Extended Additional" ) ] = qMakePair ( 0x1E00,0x1EFF );
	uniPlanes[ tr( "056Greek Extended" ) ] = qMakePair ( 0x1F00,0x1FFF );
	uniPlanes[ tr( "057General Punctuation" ) ] = qMakePair ( 0x2000,0x206F );
	uniPlanes[ tr( "058Superscripts and Subscripts" ) ] = qMakePair ( 0x2070,0x209F );
	uniPlanes[ tr( "059Currency Symbols" ) ] = qMakePair ( 0x20A0,0x20CF );
	uniPlanes[ tr( "060Combining Diacritical Marks for Symbols" ) ] = qMakePair ( 0x20D0,0x20FF );
	uniPlanes[ tr( "061Letterlike Symbols" ) ] = qMakePair ( 0x2100,0x214F );
	uniPlanes[ tr( "062Number Forms" ) ] = qMakePair ( 0x2150,0x218F );
	uniPlanes[ tr( "063Arrows" ) ] = qMakePair ( 0x2190,0x21FF );
	uniPlanes[ tr( "064Mathematical Operators" ) ] = qMakePair ( 0x2200,0x22FF );
	uniPlanes[ tr( "065Miscellaneous Technical" ) ] = qMakePair ( 0x2300,0x23FF );
	uniPlanes[ tr( "066Control Pictures" ) ] = qMakePair ( 0x2400,0x243F );
	uniPlanes[ tr( "067Optical Character Recognition" ) ] = qMakePair ( 0x2440,0x245F );
	uniPlanes[ tr( "068Enclosed Alphanumerics" ) ] = qMakePair ( 0x2460,0x24FF );
	uniPlanes[ tr( "069Box Drawing" ) ] = qMakePair ( 0x2500,0x257F );
	uniPlanes[ tr( "070Block Elements" ) ] = qMakePair ( 0x2580,0x259F );
	uniPlanes[ tr( "071Geometric Shapes" ) ] = qMakePair ( 0x25A0,0x25FF );
	uniPlanes[ tr( "072Miscellaneous Symbols" ) ] = qMakePair ( 0x2600,0x26FF );
	uniPlanes[ tr( "073Dingbats" ) ] = qMakePair ( 0x2700,0x27BF );
	uniPlanes[ tr( "074Miscellaneous Mathematical Symbols-A" ) ] = qMakePair ( 0x27C0,0x27EF );
	uniPlanes[ tr( "075Supplemental Arrows-A" ) ] = qMakePair ( 0x27F0,0x27FF );
	uniPlanes[ tr( "076Braille Patterns" ) ] = qMakePair ( 0x2800,0x28FF );
	uniPlanes[ tr( "077Supplemental Arrows-B" ) ] = qMakePair ( 0x2900,0x297F );
	uniPlanes[ tr( "078Miscellaneous Mathematical Symbols-B" ) ] = qMakePair ( 0x2980,0x29FF );
	uniPlanes[ tr( "079Supplemental Mathematical Operators" ) ] = qMakePair ( 0x2A00,0x2AFF );
	uniPlanes[ tr( "080Miscellaneous Symbols and Arrows" ) ] = qMakePair ( 0x2B00,0x2BFF );
	uniPlanes[ tr( "081Glagolitic" ) ] = qMakePair ( 0x2C00,0x2C5F );
	uniPlanes[ tr( "082Latin Extended-C" ) ] = qMakePair ( 0x2C60,0x2C7F );
	uniPlanes[ tr( "083Coptic" ) ] = qMakePair ( 0x2C80,0x2CFF );
	uniPlanes[ tr( "084Georgian Supplement" ) ] = qMakePair ( 0x2D00,0x2D2F );
	uniPlanes[ tr( "085Tifinagh" ) ] = qMakePair ( 0x2D30,0x2D7F );
	uniPlanes[ tr( "086Ethiopic Extended" ) ] = qMakePair ( 0x2D80,0x2DDF );
	uniPlanes[ tr( "087Supplemental Punctuation" ) ] = qMakePair ( 0x2E00,0x2E7F );
	uniPlanes[ tr( "088CJK Radicals Supplement" ) ] = qMakePair ( 0x2E80,0x2EFF );
	uniPlanes[ tr( "089Kangxi Radicals" ) ] = qMakePair ( 0x2F00,0x2FDF );
	uniPlanes[ tr( "090Ideographic Description Characters" ) ] = qMakePair ( 0x2FF0,0x2FFF );
	uniPlanes[ tr( "091CJK Symbols and Punctuation" ) ] = qMakePair ( 0x3000,0x303F );
	uniPlanes[ tr( "092Hiragana" ) ] = qMakePair ( 0x3040,0x309F );
	uniPlanes[ tr( "093Katakana" ) ] = qMakePair ( 0x30A0,0x30FF );
	uniPlanes[ tr( "094Bopomofo" ) ] = qMakePair ( 0x3100,0x312F );
	uniPlanes[ tr( "095Hangul Compatibility Jamo" ) ] = qMakePair ( 0x3130,0x318F );
	uniPlanes[ tr( "096Kanbun" ) ] = qMakePair ( 0x3190,0x319F );
	uniPlanes[ tr( "097Bopomofo Extended" ) ] = qMakePair ( 0x31A0,0x31BF );
	uniPlanes[ tr( "098CJK Strokes" ) ] = qMakePair ( 0x31C0,0x31EF );
	uniPlanes[ tr( "099Katakana Phonetic Extensions" ) ] = qMakePair ( 0x31F0,0x31FF );
	uniPlanes[ tr( "100Enclosed CJK Letters and Months" ) ] = qMakePair ( 0x3200,0x32FF );
	uniPlanes[ tr( "101CJK Compatibility" ) ] = qMakePair ( 0x3300,0x33FF );
	uniPlanes[ tr( "102CJK Unified Ideographs Extension A" ) ] = qMakePair ( 0x3400,0x4DBF );
	uniPlanes[ tr( "103Yijing Hexagram Symbols" ) ] = qMakePair ( 0x4DC0,0x4DFF );
	uniPlanes[ tr( "104CJK Unified Ideographs" ) ] = qMakePair ( 0x4E00,0x9FFF );
	uniPlanes[ tr( "105Yi Syllables" ) ] = qMakePair ( 0xA000,0xA48F );
	uniPlanes[ tr( "106Yi Radicals" ) ] = qMakePair ( 0xA490,0xA4CF );
	uniPlanes[ tr( "107Modifier Tone Letters" ) ] = qMakePair ( 0xA700,0xA71F );
	uniPlanes[ tr( "108Latin Extended-D" ) ] = qMakePair ( 0xA720,0xA7FF );
	uniPlanes[ tr( "109Syloti Nagri" ) ] = qMakePair ( 0xA800,0xA82F );
	uniPlanes[ tr( "110Phags-pa" ) ] = qMakePair ( 0xA840,0xA87F );
	uniPlanes[ tr( "111Hangul Syllables" ) ] = qMakePair ( 0xAC00,0xD7AF );
	uniPlanes[ tr( "112High Surrogates" ) ] = qMakePair ( 0xD800,0xDB7F );
	uniPlanes[ tr( "113High Private Use Surrogates" ) ] = qMakePair ( 0xDB80,0xDBFF );
	uniPlanes[ tr( "114Low Surrogates" ) ] = qMakePair ( 0xDC00,0xDFFF );
	uniPlanes[ tr( "115Private Use Area" ) ] = qMakePair ( 0xE000,0xF8FF );
	uniPlanes[ tr( "116CJK Compatibility Ideographs" ) ] = qMakePair ( 0xF900,0xFAFF );
	uniPlanes[ tr( "117Alphabetic Presentation Forms" ) ] = qMakePair ( 0xFB00,0xFB4F );
	uniPlanes[ tr( "118Arabic Presentation Forms-A" ) ] = qMakePair ( 0xFB50,0xFDFF );
	uniPlanes[ tr( "119Variation Selectors" ) ] = qMakePair ( 0xFE00,0xFE0F );
	uniPlanes[ tr( "120Vertical Forms" ) ] = qMakePair ( 0xFE10,0xFE1F );
	uniPlanes[ tr( "121Combining Half Marks" ) ] = qMakePair ( 0xFE20,0xFE2F );
	uniPlanes[ tr( "122CJK Compatibility Forms" ) ] = qMakePair ( 0xFE30,0xFE4F );
	uniPlanes[ tr( "123Small Form Variants" ) ] = qMakePair ( 0xFE50,0xFE6F );
	uniPlanes[ tr( "124Arabic Presentation Forms-B" ) ] = qMakePair ( 0xFE70,0xFEFF );
	uniPlanes[ tr( "125Halfwidth and Fullwidth Forms" ) ] = qMakePair ( 0xFF00,0xFFEF );
	uniPlanes[ tr( "126Specials" ) ] = qMakePair ( 0xFFF0,0xFFFF );
	// TODO split planes into at least scripts
	uniPlanes[ tr( "127Supplementary Multilingual Plane (SMP)" ) ] = qMakePair ( 0x10000,0x1FFFF ) ;
	uniPlanes[ tr( "128Supplementary Ideographic Plane (SIP)" ) ] = qMakePair ( 0x20000,0x2FFFF ) ;
	uniPlanes[ tr( "129unassigned" ) ] = qMakePair ( 0x30000,0xDFFFF ) ;
	uniPlanes[ tr( "130Supplementary Special-purpose Plane (SSP)" ) ] = qMakePair ( 0xE0000,0xEFFFF ) ;
	uniPlanes[ tr( "131Private Use Area 1 (PUA)" ) ] = qMakePair ( 0xF0000,0xFFFFF ) ;
	uniPlanes[ tr( "132Private Use Area 2 (PUA)" ) ] = qMakePair ( 0x100000,0x10FFFF ) ;
	uniPlanes[ tr( "133Un-Mapped Glyphs" ) ] = qMakePair ( -1,100 ) ;
	uniPlanes[ tr( "134View all mapped glyphs" ) ] = qMakePair ( 0, 0x10FFFF ) ;

}

void MainViewWidget::fillUniPlanesCombo ( FontItem* item )
{
	uniPlaneCombo->clear();
	QStringList plist= uniPlanes.keys();
	for ( int i= 0;i<plist.count();++i )
	{
		QString p=plist.at ( i );
		if ( p.isEmpty() )
			continue;
		int begin = uniPlanes[p].first;
		int end = uniPlanes[p].second;
		int codecount = item->countCoverage ( begin,end );
		if ( codecount > 0 )
		{
// 			qDebug() << p << codecount;
			uniPlaneCombo->addItem ( p.mid ( 3 ), p.mid ( 0,3 ) );
		}
	}
	uniPlaneCombo->setCurrentIndex ( 0 );

}

void MainViewWidget::keyPressEvent ( QKeyEvent * event )
{
// 	qDebug() << " MainViewWidget::keyPressEvent(QKeyEvent * "<<event<<")";
// 	if ( event->key() == Qt::Key_Space &&  event->modifiers().testFlag ( Qt::ControlModifier ) )
// 	{
// 		// Switch list view
// 		if ( m_lists->fontlistTab->currentIndex() == 0 )
// 			m_lists->fontlistTab->setCurrentIndex ( 1 );
// 		else
// 			m_lists->fontlistTab->setCurrentIndex ( 0 );
// 	}
}

void MainViewWidget::slotAdjustGlyphView ( int width )
{
	if ( !theVeryFont )
		return;

// 	theVeryFont->adjustGlyphsPerRow ( width );
	slotView ( true );
}

void MainViewWidget::fillOTTree()
{
	OpenTypeTree->clear();
	langCombo->clear();
	langCombo->setEnabled ( false );
	useShaperCheck->setCheckState ( Qt::Unchecked );
	useShaperCheck->setEnabled ( false );
	QStringList scripts;
	if ( theVeryFont && theVeryFont->isOpenType() )
	{
		FmOtf * otf = theVeryFont->takeOTFInstance();
		foreach ( QString table, otf->get_tables() )
		{
			otf->set_table ( table );
			QTreeWidgetItem *tab_item = new QTreeWidgetItem ( OpenTypeTree,QStringList ( table ) );
			tab_item->setExpanded ( true );
			foreach ( QString script, otf->get_scripts() )
			{
				scripts << script;
				otf->set_script ( script );
				QTreeWidgetItem *script_item = new QTreeWidgetItem ( tab_item, QStringList ( script ) );
				script_item->setExpanded ( true );
				foreach ( QString lang, otf->get_langs() )
				{
					otf->set_lang ( lang );
					QTreeWidgetItem *lang_item = new QTreeWidgetItem ( script_item, QStringList ( lang ) );
					lang_item->setExpanded ( true );
					foreach ( QString feature, otf->get_features() )
					{
						QStringList f ( feature );
						f << OTTagMeans ( feature );
						QTreeWidgetItem *feature_item = new QTreeWidgetItem ( lang_item, f );
						feature_item->setCheckState ( 0, Qt::Unchecked );
					}
				}
			}
		}
		OpenTypeTree->resizeColumnToContents ( 0 ) ;
		theVeryFont->releaseOTFInstance ( otf );
	}
	scripts = scripts.toSet().toList();
// 	scripts.removeAll ( "latn" );
	if ( !scripts.isEmpty() )
	{
		langCombo->setEnabled ( true );
		useShaperCheck->setEnabled ( true );
		langCombo->addItems ( scripts );
	}
}

OTFSet MainViewWidget::deFillOTTree()
{
// 	qDebug() << "MainViewWidget::deFillOTTree()";
	OTFSet ret;
// 	qDebug() << OpenTypeTree->topLevelItemCount();
	for ( int table_index = 0; table_index < OpenTypeTree->topLevelItemCount(); ++table_index ) //tables
	{
// 		qDebug() << "table_index = " << table_index;
		QTreeWidgetItem * table_item = OpenTypeTree->topLevelItem ( table_index ) ;
// 		qDebug() <<  table_item->text(0);
		for ( int script_index = 0; script_index < table_item->childCount();++script_index ) //scripts
		{
			QTreeWidgetItem * script_item = table_item->child ( script_index );
// 			qDebug() << "\tscript_index = " <<  script_index << script_item->text(0);
			for ( int lang_index = 0; lang_index < script_item->childCount(); ++lang_index ) //langs
			{
				QTreeWidgetItem * lang_item = script_item->child ( lang_index );
// 				qDebug() << "\t\tlang_index = "<< lang_index << lang_item->text(0);
				for ( int feature_index = 0; feature_index < lang_item->childCount(); ++feature_index ) //features
				{
// 					qDebug() << lang_item->childCount() <<" / "<<  feature_index;
					QTreeWidgetItem * feature_item = lang_item->child ( feature_index );
// 					qDebug() << "\t\t\tfeature_item -> "<< feature_item->text(0);
					if ( feature_item->checkState ( 0 ) == Qt::Checked )
					{
						if ( table_item->text ( 0 ) == "GPOS" )
						{
							ret.script = script_item->text ( 0 );
							ret.lang = lang_item->text ( 0 );
							ret.gpos_features.append ( feature_item->text ( 0 ) );
						}
						if ( table_item->text ( 0 ) == "GSUB" )
						{
							ret.script = script_item->text ( 0 );
							ret.lang = lang_item->text ( 0 );
							ret.gsub_features.append ( feature_item->text ( 0 ) );
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
	slotView ( true );
}

void MainViewWidget::slotSampleChanged()
{
	slotView ( true );
}

void MainViewWidget::refillSampleList()
{
	sampleTextCombo->clear();
	QStringList sl = typo->namedSamplesNames();
	for ( int i = 0;i < sl.count(); ++i )
	{
		if ( sl[i] == "default" )
		{
			continue;
		}
		else
		{
			sampleTextCombo->addItem ( sl[i] );
		}
	}
	sampleTextCombo-> insertItem ( 0,tr ( "default" ) );
	sampleTextCombo->setCurrentIndex ( 0 );

}

void MainViewWidget::slotFTRasterChanged()
{
	slotView ( true );
}

void MainViewWidget::slotWantShape()
{
	slotView ( true );
}

void MainViewWidget::slotChangeScript()
{
	if ( useShaperCheck->checkState() == Qt::Checked )
	{
		slotView ( true );
	}
}

void MainViewWidget::slotSwitchRTL()
{
	slotView ( true );
}

void MainViewWidget::slotPlaneSelected ( int i )
{
	slotShowAllGlyph();
	slotView ( true );
	abcView->verticalScrollBar()->setValue ( 0 );
}


void MainViewWidget::slotShowOneGlyph()
{
	if ( abcScene->selectedItems().isEmpty() )
		return;

	curGlyph = reinterpret_cast<QGraphicsRectItem*> ( abcScene->selectedItems().first() );
	curGlyph->setSelected(false);
	if ( fancyGlyphInUse < 0 )
	{
		if(curGlyph->data ( 3 ).toInt() > 0)// Is a codepoint
			fancyGlyphInUse = theVeryFont->showFancyGlyph ( abcView, curGlyph->data ( 3 ).toInt() );
		else // Is a glyph index
			fancyGlyphInUse = theVeryFont->showFancyGlyph ( abcView, curGlyph->data ( 2 ).toInt() , true);
		if ( fancyGlyphInUse < 0 )
			return;
		abcView->setState(FMGlyphsView::SingleView);
	}
	

}

void MainViewWidget::slotShowAllGlyph()
{
	if (fancyGlyphInUse < 0)
		return;
	theVeryFont->hideFancyGlyph ( fancyGlyphInUse );
	fancyGlyphInUse = -1;
	abcView->setState ( FMGlyphsView::AllView );

}

void MainViewWidget::slotUpdateGView()
{
	// If all is how I think it must be, we don’t need to check anything here :)
	if(theVeryFont)
	{
		QString pkey = uniPlaneCombo->itemData ( uniPlaneCombo->currentIndex() ).toString();
		QPair<int,int> uniPair ( uniPlanes[pkey + uniPlaneCombo->currentText() ] );
		theVeryFont->renderAll ( abcScene , uniPair.first, uniPair.second );
		
	}
}

void MainViewWidget::slotUpdateSView()
{
	if(loremView->isVisible())
		slotView(true);
}

void MainViewWidget::slotSwitchCheckState(QListWidgetItem * item)
{
	if(contextMenuReq)
	{
		qDebug() << "\tWant contextual popup menu";
		typotek *typo = typotek::getInstance();
		QStringList sets = typo->tagsets();
		QMenu menu(tagsListWidget);
		for(int i=0; i< sets.count(); ++i)
		{
			
			if(typo->tagsOfSet(sets[i]).contains(item->text()))
			{
				QAction *entry = menu.addAction(QString("Remove from %1").arg(sets[i]));
				entry->setData(sets[i]);
			}
			else
			{
				QAction *entry = menu.addAction(QString("Add to %1").arg(sets[i]));
				entry->setData(sets[i]);
			}
		}
		QAction *sel = menu.exec(contextMenuPos);
		if(sel /*&& sel != mTitle*/)
		{
			if(sel->text().startsWith("Add"))
			{
				typo->addTagToSet(sel->data().toString(), item->text());
			}
			else
			{
				typo->removeTagFromSet(sel->data().toString(), item->text());
			}
		}
		contextMenuReq = false;
		return;
	}
	
	slotFinalize();
}

void MainViewWidget::slotNewTag()
{
	if ( newTagText->text().isEmpty() )
		return;
	if ( typotek::tagsList.contains ( newTagText->text() ) )
		return;

	typotek::tagsList.append ( newTagText->text() );
	QListWidgetItem *lit = new QListWidgetItem ( newTagText->text() );
	lit->setCheckState ( Qt::Checked );
	tagsListWidget->addItem ( lit );
	slotFinalize();
	emit tagAdded(newTagText->text());
	newTagText->clear();

}

void MainViewWidget::slotContextMenu(QPoint pos)
{
	contextMenuReq = true;
	contextMenuPos = tagsListWidget->mapToGlobal( pos );
}

void MainViewWidget::slotFinalize()
{
	QStringList plusTags;
	QStringList noTags;
	for ( int i=0;i< tagsListWidget->count();++i )
	{
		if ( tagsListWidget->item ( i )->checkState() == Qt::Checked )
			plusTags.append ( tagsListWidget->item ( i )->text() );
		if( tagsListWidget->item ( i )->checkState() == Qt::Unchecked )
			noTags.append ( tagsListWidget->item ( i )->text() );
	}
		
	QStringList sourceTags;
	for ( int i=0;i<theTaggedFonts.count();++i )
	{
		sourceTags = theTaggedFonts[i]->tags();
			// sourceTags -= noTags;
		for(int t = 0; t < noTags.count(); ++t)
		{
			sourceTags.removeAll(noTags[t]);
		}
		sourceTags += plusTags;
		sourceTags = sourceTags.toSet().toList();
		theTaggedFonts[i]->setTags ( sourceTags );
	}

}

void MainViewWidget::prepare(QList< FontItem * > fonts)
{
	slotFinalize();
	theTaggedFonts.clear();
	theTaggedFonts = fonts;
	for ( int i= theTaggedFonts.count() - 1; i >= 0; --i )
	{
		if ( theTaggedFonts[i]->isLocked() )
			theTaggedFonts.removeAt ( i );
	}
	tagsListWidget->clear();

	QString tot;
	for ( int i=0;i<theTaggedFonts.count();++i )
	{
		bool last = i == theTaggedFonts.count() - 1;
		tot.append (  theTaggedFonts[i]->fancyName() + (last ? "" : "\n") );
	}
// 	QString itsagroup = theFonts.count() > 1 ? " - " + theFonts.last()->name() :"";
// 	titleLabel->setText ( tit.arg ( theFonts[0]->fancyName() ) + itsagroup );
	if(theTaggedFonts.count() > 1)
	{
		titleLabel->setText ( theTaggedFonts[0]->family() + " (family)");
	}
	else
	{
		titleLabel->setText ( theTaggedFonts[0]->fancyName() );
	}
	titleLabel->setToolTip ( tot );

	for ( int i=0; i < typotek::tagsList.count(); ++i )
	{
		QString cur_tag = typotek::tagsList[i];

		if ( cur_tag.isEmpty() || cur_tag.contains ( "Activated_" )  )
			continue;

		QListWidgetItem *lit;
		
		{
			lit = new QListWidgetItem ( cur_tag );
			lit->setCheckState ( Qt::Unchecked );
			int YesState = 0;
			for ( int i=0;i<theTaggedFonts.count();++i )
			{
				if ( theTaggedFonts[i]->tags().contains ( cur_tag ) )
					++YesState;
			}
			if(YesState == theTaggedFonts.count())
				lit->setCheckState ( Qt::Checked );
			else if(YesState > 0 && YesState < theTaggedFonts.count())
				lit->setCheckState ( Qt::PartiallyChecked);
			
			tagsListWidget->addItem ( lit );
		}
	}
}

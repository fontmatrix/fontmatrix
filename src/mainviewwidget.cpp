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
#include "fmactivate.h"
#include "fmactivationreport.h"
#include "fmaltcontext.h"
#include "fmbaseshaper.h"
#include "fmglyphhighlight.h"
#include "fmglyphsview.h"
#include "fminfodisplay.h"
#include "fmlayout.h"
#include "fmotf.h"
#include "fmpaths.h"
#include "fmpreviewlist.h"
#include "fmuniblocks.h"
#include "fontitem.h"
#include "listdockwidget.h"
#include "opentypetags.h"
#include "panosematch.h"
#include "systray.h"
#include "typotek.h"
#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "tagswidget.h"
#include "fmutils.h"

#include <cstdlib>

#include <QString>
#include <QCompleter>
#include <QDebug>
#include <QGraphicsItemAnimation>
#include <QGraphicsItem>
#include <QTransform>
#include <QDialog>
#include <QGridLayout>
#include <QGraphicsRectItem>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QScrollBar>
#include <QGraphicsRectItem>
#include <QProcess>
#include <QProgressDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QStringListModel>
#include <QTime>
#include <QTimeLine>
#include <QClipboard>
#include <QMutex>
#include <QButtonGroup>
#include <QInputDialog>

// #include <QTimeLine>
// #include <QGraphicsItemAnimation>


MainViewWidget::MainViewWidget ( QWidget *parent )
		: QWidget ( parent )
{
	setupUi ( this );
	
	m_forceReloadSelection = false;
	uRangeIsNotEmpty = false;
	currentFonts.clear();

	QSettings settings;
	sampleFontSize = settings.value("Sample/FontSize", 14.0).toDouble();
	sampleInterSize = settings.value("Sample/Interline", 18.0).toDouble();
	sampleRatio = sampleInterSize / sampleFontSize  ;
	liveFontSizeSpin->setValue(sampleFontSize);

	iconPS1 =  QIcon(":/icon-PS1");
	iconTTF =  QIcon(":/icon-TTF");
	iconOTF =  QIcon(":/icon-OTF");
	
	unMapGlyphName = tr("Un-Mapped Glyphs");
	allMappedGlyphName = tr("View all mapped glyphs");
	
	textLayout = FMLayout::getLayout();
	
	radioRenderGroup = new QButtonGroup();
	radioRenderGroup->addButton(freetypeRadio);
	radioRenderGroup->addButton(nativeRadio);
	stackedTools->setCurrentIndex(VIEW_PAGE_SAMPLES);
	toolPanelWidth = splitter_2->sizes().at(1);
	restoreSplitterState();
	if(toolPanelWidth == 0)
	{
		sampleButton->setChecked(false);
		stackedTools->hide();
		toolPanelWidth = splitter_2->width()/3;
	}
	radioFTHintingGroup = new QButtonGroup(freetypeRadio);
	radioFTHintingGroup->addButton(noHinting);
	radioFTHintingGroup->addButton(lightHinting);
	radioFTHintingGroup->addButton(normalHinting);
	
	theVeryFont = 0;
	typo = typotek::getInstance();
	m_lists = ListDockWidget::getInstance();
// 	currentFonts = typo->getAllFonts();
	currentFonts = FMFontDb::DB()->AllFonts();
	fontsetHasChanged = true;
	curGlyph = 0;
	fancyGlyphInUse = -1;

	activateByFamilyOnly = settings.value("ActivateOnlyFamily", false).toBool();
	m_lists->actFacesButton->setChecked(!activateByFamilyOnly);

// 	fillUniPlanes();
	refillSampleList();
	uniLine->setEnabled(false);

	fontInfoText->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);

	abcScene = new QGraphicsScene;
	loremScene = new QGraphicsScene;
	ftScene =  new QGraphicsScene;
	playScene = new QGraphicsScene;
	QRectF pageRect ( 0,0,597.6,842.4 ); //TODO find means to smartly decide of page size (here, iso A4)
	
	loremScene->setSceneRect ( pageRect );
// 	QGraphicsRectItem *backp = loremScene->addRect ( pageRect,QPen(),Qt::white );
// 	backp->setEnabled ( false );
	
	ftScene->setSceneRect ( 0,0, 597.6 * typotek::getInstance()->getDpiX() / 72.0, 842.4 * typotek::getInstance()->getDpiX() / 72.0);
	

	abcView->setScene ( abcScene );
	abcView->setRenderHint ( QPainter::Antialiasing, true );

	loremView->setScene ( loremScene );
	loremView->locker = false;
	double horiScaleT (typotek::getInstance()->getDpiX() / 72.0);
	double vertScaleT ( typotek::getInstance()->getDpiY() / 72.0);
	QTransform adjustAbsoluteViewT( horiScaleT , 0, 0,vertScaleT, 0, 0 );
	loremView->setTransform ( adjustAbsoluteViewT , false );

	loremView_FT->setScene ( ftScene );
	loremView_FT->locker = false;
	loremView_FT->fakePage();

	playScene->setSceneRect ( 0,0,10000,10000 );
	playView->setScene( playScene );

	sampleText= typo->namedSample (typo->defaultSampleName());
	
	
	QMap<QString, int> sTypes(FMShaperFactory::types());
	for(QMap<QString, int>::iterator sIt = sTypes.begin(); sIt != sTypes.end() ; ++sIt)
	{
		shaperTypeCombo->addItem(sIt.key(), sIt.value());
	}
	
	classSplitter->restoreState(settings.value("WState/ClassificationSplitter").toByteArray());
	slotShowClassification();
	currentOrdering = "family" ;

	QStringListModel* cslModel(new QStringListModel);
	QCompleter* cslCompleter(new QCompleter(charSearchLine));
	cslCompleter->setModel(cslModel);
	charSearchLine->setCompleter(cslCompleter);
	
	doConnect();
}


MainViewWidget::~MainViewWidget()
{
}


void MainViewWidget::doConnect()
{
	connect(FMActivate::getInstance(), SIGNAL(activationEvent(const QStringList&)), this, SLOT(refreshActStatus(const QStringList&)));

	connect ( m_lists->fontTree,SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ),this,SLOT ( slotFontSelected ( QTreeWidgetItem*, int ) ) );
	connect ( m_lists->fontTree,SIGNAL ( currentChanged (QTreeWidgetItem*, int ) ), this,SLOT (slotFontSelected ( QTreeWidgetItem*, int ) ) );
	connect ( m_lists->searchString,SIGNAL ( returnPressed() ),this,SLOT ( slotSearch() ) );
	connect ( m_lists->viewAllButton,SIGNAL ( released() ),this,SLOT ( slotViewAll() ) );
	connect ( m_lists->fontTree,SIGNAL ( itemExpanded ( QTreeWidgetItem* ) ),this,SLOT ( slotItemOpened ( QTreeWidgetItem* ) ) );
	connect ( m_lists->tagsCombo,SIGNAL ( activated ( const QString& ) ),this,SLOT ( slotFilterTag ( QString ) ) );
	connect ( m_lists, SIGNAL(folderSelectFont(const QString&)), this, SLOT(slotSelectFromFolders(const QString&)));
	connect ( this, SIGNAL(listChanged()), m_lists, SLOT(slotPreviewUpdate()));
	connect ( m_lists->actFacesButton, SIGNAL(toggled( bool )), this, SLOT(toggleFacesCheckBoxes(bool)) );
	
	connect ( this, SIGNAL(listChanged()), typo, SLOT(showToltalFilteredFonts()));
	
	connect( fontInfoText, SIGNAL(linkClicked ( const QUrl& )), this, SLOT(slotWebLink(const QUrl&)));
	connect( fontInfoText, SIGNAL(loadStarted () ),this,SLOT(slotWebStart()));
	connect( fontInfoText, SIGNAL(loadProgress ( int )  ),this, SLOT(slotWebLoad(int)));
	connect( fontInfoText, SIGNAL(loadFinished ( bool ) ),this,SLOT(slotWebFinished(bool)));

	connect (radioRenderGroup,SIGNAL(buttonClicked( QAbstractButton* )),this,SLOT(slotChangeViewPage(QAbstractButton*)));
	connect (radioFTHintingGroup, SIGNAL(buttonClicked(int)),this,SLOT(slotHintChanged(int)));
	
	connect (openTypeButton,SIGNAL(clicked( bool )),this,SLOT(slotChangeViewPageSetting( bool )));
	connect (settingsButton,SIGNAL(clicked( bool )),this,SLOT(slotChangeViewPageSetting( bool )));
	connect (sampleButton,SIGNAL(clicked( bool )),this,SLOT(slotChangeViewPageSetting( bool )));
	
	connect ( abcView,SIGNAL ( pleaseShowSelected() ),this,SLOT ( slotShowOneGlyph() ) );
	connect ( abcView,SIGNAL ( pleaseShowAll() ),this,SLOT ( slotShowAllGlyph() ) );
	connect ( abcView,SIGNAL ( refit ( int ) ),this,SLOT ( slotAdjustGlyphView ( int ) ) );
	connect ( abcView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateGView()));
	connect ( abcView, SIGNAL(pleaseUpdateSingle()), this, SLOT(slotUpdateGViewSingle()));
	connect ( uniPlaneCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotPlaneSelected ( int ) ) );
	connect ( clipboardCheck, SIGNAL (toggled ( bool )),this,SLOT(slotShowULine(bool)));
	connect ( charSearchLine, SIGNAL(returnPressed()), this, SLOT(slotSearchCharName()));

	connect ( loremView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateSView()));
	connect ( loremView, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));
	

	connect ( loremView_FT, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));
	connect ( loremView_FT, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateRView()));
	
	connect ( textLayout, SIGNAL(updateLayout()),this, SLOT(slotView()));
	connect ( this, SIGNAL(stopLayout()), textLayout,SLOT(stopLayout()));

	connect ( playView, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));

	
	connect ( sampleTextTree,SIGNAL ( itemSelectionChanged ()),this,SLOT ( slotSampleChanged() ) );
	connect ( sampleTextButton, SIGNAL(released()),this, SLOT(slotEditSampleText()));
	connect ( liveFontSizeSpin, SIGNAL( editingFinished() ),this,SLOT(slotLiveFontSize()));

	connect ( OpenTypeTree, SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ), this, SLOT ( slotFeatureChanged() ) );
	connect ( saveDefOTFBut, SIGNAL(released()),this,SLOT(slotDefaultOTF()));
	connect ( resetDefOTFBut, SIGNAL(released()),this,SLOT(slotResetOTF()));
	connect ( shaperTypeCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotChangeScript() ) );
	connect ( langCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotChangeScript() ) );
	
	connect ( textProgression, SIGNAL ( stateChanged (  ) ),this ,SLOT(slotProgressionChanged()));
	connect ( useShaperCheck,SIGNAL ( stateChanged ( int ) ),this,SLOT ( slotWantShape() ) );
	
	connect ( classificationView, SIGNAL(selectedField(const QString&)), this, SLOT(slotUpdateClassDescription(const QString&)) );
	connect ( classificationView, SIGNAL(filterChanged()), this, SLOT(slotPanoseFilter()));
	connect ( classSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(slotSaveClassSplitter()));
}

void MainViewWidget::disConnect()
{
	disconnect(FMActivate::getInstance(), SIGNAL(activationEvent(const QStringList&)), this, SLOT(refreshActStatus(const QStringList&)));

	disconnect ( m_lists->fontTree,SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ),this,SLOT ( slotFontSelected ( QTreeWidgetItem*, int ) ) );
	disconnect ( m_lists->fontTree,SIGNAL ( currentChanged (QTreeWidgetItem*, int ) ), this,SLOT (slotFontSelected ( QTreeWidgetItem*, int ) ) );
	disconnect ( m_lists->searchString,SIGNAL ( returnPressed() ),this,SLOT ( slotSearch() ) );
	disconnect ( m_lists->viewAllButton,SIGNAL ( released() ),this,SLOT ( slotViewAll() ) );
	disconnect ( m_lists->fontTree,SIGNAL ( itemExpanded ( QTreeWidgetItem* ) ),this,SLOT ( slotItemOpened ( QTreeWidgetItem* ) ) );
	disconnect ( m_lists->tagsCombo,SIGNAL ( activated ( const QString& ) ),this,SLOT ( slotFilterTag ( QString ) ) );
	disconnect ( m_lists, SIGNAL(folderSelectFont(const QString&)), this, SLOT(slotSelectFromFolders(const QString&)));
	disconnect ( this, SIGNAL(listChanged()), m_lists, SLOT(slotPreviewUpdate()));
	disconnect ( m_lists->actFacesButton, SIGNAL(toggled( bool )), this, SLOT(toggleFacesCheckBoxes(bool)) );
	
	disconnect ( this, SIGNAL(listChanged()), typo, SLOT(showToltalFilteredFonts()));
	
	disconnect( fontInfoText, SIGNAL(linkClicked ( const QUrl& )), this, SLOT(slotWebLink(const QUrl&)));
	disconnect( fontInfoText, SIGNAL(loadStarted () ),this,SLOT(slotWebStart()));
	disconnect( fontInfoText, SIGNAL(loadProgress ( int )  ),this, SLOT(slotWebLoad(int)));
	disconnect( fontInfoText, SIGNAL(loadFinished ( bool ) ),this,SLOT(slotWebFinished(bool)));

	disconnect (radioRenderGroup,SIGNAL(buttonClicked( QAbstractButton* )),this,SLOT(slotChangeViewPage(QAbstractButton*)));
	disconnect (radioFTHintingGroup, SIGNAL(buttonClicked(int)),this,SLOT(slotHintChanged(int)));
	
	disconnect (openTypeButton,SIGNAL(clicked( bool )),this,SLOT(slotChangeViewPageSetting( bool )));
	disconnect (settingsButton,SIGNAL(clicked( bool )),this,SLOT(slotChangeViewPageSetting( bool )));
	disconnect (sampleButton,SIGNAL(clicked( bool )),this,SLOT(slotChangeViewPageSetting( bool )));
	
	disconnect ( abcView,SIGNAL ( pleaseShowSelected() ),this,SLOT ( slotShowOneGlyph() ) );
	disconnect ( abcView,SIGNAL ( pleaseShowAll() ),this,SLOT ( slotShowAllGlyph() ) );
	disconnect ( abcView,SIGNAL ( refit ( int ) ),this,SLOT ( slotAdjustGlyphView ( int ) ) );
	disconnect ( abcView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateGView()));
	disconnect ( abcView, SIGNAL(pleaseUpdateSingle()), this, SLOT(slotUpdateGViewSingle()));
	disconnect ( uniPlaneCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotPlaneSelected ( int ) ) );
	disconnect ( clipboardCheck, SIGNAL (toggled ( bool )),this,SLOT(slotShowULine(bool)));
	disconnect ( charSearchLine, SIGNAL(returnPressed()), this, SLOT(slotSearchCharName()));

	disconnect ( loremView, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateSView()));
	disconnect ( loremView, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));
	

	disconnect ( loremView_FT, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));
	disconnect ( loremView_FT, SIGNAL(pleaseUpdateMe()), this, SLOT(slotUpdateRView()));
	
	disconnect ( textLayout, SIGNAL(updateLayout()),this, SLOT(slotView()));
	disconnect ( this, SIGNAL(stopLayout()), textLayout,SLOT(stopLayout()));

	disconnect ( playView, SIGNAL(pleaseZoom(int)),this,SLOT(slotZoom(int)));

	
	disconnect ( sampleTextTree,SIGNAL ( itemSelectionChanged() ),this,SLOT ( slotSampleChanged() ) );
	disconnect ( sampleTextButton, SIGNAL(released()),this, SLOT(slotEditSampleText()));
	disconnect ( liveFontSizeSpin, SIGNAL( editingFinished() ),this,SLOT(slotLiveFontSize()));

	disconnect ( OpenTypeTree, SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ), this, SLOT ( slotFeatureChanged() ) );
	disconnect ( saveDefOTFBut, SIGNAL(released()),this,SLOT(slotDefaultOTF()));
	disconnect ( resetDefOTFBut, SIGNAL(released()),this,SLOT(slotResetOTF()));
	disconnect ( shaperTypeCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotChangeScript() ) );
	disconnect ( langCombo,SIGNAL ( activated ( int ) ),this,SLOT ( slotChangeScript() ) );
	
	disconnect ( textProgression, SIGNAL ( stateChanged (  ) ),this ,SLOT(slotProgressionChanged()));
	disconnect ( useShaperCheck,SIGNAL ( stateChanged ( int ) ),this,SLOT ( slotWantShape() ) );
	
	disconnect ( classificationView, SIGNAL(selectedField(const QString&)), this, SLOT(slotUpdateClassDescription(const QString&)) );
	disconnect ( classificationView, SIGNAL(filterChanged()), this, SLOT(slotPanoseFilter()));
	disconnect ( classSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(slotSaveClassSplitter()));
}

void MainViewWidget::fillTree()
{
// 	qDebug()<< "MainViewWidget::fillTree("<< curItemName <<")";
// 	QTime fillTime(0, 0, 0, 0);
// 	fillTime.start();
	m_lists->savePosition();
	QTreeWidgetItem *curItem = 0;

	// We log expanded families item
	openKeys.clear();
	for ( int i=0; i < m_lists->fontTree->topLevelItemCount();++i )
	{
		QTreeWidgetItem *topit = m_lists->fontTree->topLevelItem ( i );
		for ( int j=0;j < topit->childCount();++j )
			if ( topit->child ( j )->isExpanded() )
				openKeys << topit->child ( j )->text ( 0 );
	}
	m_lists->fontTree->clear();

// 	qDebug("LOGS Time elapsed: %d ms", fillTime.elapsed());
// 	fillTime.restart();

	// build the in-memory tree that hold the ordered current fonts set
	QMap< QChar,QMap< QString,QMap< QString,FontItem*> > > keyList;
	QMap< QString,QString > realFamilyName;
	orderedCurrentFonts.clear();
	for ( int i=0; i < currentFonts.count();++i )
	{
		QString family = currentFonts[i]->family();
		QString ordFamily = family.toUpper();
		QString variant = currentFonts[i]->variant();
		if( keyList.contains(ordFamily[0]) && keyList[ordFamily[0]].contains(ordFamily) && keyList[ordFamily[0]][ordFamily].contains(variant) )
		{
			int unique = 2;
			QString uniString(variant +" -%1");
			while(keyList[ordFamily[0]][ordFamily].contains(uniString.arg(unique,2)))
			{
				++unique;
			}
			variant = uniString.arg(unique,2);
		}
		keyList[ordFamily[0]][ordFamily][variant] = ( currentFonts[i] );
		realFamilyName[ordFamily] = family;
	}

// 	qDebug("MULTI Time elapsed: %d ms", fillTime.elapsed());
// 	fillTime.restart();

	// Rebuild the the tree
	QFont alphaFont ( "helvetica",14,QFont::Bold,false );

	QMap< QChar,QMap< QString,QMap< QString,FontItem*> > >::const_iterator kit;
	QMap< QString,QMap< QString,FontItem*> >::const_iterator oit;
	QMap< QString,FontItem*>::const_iterator fit;

// 	QTime tt(0,0);
// 	tt.start();
// 	int tttotal(0);
// 	int tcount(0);
	
	QMap<FontItem*,bool> act ;
	FMFontDb::DB()->TransactionBegin();
	for( kit = keyList.constBegin() ; kit != keyList.constEnd() ; ++kit )
	{
		for (oit = kit.value().constBegin(); oit !=  kit.value().constEnd(); ++ oit )
		{
			for(fit = oit.value().constBegin(); fit != oit.value().constEnd(); ++fit)
			{
				act[fit.value()] = fit.value()->isActivated();
			}
		}
	}
	FMFontDb::DB()->TransactionEnd();
	
	for( kit = keyList.constBegin() ; kit != keyList.constEnd() ; ++kit )
	{
		QChar firstChar ( kit.key() );
		QTreeWidgetItem *alpha = new QTreeWidgetItem ( m_lists->fontTree );
		alpha->setText ( 0, firstChar );
		alpha->setFont ( 0, alphaFont );
		alpha->setData ( 0,100,"alpha" );
		alpha->setBackgroundColor ( 0,Qt::lightGray );
		alpha->setBackgroundColor ( 1,Qt::lightGray );
		bool alphaIsUsed = false;
		for (oit = kit.value().constBegin(); oit !=  kit.value().constEnd(); ++ oit )
		{

			QString fam( realFamilyName[oit.key()] );
			bool isExpanded = false;
			QTreeWidgetItem *ord = new QTreeWidgetItem ( alpha );
			
			ord->setData ( 0,100,"family" );
			ord->setCheckState ( 0,Qt::Unchecked );
			bool chekno = false;
			bool checkyes = false;
			if ( openKeys.contains ( fam ) )
			{
				ord->setExpanded ( true );
				isExpanded = true;
			}

			for(fit = oit.value().constBegin(); fit != oit.value().constEnd(); ++fit)
			{
// 				++tcount;
// 				tt.restart();
				FontItem *fPointer = fit.value();
				orderedCurrentFonts << fPointer;
				QString var( fit.key() );
				if(typo->isSysFont(fit.value()))
					var+="*";
				QTreeWidgetItem *entry = new QTreeWidgetItem ( ord );
				entry->setText ( 0,  var );
// 				entry->setText ( 1, fPointer->path() );
				entry->setToolTip( 0, fPointer->path() );
				entry->setData ( 0, 100, "fontfile" );
				if(/*fPointer->isLocked() ||*/ fPointer->isRemote() )
				{
					entry->setFlags(Qt::ItemIsSelectable);
				}

				if(fPointer->type() == "CFF")
						entry->setIcon(0, iconOTF );
				else if(fPointer->type() == "TrueType")
						entry->setIcon(0, iconTTF);
				else if(fPointer->type() == "Type 1")
						entry->setIcon(0, iconPS1);

				
				if ( act[fPointer] )
				{
					checkyes = true;
				}
				else
				{
					chekno = true;
				}
				if( !activateByFamilyOnly )
				{
					entry->setCheckState ( 0 , act[fPointer] ?  Qt::Checked : Qt::Unchecked );
				}
				
				entry->setData ( 0,200, entry->checkState ( 0 ) );

				if ( entry->toolTip( 0 ) == curItemName )
					curItem = entry;
// 				tttotal += tt.elapsed();
			}

			if ( checkyes && chekno )
				ord->setCheckState ( 0,Qt::PartiallyChecked );
			else if ( checkyes )
				ord->setCheckState ( 0,Qt::Checked );
			// track checkState
			ord->setData ( 0,200,ord->checkState ( 0 ) );
			ord->setText ( 0, fam + "  ["+ QString::number ( ord->childCount() ) +"]");
// 			ord->setText ( 1, QString::number ( ord->childCount() ) );
			alphaIsUsed = true;

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
// 	qDebug()<<"SUB TREE Time Total: "<<tttotal<<" ms; Iterations : "  << tcount<< "; Average" << (double)tttotal / (double)tcount <<" ms/It";
// 	qDebug("TREE Time elapsed: %d ms", fillTime.elapsed());
// 	fillTime.restart();

// 	m_lists->previewList->slotRefill ( currentFonts, fontsetHasChanged );
	if ( curItem )
	{
// 		qDebug() << "get curitem : " << curItem->text ( 0 ) << curItem->text ( 1 );
		m_lists->restorePosition();
		if( !m_lists->nameItemIsVisible(curItem) )
		{
			m_lists->fontTree->scrollToItem ( curItem, QAbstractItemView::PositionAtCenter );
		}

// // 		QColor scol (255,240,221,255);
// // 		QColor pcol (255,211,155,255);
// 		QColor scol (QApplication::palette().highlight().color());
// 		QColor pcol (scol);
// 		QFont selFont;
// 		selFont.setBold(true);
// 		curItem->parent()->setBackgroundColor ( 0,pcol );
// 		curItem->parent()->setBackgroundColor ( 1,pcol );
// 		curItem->parent()->setFont(0, selFont);
// 		curItem->setBackgroundColor ( 0,scol );
// // 		curItem->setBackgroundColor ( 1,scol );
// 		curItem->setFont(0,selFont);
		QFont selFont;
		selFont.setBold ( true );
		curItem->parent()->setFont ( 0, selFont );
		curItem->setSelected(true);
	}
	else
	{
		qDebug() << "NO CURITEM";
	}
	m_lists->fontTree->resizeColumnToContents ( 0 )  ;
// 	m_lists->fontTree->resizeColumnToContents ( 1 ) ;
// 	m_lists->fontTree->setColumnWidth(0,200);

	fontsetHasChanged = false;
	listChanged();
// 	m_lists->slotPreviewUpdate();
// 	qDebug("END Time elapsed: %d ms", fillTime.elapsed());
}

void MainViewWidget::updateTree ( bool checkFontActive )
{
	QTreeWidgetItem *curItem = 0;
	QFont deselect;
	int topCount ( m_lists->fontTree->topLevelItemCount() );
	for ( int topIdx ( 0 ) ; topIdx < topCount; ++topIdx )
	{
		QTreeWidgetItem *topItem ( m_lists->fontTree->topLevelItem ( topIdx ) );
		int famCount ( topItem->childCount() );
		for ( int famIdx ( 0 ); famIdx < famCount; ++ famIdx )
		{
			QTreeWidgetItem *famItem ( topItem->child ( famIdx ) );
			int varCount ( famItem->childCount() );
			for ( int varIdx ( 0 ); varIdx < varCount; ++ varIdx )
			{
				QTreeWidgetItem *varItem ( famItem->child ( varIdx ) );
				if ( varItem->toolTip ( 0 ) == lastIndex )
				{
					varItem->setFont ( 0, deselect );
					varItem->setBackgroundColor ( 0, Qt::transparent );
					varItem->setBackgroundColor ( 1, Qt::transparent );
					varItem->parent()->setFont ( 0, deselect );
					varItem->parent()->setBackgroundColor ( 0, Qt::transparent );
					varItem->parent()->setBackgroundColor ( 1, Qt::transparent );
					varItem->setSelected(false);
				}
				else if ( varItem->toolTip ( 0 ) == curItemName )
				{
					curItem = varItem;
				}
			}
		}
	}
	
	// Check if active
	if ( checkFontActive )
	{
		for ( int topIdx ( 0 ) ; topIdx < topCount; ++topIdx )
		{
			QTreeWidgetItem *topItem ( m_lists->fontTree->topLevelItem ( topIdx ) );
			int famCount ( topItem->childCount() );
			for ( int famIdx ( 0 ); famIdx < famCount; ++ famIdx )
			{
				QTreeWidgetItem *famItem ( topItem->child ( famIdx ) );
				int varCount ( famItem->childCount() );
				if ( famItem->isExpanded() )
				{
					for ( int varIdx ( 0 ); varIdx < varCount; ++ varIdx )
					{
						QTreeWidgetItem *varItem ( famItem->child ( varIdx ) );
						// Check if active

						QString s ( varItem->toolTip ( 0 ) );
						FontItem* f ( FMFontDb::DB()->Font ( s ) );
						if ( f && f->isActivated())
						{
							if( !activateByFamilyOnly )
								varItem->setCheckState ( 0,Qt::Checked );
						}
						else
						{
							if( !activateByFamilyOnly )
								varItem->setCheckState ( 0,Qt::Unchecked );
						}
					}
				}
			}
		}
	}
// 	m_lists->previewList->slotRefill ( currentFonts, false );
	if ( curItem )
	{
		QFont selFont;
		selFont.setBold ( true );
		curItem->parent()->setFont ( 0, selFont );
		curItem->setSelected(true);
	}
	else
	{
		qDebug() << "NO CURITEM";
	}
	if ( !m_lists->nameItemIsVisible ( curItem ) )
	{
		m_lists->fontTree->scrollToItem ( curItem, QAbstractItemView::PositionAtCenter );
	}
	fontsetHasChanged = false;
}

void MainViewWidget::refreshActStatus(const QStringList& flist)
{
	if(flist.isEmpty())
		return;
	QStringList l_flist(flist);
	int topCount ( m_lists->fontTree->topLevelItemCount() );
	for ( int topIdx ( 0 ) ; topIdx < topCount; ++topIdx )
	{
		QTreeWidgetItem *topItem ( m_lists->fontTree->topLevelItem ( topIdx ) );
		int famCount ( topItem->childCount() );
		for ( int famIdx ( 0 ); famIdx < famCount; ++ famIdx )
		{
			QTreeWidgetItem *famItem ( topItem->child ( famIdx ) );
			int varCount ( famItem->childCount() );
			if ( famItem->isExpanded() )
			{
				for ( int varIdx ( 0 ); varIdx < varCount; ++ varIdx )
				{
					QTreeWidgetItem *varItem ( famItem->child ( varIdx ) );
					// Check if active

					QString s ( varItem->toolTip ( 0 ) );
					if(l_flist.contains(s))
					{
						FontItem* f ( FMFontDb::DB()->Font ( s ) );
						if ( f && f->isActivated())
							varItem->setCheckState ( 0, Qt::Checked );
						else
							varItem->setCheckState ( 0, Qt::Unchecked );

						l_flist.removeAll(s);
						if(l_flist.isEmpty())
							return;
					}
				}
			}
		}
	}
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
// 		fillTree();
	}

	if ( item->data ( 0,100 ).toString() == "family" )
	{
// 		qDebug() << "Item is a family";
		item->setExpanded(true);
		bool wantView = true;
		bool hasChild = false;
		QStringList names;
		QMap<QString, QString> variantMap;
		for ( int i=0; i < item->childCount(); ++i )
		{
			hasChild = true;
			if ( item->child( i )->toolTip(0) == curItemName )
				wantView = false;
			names << item->child( i )->toolTip( 0 ) ;
			variantMap[item->child ( i )->text ( 0 ) ] = item->child ( i )->toolTip(0) ;
		}
		slotFontActionByNames ( names );
		int oldc = item->data ( 0,200 ).toInt();
		if ( oldc == item->checkState ( 0 ) ) // filters when checkbox has not been hit
		{
			// TODO keep an eye on it
// 			fillTree();

		}
		else if ( item->checkState ( 0 ) != Qt::PartiallyChecked )
		{

			bool cs = item->checkState ( 0 ) == Qt::Checked ? true : false;
			item->setData( 0,200, item->checkState ( 0 ) );
			QList<FontItem*> todo;
			for ( int i=0; i<item->childCount(); ++i )
			{
				todo << FMFontDb::DB()->Font( item->child ( i )->toolTip(0) );
			}
// 			for (int fIndex(0);fIndex < todo.count(); ++fIndex)
// 			{
// 				FontItem* afont = todo[fIndex];
// 				if(fIndex == todo.count() - 1)
// 					activation ( afont, cs , true);
// 				else
// 					activation ( afont, cs , false);
// 			}
			activation ( todo, cs );

		}
		else
		{
			qDebug() << "Something wrong, Qt::PartiallyChecked should not be reached" ;

		}
		if ( wantView && hasChild )
		{
			QString select;

			if ( variantMap.contains ( "Regular" ) )
				select =  variantMap["Regular"];
			else if ( variantMap.contains ( "Roman" ) )
				select =  variantMap["Roman"];
			else if ( variantMap.contains ( "Medium" ) )
				select =  variantMap["Medium"];
			else if ( variantMap.contains ( "Book" ) )
				select =  variantMap["Book"];
			else
				select =  * ( variantMap.begin() );

// 			m_lists->previewList->slotSelect(select);
			slotFontSelectedByName(select);

		}
		return;
	}

	if ( item->data ( 0,100 ).toString() == "fontfile" )
	{
		QString fontname(item->toolTip(0));
		bool wantActivate = (item->checkState(0) == Qt::Checked) ? true : false;
// 		m_lists->previewList->slotSelect(fontname);
		slotFontSelectedByName(fontname);
// 		if ( !theVeryFont->isLocked() )
		{
			if(theVeryFont->isActivated())
			{
				if(!wantActivate)
				{
					QList<FontItem*> fl;
					fl << theVeryFont;
					activation(fl,false);
				}
			}
			else
			{
				if(wantActivate)
				{
					QList<FontItem*> fl;
					fl << theVeryFont;
					activation(fl,true);
				}
			}
		}
	}
	return;

}

void MainViewWidget::slotFontSelectedByName ( QString fname )
{

	if ( fname.isEmpty()
		|| ((fname ==  faceIndex) && (!m_forceReloadSelection)) )
		return;
	m_forceReloadSelection = false;
	lastIndex = faceIndex;
	faceIndex = fname;
	curItemName = faceIndex;

	{
// 		qDebug() << "Font has changed \n\tOLD : "<<lastIndex<<"\n\tNEW : " << faceIndex ;
		if(abcView->state() == FMGlyphsView::SingleView)
			slotShowAllGlyph();
		theVeryFont = FMFontDb::DB()->Font( faceIndex );
// 		theVeryFont->updateItem();
		slotFontActionByName ( fname );
		if(theVeryFont->isRemote())
		{
			qDebug() << faceIndex <<" is remote";
			if(!theVeryFont->isCached())
			{
				connect(theVeryFont,SIGNAL(dowloadFinished()), this, SLOT(slotRemoteFinished()));
				theVeryFont->getFromNetwork();
				currentDownload = faceIndex ;
				faceIndex = lastIndex;
				return;
			}
			else
			{
				currentDownload = "";
			}
		}
		fillOTTree();
		fillUniPlanesCombo ( theVeryFont );
		QStringListModel *m = reinterpret_cast<QStringListModel*>(charSearchLine->completer()->model());
		if(m) 
			m->setStringList(theVeryFont->getNames());
		slotView ( true );
		typo->setWindowTitle ( theVeryFont->fancyName() + " - Fontmatrix" );
		m_lists->fontTree->headerItem()->setText(0, tr("Names")+" ("+theVeryFont->family()+")");
		typo->presentFontName ( theVeryFont->fancyName() );
// 		fillTree();
		updateTree();
		m_lists->listPreview->setCurrentFont(theVeryFont->path());
		abcView->verticalScrollBar()->setValue ( 0 );
		
		// Update panose widget
		QStringList plist(FMFontDb::DB()->getValue(theVeryFont->path(), FMFontDb::Panose).toString().split(":"));
		if(plist.count() == 10)
		{
			QMap<QString,QStringList> filter;
			FontStrings::PanoseKey pk(FontStrings::firstPanoseKey());
			do
			{
				QString v(FontStrings::Panose().value(pk).value(plist[pk].toInt()));
				filter[FontStrings::PanoseKeyName(pk)] << v;
// 				qDebug()<<"F"<<FontStrings::PanoseKeyName(pk)<<v;
				pk = FontStrings::nextPanoseKey(pk);
			}
			while(pk != FontStrings::InvalidPK);
			disconnect ( classificationView, SIGNAL(filterChanged()), this, SLOT(slotPanoseFilter()));
			classificationView->setFilter(filter);
			connect ( classificationView, SIGNAL(filterChanged()), this, SLOT(slotPanoseFilter()));
		}
	}


}


void MainViewWidget::slotInfoFont()
{
	if(theVeryFont)
	{
		FMInfoDisplay fid(theVeryFont);
                fontInfoText->setContent(fid.getHtml().toUtf8(), "application/xhtml+xml");
	}
	

}

void MainViewWidget::slotView ( bool needDeRendering )
{
	QTime t;
	t.start();
	FontItem *l = FMFontDb::DB()->Font( lastIndex );
	FontItem *f = FMFontDb::DB()->Font( faceIndex );
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

	bool wantDeviceDependant = loremView_FT->isVisible();
	unsigned int storedHinting(theVeryFont->getFTHintMode());
	if(wantDeviceDependant)
	{
		theVeryFont->setFTHintMode(hinting());
	}

	if(textProgression->inLine() == TextProgression::INLINE_LTR )
		theVeryFont->setProgression(PROGRESSION_LTR );
	else if(textProgression->inLine() == TextProgression::INLINE_RTL )
		theVeryFont->setProgression(PROGRESSION_RTL);
	else if(textProgression->inLine() == TextProgression::INLINE_TTB )
		theVeryFont->setProgression(PROGRESSION_TTB );
	else if(textProgression->inLine() == TextProgression::INLINE_BTT )
		theVeryFont->setProgression(PROGRESSION_BTT);

	theVeryFont->setFTRaster ( wantDeviceDependant );
	theVeryFont->setShaperType(shaperTypeCombo->itemData(  shaperTypeCombo->currentIndex() ).toInt() );

	if ( loremView->isVisible() || loremView_FT->isVisible() )
	{
//		qDebug()<<"lv(ft) is visible";
		if(textLayout->isRunning())
		{
//			qDebug()<<"tl is running";
			textLayout->stopLayout();
		}
		else
		{
//			qDebug()<<"tl is NOT running";
			QGraphicsScene *targetScene;
			loremView_FT->unSheduleUpdate();
			loremView->unSheduleUpdate();
			if(loremView->isVisible())
			{
				targetScene = loremScene;
			}
			else if(loremView_FT->isVisible())
			{
				targetScene = ftScene;
			}
			
			bool processFeatures = f->isOpenType() &&  !deFillOTTree().isEmpty();
			QString script = langCombo->currentText();
                        bool processScript =  f->isOpenType() && ( useShaperCheck->checkState() == Qt::Checked ) && ( !script.isEmpty() );

			textLayout->setTheFont(theVeryFont);
			textLayout->setDeviceIndy(!wantDeviceDependant);
			textLayout->setTheScene(targetScene);
			textLayout->setAdjustedSampleInter( sampleInterSize );
			
			double fSize(sampleFontSize);
			
			QList<GlyphList> list;
			QStringList stl( typo->namedSample(sampleTextTree->currentItem()->data(0, Qt::UserRole).toString() ).split("\n"));
			if ( processScript )
			{
				for(int p(0);p<stl.count();++p)
				{
					list << theVeryFont->glyphs( stl[p] , fSize, script );
				}
			}
			else if(processFeatures)
			{
				// Experimental code to handle alternate is commented out
				// Do not uncomment
//				FMAltContext * actx ( FMAltContextLib::SetCurrentContext(sampleTextTree->currentText(), theVeryFont->path()));
//				int rs(0);
//				actx->setPar(rs);
				for(int p(0);p<stl.count();++p)	
				{
					list << theVeryFont->glyphs( stl[p] , fSize, deFillOTTree());
//					actx->setPar(++rs);
				}
//				actx->cleanup();
//				FMAltContextLib::SetCurrentContext(sampleTextTree->currentText(), theVeryFont->path());
			}
			else
			{
				for(int p(0);p<stl.count();++p)
					list << theVeryFont->glyphs( stl[p] , fSize  );
			}
			textLayout->doLayout(list, fSize);
// 			if (loremView->isVisible() /*&& fitViewCheck->isChecked()*/ )
// 			{
// 				loremView->fitInView ( textLayout->getRect(), Qt::KeepAspectRatio );
// 			}
			textLayout->start(QThread::LowestPriority);
		}
	}
	else if(!loremView->isVisible() && !loremView_FT->isVisible())
	{
		loremView->sheduleUpdate();
		loremView_FT->sheduleUpdate();
	}
	
	slotUpdateGView();
	slotInfoFont();

}


void MainViewWidget::slotSearch()
{
// 	qDebug()<<"slotSearch";
	m_lists->fontTree->clear();
// 	m_lists->previewList->slotRefill(QList<FontItem*>(), true);
	fontsetHasChanged = true;

	QApplication::setOverrideCursor ( Qt::WaitCursor );
	QString fs ( m_lists->searchString->text() );

	int field(  m_lists->getCurrentFieldAction()->data().toInt() );
	QList<FontItem*> tmpList;
	
	if(field == FILTER_FIELD_SPECIAL_UNICODE)  //Unicode
	{
		QList<FontItem*> allList;

		int startC(0xFFFFFFFF);
		int endC(0);
		int patCount(fs.count());
		for(int a(0); a < patCount; ++a)
		{
			unsigned int ca(fs[a].unicode());
			if( ca < startC)
				startC = ca;
			if(ca > endC)
				endC = ca;
		}
		
		// FontItem->countCoverage is very costly, so we take some code from operateFilter
		// to avoid calling it too much, if possible.
		bool queue(m_lists->getOperation().contains("AND"));
		allList = queue ? currentFonts : FMFontDb::DB()->AllFonts();
		int superSetCount(allList.count()); 
		for ( int i =0; i < superSetCount; ++i )
		{
			int cc(allList[i]->countCoverage ( startC, endC ) );
			if ( cc >= patCount )
			{
				tmpList.append ( allList[i]);
			}
		}
		
		operateFilter( tmpList, QString("U://") + QString(fs)  );
	}
	else if(field == FMFontDb::AllInfo)
	{
		FMFontDb::InfoItem k;

		tmpList.clear();
		for(int gIdx(0); gIdx < FontStrings::Names().keys().count() ; ++gIdx)
		{
			k = FontStrings::Names().keys()[gIdx];
			if(k !=  FMFontDb::AllInfo)
			{
				tmpList +=  FMFontDb::DB()->Fonts(fs,k);
			}
		}
		
		operateFilter(tmpList, fs);
			
	}
	else
	{
		tmpList =  FMFontDb::DB()->Fonts(fs, FMFontDb::InfoItem(field ) );
		operateFilter(tmpList, fs);
	}
	
	currentOrdering = "family";
	fillTree();
	m_lists->searchString->clear();
	
	QApplication::restoreOverrideCursor();
}


void MainViewWidget::slotFilterTag ( QString tag )
{
	int tIdx(m_lists->tagsCombo->currentIndex());
	if(tIdx < 0)
		return;

	QString key(m_lists->tagsCombo->itemData(tIdx).toString());
	
	if(key == "TAG") // regular tag
	{
		m_lists->fontTree->clear();
		fontsetHasChanged = true;
		operateFilter( FMFontDb::DB()->Fonts(tag, FMFontDb::Tags ), tag);
		currentOrdering = "family";
		fillTree();
	}
	else if(key == "ALL_ACTIVATED")
	{
		m_lists->fontTree->clear();
		fontsetHasChanged = true;
		operateFilter( FMFontDb::DB()->Fonts(1, FMFontDb::Activation ), tr("Activated"));
		currentOrdering = "family";
		fillTree();
	}
	else if(key == "SIMILAR")
	{
		if(theVeryFont)
		{
			m_lists->fontTree->clear();
			fontsetHasChanged = true;
			operateFilter( PanoseMatchFont::similar(theVeryFont, typo->getPanoseMatchTreshold() ), "S://"+ theVeryFont->family());
			fillTree();
		}
	}
}

void MainViewWidget::operateFilter(QList< FontItem * > allFiltered, const QString filterName)
{
	QList<FontItem*> tmpList = allFiltered;
	QList<FontItem*> negList;
	QList<FontItem*> queList;	
	
	QStringList ops(m_lists->getOperation());
	bool negate(ops.contains("NOT"));
	bool queue(ops.contains("AND"));
	m_lists->clearOperation();
	
	if(queue)
	{
		addFilterToCrumb((negate?"!":"") +filterName);
		queList = currentFonts;
	}
	else
	{
		setCrumb();
		addFilterToCrumb((negate?"!":"") +filterName);
	}
	if(negate)
		negList = FMFontDb::DB()->AllFonts();
		
	currentFonts.clear();
	
	if(negate)
	{
		if(queue)
		{
			foreach(FontItem* f, negList)
			{
				if(!currentFonts.contains(f) && !tmpList.contains(f) && queList.contains(f))
					currentFonts.append(f);
			}
		}
		else // not queue
		{
			foreach(FontItem* f, negList)
			{
				if(!currentFonts.contains(f) && !tmpList.contains(f))
					currentFonts.append(f);
			}
		}
	}
	else // not negate
	{
		if(queue)
		{
			foreach(FontItem* f, tmpList)
			{
				if(!currentFonts.contains(f) && queList.contains(f))
					currentFonts.append(f);
			}
		}
		else // not queue
		{
			foreach(FontItem* f, tmpList)
			{
				if(!currentFonts.contains(f))
					currentFonts.append(f);
			}
		}
	}	
}


void MainViewWidget::slotFontAction ( QTreeWidgetItem * item, int column )
{
// 	qDebug()<<"MainViewWidget::slotFontAction";
	if ( column >2 ) return;

	FontItem * FoIt = FMFontDb::DB()->Font( item->text ( 1 ) );
	if ( FoIt/* && (!FoIt->isLocked())*/ )
	{
		QList<FontItem*> fl;
		fl.append ( FoIt );
		TagsWidget::getInstance()->prepare ( fl );
	}
}

void MainViewWidget::slotFontActionByName (const QString &fname )
{
// 	qDebug()<<"MainViewWidget::slotFontActionByName ("<< fname <<")";
	FontItem * FoIt = FMFontDb::DB()->Font( fname );
	if ( FoIt/* && (!FoIt->isLocked())*/ )
	{
		QList<FontItem*> fl;
		fl.append ( FoIt );
		TagsWidget::getInstance()->prepare ( fl );
	}
}

void MainViewWidget::slotFontActionByNames ( QStringList fnames )
{
// 	qDebug()<<"MainViewWidget::slotFontActionByNames ("<< fnames.join(";") <<")";
	QList<FontItem*> FoIt;
	for ( int i= 0; i < fnames.count() ; ++i )
	{
		FoIt.append ( FMFontDb::DB()->Font( fnames[i] ) );
	}
	if ( FoIt.count() )
		TagsWidget::getInstance()->prepare ( FoIt );
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

	TagsWidget::getInstance()->prepare ( fl );
}


void MainViewWidget::slotZoom ( int z )
{
	double delta =  1.0 + ( z/1000.0 ) ;
	QTransform trans;
	trans.scale ( delta,delta );

	QGraphicsView * concernedView;
	if ( loremView_FT->isVisible() )
		concernedView = loremView_FT;
	else if ( loremView->isVisible() )
	{
		concernedView = loremView;
		if ( delta == 1.0 )
		{
			double horiScaleT (typotek::getInstance()->getDpiX() / 72.0);
			double vertScaleT ( typotek::getInstance()->getDpiY() / 72.0);
			QTransform adjustAbsoluteViewT( horiScaleT , 0, 0,vertScaleT, 0, 0 );
			trans =  adjustAbsoluteViewT;
		}
	}
	else if ( playView->isVisible() )
		concernedView = playView;

	concernedView->setTransform ( trans, ( z == 0 ) ? false : true );

}

void MainViewWidget::slotAppendTag ( QString tag )
{
	emit newTag ( tag );
	m_lists->reloadTagsCombo();
}

void MainViewWidget::activation(QList< FontItem * > fit, bool act)
{
	// First check if one of the font is in a different state than required
	QList< FontItem * > actualF;
	for(int i(0); i < fit.count(); ++i)
	{
		if(fit[i]->isActivated() != act)
			actualF.append(fit[i]);
	}
	if(actualF.count() == 0)
		return;

	// TODO check for duplicates before we activate them.

	// we tr("purge") errors;
	FMActivate::getInstance()->errors();
	FMActivate::getInstance()->activate(actualF, act);
	QMap<QString,QString> actErr(FMActivate::getInstance()->errors());
	if(actErr.count() > 0)
	{
		FMActivationReport ar(this, actErr);
		ar.exec();
	}

//	updateTree(true);
}

void MainViewWidget::slotDesactivateAll()
{
	activation(currentFonts, false);
}

void MainViewWidget::slotActivateAll()
{
	activation(currentFonts, true);
}

void MainViewWidget::slotSetSampleText ( QString s )
{
	sampleText = s ;
	slotView ( true );

}

void MainViewWidget::slotActivate ( bool act, QTreeWidgetItem * item, int column )
{
	if ( column >2 ) return;
	FontItem * FoIt = FMFontDb::DB()->Font( item->text ( 1 ) );
	if ( FoIt )
	{
		QList<FontItem*> fl;
		fl.append(FoIt);
		activation ( fl, act );
	}
}

void MainViewWidget::slotReloadFontList()
{
	currentFonts.clear();
	currentFonts = FMFontDb::DB()->AllFonts();
	fontsetHasChanged = true;
	fillTree();
}

void MainViewWidget::slotSwitchAntiAlias ( bool aa )
{
	loremView->setRenderHint ( QPainter::Antialiasing, aa );
}


void MainViewWidget::slotViewAll()
{
	fontsetHasChanged = true;
	currentFonts = FMFontDb::DB()->AllFonts();
	fillTree();
	setCrumb();
}

void MainViewWidget::slotViewActivated()
{
// 	slotFilterTag ( "Activated_On" );
}

void MainViewWidget::fillUniPlanesCombo ( FontItem* item )
{
	QString stickyRange(uniPlaneCombo->currentText());
// 	qDebug()<<"STiCKyRaNGe :: "<<stickyRange;
	int stickyIndex(0);

	uniPlaneCombo->clear();
	
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
			uniPlaneCombo->addItem ( block );
			if(block == stickyRange)
			{
				stickyIndex = uniPlaneCombo->count() - 1;
				uRangeIsNotEmpty = true;
			}
		}
		else
		{
			if(block == stickyRange)
			{
				stickyIndex = uniPlaneCombo->count() - 1;
				uRangeIsNotEmpty = true;
			}
		}
		
	} while(lastBlock != block);
	if(item->countCoverage ( -1 , 100 ) > 0)
	{
		uniPlaneCombo->addItem( unMapGlyphName );
		if(unMapGlyphName == stickyRange)
		{
			stickyIndex = uniPlaneCombo->count() - 1;
			uRangeIsNotEmpty = true;
		}
	}
	uniPlaneCombo->addItem( allMappedGlyphName );
	if(allMappedGlyphName == stickyRange)
	{
		stickyIndex = uniPlaneCombo->count() - 1;
		uRangeIsNotEmpty = true;
	}
	
	uniPlaneCombo->setCurrentIndex ( stickyIndex );

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
		FMOtf * otf = theVeryFont->takeOTFInstance();
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
						if(table == "GPOS")
						{
							if(typo->getDefaultOTFScript() == script && typo->getDefaultOTFLang() == lang && typo->getDefaultOTFGPOS().contains(feature) )
							{
								feature_item->setCheckState ( 0, Qt::Checked );
							}
						}
						else if(table == "GSUB")
						{
							if(typo->getDefaultOTFScript() == script && typo->getDefaultOTFLang() == lang && typo->getDefaultOTFGSUB().contains(feature) )
							{
								feature_item->setCheckState ( 0, Qt::Checked );
							}
						}
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


void MainViewWidget::slotDefaultOTF()
{
	OTFSet ots(deFillOTTree());
	
	typo->setDefaultOTFScript(ots.script);
	typo->setDefaultOTFLang(ots.lang);
	typo->setDefaultOTFGPOS(ots.gpos_features);
	typo->setDefaultOTFGSUB(ots.gsub_features);
}

void MainViewWidget::slotResetOTF()
{
	typo->setDefaultOTFScript(QString());
	typo->setDefaultOTFLang(QString());
	typo->setDefaultOTFGPOS(QStringList());
	typo->setDefaultOTFGSUB(QStringList());
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

#define MAX_PALYSTRING_LEN 30

void MainViewWidget::refillSampleList()
{
	sampleTextTree->clear();

	QTreeWidgetItem * curIt = 0;
	QMap<QString, QList<QString> > sl = typo->namedSamplesNames();
	QList<QString> ul( sl.take(QString("User")) );
	if(ul.count())
	{
		QTreeWidgetItem * uRoot = new QTreeWidgetItem(sampleTextTree);
		//: Identify root of user defined sample texts
		uRoot->setText(0, tr("User"));
		bool first(true);
		foreach(QString uk, ul)
		{
			if(first)
			{
				first = false;
				uRoot->setData(0, Qt::UserRole , QString("User::") + uk);
				curIt = uRoot;
			}
			QTreeWidgetItem * it = new QTreeWidgetItem();
			it->setText(0, uk);
			it->setData(0, Qt::UserRole , QString("User::") + uk);
			uRoot->addChild(it);
		}
	}
	foreach(QString k, sl.keys())
	{
		QTreeWidgetItem * kRoot = new QTreeWidgetItem(sampleTextTree);
		kRoot->setText(0, k);
		bool first(true);
		foreach(QString n, sl[k])
		{
			if(first)
			{
				first = false;
				kRoot->setData(0, Qt::UserRole , k + QString("::") + n);
				if(!curIt)
					curIt = kRoot;
			}
			QTreeWidgetItem * it = new QTreeWidgetItem();
			it->setText(0, n);
			it->setData(0, Qt::UserRole, k + QString("::") + n);
			kRoot->addChild(it);
		}
	}

	sampleTextTree->setCurrentItem(curIt);
}

void MainViewWidget::slotFTRasterChanged()
{
// 	fitViewCheck->setChecked(false);
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


void MainViewWidget::slotPlaneSelected ( int i )
{
	qDebug()<<"slotPlaneSelected"<<i<<uniPlaneCombo->currentIndex();
	if(i != uniPlaneCombo->currentIndex())
		uniPlaneCombo->setCurrentIndex(i);
	
	bool stickState = uRangeIsNotEmpty;
	uRangeIsNotEmpty = true;
	slotShowAllGlyph();
	slotUpdateGView();
	if( (stickState == false) && theVeryFont)
	{
		fillUniPlanesCombo(theVeryFont);
	}
	abcView->verticalScrollBar()->setValue ( 0 );
}

void MainViewWidget::slotSearchCharName()
{	
	if(!theVeryFont)
		return;
	QString name(charSearchLine->text());
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
			int idx(uniPlaneCombo->findText(key));
			slotPlaneSelected(idx);
			int sv(0);
			bool first(true);
			do{
				if(first)
					first = false;
				else
				{
					abcView->verticalScrollBar()->setValue(sv + abcView->height());
					sv = abcView->verticalScrollBar()->value();
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
			}while(sv < abcView->verticalScrollBar()->maximum());
			return;
		}
	}
	
	// if user was looking for a name and we did not find it in
	// Unicode blocks, it must be unmapped.
	if(!searchCodepoint)
	{
		int idx(uniPlaneCombo->findText(unMapGlyphName));
		slotPlaneSelected(idx);
		int sv(0);
		bool first(true);
		do{
			if(first)
				first = false;
			else
			{
				abcView->verticalScrollBar()->setValue(sv + abcView->height());
				sv = abcView->verticalScrollBar()->value();
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
		}while(sv < abcView->verticalScrollBar()->maximum());
		return;
	}
	
}

void MainViewWidget::slotShowOneGlyph()
{
	qDebug() <<"slotShowOneGlyph()"<<abcScene->selectedItems().count();
	if ( abcScene->selectedItems().isEmpty() )
		return;
	if ( abcView->lock() )
	{
		curGlyph = reinterpret_cast<QGraphicsRectItem*> ( abcScene->selectedItems().first() );
		curGlyph->setSelected ( false );
		if ( fancyGlyphInUse < 0 )
		{
			if ( curGlyph->data ( 3 ).toInt() > 0 ) // Is a codepoint
			{
				fancyGlyphData = curGlyph->data ( 3 ).toInt();
				if(clipboardCheck->isChecked())
				{
					new FMGlyphHighlight(abcScene, curGlyph->rect());
					QString simpleC;
					simpleC += QChar(fancyGlyphData);
					QApplication::clipboard()->setText(simpleC, QClipboard::Clipboard);
					uniLine->setText(uniLine->text() + simpleC);
				}
				else
					fancyGlyphInUse = theVeryFont->showFancyGlyph ( abcView, fancyGlyphData );
			}
			else // Is a glyph index
			{
				fancyGlyphData = curGlyph->data ( 2 ).toInt();
				fancyGlyphInUse = theVeryFont->showFancyGlyph ( abcView, fancyGlyphData , true );
			}
			if ( fancyGlyphInUse < 0 )
			{
				abcView->unlock();
				return;
			}
			abcView->setState ( FMGlyphsView::SingleView );
		}
		abcView->unlock();
	}
	else
		qDebug()<<"cannot lock ABCview";
}

void MainViewWidget::slotShowAllGlyph()
{
// 	qDebug() <<"slotShowAllGlyph()";
	if ( fancyGlyphInUse < 0 )
		return;
	if ( abcView->lock() )
	{
// 		qDebug()<<"View Locked";
		theVeryFont->hideFancyGlyph ( fancyGlyphInUse );
		fancyGlyphInUse = -1;
		abcView->setState ( FMGlyphsView::AllView );

		abcView->unlock();
	}
// 	qDebug() <<"ENDOF slotShowAllGlyph()";
}

void MainViewWidget::slotUpdateGView()
{
// 	qDebug()<<"slotUpdateGView"<<uniPlaneCombo->currentText();
// 	printBacktrace(32);
	// If all is how I think it must be, we don’t need to check anything here :)
	if(theVeryFont && abcView->lock())
	{
		QPair<int,int> uniPair;
		QString curBlockText(uniPlaneCombo->currentText());
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
		unicodeCoverageStat->setText ( statstring );

		theVeryFont->renderAll ( abcScene , uniPair.first, uniPair.second );
		abcView->unlock();
	}
}


void MainViewWidget::slotUpdateGViewSingle()
{
// 	qDebug()<<"slotUpdateGViewSingle";
	if ( theVeryFont && abcView->lock())
	{
// 			qDebug() <<"1.FGI"<<fancyGlyphInUse;
			theVeryFont->hideFancyGlyph ( fancyGlyphInUse );
			if ( fancyGlyphData > 0 ) // Is a codepoint
			{
				fancyGlyphInUse = theVeryFont->showFancyGlyph ( abcView, fancyGlyphData );
// 				qDebug() <<"2.FGI"<<fancyGlyphInUse;
			}
			else // Is a glyph index
			{
				fancyGlyphInUse = theVeryFont->showFancyGlyph ( abcView, fancyGlyphData , true );
// 				qDebug() <<"3.FGI"<<fancyGlyphInUse;
			}
			abcView->unlock();

	}

}



void MainViewWidget::slotUpdateSView()
{
	if(loremView->isVisible())
		slotView(true);
}

void MainViewWidget::slotUpdateRView()
{
	if(loremView_FT->isVisible())
		slotView(true);
}


void MainViewWidget::slotUpdateTree()
{
	updateTree(true);
}


void MainViewWidget::slotEditSampleText()
{
	typo->slotPrefsPanel(PrefsPanelDialog::PAGE_SAMPLETEXT);
}

void MainViewWidget::slotRemoveCurrentItem()
{
	if(curItemName.isEmpty())
		return;
	if(theVeryFont->isActivated())
	{
		QMessageBox::information(this, tr("Fontmatrix takes care of you"), curItemName + tr(" is activated.\nIf you want to remove it from Fontmatrix database, please deactivate it first."), QMessageBox::Yes );
		return;
	}
	if( QMessageBox::question ( this, tr("Fontmatrix safe"), tr("You are about to remove a font from Fontmatrix database") +"\n"+curItemName+"\n" + tr("Do you want to continue?"),QMessageBox::Yes |  QMessageBox::No, QMessageBox::No) == QMessageBox::Yes )
	{
		theVeryFont->deRenderAll();
		currentFonts.removeAll(theVeryFont);
		TagsWidget::getInstance()->removeFromTagged(theVeryFont);
		theVeryFont  = 0 ;
		typo->removeFontItem(curItemName);
		curItemName = lastIndex = faceIndex = "";
		fontsetHasChanged = true;
		fillTree();
	}
}

void MainViewWidget::slotLiveFontSize()
{
	double fs( liveFontSizeSpin->value() );
	reSize(fs, fs * sampleRatio);
	slotView(true);
}

void MainViewWidget::slotRemoteFinished()
{
	qDebug()<<"slotRemoteFinished : "<<currentDownload;
	slotFontSelectedByName(currentDownload);
	slotInfoFont();
	slotUpdateGView();
	slotUpdateRView();
	slotUpdateSView();

}

void MainViewWidget::slotProgressionChanged()
{
	slotView(true);
}

QString MainViewWidget::sampleName()
{
	QString ret( sampleTextTree->currentItem()->data(0, Qt::UserRole).toString() );
	if (ret.isEmpty())
		ret = typo->defaultSampleName();
	return ret;
}

void MainViewWidget::displayWelcomeMessage()
{
	if(!typotek::getInstance()->welcomeURL().isEmpty())
	{
		fontInfoText->load(QUrl(typotek::getInstance()->welcomeURL()));
		return;
	}
	
	QString welcomeFontName;
	QString welcomeSVG;
	if(FMFontDb::DB()->FontCount() > 0)
	{
			QList<FontItem*> fl(FMFontDb::DB()->AllFonts());
			QString welcomeString(tr("Welcome to Fontmatrix") );
			if(fl.count() > 0)
			{
				int flcount(fl.count());
				int rIdx( std::rand() % flcount );
				QList<int> triedFont;
				while(triedFont.count() < flcount)
				{
					while(triedFont.contains(rIdx))
						rIdx = std::rand() %  flcount;
					triedFont << rIdx;
					FontItem * f(fl[rIdx]);
					if(f->hasChars(welcomeString))
					{
						QStringList wList(welcomeString.split(" "));
						foreach(const QString& wPart, wList)
						{
							welcomeSVG += "<div>";
							welcomeSVG += f->renderSVG( wPart , QString(QSettings().value("General/WelcomeSize", tr("122.0", "Size of the welcome message" )).toString()).toDouble());
							welcomeSVG += "</div>";
						}
						welcomeFontName = f->fancyName();
						break;
					}	
				}
			}
	}
// 	QString ResPat(FMPaths::ResourcesDir());
// 	QFile wFile( ResPat + "welcome_"+ FMPaths::sysLoc() + ".html");
// 	wFile.open(QIODevice::ReadOnly | QIODevice::Text);
// 	QByteArray wArray(wFile.readAll());
// 	QString wString(QString::fromUtf8(wArray.data(),wArray.length()));
// 	wString.replace("##RECOURCES_DIR##", QUrl::fromLocalFile(ResPat).toString() );
// 	wString.replace("##WELCOME_MESSAGE##", welcomeSVG);
// 	wString.replace("##WELCOME_FONT##", welcomeFontName);
	QString wString;
	wString += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" ;
	wString += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">" ;
	wString += "<html xmlns=\"http://www.w3.org/1999/xhtml\"><body style=\"background-color:#eee;\">" ;
	wString += welcomeSVG ;
	wString +="<div style=\"font-family:sans-serif;text-align:right;\">"+welcomeFontName+"</div>";
	wString += "</body></html>" ;
	fontInfoText->setContent(wString.toUtf8(), "application/xhtml+xml");
}

// QTextDocument * MainViewWidget::infoDocument()
// {
// 	return fontInfoText->document();
// }

QGraphicsScene * MainViewWidget::currentSampleScene()
{
	if(!loremView->isVisible())
	{
		tabWidget->setCurrentIndex(1);
		stackedViews->setCurrentIndex(VIEW_PAGE_ABSOLUTE);
		slotView(true);
	}
	return loremScene;
}

FMPlayGround * MainViewWidget::getPlayground()
{
	return playView;
}

void MainViewWidget::addFilterToCrumb(QString filter)
{
	QString t(m_lists->filtersCrumb->text());
	t += "[" + filter.trimmed() + "]";
	m_lists->filtersCrumb->setText(t);
}

void MainViewWidget::setCrumb(QString text)
{
	m_lists->filtersCrumb->setText(text);
}

// Don’t know if it’s really useful
// It will be used for track down problems at least
void MainViewWidget::slotSelectFromFolders(const QString &f)
{
// 	qDebug()<<"or even this other";
	slotFontSelectedByName(f);
}

QWebView * MainViewWidget::info()
{
	return fontInfoText;
}

void MainViewWidget::slotChangeViewPageSetting ( bool ch )
{
// 	qDebug() <<"MainViewWidget::slotChangeViewPageSetting("<<ch<<")";
	QString butName ( sender()->objectName() );
	if ( !ch )
	{
		toolPanelWidth = splitter_2->sizes().at ( 1 ) ;
		stackedTools->hide();
	}
	else
	{
		stackedTools->show();
		if ( splitter_2->sizes().at ( 1 ) == 0 )
		{
			QList<int> li;
			li << splitter_2->width() - toolPanelWidth << toolPanelWidth;
			splitter_2->setSizes ( li );
		}
	}

	QMap<QString, QToolButton*> bmap;
	QMap<QString, int> pmap;
	bmap[ "settingsButton" ] = settingsButton;
	bmap[ "openTypeButton" ] = openTypeButton;
	bmap[ "sampleButton" ] = sampleButton;
	pmap[ "settingsButton" ] = VIEW_PAGE_SETTINGS;
	pmap[ "openTypeButton" ] = VIEW_PAGE_OPENTYPE;
	pmap[ "sampleButton" ] = VIEW_PAGE_SAMPLES;
	
	foreach(QString pk, bmap.keys())
	{
		if(butName == pk)
		{
			stackedTools->setCurrentIndex(pmap[pk]);
		}
		else
		{
			bmap[pk]->setChecked ( false );
		}
	}
}

void MainViewWidget::slotChangeViewPage(QAbstractButton* but)
{
	QString radioName( but->objectName() );
	
	if(radioName == "freetypeRadio" )
	{
		stackedViews->setCurrentIndex(VIEW_PAGE_FREETYPE);
		hintingSelect->setEnabled(true);
	}
	else if(radioName == "nativeRadio" )
	{
		stackedViews->setCurrentIndex(VIEW_PAGE_ABSOLUTE);
		hintingSelect->setEnabled(false);
	}
	
	slotView(true);
}

void MainViewWidget::saveSplitterState()
{
	QSettings settings;
	settings.setValue( "WState/SplitterViewState", splitter_2->saveState());
	settings.setValue( "WState/SplitterList1", ListDockWidget::getInstance()->listSplit1->saveState());
	settings.setValue( "WState/SplitterList2", ListDockWidget::getInstance()->listSplit2->saveState());
}

void MainViewWidget::restoreSplitterState()
{
	QSettings settings;
	splitter_2->restoreState(settings.value("WState/SplitterViewState").toByteArray());
	ListDockWidget::getInstance()->listSplit1->restoreState(settings.value("WState/SplitterList1").toByteArray());
	ListDockWidget::getInstance()->listSplit2->restoreState(settings.value("WState/SplitterList2").toByteArray());
}

unsigned int MainViewWidget::hinting()
{
	if(lightHinting->isChecked())
		return FT_LOAD_TARGET_LIGHT;
	else if(normalHinting->isChecked())
		return FT_LOAD_TARGET_NORMAL;
		
	return FT_LOAD_NO_HINTING ;
}

void MainViewWidget::slotHintChanged(int )
{
	slotView(true);
}

void MainViewWidget::slotWebStart()
{
// 	qDebug()<<"slotWebStart";
	typo->startProgressJob(100);
}

void MainViewWidget::slotWebFinished(bool status)
{
// 	qDebug()<<"slotWebinished"<<status;
	typo->endProgressJob();
}

void MainViewWidget::slotWebLoad(int i)
{
// 	qDebug()<<"slotWebLoad("<<i<<")";
	typo->runProgressJob(i);
}

void MainViewWidget::slotWebLink(const QUrl & url)
{
	if(typo->getWebBrowser() == QString( "Fontmatrix" ))
	{
		typo->showStatusMessage(tr("Load") + " " + url.toString());
		fontInfoText->load(url);
	}
	else
	{
		QStringList arguments(typo->getWebBrowserOptions().split(" ", QString::SkipEmptyParts));
		arguments << url.toString();

		if(!QProcess::startDetached(typo->getWebBrowser(), arguments))
		{
			arguments.removeLast();
			QMessageBox::warning(this,"Fontmatrix", QString(tr("An error occured when tried to load %1\nwith command: %2", "%1 is an url and %2 a program")).arg(url.toString()).arg( typo->getWebBrowser() + " " + arguments.join(" ")));
		}
	}
}

QList<FontItem*> MainViewWidget::curFonts()
{
// 	qDebug()<<"curFonts"<<currentFonts.count();
// 	return currentFonts;
	// #12231 
	return orderedCurrentFonts;
}

void MainViewWidget::setCurFonts(QList< FontItem * > flist)
{
	currentFonts = flist;
	fillTree();
	
}


void MainViewWidget::slotShowULine(bool checked)
{
	if(checked)
	{
		uniLine->setText("");
		uniLine->setEnabled(true);
	}
	else
	{
		uniLine->setEnabled(false);
	}
}

void MainViewWidget::slotShowClassification()
{
// 	qDebug()<<"MainViewWidget::slotShowClassification";
	// Rather testing for now
	classVariableDescription->setHtml(FontStrings::PanoseKeyInfo(FontStrings::firstPanoseKey()));
	ParallelCoorDataType pcdata;
	QList<FontDBResult> dbresult( FMFontDb::DB()->getValues(currentFonts, FMFontDb::Panose) );
	for(int i(0); i < dbresult.count() ; ++i)
	{
		QList<int> list;
		QString p(dbresult[i].second);
		QStringList pl(p.split(":"));
		if(pl.count() == 10)
		{
			for(int a(0);a < 10 ;++a)
			{
				list << pl[a].toInt();
			}
			pcdata << list;
		}
	}
	
	ParallelCoorDataSet* pcs(new ParallelCoorDataSet);
	QMap< FontStrings::PanoseKey, QMap<int, QString> > pan(FontStrings::Panose());
	FontStrings::PanoseKey k(FontStrings::firstPanoseKey());
	while(k != FontStrings::InvalidPK)
	{
		QString key( FontStrings::PanoseKeyName(k) );
// 		qDebug()<<key;
		QList<QString> list;
		for(QMap<int, QString>::iterator i( pan[k].begin() ); i != pan[k].end(); ++i)
		{
// 			qDebug()<<i.value() ;
			list << i.value() ;
		}
		pcs->append( qMakePair(key, list) );
		k = FontStrings::nextPanoseKey(k);
	}
	
	
	pcs->setData(pcdata);
	classificationView->setDataSet(pcs);
// 	classificationView->updateGraphic();
	
}

void MainViewWidget::slotUpdateClassDescription(const QString & ks)
{
	FontStrings::PanoseKey pk(FontStrings::firstPanoseKey());
	do
	{
		if(FontStrings::PanoseKeyName(pk) == ks)
			break;
		pk = FontStrings::nextPanoseKey(pk);
	}
	while(pk != FontStrings::InvalidPK);
	
	classVariableDescription->setHtml(FontStrings::PanoseKeyInfo(pk));
}

void MainViewWidget::slotPanoseFilter()
{
	QList<FontDBResult> dbresult( FMFontDb::DB()->getValues(currentFonts, FMFontDb::Panose) );
	QList<FontItem*> fil;
	for(int i(0); i < dbresult.count() ; ++i)
	{
		QList<int> list;
		QString p(dbresult[i].second);
		QStringList pl(p.split(":"));
		if(pl.count() == 10)
		{
			for(int a(0);a < 10 ;++a)
			{
				list << pl[a].toInt();
			}
			if(classificationView->matchFilter(list))
				fil << dbresult[i].first;
		}
	}
	currentFonts = fil;
	setCrumb(classificationView->filterAsString());
	fillTree();
}

void MainViewWidget::slotSaveClassSplitter()
{
	QSettings settings;
	settings.setValue("WState/ClassificationSplitter", classSplitter->saveState());
}

void MainViewWidget::toggleFacesCheckBoxes(bool state)
{
	if(state == activateByFamilyOnly)
	{
		activateByFamilyOnly = !state;
		fillTree();
	}
}

void MainViewWidget::forceReloadSelection()
{
	m_forceReloadSelection = true;
}

double MainViewWidget::playgroundFontSize()
{
	return playFontSize->value();
}

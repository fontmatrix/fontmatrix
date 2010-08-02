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
#include "fmmissingfonthelper.h"
#include "fmotf.h"
#include "fmpaths.h"
#include "fmpreviewlist.h"
#include "fmuniblocks.h"
#include "fontitem.h"
#include "listdockwidget.h"
//#include "opentypetags.h"
#include "panosematch.h"
#include "systray.h"
#include "typotek.h"
#include "fmfontdb.h"
#include "fmfontstrings.h"
#include "tagswidget.h"
#include "fmutils.h"
#include "panosewidget.h"

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
#include <QTimer>
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
	quickSearchWidget->setVisible(false);

	quickSearchWait = 4000;
	quickSearchTimer = 0;
	m_forceReloadSelection = false;
	FMFontDb::DB()->clearFilteredFonts();

	listView->setNumCol(4);
	listView->setModelColumn(1);
	listView->setViewMode(QListView::IconMode);
	listView->setIconSize(QSize(qRound(listView->width() ), 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
	listView->setUniformItemSizes(true);
	listView->setMovement(QListView::Static);

	previewModel = new FMPreviewModel( this, listView );
	previewModel->setSpecString("<family>");
	previewModel->setFamilyMode(true);
	listView->setModel(previewModel);



	iconPS1 =  QIcon(":/icon-PS1");
	iconTTF =  QIcon(":/icon-TTF");
	iconOTF =  QIcon(":/icon-OTF");

	
	theVeryFont = 0;
	typo = typotek::getInstance();
	m_lists = ListDockWidget::getInstance();
	// 	currentFonts = typo->getAllFonts();
	FMFontDb::DB()->filterAllFonts();
	fontsetHasChanged = true;

	QSettings settings;
	activateByFamilyOnly = settings.value("ActivateOnlyFamily", false).toBool();
	m_lists->actFacesButton->setChecked(!activateByFamilyOnly);

	// 	fillUniPlanes();
	//	refillSampleList();

	//	fontInfoText->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);



	sampleText= typo->namedSample();
	currentOrdering = "family" ;


	
	doConnect();
	listView->setFocus(Qt::OtherFocusReason);
}


MainViewWidget::~MainViewWidget()
{
}


void MainViewWidget::doConnect()
{
	connect(FMActivate::getInstance(), SIGNAL(activationEvent(const QStringList&)), this, SLOT(refreshActStatus(const QStringList&)));

	connect ( m_lists->fontTree,SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ),this,SLOT ( slotFontSelected ( QTreeWidgetItem*, int ) ) );
	connect ( m_lists->fontTree,SIGNAL ( currentChanged (QTreeWidgetItem*, int ) ), this,SLOT (slotFontSelected ( QTreeWidgetItem*, int ) ) );
	connect(filterBar, SIGNAL(initSearch(int,QString)), this, SLOT(slotSearch(int,QString)));
	connect ( m_lists->fontTree,SIGNAL ( itemExpanded ( QTreeWidgetItem* ) ),this,SLOT ( slotItemOpened ( QTreeWidgetItem* ) ) );
	connect ( m_lists, SIGNAL(folderSelectFont(const QString&)), this, SLOT(slotSelectFromFolders(const QString&)));
	connect(familyWidget, SIGNAL(familyStateChanged()), previewModel, SLOT(dataChanged()));
	connect ( m_lists->actFacesButton, SIGNAL(toggled( bool )), this, SLOT(toggleFacesCheckBoxes(bool)) );

	connect ( this, SIGNAL(listChanged()), previewModel, SLOT(dataChanged()));
	connect ( this, SIGNAL(listChanged()), typo, SLOT(showToltalFilteredFonts()));
	connect(filterBar,SIGNAL(filterChanged()),previewModel,SLOT(dataChanged()));
	connect(familyWidget, SIGNAL(tagAdded()), filterBar, SLOT(loadTags()));
	connect(familyWidget, SIGNAL(tagChanged()), filterBar, SLOT(loadTags()));


	connect(listView, SIGNAL(widthChanged(int)),this,SLOT(slotPreviewUpdateSize(int)));
	connect(listView, SIGNAL(activated(const QModelIndex&)), this, SLOT(slotShowFamily(const QModelIndex&)));
	connect(familyWidget, SIGNAL(backToList()), this, SLOT(slotQuitFamily()));
	connect(familyWidget, SIGNAL(fontSelected(const QString&)), this, SLOT(slotFontSelectedByName(const QString&)));

	connect(quickSearch, SIGNAL(textEdited(QString)), this, SLOT(slotQuickSearch(QString)));
}

void MainViewWidget::disConnect()
{
	disconnect(FMActivate::getInstance(), SIGNAL(activationEvent(const QStringList&)), this, SLOT(refreshActStatus(const QStringList&)));

	disconnect ( m_lists->fontTree,SIGNAL ( itemClicked ( QTreeWidgetItem*, int ) ),this,SLOT ( slotFontSelected ( QTreeWidgetItem*, int ) ) );
	disconnect ( m_lists->fontTree,SIGNAL ( currentChanged (QTreeWidgetItem*, int ) ), this,SLOT (slotFontSelected ( QTreeWidgetItem*, int ) ) );
	disconnect(filterBar, SIGNAL(initSearch(int,QString)), this, SLOT(slotSearch(int,QString)));
	disconnect ( m_lists->fontTree,SIGNAL ( itemExpanded ( QTreeWidgetItem* ) ),this,SLOT ( slotItemOpened ( QTreeWidgetItem* ) ) );
	disconnect ( m_lists, SIGNAL(folderSelectFont(const QString&)), this, SLOT(slotSelectFromFolders(const QString&)));
	disconnect(familyWidget, SIGNAL(familyStateChanged()), previewModel, SLOT(dataChanged()));
	disconnect ( m_lists->actFacesButton, SIGNAL(toggled( bool )), this, SLOT(toggleFacesCheckBoxes(bool)) );

	disconnect ( this, SIGNAL(listChanged()), previewModel, SLOT(dataChanged()));
	disconnect ( this, SIGNAL(listChanged()), typo, SLOT(showToltalFilteredFonts()));
	disconnect(filterBar,SIGNAL(filterChanged()),previewModel,SLOT(dataChanged()));
	disconnect(familyWidget, SIGNAL(tagAdded()), filterBar, SLOT(loadTags()));
	disconnect(familyWidget, SIGNAL(tagChanged()), filterBar, SLOT(loadTags()));
	
	//	disconnect( filterBar , SIGNAL(panoseFilter(QMap<int,QList<int> >)), this, SLOT(slotPanoseFilter(QMap<int,QList<int> >)));

	disconnect(listView, SIGNAL(widthChanged(int)),this,SLOT(slotPreviewUpdateSize(int)));
	disconnect(listView, SIGNAL(activated(const QModelIndex&)), this, SLOT(slotShowFamily(const QModelIndex&)));
	disconnect(familyWidget, SIGNAL(backToList()), this, SLOT(slotQuitFamily()));
	disconnect(familyWidget, SIGNAL(fontSelected(const QString&)), this, SLOT(slotFontSelectedByName(const QString&)));

	disconnect(quickSearch, SIGNAL(textEdited(QString)), this, SLOT(slotQuickSearch(QString)));
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
	QList<FontItem*> currentFonts(FMFontDb::DB()->getFilteredFonts());
	int cfi(currentFonts.count());
	for ( int i=0; i < cfi;++i )
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
        qDebug() << "font select"<<item;
	if ( item->data ( 0,100 ).toString() == "alpha" )
	{
		// 		qDebug() << "Item is an alpha";
		return;
		// 		fillTree();
	}

        else if ( item->data ( 0,100 ).toString() == "family" )
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
		//                if(!slotFontActionByNames ( names ))
		//                {
		//                    delete(new FMMissingFontHelper(names));
		//                    return;
		//                }
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

        else if ( item->data ( 0,100 ).toString() == "fontfile" )
	{
		QString fontname(item->toolTip(0));
		bool wantActivate = (item->checkState(0) == Qt::Checked) ? true : false;
		// 		m_lists->previewList->slotSelect(fontname);
                if(!slotFontSelectedByName(fontname))
                {
			delete(new FMMissingFontHelper(fontname));
			return;
                }
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

bool MainViewWidget::slotFontSelectedByName (const QString& fname )
{

	if ( fname.isEmpty()
		|| ((fname ==  faceIndex) && (!m_forceReloadSelection)) )
                return false;
	m_forceReloadSelection = false;
	lastIndex = faceIndex;
	faceIndex = fname;
	curItemName = faceIndex;

	{
		// 		qDebug() << "Font has changed \n\tOLD : "<<lastIndex<<"\n\tNEW : " << faceIndex ;

		theVeryFont = FMFontDb::DB()->Font( faceIndex );
                if(!theVeryFont)
			return false;
		// 		theVeryFont->updateItem();
		//		slotFontActionByName ( fname );
		//		if(theVeryFont->isRemote())
		//		{
		//			qDebug() << faceIndex <<" is remote";
		//			if(!theVeryFont->isCached())
		//			{
		//				connect(theVeryFont,SIGNAL(dowloadFinished()), this, SLOT(slotRemoteFinished()));
		//				theVeryFont->getFromNetwork();
		//				currentDownload = faceIndex ;
		//				faceIndex = lastIndex;
		//				return false;
		//			}
		//			else
		//			{
		//				currentDownload = "";
		//			}
		//		}
		//		fillOTTree();

		//		slotView ( true );
		typo->setWindowTitle ( theVeryFont->fancyName() + " - Fontmatrix" );
		m_lists->fontTree->headerItem()->setText(0, tr("Names")+" ("+theVeryFont->family()+")");
		typo->presentFontName ( theVeryFont->fancyName() );
		// 		fillTree();
		updateTree();
		//		m_lists->listPreview->setCurrentFont(theVeryFont->path());
	}

        return true;
}


//void MainViewWidget::slotInfoFont()
//{
//	if(theVeryFont)
//	{
//		FMInfoDisplay fid(theVeryFont);
//                fontInfoText->setContent(fid.getHtml().toUtf8(), "application/xhtml+xml");
//	}


//}

//void MainViewWidget::slotView ( bool needDeRendering )
//{
//	QTime t;
//	t.start();
//	FontItem *l = FMFontDb::DB()->Font( lastIndex );
//	FontItem *f = FMFontDb::DB()->Font( faceIndex );
//	if ( !f )
//		return;
//	if ( needDeRendering )
//	{
//		if ( l )
//		{
//			l->deRenderAll();
//		}
//		f->deRenderAll();

//		curGlyph = 0;
//	}

//	bool wantDeviceDependant = loremView_FT->isVisible();
//	unsigned int storedHinting(theVeryFont->getFTHintMode());
//	if(wantDeviceDependant)
//	{
//		theVeryFont->setFTHintMode(hinting());
//	}

//	if(textProgression->inLine() == TextProgression::INLINE_LTR )
//		theVeryFont->setProgression(PROGRESSION_LTR );
//	else if(textProgression->inLine() == TextProgression::INLINE_RTL )
//		theVeryFont->setProgression(PROGRESSION_RTL);
//	else if(textProgression->inLine() == TextProgression::INLINE_TTB )
//		theVeryFont->setProgression(PROGRESSION_TTB );
//	else if(textProgression->inLine() == TextProgression::INLINE_BTT )
//		theVeryFont->setProgression(PROGRESSION_BTT);

//	theVeryFont->setFTRaster ( wantDeviceDependant );
//	theVeryFont->setShaperType(shaperTypeCombo->itemData(  shaperTypeCombo->currentIndex() ).toInt() );

//	if ( loremView->isVisible() || loremView_FT->isVisible() )
//	{
////		qDebug()<<"lv(ft) is visible";
//		if(textLayout->isRunning())
//		{
////			qDebug()<<"tl is running";
//			textLayout->stopLayout();
//		}
//		else
//		{
////			qDebug()<<"tl is NOT running";
//			QGraphicsScene *targetScene;
//			loremView_FT->unSheduleUpdate();
//			loremView->unSheduleUpdate();
//			if(loremView->isVisible())
//			{
//				targetScene = loremScene;
//			}
//			else if(loremView_FT->isVisible())
//			{
//				targetScene = ftScene;
//			}

//			bool processFeatures = f->isOpenType() &&  !deFillOTTree().isEmpty();
//			QString script = langCombo->currentText();
//                        bool processScript =  f->isOpenType() && ( useShaperCheck->checkState() == Qt::Checked ) && ( !script.isEmpty() );

//			textLayout->setTheFont(theVeryFont);
//			textLayout->setDeviceIndy(!wantDeviceDependant);
//			textLayout->setTheScene(targetScene);
//			textLayout->setAdjustedSampleInter( sampleInterSize );

//			double fSize(sampleFontSize);

//			QList<GlyphList> list;
//			QStringList stl( typo->namedSample(sampleTextTree->currentItem()->data(0, Qt::UserRole).toString() ).split("\n"));
//			if ( processScript )
//			{
//				for(int p(0);p<stl.count();++p)
//				{
//					list << theVeryFont->glyphs( stl[p] , fSize, script );
//				}
//			}
//			else if(processFeatures)
//			{
//				// Experimental code to handle alternate is commented out
//				// Do not uncomment
////				FMAltContext * actx ( FMAltContextLib::SetCurrentContext(sampleTextTree->currentText(), theVeryFont->path()));
////				int rs(0);
////				actx->setPar(rs);
//				for(int p(0);p<stl.count();++p)
//				{
//					list << theVeryFont->glyphs( stl[p] , fSize, deFillOTTree());
////					actx->setPar(++rs);
//				}
////				actx->cleanup();
////				FMAltContextLib::SetCurrentContext(sampleTextTree->currentText(), theVeryFont->path());
//			}
//			else
//			{
//				for(int p(0);p<stl.count();++p)
//					list << theVeryFont->glyphs( stl[p] , fSize  );
//			}
//			textLayout->doLayout(list, fSize);
//// 			if (loremView->isVisible() /*&& fitViewCheck->isChecked()*/ )
//// 			{
//// 				loremView->fitInView ( textLayout->getRect(), Qt::KeepAspectRatio );
//// 			}
//			textLayout->start(QThread::LowestPriority);
//		}
//	}
//	else if(!loremView->isVisible() && !loremView_FT->isVisible())
//	{
//		loremView->sheduleUpdate();
//		loremView_FT->sheduleUpdate();
//	}

//	slotUpdateGView();
////	slotInfoFont();

//}


void MainViewWidget::slotSearch(int field, QString text)
{
	// 	qDebug()<<"slotSearch";
	m_lists->fontTree->clear();
	// 	m_lists->previewList->slotRefill(QList<FontItem*>(), true);
	fontsetHasChanged = true;

	QApplication::setOverrideCursor ( Qt::WaitCursor );
	QString fs ( text );

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
		allList = queue ? FMFontDb::DB()->getFilteredFonts() : FMFontDb::DB()->AllFonts();
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

void MainViewWidget::slotShowFamily(const QModelIndex& familyIdx)
{
	FontItem * fItem(FMFontDb::DB()->getFilteredFonts(true).at(familyIdx.row()));
	if(!fItem)
	{
		qDebug()<<"\t-FontItme invalid";
		return;
	}
	QList<FontItem*> fl(FMFontDb::DB()->FamilySet(fItem->family()));
	foreach(FontItem* f,fl)
	{
		qDebug() <<"F"<< f->fancyName();
	}
	familyWidget->setFamily(fItem->family());
	previewStack->setCurrentIndex(1);
}

void MainViewWidget::slotQuitFamily()
{
	previewStack->setCurrentIndex(0);
}

//void MainViewWidget::slotFilterTag ( QString tag )
//{
//	int tIdx(filterBar->tagsCombo()->currentIndex());
//	if(tIdx < 0)
//		return;

//	QString key(filterBar->tagsCombo()->itemData(tIdx).toString());

//	if(key == "TAG") // regular tag
//	{
//		m_lists->fontTree->clear();
//		fontsetHasChanged = true;
//		operateFilter( FMFontDb::DB()->Fonts(tag, FMFontDb::Tags ), tag);
//		currentOrdering = "family";
//		fillTree();
//	}
//	else if(key == "ALL_ACTIVATED")
//	{
//		m_lists->fontTree->clear();
//		fontsetHasChanged = true;
//		operateFilter( FMFontDb::DB()->Fonts(1, FMFontDb::Activation ), tr("Activated"));
//		currentOrdering = "family";
//		fillTree();
//	}
//	else if(key == "SIMILAR")
//	{
//		if(theVeryFont)
//		{
//			m_lists->fontTree->clear();
//			fontsetHasChanged = true;
//			operateFilter( PanoseMatchFont::similar(theVeryFont, typo->getPanoseMatchTreshold() ), "S://"+ theVeryFont->family());
//			fillTree();
//		}
//	}
//}

void MainViewWidget::operateFilter(QList< FontItem * > allFiltered, const QString filterName)
{
	QList<FontItem*> tmpList = allFiltered;
	QList<FontItem*> negList;
	QList<FontItem*> queList;	
	
	QStringList ops(m_lists->getOperation());
	bool negate(ops.contains("NOT"));
	bool queue(ops.contains("AND"));
	m_lists->clearOperation();
	
	FMFontDb* fmdb(FMFontDb::DB());

	if(queue)
	{
		addFilterToCrumb((negate?"!":"") +filterName);
		queList = fmdb->getFilteredFonts();
	}
	else
	{
		setCrumb();
		addFilterToCrumb((negate?"!":"") +filterName);
	}
	if(negate)
		negList = fmdb->AllFonts();

	fmdb->clearFilteredFonts();
	
	if(negate)
	{
		if(queue)
		{
			foreach(FontItem* f, negList)
			{
				if(!fmdb->isFiltered(f) && !tmpList.contains(f) && queList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else // not queue
		{
			foreach(FontItem* f, negList)
			{
				if(!fmdb->isFiltered(f) && !tmpList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
	}
	else // not negate
	{
		if(queue)
		{
			foreach(FontItem* f, tmpList)
			{
				if(!fmdb->isFiltered(f) && queList.contains(f))
					fmdb->insertFilteredFont(f);
			}
		}
		else // not queue
		{
			foreach(FontItem* f, tmpList)
			{
				if(!fmdb->isFiltered(f))
					fmdb->insertFilteredFont(f);
			}
		}
	}	
}


//void MainViewWidget::slotFontAction ( QTreeWidgetItem * item, int column )
//{
//// 	qDebug()<<"MainViewWidget::slotFontAction";
//	if ( column >2 ) return;

//	FontItem * FoIt = FMFontDb::DB()->Font( item->text ( 1 ) );
//	if ( FoIt/* && (!FoIt->isLocked())*/ )
//	{
//		QList<FontItem*> fl;
//		fl.append ( FoIt );
//		familyWidget->tagWidget()->prepare ( fl );
//	}
//}

//bool MainViewWidget::slotFontActionByName (const QString &fname )
//{
//// 	qDebug()<<"MainViewWidget::slotFontActionByName ("<< fname <<")";
//	FontItem * FoIt = FMFontDb::DB()->Font( fname );
//	if ( FoIt/* && (!FoIt->isLocked())*/ )
//	{
//		QList<FontItem*> fl;
//		fl.append ( FoIt );
//		familyWidget->tagWidget()->prepare ( fl );
//	}
//        else
//            return false;
//        return true;
//}

//bool MainViewWidget::slotFontActionByNames ( QStringList fnames )
//{
//// 	qDebug()<<"MainViewWidget::slotFontActionByNames ("<< fnames.join(";") <<")";
//	QList<FontItem*> FoIt;
//	for ( int i= 0; i < fnames.count() ; ++i )
//	{
//                FontItem* ti(FMFontDb::DB()->Font( fnames[i] ));
//                if(ti)
//                    FoIt.append ( ti );
//                else
//                {
//                    return false;
//                }
//	}
//	if ( FoIt.count() )
//		familyWidget->tagWidget()->prepare ( FoIt );
//        return true;
//}


//void MainViewWidget::slotEditAll()
//{
////	QList<FontItem*> fl;
////	for ( int i =0; i< currentFonts.count(); ++i )
////	{
////		fl.append ( currentFonts[i] );
////	}
//	if ( FMFontDb::DB()->countFilteredFonts() == 0 )
//		return;

//	familyWidget->tagWidget()->prepare ( FMFontDb::DB()->getFilteredFonts() );
//}



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
	activation(FMFontDb::DB()->getFilteredFonts(), false);
}

void MainViewWidget::slotActivateAll()
{
	activation(FMFontDb::DB()->getFilteredFonts(), true);
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
	FMFontDb::DB()->filterAllFonts();
	fontsetHasChanged = true;
	fillTree();
}


void MainViewWidget::slotViewAll()
{
	FMFontDb::DB()->filterAllFonts();
	//	filterBar->tagsCombo()->setCurrentIndex(0);
	fontsetHasChanged = true;
	fillTree();
	setCrumb();
}

void MainViewWidget::slotViewActivated()
{
	// 	slotFilterTag ( "Activated_On" );
}

void MainViewWidget::keyPressEvent ( QKeyEvent * e )
{
	qDebug() << " MainViewWidget::keyPressEvent(QKeyEvent * "<<e<<")";
	if(e->text().isEmpty())
		return;
	slotQuickSearch(e->text());
}



#define MAX_PALYSTRING_LEN 30

void MainViewWidget::slotFTRasterChanged()
{
	// 	fitViewCheck->setChecked(false);
	//	slotView ( true );
}


void MainViewWidget::slotUpdateTree()
{
	updateTree(true);
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
		FMFontDb::DB()->removeFilteredFont(theVeryFont);
		familyWidget->tagWidget()->removeFromTagged(theVeryFont);
		theVeryFont  = 0 ;
		typo->removeFontItem(curItemName);
		curItemName = lastIndex = faceIndex = "";
		fontsetHasChanged = true;
		fillTree();
	}
}


void MainViewWidget::slotRemoteFinished()
{
	qDebug()<<"slotRemoteFinished : "<<currentDownload;
	slotFontSelectedByName(currentDownload);
	//	slotInfoFont();
	//	slotUpdateGView();
	//	slotUpdateRView();
	//	slotUpdateSView();

}



QString MainViewWidget::sampleName()
{
	QString ret/*( sampleTextTree->currentItem()->data(0, Qt::UserRole).toString() )*/;
	if (ret.isEmpty())
		ret = typo->defaultSampleName();
	return ret;
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
	return familyWidget->info();
}




void MainViewWidget::saveSplitterState()
{
	QSettings settings;
	//	settings.setValue( "WState/SplitterViewState", splitter_2->saveState());
	settings.setValue( "WState/SplitterList1", ListDockWidget::getInstance()->listSplit1->saveState());
	settings.setValue( "WState/SplitterList2", ListDockWidget::getInstance()->listSplit2->saveState());
}

void MainViewWidget::restoreSplitterState()
{
	QSettings settings;
	//	splitter_2->restoreState(settings.value("WState/SplitterViewState").toByteArray());
	ListDockWidget::getInstance()->listSplit1->restoreState(settings.value("WState/SplitterList1").toByteArray());
	ListDockWidget::getInstance()->listSplit2->restoreState(settings.value("WState/SplitterList2").toByteArray());
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
	FMFontDb::DB()->setFilterdFonts(flist);
	fillTree();
	
}


void MainViewWidget::slotPanoseFilter(const QMap<int,QList<int> >& filter)
{
	QList<FontDBResult> dbresult( FMFontDb::DB()->getValues( FMFontDb::Panose ) );
	QList<FontItem*> fil;
	for(int i(0); i < dbresult.count() ; ++i)
	{
		QList<int> list;
		QString p(dbresult[i].second);
		QStringList pl(p.split(":"));
		bool match(true);
		foreach(const int& k , filter.keys())
		{
			if(!filter[k].contains(pl[k].toInt()))
			{
				match = false;
				break;
			}
		}
		if(match)
			fil << dbresult[i].first;
	}
	FMFontDb::DB()->setFilterdFonts( fil );
	//	setCrumb(classificationView->filterAsString());
	fillTree();
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


void MainViewWidget::slotPreviewUpdateSize(int w)
{
	listView->setIconSize(QSize(qRound(w ), 1.3 * typotek::getInstance()->getPreviewSize() * typotek::getInstance()->getDpiY() / 72.0));
}

void MainViewWidget::slotQuickSearch(const QString& text)
{
	int t(quickSearchTime.elapsed());
	bool hasText(false);
	qDebug()<<text<<t<<quickSearchString;
	if(quickSearchString.isEmpty() || (t > quickSearchWait) )
	{
		quickSearchWidget->show();
		if(!quickSearchTimer)
		{
			quickSearchTimer = new QTimer;
			connect(quickSearchTimer, SIGNAL(timeout()), this, SLOT(slotEndQuickSearch()));
		}
		quickSearchTimer->start(quickSearchWait);
		quickSearchString = text;
		quickSearch->setText(quickSearchString);
		quickSearch->setFocus(Qt::OtherFocusReason);
		quickSearchTime.start();
		hasText = listView->moveTo(quickSearchString);
	}
	else if(t < quickSearchWait)
	{
		if(sender() != quickSearch)
		{
			quickSearch->setText(quickSearchString);
			quickSearchString += text;
		}
		else
			quickSearchString = text;
		quickSearchTime.restart();
		quickSearchTimer->start(quickSearchWait);
		hasText = listView->moveTo(quickSearchString);

	}
	else
	{
		quickSearchString.clear();
		quickSearchWidget->hide();
	}
	if(hasText)
		quickSearch->setStyleSheet(QString());
	else
		quickSearch->setStyleSheet(QString("background-color:#F44;"));
}

void MainViewWidget::slotEndQuickSearch()
{
	quickSearchWidget->hide();
	quickSearchString.clear();
	quickSearchTimer->stop();
	listView->setFocus(Qt::OtherFocusReason);
	quickSearch->setStyleSheet(QString());
}

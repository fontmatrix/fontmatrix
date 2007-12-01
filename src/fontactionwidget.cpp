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
#include "fontactionwidget.h"
#include "fontitem.h"
#include "typotek.h"
#include "typotekadaptator.h"

#include <QDebug>
#include <QListWidgetItem>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QAction>


FontActionWidget::FontActionWidget(QWidget * parent)
	: QWidget ( parent )
{
	setupUi ( this );
	isOk = false;
	contextMenuReq = false;
	
	tagsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	
	doConnect();
}

FontActionWidget::FontActionWidget ( TypotekAdaptator* ada,QWidget* parent ) 
	: QWidget ( parent ), adaptator(ada)
{
	setupUi ( this );
	isOk = false;
	contextMenuReq = false;
	
	tagsListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	
	doConnect();

}

void FontActionWidget::prepare ( QList< FontItem * > fonts )
{

	slotFinalize();
	isOk = false;
	theFonts.clear();
	theFonts = fonts;
	for ( int i= theFonts.count() - 1; i >= 0; --i )
	{
		if ( theFonts[i]->isLocked() )
			theFonts.removeAt ( i );
	}
	tagsListWidget->clear();

	QString tot;
	for ( int i=0;i<theFonts.count();++i )
	{
		bool last = i == theFonts.count() - 1;
		tot.append (  theFonts[i]->fancyName() + (last ? "" : "\n") );
	}
// 	QString itsagroup = theFonts.count() > 1 ? " - " + theFonts.last()->name() :"";
// 	titleLabel->setText ( tit.arg ( theFonts[0]->fancyName() ) + itsagroup );
	if(theFonts.count() > 1)
	{
		titleLabel->setText ( theFonts[0]->family() + " (family)");
	}
	else
	{
		titleLabel->setText ( theFonts[0]->fancyName() );
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
			for ( int i=0;i<theFonts.count();++i )
			{
					if ( theFonts[i]->tags().contains ( cur_tag ) )
						++YesState;
			}
			if(YesState == theFonts.count())
				lit->setCheckState ( Qt::Checked );
			else if(YesState > 0 && YesState < theFonts.count())
				lit->setCheckState ( Qt::PartiallyChecked);
			
			tagsListWidget->addItem ( lit );
		}
	}
	
}

void FontActionWidget::doConnect()
{
// 	connect ( buttonBox,SIGNAL ( accepted() ),this,SLOT ( slotOk() ) );
// 	connect ( buttonBox,SIGNAL ( rejected() ),this,SLOT ( slotCancel() ) );

	connect(tagsListWidget,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
	
	connect ( tagsListWidget,SIGNAL ( itemClicked ( QListWidgetItem* ) ),this,SLOT ( slotSwitchCheckState ( QListWidgetItem* ) ) );
	connect ( newTagButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotNewTag() ) );
	
	
}

FontActionWidget::~FontActionWidget()
{
}

void FontActionWidget::slotOk()
{
	isOk = true;
	
// 	close();
	slotFinalize();
}

void FontActionWidget::slotCancel()
{
	isOk = false;
	
// 	close();
	slotFinalize();
}

void FontActionWidget::slotSwitchCheckState ( QListWidgetItem * item )
{
	qDebug() << "FontActionWidget::slotSwitchCheckState ( QListWidgetItem * "<<item<<" )";
// 	item->setCheckState ( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
	
	if(contextMenuReq)
	{
		typotek *typo = typotek::getInstance();
		QStringList sets = typo->tagsets();
		QMenu menu(tagsListWidget);
// 		QAction *mTitle = menu.addAction("Add To Set");
		for(int i=0; i< sets.count(); ++i)
		{
			
			if(typo->tagsOfSet(sets[i]).contains(item->text()))
			{
				QAction *entry = menu.addAction(QString("Remove from %1").arg(sets[i]));
				entry->setData(sets[i]);
// 				entry->setEnabled(false);
// 				QAction ent(sets[i], mTitle);
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

void FontActionWidget::slotNewTag()
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

void FontActionWidget::slotFinalize()
{
// 	if ( isOk )
// 	{
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
		for ( int i=0;i<theFonts.count();++i )
		{
			sourceTags = theFonts[i]->tags();
			// sourceTags -= noTags;
			for(int t = 0; t < noTags.count(); ++t)
			{
				sourceTags.removeAll(noTags[t]);
			}
			sourceTags += plusTags;
			sourceTags = sourceTags.toSet().toList();
			theFonts[i]->setTags ( sourceTags );
		}
// 	}

	emit cleanMe();
}

void FontActionWidget::slotContextMenu(QPoint pos)
{
	qDebug() << "FontActionWidget::slotContextMenu("<<pos<<")";
	contextMenuReq = true;
	contextMenuPos = tagsListWidget->mapToGlobal( pos );
}








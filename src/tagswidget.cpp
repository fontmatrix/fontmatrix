//
// C++ Implementation: tagswidget
//
// Description:
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

#include "typotek.h"
#include "tagswidget.h"
#include "fontitem.h"
#include "fmfontdb.h"
#include "listdockwidget.h"


TagsWidget * TagsWidget::instance = 0;
TagsWidget::TagsWidget ( QWidget * parent )
		:QWidget ( parent )
{
	setupUi ( this );

	tagsListWidget->setContextMenuPolicy ( Qt::CustomContextMenu );


	connect ( tagsListWidget,SIGNAL ( itemClicked ( QListWidgetItem* ) ),this,SLOT ( slotSwitchCheckState ( QListWidgetItem* ) ) );
	connect ( newTagButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotNewTag() ) );
	connect ( newTagName, SIGNAL ( editingFinished() ), this, SLOT ( slotNewTag() ) );
	connect ( tagsListWidget,SIGNAL ( customContextMenuRequested ( const QPoint & ) ), this, SLOT ( slotContextMenu ( QPoint ) ) );
}

TagsWidget::~ TagsWidget()
{
}

TagsWidget * TagsWidget::getInstance()
{
	if ( !instance )
	{
		instance = new TagsWidget ( 0 );
		Q_ASSERT ( instance );
	}
	return instance;
}

void TagsWidget::slotSwitchCheckState ( QListWidgetItem * item )
{
	slotFinalize();
}

void TagsWidget::slotNewTag()
{
	QString nTag ( newTagName->text() );
	newTagName->clear();
	bool ok;
// 	nTag = QInputDialog::getText(this,"Fontmatrix",tr("Add new tag"),QLineEdit::Normal, QString() , &ok );
	if ( nTag.isEmpty() || FMFontDb::DB()->getTags().contains ( nTag ) )
		return;

	FMFontDb::DB()->addTagToDB ( nTag );
	QListWidgetItem *lit = new QListWidgetItem ( nTag );
	lit->setCheckState ( Qt::Checked );
	tagsListWidget->addItem ( lit );
	slotFinalize();

	ListDockWidget::getInstance()->reloadTagsCombo();
}

void TagsWidget::slotFinalize()
{
	QStringList plusTags;
	QStringList noTags;
	for ( int i=0;i< tagsListWidget->count();++i )
	{
		if ( tagsListWidget->item ( i )->checkState() == Qt::Checked )
			plusTags.append ( tagsListWidget->item ( i )->text() );
		if ( tagsListWidget->item ( i )->checkState() == Qt::Unchecked )
			noTags.append ( tagsListWidget->item ( i )->text() );
	}

	QStringList sourceTags;
	
	QMap<QString,QStringList> refTagLists;
	for ( int i=0;i<theTaggedFonts.count();++i )
	{
		refTagLists[theTaggedFonts[i]->path()] = theTaggedFonts[i]->tags();
	}
	FMFontDb::DB()->TransactionBegin();
	for ( int i=0;i<theTaggedFonts.count();++i )
	{
		QStringList refTags = sourceTags = refTagLists[theTaggedFonts[i]->path()];
// 		qDebug() <<refTags.join ( " " );
		for ( int t = 0; t < noTags.count(); ++t )
		{
			sourceTags.removeAll ( noTags[t] );
		}
		sourceTags += plusTags;
		sourceTags = sourceTags.toSet().toList();
		bool changed ( false );
// 		qDebug() <<sourceTags.join ( " " );
		if ( refTags.count() != sourceTags.count() )
			changed = true;
		else
		{
			foreach ( QString t, sourceTags )
			{
				if ( !refTags.contains ( t ) )
				{
					changed = true;
					break;
				}
			}
		}
		if ( changed )
			theTaggedFonts[i]->setTags ( sourceTags );
	}
	FMFontDb::DB()->TransactionEnd();
}

void TagsWidget::prepare ( QList< FontItem * > fonts )
{
	theTaggedFonts.clear();
	theTaggedFonts = fonts;

	bool readOnly ( false );
// 	for ( int i ( 0 ); i < theTaggedFonts.count() ; ++i )
// 	{
// 		if ( theTaggedFonts[i]->isLocked() )
// 		{
// 			readOnly = true;
// 			break;
// 		}
// 	}
	tagsListWidget->clear();
	QString tot;
	for ( int i=0;i<theTaggedFonts.count();++i )
	{
		tot.append ( theTaggedFonts[i]->fancyName() +  "\n" );
	}
	if ( theTaggedFonts.count() > 1 )
	{
		titleLabel->setText ( theTaggedFonts[0]->family() + " (family)" );
	}
	else
	{
		titleLabel->setText ( theTaggedFonts[0]->fancyName() );
	}
	titleLabel->setToolTip ( tot );
	QStringList tagsList ( FMFontDb::DB()->getTags() );
	
	QString sysTag(typotek::getInstance()->getSysTagName());
	tagsList.removeAll(sysTag);
	
	QMap<FontItem*, QStringList> tmap;
	foreach(FontItem* fi, theTaggedFonts)
	{
		if(fi)
			tmap[fi] = FMFontDb::DB()->getValue(fi->path(), FMFontDb::Tags, false).toStringList();
	}
	for ( int i=0; i < tagsList.count(); ++i )
	{
		QString cur_tag = tagsList[i];

		if ( (cur_tag.isEmpty()) || (cur_tag == sysTag) )
			continue;

		QListWidgetItem *lit;

		{
			lit = new QListWidgetItem ( cur_tag );
			lit->setCheckState ( Qt::Unchecked );
			int YesState = 0;
			for ( int i=0;i<theTaggedFonts.count();++i )
			{
				if ( tmap[theTaggedFonts[i]].contains ( cur_tag ) )
					++YesState;
			}
			if ( YesState == theTaggedFonts.count() )
				lit->setCheckState ( Qt::Checked );
			else if ( YesState > 0 && YesState < theTaggedFonts.count() )
				lit->setCheckState ( Qt::PartiallyChecked );

			tagsListWidget->addItem ( lit );
			if ( readOnly )
				lit->setFlags ( 0 );// No NoItemFlags in Qt < 4.4
		}
	}
}

void TagsWidget::newTag()
{
	slotNewTag();
}

void TagsWidget::slotContextMenu ( QPoint pos )
{
// 	if(theTaggedFonts.isEmpty())
// 		return;

	if(!tagsListWidget->selectedItems().count())
		return;
	currentTag = tagsListWidget->selectedItems().first()->text();
	if ( currentTag.isEmpty() )
		return;
// 	QString fs(theTaggedFonts.first()->fancyName() + ((theTaggedFonts.count() > 1) ? "…" :""));

	foreach ( QAction* a, contAction )
	{
		delete a;
	}
	contAction.clear();

// 	contAction << new QAction(tr("Untag") + " " + fs, this);
// 	connect(contAction.last(),SIGNAL(triggered()), this, SLOT(slotActUntag()));

	contAction << new QAction ( tr ( "Edit", "followed by a tag name" ) + QString ( " \""+currentTag+"\"" ), this );
	connect ( contAction.last(),SIGNAL ( triggered() ), this, SLOT ( slotActEditTag() ) );
//
	contAction << new QAction ( tr ( "Remove tag \"%1\" from database", "the %%1 is a tag name" ).arg ( currentTag ), this );
	connect ( contAction.last(),SIGNAL ( triggered() ), this, SLOT ( slotActRemovetag() ) );

	QMenu menu ( tagsListWidget );
	foreach ( QAction* a, contAction )
	{
		menu.addAction ( a );
	}
	menu.exec ( QCursor::pos() );
}

void TagsWidget::slotActRemovetag()
{
// 	qDebug() <<"TagsWidget::slotActRemovetag";
	QString message;
	message = tr ( "Please confirm that you want to remove\nthe following tag from database:" ) + " " + currentTag;
	if ( QMessageBox::question ( typotek::getInstance(),
	                             "Fontmatrix",
	                             message ,
	                             QMessageBox::Ok | QMessageBox::Cancel,
	                             QMessageBox::Cancel )
	        == QMessageBox::Ok )
	{
		FMFontDb::DB()->removeTagFromDB ( currentTag );
		prepare(theTaggedFonts);
	}

}

void TagsWidget::slotActEditTag()
{
// 	qDebug()<<"TagsWidget::slotActEditTag";
	QString fromT(currentTag);
	QString message;
	message = tr ( "Please provide a replacement name for\nthe following tag:") + " " + currentTag ;
	QString nt = QInputDialog::getText ( typotek::getInstance(),
	                                     "Fontmatrix",
	                                     message,
	                                     QLineEdit::Normal,
	                                     currentTag ) ;
	if ( ( nt != currentTag ) && ( !nt.isEmpty() ) )
	{
		FMFontDb::DB()->editTag ( currentTag, nt );
		prepare(theTaggedFonts);
	}
	
}




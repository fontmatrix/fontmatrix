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
#include <QDebug>

#include "tagswidget.h"
#include "fmfontdb.h"
#include "listdockwidget.h"


TagsWidget * TagsWidget::instance = 0;
TagsWidget::TagsWidget(QWidget * parent)
	:QWidget(parent)
{
	setupUi ( this );
	
	connect ( tagsListWidget,SIGNAL ( itemClicked ( QListWidgetItem* ) ),this,SLOT ( slotSwitchCheckState ( QListWidgetItem* ) ) );
	connect ( newTagButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotNewTag() ) );
}

TagsWidget::~ TagsWidget()
{
}

TagsWidget * TagsWidget::getInstance()
{
	if(!instance)
	{
		instance = new TagsWidget(0);
		Q_ASSERT(instance);
	}
	return instance;
}

void TagsWidget::slotSwitchCheckState(QListWidgetItem * item)
{
	slotFinalize();
}

void TagsWidget::slotNewTag()
{
	QString nTag;
	bool ok;
	nTag = QInputDialog::getText(this,"Fontmatrix",tr("Add new tag"),QLineEdit::Normal, QString() , &ok );
	if ( !ok || nTag.isEmpty() || FMFontDb::DB()->getTags().contains ( nTag ))
		return;

	FMFontDb::DB()->addTagToDB( nTag );
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
		if( tagsListWidget->item ( i )->checkState() == Qt::Unchecked )
			noTags.append ( tagsListWidget->item ( i )->text() );
	}

	QStringList sourceTags;
	QStringList refTags;
	for ( int i=0;i<theTaggedFonts.count();++i )
	{
		refTags = sourceTags = theTaggedFonts[i]->tags();
		qDebug()<<refTags.join(" ");
		for(int t = 0; t < noTags.count(); ++t)
		{
			sourceTags.removeAll(noTags[t]);
		}
		sourceTags += plusTags;
		sourceTags = sourceTags.toSet().toList();
		bool changed(false);
		qDebug()<<sourceTags.join(" ");
		if(refTags.count() != sourceTags.count())
			changed = true;
		else
		{
			foreach(QString t, sourceTags)
			{
				if(!refTags.contains(t))
				{
					changed = true;
					break;
				}
			}
		}
		if(changed)
			theTaggedFonts[i]->setTags ( sourceTags );
	}
}

void TagsWidget::prepare(QList< FontItem * > fonts)
{
	theTaggedFonts.clear();
	theTaggedFonts = fonts;

	bool readOnly(false);
	for ( int i(0); i < theTaggedFonts.count() ; ++i )
	{
		if ( theTaggedFonts[i]->isLocked() )
		{
			readOnly = true;
			break;
		}
	}
	tagsListWidget->clear();
	QString tot;
	for ( int i=0;i<theTaggedFonts.count();++i )
	{
		tot.append (  theTaggedFonts[i]->fancyName() +  "\n" );
	}
	if(theTaggedFonts.count() > 1)
	{
		titleLabel->setText ( theTaggedFonts[0]->family() + " (family)");
	}
	else
	{
		titleLabel->setText ( theTaggedFonts[0]->fancyName() );
	}
	titleLabel->setToolTip ( tot );
	QStringList tagsList(FMFontDb::DB()->getTags());
	for ( int i=0; i < tagsList.count(); ++i )
	{
		QString cur_tag = tagsList[i];

		if ( cur_tag.isEmpty() )
			continue;

		QListWidgetItem *lit;

		{
			lit = new QListWidgetItem ( cur_tag );
			lit->setCheckState ( Qt::Unchecked );
			int YesState = 0;
			for ( int i=0;i<theTaggedFonts.count();++i )
			{
				QStringList fTags(theTaggedFonts[i]->tags());
				if ( fTags.contains ( cur_tag ) )
					++YesState;
			}
			if(YesState == theTaggedFonts.count())
				lit->setCheckState ( Qt::Checked );
			else if(YesState > 0 && YesState < theTaggedFonts.count())
				lit->setCheckState ( Qt::PartiallyChecked);

			tagsListWidget->addItem ( lit );
			if(readOnly)
				lit->setFlags(0);// No NoItemFlags in Qt < 4.4
		}
	}
}

void TagsWidget::newTag()
{
	slotNewTag();
}




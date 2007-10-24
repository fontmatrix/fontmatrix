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



FontActionWidget::FontActionWidget ( TypotekAdaptator* ada,QWidget* parent ) : QWidget ( parent ), adaptator(ada)
{
	setupUi ( this );
	isOk = false;
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

	qDebug() << theFonts.count();

	tagsListWidget->clear();


	QString tit ( "%1" );
	QString tot;
	for ( int i=0;i<theFonts.count();++i )
	{
		tot.append ( "[" + theFonts[i]->name() + "] " );
	}
	QString itsagroup = theFonts.count() > 1 ? " - " + theFonts.last()->name() :"";
	titleLabel->setText ( tit.arg ( theFonts[0]->name() ) + itsagroup );
	titleLabel->setToolTip ( tot );

	for ( int i=0; i < typotek::tagsList.count(); ++i )
	{
		QString cur_tag = typotek::tagsList[i];

		if ( cur_tag.isEmpty() )
			continue;

		QListWidgetItem *lit;
		if ( !cur_tag.contains ( "Activated_" ) )
		{
			lit = new QListWidgetItem ( cur_tag );
			lit->setCheckState ( Qt::Unchecked );

			if ( theFonts.count() == 1 )
			{
				if ( theFonts[0]->tags().contains ( cur_tag ) )
					lit->setCheckState ( Qt::Checked );
			}
			tagsListWidget->addItem ( lit );
		}
	}
	
}

void FontActionWidget::doConnect()
{
// 	connect ( buttonBox,SIGNAL ( accepted() ),this,SLOT ( slotOk() ) );
// 	connect ( buttonBox,SIGNAL ( rejected() ),this,SLOT ( slotCancel() ) );

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
	item->setCheckState ( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
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

}

void FontActionWidget::slotFinalize()
{
// 	if ( isOk )
// 	{
		QStringList tags;
		for ( int i=0;i< tagsListWidget->count();++i )
		{
			if ( tagsListWidget->item ( i )->checkState() == Qt::Checked )
				tags.append ( tagsListWidget->item ( i )->text() );
		}
		
		
		for ( int i=0;i<theFonts.count();++i )
		{
			if(theFonts[i]->tags().contains("Activated_On"))
				tags << "Activated_On";
			if(theFonts[i]->tags().contains("Activated_Off"))
				tags << "Activated_Off";
			theFonts[i]->setTags ( tags );
		}
// 	}

	emit cleanMe();
}





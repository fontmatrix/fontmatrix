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
#include <QMenu>
#include <QDebug>
#include <QFont>

#include "typotek.h"
#include "tagswidget.h"
#include "fontitem.h"
#include "fmfontdb.h"
#include "tagswidget_listmodel.h"



TagsWidget::TagsWidget ( QWidget * parent )
		:QWidget ( parent )
{
	setupUi ( this );

	model = new TagsWidget_ListModel(this);
	tagsListView->setModel(model);

	connect ( newTagButton,SIGNAL ( clicked ( bool ) ),this,SLOT ( slotNewTag() ) );
	connect( removeTagButton, SIGNAL(clicked()), this, SLOT(slotActRemovetag()));

}

TagsWidget::~ TagsWidget()
{
}

void TagsWidget::prepare(QList<FontItem *> fonts)
{
	model->setFonts(fonts);
}

void TagsWidget::slotNewTag()
{

//	QString nTag(tr("New Tag"));
//	newTagBeingEdited = new QListWidgetItem(0);
//	newTagBeingEdited->setText(nTag);
//	newTagBeingEdited->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
//	newTagBeingEdited->setCheckState(Qt::Unchecked);
//	FMFontDb::DB()->addTagToDB ( nTag );
//	currentTag = nTag;


//	tagsListWidget->setFocus();
//	tagsListWidget->addItem(newTagBeingEdited);
//	tagsListWidget->scrollToItem(newTagBeingEdited);
//	tagsListWidget->setCurrentItem(newTagBeingEdited);
//	tagsListWidget->openPersistentEditor(newTagBeingEdited);

//	disconnect(tagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(slotActEditTag(QListWidgetItem*)));
//	connect(tagsListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(slotActEditTag(QListWidgetItem*)));
}



void TagsWidget::slotActRemovetag()
{
//	if(currentTag.isEmpty())
//		return;
	QString message;
	message = tr ( "Please confirm that you want to remove\nthe following tag from database:" ) /*+ " " + currentTag*/;
	if ( QMessageBox::question ( typotek::getInstance(),
	                             "Fontmatrix",
	                             message ,
	                             QMessageBox::Ok | QMessageBox::Cancel,
	                             QMessageBox::Cancel )
	        == QMessageBox::Ok )
	{
//		FMFontDb::DB()->removeTagFromDB ( currentTag );
	}

}


